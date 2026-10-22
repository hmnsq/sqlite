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
#شامل "sqliteInt.h"
#شامل "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
تعریفنوع ساختار HiddenIndexInfo HiddenIndexInfo;
ساختار HiddenIndexInfo {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  صحیح eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 mIn;                 /* Mask of terms that are <col> IN (...) */
  u32 mHandleIn;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_مقدار *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#define SZ_HIDDENINDEXINFO(N) \
                  (offsetof(HiddenIndexInfo,aRhs) + (N)*اندازه(sqlite3_مقدار*))

/* Forward declaration of methods */
ایستا صحیح whereLoopResize(sqlite3*, WhereLoop*, صحیح);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_Row_شمار(WhereInfo *pWInfo){
  بازگشت pWInfo->nRowOut;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
صحیح sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  بازگشت pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
صحیح sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  بازگشت pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
صحیح sqlite3WhereOrderByLimitOptLabel(WhereInfo *pWInfo){
  WhereLevel *pInner;
  اگر( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    بازگشت pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  أكد( pInner->addrNxt!=0 );
  بازگشت pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
تهی sqlite3WhereMinMaxOptEarlyOut(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  صحیح i;
  اگر( !pWInfo->bOrderedInnerLoop ) بازگشت;
  اگر( pWInfo->nOBSat==0 ) بازگشت;
  برای(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    اگر( (pInner->pWLoop->wsFlags & WHERE_COLUMN_IN)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      بازگشت;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
صحیح sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  أكد( pWInfo->iContinue!=0 );
  بازگشت pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
صحیح sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  بازگشت pWInfo->iBreak;
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
صحیح sqlite3WhereOkOnePass(WhereInfo *pWInfo, صحیح *aiCur){
  memcpy(aiCur, pWInfo->aiCurOnePass, اندازه(صحیح)*2);
#ifdef WHERETRACE_ENABLED
  اگر( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#endif
  بازگشت pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
صحیح sqlite3WhereUsesDeferredSeek(WhereInfo *pWInfo){
  بازگشت pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
ایستا تهی whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc){
  pDest->n = pSrc->n;
  memcpy(pDest->a, pSrc->a, pDest->n*اندازه(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
ایستا صحیح where_Or_درج(
  WhereOrSet *pSet,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst rRun,           /* Run-cost of the new entry */
  LogEst nOut            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  برای(i=pSet->n, p=pSet->a; i>0; i--, p++){
    اگر( rRun<=p->rRun && (prereq & p->prereq)==prereq ){
      برو where_Or_درج_done;
    }
    اگر( p->rRun<=rRun && (p->prereq & prereq)==p->prereq ){
      بازگشت 0;
    }
  }
  اگر( pSet->n<N_OR_COST ){
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  }وگرنه{
    p = pSet->a;
    برای(i=1; i<pSet->n; i++){
      اگر( p->rRun>pSet->a[i].rRun ) p = pSet->a + i;
    }
    اگر( p->rRun<=rRun ) بازگشت 0;
  }
where_Or_درج_done:
  p->prereq = prereq;
  p->rRun = rRun;
  اگر( p->nOut>nOut ) p->nOut = nOut;
  بازگشت 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, صحیح iCursor){
  صحیح i;
  أكد( pMaskSet->n<=(صحیح)اندازه(Bitmask)*8 );
  أكد( pMaskSet->n>0 || pMaskSet->ix[0]<0 );
  أكد( iCursor>=-1 );
  اگر( pMaskSet->ix[0]==iCursor ){
    بازگشت 1;
  }
  برای(i=1; i<pMaskSet->n; i++){
    اگر( pMaskSet->ix[i]==iCursor ){
      بازگشت MASKBIT(i);
    }
  }
  بازگشت 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
تهی *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  WhereMemBlock *pBlock;
  pBlock = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+اندازه(*pBlock));
  اگر( pBlock ){
    pBlock->pNext = pWInfo->pMemToFree;
    pBlock->sz = nByte;
    pWInfo->pMemToFree = pBlock;
    pBlock++;
  }
  بازگشت (تهی*)pBlock;
}
تهی *sqlite3WhereRealloc(WhereInfo *pWInfo, تهی *pOld, u64 nByte){
  تهی *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  اگر( pNew && pOld ){
    WhereMemBlock *pOldBlk = (WhereMemBlock*)pOld;
    pOldBlk--;
    أكد( pOldBlk->sz<nByte );
    memcpy(pNew, pOld, pOldBlk->sz);
  }
  بازگشت pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
ایستا تهی createMask(WhereMaskSet *pMaskSet, صحیح iCursor){
  أكد( pMaskSet->n < ArraySize(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
ایستا عبارت *where_راست_Subexpr_Is_Column(عبارت *p){
  p = sqlite3_عبارت_Skip_Collate_And_Likely(p->p_راست);
  اگر( ALWAYS(p!=0) && p->op==TK_COLUMN && !عبارت_Has_Property(p, EP_FixedCol) ){
    بازگشت p;
  }
  بازگشت 0;
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
ایستا SQLITE_NOINLINE ثابت نویسه *indexInAffinityOk(
  Parse *pParse, 
  WhereTerm *pTerm, 
  u8 idxaff
){
  عبارت *pX = pTerm->p_عبارت;
  عبارت inexpr;

  أكد( pTerm->eOperator & WO_IN );

  اگر( عبارت_Use_XSelect(pX) ){
    صحیح iField = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    اگر( sqlite3_عبارت_Is_Vector(pX->p_چپ) ){
      iField = pTerm->u.x.iField - 1;
      inexpr.p_چپ = pX->p_چپ->x.pList->a[iField].p_عبارت;
    }وگرنه{
      inexpr.p_چپ = pX->p_چپ;
    }
    inexpr.p_راست = pX->x.pSelect->pEList->a[iField].p_عبارت;
    pX = &inexpr;
  }

  اگر( sqlite3IndexAffinityOk(pX, idxaff) ){
    CollSeq *pRet = sqlite3_عبارت_Compare_Coll_Seq(pParse, pX);
    بازگشت pRet ? pRet->zName : sqlite3_Str_دودویی;
  }
  بازگشت 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
ایستا WhereTerm *whereScanNext(WhereScan *pScan){
  صحیح iCur;            /* The cursor on the LHS of the term */
  i16 iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  عبارت *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  WhereTerm *pTerm;    /* The term being tested */
  صحیح k = pScan->k;    /* Where to start scanning */

  أكد( pScan->iEquiv<=pScan->nEquiv );
  pWC = pScan->pWC;
  وقتی(1){
    iColumn = pScan->aiColumn[pScan->iEquiv-1];
    iCur = pScan->aiCur[pScan->iEquiv-1];
    أكد( pWC!=0 );
    أكد( iCur>=0 );
    انجامبده{
      برای(pTerm=pWC->a+k; k<pWC->nTerm; k++, pTerm++){
        أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 || pTerm->چپ_Cursor<0 );
        اگر( pTerm->چپ_Cursor==iCur
         && pTerm->u.x.چپ_Column==iColumn
         && (iColumn!=XN_عبارت
             || sqlite3_عبارت_Compare_Skip(pTerm->p_عبارت->p_چپ,
                                       pScan->p_Idx_عبارت,iCur)==0)
         && (pScan->iEquiv<=1 || !عبارت_Has_Property(pTerm->p_عبارت, EP_OuterON))
        ){
          اگر( (pTerm->eOperator & WO_EQUIV)!=0
           && pScan->nEquiv<ArraySize(pScan->aiCur)
           && (pX = where_راست_Subexpr_Is_Column(pTerm->p_عبارت))!=0
          ){
            صحیح j;
            برای(j=0; j<pScan->nEquiv; j++){
              اگر( pScan->aiCur[j]==pX->iTable
               && pScan->aiColumn[j]==pX->iColumn ){
                  بشکن;
              }
            }
            اگر( j==pScan->nEquiv ){
              pScan->aiCur[j] = pX->iTable;
              pScan->aiColumn[j] = pX->iColumn;
              pScan->nEquiv++;
            }
          }
          اگر( (pTerm->eOperator & pScan->opMask)!=0 ){
            /* Verify the affinity and collating sequence match */
            اگر( pScan->zCollName && (pTerm->eOperator & WO_ISNULL)==0 ){
              ثابت نویسه *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->p_عبارت;

              اگر( (pTerm->eOperator & WO_IN) ){
                zCollName = indexInAffinityOk(pParse, pTerm, pScan->idxaff);
                اگر( !zCollName ) ادامه;
              }وگرنه{
                CollSeq *pColl;
                اگر( !sqlite3IndexAffinityOk(pX, pScan->idxaff) ){
                  ادامه;
                }
                أكد(pX->p_چپ);
                pColl = sqlite3_عبارت_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_دودویی;
              }

              اگر( sqlite3StrICmp(zCollName, pScan->zCollName) ){
                ادامه;
              }
            }
            اگر( (pTerm->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = pTerm->p_عبارت->p_راست, ALWAYS(pX!=0))
             && pX->op==TK_COLUMN
             && pX->iTable==pScan->aiCur[0]
             && pX->iColumn==pScan->aiColumn[0]
            ){
              testcase( pTerm->eOperator & WO_IS );
              ادامه;
            }
            pScan->pWC = pWC;
            pScan->k = k+1;
#ifdef WHERETRACE_ENABLED
            اگر( (sqlite3WhereTrace & 0x20000)!=0 && pScan->nEquiv>1 ){
              صحیح ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 pScan->aiCur[0], pScan->aiColumn[0], pTerm->iTerm);
              برای(ii=1; ii<pScan->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   pScan->aiCur[ii], pScan->aiColumn[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#endif
            بازگشت pTerm;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }وقتی( pWC!=0 );
    اگر( pScan->iEquiv>=pScan->nEquiv ) بشکن;
    pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv++;
  }
  بازگشت 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
ایستا SQLITE_NOINLINE WhereTerm *where_Scan_Init_Index_عبارت(WhereScan *pScan){
  pScan->idxaff = sqlite3_عبارت_Affinity(pScan->p_Idx_عبارت);
  بازگشت whereScanNext(pScan);
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
ایستا WhereTerm *whereScanInit(
  WhereScan *pScan,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  صحیح iCur,               /* Cursor to scan for */
  صحیح iColumn,            /* Column to scan for */
  u32 opMask,             /* Operator(s) to scan for */
  Index *pIdx             /* Must be compatible with this index */
){
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  pScan->p_Idx_عبارت = 0;
  pScan->idxaff = 0;
  pScan->zCollName = 0;
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aiCur[0] = iCur;
  pScan->nEquiv = 1;
  pScan->iEquiv = 1;
  اگر( pIdx ){
    صحیح j = iColumn;
    iColumn = pIdx->aiColumn[j];
    اگر( iColumn==pIdx->pTable->iPKey ){
      iColumn = XN_ROWID;
    }وگرنه اگر( iColumn>=0 ){
      pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
      pScan->zCollName = pIdx->azColl[j];
    }وگرنه اگر( iColumn==XN_عبارت ){
      pScan->p_Idx_عبارت = pIdx->a_Col_عبارت->a[j].p_عبارت;
      pScan->zCollName = pIdx->azColl[j];
      pScan->aiColumn[0] = XN_عبارت;
      بازگشت where_Scan_Init_Index_عبارت(pScan);
    }
  }وگرنه اگر( iColumn==XN_عبارت ){
    بازگشت 0;
  }
  pScan->aiColumn[0] = iColumn;
  بازگشت whereScanNext(pScan);
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
  صحیح iCur,             /* Cursor number of LHS */
  صحیح iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  Index *pIdx           /* Must be compatible with this index, if not NULL */
){
  WhereTerm *pResult = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  وقتی( p ){
    اگر( (p->prereq_راست & notReady)==0 ){
      اگر( p->prereq_راست==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        بازگشت p;
      }
      اگر( pResult==0 ) pResult = p;
    }
    p = whereScanNext(&scan);
  }
  بازگشت pResult;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
ایستا صحیح findIndexCol(
  Parse *pParse,                  /* Parse context */
  عبارت_List *pList,                /* Expression list to search */
  صحیح iBase,                      /* Cursor for table associated with pIdx */
  Index *pIdx,                    /* Index to match column of */
  صحیح iCol                        /* Column of index to match */
){
  صحیح i;
  ثابت نویسه *zColl = pIdx->azColl[iCol];

  برای(i=0; i<pList->n_عبارت; i++){
    عبارت *p = sqlite3_عبارت_Skip_Collate_And_Likely(pList->a[i].p_عبارت);
    اگر( ALWAYS(p!=0)
     && (p->op==TK_COLUMN || p->op==TK_AGG_COLUMN)
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->iTable==iBase
    ){
      CollSeq *pColl = sqlite3_عبارت_NNColl_Seq(pParse, pList->a[i].p_عبارت);
      اگر( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        بازگشت i;
      }
    }
  }

  بازگشت -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
ایستا صحیح indexColumnNotNull(Index *pIdx, صحیح iCol){
  صحیح j;
  أكد( pIdx!=0 );
  أكد( iCol>=0 && iCol<pIdx->nColumn );
  j = pIdx->aiColumn[iCol];
  اگر( j>=0 ){
    بازگشت pIdx->pTable->aCol[j].notNull;
  }وگرنه اگر( j==(-1) ){
    بازگشت 1;
  }وگرنه{
    أكد( j==(-2) );
    بازگشت 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
ایستا صحیح isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  عبارت_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  Table *pTab;
  Index *pIdx;
  صحیح i;                         
  صحیح iBase;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  اگر( pTabList->nSrc!=1 ) بازگشت 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  برای(i=0; i<pDistinct->n_عبارت; i++){
    عبارت *p = sqlite3_عبارت_Skip_Collate_And_Likely(pDistinct->a[i].p_عبارت);
    اگر( NEVER(p==0) ) ادامه;
    اگر( p->op!=TK_COLUMN && p->op!=TK_AGG_COLUMN ) ادامه;
    اگر( p->iTable==iBase && p->iColumn<0 ) بازگشت 1;
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
  برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    اگر( !IsUniqueIndex(pIdx) ) ادامه;
    اگر( pIdx->pPartIdxWhere ) ادامه;
    برای(i=0; i<pIdx->n_کلید_Col; i++){
      اگر( 0==sqlite3WhereFindTerm(pWC, iBase, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        اگر( findIndexCol(pParse, pDistinct, iBase, pIdx, i)<0 ) بشکن;
        اگر( indexColumnNotNull(pIdx, i)==0 ) بشکن;
      }
    }
    اگر( i==pIdx->n_کلید_Col ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      بازگشت 1;
    }
  }

  بازگشت 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
ایستا LogEst estLog(LogEst N){
  بازگشت N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
ایستا تهی translateColumnToCopy(
  Parse *pParse,      /* Parsing context */
  صحیح iStart,         /* Translate from this opcode to the end */
  صحیح iTabCur,        /* OP_Column/OP_Rowid references to this table */
  صحیح iRegister,      /* The first column is in this register */
  صحیح iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  صحیح iEnd = sqlite3VdbeCurrentAddr(v);
  اگر( pParse->db->mallocFailed ) بازگشت;
#ifdef SQLITE_DEBUG
  اگر( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    printf("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, iStart, iEnd);
  }
#endif
  برای(; iStart<iEnd; iStart++, pOp++){
    اگر( pOp->p1!=iTabCur ) ادامه;
    اگر( pOp->opcode==OP_Column ){
#ifdef SQLITE_DEBUG
      اگر( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE OP_Column to OP_Copy at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }وگرنه اگر( pOp->opcode==OP_Rowid ){
#ifdef SQLITE_DEBUG
      اگر( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE OP_Rowid to OP_Sequence at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      اگر( iAutoidxCur==0 ){
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
#اگر !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
ایستا تهی whereTraceIndexInfoInputs(
  sqlite3_index_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  صحیح i;
  اگر( (sqlite3WhereTrace & 0x10)==0 ) بازگشت;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  برای(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].iTermOffset,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  برای(i=0; i<p->nOrderBy; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->aOrderBy[i].iColumn,
       p->aOrderBy[i].desc);
  }
}
ایستا تهی whereTraceIndexInfoOutputs(
  sqlite3_index_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  صحیح i;
  اگر( (sqlite3WhereTrace & 0x10)==0 ) بازگشت;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  برای(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argvIndex,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_رقم);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->orderByConsumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimatedRows);
}
#وگرنه
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
ایستا صحیح constraintCompatibleWithOuterJoin(
  ثابت WhereTerm *pTerm,       /* WHERE clause term to check */
  ثابت SrcItem *pSrc           /* Table we are trying to access */
){
  أكد( (pSrc->fg.jointype&(JT_چپ|JT_LTORJ|JT_راست))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))==JT_چپ );
  testcase( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))==JT_LTORJ );
  testcase( عبارت_Has_Property(pTerm->p_عبارت, EP_OuterON) )
  testcase( عبارت_Has_Property(pTerm->p_عبارت, EP_InnerON) );
  اگر( !عبارت_Has_Property(pTerm->p_عبارت, EP_OuterON|EP_InnerON)
   || pTerm->p_عبارت->w.iJoin != pSrc->iCursor
  ){
    بازگشت 0;
  }
  اگر( (pSrc->fg.jointype & (JT_چپ|JT_راست))!=0
   && NEVER(عبارت_Has_Property(pTerm->p_عبارت, EP_InnerON))
  ){
    بازگشت 0;
  }
  بازگشت 1;
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
ایستا SQLITE_NOINLINE صحیح columnIsGoodIndexCandidate(
  ثابت Table *pTab,
  صحیح iCol
){
  ثابت Index *pIdx;
  برای(pIdx = pTab->pIndex; pIdx!=0; pIdx=pIdx->pNext){
    صحیح j;
    برای(j=0; j<pIdx->n_کلید_Col; j++){
       اگر( pIdx->aiColumn[j]==iCol ){
         اگر( j==0 ) بازگشت 0;
         اگر( pIdx->hasStat1 && pIdx->aiRowLogEst[j+1]>20 ) بازگشت 0;
         بشکن;
       }
    }
  }
  بازگشت 1;
}
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */



#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
ایستا صحیح termCanDriveIndex(
  ثابت WhereTerm *pTerm,        /* WHERE clause term to check */
  ثابت SrcItem *pSrc,           /* Table we are trying to access */
  ثابت Bitmask notReady         /* Tables in outer loops of the join */
){
  نویسه aff;
  صحیح چپ_Col;
  
  اگر( pTerm->چپ_Cursor!=pSrc->iCursor ) بازگشت 0;
  اگر( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) بازگشت 0;
  أكد( (pSrc->fg.jointype & JT_راست)==0 );
  اگر( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0
   && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
  ){
    بازگشت 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  اگر( (pTerm->prereq_راست & notReady)!=0 ) بازگشت 0;
  أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
  چپ_Col = pTerm->u.x.چپ_Column;
  اگر( چپ_Col<0 ) بازگشت 0;
  aff = pSrc->pSTab->aCol[چپ_Col].affinity;
  اگر( !sqlite3IndexAffinityOk(pTerm->p_عبارت, aff) ) بازگشت 0;
  testcase( pTerm->p_عبارت->op==TK_IS );
  بازگشت columnIsGoodIndexCandidate(pSrc->pSTab, چپ_Col);
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
ایستا تهی explainAutomaticIndex(
  Parse *pParse,
  Index *pIdx,                    /* Automatic index to explain */
  صحیح bPartial,                   /* True if pIdx is a partial index */
  صحیح *pAddrExplain               /* OUT: Address of OP_Explain */
){
  اگر( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    Table *pTab = pIdx->pTable;
    ثابت نویسه *zSep = "";
    نویسه *zText = 0;
    صحیح ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    أكد( pIdx->nColumn>1 );
    أكد( pIdx->aiColumn[pIdx->nColumn-1]==XN_ROWID || !HasRowid(pTab) );
    برای(ii=0; ii<(pIdx->nColumn-1); ii++){
      ثابت نویسه *zName = 0;
      صحیح iCol = pIdx->aiColumn[ii];

      zName = pTab->aCol[iCol].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    zText = sqlite3_str_finish(pStr);
    اگر( zText==0 ){
      sqlite3OomFault(pParse->db);
    }وگرنه{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", zText, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(zText);
    }
  }
}
#وگرنه
# define explainAutomaticIndex(a,b,c,d)
#endif

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
ایستا SQLITE_NOINLINE تهی constructAutomaticIndex(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  ثابت Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  صحیح n_کلید_Col;                /* Number of columns in the constructed index */
  WhereTerm *pTerm;           /* A single term of the WHERE clause */
  WhereTerm *pWCEnd;          /* End of pWC->a[] */
  Index *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  صحیح addrInit;               /* Address of the initialization bypass jump */
  Table *pTable;              /* The table being indexed */
  صحیح addr_بالاترین;                /* Top of the index fill loop */
  صحیح regRecord;              /* Register holding an index record */
  صحیح n;                      /* Column counter */
  صحیح i;                      /* Loop counter */
  صحیح mxBitCol;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  نویسه *zNotUsed;             /* Extra space on the end of pIdx */
  Bitmask idxCols;            /* Bitmap of columns used for indexing */
  Bitmask extraCols;          /* Bitmap of additional columns */
  u8 sentWarning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  عبارت *pPartial = 0;         /* Partial Index Expression */
  صحیح iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  SrcItem *pSrc;              /* The FROM clause term to get the next index */
  صحیح addrCounter = 0;        /* Address where integer counter is initialized */
  صحیح regBase;                /* Array of registers where record is assembled */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addrExp = 0;            /* Address of OP_Explain */
#endif

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  أكد( v!=0 );
  addrInit = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_کلید_Col = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  pTable = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  برای(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    عبارت *p_عبارت = pTerm->p_عبارت;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    اگر( (pTerm->wtFlags & TERM_VIRTUAL)==0
     && sqlite3_عبارت_Is_Single_Table_Constraint(p_عبارت, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_عبارت_And(pParse, pPartial,
                                sqlite3_عبارت_Dup(pParse->db, p_عبارت, 0));
    }
    اگر( termCanDriveIndex(pTerm, pSrc, notReady) ){
      صحیح iCol;
      Bitmask cMask;
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.چپ_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      اگر( !sentWarning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", pTable->zName,
            pTable->aCol[iCol].zCnName);
        sentWarning = 1;
      }
      اگر( (idxCols & cMask)==0 ){
        اگر( whereLoopResize(pParse->db, pLoop, n_کلید_Col+1) ){
          برو end_auto_index_create;
        }
        pLoop->aLTerm[n_کلید_Col++] = pTerm;
        idxCols |= cMask;
      }
    }
  }
  أكد( n_کلید_Col>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_کلید_Col;
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
  اگر( IsView(pTable) ){
    extraCols = ALLBITS & ~idxCols;
  }وگرنه{
    extraCols = pSrc->colUsed & (~idxCols | MASKBIT(BMS-1));
  }
  اگر( !HasRowid(pTable) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    برای(i=0; i<pTable->nCol; i++){
      اگر( (pTable->aCol[i].colFlags & COLFLAG_PRIMKEY)==0 ) ادامه;
      اگر( i>=BMS-1 ){
        extraCols |= MASKBIT(BMS-1);
        بشکن;
      }
      اگر( idxCols & MASKBIT(i) ) ادامه;
      extraCols |= MASKBIT(i);
    }
  }
  mxBitCol = MIN(BMS-1,pTable->nCol);
  testcase( pTable->nCol==BMS-1 );
  testcase( pTable->nCol==BMS-2 );
  برای(i=0; i<mxBitCol; i++){
    اگر( extraCols & MASKBIT(i) ) n_کلید_Col++;
  }
  اگر( pSrc->colUsed & MASKBIT(BMS-1) ){
    n_کلید_Col += pTable->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  أكد( n_کلید_Col <= pTable->nCol + MAX(0, pTable->nCol - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3AllocateIndexObject(pParse->db, n_کلید_Col+HasRowid(pTable),
                                    0, &zNotUsed);
  اگر( pIdx==0 ) برو end_auto_index_create;
  pLoop->u.btree.pIndex = pIdx;
  pIdx->zName = "auto-index";
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  برای(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    اگر( termCanDriveIndex(pTerm, pSrc, notReady) ){
      صحیح iCol;
      Bitmask cMask;
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.چپ_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      اگر( (idxCols & cMask)==0 ){
        عبارت *pX = pTerm->p_عبارت;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.x.چپ_Column;
        pColl = sqlite3_عبارت_Compare_Coll_Seq(pParse, pX);
        أكد( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        اگر( !sqlite3_Is_دودویی(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_دودویی;
        n++;
      }
    }
  }
  أكد( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  برای(i=0; i<mxBitCol; i++){
    اگر( extraCols & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_دودویی;
      n++;
    }
  }
  اگر( pSrc->colUsed & MASKBIT(BMS-1) ){
    برای(i=BMS-1; i<pTable->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_دودویی;
      n++;
    }
  }
  أكد( n==n_کلید_Col );
  اگر( HasRowid(pTable) ){
    pIdx->aiColumn[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_دودویی;
  }

  /* Create the automatic index */
  explainAutomaticIndex(pParse, pIdx, pPartial!=0, &addrExp);
  أكد( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_کلید_Col+1);
  sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pIdx);
  VdbeComment((v, "for %s", pTable->zName));
  اگر( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  أكد( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  اگر( pSrc->fg.viaCoroutine ){
    صحیح regYield;
    Subquery *pSubq;
    أكد( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    أكد( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3VdbeAddOp2(v, OP_Integer, 0, 0);
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_بالاترین =  sqlite3VdbeAddOp1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }وگرنه{
    أكد( pLevel->addrHalt );
    addr_بالاترین = sqlite3VdbeAddOp2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  اگر( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_عبارت_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  regRecord = sqlite3GetTempReg(pParse);
  regBase = sqlite3_Generate_Index_کلید(
      pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0
  );
  اگر( pLevel->regFilter ){
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0,
                         regBase, pLoop->u.btree.nEq);
  }
  sqlite3VdbeScanStatusCounters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3VdbeAddOp2(v, OP_Idx_درج, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  اگر( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  اگر( pSrc->fg.viaCoroutine ){
    أكد( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, regBase+n);
    testcase( pParse->db->mallocFailed );
    أكد( pLevel->iIdxCur>0 );
    translateColumnToCopy(pParse, addr_بالاترین, pLevel->iTabCur,
                          pSrc->u4.pSubq->regResult, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_بالاترین);
    pSrc->fg.viaCoroutine = 0;
    sqlite3VdbeJumpHere(v, addr_بالاترین);
  }وگرنه{
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addr_بالاترین+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    اگر( (pSrc->fg.jointype & JT_چپ)!=0 ){
      sqlite3VdbeJumpHere(v, addr_بالاترین);
    }
  }
  sqlite3ReleaseTempReg(pParse, regRecord);
 
  /* Jump here when skipping the initialization */
  sqlite3VdbeJumpHere(v, addrInit);
  sqlite3VdbeScanStatusRange(v, addrExp, addrExp, -1);

end_auto_index_create:
  sqlite3_عبارت_Delete(pParse->db, pPartial);
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
ایستا SQLITE_NOINLINE تهی sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  صحیح iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  صحیح addrOnce;                        /* Address of opening OP_Once */
  صحیح addr_بالاترین;                         /* Address of OP_Rewind */
  صحیح addrCont;                        /* Jump here to skip a row */
  ثابت WhereTerm *pTerm;              /* For looping over WHERE clause terms */
  ثابت WhereTerm *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  صحیح iCur;                            /* Cursor for table getting the filter */
  Indexed_عبارت *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  Indexed_عبارت *saved_p_Idx_Part_عبارت;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_عبارت = pParse->p_Idx_Part_عبارت;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_عبارت = 0;

  أكد( pLoop!=0 );
  أكد( v!=0 );
  أكد( pLoop->wsFlags & WHERE_BLOOMFILTER );
  أكد( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  انجامبده{
    ثابت SrcList *pTabList;
    ثابت SrcItem *pItem;
    ثابت Table *pTab;
    u64 sz;
    صحیح iSrc;
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
    أكد( pItem!=0 );
    pTab = pItem->pSTab;
    أكد( pTab!=0 );
    sz = sqlite3LogEstToInt(pTab->nRowLogEst);
    اگر( sz<10000 ){
      sz = 10000;
    }وگرنه اگر( sz>10000000 ){
      sz = 10000000;
    }
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp2(v, OP_Blob, (صحیح)sz, pLevel->regFilter);

    addr_بالاترین = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.nTerm];
    برای(pTerm=pWInfo->sWC.a; pTerm<pWCEnd; pTerm++){
      عبارت *p_عبارت = pTerm->p_عبارت;
      اگر( (pTerm->wtFlags & TERM_VIRTUAL)==0
       && sqlite3_عبارت_Is_Single_Table_Constraint(p_عبارت, pTabList, iSrc, 0)
      ){
        sqlite3_عبارت_If_False(pParse, pTerm->p_عبارت, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    اگر( pLoop->wsFlags & WHERE_IPK ){
      صحیح r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, r1);
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, 1);
      sqlite3ReleaseTempReg(pParse, r1);
    }وگرنه{
      Index *pIdx = pLoop->u.btree.pIndex;
      صحیح n = pLoop->u.btree.nEq;
      صحیح r1 = sqlite3GetTempRange(pParse, n);
      صحیح jj;
      أكد( pIdx!=0 );
      برای(jj=0; jj<n; jj++){
        أكد( pIdx->pTable==pItem->pSTab );
        sqlite3_عبارت_Code_Load_Index_Column(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, n);
      sqlite3ReleaseTempRange(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addr_بالاترین+1);
    VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr_بالاترین);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    وقتی( ++iLevel < pWInfo->nLevel ){
      ثابت SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      اگر( pTabItem->fg.jointype & (JT_چپ|JT_LTORJ) ) ادامه;
      pLoop = pLevel->pWLoop;
      اگر( NEVER(pLoop==0) ) ادامه;
      اگر( pLoop->prereq & notReady ) ادامه;
      اگر( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_COLUMN_IN))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        بشکن;
      }
    }
  }وقتی( iLevel < pWInfo->nLevel );
  sqlite3VdbeJumpHere(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_عبارت = saved_p_Idx_Part_عبارت;
}


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
ایستا WhereTerm *termFromWhereClause(WhereClause *pWC, صحیح iTerm){
  WhereClause *p;
  برای(p=pWC; p; p=p->pOuter){
    اگر( iTerm<p->nTerm ) بازگشت &p->a[iTerm];
    iTerm -= p->nTerm;
  }
  بازگشت 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
ایستا sqlite3_index_info *allocateIndexInfo(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  SrcItem *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  صحیح i, j;
  صحیح nTerm;
  Parse *pParse = pWInfo->pParse;
  ساختار sqlite3_index_constraint *pIdxCons;
  ساختار sqlite3_index_orderby *pIdxOrderBy;
  ساختار sqlite3_index_constraint_usage *pUsage;
  ساختار HiddenIndexInfo *pHidden;
  WhereTerm *pTerm;
  صحیح nOrderBy;
  sqlite3_index_info *pIdxInfo;
  u16 mNoOmit = 0;
  ثابت Table *pTab;
  صحیح eDistinct = 0;
  عبارت_List *pOrderBy = pWInfo->pOrderBy;
  WhereClause *p;

  أكد( pSrc!=0 );
  pTab = pSrc->pSTab;
  أكد( pTab!=0 );
  أكد( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  برای(p=pWC, nTerm=0; p; p=p->pOuter){
    برای(i=0, pTerm=p->a; i<p->nTerm; i++, pTerm++){
      pTerm->wtFlags &= ~TERM_OK;
      اگر( pTerm->چپ_Cursor != pSrc->iCursor ) ادامه;
      اگر( pTerm->prereq_راست & mUnusable ) ادامه;
      أكد( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
      testcase( pTerm->eOperator & WO_IN );
      testcase( pTerm->eOperator & WO_ISNULL );
      testcase( pTerm->eOperator & WO_IS );
      testcase( pTerm->eOperator & WO_ALL );
      اگر( (pTerm->eOperator & ~(WO_EQUIV))==0 ) ادامه;
      اگر( pTerm->wtFlags & TERM_VNULL ) ادامه;

      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      أكد( pTerm->u.x.چپ_Column>=XN_ROWID );
      أكد( pTerm->u.x.چپ_Column<pTab->nCol );
      اگر( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0
          && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
        ){
        ادامه;
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
  اگر( pOrderBy ){
    صحیح n = pOrderBy->n_عبارت;
    برای(i=0; i<n; i++){
      عبارت *p_عبارت = pOrderBy->a[i].p_عبارت;
      عبارت *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      اگر( sqlite3_عبارت_Is_Constant(0, p_عبارت) ){
        ادامه;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      اگر( pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) بشکن;

      /* First case - a direct column references without a COLLATE operator */
      اگر( p_عبارت->op==TK_COLUMN && p_عبارت->iTable==pSrc->iCursor ){
        أكد( p_عبارت->iColumn>=XN_ROWID && p_عبارت->iColumn<pTab->nCol );
        ادامه;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      اگر( p_عبارت->op==TK_COLLATE
       && (pE2 = p_عبارت->p_چپ)->op==TK_COLUMN
       && pE2->iTable==pSrc->iCursor
      ){
        ثابت نویسه *zColl;  /* The collating sequence name */
        أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
        أكد( p_عبارت->u.zToken!=0 );
        أكد( pE2->iColumn>=XN_ROWID && pE2->iColumn<pTab->nCol );
        p_عبارت->iColumn = pE2->iColumn;
        اگر( pE2->iColumn<0 ) ادامه;  /* Collseq does not matter for rowid */
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        اگر( zColl==0 ) zColl = sqlite3_Str_دودویی;
        اگر( sqlite3_stricmp(p_عبارت->u.zToken, zColl)==0 ) ادامه;
      }

      /* No matches cause a break out of the loop */
      بشکن;
    }
    اگر( i==n ){
      صحیح bSortByGroup = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      nOrderBy = n;
      اگر( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowidUsed ){
        eDistinct = 2 + bSortByGroup;
      }وگرنه اگر( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - bSortByGroup;
      }وگرنه اگر( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, اندازه(*pIdxInfo)
                           + (اندازه(*pIdxCons) + اندازه(*pUsage))*nTerm
                           + اندازه(*pIdxOrderBy)*nOrderBy
                           + SZ_HIDDENINDEXINFO(nTerm) );
  اگر( pIdxInfo==0 ){
    sqlite3_Error_پیام(pParse, "out of memory");
    بازگشت 0;
  }
  pHidden = (ساختار HiddenIndexInfo*)&pIdxInfo[1];
  pIdxCons = (ساختار sqlite3_index_constraint*)&pHidden->aRhs[nTerm];
  pIdxOrderBy = (ساختار sqlite3_index_orderby*)&pIdxCons[nTerm];
  pUsage = (ساختار sqlite3_index_constraint_usage*)&pIdxOrderBy[nOrderBy];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->aOrderBy = pIdxOrderBy;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->colUsed = (sqlite3_int64)pSrc->colUsed;
  اگر( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    Index *pPk = sqlite3_Primary_کلید_Index((Table*)pTab);
    أكد( pPk!=0 );
    برای(i=0; i<pPk->n_کلید_Col; i++){
      صحیح iCol = pPk->aiColumn[i];
      أكد( iCol>=0 );
      اگر( iCol>=BMS-1 ) iCol = BMS-1;
      pIdxInfo->colUsed |= MASKBIT(iCol);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->mIn = 0;
  برای(p=pWC, i=j=0; p; p=p->pOuter){
    صحیح nLast = i+p->nTerm;;
    برای(pTerm=p->a; i<nLast; i++, pTerm++){
      u16 op;
      اگر( (pTerm->wtFlags & TERM_OK)==0 ) ادامه;
      pIdxCons[j].iColumn = pTerm->u.x.چپ_Column;
      pIdxCons[j].iTermOffset = i;
      op = pTerm->eOperator & WO_ALL;
      اگر( op==WO_IN ){
        اگر( (pTerm->wtFlags & TERM_SLICE)==0 ){
          pHidden->mIn |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      اگر( op==WO_AUX ){
        pIdxCons[j].op = pTerm->eMatchOp;
      }وگرنه اگر( op & (WO_ISNULL|WO_IS) ){
        اگر( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_ISNULL;
        }وگرنه{
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_IS;
        }
      }وگرنه{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        أكد( WO_EQ==SQLITE_INDEX_CONSTRAINT_EQ );
        أكد( WO_LT==SQLITE_INDEX_CONSTRAINT_LT );
        أكد( WO_LE==SQLITE_INDEX_CONSTRAINT_LE );
        أكد( WO_GT==SQLITE_INDEX_CONSTRAINT_GT );
        أكد( WO_GE==SQLITE_INDEX_CONSTRAINT_GE );
        أكد( pTerm->eOperator&(WO_IN|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        اگر( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_عبارت_Is_Vector(pTerm->p_عبارت->p_راست)
          ){
          testcase( j!=i );
          اگر( j<16 ) mNoOmit |= (1 << j);
          اگر( op==WO_LT ) pIdxCons[j].op = WO_LE;
          اگر( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  أكد( j==nTerm );
  pIdxInfo->nConstraint = j;
  برای(i=j=0; i<nOrderBy; i++){
    عبارت *p_عبارت = pOrderBy->a[i].p_عبارت;
    اگر( sqlite3_عبارت_Is_Constant(0, p_عبارت) ) ادامه;
    أكد( p_عبارت->op==TK_COLUMN
         || (p_عبارت->op==TK_COLLATE && p_عبارت->p_چپ->op==TK_COLUMN
              && p_عبارت->iColumn==p_عبارت->p_چپ->iColumn) );
    pIdxOrderBy[j].iColumn = p_عبارت->iColumn;
    pIdxOrderBy[j].desc = pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC;
    j++;
  }
  pIdxInfo->nOrderBy = j;

  *pmNoOmit = mNoOmit;
  بازگشت pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
ایستا تهی freeIdxStr(sqlite3_index_info *pIdxInfo){
  اگر( pIdxInfo->needToFreeIdxStr ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->needToFreeIdxStr = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
ایستا تهی freeIndexInfo(sqlite3 *db, sqlite3_index_info *pIdxInfo){
  HiddenIndexInfo *pHidden;
  صحیح i;
  أكد( pIdxInfo!=0 );
  pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  أكد( pHidden->pParse!=0 );
  أكد( pHidden->pParse->db==db );
  برای(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_مقدار_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
ایستا صحیح vtabBestIndex(Parse *pParse, Table *pTab, sqlite3_index_info *p){
  صحیح rc;
  sqlite3_vtab *pVtab;

  أكد( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  whereTraceIndexInfoInputs(p, pTab);
  pParse->db->nSchemaLock++;
  rc = pVtab->pModule->xBestIndex(pVtab, p);
  pParse->db->nSchemaLock--;
  whereTraceIndexInfoOutputs(p, pTab);

  اگر( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    اگر( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }وگرنه اگر( !pVtab->z_Err_پیام ){
      sqlite3_Error_پیام(pParse, "%s", sqlite3ErrStr(rc));
    }وگرنه{
      sqlite3_Error_پیام(pParse, "%s", pVtab->z_Err_پیام);
    }
  }
  اگر( pTab->u.vtab.p->bAllSchemas ){
    sqlite3VtabUsesAllSchemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_پیام);
  pVtab->z_Err_پیام = 0;
  بازگشت rc;
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
ایستا صحیح where_کلید_Stats(
  Parse *pParse,              /* Database connection */
  Index *pIdx,                /* Index to consider domain of */
  UnpackedRecord *pRec,       /* Vector of values to consider */
  صحیح roundUp,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  IndexSample *aSample = pIdx->aSample;
  صحیح iCol;                   /* Index of required stats in anEq[] etc. */
  صحیح i;                      /* Index of first sample >= pRec */
  صحیح iSample;                /* Smallest sample larger than or equal to pRec */
  صحیح iMin = 0;               /* Smallest sample not yet tested */
  صحیح iTest;                  /* Next sample to test */
  صحیح res;                    /* Result of comparison operation */
  صحیح nField;                 /* Number of fields in pRec */
  tRowcnt iLower = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER( pParse );
#endif
  أكد( pRec!=0 );
  أكد( pIdx->nSample>0 );
  أكد( pRec->nField>0 );


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
  اگر( !HasRowid(pIdx->pTable) && Is_Primary_کلید_Index(pIdx) ){
    nField = pIdx->n_کلید_Col;
  }وگرنه{
    nField = pIdx->nColumn;
  }
  nField = MIN(pRec->nField, nField);
  iCol = 0;
  iSample = pIdx->nSample * nField;
  انجامبده{
    صحیح iSamp;                    /* Index in aSample[] of test sample */
    صحیح n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / nField;
    اگر( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      برای(n=(iTest % nField) + 1; n<nField; n++){
        اگر( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) بشکن;
      }
    }وگرنه{
      n = iTest + 1;
    }

    pRec->nField = n;
    res = sqlite3VdbeRecordCompare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    اگر( res<0 ){
      iLower = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }وگرنه اگر( res==0 && n<nField ){
      iLower = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }وگرنه{
      iSample = iTest;
      iCol = n-1;
    }
  }وقتی( res && iMin<iSample );
  i = iSample / nField;

#ifdef SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  اگر( pParse->db->mallocFailed==0 ){
    اگر( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      أكد( i<pIdx->nSample );
      أكد( iCol==nField-1 );
      pRec->nField = nField;
      أكد( 0==sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }وگرنه{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      أكد( i<=pIdx->nSample && i>=0 );
      pRec->nField = iCol+1;
      أكد( i==pIdx->nSample
           || sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      اگر( iCol>0 ){
        pRec->nField = iCol;
        أكد( sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      اگر( i>0 ){
        pRec->nField = nField;
        أكد( sqlite3VdbeRecordCompare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#endif /* ifdef SQLITE_DEBUG */

  اگر( res==0 ){
    /* Record pRec is equal to sample i */
    أكد( iCol==nField-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }وگرنه{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt iUpper, iGap;
    اگر( i>=pIdx->nSample ){
      iUpper = pIdx->nRowEst0;
    }وگرنه{
      iUpper = aSample[i].anLt[iCol];
    }

    اگر( iLower>=iUpper ){
      iGap = 0;
    }وگرنه{
      iGap = iUpper - iLower;
    }
    اگر( roundUp ){
      iGap = (iGap*2)/3;
    }وگرنه{
      iGap = iGap/3;
    }
    aStat[0] = iLower + iGap;
    aStat[1] = pIdx->aAvgEq[nField-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->nField = nField;
  بازگشت i;
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
ایستا LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew){
  LogEst nRet = nNew;
  اگر( pTerm ){
    اگر( pTerm->truthProb<=0 ){
      nRet += pTerm->truthProb;
    }وگرنه اگر( (pTerm->wtFlags & TERM_VNULL)==0 ){
      nRet -= 20;        أكد( 20==sqlite3LogEst(4) );
    }
  }
  بازگشت nRet;
}


#ifdef SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
نویسه sqlite3IndexColumnAffinity(sqlite3 *db, Index *pIdx, صحیح iCol){
  أكد( iCol>=0 && iCol<pIdx->nColumn );
  اگر( !pIdx->zColAff ){
    اگر( sqlite3IndexAffinityStr(db, pIdx)==0 ) بازگشت SQLITE_AFF_BLOB;
  }
  أكد( pIdx->zColAff[iCol]!=0 );
  بازگشت pIdx->zColAff[iCol];
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
ایستا صحیح whereRangeSkipScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  صحیح *pbDone          /* Set to true if at least one expr. value extracted */
){
  Index *p = pLoop->u.btree.pIndex;
  صحیح nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  صحیح nLower = -1;
  صحیح nUpper = p->nSample+1;
  صحیح rc = SQLITE_OK;
  u8 aff = sqlite3IndexColumnAffinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_مقدار *p1 = 0;          /* Value extracted from pLower */
  sqlite3_مقدار *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_مقدار *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  اگر( pLower ){
    rc = sqlite3_Stat4_مقدار_From_عبارت(pParse, pLower->p_عبارت->p_راست, aff, &p1);
    nLower = 0;
  }
  اگر( pUpper && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_مقدار_From_عبارت(pParse, pUpper->p_عبارت->p_راست, aff, &p2);
    nUpper = p2 ? 0 : p->nSample;
  }

  اگر( p1 || p2 ){
    صحیح i;
    صحیح nDiff;
    برای(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      اگر( rc==SQLITE_OK && p1 ){
        صحیح res = sqlite3MemCompare(p1, pVal, pColl);
        اگر( res>=0 ) nLower++;
      }
      اگر( rc==SQLITE_OK && p2 ){
        صحیح res = sqlite3MemCompare(p2, pVal, pColl);
        اگر( res>=0 ) nUpper++;
      }
    }
    nDiff = (nUpper - nLower);
    اگر( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    اگر( nDiff!=1 || pUpper==0 || pLower==0 ){
      صحیح nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->nOut -= nAdjust;
      *pbDone = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           nLower, nUpper, nAdjust*-1, pLoop->nOut));
    }

  }وگرنه{
    أكد( *pbDone==0 );
  }

  sqlite3_مقدار_Free(p1);
  sqlite3_مقدار_Free(p2);
  sqlite3_مقدار_Free(pVal);

  بازگشت rc;
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
ایستا صحیح whereRangeScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  صحیح rc = SQLITE_OK;
  صحیح nOut = pLoop->nOut;
  LogEst nNew;

#ifdef SQLITE_ENABLE_STAT4
  Index *p = pLoop->u.btree.pIndex;
  صحیح nEq = pLoop->u.btree.nEq;

  اگر( p->nSample>0 && ALWAYS(nEq<p->nSampleCol)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    اگر( nEq==pBuilder->nRecValid ){
      UnpackedRecord *pRec = pBuilder->pRec;
      tRowcnt a[2];
      صحیح nBtm = pLoop->u.btree.nBtm;
      صحیح n_بالاترین = pLoop->u.btree.n_بالاترین;

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
      صحیح iLwrIdx = -2;   /* aSample[] for the lower bound */
      صحیح iUprIdx = -1;   /* aSample[] for the upper bound */

      اگر( pRec ){
        testcase( pRec->nField!=pBuilder->nRecValid );
        pRec->nField = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      اگر( nEq==0 ){
        iLower = 0;
        iUpper = p->nRowEst0;
      }وگرنه{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_کلید_Stats(pParse, p, pRec, 0, a);
        iLower = a[0];
        iUpper = a[0] + a[1];
      }

      أكد( pLower==0 || (pLower->eOperator & (WO_GT|WO_GE))!=0 );
      أكد( pUpper==0 || (pUpper->eOperator & (WO_LT|WO_LE))!=0 );
      أكد( p->aSortOrder!=0 );
      اگر( p->aSortOrder[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(WhereTerm*, pLower, pUpper);
        SWAP(صحیح, nBtm, n_بالاترین);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      اگر( pLower ){
        صحیح n;                    /* Values extracted from pExpr */
        عبارت *p_عبارت = pLower->p_عبارت->p_راست;
        rc = sqlite3_Stat4_Probe_Set_مقدار(pParse, p, &pRec, p_عبارت, nBtm, nEq, &n);
        اگر( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          اگر( sqlite3_عبارت_Vector_Size(p_عبارت)>n ) mask = (WO_LE|WO_LT);
          iLwrIdx = where_کلید_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((pLower->eOperator & mask) ? a[1] : 0);
          اگر( iNew>iLower ) iLower = iNew;
          nOut--;
          pLower = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      اگر( pUpper ){
        صحیح n;                    /* Values extracted from pExpr */
        عبارت *p_عبارت = pUpper->p_عبارت->p_راست;
        rc = sqlite3_Stat4_Probe_Set_مقدار(pParse, p, &pRec, p_عبارت, n_بالاترین, nEq, &n);
        اگر( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          اگر( sqlite3_عبارت_Vector_Size(p_عبارت)>n ) mask = (WO_LE|WO_LT);
          iUprIdx = where_کلید_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((pUpper->eOperator & mask) ? a[1] : 0);
          اگر( iNew<iUpper ) iUpper = iNew;
          nOut--;
          pUpper = 0;
        }
      }

      pBuilder->pRec = pRec;
      اگر( rc==SQLITE_OK ){
        اگر( iUpper>iLower ){
          nNew = sqlite3LogEst(iUpper - iLower);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          اگر( iLwrIdx==iUprIdx ){ nNew -= 20; }
          أكد( 20==sqlite3LogEst(4) );
        }وگرنه{
          nNew = 10;        أكد( 10==sqlite3LogEst(2) );
        }
        اگر( nNew<nOut ){
          nOut = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)iLower, (u32)iUpper, nOut));
      }
    }وگرنه{
      صحیح bDone = 0;
      rc = whereRangeSkipScanEst(pParse, pLower, pUpper, pLoop, &bDone);
      اگر( bDone ) بازگشت rc;
    }
  }
#وگرنه
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pBuilder);
  أكد( pLower || pUpper );
#endif
  أكد( pUpper==0 || (pUpper->wtFlags & TERM_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  اگر( pLower && pLower->truthProb>0 && pUpper && pUpper->truthProb>0 ){
    nNew -= 20;
  }

  nOut -= (pLower!=0) + (pUpper!=0);
  اگر( nNew<10 ) nNew = 10;
  اگر( nNew<nOut ) nOut = nNew;
#اگر defined(WHERETRACE_ENABLED)
  اگر( pLoop->nOut>nOut ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->nOut, nOut));
  }
#endif
  pLoop->nOut = (LogEst)nOut;
  بازگشت rc;
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
ایستا صحیح whereEqualScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  عبارت *p_عبارت,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  صحیح nEq = pBuilder->pNew->u.btree.nEq;
  UnpackedRecord *pRec = pBuilder->pRec;
  صحیح rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  صحیح bOk;

  أكد( nEq>=1 );
  أكد( nEq<=p->nColumn );
  أكد( p->aSample!=0 );
  أكد( p->nSample>0 );
  أكد( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  اگر( pBuilder->nRecValid<(nEq-1) ){
    بازگشت SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  اگر( nEq>=p->nColumn ){
    *pnRow = 1;
    بازگشت SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_Set_مقدار(pParse, p, &pRec, p_عبارت, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  اگر( rc!=SQLITE_OK ) بازگشت rc;
  اگر( bOk==0 ) بازگشت SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_کلید_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (صحیح)a[1]));
  *pnRow = a[1];
 
  بازگشت rc;
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
ایستا صحیح whereInScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  عبارت_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  i64 nRow0 = sqlite3LogEstToInt(p->aiRowLogEst[0]);
  صحیح nRecValid = pBuilder->nRecValid;
  صحیح rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  صحیح i;                  /* Loop counter */

  أكد( p->aSample!=0 );
  برای(i=0; rc==SQLITE_OK && i<pList->n_عبارت; i++){
    nEst = nRow0;
    rc = whereEqualScanEst(pParse, pBuilder, pList->a[i].p_عبارت, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  اگر( rc==SQLITE_OK ){
    اگر( nRowEst > (tRowcnt)nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", nRowEst));
  }
  أكد( pBuilder->nRecValid==nRecValid );
  بازگشت rc;
}
#endif /* SQLITE_ENABLE_STAT4 */


#اگر defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
تهی sqlite3WhereTermPrint(WhereTerm *pTerm, صحیح iTerm){
  اگر( pTerm==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", iTerm);
  }وگرنه{
    نویسه zType[8];
    نویسه z_چپ[50];
    memcpy(zType, "....", 5);
    اگر( pTerm->wtFlags & TERM_VIRTUAL ) zType[0] = 'V';
    اگر( pTerm->eOperator & WO_EQUIV  ) zType[1] = 'E';
    اگر( عبارت_Has_Property(pTerm->p_عبارت, EP_OuterON) ) zType[2] = 'L';
    اگر( pTerm->wtFlags & TERM_CODED  ) zType[3] = 'C';
    اگر( pTerm->eOperator & WO_SINGLE ){
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(اندازه(z_چپ),z_چپ,"left={%d:%d}",
                       pTerm->چپ_Cursor, pTerm->u.x.چپ_Column);
    }وگرنه اگر( (pTerm->eOperator & WO_OR)!=0 && pTerm->u.pOrInfo!=0 ){
      sqlite3_snprintf(اندازه(z_چپ),z_چپ,"indexable=0x%llx",
                       pTerm->u.pOrInfo->indexable);
    }وگرنه{
      sqlite3_snprintf(اندازه(z_چپ),z_چپ,"left=%d", pTerm->چپ_Cursor);
    }
    iTerm = pTerm->iTerm = MAX(iTerm,pTerm->iTerm);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       iTerm, pTerm, zType, z_چپ, pTerm->eOperator, pTerm->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    اگر( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        pTerm->truthProb, (u64)pTerm->prereqAll, (u64)pTerm->prereq_راست);
    }
    اگر( (pTerm->eOperator & (WO_OR|WO_AND))==0 && pTerm->u.x.iField ){
      sqlite3DebugPrintf(" iField=%d", pTerm->u.x.iField);
    }
    اگر( pTerm->iParent>=0 ){
      sqlite3DebugPrintf(" iParent=%d", pTerm->iParent);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_درخت_View_عبارت(0, pTerm->p_عبارت, 0);
  }
}
تهی sqlite3ShowWhereTerm(WhereTerm *pTerm){
  sqlite3WhereTermPrint(pTerm, 0);
}
#endif

#ifdef WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
تهی sqlite3WhereClausePrint(WhereClause *pWC){
  صحیح i;
  برای(i=0; i<pWC->nTerm; i++){
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
تهی sqlite3WhereLoopPrint(ثابت WhereLoop *p, ثابت WhereClause *pWC){
  WhereInfo *pWInfo;
  اگر( pWC ){
    صحیح nb;
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
  }وگرنه{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->maskSelf, p->prereq & 0xfff, p->cId, p->iTab);
  }
  اگر( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ثابت نویسه *zName;
    اگر( p->u.btree.pIndex && (zName = p->u.btree.pIndex->zName)!=0 ){
      اگر( strncmp(zName, "sqlite_autoindex_", 17)==0 ){
        صحیح i = sqlite3Strlen30(zName) - 1;
        وقتی( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }وگرنه{
      sqlite3DebugPrintf("%20s","");
    }
  }وگرنه{
    نویسه *z;
    اگر( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_رقم, p->u.vtab.idxStr, p->u.vtab.omitMask);
    }وگرنه{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_رقم, p->u.vtab.omitMask);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  اگر( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }وگرنه{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  اگر( pWInfo && pWInfo->bStarUsed && p->rStarDelta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->rSetup, p->rRun, p->nOut, p->rStarDelta);
  }وگرنه{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->rSetup, p->rRun, p->nOut);
  }
  اگر( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    صحیح i;
    برای(i=0; i<p->nLTerm; i++){
      sqlite3WhereTermPrint(p->aLTerm[i], i);
    }
  }
}
تهی sqlite3ShowWhereLoop(ثابت WhereLoop *p){
  اگر( p ) sqlite3WhereLoopPrint(p, 0);
}
تهی sqlite3ShowWhereLoopList(ثابت WhereLoop *p){
  وقتی( p ){
    sqlite3ShowWhereLoop(p);
    p = p->pNextLoop;
  }
}
#endif

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
ایستا تهی whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = ArraySize(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
ایستا تهی whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  اگر( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_INDEX) ){
    اگر( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }وگرنه اگر( (p->wsFlags & WHERE_AUTO_INDEX)!=0 && p->u.btree.pIndex!=0 ){
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
ایستا تهی whereLoopClear(sqlite3 *db, WhereLoop *p){
  اگر( p->aLTerm!=p->aLTermSpace ){
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
ایستا صحیح whereLoopResize(sqlite3 *db, WhereLoop *p, صحیح n){
  WhereTerm **paNew;
  اگر( p->nLSlot>=n ) بازگشت SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, اندازه(p->aLTerm[0])*n);
  اگر( paNew==0 ) بازگشت SQLITE_NOMEM_BKPT;
  memcpy(paNew, p->aLTerm, اندازه(p->aLTerm[0])*p->nLSlot);
  اگر( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  بازگشت SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
ایستا صحیح whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom){
  whereLoopClearUnion(db, pTo);
  اگر( pFrom->nLTerm > pTo->nLSlot
   && whereLoopResize(db, pTo, pFrom->nLTerm)
  ){
    memset(pTo, 0, WHERE_LOOP_XFER_SZ);
    بازگشت SQLITE_NOMEM_BKPT;
  }
  memcpy(pTo, pFrom, WHERE_LOOP_XFER_SZ);
  memcpy(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm*اندازه(pTo->aLTerm[0]));
  اگر( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }وگرنه اگر( (pFrom->wsFlags & WHERE_AUTO_INDEX)!=0 ){
    pFrom->u.btree.pIndex = 0;
  }
  بازگشت SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
ایستا تهی whereLoopDelete(sqlite3 *db, WhereLoop *p){
  أكد( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
ایستا تهی whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  أكد( pWInfo!=0 );
  أكد( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  وقتی( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->pNextLoop;
    whereLoopDelete(db, p);
  }
  وقتی( pWInfo->pMemToFree ){
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
ایستا صحیح whereLoopCheaperProperSubset(
  ثابت WhereLoop *pX,       /* First WhereLoop to compare */
  ثابت WhereLoop *pY        /* Compare against this WhereLoop */
){
  صحیح i, j;
  اگر( pX->rRun>pY->rRun && pX->nOut>pY->nOut ) بازگشت 0; /* (1d) and (2a) */
  أكد( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  اگر( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.pIndex==pY->u.btree.pIndex             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    بازگشت 1;  /* Case 1 is true */
  }
  اگر( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    بازگشت 0;                                            /* (2b) */
  }
  اگر( pY->nSkip > pX->nSkip ) بازگشت 0;                  /* (2d) */
  برای(i=pX->nLTerm-1; i>=0; i--){
    اگر( pX->aLTerm[i]==0 ) ادامه;
    برای(j=pY->nLTerm-1; j>=0; j--){
      اگر( pY->aLTerm[j]==pX->aLTerm[i] ) بشکن;
    }
    اگر( j<0 ) بازگشت 0;                                  /* (2c) */
  }
  اگر( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    بازگشت 0;                                            /* (2e) */
  }
  بازگشت 1;  /* Case 2 is true */
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
ایستا تهی whereLoopAdjustCost(ثابت WhereLoop *p, WhereLoop *pTemplate){
  اگر( (pTemplate->wsFlags & WHERE_INDEXED)==0 ) بازگشت;
  برای(; p; p=p->pNextLoop){
    اگر( p->iTab!=pTemplate->iTab ) ادامه;
    اگر( (p->wsFlags & WHERE_INDEXED)==0 ) ادامه;
    اگر( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut,
                       MIN(p->rRun, pTemplate->rRun),
                       MIN(p->nOut - 1, pTemplate->nOut)));
      pTemplate->rRun = MIN(p->rRun, pTemplate->rRun);
      pTemplate->nOut = MIN(p->nOut - 1, pTemplate->nOut);
    }وگرنه اگر( whereLoopCheaperProperSubset(pTemplate, p) ){
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
ایستا WhereLoop **whereLoopFindLesser(
  WhereLoop **ppPrev,
  ثابت WhereLoop *pTemplate
){
  WhereLoop *p;
  برای(p=(*ppPrev); p; ppPrev=&p->pNextLoop, p=*ppPrev){
    اگر( p->iTab!=pTemplate->iTab || p->iSortIdx!=pTemplate->iSortIdx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      ادامه;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    أكد( p->rSetup==0 || pTemplate->rSetup==0
                 || p->rSetup==pTemplate->rSetup );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    أكد( p->rSetup>=pTemplate->rSetup );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    اگر( (p->wsFlags & WHERE_AUTO_INDEX)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_INDEXED)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      بشکن;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    اگر( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->rSetup<=pTemplate->rSetup                  /* (2a) */
     && p->rRun<=pTemplate->rRun                      /* (2b) */
     && p->nOut<=pTemplate->nOut                      /* (2c) */
    ){
      بازگشت 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    اگر( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->rRun>=pTemplate->rRun                             /* (2a) */
     && p->nOut>=pTemplate->nOut                             /* (2b) */
    ){
      أكد( p->rSetup>=pTemplate->rSetup ); /* SETUP-INVARIANT above */
      بشکن;   /* Cause p to be overwritten by pTemplate */
    }
  }
  بازگشت ppPrev;
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
ایستا صحیح where_Loop_درج(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  صحیح rc;

  /* Stop the search once we hit the query planner search limit */
  اگر( pBuilder->iPlanLimit==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    اگر( pBuilder->pOrSet ) pBuilder->pOrSet->n = 0;
    بازگشت SQLITE_DONE;
  }
  pBuilder->iPlanLimit--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  اگر( pBuilder->pOrSet!=0 ){
    اگر( pTemplate->nLTerm ){
#اگر WHERETRACE_ENABLED
      u16 n = pBuilder->pOrSet->n;
      صحیح x =
#endif
      where_Or_درج(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun,
                                    pTemplate->nOut);
#اگر WHERETRACE_ENABLED /* 0x8 */
      اگر( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#endif
    }
    بازگشت SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  اگر( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#اگر WHERETRACE_ENABLED /* 0x8 */
    اگر( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#endif
    بازگشت SQLITE_OK; 
  }وگرنه{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#اگر WHERETRACE_ENABLED /* 0x8 */
  اگر( sqlite3WhereTrace & 0x8 ){
    اگر( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }وگرنه{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#endif
  اگر( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, اندازه(WhereLoop));
    اگر( p==0 ) بازگشت SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->pNextLoop = 0;
  }وگرنه{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **ppTail = &p->pNextLoop;
    WhereLoop *pToDel;
    وقتی( *ppTail ){
      ppTail = whereLoopFindLesser(ppTail, pTemplate);
      اگر( ppTail==0 ) بشکن;
      pToDel = *ppTail;
      اگر( pToDel==0 ) بشکن;
      *ppTail = pToDel->pNextLoop;
#اگر WHERETRACE_ENABLED /* 0x8 */
      اگر( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(pToDel, pBuilder->pWC);
      }
#endif
      whereLoopDelete(db, pToDel);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  اگر( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    Index *pIndex = p->u.btree.pIndex;
    اگر( pIndex && pIndex->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.pIndex = 0;
    }
  }
  بازگشت rc;
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
ایستا صحیح عبارت_گره_Pattern_Length_Est(Walker *pWalker, عبارت *p_عبارت){
  اگر( p_عبارت->op==TK_STRING ){
    صحیح sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_عبارت->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    اگر( pWalker->eCode ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }وگرنه{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    وقتی( (c = *(z++))!=0 ){
      اگر( c==c3 ){
        اگر( *z ) z++;
        وقتی( *z && *z!=']' ) z++;
      }وگرنه اگر( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    اگر( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  بازگشت WRC_Continue;
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
ایستا صحیح estLikePatternLength(عبارت *p, u16 eCode){
  Walker w;
  w.u.sz = 0;
  w.eCode = eCode;
  w.x_عبارت_Callback = عبارت_گره_Pattern_Length_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.u.sz;
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
ایستا تهی whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst nRow            /* Number of rows in the entire table */
){
  WhereTerm *pTerm, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->maskSelf);
  صحیح i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  اگر( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->pOrderBy==0 ){
    بازگشت;
  }

  أكد( (pLoop->wsFlags & WHERE_AUTO_INDEX)==0 );
  برای(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
    أكد( pTerm!=0 );
    اگر( (pTerm->prereqAll & notAllowed)!=0 ) ادامه;
    اگر( (pTerm->prereqAll & pLoop->maskSelf)==0 ) ادامه;
    اگر( (pTerm->wtFlags & TERM_VIRTUAL)!=0 ) ادامه;
    برای(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      اگر( pX==0 ) ادامه;
      اگر( pX==pTerm ) بشکن;
      اگر( pX->iParent>=0 && (&pWC->a[pX->iParent])==pTerm ) بشکن;
    }
    اگر( j<0 ){
      sqlite3ProgressCheck(pWC->pWInfo->pParse);
      اگر( pLoop->maskSelf==pTerm->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        اگر( (pTerm->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_چپ|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      اگر( pTerm->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->nOut += pTerm->truthProb;
      }وگرنه{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        عبارت *p_Op_عبارت = pTerm->p_عبارت;
        pLoop->nOut--;
        اگر( (pTerm->eOperator&(WO_EQ|WO_IS))!=0
         && (pTerm->wtFlags & TERM_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          عبارت *p_راست = p_Op_عبارت->p_راست;
          Parse *pParse = pWC->pWInfo->pParse;
          صحیح k = 0;
          testcase( p_Op_عبارت->op==TK_IS );
          اگر( sqlite3_عبارت_Is_Integer(p_راست, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }وگرنه{
            k = 20;
          }
          اگر( iReduce<k ){
            pTerm->wtFlags |= TERM_HEURTRUTH;
            iReduce = k;
          }
        }وگرنه
        اگر( عبارت_Has_Property(p_Op_عبارت, EP_InfixFunc) 
         && p_Op_عبارت->op==TK_FUNCTION
        ){
          صحیح eOp;
          أكد( عبارت_Use_XList(p_Op_عبارت) );
          أكد( p_Op_عبارت->x.pList->n_عبارت>=2 );
          eOp = sqlite3_عبارت_Is_Like_Operator(p_Op_عبارت);
          اگر( ALWAYS(eOp>0) ){
            صحیح szPattern;
            عبارت *pRHS = p_Op_عبارت->x.pList->a[0].p_عبارت;
            eOp = eOp==SQLITE_INDEX_CONSTRAINT_LIKE;
            szPattern = estLikePatternLength(pRHS, eOp);
            اگر( szPattern>0 ){
              pLoop->nOut -= szPattern*2;
            }
          }
        }
      }
    }
  }
  اگر( pLoop->nOut > nRow-iReduce ){
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
ایستا صحیح whereRangeVectorLen(
  Parse *pParse,       /* Parsing context */
  صحیح iCur,            /* Cursor open on pIdx */
  Index *pIdx,         /* The index to be used for a inequality constraint */
  صحیح nEq,             /* Number of prior equality constraints on same index */
  WhereTerm *pTerm     /* The vector inequality constraint */
){
  صحیح nCmp = sqlite3_عبارت_Vector_Size(pTerm->p_عبارت->p_چپ);
  صحیح i;

  nCmp = MIN(nCmp, (pIdx->nColumn - nEq));
  برای(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    نویسه aff;                     /* Comparison affinity */
    نویسه idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    عبارت *pLhs, *pRhs;

    أكد( عبارت_Use_XList(pTerm->p_عبارت->p_چپ) );
    pLhs = pTerm->p_عبارت->p_چپ->x.pList->a[i].p_عبارت;
    pRhs = pTerm->p_عبارت->p_راست;
    اگر( عبارت_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_عبارت;
    }وگرنه{
      pRhs = pRhs->x.pList->a[i].p_عبارت;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    اگر( pLhs->op!=TK_COLUMN
     || pLhs->iTable!=iCur
     || pLhs->iColumn!=pIdx->aiColumn[i+nEq]
     || pIdx->aSortOrder[i+nEq]!=pIdx->aSortOrder[nEq]
    ){
      بشکن;
    }

    testcase( pLhs->iColumn==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_عبارت_Affinity(pLhs));
    idxaff = sqlite3TableColumnAffinity(pIdx->pTable, pLhs->iColumn);
    اگر( aff!=idxaff ) بشکن;

    اگر( عبارت_Has_Property(pTerm->p_عبارت, EP_Commuted) ) SWAP(عبارت*, pRhs, pLhs);
    pColl = sqlite3_دودویی_Compare_Coll_Seq(pParse, pLhs, pRhs);
    اگر( pColl==0 ) بشکن;
    اگر( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) بشکن;
  }
  بازگشت i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#ifdef SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#وگرنه
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
ایستا صحیح whereLoopAddBtreeIndex(
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
  صحیح opMask;                     /* Valid operators for constraints */
  WhereScan scan;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_بالاترین;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_nOut;              /* Original value of pNew->nOut */
  صحیح rc = SQLITE_OK;             /* Return code */
  LogEst rSize;                   /* Number of rows in the table */
  LogEst rLogSize;                /* Logarithm of table size */
  WhereTerm *p_بالاترین = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  أكد( db->mallocFailed==0 || pParse->nErr>0 );
  اگر( pParse->nErr ){
    بازگشت pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->pTable->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->rRun));

  أكد( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pNew->wsFlags & WHERE_بالاترین_LIMIT)==0 );
  اگر( pNew->wsFlags & WHERE_BTM_LIMIT ){
    opMask = WO_LT|WO_LE;
  }وگرنه{
    أكد( pNew->u.btree.nBtm==0 );
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  اگر( pProbe->bUnordered ){
    opMask &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  أكد( pNew->u.btree.nEq<pProbe->nColumn );
  أكد( pNew->u.btree.nEq<pProbe->n_کلید_Col
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_بالاترین = pNew->u.btree.n_بالاترین;
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
  برای(; rc==SQLITE_OK && pTerm!=0; pTerm = whereScanNext(&scan)){
    u16 eOp = pTerm->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst nOutUnadjusted;        /* nOut before IN() and WHERE adjustments */
    صحیح nIn = 0;
#ifdef SQLITE_ENABLE_STAT4
    صحیح nRecValid = pBuilder->nRecValid;
#endif
    اگر( (eOp==WO_ISNULL || (pTerm->wtFlags&TERM_VNULL)!=0)
     && indexColumnNotNull(pProbe, saved_nEq)
    ){
      ادامه; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    اگر( pTerm->prereq_راست & pNew->maskSelf ) ادامه;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    اگر( pTerm->wtFlags & TERM_LIKEOPT && pTerm->eOperator==WO_LT ) ادامه;

    اگر( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0
     && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
    ){
      ادامه;
    }
    اگر( IsUniqueIndex(pProbe) && saved_nEq==pProbe->n_کلید_Col-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }وگرنه{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_INDEXED;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_بالاترین = saved_n_بالاترین;
    pNew->nLTerm = saved_nLTerm;
    اگر( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       بشکن; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereq_راست) & ~pNew->maskSelf;

    أكد( nInMul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0
        || (pNew->wsFlags & WHERE_COLUMN_IN)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    اگر( eOp & WO_IN ){
      عبارت *p_عبارت = pTerm->p_عبارت;
      اگر( عبارت_Use_XSelect(p_عبارت) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        صحیح i;
        صحیح bRedundant = 0;
        nIn = 46;  أكد( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        برای(i=0; i<pNew->nLTerm-1; i++){
          اگر( pNew->aLTerm[i] && pNew->aLTerm[i]->p_عبارت==p_عبارت ){
            nIn = 0;
            اگر( pNew->aLTerm[i]->u.x.iField == pTerm->u.x.iField ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        اگر( bRedundant ){
          pNew->nLTerm--;
          ادامه;
        }
      }وگرنه اگر( ALWAYS(p_عبارت->x.pList && p_عبارت->x.pList->n_عبارت) ){
        /* "x IN (value, value, ...)" */
        nIn = sqlite3LogEst(p_عبارت->x.pList->n_عبارت);
      }
      اگر( pProbe->hasStat1 && rLogSize>=10 ){
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
        اگر( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, nIn, rLogSize, x));
        }وگرنه اگر( nInMul<2 && OptimizationEnabled(db, SQLITE_SeekScan) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, nIn, rLogSize, x, nInMul));
          pNew->wsFlags |= WHERE_IN_SEEKSCAN;
        }وگرنه{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, nIn, rLogSize, x, nInMul));
          ادامه;
        }
      }
      pNew->wsFlags |= WHERE_COLUMN_IN;
    }وگرنه اگر( eOp & (WO_EQ|WO_IS) ){
      صحیح iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      أكد( saved_nEq==pNew->u.btree.nEq );
      اگر( iCol==XN_ROWID
       || (iCol>=0 && nInMul==0 && saved_nEq==pProbe->n_کلید_Col-1)
      ){
        اگر( iCol==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_کلید_Col==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }وگرنه{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      اگر( scan.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }وگرنه اگر( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }وگرنه{
      صحیح nVecLen = whereRangeVectorLen(
          pParse, pSrc->iCursor, pProbe, saved_nEq, pTerm
      );
      اگر( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_LIMIT;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = pTerm;
        p_بالاترین = 0;
        اگر( pTerm->wtFlags & TERM_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_بالاترین = &pTerm[1];
          أكد( (p_بالاترین-(pTerm->pWC->a))<pTerm->pWC->nTerm );
          أكد( p_بالاترین->wtFlags & TERM_LIKEOPT );
          أكد( p_بالاترین->eOperator==WO_LT );
          اگر( whereLoopResize(db, pNew, pNew->nLTerm+1) ) بشکن; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_بالاترین;
          pNew->wsFlags |= WHERE_بالاترین_LIMIT;
          pNew->u.btree.n_بالاترین = 1;
        }
      }وگرنه{
        أكد( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_بالاترین_LIMIT;
        pNew->u.btree.n_بالاترین = nVecLen;
        p_بالاترین = pTerm;
        pBtm = (pNew->wsFlags & WHERE_BTM_LIMIT)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    أكد( pNew->nOut==saved_nOut );
    اگر( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      whereRangeScanEst(pParse, pBuilder, pBtm, p_بالاترین, pNew);
    }وگرنه{
      صحیح nEq = ++pNew->u.btree.nEq;
      أكد( eOp & (WO_ISNULL|WO_EQ|WO_IN|WO_IS) );

      أكد( pNew->nOut==saved_nOut );
      اگر( pTerm->truthProb<=0 && pProbe->aiColumn[saved_nEq]>=0 ){
        أكد( (eOp & WO_IN) || nIn==0 );
        testcase( eOp & WO_IN );
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      }وگرنه{
#ifdef SQLITE_ENABLE_STAT4
        tRowcnt nOut = 0;
        اگر( nInMul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->nSampleCol)
         && ((eOp & WO_IN)==0 || عبارت_Use_XList(pTerm->p_عبارت))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          عبارت *p_عبارت = pTerm->p_عبارت;
          اگر( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = whereEqualScanEst(pParse, pBuilder, p_عبارت->p_راست, &nOut);
          }وگرنه{
            rc = whereInScanEst(pParse, pBuilder, p_عبارت->x.pList, &nOut);
          }
          اگر( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          اگر( rc!=SQLITE_OK ) بشکن;          /* Jump out of the pTerm loop */
          اگر( nOut ){
            pNew->nOut = sqlite3LogEst(nOut);
            اگر( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->nOut+10 > pProbe->aiRowLogEst[0]
            ){
#اگر WHERETRACE_ENABLED /* 0x01 */
              اگر( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3WhereTermPrint(pTerm, 999);
              }
#endif
              pTerm->wtFlags |= TERM_HIGHTRUTH;
              اگر( pTerm->wtFlags & TERM_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            اگر( pNew->nOut>saved_nOut ) pNew->nOut = saved_nOut;
            pNew->nOut -= nIn;
          }
        }
        اگر( nOut==0 )
#endif
        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          اگر( eOp & WO_ISNULL ){
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
    أكد( pSrc->pSTab->szTabRow>0 );
    اگر( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      rCostIdx = pNew->nOut + 16;
    }وگرنه{
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
    اگر( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    }
    ApplyCostMultiplier(pNew->rRun, pProbe->pTable->costMult);

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, rSize);
    اگر( pSrc->fg.fromExists ) pNew->nOut = 0;
    rc = where_Loop_درج(pBuilder, pNew);

    اگر( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->nOut = saved_nOut;
    }وگرنه{
      pNew->nOut = nOutUnadjusted;
    }

    اگر( (pNew->wsFlags & WHERE_بالاترین_LIMIT)==0
     && pNew->u.btree.nEq<pProbe->nColumn
     && (pNew->u.btree.nEq<pProbe->n_کلید_Col ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      اگر( pNew->u.btree.nEq>3 ){
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
  pNew->u.btree.n_بالاترین = saved_n_بالاترین;
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
  أكد( 42==sqlite3LogEst(18) );
  اگر( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_کلید_Col
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
  بازگشت rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
ایستا صحیح indexMightHelpWithOrderBy(
  WhereLoopBuilder *pBuilder,
  Index *pIndex,
  صحیح iCursor
){
  عبارت_List *pOB;
  عبارت_List *a_Col_عبارت;
  صحیح ii, jj;

  اگر( pIndex->bUnordered ) بازگشت 0;
  اگر( (pOB = pBuilder->pWInfo->pOrderBy)==0 ) بازگشت 0;
  برای(ii=0; ii<pOB->n_عبارت; ii++){
    عبارت *p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(pOB->a[ii].p_عبارت);
    اگر( NEVER(p_عبارت==0) ) ادامه;
    اگر( (p_عبارت->op==TK_COLUMN || p_عبارت->op==TK_AGG_COLUMN) 
     && p_عبارت->iTable==iCursor 
    ){
      اگر( p_عبارت->iColumn<0 ) بازگشت 1;
      برای(jj=0; jj<pIndex->n_کلید_Col; jj++){
        اگر( p_عبارت->iColumn==pIndex->aiColumn[jj] ) بازگشت 1;
      }
    }وگرنه اگر( (a_Col_عبارت = pIndex->a_Col_عبارت)!=0 ){
      برای(jj=0; jj<pIndex->n_کلید_Col; jj++){
        اگر( pIndex->aiColumn[jj]!=XN_عبارت ) ادامه;
        اگر( sqlite3_عبارت_Compare_Skip(p_عبارت,a_Col_عبارت->a[jj].p_عبارت,iCursor)==0 ){
          بازگشت 1;
        }
      }
    }
  }
  بازگشت 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
ایستا صحیح whereUsablePartialIndex(
  صحیح iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  عبارت *pWhere          /* The WHERE clause from the partial index */
){
  صحیح i;
  WhereTerm *pTerm;
  Parse *pParse;

  اگر( jointype & JT_LTORJ ) بازگشت 0;
  pParse = pWC->pWInfo->pParse;
  وقتی( pWhere->op==TK_AND ){
    اگر( !whereUsablePartialIndex(iTab,jointype,pWC,pWhere->p_چپ) ) بازگشت 0;
    pWhere = pWhere->p_راست;
  }
  برای(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    عبارت *p_عبارت;
    p_عبارت = pTerm->p_عبارت;
    اگر( (!عبارت_Has_Property(p_عبارت, EP_OuterON) || p_عبارت->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || عبارت_Has_Property(p_عبارت, EP_OuterON))
     && sqlite3_عبارت_Implies_عبارت(pParse, p_عبارت, pWhere, iTab)
     && !sqlite3_عبارت_Implies_عبارت(pParse, p_عبارت, pWhere, -1)
     && (pTerm->wtFlags & TERM_VNULL)==0
    ){
      بازگشت 1;
    }
  }
  بازگشت 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
ایستا صحیح عبارت_Is_Covered_By_Index(
  ثابت عبارت *p_عبارت,
  ثابت Index *pIdx,
  صحیح iTabCur
){
  صحیح i;
  برای(i=0; i<pIdx->nColumn; i++){
    اگر( pIdx->aiColumn[i]==XN_عبارت
     && sqlite3_عبارت_Compare(0, p_عبارت, pIdx->a_Col_عبارت->a[i].p_عبارت, iTabCur)==0
    ){
      بازگشت 1;
    }
  }
  بازگشت 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
تعریفنوع ساختار CoveringIndexCheck CoveringIndexCheck;
ساختار CoveringIndexCheck {
  Index *pIdx;       /* The index */
  صحیح iTabCur;       /* Cursor number for the corresponding table */
  u8 b_عبارت;          /* Uses an indexed expression */
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
ایستا صحیح whereIsCoveringIndexWalkCallback(Walker *pWalk, عبارت *p_عبارت){
  صحیح i;                    /* Loop counter */
  ثابت Index *pIdx;        /* The index of interest */
  ثابت i16 *aiColumn;      /* Columns contained in the index */
  u16 nColumn;              /* Number of columns in the index */
  CoveringIndexCheck *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  اگر( (p_عبارت->op==TK_COLUMN || p_عبارت->op==TK_AGG_COLUMN) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    اگر( p_عبارت->iTable!=pCk->iTabCur ) بازگشت WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    برای(i=0; i<nColumn; i++){
      اگر( aiColumn[i]==p_عبارت->iColumn ) بازگشت WRC_Continue;
    }
    pCk->bUnidx = 1;
    بازگشت WRC_Abort;
  }وگرنه اگر( pIdx->b_Has_عبارت
         && عبارت_Is_Covered_By_Index(p_عبارت, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_عبارت = 1;
    بازگشت WRC_Prune;
  }
  بازگشت WRC_Continue;
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
ایستا SQLITE_NOINLINE u32 whereIsCoveringIndex(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  Index *pIdx,           /* Index that is being tested */
  صحیح iTabCur            /* Cursor for the table being indexed */
){
  صحیح i, rc;
  ساختار CoveringIndexCheck ck;
  Walker w;
  اگر( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    بازگشت 0;
  }
  اگر( pIdx->b_Has_عبارت==0 ){
    برای(i=0; i<pIdx->nColumn; i++){
      اگر( pIdx->aiColumn[i]>=BMS-1 ) بشکن;
    }
    اگر( i>=pIdx->nColumn ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      بازگشت 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_عبارت = 0;
  ck.bUnidx = 0;
  memset(&w, 0, اندازه(w));
  w.x_عبارت_Callback = whereIsCoveringIndexWalkCallback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  اگر( ck.bUnidx ){
    rc = 0;
  }وگرنه اگر( ck.b_عبارت ){
    rc = WHERE_EXPRIDX;
  }وگرنه{
    rc = WHERE_IDX_ONLY;
  }
  بازگشت rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
ایستا تهی where_Indexed_عبارت_Cleanup(sqlite3 *db, تهی *pObject){
  Indexed_عبارت **pp = (Indexed_عبارت**)pObject;
  وقتی( *pp!=0 ){
    Indexed_عبارت *p = *pp;
    *pp = p->pIENext;
    sqlite3_عبارت_Delete(db, p->p_عبارت);
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
ایستا تهی where_Part_Idx_عبارت(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Partial index being processed */
  عبارت *pPart,                    /* WHERE clause being processed */
  Bitmask *pMask,                 /* Mask to clear bits in */
  صحیح iIdxCur,                    /* Cursor number for index */
  SrcItem *pItem                  /* The FROM clause entry for the table */
){
  أكد( pItem==0 || (pItem->fg.jointype & JT_راست)==0 );
  أكد( (pItem==0 || pMask==0) && (pMask!=0 || pItem!=0) );

  اگر( pPart->op==TK_AND ){
    where_Part_Idx_عبارت(pParse, pIdx, pPart->p_راست, pMask, iIdxCur, pItem);
    pPart = pPart->p_چپ;
  }

  اگر( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    عبارت *p_چپ = pPart->p_چپ;
    عبارت *p_راست = pPart->p_راست;
    Table *p_چپ_Tab;            /* Table of the left operand */
    Column *p_چپ_Col;           /* Column of the left operand */
    u8 aff_راست, aff_چپ;

    اگر( p_چپ->op!=TK_COLUMN ) بازگشت;
    اگر( !sqlite3_عبارت_Is_Constant(0, p_راست) ) بازگشت;
    اگر( !sqlite3_Is_دودویی(sqlite3_عبارت_Compare_Coll_Seq(pParse, pPart)) ) بازگشت;
    اگر( p_چپ->iColumn<0 ) بازگشت;
    p_چپ_Tab = pIdx->pTable;
    p_چپ_Col = &p_چپ_Tab->aCol[p_چپ->iColumn];
    aff_چپ = p_چپ_Col->affinity;
    أكد( SQLITE_AFF_BLOB<SQLITE_AFF_TEXT );
    اگر( aff_چپ<SQLITE_AFF_TEXT ) بازگشت;
    aff_راست = sqlite3_عبارت_Affinity(p_راست);
    اگر( aff_راست!=0 && aff_راست!=aff_چپ ) بازگشت;
    اگر( pItem ){
      sqlite3 *db = pParse->db;
      Indexed_عبارت *p = (Indexed_عبارت*)sqlite3DbMallocRaw(db, اندازه(*p));
      اگر( p ){
        صحیح bNullRow = (pItem->fg.jointype&(JT_چپ|JT_LTORJ))!=0;
        p_راست = sqlite3_عبارت_Dup(db, p_راست, 0);
        اگر( (p_چپ_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3ColumnType(p_چپ_Col, 0);
          أكد( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (صحیح)strlen(t.z);
          p->p_عبارت = sqlite3_عبارت_Alloc(db, TK_CAST, &t, 1);
          sqlite3_عبارت_Attach_Subtrees(db, p->p_عبارت, p_راست, 0);
        }وگرنه{
          p->p_عبارت = p_راست;
        }
        p->iDataCur = pItem->iCursor;
        p->iIdxCur = iIdxCur;
        p->iIdxCol = p_چپ->iColumn;
        p->bMaybeNullRow = bNullRow;
        p->pIENext = pParse->p_Idx_Part_عبارت;
        p->aff = aff_چپ;
        pParse->p_Idx_Part_عبارت = p;
        اگر( p->pIENext==0 ){
          تهی *pArg = (تهی*)&pParse->p_Idx_Part_عبارت;
          sqlite3ParserAddCleanup(pParse, where_Indexed_عبارت_Cleanup, pArg);
        }
      }
    }وگرنه اگر( p_چپ->iColumn<(BMS-1) ){
      *pMask &= ~((Bitmask)1 << p_چپ->iColumn);
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
ایستا صحیح whereLoopAddBtree(
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
  صحیح rc = SQLITE_OK;         /* Return code */
  صحیح iSortIdx;               /* Index number */
  صحیح b;                      /* A boolean value */
  LogEst rSize;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  Table *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  أكد( !IsVirtual(pSrc->pSTab) );

  اگر( pSrc->fg.isIndexedBy ){
    أكد( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }وگرنه اگر( !HasRowid(pTab) ){
    اگر( pSrc->fg.notIndexed ){
      pProbe = sqlite3_Primary_کلید_Index(pTab);
    }وگرنه{
      pProbe = pTab->pIndex;
    }
  }وگرنه{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    Index *pFirst;                  /* First of real indices on the table */
    memset(&sPk, 0, اندازه(Index));
    sPk.n_کلید_Col = 1;
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
  اگر( !pBuilder->pOrSet      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_راست_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_AutoIndex)!=0
   && !pSrc->fg.isIndexedBy  /* Has no INDEXED BY clause */
   && !pSrc->fg.notIndexed   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_راست)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst rLogSize;         /* Logarithm of the number of rows in the table */
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    rLogSize = estLog(rSize);
    برای(pTerm=pWC->a; rc==SQLITE_OK && pTerm<pWCEnd; pTerm++){
      اگر( pTerm->prereq_راست & pNew->maskSelf ) ادامه;
      اگر( termCanDriveIndex(pTerm, pSrc, 0) ){
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
        اگر( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->rSetup += 28;
        }وگرنه{
          pNew->rSetup -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        ApplyCostMultiplier(pNew->rSetup, pTab->costMult);
        اگر( pNew->rSetup<0 ) pNew->rSetup = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->nOut = 43;  أكد( 43==sqlite3LogEst(20) );
        pNew->rRun = sqlite3LogEstAdd(rLogSize,pNew->nOut);
        pNew->wsFlags = WHERE_AUTO_INDEX;
        pNew->prereq = mPrereq | pTerm->prereq_راست;
        rc = where_Loop_درج(pBuilder, pNew);
      }
    }
  }
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  برای(iSortIdx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.isIndexedBy||pSrc->fg.notIndexed)?0:pProbe->pNext,
         iSortIdx++
  ){
    اگر( pProbe->pPartIdxWhere!=0
     && !whereUsablePartialIndex(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      ادامه;  /* Partial index inappropriate for this query */
    }
    اگر( pProbe->bNoQuery ) ادامه;
    rSize = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_بالاترین = 0;
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
    أكد( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    اگر( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
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
#وگرنه
      pNew->rRun = rSize + 16;
#endif
      ApplyCostMultiplier(pNew->rRun, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, rSize);
      اگر( pSrc->fg.isSubquery ){
        اگر( pSrc->fg.viaCoroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        اگر( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.pOrderBy = pSrc->u4.pSubq->pSelect->pOrderBy;
        }
      }وگرنه اگر( pSrc->fg.fromExists ){
        pNew->nOut = 0;
      }
      rc = where_Loop_درج(pBuilder, pNew);
      pNew->nOut = rSize;
      اگر( rc ) بشکن;
    }وگرنه{
      Bitmask m;
      اگر( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
      }وگرنه{
        m = pSrc->colUsed & pProbe->colNotIdxed;
        اگر( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_عبارت(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_INDEXED;
        اگر( m==TOPBIT || (pProbe->b_Has_عبارت && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = whereIsCoveringIndex(pWInfo, pProbe, pSrc->iCursor);
          اگر( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            أكد( m!=0 );
          }وگرنه{
            m = 0;
            pNew->wsFlags |= isCov;
            اگر( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }وگرنه{
              أكد( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }وگرنه اگر( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
        }
      }

      /* Full scan via index */
      اگر( b
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
        اگر( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = rSize + 16;  /* Base cost:  N*3 */
          صحیح ii;
          صحیح iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          برای(ii=0; ii<pWC2->nTerm; ii++){
            WhereTerm *pTerm = &pWC2->a[ii];
            اگر( !sqlite3_عبارت_Covered_By_Index(pTerm->p_عبارت, iCur, pProbe) ){
              بشکن;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            اگر( pTerm->truthProb<=0 ){
              nLookup += pTerm->truthProb;
            }وگرنه{
              nLookup--;
              اگر( pTerm->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, nLookup);
        }
        ApplyCostMultiplier(pNew->rRun, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, rSize);
        اگر( (pSrc->fg.jointype & JT_راست)!=0 && pProbe->a_Col_عبارت ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }وگرنه{
          اگر( pSrc->fg.fromExists ) pNew->nOut = 0;
          rc = where_Loop_درج(pBuilder, pNew);
        }
        pNew->nOut = rSize;
        اگر( rc ) بشکن;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, 0);
    اگر( pBuilder->bldFlags1==SQLITE_BLDF1_INDEXED ){
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
  بازگشت rc;
}

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
ایستا صحیح isLimitTerm(WhereTerm *pTerm){
  أكد( pTerm->eOperator==WO_AUX || pTerm->eMatchOp==0 );
  بازگشت pTerm->eMatchOp>=SQLITE_INDEX_CONSTRAINT_LIMIT
      && pTerm->eMatchOp<=SQLITE_INDEX_CONSTRAINT_OFFSET;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
ایستا صحیح allConstraintsUsed(
  ساختار sqlite3_index_constraint_usage *aUsage, 
  صحیح nCons
){
  صحیح ii;
  برای(ii=0; ii<nCons; ii++){
    اگر( aUsage[ii].argvIndex<=0 ) بازگشت 0;
  }
  بازگشت 1;
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
ایستا صحیح whereLoopAddVirtualOne(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_index_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  صحیح *pbIn,                      /* OUT: True if plan uses an IN(...) op */
  صحیح *pbRetryLimit               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  ساختار sqlite3_index_constraint *pIdxCons;
  ساختار sqlite3_index_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  صحیح i;
  صحیح mxTerm;
  صحیح rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  صحیح nConstraint = pIdxInfo->nConstraint;

  أكد( (mUsable & mPrereq)==mPrereq );
  *pbIn = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(ساختار sqlite3_index_constraint**)&pIdxInfo->aConstraint;
  برای(i=0; i<nConstraint; i++, pIdxCons++){
    WhereTerm *pTerm = termFromWhereClause(pWC, pIdxCons->iTermOffset);
    pIdxCons->usable = 0;
    اگر( (pTerm->prereq_راست & mUsable)==pTerm->prereq_راست
     && (pTerm->eOperator & mExclude)==0
     && (pbRetryLimit || !isLimitTerm(pTerm))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, اندازه(pUsage[0])*nConstraint);
  أكد( pIdxInfo->needToFreeIdxStr==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_رقم = 0;
  pIdxInfo->orderByConsumed = 0;
  pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (دوگانه)2;
  pIdxInfo->estimatedRows = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->mHandleIn = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtabBestIndex(pParse, pSrc->pSTab, pIdxInfo);
  اگر( rc ){
    اگر( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      بازگشت SQLITE_OK;
    }
    بازگشت rc;
  }

  mxTerm = -1;
  أكد( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, اندازه(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, اندازه(pNew->u.vtab));
  pIdxCons = *(ساختار sqlite3_index_constraint**)&pIdxInfo->aConstraint;
  برای(i=0; i<nConstraint; i++, pIdxCons++){
    صحیح iTerm;
    اگر( (iTerm = pUsage[i].argvIndex - 1)>=0 ){
      WhereTerm *pTerm;
      صحیح j = pIdxCons->iTermOffset;
      اگر( iTerm>=nConstraint
       || j<0
       || (pTerm = termFromWhereClause(pWC, j))==0
       || pNew->aLTerm[iTerm]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_پیام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        بازگشت SQLITE_ERROR;
      }
      testcase( iTerm==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->nTerm-1 );
      pNew->prereq |= pTerm->prereq_راست;
      أكد( iTerm<pNew->nLSlot );
      pNew->aLTerm[iTerm] = pTerm;
      اگر( iTerm>mxTerm ) mxTerm = iTerm;
      testcase( iTerm==15 );
      testcase( iTerm==16 );
      اگر( pUsage[i].omit ){
        اگر( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=iTerm );
          pNew->u.vtab.omitMask |= 1<<iTerm;
        }وگرنه{
          testcase( i!=iTerm );
        }
        اگر( pTerm->eMatchOp==SQLITE_INDEX_CONSTRAINT_OFFSET ){
          pNew->u.vtab.bOmitOffset = 1;
        }
      }
      اگر( SMASKBIT32(i) & pHidden->mHandleIn ){
        pNew->u.vtab.mHandleIn |= MASKBIT32(iTerm);
      }وگرنه اگر( (pTerm->eOperator & WO_IN)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->orderByConsumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_INDEX_SCAN_UNIQUE;
        *pbIn = 1; أكد( (mExclude & WO_IN)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      أكد( pbRetryLimit || !isLimitTerm(pTerm) );
      أكد( !isLimitTerm(pTerm) || i>=nConstraint-2 );
      أكد( !isLimitTerm(pTerm) || i==nConstraint-1 || isLimitTerm(pTerm+1) );

      اگر( isLimitTerm(pTerm) && (*pbIn || !allConstraintsUsed(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pbRetryLimit = 1;
        بازگشت SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mxTerm+1;
  برای(i=0; i<=mxTerm; i++){
    اگر( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_پیام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      بازگشت SQLITE_ERROR;
    }
  }
  أكد( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_رقم = pIdxInfo->idx_رقم;
  pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
  pIdxInfo->needToFreeIdxStr = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ?
      pIdxInfo->nOrderBy : 0);
  pNew->u.vtab.b_Idx_رقم_Hex = (pIdxInfo->idxFlags&SQLITE_INDEX_SCAN_HEX)!=0;
  pNew->rSetup = 0;
  pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  اگر( pIdxInfo->idxFlags & SQLITE_INDEX_SCAN_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }وگرنه{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_درج(pBuilder, pNew);
  اگر( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pbIn, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  بازگشت rc;
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
ثابت نویسه *sqlite3_vtab_collation(sqlite3_index_info *pIdxInfo, صحیح iCons){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  ثابت نویسه *zRet = 0;
  اگر( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    صحیح iTerm = pIdxInfo->aConstraint[iCons].iTermOffset;
    عبارت *pX = termFromWhereClause(pHidden->pWC, iTerm)->p_عبارت;
    اگر( pX->p_چپ ){
      pC = sqlite3_عبارت_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_دودویی);
  }
  بازگشت zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
صحیح sqlite3_vtab_in(sqlite3_index_info *pIdxInfo, صحیح iCons, صحیح bHandle){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  اگر( m & pHidden->mIn ){
    اگر( bHandle==0 ){
      pHidden->mHandleIn &= ~m;
    }وگرنه اگر( bHandle>0 ){
      pHidden->mHandleIn |= m;
    }
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
صحیح sqlite3_vtab_rhs_مقدار(
  sqlite3_index_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  صحیح iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_مقدار **ppVal           /* Write value extracted here */
){
  HiddenIndexInfo *pH = (HiddenIndexInfo*)&pIdxInfo[1];
  sqlite3_مقدار *pVal = 0;
  صحیح rc = SQLITE_OK;
  اگر( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }وگرنه{
    اگر( pH->aRhs[iCons]==0 ){
      WhereTerm *pTerm = termFromWhereClause(
          pH->pWC, pIdxInfo->aConstraint[iCons].iTermOffset
      );
      rc = sqlite3_مقدار_From_عبارت(
          pH->pParse->db, pTerm->p_عبارت->p_راست, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  اگر( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  بازگشت rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
صحیح sqlite3_vtab_distinct(sqlite3_index_info *pIdxInfo){
  HiddenIndexInfo *pHidden = (HiddenIndexInfo*)&pIdxInfo[1];
  أكد( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  بازگشت pHidden->eDistinct;
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
تهی sqlite3VtabUsesAllSchemas(Parse *pParse){
  صحیح nDb = pParse->db->nDb;
  صحیح i;
  برای(i=0; i<nDb; i++){
    sqlite3CodeVerifySchema(pParse, i);
  }
  اگر( DbMaskNonZero(pParse->writeMask) ){
    برای(i=0; i<nDb; i++){
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
ایستا صحیح whereLoopAddVirtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  صحیح rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  SrcItem *pSrc;               /* The FROM clause term to search */
  sqlite3_index_info *p;       /* Object to pass to xBestIndex() */
  صحیح nConstraint;             /* Number of constraints in p */
  صحیح bIn;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask mBest;               /* Tables used by best possible plan */
  u16 mNoOmit;
  صحیح bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  أكد( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  أكد( IsVirtual(pSrc->pSTab) );
  p = allocateIndexInfo(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  اگر( p==0 ) بازگشت SQLITE_NOMEM_BKPT;
  pNew->rSetup = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  اگر( whereLoopResize(pParse->db, pNew, nConstraint) ){
    freeIndexInfo(pParse->db, p);
    بازگشت SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = whereLoopAddVirtualOne(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, &bRetry
  );
  اگر( bRetry ){
    أكد( rc==SQLITE_OK );
    rc = whereLoopAddVirtualOne(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  اگر( rc==SQLITE_OK && ((mBest = (pNew->prereq & ~mPrereq))!=0 || bIn) ){
    صحیح seenZero = 0;             /* True if a plan with no prereqs seen */
    صحیح seenZeroNoIN = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask mBestNoIn = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    اگر( bIn ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, ALLBITS, WO_IN, p, mNoOmit, &bIn, 0);
      أكد( bIn==0 );
      mBestNoIn = pNew->prereq & ~mPrereq;
      اگر( mBestNoIn==0 ){
        seenZero = 1;
        seenZeroNoIN = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    وقتی( rc==SQLITE_OK ){
      صحیح i;
      Bitmask mNext = ALLBITS;
      أكد( mNext>0 );
      برای(i=0; i<nConstraint; i++){
        صحیح iTerm = p->aConstraint[i].iTermOffset;
        Bitmask mThis = termFromWhereClause(pWC, iTerm)->prereq_راست & ~mPrereq;
        اگر( mThis>mPrev && mThis<mNext ) mNext = mThis;
      }
      mPrev = mNext;
      اگر( mNext==ALLBITS ) بشکن;
      اگر( mNext==mBest || mNext==mBestNoIn ) ادامه;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)mNext));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mNext|mPrereq, 0, p, mNoOmit, &bIn, 0);
      اگر( pNew->prereq==mPrereq ){
        seenZero = 1;
        اگر( bIn==0 ) seenZeroNoIN = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    اگر( rc==SQLITE_OK && seenZero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &bIn, 0);
      اگر( bIn==0 ) seenZeroNoIN = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    اگر( rc==SQLITE_OK && seenZeroNoIN==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, WO_IN, p, mNoOmit, &bIn, 0);
    }
  }

  freeIndexInfo(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  بازگشت rc;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
ایستا صحیح whereLoopAddOr(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  WhereTerm *pTerm, *pWCEnd;
  صحیح rc = SQLITE_OK;
  صحیح iCur;
  WhereClause tempWC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet sSum, sCur;
  SrcItem *pItem;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  memset(&sSum, 0, اندازه(sSum));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  اگر( pItem->fg.jointype & JT_راست ) بازگشت SQLITE_OK;

  برای(pTerm=pWC->a; pTerm<pWCEnd && rc==SQLITE_OK; pTerm++){
    اگر( (pTerm->eOperator & WO_OR)!=0
     && (pTerm->u.pOrInfo->indexable & pNew->maskSelf)!=0
     && (!عبارت_Has_Property(pTerm->p_عبارت, EP_OuterON)
          || pTerm->p_عبارت->w.iJoin==iCur)
    ){
      WhereClause * ثابت pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm * ثابت pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      صحیح once = 1;
      صحیح i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.pOrSet = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", pTerm));
      برای(pOrTerm=pOrWC->a; pOrTerm<pOrWCEnd; pOrTerm++){
        اگر( (pOrTerm->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        }وگرنه اگر( pOrTerm->چپ_Cursor==iCur ){
          tempWC.pWInfo = pWC->pWInfo;
          tempWC.pOuter = pWC;
          tempWC.op = TK_AND;
          tempWC.nTerm = 1;
          tempWC.nBase = 1;
          tempWC.a = pOrTerm;
          sSubBuild.pWC = &tempWC;
        }وگرنه{
          ادامه;
        }
        sCur.n = 0;
#ifdef WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (صحیح)(pOrTerm-pOrWC->a), pTerm, sSubBuild.pWC->nTerm));
        اگر( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
        اگر( IsVirtual(pItem->pSTab) ){
          rc = whereLoopAddVirtual(&sSubBuild, mPrereq, mUnusable);
        }وگرنه
#endif
        {
          rc = whereLoopAddBtree(&sSubBuild, mPrereq);
        }
        اگر( rc==SQLITE_OK ){
          rc = whereLoopAddOr(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_DONE );
        اگر( sCur.n==0 ){
          sSum.n = 0;
          بشکن;
        }وگرنه اگر( once ){
          whereOrMove(&sSum, &sCur);
          once = 0;
        }وگرنه{
          WhereOrSet sPrev;
          whereOrMove(&sPrev, &sSum);
          sSum.n = 0;
          برای(i=0; i<sPrev.n; i++){
            برای(j=0; j<sCur.n; j++){
              where_Or_درج(&sSum, sPrev.a[i].prereq | sCur.a[j].prereq,
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
      memset(&pNew->u, 0, اندازه(pNew->u));
      برای(i=0; rc==SQLITE_OK && i<sSum.n; i++){
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
        rc = where_Loop_درج(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", pTerm));
    }
  }
  بازگشت rc;
}

/*
** Add all WhereLoop objects for all tables
*/
ایستا صحیح whereLoopAddAll(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask mPrior = 0;
  صحیح iTab;
  SrcList *pTabList = pWInfo->pTabList;
  SrcItem *pItem;
  SrcItem *pEnd = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  صحیح rc = SQLITE_OK;
  صحیح bFirstPastRJ = 0;
  صحیح has_راست_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  أكد( pNew->nLTerm==0 );
  أكد( pNew->wsFlags==0 );
  أكد( pNew->nLSlot>=ArraySize(pNew->aLTermSpace) );
  أكد( pNew->aLTerm!=0 );

  pBuilder->iPlanLimit = SQLITE_QUERY_PLANNER_LIMIT;
  برای(iTab=0, pItem=pTabList->a; pItem<pEnd; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->iPlanLimit += SQLITE_QUERY_PLANNER_LIMIT_INCR;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    اگر( bFirstPastRJ
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
      اگر( pItem->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( pItem->fg.jointype & JT_LTORJ );
        testcase( pItem->fg.jointype & JT_CROSS );
        has_راست_Cross_Join = 1;
      }
      mPrereq |= mPrior;
      bFirstPastRJ = (pItem->fg.jointype & JT_راست)!=0;
    }وگرنه اگر( pItem->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      WhereTerm *pTerm;
      صحیح i;
      برای(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
        اگر( (pNew->maskSelf & pTerm->prereqAll)!=0 ){
          mPrereq |= (pTerm->prereqAll & (pNew->maskSelf-1));
        }
      }
    }وگرنه اگر( !has_راست_Cross_Join ){
      mPrereq = 0;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    اگر( IsVirtual(pItem->pSTab) ){
      SrcItem *p;
      برای(p=&pItem[1]; p<pEnd; p++){
        اگر( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mPrereq, mUnusable);
    }وگرنه
#endif /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = whereLoopAddBtree(pBuilder, mPrereq);
    }
    اگر( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = whereLoopAddOr(pBuilder, mPrereq, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    اگر( rc || db->mallocFailed ){
      اگر( rc==SQLITE_DONE ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }وگرنه{
        بشکن;
      }
    }
  }

  whereLoopClear(db, pNew);
  بازگشت rc;
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
ایستا SQLITE_NOINLINE صحیح wherePathMatchSubqueryOB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  صحیح iLoop,              /* Which level of the nested loop.  0==outermost */
  صحیح iCur,               /* Cursor used by the this loop */
  عبارت_List *pOrderBy,     /* The ORDER BY clause on the whole query */
  Bitmask *pRevMask,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  صحیح iOB;                /* Index into pOrderBy->a[] */
  صحیح jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  عبارت *pOBExpr;          /* Current term of outer ORDER BY */
  عبارت_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.pOrderBy;
  أكد( pSubOB!=0 );
  برای(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  برای(jSub=0; jSub<pSubOB->n_عبارت && iOB<pOrderBy->n_عبارت; jSub++, iOB++){
    اگر( pSubOB->a[jSub].u.x.iOrderByCol==0 ) بشکن;
    pOBExpr = pOrderBy->a[iOB].p_عبارت;
    اگر( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) بشکن;
    اگر( pOBExpr->iTable!=iCur ) بشکن;
    اگر( pOBExpr->iColumn!=pSubOB->a[jSub].u.x.iOrderByCol-1 ) بشکن;
    اگر( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = pOrderBy->a[iOB].fg.sortFlags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.sortFlags;   /* sortFlags for jSub */
      اگر( (sfSub & KEYINFO_ORDER_BIGNULL) != (sfOB & KEYINFO_ORDER_BIGNULL) ){
        بشکن;
      }
      revIdx = sfSub & KEYINFO_ORDER_DESC;
      اگر( jSub>0 ){
        اگر( (rev^revIdx)!=(sfOB & KEYINFO_ORDER_DESC) ){
          بشکن;
        }
      }وگرنه{
        rev = revIdx ^ (sfOB & KEYINFO_ORDER_DESC);
        اگر( rev ){
          اگر( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            بشکن;
          }
          *pRevMask |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  بازگشت jSub>0;
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
ایستا i8 wherePathSatisfiesOrderBy(
  WhereInfo *pWInfo,    /* The WHERE clause */
  عبارت_List *pOrderBy,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
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
  u16 n_کلید_Col;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 nOrderBy;         /* Number terms in the ORDER BY clause */
  صحیح iLoop;            /* Index of WhereLoop in pPath being processed */
  صحیح i, j;             /* Loop counters */
  صحیح iCur;             /* Cursor number for current WhereLoop */
  صحیح iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  WhereTerm *pTerm;     /* A single term of the WHERE clause */
  عبارت *pOBExpr;        /* An expression from the ORDER BY clause */
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

  أكد( pOrderBy!=0 );
  اگر( nLoop && OptimizationDisabled(db, SQLITE_OrderByIdxJoin) ) بازگشت 0;

  nOrderBy = pOrderBy->n_عبارت;
  testcase( nOrderBy==BMS-1 );
  اگر( nOrderBy>BMS-1 ) بازگشت 0;  /* Cannot optimize overly large ORDER BYs */
  isOrderDistinct = 1;
  obDone = MASKBIT(nOrderBy)-1;
  orderDistinctMask = 0;
  ready = 0;
  eqOpMask = WO_EQ | WO_IS | WO_ISNULL;
  اگر( wctrlFlags & (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eqOpMask |= WO_IN;
  }
  برای(iLoop=0; isOrderDistinct && obSat<obDone && iLoop<=nLoop; iLoop++){
    اگر( iLoop>0 ) ready |= pLoop->maskSelf;
    اگر( iLoop<nLoop ){
      pLoop = pPath->aLoop[iLoop];
      اگر( wctrlFlags & WHERE_ORDERBY_LIMIT ) ادامه;
    }وگرنه{
      pLoop = pLast;
    }
    اگر( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      اگر( pLoop->u.vtab.isOrdered && pWInfo->pOrderBy==pOrderBy ){
        obSat = obDone;
      }وگرنه{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        isOrderDistinct = 0;
      }
      بشکن;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    برای(i=0; i<nOrderBy; i++){
      اگر( MASKBIT(i) & obSat ) ادامه;
      pOBExpr = sqlite3_عبارت_Skip_Collate_And_Likely(pOrderBy->a[i].p_عبارت);
      اگر( NEVER(pOBExpr==0) ) ادامه;
      اگر( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) ادامه;
      اگر( pOBExpr->iTable!=iCur ) ادامه;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, eqOpMask, 0);
      اگر( pTerm==0 ) ادامه;
      اگر( pTerm->eOperator==WO_IN ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        أكد( wctrlFlags &
               (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        برای(j=0; j<pLoop->nLTerm && pTerm!=pLoop->aLTerm[j]; j++){}
        اگر( j>=pLoop->nLTerm ) ادامه;
      }
      اگر( (pTerm->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_عبارت_NNColl_Seq(pParse, pOrderBy->a[i].p_عبارت);
        ثابت عبارت *pCExpr = pTerm->p_عبارت;
        CollSeq *pColl2 = sqlite3_عبارت_Compare_Coll_Seq(pParse, pCExpr);
        نویسه aff_راست;
        أكد( pColl1 );
        اگر( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          ادامه;
        }
        aff_راست = sqlite3_عبارت_Affinity(pCExpr->p_راست);
        /* Left operand must be a column, hence always has affinity */
        أكد( sqlite3_عبارت_Affinity(pCExpr->p_چپ)!=0 );
        اگر( aff_راست!=0 && aff_راست!=sqlite3_عبارت_Affinity(pCExpr->p_چپ) ){
          ادامه;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    اگر( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      اگر( pLoop->wsFlags & WHERE_IPK ){
        اگر( pLoop->u.btree.pOrderBy
         && OptimizationEnabled(db, SQLITE_OrderBySubq)
         &&  wherePathMatchSubqueryOB(pLoop,iLoop,iCur,
                                     pOrderBy,pRevMask, &obSat, wctrlFlags)
        ){
          nColumn = 0;
          isOrderDistinct = 0;
        }وگرنه{
          nColumn = 1;
        }
        pIndex = 0;
        n_کلید_Col = 1;
      }وگرنه اگر( (pIndex = pLoop->u.btree.pIndex)==0 || pIndex->bUnordered ){
        بازگشت 0;
      }وگرنه{
        n_کلید_Col = pIndex->n_کلید_Col;
        nColumn = pIndex->nColumn;
        أكد( nColumn==n_کلید_Col+1 || !HasRowid(pIndex->pTable) );
        أكد( pIndex->aiColumn[nColumn-1]==XN_ROWID
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
      برای(j=0; j<nColumn; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        أكد( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        اگر( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          اگر( (eOp & eqOpMask)!=0 ){
            اگر( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( isOrderDistinct );
              isOrderDistinct = 0;
            }
            ادامه; 
          }وگرنه اگر( ALWAYS(eOp & WO_IN) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            عبارت *pX = pLoop->aLTerm[j]->p_عبارت;
            برای(i=j+1; i<pLoop->u.btree.nEq; i++){
              اگر( pLoop->aLTerm[i]->p_عبارت==pX ){
                أكد( (pLoop->aLTerm[i]->eOperator & WO_IN) );
                bOnce = 0;
                بشکن;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        اگر( pIndex ){
          iColumn = pIndex->aiColumn[j];
          revIdx = pIndex->aSortOrder[j] & KEYINFO_ORDER_DESC;
          اگر( iColumn==pIndex->pTable->iPKey ) iColumn = XN_ROWID;
        }وگرنه{
          iColumn = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        اگر( isOrderDistinct ){
          اگر( iColumn>=0
           && j>=pLoop->u.btree.nEq
           && pIndex->pTable->aCol[iColumn].notNull==0
          ){
            isOrderDistinct = 0;
          }
          اگر( iColumn==XN_عبارت ){
            isOrderDistinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        isMatch = 0;
        برای(i=0; bOnce && i<nOrderBy; i++){
          اگر( MASKBIT(i) & obSat ) ادامه;
          pOBExpr = sqlite3_عبارت_Skip_Collate_And_Likely(pOrderBy->a[i].p_عبارت);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          اگر( NEVER(pOBExpr==0) ) ادامه;
          اگر( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          اگر( iColumn>=XN_ROWID ){
            اگر( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) ادامه;
            اگر( pOBExpr->iTable!=iCur ) ادامه;
            اگر( pOBExpr->iColumn!=iColumn ) ادامه;
          }وگرنه{
            عبارت *p_Ix_عبارت = pIndex->a_Col_عبارت->a[j].p_عبارت;
            اگر( sqlite3_عبارت_Compare_Skip(pOBExpr, p_Ix_عبارت, iCur) ){
              ادامه;
            }
          }
          اگر( iColumn!=XN_ROWID ){
            pColl = sqlite3_عبارت_NNColl_Seq(pWInfo->pParse, pOrderBy->a[i].p_عبارت);
            اگر( sqlite3StrICmp(pColl->zName, pIndex->azColl[j])!=0 ) ادامه;
          }
          اگر( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.nDistinctCol = j+1;
          }
          isMatch = 1;
          بشکن;
        }
        اگر( isMatch && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          اگر( revSet ){
            اگر( (rev ^ revIdx)
                           != (pOrderBy->a[i].fg.sortFlags&KEYINFO_ORDER_DESC)
            ){
              isMatch = 0;
            }
          }وگرنه{
            rev = revIdx ^ (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC);
            اگر( rev ) *pRevMask |= MASKBIT(iLoop);
            revSet = 1;
          }
        }
        اگر( isMatch && (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL) ){
          اگر( j==pLoop->u.btree.nEq 
           && (j<n_کلید_Col||HasRowid(pIndex->pTable)||!Is_Primary_کلید_Index(pIndex))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_SORT;
          }وگرنه{
            isMatch = 0;
          }
        }
        اگر( isMatch ){
          اگر( iColumn==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }وگرنه{
          /* No match found */
          أكد( n_کلید_Col>0 );
          اگر( j<n_کلید_Col ){
            testcase( isOrderDistinct!=0 );
            isOrderDistinct = 0;
          }
          بشکن;
        }
      } /* end Loop over all index columns */
      اگر( distinctColumns ){
        testcase( isOrderDistinct==0 );
        isOrderDistinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    اگر( isOrderDistinct ){
      orderDistinctMask |= pLoop->maskSelf;
      برای(i=0; i<nOrderBy; i++){
        عبارت *p;
        Bitmask mTerm;
        اگر( MASKBIT(i) & obSat ) ادامه;
        p = pOrderBy->a[i].p_عبارت;
        mTerm = sqlite3_Where_عبارت_Usage(&pWInfo->sMaskSet,p);
        اگر( mTerm==0 && !sqlite3_عبارت_Is_Constant(0,p) ) ادامه;
        اگر( (mTerm&~orderDistinctMask)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  اگر( obSat==obDone ) بازگشت (i8)nOrderBy;
  اگر( !isOrderDistinct ){
    برای(i=nOrderBy-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      اگر( (obSat&m)==m ) بازگشت i;
    }
    بازگشت 0;
  }
  بازگشت -1;
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
صحیح sqlite3WhereIsSorted(WhereInfo *pWInfo){
  أكد( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  أكد( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  بازگشت pWInfo->sorted;
}

#ifdef WHERETRACE_ENABLED
/* For debugging use only: */
ایستا ثابت نویسه *wherePathName(WherePath *pPath, صحیح nLoop, WhereLoop *pLast){
  ایستا نویسه zName[65];
  صحیح i;
  برای(i=0; i<nLoop; i++){ zName[i] = pPath->aLoop[i]->cId; }
  اگر( pLast ) zName[i++] = pLast->cId;
  zName[i] = 0;
  بازگشت zName;
}
#endif

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
ایستا LogEst whereSortingCost(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst nRow,       /* Estimated number of rows to sort */
  صحیح nOrderBy,      /* Number of ORDER BY clause terms */
  صحیح nSorted        /* Number of initial ORDER BY terms naturally in order */
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
  أكد( pWInfo->pSelect!=0 );
  أكد( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->n_عبارت+59)/30);
  rSortCost = nRow + nCol;
  اگر( nSorted>0 ){
    /* Scale the result by (Y/X) */
    rSortCost += sqlite3LogEst((nOrderBy-nSorted)*100/nOrderBy) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  اگر( (pWInfo->wctrlFlags & WHERE_USE_LIMIT)!=0 ){
    rSortCost += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    اگر( nSorted!=0 ){
      rSortCost += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    اگر( pWInfo->iLimit<nRow ){
      nRow = pWInfo->iLimit;
    }
  }وگرنه اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    اگر( nRow>10 ){ nRow -= 10;  أكد( 10==sqlite3LogEst(2) ); }
  }
  rSortCost += estLog(nRow);
  بازگشت rSortCost;
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
ایستا صحیح computeMxChoice(WhereInfo *pWInfo){
  صحیح nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#ifdef SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  برای(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
    أكد( pWLoop->maskSelf==MASKBIT(pWLoop->iTab) );
    أكد( pWLoop->pNextLoop==0 || pWLoop->iTab<=pWLoop->pNextLoop->iTab );
  }
#endif /* SQLITE_DEBUG */

  اگر( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->bStarDone
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_StarQuery)
  ){
    SrcItem *aFromTabs;    /* All terms of the FROM clause */
    صحیح iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *pStart;     /* Where to start searching for dimension-tables */

    pWInfo->bStarDone = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    أكد( !pWInfo->bStarUsed );
    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    برای(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      صحیح nDep = 0;             /* Number of dimension tables */
      LogEst mxRun;             /* Maximum SCAN cost of a fact table */
      Bitmask mSeen = 0;        /* Mask of dimension tables */
      SrcItem *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      اگر( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        اگر( iFromIdx+3 > nLoop ){
          بشکن;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        وقتی( pStart && pStart->iTab<=iFromIdx ){
          pStart = pStart->pNextLoop;
        }
      }
      برای(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        اگر( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          بشکن; /* Constraint (bb) */
        }
        اگر( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->maskSelf & mSeen)==0  /* pWInfo not already a dependency */
         && (pWLoop->maskSelf & mSelfJoin)==0 /* Not a self-join */
        ){
          اگر( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }وگرنه{
            nDep++;
            mSeen |= pWLoop->maskSelf;
          }
        }
      }
      اگر( nDep<=2 ){
        ادامه; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#ifdef WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      اگر( !pWInfo->bStarUsed ){
        برای(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
          pWLoop->rStarDelta = 0;
        }
      }
#endif
#ifdef WHERETRACE_ENABLED /* 0x80000 */
      اگر( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = mSeen;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        برای(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
          اگر( mShow & pWLoop->maskSelf ){
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
      برای(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        اگر( pWLoop->iTab<iFromIdx ) ادامه;
        اگر( pWLoop->iTab>iFromIdx ) بشکن;
        اگر( pWLoop->rRun>mxRun ) mxRun = pWLoop->rRun;
      }
      اگر( ALWAYS(mxRun<LOGEST_MAX) ) mxRun++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      برای(pWLoop=pStart; pWLoop; pWLoop=pWLoop->pNextLoop){
        اگر( (pWLoop->maskSelf & mSeen)==0 ) ادامه;
        اگر( pWLoop->nLTerm ) ادامه;
        اگر( pWLoop->rRun<mxRun ){
#ifdef WHERETRACE_ENABLED /* 0x80000 */
          اگر( sqlite3WhereTrace & 0x80000 ){
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
    اگر( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->bStarUsed ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      برای(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        اگر( pWLoop->rStarDelta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#endif
  }
  بازگشت pWInfo->bStarUsed ? 18 : 12;
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
ایستا SQLITE_NOINLINE صحیح whereLoopIsNoBetter(
  ثابت WhereLoop *pCandidate,
  ثابت WhereLoop *pBaseline
){
  اگر( (pCandidate->wsFlags & WHERE_INDEXED)==0 ) بازگشت 1;
  اگر( (pBaseline->wsFlags & WHERE_INDEXED)==0 ) بازگشت 1;
  اگر( pCandidate->u.btree.pIndex->szIdxRow <
        pBaseline->u.btree.pIndex->szIdxRow ) بازگشت 0;
  بازگشت 1;
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
ایستا صحیح wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst){
  صحیح mxChoice;             /* Maximum number of simultaneous paths tracked */
  صحیح nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  صحیح iLoop;                /* Loop counter over the terms of the join */
  صحیح ii, jj;               /* Loop counters */
  صحیح mxI = 0;              /* Index of next entry to replace */
  صحیح nOrderBy;             /* Number of ORDER BY clause terms */
  LogEst mxCost = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  صحیح nTo, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  WherePath *aFrom;         /* All nFrom paths at the previous level */
  WherePath *aTo;           /* The nTo best paths at the current level */
  WherePath *pFrom;         /* An element of aFrom[] that we are working on */
  WherePath *pTo;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *aSortCost = 0;    /* Sorting and partial sorting costs */
  نویسه *pSpace;             /* Temporary memory used by this routine */
  صحیح nSpace;               /* Bytes of space allocated at pSpace */

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
  اگر( nLoop<=1 ){
    mxChoice = 1;
  }وگرنه اگر( nLoop==2 ){
    mxChoice = 5;
  }وگرنه اگر( pParse->nErr ){
    mxChoice = 1;
  }وگرنه{
    mxChoice = computeMxChoice(pWInfo);
  }
  أكد( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  اگر( pWInfo->pOrderBy==0 || nRowEst==0 ){
    nOrderBy = 0;
  }وگرنه{
    nOrderBy = pWInfo->pOrderBy->n_عبارت;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (اندازه(WherePath)+اندازه(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += اندازه(LogEst) * nOrderBy;
  pSpace = sqlite3_پشته_Alloc_Raw_NN(pParse->db, nSpace);
  اگر( pSpace==0 ) بازگشت SQLITE_NOMEM_BKPT;
  aTo = (WherePath*)pSpace;
  aFrom = aTo+mxChoice;
  memset(aFrom, 0, اندازه(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  برای(ii=mxChoice*2, pFrom=aTo; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  اگر( nOrderBy ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    aSortCost = (LogEst*)pX;
    memset(aSortCost, 0, اندازه(LogEst) * nOrderBy);
  }
  أكد( aSortCost==0 || &pSpace[nSpace]==(نویسه*)&aSortCost[nOrderBy] );
  أكد( aSortCost!=0 || &pSpace[nSpace]==(نویسه*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  أكد( 48==sqlite3LogEst(28) );
  nFrom = 1;
  أكد( aFrom[0].isOrdered==0 );
  اگر( nOrderBy ){
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
  برای(iLoop=0; iLoop<nLoop; iLoop++){
    nTo = 0;
    برای(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      برای(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        LogEst nOut;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask maskNew;                  /* Mask of src visited by (..) */
        Bitmask revMask;                  /* Mask of rev-order loops for (..) */

        اگر( (pWLoop->prereq & ~pFrom->maskLoop)!=0 ) ادامه;
        اگر( (pWLoop->maskSelf & pFrom->maskLoop)!=0 ) ادامه;
        اگر( (pWLoop->wsFlags & WHERE_AUTO_INDEX)!=0 && pFrom->nRow<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          أكد( 10==sqlite3LogEst(2) );
          ادامه;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->rRun + pFrom->nRow;
        اگر( pWLoop->rSetup ){
          rUnsort = sqlite3LogEstAdd(pWLoop->rSetup, rUnsort);
        }
        rUnsort = sqlite3LogEstAdd(rUnsort, pFrom->rUnsort);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        isOrdered = pFrom->isOrdered;
        اگر( isOrdered<0 ){
          revMask = 0;
          isOrdered = wherePathSatisfiesOrderBy(pWInfo,
                       pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &revMask);
        }وگرنه{
          revMask = pFrom->revLoop;
        }
        اگر( isOrdered>=0 && isOrdered<nOrderBy ){
          اگر( aSortCost[isOrdered]==0 ){
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
        }وگرنه{
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
        برای(jj=0, pTo=aTo; jj<nTo; jj++, pTo++){
          اگر( pTo->maskLoop==maskNew
           && ( ((pTo->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==nTo-1 );
            بشکن;
          }
        }
        اگر( jj>=nTo ){
          /* None of the existing best-so-far paths match the candidate. */
          اگر( nTo>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
            اگر( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#endif
            ادامه;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          اگر( nTo<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = nTo++;
          }وگرنه{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          pTo = &aTo[jj];
#ifdef WHERETRACE_ENABLED /* 0x4 */
          اگر( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#endif
        }وگرنه{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          اگر( (pTo->rCost<rCost)
           || (pTo->rCost==rCost && pTo->nRow<nOut)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort<rUnsort)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, pTo->aLoop[iLoop]) )
          ){
#ifdef WHERETRACE_ENABLED /* 0x4 */
            اگر( sqlite3WhereTrace&0x4 ){
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
            ادامه;
          }
          testcase( pTo->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
          اگر( sqlite3WhereTrace&0x4 ){
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
        memcpy(pTo->aLoop, pFrom->aLoop, اندازه(WhereLoop*)*iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        اگر( nTo>=mxChoice ){
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsort = aTo[0].nRow;
          برای(jj=1, pTo=&aTo[1]; jj<mxChoice; jj++, pTo++){
            اگر( pTo->rCost>mxCost
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
    اگر( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      صحیح nDone = 0;
      صحیح nProgress;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      انجامبده{
        nProgress = 0;
        rMin = 0x7fff;
        برای(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          اگر( pTo->rCost>rFloor && pTo->rCost<rMin ) rMin = pTo->rCost;
        }
        برای(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          اگر( pTo->rCost==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
               pTo->isOrdered>=0 ? (pTo->isOrdered+'0') : '?');
            اگر( pTo->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", pTo->revLoop);
            }وگرنه{
              sqlite3DebugPrintf("\n");
            }
            nDone++;
            nProgress++;
          }
        }
        rFloor = rMin;
      }وقتی( nDone<nTo && nProgress>0 );
    }
#endif

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  اگر( nFrom==0 ){
    sqlite3_Error_پیام(pParse, "no query solution");
    sqlite3_پشته_Free_NN(pParse->db, pSpace);
    بازگشت SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  أكد( nFrom==1 );
  pFrom = aFrom;

  أكد( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  برای(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask notUsed;
    صحیح rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pResultSet, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &notUsed);
    اگر( rc==pWInfo->pResultSet->n_عبارت ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  اگر( pWInfo->pOrderBy ){
    pWInfo->nOBSat = pFrom->isOrdered;
    اگر( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      اگر( pFrom->isOrdered==pWInfo->pOrderBy->n_عبارت ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      أكد( pWInfo->pSelect->pOrderBy==0
           || pWInfo->nOBSat <= pWInfo->pSelect->pOrderBy->n_عبارت );
    }وگرنه{
      pWInfo->revMask = pFrom->revLoop;
      اگر( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        اگر( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          اگر( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_COLUMN_IN))!=(WHERE_IPK|WHERE_COLUMN_IN)
          ){
            Bitmask m = 0;
            صحیح rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom,
                      WHERE_ORDERBY_LIMIT, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_COLUMN_IN );
            اگر( rc==pWInfo->pOrderBy->n_عبارت ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->revMask = m;
            }
          }
        }
      }وگرنه اگر( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    اگر( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->pOrderBy->n_عبارت && nLoop>0
    ){
      Bitmask revMask = 0;
      صحیح nOrder = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &revMask
      );
      أكد( pWInfo->sorted==0 );
      اگر( nOrder==pWInfo->pOrderBy->n_عبارت ){
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
  sqlite3_پشته_Free_NN(pParse->db, pSpace);
  بازگشت SQLITE_OK;
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
ایستا SQLITE_NOINLINE تهی whereInterstageHeuristic(WhereInfo *pWInfo){
  صحیح i;
#ifdef WHERETRACE_ENABLED
  صحیح once = 0;
#endif
  برای(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    اگر( p==0 ) بشکن;
    اگر( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      بشکن;
    }
    اگر( (p->wsFlags & (WHERE_COLUMN_EQ|WHERE_COLUMN_NULL|WHERE_COLUMN_IN))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      برای(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->pNextLoop){
        اگر( pLoop->iTab!=iTab ) ادامه;
        اگر( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_INDEX))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          ادامه;
        }
#ifdef WHERETRACE_ENABLED
        اگر( sqlite3WhereTrace & 0x80 ){
          اگر( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#endif /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }وگرنه{
      بشکن;
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
ایستا صحیح whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  صحیح iCur;
  صحیح j;
  Table *pTab;
  Index *pIdx;
  WhereScan scan;

  pWInfo = pBuilder->pWInfo;
  اگر( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) بازگشت 0;
  أكد( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  اگر( IsVirtual(pTab) ) بازگشت 0;
  اگر( pItem->fg.isIndexedBy || pItem->fg.notIndexed ){
    testcase( pItem->fg.isIndexedBy );
    testcase( pItem->fg.notIndexed );
    بازگشت 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = whereScanInit(&scan, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  وقتی( pTerm && pTerm->prereq_راست ) pTerm = whereScanNext(&scan);
  اگر( pTerm ){
    testcase( pTerm->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->rRun = 33;  /* 33==sqlite3LogEst(10) */
  }وگرنه{
    برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      صحیح opMask;
      أكد( pLoop->aLTermSpace==pLoop->aLTerm );
      اگر( !IsUniqueIndex(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_کلید_Col>ArraySize(pLoop->aLTermSpace)
      ) ادامه;
      opMask = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      برای(j=0; j<pIdx->n_کلید_Col; j++){
        pTerm = whereScanInit(&scan, pWC, iCur, j, opMask, pIdx);
        وقتی( pTerm && pTerm->prereq_راست ) pTerm = whereScanNext(&scan);
        اگر( pTerm==0 ) بشکن;
        testcase( pTerm->eOperator & WO_IS );
        pLoop->aLTerm[j] = pTerm;
      }
      اگر( j!=pIdx->n_کلید_Col ) ادامه;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_INDEXED;
      اگر( pIdx->isCovering || (pItem->colUsed & pIdx->colNotIdxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.pIndex = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->rRun = 39;  /* 39==sqlite3LogEst(15) */
      بشکن;
    }
  }
  اگر( pLoop->wsFlags ){
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    أكد( pWInfo->sMaskSet.n==1 && iCur==pWInfo->sMaskSet.ix[0] );
    pLoop->maskSelf = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    اگر( pWInfo->pOrderBy ) pWInfo->nOBSat =  pWInfo->pOrderBy->n_عبارت;
    اگر( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    اگر( scan.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#ifdef SQLITE_DEBUG
    pLoop->cId = '0';
#endif
#ifdef WHERETRACE_ENABLED
    اگر( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#endif
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** Helper function for exprIsDeterministic().
*/
ایستا صحیح عبارت_گره_Is_Deterministic(Walker *pWalker, عبارت *p_عبارت){
  اگر( p_عبارت->op==TK_FUNCTION && عبارت_Has_Property(p_عبارت, EP_ConstFunc)==0 ){
    pWalker->eCode = 0;
    بازگشت WRC_Abort;
  }
  بازگشت WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
ایستا صحیح عبارت_Is_Deterministic(عبارت *p){
  Walker w;
  memset(&w, 0, اندازه(w));
  w.eCode = 1;
  w.x_عبارت_Callback = عبارت_گره_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.eCode;
}

 
#ifdef WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
ایستا تهی showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  اگر( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    صحیح i;
    ایستا ثابت نویسه zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    برای(p=pWInfo->pLoops, i=0; p; p=p->pNextLoop, i++){
      p->cId = zLabel[i%(اندازه(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#وگرنه
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
ایستا SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  صحیح i;
  Bitmask tabUsed;
  صحیح has_راست_Join;

  /* Preconditions checked by the caller */
  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  أكد( pWInfo->pResultSet!=0 );
  أكد( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tabUsed = sqlite3_Where_عبارت_List_Usage(&pWInfo->sMaskSet, pWInfo->pResultSet);
  اگر( pWInfo->pOrderBy ){
    tabUsed |= sqlite3_Where_عبارت_List_Usage(&pWInfo->sMaskSet, pWInfo->pOrderBy);
  }
  has_راست_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  برای(i=pWInfo->nLevel-1; i>=1; i--){
    WhereTerm *pTerm, *pEnd;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    اگر( (pItem->fg.jointype & (JT_چپ|JT_راست))!=JT_چپ ) ادامه;
    اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      ادامه;
    }
    اگر( (tabUsed & pLoop->maskSelf)!=0 ) ادامه;
    pEnd = pWInfo->sWC.a + pWInfo->sWC.nTerm;
    برای(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      اگر( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        اگر( !عبارت_Has_Property(pTerm->p_عبارت, EP_OuterON)
         || pTerm->p_عبارت->w.iJoin!=pItem->iCursor
        ){
          بشکن;
        }
      }
      اگر( has_راست_Join
       && عبارت_Has_Property(pTerm->p_عبارت, EP_InnerON)
       && NEVER(pTerm->p_عبارت->w.iJoin==pItem->iCursor)
      ){
        بشکن;  /* restriction (5) */
      }
    }
    اگر( pTerm<pEnd ) ادامه;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->revMask>>1) & ~m1)!=0 );
    pWInfo->revMask = (m1 & pWInfo->revMask) | ((pWInfo->revMask>>1) & ~m1);
    notReady &= ~pLoop->maskSelf;
    برای(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      اگر( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        pTerm->wtFlags |= TERM_CODED;
        pTerm->prereqAll = 0;
      }
    }
    اگر( i!=pWInfo->nLevel-1 ){
      صحیح nByte = (pWInfo->nLevel-1-i) * اندازه(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    أكد( pWInfo->nLevel>0 );
  }
  بازگشت notReady;
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
ایستا SQLITE_NOINLINE تهی whereCheckIfBloomFilterIsUseful(
  ثابت WhereInfo *pWInfo
){
  صحیح i;
  LogEst n_جستجو = 0;

  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  برای(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ثابت unsigned صحیح reqFlags = (WHERE_SELFCULL|WHERE_COLUMN_EQ);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    Table *pTab = pItem->pSTab;
    اگر( (pTab->tabFlags & TF_HasStat1)==0 ) بشکن;
    pTab->tabFlags |= TF_MaybeReanalyze;
    اگر( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_جستجو > pTab->nRowLogEst
    ){
      testcase( pItem->fg.jointype & JT_چپ );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (دوگانه)sqlite3LogEstToInt(n_جستجو), pTab->zName,
         (دوگانه)sqlite3LogEstToInt(pTab->nRowLogEst)));
    }
    n_جستجو += pLoop->nOut;
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
ایستا SQLITE_NOINLINE تهی where_Add_Indexed_عبارت(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  Index *pIdx,       /* The index-on-expression that contains the expressions */
  صحیح iIdxCur,       /* Cursor number for pIdx */
  SrcItem *pTabItem  /* The FROM clause entry for the table */
){
  صحیح i;
  Indexed_عبارت *p;
  Table *pTab;
  أكد( pIdx->b_Has_عبارت );
  pTab = pIdx->pTable;
  برای(i=0; i<pIdx->nColumn; i++){
    عبارت *p_عبارت;
    صحیح j = pIdx->aiColumn[i];
    اگر( j==XN_عبارت ){
      p_عبارت = pIdx->a_Col_عبارت->a[i].p_عبارت;
    }وگرنه اگر( j>=0 && (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)!=0 ){
      p_عبارت = sqlite3_Column_عبارت(pTab, &pTab->aCol[j]);
    }وگرنه{
      ادامه;
    }
    اگر( sqlite3_عبارت_Is_Constant(0,p_عبارت) ) ادامه;
    p = sqlite3DbMallocRaw(pParse->db,  اندازه(Indexed_عبارت));
    اگر( p==0 ) بشکن;
    p->pIENext = pParse->pIdxEpr;
#ifdef WHERETRACE_ENABLED
    اگر( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      اگر( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_عبارت(p_عبارت);
    }
#endif
    p->p_عبارت = sqlite3_عبارت_Dup(pParse->db, p_عبارت, 0);
    p->iDataCur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0;
    اگر( sqlite3IndexAffinityStr(pParse->db, pIdx) ){
      p->aff = pIdx->zColAff[i];
    }
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#endif
    pParse->pIdxEpr = p;
    اگر( p->pIENext==0 ){
      تهی *pArg = (تهی*)&pParse->pIdxEpr;
      sqlite3ParserAddCleanup(pParse, where_Indexed_عبارت_Cleanup, pArg);
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
ایستا SQLITE_NOINLINE تهی whereReverseScanOrder(WhereInfo *pWInfo){
  صحیح ii;
  برای(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    اگر( !pItem->fg.isCte
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
  عبارت *pWhere,           /* The WHERE clause */
  عبارت_List *pOrderBy,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  عبارت_List *pResultSet,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  صحیح iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  صحیح nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  صحیح nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  WhereMaskSet *pMaskSet;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  صحیح ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  صحیح rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  أكد( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  أكد( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_LIMIT)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, اندازه(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( pOrderBy && pOrderBy->n_عبارت==BMS-1 );
  اگر( pOrderBy && pOrderBy->n_عبارت>=BMS ){
    pOrderBy = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  اگر( pTabList->nSrc>BMS ){
    sqlite3_Error_پیام(pParse, "at most %d tables in a join", BMS);
    بازگشت 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + اندازه(WhereLoop));
  اگر( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    برو whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;
#اگر WHERETRACE_ENABLED
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
  memset(&pWInfo->a[0], 0, اندازه(WhereLoop)+nTabList*اندازه(WhereLevel));
  أكد( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  pMaskSet = &pWInfo->sMaskSet;
  pMaskSet->n = 0;
  pMaskSet->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((نویسه*)pWInfo)+nByteWInfo);
  أكد( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
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
  اگر( nTabList==0 ){
    اگر( pOrderBy ) pWInfo->nOBSat = pOrderBy->n_عبارت;
    اگر( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    اگر( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_مقدار)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }وگرنه{
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
    انجامبده{
      createMask(pMaskSet, pTabList->a[ii].iCursor);
      sqlite3WhereTabFuncArgs(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }وقتی( (++ii)<pTabList->nSrc );
  #ifdef SQLITE_DEBUG
    {
      Bitmask mx = 0;
      برای(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3WhereGetMask(pMaskSet, pTabList->a[ii].iCursor);
        أكد( m>=mx );
        mx = m;
      }
    }
  #endif
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_عبارت_Analyze(pTabList, &pWInfo->sWC);
  اگر( pSelect && pSelect->pLimit ){
    sqlite3WhereAddLimit(&pWInfo->sWC, pSelect);
  }
  اگر( pParse->nErr ) برو whereBeginError;

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
  برای(ii=0; ii<sWLB.pWC->nBase; ii++){
    WhereTerm *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    عبارت *pX;                          /* The expression of pT */
    اگر( pT->wtFlags & TERM_VIRTUAL ) ادامه;
    pX = pT->p_عبارت;
    أكد( pX!=0 );
    أكد( pT->prereqAll!=0 || !عبارت_Has_Property(pX, EP_OuterON) );
    اگر( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || عبارت_Is_Deterministic(pX))   /* Condition (4) */
     && !(عبارت_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_عبارت_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= TERM_CODED;
    }
  }

  اگر( wctrlFlags & WHERE_WANT_DISTINCT ){
    اگر( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }وگرنه اگر( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, pResultSet) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }وگرنه اگر( pOrderBy==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->pOrderBy = pResultSet;
    }
  }

  /* Construct the WhereLoop objects */
#اگر defined(WHERETRACE_ENABLED)
  اگر( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    اگر( wctrlFlags & WHERE_USE_LIMIT ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    اگر( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, اندازه(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.pOrderBy = pOrderBy;
      sSelect.pEList = pResultSet;
      sqlite3_درخت_View_Select(0, &sSelect, 0);
    }
    اگر( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#endif

  اگر( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = whereLoopAddAll(&sWLB);
    اگر( rc ) برو whereBeginError;

#ifdef SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    اگر( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      وقتی( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->pNextLoop;
        whereLoopDelete(db, p);
      }
      rc = whereLoopAddAll(&sWLB);
      اگر( rc ) برو whereBeginError;
    }
#endif
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    wherePathSolver(pWInfo, 0);
    اگر( db->mallocFailed ) برو whereBeginError;
    اگر( pWInfo->pOrderBy ){
       whereInterstageHeuristic(pWInfo);
       wherePathSolver(pWInfo, pWInfo->nRowOut<0 ? 1 : pWInfo->nRowOut+1);
       اگر( db->mallocFailed ) برو whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->nRowOut, pWInfo->nRowOut-30));
      pWInfo->nRowOut -= 30;
    }

  }
  أكد( pWInfo->pTabList!=0 );
  اگر( pWInfo->pOrderBy==0 && (db->flags & SQLITE_ReverseOrder)!=0 ){
    whereReverseScanOrder(pWInfo);
  }
  اگر( pParse->nErr ){
    برو whereBeginError;
  }
  أكد( db->mallocFailed==0 );
#ifdef WHERETRACE_ENABLED
  اگر( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->rTotalCost, pWInfo->nRowOut);
    اگر( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->revMask);
    }
    انتخاب( pWInfo->eDistinct ){
      حالت WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        بشکن;
      }
      حالت WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        بشکن;
      }
      حالت WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        بشکن;
      }
    }
    sqlite3DebugPrintf("\n");
    برای(ii=0; ii<pWInfo->nLevel; ii++){
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
  اگر( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && pResultSet!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    أكد( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  اگر( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#اگر defined(WHERETRACE_ENABLED)
  اگر( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
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
  أكد( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  اگر( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    صحیح wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    صحیح bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    أكد( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    اگر( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      اگر( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        اگر( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  برای(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    Table *pTab;     /* Table to open */
    صحیح iDb;         /* Index of database containing table/index */
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    اگر( ii==0 || (pTabItem[0].fg.jointype & JT_چپ)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }وگرنه اگر( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }وگرنه{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    اگر( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }وگرنه
#ifndef SQLITE_OMIT_VIRTUALTABLE
    اگر( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ثابت نویسه *pVTab = (ثابت نویسه *)sqlite3GetVTable(db, pTab);
      صحیح iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }وگرنه اگر( IsVirtual(pTab) ){
      /* noop */
    }وگرنه
#endif
    اگر( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (pTabItem->fg.jointype & (JT_LTORJ|JT_راست))!=0
    ){
      صحیح op = OP_OpenRead;
      اگر( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);
      أكد( pTabItem->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS );
      اگر( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->nCol<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_INDEX|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = pTabItem->colUsed;
        صحیح n = 0;
        برای(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_TO_PTR(n), P4_INT32);
        أكد( n<=pTab->nCol );
      }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
      اگر( pLoop->u.btree.pIndex!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }وگرنه
#endif
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
      sqlite3VdbeAddOp3(v, OP_ColumnsUsed, pTabItem->iCursor,
                    LOWER32(pTabItem->colUsed), UPPER32(pTabItem->colUsed));
#endif
      اگر( ii>=2
       && (pTabItem[0].fg.jointype & (JT_LTORJ|JT_چپ))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3VdbeAddOp2(v, OP_IfEmpty, pTabItem->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }وگرنه{
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    اگر( pLoop->wsFlags & WHERE_INDEXED ){
      Index *pIx = pLoop->u.btree.pIndex;
      صحیح iIndexCur;
      صحیح op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      أكد( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      اگر( !HasRowid(pTab) && Is_Primary_کلید_Index(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        iIndexCur = pLevel->iTabCur;
        op = 0;
      }وگرنه اگر( pWInfo->eOnePass!=ONEPASS_OFF ){
        Index *pJ = pTabItem->pSTab->pIndex;
        iIndexCur = iAuxArg;
        أكد( wctrlFlags & WHERE_ONEPASS_DESIRED );
        وقتی( ALWAYS(pJ) && pJ!=pIx ){
          iIndexCur++;
          pJ = pJ->pNext;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = iIndexCur;
      }وگرنه اگر( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        iIndexCur = iAuxArg;
        op = OP_ReopenIdx;
      }وگرنه{
        iIndexCur = pParse->nTab++;
        اگر( pIx->b_Has_عبارت && OptimizationEnabled(db, SQLITE_Indexed_عبارت) ){
          where_Add_Indexed_عبارت(pParse, pIx, iIndexCur, pTabItem);
        }
        اگر( pIx->pPartIdxWhere && (pTabItem->fg.jointype & JT_راست)==0 ){
          where_Part_Idx_عبارت(
              pParse, pIx, pIx->pPartIdxWhere, 0, iIndexCur, pTabItem
          );
        }
      }
      pLevel->iIdxCur = iIndexCur;
      أكد( pIx!=0 );
      أكد( pIx->pSchema==pTab->pSchema );
      أكد( iIndexCur>=0 );
      اگر( op ){
        sqlite3VdbeAddOp3(v, op, iIndexCur, pIx->tnum, iDb);
        sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pIx);
        اگر( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
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
          صحیح ii, jj;
          برای(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            اگر( jj<0 ) ادامه;
            اگر( jj>63 ) jj = 63;
            اگر( (pTabItem->colUsed & MASKBIT(jj))==0 ) ادامه;
            colUsed |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3VdbeAddOp3(v, OP_ColumnsUsed, iIndexCur,
                            LOWER32(colUsed),UPPER32(colUsed));
        }
#endif /* SQLITE_ENABLE_COLUMN_USED_MASK */
#ifdef SQLITE_ENABLE_CURSOR_HINTS
        اگر( HasRowid(pTab) ){
          sqlite3VdbeAddOp3(v, OP_CursorHint, iIndexCur, 0, pTabItem->iCursor);
        }
#endif
      }
    }
    اگر( iDb>=0 ) sqlite3CodeVerifySchema(pParse, iDb);
    اگر( (pTabItem->fg.jointype & JT_راست)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, اندازه(Where_راست_Join)))!=0
    ){
      Where_راست_Join *pRJ = pLevel->pRJ;
      صحیح bBloomable = 0;
      pRJ->iMatch = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, pRJ->regReturn);
      أكد( pTab==pTabItem->pSTab );
      اگر( HasRowid(pTab) ){
        کلید_Info *pInfo;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, 1);
        bBloomable = 1;
        pInfo = sqlite3_کلید_Info_Alloc(pParse->db, 1, 0);
        اگر( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->aSortFlags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }وگرنه{
        Index *pPk = sqlite3_Primary_کلید_Index(pTab);
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, pPk->n_کلید_Col);
        sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pPk);
        bBloomable = sqlite3IndexBloomable(pPk,pPk->n_کلید_Col);
      }
      اگر( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Blob, 65536, pRJ->regBloom);
      }وگرنه{
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
  pWInfo->i_بالاترین = sqlite3VdbeCurrentAddr(v);
  اگر( db->mallocFailed ) برو whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  برای(ii=0; ii<nTabList; ii++){
    صحیح addrExplain;
    صحیح wsFlags;
    SrcItem *pSrc;
    اگر( pParse->nErr ) برو whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    اگر( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      صحیح iOnce = 0;
      أكد( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      اگر( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }وگرنه{
        iOnce = 0;
      }
      sqlite3VdbeAddOp2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      اگر( iOnce )  sqlite3VdbeJumpHere(v, iOnce);
    }
    أكد( pTabList == pWInfo->pTabList );
    اگر( (wsFlags & (WHERE_AUTO_INDEX|WHERE_BLOOMFILTER))!=0 ){
      اگر( (wsFlags & WHERE_AUTO_INDEX)!=0 ){
#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
        constructAutomaticIndex(pParse, &pWInfo->sWC, notReady, pLevel);
#endif
      }وگرنه{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      اگر( db->mallocFailed ) برو whereBeginError;
    }
    addrExplain = sqlite3WhereExplainOneScan(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    اگر( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3WhereAddScanStatus(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->iEndWhere = sqlite3VdbeCurrentAddr(v);
  بازگشت pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  اگر( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#ifdef WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#endif /* WHERETRACE_ENABLED */
  بازگشت 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#ifndef SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#وگرنه
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  ایستا تهی sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    صحیح pc,
    VdbeOp *pOp
  ){
    اگر( (db->flags & SQLITE_VdbeAddopTrace)==0 ) بازگشت;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3ShowWhereTerm(0); /* So compiler won't complain about unused func */
  }
#endif

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
تهی sqlite3WhereEnd(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  صحیح iEnd = sqlite3VdbeCurrentAddr(v);
  صحیح nRJ = 0;
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  صحیح addrSeek = 0;
#endif

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  برای(i=pWInfo->nLevel-1; i>=0; i--){
    صحیح addr;
    pLevel = &pWInfo->a[i];
    اگر( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_راست_Join *pRJ = pLevel->pRJ;
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
    اگر( pLevel->op!=OP_Noop ){
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      Index *pIdx;
      صحیح n;
      اگر( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_INDEXED)!=0
       && (pIdx = pLoop->u.btree.pIndex)->hasStat1
       && (n = pLoop->u.btree.nDistinctCol)>0
       && pIdx->aiRowLogEst[n]>=36
      ){
        صحیح r1 = pParse->nMem+1;
        صحیح j, op;
        صحیح addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        اگر( pLevel->i_چپ_Join ){
          addrIfNull = sqlite3VdbeAddOp2(v, OP_IfNullRow, pLevel->iIdxCur, r1);
        }
        برای(j=0; j<n; j++){
          sqlite3VdbeAddOp3(v, OP_Column, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3VdbeAddOp4Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3VdbeAddOp2(v, OP_Goto, 1, pLevel->p2);
        اگر( pLevel->i_چپ_Join ){
          sqlite3VdbeJumpHere(v, addrIfNull);
        }
      }
#endif /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    اگر( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }وگرنه اگر( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      صحیح iBypass = sqlite3VdbeAddOp0(v, OP_Goto);
      أكد( pLoop->wsFlags & WHERE_AUTO_INDEX );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3VdbeAddOp1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3VdbeJumpHere(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    اگر( pLevel->op!=OP_Noop ){
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_Next);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      اگر( addrSeek ){
        sqlite3VdbeJumpHere(v, addrSeek);
        addrSeek = 0;
      }
#endif
      اگر( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3VdbeAddOp2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    اگر( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
    أكد( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_IN_ABLE) && pLevel->u.in.nIn>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    اگر( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3VdbeAddOp2(v, OP_DecrJumpZero, (صحیح)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#endif
    اگر( (pLoop->wsFlags & WHERE_IN_ABLE)!=0 && pLevel->u.in.nIn>0 ){
      ساختار InLoop *pIn;
      صحیح j;
      برای(j=pLevel->u.in.nIn, pIn=&pLevel->u.in.aInLoop[j-1]; j>0; j--, pIn--){
        أكد( sqlite3VdbeGetOp(v, pIn->addr_In_بالاترین+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3VdbeJumpHere(v, pIn->addr_In_بالاترین+1);
        اگر( pIn->eEndLoopOp!=OP_Noop ){
          اگر( pIn->nPrefix ){
            صحیح bEarlyOut =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0;
            اگر( pLevel->i_چپ_Join ){
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
            اگر( bEarlyOut ){
              sqlite3VdbeAddOp4Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  pIn->iBase, pIn->nPrefix);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3VdbeJumpHere(v, pIn->addr_In_بالاترین+1);
            }
          }
          sqlite3VdbeAddOp2(v, pIn->eEndLoopOp, pIn->iCur, pIn->addr_In_بالاترین);
          VdbeCoverage(v);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_Prev);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_Next);
        }
        sqlite3VdbeJumpHere(v, pIn->addr_In_بالاترین-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    اگر( pLevel->pRJ ){
      sqlite3VdbeAddOp3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    اگر( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.pIndex->zName));
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip-2);
    }
    اگر( pLevel->i_چپ_Join ){
      صحیح ws = pLoop->wsFlags;
      addr = sqlite3VdbeAddOp1(v, OP_IfPos, pLevel->i_چپ_Join); VdbeCoverage(v);
      أكد( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_INDEXED)!=0 );
      اگر( (ws & WHERE_IDX_ONLY)==0 ){
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];
        أكد( pLevel->iTabCur==pSrc->iCursor );
        اگر( pSrc->fg.viaCoroutine ){
          صحیح m, n;
          أكد( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->regResult;
          أكد( pSrc->pSTab!=0 );
          m = pSrc->pSTab->nCol;
          sqlite3VdbeAddOp3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iTabCur);
      }
      اگر( (ws & WHERE_INDEXED)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        اگر( ws & WHERE_MULTI_OR ){
          Index *pIx = pLevel->u.pCoveringIdx;
          صحیح iDb = sqlite3SchemaToIndex(db, pIx->pSchema);
          sqlite3VdbeAddOp3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pIx);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
      }
      اگر( ws & WHERE_IN_ABLE ){
        صحیح m;
        برای(m=0; m<pLevel->u.in.nIn; m++){
          ساختار InLoop *pIn = &pLevel->u.in.aInLoop[m];
          اگر( pIn->eEndLoopOp!=OP_Noop ){
            sqlite3VdbeAddOp1(v, OP_NullRow, pIn->iCur);
          }
        }
      }
      اگر( pLevel->op==OP_Return ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }وگرنه{
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  أكد( pWInfo->nLevel<=pTabList->nSrc );
  برای(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    صحیح k, last;
    VdbeOp *pOp, *pLastOp;
    Index *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pSTab;
    أكد( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    اگر( pLevel->pRJ ){
      sqlite3_Where_راست_Join_Loop(pWInfo, i, pLevel);
      ادامه;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    اگر( pTabItem->fg.viaCoroutine ){
      testcase( pParse->db->mallocFailed );
      أكد( pTabItem->fg.isSubquery );
      أكد( pTabItem->u4.pSubq->regResult>=0 );
      translateColumnToCopy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->u4.pSubq->regResult, 0);
      ادامه;
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
    اگر( pLoop->wsFlags & (WHERE_INDEXED|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.pIndex;
    }وگرنه اگر( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    اگر( pIdx
     && !db->mallocFailed
    ){
      اگر( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->pTable) ){
        last = iEnd;
      }وگرنه{
        last = pWInfo->iEndWhere;
      }
      اگر( pIdx->b_Has_عبارت ){
        Indexed_عبارت *p = pParse->pIdxEpr;
        وقتی( p ){
          اگر( p->iIdxCur==pLevel->iIdxCur ){
#ifdef WHERETRACE_ENABLED
            اگر( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->iIdxCol);
              اگر( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_عبارت(p->p_عبارت);
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
      اگر( db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, last-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      أكد( pOp->opcode!=OP_Column || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_IfNullRow || pOp->p1!=pLevel->iTabCur );
#endif
      pOp = sqlite3VdbeGetOp(v, k);
      pLastOp = pOp + (last - k);
      أكد( pOp<=pLastOp );
      انجامبده{
        اگر( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }وگرنه اگر( pOp->opcode==OP_Column
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
         || pOp->opcode==OP_Offset
#endif
        ){
          صحیح x = pOp->p2;
          أكد( pIdx->pTable==pTab );
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
          اگر( pOp->opcode==OP_Offset ){
            /* Do not need to translate the column number */
          }وگرنه
#endif
          اگر( !HasRowid(pTab) ){
            Index *pPk = sqlite3_Primary_کلید_Index(pTab);
            x = pPk->aiColumn[x];
            أكد( x>=0 );
          }وگرنه{
            testcase( x!=sqlite3StorageColumnToTable(pTab,x) );
            x = sqlite3StorageColumnToTable(pTab,x);
          }
          x = sqlite3TableColumnToIndex(pIdx, x);
          اگر( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }وگرنه اگر( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            اگر( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_پیام(pParse, "internal query planner error");
              pParse->rc = SQLITE_INTERNAL;
            }وگرنه{
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
        }وگرنه اگر( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }وگرنه اگر( pOp->opcode==OP_IfNullRow ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#ifdef SQLITE_DEBUG
        k++;
#endif
      }وقتی( (++pOp)<pLastOp );
#ifdef SQLITE_DEBUG
      اگر( db->flags & SQLITE_VdbeAddopTrace ) printf("TRANSLATE complete\n");
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
  بازگشت;
}
