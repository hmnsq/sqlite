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
#समावेश "sqliteInt.h"
#समावेश "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
प्रकार काष्ठा Hidden_सूचकांक_Info Hidden_सूचकांक_Info;
काष्ठा Hidden_सूचकांक_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  पूर्णांक eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 mIn;                 /* Mask of terms that are <col> IN (...) */
  u32 mHandleIn;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_मूल्य *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#घोषणा SZ_HIDDENINDEXINFO(N) \
                  (दुरत्व(Hidden_सूचकांक_Info,aRhs) + (N)*माप(sqlite3_मूल्य*))

/* Forward declaration of methods */
अटल पूर्णांक whereLoopResize(sqlite3*, WhereLoop*, पूर्णांक);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_Row_गिनती(WhereInfo *pWInfo){
  वापस pWInfo->nRowOut;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
पूर्णांक sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  वापस pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
पूर्णांक sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  वापस pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
पूर्णांक sqlite3WhereOrderByLimitOptLabel(WhereInfo *pWInfo){
  WhereLevel *pInner;
  अगर( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    वापस pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  निश्चित( pInner->addrNxt!=0 );
  वापस pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
व्योम sqlite3WhereMinMaxOptEarlyOut(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  पूर्णांक i;
  अगर( !pWInfo->bOrderedInnerLoop ) वापस;
  अगर( pWInfo->nOBSat==0 ) वापस;
  क्रम(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    अगर( (pInner->pWLoop->wsFlags & WHERE_COLUMN_IN)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      वापस;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
पूर्णांक sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  निश्चित( pWInfo->iContinue!=0 );
  वापस pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
पूर्णांक sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  वापस pWInfo->iBreak;
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
पूर्णांक sqlite3WhereOkOnePass(WhereInfo *pWInfo, पूर्णांक *aiCur){
  स_नकल(aiCur, pWInfo->aiCurOnePass, माप(पूर्णांक)*2);
#अगर_घोषित WHERETRACE_ENABLED
  अगर( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#पूर्ण_अगर
  वापस pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
पूर्णांक sqlite3WhereUsesDeferredSeek(WhereInfo *pWInfo){
  वापस pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
अटल व्योम whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc){
  pDest->n = pSrc->n;
  स_नकल(pDest->a, pSrc->a, pDest->n*माप(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
अटल पूर्णांक where_Or_जोड़ो(
  WhereOrSet *pSet,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst rRun,           /* Run-cost of the new entry */
  LogEst nOut            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  क्रम(i=pSet->n, p=pSet->a; i>0; i--, p++){
    अगर( rRun<=p->rRun && (prereq & p->prereq)==prereq ){
      जाओ where_Or_जोड़ो_done;
    }
    अगर( p->rRun<=rRun && (p->prereq & prereq)==p->prereq ){
      वापस 0;
    }
  }
  अगर( pSet->n<N_OR_COST ){
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  }अन्यथा{
    p = pSet->a;
    क्रम(i=1; i<pSet->n; i++){
      अगर( p->rRun>pSet->a[i].rRun ) p = pSet->a + i;
    }
    अगर( p->rRun<=rRun ) वापस 0;
  }
where_Or_जोड़ो_done:
  p->prereq = prereq;
  p->rRun = rRun;
  अगर( p->nOut>nOut ) p->nOut = nOut;
  वापस 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, पूर्णांक iCursor){
  पूर्णांक i;
  निश्चित( pMaskSet->n<=(पूर्णांक)माप(Bitmask)*8 );
  निश्चित( pMaskSet->n>0 || pMaskSet->ix[0]<0 );
  निश्चित( iCursor>=-1 );
  अगर( pMaskSet->ix[0]==iCursor ){
    वापस 1;
  }
  क्रम(i=1; i<pMaskSet->n; i++){
    अगर( pMaskSet->ix[i]==iCursor ){
      वापस MASKBIT(i);
    }
  }
  वापस 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
व्योम *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  WhereMemBlock *pBlock;
  pBlock = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+माप(*pBlock));
  अगर( pBlock ){
    pBlock->p_अग्र = pWInfo->pMemToFree;
    pBlock->sz = nByte;
    pWInfo->pMemToFree = pBlock;
    pBlock++;
  }
  वापस (व्योम*)pBlock;
}
व्योम *sqlite3WhereRealloc(WhereInfo *pWInfo, व्योम *pOld, u64 nByte){
  व्योम *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  अगर( pNew && pOld ){
    WhereMemBlock *pOldBlk = (WhereMemBlock*)pOld;
    pOldBlk--;
    निश्चित( pOldBlk->sz<nByte );
    स_नकल(pNew, pOld, pOldBlk->sz);
  }
  वापस pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
अटल व्योम createMask(WhereMaskSet *pMaskSet, पूर्णांक iCursor){
  निश्चित( pMaskSet->n < Array_आकार(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
अटल व्यंजक *where_दायाँ_Subexpr_Is_Column(व्यंजक *p){
  p = sqlite3_व्यंजक_Skip_Collate_And_Likely(p->p_दायाँ);
  अगर( ALWAYS(p!=0) && p->op==TK_COLUMN && !व्यंजक_Has_Property(p, EP_FixedCol) ){
    वापस p;
  }
  वापस 0;
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
अटल SQLITE_NOINLINE स्थिर अक्षर *सूचकांक_In_Affinity_Ok(
  Parse *pParse, 
  WhereTerm *pTerm, 
  u8 idxaff
){
  व्यंजक *pX = pTerm->p_व्यंजक;
  व्यंजक inexpr;

  निश्चित( pTerm->eOperator & WO_IN );

  अगर( व्यंजक_Use_XSelect(pX) ){
    पूर्णांक iField = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    अगर( sqlite3_व्यंजक_Is_Vector(pX->p_बायाँ) ){
      iField = pTerm->u.x.iField - 1;
      inexpr.p_बायाँ = pX->p_बायाँ->x.pList->a[iField].p_व्यंजक;
    }अन्यथा{
      inexpr.p_बायाँ = pX->p_बायाँ;
    }
    inexpr.p_दायाँ = pX->x.pSelect->pEList->a[iField].p_व्यंजक;
    pX = &inexpr;
  }

  अगर( sqlite3_सूचकांक_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pX);
    वापस pRet ? pRet->zName : sqlite3_Str_द्विआधारी;
  }
  वापस 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
अटल WhereTerm *where_Scan_अग्र(WhereScan *pScan){
  पूर्णांक iCur;            /* The cursor on the LHS of the term */
  i16 iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  व्यंजक *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  WhereTerm *pTerm;    /* The term being tested */
  पूर्णांक k = pScan->k;    /* Where to start scanning */

  निश्चित( pScan->iEquiv<=pScan->nEquiv );
  pWC = pScan->pWC;
  जबतक(1){
    iColumn = pScan->aiColumn[pScan->iEquiv-1];
    iCur = pScan->aiCur[pScan->iEquiv-1];
    निश्चित( pWC!=0 );
    निश्चित( iCur>=0 );
    करो{
      क्रम(pTerm=pWC->a+k; k<pWC->nTerm; k++, pTerm++){
        निश्चित( (pTerm->eOperator & (WO_OR|WO_AND))==0 || pTerm->बायाँ_Cursor<0 );
        अगर( pTerm->बायाँ_Cursor==iCur
         && pTerm->u.x.बायाँ_Column==iColumn
         && (iColumn!=XN_व्यंजक
             || sqlite3_व्यंजक_Compare_Skip(pTerm->p_व्यंजक->p_बायाँ,
                                       pScan->p_Idx_व्यंजक,iCur)==0)
         && (pScan->iEquiv<=1 || !व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_OuterON))
        ){
          अगर( (pTerm->eOperator & WO_EQUIV)!=0
           && pScan->nEquiv<Array_आकार(pScan->aiCur)
           && (pX = where_दायाँ_Subexpr_Is_Column(pTerm->p_व्यंजक))!=0
          ){
            पूर्णांक j;
            क्रम(j=0; j<pScan->nEquiv; j++){
              अगर( pScan->aiCur[j]==pX->iTable
               && pScan->aiColumn[j]==pX->iColumn ){
                  अवरोध;
              }
            }
            अगर( j==pScan->nEquiv ){
              pScan->aiCur[j] = pX->iTable;
              pScan->aiColumn[j] = pX->iColumn;
              pScan->nEquiv++;
            }
          }
          अगर( (pTerm->eOperator & pScan->opMask)!=0 ){
            /* Verify the affinity and collating sequence match */
            अगर( pScan->zCollName && (pTerm->eOperator & WO_ISNULL)==0 ){
              स्थिर अक्षर *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->p_व्यंजक;

              अगर( (pTerm->eOperator & WO_IN) ){
                zCollName = सूचकांक_In_Affinity_Ok(pParse, pTerm, pScan->idxaff);
                अगर( !zCollName ) जारी;
              }अन्यथा{
                CollSeq *pColl;
                अगर( !sqlite3_सूचकांक_Affinity_Ok(pX, pScan->idxaff) ){
                  जारी;
                }
                निश्चित(pX->p_बायाँ);
                pColl = sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_द्विआधारी;
              }

              अगर( sqlite3StrICmp(zCollName, pScan->zCollName) ){
                जारी;
              }
            }
            अगर( (pTerm->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = pTerm->p_व्यंजक->p_दायाँ, ALWAYS(pX!=0))
             && pX->op==TK_COLUMN
             && pX->iTable==pScan->aiCur[0]
             && pX->iColumn==pScan->aiColumn[0]
            ){
              testcase( pTerm->eOperator & WO_IS );
              जारी;
            }
            pScan->pWC = pWC;
            pScan->k = k+1;
#अगर_घोषित WHERETRACE_ENABLED
            अगर( (sqlite3WhereTrace & 0x20000)!=0 && pScan->nEquiv>1 ){
              पूर्णांक ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 pScan->aiCur[0], pScan->aiColumn[0], pTerm->iTerm);
              क्रम(ii=1; ii<pScan->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   pScan->aiCur[ii], pScan->aiColumn[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#पूर्ण_अगर
            वापस pTerm;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }जबतक( pWC!=0 );
    अगर( pScan->iEquiv>=pScan->nEquiv ) अवरोध;
    pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv++;
  }
  वापस 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
अटल SQLITE_NOINLINE WhereTerm *where_Scan_Init_सूचकांक_व्यंजक(WhereScan *pScan){
  pScan->idxaff = sqlite3_व्यंजक_Affinity(pScan->p_Idx_व्यंजक);
  वापस where_Scan_अग्र(pScan);
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
अटल WhereTerm *whereScanInit(
  WhereScan *pScan,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  पूर्णांक iCur,               /* Cursor to scan for */
  पूर्णांक iColumn,            /* Column to scan for */
  u32 opMask,             /* Operator(s) to scan for */
  सूचकांक *pIdx             /* Must be compatible with this index */
){
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  pScan->p_Idx_व्यंजक = 0;
  pScan->idxaff = 0;
  pScan->zCollName = 0;
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aiCur[0] = iCur;
  pScan->nEquiv = 1;
  pScan->iEquiv = 1;
  अगर( pIdx ){
    पूर्णांक j = iColumn;
    iColumn = pIdx->aiColumn[j];
    अगर( iColumn==pIdx->pTable->iPKey ){
      iColumn = XN_ROWID;
    }अन्यथा अगर( iColumn>=0 ){
      pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
      pScan->zCollName = pIdx->azColl[j];
    }अन्यथा अगर( iColumn==XN_व्यंजक ){
      pScan->p_Idx_व्यंजक = pIdx->a_Col_व्यंजक->a[j].p_व्यंजक;
      pScan->zCollName = pIdx->azColl[j];
      pScan->aiColumn[0] = XN_व्यंजक;
      वापस where_Scan_Init_सूचकांक_व्यंजक(pScan);
    }
  }अन्यथा अगर( iColumn==XN_व्यंजक ){
    वापस 0;
  }
  pScan->aiColumn[0] = iColumn;
  वापस where_Scan_अग्र(pScan);
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
  पूर्णांक iCur,             /* Cursor number of LHS */
  पूर्णांक iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  सूचकांक *pIdx           /* Must be compatible with this index, if not NULL */
){
  WhereTerm *p_परिणाम = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  जबतक( p ){
    अगर( (p->prereq_दायाँ & notReady)==0 ){
      अगर( p->prereq_दायाँ==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        वापस p;
      }
      अगर( p_परिणाम==0 ) p_परिणाम = p;
    }
    p = where_Scan_अग्र(&scan);
  }
  वापस p_परिणाम;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
अटल पूर्णांक find_सूचकांक_Col(
  Parse *pParse,                  /* Parse context */
  व्यंजक_List *pList,                /* Expression list to search */
  पूर्णांक iBase,                      /* Cursor for table associated with pIdx */
  सूचकांक *pIdx,                    /* Index to match column of */
  पूर्णांक iCol                        /* Column of index to match */
){
  पूर्णांक i;
  स्थिर अक्षर *zColl = pIdx->azColl[iCol];

  क्रम(i=0; i<pList->n_व्यंजक; i++){
    व्यंजक *p = sqlite3_व्यंजक_Skip_Collate_And_Likely(pList->a[i].p_व्यंजक);
    अगर( ALWAYS(p!=0)
     && (p->op==TK_COLUMN || p->op==TK_AGG_COLUMN)
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->iTable==iBase
    ){
      CollSeq *pColl = sqlite3_व्यंजक_NNColl_Seq(pParse, pList->a[i].p_व्यंजक);
      अगर( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        वापस i;
      }
    }
  }

  वापस -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
अटल पूर्णांक सूचकांक_Column_Not_Null(सूचकांक *pIdx, पूर्णांक iCol){
  पूर्णांक j;
  निश्चित( pIdx!=0 );
  निश्चित( iCol>=0 && iCol<pIdx->nColumn );
  j = pIdx->aiColumn[iCol];
  अगर( j>=0 ){
    वापस pIdx->pTable->aCol[j].notNull;
  }अन्यथा अगर( j==(-1) ){
    वापस 1;
  }अन्यथा{
    निश्चित( j==(-2) );
    वापस 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
अटल पूर्णांक isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  व्यंजक_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  Table *pTab;
  सूचकांक *pIdx;
  पूर्णांक i;                         
  पूर्णांक iBase;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  अगर( pTabList->nSrc!=1 ) वापस 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  क्रम(i=0; i<pDistinct->n_व्यंजक; i++){
    व्यंजक *p = sqlite3_व्यंजक_Skip_Collate_And_Likely(pDistinct->a[i].p_व्यंजक);
    अगर( NEVER(p==0) ) जारी;
    अगर( p->op!=TK_COLUMN && p->op!=TK_AGG_COLUMN ) जारी;
    अगर( p->iTable==iBase && p->iColumn<0 ) वापस 1;
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
  क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
    अगर( !Is_Unique_सूचकांक(pIdx) ) जारी;
    अगर( pIdx->pPartIdxWhere ) जारी;
    क्रम(i=0; i<pIdx->n_कुंजी_Col; i++){
      अगर( 0==sqlite3WhereFindTerm(pWC, iBase, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        अगर( find_सूचकांक_Col(pParse, pDistinct, iBase, pIdx, i)<0 ) अवरोध;
        अगर( सूचकांक_Column_Not_Null(pIdx, i)==0 ) अवरोध;
      }
    }
    अगर( i==pIdx->n_कुंजी_Col ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      वापस 1;
    }
  }

  वापस 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
अटल LogEst estLog(LogEst N){
  वापस N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
अटल व्योम translateColumnToCopy(
  Parse *pParse,      /* Parsing context */
  पूर्णांक iStart,         /* Translate from this opcode to the end */
  पूर्णांक iTabCur,        /* OP_Column/OP_Rowid references to this table */
  पूर्णांक iRegister,      /* The first column is in this register */
  पूर्णांक iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  पूर्णांक iEnd = sqlite3VdbeCurrentAddr(v);
  अगर( pParse->db->mallocFailed ) वापस;
#अगर_घोषित SQLITE_DEBUG
  अगर( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    म_लिखो("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, iStart, iEnd);
  }
#पूर्ण_अगर
  क्रम(; iStart<iEnd; iStart++, pOp++){
    अगर( pOp->p1!=iTabCur ) जारी;
    अगर( pOp->opcode==OP_Column ){
#अगर_घोषित SQLITE_DEBUG
      अगर( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        म_लिखो("TRANSLATE OP_Column to OP_Copy at %d\n", iStart);
      }
#पूर्ण_अगर
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }अन्यथा अगर( pOp->opcode==OP_Rowid ){
#अगर_घोषित SQLITE_DEBUG
      अगर( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        म_लिखो("TRANSLATE OP_Rowid to OP_Sequence at %d\n", iStart);
      }
#पूर्ण_अगर
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
      अगर( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#पूर्ण_अगर
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#अगर !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
अटल व्योम where_Trace_सूचकांक_Info_Inputs(
  sqlite3_सूचकांक_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  पूर्णांक i;
  अगर( (sqlite3WhereTrace & 0x10)==0 ) वापस;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  क्रम(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].iTermOffset,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  क्रम(i=0; i<p->nOrderBy; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->aOrderBy[i].iColumn,
       p->aOrderBy[i].desc);
  }
}
अटल व्योम where_Trace_सूचकांक_Info_Outputs(
  sqlite3_सूचकांक_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  पूर्णांक i;
  अगर( (sqlite3WhereTrace & 0x10)==0 ) वापस;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  क्रम(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_सूचकांक,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_अंक);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->orderByConsumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimatedRows);
}
#अन्यथा
#घोषणा where_Trace_सूचकांक_Info_Inputs(A,B)
#घोषणा where_Trace_सूचकांक_Info_Outputs(A,B)
#पूर्ण_अगर

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
अटल पूर्णांक constraintCompatibleWithOuterJoin(
  स्थिर WhereTerm *pTerm,       /* WHERE clause term to check */
  स्थिर SrcItem *pSrc           /* Table we are trying to access */
){
  निश्चित( (pSrc->fg.jointype&(JT_बायाँ|JT_LTORJ|JT_दायाँ))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_बायाँ|JT_LTORJ|JT_दायाँ))==JT_बायाँ );
  testcase( (pSrc->fg.jointype & (JT_बायाँ|JT_LTORJ|JT_दायाँ))==JT_LTORJ );
  testcase( व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_OuterON) )
  testcase( व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_InnerON) );
  अगर( !व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_OuterON|EP_InnerON)
   || pTerm->p_व्यंजक->w.iJoin != pSrc->iCursor
  ){
    वापस 0;
  }
  अगर( (pSrc->fg.jointype & (JT_बायाँ|JT_दायाँ))!=0
   && NEVER(व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_InnerON))
  ){
    वापस 0;
  }
  वापस 1;
}

#अगर_अघोषित SQLITE_OMIT_AUTOMATIC_सूचकांक
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
अटल SQLITE_NOINLINE पूर्णांक column_Is_Good_सूचकांक_Candidate(
  स्थिर Table *pTab,
  पूर्णांक iCol
){
  स्थिर सूचकांक *pIdx;
  क्रम(pIdx = pTab->p_सूचकांक; pIdx!=0; pIdx=pIdx->p_अग्र){
    पूर्णांक j;
    क्रम(j=0; j<pIdx->n_कुंजी_Col; j++){
       अगर( pIdx->aiColumn[j]==iCol ){
         अगर( j==0 ) वापस 0;
         अगर( pIdx->hasStat1 && pIdx->aiRowLogEst[j+1]>20 ) वापस 0;
         अवरोध;
       }
    }
  }
  वापस 1;
}
#पूर्ण_अगर /* SQLITE_OMIT_AUTOMATIC_INDEX */



#अगर_अघोषित SQLITE_OMIT_AUTOMATIC_सूचकांक
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
अटल पूर्णांक term_Can_Drive_सूचकांक(
  स्थिर WhereTerm *pTerm,        /* WHERE clause term to check */
  स्थिर SrcItem *pSrc,           /* Table we are trying to access */
  स्थिर Bitmask notReady         /* Tables in outer loops of the join */
){
  अक्षर aff;
  पूर्णांक बायाँ_Col;
  
  अगर( pTerm->बायाँ_Cursor!=pSrc->iCursor ) वापस 0;
  अगर( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) वापस 0;
  निश्चित( (pSrc->fg.jointype & JT_दायाँ)==0 );
  अगर( (pSrc->fg.jointype & (JT_बायाँ|JT_LTORJ|JT_दायाँ))!=0
   && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
  ){
    वापस 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  अगर( (pTerm->prereq_दायाँ & notReady)!=0 ) वापस 0;
  निश्चित( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
  बायाँ_Col = pTerm->u.x.बायाँ_Column;
  अगर( बायाँ_Col<0 ) वापस 0;
  aff = pSrc->pSTab->aCol[बायाँ_Col].affinity;
  अगर( !sqlite3_सूचकांक_Affinity_Ok(pTerm->p_व्यंजक, aff) ) वापस 0;
  testcase( pTerm->p_व्यंजक->op==TK_IS );
  वापस column_Is_Good_सूचकांक_Candidate(pSrc->pSTab, बायाँ_Col);
}
#पूर्ण_अगर


#अगर_अघोषित SQLITE_OMIT_AUTOMATIC_सूचकांक

#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
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
अटल व्योम explain_Automatic_सूचकांक(
  Parse *pParse,
  सूचकांक *pIdx,                    /* Automatic index to explain */
  पूर्णांक bPartial,                   /* True if pIdx is a partial index */
  पूर्णांक *pAddrExplain               /* OUT: Address of OP_Explain */
){
  अगर( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    Table *pTab = pIdx->pTable;
    स्थिर अक्षर *zSep = "";
    अक्षर *zText = 0;
    पूर्णांक ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    निश्चित( pIdx->nColumn>1 );
    निश्चित( pIdx->aiColumn[pIdx->nColumn-1]==XN_ROWID || !HasRowid(pTab) );
    क्रम(ii=0; ii<(pIdx->nColumn-1); ii++){
      स्थिर अक्षर *zName = 0;
      पूर्णांक iCol = pIdx->aiColumn[ii];

      zName = pTab->aCol[iCol].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    zText = sqlite3_str_finish(pStr);
    अगर( zText==0 ){
      sqlite3OomFault(pParse->db);
    }अन्यथा{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", zText, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(zText);
    }
  }
}
#अन्यथा
# define explain_Automatic_सूचकांक(a,b,c,d)
#पूर्ण_अगर

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
अटल SQLITE_NOINLINE व्योम construct_Automatic_सूचकांक(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  स्थिर Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  पूर्णांक n_कुंजी_Col;                /* Number of columns in the constructed index */
  WhereTerm *pTerm;           /* A single term of the WHERE clause */
  WhereTerm *pWCEnd;          /* End of pWC->a[] */
  सूचकांक *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  पूर्णांक addrInit;               /* Address of the initialization bypass jump */
  Table *pTable;              /* The table being indexed */
  पूर्णांक addr_शीर्ष;                /* Top of the index fill loop */
  पूर्णांक regRecord;              /* Register holding an index record */
  पूर्णांक n;                      /* Column counter */
  पूर्णांक i;                      /* Loop counter */
  पूर्णांक mxBitCol;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  अक्षर *zNotUsed;             /* Extra space on the end of pIdx */
  Bitmask idxCols;            /* Bitmap of columns used for indexing */
  Bitmask extraCols;          /* Bitmap of additional columns */
  u8 sentWarning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  व्यंजक *pPartial = 0;         /* Partial Index Expression */
  पूर्णांक iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  SrcItem *pSrc;              /* The FROM clause term to get the next index */
  पूर्णांक addr_गणक = 0;        /* Address where integer counter is initialized */
  पूर्णांक regBase;                /* Array of registers where record is assembled */
#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णांक addrExp = 0;            /* Address of OP_Explain */
#पूर्ण_अगर

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  निश्चित( v!=0 );
  addrInit = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_कुंजी_Col = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  pTable = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  क्रम(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    व्यंजक *p_व्यंजक = pTerm->p_व्यंजक;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    अगर( (pTerm->wtFlags & TERM_VIRTUAL)==0
     && sqlite3_व्यंजक_Is_Single_Table_Constraint(p_व्यंजक, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_व्यंजक_And(pParse, pPartial,
                                sqlite3_व्यंजक_Dup(pParse->db, p_व्यंजक, 0));
    }
    अगर( term_Can_Drive_सूचकांक(pTerm, pSrc, notReady) ){
      पूर्णांक iCol;
      Bitmask cMask;
      निश्चित( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.बायाँ_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      अगर( !sentWarning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", pTable->zName,
            pTable->aCol[iCol].zCnName);
        sentWarning = 1;
      }
      अगर( (idxCols & cMask)==0 ){
        अगर( whereLoopResize(pParse->db, pLoop, n_कुंजी_Col+1) ){
          जाओ end_auto_सूचकांक_create;
        }
        pLoop->aLTerm[n_कुंजी_Col++] = pTerm;
        idxCols |= cMask;
      }
    }
  }
  निश्चित( n_कुंजी_Col>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_कुंजी_Col;
  pLoop->wsFlags = WHERE_COLUMN_EQ | WHERE_IDX_ONLY | WHERE_INDEXED
                     | WHERE_AUTO_सूचकांक;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  अगर( IsView(pTable) ){
    extraCols = ALLBITS & ~idxCols;
  }अन्यथा{
    extraCols = pSrc->colUsed & (~idxCols | MASKBIT(BMS-1));
  }
  अगर( !HasRowid(pTable) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    क्रम(i=0; i<pTable->nCol; i++){
      अगर( (pTable->aCol[i].colFlags & COLFLAG_PRIMKEY)==0 ) जारी;
      अगर( i>=BMS-1 ){
        extraCols |= MASKBIT(BMS-1);
        अवरोध;
      }
      अगर( idxCols & MASKBIT(i) ) जारी;
      extraCols |= MASKBIT(i);
    }
  }
  mxBitCol = MIN(BMS-1,pTable->nCol);
  testcase( pTable->nCol==BMS-1 );
  testcase( pTable->nCol==BMS-2 );
  क्रम(i=0; i<mxBitCol; i++){
    अगर( extraCols & MASKBIT(i) ) n_कुंजी_Col++;
  }
  अगर( pSrc->colUsed & MASKBIT(BMS-1) ){
    n_कुंजी_Col += pTable->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  निश्चित( n_कुंजी_Col <= pTable->nCol + MAX(0, pTable->nCol - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_सूचकांक_Object(pParse->db, n_कुंजी_Col+HasRowid(pTable),
                                    0, &zNotUsed);
  अगर( pIdx==0 ) जाओ end_auto_सूचकांक_create;
  pLoop->u.btree.p_सूचकांक = pIdx;
  pIdx->zName = "auto-index";
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  क्रम(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    अगर( term_Can_Drive_सूचकांक(pTerm, pSrc, notReady) ){
      पूर्णांक iCol;
      Bitmask cMask;
      निश्चित( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.बायाँ_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      अगर( (idxCols & cMask)==0 ){
        व्यंजक *pX = pTerm->p_व्यंजक;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.x.बायाँ_Column;
        pColl = sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pX);
        निश्चित( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        अगर( !sqlite3_Is_द्विआधारी(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_द्विआधारी;
        n++;
      }
    }
  }
  निश्चित( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  क्रम(i=0; i<mxBitCol; i++){
    अगर( extraCols & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_द्विआधारी;
      n++;
    }
  }
  अगर( pSrc->colUsed & MASKBIT(BMS-1) ){
    क्रम(i=BMS-1; i<pTable->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_द्विआधारी;
      n++;
    }
  }
  निश्चित( n==n_कुंजी_Col );
  अगर( HasRowid(pTable) ){
    pIdx->aiColumn[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_द्विआधारी;
  }

  /* Create the automatic index */
  explain_Automatic_सूचकांक(pParse, pIdx, pPartial!=0, &addrExp);
  निश्चित( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_कुंजी_Col+1);
  sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIdx);
  VdbeComment((v, "for %s", pTable->zName));
  अगर( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  निश्चित( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  अगर( pSrc->fg.viaCoroutine ){
    पूर्णांक regYield;
    Subquery *pSubq;
    निश्चित( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    निश्चित( pSubq!=0 );
    regYield = pSubq->regReturn;
    addr_गणक = sqlite3VdbeAddOp2(v, OP_Integer, 0, 0);
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_शीर्ष =  sqlite3VdbeAddOp1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }अन्यथा{
    निश्चित( pLevel->addrHalt );
    addr_शीर्ष = sqlite3VdbeAddOp2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  अगर( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_व्यंजक_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  regRecord = sqlite3_Get_अस्थायी_Reg(pParse);
  regBase = sqlite3_Generate_सूचकांक_कुंजी(
      pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0
  );
  अगर( pLevel->regFilter ){
    निश्चित( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0,
                         regBase, pLoop->u.btree.nEq);
  }
  sqlite3VdbeScanStatusCounters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3VdbeAddOp2(v, OP_Idx_जोड़ो, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  अगर( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  अगर( pSrc->fg.viaCoroutine ){
    निश्चित( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addr_गणक, regBase+n);
    testcase( pParse->db->mallocFailed );
    निश्चित( pLevel->iIdxCur>0 );
    translateColumnToCopy(pParse, addr_शीर्ष, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_परिणाम, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_शीर्ष);
    pSrc->fg.viaCoroutine = 0;
    sqlite3VdbeJumpHere(v, addr_शीर्ष);
  }अन्यथा{
    sqlite3VdbeAddOp2(v, OP_अग्र, pLevel->iTabCur, addr_शीर्ष+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    अगर( (pSrc->fg.jointype & JT_बायाँ)!=0 ){
      sqlite3VdbeJumpHere(v, addr_शीर्ष);
    }
  }
  sqlite3_Release_अस्थायी_Reg(pParse, regRecord);
 
  /* Jump here when skipping the initialization */
  sqlite3VdbeJumpHere(v, addrInit);
  sqlite3VdbeScanStatusRange(v, addrExp, addrExp, -1);

end_auto_सूचकांक_create:
  sqlite3_व्यंजक_Delete(pParse->db, pPartial);
}
#पूर्ण_अगर /* SQLITE_OMIT_AUTOMATIC_INDEX */

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
अटल SQLITE_NOINLINE व्योम sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  पूर्णांक iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  पूर्णांक addrOnce;                        /* Address of opening OP_Once */
  पूर्णांक addr_शीर्ष;                         /* Address of OP_Rewind */
  पूर्णांक addrCont;                        /* Jump here to skip a row */
  स्थिर WhereTerm *pTerm;              /* For looping over WHERE clause terms */
  स्थिर WhereTerm *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  पूर्णांक iCur;                            /* Cursor for table getting the filter */
  Indexed_व्यंजक *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  Indexed_व्यंजक *saved_p_Idx_Part_व्यंजक;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_व्यंजक = pParse->p_Idx_Part_व्यंजक;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_व्यंजक = 0;

  निश्चित( pLoop!=0 );
  निश्चित( v!=0 );
  निश्चित( pLoop->wsFlags & WHERE_BLOOMFILTER );
  निश्चित( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  करो{
    स्थिर SrcList *pTabList;
    स्थिर SrcItem *pItem;
    स्थिर Table *pTab;
    u64 sz;
    पूर्णांक iSrc;
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
    निश्चित( pItem!=0 );
    pTab = pItem->pSTab;
    निश्चित( pTab!=0 );
    sz = sqlite3LogEstToInt(pTab->nRowLogEst);
    अगर( sz<10000 ){
      sz = 10000;
    }अन्यथा अगर( sz>10000000 ){
      sz = 10000000;
    }
    निश्चित( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp2(v, OP_Blob, (पूर्णांक)sz, pLevel->regFilter);

    addr_शीर्ष = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.nTerm];
    क्रम(pTerm=pWInfo->sWC.a; pTerm<pWCEnd; pTerm++){
      व्यंजक *p_व्यंजक = pTerm->p_व्यंजक;
      अगर( (pTerm->wtFlags & TERM_VIRTUAL)==0
       && sqlite3_व्यंजक_Is_Single_Table_Constraint(p_व्यंजक, pTabList, iSrc, 0)
      ){
        sqlite3_व्यंजक_If_False(pParse, pTerm->p_व्यंजक, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    अगर( pLoop->wsFlags & WHERE_IPK ){
      पूर्णांक r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, r1);
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
    }अन्यथा{
      सूचकांक *pIdx = pLoop->u.btree.p_सूचकांक;
      पूर्णांक n = pLoop->u.btree.nEq;
      पूर्णांक r1 = sqlite3_Get_अस्थायी_Range(pParse, n);
      पूर्णांक jj;
      निश्चित( pIdx!=0 );
      क्रम(jj=0; jj<n; jj++){
        निश्चित( pIdx->pTable==pItem->pSTab );
        sqlite3_व्यंजक_Code_Load_सूचकांक_Column(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_अस्थायी_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3VdbeAddOp2(v, OP_अग्र, pLevel->iTabCur, addr_शीर्ष+1);
    VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr_शीर्ष);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    जबतक( ++iLevel < pWInfo->nLevel ){
      स्थिर SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      अगर( pTabItem->fg.jointype & (JT_बायाँ|JT_LTORJ) ) जारी;
      pLoop = pLevel->pWLoop;
      अगर( NEVER(pLoop==0) ) जारी;
      अगर( pLoop->prereq & notReady ) जारी;
      अगर( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_COLUMN_IN))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        अवरोध;
      }
    }
  }जबतक( iLevel < pWInfo->nLevel );
  sqlite3VdbeJumpHere(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_व्यंजक = saved_p_Idx_Part_व्यंजक;
}


#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
अटल WhereTerm *termFromWhereClause(WhereClause *pWC, पूर्णांक iTerm){
  WhereClause *p;
  क्रम(p=pWC; p; p=p->pOuter){
    अगर( iTerm<p->nTerm ) वापस &p->a[iTerm];
    iTerm -= p->nTerm;
  }
  वापस 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
अटल sqlite3_सूचकांक_info *allocate_सूचकांक_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  SrcItem *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  पूर्णांक i, j;
  पूर्णांक nTerm;
  Parse *pParse = pWInfo->pParse;
  काष्ठा sqlite3_सूचकांक_constraint *pIdxCons;
  काष्ठा sqlite3_सूचकांक_orderby *pIdxOrderBy;
  काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage;
  काष्ठा Hidden_सूचकांक_Info *pHidden;
  WhereTerm *pTerm;
  पूर्णांक nOrderBy;
  sqlite3_सूचकांक_info *pIdxInfo;
  u16 mNoOmit = 0;
  स्थिर Table *pTab;
  पूर्णांक eDistinct = 0;
  व्यंजक_List *pOrderBy = pWInfo->pOrderBy;
  WhereClause *p;

  निश्चित( pSrc!=0 );
  pTab = pSrc->pSTab;
  निश्चित( pTab!=0 );
  निश्चित( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  क्रम(p=pWC, nTerm=0; p; p=p->pOuter){
    क्रम(i=0, pTerm=p->a; i<p->nTerm; i++, pTerm++){
      pTerm->wtFlags &= ~TERM_OK;
      अगर( pTerm->बायाँ_Cursor != pSrc->iCursor ) जारी;
      अगर( pTerm->prereq_दायाँ & mUnusable ) जारी;
      निश्चित( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
      testcase( pTerm->eOperator & WO_IN );
      testcase( pTerm->eOperator & WO_ISNULL );
      testcase( pTerm->eOperator & WO_IS );
      testcase( pTerm->eOperator & WO_ALL );
      अगर( (pTerm->eOperator & ~(WO_EQUIV))==0 ) जारी;
      अगर( pTerm->wtFlags & TERM_VNULL ) जारी;

      निश्चित( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      निश्चित( pTerm->u.x.बायाँ_Column>=XN_ROWID );
      निश्चित( pTerm->u.x.बायाँ_Column<pTab->nCol );
      अगर( (pSrc->fg.jointype & (JT_बायाँ|JT_LTORJ|JT_दायाँ))!=0
          && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
        ){
        जारी;
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
  अगर( pOrderBy ){
    पूर्णांक n = pOrderBy->n_व्यंजक;
    क्रम(i=0; i<n; i++){
      व्यंजक *p_व्यंजक = pOrderBy->a[i].p_व्यंजक;
      व्यंजक *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      अगर( sqlite3_व्यंजक_Is_Constant(0, p_व्यंजक) ){
        जारी;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      अगर( pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) अवरोध;

      /* First case - a direct column references without a COLLATE operator */
      अगर( p_व्यंजक->op==TK_COLUMN && p_व्यंजक->iTable==pSrc->iCursor ){
        निश्चित( p_व्यंजक->iColumn>=XN_ROWID && p_व्यंजक->iColumn<pTab->nCol );
        जारी;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      अगर( p_व्यंजक->op==TK_COLLATE
       && (pE2 = p_व्यंजक->p_बायाँ)->op==TK_COLUMN
       && pE2->iTable==pSrc->iCursor
      ){
        स्थिर अक्षर *zColl;  /* The collating sequence name */
        निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
        निश्चित( p_व्यंजक->u.zToken!=0 );
        निश्चित( pE2->iColumn>=XN_ROWID && pE2->iColumn<pTab->nCol );
        p_व्यंजक->iColumn = pE2->iColumn;
        अगर( pE2->iColumn<0 ) जारी;  /* Collseq does not matter for rowid */
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        अगर( zColl==0 ) zColl = sqlite3_Str_द्विआधारी;
        अगर( sqlite3_stricmp(p_व्यंजक->u.zToken, zColl)==0 ) जारी;
      }

      /* No matches cause a break out of the loop */
      अवरोध;
    }
    अगर( i==n ){
      पूर्णांक bSortByGroup = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      nOrderBy = n;
      अगर( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowidUsed ){
        eDistinct = 2 + bSortByGroup;
      }अन्यथा अगर( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - bSortByGroup;
      }अन्यथा अगर( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, माप(*pIdxInfo)
                           + (माप(*pIdxCons) + माप(*pUsage))*nTerm
                           + माप(*pIdxOrderBy)*nOrderBy
                           + SZ_HIDDENINDEXINFO(nTerm) );
  अगर( pIdxInfo==0 ){
    sqlite3_Error_संदेश(pParse, "out of memory");
    वापस 0;
  }
  pHidden = (काष्ठा Hidden_सूचकांक_Info*)&pIdxInfo[1];
  pIdxCons = (काष्ठा sqlite3_सूचकांक_constraint*)&pHidden->aRhs[nTerm];
  pIdxOrderBy = (काष्ठा sqlite3_सूचकांक_orderby*)&pIdxCons[nTerm];
  pUsage = (काष्ठा sqlite3_सूचकांक_constraint_usage*)&pIdxOrderBy[nOrderBy];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->aOrderBy = pIdxOrderBy;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->colUsed = (sqlite3_int64)pSrc->colUsed;
  अगर( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक((Table*)pTab);
    निश्चित( pPk!=0 );
    क्रम(i=0; i<pPk->n_कुंजी_Col; i++){
      पूर्णांक iCol = pPk->aiColumn[i];
      निश्चित( iCol>=0 );
      अगर( iCol>=BMS-1 ) iCol = BMS-1;
      pIdxInfo->colUsed |= MASKBIT(iCol);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->mIn = 0;
  क्रम(p=pWC, i=j=0; p; p=p->pOuter){
    पूर्णांक nLast = i+p->nTerm;;
    क्रम(pTerm=p->a; i<nLast; i++, pTerm++){
      u16 op;
      अगर( (pTerm->wtFlags & TERM_OK)==0 ) जारी;
      pIdxCons[j].iColumn = pTerm->u.x.बायाँ_Column;
      pIdxCons[j].iTermOffset = i;
      op = pTerm->eOperator & WO_ALL;
      अगर( op==WO_IN ){
        अगर( (pTerm->wtFlags & TERM_SLICE)==0 ){
          pHidden->mIn |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      अगर( op==WO_AUX ){
        pIdxCons[j].op = pTerm->eMatchOp;
      }अन्यथा अगर( op & (WO_ISNULL|WO_IS) ){
        अगर( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_सूचकांक_CONSTRAINT_ISNULL;
        }अन्यथा{
          pIdxCons[j].op = SQLITE_सूचकांक_CONSTRAINT_IS;
        }
      }अन्यथा{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        निश्चित( WO_EQ==SQLITE_सूचकांक_CONSTRAINT_EQ );
        निश्चित( WO_LT==SQLITE_सूचकांक_CONSTRAINT_LT );
        निश्चित( WO_LE==SQLITE_सूचकांक_CONSTRAINT_LE );
        निश्चित( WO_GT==SQLITE_सूचकांक_CONSTRAINT_GT );
        निश्चित( WO_GE==SQLITE_सूचकांक_CONSTRAINT_GE );
        निश्चित( pTerm->eOperator&(WO_IN|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        अगर( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_व्यंजक_Is_Vector(pTerm->p_व्यंजक->p_दायाँ)
          ){
          testcase( j!=i );
          अगर( j<16 ) mNoOmit |= (1 << j);
          अगर( op==WO_LT ) pIdxCons[j].op = WO_LE;
          अगर( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  निश्चित( j==nTerm );
  pIdxInfo->nConstraint = j;
  क्रम(i=j=0; i<nOrderBy; i++){
    व्यंजक *p_व्यंजक = pOrderBy->a[i].p_व्यंजक;
    अगर( sqlite3_व्यंजक_Is_Constant(0, p_व्यंजक) ) जारी;
    निश्चित( p_व्यंजक->op==TK_COLUMN
         || (p_व्यंजक->op==TK_COLLATE && p_व्यंजक->p_बायाँ->op==TK_COLUMN
              && p_व्यंजक->iColumn==p_व्यंजक->p_बायाँ->iColumn) );
    pIdxOrderBy[j].iColumn = p_व्यंजक->iColumn;
    pIdxOrderBy[j].desc = pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC;
    j++;
  }
  pIdxInfo->nOrderBy = j;

  *pmNoOmit = mNoOmit;
  वापस pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
अटल व्योम freeIdxStr(sqlite3_सूचकांक_info *pIdxInfo){
  अगर( pIdxInfo->needToFreeIdxStr ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->needToFreeIdxStr = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
अटल व्योम free_सूचकांक_Info(sqlite3 *db, sqlite3_सूचकांक_info *pIdxInfo){
  Hidden_सूचकांक_Info *pHidden;
  पूर्णांक i;
  निश्चित( pIdxInfo!=0 );
  pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  निश्चित( pHidden->pParse!=0 );
  निश्चित( pHidden->pParse->db==db );
  क्रम(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_मूल्य_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
अटल पूर्णांक vtab_Best_सूचकांक(Parse *pParse, Table *pTab, sqlite3_सूचकांक_info *p){
  पूर्णांक rc;
  sqlite3_vtab *pVtab;

  निश्चित( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_सूचकांक_Info_Inputs(p, pTab);
  pParse->db->nSchemaLock++;
  rc = pVtab->pModule->x_Best_सूचकांक(pVtab, p);
  pParse->db->nSchemaLock--;
  where_Trace_सूचकांक_Info_Outputs(p, pTab);

  अगर( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    अगर( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }अन्यथा अगर( !pVtab->z_Err_संदेश ){
      sqlite3_Error_संदेश(pParse, "%s", sqlite3ErrStr(rc));
    }अन्यथा{
      sqlite3_Error_संदेश(pParse, "%s", pVtab->z_Err_संदेश);
    }
  }
  अगर( pTab->u.vtab.p->bAllSchemas ){
    sqlite3VtabUsesAllSchemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_संदेश);
  pVtab->z_Err_संदेश = 0;
  वापस rc;
}
#पूर्ण_अगर /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#अगर_घोषित SQLITE_ENABLE_STAT4
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
अटल पूर्णांक where_कुंजी_Stats(
  Parse *pParse,              /* Database connection */
  सूचकांक *pIdx,                /* Index to consider domain of */
  UnpackedRecord *pRec,       /* Vector of values to consider */
  पूर्णांक roundUp,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  सूचकांक_Sample *aSample = pIdx->aSample;
  पूर्णांक iCol;                   /* Index of required stats in anEq[] etc. */
  पूर्णांक i;                      /* Index of first sample >= pRec */
  पूर्णांक iSample;                /* Smallest sample larger than or equal to pRec */
  पूर्णांक iMin = 0;               /* Smallest sample not yet tested */
  पूर्णांक iTest;                  /* Next sample to test */
  पूर्णांक res;                    /* Result of comparison operation */
  पूर्णांक nField;                 /* Number of fields in pRec */
  tRowcnt iLower = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#अगर_अघोषित SQLITE_DEBUG
  UNUSED_PARAMETER( pParse );
#पूर्ण_अगर
  निश्चित( pRec!=0 );
  निश्चित( pIdx->nSample>0 );
  निश्चित( pRec->nField>0 );


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
  अगर( !HasRowid(pIdx->pTable) && Is_Primary_कुंजी_सूचकांक(pIdx) ){
    nField = pIdx->n_कुंजी_Col;
  }अन्यथा{
    nField = pIdx->nColumn;
  }
  nField = MIN(pRec->nField, nField);
  iCol = 0;
  iSample = pIdx->nSample * nField;
  करो{
    पूर्णांक iSamp;                    /* Index in aSample[] of test sample */
    पूर्णांक n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / nField;
    अगर( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      क्रम(n=(iTest % nField) + 1; n<nField; n++){
        अगर( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) अवरोध;
      }
    }अन्यथा{
      n = iTest + 1;
    }

    pRec->nField = n;
    res = sqlite3VdbeRecordCompare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    अगर( res<0 ){
      iLower = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }अन्यथा अगर( res==0 && n<nField ){
      iLower = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }अन्यथा{
      iSample = iTest;
      iCol = n-1;
    }
  }जबतक( res && iMin<iSample );
  i = iSample / nField;

#अगर_घोषित SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  अगर( pParse->db->mallocFailed==0 ){
    अगर( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      निश्चित( i<pIdx->nSample );
      निश्चित( iCol==nField-1 );
      pRec->nField = nField;
      निश्चित( 0==sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }अन्यथा{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      निश्चित( i<=pIdx->nSample && i>=0 );
      pRec->nField = iCol+1;
      निश्चित( i==pIdx->nSample
           || sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      अगर( iCol>0 ){
        pRec->nField = iCol;
        निश्चित( sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      अगर( i>0 ){
        pRec->nField = nField;
        निश्चित( sqlite3VdbeRecordCompare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#पूर्ण_अगर /* ifdef SQLITE_DEBUG */

  अगर( res==0 ){
    /* Record pRec is equal to sample i */
    निश्चित( iCol==nField-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }अन्यथा{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt iUpper, iGap;
    अगर( i>=pIdx->nSample ){
      iUpper = pIdx->nRowEst0;
    }अन्यथा{
      iUpper = aSample[i].anLt[iCol];
    }

    अगर( iLower>=iUpper ){
      iGap = 0;
    }अन्यथा{
      iGap = iUpper - iLower;
    }
    अगर( roundUp ){
      iGap = (iGap*2)/3;
    }अन्यथा{
      iGap = iGap/3;
    }
    aStat[0] = iLower + iGap;
    aStat[1] = pIdx->aAvgEq[nField-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->nField = nField;
  वापस i;
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

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
अटल LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew){
  LogEst nRet = nNew;
  अगर( pTerm ){
    अगर( pTerm->truthProb<=0 ){
      nRet += pTerm->truthProb;
    }अन्यथा अगर( (pTerm->wtFlags & TERM_VNULL)==0 ){
      nRet -= 20;        निश्चित( 20==sqlite3LogEst(4) );
    }
  }
  वापस nRet;
}


#अगर_घोषित SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
अक्षर sqlite3_सूचकांक_Column_Affinity(sqlite3 *db, सूचकांक *pIdx, पूर्णांक iCol){
  निश्चित( iCol>=0 && iCol<pIdx->nColumn );
  अगर( !pIdx->zColAff ){
    अगर( sqlite3_सूचकांक_Affinity_Str(db, pIdx)==0 ) वापस SQLITE_AFF_BLOB;
  }
  निश्चित( pIdx->zColAff[iCol]!=0 );
  वापस pIdx->zColAff[iCol];
}
#पूर्ण_अगर


#अगर_घोषित SQLITE_ENABLE_STAT4
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
अटल पूर्णांक whereRangeSkipScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  पूर्णांक *pbDone          /* Set to true if at least one expr. value extracted */
){
  सूचकांक *p = pLoop->u.btree.p_सूचकांक;
  पूर्णांक nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  पूर्णांक nLower = -1;
  पूर्णांक nUpper = p->nSample+1;
  पूर्णांक rc = SQLITE_OK;
  u8 aff = sqlite3_सूचकांक_Column_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_मूल्य *p1 = 0;          /* Value extracted from pLower */
  sqlite3_मूल्य *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_मूल्य *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  अगर( pLower ){
    rc = sqlite3_Stat4_मूल्य_From_व्यंजक(pParse, pLower->p_व्यंजक->p_दायाँ, aff, &p1);
    nLower = 0;
  }
  अगर( pUpper && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_मूल्य_From_व्यंजक(pParse, pUpper->p_व्यंजक->p_दायाँ, aff, &p2);
    nUpper = p2 ? 0 : p->nSample;
  }

  अगर( p1 || p2 ){
    पूर्णांक i;
    पूर्णांक nDiff;
    क्रम(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      अगर( rc==SQLITE_OK && p1 ){
        पूर्णांक res = sqlite3MemCompare(p1, pVal, pColl);
        अगर( res>=0 ) nLower++;
      }
      अगर( rc==SQLITE_OK && p2 ){
        पूर्णांक res = sqlite3MemCompare(p2, pVal, pColl);
        अगर( res>=0 ) nUpper++;
      }
    }
    nDiff = (nUpper - nLower);
    अगर( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    अगर( nDiff!=1 || pUpper==0 || pLower==0 ){
      पूर्णांक nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->nOut -= nAdjust;
      *pbDone = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           nLower, nUpper, nAdjust*-1, pLoop->nOut));
    }

  }अन्यथा{
    निश्चित( *pbDone==0 );
  }

  sqlite3_मूल्य_Free(p1);
  sqlite3_मूल्य_Free(p2);
  sqlite3_मूल्य_Free(pVal);

  वापस rc;
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

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
अटल पूर्णांक whereRangeScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक nOut = pLoop->nOut;
  LogEst nNew;

#अगर_घोषित SQLITE_ENABLE_STAT4
  सूचकांक *p = pLoop->u.btree.p_सूचकांक;
  पूर्णांक nEq = pLoop->u.btree.nEq;

  अगर( p->nSample>0 && ALWAYS(nEq<p->nSampleCol)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    अगर( nEq==pBuilder->nRecValid ){
      UnpackedRecord *pRec = pBuilder->pRec;
      tRowcnt a[2];
      पूर्णांक nBtm = pLoop->u.btree.nBtm;
      पूर्णांक n_शीर्ष = pLoop->u.btree.n_शीर्ष;

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
      पूर्णांक iLwrIdx = -2;   /* aSample[] for the lower bound */
      पूर्णांक iUprIdx = -1;   /* aSample[] for the upper bound */

      अगर( pRec ){
        testcase( pRec->nField!=pBuilder->nRecValid );
        pRec->nField = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      अगर( nEq==0 ){
        iLower = 0;
        iUpper = p->nRowEst0;
      }अन्यथा{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_कुंजी_Stats(pParse, p, pRec, 0, a);
        iLower = a[0];
        iUpper = a[0] + a[1];
      }

      निश्चित( pLower==0 || (pLower->eOperator & (WO_GT|WO_GE))!=0 );
      निश्चित( pUpper==0 || (pUpper->eOperator & (WO_LT|WO_LE))!=0 );
      निश्चित( p->aSortOrder!=0 );
      अगर( p->aSortOrder[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(WhereTerm*, pLower, pUpper);
        SWAP(पूर्णांक, nBtm, n_शीर्ष);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      अगर( pLower ){
        पूर्णांक n;                    /* Values extracted from pExpr */
        व्यंजक *p_व्यंजक = pLower->p_व्यंजक->p_दायाँ;
        rc = sqlite3_Stat4_Probe_Set_मूल्य(pParse, p, &pRec, p_व्यंजक, nBtm, nEq, &n);
        अगर( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          अगर( sqlite3_व्यंजक_Vector_आकार(p_व्यंजक)>n ) mask = (WO_LE|WO_LT);
          iLwrIdx = where_कुंजी_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((pLower->eOperator & mask) ? a[1] : 0);
          अगर( iNew>iLower ) iLower = iNew;
          nOut--;
          pLower = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      अगर( pUpper ){
        पूर्णांक n;                    /* Values extracted from pExpr */
        व्यंजक *p_व्यंजक = pUpper->p_व्यंजक->p_दायाँ;
        rc = sqlite3_Stat4_Probe_Set_मूल्य(pParse, p, &pRec, p_व्यंजक, n_शीर्ष, nEq, &n);
        अगर( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          अगर( sqlite3_व्यंजक_Vector_आकार(p_व्यंजक)>n ) mask = (WO_LE|WO_LT);
          iUprIdx = where_कुंजी_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((pUpper->eOperator & mask) ? a[1] : 0);
          अगर( iNew<iUpper ) iUpper = iNew;
          nOut--;
          pUpper = 0;
        }
      }

      pBuilder->pRec = pRec;
      अगर( rc==SQLITE_OK ){
        अगर( iUpper>iLower ){
          nNew = sqlite3LogEst(iUpper - iLower);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          अगर( iLwrIdx==iUprIdx ){ nNew -= 20; }
          निश्चित( 20==sqlite3LogEst(4) );
        }अन्यथा{
          nNew = 10;        निश्चित( 10==sqlite3LogEst(2) );
        }
        अगर( nNew<nOut ){
          nOut = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)iLower, (u32)iUpper, nOut));
      }
    }अन्यथा{
      पूर्णांक bDone = 0;
      rc = whereRangeSkipScanEst(pParse, pLower, pUpper, pLoop, &bDone);
      अगर( bDone ) वापस rc;
    }
  }
#अन्यथा
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pBuilder);
  निश्चित( pLower || pUpper );
#पूर्ण_अगर
  निश्चित( pUpper==0 || (pUpper->wtFlags & TERM_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  अगर( pLower && pLower->truthProb>0 && pUpper && pUpper->truthProb>0 ){
    nNew -= 20;
  }

  nOut -= (pLower!=0) + (pUpper!=0);
  अगर( nNew<10 ) nNew = 10;
  अगर( nNew<nOut ) nOut = nNew;
#अगर defined(WHERETRACE_ENABLED)
  अगर( pLoop->nOut>nOut ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->nOut, nOut));
  }
#पूर्ण_अगर
  pLoop->nOut = (LogEst)nOut;
  वापस rc;
}

#अगर_घोषित SQLITE_ENABLE_STAT4
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
अटल पूर्णांक whereEqualScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  व्यंजक *p_व्यंजक,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  सूचकांक *p = pBuilder->pNew->u.btree.p_सूचकांक;
  पूर्णांक nEq = pBuilder->pNew->u.btree.nEq;
  UnpackedRecord *pRec = pBuilder->pRec;
  पूर्णांक rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  पूर्णांक bOk;

  निश्चित( nEq>=1 );
  निश्चित( nEq<=p->nColumn );
  निश्चित( p->aSample!=0 );
  निश्चित( p->nSample>0 );
  निश्चित( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  अगर( pBuilder->nRecValid<(nEq-1) ){
    वापस SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  अगर( nEq>=p->nColumn ){
    *pnRow = 1;
    वापस SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_Set_मूल्य(pParse, p, &pRec, p_व्यंजक, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  अगर( rc!=SQLITE_OK ) वापस rc;
  अगर( bOk==0 ) वापस SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_कुंजी_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (पूर्णांक)a[1]));
  *pnRow = a[1];
 
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

#अगर_घोषित SQLITE_ENABLE_STAT4
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
अटल पूर्णांक whereInScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  व्यंजक_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  सूचकांक *p = pBuilder->pNew->u.btree.p_सूचकांक;
  i64 nRow0 = sqlite3LogEstToInt(p->aiRowLogEst[0]);
  पूर्णांक nRecValid = pBuilder->nRecValid;
  पूर्णांक rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  पूर्णांक i;                  /* Loop counter */

  निश्चित( p->aSample!=0 );
  क्रम(i=0; rc==SQLITE_OK && i<pList->n_व्यंजक; i++){
    nEst = nRow0;
    rc = whereEqualScanEst(pParse, pBuilder, pList->a[i].p_व्यंजक, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  अगर( rc==SQLITE_OK ){
    अगर( nRowEst > (tRowcnt)nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", nRowEst));
  }
  निश्चित( pBuilder->nRecValid==nRecValid );
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */


#अगर defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
व्योम sqlite3WhereTermPrint(WhereTerm *pTerm, पूर्णांक iTerm){
  अगर( pTerm==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", iTerm);
  }अन्यथा{
    अक्षर zType[8];
    अक्षर z_बायाँ[50];
    स_नकल(zType, "....", 5);
    अगर( pTerm->wtFlags & TERM_VIRTUAL ) zType[0] = 'V';
    अगर( pTerm->eOperator & WO_EQUIV  ) zType[1] = 'E';
    अगर( व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_OuterON) ) zType[2] = 'L';
    अगर( pTerm->wtFlags & TERM_CODED  ) zType[3] = 'C';
    अगर( pTerm->eOperator & WO_SINGLE ){
      निश्चित( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(माप(z_बायाँ),z_बायाँ,"left={%d:%d}",
                       pTerm->बायाँ_Cursor, pTerm->u.x.बायाँ_Column);
    }अन्यथा अगर( (pTerm->eOperator & WO_OR)!=0 && pTerm->u.pOrInfo!=0 ){
      sqlite3_snprintf(माप(z_बायाँ),z_बायाँ,"indexable=0x%llx",
                       pTerm->u.pOrInfo->indexable);
    }अन्यथा{
      sqlite3_snprintf(माप(z_बायाँ),z_बायाँ,"left=%d", pTerm->बायाँ_Cursor);
    }
    iTerm = pTerm->iTerm = MAX(iTerm,pTerm->iTerm);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       iTerm, pTerm, zType, z_बायाँ, pTerm->eOperator, pTerm->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    अगर( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        pTerm->truthProb, (u64)pTerm->prereqAll, (u64)pTerm->prereq_दायाँ);
    }
    अगर( (pTerm->eOperator & (WO_OR|WO_AND))==0 && pTerm->u.x.iField ){
      sqlite3DebugPrintf(" iField=%d", pTerm->u.x.iField);
    }
    अगर( pTerm->iParent>=0 ){
      sqlite3DebugPrintf(" iParent=%d", pTerm->iParent);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_वृक्ष_View_व्यंजक(0, pTerm->p_व्यंजक, 0);
  }
}
व्योम sqlite3ShowWhereTerm(WhereTerm *pTerm){
  sqlite3WhereTermPrint(pTerm, 0);
}
#पूर्ण_अगर

#अगर_घोषित WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
व्योम sqlite3WhereClausePrint(WhereClause *pWC){
  पूर्णांक i;
  क्रम(i=0; i<pWC->nTerm; i++){
    sqlite3WhereTermPrint(&pWC->a[i], i);
  }
}
#पूर्ण_अगर

#अगर_घोषित WHERETRACE_ENABLED
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
व्योम sqlite3WhereLoopPrint(स्थिर WhereLoop *p, स्थिर WhereClause *pWC){
  WhereInfo *pWInfo;
  अगर( pWC ){
    पूर्णांक nb;
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
  }अन्यथा{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->maskSelf, p->prereq & 0xfff, p->cId, p->iTab);
  }
  अगर( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    स्थिर अक्षर *zName;
    अगर( p->u.btree.p_सूचकांक && (zName = p->u.btree.p_सूचकांक->zName)!=0 ){
      अगर( म_भेदन(zName, "sqlite_autoindex_", 17)==0 ){
        पूर्णांक i = sqlite3Strlen30(zName) - 1;
        जबतक( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }अन्यथा{
      sqlite3DebugPrintf("%20s","");
    }
  }अन्यथा{
    अक्षर *z;
    अगर( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_अंक, p->u.vtab.idxStr, p->u.vtab.omitMask);
    }अन्यथा{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_अंक, p->u.vtab.omitMask);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  अगर( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }अन्यथा{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  अगर( pWInfo && pWInfo->bStarUsed && p->rStarDelta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->rSetup, p->rRun, p->nOut, p->rStarDelta);
  }अन्यथा{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->rSetup, p->rRun, p->nOut);
  }
  अगर( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    पूर्णांक i;
    क्रम(i=0; i<p->nLTerm; i++){
      sqlite3WhereTermPrint(p->aLTerm[i], i);
    }
  }
}
व्योम sqlite3ShowWhereLoop(स्थिर WhereLoop *p){
  अगर( p ) sqlite3WhereLoopPrint(p, 0);
}
व्योम sqlite3ShowWhereLoopList(स्थिर WhereLoop *p){
  जबतक( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_अग्र_Loop;
  }
}
#पूर्ण_अगर

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
अटल व्योम whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_आकार(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
अटल व्योम whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  अगर( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_सूचकांक) ){
    अगर( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }अन्यथा अगर( (p->wsFlags & WHERE_AUTO_सूचकांक)!=0 && p->u.btree.p_सूचकांक!=0 ){
      sqlite3DbFree(db, p->u.btree.p_सूचकांक->zColAff);
      sqlite3DbFreeNN(db, p->u.btree.p_सूचकांक);
      p->u.btree.p_सूचकांक = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
अटल व्योम whereLoopClear(sqlite3 *db, WhereLoop *p){
  अगर( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_आकार(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
अटल पूर्णांक whereLoopResize(sqlite3 *db, WhereLoop *p, पूर्णांक n){
  WhereTerm **paNew;
  अगर( p->nLSlot>=n ) वापस SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, माप(p->aLTerm[0])*n);
  अगर( paNew==0 ) वापस SQLITE_NOMEM_BKPT;
  स_नकल(paNew, p->aLTerm, माप(p->aLTerm[0])*p->nLSlot);
  अगर( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  वापस SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
अटल पूर्णांक whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom){
  whereLoopClearUnion(db, pTo);
  अगर( pFrom->nLTerm > pTo->nLSlot
   && whereLoopResize(db, pTo, pFrom->nLTerm)
  ){
    स_रखो(pTo, 0, WHERE_LOOP_XFER_SZ);
    वापस SQLITE_NOMEM_BKPT;
  }
  स_नकल(pTo, pFrom, WHERE_LOOP_XFER_SZ);
  स_नकल(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm*माप(pTo->aLTerm[0]));
  अगर( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }अन्यथा अगर( (pFrom->wsFlags & WHERE_AUTO_सूचकांक)!=0 ){
    pFrom->u.btree.p_सूचकांक = 0;
  }
  वापस SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
अटल व्योम whereLoopDelete(sqlite3 *db, WhereLoop *p){
  निश्चित( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
अटल व्योम whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  निश्चित( pWInfo!=0 );
  निश्चित( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  जबतक( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_अग्र_Loop;
    whereLoopDelete(db, p);
  }
  जबतक( pWInfo->pMemToFree ){
    WhereMemBlock *p_अग्र = pWInfo->pMemToFree->p_अग्र;
    sqlite3DbNNFreeNN(db, pWInfo->pMemToFree);
    pWInfo->pMemToFree = p_अग्र;
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
अटल पूर्णांक whereLoopCheaperProperSubset(
  स्थिर WhereLoop *pX,       /* First WhereLoop to compare */
  स्थिर WhereLoop *pY        /* Compare against this WhereLoop */
){
  पूर्णांक i, j;
  अगर( pX->rRun>pY->rRun && pX->nOut>pY->nOut ) वापस 0; /* (1d) and (2a) */
  निश्चित( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  निश्चित( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  अगर( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_सूचकांक==pY->u.btree.p_सूचकांक             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    वापस 1;  /* Case 1 is true */
  }
  अगर( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    वापस 0;                                            /* (2b) */
  }
  अगर( pY->nSkip > pX->nSkip ) वापस 0;                  /* (2d) */
  क्रम(i=pX->nLTerm-1; i>=0; i--){
    अगर( pX->aLTerm[i]==0 ) जारी;
    क्रम(j=pY->nLTerm-1; j>=0; j--){
      अगर( pY->aLTerm[j]==pX->aLTerm[i] ) अवरोध;
    }
    अगर( j<0 ) वापस 0;                                  /* (2c) */
  }
  अगर( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    वापस 0;                                            /* (2e) */
  }
  वापस 1;  /* Case 2 is true */
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
अटल व्योम whereLoopAdjustCost(स्थिर WhereLoop *p, WhereLoop *pTemplate){
  अगर( (pTemplate->wsFlags & WHERE_INDEXED)==0 ) वापस;
  क्रम(; p; p=p->p_अग्र_Loop){
    अगर( p->iTab!=pTemplate->iTab ) जारी;
    अगर( (p->wsFlags & WHERE_INDEXED)==0 ) जारी;
    अगर( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut,
                       MIN(p->rRun, pTemplate->rRun),
                       MIN(p->nOut - 1, pTemplate->nOut)));
      pTemplate->rRun = MIN(p->rRun, pTemplate->rRun);
      pTemplate->nOut = MIN(p->nOut - 1, pTemplate->nOut);
    }अन्यथा अगर( whereLoopCheaperProperSubset(pTemplate, p) ){
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
अटल WhereLoop **whereLoopFindLesser(
  WhereLoop **pp_पूर्व,
  स्थिर WhereLoop *pTemplate
){
  WhereLoop *p;
  क्रम(p=(*pp_पूर्व); p; pp_पूर्व=&p->p_अग्र_Loop, p=*pp_पूर्व){
    अगर( p->iTab!=pTemplate->iTab || p->iSortIdx!=pTemplate->iSortIdx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      जारी;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    निश्चित( p->rSetup==0 || pTemplate->rSetup==0
                 || p->rSetup==pTemplate->rSetup );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    निश्चित( p->rSetup>=pTemplate->rSetup );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    अगर( (p->wsFlags & WHERE_AUTO_सूचकांक)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_INDEXED)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      अवरोध;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    अगर( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->rSetup<=pTemplate->rSetup                  /* (2a) */
     && p->rRun<=pTemplate->rRun                      /* (2b) */
     && p->nOut<=pTemplate->nOut                      /* (2c) */
    ){
      वापस 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    अगर( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->rRun>=pTemplate->rRun                             /* (2a) */
     && p->nOut>=pTemplate->nOut                             /* (2b) */
    ){
      निश्चित( p->rSetup>=pTemplate->rSetup ); /* SETUP-INVARIANT above */
      अवरोध;   /* Cause p to be overwritten by pTemplate */
    }
  }
  वापस pp_पूर्व;
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
अटल पूर्णांक where_Loop_जोड़ो(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **pp_पूर्व, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  पूर्णांक rc;

  /* Stop the search once we hit the query planner search limit */
  अगर( pBuilder->iPlanLimit==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    अगर( pBuilder->pOrSet ) pBuilder->pOrSet->n = 0;
    वापस SQLITE_DONE;
  }
  pBuilder->iPlanLimit--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  अगर( pBuilder->pOrSet!=0 ){
    अगर( pTemplate->nLTerm ){
#अगर WHERETRACE_ENABLED
      u16 n = pBuilder->pOrSet->n;
      पूर्णांक x =
#पूर्ण_अगर
      where_Or_जोड़ो(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun,
                                    pTemplate->nOut);
#अगर WHERETRACE_ENABLED /* 0x8 */
      अगर( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#पूर्ण_अगर
    }
    वापस SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  pp_पूर्व = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  अगर( pp_पूर्व==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#अगर WHERETRACE_ENABLED /* 0x8 */
    अगर( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#पूर्ण_अगर
    वापस SQLITE_OK; 
  }अन्यथा{
    p = *pp_पूर्व;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#अगर WHERETRACE_ENABLED /* 0x8 */
  अगर( sqlite3WhereTrace & 0x8 ){
    अगर( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }अन्यथा{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#पूर्ण_अगर
  अगर( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *pp_पूर्व = p = sqlite3DbMallocRawNN(db, माप(WhereLoop));
    अगर( p==0 ) वापस SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_अग्र_Loop = 0;
  }अन्यथा{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_पुच्छ = &p->p_अग्र_Loop;
    WhereLoop *pToDel;
    जबतक( *pp_पुच्छ ){
      pp_पुच्छ = whereLoopFindLesser(pp_पुच्छ, pTemplate);
      अगर( pp_पुच्छ==0 ) अवरोध;
      pToDel = *pp_पुच्छ;
      अगर( pToDel==0 ) अवरोध;
      *pp_पुच्छ = pToDel->p_अग्र_Loop;
#अगर WHERETRACE_ENABLED /* 0x8 */
      अगर( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(pToDel, pBuilder->pWC);
      }
#पूर्ण_अगर
      whereLoopDelete(db, pToDel);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  अगर( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    सूचकांक *p_सूचकांक = p->u.btree.p_सूचकांक;
    अगर( p_सूचकांक && p_सूचकांक->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_सूचकांक = 0;
    }
  }
  वापस rc;
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
अटल पूर्णांक व्यंजक_गाँठ_Pattern_लंबाई_Est(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_STRING ){
    पूर्णांक sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_व्यंजक->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    अगर( pWalker->eCode ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }अन्यथा{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    जबतक( (c = *(z++))!=0 ){
      अगर( c==c3 ){
        अगर( *z ) z++;
        जबतक( *z && *z!=']' ) z++;
      }अन्यथा अगर( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    अगर( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  वापस WRC_Continue;
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
अटल पूर्णांक est_Like_Pattern_लंबाई(व्यंजक *p, u16 eCode){
  Walker w;
  w.u.sz = 0;
  w.eCode = eCode;
  w.x_व्यंजक_Callback = व्यंजक_गाँठ_Pattern_लंबाई_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#अगर_घोषित SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#पूर्ण_अगर
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.u.sz;
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
अटल व्योम whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst nRow            /* Number of rows in the entire table */
){
  WhereTerm *pTerm, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->maskSelf);
  पूर्णांक i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  अगर( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->pOrderBy==0 ){
    वापस;
  }

  निश्चित( (pLoop->wsFlags & WHERE_AUTO_सूचकांक)==0 );
  क्रम(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
    निश्चित( pTerm!=0 );
    अगर( (pTerm->prereqAll & notAllowed)!=0 ) जारी;
    अगर( (pTerm->prereqAll & pLoop->maskSelf)==0 ) जारी;
    अगर( (pTerm->wtFlags & TERM_VIRTUAL)!=0 ) जारी;
    क्रम(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      अगर( pX==0 ) जारी;
      अगर( pX==pTerm ) अवरोध;
      अगर( pX->iParent>=0 && (&pWC->a[pX->iParent])==pTerm ) अवरोध;
    }
    अगर( j<0 ){
      sqlite3ProgressCheck(pWC->pWInfo->pParse);
      अगर( pLoop->maskSelf==pTerm->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        अगर( (pTerm->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_बायाँ|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      अगर( pTerm->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->nOut += pTerm->truthProb;
      }अन्यथा{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        व्यंजक *p_Op_व्यंजक = pTerm->p_व्यंजक;
        pLoop->nOut--;
        अगर( (pTerm->eOperator&(WO_EQ|WO_IS))!=0
         && (pTerm->wtFlags & TERM_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          व्यंजक *p_दायाँ = p_Op_व्यंजक->p_दायाँ;
          Parse *pParse = pWC->pWInfo->pParse;
          पूर्णांक k = 0;
          testcase( p_Op_व्यंजक->op==TK_IS );
          अगर( sqlite3_व्यंजक_Is_Integer(p_दायाँ, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }अन्यथा{
            k = 20;
          }
          अगर( iReduce<k ){
            pTerm->wtFlags |= TERM_HEURTRUTH;
            iReduce = k;
          }
        }अन्यथा
        अगर( व्यंजक_Has_Property(p_Op_व्यंजक, EP_InfixFunc) 
         && p_Op_व्यंजक->op==TK_FUNCTION
        ){
          पूर्णांक eOp;
          निश्चित( व्यंजक_Use_XList(p_Op_व्यंजक) );
          निश्चित( p_Op_व्यंजक->x.pList->n_व्यंजक>=2 );
          eOp = sqlite3_व्यंजक_Is_Like_Operator(p_Op_व्यंजक);
          अगर( ALWAYS(eOp>0) ){
            पूर्णांक szPattern;
            व्यंजक *pRHS = p_Op_व्यंजक->x.pList->a[0].p_व्यंजक;
            eOp = eOp==SQLITE_सूचकांक_CONSTRAINT_LIKE;
            szPattern = est_Like_Pattern_लंबाई(pRHS, eOp);
            अगर( szPattern>0 ){
              pLoop->nOut -= szPattern*2;
            }
          }
        }
      }
    }
  }
  अगर( pLoop->nOut > nRow-iReduce ){
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
अटल पूर्णांक whereRangeVectorLen(
  Parse *pParse,       /* Parsing context */
  पूर्णांक iCur,            /* Cursor open on pIdx */
  सूचकांक *pIdx,         /* The index to be used for a inequality constraint */
  पूर्णांक nEq,             /* Number of prior equality constraints on same index */
  WhereTerm *pTerm     /* The vector inequality constraint */
){
  पूर्णांक nCmp = sqlite3_व्यंजक_Vector_आकार(pTerm->p_व्यंजक->p_बायाँ);
  पूर्णांक i;

  nCmp = MIN(nCmp, (pIdx->nColumn - nEq));
  क्रम(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    अक्षर aff;                     /* Comparison affinity */
    अक्षर idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    व्यंजक *pLhs, *pRhs;

    निश्चित( व्यंजक_Use_XList(pTerm->p_व्यंजक->p_बायाँ) );
    pLhs = pTerm->p_व्यंजक->p_बायाँ->x.pList->a[i].p_व्यंजक;
    pRhs = pTerm->p_व्यंजक->p_दायाँ;
    अगर( व्यंजक_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_व्यंजक;
    }अन्यथा{
      pRhs = pRhs->x.pList->a[i].p_व्यंजक;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    अगर( pLhs->op!=TK_COLUMN
     || pLhs->iTable!=iCur
     || pLhs->iColumn!=pIdx->aiColumn[i+nEq]
     || pIdx->aSortOrder[i+nEq]!=pIdx->aSortOrder[nEq]
    ){
      अवरोध;
    }

    testcase( pLhs->iColumn==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_व्यंजक_Affinity(pLhs));
    idxaff = sqlite3TableColumnAffinity(pIdx->pTable, pLhs->iColumn);
    अगर( aff!=idxaff ) अवरोध;

    अगर( व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_Commuted) ) SWAP(व्यंजक*, pRhs, pLhs);
    pColl = sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, pLhs, pRhs);
    अगर( pColl==0 ) अवरोध;
    अगर( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) अवरोध;
  }
  वापस i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#अगर_घोषित SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#अन्यथा
# define ApplyCostMultiplier(C,T)
#पूर्ण_अगर

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
अटल पूर्णांक where_Loop_Add_Btree_सूचकांक(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  SrcItem *pSrc,                  /* FROM clause term being analyzed */
  सूचकांक *pProbe,                  /* An index on pSrc */
  LogEst nInMul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  WhereTerm *pTerm;               /* A WhereTerm under consideration */
  पूर्णांक opMask;                     /* Valid operators for constraints */
  WhereScan scan;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_शीर्ष;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_nOut;              /* Original value of pNew->nOut */
  पूर्णांक rc = SQLITE_OK;             /* Return code */
  LogEst r_आकार;                   /* Number of rows in the table */
  LogEst r_Log_आकार;                /* Logarithm of table size */
  WhereTerm *p_शीर्ष_2 = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  निश्चित( db->mallocFailed==0 || pParse->nErr>0 );
  अगर( pParse->nErr ){
    वापस pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->pTable->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->rRun));

  निश्चित( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  निश्चित( (pNew->wsFlags & WHERE_शीर्ष_LIMIT)==0 );
  अगर( pNew->wsFlags & WHERE_BTM_LIMIT ){
    opMask = WO_LT|WO_LE;
  }अन्यथा{
    निश्चित( pNew->u.btree.nBtm==0 );
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  अगर( pProbe->bUnordered ){
    opMask &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  निश्चित( pNew->u.btree.nEq<pProbe->nColumn );
  निश्चित( pNew->u.btree.nEq<pProbe->n_कुंजी_Col
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_शीर्ष = pNew->u.btree.n_शीर्ष;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_nOut = pNew->nOut;
  pTerm = whereScanInit(&scan, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        opMask, pProbe);
  pNew->rSetup = 0;
  r_आकार = pProbe->aiRowLogEst[0];
  r_Log_आकार = estLog(r_आकार);
  क्रम(; rc==SQLITE_OK && pTerm!=0; pTerm = where_Scan_अग्र(&scan)){
    u16 eOp = pTerm->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst nOutUnadjusted;        /* nOut before IN() and WHERE adjustments */
    पूर्णांक nIn = 0;
#अगर_घोषित SQLITE_ENABLE_STAT4
    पूर्णांक nRecValid = pBuilder->nRecValid;
#पूर्ण_अगर
    अगर( (eOp==WO_ISNULL || (pTerm->wtFlags&TERM_VNULL)!=0)
     && सूचकांक_Column_Not_Null(pProbe, saved_nEq)
    ){
      जारी; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    अगर( pTerm->prereq_दायाँ & pNew->maskSelf ) जारी;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    अगर( pTerm->wtFlags & TERM_LIKEOPT && pTerm->eOperator==WO_LT ) जारी;

    अगर( (pSrc->fg.jointype & (JT_बायाँ|JT_LTORJ|JT_दायाँ))!=0
     && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
    ){
      जारी;
    }
    अगर( Is_Unique_सूचकांक(pProbe) && saved_nEq==pProbe->n_कुंजी_Col-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }अन्यथा{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_INDEXED;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_शीर्ष = saved_n_शीर्ष;
    pNew->nLTerm = saved_nLTerm;
    अगर( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       अवरोध; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereq_दायाँ) & ~pNew->maskSelf;

    निश्चित( nInMul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0
        || (pNew->wsFlags & WHERE_COLUMN_IN)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    अगर( eOp & WO_IN ){
      व्यंजक *p_व्यंजक = pTerm->p_व्यंजक;
      अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        पूर्णांक i;
        पूर्णांक bRedundant = 0;
        nIn = 46;  निश्चित( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        क्रम(i=0; i<pNew->nLTerm-1; i++){
          अगर( pNew->aLTerm[i] && pNew->aLTerm[i]->p_व्यंजक==p_व्यंजक ){
            nIn = 0;
            अगर( pNew->aLTerm[i]->u.x.iField == pTerm->u.x.iField ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        अगर( bRedundant ){
          pNew->nLTerm--;
          जारी;
        }
      }अन्यथा अगर( ALWAYS(p_व्यंजक->x.pList && p_व्यंजक->x.pList->n_व्यंजक) ){
        /* "x IN (value, value, ...)" */
        nIn = sqlite3LogEst(p_व्यंजक->x.pList->n_व्यंजक);
      }
      अगर( pProbe->hasStat1 && r_Log_आकार>=10 ){
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
        x = M + logK + 10 - (nIn + r_Log_आकार);
        अगर( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, nIn, r_Log_आकार, x));
        }अन्यथा अगर( nInMul<2 && OptimizationEnabled(db, SQLITE_SeekScan) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, nIn, r_Log_आकार, x, nInMul));
          pNew->wsFlags |= WHERE_IN_SEEKSCAN;
        }अन्यथा{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, nIn, r_Log_आकार, x, nInMul));
          जारी;
        }
      }
      pNew->wsFlags |= WHERE_COLUMN_IN;
    }अन्यथा अगर( eOp & (WO_EQ|WO_IS) ){
      पूर्णांक iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      निश्चित( saved_nEq==pNew->u.btree.nEq );
      अगर( iCol==XN_ROWID
       || (iCol>=0 && nInMul==0 && saved_nEq==pProbe->n_कुंजी_Col-1)
      ){
        अगर( iCol==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_कुंजी_Col==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }अन्यथा{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      अगर( scan.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }अन्यथा अगर( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }अन्यथा{
      पूर्णांक nVecLen = whereRangeVectorLen(
          pParse, pSrc->iCursor, pProbe, saved_nEq, pTerm
      );
      अगर( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_LIMIT;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = pTerm;
        p_शीर्ष_2 = 0;
        अगर( pTerm->wtFlags & TERM_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_शीर्ष_2 = &pTerm[1];
          निश्चित( (p_शीर्ष_2-(pTerm->pWC->a))<pTerm->pWC->nTerm );
          निश्चित( p_शीर्ष_2->wtFlags & TERM_LIKEOPT );
          निश्चित( p_शीर्ष_2->eOperator==WO_LT );
          अगर( whereLoopResize(db, pNew, pNew->nLTerm+1) ) अवरोध; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_शीर्ष_2;
          pNew->wsFlags |= WHERE_शीर्ष_LIMIT;
          pNew->u.btree.n_शीर्ष = 1;
        }
      }अन्यथा{
        निश्चित( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_शीर्ष_LIMIT;
        pNew->u.btree.n_शीर्ष = nVecLen;
        p_शीर्ष_2 = pTerm;
        pBtm = (pNew->wsFlags & WHERE_BTM_LIMIT)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    निश्चित( pNew->nOut==saved_nOut );
    अगर( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      whereRangeScanEst(pParse, pBuilder, pBtm, p_शीर्ष_2, pNew);
    }अन्यथा{
      पूर्णांक nEq = ++pNew->u.btree.nEq;
      निश्चित( eOp & (WO_ISNULL|WO_EQ|WO_IN|WO_IS) );

      निश्चित( pNew->nOut==saved_nOut );
      अगर( pTerm->truthProb<=0 && pProbe->aiColumn[saved_nEq]>=0 ){
        निश्चित( (eOp & WO_IN) || nIn==0 );
        testcase( eOp & WO_IN );
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      }अन्यथा{
#अगर_घोषित SQLITE_ENABLE_STAT4
        tRowcnt nOut = 0;
        अगर( nInMul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->nSampleCol)
         && ((eOp & WO_IN)==0 || व्यंजक_Use_XList(pTerm->p_व्यंजक))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          व्यंजक *p_व्यंजक = pTerm->p_व्यंजक;
          अगर( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = whereEqualScanEst(pParse, pBuilder, p_व्यंजक->p_दायाँ, &nOut);
          }अन्यथा{
            rc = whereInScanEst(pParse, pBuilder, p_व्यंजक->x.pList, &nOut);
          }
          अगर( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          अगर( rc!=SQLITE_OK ) अवरोध;          /* Jump out of the pTerm loop */
          अगर( nOut ){
            pNew->nOut = sqlite3LogEst(nOut);
            अगर( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->nOut+10 > pProbe->aiRowLogEst[0]
            ){
#अगर WHERETRACE_ENABLED /* 0x01 */
              अगर( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3WhereTermPrint(pTerm, 999);
              }
#पूर्ण_अगर
              pTerm->wtFlags |= TERM_HIGHTRUTH;
              अगर( pTerm->wtFlags & TERM_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            अगर( pNew->nOut>saved_nOut ) pNew->nOut = saved_nOut;
            pNew->nOut -= nIn;
          }
        }
        अगर( nOut==0 )
#पूर्ण_अगर
        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          अगर( eOp & WO_ISNULL ){
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
    निश्चित( pSrc->pSTab->szTabRow>0 );
    अगर( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      rCostIdx = pNew->nOut + 16;
    }अन्यथा{
      rCostIdx = pNew->nOut + 1 + (15*pProbe->szIdxRow)/pSrc->pSTab->szTabRow;
    }
    rCostIdx = sqlite3LogEstAdd(r_Log_आकार, rCostIdx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->rRun = rCostIdx;
    अगर( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    }
    ApplyCostMultiplier(pNew->rRun, pProbe->pTable->costMult);

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_आकार);
    अगर( pSrc->fg.fromExists ) pNew->nOut = 0;
    rc = where_Loop_जोड़ो(pBuilder, pNew);

    अगर( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->nOut = saved_nOut;
    }अन्यथा{
      pNew->nOut = nOutUnadjusted;
    }

    अगर( (pNew->wsFlags & WHERE_शीर्ष_LIMIT)==0
     && pNew->u.btree.nEq<pProbe->nColumn
     && (pNew->u.btree.nEq<pProbe->n_कुंजी_Col ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      अगर( pNew->u.btree.nEq>3 ){
        sqlite3ProgressCheck(pParse);
      }
      where_Loop_Add_Btree_सूचकांक(pBuilder, pSrc, pProbe, nInMul+nIn);
    }
    pNew->nOut = saved_nOut;
#अगर_घोषित SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#पूर्ण_अगर
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_शीर्ष = saved_n_शीर्ष;
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
  निश्चित( 42==sqlite3LogEst(18) );
  अगर( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_कुंजी_Col
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
    where_Loop_Add_Btree_सूचकांक(pBuilder, pSrc, pProbe, nIter + nInMul);
    pNew->nOut = saved_nOut;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->pTable->zName, pProbe->zName, saved_nEq, rc));
  वापस rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
अटल पूर्णांक सूचकांक_Might_Help_With_Order_By(
  WhereLoopBuilder *pBuilder,
  सूचकांक *p_सूचकांक,
  पूर्णांक iCursor
){
  व्यंजक_List *pOB;
  व्यंजक_List *a_Col_व्यंजक;
  पूर्णांक ii, jj;

  अगर( p_सूचकांक->bUnordered ) वापस 0;
  अगर( (pOB = pBuilder->pWInfo->pOrderBy)==0 ) वापस 0;
  क्रम(ii=0; ii<pOB->n_व्यंजक; ii++){
    व्यंजक *p_व्यंजक = sqlite3_व्यंजक_Skip_Collate_And_Likely(pOB->a[ii].p_व्यंजक);
    अगर( NEVER(p_व्यंजक==0) ) जारी;
    अगर( (p_व्यंजक->op==TK_COLUMN || p_व्यंजक->op==TK_AGG_COLUMN) 
     && p_व्यंजक->iTable==iCursor 
    ){
      अगर( p_व्यंजक->iColumn<0 ) वापस 1;
      क्रम(jj=0; jj<p_सूचकांक->n_कुंजी_Col; jj++){
        अगर( p_व्यंजक->iColumn==p_सूचकांक->aiColumn[jj] ) वापस 1;
      }
    }अन्यथा अगर( (a_Col_व्यंजक = p_सूचकांक->a_Col_व्यंजक)!=0 ){
      क्रम(jj=0; jj<p_सूचकांक->n_कुंजी_Col; jj++){
        अगर( p_सूचकांक->aiColumn[jj]!=XN_व्यंजक ) जारी;
        अगर( sqlite3_व्यंजक_Compare_Skip(p_व्यंजक,a_Col_व्यंजक->a[jj].p_व्यंजक,iCursor)==0 ){
          वापस 1;
        }
      }
    }
  }
  वापस 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
अटल पूर्णांक where_Usable_Partial_सूचकांक(
  पूर्णांक iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  व्यंजक *pWhere          /* The WHERE clause from the partial index */
){
  पूर्णांक i;
  WhereTerm *pTerm;
  Parse *pParse;

  अगर( jointype & JT_LTORJ ) वापस 0;
  pParse = pWC->pWInfo->pParse;
  जबतक( pWhere->op==TK_AND ){
    अगर( !where_Usable_Partial_सूचकांक(iTab,jointype,pWC,pWhere->p_बायाँ) ) वापस 0;
    pWhere = pWhere->p_दायाँ;
  }
  क्रम(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    व्यंजक *p_व्यंजक;
    p_व्यंजक = pTerm->p_व्यंजक;
    अगर( (!व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) || p_व्यंजक->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || व्यंजक_Has_Property(p_व्यंजक, EP_OuterON))
     && sqlite3_व्यंजक_Implies_व्यंजक(pParse, p_व्यंजक, pWhere, iTab)
     && !sqlite3_व्यंजक_Implies_व्यंजक(pParse, p_व्यंजक, pWhere, -1)
     && (pTerm->wtFlags & TERM_VNULL)==0
    ){
      वापस 1;
    }
  }
  वापस 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
अटल पूर्णांक व्यंजक_Is_Covered_By_सूचकांक(
  स्थिर व्यंजक *p_व्यंजक,
  स्थिर सूचकांक *pIdx,
  पूर्णांक iTabCur
){
  पूर्णांक i;
  क्रम(i=0; i<pIdx->nColumn; i++){
    अगर( pIdx->aiColumn[i]==XN_व्यंजक
     && sqlite3_व्यंजक_Compare(0, p_व्यंजक, pIdx->a_Col_व्यंजक->a[i].p_व्यंजक, iTabCur)==0
    ){
      वापस 1;
    }
  }
  वापस 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
प्रकार काष्ठा Covering_सूचकांक_Check Covering_सूचकांक_Check;
काष्ठा Covering_सूचकांक_Check {
  सूचकांक *pIdx;       /* The index */
  पूर्णांक iTabCur;       /* Cursor number for the corresponding table */
  u8 b_व्यंजक;          /* Uses an indexed expression */
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
अटल पूर्णांक where_Is_Covering_सूचकांक_Walk_Callback(Walker *pWalk, व्यंजक *p_व्यंजक){
  पूर्णांक i;                    /* Loop counter */
  स्थिर सूचकांक *pIdx;        /* The index of interest */
  स्थिर i16 *aiColumn;      /* Columns contained in the index */
  u16 nColumn;              /* Number of columns in the index */
  Covering_सूचकांक_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  अगर( (p_व्यंजक->op==TK_COLUMN || p_व्यंजक->op==TK_AGG_COLUMN) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    अगर( p_व्यंजक->iTable!=pCk->iTabCur ) वापस WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    क्रम(i=0; i<nColumn; i++){
      अगर( aiColumn[i]==p_व्यंजक->iColumn ) वापस WRC_Continue;
    }
    pCk->bUnidx = 1;
    वापस WRC_Abort;
  }अन्यथा अगर( pIdx->b_Has_व्यंजक
         && व्यंजक_Is_Covered_By_सूचकांक(p_व्यंजक, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_व्यंजक = 1;
    वापस WRC_Prune;
  }
  वापस WRC_Continue;
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
अटल SQLITE_NOINLINE u32 where_Is_Covering_सूचकांक(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  सूचकांक *pIdx,           /* Index that is being tested */
  पूर्णांक iTabCur            /* Cursor for the table being indexed */
){
  पूर्णांक i, rc;
  काष्ठा Covering_सूचकांक_Check ck;
  Walker w;
  अगर( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    वापस 0;
  }
  अगर( pIdx->b_Has_व्यंजक==0 ){
    क्रम(i=0; i<pIdx->nColumn; i++){
      अगर( pIdx->aiColumn[i]>=BMS-1 ) अवरोध;
    }
    अगर( i>=pIdx->nColumn ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      वापस 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_व्यंजक = 0;
  ck.bUnidx = 0;
  स_रखो(&w, 0, माप(w));
  w.x_व्यंजक_Callback = where_Is_Covering_सूचकांक_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  अगर( ck.bUnidx ){
    rc = 0;
  }अन्यथा अगर( ck.b_व्यंजक ){
    rc = WHERE_EXPRIDX;
  }अन्यथा{
    rc = WHERE_IDX_ONLY;
  }
  वापस rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
अटल व्योम where_Indexed_व्यंजक_Cleanup(sqlite3 *db, व्योम *pObject){
  Indexed_व्यंजक **pp = (Indexed_व्यंजक**)pObject;
  जबतक( *pp!=0 ){
    Indexed_व्यंजक *p = *pp;
    *pp = p->pIENext;
    sqlite3_व्यंजक_Delete(db, p->p_व्यंजक);
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
अटल व्योम where_Part_Idx_व्यंजक(
  Parse *pParse,                  /* Parse context */
  सूचकांक *pIdx,                    /* Partial index being processed */
  व्यंजक *pPart,                    /* WHERE clause being processed */
  Bitmask *pMask,                 /* Mask to clear bits in */
  पूर्णांक iIdxCur,                    /* Cursor number for index */
  SrcItem *pItem                  /* The FROM clause entry for the table */
){
  निश्चित( pItem==0 || (pItem->fg.jointype & JT_दायाँ)==0 );
  निश्चित( (pItem==0 || pMask==0) && (pMask!=0 || pItem!=0) );

  अगर( pPart->op==TK_AND ){
    where_Part_Idx_व्यंजक(pParse, pIdx, pPart->p_दायाँ, pMask, iIdxCur, pItem);
    pPart = pPart->p_बायाँ;
  }

  अगर( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    व्यंजक *p_बायाँ = pPart->p_बायाँ;
    व्यंजक *p_दायाँ = pPart->p_दायाँ;
    Table *p_बायाँ_Tab;            /* Table of the left operand */
    Column *p_बायाँ_Col;           /* Column of the left operand */
    u8 aff_दायाँ, aff_बायाँ;

    अगर( p_बायाँ->op!=TK_COLUMN ) वापस;
    अगर( !sqlite3_व्यंजक_Is_Constant(0, p_दायाँ) ) वापस;
    अगर( !sqlite3_Is_द्विआधारी(sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pPart)) ) वापस;
    अगर( p_बायाँ->iColumn<0 ) वापस;
    p_बायाँ_Tab = pIdx->pTable;
    p_बायाँ_Col = &p_बायाँ_Tab->aCol[p_बायाँ->iColumn];
    aff_बायाँ = p_बायाँ_Col->affinity;
    निश्चित( SQLITE_AFF_BLOB<SQLITE_AFF_TEXT );
    अगर( aff_बायाँ<SQLITE_AFF_TEXT ) वापस;
    aff_दायाँ = sqlite3_व्यंजक_Affinity(p_दायाँ);
    अगर( aff_दायाँ!=0 && aff_दायाँ!=aff_बायाँ ) वापस;
    अगर( pItem ){
      sqlite3 *db = pParse->db;
      Indexed_व्यंजक *p = (Indexed_व्यंजक*)sqlite3DbMallocRaw(db, माप(*p));
      अगर( p ){
        पूर्णांक bNullRow = (pItem->fg.jointype&(JT_बायाँ|JT_LTORJ))!=0;
        p_दायाँ = sqlite3_व्यंजक_Dup(db, p_दायाँ, 0);
        अगर( (p_बायाँ_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3ColumnType(p_बायाँ_Col, 0);
          निश्चित( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (पूर्णांक)म_माप(t.z);
          p->p_व्यंजक = sqlite3_व्यंजक_Alloc(db, TK_CAST, &t, 1);
          sqlite3_व्यंजक_Attach_Subtrees(db, p->p_व्यंजक, p_दायाँ, 0);
        }अन्यथा{
          p->p_व्यंजक = p_दायाँ;
        }
        p->i_तथ्य_Cur = pItem->iCursor;
        p->iIdxCur = iIdxCur;
        p->iIdxCol = p_बायाँ->iColumn;
        p->bMaybeNullRow = bNullRow;
        p->pIENext = pParse->p_Idx_Part_व्यंजक;
        p->aff = aff_बायाँ;
        pParse->p_Idx_Part_व्यंजक = p;
        अगर( p->pIENext==0 ){
          व्योम *pArg = (व्योम*)&pParse->p_Idx_Part_व्यंजक;
          sqlite3ParserAddCleanup(pParse, where_Indexed_व्यंजक_Cleanup, pArg);
        }
      }
    }अन्यथा अगर( p_बायाँ->iColumn<(BMS-1) ){
      *pMask &= ~((Bitmask)1 << p_बायाँ->iColumn);
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
अटल पूर्णांक whereLoopAddBtree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  सूचकांक *pProbe;              /* An index we are evaluating */
  सूचकांक sPk;                  /* A fake index object for the primary key */
  LogEst aiRowEstPk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 aiColumnPk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  SrcItem *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  पूर्णांक rc = SQLITE_OK;         /* Return code */
  पूर्णांक iSortIdx;               /* Index number */
  पूर्णांक b;                      /* A boolean value */
  LogEst r_आकार;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  Table *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  निश्चित( !IsVirtual(pSrc->pSTab) );

  अगर( pSrc->fg.isIndexedBy ){
    निश्चित( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }अन्यथा अगर( !HasRowid(pTab) ){
    अगर( pSrc->fg.notIndexed ){
      pProbe = sqlite3_Primary_कुंजी_सूचकांक(pTab);
    }अन्यथा{
      pProbe = pTab->p_सूचकांक;
    }
  }अन्यथा{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    सूचकांक *pFirst;                  /* First of real indices on the table */
    स_रखो(&sPk, 0, माप(सूचकांक));
    sPk.n_कुंजी_Col = 1;
    sPk.nColumn = 1;
    sPk.aiColumn = &aiColumnPk;
    sPk.aiRowLogEst = aiRowEstPk;
    sPk.onError = OE_Replace;
    sPk.pTable = pTab;
    sPk.szIdxRow = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    aiRowEstPk[0] = pTab->nRowLogEst;
    aiRowEstPk[1] = 0;
    pFirst = pSrc->pSTab->p_सूचकांक;
    sPk.p_अग्र = pFirst;
    pProbe = &sPk;
  }
  r_आकार = pTab->nRowLogEst;

#अगर_अघोषित SQLITE_OMIT_AUTOMATIC_सूचकांक
  /* Automatic indexes */
  अगर( !pBuilder->pOrSet      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_दायाँ_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_सूचकांक)!=0
   && !pSrc->fg.isIndexedBy  /* Has no INDEXED BY clause */
   && !pSrc->fg.notIndexed   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_दायाँ)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_आकार;         /* Logarithm of the number of rows in the table */
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    r_Log_आकार = estLog(r_आकार);
    क्रम(pTerm=pWC->a; rc==SQLITE_OK && pTerm<pWCEnd; pTerm++){
      अगर( pTerm->prereq_दायाँ & pNew->maskSelf ) जारी;
      अगर( term_Can_Drive_सूचकांक(pTerm, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_सूचकांक = 0;
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
        pNew->rSetup = r_Log_आकार + r_आकार;
        अगर( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->rSetup += 28;
        }अन्यथा{
          pNew->rSetup -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        ApplyCostMultiplier(pNew->rSetup, pTab->costMult);
        अगर( pNew->rSetup<0 ) pNew->rSetup = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->nOut = 43;  निश्चित( 43==sqlite3LogEst(20) );
        pNew->rRun = sqlite3LogEstAdd(r_Log_आकार,pNew->nOut);
        pNew->wsFlags = WHERE_AUTO_सूचकांक;
        pNew->prereq = mPrereq | pTerm->prereq_दायाँ;
        rc = where_Loop_जोड़ो(pBuilder, pNew);
      }
    }
  }
#पूर्ण_अगर /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  क्रम(iSortIdx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.isIndexedBy||pSrc->fg.notIndexed)?0:pProbe->p_अग्र,
         iSortIdx++
  ){
    अगर( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_सूचकांक(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      जारी;  /* Partial index inappropriate for this query */
    }
    अगर( pProbe->bNoQuery ) जारी;
    r_आकार = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_शीर्ष = 0;
    pNew->u.btree.nDistinctCol = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->iSortIdx = 0;
    pNew->rSetup = 0;
    pNew->prereq = mPrereq;
    pNew->nOut = r_आकार;
    pNew->u.btree.p_सूचकांक = pProbe;
    pNew->u.btree.pOrderBy = 0;
    b = सूचकांक_Might_Help_With_Order_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    निश्चित( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    अगर( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
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
#अगर_घोषित SQLITE_ENABLE_STAT4
      pNew->rRun = r_आकार + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#अन्यथा
      pNew->rRun = r_आकार + 16;
#पूर्ण_अगर
      ApplyCostMultiplier(pNew->rRun, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, r_आकार);
      अगर( pSrc->fg.isSubquery ){
        अगर( pSrc->fg.viaCoroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        अगर( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.pOrderBy = pSrc->u4.pSubq->pSelect->pOrderBy;
        }
      }अन्यथा अगर( pSrc->fg.fromExists ){
        pNew->nOut = 0;
      }
      rc = where_Loop_जोड़ो(pBuilder, pNew);
      pNew->nOut = r_आकार;
      अगर( rc ) अवरोध;
    }अन्यथा{
      Bitmask m;
      अगर( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
      }अन्यथा{
        m = pSrc->colUsed & pProbe->colNotIdxed;
        अगर( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_व्यंजक(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_INDEXED;
        अगर( m==TOPBIT || (pProbe->b_Has_व्यंजक && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_सूचकांक(pWInfo, pProbe, pSrc->iCursor);
          अगर( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            निश्चित( m!=0 );
          }अन्यथा{
            m = 0;
            pNew->wsFlags |= isCov;
            अगर( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }अन्यथा{
              निश्चित( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }अन्यथा अगर( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
        }
      }

      /* Full scan via index */
      अगर( b
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
        pNew->rRun = r_आकार + 1 + (15*pProbe->szIdxRow)/pTab->szTabRow;
        अगर( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_आकार + 16;  /* Base cost:  N*3 */
          पूर्णांक ii;
          पूर्णांक iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          क्रम(ii=0; ii<pWC2->nTerm; ii++){
            WhereTerm *pTerm = &pWC2->a[ii];
            अगर( !sqlite3_व्यंजक_Covered_By_सूचकांक(pTerm->p_व्यंजक, iCur, pProbe) ){
              अवरोध;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            अगर( pTerm->truthProb<=0 ){
              nLookup += pTerm->truthProb;
            }अन्यथा{
              nLookup--;
              अगर( pTerm->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, nLookup);
        }
        ApplyCostMultiplier(pNew->rRun, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, r_आकार);
        अगर( (pSrc->fg.jointype & JT_दायाँ)!=0 && pProbe->a_Col_व्यंजक ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }अन्यथा{
          अगर( pSrc->fg.fromExists ) pNew->nOut = 0;
          rc = where_Loop_जोड़ो(pBuilder, pNew);
        }
        pNew->nOut = r_आकार;
        अगर( rc ) अवरोध;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_Add_Btree_सूचकांक(pBuilder, pSrc, pProbe, 0);
    अगर( pBuilder->bldFlags1==SQLITE_BLDF1_INDEXED ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#अगर_घोषित SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#पूर्ण_अगर
  }
  वापस rc;
}

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
अटल पूर्णांक isLimitTerm(WhereTerm *pTerm){
  निश्चित( pTerm->eOperator==WO_AUX || pTerm->eMatchOp==0 );
  वापस pTerm->eMatchOp>=SQLITE_सूचकांक_CONSTRAINT_LIMIT
      && pTerm->eMatchOp<=SQLITE_सूचकांक_CONSTRAINT_OFFSET;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
अटल पूर्णांक allConstraintsUsed(
  काष्ठा sqlite3_सूचकांक_constraint_usage *aUsage, 
  पूर्णांक nCons
){
  पूर्णांक ii;
  क्रम(ii=0; ii<nCons; ii++){
    अगर( aUsage[ii].argv_सूचकांक<=0 ) वापस 0;
  }
  वापस 1;
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
अटल पूर्णांक whereLoopAddVirtualOne(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_सूचकांक_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  पूर्णांक *pbIn,                      /* OUT: True if plan uses an IN(...) op */
  पूर्णांक *pbRetryLimit               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  काष्ठा sqlite3_सूचकांक_constraint *pIdxCons;
  काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  पूर्णांक i;
  पूर्णांक mxTerm;
  पूर्णांक rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  पूर्णांक nConstraint = pIdxInfo->nConstraint;

  निश्चित( (mUsable & mPrereq)==mPrereq );
  *pbIn = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(काष्ठा sqlite3_सूचकांक_constraint**)&pIdxInfo->aConstraint;
  क्रम(i=0; i<nConstraint; i++, pIdxCons++){
    WhereTerm *pTerm = termFromWhereClause(pWC, pIdxCons->iTermOffset);
    pIdxCons->usable = 0;
    अगर( (pTerm->prereq_दायाँ & mUsable)==pTerm->prereq_दायाँ
     && (pTerm->eOperator & mExclude)==0
     && (pbRetryLimit || !isLimitTerm(pTerm))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  स_रखो(pUsage, 0, माप(pUsage[0])*nConstraint);
  निश्चित( pIdxInfo->needToFreeIdxStr==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_अंक = 0;
  pIdxInfo->orderByConsumed = 0;
  pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (द्विगुन)2;
  pIdxInfo->estimatedRows = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->mHandleIn = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_Best_सूचकांक(pParse, pSrc->pSTab, pIdxInfo);
  अगर( rc ){
    अगर( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      वापस SQLITE_OK;
    }
    वापस rc;
  }

  mxTerm = -1;
  निश्चित( pNew->nLSlot>=nConstraint );
  स_रखो(pNew->aLTerm, 0, माप(pNew->aLTerm[0])*nConstraint );
  स_रखो(&pNew->u.vtab, 0, माप(pNew->u.vtab));
  pIdxCons = *(काष्ठा sqlite3_सूचकांक_constraint**)&pIdxInfo->aConstraint;
  क्रम(i=0; i<nConstraint; i++, pIdxCons++){
    पूर्णांक iTerm;
    अगर( (iTerm = pUsage[i].argv_सूचकांक - 1)>=0 ){
      WhereTerm *pTerm;
      पूर्णांक j = pIdxCons->iTermOffset;
      अगर( iTerm>=nConstraint
       || j<0
       || (pTerm = termFromWhereClause(pWC, j))==0
       || pNew->aLTerm[iTerm]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_संदेश(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        वापस SQLITE_ERROR;
      }
      testcase( iTerm==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->nTerm-1 );
      pNew->prereq |= pTerm->prereq_दायाँ;
      निश्चित( iTerm<pNew->nLSlot );
      pNew->aLTerm[iTerm] = pTerm;
      अगर( iTerm>mxTerm ) mxTerm = iTerm;
      testcase( iTerm==15 );
      testcase( iTerm==16 );
      अगर( pUsage[i].omit ){
        अगर( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=iTerm );
          pNew->u.vtab.omitMask |= 1<<iTerm;
        }अन्यथा{
          testcase( i!=iTerm );
        }
        अगर( pTerm->eMatchOp==SQLITE_सूचकांक_CONSTRAINT_OFFSET ){
          pNew->u.vtab.bOmitOffset = 1;
        }
      }
      अगर( SMASKBIT32(i) & pHidden->mHandleIn ){
        pNew->u.vtab.mHandleIn |= MASKBIT32(iTerm);
      }अन्यथा अगर( (pTerm->eOperator & WO_IN)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->orderByConsumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_सूचकांक_SCAN_UNIQUE;
        *pbIn = 1; निश्चित( (mExclude & WO_IN)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      निश्चित( pbRetryLimit || !isLimitTerm(pTerm) );
      निश्चित( !isLimitTerm(pTerm) || i>=nConstraint-2 );
      निश्चित( !isLimitTerm(pTerm) || i==nConstraint-1 || isLimitTerm(pTerm+1) );

      अगर( isLimitTerm(pTerm) && (*pbIn || !allConstraintsUsed(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pbRetryLimit = 1;
        वापस SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mxTerm+1;
  क्रम(i=0; i<=mxTerm; i++){
    अगर( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_संदेश(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      वापस SQLITE_ERROR;
    }
  }
  निश्चित( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_अंक = pIdxInfo->idx_अंक;
  pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
  pIdxInfo->needToFreeIdxStr = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ?
      pIdxInfo->nOrderBy : 0);
  pNew->u.vtab.b_Idx_अंक_Hex = (pIdxInfo->idxFlags&SQLITE_सूचकांक_SCAN_HEX)!=0;
  pNew->rSetup = 0;
  pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  अगर( pIdxInfo->idxFlags & SQLITE_सूचकांक_SCAN_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }अन्यथा{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_जोड़ो(pBuilder, pNew);
  अगर( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pbIn, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  वापस rc;
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
स्थिर अक्षर *sqlite3_vtab_collation(sqlite3_सूचकांक_info *pIdxInfo, पूर्णांक iCons){
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  स्थिर अक्षर *zRet = 0;
  अगर( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    पूर्णांक iTerm = pIdxInfo->aConstraint[iCons].iTermOffset;
    व्यंजक *pX = termFromWhereClause(pHidden->pWC, iTerm)->p_व्यंजक;
    अगर( pX->p_बायाँ ){
      pC = sqlite3_व्यंजक_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_द्विआधारी);
  }
  वापस zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
पूर्णांक sqlite3_vtab_in(sqlite3_सूचकांक_info *pIdxInfo, पूर्णांक iCons, पूर्णांक bHandle){
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  अगर( m & pHidden->mIn ){
    अगर( bHandle==0 ){
      pHidden->mHandleIn &= ~m;
    }अन्यथा अगर( bHandle>0 ){
      pHidden->mHandleIn |= m;
    }
    वापस 1;
  }
  वापस 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
पूर्णांक sqlite3_vtab_rhs_मूल्य(
  sqlite3_सूचकांक_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  पूर्णांक iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_मूल्य **ppVal           /* Write value extracted here */
){
  Hidden_सूचकांक_Info *pH = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  sqlite3_मूल्य *pVal = 0;
  पूर्णांक rc = SQLITE_OK;
  अगर( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }अन्यथा{
    अगर( pH->aRhs[iCons]==0 ){
      WhereTerm *pTerm = termFromWhereClause(
          pH->pWC, pIdxInfo->aConstraint[iCons].iTermOffset
      );
      rc = sqlite3_मूल्य_From_व्यंजक(
          pH->pParse->db, pTerm->p_व्यंजक->p_दायाँ, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  अगर( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  वापस rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
पूर्णांक sqlite3_vtab_distinct(sqlite3_सूचकांक_info *pIdxInfo){
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  निश्चित( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  वापस pHidden->eDistinct;
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
व्योम sqlite3VtabUsesAllSchemas(Parse *pParse){
  पूर्णांक nDb = pParse->db->nDb;
  पूर्णांक i;
  क्रम(i=0; i<nDb; i++){
    sqlite3CodeVerifySchema(pParse, i);
  }
  अगर( DbMaskNonZero(pParse->writeMask) ){
    क्रम(i=0; i<nDb; i++){
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
अटल पूर्णांक whereLoopAddVirtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  पूर्णांक rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  SrcItem *pSrc;               /* The FROM clause term to search */
  sqlite3_सूचकांक_info *p;       /* Object to pass to xBestIndex() */
  पूर्णांक nConstraint;             /* Number of constraints in p */
  पूर्णांक bIn;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask mBest;               /* Tables used by best possible plan */
  u16 mNoOmit;
  पूर्णांक bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  निश्चित( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  निश्चित( IsVirtual(pSrc->pSTab) );
  p = allocate_सूचकांक_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  अगर( p==0 ) वापस SQLITE_NOMEM_BKPT;
  pNew->rSetup = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  अगर( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_सूचकांक_Info(pParse->db, p);
    वापस SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = whereLoopAddVirtualOne(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, &bRetry
  );
  अगर( bRetry ){
    निश्चित( rc==SQLITE_OK );
    rc = whereLoopAddVirtualOne(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  अगर( rc==SQLITE_OK && ((mBest = (pNew->prereq & ~mPrereq))!=0 || bIn) ){
    पूर्णांक seenZero = 0;             /* True if a plan with no prereqs seen */
    पूर्णांक seenZeroNoIN = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask m_पूर्व = 0;
    Bitmask mBestNoIn = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    अगर( bIn ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, ALLBITS, WO_IN, p, mNoOmit, &bIn, 0);
      निश्चित( bIn==0 );
      mBestNoIn = pNew->prereq & ~mPrereq;
      अगर( mBestNoIn==0 ){
        seenZero = 1;
        seenZeroNoIN = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    जबतक( rc==SQLITE_OK ){
      पूर्णांक i;
      Bitmask m_अग्र = ALLBITS;
      निश्चित( m_अग्र>0 );
      क्रम(i=0; i<nConstraint; i++){
        पूर्णांक iTerm = p->aConstraint[i].iTermOffset;
        Bitmask mThis = termFromWhereClause(pWC, iTerm)->prereq_दायाँ & ~mPrereq;
        अगर( mThis>m_पूर्व && mThis<m_अग्र ) m_अग्र = mThis;
      }
      m_पूर्व = m_अग्र;
      अगर( m_अग्र==ALLBITS ) अवरोध;
      अगर( m_अग्र==mBest || m_अग्र==mBestNoIn ) जारी;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)m_पूर्व, (sqlite3_uint64)m_अग्र));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, m_अग्र|mPrereq, 0, p, mNoOmit, &bIn, 0);
      अगर( pNew->prereq==mPrereq ){
        seenZero = 1;
        अगर( bIn==0 ) seenZeroNoIN = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    अगर( rc==SQLITE_OK && seenZero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &bIn, 0);
      अगर( bIn==0 ) seenZeroNoIN = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    अगर( rc==SQLITE_OK && seenZeroNoIN==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, WO_IN, p, mNoOmit, &bIn, 0);
    }
  }

  free_सूचकांक_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
अटल पूर्णांक whereLoopAddOr(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  WhereTerm *pTerm, *pWCEnd;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक iCur;
  WhereClause अस्थायी_WC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet s_योग, sCur;
  SrcItem *pItem;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  स_रखो(&s_योग, 0, माप(s_योग));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  अगर( pItem->fg.jointype & JT_दायाँ ) वापस SQLITE_OK;

  क्रम(pTerm=pWC->a; pTerm<pWCEnd && rc==SQLITE_OK; pTerm++){
    अगर( (pTerm->eOperator & WO_OR)!=0
     && (pTerm->u.pOrInfo->indexable & pNew->maskSelf)!=0
     && (!व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_OuterON)
          || pTerm->p_व्यंजक->w.iJoin==iCur)
    ){
      WhereClause * स्थिर pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm * स्थिर pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      पूर्णांक once = 1;
      पूर्णांक i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.pOrSet = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", pTerm));
      क्रम(pOrTerm=pOrWC->a; pOrTerm<pOrWCEnd; pOrTerm++){
        अगर( (pOrTerm->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        }अन्यथा अगर( pOrTerm->बायाँ_Cursor==iCur ){
          अस्थायी_WC.pWInfo = pWC->pWInfo;
          अस्थायी_WC.pOuter = pWC;
          अस्थायी_WC.op = TK_AND;
          अस्थायी_WC.nTerm = 1;
          अस्थायी_WC.nBase = 1;
          अस्थायी_WC.a = pOrTerm;
          sSubBuild.pWC = &अस्थायी_WC;
        }अन्यथा{
          जारी;
        }
        sCur.n = 0;
#अगर_घोषित WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (पूर्णांक)(pOrTerm-pOrWC->a), pTerm, sSubBuild.pWC->nTerm));
        अगर( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
        अगर( IsVirtual(pItem->pSTab) ){
          rc = whereLoopAddVirtual(&sSubBuild, mPrereq, mUnusable);
        }अन्यथा
#पूर्ण_अगर
        {
          rc = whereLoopAddBtree(&sSubBuild, mPrereq);
        }
        अगर( rc==SQLITE_OK ){
          rc = whereLoopAddOr(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_DONE );
        अगर( sCur.n==0 ){
          s_योग.n = 0;
          अवरोध;
        }अन्यथा अगर( once ){
          whereOrMove(&s_योग, &sCur);
          once = 0;
        }अन्यथा{
          WhereOrSet s_पूर्व;
          whereOrMove(&s_पूर्व, &s_योग);
          s_योग.n = 0;
          क्रम(i=0; i<s_पूर्व.n; i++){
            क्रम(j=0; j<sCur.n; j++){
              where_Or_जोड़ो(&s_योग, s_पूर्व.a[i].prereq | sCur.a[j].prereq,
                            sqlite3LogEstAdd(s_पूर्व.a[i].rRun, sCur.a[j].rRun),
                            sqlite3LogEstAdd(s_पूर्व.a[i].nOut, sCur.a[j].nOut));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = pTerm;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->rSetup = 0;
      pNew->iSortIdx = 0;
      स_रखो(&pNew->u, 0, माप(pNew->u));
      क्रम(i=0; rc==SQLITE_OK && i<s_योग.n; i++){
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
        pNew->rRun = s_योग.a[i].rRun + 1;
        pNew->nOut = s_योग.a[i].nOut;
        pNew->prereq = s_योग.a[i].prereq;
        rc = where_Loop_जोड़ो(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", pTerm));
    }
  }
  वापस rc;
}

/*
** Add all WhereLoop objects for all tables
*/
अटल पूर्णांक whereLoopAddAll(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask mPrior = 0;
  पूर्णांक iTab;
  SrcList *pTabList = pWInfo->pTabList;
  SrcItem *pItem;
  SrcItem *pEnd = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक bFirstPastRJ = 0;
  पूर्णांक has_दायाँ_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  निश्चित( pNew->nLTerm==0 );
  निश्चित( pNew->wsFlags==0 );
  निश्चित( pNew->nLSlot>=Array_आकार(pNew->aLTermSpace) );
  निश्चित( pNew->aLTerm!=0 );

  pBuilder->iPlanLimit = SQLITE_QUERY_PLANNER_LIMIT;
  क्रम(iTab=0, pItem=pTabList->a; pItem<pEnd; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->iPlanLimit += SQLITE_QUERY_PLANNER_LIMIT_INCR;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    अगर( bFirstPastRJ
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
      अगर( pItem->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( pItem->fg.jointype & JT_LTORJ );
        testcase( pItem->fg.jointype & JT_CROSS );
        has_दायाँ_Cross_Join = 1;
      }
      mPrereq |= mPrior;
      bFirstPastRJ = (pItem->fg.jointype & JT_दायाँ)!=0;
    }अन्यथा अगर( pItem->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      WhereTerm *pTerm;
      पूर्णांक i;
      क्रम(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
        अगर( (pNew->maskSelf & pTerm->prereqAll)!=0 ){
          mPrereq |= (pTerm->prereqAll & (pNew->maskSelf-1));
        }
      }
    }अन्यथा अगर( !has_दायाँ_Cross_Join ){
      mPrereq = 0;
    }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    अगर( IsVirtual(pItem->pSTab) ){
      SrcItem *p;
      क्रम(p=&pItem[1]; p<pEnd; p++){
        अगर( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mPrereq, mUnusable);
    }अन्यथा
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = whereLoopAddBtree(pBuilder, mPrereq);
    }
    अगर( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = whereLoopAddOr(pBuilder, mPrereq, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    अगर( rc || db->mallocFailed ){
      अगर( rc==SQLITE_DONE ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }अन्यथा{
        अवरोध;
      }
    }
  }

  whereLoopClear(db, pNew);
  वापस rc;
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
अटल SQLITE_NOINLINE पूर्णांक wherePathMatchSubqueryOB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  पूर्णांक iLoop,              /* Which level of the nested loop.  0==outermost */
  पूर्णांक iCur,               /* Cursor used by the this loop */
  व्यंजक_List *pOrderBy,     /* The ORDER BY clause on the whole query */
  Bitmask *pRevMask,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  पूर्णांक iOB;                /* Index into pOrderBy->a[] */
  पूर्णांक jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  व्यंजक *pOBExpr;          /* Current term of outer ORDER BY */
  व्यंजक_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.pOrderBy;
  निश्चित( pSubOB!=0 );
  क्रम(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  क्रम(jSub=0; jSub<pSubOB->n_व्यंजक && iOB<pOrderBy->n_व्यंजक; jSub++, iOB++){
    अगर( pSubOB->a[jSub].u.x.iOrderByCol==0 ) अवरोध;
    pOBExpr = pOrderBy->a[iOB].p_व्यंजक;
    अगर( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) अवरोध;
    अगर( pOBExpr->iTable!=iCur ) अवरोध;
    अगर( pOBExpr->iColumn!=pSubOB->a[jSub].u.x.iOrderByCol-1 ) अवरोध;
    अगर( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = pOrderBy->a[iOB].fg.sortFlags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.sortFlags;   /* sortFlags for jSub */
      अगर( (sfSub & KEYINFO_ORDER_BIGNULL) != (sfOB & KEYINFO_ORDER_BIGNULL) ){
        अवरोध;
      }
      revIdx = sfSub & KEYINFO_ORDER_DESC;
      अगर( jSub>0 ){
        अगर( (rev^revIdx)!=(sfOB & KEYINFO_ORDER_DESC) ){
          अवरोध;
        }
      }अन्यथा{
        rev = revIdx ^ (sfOB & KEYINFO_ORDER_DESC);
        अगर( rev ){
          अगर( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            अवरोध;
          }
          *pRevMask |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  वापस jSub>0;
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
अटल i8 wherePathSatisfiesOrderBy(
  WhereInfo *pWInfo,    /* The WHERE clause */
  व्यंजक_List *pOrderBy,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
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
  u16 n_कुंजी_Col;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 nOrderBy;         /* Number terms in the ORDER BY clause */
  पूर्णांक iLoop;            /* Index of WhereLoop in pPath being processed */
  पूर्णांक i, j;             /* Loop counters */
  पूर्णांक iCur;             /* Cursor number for current WhereLoop */
  पूर्णांक iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  WhereTerm *pTerm;     /* A single term of the WHERE clause */
  व्यंजक *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  सूचकांक *p_सूचकांक;        /* The index associated with pLoop */
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

  निश्चित( pOrderBy!=0 );
  अगर( nLoop && OptimizationDisabled(db, SQLITE_OrderByIdxJoin) ) वापस 0;

  nOrderBy = pOrderBy->n_व्यंजक;
  testcase( nOrderBy==BMS-1 );
  अगर( nOrderBy>BMS-1 ) वापस 0;  /* Cannot optimize overly large ORDER BYs */
  isOrderDistinct = 1;
  obDone = MASKBIT(nOrderBy)-1;
  orderDistinctMask = 0;
  ready = 0;
  eqOpMask = WO_EQ | WO_IS | WO_ISNULL;
  अगर( wctrlFlags & (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eqOpMask |= WO_IN;
  }
  क्रम(iLoop=0; isOrderDistinct && obSat<obDone && iLoop<=nLoop; iLoop++){
    अगर( iLoop>0 ) ready |= pLoop->maskSelf;
    अगर( iLoop<nLoop ){
      pLoop = pPath->aLoop[iLoop];
      अगर( wctrlFlags & WHERE_ORDERBY_LIMIT ) जारी;
    }अन्यथा{
      pLoop = pLast;
    }
    अगर( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      अगर( pLoop->u.vtab.isOrdered && pWInfo->pOrderBy==pOrderBy ){
        obSat = obDone;
      }अन्यथा{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        isOrderDistinct = 0;
      }
      अवरोध;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    क्रम(i=0; i<nOrderBy; i++){
      अगर( MASKBIT(i) & obSat ) जारी;
      pOBExpr = sqlite3_व्यंजक_Skip_Collate_And_Likely(pOrderBy->a[i].p_व्यंजक);
      अगर( NEVER(pOBExpr==0) ) जारी;
      अगर( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) जारी;
      अगर( pOBExpr->iTable!=iCur ) जारी;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, eqOpMask, 0);
      अगर( pTerm==0 ) जारी;
      अगर( pTerm->eOperator==WO_IN ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        निश्चित( wctrlFlags &
               (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        क्रम(j=0; j<pLoop->nLTerm && pTerm!=pLoop->aLTerm[j]; j++){}
        अगर( j>=pLoop->nLTerm ) जारी;
      }
      अगर( (pTerm->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_व्यंजक_NNColl_Seq(pParse, pOrderBy->a[i].p_व्यंजक);
        स्थिर व्यंजक *pCExpr = pTerm->p_व्यंजक;
        CollSeq *pColl2 = sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pCExpr);
        अक्षर aff_दायाँ;
        निश्चित( pColl1 );
        अगर( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          जारी;
        }
        aff_दायाँ = sqlite3_व्यंजक_Affinity(pCExpr->p_दायाँ);
        /* Left operand must be a column, hence always has affinity */
        निश्चित( sqlite3_व्यंजक_Affinity(pCExpr->p_बायाँ)!=0 );
        अगर( aff_दायाँ!=0 && aff_दायाँ!=sqlite3_व्यंजक_Affinity(pCExpr->p_बायाँ) ){
          जारी;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    अगर( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      अगर( pLoop->wsFlags & WHERE_IPK ){
        अगर( pLoop->u.btree.pOrderBy
         && OptimizationEnabled(db, SQLITE_OrderBySubq)
         &&  wherePathMatchSubqueryOB(pLoop,iLoop,iCur,
                                     pOrderBy,pRevMask, &obSat, wctrlFlags)
        ){
          nColumn = 0;
          isOrderDistinct = 0;
        }अन्यथा{
          nColumn = 1;
        }
        p_सूचकांक = 0;
        n_कुंजी_Col = 1;
      }अन्यथा अगर( (p_सूचकांक = pLoop->u.btree.p_सूचकांक)==0 || p_सूचकांक->bUnordered ){
        वापस 0;
      }अन्यथा{
        n_कुंजी_Col = p_सूचकांक->n_कुंजी_Col;
        nColumn = p_सूचकांक->nColumn;
        निश्चित( nColumn==n_कुंजी_Col+1 || !HasRowid(p_सूचकांक->pTable) );
        निश्चित( p_सूचकांक->aiColumn[nColumn-1]==XN_ROWID
                          || !HasRowid(p_सूचकांक->pTable));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        isOrderDistinct = Is_Unique_सूचकांक(p_सूचकांक)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = revSet = 0;
      distinctColumns = 0;
      क्रम(j=0; j<nColumn; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        निश्चित( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        अगर( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          अगर( (eOp & eqOpMask)!=0 ){
            अगर( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( isOrderDistinct );
              isOrderDistinct = 0;
            }
            जारी; 
          }अन्यथा अगर( ALWAYS(eOp & WO_IN) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            व्यंजक *pX = pLoop->aLTerm[j]->p_व्यंजक;
            क्रम(i=j+1; i<pLoop->u.btree.nEq; i++){
              अगर( pLoop->aLTerm[i]->p_व्यंजक==pX ){
                निश्चित( (pLoop->aLTerm[i]->eOperator & WO_IN) );
                bOnce = 0;
                अवरोध;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        अगर( p_सूचकांक ){
          iColumn = p_सूचकांक->aiColumn[j];
          revIdx = p_सूचकांक->aSortOrder[j] & KEYINFO_ORDER_DESC;
          अगर( iColumn==p_सूचकांक->pTable->iPKey ) iColumn = XN_ROWID;
        }अन्यथा{
          iColumn = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        अगर( isOrderDistinct ){
          अगर( iColumn>=0
           && j>=pLoop->u.btree.nEq
           && p_सूचकांक->pTable->aCol[iColumn].notNull==0
          ){
            isOrderDistinct = 0;
          }
          अगर( iColumn==XN_व्यंजक ){
            isOrderDistinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        isMatch = 0;
        क्रम(i=0; bOnce && i<nOrderBy; i++){
          अगर( MASKBIT(i) & obSat ) जारी;
          pOBExpr = sqlite3_व्यंजक_Skip_Collate_And_Likely(pOrderBy->a[i].p_व्यंजक);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          अगर( NEVER(pOBExpr==0) ) जारी;
          अगर( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          अगर( iColumn>=XN_ROWID ){
            अगर( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) जारी;
            अगर( pOBExpr->iTable!=iCur ) जारी;
            अगर( pOBExpr->iColumn!=iColumn ) जारी;
          }अन्यथा{
            व्यंजक *p_Ix_व्यंजक = p_सूचकांक->a_Col_व्यंजक->a[j].p_व्यंजक;
            अगर( sqlite3_व्यंजक_Compare_Skip(pOBExpr, p_Ix_व्यंजक, iCur) ){
              जारी;
            }
          }
          अगर( iColumn!=XN_ROWID ){
            pColl = sqlite3_व्यंजक_NNColl_Seq(pWInfo->pParse, pOrderBy->a[i].p_व्यंजक);
            अगर( sqlite3StrICmp(pColl->zName, p_सूचकांक->azColl[j])!=0 ) जारी;
          }
          अगर( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.nDistinctCol = j+1;
          }
          isMatch = 1;
          अवरोध;
        }
        अगर( isMatch && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          अगर( revSet ){
            अगर( (rev ^ revIdx)
                           != (pOrderBy->a[i].fg.sortFlags&KEYINFO_ORDER_DESC)
            ){
              isMatch = 0;
            }
          }अन्यथा{
            rev = revIdx ^ (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC);
            अगर( rev ) *pRevMask |= MASKBIT(iLoop);
            revSet = 1;
          }
        }
        अगर( isMatch && (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL) ){
          अगर( j==pLoop->u.btree.nEq 
           && (j<n_कुंजी_Col||HasRowid(p_सूचकांक->pTable)||!Is_Primary_कुंजी_सूचकांक(p_सूचकांक))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_SORT;
          }अन्यथा{
            isMatch = 0;
          }
        }
        अगर( isMatch ){
          अगर( iColumn==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }अन्यथा{
          /* No match found */
          निश्चित( n_कुंजी_Col>0 );
          अगर( j<n_कुंजी_Col ){
            testcase( isOrderDistinct!=0 );
            isOrderDistinct = 0;
          }
          अवरोध;
        }
      } /* end Loop over all index columns */
      अगर( distinctColumns ){
        testcase( isOrderDistinct==0 );
        isOrderDistinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    अगर( isOrderDistinct ){
      orderDistinctMask |= pLoop->maskSelf;
      क्रम(i=0; i<nOrderBy; i++){
        व्यंजक *p;
        Bitmask mTerm;
        अगर( MASKBIT(i) & obSat ) जारी;
        p = pOrderBy->a[i].p_व्यंजक;
        mTerm = sqlite3_Where_व्यंजक_Usage(&pWInfo->sMaskSet,p);
        अगर( mTerm==0 && !sqlite3_व्यंजक_Is_Constant(0,p) ) जारी;
        अगर( (mTerm&~orderDistinctMask)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  अगर( obSat==obDone ) वापस (i8)nOrderBy;
  अगर( !isOrderDistinct ){
    क्रम(i=nOrderBy-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      अगर( (obSat&m)==m ) वापस i;
    }
    वापस 0;
  }
  वापस -1;
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
पूर्णांक sqlite3WhereIsSorted(WhereInfo *pWInfo){
  निश्चित( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  निश्चित( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  वापस pWInfo->sorted;
}

#अगर_घोषित WHERETRACE_ENABLED
/* For debugging use only: */
अटल स्थिर अक्षर *wherePathName(WherePath *pPath, पूर्णांक nLoop, WhereLoop *pLast){
  अटल अक्षर zName[65];
  पूर्णांक i;
  क्रम(i=0; i<nLoop; i++){ zName[i] = pPath->aLoop[i]->cId; }
  अगर( pLast ) zName[i++] = pLast->cId;
  zName[i] = 0;
  वापस zName;
}
#पूर्ण_अगर

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
अटल LogEst whereSortingCost(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst nRow,       /* Estimated number of rows to sort */
  पूर्णांक nOrderBy,      /* Number of ORDER BY clause terms */
  पूर्णांक nSorted        /* Number of initial ORDER BY terms naturally in order */
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
  निश्चित( pWInfo->pSelect!=0 );
  निश्चित( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->n_व्यंजक+59)/30);
  rSortCost = nRow + nCol;
  अगर( nSorted>0 ){
    /* Scale the result by (Y/X) */
    rSortCost += sqlite3LogEst((nOrderBy-nSorted)*100/nOrderBy) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  अगर( (pWInfo->wctrlFlags & WHERE_USE_LIMIT)!=0 ){
    rSortCost += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    अगर( nSorted!=0 ){
      rSortCost += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    अगर( pWInfo->iLimit<nRow ){
      nRow = pWInfo->iLimit;
    }
  }अन्यथा अगर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    अगर( nRow>10 ){ nRow -= 10;  निश्चित( 10==sqlite3LogEst(2) ); }
  }
  rSortCost += estLog(nRow);
  वापस rSortCost;
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
अटल पूर्णांक computeMxChoice(WhereInfo *pWInfo){
  पूर्णांक nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#अगर_घोषित SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  क्रम(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
    निश्चित( pWLoop->maskSelf==MASKBIT(pWLoop->iTab) );
    निश्चित( pWLoop->p_अग्र_Loop==0 || pWLoop->iTab<=pWLoop->p_अग्र_Loop->iTab );
  }
#पूर्ण_अगर /* SQLITE_DEBUG */

  अगर( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->bStarDone
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_StarQuery)
  ){
    SrcItem *aFromTabs;    /* All terms of the FROM clause */
    पूर्णांक iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *pStart;     /* Where to start searching for dimension-tables */

    pWInfo->bStarDone = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    निश्चित( !pWInfo->bStarUsed );
    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    क्रम(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      पूर्णांक nDep = 0;             /* Number of dimension tables */
      LogEst mxRun;             /* Maximum SCAN cost of a fact table */
      Bitmask mSeen = 0;        /* Mask of dimension tables */
      SrcItem *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      अगर( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        अगर( iFromIdx+3 > nLoop ){
          अवरोध;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        जबतक( pStart && pStart->iTab<=iFromIdx ){
          pStart = pStart->p_अग्र_Loop;
        }
      }
      क्रम(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        अगर( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          अवरोध; /* Constraint (bb) */
        }
        अगर( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->maskSelf & mSeen)==0  /* pWInfo not already a dependency */
         && (pWLoop->maskSelf & mSelfJoin)==0 /* Not a self-join */
        ){
          अगर( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }अन्यथा{
            nDep++;
            mSeen |= pWLoop->maskSelf;
          }
        }
      }
      अगर( nDep<=2 ){
        जारी; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#अगर_घोषित WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      अगर( !pWInfo->bStarUsed ){
        क्रम(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
          pWLoop->rStarDelta = 0;
        }
      }
#पूर्ण_अगर
#अगर_घोषित WHERETRACE_ENABLED /* 0x80000 */
      अगर( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = mSeen;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        क्रम(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
          अगर( mShow & pWLoop->maskSelf ){
            SrcItem *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->maskSelf;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#पूर्ण_अगर
      pWInfo->bStarUsed = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mxRun = LOGEST_MIN;
      क्रम(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        अगर( pWLoop->iTab<iFromIdx ) जारी;
        अगर( pWLoop->iTab>iFromIdx ) अवरोध;
        अगर( pWLoop->rRun>mxRun ) mxRun = pWLoop->rRun;
      }
      अगर( ALWAYS(mxRun<LOGEST_MAX) ) mxRun++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      क्रम(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        अगर( (pWLoop->maskSelf & mSeen)==0 ) जारी;
        अगर( pWLoop->nLTerm ) जारी;
        अगर( pWLoop->rRun<mxRun ){
#अगर_घोषित WHERETRACE_ENABLED /* 0x80000 */
          अगर( sqlite3WhereTrace & 0x80000 ){
            SrcItem *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mxRun
            );
          }
          pWLoop->rStarDelta = mxRun - pWLoop->rRun;
#पूर्ण_अगर /* WHERETRACE_ENABLED */
          pWLoop->rRun = mxRun;
        }
      }
    }
#अगर_घोषित WHERETRACE_ENABLED /* 0x80000 */
    अगर( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->bStarUsed ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      क्रम(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        अगर( pWLoop->rStarDelta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#पूर्ण_अगर
  }
  वापस pWInfo->bStarUsed ? 18 : 12;
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
अटल SQLITE_NOINLINE पूर्णांक whereLoopIsNoBetter(
  स्थिर WhereLoop *pCandidate,
  स्थिर WhereLoop *pBaseline
){
  अगर( (pCandidate->wsFlags & WHERE_INDEXED)==0 ) वापस 1;
  अगर( (pBaseline->wsFlags & WHERE_INDEXED)==0 ) वापस 1;
  अगर( pCandidate->u.btree.p_सूचकांक->szIdxRow <
        pBaseline->u.btree.p_सूचकांक->szIdxRow ) वापस 0;
  वापस 1;
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
अटल पूर्णांक wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst){
  पूर्णांक mxChoice;             /* Maximum number of simultaneous paths tracked */
  पूर्णांक nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  पूर्णांक iLoop;                /* Loop counter over the terms of the join */
  पूर्णांक ii, jj;               /* Loop counters */
  पूर्णांक mxI = 0;              /* Index of next entry to replace */
  पूर्णांक nOrderBy;             /* Number of ORDER BY clause terms */
  LogEst mxCost = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  पूर्णांक nTo, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  WherePath *aFrom;         /* All nFrom paths at the previous level */
  WherePath *aTo;           /* The nTo best paths at the current level */
  WherePath *pFrom;         /* An element of aFrom[] that we are working on */
  WherePath *pTo;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *aSortCost = 0;    /* Sorting and partial sorting costs */
  अक्षर *pSpace;             /* Temporary memory used by this routine */
  पूर्णांक nSpace;               /* Bytes of space allocated at pSpace */

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
  अगर( nLoop<=1 ){
    mxChoice = 1;
  }अन्यथा अगर( nLoop==2 ){
    mxChoice = 5;
  }अन्यथा अगर( pParse->nErr ){
    mxChoice = 1;
  }अन्यथा{
    mxChoice = computeMxChoice(pWInfo);
  }
  निश्चित( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  अगर( pWInfo->pOrderBy==0 || nRowEst==0 ){
    nOrderBy = 0;
  }अन्यथा{
    nOrderBy = pWInfo->pOrderBy->n_व्यंजक;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (माप(WherePath)+माप(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += माप(LogEst) * nOrderBy;
  pSpace = sqlite3_ढेर_Alloc_Raw_NN(pParse->db, nSpace);
  अगर( pSpace==0 ) वापस SQLITE_NOMEM_BKPT;
  aTo = (WherePath*)pSpace;
  aFrom = aTo+mxChoice;
  स_रखो(aFrom, 0, माप(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  क्रम(ii=mxChoice*2, pFrom=aTo; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  अगर( nOrderBy ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    aSortCost = (LogEst*)pX;
    स_रखो(aSortCost, 0, माप(LogEst) * nOrderBy);
  }
  निश्चित( aSortCost==0 || &pSpace[nSpace]==(अक्षर*)&aSortCost[nOrderBy] );
  निश्चित( aSortCost!=0 || &pSpace[nSpace]==(अक्षर*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  निश्चित( 48==sqlite3LogEst(28) );
  nFrom = 1;
  निश्चित( aFrom[0].isOrdered==0 );
  अगर( nOrderBy ){
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
  क्रम(iLoop=0; iLoop<nLoop; iLoop++){
    nTo = 0;
    क्रम(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      क्रम(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        LogEst nOut;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask maskNew;                  /* Mask of src visited by (..) */
        Bitmask revMask;                  /* Mask of rev-order loops for (..) */

        अगर( (pWLoop->prereq & ~pFrom->maskLoop)!=0 ) जारी;
        अगर( (pWLoop->maskSelf & pFrom->maskLoop)!=0 ) जारी;
        अगर( (pWLoop->wsFlags & WHERE_AUTO_सूचकांक)!=0 && pFrom->nRow<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          निश्चित( 10==sqlite3LogEst(2) );
          जारी;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->rRun + pFrom->nRow;
        अगर( pWLoop->rSetup ){
          rUnsort = sqlite3LogEstAdd(pWLoop->rSetup, rUnsort);
        }
        rUnsort = sqlite3LogEstAdd(rUnsort, pFrom->rUnsort);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        isOrdered = pFrom->isOrdered;
        अगर( isOrdered<0 ){
          revMask = 0;
          isOrdered = wherePathSatisfiesOrderBy(pWInfo,
                       pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &revMask);
        }अन्यथा{
          revMask = pFrom->revLoop;
        }
        अगर( isOrdered>=0 && isOrdered<nOrderBy ){
          अगर( aSortCost[isOrdered]==0 ){
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
        }अन्यथा{
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
        क्रम(jj=0, pTo=aTo; jj<nTo; jj++, pTo++){
          अगर( pTo->maskLoop==maskNew
           && ( ((pTo->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==nTo-1 );
            अवरोध;
          }
        }
        अगर( jj>=nTo ){
          /* None of the existing best-so-far paths match the candidate. */
          अगर( nTo>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#अगर_घोषित WHERETRACE_ENABLED /* 0x4 */
            अगर( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#पूर्ण_अगर
            जारी;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          अगर( nTo<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = nTo++;
          }अन्यथा{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          pTo = &aTo[jj];
#अगर_घोषित WHERETRACE_ENABLED /* 0x4 */
          अगर( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#पूर्ण_अगर
        }अन्यथा{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          अगर( (pTo->rCost<rCost)
           || (pTo->rCost==rCost && pTo->nRow<nOut)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort<rUnsort)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, pTo->aLoop[iLoop]) )
          ){
#अगर_घोषित WHERETRACE_ENABLED /* 0x4 */
            अगर( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
                  pTo->rUnsort, pTo->isOrdered>=0 ? pTo->isOrdered+'0' : '?');
            }
#पूर्ण_अगर
            /* Discard the candidate path from further consideration */
            testcase( pTo->rCost==rCost );
            जारी;
          }
          testcase( pTo->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#अगर_घोषित WHERETRACE_ENABLED /* 0x4 */
          अगर( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
                pTo->rUnsort, pTo->isOrdered>=0 ? pTo->isOrdered+'0' : '?');
          }
#पूर्ण_अगर
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        pTo->maskLoop = pFrom->maskLoop | pWLoop->maskSelf;
        pTo->revLoop = revMask;
        pTo->nRow = nOut;
        pTo->rCost = rCost;
        pTo->rUnsort = rUnsort;
        pTo->isOrdered = isOrdered;
        स_नकल(pTo->aLoop, pFrom->aLoop, माप(WhereLoop*)*iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        अगर( nTo>=mxChoice ){
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsort = aTo[0].nRow;
          क्रम(jj=1, pTo=&aTo[1]; jj<mxChoice; jj++, pTo++){
            अगर( pTo->rCost>mxCost
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

#अगर_घोषित WHERETRACE_ENABLED  /* >=2 */
    अगर( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      पूर्णांक nDone = 0;
      पूर्णांक nProgress;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      करो{
        nProgress = 0;
        rMin = 0x7fff;
        क्रम(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          अगर( pTo->rCost>rFloor && pTo->rCost<rMin ) rMin = pTo->rCost;
        }
        क्रम(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          अगर( pTo->rCost==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
               pTo->isOrdered>=0 ? (pTo->isOrdered+'0') : '?');
            अगर( pTo->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", pTo->revLoop);
            }अन्यथा{
              sqlite3DebugPrintf("\n");
            }
            nDone++;
            nProgress++;
          }
        }
        rFloor = rMin;
      }जबतक( nDone<nTo && nProgress>0 );
    }
#पूर्ण_अगर

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  अगर( nFrom==0 ){
    sqlite3_Error_संदेश(pParse, "no query solution");
    sqlite3_ढेर_Free_NN(pParse->db, pSpace);
    वापस SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  निश्चित( nFrom==1 );
  pFrom = aFrom;

  निश्चित( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  क्रम(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  अगर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask notUsed;
    पूर्णांक rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->p_परिणाम_Set, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &notUsed);
    अगर( rc==pWInfo->p_परिणाम_Set->n_व्यंजक ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  अगर( pWInfo->pOrderBy ){
    pWInfo->nOBSat = pFrom->isOrdered;
    अगर( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      अगर( pFrom->isOrdered==pWInfo->pOrderBy->n_व्यंजक ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      निश्चित( pWInfo->pSelect->pOrderBy==0
           || pWInfo->nOBSat <= pWInfo->pSelect->pOrderBy->n_व्यंजक );
    }अन्यथा{
      pWInfo->revMask = pFrom->revLoop;
      अगर( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        अगर( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          अगर( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_COLUMN_IN))!=(WHERE_IPK|WHERE_COLUMN_IN)
          ){
            Bitmask m = 0;
            पूर्णांक rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom,
                      WHERE_ORDERBY_LIMIT, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_COLUMN_IN );
            अगर( rc==pWInfo->pOrderBy->n_व्यंजक ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->revMask = m;
            }
          }
        }
      }अन्यथा अगर( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    अगर( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->pOrderBy->n_व्यंजक && nLoop>0
    ){
      Bitmask revMask = 0;
      पूर्णांक nOrder = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &revMask
      );
      निश्चित( pWInfo->sorted==0 );
      अगर( nOrder==pWInfo->pOrderBy->n_व्यंजक ){
        pWInfo->sorted = 1;
        pWInfo->revMask = revMask;
      }
    }
  }

  pWInfo->nRowOut = pFrom->nRow;
#अगर_घोषित WHERETRACE_ENABLED
  pWInfo->r_योग_Cost = pFrom->rCost;
#पूर्ण_अगर

  /* Free temporary memory and return success */
  sqlite3_ढेर_Free_NN(pParse->db, pSpace);
  वापस SQLITE_OK;
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
अटल SQLITE_NOINLINE व्योम whereInterstageHeuristic(WhereInfo *pWInfo){
  पूर्णांक i;
#अगर_घोषित WHERETRACE_ENABLED
  पूर्णांक once = 0;
#पूर्ण_अगर
  क्रम(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    अगर( p==0 ) अवरोध;
    अगर( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      अवरोध;
    }
    अगर( (p->wsFlags & (WHERE_COLUMN_EQ|WHERE_COLUMN_NULL|WHERE_COLUMN_IN))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      क्रम(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_अग्र_Loop){
        अगर( pLoop->iTab!=iTab ) जारी;
        अगर( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_सूचकांक))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          जारी;
        }
#अगर_घोषित WHERETRACE_ENABLED
        अगर( sqlite3WhereTrace & 0x80 ){
          अगर( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#पूर्ण_अगर /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }अन्यथा{
      अवरोध;
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
अटल पूर्णांक whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  पूर्णांक iCur;
  पूर्णांक j;
  Table *pTab;
  सूचकांक *pIdx;
  WhereScan scan;

  pWInfo = pBuilder->pWInfo;
  अगर( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) वापस 0;
  निश्चित( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  अगर( IsVirtual(pTab) ) वापस 0;
  अगर( pItem->fg.isIndexedBy || pItem->fg.notIndexed ){
    testcase( pItem->fg.isIndexedBy );
    testcase( pItem->fg.notIndexed );
    वापस 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = whereScanInit(&scan, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  जबतक( pTerm && pTerm->prereq_दायाँ ) pTerm = where_Scan_अग्र(&scan);
  अगर( pTerm ){
    testcase( pTerm->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->rRun = 33;  /* 33==sqlite3LogEst(10) */
  }अन्यथा{
    क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
      पूर्णांक opMask;
      निश्चित( pLoop->aLTermSpace==pLoop->aLTerm );
      अगर( !Is_Unique_सूचकांक(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_कुंजी_Col>Array_आकार(pLoop->aLTermSpace)
      ) जारी;
      opMask = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      क्रम(j=0; j<pIdx->n_कुंजी_Col; j++){
        pTerm = whereScanInit(&scan, pWC, iCur, j, opMask, pIdx);
        जबतक( pTerm && pTerm->prereq_दायाँ ) pTerm = where_Scan_अग्र(&scan);
        अगर( pTerm==0 ) अवरोध;
        testcase( pTerm->eOperator & WO_IS );
        pLoop->aLTerm[j] = pTerm;
      }
      अगर( j!=pIdx->n_कुंजी_Col ) जारी;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_INDEXED;
      अगर( pIdx->isCovering || (pItem->colUsed & pIdx->colNotIdxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_सूचकांक = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->rRun = 39;  /* 39==sqlite3LogEst(15) */
      अवरोध;
    }
  }
  अगर( pLoop->wsFlags ){
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    निश्चित( pWInfo->sMaskSet.n==1 && iCur==pWInfo->sMaskSet.ix[0] );
    pLoop->maskSelf = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    अगर( pWInfo->pOrderBy ) pWInfo->nOBSat =  pWInfo->pOrderBy->n_व्यंजक;
    अगर( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    अगर( scan.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#अगर_घोषित SQLITE_DEBUG
    pLoop->cId = '0';
#पूर्ण_अगर
#अगर_घोषित WHERETRACE_ENABLED
    अगर( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#पूर्ण_अगर
    वापस 1;
  }
  वापस 0;
}

/*
** Helper function for exprIsDeterministic().
*/
अटल पूर्णांक व्यंजक_गाँठ_Is_Deterministic(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_FUNCTION && व्यंजक_Has_Property(p_व्यंजक, EP_ConstFunc)==0 ){
    pWalker->eCode = 0;
    वापस WRC_Abort;
  }
  वापस WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
अटल पूर्णांक व्यंजक_Is_Deterministic(व्यंजक *p){
  Walker w;
  स_रखो(&w, 0, माप(w));
  w.eCode = 1;
  w.x_व्यंजक_Callback = व्यंजक_गाँठ_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.eCode;
}

 
#अगर_घोषित WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
अटल व्योम showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  अगर( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    पूर्णांक i;
    अटल स्थिर अक्षर zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    क्रम(p=pWInfo->pLoops, i=0; p; p=p->p_अग्र_Loop, i++){
      p->cId = zLabel[i%(माप(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#अन्यथा
# define WHERETRACE_ALL_LOOPS(W,C)
#पूर्ण_अगर

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
अटल SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  पूर्णांक i;
  Bitmask tabUsed;
  पूर्णांक has_दायाँ_Join;

  /* Preconditions checked by the caller */
  निश्चित( pWInfo->nLevel>=2 );
  निश्चित( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  निश्चित( pWInfo->p_परिणाम_Set!=0 );
  निश्चित( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tabUsed = sqlite3_Where_व्यंजक_List_Usage(&pWInfo->sMaskSet, pWInfo->p_परिणाम_Set);
  अगर( pWInfo->pOrderBy ){
    tabUsed |= sqlite3_Where_व्यंजक_List_Usage(&pWInfo->sMaskSet, pWInfo->pOrderBy);
  }
  has_दायाँ_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  क्रम(i=pWInfo->nLevel-1; i>=1; i--){
    WhereTerm *pTerm, *pEnd;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    अगर( (pItem->fg.jointype & (JT_बायाँ|JT_दायाँ))!=JT_बायाँ ) जारी;
    अगर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      जारी;
    }
    अगर( (tabUsed & pLoop->maskSelf)!=0 ) जारी;
    pEnd = pWInfo->sWC.a + pWInfo->sWC.nTerm;
    क्रम(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      अगर( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        अगर( !व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_OuterON)
         || pTerm->p_व्यंजक->w.iJoin!=pItem->iCursor
        ){
          अवरोध;
        }
      }
      अगर( has_दायाँ_Join
       && व्यंजक_Has_Property(pTerm->p_व्यंजक, EP_InnerON)
       && NEVER(pTerm->p_व्यंजक->w.iJoin==pItem->iCursor)
      ){
        अवरोध;  /* restriction (5) */
      }
    }
    अगर( pTerm<pEnd ) जारी;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->revMask>>1) & ~m1)!=0 );
    pWInfo->revMask = (m1 & pWInfo->revMask) | ((pWInfo->revMask>>1) & ~m1);
    notReady &= ~pLoop->maskSelf;
    क्रम(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      अगर( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        pTerm->wtFlags |= TERM_CODED;
        pTerm->prereqAll = 0;
      }
    }
    अगर( i!=pWInfo->nLevel-1 ){
      पूर्णांक nByte = (pWInfo->nLevel-1-i) * माप(WhereLevel);
      स_हटाओ(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    निश्चित( pWInfo->nLevel>0 );
  }
  वापस notReady;
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
अटल SQLITE_NOINLINE व्योम whereCheckIfBloomFilterIsUseful(
  स्थिर WhereInfo *pWInfo
){
  पूर्णांक i;
  LogEst n_खोज = 0;

  निश्चित( pWInfo->nLevel>=2 );
  निश्चित( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  क्रम(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    स्थिर अचिन्हित पूर्णांक reqFlags = (WHERE_SELFCULL|WHERE_COLUMN_EQ);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    Table *pTab = pItem->pSTab;
    अगर( (pTab->tabFlags & TF_HasStat1)==0 ) अवरोध;
    pTab->tabFlags |= TF_MaybeReanalyze;
    अगर( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_खोज > pTab->nRowLogEst
    ){
      testcase( pItem->fg.jointype & JT_बायाँ );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (द्विगुन)sqlite3LogEstToInt(n_खोज), pTab->zName,
         (द्विगुन)sqlite3LogEstToInt(pTab->nRowLogEst)));
    }
    n_खोज += pLoop->nOut;
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
अटल SQLITE_NOINLINE व्योम where_Add_Indexed_व्यंजक(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  सूचकांक *pIdx,       /* The index-on-expression that contains the expressions */
  पूर्णांक iIdxCur,       /* Cursor number for pIdx */
  SrcItem *pTabItem  /* The FROM clause entry for the table */
){
  पूर्णांक i;
  Indexed_व्यंजक *p;
  Table *pTab;
  निश्चित( pIdx->b_Has_व्यंजक );
  pTab = pIdx->pTable;
  क्रम(i=0; i<pIdx->nColumn; i++){
    व्यंजक *p_व्यंजक;
    पूर्णांक j = pIdx->aiColumn[i];
    अगर( j==XN_व्यंजक ){
      p_व्यंजक = pIdx->a_Col_व्यंजक->a[i].p_व्यंजक;
    }अन्यथा अगर( j>=0 && (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)!=0 ){
      p_व्यंजक = sqlite3_Column_व्यंजक(pTab, &pTab->aCol[j]);
    }अन्यथा{
      जारी;
    }
    अगर( sqlite3_व्यंजक_Is_Constant(0,p_व्यंजक) ) जारी;
    p = sqlite3DbMallocRaw(pParse->db,  माप(Indexed_व्यंजक));
    अगर( p==0 ) अवरोध;
    p->pIENext = pParse->pIdxEpr;
#अगर_घोषित WHERETRACE_ENABLED
    अगर( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      अगर( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_व्यंजक(p_व्यंजक);
    }
#पूर्ण_अगर
    p->p_व्यंजक = sqlite3_व्यंजक_Dup(pParse->db, p_व्यंजक, 0);
    p->i_तथ्य_Cur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (JT_बायाँ|JT_LTORJ|JT_दायाँ))!=0;
    अगर( sqlite3_सूचकांक_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->zColAff[i];
    }
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#पूर्ण_अगर
    pParse->pIdxEpr = p;
    अगर( p->pIENext==0 ){
      व्योम *pArg = (व्योम*)&pParse->pIdxEpr;
      sqlite3ParserAddCleanup(pParse, where_Indexed_व्यंजक_Cleanup, pArg);
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
अटल SQLITE_NOINLINE व्योम whereReverseScanOrder(WhereInfo *pWInfo){
  पूर्णांक ii;
  क्रम(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    अगर( !pItem->fg.isCte
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
  व्यंजक *pWhere,           /* The WHERE clause */
  व्यंजक_List *pOrderBy,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  व्यंजक_List *p_परिणाम_Set,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  पूर्णांक iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  पूर्णांक nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  पूर्णांक nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  WhereMaskSet *pMaskSet;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  पूर्णांक ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  पूर्णांक rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  निश्चित( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  निश्चित( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_LIMIT)==0 );

  /* Variable initialization */
  db = pParse->db;
  स_रखो(&sWLB, 0, माप(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( pOrderBy && pOrderBy->n_व्यंजक==BMS-1 );
  अगर( pOrderBy && pOrderBy->n_व्यंजक>=BMS ){
    pOrderBy = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  अगर( pTabList->nSrc>BMS ){
    sqlite3_Error_संदेश(pParse, "at most %d tables in a join", BMS);
    वापस 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + माप(WhereLoop));
  अगर( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    जाओ whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;
#अगर WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#पूर्ण_अगर
  pWInfo->p_परिणाम_Set = p_परिणाम_Set;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->iLimit = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  स_रखो(&pWInfo->nOBSat, 0,
         दुरत्व(WhereInfo,sWC) - दुरत्व(WhereInfo,nOBSat));
  स_रखो(&pWInfo->a[0], 0, माप(WhereLoop)+nTabList*माप(WhereLevel));
  निश्चित( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  pMaskSet = &pWInfo->sMaskSet;
  pMaskSet->n = 0;
  pMaskSet->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((अक्षर*)pWInfo)+nByteWInfo);
  निश्चित( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#अगर_घोषित SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#पूर्ण_अगर

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3WhereSplit(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  अगर( nTabList==0 ){
    अगर( pOrderBy ) pWInfo->nOBSat = pOrderBy->n_व्यंजक;
    अगर( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    अगर( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_मूल्य)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }अन्यथा{
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
    करो{
      createMask(pMaskSet, pTabList->a[ii].iCursor);
      sqlite3WhereTabFuncArgs(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }जबतक( (++ii)<pTabList->nSrc );
  #अगर_घोषित SQLITE_DEBUG
    {
      Bitmask mx = 0;
      क्रम(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3WhereGetMask(pMaskSet, pTabList->a[ii].iCursor);
        निश्चित( m>=mx );
        mx = m;
      }
    }
  #पूर्ण_अगर
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_व्यंजक_Analyze(pTabList, &pWInfo->sWC);
  अगर( pSelect && pSelect->pLimit ){
    sqlite3WhereAddLimit(&pWInfo->sWC, pSelect);
  }
  अगर( pParse->nErr ) जाओ whereBeginError;

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
  क्रम(ii=0; ii<sWLB.pWC->nBase; ii++){
    WhereTerm *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    व्यंजक *pX;                          /* The expression of pT */
    अगर( pT->wtFlags & TERM_VIRTUAL ) जारी;
    pX = pT->p_व्यंजक;
    निश्चित( pX!=0 );
    निश्चित( pT->prereqAll!=0 || !व्यंजक_Has_Property(pX, EP_OuterON) );
    अगर( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || व्यंजक_Is_Deterministic(pX))   /* Condition (4) */
     && !(व्यंजक_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_व्यंजक_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= TERM_CODED;
    }
  }

  अगर( wctrlFlags & WHERE_WANT_DISTINCT ){
    अगर( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }अन्यथा अगर( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_परिणाम_Set) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }अन्यथा अगर( pOrderBy==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->pOrderBy = p_परिणाम_Set;
    }
  }

  /* Construct the WhereLoop objects */
#अगर defined(WHERETRACE_ENABLED)
  अगर( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    अगर( wctrlFlags & WHERE_USE_LIMIT ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    अगर( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      स_रखो(&sSelect, 0, माप(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.pOrderBy = pOrderBy;
      sSelect.pEList = p_परिणाम_Set;
      sqlite3_वृक्ष_View_Select(0, &sSelect, 0);
    }
    अगर( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#पूर्ण_अगर

  अगर( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = whereLoopAddAll(&sWLB);
    अगर( rc ) जाओ whereBeginError;

#अगर_घोषित SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    अगर( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      जबतक( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_अग्र_Loop;
        whereLoopDelete(db, p);
      }
      rc = whereLoopAddAll(&sWLB);
      अगर( rc ) जाओ whereBeginError;
    }
#पूर्ण_अगर
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    wherePathSolver(pWInfo, 0);
    अगर( db->mallocFailed ) जाओ whereBeginError;
    अगर( pWInfo->pOrderBy ){
       whereInterstageHeuristic(pWInfo);
       wherePathSolver(pWInfo, pWInfo->nRowOut<0 ? 1 : pWInfo->nRowOut+1);
       अगर( db->mallocFailed ) जाओ whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    अगर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->nRowOut, pWInfo->nRowOut-30));
      pWInfo->nRowOut -= 30;
    }

  }
  निश्चित( pWInfo->pTabList!=0 );
  अगर( pWInfo->pOrderBy==0 && (db->flags & SQLITE_ReverseOrder)!=0 ){
    whereReverseScanOrder(pWInfo);
  }
  अगर( pParse->nErr ){
    जाओ whereBeginError;
  }
  निश्चित( db->mallocFailed==0 );
#अगर_घोषित WHERETRACE_ENABLED
  अगर( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_योग_Cost, pWInfo->nRowOut);
    अगर( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->revMask);
    }
    चयन( pWInfo->eDistinct ){
      हाल WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        अवरोध;
      }
      हाल WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        अवरोध;
      }
      हाल WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        अवरोध;
      }
    }
    sqlite3DebugPrintf("\n");
    क्रम(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#पूर्ण_अगर

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
  अगर( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && p_परिणाम_Set!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    निश्चित( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  अगर( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#अगर defined(WHERETRACE_ENABLED)
  अगर( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
    sqlite3DebugPrintf("---- WHERE clause at end of analysis:\n");
    sqlite3WhereClausePrint(sWLB.pWC);
  }
  WHERETRACE(0xffffffff,("*** Optimizer Finished ***\n"));
#पूर्ण_अगर
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
  निश्चित( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  अगर( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    पूर्णांक wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    पूर्णांक bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    निश्चित( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    अगर( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      अगर( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        अगर( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  क्रम(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    Table *pTab;     /* Table to open */
    पूर्णांक iDb;         /* Index of database containing table/index */
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    अगर( ii==0 || (pTabItem[0].fg.jointype & JT_बायाँ)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }अन्यथा अगर( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }अन्यथा{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    अगर( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }अन्यथा
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    अगर( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      स्थिर अक्षर *pVTab = (स्थिर अक्षर *)sqlite3GetVTable(db, pTab);
      पूर्णांक iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }अन्यथा अगर( IsVirtual(pTab) ){
      /* noop */
    }अन्यथा
#पूर्ण_अगर
    अगर( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (pTabItem->fg.jointype & (JT_LTORJ|JT_दायाँ))!=0
    ){
      पूर्णांक op = OP_OpenRead;
      अगर( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);
      निश्चित( pTabItem->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS );
      अगर( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->nCol<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_सूचकांक|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = pTabItem->colUsed;
        पूर्णांक n = 0;
        क्रम(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_TO_PTR(n), P4_INT32);
        निश्चित( n<=pTab->nCol );
      }
#अगर_घोषित SQLITE_ENABLE_CURSOR_HINTS
      अगर( pLoop->u.btree.p_सूचकांक!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }अन्यथा
#पूर्ण_अगर
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#अगर_घोषित SQLITE_ENABLE_COLUMN_USED_MASK
      sqlite3VdbeAddOp3(v, OP_ColumnsUsed, pTabItem->iCursor,
                    LOWER32(pTabItem->colUsed), UPPER32(pTabItem->colUsed));
#पूर्ण_अगर
      अगर( ii>=2
       && (pTabItem[0].fg.jointype & (JT_LTORJ|JT_बायाँ))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3VdbeAddOp2(v, OP_IfEmpty, pTabItem->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }अन्यथा{
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    अगर( pLoop->wsFlags & WHERE_INDEXED ){
      सूचकांक *pIx = pLoop->u.btree.p_सूचकांक;
      पूर्णांक i_सूचकांक_Cur;
      पूर्णांक op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      निश्चित( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      अगर( !HasRowid(pTab) && Is_Primary_कुंजी_सूचकांक(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_सूचकांक_Cur = pLevel->iTabCur;
        op = 0;
      }अन्यथा अगर( pWInfo->eOnePass!=ONEPASS_OFF ){
        सूचकांक *pJ = pTabItem->pSTab->p_सूचकांक;
        i_सूचकांक_Cur = iAuxArg;
        निश्चित( wctrlFlags & WHERE_ONEPASS_DESIRED );
        जबतक( ALWAYS(pJ) && pJ!=pIx ){
          i_सूचकांक_Cur++;
          pJ = pJ->p_अग्र;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_सूचकांक_Cur;
      }अन्यथा अगर( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_सूचकांक_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }अन्यथा{
        i_सूचकांक_Cur = pParse->nTab++;
        अगर( pIx->b_Has_व्यंजक && OptimizationEnabled(db, SQLITE_Indexed_व्यंजक) ){
          where_Add_Indexed_व्यंजक(pParse, pIx, i_सूचकांक_Cur, pTabItem);
        }
        अगर( pIx->pPartIdxWhere && (pTabItem->fg.jointype & JT_दायाँ)==0 ){
          where_Part_Idx_व्यंजक(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_सूचकांक_Cur, pTabItem
          );
        }
      }
      pLevel->iIdxCur = i_सूचकांक_Cur;
      निश्चित( pIx!=0 );
      निश्चित( pIx->pSchema==pTab->pSchema );
      निश्चित( i_सूचकांक_Cur>=0 );
      अगर( op ){
        sqlite3VdbeAddOp3(v, op, i_सूचकांक_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIx);
        अगर( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_COLUMN_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_SORT)==0
         && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#अगर_घोषित SQLITE_ENABLE_COLUMN_USED_MASK
        {
          u64 colUsed = 0;
          पूर्णांक ii, jj;
          क्रम(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            अगर( jj<0 ) जारी;
            अगर( jj>63 ) jj = 63;
            अगर( (pTabItem->colUsed & MASKBIT(jj))==0 ) जारी;
            colUsed |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3VdbeAddOp3(v, OP_ColumnsUsed, i_सूचकांक_Cur,
                            LOWER32(colUsed),UPPER32(colUsed));
        }
#पूर्ण_अगर /* SQLITE_ENABLE_COLUMN_USED_MASK */
#अगर_घोषित SQLITE_ENABLE_CURSOR_HINTS
        अगर( HasRowid(pTab) ){
          sqlite3VdbeAddOp3(v, OP_CursorHint, i_सूचकांक_Cur, 0, pTabItem->iCursor);
        }
#पूर्ण_अगर
      }
    }
    अगर( iDb>=0 ) sqlite3CodeVerifySchema(pParse, iDb);
    अगर( (pTabItem->fg.jointype & JT_दायाँ)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, माप(Where_दायाँ_Join)))!=0
    ){
      Where_दायाँ_Join *pRJ = pLevel->pRJ;
      पूर्णांक bBloomable = 0;
      pRJ->iMatch = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, pRJ->regReturn);
      निश्चित( pTab==pTabItem->pSTab );
      अगर( HasRowid(pTab) ){
        कुंजी_Info *pInfo;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, 1);
        bBloomable = 1;
        pInfo = sqlite3_कुंजी_Info_Alloc(pParse->db, 1, 0);
        अगर( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->aSortFlags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }अन्यथा{
        सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, pPk->n_कुंजी_Col);
        sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pPk);
        bBloomable = sqlite3_सूचकांक_Bloomable(pPk,pPk->n_कुंजी_Col);
      }
      अगर( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Blob, 65536, pRJ->regBloom);
      }अन्यथा{
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
  pWInfo->i_शीर्ष_2 = sqlite3VdbeCurrentAddr(v);
  अगर( db->mallocFailed ) जाओ whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  क्रम(ii=0; ii<nTabList; ii++){
    पूर्णांक addrExplain;
    पूर्णांक wsFlags;
    SrcItem *pSrc;
    अगर( pParse->nErr ) जाओ whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    अगर( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      पूर्णांक iOnce = 0;
      निश्चित( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      अगर( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }अन्यथा{
        iOnce = 0;
      }
      sqlite3VdbeAddOp2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      अगर( iOnce )  sqlite3VdbeJumpHere(v, iOnce);
    }
    निश्चित( pTabList == pWInfo->pTabList );
    अगर( (wsFlags & (WHERE_AUTO_सूचकांक|WHERE_BLOOMFILTER))!=0 ){
      अगर( (wsFlags & WHERE_AUTO_सूचकांक)!=0 ){
#अगर_अघोषित SQLITE_OMIT_AUTOMATIC_सूचकांक
        construct_Automatic_सूचकांक(pParse, &pWInfo->sWC, notReady, pLevel);
#पूर्ण_अगर
      }अन्यथा{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      अगर( db->mallocFailed ) जाओ whereBeginError;
    }
    addrExplain = sqlite3WhereExplainOneScan(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    अगर( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3WhereAddScanStatus(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->iEndWhere = sqlite3VdbeCurrentAddr(v);
  वापस pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  अगर( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#अगर_घोषित WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#पूर्ण_अगर /* WHERETRACE_ENABLED */
  वापस 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#अगर_अघोषित SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#अन्यथा
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  अटल व्योम sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    पूर्णांक pc,
    VdbeOp *pOp
  ){
    अगर( (db->flags & SQLITE_VdbeAddopTrace)==0 ) वापस;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3ShowWhereTerm(0); /* So compiler won't complain about unused func */
  }
#पूर्ण_अगर

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
व्योम sqlite3WhereEnd(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  पूर्णांक i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  पूर्णांक iEnd = sqlite3VdbeCurrentAddr(v);
  पूर्णांक nRJ = 0;
#अगर_अघोषित SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  पूर्णांक addrSeek = 0;
#पूर्ण_अगर

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  क्रम(i=pWInfo->nLevel-1; i>=0; i--){
    पूर्णांक addr;
    pLevel = &pWInfo->a[i];
    अगर( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_दायाँ_Join *pRJ = pLevel->pRJ;
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
    अगर( pLevel->op!=OP_Noop ){
#अगर_अघोषित SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      सूचकांक *pIdx;
      पूर्णांक n;
      अगर( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_INDEXED)!=0
       && (pIdx = pLoop->u.btree.p_सूचकांक)->hasStat1
       && (n = pLoop->u.btree.nDistinctCol)>0
       && pIdx->aiRowLogEst[n]>=36
      ){
        पूर्णांक r1 = pParse->nMem+1;
        पूर्णांक j, op;
        पूर्णांक addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        अगर( pLevel->i_बायाँ_Join ){
          addrIfNull = sqlite3VdbeAddOp2(v, OP_IfNullRow, pLevel->iIdxCur, r1);
        }
        क्रम(j=0; j<n; j++){
          sqlite3VdbeAddOp3(v, OP_Column, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_पूर्व ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3VdbeAddOp4Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3VdbeAddOp2(v, OP_Goto, 1, pLevel->p2);
        अगर( pLevel->i_बायाँ_Join ){
          sqlite3VdbeJumpHere(v, addrIfNull);
        }
      }
#पूर्ण_अगर /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    अगर( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }अन्यथा अगर( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      पूर्णांक iBypass = sqlite3VdbeAddOp0(v, OP_Goto);
      निश्चित( pLoop->wsFlags & WHERE_AUTO_सूचकांक );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3VdbeAddOp1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3VdbeJumpHere(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    अगर( pLevel->op!=OP_Noop ){
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_अग्र);
      VdbeCoverageIf(v, pLevel->op==OP_पूर्व);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#अगर_अघोषित SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      अगर( addrSeek ){
        sqlite3VdbeJumpHere(v, addrSeek);
        addrSeek = 0;
      }
#पूर्ण_अगर
      अगर( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3VdbeAddOp2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    अगर( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#अगर_अघोषित SQLITE_LIKE_DOESNT_MATCH_BLOBS
    निश्चित( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_IN_ABLE) && pLevel->u.in.nIn>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    अगर( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3VdbeAddOp2(v, OP_DecrJumpZero, (पूर्णांक)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#पूर्ण_अगर
    अगर( (pLoop->wsFlags & WHERE_IN_ABLE)!=0 && pLevel->u.in.nIn>0 ){
      काष्ठा InLoop *pIn;
      पूर्णांक j;
      क्रम(j=pLevel->u.in.nIn, pIn=&pLevel->u.in.aInLoop[j-1]; j>0; j--, pIn--){
        निश्चित( sqlite3VdbeGetOp(v, pIn->addr_In_शीर्ष+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3VdbeJumpHere(v, pIn->addr_In_शीर्ष+1);
        अगर( pIn->eEndLoopOp!=OP_Noop ){
          अगर( pIn->nPrefix ){
            पूर्णांक bEarlyOut =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0;
            अगर( pLevel->i_बायाँ_Join ){
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
            अगर( bEarlyOut ){
              sqlite3VdbeAddOp4Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  pIn->iBase, pIn->nPrefix);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3VdbeJumpHere(v, pIn->addr_In_शीर्ष+1);
            }
          }
          sqlite3VdbeAddOp2(v, pIn->eEndLoopOp, pIn->iCur, pIn->addr_In_शीर्ष);
          VdbeCoverage(v);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_पूर्व);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_अग्र);
        }
        sqlite3VdbeJumpHere(v, pIn->addr_In_शीर्ष-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    अगर( pLevel->pRJ ){
      sqlite3VdbeAddOp3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    अगर( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_सूचकांक->zName));
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip-2);
    }
    अगर( pLevel->i_बायाँ_Join ){
      पूर्णांक ws = pLoop->wsFlags;
      addr = sqlite3VdbeAddOp1(v, OP_IfPos, pLevel->i_बायाँ_Join); VdbeCoverage(v);
      निश्चित( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_INDEXED)!=0 );
      अगर( (ws & WHERE_IDX_ONLY)==0 ){
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];
        निश्चित( pLevel->iTabCur==pSrc->iCursor );
        अगर( pSrc->fg.viaCoroutine ){
          पूर्णांक m, n;
          निश्चित( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_परिणाम;
          निश्चित( pSrc->pSTab!=0 );
          m = pSrc->pSTab->nCol;
          sqlite3VdbeAddOp3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iTabCur);
      }
      अगर( (ws & WHERE_INDEXED)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        अगर( ws & WHERE_MULTI_OR ){
          सूचकांक *pIx = pLevel->u.pCoveringIdx;
          पूर्णांक iDb = sqlite3_Schema_To_सूचकांक(db, pIx->pSchema);
          sqlite3VdbeAddOp3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIx);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
      }
      अगर( ws & WHERE_IN_ABLE ){
        पूर्णांक m;
        क्रम(m=0; m<pLevel->u.in.nIn; m++){
          काष्ठा InLoop *pIn = &pLevel->u.in.aInLoop[m];
          अगर( pIn->eEndLoopOp!=OP_Noop ){
            sqlite3VdbeAddOp1(v, OP_NullRow, pIn->iCur);
          }
        }
      }
      अगर( pLevel->op==OP_Return ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }अन्यथा{
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  निश्चित( pWInfo->nLevel<=pTabList->nSrc );
  क्रम(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    पूर्णांक k, last;
    VdbeOp *pOp, *pLastOp;
    सूचकांक *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pSTab;
    निश्चित( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    अगर( pLevel->pRJ ){
      sqlite3_Where_दायाँ_Join_Loop(pWInfo, i, pLevel);
      जारी;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    अगर( pTabItem->fg.viaCoroutine ){
      testcase( pParse->db->mallocFailed );
      निश्चित( pTabItem->fg.isSubquery );
      निश्चित( pTabItem->u4.pSubq->reg_परिणाम>=0 );
      translateColumnToCopy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->u4.pSubq->reg_परिणाम, 0);
      जारी;
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
    अगर( pLoop->wsFlags & (WHERE_INDEXED|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_सूचकांक;
    }अन्यथा अगर( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    अगर( pIdx
     && !db->mallocFailed
    ){
      अगर( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->pTable) ){
        last = iEnd;
      }अन्यथा{
        last = pWInfo->iEndWhere;
      }
      अगर( pIdx->b_Has_व्यंजक ){
        Indexed_व्यंजक *p = pParse->pIdxEpr;
        जबतक( p ){
          अगर( p->iIdxCur==pLevel->iIdxCur ){
#अगर_घोषित WHERETRACE_ENABLED
            अगर( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->iIdxCol);
              अगर( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_व्यंजक(p->p_व्यंजक);
            }
#पूर्ण_अगर
            p->i_तथ्य_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#अगर_घोषित SQLITE_DEBUG
      अगर( db->flags & SQLITE_VdbeAddopTrace ){
        म_लिखो("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, last-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      निश्चित( pOp->opcode!=OP_Column || pOp->p1!=pLevel->iTabCur );
      निश्चित( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      निश्चित( pOp->opcode!=OP_IfNullRow || pOp->p1!=pLevel->iTabCur );
#पूर्ण_अगर
      pOp = sqlite3VdbeGetOp(v, k);
      pLastOp = pOp + (last - k);
      निश्चित( pOp<=pLastOp );
      करो{
        अगर( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }अन्यथा अगर( pOp->opcode==OP_Column
#अगर_घोषित SQLITE_ENABLE_OFFSET_SQL_FUNC
         || pOp->opcode==OP_Offset
#पूर्ण_अगर
        ){
          पूर्णांक x = pOp->p2;
          निश्चित( pIdx->pTable==pTab );
#अगर_घोषित SQLITE_ENABLE_OFFSET_SQL_FUNC
          अगर( pOp->opcode==OP_Offset ){
            /* Do not need to translate the column number */
          }अन्यथा
#पूर्ण_अगर
          अगर( !HasRowid(pTab) ){
            सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
            x = pPk->aiColumn[x];
            निश्चित( x>=0 );
          }अन्यथा{
            testcase( x!=sqlite3StorageColumnToTable(pTab,x) );
            x = sqlite3StorageColumnToTable(pTab,x);
          }
          x = sqlite3_Table_Column_To_सूचकांक(pIdx, x);
          अगर( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }अन्यथा अगर( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            अगर( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_संदेश(pParse, "internal query planner error");
              pParse->rc = SQLITE_INTERNAL;
            }अन्यथा{
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
        }अन्यथा अगर( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }अन्यथा अगर( pOp->opcode==OP_IfNullRow ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#अगर_घोषित SQLITE_DEBUG
        k++;
#पूर्ण_अगर
      }जबतक( (++pOp)<pLastOp );
#अगर_घोषित SQLITE_DEBUG
      अगर( db->flags & SQLITE_VdbeAddopTrace ) म_लिखो("TRANSLATE complete\n");
#पूर्ण_अगर
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
  वापस;
}
