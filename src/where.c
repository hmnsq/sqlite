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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
প্রকারলেখ কাঠামো Hidden_সূচক_Info Hidden_সূচক_Info;
কাঠামো Hidden_সূচক_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  পূর্ণসংখ্যা eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 mIn;                 /* Mask of terms that are <col> IN (...) */
  u32 mHandleIn;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_মান *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#define SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_সূচক_Info,aRhs) + (N)*আকার(sqlite3_মান*))

/* Forward declaration of methods */
স্থির পূর্ণসংখ্যা whereLoopResize(sqlite3*, WhereLoop*, পূর্ণসংখ্যা);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_Row_গুনতি(WhereInfo *pWInfo){
  ফেরত pWInfo->nRowOut;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
পূর্ণসংখ্যা sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  ফেরত pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
পূর্ণসংখ্যা sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  ফেরত pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
পূর্ণসংখ্যা sqlite3WhereOrderByLimitOptLabel(WhereInfo *pWInfo){
  WhereLevel *pInner;
  যদি( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    ফেরত pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  প্রতিজ্ঞা( pInner->addrNxt!=0 );
  ফেরত pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
শূন্য sqlite3WhereMinMaxOptEarlyOut(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  পূর্ণসংখ্যা i;
  যদি( !pWInfo->bOrderedInnerLoop ) ফেরত;
  যদি( pWInfo->nOBSat==0 ) ফেরত;
  জন্য(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    যদি( (pInner->pWLoop->wsFlags & WHERE_COLUMN_IN)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      ফেরত;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
পূর্ণসংখ্যা sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  প্রতিজ্ঞা( pWInfo->iContinue!=0 );
  ফেরত pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
পূর্ণসংখ্যা sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  ফেরত pWInfo->iBreak;
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
পূর্ণসংখ্যা sqlite3WhereOkOnePass(WhereInfo *pWInfo, পূর্ণসংখ্যা *aiCur){
  memcpy(aiCur, pWInfo->aiCurOnePass, আকার(পূর্ণসংখ্যা)*2);
#ifdef WHERETRACE_ENABLED
  যদি( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#endif
  ফেরত pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
পূর্ণসংখ্যা sqlite3WhereUsesDeferredSeek(WhereInfo *pWInfo){
  ফেরত pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
স্থির শূন্য whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc){
  pDest->n = pSrc->n;
  memcpy(pDest->a, pSrc->a, pDest->n*আকার(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
স্থির পূর্ণসংখ্যা where_Or_ঢোকাও(
  WhereOrSet *pSet,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst rRun,           /* Run-cost of the new entry */
  LogEst nOut            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  জন্য(i=pSet->n, p=pSet->a; i>0; i--, p++){
    যদি( rRun<=p->rRun && (prereq & p->prereq)==prereq ){
      যাও where_Or_ঢোকাও_done;
    }
    যদি( p->rRun<=rRun && (p->prereq & prereq)==p->prereq ){
      ফেরত 0;
    }
  }
  যদি( pSet->n<N_OR_COST ){
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  }অন্যথা{
    p = pSet->a;
    জন্য(i=1; i<pSet->n; i++){
      যদি( p->rRun>pSet->a[i].rRun ) p = pSet->a + i;
    }
    যদি( p->rRun<=rRun ) ফেরত 0;
  }
where_Or_ঢোকাও_done:
  p->prereq = prereq;
  p->rRun = rRun;
  যদি( p->nOut>nOut ) p->nOut = nOut;
  ফেরত 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, পূর্ণসংখ্যা iCursor){
  পূর্ণসংখ্যা i;
  প্রতিজ্ঞা( pMaskSet->n<=(পূর্ণসংখ্যা)আকার(Bitmask)*8 );
  প্রতিজ্ঞা( pMaskSet->n>0 || pMaskSet->ix[0]<0 );
  প্রতিজ্ঞা( iCursor>=-1 );
  যদি( pMaskSet->ix[0]==iCursor ){
    ফেরত 1;
  }
  জন্য(i=1; i<pMaskSet->n; i++){
    যদি( pMaskSet->ix[i]==iCursor ){
      ফেরত MASKBIT(i);
    }
  }
  ফেরত 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
শূন্য *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  WhereMemBlock *pBlock;
  pBlock = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+আকার(*pBlock));
  যদি( pBlock ){
    pBlock->p_পরবর্তী = pWInfo->pMemToFree;
    pBlock->sz = nByte;
    pWInfo->pMemToFree = pBlock;
    pBlock++;
  }
  ফেরত (শূন্য*)pBlock;
}
শূন্য *sqlite3WhereRealloc(WhereInfo *pWInfo, শূন্য *pOld, u64 nByte){
  শূন্য *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  যদি( pNew && pOld ){
    WhereMemBlock *pOldBlk = (WhereMemBlock*)pOld;
    pOldBlk--;
    প্রতিজ্ঞা( pOldBlk->sz<nByte );
    memcpy(pNew, pOld, pOldBlk->sz);
  }
  ফেরত pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
স্থির শূন্য createMask(WhereMaskSet *pMaskSet, পূর্ণসংখ্যা iCursor){
  প্রতিজ্ঞা( pMaskSet->n < Array_আকার(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
স্থির রাশিমালা *where_ডান_Subexpr_Is_Column(রাশিমালা *p){
  p = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p->p_ডান);
  যদি( ALWAYS(p!=0) && p->op==TK_COLUMN && !রাশিমালা_Has_Property(p, EP_FixedCol) ){
    ফেরত p;
  }
  ফেরত 0;
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
স্থির SQLITE_NOINLINE ধ্রুবক অক্ষর *সূচক_In_Affinity_Ok(
  Parse *pParse, 
  WhereTerm *pTerm, 
  u8 idxaff
){
  রাশিমালা *pX = pTerm->p_রাশিমালা;
  রাশিমালা inexpr;

  প্রতিজ্ঞা( pTerm->eOperator & WO_IN );

  যদি( রাশিমালা_Use_XSelect(pX) ){
    পূর্ণসংখ্যা iField = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    যদি( sqlite3_রাশিমালা_Is_Vector(pX->p_বাম) ){
      iField = pTerm->u.x.iField - 1;
      inexpr.p_বাম = pX->p_বাম->x.pList->a[iField].p_রাশিমালা;
    }অন্যথা{
      inexpr.p_বাম = pX->p_বাম;
    }
    inexpr.p_ডান = pX->x.pSelect->pEList->a[iField].p_রাশিমালা;
    pX = &inexpr;
  }

  যদি( sqlite3_সূচক_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pX);
    ফেরত pRet ? pRet->zName : sqlite3_Str_দ্বিমিক;
  }
  ফেরত 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
স্থির WhereTerm *where_Scan_পরবর্তী(WhereScan *pScan){
  পূর্ণসংখ্যা iCur;            /* The cursor on the LHS of the term */
  i16 iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  রাশিমালা *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  WhereTerm *pTerm;    /* The term being tested */
  পূর্ণসংখ্যা k = pScan->k;    /* Where to start scanning */

  প্রতিজ্ঞা( pScan->iEquiv<=pScan->nEquiv );
  pWC = pScan->pWC;
  যখন(1){
    iColumn = pScan->aiColumn[pScan->iEquiv-1];
    iCur = pScan->aiCur[pScan->iEquiv-1];
    প্রতিজ্ঞা( pWC!=0 );
    প্রতিজ্ঞা( iCur>=0 );
    করো{
      জন্য(pTerm=pWC->a+k; k<pWC->nTerm; k++, pTerm++){
        প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 || pTerm->বাম_Cursor<0 );
        যদি( pTerm->বাম_Cursor==iCur
         && pTerm->u.x.বাম_Column==iColumn
         && (iColumn!=XN_রাশিমালা
             || sqlite3_রাশিমালা_Compare_Skip(pTerm->p_রাশিমালা->p_বাম,
                                       pScan->p_Idx_রাশিমালা,iCur)==0)
         && (pScan->iEquiv<=1 || !রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON))
        ){
          যদি( (pTerm->eOperator & WO_EQUIV)!=0
           && pScan->nEquiv<Array_আকার(pScan->aiCur)
           && (pX = where_ডান_Subexpr_Is_Column(pTerm->p_রাশিমালা))!=0
          ){
            পূর্ণসংখ্যা j;
            জন্য(j=0; j<pScan->nEquiv; j++){
              যদি( pScan->aiCur[j]==pX->iTable
               && pScan->aiColumn[j]==pX->iColumn ){
                  বিরতি;
              }
            }
            যদি( j==pScan->nEquiv ){
              pScan->aiCur[j] = pX->iTable;
              pScan->aiColumn[j] = pX->iColumn;
              pScan->nEquiv++;
            }
          }
          যদি( (pTerm->eOperator & pScan->opMask)!=0 ){
            /* Verify the affinity and collating sequence match */
            যদি( pScan->zCollName && (pTerm->eOperator & WO_ISNULL)==0 ){
              ধ্রুবক অক্ষর *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->p_রাশিমালা;

              যদি( (pTerm->eOperator & WO_IN) ){
                zCollName = সূচক_In_Affinity_Ok(pParse, pTerm, pScan->idxaff);
                যদি( !zCollName ) চালু;
              }অন্যথা{
                CollSeq *pColl;
                যদি( !sqlite3_সূচক_Affinity_Ok(pX, pScan->idxaff) ){
                  চালু;
                }
                প্রতিজ্ঞা(pX->p_বাম);
                pColl = sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_দ্বিমিক;
              }

              যদি( sqlite3StrICmp(zCollName, pScan->zCollName) ){
                চালু;
              }
            }
            যদি( (pTerm->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = pTerm->p_রাশিমালা->p_ডান, ALWAYS(pX!=0))
             && pX->op==TK_COLUMN
             && pX->iTable==pScan->aiCur[0]
             && pX->iColumn==pScan->aiColumn[0]
            ){
              testcase( pTerm->eOperator & WO_IS );
              চালু;
            }
            pScan->pWC = pWC;
            pScan->k = k+1;
#ifdef WHERETRACE_ENABLED
            যদি( (sqlite3WhereTrace & 0x20000)!=0 && pScan->nEquiv>1 ){
              পূর্ণসংখ্যা ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 pScan->aiCur[0], pScan->aiColumn[0], pTerm->iTerm);
              জন্য(ii=1; ii<pScan->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   pScan->aiCur[ii], pScan->aiColumn[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#endif
            ফেরত pTerm;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }যখন( pWC!=0 );
    যদি( pScan->iEquiv>=pScan->nEquiv ) বিরতি;
    pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv++;
  }
  ফেরত 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
স্থির SQLITE_NOINLINE WhereTerm *where_Scan_Init_সূচক_রাশিমালা(WhereScan *pScan){
  pScan->idxaff = sqlite3_রাশিমালা_Affinity(pScan->p_Idx_রাশিমালা);
  ফেরত where_Scan_পরবর্তী(pScan);
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
স্থির WhereTerm *whereScanInit(
  WhereScan *pScan,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  পূর্ণসংখ্যা iCur,               /* Cursor to scan for */
  পূর্ণসংখ্যা iColumn,            /* Column to scan for */
  u32 opMask,             /* Operator(s) to scan for */
  সূচক *pIdx             /* Must be compatible with this index */
){
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  pScan->p_Idx_রাশিমালা = 0;
  pScan->idxaff = 0;
  pScan->zCollName = 0;
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aiCur[0] = iCur;
  pScan->nEquiv = 1;
  pScan->iEquiv = 1;
  যদি( pIdx ){
    পূর্ণসংখ্যা j = iColumn;
    iColumn = pIdx->aiColumn[j];
    যদি( iColumn==pIdx->pTable->iPKey ){
      iColumn = XN_ROWID;
    }অন্যথা যদি( iColumn>=0 ){
      pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
      pScan->zCollName = pIdx->azColl[j];
    }অন্যথা যদি( iColumn==XN_রাশিমালা ){
      pScan->p_Idx_রাশিমালা = pIdx->a_Col_রাশিমালা->a[j].p_রাশিমালা;
      pScan->zCollName = pIdx->azColl[j];
      pScan->aiColumn[0] = XN_রাশিমালা;
      ফেরত where_Scan_Init_সূচক_রাশিমালা(pScan);
    }
  }অন্যথা যদি( iColumn==XN_রাশিমালা ){
    ফেরত 0;
  }
  pScan->aiColumn[0] = iColumn;
  ফেরত where_Scan_পরবর্তী(pScan);
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
  পূর্ণসংখ্যা iCur,             /* Cursor number of LHS */
  পূর্ণসংখ্যা iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  সূচক *pIdx           /* Must be compatible with this index, if not NULL */
){
  WhereTerm *p_ফলাফল = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  যখন( p ){
    যদি( (p->prereq_ডান & notReady)==0 ){
      যদি( p->prereq_ডান==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        ফেরত p;
      }
      যদি( p_ফলাফল==0 ) p_ফলাফল = p;
    }
    p = where_Scan_পরবর্তী(&scan);
  }
  ফেরত p_ফলাফল;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
স্থির পূর্ণসংখ্যা find_সূচক_Col(
  Parse *pParse,                  /* Parse context */
  রাশিমালা_List *pList,                /* Expression list to search */
  পূর্ণসংখ্যা iBase,                      /* Cursor for table associated with pIdx */
  সূচক *pIdx,                    /* Index to match column of */
  পূর্ণসংখ্যা iCol                        /* Column of index to match */
){
  পূর্ণসংখ্যা i;
  ধ্রুবক অক্ষর *zColl = pIdx->azColl[iCol];

  জন্য(i=0; i<pList->n_রাশিমালা; i++){
    রাশিমালা *p = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pList->a[i].p_রাশিমালা);
    যদি( ALWAYS(p!=0)
     && (p->op==TK_COLUMN || p->op==TK_AGG_COLUMN)
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->iTable==iBase
    ){
      CollSeq *pColl = sqlite3_রাশিমালা_NNColl_Seq(pParse, pList->a[i].p_রাশিমালা);
      যদি( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        ফেরত i;
      }
    }
  }

  ফেরত -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
স্থির পূর্ণসংখ্যা সূচক_Column_Not_Null(সূচক *pIdx, পূর্ণসংখ্যা iCol){
  পূর্ণসংখ্যা j;
  প্রতিজ্ঞা( pIdx!=0 );
  প্রতিজ্ঞা( iCol>=0 && iCol<pIdx->nColumn );
  j = pIdx->aiColumn[iCol];
  যদি( j>=0 ){
    ফেরত pIdx->pTable->aCol[j].notNull;
  }অন্যথা যদি( j==(-1) ){
    ফেরত 1;
  }অন্যথা{
    প্রতিজ্ঞা( j==(-2) );
    ফেরত 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
স্থির পূর্ণসংখ্যা isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  রাশিমালা_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  Table *pTab;
  সূচক *pIdx;
  পূর্ণসংখ্যা i;                         
  পূর্ণসংখ্যা iBase;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  যদি( pTabList->nSrc!=1 ) ফেরত 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  জন্য(i=0; i<pDistinct->n_রাশিমালা; i++){
    রাশিমালা *p = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pDistinct->a[i].p_রাশিমালা);
    যদি( NEVER(p==0) ) চালু;
    যদি( p->op!=TK_COLUMN && p->op!=TK_AGG_COLUMN ) চালু;
    যদি( p->iTable==iBase && p->iColumn<0 ) ফেরত 1;
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
  জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
    যদি( !Is_Unique_সূচক(pIdx) ) চালু;
    যদি( pIdx->pPartIdxWhere ) চালু;
    জন্য(i=0; i<pIdx->n_চাবি_Col; i++){
      যদি( 0==sqlite3WhereFindTerm(pWC, iBase, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        যদি( find_সূচক_Col(pParse, pDistinct, iBase, pIdx, i)<0 ) বিরতি;
        যদি( সূচক_Column_Not_Null(pIdx, i)==0 ) বিরতি;
      }
    }
    যদি( i==pIdx->n_চাবি_Col ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      ফেরত 1;
    }
  }

  ফেরত 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
স্থির LogEst estLog(LogEst N){
  ফেরত N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
স্থির শূন্য translateColumnToCopy(
  Parse *pParse,      /* Parsing context */
  পূর্ণসংখ্যা iStart,         /* Translate from this opcode to the end */
  পূর্ণসংখ্যা iTabCur,        /* OP_Column/OP_Rowid references to this table */
  পূর্ণসংখ্যা iRegister,      /* The first column is in this register */
  পূর্ণসংখ্যা iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  পূর্ণসংখ্যা iEnd = sqlite3VdbeCurrentAddr(v);
  যদি( pParse->db->mallocFailed ) ফেরত;
#ifdef SQLITE_DEBUG
  যদি( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    printf("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, iStart, iEnd);
  }
#endif
  জন্য(; iStart<iEnd; iStart++, pOp++){
    যদি( pOp->p1!=iTabCur ) চালু;
    যদি( pOp->opcode==OP_Column ){
#ifdef SQLITE_DEBUG
      যদি( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE OP_Column to OP_Copy at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }অন্যথা যদি( pOp->opcode==OP_Rowid ){
#ifdef SQLITE_DEBUG
      যদি( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE OP_Rowid to OP_Sequence at %d\n", iStart);
      }
#endif
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      যদি( iAutoidxCur==0 ){
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
#যদি !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
স্থির শূন্য where_Trace_সূচক_Info_Inputs(
  sqlite3_সূচক_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  পূর্ণসংখ্যা i;
  যদি( (sqlite3WhereTrace & 0x10)==0 ) ফেরত;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  জন্য(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].iTermOffset,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  জন্য(i=0; i<p->nOrderBy; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->aOrderBy[i].iColumn,
       p->aOrderBy[i].desc);
  }
}
স্থির শূন্য where_Trace_সূচক_Info_Outputs(
  sqlite3_সূচক_info *p,   /* The IndexInfo object */
  Table *pTab              /* The TABLE that is the virtual table */
){
  পূর্ণসংখ্যা i;
  যদি( (sqlite3WhereTrace & 0x10)==0 ) ফেরত;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  জন্য(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_সূচক,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_অঙ্ক);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->orderByConsumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimatedRows);
}
#অন্যথা
#define where_Trace_সূচক_Info_Inputs(A,B)
#define where_Trace_সূচক_Info_Outputs(A,B)
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
স্থির পূর্ণসংখ্যা constraintCompatibleWithOuterJoin(
  ধ্রুবক WhereTerm *pTerm,       /* WHERE clause term to check */
  ধ্রুবক SrcItem *pSrc           /* Table we are trying to access */
){
  প্রতিজ্ঞা( (pSrc->fg.jointype&(JT_বাম|JT_LTORJ|JT_ডান))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান))==JT_বাম );
  testcase( (pSrc->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান))==JT_LTORJ );
  testcase( রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON) )
  testcase( রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_InnerON) );
  যদি( !রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON|EP_InnerON)
   || pTerm->p_রাশিমালা->w.iJoin != pSrc->iCursor
  ){
    ফেরত 0;
  }
  যদি( (pSrc->fg.jointype & (JT_বাম|JT_ডান))!=0
   && NEVER(রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_InnerON))
  ){
    ফেরত 0;
  }
  ফেরত 1;
}

#ifndef SQLITE_OMIT_AUTOMATIC_সূচক
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
স্থির SQLITE_NOINLINE পূর্ণসংখ্যা column_Is_Good_সূচক_Candidate(
  ধ্রুবক Table *pTab,
  পূর্ণসংখ্যা iCol
){
  ধ্রুবক সূচক *pIdx;
  জন্য(pIdx = pTab->p_সূচক; pIdx!=0; pIdx=pIdx->p_পরবর্তী){
    পূর্ণসংখ্যা j;
    জন্য(j=0; j<pIdx->n_চাবি_Col; j++){
       যদি( pIdx->aiColumn[j]==iCol ){
         যদি( j==0 ) ফেরত 0;
         যদি( pIdx->hasStat1 && pIdx->aiRowLogEst[j+1]>20 ) ফেরত 0;
         বিরতি;
       }
    }
  }
  ফেরত 1;
}
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */



#ifndef SQLITE_OMIT_AUTOMATIC_সূচক
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
স্থির পূর্ণসংখ্যা term_Can_Drive_সূচক(
  ধ্রুবক WhereTerm *pTerm,        /* WHERE clause term to check */
  ধ্রুবক SrcItem *pSrc,           /* Table we are trying to access */
  ধ্রুবক Bitmask notReady         /* Tables in outer loops of the join */
){
  অক্ষর aff;
  পূর্ণসংখ্যা বাম_Col;
  
  যদি( pTerm->বাম_Cursor!=pSrc->iCursor ) ফেরত 0;
  যদি( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) ফেরত 0;
  প্রতিজ্ঞা( (pSrc->fg.jointype & JT_ডান)==0 );
  যদি( (pSrc->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান))!=0
   && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
  ){
    ফেরত 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  যদি( (pTerm->prereq_ডান & notReady)!=0 ) ফেরত 0;
  প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
  বাম_Col = pTerm->u.x.বাম_Column;
  যদি( বাম_Col<0 ) ফেরত 0;
  aff = pSrc->pSTab->aCol[বাম_Col].affinity;
  যদি( !sqlite3_সূচক_Affinity_Ok(pTerm->p_রাশিমালা, aff) ) ফেরত 0;
  testcase( pTerm->p_রাশিমালা->op==TK_IS );
  ফেরত column_Is_Good_সূচক_Candidate(pSrc->pSTab, বাম_Col);
}
#endif


#ifndef SQLITE_OMIT_AUTOMATIC_সূচক

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
স্থির শূন্য explain_Automatic_সূচক(
  Parse *pParse,
  সূচক *pIdx,                    /* Automatic index to explain */
  পূর্ণসংখ্যা bPartial,                   /* True if pIdx is a partial index */
  পূর্ণসংখ্যা *pAddrExplain               /* OUT: Address of OP_Explain */
){
  যদি( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    Table *pTab = pIdx->pTable;
    ধ্রুবক অক্ষর *zSep = "";
    অক্ষর *zText = 0;
    পূর্ণসংখ্যা ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    প্রতিজ্ঞা( pIdx->nColumn>1 );
    প্রতিজ্ঞা( pIdx->aiColumn[pIdx->nColumn-1]==XN_ROWID || !HasRowid(pTab) );
    জন্য(ii=0; ii<(pIdx->nColumn-1); ii++){
      ধ্রুবক অক্ষর *zName = 0;
      পূর্ণসংখ্যা iCol = pIdx->aiColumn[ii];

      zName = pTab->aCol[iCol].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    zText = sqlite3_str_finish(pStr);
    যদি( zText==0 ){
      sqlite3OomFault(pParse->db);
    }অন্যথা{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", zText, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(zText);
    }
  }
}
#অন্যথা
# define explain_Automatic_সূচক(a,b,c,d)
#endif

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
স্থির SQLITE_NOINLINE শূন্য construct_Automatic_সূচক(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  ধ্রুবক Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  পূর্ণসংখ্যা n_চাবি_Col;                /* Number of columns in the constructed index */
  WhereTerm *pTerm;           /* A single term of the WHERE clause */
  WhereTerm *pWCEnd;          /* End of pWC->a[] */
  সূচক *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  পূর্ণসংখ্যা addrInit;               /* Address of the initialization bypass jump */
  Table *pTable;              /* The table being indexed */
  পূর্ণসংখ্যা addr_শীর্ষ;                /* Top of the index fill loop */
  পূর্ণসংখ্যা regRecord;              /* Register holding an index record */
  পূর্ণসংখ্যা n;                      /* Column counter */
  পূর্ণসংখ্যা i;                      /* Loop counter */
  পূর্ণসংখ্যা mxBitCol;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  অক্ষর *zNotUsed;             /* Extra space on the end of pIdx */
  Bitmask idxCols;            /* Bitmap of columns used for indexing */
  Bitmask extraCols;          /* Bitmap of additional columns */
  u8 sentWarning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  রাশিমালা *pPartial = 0;         /* Partial Index Expression */
  পূর্ণসংখ্যা iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  SrcItem *pSrc;              /* The FROM clause term to get the next index */
  পূর্ণসংখ্যা addr_গণক = 0;        /* Address where integer counter is initialized */
  পূর্ণসংখ্যা regBase;                /* Array of registers where record is assembled */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  পূর্ণসংখ্যা addrExp = 0;            /* Address of OP_Explain */
#endif

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  প্রতিজ্ঞা( v!=0 );
  addrInit = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_চাবি_Col = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  pTable = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  জন্য(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    রাশিমালা *p_রাশিমালা = pTerm->p_রাশিমালা;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    যদি( (pTerm->wtFlags & TERM_VIRTUAL)==0
     && sqlite3_রাশিমালা_Is_Single_Table_Constraint(p_রাশিমালা, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_রাশিমালা_And(pParse, pPartial,
                                sqlite3_রাশিমালা_Dup(pParse->db, p_রাশিমালা, 0));
    }
    যদি( term_Can_Drive_সূচক(pTerm, pSrc, notReady) ){
      পূর্ণসংখ্যা iCol;
      Bitmask cMask;
      প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.বাম_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      যদি( !sentWarning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", pTable->zName,
            pTable->aCol[iCol].zCnName);
        sentWarning = 1;
      }
      যদি( (idxCols & cMask)==0 ){
        যদি( whereLoopResize(pParse->db, pLoop, n_চাবি_Col+1) ){
          যাও end_auto_সূচক_create;
        }
        pLoop->aLTerm[n_চাবি_Col++] = pTerm;
        idxCols |= cMask;
      }
    }
  }
  প্রতিজ্ঞা( n_চাবি_Col>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_চাবি_Col;
  pLoop->wsFlags = WHERE_COLUMN_EQ | WHERE_IDX_ONLY | WHERE_INDEXED
                     | WHERE_AUTO_সূচক;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  যদি( IsView(pTable) ){
    extraCols = ALLBITS & ~idxCols;
  }অন্যথা{
    extraCols = pSrc->colUsed & (~idxCols | MASKBIT(BMS-1));
  }
  যদি( !HasRowid(pTable) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    জন্য(i=0; i<pTable->nCol; i++){
      যদি( (pTable->aCol[i].colFlags & COLFLAG_PRIMKEY)==0 ) চালু;
      যদি( i>=BMS-1 ){
        extraCols |= MASKBIT(BMS-1);
        বিরতি;
      }
      যদি( idxCols & MASKBIT(i) ) চালু;
      extraCols |= MASKBIT(i);
    }
  }
  mxBitCol = MIN(BMS-1,pTable->nCol);
  testcase( pTable->nCol==BMS-1 );
  testcase( pTable->nCol==BMS-2 );
  জন্য(i=0; i<mxBitCol; i++){
    যদি( extraCols & MASKBIT(i) ) n_চাবি_Col++;
  }
  যদি( pSrc->colUsed & MASKBIT(BMS-1) ){
    n_চাবি_Col += pTable->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  প্রতিজ্ঞা( n_চাবি_Col <= pTable->nCol + MAX(0, pTable->nCol - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_সূচক_Object(pParse->db, n_চাবি_Col+HasRowid(pTable),
                                    0, &zNotUsed);
  যদি( pIdx==0 ) যাও end_auto_সূচক_create;
  pLoop->u.btree.p_সূচক = pIdx;
  pIdx->zName = "auto-index";
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  জন্য(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    যদি( term_Can_Drive_সূচক(pTerm, pSrc, notReady) ){
      পূর্ণসংখ্যা iCol;
      Bitmask cMask;
      প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      iCol = pTerm->u.x.বাম_Column;
      cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      যদি( (idxCols & cMask)==0 ){
        রাশিমালা *pX = pTerm->p_রাশিমালা;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.x.বাম_Column;
        pColl = sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pX);
        প্রতিজ্ঞা( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        যদি( !sqlite3_Is_দ্বিমিক(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_দ্বিমিক;
        n++;
      }
    }
  }
  প্রতিজ্ঞা( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  জন্য(i=0; i<mxBitCol; i++){
    যদি( extraCols & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_দ্বিমিক;
      n++;
    }
  }
  যদি( pSrc->colUsed & MASKBIT(BMS-1) ){
    জন্য(i=BMS-1; i<pTable->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_দ্বিমিক;
      n++;
    }
  }
  প্রতিজ্ঞা( n==n_চাবি_Col );
  যদি( HasRowid(pTable) ){
    pIdx->aiColumn[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_দ্বিমিক;
  }

  /* Create the automatic index */
  explain_Automatic_সূচক(pParse, pIdx, pPartial!=0, &addrExp);
  প্রতিজ্ঞা( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_চাবি_Col+1);
  sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pIdx);
  VdbeComment((v, "for %s", pTable->zName));
  যদি( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  প্রতিজ্ঞা( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  যদি( pSrc->fg.viaCoroutine ){
    পূর্ণসংখ্যা regYield;
    Subquery *pSubq;
    প্রতিজ্ঞা( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    প্রতিজ্ঞা( pSubq!=0 );
    regYield = pSubq->regReturn;
    addr_গণক = sqlite3VdbeAddOp2(v, OP_Integer, 0, 0);
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_শীর্ষ =  sqlite3VdbeAddOp1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }অন্যথা{
    প্রতিজ্ঞা( pLevel->addrHalt );
    addr_শীর্ষ = sqlite3VdbeAddOp2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  যদি( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_রাশিমালা_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  regRecord = sqlite3_Get_অস্থায়ী_Reg(pParse);
  regBase = sqlite3_Generate_সূচক_চাবি(
      pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0
  );
  যদি( pLevel->regFilter ){
    প্রতিজ্ঞা( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0,
                         regBase, pLoop->u.btree.nEq);
  }
  sqlite3VdbeScanStatusCounters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3VdbeAddOp2(v, OP_Idx_ঢোকাও, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  যদি( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  যদি( pSrc->fg.viaCoroutine ){
    প্রতিজ্ঞা( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addr_গণক, regBase+n);
    testcase( pParse->db->mallocFailed );
    প্রতিজ্ঞা( pLevel->iIdxCur>0 );
    translateColumnToCopy(pParse, addr_শীর্ষ, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_ফলাফল, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_শীর্ষ);
    pSrc->fg.viaCoroutine = 0;
    sqlite3VdbeJumpHere(v, addr_শীর্ষ);
  }অন্যথা{
    sqlite3VdbeAddOp2(v, OP_পরবর্তী, pLevel->iTabCur, addr_শীর্ষ+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    যদি( (pSrc->fg.jointype & JT_বাম)!=0 ){
      sqlite3VdbeJumpHere(v, addr_শীর্ষ);
    }
  }
  sqlite3_Release_অস্থায়ী_Reg(pParse, regRecord);
 
  /* Jump here when skipping the initialization */
  sqlite3VdbeJumpHere(v, addrInit);
  sqlite3VdbeScanStatusRange(v, addrExp, addrExp, -1);

end_auto_সূচক_create:
  sqlite3_রাশিমালা_Delete(pParse->db, pPartial);
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
স্থির SQLITE_NOINLINE শূন্য sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  পূর্ণসংখ্যা iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  পূর্ণসংখ্যা addrOnce;                        /* Address of opening OP_Once */
  পূর্ণসংখ্যা addr_শীর্ষ;                         /* Address of OP_Rewind */
  পূর্ণসংখ্যা addrCont;                        /* Jump here to skip a row */
  ধ্রুবক WhereTerm *pTerm;              /* For looping over WHERE clause terms */
  ধ্রুবক WhereTerm *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  পূর্ণসংখ্যা iCur;                            /* Cursor for table getting the filter */
  Indexed_রাশিমালা *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  Indexed_রাশিমালা *saved_p_Idx_Part_রাশিমালা;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_রাশিমালা = pParse->p_Idx_Part_রাশিমালা;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_রাশিমালা = 0;

  প্রতিজ্ঞা( pLoop!=0 );
  প্রতিজ্ঞা( v!=0 );
  প্রতিজ্ঞা( pLoop->wsFlags & WHERE_BLOOMFILTER );
  প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  করো{
    ধ্রুবক SrcList *pTabList;
    ধ্রুবক SrcItem *pItem;
    ধ্রুবক Table *pTab;
    u64 sz;
    পূর্ণসংখ্যা iSrc;
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
    প্রতিজ্ঞা( pItem!=0 );
    pTab = pItem->pSTab;
    প্রতিজ্ঞা( pTab!=0 );
    sz = sqlite3LogEstToInt(pTab->nRowLogEst);
    যদি( sz<10000 ){
      sz = 10000;
    }অন্যথা যদি( sz>10000000 ){
      sz = 10000000;
    }
    প্রতিজ্ঞা( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3VdbeAddOp2(v, OP_Blob, (পূর্ণসংখ্যা)sz, pLevel->regFilter);

    addr_শীর্ষ = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.nTerm];
    জন্য(pTerm=pWInfo->sWC.a; pTerm<pWCEnd; pTerm++){
      রাশিমালা *p_রাশিমালা = pTerm->p_রাশিমালা;
      যদি( (pTerm->wtFlags & TERM_VIRTUAL)==0
       && sqlite3_রাশিমালা_Is_Single_Table_Constraint(p_রাশিমালা, pTabList, iSrc, 0)
      ){
        sqlite3_রাশিমালা_If_False(pParse, pTerm->p_রাশিমালা, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    যদি( pLoop->wsFlags & WHERE_IPK ){
      পূর্ণসংখ্যা r1 = sqlite3_Get_অস্থায়ী_Reg(pParse);
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, r1);
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_অস্থায়ী_Reg(pParse, r1);
    }অন্যথা{
      সূচক *pIdx = pLoop->u.btree.p_সূচক;
      পূর্ণসংখ্যা n = pLoop->u.btree.nEq;
      পূর্ণসংখ্যা r1 = sqlite3_Get_অস্থায়ী_Range(pParse, n);
      পূর্ণসংখ্যা jj;
      প্রতিজ্ঞা( pIdx!=0 );
      জন্য(jj=0; jj<n; jj++){
        প্রতিজ্ঞা( pIdx->pTable==pItem->pSTab );
        sqlite3_রাশিমালা_Code_Load_সূচক_Column(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_অস্থায়ী_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3VdbeAddOp2(v, OP_পরবর্তী, pLevel->iTabCur, addr_শীর্ষ+1);
    VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr_শীর্ষ);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    যখন( ++iLevel < pWInfo->nLevel ){
      ধ্রুবক SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      যদি( pTabItem->fg.jointype & (JT_বাম|JT_LTORJ) ) চালু;
      pLoop = pLevel->pWLoop;
      যদি( NEVER(pLoop==0) ) চালু;
      যদি( pLoop->prereq & notReady ) চালু;
      যদি( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_COLUMN_IN))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        বিরতি;
      }
    }
  }যখন( iLevel < pWInfo->nLevel );
  sqlite3VdbeJumpHere(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_রাশিমালা = saved_p_Idx_Part_রাশিমালা;
}


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
স্থির WhereTerm *termFromWhereClause(WhereClause *pWC, পূর্ণসংখ্যা iTerm){
  WhereClause *p;
  জন্য(p=pWC; p; p=p->pOuter){
    যদি( iTerm<p->nTerm ) ফেরত &p->a[iTerm];
    iTerm -= p->nTerm;
  }
  ফেরত 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
স্থির sqlite3_সূচক_info *allocate_সূচক_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  SrcItem *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  পূর্ণসংখ্যা i, j;
  পূর্ণসংখ্যা nTerm;
  Parse *pParse = pWInfo->pParse;
  কাঠামো sqlite3_সূচক_constraint *pIdxCons;
  কাঠামো sqlite3_সূচক_orderby *pIdxOrderBy;
  কাঠামো sqlite3_সূচক_constraint_usage *pUsage;
  কাঠামো Hidden_সূচক_Info *pHidden;
  WhereTerm *pTerm;
  পূর্ণসংখ্যা nOrderBy;
  sqlite3_সূচক_info *pIdxInfo;
  u16 mNoOmit = 0;
  ধ্রুবক Table *pTab;
  পূর্ণসংখ্যা eDistinct = 0;
  রাশিমালা_List *pOrderBy = pWInfo->pOrderBy;
  WhereClause *p;

  প্রতিজ্ঞা( pSrc!=0 );
  pTab = pSrc->pSTab;
  প্রতিজ্ঞা( pTab!=0 );
  প্রতিজ্ঞা( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  জন্য(p=pWC, nTerm=0; p; p=p->pOuter){
    জন্য(i=0, pTerm=p->a; i<p->nTerm; i++, pTerm++){
      pTerm->wtFlags &= ~TERM_OK;
      যদি( pTerm->বাম_Cursor != pSrc->iCursor ) চালু;
      যদি( pTerm->prereq_ডান & mUnusable ) চালু;
      প্রতিজ্ঞা( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
      testcase( pTerm->eOperator & WO_IN );
      testcase( pTerm->eOperator & WO_ISNULL );
      testcase( pTerm->eOperator & WO_IS );
      testcase( pTerm->eOperator & WO_ALL );
      যদি( (pTerm->eOperator & ~(WO_EQUIV))==0 ) চালু;
      যদি( pTerm->wtFlags & TERM_VNULL ) চালু;

      প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      প্রতিজ্ঞা( pTerm->u.x.বাম_Column>=XN_ROWID );
      প্রতিজ্ঞা( pTerm->u.x.বাম_Column<pTab->nCol );
      যদি( (pSrc->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান))!=0
          && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
        ){
        চালু;
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
  যদি( pOrderBy ){
    পূর্ণসংখ্যা n = pOrderBy->n_রাশিমালা;
    জন্য(i=0; i<n; i++){
      রাশিমালা *p_রাশিমালা = pOrderBy->a[i].p_রাশিমালা;
      রাশিমালা *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      যদি( sqlite3_রাশিমালা_Is_Constant(0, p_রাশিমালা) ){
        চালু;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      যদি( pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) বিরতি;

      /* First case - a direct column references without a COLLATE operator */
      যদি( p_রাশিমালা->op==TK_COLUMN && p_রাশিমালা->iTable==pSrc->iCursor ){
        প্রতিজ্ঞা( p_রাশিমালা->iColumn>=XN_ROWID && p_রাশিমালা->iColumn<pTab->nCol );
        চালু;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      যদি( p_রাশিমালা->op==TK_COLLATE
       && (pE2 = p_রাশিমালা->p_বাম)->op==TK_COLUMN
       && pE2->iTable==pSrc->iCursor
      ){
        ধ্রুবক অক্ষর *zColl;  /* The collating sequence name */
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
        প্রতিজ্ঞা( p_রাশিমালা->u.zToken!=0 );
        প্রতিজ্ঞা( pE2->iColumn>=XN_ROWID && pE2->iColumn<pTab->nCol );
        p_রাশিমালা->iColumn = pE2->iColumn;
        যদি( pE2->iColumn<0 ) চালু;  /* Collseq does not matter for rowid */
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        যদি( zColl==0 ) zColl = sqlite3_Str_দ্বিমিক;
        যদি( sqlite3_stricmp(p_রাশিমালা->u.zToken, zColl)==0 ) চালু;
      }

      /* No matches cause a break out of the loop */
      বিরতি;
    }
    যদি( i==n ){
      পূর্ণসংখ্যা bSortByGroup = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      nOrderBy = n;
      যদি( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowidUsed ){
        eDistinct = 2 + bSortByGroup;
      }অন্যথা যদি( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - bSortByGroup;
      }অন্যথা যদি( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, আকার(*pIdxInfo)
                           + (আকার(*pIdxCons) + আকার(*pUsage))*nTerm
                           + আকার(*pIdxOrderBy)*nOrderBy
                           + SZ_HIDDENINDEXINFO(nTerm) );
  যদি( pIdxInfo==0 ){
    sqlite3_Error_বার্তা(pParse, "out of memory");
    ফেরত 0;
  }
  pHidden = (কাঠামো Hidden_সূচক_Info*)&pIdxInfo[1];
  pIdxCons = (কাঠামো sqlite3_সূচক_constraint*)&pHidden->aRhs[nTerm];
  pIdxOrderBy = (কাঠামো sqlite3_সূচক_orderby*)&pIdxCons[nTerm];
  pUsage = (কাঠামো sqlite3_সূচক_constraint_usage*)&pIdxOrderBy[nOrderBy];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->aOrderBy = pIdxOrderBy;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->colUsed = (sqlite3_int64)pSrc->colUsed;
  যদি( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    সূচক *pPk = sqlite3_Primary_চাবি_সূচক((Table*)pTab);
    প্রতিজ্ঞা( pPk!=0 );
    জন্য(i=0; i<pPk->n_চাবি_Col; i++){
      পূর্ণসংখ্যা iCol = pPk->aiColumn[i];
      প্রতিজ্ঞা( iCol>=0 );
      যদি( iCol>=BMS-1 ) iCol = BMS-1;
      pIdxInfo->colUsed |= MASKBIT(iCol);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->mIn = 0;
  জন্য(p=pWC, i=j=0; p; p=p->pOuter){
    পূর্ণসংখ্যা nLast = i+p->nTerm;;
    জন্য(pTerm=p->a; i<nLast; i++, pTerm++){
      u16 op;
      যদি( (pTerm->wtFlags & TERM_OK)==0 ) চালু;
      pIdxCons[j].iColumn = pTerm->u.x.বাম_Column;
      pIdxCons[j].iTermOffset = i;
      op = pTerm->eOperator & WO_ALL;
      যদি( op==WO_IN ){
        যদি( (pTerm->wtFlags & TERM_SLICE)==0 ){
          pHidden->mIn |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      যদি( op==WO_AUX ){
        pIdxCons[j].op = pTerm->eMatchOp;
      }অন্যথা যদি( op & (WO_ISNULL|WO_IS) ){
        যদি( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_সূচক_CONSTRAINT_ISNULL;
        }অন্যথা{
          pIdxCons[j].op = SQLITE_সূচক_CONSTRAINT_IS;
        }
      }অন্যথা{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        প্রতিজ্ঞা( WO_EQ==SQLITE_সূচক_CONSTRAINT_EQ );
        প্রতিজ্ঞা( WO_LT==SQLITE_সূচক_CONSTRAINT_LT );
        প্রতিজ্ঞা( WO_LE==SQLITE_সূচক_CONSTRAINT_LE );
        প্রতিজ্ঞা( WO_GT==SQLITE_সূচক_CONSTRAINT_GT );
        প্রতিজ্ঞা( WO_GE==SQLITE_সূচক_CONSTRAINT_GE );
        প্রতিজ্ঞা( pTerm->eOperator&(WO_IN|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        যদি( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_রাশিমালা_Is_Vector(pTerm->p_রাশিমালা->p_ডান)
          ){
          testcase( j!=i );
          যদি( j<16 ) mNoOmit |= (1 << j);
          যদি( op==WO_LT ) pIdxCons[j].op = WO_LE;
          যদি( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  প্রতিজ্ঞা( j==nTerm );
  pIdxInfo->nConstraint = j;
  জন্য(i=j=0; i<nOrderBy; i++){
    রাশিমালা *p_রাশিমালা = pOrderBy->a[i].p_রাশিমালা;
    যদি( sqlite3_রাশিমালা_Is_Constant(0, p_রাশিমালা) ) চালু;
    প্রতিজ্ঞা( p_রাশিমালা->op==TK_COLUMN
         || (p_রাশিমালা->op==TK_COLLATE && p_রাশিমালা->p_বাম->op==TK_COLUMN
              && p_রাশিমালা->iColumn==p_রাশিমালা->p_বাম->iColumn) );
    pIdxOrderBy[j].iColumn = p_রাশিমালা->iColumn;
    pIdxOrderBy[j].desc = pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC;
    j++;
  }
  pIdxInfo->nOrderBy = j;

  *pmNoOmit = mNoOmit;
  ফেরত pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
স্থির শূন্য freeIdxStr(sqlite3_সূচক_info *pIdxInfo){
  যদি( pIdxInfo->needToFreeIdxStr ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->needToFreeIdxStr = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
স্থির শূন্য free_সূচক_Info(sqlite3 *db, sqlite3_সূচক_info *pIdxInfo){
  Hidden_সূচক_Info *pHidden;
  পূর্ণসংখ্যা i;
  প্রতিজ্ঞা( pIdxInfo!=0 );
  pHidden = (Hidden_সূচক_Info*)&pIdxInfo[1];
  প্রতিজ্ঞা( pHidden->pParse!=0 );
  প্রতিজ্ঞা( pHidden->pParse->db==db );
  জন্য(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_মান_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
স্থির পূর্ণসংখ্যা vtab_Best_সূচক(Parse *pParse, Table *pTab, sqlite3_সূচক_info *p){
  পূর্ণসংখ্যা rc;
  sqlite3_vtab *pVtab;

  প্রতিজ্ঞা( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_সূচক_Info_Inputs(p, pTab);
  pParse->db->nSchemaLock++;
  rc = pVtab->pModule->x_Best_সূচক(pVtab, p);
  pParse->db->nSchemaLock--;
  where_Trace_সূচক_Info_Outputs(p, pTab);

  যদি( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    যদি( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }অন্যথা যদি( !pVtab->z_Err_বার্তা ){
      sqlite3_Error_বার্তা(pParse, "%s", sqlite3ErrStr(rc));
    }অন্যথা{
      sqlite3_Error_বার্তা(pParse, "%s", pVtab->z_Err_বার্তা);
    }
  }
  যদি( pTab->u.vtab.p->bAllSchemas ){
    sqlite3VtabUsesAllSchemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_বার্তা);
  pVtab->z_Err_বার্তা = 0;
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা where_চাবি_Stats(
  Parse *pParse,              /* Database connection */
  সূচক *pIdx,                /* Index to consider domain of */
  UnpackedRecord *pRec,       /* Vector of values to consider */
  পূর্ণসংখ্যা roundUp,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  সূচক_Sample *aSample = pIdx->aSample;
  পূর্ণসংখ্যা iCol;                   /* Index of required stats in anEq[] etc. */
  পূর্ণসংখ্যা i;                      /* Index of first sample >= pRec */
  পূর্ণসংখ্যা iSample;                /* Smallest sample larger than or equal to pRec */
  পূর্ণসংখ্যা iMin = 0;               /* Smallest sample not yet tested */
  পূর্ণসংখ্যা iTest;                  /* Next sample to test */
  পূর্ণসংখ্যা res;                    /* Result of comparison operation */
  পূর্ণসংখ্যা nField;                 /* Number of fields in pRec */
  tRowcnt iLower = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER( pParse );
#endif
  প্রতিজ্ঞা( pRec!=0 );
  প্রতিজ্ঞা( pIdx->nSample>0 );
  প্রতিজ্ঞা( pRec->nField>0 );


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
  যদি( !HasRowid(pIdx->pTable) && Is_Primary_চাবি_সূচক(pIdx) ){
    nField = pIdx->n_চাবি_Col;
  }অন্যথা{
    nField = pIdx->nColumn;
  }
  nField = MIN(pRec->nField, nField);
  iCol = 0;
  iSample = pIdx->nSample * nField;
  করো{
    পূর্ণসংখ্যা iSamp;                    /* Index in aSample[] of test sample */
    পূর্ণসংখ্যা n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / nField;
    যদি( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      জন্য(n=(iTest % nField) + 1; n<nField; n++){
        যদি( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) বিরতি;
      }
    }অন্যথা{
      n = iTest + 1;
    }

    pRec->nField = n;
    res = sqlite3VdbeRecordCompare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    যদি( res<0 ){
      iLower = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }অন্যথা যদি( res==0 && n<nField ){
      iLower = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }অন্যথা{
      iSample = iTest;
      iCol = n-1;
    }
  }যখন( res && iMin<iSample );
  i = iSample / nField;

#ifdef SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  যদি( pParse->db->mallocFailed==0 ){
    যদি( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      প্রতিজ্ঞা( i<pIdx->nSample );
      প্রতিজ্ঞা( iCol==nField-1 );
      pRec->nField = nField;
      প্রতিজ্ঞা( 0==sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }অন্যথা{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      প্রতিজ্ঞা( i<=pIdx->nSample && i>=0 );
      pRec->nField = iCol+1;
      প্রতিজ্ঞা( i==pIdx->nSample
           || sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      যদি( iCol>0 ){
        pRec->nField = iCol;
        প্রতিজ্ঞা( sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      যদি( i>0 ){
        pRec->nField = nField;
        প্রতিজ্ঞা( sqlite3VdbeRecordCompare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#endif /* ifdef SQLITE_DEBUG */

  যদি( res==0 ){
    /* Record pRec is equal to sample i */
    প্রতিজ্ঞা( iCol==nField-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }অন্যথা{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt iUpper, iGap;
    যদি( i>=pIdx->nSample ){
      iUpper = pIdx->nRowEst0;
    }অন্যথা{
      iUpper = aSample[i].anLt[iCol];
    }

    যদি( iLower>=iUpper ){
      iGap = 0;
    }অন্যথা{
      iGap = iUpper - iLower;
    }
    যদি( roundUp ){
      iGap = (iGap*2)/3;
    }অন্যথা{
      iGap = iGap/3;
    }
    aStat[0] = iLower + iGap;
    aStat[1] = pIdx->aAvgEq[nField-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->nField = nField;
  ফেরত i;
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
স্থির LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew){
  LogEst nRet = nNew;
  যদি( pTerm ){
    যদি( pTerm->truthProb<=0 ){
      nRet += pTerm->truthProb;
    }অন্যথা যদি( (pTerm->wtFlags & TERM_VNULL)==0 ){
      nRet -= 20;        প্রতিজ্ঞা( 20==sqlite3LogEst(4) );
    }
  }
  ফেরত nRet;
}


#ifdef SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
অক্ষর sqlite3_সূচক_Column_Affinity(sqlite3 *db, সূচক *pIdx, পূর্ণসংখ্যা iCol){
  প্রতিজ্ঞা( iCol>=0 && iCol<pIdx->nColumn );
  যদি( !pIdx->zColAff ){
    যদি( sqlite3_সূচক_Affinity_Str(db, pIdx)==0 ) ফেরত SQLITE_AFF_BLOB;
  }
  প্রতিজ্ঞা( pIdx->zColAff[iCol]!=0 );
  ফেরত pIdx->zColAff[iCol];
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
স্থির পূর্ণসংখ্যা whereRangeSkipScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  পূর্ণসংখ্যা *pbDone          /* Set to true if at least one expr. value extracted */
){
  সূচক *p = pLoop->u.btree.p_সূচক;
  পূর্ণসংখ্যা nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  পূর্ণসংখ্যা nLower = -1;
  পূর্ণসংখ্যা nUpper = p->nSample+1;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  u8 aff = sqlite3_সূচক_Column_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_মান *p1 = 0;          /* Value extracted from pLower */
  sqlite3_মান *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_মান *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  যদি( pLower ){
    rc = sqlite3_Stat4_মান_From_রাশিমালা(pParse, pLower->p_রাশিমালা->p_ডান, aff, &p1);
    nLower = 0;
  }
  যদি( pUpper && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_মান_From_রাশিমালা(pParse, pUpper->p_রাশিমালা->p_ডান, aff, &p2);
    nUpper = p2 ? 0 : p->nSample;
  }

  যদি( p1 || p2 ){
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা nDiff;
    জন্য(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      যদি( rc==SQLITE_OK && p1 ){
        পূর্ণসংখ্যা res = sqlite3MemCompare(p1, pVal, pColl);
        যদি( res>=0 ) nLower++;
      }
      যদি( rc==SQLITE_OK && p2 ){
        পূর্ণসংখ্যা res = sqlite3MemCompare(p2, pVal, pColl);
        যদি( res>=0 ) nUpper++;
      }
    }
    nDiff = (nUpper - nLower);
    যদি( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    যদি( nDiff!=1 || pUpper==0 || pLower==0 ){
      পূর্ণসংখ্যা nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->nOut -= nAdjust;
      *pbDone = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           nLower, nUpper, nAdjust*-1, pLoop->nOut));
    }

  }অন্যথা{
    প্রতিজ্ঞা( *pbDone==0 );
  }

  sqlite3_মান_Free(p1);
  sqlite3_মান_Free(p2);
  sqlite3_মান_Free(pVal);

  ফেরত rc;
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
স্থির পূর্ণসংখ্যা whereRangeScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা nOut = pLoop->nOut;
  LogEst nNew;

#ifdef SQLITE_ENABLE_STAT4
  সূচক *p = pLoop->u.btree.p_সূচক;
  পূর্ণসংখ্যা nEq = pLoop->u.btree.nEq;

  যদি( p->nSample>0 && ALWAYS(nEq<p->nSampleCol)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    যদি( nEq==pBuilder->nRecValid ){
      UnpackedRecord *pRec = pBuilder->pRec;
      tRowcnt a[2];
      পূর্ণসংখ্যা nBtm = pLoop->u.btree.nBtm;
      পূর্ণসংখ্যা n_শীর্ষ = pLoop->u.btree.n_শীর্ষ;

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
      পূর্ণসংখ্যা iLwrIdx = -2;   /* aSample[] for the lower bound */
      পূর্ণসংখ্যা iUprIdx = -1;   /* aSample[] for the upper bound */

      যদি( pRec ){
        testcase( pRec->nField!=pBuilder->nRecValid );
        pRec->nField = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      যদি( nEq==0 ){
        iLower = 0;
        iUpper = p->nRowEst0;
      }অন্যথা{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_চাবি_Stats(pParse, p, pRec, 0, a);
        iLower = a[0];
        iUpper = a[0] + a[1];
      }

      প্রতিজ্ঞা( pLower==0 || (pLower->eOperator & (WO_GT|WO_GE))!=0 );
      প্রতিজ্ঞা( pUpper==0 || (pUpper->eOperator & (WO_LT|WO_LE))!=0 );
      প্রতিজ্ঞা( p->aSortOrder!=0 );
      যদি( p->aSortOrder[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(WhereTerm*, pLower, pUpper);
        SWAP(পূর্ণসংখ্যা, nBtm, n_শীর্ষ);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      যদি( pLower ){
        পূর্ণসংখ্যা n;                    /* Values extracted from pExpr */
        রাশিমালা *p_রাশিমালা = pLower->p_রাশিমালা->p_ডান;
        rc = sqlite3_Stat4_Probe_Set_মান(pParse, p, &pRec, p_রাশিমালা, nBtm, nEq, &n);
        যদি( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          যদি( sqlite3_রাশিমালা_Vector_আকার(p_রাশিমালা)>n ) mask = (WO_LE|WO_LT);
          iLwrIdx = where_চাবি_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((pLower->eOperator & mask) ? a[1] : 0);
          যদি( iNew>iLower ) iLower = iNew;
          nOut--;
          pLower = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      যদি( pUpper ){
        পূর্ণসংখ্যা n;                    /* Values extracted from pExpr */
        রাশিমালা *p_রাশিমালা = pUpper->p_রাশিমালা->p_ডান;
        rc = sqlite3_Stat4_Probe_Set_মান(pParse, p, &pRec, p_রাশিমালা, n_শীর্ষ, nEq, &n);
        যদি( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 mask = WO_GT|WO_LE;
          যদি( sqlite3_রাশিমালা_Vector_আকার(p_রাশিমালা)>n ) mask = (WO_LE|WO_LT);
          iUprIdx = where_চাবি_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((pUpper->eOperator & mask) ? a[1] : 0);
          যদি( iNew<iUpper ) iUpper = iNew;
          nOut--;
          pUpper = 0;
        }
      }

      pBuilder->pRec = pRec;
      যদি( rc==SQLITE_OK ){
        যদি( iUpper>iLower ){
          nNew = sqlite3LogEst(iUpper - iLower);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          যদি( iLwrIdx==iUprIdx ){ nNew -= 20; }
          প্রতিজ্ঞা( 20==sqlite3LogEst(4) );
        }অন্যথা{
          nNew = 10;        প্রতিজ্ঞা( 10==sqlite3LogEst(2) );
        }
        যদি( nNew<nOut ){
          nOut = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)iLower, (u32)iUpper, nOut));
      }
    }অন্যথা{
      পূর্ণসংখ্যা bDone = 0;
      rc = whereRangeSkipScanEst(pParse, pLower, pUpper, pLoop, &bDone);
      যদি( bDone ) ফেরত rc;
    }
  }
#অন্যথা
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pBuilder);
  প্রতিজ্ঞা( pLower || pUpper );
#endif
  প্রতিজ্ঞা( pUpper==0 || (pUpper->wtFlags & TERM_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  যদি( pLower && pLower->truthProb>0 && pUpper && pUpper->truthProb>0 ){
    nNew -= 20;
  }

  nOut -= (pLower!=0) + (pUpper!=0);
  যদি( nNew<10 ) nNew = 10;
  যদি( nNew<nOut ) nOut = nNew;
#যদি defined(WHERETRACE_ENABLED)
  যদি( pLoop->nOut>nOut ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->nOut, nOut));
  }
#endif
  pLoop->nOut = (LogEst)nOut;
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা whereEqualScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  রাশিমালা *p_রাশিমালা,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  সূচক *p = pBuilder->pNew->u.btree.p_সূচক;
  পূর্ণসংখ্যা nEq = pBuilder->pNew->u.btree.nEq;
  UnpackedRecord *pRec = pBuilder->pRec;
  পূর্ণসংখ্যা rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  পূর্ণসংখ্যা bOk;

  প্রতিজ্ঞা( nEq>=1 );
  প্রতিজ্ঞা( nEq<=p->nColumn );
  প্রতিজ্ঞা( p->aSample!=0 );
  প্রতিজ্ঞা( p->nSample>0 );
  প্রতিজ্ঞা( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  যদি( pBuilder->nRecValid<(nEq-1) ){
    ফেরত SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  যদি( nEq>=p->nColumn ){
    *pnRow = 1;
    ফেরত SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_Set_মান(pParse, p, &pRec, p_রাশিমালা, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  যদি( rc!=SQLITE_OK ) ফেরত rc;
  যদি( bOk==0 ) ফেরত SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_চাবি_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (পূর্ণসংখ্যা)a[1]));
  *pnRow = a[1];
 
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা whereInScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  রাশিমালা_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  সূচক *p = pBuilder->pNew->u.btree.p_সূচক;
  i64 nRow0 = sqlite3LogEstToInt(p->aiRowLogEst[0]);
  পূর্ণসংখ্যা nRecValid = pBuilder->nRecValid;
  পূর্ণসংখ্যা rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  পূর্ণসংখ্যা i;                  /* Loop counter */

  প্রতিজ্ঞা( p->aSample!=0 );
  জন্য(i=0; rc==SQLITE_OK && i<pList->n_রাশিমালা; i++){
    nEst = nRow0;
    rc = whereEqualScanEst(pParse, pBuilder, pList->a[i].p_রাশিমালা, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  যদি( rc==SQLITE_OK ){
    যদি( nRowEst > (tRowcnt)nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", nRowEst));
  }
  প্রতিজ্ঞা( pBuilder->nRecValid==nRecValid );
  ফেরত rc;
}
#endif /* SQLITE_ENABLE_STAT4 */


#যদি defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
শূন্য sqlite3WhereTermPrint(WhereTerm *pTerm, পূর্ণসংখ্যা iTerm){
  যদি( pTerm==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", iTerm);
  }অন্যথা{
    অক্ষর zType[8];
    অক্ষর z_বাম[50];
    memcpy(zType, "....", 5);
    যদি( pTerm->wtFlags & TERM_VIRTUAL ) zType[0] = 'V';
    যদি( pTerm->eOperator & WO_EQUIV  ) zType[1] = 'E';
    যদি( রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON) ) zType[2] = 'L';
    যদি( pTerm->wtFlags & TERM_CODED  ) zType[3] = 'C';
    যদি( pTerm->eOperator & WO_SINGLE ){
      প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(আকার(z_বাম),z_বাম,"left={%d:%d}",
                       pTerm->বাম_Cursor, pTerm->u.x.বাম_Column);
    }অন্যথা যদি( (pTerm->eOperator & WO_OR)!=0 && pTerm->u.pOrInfo!=0 ){
      sqlite3_snprintf(আকার(z_বাম),z_বাম,"indexable=0x%llx",
                       pTerm->u.pOrInfo->indexable);
    }অন্যথা{
      sqlite3_snprintf(আকার(z_বাম),z_বাম,"left=%d", pTerm->বাম_Cursor);
    }
    iTerm = pTerm->iTerm = MAX(iTerm,pTerm->iTerm);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       iTerm, pTerm, zType, z_বাম, pTerm->eOperator, pTerm->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    যদি( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        pTerm->truthProb, (u64)pTerm->prereqAll, (u64)pTerm->prereq_ডান);
    }
    যদি( (pTerm->eOperator & (WO_OR|WO_AND))==0 && pTerm->u.x.iField ){
      sqlite3DebugPrintf(" iField=%d", pTerm->u.x.iField);
    }
    যদি( pTerm->iParent>=0 ){
      sqlite3DebugPrintf(" iParent=%d", pTerm->iParent);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_বৃক্ষ_View_রাশিমালা(0, pTerm->p_রাশিমালা, 0);
  }
}
শূন্য sqlite3ShowWhereTerm(WhereTerm *pTerm){
  sqlite3WhereTermPrint(pTerm, 0);
}
#endif

#ifdef WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
শূন্য sqlite3WhereClausePrint(WhereClause *pWC){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<pWC->nTerm; i++){
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
শূন্য sqlite3WhereLoopPrint(ধ্রুবক WhereLoop *p, ধ্রুবক WhereClause *pWC){
  WhereInfo *pWInfo;
  যদি( pWC ){
    পূর্ণসংখ্যা nb;
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
  }অন্যথা{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->maskSelf, p->prereq & 0xfff, p->cId, p->iTab);
  }
  যদি( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ধ্রুবক অক্ষর *zName;
    যদি( p->u.btree.p_সূচক && (zName = p->u.btree.p_সূচক->zName)!=0 ){
      যদি( strncmp(zName, "sqlite_autoindex_", 17)==0 ){
        পূর্ণসংখ্যা i = sqlite3Strlen30(zName) - 1;
        যখন( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }অন্যথা{
      sqlite3DebugPrintf("%20s","");
    }
  }অন্যথা{
    অক্ষর *z;
    যদি( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_অঙ্ক, p->u.vtab.idxStr, p->u.vtab.omitMask);
    }অন্যথা{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_অঙ্ক, p->u.vtab.omitMask);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  যদি( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }অন্যথা{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  যদি( pWInfo && pWInfo->bStarUsed && p->rStarDelta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->rSetup, p->rRun, p->nOut, p->rStarDelta);
  }অন্যথা{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->rSetup, p->rRun, p->nOut);
  }
  যদি( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    পূর্ণসংখ্যা i;
    জন্য(i=0; i<p->nLTerm; i++){
      sqlite3WhereTermPrint(p->aLTerm[i], i);
    }
  }
}
শূন্য sqlite3ShowWhereLoop(ধ্রুবক WhereLoop *p){
  যদি( p ) sqlite3WhereLoopPrint(p, 0);
}
শূন্য sqlite3ShowWhereLoopList(ধ্রুবক WhereLoop *p){
  যখন( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_পরবর্তী_Loop;
  }
}
#endif

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
স্থির শূন্য whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_আকার(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
স্থির শূন্য whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  যদি( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_সূচক) ){
    যদি( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }অন্যথা যদি( (p->wsFlags & WHERE_AUTO_সূচক)!=0 && p->u.btree.p_সূচক!=0 ){
      sqlite3DbFree(db, p->u.btree.p_সূচক->zColAff);
      sqlite3DbFreeNN(db, p->u.btree.p_সূচক);
      p->u.btree.p_সূচক = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
স্থির শূন্য whereLoopClear(sqlite3 *db, WhereLoop *p){
  যদি( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_আকার(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
স্থির পূর্ণসংখ্যা whereLoopResize(sqlite3 *db, WhereLoop *p, পূর্ণসংখ্যা n){
  WhereTerm **paNew;
  যদি( p->nLSlot>=n ) ফেরত SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, আকার(p->aLTerm[0])*n);
  যদি( paNew==0 ) ফেরত SQLITE_NOMEM_BKPT;
  memcpy(paNew, p->aLTerm, আকার(p->aLTerm[0])*p->nLSlot);
  যদি( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  ফেরত SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
স্থির পূর্ণসংখ্যা whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom){
  whereLoopClearUnion(db, pTo);
  যদি( pFrom->nLTerm > pTo->nLSlot
   && whereLoopResize(db, pTo, pFrom->nLTerm)
  ){
    memset(pTo, 0, WHERE_LOOP_XFER_SZ);
    ফেরত SQLITE_NOMEM_BKPT;
  }
  memcpy(pTo, pFrom, WHERE_LOOP_XFER_SZ);
  memcpy(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm*আকার(pTo->aLTerm[0]));
  যদি( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }অন্যথা যদি( (pFrom->wsFlags & WHERE_AUTO_সূচক)!=0 ){
    pFrom->u.btree.p_সূচক = 0;
  }
  ফেরত SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
স্থির শূন্য whereLoopDelete(sqlite3 *db, WhereLoop *p){
  প্রতিজ্ঞা( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
স্থির শূন্য whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  প্রতিজ্ঞা( pWInfo!=0 );
  প্রতিজ্ঞা( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  যখন( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_পরবর্তী_Loop;
    whereLoopDelete(db, p);
  }
  যখন( pWInfo->pMemToFree ){
    WhereMemBlock *p_পরবর্তী = pWInfo->pMemToFree->p_পরবর্তী;
    sqlite3DbNNFreeNN(db, pWInfo->pMemToFree);
    pWInfo->pMemToFree = p_পরবর্তী;
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
স্থির পূর্ণসংখ্যা whereLoopCheaperProperSubset(
  ধ্রুবক WhereLoop *pX,       /* First WhereLoop to compare */
  ধ্রুবক WhereLoop *pY        /* Compare against this WhereLoop */
){
  পূর্ণসংখ্যা i, j;
  যদি( pX->rRun>pY->rRun && pX->nOut>pY->nOut ) ফেরত 0; /* (1d) and (2a) */
  প্রতিজ্ঞা( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  প্রতিজ্ঞা( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  যদি( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_সূচক==pY->u.btree.p_সূচক             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    ফেরত 1;  /* Case 1 is true */
  }
  যদি( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    ফেরত 0;                                            /* (2b) */
  }
  যদি( pY->nSkip > pX->nSkip ) ফেরত 0;                  /* (2d) */
  জন্য(i=pX->nLTerm-1; i>=0; i--){
    যদি( pX->aLTerm[i]==0 ) চালু;
    জন্য(j=pY->nLTerm-1; j>=0; j--){
      যদি( pY->aLTerm[j]==pX->aLTerm[i] ) বিরতি;
    }
    যদি( j<0 ) ফেরত 0;                                  /* (2c) */
  }
  যদি( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    ফেরত 0;                                            /* (2e) */
  }
  ফেরত 1;  /* Case 2 is true */
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
স্থির শূন্য whereLoopAdjustCost(ধ্রুবক WhereLoop *p, WhereLoop *pTemplate){
  যদি( (pTemplate->wsFlags & WHERE_INDEXED)==0 ) ফেরত;
  জন্য(; p; p=p->p_পরবর্তী_Loop){
    যদি( p->iTab!=pTemplate->iTab ) চালু;
    যদি( (p->wsFlags & WHERE_INDEXED)==0 ) চালু;
    যদি( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut,
                       MIN(p->rRun, pTemplate->rRun),
                       MIN(p->nOut - 1, pTemplate->nOut)));
      pTemplate->rRun = MIN(p->rRun, pTemplate->rRun);
      pTemplate->nOut = MIN(p->nOut - 1, pTemplate->nOut);
    }অন্যথা যদি( whereLoopCheaperProperSubset(pTemplate, p) ){
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
স্থির WhereLoop **whereLoopFindLesser(
  WhereLoop **pp_পূর্ব,
  ধ্রুবক WhereLoop *pTemplate
){
  WhereLoop *p;
  জন্য(p=(*pp_পূর্ব); p; pp_পূর্ব=&p->p_পরবর্তী_Loop, p=*pp_পূর্ব){
    যদি( p->iTab!=pTemplate->iTab || p->iSortIdx!=pTemplate->iSortIdx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      চালু;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    প্রতিজ্ঞা( p->rSetup==0 || pTemplate->rSetup==0
                 || p->rSetup==pTemplate->rSetup );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    প্রতিজ্ঞা( p->rSetup>=pTemplate->rSetup );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    যদি( (p->wsFlags & WHERE_AUTO_সূচক)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_INDEXED)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      বিরতি;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    যদি( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->rSetup<=pTemplate->rSetup                  /* (2a) */
     && p->rRun<=pTemplate->rRun                      /* (2b) */
     && p->nOut<=pTemplate->nOut                      /* (2c) */
    ){
      ফেরত 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    যদি( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->rRun>=pTemplate->rRun                             /* (2a) */
     && p->nOut>=pTemplate->nOut                             /* (2b) */
    ){
      প্রতিজ্ঞা( p->rSetup>=pTemplate->rSetup ); /* SETUP-INVARIANT above */
      বিরতি;   /* Cause p to be overwritten by pTemplate */
    }
  }
  ফেরত pp_পূর্ব;
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
স্থির পূর্ণসংখ্যা where_Loop_ঢোকাও(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **pp_পূর্ব, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  পূর্ণসংখ্যা rc;

  /* Stop the search once we hit the query planner search limit */
  যদি( pBuilder->iPlanLimit==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    যদি( pBuilder->pOrSet ) pBuilder->pOrSet->n = 0;
    ফেরত SQLITE_DONE;
  }
  pBuilder->iPlanLimit--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  যদি( pBuilder->pOrSet!=0 ){
    যদি( pTemplate->nLTerm ){
#যদি WHERETRACE_ENABLED
      u16 n = pBuilder->pOrSet->n;
      পূর্ণসংখ্যা x =
#endif
      where_Or_ঢোকাও(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun,
                                    pTemplate->nOut);
#যদি WHERETRACE_ENABLED /* 0x8 */
      যদি( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#endif
    }
    ফেরত SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  pp_পূর্ব = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  যদি( pp_পূর্ব==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#যদি WHERETRACE_ENABLED /* 0x8 */
    যদি( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#endif
    ফেরত SQLITE_OK; 
  }অন্যথা{
    p = *pp_পূর্ব;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#যদি WHERETRACE_ENABLED /* 0x8 */
  যদি( sqlite3WhereTrace & 0x8 ){
    যদি( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }অন্যথা{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#endif
  যদি( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *pp_পূর্ব = p = sqlite3DbMallocRawNN(db, আকার(WhereLoop));
    যদি( p==0 ) ফেরত SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_পরবর্তী_Loop = 0;
  }অন্যথা{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_লেজ = &p->p_পরবর্তী_Loop;
    WhereLoop *pToDel;
    যখন( *pp_লেজ ){
      pp_লেজ = whereLoopFindLesser(pp_লেজ, pTemplate);
      যদি( pp_লেজ==0 ) বিরতি;
      pToDel = *pp_লেজ;
      যদি( pToDel==0 ) বিরতি;
      *pp_লেজ = pToDel->p_পরবর্তী_Loop;
#যদি WHERETRACE_ENABLED /* 0x8 */
      যদি( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(pToDel, pBuilder->pWC);
      }
#endif
      whereLoopDelete(db, pToDel);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  যদি( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    সূচক *p_সূচক = p->u.btree.p_সূচক;
    যদি( p_সূচক && p_সূচক->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_সূচক = 0;
    }
  }
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা রাশিমালা_গ্রন্থি_Pattern_দৈর্ঘ্য_Est(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_STRING ){
    পূর্ণসংখ্যা sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_রাশিমালা->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    যদি( pWalker->eCode ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }অন্যথা{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    যখন( (c = *(z++))!=0 ){
      যদি( c==c3 ){
        যদি( *z ) z++;
        যখন( *z && *z!=']' ) z++;
      }অন্যথা যদি( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    যদি( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  ফেরত WRC_Continue;
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
স্থির পূর্ণসংখ্যা est_Like_Pattern_দৈর্ঘ্য(রাশিমালা *p, u16 eCode){
  Walker w;
  w.u.sz = 0;
  w.eCode = eCode;
  w.x_রাশিমালা_Callback = রাশিমালা_গ্রন্থি_Pattern_দৈর্ঘ্য_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_রাশিমালা(&w, p);
  ফেরত w.u.sz;
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
স্থির শূন্য whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst nRow            /* Number of rows in the entire table */
){
  WhereTerm *pTerm, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->maskSelf);
  পূর্ণসংখ্যা i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  যদি( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->pOrderBy==0 ){
    ফেরত;
  }

  প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_AUTO_সূচক)==0 );
  জন্য(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
    প্রতিজ্ঞা( pTerm!=0 );
    যদি( (pTerm->prereqAll & notAllowed)!=0 ) চালু;
    যদি( (pTerm->prereqAll & pLoop->maskSelf)==0 ) চালু;
    যদি( (pTerm->wtFlags & TERM_VIRTUAL)!=0 ) চালু;
    জন্য(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      যদি( pX==0 ) চালু;
      যদি( pX==pTerm ) বিরতি;
      যদি( pX->iParent>=0 && (&pWC->a[pX->iParent])==pTerm ) বিরতি;
    }
    যদি( j<0 ){
      sqlite3ProgressCheck(pWC->pWInfo->pParse);
      যদি( pLoop->maskSelf==pTerm->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        যদি( (pTerm->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_বাম|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      যদি( pTerm->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->nOut += pTerm->truthProb;
      }অন্যথা{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        রাশিমালা *p_Op_রাশিমালা = pTerm->p_রাশিমালা;
        pLoop->nOut--;
        যদি( (pTerm->eOperator&(WO_EQ|WO_IS))!=0
         && (pTerm->wtFlags & TERM_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          রাশিমালা *p_ডান = p_Op_রাশিমালা->p_ডান;
          Parse *pParse = pWC->pWInfo->pParse;
          পূর্ণসংখ্যা k = 0;
          testcase( p_Op_রাশিমালা->op==TK_IS );
          যদি( sqlite3_রাশিমালা_Is_Integer(p_ডান, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }অন্যথা{
            k = 20;
          }
          যদি( iReduce<k ){
            pTerm->wtFlags |= TERM_HEURTRUTH;
            iReduce = k;
          }
        }অন্যথা
        যদি( রাশিমালা_Has_Property(p_Op_রাশিমালা, EP_InfixFunc) 
         && p_Op_রাশিমালা->op==TK_FUNCTION
        ){
          পূর্ণসংখ্যা eOp;
          প্রতিজ্ঞা( রাশিমালা_Use_XList(p_Op_রাশিমালা) );
          প্রতিজ্ঞা( p_Op_রাশিমালা->x.pList->n_রাশিমালা>=2 );
          eOp = sqlite3_রাশিমালা_Is_Like_Operator(p_Op_রাশিমালা);
          যদি( ALWAYS(eOp>0) ){
            পূর্ণসংখ্যা szPattern;
            রাশিমালা *pRHS = p_Op_রাশিমালা->x.pList->a[0].p_রাশিমালা;
            eOp = eOp==SQLITE_সূচক_CONSTRAINT_LIKE;
            szPattern = est_Like_Pattern_দৈর্ঘ্য(pRHS, eOp);
            যদি( szPattern>0 ){
              pLoop->nOut -= szPattern*2;
            }
          }
        }
      }
    }
  }
  যদি( pLoop->nOut > nRow-iReduce ){
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
স্থির পূর্ণসংখ্যা whereRangeVectorLen(
  Parse *pParse,       /* Parsing context */
  পূর্ণসংখ্যা iCur,            /* Cursor open on pIdx */
  সূচক *pIdx,         /* The index to be used for a inequality constraint */
  পূর্ণসংখ্যা nEq,             /* Number of prior equality constraints on same index */
  WhereTerm *pTerm     /* The vector inequality constraint */
){
  পূর্ণসংখ্যা nCmp = sqlite3_রাশিমালা_Vector_আকার(pTerm->p_রাশিমালা->p_বাম);
  পূর্ণসংখ্যা i;

  nCmp = MIN(nCmp, (pIdx->nColumn - nEq));
  জন্য(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    অক্ষর aff;                     /* Comparison affinity */
    অক্ষর idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    রাশিমালা *pLhs, *pRhs;

    প্রতিজ্ঞা( রাশিমালা_Use_XList(pTerm->p_রাশিমালা->p_বাম) );
    pLhs = pTerm->p_রাশিমালা->p_বাম->x.pList->a[i].p_রাশিমালা;
    pRhs = pTerm->p_রাশিমালা->p_ডান;
    যদি( রাশিমালা_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_রাশিমালা;
    }অন্যথা{
      pRhs = pRhs->x.pList->a[i].p_রাশিমালা;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    যদি( pLhs->op!=TK_COLUMN
     || pLhs->iTable!=iCur
     || pLhs->iColumn!=pIdx->aiColumn[i+nEq]
     || pIdx->aSortOrder[i+nEq]!=pIdx->aSortOrder[nEq]
    ){
      বিরতি;
    }

    testcase( pLhs->iColumn==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_রাশিমালা_Affinity(pLhs));
    idxaff = sqlite3TableColumnAffinity(pIdx->pTable, pLhs->iColumn);
    যদি( aff!=idxaff ) বিরতি;

    যদি( রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_Commuted) ) SWAP(রাশিমালা*, pRhs, pLhs);
    pColl = sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, pLhs, pRhs);
    যদি( pColl==0 ) বিরতি;
    যদি( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) বিরতি;
  }
  ফেরত i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#ifdef SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#অন্যথা
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
স্থির পূর্ণসংখ্যা where_Loop_Add_Btree_সূচক(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  SrcItem *pSrc,                  /* FROM clause term being analyzed */
  সূচক *pProbe,                  /* An index on pSrc */
  LogEst nInMul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  WhereTerm *pTerm;               /* A WhereTerm under consideration */
  পূর্ণসংখ্যা opMask;                     /* Valid operators for constraints */
  WhereScan scan;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_শীর্ষ;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_nOut;              /* Original value of pNew->nOut */
  পূর্ণসংখ্যা rc = SQLITE_OK;             /* Return code */
  LogEst r_আকার;                   /* Number of rows in the table */
  LogEst r_Log_আকার;                /* Logarithm of table size */
  WhereTerm *p_শীর্ষ_2 = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  প্রতিজ্ঞা( db->mallocFailed==0 || pParse->nErr>0 );
  যদি( pParse->nErr ){
    ফেরত pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->pTable->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->rRun));

  প্রতিজ্ঞা( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  প্রতিজ্ঞা( (pNew->wsFlags & WHERE_শীর্ষ_LIMIT)==0 );
  যদি( pNew->wsFlags & WHERE_BTM_LIMIT ){
    opMask = WO_LT|WO_LE;
  }অন্যথা{
    প্রতিজ্ঞা( pNew->u.btree.nBtm==0 );
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  যদি( pProbe->bUnordered ){
    opMask &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  প্রতিজ্ঞা( pNew->u.btree.nEq<pProbe->nColumn );
  প্রতিজ্ঞা( pNew->u.btree.nEq<pProbe->n_চাবি_Col
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_শীর্ষ = pNew->u.btree.n_শীর্ষ;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_nOut = pNew->nOut;
  pTerm = whereScanInit(&scan, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        opMask, pProbe);
  pNew->rSetup = 0;
  r_আকার = pProbe->aiRowLogEst[0];
  r_Log_আকার = estLog(r_আকার);
  জন্য(; rc==SQLITE_OK && pTerm!=0; pTerm = where_Scan_পরবর্তী(&scan)){
    u16 eOp = pTerm->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst nOutUnadjusted;        /* nOut before IN() and WHERE adjustments */
    পূর্ণসংখ্যা nIn = 0;
#ifdef SQLITE_ENABLE_STAT4
    পূর্ণসংখ্যা nRecValid = pBuilder->nRecValid;
#endif
    যদি( (eOp==WO_ISNULL || (pTerm->wtFlags&TERM_VNULL)!=0)
     && সূচক_Column_Not_Null(pProbe, saved_nEq)
    ){
      চালু; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    যদি( pTerm->prereq_ডান & pNew->maskSelf ) চালু;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    যদি( pTerm->wtFlags & TERM_LIKEOPT && pTerm->eOperator==WO_LT ) চালু;

    যদি( (pSrc->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান))!=0
     && !constraintCompatibleWithOuterJoin(pTerm,pSrc)
    ){
      চালু;
    }
    যদি( Is_Unique_সূচক(pProbe) && saved_nEq==pProbe->n_চাবি_Col-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }অন্যথা{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_INDEXED;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_শীর্ষ = saved_n_শীর্ষ;
    pNew->nLTerm = saved_nLTerm;
    যদি( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       বিরতি; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereq_ডান) & ~pNew->maskSelf;

    প্রতিজ্ঞা( nInMul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0
        || (pNew->wsFlags & WHERE_COLUMN_IN)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    যদি( eOp & WO_IN ){
      রাশিমালা *p_রাশিমালা = pTerm->p_রাশিমালা;
      যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        পূর্ণসংখ্যা i;
        পূর্ণসংখ্যা bRedundant = 0;
        nIn = 46;  প্রতিজ্ঞা( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        জন্য(i=0; i<pNew->nLTerm-1; i++){
          যদি( pNew->aLTerm[i] && pNew->aLTerm[i]->p_রাশিমালা==p_রাশিমালা ){
            nIn = 0;
            যদি( pNew->aLTerm[i]->u.x.iField == pTerm->u.x.iField ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        যদি( bRedundant ){
          pNew->nLTerm--;
          চালু;
        }
      }অন্যথা যদি( ALWAYS(p_রাশিমালা->x.pList && p_রাশিমালা->x.pList->n_রাশিমালা) ){
        /* "x IN (value, value, ...)" */
        nIn = sqlite3LogEst(p_রাশিমালা->x.pList->n_রাশিমালা);
      }
      যদি( pProbe->hasStat1 && r_Log_আকার>=10 ){
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
        x = M + logK + 10 - (nIn + r_Log_আকার);
        যদি( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, nIn, r_Log_আকার, x));
        }অন্যথা যদি( nInMul<2 && OptimizationEnabled(db, SQLITE_SeekScan) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, nIn, r_Log_আকার, x, nInMul));
          pNew->wsFlags |= WHERE_IN_SEEKSCAN;
        }অন্যথা{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, nIn, r_Log_আকার, x, nInMul));
          চালু;
        }
      }
      pNew->wsFlags |= WHERE_COLUMN_IN;
    }অন্যথা যদি( eOp & (WO_EQ|WO_IS) ){
      পূর্ণসংখ্যা iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      প্রতিজ্ঞা( saved_nEq==pNew->u.btree.nEq );
      যদি( iCol==XN_ROWID
       || (iCol>=0 && nInMul==0 && saved_nEq==pProbe->n_চাবি_Col-1)
      ){
        যদি( iCol==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_চাবি_Col==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }অন্যথা{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      যদি( scan.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }অন্যথা যদি( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }অন্যথা{
      পূর্ণসংখ্যা nVecLen = whereRangeVectorLen(
          pParse, pSrc->iCursor, pProbe, saved_nEq, pTerm
      );
      যদি( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_LIMIT;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = pTerm;
        p_শীর্ষ_2 = 0;
        যদি( pTerm->wtFlags & TERM_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_শীর্ষ_2 = &pTerm[1];
          প্রতিজ্ঞা( (p_শীর্ষ_2-(pTerm->pWC->a))<pTerm->pWC->nTerm );
          প্রতিজ্ঞা( p_শীর্ষ_2->wtFlags & TERM_LIKEOPT );
          প্রতিজ্ঞা( p_শীর্ষ_2->eOperator==WO_LT );
          যদি( whereLoopResize(db, pNew, pNew->nLTerm+1) ) বিরতি; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_শীর্ষ_2;
          pNew->wsFlags |= WHERE_শীর্ষ_LIMIT;
          pNew->u.btree.n_শীর্ষ = 1;
        }
      }অন্যথা{
        প্রতিজ্ঞা( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_শীর্ষ_LIMIT;
        pNew->u.btree.n_শীর্ষ = nVecLen;
        p_শীর্ষ_2 = pTerm;
        pBtm = (pNew->wsFlags & WHERE_BTM_LIMIT)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    প্রতিজ্ঞা( pNew->nOut==saved_nOut );
    যদি( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      whereRangeScanEst(pParse, pBuilder, pBtm, p_শীর্ষ_2, pNew);
    }অন্যথা{
      পূর্ণসংখ্যা nEq = ++pNew->u.btree.nEq;
      প্রতিজ্ঞা( eOp & (WO_ISNULL|WO_EQ|WO_IN|WO_IS) );

      প্রতিজ্ঞা( pNew->nOut==saved_nOut );
      যদি( pTerm->truthProb<=0 && pProbe->aiColumn[saved_nEq]>=0 ){
        প্রতিজ্ঞা( (eOp & WO_IN) || nIn==0 );
        testcase( eOp & WO_IN );
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      }অন্যথা{
#ifdef SQLITE_ENABLE_STAT4
        tRowcnt nOut = 0;
        যদি( nInMul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->nSampleCol)
         && ((eOp & WO_IN)==0 || রাশিমালা_Use_XList(pTerm->p_রাশিমালা))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          রাশিমালা *p_রাশিমালা = pTerm->p_রাশিমালা;
          যদি( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = whereEqualScanEst(pParse, pBuilder, p_রাশিমালা->p_ডান, &nOut);
          }অন্যথা{
            rc = whereInScanEst(pParse, pBuilder, p_রাশিমালা->x.pList, &nOut);
          }
          যদি( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          যদি( rc!=SQLITE_OK ) বিরতি;          /* Jump out of the pTerm loop */
          যদি( nOut ){
            pNew->nOut = sqlite3LogEst(nOut);
            যদি( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->nOut+10 > pProbe->aiRowLogEst[0]
            ){
#যদি WHERETRACE_ENABLED /* 0x01 */
              যদি( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3WhereTermPrint(pTerm, 999);
              }
#endif
              pTerm->wtFlags |= TERM_HIGHTRUTH;
              যদি( pTerm->wtFlags & TERM_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            যদি( pNew->nOut>saved_nOut ) pNew->nOut = saved_nOut;
            pNew->nOut -= nIn;
          }
        }
        যদি( nOut==0 )
#endif
        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          যদি( eOp & WO_ISNULL ){
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
    প্রতিজ্ঞা( pSrc->pSTab->szTabRow>0 );
    যদি( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      rCostIdx = pNew->nOut + 16;
    }অন্যথা{
      rCostIdx = pNew->nOut + 1 + (15*pProbe->szIdxRow)/pSrc->pSTab->szTabRow;
    }
    rCostIdx = sqlite3LogEstAdd(r_Log_আকার, rCostIdx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->rRun = rCostIdx;
    যদি( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    }
    ApplyCostMultiplier(pNew->rRun, pProbe->pTable->costMult);

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_আকার);
    যদি( pSrc->fg.fromExists ) pNew->nOut = 0;
    rc = where_Loop_ঢোকাও(pBuilder, pNew);

    যদি( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->nOut = saved_nOut;
    }অন্যথা{
      pNew->nOut = nOutUnadjusted;
    }

    যদি( (pNew->wsFlags & WHERE_শীর্ষ_LIMIT)==0
     && pNew->u.btree.nEq<pProbe->nColumn
     && (pNew->u.btree.nEq<pProbe->n_চাবি_Col ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      যদি( pNew->u.btree.nEq>3 ){
        sqlite3ProgressCheck(pParse);
      }
      where_Loop_Add_Btree_সূচক(pBuilder, pSrc, pProbe, nInMul+nIn);
    }
    pNew->nOut = saved_nOut;
#ifdef SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#endif
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_শীর্ষ = saved_n_শীর্ষ;
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
  প্রতিজ্ঞা( 42==sqlite3LogEst(18) );
  যদি( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_চাবি_Col
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
    where_Loop_Add_Btree_সূচক(pBuilder, pSrc, pProbe, nIter + nInMul);
    pNew->nOut = saved_nOut;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->pTable->zName, pProbe->zName, saved_nEq, rc));
  ফেরত rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
স্থির পূর্ণসংখ্যা সূচক_Might_Help_With_Order_By(
  WhereLoopBuilder *pBuilder,
  সূচক *p_সূচক,
  পূর্ণসংখ্যা iCursor
){
  রাশিমালা_List *pOB;
  রাশিমালা_List *a_Col_রাশিমালা;
  পূর্ণসংখ্যা ii, jj;

  যদি( p_সূচক->bUnordered ) ফেরত 0;
  যদি( (pOB = pBuilder->pWInfo->pOrderBy)==0 ) ফেরত 0;
  জন্য(ii=0; ii<pOB->n_রাশিমালা; ii++){
    রাশিমালা *p_রাশিমালা = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pOB->a[ii].p_রাশিমালা);
    যদি( NEVER(p_রাশিমালা==0) ) চালু;
    যদি( (p_রাশিমালা->op==TK_COLUMN || p_রাশিমালা->op==TK_AGG_COLUMN) 
     && p_রাশিমালা->iTable==iCursor 
    ){
      যদি( p_রাশিমালা->iColumn<0 ) ফেরত 1;
      জন্য(jj=0; jj<p_সূচক->n_চাবি_Col; jj++){
        যদি( p_রাশিমালা->iColumn==p_সূচক->aiColumn[jj] ) ফেরত 1;
      }
    }অন্যথা যদি( (a_Col_রাশিমালা = p_সূচক->a_Col_রাশিমালা)!=0 ){
      জন্য(jj=0; jj<p_সূচক->n_চাবি_Col; jj++){
        যদি( p_সূচক->aiColumn[jj]!=XN_রাশিমালা ) চালু;
        যদি( sqlite3_রাশিমালা_Compare_Skip(p_রাশিমালা,a_Col_রাশিমালা->a[jj].p_রাশিমালা,iCursor)==0 ){
          ফেরত 1;
        }
      }
    }
  }
  ফেরত 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
স্থির পূর্ণসংখ্যা where_Usable_Partial_সূচক(
  পূর্ণসংখ্যা iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  রাশিমালা *pWhere          /* The WHERE clause from the partial index */
){
  পূর্ণসংখ্যা i;
  WhereTerm *pTerm;
  Parse *pParse;

  যদি( jointype & JT_LTORJ ) ফেরত 0;
  pParse = pWC->pWInfo->pParse;
  যখন( pWhere->op==TK_AND ){
    যদি( !where_Usable_Partial_সূচক(iTab,jointype,pWC,pWhere->p_বাম) ) ফেরত 0;
    pWhere = pWhere->p_ডান;
  }
  জন্য(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    রাশিমালা *p_রাশিমালা;
    p_রাশিমালা = pTerm->p_রাশিমালা;
    যদি( (!রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) || p_রাশিমালা->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON))
     && sqlite3_রাশিমালা_Implies_রাশিমালা(pParse, p_রাশিমালা, pWhere, iTab)
     && !sqlite3_রাশিমালা_Implies_রাশিমালা(pParse, p_রাশিমালা, pWhere, -1)
     && (pTerm->wtFlags & TERM_VNULL)==0
    ){
      ফেরত 1;
    }
  }
  ফেরত 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
স্থির পূর্ণসংখ্যা রাশিমালা_Is_Covered_By_সূচক(
  ধ্রুবক রাশিমালা *p_রাশিমালা,
  ধ্রুবক সূচক *pIdx,
  পূর্ণসংখ্যা iTabCur
){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<pIdx->nColumn; i++){
    যদি( pIdx->aiColumn[i]==XN_রাশিমালা
     && sqlite3_রাশিমালা_Compare(0, p_রাশিমালা, pIdx->a_Col_রাশিমালা->a[i].p_রাশিমালা, iTabCur)==0
    ){
      ফেরত 1;
    }
  }
  ফেরত 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
প্রকারলেখ কাঠামো Covering_সূচক_Check Covering_সূচক_Check;
কাঠামো Covering_সূচক_Check {
  সূচক *pIdx;       /* The index */
  পূর্ণসংখ্যা iTabCur;       /* Cursor number for the corresponding table */
  u8 b_রাশিমালা;          /* Uses an indexed expression */
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
স্থির পূর্ণসংখ্যা where_Is_Covering_সূচক_Walk_Callback(Walker *pWalk, রাশিমালা *p_রাশিমালা){
  পূর্ণসংখ্যা i;                    /* Loop counter */
  ধ্রুবক সূচক *pIdx;        /* The index of interest */
  ধ্রুবক i16 *aiColumn;      /* Columns contained in the index */
  u16 nColumn;              /* Number of columns in the index */
  Covering_সূচক_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  যদি( (p_রাশিমালা->op==TK_COLUMN || p_রাশিমালা->op==TK_AGG_COLUMN) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    যদি( p_রাশিমালা->iTable!=pCk->iTabCur ) ফেরত WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    জন্য(i=0; i<nColumn; i++){
      যদি( aiColumn[i]==p_রাশিমালা->iColumn ) ফেরত WRC_Continue;
    }
    pCk->bUnidx = 1;
    ফেরত WRC_Abort;
  }অন্যথা যদি( pIdx->b_Has_রাশিমালা
         && রাশিমালা_Is_Covered_By_সূচক(p_রাশিমালা, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_রাশিমালা = 1;
    ফেরত WRC_Prune;
  }
  ফেরত WRC_Continue;
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
স্থির SQLITE_NOINLINE u32 where_Is_Covering_সূচক(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  সূচক *pIdx,           /* Index that is being tested */
  পূর্ণসংখ্যা iTabCur            /* Cursor for the table being indexed */
){
  পূর্ণসংখ্যা i, rc;
  কাঠামো Covering_সূচক_Check ck;
  Walker w;
  যদি( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    ফেরত 0;
  }
  যদি( pIdx->b_Has_রাশিমালা==0 ){
    জন্য(i=0; i<pIdx->nColumn; i++){
      যদি( pIdx->aiColumn[i]>=BMS-1 ) বিরতি;
    }
    যদি( i>=pIdx->nColumn ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      ফেরত 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_রাশিমালা = 0;
  ck.bUnidx = 0;
  memset(&w, 0, আকার(w));
  w.x_রাশিমালা_Callback = where_Is_Covering_সূচক_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  যদি( ck.bUnidx ){
    rc = 0;
  }অন্যথা যদি( ck.b_রাশিমালা ){
    rc = WHERE_EXPRIDX;
  }অন্যথা{
    rc = WHERE_IDX_ONLY;
  }
  ফেরত rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
স্থির শূন্য where_Indexed_রাশিমালা_Cleanup(sqlite3 *db, শূন্য *pObject){
  Indexed_রাশিমালা **pp = (Indexed_রাশিমালা**)pObject;
  যখন( *pp!=0 ){
    Indexed_রাশিমালা *p = *pp;
    *pp = p->pIENext;
    sqlite3_রাশিমালা_Delete(db, p->p_রাশিমালা);
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
স্থির শূন্য where_Part_Idx_রাশিমালা(
  Parse *pParse,                  /* Parse context */
  সূচক *pIdx,                    /* Partial index being processed */
  রাশিমালা *pPart,                    /* WHERE clause being processed */
  Bitmask *pMask,                 /* Mask to clear bits in */
  পূর্ণসংখ্যা iIdxCur,                    /* Cursor number for index */
  SrcItem *pItem                  /* The FROM clause entry for the table */
){
  প্রতিজ্ঞা( pItem==0 || (pItem->fg.jointype & JT_ডান)==0 );
  প্রতিজ্ঞা( (pItem==0 || pMask==0) && (pMask!=0 || pItem!=0) );

  যদি( pPart->op==TK_AND ){
    where_Part_Idx_রাশিমালা(pParse, pIdx, pPart->p_ডান, pMask, iIdxCur, pItem);
    pPart = pPart->p_বাম;
  }

  যদি( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    রাশিমালা *p_বাম = pPart->p_বাম;
    রাশিমালা *p_ডান = pPart->p_ডান;
    Table *p_বাম_Tab;            /* Table of the left operand */
    Column *p_বাম_Col;           /* Column of the left operand */
    u8 aff_ডান, aff_বাম;

    যদি( p_বাম->op!=TK_COLUMN ) ফেরত;
    যদি( !sqlite3_রাশিমালা_Is_Constant(0, p_ডান) ) ফেরত;
    যদি( !sqlite3_Is_দ্বিমিক(sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pPart)) ) ফেরত;
    যদি( p_বাম->iColumn<0 ) ফেরত;
    p_বাম_Tab = pIdx->pTable;
    p_বাম_Col = &p_বাম_Tab->aCol[p_বাম->iColumn];
    aff_বাম = p_বাম_Col->affinity;
    প্রতিজ্ঞা( SQLITE_AFF_BLOB<SQLITE_AFF_TEXT );
    যদি( aff_বাম<SQLITE_AFF_TEXT ) ফেরত;
    aff_ডান = sqlite3_রাশিমালা_Affinity(p_ডান);
    যদি( aff_ডান!=0 && aff_ডান!=aff_বাম ) ফেরত;
    যদি( pItem ){
      sqlite3 *db = pParse->db;
      Indexed_রাশিমালা *p = (Indexed_রাশিমালা*)sqlite3DbMallocRaw(db, আকার(*p));
      যদি( p ){
        পূর্ণসংখ্যা bNullRow = (pItem->fg.jointype&(JT_বাম|JT_LTORJ))!=0;
        p_ডান = sqlite3_রাশিমালা_Dup(db, p_ডান, 0);
        যদি( (p_বাম_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3ColumnType(p_বাম_Col, 0);
          প্রতিজ্ঞা( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (পূর্ণসংখ্যা)strlen(t.z);
          p->p_রাশিমালা = sqlite3_রাশিমালা_Alloc(db, TK_CAST, &t, 1);
          sqlite3_রাশিমালা_Attach_Subtrees(db, p->p_রাশিমালা, p_ডান, 0);
        }অন্যথা{
          p->p_রাশিমালা = p_ডান;
        }
        p->i_তথ্য_Cur = pItem->iCursor;
        p->iIdxCur = iIdxCur;
        p->iIdxCol = p_বাম->iColumn;
        p->bMaybeNullRow = bNullRow;
        p->pIENext = pParse->p_Idx_Part_রাশিমালা;
        p->aff = aff_বাম;
        pParse->p_Idx_Part_রাশিমালা = p;
        যদি( p->pIENext==0 ){
          শূন্য *pArg = (শূন্য*)&pParse->p_Idx_Part_রাশিমালা;
          sqlite3ParserAddCleanup(pParse, where_Indexed_রাশিমালা_Cleanup, pArg);
        }
      }
    }অন্যথা যদি( p_বাম->iColumn<(BMS-1) ){
      *pMask &= ~((Bitmask)1 << p_বাম->iColumn);
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
স্থির পূর্ণসংখ্যা whereLoopAddBtree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  সূচক *pProbe;              /* An index we are evaluating */
  সূচক sPk;                  /* A fake index object for the primary key */
  LogEst aiRowEstPk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 aiColumnPk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  SrcItem *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  পূর্ণসংখ্যা rc = SQLITE_OK;         /* Return code */
  পূর্ণসংখ্যা iSortIdx;               /* Index number */
  পূর্ণসংখ্যা b;                      /* A boolean value */
  LogEst r_আকার;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  Table *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  প্রতিজ্ঞা( !IsVirtual(pSrc->pSTab) );

  যদি( pSrc->fg.isIndexedBy ){
    প্রতিজ্ঞা( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }অন্যথা যদি( !HasRowid(pTab) ){
    যদি( pSrc->fg.notIndexed ){
      pProbe = sqlite3_Primary_চাবি_সূচক(pTab);
    }অন্যথা{
      pProbe = pTab->p_সূচক;
    }
  }অন্যথা{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    সূচক *pFirst;                  /* First of real indices on the table */
    memset(&sPk, 0, আকার(সূচক));
    sPk.n_চাবি_Col = 1;
    sPk.nColumn = 1;
    sPk.aiColumn = &aiColumnPk;
    sPk.aiRowLogEst = aiRowEstPk;
    sPk.onError = OE_Replace;
    sPk.pTable = pTab;
    sPk.szIdxRow = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    aiRowEstPk[0] = pTab->nRowLogEst;
    aiRowEstPk[1] = 0;
    pFirst = pSrc->pSTab->p_সূচক;
    sPk.p_পরবর্তী = pFirst;
    pProbe = &sPk;
  }
  r_আকার = pTab->nRowLogEst;

#ifndef SQLITE_OMIT_AUTOMATIC_সূচক
  /* Automatic indexes */
  যদি( !pBuilder->pOrSet      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_ডান_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_সূচক)!=0
   && !pSrc->fg.isIndexedBy  /* Has no INDEXED BY clause */
   && !pSrc->fg.notIndexed   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_ডান)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_আকার;         /* Logarithm of the number of rows in the table */
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    r_Log_আকার = estLog(r_আকার);
    জন্য(pTerm=pWC->a; rc==SQLITE_OK && pTerm<pWCEnd; pTerm++){
      যদি( pTerm->prereq_ডান & pNew->maskSelf ) চালু;
      যদি( term_Can_Drive_সূচক(pTerm, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_সূচক = 0;
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
        pNew->rSetup = r_Log_আকার + r_আকার;
        যদি( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->rSetup += 28;
        }অন্যথা{
          pNew->rSetup -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        ApplyCostMultiplier(pNew->rSetup, pTab->costMult);
        যদি( pNew->rSetup<0 ) pNew->rSetup = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->nOut = 43;  প্রতিজ্ঞা( 43==sqlite3LogEst(20) );
        pNew->rRun = sqlite3LogEstAdd(r_Log_আকার,pNew->nOut);
        pNew->wsFlags = WHERE_AUTO_সূচক;
        pNew->prereq = mPrereq | pTerm->prereq_ডান;
        rc = where_Loop_ঢোকাও(pBuilder, pNew);
      }
    }
  }
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  জন্য(iSortIdx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.isIndexedBy||pSrc->fg.notIndexed)?0:pProbe->p_পরবর্তী,
         iSortIdx++
  ){
    যদি( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_সূচক(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      চালু;  /* Partial index inappropriate for this query */
    }
    যদি( pProbe->bNoQuery ) চালু;
    r_আকার = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_শীর্ষ = 0;
    pNew->u.btree.nDistinctCol = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->iSortIdx = 0;
    pNew->rSetup = 0;
    pNew->prereq = mPrereq;
    pNew->nOut = r_আকার;
    pNew->u.btree.p_সূচক = pProbe;
    pNew->u.btree.pOrderBy = 0;
    b = সূচক_Might_Help_With_Order_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    প্রতিজ্ঞা( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    যদি( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
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
      pNew->rRun = r_আকার + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#অন্যথা
      pNew->rRun = r_আকার + 16;
#endif
      ApplyCostMultiplier(pNew->rRun, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, r_আকার);
      যদি( pSrc->fg.isSubquery ){
        যদি( pSrc->fg.viaCoroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        যদি( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.pOrderBy = pSrc->u4.pSubq->pSelect->pOrderBy;
        }
      }অন্যথা যদি( pSrc->fg.fromExists ){
        pNew->nOut = 0;
      }
      rc = where_Loop_ঢোকাও(pBuilder, pNew);
      pNew->nOut = r_আকার;
      যদি( rc ) বিরতি;
    }অন্যথা{
      Bitmask m;
      যদি( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
      }অন্যথা{
        m = pSrc->colUsed & pProbe->colNotIdxed;
        যদি( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_রাশিমালা(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_INDEXED;
        যদি( m==TOPBIT || (pProbe->b_Has_রাশিমালা && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_সূচক(pWInfo, pProbe, pSrc->iCursor);
          যদি( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            প্রতিজ্ঞা( m!=0 );
          }অন্যথা{
            m = 0;
            pNew->wsFlags |= isCov;
            যদি( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }অন্যথা{
              প্রতিজ্ঞা( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }অন্যথা যদি( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
        }
      }

      /* Full scan via index */
      যদি( b
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
        pNew->rRun = r_আকার + 1 + (15*pProbe->szIdxRow)/pTab->szTabRow;
        যদি( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_আকার + 16;  /* Base cost:  N*3 */
          পূর্ণসংখ্যা ii;
          পূর্ণসংখ্যা iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          জন্য(ii=0; ii<pWC2->nTerm; ii++){
            WhereTerm *pTerm = &pWC2->a[ii];
            যদি( !sqlite3_রাশিমালা_Covered_By_সূচক(pTerm->p_রাশিমালা, iCur, pProbe) ){
              বিরতি;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            যদি( pTerm->truthProb<=0 ){
              nLookup += pTerm->truthProb;
            }অন্যথা{
              nLookup--;
              যদি( pTerm->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, nLookup);
        }
        ApplyCostMultiplier(pNew->rRun, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, r_আকার);
        যদি( (pSrc->fg.jointype & JT_ডান)!=0 && pProbe->a_Col_রাশিমালা ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }অন্যথা{
          যদি( pSrc->fg.fromExists ) pNew->nOut = 0;
          rc = where_Loop_ঢোকাও(pBuilder, pNew);
        }
        pNew->nOut = r_আকার;
        যদি( rc ) বিরতি;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_Add_Btree_সূচক(pBuilder, pSrc, pProbe, 0);
    যদি( pBuilder->bldFlags1==SQLITE_BLDF1_INDEXED ){
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
  ফেরত rc;
}

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
স্থির পূর্ণসংখ্যা isLimitTerm(WhereTerm *pTerm){
  প্রতিজ্ঞা( pTerm->eOperator==WO_AUX || pTerm->eMatchOp==0 );
  ফেরত pTerm->eMatchOp>=SQLITE_সূচক_CONSTRAINT_LIMIT
      && pTerm->eMatchOp<=SQLITE_সূচক_CONSTRAINT_OFFSET;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
স্থির পূর্ণসংখ্যা allConstraintsUsed(
  কাঠামো sqlite3_সূচক_constraint_usage *aUsage, 
  পূর্ণসংখ্যা nCons
){
  পূর্ণসংখ্যা ii;
  জন্য(ii=0; ii<nCons; ii++){
    যদি( aUsage[ii].argv_সূচক<=0 ) ফেরত 0;
  }
  ফেরত 1;
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
স্থির পূর্ণসংখ্যা whereLoopAddVirtualOne(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_সূচক_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  পূর্ণসংখ্যা *pbIn,                      /* OUT: True if plan uses an IN(...) op */
  পূর্ণসংখ্যা *pbRetryLimit               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_সূচক_Info *pHidden = (Hidden_সূচক_Info*)&pIdxInfo[1];
  কাঠামো sqlite3_সূচক_constraint *pIdxCons;
  কাঠামো sqlite3_সূচক_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা mxTerm;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  পূর্ণসংখ্যা nConstraint = pIdxInfo->nConstraint;

  প্রতিজ্ঞা( (mUsable & mPrereq)==mPrereq );
  *pbIn = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(কাঠামো sqlite3_সূচক_constraint**)&pIdxInfo->aConstraint;
  জন্য(i=0; i<nConstraint; i++, pIdxCons++){
    WhereTerm *pTerm = termFromWhereClause(pWC, pIdxCons->iTermOffset);
    pIdxCons->usable = 0;
    যদি( (pTerm->prereq_ডান & mUsable)==pTerm->prereq_ডান
     && (pTerm->eOperator & mExclude)==0
     && (pbRetryLimit || !isLimitTerm(pTerm))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, আকার(pUsage[0])*nConstraint);
  প্রতিজ্ঞা( pIdxInfo->needToFreeIdxStr==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_অঙ্ক = 0;
  pIdxInfo->orderByConsumed = 0;
  pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (দ্বিগুণ)2;
  pIdxInfo->estimatedRows = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->mHandleIn = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_Best_সূচক(pParse, pSrc->pSTab, pIdxInfo);
  যদি( rc ){
    যদি( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      ফেরত SQLITE_OK;
    }
    ফেরত rc;
  }

  mxTerm = -1;
  প্রতিজ্ঞা( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, আকার(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, আকার(pNew->u.vtab));
  pIdxCons = *(কাঠামো sqlite3_সূচক_constraint**)&pIdxInfo->aConstraint;
  জন্য(i=0; i<nConstraint; i++, pIdxCons++){
    পূর্ণসংখ্যা iTerm;
    যদি( (iTerm = pUsage[i].argv_সূচক - 1)>=0 ){
      WhereTerm *pTerm;
      পূর্ণসংখ্যা j = pIdxCons->iTermOffset;
      যদি( iTerm>=nConstraint
       || j<0
       || (pTerm = termFromWhereClause(pWC, j))==0
       || pNew->aLTerm[iTerm]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_বার্তা(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        ফেরত SQLITE_ERROR;
      }
      testcase( iTerm==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->nTerm-1 );
      pNew->prereq |= pTerm->prereq_ডান;
      প্রতিজ্ঞা( iTerm<pNew->nLSlot );
      pNew->aLTerm[iTerm] = pTerm;
      যদি( iTerm>mxTerm ) mxTerm = iTerm;
      testcase( iTerm==15 );
      testcase( iTerm==16 );
      যদি( pUsage[i].omit ){
        যদি( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=iTerm );
          pNew->u.vtab.omitMask |= 1<<iTerm;
        }অন্যথা{
          testcase( i!=iTerm );
        }
        যদি( pTerm->eMatchOp==SQLITE_সূচক_CONSTRAINT_OFFSET ){
          pNew->u.vtab.bOmitOffset = 1;
        }
      }
      যদি( SMASKBIT32(i) & pHidden->mHandleIn ){
        pNew->u.vtab.mHandleIn |= MASKBIT32(iTerm);
      }অন্যথা যদি( (pTerm->eOperator & WO_IN)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->orderByConsumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_সূচক_SCAN_UNIQUE;
        *pbIn = 1; প্রতিজ্ঞা( (mExclude & WO_IN)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      প্রতিজ্ঞা( pbRetryLimit || !isLimitTerm(pTerm) );
      প্রতিজ্ঞা( !isLimitTerm(pTerm) || i>=nConstraint-2 );
      প্রতিজ্ঞা( !isLimitTerm(pTerm) || i==nConstraint-1 || isLimitTerm(pTerm+1) );

      যদি( isLimitTerm(pTerm) && (*pbIn || !allConstraintsUsed(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pbRetryLimit = 1;
        ফেরত SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mxTerm+1;
  জন্য(i=0; i<=mxTerm; i++){
    যদি( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_বার্তা(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      ফেরত SQLITE_ERROR;
    }
  }
  প্রতিজ্ঞা( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_অঙ্ক = pIdxInfo->idx_অঙ্ক;
  pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
  pIdxInfo->needToFreeIdxStr = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ?
      pIdxInfo->nOrderBy : 0);
  pNew->u.vtab.b_Idx_অঙ্ক_Hex = (pIdxInfo->idxFlags&SQLITE_সূচক_SCAN_HEX)!=0;
  pNew->rSetup = 0;
  pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  যদি( pIdxInfo->idxFlags & SQLITE_সূচক_SCAN_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }অন্যথা{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_ঢোকাও(pBuilder, pNew);
  যদি( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pbIn, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  ফেরত rc;
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
ধ্রুবক অক্ষর *sqlite3_vtab_collation(sqlite3_সূচক_info *pIdxInfo, পূর্ণসংখ্যা iCons){
  Hidden_সূচক_Info *pHidden = (Hidden_সূচক_Info*)&pIdxInfo[1];
  ধ্রুবক অক্ষর *zRet = 0;
  যদি( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    পূর্ণসংখ্যা iTerm = pIdxInfo->aConstraint[iCons].iTermOffset;
    রাশিমালা *pX = termFromWhereClause(pHidden->pWC, iTerm)->p_রাশিমালা;
    যদি( pX->p_বাম ){
      pC = sqlite3_রাশিমালা_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_দ্বিমিক);
  }
  ফেরত zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
পূর্ণসংখ্যা sqlite3_vtab_in(sqlite3_সূচক_info *pIdxInfo, পূর্ণসংখ্যা iCons, পূর্ণসংখ্যা bHandle){
  Hidden_সূচক_Info *pHidden = (Hidden_সূচক_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  যদি( m & pHidden->mIn ){
    যদি( bHandle==0 ){
      pHidden->mHandleIn &= ~m;
    }অন্যথা যদি( bHandle>0 ){
      pHidden->mHandleIn |= m;
    }
    ফেরত 1;
  }
  ফেরত 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
পূর্ণসংখ্যা sqlite3_vtab_rhs_মান(
  sqlite3_সূচক_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  পূর্ণসংখ্যা iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_মান **ppVal           /* Write value extracted here */
){
  Hidden_সূচক_Info *pH = (Hidden_সূচক_Info*)&pIdxInfo[1];
  sqlite3_মান *pVal = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  যদি( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }অন্যথা{
    যদি( pH->aRhs[iCons]==0 ){
      WhereTerm *pTerm = termFromWhereClause(
          pH->pWC, pIdxInfo->aConstraint[iCons].iTermOffset
      );
      rc = sqlite3_মান_From_রাশিমালা(
          pH->pParse->db, pTerm->p_রাশিমালা->p_ডান, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  যদি( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  ফেরত rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
পূর্ণসংখ্যা sqlite3_vtab_distinct(sqlite3_সূচক_info *pIdxInfo){
  Hidden_সূচক_Info *pHidden = (Hidden_সূচক_Info*)&pIdxInfo[1];
  প্রতিজ্ঞা( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  ফেরত pHidden->eDistinct;
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
শূন্য sqlite3VtabUsesAllSchemas(Parse *pParse){
  পূর্ণসংখ্যা nDb = pParse->db->nDb;
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<nDb; i++){
    sqlite3CodeVerifySchema(pParse, i);
  }
  যদি( DbMaskNonZero(pParse->writeMask) ){
    জন্য(i=0; i<nDb; i++){
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
স্থির পূর্ণসংখ্যা whereLoopAddVirtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  SrcItem *pSrc;               /* The FROM clause term to search */
  sqlite3_সূচক_info *p;       /* Object to pass to xBestIndex() */
  পূর্ণসংখ্যা nConstraint;             /* Number of constraints in p */
  পূর্ণসংখ্যা bIn;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask mBest;               /* Tables used by best possible plan */
  u16 mNoOmit;
  পূর্ণসংখ্যা bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  প্রতিজ্ঞা( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  প্রতিজ্ঞা( IsVirtual(pSrc->pSTab) );
  p = allocate_সূচক_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  যদি( p==0 ) ফেরত SQLITE_NOMEM_BKPT;
  pNew->rSetup = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  যদি( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_সূচক_Info(pParse->db, p);
    ফেরত SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = whereLoopAddVirtualOne(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, &bRetry
  );
  যদি( bRetry ){
    প্রতিজ্ঞা( rc==SQLITE_OK );
    rc = whereLoopAddVirtualOne(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &bIn, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  যদি( rc==SQLITE_OK && ((mBest = (pNew->prereq & ~mPrereq))!=0 || bIn) ){
    পূর্ণসংখ্যা seenZero = 0;             /* True if a plan with no prereqs seen */
    পূর্ণসংখ্যা seenZeroNoIN = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask m_পূর্ব = 0;
    Bitmask mBestNoIn = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    যদি( bIn ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, ALLBITS, WO_IN, p, mNoOmit, &bIn, 0);
      প্রতিজ্ঞা( bIn==0 );
      mBestNoIn = pNew->prereq & ~mPrereq;
      যদি( mBestNoIn==0 ){
        seenZero = 1;
        seenZeroNoIN = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    যখন( rc==SQLITE_OK ){
      পূর্ণসংখ্যা i;
      Bitmask m_পরবর্তী = ALLBITS;
      প্রতিজ্ঞা( m_পরবর্তী>0 );
      জন্য(i=0; i<nConstraint; i++){
        পূর্ণসংখ্যা iTerm = p->aConstraint[i].iTermOffset;
        Bitmask mThis = termFromWhereClause(pWC, iTerm)->prereq_ডান & ~mPrereq;
        যদি( mThis>m_পূর্ব && mThis<m_পরবর্তী ) m_পরবর্তী = mThis;
      }
      m_পূর্ব = m_পরবর্তী;
      যদি( m_পরবর্তী==ALLBITS ) বিরতি;
      যদি( m_পরবর্তী==mBest || m_পরবর্তী==mBestNoIn ) চালু;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)m_পূর্ব, (sqlite3_uint64)m_পরবর্তী));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, m_পরবর্তী|mPrereq, 0, p, mNoOmit, &bIn, 0);
      যদি( pNew->prereq==mPrereq ){
        seenZero = 1;
        যদি( bIn==0 ) seenZeroNoIN = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    যদি( rc==SQLITE_OK && seenZero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &bIn, 0);
      যদি( bIn==0 ) seenZeroNoIN = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    যদি( rc==SQLITE_OK && seenZeroNoIN==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = whereLoopAddVirtualOne(
          pBuilder, mPrereq, mPrereq, WO_IN, p, mNoOmit, &bIn, 0);
    }
  }

  free_সূচক_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  ফেরত rc;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
স্থির পূর্ণসংখ্যা whereLoopAddOr(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  WhereTerm *pTerm, *pWCEnd;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা iCur;
  WhereClause অস্থায়ী_WC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet s_যোগ, sCur;
  SrcItem *pItem;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  memset(&s_যোগ, 0, আকার(s_যোগ));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  যদি( pItem->fg.jointype & JT_ডান ) ফেরত SQLITE_OK;

  জন্য(pTerm=pWC->a; pTerm<pWCEnd && rc==SQLITE_OK; pTerm++){
    যদি( (pTerm->eOperator & WO_OR)!=0
     && (pTerm->u.pOrInfo->indexable & pNew->maskSelf)!=0
     && (!রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON)
          || pTerm->p_রাশিমালা->w.iJoin==iCur)
    ){
      WhereClause * ধ্রুবক pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm * ধ্রুবক pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      পূর্ণসংখ্যা once = 1;
      পূর্ণসংখ্যা i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.pOrSet = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", pTerm));
      জন্য(pOrTerm=pOrWC->a; pOrTerm<pOrWCEnd; pOrTerm++){
        যদি( (pOrTerm->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        }অন্যথা যদি( pOrTerm->বাম_Cursor==iCur ){
          অস্থায়ী_WC.pWInfo = pWC->pWInfo;
          অস্থায়ী_WC.pOuter = pWC;
          অস্থায়ী_WC.op = TK_AND;
          অস্থায়ী_WC.nTerm = 1;
          অস্থায়ী_WC.nBase = 1;
          অস্থায়ী_WC.a = pOrTerm;
          sSubBuild.pWC = &অস্থায়ী_WC;
        }অন্যথা{
          চালু;
        }
        sCur.n = 0;
#ifdef WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (পূর্ণসংখ্যা)(pOrTerm-pOrWC->a), pTerm, sSubBuild.pWC->nTerm));
        যদি( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
        যদি( IsVirtual(pItem->pSTab) ){
          rc = whereLoopAddVirtual(&sSubBuild, mPrereq, mUnusable);
        }অন্যথা
#endif
        {
          rc = whereLoopAddBtree(&sSubBuild, mPrereq);
        }
        যদি( rc==SQLITE_OK ){
          rc = whereLoopAddOr(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_DONE );
        যদি( sCur.n==0 ){
          s_যোগ.n = 0;
          বিরতি;
        }অন্যথা যদি( once ){
          whereOrMove(&s_যোগ, &sCur);
          once = 0;
        }অন্যথা{
          WhereOrSet s_পূর্ব;
          whereOrMove(&s_পূর্ব, &s_যোগ);
          s_যোগ.n = 0;
          জন্য(i=0; i<s_পূর্ব.n; i++){
            জন্য(j=0; j<sCur.n; j++){
              where_Or_ঢোকাও(&s_যোগ, s_পূর্ব.a[i].prereq | sCur.a[j].prereq,
                            sqlite3LogEstAdd(s_পূর্ব.a[i].rRun, sCur.a[j].rRun),
                            sqlite3LogEstAdd(s_পূর্ব.a[i].nOut, sCur.a[j].nOut));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = pTerm;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->rSetup = 0;
      pNew->iSortIdx = 0;
      memset(&pNew->u, 0, আকার(pNew->u));
      জন্য(i=0; rc==SQLITE_OK && i<s_যোগ.n; i++){
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
        pNew->rRun = s_যোগ.a[i].rRun + 1;
        pNew->nOut = s_যোগ.a[i].nOut;
        pNew->prereq = s_যোগ.a[i].prereq;
        rc = where_Loop_ঢোকাও(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", pTerm));
    }
  }
  ফেরত rc;
}

/*
** Add all WhereLoop objects for all tables
*/
স্থির পূর্ণসংখ্যা whereLoopAddAll(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask mPrior = 0;
  পূর্ণসংখ্যা iTab;
  SrcList *pTabList = pWInfo->pTabList;
  SrcItem *pItem;
  SrcItem *pEnd = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা bFirstPastRJ = 0;
  পূর্ণসংখ্যা has_ডান_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  প্রতিজ্ঞা( pNew->nLTerm==0 );
  প্রতিজ্ঞা( pNew->wsFlags==0 );
  প্রতিজ্ঞা( pNew->nLSlot>=Array_আকার(pNew->aLTermSpace) );
  প্রতিজ্ঞা( pNew->aLTerm!=0 );

  pBuilder->iPlanLimit = SQLITE_QUERY_PLANNER_LIMIT;
  জন্য(iTab=0, pItem=pTabList->a; pItem<pEnd; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->iPlanLimit += SQLITE_QUERY_PLANNER_LIMIT_INCR;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    যদি( bFirstPastRJ
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
      যদি( pItem->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( pItem->fg.jointype & JT_LTORJ );
        testcase( pItem->fg.jointype & JT_CROSS );
        has_ডান_Cross_Join = 1;
      }
      mPrereq |= mPrior;
      bFirstPastRJ = (pItem->fg.jointype & JT_ডান)!=0;
    }অন্যথা যদি( pItem->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      WhereTerm *pTerm;
      পূর্ণসংখ্যা i;
      জন্য(i=pWC->nBase, pTerm=pWC->a; i>0; i--, pTerm++){
        যদি( (pNew->maskSelf & pTerm->prereqAll)!=0 ){
          mPrereq |= (pTerm->prereqAll & (pNew->maskSelf-1));
        }
      }
    }অন্যথা যদি( !has_ডান_Cross_Join ){
      mPrereq = 0;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    যদি( IsVirtual(pItem->pSTab) ){
      SrcItem *p;
      জন্য(p=&pItem[1]; p<pEnd; p++){
        যদি( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mPrereq, mUnusable);
    }অন্যথা
#endif /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = whereLoopAddBtree(pBuilder, mPrereq);
    }
    যদি( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = whereLoopAddOr(pBuilder, mPrereq, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    যদি( rc || db->mallocFailed ){
      যদি( rc==SQLITE_DONE ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }অন্যথা{
        বিরতি;
      }
    }
  }

  whereLoopClear(db, pNew);
  ফেরত rc;
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
স্থির SQLITE_NOINLINE পূর্ণসংখ্যা wherePathMatchSubqueryOB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  পূর্ণসংখ্যা iLoop,              /* Which level of the nested loop.  0==outermost */
  পূর্ণসংখ্যা iCur,               /* Cursor used by the this loop */
  রাশিমালা_List *pOrderBy,     /* The ORDER BY clause on the whole query */
  Bitmask *pRevMask,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  পূর্ণসংখ্যা iOB;                /* Index into pOrderBy->a[] */
  পূর্ণসংখ্যা jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  রাশিমালা *pOBExpr;          /* Current term of outer ORDER BY */
  রাশিমালা_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.pOrderBy;
  প্রতিজ্ঞা( pSubOB!=0 );
  জন্য(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  জন্য(jSub=0; jSub<pSubOB->n_রাশিমালা && iOB<pOrderBy->n_রাশিমালা; jSub++, iOB++){
    যদি( pSubOB->a[jSub].u.x.iOrderByCol==0 ) বিরতি;
    pOBExpr = pOrderBy->a[iOB].p_রাশিমালা;
    যদি( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) বিরতি;
    যদি( pOBExpr->iTable!=iCur ) বিরতি;
    যদি( pOBExpr->iColumn!=pSubOB->a[jSub].u.x.iOrderByCol-1 ) বিরতি;
    যদি( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = pOrderBy->a[iOB].fg.sortFlags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.sortFlags;   /* sortFlags for jSub */
      যদি( (sfSub & KEYINFO_ORDER_BIGNULL) != (sfOB & KEYINFO_ORDER_BIGNULL) ){
        বিরতি;
      }
      revIdx = sfSub & KEYINFO_ORDER_DESC;
      যদি( jSub>0 ){
        যদি( (rev^revIdx)!=(sfOB & KEYINFO_ORDER_DESC) ){
          বিরতি;
        }
      }অন্যথা{
        rev = revIdx ^ (sfOB & KEYINFO_ORDER_DESC);
        যদি( rev ){
          যদি( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            বিরতি;
          }
          *pRevMask |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  ফেরত jSub>0;
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
স্থির i8 wherePathSatisfiesOrderBy(
  WhereInfo *pWInfo,    /* The WHERE clause */
  রাশিমালা_List *pOrderBy,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
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
  u16 n_চাবি_Col;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 nOrderBy;         /* Number terms in the ORDER BY clause */
  পূর্ণসংখ্যা iLoop;            /* Index of WhereLoop in pPath being processed */
  পূর্ণসংখ্যা i, j;             /* Loop counters */
  পূর্ণসংখ্যা iCur;             /* Cursor number for current WhereLoop */
  পূর্ণসংখ্যা iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  WhereTerm *pTerm;     /* A single term of the WHERE clause */
  রাশিমালা *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  সূচক *p_সূচক;        /* The index associated with pLoop */
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

  প্রতিজ্ঞা( pOrderBy!=0 );
  যদি( nLoop && OptimizationDisabled(db, SQLITE_OrderByIdxJoin) ) ফেরত 0;

  nOrderBy = pOrderBy->n_রাশিমালা;
  testcase( nOrderBy==BMS-1 );
  যদি( nOrderBy>BMS-1 ) ফেরত 0;  /* Cannot optimize overly large ORDER BYs */
  isOrderDistinct = 1;
  obDone = MASKBIT(nOrderBy)-1;
  orderDistinctMask = 0;
  ready = 0;
  eqOpMask = WO_EQ | WO_IS | WO_ISNULL;
  যদি( wctrlFlags & (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eqOpMask |= WO_IN;
  }
  জন্য(iLoop=0; isOrderDistinct && obSat<obDone && iLoop<=nLoop; iLoop++){
    যদি( iLoop>0 ) ready |= pLoop->maskSelf;
    যদি( iLoop<nLoop ){
      pLoop = pPath->aLoop[iLoop];
      যদি( wctrlFlags & WHERE_ORDERBY_LIMIT ) চালু;
    }অন্যথা{
      pLoop = pLast;
    }
    যদি( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      যদি( pLoop->u.vtab.isOrdered && pWInfo->pOrderBy==pOrderBy ){
        obSat = obDone;
      }অন্যথা{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        isOrderDistinct = 0;
      }
      বিরতি;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    জন্য(i=0; i<nOrderBy; i++){
      যদি( MASKBIT(i) & obSat ) চালু;
      pOBExpr = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pOrderBy->a[i].p_রাশিমালা);
      যদি( NEVER(pOBExpr==0) ) চালু;
      যদি( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) চালু;
      যদি( pOBExpr->iTable!=iCur ) চালু;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, eqOpMask, 0);
      যদি( pTerm==0 ) চালু;
      যদি( pTerm->eOperator==WO_IN ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        প্রতিজ্ঞা( wctrlFlags &
               (WHERE_ORDERBY_LIMIT|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        জন্য(j=0; j<pLoop->nLTerm && pTerm!=pLoop->aLTerm[j]; j++){}
        যদি( j>=pLoop->nLTerm ) চালু;
      }
      যদি( (pTerm->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_রাশিমালা_NNColl_Seq(pParse, pOrderBy->a[i].p_রাশিমালা);
        ধ্রুবক রাশিমালা *pCExpr = pTerm->p_রাশিমালা;
        CollSeq *pColl2 = sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pCExpr);
        অক্ষর aff_ডান;
        প্রতিজ্ঞা( pColl1 );
        যদি( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          চালু;
        }
        aff_ডান = sqlite3_রাশিমালা_Affinity(pCExpr->p_ডান);
        /* Left operand must be a column, hence always has affinity */
        প্রতিজ্ঞা( sqlite3_রাশিমালা_Affinity(pCExpr->p_বাম)!=0 );
        যদি( aff_ডান!=0 && aff_ডান!=sqlite3_রাশিমালা_Affinity(pCExpr->p_বাম) ){
          চালু;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    যদি( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      যদি( pLoop->wsFlags & WHERE_IPK ){
        যদি( pLoop->u.btree.pOrderBy
         && OptimizationEnabled(db, SQLITE_OrderBySubq)
         &&  wherePathMatchSubqueryOB(pLoop,iLoop,iCur,
                                     pOrderBy,pRevMask, &obSat, wctrlFlags)
        ){
          nColumn = 0;
          isOrderDistinct = 0;
        }অন্যথা{
          nColumn = 1;
        }
        p_সূচক = 0;
        n_চাবি_Col = 1;
      }অন্যথা যদি( (p_সূচক = pLoop->u.btree.p_সূচক)==0 || p_সূচক->bUnordered ){
        ফেরত 0;
      }অন্যথা{
        n_চাবি_Col = p_সূচক->n_চাবি_Col;
        nColumn = p_সূচক->nColumn;
        প্রতিজ্ঞা( nColumn==n_চাবি_Col+1 || !HasRowid(p_সূচক->pTable) );
        প্রতিজ্ঞা( p_সূচক->aiColumn[nColumn-1]==XN_ROWID
                          || !HasRowid(p_সূচক->pTable));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        isOrderDistinct = Is_Unique_সূচক(p_সূচক)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = revSet = 0;
      distinctColumns = 0;
      জন্য(j=0; j<nColumn; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        প্রতিজ্ঞা( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        যদি( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          যদি( (eOp & eqOpMask)!=0 ){
            যদি( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( isOrderDistinct );
              isOrderDistinct = 0;
            }
            চালু; 
          }অন্যথা যদি( ALWAYS(eOp & WO_IN) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            রাশিমালা *pX = pLoop->aLTerm[j]->p_রাশিমালা;
            জন্য(i=j+1; i<pLoop->u.btree.nEq; i++){
              যদি( pLoop->aLTerm[i]->p_রাশিমালা==pX ){
                প্রতিজ্ঞা( (pLoop->aLTerm[i]->eOperator & WO_IN) );
                bOnce = 0;
                বিরতি;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        যদি( p_সূচক ){
          iColumn = p_সূচক->aiColumn[j];
          revIdx = p_সূচক->aSortOrder[j] & KEYINFO_ORDER_DESC;
          যদি( iColumn==p_সূচক->pTable->iPKey ) iColumn = XN_ROWID;
        }অন্যথা{
          iColumn = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        যদি( isOrderDistinct ){
          যদি( iColumn>=0
           && j>=pLoop->u.btree.nEq
           && p_সূচক->pTable->aCol[iColumn].notNull==0
          ){
            isOrderDistinct = 0;
          }
          যদি( iColumn==XN_রাশিমালা ){
            isOrderDistinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        isMatch = 0;
        জন্য(i=0; bOnce && i<nOrderBy; i++){
          যদি( MASKBIT(i) & obSat ) চালু;
          pOBExpr = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pOrderBy->a[i].p_রাশিমালা);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          যদি( NEVER(pOBExpr==0) ) চালু;
          যদি( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          যদি( iColumn>=XN_ROWID ){
            যদি( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) চালু;
            যদি( pOBExpr->iTable!=iCur ) চালু;
            যদি( pOBExpr->iColumn!=iColumn ) চালু;
          }অন্যথা{
            রাশিমালা *p_Ix_রাশিমালা = p_সূচক->a_Col_রাশিমালা->a[j].p_রাশিমালা;
            যদি( sqlite3_রাশিমালা_Compare_Skip(pOBExpr, p_Ix_রাশিমালা, iCur) ){
              চালু;
            }
          }
          যদি( iColumn!=XN_ROWID ){
            pColl = sqlite3_রাশিমালা_NNColl_Seq(pWInfo->pParse, pOrderBy->a[i].p_রাশিমালা);
            যদি( sqlite3StrICmp(pColl->zName, p_সূচক->azColl[j])!=0 ) চালু;
          }
          যদি( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.nDistinctCol = j+1;
          }
          isMatch = 1;
          বিরতি;
        }
        যদি( isMatch && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          যদি( revSet ){
            যদি( (rev ^ revIdx)
                           != (pOrderBy->a[i].fg.sortFlags&KEYINFO_ORDER_DESC)
            ){
              isMatch = 0;
            }
          }অন্যথা{
            rev = revIdx ^ (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_DESC);
            যদি( rev ) *pRevMask |= MASKBIT(iLoop);
            revSet = 1;
          }
        }
        যদি( isMatch && (pOrderBy->a[i].fg.sortFlags & KEYINFO_ORDER_BIGNULL) ){
          যদি( j==pLoop->u.btree.nEq 
           && (j<n_চাবি_Col||HasRowid(p_সূচক->pTable)||!Is_Primary_চাবি_সূচক(p_সূচক))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_SORT;
          }অন্যথা{
            isMatch = 0;
          }
        }
        যদি( isMatch ){
          যদি( iColumn==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }অন্যথা{
          /* No match found */
          প্রতিজ্ঞা( n_চাবি_Col>0 );
          যদি( j<n_চাবি_Col ){
            testcase( isOrderDistinct!=0 );
            isOrderDistinct = 0;
          }
          বিরতি;
        }
      } /* end Loop over all index columns */
      যদি( distinctColumns ){
        testcase( isOrderDistinct==0 );
        isOrderDistinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    যদি( isOrderDistinct ){
      orderDistinctMask |= pLoop->maskSelf;
      জন্য(i=0; i<nOrderBy; i++){
        রাশিমালা *p;
        Bitmask mTerm;
        যদি( MASKBIT(i) & obSat ) চালু;
        p = pOrderBy->a[i].p_রাশিমালা;
        mTerm = sqlite3_Where_রাশিমালা_Usage(&pWInfo->sMaskSet,p);
        যদি( mTerm==0 && !sqlite3_রাশিমালা_Is_Constant(0,p) ) চালু;
        যদি( (mTerm&~orderDistinctMask)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  যদি( obSat==obDone ) ফেরত (i8)nOrderBy;
  যদি( !isOrderDistinct ){
    জন্য(i=nOrderBy-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      যদি( (obSat&m)==m ) ফেরত i;
    }
    ফেরত 0;
  }
  ফেরত -1;
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
পূর্ণসংখ্যা sqlite3WhereIsSorted(WhereInfo *pWInfo){
  প্রতিজ্ঞা( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  প্রতিজ্ঞা( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  ফেরত pWInfo->sorted;
}

#ifdef WHERETRACE_ENABLED
/* For debugging use only: */
স্থির ধ্রুবক অক্ষর *wherePathName(WherePath *pPath, পূর্ণসংখ্যা nLoop, WhereLoop *pLast){
  স্থির অক্ষর zName[65];
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<nLoop; i++){ zName[i] = pPath->aLoop[i]->cId; }
  যদি( pLast ) zName[i++] = pLast->cId;
  zName[i] = 0;
  ফেরত zName;
}
#endif

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
স্থির LogEst whereSortingCost(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst nRow,       /* Estimated number of rows to sort */
  পূর্ণসংখ্যা nOrderBy,      /* Number of ORDER BY clause terms */
  পূর্ণসংখ্যা nSorted        /* Number of initial ORDER BY terms naturally in order */
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
  প্রতিজ্ঞা( pWInfo->pSelect!=0 );
  প্রতিজ্ঞা( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->n_রাশিমালা+59)/30);
  rSortCost = nRow + nCol;
  যদি( nSorted>0 ){
    /* Scale the result by (Y/X) */
    rSortCost += sqlite3LogEst((nOrderBy-nSorted)*100/nOrderBy) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  যদি( (pWInfo->wctrlFlags & WHERE_USE_LIMIT)!=0 ){
    rSortCost += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    যদি( nSorted!=0 ){
      rSortCost += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    যদি( pWInfo->iLimit<nRow ){
      nRow = pWInfo->iLimit;
    }
  }অন্যথা যদি( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    যদি( nRow>10 ){ nRow -= 10;  প্রতিজ্ঞা( 10==sqlite3LogEst(2) ); }
  }
  rSortCost += estLog(nRow);
  ফেরত rSortCost;
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
স্থির পূর্ণসংখ্যা computeMxChoice(WhereInfo *pWInfo){
  পূর্ণসংখ্যা nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#ifdef SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  জন্য(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
    প্রতিজ্ঞা( pWLoop->maskSelf==MASKBIT(pWLoop->iTab) );
    প্রতিজ্ঞা( pWLoop->p_পরবর্তী_Loop==0 || pWLoop->iTab<=pWLoop->p_পরবর্তী_Loop->iTab );
  }
#endif /* SQLITE_DEBUG */

  যদি( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->bStarDone
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_StarQuery)
  ){
    SrcItem *aFromTabs;    /* All terms of the FROM clause */
    পূর্ণসংখ্যা iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *pStart;     /* Where to start searching for dimension-tables */

    pWInfo->bStarDone = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    প্রতিজ্ঞা( !pWInfo->bStarUsed );
    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    জন্য(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      পূর্ণসংখ্যা nDep = 0;             /* Number of dimension tables */
      LogEst mxRun;             /* Maximum SCAN cost of a fact table */
      Bitmask mSeen = 0;        /* Mask of dimension tables */
      SrcItem *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      যদি( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        যদি( iFromIdx+3 > nLoop ){
          বিরতি;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        যখন( pStart && pStart->iTab<=iFromIdx ){
          pStart = pStart->p_পরবর্তী_Loop;
        }
      }
      জন্য(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
        যদি( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          বিরতি; /* Constraint (bb) */
        }
        যদি( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->maskSelf & mSeen)==0  /* pWInfo not already a dependency */
         && (pWLoop->maskSelf & mSelfJoin)==0 /* Not a self-join */
        ){
          যদি( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }অন্যথা{
            nDep++;
            mSeen |= pWLoop->maskSelf;
          }
        }
      }
      যদি( nDep<=2 ){
        চালু; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#ifdef WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      যদি( !pWInfo->bStarUsed ){
        জন্য(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
          pWLoop->rStarDelta = 0;
        }
      }
#endif
#ifdef WHERETRACE_ENABLED /* 0x80000 */
      যদি( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = mSeen;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        জন্য(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
          যদি( mShow & pWLoop->maskSelf ){
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
      জন্য(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
        যদি( pWLoop->iTab<iFromIdx ) চালু;
        যদি( pWLoop->iTab>iFromIdx ) বিরতি;
        যদি( pWLoop->rRun>mxRun ) mxRun = pWLoop->rRun;
      }
      যদি( ALWAYS(mxRun<LOGEST_MAX) ) mxRun++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      জন্য(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
        যদি( (pWLoop->maskSelf & mSeen)==0 ) চালু;
        যদি( pWLoop->nLTerm ) চালু;
        যদি( pWLoop->rRun<mxRun ){
#ifdef WHERETRACE_ENABLED /* 0x80000 */
          যদি( sqlite3WhereTrace & 0x80000 ){
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
    যদি( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->bStarUsed ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      জন্য(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
        যদি( pWLoop->rStarDelta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#endif
  }
  ফেরত pWInfo->bStarUsed ? 18 : 12;
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
স্থির SQLITE_NOINLINE পূর্ণসংখ্যা whereLoopIsNoBetter(
  ধ্রুবক WhereLoop *pCandidate,
  ধ্রুবক WhereLoop *pBaseline
){
  যদি( (pCandidate->wsFlags & WHERE_INDEXED)==0 ) ফেরত 1;
  যদি( (pBaseline->wsFlags & WHERE_INDEXED)==0 ) ফেরত 1;
  যদি( pCandidate->u.btree.p_সূচক->szIdxRow <
        pBaseline->u.btree.p_সূচক->szIdxRow ) ফেরত 0;
  ফেরত 1;
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
স্থির পূর্ণসংখ্যা wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst){
  পূর্ণসংখ্যা mxChoice;             /* Maximum number of simultaneous paths tracked */
  পূর্ণসংখ্যা nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  পূর্ণসংখ্যা iLoop;                /* Loop counter over the terms of the join */
  পূর্ণসংখ্যা ii, jj;               /* Loop counters */
  পূর্ণসংখ্যা mxI = 0;              /* Index of next entry to replace */
  পূর্ণসংখ্যা nOrderBy;             /* Number of ORDER BY clause terms */
  LogEst mxCost = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  পূর্ণসংখ্যা nTo, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  WherePath *aFrom;         /* All nFrom paths at the previous level */
  WherePath *aTo;           /* The nTo best paths at the current level */
  WherePath *pFrom;         /* An element of aFrom[] that we are working on */
  WherePath *pTo;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *aSortCost = 0;    /* Sorting and partial sorting costs */
  অক্ষর *pSpace;             /* Temporary memory used by this routine */
  পূর্ণসংখ্যা nSpace;               /* Bytes of space allocated at pSpace */

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
  যদি( nLoop<=1 ){
    mxChoice = 1;
  }অন্যথা যদি( nLoop==2 ){
    mxChoice = 5;
  }অন্যথা যদি( pParse->nErr ){
    mxChoice = 1;
  }অন্যথা{
    mxChoice = computeMxChoice(pWInfo);
  }
  প্রতিজ্ঞা( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  যদি( pWInfo->pOrderBy==0 || nRowEst==0 ){
    nOrderBy = 0;
  }অন্যথা{
    nOrderBy = pWInfo->pOrderBy->n_রাশিমালা;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (আকার(WherePath)+আকার(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += আকার(LogEst) * nOrderBy;
  pSpace = sqlite3_স্তূপ_Alloc_Raw_NN(pParse->db, nSpace);
  যদি( pSpace==0 ) ফেরত SQLITE_NOMEM_BKPT;
  aTo = (WherePath*)pSpace;
  aFrom = aTo+mxChoice;
  memset(aFrom, 0, আকার(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  জন্য(ii=mxChoice*2, pFrom=aTo; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  যদি( nOrderBy ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    aSortCost = (LogEst*)pX;
    memset(aSortCost, 0, আকার(LogEst) * nOrderBy);
  }
  প্রতিজ্ঞা( aSortCost==0 || &pSpace[nSpace]==(অক্ষর*)&aSortCost[nOrderBy] );
  প্রতিজ্ঞা( aSortCost!=0 || &pSpace[nSpace]==(অক্ষর*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  প্রতিজ্ঞা( 48==sqlite3LogEst(28) );
  nFrom = 1;
  প্রতিজ্ঞা( aFrom[0].isOrdered==0 );
  যদি( nOrderBy ){
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
  জন্য(iLoop=0; iLoop<nLoop; iLoop++){
    nTo = 0;
    জন্য(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      জন্য(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_পরবর্তী_Loop){
        LogEst nOut;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask maskNew;                  /* Mask of src visited by (..) */
        Bitmask revMask;                  /* Mask of rev-order loops for (..) */

        যদি( (pWLoop->prereq & ~pFrom->maskLoop)!=0 ) চালু;
        যদি( (pWLoop->maskSelf & pFrom->maskLoop)!=0 ) চালু;
        যদি( (pWLoop->wsFlags & WHERE_AUTO_সূচক)!=0 && pFrom->nRow<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          প্রতিজ্ঞা( 10==sqlite3LogEst(2) );
          চালু;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->rRun + pFrom->nRow;
        যদি( pWLoop->rSetup ){
          rUnsort = sqlite3LogEstAdd(pWLoop->rSetup, rUnsort);
        }
        rUnsort = sqlite3LogEstAdd(rUnsort, pFrom->rUnsort);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        isOrdered = pFrom->isOrdered;
        যদি( isOrdered<0 ){
          revMask = 0;
          isOrdered = wherePathSatisfiesOrderBy(pWInfo,
                       pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &revMask);
        }অন্যথা{
          revMask = pFrom->revLoop;
        }
        যদি( isOrdered>=0 && isOrdered<nOrderBy ){
          যদি( aSortCost[isOrdered]==0 ){
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
        }অন্যথা{
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
        জন্য(jj=0, pTo=aTo; jj<nTo; jj++, pTo++){
          যদি( pTo->maskLoop==maskNew
           && ( ((pTo->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==nTo-1 );
            বিরতি;
          }
        }
        যদি( jj>=nTo ){
          /* None of the existing best-so-far paths match the candidate. */
          যদি( nTo>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
            যদি( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#endif
            চালু;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          যদি( nTo<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = nTo++;
          }অন্যথা{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          pTo = &aTo[jj];
#ifdef WHERETRACE_ENABLED /* 0x4 */
          যদি( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#endif
        }অন্যথা{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          যদি( (pTo->rCost<rCost)
           || (pTo->rCost==rCost && pTo->nRow<nOut)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort<rUnsort)
           || (pTo->rCost==rCost && pTo->nRow==nOut && pTo->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, pTo->aLoop[iLoop]) )
          ){
#ifdef WHERETRACE_ENABLED /* 0x4 */
            যদি( sqlite3WhereTrace&0x4 ){
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
            চালু;
          }
          testcase( pTo->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
          যদি( sqlite3WhereTrace&0x4 ){
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
        memcpy(pTo->aLoop, pFrom->aLoop, আকার(WhereLoop*)*iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        যদি( nTo>=mxChoice ){
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsort = aTo[0].nRow;
          জন্য(jj=1, pTo=&aTo[1]; jj<mxChoice; jj++, pTo++){
            যদি( pTo->rCost>mxCost
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
    যদি( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      পূর্ণসংখ্যা nDone = 0;
      পূর্ণসংখ্যা nProgress;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      করো{
        nProgress = 0;
        rMin = 0x7fff;
        জন্য(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          যদি( pTo->rCost>rFloor && pTo->rCost<rMin ) rMin = pTo->rCost;
        }
        জন্য(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
          যদি( pTo->rCost==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
               pTo->isOrdered>=0 ? (pTo->isOrdered+'0') : '?');
            যদি( pTo->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", pTo->revLoop);
            }অন্যথা{
              sqlite3DebugPrintf("\n");
            }
            nDone++;
            nProgress++;
          }
        }
        rFloor = rMin;
      }যখন( nDone<nTo && nProgress>0 );
    }
#endif

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  যদি( nFrom==0 ){
    sqlite3_Error_বার্তা(pParse, "no query solution");
    sqlite3_স্তূপ_Free_NN(pParse->db, pSpace);
    ফেরত SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  প্রতিজ্ঞা( nFrom==1 );
  pFrom = aFrom;

  প্রতিজ্ঞা( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  জন্য(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  যদি( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask notUsed;
    পূর্ণসংখ্যা rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->p_ফলাফল_Set, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &notUsed);
    যদি( rc==pWInfo->p_ফলাফল_Set->n_রাশিমালা ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  যদি( pWInfo->pOrderBy ){
    pWInfo->nOBSat = pFrom->isOrdered;
    যদি( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      যদি( pFrom->isOrdered==pWInfo->pOrderBy->n_রাশিমালা ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      প্রতিজ্ঞা( pWInfo->pSelect->pOrderBy==0
           || pWInfo->nOBSat <= pWInfo->pSelect->pOrderBy->n_রাশিমালা );
    }অন্যথা{
      pWInfo->revMask = pFrom->revLoop;
      যদি( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        যদি( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          যদি( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_COLUMN_IN))!=(WHERE_IPK|WHERE_COLUMN_IN)
          ){
            Bitmask m = 0;
            পূর্ণসংখ্যা rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom,
                      WHERE_ORDERBY_LIMIT, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_COLUMN_IN );
            যদি( rc==pWInfo->pOrderBy->n_রাশিমালা ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->revMask = m;
            }
          }
        }
      }অন্যথা যদি( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    যদি( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->pOrderBy->n_রাশিমালা && nLoop>0
    ){
      Bitmask revMask = 0;
      পূর্ণসংখ্যা nOrder = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &revMask
      );
      প্রতিজ্ঞা( pWInfo->sorted==0 );
      যদি( nOrder==pWInfo->pOrderBy->n_রাশিমালা ){
        pWInfo->sorted = 1;
        pWInfo->revMask = revMask;
      }
    }
  }

  pWInfo->nRowOut = pFrom->nRow;
#ifdef WHERETRACE_ENABLED
  pWInfo->r_যোগ_Cost = pFrom->rCost;
#endif

  /* Free temporary memory and return success */
  sqlite3_স্তূপ_Free_NN(pParse->db, pSpace);
  ফেরত SQLITE_OK;
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
স্থির SQLITE_NOINLINE শূন্য whereInterstageHeuristic(WhereInfo *pWInfo){
  পূর্ণসংখ্যা i;
#ifdef WHERETRACE_ENABLED
  পূর্ণসংখ্যা once = 0;
#endif
  জন্য(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    যদি( p==0 ) বিরতি;
    যদি( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      বিরতি;
    }
    যদি( (p->wsFlags & (WHERE_COLUMN_EQ|WHERE_COLUMN_NULL|WHERE_COLUMN_IN))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      জন্য(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_পরবর্তী_Loop){
        যদি( pLoop->iTab!=iTab ) চালু;
        যদি( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_সূচক))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          চালু;
        }
#ifdef WHERETRACE_ENABLED
        যদি( sqlite3WhereTrace & 0x80 ){
          যদি( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#endif /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }অন্যথা{
      বিরতি;
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
স্থির পূর্ণসংখ্যা whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  পূর্ণসংখ্যা iCur;
  পূর্ণসংখ্যা j;
  Table *pTab;
  সূচক *pIdx;
  WhereScan scan;

  pWInfo = pBuilder->pWInfo;
  যদি( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) ফেরত 0;
  প্রতিজ্ঞা( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  যদি( IsVirtual(pTab) ) ফেরত 0;
  যদি( pItem->fg.isIndexedBy || pItem->fg.notIndexed ){
    testcase( pItem->fg.isIndexedBy );
    testcase( pItem->fg.notIndexed );
    ফেরত 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = whereScanInit(&scan, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  যখন( pTerm && pTerm->prereq_ডান ) pTerm = where_Scan_পরবর্তী(&scan);
  যদি( pTerm ){
    testcase( pTerm->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->rRun = 33;  /* 33==sqlite3LogEst(10) */
  }অন্যথা{
    জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
      পূর্ণসংখ্যা opMask;
      প্রতিজ্ঞা( pLoop->aLTermSpace==pLoop->aLTerm );
      যদি( !Is_Unique_সূচক(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_চাবি_Col>Array_আকার(pLoop->aLTermSpace)
      ) চালু;
      opMask = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      জন্য(j=0; j<pIdx->n_চাবি_Col; j++){
        pTerm = whereScanInit(&scan, pWC, iCur, j, opMask, pIdx);
        যখন( pTerm && pTerm->prereq_ডান ) pTerm = where_Scan_পরবর্তী(&scan);
        যদি( pTerm==0 ) বিরতি;
        testcase( pTerm->eOperator & WO_IS );
        pLoop->aLTerm[j] = pTerm;
      }
      যদি( j!=pIdx->n_চাবি_Col ) চালু;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_INDEXED;
      যদি( pIdx->isCovering || (pItem->colUsed & pIdx->colNotIdxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_সূচক = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->rRun = 39;  /* 39==sqlite3LogEst(15) */
      বিরতি;
    }
  }
  যদি( pLoop->wsFlags ){
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    প্রতিজ্ঞা( pWInfo->sMaskSet.n==1 && iCur==pWInfo->sMaskSet.ix[0] );
    pLoop->maskSelf = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    যদি( pWInfo->pOrderBy ) pWInfo->nOBSat =  pWInfo->pOrderBy->n_রাশিমালা;
    যদি( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    যদি( scan.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#ifdef SQLITE_DEBUG
    pLoop->cId = '0';
#endif
#ifdef WHERETRACE_ENABLED
    যদি( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#endif
    ফেরত 1;
  }
  ফেরত 0;
}

/*
** Helper function for exprIsDeterministic().
*/
স্থির পূর্ণসংখ্যা রাশিমালা_গ্রন্থি_Is_Deterministic(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_FUNCTION && রাশিমালা_Has_Property(p_রাশিমালা, EP_ConstFunc)==0 ){
    pWalker->eCode = 0;
    ফেরত WRC_Abort;
  }
  ফেরত WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
স্থির পূর্ণসংখ্যা রাশিমালা_Is_Deterministic(রাশিমালা *p){
  Walker w;
  memset(&w, 0, আকার(w));
  w.eCode = 1;
  w.x_রাশিমালা_Callback = রাশিমালা_গ্রন্থি_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_রাশিমালা(&w, p);
  ফেরত w.eCode;
}

 
#ifdef WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
স্থির শূন্য showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  যদি( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    পূর্ণসংখ্যা i;
    স্থির ধ্রুবক অক্ষর zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    জন্য(p=pWInfo->pLoops, i=0; p; p=p->p_পরবর্তী_Loop, i++){
      p->cId = zLabel[i%(আকার(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#অন্যথা
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
স্থির SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  পূর্ণসংখ্যা i;
  Bitmask tabUsed;
  পূর্ণসংখ্যা has_ডান_Join;

  /* Preconditions checked by the caller */
  প্রতিজ্ঞা( pWInfo->nLevel>=2 );
  প্রতিজ্ঞা( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  প্রতিজ্ঞা( pWInfo->p_ফলাফল_Set!=0 );
  প্রতিজ্ঞা( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tabUsed = sqlite3_Where_রাশিমালা_List_Usage(&pWInfo->sMaskSet, pWInfo->p_ফলাফল_Set);
  যদি( pWInfo->pOrderBy ){
    tabUsed |= sqlite3_Where_রাশিমালা_List_Usage(&pWInfo->sMaskSet, pWInfo->pOrderBy);
  }
  has_ডান_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  জন্য(i=pWInfo->nLevel-1; i>=1; i--){
    WhereTerm *pTerm, *pEnd;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    যদি( (pItem->fg.jointype & (JT_বাম|JT_ডান))!=JT_বাম ) চালু;
    যদি( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      চালু;
    }
    যদি( (tabUsed & pLoop->maskSelf)!=0 ) চালু;
    pEnd = pWInfo->sWC.a + pWInfo->sWC.nTerm;
    জন্য(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      যদি( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        যদি( !রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON)
         || pTerm->p_রাশিমালা->w.iJoin!=pItem->iCursor
        ){
          বিরতি;
        }
      }
      যদি( has_ডান_Join
       && রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_InnerON)
       && NEVER(pTerm->p_রাশিমালা->w.iJoin==pItem->iCursor)
      ){
        বিরতি;  /* restriction (5) */
      }
    }
    যদি( pTerm<pEnd ) চালু;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->revMask>>1) & ~m1)!=0 );
    pWInfo->revMask = (m1 & pWInfo->revMask) | ((pWInfo->revMask>>1) & ~m1);
    notReady &= ~pLoop->maskSelf;
    জন্য(pTerm=pWInfo->sWC.a; pTerm<pEnd; pTerm++){
      যদি( (pTerm->prereqAll & pLoop->maskSelf)!=0 ){
        pTerm->wtFlags |= TERM_CODED;
        pTerm->prereqAll = 0;
      }
    }
    যদি( i!=pWInfo->nLevel-1 ){
      পূর্ণসংখ্যা nByte = (pWInfo->nLevel-1-i) * আকার(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    প্রতিজ্ঞা( pWInfo->nLevel>0 );
  }
  ফেরত notReady;
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
স্থির SQLITE_NOINLINE শূন্য whereCheckIfBloomFilterIsUseful(
  ধ্রুবক WhereInfo *pWInfo
){
  পূর্ণসংখ্যা i;
  LogEst n_অনুসন্ধান = 0;

  প্রতিজ্ঞা( pWInfo->nLevel>=2 );
  প্রতিজ্ঞা( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  জন্য(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ধ্রুবক unsigned পূর্ণসংখ্যা reqFlags = (WHERE_SELFCULL|WHERE_COLUMN_EQ);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    Table *pTab = pItem->pSTab;
    যদি( (pTab->tabFlags & TF_HasStat1)==0 ) বিরতি;
    pTab->tabFlags |= TF_MaybeReanalyze;
    যদি( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_অনুসন্ধান > pTab->nRowLogEst
    ){
      testcase( pItem->fg.jointype & JT_বাম );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (দ্বিগুণ)sqlite3LogEstToInt(n_অনুসন্ধান), pTab->zName,
         (দ্বিগুণ)sqlite3LogEstToInt(pTab->nRowLogEst)));
    }
    n_অনুসন্ধান += pLoop->nOut;
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
স্থির SQLITE_NOINLINE শূন্য where_Add_Indexed_রাশিমালা(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  সূচক *pIdx,       /* The index-on-expression that contains the expressions */
  পূর্ণসংখ্যা iIdxCur,       /* Cursor number for pIdx */
  SrcItem *pTabItem  /* The FROM clause entry for the table */
){
  পূর্ণসংখ্যা i;
  Indexed_রাশিমালা *p;
  Table *pTab;
  প্রতিজ্ঞা( pIdx->b_Has_রাশিমালা );
  pTab = pIdx->pTable;
  জন্য(i=0; i<pIdx->nColumn; i++){
    রাশিমালা *p_রাশিমালা;
    পূর্ণসংখ্যা j = pIdx->aiColumn[i];
    যদি( j==XN_রাশিমালা ){
      p_রাশিমালা = pIdx->a_Col_রাশিমালা->a[i].p_রাশিমালা;
    }অন্যথা যদি( j>=0 && (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)!=0 ){
      p_রাশিমালা = sqlite3_Column_রাশিমালা(pTab, &pTab->aCol[j]);
    }অন্যথা{
      চালু;
    }
    যদি( sqlite3_রাশিমালা_Is_Constant(0,p_রাশিমালা) ) চালু;
    p = sqlite3DbMallocRaw(pParse->db,  আকার(Indexed_রাশিমালা));
    যদি( p==0 ) বিরতি;
    p->pIENext = pParse->pIdxEpr;
#ifdef WHERETRACE_ENABLED
    যদি( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      যদি( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_রাশিমালা(p_রাশিমালা);
    }
#endif
    p->p_রাশিমালা = sqlite3_রাশিমালা_Dup(pParse->db, p_রাশিমালা, 0);
    p->i_তথ্য_Cur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান))!=0;
    যদি( sqlite3_সূচক_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->zColAff[i];
    }
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#endif
    pParse->pIdxEpr = p;
    যদি( p->pIENext==0 ){
      শূন্য *pArg = (শূন্য*)&pParse->pIdxEpr;
      sqlite3ParserAddCleanup(pParse, where_Indexed_রাশিমালা_Cleanup, pArg);
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
স্থির SQLITE_NOINLINE শূন্য whereReverseScanOrder(WhereInfo *pWInfo){
  পূর্ণসংখ্যা ii;
  জন্য(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    যদি( !pItem->fg.isCte
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
  রাশিমালা *pWhere,           /* The WHERE clause */
  রাশিমালা_List *pOrderBy,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  রাশিমালা_List *p_ফলাফল_Set,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  পূর্ণসংখ্যা iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  পূর্ণসংখ্যা nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  পূর্ণসংখ্যা nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  WhereMaskSet *pMaskSet;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  পূর্ণসংখ্যা ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  পূর্ণসংখ্যা rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  প্রতিজ্ঞা( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  প্রতিজ্ঞা( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_LIMIT)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, আকার(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( pOrderBy && pOrderBy->n_রাশিমালা==BMS-1 );
  যদি( pOrderBy && pOrderBy->n_রাশিমালা>=BMS ){
    pOrderBy = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  যদি( pTabList->nSrc>BMS ){
    sqlite3_Error_বার্তা(pParse, "at most %d tables in a join", BMS);
    ফেরত 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + আকার(WhereLoop));
  যদি( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    যাও whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;
#যদি WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#endif
  pWInfo->p_ফলাফল_Set = p_ফলাফল_Set;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->iLimit = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  memset(&pWInfo->a[0], 0, আকার(WhereLoop)+nTabList*আকার(WhereLevel));
  প্রতিজ্ঞা( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  pMaskSet = &pWInfo->sMaskSet;
  pMaskSet->n = 0;
  pMaskSet->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((অক্ষর*)pWInfo)+nByteWInfo);
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
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
  যদি( nTabList==0 ){
    যদি( pOrderBy ) pWInfo->nOBSat = pOrderBy->n_রাশিমালা;
    যদি( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    যদি( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_মান)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }অন্যথা{
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
    করো{
      createMask(pMaskSet, pTabList->a[ii].iCursor);
      sqlite3WhereTabFuncArgs(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }যখন( (++ii)<pTabList->nSrc );
  #ifdef SQLITE_DEBUG
    {
      Bitmask mx = 0;
      জন্য(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3WhereGetMask(pMaskSet, pTabList->a[ii].iCursor);
        প্রতিজ্ঞা( m>=mx );
        mx = m;
      }
    }
  #endif
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_রাশিমালা_Analyze(pTabList, &pWInfo->sWC);
  যদি( pSelect && pSelect->pLimit ){
    sqlite3WhereAddLimit(&pWInfo->sWC, pSelect);
  }
  যদি( pParse->nErr ) যাও whereBeginError;

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
  জন্য(ii=0; ii<sWLB.pWC->nBase; ii++){
    WhereTerm *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    রাশিমালা *pX;                          /* The expression of pT */
    যদি( pT->wtFlags & TERM_VIRTUAL ) চালু;
    pX = pT->p_রাশিমালা;
    প্রতিজ্ঞা( pX!=0 );
    প্রতিজ্ঞা( pT->prereqAll!=0 || !রাশিমালা_Has_Property(pX, EP_OuterON) );
    যদি( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || রাশিমালা_Is_Deterministic(pX))   /* Condition (4) */
     && !(রাশিমালা_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_রাশিমালা_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= TERM_CODED;
    }
  }

  যদি( wctrlFlags & WHERE_WANT_DISTINCT ){
    যদি( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }অন্যথা যদি( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_ফলাফল_Set) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }অন্যথা যদি( pOrderBy==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->pOrderBy = p_ফলাফল_Set;
    }
  }

  /* Construct the WhereLoop objects */
#যদি defined(WHERETRACE_ENABLED)
  যদি( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    যদি( wctrlFlags & WHERE_USE_LIMIT ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    যদি( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, আকার(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.pOrderBy = pOrderBy;
      sSelect.pEList = p_ফলাফল_Set;
      sqlite3_বৃক্ষ_View_Select(0, &sSelect, 0);
    }
    যদি( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#endif

  যদি( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = whereLoopAddAll(&sWLB);
    যদি( rc ) যাও whereBeginError;

#ifdef SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    যদি( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      যখন( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_পরবর্তী_Loop;
        whereLoopDelete(db, p);
      }
      rc = whereLoopAddAll(&sWLB);
      যদি( rc ) যাও whereBeginError;
    }
#endif
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    wherePathSolver(pWInfo, 0);
    যদি( db->mallocFailed ) যাও whereBeginError;
    যদি( pWInfo->pOrderBy ){
       whereInterstageHeuristic(pWInfo);
       wherePathSolver(pWInfo, pWInfo->nRowOut<0 ? 1 : pWInfo->nRowOut+1);
       যদি( db->mallocFailed ) যাও whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    যদি( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->nRowOut, pWInfo->nRowOut-30));
      pWInfo->nRowOut -= 30;
    }

  }
  প্রতিজ্ঞা( pWInfo->pTabList!=0 );
  যদি( pWInfo->pOrderBy==0 && (db->flags & SQLITE_ReverseOrder)!=0 ){
    whereReverseScanOrder(pWInfo);
  }
  যদি( pParse->nErr ){
    যাও whereBeginError;
  }
  প্রতিজ্ঞা( db->mallocFailed==0 );
#ifdef WHERETRACE_ENABLED
  যদি( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_যোগ_Cost, pWInfo->nRowOut);
    যদি( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->revMask);
    }
    নির্বাচন( pWInfo->eDistinct ){
      ক্ষেত্র WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        বিরতি;
      }
      ক্ষেত্র WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        বিরতি;
      }
      ক্ষেত্র WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        বিরতি;
      }
    }
    sqlite3DebugPrintf("\n");
    জন্য(ii=0; ii<pWInfo->nLevel; ii++){
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
  যদি( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && p_ফলাফল_Set!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    প্রতিজ্ঞা( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  যদি( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#যদি defined(WHERETRACE_ENABLED)
  যদি( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
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
  প্রতিজ্ঞা( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  যদি( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    পূর্ণসংখ্যা wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    পূর্ণসংখ্যা bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    প্রতিজ্ঞা( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    যদি( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      যদি( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        যদি( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  জন্য(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    Table *pTab;     /* Table to open */
    পূর্ণসংখ্যা iDb;         /* Index of database containing table/index */
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3_Schema_To_সূচক(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    যদি( ii==0 || (pTabItem[0].fg.jointype & JT_বাম)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }অন্যথা যদি( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }অন্যথা{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    যদি( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }অন্যথা
#ifndef SQLITE_OMIT_VIRTUALTABLE
    যদি( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ধ্রুবক অক্ষর *pVTab = (ধ্রুবক অক্ষর *)sqlite3GetVTable(db, pTab);
      পূর্ণসংখ্যা iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }অন্যথা যদি( IsVirtual(pTab) ){
      /* noop */
    }অন্যথা
#endif
    যদি( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (pTabItem->fg.jointype & (JT_LTORJ|JT_ডান))!=0
    ){
      পূর্ণসংখ্যা op = OP_OpenRead;
      যদি( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);
      প্রতিজ্ঞা( pTabItem->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS );
      যদি( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->nCol<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_সূচক|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = pTabItem->colUsed;
        পূর্ণসংখ্যা n = 0;
        জন্য(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_TO_PTR(n), P4_INT32);
        প্রতিজ্ঞা( n<=pTab->nCol );
      }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
      যদি( pLoop->u.btree.p_সূচক!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }অন্যথা
#endif
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
      sqlite3VdbeAddOp3(v, OP_ColumnsUsed, pTabItem->iCursor,
                    LOWER32(pTabItem->colUsed), UPPER32(pTabItem->colUsed));
#endif
      যদি( ii>=2
       && (pTabItem[0].fg.jointype & (JT_LTORJ|JT_বাম))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3VdbeAddOp2(v, OP_IfEmpty, pTabItem->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }অন্যথা{
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    যদি( pLoop->wsFlags & WHERE_INDEXED ){
      সূচক *pIx = pLoop->u.btree.p_সূচক;
      পূর্ণসংখ্যা i_সূচক_Cur;
      পূর্ণসংখ্যা op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      প্রতিজ্ঞা( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      যদি( !HasRowid(pTab) && Is_Primary_চাবি_সূচক(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_সূচক_Cur = pLevel->iTabCur;
        op = 0;
      }অন্যথা যদি( pWInfo->eOnePass!=ONEPASS_OFF ){
        সূচক *pJ = pTabItem->pSTab->p_সূচক;
        i_সূচক_Cur = iAuxArg;
        প্রতিজ্ঞা( wctrlFlags & WHERE_ONEPASS_DESIRED );
        যখন( ALWAYS(pJ) && pJ!=pIx ){
          i_সূচক_Cur++;
          pJ = pJ->p_পরবর্তী;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_সূচক_Cur;
      }অন্যথা যদি( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_সূচক_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }অন্যথা{
        i_সূচক_Cur = pParse->nTab++;
        যদি( pIx->b_Has_রাশিমালা && OptimizationEnabled(db, SQLITE_Indexed_রাশিমালা) ){
          where_Add_Indexed_রাশিমালা(pParse, pIx, i_সূচক_Cur, pTabItem);
        }
        যদি( pIx->pPartIdxWhere && (pTabItem->fg.jointype & JT_ডান)==0 ){
          where_Part_Idx_রাশিমালা(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_সূচক_Cur, pTabItem
          );
        }
      }
      pLevel->iIdxCur = i_সূচক_Cur;
      প্রতিজ্ঞা( pIx!=0 );
      প্রতিজ্ঞা( pIx->pSchema==pTab->pSchema );
      প্রতিজ্ঞা( i_সূচক_Cur>=0 );
      যদি( op ){
        sqlite3VdbeAddOp3(v, op, i_সূচক_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pIx);
        যদি( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
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
          পূর্ণসংখ্যা ii, jj;
          জন্য(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            যদি( jj<0 ) চালু;
            যদি( jj>63 ) jj = 63;
            যদি( (pTabItem->colUsed & MASKBIT(jj))==0 ) চালু;
            colUsed |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3VdbeAddOp3(v, OP_ColumnsUsed, i_সূচক_Cur,
                            LOWER32(colUsed),UPPER32(colUsed));
        }
#endif /* SQLITE_ENABLE_COLUMN_USED_MASK */
#ifdef SQLITE_ENABLE_CURSOR_HINTS
        যদি( HasRowid(pTab) ){
          sqlite3VdbeAddOp3(v, OP_CursorHint, i_সূচক_Cur, 0, pTabItem->iCursor);
        }
#endif
      }
    }
    যদি( iDb>=0 ) sqlite3CodeVerifySchema(pParse, iDb);
    যদি( (pTabItem->fg.jointype & JT_ডান)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, আকার(Where_ডান_Join)))!=0
    ){
      Where_ডান_Join *pRJ = pLevel->pRJ;
      পূর্ণসংখ্যা bBloomable = 0;
      pRJ->iMatch = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, pRJ->regReturn);
      প্রতিজ্ঞা( pTab==pTabItem->pSTab );
      যদি( HasRowid(pTab) ){
        চাবি_Info *pInfo;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, 1);
        bBloomable = 1;
        pInfo = sqlite3_চাবি_Info_Alloc(pParse->db, 1, 0);
        যদি( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->aSortFlags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }অন্যথা{
        সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pRJ->iMatch, pPk->n_চাবি_Col);
        sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pPk);
        bBloomable = sqlite3_সূচক_Bloomable(pPk,pPk->n_চাবি_Col);
      }
      যদি( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Blob, 65536, pRJ->regBloom);
      }অন্যথা{
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
  pWInfo->i_শীর্ষ_2 = sqlite3VdbeCurrentAddr(v);
  যদি( db->mallocFailed ) যাও whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  জন্য(ii=0; ii<nTabList; ii++){
    পূর্ণসংখ্যা addrExplain;
    পূর্ণসংখ্যা wsFlags;
    SrcItem *pSrc;
    যদি( pParse->nErr ) যাও whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    যদি( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      পূর্ণসংখ্যা iOnce = 0;
      প্রতিজ্ঞা( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      যদি( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }অন্যথা{
        iOnce = 0;
      }
      sqlite3VdbeAddOp2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      যদি( iOnce )  sqlite3VdbeJumpHere(v, iOnce);
    }
    প্রতিজ্ঞা( pTabList == pWInfo->pTabList );
    যদি( (wsFlags & (WHERE_AUTO_সূচক|WHERE_BLOOMFILTER))!=0 ){
      যদি( (wsFlags & WHERE_AUTO_সূচক)!=0 ){
#ifndef SQLITE_OMIT_AUTOMATIC_সূচক
        construct_Automatic_সূচক(pParse, &pWInfo->sWC, notReady, pLevel);
#endif
      }অন্যথা{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      যদি( db->mallocFailed ) যাও whereBeginError;
    }
    addrExplain = sqlite3WhereExplainOneScan(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    যদি( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3WhereAddScanStatus(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->iEndWhere = sqlite3VdbeCurrentAddr(v);
  ফেরত pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  যদি( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#ifdef WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#endif /* WHERETRACE_ENABLED */
  ফেরত 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#ifndef SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#অন্যথা
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  স্থির শূন্য sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    পূর্ণসংখ্যা pc,
    VdbeOp *pOp
  ){
    যদি( (db->flags & SQLITE_VdbeAddopTrace)==0 ) ফেরত;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3ShowWhereTerm(0); /* So compiler won't complain about unused func */
  }
#endif

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
শূন্য sqlite3WhereEnd(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  পূর্ণসংখ্যা i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  পূর্ণসংখ্যা iEnd = sqlite3VdbeCurrentAddr(v);
  পূর্ণসংখ্যা nRJ = 0;
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  পূর্ণসংখ্যা addrSeek = 0;
#endif

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  জন্য(i=pWInfo->nLevel-1; i>=0; i--){
    পূর্ণসংখ্যা addr;
    pLevel = &pWInfo->a[i];
    যদি( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_ডান_Join *pRJ = pLevel->pRJ;
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
    যদি( pLevel->op!=OP_Noop ){
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      সূচক *pIdx;
      পূর্ণসংখ্যা n;
      যদি( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_INDEXED)!=0
       && (pIdx = pLoop->u.btree.p_সূচক)->hasStat1
       && (n = pLoop->u.btree.nDistinctCol)>0
       && pIdx->aiRowLogEst[n]>=36
      ){
        পূর্ণসংখ্যা r1 = pParse->nMem+1;
        পূর্ণসংখ্যা j, op;
        পূর্ণসংখ্যা addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        যদি( pLevel->i_বাম_Join ){
          addrIfNull = sqlite3VdbeAddOp2(v, OP_IfNullRow, pLevel->iIdxCur, r1);
        }
        জন্য(j=0; j<n; j++){
          sqlite3VdbeAddOp3(v, OP_Column, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_পূর্ব ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3VdbeAddOp4Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3VdbeAddOp2(v, OP_Goto, 1, pLevel->p2);
        যদি( pLevel->i_বাম_Join ){
          sqlite3VdbeJumpHere(v, addrIfNull);
        }
      }
#endif /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    যদি( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }অন্যথা যদি( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      পূর্ণসংখ্যা iBypass = sqlite3VdbeAddOp0(v, OP_Goto);
      প্রতিজ্ঞা( pLoop->wsFlags & WHERE_AUTO_সূচক );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3VdbeAddOp1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3VdbeJumpHere(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    যদি( pLevel->op!=OP_Noop ){
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_পরবর্তী);
      VdbeCoverageIf(v, pLevel->op==OP_পূর্ব);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#ifndef SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      যদি( addrSeek ){
        sqlite3VdbeJumpHere(v, addrSeek);
        addrSeek = 0;
      }
#endif
      যদি( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3VdbeAddOp2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    যদি( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
    প্রতিজ্ঞা( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_IN_ABLE) && pLevel->u.in.nIn>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    যদি( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3VdbeAddOp2(v, OP_DecrJumpZero, (পূর্ণসংখ্যা)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#endif
    যদি( (pLoop->wsFlags & WHERE_IN_ABLE)!=0 && pLevel->u.in.nIn>0 ){
      কাঠামো InLoop *pIn;
      পূর্ণসংখ্যা j;
      জন্য(j=pLevel->u.in.nIn, pIn=&pLevel->u.in.aInLoop[j-1]; j>0; j--, pIn--){
        প্রতিজ্ঞা( sqlite3VdbeGetOp(v, pIn->addr_In_শীর্ষ+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3VdbeJumpHere(v, pIn->addr_In_শীর্ষ+1);
        যদি( pIn->eEndLoopOp!=OP_Noop ){
          যদি( pIn->nPrefix ){
            পূর্ণসংখ্যা bEarlyOut =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0;
            যদি( pLevel->i_বাম_Join ){
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
            যদি( bEarlyOut ){
              sqlite3VdbeAddOp4Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  pIn->iBase, pIn->nPrefix);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3VdbeJumpHere(v, pIn->addr_In_শীর্ষ+1);
            }
          }
          sqlite3VdbeAddOp2(v, pIn->eEndLoopOp, pIn->iCur, pIn->addr_In_শীর্ষ);
          VdbeCoverage(v);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_পূর্ব);
          VdbeCoverageIf(v, pIn->eEndLoopOp==OP_পরবর্তী);
        }
        sqlite3VdbeJumpHere(v, pIn->addr_In_শীর্ষ-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    যদি( pLevel->pRJ ){
      sqlite3VdbeAddOp3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    যদি( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_সূচক->zName));
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip-2);
    }
    যদি( pLevel->i_বাম_Join ){
      পূর্ণসংখ্যা ws = pLoop->wsFlags;
      addr = sqlite3VdbeAddOp1(v, OP_IfPos, pLevel->i_বাম_Join); VdbeCoverage(v);
      প্রতিজ্ঞা( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_INDEXED)!=0 );
      যদি( (ws & WHERE_IDX_ONLY)==0 ){
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];
        প্রতিজ্ঞা( pLevel->iTabCur==pSrc->iCursor );
        যদি( pSrc->fg.viaCoroutine ){
          পূর্ণসংখ্যা m, n;
          প্রতিজ্ঞা( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_ফলাফল;
          প্রতিজ্ঞা( pSrc->pSTab!=0 );
          m = pSrc->pSTab->nCol;
          sqlite3VdbeAddOp3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iTabCur);
      }
      যদি( (ws & WHERE_INDEXED)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        যদি( ws & WHERE_MULTI_OR ){
          সূচক *pIx = pLevel->u.pCoveringIdx;
          পূর্ণসংখ্যা iDb = sqlite3_Schema_To_সূচক(db, pIx->pSchema);
          sqlite3VdbeAddOp3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pIx);
        }
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
      }
      যদি( ws & WHERE_IN_ABLE ){
        পূর্ণসংখ্যা m;
        জন্য(m=0; m<pLevel->u.in.nIn; m++){
          কাঠামো InLoop *pIn = &pLevel->u.in.aInLoop[m];
          যদি( pIn->eEndLoopOp!=OP_Noop ){
            sqlite3VdbeAddOp1(v, OP_NullRow, pIn->iCur);
          }
        }
      }
      যদি( pLevel->op==OP_Return ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }অন্যথা{
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  প্রতিজ্ঞা( pWInfo->nLevel<=pTabList->nSrc );
  জন্য(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    পূর্ণসংখ্যা k, last;
    VdbeOp *pOp, *pLastOp;
    সূচক *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pSTab;
    প্রতিজ্ঞা( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    যদি( pLevel->pRJ ){
      sqlite3_Where_ডান_Join_Loop(pWInfo, i, pLevel);
      চালু;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    যদি( pTabItem->fg.viaCoroutine ){
      testcase( pParse->db->mallocFailed );
      প্রতিজ্ঞা( pTabItem->fg.isSubquery );
      প্রতিজ্ঞা( pTabItem->u4.pSubq->reg_ফলাফল>=0 );
      translateColumnToCopy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->u4.pSubq->reg_ফলাফল, 0);
      চালু;
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
    যদি( pLoop->wsFlags & (WHERE_INDEXED|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_সূচক;
    }অন্যথা যদি( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    যদি( pIdx
     && !db->mallocFailed
    ){
      যদি( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->pTable) ){
        last = iEnd;
      }অন্যথা{
        last = pWInfo->iEndWhere;
      }
      যদি( pIdx->b_Has_রাশিমালা ){
        Indexed_রাশিমালা *p = pParse->pIdxEpr;
        যখন( p ){
          যদি( p->iIdxCur==pLevel->iIdxCur ){
#ifdef WHERETRACE_ENABLED
            যদি( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->iIdxCol);
              যদি( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_রাশিমালা(p->p_রাশিমালা);
            }
#endif
            p->i_তথ্য_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#ifdef SQLITE_DEBUG
      যদি( db->flags & SQLITE_VdbeAddopTrace ){
        printf("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, last-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      প্রতিজ্ঞা( pOp->opcode!=OP_Column || pOp->p1!=pLevel->iTabCur );
      প্রতিজ্ঞা( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      প্রতিজ্ঞা( pOp->opcode!=OP_IfNullRow || pOp->p1!=pLevel->iTabCur );
#endif
      pOp = sqlite3VdbeGetOp(v, k);
      pLastOp = pOp + (last - k);
      প্রতিজ্ঞা( pOp<=pLastOp );
      করো{
        যদি( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }অন্যথা যদি( pOp->opcode==OP_Column
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
         || pOp->opcode==OP_Offset
#endif
        ){
          পূর্ণসংখ্যা x = pOp->p2;
          প্রতিজ্ঞা( pIdx->pTable==pTab );
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
          যদি( pOp->opcode==OP_Offset ){
            /* Do not need to translate the column number */
          }অন্যথা
#endif
          যদি( !HasRowid(pTab) ){
            সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
            x = pPk->aiColumn[x];
            প্রতিজ্ঞা( x>=0 );
          }অন্যথা{
            testcase( x!=sqlite3StorageColumnToTable(pTab,x) );
            x = sqlite3StorageColumnToTable(pTab,x);
          }
          x = sqlite3_Table_Column_To_সূচক(pIdx, x);
          যদি( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }অন্যথা যদি( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            যদি( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_বার্তা(pParse, "internal query planner error");
              pParse->rc = SQLITE_INTERNAL;
            }অন্যথা{
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
        }অন্যথা যদি( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }অন্যথা যদি( pOp->opcode==OP_IfNullRow ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#ifdef SQLITE_DEBUG
        k++;
#endif
      }যখন( (++pOp)<pLastOp );
#ifdef SQLITE_DEBUG
      যদি( db->flags & SQLITE_VdbeAddopTrace ) printf("TRANSLATE complete\n");
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
  ফেরত;
}
