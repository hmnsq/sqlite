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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
प्रकारव्याख्या संरचना HiddenIndexInfo HiddenIndexInfo;
संरचना HiddenIndexInfo {
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
#define SZ_HIDDENINDEXINFO(N) \
                  (दुरत्व(HiddenIndexInfo,aRhs) + (N)*आकार(sqlite3_मूल्य*))

/* Forward declaration of methods */
स्थिर पूर्णांक whereLoopResize(sqlite3*, WhereLoop*, पूर्णांक);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_Row_मोजणी(WhereInfo *pWInfo){
  परत pWInfo->nRowOut;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
पूर्णांक sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  परत pWInfo->eDistinct;
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
  परत pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
  जर( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    परत pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  प्रतिज्ञा( pInner->addrNxt!=0 );
  परत pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
शून्य sqlite3WhereMinMaxOptEarlyOut(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  पूर्णांक i;
  जर( !pWInfo->bOrderedInnerLoop ) परत;
  जर( pWInfo->nOBSat==0 ) परत;
  साठी(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    जर( (pInner->pWLoop->wsFlags & WHERE_COLUMN_IN)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      परत;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
पूर्णांक sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  प्रतिज्ञा( pWInfo->iContinue!=0 );
  परत pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
पूर्णांक sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  परत pWInfo->iBreak;
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
  स_नकल(aiCur, pWInfo->aiCurOnePass, आकार(पूर्णांक)*2);
#ifdef WHERETRACE_ENABLED
  जर( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#endif
  परत pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
पूर्णांक sqlite3WhereUsesDeferredSeek(WhereInfo *pWInfo){
  परत pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
स्थिर शून्य whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc){
  pDest->n = pSrc->n;
  स_नकल(pDest->a, pSrc->a, pDest->n*आकार(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
स्थिर पूर्णांक where_Or_घाला(
  WhereOrSet *pSet,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst rRun,           /* Run-cost of the new entry */
  LogEst nOut            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  साठी(i=pSet->n, p=pSet->a; i>0; i--, p++){
    जर( rRun<=p->rRun && (prereq & p->prereq)==prereq ){
      जा where_Or_घाला_done;
    }
    जर( p->rRun<=rRun && (p->prereq & prereq)==p->prereq ){
      परत 0;
    }
  }
  जर( pSet->n<N_OR_COST ){
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  }अन्यथा{
    p = pSet->a;
    साठी(i=1; i<pSet->n; i++){
      जर( p->rRun>pSet->a[i].rRun ) p = pSet->a + i;
    }
    जर( p->rRun<=rRun ) परत 0;
  }
where_Or_घाला_done:
  p->prereq = prereq;
  p->rRun = rRun;
  जर( p->nOut>nOut ) p->nOut = nOut;
  परत 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, पूर्णांक iCursor){
  पूर्णांक i;
  प्रतिज्ञा( pMaskSet->n<=(पूर्णांक)आकार(Bitmask)*8 );
  प्रतिज्ञा( pMaskSet->n>0 || pMaskSet->ix[0]<0 );
  प्रतिज्ञा( iCursor>=-1 );
  जर( pMaskSet->ix[0]==iCursor ){
    परत 1;
  }
  साठी(i=1; i<pMaskSet->n; i++){
    जर( pMaskSet->ix[i]==iCursor ){
      परत MASKBIT(i);
    }
  }
  परत 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
शून्य *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  WhereMemBlock *pBlock;
  pBlock = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+आकार(*pBlock));
  जर( pBlock ){
    pBlock->pNext = pWInfo->pMemToFree;
    pBlock->sz = nByte;
    pWInfo->pMemToFree = pBlock;
    pBlock++;
  }
  परत (शून्य*)pBlock;
}
शून्य *sqlite3WhereRealloc(WhereInfo *pWInfo, शून्य *pOld, u64 nByte){
  शून्य *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  जर( pNew && pOld ){
    WhereMemBlock *pOldBlk = (WhereMemBlock*)pOld;
    pOldBlk--;
    प्रतिज्ञा( pOldBlk->sz<nByte );
    स_नकल(pNew, pOld, pOldBlk->sz);
  }
  परत pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
स्थिर शून्य createMask(WhereMaskSet *pMaskSet, पूर्णांक iCursor){
  प्रतिज्ञा( pMaskSet->n < ArraySize(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
स्थिर अभिव्यक्ती *where_उजवा_Subexpr_Is_Column(अभिव्यक्ती *p){
  p = sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(p->p_उजवा);
  जर( ALWAYS(p!=0) && p->op==TK_COLUMN && !अभिव्यक्ती_Has_Property(p, EP_FixedCol) ){
    परत p;
  }
  परत 0;
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
स्थिर SQLITE_NOINLINE स्थिरांक अक्षर *indexInAffinityOk(
  Parse *pParse, 
  WhereTerm *pTerm, 
  u8 idxaff
){
  अभिव्यक्ती *pX = pTerm->p_अभिव्यक्ती;
  अभिव्यक्ती inexpr;

  प्रतिज्ञा( pTerm->eOperator & WO_IN );

  जर( अभिव्यक्ती_Use_XSelect(pX) ){
    पूर्णांक iField = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    जर( sqlite3_अभिव्यक्ती_Is_Vector(pX->p_डावा) ){
      iField = pTerm->u.x.iField - 1;
      inexpr.p_डावा = pX->p_डावा->x.pList->a[iField].p_अभिव्यक्ती;
    }अन्यथा{
      inexpr.p_डावा = pX->p_डावा;
    }
    inexpr.p_उजवा = pX->x.pSelect->pEList->a[iField].p_अभिव्यक्ती;
    pX = &inexpr;
  }

  जर( sqlite3IndexAffinityOk(pX, idxaff) ){
    CollSeq *pRet = sqlite3_अभिव्यक्ती_Compare_Coll_Seq(pParse, pX);
    परत pRet ? pRet->zName : sqlite3_Str_द्विमान;
  }
  परत 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
स्थिर WhereTerm *whereScanNext(WhereScan *pScan){
  पूर्णांक iCur;            /* The cursor on the LHS of the term */
  i16 iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  अभिव्यक्ती *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  WhereTerm *pTerm;    /* The term being tested */
  पूर्णांक k = pScan->k;    /* Where to start scanning */

  प्रतिज्ञा( pScan->iEquiv<=pScan->nEquiv );
  pWC = pScan->pWC;
  पर्यंत(1){
    iColumn = pScan->aiColumn[pScan->iEquiv-1];
    iCur = pScan->aiCur[pScan->iEquiv-1];
    प्रतिज्ञा( pWC!=0 );
    प्रतिज्ञा( iCur>=0 );
    करा{
      साठी(pTerm=pWC->a+k; k<pWC->nTerm; k++, pTerm++){
        प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 || pTerm->डावा_Cursor<0 );
        जर( pTerm->डावा_Cursor==iCur
         && pTerm->u.x.डावा_Column==iColumn
         && (iColumn!=XN_अभिव्यक्ती
             || sqlite3_अभिव्यक्ती_Compare_Skip(pTerm->p_अभिव्यक्ती->p_डावा,
                                       pScan->p_Idx_अभिव्यक्ती,iCur)==0)
         && (pScan->iEquiv<=1 || !अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_OuterON))
        ){
          जर( (pTerm->eOperator & WO_EQUIV)!=0
           && pScan->nEquiv<ArraySize(pScan->aiCur)
           && (pX = where_उजवा_Subexpr_Is_Column(pTerm->p_अभिव्यक्ती))!=0
          ){
            पूर्णांक j;
            साठी(j=0; j<pScan->nEquiv; j++){
              जर( pScan->aiCur[j]==pX->iTable
               && pScan->aiColumn[j]==pX->iColumn ){
                  थांबा;
              }
            }
            जर( j==pScan->nEquiv ){
              pScan->aiCur[j] = pX->iTable;
              pScan->aiColumn[j] = pX->iColumn;
              pScan->nEquiv++;
            }
          }
          जर( (pTerm->eOperator & pScan->opMask)!=0 ){
            /* Verify the affinity and collating sequence match */
            जर( pScan->zCollName && (pTerm->eOperator & WO_ISNULL)==0 ){
              स्थिरांक अक्षर *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->p_अभिव्यक्ती;

              जर( (pTerm->eOperator & WO_IN) ){
                zCollName = indexInAffinityOk(pParse, pTerm, pScan->idxaff);
                जर( !zCollName ) सुरूठेवा;
              }अन्यथा{
                CollSeq *pColl;
                जर( !sqlite3IndexAffinityOk(pX, pScan->idxaff) ){
                  सुरूठेवा;
                }
                प्रतिज्ञा(pX->p_डावा);
                pColl = sqlite3_अभिव्यक्ती_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_द्विमान;
              }

              जर( sqlite3StrICmp(zCollName, pScan->zCollName) ){
                सुरूठेवा;
              }
            }
            जर( (pTerm->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = pTerm->p_अभिव्यक्ती->p_उजवा, ALWAYS(pX!=0))
             && pX->op==TK_COLUMN
             && pX->iTable==pScan->aiCur[0]
             && pX->iColumn==pScan->aiColumn[0]
            ){
              testcase( pTerm->eOperator & WO_IS );
              सुरूठेवा;
            }
            pScan->pWC = pWC;
            pScan->k = k+1;
#ifdef WHERETRACE_ENABLED
            जर( (sqlite3WhereTrace & 0x20000)!=0 && pScan->nEquiv>1 ){
              पूर्णांक ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 pScan->aiCur[0], pScan->aiColumn[0], pTerm->iTerm);
              साठी(ii=1; ii<pScan->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   pScan->aiCur[ii], pScan->aiColumn[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#endif
            परत pTerm;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }पर्यंत( pWC!=0 );
    जर( pScan->iEquiv>=pScan->nEquiv ) थांबा;
    pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv++;
  }
  परत 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
स्थिर SQLITE_NOINLINE WhereTerm *where_Scan_Init_Index_अभिव्यक्ती(WhereScan *pScan){
  pScan->idxaff = sqlite3_अभिव्यक्ती_Affinity(pScan->p_Idx_अभिव्यक्ती);
  परत whereScanNext(pScan);
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
स्थिर WhereTerm *whereScanInit(
  WhereScan *pScan,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  पूर्णांक iCur,               /* Cursor to scan for */
  पूर्णांक iColumn,            /* Column to scan for */
  u32 opMask,             /* Operator(s) to scan for */
  Index *pIdx             /* Must be compatible with this index */
){
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  pScan->p_Idx_अभिव्यक्ती = 0;
  pScan->idxaff = 0;
  pScan->zCollName = 0;
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aiCur[0] = iCur;
  pScan->nEquiv = 1;
  pScan->iEquiv = 1;
  जर( pIdx ){
    पूर्णांक j = iColumn;
    iColumn = pIdx->aiColumn[j];
    जर( iColumn==pIdx->pTable->iPKey ){
      iColumn = XN_ROWID;
    }अन्यथा जर( iColumn>=0 ){
      pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
      pScan->zCollName = pIdx->azColl[j];
    }अन्यथा जर( iColumn==XN_अभिव्यक्ती ){
      pScan->p_Idx_अभिव्यक्ती = pIdx->a_Col_अभिव्यक्ती->a[j].p_अभिव्यक्ती;
      pScan->zCollName = pIdx->azColl[j];
      pScan->aiColumn[0] = XN_अभिव्यक्ती;
      परत where_Scan_Init_Index_अभिव्यक्ती(pScan);
    }
  }अन्यथा जर( iColumn==XN_अभिव्यक्ती ){
    परत 0;
  }
  pScan->aiColumn[0] = iColumn;
  परत whereScanNext(pScan);
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
  Index *pIdx           /* Must be compatible with this index, if not NULL */
){
  WhereTerm *pResult = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  पर्यंत( p ){
    जर( (p->prereq_उजवा & notReady)==0 ){
      जर( p->prereq_उजवा==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        परत p;
      }
      जर( pResult==0 ) pResult = p;
    }
    p = whereScanNext(&scan);
  }
  परत pResult;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
स्थिर पूर्णांक findIndexCol(
  Parse *pParse,                  /* Parse context */
  अभिव्यक्ती_List *pList,                /* Expression list to search */
  पूर्णांक iBase,                      /* Cursor for table associated with pIdx */
  Index *pIdx,                    /* Index to match column of */
  पूर्णांक iCol                        /* Column of index to match */
){
  पूर्णांक i;
  स्थिरांक अक्षर *zColl = pIdx->azColl[iCol];

  साठी(i=0; i<pList->n_अभिव्यक्ती; i++){
    अभिव्यक्ती *p = sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(pList->a[i].p_अभिव्यक्ती);
    जर( ALWAYS(p!=0)
     && (p->op==TK_COLUMN || p->op==TK_AGG_COLUMN)
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->iTable==iBase
    ){
      CollSeq *pColl = sqlite3_अभिव्यक्ती_NNColl_Seq(pParse, pList->a[i].p_अभिव्यक्ती);
      जर( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        परत i;
      }
    }
  }

  परत -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
स्थिर पूर्णांक indexColumnNotNull(Index *pIdx, पूर्णांक iCol){
  पूर्णांक j;
  प्रतिज्ञा( pIdx!=0 );
  प्रतिज्ञा( iCol>=0 && iCol<pIdx->nColumn );
  j = pIdx->aiColumn[iCol];
  जर( j>=0 ){
    परत pIdx->pTable->aCol[j].notNull;
  }अन्यथा जर( j==(-1) ){
    परत 1;
  }अन्यथा{
    प्रतिज्ञा( j==(-2) );
    परत 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
स्थिर पूर्णांक isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  अभिव्यक्ती_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  Table *pTab;
  Index *pIdx;
  पूर्णांक i;                         
  पूर्णांक iBase;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  जर( pTabList->nSrc!=1 ) परत 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  साठी(i=0; i<pDistinct->n_अभिव्यक्ती; i++){
    अभिव्यक्ती *p = sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(pDistinct->a[i].p_अभिव्यक्ती);
    जर( NEVER(p==0) ) सुरूठेवा;
    जर( p->op!=TK_COLUMN && p->op!=TK_AGG_COLUMN ) सुरूठेवा;
    जर( p->iTable==iBase && p->iColumn<0 ) परत 1;
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
  साठी(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    जर( !IsUniqueIndex(pIdx) ) सुरूठेवा;
    जर( pIdx->pPartIdxWhere ) सुरूठेवा;
    साठी(i=0; i<pIdx->n_किल्ली_Col; i++){
      जर( 0==sqlite3WhereFindTerm(pWC, iBase, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        जर( findIndexCol(pParse, pDistinct, iBase, pIdx, i)<0 ) थांबा;
        जर( indexColumnNotNull(pIdx, i)==0 ) थांबा;
      }
    }
    जर( i==pIdx->n_किल्ली_Col ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      परत 1;
    }
  }

  परत 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
स्थिर LogEst estLog(LogEst N){
  परत N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
स्थिर शून्य translateColumnToCopy(
  Parse *pParse,      /* Parsing context */
  पूर्णांक iStart,         /* Translate from this opcode to the end */
  पूर्णांक iTabCur,        /* OP_Column/OP_Rowid references to this table */
  पूर्णांक iRegister,      /* The first column is in this register */
  पूर्णांक iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  पूर्णांक iEnd = sqlite3VdbeCurrentAddr(v);
  जर( pParse->db->mallocFailed ) परत;
#ifdef SQLITE_DEBUG
  जर( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    म_लिखो("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, iStart, iEnd);
  }
#endif
  साठी(; iStart<iEnd; iStart++, pOp++){
    जर( pOp->p1!=iTabCur ) सुरूठेवा;
    जर( pOp->opcode==OP_Column ){
#ifdef SQLITE_DEBUG
      जर( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        म_लिखो("TRANSLATE OP_Column to OP_Copy at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }अन्यथा जर( pOp->opcode==OP_Rowid ){
#ifdef SQLITE_DEBUG
      जर( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        म_लिखो("TRANSLATE OP_Rowid to OP_Sequence at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      जर( iAutoidxCur==0 ){
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
#जर !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
स्थिर शून्य whereTraceIndexInfoInputs(
  sqlite3_index_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  पूर्णांक i;
  जर( (sqlite3WhereTrace & 0x10)==0 ) परत;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  साठी(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].iTermOffset,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  साठी(i=0; i<p->nOrderBy; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->aOrderBy[i].iColumn,
       p->aOrderBy[i].desc);
  }
}
स्थिर शून्य whereTraceIndexInfoOutputs(
  sqlite3_index_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  पूर्णांक i;
  जर( (sqlite3WhereTrace & 0x10)==0 ) परत;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  साठी(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argvIndex,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_अंक);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->orderByConsumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimatedRows);
}
#अन्यथा
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
स्थिर पूर्णांक constraintCompatibleWithOuterJoin(
  स्थिरांक WhereTerm *pTerm,       /* WHERE clause term to check */
  स्थिरांक SrcItem *pSrc           /* Table we are trying to access */
){
  प्रतिज्ञा( (pSrc->fg.jointype&(JT_डावा|JT_LTORJ|JT_उजवा))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_डावा|JT_LTORJ|JT_उजवा))==JT_डावा );
  testcase( (pSrc->fg.jointype & (JT_डावा|JT_LTORJ|JT_उजवा))==JT_LTORJ );
  testcase( अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_OuterON) )
  testcase( अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_InnerON) );
  जर( !अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_OuterON|EP_InnerON)
   || pTerm->p_अभिव्यक्ती->w.iJoin != pSrc->iCursor
  ){
    परत 0;
  }
  जर( (pSrc->fg.jointype & (JT_डावा|JT_उजवा))!=0
   && NEVER(अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_InnerON))
  ){
    परत 0;
  }
  परत 1;
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
स्थिर SQLITE_NOINLINE पूर्णांक columnIsGoodIndexCandidate(
  स्थिरांक Table *pTab,
  पूर्णांक iCol
){
  स्थिरांक Index *pIdx;
  साठी(pIdx = pTab->pIndex; pIdx!=0; pIdx=pIdx->pNext){
    पूर्णांक j;
    साठी(j=0; j<pIdx->n_किल्ली_Col; j++){
       जर( pIdx->aiColumn[j]==iCol ){
         जर( j==0 ) परत 0;
         जर( pIdx->hasStat1 && pIdx->aiRowLogEst[j+1]>20 ) परत 0;
         थांबा;
       }
    }
  }
  परत 1;
}
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */



#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
स्थिर पूर्णांक termCanDriveIndex(
  स्थिरांक WhereTerm *pTerm,        /* WHERE clause term to check */
  स्थिरांक SrcItem *pSrc,           /* Table we are trying to access */
  स्थिरांक Bitmask notReady         /* Tables in outer loops of the join */
){
  अक्षर aff;
  पूर्णांक डावा_Col;
  
  जर( pTerm->डावा_Cursor!=pSrc->iCursor ) परत 0;
  जर( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) परत 0;
  प्रतिज्ञा( (pSrc->fg.jointype & JT_उजवा)==0 );
  जर( (pSrc->fg.jointype & (JT_डावा|JT_LTORJ|JT_उजवा))!=0
   && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
  ){
    परत 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  जर( (pTerm->prereq_उजवा & notReady)!=0 ) परत 0;
  प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
  डावा_Col = pTerm->u.x.डावा_Column;
  जर( डावा_Col<0 ) परत 0;
  aff = pSrc->pSTab->aCol[डावा_Col].affinity;
  जर( !sqlite3IndexAffinityOk(pTerm->p_अभिव्यक्ती, aff) ) परत 0;
  testcase( pTerm->p_अभिव्यक्ती->op==TK_IS );
  परत columnIsGoodIndexCandidate(pSrc->pSTab, डावा_Col);
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
स्थिर शून्य explainAutomaticIndex(
  Parse *pParse,
  Index *pIdx,                    /* Automatic index to explain */
  पूर्णांक bPartial,                   /* True if pIdx is a partial index */
  पूर्णांक *pAddrExplain               /* OUT: Address of OP_Explain */
){
  जर( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    Table *pTab = pIdx->pTable;
    स्थिरांक अक्षर *zSep = "";
    अक्षर *zText = 0;
    पूर्णांक ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    प्रतिज्ञा( pIdx->nColumn>1 );
    प्रतिज्ञा( pIdx->aiColumn[pIdx->nColumn-1]==XN_ROWID || !HasRowid(pTab) );
    साठी(ii=0; ii<(pIdx->nColumn-1); ii++){
      स्थिरांक अक्षर *zName = 0;
      पूर्णांक iCol = pIdx->aiColumn[ii];

      zName = pTab->aCol[iCol].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    zText = sqlite3_str_finish(pStr);
    जर( zText==0 ){
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
# define explainAutomaticIndex(a,b,c,d)
#endif

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
स्थिर SQLITE_NOINLINE शून्य constructAutomaticIndex(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  स्थिरांक Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  पूर्णांक n_किल्ली_Col;                /* Number of columns in the constructed index */
  WhereTerm *pTerm;           /* A single term of the WHERE clause */
  WhereTerm *pWCEnd;          /* End of pWC->a[] */
  Index *pIdx;                /* Object describing the transient index */
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
  अभिव्यक्ती *pPartial = 0;         /* Partial Index Expression */
  पूर्णांक iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  SrcItem *pSrc;              /* The FROM clause term to get the next index */
  पूर्णांक addrCounter = 0;        /* Address where integer counter is initialized */
  पूर्णांक regBase;                /* Array of registers where record is assembled */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णांक addrExp = 0;            /* Address of OP_Explain */
#endif

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );
  addrInit = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_किल्ली_Col = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  pTable = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  साठी(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    अभिव्यक्ती *p_अभिव्यक्ती = pTerm->p_अभिव्यक्ती;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    जर( (pTerm->wtFlags & TERM_VIRTUAL)==0
     && sqlite3_अभिव्यक्ती_Is_Single_Table_Constraint(p_अभिव्यक्ती, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_अभिव्यक्ती_And(pParse, pPartial,
                                sqlite3_अभिव्यक्ती_Dup(pParse->db, p_अभिव्यक्ती, 0));
    }
    जर( termCanDriveIndex(pTerm, pSrc, notReady) ){
      पूर्णांक iCol;
      Bitmask cMask;
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.डावा_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      जर( !sentWarning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", pTable->zName,
            pTable->aCol[iCol].zCnName);
        sentWarning = 1;
      }
      जर( (idxCols & cMask)==0 ){
        जर( whereLoopResize(pParse->db, pLoop, n_किल्ली_Col+1) ){
          जा end_auto_index_create;
        }
        pLoop->aLTerm[n_किल्ली_Col++] = pTerm;
        idxCols |= cMask;
      }
    }
  }
  प्रतिज्ञा( n_किल्ली_Col>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_किल्ली_Col;
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
  जर( IsView(pTable) ){
    extraCols = ALLBITS & ~idxCols;
  }अन्यथा{
    extraCols = pSrc->colUsed & (~idxCols | MASKBIT(BMS-1));
  }
  जर( !HasRowid(pTable) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    साठी(i=0; i<pTable->nCol; i++){
      जर( (pTable->aCol[i].colFlags & COLFLAG_PRIMKEY)==0 ) सुरूठेवा;
      जर( i>=BMS-1 ){
        extraCols |= MASKBIT(BMS-1);
        थांबा;
      }
      जर( idxCols & MASKBIT(i) ) सुरूठेवा;
      extraCols |= MASKBIT(i);
    }
  }
  mxBitCol = MIN(BMS-1,pTable->nCol);
  testcase( pTable->nCol==BMS-1 );
  testcase( pTable->nCol==BMS-2 );
  साठी(i=0; i<mxBitCol; i++){
    जर( extraCols & MASKBIT(i) ) n_किल्ली_Col++;
  }
  जर( pSrc->colUsed & MASKBIT(BMS-1) ){
    n_किल्ली_Col += pTable->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  प्रतिज्ञा( n_किल्ली_Col <= pTable->nCol + MAX(0, pTable->nCol - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3AllocateIndexObject(pParse->db, n_किल्ली_Col+HasRowid(pTable),
                                    0, &zNotUsed);
  जर( pIdx==0 ) जा end_auto_index_create;
  pLoop->u.btree.pIndex = pIdx;
  pIdx->zName = "auto-index";
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  साठी(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    जर( termCanDriveIndex(pTerm, pSrc, notReady) ){
      पूर्णांक iCol;
      Bitmask cMask;
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.डावा_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      जर( (idxCols & cMask)==0 ){
        अभिव्यक्ती *pX = pTerm->p_अभिव्यक्ती;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.x.डावा_Column;
        pColl = sqlite3_अभिव्यक्ती_Compare_Coll_Seq(pParse, pX);
        प्रतिज्ञा( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        जर( !sqlite3_Is_द्विमान(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_द्विमान;
        n++;
      }
    }
  }
  प्रतिज्ञा( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  साठी(i=0; i<mxBitCol; i++){
    जर( extraCols & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_द्विमान;
      n++;
    }
  }
  जर( pSrc->colUsed & MASKBIT(BMS-1) ){
    साठी(i=BMS-1; i<pTable->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_द्विमान;
      n++;
    }
  }
  प्रतिज्ञा( n==n_किल्ली_Col );
  जर( HasRowid(pTable) ){
    pIdx->aiColumn[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_द्विमान;
  }

  /* Create the automatic index */
  explainAutomaticIndex(pParse, pIdx, pPartial!=0, &addrExp);
  प्रतिज्ञा( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_किल्ली_Col+1);
  sqlite3_Vdbe_Set_P4_किल्ली_Info(pParse, pIdx);
  VdbeComment((v, "for %s", pTable->zName));
  जर( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  प्रतिज्ञा( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  जर( pSrc->fg.viaCoroutine ){
    पूर्णांक regYield;
    Subquery *pSubq;
    प्रतिज्ञा( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    प्रतिज्ञा( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3VdbeAddOp2(v, OP_Integer, 0, 0);
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_शीर्ष =  sqlite3VdbeAddOp1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }अन्यथा{
    प्रतिज्ञा( pLevel->addrHalt );
    addr_शीर्ष = sqlite3VdbeAddOp2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  जर( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_अभिव्यक्ती_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  regRecord = sqlite3GetTempReg(pParse);
  regBase = sqlite3_Generate_Index_किल्ली(
      pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0
  );
  जर( pLevel->regFilter ){
    प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0,
                         regBase, pLoop->u.btree.nEq);
  }
  sqlite3VdbeScanStatusCounters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3VdbeAddOp2(v, OP_Idx_घाला, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  जर( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  जर( pSrc->fg.viaCoroutine ){
    प्रतिज्ञा( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, regBase+n);
    testcase( pParse->db->mallocFailed );
    प्रतिज्ञा( pLevel->iIdxCur>0 );
    translateColumnToCopy(pParse, addr_शीर्ष, pLevel->iTabCur,
                          pSrc->u4.pSubq->regResult, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_शीर्ष);
    pSrc->fg.viaCoroutine = 0;
    sqlite3VdbeJumpHere(v, addr_शीर्ष);
  }अन्यथा{
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addr_शीर्ष+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    जर( (pSrc->fg.jointype & JT_डावा)!=0 ){
      sqlite3VdbeJumpHere(v, addr_शीर्ष);
    }
  }
  sqlite3ReleaseTempReg(pParse, regRecord);
 
  /* Jump here when skipping the initialization */
  sqlite3VdbeJumpHere(v, addrInit);
  sqlite3VdbeScanStatusRange(v, addrExp, addrExp, -1);

end_auto_index_create:
  sqlite3_अभिव्यक्ती_Delete(pParse->db, pPartial);
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
स्थिर SQLITE_NOINLINE शून्य sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  पूर्णांक iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  पूर्णांक addrOnce;                        /* Address of opening OP_Once */
  पूर्णांक addr_शीर्ष;                         /* Address of OP_Rewind */
  पूर्णांक addrCont;                        /* Jump here to skip a row */
  स्थिरांक WhereTerm *pTerm;              /* For looping over WHERE clause terms */
  स्थिरांक WhereTerm *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  पूर्णांक iCur;                            /* Cursor for table getting the filter */
  Indexed_अभिव्यक्ती *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  Indexed_अभिव्यक्ती *saved_p_Idx_Part_अभिव्यक्ती;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_अभिव्यक्ती = pParse->p_Idx_Part_अभिव्यक्ती;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_अभिव्यक्ती = 0;

  प्रतिज्ञा( pLoop!=0 );
  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pLoop->wsFlags & WHERE_BLOOMFILTER );
  प्रतिज्ञा( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  करा{
    स्थिरांक SrcList *pTabList;
    स्थिरांक SrcItem *pItem;
    स्थिरांक Table *pTab;
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
    प्रतिज्ञा( pItem!=0 );
    pTab = pItem->pSTab;
    प्रतिज्ञा( pTab!=0 );
    sz = sqlite3LogEstToInt(pTab->nRowLogEst);
    जर( sz<10000 ){
      sz = 10000;
    }अन्यथा जर( sz>10000000 ){
      sz = 10000000;
    }
    प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp2(v, OP_Blob, (पूर्णांक)sz, pLevel->regFilter);

    addr_शीर्ष = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.nTerm];
    साठी(pTerm=pWInfo->sWC.a; pTerm<pWCEnd; pTerm++){
      अभिव्यक्ती *p_अभिव्यक्ती = pTerm->p_अभिव्यक्ती;
      जर( (pTerm->wtFlags & TERM_VIRTUAL)==0
       && sqlite3_अभिव्यक्ती_Is_Single_Table_Constraint(p_अभिव्यक्ती, pTabList, iSrc, 0)
      ){
        sqlite3_अभिव्यक्ती_If_False(pParse, pTerm->p_अभिव्यक्ती, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    जर( pLoop->wsFlags & WHERE_IPK ){
      पूर्णांक r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, r1);
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, 1);
      sqlite3ReleaseTempReg(pParse, r1);
    }अन्यथा{
      Index *pIdx = pLoop->u.btree.pIndex;
      पूर्णांक n = pLoop->u.btree.nEq;
      पूर्णांक r1 = sqlite3GetTempRange(pParse, n);
      पूर्णांक jj;
      प्रतिज्ञा( pIdx!=0 );
      साठी(jj=0; jj<n; jj++){
        प्रतिज्ञा( pIdx->pTable==pItem->pSTab );
        sqlite3_अभिव्यक्ती_Code_Load_Index_Column(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, n);
      sqlite3ReleaseTempRange(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addr_शीर्ष+1);
    VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr_शीर्ष);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    पर्यंत( ++iLevel < pWInfo->nLevel ){
      स्थिरांक SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      जर( pTabItem->fg.jointype & (JT_डावा|JT_LTORJ) ) सुरूठेवा;
      pLoop = pLevel->pWLoop;
      जर( NEVER(pLoop==0) ) सुरूठेवा;
      जर( pLoop->prereq & notReady ) सुरूठेवा;
      जर( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_COLUMN_IN))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        थांबा;
      }
    }
  }पर्यंत( iLevel < pWInfo->nLevel );
  sqlite3VdbeJumpHere(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_अभिव्यक्ती = saved_p_Idx_Part_अभिव्यक्ती;
}


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
स्थिर WhereTerm *termFromWhereClause(WhereClause *pWC, पूर्णांक iTerm){
  WhereClause *p;
  साठी(p=pWC; p; p=p->pOuter){
    जर( iTerm<p->nTerm ) परत &p->a[iTerm];
    iTerm -= p->nTerm;
  }
  परत 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
स्थिर sqlite3_index_info *allocateIndexInfo(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  SrcItem *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  पूर्णांक i, j;
  पूर्णांक nTerm;
  Parse *pParse = pWInfo->pParse;
  संरचना sqlite3_index_constraint *pIdxCons;
  संरचना sqlite3_index_orderby *pIdxOrderBy;
  संरचना sqlite3_index_constraint_usage *pUsage;
  संरचना HiddenIndexInfo *pHidden;
  WhereTerm *pTerm;
  पूर्णांक nOrderBy;
  sqlite3_index_info *pIdxInfo;
  u16 mNoOmit = 0;
  स्थिरांक Table *pTab;
  पूर्णांक eDistinct = 0;
  अभिव्यक्ती_List *pOrderBy = pWInfo->pOrderBy;
  WhereClause *p;

  प्रतिज्ञा( pSrc!=0 );
  pTab = pSrc->pSTab;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  साठी(p=pWC, nTerm=0; p; p=p->pOuter){
    साठी(i=0, pTerm=p->a; i<p->nTerm; i++, pTerm++){
      pTerm->wtFlags &= ~TERM_OK;
      जर( pTerm->डावा_Cursor != pSrc->iCursor ) सुरूठेवा;
      जर( pTerm->prereq_उजवा & mUnusable ) सुरूठेवा;
      प्रतिज्ञा( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
      testcase( pTerm->eOperator & WO_IN );
      testcase( pTerm->eOperator & WO_ISNULL );
      testcase( pTerm->eOperator & WO_IS );
      testcase( pTerm->eOperator & WO_ALL );
      जर( (pTerm->eOperator & ~(WO_EQUIV))==0 ) सुरूठेवा;
      जर( pTerm->wtFlags & TERM_VNULL ) सुरूठेवा;

      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      प्रतिज्ञा( pTerm->u.x.डावा_Column>=XN_ROWID );
      प्रतिज्ञा( pTerm->u.x.डावा_Column<pTab->nCol );
      जर( (pSrc->fg.jointype & (JT_डावा|JT_LTORJ|JT_उजवा))!=0
          && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
        ){
        सुरूठेवा;
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
  जर( pOrderBy ){
    पूर्णांक n = pOrderBy->n_अभिव्यक्ती;
    साठी(i=0; i<n; i++){
      अभिव्यक्ती *p_अभिव्यक्ती = pOrderBy->a[i].p_अभिव्यक्ती;
      अभिव्यक्ती *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      जर( sqlite3_अभिव्यक्ती_Is_Constant(0, p_अभिव्यक्ती) ){
        सुरूठेवा;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      जर( pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) थांबा;

      /* First case - a direct column references without a COLLATE operator */
      जर( p_अभिव्यक्ती->op==TK_COLUMN && p_अभिव्यक्ती->iTable==pSrc->iCursor ){
        प्रतिज्ञा( p_अभिव्यक्ती->iColumn>=XN_ROWID && p_अभिव्यक्ती->iColumn<pTab->nCol );
        सुरूठेवा;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      जर( p_अभिव्यक्ती->op==TK_COLLATE
       && (pE2 = p_अभिव्यक्ती->p_डावा)->op==TK_COLUMN
       && pE2->iTable==pSrc->iCursor
      ){
        स्थिरांक अक्षर *zColl;  /* The collating sequence name */
        प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
        प्रतिज्ञा( p_अभिव्यक्ती->u.zToken!=0 );
        प्रतिज्ञा( pE2->iColumn>=XN_ROWID && pE2->iColumn<pTab->nCol );
        p_अभिव्यक्ती->iColumn = pE2->iColumn;
        जर( pE2->iColumn<0 ) सुरूठेवा;  /* Collseq does not matter for rowid */
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        जर( zColl==0 ) zColl = sqlite3_Str_द्विमान;
        जर( sqlite3_stricmp(p_अभिव्यक्ती->u.zToken, zColl)==0 ) सुरूठेवा;
      }

      /* No matches cause a break out of the loop */
      थांबा;
    }
    जर( i==n ){
      पूर्णांक bSortByGroup = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      nOrderBy = n;
      जर( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowidUsed ){
        eDistinct = 2 + bSortByGroup;
      }अन्यथा जर( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - bSortByGroup;
      }अन्यथा जर( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, आकार(*pIdxInfo)
                           + (आकार(*pIdxCons) + आकार(*pUsage))*nTerm
                           + आकार(*pIdxOrderBy)*nOrderBy
                           + SZ_HIDDENINDEXINFO(nTerm) );
  जर( pIdxInfo==0 ){
    sqlite3_Error_संदेश(pParse, "out of memory");
    परत 0;
  }
  pHidden = (संरचना HiddenIndexInfo*)&pIdxInfo[1];
  pIdxCons = (संरचना sqlite3_index_constraint*)&pHidden->aRhs[nTerm];
  pIdxOrderBy = (संरचना sqlite3_index_orderby*)&pIdxCons[nTerm];
  pUsage = (संरचना sqlite3_index_constraint_usage*)&pIdxOrderBy[nOrderBy];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->aOrderBy = pIdxOrderBy;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->colUsed = (sqlite3_int64)pSrc->colUsed;
  जर( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    Index *pPk = sqlite3_Primary_किल्ली_Index((Table*)pTab);
    प्रतिज्ञा( pPk!=0 );
    साठी(i=0; i<pPk->n_किल्ली_Col; i++){
      पूर्णांक iCol = pPk->aiColumn[i];
      प्रतिज्ञा( iCol>=0 );
      जर( iCol>=BMS-1 ) iCol = BMS-1;
      pIdxInfo->colUsed |= MASKBIT(iCol);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->mIn = 0;
  साठी(p=pWC, i=j=0; p; p=p->pOuter){
    पूर्णांक nLast = i+p->nTerm;;
    साठी(pTerm=p->a; i<nLast; i++, pTerm++){
      u16 op;
      जर( (pTerm->wtFlags & TERM_OK)==0 ) सुरूठेवा;
      pIdxCons[j].iColumn = pTerm->u.x.डावा_Column;
      pIdxCons[j].iTermOffset = i;
      op = pTerm->eOperator & WO_ALL;
      जर( op==WO_IN ){
        जर( (pTerm->wtFlags & TERM_SLICE)==0 ){
          pHidden->mIn |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      जर( op==WO_AUX ){
        pIdxCons[j].op = pTerm->eMatchOp;
      }अन्यथा जर( op & (WO_ISNULL|WO_IS) ){
        जर( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_ISNULL;
        }अन्यथा{
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_IS;
        }
      }अन्यथा{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        प्रतिज्ञा( WO_EQ==SQLITE_INDEX_CONSTRAINT_EQ );
        प्रतिज्ञा( WO_LT==SQLITE_INDEX_CONSTRAINT_LT );
        प्रतिज्ञा( WO_LE==SQLITE_INDEX_CONSTRAINT_LE );
        प्रतिज्ञा( WO_GT==SQLITE_INDEX_CONSTRAINT_GT );
        प्रतिज्ञा( WO_GE==SQLITE_INDEX_CONSTRAINT_GE );
        प्रतिज्ञा( pTerm->eOperator&(WO_IN|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        जर( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_अभिव्यक्ती_Is_Vector(pTerm->p_अभिव्यक्ती->p_उजवा)
          ){
          testcase( j!=i );
          जर( j<16 ) mNoOmit |= (1 << j);
          जर( op==WO_LT ) pIdxCons[j].op = WO_LE;
          जर( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  प्रतिज्ञा( j==nTerm );
  pIdxInfo->nConstraint = j;
  साठी(i=j=0; i<nOrderBy; i++){
    अभिव्यक्ती *p_अभिव्यक्ती = pOrderBy->a[i].p_अभिव्यक्ती;
    जर( sqlite3_अभिव्यक्ती_Is_Constant(0, p_अभिव्यक्ती) ) सुरूठेवा;
    प्रतिज्ञा( p_अभिव्यक्ती->op==TK_COLUMN
         || (p_अभिव्यक्ती->op==TK_COLLATE && p_अभिव्यक्ती->p_डावा->op==TK_COLUMN
              && p_अभिव्यक्ती->iColumn==p_अभिव्यक्ती->p_डावा->iColumn) );
    pIdxOrderBy[j].iColumn = p_अभिव्यक्ती->iColumn;
    pIdxOrderBy[j].desc = pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC;
    j++;
  }
  pIdxInfo->nOrderBy = j;

  *pmNoOmit = mNoOmit;
  परत pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
स्थिर शून्य freeIdxStr(sqlite3_index_info *pIdxInfo){
  जर( pIdxInfo->needToFreeIdxStr ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->needToFreeIdxStr = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
स्थिर शून्य freeIndexInfo(sqlite3 *db, sqlite3_index_info *pIdxInfo){
  HiddenIndexInfo *pHidden;
  पूर्णांक i;
  प्रतिज्ञा( pIdxInfo!=0 );
  pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  प्रतिज्ञा( pHidden->pParse!=0 );
  प्रतिज्ञा( pHidden->pParse->db==db );
  साठी(i=0; i<pIdxInfo->nConstraint; i++){
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
स्थिर पूर्णांक vtabBestIndex(Parse *pParse, Table *pTab, sqlite3_index_info *p){
  पूर्णांक rc;
  sqlite3_vtab *pVtab;

  प्रतिज्ञा( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  whereTraceIndexInfoInputs(p, pTab);
  pParse->db->nSchemaLock++;
  rc = pVtab->pModule->xBestIndex(pVtab, p);
  pParse->db->nSchemaLock--;
  whereTraceIndexInfoOutputs(p, pTab);

  जर( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    जर( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }अन्यथा जर( !pVtab->z_Err_संदेश ){
      sqlite3_Error_संदेश(pParse, "%s", sqlite3ErrStr(rc));
    }अन्यथा{
      sqlite3_Error_संदेश(pParse, "%s", pVtab->z_Err_संदेश);
    }
  }
  जर( pTab->u.vtab.p->bAllSchemas ){
    sqlite3VtabUsesAllSchemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_संदेश);
  pVtab->z_Err_संदेश = 0;
  परत rc;
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
स्थिर पूर्णांक where_किल्ली_Stats(
  Parse *pParse,              /* Database connection */
  Index *pIdx,                /* Index to consider domain of */
  UnpackedRecord *pRec,       /* Vector of values to consider */
  पूर्णांक roundUp,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  IndexSample *aSample = pIdx->aSample;
  पूर्णांक iCol;                   /* Index of required stats in anEq[] etc. */
  पूर्णांक i;                      /* Index of first sample >= pRec */
  पूर्णांक iSample;                /* Smallest sample larger than or equal to pRec */
  पूर्णांक iMin = 0;               /* Smallest sample not yet tested */
  पूर्णांक iTest;                  /* Next sample to test */
  पूर्णांक res;                    /* Result of comparison operation */
  पूर्णांक nField;                 /* Number of fields in pRec */
  tRowcnt iLower = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER( pParse );
#endif
  प्रतिज्ञा( pRec!=0 );
  प्रतिज्ञा( pIdx->nSample>0 );
  प्रतिज्ञा( pRec->nField>0 );


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
  जर( !HasRowid(pIdx->pTable) && Is_Primary_किल्ली_Index(pIdx) ){
    nField = pIdx->n_किल्ली_Col;
  }अन्यथा{
    nField = pIdx->nColumn;
  }
  nField = MIN(pRec->nField, nField);
  iCol = 0;
  iSample = pIdx->nSample * nField;
  करा{
    पूर्णांक iSamp;                    /* Index in aSample[] of test sample */
    पूर्णांक n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / nField;
    जर( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      साठी(n=(iTest % nField) + 1; n<nField; n++){
        जर( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) थांबा;
      }
    }अन्यथा{
      n = iTest + 1;
    }

    pRec->nField = n;
    res = sqlite3VdbeRecordCompare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    जर( res<0 ){
      iLower = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }अन्यथा जर( res==0 && n<nField ){
      iLower = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }अन्यथा{
      iSample = iTest;
      iCol = n-1;
    }
  }पर्यंत( res && iMin<iSample );
  i = iSample / nField;

#ifdef SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  जर( pParse->db->mallocFailed==0 ){
    जर( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      प्रतिज्ञा( i<pIdx->nSample );
      प्रतिज्ञा( iCol==nField-1 );
      pRec->nField = nField;
      प्रतिज्ञा( 0==sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }अन्यथा{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      प्रतिज्ञा( i<=pIdx->nSample && i>=0 );
      pRec->nField = iCol+1;
      प्रतिज्ञा( i==pIdx->nSample
           || sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      जर( iCol>0 ){
        pRec->nField = iCol;
        प्रतिज्ञा( sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      जर( i>0 ){
        pRec->nField = nField;
        प्रतिज्ञा( sqlite3VdbeRecordCompare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#endif /* ifdef SQLITE_DEBUG */

  जर( res==0 ){
    /* Record pRec is equal to sample i */
    प्रतिज्ञा( iCol==nField-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }अन्यथा{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt iUpper, iGap;
    जर( i>=pIdx->nSample ){
      iUpper = pIdx->nRowEst0;
    }अन्यथा{
      iUpper = aSample[i].anLt[iCol];
    }

    जर( iLower>=iUpper ){
      iGap = 0;
    }अन्यथा{
      iGap = iUpper - iLower;
    }
    जर( roundUp ){
      iGap = (iGap*2)/3;
    }अन्यथा{
      iGap = iGap/3;
    }
    aStat[0] = iLower + iGap;
    aStat[1] = pIdx->aAvgEq[nField-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->nField = nField;
  परत i;
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
स्थिर LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew){
  LogEst nRet = nNew;
  जर( pTerm ){
    जर( pTerm->truthProb<=0 ){
      nRet += pTerm->truthProb;
    }अन्यथा जर( (pTerm->wtFlags & TERM_VNULL)==0 ){
      nRet -= 20;        प्रतिज्ञा( 20==sqlite3LogEst(4) );
    }
  }
  परत nRet;
}


#ifdef SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
अक्षर sqlite3IndexColumnAffinity(sqlite3 *db, Index *pIdx, पूर्णांक iCol){
  प्रतिज्ञा( iCol>=0 && iCol<pIdx->nColumn );
  जर( !pIdx->zColAff ){
    जर( sqlite3IndexAffinityStr(db, pIdx)==0 ) परत SQLITE_AFF_BLOB;
  }
  प्रतिज्ञा( pIdx->zColAff[iCol]!=0 );
  परत pIdx->zColAff[iCol];
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
स्थिर पूर्णांक whereRangeSkipScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  पूर्णांक *pbDone          /* Set to true if at least one expr. value extracted */
){
  Index *p = pLoop->u.btree.pIndex;
  पूर्णांक nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  पूर्णांक nLower = -1;
  पूर्णांक nUpper = p->nSample+1;
  पूर्णांक rc = SQLITE_OK;
  u8 aff = sqlite3IndexColumnAffinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_मूल्य *p1 = 0;          /* Value extracted from pLower */
  sqlite3_मूल्य *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_मूल्य *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  जर( pLower ){
    rc = sqlite3_Stat4_मूल्य_From_अभिव्यक्ती(pParse, pLower->p_अभिव्यक्ती->p_उजवा, aff, &p1);
    nLower = 0;
  }
  जर( pUpper && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_मूल्य_From_अभिव्यक्ती(pParse, pUpper->p_अभिव्यक्ती->p_उजवा, aff, &p2);
    nUpper = p2 ? 0 : p->nSample;
  }

  जर( p1 || p2 ){
    पूर्णांक i;
    पूर्णांक nDiff;
    साठी(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      जर( rc==SQLITE_OK && p1 ){
        पूर्णांक res = sqlite3MemCompare(p1, pVal, pColl);
        जर( res>=0 ) nLower++;
      }
      जर( rc==SQLITE_OK && p2 ){
        पूर्णांक res = sqlite3MemCompare(p2, pVal, pColl);
        जर( res>=0 ) nUpper++;
      }
    }
    nDiff = (nUpper - nLower);
    जर( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    जर( nDiff!=1 || pUpper==0 || pLower==0 ){
      पूर्णांक nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->nOut -= nAdjust;
      *pbDone = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           nLower, nUpper, nAdjust*-1, pLoop->nOut));
    }

  }अन्यथा{
    प्रतिज्ञा( *pbDone==0 );
  }

  sqlite3_मूल्य_Free(p1);
  sqlite3_मूल्य_Free(p2);
  sqlite3_मूल्य_Free(pVal);

  परत rc;
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
स्थिर पूर्णांक whereRangeScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक nOut = pLoop->nOut;
  LogEst nNew;

#ifdef SQLITE_ENABLE_STAT4
  Index *p = pLoop->u.btree.pIndex;
  पूर्णांक nEq = pLoop->u.btree.nEq;

  जर( p->nSample>0 && ALWAYS(nEq<p->nSampleCol)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    जर( nEq==pBuilder->nRecValid ){
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

      जर( pRec ){
        testcase( pRec->nField!=pBuilder->nRecValid );
        pRec->nField = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      जर( nEq==0 ){
        iLower = 0;
        iUpper = p->nRowEst0;
      }अन्यथा{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_किल्ली_Stats(pParse, p, pRec, 0, a);
        iLower = a[0];
        iUpper = a[0] + a[1];
      }

      प्रतिज्ञा( pLower==0 || (pLower->eOperator & (WO_GT|WO_GE))!=0 );
      प्रतिज्ञा( pUpper==0 || (pUpper->eOperator & (WO_LT|WO_LE))!=0 );
      प्रतिज्ञा( p->aSortOrder!=0 );
      जर( p->aSortOrder[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(WhereTerm*, pLower, pUpper);
        SWAP(पूर्णांक, nBtm, n_शीर्ष);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      जर( pLower ){
        पूर्णांक n;                    /* Values extracted from pExpr */
        अभिव्यक्ती *p_अभिव्यक्ती = pLower->p_अभिव्यक्ती->p_उजवा;
        rc = sqlite3_Stat4_Probe_Set_मूल्य(pParse, p, &pRec, p_अभिव्यक्ती, nBtm, nEq, &n);
        जर( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          जर( sqlite3_अभिव्यक्ती_Vector_Size(p_अभिव्यक्ती)>n ) mask = (WO_LE|WO_LT);
          iLwrIdx = where_किल्ली_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((pLower->eOperator & mask) ? a[1] : 0);
          जर( iNew>iLower ) iLower = iNew;
          nOut--;
          pLower = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      जर( pUpper ){
        पूर्णांक n;                    /* Values extracted from pExpr */
        अभिव्यक्ती *p_अभिव्यक्ती = pUpper->p_अभिव्यक्ती->p_उजवा;
        rc = sqlite3_Stat4_Probe_Set_मूल्य(pParse, p, &pRec, p_अभिव्यक्ती, n_शीर्ष, nEq, &n);
        जर( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          जर( sqlite3_अभिव्यक्ती_Vector_Size(p_अभिव्यक्ती)>n ) mask = (WO_LE|WO_LT);
          iUprIdx = where_किल्ली_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((pUpper->eOperator & mask) ? a[1] : 0);
          जर( iNew<iUpper ) iUpper = iNew;
          nOut--;
          pUpper = 0;
        }
      }

      pBuilder->pRec = pRec;
      जर( rc==SQLITE_OK ){
        जर( iUpper>iLower ){
          nNew = sqlite3LogEst(iUpper - iLower);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          जर( iLwrIdx==iUprIdx ){ nNew -= 20; }
          प्रतिज्ञा( 20==sqlite3LogEst(4) );
        }अन्यथा{
          nNew = 10;        प्रतिज्ञा( 10==sqlite3LogEst(2) );
        }
        जर( nNew<nOut ){
          nOut = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)iLower, (u32)iUpper, nOut));
      }
    }अन्यथा{
      पूर्णांक bDone = 0;
      rc = whereRangeSkipScanEst(pParse, pLower, pUpper, pLoop, &bDone);
      जर( bDone ) परत rc;
    }
  }
#अन्यथा
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pBuilder);
  प्रतिज्ञा( pLower || pUpper );
#endif
  प्रतिज्ञा( pUpper==0 || (pUpper->wtFlags & TERM_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  जर( pLower && pLower->truthProb>0 && pUpper && pUpper->truthProb>0 ){
    nNew -= 20;
  }

  nOut -= (pLower!=0) + (pUpper!=0);
  जर( nNew<10 ) nNew = 10;
  जर( nNew<nOut ) nOut = nNew;
#जर defined(WHERETRACE_ENABLED)
  जर( pLoop->nOut>nOut ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->nOut, nOut));
  }
#endif
  pLoop->nOut = (LogEst)nOut;
  परत rc;
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
स्थिर पूर्णांक whereEqualScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  अभिव्यक्ती *p_अभिव्यक्ती,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  पूर्णांक nEq = pBuilder->pNew->u.btree.nEq;
  UnpackedRecord *pRec = pBuilder->pRec;
  पूर्णांक rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  पूर्णांक bOk;

  प्रतिज्ञा( nEq>=1 );
  प्रतिज्ञा( nEq<=p->nColumn );
  प्रतिज्ञा( p->aSample!=0 );
  प्रतिज्ञा( p->nSample>0 );
  प्रतिज्ञा( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  जर( pBuilder->nRecValid<(nEq-1) ){
    परत SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  जर( nEq>=p->nColumn ){
    *pnRow = 1;
    परत SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_Set_मूल्य(pParse, p, &pRec, p_अभिव्यक्ती, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  जर( rc!=SQLITE_OK ) परत rc;
  जर( bOk==0 ) परत SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_किल्ली_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (पूर्णांक)a[1]));
  *pnRow = a[1];
 
  परत rc;
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
स्थिर पूर्णांक whereInScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  अभिव्यक्ती_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  i64 nRow0 = sqlite3LogEstToInt(p->aiRowLogEst[0]);
  पूर्णांक nRecValid = pBuilder->nRecValid;
  पूर्णांक rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  पूर्णांक i;                  /* Loop counter */

  प्रतिज्ञा( p->aSample!=0 );
  साठी(i=0; rc==SQLITE_OK && i<pList->n_अभिव्यक्ती; i++){
    nEst = nRow0;
    rc = whereEqualScanEst(pParse, pBuilder, pList->a[i].p_अभिव्यक्ती, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  जर( rc==SQLITE_OK ){
    जर( nRowEst > (tRowcnt)nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", nRowEst));
  }
  प्रतिज्ञा( pBuilder->nRecValid==nRecValid );
  परत rc;
}
#endif /* SQLITE_ENABLE_STAT4 */


#जर defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
शून्य sqlite3WhereTermPrint(WhereTerm *pTerm, पूर्णांक iTerm){
  जर( pTerm==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", iTerm);
  }अन्यथा{
    अक्षर zType[8];
    अक्षर z_डावा[50];
    स_नकल(zType, "....", 5);
    जर( pTerm->wtFlags & TERM_VIRTUAL ) zType[0] = 'V';
    जर( pTerm->eOperator & WO_EQUIV  ) zType[1] = 'E';
    जर( अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_OuterON) ) zType[2] = 'L';
    जर( pTerm->wtFlags & TERM_CODED  ) zType[3] = 'C';
    जर( pTerm->eOperator & WO_SINGLE ){
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(आकार(z_डावा),z_डावा,"left={%d:%d}",
                       pTerm->डावा_Cursor, pTerm->u.x.डावा_Column);
    }अन्यथा जर( (pTerm->eOperator & WO_OR)!=0 && pTerm->u.pOrInfo!=0 ){
      sqlite3_snprintf(आकार(z_डावा),z_डावा,"indexable=0x%llx",
                       pTerm->u.pOrInfo->indexable);
    }अन्यथा{
      sqlite3_snprintf(आकार(z_डावा),z_डावा,"left=%d", pTerm->डावा_Cursor);
    }
    iTerm = pTerm->iTerm = MAX(iTerm,pTerm->iTerm);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       iTerm, pTerm, zType, z_डावा, pTerm->eOperator, pTerm->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    जर( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        pTerm->truthProb, (u64)pTerm->prereqAll, (u64)pTerm->prereq_उजवा);
    }
    जर( (pTerm->eOperator & (WO_OR|WO_AND))==0 && pTerm->u.x.iField ){
      sqlite3DebugPrintf(" iField=%d", pTerm->u.x.iField);
    }
    जर( pTerm->iParent>=0 ){
      sqlite3DebugPrintf(" iParent=%d", pTerm->iParent);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_वृक्ष_View_अभिव्यक्ती(0, pTerm->p_अभिव्यक्ती, 0);
  }
}
शून्य sqlite3ShowWhereTerm(WhereTerm *pTerm){
  sqlite3WhereTermPrint(pTerm, 0);
}
#endif

#ifdef WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
शून्य sqlite3WhereClausePrint(WhereClause *pWC){
  पूर्णांक i;
  साठी(i=0; i<pWC->nTerm; i++){
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
शून्य sqlite3WhereLoopPrint(स्थिरांक WhereLoop *p, स्थिरांक WhereClause *pWC){
  WhereInfo *pWInfo;
  जर( pWC ){
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
  जर( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    स्थिरांक अक्षर *zName;
    जर( p->u.btree.pIndex && (zName = p->u.btree.pIndex->zName)!=0 ){
      जर( म_भेदन(zName, "sqlite_autoindex_", 17)==0 ){
        पूर्णांक i = sqlite3Strlen30(zName) - 1;
        पर्यंत( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }अन्यथा{
      sqlite3DebugPrintf("%20s","");
    }
  }अन्यथा{
    अक्षर *z;
    जर( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_अंक, p->u.vtab.idxStr, p->u.vtab.omitMask);
    }अन्यथा{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_अंक, p->u.vtab.omitMask);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  जर( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }अन्यथा{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  जर( pWInfo && pWInfo->bStarUsed && p->rStarDelta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->rSetup, p->rRun, p->nOut, p->rStarDelta);
  }अन्यथा{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->rSetup, p->rRun, p->nOut);
  }
  जर( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    पूर्णांक i;
    साठी(i=0; i<p->nLTerm; i++){
      sqlite3WhereTermPrint(p->aLTerm[i], i);
    }
  }
}
शून्य sqlite3ShowWhereLoop(स्थिरांक WhereLoop *p){
  जर( p ) sqlite3WhereLoopPrint(p, 0);
}
शून्य sqlite3ShowWhereLoopList(स्थिरांक WhereLoop *p){
  पर्यंत( p ){
    sqlite3ShowWhereLoop(p);
    p = p->pNextLoop;
  }
}
#endif

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
स्थिर शून्य whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = ArraySize(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
स्थिर शून्य whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  जर( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_INDEX) ){
    जर( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }अन्यथा जर( (p->wsFlags & WHERE_AUTO_INDEX)!=0 && p->u.btree.pIndex!=0 ){
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
स्थिर शून्य whereLoopClear(sqlite3 *db, WhereLoop *p){
  जर( p->aLTerm!=p->aLTermSpace ){
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
स्थिर पूर्णांक whereLoopResize(sqlite3 *db, WhereLoop *p, पूर्णांक n){
  WhereTerm **paNew;
  जर( p->nLSlot>=n ) परत SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, आकार(p->aLTerm[0])*n);
  जर( paNew==0 ) परत SQLITE_NOMEM_BKPT;
  स_नकल(paNew, p->aLTerm, आकार(p->aLTerm[0])*p->nLSlot);
  जर( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  परत SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
स्थिर पूर्णांक whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom){
  whereLoopClearUnion(db, pTo);
  जर( pFrom->nLTerm > pTo->nLSlot
   && whereLoopResize(db, pTo, pFrom->nLTerm)
  ){
    स_रखो(pTo, 0, WHERE_LOOP_XFER_SZ);
    परत SQLITE_NOMEM_BKPT;
  }
  स_नकल(pTo, pFrom, WHERE_LOOP_XFER_SZ);
  स_नकल(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm*आकार(pTo->aLTerm[0]));
  जर( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }अन्यथा जर( (pFrom->wsFlags & WHERE_AUTO_INDEX)!=0 ){
    pFrom->u.btree.pIndex = 0;
  }
  परत SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
स्थिर शून्य whereLoopDelete(sqlite3 *db, WhereLoop *p){
  प्रतिज्ञा( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
स्थिर शून्य whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  प्रतिज्ञा( pWInfo!=0 );
  प्रतिज्ञा( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  पर्यंत( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->pNextLoop;
    whereLoopDelete(db, p);
  }
  पर्यंत( pWInfo->pMemToFree ){
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
स्थिर पूर्णांक whereLoopCheaperProperSubset(
  स्थिरांक WhereLoop *pX,       /* First WhereLoop to compare */
  स्थिरांक WhereLoop *pY        /* Compare against this WhereLoop */
){
  पूर्णांक i, j;
  जर( pX->rRun>pY->rRun && pX->nOut>pY->nOut ) परत 0; /* (1d) and (2a) */
  प्रतिज्ञा( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  प्रतिज्ञा( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  जर( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.pIndex==pY->u.btree.pIndex             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    परत 1;  /* Case 1 is true */
  }
  जर( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    परत 0;                                            /* (2b) */
  }
  जर( pY->nSkip > pX->nSkip ) परत 0;                  /* (2d) */
  साठी(i=pX->nLTerm-1; i>=0; i--){
    जर( pX->aLTerm[i]==0 ) सुरूठेवा;
    साठी(j=pY->nLTerm-1; j>=0; j--){
      जर( pY->aLTerm[j]==pX->aLTerm[i] ) थांबा;
    }
    जर( j<0 ) परत 0;                                  /* (2c) */
  }
  जर( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    परत 0;                                            /* (2e) */
  }
  परत 1;  /* Case 2 is true */
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
स्थिर शून्य whereLoopAdjustCost(स्थिरांक WhereLoop *p, WhereLoop *pTemplate){
  जर( (pTemplate->wsFlags & WHERE_INDEXED)==0 ) परत;
  साठी(; p; p=p->pNextLoop){
    जर( p->iTab!=pTemplate->iTab ) सुरूठेवा;
    जर( (p->wsFlags & WHERE_INDEXED)==0 ) सुरूठेवा;
    जर( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut,
                       MIN(p->rRun, pTemplate->rRun),
                       MIN(p->nOut - 1, pTemplate->nOut)));
      pTemplate->rRun = MIN(p->rRun, pTemplate->rRun);
      pTemplate->nOut = MIN(p->nOut - 1, pTemplate->nOut);
    }अन्यथा जर( whereLoopCheaperProperSubset(pTemplate, p) ){
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
स्थिर WhereLoop **whereLoopFindLesser(
  WhereLoop **ppPrev,
  स्थिरांक WhereLoop *pTemplate
){
  WhereLoop *p;
  साठी(p=(*ppPrev); p; ppPrev=&p->pNextLoop, p=*ppPrev){
    जर( p->iTab!=pTemplate->iTab || p->iSortIdx!=pTemplate->iSortIdx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      सुरूठेवा;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    प्रतिज्ञा( p->rSetup==0 || pTemplate->rSetup==0
                 || p->rSetup==pTemplate->rSetup );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    प्रतिज्ञा( p->rSetup>=pTemplate->rSetup );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    जर( (p->wsFlags & WHERE_AUTO_INDEX)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_INDEXED)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      थांबा;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    जर( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->rSetup<=pTemplate->rSetup                  /* (2a) */
     && p->rRun<=pTemplate->rRun                      /* (2b) */
     && p->nOut<=pTemplate->nOut                      /* (2c) */
    ){
      परत 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    जर( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->rRun>=pTemplate->rRun                             /* (2a) */
     && p->nOut>=pTemplate->nOut                             /* (2b) */
    ){
      प्रतिज्ञा( p->rSetup>=pTemplate->rSetup ); /* SETUP-INVARIANT above */
      थांबा;   /* Cause p to be overwritten by pTemplate */
    }
  }
  परत ppPrev;
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
स्थिर पूर्णांक where_Loop_घाला(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  पूर्णांक rc;

  /* Stop the search once we hit the query planner search limit */
  जर( pBuilder->iPlanLimit==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    जर( pBuilder->pOrSet ) pBuilder->pOrSet->n = 0;
    परत SQLITE_DONE;
  }
  pBuilder->iPlanLimit--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  जर( pBuilder->pOrSet!=0 ){
    जर( pTemplate->nLTerm ){
#जर WHERETRACE_ENABLED
      u16 n = pBuilder->pOrSet->n;
      पूर्णांक x =
#endif
      where_Or_घाला(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun,
                                    pTemplate->nOut);
#जर WHERETRACE_ENABLED /* 0x8 */
      जर( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#endif
    }
    परत SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  जर( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#जर WHERETRACE_ENABLED /* 0x8 */
    जर( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#endif
    परत SQLITE_OK; 
  }अन्यथा{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#जर WHERETRACE_ENABLED /* 0x8 */
  जर( sqlite3WhereTrace & 0x8 ){
    जर( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }अन्यथा{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#endif
  जर( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, आकार(WhereLoop));
    जर( p==0 ) परत SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->pNextLoop = 0;
  }अन्यथा{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **ppTail = &p->pNextLoop;
    WhereLoop *pToDel;
    पर्यंत( *ppTail ){
      ppTail = whereLoopFindLesser(ppTail, pTemplate);
      जर( ppTail==0 ) थांबा;
      pToDel = *ppTail;
      जर( pToDel==0 ) थांबा;
      *ppTail = pToDel->pNextLoop;
#जर WHERETRACE_ENABLED /* 0x8 */
      जर( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(pToDel, pBuilder->pWC);
      }
#endif
      whereLoopDelete(db, pToDel);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  जर( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    Index *pIndex = p->u.btree.pIndex;
    जर( pIndex && pIndex->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.pIndex = 0;
    }
  }
  परत rc;
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
स्थिर पूर्णांक अभिव्यक्ती_गाठ_Pattern_Length_Est(Walker *pWalker, अभिव्यक्ती *p_अभिव्यक्ती){
  जर( p_अभिव्यक्ती->op==TK_STRING ){
    पूर्णांक sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_अभिव्यक्ती->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    जर( pWalker->eCode ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }अन्यथा{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    पर्यंत( (c = *(z++))!=0 ){
      जर( c==c3 ){
        जर( *z ) z++;
        पर्यंत( *z && *z!=']' ) z++;
      }अन्यथा जर( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    जर( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  परत WRC_Continue;
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
स्थिर पूर्णांक estLikePatternLength(अभिव्यक्ती *p, u16 eCode){
  Walker w;
  w.u.sz = 0;
  w.eCode = eCode;
  w.x_अभिव्यक्ती_Callback = अभिव्यक्ती_गाठ_Pattern_Length_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_अभिव्यक्ती(&w, p);
  परत w.u.sz;
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
स्थिर शून्य whereLoopOutputAdjust(
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
  जर( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->pOrderBy==0 ){
    परत;
  }

  प्रतिज्ञा( (pLoop->wsFlags & WHERE_AUTO_INDEX)==0 );
  साठी(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
    प्रतिज्ञा( pTerm!=0 );
    जर( (pTerm->prereqAll & notAllowed)!=0 ) सुरूठेवा;
    जर( (pTerm->prereqAll & pLoop->maskSelf)==0 ) सुरूठेवा;
    जर( (pTerm->wtFlags & TERM_VIRTUAL)!=0 ) सुरूठेवा;
    साठी(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      जर( pX==0 ) सुरूठेवा;
      जर( pX==pTerm ) थांबा;
      जर( pX->iParent>=0 && (&pWC->a[pX->iParent])==pTerm ) थांबा;
    }
    जर( j<0 ){
      sqlite3ProgressCheck(pWC->pWInfo->pParse);
      जर( pLoop->maskSelf==pTerm->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        जर( (pTerm->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_डावा|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      जर( pTerm->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->nOut += pTerm->truthProb;
      }अन्यथा{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        अभिव्यक्ती *p_Op_अभिव्यक्ती = pTerm->p_अभिव्यक्ती;
        pLoop->nOut--;
        जर( (pTerm->eOperator&(WO_EQ|WO_IS))!=0
         && (pTerm->wtFlags & TERM_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          अभिव्यक्ती *p_उजवा = p_Op_अभिव्यक्ती->p_उजवा;
          Parse *pParse = pWC->pWInfo->pParse;
          पूर्णांक k = 0;
          testcase( p_Op_अभिव्यक्ती->op==TK_IS );
          जर( sqlite3_अभिव्यक्ती_Is_Integer(p_उजवा, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }अन्यथा{
            k = 20;
          }
          जर( iReduce<k ){
            pTerm->wtFlags |= TERM_HEURTRUTH;
            iReduce = k;
          }
        }अन्यथा
        जर( अभिव्यक्ती_Has_Property(p_Op_अभिव्यक्ती, EP_InfixFunc) 
         && p_Op_अभिव्यक्ती->op==TK_FUNCTION
        ){
          पूर्णांक eOp;
          प्रतिज्ञा( अभिव्यक्ती_Use_XList(p_Op_अभिव्यक्ती) );
          प्रतिज्ञा( p_Op_अभिव्यक्ती->x.pList->n_अभिव्यक्ती>=2 );
          eOp = sqlite3_अभिव्यक्ती_Is_Like_Operator(p_Op_अभिव्यक्ती);
          जर( ALWAYS(eOp>0) ){
            पूर्णांक szPattern;
            अभिव्यक्ती *pRHS = p_Op_अभिव्यक्ती->x.pList->a[0].p_अभिव्यक्ती;
            eOp = eOp==SQLITE_INDEX_CONSTRAINT_LIKE;
            szPattern = estLikePatternLength(pRHS, eOp);
            जर( szPattern>0 ){
              pLoop->nOut -= szPattern*2;
            }
          }
        }
      }
    }
  }
  जर( pLoop->nOut > nRow-iReduce ){
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
स्थिर पूर्णांक whereRangeVectorLen(
  Parse *pParse,       /* Parsing context */
  पूर्णांक iCur,            /* Cursor open on pIdx */
  Index *pIdx,         /* The index to be used for a inequality constraint */
  पूर्णांक nEq,             /* Number of prior equality constraints on same index */
  WhereTerm *pTerm     /* The vector inequality constraint */
){
  पूर्णांक nCmp = sqlite3_अभिव्यक्ती_Vector_Size(pTerm->p_अभिव्यक्ती->p_डावा);
  पूर्णांक i;

  nCmp = MIN(nCmp, (pIdx->nColumn - nEq));
  साठी(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    अक्षर aff;                     /* Comparison affinity */
    अक्षर idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    अभिव्यक्ती *pLhs, *pRhs;

    प्रतिज्ञा( अभिव्यक्ती_Use_XList(pTerm->p_अभिव्यक्ती->p_डावा) );
    pLhs = pTerm->p_अभिव्यक्ती->p_डावा->x.pList->a[i].p_अभिव्यक्ती;
    pRhs = pTerm->p_अभिव्यक्ती->p_उजवा;
    जर( अभिव्यक्ती_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_अभिव्यक्ती;
    }अन्यथा{
      pRhs = pRhs->x.pList->a[i].p_अभिव्यक्ती;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    जर( pLhs->op!=TK_COLUMN
     || pLhs->iTable!=iCur
     || pLhs->iColumn!=pIdx->aiColumn[i+nEq]
     || pIdx->aSortOrder[i+nEq]!=pIdx->aSortOrder[nEq]
    ){
      थांबा;
    }

    testcase( pLhs->iColumn==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_अभिव्यक्ती_Affinity(pLhs));
    idxaff = sqlite3TableColumnAffinity(pIdx->pTable, pLhs->iColumn);
    जर( aff!=idxaff ) थांबा;

    जर( अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_Commuted) ) SWAP(अभिव्यक्ती*, pRhs, pLhs);
    pColl = sqlite3_द्विमान_Compare_Coll_Seq(pParse, pLhs, pRhs);
    जर( pColl==0 ) थांबा;
    जर( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) थांबा;
  }
  परत i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#ifdef SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#अन्यथा
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
स्थिर पूर्णांक whereLoopAddBtreeIndex(
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
  LogEst rSize;                   /* Number of rows in the table */
  LogEst rLogSize;                /* Logarithm of table size */
  WhereTerm *p_शीर्ष = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  प्रतिज्ञा( db->mallocFailed==0 || pParse->nErr>0 );
  जर( pParse->nErr ){
    परत pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->pTable->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->rRun));

  प्रतिज्ञा( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  प्रतिज्ञा( (pNew->wsFlags & WHERE_शीर्ष_LIMIT)==0 );
  जर( pNew->wsFlags & WHERE_BTM_LIMIT ){
    opMask = WO_LT|WO_LE;
  }अन्यथा{
    प्रतिज्ञा( pNew->u.btree.nBtm==0 );
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  जर( pProbe->bUnordered ){
    opMask &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  प्रतिज्ञा( pNew->u.btree.nEq<pProbe->nColumn );
  प्रतिज्ञा( pNew->u.btree.nEq<pProbe->n_किल्ली_Col
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
  rSize = pProbe->aiRowLogEst[0];
  rLogSize = estLog(rSize);
  साठी(; rc==SQLITE_OK && pTerm!=0; pTerm = whereScanNext(&scan)){
    u16 eOp = pTerm->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst nOutUnadjusted;        /* nOut before IN() and WHERE adjustments */
    पूर्णांक nIn = 0;
#ifdef SQLITE_ENABLE_STAT4
    पूर्णांक nRecValid = pBuilder->nRecValid;
#endif
    जर( (eOp==WO_ISNULL || (pTerm->wtFlags&TERM_VNULL)!=0)
     && indexColumnNotNull(pProbe, saved_nEq)
    ){
      सुरूठेवा; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    जर( pTerm->prereq_उजवा & pNew->maskSelf ) सुरूठेवा;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    जर( pTerm->wtFlags & TERM_LIKEOPT && pTerm->eOperator==WO_LT ) सुरूठेवा;

    जर( (pSrc->fg.jointype & (JT_डावा|JT_LTORJ|JT_उजवा))!=0
     && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
    ){
      सुरूठेवा;
    }
    जर( IsUniqueIndex(pProbe) && saved_nEq==pProbe->n_किल्ली_Col-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }अन्यथा{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_INDEXED;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_शीर्ष = saved_n_शीर्ष;
    pNew->nLTerm = saved_nLTerm;
    जर( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       थांबा; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereq_उजवा) & ~pNew->maskSelf;

    प्रतिज्ञा( nInMul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0
        || (pNew->wsFlags & WHERE_COLUMN_IN)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    जर( eOp & WO_IN ){
      अभिव्यक्ती *p_अभिव्यक्ती = pTerm->p_अभिव्यक्ती;
      जर( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        पूर्णांक i;
        पूर्णांक bRedundant = 0;
        nIn = 46;  प्रतिज्ञा( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        साठी(i=0; i<pNew->nLTerm-1; i++){
          जर( pNew->aLTerm[i] && pNew->aLTerm[i]->p_अभिव्यक्ती==p_अभिव्यक्ती ){
            nIn = 0;
            जर( pNew->aLTerm[i]->u.x.iField == pTerm->u.x.iField ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        जर( bRedundant ){
          pNew->nLTerm--;
          सुरूठेवा;
        }
      }अन्यथा जर( ALWAYS(p_अभिव्यक्ती->x.pList && p_अभिव्यक्ती->x.pList->n_अभिव्यक्ती) ){
        /* "x IN (value, value, ...)" */
        nIn = sqlite3LogEst(p_अभिव्यक्ती->x.pList->n_अभिव्यक्ती);
      }
      जर( pProbe->hasStat1 && rLogSize>=10 ){
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
        जर( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, nIn, rLogSize, x));
        }अन्यथा जर( nInMul<2 && OptimizationEnabled(db, SQLITE_SeekScan) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, nIn, rLogSize, x, nInMul));
          pNew->wsFlags |= WHERE_IN_SEEKSCAN;
        }अन्यथा{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, nIn, rLogSize, x, nInMul));
          सुरूठेवा;
        }
      }
      pNew->wsFlags |= WHERE_COLUMN_IN;
    }अन्यथा जर( eOp & (WO_EQ|WO_IS) ){
      पूर्णांक iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      प्रतिज्ञा( saved_nEq==pNew->u.btree.nEq );
      जर( iCol==XN_ROWID
       || (iCol>=0 && nInMul==0 && saved_nEq==pProbe->n_किल्ली_Col-1)
      ){
        जर( iCol==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_किल्ली_Col==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }अन्यथा{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      जर( scan.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }अन्यथा जर( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }अन्यथा{
      पूर्णांक nVecLen = whereRangeVectorLen(
          pParse, pSrc->iCursor, pProbe, saved_nEq, pTerm
      );
      जर( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_LIMIT;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = pTerm;
        p_शीर्ष = 0;
        जर( pTerm->wtFlags & TERM_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_शीर्ष = &pTerm[1];
          प्रतिज्ञा( (p_शीर्ष-(pTerm->pWC->a))<pTerm->pWC->nTerm );
          प्रतिज्ञा( p_शीर्ष->wtFlags & TERM_LIKEOPT );
          प्रतिज्ञा( p_शीर्ष->eOperator==WO_LT );
          जर( whereLoopResize(db, pNew, pNew->nLTerm+1) ) थांबा; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_शीर्ष;
          pNew->wsFlags |= WHERE_शीर्ष_LIMIT;
          pNew->u.btree.n_शीर्ष = 1;
        }
      }अन्यथा{
        प्रतिज्ञा( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_शीर्ष_LIMIT;
        pNew->u.btree.n_शीर्ष = nVecLen;
        p_शीर्ष = pTerm;
        pBtm = (pNew->wsFlags & WHERE_BTM_LIMIT)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    प्रतिज्ञा( pNew->nOut==saved_nOut );
    जर( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      whereRangeScanEst(pParse, pBuilder, pBtm, p_शीर्ष, pNew);
    }अन्यथा{
      पूर्णांक nEq = ++pNew->u.btree.nEq;
      प्रतिज्ञा( eOp & (WO_ISNULL|WO_EQ|WO_IN|WO_IS) );

      प्रतिज्ञा( pNew->nOut==saved_nOut );
      जर( pTerm->truthProb<=0 && pProbe->aiColumn[saved_nEq]>=0 ){
        प्रतिज्ञा( (eOp & WO_IN) || nIn==0 );
        testcase( eOp & WO_IN );
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      }अन्यथा{
#ifdef SQLITE_ENABLE_STAT4
        tRowcnt nOut = 0;
        जर( nInMul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->nSampleCol)
         && ((eOp & WO_IN)==0 || अभिव्यक्ती_Use_XList(pTerm->p_अभिव्यक्ती))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          अभिव्यक्ती *p_अभिव्यक्ती = pTerm->p_अभिव्यक्ती;
          जर( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = whereEqualScanEst(pParse, pBuilder, p_अभिव्यक्ती->p_उजवा, &nOut);
          }अन्यथा{
            rc = whereInScanEst(pParse, pBuilder, p_अभिव्यक्ती->x.pList, &nOut);
          }
          जर( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          जर( rc!=SQLITE_OK ) थांबा;          /* Jump out of the pTerm loop */
          जर( nOut ){
            pNew->nOut = sqlite3LogEst(nOut);
            जर( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->nOut+10 > pProbe->aiRowLogEst[0]
            ){
#जर WHERETRACE_ENABLED /* 0x01 */
              जर( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3WhereTermPrint(pTerm, 999);
              }
#endif
              pTerm->wtFlags |= TERM_HIGHTRUTH;
              जर( pTerm->wtFlags & TERM_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            जर( pNew->nOut>saved_nOut ) pNew->nOut = saved_nOut;
            pNew->nOut -= nIn;
          }
        }
        जर( nOut==0 )
#endif
        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          जर( eOp & WO_ISNULL ){
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
    प्रतिज्ञा( pSrc->pSTab->szTabRow>0 );
    जर( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      rCostIdx = pNew->nOut + 16;
    }अन्यथा{
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
    जर( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    }
    ApplyCostMultiplier(pNew->rRun, pProbe->pTable->costMult);

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, rSize);
    जर( pSrc->fg.fromExists ) pNew->nOut = 0;
    rc = where_Loop_घाला(pBuilder, pNew);

    जर( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->nOut = saved_nOut;
    }अन्यथा{
      pNew->nOut = nOutUnadjusted;
    }

    जर( (pNew->wsFlags & WHERE_शीर्ष_LIMIT)==0
     && pNew->u.btree.nEq<pProbe->nColumn
     && (pNew->u.btree.nEq<pProbe->n_किल्ली_Col ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      जर( pNew->u.btree.nEq>3 ){
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
  प्रतिज्ञा( 42==sqlite3LogEst(18) );
  जर( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_किल्ली_Col
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
  परत rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
स्थिर पूर्णांक indexMightHelpWithOrderBy(
  WhereLoopBuilder *pBuilder,
  Index *pIndex,
  पूर्णांक iCursor
){
  अभिव्यक्ती_List *pOB;
  अभिव्यक्ती_List *a_Col_अभिव्यक्ती;
  पूर्णांक ii, jj;

  जर( pIndex->bUnordered ) परत 0;
  जर( (pOB = pBuilder->pWInfo->pOrderBy)==0 ) परत 0;
  साठी(ii=0; ii<pOB->n_अभिव्यक्ती; ii++){
    अभिव्यक्ती *p_अभिव्यक्ती = sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(pOB->a[ii].p_अभिव्यक्ती);
    जर( NEVER(p_अभिव्यक्ती==0) ) सुरूठेवा;
    जर( (p_अभिव्यक्ती->op==TK_COLUMN || p_अभिव्यक्ती->op==TK_AGG_COLUMN) 
     && p_अभिव्यक्ती->iTable==iCursor 
    ){
      जर( p_अभिव्यक्ती->iColumn<0 ) परत 1;
      साठी(jj=0; jj<pIndex->n_किल्ली_Col; jj++){
        जर( p_अभिव्यक्ती->iColumn==pIndex->aiColumn[jj] ) परत 1;
      }
    }अन्यथा जर( (a_Col_अभिव्यक्ती = pIndex->a_Col_अभिव्यक्ती)!=0 ){
      साठी(jj=0; jj<pIndex->n_किल्ली_Col; jj++){
        जर( pIndex->aiColumn[jj]!=XN_अभिव्यक्ती ) सुरूठेवा;
        जर( sqlite3_अभिव्यक्ती_Compare_Skip(p_अभिव्यक्ती,a_Col_अभिव्यक्ती->a[jj].p_अभिव्यक्ती,iCursor)==0 ){
          परत 1;
        }
      }
    }
  }
  परत 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
स्थिर पूर्णांक whereUsablePartialIndex(
  पूर्णांक iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  अभिव्यक्ती *pWhere          /* The WHERE clause from the partial index */
){
  पूर्णांक i;
  WhereTerm *pTerm;
  Parse *pParse;

  जर( jointype & JT_LTORJ ) परत 0;
  pParse = pWC->pWInfo->pParse;
  पर्यंत( pWhere->op==TK_AND ){
    जर( !whereUsablePartialIndex(iTab,jointype,pWC,pWhere->p_डावा) ) परत 0;
    pWhere = pWhere->p_उजवा;
  }
  साठी(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    अभिव्यक्ती *p_अभिव्यक्ती;
    p_अभिव्यक्ती = pTerm->p_अभिव्यक्ती;
    जर( (!अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_OuterON) || p_अभिव्यक्ती->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_OuterON))
     && sqlite3_अभिव्यक्ती_Implies_अभिव्यक्ती(pParse, p_अभिव्यक्ती, pWhere, iTab)
     && !sqlite3_अभिव्यक्ती_Implies_अभिव्यक्ती(pParse, p_अभिव्यक्ती, pWhere, -1)
     && (pTerm->wtFlags & TERM_VNULL)==0
    ){
      परत 1;
    }
  }
  परत 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
स्थिर पूर्णांक अभिव्यक्ती_Is_Covered_By_Index(
  स्थिरांक अभिव्यक्ती *p_अभिव्यक्ती,
  स्थिरांक Index *pIdx,
  पूर्णांक iTabCur
){
  पूर्णांक i;
  साठी(i=0; i<pIdx->nColumn; i++){
    जर( pIdx->aiColumn[i]==XN_अभिव्यक्ती
     && sqlite3_अभिव्यक्ती_Compare(0, p_अभिव्यक्ती, pIdx->a_Col_अभिव्यक्ती->a[i].p_अभिव्यक्ती, iTabCur)==0
    ){
      परत 1;
    }
  }
  परत 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
प्रकारव्याख्या संरचना CoveringIndexCheck CoveringIndexCheck;
संरचना CoveringIndexCheck {
  Index *pIdx;       /* The index */
  पूर्णांक iTabCur;       /* Cursor number for the corresponding table */
  u8 b_अभिव्यक्ती;          /* Uses an indexed expression */
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
स्थिर पूर्णांक whereIsCoveringIndexWalkCallback(Walker *pWalk, अभिव्यक्ती *p_अभिव्यक्ती){
  पूर्णांक i;                    /* Loop counter */
  स्थिरांक Index *pIdx;        /* The index of interest */
  स्थिरांक i16 *aiColumn;      /* Columns contained in the index */
  u16 nColumn;              /* Number of columns in the index */
  CoveringIndexCheck *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  जर( (p_अभिव्यक्ती->op==TK_COLUMN || p_अभिव्यक्ती->op==TK_AGG_COLUMN) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    जर( p_अभिव्यक्ती->iTable!=pCk->iTabCur ) परत WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    साठी(i=0; i<nColumn; i++){
      जर( aiColumn[i]==p_अभिव्यक्ती->iColumn ) परत WRC_Continue;
    }
    pCk->bUnidx = 1;
    परत WRC_Abort;
  }अन्यथा जर( pIdx->b_Has_अभिव्यक्ती
         && अभिव्यक्ती_Is_Covered_By_Index(p_अभिव्यक्ती, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_अभिव्यक्ती = 1;
    परत WRC_Prune;
  }
  परत WRC_Continue;
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
स्थिर SQLITE_NOINLINE u32 whereIsCoveringIndex(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  Index *pIdx,           /* Index that is being tested */
  पूर्णांक iTabCur            /* Cursor for the table being indexed */
){
  पूर्णांक i, rc;
  संरचना CoveringIndexCheck ck;
  Walker w;
  जर( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    परत 0;
  }
  जर( pIdx->b_Has_अभिव्यक्ती==0 ){
    साठी(i=0; i<pIdx->nColumn; i++){
      जर( pIdx->aiColumn[i]>=BMS-1 ) थांबा;
    }
    जर( i>=pIdx->nColumn ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      परत 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_अभिव्यक्ती = 0;
  ck.bUnidx = 0;
  स_रखो(&w, 0, आकार(w));
  w.x_अभिव्यक्ती_Callback = whereIsCoveringIndexWalkCallback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  जर( ck.bUnidx ){
    rc = 0;
  }अन्यथा जर( ck.b_अभिव्यक्ती ){
    rc = WHERE_EXPRIDX;
  }अन्यथा{
    rc = WHERE_IDX_ONLY;
  }
  परत rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
स्थिर शून्य where_Indexed_अभिव्यक्ती_Cleanup(sqlite3 *db, शून्य *pObject){
  Indexed_अभिव्यक्ती **pp = (Indexed_अभिव्यक्ती**)pObject;
  पर्यंत( *pp!=0 ){
    Indexed_अभिव्यक्ती *p = *pp;
    *pp = p->pIENext;
    sqlite3_अभिव्यक्ती_Delete(db, p->p_अभिव्यक्ती);
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
स्थिर शून्य where_Part_Idx_अभिव्यक्ती(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Partial index being processed */
  अभिव्यक्ती *pPart,                    /* WHERE clause being processed */
  Bitmask *pMask,                 /* Mask to clear bits in */
  पूर्णांक iIdxCur,                    /* Cursor number for index */
  SrcItem *pItem                  /* The FROM clause entry for the table */
){
  प्रतिज्ञा( pItem==0 || (pItem->fg.jointype & JT_उजवा)==0 );
  प्रतिज्ञा( (pItem==0 || pMask==0) && (pMask!=0 || pItem!=0) );

  जर( pPart->op==TK_AND ){
    where_Part_Idx_अभिव्यक्ती(pParse, pIdx, pPart->p_उजवा, pMask, iIdxCur, pItem);
    pPart = pPart->p_डावा;
  }

  जर( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    अभिव्यक्ती *p_डावा = pPart->p_डावा;
    अभिव्यक्ती *p_उजवा = pPart->p_उजवा;
    Table *p_डावा_Tab;            /* Table of the left operand */
    Column *p_डावा_Col;           /* Column of the left operand */
    u8 aff_उजवा, aff_डावा;

    जर( p_डावा->op!=TK_COLUMN ) परत;
    जर( !sqlite3_अभिव्यक्ती_Is_Constant(0, p_उजवा) ) परत;
    जर( !sqlite3_Is_द्विमान(sqlite3_अभिव्यक्ती_Compare_Coll_Seq(pParse, pPart)) ) परत;
    जर( p_डावा->iColumn<0 ) परत;
    p_डावा_Tab = pIdx->pTable;
    p_डावा_Col = &p_डावा_Tab->aCol[p_डावा->iColumn];
    aff_डावा = p_डावा_Col->affinity;
    प्रतिज्ञा( SQLITE_AFF_BLOB<SQLITE_AFF_TEXT );
    जर( aff_डावा<SQLITE_AFF_TEXT ) परत;
    aff_उजवा = sqlite3_अभिव्यक्ती_Affinity(p_उजवा);
    जर( aff_उजवा!=0 && aff_उजवा!=aff_डावा ) परत;
    जर( pItem ){
      sqlite3 *db = pParse->db;
      Indexed_अभिव्यक्ती *p = (Indexed_अभिव्यक्ती*)sqlite3DbMallocRaw(db, आकार(*p));
      जर( p ){
        पूर्णांक bNullRow = (pItem->fg.jointype&(JT_डावा|JT_LTORJ))!=0;
        p_उजवा = sqlite3_अभिव्यक्ती_Dup(db, p_उजवा, 0);
        जर( (p_डावा_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3ColumnType(p_डावा_Col, 0);
          प्रतिज्ञा( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (पूर्णांक)म_माप(t.z);
          p->p_अभिव्यक्ती = sqlite3_अभिव्यक्ती_Alloc(db, TK_CAST, &t, 1);
          sqlite3_अभिव्यक्ती_Attach_Subtrees(db, p->p_अभिव्यक्ती, p_उजवा, 0);
        }अन्यथा{
          p->p_अभिव्यक्ती = p_उजवा;
        }
        p->iDataCur = pItem->iCursor;
        p->iIdxCur = iIdxCur;
        p->iIdxCol = p_डावा->iColumn;
        p->bMaybeNullRow = bNullRow;
        p->pIENext = pParse->p_Idx_Part_अभिव्यक्ती;
        p->aff = aff_डावा;
        pParse->p_Idx_Part_अभिव्यक्ती = p;
        जर( p->pIENext==0 ){
          शून्य *pArg = (शून्य*)&pParse->p_Idx_Part_अभिव्यक्ती;
          sqlite3ParserAddCleanup(pParse, where_Indexed_अभिव्यक्ती_Cleanup, pArg);
        }
      }
    }अन्यथा जर( p_डावा->iColumn<(BMS-1) ){
      *pMask &= ~((Bitmask)1 << p_डावा->iColumn);
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
स्थिर पूर्णांक whereLoopAddBtree(
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
  पूर्णांक rc = SQLITE_OK;         /* Return code */
  पूर्णांक iSortIdx;               /* Index number */
  पूर्णांक b;                      /* A boolean value */
  LogEst rSize;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  Table *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  प्रतिज्ञा( !IsVirtual(pSrc->pSTab) );

  जर( pSrc->fg.isIndexedBy ){
    प्रतिज्ञा( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }अन्यथा जर( !HasRowid(pTab) ){
    जर( pSrc->fg.notIndexed ){
      pProbe = sqlite3_Primary_किल्ली_Index(pTab);
    }अन्यथा{
      pProbe = pTab->pIndex;
    }
  }अन्यथा{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    Index *pFirst;                  /* First of real indices on the table */
    स_रखो(&sPk, 0, आकार(Index));
    sPk.n_किल्ली_Col = 1;
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
  जर( !pBuilder->pOrSet      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_उजवा_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_AutoIndex)!=0
   && !pSrc->fg.isIndexedBy  /* Has no INDEXED BY clause */
   && !pSrc->fg.notIndexed   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_उजवा)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst rLogSize;         /* Logarithm of the number of rows in the table */
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    rLogSize = estLog(rSize);
    साठी(pTerm=pWC->a; rc==SQLITE_OK && pTerm<pWCEnd; pTerm++){
      जर( pTerm->prereq_उजवा & pNew->maskSelf ) सुरूठेवा;
      जर( termCanDriveIndex(pTerm, pSrc, 0) ){
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
        जर( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->rSetup += 28;
        }अन्यथा{
          pNew->rSetup -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        ApplyCostMultiplier(pNew->rSetup, pTab->costMult);
        जर( pNew->rSetup<0 ) pNew->rSetup = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->nOut = 43;  प्रतिज्ञा( 43==sqlite3LogEst(20) );
        pNew->rRun = sqlite3LogEstAdd(rLogSize,pNew->nOut);
        pNew->wsFlags = WHERE_AUTO_INDEX;
        pNew->prereq = mPrereq | pTerm->prereq_उजवा;
        rc = where_Loop_घाला(pBuilder, pNew);
      }
    }
  }
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  साठी(iSortIdx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.isIndexedBy||pSrc->fg.notIndexed)?0:pProbe->pNext,
         iSortIdx++
  ){
    जर( pProbe->pPartIdxWhere!=0
     && !whereUsablePartialIndex(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      सुरूठेवा;  /* Partial index inappropriate for this query */
    }
    जर( pProbe->bNoQuery ) सुरूठेवा;
    rSize = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_शीर्ष = 0;
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
    प्रतिज्ञा( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    जर( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
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
#अन्यथा
      pNew->rRun = rSize + 16;
#endif
      ApplyCostMultiplier(pNew->rRun, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, rSize);
      जर( pSrc->fg.isSubquery ){
        जर( pSrc->fg.viaCoroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        जर( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.pOrderBy = pSrc->u4.pSubq->pSelect->pOrderBy;
        }
      }अन्यथा जर( pSrc->fg.fromExists ){
        pNew->nOut = 0;
      }
      rc = where_Loop_घाला(pBuilder, pNew);
      pNew->nOut = rSize;
      जर( rc ) थांबा;
    }अन्यथा{
      Bitmask m;
      जर( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
      }अन्यथा{
        m = pSrc->colUsed & pProbe->colNotIdxed;
        जर( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_अभिव्यक्ती(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_INDEXED;
        जर( m==TOPBIT || (pProbe->b_Has_अभिव्यक्ती && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = whereIsCoveringIndex(pWInfo, pProbe, pSrc->iCursor);
          जर( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            प्रतिज्ञा( m!=0 );
          }अन्यथा{
            m = 0;
            pNew->wsFlags |= isCov;
            जर( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }अन्यथा{
              प्रतिज्ञा( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }अन्यथा जर( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
        }
      }

      /* Full scan via index */
      जर( b
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
        जर( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = rSize + 16;  /* Base cost:  N*3 */
          पूर्णांक ii;
          पूर्णांक iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          साठी(ii=0; ii<pWC2->nTerm; ii++){
            WhereTerm *pTerm = &pWC2->a[ii];
            जर( !sqlite3_अभिव्यक्ती_Covered_By_Index(pTerm->p_अभिव्यक्ती, iCur, pProbe) ){
              थांबा;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            जर( pTerm->truthProb<=0 ){
              nLookup += pTerm->truthProb;
            }अन्यथा{
              nLookup--;
              जर( pTerm->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, nLookup);
        }
        ApplyCostMultiplier(pNew->rRun, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, rSize);
        जर( (pSrc->fg.jointype & JT_उजवा)!=0 && pProbe->a_Col_अभिव्यक्ती ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }अन्यथा{
          जर( pSrc->fg.fromExists ) pNew->nOut = 0;
          rc = where_Loop_घाला(pBuilder, pNew);
        }
        pNew->nOut = rSize;
        जर( rc ) थांबा;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, 0);
    जर( pBuilder->bldFlags1==SQLITE_BLDF1_INDEXED ){
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
  परत rc;
}

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
स्थिर पूर्णांक isLimitTerm(WhereTerm *pTerm){
  प्रतिज्ञा( pTerm->eOperator==WO_AUX || pTerm->eMatchOp==0 );
  परत pTerm->eMatchOp>=SQLITE_INDEX_CONSTRAINT_LIMIT
      && pTerm->eMatchOp<=SQLITE_INDEX_CONSTRAINT_OFFSET;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
स्थिर पूर्णांक allConstraintsUsed(
  संरचना sqlite3_index_constraint_usage *aUsage, 
  पूर्णांक nCons
){
  पूर्णांक ii;
  साठी(ii=0; ii<nCons; ii++){
    जर( aUsage[ii].argvIndex<=0 ) परत 0;
  }
  परत 1;
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
स्थिर पूर्णांक whereLoopAddVirtualOne(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_index_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  पूर्णांक *pbIn,                      /* OUT: True if plan uses an IN(...) op */
  पूर्णांक *pbRetryLimit               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  संरचना sqlite3_index_constraint *pIdxCons;
  संरचना sqlite3_index_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  पूर्णांक i;
  पूर्णांक mxTerm;
  पूर्णांक rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  पूर्णांक nConstraint = pIdxInfo->nConstraint;

  प्रतिज्ञा( (mUsable & mPrereq)==mPrereq );
  *pbIn = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(संरचना sqlite3_index_constraint**)&pIdxInfo->aConstraint;
  साठी(i=0; i<nConstraint; i++, pIdxCons++){
    WhereTerm *pTerm = termFromWhereClause(pWC, pIdxCons->iTermOffset);
    pIdxCons->usable = 0;
    जर( (pTerm->prereq_उजवा & mUsable)==pTerm->prereq_उजवा
     && (pTerm->eOperator & mExclude)==0
     && (pbRetryLimit || !isLimitTerm(pTerm))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  स_रखो(pUsage, 0, आकार(pUsage[0])*nConstraint);
  प्रतिज्ञा( pIdxInfo->needToFreeIdxStr==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_अंक = 0;
  pIdxInfo->orderByConsumed = 0;
  pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (द्विगुण)2;
  pIdxInfo->estimatedRows = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->mHandleIn = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtabBestIndex(pParse, pSrc->pSTab, pIdxInfo);
  जर( rc ){
    जर( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      परत SQLITE_OK;
    }
    परत rc;
  }

  mxTerm = -1;
  प्रतिज्ञा( pNew->nLSlot>=nConstraint );
  स_रखो(pNew->aLTerm, 0, आकार(pNew->aLTerm[0])*nConstraint );
  स_रखो(&pNew->u.vtab, 0, आकार(pNew->u.vtab));
  pIdxCons = *(संरचना sqlite3_index_constraint**)&pIdxInfo->aConstraint;
  साठी(i=0; i<nConstraint; i++, pIdxCons++){
    पूर्णांक iTerm;
    जर( (iTerm = pUsage[i].argvIndex - 1)>=0 ){
      WhereTerm *pTerm;
      पूर्णांक j = pIdxCons->iTermOffset;
      जर( iTerm>=nConstraint
       || j<0
       || (pTerm = termFromWhereClause(pWC, j))==0
       || pNew->aLTerm[iTerm]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_संदेश(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        परत SQLITE_ERROR;
      }
      testcase( iTerm==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->nTerm-1 );
      pNew->prereq |= pTerm->prereq_उजवा;
      प्रतिज्ञा( iTerm<pNew->nLSlot );
      pNew->aLTerm[iTerm] = pTerm;
      जर( iTerm>mxTerm ) mxTerm = iTerm;
      testcase( iTerm==15 );
      testcase( iTerm==16 );
      जर( pUsage[i].omit ){
        जर( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=iTerm );
          pNew->u.vtab.omitMask |= 1<<iTerm;
        }अन्यथा{
          testcase( i!=iTerm );
        }
        जर( pTerm->eMatchOp==SQLITE_INDEX_CONSTRAINT_OFFSET ){
          pNew->u.vtab.bOmitOffset = 1;
        }
      }
      जर( SMASKBIT32(i) & pHidden->mHandleIn ){
        pNew->u.vtab.mHandleIn |= MASKBIT32(iTerm);
      }अन्यथा जर( (pTerm->eOperator & WO_IN)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->orderByConsumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_INDEX_SCAN_UNIQUE;
        *pbIn = 1; प्रतिज्ञा( (mExclude & WO_IN)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      प्रतिज्ञा( pbRetryLimit || !isLimitTerm(pTerm) );
      प्रतिज्ञा( !isLimitTerm(pTerm) || i>=nConstraint-2 );
      प्रतिज्ञा( !isLimitTerm(pTerm) || i==nConstraint-1 || isLimitTerm(pTerm+1) );

      जर( isLimitTerm(pTerm) && (*pbIn || !allConstraintsUsed(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pbRetryLimit = 1;
        परत SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mxTerm+1;
  साठी(i=0; i<=mxTerm; i++){
    जर( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_संदेश(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      परत SQLITE_ERROR;
    }
  }
  प्रतिज्ञा( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_अंक = pIdxInfo->idx_अंक;
  pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
  pIdxInfo->needToFreeIdxStr = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ?
      pIdxInfo->nOrderBy : 0);
  pNew->u.vtab.b_Idx_अंक_Hex = (pIdxInfo->idxFlags&SQLITE_INDEX_SCAN_HEX)!=0;
  pNew->rSetup = 0;
  pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  जर( pIdxInfo->idxFlags & SQLITE_INDEX_SCAN_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }अन्यथा{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_घाला(pBuilder, pNew);
  जर( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pbIn, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  परत rc;
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
स्थिरांक अक्षर *sqlite3_vtab_collation(sqlite3_index_info *pIdxInfo, पूर्णांक iCons){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  स्थिरांक अक्षर *zRet = 0;
  जर( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    पूर्णांक iTerm = pIdxInfo->aConstraint[iCons].iTermOffset;
    अभिव्यक्ती *pX = termFromWhereClause(pHidden->pWC, iTerm)->p_अभिव्यक्ती;
    जर( pX->p_डावा ){
      pC = sqlite3_अभिव्यक्ती_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_द्विमान);
  }
  परत zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
पूर्णांक sqlite3_vtab_in(sqlite3_index_info *pIdxInfo, पूर्णांक iCons, पूर्णांक bHandle){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  जर( m & pHidden->mIn ){
    जर( bHandle==0 ){
      pHidden->mHandleIn &= ~m;
    }अन्यथा जर( bHandle>0 ){
      pHidden->mHandleIn |= m;
    }
    परत 1;
  }
  परत 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
पूर्णांक sqlite3_vtab_rhs_मूल्य(
  sqlite3_index_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  पूर्णांक iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_मूल्य **ppVal           /* Write value extracted here */
){
  HiddenIndexInfo *pH = (HiddenIndexInfo*)&pIdxInfo[1];
  sqlite3_मूल्य *pVal = 0;
  पूर्णांक rc = SQLITE_OK;
  जर( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }अन्यथा{
    जर( pH->aRhs[iCons]==0 ){
      WhereTerm *pTerm = termFromWhereClause(
          pH->pWC, pIdxInfo->aConstraint[iCons].iTermOffset
      );
      rc = sqlite3_मूल्य_From_अभिव्यक्ती(
          pH->pParse->db, pTerm->p_अभिव्यक्ती->p_उजवा, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  जर( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  परत rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
पूर्णांक sqlite3_vtab_distinct(sqlite3_index_info *pIdxInfo){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  प्रतिज्ञा( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  परत pHidden->eDistinct;
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
शून्य sqlite3VtabUsesAllSchemas(Parse *pParse){
  पूर्णांक nDb = pParse->db->nDb;
  पूर्णांक i;
  साठी(i=0; i<nDb; i++){
    sqlite3CodeVerifySchema(pParse, i);
  }
  जर( DbMaskNonZero(pParse->writeMask) ){
    साठी(i=0; i<nDb; i++){
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
स्थिर पूर्णांक whereLoopAddVirtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  पूर्णांक rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  SrcItem *pSrc;               /* The FROM clause term to search */
  sqlite3_index_info *p;       /* Object to pass to xBestIndex() */
  पूर्णांक nConstraint;             /* Number of constraints in p */
  पूर्णांक bIn;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask mBest;               /* Tables used by best possible plan */
  u16 mNoOmit;
  पूर्णांक bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  प्रतिज्ञा( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  प्रतिज्ञा( IsVirtual(pSrc->pSTab) );
  p = allocateIndexInfo(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  जर( p==0 ) परत SQLITE_NOMEM_BKPT;
  pNew->rSetup = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  जर( whereLoopResize(pParse->db, pNew, nConstraint) ){
    freeIndexInfo(pParse->db, p);
    परत SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = whereLoopAddVirtualOne(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, &bRetry
  );
  जर( bRetry ){
    प्रतिज्ञा( rc==SQLITE_OK );
    rc = whereLoopAddVirtualOne(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  जर( rc==SQLITE_OK && ((mBest = (pNew->prereq & ~mPrereq))!=0 || bIn) ){
    पूर्णांक seenZero = 0;             /* True if a plan with no prereqs seen */
    पूर्णांक seenZeroNoIN = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask mBestNoIn = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    जर( bIn ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, ALLBITS, WO_IN, p, mNoOmit, &bIn, 0);
      प्रतिज्ञा( bIn==0 );
      mBestNoIn = pNew->prereq & ~mPrereq;
      जर( mBestNoIn==0 ){
        seenZero = 1;
        seenZeroNoIN = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    पर्यंत( rc==SQLITE_OK ){
      पूर्णांक i;
      Bitmask mNext = ALLBITS;
      प्रतिज्ञा( mNext>0 );
      साठी(i=0; i<nConstraint; i++){
        पूर्णांक iTerm = p->aConstraint[i].iTermOffset;
        Bitmask mThis = termFromWhereClause(pWC, iTerm)->prereq_उजवा & ~mPrereq;
        जर( mThis>mPrev && mThis<mNext ) mNext = mThis;
      }
      mPrev = mNext;
      जर( mNext==ALLBITS ) थांबा;
      जर( mNext==mBest || mNext==mBestNoIn ) सुरूठेवा;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)mNext));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mNext|mPrereq, 0, p, mNoOmit, &bIn, 0);
      जर( pNew->prereq==mPrereq ){
        seenZero = 1;
        जर( bIn==0 ) seenZeroNoIN = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    जर( rc==SQLITE_OK && seenZero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &bIn, 0);
      जर( bIn==0 ) seenZeroNoIN = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    जर( rc==SQLITE_OK && seenZeroNoIN==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, WO_IN, p, mNoOmit, &bIn, 0);
    }
  }

  freeIndexInfo(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  परत rc;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
स्थिर पूर्णांक whereLoopAddOr(
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
  WhereClause tempWC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet sSum, sCur;
  SrcItem *pItem;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  स_रखो(&sSum, 0, आकार(sSum));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  जर( pItem->fg.jointype & JT_उजवा ) परत SQLITE_OK;

  साठी(pTerm=pWC->a; pTerm<pWCEnd && rc==SQLITE_OK; pTerm++){
    जर( (pTerm->eOperator & WO_OR)!=0
     && (pTerm->u.pOrInfo->indexable & pNew->maskSelf)!=0
     && (!अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_OuterON)
          || pTerm->p_अभिव्यक्ती->w.iJoin==iCur)
    ){
      WhereClause * स्थिरांक pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm * स्थिरांक pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      पूर्णांक once = 1;
      पूर्णांक i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.pOrSet = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", pTerm));
      साठी(pOrTerm=pOrWC->a; pOrTerm<pOrWCEnd; pOrTerm++){
        जर( (pOrTerm->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        }अन्यथा जर( pOrTerm->डावा_Cursor==iCur ){
          tempWC.pWInfo = pWC->pWInfo;
          tempWC.pOuter = pWC;
          tempWC.op = TK_AND;
          tempWC.nTerm = 1;
          tempWC.nBase = 1;
          tempWC.a = pOrTerm;
          sSubBuild.pWC = &tempWC;
        }अन्यथा{
          सुरूठेवा;
        }
        sCur.n = 0;
#ifdef WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (पूर्णांक)(pOrTerm-pOrWC->a), pTerm, sSubBuild.pWC->nTerm));
        जर( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
        जर( IsVirtual(pItem->pSTab) ){
          rc = whereLoopAddVirtual(&sSubBuild, mPrereq, mUnusable);
        }अन्यथा
#endif
        {
          rc = whereLoopAddBtree(&sSubBuild, mPrereq);
        }
        जर( rc==SQLITE_OK ){
          rc = whereLoopAddOr(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_DONE );
        जर( sCur.n==0 ){
          sSum.n = 0;
          थांबा;
        }अन्यथा जर( once ){
          whereOrMove(&sSum, &sCur);
          once = 0;
        }अन्यथा{
          WhereOrSet sPrev;
          whereOrMove(&sPrev, &sSum);
          sSum.n = 0;
          साठी(i=0; i<sPrev.n; i++){
            साठी(j=0; j<sCur.n; j++){
              where_Or_घाला(&sSum, sPrev.a[i].prereq | sCur.a[j].prereq,
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
      स_रखो(&pNew->u, 0, आकार(pNew->u));
      साठी(i=0; rc==SQLITE_OK && i<sSum.n; i++){
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
        rc = where_Loop_घाला(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", pTerm));
    }
  }
  परत rc;
}

/*
** Add all WhereLoop objects for all tables
*/
स्थिर पूर्णांक whereLoopAddAll(WhereLoopBuilder *pBuilder){
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
  पूर्णांक has_उजवा_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  प्रतिज्ञा( pNew->nLTerm==0 );
  प्रतिज्ञा( pNew->wsFlags==0 );
  प्रतिज्ञा( pNew->nLSlot>=ArraySize(pNew->aLTermSpace) );
  प्रतिज्ञा( pNew->aLTerm!=0 );

  pBuilder->iPlanLimit = SQLITE_QUERY_PLANNER_LIMIT;
  साठी(iTab=0, pItem=pTabList->a; pItem<pEnd; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->iPlanLimit += SQLITE_QUERY_PLANNER_LIMIT_INCR;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    जर( bFirstPastRJ
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
      जर( pItem->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( pItem->fg.jointype & JT_LTORJ );
        testcase( pItem->fg.jointype & JT_CROSS );
        has_उजवा_Cross_Join = 1;
      }
      mPrereq |= mPrior;
      bFirstPastRJ = (pItem->fg.jointype & JT_उजवा)!=0;
    }अन्यथा जर( pItem->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      WhereTerm *pTerm;
      पूर्णांक i;
      साठी(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
        जर( (pNew->maskSelf & pTerm->prereqAll)!=0 ){
          mPrereq |= (pTerm->prereqAll & (pNew->maskSelf-1));
        }
      }
    }अन्यथा जर( !has_उजवा_Cross_Join ){
      mPrereq = 0;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    जर( IsVirtual(pItem->pSTab) ){
      SrcItem *p;
      साठी(p=&pItem[1]; p<pEnd; p++){
        जर( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mPrereq, mUnusable);
    }अन्यथा
#endif /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = whereLoopAddBtree(pBuilder, mPrereq);
    }
    जर( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = whereLoopAddOr(pBuilder, mPrereq, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    जर( rc || db->mallocFailed ){
      जर( rc==SQLITE_DONE ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }अन्यथा{
        थांबा;
      }
    }
  }

  whereLoopClear(db, pNew);
  परत rc;
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
स्थिर SQLITE_NOINLINE पूर्णांक wherePathMatchSubqueryOB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  पूर्णांक iLoop,              /* Which level of the nested loop.  0==outermost */
  पूर्णांक iCur,               /* Cursor used by the this loop */
  अभिव्यक्ती_List *pOrderBy,     /* The ORDER BY clause on the whole query */
  Bitmask *pRevMask,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  पूर्णांक iOB;                /* Index into pOrderBy->a[] */
  पूर्णांक jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  अभिव्यक्ती *pOBExpr;          /* Current term of outer ORDER BY */
  अभिव्यक्ती_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.pOrderBy;
  प्रतिज्ञा( pSubOB!=0 );
  साठी(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  साठी(jSub=0; jSub<pSubOB->n_अभिव्यक्ती && iOB<pOrderBy->n_अभिव्यक्ती; jSub++, iOB++){
    जर( pSubOB->a[jSub].u.x.iOrderByCol==0 ) थांबा;
    pOBExpr = pOrderBy->a[iOB].p_अभिव्यक्ती;
    जर( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) थांबा;
    जर( pOBExpr->iTable!=iCur ) थांबा;
    जर( pOBExpr->iColumn!=pSubOB->a[jSub].u.x.iOrderByCol-1 ) थांबा;
    जर( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = pOrderBy->a[iOB].fg.sortFlags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.sortFlags;   /* sortFlags for jSub */
      जर( (sfSub & KEYINFO_ORDER_BIGNULL) != (sfOB & KEYINFO_ORDER_BIGNULL) ){
        थांबा;
      }
      revIdx = sfSub & KEYINFO_ORDER_DESC;
      जर( jSub>0 ){
        जर( (rev^revIdx)!=(sfOB & KEYINFO_ORDER_DESC) ){
          थांबा;
        }
      }अन्यथा{
        rev = revIdx ^ (sfOB & KEYINFO_ORDER_DESC);
        जर( rev ){
          जर( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            थांबा;
          }
          *pRevMask |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  परत jSub>0;
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
स्थिर i8 wherePathSatisfiesOrderBy(
  WhereInfo *pWInfo,    /* The WHERE clause */
  अभिव्यक्ती_List *pOrderBy,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
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
  u16 n_किल्ली_Col;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 nOrderBy;         /* Number terms in the ORDER BY clause */
  पूर्णांक iLoop;            /* Index of WhereLoop in pPath being processed */
  पूर्णांक i, j;             /* Loop counters */
  पूर्णांक iCur;             /* Cursor number for current WhereLoop */
  पूर्णांक iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  WhereTerm *pTerm;     /* A single term of the WHERE clause */
  अभिव्यक्ती *pOBExpr;        /* An expression from the ORDER BY clause */
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

  प्रतिज्ञा( pOrderBy!=0 );
  जर( nLoop && OptimizationDisabled(db, SQLITE_OrderByIdxJoin) ) परत 0;

  nOrderBy = pOrderBy->n_अभिव्यक्ती;
  testcase( nOrderBy==BMS-1 );
  जर( nOrderBy>BMS-1 ) परत 0;  /* Cannot optimize overly large ORDER BYs */
  isOrderDistinct = 1;
  obDone = MASKBIT(nOrderBy)-1;
  orderDistinctMask = 0;
  ready = 0;
  eqOpMask = WO_EQ | WO_IS | WO_ISNULL;
  जर( wctrlFlags & (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eqOpMask |= WO_IN;
  }
  साठी(iLoop=0; isOrderDistinct && obSat<obDone && iLoop<=nLoop; iLoop++){
    जर( iLoop>0 ) ready |= pLoop->maskSelf;
    जर( iLoop<nLoop ){
      pLoop = pPath->aLoop[iLoop];
      जर( wctrlFlags & WHERE_ORDERBY_LIMIT ) सुरूठेवा;
    }अन्यथा{
      pLoop = pLast;
    }
    जर( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      जर( pLoop->u.vtab.isOrdered && pWInfo->pOrderBy==pOrderBy ){
        obSat = obDone;
      }अन्यथा{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        isOrderDistinct = 0;
      }
      थांबा;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    साठी(i=0; i<nOrderBy; i++){
      जर( MASKBIT(i) & obSat ) सुरूठेवा;
      pOBExpr = sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(pOrderBy->a[i].p_अभिव्यक्ती);
      जर( NEVER(pOBExpr==0) ) सुरूठेवा;
      जर( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) सुरूठेवा;
      जर( pOBExpr->iTable!=iCur ) सुरूठेवा;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, eqOpMask, 0);
      जर( pTerm==0 ) सुरूठेवा;
      जर( pTerm->eOperator==WO_IN ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        प्रतिज्ञा( wctrlFlags &
               (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        साठी(j=0; j<pLoop->nLTerm && pTerm!=pLoop->aLTerm[j]; j++){}
        जर( j>=pLoop->nLTerm ) सुरूठेवा;
      }
      जर( (pTerm->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_अभिव्यक्ती_NNColl_Seq(pParse, pOrderBy->a[i].p_अभिव्यक्ती);
        स्थिरांक अभिव्यक्ती *pCExpr = pTerm->p_अभिव्यक्ती;
        CollSeq *pColl2 = sqlite3_अभिव्यक्ती_Compare_Coll_Seq(pParse, pCExpr);
        अक्षर aff_उजवा;
        प्रतिज्ञा( pColl1 );
        जर( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          सुरूठेवा;
        }
        aff_उजवा = sqlite3_अभिव्यक्ती_Affinity(pCExpr->p_उजवा);
        /* Left operand must be a column, hence always has affinity */
        प्रतिज्ञा( sqlite3_अभिव्यक्ती_Affinity(pCExpr->p_डावा)!=0 );
        जर( aff_उजवा!=0 && aff_उजवा!=sqlite3_अभिव्यक्ती_Affinity(pCExpr->p_डावा) ){
          सुरूठेवा;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    जर( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      जर( pLoop->wsFlags & WHERE_IPK ){
        जर( pLoop->u.btree.pOrderBy
         && OptimizationEnabled(db, SQLITE_OrderBySubq)
         &&  wherePathMatchSubqueryOB(pLoop,iLoop,iCur,
                                     pOrderBy,pRevMask, &obSat, wctrlFlags)
        ){
          nColumn = 0;
          isOrderDistinct = 0;
        }अन्यथा{
          nColumn = 1;
        }
        pIndex = 0;
        n_किल्ली_Col = 1;
      }अन्यथा जर( (pIndex = pLoop->u.btree.pIndex)==0 || pIndex->bUnordered ){
        परत 0;
      }अन्यथा{
        n_किल्ली_Col = pIndex->n_किल्ली_Col;
        nColumn = pIndex->nColumn;
        प्रतिज्ञा( nColumn==n_किल्ली_Col+1 || !HasRowid(pIndex->pTable) );
        प्रतिज्ञा( pIndex->aiColumn[nColumn-1]==XN_ROWID
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
      साठी(j=0; j<nColumn; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        प्रतिज्ञा( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        जर( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          जर( (eOp & eqOpMask)!=0 ){
            जर( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( isOrderDistinct );
              isOrderDistinct = 0;
            }
            सुरूठेवा; 
          }अन्यथा जर( ALWAYS(eOp & WO_IN) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            अभिव्यक्ती *pX = pLoop->aLTerm[j]->p_अभिव्यक्ती;
            साठी(i=j+1; i<pLoop->u.btree.nEq; i++){
              जर( pLoop->aLTerm[i]->p_अभिव्यक्ती==pX ){
                प्रतिज्ञा( (pLoop->aLTerm[i]->eOperator & WO_IN) );
                bOnce = 0;
                थांबा;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        जर( pIndex ){
          iColumn = pIndex->aiColumn[j];
          revIdx = pIndex->aSortOrder[j] & KEYINFO_ORDER_DESC;
          जर( iColumn==pIndex->pTable->iPKey ) iColumn = XN_ROWID;
        }अन्यथा{
          iColumn = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        जर( isOrderDistinct ){
          जर( iColumn>=0
           && j>=pLoop->u.btree.nEq
           && pIndex->pTable->aCol[iColumn].notNull==0
          ){
            isOrderDistinct = 0;
          }
          जर( iColumn==XN_अभिव्यक्ती ){
            isOrderDistinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        isMatch = 0;
        साठी(i=0; bOnce && i<nOrderBy; i++){
          जर( MASKBIT(i) & obSat ) सुरूठेवा;
          pOBExpr = sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(pOrderBy->a[i].p_अभिव्यक्ती);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          जर( NEVER(pOBExpr==0) ) सुरूठेवा;
          जर( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          जर( iColumn>=XN_ROWID ){
            जर( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) सुरूठेवा;
            जर( pOBExpr->iTable!=iCur ) सुरूठेवा;
            जर( pOBExpr->iColumn!=iColumn ) सुरूठेवा;
          }अन्यथा{
            अभिव्यक्ती *p_Ix_अभिव्यक्ती = pIndex->a_Col_अभिव्यक्ती->a[j].p_अभिव्यक्ती;
            जर( sqlite3_अभिव्यक्ती_Compare_Skip(pOBExpr, p_Ix_अभिव्यक्ती, iCur) ){
              सुरूठेवा;
            }
          }
          जर( iColumn!=XN_ROWID ){
            pColl = sqlite3_अभिव्यक्ती_NNColl_Seq(pWInfo->pParse, pOrderBy->a[i].p_अभिव्यक्ती);
            जर( sqlite3StrICmp(pColl->zName, pIndex->azColl[j])!=0 ) सुरूठेवा;
          }
          जर( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.nDistinctCol = j+1;
          }
          isMatch = 1;
          थांबा;
        }
        जर( isMatch && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          जर( revSet ){
            जर( (rev ^ revIdx)
                           != (pOrderBy->a[i].fg.sortFlags&KEYINFO_ORDER_DESC)
            ){
              isMatch = 0;
            }
          }अन्यथा{
            rev = revIdx ^ (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC);
            जर( rev ) *pRevMask |= MASKBIT(iLoop);
            revSet = 1;
          }
        }
        जर( isMatch && (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL) ){
          जर( j==pLoop->u.btree.nEq 
           && (j<n_किल्ली_Col||HasRowid(pIndex->pTable)||!Is_Primary_किल्ली_Index(pIndex))
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
        जर( isMatch ){
          जर( iColumn==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }अन्यथा{
          /* No match found */
          प्रतिज्ञा( n_किल्ली_Col>0 );
          जर( j<n_किल्ली_Col ){
            testcase( isOrderDistinct!=0 );
            isOrderDistinct = 0;
          }
          थांबा;
        }
      } /* end Loop over all index columns */
      जर( distinctColumns ){
        testcase( isOrderDistinct==0 );
        isOrderDistinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    जर( isOrderDistinct ){
      orderDistinctMask |= pLoop->maskSelf;
      साठी(i=0; i<nOrderBy; i++){
        अभिव्यक्ती *p;
        Bitmask mTerm;
        जर( MASKBIT(i) & obSat ) सुरूठेवा;
        p = pOrderBy->a[i].p_अभिव्यक्ती;
        mTerm = sqlite3_Where_अभिव्यक्ती_Usage(&pWInfo->sMaskSet,p);
        जर( mTerm==0 && !sqlite3_अभिव्यक्ती_Is_Constant(0,p) ) सुरूठेवा;
        जर( (mTerm&~orderDistinctMask)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  जर( obSat==obDone ) परत (i8)nOrderBy;
  जर( !isOrderDistinct ){
    साठी(i=nOrderBy-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      जर( (obSat&m)==m ) परत i;
    }
    परत 0;
  }
  परत -1;
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
  प्रतिज्ञा( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  प्रतिज्ञा( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  परत pWInfo->sorted;
}

#ifdef WHERETRACE_ENABLED
/* For debugging use only: */
स्थिर स्थिरांक अक्षर *wherePathName(WherePath *pPath, पूर्णांक nLoop, WhereLoop *pLast){
  स्थिर अक्षर zName[65];
  पूर्णांक i;
  साठी(i=0; i<nLoop; i++){ zName[i] = pPath->aLoop[i]->cId; }
  जर( pLast ) zName[i++] = pLast->cId;
  zName[i] = 0;
  परत zName;
}
#endif

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
स्थिर LogEst whereSortingCost(
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
  प्रतिज्ञा( pWInfo->pSelect!=0 );
  प्रतिज्ञा( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->n_अभिव्यक्ती+59)/30);
  rSortCost = nRow + nCol;
  जर( nSorted>0 ){
    /* Scale the result by (Y/X) */
    rSortCost += sqlite3LogEst((nOrderBy-nSorted)*100/nOrderBy) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  जर( (pWInfo->wctrlFlags & WHERE_USE_LIMIT)!=0 ){
    rSortCost += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    जर( nSorted!=0 ){
      rSortCost += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    जर( pWInfo->iLimit<nRow ){
      nRow = pWInfo->iLimit;
    }
  }अन्यथा जर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    जर( nRow>10 ){ nRow -= 10;  प्रतिज्ञा( 10==sqlite3LogEst(2) ); }
  }
  rSortCost += estLog(nRow);
  परत rSortCost;
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
स्थिर पूर्णांक computeMxChoice(WhereInfo *pWInfo){
  पूर्णांक nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#ifdef SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  साठी(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
    प्रतिज्ञा( pWLoop->maskSelf==MASKBIT(pWLoop->iTab) );
    प्रतिज्ञा( pWLoop->pNextLoop==0 || pWLoop->iTab<=pWLoop->pNextLoop->iTab );
  }
#endif /* SQLITE_DEBUG */

  जर( nLoop>=4  /* Constraint (aa) */
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
    प्रतिज्ञा( !pWInfo->bStarUsed );
    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    साठी(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      पूर्णांक nDep = 0;             /* Number of dimension tables */
      LogEst mxRun;             /* Maximum SCAN cost of a fact table */
      Bitmask mSeen = 0;        /* Mask of dimension tables */
      SrcItem *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      जर( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        जर( iFromIdx+3 > nLoop ){
          थांबा;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        पर्यंत( pStart && pStart->iTab<=iFromIdx ){
          pStart = pStart->pNextLoop;
        }
      }
      साठी(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        जर( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          थांबा; /* Constraint (bb) */
        }
        जर( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->maskSelf & mSeen)==0  /* pWInfo not already a dependency */
         && (pWLoop->maskSelf & mSelfJoin)==0 /* Not a self-join */
        ){
          जर( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }अन्यथा{
            nDep++;
            mSeen |= pWLoop->maskSelf;
          }
        }
      }
      जर( nDep<=2 ){
        सुरूठेवा; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#ifdef WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      जर( !pWInfo->bStarUsed ){
        साठी(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
          pWLoop->rStarDelta = 0;
        }
      }
#endif
#ifdef WHERETRACE_ENABLED /* 0x80000 */
      जर( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = mSeen;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        साठी(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
          जर( mShow & pWLoop->maskSelf ){
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
      साठी(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        जर( pWLoop->iTab<iFromIdx ) सुरूठेवा;
        जर( pWLoop->iTab>iFromIdx ) थांबा;
        जर( pWLoop->rRun>mxRun ) mxRun = pWLoop->rRun;
      }
      जर( ALWAYS(mxRun<LOGEST_MAX) ) mxRun++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      साठी(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        जर( (pWLoop->maskSelf & mSeen)==0 ) सुरूठेवा;
        जर( pWLoop->nLTerm ) सुरूठेवा;
        जर( pWLoop->rRun<mxRun ){
#ifdef WHERETRACE_ENABLED /* 0x80000 */
          जर( sqlite3WhereTrace & 0x80000 ){
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
    जर( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->bStarUsed ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      साठी(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        जर( pWLoop->rStarDelta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#endif
  }
  परत pWInfo->bStarUsed ? 18 : 12;
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
स्थिर SQLITE_NOINLINE पूर्णांक whereLoopIsNoBetter(
  स्थिरांक WhereLoop *pCandidate,
  स्थिरांक WhereLoop *pBaseline
){
  जर( (pCandidate->wsFlags & WHERE_INDEXED)==0 ) परत 1;
  जर( (pBaseline->wsFlags & WHERE_INDEXED)==0 ) परत 1;
  जर( pCandidate->u.btree.pIndex->szIdxRow <
        pBaseline->u.btree.pIndex->szIdxRow ) परत 0;
  परत 1;
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
स्थिर पूर्णांक wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst){
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
  जर( nLoop<=1 ){
    mxChoice = 1;
  }अन्यथा जर( nLoop==2 ){
    mxChoice = 5;
  }अन्यथा जर( pParse->nErr ){
    mxChoice = 1;
  }अन्यथा{
    mxChoice = computeMxChoice(pWInfo);
  }
  प्रतिज्ञा( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  जर( pWInfo->pOrderBy==0 || nRowEst==0 ){
    nOrderBy = 0;
  }अन्यथा{
    nOrderBy = pWInfo->pOrderBy->n_अभिव्यक्ती;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (आकार(WherePath)+आकार(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += आकार(LogEst) * nOrderBy;
  pSpace = sqlite3_चळत_Alloc_Raw_NN(pParse->db, nSpace);
  जर( pSpace==0 ) परत SQLITE_NOMEM_BKPT;
  aTo = (WherePath*)pSpace;
  aFrom = aTo+mxChoice;
  स_रखो(aFrom, 0, आकार(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  साठी(ii=mxChoice*2, pFrom=aTo; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  जर( nOrderBy ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    aSortCost = (LogEst*)pX;
    स_रखो(aSortCost, 0, आकार(LogEst) * nOrderBy);
  }
  प्रतिज्ञा( aSortCost==0 || &pSpace[nSpace]==(अक्षर*)&aSortCost[nOrderBy] );
  प्रतिज्ञा( aSortCost!=0 || &pSpace[nSpace]==(अक्षर*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  प्रतिज्ञा( 48==sqlite3LogEst(28) );
  nFrom = 1;
  प्रतिज्ञा( aFrom[0].isOrdered==0 );
  जर( nOrderBy ){
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
  साठी(iLoop=0; iLoop<nLoop; iLoop++){
    nTo = 0;
    साठी(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      साठी(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        LogEst nOut;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask maskNew;                  /* Mask of src visited by (..) */
        Bitmask revMask;                  /* Mask of rev-order loops for (..) */

        जर( (pWLoop->prereq & ~pFrom->maskLoop)!=0 ) सुरूठेवा;
        जर( (pWLoop->maskSelf & pFrom->maskLoop)!=0 ) सुरूठेवा;
        जर( (pWLoop->wsFlags & WHERE_AUTO_INDEX)!=0 && pFrom->nRow<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          प्रतिज्ञा( 10==sqlite3LogEst(2) );
          सुरूठेवा;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->rRun + pFrom->nRow;
        जर( pWLoop->rSetup ){
          rUnsort = sqlite3LogEstAdd(pWLoop->rSetup, rUnsort);
        }
        rUnsort = sqlite3LogEstAdd(rUnsort, pFrom->rUnsort);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        isOrdered = pFrom->isOrdered;
        जर( isOrdered<0 ){
          revMask = 0;
          isOrdered = wherePathSatisfiesOrderBy(pWInfo,
                       pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &revMask);
        }अन्यथा{
          revMask = pFrom->revLoop;
        }
        जर( isOrdered>=0 && isOrdered<nOrderBy ){
          जर( aSortCost[isOrdered]==0 ){
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
        साठी(jj=0, pTo=aTo; jj<nTo; jj++, pTo++){
          जर( pTo->maskLoop==maskNew
           && ( ((pTo->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==nTo-1 );
            थांबा;
          }
        }
        जर( jj>=nTo ){
          /* None of the existing best-so-far paths match the candidate. */
          जर( nTo>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
            जर( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#endif
            सुरूठेवा;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          जर( nTo<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = nTo++;
          }अन्यथा{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          pTo = &aTo[jj];
#ifdef WHERETRACE_ENABLED /* 0x4 */
          जर( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#endif
        }अन्यथा{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          जर( (pTo->rCost<rCost)
           || (pTo->rCost==rCost && pTo->nRow<nOut)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort<rUnsort)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, pTo->aLoop[iLoop]) )
          ){
#ifdef WHERETRACE_ENABLED /* 0x4 */
            जर( sqlite3WhereTrace&0x4 ){
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
            सुरूठेवा;
          }
          testcase( pTo->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
          जर( sqlite3WhereTrace&0x4 ){
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
        स_नकल(pTo->aLoop, pFrom->aLoop, आकार(WhereLoop*)*iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        जर( nTo>=mxChoice ){
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsort = aTo[0].nRow;
          साठी(jj=1, pTo=&aTo[1]; jj<mxChoice; jj++, pTo++){
            जर( pTo->rCost>mxCost
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
    जर( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      पूर्णांक nDone = 0;
      पूर्णांक nProgress;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      करा{
        nProgress = 0;
        rMin = 0x7fff;
        साठी(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          जर( pTo->rCost>rFloor && pTo->rCost<rMin ) rMin = pTo->rCost;
        }
        साठी(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          जर( pTo->rCost==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
               pTo->isOrdered>=0 ? (pTo->isOrdered+'0') : '?');
            जर( pTo->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", pTo->revLoop);
            }अन्यथा{
              sqlite3DebugPrintf("\n");
            }
            nDone++;
            nProgress++;
          }
        }
        rFloor = rMin;
      }पर्यंत( nDone<nTo && nProgress>0 );
    }
#endif

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  जर( nFrom==0 ){
    sqlite3_Error_संदेश(pParse, "no query solution");
    sqlite3_चळत_Free_NN(pParse->db, pSpace);
    परत SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  प्रतिज्ञा( nFrom==1 );
  pFrom = aFrom;

  प्रतिज्ञा( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  साठी(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  जर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask notUsed;
    पूर्णांक rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pResultSet, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &notUsed);
    जर( rc==pWInfo->pResultSet->n_अभिव्यक्ती ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  जर( pWInfo->pOrderBy ){
    pWInfo->nOBSat = pFrom->isOrdered;
    जर( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      जर( pFrom->isOrdered==pWInfo->pOrderBy->n_अभिव्यक्ती ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      प्रतिज्ञा( pWInfo->pSelect->pOrderBy==0
           || pWInfo->nOBSat <= pWInfo->pSelect->pOrderBy->n_अभिव्यक्ती );
    }अन्यथा{
      pWInfo->revMask = pFrom->revLoop;
      जर( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        जर( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          जर( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_COLUMN_IN))!=(WHERE_IPK|WHERE_COLUMN_IN)
          ){
            Bitmask m = 0;
            पूर्णांक rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom,
                      WHERE_ORDERBY_LIMIT, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_COLUMN_IN );
            जर( rc==pWInfo->pOrderBy->n_अभिव्यक्ती ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->revMask = m;
            }
          }
        }
      }अन्यथा जर( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    जर( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->pOrderBy->n_अभिव्यक्ती && nLoop>0
    ){
      Bitmask revMask = 0;
      पूर्णांक nOrder = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &revMask
      );
      प्रतिज्ञा( pWInfo->sorted==0 );
      जर( nOrder==pWInfo->pOrderBy->n_अभिव्यक्ती ){
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
  sqlite3_चळत_Free_NN(pParse->db, pSpace);
  परत SQLITE_OK;
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
स्थिर SQLITE_NOINLINE शून्य whereInterstageHeuristic(WhereInfo *pWInfo){
  पूर्णांक i;
#ifdef WHERETRACE_ENABLED
  पूर्णांक once = 0;
#endif
  साठी(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    जर( p==0 ) थांबा;
    जर( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      थांबा;
    }
    जर( (p->wsFlags & (WHERE_COLUMN_EQ|WHERE_COLUMN_NULL|WHERE_COLUMN_IN))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      साठी(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->pNextLoop){
        जर( pLoop->iTab!=iTab ) सुरूठेवा;
        जर( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_INDEX))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          सुरूठेवा;
        }
#ifdef WHERETRACE_ENABLED
        जर( sqlite3WhereTrace & 0x80 ){
          जर( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#endif /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }अन्यथा{
      थांबा;
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
स्थिर पूर्णांक whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  पूर्णांक iCur;
  पूर्णांक j;
  Table *pTab;
  Index *pIdx;
  WhereScan scan;

  pWInfo = pBuilder->pWInfo;
  जर( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) परत 0;
  प्रतिज्ञा( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  जर( IsVirtual(pTab) ) परत 0;
  जर( pItem->fg.isIndexedBy || pItem->fg.notIndexed ){
    testcase( pItem->fg.isIndexedBy );
    testcase( pItem->fg.notIndexed );
    परत 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = whereScanInit(&scan, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  पर्यंत( pTerm && pTerm->prereq_उजवा ) pTerm = whereScanNext(&scan);
  जर( pTerm ){
    testcase( pTerm->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->rRun = 33;  /* 33==sqlite3LogEst(10) */
  }अन्यथा{
    साठी(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      पूर्णांक opMask;
      प्रतिज्ञा( pLoop->aLTermSpace==pLoop->aLTerm );
      जर( !IsUniqueIndex(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_किल्ली_Col>ArraySize(pLoop->aLTermSpace)
      ) सुरूठेवा;
      opMask = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      साठी(j=0; j<pIdx->n_किल्ली_Col; j++){
        pTerm = whereScanInit(&scan, pWC, iCur, j, opMask, pIdx);
        पर्यंत( pTerm && pTerm->prereq_उजवा ) pTerm = whereScanNext(&scan);
        जर( pTerm==0 ) थांबा;
        testcase( pTerm->eOperator & WO_IS );
        pLoop->aLTerm[j] = pTerm;
      }
      जर( j!=pIdx->n_किल्ली_Col ) सुरूठेवा;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_INDEXED;
      जर( pIdx->isCovering || (pItem->colUsed & pIdx->colNotIdxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.pIndex = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->rRun = 39;  /* 39==sqlite3LogEst(15) */
      थांबा;
    }
  }
  जर( pLoop->wsFlags ){
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    प्रतिज्ञा( pWInfo->sMaskSet.n==1 && iCur==pWInfo->sMaskSet.ix[0] );
    pLoop->maskSelf = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    जर( pWInfo->pOrderBy ) pWInfo->nOBSat =  pWInfo->pOrderBy->n_अभिव्यक्ती;
    जर( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    जर( scan.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#ifdef SQLITE_DEBUG
    pLoop->cId = '0';
#endif
#ifdef WHERETRACE_ENABLED
    जर( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#endif
    परत 1;
  }
  परत 0;
}

/*
** Helper function for exprIsDeterministic().
*/
स्थिर पूर्णांक अभिव्यक्ती_गाठ_Is_Deterministic(Walker *pWalker, अभिव्यक्ती *p_अभिव्यक्ती){
  जर( p_अभिव्यक्ती->op==TK_FUNCTION && अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_ConstFunc)==0 ){
    pWalker->eCode = 0;
    परत WRC_Abort;
  }
  परत WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
स्थिर पूर्णांक अभिव्यक्ती_Is_Deterministic(अभिव्यक्ती *p){
  Walker w;
  स_रखो(&w, 0, आकार(w));
  w.eCode = 1;
  w.x_अभिव्यक्ती_Callback = अभिव्यक्ती_गाठ_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_अभिव्यक्ती(&w, p);
  परत w.eCode;
}

 
#ifdef WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
स्थिर शून्य showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  जर( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    पूर्णांक i;
    स्थिर स्थिरांक अक्षर zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    साठी(p=pWInfo->pLoops, i=0; p; p=p->pNextLoop, i++){
      p->cId = zLabel[i%(आकार(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#अन्यथा
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
स्थिर SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  पूर्णांक i;
  Bitmask tabUsed;
  पूर्णांक has_उजवा_Join;

  /* Preconditions checked by the caller */
  प्रतिज्ञा( pWInfo->nLevel>=2 );
  प्रतिज्ञा( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  प्रतिज्ञा( pWInfo->pResultSet!=0 );
  प्रतिज्ञा( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tabUsed = sqlite3_Where_अभिव्यक्ती_List_Usage(&pWInfo->sMaskSet, pWInfo->pResultSet);
  जर( pWInfo->pOrderBy ){
    tabUsed |= sqlite3_Where_अभिव्यक्ती_List_Usage(&pWInfo->sMaskSet, pWInfo->pOrderBy);
  }
  has_उजवा_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  साठी(i=pWInfo->nLevel-1; i>=1; i--){
    WhereTerm *pTerm, *pEnd;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    जर( (pItem->fg.jointype & (JT_डावा|JT_उजवा))!=JT_डावा ) सुरूठेवा;
    जर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      सुरूठेवा;
    }
    जर( (tabUsed & pLoop->maskSelf)!=0 ) सुरूठेवा;
    pEnd = pWInfo->sWC.a + pWInfo->sWC.nTerm;
    साठी(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      जर( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        जर( !अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_OuterON)
         || pTerm->p_अभिव्यक्ती->w.iJoin!=pItem->iCursor
        ){
          थांबा;
        }
      }
      जर( has_उजवा_Join
       && अभिव्यक्ती_Has_Property(pTerm->p_अभिव्यक्ती, EP_InnerON)
       && NEVER(pTerm->p_अभिव्यक्ती->w.iJoin==pItem->iCursor)
      ){
        थांबा;  /* restriction (5) */
      }
    }
    जर( pTerm<pEnd ) सुरूठेवा;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->revMask>>1) & ~m1)!=0 );
    pWInfo->revMask = (m1 & pWInfo->revMask) | ((pWInfo->revMask>>1) & ~m1);
    notReady &= ~pLoop->maskSelf;
    साठी(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      जर( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        pTerm->wtFlags |= TERM_CODED;
        pTerm->prereqAll = 0;
      }
    }
    जर( i!=pWInfo->nLevel-1 ){
      पूर्णांक nByte = (pWInfo->nLevel-1-i) * आकार(WhereLevel);
      स_हटाओ(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    प्रतिज्ञा( pWInfo->nLevel>0 );
  }
  परत notReady;
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
स्थिर SQLITE_NOINLINE शून्य whereCheckIfBloomFilterIsUseful(
  स्थिरांक WhereInfo *pWInfo
){
  पूर्णांक i;
  LogEst n_शोध = 0;

  प्रतिज्ञा( pWInfo->nLevel>=2 );
  प्रतिज्ञा( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  साठी(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    स्थिरांक अचिन्हित पूर्णांक reqFlags = (WHERE_SELFCULL|WHERE_COLUMN_EQ);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    Table *pTab = pItem->pSTab;
    जर( (pTab->tabFlags & TF_HasStat1)==0 ) थांबा;
    pTab->tabFlags |= TF_MaybeReanalyze;
    जर( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_शोध > pTab->nRowLogEst
    ){
      testcase( pItem->fg.jointype & JT_डावा );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (द्विगुण)sqlite3LogEstToInt(n_शोध), pTab->zName,
         (द्विगुण)sqlite3LogEstToInt(pTab->nRowLogEst)));
    }
    n_शोध += pLoop->nOut;
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
स्थिर SQLITE_NOINLINE शून्य where_Add_Indexed_अभिव्यक्ती(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  Index *pIdx,       /* The index-on-expression that contains the expressions */
  पूर्णांक iIdxCur,       /* Cursor number for pIdx */
  SrcItem *pTabItem  /* The FROM clause entry for the table */
){
  पूर्णांक i;
  Indexed_अभिव्यक्ती *p;
  Table *pTab;
  प्रतिज्ञा( pIdx->b_Has_अभिव्यक्ती );
  pTab = pIdx->pTable;
  साठी(i=0; i<pIdx->nColumn; i++){
    अभिव्यक्ती *p_अभिव्यक्ती;
    पूर्णांक j = pIdx->aiColumn[i];
    जर( j==XN_अभिव्यक्ती ){
      p_अभिव्यक्ती = pIdx->a_Col_अभिव्यक्ती->a[i].p_अभिव्यक्ती;
    }अन्यथा जर( j>=0 && (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)!=0 ){
      p_अभिव्यक्ती = sqlite3_Column_अभिव्यक्ती(pTab, &pTab->aCol[j]);
    }अन्यथा{
      सुरूठेवा;
    }
    जर( sqlite3_अभिव्यक्ती_Is_Constant(0,p_अभिव्यक्ती) ) सुरूठेवा;
    p = sqlite3DbMallocRaw(pParse->db,  आकार(Indexed_अभिव्यक्ती));
    जर( p==0 ) थांबा;
    p->pIENext = pParse->pIdxEpr;
#ifdef WHERETRACE_ENABLED
    जर( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      जर( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_अभिव्यक्ती(p_अभिव्यक्ती);
    }
#endif
    p->p_अभिव्यक्ती = sqlite3_अभिव्यक्ती_Dup(pParse->db, p_अभिव्यक्ती, 0);
    p->iDataCur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (JT_डावा|JT_LTORJ|JT_उजवा))!=0;
    जर( sqlite3IndexAffinityStr(pParse->db, pIdx) ){
      p->aff = pIdx->zColAff[i];
    }
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#endif
    pParse->pIdxEpr = p;
    जर( p->pIENext==0 ){
      शून्य *pArg = (शून्य*)&pParse->pIdxEpr;
      sqlite3ParserAddCleanup(pParse, where_Indexed_अभिव्यक्ती_Cleanup, pArg);
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
स्थिर SQLITE_NOINLINE शून्य whereReverseScanOrder(WhereInfo *pWInfo){
  पूर्णांक ii;
  साठी(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    जर( !pItem->fg.isCte
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
  अभिव्यक्ती *pWhere,           /* The WHERE clause */
  अभिव्यक्ती_List *pOrderBy,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  अभिव्यक्ती_List *pResultSet,   /* Query result set.  Req'd for DISTINCT */
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

  प्रतिज्ञा( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  प्रतिज्ञा( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_LIMIT)==0 );

  /* Variable initialization */
  db = pParse->db;
  स_रखो(&sWLB, 0, आकार(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( pOrderBy && pOrderBy->n_अभिव्यक्ती==BMS-1 );
  जर( pOrderBy && pOrderBy->n_अभिव्यक्ती>=BMS ){
    pOrderBy = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  जर( pTabList->nSrc>BMS ){
    sqlite3_Error_संदेश(pParse, "at most %d tables in a join", BMS);
    परत 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + आकार(WhereLoop));
  जर( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    जा whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;
#जर WHERETRACE_ENABLED
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
  स_रखो(&pWInfo->nOBSat, 0,
         दुरत्व(WhereInfo,sWC) - दुरत्व(WhereInfo,nOBSat));
  स_रखो(&pWInfo->a[0], 0, आकार(WhereLoop)+nTabList*आकार(WhereLevel));
  प्रतिज्ञा( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  pMaskSet = &pWInfo->sMaskSet;
  pMaskSet->n = 0;
  pMaskSet->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((अक्षर*)pWInfo)+nByteWInfo);
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
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
  जर( nTabList==0 ){
    जर( pOrderBy ) pWInfo->nOBSat = pOrderBy->n_अभिव्यक्ती;
    जर( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    जर( ALWAYS(pWInfo->pSelect)
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
    करा{
      createMask(pMaskSet, pTabList->a[ii].iCursor);
      sqlite3WhereTabFuncArgs(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }पर्यंत( (++ii)<pTabList->nSrc );
  #ifdef SQLITE_DEBUG
    {
      Bitmask mx = 0;
      साठी(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3WhereGetMask(pMaskSet, pTabList->a[ii].iCursor);
        प्रतिज्ञा( m>=mx );
        mx = m;
      }
    }
  #endif
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_अभिव्यक्ती_Analyze(pTabList, &pWInfo->sWC);
  जर( pSelect && pSelect->pLimit ){
    sqlite3WhereAddLimit(&pWInfo->sWC, pSelect);
  }
  जर( pParse->nErr ) जा whereBeginError;

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
  साठी(ii=0; ii<sWLB.pWC->nBase; ii++){
    WhereTerm *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    अभिव्यक्ती *pX;                          /* The expression of pT */
    जर( pT->wtFlags & TERM_VIRTUAL ) सुरूठेवा;
    pX = pT->p_अभिव्यक्ती;
    प्रतिज्ञा( pX!=0 );
    प्रतिज्ञा( pT->prereqAll!=0 || !अभिव्यक्ती_Has_Property(pX, EP_OuterON) );
    जर( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || अभिव्यक्ती_Is_Deterministic(pX))   /* Condition (4) */
     && !(अभिव्यक्ती_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_अभिव्यक्ती_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= TERM_CODED;
    }
  }

  जर( wctrlFlags & WHERE_WANT_DISTINCT ){
    जर( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }अन्यथा जर( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, pResultSet) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }अन्यथा जर( pOrderBy==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->pOrderBy = pResultSet;
    }
  }

  /* Construct the WhereLoop objects */
#जर defined(WHERETRACE_ENABLED)
  जर( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    जर( wctrlFlags & WHERE_USE_LIMIT ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    जर( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      स_रखो(&sSelect, 0, आकार(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.pOrderBy = pOrderBy;
      sSelect.pEList = pResultSet;
      sqlite3_वृक्ष_View_Select(0, &sSelect, 0);
    }
    जर( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#endif

  जर( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = whereLoopAddAll(&sWLB);
    जर( rc ) जा whereBeginError;

#ifdef SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    जर( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      पर्यंत( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->pNextLoop;
        whereLoopDelete(db, p);
      }
      rc = whereLoopAddAll(&sWLB);
      जर( rc ) जा whereBeginError;
    }
#endif
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    wherePathSolver(pWInfo, 0);
    जर( db->mallocFailed ) जा whereBeginError;
    जर( pWInfo->pOrderBy ){
       whereInterstageHeuristic(pWInfo);
       wherePathSolver(pWInfo, pWInfo->nRowOut<0 ? 1 : pWInfo->nRowOut+1);
       जर( db->mallocFailed ) जा whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    जर( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->nRowOut, pWInfo->nRowOut-30));
      pWInfo->nRowOut -= 30;
    }

  }
  प्रतिज्ञा( pWInfo->pTabList!=0 );
  जर( pWInfo->pOrderBy==0 && (db->flags & SQLITE_ReverseOrder)!=0 ){
    whereReverseScanOrder(pWInfo);
  }
  जर( pParse->nErr ){
    जा whereBeginError;
  }
  प्रतिज्ञा( db->mallocFailed==0 );
#ifdef WHERETRACE_ENABLED
  जर( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->rTotalCost, pWInfo->nRowOut);
    जर( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->revMask);
    }
    निवड( pWInfo->eDistinct ){
      स्थिती WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        थांबा;
      }
      स्थिती WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        थांबा;
      }
      स्थिती WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        थांबा;
      }
    }
    sqlite3DebugPrintf("\n");
    साठी(ii=0; ii<pWInfo->nLevel; ii++){
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
  जर( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && pResultSet!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    प्रतिज्ञा( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  जर( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#जर defined(WHERETRACE_ENABLED)
  जर( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
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
  प्रतिज्ञा( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  जर( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    पूर्णांक wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    पूर्णांक bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    प्रतिज्ञा( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    जर( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      जर( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        जर( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  साठी(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    Table *pTab;     /* Table to open */
    पूर्णांक iDb;         /* Index of database containing table/index */
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    जर( ii==0 || (pTabItem[0].fg.jointype & JT_डावा)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }अन्यथा जर( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }अन्यथा{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    जर( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }अन्यथा
#ifndef SQLITE_OMIT_VIRTUALTABLE
    जर( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      स्थिरांक अक्षर *pVTab = (स्थिरांक अक्षर *)sqlite3GetVTable(db, pTab);
      पूर्णांक iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }अन्यथा जर( IsVirtual(pTab) ){
      /* noop */
    }अन्यथा
#endif
    जर( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (pTabItem->fg.jointype & (JT_LTORJ|JT_उजवा))!=0
    ){
      पूर्णांक op = OP_OpenRead;
      जर( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);
      प्रतिज्ञा( pTabItem->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS );
      जर( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->nCol<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_INDEX|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = pTabItem->colUsed;
        पूर्णांक n = 0;
        साठी(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_TO_PTR(n), P4_INT32);
        प्रतिज्ञा( n<=pTab->nCol );
      }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
      जर( pLoop->u.btree.pIndex!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }अन्यथा
#endif
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
      sqlite3VdbeAddOp3(v, OP_ColumnsUsed, pTabItem->iCursor,
                    LOWER32(pTabItem->colUsed), UPPER32(pTabItem->colUsed));
#endif
      जर( ii>=2
       && (pTabItem[0].fg.jointype & (JT_LTORJ|JT_डावा))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3VdbeAddOp2(v, OP_IfEmpty, pTabItem->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }अन्यथा{
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    जर( pLoop->wsFlags & WHERE_INDEXED ){
      Index *pIx = pLoop->u.btree.pIndex;
      पूर्णांक iIndexCur;
      पूर्णांक op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      प्रतिज्ञा( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      जर( !HasRowid(pTab) && Is_Primary_किल्ली_Index(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        iIndexCur = pLevel->iTabCur;
        op = 0;
      }अन्यथा जर( pWInfo->eOnePass!=ONEPASS_OFF ){
        Index *pJ = pTabItem->pSTab->pIndex;
        iIndexCur = iAuxArg;
        प्रतिज्ञा( wctrlFlags & WHERE_ONEPASS_DESIRED );
        पर्यंत( ALWAYS(pJ) && pJ!=pIx ){
          iIndexCur++;
          pJ = pJ->pNext;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = iIndexCur;
      }अन्यथा जर( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        iIndexCur = iAuxArg;
        op = OP_ReopenIdx;
      }अन्यथा{
        iIndexCur = pParse->nTab++;
        जर( pIx->b_Has_अभिव्यक्ती && OptimizationEnabled(db, SQLITE_Indexed_अभिव्यक्ती) ){
          where_Add_Indexed_अभिव्यक्ती(pParse, pIx, iIndexCur, pTabItem);
        }
        जर( pIx->pPartIdxWhere && (pTabItem->fg.jointype & JT_उजवा)==0 ){
          where_Part_Idx_अभिव्यक्ती(
              pParse, pIx, pIx->pPartIdxWhere, 0, iIndexCur, pTabItem
          );
        }
      }
      pLevel->iIdxCur = iIndexCur;
      प्रतिज्ञा( pIx!=0 );
      प्रतिज्ञा( pIx->pSchema==pTab->pSchema );
      प्रतिज्ञा( iIndexCur>=0 );
      जर( op ){
        sqlite3VdbeAddOp3(v, op, iIndexCur, pIx->tnum, iDb);
        sqlite3_Vdbe_Set_P4_किल्ली_Info(pParse, pIx);
        जर( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
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
          पूर्णांक ii, jj;
          साठी(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            जर( jj<0 ) सुरूठेवा;
            जर( jj>63 ) jj = 63;
            जर( (pTabItem->colUsed & MASKBIT(jj))==0 ) सुरूठेवा;
            colUsed |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3VdbeAddOp3(v, OP_ColumnsUsed, iIndexCur,
                            LOWER32(colUsed),UPPER32(colUsed));
        }
#endif /* SQLITE_ENABLE_COLUMN_USED_MASK */
#ifdef SQLITE_ENABLE_CURSOR_HINTS
        जर( HasRowid(pTab) ){
          sqlite3VdbeAddOp3(v, OP_CursorHint, iIndexCur, 0, pTabItem->iCursor);
        }
#endif
      }
    }
    जर( iDb>=0 ) sqlite3CodeVerifySchema(pParse, iDb);
    जर( (pTabItem->fg.jointype & JT_उजवा)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, आकार(Where_उजवा_Join)))!=0
    ){
      Where_उजवा_Join *pRJ = pLevel->pRJ;
      पूर्णांक bBloomable = 0;
      pRJ->iMatch = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, pRJ->regReturn);
      प्रतिज्ञा( pTab==pTabItem->pSTab );
      जर( HasRowid(pTab) ){
        किल्ली_Info *pInfo;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, 1);
        bBloomable = 1;
        pInfo = sqlite3_किल्ली_Info_Alloc(pParse->db, 1, 0);
        जर( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->aSortFlags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }अन्यथा{
        Index *pPk = sqlite3_Primary_किल्ली_Index(pTab);
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, pPk->n_किल्ली_Col);
        sqlite3_Vdbe_Set_P4_किल्ली_Info(pParse, pPk);
        bBloomable = sqlite3IndexBloomable(pPk,pPk->n_किल्ली_Col);
      }
      जर( bBloomable ){
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
  pWInfo->i_शीर्ष = sqlite3VdbeCurrentAddr(v);
  जर( db->mallocFailed ) जा whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  साठी(ii=0; ii<nTabList; ii++){
    पूर्णांक addrExplain;
    पूर्णांक wsFlags;
    SrcItem *pSrc;
    जर( pParse->nErr ) जा whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    जर( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      पूर्णांक iOnce = 0;
      प्रतिज्ञा( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      जर( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }अन्यथा{
        iOnce = 0;
      }
      sqlite3VdbeAddOp2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      जर( iOnce )  sqlite3VdbeJumpHere(v, iOnce);
    }
    प्रतिज्ञा( pTabList == pWInfo->pTabList );
    जर( (wsFlags & (WHERE_AUTO_INDEX|WHERE_BLOOMFILTER))!=0 ){
      जर( (wsFlags & WHERE_AUTO_INDEX)!=0 ){
#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
        constructAutomaticIndex(pParse, &pWInfo->sWC, notReady, pLevel);
#endif
      }अन्यथा{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      जर( db->mallocFailed ) जा whereBeginError;
    }
    addrExplain = sqlite3WhereExplainOneScan(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    जर( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3WhereAddScanStatus(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->iEndWhere = sqlite3VdbeCurrentAddr(v);
  परत pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  जर( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#ifdef WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#endif /* WHERETRACE_ENABLED */
  परत 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#ifndef SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#अन्यथा
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  स्थिर शून्य sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    पूर्णांक pc,
    VdbeOp *pOp
  ){
    जर( (db->flags & SQLITE_VdbeAddopTrace)==0 ) परत;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3ShowWhereTerm(0); /* So compiler won't complain about unused func */
  }
#endif

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
शून्य sqlite3WhereEnd(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  पूर्णांक i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  पूर्णांक iEnd = sqlite3VdbeCurrentAddr(v);
  पूर्णांक nRJ = 0;
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  पूर्णांक addrSeek = 0;
#endif

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  साठी(i=pWInfo->nLevel-1; i>=0; i--){
    पूर्णांक addr;
    pLevel = &pWInfo->a[i];
    जर( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_उजवा_Join *pRJ = pLevel->pRJ;
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
    जर( pLevel->op!=OP_Noop ){
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      Index *pIdx;
      पूर्णांक n;
      जर( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_INDEXED)!=0
       && (pIdx = pLoop->u.btree.pIndex)->hasStat1
       && (n = pLoop->u.btree.nDistinctCol)>0
       && pIdx->aiRowLogEst[n]>=36
      ){
        पूर्णांक r1 = pParse->nMem+1;
        पूर्णांक j, op;
        पूर्णांक addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        जर( pLevel->i_डावा_Join ){
          addrIfNull = sqlite3VdbeAddOp2(v, OP_IfNullRow, pLevel->iIdxCur, r1);
        }
        साठी(j=0; j<n; j++){
          sqlite3VdbeAddOp3(v, OP_Column, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3VdbeAddOp4Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3VdbeAddOp2(v, OP_Goto, 1, pLevel->p2);
        जर( pLevel->i_डावा_Join ){
          sqlite3VdbeJumpHere(v, addrIfNull);
        }
      }
#endif /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    जर( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }अन्यथा जर( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      पूर्णांक iBypass = sqlite3VdbeAddOp0(v, OP_Goto);
      प्रतिज्ञा( pLoop->wsFlags & WHERE_AUTO_INDEX );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3VdbeAddOp1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3VdbeJumpHere(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    जर( pLevel->op!=OP_Noop ){
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_Next);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      जर( addrSeek ){
        sqlite3VdbeJumpHere(v, addrSeek);
        addrSeek = 0;
      }
#endif
      जर( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3VdbeAddOp2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    जर( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
    प्रतिज्ञा( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_IN_ABLE) && pLevel->u.in.nIn>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    जर( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3VdbeAddOp2(v, OP_DecrJumpZero, (पूर्णांक)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#endif
    जर( (pLoop->wsFlags & WHERE_IN_ABLE)!=0 && pLevel->u.in.nIn>0 ){
      संरचना InLoop *pIn;
      पूर्णांक j;
      साठी(j=pLevel->u.in.nIn, pIn=&pLevel->u.in.aInLoop[j-1]; j>0; j--, pIn--){
        प्रतिज्ञा( sqlite3VdbeGetOp(v, pIn->addr_In_शीर्ष+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3VdbeJumpHere(v, pIn->addr_In_शीर्ष+1);
        जर( pIn->eEndLoopOp!=OP_Noop ){
          जर( pIn->nPrefix ){
            पूर्णांक bEarlyOut =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0;
            जर( pLevel->i_डावा_Join ){
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
            जर( bEarlyOut ){
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
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_Prev);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_Next);
        }
        sqlite3VdbeJumpHere(v, pIn->addr_In_शीर्ष-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    जर( pLevel->pRJ ){
      sqlite3VdbeAddOp3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    जर( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.pIndex->zName));
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip-2);
    }
    जर( pLevel->i_डावा_Join ){
      पूर्णांक ws = pLoop->wsFlags;
      addr = sqlite3VdbeAddOp1(v, OP_IfPos, pLevel->i_डावा_Join); VdbeCoverage(v);
      प्रतिज्ञा( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_INDEXED)!=0 );
      जर( (ws & WHERE_IDX_ONLY)==0 ){
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];
        प्रतिज्ञा( pLevel->iTabCur==pSrc->iCursor );
        जर( pSrc->fg.viaCoroutine ){
          पूर्णांक m, n;
          प्रतिज्ञा( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->regResult;
          प्रतिज्ञा( pSrc->pSTab!=0 );
          m = pSrc->pSTab->nCol;
          sqlite3VdbeAddOp3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iTabCur);
      }
      जर( (ws & WHERE_INDEXED)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        जर( ws & WHERE_MULTI_OR ){
          Index *pIx = pLevel->u.pCoveringIdx;
          पूर्णांक iDb = sqlite3SchemaToIndex(db, pIx->pSchema);
          sqlite3VdbeAddOp3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_Set_P4_किल्ली_Info(pParse, pIx);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
      }
      जर( ws & WHERE_IN_ABLE ){
        पूर्णांक m;
        साठी(m=0; m<pLevel->u.in.nIn; m++){
          संरचना InLoop *pIn = &pLevel->u.in.aInLoop[m];
          जर( pIn->eEndLoopOp!=OP_Noop ){
            sqlite3VdbeAddOp1(v, OP_NullRow, pIn->iCur);
          }
        }
      }
      जर( pLevel->op==OP_Return ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }अन्यथा{
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  प्रतिज्ञा( pWInfo->nLevel<=pTabList->nSrc );
  साठी(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    पूर्णांक k, last;
    VdbeOp *pOp, *pLastOp;
    Index *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pSTab;
    प्रतिज्ञा( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    जर( pLevel->pRJ ){
      sqlite3_Where_उजवा_Join_Loop(pWInfo, i, pLevel);
      सुरूठेवा;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    जर( pTabItem->fg.viaCoroutine ){
      testcase( pParse->db->mallocFailed );
      प्रतिज्ञा( pTabItem->fg.isSubquery );
      प्रतिज्ञा( pTabItem->u4.pSubq->regResult>=0 );
      translateColumnToCopy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->u4.pSubq->regResult, 0);
      सुरूठेवा;
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
    जर( pLoop->wsFlags & (WHERE_INDEXED|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.pIndex;
    }अन्यथा जर( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    जर( pIdx
     && !db->mallocFailed
    ){
      जर( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->pTable) ){
        last = iEnd;
      }अन्यथा{
        last = pWInfo->iEndWhere;
      }
      जर( pIdx->b_Has_अभिव्यक्ती ){
        Indexed_अभिव्यक्ती *p = pParse->pIdxEpr;
        पर्यंत( p ){
          जर( p->iIdxCur==pLevel->iIdxCur ){
#ifdef WHERETRACE_ENABLED
            जर( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->iIdxCol);
              जर( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_अभिव्यक्ती(p->p_अभिव्यक्ती);
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
      जर( db->flags & SQLITE_VdbeAddopTrace ){
        म_लिखो("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, last-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      प्रतिज्ञा( pOp->opcode!=OP_Column || pOp->p1!=pLevel->iTabCur );
      प्रतिज्ञा( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      प्रतिज्ञा( pOp->opcode!=OP_IfNullRow || pOp->p1!=pLevel->iTabCur );
#endif
      pOp = sqlite3VdbeGetOp(v, k);
      pLastOp = pOp + (last - k);
      प्रतिज्ञा( pOp<=pLastOp );
      करा{
        जर( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }अन्यथा जर( pOp->opcode==OP_Column
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
         || pOp->opcode==OP_Offset
#endif
        ){
          पूर्णांक x = pOp->p2;
          प्रतिज्ञा( pIdx->pTable==pTab );
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
          जर( pOp->opcode==OP_Offset ){
            /* Do not need to translate the column number */
          }अन्यथा
#endif
          जर( !HasRowid(pTab) ){
            Index *pPk = sqlite3_Primary_किल्ली_Index(pTab);
            x = pPk->aiColumn[x];
            प्रतिज्ञा( x>=0 );
          }अन्यथा{
            testcase( x!=sqlite3StorageColumnToTable(pTab,x) );
            x = sqlite3StorageColumnToTable(pTab,x);
          }
          x = sqlite3TableColumnToIndex(pIdx, x);
          जर( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }अन्यथा जर( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            जर( pLoop->wsFlags & WHERE_IDX_ONLY ){
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
        }अन्यथा जर( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }अन्यथा जर( pOp->opcode==OP_IfNullRow ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#ifdef SQLITE_DEBUG
        k++;
#endif
      }पर्यंत( (++pOp)<pLastOp );
#ifdef SQLITE_DEBUG
      जर( db->flags & SQLITE_VdbeAddopTrace ) म_लिखो("TRANSLATE complete\n");
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
  परत;
}
