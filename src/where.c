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
#ضمن "sqliteInt.h"
#ضمن "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
تعريفنوع بنية Hidden_شاخص_Info Hidden_شاخص_Info;
بنية Hidden_شاخص_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  صحيح eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 m_درون;                 /* Mask of terms that are <col> IN (...) */
  u32 m_Handle_درون;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_مقدار *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#تعریف_کن SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_شاخص_Info,aRhs) + (N)*حجم(sqlite3_مقدار*))

/* Forward declaration of methods */
ساكن صحيح whereLoopResize(sqlite3*, WhereLoop*, صحيح);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_ردیف_شمار(WhereInfo *pWInfo){
  أرجع pWInfo->n_ردیف_بیرون;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
صحيح sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  أرجع pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
صحيح sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  أرجع pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
صحيح sqlite3_Where_ترتیب_By_حد_Opt_Label(WhereInfo *pWInfo){
  WhereLevel *pInner;
  إذا( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    أرجع pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  أكد( pInner->addrNxt!=0 );
  أرجع pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
فراغ sqlite3_Where_Min_Max_Opt_Early_بیرون(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  صحيح i;
  إذا( !pWInfo->bOrderedInnerLoop ) أرجع;
  إذا( pWInfo->nOBSat==0 ) أرجع;
  لكل(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    إذا( (pInner->pWLoop->wsFlags & WHERE_ستونی_درون)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      أرجع;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
صحيح sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  أكد( pWInfo->iContinue!=0 );
  أرجع pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
صحيح sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  أرجع pWInfo->iBreak;
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
صحيح sqlite3WhereOkOnePass(WhereInfo *pWInfo, صحيح *aiCur){
  کپی_حافظه(aiCur, pWInfo->aiCurOnePass, حجم(صحيح)*2);
#اگر_تعریف_شده WHERETRACE_ENABLED
  إذا( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#پایان_اگر
  أرجع pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
صحيح sqlite3_Where_استفاده_میکند_Deferred_Seek(WhereInfo *pWInfo){
  أرجع pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
ساكن فراغ whereOrMove(Where_Or_مجموعه *pDest, Where_Or_مجموعه *pSrc){
  pDest->n = pSrc->n;
  کپی_حافظه(pDest->a, pSrc->a, pDest->n*حجم(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
ساكن صحيح where_Or_درج(
  Where_Or_مجموعه *p_مجموعه,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst r_پیاپی,           /* Run-cost of the new entry */
  LogEst n_بیرون            /* Number of outputs for the new entry */
){
  u16 i;
  Where_Or_هزینه *p;
  لكل(i=p_مجموعه->n, p=p_مجموعه->a; i>0; i--, p++){
    إذا( r_پیاپی<=p->r_پیاپی && (prereq & p->prereq)==prereq ){
      اذهب where_Or_درج_انجامشده;
    }
    إذا( p->r_پیاپی<=r_پیاپی && (p->prereq & prereq)==p->prereq ){
      أرجع 0;
    }
  }
  إذا( p_مجموعه->n<N_OR_هزینه ){
    p = &p_مجموعه->a[p_مجموعه->n++];
    p->n_بیرون = n_بیرون;
  }وإلا{
    p = p_مجموعه->a;
    لكل(i=1; i<p_مجموعه->n; i++){
      إذا( p->r_پیاپی>p_مجموعه->a[i].r_پیاپی ) p = p_مجموعه->a + i;
    }
    إذا( p->r_پیاپی<=r_پیاپی ) أرجع 0;
  }
where_Or_درج_انجامشده:
  p->prereq = prereq;
  p->r_پیاپی = r_پیاپی;
  إذا( p->n_بیرون>n_بیرون ) p->n_بیرون = n_بیرون;
  أرجع 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3_Where_Get_ماسک(Where_ماسک_مجموعه *p_ماسک_مجموعه, صحيح iCursor){
  صحيح i;
  أكد( p_ماسک_مجموعه->n<=(صحيح)حجم(Bitmask)*8 );
  أكد( p_ماسک_مجموعه->n>0 || p_ماسک_مجموعه->ix[0]<0 );
  أكد( iCursor>=-1 );
  إذا( p_ماسک_مجموعه->ix[0]==iCursor ){
    أرجع 1;
  }
  لكل(i=1; i<p_ماسک_مجموعه->n; i++){
    إذا( p_ماسک_مجموعه->ix[i]==iCursor ){
      أرجع MASKBIT(i);
    }
  }
  أرجع 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
فراغ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  Where_Mem_بلوک *p_بلوک;
  p_بلوک = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+حجم(*p_بلوک));
  إذا( p_بلوک ){
    p_بلوک->p_بعدی = pWInfo->p_Mem_مقصد_Free;
    p_بلوک->sz = nByte;
    pWInfo->p_Mem_مقصد_Free = p_بلوک;
    p_بلوک++;
  }
  أرجع (فراغ*)p_بلوک;
}
فراغ *sqlite3WhereRealloc(WhereInfo *pWInfo, فراغ *pOld, u64 nByte){
  فراغ *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  إذا( pNew && pOld ){
    Where_Mem_بلوک *pOldBlk = (Where_Mem_بلوک*)pOld;
    pOldBlk--;
    أكد( pOldBlk->sz<nByte );
    کپی_حافظه(pNew, pOld, pOldBlk->sz);
  }
  أرجع pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
ساكن فراغ create_ماسک(Where_ماسک_مجموعه *p_ماسک_مجموعه, صحيح iCursor){
  أكد( p_ماسک_مجموعه->n < Array_اندازه(p_ماسک_مجموعه->ix) );
  p_ماسک_مجموعه->ix[p_ماسک_مجموعه->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
ساكن عبارت *where_راست_Subexpr_Is_ستونی(عبارت *p){
  p = sqlite3_عبارت_Skip_Collate_And_Likely(p->p_راست);
  إذا( ALWAYS(p!=0) && p->op==TK_ستونی && !عبارت_Has_Property(p, EP_Fixed_ستون) ){
    أرجع p;
  }
  أرجع 0;
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
ساكن SQLITE_NOINLINE ثابت حرف *شاخص_درون_Affinity_Ok(
  Parse *pParse, 
  Where_جمله *p_جمله, 
  u8 idxaff
){
  عبارت *pX = p_جمله->p_عبارت;
  عبارت inexpr;

  أكد( p_جمله->eOperator & WO_درون );

  إذا( عبارت_Use_XSelect(pX) ){
    صحيح i_فیلد = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    إذا( sqlite3_عبارت_Is_وکتور(pX->p_چپ) ){
      i_فیلد = p_جمله->u.x.i_فیلد - 1;
      inexpr.p_چپ = pX->p_چپ->x.pList->a[i_فیلد].p_عبارت;
    }وإلا{
      inexpr.p_چپ = pX->p_چپ;
    }
    inexpr.p_راست = pX->x.pSelect->pEList->a[i_فیلد].p_عبارت;
    pX = &inexpr;
  }

  إذا( sqlite3_شاخص_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_عبارت_Compare_Coll_Seq(pParse, pX);
    أرجع pRet ? pRet->zName : sqlite3_Str_دودویی;
  }
  أرجع 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
ساكن Where_جمله *where_پیمایش_بعدی(Where_پیمایش *p_پیمایش){
  صحيح iCur;            /* The cursor on the LHS of the term */
  i16 i_ستونی;         /* The column on the LHS of the term.  -1 for IPK */
  عبارت *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  Where_جمله *p_جمله;    /* The term being tested */
  صحيح k = p_پیمایش->k;    /* Where to start scanning */

  أكد( p_پیمایش->iEquiv<=p_پیمایش->nEquiv );
  pWC = p_پیمایش->pWC;
  طالما(1){
    i_ستونی = p_پیمایش->ai_ستونی[p_پیمایش->iEquiv-1];
    iCur = p_پیمایش->aiCur[p_پیمایش->iEquiv-1];
    أكد( pWC!=0 );
    أكد( iCur>=0 );
    افعل{
      لكل(p_جمله=pWC->a+k; k<pWC->n_جمله; k++, p_جمله++){
        أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 || p_جمله->چپ_Cursor<0 );
        إذا( p_جمله->چپ_Cursor==iCur
         && p_جمله->u.x.چپ_ستونی==i_ستونی
         && (i_ستونی!=XN_عبارت
             || sqlite3_عبارت_Compare_Skip(p_جمله->p_عبارت->p_چپ,
                                       p_پیمایش->p_Idx_عبارت,iCur)==0)
         && (p_پیمایش->iEquiv<=1 || !عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON))
        ){
          إذا( (p_جمله->eOperator & WO_EQUIV)!=0
           && p_پیمایش->nEquiv<Array_اندازه(p_پیمایش->aiCur)
           && (pX = where_راست_Subexpr_Is_ستونی(p_جمله->p_عبارت))!=0
          ){
            صحيح j;
            لكل(j=0; j<p_پیمایش->nEquiv; j++){
              إذا( p_پیمایش->aiCur[j]==pX->i_جدول
               && p_پیمایش->ai_ستونی[j]==pX->i_ستونی ){
                  اكسر;
              }
            }
            إذا( j==p_پیمایش->nEquiv ){
              p_پیمایش->aiCur[j] = pX->i_جدول;
              p_پیمایش->ai_ستونی[j] = pX->i_ستونی;
              p_پیمایش->nEquiv++;
            }
          }
          إذا( (p_جمله->eOperator & p_پیمایش->op_ماسک)!=0 ){
            /* Verify the affinity and collating sequence match */
            إذا( p_پیمایش->zCollName && (p_جمله->eOperator & WO_ISNULL)==0 ){
              ثابت حرف *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = p_جمله->p_عبارت;

              إذا( (p_جمله->eOperator & WO_درون) ){
                zCollName = شاخص_درون_Affinity_Ok(pParse, p_جمله, p_پیمایش->idxaff);
                إذا( !zCollName ) استمر;
              }وإلا{
                CollSeq *pColl;
                إذا( !sqlite3_شاخص_Affinity_Ok(pX, p_پیمایش->idxaff) ){
                  استمر;
                }
                أكد(pX->p_چپ);
                pColl = sqlite3_عبارت_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_دودویی;
              }

              إذا( sqlite3StrICmp(zCollName, p_پیمایش->zCollName) ){
                استمر;
              }
            }
            إذا( (p_جمله->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = p_جمله->p_عبارت->p_راست, ALWAYS(pX!=0))
             && pX->op==TK_ستونی
             && pX->i_جدول==p_پیمایش->aiCur[0]
             && pX->i_ستونی==p_پیمایش->ai_ستونی[0]
            ){
              testcase( p_جمله->eOperator & WO_IS );
              استمر;
            }
            p_پیمایش->pWC = pWC;
            p_پیمایش->k = k+1;
#اگر_تعریف_شده WHERETRACE_ENABLED
            إذا( (sqlite3WhereTrace & 0x20000)!=0 && p_پیمایش->nEquiv>1 ){
              صحيح ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 p_پیمایش->aiCur[0], p_پیمایش->ai_ستونی[0], p_جمله->i_جمله);
              لكل(ii=1; ii<p_پیمایش->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   p_پیمایش->aiCur[ii], p_پیمایش->ai_ستونی[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#پایان_اگر
            أرجع p_جمله;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }طالما( pWC!=0 );
    إذا( p_پیمایش->iEquiv>=p_پیمایش->nEquiv ) اكسر;
    pWC = p_پیمایش->pOrigWC;
    k = 0;
    p_پیمایش->iEquiv++;
  }
  أرجع 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
ساكن SQLITE_NOINLINE Where_جمله *where_پیمایش_Init_شاخص_عبارت(Where_پیمایش *p_پیمایش){
  p_پیمایش->idxaff = sqlite3_عبارت_Affinity(p_پیمایش->p_Idx_عبارت);
  أرجع where_پیمایش_بعدی(p_پیمایش);
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
ساكن Where_جمله *where_پیمایش_Init(
  Where_پیمایش *p_پیمایش,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  صحيح iCur,               /* Cursor to scan for */
  صحيح i_ستونی,            /* Column to scan for */
  u32 op_ماسک,             /* Operator(s) to scan for */
  شاخص *pIdx             /* Must be compatible with this index */
){
  p_پیمایش->pOrigWC = pWC;
  p_پیمایش->pWC = pWC;
  p_پیمایش->p_Idx_عبارت = 0;
  p_پیمایش->idxaff = 0;
  p_پیمایش->zCollName = 0;
  p_پیمایش->op_ماسک = op_ماسک;
  p_پیمایش->k = 0;
  p_پیمایش->aiCur[0] = iCur;
  p_پیمایش->nEquiv = 1;
  p_پیمایش->iEquiv = 1;
  إذا( pIdx ){
    صحيح j = i_ستونی;
    i_ستونی = pIdx->ai_ستونی[j];
    إذا( i_ستونی==pIdx->p_جدول->iPKey ){
      i_ستونی = XN_ROWID;
    }وإلا إذا( i_ستونی>=0 ){
      p_پیمایش->idxaff = pIdx->p_جدول->a_ستون[i_ستونی].affinity;
      p_پیمایش->zCollName = pIdx->azColl[j];
    }وإلا إذا( i_ستونی==XN_عبارت ){
      p_پیمایش->p_Idx_عبارت = pIdx->a_ستون_عبارت->a[j].p_عبارت;
      p_پیمایش->zCollName = pIdx->azColl[j];
      p_پیمایش->ai_ستونی[0] = XN_عبارت;
      أرجع where_پیمایش_Init_شاخص_عبارت(p_پیمایش);
    }
  }وإلا إذا( i_ستونی==XN_عبارت ){
    أرجع 0;
  }
  p_پیمایش->ai_ستونی[0] = i_ستونی;
  أرجع where_پیمایش_بعدی(p_پیمایش);
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
Where_جمله *sqlite3_Where_پیدا_کن_جمله(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  صحيح iCur,             /* Cursor number of LHS */
  صحيح i_ستونی,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  شاخص *pIdx           /* Must be compatible with this index, if not NULL */
){
  Where_جمله *p_نتیجه = 0;
  Where_جمله *p;
  Where_پیمایش پیمایش;

  p = where_پیمایش_Init(&پیمایش, pWC, iCur, i_ستونی, op, pIdx);
  op &= WO_EQ|WO_IS;
  طالما( p ){
    إذا( (p->prereq_راست & notReady)==0 ){
      إذا( p->prereq_راست==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        أرجع p;
      }
      إذا( p_نتیجه==0 ) p_نتیجه = p;
    }
    p = where_پیمایش_بعدی(&پیمایش);
  }
  أرجع p_نتیجه;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
ساكن صحيح پیدا_کن_شاخص_ستون(
  Parse *pParse,                  /* Parse context */
  عبارت_List *pList,                /* Expression list to search */
  صحيح i_پایه,                      /* Cursor for table associated with pIdx */
  شاخص *pIdx,                    /* Index to match column of */
  صحيح i_ستون                        /* Column of index to match */
){
  صحيح i;
  ثابت حرف *zColl = pIdx->azColl[i_ستون];

  لكل(i=0; i<pList->n_عبارت; i++){
    عبارت *p = sqlite3_عبارت_Skip_Collate_And_Likely(pList->a[i].p_عبارت);
    إذا( ALWAYS(p!=0)
     && (p->op==TK_ستونی || p->op==TK_AGG_ستونی)
     && p->i_ستونی==pIdx->ai_ستونی[i_ستون]
     && p->i_جدول==i_پایه
    ){
      CollSeq *pColl = sqlite3_عبارت_NNColl_Seq(pParse, pList->a[i].p_عبارت);
      إذا( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        أرجع i;
      }
    }
  }

  أرجع -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
ساكن صحيح شاخص_ستونی_Not_Null(شاخص *pIdx, صحيح i_ستون){
  صحيح j;
  أكد( pIdx!=0 );
  أكد( i_ستون>=0 && i_ستون<pIdx->n_ستونی );
  j = pIdx->ai_ستونی[i_ستون];
  إذا( j>=0 ){
    أرجع pIdx->p_جدول->a_ستون[j].notNull;
  }وإلا إذا( j==(-1) ){
    أرجع 1;
  }وإلا{
    أكد( j==(-2) );
    أرجع 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
ساكن صحيح isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  عبارت_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  جدول *pTab;
  شاخص *pIdx;
  صحيح i;                         
  صحيح i_پایه;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  إذا( pTabList->nSrc!=1 ) أرجع 0;
  i_پایه = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  لكل(i=0; i<pDistinct->n_عبارت; i++){
    عبارت *p = sqlite3_عبارت_Skip_Collate_And_Likely(pDistinct->a[i].p_عبارت);
    إذا( NEVER(p==0) ) استمر;
    إذا( p->op!=TK_ستونی && p->op!=TK_AGG_ستونی ) استمر;
    إذا( p->i_جدول==i_پایه && p->i_ستونی<0 ) أرجع 1;
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
  لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بعدی){
    إذا( !Is_Unique_شاخص(pIdx) ) استمر;
    إذا( pIdx->pPartIdxWhere ) استمر;
    لكل(i=0; i<pIdx->n_کلید_ستون; i++){
      إذا( 0==sqlite3_Where_پیدا_کن_جمله(pWC, i_پایه, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        إذا( پیدا_کن_شاخص_ستون(pParse, pDistinct, i_پایه, pIdx, i)<0 ) اكسر;
        إذا( شاخص_ستونی_Not_Null(pIdx, i)==0 ) اكسر;
      }
    }
    إذا( i==pIdx->n_کلید_ستون ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      أرجع 1;
    }
  }

  أرجع 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
ساكن LogEst estLog(LogEst N){
  أرجع N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
ساكن فراغ translate_ستونی_مقصد_Copy(
  Parse *pParse,      /* Parsing context */
  صحيح i_شروع,         /* Translate from this opcode to the end */
  صحيح iTabCur,        /* OP_Column/OP_Rowid references to this table */
  صحيح i_ثبات,      /* The first column is in this register */
  صحيح iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, i_شروع);
  صحيح i_انتها = sqlite3VdbeCurrentAddr(v);
  إذا( pParse->db->mallocFailed ) أرجع;
#اگر_تعریف_شده SQLITE_DEBUG
  إذا( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    چاپ_کن("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, i_شروع, i_انتها);
  }
#پایان_اگر
  لكل(; i_شروع<i_انتها; i_شروع++, pOp++){
    إذا( pOp->p1!=iTabCur ) استمر;
    إذا( pOp->opcode==OP_ستونی ){
#اگر_تعریف_شده SQLITE_DEBUG
      إذا( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        چاپ_کن("TRANSLATE OP_Column to OP_Copy at %d\n", i_شروع);
      }
#پایان_اگر
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + i_ثبات;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }وإلا إذا( pOp->opcode==OP_Rowid ){
#اگر_تعریف_شده SQLITE_DEBUG
      إذا( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        چاپ_کن("TRANSLATE OP_Rowid to OP_Sequence at %d\n", i_شروع);
      }
#پایان_اگر
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#اگر_تعریف_شده SQLITE_ALLOW_ROWID_درون_VIEW
      إذا( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#پایان_اگر
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#إذا !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
ساكن فراغ where_Trace_شاخص_Info_Inputs(
  sqlite3_شاخص_info *p,   /* The IndexInfo object */
  جدول *pTab              /* The TABLE that is the virtual table */
){
  صحيح i;
  إذا( (sqlite3WhereTrace & 0x10)==0 ) أرجع;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  لكل(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].i_ستونی,
       p->aConstraint[i].i_جمله_جابه_جایی,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  لكل(i=0; i<p->n_ترتیب_By; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->a_ترتیب_By[i].i_ستونی,
       p->a_ترتیب_By[i].desc);
  }
}
ساكن فراغ where_Trace_شاخص_Info_Outputs(
  sqlite3_شاخص_info *p,   /* The IndexInfo object */
  جدول *pTab              /* The TABLE that is the virtual table */
){
  صحيح i;
  إذا( (sqlite3WhereTrace & 0x10)==0 ) أرجع;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  لكل(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_شاخص,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_رقم);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->ترتیب_By_Consumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimated_هزینه);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimated_ردیفها);
}
#وإلا
#تعریف_کن where_Trace_شاخص_Info_Inputs(A,B)
#تعریف_کن where_Trace_شاخص_Info_Outputs(A,B)
#پایان_اگر

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
ساكن صحيح constraintCompatibleWithOuterJoin(
  ثابت Where_جمله *p_جمله,       /* WHERE clause term to check */
  ثابت Src_قلم *pSrc           /* Table we are trying to access */
){
  أكد( (pSrc->fg.jointype&(JT_چپ|JT_LTORJ|JT_راست))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))==JT_چپ );
  testcase( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))==JT_LTORJ );
  testcase( عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON) )
  testcase( عبارت_Has_Property(p_جمله->p_عبارت, EP_InnerON) );
  إذا( !عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON|EP_InnerON)
   || p_جمله->p_عبارت->w.iJoin != pSrc->iCursor
  ){
    أرجع 0;
  }
  إذا( (pSrc->fg.jointype & (JT_چپ|JT_راست))!=0
   && NEVER(عبارت_Has_Property(p_جمله->p_عبارت, EP_InnerON))
  ){
    أرجع 0;
  }
  أرجع 1;
}

#اگر_تعریف_نشده SQLITE_OMIT_AUTOMATIC_شاخص
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
ساكن SQLITE_NOINLINE صحيح ستونی_Is_Good_شاخص_Candidate(
  ثابت جدول *pTab,
  صحيح i_ستون
){
  ثابت شاخص *pIdx;
  لكل(pIdx = pTab->p_شاخص; pIdx!=0; pIdx=pIdx->p_بعدی){
    صحيح j;
    لكل(j=0; j<pIdx->n_کلید_ستون; j++){
       إذا( pIdx->ai_ستونی[j]==i_ستون ){
         إذا( j==0 ) أرجع 0;
         إذا( pIdx->hasStat1 && pIdx->ai_ردیف_Log_Est[j+1]>20 ) أرجع 0;
         اكسر;
       }
    }
  }
  أرجع 1;
}
#پایان_اگر /* SQLITE_OMIT_AUTOMATIC_INDEX */



#اگر_تعریف_نشده SQLITE_OMIT_AUTOMATIC_شاخص
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
ساكن صحيح جمله_Can_Drive_شاخص(
  ثابت Where_جمله *p_جمله,        /* WHERE clause term to check */
  ثابت Src_قلم *pSrc,           /* Table we are trying to access */
  ثابت Bitmask notReady         /* Tables in outer loops of the join */
){
  حرف aff;
  صحيح چپ_ستون;
  
  إذا( p_جمله->چپ_Cursor!=pSrc->iCursor ) أرجع 0;
  إذا( (p_جمله->eOperator & (WO_EQ|WO_IS))==0 ) أرجع 0;
  أكد( (pSrc->fg.jointype & JT_راست)==0 );
  إذا( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0
   && !constraintCompatibleWithOuterJoin(p_جمله,pSrc)
  ){
    أرجع 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  إذا( (p_جمله->prereq_راست & notReady)!=0 ) أرجع 0;
  أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
  چپ_ستون = p_جمله->u.x.چپ_ستونی;
  إذا( چپ_ستون<0 ) أرجع 0;
  aff = pSrc->pSTab->a_ستون[چپ_ستون].affinity;
  إذا( !sqlite3_شاخص_Affinity_Ok(p_جمله->p_عبارت, aff) ) أرجع 0;
  testcase( p_جمله->p_عبارت->op==TK_IS );
  أرجع ستونی_Is_Good_شاخص_Candidate(pSrc->pSTab, چپ_ستون);
}
#پایان_اگر


#اگر_تعریف_نشده SQLITE_OMIT_AUTOMATIC_شاخص

#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
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
ساكن فراغ explain_Automatic_شاخص(
  Parse *pParse,
  شاخص *pIdx,                    /* Automatic index to explain */
  صحيح bPartial,                   /* True if pIdx is a partial index */
  صحيح *pAddrExplain               /* OUT: Address of OP_Explain */
){
  إذا( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    جدول *pTab = pIdx->p_جدول;
    ثابت حرف *zSep = "";
    حرف *z_متن = 0;
    صحيح ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    أكد( pIdx->n_ستونی>1 );
    أكد( pIdx->ai_ستونی[pIdx->n_ستونی-1]==XN_ROWID || !HasRowid(pTab) );
    لكل(ii=0; ii<(pIdx->n_ستونی-1); ii++){
      ثابت حرف *zName = 0;
      صحيح i_ستون = pIdx->ai_ستونی[ii];

      zName = pTab->a_ستون[i_ستون].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    z_متن = sqlite3_str_پایان(pStr);
    إذا( z_متن==0 ){
      sqlite3OomFault(pParse->db);
    }وإلا{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", z_متن, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(z_متن);
    }
  }
}
#وإلا
# define explain_Automatic_شاخص(a,b,c,d)
#پایان_اگر

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
ساكن SQLITE_NOINLINE فراغ construct_Automatic_شاخص(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  ثابت Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  صحيح n_کلید_ستون;                /* Number of columns in the constructed index */
  Where_جمله *p_جمله;           /* A single term of the WHERE clause */
  Where_جمله *pWCEnd;          /* End of pWC->a[] */
  شاخص *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  صحيح addrInit;               /* Address of the initialization bypass jump */
  جدول *p_جدول;              /* The table being indexed */
  صحيح addr_بالاترین;                /* Top of the index fill loop */
  صحيح reg_رکورد;              /* Register holding an index record */
  صحيح n;                      /* Column counter */
  صحيح i;                      /* Loop counter */
  صحيح mx_بیت_ستون;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  حرف *z_Not_استفاده_شده;             /* Extra space on the end of pIdx */
  Bitmask idx_تعداد_ستون;            /* Bitmap of columns used for indexing */
  Bitmask extra_تعداد_ستون;          /* Bitmap of additional columns */
  u8 فرستاده_Warning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  عبارت *pPartial = 0;         /* Partial Index Expression */
  صحيح iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  Src_قلم *pSrc;              /* The FROM clause term to get the next index */
  صحيح addrCounter = 0;        /* Address where integer counter is initialized */
  صحيح reg_پایه;                /* Array of registers where record is assembled */
#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExp = 0;            /* Address of OP_Explain */
#پایان_اگر

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  أكد( v!=0 );
  addrInit = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_کلید_ستون = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  p_جدول = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->n_جمله];
  pLoop = pLevel->pWLoop;
  idx_تعداد_ستون = 0;
  لكل(p_جمله=pWC->a; p_جمله<pWCEnd; p_جمله++){
    عبارت *p_عبارت = p_جمله->p_عبارت;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    إذا( (p_جمله->wtFlags & جمله_VIRTUAL)==0
     && sqlite3_عبارت_Is_Single_جدول_Constraint(p_عبارت, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_عبارت_And(pParse, pPartial,
                                sqlite3_عبارت_Dup(pParse->db, p_عبارت, 0));
    }
    إذا( جمله_Can_Drive_شاخص(p_جمله, pSrc, notReady) ){
      صحيح i_ستون;
      Bitmask c_ماسک;
      أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
      i_ستون = p_جمله->u.x.چپ_ستونی;
      c_ماسک = i_ستون>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ستون);
      testcase( i_ستون==BMS );
      testcase( i_ستون==BMS-1 );
      إذا( !فرستاده_Warning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", p_جدول->zName,
            p_جدول->a_ستون[i_ستون].zCnName);
        فرستاده_Warning = 1;
      }
      إذا( (idx_تعداد_ستون & c_ماسک)==0 ){
        إذا( whereLoopResize(pParse->db, pLoop, n_کلید_ستون+1) ){
          اذهب انتها_auto_شاخص_create;
        }
        pLoop->aLTerm[n_کلید_ستون++] = p_جمله;
        idx_تعداد_ستون |= c_ماسک;
      }
    }
  }
  أكد( n_کلید_ستون>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_کلید_ستون;
  pLoop->wsFlags = WHERE_ستونی_EQ | WHERE_IDX_ONLY | WHERE_نمایهای
                     | WHERE_AUTO_شاخص;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  إذا( IsView(p_جدول) ){
    extra_تعداد_ستون = ALLBITS & ~idx_تعداد_ستون;
  }وإلا{
    extra_تعداد_ستون = pSrc->ستون_استفاده_شده & (~idx_تعداد_ستون | MASKBIT(BMS-1));
  }
  إذا( !HasRowid(p_جدول) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    لكل(i=0; i<p_جدول->n_ستون; i++){
      إذا( (p_جدول->a_ستون[i].ستون_Flags & COLFLAG_PRIMKEY)==0 ) استمر;
      إذا( i>=BMS-1 ){
        extra_تعداد_ستون |= MASKBIT(BMS-1);
        اكسر;
      }
      إذا( idx_تعداد_ستون & MASKBIT(i) ) استمر;
      extra_تعداد_ستون |= MASKBIT(i);
    }
  }
  mx_بیت_ستون = MIN(BMS-1,p_جدول->n_ستون);
  testcase( p_جدول->n_ستون==BMS-1 );
  testcase( p_جدول->n_ستون==BMS-2 );
  لكل(i=0; i<mx_بیت_ستون; i++){
    إذا( extra_تعداد_ستون & MASKBIT(i) ) n_کلید_ستون++;
  }
  إذا( pSrc->ستون_استفاده_شده & MASKBIT(BMS-1) ){
    n_کلید_ستون += p_جدول->n_ستون - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  أكد( n_کلید_ستون <= p_جدول->n_ستون + MAX(0, p_جدول->n_ستون - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_شاخص_Object(pParse->db, n_کلید_ستون+HasRowid(p_جدول),
                                    0, &z_Not_استفاده_شده);
  إذا( pIdx==0 ) اذهب انتها_auto_شاخص_create;
  pLoop->u.btree.p_شاخص = pIdx;
  pIdx->zName = "auto-index";
  pIdx->p_جدول = p_جدول;
  n = 0;
  idx_تعداد_ستون = 0;
  لكل(p_جمله=pWC->a; p_جمله<pWCEnd; p_جمله++){
    إذا( جمله_Can_Drive_شاخص(p_جمله, pSrc, notReady) ){
      صحيح i_ستون;
      Bitmask c_ماسک;
      أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
      i_ستون = p_جمله->u.x.چپ_ستونی;
      c_ماسک = i_ستون>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ستون);
      testcase( i_ستون==BMS-1 );
      testcase( i_ستون==BMS );
      إذا( (idx_تعداد_ستون & c_ماسک)==0 ){
        عبارت *pX = p_جمله->p_عبارت;
        idx_تعداد_ستون |= c_ماسک;
        pIdx->ai_ستونی[n] = p_جمله->u.x.چپ_ستونی;
        pColl = sqlite3_عبارت_Compare_Coll_Seq(pParse, pX);
        أكد( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        إذا( !sqlite3_Is_دودویی(pColl) ){
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
  لكل(i=0; i<mx_بیت_ستون; i++){
    إذا( extra_تعداد_ستون & MASKBIT(i) ){
      pIdx->ai_ستونی[n] = i;
      pIdx->azColl[n] = sqlite3_Str_دودویی;
      n++;
    }
  }
  إذا( pSrc->ستون_استفاده_شده & MASKBIT(BMS-1) ){
    لكل(i=BMS-1; i<p_جدول->n_ستون; i++){
      pIdx->ai_ستونی[n] = i;
      pIdx->azColl[n] = sqlite3_Str_دودویی;
      n++;
    }
  }
  أكد( n==n_کلید_ستون );
  إذا( HasRowid(p_جدول) ){
    pIdx->ai_ستونی[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_دودویی;
  }

  /* Create the automatic index */
  explain_Automatic_شاخص(pParse, pIdx, pPartial!=0, &addrExp);
  أكد( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3_Vdbe_جمع_کن_Op2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_کلید_ستون+1);
  sqlite3_Vdbe_مجموعه_P4_کلید_Info(pParse, pIdx);
  VdbeComment((v, "for %s", p_جدول->zName));
  إذا( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  أكد( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  إذا( pSrc->fg.از_طریق_Coroutine ){
    صحيح regYield;
    Subquery *pSubq;
    أكد( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    أكد( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, 0);
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_بالاترین =  sqlite3_Vdbe_جمع_کن_Op1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }وإلا{
    أكد( pLevel->addrHalt );
    addr_بالاترین = sqlite3_Vdbe_جمع_کن_Op2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  إذا( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_عبارت_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  reg_رکورد = sqlite3_Get_موقت_Reg(pParse);
  reg_پایه = sqlite3_Generate_شاخص_کلید(
      pParse, pIdx, pLevel->iTabCur, reg_رکورد, 0, 0, 0, 0
  );
  إذا( pLevel->regFilter ){
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter_جمع_کن, pLevel->regFilter, 0,
                         reg_پایه, pLoop->u.btree.nEq);
  }
  sqlite3_Vdbe_پیمایش_وضعیت_Counters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3_Vdbe_جمع_کن_Op2(v, OP_Idx_درج, pLevel->iIdxCur, reg_رکورد);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  إذا( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  إذا( pSrc->fg.از_طریق_Coroutine ){
    أكد( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, reg_پایه+n);
    testcase( pParse->db->mallocFailed );
    أكد( pLevel->iIdxCur>0 );
    translate_ستونی_مقصد_Copy(pParse, addr_بالاترین, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_نتیجه, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_بالاترین);
    pSrc->fg.از_طریق_Coroutine = 0;
    sqlite3_Vdbe_Jump_اینجا(v, addr_بالاترین);
  }وإلا{
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_بعدی, pLevel->iTabCur, addr_بالاترین+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    إذا( (pSrc->fg.jointype & JT_چپ)!=0 ){
      sqlite3_Vdbe_Jump_اینجا(v, addr_بالاترین);
    }
  }
  sqlite3_Release_موقت_Reg(pParse, reg_رکورد);
 
  /* Jump here when skipping the initialization */
  sqlite3_Vdbe_Jump_اینجا(v, addrInit);
  sqlite3_Vdbe_پیمایش_وضعیت_Range(v, addrExp, addrExp, -1);

انتها_auto_شاخص_create:
  sqlite3_عبارت_Delete(pParse->db, pPartial);
}
#پایان_اگر /* SQLITE_OMIT_AUTOMATIC_INDEX */

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
ساكن SQLITE_NOINLINE فراغ sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  صحيح iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  صحيح addrOnce;                        /* Address of opening OP_Once */
  صحيح addr_بالاترین;                         /* Address of OP_Rewind */
  صحيح addrCont;                        /* Jump here to skip a row */
  ثابت Where_جمله *p_جمله;              /* For looping over WHERE clause terms */
  ثابت Where_جمله *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  صحيح iCur;                            /* Cursor for table getting the filter */
  نمایهای_عبارت *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  نمایهای_عبارت *saved_p_Idx_Part_عبارت;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_عبارت = pParse->p_Idx_Part_عبارت;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_عبارت = 0;

  أكد( pLoop!=0 );
  أكد( v!=0 );
  أكد( pLoop->wsFlags & WHERE_BLOOMFILTER );
  أكد( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
  افعل{
    ثابت SrcList *pTabList;
    ثابت Src_قلم *p_قلم;
    ثابت جدول *pTab;
    u64 sz;
    صحيح iSrc;
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
    p_قلم = &pTabList->a[iSrc];
    أكد( p_قلم!=0 );
    pTab = p_قلم->pSTab;
    أكد( pTab!=0 );
    sz = sqlite3_Log_Est_مقصد_Int(pTab->n_ردیف_Log_Est);
    إذا( sz<10000 ){
      sz = 10000;
    }وإلا إذا( sz>10000000 ){
      sz = 10000000;
    }
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Blob, (صحيح)sz, pLevel->regFilter);

    addr_بالاترین = sqlite3_Vdbe_جمع_کن_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.n_جمله];
    لكل(p_جمله=pWInfo->sWC.a; p_جمله<pWCEnd; p_جمله++){
      عبارت *p_عبارت = p_جمله->p_عبارت;
      إذا( (p_جمله->wtFlags & جمله_VIRTUAL)==0
       && sqlite3_عبارت_Is_Single_جدول_Constraint(p_عبارت, pTabList, iSrc, 0)
      ){
        sqlite3_عبارت_If_False(pParse, p_جمله->p_عبارت, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    إذا( pLoop->wsFlags & WHERE_IPK ){
      صحيح r1 = sqlite3_Get_موقت_Reg(pParse);
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Rowid, iCur, r1);
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter_جمع_کن, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_موقت_Reg(pParse, r1);
    }وإلا{
      شاخص *pIdx = pLoop->u.btree.p_شاخص;
      صحيح n = pLoop->u.btree.nEq;
      صحيح r1 = sqlite3_Get_موقت_Range(pParse, n);
      صحيح jj;
      أكد( pIdx!=0 );
      لكل(jj=0; jj<n; jj++){
        أكد( pIdx->p_جدول==p_قلم->pSTab );
        sqlite3_عبارت_کد_بارگیری_شاخص_ستونی(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter_جمع_کن, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_موقت_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_بعدی, pLevel->iTabCur, addr_بالاترین+1);
    VdbeCoverage(v);
    sqlite3_Vdbe_Jump_اینجا(v, addr_بالاترین);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    طالما( ++iLevel < pWInfo->nLevel ){
      ثابت Src_قلم *p_Tab_قلم;
      pLevel = &pWInfo->a[iLevel];
      p_Tab_قلم = &pWInfo->pTabList->a[pLevel->iFrom];
      إذا( p_Tab_قلم->fg.jointype & (JT_چپ|JT_LTORJ) ) استمر;
      pLoop = pLevel->pWLoop;
      إذا( NEVER(pLoop==0) ) استمر;
      إذا( pLoop->prereq & notReady ) استمر;
      إذا( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_ستونی_درون))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        اكسر;
      }
    }
  }طالما( iLevel < pWInfo->nLevel );
  sqlite3_Vdbe_Jump_اینجا(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_عبارت = saved_p_Idx_Part_عبارت;
}


#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
ساكن Where_جمله *جمله_From_Where_Clause(WhereClause *pWC, صحيح i_جمله){
  WhereClause *p;
  لكل(p=pWC; p; p=p->pOuter){
    إذا( i_جمله<p->n_جمله ) أرجع &p->a[i_جمله];
    i_جمله -= p->n_جمله;
  }
  أرجع 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
ساكن sqlite3_شاخص_info *allocate_شاخص_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  Src_قلم *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  صحيح i, j;
  صحيح n_جمله;
  Parse *pParse = pWInfo->pParse;
  بنية sqlite3_شاخص_constraint *pIdxCons;
  بنية sqlite3_شاخص_orderby *p_Idx_ترتیب_By;
  بنية sqlite3_شاخص_constraint_usage *pUsage;
  بنية Hidden_شاخص_Info *pHidden;
  Where_جمله *p_جمله;
  صحيح n_ترتیب_By;
  sqlite3_شاخص_info *pIdxInfo;
  u16 mNoOmit = 0;
  ثابت جدول *pTab;
  صحيح eDistinct = 0;
  عبارت_List *p_ترتیب_By = pWInfo->p_ترتیب_By;
  WhereClause *p;

  أكد( pSrc!=0 );
  pTab = pSrc->pSTab;
  أكد( pTab!=0 );
  أكد( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  لكل(p=pWC, n_جمله=0; p; p=p->pOuter){
    لكل(i=0, p_جمله=p->a; i<p->n_جمله; i++, p_جمله++){
      p_جمله->wtFlags &= ~جمله_OK;
      إذا( p_جمله->چپ_Cursor != pSrc->iCursor ) استمر;
      إذا( p_جمله->prereq_راست & mUnusable ) استمر;
      أكد( Is_Power_از_Two(p_جمله->eOperator & ~WO_EQUIV) );
      testcase( p_جمله->eOperator & WO_درون );
      testcase( p_جمله->eOperator & WO_ISNULL );
      testcase( p_جمله->eOperator & WO_IS );
      testcase( p_جمله->eOperator & WO_ALL );
      إذا( (p_جمله->eOperator & ~(WO_EQUIV))==0 ) استمر;
      إذا( p_جمله->wtFlags & جمله_VNULL ) استمر;

      أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
      أكد( p_جمله->u.x.چپ_ستونی>=XN_ROWID );
      أكد( p_جمله->u.x.چپ_ستونی<pTab->n_ستون );
      إذا( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0
          && !constraintCompatibleWithOuterJoin(p_جمله,pSrc)
        ){
        استمر;
      }
      n_جمله++;
      p_جمله->wtFlags |= جمله_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  n_ترتیب_By = 0;
  إذا( p_ترتیب_By ){
    صحيح n = p_ترتیب_By->n_عبارت;
    لكل(i=0; i<n; i++){
      عبارت *p_عبارت = p_ترتیب_By->a[i].p_عبارت;
      عبارت *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      إذا( sqlite3_عبارت_Is_Constant(0, p_عبارت) ){
        استمر;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      إذا( p_ترتیب_By->a[i].fg.مرتب_سازی_Flags & KEYINFO_ترتیب_BIGNULL ) اكسر;

      /* First case - a direct column references without a COLLATE operator */
      إذا( p_عبارت->op==TK_ستونی && p_عبارت->i_جدول==pSrc->iCursor ){
        أكد( p_عبارت->i_ستونی>=XN_ROWID && p_عبارت->i_ستونی<pTab->n_ستون );
        استمر;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      إذا( p_عبارت->op==TK_COLLATE
       && (pE2 = p_عبارت->p_چپ)->op==TK_ستونی
       && pE2->i_جدول==pSrc->iCursor
      ){
        ثابت حرف *zColl;  /* The collating sequence name */
        أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
        أكد( p_عبارت->u.zToken!=0 );
        أكد( pE2->i_ستونی>=XN_ROWID && pE2->i_ستونی<pTab->n_ستون );
        p_عبارت->i_ستونی = pE2->i_ستونی;
        إذا( pE2->i_ستونی<0 ) استمر;  /* Collseq does not matter for rowid */
        zColl = sqlite3_ستونی_Coll(&pTab->a_ستون[pE2->i_ستونی]);
        إذا( zColl==0 ) zColl = sqlite3_Str_دودویی;
        إذا( sqlite3_stricmp(p_عبارت->u.zToken, zColl)==0 ) استمر;
      }

      /* No matches cause a break out of the loop */
      اكسر;
    }
    إذا( i==n ){
      صحيح b_مرتب_سازی_By_گروه = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      n_ترتیب_By = n;
      إذا( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowid_استفاده_شده ){
        eDistinct = 2 + b_مرتب_سازی_By_گروه;
      }وإلا إذا( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - b_مرتب_سازی_By_گروه;
      }وإلا إذا( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, حجم(*pIdxInfo)
                           + (حجم(*pIdxCons) + حجم(*pUsage))*n_جمله
                           + حجم(*p_Idx_ترتیب_By)*n_ترتیب_By
                           + SZ_HIDDENINDEXINFO(n_جمله) );
  إذا( pIdxInfo==0 ){
    sqlite3_Error_پیام(pParse, "out of memory");
    أرجع 0;
  }
  pHidden = (بنية Hidden_شاخص_Info*)&pIdxInfo[1];
  pIdxCons = (بنية sqlite3_شاخص_constraint*)&pHidden->aRhs[n_جمله];
  p_Idx_ترتیب_By = (بنية sqlite3_شاخص_orderby*)&pIdxCons[n_جمله];
  pUsage = (بنية sqlite3_شاخص_constraint_usage*)&p_Idx_ترتیب_By[n_ترتیب_By];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->a_ترتیب_By = p_Idx_ترتیب_By;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->ستون_استفاده_شده = (sqlite3_int64)pSrc->ستون_استفاده_شده;
  إذا( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    شاخص *pPk = sqlite3_Primary_کلید_شاخص((جدول*)pTab);
    أكد( pPk!=0 );
    لكل(i=0; i<pPk->n_کلید_ستون; i++){
      صحيح i_ستون = pPk->ai_ستونی[i];
      أكد( i_ستون>=0 );
      إذا( i_ستون>=BMS-1 ) i_ستون = BMS-1;
      pIdxInfo->ستون_استفاده_شده |= MASKBIT(i_ستون);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->m_درون = 0;
  لكل(p=pWC, i=j=0; p; p=p->pOuter){
    صحيح n_آخرین = i+p->n_جمله;;
    لكل(p_جمله=p->a; i<n_آخرین; i++, p_جمله++){
      u16 op;
      إذا( (p_جمله->wtFlags & جمله_OK)==0 ) استمر;
      pIdxCons[j].i_ستونی = p_جمله->u.x.چپ_ستونی;
      pIdxCons[j].i_جمله_جابه_جایی = i;
      op = p_جمله->eOperator & WO_ALL;
      إذا( op==WO_درون ){
        إذا( (p_جمله->wtFlags & جمله_برش)==0 ){
          pHidden->m_درون |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      إذا( op==WO_AUX ){
        pIdxCons[j].op = p_جمله->e_تطابق_Op;
      }وإلا إذا( op & (WO_ISNULL|WO_IS) ){
        إذا( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_شاخص_CONSTRAINT_ISNULL;
        }وإلا{
          pIdxCons[j].op = SQLITE_شاخص_CONSTRAINT_IS;
        }
      }وإلا{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        أكد( WO_EQ==SQLITE_شاخص_CONSTRAINT_EQ );
        أكد( WO_LT==SQLITE_شاخص_CONSTRAINT_LT );
        أكد( WO_LE==SQLITE_شاخص_CONSTRAINT_LE );
        أكد( WO_GT==SQLITE_شاخص_CONSTRAINT_GT );
        أكد( WO_GE==SQLITE_شاخص_CONSTRAINT_GE );
        أكد( p_جمله->eOperator&(WO_درون|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        إذا( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_عبارت_Is_وکتور(p_جمله->p_عبارت->p_راست)
          ){
          testcase( j!=i );
          إذا( j<16 ) mNoOmit |= (1 << j);
          إذا( op==WO_LT ) pIdxCons[j].op = WO_LE;
          إذا( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  أكد( j==n_جمله );
  pIdxInfo->nConstraint = j;
  لكل(i=j=0; i<n_ترتیب_By; i++){
    عبارت *p_عبارت = p_ترتیب_By->a[i].p_عبارت;
    إذا( sqlite3_عبارت_Is_Constant(0, p_عبارت) ) استمر;
    أكد( p_عبارت->op==TK_ستونی
         || (p_عبارت->op==TK_COLLATE && p_عبارت->p_چپ->op==TK_ستونی
              && p_عبارت->i_ستونی==p_عبارت->p_چپ->i_ستونی) );
    p_Idx_ترتیب_By[j].i_ستونی = p_عبارت->i_ستونی;
    p_Idx_ترتیب_By[j].desc = p_ترتیب_By->a[i].fg.مرتب_سازی_Flags & KEYINFO_ترتیب_DESC;
    j++;
  }
  pIdxInfo->n_ترتیب_By = j;

  *pmNoOmit = mNoOmit;
  أرجع pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
ساكن فراغ freeIdxStr(sqlite3_شاخص_info *pIdxInfo){
  إذا( pIdxInfo->need_مقصد_Free_Idx_Str ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->need_مقصد_Free_Idx_Str = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
ساكن فراغ free_شاخص_Info(sqlite3 *db, sqlite3_شاخص_info *pIdxInfo){
  Hidden_شاخص_Info *pHidden;
  صحيح i;
  أكد( pIdxInfo!=0 );
  pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  أكد( pHidden->pParse!=0 );
  أكد( pHidden->pParse->db==db );
  لكل(i=0; i<pIdxInfo->nConstraint; i++){
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
ساكن صحيح vtab_بهترین_شاخص(Parse *pParse, جدول *pTab, sqlite3_شاخص_info *p){
  صحيح rc;
  sqlite3_vtab *pVtab;

  أكد( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_شاخص_Info_Inputs(p, pTab);
  pParse->db->n_Schema_قفل++;
  rc = pVtab->pModule->x_بهترین_شاخص(pVtab, p);
  pParse->db->n_Schema_قفل--;
  where_Trace_شاخص_Info_Outputs(p, pTab);

  إذا( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    إذا( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }وإلا إذا( !pVtab->z_Err_پیام ){
      sqlite3_Error_پیام(pParse, "%s", sqlite3ErrStr(rc));
    }وإلا{
      sqlite3_Error_پیام(pParse, "%s", pVtab->z_Err_پیام);
    }
  }
  إذا( pTab->u.vtab.p->bAllSchemas ){
    sqlite3_Vtab_استفاده_میکند_All_Schemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_پیام);
  pVtab->z_Err_پیام = 0;
  أرجع rc;
}
#پایان_اگر /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#اگر_تعریف_شده SQLITE_ENABLE_STAT4
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
ساكن صحيح where_کلید_Stats(
  Parse *pParse,              /* Database connection */
  شاخص *pIdx,                /* Index to consider domain of */
  Unpacked_رکورد *pRec,       /* Vector of values to consider */
  صحيح دور_به_بالا,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  شاخص_Sample *aSample = pIdx->aSample;
  صحيح i_ستون;                   /* Index of required stats in anEq[] etc. */
  صحيح i;                      /* Index of first sample >= pRec */
  صحيح iSample;                /* Smallest sample larger than or equal to pRec */
  صحيح iMin = 0;               /* Smallest sample not yet tested */
  صحيح iTest;                  /* Next sample to test */
  صحيح res;                    /* Result of comparison operation */
  صحيح n_فیلد;                 /* Number of fields in pRec */
  tRowcnt i_پایینی = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#اگر_تعریف_نشده SQLITE_DEBUG
  استفادهنشده_PARAMETER( pParse );
#پایان_اگر
  أكد( pRec!=0 );
  أكد( pIdx->nSample>0 );
  أكد( pRec->n_فیلد>0 );


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
  إذا( !HasRowid(pIdx->p_جدول) && Is_Primary_کلید_شاخص(pIdx) ){
    n_فیلد = pIdx->n_کلید_ستون;
  }وإلا{
    n_فیلد = pIdx->n_ستونی;
  }
  n_فیلد = MIN(pRec->n_فیلد, n_فیلد);
  i_ستون = 0;
  iSample = pIdx->nSample * n_فیلد;
  افعل{
    صحيح iSamp;                    /* Index in aSample[] of test sample */
    صحيح n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / n_فیلد;
    إذا( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      لكل(n=(iTest % n_فیلد) + 1; n<n_فیلد; n++){
        إذا( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) اكسر;
      }
    }وإلا{
      n = iTest + 1;
    }

    pRec->n_فیلد = n;
    res = sqlite3_Vdbe_رکورد_Compare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    إذا( res<0 ){
      i_پایینی = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }وإلا إذا( res==0 && n<n_فیلد ){
      i_پایینی = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }وإلا{
      iSample = iTest;
      i_ستون = n-1;
    }
  }طالما( res && iMin<iSample );
  i = iSample / n_فیلد;

#اگر_تعریف_شده SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  إذا( pParse->db->mallocFailed==0 ){
    إذا( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      أكد( i<pIdx->nSample );
      أكد( i_ستون==n_فیلد-1 );
      pRec->n_فیلد = n_فیلد;
      أكد( 0==sqlite3_Vdbe_رکورد_Compare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }وإلا{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      أكد( i<=pIdx->nSample && i>=0 );
      pRec->n_فیلد = i_ستون+1;
      أكد( i==pIdx->nSample
           || sqlite3_Vdbe_رکورد_Compare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      إذا( i_ستون>0 ){
        pRec->n_فیلد = i_ستون;
        أكد( sqlite3_Vdbe_رکورد_Compare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      إذا( i>0 ){
        pRec->n_فیلد = n_فیلد;
        أكد( sqlite3_Vdbe_رکورد_Compare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#پایان_اگر /* ifdef SQLITE_DEBUG */

  إذا( res==0 ){
    /* Record pRec is equal to sample i */
    أكد( i_ستون==n_فیلد-1 );
    aStat[0] = aSample[i].anLt[i_ستون];
    aStat[1] = aSample[i].anEq[i_ستون];
  }وإلا{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt i_فوقانی, iGap;
    إذا( i>=pIdx->nSample ){
      i_فوقانی = pIdx->n_ردیف_Est0;
    }وإلا{
      i_فوقانی = aSample[i].anLt[i_ستون];
    }

    إذا( i_پایینی>=i_فوقانی ){
      iGap = 0;
    }وإلا{
      iGap = i_فوقانی - i_پایینی;
    }
    إذا( دور_به_بالا ){
      iGap = (iGap*2)/3;
    }وإلا{
      iGap = iGap/3;
    }
    aStat[0] = i_پایینی + iGap;
    aStat[1] = pIdx->aAvgEq[n_فیلد-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->n_فیلد = n_فیلد;
  أرجع i;
}
#پایان_اگر /* SQLITE_ENABLE_STAT4 */

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
ساكن LogEst whereRangeAdjust(Where_جمله *p_جمله, LogEst nNew){
  LogEst nRet = nNew;
  إذا( p_جمله ){
    إذا( p_جمله->truthProb<=0 ){
      nRet += p_جمله->truthProb;
    }وإلا إذا( (p_جمله->wtFlags & جمله_VNULL)==0 ){
      nRet -= 20;        أكد( 20==sqlite3LogEst(4) );
    }
  }
  أرجع nRet;
}


#اگر_تعریف_شده SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
حرف sqlite3_شاخص_ستونی_Affinity(sqlite3 *db, شاخص *pIdx, صحيح i_ستون){
  أكد( i_ستون>=0 && i_ستون<pIdx->n_ستونی );
  إذا( !pIdx->z_ستون_Aff ){
    إذا( sqlite3_شاخص_Affinity_Str(db, pIdx)==0 ) أرجع SQLITE_AFF_BLOB;
  }
  أكد( pIdx->z_ستون_Aff[i_ستون]!=0 );
  أرجع pIdx->z_ستون_Aff[i_ستون];
}
#پایان_اگر


#اگر_تعریف_شده SQLITE_ENABLE_STAT4
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
ساكن صحيح where_Range_Skip_پیمایش_Est(
  Parse *pParse,       /* Parsing & code generating context */
  Where_جمله *p_پایینی,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_جمله *p_فوقانی,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  صحيح *pb_انجامشده          /* Set to true if at least one expr. value extracted */
){
  شاخص *p = pLoop->u.btree.p_شاخص;
  صحيح nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  صحيح n_پایینی = -1;
  صحيح n_فوقانی = p->nSample+1;
  صحيح rc = SQLITE_OK;
  u8 aff = sqlite3_شاخص_ستونی_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_مقدار *p1 = 0;          /* Value extracted from pLower */
  sqlite3_مقدار *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_مقدار *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  إذا( p_پایینی ){
    rc = sqlite3_Stat4_مقدار_From_عبارت(pParse, p_پایینی->p_عبارت->p_راست, aff, &p1);
    n_پایینی = 0;
  }
  إذا( p_فوقانی && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_مقدار_From_عبارت(pParse, p_فوقانی->p_عبارت->p_راست, aff, &p2);
    n_فوقانی = p2 ? 0 : p->nSample;
  }

  إذا( p1 || p2 ){
    صحيح i;
    صحيح nDiff;
    لكل(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3_Stat4_ستونی(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      إذا( rc==SQLITE_OK && p1 ){
        صحيح res = sqlite3MemCompare(p1, pVal, pColl);
        إذا( res>=0 ) n_پایینی++;
      }
      إذا( rc==SQLITE_OK && p2 ){
        صحيح res = sqlite3MemCompare(p2, pVal, pColl);
        إذا( res>=0 ) n_فوقانی++;
      }
    }
    nDiff = (n_فوقانی - n_پایینی);
    إذا( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    إذا( nDiff!=1 || p_فوقانی==0 || p_پایینی==0 ){
      صحيح nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->n_بیرون -= nAdjust;
      *pb_انجامشده = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           n_پایینی, n_فوقانی, nAdjust*-1, pLoop->n_بیرون));
    }

  }وإلا{
    أكد( *pb_انجامشده==0 );
  }

  sqlite3_مقدار_Free(p1);
  sqlite3_مقدار_Free(p2);
  sqlite3_مقدار_Free(pVal);

  أرجع rc;
}
#پایان_اگر /* SQLITE_ENABLE_STAT4 */

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
ساكن صحيح where_Range_پیمایش_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Where_جمله *p_پایینی,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_جمله *p_فوقانی,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  صحيح rc = SQLITE_OK;
  صحيح n_بیرون = pLoop->n_بیرون;
  LogEst nNew;

#اگر_تعریف_شده SQLITE_ENABLE_STAT4
  شاخص *p = pLoop->u.btree.p_شاخص;
  صحيح nEq = pLoop->u.btree.nEq;

  إذا( p->nSample>0 && ALWAYS(nEq<p->n_Sample_ستون)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    إذا( nEq==pBuilder->nRecValid ){
      Unpacked_رکورد *pRec = pBuilder->pRec;
      tRowcnt a[2];
      صحيح nBtm = pLoop->u.btree.nBtm;
      صحيح n_بالاترین = pLoop->u.btree.n_بالاترین;

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
      tRowcnt i_پایینی;     /* Rows less than the lower bound */
      tRowcnt i_فوقانی;     /* Rows less than the upper bound */
      صحيح iLwrIdx = -2;   /* aSample[] for the lower bound */
      صحيح iUprIdx = -1;   /* aSample[] for the upper bound */

      إذا( pRec ){
        testcase( pRec->n_فیلد!=pBuilder->nRecValid );
        pRec->n_فیلد = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      إذا( nEq==0 ){
        i_پایینی = 0;
        i_فوقانی = p->n_ردیف_Est0;
      }وإلا{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_کلید_Stats(pParse, p, pRec, 0, a);
        i_پایینی = a[0];
        i_فوقانی = a[0] + a[1];
      }

      أكد( p_پایینی==0 || (p_پایینی->eOperator & (WO_GT|WO_GE))!=0 );
      أكد( p_فوقانی==0 || (p_فوقانی->eOperator & (WO_LT|WO_LE))!=0 );
      أكد( p->a_مرتب_سازی_ترتیب!=0 );
      إذا( p->a_مرتب_سازی_ترتیب[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        جابجا(Where_جمله*, p_پایینی, p_فوقانی);
        جابجا(صحيح, nBtm, n_بالاترین);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      إذا( p_پایینی ){
        صحيح n;                    /* Values extracted from pExpr */
        عبارت *p_عبارت = p_پایینی->p_عبارت->p_راست;
        rc = sqlite3_Stat4_Probe_مجموعه_مقدار(pParse, p, &pRec, p_عبارت, nBtm, nEq, &n);
        إذا( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ماسک = WO_GT|WO_LE;
          إذا( sqlite3_عبارت_وکتور_اندازه(p_عبارت)>n ) ماسک = (WO_LE|WO_LT);
          iLwrIdx = where_کلید_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((p_پایینی->eOperator & ماسک) ? a[1] : 0);
          إذا( iNew>i_پایینی ) i_پایینی = iNew;
          n_بیرون--;
          p_پایینی = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      إذا( p_فوقانی ){
        صحيح n;                    /* Values extracted from pExpr */
        عبارت *p_عبارت = p_فوقانی->p_عبارت->p_راست;
        rc = sqlite3_Stat4_Probe_مجموعه_مقدار(pParse, p, &pRec, p_عبارت, n_بالاترین, nEq, &n);
        إذا( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ماسک = WO_GT|WO_LE;
          إذا( sqlite3_عبارت_وکتور_اندازه(p_عبارت)>n ) ماسک = (WO_LE|WO_LT);
          iUprIdx = where_کلید_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((p_فوقانی->eOperator & ماسک) ? a[1] : 0);
          إذا( iNew<i_فوقانی ) i_فوقانی = iNew;
          n_بیرون--;
          p_فوقانی = 0;
        }
      }

      pBuilder->pRec = pRec;
      إذا( rc==SQLITE_OK ){
        إذا( i_فوقانی>i_پایینی ){
          nNew = sqlite3LogEst(i_فوقانی - i_پایینی);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          إذا( iLwrIdx==iUprIdx ){ nNew -= 20; }
          أكد( 20==sqlite3LogEst(4) );
        }وإلا{
          nNew = 10;        أكد( 10==sqlite3LogEst(2) );
        }
        إذا( nNew<n_بیرون ){
          n_بیرون = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)i_پایینی, (u32)i_فوقانی, n_بیرون));
      }
    }وإلا{
      صحيح b_انجامشده = 0;
      rc = where_Range_Skip_پیمایش_Est(pParse, p_پایینی, p_فوقانی, pLoop, &b_انجامشده);
      إذا( b_انجامشده ) أرجع rc;
    }
  }
#وإلا
  استفادهنشده_PARAMETER(pParse);
  استفادهنشده_PARAMETER(pBuilder);
  أكد( p_پایینی || p_فوقانی );
#پایان_اگر
  أكد( p_فوقانی==0 || (p_فوقانی->wtFlags & جمله_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(p_پایینی, n_بیرون);
  nNew = whereRangeAdjust(p_فوقانی, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  إذا( p_پایینی && p_پایینی->truthProb>0 && p_فوقانی && p_فوقانی->truthProb>0 ){
    nNew -= 20;
  }

  n_بیرون -= (p_پایینی!=0) + (p_فوقانی!=0);
  إذا( nNew<10 ) nNew = 10;
  إذا( nNew<n_بیرون ) n_بیرون = nNew;
#إذا defined(WHERETRACE_ENABLED)
  إذا( pLoop->n_بیرون>n_بیرون ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->n_بیرون, n_بیرون));
  }
#پایان_اگر
  pLoop->n_بیرون = (LogEst)n_بیرون;
  أرجع rc;
}

#اگر_تعریف_شده SQLITE_ENABLE_STAT4
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
ساكن صحيح where_Equal_پیمایش_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  عبارت *p_عبارت,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pn_ردیف       /* Write the revised row estimate here */
){
  شاخص *p = pBuilder->pNew->u.btree.p_شاخص;
  صحيح nEq = pBuilder->pNew->u.btree.nEq;
  Unpacked_رکورد *pRec = pBuilder->pRec;
  صحيح rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  صحيح bOk;

  أكد( nEq>=1 );
  أكد( nEq<=p->n_ستونی );
  أكد( p->aSample!=0 );
  أكد( p->nSample>0 );
  أكد( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  إذا( pBuilder->nRecValid<(nEq-1) ){
    أرجع SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  إذا( nEq>=p->n_ستونی ){
    *pn_ردیف = 1;
    أرجع SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_مجموعه_مقدار(pParse, p, &pRec, p_عبارت, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  إذا( rc!=SQLITE_OK ) أرجع rc;
  إذا( bOk==0 ) أرجع SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_کلید_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (صحيح)a[1]));
  *pn_ردیف = a[1];
 
  أرجع rc;
}
#پایان_اگر /* SQLITE_ENABLE_STAT4 */

#اگر_تعریف_شده SQLITE_ENABLE_STAT4
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
ساكن صحيح where_درون_پیمایش_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  عبارت_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pn_ردیف       /* Write the revised row estimate here */
){
  شاخص *p = pBuilder->pNew->u.btree.p_شاخص;
  i64 nRow0 = sqlite3_Log_Est_مقصد_Int(p->ai_ردیف_Log_Est[0]);
  صحيح nRecValid = pBuilder->nRecValid;
  صحيح rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt n_ردیف_Est = 0;    /* New estimate of the number of rows */
  صحيح i;                  /* Loop counter */

  أكد( p->aSample!=0 );
  لكل(i=0; rc==SQLITE_OK && i<pList->n_عبارت; i++){
    nEst = nRow0;
    rc = where_Equal_پیمایش_Est(pParse, pBuilder, pList->a[i].p_عبارت, &nEst);
    n_ردیف_Est += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  إذا( rc==SQLITE_OK ){
    إذا( n_ردیف_Est > (tRowcnt)nRow0 ) n_ردیف_Est = nRow0;
    *pn_ردیف = n_ردیف_Est;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", n_ردیف_Est));
  }
  أكد( pBuilder->nRecValid==nRecValid );
  أرجع rc;
}
#پایان_اگر /* SQLITE_ENABLE_STAT4 */


#إذا defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
فراغ sqlite3_Where_جمله_Print(Where_جمله *p_جمله, صحيح i_جمله){
  إذا( p_جمله==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", i_جمله);
  }وإلا{
    حرف zType[8];
    حرف z_چپ[50];
    کپی_حافظه(zType, "....", 5);
    إذا( p_جمله->wtFlags & جمله_VIRTUAL ) zType[0] = 'V';
    إذا( p_جمله->eOperator & WO_EQUIV  ) zType[1] = 'E';
    إذا( عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON) ) zType[2] = 'L';
    إذا( p_جمله->wtFlags & جمله_CODED  ) zType[3] = 'C';
    إذا( p_جمله->eOperator & WO_SINGLE ){
      أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(حجم(z_چپ),z_چپ,"left={%d:%d}",
                       p_جمله->چپ_Cursor, p_جمله->u.x.چپ_ستونی);
    }وإلا إذا( (p_جمله->eOperator & WO_OR)!=0 && p_جمله->u.pOrInfo!=0 ){
      sqlite3_snprintf(حجم(z_چپ),z_چپ,"indexable=0x%llx",
                       p_جمله->u.pOrInfo->indexable);
    }وإلا{
      sqlite3_snprintf(حجم(z_چپ),z_چپ,"left=%d", p_جمله->چپ_Cursor);
    }
    i_جمله = p_جمله->i_جمله = MAX(i_جمله,p_جمله->i_جمله);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       i_جمله, p_جمله, zType, z_چپ, p_جمله->eOperator, p_جمله->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    إذا( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        p_جمله->truthProb, (u64)p_جمله->prereqAll, (u64)p_جمله->prereq_راست);
    }
    إذا( (p_جمله->eOperator & (WO_OR|WO_AND))==0 && p_جمله->u.x.i_فیلد ){
      sqlite3DebugPrintf(" iField=%d", p_جمله->u.x.i_فیلد);
    }
    إذا( p_جمله->i_والد>=0 ){
      sqlite3DebugPrintf(" iParent=%d", p_جمله->i_والد);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_درخت_View_عبارت(0, p_جمله->p_عبارت, 0);
  }
}
فراغ sqlite3_Show_Where_جمله(Where_جمله *p_جمله){
  sqlite3_Where_جمله_Print(p_جمله, 0);
}
#پایان_اگر

#اگر_تعریف_شده WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
فراغ sqlite3WhereClausePrint(WhereClause *pWC){
  صحيح i;
  لكل(i=0; i<pWC->n_جمله; i++){
    sqlite3_Where_جمله_Print(&pWC->a[i], i);
  }
}
#پایان_اگر

#اگر_تعریف_شده WHERETRACE_ENABLED
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
فراغ sqlite3WhereLoopPrint(ثابت WhereLoop *p, ثابت WhereClause *pWC){
  WhereInfo *pWInfo;
  إذا( pWC ){
    صحيح nb;
    Src_قلم *p_قلم;
    جدول *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    p_قلم = pWInfo->pTabList->a + p->iTab;
    pTab = p_قلم->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->ماسک_Self, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       p_قلم->zAlias ? p_قلم->zAlias : pTab->zName);
  }وإلا{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->ماسک_Self, p->prereq & 0xfff, p->cId, p->iTab);
  }
  إذا( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ثابت حرف *zName;
    إذا( p->u.btree.p_شاخص && (zName = p->u.btree.p_شاخص->zName)!=0 ){
      إذا( مقایسه_رشته(zName, "sqlite_autoindex_", 17)==0 ){
        صحيح i = sqlite3Strlen30(zName) - 1;
        طالما( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }وإلا{
      sqlite3DebugPrintf("%20s","");
    }
  }وإلا{
    حرف *z;
    إذا( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_رقم, p->u.vtab.idxStr, p->u.vtab.omit_ماسک);
    }وإلا{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_رقم, p->u.vtab.omit_ماسک);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  إذا( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }وإلا{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  إذا( pWInfo && pWInfo->b_ستاره_استفاده_شده && p->r_ستاره_Delta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->r_راهاندازی, p->r_پیاپی, p->n_بیرون, p->r_ستاره_Delta);
  }وإلا{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->r_راهاندازی, p->r_پیاپی, p->n_بیرون);
  }
  إذا( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    صحيح i;
    لكل(i=0; i<p->nLTerm; i++){
      sqlite3_Where_جمله_Print(p->aLTerm[i], i);
    }
  }
}
فراغ sqlite3ShowWhereLoop(ثابت WhereLoop *p){
  إذا( p ) sqlite3WhereLoopPrint(p, 0);
}
فراغ sqlite3ShowWhereLoopList(ثابت WhereLoop *p){
  طالما( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_بعدی_Loop;
  }
}
#پایان_اگر

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
ساكن فراغ whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_اندازه(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
ساكن فراغ whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  إذا( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_شاخص) ){
    إذا( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }وإلا إذا( (p->wsFlags & WHERE_AUTO_شاخص)!=0 && p->u.btree.p_شاخص!=0 ){
      sqlite3DbFree(db, p->u.btree.p_شاخص->z_ستون_Aff);
      sqlite3DbFreeNN(db, p->u.btree.p_شاخص);
      p->u.btree.p_شاخص = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
ساكن فراغ whereLoopClear(sqlite3 *db, WhereLoop *p){
  إذا( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_اندازه(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
ساكن صحيح whereLoopResize(sqlite3 *db, WhereLoop *p, صحيح n){
  Where_جمله **paNew;
  إذا( p->nLSlot>=n ) أرجع SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, حجم(p->aLTerm[0])*n);
  إذا( paNew==0 ) أرجع SQLITE_NOMEM_BKPT;
  کپی_حافظه(paNew, p->aLTerm, حجم(p->aLTerm[0])*p->nLSlot);
  إذا( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  أرجع SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
ساكن صحيح whereLoopXfer(sqlite3 *db, WhereLoop *p_مقصد, WhereLoop *pFrom){
  whereLoopClearUnion(db, p_مقصد);
  إذا( pFrom->nLTerm > p_مقصد->nLSlot
   && whereLoopResize(db, p_مقصد, pFrom->nLTerm)
  ){
    memset(p_مقصد, 0, WHERE_LOOP_XFER_SZ);
    أرجع SQLITE_NOMEM_BKPT;
  }
  کپی_حافظه(p_مقصد, pFrom, WHERE_LOOP_XFER_SZ);
  کپی_حافظه(p_مقصد->aLTerm, pFrom->aLTerm, p_مقصد->nLTerm*حجم(p_مقصد->aLTerm[0]));
  إذا( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }وإلا إذا( (pFrom->wsFlags & WHERE_AUTO_شاخص)!=0 ){
    pFrom->u.btree.p_شاخص = 0;
  }
  أرجع SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
ساكن فراغ whereLoopDelete(sqlite3 *db, WhereLoop *p){
  أكد( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
ساكن فراغ whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  أكد( pWInfo!=0 );
  أكد( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  طالما( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_بعدی_Loop;
    whereLoopDelete(db, p);
  }
  طالما( pWInfo->p_Mem_مقصد_Free ){
    Where_Mem_بلوک *p_بعدی = pWInfo->p_Mem_مقصد_Free->p_بعدی;
    sqlite3DbNNFreeNN(db, pWInfo->p_Mem_مقصد_Free);
    pWInfo->p_Mem_مقصد_Free = p_بعدی;
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
ساكن صحيح whereLoopCheaperProperSubset(
  ثابت WhereLoop *pX,       /* First WhereLoop to compare */
  ثابت WhereLoop *pY        /* Compare against this WhereLoop */
){
  صحيح i, j;
  إذا( pX->r_پیاپی>pY->r_پیاپی && pX->n_بیرون>pY->n_بیرون ) أرجع 0; /* (1d) and (2a) */
  أكد( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  إذا( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_شاخص==pY->u.btree.p_شاخص             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    أرجع 1;  /* Case 1 is true */
  }
  إذا( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    أرجع 0;                                            /* (2b) */
  }
  إذا( pY->nSkip > pX->nSkip ) أرجع 0;                  /* (2d) */
  لكل(i=pX->nLTerm-1; i>=0; i--){
    إذا( pX->aLTerm[i]==0 ) استمر;
    لكل(j=pY->nLTerm-1; j>=0; j--){
      إذا( pY->aLTerm[j]==pX->aLTerm[i] ) اكسر;
    }
    إذا( j<0 ) أرجع 0;                                  /* (2c) */
  }
  إذا( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    أرجع 0;                                            /* (2e) */
  }
  أرجع 1;  /* Case 2 is true */
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
ساكن فراغ where_Loop_Adjust_هزینه(ثابت WhereLoop *p, WhereLoop *pTemplate){
  إذا( (pTemplate->wsFlags & WHERE_نمایهای)==0 ) أرجع;
  لكل(; p; p=p->p_بعدی_Loop){
    إذا( p->iTab!=pTemplate->iTab ) استمر;
    إذا( (p->wsFlags & WHERE_نمایهای)==0 ) استمر;
    إذا( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_پیاپی, pTemplate->n_بیرون,
                       MIN(p->r_پیاپی, pTemplate->r_پیاپی),
                       MIN(p->n_بیرون - 1, pTemplate->n_بیرون)));
      pTemplate->r_پیاپی = MIN(p->r_پیاپی, pTemplate->r_پیاپی);
      pTemplate->n_بیرون = MIN(p->n_بیرون - 1, pTemplate->n_بیرون);
    }وإلا إذا( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_پیاپی, pTemplate->n_بیرون,
                       MAX(p->r_پیاپی, pTemplate->r_پیاپی),
                       MAX(p->n_بیرون + 1, pTemplate->n_بیرون)));
      pTemplate->r_پیاپی = MAX(p->r_پیاپی, pTemplate->r_پیاپی);
      pTemplate->n_بیرون = MAX(p->n_بیرون + 1, pTemplate->n_بیرون);
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
ساكن WhereLoop **where_Loop_پیدا_کن_Lesser(
  WhereLoop **ppPrev,
  ثابت WhereLoop *pTemplate
){
  WhereLoop *p;
  لكل(p=(*ppPrev); p; ppPrev=&p->p_بعدی_Loop, p=*ppPrev){
    إذا( p->iTab!=pTemplate->iTab || p->i_مرتب_سازی_Idx!=pTemplate->i_مرتب_سازی_Idx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      استمر;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    أكد( p->r_راهاندازی==0 || pTemplate->r_راهاندازی==0
                 || p->r_راهاندازی==pTemplate->r_راهاندازی );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    أكد( p->r_راهاندازی>=pTemplate->r_راهاندازی );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    إذا( (p->wsFlags & WHERE_AUTO_شاخص)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_نمایهای)!=0
     && (pTemplate->wsFlags & WHERE_ستونی_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      اكسر;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    إذا( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->r_راهاندازی<=pTemplate->r_راهاندازی                  /* (2a) */
     && p->r_پیاپی<=pTemplate->r_پیاپی                      /* (2b) */
     && p->n_بیرون<=pTemplate->n_بیرون                      /* (2c) */
    ){
      أرجع 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    إذا( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->r_پیاپی>=pTemplate->r_پیاپی                             /* (2a) */
     && p->n_بیرون>=pTemplate->n_بیرون                             /* (2b) */
    ){
      أكد( p->r_راهاندازی>=pTemplate->r_راهاندازی ); /* SETUP-INVARIANT above */
      اكسر;   /* Cause p to be overwritten by pTemplate */
    }
  }
  أرجع ppPrev;
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
ساكن صحيح where_Loop_درج(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  صحيح rc;

  /* Stop the search once we hit the query planner search limit */
  إذا( pBuilder->i_Plan_حد==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    إذا( pBuilder->p_Or_مجموعه ) pBuilder->p_Or_مجموعه->n = 0;
    أرجع SQLITE_انجامشده;
  }
  pBuilder->i_Plan_حد--;

  where_Loop_Adjust_هزینه(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  إذا( pBuilder->p_Or_مجموعه!=0 ){
    إذا( pTemplate->nLTerm ){
#إذا WHERETRACE_ENABLED
      u16 n = pBuilder->p_Or_مجموعه->n;
      صحيح x =
#پایان_اگر
      where_Or_درج(pBuilder->p_Or_مجموعه, pTemplate->prereq, pTemplate->r_پیاپی,
                                    pTemplate->n_بیرون);
#إذا WHERETRACE_ENABLED /* 0x8 */
      إذا( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#پایان_اگر
    }
    أرجع SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = where_Loop_پیدا_کن_Lesser(&pWInfo->pLoops, pTemplate);

  إذا( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#إذا WHERETRACE_ENABLED /* 0x8 */
    إذا( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#پایان_اگر
    أرجع SQLITE_OK; 
  }وإلا{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#إذا WHERETRACE_ENABLED /* 0x8 */
  إذا( sqlite3WhereTrace & 0x8 ){
    إذا( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }وإلا{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#پایان_اگر
  إذا( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, حجم(WhereLoop));
    إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_بعدی_Loop = 0;
  }وإلا{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_دم = &p->p_بعدی_Loop;
    WhereLoop *p_مقصد_Del;
    طالما( *pp_دم ){
      pp_دم = where_Loop_پیدا_کن_Lesser(pp_دم, pTemplate);
      إذا( pp_دم==0 ) اكسر;
      p_مقصد_Del = *pp_دم;
      إذا( p_مقصد_Del==0 ) اكسر;
      *pp_دم = p_مقصد_Del->p_بعدی_Loop;
#إذا WHERETRACE_ENABLED /* 0x8 */
      إذا( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(p_مقصد_Del, pBuilder->pWC);
      }
#پایان_اگر
      whereLoopDelete(db, p_مقصد_Del);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  إذا( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    شاخص *p_شاخص = p->u.btree.p_شاخص;
    إذا( p_شاخص && p_شاخص->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_شاخص = 0;
    }
  }
  أرجع rc;
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
ساكن صحيح عبارت_گره_الگو_درازی_Est(Walker *pWalker, عبارت *p_عبارت){
  إذا( p_عبارت->op==TK_STRING ){
    صحيح sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_عبارت->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    إذا( pWalker->e_کد ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }وإلا{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    طالما( (c = *(z++))!=0 ){
      إذا( c==c3 ){
        إذا( *z ) z++;
        طالما( *z && *z!=']' ) z++;
      }وإلا إذا( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    إذا( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  أرجع WRC_Continue;
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
ساكن صحيح est_Like_الگو_درازی(عبارت *p, u16 e_کد){
  Walker w;
  w.u.sz = 0;
  w.e_کد = e_کد;
  w.x_عبارت_Callback = عبارت_گره_الگو_درازی_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شده SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#پایان_اگر
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.u.sz;
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
ساكن فراغ whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst n_ردیف            /* Number of rows in the entire table */
){
  Where_جمله *p_جمله, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->ماسک_Self);
  صحيح i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  إذا( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->p_ترتیب_By==0 ){
    أرجع;
  }

  أكد( (pLoop->wsFlags & WHERE_AUTO_شاخص)==0 );
  لكل(i=pWC->n_پایه, p_جمله=pWC->a; i>0; i--, p_جمله++){
    أكد( p_جمله!=0 );
    إذا( (p_جمله->prereqAll & notAllowed)!=0 ) استمر;
    إذا( (p_جمله->prereqAll & pLoop->ماسک_Self)==0 ) استمر;
    إذا( (p_جمله->wtFlags & جمله_VIRTUAL)!=0 ) استمر;
    لكل(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      إذا( pX==0 ) استمر;
      إذا( pX==p_جمله ) اكسر;
      إذا( pX->i_والد>=0 && (&pWC->a[pX->i_والد])==p_جمله ) اكسر;
    }
    إذا( j<0 ){
      sqlite3_پیشرفت_Check(pWC->pWInfo->pParse);
      إذا( pLoop->ماسک_Self==p_جمله->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        إذا( (p_جمله->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_چپ|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      إذا( p_جمله->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->n_بیرون += p_جمله->truthProb;
      }وإلا{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        عبارت *p_Op_عبارت = p_جمله->p_عبارت;
        pLoop->n_بیرون--;
        إذا( (p_جمله->eOperator&(WO_EQ|WO_IS))!=0
         && (p_جمله->wtFlags & جمله_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          عبارت *p_راست = p_Op_عبارت->p_راست;
          Parse *pParse = pWC->pWInfo->pParse;
          صحيح k = 0;
          testcase( p_Op_عبارت->op==TK_IS );
          إذا( sqlite3_عبارت_Is_Integer(p_راست, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }وإلا{
            k = 20;
          }
          إذا( iReduce<k ){
            p_جمله->wtFlags |= جمله_HEURTRUTH;
            iReduce = k;
          }
        }وإلا
        إذا( عبارت_Has_Property(p_Op_عبارت, EP_InfixFunc) 
         && p_Op_عبارت->op==TK_FUNCTION
        ){
          صحيح eOp;
          أكد( عبارت_Use_XList(p_Op_عبارت) );
          أكد( p_Op_عبارت->x.pList->n_عبارت>=2 );
          eOp = sqlite3_عبارت_Is_Like_Operator(p_Op_عبارت);
          إذا( ALWAYS(eOp>0) ){
            صحيح sz_الگو;
            عبارت *pRHS = p_Op_عبارت->x.pList->a[0].p_عبارت;
            eOp = eOp==SQLITE_شاخص_CONSTRAINT_LIKE;
            sz_الگو = est_Like_الگو_درازی(pRHS, eOp);
            إذا( sz_الگو>0 ){
              pLoop->n_بیرون -= sz_الگو*2;
            }
          }
        }
      }
    }
  }
  إذا( pLoop->n_بیرون > n_ردیف-iReduce ){
    pLoop->n_بیرون = n_ردیف - iReduce;
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
ساكن صحيح where_Range_وکتور_Len(
  Parse *pParse,       /* Parsing context */
  صحيح iCur,            /* Cursor open on pIdx */
  شاخص *pIdx,         /* The index to be used for a inequality constraint */
  صحيح nEq,             /* Number of prior equality constraints on same index */
  Where_جمله *p_جمله     /* The vector inequality constraint */
){
  صحيح nCmp = sqlite3_عبارت_وکتور_اندازه(p_جمله->p_عبارت->p_چپ);
  صحيح i;

  nCmp = MIN(nCmp, (pIdx->n_ستونی - nEq));
  لكل(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    حرف aff;                     /* Comparison affinity */
    حرف idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    عبارت *pLhs, *pRhs;

    أكد( عبارت_Use_XList(p_جمله->p_عبارت->p_چپ) );
    pLhs = p_جمله->p_عبارت->p_چپ->x.pList->a[i].p_عبارت;
    pRhs = p_جمله->p_عبارت->p_راست;
    إذا( عبارت_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_عبارت;
    }وإلا{
      pRhs = pRhs->x.pList->a[i].p_عبارت;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    إذا( pLhs->op!=TK_ستونی
     || pLhs->i_جدول!=iCur
     || pLhs->i_ستونی!=pIdx->ai_ستونی[i+nEq]
     || pIdx->a_مرتب_سازی_ترتیب[i+nEq]!=pIdx->a_مرتب_سازی_ترتیب[nEq]
    ){
      اكسر;
    }

    testcase( pLhs->i_ستونی==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_عبارت_Affinity(pLhs));
    idxaff = sqlite3_جدول_ستونی_Affinity(pIdx->p_جدول, pLhs->i_ستونی);
    إذا( aff!=idxaff ) اكسر;

    إذا( عبارت_Has_Property(p_جمله->p_عبارت, EP_Commuted) ) جابجا(عبارت*, pRhs, pLhs);
    pColl = sqlite3_دودویی_Compare_Coll_Seq(pParse, pLhs, pRhs);
    إذا( pColl==0 ) اكسر;
    إذا( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) اكسر;
  }
  أرجع i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#اگر_تعریف_شده SQLITE_ENABLE_COSTMULT
# define Apply_هزینه_Multiplier(C,T)  C += T
#وإلا
# define Apply_هزینه_Multiplier(C,T)
#پایان_اگر

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
ساكن صحيح where_Loop_جمع_کن_Btree_شاخص(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  Src_قلم *pSrc,                  /* FROM clause term being analyzed */
  شاخص *pProbe,                  /* An index on pSrc */
  LogEst n_درون_Mul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  Where_جمله *p_جمله;               /* A WhereTerm under consideration */
  صحيح op_ماسک;                     /* Valid operators for constraints */
  Where_پیمایش پیمایش;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_بالاترین;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_n_بیرون;              /* Original value of pNew->nOut */
  صحيح rc = SQLITE_OK;             /* Return code */
  LogEst r_اندازه;                   /* Number of rows in the table */
  LogEst r_Log_اندازه;                /* Logarithm of table size */
  Where_جمله *p_بالاترین = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  أكد( db->mallocFailed==0 || pParse->nErr>0 );
  إذا( pParse->nErr ){
    أرجع pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->p_جدول->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->r_پیاپی));

  أكد( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pNew->wsFlags & WHERE_بالاترین_حد)==0 );
  إذا( pNew->wsFlags & WHERE_BTM_حد ){
    op_ماسک = WO_LT|WO_LE;
  }وإلا{
    أكد( pNew->u.btree.nBtm==0 );
    op_ماسک = WO_EQ|WO_درون|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  إذا( pProbe->bUnordered ){
    op_ماسک &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  أكد( pNew->u.btree.nEq<pProbe->n_ستونی );
  أكد( pNew->u.btree.nEq<pProbe->n_کلید_ستون
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_بالاترین = pNew->u.btree.n_بالاترین;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_n_بیرون = pNew->n_بیرون;
  p_جمله = where_پیمایش_Init(&پیمایش, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        op_ماسک, pProbe);
  pNew->r_راهاندازی = 0;
  r_اندازه = pProbe->ai_ردیف_Log_Est[0];
  r_Log_اندازه = estLog(r_اندازه);
  لكل(; rc==SQLITE_OK && p_جمله!=0; p_جمله = where_پیمایش_بعدی(&پیمایش)){
    u16 eOp = p_جمله->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst r_هزینه_Idx;
    LogEst n_بیرون_Unadjusted;        /* nOut before IN() and WHERE adjustments */
    صحيح n_درون = 0;
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
    صحيح nRecValid = pBuilder->nRecValid;
#پایان_اگر
    إذا( (eOp==WO_ISNULL || (p_جمله->wtFlags&جمله_VNULL)!=0)
     && شاخص_ستونی_Not_Null(pProbe, saved_nEq)
    ){
      استمر; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    إذا( p_جمله->prereq_راست & pNew->ماسک_Self ) استمر;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    إذا( p_جمله->wtFlags & جمله_LIKEOPT && p_جمله->eOperator==WO_LT ) استمر;

    إذا( (pSrc->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0
     && !constraintCompatibleWithOuterJoin(p_جمله,pSrc)
    ){
      استمر;
    }
    إذا( Is_Unique_شاخص(pProbe) && saved_nEq==pProbe->n_کلید_ستون-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }وإلا{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_نمایهای;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_بالاترین = saved_n_بالاترین;
    pNew->nLTerm = saved_nLTerm;
    إذا( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       اكسر; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = p_جمله;
    pNew->prereq = (saved_prereq | p_جمله->prereq_راست) & ~pNew->ماسک_Self;

    أكد( n_درون_Mul==0
        || (pNew->wsFlags & WHERE_ستونی_NULL)!=0
        || (pNew->wsFlags & WHERE_ستونی_درون)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    إذا( eOp & WO_درون ){
      عبارت *p_عبارت = p_جمله->p_عبارت;
      إذا( عبارت_Use_XSelect(p_عبارت) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        صحيح i;
        صحيح bRedundant = 0;
        n_درون = 46;  أكد( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        لكل(i=0; i<pNew->nLTerm-1; i++){
          إذا( pNew->aLTerm[i] && pNew->aLTerm[i]->p_عبارت==p_عبارت ){
            n_درون = 0;
            إذا( pNew->aLTerm[i]->u.x.i_فیلد == p_جمله->u.x.i_فیلد ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        إذا( bRedundant ){
          pNew->nLTerm--;
          استمر;
        }
      }وإلا إذا( ALWAYS(p_عبارت->x.pList && p_عبارت->x.pList->n_عبارت) ){
        /* "x IN (value, value, ...)" */
        n_درون = sqlite3LogEst(p_عبارت->x.pList->n_عبارت);
      }
      إذا( pProbe->hasStat1 && r_Log_اندازه>=10 ){
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
        M = pProbe->ai_ردیف_Log_Est[saved_nEq];
        logK = estLog(n_درون);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (n_درون + r_Log_اندازه);
        إذا( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, n_درون, r_Log_اندازه, x));
        }وإلا إذا( n_درون_Mul<2 && OptimizationEnabled(db, SQLITE_Seek_پیمایش) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, n_درون, r_Log_اندازه, x, n_درون_Mul));
          pNew->wsFlags |= WHERE_درون_SEEKSCAN;
        }وإلا{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, n_درون, r_Log_اندازه, x, n_درون_Mul));
          استمر;
        }
      }
      pNew->wsFlags |= WHERE_ستونی_درون;
    }وإلا إذا( eOp & (WO_EQ|WO_IS) ){
      صحيح i_ستون = pProbe->ai_ستونی[saved_nEq];
      pNew->wsFlags |= WHERE_ستونی_EQ;
      أكد( saved_nEq==pNew->u.btree.nEq );
      إذا( i_ستون==XN_ROWID
       || (i_ستون>=0 && n_درون_Mul==0 && saved_nEq==pProbe->n_کلید_ستون-1)
      ){
        إذا( i_ستون==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_کلید_ستون==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }وإلا{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      إذا( پیمایش.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }وإلا إذا( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_ستونی_NULL;
    }وإلا{
      صحيح nVecLen = where_Range_وکتور_Len(
          pParse, pSrc->iCursor, pProbe, saved_nEq, p_جمله
      );
      إذا( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_ستونی_RANGE|WHERE_BTM_حد;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = p_جمله;
        p_بالاترین = 0;
        إذا( p_جمله->wtFlags & جمله_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_بالاترین = &p_جمله[1];
          أكد( (p_بالاترین-(p_جمله->pWC->a))<p_جمله->pWC->n_جمله );
          أكد( p_بالاترین->wtFlags & جمله_LIKEOPT );
          أكد( p_بالاترین->eOperator==WO_LT );
          إذا( whereLoopResize(db, pNew, pNew->nLTerm+1) ) اكسر; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_بالاترین;
          pNew->wsFlags |= WHERE_بالاترین_حد;
          pNew->u.btree.n_بالاترین = 1;
        }
      }وإلا{
        أكد( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_ستونی_RANGE|WHERE_بالاترین_حد;
        pNew->u.btree.n_بالاترین = nVecLen;
        p_بالاترین = p_جمله;
        pBtm = (pNew->wsFlags & WHERE_BTM_حد)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    أكد( pNew->n_بیرون==saved_n_بیرون );
    إذا( pNew->wsFlags & WHERE_ستونی_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      where_Range_پیمایش_Est(pParse, pBuilder, pBtm, p_بالاترین, pNew);
    }وإلا{
      صحيح nEq = ++pNew->u.btree.nEq;
      أكد( eOp & (WO_ISNULL|WO_EQ|WO_درون|WO_IS) );

      أكد( pNew->n_بیرون==saved_n_بیرون );
      إذا( p_جمله->truthProb<=0 && pProbe->ai_ستونی[saved_nEq]>=0 ){
        أكد( (eOp & WO_درون) || n_درون==0 );
        testcase( eOp & WO_درون );
        pNew->n_بیرون += p_جمله->truthProb;
        pNew->n_بیرون -= n_درون;
      }وإلا{
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
        tRowcnt n_بیرون = 0;
        إذا( n_درون_Mul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->n_Sample_ستون)
         && ((eOp & WO_درون)==0 || عبارت_Use_XList(p_جمله->p_عبارت))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          عبارت *p_عبارت = p_جمله->p_عبارت;
          إذا( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = where_Equal_پیمایش_Est(pParse, pBuilder, p_عبارت->p_راست, &n_بیرون);
          }وإلا{
            rc = where_درون_پیمایش_Est(pParse, pBuilder, p_عبارت->x.pList, &n_بیرون);
          }
          إذا( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          إذا( rc!=SQLITE_OK ) اكسر;          /* Jump out of the pTerm loop */
          إذا( n_بیرون ){
            pNew->n_بیرون = sqlite3LogEst(n_بیرون);
            إذا( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->n_بیرون+10 > pProbe->ai_ردیف_Log_Est[0]
            ){
#إذا WHERETRACE_ENABLED /* 0x01 */
              إذا( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3_Where_جمله_Print(p_جمله, 999);
              }
#پایان_اگر
              p_جمله->wtFlags |= جمله_HIGHTRUTH;
              إذا( p_جمله->wtFlags & جمله_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            إذا( pNew->n_بیرون>saved_n_بیرون ) pNew->n_بیرون = saved_n_بیرون;
            pNew->n_بیرون -= n_درون;
          }
        }
        إذا( n_بیرون==0 )
#پایان_اگر
        {
          pNew->n_بیرون += (pProbe->ai_ردیف_Log_Est[nEq] - pProbe->ai_ردیف_Log_Est[nEq-1]);
          إذا( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->n_بیرون += 10;
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
    أكد( pSrc->pSTab->sz_Tab_ردیف>0 );
    إذا( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      r_هزینه_Idx = pNew->n_بیرون + 16;
    }وإلا{
      r_هزینه_Idx = pNew->n_بیرون + 1 + (15*pProbe->sz_Idx_ردیف)/pSrc->pSTab->sz_Tab_ردیف;
    }
    r_هزینه_Idx = sqlite3_Log_Est_جمع_کن(r_Log_اندازه, r_هزینه_Idx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->r_پیاپی = r_هزینه_Idx;
    إذا( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->r_پیاپی = sqlite3_Log_Est_جمع_کن(pNew->r_پیاپی, pNew->n_بیرون + 16);
    }
    Apply_هزینه_Multiplier(pNew->r_پیاپی, pProbe->p_جدول->هزینه_Mult);

    n_بیرون_Unadjusted = pNew->n_بیرون;
    pNew->r_پیاپی += n_درون_Mul + n_درون;
    pNew->n_بیرون += n_درون_Mul + n_درون;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_اندازه);
    إذا( pSrc->fg.fromExists ) pNew->n_بیرون = 0;
    rc = where_Loop_درج(pBuilder, pNew);

    إذا( pNew->wsFlags & WHERE_ستونی_RANGE ){
      pNew->n_بیرون = saved_n_بیرون;
    }وإلا{
      pNew->n_بیرون = n_بیرون_Unadjusted;
    }

    إذا( (pNew->wsFlags & WHERE_بالاترین_حد)==0
     && pNew->u.btree.nEq<pProbe->n_ستونی
     && (pNew->u.btree.nEq<pProbe->n_کلید_ستون ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      إذا( pNew->u.btree.nEq>3 ){
        sqlite3_پیشرفت_Check(pParse);
      }
      where_Loop_جمع_کن_Btree_شاخص(pBuilder, pSrc, pProbe, n_درون_Mul+n_درون);
    }
    pNew->n_بیرون = saved_n_بیرون;
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#پایان_اگر
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_بالاترین = saved_n_بالاترین;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->n_بیرون = saved_n_بیرون;
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
  إذا( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_کلید_ستون
   && saved_nEq==pNew->nLTerm
   && pProbe->no_Skip_پیمایش==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_Skip_پیمایش)
   && pProbe->ai_ردیف_Log_Est[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->ai_ردیف_Log_Est[saved_nEq] - pProbe->ai_ردیف_Log_Est[saved_nEq+1];
    pNew->n_بیرون -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    where_Loop_جمع_کن_Btree_شاخص(pBuilder, pSrc, pProbe, nIter + n_درون_Mul);
    pNew->n_بیرون = saved_n_بیرون;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->p_جدول->zName, pProbe->zName, saved_nEq, rc));
  أرجع rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
ساكن صحيح شاخص_Might_Help_With_ترتیب_By(
  WhereLoopBuilder *pBuilder,
  شاخص *p_شاخص,
  صحيح iCursor
){
  عبارت_List *pOB;
  عبارت_List *a_ستون_عبارت;
  صحيح ii, jj;

  إذا( p_شاخص->bUnordered ) أرجع 0;
  إذا( (pOB = pBuilder->pWInfo->p_ترتیب_By)==0 ) أرجع 0;
  لكل(ii=0; ii<pOB->n_عبارت; ii++){
    عبارت *p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(pOB->a[ii].p_عبارت);
    إذا( NEVER(p_عبارت==0) ) استمر;
    إذا( (p_عبارت->op==TK_ستونی || p_عبارت->op==TK_AGG_ستونی) 
     && p_عبارت->i_جدول==iCursor 
    ){
      إذا( p_عبارت->i_ستونی<0 ) أرجع 1;
      لكل(jj=0; jj<p_شاخص->n_کلید_ستون; jj++){
        إذا( p_عبارت->i_ستونی==p_شاخص->ai_ستونی[jj] ) أرجع 1;
      }
    }وإلا إذا( (a_ستون_عبارت = p_شاخص->a_ستون_عبارت)!=0 ){
      لكل(jj=0; jj<p_شاخص->n_کلید_ستون; jj++){
        إذا( p_شاخص->ai_ستونی[jj]!=XN_عبارت ) استمر;
        إذا( sqlite3_عبارت_Compare_Skip(p_عبارت,a_ستون_عبارت->a[jj].p_عبارت,iCursor)==0 ){
          أرجع 1;
        }
      }
    }
  }
  أرجع 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
ساكن صحيح where_Usable_Partial_شاخص(
  صحيح iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  عبارت *pWhere          /* The WHERE clause from the partial index */
){
  صحيح i;
  Where_جمله *p_جمله;
  Parse *pParse;

  إذا( jointype & JT_LTORJ ) أرجع 0;
  pParse = pWC->pWInfo->pParse;
  طالما( pWhere->op==TK_AND ){
    إذا( !where_Usable_Partial_شاخص(iTab,jointype,pWC,pWhere->p_چپ) ) أرجع 0;
    pWhere = pWhere->p_راست;
  }
  لكل(i=0, p_جمله=pWC->a; i<pWC->n_جمله; i++, p_جمله++){
    عبارت *p_عبارت;
    p_عبارت = p_جمله->p_عبارت;
    إذا( (!عبارت_Has_Property(p_عبارت, EP_OuterON) || p_عبارت->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || عبارت_Has_Property(p_عبارت, EP_OuterON))
     && sqlite3_عبارت_نتیجه_میدهد_عبارت(pParse, p_عبارت, pWhere, iTab)
     && !sqlite3_عبارت_نتیجه_میدهد_عبارت(pParse, p_عبارت, pWhere, -1)
     && (p_جمله->wtFlags & جمله_VNULL)==0
    ){
      أرجع 1;
    }
  }
  أرجع 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
ساكن صحيح عبارت_Is_Covered_By_شاخص(
  ثابت عبارت *p_عبارت,
  ثابت شاخص *pIdx,
  صحيح iTabCur
){
  صحيح i;
  لكل(i=0; i<pIdx->n_ستونی; i++){
    إذا( pIdx->ai_ستونی[i]==XN_عبارت
     && sqlite3_عبارت_Compare(0, p_عبارت, pIdx->a_ستون_عبارت->a[i].p_عبارت, iTabCur)==0
    ){
      أرجع 1;
    }
  }
  أرجع 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
تعريفنوع بنية Covering_شاخص_Check Covering_شاخص_Check;
بنية Covering_شاخص_Check {
  شاخص *pIdx;       /* The index */
  صحيح iTabCur;       /* Cursor number for the corresponding table */
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
ساكن صحيح where_Is_Covering_شاخص_Walk_Callback(Walker *pWalk, عبارت *p_عبارت){
  صحيح i;                    /* Loop counter */
  ثابت شاخص *pIdx;        /* The index of interest */
  ثابت i16 *ai_ستونی;      /* Columns contained in the index */
  u16 n_ستونی;              /* Number of columns in the index */
  Covering_شاخص_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  إذا( (p_عبارت->op==TK_ستونی || p_عبارت->op==TK_AGG_ستونی) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    إذا( p_عبارت->i_جدول!=pCk->iTabCur ) أرجع WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    ai_ستونی = pIdx->ai_ستونی;
    n_ستونی = pIdx->n_ستونی;
    لكل(i=0; i<n_ستونی; i++){
      إذا( ai_ستونی[i]==p_عبارت->i_ستونی ) أرجع WRC_Continue;
    }
    pCk->bUnidx = 1;
    أرجع WRC_Abort;
  }وإلا إذا( pIdx->b_Has_عبارت
         && عبارت_Is_Covered_By_شاخص(p_عبارت, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_عبارت = 1;
    أرجع WRC_Prune;
  }
  أرجع WRC_Continue;
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
ساكن SQLITE_NOINLINE u32 where_Is_Covering_شاخص(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  شاخص *pIdx,           /* Index that is being tested */
  صحيح iTabCur            /* Cursor for the table being indexed */
){
  صحيح i, rc;
  بنية Covering_شاخص_Check ck;
  Walker w;
  إذا( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    أرجع 0;
  }
  إذا( pIdx->b_Has_عبارت==0 ){
    لكل(i=0; i<pIdx->n_ستونی; i++){
      إذا( pIdx->ai_ستونی[i]>=BMS-1 ) اكسر;
    }
    إذا( i>=pIdx->n_ستونی ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      أرجع 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_عبارت = 0;
  ck.bUnidx = 0;
  memset(&w, 0, حجم(w));
  w.x_عبارت_Callback = where_Is_Covering_شاخص_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  إذا( ck.bUnidx ){
    rc = 0;
  }وإلا إذا( ck.b_عبارت ){
    rc = WHERE_EXPRIDX;
  }وإلا{
    rc = WHERE_IDX_ONLY;
  }
  أرجع rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
ساكن فراغ where_نمایهای_عبارت_Cleanup(sqlite3 *db, فراغ *pObject){
  نمایهای_عبارت **pp = (نمایهای_عبارت**)pObject;
  طالما( *pp!=0 ){
    نمایهای_عبارت *p = *pp;
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
ساكن فراغ where_Part_Idx_عبارت(
  Parse *pParse,                  /* Parse context */
  شاخص *pIdx,                    /* Partial index being processed */
  عبارت *pPart,                    /* WHERE clause being processed */
  Bitmask *p_ماسک,                 /* Mask to clear bits in */
  صحيح iIdxCur,                    /* Cursor number for index */
  Src_قلم *p_قلم                  /* The FROM clause entry for the table */
){
  أكد( p_قلم==0 || (p_قلم->fg.jointype & JT_راست)==0 );
  أكد( (p_قلم==0 || p_ماسک==0) && (p_ماسک!=0 || p_قلم!=0) );

  إذا( pPart->op==TK_AND ){
    where_Part_Idx_عبارت(pParse, pIdx, pPart->p_راست, p_ماسک, iIdxCur, p_قلم);
    pPart = pPart->p_چپ;
  }

  إذا( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    عبارت *p_چپ = pPart->p_چپ;
    عبارت *p_راست = pPart->p_راست;
    جدول *p_چپ_Tab;            /* Table of the left operand */
    ستونی *p_چپ_ستون;           /* Column of the left operand */
    u8 aff_راست, aff_چپ;

    إذا( p_چپ->op!=TK_ستونی ) أرجع;
    إذا( !sqlite3_عبارت_Is_Constant(0, p_راست) ) أرجع;
    إذا( !sqlite3_Is_دودویی(sqlite3_عبارت_Compare_Coll_Seq(pParse, pPart)) ) أرجع;
    إذا( p_چپ->i_ستونی<0 ) أرجع;
    p_چپ_Tab = pIdx->p_جدول;
    p_چپ_ستون = &p_چپ_Tab->a_ستون[p_چپ->i_ستونی];
    aff_چپ = p_چپ_ستون->affinity;
    أكد( SQLITE_AFF_BLOB<SQLITE_AFF_متن );
    إذا( aff_چپ<SQLITE_AFF_متن ) أرجع;
    aff_راست = sqlite3_عبارت_Affinity(p_راست);
    إذا( aff_راست!=0 && aff_راست!=aff_چپ ) أرجع;
    إذا( p_قلم ){
      sqlite3 *db = pParse->db;
      نمایهای_عبارت *p = (نمایهای_عبارت*)sqlite3DbMallocRaw(db, حجم(*p));
      إذا( p ){
        صحيح b_Null_ردیف = (p_قلم->fg.jointype&(JT_چپ|JT_LTORJ))!=0;
        p_راست = sqlite3_عبارت_Dup(db, p_راست, 0);
        إذا( (p_چپ_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3_ستونی_Type(p_چپ_ستون, 0);
          أكد( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (صحيح)طول_رشته(t.z);
          p->p_عبارت = sqlite3_عبارت_Alloc(db, TK_CAST, &t, 1);
          sqlite3_عبارت_Attach_Subtrees(db, p->p_عبارت, p_راست, 0);
        }وإلا{
          p->p_عبارت = p_راست;
        }
        p->i_داده_Cur = p_قلم->iCursor;
        p->iIdxCur = iIdxCur;
        p->i_Idx_ستون = p_چپ->i_ستونی;
        p->b_Maybe_Null_ردیف = b_Null_ردیف;
        p->pIENext = pParse->p_Idx_Part_عبارت;
        p->aff = aff_چپ;
        pParse->p_Idx_Part_عبارت = p;
        إذا( p->pIENext==0 ){
          فراغ *pArg = (فراغ*)&pParse->p_Idx_Part_عبارت;
          sqlite3_Parser_جمع_کن_Cleanup(pParse, where_نمایهای_عبارت_Cleanup, pArg);
        }
      }
    }وإلا إذا( p_چپ->i_ستونی<(BMS-1) ){
      *p_ماسک &= ~((Bitmask)1 << p_چپ->i_ستونی);
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
ساكن صحيح where_Loop_جمع_کن_Btree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  شاخص *pProbe;              /* An index we are evaluating */
  شاخص sPk;                  /* A fake index object for the primary key */
  LogEst ai_ردیف_Est_Pk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 ai_ستونی_Pk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  Src_قلم *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  صحيح rc = SQLITE_OK;         /* Return code */
  صحيح i_مرتب_سازی_Idx;               /* Index number */
  صحيح b;                      /* A boolean value */
  LogEst r_اندازه;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  جدول *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  أكد( !IsVirtual(pSrc->pSTab) );

  إذا( pSrc->fg.is_نمایهای_By ){
    أكد( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }وإلا إذا( !HasRowid(pTab) ){
    إذا( pSrc->fg.not_نمایهای ){
      pProbe = sqlite3_Primary_کلید_شاخص(pTab);
    }وإلا{
      pProbe = pTab->p_شاخص;
    }
  }وإلا{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    شاخص *p_اولی;                  /* First of real indices on the table */
    memset(&sPk, 0, حجم(شاخص));
    sPk.n_کلید_ستون = 1;
    sPk.n_ستونی = 1;
    sPk.ai_ستونی = &ai_ستونی_Pk;
    sPk.ai_ردیف_Log_Est = ai_ردیف_Est_Pk;
    sPk.onError = OE_Replace;
    sPk.p_جدول = pTab;
    sPk.sz_Idx_ردیف = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    ai_ردیف_Est_Pk[0] = pTab->n_ردیف_Log_Est;
    ai_ردیف_Est_Pk[1] = 0;
    p_اولی = pSrc->pSTab->p_شاخص;
    sPk.p_بعدی = p_اولی;
    pProbe = &sPk;
  }
  r_اندازه = pTab->n_ردیف_Log_Est;

#اگر_تعریف_نشده SQLITE_OMIT_AUTOMATIC_شاخص
  /* Automatic indexes */
  إذا( !pBuilder->p_Or_مجموعه      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_راست_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_شاخص)!=0
   && !pSrc->fg.is_نمایهای_By  /* Has no INDEXED BY clause */
   && !pSrc->fg.not_نمایهای   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_راست)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_اندازه;         /* Logarithm of the number of rows in the table */
    Where_جمله *p_جمله;
    Where_جمله *pWCEnd = pWC->a + pWC->n_جمله;
    r_Log_اندازه = estLog(r_اندازه);
    لكل(p_جمله=pWC->a; rc==SQLITE_OK && p_جمله<pWCEnd; p_جمله++){
      إذا( p_جمله->prereq_راست & pNew->ماسک_Self ) استمر;
      إذا( جمله_Can_Drive_شاخص(p_جمله, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_شاخص = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = p_جمله;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->r_راهاندازی = r_Log_اندازه + r_اندازه;
        إذا( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->r_راهاندازی += 28;
        }وإلا{
          pNew->r_راهاندازی -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        Apply_هزینه_Multiplier(pNew->r_راهاندازی, pTab->هزینه_Mult);
        إذا( pNew->r_راهاندازی<0 ) pNew->r_راهاندازی = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->n_بیرون = 43;  أكد( 43==sqlite3LogEst(20) );
        pNew->r_پیاپی = sqlite3_Log_Est_جمع_کن(r_Log_اندازه,pNew->n_بیرون);
        pNew->wsFlags = WHERE_AUTO_شاخص;
        pNew->prereq = mPrereq | p_جمله->prereq_راست;
        rc = where_Loop_درج(pBuilder, pNew);
      }
    }
  }
#پایان_اگر /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  لكل(i_مرتب_سازی_Idx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.is_نمایهای_By||pSrc->fg.not_نمایهای)?0:pProbe->p_بعدی,
         i_مرتب_سازی_Idx++
  ){
    إذا( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_شاخص(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      استمر;  /* Partial index inappropriate for this query */
    }
    إذا( pProbe->bNoQuery ) استمر;
    r_اندازه = pProbe->ai_ردیف_Log_Est[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_بالاترین = 0;
    pNew->u.btree.n_Distinct_ستون = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->i_مرتب_سازی_Idx = 0;
    pNew->r_راهاندازی = 0;
    pNew->prereq = mPrereq;
    pNew->n_بیرون = r_اندازه;
    pNew->u.btree.p_شاخص = pProbe;
    pNew->u.btree.p_ترتیب_By = 0;
    b = شاخص_Might_Help_With_ترتیب_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    أكد( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    إذا( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->i_مرتب_سازی_Idx = b ? i_مرتب_سازی_Idx : 0;
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
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
      pNew->r_پیاپی = r_اندازه + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#وإلا
      pNew->r_پیاپی = r_اندازه + 16;
#پایان_اگر
      Apply_هزینه_Multiplier(pNew->r_پیاپی, pTab->هزینه_Mult);
      whereLoopOutputAdjust(pWC, pNew, r_اندازه);
      إذا( pSrc->fg.isSubquery ){
        إذا( pSrc->fg.از_طریق_Coroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        إذا( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.p_ترتیب_By = pSrc->u4.pSubq->pSelect->p_ترتیب_By;
        }
      }وإلا إذا( pSrc->fg.fromExists ){
        pNew->n_بیرون = 0;
      }
      rc = where_Loop_درج(pBuilder, pNew);
      pNew->n_بیرون = r_اندازه;
      إذا( rc ) اكسر;
    }وإلا{
      Bitmask m;
      إذا( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_نمایهای;
      }وإلا{
        m = pSrc->ستون_استفاده_شده & pProbe->ستون_Not_Idxed;
        إذا( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_عبارت(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_نمایهای;
        إذا( m==TOPBIT || (pProbe->b_Has_عبارت && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_شاخص(pWInfo, pProbe, pSrc->iCursor);
          إذا( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            أكد( m!=0 );
          }وإلا{
            m = 0;
            pNew->wsFlags |= isCov;
            إذا( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }وإلا{
              أكد( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }وإلا إذا( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_نمایهای;
        }
      }

      /* Full scan via index */
      إذا( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.is_نمایهای_By
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->sz_Idx_ردیف<pTab->sz_Tab_ردیف)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_Cover_Idx_پیمایش)
          )
      ){
        pNew->i_مرتب_سازی_Idx = b ? i_مرتب_سازی_Idx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->r_پیاپی = r_اندازه + 1 + (15*pProbe->sz_Idx_ردیف)/pTab->sz_Tab_ردیف;
        إذا( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_اندازه + 16;  /* Base cost:  N*3 */
          صحيح ii;
          صحيح iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          لكل(ii=0; ii<pWC2->n_جمله; ii++){
            Where_جمله *p_جمله = &pWC2->a[ii];
            إذا( !sqlite3_عبارت_Covered_By_شاخص(p_جمله->p_عبارت, iCur, pProbe) ){
              اكسر;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            إذا( p_جمله->truthProb<=0 ){
              nLookup += p_جمله->truthProb;
            }وإلا{
              nLookup--;
              إذا( p_جمله->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->r_پیاپی = sqlite3_Log_Est_جمع_کن(pNew->r_پیاپی, nLookup);
        }
        Apply_هزینه_Multiplier(pNew->r_پیاپی, pTab->هزینه_Mult);
        whereLoopOutputAdjust(pWC, pNew, r_اندازه);
        إذا( (pSrc->fg.jointype & JT_راست)!=0 && pProbe->a_ستون_عبارت ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }وإلا{
          إذا( pSrc->fg.fromExists ) pNew->n_بیرون = 0;
          rc = where_Loop_درج(pBuilder, pNew);
        }
        pNew->n_بیرون = r_اندازه;
        إذا( rc ) اكسر;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_جمع_کن_Btree_شاخص(pBuilder, pSrc, pProbe, 0);
    إذا( pBuilder->bldFlags1==SQLITE_BLDF1_نمایهای ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#پایان_اگر
  }
  أرجع rc;
}

#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
ساكن صحيح is_حد_جمله(Where_جمله *p_جمله){
  أكد( p_جمله->eOperator==WO_AUX || p_جمله->e_تطابق_Op==0 );
  أرجع p_جمله->e_تطابق_Op>=SQLITE_شاخص_CONSTRAINT_حد
      && p_جمله->e_تطابق_Op<=SQLITE_شاخص_CONSTRAINT_جابه_جایی;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
ساكن صحيح all_Constraints_استفاده_شده(
  بنية sqlite3_شاخص_constraint_usage *aUsage, 
  صحيح nCons
){
  صحيح ii;
  لكل(ii=0; ii<nCons; ii++){
    إذا( aUsage[ii].argv_شاخص<=0 ) أرجع 0;
  }
  أرجع 1;
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
ساكن صحيح where_Loop_جمع_کن_Virtual_One(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_شاخص_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  صحيح *pb_درون,                      /* OUT: True if plan uses an IN(...) op */
  صحيح *pb_Retry_حد               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_شاخص_Info *pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  بنية sqlite3_شاخص_constraint *pIdxCons;
  بنية sqlite3_شاخص_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  صحيح i;
  صحيح mx_جمله;
  صحيح rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  Src_قلم *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  صحيح nConstraint = pIdxInfo->nConstraint;

  أكد( (mUsable & mPrereq)==mPrereq );
  *pb_درون = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(بنية sqlite3_شاخص_constraint**)&pIdxInfo->aConstraint;
  لكل(i=0; i<nConstraint; i++, pIdxCons++){
    Where_جمله *p_جمله = جمله_From_Where_Clause(pWC, pIdxCons->i_جمله_جابه_جایی);
    pIdxCons->usable = 0;
    إذا( (p_جمله->prereq_راست & mUsable)==p_جمله->prereq_راست
     && (p_جمله->eOperator & mExclude)==0
     && (pb_Retry_حد || !is_حد_جمله(p_جمله))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, حجم(pUsage[0])*nConstraint);
  أكد( pIdxInfo->need_مقصد_Free_Idx_Str==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_رقم = 0;
  pIdxInfo->ترتیب_By_Consumed = 0;
  pIdxInfo->estimated_هزینه = SQLITE_BIG_DBL / (مزدوج)2;
  pIdxInfo->estimated_ردیفها = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->m_Handle_درون = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_بهترین_شاخص(pParse, pSrc->pSTab, pIdxInfo);
  إذا( rc ){
    إذا( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      أرجع SQLITE_OK;
    }
    أرجع rc;
  }

  mx_جمله = -1;
  أكد( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, حجم(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, حجم(pNew->u.vtab));
  pIdxCons = *(بنية sqlite3_شاخص_constraint**)&pIdxInfo->aConstraint;
  لكل(i=0; i<nConstraint; i++, pIdxCons++){
    صحيح i_جمله;
    إذا( (i_جمله = pUsage[i].argv_شاخص - 1)>=0 ){
      Where_جمله *p_جمله;
      صحيح j = pIdxCons->i_جمله_جابه_جایی;
      إذا( i_جمله>=nConstraint
       || j<0
       || (p_جمله = جمله_From_Where_Clause(pWC, j))==0
       || pNew->aLTerm[i_جمله]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_پیام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        أرجع SQLITE_ERROR;
      }
      testcase( i_جمله==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->n_جمله-1 );
      pNew->prereq |= p_جمله->prereq_راست;
      أكد( i_جمله<pNew->nLSlot );
      pNew->aLTerm[i_جمله] = p_جمله;
      إذا( i_جمله>mx_جمله ) mx_جمله = i_جمله;
      testcase( i_جمله==15 );
      testcase( i_جمله==16 );
      إذا( pUsage[i].omit ){
        إذا( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=i_جمله );
          pNew->u.vtab.omit_ماسک |= 1<<i_جمله;
        }وإلا{
          testcase( i!=i_جمله );
        }
        إذا( p_جمله->e_تطابق_Op==SQLITE_شاخص_CONSTRAINT_جابه_جایی ){
          pNew->u.vtab.b_Omit_جابه_جایی = 1;
        }
      }
      إذا( SMASKBIT32(i) & pHidden->m_Handle_درون ){
        pNew->u.vtab.m_Handle_درون |= MASKBIT32(i_جمله);
      }وإلا إذا( (p_جمله->eOperator & WO_درون)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->ترتیب_By_Consumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_شاخص_پیمایش_UNIQUE;
        *pb_درون = 1; أكد( (mExclude & WO_درون)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      أكد( pb_Retry_حد || !is_حد_جمله(p_جمله) );
      أكد( !is_حد_جمله(p_جمله) || i>=nConstraint-2 );
      أكد( !is_حد_جمله(p_جمله) || i==nConstraint-1 || is_حد_جمله(p_جمله+1) );

      إذا( is_حد_جمله(p_جمله) && (*pb_درون || !all_Constraints_استفاده_شده(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pb_Retry_حد = 1;
        أرجع SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mx_جمله+1;
  لكل(i=0; i<=mx_جمله; i++){
    إذا( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_پیام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      أرجع SQLITE_ERROR;
    }
  }
  أكد( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_رقم = pIdxInfo->idx_رقم;
  pNew->u.vtab.needFree = pIdxInfo->need_مقصد_Free_Idx_Str;
  pIdxInfo->need_مقصد_Free_Idx_Str = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->ترتیب_By_Consumed ?
      pIdxInfo->n_ترتیب_By : 0);
  pNew->u.vtab.b_Idx_رقم_Hex = (pIdxInfo->idxFlags&SQLITE_شاخص_پیمایش_HEX)!=0;
  pNew->r_راهاندازی = 0;
  pNew->r_پیاپی = sqlite3LogEstFromDouble(pIdxInfo->estimated_هزینه);
  pNew->n_بیرون = sqlite3LogEst(pIdxInfo->estimated_ردیفها);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  إذا( pIdxInfo->idxFlags & SQLITE_شاخص_پیمایش_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }وإلا{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_درج(pBuilder, pNew);
  إذا( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pb_درون, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  أرجع rc;
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
ثابت حرف *sqlite3_vtab_collation(sqlite3_شاخص_info *pIdxInfo, صحيح iCons){
  Hidden_شاخص_Info *pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  ثابت حرف *zRet = 0;
  إذا( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    صحيح i_جمله = pIdxInfo->aConstraint[iCons].i_جمله_جابه_جایی;
    عبارت *pX = جمله_From_Where_Clause(pHidden->pWC, i_جمله)->p_عبارت;
    إذا( pX->p_چپ ){
      pC = sqlite3_عبارت_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_دودویی);
  }
  أرجع zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
صحيح sqlite3_vtab_درون(sqlite3_شاخص_info *pIdxInfo, صحيح iCons, صحيح bHandle){
  Hidden_شاخص_Info *pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  إذا( m & pHidden->m_درون ){
    إذا( bHandle==0 ){
      pHidden->m_Handle_درون &= ~m;
    }وإلا إذا( bHandle>0 ){
      pHidden->m_Handle_درون |= m;
    }
    أرجع 1;
  }
  أرجع 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
صحيح sqlite3_vtab_rhs_مقدار(
  sqlite3_شاخص_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  صحيح iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_مقدار **ppVal           /* Write value extracted here */
){
  Hidden_شاخص_Info *pH = (Hidden_شاخص_Info*)&pIdxInfo[1];
  sqlite3_مقدار *pVal = 0;
  صحيح rc = SQLITE_OK;
  إذا( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }وإلا{
    إذا( pH->aRhs[iCons]==0 ){
      Where_جمله *p_جمله = جمله_From_Where_Clause(
          pH->pWC, pIdxInfo->aConstraint[iCons].i_جمله_جابه_جایی
      );
      rc = sqlite3_مقدار_From_عبارت(
          pH->pParse->db, p_جمله->p_عبارت->p_راست, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  إذا( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  أرجع rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
صحيح sqlite3_vtab_distinct(sqlite3_شاخص_info *pIdxInfo){
  Hidden_شاخص_Info *pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  أكد( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  أرجع pHidden->eDistinct;
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
فراغ sqlite3_Vtab_استفاده_میکند_All_Schemas(Parse *pParse){
  صحيح nDb = pParse->db->nDb;
  صحيح i;
  لكل(i=0; i<nDb; i++){
    sqlite3_کد_Verify_Schema(pParse, i);
  }
  إذا( Db_ماسک_Non_Zero(pParse->write_ماسک) ){
    لكل(i=0; i<nDb; i++){
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
ساكن صحيح where_Loop_جمع_کن_Virtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  صحيح rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  Src_قلم *pSrc;               /* The FROM clause term to search */
  sqlite3_شاخص_info *p;       /* Object to pass to xBestIndex() */
  صحيح nConstraint;             /* Number of constraints in p */
  صحيح b_درون;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask m_بهترین;               /* Tables used by best possible plan */
  u16 mNoOmit;
  صحيح bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  أكد( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  أكد( IsVirtual(pSrc->pSTab) );
  p = allocate_شاخص_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
  pNew->r_راهاندازی = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  إذا( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_شاخص_Info(pParse->db, p);
    أرجع SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = where_Loop_جمع_کن_Virtual_One(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_درون, &bRetry
  );
  إذا( bRetry ){
    أكد( rc==SQLITE_OK );
    rc = where_Loop_جمع_کن_Virtual_One(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_درون, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  إذا( rc==SQLITE_OK && ((m_بهترین = (pNew->prereq & ~mPrereq))!=0 || b_درون) ){
    صحيح دیده_Zero = 0;             /* True if a plan with no prereqs seen */
    صحيح دیده_Zero_No_درون = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask m_بهترین_No_درون = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    إذا( b_درون ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = where_Loop_جمع_کن_Virtual_One(
          pBuilder, mPrereq, ALLBITS, WO_درون, p, mNoOmit, &b_درون, 0);
      أكد( b_درون==0 );
      m_بهترین_No_درون = pNew->prereq & ~mPrereq;
      إذا( m_بهترین_No_درون==0 ){
        دیده_Zero = 1;
        دیده_Zero_No_درون = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    طالما( rc==SQLITE_OK ){
      صحيح i;
      Bitmask m_بعدی = ALLBITS;
      أكد( m_بعدی>0 );
      لكل(i=0; i<nConstraint; i++){
        صحيح i_جمله = p->aConstraint[i].i_جمله_جابه_جایی;
        Bitmask mThis = جمله_From_Where_Clause(pWC, i_جمله)->prereq_راست & ~mPrereq;
        إذا( mThis>mPrev && mThis<m_بعدی ) m_بعدی = mThis;
      }
      mPrev = m_بعدی;
      إذا( m_بعدی==ALLBITS ) اكسر;
      إذا( m_بعدی==m_بهترین || m_بعدی==m_بهترین_No_درون ) استمر;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)m_بعدی));
      rc = where_Loop_جمع_کن_Virtual_One(
          pBuilder, mPrereq, m_بعدی|mPrereq, 0, p, mNoOmit, &b_درون, 0);
      إذا( pNew->prereq==mPrereq ){
        دیده_Zero = 1;
        إذا( b_درون==0 ) دیده_Zero_No_درون = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    إذا( rc==SQLITE_OK && دیده_Zero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = where_Loop_جمع_کن_Virtual_One(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &b_درون, 0);
      إذا( b_درون==0 ) دیده_Zero_No_درون = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    إذا( rc==SQLITE_OK && دیده_Zero_No_درون==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = where_Loop_جمع_کن_Virtual_One(
          pBuilder, mPrereq, mPrereq, WO_درون, p, mNoOmit, &b_درون, 0);
    }
  }

  free_شاخص_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  أرجع rc;
}
#پایان_اگر /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
ساكن صحيح where_Loop_جمع_کن_Or(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  Where_جمله *p_جمله, *pWCEnd;
  صحيح rc = SQLITE_OK;
  صحيح iCur;
  WhereClause موقت_WC;
  WhereLoopBuilder sSubBuild;
  Where_Or_مجموعه s_جمع, sCur;
  Src_قلم *p_قلم;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->n_جمله;
  pNew = pBuilder->pNew;
  memset(&s_جمع, 0, حجم(s_جمع));
  p_قلم = pWInfo->pTabList->a + pNew->iTab;
  iCur = p_قلم->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  إذا( p_قلم->fg.jointype & JT_راست ) أرجع SQLITE_OK;

  لكل(p_جمله=pWC->a; p_جمله<pWCEnd && rc==SQLITE_OK; p_جمله++){
    إذا( (p_جمله->eOperator & WO_OR)!=0
     && (p_جمله->u.pOrInfo->indexable & pNew->ماسک_Self)!=0
     && (!عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON)
          || p_جمله->p_عبارت->w.iJoin==iCur)
    ){
      WhereClause * ثابت pOrWC = &p_جمله->u.pOrInfo->wc;
      Where_جمله * ثابت pOrWCEnd = &pOrWC->a[pOrWC->n_جمله];
      Where_جمله *p_Or_جمله;
      صحيح once = 1;
      صحيح i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.p_Or_مجموعه = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", p_جمله));
      لكل(p_Or_جمله=pOrWC->a; p_Or_جمله<pOrWCEnd; p_Or_جمله++){
        إذا( (p_Or_جمله->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &p_Or_جمله->u.pAndInfo->wc;
        }وإلا إذا( p_Or_جمله->چپ_Cursor==iCur ){
          موقت_WC.pWInfo = pWC->pWInfo;
          موقت_WC.pOuter = pWC;
          موقت_WC.op = TK_AND;
          موقت_WC.n_جمله = 1;
          موقت_WC.n_پایه = 1;
          موقت_WC.a = p_Or_جمله;
          sSubBuild.pWC = &موقت_WC;
        }وإلا{
          استمر;
        }
        sCur.n = 0;
#اگر_تعریف_شده WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (صحيح)(p_Or_جمله-pOrWC->a), p_جمله, sSubBuild.pWC->n_جمله));
        إذا( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
        إذا( IsVirtual(p_قلم->pSTab) ){
          rc = where_Loop_جمع_کن_Virtual(&sSubBuild, mPrereq, mUnusable);
        }وإلا
#پایان_اگر
        {
          rc = where_Loop_جمع_کن_Btree(&sSubBuild, mPrereq);
        }
        إذا( rc==SQLITE_OK ){
          rc = where_Loop_جمع_کن_Or(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_انجامشده );
        إذا( sCur.n==0 ){
          s_جمع.n = 0;
          اكسر;
        }وإلا إذا( once ){
          whereOrMove(&s_جمع, &sCur);
          once = 0;
        }وإلا{
          Where_Or_مجموعه sPrev;
          whereOrMove(&sPrev, &s_جمع);
          s_جمع.n = 0;
          لكل(i=0; i<sPrev.n; i++){
            لكل(j=0; j<sCur.n; j++){
              where_Or_درج(&s_جمع, sPrev.a[i].prereq | sCur.a[j].prereq,
                            sqlite3_Log_Est_جمع_کن(sPrev.a[i].r_پیاپی, sCur.a[j].r_پیاپی),
                            sqlite3_Log_Est_جمع_کن(sPrev.a[i].n_بیرون, sCur.a[j].n_بیرون));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = p_جمله;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->r_راهاندازی = 0;
      pNew->i_مرتب_سازی_Idx = 0;
      memset(&pNew->u, 0, حجم(pNew->u));
      لكل(i=0; rc==SQLITE_OK && i<s_جمع.n; i++){
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
        pNew->r_پیاپی = s_جمع.a[i].r_پیاپی + 1;
        pNew->n_بیرون = s_جمع.a[i].n_بیرون;
        pNew->prereq = s_جمع.a[i].prereq;
        rc = where_Loop_درج(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", p_جمله));
    }
  }
  أرجع rc;
}

/*
** Add all WhereLoop objects for all tables
*/
ساكن صحيح where_Loop_جمع_کن_All(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask m_پیشین = 0;
  صحيح iTab;
  SrcList *pTabList = pWInfo->pTabList;
  Src_قلم *p_قلم;
  Src_قلم *p_انتها = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  صحيح rc = SQLITE_OK;
  صحيح b_اولی_Past_RJ = 0;
  صحيح has_راست_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  أكد( pNew->nLTerm==0 );
  أكد( pNew->wsFlags==0 );
  أكد( pNew->nLSlot>=Array_اندازه(pNew->aLTermSpace) );
  أكد( pNew->aLTerm!=0 );

  pBuilder->i_Plan_حد = SQLITE_QUERY_PLANNER_حد;
  لكل(iTab=0, p_قلم=pTabList->a; p_قلم<p_انتها; iTab++, p_قلم++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->i_Plan_حد += SQLITE_QUERY_PLANNER_حد_INCR;
    pNew->ماسک_Self = sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, p_قلم->iCursor);
    إذا( b_اولی_Past_RJ
     || (p_قلم->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
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
      إذا( p_قلم->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( p_قلم->fg.jointype & JT_LTORJ );
        testcase( p_قلم->fg.jointype & JT_CROSS );
        has_راست_Cross_Join = 1;
      }
      mPrereq |= m_پیشین;
      b_اولی_Past_RJ = (p_قلم->fg.jointype & JT_راست)!=0;
    }وإلا إذا( p_قلم->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      Where_جمله *p_جمله;
      صحيح i;
      لكل(i=pWC->n_پایه, p_جمله=pWC->a; i>0; i--, p_جمله++){
        إذا( (pNew->ماسک_Self & p_جمله->prereqAll)!=0 ){
          mPrereq |= (p_جمله->prereqAll & (pNew->ماسک_Self-1));
        }
      }
    }وإلا إذا( !has_راست_Cross_Join ){
      mPrereq = 0;
    }
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
    إذا( IsVirtual(p_قلم->pSTab) ){
      Src_قلم *p;
      لكل(p=&p_قلم[1]; p<p_انتها; p++){
        إذا( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, p->iCursor);
        }
      }
      rc = where_Loop_جمع_کن_Virtual(pBuilder, mPrereq, mUnusable);
    }وإلا
#پایان_اگر /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = where_Loop_جمع_کن_Btree(pBuilder, mPrereq);
    }
    إذا( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = where_Loop_جمع_کن_Or(pBuilder, mPrereq, mUnusable);
    }
    m_پیشین |= pNew->ماسک_Self;
    إذا( rc || db->mallocFailed ){
      إذا( rc==SQLITE_انجامشده ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }وإلا{
        اكسر;
      }
    }
  }

  whereLoopClear(db, pNew);
  أرجع rc;
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
ساكن SQLITE_NOINLINE صحيح where_مسیر_تطابق_Subquery_OB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  صحيح iLoop,              /* Which level of the nested loop.  0==outermost */
  صحيح iCur,               /* Cursor used by the this loop */
  عبارت_List *p_ترتیب_By,     /* The ORDER BY clause on the whole query */
  Bitmask *p_Rev_ماسک,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  صحيح iOB;                /* Index into pOrderBy->a[] */
  صحيح jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  عبارت *pOBExpr;          /* Current term of outer ORDER BY */
  عبارت_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.p_ترتیب_By;
  أكد( pSubOB!=0 );
  لكل(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  لكل(jSub=0; jSub<pSubOB->n_عبارت && iOB<p_ترتیب_By->n_عبارت; jSub++, iOB++){
    إذا( pSubOB->a[jSub].u.x.i_ترتیب_By_ستون==0 ) اكسر;
    pOBExpr = p_ترتیب_By->a[iOB].p_عبارت;
    إذا( pOBExpr->op!=TK_ستونی && pOBExpr->op!=TK_AGG_ستونی ) اكسر;
    إذا( pOBExpr->i_جدول!=iCur ) اكسر;
    إذا( pOBExpr->i_ستونی!=pSubOB->a[jSub].u.x.i_ترتیب_By_ستون-1 ) اكسر;
    إذا( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = p_ترتیب_By->a[iOB].fg.مرتب_سازی_Flags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.مرتب_سازی_Flags;   /* sortFlags for jSub */
      إذا( (sfSub & KEYINFO_ترتیب_BIGNULL) != (sfOB & KEYINFO_ترتیب_BIGNULL) ){
        اكسر;
      }
      revIdx = sfSub & KEYINFO_ترتیب_DESC;
      إذا( jSub>0 ){
        إذا( (rev^revIdx)!=(sfOB & KEYINFO_ترتیب_DESC) ){
          اكسر;
        }
      }وإلا{
        rev = revIdx ^ (sfOB & KEYINFO_ترتیب_DESC);
        إذا( rev ){
          إذا( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            اكسر;
          }
          *p_Rev_ماسک |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  أرجع jSub>0;
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
ساكن i8 where_مسیر_Satisfies_ترتیب_By(
  WhereInfo *pWInfo,    /* The WHERE clause */
  عبارت_List *p_ترتیب_By,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  Where_مسیر *p_مسیر,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *p_آخرین,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *p_Rev_ماسک     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 rev_مجموعه;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 is_ترتیب_Distinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 is_تطابق;           /* iColumn matches a term of the ORDER BY clause */
  u16 eq_Op_ماسک;         /* Allowed equality operators */
  u16 n_کلید_ستون;          /* Number of key columns in pIndex */
  u16 n_ستونی;          /* Total number of ordered columns in the index */
  u16 n_ترتیب_By;         /* Number terms in the ORDER BY clause */
  صحيح iLoop;            /* Index of WhereLoop in pPath being processed */
  صحيح i, j;             /* Loop counters */
  صحيح iCur;             /* Cursor number for current WhereLoop */
  صحيح i_ستونی;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  Where_جمله *p_جمله;     /* A single term of the WHERE clause */
  عبارت *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  شاخص *p_شاخص;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask ob_انجامشده;       /* Mask of all ORDER BY terms */
  Bitmask ترتیب_Distinct_ماسک;  /* Mask of all well-ordered loops */
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

  أكد( p_ترتیب_By!=0 );
  إذا( nLoop && OptimizationDisabled(db, SQLITE_ترتیب_By_Idx_Join) ) أرجع 0;

  n_ترتیب_By = p_ترتیب_By->n_عبارت;
  testcase( n_ترتیب_By==BMS-1 );
  إذا( n_ترتیب_By>BMS-1 ) أرجع 0;  /* Cannot optimize overly large ORDER BYs */
  is_ترتیب_Distinct = 1;
  ob_انجامشده = MASKBIT(n_ترتیب_By)-1;
  ترتیب_Distinct_ماسک = 0;
  ready = 0;
  eq_Op_ماسک = WO_EQ | WO_IS | WO_ISNULL;
  إذا( wctrlFlags & (WHERE_ORDERBY_حد|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eq_Op_ماسک |= WO_درون;
  }
  لكل(iLoop=0; is_ترتیب_Distinct && obSat<ob_انجامشده && iLoop<=nLoop; iLoop++){
    إذا( iLoop>0 ) ready |= pLoop->ماسک_Self;
    إذا( iLoop<nLoop ){
      pLoop = p_مسیر->aLoop[iLoop];
      إذا( wctrlFlags & WHERE_ORDERBY_حد ) استمر;
    }وإلا{
      pLoop = p_آخرین;
    }
    إذا( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      إذا( pLoop->u.vtab.isOrdered && pWInfo->p_ترتیب_By==p_ترتیب_By ){
        obSat = ob_انجامشده;
      }وإلا{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        is_ترتیب_Distinct = 0;
      }
      اكسر;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    لكل(i=0; i<n_ترتیب_By; i++){
      إذا( MASKBIT(i) & obSat ) استمر;
      pOBExpr = sqlite3_عبارت_Skip_Collate_And_Likely(p_ترتیب_By->a[i].p_عبارت);
      إذا( NEVER(pOBExpr==0) ) استمر;
      إذا( pOBExpr->op!=TK_ستونی && pOBExpr->op!=TK_AGG_ستونی ) استمر;
      إذا( pOBExpr->i_جدول!=iCur ) استمر;
      p_جمله = sqlite3_Where_پیدا_کن_جمله(&pWInfo->sWC, iCur, pOBExpr->i_ستونی,
                       ~ready, eq_Op_ماسک, 0);
      إذا( p_جمله==0 ) استمر;
      إذا( p_جمله->eOperator==WO_درون ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        أكد( wctrlFlags &
               (WHERE_ORDERBY_حد|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        لكل(j=0; j<pLoop->nLTerm && p_جمله!=pLoop->aLTerm[j]; j++){}
        إذا( j>=pLoop->nLTerm ) استمر;
      }
      إذا( (p_جمله->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->i_ستونی>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_عبارت_NNColl_Seq(pParse, p_ترتیب_By->a[i].p_عبارت);
        ثابت عبارت *pCExpr = p_جمله->p_عبارت;
        CollSeq *pColl2 = sqlite3_عبارت_Compare_Coll_Seq(pParse, pCExpr);
        حرف aff_راست;
        أكد( pColl1 );
        إذا( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          استمر;
        }
        aff_راست = sqlite3_عبارت_Affinity(pCExpr->p_راست);
        /* Left operand must be a column, hence always has affinity */
        أكد( sqlite3_عبارت_Affinity(pCExpr->p_چپ)!=0 );
        إذا( aff_راست!=0 && aff_راست!=sqlite3_عبارت_Affinity(pCExpr->p_چپ) ){
          استمر;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    إذا( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      إذا( pLoop->wsFlags & WHERE_IPK ){
        إذا( pLoop->u.btree.p_ترتیب_By
         && OptimizationEnabled(db, SQLITE_ترتیب_By_Subq)
         &&  where_مسیر_تطابق_Subquery_OB(pLoop,iLoop,iCur,
                                     p_ترتیب_By,p_Rev_ماسک, &obSat, wctrlFlags)
        ){
          n_ستونی = 0;
          is_ترتیب_Distinct = 0;
        }وإلا{
          n_ستونی = 1;
        }
        p_شاخص = 0;
        n_کلید_ستون = 1;
      }وإلا إذا( (p_شاخص = pLoop->u.btree.p_شاخص)==0 || p_شاخص->bUnordered ){
        أرجع 0;
      }وإلا{
        n_کلید_ستون = p_شاخص->n_کلید_ستون;
        n_ستونی = p_شاخص->n_ستونی;
        أكد( n_ستونی==n_کلید_ستون+1 || !HasRowid(p_شاخص->p_جدول) );
        أكد( p_شاخص->ai_ستونی[n_ستونی-1]==XN_ROWID
                          || !HasRowid(p_شاخص->p_جدول));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        is_ترتیب_Distinct = Is_Unique_شاخص(p_شاخص)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = rev_مجموعه = 0;
      distinctColumns = 0;
      لكل(j=0; j<n_ستونی; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        أكد( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        إذا( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          إذا( (eOp & eq_Op_ماسک)!=0 ){
            إذا( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( is_ترتیب_Distinct );
              is_ترتیب_Distinct = 0;
            }
            استمر; 
          }وإلا إذا( ALWAYS(eOp & WO_درون) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            عبارت *pX = pLoop->aLTerm[j]->p_عبارت;
            لكل(i=j+1; i<pLoop->u.btree.nEq; i++){
              إذا( pLoop->aLTerm[i]->p_عبارت==pX ){
                أكد( (pLoop->aLTerm[i]->eOperator & WO_درون) );
                bOnce = 0;
                اكسر;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        إذا( p_شاخص ){
          i_ستونی = p_شاخص->ai_ستونی[j];
          revIdx = p_شاخص->a_مرتب_سازی_ترتیب[j] & KEYINFO_ترتیب_DESC;
          إذا( i_ستونی==p_شاخص->p_جدول->iPKey ) i_ستونی = XN_ROWID;
        }وإلا{
          i_ستونی = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        إذا( is_ترتیب_Distinct ){
          إذا( i_ستونی>=0
           && j>=pLoop->u.btree.nEq
           && p_شاخص->p_جدول->a_ستون[i_ستونی].notNull==0
          ){
            is_ترتیب_Distinct = 0;
          }
          إذا( i_ستونی==XN_عبارت ){
            is_ترتیب_Distinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        is_تطابق = 0;
        لكل(i=0; bOnce && i<n_ترتیب_By; i++){
          إذا( MASKBIT(i) & obSat ) استمر;
          pOBExpr = sqlite3_عبارت_Skip_Collate_And_Likely(p_ترتیب_By->a[i].p_عبارت);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          إذا( NEVER(pOBExpr==0) ) استمر;
          إذا( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          إذا( i_ستونی>=XN_ROWID ){
            إذا( pOBExpr->op!=TK_ستونی && pOBExpr->op!=TK_AGG_ستونی ) استمر;
            إذا( pOBExpr->i_جدول!=iCur ) استمر;
            إذا( pOBExpr->i_ستونی!=i_ستونی ) استمر;
          }وإلا{
            عبارت *p_Ix_عبارت = p_شاخص->a_ستون_عبارت->a[j].p_عبارت;
            إذا( sqlite3_عبارت_Compare_Skip(pOBExpr, p_Ix_عبارت, iCur) ){
              استمر;
            }
          }
          إذا( i_ستونی!=XN_ROWID ){
            pColl = sqlite3_عبارت_NNColl_Seq(pWInfo->pParse, p_ترتیب_By->a[i].p_عبارت);
            إذا( sqlite3StrICmp(pColl->zName, p_شاخص->azColl[j])!=0 ) استمر;
          }
          إذا( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.n_Distinct_ستون = j+1;
          }
          is_تطابق = 1;
          اكسر;
        }
        إذا( is_تطابق && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          إذا( rev_مجموعه ){
            إذا( (rev ^ revIdx)
                           != (p_ترتیب_By->a[i].fg.مرتب_سازی_Flags&KEYINFO_ترتیب_DESC)
            ){
              is_تطابق = 0;
            }
          }وإلا{
            rev = revIdx ^ (p_ترتیب_By->a[i].fg.مرتب_سازی_Flags & KEYINFO_ترتیب_DESC);
            إذا( rev ) *p_Rev_ماسک |= MASKBIT(iLoop);
            rev_مجموعه = 1;
          }
        }
        إذا( is_تطابق && (p_ترتیب_By->a[i].fg.مرتب_سازی_Flags & KEYINFO_ترتیب_BIGNULL) ){
          إذا( j==pLoop->u.btree.nEq 
           && (j<n_کلید_ستون||HasRowid(p_شاخص->p_جدول)||!Is_Primary_کلید_شاخص(p_شاخص))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_مرتب_سازی;
          }وإلا{
            is_تطابق = 0;
          }
        }
        إذا( is_تطابق ){
          إذا( i_ستونی==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }وإلا{
          /* No match found */
          أكد( n_کلید_ستون>0 );
          إذا( j<n_کلید_ستون ){
            testcase( is_ترتیب_Distinct!=0 );
            is_ترتیب_Distinct = 0;
          }
          اكسر;
        }
      } /* end Loop over all index columns */
      إذا( distinctColumns ){
        testcase( is_ترتیب_Distinct==0 );
        is_ترتیب_Distinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    إذا( is_ترتیب_Distinct ){
      ترتیب_Distinct_ماسک |= pLoop->ماسک_Self;
      لكل(i=0; i<n_ترتیب_By; i++){
        عبارت *p;
        Bitmask m_جمله;
        إذا( MASKBIT(i) & obSat ) استمر;
        p = p_ترتیب_By->a[i].p_عبارت;
        m_جمله = sqlite3_Where_عبارت_Usage(&pWInfo->s_ماسک_مجموعه,p);
        إذا( m_جمله==0 && !sqlite3_عبارت_Is_Constant(0,p) ) استمر;
        إذا( (m_جمله&~ترتیب_Distinct_ماسک)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  إذا( obSat==ob_انجامشده ) أرجع (i8)n_ترتیب_By;
  إذا( !is_ترتیب_Distinct ){
    لكل(i=n_ترتیب_By-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      إذا( (obSat&m)==m ) أرجع i;
    }
    أرجع 0;
  }
  أرجع -1;
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
صحيح sqlite3WhereIsSorted(WhereInfo *pWInfo){
  أكد( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  أكد( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  أرجع pWInfo->sorted;
}

#اگر_تعریف_شده WHERETRACE_ENABLED
/* For debugging use only: */
ساكن ثابت حرف *where_مسیر_Name(Where_مسیر *p_مسیر, صحيح nLoop, WhereLoop *p_آخرین){
  ساكن حرف zName[65];
  صحيح i;
  لكل(i=0; i<nLoop; i++){ zName[i] = p_مسیر->aLoop[i]->cId; }
  إذا( p_آخرین ) zName[i++] = p_آخرین->cId;
  zName[i] = 0;
  أرجع zName;
}
#پایان_اگر

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
ساكن LogEst where_ترتیبدهی_هزینه(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst n_ردیف,       /* Estimated number of rows to sort */
  صحيح n_ترتیب_By,      /* Number of ORDER BY clause terms */
  صحيح nSorted        /* Number of initial ORDER BY terms naturally in order */
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
  LogEst r_مرتب_سازی_هزینه, n_ستون;
  أكد( pWInfo->pSelect!=0 );
  أكد( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  n_ستون = sqlite3LogEst((pWInfo->pSelect->pEList->n_عبارت+59)/30);
  r_مرتب_سازی_هزینه = n_ردیف + n_ستون;
  إذا( nSorted>0 ){
    /* Scale the result by (Y/X) */
    r_مرتب_سازی_هزینه += sqlite3LogEst((n_ترتیب_By-nSorted)*100/n_ترتیب_By) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  إذا( (pWInfo->wctrlFlags & WHERE_USE_حد)!=0 ){
    r_مرتب_سازی_هزینه += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    إذا( nSorted!=0 ){
      r_مرتب_سازی_هزینه += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    إذا( pWInfo->i_حد<n_ردیف ){
      n_ردیف = pWInfo->i_حد;
    }
  }وإلا إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    إذا( n_ردیف>10 ){ n_ردیف -= 10;  أكد( 10==sqlite3LogEst(2) ); }
  }
  r_مرتب_سازی_هزینه += estLog(n_ردیف);
  أرجع r_مرتب_سازی_هزینه;
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
ساكن صحيح computeMxChoice(WhereInfo *pWInfo){
  صحيح nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#اگر_تعریف_شده SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
    أكد( pWLoop->ماسک_Self==MASKBIT(pWLoop->iTab) );
    أكد( pWLoop->p_بعدی_Loop==0 || pWLoop->iTab<=pWLoop->p_بعدی_Loop->iTab );
  }
#پایان_اگر /* SQLITE_DEBUG */

  إذا( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->b_ستاره_انجامشده
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_ستاره_Query)
  ){
    Src_قلم *aFromTabs;    /* All terms of the FROM clause */
    صحيح iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *p_شروع;     /* Where to start searching for dimension-tables */

    pWInfo->b_ستاره_انجامشده = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    أكد( !pWInfo->b_ستاره_استفاده_شده );
    aFromTabs = pWInfo->pTabList->a;
    p_شروع = pWInfo->pLoops;
    لكل(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      صحيح nDep = 0;             /* Number of dimension tables */
      LogEst mx_پیاپی;             /* Maximum SCAN cost of a fact table */
      Bitmask m_دیده = 0;        /* Mask of dimension tables */
      Src_قلم *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      إذا( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        إذا( iFromIdx+3 > nLoop ){
          اكسر;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        طالما( p_شروع && p_شروع->iTab<=iFromIdx ){
          p_شروع = p_شروع->p_بعدی_Loop;
        }
      }
      لكل(pWLoop=p_شروع; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
        إذا( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          اكسر; /* Constraint (bb) */
        }
        إذا( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->ماسک_Self & m_دیده)==0  /* pWInfo not already a dependency */
         && (pWLoop->ماسک_Self & mSelfJoin)==0 /* Not a self-join */
        ){
          إذا( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }وإلا{
            nDep++;
            m_دیده |= pWLoop->ماسک_Self;
          }
        }
      }
      إذا( nDep<=2 ){
        استمر; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#اگر_تعریف_شده WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      إذا( !pWInfo->b_ستاره_استفاده_شده ){
        لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
          pWLoop->r_ستاره_Delta = 0;
        }
      }
#پایان_اگر
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x80000 */
      إذا( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = m_دیده;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        لكل(pWLoop=p_شروع; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
          إذا( mShow & pWLoop->ماسک_Self ){
            Src_قلم *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->ماسک_Self;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#پایان_اگر
      pWInfo->b_ستاره_استفاده_شده = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mx_پیاپی = LOGEST_MIN;
      لكل(pWLoop=p_شروع; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
        إذا( pWLoop->iTab<iFromIdx ) استمر;
        إذا( pWLoop->iTab>iFromIdx ) اكسر;
        إذا( pWLoop->r_پیاپی>mx_پیاپی ) mx_پیاپی = pWLoop->r_پیاپی;
      }
      إذا( ALWAYS(mx_پیاپی<LOGEST_MAX) ) mx_پیاپی++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      لكل(pWLoop=p_شروع; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
        إذا( (pWLoop->ماسک_Self & m_دیده)==0 ) استمر;
        إذا( pWLoop->nLTerm ) استمر;
        إذا( pWLoop->r_پیاپی<mx_پیاپی ){
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x80000 */
          إذا( sqlite3WhereTrace & 0x80000 ){
            Src_قلم *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mx_پیاپی
            );
          }
          pWLoop->r_ستاره_Delta = mx_پیاپی - pWLoop->r_پیاپی;
#پایان_اگر /* WHERETRACE_ENABLED */
          pWLoop->r_پیاپی = mx_پیاپی;
        }
      }
    }
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x80000 */
    إذا( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->b_ستاره_استفاده_شده ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
        إذا( pWLoop->r_ستاره_Delta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#پایان_اگر
  }
  أرجع pWInfo->b_ستاره_استفاده_شده ? 18 : 12;
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
ساكن SQLITE_NOINLINE صحيح whereLoopIsNoBetter(
  ثابت WhereLoop *pCandidate,
  ثابت WhereLoop *pBaseline
){
  إذا( (pCandidate->wsFlags & WHERE_نمایهای)==0 ) أرجع 1;
  إذا( (pBaseline->wsFlags & WHERE_نمایهای)==0 ) أرجع 1;
  إذا( pCandidate->u.btree.p_شاخص->sz_Idx_ردیف <
        pBaseline->u.btree.p_شاخص->sz_Idx_ردیف ) أرجع 0;
  أرجع 1;
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
ساكن صحيح where_مسیر_Solver(WhereInfo *pWInfo, LogEst n_ردیف_Est){
  صحيح mxChoice;             /* Maximum number of simultaneous paths tracked */
  صحيح nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  صحيح iLoop;                /* Loop counter over the terms of the join */
  صحيح ii, jj;               /* Loop counters */
  صحيح mxI = 0;              /* Index of next entry to replace */
  صحيح n_ترتیب_By;             /* Number of ORDER BY clause terms */
  LogEst mx_هزینه = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  صحيح n_مقصد, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  Where_مسیر *aFrom;         /* All nFrom paths at the previous level */
  Where_مسیر *a_مقصد;           /* The nTo best paths at the current level */
  Where_مسیر *pFrom;         /* An element of aFrom[] that we are working on */
  Where_مسیر *p_مقصد;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *a_مرتب_سازی_هزینه = 0;    /* Sorting and partial sorting costs */
  حرف *pSpace;             /* Temporary memory used by this routine */
  صحيح nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     n_ردیف_Est, pParse->nQueryLoop));
  /* TUNING: mxChoice is the maximum number of possible paths to preserve
  ** at each step.  Based on the number of loops in the FROM clause:
  **
  **     nLoop      mxChoice
  **     -----      --------
  **       1            1            // the most common case
  **       2            5
  **       3+        12 or 18        // see computeMxChoice()
  */
  إذا( nLoop<=1 ){
    mxChoice = 1;
  }وإلا إذا( nLoop==2 ){
    mxChoice = 5;
  }وإلا إذا( pParse->nErr ){
    mxChoice = 1;
  }وإلا{
    mxChoice = computeMxChoice(pWInfo);
  }
  أكد( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  إذا( pWInfo->p_ترتیب_By==0 || n_ردیف_Est==0 ){
    n_ترتیب_By = 0;
  }وإلا{
    n_ترتیب_By = pWInfo->p_ترتیب_By->n_عبارت;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (حجم(Where_مسیر)+حجم(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += حجم(LogEst) * n_ترتیب_By;
  pSpace = sqlite3_پشته_Alloc_Raw_NN(pParse->db, nSpace);
  إذا( pSpace==0 ) أرجع SQLITE_NOMEM_BKPT;
  a_مقصد = (Where_مسیر*)pSpace;
  aFrom = a_مقصد+mxChoice;
  memset(aFrom, 0, حجم(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  لكل(ii=mxChoice*2, pFrom=a_مقصد; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  إذا( n_ترتیب_By ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    a_مرتب_سازی_هزینه = (LogEst*)pX;
    memset(a_مرتب_سازی_هزینه, 0, حجم(LogEst) * n_ترتیب_By);
  }
  أكد( a_مرتب_سازی_هزینه==0 || &pSpace[nSpace]==(حرف*)&a_مرتب_سازی_هزینه[n_ترتیب_By] );
  أكد( a_مرتب_سازی_هزینه!=0 || &pSpace[nSpace]==(حرف*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].n_ردیف = MIN(pParse->nQueryLoop, 48);  أكد( 48==sqlite3LogEst(28) );
  nFrom = 1;
  أكد( aFrom[0].isOrdered==0 );
  إذا( n_ترتیب_By ){
    /* If nLoop is zero, then there are no FROM terms in the query. Since
    ** in this case the query may return a maximum of one row, the results
    ** are already in the requested order. Set isOrdered to nOrderBy to
    ** indicate this. Or, if nLoop is greater than zero, set isOrdered to
    ** -1, indicating that the result set may or may not be ordered,
    ** depending on the loops added to the current plan.  */
    aFrom[0].isOrdered = nLoop>0 ? -1 : n_ترتیب_By;
  }

  /* Compute successively longer WherePaths using the previous generation
  ** of WherePaths as the basis for the next.  Keep track of the mxChoice
  ** best paths at each generation */
  لكل(iLoop=0; iLoop<nLoop; iLoop++){
    n_مقصد = 0;
    لكل(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بعدی_Loop){
        LogEst n_بیرون;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst r_هزینه;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask ماسک_New;                  /* Mask of src visited by (..) */
        Bitmask rev_ماسک;                  /* Mask of rev-order loops for (..) */

        إذا( (pWLoop->prereq & ~pFrom->ماسک_Loop)!=0 ) استمر;
        إذا( (pWLoop->ماسک_Self & pFrom->ماسک_Loop)!=0 ) استمر;
        إذا( (pWLoop->wsFlags & WHERE_AUTO_شاخص)!=0 && pFrom->n_ردیف<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          أكد( 10==sqlite3LogEst(2) );
          استمر;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->r_پیاپی + pFrom->n_ردیف;
        إذا( pWLoop->r_راهاندازی ){
          rUnsort = sqlite3_Log_Est_جمع_کن(pWLoop->r_راهاندازی, rUnsort);
        }
        rUnsort = sqlite3_Log_Est_جمع_کن(rUnsort, pFrom->rUnsort);
        n_بیرون = pFrom->n_ردیف + pWLoop->n_بیرون;
        ماسک_New = pFrom->ماسک_Loop | pWLoop->ماسک_Self;
        isOrdered = pFrom->isOrdered;
        إذا( isOrdered<0 ){
          rev_ماسک = 0;
          isOrdered = where_مسیر_Satisfies_ترتیب_By(pWInfo,
                       pWInfo->p_ترتیب_By, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &rev_ماسک);
        }وإلا{
          rev_ماسک = pFrom->revLoop;
        }
        إذا( isOrdered>=0 && isOrdered<n_ترتیب_By ){
          إذا( a_مرتب_سازی_هزینه[isOrdered]==0 ){
            a_مرتب_سازی_هزینه[isOrdered] = where_ترتیبدهی_هزینه(
                pWInfo, n_ردیف_Est, n_ترتیب_By, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          r_هزینه = sqlite3_Log_Est_جمع_کن(rUnsort, a_مرتب_سازی_هزینه[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               a_مرتب_سازی_هزینه[isOrdered], (n_ترتیب_By-isOrdered), n_ترتیب_By,
               rUnsort, r_هزینه));
        }وإلا{
          r_هزینه = rUnsort;
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
        testcase( n_مقصد==0 );
        لكل(jj=0, p_مقصد=a_مقصد; jj<n_مقصد; jj++, p_مقصد++){
          إذا( p_مقصد->ماسک_Loop==ماسک_New
           && ( ((p_مقصد->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==n_مقصد-1 );
            اكسر;
          }
        }
        إذا( jj>=n_مقصد ){
          /* None of the existing best-so-far paths match the candidate. */
          إذا( n_مقصد>=mxChoice
           && (r_هزینه>mx_هزینه || (r_هزینه==mx_هزینه && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x4 */
            إذا( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  where_مسیر_Name(pFrom, iLoop, pWLoop), r_هزینه, n_بیرون, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#پایان_اگر
            استمر;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          إذا( n_مقصد<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = n_مقصد++;
          }وإلا{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          p_مقصد = &a_مقصد[jj];
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x4 */
          إذا( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                where_مسیر_Name(pFrom, iLoop, pWLoop), r_هزینه, n_بیرون, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#پایان_اگر
        }وإلا{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          إذا( (p_مقصد->r_هزینه<r_هزینه)
           || (p_مقصد->r_هزینه==r_هزینه && p_مقصد->n_ردیف<n_بیرون)
           || (p_مقصد->r_هزینه==r_هزینه && p_مقصد->n_ردیف==n_بیرون && p_مقصد->rUnsort<rUnsort)
           || (p_مقصد->r_هزینه==r_هزینه && p_مقصد->n_ردیف==n_بیرون && p_مقصد->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, p_مقصد->aLoop[iLoop]) )
          ){
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x4 */
            إذا( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  where_مسیر_Name(pFrom, iLoop, pWLoop), r_هزینه, n_بیرون, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  where_مسیر_Name(p_مقصد, iLoop+1, 0), p_مقصد->r_هزینه, p_مقصد->n_ردیف,
                  p_مقصد->rUnsort, p_مقصد->isOrdered>=0 ? p_مقصد->isOrdered+'0' : '?');
            }
#پایان_اگر
            /* Discard the candidate path from further consideration */
            testcase( p_مقصد->r_هزینه==r_هزینه );
            استمر;
          }
          testcase( p_مقصد->r_هزینه==r_هزینه+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x4 */
          إذا( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                where_مسیر_Name(pFrom, iLoop, pWLoop), r_هزینه, n_بیرون, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                where_مسیر_Name(p_مقصد, iLoop+1, 0), p_مقصد->r_هزینه, p_مقصد->n_ردیف,
                p_مقصد->rUnsort, p_مقصد->isOrdered>=0 ? p_مقصد->isOrdered+'0' : '?');
          }
#پایان_اگر
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        p_مقصد->ماسک_Loop = pFrom->ماسک_Loop | pWLoop->ماسک_Self;
        p_مقصد->revLoop = rev_ماسک;
        p_مقصد->n_ردیف = n_بیرون;
        p_مقصد->r_هزینه = r_هزینه;
        p_مقصد->rUnsort = rUnsort;
        p_مقصد->isOrdered = isOrdered;
        کپی_حافظه(p_مقصد->aLoop, pFrom->aLoop, حجم(WhereLoop*)*iLoop);
        p_مقصد->aLoop[iLoop] = pWLoop;
        إذا( n_مقصد>=mxChoice ){
          mxI = 0;
          mx_هزینه = a_مقصد[0].r_هزینه;
          mxUnsort = a_مقصد[0].n_ردیف;
          لكل(jj=1, p_مقصد=&a_مقصد[1]; jj<mxChoice; jj++, p_مقصد++){
            إذا( p_مقصد->r_هزینه>mx_هزینه
             || (p_مقصد->r_هزینه==mx_هزینه && p_مقصد->rUnsort>mxUnsort)
            ){
              mx_هزینه = p_مقصد->r_هزینه;
              mxUnsort = p_مقصد->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#اگر_تعریف_شده WHERETRACE_ENABLED  /* >=2 */
    إذا( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      صحيح n_انجامشده = 0;
      صحيح n_پیشرفت;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      افعل{
        n_پیشرفت = 0;
        rMin = 0x7fff;
        لكل(ii=0, p_مقصد=a_مقصد; ii<n_مقصد; ii++, p_مقصد++){
          إذا( p_مقصد->r_هزینه>rFloor && p_مقصد->r_هزینه<rMin ) rMin = p_مقصد->r_هزینه;
        }
        لكل(ii=0, p_مقصد=a_مقصد; ii<n_مقصد; ii++, p_مقصد++){
          إذا( p_مقصد->r_هزینه==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               where_مسیر_Name(p_مقصد, iLoop+1, 0), p_مقصد->r_هزینه, p_مقصد->n_ردیف,
               p_مقصد->isOrdered>=0 ? (p_مقصد->isOrdered+'0') : '?');
            إذا( p_مقصد->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", p_مقصد->revLoop);
            }وإلا{
              sqlite3DebugPrintf("\n");
            }
            n_انجامشده++;
            n_پیشرفت++;
          }
        }
        rFloor = rMin;
      }طالما( n_انجامشده<n_مقصد && n_پیشرفت>0 );
    }
#پایان_اگر

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = a_مقصد;
    a_مقصد = aFrom;
    aFrom = pFrom;
    nFrom = n_مقصد;
  }

  إذا( nFrom==0 ){
    sqlite3_Error_پیام(pParse, "no query solution");
    sqlite3_پشته_Free_NN(pParse->db, pSpace);
    أرجع SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  أكد( nFrom==1 );
  pFrom = aFrom;

  أكد( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  لكل(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && n_ردیف_Est
  ){
    Bitmask not_استفاده_شده;
    صحيح rc = where_مسیر_Satisfies_ترتیب_By(pWInfo, pWInfo->p_نتیجه_مجموعه, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &not_استفاده_شده);
    إذا( rc==pWInfo->p_نتیجه_مجموعه->n_عبارت ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  إذا( pWInfo->p_ترتیب_By ){
    pWInfo->nOBSat = pFrom->isOrdered;
    إذا( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      إذا( pFrom->isOrdered==pWInfo->p_ترتیب_By->n_عبارت ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      أكد( pWInfo->pSelect->p_ترتیب_By==0
           || pWInfo->nOBSat <= pWInfo->pSelect->p_ترتیب_By->n_عبارت );
    }وإلا{
      pWInfo->rev_ماسک = pFrom->revLoop;
      إذا( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        إذا( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          إذا( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_ستونی_درون))!=(WHERE_IPK|WHERE_ستونی_درون)
          ){
            Bitmask m = 0;
            صحيح rc = where_مسیر_Satisfies_ترتیب_By(pWInfo, pWInfo->p_ترتیب_By, pFrom,
                      WHERE_ORDERBY_حد, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_ستونی_درون );
            إذا( rc==pWInfo->p_ترتیب_By->n_عبارت ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->rev_ماسک = m;
            }
          }
        }
      }وإلا إذا( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    إذا( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->p_ترتیب_By->n_عبارت && nLoop>0
    ){
      Bitmask rev_ماسک = 0;
      صحيح n_ترتیب = where_مسیر_Satisfies_ترتیب_By(pWInfo, pWInfo->p_ترتیب_By,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &rev_ماسک
      );
      أكد( pWInfo->sorted==0 );
      إذا( n_ترتیب==pWInfo->p_ترتیب_By->n_عبارت ){
        pWInfo->sorted = 1;
        pWInfo->rev_ماسک = rev_ماسک;
      }
    }
  }

  pWInfo->n_ردیف_بیرون = pFrom->n_ردیف;
#اگر_تعریف_شده WHERETRACE_ENABLED
  pWInfo->r_مجموع_هزینه = pFrom->r_هزینه;
#پایان_اگر

  /* Free temporary memory and return success */
  sqlite3_پشته_Free_NN(pParse->db, pSpace);
  أرجع SQLITE_OK;
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
ساكن SQLITE_NOINLINE فراغ whereInterstageHeuristic(WhereInfo *pWInfo){
  صحيح i;
#اگر_تعریف_شده WHERETRACE_ENABLED
  صحيح once = 0;
#پایان_اگر
  لكل(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    إذا( p==0 ) اكسر;
    إذا( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      اكسر;
    }
    إذا( (p->wsFlags & (WHERE_ستونی_EQ|WHERE_ستونی_NULL|WHERE_ستونی_درون))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      لكل(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_بعدی_Loop){
        إذا( pLoop->iTab!=iTab ) استمر;
        إذا( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_شاخص))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          استمر;
        }
#اگر_تعریف_شده WHERETRACE_ENABLED
        إذا( sqlite3WhereTrace & 0x80 ){
          إذا( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#پایان_اگر /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }وإلا{
      اكسر;
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
ساكن صحيح whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  Src_قلم *p_قلم;
  WhereClause *pWC;
  Where_جمله *p_جمله;
  WhereLoop *pLoop;
  صحيح iCur;
  صحيح j;
  جدول *pTab;
  شاخص *pIdx;
  Where_پیمایش پیمایش;

  pWInfo = pBuilder->pWInfo;
  إذا( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) أرجع 0;
  أكد( pWInfo->pTabList->nSrc>=1 );
  p_قلم = pWInfo->pTabList->a;
  pTab = p_قلم->pSTab;
  إذا( IsVirtual(pTab) ) أرجع 0;
  إذا( p_قلم->fg.is_نمایهای_By || p_قلم->fg.not_نمایهای ){
    testcase( p_قلم->fg.is_نمایهای_By );
    testcase( p_قلم->fg.not_نمایهای );
    أرجع 0;
  }
  iCur = p_قلم->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  p_جمله = where_پیمایش_Init(&پیمایش, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  طالما( p_جمله && p_جمله->prereq_راست ) p_جمله = where_پیمایش_بعدی(&پیمایش);
  إذا( p_جمله ){
    testcase( p_جمله->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_ستونی_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = p_جمله;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->r_پیاپی = 33;  /* 33==sqlite3LogEst(10) */
  }وإلا{
    لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بعدی){
      صحيح op_ماسک;
      أكد( pLoop->aLTermSpace==pLoop->aLTerm );
      إذا( !Is_Unique_شاخص(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_کلید_ستون>Array_اندازه(pLoop->aLTermSpace)
      ) استمر;
      op_ماسک = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      لكل(j=0; j<pIdx->n_کلید_ستون; j++){
        p_جمله = where_پیمایش_Init(&پیمایش, pWC, iCur, j, op_ماسک, pIdx);
        طالما( p_جمله && p_جمله->prereq_راست ) p_جمله = where_پیمایش_بعدی(&پیمایش);
        إذا( p_جمله==0 ) اكسر;
        testcase( p_جمله->eOperator & WO_IS );
        pLoop->aLTerm[j] = p_جمله;
      }
      إذا( j!=pIdx->n_کلید_ستون ) استمر;
      pLoop->wsFlags = WHERE_ستونی_EQ|WHERE_ONEROW|WHERE_نمایهای;
      إذا( pIdx->isCovering || (p_قلم->ستون_استفاده_شده & pIdx->ستون_Not_Idxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_شاخص = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->r_پیاپی = 39;  /* 39==sqlite3LogEst(15) */
      اكسر;
    }
  }
  إذا( pLoop->wsFlags ){
    pLoop->n_بیرون = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    أكد( pWInfo->s_ماسک_مجموعه.n==1 && iCur==pWInfo->s_ماسک_مجموعه.ix[0] );
    pLoop->ماسک_Self = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->n_ردیف_بیرون = 1;
    إذا( pWInfo->p_ترتیب_By ) pWInfo->nOBSat =  pWInfo->p_ترتیب_By->n_عبارت;
    إذا( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    إذا( پیمایش.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#اگر_تعریف_شده SQLITE_DEBUG
    pLoop->cId = '0';
#پایان_اگر
#اگر_تعریف_شده WHERETRACE_ENABLED
    إذا( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#پایان_اگر
    أرجع 1;
  }
  أرجع 0;
}

/*
** Helper function for exprIsDeterministic().
*/
ساكن صحيح عبارت_گره_Is_Deterministic(Walker *pWalker, عبارت *p_عبارت){
  إذا( p_عبارت->op==TK_FUNCTION && عبارت_Has_Property(p_عبارت, EP_ConstFunc)==0 ){
    pWalker->e_کد = 0;
    أرجع WRC_Abort;
  }
  أرجع WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
ساكن صحيح عبارت_Is_Deterministic(عبارت *p){
  Walker w;
  memset(&w, 0, حجم(w));
  w.e_کد = 1;
  w.x_عبارت_Callback = عبارت_گره_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.e_کد;
}

 
#اگر_تعریف_شده WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
ساكن فراغ showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  إذا( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    صحيح i;
    ساكن ثابت حرف zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    لكل(p=pWInfo->pLoops, i=0; p; p=p->p_بعدی_Loop, i++){
      p->cId = zLabel[i%(حجم(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#وإلا
# define WHERETRACE_ALL_LOOPS(W,C)
#پایان_اگر

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
ساكن SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  صحيح i;
  Bitmask tab_استفاده_شده;
  صحيح has_راست_Join;

  /* Preconditions checked by the caller */
  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  أكد( pWInfo->p_نتیجه_مجموعه!=0 );
  أكد( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tab_استفاده_شده = sqlite3_Where_عبارت_List_Usage(&pWInfo->s_ماسک_مجموعه, pWInfo->p_نتیجه_مجموعه);
  إذا( pWInfo->p_ترتیب_By ){
    tab_استفاده_شده |= sqlite3_Where_عبارت_List_Usage(&pWInfo->s_ماسک_مجموعه, pWInfo->p_ترتیب_By);
  }
  has_راست_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  لكل(i=pWInfo->nLevel-1; i>=1; i--){
    Where_جمله *p_جمله, *p_انتها;
    Src_قلم *p_قلم;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    p_قلم = &pWInfo->pTabList->a[pLoop->iTab];
    إذا( (p_قلم->fg.jointype & (JT_چپ|JT_راست))!=JT_چپ ) استمر;
    إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      استمر;
    }
    إذا( (tab_استفاده_شده & pLoop->ماسک_Self)!=0 ) استمر;
    p_انتها = pWInfo->sWC.a + pWInfo->sWC.n_جمله;
    لكل(p_جمله=pWInfo->sWC.a; p_جمله<p_انتها; p_جمله++){
      إذا( (p_جمله->prereqAll & pLoop->ماسک_Self)!=0 ){
        إذا( !عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON)
         || p_جمله->p_عبارت->w.iJoin!=p_قلم->iCursor
        ){
          اكسر;
        }
      }
      إذا( has_راست_Join
       && عبارت_Has_Property(p_جمله->p_عبارت, EP_InnerON)
       && NEVER(p_جمله->p_عبارت->w.iJoin==p_قلم->iCursor)
      ){
        اكسر;  /* restriction (5) */
      }
    }
    إذا( p_جمله<p_انتها ) استمر;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->rev_ماسک>>1) & ~m1)!=0 );
    pWInfo->rev_ماسک = (m1 & pWInfo->rev_ماسک) | ((pWInfo->rev_ماسک>>1) & ~m1);
    notReady &= ~pLoop->ماسک_Self;
    لكل(p_جمله=pWInfo->sWC.a; p_جمله<p_انتها; p_جمله++){
      إذا( (p_جمله->prereqAll & pLoop->ماسک_Self)!=0 ){
        p_جمله->wtFlags |= جمله_CODED;
        p_جمله->prereqAll = 0;
      }
    }
    إذا( i!=pWInfo->nLevel-1 ){
      صحيح nByte = (pWInfo->nLevel-1-i) * حجم(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    أكد( pWInfo->nLevel>0 );
  }
  أرجع notReady;
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
ساكن SQLITE_NOINLINE فراغ whereCheckIfBloomFilterIsUseful(
  ثابت WhereInfo *pWInfo
){
  صحيح i;
  LogEst n_جستجو = 0;

  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  لكل(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ثابت بیعلامت صحيح reqFlags = (WHERE_SELFCULL|WHERE_ستونی_EQ);
    Src_قلم *p_قلم = &pWInfo->pTabList->a[pLoop->iTab];
    جدول *pTab = p_قلم->pSTab;
    إذا( (pTab->tabFlags & TF_HasStat1)==0 ) اكسر;
    pTab->tabFlags |= TF_MaybeReanalyze;
    إذا( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_جستجو > pTab->n_ردیف_Log_Est
    ){
      testcase( p_قلم->fg.jointype & JT_چپ );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (مزدوج)sqlite3_Log_Est_مقصد_Int(n_جستجو), pTab->zName,
         (مزدوج)sqlite3_Log_Est_مقصد_Int(pTab->n_ردیف_Log_Est)));
    }
    n_جستجو += pLoop->n_بیرون;
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
ساكن SQLITE_NOINLINE فراغ where_جمع_کن_نمایهای_عبارت(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  شاخص *pIdx,       /* The index-on-expression that contains the expressions */
  صحيح iIdxCur,       /* Cursor number for pIdx */
  Src_قلم *p_Tab_قلم  /* The FROM clause entry for the table */
){
  صحيح i;
  نمایهای_عبارت *p;
  جدول *pTab;
  أكد( pIdx->b_Has_عبارت );
  pTab = pIdx->p_جدول;
  لكل(i=0; i<pIdx->n_ستونی; i++){
    عبارت *p_عبارت;
    صحيح j = pIdx->ai_ستونی[i];
    إذا( j==XN_عبارت ){
      p_عبارت = pIdx->a_ستون_عبارت->a[i].p_عبارت;
    }وإلا إذا( j>=0 && (pTab->a_ستون[j].ستون_Flags & COLFLAG_VIRTUAL)!=0 ){
      p_عبارت = sqlite3_ستونی_عبارت(pTab, &pTab->a_ستون[j]);
    }وإلا{
      استمر;
    }
    إذا( sqlite3_عبارت_Is_Constant(0,p_عبارت) ) استمر;
    p = sqlite3DbMallocRaw(pParse->db,  حجم(نمایهای_عبارت));
    إذا( p==0 ) اكسر;
    p->pIENext = pParse->pIdxEpr;
#اگر_تعریف_شده WHERETRACE_ENABLED
    إذا( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      إذا( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_عبارت(p_عبارت);
    }
#پایان_اگر
    p->p_عبارت = sqlite3_عبارت_Dup(pParse->db, p_عبارت, 0);
    p->i_داده_Cur = p_Tab_قلم->iCursor;
    p->iIdxCur = iIdxCur;
    p->i_Idx_ستون = i;
    p->b_Maybe_Null_ردیف = (p_Tab_قلم->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست))!=0;
    إذا( sqlite3_شاخص_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->z_ستون_Aff[i];
    }
#اگر_تعریف_شده SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#پایان_اگر
    pParse->pIdxEpr = p;
    إذا( p->pIENext==0 ){
      فراغ *pArg = (فراغ*)&pParse->pIdxEpr;
      sqlite3_Parser_جمع_کن_Cleanup(pParse, where_نمایهای_عبارت_Cleanup, pArg);
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
ساكن SQLITE_NOINLINE فراغ where_معکوس_پیمایش_ترتیب(WhereInfo *pWInfo){
  صحيح ii;
  لكل(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    Src_قلم *p_قلم = &pWInfo->pTabList->a[ii];
    إذا( !p_قلم->fg.isCte
     || p_قلم->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(p_قلم->fg.isSubquery==0)
     || p_قلم->u4.pSubq->pSelect->p_ترتیب_By==0
    ){
      pWInfo->rev_ماسک |= MASKBIT(ii);
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
  عبارت_List *p_ترتیب_By,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  عبارت_List *p_نتیجه_مجموعه,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  صحيح iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  صحيح nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  صحيح nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  Where_ماسک_مجموعه *p_ماسک_مجموعه;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  صحيح ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  صحيح rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  أكد( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  أكد( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_حد)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, حجم(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( p_ترتیب_By && p_ترتیب_By->n_عبارت==BMS-1 );
  إذا( p_ترتیب_By && p_ترتیب_By->n_عبارت>=BMS ){
    p_ترتیب_By = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  إذا( pTabList->nSrc>BMS ){
    sqlite3_Error_پیام(pParse, "at most %d tables in a join", BMS);
    أرجع 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + حجم(WhereLoop));
  إذا( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    اذهب whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->p_ترتیب_By = p_ترتیب_By;
#إذا WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#پایان_اگر
  pWInfo->p_نتیجه_مجموعه = p_نتیجه_مجموعه;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->i_حد = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  memset(&pWInfo->a[0], 0, حجم(WhereLoop)+nTabList*حجم(WhereLevel));
  أكد( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  p_ماسک_مجموعه = &pWInfo->s_ماسک_مجموعه;
  p_ماسک_مجموعه->n = 0;
  p_ماسک_مجموعه->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((حرف*)pWInfo)+nByteWInfo);
  أكد( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#اگر_تعریف_شده SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#پایان_اگر

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3_Where_تقسیم(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  إذا( nTabList==0 ){
    إذا( p_ترتیب_By ) pWInfo->nOBSat = p_ترتیب_By->n_عبارت;
    إذا( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    إذا( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_مقدار)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }وإلا{
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
    افعل{
      create_ماسک(p_ماسک_مجموعه, pTabList->a[ii].iCursor);
      sqlite3_Where_Tab_Func_آرگومانها(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }طالما( (++ii)<pTabList->nSrc );
  #اگر_تعریف_شده SQLITE_DEBUG
    {
      Bitmask mx = 0;
      لكل(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3_Where_Get_ماسک(p_ماسک_مجموعه, pTabList->a[ii].iCursor);
        أكد( m>=mx );
        mx = m;
      }
    }
  #پایان_اگر
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_عبارت_Analyze(pTabList, &pWInfo->sWC);
  إذا( pSelect && pSelect->p_حد ){
    sqlite3_Where_جمع_کن_حد(&pWInfo->sWC, pSelect);
  }
  إذا( pParse->nErr ) اذهب whereBeginError;

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
  لكل(ii=0; ii<sWLB.pWC->n_پایه; ii++){
    Where_جمله *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    عبارت *pX;                          /* The expression of pT */
    إذا( pT->wtFlags & جمله_VIRTUAL ) استمر;
    pX = pT->p_عبارت;
    أكد( pX!=0 );
    أكد( pT->prereqAll!=0 || !عبارت_Has_Property(pX, EP_OuterON) );
    إذا( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || عبارت_Is_Deterministic(pX))   /* Condition (4) */
     && !(عبارت_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_عبارت_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= جمله_CODED;
    }
  }

  إذا( wctrlFlags & WHERE_WANT_DISTINCT ){
    إذا( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }وإلا إذا( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_نتیجه_مجموعه) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }وإلا إذا( p_ترتیب_By==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->p_ترتیب_By = p_نتیجه_مجموعه;
    }
  }

  /* Construct the WhereLoop objects */
#إذا defined(WHERETRACE_ENABLED)
  إذا( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    إذا( wctrlFlags & WHERE_USE_حد ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    إذا( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, حجم(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.p_ترتیب_By = p_ترتیب_By;
      sSelect.pEList = p_نتیجه_مجموعه;
      sqlite3_درخت_View_Select(0, &sSelect, 0);
    }
    إذا( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#پایان_اگر

  إذا( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = where_Loop_جمع_کن_All(&sWLB);
    إذا( rc ) اذهب whereBeginError;

#اگر_تعریف_شده SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    إذا( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      طالما( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_بعدی_Loop;
        whereLoopDelete(db, p);
      }
      rc = where_Loop_جمع_کن_All(&sWLB);
      إذا( rc ) اذهب whereBeginError;
    }
#پایان_اگر
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    where_مسیر_Solver(pWInfo, 0);
    إذا( db->mallocFailed ) اذهب whereBeginError;
    إذا( pWInfo->p_ترتیب_By ){
       whereInterstageHeuristic(pWInfo);
       where_مسیر_Solver(pWInfo, pWInfo->n_ردیف_بیرون<0 ? 1 : pWInfo->n_ردیف_بیرون+1);
       إذا( db->mallocFailed ) اذهب whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->n_ردیف_بیرون, pWInfo->n_ردیف_بیرون-30));
      pWInfo->n_ردیف_بیرون -= 30;
    }

  }
  أكد( pWInfo->pTabList!=0 );
  إذا( pWInfo->p_ترتیب_By==0 && (db->flags & SQLITE_معکوس_ترتیب)!=0 ){
    where_معکوس_پیمایش_ترتیب(pWInfo);
  }
  إذا( pParse->nErr ){
    اذهب whereBeginError;
  }
  أكد( db->mallocFailed==0 );
#اگر_تعریف_شده WHERETRACE_ENABLED
  إذا( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_مجموع_هزینه, pWInfo->n_ردیف_بیرون);
    إذا( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->rev_ماسک);
    }
    بدل( pWInfo->eDistinct ){
      حالة WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        اكسر;
      }
      حالة WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        اكسر;
      }
      حالة WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        اكسر;
      }
    }
    sqlite3DebugPrintf("\n");
    لكل(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#پایان_اگر

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
  إذا( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && p_نتیجه_مجموعه!=0           /* Condition (1) */
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
  إذا( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#إذا defined(WHERETRACE_ENABLED)
  إذا( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
    sqlite3DebugPrintf("---- WHERE clause at end of analysis:\n");
    sqlite3WhereClausePrint(sWLB.pWC);
  }
  WHERETRACE(0xffffffff,("*** Optimizer Finished ***\n"));
#پایان_اگر
  pWInfo->pParse->nQueryLoop += pWInfo->n_ردیف_بیرون;

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
  إذا( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    صحيح wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    صحيح bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    أكد( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    إذا( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      إذا( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        إذا( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  لكل(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    جدول *pTab;     /* Table to open */
    صحيح iDb;         /* Index of database containing table/index */
    Src_قلم *p_Tab_قلم;

    p_Tab_قلم = &pTabList->a[pLevel->iFrom];
    pTab = p_Tab_قلم->pSTab;
    iDb = sqlite3_Schema_مقصد_شاخص(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    إذا( ii==0 || (p_Tab_قلم[0].fg.jointype & JT_چپ)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }وإلا إذا( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }وإلا{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    إذا( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }وإلا
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
    إذا( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ثابت حرف *pVTab = (ثابت حرف *)sqlite3GetVTable(db, pTab);
      صحيح iCur = p_Tab_قلم->iCursor;
      sqlite3_Vdbe_جمع_کن_Op4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }وإلا إذا( IsVirtual(pTab) ){
      /* noop */
    }وإلا
#پایان_اگر
    إذا( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (p_Tab_قلم->fg.jointype & (JT_LTORJ|JT_راست))!=0
    ){
      صحيح op = OP_OpenRead;
      إذا( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = p_Tab_قلم->iCursor;
      };
      sqlite3_Open_جدول(pParse, p_Tab_قلم->iCursor, iDb, pTab, op);
      أكد( p_Tab_قلم->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_ستون==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_ستون==BMS );
      إذا( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->n_ستون<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_شاخص|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = p_Tab_قلم->ستون_استفاده_شده;
        صحيح n = 0;
        لكل(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_مقصد_PTR(n), P4_INT32);
        أكد( n<=pTab->n_ستون );
      }
#اگر_تعریف_شده SQLITE_ENABLE_CURSOR_HINTS
      إذا( pLoop->u.btree.p_شاخص!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }وإلا
#پایان_اگر
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#اگر_تعریف_شده SQLITE_ENABLE_ستونی_استفاده_شده_ماسک
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Columns_استفاده_شده, p_Tab_قلم->iCursor,
                    LOWER32(p_Tab_قلم->ستون_استفاده_شده), UPPER32(p_Tab_قلم->ستون_استفاده_شده));
#پایان_اگر
      إذا( ii>=2
       && (p_Tab_قلم[0].fg.jointype & (JT_LTORJ|JT_چپ))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_If_خالی, p_Tab_قلم->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }وإلا{
      sqlite3_جدول_قفل(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    إذا( pLoop->wsFlags & WHERE_نمایهای ){
      شاخص *pIx = pLoop->u.btree.p_شاخص;
      صحيح i_شاخص_Cur;
      صحيح op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      أكد( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      إذا( !HasRowid(pTab) && Is_Primary_کلید_شاخص(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_شاخص_Cur = pLevel->iTabCur;
        op = 0;
      }وإلا إذا( pWInfo->eOnePass!=ONEPASS_OFF ){
        شاخص *pJ = p_Tab_قلم->pSTab->p_شاخص;
        i_شاخص_Cur = iAuxArg;
        أكد( wctrlFlags & WHERE_ONEPASS_DESIRED );
        طالما( ALWAYS(pJ) && pJ!=pIx ){
          i_شاخص_Cur++;
          pJ = pJ->p_بعدی;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_شاخص_Cur;
      }وإلا إذا( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_شاخص_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }وإلا{
        i_شاخص_Cur = pParse->nTab++;
        إذا( pIx->b_Has_عبارت && OptimizationEnabled(db, SQLITE_نمایهای_عبارت) ){
          where_جمع_کن_نمایهای_عبارت(pParse, pIx, i_شاخص_Cur, p_Tab_قلم);
        }
        إذا( pIx->pPartIdxWhere && (p_Tab_قلم->fg.jointype & JT_راست)==0 ){
          where_Part_Idx_عبارت(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_شاخص_Cur, p_Tab_قلم
          );
        }
      }
      pLevel->iIdxCur = i_شاخص_Cur;
      أكد( pIx!=0 );
      أكد( pIx->pSchema==pTab->pSchema );
      أكد( i_شاخص_Cur>=0 );
      إذا( op ){
        sqlite3_Vdbe_جمع_کن_Op3(v, op, i_شاخص_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_مجموعه_P4_کلید_Info(pParse, pIx);
        إذا( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_ستونی_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_مرتب_سازی)==0
         && (pLoop->wsFlags & WHERE_درون_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#اگر_تعریف_شده SQLITE_ENABLE_ستونی_استفاده_شده_ماسک
        {
          u64 ستون_استفاده_شده = 0;
          صحيح ii, jj;
          لكل(ii=0; ii<pIx->n_ستونی; ii++){
            jj = pIx->ai_ستونی[ii];
            إذا( jj<0 ) استمر;
            إذا( jj>63 ) jj = 63;
            إذا( (p_Tab_قلم->ستون_استفاده_شده & MASKBIT(jj))==0 ) استمر;
            ستون_استفاده_شده |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_Columns_استفاده_شده, i_شاخص_Cur,
                            LOWER32(ستون_استفاده_شده),UPPER32(ستون_استفاده_شده));
        }
#پایان_اگر /* SQLITE_ENABLE_COLUMN_USED_MASK */
#اگر_تعریف_شده SQLITE_ENABLE_CURSOR_HINTS
        إذا( HasRowid(pTab) ){
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_CursorHint, i_شاخص_Cur, 0, p_Tab_قلم->iCursor);
        }
#پایان_اگر
      }
    }
    إذا( iDb>=0 ) sqlite3_کد_Verify_Schema(pParse, iDb);
    إذا( (p_Tab_قلم->fg.jointype & JT_راست)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, حجم(Where_راست_Join)))!=0
    ){
      Where_راست_Join *pRJ = pLevel->pRJ;
      صحيح bBloomable = 0;
      pRJ->i_تطابق = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, pRJ->regReturn);
      أكد( pTab==p_Tab_قلم->pSTab );
      إذا( HasRowid(pTab) ){
        کلید_Info *pInfo;
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_OpenEphemeral, pRJ->i_تطابق, 1);
        bBloomable = 1;
        pInfo = sqlite3_کلید_Info_Alloc(pParse->db, 1, 0);
        إذا( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->a_مرتب_سازی_Flags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }وإلا{
        شاخص *pPk = sqlite3_Primary_کلید_شاخص(pTab);
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_OpenEphemeral, pRJ->i_تطابق, pPk->n_کلید_ستون);
        sqlite3_Vdbe_مجموعه_P4_کلید_Info(pParse, pPk);
        bBloomable = sqlite3_شاخص_Bloomable(pPk,pPk->n_کلید_ستون);
      }
      إذا( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Blob, 65536, pRJ->regBloom);
      }وإلا{
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
  إذا( db->mallocFailed ) اذهب whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  لكل(ii=0; ii<nTabList; ii++){
    صحيح addrExplain;
    صحيح wsFlags;
    Src_قلم *pSrc;
    إذا( pParse->nErr ) اذهب whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    إذا( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      صحيح iOnce = 0;
      أكد( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      إذا( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
      }وإلا{
        iOnce = 0;
      }
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      إذا( iOnce )  sqlite3_Vdbe_Jump_اینجا(v, iOnce);
    }
    أكد( pTabList == pWInfo->pTabList );
    إذا( (wsFlags & (WHERE_AUTO_شاخص|WHERE_BLOOMFILTER))!=0 ){
      إذا( (wsFlags & WHERE_AUTO_شاخص)!=0 ){
#اگر_تعریف_نشده SQLITE_OMIT_AUTOMATIC_شاخص
        construct_Automatic_شاخص(pParse, &pWInfo->sWC, notReady, pLevel);
#پایان_اگر
      }وإلا{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      إذا( db->mallocFailed ) اذهب whereBeginError;
    }
    addrExplain = sqlite3_Where_Explain_One_پیمایش(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3_Where_کد_One_Loop_شروع(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    إذا( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3_Where_جمع_کن_پیمایش_وضعیت(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->i_انتها_Where = sqlite3VdbeCurrentAddr(v);
  أرجع pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  إذا( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#اگر_تعریف_شده WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#پایان_اگر /* WHERETRACE_ENABLED */
  أرجع 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#اگر_تعریف_نشده SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#وإلا
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  ساكن فراغ sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    صحيح pc,
    VdbeOp *pOp
  ){
    إذا( (db->flags & SQLITE_VdbeAddopTrace)==0 ) أرجع;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3_Show_Where_جمله(0); /* So compiler won't complain about unused func */
  }
#پایان_اگر

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
فراغ sqlite3_Where_انتها(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  صحيح i_انتها = sqlite3VdbeCurrentAddr(v);
  صحيح nRJ = 0;
#اگر_تعریف_نشده SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  صحيح addrSeek = 0;
#پایان_اگر

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  لكل(i=pWInfo->nLevel-1; i>=0; i--){
    صحيح addr;
    pLevel = &pWInfo->a[i];
    إذا( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_راست_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->انتها_Subrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    إذا( pLevel->op!=OP_Noop ){
#اگر_تعریف_نشده SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      شاخص *pIdx;
      صحيح n;
      إذا( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_نمایهای)!=0
       && (pIdx = pLoop->u.btree.p_شاخص)->hasStat1
       && (n = pLoop->u.btree.n_Distinct_ستون)>0
       && pIdx->ai_ردیف_Log_Est[n]>=36
      ){
        صحيح r1 = pParse->nMem+1;
        صحيح j, op;
        صحيح addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        إذا( pLevel->i_چپ_Join ){
          addrIfNull = sqlite3_Vdbe_جمع_کن_Op2(v, OP_If_Null_ردیف, pLevel->iIdxCur, r1);
        }
        لكل(j=0; j<n; j++){
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3_Vdbe_جمع_کن_Op4_Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 1, pLevel->p2);
        إذا( pLevel->i_چپ_Join ){
          sqlite3_Vdbe_Jump_اینجا(v, addrIfNull);
        }
      }
#پایان_اگر /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    إذا( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }وإلا إذا( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      صحيح iBypass = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Goto);
      أكد( pLoop->wsFlags & WHERE_AUTO_شاخص );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3_Vdbe_جمع_کن_Op1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3_Vdbe_Jump_اینجا(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    إذا( pLevel->op!=OP_Noop ){
      sqlite3_Vdbe_جمع_کن_Op3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_بعدی);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#اگر_تعریف_نشده SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      إذا( addrSeek ){
        sqlite3_Vdbe_Jump_اینجا(v, addrSeek);
        addrSeek = 0;
      }
#پایان_اگر
      إذا( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    إذا( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#اگر_تعریف_نشده SQLITE_LIKE_DOESNT_تطابق_BLOBS
    أكد( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_درون_ABLE) && pLevel->u.درون.n_درون>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    إذا( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_DecrJumpZero, (صحيح)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#پایان_اگر
    إذا( (pLoop->wsFlags & WHERE_درون_ABLE)!=0 && pLevel->u.درون.n_درون>0 ){
      بنية درون_Loop *p_درون;
      صحيح j;
      لكل(j=pLevel->u.درون.n_درون, p_درون=&pLevel->u.درون.a_درون_Loop[j-1]; j>0; j--, p_درون--){
        أكد( sqlite3VdbeGetOp(v, p_درون->addr_درون_بالاترین+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3_Vdbe_Jump_اینجا(v, p_درون->addr_درون_بالاترین+1);
        إذا( p_درون->e_انتها_Loop_Op!=OP_Noop ){
          إذا( p_درون->n_پیشوند ){
            صحيح b_Early_بیرون =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_درون_EARLYOUT)!=0;
            إذا( pLevel->i_چپ_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3_Vdbe_جمع_کن_Op3(v, OP_If_Null_ردیف, p_درون->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + b_Early_بیرون, p_درون->i_پایه);
              VdbeCoverage(v);
            }
            إذا( b_Early_بیرون ){
              sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  p_درون->i_پایه, p_درون->n_پیشوند);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3_Vdbe_Jump_اینجا(v, p_درون->addr_درون_بالاترین+1);
            }
          }
          sqlite3_Vdbe_جمع_کن_Op2(v, p_درون->e_انتها_Loop_Op, p_درون->iCur, p_درون->addr_درون_بالاترین);
          VdbeCoverage(v);
          VdbeCoverageIf(v, p_درون->e_انتها_Loop_Op==OP_Prev);
          VdbeCoverageIf(v, p_درون->e_انتها_Loop_Op==OP_بعدی);
        }
        sqlite3_Vdbe_Jump_اینجا(v, p_درون->addr_درون_بالاترین-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    إذا( pLevel->pRJ ){
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    إذا( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_شاخص->zName));
      sqlite3_Vdbe_Jump_اینجا(v, pLevel->addrSkip);
      sqlite3_Vdbe_Jump_اینجا(v, pLevel->addrSkip-2);
    }
    إذا( pLevel->i_چپ_Join ){
      صحيح ws = pLoop->wsFlags;
      addr = sqlite3_Vdbe_جمع_کن_Op1(v, OP_IfPos, pLevel->i_چپ_Join); VdbeCoverage(v);
      أكد( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_نمایهای)!=0 );
      إذا( (ws & WHERE_IDX_ONLY)==0 ){
        Src_قلم *pSrc = &pTabList->a[pLevel->iFrom];
        أكد( pLevel->iTabCur==pSrc->iCursor );
        إذا( pSrc->fg.از_طریق_Coroutine ){
          صحيح m, n;
          أكد( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_نتیجه;
          أكد( pSrc->pSTab!=0 );
          m = pSrc->pSTab->n_ستون;
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, pLevel->iTabCur);
      }
      إذا( (ws & WHERE_نمایهای)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        إذا( ws & WHERE_MULTI_OR ){
          شاخص *pIx = pLevel->u.pCoveringIdx;
          صحيح iDb = sqlite3_Schema_مقصد_شاخص(db, pIx->pSchema);
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_مجموعه_P4_کلید_Info(pParse, pIx);
        }
        sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, pLevel->iIdxCur);
      }
      إذا( ws & WHERE_درون_ABLE ){
        صحيح m;
        لكل(m=0; m<pLevel->u.درون.n_درون; m++){
          بنية درون_Loop *p_درون = &pLevel->u.درون.a_درون_Loop[m];
          إذا( p_درون->e_انتها_Loop_Op!=OP_Noop ){
            sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, p_درون->iCur);
          }
        }
      }
      إذا( pLevel->op==OP_Return ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Gosub, pLevel->p1, pLevel->addr_اولی);
      }وإلا{
        sqlite3VdbeGoto(v, pLevel->addr_اولی);
      }
      sqlite3_Vdbe_Jump_اینجا(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  أكد( pWInfo->nLevel<=pTabList->nSrc );
  لكل(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    صحيح k, آخرین_2;
    VdbeOp *pOp, *p_آخرین_Op;
    شاخص *pIdx = 0;
    Src_قلم *p_Tab_قلم = &pTabList->a[pLevel->iFrom];
    جدول *pTab = p_Tab_قلم->pSTab;
    أكد( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    إذا( pLevel->pRJ ){
      sqlite3_Where_راست_Join_Loop(pWInfo, i, pLevel);
      استمر;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    إذا( p_Tab_قلم->fg.از_طریق_Coroutine ){
      testcase( pParse->db->mallocFailed );
      أكد( p_Tab_قلم->fg.isSubquery );
      أكد( p_Tab_قلم->u4.pSubq->reg_نتیجه>=0 );
      translate_ستونی_مقصد_Copy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            p_Tab_قلم->u4.pSubq->reg_نتیجه, 0);
      استمر;
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
    إذا( pLoop->wsFlags & (WHERE_نمایهای|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_شاخص;
    }وإلا إذا( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    إذا( pIdx
     && !db->mallocFailed
    ){
      إذا( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->p_جدول) ){
        آخرین_2 = i_انتها;
      }وإلا{
        آخرین_2 = pWInfo->i_انتها_Where;
      }
      إذا( pIdx->b_Has_عبارت ){
        نمایهای_عبارت *p = pParse->pIdxEpr;
        طالما( p ){
          إذا( p->iIdxCur==pLevel->iIdxCur ){
#اگر_تعریف_شده WHERETRACE_ENABLED
            إذا( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->i_Idx_ستون);
              إذا( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_عبارت(p->p_عبارت);
            }
#پایان_اگر
            p->i_داده_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#اگر_تعریف_شده SQLITE_DEBUG
      إذا( db->flags & SQLITE_VdbeAddopTrace ){
        چاپ_کن("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, آخرین_2-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      أكد( pOp->opcode!=OP_ستونی || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_If_Null_ردیف || pOp->p1!=pLevel->iTabCur );
#پایان_اگر
      pOp = sqlite3VdbeGetOp(v, k);
      p_آخرین_Op = pOp + (آخرین_2 - k);
      أكد( pOp<=p_آخرین_Op );
      افعل{
        إذا( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }وإلا إذا( pOp->opcode==OP_ستونی
#اگر_تعریف_شده SQLITE_ENABLE_جابه_جایی_SQL_FUNC
         || pOp->opcode==OP_جابه_جایی
#پایان_اگر
        ){
          صحيح x = pOp->p2;
          أكد( pIdx->p_جدول==pTab );
#اگر_تعریف_شده SQLITE_ENABLE_جابه_جایی_SQL_FUNC
          إذا( pOp->opcode==OP_جابه_جایی ){
            /* Do not need to translate the column number */
          }وإلا
#پایان_اگر
          إذا( !HasRowid(pTab) ){
            شاخص *pPk = sqlite3_Primary_کلید_شاخص(pTab);
            x = pPk->ai_ستونی[x];
            أكد( x>=0 );
          }وإلا{
            testcase( x!=sqlite3_Storage_ستونی_مقصد_جدول(pTab,x) );
            x = sqlite3_Storage_ستونی_مقصد_جدول(pTab,x);
          }
          x = sqlite3_جدول_ستونی_مقصد_شاخص(pIdx, x);
          إذا( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }وإلا إذا( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            إذا( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_پیام(pParse, "internal query planner error");
              pParse->rc = SQLITE_داخلی;
            }وإلا{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3_Where_جمع_کن_Explain_متن(pParse,
                  pLevel->addrBody-1,
                  pTabList,
                  pLevel,
                  pWInfo->wctrlFlags
              );
            }
          }
        }وإلا إذا( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }وإلا إذا( pOp->opcode==OP_If_Null_ردیف ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#اگر_تعریف_شده SQLITE_DEBUG
        k++;
#پایان_اگر
      }طالما( (++pOp)<p_آخرین_Op );
#اگر_تعریف_شده SQLITE_DEBUG
      إذا( db->flags & SQLITE_VdbeAddopTrace ) چاپ_کن("TRANSLATE complete\n");
#پایان_اگر
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
  أرجع;
}
