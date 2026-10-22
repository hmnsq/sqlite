/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.  This module is responsible for
** generating the code that loops through a table looking for applicable
** rows.  Indices are selected and used to speed the search when doing
** so is applicable.  Because this module is responsible for selecting
** indices, you might also think of this module as the "query optimizer".
*/
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
પ્રકારવ્યાખ્યા સંરચના HiddenIndexInfo HiddenIndexInfo;
સંરચના HiddenIndexInfo {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  પૂર્ણાંક eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 mIn;                 /* Mask of terms that are <col> IN (...) */
  u32 mHandleIn;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_કિંમત *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#define SZ_HIDDENINDEXINFO(N) \
                  (offsetof(HiddenIndexInfo,aRhs) + (N)*કદ(sqlite3_કિંમત*))

/* Forward declaration of methods */
સ્થિર પૂર્ણાંક whereLoopResize(sqlite3*, WhereLoop*, પૂર્ણાંક);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_Row_ગણતરી(WhereInfo *pWInfo){
  પાછા pWInfo->nRowOut;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
પૂર્ણાંક sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  પાછા pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
પૂર્ણાંક sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  પાછા pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
}

/*
** In the ORDER BY LIMIT optimization, if the inner-most loop is known
** to emit rows in increasing order, and if the last row emitted by the
** inner-most loop did not fit within the sorter, then we can skip all
** subsequent rows for the current iteration of the inner loop (because they
** will not fit in the sorter either) and continue with the second inner
** loop - the loop immediately outside the inner-most.
**
** When a row does not fit in the sorter (because the sorter already
** holds LIMIT+OFFSET rows that are smaller), then a jump is made to the
** label returned by this function.
**
** If the ORDER BY LIMIT optimization applies, the jump destination should
** be the continuation for the second-inner-most loop.  If the ORDER BY
** LIMIT optimization does not apply, then the jump destination should
** be the continuation for the inner-most loop.
**
** It is always safe for this routine to return the continuation of the
** inner-most loop, in the sense that a correct answer will result. 
** Returning the continuation the second inner loop is an optimization
** that might make the code run a little faster, but should not change
** the final answer.
*/
પૂર્ણાંક sqlite3WhereOrderByLimitOptLabel(WhereInfo *pWInfo){
  WhereLevel *pInner;
  જો( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    પાછા pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  પ્રતિજ્ઞા( pInner->addrNxt!=0 );
  પાછા pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
}

/*
** While generating code for the min/max optimization, after handling
** the aggregate-step call to min() or max(), check to see if any
** additional looping is required.  If the output order is such that
** we are certain that the correct answer has already been found, then
** code an OP_Goto to by pass subsequent processing.
**
** Any extra OP_Goto that is coded here is an optimization.  The
** correct answer should be obtained regardless.  This OP_Goto just
** makes the answer appear faster.
*/
શૂન્ય sqlite3WhereMinMaxOptEarlyOut(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  પૂર્ણાંક i;
  જો( !pWInfo->bOrderedInnerLoop ) પાછા;
  જો( pWInfo->nOBSat==0 ) પાછા;
  માટે(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    જો( (pInner->pWLoop->wsFlags & WHERE_COLUMN_IN)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      પાછા;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
પૂર્ણાંક sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  પ્રતિજ્ઞા( pWInfo->iContinue!=0 );
  પાછા pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
પૂર્ણાંક sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  પાછા pWInfo->iBreak;
}

/*
** Return ONEPASS_OFF (0) if an UPDATE or DELETE statement is unable to
** operate directly on the rowids returned by a WHERE clause.  Return
** ONEPASS_SINGLE (1) if the statement can operation directly because only
** a single row is to be changed.  Return ONEPASS_MULTI (2) if the one-pass
** optimization can be used on multiple
**
** If the ONEPASS optimization is used (if this routine returns true)
** then also write the indices of open cursors used by ONEPASS
** into aiCur[0] and aiCur[1].  iaCur[0] gets the cursor of the data
** table and aiCur[1] gets the cursor used by an auxiliary index.
** Either value may be -1, indicating that cursor is not used.
** Any cursors returned will have been opened for writing.
**
** aiCur[0] and aiCur[1] both get -1 if the where-clause logic is
** unable to use the ONEPASS optimization.
*/
પૂર્ણાંક sqlite3WhereOkOnePass(WhereInfo *pWInfo, પૂર્ણાંક *aiCur){
  memcpy(aiCur, pWInfo->aiCurOnePass, કદ(પૂર્ણાંક)*2);
#ifdef WHERETRACE_ENABLED
  જો( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#endif
  પાછા pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
પૂર્ણાંક sqlite3WhereUsesDeferredSeek(WhereInfo *pWInfo){
  પાછા pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
સ્થિર શૂન્ય whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc){
  pDest->n = pSrc->n;
  memcpy(pDest->a, pSrc->a, pDest->n*કદ(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
સ્થિર પૂર્ણાંક where_Or_ઉમેરો(
  WhereOrSet *pSet,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst rRun,           /* Run-cost of the new entry */
  LogEst nOut            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  માટે(i=pSet->n, p=pSet->a; i>0; i--, p++){
    જો( rRun<=p->rRun && (prereq & p->prereq)==prereq ){
      જાઓ where_Or_ઉમેરો_done;
    }
    જો( p->rRun<=rRun && (p->prereq & prereq)==p->prereq ){
      પાછા 0;
    }
  }
  જો( pSet->n<N_OR_COST ){
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  }નહીંતર{
    p = pSet->a;
    માટે(i=1; i<pSet->n; i++){
      જો( p->rRun>pSet->a[i].rRun ) p = pSet->a + i;
    }
    જો( p->rRun<=rRun ) પાછા 0;
  }
where_Or_ઉમેરો_done:
  p->prereq = prereq;
  p->rRun = rRun;
  જો( p->nOut>nOut ) p->nOut = nOut;
  પાછા 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, પૂર્ણાંક iCursor){
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( pMaskSet->n<=(પૂર્ણાંક)કદ(Bitmask)*8 );
  પ્રતિજ્ઞા( pMaskSet->n>0 || pMaskSet->ix[0]<0 );
  પ્રતિજ્ઞા( iCursor>=-1 );
  જો( pMaskSet->ix[0]==iCursor ){
    પાછા 1;
  }
  માટે(i=1; i<pMaskSet->n; i++){
    જો( pMaskSet->ix[i]==iCursor ){
      પાછા MASKBIT(i);
    }
  }
  પાછા 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
શૂન્ય *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  WhereMemBlock *pBlock;
  pBlock = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+કદ(*pBlock));
  જો( pBlock ){
    pBlock->pNext = pWInfo->pMemToFree;
    pBlock->sz = nByte;
    pWInfo->pMemToFree = pBlock;
    pBlock++;
  }
  પાછા (શૂન્ય*)pBlock;
}
શૂન્ય *sqlite3WhereRealloc(WhereInfo *pWInfo, શૂન્ય *pOld, u64 nByte){
  શૂન્ય *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  જો( pNew && pOld ){
    WhereMemBlock *pOldBlk = (WhereMemBlock*)pOld;
    pOldBlk--;
    પ્રતિજ્ઞા( pOldBlk->sz<nByte );
    memcpy(pNew, pOld, pOldBlk->sz);
  }
  પાછા pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
સ્થિર શૂન્ય createMask(WhereMaskSet *pMaskSet, પૂર્ણાંક iCursor){
  પ્રતિજ્ઞા( pMaskSet->n < ArraySize(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
સ્થિર પદાવલિ *where_જમણું_Subexpr_Is_Column(પદાવલિ *p){
  p = sqlite3_પદાવલિ_Skip_Collate_And_Likely(p->p_જમણું);
  જો( ALWAYS(p!=0) && p->op==TK_COLUMN && !પદાવલિ_Has_Property(p, EP_FixedCol) ){
    પાછા p;
  }
  પાછા 0;
}

/*
** Term pTerm is guaranteed to be a WO_IN term. It may be a component term
** of a vector IN expression of the form "(x, y, ...) IN (SELECT ...)".
** This function checks to see if the term is compatible with an index
** column with affinity idxaff (one of the SQLITE_AFF_XYZ values). If so,
** it returns a pointer to the name of the collation sequence (e.g. "BINARY"
** or "NOCASE") used by the comparison in pTerm. If it is not compatible
** with affinity idxaff, NULL is returned.
*/
સ્થિર SQLITE_NOINLINE અચલ અક્ષર *indexInAffinityOk(
  Parse *pParse, 
  WhereTerm *pTerm, 
  u8 idxaff
){
  પદાવલિ *pX = pTerm->p_પદાવલિ;
  પદાવલિ inexpr;

  પ્રતિજ્ઞા( pTerm->eOperator & WO_IN );

  જો( પદાવલિ_Use_XSelect(pX) ){
    પૂર્ણાંક iField = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    જો( sqlite3_પદાવલિ_Is_Vector(pX->p_ડાબું) ){
      iField = pTerm->u.x.iField - 1;
      inexpr.p_ડાબું = pX->p_ડાબું->x.pList->a[iField].p_પદાવલિ;
    }નહીંતર{
      inexpr.p_ડાબું = pX->p_ડાબું;
    }
    inexpr.p_જમણું = pX->x.pSelect->pEList->a[iField].p_પદાવલિ;
    pX = &inexpr;
  }

  જો( sqlite3IndexAffinityOk(pX, idxaff) ){
    CollSeq *pRet = sqlite3_પદાવલિ_Compare_Coll_Seq(pParse, pX);
    પાછા pRet ? pRet->zName : sqlite3_Str_દ્વિઅંકી;
  }
  પાછા 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
સ્થિર WhereTerm *whereScanNext(WhereScan *pScan){
  પૂર્ણાંક iCur;            /* The cursor on the LHS of the term */
  i16 iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  પદાવલિ *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  WhereTerm *pTerm;    /* The term being tested */
  પૂર્ણાંક k = pScan->k;    /* Where to start scanning */

  પ્રતિજ્ઞા( pScan->iEquiv<=pScan->nEquiv );
  pWC = pScan->pWC;
  જ્યારે(1){
    iColumn = pScan->aiColumn[pScan->iEquiv-1];
    iCur = pScan->aiCur[pScan->iEquiv-1];
    પ્રતિજ્ઞા( pWC!=0 );
    પ્રતિજ્ઞા( iCur>=0 );
    કરો{
      માટે(pTerm=pWC->a+k; k<pWC->nTerm; k++, pTerm++){
        પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 || pTerm->ડાબું_Cursor<0 );
        જો( pTerm->ડાબું_Cursor==iCur
         && pTerm->u.x.ડાબું_Column==iColumn
         && (iColumn!=XN_પદાવલિ
             || sqlite3_પદાવલિ_Compare_Skip(pTerm->p_પદાવલિ->p_ડાબું,
                                       pScan->p_Idx_પદાવલિ,iCur)==0)
         && (pScan->iEquiv<=1 || !પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON))
        ){
          જો( (pTerm->eOperator & WO_EQUIV)!=0
           && pScan->nEquiv<ArraySize(pScan->aiCur)
           && (pX = where_જમણું_Subexpr_Is_Column(pTerm->p_પદાવલિ))!=0
          ){
            પૂર્ણાંક j;
            માટે(j=0; j<pScan->nEquiv; j++){
              જો( pScan->aiCur[j]==pX->iTable
               && pScan->aiColumn[j]==pX->iColumn ){
                  તોડો;
              }
            }
            જો( j==pScan->nEquiv ){
              pScan->aiCur[j] = pX->iTable;
              pScan->aiColumn[j] = pX->iColumn;
              pScan->nEquiv++;
            }
          }
          જો( (pTerm->eOperator & pScan->opMask)!=0 ){
            /* Verify the affinity and collating sequence match */
            જો( pScan->zCollName && (pTerm->eOperator & WO_ISNULL)==0 ){
              અચલ અક્ષર *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->p_પદાવલિ;

              જો( (pTerm->eOperator & WO_IN) ){
                zCollName = indexInAffinityOk(pParse, pTerm, pScan->idxaff);
                જો( !zCollName ) ચાલુ;
              }નહીંતર{
                CollSeq *pColl;
                જો( !sqlite3IndexAffinityOk(pX, pScan->idxaff) ){
                  ચાલુ;
                }
                પ્રતિજ્ઞા(pX->p_ડાબું);
                pColl = sqlite3_પદાવલિ_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_દ્વિઅંકી;
              }

              જો( sqlite3StrICmp(zCollName, pScan->zCollName) ){
                ચાલુ;
              }
            }
            જો( (pTerm->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = pTerm->p_પદાવલિ->p_જમણું, ALWAYS(pX!=0))
             && pX->op==TK_COLUMN
             && pX->iTable==pScan->aiCur[0]
             && pX->iColumn==pScan->aiColumn[0]
            ){
              testcase( pTerm->eOperator & WO_IS );
              ચાલુ;
            }
            pScan->pWC = pWC;
            pScan->k = k+1;
#ifdef WHERETRACE_ENABLED
            જો( (sqlite3WhereTrace & 0x20000)!=0 && pScan->nEquiv>1 ){
              પૂર્ણાંક ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 pScan->aiCur[0], pScan->aiColumn[0], pTerm->iTerm);
              માટે(ii=1; ii<pScan->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   pScan->aiCur[ii], pScan->aiColumn[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#endif
            પાછા pTerm;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }જ્યારે( pWC!=0 );
    જો( pScan->iEquiv>=pScan->nEquiv ) તોડો;
    pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv++;
  }
  પાછા 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
સ્થિર SQLITE_NOINLINE WhereTerm *where_Scan_Init_Index_પદાવલિ(WhereScan *pScan){
  pScan->idxaff = sqlite3_પદાવલિ_Affinity(pScan->p_Idx_પદાવલિ);
  પાછા whereScanNext(pScan);
}

/*
** Initialize a WHERE clause scanner object.  Return a pointer to the
** first match.  Return NULL if there are no matches.
**
** The scanner will be searching the WHERE clause pWC.  It will look
** for terms of the form "X <op> <expr>" where X is column iColumn of table
** iCur.   Or if pIdx!=0 then X is column iColumn of index pIdx.  pIdx
** must be one of the indexes of table iCur.
**
** The <op> must be one of the operators described by opMask.
**
** If the search is for X and the WHERE clause contains terms of the
** form X=Y then this routine might also return terms of the form
** "Y <op> <expr>".  The number of levels of transitivity is limited,
** but is enough to handle most commonly occurring SQL statements.
**
** If X is not the INTEGER PRIMARY KEY then X must be compatible with
** index pIdx.
*/
સ્થિર WhereTerm *whereScanInit(
  WhereScan *pScan,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  પૂર્ણાંક iCur,               /* Cursor to scan for */
  પૂર્ણાંક iColumn,            /* Column to scan for */
  u32 opMask,             /* Operator(s) to scan for */
  Index *pIdx             /* Must be compatible with this index */
){
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  pScan->p_Idx_પદાવલિ = 0;
  pScan->idxaff = 0;
  pScan->zCollName = 0;
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aiCur[0] = iCur;
  pScan->nEquiv = 1;
  pScan->iEquiv = 1;
  જો( pIdx ){
    પૂર્ણાંક j = iColumn;
    iColumn = pIdx->aiColumn[j];
    જો( iColumn==pIdx->pTable->iPKey ){
      iColumn = XN_ROWID;
    }નહીંતર જો( iColumn>=0 ){
      pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
      pScan->zCollName = pIdx->azColl[j];
    }નહીંતર જો( iColumn==XN_પદાવલિ ){
      pScan->p_Idx_પદાવલિ = pIdx->a_Col_પદાવલિ->a[j].p_પદાવલિ;
      pScan->zCollName = pIdx->azColl[j];
      pScan->aiColumn[0] = XN_પદાવલિ;
      પાછા where_Scan_Init_Index_પદાવલિ(pScan);
    }
  }નહીંતર જો( iColumn==XN_પદાવલિ ){
    પાછા 0;
  }
  pScan->aiColumn[0] = iColumn;
  પાછા whereScanNext(pScan);
}

/*
** Search for a term in the WHERE clause that is of the form "X <op> <expr>"
** where X is a reference to the iColumn of table iCur or of index pIdx
** if pIdx!=0 and <op> is one of the WO_xx operator codes specified by
** the op parameter.  Return a pointer to the term.  Return 0 if not found.
**
** If pIdx!=0 then it must be one of the indexes of table iCur. 
** Search for terms matching the iColumn-th column of pIdx
** rather than the iColumn-th column of table iCur.
**
** The term returned might by Y=<expr> if there is another constraint in
** the WHERE clause that specifies that X=Y.  Any such constraints will be
** identified by the WO_EQUIV bit in the pTerm->eOperator field.  The
** aiCur[]/iaColumn[] arrays hold X and all its equivalents. There are 11
** slots in aiCur[]/aiColumn[] so that means we can look for X plus up to 10
** other equivalent values.  Hence a search for X will return <expr> if X=A1
** and A1=A2 and A2=A3 and ... and A9=A10 and A10=<expr>.
**
** If there are multiple terms in the WHERE clause of the form "X <op> <expr>"
** then try for the one with no dependencies on <expr> - in other words where
** <expr> is a constant expression of some kind.  Only return entries of
** the form "X <op> Y" where Y is a column in another table if no terms of
** the form "X <op> <const-expr>" exist.   If no terms with a constant RHS
** exist, try to return a term that does not use WO_EQUIV.
*/
WhereTerm *sqlite3WhereFindTerm(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  પૂર્ણાંક iCur,             /* Cursor number of LHS */
  પૂર્ણાંક iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  Index *pIdx           /* Must be compatible with this index, if not NULL */
){
  WhereTerm *pResult = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  જ્યારે( p ){
    જો( (p->prereq_જમણું & notReady)==0 ){
      જો( p->prereq_જમણું==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        પાછા p;
      }
      જો( pResult==0 ) pResult = p;
    }
    p = whereScanNext(&scan);
  }
  પાછા pResult;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
સ્થિર પૂર્ણાંક findIndexCol(
  Parse *pParse,                  /* Parse context */
  પદાવલિ_List *pList,                /* Expression list to search */
  પૂર્ણાંક iBase,                      /* Cursor for table associated with pIdx */
  Index *pIdx,                    /* Index to match column of */
  પૂર્ણાંક iCol                        /* Column of index to match */
){
  પૂર્ણાંક i;
  અચલ અક્ષર *zColl = pIdx->azColl[iCol];

  માટે(i=0; i<pList->n_પદાવલિ; i++){
    પદાવલિ *p = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pList->a[i].p_પદાવલિ);
    જો( ALWAYS(p!=0)
     && (p->op==TK_COLUMN || p->op==TK_AGG_COLUMN)
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->iTable==iBase
    ){
      CollSeq *pColl = sqlite3_પદાવલિ_NNColl_Seq(pParse, pList->a[i].p_પદાવલિ);
      જો( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        પાછા i;
      }
    }
  }

  પાછા -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
સ્થિર પૂર્ણાંક indexColumnNotNull(Index *pIdx, પૂર્ણાંક iCol){
  પૂર્ણાંક j;
  પ્રતિજ્ઞા( pIdx!=0 );
  પ્રતિજ્ઞા( iCol>=0 && iCol<pIdx->nColumn );
  j = pIdx->aiColumn[iCol];
  જો( j>=0 ){
    પાછા pIdx->pTable->aCol[j].notNull;
  }નહીંતર જો( j==(-1) ){
    પાછા 1;
  }નહીંતર{
    પ્રતિજ્ઞા( j==(-2) );
    પાછા 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
સ્થિર પૂર્ણાંક isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  પદાવલિ_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  Table *pTab;
  Index *pIdx;
  પૂર્ણાંક i;                         
  પૂર્ણાંક iBase;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  જો( pTabList->nSrc!=1 ) પાછા 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  માટે(i=0; i<pDistinct->n_પદાવલિ; i++){
    પદાવલિ *p = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pDistinct->a[i].p_પદાવલિ);
    જો( NEVER(p==0) ) ચાલુ;
    જો( p->op!=TK_COLUMN && p->op!=TK_AGG_COLUMN ) ચાલુ;
    જો( p->iTable==iBase && p->iColumn<0 ) પાછા 1;
  }

  /* Loop through all indices on the table, checking each to see if it makes
  ** the DISTINCT qualifier redundant. It does so if:
  **
  **   1. The index is itself UNIQUE, and
  **
  **   2. All of the columns in the index are either part of the pDistinct
  **      list, or else the WHERE clause contains a term of the form "col=X",
  **      where X is a constant value. The collation sequences of the
  **      comparison and select-list expressions must match those of the index.
  **
  **   3. All of those index columns for which the WHERE clause does not
  **      contain a "col=X" term are subject to a NOT NULL constraint.
  */
  માટે(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    જો( !IsUniqueIndex(pIdx) ) ચાલુ;
    જો( pIdx->pPartIdxWhere ) ચાલુ;
    માટે(i=0; i<pIdx->n_ચાવી_Col; i++){
      જો( 0==sqlite3WhereFindTerm(pWC, iBase, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        જો( findIndexCol(pParse, pDistinct, iBase, pIdx, i)<0 ) તોડો;
        જો( indexColumnNotNull(pIdx, i)==0 ) તોડો;
      }
    }
    જો( i==pIdx->n_ચાવી_Col ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      પાછા 1;
    }
  }

  પાછા 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
સ્થિર LogEst estLog(LogEst N){
  પાછા N<=10 ? 0 : sqlite3LogEst(N) - 33;
}

/*
** Convert OP_Column opcodes to OP_Copy in previously generated code.
**
** This routine runs over generated VDBE code and translates OP_Column
** opcodes into OP_Copy when the table is being accessed via co-routine
** instead of via table lookup.
**
** If the iAutoidxCur is not zero, then any OP_Rowid instructions on
** cursor iTabCur are transformed into OP_Sequence opcode for the
** iAutoidxCur cursor, in order to generate unique rowids for the
** automatic index being generated.
*/
સ્થિર શૂન્ય translateColumnToCopy(
  Parse *pParse,      /* Parsing context */
  પૂર્ણાંક iStart,         /* Translate from this opcode to the end */
  પૂર્ણાંક iTabCur,        /* OP_Column/OP_Rowid references to this table */
  પૂર્ણાંક iRegister,      /* The first column is in this register */
  પૂર્ણાંક iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  પૂર્ણાંક iEnd = sqlite3VdbeCurrentAddr(v);
  જો( pParse->db->mallocFailed ) પાછા;
#ifdef SQLITE_DEBUG
  જો( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    printf("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, iStart, iEnd);
  }
#endif
  માટે(; iStart<iEnd; iStart++, pOp++){
    જો( pOp->p1!=iTabCur ) ચાલુ;
    જો( pOp->opcode==OP_Column ){
#ifdef SQLITE_DEBUG
      જો( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE OP_Column to OP_Copy at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }નહીંતર જો( pOp->opcode==OP_Rowid ){
#ifdef SQLITE_DEBUG
      જો( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE OP_Rowid to OP_Sequence at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      જો( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#endif
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#જો !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
સ્થિર શૂન્ય whereTraceIndexInfoInputs(
  sqlite3_index_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  પૂર્ણાંક i;
  જો( (sqlite3WhereTrace & 0x10)==0 ) પાછા;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  માટે(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].iTermOffset,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  માટે(i=0; i<p->nOrderBy; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->aOrderBy[i].iColumn,
       p->aOrderBy[i].desc);
  }
}
સ્થિર શૂન્ય whereTraceIndexInfoOutputs(
  sqlite3_index_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  પૂર્ણાંક i;
  જો( (sqlite3WhereTrace & 0x10)==0 ) પાછા;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  માટે(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argvIndex,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_અંક);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->orderByConsumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimatedRows);
}
#નહીંતર
#define whereTraceIndexInfoInputs(A,B)
#define whereTraceIndexInfoOutputs(A,B)
#endif

/*
** We know that pSrc is an operand of an outer join.  Return true if
** pTerm is a constraint that is compatible with that join.
**
** pTerm must be EP_OuterON if pSrc is the right operand of an
** outer join.  pTerm can be either EP_OuterON or EP_InnerON if pSrc
** is the left operand of a RIGHT join.
**
** See https://sqlite.org/forum/forumpost/206d99a16dd9212f
** for an example of a WHERE clause constraints that may not be used on
** the right table of a RIGHT JOIN because the constraint implies a
** not-NULL condition on the left table of the RIGHT JOIN.
*/
સ્થિર પૂર્ણાંક constraintCompatibleWithOuterJoin(
  અચલ WhereTerm *pTerm,       /* WHERE clause term to check */
  અચલ SrcItem *pSrc           /* Table we are trying to access */
){
  પ્રતિજ્ઞા( (pSrc->fg.jointype&(JT_ડાબું|JT_LTORJ|JT_જમણું))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું))==JT_ડાબું );
  testcase( (pSrc->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું))==JT_LTORJ );
  testcase( પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON) )
  testcase( પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_InnerON) );
  જો( !પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON|EP_InnerON)
   || pTerm->p_પદાવલિ->w.iJoin != pSrc->iCursor
  ){
    પાછા 0;
  }
  જો( (pSrc->fg.jointype & (JT_ડાબું|JT_જમણું))!=0
   && NEVER(પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_InnerON))
  ){
    પાછા 0;
  }
  પાછા 1;
}

#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
/*
** Return true if column iCol of table pTab seem like it might be a
** good column to use as part of a query-time index.
**
** Current algorithm (subject to improvement!):
**
**   1.   If iCol is already the left-most column of some other index,
**        then return false.
**
**   2.   If iCol is part of an existing index that has an aiRowLogEst of
**        more than 20, then return false.
**
**   3.   If no disqualifying conditions above are found, return true.
**
** 2025-01-03: I experimented with a new rule that returns false if the
** the datatype of the column is "BOOLEAN". This did not improve
** performance on any queries at hand, but it did burn CPU cycles, so the
** idea was not committed.
*/
સ્થિર SQLITE_NOINLINE પૂર્ણાંક columnIsGoodIndexCandidate(
  અચલ Table *pTab,
  પૂર્ણાંક iCol
){
  અચલ Index *pIdx;
  માટે(pIdx = pTab->pIndex; pIdx!=0; pIdx=pIdx->pNext){
    પૂર્ણાંક j;
    માટે(j=0; j<pIdx->n_ચાવી_Col; j++){
       જો( pIdx->aiColumn[j]==iCol ){
         જો( j==0 ) પાછા 0;
         જો( pIdx->hasStat1 && pIdx->aiRowLogEst[j+1]>20 ) પાછા 0;
         તોડો;
       }
    }
  }
  પાછા 1;
}
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */



#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
સ્થિર પૂર્ણાંક termCanDriveIndex(
  અચલ WhereTerm *pTerm,        /* WHERE clause term to check */
  અચલ SrcItem *pSrc,           /* Table we are trying to access */
  અચલ Bitmask notReady         /* Tables in outer loops of the join */
){
  અક્ષર aff;
  પૂર્ણાંક ડાબું_Col;
  
  જો( pTerm->ડાબું_Cursor!=pSrc->iCursor ) પાછા 0;
  જો( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) પાછા 0;
  પ્રતિજ્ઞા( (pSrc->fg.jointype & JT_જમણું)==0 );
  જો( (pSrc->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું))!=0
   && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
  ){
    પાછા 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  જો( (pTerm->prereq_જમણું & notReady)!=0 ) પાછા 0;
  પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
  ડાબું_Col = pTerm->u.x.ડાબું_Column;
  જો( ડાબું_Col<0 ) પાછા 0;
  aff = pSrc->pSTab->aCol[ડાબું_Col].affinity;
  જો( !sqlite3IndexAffinityOk(pTerm->p_પદાવલિ, aff) ) પાછા 0;
  testcase( pTerm->p_પદાવલિ->op==TK_IS );
  પાછા columnIsGoodIndexCandidate(pSrc->pSTab, ડાબું_Col);
}
#endif


#ifndef SQLITE_OMIT_AUTOMATIC_INDEX

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Argument pIdx represents an automatic index that the current statement
** will create and populate. Add an OP_Explain with text of the form:
**
**     CREATE AUTOMATIC INDEX ON <table>(<cols>) [WHERE <expr>]
**
** This is only required if sqlite3_stmt_scanstatus() is enabled, to
** associate an SQLITE_SCANSTAT_NCYCLE and SQLITE_SCANSTAT_NLOOP
** values with. In order to avoid breaking legacy code and test cases,
** the OP_Explain is not added if this is an EXPLAIN QUERY PLAN command.
*/
સ્થિર શૂન્ય explainAutomaticIndex(
  Parse *pParse,
  Index *pIdx,                    /* Automatic index to explain */
  પૂર્ણાંક bPartial,                   /* True if pIdx is a partial index */
  પૂર્ણાંક *pAddrExplain               /* OUT: Address of OP_Explain */
){
  જો( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    Table *pTab = pIdx->pTable;
    અચલ અક્ષર *zSep = "";
    અક્ષર *zText = 0;
    પૂર્ણાંક ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    પ્રતિજ્ઞા( pIdx->nColumn>1 );
    પ્રતિજ્ઞા( pIdx->aiColumn[pIdx->nColumn-1]==XN_ROWID || !HasRowid(pTab) );
    માટે(ii=0; ii<(pIdx->nColumn-1); ii++){
      અચલ અક્ષર *zName = 0;
      પૂર્ણાંક iCol = pIdx->aiColumn[ii];

      zName = pTab->aCol[iCol].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    zText = sqlite3_str_finish(pStr);
    જો( zText==0 ){
      sqlite3OomFault(pParse->db);
    }નહીંતર{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", zText, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(zText);
    }
  }
}
#નહીંતર
# define explainAutomaticIndex(a,b,c,d)
#endif

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય constructAutomaticIndex(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  અચલ Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  પૂર્ણાંક n_ચાવી_Col;                /* Number of columns in the constructed index */
  WhereTerm *pTerm;           /* A single term of the WHERE clause */
  WhereTerm *pWCEnd;          /* End of pWC->a[] */
  Index *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  પૂર્ણાંક addrInit;               /* Address of the initialization bypass jump */
  Table *pTable;              /* The table being indexed */
  પૂર્ણાંક addr_ટોચ;                /* Top of the index fill loop */
  પૂર્ણાંક regRecord;              /* Register holding an index record */
  પૂર્ણાંક n;                      /* Column counter */
  પૂર્ણાંક i;                      /* Loop counter */
  પૂર્ણાંક mxBitCol;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  અક્ષર *zNotUsed;             /* Extra space on the end of pIdx */
  Bitmask idxCols;            /* Bitmap of columns used for indexing */
  Bitmask extraCols;          /* Bitmap of additional columns */
  u8 sentWarning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  પદાવલિ *pPartial = 0;         /* Partial Index Expression */
  પૂર્ણાંક iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  SrcItem *pSrc;              /* The FROM clause term to get the next index */
  પૂર્ણાંક addrCounter = 0;        /* Address where integer counter is initialized */
  પૂર્ણાંક regBase;                /* Array of registers where record is assembled */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  પૂર્ણાંક addrExp = 0;            /* Address of OP_Explain */
#endif

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );
  addrInit = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_ચાવી_Col = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  pTable = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  માટે(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    પદાવલિ *p_પદાવલિ = pTerm->p_પદાવલિ;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    જો( (pTerm->wtFlags & TERM_VIRTUAL)==0
     && sqlite3_પદાવલિ_Is_Single_Table_Constraint(p_પદાવલિ, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_પદાવલિ_And(pParse, pPartial,
                                sqlite3_પદાવલિ_Dup(pParse->db, p_પદાવલિ, 0));
    }
    જો( termCanDriveIndex(pTerm, pSrc, notReady) ){
      પૂર્ણાંક iCol;
      Bitmask cMask;
      પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.ડાબું_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      જો( !sentWarning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", pTable->zName,
            pTable->aCol[iCol].zCnName);
        sentWarning = 1;
      }
      જો( (idxCols & cMask)==0 ){
        જો( whereLoopResize(pParse->db, pLoop, n_ચાવી_Col+1) ){
          જાઓ end_auto_index_create;
        }
        pLoop->aLTerm[n_ચાવી_Col++] = pTerm;
        idxCols |= cMask;
      }
    }
  }
  પ્રતિજ્ઞા( n_ચાવી_Col>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_ચાવી_Col;
  pLoop->wsFlags = WHERE_COLUMN_EQ | WHERE_IDX_ONLY | WHERE_INDEXED
                     | WHERE_AUTO_INDEX;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  જો( IsView(pTable) ){
    extraCols = ALLBITS & ~idxCols;
  }નહીંતર{
    extraCols = pSrc->colUsed & (~idxCols | MASKBIT(BMS-1));
  }
  જો( !HasRowid(pTable) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    માટે(i=0; i<pTable->nCol; i++){
      જો( (pTable->aCol[i].colFlags & COLFLAG_PRIMKEY)==0 ) ચાલુ;
      જો( i>=BMS-1 ){
        extraCols |= MASKBIT(BMS-1);
        તોડો;
      }
      જો( idxCols & MASKBIT(i) ) ચાલુ;
      extraCols |= MASKBIT(i);
    }
  }
  mxBitCol = MIN(BMS-1,pTable->nCol);
  testcase( pTable->nCol==BMS-1 );
  testcase( pTable->nCol==BMS-2 );
  માટે(i=0; i<mxBitCol; i++){
    જો( extraCols & MASKBIT(i) ) n_ચાવી_Col++;
  }
  જો( pSrc->colUsed & MASKBIT(BMS-1) ){
    n_ચાવી_Col += pTable->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  પ્રતિજ્ઞા( n_ચાવી_Col <= pTable->nCol + MAX(0, pTable->nCol - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3AllocateIndexObject(pParse->db, n_ચાવી_Col+HasRowid(pTable),
                                    0, &zNotUsed);
  જો( pIdx==0 ) જાઓ end_auto_index_create;
  pLoop->u.btree.pIndex = pIdx;
  pIdx->zName = "auto-index";
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  માટે(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    જો( termCanDriveIndex(pTerm, pSrc, notReady) ){
      પૂર્ણાંક iCol;
      Bitmask cMask;
      પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.ડાબું_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      જો( (idxCols & cMask)==0 ){
        પદાવલિ *pX = pTerm->p_પદાવલિ;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.x.ડાબું_Column;
        pColl = sqlite3_પદાવલિ_Compare_Coll_Seq(pParse, pX);
        પ્રતિજ્ઞા( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        જો( !sqlite3_Is_દ્વિઅંકી(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_દ્વિઅંકી;
        n++;
      }
    }
  }
  પ્રતિજ્ઞા( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  માટે(i=0; i<mxBitCol; i++){
    જો( extraCols & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_દ્વિઅંકી;
      n++;
    }
  }
  જો( pSrc->colUsed & MASKBIT(BMS-1) ){
    માટે(i=BMS-1; i<pTable->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_દ્વિઅંકી;
      n++;
    }
  }
  પ્રતિજ્ઞા( n==n_ચાવી_Col );
  જો( HasRowid(pTable) ){
    pIdx->aiColumn[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_દ્વિઅંકી;
  }

  /* Create the automatic index */
  explainAutomaticIndex(pParse, pIdx, pPartial!=0, &addrExp);
  પ્રતિજ્ઞા( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_ચાવી_Col+1);
  sqlite3_Vdbe_Set_P4_ચાવી_Info(pParse, pIdx);
  VdbeComment((v, "for %s", pTable->zName));
  જો( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  પ્રતિજ્ઞા( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  જો( pSrc->fg.viaCoroutine ){
    પૂર્ણાંક regYield;
    Subquery *pSubq;
    પ્રતિજ્ઞા( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    પ્રતિજ્ઞા( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3VdbeAddOp2(v, OP_Integer, 0, 0);
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_ટોચ =  sqlite3VdbeAddOp1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }નહીંતર{
    પ્રતિજ્ઞા( pLevel->addrHalt );
    addr_ટોચ = sqlite3VdbeAddOp2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  જો( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_પદાવલિ_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  regRecord = sqlite3GetTempReg(pParse);
  regBase = sqlite3_Generate_Index_ચાવી(
      pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0
  );
  જો( pLevel->regFilter ){
    પ્રતિજ્ઞા( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0,
                         regBase, pLoop->u.btree.nEq);
  }
  sqlite3VdbeScanStatusCounters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3VdbeAddOp2(v, OP_Idx_ઉમેરો, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  જો( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  જો( pSrc->fg.viaCoroutine ){
    પ્રતિજ્ઞા( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, regBase+n);
    testcase( pParse->db->mallocFailed );
    પ્રતિજ્ઞા( pLevel->iIdxCur>0 );
    translateColumnToCopy(pParse, addr_ટોચ, pLevel->iTabCur,
                          pSrc->u4.pSubq->regResult, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_ટોચ);
    pSrc->fg.viaCoroutine = 0;
    sqlite3VdbeJumpHere(v, addr_ટોચ);
  }નહીંતર{
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addr_ટોચ+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    જો( (pSrc->fg.jointype & JT_ડાબું)!=0 ){
      sqlite3VdbeJumpHere(v, addr_ટોચ);
    }
  }
  sqlite3ReleaseTempReg(pParse, regRecord);
 
  /* Jump here when skipping the initialization */
  sqlite3VdbeJumpHere(v, addrInit);
  sqlite3VdbeScanStatusRange(v, addrExp, addrExp, -1);

end_auto_index_create:
  sqlite3_પદાવલિ_Delete(pParse->db, pPartial);
}
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

/*
** Generate bytecode that will initialize a Bloom filter that is appropriate
** for pLevel.
**
** If there are inner loops within pLevel that have the WHERE_BLOOMFILTER
** flag set, initialize a Bloomfilter for them as well.
**
** When the Bloom filter is initialized, the WHERE_BLOOMFILTER flag is cleared
** from the loop, but the regFilter value is set to a register that implements
** the Bloom filter.  When regFilter is positive, the
** sqlite3WhereCodeOneLoopStart() will generate code to test the Bloom filter
** and skip the subsequence B-Tree seek if the Bloom filter indicates that
** no matching rows exist.
**
** This routine may only be called if it has previously been determined that
** the loop would benefit from a Bloom filter, and the WHERE_BLOOMFILTER bit
** is set.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  પૂર્ણાંક iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  પૂર્ણાંક addrOnce;                        /* Address of opening OP_Once */
  પૂર્ણાંક addr_ટોચ;                         /* Address of OP_Rewind */
  પૂર્ણાંક addrCont;                        /* Jump here to skip a row */
  અચલ WhereTerm *pTerm;              /* For looping over WHERE clause terms */
  અચલ WhereTerm *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  પૂર્ણાંક iCur;                            /* Cursor for table getting the filter */
  Indexed_પદાવલિ *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  Indexed_પદાવલિ *saved_p_Idx_Part_પદાવલિ;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_પદાવલિ = pParse->p_Idx_Part_પદાવલિ;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_પદાવલિ = 0;

  પ્રતિજ્ઞા( pLoop!=0 );
  પ્રતિજ્ઞા( v!=0 );
  પ્રતિજ્ઞા( pLoop->wsFlags & WHERE_BLOOMFILTER );
  પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  કરો{
    અચલ SrcList *pTabList;
    અચલ SrcItem *pItem;
    અચલ Table *pTab;
    u64 sz;
    પૂર્ણાંક iSrc;
    sqlite3WhereExplainBloomFilter(pParse, pWInfo, pLevel);
    addrCont = sqlite3VdbeMakeLabel(pParse);
    iCur = pLevel->iTabCur;
    pLevel->regFilter = ++pParse->nMem;

    /* The Bloom filter is a Blob held in a register.  Initialize it
    ** to zero-filled blob of at least 80K bits, but maybe more if the
    ** estimated size of the table is larger.  We could actually
    ** measure the size of the table at run-time using OP_Count with
    ** P3==1 and use that value to initialize the blob.  But that makes
    ** testing complicated.  By basing the blob size on the value in the
    ** sqlite_stat1 table, testing is much easier.
    */
    pTabList = pWInfo->pTabList;
    iSrc = pLevel->iFrom;
    pItem = &pTabList->a[iSrc];
    પ્રતિજ્ઞા( pItem!=0 );
    pTab = pItem->pSTab;
    પ્રતિજ્ઞા( pTab!=0 );
    sz = sqlite3LogEstToInt(pTab->nRowLogEst);
    જો( sz<10000 ){
      sz = 10000;
    }નહીંતર જો( sz>10000000 ){
      sz = 10000000;
    }
    પ્રતિજ્ઞા( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp2(v, OP_Blob, (પૂર્ણાંક)sz, pLevel->regFilter);

    addr_ટોચ = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.nTerm];
    માટે(pTerm=pWInfo->sWC.a; pTerm<pWCEnd; pTerm++){
      પદાવલિ *p_પદાવલિ = pTerm->p_પદાવલિ;
      જો( (pTerm->wtFlags & TERM_VIRTUAL)==0
       && sqlite3_પદાવલિ_Is_Single_Table_Constraint(p_પદાવલિ, pTabList, iSrc, 0)
      ){
        sqlite3_પદાવલિ_If_False(pParse, pTerm->p_પદાવલિ, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    જો( pLoop->wsFlags & WHERE_IPK ){
      પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, r1);
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, 1);
      sqlite3ReleaseTempReg(pParse, r1);
    }નહીંતર{
      Index *pIdx = pLoop->u.btree.pIndex;
      પૂર્ણાંક n = pLoop->u.btree.nEq;
      પૂર્ણાંક r1 = sqlite3GetTempRange(pParse, n);
      પૂર્ણાંક jj;
      પ્રતિજ્ઞા( pIdx!=0 );
      માટે(jj=0; jj<n; jj++){
        પ્રતિજ્ઞા( pIdx->pTable==pItem->pSTab );
        sqlite3_પદાવલિ_Code_Load_Index_Column(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, n);
      sqlite3ReleaseTempRange(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addr_ટોચ+1);
    VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr_ટોચ);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    જ્યારે( ++iLevel < pWInfo->nLevel ){
      અચલ SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      જો( pTabItem->fg.jointype & (JT_ડાબું|JT_LTORJ) ) ચાલુ;
      pLoop = pLevel->pWLoop;
      જો( NEVER(pLoop==0) ) ચાલુ;
      જો( pLoop->prereq & notReady ) ચાલુ;
      જો( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_COLUMN_IN))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        તોડો;
      }
    }
  }જ્યારે( iLevel < pWInfo->nLevel );
  sqlite3VdbeJumpHere(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_પદાવલિ = saved_p_Idx_Part_પદાવલિ;
}


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
સ્થિર WhereTerm *termFromWhereClause(WhereClause *pWC, પૂર્ણાંક iTerm){
  WhereClause *p;
  માટે(p=pWC; p; p=p->pOuter){
    જો( iTerm<p->nTerm ) પાછા &p->a[iTerm];
    iTerm -= p->nTerm;
  }
  પાછા 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
સ્થિર sqlite3_index_info *allocateIndexInfo(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  SrcItem *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  પૂર્ણાંક i, j;
  પૂર્ણાંક nTerm;
  Parse *pParse = pWInfo->pParse;
  સંરચના sqlite3_index_constraint *pIdxCons;
  સંરચના sqlite3_index_orderby *pIdxOrderBy;
  સંરચના sqlite3_index_constraint_usage *pUsage;
  સંરચના HiddenIndexInfo *pHidden;
  WhereTerm *pTerm;
  પૂર્ણાંક nOrderBy;
  sqlite3_index_info *pIdxInfo;
  u16 mNoOmit = 0;
  અચલ Table *pTab;
  પૂર્ણાંક eDistinct = 0;
  પદાવલિ_List *pOrderBy = pWInfo->pOrderBy;
  WhereClause *p;

  પ્રતિજ્ઞા( pSrc!=0 );
  pTab = pSrc->pSTab;
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  માટે(p=pWC, nTerm=0; p; p=p->pOuter){
    માટે(i=0, pTerm=p->a; i<p->nTerm; i++, pTerm++){
      pTerm->wtFlags &= ~TERM_OK;
      જો( pTerm->ડાબું_Cursor != pSrc->iCursor ) ચાલુ;
      જો( pTerm->prereq_જમણું & mUnusable ) ચાલુ;
      પ્રતિજ્ઞા( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
      testcase( pTerm->eOperator & WO_IN );
      testcase( pTerm->eOperator & WO_ISNULL );
      testcase( pTerm->eOperator & WO_IS );
      testcase( pTerm->eOperator & WO_ALL );
      જો( (pTerm->eOperator & ~(WO_EQUIV))==0 ) ચાલુ;
      જો( pTerm->wtFlags & TERM_VNULL ) ચાલુ;

      પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      પ્રતિજ્ઞા( pTerm->u.x.ડાબું_Column>=XN_ROWID );
      પ્રતિજ્ઞા( pTerm->u.x.ડાબું_Column<pTab->nCol );
      જો( (pSrc->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું))!=0
          && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
        ){
        ચાલુ;
      }
      nTerm++;
      pTerm->wtFlags |= TERM_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  nOrderBy = 0;
  જો( pOrderBy ){
    પૂર્ણાંક n = pOrderBy->n_પદાવલિ;
    માટે(i=0; i<n; i++){
      પદાવલિ *p_પદાવલિ = pOrderBy->a[i].p_પદાવલિ;
      પદાવલિ *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      જો( sqlite3_પદાવલિ_Is_Constant(0, p_પદાવલિ) ){
        ચાલુ;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      જો( pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) તોડો;

      /* First case - a direct column references without a COLLATE operator */
      જો( p_પદાવલિ->op==TK_COLUMN && p_પદાવલિ->iTable==pSrc->iCursor ){
        પ્રતિજ્ઞા( p_પદાવલિ->iColumn>=XN_ROWID && p_પદાવલિ->iColumn<pTab->nCol );
        ચાલુ;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      જો( p_પદાવલિ->op==TK_COLLATE
       && (pE2 = p_પદાવલિ->p_ડાબું)->op==TK_COLUMN
       && pE2->iTable==pSrc->iCursor
      ){
        અચલ અક્ષર *zColl;  /* The collating sequence name */
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
        પ્રતિજ્ઞા( p_પદાવલિ->u.zToken!=0 );
        પ્રતિજ્ઞા( pE2->iColumn>=XN_ROWID && pE2->iColumn<pTab->nCol );
        p_પદાવલિ->iColumn = pE2->iColumn;
        જો( pE2->iColumn<0 ) ચાલુ;  /* Collseq does not matter for rowid */
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        જો( zColl==0 ) zColl = sqlite3_Str_દ્વિઅંકી;
        જો( sqlite3_stricmp(p_પદાવલિ->u.zToken, zColl)==0 ) ચાલુ;
      }

      /* No matches cause a break out of the loop */
      તોડો;
    }
    જો( i==n ){
      પૂર્ણાંક bSortByGroup = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      nOrderBy = n;
      જો( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowidUsed ){
        eDistinct = 2 + bSortByGroup;
      }નહીંતર જો( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - bSortByGroup;
      }નહીંતર જો( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, કદ(*pIdxInfo)
                           + (કદ(*pIdxCons) + કદ(*pUsage))*nTerm
                           + કદ(*pIdxOrderBy)*nOrderBy
                           + SZ_HIDDENINDEXINFO(nTerm) );
  જો( pIdxInfo==0 ){
    sqlite3_Error_સંદેશ(pParse, "out of memory");
    પાછા 0;
  }
  pHidden = (સંરચના HiddenIndexInfo*)&pIdxInfo[1];
  pIdxCons = (સંરચના sqlite3_index_constraint*)&pHidden->aRhs[nTerm];
  pIdxOrderBy = (સંરચના sqlite3_index_orderby*)&pIdxCons[nTerm];
  pUsage = (સંરચના sqlite3_index_constraint_usage*)&pIdxOrderBy[nOrderBy];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->aOrderBy = pIdxOrderBy;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->colUsed = (sqlite3_int64)pSrc->colUsed;
  જો( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    Index *pPk = sqlite3_Primary_ચાવી_Index((Table*)pTab);
    પ્રતિજ્ઞા( pPk!=0 );
    માટે(i=0; i<pPk->n_ચાવી_Col; i++){
      પૂર્ણાંક iCol = pPk->aiColumn[i];
      પ્રતિજ્ઞા( iCol>=0 );
      જો( iCol>=BMS-1 ) iCol = BMS-1;
      pIdxInfo->colUsed |= MASKBIT(iCol);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->mIn = 0;
  માટે(p=pWC, i=j=0; p; p=p->pOuter){
    પૂર્ણાંક nLast = i+p->nTerm;;
    માટે(pTerm=p->a; i<nLast; i++, pTerm++){
      u16 op;
      જો( (pTerm->wtFlags & TERM_OK)==0 ) ચાલુ;
      pIdxCons[j].iColumn = pTerm->u.x.ડાબું_Column;
      pIdxCons[j].iTermOffset = i;
      op = pTerm->eOperator & WO_ALL;
      જો( op==WO_IN ){
        જો( (pTerm->wtFlags & TERM_SLICE)==0 ){
          pHidden->mIn |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      જો( op==WO_AUX ){
        pIdxCons[j].op = pTerm->eMatchOp;
      }નહીંતર જો( op & (WO_ISNULL|WO_IS) ){
        જો( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_ISNULL;
        }નહીંતર{
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_IS;
        }
      }નહીંતર{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        પ્રતિજ્ઞા( WO_EQ==SQLITE_INDEX_CONSTRAINT_EQ );
        પ્રતિજ્ઞા( WO_LT==SQLITE_INDEX_CONSTRAINT_LT );
        પ્રતિજ્ઞા( WO_LE==SQLITE_INDEX_CONSTRAINT_LE );
        પ્રતિજ્ઞા( WO_GT==SQLITE_INDEX_CONSTRAINT_GT );
        પ્રતિજ્ઞા( WO_GE==SQLITE_INDEX_CONSTRAINT_GE );
        પ્રતિજ્ઞા( pTerm->eOperator&(WO_IN|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        જો( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_પદાવલિ_Is_Vector(pTerm->p_પદાવલિ->p_જમણું)
          ){
          testcase( j!=i );
          જો( j<16 ) mNoOmit |= (1 << j);
          જો( op==WO_LT ) pIdxCons[j].op = WO_LE;
          જો( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  પ્રતિજ્ઞા( j==nTerm );
  pIdxInfo->nConstraint = j;
  માટે(i=j=0; i<nOrderBy; i++){
    પદાવલિ *p_પદાવલિ = pOrderBy->a[i].p_પદાવલિ;
    જો( sqlite3_પદાવલિ_Is_Constant(0, p_પદાવલિ) ) ચાલુ;
    પ્રતિજ્ઞા( p_પદાવલિ->op==TK_COLUMN
         || (p_પદાવલિ->op==TK_COLLATE && p_પદાવલિ->p_ડાબું->op==TK_COLUMN
              && p_પદાવલિ->iColumn==p_પદાવલિ->p_ડાબું->iColumn) );
    pIdxOrderBy[j].iColumn = p_પદાવલિ->iColumn;
    pIdxOrderBy[j].desc = pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC;
    j++;
  }
  pIdxInfo->nOrderBy = j;

  *pmNoOmit = mNoOmit;
  પાછા pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
સ્થિર શૂન્ય freeIdxStr(sqlite3_index_info *pIdxInfo){
  જો( pIdxInfo->needToFreeIdxStr ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->needToFreeIdxStr = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
સ્થિર શૂન્ય freeIndexInfo(sqlite3 *db, sqlite3_index_info *pIdxInfo){
  HiddenIndexInfo *pHidden;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( pIdxInfo!=0 );
  pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  પ્રતિજ્ઞા( pHidden->pParse!=0 );
  પ્રતિજ્ઞા( pHidden->pParse->db==db );
  માટે(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_કિંમત_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
    pHidden->aRhs[i] = 0;
  }
  freeIdxStr(pIdxInfo);
  sqlite3DbFree(db, pIdxInfo);
}

/*
** The table object reference passed as the second argument to this function
** must represent a virtual table. This function invokes the xBestIndex()
** method of the virtual table with the sqlite3_index_info object that
** comes in as the 3rd argument to this function.
**
** If an error occurs, pParse is populated with an error message and an
** appropriate error code is returned.  A return of SQLITE_CONSTRAINT from
** xBestIndex is not considered an error.  SQLITE_CONSTRAINT indicates that
** the current configuration of "unusable" flags in sqlite3_index_info can
** not result in a valid plan.
**
** Whether or not an error is returned, it is the responsibility of the
** caller to eventually free p->idxStr if p->needToFreeIdxStr indicates
** that this is required.
*/
સ્થિર પૂર્ણાંક vtabBestIndex(Parse *pParse, Table *pTab, sqlite3_index_info *p){
  પૂર્ણાંક rc;
  sqlite3_vtab *pVtab;

  પ્રતિજ્ઞા( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  whereTraceIndexInfoInputs(p, pTab);
  pParse->db->nSchemaLock++;
  rc = pVtab->pModule->xBestIndex(pVtab, p);
  pParse->db->nSchemaLock--;
  whereTraceIndexInfoOutputs(p, pTab);

  જો( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    જો( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }નહીંતર જો( !pVtab->z_Err_સંદેશ ){
      sqlite3_Error_સંદેશ(pParse, "%s", sqlite3ErrStr(rc));
    }નહીંતર{
      sqlite3_Error_સંદેશ(pParse, "%s", pVtab->z_Err_સંદેશ);
    }
  }
  જો( pTab->u.vtab.p->bAllSchemas ){
    sqlite3VtabUsesAllSchemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_સંદેશ);
  pVtab->z_Err_સંદેશ = 0;
  પાછા rc;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ifdef SQLITE_ENABLE_STAT4
/*
** Estimate the location of a particular key among all keys in an
** index.  Store the results in aStat as follows:
**
**    aStat[0]      Est. number of rows less than pRec
**    aStat[1]      Est. number of rows equal to pRec
**
** Return the index of the sample that is the smallest sample that
** is greater than or equal to pRec. Note that this index is not an index
** into the aSample[] array - it is an index into a virtual set of samples
** based on the contents of aSample[] and the number of fields in record
** pRec.
*/
સ્થિર પૂર્ણાંક where_ચાવી_Stats(
  Parse *pParse,              /* Database connection */
  Index *pIdx,                /* Index to consider domain of */
  UnpackedRecord *pRec,       /* Vector of values to consider */
  પૂર્ણાંક roundUp,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  IndexSample *aSample = pIdx->aSample;
  પૂર્ણાંક iCol;                   /* Index of required stats in anEq[] etc. */
  પૂર્ણાંક i;                      /* Index of first sample >= pRec */
  પૂર્ણાંક iSample;                /* Smallest sample larger than or equal to pRec */
  પૂર્ણાંક iMin = 0;               /* Smallest sample not yet tested */
  પૂર્ણાંક iTest;                  /* Next sample to test */
  પૂર્ણાંક res;                    /* Result of comparison operation */
  પૂર્ણાંક nField;                 /* Number of fields in pRec */
  tRowcnt iLower = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER( pParse );
#endif
  પ્રતિજ્ઞા( pRec!=0 );
  પ્રતિજ્ઞા( pIdx->nSample>0 );
  પ્રતિજ્ઞા( pRec->nField>0 );


  /* Do a binary search to find the first sample greater than or equal
  ** to pRec. If pRec contains a single field, the set of samples to search
  ** is simply the aSample[] array. If the samples in aSample[] contain more
  ** than one fields, all fields following the first are ignored.
  **
  ** If pRec contains N fields, where N is more than one, then as well as the
  ** samples in aSample[] (truncated to N fields), the search also has to
  ** consider prefixes of those samples. For example, if the set of samples
  ** in aSample is:
  **
  **     aSample[0] = (a, 5)
  **     aSample[1] = (a, 10)
  **     aSample[2] = (b, 5)
  **     aSample[3] = (c, 100)
  **     aSample[4] = (c, 105)
  **
  ** Then the search space should ideally be the samples above and the
  ** unique prefixes [a], [b] and [c]. But since that is hard to organize,
  ** the code actually searches this set:
  **
  **     0: (a)
  **     1: (a, 5)
  **     2: (a, 10)
  **     3: (a, 10)
  **     4: (b)
  **     5: (b, 5)
  **     6: (c)
  **     7: (c, 100)
  **     8: (c, 105)
  **     9: (c, 105)
  **
  ** For each sample in the aSample[] array, N samples are present in the
  ** effective sample array. In the above, samples 0 and 1 are based on
  ** sample aSample[0]. Samples 2 and 3 on aSample[1] etc.
  **
  ** Often, sample i of each block of N effective samples has (i+1) fields.
  ** Except, each sample may be extended to ensure that it is greater than or
  ** equal to the previous sample in the array. For example, in the above,
  ** sample 2 is the first sample of a block of N samples, so at first it
  ** appears that it should be 1 field in size. However, that would make it
  ** smaller than sample 1, so the binary search would not work. As a result,
  ** it is extended to two fields. The duplicates that this creates do not
  ** cause any problems.
  */
  જો( !HasRowid(pIdx->pTable) && Is_Primary_ચાવી_Index(pIdx) ){
    nField = pIdx->n_ચાવી_Col;
  }નહીંતર{
    nField = pIdx->nColumn;
  }
  nField = MIN(pRec->nField, nField);
  iCol = 0;
  iSample = pIdx->nSample * nField;
  કરો{
    પૂર્ણાંક iSamp;                    /* Index in aSample[] of test sample */
    પૂર્ણાંક n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / nField;
    જો( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      માટે(n=(iTest % nField) + 1; n<nField; n++){
        જો( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) તોડો;
      }
    }નહીંતર{
      n = iTest + 1;
    }

    pRec->nField = n;
    res = sqlite3VdbeRecordCompare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    જો( res<0 ){
      iLower = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }નહીંતર જો( res==0 && n<nField ){
      iLower = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }નહીંતર{
      iSample = iTest;
      iCol = n-1;
    }
  }જ્યારે( res && iMin<iSample );
  i = iSample / nField;

#ifdef SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  જો( pParse->db->mallocFailed==0 ){
    જો( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      પ્રતિજ્ઞા( i<pIdx->nSample );
      પ્રતિજ્ઞા( iCol==nField-1 );
      pRec->nField = nField;
      પ્રતિજ્ઞા( 0==sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }નહીંતર{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      પ્રતિજ્ઞા( i<=pIdx->nSample && i>=0 );
      pRec->nField = iCol+1;
      પ્રતિજ્ઞા( i==pIdx->nSample
           || sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      જો( iCol>0 ){
        pRec->nField = iCol;
        પ્રતિજ્ઞા( sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      જો( i>0 ){
        pRec->nField = nField;
        પ્રતિજ્ઞા( sqlite3VdbeRecordCompare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#endif /* ifdef SQLITE_DEBUG */

  જો( res==0 ){
    /* Record pRec is equal to sample i */
    પ્રતિજ્ઞા( iCol==nField-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }નહીંતર{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt iUpper, iGap;
    જો( i>=pIdx->nSample ){
      iUpper = pIdx->nRowEst0;
    }નહીંતર{
      iUpper = aSample[i].anLt[iCol];
    }

    જો( iLower>=iUpper ){
      iGap = 0;
    }નહીંતર{
      iGap = iUpper - iLower;
    }
    જો( roundUp ){
      iGap = (iGap*2)/3;
    }નહીંતર{
      iGap = iGap/3;
    }
    aStat[0] = iLower + iGap;
    aStat[1] = pIdx->aAvgEq[nField-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->nField = nField;
  પાછા i;
}
#endif /* SQLITE_ENABLE_STAT4 */

/*
** If it is not NULL, pTerm is a term that provides an upper or lower
** bound on a range scan. Without considering pTerm, it is estimated
** that the scan will visit nNew rows. This function returns the number
** estimated to be visited after taking pTerm into account.
**
** If the user explicitly specified a likelihood() value for this term,
** then the return value is the likelihood multiplied by the number of
** input rows. Otherwise, this function assumes that an "IS NOT NULL" term
** has a likelihood of 0.50, and any other term a likelihood of 0.25.
*/
સ્થિર LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew){
  LogEst nRet = nNew;
  જો( pTerm ){
    જો( pTerm->truthProb<=0 ){
      nRet += pTerm->truthProb;
    }નહીંતર જો( (pTerm->wtFlags & TERM_VNULL)==0 ){
      nRet -= 20;        પ્રતિજ્ઞા( 20==sqlite3LogEst(4) );
    }
  }
  પાછા nRet;
}


#ifdef SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
અક્ષર sqlite3IndexColumnAffinity(sqlite3 *db, Index *pIdx, પૂર્ણાંક iCol){
  પ્રતિજ્ઞા( iCol>=0 && iCol<pIdx->nColumn );
  જો( !pIdx->zColAff ){
    જો( sqlite3IndexAffinityStr(db, pIdx)==0 ) પાછા SQLITE_AFF_BLOB;
  }
  પ્રતિજ્ઞા( pIdx->zColAff[iCol]!=0 );
  પાછા pIdx->zColAff[iCol];
}
#endif


#ifdef SQLITE_ENABLE_STAT4
/*
** This function is called to estimate the number of rows visited by a
** range-scan on a skip-scan index. For example:
**
**   CREATE INDEX i1 ON t1(a, b, c);
**   SELECT * FROM t1 WHERE a=? AND c BETWEEN ? AND ?;
**
** Value pLoop->nOut is currently set to the estimated number of rows
** visited for scanning (a=? AND b=?). This function reduces that estimate
** by some factor to account for the (c BETWEEN ? AND ?) expression based
** on the stat4 data for the index. this scan will be performed multiple
** times (once for each (a,b) combination that matches a=?) is dealt with
** by the caller.
**
** It does this by scanning through all stat4 samples, comparing values
** extracted from pLower and pUpper with the corresponding column in each
** sample. If L and U are the number of samples found to be less than or
** equal to the values extracted from pLower and pUpper respectively, and
** N is the total number of samples, the pLoop->nOut value is adjusted
** as follows:
**
**   nOut = nOut * ( min(U - L, 1) / N )
**
** If pLower is NULL, or a value cannot be extracted from the term, L is
** set to zero. If pUpper is NULL, or a value cannot be extracted from it,
** U is set to N.
**
** Normally, this function sets *pbDone to 1 before returning. However,
** if no value can be extracted from either pLower or pUpper (and so the
** estimate of the number of rows delivered remains unchanged), *pbDone
** is left as is.
**
** If an error occurs, an SQLite error code is returned. Otherwise,
** SQLITE_OK.
*/
સ્થિર પૂર્ણાંક whereRangeSkipScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  પૂર્ણાંક *pbDone          /* Set to true if at least one expr. value extracted */
){
  Index *p = pLoop->u.btree.pIndex;
  પૂર્ણાંક nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  પૂર્ણાંક nLower = -1;
  પૂર્ણાંક nUpper = p->nSample+1;
  પૂર્ણાંક rc = SQLITE_OK;
  u8 aff = sqlite3IndexColumnAffinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_કિંમત *p1 = 0;          /* Value extracted from pLower */
  sqlite3_કિંમત *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_કિંમત *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  જો( pLower ){
    rc = sqlite3_Stat4_કિંમત_From_પદાવલિ(pParse, pLower->p_પદાવલિ->p_જમણું, aff, &p1);
    nLower = 0;
  }
  જો( pUpper && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_કિંમત_From_પદાવલિ(pParse, pUpper->p_પદાવલિ->p_જમણું, aff, &p2);
    nUpper = p2 ? 0 : p->nSample;
  }

  જો( p1 || p2 ){
    પૂર્ણાંક i;
    પૂર્ણાંક nDiff;
    માટે(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      જો( rc==SQLITE_OK && p1 ){
        પૂર્ણાંક res = sqlite3MemCompare(p1, pVal, pColl);
        જો( res>=0 ) nLower++;
      }
      જો( rc==SQLITE_OK && p2 ){
        પૂર્ણાંક res = sqlite3MemCompare(p2, pVal, pColl);
        જો( res>=0 ) nUpper++;
      }
    }
    nDiff = (nUpper - nLower);
    જો( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    જો( nDiff!=1 || pUpper==0 || pLower==0 ){
      પૂર્ણાંક nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->nOut -= nAdjust;
      *pbDone = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           nLower, nUpper, nAdjust*-1, pLoop->nOut));
    }

  }નહીંતર{
    પ્રતિજ્ઞા( *pbDone==0 );
  }

  sqlite3_કિંમત_Free(p1);
  sqlite3_કિંમત_Free(p2);
  sqlite3_કિંમત_Free(pVal);

  પાછા rc;
}
#endif /* SQLITE_ENABLE_STAT4 */

/*
** This function is used to estimate the number of rows that will be visited
** by scanning an index for a range of values. The range may have an upper
** bound, a lower bound, or both. The WHERE clause terms that set the upper
** and lower bounds are represented by pLower and pUpper respectively. For
** example, assuming that index p is on t1(a):
**
**   ... FROM t1 WHERE a > ? AND a < ? ...
**                    |_____|   |_____|
**                       |         |
**                     pLower    pUpper
**
** If either of the upper or lower bound is not present, then NULL is passed in
** place of the corresponding WhereTerm.
**
** The value in (pBuilder->pNew->u.btree.nEq) is the number of the index
** column subject to the range constraint. Or, equivalently, the number of
** equality constraints optimized by the proposed index scan. For example,
** assuming index p is on t1(a, b), and the SQL query is:
**
**   ... FROM t1 WHERE a = ? AND b > ? AND b < ? ...
**
** then nEq is set to 1 (as the range restricted column, b, is the second
** left-most column of the index). Or, if the query is:
**
**   ... FROM t1 WHERE a > ? AND a < ? ...
**
** then nEq is set to 0.
**
** When this function is called, *pnOut is set to the sqlite3LogEst() of the
** number of rows that the index scan is expected to visit without
** considering the range constraints. If nEq is 0, then *pnOut is the number of
** rows in the index. Assuming no error occurs, *pnOut is adjusted (reduced)
** to account for the range constraints pLower and pUpper.
**
** In the absence of sqlite_stat4 ANALYZE data, or if such data cannot be
** used, a single range inequality reduces the search space by a factor of 4.
** and a pair of constraints (x>? AND x<?) reduces the expected number of
** rows visited by a factor of 64.
*/
સ્થિર પૂર્ણાંક whereRangeScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક nOut = pLoop->nOut;
  LogEst nNew;

#ifdef SQLITE_ENABLE_STAT4
  Index *p = pLoop->u.btree.pIndex;
  પૂર્ણાંક nEq = pLoop->u.btree.nEq;

  જો( p->nSample>0 && ALWAYS(nEq<p->nSampleCol)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    જો( nEq==pBuilder->nRecValid ){
      UnpackedRecord *pRec = pBuilder->pRec;
      tRowcnt a[2];
      પૂર્ણાંક nBtm = pLoop->u.btree.nBtm;
      પૂર્ણાંક n_ટોચ = pLoop->u.btree.n_ટોચ;

      /* Variable iLower will be set to the estimate of the number of rows in
      ** the index that are less than the lower bound of the range query. The
      ** lower bound being the concatenation of $P and $L, where $P is the
      ** key-prefix formed by the nEq values matched against the nEq left-most
      ** columns of the index, and $L is the value in pLower.
      **
      ** Or, if pLower is NULL or $L cannot be extracted from it (because it
      ** is not a simple variable or literal value), the lower bound of the
      ** range is $P. Due to a quirk in the way whereKeyStats() works, even
      ** if $L is available, whereKeyStats() is called for both ($P) and
      ** ($P:$L) and the larger of the two returned values is used.
      **
      ** Similarly, iUpper is to be set to the estimate of the number of rows
      ** less than the upper bound of the range query. Where the upper bound
      ** is either ($P) or ($P:$U). Again, even if $U is available, both values
      ** of iUpper are requested of whereKeyStats() and the smaller used.
      **
      ** The number of rows between the two bounds is then just iUpper-iLower.
      */
      tRowcnt iLower;     /* Rows less than the lower bound */
      tRowcnt iUpper;     /* Rows less than the upper bound */
      પૂર્ણાંક iLwrIdx = -2;   /* aSample[] for the lower bound */
      પૂર્ણાંક iUprIdx = -1;   /* aSample[] for the upper bound */

      જો( pRec ){
        testcase( pRec->nField!=pBuilder->nRecValid );
        pRec->nField = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      જો( nEq==0 ){
        iLower = 0;
        iUpper = p->nRowEst0;
      }નહીંતર{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_ચાવી_Stats(pParse, p, pRec, 0, a);
        iLower = a[0];
        iUpper = a[0] + a[1];
      }

      પ્રતિજ્ઞા( pLower==0 || (pLower->eOperator & (WO_GT|WO_GE))!=0 );
      પ્રતિજ્ઞા( pUpper==0 || (pUpper->eOperator & (WO_LT|WO_LE))!=0 );
      પ્રતિજ્ઞા( p->aSortOrder!=0 );
      જો( p->aSortOrder[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(WhereTerm*, pLower, pUpper);
        SWAP(પૂર્ણાંક, nBtm, n_ટોચ);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      જો( pLower ){
        પૂર્ણાંક n;                    /* Values extracted from pExpr */
        પદાવલિ *p_પદાવલિ = pLower->p_પદાવલિ->p_જમણું;
        rc = sqlite3_Stat4_Probe_Set_કિંમત(pParse, p, &pRec, p_પદાવલિ, nBtm, nEq, &n);
        જો( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          જો( sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ)>n ) mask = (WO_LE|WO_LT);
          iLwrIdx = where_ચાવી_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((pLower->eOperator & mask) ? a[1] : 0);
          જો( iNew>iLower ) iLower = iNew;
          nOut--;
          pLower = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      જો( pUpper ){
        પૂર્ણાંક n;                    /* Values extracted from pExpr */
        પદાવલિ *p_પદાવલિ = pUpper->p_પદાવલિ->p_જમણું;
        rc = sqlite3_Stat4_Probe_Set_કિંમત(pParse, p, &pRec, p_પદાવલિ, n_ટોચ, nEq, &n);
        જો( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          જો( sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ)>n ) mask = (WO_LE|WO_LT);
          iUprIdx = where_ચાવી_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((pUpper->eOperator & mask) ? a[1] : 0);
          જો( iNew<iUpper ) iUpper = iNew;
          nOut--;
          pUpper = 0;
        }
      }

      pBuilder->pRec = pRec;
      જો( rc==SQLITE_OK ){
        જો( iUpper>iLower ){
          nNew = sqlite3LogEst(iUpper - iLower);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          જો( iLwrIdx==iUprIdx ){ nNew -= 20; }
          પ્રતિજ્ઞા( 20==sqlite3LogEst(4) );
        }નહીંતર{
          nNew = 10;        પ્રતિજ્ઞા( 10==sqlite3LogEst(2) );
        }
        જો( nNew<nOut ){
          nOut = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)iLower, (u32)iUpper, nOut));
      }
    }નહીંતર{
      પૂર્ણાંક bDone = 0;
      rc = whereRangeSkipScanEst(pParse, pLower, pUpper, pLoop, &bDone);
      જો( bDone ) પાછા rc;
    }
  }
#નહીંતર
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pBuilder);
  પ્રતિજ્ઞા( pLower || pUpper );
#endif
  પ્રતિજ્ઞા( pUpper==0 || (pUpper->wtFlags & TERM_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  જો( pLower && pLower->truthProb>0 && pUpper && pUpper->truthProb>0 ){
    nNew -= 20;
  }

  nOut -= (pLower!=0) + (pUpper!=0);
  જો( nNew<10 ) nNew = 10;
  જો( nNew<nOut ) nOut = nNew;
#જો defined(WHERETRACE_ENABLED)
  જો( pLoop->nOut>nOut ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->nOut, nOut));
  }
#endif
  pLoop->nOut = (LogEst)nOut;
  પાછા rc;
}

#ifdef SQLITE_ENABLE_STAT4
/*
** Estimate the number of rows that will be returned based on
** an equality constraint x=VALUE and where that VALUE occurs in
** the histogram data.  This only works when x is the left-most
** column of an index and sqlite_stat4 histogram data is available
** for that index.  When pExpr==NULL that means the constraint is
** "x IS NULL" instead of "x=VALUE".
**
** Write the estimated row count into *pnRow and return SQLITE_OK.
** If unable to make an estimate, leave *pnRow unchanged and return
** non-zero.
**
** This routine can fail if it is unable to load a collating sequence
** required for string comparison, or if unable to allocate memory
** for a UTF conversion required for comparison.  The error is stored
** in the pParse structure.
*/
સ્થિર પૂર્ણાંક whereEqualScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  પદાવલિ *p_પદાવલિ,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  પૂર્ણાંક nEq = pBuilder->pNew->u.btree.nEq;
  UnpackedRecord *pRec = pBuilder->pRec;
  પૂર્ણાંક rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  પૂર્ણાંક bOk;

  પ્રતિજ્ઞા( nEq>=1 );
  પ્રતિજ્ઞા( nEq<=p->nColumn );
  પ્રતિજ્ઞા( p->aSample!=0 );
  પ્રતિજ્ઞા( p->nSample>0 );
  પ્રતિજ્ઞા( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  જો( pBuilder->nRecValid<(nEq-1) ){
    પાછા SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  જો( nEq>=p->nColumn ){
    *pnRow = 1;
    પાછા SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_Set_કિંમત(pParse, p, &pRec, p_પદાવલિ, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  જો( rc!=SQLITE_OK ) પાછા rc;
  જો( bOk==0 ) પાછા SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_ચાવી_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (પૂર્ણાંક)a[1]));
  *pnRow = a[1];
 
  પાછા rc;
}
#endif /* SQLITE_ENABLE_STAT4 */

#ifdef SQLITE_ENABLE_STAT4
/*
** Estimate the number of rows that will be returned based on
** an IN constraint where the right-hand side of the IN operator
** is a list of values.  Example:
**
**        WHERE x IN (1,2,3,4)
**
** Write the estimated row count into *pnRow and return SQLITE_OK.
** If unable to make an estimate, leave *pnRow unchanged and return
** non-zero.
**
** This routine can fail if it is unable to load a collating sequence
** required for string comparison, or if unable to allocate memory
** for a UTF conversion required for comparison.  The error is stored
** in the pParse structure.
*/
સ્થિર પૂર્ણાંક whereInScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  પદાવલિ_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  i64 nRow0 = sqlite3LogEstToInt(p->aiRowLogEst[0]);
  પૂર્ણાંક nRecValid = pBuilder->nRecValid;
  પૂર્ણાંક rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  પૂર્ણાંક i;                  /* Loop counter */

  પ્રતિજ્ઞા( p->aSample!=0 );
  માટે(i=0; rc==SQLITE_OK && i<pList->n_પદાવલિ; i++){
    nEst = nRow0;
    rc = whereEqualScanEst(pParse, pBuilder, pList->a[i].p_પદાવલિ, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  જો( rc==SQLITE_OK ){
    જો( nRowEst > (tRowcnt)nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", nRowEst));
  }
  પ્રતિજ્ઞા( pBuilder->nRecValid==nRecValid );
  પાછા rc;
}
#endif /* SQLITE_ENABLE_STAT4 */


#જો defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
શૂન્ય sqlite3WhereTermPrint(WhereTerm *pTerm, પૂર્ણાંક iTerm){
  જો( pTerm==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", iTerm);
  }નહીંતર{
    અક્ષર zType[8];
    અક્ષર z_ડાબું[50];
    memcpy(zType, "....", 5);
    જો( pTerm->wtFlags & TERM_VIRTUAL ) zType[0] = 'V';
    જો( pTerm->eOperator & WO_EQUIV  ) zType[1] = 'E';
    જો( પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON) ) zType[2] = 'L';
    જો( pTerm->wtFlags & TERM_CODED  ) zType[3] = 'C';
    જો( pTerm->eOperator & WO_SINGLE ){
      પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(કદ(z_ડાબું),z_ડાબું,"left={%d:%d}",
                       pTerm->ડાબું_Cursor, pTerm->u.x.ડાબું_Column);
    }નહીંતર જો( (pTerm->eOperator & WO_OR)!=0 && pTerm->u.pOrInfo!=0 ){
      sqlite3_snprintf(કદ(z_ડાબું),z_ડાબું,"indexable=0x%llx",
                       pTerm->u.pOrInfo->indexable);
    }નહીંતર{
      sqlite3_snprintf(કદ(z_ડાબું),z_ડાબું,"left=%d", pTerm->ડાબું_Cursor);
    }
    iTerm = pTerm->iTerm = MAX(iTerm,pTerm->iTerm);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       iTerm, pTerm, zType, z_ડાબું, pTerm->eOperator, pTerm->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    જો( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        pTerm->truthProb, (u64)pTerm->prereqAll, (u64)pTerm->prereq_જમણું);
    }
    જો( (pTerm->eOperator & (WO_OR|WO_AND))==0 && pTerm->u.x.iField ){
      sqlite3DebugPrintf(" iField=%d", pTerm->u.x.iField);
    }
    જો( pTerm->iParent>=0 ){
      sqlite3DebugPrintf(" iParent=%d", pTerm->iParent);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_વૃક્ષ_View_પદાવલિ(0, pTerm->p_પદાવલિ, 0);
  }
}
શૂન્ય sqlite3ShowWhereTerm(WhereTerm *pTerm){
  sqlite3WhereTermPrint(pTerm, 0);
}
#endif

#ifdef WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
શૂન્ય sqlite3WhereClausePrint(WhereClause *pWC){
  પૂર્ણાંક i;
  માટે(i=0; i<pWC->nTerm; i++){
    sqlite3WhereTermPrint(&pWC->a[i], i);
  }
}
#endif

#ifdef WHERETRACE_ENABLED
/*
** Print a WhereLoop object for debugging purposes
**
** Format example:
**
**     .--- Position in WHERE clause           rSetup, rRun, nOut ---.
**     |                                                             |
**     |  .--- selfMask                       nTerm ------.          |
**     |  |                                               |          |
**     |  |   .-- prereq    Idx          wsFlags----.     |          |
**     |  |   |             Name                    |     |          |
**     |  |   |           __|__        nEq ---.  ___|__   |        __|__
**     | / \ / \         /     \              | /      \ / \      /     \
**     1.002.001         t2.t2xy              2 f 010241 N 2 cost 0,56,31
*/
શૂન્ય sqlite3WhereLoopPrint(અચલ WhereLoop *p, અચલ WhereClause *pWC){
  WhereInfo *pWInfo;
  જો( pWC ){
    પૂર્ણાંક nb;
    SrcItem *pItem;
    Table *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    pItem = pWInfo->pTabList->a + p->iTab;
    pTab = pItem->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->maskSelf, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       pItem->zAlias ? pItem->zAlias : pTab->zName);
  }નહીંતર{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->maskSelf, p->prereq & 0xfff, p->cId, p->iTab);
  }
  જો( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    અચલ અક્ષર *zName;
    જો( p->u.btree.pIndex && (zName = p->u.btree.pIndex->zName)!=0 ){
      જો( strncmp(zName, "sqlite_autoindex_", 17)==0 ){
        પૂર્ણાંક i = sqlite3Strlen30(zName) - 1;
        જ્યારે( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }નહીંતર{
      sqlite3DebugPrintf("%20s","");
    }
  }નહીંતર{
    અક્ષર *z;
    જો( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_અંક, p->u.vtab.idxStr, p->u.vtab.omitMask);
    }નહીંતર{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_અંક, p->u.vtab.omitMask);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  જો( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }નહીંતર{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  જો( pWInfo && pWInfo->bStarUsed && p->rStarDelta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->rSetup, p->rRun, p->nOut, p->rStarDelta);
  }નહીંતર{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->rSetup, p->rRun, p->nOut);
  }
  જો( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    પૂર્ણાંક i;
    માટે(i=0; i<p->nLTerm; i++){
      sqlite3WhereTermPrint(p->aLTerm[i], i);
    }
  }
}
શૂન્ય sqlite3ShowWhereLoop(અચલ WhereLoop *p){
  જો( p ) sqlite3WhereLoopPrint(p, 0);
}
શૂન્ય sqlite3ShowWhereLoopList(અચલ WhereLoop *p){
  જ્યારે( p ){
    sqlite3ShowWhereLoop(p);
    p = p->pNextLoop;
  }
}
#endif

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
સ્થિર શૂન્ય whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = ArraySize(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
સ્થિર શૂન્ય whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  જો( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_INDEX) ){
    જો( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }નહીંતર જો( (p->wsFlags & WHERE_AUTO_INDEX)!=0 && p->u.btree.pIndex!=0 ){
      sqlite3DbFree(db, p->u.btree.pIndex->zColAff);
      sqlite3DbFreeNN(db, p->u.btree.pIndex);
      p->u.btree.pIndex = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
સ્થિર શૂન્ય whereLoopClear(sqlite3 *db, WhereLoop *p){
  જો( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = ArraySize(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
સ્થિર પૂર્ણાંક whereLoopResize(sqlite3 *db, WhereLoop *p, પૂર્ણાંક n){
  WhereTerm **paNew;
  જો( p->nLSlot>=n ) પાછા SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, કદ(p->aLTerm[0])*n);
  જો( paNew==0 ) પાછા SQLITE_NOMEM_BKPT;
  memcpy(paNew, p->aLTerm, કદ(p->aLTerm[0])*p->nLSlot);
  જો( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  પાછા SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
સ્થિર પૂર્ણાંક whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom){
  whereLoopClearUnion(db, pTo);
  જો( pFrom->nLTerm > pTo->nLSlot
   && whereLoopResize(db, pTo, pFrom->nLTerm)
  ){
    memset(pTo, 0, WHERE_LOOP_XFER_SZ);
    પાછા SQLITE_NOMEM_BKPT;
  }
  memcpy(pTo, pFrom, WHERE_LOOP_XFER_SZ);
  memcpy(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm*કદ(pTo->aLTerm[0]));
  જો( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }નહીંતર જો( (pFrom->wsFlags & WHERE_AUTO_INDEX)!=0 ){
    pFrom->u.btree.pIndex = 0;
  }
  પાછા SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
સ્થિર શૂન્ય whereLoopDelete(sqlite3 *db, WhereLoop *p){
  પ્રતિજ્ઞા( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
સ્થિર શૂન્ય whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  પ્રતિજ્ઞા( pWInfo!=0 );
  પ્રતિજ્ઞા( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  જ્યારે( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->pNextLoop;
    whereLoopDelete(db, p);
  }
  જ્યારે( pWInfo->pMemToFree ){
    WhereMemBlock *pNext = pWInfo->pMemToFree->pNext;
    sqlite3DbNNFreeNN(db, pWInfo->pMemToFree);
    pWInfo->pMemToFree = pNext;
  }
  sqlite3DbNNFreeNN(db, pWInfo);
}

/*
** Return TRUE if X is a proper subset of Y but is of equal or less cost.
** In other words, return true if all constraints of X are also part of Y
** and Y has additional constraints that might speed the search that X lacks
** but the cost of running X is not more than the cost of running Y.
**
** In other words, return true if the cost relationship between X and Y
** is inverted and needs to be adjusted.
**
** Case 1:
**
**   (1a)  X and Y use the same index.
**   (1b)  X has fewer == terms than Y
**   (1c)  Neither X nor Y use skip-scan
**   (1d)  X does not have a a greater cost than Y
**
** Case 2:
**
**   (2a)  X has the same or lower cost, or returns the same or fewer rows,
**         than Y.
**   (2b)  X uses fewer WHERE clause terms than Y
**   (2c)  Every WHERE clause term used by X is also used by Y
**   (2d)  X skips at least as many columns as Y
**   (2e)  If X is a covering index, than Y is too
*/
સ્થિર પૂર્ણાંક whereLoopCheaperProperSubset(
  અચલ WhereLoop *pX,       /* First WhereLoop to compare */
  અચલ WhereLoop *pY        /* Compare against this WhereLoop */
){
  પૂર્ણાંક i, j;
  જો( pX->rRun>pY->rRun && pX->nOut>pY->nOut ) પાછા 0; /* (1d) and (2a) */
  પ્રતિજ્ઞા( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  પ્રતિજ્ઞા( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  જો( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.pIndex==pY->u.btree.pIndex             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    પાછા 1;  /* Case 1 is true */
  }
  જો( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    પાછા 0;                                            /* (2b) */
  }
  જો( pY->nSkip > pX->nSkip ) પાછા 0;                  /* (2d) */
  માટે(i=pX->nLTerm-1; i>=0; i--){
    જો( pX->aLTerm[i]==0 ) ચાલુ;
    માટે(j=pY->nLTerm-1; j>=0; j--){
      જો( pY->aLTerm[j]==pX->aLTerm[i] ) તોડો;
    }
    જો( j<0 ) પાછા 0;                                  /* (2c) */
  }
  જો( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    પાછા 0;                                            /* (2e) */
  }
  પાછા 1;  /* Case 2 is true */
}

/*
** Try to adjust the cost and number of output rows of WhereLoop pTemplate
** upwards or downwards so that:
**
**   (1) pTemplate costs less than any other WhereLoops that are a proper
**       subset of pTemplate
**
**   (2) pTemplate costs more than any other WhereLoops for which pTemplate
**       is a proper subset.
**
** To say "WhereLoop X is a proper subset of Y" means that X uses fewer
** WHERE clause terms than Y and that every WHERE clause term used by X is
** also used by Y.
*/
સ્થિર શૂન્ય whereLoopAdjustCost(અચલ WhereLoop *p, WhereLoop *pTemplate){
  જો( (pTemplate->wsFlags & WHERE_INDEXED)==0 ) પાછા;
  માટે(; p; p=p->pNextLoop){
    જો( p->iTab!=pTemplate->iTab ) ચાલુ;
    જો( (p->wsFlags & WHERE_INDEXED)==0 ) ચાલુ;
    જો( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut,
                       MIN(p->rRun, pTemplate->rRun),
                       MIN(p->nOut - 1, pTemplate->nOut)));
      pTemplate->rRun = MIN(p->rRun, pTemplate->rRun);
      pTemplate->nOut = MIN(p->nOut - 1, pTemplate->nOut);
    }નહીંતર જો( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut,
                       MAX(p->rRun, pTemplate->rRun),
                       MAX(p->nOut + 1, pTemplate->nOut)));
      pTemplate->rRun = MAX(p->rRun, pTemplate->rRun);
      pTemplate->nOut = MAX(p->nOut + 1, pTemplate->nOut);
    }
  }
}

/*
** Search the list of WhereLoops in *ppPrev looking for one that can be
** replaced by pTemplate.
**
** Return NULL if pTemplate does not belong on the WhereLoop list.
** In other words if pTemplate ought to be dropped from further consideration.
**
** If pX is a WhereLoop that pTemplate can replace, then return the
** link that points to pX.
**
** If pTemplate cannot replace any existing element of the list but needs
** to be added to the list as a new entry, then return a pointer to the
** tail of the list.
*/
સ્થિર WhereLoop **whereLoopFindLesser(
  WhereLoop **ppPrev,
  અચલ WhereLoop *pTemplate
){
  WhereLoop *p;
  માટે(p=(*ppPrev); p; ppPrev=&p->pNextLoop, p=*ppPrev){
    જો( p->iTab!=pTemplate->iTab || p->iSortIdx!=pTemplate->iSortIdx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      ચાલુ;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    પ્રતિજ્ઞા( p->rSetup==0 || pTemplate->rSetup==0
                 || p->rSetup==pTemplate->rSetup );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    પ્રતિજ્ઞા( p->rSetup>=pTemplate->rSetup );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    જો( (p->wsFlags & WHERE_AUTO_INDEX)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_INDEXED)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      તોડો;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    જો( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->rSetup<=pTemplate->rSetup                  /* (2a) */
     && p->rRun<=pTemplate->rRun                      /* (2b) */
     && p->nOut<=pTemplate->nOut                      /* (2c) */
    ){
      પાછા 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    જો( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->rRun>=pTemplate->rRun                             /* (2a) */
     && p->nOut>=pTemplate->nOut                             /* (2b) */
    ){
      પ્રતિજ્ઞા( p->rSetup>=pTemplate->rSetup ); /* SETUP-INVARIANT above */
      તોડો;   /* Cause p to be overwritten by pTemplate */
    }
  }
  પાછા ppPrev;
}

/*
** Insert or replace a WhereLoop entry using the template supplied.
**
** An existing WhereLoop entry might be overwritten if the new template
** is better and has fewer dependencies.  Or the template will be ignored
** and no insert will occur if an existing WhereLoop is faster and has
** fewer dependencies than the template.  Otherwise a new WhereLoop is
** added based on the template.
**
** If pBuilder->pOrSet is not NULL then we care about only the
** prerequisites and rRun and nOut costs of the N best loops.  That
** information is gathered in the pBuilder->pOrSet object.  This special
** processing mode is used only for OR clause processing.
**
** When accumulating multiple loops (when pBuilder->pOrSet is NULL) we
** still might overwrite similar loops with the new template if the
** new template is better.  Loops may be overwritten if the following
** conditions are met:
**
**    (1)  They have the same iTab.
**    (2)  They have the same iSortIdx.
**    (3)  The template has same or fewer dependencies than the current loop
**    (4)  The template has the same or lower cost than the current loop
*/
સ્થિર પૂર્ણાંક where_Loop_ઉમેરો(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  પૂર્ણાંક rc;

  /* Stop the search once we hit the query planner search limit */
  જો( pBuilder->iPlanLimit==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    જો( pBuilder->pOrSet ) pBuilder->pOrSet->n = 0;
    પાછા SQLITE_DONE;
  }
  pBuilder->iPlanLimit--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  જો( pBuilder->pOrSet!=0 ){
    જો( pTemplate->nLTerm ){
#જો WHERETRACE_ENABLED
      u16 n = pBuilder->pOrSet->n;
      પૂર્ણાંક x =
#endif
      where_Or_ઉમેરો(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun,
                                    pTemplate->nOut);
#જો WHERETRACE_ENABLED /* 0x8 */
      જો( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#endif
    }
    પાછા SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  જો( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#જો WHERETRACE_ENABLED /* 0x8 */
    જો( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#endif
    પાછા SQLITE_OK; 
  }નહીંતર{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#જો WHERETRACE_ENABLED /* 0x8 */
  જો( sqlite3WhereTrace & 0x8 ){
    જો( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }નહીંતર{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#endif
  જો( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, કદ(WhereLoop));
    જો( p==0 ) પાછા SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->pNextLoop = 0;
  }નહીંતર{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **ppTail = &p->pNextLoop;
    WhereLoop *pToDel;
    જ્યારે( *ppTail ){
      ppTail = whereLoopFindLesser(ppTail, pTemplate);
      જો( ppTail==0 ) તોડો;
      pToDel = *ppTail;
      જો( pToDel==0 ) તોડો;
      *ppTail = pToDel->pNextLoop;
#જો WHERETRACE_ENABLED /* 0x8 */
      જો( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(pToDel, pBuilder->pWC);
      }
#endif
      whereLoopDelete(db, pToDel);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  જો( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    Index *pIndex = p->u.btree.pIndex;
    જો( pIndex && pIndex->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.pIndex = 0;
    }
  }
  પાછા rc;
}

/*
** Callback for estLikePatternLength().
**
** If this node is a string literal that is longer pWalker->sz, then set
** pWalker->sz to the byte length of that string literal.
**
** pWalker->eCode indicates how to count characters:
**
**    eCode==0     Count as a GLOB pattern
**    eCode==1     Count as a LIKE pattern
*/
સ્થિર પૂર્ણાંક પદાવલિ_ગાંઠ_Pattern_Length_Est(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_STRING ){
    પૂર્ણાંક sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_પદાવલિ->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    જો( pWalker->eCode ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }નહીંતર{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    જ્યારે( (c = *(z++))!=0 ){
      જો( c==c3 ){
        જો( *z ) z++;
        જ્યારે( *z && *z!=']' ) z++;
      }નહીંતર જો( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    જો( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  પાછા WRC_Continue;
}

/*
** Return the length of the longest string literal in the given
** expression.
**
** eCode indicates how to count characters:
**
**    eCode==0     Count as a GLOB pattern
**    eCode==1     Count as a LIKE pattern
*/
સ્થિર પૂર્ણાંક estLikePatternLength(પદાવલિ *p, u16 eCode){
  Walker w;
  w.u.sz = 0;
  w.eCode = eCode;
  w.x_પદાવલિ_Callback = પદાવલિ_ગાંઠ_Pattern_Length_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.u.sz;
}

/*
** Adjust the WhereLoop.nOut value downward to account for terms of the
** WHERE clause that reference the loop but which are not used by an
** index.
*
** For every WHERE clause term that is not used by the index
** and which has a truth probability assigned by one of the likelihood(),
** likely(), or unlikely() SQL functions, reduce the estimated number
** of output rows by the probability specified.
**
** TUNING:  For every WHERE clause term that is not used by the index
** and which does not have an assigned truth probability, heuristics
** described below are used to try to estimate the truth probability.
** TODO --> Perhaps this is something that could be improved by better
** table statistics.
**
** Heuristic 1:  Estimate the truth probability as 93.75%.  The 93.75%
** value corresponds to -1 in LogEst notation, so this means decrement
** the WhereLoop.nOut field for every such WHERE clause term.
**
** Heuristic 2:  If there exists one or more WHERE clause terms of the
** form "x==EXPR" and EXPR is not a constant 0 or 1, then make sure the
** final output row estimate is no greater than 1/4 of the total number
** of rows in the table.  In other words, assume that x==EXPR will filter
** out at least 3 out of 4 rows.  If EXPR is -1 or 0 or 1, then maybe the
** "x" column is boolean or else -1 or 0 or 1 is a common default value
** on the "x" column and so in that case only cap the output row estimate
** at 1/2 instead of 1/4.
**
** Heuristic 3:  If there is a LIKE or GLOB (or REGEXP or MATCH) operator
** with a large constant pattern, then reduce the size of the search
** space according to the length of the pattern, under the theory that
** longer patterns are less likely to match.  This heuristic was added
** to give better output-row count estimates when preparing queries for
** the Join-Order Benchmarks.  See forum thread 2026-01-30T09:57:54z
*/
સ્થિર શૂન્ય whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst nRow            /* Number of rows in the entire table */
){
  WhereTerm *pTerm, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->maskSelf);
  પૂર્ણાંક i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  જો( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->pOrderBy==0 ){
    પાછા;
  }

  પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_AUTO_INDEX)==0 );
  માટે(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
    પ્રતિજ્ઞા( pTerm!=0 );
    જો( (pTerm->prereqAll & notAllowed)!=0 ) ચાલુ;
    જો( (pTerm->prereqAll & pLoop->maskSelf)==0 ) ચાલુ;
    જો( (pTerm->wtFlags & TERM_VIRTUAL)!=0 ) ચાલુ;
    માટે(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      જો( pX==0 ) ચાલુ;
      જો( pX==pTerm ) તોડો;
      જો( pX->iParent>=0 && (&pWC->a[pX->iParent])==pTerm ) તોડો;
    }
    જો( j<0 ){
      sqlite3ProgressCheck(pWC->pWInfo->pParse);
      જો( pLoop->maskSelf==pTerm->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        જો( (pTerm->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_ડાબું|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      જો( pTerm->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->nOut += pTerm->truthProb;
      }નહીંતર{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        પદાવલિ *p_Op_પદાવલિ = pTerm->p_પદાવલિ;
        pLoop->nOut--;
        જો( (pTerm->eOperator&(WO_EQ|WO_IS))!=0
         && (pTerm->wtFlags & TERM_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          પદાવલિ *p_જમણું = p_Op_પદાવલિ->p_જમણું;
          Parse *pParse = pWC->pWInfo->pParse;
          પૂર્ણાંક k = 0;
          testcase( p_Op_પદાવલિ->op==TK_IS );
          જો( sqlite3_પદાવલિ_Is_Integer(p_જમણું, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }નહીંતર{
            k = 20;
          }
          જો( iReduce<k ){
            pTerm->wtFlags |= TERM_HEURTRUTH;
            iReduce = k;
          }
        }નહીંતર
        જો( પદાવલિ_Has_Property(p_Op_પદાવલિ, EP_InfixFunc) 
         && p_Op_પદાવલિ->op==TK_FUNCTION
        ){
          પૂર્ણાંક eOp;
          પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_Op_પદાવલિ) );
          પ્રતિજ્ઞા( p_Op_પદાવલિ->x.pList->n_પદાવલિ>=2 );
          eOp = sqlite3_પદાવલિ_Is_Like_Operator(p_Op_પદાવલિ);
          જો( ALWAYS(eOp>0) ){
            પૂર્ણાંક szPattern;
            પદાવલિ *pRHS = p_Op_પદાવલિ->x.pList->a[0].p_પદાવલિ;
            eOp = eOp==SQLITE_INDEX_CONSTRAINT_LIKE;
            szPattern = estLikePatternLength(pRHS, eOp);
            જો( szPattern>0 ){
              pLoop->nOut -= szPattern*2;
            }
          }
        }
      }
    }
  }
  જો( pLoop->nOut > nRow-iReduce ){
    pLoop->nOut = nRow - iReduce;
  }
}

/*
** Term pTerm is a vector range comparison operation. The first comparison
** in the vector can be optimized using column nEq of the index. This
** function returns the total number of vector elements that can be used
** as part of the range comparison.
**
** For example, if the query is:
**
**   WHERE a = ? AND (b, c, d) > (?, ?, ?)
**
** and the index:
**
**   CREATE INDEX ... ON (a, b, c, d, e)
**
** then this function would be invoked with nEq=1. The value returned in
** this case is 3.
*/
સ્થિર પૂર્ણાંક whereRangeVectorLen(
  Parse *pParse,       /* Parsing context */
  પૂર્ણાંક iCur,            /* Cursor open on pIdx */
  Index *pIdx,         /* The index to be used for a inequality constraint */
  પૂર્ણાંક nEq,             /* Number of prior equality constraints on same index */
  WhereTerm *pTerm     /* The vector inequality constraint */
){
  પૂર્ણાંક nCmp = sqlite3_પદાવલિ_Vector_Size(pTerm->p_પદાવલિ->p_ડાબું);
  પૂર્ણાંક i;

  nCmp = MIN(nCmp, (pIdx->nColumn - nEq));
  માટે(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    અક્ષર aff;                     /* Comparison affinity */
    અક્ષર idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    પદાવલિ *pLhs, *pRhs;

    પ્રતિજ્ઞા( પદાવલિ_Use_XList(pTerm->p_પદાવલિ->p_ડાબું) );
    pLhs = pTerm->p_પદાવલિ->p_ડાબું->x.pList->a[i].p_પદાવલિ;
    pRhs = pTerm->p_પદાવલિ->p_જમણું;
    જો( પદાવલિ_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_પદાવલિ;
    }નહીંતર{
      pRhs = pRhs->x.pList->a[i].p_પદાવલિ;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    જો( pLhs->op!=TK_COLUMN
     || pLhs->iTable!=iCur
     || pLhs->iColumn!=pIdx->aiColumn[i+nEq]
     || pIdx->aSortOrder[i+nEq]!=pIdx->aSortOrder[nEq]
    ){
      તોડો;
    }

    testcase( pLhs->iColumn==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_પદાવલિ_Affinity(pLhs));
    idxaff = sqlite3TableColumnAffinity(pIdx->pTable, pLhs->iColumn);
    જો( aff!=idxaff ) તોડો;

    જો( પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_Commuted) ) SWAP(પદાવલિ*, pRhs, pLhs);
    pColl = sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, pLhs, pRhs);
    જો( pColl==0 ) તોડો;
    જો( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) તોડો;
  }
  પાછા i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#ifdef SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#નહીંતર
# define ApplyCostMultiplier(C,T)
#endif

/*
** We have so far matched pBuilder->pNew->u.btree.nEq terms of the
** index pIndex. Try to match one more.
**
** When this function is called, pBuilder->pNew->nOut contains the
** number of rows expected to be visited by filtering using the nEq
** terms only. If it is modified, this value is restored before this
** function returns.
**
** If pProbe->idxType==SQLITE_IDXTYPE_IPK, that means pIndex is
** a fake index used for the INTEGER PRIMARY KEY.
*/
સ્થિર પૂર્ણાંક whereLoopAddBtreeIndex(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  SrcItem *pSrc,                  /* FROM clause term being analyzed */
  Index *pProbe,                  /* An index on pSrc */
  LogEst nInMul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  WhereTerm *pTerm;               /* A WhereTerm under consideration */
  પૂર્ણાંક opMask;                     /* Valid operators for constraints */
  WhereScan scan;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_ટોચ;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_nOut;              /* Original value of pNew->nOut */
  પૂર્ણાંક rc = SQLITE_OK;             /* Return code */
  LogEst rSize;                   /* Number of rows in the table */
  LogEst rLogSize;                /* Logarithm of table size */
  WhereTerm *p_ટોચ = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  પ્રતિજ્ઞા( db->mallocFailed==0 || pParse->nErr>0 );
  જો( pParse->nErr ){
    પાછા pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->pTable->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->rRun));

  પ્રતિજ્ઞા( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  પ્રતિજ્ઞા( (pNew->wsFlags & WHERE_ટોચ_LIMIT)==0 );
  જો( pNew->wsFlags & WHERE_BTM_LIMIT ){
    opMask = WO_LT|WO_LE;
  }નહીંતર{
    પ્રતિજ્ઞા( pNew->u.btree.nBtm==0 );
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  જો( pProbe->bUnordered ){
    opMask &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  પ્રતિજ્ઞા( pNew->u.btree.nEq<pProbe->nColumn );
  પ્રતિજ્ઞા( pNew->u.btree.nEq<pProbe->n_ચાવી_Col
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_ટોચ = pNew->u.btree.n_ટોચ;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_nOut = pNew->nOut;
  pTerm = whereScanInit(&scan, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        opMask, pProbe);
  pNew->rSetup = 0;
  rSize = pProbe->aiRowLogEst[0];
  rLogSize = estLog(rSize);
  માટે(; rc==SQLITE_OK && pTerm!=0; pTerm = whereScanNext(&scan)){
    u16 eOp = pTerm->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst nOutUnadjusted;        /* nOut before IN() and WHERE adjustments */
    પૂર્ણાંક nIn = 0;
#ifdef SQLITE_ENABLE_STAT4
    પૂર્ણાંક nRecValid = pBuilder->nRecValid;
#endif
    જો( (eOp==WO_ISNULL || (pTerm->wtFlags&TERM_VNULL)!=0)
     && indexColumnNotNull(pProbe, saved_nEq)
    ){
      ચાલુ; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    જો( pTerm->prereq_જમણું & pNew->maskSelf ) ચાલુ;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    જો( pTerm->wtFlags & TERM_LIKEOPT && pTerm->eOperator==WO_LT ) ચાલુ;

    જો( (pSrc->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું))!=0
     && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
    ){
      ચાલુ;
    }
    જો( IsUniqueIndex(pProbe) && saved_nEq==pProbe->n_ચાવી_Col-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }નહીંતર{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_INDEXED;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_ટોચ = saved_n_ટોચ;
    pNew->nLTerm = saved_nLTerm;
    જો( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       તોડો; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereq_જમણું) & ~pNew->maskSelf;

    પ્રતિજ્ઞા( nInMul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0
        || (pNew->wsFlags & WHERE_COLUMN_IN)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    જો( eOp & WO_IN ){
      પદાવલિ *p_પદાવલિ = pTerm->p_પદાવલિ;
      જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        પૂર્ણાંક i;
        પૂર્ણાંક bRedundant = 0;
        nIn = 46;  પ્રતિજ્ઞા( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        માટે(i=0; i<pNew->nLTerm-1; i++){
          જો( pNew->aLTerm[i] && pNew->aLTerm[i]->p_પદાવલિ==p_પદાવલિ ){
            nIn = 0;
            જો( pNew->aLTerm[i]->u.x.iField == pTerm->u.x.iField ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        જો( bRedundant ){
          pNew->nLTerm--;
          ચાલુ;
        }
      }નહીંતર જો( ALWAYS(p_પદાવલિ->x.pList && p_પદાવલિ->x.pList->n_પદાવલિ) ){
        /* "x IN (value, value, ...)" */
        nIn = sqlite3LogEst(p_પદાવલિ->x.pList->n_પદાવલિ);
      }
      જો( pProbe->hasStat1 && rLogSize>=10 ){
        LogEst M, logK, x;
        /* Let:
        **   N = the total number of rows in the table
        **   K = the number of entries on the RHS of the IN operator
        **   M = the number of rows in the table that match terms to the
        **       to the left in the same index.  If the IN operator is on
        **       the left-most index column, M==N.
        **
        ** Given the definitions above, it is better to omit the IN operator
        ** from the index lookup and instead do a scan of the M elements,
        ** testing each scanned row against the IN operator separately, if:
        **
        **        M*log(K) < K*log(N)
        **
        ** Our estimates for M, K, and N might be inaccurate, so we build in
        ** a safety margin of 2 (LogEst: 10) that favors using the IN operator
        ** with the index, as using an index has better worst-case behavior.
        ** If we do not have real sqlite_stat1 data, always prefer to use
        ** the index.  Do not bother with this optimization on very small
        ** tables (less than 2 rows) as it is pointless in that case.
        */
        M = pProbe->aiRowLogEst[saved_nEq];
        logK = estLog(nIn);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (nIn + rLogSize);
        જો( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, nIn, rLogSize, x));
        }નહીંતર જો( nInMul<2 && OptimizationEnabled(db, SQLITE_SeekScan) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, nIn, rLogSize, x, nInMul));
          pNew->wsFlags |= WHERE_IN_SEEKSCAN;
        }નહીંતર{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, nIn, rLogSize, x, nInMul));
          ચાલુ;
        }
      }
      pNew->wsFlags |= WHERE_COLUMN_IN;
    }નહીંતર જો( eOp & (WO_EQ|WO_IS) ){
      પૂર્ણાંક iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      પ્રતિજ્ઞા( saved_nEq==pNew->u.btree.nEq );
      જો( iCol==XN_ROWID
       || (iCol>=0 && nInMul==0 && saved_nEq==pProbe->n_ચાવી_Col-1)
      ){
        જો( iCol==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_ચાવી_Col==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }નહીંતર{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      જો( scan.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }નહીંતર જો( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }નહીંતર{
      પૂર્ણાંક nVecLen = whereRangeVectorLen(
          pParse, pSrc->iCursor, pProbe, saved_nEq, pTerm
      );
      જો( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_LIMIT;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = pTerm;
        p_ટોચ = 0;
        જો( pTerm->wtFlags & TERM_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_ટોચ = &pTerm[1];
          પ્રતિજ્ઞા( (p_ટોચ-(pTerm->pWC->a))<pTerm->pWC->nTerm );
          પ્રતિજ્ઞા( p_ટોચ->wtFlags & TERM_LIKEOPT );
          પ્રતિજ્ઞા( p_ટોચ->eOperator==WO_LT );
          જો( whereLoopResize(db, pNew, pNew->nLTerm+1) ) તોડો; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_ટોચ;
          pNew->wsFlags |= WHERE_ટોચ_LIMIT;
          pNew->u.btree.n_ટોચ = 1;
        }
      }નહીંતર{
        પ્રતિજ્ઞા( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_ટોચ_LIMIT;
        pNew->u.btree.n_ટોચ = nVecLen;
        p_ટોચ = pTerm;
        pBtm = (pNew->wsFlags & WHERE_BTM_LIMIT)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    પ્રતિજ્ઞા( pNew->nOut==saved_nOut );
    જો( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      whereRangeScanEst(pParse, pBuilder, pBtm, p_ટોચ, pNew);
    }નહીંતર{
      પૂર્ણાંક nEq = ++pNew->u.btree.nEq;
      પ્રતિજ્ઞા( eOp & (WO_ISNULL|WO_EQ|WO_IN|WO_IS) );

      પ્રતિજ્ઞા( pNew->nOut==saved_nOut );
      જો( pTerm->truthProb<=0 && pProbe->aiColumn[saved_nEq]>=0 ){
        પ્રતિજ્ઞા( (eOp & WO_IN) || nIn==0 );
        testcase( eOp & WO_IN );
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      }નહીંતર{
#ifdef SQLITE_ENABLE_STAT4
        tRowcnt nOut = 0;
        જો( nInMul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->nSampleCol)
         && ((eOp & WO_IN)==0 || પદાવલિ_Use_XList(pTerm->p_પદાવલિ))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          પદાવલિ *p_પદાવલિ = pTerm->p_પદાવલિ;
          જો( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = whereEqualScanEst(pParse, pBuilder, p_પદાવલિ->p_જમણું, &nOut);
          }નહીંતર{
            rc = whereInScanEst(pParse, pBuilder, p_પદાવલિ->x.pList, &nOut);
          }
          જો( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          જો( rc!=SQLITE_OK ) તોડો;          /* Jump out of the pTerm loop */
          જો( nOut ){
            pNew->nOut = sqlite3LogEst(nOut);
            જો( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->nOut+10 > pProbe->aiRowLogEst[0]
            ){
#જો WHERETRACE_ENABLED /* 0x01 */
              જો( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3WhereTermPrint(pTerm, 999);
              }
#endif
              pTerm->wtFlags |= TERM_HIGHTRUTH;
              જો( pTerm->wtFlags & TERM_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            જો( pNew->nOut>saved_nOut ) pNew->nOut = saved_nOut;
            pNew->nOut -= nIn;
          }
        }
        જો( nOut==0 )
#endif
        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          જો( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->nOut += 10;
          }
        }
      }
    }

    /* Set rCostIdx to the estimated cost of visiting selected rows in the
    ** index.  The estimate is the sum of two values:
    **   1.  The cost of doing one search-by-key to find the first matching
    **       entry
    **   2.  Stepping forward in the index pNew->nOut times to find all
    **       additional matching entries.
    */
    પ્રતિજ્ઞા( pSrc->pSTab->szTabRow>0 );
    જો( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      rCostIdx = pNew->nOut + 16;
    }નહીંતર{
      rCostIdx = pNew->nOut + 1 + (15*pProbe->szIdxRow)/pSrc->pSTab->szTabRow;
    }
    rCostIdx = sqlite3LogEstAdd(rLogSize, rCostIdx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->rRun = rCostIdx;
    જો( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    }
    ApplyCostMultiplier(pNew->rRun, pProbe->pTable->costMult);

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, rSize);
    જો( pSrc->fg.fromExists ) pNew->nOut = 0;
    rc = where_Loop_ઉમેરો(pBuilder, pNew);

    જો( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->nOut = saved_nOut;
    }નહીંતર{
      pNew->nOut = nOutUnadjusted;
    }

    જો( (pNew->wsFlags & WHERE_ટોચ_LIMIT)==0
     && pNew->u.btree.nEq<pProbe->nColumn
     && (pNew->u.btree.nEq<pProbe->n_ચાવી_Col ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      જો( pNew->u.btree.nEq>3 ){
        sqlite3ProgressCheck(pParse);
      }
      whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, nInMul+nIn);
    }
    pNew->nOut = saved_nOut;
#ifdef SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#endif
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_ટોચ = saved_n_ટોચ;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->nOut = saved_nOut;
  pNew->nLTerm = saved_nLTerm;

  /* Consider using a skip-scan if there are no WHERE clause constraints
  ** available for the left-most terms of the index, and if the average
  ** number of repeats in the left-most terms is at least 18.
  **
  ** The magic number 18 is selected on the basis that scanning 17 rows
  ** is almost always quicker than an index seek (even though if the index
  ** contains fewer than 2^17 rows we assume otherwise in other parts of
  ** the code). And, even if it is not, it should not be too much slower.
  ** On the other hand, the extra seeks could end up being significantly
  ** more expensive.  */
  પ્રતિજ્ઞા( 42==sqlite3LogEst(18) );
  જો( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_ચાવી_Col
   && saved_nEq==pNew->nLTerm
   && pProbe->noSkipScan==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_SkipScan)
   && pProbe->aiRowLogEst[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->aiRowLogEst[saved_nEq] - pProbe->aiRowLogEst[saved_nEq+1];
    pNew->nOut -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, nIter + nInMul);
    pNew->nOut = saved_nOut;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->pTable->zName, pProbe->zName, saved_nEq, rc));
  પાછા rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
સ્થિર પૂર્ણાંક indexMightHelpWithOrderBy(
  WhereLoopBuilder *pBuilder,
  Index *pIndex,
  પૂર્ણાંક iCursor
){
  પદાવલિ_List *pOB;
  પદાવલિ_List *a_Col_પદાવલિ;
  પૂર્ણાંક ii, jj;

  જો( pIndex->bUnordered ) પાછા 0;
  જો( (pOB = pBuilder->pWInfo->pOrderBy)==0 ) પાછા 0;
  માટે(ii=0; ii<pOB->n_પદાવલિ; ii++){
    પદાવલિ *p_પદાવલિ = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pOB->a[ii].p_પદાવલિ);
    જો( NEVER(p_પદાવલિ==0) ) ચાલુ;
    જો( (p_પદાવલિ->op==TK_COLUMN || p_પદાવલિ->op==TK_AGG_COLUMN) 
     && p_પદાવલિ->iTable==iCursor 
    ){
      જો( p_પદાવલિ->iColumn<0 ) પાછા 1;
      માટે(jj=0; jj<pIndex->n_ચાવી_Col; jj++){
        જો( p_પદાવલિ->iColumn==pIndex->aiColumn[jj] ) પાછા 1;
      }
    }નહીંતર જો( (a_Col_પદાવલિ = pIndex->a_Col_પદાવલિ)!=0 ){
      માટે(jj=0; jj<pIndex->n_ચાવી_Col; jj++){
        જો( pIndex->aiColumn[jj]!=XN_પદાવલિ ) ચાલુ;
        જો( sqlite3_પદાવલિ_Compare_Skip(p_પદાવલિ,a_Col_પદાવલિ->a[jj].p_પદાવલિ,iCursor)==0 ){
          પાછા 1;
        }
      }
    }
  }
  પાછા 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
સ્થિર પૂર્ણાંક whereUsablePartialIndex(
  પૂર્ણાંક iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  પદાવલિ *pWhere          /* The WHERE clause from the partial index */
){
  પૂર્ણાંક i;
  WhereTerm *pTerm;
  Parse *pParse;

  જો( jointype & JT_LTORJ ) પાછા 0;
  pParse = pWC->pWInfo->pParse;
  જ્યારે( pWhere->op==TK_AND ){
    જો( !whereUsablePartialIndex(iTab,jointype,pWC,pWhere->p_ડાબું) ) પાછા 0;
    pWhere = pWhere->p_જમણું;
  }
  માટે(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    પદાવલિ *p_પદાવલિ;
    p_પદાવલિ = pTerm->p_પદાવલિ;
    જો( (!પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) || p_પદાવલિ->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON))
     && sqlite3_પદાવલિ_Implies_પદાવલિ(pParse, p_પદાવલિ, pWhere, iTab)
     && !sqlite3_પદાવલિ_Implies_પદાવલિ(pParse, p_પદાવલિ, pWhere, -1)
     && (pTerm->wtFlags & TERM_VNULL)==0
    ){
      પાછા 1;
    }
  }
  પાછા 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Is_Covered_By_Index(
  અચલ પદાવલિ *p_પદાવલિ,
  અચલ Index *pIdx,
  પૂર્ણાંક iTabCur
){
  પૂર્ણાંક i;
  માટે(i=0; i<pIdx->nColumn; i++){
    જો( pIdx->aiColumn[i]==XN_પદાવલિ
     && sqlite3_પદાવલિ_Compare(0, p_પદાવલિ, pIdx->a_Col_પદાવલિ->a[i].p_પદાવલિ, iTabCur)==0
    ){
      પાછા 1;
    }
  }
  પાછા 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
પ્રકારવ્યાખ્યા સંરચના CoveringIndexCheck CoveringIndexCheck;
સંરચના CoveringIndexCheck {
  Index *pIdx;       /* The index */
  પૂર્ણાંક iTabCur;       /* Cursor number for the corresponding table */
  u8 b_પદાવલિ;          /* Uses an indexed expression */
  u8 bUnidx;         /* Uses an unindexed column not within an indexed expr */
};

/*
** Information passed in is pWalk->u.pCovIdxCk.  Call it pCk.
**
** If the Expr node references the table with cursor pCk->iTabCur, then
** make sure that column is covered by the index pCk->pIdx.  We know that
** all columns less than 63 (really BMS-1) are covered, so we don't need
** to check them.  But we do need to check any column at 63 or greater.
**
** If the index does not cover the column, then set pWalk->eCode to
** non-zero and return WRC_Abort to stop the search.
**
** If this node does not disprove that the index can be a covering index,
** then just return WRC_Continue, to continue the search.
**
** If pCk->pIdx contains indexed expressions and one of those expressions
** matches pExpr, then prune the search.
*/
સ્થિર પૂર્ણાંક whereIsCoveringIndexWalkCallback(Walker *pWalk, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક i;                    /* Loop counter */
  અચલ Index *pIdx;        /* The index of interest */
  અચલ i16 *aiColumn;      /* Columns contained in the index */
  u16 nColumn;              /* Number of columns in the index */
  CoveringIndexCheck *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  જો( (p_પદાવલિ->op==TK_COLUMN || p_પદાવલિ->op==TK_AGG_COLUMN) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    જો( p_પદાવલિ->iTable!=pCk->iTabCur ) પાછા WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    માટે(i=0; i<nColumn; i++){
      જો( aiColumn[i]==p_પદાવલિ->iColumn ) પાછા WRC_Continue;
    }
    pCk->bUnidx = 1;
    પાછા WRC_Abort;
  }નહીંતર જો( pIdx->b_Has_પદાવલિ
         && પદાવલિ_Is_Covered_By_Index(p_પદાવલિ, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_પદાવલિ = 1;
    પાછા WRC_Prune;
  }
  પાછા WRC_Continue;
}


/*
** pIdx is an index that covers all of the low-number columns used by
** pWInfo->pSelect (columns from 0 through 62) or an index that has
** expressions terms.  Hence, we cannot determine whether or not it is
** a covering index by using the colUsed bitmasks.  We have to do a search
** to see if the index is covering.  This routine does that search.
**
** The return value is one of these:
**
**      0                The index is definitely not a covering index
**
**      WHERE_IDX_ONLY   The index is definitely a covering index
**
**      WHERE_EXPRIDX    The index is likely a covering index, but it is
**                       difficult to determine precisely because of the
**                       expressions that are indexed.  Score it as a
**                       covering index, but still keep the main table open
**                       just in case we need it.
**
** This routine is an optimization.  It is always safe to return zero.
** But returning one of the other two values when zero should have been
** returned can lead to incorrect bytecode and assertion faults.
*/
સ્થિર SQLITE_NOINLINE u32 whereIsCoveringIndex(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  Index *pIdx,           /* Index that is being tested */
  પૂર્ણાંક iTabCur            /* Cursor for the table being indexed */
){
  પૂર્ણાંક i, rc;
  સંરચના CoveringIndexCheck ck;
  Walker w;
  જો( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    પાછા 0;
  }
  જો( pIdx->b_Has_પદાવલિ==0 ){
    માટે(i=0; i<pIdx->nColumn; i++){
      જો( pIdx->aiColumn[i]>=BMS-1 ) તોડો;
    }
    જો( i>=pIdx->nColumn ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      પાછા 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_પદાવલિ = 0;
  ck.bUnidx = 0;
  memset(&w, 0, કદ(w));
  w.x_પદાવલિ_Callback = whereIsCoveringIndexWalkCallback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  જો( ck.bUnidx ){
    rc = 0;
  }નહીંતર જો( ck.b_પદાવલિ ){
    rc = WHERE_EXPRIDX;
  }નહીંતર{
    rc = WHERE_IDX_ONLY;
  }
  પાછા rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
સ્થિર શૂન્ય where_Indexed_પદાવલિ_Cleanup(sqlite3 *db, શૂન્ય *pObject){
  Indexed_પદાવલિ **pp = (Indexed_પદાવલિ**)pObject;
  જ્યારે( *pp!=0 ){
    Indexed_પદાવલિ *p = *pp;
    *pp = p->pIENext;
    sqlite3_પદાવલિ_Delete(db, p->p_પદાવલિ);
    sqlite3DbFreeNN(db, p);
  }
}

/*
** This function is called for a partial index - one with a WHERE clause - in 
** two scenarios. In both cases, it determines whether or not the WHERE 
** clause on the index implies that a column of the table may be safely
** replaced by a constant expression. For example, in the following 
** SELECT:
**
**   CREATE INDEX i1 ON t1(b, c) WHERE a=<expr>;
**   SELECT a, b, c FROM t1 WHERE a=<expr> AND b=?;
**
** The "a" in the select-list may be replaced by <expr>, iff:
**
**    (a) <expr> is a constant expression, and
**    (b) The (a=<expr>) comparison uses the BINARY collation sequence, and
**    (c) Column "a" has an affinity other than NONE or BLOB.
**
** If argument pItem is NULL, then pMask must not be NULL. In this case this 
** function is being called as part of determining whether or not pIdx
** is a covering index. This function clears any bits in (*pMask) 
** corresponding to columns that may be replaced by constants as described
** above.
**
** Otherwise, if pItem is not NULL, then this function is being called
** as part of coding a loop that uses index pIdx. In this case, add entries
** to the Parse.pIdxPartExpr list for each column that can be replaced
** by a constant.
*/
સ્થિર શૂન્ય where_Part_Idx_પદાવલિ(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Partial index being processed */
  પદાવલિ *pPart,                    /* WHERE clause being processed */
  Bitmask *pMask,                 /* Mask to clear bits in */
  પૂર્ણાંક iIdxCur,                    /* Cursor number for index */
  SrcItem *pItem                  /* The FROM clause entry for the table */
){
  પ્રતિજ્ઞા( pItem==0 || (pItem->fg.jointype & JT_જમણું)==0 );
  પ્રતિજ્ઞા( (pItem==0 || pMask==0) && (pMask!=0 || pItem!=0) );

  જો( pPart->op==TK_AND ){
    where_Part_Idx_પદાવલિ(pParse, pIdx, pPart->p_જમણું, pMask, iIdxCur, pItem);
    pPart = pPart->p_ડાબું;
  }

  જો( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    પદાવલિ *p_ડાબું = pPart->p_ડાબું;
    પદાવલિ *p_જમણું = pPart->p_જમણું;
    Table *p_ડાબું_Tab;            /* Table of the left operand */
    Column *p_ડાબું_Col;           /* Column of the left operand */
    u8 aff_જમણું, aff_ડાબું;

    જો( p_ડાબું->op!=TK_COLUMN ) પાછા;
    જો( !sqlite3_પદાવલિ_Is_Constant(0, p_જમણું) ) પાછા;
    જો( !sqlite3_Is_દ્વિઅંકી(sqlite3_પદાવલિ_Compare_Coll_Seq(pParse, pPart)) ) પાછા;
    જો( p_ડાબું->iColumn<0 ) પાછા;
    p_ડાબું_Tab = pIdx->pTable;
    p_ડાબું_Col = &p_ડાબું_Tab->aCol[p_ડાબું->iColumn];
    aff_ડાબું = p_ડાબું_Col->affinity;
    પ્રતિજ્ઞા( SQLITE_AFF_BLOB<SQLITE_AFF_TEXT );
    જો( aff_ડાબું<SQLITE_AFF_TEXT ) પાછા;
    aff_જમણું = sqlite3_પદાવલિ_Affinity(p_જમણું);
    જો( aff_જમણું!=0 && aff_જમણું!=aff_ડાબું ) પાછા;
    જો( pItem ){
      sqlite3 *db = pParse->db;
      Indexed_પદાવલિ *p = (Indexed_પદાવલિ*)sqlite3DbMallocRaw(db, કદ(*p));
      જો( p ){
        પૂર્ણાંક bNullRow = (pItem->fg.jointype&(JT_ડાબું|JT_LTORJ))!=0;
        p_જમણું = sqlite3_પદાવલિ_Dup(db, p_જમણું, 0);
        જો( (p_ડાબું_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3ColumnType(p_ડાબું_Col, 0);
          પ્રતિજ્ઞા( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (પૂર્ણાંક)strlen(t.z);
          p->p_પદાવલિ = sqlite3_પદાવલિ_Alloc(db, TK_CAST, &t, 1);
          sqlite3_પદાવલિ_Attach_Subtrees(db, p->p_પદાવલિ, p_જમણું, 0);
        }નહીંતર{
          p->p_પદાવલિ = p_જમણું;
        }
        p->iDataCur = pItem->iCursor;
        p->iIdxCur = iIdxCur;
        p->iIdxCol = p_ડાબું->iColumn;
        p->bMaybeNullRow = bNullRow;
        p->pIENext = pParse->p_Idx_Part_પદાવલિ;
        p->aff = aff_ડાબું;
        pParse->p_Idx_Part_પદાવલિ = p;
        જો( p->pIENext==0 ){
          શૂન્ય *pArg = (શૂન્ય*)&pParse->p_Idx_Part_પદાવલિ;
          sqlite3ParserAddCleanup(pParse, where_Indexed_પદાવલિ_Cleanup, pArg);
        }
      }
    }નહીંતર જો( p_ડાબું->iColumn<(BMS-1) ){
      *pMask &= ~((Bitmask)1 << p_ડાબું->iColumn);
    }
  }
}


/*
** Add all WhereLoop objects for a single table of the join where the table
** is identified by pBuilder->pNew->iTab.  That table is guaranteed to be
** a b-tree table, not a virtual table.
**
** The costs (WhereLoop.rRun) of the b-tree loops added by this function
** are calculated as follows:
**
** For a full scan, assuming the table (or index) contains nRow rows:
**
**     cost = nRow * 3.0                    // full-table scan
**     cost = nRow * K                      // scan of covering index
**     cost = nRow * (K+3.0)                // scan of non-covering index
**
** where K is a value between 1.1 and 3.0 set based on the relative
** estimated average size of the index and table records.
**
** For an index scan, where nVisit is the number of index rows visited
** by the scan, and nSeek is the number of seek operations required on
** the index b-tree:
**
**     cost = nSeek * (log(nRow) + K * nVisit)          // covering index
**     cost = nSeek * (log(nRow) + (K+3.0) * nVisit)    // non-covering index
**
** Normally, nSeek is 1. nSeek values greater than 1 come about if the
** WHERE clause includes "x IN (....)" terms used in place of "x=?". Or when
** implicit "x IN (SELECT x FROM tbl)" terms are added for skip-scans.
**
** The estimated values (nRow, nVisit, nSeek) often contain a large amount
** of uncertainty.  For this reason, scoring is designed to pick plans that
** "do the least harm" if the estimates are inaccurate.  For example, a
** log(nRow) factor is omitted from a non-covering index scan in order to
** bias the scoring in favor of using an index, since the worst-case
** performance of using an index is far better than the worst-case performance
** of a full table scan.
*/
સ્થિર પૂર્ણાંક whereLoopAddBtree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  Index *pProbe;              /* An index we are evaluating */
  Index sPk;                  /* A fake index object for the primary key */
  LogEst aiRowEstPk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 aiColumnPk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  SrcItem *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  પૂર્ણાંક rc = SQLITE_OK;         /* Return code */
  પૂર્ણાંક iSortIdx;               /* Index number */
  પૂર્ણાંક b;                      /* A boolean value */
  LogEst rSize;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  Table *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  પ્રતિજ્ઞા( !IsVirtual(pSrc->pSTab) );

  જો( pSrc->fg.isIndexedBy ){
    પ્રતિજ્ઞા( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }નહીંતર જો( !HasRowid(pTab) ){
    જો( pSrc->fg.notIndexed ){
      pProbe = sqlite3_Primary_ચાવી_Index(pTab);
    }નહીંતર{
      pProbe = pTab->pIndex;
    }
  }નહીંતર{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    Index *pFirst;                  /* First of real indices on the table */
    memset(&sPk, 0, કદ(Index));
    sPk.n_ચાવી_Col = 1;
    sPk.nColumn = 1;
    sPk.aiColumn = &aiColumnPk;
    sPk.aiRowLogEst = aiRowEstPk;
    sPk.onError = OE_Replace;
    sPk.pTable = pTab;
    sPk.szIdxRow = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    aiRowEstPk[0] = pTab->nRowLogEst;
    aiRowEstPk[1] = 0;
    pFirst = pSrc->pSTab->pIndex;
    sPk.pNext = pFirst;
    pProbe = &sPk;
  }
  rSize = pTab->nRowLogEst;

#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
  /* Automatic indexes */
  જો( !pBuilder->pOrSet      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_જમણું_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_AutoIndex)!=0
   && !pSrc->fg.isIndexedBy  /* Has no INDEXED BY clause */
   && !pSrc->fg.notIndexed   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_જમણું)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst rLogSize;         /* Logarithm of the number of rows in the table */
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    rLogSize = estLog(rSize);
    માટે(pTerm=pWC->a; rc==SQLITE_OK && pTerm<pWCEnd; pTerm++){
      જો( pTerm->prereq_જમણું & pNew->maskSelf ) ચાલુ;
      જો( termCanDriveIndex(pTerm, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.pIndex = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = pTerm;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->rSetup = rLogSize + rSize;
        જો( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->rSetup += 28;
        }નહીંતર{
          pNew->rSetup -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        ApplyCostMultiplier(pNew->rSetup, pTab->costMult);
        જો( pNew->rSetup<0 ) pNew->rSetup = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->nOut = 43;  પ્રતિજ્ઞા( 43==sqlite3LogEst(20) );
        pNew->rRun = sqlite3LogEstAdd(rLogSize,pNew->nOut);
        pNew->wsFlags = WHERE_AUTO_INDEX;
        pNew->prereq = mPrereq | pTerm->prereq_જમણું;
        rc = where_Loop_ઉમેરો(pBuilder, pNew);
      }
    }
  }
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  માટે(iSortIdx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.isIndexedBy||pSrc->fg.notIndexed)?0:pProbe->pNext,
         iSortIdx++
  ){
    જો( pProbe->pPartIdxWhere!=0
     && !whereUsablePartialIndex(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      ચાલુ;  /* Partial index inappropriate for this query */
    }
    જો( pProbe->bNoQuery ) ચાલુ;
    rSize = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_ટોચ = 0;
    pNew->u.btree.nDistinctCol = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->iSortIdx = 0;
    pNew->rSetup = 0;
    pNew->prereq = mPrereq;
    pNew->nOut = rSize;
    pNew->u.btree.pIndex = pProbe;
    pNew->u.btree.pOrderBy = 0;
    b = indexMightHelpWithOrderBy(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    પ્રતિજ્ઞા( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    જો( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->iSortIdx = b ? iSortIdx : 0;
      /* TUNING: Cost of full table scan is 3.0*N.  The 3.0 factor is an
      ** extra cost designed to discourage the use of full table scans,
      ** since index lookups have better worst-case performance if our
      ** stat guesses are wrong.  Reduce the 3.0 penalty slightly
      ** (to 2.75) if we have valid STAT4 information for the table.
      ** At 2.75, a full table scan is preferred over using an index on
      ** a column with just two distinct values where each value has about
      ** an equal number of appearances.  Without STAT4 data, we still want
      ** to use an index in that case, since the constraint might be for
      ** the scarcer of the two values, and in that case an index lookup is
      ** better.
      */
#ifdef SQLITE_ENABLE_STAT4
      pNew->rRun = rSize + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#નહીંતર
      pNew->rRun = rSize + 16;
#endif
      ApplyCostMultiplier(pNew->rRun, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, rSize);
      જો( pSrc->fg.isSubquery ){
        જો( pSrc->fg.viaCoroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        જો( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.pOrderBy = pSrc->u4.pSubq->pSelect->pOrderBy;
        }
      }નહીંતર જો( pSrc->fg.fromExists ){
        pNew->nOut = 0;
      }
      rc = where_Loop_ઉમેરો(pBuilder, pNew);
      pNew->nOut = rSize;
      જો( rc ) તોડો;
    }નહીંતર{
      Bitmask m;
      જો( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
      }નહીંતર{
        m = pSrc->colUsed & pProbe->colNotIdxed;
        જો( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_પદાવલિ(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_INDEXED;
        જો( m==TOPBIT || (pProbe->b_Has_પદાવલિ && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = whereIsCoveringIndex(pWInfo, pProbe, pSrc->iCursor);
          જો( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            પ્રતિજ્ઞા( m!=0 );
          }નહીંતર{
            m = 0;
            pNew->wsFlags |= isCov;
            જો( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }નહીંતર{
              પ્રતિજ્ઞા( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }નહીંતર જો( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
        }
      }

      /* Full scan via index */
      જો( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.isIndexedBy
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->szIdxRow<pTab->szTabRow)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_CoverIdxScan)
          )
      ){
        pNew->iSortIdx = b ? iSortIdx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->rRun = rSize + 1 + (15*pProbe->szIdxRow)/pTab->szTabRow;
        જો( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = rSize + 16;  /* Base cost:  N*3 */
          પૂર્ણાંક ii;
          પૂર્ણાંક iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          માટે(ii=0; ii<pWC2->nTerm; ii++){
            WhereTerm *pTerm = &pWC2->a[ii];
            જો( !sqlite3_પદાવલિ_Covered_By_Index(pTerm->p_પદાવલિ, iCur, pProbe) ){
              તોડો;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            જો( pTerm->truthProb<=0 ){
              nLookup += pTerm->truthProb;
            }નહીંતર{
              nLookup--;
              જો( pTerm->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, nLookup);
        }
        ApplyCostMultiplier(pNew->rRun, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, rSize);
        જો( (pSrc->fg.jointype & JT_જમણું)!=0 && pProbe->a_Col_પદાવલિ ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }નહીંતર{
          જો( pSrc->fg.fromExists ) pNew->nOut = 0;
          rc = where_Loop_ઉમેરો(pBuilder, pNew);
        }
        pNew->nOut = rSize;
        જો( rc ) તોડો;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, 0);
    જો( pBuilder->bldFlags1==SQLITE_BLDF1_INDEXED ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#ifdef SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#endif
  }
  પાછા rc;
}

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
સ્થિર પૂર્ણાંક isLimitTerm(WhereTerm *pTerm){
  પ્રતિજ્ઞા( pTerm->eOperator==WO_AUX || pTerm->eMatchOp==0 );
  પાછા pTerm->eMatchOp>=SQLITE_INDEX_CONSTRAINT_LIMIT
      && pTerm->eMatchOp<=SQLITE_INDEX_CONSTRAINT_OFFSET;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
સ્થિર પૂર્ણાંક allConstraintsUsed(
  સંરચના sqlite3_index_constraint_usage *aUsage, 
  પૂર્ણાંક nCons
){
  પૂર્ણાંક ii;
  માટે(ii=0; ii<nCons; ii++){
    જો( aUsage[ii].argvIndex<=0 ) પાછા 0;
  }
  પાછા 1;
}

/*
** Argument pIdxInfo is already populated with all constraints that may
** be used by the virtual table identified by pBuilder->pNew->iTab. This
** function marks a subset of those constraints usable, invokes the
** xBestIndex method and adds the returned plan to pBuilder.
**
** A constraint is marked usable if:
**
**   * Argument mUsable indicates that its prerequisites are available, and
**
**   * It is not one of the operators specified in the mExclude mask passed
**     as the fourth argument (which in practice is either WO_IN or 0).
**
** Argument mPrereq is a mask of tables that must be scanned before the
** virtual table in question. These are added to the plans prerequisites
** before it is added to pBuilder.
**
** Output parameter *pbIn is set to true if the plan added to pBuilder
** uses one or more WO_IN terms, or false otherwise.
*/
સ્થિર પૂર્ણાંક whereLoopAddVirtualOne(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_index_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  પૂર્ણાંક *pbIn,                      /* OUT: True if plan uses an IN(...) op */
  પૂર્ણાંક *pbRetryLimit               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  સંરચના sqlite3_index_constraint *pIdxCons;
  સંરચના sqlite3_index_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  પૂર્ણાંક i;
  પૂર્ણાંક mxTerm;
  પૂર્ણાંક rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  પૂર્ણાંક nConstraint = pIdxInfo->nConstraint;

  પ્રતિજ્ઞા( (mUsable & mPrereq)==mPrereq );
  *pbIn = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(સંરચના sqlite3_index_constraint**)&pIdxInfo->aConstraint;
  માટે(i=0; i<nConstraint; i++, pIdxCons++){
    WhereTerm *pTerm = termFromWhereClause(pWC, pIdxCons->iTermOffset);
    pIdxCons->usable = 0;
    જો( (pTerm->prereq_જમણું & mUsable)==pTerm->prereq_જમણું
     && (pTerm->eOperator & mExclude)==0
     && (pbRetryLimit || !isLimitTerm(pTerm))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, કદ(pUsage[0])*nConstraint);
  પ્રતિજ્ઞા( pIdxInfo->needToFreeIdxStr==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_અંક = 0;
  pIdxInfo->orderByConsumed = 0;
  pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (દ્વિગુણ)2;
  pIdxInfo->estimatedRows = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->mHandleIn = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtabBestIndex(pParse, pSrc->pSTab, pIdxInfo);
  જો( rc ){
    જો( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      પાછા SQLITE_OK;
    }
    પાછા rc;
  }

  mxTerm = -1;
  પ્રતિજ્ઞા( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, કદ(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, કદ(pNew->u.vtab));
  pIdxCons = *(સંરચના sqlite3_index_constraint**)&pIdxInfo->aConstraint;
  માટે(i=0; i<nConstraint; i++, pIdxCons++){
    પૂર્ણાંક iTerm;
    જો( (iTerm = pUsage[i].argvIndex - 1)>=0 ){
      WhereTerm *pTerm;
      પૂર્ણાંક j = pIdxCons->iTermOffset;
      જો( iTerm>=nConstraint
       || j<0
       || (pTerm = termFromWhereClause(pWC, j))==0
       || pNew->aLTerm[iTerm]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_સંદેશ(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        પાછા SQLITE_ERROR;
      }
      testcase( iTerm==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->nTerm-1 );
      pNew->prereq |= pTerm->prereq_જમણું;
      પ્રતિજ્ઞા( iTerm<pNew->nLSlot );
      pNew->aLTerm[iTerm] = pTerm;
      જો( iTerm>mxTerm ) mxTerm = iTerm;
      testcase( iTerm==15 );
      testcase( iTerm==16 );
      જો( pUsage[i].omit ){
        જો( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=iTerm );
          pNew->u.vtab.omitMask |= 1<<iTerm;
        }નહીંતર{
          testcase( i!=iTerm );
        }
        જો( pTerm->eMatchOp==SQLITE_INDEX_CONSTRAINT_OFFSET ){
          pNew->u.vtab.bOmitOffset = 1;
        }
      }
      જો( SMASKBIT32(i) & pHidden->mHandleIn ){
        pNew->u.vtab.mHandleIn |= MASKBIT32(iTerm);
      }નહીંતર જો( (pTerm->eOperator & WO_IN)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->orderByConsumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_INDEX_SCAN_UNIQUE;
        *pbIn = 1; પ્રતિજ્ઞા( (mExclude & WO_IN)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      પ્રતિજ્ઞા( pbRetryLimit || !isLimitTerm(pTerm) );
      પ્રતિજ્ઞા( !isLimitTerm(pTerm) || i>=nConstraint-2 );
      પ્રતિજ્ઞા( !isLimitTerm(pTerm) || i==nConstraint-1 || isLimitTerm(pTerm+1) );

      જો( isLimitTerm(pTerm) && (*pbIn || !allConstraintsUsed(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pbRetryLimit = 1;
        પાછા SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mxTerm+1;
  માટે(i=0; i<=mxTerm; i++){
    જો( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_સંદેશ(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      પાછા SQLITE_ERROR;
    }
  }
  પ્રતિજ્ઞા( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_અંક = pIdxInfo->idx_અંક;
  pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
  pIdxInfo->needToFreeIdxStr = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ?
      pIdxInfo->nOrderBy : 0);
  pNew->u.vtab.b_Idx_અંક_Hex = (pIdxInfo->idxFlags&SQLITE_INDEX_SCAN_HEX)!=0;
  pNew->rSetup = 0;
  pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  જો( pIdxInfo->idxFlags & SQLITE_INDEX_SCAN_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }નહીંતર{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_ઉમેરો(pBuilder, pNew);
  જો( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pbIn, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  પાછા rc;
}

/*
** Return the collating sequence for a constraint passed into xBestIndex.
**
** pIdxInfo must be an sqlite3_index_info structure passed into xBestIndex.
** This routine depends on there being a HiddenIndexInfo structure immediately
** following the sqlite3_index_info structure.
**
** Return a pointer to the collation name:
**
**    1. If there is an explicit COLLATE operator on the constraint, return it.
**
**    2. Else, if the column has an alternative collation, return that.
**
**    3. Otherwise, return "BINARY".
*/
અચલ અક્ષર *sqlite3_vtab_collation(sqlite3_index_info *pIdxInfo, પૂર્ણાંક iCons){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  અચલ અક્ષર *zRet = 0;
  જો( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    પૂર્ણાંક iTerm = pIdxInfo->aConstraint[iCons].iTermOffset;
    પદાવલિ *pX = termFromWhereClause(pHidden->pWC, iTerm)->p_પદાવલિ;
    જો( pX->p_ડાબું ){
      pC = sqlite3_પદાવલિ_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_દ્વિઅંકી);
  }
  પાછા zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
પૂર્ણાંક sqlite3_vtab_in(sqlite3_index_info *pIdxInfo, પૂર્ણાંક iCons, પૂર્ણાંક bHandle){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  જો( m & pHidden->mIn ){
    જો( bHandle==0 ){
      pHidden->mHandleIn &= ~m;
    }નહીંતર જો( bHandle>0 ){
      pHidden->mHandleIn |= m;
    }
    પાછા 1;
  }
  પાછા 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
પૂર્ણાંક sqlite3_vtab_rhs_કિંમત(
  sqlite3_index_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  પૂર્ણાંક iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_કિંમત **ppVal           /* Write value extracted here */
){
  HiddenIndexInfo *pH = (HiddenIndexInfo*)&pIdxInfo[1];
  sqlite3_કિંમત *pVal = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  જો( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }નહીંતર{
    જો( pH->aRhs[iCons]==0 ){
      WhereTerm *pTerm = termFromWhereClause(
          pH->pWC, pIdxInfo->aConstraint[iCons].iTermOffset
      );
      rc = sqlite3_કિંમત_From_પદાવલિ(
          pH->pParse->db, pTerm->p_પદાવલિ->p_જમણું, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  જો( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  પાછા rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
પૂર્ણાંક sqlite3_vtab_distinct(sqlite3_index_info *pIdxInfo){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  પ્રતિજ્ઞા( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  પાછા pHidden->eDistinct;
}

/*
** Cause the prepared statement that is associated with a call to
** xBestIndex to potentially use all schemas.  If the statement being
** prepared is read-only, then just start read transactions on all
** schemas.  But if this is a write operation, start writes on all
** schemas.
**
** This is used by the (built-in) sqlite_dbpage virtual table.
*/
શૂન્ય sqlite3VtabUsesAllSchemas(Parse *pParse){
  પૂર્ણાંક nDb = pParse->db->nDb;
  પૂર્ણાંક i;
  માટે(i=0; i<nDb; i++){
    sqlite3CodeVerifySchema(pParse, i);
  }
  જો( DbMaskNonZero(pParse->writeMask) ){
    માટે(i=0; i<nDb; i++){
      sqlite3BeginWriteOperation(pParse, 0, i);
    }
  }
}

/*
** Add all WhereLoop objects for a table of the join identified by
** pBuilder->pNew->iTab.  That table is guaranteed to be a virtual table.
**
** If there are no LEFT or CROSS JOIN joins in the query, both mPrereq and
** mUnusable are set to 0. Otherwise, mPrereq is a mask of all FROM clause
** entries that occur before the virtual table in the FROM clause and are
** separated from it by at least one LEFT or CROSS JOIN. Similarly, the
** mUnusable mask contains all FROM clause entries that occur after the
** virtual table and are separated from it by at least one LEFT or
** CROSS JOIN.
**
** For example, if the query were:
**
**   ... FROM t1, t2 LEFT JOIN t3, t4, vt CROSS JOIN t5, t6;
**
** then mPrereq corresponds to (t1, t2) and mUnusable to (t5, t6).
**
** All the tables in mPrereq must be scanned before the current virtual
** table. So any terms for which all prerequisites are satisfied by
** mPrereq may be specified as "usable" in all calls to xBestIndex.
** Conversely, all tables in mUnusable must be scanned after the current
** virtual table, so any terms for which the prerequisites overlap with
** mUnusable should always be configured as "not-usable" for xBestIndex.
*/
સ્થિર પૂર્ણાંક whereLoopAddVirtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  પૂર્ણાંક rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  SrcItem *pSrc;               /* The FROM clause term to search */
  sqlite3_index_info *p;       /* Object to pass to xBestIndex() */
  પૂર્ણાંક nConstraint;             /* Number of constraints in p */
  પૂર્ણાંક bIn;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask mBest;               /* Tables used by best possible plan */
  u16 mNoOmit;
  પૂર્ણાંક bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  પ્રતિજ્ઞા( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  પ્રતિજ્ઞા( IsVirtual(pSrc->pSTab) );
  p = allocateIndexInfo(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  જો( p==0 ) પાછા SQLITE_NOMEM_BKPT;
  pNew->rSetup = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  જો( whereLoopResize(pParse->db, pNew, nConstraint) ){
    freeIndexInfo(pParse->db, p);
    પાછા SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = whereLoopAddVirtualOne(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, &bRetry
  );
  જો( bRetry ){
    પ્રતિજ્ઞા( rc==SQLITE_OK );
    rc = whereLoopAddVirtualOne(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  જો( rc==SQLITE_OK && ((mBest = (pNew->prereq & ~mPrereq))!=0 || bIn) ){
    પૂર્ણાંક seenZero = 0;             /* True if a plan with no prereqs seen */
    પૂર્ણાંક seenZeroNoIN = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask mBestNoIn = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    જો( bIn ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, ALLBITS, WO_IN, p, mNoOmit, &bIn, 0);
      પ્રતિજ્ઞા( bIn==0 );
      mBestNoIn = pNew->prereq & ~mPrereq;
      જો( mBestNoIn==0 ){
        seenZero = 1;
        seenZeroNoIN = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    જ્યારે( rc==SQLITE_OK ){
      પૂર્ણાંક i;
      Bitmask mNext = ALLBITS;
      પ્રતિજ્ઞા( mNext>0 );
      માટે(i=0; i<nConstraint; i++){
        પૂર્ણાંક iTerm = p->aConstraint[i].iTermOffset;
        Bitmask mThis = termFromWhereClause(pWC, iTerm)->prereq_જમણું & ~mPrereq;
        જો( mThis>mPrev && mThis<mNext ) mNext = mThis;
      }
      mPrev = mNext;
      જો( mNext==ALLBITS ) તોડો;
      જો( mNext==mBest || mNext==mBestNoIn ) ચાલુ;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)mNext));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mNext|mPrereq, 0, p, mNoOmit, &bIn, 0);
      જો( pNew->prereq==mPrereq ){
        seenZero = 1;
        જો( bIn==0 ) seenZeroNoIN = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    જો( rc==SQLITE_OK && seenZero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &bIn, 0);
      જો( bIn==0 ) seenZeroNoIN = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    જો( rc==SQLITE_OK && seenZeroNoIN==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, WO_IN, p, mNoOmit, &bIn, 0);
    }
  }

  freeIndexInfo(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  પાછા rc;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
સ્થિર પૂર્ણાંક whereLoopAddOr(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  WhereTerm *pTerm, *pWCEnd;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક iCur;
  WhereClause tempWC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet sSum, sCur;
  SrcItem *pItem;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  memset(&sSum, 0, કદ(sSum));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  જો( pItem->fg.jointype & JT_જમણું ) પાછા SQLITE_OK;

  માટે(pTerm=pWC->a; pTerm<pWCEnd && rc==SQLITE_OK; pTerm++){
    જો( (pTerm->eOperator & WO_OR)!=0
     && (pTerm->u.pOrInfo->indexable & pNew->maskSelf)!=0
     && (!પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON)
          || pTerm->p_પદાવલિ->w.iJoin==iCur)
    ){
      WhereClause * અચલ pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm * અચલ pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      પૂર્ણાંક once = 1;
      પૂર્ણાંક i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.pOrSet = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", pTerm));
      માટે(pOrTerm=pOrWC->a; pOrTerm<pOrWCEnd; pOrTerm++){
        જો( (pOrTerm->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        }નહીંતર જો( pOrTerm->ડાબું_Cursor==iCur ){
          tempWC.pWInfo = pWC->pWInfo;
          tempWC.pOuter = pWC;
          tempWC.op = TK_AND;
          tempWC.nTerm = 1;
          tempWC.nBase = 1;
          tempWC.a = pOrTerm;
          sSubBuild.pWC = &tempWC;
        }નહીંતર{
          ચાલુ;
        }
        sCur.n = 0;
#ifdef WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (પૂર્ણાંક)(pOrTerm-pOrWC->a), pTerm, sSubBuild.pWC->nTerm));
        જો( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
        જો( IsVirtual(pItem->pSTab) ){
          rc = whereLoopAddVirtual(&sSubBuild, mPrereq, mUnusable);
        }નહીંતર
#endif
        {
          rc = whereLoopAddBtree(&sSubBuild, mPrereq);
        }
        જો( rc==SQLITE_OK ){
          rc = whereLoopAddOr(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_DONE );
        જો( sCur.n==0 ){
          sSum.n = 0;
          તોડો;
        }નહીંતર જો( once ){
          whereOrMove(&sSum, &sCur);
          once = 0;
        }નહીંતર{
          WhereOrSet sPrev;
          whereOrMove(&sPrev, &sSum);
          sSum.n = 0;
          માટે(i=0; i<sPrev.n; i++){
            માટે(j=0; j<sCur.n; j++){
              where_Or_ઉમેરો(&sSum, sPrev.a[i].prereq | sCur.a[j].prereq,
                            sqlite3LogEstAdd(sPrev.a[i].rRun, sCur.a[j].rRun),
                            sqlite3LogEstAdd(sPrev.a[i].nOut, sCur.a[j].nOut));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = pTerm;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->rSetup = 0;
      pNew->iSortIdx = 0;
      memset(&pNew->u, 0, કદ(pNew->u));
      માટે(i=0; rc==SQLITE_OK && i<sSum.n; i++){
        /* TUNING: Currently sSum.a[i].rRun is set to the sum of the costs
        ** of all sub-scans required by the OR-scan. However, due to rounding
        ** errors, it may be that the cost of the OR-scan is equal to its
        ** most expensive sub-scan. Add the smallest possible penalty
        ** (equivalent to multiplying the cost by 1.07) to ensure that
        ** this does not happen. Otherwise, for WHERE clauses such as the
        ** following where there is an index on "y":
        **
        **     WHERE likelihood(x=?, 0.99) OR y=?
        **
        ** the planner may elect to "OR" together a full-table scan and an
        ** index lookup. And other similarly odd results.  */
        pNew->rRun = sSum.a[i].rRun + 1;
        pNew->nOut = sSum.a[i].nOut;
        pNew->prereq = sSum.a[i].prereq;
        rc = where_Loop_ઉમેરો(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", pTerm));
    }
  }
  પાછા rc;
}

/*
** Add all WhereLoop objects for all tables
*/
સ્થિર પૂર્ણાંક whereLoopAddAll(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask mPrior = 0;
  પૂર્ણાંક iTab;
  SrcList *pTabList = pWInfo->pTabList;
  SrcItem *pItem;
  SrcItem *pEnd = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક bFirstPastRJ = 0;
  પૂર્ણાંક has_જમણું_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  પ્રતિજ્ઞા( pNew->nLTerm==0 );
  પ્રતિજ્ઞા( pNew->wsFlags==0 );
  પ્રતિજ્ઞા( pNew->nLSlot>=ArraySize(pNew->aLTermSpace) );
  પ્રતિજ્ઞા( pNew->aLTerm!=0 );

  pBuilder->iPlanLimit = SQLITE_QUERY_PLANNER_LIMIT;
  માટે(iTab=0, pItem=pTabList->a; pItem<pEnd; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->iPlanLimit += SQLITE_QUERY_PLANNER_LIMIT_INCR;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    જો( bFirstPastRJ
     || (pItem->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
    ){
      /* Add prerequisites to prevent reordering of FROM clause terms
      ** across CROSS joins and outer joins.  The bFirstPastRJ boolean
      ** prevents the right operand of a RIGHT JOIN from being swapped with
      ** other elements even further to the right.
      **
      ** The hasRightCrossJoin flag prevent FROM-clause terms from moving
      ** from the right side of a LEFT JOIN or CROSS JOIN over to the
      ** left side of that same join.  This is a required restriction in
      ** the case of LEFT JOIN - an incorrect answer may results if it is
      ** not enforced.  This restriction is not required for CROSS JOIN.
      ** It is provided merely as a means of controlling join order, under
      ** the theory that no real-world queries that care about performance
      ** actually use the CROSS JOIN syntax.
      */
      જો( pItem->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( pItem->fg.jointype & JT_LTORJ );
        testcase( pItem->fg.jointype & JT_CROSS );
        has_જમણું_Cross_Join = 1;
      }
      mPrereq |= mPrior;
      bFirstPastRJ = (pItem->fg.jointype & JT_જમણું)!=0;
    }નહીંતર જો( pItem->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      WhereTerm *pTerm;
      પૂર્ણાંક i;
      માટે(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
        જો( (pNew->maskSelf & pTerm->prereqAll)!=0 ){
          mPrereq |= (pTerm->prereqAll & (pNew->maskSelf-1));
        }
      }
    }નહીંતર જો( !has_જમણું_Cross_Join ){
      mPrereq = 0;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    જો( IsVirtual(pItem->pSTab) ){
      SrcItem *p;
      માટે(p=&pItem[1]; p<pEnd; p++){
        જો( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mPrereq, mUnusable);
    }નહીંતર
#endif /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = whereLoopAddBtree(pBuilder, mPrereq);
    }
    જો( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = whereLoopAddOr(pBuilder, mPrereq, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    જો( rc || db->mallocFailed ){
      જો( rc==SQLITE_DONE ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }નહીંતર{
        તોડો;
      }
    }
  }

  whereLoopClear(db, pNew);
  પાછા rc;
}

/* Implementation of the order-by-subquery optimization:
**
** WhereLoop pLoop, which the iLoop-th term of the nested loop, is really
** a subquery or CTE that has an ORDER BY clause.  See if any of the terms
** in the subquery ORDER BY clause will satisfy pOrderBy from the outer
** query.  Mark off all satisfied terms (by setting bits in *pOBSat) and
** return TRUE if they do.  If not, return false.
**
** Example:
**
**    CREATE TABLE t1(a,b,c, PRIMARY KEY(a,b));
**    CREATE TABLE t2(x,y);
**    WITH t3(p,q) AS MATERIALIZED (SELECT x+y, x-y FROM t2 ORDER BY x+y)
**       SELECT * FROM t3 JOIN t1 ON a=q ORDER BY p, b;
**
** The CTE named "t3" comes out in the natural order of "p", so the first
** first them of "ORDER BY p,b" is satisfied by a sequential scan of "t3"
** and sorting only needs to occur on the second term "b".
**
** Limitations:
**
** (1)  The optimization is not applied if the outer ORDER BY contains
**      a COLLATE clause.  The optimization might be applied if the
**      outer ORDER BY uses NULLS FIRST, NULLS LAST, ASC, and/or DESC as
**      long as the subquery ORDER BY does the same.  But if the
**      outer ORDER BY uses COLLATE, even a redundant COLLATE, the
**      optimization is bypassed.
**
** (2)  The subquery ORDER BY terms must exactly match subquery result
**      columns, including any COLLATE annotations.  This routine relies
**      on iOrderByCol to do matching between order by terms and result
**      columns, and iOrderByCol will not be set if the result column
**      and ORDER BY collations differ.
**
** (3)  The subquery and outer ORDER BY can be in opposite directions as
**      long as  the subquery is materialized.  If the subquery is
**      implemented as a co-routine, the sort orders must be in the same
**      direction because there is no way to run a co-routine backwards.
*/
સ્થિર SQLITE_NOINLINE પૂર્ણાંક wherePathMatchSubqueryOB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  પૂર્ણાંક iLoop,              /* Which level of the nested loop.  0==outermost */
  પૂર્ણાંક iCur,               /* Cursor used by the this loop */
  પદાવલિ_List *pOrderBy,     /* The ORDER BY clause on the whole query */
  Bitmask *pRevMask,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  પૂર્ણાંક iOB;                /* Index into pOrderBy->a[] */
  પૂર્ણાંક jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  પદાવલિ *pOBExpr;          /* Current term of outer ORDER BY */
  પદાવલિ_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.pOrderBy;
  પ્રતિજ્ઞા( pSubOB!=0 );
  માટે(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  માટે(jSub=0; jSub<pSubOB->n_પદાવલિ && iOB<pOrderBy->n_પદાવલિ; jSub++, iOB++){
    જો( pSubOB->a[jSub].u.x.iOrderByCol==0 ) તોડો;
    pOBExpr = pOrderBy->a[iOB].p_પદાવલિ;
    જો( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) તોડો;
    જો( pOBExpr->iTable!=iCur ) તોડો;
    જો( pOBExpr->iColumn!=pSubOB->a[jSub].u.x.iOrderByCol-1 ) તોડો;
    જો( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = pOrderBy->a[iOB].fg.sortFlags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.sortFlags;   /* sortFlags for jSub */
      જો( (sfSub & KEYINFO_ORDER_BIGNULL) != (sfOB & KEYINFO_ORDER_BIGNULL) ){
        તોડો;
      }
      revIdx = sfSub & KEYINFO_ORDER_DESC;
      જો( jSub>0 ){
        જો( (rev^revIdx)!=(sfOB & KEYINFO_ORDER_DESC) ){
          તોડો;
        }
      }નહીંતર{
        rev = revIdx ^ (sfOB & KEYINFO_ORDER_DESC);
        જો( rev ){
          જો( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            તોડો;
          }
          *pRevMask |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  પાછા jSub>0;
}

/*
** Examine a WherePath (with the addition of the extra WhereLoop of the 6th
** parameters) to see if it outputs rows in the requested ORDER BY
** (or GROUP BY) without requiring a separate sort operation.  Return N:
**
**   N>0:   N terms of the ORDER BY clause are satisfied
**   N==0:  No terms of the ORDER BY clause are satisfied
**   N<0:   Unknown yet how many terms of ORDER BY might be satisfied.  
**
** Note that processing for WHERE_GROUPBY and WHERE_DISTINCTBY is not as
** strict.  With GROUP BY and DISTINCT the only requirement is that
** equivalent rows appear immediately adjacent to one another.  GROUP BY
** and DISTINCT do not require rows to appear in any particular order as long
** as equivalent rows are grouped together.  Thus for GROUP BY and DISTINCT
** the pOrderBy terms can be matched in any order.  With ORDER BY, the
** pOrderBy terms must be matched in strict left-to-right order.
*/
સ્થિર i8 wherePathSatisfiesOrderBy(
  WhereInfo *pWInfo,    /* The WHERE clause */
  પદાવલિ_List *pOrderBy,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  WherePath *pPath,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *pLast,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *pRevMask     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 revSet;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 isOrderDistinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 isMatch;           /* iColumn matches a term of the ORDER BY clause */
  u16 eqOpMask;         /* Allowed equality operators */
  u16 n_ચાવી_Col;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 nOrderBy;         /* Number terms in the ORDER BY clause */
  પૂર્ણાંક iLoop;            /* Index of WhereLoop in pPath being processed */
  પૂર્ણાંક i, j;             /* Loop counters */
  પૂર્ણાંક iCur;             /* Cursor number for current WhereLoop */
  પૂર્ણાંક iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  WhereTerm *pTerm;     /* A single term of the WHERE clause */
  પદાવલિ *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  Index *pIndex;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask obDone;       /* Mask of all ORDER BY terms */
  Bitmask orderDistinctMask;  /* Mask of all well-ordered loops */
  Bitmask ready;              /* Mask of inner loops */

  /*
  ** We say the WhereLoop is "one-row" if it generates no more than one
  ** row of output.  A WhereLoop is one-row if all of the following are true:
  **  (a) All index columns match with WHERE_COLUMN_EQ.
  **  (b) The index is unique
  ** Any WhereLoop with an WHERE_COLUMN_EQ constraint on the rowid is one-row.
  ** Every one-row WhereLoop will have the WHERE_ONEROW bit set in wsFlags.
  **
  ** We say the WhereLoop is "order-distinct" if the set of columns from
  ** that WhereLoop that are in the ORDER BY clause are different for every
  ** row of the WhereLoop.  Every one-row WhereLoop is automatically
  ** order-distinct.   A WhereLoop that has no columns in the ORDER BY clause
  ** is not order-distinct. To be order-distinct is not quite the same as being
  ** UNIQUE since a UNIQUE column or index can have multiple rows that
  ** are NULL and NULL values are equivalent for the purpose of order-distinct.
  ** To be order-distinct, the columns must be UNIQUE and NOT NULL.
  **
  ** The rowid for a table is always UNIQUE and NOT NULL so whenever the
  ** rowid appears in the ORDER BY clause, the corresponding WhereLoop is
  ** automatically order-distinct.
  */

  પ્રતિજ્ઞા( pOrderBy!=0 );
  જો( nLoop && OptimizationDisabled(db, SQLITE_OrderByIdxJoin) ) પાછા 0;

  nOrderBy = pOrderBy->n_પદાવલિ;
  testcase( nOrderBy==BMS-1 );
  જો( nOrderBy>BMS-1 ) પાછા 0;  /* Cannot optimize overly large ORDER BYs */
  isOrderDistinct = 1;
  obDone = MASKBIT(nOrderBy)-1;
  orderDistinctMask = 0;
  ready = 0;
  eqOpMask = WO_EQ | WO_IS | WO_ISNULL;
  જો( wctrlFlags & (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eqOpMask |= WO_IN;
  }
  માટે(iLoop=0; isOrderDistinct && obSat<obDone && iLoop<=nLoop; iLoop++){
    જો( iLoop>0 ) ready |= pLoop->maskSelf;
    જો( iLoop<nLoop ){
      pLoop = pPath->aLoop[iLoop];
      જો( wctrlFlags & WHERE_ORDERBY_LIMIT ) ચાલુ;
    }નહીંતર{
      pLoop = pLast;
    }
    જો( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      જો( pLoop->u.vtab.isOrdered && pWInfo->pOrderBy==pOrderBy ){
        obSat = obDone;
      }નહીંતર{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        isOrderDistinct = 0;
      }
      તોડો;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    માટે(i=0; i<nOrderBy; i++){
      જો( MASKBIT(i) & obSat ) ચાલુ;
      pOBExpr = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pOrderBy->a[i].p_પદાવલિ);
      જો( NEVER(pOBExpr==0) ) ચાલુ;
      જો( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) ચાલુ;
      જો( pOBExpr->iTable!=iCur ) ચાલુ;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, eqOpMask, 0);
      જો( pTerm==0 ) ચાલુ;
      જો( pTerm->eOperator==WO_IN ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        પ્રતિજ્ઞા( wctrlFlags &
               (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        માટે(j=0; j<pLoop->nLTerm && pTerm!=pLoop->aLTerm[j]; j++){}
        જો( j>=pLoop->nLTerm ) ચાલુ;
      }
      જો( (pTerm->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_પદાવલિ_NNColl_Seq(pParse, pOrderBy->a[i].p_પદાવલિ);
        અચલ પદાવલિ *pCExpr = pTerm->p_પદાવલિ;
        CollSeq *pColl2 = sqlite3_પદાવલિ_Compare_Coll_Seq(pParse, pCExpr);
        અક્ષર aff_જમણું;
        પ્રતિજ્ઞા( pColl1 );
        જો( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          ચાલુ;
        }
        aff_જમણું = sqlite3_પદાવલિ_Affinity(pCExpr->p_જમણું);
        /* Left operand must be a column, hence always has affinity */
        પ્રતિજ્ઞા( sqlite3_પદાવલિ_Affinity(pCExpr->p_ડાબું)!=0 );
        જો( aff_જમણું!=0 && aff_જમણું!=sqlite3_પદાવલિ_Affinity(pCExpr->p_ડાબું) ){
          ચાલુ;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    જો( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      જો( pLoop->wsFlags & WHERE_IPK ){
        જો( pLoop->u.btree.pOrderBy
         && OptimizationEnabled(db, SQLITE_OrderBySubq)
         &&  wherePathMatchSubqueryOB(pLoop,iLoop,iCur,
                                     pOrderBy,pRevMask, &obSat, wctrlFlags)
        ){
          nColumn = 0;
          isOrderDistinct = 0;
        }નહીંતર{
          nColumn = 1;
        }
        pIndex = 0;
        n_ચાવી_Col = 1;
      }નહીંતર જો( (pIndex = pLoop->u.btree.pIndex)==0 || pIndex->bUnordered ){
        પાછા 0;
      }નહીંતર{
        n_ચાવી_Col = pIndex->n_ચાવી_Col;
        nColumn = pIndex->nColumn;
        પ્રતિજ્ઞા( nColumn==n_ચાવી_Col+1 || !HasRowid(pIndex->pTable) );
        પ્રતિજ્ઞા( pIndex->aiColumn[nColumn-1]==XN_ROWID
                          || !HasRowid(pIndex->pTable));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        isOrderDistinct = IsUniqueIndex(pIndex)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = revSet = 0;
      distinctColumns = 0;
      માટે(j=0; j<nColumn; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        પ્રતિજ્ઞા( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        જો( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
          u16 eOp = pLoop->aLTerm[j]->eOperator;

          /* Skip over == and IS and ISNULL terms.  (Also skip IN terms when
          ** doing WHERE_ORDERBY_LIMIT processing).  Except, IS and ISNULL
          ** terms imply that the index is not UNIQUE NOT NULL in which case
          ** the loop need to be marked as not order-distinct because it can
          ** have repeated NULL rows.
          **
          ** If the current term is a column of an ((?,?) IN (SELECT...))
          ** expression for which the SELECT returns more than one column,
          ** check that it is the only column used by this loop. Otherwise,
          ** if it is one of two or more, none of the columns can be
          ** considered to match an ORDER BY term.
          */
          જો( (eOp & eqOpMask)!=0 ){
            જો( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( isOrderDistinct );
              isOrderDistinct = 0;
            }
            ચાલુ; 
          }નહીંતર જો( ALWAYS(eOp & WO_IN) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            પદાવલિ *pX = pLoop->aLTerm[j]->p_પદાવલિ;
            માટે(i=j+1; i<pLoop->u.btree.nEq; i++){
              જો( pLoop->aLTerm[i]->p_પદાવલિ==pX ){
                પ્રતિજ્ઞા( (pLoop->aLTerm[i]->eOperator & WO_IN) );
                bOnce = 0;
                તોડો;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        જો( pIndex ){
          iColumn = pIndex->aiColumn[j];
          revIdx = pIndex->aSortOrder[j] & KEYINFO_ORDER_DESC;
          જો( iColumn==pIndex->pTable->iPKey ) iColumn = XN_ROWID;
        }નહીંતર{
          iColumn = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        જો( isOrderDistinct ){
          જો( iColumn>=0
           && j>=pLoop->u.btree.nEq
           && pIndex->pTable->aCol[iColumn].notNull==0
          ){
            isOrderDistinct = 0;
          }
          જો( iColumn==XN_પદાવલિ ){
            isOrderDistinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        isMatch = 0;
        માટે(i=0; bOnce && i<nOrderBy; i++){
          જો( MASKBIT(i) & obSat ) ચાલુ;
          pOBExpr = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pOrderBy->a[i].p_પદાવલિ);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          જો( NEVER(pOBExpr==0) ) ચાલુ;
          જો( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          જો( iColumn>=XN_ROWID ){
            જો( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) ચાલુ;
            જો( pOBExpr->iTable!=iCur ) ચાલુ;
            જો( pOBExpr->iColumn!=iColumn ) ચાલુ;
          }નહીંતર{
            પદાવલિ *p_Ix_પદાવલિ = pIndex->a_Col_પદાવલિ->a[j].p_પદાવલિ;
            જો( sqlite3_પદાવલિ_Compare_Skip(pOBExpr, p_Ix_પદાવલિ, iCur) ){
              ચાલુ;
            }
          }
          જો( iColumn!=XN_ROWID ){
            pColl = sqlite3_પદાવલિ_NNColl_Seq(pWInfo->pParse, pOrderBy->a[i].p_પદાવલિ);
            જો( sqlite3StrICmp(pColl->zName, pIndex->azColl[j])!=0 ) ચાલુ;
          }
          જો( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.nDistinctCol = j+1;
          }
          isMatch = 1;
          તોડો;
        }
        જો( isMatch && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          જો( revSet ){
            જો( (rev ^ revIdx)
                           != (pOrderBy->a[i].fg.sortFlags&KEYINFO_ORDER_DESC)
            ){
              isMatch = 0;
            }
          }નહીંતર{
            rev = revIdx ^ (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC);
            જો( rev ) *pRevMask |= MASKBIT(iLoop);
            revSet = 1;
          }
        }
        જો( isMatch && (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL) ){
          જો( j==pLoop->u.btree.nEq 
           && (j<n_ચાવી_Col||HasRowid(pIndex->pTable)||!Is_Primary_ચાવી_Index(pIndex))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_SORT;
          }નહીંતર{
            isMatch = 0;
          }
        }
        જો( isMatch ){
          જો( iColumn==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }નહીંતર{
          /* No match found */
          પ્રતિજ્ઞા( n_ચાવી_Col>0 );
          જો( j<n_ચાવી_Col ){
            testcase( isOrderDistinct!=0 );
            isOrderDistinct = 0;
          }
          તોડો;
        }
      } /* end Loop over all index columns */
      જો( distinctColumns ){
        testcase( isOrderDistinct==0 );
        isOrderDistinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    જો( isOrderDistinct ){
      orderDistinctMask |= pLoop->maskSelf;
      માટે(i=0; i<nOrderBy; i++){
        પદાવલિ *p;
        Bitmask mTerm;
        જો( MASKBIT(i) & obSat ) ચાલુ;
        p = pOrderBy->a[i].p_પદાવલિ;
        mTerm = sqlite3_Where_પદાવલિ_Usage(&pWInfo->sMaskSet,p);
        જો( mTerm==0 && !sqlite3_પદાવલિ_Is_Constant(0,p) ) ચાલુ;
        જો( (mTerm&~orderDistinctMask)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  જો( obSat==obDone ) પાછા (i8)nOrderBy;
  જો( !isOrderDistinct ){
    માટે(i=nOrderBy-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      જો( (obSat&m)==m ) પાછા i;
    }
    પાછા 0;
  }
  પાછા -1;
}


/*
** If the WHERE_GROUPBY flag is set in the mask passed to sqlite3WhereBegin(),
** the planner assumes that the specified pOrderBy list is actually a GROUP
** BY clause - and so any order that groups rows as required satisfies the
** request.
**
** Normally, in this case it is not possible for the caller to determine
** whether or not the rows are really being delivered in sorted order, or
** just in some other order that provides the required grouping. However,
** if the WHERE_SORTBYGROUP flag is also passed to sqlite3WhereBegin(), then
** this function may be called on the returned WhereInfo object. It returns
** true if the rows really will be sorted in the specified order, or false
** otherwise.
**
** For example, assuming:
**
**   CREATE INDEX i1 ON t1(x, Y);
**
** then
**
**   SELECT * FROM t1 GROUP BY x,y ORDER BY x,y;   -- IsSorted()==1
**   SELECT * FROM t1 GROUP BY y,x ORDER BY y,x;   -- IsSorted()==0
*/
પૂર્ણાંક sqlite3WhereIsSorted(WhereInfo *pWInfo){
  પ્રતિજ્ઞા( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  પ્રતિજ્ઞા( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  પાછા pWInfo->sorted;
}

#ifdef WHERETRACE_ENABLED
/* For debugging use only: */
સ્થિર અચલ અક્ષર *wherePathName(WherePath *pPath, પૂર્ણાંક nLoop, WhereLoop *pLast){
  સ્થિર અક્ષર zName[65];
  પૂર્ણાંક i;
  માટે(i=0; i<nLoop; i++){ zName[i] = pPath->aLoop[i]->cId; }
  જો( pLast ) zName[i++] = pLast->cId;
  zName[i] = 0;
  પાછા zName;
}
#endif

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
સ્થિર LogEst whereSortingCost(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst nRow,       /* Estimated number of rows to sort */
  પૂર્ણાંક nOrderBy,      /* Number of ORDER BY clause terms */
  પૂર્ણાંક nSorted        /* Number of initial ORDER BY terms naturally in order */
){
  /* Estimated cost of a full external sort, where N is
  ** the number of rows to sort is:
  **
  **   cost = (K * N * log(N)).
  **
  ** Or, if the order-by clause has X terms but only the last Y
  ** terms are out of order, then block-sorting will reduce the
  ** sorting cost to:
  **
  **   cost = (K * N * log(N)) * (Y/X)
  **
  ** The constant K is at least 2.0 but will be larger if there are a
  ** large number of columns to be sorted, as the sorting time is
  ** proportional to the amount of content to be sorted.  The algorithm
  ** does not currently distinguish between fat columns (BLOBs and TEXTs)
  ** and skinny columns (INTs).  It just uses the number of columns as
  ** an approximation for the row width.
  **
  ** And extra factor of 2.0 or 3.0 is added to the sorting cost if the sort
  ** is built using OP_IdxInsert and OP_Sort rather than with OP_SorterInsert.
  */
  LogEst rSortCost, nCol;
  પ્રતિજ્ઞા( pWInfo->pSelect!=0 );
  પ્રતિજ્ઞા( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->n_પદાવલિ+59)/30);
  rSortCost = nRow + nCol;
  જો( nSorted>0 ){
    /* Scale the result by (Y/X) */
    rSortCost += sqlite3LogEst((nOrderBy-nSorted)*100/nOrderBy) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  જો( (pWInfo->wctrlFlags & WHERE_USE_LIMIT)!=0 ){
    rSortCost += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    જો( nSorted!=0 ){
      rSortCost += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    જો( pWInfo->iLimit<nRow ){
      nRow = pWInfo->iLimit;
    }
  }નહીંતર જો( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    જો( nRow>10 ){ nRow -= 10;  પ્રતિજ્ઞા( 10==sqlite3LogEst(2) ); }
  }
  rSortCost += estLog(nRow);
  પાછા rSortCost;
}

/*
** Compute the maximum number of paths in the solver algorithm, for
** queries that have three or more terms in the FROM clause.  Queries with
** two or fewer FROM clause terms are handled by the caller.
**
** Query planning is NP-hard.  We must limit the number of paths at
** each step of the solver search algorithm to avoid exponential behavior.
**
** The value returned is a tuning parameter.  Currently the value is:
**
**     18    for star queries
**     12    otherwise
**
** For the purposes of this heuristic, a star-query is defined as a query
** with a central "fact" table that is joined against multiple
** "dimension" tables, subject to the following constraints:
**
**   (aa)  Only a five-way or larger join is considered for this
**         optimization.  If there are fewer than four terms in the FROM
**         clause, this heuristic does not apply.
**
**   (bb)  The join between the fact table and the dimension tables must
**         be an INNER join.  CROSS and OUTER JOINs do not qualify.
**
**   (cc)  A table must have 3 or more dimension tables in order to be
**         considered a fact table. (Was 4 prior to 2026-02-10.)
**
**   (dd)  A table that is a self-join cannot be a dimension table.
**         Dimension tables are joined against fact tables.
**
** SIDE EFFECT:  (and really the whole point of this subroutine)
**
** If pWInfo describes a star-query, then the cost for SCANs of dimension
** WhereLoops is increased to be slightly larger than the cost of a SCAN
** in the fact table.  Only SCAN costs are increased.  SEARCH costs are
** unchanged. This heuristic helps keep fact tables in outer loops. Without
** this heuristic, paths with fact tables in outer loops tend to get pruned
** by the mxChoice limit on the number of paths, resulting in poor query
** plans.  See the starschema1.test test module for examples of queries
** that need this heuristic to find good query plans.
**
** This heuristic can be completely disabled, so that no query is
** considered a star-query, using SQLITE_TESTCTRL_OPTIMIZATION to
** disable the SQLITE_StarQuery optimization.  In the CLI, the command
** to do that is:  ".testctrl opt -starquery".
**
** HISTORICAL NOTES:
**
** This optimization was first added on 2024-05-09 by check-in 38db9b5c83d.
** The original optimization reduced the cost and output size estimate for
** fact tables to help them move to outer loops.  But months later (as people
** started upgrading) performance regression reports started caming in,
** including:
**
**    forum post b18ef983e68d06d1 (2024-12-21)
**    forum post 0025389d0860af82 (2025-01-14)
**    forum post d87570a145599033 (2025-01-17)
**
** To address these, the criteria for a star-query was tightened to exclude
** cases where the fact and dimensions are separated by an outer join, and
** the affect of star-schema detection was changed to increase the rRun cost
** on just full table scans of dimension tables, rather than reducing costs
** in the all access methods of the fact table.
*/
સ્થિર પૂર્ણાંક computeMxChoice(WhereInfo *pWInfo){
  પૂર્ણાંક nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#ifdef SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  માટે(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
    પ્રતિજ્ઞા( pWLoop->maskSelf==MASKBIT(pWLoop->iTab) );
    પ્રતિજ્ઞા( pWLoop->pNextLoop==0 || pWLoop->iTab<=pWLoop->pNextLoop->iTab );
  }
#endif /* SQLITE_DEBUG */

  જો( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->bStarDone
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_StarQuery)
  ){
    SrcItem *aFromTabs;    /* All terms of the FROM clause */
    પૂર્ણાંક iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *pStart;     /* Where to start searching for dimension-tables */

    pWInfo->bStarDone = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    પ્રતિજ્ઞા( !pWInfo->bStarUsed );
    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    માટે(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      પૂર્ણાંક nDep = 0;             /* Number of dimension tables */
      LogEst mxRun;             /* Maximum SCAN cost of a fact table */
      Bitmask mSeen = 0;        /* Mask of dimension tables */
      SrcItem *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      જો( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        જો( iFromIdx+3 > nLoop ){
          તોડો;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        જ્યારે( pStart && pStart->iTab<=iFromIdx ){
          pStart = pStart->pNextLoop;
        }
      }
      માટે(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        જો( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          તોડો; /* Constraint (bb) */
        }
        જો( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->maskSelf & mSeen)==0  /* pWInfo not already a dependency */
         && (pWLoop->maskSelf & mSelfJoin)==0 /* Not a self-join */
        ){
          જો( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }નહીંતર{
            nDep++;
            mSeen |= pWLoop->maskSelf;
          }
        }
      }
      જો( nDep<=2 ){
        ચાલુ; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#ifdef WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      જો( !pWInfo->bStarUsed ){
        માટે(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
          pWLoop->rStarDelta = 0;
        }
      }
#endif
#ifdef WHERETRACE_ENABLED /* 0x80000 */
      જો( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = mSeen;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        માટે(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
          જો( mShow & pWLoop->maskSelf ){
            SrcItem *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->maskSelf;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#endif
      pWInfo->bStarUsed = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mxRun = LOGEST_MIN;
      માટે(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        જો( pWLoop->iTab<iFromIdx ) ચાલુ;
        જો( pWLoop->iTab>iFromIdx ) તોડો;
        જો( pWLoop->rRun>mxRun ) mxRun = pWLoop->rRun;
      }
      જો( ALWAYS(mxRun<LOGEST_MAX) ) mxRun++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      માટે(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        જો( (pWLoop->maskSelf & mSeen)==0 ) ચાલુ;
        જો( pWLoop->nLTerm ) ચાલુ;
        જો( pWLoop->rRun<mxRun ){
#ifdef WHERETRACE_ENABLED /* 0x80000 */
          જો( sqlite3WhereTrace & 0x80000 ){
            SrcItem *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mxRun
            );
          }
          pWLoop->rStarDelta = mxRun - pWLoop->rRun;
#endif /* WHERETRACE_ENABLED */
          pWLoop->rRun = mxRun;
        }
      }
    }
#ifdef WHERETRACE_ENABLED /* 0x80000 */
    જો( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->bStarUsed ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      માટે(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        જો( pWLoop->rStarDelta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#endif
  }
  પાછા pWInfo->bStarUsed ? 18 : 12;
}

/*
** Two WhereLoop objects, pCandidate and pBaseline, are known to have the
** same cost.  Look deep into each to see if pCandidate is even slightly
** better than pBaseline.  Return false if it is, if pCandidate is is preferred.
** Return true if pBaseline is preferred or if we cannot tell the difference.
**
**    Result       Meaning
**    --------     ----------------------------------------------------------
**    true         We cannot tell the difference in pCandidate and pBaseline
**    false        pCandidate seems like a better choice than pBaseline
*/
સ્થિર SQLITE_NOINLINE પૂર્ણાંક whereLoopIsNoBetter(
  અચલ WhereLoop *pCandidate,
  અચલ WhereLoop *pBaseline
){
  જો( (pCandidate->wsFlags & WHERE_INDEXED)==0 ) પાછા 1;
  જો( (pBaseline->wsFlags & WHERE_INDEXED)==0 ) પાછા 1;
  જો( pCandidate->u.btree.pIndex->szIdxRow <
        pBaseline->u.btree.pIndex->szIdxRow ) પાછા 0;
  પાછા 1;
}

/*
** Given the list of WhereLoop objects at pWInfo->pLoops, this routine
** attempts to find the lowest cost path that visits each WhereLoop
** once.  This path is then loaded into the pWInfo->a[].pWLoop fields.
**
** Assume that the total number of output rows that will need to be sorted
** will be nRowEst (in the 10*log2 representation).  Or, ignore sorting
** costs if nRowEst==0.
**
** Return SQLITE_OK on success or SQLITE_NOMEM of a memory allocation
** error occurs.
*/
સ્થિર પૂર્ણાંક wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst){
  પૂર્ણાંક mxChoice;             /* Maximum number of simultaneous paths tracked */
  પૂર્ણાંક nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  પૂર્ણાંક iLoop;                /* Loop counter over the terms of the join */
  પૂર્ણાંક ii, jj;               /* Loop counters */
  પૂર્ણાંક mxI = 0;              /* Index of next entry to replace */
  પૂર્ણાંક nOrderBy;             /* Number of ORDER BY clause terms */
  LogEst mxCost = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  પૂર્ણાંક nTo, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  WherePath *aFrom;         /* All nFrom paths at the previous level */
  WherePath *aTo;           /* The nTo best paths at the current level */
  WherePath *pFrom;         /* An element of aFrom[] that we are working on */
  WherePath *pTo;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *aSortCost = 0;    /* Sorting and partial sorting costs */
  અક્ષર *pSpace;             /* Temporary memory used by this routine */
  પૂર્ણાંક nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     nRowEst, pParse->nQueryLoop));
  /* TUNING: mxChoice is the maximum number of possible paths to preserve
  ** at each step.  Based on the number of loops in the FROM clause:
  **
  **     nLoop      mxChoice
  **     -----      --------
  **       1            1            // the most common case
  **       2            5
  **       3+        12 or 18        // see computeMxChoice()
  */
  જો( nLoop<=1 ){
    mxChoice = 1;
  }નહીંતર જો( nLoop==2 ){
    mxChoice = 5;
  }નહીંતર જો( pParse->nErr ){
    mxChoice = 1;
  }નહીંતર{
    mxChoice = computeMxChoice(pWInfo);
  }
  પ્રતિજ્ઞા( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  જો( pWInfo->pOrderBy==0 || nRowEst==0 ){
    nOrderBy = 0;
  }નહીંતર{
    nOrderBy = pWInfo->pOrderBy->n_પદાવલિ;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (કદ(WherePath)+કદ(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += કદ(LogEst) * nOrderBy;
  pSpace = sqlite3_ઢગલો_Alloc_Raw_NN(pParse->db, nSpace);
  જો( pSpace==0 ) પાછા SQLITE_NOMEM_BKPT;
  aTo = (WherePath*)pSpace;
  aFrom = aTo+mxChoice;
  memset(aFrom, 0, કદ(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  માટે(ii=mxChoice*2, pFrom=aTo; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  જો( nOrderBy ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    aSortCost = (LogEst*)pX;
    memset(aSortCost, 0, કદ(LogEst) * nOrderBy);
  }
  પ્રતિજ્ઞા( aSortCost==0 || &pSpace[nSpace]==(અક્ષર*)&aSortCost[nOrderBy] );
  પ્રતિજ્ઞા( aSortCost!=0 || &pSpace[nSpace]==(અક્ષર*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  પ્રતિજ્ઞા( 48==sqlite3LogEst(28) );
  nFrom = 1;
  પ્રતિજ્ઞા( aFrom[0].isOrdered==0 );
  જો( nOrderBy ){
    /* If nLoop is zero, then there are no FROM terms in the query. Since
    ** in this case the query may return a maximum of one row, the results
    ** are already in the requested order. Set isOrdered to nOrderBy to
    ** indicate this. Or, if nLoop is greater than zero, set isOrdered to
    ** -1, indicating that the result set may or may not be ordered,
    ** depending on the loops added to the current plan.  */
    aFrom[0].isOrdered = nLoop>0 ? -1 : nOrderBy;
  }

  /* Compute successively longer WherePaths using the previous generation
  ** of WherePaths as the basis for the next.  Keep track of the mxChoice
  ** best paths at each generation */
  માટે(iLoop=0; iLoop<nLoop; iLoop++){
    nTo = 0;
    માટે(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      માટે(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        LogEst nOut;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask maskNew;                  /* Mask of src visited by (..) */
        Bitmask revMask;                  /* Mask of rev-order loops for (..) */

        જો( (pWLoop->prereq & ~pFrom->maskLoop)!=0 ) ચાલુ;
        જો( (pWLoop->maskSelf & pFrom->maskLoop)!=0 ) ચાલુ;
        જો( (pWLoop->wsFlags & WHERE_AUTO_INDEX)!=0 && pFrom->nRow<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          પ્રતિજ્ઞા( 10==sqlite3LogEst(2) );
          ચાલુ;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->rRun + pFrom->nRow;
        જો( pWLoop->rSetup ){
          rUnsort = sqlite3LogEstAdd(pWLoop->rSetup, rUnsort);
        }
        rUnsort = sqlite3LogEstAdd(rUnsort, pFrom->rUnsort);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        isOrdered = pFrom->isOrdered;
        જો( isOrdered<0 ){
          revMask = 0;
          isOrdered = wherePathSatisfiesOrderBy(pWInfo,
                       pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &revMask);
        }નહીંતર{
          revMask = pFrom->revLoop;
        }
        જો( isOrdered>=0 && isOrdered<nOrderBy ){
          જો( aSortCost[isOrdered]==0 ){
            aSortCost[isOrdered] = whereSortingCost(
                pWInfo, nRowEst, nOrderBy, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          rCost = sqlite3LogEstAdd(rUnsort, aSortCost[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               aSortCost[isOrdered], (nOrderBy-isOrdered), nOrderBy,
               rUnsort, rCost));
        }નહીંતર{
          rCost = rUnsort;
          rUnsort -= 2;  /* TUNING:  Slight bias in favor of no-sort plans */
        }

        /* Check to see if pWLoop should be added to the set of
        ** mxChoice best-so-far paths.
        **
        ** First look for an existing path among best-so-far paths
        ** that:
        **     (1) covers the same set of loops, and
        **     (2) has a compatible isOrdered value.
        **
        ** "Compatible isOrdered value" means either
        **     (A) both have isOrdered==-1, or
        **     (B) both have isOrder>=0, or
        **     (C) ordering does not matter because this is the last round
        **         of the solver.
        **
        ** The term "((pTo->isOrdered^isOrdered)&0x80)==0" is equivalent
        ** to (pTo->isOrdered==(-1))==(isOrdered==(-1))" for the range
        ** of legal values for isOrdered, -1..64.
        */
        testcase( nTo==0 );
        માટે(jj=0, pTo=aTo; jj<nTo; jj++, pTo++){
          જો( pTo->maskLoop==maskNew
           && ( ((pTo->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==nTo-1 );
            તોડો;
          }
        }
        જો( jj>=nTo ){
          /* None of the existing best-so-far paths match the candidate. */
          જો( nTo>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
            જો( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#endif
            ચાલુ;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          જો( nTo<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = nTo++;
          }નહીંતર{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          pTo = &aTo[jj];
#ifdef WHERETRACE_ENABLED /* 0x4 */
          જો( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#endif
        }નહીંતર{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          જો( (pTo->rCost<rCost)
           || (pTo->rCost==rCost && pTo->nRow<nOut)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort<rUnsort)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, pTo->aLoop[iLoop]) )
          ){
#ifdef WHERETRACE_ENABLED /* 0x4 */
            જો( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
                  pTo->rUnsort, pTo->isOrdered>=0 ? pTo->isOrdered+'0' : '?');
            }
#endif
            /* Discard the candidate path from further consideration */
            testcase( pTo->rCost==rCost );
            ચાલુ;
          }
          testcase( pTo->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
          જો( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
                pTo->rUnsort, pTo->isOrdered>=0 ? pTo->isOrdered+'0' : '?');
          }
#endif
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        pTo->maskLoop = pFrom->maskLoop | pWLoop->maskSelf;
        pTo->revLoop = revMask;
        pTo->nRow = nOut;
        pTo->rCost = rCost;
        pTo->rUnsort = rUnsort;
        pTo->isOrdered = isOrdered;
        memcpy(pTo->aLoop, pFrom->aLoop, કદ(WhereLoop*)*iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        જો( nTo>=mxChoice ){
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsort = aTo[0].nRow;
          માટે(jj=1, pTo=&aTo[1]; jj<mxChoice; jj++, pTo++){
            જો( pTo->rCost>mxCost
             || (pTo->rCost==mxCost && pTo->rUnsort>mxUnsort)
            ){
              mxCost = pTo->rCost;
              mxUnsort = pTo->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#ifdef WHERETRACE_ENABLED  /* >=2 */
    જો( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      પૂર્ણાંક nDone = 0;
      પૂર્ણાંક nProgress;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      કરો{
        nProgress = 0;
        rMin = 0x7fff;
        માટે(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          જો( pTo->rCost>rFloor && pTo->rCost<rMin ) rMin = pTo->rCost;
        }
        માટે(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          જો( pTo->rCost==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
               pTo->isOrdered>=0 ? (pTo->isOrdered+'0') : '?');
            જો( pTo->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", pTo->revLoop);
            }નહીંતર{
              sqlite3DebugPrintf("\n");
            }
            nDone++;
            nProgress++;
          }
        }
        rFloor = rMin;
      }જ્યારે( nDone<nTo && nProgress>0 );
    }
#endif

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  જો( nFrom==0 ){
    sqlite3_Error_સંદેશ(pParse, "no query solution");
    sqlite3_ઢગલો_Free_NN(pParse->db, pSpace);
    પાછા SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  પ્રતિજ્ઞા( nFrom==1 );
  pFrom = aFrom;

  પ્રતિજ્ઞા( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  માટે(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  જો( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask notUsed;
    પૂર્ણાંક rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pResultSet, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &notUsed);
    જો( rc==pWInfo->pResultSet->n_પદાવલિ ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  જો( pWInfo->pOrderBy ){
    pWInfo->nOBSat = pFrom->isOrdered;
    જો( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      જો( pFrom->isOrdered==pWInfo->pOrderBy->n_પદાવલિ ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      પ્રતિજ્ઞા( pWInfo->pSelect->pOrderBy==0
           || pWInfo->nOBSat <= pWInfo->pSelect->pOrderBy->n_પદાવલિ );
    }નહીંતર{
      pWInfo->revMask = pFrom->revLoop;
      જો( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        જો( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          જો( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_COLUMN_IN))!=(WHERE_IPK|WHERE_COLUMN_IN)
          ){
            Bitmask m = 0;
            પૂર્ણાંક rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom,
                      WHERE_ORDERBY_LIMIT, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_COLUMN_IN );
            જો( rc==pWInfo->pOrderBy->n_પદાવલિ ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->revMask = m;
            }
          }
        }
      }નહીંતર જો( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    જો( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->pOrderBy->n_પદાવલિ && nLoop>0
    ){
      Bitmask revMask = 0;
      પૂર્ણાંક nOrder = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &revMask
      );
      પ્રતિજ્ઞા( pWInfo->sorted==0 );
      જો( nOrder==pWInfo->pOrderBy->n_પદાવલિ ){
        pWInfo->sorted = 1;
        pWInfo->revMask = revMask;
      }
    }
  }

  pWInfo->nRowOut = pFrom->nRow;
#ifdef WHERETRACE_ENABLED
  pWInfo->rTotalCost = pFrom->rCost;
#endif

  /* Free temporary memory and return success */
  sqlite3_ઢગલો_Free_NN(pParse->db, pSpace);
  પાછા SQLITE_OK;
}

/*
** This routine implements a heuristic designed to improve query planning.
** This routine is called in between the first and second call to
** wherePathSolver().  Hence the name "Interstage" "Heuristic".
**
** The first call to wherePathSolver() (hereafter just "solver()") computes
** the best path without regard to the order of the outputs.  The second call
** to the solver() builds upon the first call to try to find an alternative
** path that satisfies the ORDER BY clause.
**
** This routine looks at the results of the first solver() run, and for
** every FROM clause term in the resulting query plan that uses an equality
** constraint against an index, disable other WhereLoops for that same
** FROM clause term that would try to do a full-table scan.  This prevents
** an index search from being converted into a full-table scan in order to
** satisfy an ORDER BY clause, since even though we might get slightly better
** performance using the full-scan without sorting if the output size
** estimates are very precise, we might also get severe performance
** degradation using the full-scan if the output size estimate is too large.
** It is better to err on the side of caution.
**
** Except, if the first solver() call generated a full-table scan in an outer
** loop then stop this analysis at the first full-scan, since the second
** solver() run might try to swap that full-scan for another in order to
** get the output into the correct order.  In other words, we allow a
** rewrite like this:
**
**     First Solver()                      Second Solver()
**       |-- SCAN t1                         |-- SCAN t2
**       |-- SEARCH t2                       `-- SEARCH t1
**       `-- SORT USING B-TREE
**
** The purpose of this routine is to disallow rewrites such as:
**
**     First Solver()                      Second Solver()
**       |-- SEARCH t1                       |-- SCAN t2     <--- bad!
**       |-- SEARCH t2                       `-- SEARCH t1
**       `-- SORT USING B-TREE
**
** See test cases in test/whereN.test for the real-world query that
** originally provoked this heuristic.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય whereInterstageHeuristic(WhereInfo *pWInfo){
  પૂર્ણાંક i;
#ifdef WHERETRACE_ENABLED
  પૂર્ણાંક once = 0;
#endif
  માટે(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    જો( p==0 ) તોડો;
    જો( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      તોડો;
    }
    જો( (p->wsFlags & (WHERE_COLUMN_EQ|WHERE_COLUMN_NULL|WHERE_COLUMN_IN))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      માટે(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->pNextLoop){
        જો( pLoop->iTab!=iTab ) ચાલુ;
        જો( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_INDEX))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          ચાલુ;
        }
#ifdef WHERETRACE_ENABLED
        જો( sqlite3WhereTrace & 0x80 ){
          જો( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#endif /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }નહીંતર{
      તોડો;
    }
  }
}

/*
** Most queries use only a single table (they are not joins) and have
** simple == constraints against indexed fields.  This routine attempts
** to plan those simple cases using much less ceremony than the
** general-purpose query planner, and thereby yield faster sqlite3_prepare()
** times for the common case.
**
** Return non-zero on success, if this query can be handled by this
** no-frills query planner.  Return zero if this query needs the
** general-purpose query planner.
*/
સ્થિર પૂર્ણાંક whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  પૂર્ણાંક iCur;
  પૂર્ણાંક j;
  Table *pTab;
  Index *pIdx;
  WhereScan scan;

  pWInfo = pBuilder->pWInfo;
  જો( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) પાછા 0;
  પ્રતિજ્ઞા( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  જો( IsVirtual(pTab) ) પાછા 0;
  જો( pItem->fg.isIndexedBy || pItem->fg.notIndexed ){
    testcase( pItem->fg.isIndexedBy );
    testcase( pItem->fg.notIndexed );
    પાછા 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = whereScanInit(&scan, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  જ્યારે( pTerm && pTerm->prereq_જમણું ) pTerm = whereScanNext(&scan);
  જો( pTerm ){
    testcase( pTerm->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->rRun = 33;  /* 33==sqlite3LogEst(10) */
  }નહીંતર{
    માટે(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      પૂર્ણાંક opMask;
      પ્રતિજ્ઞા( pLoop->aLTermSpace==pLoop->aLTerm );
      જો( !IsUniqueIndex(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_ચાવી_Col>ArraySize(pLoop->aLTermSpace)
      ) ચાલુ;
      opMask = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      માટે(j=0; j<pIdx->n_ચાવી_Col; j++){
        pTerm = whereScanInit(&scan, pWC, iCur, j, opMask, pIdx);
        જ્યારે( pTerm && pTerm->prereq_જમણું ) pTerm = whereScanNext(&scan);
        જો( pTerm==0 ) તોડો;
        testcase( pTerm->eOperator & WO_IS );
        pLoop->aLTerm[j] = pTerm;
      }
      જો( j!=pIdx->n_ચાવી_Col ) ચાલુ;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_INDEXED;
      જો( pIdx->isCovering || (pItem->colUsed & pIdx->colNotIdxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.pIndex = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->rRun = 39;  /* 39==sqlite3LogEst(15) */
      તોડો;
    }
  }
  જો( pLoop->wsFlags ){
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    પ્રતિજ્ઞા( pWInfo->sMaskSet.n==1 && iCur==pWInfo->sMaskSet.ix[0] );
    pLoop->maskSelf = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    જો( pWInfo->pOrderBy ) pWInfo->nOBSat =  pWInfo->pOrderBy->n_પદાવલિ;
    જો( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    જો( scan.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#ifdef SQLITE_DEBUG
    pLoop->cId = '0';
#endif
#ifdef WHERETRACE_ENABLED
    જો( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#endif
    પાછા 1;
  }
  પાછા 0;
}

/*
** Helper function for exprIsDeterministic().
*/
સ્થિર પૂર્ણાંક પદાવલિ_ગાંઠ_Is_Deterministic(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_FUNCTION && પદાવલિ_Has_Property(p_પદાવલિ, EP_ConstFunc)==0 ){
    pWalker->eCode = 0;
    પાછા WRC_Abort;
  }
  પાછા WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Is_Deterministic(પદાવલિ *p){
  Walker w;
  memset(&w, 0, કદ(w));
  w.eCode = 1;
  w.x_પદાવલિ_Callback = પદાવલિ_ગાંઠ_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.eCode;
}

 
#ifdef WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
સ્થિર શૂન્ય showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  જો( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    પૂર્ણાંક i;
    સ્થિર અચલ અક્ષર zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    માટે(p=pWInfo->pLoops, i=0; p; p=p->pNextLoop, i++){
      p->cId = zLabel[i%(કદ(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#નહીંતર
# define WHERETRACE_ALL_LOOPS(W,C)
#endif

/* Attempt to omit tables from a join that do not affect the result.
** For a table to not affect the result, the following must be true:
**
**   1) The query must not be an aggregate.
**   2) The table must be the RHS of a LEFT JOIN.
**   3) Either the query must be DISTINCT, or else the ON or USING clause
**      must contain a constraint that limits the scan of the table to
**      at most a single row.
**   4) The table must not be referenced by any part of the query apart
**      from its own USING or ON clause.
**   5) The table must not have an inner-join ON or USING clause if there is
**      a RIGHT JOIN anywhere in the query.  Otherwise the ON/USING clause
**      might move from the right side to the left side of the RIGHT JOIN.
**      Note: Due to (2), this condition can only arise if the table is
**      the right-most table of a subquery that was flattened into the
**      main query and that subquery was the right-hand operand of an
**      inner join that held an ON or USING clause.
**   6) The ORDER BY clause has 63 or fewer terms
**   7) The omit-noop-join optimization is enabled.
**
** Items (1), (6), and (7) are checked by the caller.
**
** For example, given:
**
**     CREATE TABLE t1(ipk INTEGER PRIMARY KEY, v1);
**     CREATE TABLE t2(ipk INTEGER PRIMARY KEY, v2);
**     CREATE TABLE t3(ipk INTEGER PRIMARY KEY, v3);
**
** then table t2 can be omitted from the following:
**
**     SELECT v1, v3 FROM t1
**       LEFT JOIN t2 ON (t1.ipk=t2.ipk)
**       LEFT JOIN t3 ON (t1.ipk=t3.ipk)
**
** or from:
**
**     SELECT DISTINCT v1, v3 FROM t1
**       LEFT JOIN t2
**       LEFT JOIN t3 ON (t1.ipk=t3.ipk)
*/
સ્થિર SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  પૂર્ણાંક i;
  Bitmask tabUsed;
  પૂર્ણાંક has_જમણું_Join;

  /* Preconditions checked by the caller */
  પ્રતિજ્ઞા( pWInfo->nLevel>=2 );
  પ્રતિજ્ઞા( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  પ્રતિજ્ઞા( pWInfo->pResultSet!=0 );
  પ્રતિજ્ઞા( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tabUsed = sqlite3_Where_પદાવલિ_List_Usage(&pWInfo->sMaskSet, pWInfo->pResultSet);
  જો( pWInfo->pOrderBy ){
    tabUsed |= sqlite3_Where_પદાવલિ_List_Usage(&pWInfo->sMaskSet, pWInfo->pOrderBy);
  }
  has_જમણું_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  માટે(i=pWInfo->nLevel-1; i>=1; i--){
    WhereTerm *pTerm, *pEnd;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    જો( (pItem->fg.jointype & (JT_ડાબું|JT_જમણું))!=JT_ડાબું ) ચાલુ;
    જો( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      ચાલુ;
    }
    જો( (tabUsed & pLoop->maskSelf)!=0 ) ચાલુ;
    pEnd = pWInfo->sWC.a + pWInfo->sWC.nTerm;
    માટે(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      જો( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        જો( !પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON)
         || pTerm->p_પદાવલિ->w.iJoin!=pItem->iCursor
        ){
          તોડો;
        }
      }
      જો( has_જમણું_Join
       && પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_InnerON)
       && NEVER(pTerm->p_પદાવલિ->w.iJoin==pItem->iCursor)
      ){
        તોડો;  /* restriction (5) */
      }
    }
    જો( pTerm<pEnd ) ચાલુ;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->revMask>>1) & ~m1)!=0 );
    pWInfo->revMask = (m1 & pWInfo->revMask) | ((pWInfo->revMask>>1) & ~m1);
    notReady &= ~pLoop->maskSelf;
    માટે(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      જો( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        pTerm->wtFlags |= TERM_CODED;
        pTerm->prereqAll = 0;
      }
    }
    જો( i!=pWInfo->nLevel-1 ){
      પૂર્ણાંક nByte = (pWInfo->nLevel-1-i) * કદ(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    પ્રતિજ્ઞા( pWInfo->nLevel>0 );
  }
  પાછા notReady;
}

/*
** Check to see if there are any SEARCH loops that might benefit from
** using a Bloom filter.  Consider a Bloom filter if:
**
**   (1)  The SEARCH happens more than N times where N is the number
**        of rows in the table that is being considered for the Bloom
**        filter.
**   (2)  Some searches are expected to find zero rows.  (This is determined
**        by the WHERE_SELFCULL flag on the term.)
**   (3)  Bloom-filter processing is not disabled.  (Checked by the
**        caller.)
**   (4)  The size of the table being searched is known by ANALYZE.
**
** This block of code merely checks to see if a Bloom filter would be
** appropriate, and if so sets the WHERE_BLOOMFILTER flag on the
** WhereLoop.  The implementation of the Bloom filter comes further
** down where the code for each WhereLoop is generated.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય whereCheckIfBloomFilterIsUseful(
  અચલ WhereInfo *pWInfo
){
  પૂર્ણાંક i;
  LogEst n_શોધ = 0;

  પ્રતિજ્ઞા( pWInfo->nLevel>=2 );
  પ્રતિજ્ઞા( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  માટે(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    અચલ unsigned પૂર્ણાંક reqFlags = (WHERE_SELFCULL|WHERE_COLUMN_EQ);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    Table *pTab = pItem->pSTab;
    જો( (pTab->tabFlags & TF_HasStat1)==0 ) તોડો;
    pTab->tabFlags |= TF_MaybeReanalyze;
    જો( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_શોધ > pTab->nRowLogEst
    ){
      testcase( pItem->fg.jointype & JT_ડાબું );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (દ્વિગુણ)sqlite3LogEstToInt(n_શોધ), pTab->zName,
         (દ્વિગુણ)sqlite3LogEstToInt(pTab->nRowLogEst)));
    }
    n_શોધ += pLoop->nOut;
  }
}

/*
** The index pIdx is used by a query and contains one or more expressions.
** In other words pIdx is an index on an expression.  iIdxCur is the cursor
** number for the index and iDataCur is the cursor number for the corresponding
** table.
**
** This routine adds IndexedExpr entries to the Parse->pIdxEpr field for
** each of the expressions in the index so that the expression code generator
** will know to replace occurrences of the indexed expression with
** references to the corresponding column of the index.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય where_Add_Indexed_પદાવલિ(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  Index *pIdx,       /* The index-on-expression that contains the expressions */
  પૂર્ણાંક iIdxCur,       /* Cursor number for pIdx */
  SrcItem *pTabItem  /* The FROM clause entry for the table */
){
  પૂર્ણાંક i;
  Indexed_પદાવલિ *p;
  Table *pTab;
  પ્રતિજ્ઞા( pIdx->b_Has_પદાવલિ );
  pTab = pIdx->pTable;
  માટે(i=0; i<pIdx->nColumn; i++){
    પદાવલિ *p_પદાવલિ;
    પૂર્ણાંક j = pIdx->aiColumn[i];
    જો( j==XN_પદાવલિ ){
      p_પદાવલિ = pIdx->a_Col_પદાવલિ->a[i].p_પદાવલિ;
    }નહીંતર જો( j>=0 && (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)!=0 ){
      p_પદાવલિ = sqlite3_Column_પદાવલિ(pTab, &pTab->aCol[j]);
    }નહીંતર{
      ચાલુ;
    }
    જો( sqlite3_પદાવલિ_Is_Constant(0,p_પદાવલિ) ) ચાલુ;
    p = sqlite3DbMallocRaw(pParse->db,  કદ(Indexed_પદાવલિ));
    જો( p==0 ) તોડો;
    p->pIENext = pParse->pIdxEpr;
#ifdef WHERETRACE_ENABLED
    જો( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      જો( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_પદાવલિ(p_પદાવલિ);
    }
#endif
    p->p_પદાવલિ = sqlite3_પદાવલિ_Dup(pParse->db, p_પદાવલિ, 0);
    p->iDataCur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું))!=0;
    જો( sqlite3IndexAffinityStr(pParse->db, pIdx) ){
      p->aff = pIdx->zColAff[i];
    }
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#endif
    pParse->pIdxEpr = p;
    જો( p->pIENext==0 ){
      શૂન્ય *pArg = (શૂન્ય*)&pParse->pIdxEpr;
      sqlite3ParserAddCleanup(pParse, where_Indexed_પદાવલિ_Cleanup, pArg);
    }
  }
}

/*
** Set the reverse-scan order mask to one for all tables in the query
** with the exception of MATERIALIZED common table expressions that have
** their own internal ORDER BY clauses.
**
** This implements the PRAGMA reverse_unordered_selects=ON setting.
** (Also SQLITE_DBCONFIG_REVERSE_SCANORDER).
*/
સ્થિર SQLITE_NOINLINE શૂન્ય whereReverseScanOrder(WhereInfo *pWInfo){
  પૂર્ણાંક ii;
  માટે(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    જો( !pItem->fg.isCte
     || pItem->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(pItem->fg.isSubquery==0)
     || pItem->u4.pSubq->pSelect->pOrderBy==0
    ){
      pWInfo->revMask |= MASKBIT(ii);
    }
  }
}

/*
** Generate the beginning of the loop used for WHERE clause processing.
** The return value is a pointer to an opaque structure that contains
** information needed to terminate the loop.  Later, the calling routine
** should invoke sqlite3WhereEnd() with the return value of this function
** in order to complete the WHERE clause processing.
**
** If an error occurs, this routine returns NULL.
**
** The basic idea is to do a nested loop, one loop for each table in
** the FROM clause of a select.  (INSERT and UPDATE statements are the
** same as a SELECT with only a single table in the FROM clause.)  For
** example, if the SQL is this:
**
**       SELECT * FROM t1, t2, t3 WHERE ...;
**
** Then the code generated is conceptually like the following:
**
**      foreach row1 in t1 do       \    Code generated
**        foreach row2 in t2 do      |-- by sqlite3WhereBegin()
**          foreach row3 in t3 do   /
**            ...
**          end                     \    Code generated
**        end                        |-- by sqlite3WhereEnd()
**      end                         /
**
** Note that the loops might not be nested in the order in which they
** appear in the FROM clause if a different order is better able to make
** use of indices.  Note also that when the IN operator appears in
** the WHERE clause, it might result in additional nested loops for
** scanning through all values on the right-hand side of the IN.
**
** There are Btree cursors associated with each table.  t1 uses cursor
** number pTabList->a[0].iCursor.  t2 uses the cursor pTabList->a[1].iCursor.
** And so forth.  This routine generates code to open those VDBE cursors
** and sqlite3WhereEnd() generates the code to close them.
**
** The code that sqlite3WhereBegin() generates leaves the cursors named
** in pTabList pointing at their appropriate entries.  The [...] code
** can use OP_Column and OP_Rowid opcodes on these cursors to extract
** data from the various tables of the loop.
**
** If the WHERE clause is empty, the foreach loops must each scan their
** entire tables.  Thus a three-way join is an O(N^3) operation.  But if
** the tables have indices and there are terms in the WHERE clause that
** refer to those indices, a complete table scan can be avoided and the
** code will run much faster.  Most of the work of this routine is checking
** to see if there are indices that can be used to speed up the loop.
**
** Terms of the WHERE clause are also used to limit which rows actually
** make it to the "..." in the middle of the loop.  After each "foreach",
** terms of the WHERE clause that use only terms in that loop and outer
** loops are evaluated and if false a jump is made around all subsequent
** inner loops (or around the "..." if the test occurs within the inner-
** most loop)
**
** OUTER JOINS
**
** An outer join of tables t1 and t2 is conceptually coded as follows:
**
**    foreach row1 in t1 do
**      flag = 0
**      foreach row2 in t2 do
**        start:
**          ...
**          flag = 1
**      end
**      if flag==0 then
**        move the row2 cursor to a null row
**        goto start
**      fi
**    end
**
** ORDER BY CLAUSE PROCESSING
**
** pOrderBy is a pointer to the ORDER BY clause (or the GROUP BY clause
** if the WHERE_GROUPBY flag is set in wctrlFlags) of a SELECT statement
** if there is one.  If there is no ORDER BY clause or if this routine
** is called from an UPDATE or DELETE statement, then pOrderBy is NULL.
**
** The iIdxCur parameter is the cursor number of an index.  If
** WHERE_OR_SUBCLAUSE is set, iIdxCur is the cursor number of an index
** to use for OR clause processing.  The WHERE clause should use this
** specific cursor.  If WHERE_ONEPASS_DESIRED is set, then iIdxCur is
** the first cursor in an array of cursors for all indices.  iIdxCur should
** be used to compute the appropriate cursor depending on which index is
** used.
*/
WhereInfo *sqlite3WhereBegin(
  Parse *pParse,          /* The parser context */
  SrcList *pTabList,      /* FROM clause: A list of all tables to be scanned */
  પદાવલિ *pWhere,           /* The WHERE clause */
  પદાવલિ_List *pOrderBy,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  પદાવલિ_List *pResultSet,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  પૂર્ણાંક iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  પૂર્ણાંક nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  પૂર્ણાંક nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  WhereMaskSet *pMaskSet;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  પૂર્ણાંક ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  પૂર્ણાંક rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  પ્રતિજ્ઞા( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  પ્રતિજ્ઞા( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_LIMIT)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, કદ(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( pOrderBy && pOrderBy->n_પદાવલિ==BMS-1 );
  જો( pOrderBy && pOrderBy->n_પદાવલિ>=BMS ){
    pOrderBy = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  જો( pTabList->nSrc>BMS ){
    sqlite3_Error_સંદેશ(pParse, "at most %d tables in a join", BMS);
    પાછા 0;
  }

  /* This function normally generates a nested loop for all tables in
  ** pTabList.  But if the WHERE_OR_SUBCLAUSE flag is set, then we should
  ** only generate code for the first table in pTabList and assume that
  ** any cursors associated with subsequent tables are uninitialized.
  */
  nTabList = (wctrlFlags & WHERE_OR_SUBCLAUSE) ? 1 : pTabList->nSrc;

  /* Allocate and initialize the WhereInfo structure that will become the
  ** return value. A single allocation is used to store the WhereInfo
  ** struct, the contents of WhereInfo.a[], the WhereClause structure
  ** and the WhereMaskSet structure. Since WhereClause contains an 8-byte
  ** field (type Bitmask) it must be aligned on an 8-byte boundary on
  ** some architectures. Hence the ROUND8() below.
  */
  nByteWInfo = SZ_WHEREINFO(nTabList);
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + કદ(WhereLoop));
  જો( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    જાઓ whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;
#જો WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#endif
  pWInfo->pResultSet = pResultSet;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->iLimit = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  memset(&pWInfo->a[0], 0, કદ(WhereLoop)+nTabList*કદ(WhereLevel));
  પ્રતિજ્ઞા( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  pMaskSet = &pWInfo->sMaskSet;
  pMaskSet->n = 0;
  pMaskSet->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((અક્ષર*)pWInfo)+nByteWInfo);
  પ્રતિજ્ઞા( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#ifdef SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#endif

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3WhereSplit(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  જો( nTabList==0 ){
    જો( pOrderBy ) pWInfo->nOBSat = pOrderBy->n_પદાવલિ;
    જો( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    જો( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_કિંમત)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }નહીંતર{
    /* Assign a bit from the bitmask to every term in the FROM clause.
    **
    ** The N-th term of the FROM clause is assigned a bitmask of 1<<N.
    **
    ** The rule of the previous sentence ensures that if X is the bitmask for
    ** a table T, then X-1 is the bitmask for all other tables to the left of T.
    ** Knowing the bitmask for all tables to the left of a left join is
    ** important.  Ticket #3015.
    **
    ** Note that bitmasks are created for all pTabList->nSrc tables in
    ** pTabList, not just the first nTabList tables.  nTabList is normally
    ** equal to pTabList->nSrc but might be shortened to 1 if the
    ** WHERE_OR_SUBCLAUSE flag is set.
    */
    ii = 0;
    કરો{
      createMask(pMaskSet, pTabList->a[ii].iCursor);
      sqlite3WhereTabFuncArgs(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }જ્યારે( (++ii)<pTabList->nSrc );
  #ifdef SQLITE_DEBUG
    {
      Bitmask mx = 0;
      માટે(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3WhereGetMask(pMaskSet, pTabList->a[ii].iCursor);
        પ્રતિજ્ઞા( m>=mx );
        mx = m;
      }
    }
  #endif
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_પદાવલિ_Analyze(pTabList, &pWInfo->sWC);
  જો( pSelect && pSelect->pLimit ){
    sqlite3WhereAddLimit(&pWInfo->sWC, pSelect);
  }
  જો( pParse->nErr ) જાઓ whereBeginError;

  /* The False-WHERE-Term-Bypass optimization:
  **
  ** If there are WHERE terms that are false, then no rows will be output,
  ** so skip over all of the code generated here.
  **
  ** Conditions:
  **
  **   (1)  The WHERE term must not refer to any tables in the join.
  **   (2)  The term must not come from an ON clause on the
  **        right-hand side of a LEFT or FULL JOIN.
  **   (3)  The term must not come from an ON clause, or there must be
  **        no RIGHT or FULL OUTER joins in pTabList.
  **   (4)  If the expression contains non-deterministic functions
  **        that are not within a sub-select. This is not required
  **        for correctness but rather to preserves SQLite's legacy
  **        behaviour in the following two cases:
  **
  **          WHERE random()>0;           -- eval random() once per row
  **          WHERE (SELECT random())>0;  -- eval random() just once overall
  **
  ** Note that the Where term need not be a constant in order for this
  ** optimization to apply, though it does need to be constant relative to
  ** the current subquery (condition 1).  The term might include variables
  ** from outer queries so that the value of the term changes from one
  ** invocation of the current subquery to the next.
  */
  માટે(ii=0; ii<sWLB.pWC->nBase; ii++){
    WhereTerm *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    પદાવલિ *pX;                          /* The expression of pT */
    જો( pT->wtFlags & TERM_VIRTUAL ) ચાલુ;
    pX = pT->p_પદાવલિ;
    પ્રતિજ્ઞા( pX!=0 );
    પ્રતિજ્ઞા( pT->prereqAll!=0 || !પદાવલિ_Has_Property(pX, EP_OuterON) );
    જો( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || પદાવલિ_Is_Deterministic(pX))   /* Condition (4) */
     && !(પદાવલિ_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_પદાવલિ_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= TERM_CODED;
    }
  }

  જો( wctrlFlags & WHERE_WANT_DISTINCT ){
    જો( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }નહીંતર જો( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, pResultSet) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }નહીંતર જો( pOrderBy==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->pOrderBy = pResultSet;
    }
  }

  /* Construct the WhereLoop objects */
#જો defined(WHERETRACE_ENABLED)
  જો( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    જો( wctrlFlags & WHERE_USE_LIMIT ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    જો( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, કદ(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.pOrderBy = pOrderBy;
      sSelect.pEList = pResultSet;
      sqlite3_વૃક્ષ_View_Select(0, &sSelect, 0);
    }
    જો( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#endif

  જો( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = whereLoopAddAll(&sWLB);
    જો( rc ) જાઓ whereBeginError;

#ifdef SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    જો( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      જ્યારે( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->pNextLoop;
        whereLoopDelete(db, p);
      }
      rc = whereLoopAddAll(&sWLB);
      જો( rc ) જાઓ whereBeginError;
    }
#endif
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    wherePathSolver(pWInfo, 0);
    જો( db->mallocFailed ) જાઓ whereBeginError;
    જો( pWInfo->pOrderBy ){
       whereInterstageHeuristic(pWInfo);
       wherePathSolver(pWInfo, pWInfo->nRowOut<0 ? 1 : pWInfo->nRowOut+1);
       જો( db->mallocFailed ) જાઓ whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    જો( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->nRowOut, pWInfo->nRowOut-30));
      pWInfo->nRowOut -= 30;
    }

  }
  પ્રતિજ્ઞા( pWInfo->pTabList!=0 );
  જો( pWInfo->pOrderBy==0 && (db->flags & SQLITE_ReverseOrder)!=0 ){
    whereReverseScanOrder(pWInfo);
  }
  જો( pParse->nErr ){
    જાઓ whereBeginError;
  }
  પ્રતિજ્ઞા( db->mallocFailed==0 );
#ifdef WHERETRACE_ENABLED
  જો( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->rTotalCost, pWInfo->nRowOut);
    જો( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->revMask);
    }
    પસંદગી( pWInfo->eDistinct ){
      સ્થિતિ WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        તોડો;
      }
      સ્થિતિ WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        તોડો;
      }
      સ્થિતિ WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        તોડો;
      }
    }
    sqlite3DebugPrintf("\n");
    માટે(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#endif

  /* Attempt to omit tables from a join that do not affect the result.
  ** See the comment on whereOmitNoopJoin() for further information.
  **
  ** This query optimization is factored out into a separate "no-inline"
  ** procedure to keep the sqlite3WhereBegin() procedure from becoming
  ** too large.  If sqlite3WhereBegin() becomes too large, that prevents
  ** some C-compiler optimizers from in-lining the
  ** sqlite3WhereCodeOneLoopStart() procedure, and it is important to
  ** in-line sqlite3WhereCodeOneLoopStart() for performance reasons.
  */
  notReady = ~(Bitmask)0;
  જો( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && pResultSet!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    પ્રતિજ્ઞા( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  જો( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#જો defined(WHERETRACE_ENABLED)
  જો( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
    sqlite3DebugPrintf("---- WHERE clause at end of analysis:\n");
    sqlite3WhereClausePrint(sWLB.pWC);
  }
  WHERETRACE(0xffffffff,("*** Optimizer Finished ***\n"));
#endif
  pWInfo->pParse->nQueryLoop += pWInfo->nRowOut;

  /* If the caller is an UPDATE or DELETE statement that is requesting
  ** to use a one-pass algorithm, determine if this is appropriate.
  **
  ** A one-pass approach can be used if the caller has requested one
  ** and either (a) the scan visits at most one row or (b) each
  ** of the following are true:
  **
  **   * the caller has indicated that a one-pass approach can be used
  **     with multiple rows (by setting WHERE_ONEPASS_MULTIROW), and
  **   * the table is not a virtual table, and
  **   * either the scan does not use the OR optimization or the caller
  **     is a DELETE operation (WHERE_DUPLICATES_OK is only specified
  **     for DELETE).
  **
  ** The last qualification is because an UPDATE statement uses
  ** WhereInfo.aiCurOnePass[1] to determine whether or not it really can
  ** use a one-pass approach, and this is not set accurately for scans
  ** that use the OR optimization.
  */
  પ્રતિજ્ઞા( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  જો( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    પૂર્ણાંક wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    પૂર્ણાંક bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    પ્રતિજ્ઞા( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    જો( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      જો( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        જો( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  માટે(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    Table *pTab;     /* Table to open */
    પૂર્ણાંક iDb;         /* Index of database containing table/index */
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    જો( ii==0 || (pTabItem[0].fg.jointype & JT_ડાબું)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }નહીંતર જો( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }નહીંતર{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    જો( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }નહીંતર
#ifndef SQLITE_OMIT_VIRTUALTABLE
    જો( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      અચલ અક્ષર *pVTab = (અચલ અક્ષર *)sqlite3GetVTable(db, pTab);
      પૂર્ણાંક iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }નહીંતર જો( IsVirtual(pTab) ){
      /* noop */
    }નહીંતર
#endif
    જો( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (pTabItem->fg.jointype & (JT_LTORJ|JT_જમણું))!=0
    ){
      પૂર્ણાંક op = OP_OpenRead;
      જો( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);
      પ્રતિજ્ઞા( pTabItem->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS );
      જો( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->nCol<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_INDEX|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = pTabItem->colUsed;
        પૂર્ણાંક n = 0;
        માટે(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_TO_PTR(n), P4_INT32);
        પ્રતિજ્ઞા( n<=pTab->nCol );
      }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
      જો( pLoop->u.btree.pIndex!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }નહીંતર
#endif
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
      sqlite3VdbeAddOp3(v, OP_ColumnsUsed, pTabItem->iCursor,
                    LOWER32(pTabItem->colUsed), UPPER32(pTabItem->colUsed));
#endif
      જો( ii>=2
       && (pTabItem[0].fg.jointype & (JT_LTORJ|JT_ડાબું))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3VdbeAddOp2(v, OP_IfEmpty, pTabItem->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }નહીંતર{
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    જો( pLoop->wsFlags & WHERE_INDEXED ){
      Index *pIx = pLoop->u.btree.pIndex;
      પૂર્ણાંક iIndexCur;
      પૂર્ણાંક op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      પ્રતિજ્ઞા( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      જો( !HasRowid(pTab) && Is_Primary_ચાવી_Index(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        iIndexCur = pLevel->iTabCur;
        op = 0;
      }નહીંતર જો( pWInfo->eOnePass!=ONEPASS_OFF ){
        Index *pJ = pTabItem->pSTab->pIndex;
        iIndexCur = iAuxArg;
        પ્રતિજ્ઞા( wctrlFlags & WHERE_ONEPASS_DESIRED );
        જ્યારે( ALWAYS(pJ) && pJ!=pIx ){
          iIndexCur++;
          pJ = pJ->pNext;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = iIndexCur;
      }નહીંતર જો( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        iIndexCur = iAuxArg;
        op = OP_ReopenIdx;
      }નહીંતર{
        iIndexCur = pParse->nTab++;
        જો( pIx->b_Has_પદાવલિ && OptimizationEnabled(db, SQLITE_Indexed_પદાવલિ) ){
          where_Add_Indexed_પદાવલિ(pParse, pIx, iIndexCur, pTabItem);
        }
        જો( pIx->pPartIdxWhere && (pTabItem->fg.jointype & JT_જમણું)==0 ){
          where_Part_Idx_પદાવલિ(
              pParse, pIx, pIx->pPartIdxWhere, 0, iIndexCur, pTabItem
          );
        }
      }
      pLevel->iIdxCur = iIndexCur;
      પ્રતિજ્ઞા( pIx!=0 );
      પ્રતિજ્ઞા( pIx->pSchema==pTab->pSchema );
      પ્રતિજ્ઞા( iIndexCur>=0 );
      જો( op ){
        sqlite3VdbeAddOp3(v, op, iIndexCur, pIx->tnum, iDb);
        sqlite3_Vdbe_Set_P4_ચાવી_Info(pParse, pIx);
        જો( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_COLUMN_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_SORT)==0
         && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
        {
          u64 colUsed = 0;
          પૂર્ણાંક ii, jj;
          માટે(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            જો( jj<0 ) ચાલુ;
            જો( jj>63 ) jj = 63;
            જો( (pTabItem->colUsed & MASKBIT(jj))==0 ) ચાલુ;
            colUsed |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3VdbeAddOp3(v, OP_ColumnsUsed, iIndexCur,
                            LOWER32(colUsed),UPPER32(colUsed));
        }
#endif /* SQLITE_ENABLE_COLUMN_USED_MASK */
#ifdef SQLITE_ENABLE_CURSOR_HINTS
        જો( HasRowid(pTab) ){
          sqlite3VdbeAddOp3(v, OP_CursorHint, iIndexCur, 0, pTabItem->iCursor);
        }
#endif
      }
    }
    જો( iDb>=0 ) sqlite3CodeVerifySchema(pParse, iDb);
    જો( (pTabItem->fg.jointype & JT_જમણું)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, કદ(Where_જમણું_Join)))!=0
    ){
      Where_જમણું_Join *pRJ = pLevel->pRJ;
      પૂર્ણાંક bBloomable = 0;
      pRJ->iMatch = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, pRJ->regReturn);
      પ્રતિજ્ઞા( pTab==pTabItem->pSTab );
      જો( HasRowid(pTab) ){
        ચાવી_Info *pInfo;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, 1);
        bBloomable = 1;
        pInfo = sqlite3_ચાવી_Info_Alloc(pParse->db, 1, 0);
        જો( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->aSortFlags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }નહીંતર{
        Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, pPk->n_ચાવી_Col);
        sqlite3_Vdbe_Set_P4_ચાવી_Info(pParse, pPk);
        bBloomable = sqlite3IndexBloomable(pPk,pPk->n_ચાવી_Col);
      }
      જો( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Blob, 65536, pRJ->regBloom);
      }નહીંતર{
        pRJ->regBloom = 0;
      }
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      /* The nature of RIGHT JOIN processing is such that it messes up
      ** the output order.  So omit any ORDER BY/GROUP BY elimination
      ** optimizations.  We need to do an actual sort for RIGHT JOIN. */
      pWInfo->nOBSat = 0;
      pWInfo->eDistinct = WHERE_DISTINCT_UNORDERED;
    }
  }
  pWInfo->i_ટોચ = sqlite3VdbeCurrentAddr(v);
  જો( db->mallocFailed ) જાઓ whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  માટે(ii=0; ii<nTabList; ii++){
    પૂર્ણાંક addrExplain;
    પૂર્ણાંક wsFlags;
    SrcItem *pSrc;
    જો( pParse->nErr ) જાઓ whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    જો( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      પૂર્ણાંક iOnce = 0;
      પ્રતિજ્ઞા( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      જો( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }નહીંતર{
        iOnce = 0;
      }
      sqlite3VdbeAddOp2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      જો( iOnce )  sqlite3VdbeJumpHere(v, iOnce);
    }
    પ્રતિજ્ઞા( pTabList == pWInfo->pTabList );
    જો( (wsFlags & (WHERE_AUTO_INDEX|WHERE_BLOOMFILTER))!=0 ){
      જો( (wsFlags & WHERE_AUTO_INDEX)!=0 ){
#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
        constructAutomaticIndex(pParse, &pWInfo->sWC, notReady, pLevel);
#endif
      }નહીંતર{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      જો( db->mallocFailed ) જાઓ whereBeginError;
    }
    addrExplain = sqlite3WhereExplainOneScan(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    જો( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3WhereAddScanStatus(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->iEndWhere = sqlite3VdbeCurrentAddr(v);
  પાછા pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  જો( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#ifdef WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#endif /* WHERETRACE_ENABLED */
  પાછા 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#ifndef SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#નહીંતર
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  સ્થિર શૂન્ય sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    પૂર્ણાંક pc,
    VdbeOp *pOp
  ){
    જો( (db->flags & SQLITE_VdbeAddopTrace)==0 ) પાછા;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3ShowWhereTerm(0); /* So compiler won't complain about unused func */
  }
#endif

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
શૂન્ય sqlite3WhereEnd(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  પૂર્ણાંક iEnd = sqlite3VdbeCurrentAddr(v);
  પૂર્ણાંક nRJ = 0;
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  પૂર્ણાંક addrSeek = 0;
#endif

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  માટે(i=pWInfo->nLevel-1; i>=0; i--){
    પૂર્ણાંક addr;
    pLevel = &pWInfo->a[i];
    જો( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_જમણું_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->endSubrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    જો( pLevel->op!=OP_Noop ){
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      Index *pIdx;
      પૂર્ણાંક n;
      જો( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_INDEXED)!=0
       && (pIdx = pLoop->u.btree.pIndex)->hasStat1
       && (n = pLoop->u.btree.nDistinctCol)>0
       && pIdx->aiRowLogEst[n]>=36
      ){
        પૂર્ણાંક r1 = pParse->nMem+1;
        પૂર્ણાંક j, op;
        પૂર્ણાંક addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        જો( pLevel->i_ડાબું_Join ){
          addrIfNull = sqlite3VdbeAddOp2(v, OP_IfNullRow, pLevel->iIdxCur, r1);
        }
        માટે(j=0; j<n; j++){
          sqlite3VdbeAddOp3(v, OP_Column, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3VdbeAddOp4Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3VdbeAddOp2(v, OP_Goto, 1, pLevel->p2);
        જો( pLevel->i_ડાબું_Join ){
          sqlite3VdbeJumpHere(v, addrIfNull);
        }
      }
#endif /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    જો( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }નહીંતર જો( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      પૂર્ણાંક iBypass = sqlite3VdbeAddOp0(v, OP_Goto);
      પ્રતિજ્ઞા( pLoop->wsFlags & WHERE_AUTO_INDEX );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3VdbeAddOp1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3VdbeJumpHere(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    જો( pLevel->op!=OP_Noop ){
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_Next);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      જો( addrSeek ){
        sqlite3VdbeJumpHere(v, addrSeek);
        addrSeek = 0;
      }
#endif
      જો( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3VdbeAddOp2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    જો( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
    પ્રતિજ્ઞા( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_IN_ABLE) && pLevel->u.in.nIn>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    જો( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3VdbeAddOp2(v, OP_DecrJumpZero, (પૂર્ણાંક)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#endif
    જો( (pLoop->wsFlags & WHERE_IN_ABLE)!=0 && pLevel->u.in.nIn>0 ){
      સંરચના InLoop *pIn;
      પૂર્ણાંક j;
      માટે(j=pLevel->u.in.nIn, pIn=&pLevel->u.in.aInLoop[j-1]; j>0; j--, pIn--){
        પ્રતિજ્ઞા( sqlite3VdbeGetOp(v, pIn->addr_In_ટોચ+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3VdbeJumpHere(v, pIn->addr_In_ટોચ+1);
        જો( pIn->eEndLoopOp!=OP_Noop ){
          જો( pIn->nPrefix ){
            પૂર્ણાંક bEarlyOut =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0;
            જો( pLevel->i_ડાબું_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3VdbeAddOp3(v, OP_IfNullRow, pIn->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + bEarlyOut, pIn->iBase);
              VdbeCoverage(v);
            }
            જો( bEarlyOut ){
              sqlite3VdbeAddOp4Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  pIn->iBase, pIn->nPrefix);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3VdbeJumpHere(v, pIn->addr_In_ટોચ+1);
            }
          }
          sqlite3VdbeAddOp2(v, pIn->eEndLoopOp, pIn->iCur, pIn->addr_In_ટોચ);
          VdbeCoverage(v);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_Prev);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_Next);
        }
        sqlite3VdbeJumpHere(v, pIn->addr_In_ટોચ-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    જો( pLevel->pRJ ){
      sqlite3VdbeAddOp3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    જો( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.pIndex->zName));
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip-2);
    }
    જો( pLevel->i_ડાબું_Join ){
      પૂર્ણાંક ws = pLoop->wsFlags;
      addr = sqlite3VdbeAddOp1(v, OP_IfPos, pLevel->i_ડાબું_Join); VdbeCoverage(v);
      પ્રતિજ્ઞા( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_INDEXED)!=0 );
      જો( (ws & WHERE_IDX_ONLY)==0 ){
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];
        પ્રતિજ્ઞા( pLevel->iTabCur==pSrc->iCursor );
        જો( pSrc->fg.viaCoroutine ){
          પૂર્ણાંક m, n;
          પ્રતિજ્ઞા( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->regResult;
          પ્રતિજ્ઞા( pSrc->pSTab!=0 );
          m = pSrc->pSTab->nCol;
          sqlite3VdbeAddOp3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iTabCur);
      }
      જો( (ws & WHERE_INDEXED)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        જો( ws & WHERE_MULTI_OR ){
          Index *pIx = pLevel->u.pCoveringIdx;
          પૂર્ણાંક iDb = sqlite3SchemaToIndex(db, pIx->pSchema);
          sqlite3VdbeAddOp3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_Set_P4_ચાવી_Info(pParse, pIx);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
      }
      જો( ws & WHERE_IN_ABLE ){
        પૂર્ણાંક m;
        માટે(m=0; m<pLevel->u.in.nIn; m++){
          સંરચના InLoop *pIn = &pLevel->u.in.aInLoop[m];
          જો( pIn->eEndLoopOp!=OP_Noop ){
            sqlite3VdbeAddOp1(v, OP_NullRow, pIn->iCur);
          }
        }
      }
      જો( pLevel->op==OP_Return ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }નહીંતર{
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  પ્રતિજ્ઞા( pWInfo->nLevel<=pTabList->nSrc );
  માટે(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    પૂર્ણાંક k, last;
    VdbeOp *pOp, *pLastOp;
    Index *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pSTab;
    પ્રતિજ્ઞા( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    જો( pLevel->pRJ ){
      sqlite3_Where_જમણું_Join_Loop(pWInfo, i, pLevel);
      ચાલુ;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    જો( pTabItem->fg.viaCoroutine ){
      testcase( pParse->db->mallocFailed );
      પ્રતિજ્ઞા( pTabItem->fg.isSubquery );
      પ્રતિજ્ઞા( pTabItem->u4.pSubq->regResult>=0 );
      translateColumnToCopy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->u4.pSubq->regResult, 0);
      ચાલુ;
    }

    /* If this scan uses an index, make VDBE code substitutions to read data
    ** from the index instead of from the table where possible.  In some cases
    ** this optimization prevents the table from ever being read, which can
    ** yield a significant performance boost.
    **
    ** Calls to the code generator in between sqlite3WhereBegin and
    ** sqlite3WhereEnd will have created code that references the table
    ** directly.  This loop scans all that code looking for opcodes
    ** that reference the table and converts them into opcodes that
    ** reference the index.
    */
    જો( pLoop->wsFlags & (WHERE_INDEXED|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.pIndex;
    }નહીંતર જો( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    જો( pIdx
     && !db->mallocFailed
    ){
      જો( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->pTable) ){
        last = iEnd;
      }નહીંતર{
        last = pWInfo->iEndWhere;
      }
      જો( pIdx->b_Has_પદાવલિ ){
        Indexed_પદાવલિ *p = pParse->pIdxEpr;
        જ્યારે( p ){
          જો( p->iIdxCur==pLevel->iIdxCur ){
#ifdef WHERETRACE_ENABLED
            જો( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->iIdxCol);
              જો( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_પદાવલિ(p->p_પદાવલિ);
            }
#endif
            p->iDataCur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#ifdef SQLITE_DEBUG
      જો( db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, last-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      પ્રતિજ્ઞા( pOp->opcode!=OP_Column || pOp->p1!=pLevel->iTabCur );
      પ્રતિજ્ઞા( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      પ્રતિજ્ઞા( pOp->opcode!=OP_IfNullRow || pOp->p1!=pLevel->iTabCur );
#endif
      pOp = sqlite3VdbeGetOp(v, k);
      pLastOp = pOp + (last - k);
      પ્રતિજ્ઞા( pOp<=pLastOp );
      કરો{
        જો( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }નહીંતર જો( pOp->opcode==OP_Column
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
         || pOp->opcode==OP_Offset
#endif
        ){
          પૂર્ણાંક x = pOp->p2;
          પ્રતિજ્ઞા( pIdx->pTable==pTab );
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
          જો( pOp->opcode==OP_Offset ){
            /* Do not need to translate the column number */
          }નહીંતર
#endif
          જો( !HasRowid(pTab) ){
            Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
            x = pPk->aiColumn[x];
            પ્રતિજ્ઞા( x>=0 );
          }નહીંતર{
            testcase( x!=sqlite3StorageColumnToTable(pTab,x) );
            x = sqlite3StorageColumnToTable(pTab,x);
          }
          x = sqlite3TableColumnToIndex(pIdx, x);
          જો( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }નહીંતર જો( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            જો( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_સંદેશ(pParse, "internal query planner error");
              pParse->rc = SQLITE_INTERNAL;
            }નહીંતર{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3WhereAddExplainText(pParse,
                  pLevel->addrBody-1,
                  pTabList,
                  pLevel,
                  pWInfo->wctrlFlags
              );
            }
          }
        }નહીંતર જો( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }નહીંતર જો( pOp->opcode==OP_IfNullRow ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#ifdef SQLITE_DEBUG
        k++;
#endif
      }જ્યારે( (++pOp)<pLastOp );
#ifdef SQLITE_DEBUG
      જો( db->flags & SQLITE_VdbeAddopTrace ) printf("TRANSLATE complete\n");
#endif
    }
  }

  /* The "break" point is here, just past the end of the outer loop.
  ** Set it.
  */
  sqlite3VdbeResolveLabel(v, pWInfo->iBreak);

  /* Final cleanup
  */
  pParse->nQueryLoop = pWInfo->savedNQueryLoop;
  whereInfoFree(db, pWInfo);
  pParse->withinRJSubrtn -= nRJ;
  પાછા;
}
