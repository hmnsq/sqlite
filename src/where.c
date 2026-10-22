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
  u32 m_دننه;                 /* Mask of terms that are <col> IN (...) */
  u32 m_Handle_دننه;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_ارزښت *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#تعریف_کړه SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_شاخص_Info,aRhs) + (N)*حجم(sqlite3_ارزښت*))

/* Forward declaration of methods */
ساكن صحيح whereLoopResize(sqlite3*, WhereLoop*, صحيح);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_لیکه_شمېر(WhereInfo *pWInfo){
  أرجع pWInfo->n_لیکه_بهر;
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
فراغ sqlite3_Where_Min_Max_Opt_Early_بهر(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  صحيح i;
  إذا( !pWInfo->bOrderedInnerLoop ) أرجع;
  إذا( pWInfo->nOBSat==0 ) أرجع;
  لكل(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    إذا( (pInner->pWLoop->wsFlags & WHERE_ستنه_دننه)!=0 ){
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
  حافظه_نقل(aiCur, pWInfo->aiCurOnePass, حجم(صحيح)*2);
#که_تعریف_شوی WHERETRACE_ENABLED
  إذا( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#د_که_پای
  أرجع pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
صحيح sqlite3_Where_کاروي_Deferred_Seek(WhereInfo *pWInfo){
  أرجع pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
ساكن فراغ whereOrMove(Where_Or_سېټ *pDest, Where_Or_سېټ *pSrc){
  pDest->n = pSrc->n;
  حافظه_نقل(pDest->a, pSrc->a, pDest->n*حجم(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
ساكن صحيح where_Or_ورننباسه(
  Where_Or_سېټ *p_سېټ,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst r_پرلپسې,           /* Run-cost of the new entry */
  LogEst n_بهر            /* Number of outputs for the new entry */
){
  u16 i;
  Where_Or_لګښت *p;
  لكل(i=p_سېټ->n, p=p_سېټ->a; i>0; i--, p++){
    إذا( r_پرلپسې<=p->r_پرلپسې && (prereq & p->prereq)==prereq ){
      اذهب where_Or_ورننباسه_بشپړ;
    }
    إذا( p->r_پرلپسې<=r_پرلپسې && (p->prereq & prereq)==p->prereq ){
      أرجع 0;
    }
  }
  إذا( p_سېټ->n<N_OR_لګښت ){
    p = &p_سېټ->a[p_سېټ->n++];
    p->n_بهر = n_بهر;
  }وإلا{
    p = p_سېټ->a;
    لكل(i=1; i<p_سېټ->n; i++){
      إذا( p->r_پرلپسې>p_سېټ->a[i].r_پرلپسې ) p = p_سېټ->a + i;
    }
    إذا( p->r_پرلپسې<=r_پرلپسې ) أرجع 0;
  }
where_Or_ورننباسه_بشپړ:
  p->prereq = prereq;
  p->r_پرلپسې = r_پرلپسې;
  إذا( p->n_بهر>n_بهر ) p->n_بهر = n_بهر;
  أرجع 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3_Where_Get_ماسک(Where_ماسک_سېټ *p_ماسک_سېټ, صحيح iCursor){
  صحيح i;
  أكد( p_ماسک_سېټ->n<=(صحيح)حجم(Bitmask)*8 );
  أكد( p_ماسک_سېټ->n>0 || p_ماسک_سېټ->ix[0]<0 );
  أكد( iCursor>=-1 );
  إذا( p_ماسک_سېټ->ix[0]==iCursor ){
    أرجع 1;
  }
  لكل(i=1; i<p_ماسک_سېټ->n; i++){
    إذا( p_ماسک_سېټ->ix[i]==iCursor ){
      أرجع MASKBIT(i);
    }
  }
  أرجع 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
فراغ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  Where_Mem_بلاک *p_بلاک;
  p_بلاک = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+حجم(*p_بلاک));
  إذا( p_بلاک ){
    p_بلاک->p_بل = pWInfo->p_Mem_منزل_Free;
    p_بلاک->sz = nByte;
    pWInfo->p_Mem_منزل_Free = p_بلاک;
    p_بلاک++;
  }
  أرجع (فراغ*)p_بلاک;
}
فراغ *sqlite3WhereRealloc(WhereInfo *pWInfo, فراغ *pOld, u64 nByte){
  فراغ *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  إذا( pNew && pOld ){
    Where_Mem_بلاک *pOldBlk = (Where_Mem_بلاک*)pOld;
    pOldBlk--;
    أكد( pOldBlk->sz<nByte );
    حافظه_نقل(pNew, pOld, pOldBlk->sz);
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
ساكن فراغ create_ماسک(Where_ماسک_سېټ *p_ماسک_سېټ, صحيح iCursor){
  أكد( p_ماسک_سېټ->n < Array_اندازه(p_ماسک_سېټ->ix) );
  p_ماسک_سېټ->ix[p_ماسک_سېټ->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
ساكن بیان *where_ښی_Subexpr_Is_ستنه(بیان *p){
  p = sqlite3_بیان_Skip_Collate_And_Likely(p->p_ښی);
  إذا( ALWAYS(p!=0) && p->op==TK_ستنه && !بیان_Has_Property(p, EP_Fixed_ستون) ){
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
ساكن SQLITE_NOINLINE ثابت حرف *شاخص_دننه_Affinity_Ok(
  Parse *pParse, 
  Where_غړی *p_غړی, 
  u8 idxaff
){
  بیان *pX = p_غړی->p_بیان;
  بیان inexpr;

  أكد( p_غړی->eOperator & WO_دننه );

  إذا( بیان_Use_XSelect(pX) ){
    صحيح i_ډګر = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    إذا( sqlite3_بیان_Is_ویکټور(pX->p_کیڼ) ){
      i_ډګر = p_غړی->u.x.i_ډګر - 1;
      inexpr.p_کیڼ = pX->p_کیڼ->x.pList->a[i_ډګر].p_بیان;
    }وإلا{
      inexpr.p_کیڼ = pX->p_کیڼ;
    }
    inexpr.p_ښی = pX->x.pSelect->pEList->a[i_ډګر].p_بیان;
    pX = &inexpr;
  }

  إذا( sqlite3_شاخص_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_بیان_Compare_Coll_Seq(pParse, pX);
    أرجع pRet ? pRet->zName : sqlite3_Str_دوګونی;
  }
  أرجع 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
ساكن Where_غړی *where_لټون_بل(Where_لټون *p_لټون){
  صحيح iCur;            /* The cursor on the LHS of the term */
  i16 i_ستنه;         /* The column on the LHS of the term.  -1 for IPK */
  بیان *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  Where_غړی *p_غړی;    /* The term being tested */
  صحيح k = p_لټون->k;    /* Where to start scanning */

  أكد( p_لټون->iEquiv<=p_لټون->nEquiv );
  pWC = p_لټون->pWC;
  طالما(1){
    i_ستنه = p_لټون->ai_ستنه[p_لټون->iEquiv-1];
    iCur = p_لټون->aiCur[p_لټون->iEquiv-1];
    أكد( pWC!=0 );
    أكد( iCur>=0 );
    افعل{
      لكل(p_غړی=pWC->a+k; k<pWC->n_غړی; k++, p_غړی++){
        أكد( (p_غړی->eOperator & (WO_OR|WO_AND))==0 || p_غړی->کیڼ_Cursor<0 );
        إذا( p_غړی->کیڼ_Cursor==iCur
         && p_غړی->u.x.کیڼ_ستنه==i_ستنه
         && (i_ستنه!=XN_بیان
             || sqlite3_بیان_Compare_Skip(p_غړی->p_بیان->p_کیڼ,
                                       p_لټون->p_Idx_بیان,iCur)==0)
         && (p_لټون->iEquiv<=1 || !بیان_Has_Property(p_غړی->p_بیان, EP_OuterON))
        ){
          إذا( (p_غړی->eOperator & WO_EQUIV)!=0
           && p_لټون->nEquiv<Array_اندازه(p_لټون->aiCur)
           && (pX = where_ښی_Subexpr_Is_ستنه(p_غړی->p_بیان))!=0
          ){
            صحيح j;
            لكل(j=0; j<p_لټون->nEquiv; j++){
              إذا( p_لټون->aiCur[j]==pX->i_جدول
               && p_لټون->ai_ستنه[j]==pX->i_ستنه ){
                  اكسر;
              }
            }
            إذا( j==p_لټون->nEquiv ){
              p_لټون->aiCur[j] = pX->i_جدول;
              p_لټون->ai_ستنه[j] = pX->i_ستنه;
              p_لټون->nEquiv++;
            }
          }
          إذا( (p_غړی->eOperator & p_لټون->op_ماسک)!=0 ){
            /* Verify the affinity and collating sequence match */
            إذا( p_لټون->zCollName && (p_غړی->eOperator & WO_ISNULL)==0 ){
              ثابت حرف *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = p_غړی->p_بیان;

              إذا( (p_غړی->eOperator & WO_دننه) ){
                zCollName = شاخص_دننه_Affinity_Ok(pParse, p_غړی, p_لټون->idxaff);
                إذا( !zCollName ) استمر;
              }وإلا{
                CollSeq *pColl;
                إذا( !sqlite3_شاخص_Affinity_Ok(pX, p_لټون->idxaff) ){
                  استمر;
                }
                أكد(pX->p_کیڼ);
                pColl = sqlite3_بیان_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_دوګونی;
              }

              إذا( sqlite3StrICmp(zCollName, p_لټون->zCollName) ){
                استمر;
              }
            }
            إذا( (p_غړی->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = p_غړی->p_بیان->p_ښی, ALWAYS(pX!=0))
             && pX->op==TK_ستنه
             && pX->i_جدول==p_لټون->aiCur[0]
             && pX->i_ستنه==p_لټون->ai_ستنه[0]
            ){
              testcase( p_غړی->eOperator & WO_IS );
              استمر;
            }
            p_لټون->pWC = pWC;
            p_لټون->k = k+1;
#که_تعریف_شوی WHERETRACE_ENABLED
            إذا( (sqlite3WhereTrace & 0x20000)!=0 && p_لټون->nEquiv>1 ){
              صحيح ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 p_لټون->aiCur[0], p_لټون->ai_ستنه[0], p_غړی->i_غړی);
              لكل(ii=1; ii<p_لټون->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   p_لټون->aiCur[ii], p_لټون->ai_ستنه[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#د_که_پای
            أرجع p_غړی;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }طالما( pWC!=0 );
    إذا( p_لټون->iEquiv>=p_لټون->nEquiv ) اكسر;
    pWC = p_لټون->pOrigWC;
    k = 0;
    p_لټون->iEquiv++;
  }
  أرجع 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
ساكن SQLITE_NOINLINE Where_غړی *where_لټون_Init_شاخص_بیان(Where_لټون *p_لټون){
  p_لټون->idxaff = sqlite3_بیان_Affinity(p_لټون->p_Idx_بیان);
  أرجع where_لټون_بل(p_لټون);
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
ساكن Where_غړی *where_لټون_Init(
  Where_لټون *p_لټون,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  صحيح iCur,               /* Cursor to scan for */
  صحيح i_ستنه,            /* Column to scan for */
  u32 op_ماسک,             /* Operator(s) to scan for */
  شاخص *pIdx             /* Must be compatible with this index */
){
  p_لټون->pOrigWC = pWC;
  p_لټون->pWC = pWC;
  p_لټون->p_Idx_بیان = 0;
  p_لټون->idxaff = 0;
  p_لټون->zCollName = 0;
  p_لټون->op_ماسک = op_ماسک;
  p_لټون->k = 0;
  p_لټون->aiCur[0] = iCur;
  p_لټون->nEquiv = 1;
  p_لټون->iEquiv = 1;
  إذا( pIdx ){
    صحيح j = i_ستنه;
    i_ستنه = pIdx->ai_ستنه[j];
    إذا( i_ستنه==pIdx->p_جدول->iPKey ){
      i_ستنه = XN_ROWID;
    }وإلا إذا( i_ستنه>=0 ){
      p_لټون->idxaff = pIdx->p_جدول->a_ستون[i_ستنه].affinity;
      p_لټون->zCollName = pIdx->azColl[j];
    }وإلا إذا( i_ستنه==XN_بیان ){
      p_لټون->p_Idx_بیان = pIdx->a_ستون_بیان->a[j].p_بیان;
      p_لټون->zCollName = pIdx->azColl[j];
      p_لټون->ai_ستنه[0] = XN_بیان;
      أرجع where_لټون_Init_شاخص_بیان(p_لټون);
    }
  }وإلا إذا( i_ستنه==XN_بیان ){
    أرجع 0;
  }
  p_لټون->ai_ستنه[0] = i_ستنه;
  أرجع where_لټون_بل(p_لټون);
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
Where_غړی *sqlite3_Where_پیدا_کړه_غړی(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  صحيح iCur,             /* Cursor number of LHS */
  صحيح i_ستنه,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  شاخص *pIdx           /* Must be compatible with this index, if not NULL */
){
  Where_غړی *p_پایله = 0;
  Where_غړی *p;
  Where_لټون لټون;

  p = where_لټون_Init(&لټون, pWC, iCur, i_ستنه, op, pIdx);
  op &= WO_EQ|WO_IS;
  طالما( p ){
    إذا( (p->prereq_ښی & notReady)==0 ){
      إذا( p->prereq_ښی==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        أرجع p;
      }
      إذا( p_پایله==0 ) p_پایله = p;
    }
    p = where_لټون_بل(&لټون);
  }
  أرجع p_پایله;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
ساكن صحيح پیدا_کړه_شاخص_ستون(
  Parse *pParse,                  /* Parse context */
  بیان_List *pList,                /* Expression list to search */
  صحيح i_بنسټ,                      /* Cursor for table associated with pIdx */
  شاخص *pIdx,                    /* Index to match column of */
  صحيح i_ستون                        /* Column of index to match */
){
  صحيح i;
  ثابت حرف *zColl = pIdx->azColl[i_ستون];

  لكل(i=0; i<pList->n_بیان; i++){
    بیان *p = sqlite3_بیان_Skip_Collate_And_Likely(pList->a[i].p_بیان);
    إذا( ALWAYS(p!=0)
     && (p->op==TK_ستنه || p->op==TK_AGG_ستنه)
     && p->i_ستنه==pIdx->ai_ستنه[i_ستون]
     && p->i_جدول==i_بنسټ
    ){
      CollSeq *pColl = sqlite3_بیان_NNColl_Seq(pParse, pList->a[i].p_بیان);
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
ساكن صحيح شاخص_ستنه_Not_Null(شاخص *pIdx, صحيح i_ستون){
  صحيح j;
  أكد( pIdx!=0 );
  أكد( i_ستون>=0 && i_ستون<pIdx->n_ستنه );
  j = pIdx->ai_ستنه[i_ستون];
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
  بیان_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  جدول *pTab;
  شاخص *pIdx;
  صحيح i;                         
  صحيح i_بنسټ;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  إذا( pTabList->nSrc!=1 ) أرجع 0;
  i_بنسټ = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  لكل(i=0; i<pDistinct->n_بیان; i++){
    بیان *p = sqlite3_بیان_Skip_Collate_And_Likely(pDistinct->a[i].p_بیان);
    إذا( NEVER(p==0) ) استمر;
    إذا( p->op!=TK_ستنه && p->op!=TK_AGG_ستنه ) استمر;
    إذا( p->i_جدول==i_بنسټ && p->i_ستنه<0 ) أرجع 1;
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
  لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
    إذا( !Is_Unique_شاخص(pIdx) ) استمر;
    إذا( pIdx->pPartIdxWhere ) استمر;
    لكل(i=0; i<pIdx->n_کلی_ستون; i++){
      إذا( 0==sqlite3_Where_پیدا_کړه_غړی(pWC, i_بنسټ, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        إذا( پیدا_کړه_شاخص_ستون(pParse, pDistinct, i_بنسټ, pIdx, i)<0 ) اكسر;
        إذا( شاخص_ستنه_Not_Null(pIdx, i)==0 ) اكسر;
      }
    }
    إذا( i==pIdx->n_کلی_ستون ){
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
ساكن فراغ translate_ستنه_منزل_Copy(
  Parse *pParse,      /* Parsing context */
  صحيح i_پیل,         /* Translate from this opcode to the end */
  صحيح iTabCur,        /* OP_Column/OP_Rowid references to this table */
  صحيح i_ثبتوونکی,      /* The first column is in this register */
  صحيح iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, i_پیل);
  صحيح i_پای_ټکی = sqlite3VdbeCurrentAddr(v);
  إذا( pParse->db->mallocFailed ) أرجع;
#که_تعریف_شوی SQLITE_DEBUG
  إذا( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    چاپ_کړه("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, i_پیل, i_پای_ټکی);
  }
#د_که_پای
  لكل(; i_پیل<i_پای_ټکی; i_پیل++, pOp++){
    إذا( pOp->p1!=iTabCur ) استمر;
    إذا( pOp->opcode==OP_ستنه ){
#که_تعریف_شوی SQLITE_DEBUG
      إذا( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        چاپ_کړه("TRANSLATE OP_Column to OP_Copy at %d\n", i_پیل);
      }
#د_که_پای
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + i_ثبتوونکی;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }وإلا إذا( pOp->opcode==OP_Rowid ){
#که_تعریف_شوی SQLITE_DEBUG
      إذا( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        چاپ_کړه("TRANSLATE OP_Rowid to OP_Sequence at %d\n", i_پیل);
      }
#د_که_پای
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#که_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
      إذا( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#د_که_پای
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
       p->aConstraint[i].i_ستنه,
       p->aConstraint[i].i_غړی_دننه_واټن,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  لكل(i=0; i<p->n_ترتیب_By; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->a_ترتیب_By[i].i_ستنه,
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
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_عدد);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->ترتیب_By_Consumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimated_لګښت);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimated_لیکې);
}
#وإلا
#تعریف_کړه where_Trace_شاخص_Info_Inputs(A,B)
#تعریف_کړه where_Trace_شاخص_Info_Outputs(A,B)
#د_که_پای

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
  ثابت Where_غړی *p_غړی,       /* WHERE clause term to check */
  ثابت Src_توکی *pSrc           /* Table we are trying to access */
){
  أكد( (pSrc->fg.jointype&(JT_کیڼ|JT_LTORJ|JT_ښی))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_کیڼ|JT_LTORJ|JT_ښی))==JT_کیڼ );
  testcase( (pSrc->fg.jointype & (JT_کیڼ|JT_LTORJ|JT_ښی))==JT_LTORJ );
  testcase( بیان_Has_Property(p_غړی->p_بیان, EP_OuterON) )
  testcase( بیان_Has_Property(p_غړی->p_بیان, EP_InnerON) );
  إذا( !بیان_Has_Property(p_غړی->p_بیان, EP_OuterON|EP_InnerON)
   || p_غړی->p_بیان->w.iJoin != pSrc->iCursor
  ){
    أرجع 0;
  }
  إذا( (pSrc->fg.jointype & (JT_کیڼ|JT_ښی))!=0
   && NEVER(بیان_Has_Property(p_غړی->p_بیان, EP_InnerON))
  ){
    أرجع 0;
  }
  أرجع 1;
}

#که_نه_تعریف_شوی SQLITE_OMIT_AUTOMATIC_شاخص
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
ساكن SQLITE_NOINLINE صحيح ستنه_Is_Good_شاخص_Candidate(
  ثابت جدول *pTab,
  صحيح i_ستون
){
  ثابت شاخص *pIdx;
  لكل(pIdx = pTab->p_شاخص; pIdx!=0; pIdx=pIdx->p_بل){
    صحيح j;
    لكل(j=0; j<pIdx->n_کلی_ستون; j++){
       إذا( pIdx->ai_ستنه[j]==i_ستون ){
         إذا( j==0 ) أرجع 0;
         إذا( pIdx->hasStat1 && pIdx->ai_لیکه_Log_Est[j+1]>20 ) أرجع 0;
         اكسر;
       }
    }
  }
  أرجع 1;
}
#د_که_پای /* SQLITE_OMIT_AUTOMATIC_INDEX */



#که_نه_تعریف_شوی SQLITE_OMIT_AUTOMATIC_شاخص
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
ساكن صحيح غړی_Can_Drive_شاخص(
  ثابت Where_غړی *p_غړی,        /* WHERE clause term to check */
  ثابت Src_توکی *pSrc,           /* Table we are trying to access */
  ثابت Bitmask notReady         /* Tables in outer loops of the join */
){
  حرف aff;
  صحيح کیڼ_ستون;
  
  إذا( p_غړی->کیڼ_Cursor!=pSrc->iCursor ) أرجع 0;
  إذا( (p_غړی->eOperator & (WO_EQ|WO_IS))==0 ) أرجع 0;
  أكد( (pSrc->fg.jointype & JT_ښی)==0 );
  إذا( (pSrc->fg.jointype & (JT_کیڼ|JT_LTORJ|JT_ښی))!=0
   && !constraintCompatibleWithOuterJoin(p_غړی,pSrc)
  ){
    أرجع 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  إذا( (p_غړی->prereq_ښی & notReady)!=0 ) أرجع 0;
  أكد( (p_غړی->eOperator & (WO_OR|WO_AND))==0 );
  کیڼ_ستون = p_غړی->u.x.کیڼ_ستنه;
  إذا( کیڼ_ستون<0 ) أرجع 0;
  aff = pSrc->pSTab->a_ستون[کیڼ_ستون].affinity;
  إذا( !sqlite3_شاخص_Affinity_Ok(p_غړی->p_بیان, aff) ) أرجع 0;
  testcase( p_غړی->p_بیان->op==TK_IS );
  أرجع ستنه_Is_Good_شاخص_Candidate(pSrc->pSTab, کیڼ_ستون);
}
#د_که_پای


#که_نه_تعریف_شوی SQLITE_OMIT_AUTOMATIC_شاخص

#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
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
    أكد( pIdx->n_ستنه>1 );
    أكد( pIdx->ai_ستنه[pIdx->n_ستنه-1]==XN_ROWID || !HasRowid(pTab) );
    لكل(ii=0; ii<(pIdx->n_ستنه-1); ii++){
      ثابت حرف *zName = 0;
      صحيح i_ستون = pIdx->ai_ستنه[ii];

      zName = pTab->a_ستون[i_ستون].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    z_متن = sqlite3_str_پای(pStr);
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
#د_که_پای

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
  صحيح n_کلی_ستون;                /* Number of columns in the constructed index */
  Where_غړی *p_غړی;           /* A single term of the WHERE clause */
  Where_غړی *pWCEnd;          /* End of pWC->a[] */
  شاخص *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  صحيح addrInit;               /* Address of the initialization bypass jump */
  جدول *p_جدول;              /* The table being indexed */
  صحيح addr_سر;                /* Top of the index fill loop */
  صحيح reg_ریکارډ;              /* Register holding an index record */
  صحيح n;                      /* Column counter */
  صحيح i;                      /* Loop counter */
  صحيح mx_بټ_ستون;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  حرف *z_Not_کارول_شوی;             /* Extra space on the end of pIdx */
  Bitmask idx_ستون_شمېر;            /* Bitmap of columns used for indexing */
  Bitmask extra_ستون_شمېر;          /* Bitmap of additional columns */
  u8 لېږل_شوی_Warning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  بیان *pPartial = 0;         /* Partial Index Expression */
  صحيح iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  Src_توکی *pSrc;              /* The FROM clause term to get the next index */
  صحيح addrCounter = 0;        /* Address where integer counter is initialized */
  صحيح reg_بنسټ;                /* Array of registers where record is assembled */
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExp = 0;            /* Address of OP_Explain */
#د_که_پای

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  أكد( v!=0 );
  addrInit = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_کلی_ستون = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  p_جدول = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->n_غړی];
  pLoop = pLevel->pWLoop;
  idx_ستون_شمېر = 0;
  لكل(p_غړی=pWC->a; p_غړی<pWCEnd; p_غړی++){
    بیان *p_بیان = p_غړی->p_بیان;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    إذا( (p_غړی->wtFlags & غړی_VIRTUAL)==0
     && sqlite3_بیان_Is_Single_جدول_Constraint(p_بیان, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_بیان_And(pParse, pPartial,
                                sqlite3_بیان_Dup(pParse->db, p_بیان, 0));
    }
    إذا( غړی_Can_Drive_شاخص(p_غړی, pSrc, notReady) ){
      صحيح i_ستون;
      Bitmask c_ماسک;
      أكد( (p_غړی->eOperator & (WO_OR|WO_AND))==0 );
      i_ستون = p_غړی->u.x.کیڼ_ستنه;
      c_ماسک = i_ستون>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ستون);
      testcase( i_ستون==BMS );
      testcase( i_ستون==BMS-1 );
      إذا( !لېږل_شوی_Warning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", p_جدول->zName,
            p_جدول->a_ستون[i_ستون].zCnName);
        لېږل_شوی_Warning = 1;
      }
      إذا( (idx_ستون_شمېر & c_ماسک)==0 ){
        إذا( whereLoopResize(pParse->db, pLoop, n_کلی_ستون+1) ){
          اذهب پای_ټکی_auto_شاخص_create;
        }
        pLoop->aLTerm[n_کلی_ستون++] = p_غړی;
        idx_ستون_شمېر |= c_ماسک;
      }
    }
  }
  أكد( n_کلی_ستون>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_کلی_ستون;
  pLoop->wsFlags = WHERE_ستنه_EQ | WHERE_IDX_ONLY | WHERE_شاخصي
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
    extra_ستون_شمېر = ALLBITS & ~idx_ستون_شمېر;
  }وإلا{
    extra_ستون_شمېر = pSrc->ستون_کارول_شوی & (~idx_ستون_شمېر | MASKBIT(BMS-1));
  }
  إذا( !HasRowid(p_جدول) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    لكل(i=0; i<p_جدول->n_ستون; i++){
      إذا( (p_جدول->a_ستون[i].ستون_Flags & COLFLAG_PRIMKEY)==0 ) استمر;
      إذا( i>=BMS-1 ){
        extra_ستون_شمېر |= MASKBIT(BMS-1);
        اكسر;
      }
      إذا( idx_ستون_شمېر & MASKBIT(i) ) استمر;
      extra_ستون_شمېر |= MASKBIT(i);
    }
  }
  mx_بټ_ستون = MIN(BMS-1,p_جدول->n_ستون);
  testcase( p_جدول->n_ستون==BMS-1 );
  testcase( p_جدول->n_ستون==BMS-2 );
  لكل(i=0; i<mx_بټ_ستون; i++){
    إذا( extra_ستون_شمېر & MASKBIT(i) ) n_کلی_ستون++;
  }
  إذا( pSrc->ستون_کارول_شوی & MASKBIT(BMS-1) ){
    n_کلی_ستون += p_جدول->n_ستون - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  أكد( n_کلی_ستون <= p_جدول->n_ستون + MAX(0, p_جدول->n_ستون - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_شاخص_Object(pParse->db, n_کلی_ستون+HasRowid(p_جدول),
                                    0, &z_Not_کارول_شوی);
  إذا( pIdx==0 ) اذهب پای_ټکی_auto_شاخص_create;
  pLoop->u.btree.p_شاخص = pIdx;
  pIdx->zName = "auto-index";
  pIdx->p_جدول = p_جدول;
  n = 0;
  idx_ستون_شمېر = 0;
  لكل(p_غړی=pWC->a; p_غړی<pWCEnd; p_غړی++){
    إذا( غړی_Can_Drive_شاخص(p_غړی, pSrc, notReady) ){
      صحيح i_ستون;
      Bitmask c_ماسک;
      أكد( (p_غړی->eOperator & (WO_OR|WO_AND))==0 );
      i_ستون = p_غړی->u.x.کیڼ_ستنه;
      c_ماسک = i_ستون>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ستون);
      testcase( i_ستون==BMS-1 );
      testcase( i_ستون==BMS );
      إذا( (idx_ستون_شمېر & c_ماسک)==0 ){
        بیان *pX = p_غړی->p_بیان;
        idx_ستون_شمېر |= c_ماسک;
        pIdx->ai_ستنه[n] = p_غړی->u.x.کیڼ_ستنه;
        pColl = sqlite3_بیان_Compare_Coll_Seq(pParse, pX);
        أكد( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        إذا( !sqlite3_Is_دوګونی(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_دوګونی;
        n++;
      }
    }
  }
  أكد( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  لكل(i=0; i<mx_بټ_ستون; i++){
    إذا( extra_ستون_شمېر & MASKBIT(i) ){
      pIdx->ai_ستنه[n] = i;
      pIdx->azColl[n] = sqlite3_Str_دوګونی;
      n++;
    }
  }
  إذا( pSrc->ستون_کارول_شوی & MASKBIT(BMS-1) ){
    لكل(i=BMS-1; i<p_جدول->n_ستون; i++){
      pIdx->ai_ستنه[n] = i;
      pIdx->azColl[n] = sqlite3_Str_دوګونی;
      n++;
    }
  }
  أكد( n==n_کلی_ستون );
  إذا( HasRowid(p_جدول) ){
    pIdx->ai_ستنه[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_دوګونی;
  }

  /* Create the automatic index */
  explain_Automatic_شاخص(pParse, pIdx, pPartial!=0, &addrExp);
  أكد( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_کلی_ستون+1);
  sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pIdx);
  VdbeComment((v, "for %s", p_جدول->zName));
  إذا( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  أكد( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  إذا( pSrc->fg.له_لارې_Coroutine ){
    صحيح regYield;
    Subquery *pSubq;
    أكد( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    أكد( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, 0);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_سر =  sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }وإلا{
    أكد( pLevel->addrHalt );
    addr_سر = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  إذا( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_بیان_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  reg_ریکارډ = sqlite3_Get_لنډمهاله_Reg(pParse);
  reg_بنسټ = sqlite3_Generate_شاخص_کلی(
      pParse, pIdx, pLevel->iTabCur, reg_ریکارډ, 0, 0, 0, 0
  );
  إذا( pLevel->regFilter ){
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Filter_جمع_کړه, pLevel->regFilter, 0,
                         reg_بنسټ, pLoop->u.btree.nEq);
  }
  sqlite3_Vdbe_لټون_حالت_Counters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Idx_ورننباسه, pLevel->iIdxCur, reg_ریکارډ);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  إذا( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  إذا( pSrc->fg.له_لارې_Coroutine ){
    أكد( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, reg_بنسټ+n);
    testcase( pParse->db->mallocFailed );
    أكد( pLevel->iIdxCur>0 );
    translate_ستنه_منزل_Copy(pParse, addr_سر, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_پایله, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_سر);
    pSrc->fg.له_لارې_Coroutine = 0;
    sqlite3_Vdbe_Jump_دلته(v, addr_سر);
  }وإلا{
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, pLevel->iTabCur, addr_سر+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    إذا( (pSrc->fg.jointype & JT_کیڼ)!=0 ){
      sqlite3_Vdbe_Jump_دلته(v, addr_سر);
    }
  }
  sqlite3_Release_لنډمهاله_Reg(pParse, reg_ریکارډ);
 
  /* Jump here when skipping the initialization */
  sqlite3_Vdbe_Jump_دلته(v, addrInit);
  sqlite3_Vdbe_لټون_حالت_Range(v, addrExp, addrExp, -1);

پای_ټکی_auto_شاخص_create:
  sqlite3_بیان_Delete(pParse->db, pPartial);
}
#د_که_پای /* SQLITE_OMIT_AUTOMATIC_INDEX */

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
  صحيح addr_سر;                         /* Address of OP_Rewind */
  صحيح addrCont;                        /* Jump here to skip a row */
  ثابت Where_غړی *p_غړی;              /* For looping over WHERE clause terms */
  ثابت Where_غړی *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  صحيح iCur;                            /* Cursor for table getting the filter */
  شاخصي_بیان *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  شاخصي_بیان *saved_p_Idx_Part_بیان;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_بیان = pParse->p_Idx_Part_بیان;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_بیان = 0;

  أكد( pLoop!=0 );
  أكد( v!=0 );
  أكد( pLoop->wsFlags & WHERE_BLOOMFILTER );
  أكد( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Once); VdbeCoverage(v);
  افعل{
    ثابت SrcList *pTabList;
    ثابت Src_توکی *p_توکی;
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
    p_توکی = &pTabList->a[iSrc];
    أكد( p_توکی!=0 );
    pTab = p_توکی->pSTab;
    أكد( pTab!=0 );
    sz = sqlite3_Log_Est_منزل_Int(pTab->n_لیکه_Log_Est);
    إذا( sz<10000 ){
      sz = 10000;
    }وإلا إذا( sz>10000000 ){
      sz = 10000000;
    }
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Blob, (صحيح)sz, pLevel->regFilter);

    addr_سر = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.n_غړی];
    لكل(p_غړی=pWInfo->sWC.a; p_غړی<pWCEnd; p_غړی++){
      بیان *p_بیان = p_غړی->p_بیان;
      إذا( (p_غړی->wtFlags & غړی_VIRTUAL)==0
       && sqlite3_بیان_Is_Single_جدول_Constraint(p_بیان, pTabList, iSrc, 0)
      ){
        sqlite3_بیان_If_False(pParse, p_غړی->p_بیان, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    إذا( pLoop->wsFlags & WHERE_IPK ){
      صحيح r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Rowid, iCur, r1);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Filter_جمع_کړه, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_لنډمهاله_Reg(pParse, r1);
    }وإلا{
      شاخص *pIdx = pLoop->u.btree.p_شاخص;
      صحيح n = pLoop->u.btree.nEq;
      صحيح r1 = sqlite3_Get_لنډمهاله_Range(pParse, n);
      صحيح jj;
      أكد( pIdx!=0 );
      لكل(jj=0; jj<n; jj++){
        أكد( pIdx->p_جدول==p_توکی->pSTab );
        sqlite3_بیان_کوډ_بارول_شاخص_ستنه(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Filter_جمع_کړه, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_لنډمهاله_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, pLevel->iTabCur, addr_سر+1);
    VdbeCoverage(v);
    sqlite3_Vdbe_Jump_دلته(v, addr_سر);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    طالما( ++iLevel < pWInfo->nLevel ){
      ثابت Src_توکی *p_Tab_توکی;
      pLevel = &pWInfo->a[iLevel];
      p_Tab_توکی = &pWInfo->pTabList->a[pLevel->iFrom];
      إذا( p_Tab_توکی->fg.jointype & (JT_کیڼ|JT_LTORJ) ) استمر;
      pLoop = pLevel->pWLoop;
      إذا( NEVER(pLoop==0) ) استمر;
      إذا( pLoop->prereq & notReady ) استمر;
      إذا( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_ستنه_دننه))
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
  sqlite3_Vdbe_Jump_دلته(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_بیان = saved_p_Idx_Part_بیان;
}


#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
ساكن Where_غړی *غړی_From_Where_Clause(WhereClause *pWC, صحيح i_غړی){
  WhereClause *p;
  لكل(p=pWC; p; p=p->pOuter){
    إذا( i_غړی<p->n_غړی ) أرجع &p->a[i_غړی];
    i_غړی -= p->n_غړی;
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
  Src_توکی *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  صحيح i, j;
  صحيح n_غړی;
  Parse *pParse = pWInfo->pParse;
  بنية sqlite3_شاخص_constraint *pIdxCons;
  بنية sqlite3_شاخص_orderby *p_Idx_ترتیب_By;
  بنية sqlite3_شاخص_constraint_usage *pUsage;
  بنية Hidden_شاخص_Info *pHidden;
  Where_غړی *p_غړی;
  صحيح n_ترتیب_By;
  sqlite3_شاخص_info *pIdxInfo;
  u16 mNoOmit = 0;
  ثابت جدول *pTab;
  صحيح eDistinct = 0;
  بیان_List *p_ترتیب_By = pWInfo->p_ترتیب_By;
  WhereClause *p;

  أكد( pSrc!=0 );
  pTab = pSrc->pSTab;
  أكد( pTab!=0 );
  أكد( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  لكل(p=pWC, n_غړی=0; p; p=p->pOuter){
    لكل(i=0, p_غړی=p->a; i<p->n_غړی; i++, p_غړی++){
      p_غړی->wtFlags &= ~غړی_OK;
      إذا( p_غړی->کیڼ_Cursor != pSrc->iCursor ) استمر;
      إذا( p_غړی->prereq_ښی & mUnusable ) استمر;
      أكد( Is_Power_د_Two(p_غړی->eOperator & ~WO_EQUIV) );
      testcase( p_غړی->eOperator & WO_دننه );
      testcase( p_غړی->eOperator & WO_ISNULL );
      testcase( p_غړی->eOperator & WO_IS );
      testcase( p_غړی->eOperator & WO_ALL );
      إذا( (p_غړی->eOperator & ~(WO_EQUIV))==0 ) استمر;
      إذا( p_غړی->wtFlags & غړی_VNULL ) استمر;

      أكد( (p_غړی->eOperator & (WO_OR|WO_AND))==0 );
      أكد( p_غړی->u.x.کیڼ_ستنه>=XN_ROWID );
      أكد( p_غړی->u.x.کیڼ_ستنه<pTab->n_ستون );
      إذا( (pSrc->fg.jointype & (JT_کیڼ|JT_LTORJ|JT_ښی))!=0
          && !constraintCompatibleWithOuterJoin(p_غړی,pSrc)
        ){
        استمر;
      }
      n_غړی++;
      p_غړی->wtFlags |= غړی_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  n_ترتیب_By = 0;
  إذا( p_ترتیب_By ){
    صحيح n = p_ترتیب_By->n_بیان;
    لكل(i=0; i<n; i++){
      بیان *p_بیان = p_ترتیب_By->a[i].p_بیان;
      بیان *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      إذا( sqlite3_بیان_Is_Constant(0, p_بیان) ){
        استمر;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      إذا( p_ترتیب_By->a[i].fg.ترتیبول_Flags & KEYINFO_ترتیب_BIGNULL ) اكسر;

      /* First case - a direct column references without a COLLATE operator */
      إذا( p_بیان->op==TK_ستنه && p_بیان->i_جدول==pSrc->iCursor ){
        أكد( p_بیان->i_ستنه>=XN_ROWID && p_بیان->i_ستنه<pTab->n_ستون );
        استمر;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      إذا( p_بیان->op==TK_COLLATE
       && (pE2 = p_بیان->p_کیڼ)->op==TK_ستنه
       && pE2->i_جدول==pSrc->iCursor
      ){
        ثابت حرف *zColl;  /* The collating sequence name */
        أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
        أكد( p_بیان->u.zToken!=0 );
        أكد( pE2->i_ستنه>=XN_ROWID && pE2->i_ستنه<pTab->n_ستون );
        p_بیان->i_ستنه = pE2->i_ستنه;
        إذا( pE2->i_ستنه<0 ) استمر;  /* Collseq does not matter for rowid */
        zColl = sqlite3_ستنه_Coll(&pTab->a_ستون[pE2->i_ستنه]);
        إذا( zColl==0 ) zColl = sqlite3_Str_دوګونی;
        إذا( sqlite3_stricmp(p_بیان->u.zToken, zColl)==0 ) استمر;
      }

      /* No matches cause a break out of the loop */
      اكسر;
    }
    إذا( i==n ){
      صحيح b_ترتیبول_By_ډله = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      n_ترتیب_By = n;
      إذا( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowid_کارول_شوی ){
        eDistinct = 2 + b_ترتیبول_By_ډله;
      }وإلا إذا( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - b_ترتیبول_By_ډله;
      }وإلا إذا( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, حجم(*pIdxInfo)
                           + (حجم(*pIdxCons) + حجم(*pUsage))*n_غړی
                           + حجم(*p_Idx_ترتیب_By)*n_ترتیب_By
                           + SZ_HIDDENINDEXINFO(n_غړی) );
  إذا( pIdxInfo==0 ){
    sqlite3_Error_پیغام(pParse, "out of memory");
    أرجع 0;
  }
  pHidden = (بنية Hidden_شاخص_Info*)&pIdxInfo[1];
  pIdxCons = (بنية sqlite3_شاخص_constraint*)&pHidden->aRhs[n_غړی];
  p_Idx_ترتیب_By = (بنية sqlite3_شاخص_orderby*)&pIdxCons[n_غړی];
  pUsage = (بنية sqlite3_شاخص_constraint_usage*)&p_Idx_ترتیب_By[n_ترتیب_By];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->a_ترتیب_By = p_Idx_ترتیب_By;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->ستون_کارول_شوی = (sqlite3_int64)pSrc->ستون_کارول_شوی;
  إذا( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    شاخص *pPk = sqlite3_Primary_کلی_شاخص((جدول*)pTab);
    أكد( pPk!=0 );
    لكل(i=0; i<pPk->n_کلی_ستون; i++){
      صحيح i_ستون = pPk->ai_ستنه[i];
      أكد( i_ستون>=0 );
      إذا( i_ستون>=BMS-1 ) i_ستون = BMS-1;
      pIdxInfo->ستون_کارول_شوی |= MASKBIT(i_ستون);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->m_دننه = 0;
  لكل(p=pWC, i=j=0; p; p=p->pOuter){
    صحيح n_وروستی = i+p->n_غړی;;
    لكل(p_غړی=p->a; i<n_وروستی; i++, p_غړی++){
      u16 op;
      إذا( (p_غړی->wtFlags & غړی_OK)==0 ) استمر;
      pIdxCons[j].i_ستنه = p_غړی->u.x.کیڼ_ستنه;
      pIdxCons[j].i_غړی_دننه_واټن = i;
      op = p_غړی->eOperator & WO_ALL;
      إذا( op==WO_دننه ){
        إذا( (p_غړی->wtFlags & غړی_ټوټه)==0 ){
          pHidden->m_دننه |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      إذا( op==WO_AUX ){
        pIdxCons[j].op = p_غړی->e_سمون_Op;
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
        أكد( p_غړی->eOperator&(WO_دننه|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        إذا( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_بیان_Is_ویکټور(p_غړی->p_بیان->p_ښی)
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
  أكد( j==n_غړی );
  pIdxInfo->nConstraint = j;
  لكل(i=j=0; i<n_ترتیب_By; i++){
    بیان *p_بیان = p_ترتیب_By->a[i].p_بیان;
    إذا( sqlite3_بیان_Is_Constant(0, p_بیان) ) استمر;
    أكد( p_بیان->op==TK_ستنه
         || (p_بیان->op==TK_COLLATE && p_بیان->p_کیڼ->op==TK_ستنه
              && p_بیان->i_ستنه==p_بیان->p_کیڼ->i_ستنه) );
    p_Idx_ترتیب_By[j].i_ستنه = p_بیان->i_ستنه;
    p_Idx_ترتیب_By[j].desc = p_ترتیب_By->a[i].fg.ترتیبول_Flags & KEYINFO_ترتیب_DESC;
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
  إذا( pIdxInfo->need_منزل_Free_Idx_Str ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->need_منزل_Free_Idx_Str = 0;
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
    sqlite3_ارزښت_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
ساكن صحيح vtab_غوره_شاخص(Parse *pParse, جدول *pTab, sqlite3_شاخص_info *p){
  صحيح rc;
  sqlite3_vtab *pVtab;

  أكد( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_شاخص_Info_Inputs(p, pTab);
  pParse->db->n_Schema_قلف++;
  rc = pVtab->pModule->x_غوره_شاخص(pVtab, p);
  pParse->db->n_Schema_قلف--;
  where_Trace_شاخص_Info_Outputs(p, pTab);

  إذا( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    إذا( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }وإلا إذا( !pVtab->z_Err_پیغام ){
      sqlite3_Error_پیغام(pParse, "%s", sqlite3ErrStr(rc));
    }وإلا{
      sqlite3_Error_پیغام(pParse, "%s", pVtab->z_Err_پیغام);
    }
  }
  إذا( pTab->u.vtab.p->bAllSchemas ){
    sqlite3_Vtab_کاروي_All_Schemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_پیغام);
  pVtab->z_Err_پیغام = 0;
  أرجع rc;
}
#د_که_پای /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#که_تعریف_شوی SQLITE_ENABLE_STAT4
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
ساكن صحيح where_کلی_Stats(
  Parse *pParse,              /* Database connection */
  شاخص *pIdx,                /* Index to consider domain of */
  Unpacked_ریکارډ *pRec,       /* Vector of values to consider */
  صحيح پړاو_پورته,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  شاخص_Sample *aSample = pIdx->aSample;
  صحيح i_ستون;                   /* Index of required stats in anEq[] etc. */
  صحيح i;                      /* Index of first sample >= pRec */
  صحيح iSample;                /* Smallest sample larger than or equal to pRec */
  صحيح iMin = 0;               /* Smallest sample not yet tested */
  صحيح iTest;                  /* Next sample to test */
  صحيح res;                    /* Result of comparison operation */
  صحيح n_ډګر;                 /* Number of fields in pRec */
  tRowcnt i_ښکتنی = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#که_نه_تعریف_شوی SQLITE_DEBUG
  ناکارول_شوی_PARAMETER( pParse );
#د_که_پای
  أكد( pRec!=0 );
  أكد( pIdx->nSample>0 );
  أكد( pRec->n_ډګر>0 );


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
  إذا( !HasRowid(pIdx->p_جدول) && Is_Primary_کلی_شاخص(pIdx) ){
    n_ډګر = pIdx->n_کلی_ستون;
  }وإلا{
    n_ډګر = pIdx->n_ستنه;
  }
  n_ډګر = MIN(pRec->n_ډګر, n_ډګر);
  i_ستون = 0;
  iSample = pIdx->nSample * n_ډګر;
  افعل{
    صحيح iSamp;                    /* Index in aSample[] of test sample */
    صحيح n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / n_ډګر;
    إذا( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      لكل(n=(iTest % n_ډګر) + 1; n<n_ډګر; n++){
        إذا( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) اكسر;
      }
    }وإلا{
      n = iTest + 1;
    }

    pRec->n_ډګر = n;
    res = sqlite3_Vdbe_ریکارډ_Compare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    إذا( res<0 ){
      i_ښکتنی = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }وإلا إذا( res==0 && n<n_ډګر ){
      i_ښکتنی = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }وإلا{
      iSample = iTest;
      i_ستون = n-1;
    }
  }طالما( res && iMin<iSample );
  i = iSample / n_ډګر;

#که_تعریف_شوی SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  إذا( pParse->db->mallocFailed==0 ){
    إذا( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      أكد( i<pIdx->nSample );
      أكد( i_ستون==n_ډګر-1 );
      pRec->n_ډګر = n_ډګر;
      أكد( 0==sqlite3_Vdbe_ریکارډ_Compare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }وإلا{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      أكد( i<=pIdx->nSample && i>=0 );
      pRec->n_ډګر = i_ستون+1;
      أكد( i==pIdx->nSample
           || sqlite3_Vdbe_ریکارډ_Compare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      إذا( i_ستون>0 ){
        pRec->n_ډګر = i_ستون;
        أكد( sqlite3_Vdbe_ریکارډ_Compare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      إذا( i>0 ){
        pRec->n_ډګر = n_ډګر;
        أكد( sqlite3_Vdbe_ریکارډ_Compare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#د_که_پای /* ifdef SQLITE_DEBUG */

  إذا( res==0 ){
    /* Record pRec is equal to sample i */
    أكد( i_ستون==n_ډګر-1 );
    aStat[0] = aSample[i].anLt[i_ستون];
    aStat[1] = aSample[i].anEq[i_ستون];
  }وإلا{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt i_پاسنی, iGap;
    إذا( i>=pIdx->nSample ){
      i_پاسنی = pIdx->n_لیکه_Est0;
    }وإلا{
      i_پاسنی = aSample[i].anLt[i_ستون];
    }

    إذا( i_ښکتنی>=i_پاسنی ){
      iGap = 0;
    }وإلا{
      iGap = i_پاسنی - i_ښکتنی;
    }
    إذا( پړاو_پورته ){
      iGap = (iGap*2)/3;
    }وإلا{
      iGap = iGap/3;
    }
    aStat[0] = i_ښکتنی + iGap;
    aStat[1] = pIdx->aAvgEq[n_ډګر-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->n_ډګر = n_ډګر;
  أرجع i;
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */

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
ساكن LogEst whereRangeAdjust(Where_غړی *p_غړی, LogEst nNew){
  LogEst nRet = nNew;
  إذا( p_غړی ){
    إذا( p_غړی->truthProb<=0 ){
      nRet += p_غړی->truthProb;
    }وإلا إذا( (p_غړی->wtFlags & غړی_VNULL)==0 ){
      nRet -= 20;        أكد( 20==sqlite3LogEst(4) );
    }
  }
  أرجع nRet;
}


#که_تعریف_شوی SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
حرف sqlite3_شاخص_ستنه_Affinity(sqlite3 *db, شاخص *pIdx, صحيح i_ستون){
  أكد( i_ستون>=0 && i_ستون<pIdx->n_ستنه );
  إذا( !pIdx->z_ستون_Aff ){
    إذا( sqlite3_شاخص_Affinity_Str(db, pIdx)==0 ) أرجع SQLITE_AFF_BLOB;
  }
  أكد( pIdx->z_ستون_Aff[i_ستون]!=0 );
  أرجع pIdx->z_ستون_Aff[i_ستون];
}
#د_که_پای


#که_تعریف_شوی SQLITE_ENABLE_STAT4
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
ساكن صحيح where_Range_Skip_لټون_Est(
  Parse *pParse,       /* Parsing & code generating context */
  Where_غړی *p_ښکتنی,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_غړی *p_پاسنی,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  صحيح *pb_بشپړ          /* Set to true if at least one expr. value extracted */
){
  شاخص *p = pLoop->u.btree.p_شاخص;
  صحيح nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  صحيح n_ښکتنی = -1;
  صحيح n_پاسنی = p->nSample+1;
  صحيح rc = SQLITE_OK;
  u8 aff = sqlite3_شاخص_ستنه_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_ارزښت *p1 = 0;          /* Value extracted from pLower */
  sqlite3_ارزښت *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_ارزښت *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  إذا( p_ښکتنی ){
    rc = sqlite3_Stat4_ارزښت_From_بیان(pParse, p_ښکتنی->p_بیان->p_ښی, aff, &p1);
    n_ښکتنی = 0;
  }
  إذا( p_پاسنی && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_ارزښت_From_بیان(pParse, p_پاسنی->p_بیان->p_ښی, aff, &p2);
    n_پاسنی = p2 ? 0 : p->nSample;
  }

  إذا( p1 || p2 ){
    صحيح i;
    صحيح nDiff;
    لكل(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3_Stat4_ستنه(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      إذا( rc==SQLITE_OK && p1 ){
        صحيح res = sqlite3MemCompare(p1, pVal, pColl);
        إذا( res>=0 ) n_ښکتنی++;
      }
      إذا( rc==SQLITE_OK && p2 ){
        صحيح res = sqlite3MemCompare(p2, pVal, pColl);
        إذا( res>=0 ) n_پاسنی++;
      }
    }
    nDiff = (n_پاسنی - n_ښکتنی);
    إذا( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    إذا( nDiff!=1 || p_پاسنی==0 || p_ښکتنی==0 ){
      صحيح nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->n_بهر -= nAdjust;
      *pb_بشپړ = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           n_ښکتنی, n_پاسنی, nAdjust*-1, pLoop->n_بهر));
    }

  }وإلا{
    أكد( *pb_بشپړ==0 );
  }

  sqlite3_ارزښت_Free(p1);
  sqlite3_ارزښت_Free(p2);
  sqlite3_ارزښت_Free(pVal);

  أرجع rc;
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */

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
ساكن صحيح where_Range_لټون_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Where_غړی *p_ښکتنی,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_غړی *p_پاسنی,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  صحيح rc = SQLITE_OK;
  صحيح n_بهر = pLoop->n_بهر;
  LogEst nNew;

#که_تعریف_شوی SQLITE_ENABLE_STAT4
  شاخص *p = pLoop->u.btree.p_شاخص;
  صحيح nEq = pLoop->u.btree.nEq;

  إذا( p->nSample>0 && ALWAYS(nEq<p->n_Sample_ستون)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    إذا( nEq==pBuilder->nRecValid ){
      Unpacked_ریکارډ *pRec = pBuilder->pRec;
      tRowcnt a[2];
      صحيح nBtm = pLoop->u.btree.nBtm;
      صحيح n_سر = pLoop->u.btree.n_سر;

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
      tRowcnt i_ښکتنی;     /* Rows less than the lower bound */
      tRowcnt i_پاسنی;     /* Rows less than the upper bound */
      صحيح iLwrIdx = -2;   /* aSample[] for the lower bound */
      صحيح iUprIdx = -1;   /* aSample[] for the upper bound */

      إذا( pRec ){
        testcase( pRec->n_ډګر!=pBuilder->nRecValid );
        pRec->n_ډګر = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      إذا( nEq==0 ){
        i_ښکتنی = 0;
        i_پاسنی = p->n_لیکه_Est0;
      }وإلا{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_کلی_Stats(pParse, p, pRec, 0, a);
        i_ښکتنی = a[0];
        i_پاسنی = a[0] + a[1];
      }

      أكد( p_ښکتنی==0 || (p_ښکتنی->eOperator & (WO_GT|WO_GE))!=0 );
      أكد( p_پاسنی==0 || (p_پاسنی->eOperator & (WO_LT|WO_LE))!=0 );
      أكد( p->a_ترتیبول_ترتیب!=0 );
      إذا( p->a_ترتیبول_ترتیب[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        بدلون(Where_غړی*, p_ښکتنی, p_پاسنی);
        بدلون(صحيح, nBtm, n_سر);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      إذا( p_ښکتنی ){
        صحيح n;                    /* Values extracted from pExpr */
        بیان *p_بیان = p_ښکتنی->p_بیان->p_ښی;
        rc = sqlite3_Stat4_Probe_سېټ_ارزښت(pParse, p, &pRec, p_بیان, nBtm, nEq, &n);
        إذا( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ماسک = WO_GT|WO_LE;
          إذا( sqlite3_بیان_ویکټور_اندازه(p_بیان)>n ) ماسک = (WO_LE|WO_LT);
          iLwrIdx = where_کلی_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((p_ښکتنی->eOperator & ماسک) ? a[1] : 0);
          إذا( iNew>i_ښکتنی ) i_ښکتنی = iNew;
          n_بهر--;
          p_ښکتنی = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      إذا( p_پاسنی ){
        صحيح n;                    /* Values extracted from pExpr */
        بیان *p_بیان = p_پاسنی->p_بیان->p_ښی;
        rc = sqlite3_Stat4_Probe_سېټ_ارزښت(pParse, p, &pRec, p_بیان, n_سر, nEq, &n);
        إذا( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ماسک = WO_GT|WO_LE;
          إذا( sqlite3_بیان_ویکټور_اندازه(p_بیان)>n ) ماسک = (WO_LE|WO_LT);
          iUprIdx = where_کلی_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((p_پاسنی->eOperator & ماسک) ? a[1] : 0);
          إذا( iNew<i_پاسنی ) i_پاسنی = iNew;
          n_بهر--;
          p_پاسنی = 0;
        }
      }

      pBuilder->pRec = pRec;
      إذا( rc==SQLITE_OK ){
        إذا( i_پاسنی>i_ښکتنی ){
          nNew = sqlite3LogEst(i_پاسنی - i_ښکتنی);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          إذا( iLwrIdx==iUprIdx ){ nNew -= 20; }
          أكد( 20==sqlite3LogEst(4) );
        }وإلا{
          nNew = 10;        أكد( 10==sqlite3LogEst(2) );
        }
        إذا( nNew<n_بهر ){
          n_بهر = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)i_ښکتنی, (u32)i_پاسنی, n_بهر));
      }
    }وإلا{
      صحيح b_بشپړ = 0;
      rc = where_Range_Skip_لټون_Est(pParse, p_ښکتنی, p_پاسنی, pLoop, &b_بشپړ);
      إذا( b_بشپړ ) أرجع rc;
    }
  }
#وإلا
  ناکارول_شوی_PARAMETER(pParse);
  ناکارول_شوی_PARAMETER(pBuilder);
  أكد( p_ښکتنی || p_پاسنی );
#د_که_پای
  أكد( p_پاسنی==0 || (p_پاسنی->wtFlags & غړی_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(p_ښکتنی, n_بهر);
  nNew = whereRangeAdjust(p_پاسنی, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  إذا( p_ښکتنی && p_ښکتنی->truthProb>0 && p_پاسنی && p_پاسنی->truthProb>0 ){
    nNew -= 20;
  }

  n_بهر -= (p_ښکتنی!=0) + (p_پاسنی!=0);
  إذا( nNew<10 ) nNew = 10;
  إذا( nNew<n_بهر ) n_بهر = nNew;
#إذا defined(WHERETRACE_ENABLED)
  إذا( pLoop->n_بهر>n_بهر ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->n_بهر, n_بهر));
  }
#د_که_پای
  pLoop->n_بهر = (LogEst)n_بهر;
  أرجع rc;
}

#که_تعریف_شوی SQLITE_ENABLE_STAT4
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
ساكن صحيح where_Equal_لټون_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  بیان *p_بیان,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pn_لیکه       /* Write the revised row estimate here */
){
  شاخص *p = pBuilder->pNew->u.btree.p_شاخص;
  صحيح nEq = pBuilder->pNew->u.btree.nEq;
  Unpacked_ریکارډ *pRec = pBuilder->pRec;
  صحيح rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  صحيح bOk;

  أكد( nEq>=1 );
  أكد( nEq<=p->n_ستنه );
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
  إذا( nEq>=p->n_ستنه ){
    *pn_لیکه = 1;
    أرجع SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_سېټ_ارزښت(pParse, p, &pRec, p_بیان, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  إذا( rc!=SQLITE_OK ) أرجع rc;
  إذا( bOk==0 ) أرجع SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_کلی_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (صحيح)a[1]));
  *pn_لیکه = a[1];
 
  أرجع rc;
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */

#که_تعریف_شوی SQLITE_ENABLE_STAT4
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
ساكن صحيح where_دننه_لټون_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  بیان_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pn_لیکه       /* Write the revised row estimate here */
){
  شاخص *p = pBuilder->pNew->u.btree.p_شاخص;
  i64 nRow0 = sqlite3_Log_Est_منزل_Int(p->ai_لیکه_Log_Est[0]);
  صحيح nRecValid = pBuilder->nRecValid;
  صحيح rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt n_لیکه_Est = 0;    /* New estimate of the number of rows */
  صحيح i;                  /* Loop counter */

  أكد( p->aSample!=0 );
  لكل(i=0; rc==SQLITE_OK && i<pList->n_بیان; i++){
    nEst = nRow0;
    rc = where_Equal_لټون_Est(pParse, pBuilder, pList->a[i].p_بیان, &nEst);
    n_لیکه_Est += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  إذا( rc==SQLITE_OK ){
    إذا( n_لیکه_Est > (tRowcnt)nRow0 ) n_لیکه_Est = nRow0;
    *pn_لیکه = n_لیکه_Est;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", n_لیکه_Est));
  }
  أكد( pBuilder->nRecValid==nRecValid );
  أرجع rc;
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */


#إذا defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
فراغ sqlite3_Where_غړی_Print(Where_غړی *p_غړی, صحيح i_غړی){
  إذا( p_غړی==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", i_غړی);
  }وإلا{
    حرف zType[8];
    حرف z_کیڼ[50];
    حافظه_نقل(zType, "....", 5);
    إذا( p_غړی->wtFlags & غړی_VIRTUAL ) zType[0] = 'V';
    إذا( p_غړی->eOperator & WO_EQUIV  ) zType[1] = 'E';
    إذا( بیان_Has_Property(p_غړی->p_بیان, EP_OuterON) ) zType[2] = 'L';
    إذا( p_غړی->wtFlags & غړی_CODED  ) zType[3] = 'C';
    إذا( p_غړی->eOperator & WO_SINGLE ){
      أكد( (p_غړی->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(حجم(z_کیڼ),z_کیڼ,"left={%d:%d}",
                       p_غړی->کیڼ_Cursor, p_غړی->u.x.کیڼ_ستنه);
    }وإلا إذا( (p_غړی->eOperator & WO_OR)!=0 && p_غړی->u.pOrInfo!=0 ){
      sqlite3_snprintf(حجم(z_کیڼ),z_کیڼ,"indexable=0x%llx",
                       p_غړی->u.pOrInfo->indexable);
    }وإلا{
      sqlite3_snprintf(حجم(z_کیڼ),z_کیڼ,"left=%d", p_غړی->کیڼ_Cursor);
    }
    i_غړی = p_غړی->i_غړی = MAX(i_غړی,p_غړی->i_غړی);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       i_غړی, p_غړی, zType, z_کیڼ, p_غړی->eOperator, p_غړی->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    إذا( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        p_غړی->truthProb, (u64)p_غړی->prereqAll, (u64)p_غړی->prereq_ښی);
    }
    إذا( (p_غړی->eOperator & (WO_OR|WO_AND))==0 && p_غړی->u.x.i_ډګر ){
      sqlite3DebugPrintf(" iField=%d", p_غړی->u.x.i_ډګر);
    }
    إذا( p_غړی->i_پلار>=0 ){
      sqlite3DebugPrintf(" iParent=%d", p_غړی->i_پلار);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_ونه_View_بیان(0, p_غړی->p_بیان, 0);
  }
}
فراغ sqlite3_Show_Where_غړی(Where_غړی *p_غړی){
  sqlite3_Where_غړی_Print(p_غړی, 0);
}
#د_که_پای

#که_تعریف_شوی WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
فراغ sqlite3WhereClausePrint(WhereClause *pWC){
  صحيح i;
  لكل(i=0; i<pWC->n_غړی; i++){
    sqlite3_Where_غړی_Print(&pWC->a[i], i);
  }
}
#د_که_پای

#که_تعریف_شوی WHERETRACE_ENABLED
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
    Src_توکی *p_توکی;
    جدول *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    p_توکی = pWInfo->pTabList->a + p->iTab;
    pTab = p_توکی->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->ماسک_Self, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       p_توکی->zAlias ? p_توکی->zAlias : pTab->zName);
  }وإلا{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->ماسک_Self, p->prereq & 0xfff, p->cId, p->iTab);
  }
  إذا( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ثابت حرف *zName;
    إذا( p->u.btree.p_شاخص && (zName = p->u.btree.p_شاخص->zName)!=0 ){
      إذا( مزي_پرتله(zName, "sqlite_autoindex_", 17)==0 ){
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
                p->u.vtab.idx_عدد, p->u.vtab.idxStr, p->u.vtab.omit_ماسک);
    }وإلا{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_عدد, p->u.vtab.omit_ماسک);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  إذا( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }وإلا{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  إذا( pWInfo && pWInfo->b_ستوری_کارول_شوی && p->r_ستوری_Delta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->r_تیاری, p->r_پرلپسې, p->n_بهر, p->r_ستوری_Delta);
  }وإلا{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->r_تیاری, p->r_پرلپسې, p->n_بهر);
  }
  إذا( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    صحيح i;
    لكل(i=0; i<p->nLTerm; i++){
      sqlite3_Where_غړی_Print(p->aLTerm[i], i);
    }
  }
}
فراغ sqlite3ShowWhereLoop(ثابت WhereLoop *p){
  إذا( p ) sqlite3WhereLoopPrint(p, 0);
}
فراغ sqlite3ShowWhereLoopList(ثابت WhereLoop *p){
  طالما( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_بل_Loop;
  }
}
#د_که_پای

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
  Where_غړی **paNew;
  إذا( p->nLSlot>=n ) أرجع SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, حجم(p->aLTerm[0])*n);
  إذا( paNew==0 ) أرجع SQLITE_NOMEM_BKPT;
  حافظه_نقل(paNew, p->aLTerm, حجم(p->aLTerm[0])*p->nLSlot);
  إذا( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  أرجع SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
ساكن صحيح whereLoopXfer(sqlite3 *db, WhereLoop *p_منزل, WhereLoop *pFrom){
  whereLoopClearUnion(db, p_منزل);
  إذا( pFrom->nLTerm > p_منزل->nLSlot
   && whereLoopResize(db, p_منزل, pFrom->nLTerm)
  ){
    memset(p_منزل, 0, WHERE_LOOP_XFER_SZ);
    أرجع SQLITE_NOMEM_BKPT;
  }
  حافظه_نقل(p_منزل, pFrom, WHERE_LOOP_XFER_SZ);
  حافظه_نقل(p_منزل->aLTerm, pFrom->aLTerm, p_منزل->nLTerm*حجم(p_منزل->aLTerm[0]));
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
    pWInfo->pLoops = p->p_بل_Loop;
    whereLoopDelete(db, p);
  }
  طالما( pWInfo->p_Mem_منزل_Free ){
    Where_Mem_بلاک *p_بل = pWInfo->p_Mem_منزل_Free->p_بل;
    sqlite3DbNNFreeNN(db, pWInfo->p_Mem_منزل_Free);
    pWInfo->p_Mem_منزل_Free = p_بل;
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
  إذا( pX->r_پرلپسې>pY->r_پرلپسې && pX->n_بهر>pY->n_بهر ) أرجع 0; /* (1d) and (2a) */
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
ساكن فراغ where_Loop_Adjust_لګښت(ثابت WhereLoop *p, WhereLoop *pTemplate){
  إذا( (pTemplate->wsFlags & WHERE_شاخصي)==0 ) أرجع;
  لكل(; p; p=p->p_بل_Loop){
    إذا( p->iTab!=pTemplate->iTab ) استمر;
    إذا( (p->wsFlags & WHERE_شاخصي)==0 ) استمر;
    إذا( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_پرلپسې, pTemplate->n_بهر,
                       MIN(p->r_پرلپسې, pTemplate->r_پرلپسې),
                       MIN(p->n_بهر - 1, pTemplate->n_بهر)));
      pTemplate->r_پرلپسې = MIN(p->r_پرلپسې, pTemplate->r_پرلپسې);
      pTemplate->n_بهر = MIN(p->n_بهر - 1, pTemplate->n_بهر);
    }وإلا إذا( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_پرلپسې, pTemplate->n_بهر,
                       MAX(p->r_پرلپسې, pTemplate->r_پرلپسې),
                       MAX(p->n_بهر + 1, pTemplate->n_بهر)));
      pTemplate->r_پرلپسې = MAX(p->r_پرلپسې, pTemplate->r_پرلپسې);
      pTemplate->n_بهر = MAX(p->n_بهر + 1, pTemplate->n_بهر);
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
ساكن WhereLoop **where_Loop_پیدا_کړه_Lesser(
  WhereLoop **ppPrev,
  ثابت WhereLoop *pTemplate
){
  WhereLoop *p;
  لكل(p=(*ppPrev); p; ppPrev=&p->p_بل_Loop, p=*ppPrev){
    إذا( p->iTab!=pTemplate->iTab || p->i_ترتیبول_Idx!=pTemplate->i_ترتیبول_Idx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      استمر;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    أكد( p->r_تیاری==0 || pTemplate->r_تیاری==0
                 || p->r_تیاری==pTemplate->r_تیاری );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    أكد( p->r_تیاری>=pTemplate->r_تیاری );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    إذا( (p->wsFlags & WHERE_AUTO_شاخص)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_شاخصي)!=0
     && (pTemplate->wsFlags & WHERE_ستنه_EQ)!=0
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
     && p->r_تیاری<=pTemplate->r_تیاری                  /* (2a) */
     && p->r_پرلپسې<=pTemplate->r_پرلپسې                      /* (2b) */
     && p->n_بهر<=pTemplate->n_بهر                      /* (2c) */
    ){
      أرجع 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    إذا( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->r_پرلپسې>=pTemplate->r_پرلپسې                             /* (2a) */
     && p->n_بهر>=pTemplate->n_بهر                             /* (2b) */
    ){
      أكد( p->r_تیاری>=pTemplate->r_تیاری ); /* SETUP-INVARIANT above */
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
ساكن صحيح where_Loop_ورننباسه(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  صحيح rc;

  /* Stop the search once we hit the query planner search limit */
  إذا( pBuilder->i_Plan_حد==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    إذا( pBuilder->p_Or_سېټ ) pBuilder->p_Or_سېټ->n = 0;
    أرجع SQLITE_بشپړ;
  }
  pBuilder->i_Plan_حد--;

  where_Loop_Adjust_لګښت(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  إذا( pBuilder->p_Or_سېټ!=0 ){
    إذا( pTemplate->nLTerm ){
#إذا WHERETRACE_ENABLED
      u16 n = pBuilder->p_Or_سېټ->n;
      صحيح x =
#د_که_پای
      where_Or_ورننباسه(pBuilder->p_Or_سېټ, pTemplate->prereq, pTemplate->r_پرلپسې,
                                    pTemplate->n_بهر);
#إذا WHERETRACE_ENABLED /* 0x8 */
      إذا( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#د_که_پای
    }
    أرجع SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = where_Loop_پیدا_کړه_Lesser(&pWInfo->pLoops, pTemplate);

  إذا( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#إذا WHERETRACE_ENABLED /* 0x8 */
    إذا( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#د_که_پای
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
#د_که_پای
  إذا( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, حجم(WhereLoop));
    إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_بل_Loop = 0;
  }وإلا{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_لکۍ = &p->p_بل_Loop;
    WhereLoop *p_منزل_Del;
    طالما( *pp_لکۍ ){
      pp_لکۍ = where_Loop_پیدا_کړه_Lesser(pp_لکۍ, pTemplate);
      إذا( pp_لکۍ==0 ) اكسر;
      p_منزل_Del = *pp_لکۍ;
      إذا( p_منزل_Del==0 ) اكسر;
      *pp_لکۍ = p_منزل_Del->p_بل_Loop;
#إذا WHERETRACE_ENABLED /* 0x8 */
      إذا( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(p_منزل_Del, pBuilder->pWC);
      }
#د_که_پای
      whereLoopDelete(db, p_منزل_Del);
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
ساكن صحيح بیان_غوټه_نمونه_اوږدتیا_Est(Walker *pWalker, بیان *p_بیان){
  إذا( p_بیان->op==TK_STRING ){
    صحيح sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_بیان->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    إذا( pWalker->e_کوډ ){
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
ساكن صحيح est_Like_نمونه_اوږدتیا(بیان *p, u16 e_کوډ){
  Walker w;
  w.u.sz = 0;
  w.e_کوډ = e_کوډ;
  w.x_بیان_Callback = بیان_غوټه_نمونه_اوږدتیا_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#که_تعریف_شوی SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#د_که_پای
  sqlite3_Walk_بیان(&w, p);
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
  LogEst n_لیکه            /* Number of rows in the entire table */
){
  Where_غړی *p_غړی, *pX;
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
  لكل(i=pWC->n_بنسټ, p_غړی=pWC->a; i>0; i--, p_غړی++){
    أكد( p_غړی!=0 );
    إذا( (p_غړی->prereqAll & notAllowed)!=0 ) استمر;
    إذا( (p_غړی->prereqAll & pLoop->ماسک_Self)==0 ) استمر;
    إذا( (p_غړی->wtFlags & غړی_VIRTUAL)!=0 ) استمر;
    لكل(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      إذا( pX==0 ) استمر;
      إذا( pX==p_غړی ) اكسر;
      إذا( pX->i_پلار>=0 && (&pWC->a[pX->i_پلار])==p_غړی ) اكسر;
    }
    إذا( j<0 ){
      sqlite3_پرمختګ_Check(pWC->pWInfo->pParse);
      إذا( pLoop->ماسک_Self==p_غړی->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        إذا( (p_غړی->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_کیڼ|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      إذا( p_غړی->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->n_بهر += p_غړی->truthProb;
      }وإلا{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        بیان *p_Op_بیان = p_غړی->p_بیان;
        pLoop->n_بهر--;
        إذا( (p_غړی->eOperator&(WO_EQ|WO_IS))!=0
         && (p_غړی->wtFlags & غړی_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          بیان *p_ښی = p_Op_بیان->p_ښی;
          Parse *pParse = pWC->pWInfo->pParse;
          صحيح k = 0;
          testcase( p_Op_بیان->op==TK_IS );
          إذا( sqlite3_بیان_Is_Integer(p_ښی, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }وإلا{
            k = 20;
          }
          إذا( iReduce<k ){
            p_غړی->wtFlags |= غړی_HEURTRUTH;
            iReduce = k;
          }
        }وإلا
        إذا( بیان_Has_Property(p_Op_بیان, EP_InfixFunc) 
         && p_Op_بیان->op==TK_FUNCTION
        ){
          صحيح eOp;
          أكد( بیان_Use_XList(p_Op_بیان) );
          أكد( p_Op_بیان->x.pList->n_بیان>=2 );
          eOp = sqlite3_بیان_Is_Like_Operator(p_Op_بیان);
          إذا( ALWAYS(eOp>0) ){
            صحيح sz_نمونه;
            بیان *pRHS = p_Op_بیان->x.pList->a[0].p_بیان;
            eOp = eOp==SQLITE_شاخص_CONSTRAINT_LIKE;
            sz_نمونه = est_Like_نمونه_اوږدتیا(pRHS, eOp);
            إذا( sz_نمونه>0 ){
              pLoop->n_بهر -= sz_نمونه*2;
            }
          }
        }
      }
    }
  }
  إذا( pLoop->n_بهر > n_لیکه-iReduce ){
    pLoop->n_بهر = n_لیکه - iReduce;
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
ساكن صحيح where_Range_ویکټور_Len(
  Parse *pParse,       /* Parsing context */
  صحيح iCur,            /* Cursor open on pIdx */
  شاخص *pIdx,         /* The index to be used for a inequality constraint */
  صحيح nEq,             /* Number of prior equality constraints on same index */
  Where_غړی *p_غړی     /* The vector inequality constraint */
){
  صحيح nCmp = sqlite3_بیان_ویکټور_اندازه(p_غړی->p_بیان->p_کیڼ);
  صحيح i;

  nCmp = MIN(nCmp, (pIdx->n_ستنه - nEq));
  لكل(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    حرف aff;                     /* Comparison affinity */
    حرف idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    بیان *pLhs, *pRhs;

    أكد( بیان_Use_XList(p_غړی->p_بیان->p_کیڼ) );
    pLhs = p_غړی->p_بیان->p_کیڼ->x.pList->a[i].p_بیان;
    pRhs = p_غړی->p_بیان->p_ښی;
    إذا( بیان_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_بیان;
    }وإلا{
      pRhs = pRhs->x.pList->a[i].p_بیان;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    إذا( pLhs->op!=TK_ستنه
     || pLhs->i_جدول!=iCur
     || pLhs->i_ستنه!=pIdx->ai_ستنه[i+nEq]
     || pIdx->a_ترتیبول_ترتیب[i+nEq]!=pIdx->a_ترتیبول_ترتیب[nEq]
    ){
      اكسر;
    }

    testcase( pLhs->i_ستنه==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_بیان_Affinity(pLhs));
    idxaff = sqlite3_جدول_ستنه_Affinity(pIdx->p_جدول, pLhs->i_ستنه);
    إذا( aff!=idxaff ) اكسر;

    إذا( بیان_Has_Property(p_غړی->p_بیان, EP_Commuted) ) بدلون(بیان*, pRhs, pLhs);
    pColl = sqlite3_دوګونی_Compare_Coll_Seq(pParse, pLhs, pRhs);
    إذا( pColl==0 ) اكسر;
    إذا( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) اكسر;
  }
  أرجع i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#که_تعریف_شوی SQLITE_ENABLE_COSTMULT
# define Apply_لګښت_Multiplier(C,T)  C += T
#وإلا
# define Apply_لګښت_Multiplier(C,T)
#د_که_پای

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
ساكن صحيح where_Loop_جمع_کړه_Btree_شاخص(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  Src_توکی *pSrc,                  /* FROM clause term being analyzed */
  شاخص *pProbe,                  /* An index on pSrc */
  LogEst n_دننه_Mul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  Where_غړی *p_غړی;               /* A WhereTerm under consideration */
  صحيح op_ماسک;                     /* Valid operators for constraints */
  Where_لټون لټون;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_سر;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_n_بهر;              /* Original value of pNew->nOut */
  صحيح rc = SQLITE_OK;             /* Return code */
  LogEst r_اندازه;                   /* Number of rows in the table */
  LogEst r_Log_اندازه;                /* Logarithm of table size */
  Where_غړی *p_سر_2 = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  أكد( db->mallocFailed==0 || pParse->nErr>0 );
  إذا( pParse->nErr ){
    أرجع pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->p_جدول->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->r_پرلپسې));

  أكد( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pNew->wsFlags & WHERE_سر_حد)==0 );
  إذا( pNew->wsFlags & WHERE_BTM_حد ){
    op_ماسک = WO_LT|WO_LE;
  }وإلا{
    أكد( pNew->u.btree.nBtm==0 );
    op_ماسک = WO_EQ|WO_دننه|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  إذا( pProbe->bUnordered ){
    op_ماسک &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  أكد( pNew->u.btree.nEq<pProbe->n_ستنه );
  أكد( pNew->u.btree.nEq<pProbe->n_کلی_ستون
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_سر = pNew->u.btree.n_سر;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_n_بهر = pNew->n_بهر;
  p_غړی = where_لټون_Init(&لټون, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        op_ماسک, pProbe);
  pNew->r_تیاری = 0;
  r_اندازه = pProbe->ai_لیکه_Log_Est[0];
  r_Log_اندازه = estLog(r_اندازه);
  لكل(; rc==SQLITE_OK && p_غړی!=0; p_غړی = where_لټون_بل(&لټون)){
    u16 eOp = p_غړی->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst r_لګښت_Idx;
    LogEst n_بهر_Unadjusted;        /* nOut before IN() and WHERE adjustments */
    صحيح n_دننه = 0;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    صحيح nRecValid = pBuilder->nRecValid;
#د_که_پای
    إذا( (eOp==WO_ISNULL || (p_غړی->wtFlags&غړی_VNULL)!=0)
     && شاخص_ستنه_Not_Null(pProbe, saved_nEq)
    ){
      استمر; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    إذا( p_غړی->prereq_ښی & pNew->ماسک_Self ) استمر;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    إذا( p_غړی->wtFlags & غړی_LIKEOPT && p_غړی->eOperator==WO_LT ) استمر;

    إذا( (pSrc->fg.jointype & (JT_کیڼ|JT_LTORJ|JT_ښی))!=0
     && !constraintCompatibleWithOuterJoin(p_غړی,pSrc)
    ){
      استمر;
    }
    إذا( Is_Unique_شاخص(pProbe) && saved_nEq==pProbe->n_کلی_ستون-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }وإلا{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_شاخصي;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_سر = saved_n_سر;
    pNew->nLTerm = saved_nLTerm;
    إذا( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       اكسر; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = p_غړی;
    pNew->prereq = (saved_prereq | p_غړی->prereq_ښی) & ~pNew->ماسک_Self;

    أكد( n_دننه_Mul==0
        || (pNew->wsFlags & WHERE_ستنه_NULL)!=0
        || (pNew->wsFlags & WHERE_ستنه_دننه)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    إذا( eOp & WO_دننه ){
      بیان *p_بیان = p_غړی->p_بیان;
      إذا( بیان_Use_XSelect(p_بیان) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        صحيح i;
        صحيح bRedundant = 0;
        n_دننه = 46;  أكد( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        لكل(i=0; i<pNew->nLTerm-1; i++){
          إذا( pNew->aLTerm[i] && pNew->aLTerm[i]->p_بیان==p_بیان ){
            n_دننه = 0;
            إذا( pNew->aLTerm[i]->u.x.i_ډګر == p_غړی->u.x.i_ډګر ){
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
      }وإلا إذا( ALWAYS(p_بیان->x.pList && p_بیان->x.pList->n_بیان) ){
        /* "x IN (value, value, ...)" */
        n_دننه = sqlite3LogEst(p_بیان->x.pList->n_بیان);
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
        M = pProbe->ai_لیکه_Log_Est[saved_nEq];
        logK = estLog(n_دننه);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (n_دننه + r_Log_اندازه);
        إذا( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, n_دننه, r_Log_اندازه, x));
        }وإلا إذا( n_دننه_Mul<2 && OptimizationEnabled(db, SQLITE_Seek_لټون) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, n_دننه, r_Log_اندازه, x, n_دننه_Mul));
          pNew->wsFlags |= WHERE_دننه_SEEKSCAN;
        }وإلا{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, n_دننه, r_Log_اندازه, x, n_دننه_Mul));
          استمر;
        }
      }
      pNew->wsFlags |= WHERE_ستنه_دننه;
    }وإلا إذا( eOp & (WO_EQ|WO_IS) ){
      صحيح i_ستون = pProbe->ai_ستنه[saved_nEq];
      pNew->wsFlags |= WHERE_ستنه_EQ;
      أكد( saved_nEq==pNew->u.btree.nEq );
      إذا( i_ستون==XN_ROWID
       || (i_ستون>=0 && n_دننه_Mul==0 && saved_nEq==pProbe->n_کلی_ستون-1)
      ){
        إذا( i_ستون==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_کلی_ستون==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }وإلا{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      إذا( لټون.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }وإلا إذا( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_ستنه_NULL;
    }وإلا{
      صحيح nVecLen = where_Range_ویکټور_Len(
          pParse, pSrc->iCursor, pProbe, saved_nEq, p_غړی
      );
      إذا( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_ستنه_RANGE|WHERE_BTM_حد;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = p_غړی;
        p_سر_2 = 0;
        إذا( p_غړی->wtFlags & غړی_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_سر_2 = &p_غړی[1];
          أكد( (p_سر_2-(p_غړی->pWC->a))<p_غړی->pWC->n_غړی );
          أكد( p_سر_2->wtFlags & غړی_LIKEOPT );
          أكد( p_سر_2->eOperator==WO_LT );
          إذا( whereLoopResize(db, pNew, pNew->nLTerm+1) ) اكسر; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_سر_2;
          pNew->wsFlags |= WHERE_سر_حد;
          pNew->u.btree.n_سر = 1;
        }
      }وإلا{
        أكد( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_ستنه_RANGE|WHERE_سر_حد;
        pNew->u.btree.n_سر = nVecLen;
        p_سر_2 = p_غړی;
        pBtm = (pNew->wsFlags & WHERE_BTM_حد)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    أكد( pNew->n_بهر==saved_n_بهر );
    إذا( pNew->wsFlags & WHERE_ستنه_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      where_Range_لټون_Est(pParse, pBuilder, pBtm, p_سر_2, pNew);
    }وإلا{
      صحيح nEq = ++pNew->u.btree.nEq;
      أكد( eOp & (WO_ISNULL|WO_EQ|WO_دننه|WO_IS) );

      أكد( pNew->n_بهر==saved_n_بهر );
      إذا( p_غړی->truthProb<=0 && pProbe->ai_ستنه[saved_nEq]>=0 ){
        أكد( (eOp & WO_دننه) || n_دننه==0 );
        testcase( eOp & WO_دننه );
        pNew->n_بهر += p_غړی->truthProb;
        pNew->n_بهر -= n_دننه;
      }وإلا{
#که_تعریف_شوی SQLITE_ENABLE_STAT4
        tRowcnt n_بهر = 0;
        إذا( n_دننه_Mul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->n_Sample_ستون)
         && ((eOp & WO_دننه)==0 || بیان_Use_XList(p_غړی->p_بیان))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          بیان *p_بیان = p_غړی->p_بیان;
          إذا( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = where_Equal_لټون_Est(pParse, pBuilder, p_بیان->p_ښی, &n_بهر);
          }وإلا{
            rc = where_دننه_لټون_Est(pParse, pBuilder, p_بیان->x.pList, &n_بهر);
          }
          إذا( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          إذا( rc!=SQLITE_OK ) اكسر;          /* Jump out of the pTerm loop */
          إذا( n_بهر ){
            pNew->n_بهر = sqlite3LogEst(n_بهر);
            إذا( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->n_بهر+10 > pProbe->ai_لیکه_Log_Est[0]
            ){
#إذا WHERETRACE_ENABLED /* 0x01 */
              إذا( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3_Where_غړی_Print(p_غړی, 999);
              }
#د_که_پای
              p_غړی->wtFlags |= غړی_HIGHTRUTH;
              إذا( p_غړی->wtFlags & غړی_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            إذا( pNew->n_بهر>saved_n_بهر ) pNew->n_بهر = saved_n_بهر;
            pNew->n_بهر -= n_دننه;
          }
        }
        إذا( n_بهر==0 )
#د_که_پای
        {
          pNew->n_بهر += (pProbe->ai_لیکه_Log_Est[nEq] - pProbe->ai_لیکه_Log_Est[nEq-1]);
          إذا( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->n_بهر += 10;
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
    أكد( pSrc->pSTab->sz_Tab_لیکه>0 );
    إذا( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      r_لګښت_Idx = pNew->n_بهر + 16;
    }وإلا{
      r_لګښت_Idx = pNew->n_بهر + 1 + (15*pProbe->sz_Idx_لیکه)/pSrc->pSTab->sz_Tab_لیکه;
    }
    r_لګښت_Idx = sqlite3_Log_Est_جمع_کړه(r_Log_اندازه, r_لګښت_Idx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->r_پرلپسې = r_لګښت_Idx;
    إذا( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->r_پرلپسې = sqlite3_Log_Est_جمع_کړه(pNew->r_پرلپسې, pNew->n_بهر + 16);
    }
    Apply_لګښت_Multiplier(pNew->r_پرلپسې, pProbe->p_جدول->لګښت_Mult);

    n_بهر_Unadjusted = pNew->n_بهر;
    pNew->r_پرلپسې += n_دننه_Mul + n_دننه;
    pNew->n_بهر += n_دننه_Mul + n_دننه;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_اندازه);
    إذا( pSrc->fg.fromExists ) pNew->n_بهر = 0;
    rc = where_Loop_ورننباسه(pBuilder, pNew);

    إذا( pNew->wsFlags & WHERE_ستنه_RANGE ){
      pNew->n_بهر = saved_n_بهر;
    }وإلا{
      pNew->n_بهر = n_بهر_Unadjusted;
    }

    إذا( (pNew->wsFlags & WHERE_سر_حد)==0
     && pNew->u.btree.nEq<pProbe->n_ستنه
     && (pNew->u.btree.nEq<pProbe->n_کلی_ستون ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      إذا( pNew->u.btree.nEq>3 ){
        sqlite3_پرمختګ_Check(pParse);
      }
      where_Loop_جمع_کړه_Btree_شاخص(pBuilder, pSrc, pProbe, n_دننه_Mul+n_دننه);
    }
    pNew->n_بهر = saved_n_بهر;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#د_که_پای
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_سر = saved_n_سر;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->n_بهر = saved_n_بهر;
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
   && saved_nEq+1<pProbe->n_کلی_ستون
   && saved_nEq==pNew->nLTerm
   && pProbe->no_Skip_لټون==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_Skip_لټون)
   && pProbe->ai_لیکه_Log_Est[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->ai_لیکه_Log_Est[saved_nEq] - pProbe->ai_لیکه_Log_Est[saved_nEq+1];
    pNew->n_بهر -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    where_Loop_جمع_کړه_Btree_شاخص(pBuilder, pSrc, pProbe, nIter + n_دننه_Mul);
    pNew->n_بهر = saved_n_بهر;
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
  بیان_List *pOB;
  بیان_List *a_ستون_بیان;
  صحيح ii, jj;

  إذا( p_شاخص->bUnordered ) أرجع 0;
  إذا( (pOB = pBuilder->pWInfo->p_ترتیب_By)==0 ) أرجع 0;
  لكل(ii=0; ii<pOB->n_بیان; ii++){
    بیان *p_بیان = sqlite3_بیان_Skip_Collate_And_Likely(pOB->a[ii].p_بیان);
    إذا( NEVER(p_بیان==0) ) استمر;
    إذا( (p_بیان->op==TK_ستنه || p_بیان->op==TK_AGG_ستنه) 
     && p_بیان->i_جدول==iCursor 
    ){
      إذا( p_بیان->i_ستنه<0 ) أرجع 1;
      لكل(jj=0; jj<p_شاخص->n_کلی_ستون; jj++){
        إذا( p_بیان->i_ستنه==p_شاخص->ai_ستنه[jj] ) أرجع 1;
      }
    }وإلا إذا( (a_ستون_بیان = p_شاخص->a_ستون_بیان)!=0 ){
      لكل(jj=0; jj<p_شاخص->n_کلی_ستون; jj++){
        إذا( p_شاخص->ai_ستنه[jj]!=XN_بیان ) استمر;
        إذا( sqlite3_بیان_Compare_Skip(p_بیان,a_ستون_بیان->a[jj].p_بیان,iCursor)==0 ){
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
  بیان *pWhere          /* The WHERE clause from the partial index */
){
  صحيح i;
  Where_غړی *p_غړی;
  Parse *pParse;

  إذا( jointype & JT_LTORJ ) أرجع 0;
  pParse = pWC->pWInfo->pParse;
  طالما( pWhere->op==TK_AND ){
    إذا( !where_Usable_Partial_شاخص(iTab,jointype,pWC,pWhere->p_کیڼ) ) أرجع 0;
    pWhere = pWhere->p_ښی;
  }
  لكل(i=0, p_غړی=pWC->a; i<pWC->n_غړی; i++, p_غړی++){
    بیان *p_بیان;
    p_بیان = p_غړی->p_بیان;
    إذا( (!بیان_Has_Property(p_بیان, EP_OuterON) || p_بیان->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || بیان_Has_Property(p_بیان, EP_OuterON))
     && sqlite3_بیان_پایله_ورکوي_بیان(pParse, p_بیان, pWhere, iTab)
     && !sqlite3_بیان_پایله_ورکوي_بیان(pParse, p_بیان, pWhere, -1)
     && (p_غړی->wtFlags & غړی_VNULL)==0
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
ساكن صحيح بیان_Is_Covered_By_شاخص(
  ثابت بیان *p_بیان,
  ثابت شاخص *pIdx,
  صحيح iTabCur
){
  صحيح i;
  لكل(i=0; i<pIdx->n_ستنه; i++){
    إذا( pIdx->ai_ستنه[i]==XN_بیان
     && sqlite3_بیان_Compare(0, p_بیان, pIdx->a_ستون_بیان->a[i].p_بیان, iTabCur)==0
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
  u8 b_بیان;          /* Uses an indexed expression */
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
ساكن صحيح where_Is_Covering_شاخص_Walk_Callback(Walker *pWalk, بیان *p_بیان){
  صحيح i;                    /* Loop counter */
  ثابت شاخص *pIdx;        /* The index of interest */
  ثابت i16 *ai_ستنه;      /* Columns contained in the index */
  u16 n_ستنه;              /* Number of columns in the index */
  Covering_شاخص_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  إذا( (p_بیان->op==TK_ستنه || p_بیان->op==TK_AGG_ستنه) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    إذا( p_بیان->i_جدول!=pCk->iTabCur ) أرجع WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    ai_ستنه = pIdx->ai_ستنه;
    n_ستنه = pIdx->n_ستنه;
    لكل(i=0; i<n_ستنه; i++){
      إذا( ai_ستنه[i]==p_بیان->i_ستنه ) أرجع WRC_Continue;
    }
    pCk->bUnidx = 1;
    أرجع WRC_Abort;
  }وإلا إذا( pIdx->b_Has_بیان
         && بیان_Is_Covered_By_شاخص(p_بیان, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_بیان = 1;
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
  إذا( pIdx->b_Has_بیان==0 ){
    لكل(i=0; i<pIdx->n_ستنه; i++){
      إذا( pIdx->ai_ستنه[i]>=BMS-1 ) اكسر;
    }
    إذا( i>=pIdx->n_ستنه ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      أرجع 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_بیان = 0;
  ck.bUnidx = 0;
  memset(&w, 0, حجم(w));
  w.x_بیان_Callback = where_Is_Covering_شاخص_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  إذا( ck.bUnidx ){
    rc = 0;
  }وإلا إذا( ck.b_بیان ){
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
ساكن فراغ where_شاخصي_بیان_Cleanup(sqlite3 *db, فراغ *pObject){
  شاخصي_بیان **pp = (شاخصي_بیان**)pObject;
  طالما( *pp!=0 ){
    شاخصي_بیان *p = *pp;
    *pp = p->pIENext;
    sqlite3_بیان_Delete(db, p->p_بیان);
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
ساكن فراغ where_Part_Idx_بیان(
  Parse *pParse,                  /* Parse context */
  شاخص *pIdx,                    /* Partial index being processed */
  بیان *pPart,                    /* WHERE clause being processed */
  Bitmask *p_ماسک,                 /* Mask to clear bits in */
  صحيح iIdxCur,                    /* Cursor number for index */
  Src_توکی *p_توکی                  /* The FROM clause entry for the table */
){
  أكد( p_توکی==0 || (p_توکی->fg.jointype & JT_ښی)==0 );
  أكد( (p_توکی==0 || p_ماسک==0) && (p_ماسک!=0 || p_توکی!=0) );

  إذا( pPart->op==TK_AND ){
    where_Part_Idx_بیان(pParse, pIdx, pPart->p_ښی, p_ماسک, iIdxCur, p_توکی);
    pPart = pPart->p_کیڼ;
  }

  إذا( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    بیان *p_کیڼ = pPart->p_کیڼ;
    بیان *p_ښی = pPart->p_ښی;
    جدول *p_کیڼ_Tab;            /* Table of the left operand */
    ستنه *p_کیڼ_ستون;           /* Column of the left operand */
    u8 aff_ښی, aff_کیڼ;

    إذا( p_کیڼ->op!=TK_ستنه ) أرجع;
    إذا( !sqlite3_بیان_Is_Constant(0, p_ښی) ) أرجع;
    إذا( !sqlite3_Is_دوګونی(sqlite3_بیان_Compare_Coll_Seq(pParse, pPart)) ) أرجع;
    إذا( p_کیڼ->i_ستنه<0 ) أرجع;
    p_کیڼ_Tab = pIdx->p_جدول;
    p_کیڼ_ستون = &p_کیڼ_Tab->a_ستون[p_کیڼ->i_ستنه];
    aff_کیڼ = p_کیڼ_ستون->affinity;
    أكد( SQLITE_AFF_BLOB<SQLITE_AFF_متن );
    إذا( aff_کیڼ<SQLITE_AFF_متن ) أرجع;
    aff_ښی = sqlite3_بیان_Affinity(p_ښی);
    إذا( aff_ښی!=0 && aff_ښی!=aff_کیڼ ) أرجع;
    إذا( p_توکی ){
      sqlite3 *db = pParse->db;
      شاخصي_بیان *p = (شاخصي_بیان*)sqlite3DbMallocRaw(db, حجم(*p));
      إذا( p ){
        صحيح b_Null_لیکه = (p_توکی->fg.jointype&(JT_کیڼ|JT_LTORJ))!=0;
        p_ښی = sqlite3_بیان_Dup(db, p_ښی, 0);
        إذا( (p_کیڼ_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3_ستنه_Type(p_کیڼ_ستون, 0);
          أكد( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (صحيح)مزي_اوږدوالی(t.z);
          p->p_بیان = sqlite3_بیان_Alloc(db, TK_CAST, &t, 1);
          sqlite3_بیان_Attach_Subtrees(db, p->p_بیان, p_ښی, 0);
        }وإلا{
          p->p_بیان = p_ښی;
        }
        p->i_معلومات_Cur = p_توکی->iCursor;
        p->iIdxCur = iIdxCur;
        p->i_Idx_ستون = p_کیڼ->i_ستنه;
        p->b_Maybe_Null_لیکه = b_Null_لیکه;
        p->pIENext = pParse->p_Idx_Part_بیان;
        p->aff = aff_کیڼ;
        pParse->p_Idx_Part_بیان = p;
        إذا( p->pIENext==0 ){
          فراغ *pArg = (فراغ*)&pParse->p_Idx_Part_بیان;
          sqlite3_Parser_جمع_کړه_Cleanup(pParse, where_شاخصي_بیان_Cleanup, pArg);
        }
      }
    }وإلا إذا( p_کیڼ->i_ستنه<(BMS-1) ){
      *p_ماسک &= ~((Bitmask)1 << p_کیڼ->i_ستنه);
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
ساكن صحيح where_Loop_جمع_کړه_Btree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  شاخص *pProbe;              /* An index we are evaluating */
  شاخص sPk;                  /* A fake index object for the primary key */
  LogEst ai_لیکه_Est_Pk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 ai_ستنه_Pk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  Src_توکی *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  صحيح rc = SQLITE_OK;         /* Return code */
  صحيح i_ترتیبول_Idx;               /* Index number */
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

  إذا( pSrc->fg.is_شاخصي_By ){
    أكد( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }وإلا إذا( !HasRowid(pTab) ){
    إذا( pSrc->fg.not_شاخصي ){
      pProbe = sqlite3_Primary_کلی_شاخص(pTab);
    }وإلا{
      pProbe = pTab->p_شاخص;
    }
  }وإلا{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    شاخص *p_لومړی;                  /* First of real indices on the table */
    memset(&sPk, 0, حجم(شاخص));
    sPk.n_کلی_ستون = 1;
    sPk.n_ستنه = 1;
    sPk.ai_ستنه = &ai_ستنه_Pk;
    sPk.ai_لیکه_Log_Est = ai_لیکه_Est_Pk;
    sPk.onError = OE_Replace;
    sPk.p_جدول = pTab;
    sPk.sz_Idx_لیکه = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    ai_لیکه_Est_Pk[0] = pTab->n_لیکه_Log_Est;
    ai_لیکه_Est_Pk[1] = 0;
    p_لومړی = pSrc->pSTab->p_شاخص;
    sPk.p_بل = p_لومړی;
    pProbe = &sPk;
  }
  r_اندازه = pTab->n_لیکه_Log_Est;

#که_نه_تعریف_شوی SQLITE_OMIT_AUTOMATIC_شاخص
  /* Automatic indexes */
  إذا( !pBuilder->p_Or_سېټ      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_ښی_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_شاخص)!=0
   && !pSrc->fg.is_شاخصي_By  /* Has no INDEXED BY clause */
   && !pSrc->fg.not_شاخصي   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_ښی)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_اندازه;         /* Logarithm of the number of rows in the table */
    Where_غړی *p_غړی;
    Where_غړی *pWCEnd = pWC->a + pWC->n_غړی;
    r_Log_اندازه = estLog(r_اندازه);
    لكل(p_غړی=pWC->a; rc==SQLITE_OK && p_غړی<pWCEnd; p_غړی++){
      إذا( p_غړی->prereq_ښی & pNew->ماسک_Self ) استمر;
      إذا( غړی_Can_Drive_شاخص(p_غړی, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_شاخص = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = p_غړی;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->r_تیاری = r_Log_اندازه + r_اندازه;
        إذا( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->r_تیاری += 28;
        }وإلا{
          pNew->r_تیاری -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        Apply_لګښت_Multiplier(pNew->r_تیاری, pTab->لګښت_Mult);
        إذا( pNew->r_تیاری<0 ) pNew->r_تیاری = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->n_بهر = 43;  أكد( 43==sqlite3LogEst(20) );
        pNew->r_پرلپسې = sqlite3_Log_Est_جمع_کړه(r_Log_اندازه,pNew->n_بهر);
        pNew->wsFlags = WHERE_AUTO_شاخص;
        pNew->prereq = mPrereq | p_غړی->prereq_ښی;
        rc = where_Loop_ورننباسه(pBuilder, pNew);
      }
    }
  }
#د_که_پای /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  لكل(i_ترتیبول_Idx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.is_شاخصي_By||pSrc->fg.not_شاخصي)?0:pProbe->p_بل,
         i_ترتیبول_Idx++
  ){
    إذا( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_شاخص(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      استمر;  /* Partial index inappropriate for this query */
    }
    إذا( pProbe->bNoQuery ) استمر;
    r_اندازه = pProbe->ai_لیکه_Log_Est[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_سر = 0;
    pNew->u.btree.n_Distinct_ستون = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->i_ترتیبول_Idx = 0;
    pNew->r_تیاری = 0;
    pNew->prereq = mPrereq;
    pNew->n_بهر = r_اندازه;
    pNew->u.btree.p_شاخص = pProbe;
    pNew->u.btree.p_ترتیب_By = 0;
    b = شاخص_Might_Help_With_ترتیب_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    أكد( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    إذا( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->i_ترتیبول_Idx = b ? i_ترتیبول_Idx : 0;
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
#که_تعریف_شوی SQLITE_ENABLE_STAT4
      pNew->r_پرلپسې = r_اندازه + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#وإلا
      pNew->r_پرلپسې = r_اندازه + 16;
#د_که_پای
      Apply_لګښت_Multiplier(pNew->r_پرلپسې, pTab->لګښت_Mult);
      whereLoopOutputAdjust(pWC, pNew, r_اندازه);
      إذا( pSrc->fg.isSubquery ){
        إذا( pSrc->fg.له_لارې_Coroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        إذا( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.p_ترتیب_By = pSrc->u4.pSubq->pSelect->p_ترتیب_By;
        }
      }وإلا إذا( pSrc->fg.fromExists ){
        pNew->n_بهر = 0;
      }
      rc = where_Loop_ورننباسه(pBuilder, pNew);
      pNew->n_بهر = r_اندازه;
      إذا( rc ) اكسر;
    }وإلا{
      Bitmask m;
      إذا( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_شاخصي;
      }وإلا{
        m = pSrc->ستون_کارول_شوی & pProbe->ستون_Not_Idxed;
        إذا( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_بیان(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_شاخصي;
        إذا( m==TOPBIT || (pProbe->b_Has_بیان && !pProbe->bHasVCol && m!=0) ){
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
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_شاخصي;
        }
      }

      /* Full scan via index */
      إذا( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.is_شاخصي_By
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->sz_Idx_لیکه<pTab->sz_Tab_لیکه)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_Cover_Idx_لټون)
          )
      ){
        pNew->i_ترتیبول_Idx = b ? i_ترتیبول_Idx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->r_پرلپسې = r_اندازه + 1 + (15*pProbe->sz_Idx_لیکه)/pTab->sz_Tab_لیکه;
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
          لكل(ii=0; ii<pWC2->n_غړی; ii++){
            Where_غړی *p_غړی = &pWC2->a[ii];
            إذا( !sqlite3_بیان_Covered_By_شاخص(p_غړی->p_بیان, iCur, pProbe) ){
              اكسر;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            إذا( p_غړی->truthProb<=0 ){
              nLookup += p_غړی->truthProb;
            }وإلا{
              nLookup--;
              إذا( p_غړی->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->r_پرلپسې = sqlite3_Log_Est_جمع_کړه(pNew->r_پرلپسې, nLookup);
        }
        Apply_لګښت_Multiplier(pNew->r_پرلپسې, pTab->لګښت_Mult);
        whereLoopOutputAdjust(pWC, pNew, r_اندازه);
        إذا( (pSrc->fg.jointype & JT_ښی)!=0 && pProbe->a_ستون_بیان ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }وإلا{
          إذا( pSrc->fg.fromExists ) pNew->n_بهر = 0;
          rc = where_Loop_ورننباسه(pBuilder, pNew);
        }
        pNew->n_بهر = r_اندازه;
        إذا( rc ) اكسر;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_جمع_کړه_Btree_شاخص(pBuilder, pSrc, pProbe, 0);
    إذا( pBuilder->bldFlags1==SQLITE_BLDF1_شاخصي ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#د_که_پای
  }
  أرجع rc;
}

#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
ساكن صحيح is_حد_غړی(Where_غړی *p_غړی){
  أكد( p_غړی->eOperator==WO_AUX || p_غړی->e_سمون_Op==0 );
  أرجع p_غړی->e_سمون_Op>=SQLITE_شاخص_CONSTRAINT_حد
      && p_غړی->e_سمون_Op<=SQLITE_شاخص_CONSTRAINT_دننه_واټن;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
ساكن صحيح all_Constraints_کارول_شوی(
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
ساكن صحيح where_Loop_جمع_کړه_Virtual_One(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_شاخص_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  صحيح *pb_دننه,                      /* OUT: True if plan uses an IN(...) op */
  صحيح *pb_Retry_حد               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_شاخص_Info *pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  بنية sqlite3_شاخص_constraint *pIdxCons;
  بنية sqlite3_شاخص_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  صحيح i;
  صحيح mx_غړی;
  صحيح rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  Src_توکی *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  صحيح nConstraint = pIdxInfo->nConstraint;

  أكد( (mUsable & mPrereq)==mPrereq );
  *pb_دننه = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(بنية sqlite3_شاخص_constraint**)&pIdxInfo->aConstraint;
  لكل(i=0; i<nConstraint; i++, pIdxCons++){
    Where_غړی *p_غړی = غړی_From_Where_Clause(pWC, pIdxCons->i_غړی_دننه_واټن);
    pIdxCons->usable = 0;
    إذا( (p_غړی->prereq_ښی & mUsable)==p_غړی->prereq_ښی
     && (p_غړی->eOperator & mExclude)==0
     && (pb_Retry_حد || !is_حد_غړی(p_غړی))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, حجم(pUsage[0])*nConstraint);
  أكد( pIdxInfo->need_منزل_Free_Idx_Str==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_عدد = 0;
  pIdxInfo->ترتیب_By_Consumed = 0;
  pIdxInfo->estimated_لګښت = SQLITE_BIG_DBL / (مزدوج)2;
  pIdxInfo->estimated_لیکې = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->m_Handle_دننه = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_غوره_شاخص(pParse, pSrc->pSTab, pIdxInfo);
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

  mx_غړی = -1;
  أكد( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, حجم(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, حجم(pNew->u.vtab));
  pIdxCons = *(بنية sqlite3_شاخص_constraint**)&pIdxInfo->aConstraint;
  لكل(i=0; i<nConstraint; i++, pIdxCons++){
    صحيح i_غړی;
    إذا( (i_غړی = pUsage[i].argv_شاخص - 1)>=0 ){
      Where_غړی *p_غړی;
      صحيح j = pIdxCons->i_غړی_دننه_واټن;
      إذا( i_غړی>=nConstraint
       || j<0
       || (p_غړی = غړی_From_Where_Clause(pWC, j))==0
       || pNew->aLTerm[i_غړی]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_پیغام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        أرجع SQLITE_ERROR;
      }
      testcase( i_غړی==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->n_غړی-1 );
      pNew->prereq |= p_غړی->prereq_ښی;
      أكد( i_غړی<pNew->nLSlot );
      pNew->aLTerm[i_غړی] = p_غړی;
      إذا( i_غړی>mx_غړی ) mx_غړی = i_غړی;
      testcase( i_غړی==15 );
      testcase( i_غړی==16 );
      إذا( pUsage[i].omit ){
        إذا( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=i_غړی );
          pNew->u.vtab.omit_ماسک |= 1<<i_غړی;
        }وإلا{
          testcase( i!=i_غړی );
        }
        إذا( p_غړی->e_سمون_Op==SQLITE_شاخص_CONSTRAINT_دننه_واټن ){
          pNew->u.vtab.b_Omit_دننه_واټن = 1;
        }
      }
      إذا( SMASKBIT32(i) & pHidden->m_Handle_دننه ){
        pNew->u.vtab.m_Handle_دننه |= MASKBIT32(i_غړی);
      }وإلا إذا( (p_غړی->eOperator & WO_دننه)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->ترتیب_By_Consumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_شاخص_لټون_UNIQUE;
        *pb_دننه = 1; أكد( (mExclude & WO_دننه)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      أكد( pb_Retry_حد || !is_حد_غړی(p_غړی) );
      أكد( !is_حد_غړی(p_غړی) || i>=nConstraint-2 );
      أكد( !is_حد_غړی(p_غړی) || i==nConstraint-1 || is_حد_غړی(p_غړی+1) );

      إذا( is_حد_غړی(p_غړی) && (*pb_دننه || !all_Constraints_کارول_شوی(pUsage, i)) ){
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

  pNew->nLTerm = mx_غړی+1;
  لكل(i=0; i<=mx_غړی; i++){
    إذا( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_پیغام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      أرجع SQLITE_ERROR;
    }
  }
  أكد( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_عدد = pIdxInfo->idx_عدد;
  pNew->u.vtab.needFree = pIdxInfo->need_منزل_Free_Idx_Str;
  pIdxInfo->need_منزل_Free_Idx_Str = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->ترتیب_By_Consumed ?
      pIdxInfo->n_ترتیب_By : 0);
  pNew->u.vtab.b_Idx_عدد_Hex = (pIdxInfo->idxFlags&SQLITE_شاخص_لټون_HEX)!=0;
  pNew->r_تیاری = 0;
  pNew->r_پرلپسې = sqlite3LogEstFromDouble(pIdxInfo->estimated_لګښت);
  pNew->n_بهر = sqlite3LogEst(pIdxInfo->estimated_لیکې);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  إذا( pIdxInfo->idxFlags & SQLITE_شاخص_لټون_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }وإلا{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_ورننباسه(pBuilder, pNew);
  إذا( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pb_دننه, (sqlite3_uint64)mPrereq,
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
    صحيح i_غړی = pIdxInfo->aConstraint[iCons].i_غړی_دننه_واټن;
    بیان *pX = غړی_From_Where_Clause(pHidden->pWC, i_غړی)->p_بیان;
    إذا( pX->p_کیڼ ){
      pC = sqlite3_بیان_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_دوګونی);
  }
  أرجع zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
صحيح sqlite3_vtab_دننه(sqlite3_شاخص_info *pIdxInfo, صحيح iCons, صحيح bHandle){
  Hidden_شاخص_Info *pHidden = (Hidden_شاخص_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  إذا( m & pHidden->m_دننه ){
    إذا( bHandle==0 ){
      pHidden->m_Handle_دننه &= ~m;
    }وإلا إذا( bHandle>0 ){
      pHidden->m_Handle_دننه |= m;
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
صحيح sqlite3_vtab_rhs_ارزښت(
  sqlite3_شاخص_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  صحيح iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_ارزښت **ppVal           /* Write value extracted here */
){
  Hidden_شاخص_Info *pH = (Hidden_شاخص_Info*)&pIdxInfo[1];
  sqlite3_ارزښت *pVal = 0;
  صحيح rc = SQLITE_OK;
  إذا( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }وإلا{
    إذا( pH->aRhs[iCons]==0 ){
      Where_غړی *p_غړی = غړی_From_Where_Clause(
          pH->pWC, pIdxInfo->aConstraint[iCons].i_غړی_دننه_واټن
      );
      rc = sqlite3_ارزښت_From_بیان(
          pH->pParse->db, p_غړی->p_بیان->p_ښی, ENC(pH->pParse->db),
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
فراغ sqlite3_Vtab_کاروي_All_Schemas(Parse *pParse){
  صحيح nDb = pParse->db->nDb;
  صحيح i;
  لكل(i=0; i<nDb; i++){
    sqlite3_کوډ_Verify_Schema(pParse, i);
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
ساكن صحيح where_Loop_جمع_کړه_Virtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  صحيح rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  Src_توکی *pSrc;               /* The FROM clause term to search */
  sqlite3_شاخص_info *p;       /* Object to pass to xBestIndex() */
  صحيح nConstraint;             /* Number of constraints in p */
  صحيح b_دننه;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask m_غوره;               /* Tables used by best possible plan */
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
  pNew->r_تیاری = 0;
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
  rc = where_Loop_جمع_کړه_Virtual_One(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_دننه, &bRetry
  );
  إذا( bRetry ){
    أكد( rc==SQLITE_OK );
    rc = where_Loop_جمع_کړه_Virtual_One(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_دننه, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  إذا( rc==SQLITE_OK && ((m_غوره = (pNew->prereq & ~mPrereq))!=0 || b_دننه) ){
    صحيح کتل_شوی_Zero = 0;             /* True if a plan with no prereqs seen */
    صحيح کتل_شوی_Zero_No_دننه = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask m_غوره_No_دننه = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    إذا( b_دننه ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = where_Loop_جمع_کړه_Virtual_One(
          pBuilder, mPrereq, ALLBITS, WO_دننه, p, mNoOmit, &b_دننه, 0);
      أكد( b_دننه==0 );
      m_غوره_No_دننه = pNew->prereq & ~mPrereq;
      إذا( m_غوره_No_دننه==0 ){
        کتل_شوی_Zero = 1;
        کتل_شوی_Zero_No_دننه = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    طالما( rc==SQLITE_OK ){
      صحيح i;
      Bitmask m_بل = ALLBITS;
      أكد( m_بل>0 );
      لكل(i=0; i<nConstraint; i++){
        صحيح i_غړی = p->aConstraint[i].i_غړی_دننه_واټن;
        Bitmask mThis = غړی_From_Where_Clause(pWC, i_غړی)->prereq_ښی & ~mPrereq;
        إذا( mThis>mPrev && mThis<m_بل ) m_بل = mThis;
      }
      mPrev = m_بل;
      إذا( m_بل==ALLBITS ) اكسر;
      إذا( m_بل==m_غوره || m_بل==m_غوره_No_دننه ) استمر;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)m_بل));
      rc = where_Loop_جمع_کړه_Virtual_One(
          pBuilder, mPrereq, m_بل|mPrereq, 0, p, mNoOmit, &b_دننه, 0);
      إذا( pNew->prereq==mPrereq ){
        کتل_شوی_Zero = 1;
        إذا( b_دننه==0 ) کتل_شوی_Zero_No_دننه = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    إذا( rc==SQLITE_OK && کتل_شوی_Zero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = where_Loop_جمع_کړه_Virtual_One(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &b_دننه, 0);
      إذا( b_دننه==0 ) کتل_شوی_Zero_No_دننه = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    إذا( rc==SQLITE_OK && کتل_شوی_Zero_No_دننه==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = where_Loop_جمع_کړه_Virtual_One(
          pBuilder, mPrereq, mPrereq, WO_دننه, p, mNoOmit, &b_دننه, 0);
    }
  }

  free_شاخص_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  أرجع rc;
}
#د_که_پای /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
ساكن صحيح where_Loop_جمع_کړه_Or(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  Where_غړی *p_غړی, *pWCEnd;
  صحيح rc = SQLITE_OK;
  صحيح iCur;
  WhereClause لنډمهاله_WC;
  WhereLoopBuilder sSubBuild;
  Where_Or_سېټ s_مجموعه, sCur;
  Src_توکی *p_توکی;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->n_غړی;
  pNew = pBuilder->pNew;
  memset(&s_مجموعه, 0, حجم(s_مجموعه));
  p_توکی = pWInfo->pTabList->a + pNew->iTab;
  iCur = p_توکی->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  إذا( p_توکی->fg.jointype & JT_ښی ) أرجع SQLITE_OK;

  لكل(p_غړی=pWC->a; p_غړی<pWCEnd && rc==SQLITE_OK; p_غړی++){
    إذا( (p_غړی->eOperator & WO_OR)!=0
     && (p_غړی->u.pOrInfo->indexable & pNew->ماسک_Self)!=0
     && (!بیان_Has_Property(p_غړی->p_بیان, EP_OuterON)
          || p_غړی->p_بیان->w.iJoin==iCur)
    ){
      WhereClause * ثابت pOrWC = &p_غړی->u.pOrInfo->wc;
      Where_غړی * ثابت pOrWCEnd = &pOrWC->a[pOrWC->n_غړی];
      Where_غړی *p_Or_غړی;
      صحيح once = 1;
      صحيح i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.p_Or_سېټ = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", p_غړی));
      لكل(p_Or_غړی=pOrWC->a; p_Or_غړی<pOrWCEnd; p_Or_غړی++){
        إذا( (p_Or_غړی->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &p_Or_غړی->u.pAndInfo->wc;
        }وإلا إذا( p_Or_غړی->کیڼ_Cursor==iCur ){
          لنډمهاله_WC.pWInfo = pWC->pWInfo;
          لنډمهاله_WC.pOuter = pWC;
          لنډمهاله_WC.op = TK_AND;
          لنډمهاله_WC.n_غړی = 1;
          لنډمهاله_WC.n_بنسټ = 1;
          لنډمهاله_WC.a = p_Or_غړی;
          sSubBuild.pWC = &لنډمهاله_WC;
        }وإلا{
          استمر;
        }
        sCur.n = 0;
#که_تعریف_شوی WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (صحيح)(p_Or_غړی-pOrWC->a), p_غړی, sSubBuild.pWC->n_غړی));
        إذا( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
        إذا( IsVirtual(p_توکی->pSTab) ){
          rc = where_Loop_جمع_کړه_Virtual(&sSubBuild, mPrereq, mUnusable);
        }وإلا
#د_که_پای
        {
          rc = where_Loop_جمع_کړه_Btree(&sSubBuild, mPrereq);
        }
        إذا( rc==SQLITE_OK ){
          rc = where_Loop_جمع_کړه_Or(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_بشپړ );
        إذا( sCur.n==0 ){
          s_مجموعه.n = 0;
          اكسر;
        }وإلا إذا( once ){
          whereOrMove(&s_مجموعه, &sCur);
          once = 0;
        }وإلا{
          Where_Or_سېټ sPrev;
          whereOrMove(&sPrev, &s_مجموعه);
          s_مجموعه.n = 0;
          لكل(i=0; i<sPrev.n; i++){
            لكل(j=0; j<sCur.n; j++){
              where_Or_ورننباسه(&s_مجموعه, sPrev.a[i].prereq | sCur.a[j].prereq,
                            sqlite3_Log_Est_جمع_کړه(sPrev.a[i].r_پرلپسې, sCur.a[j].r_پرلپسې),
                            sqlite3_Log_Est_جمع_کړه(sPrev.a[i].n_بهر, sCur.a[j].n_بهر));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = p_غړی;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->r_تیاری = 0;
      pNew->i_ترتیبول_Idx = 0;
      memset(&pNew->u, 0, حجم(pNew->u));
      لكل(i=0; rc==SQLITE_OK && i<s_مجموعه.n; i++){
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
        pNew->r_پرلپسې = s_مجموعه.a[i].r_پرلپسې + 1;
        pNew->n_بهر = s_مجموعه.a[i].n_بهر;
        pNew->prereq = s_مجموعه.a[i].prereq;
        rc = where_Loop_ورننباسه(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", p_غړی));
    }
  }
  أرجع rc;
}

/*
** Add all WhereLoop objects for all tables
*/
ساكن صحيح where_Loop_جمع_کړه_All(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask m_مخکینی = 0;
  صحيح iTab;
  SrcList *pTabList = pWInfo->pTabList;
  Src_توکی *p_توکی;
  Src_توکی *p_پای_ټکی = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  صحيح rc = SQLITE_OK;
  صحيح b_لومړی_Past_RJ = 0;
  صحيح has_ښی_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  أكد( pNew->nLTerm==0 );
  أكد( pNew->wsFlags==0 );
  أكد( pNew->nLSlot>=Array_اندازه(pNew->aLTermSpace) );
  أكد( pNew->aLTerm!=0 );

  pBuilder->i_Plan_حد = SQLITE_QUERY_PLANNER_حد;
  لكل(iTab=0, p_توکی=pTabList->a; p_توکی<p_پای_ټکی; iTab++, p_توکی++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->i_Plan_حد += SQLITE_QUERY_PLANNER_حد_INCR;
    pNew->ماسک_Self = sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_سېټ, p_توکی->iCursor);
    إذا( b_لومړی_Past_RJ
     || (p_توکی->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
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
      إذا( p_توکی->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( p_توکی->fg.jointype & JT_LTORJ );
        testcase( p_توکی->fg.jointype & JT_CROSS );
        has_ښی_Cross_Join = 1;
      }
      mPrereq |= m_مخکینی;
      b_لومړی_Past_RJ = (p_توکی->fg.jointype & JT_ښی)!=0;
    }وإلا إذا( p_توکی->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      Where_غړی *p_غړی;
      صحيح i;
      لكل(i=pWC->n_بنسټ, p_غړی=pWC->a; i>0; i--, p_غړی++){
        إذا( (pNew->ماسک_Self & p_غړی->prereqAll)!=0 ){
          mPrereq |= (p_غړی->prereqAll & (pNew->ماسک_Self-1));
        }
      }
    }وإلا إذا( !has_ښی_Cross_Join ){
      mPrereq = 0;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
    إذا( IsVirtual(p_توکی->pSTab) ){
      Src_توکی *p;
      لكل(p=&p_توکی[1]; p<p_پای_ټکی; p++){
        إذا( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_سېټ, p->iCursor);
        }
      }
      rc = where_Loop_جمع_کړه_Virtual(pBuilder, mPrereq, mUnusable);
    }وإلا
#د_که_پای /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = where_Loop_جمع_کړه_Btree(pBuilder, mPrereq);
    }
    إذا( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = where_Loop_جمع_کړه_Or(pBuilder, mPrereq, mUnusable);
    }
    m_مخکینی |= pNew->ماسک_Self;
    إذا( rc || db->mallocFailed ){
      إذا( rc==SQLITE_بشپړ ){
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
ساكن SQLITE_NOINLINE صحيح where_لار_سمون_Subquery_OB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  صحيح iLoop,              /* Which level of the nested loop.  0==outermost */
  صحيح iCur,               /* Cursor used by the this loop */
  بیان_List *p_ترتیب_By,     /* The ORDER BY clause on the whole query */
  Bitmask *p_Rev_ماسک,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  صحيح iOB;                /* Index into pOrderBy->a[] */
  صحيح jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  بیان *pOBExpr;          /* Current term of outer ORDER BY */
  بیان_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.p_ترتیب_By;
  أكد( pSubOB!=0 );
  لكل(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  لكل(jSub=0; jSub<pSubOB->n_بیان && iOB<p_ترتیب_By->n_بیان; jSub++, iOB++){
    إذا( pSubOB->a[jSub].u.x.i_ترتیب_By_ستون==0 ) اكسر;
    pOBExpr = p_ترتیب_By->a[iOB].p_بیان;
    إذا( pOBExpr->op!=TK_ستنه && pOBExpr->op!=TK_AGG_ستنه ) اكسر;
    إذا( pOBExpr->i_جدول!=iCur ) اكسر;
    إذا( pOBExpr->i_ستنه!=pSubOB->a[jSub].u.x.i_ترتیب_By_ستون-1 ) اكسر;
    إذا( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = p_ترتیب_By->a[iOB].fg.ترتیبول_Flags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.ترتیبول_Flags;   /* sortFlags for jSub */
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
ساكن i8 where_لار_Satisfies_ترتیب_By(
  WhereInfo *pWInfo,    /* The WHERE clause */
  بیان_List *p_ترتیب_By,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  Where_لار *p_لار,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *p_وروستی,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *p_Rev_ماسک     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 rev_سېټ;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 is_ترتیب_Distinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 is_سمون;           /* iColumn matches a term of the ORDER BY clause */
  u16 eq_Op_ماسک;         /* Allowed equality operators */
  u16 n_کلی_ستون;          /* Number of key columns in pIndex */
  u16 n_ستنه;          /* Total number of ordered columns in the index */
  u16 n_ترتیب_By;         /* Number terms in the ORDER BY clause */
  صحيح iLoop;            /* Index of WhereLoop in pPath being processed */
  صحيح i, j;             /* Loop counters */
  صحيح iCur;             /* Cursor number for current WhereLoop */
  صحيح i_ستنه;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  Where_غړی *p_غړی;     /* A single term of the WHERE clause */
  بیان *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  شاخص *p_شاخص;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask ob_بشپړ;       /* Mask of all ORDER BY terms */
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

  n_ترتیب_By = p_ترتیب_By->n_بیان;
  testcase( n_ترتیب_By==BMS-1 );
  إذا( n_ترتیب_By>BMS-1 ) أرجع 0;  /* Cannot optimize overly large ORDER BYs */
  is_ترتیب_Distinct = 1;
  ob_بشپړ = MASKBIT(n_ترتیب_By)-1;
  ترتیب_Distinct_ماسک = 0;
  ready = 0;
  eq_Op_ماسک = WO_EQ | WO_IS | WO_ISNULL;
  إذا( wctrlFlags & (WHERE_ORDERBY_حد|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eq_Op_ماسک |= WO_دننه;
  }
  لكل(iLoop=0; is_ترتیب_Distinct && obSat<ob_بشپړ && iLoop<=nLoop; iLoop++){
    إذا( iLoop>0 ) ready |= pLoop->ماسک_Self;
    إذا( iLoop<nLoop ){
      pLoop = p_لار->aLoop[iLoop];
      إذا( wctrlFlags & WHERE_ORDERBY_حد ) استمر;
    }وإلا{
      pLoop = p_وروستی;
    }
    إذا( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      إذا( pLoop->u.vtab.isOrdered && pWInfo->p_ترتیب_By==p_ترتیب_By ){
        obSat = ob_بشپړ;
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
      pOBExpr = sqlite3_بیان_Skip_Collate_And_Likely(p_ترتیب_By->a[i].p_بیان);
      إذا( NEVER(pOBExpr==0) ) استمر;
      إذا( pOBExpr->op!=TK_ستنه && pOBExpr->op!=TK_AGG_ستنه ) استمر;
      إذا( pOBExpr->i_جدول!=iCur ) استمر;
      p_غړی = sqlite3_Where_پیدا_کړه_غړی(&pWInfo->sWC, iCur, pOBExpr->i_ستنه,
                       ~ready, eq_Op_ماسک, 0);
      إذا( p_غړی==0 ) استمر;
      إذا( p_غړی->eOperator==WO_دننه ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        أكد( wctrlFlags &
               (WHERE_ORDERBY_حد|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        لكل(j=0; j<pLoop->nLTerm && p_غړی!=pLoop->aLTerm[j]; j++){}
        إذا( j>=pLoop->nLTerm ) استمر;
      }
      إذا( (p_غړی->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->i_ستنه>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_بیان_NNColl_Seq(pParse, p_ترتیب_By->a[i].p_بیان);
        ثابت بیان *pCExpr = p_غړی->p_بیان;
        CollSeq *pColl2 = sqlite3_بیان_Compare_Coll_Seq(pParse, pCExpr);
        حرف aff_ښی;
        أكد( pColl1 );
        إذا( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          استمر;
        }
        aff_ښی = sqlite3_بیان_Affinity(pCExpr->p_ښی);
        /* Left operand must be a column, hence always has affinity */
        أكد( sqlite3_بیان_Affinity(pCExpr->p_کیڼ)!=0 );
        إذا( aff_ښی!=0 && aff_ښی!=sqlite3_بیان_Affinity(pCExpr->p_کیڼ) ){
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
         &&  where_لار_سمون_Subquery_OB(pLoop,iLoop,iCur,
                                     p_ترتیب_By,p_Rev_ماسک, &obSat, wctrlFlags)
        ){
          n_ستنه = 0;
          is_ترتیب_Distinct = 0;
        }وإلا{
          n_ستنه = 1;
        }
        p_شاخص = 0;
        n_کلی_ستون = 1;
      }وإلا إذا( (p_شاخص = pLoop->u.btree.p_شاخص)==0 || p_شاخص->bUnordered ){
        أرجع 0;
      }وإلا{
        n_کلی_ستون = p_شاخص->n_کلی_ستون;
        n_ستنه = p_شاخص->n_ستنه;
        أكد( n_ستنه==n_کلی_ستون+1 || !HasRowid(p_شاخص->p_جدول) );
        أكد( p_شاخص->ai_ستنه[n_ستنه-1]==XN_ROWID
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
      rev = rev_سېټ = 0;
      distinctColumns = 0;
      لكل(j=0; j<n_ستنه; j++){
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
          }وإلا إذا( ALWAYS(eOp & WO_دننه) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            بیان *pX = pLoop->aLTerm[j]->p_بیان;
            لكل(i=j+1; i<pLoop->u.btree.nEq; i++){
              إذا( pLoop->aLTerm[i]->p_بیان==pX ){
                أكد( (pLoop->aLTerm[i]->eOperator & WO_دننه) );
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
          i_ستنه = p_شاخص->ai_ستنه[j];
          revIdx = p_شاخص->a_ترتیبول_ترتیب[j] & KEYINFO_ترتیب_DESC;
          إذا( i_ستنه==p_شاخص->p_جدول->iPKey ) i_ستنه = XN_ROWID;
        }وإلا{
          i_ستنه = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        إذا( is_ترتیب_Distinct ){
          إذا( i_ستنه>=0
           && j>=pLoop->u.btree.nEq
           && p_شاخص->p_جدول->a_ستون[i_ستنه].notNull==0
          ){
            is_ترتیب_Distinct = 0;
          }
          إذا( i_ستنه==XN_بیان ){
            is_ترتیب_Distinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        is_سمون = 0;
        لكل(i=0; bOnce && i<n_ترتیب_By; i++){
          إذا( MASKBIT(i) & obSat ) استمر;
          pOBExpr = sqlite3_بیان_Skip_Collate_And_Likely(p_ترتیب_By->a[i].p_بیان);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          إذا( NEVER(pOBExpr==0) ) استمر;
          إذا( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          إذا( i_ستنه>=XN_ROWID ){
            إذا( pOBExpr->op!=TK_ستنه && pOBExpr->op!=TK_AGG_ستنه ) استمر;
            إذا( pOBExpr->i_جدول!=iCur ) استمر;
            إذا( pOBExpr->i_ستنه!=i_ستنه ) استمر;
          }وإلا{
            بیان *p_Ix_بیان = p_شاخص->a_ستون_بیان->a[j].p_بیان;
            إذا( sqlite3_بیان_Compare_Skip(pOBExpr, p_Ix_بیان, iCur) ){
              استمر;
            }
          }
          إذا( i_ستنه!=XN_ROWID ){
            pColl = sqlite3_بیان_NNColl_Seq(pWInfo->pParse, p_ترتیب_By->a[i].p_بیان);
            إذا( sqlite3StrICmp(pColl->zName, p_شاخص->azColl[j])!=0 ) استمر;
          }
          إذا( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.n_Distinct_ستون = j+1;
          }
          is_سمون = 1;
          اكسر;
        }
        إذا( is_سمون && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          إذا( rev_سېټ ){
            إذا( (rev ^ revIdx)
                           != (p_ترتیب_By->a[i].fg.ترتیبول_Flags&KEYINFO_ترتیب_DESC)
            ){
              is_سمون = 0;
            }
          }وإلا{
            rev = revIdx ^ (p_ترتیب_By->a[i].fg.ترتیبول_Flags & KEYINFO_ترتیب_DESC);
            إذا( rev ) *p_Rev_ماسک |= MASKBIT(iLoop);
            rev_سېټ = 1;
          }
        }
        إذا( is_سمون && (p_ترتیب_By->a[i].fg.ترتیبول_Flags & KEYINFO_ترتیب_BIGNULL) ){
          إذا( j==pLoop->u.btree.nEq 
           && (j<n_کلی_ستون||HasRowid(p_شاخص->p_جدول)||!Is_Primary_کلی_شاخص(p_شاخص))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_ترتیبول;
          }وإلا{
            is_سمون = 0;
          }
        }
        إذا( is_سمون ){
          إذا( i_ستنه==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }وإلا{
          /* No match found */
          أكد( n_کلی_ستون>0 );
          إذا( j<n_کلی_ستون ){
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
        بیان *p;
        Bitmask m_غړی;
        إذا( MASKBIT(i) & obSat ) استمر;
        p = p_ترتیب_By->a[i].p_بیان;
        m_غړی = sqlite3_Where_بیان_Usage(&pWInfo->s_ماسک_سېټ,p);
        إذا( m_غړی==0 && !sqlite3_بیان_Is_Constant(0,p) ) استمر;
        إذا( (m_غړی&~ترتیب_Distinct_ماسک)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  إذا( obSat==ob_بشپړ ) أرجع (i8)n_ترتیب_By;
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

#که_تعریف_شوی WHERETRACE_ENABLED
/* For debugging use only: */
ساكن ثابت حرف *where_لار_Name(Where_لار *p_لار, صحيح nLoop, WhereLoop *p_وروستی){
  ساكن حرف zName[65];
  صحيح i;
  لكل(i=0; i<nLoop; i++){ zName[i] = p_لار->aLoop[i]->cId; }
  إذا( p_وروستی ) zName[i++] = p_وروستی->cId;
  zName[i] = 0;
  أرجع zName;
}
#د_که_پای

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
ساكن LogEst where_ترتیبونه_لګښت(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst n_لیکه,       /* Estimated number of rows to sort */
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
  LogEst r_ترتیبول_لګښت, n_ستون;
  أكد( pWInfo->pSelect!=0 );
  أكد( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  n_ستون = sqlite3LogEst((pWInfo->pSelect->pEList->n_بیان+59)/30);
  r_ترتیبول_لګښت = n_لیکه + n_ستون;
  إذا( nSorted>0 ){
    /* Scale the result by (Y/X) */
    r_ترتیبول_لګښت += sqlite3LogEst((n_ترتیب_By-nSorted)*100/n_ترتیب_By) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  إذا( (pWInfo->wctrlFlags & WHERE_USE_حد)!=0 ){
    r_ترتیبول_لګښت += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    إذا( nSorted!=0 ){
      r_ترتیبول_لګښت += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    إذا( pWInfo->i_حد<n_لیکه ){
      n_لیکه = pWInfo->i_حد;
    }
  }وإلا إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    إذا( n_لیکه>10 ){ n_لیکه -= 10;  أكد( 10==sqlite3LogEst(2) ); }
  }
  r_ترتیبول_لګښت += estLog(n_لیکه);
  أرجع r_ترتیبول_لګښت;
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

#که_تعریف_شوی SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بل_Loop){
    أكد( pWLoop->ماسک_Self==MASKBIT(pWLoop->iTab) );
    أكد( pWLoop->p_بل_Loop==0 || pWLoop->iTab<=pWLoop->p_بل_Loop->iTab );
  }
#د_که_پای /* SQLITE_DEBUG */

  إذا( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->b_ستوری_بشپړ
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_ستوری_Query)
  ){
    Src_توکی *aFromTabs;    /* All terms of the FROM clause */
    صحيح iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *p_پیل;     /* Where to start searching for dimension-tables */

    pWInfo->b_ستوری_بشپړ = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    أكد( !pWInfo->b_ستوری_کارول_شوی );
    aFromTabs = pWInfo->pTabList->a;
    p_پیل = pWInfo->pLoops;
    لكل(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      صحيح nDep = 0;             /* Number of dimension tables */
      LogEst mx_پرلپسې;             /* Maximum SCAN cost of a fact table */
      Bitmask m_کتل_شوی = 0;        /* Mask of dimension tables */
      Src_توکی *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      إذا( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        إذا( iFromIdx+3 > nLoop ){
          اكسر;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        طالما( p_پیل && p_پیل->iTab<=iFromIdx ){
          p_پیل = p_پیل->p_بل_Loop;
        }
      }
      لكل(pWLoop=p_پیل; pWLoop; pWLoop=pWLoop->p_بل_Loop){
        إذا( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          اكسر; /* Constraint (bb) */
        }
        إذا( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->ماسک_Self & m_کتل_شوی)==0  /* pWInfo not already a dependency */
         && (pWLoop->ماسک_Self & mSelfJoin)==0 /* Not a self-join */
        ){
          إذا( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }وإلا{
            nDep++;
            m_کتل_شوی |= pWLoop->ماسک_Self;
          }
        }
      }
      إذا( nDep<=2 ){
        استمر; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#که_تعریف_شوی WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      إذا( !pWInfo->b_ستوری_کارول_شوی ){
        لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بل_Loop){
          pWLoop->r_ستوری_Delta = 0;
        }
      }
#د_که_پای
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x80000 */
      إذا( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = m_کتل_شوی;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        لكل(pWLoop=p_پیل; pWLoop; pWLoop=pWLoop->p_بل_Loop){
          إذا( mShow & pWLoop->ماسک_Self ){
            Src_توکی *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->ماسک_Self;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#د_که_پای
      pWInfo->b_ستوری_کارول_شوی = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mx_پرلپسې = LOGEST_MIN;
      لكل(pWLoop=p_پیل; pWLoop; pWLoop=pWLoop->p_بل_Loop){
        إذا( pWLoop->iTab<iFromIdx ) استمر;
        إذا( pWLoop->iTab>iFromIdx ) اكسر;
        إذا( pWLoop->r_پرلپسې>mx_پرلپسې ) mx_پرلپسې = pWLoop->r_پرلپسې;
      }
      إذا( ALWAYS(mx_پرلپسې<LOGEST_MAX) ) mx_پرلپسې++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      لكل(pWLoop=p_پیل; pWLoop; pWLoop=pWLoop->p_بل_Loop){
        إذا( (pWLoop->ماسک_Self & m_کتل_شوی)==0 ) استمر;
        إذا( pWLoop->nLTerm ) استمر;
        إذا( pWLoop->r_پرلپسې<mx_پرلپسې ){
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x80000 */
          إذا( sqlite3WhereTrace & 0x80000 ){
            Src_توکی *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mx_پرلپسې
            );
          }
          pWLoop->r_ستوری_Delta = mx_پرلپسې - pWLoop->r_پرلپسې;
#د_که_پای /* WHERETRACE_ENABLED */
          pWLoop->r_پرلپسې = mx_پرلپسې;
        }
      }
    }
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x80000 */
    إذا( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->b_ستوری_کارول_شوی ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بل_Loop){
        إذا( pWLoop->r_ستوری_Delta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#د_که_پای
  }
  أرجع pWInfo->b_ستوری_کارول_شوی ? 18 : 12;
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
  إذا( (pCandidate->wsFlags & WHERE_شاخصي)==0 ) أرجع 1;
  إذا( (pBaseline->wsFlags & WHERE_شاخصي)==0 ) أرجع 1;
  إذا( pCandidate->u.btree.p_شاخص->sz_Idx_لیکه <
        pBaseline->u.btree.p_شاخص->sz_Idx_لیکه ) أرجع 0;
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
ساكن صحيح where_لار_Solver(WhereInfo *pWInfo, LogEst n_لیکه_Est){
  صحيح mxChoice;             /* Maximum number of simultaneous paths tracked */
  صحيح nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  صحيح iLoop;                /* Loop counter over the terms of the join */
  صحيح ii, jj;               /* Loop counters */
  صحيح mxI = 0;              /* Index of next entry to replace */
  صحيح n_ترتیب_By;             /* Number of ORDER BY clause terms */
  LogEst mx_لګښت = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  صحيح n_منزل, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  Where_لار *aFrom;         /* All nFrom paths at the previous level */
  Where_لار *a_منزل;           /* The nTo best paths at the current level */
  Where_لار *pFrom;         /* An element of aFrom[] that we are working on */
  Where_لار *p_منزل;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *a_ترتیبول_لګښت = 0;    /* Sorting and partial sorting costs */
  حرف *pSpace;             /* Temporary memory used by this routine */
  صحيح nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     n_لیکه_Est, pParse->nQueryLoop));
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
  إذا( pWInfo->p_ترتیب_By==0 || n_لیکه_Est==0 ){
    n_ترتیب_By = 0;
  }وإلا{
    n_ترتیب_By = pWInfo->p_ترتیب_By->n_بیان;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (حجم(Where_لار)+حجم(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += حجم(LogEst) * n_ترتیب_By;
  pSpace = sqlite3_ډېری_Alloc_Raw_NN(pParse->db, nSpace);
  إذا( pSpace==0 ) أرجع SQLITE_NOMEM_BKPT;
  a_منزل = (Where_لار*)pSpace;
  aFrom = a_منزل+mxChoice;
  memset(aFrom, 0, حجم(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  لكل(ii=mxChoice*2, pFrom=a_منزل; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  إذا( n_ترتیب_By ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    a_ترتیبول_لګښت = (LogEst*)pX;
    memset(a_ترتیبول_لګښت, 0, حجم(LogEst) * n_ترتیب_By);
  }
  أكد( a_ترتیبول_لګښت==0 || &pSpace[nSpace]==(حرف*)&a_ترتیبول_لګښت[n_ترتیب_By] );
  أكد( a_ترتیبول_لګښت!=0 || &pSpace[nSpace]==(حرف*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].n_لیکه = MIN(pParse->nQueryLoop, 48);  أكد( 48==sqlite3LogEst(28) );
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
    n_منزل = 0;
    لكل(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      لكل(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_بل_Loop){
        LogEst n_بهر;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst r_لګښت;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask ماسک_New;                  /* Mask of src visited by (..) */
        Bitmask rev_ماسک;                  /* Mask of rev-order loops for (..) */

        إذا( (pWLoop->prereq & ~pFrom->ماسک_Loop)!=0 ) استمر;
        إذا( (pWLoop->ماسک_Self & pFrom->ماسک_Loop)!=0 ) استمر;
        إذا( (pWLoop->wsFlags & WHERE_AUTO_شاخص)!=0 && pFrom->n_لیکه<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          أكد( 10==sqlite3LogEst(2) );
          استمر;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->r_پرلپسې + pFrom->n_لیکه;
        إذا( pWLoop->r_تیاری ){
          rUnsort = sqlite3_Log_Est_جمع_کړه(pWLoop->r_تیاری, rUnsort);
        }
        rUnsort = sqlite3_Log_Est_جمع_کړه(rUnsort, pFrom->rUnsort);
        n_بهر = pFrom->n_لیکه + pWLoop->n_بهر;
        ماسک_New = pFrom->ماسک_Loop | pWLoop->ماسک_Self;
        isOrdered = pFrom->isOrdered;
        إذا( isOrdered<0 ){
          rev_ماسک = 0;
          isOrdered = where_لار_Satisfies_ترتیب_By(pWInfo,
                       pWInfo->p_ترتیب_By, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &rev_ماسک);
        }وإلا{
          rev_ماسک = pFrom->revLoop;
        }
        إذا( isOrdered>=0 && isOrdered<n_ترتیب_By ){
          إذا( a_ترتیبول_لګښت[isOrdered]==0 ){
            a_ترتیبول_لګښت[isOrdered] = where_ترتیبونه_لګښت(
                pWInfo, n_لیکه_Est, n_ترتیب_By, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          r_لګښت = sqlite3_Log_Est_جمع_کړه(rUnsort, a_ترتیبول_لګښت[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               a_ترتیبول_لګښت[isOrdered], (n_ترتیب_By-isOrdered), n_ترتیب_By,
               rUnsort, r_لګښت));
        }وإلا{
          r_لګښت = rUnsort;
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
        testcase( n_منزل==0 );
        لكل(jj=0, p_منزل=a_منزل; jj<n_منزل; jj++, p_منزل++){
          إذا( p_منزل->ماسک_Loop==ماسک_New
           && ( ((p_منزل->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==n_منزل-1 );
            اكسر;
          }
        }
        إذا( jj>=n_منزل ){
          /* None of the existing best-so-far paths match the candidate. */
          إذا( n_منزل>=mxChoice
           && (r_لګښت>mx_لګښت || (r_لګښت==mx_لګښت && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x4 */
            إذا( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  where_لار_Name(pFrom, iLoop, pWLoop), r_لګښت, n_بهر, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#د_که_پای
            استمر;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          إذا( n_منزل<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = n_منزل++;
          }وإلا{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          p_منزل = &a_منزل[jj];
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x4 */
          إذا( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                where_لار_Name(pFrom, iLoop, pWLoop), r_لګښت, n_بهر, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#د_که_پای
        }وإلا{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          إذا( (p_منزل->r_لګښت<r_لګښت)
           || (p_منزل->r_لګښت==r_لګښت && p_منزل->n_لیکه<n_بهر)
           || (p_منزل->r_لګښت==r_لګښت && p_منزل->n_لیکه==n_بهر && p_منزل->rUnsort<rUnsort)
           || (p_منزل->r_لګښت==r_لګښت && p_منزل->n_لیکه==n_بهر && p_منزل->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, p_منزل->aLoop[iLoop]) )
          ){
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x4 */
            إذا( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  where_لار_Name(pFrom, iLoop, pWLoop), r_لګښت, n_بهر, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  where_لار_Name(p_منزل, iLoop+1, 0), p_منزل->r_لګښت, p_منزل->n_لیکه,
                  p_منزل->rUnsort, p_منزل->isOrdered>=0 ? p_منزل->isOrdered+'0' : '?');
            }
#د_که_پای
            /* Discard the candidate path from further consideration */
            testcase( p_منزل->r_لګښت==r_لګښت );
            استمر;
          }
          testcase( p_منزل->r_لګښت==r_لګښت+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#که_تعریف_شوی WHERETRACE_ENABLED /* 0x4 */
          إذا( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                where_لار_Name(pFrom, iLoop, pWLoop), r_لګښت, n_بهر, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                where_لار_Name(p_منزل, iLoop+1, 0), p_منزل->r_لګښت, p_منزل->n_لیکه,
                p_منزل->rUnsort, p_منزل->isOrdered>=0 ? p_منزل->isOrdered+'0' : '?');
          }
#د_که_پای
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        p_منزل->ماسک_Loop = pFrom->ماسک_Loop | pWLoop->ماسک_Self;
        p_منزل->revLoop = rev_ماسک;
        p_منزل->n_لیکه = n_بهر;
        p_منزل->r_لګښت = r_لګښت;
        p_منزل->rUnsort = rUnsort;
        p_منزل->isOrdered = isOrdered;
        حافظه_نقل(p_منزل->aLoop, pFrom->aLoop, حجم(WhereLoop*)*iLoop);
        p_منزل->aLoop[iLoop] = pWLoop;
        إذا( n_منزل>=mxChoice ){
          mxI = 0;
          mx_لګښت = a_منزل[0].r_لګښت;
          mxUnsort = a_منزل[0].n_لیکه;
          لكل(jj=1, p_منزل=&a_منزل[1]; jj<mxChoice; jj++, p_منزل++){
            إذا( p_منزل->r_لګښت>mx_لګښت
             || (p_منزل->r_لګښت==mx_لګښت && p_منزل->rUnsort>mxUnsort)
            ){
              mx_لګښت = p_منزل->r_لګښت;
              mxUnsort = p_منزل->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#که_تعریف_شوی WHERETRACE_ENABLED  /* >=2 */
    إذا( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      صحيح n_بشپړ = 0;
      صحيح n_پرمختګ;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      افعل{
        n_پرمختګ = 0;
        rMin = 0x7fff;
        لكل(ii=0, p_منزل=a_منزل; ii<n_منزل; ii++, p_منزل++){
          إذا( p_منزل->r_لګښت>rFloor && p_منزل->r_لګښت<rMin ) rMin = p_منزل->r_لګښت;
        }
        لكل(ii=0, p_منزل=a_منزل; ii<n_منزل; ii++, p_منزل++){
          إذا( p_منزل->r_لګښت==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               where_لار_Name(p_منزل, iLoop+1, 0), p_منزل->r_لګښت, p_منزل->n_لیکه,
               p_منزل->isOrdered>=0 ? (p_منزل->isOrdered+'0') : '?');
            إذا( p_منزل->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", p_منزل->revLoop);
            }وإلا{
              sqlite3DebugPrintf("\n");
            }
            n_بشپړ++;
            n_پرمختګ++;
          }
        }
        rFloor = rMin;
      }طالما( n_بشپړ<n_منزل && n_پرمختګ>0 );
    }
#د_که_پای

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = a_منزل;
    a_منزل = aFrom;
    aFrom = pFrom;
    nFrom = n_منزل;
  }

  إذا( nFrom==0 ){
    sqlite3_Error_پیغام(pParse, "no query solution");
    sqlite3_ډېری_Free_NN(pParse->db, pSpace);
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
   && n_لیکه_Est
  ){
    Bitmask not_کارول_شوی;
    صحيح rc = where_لار_Satisfies_ترتیب_By(pWInfo, pWInfo->p_پایله_سېټ, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &not_کارول_شوی);
    إذا( rc==pWInfo->p_پایله_سېټ->n_بیان ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  إذا( pWInfo->p_ترتیب_By ){
    pWInfo->nOBSat = pFrom->isOrdered;
    إذا( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      إذا( pFrom->isOrdered==pWInfo->p_ترتیب_By->n_بیان ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      أكد( pWInfo->pSelect->p_ترتیب_By==0
           || pWInfo->nOBSat <= pWInfo->pSelect->p_ترتیب_By->n_بیان );
    }وإلا{
      pWInfo->rev_ماسک = pFrom->revLoop;
      إذا( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        إذا( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          إذا( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_ستنه_دننه))!=(WHERE_IPK|WHERE_ستنه_دننه)
          ){
            Bitmask m = 0;
            صحيح rc = where_لار_Satisfies_ترتیب_By(pWInfo, pWInfo->p_ترتیب_By, pFrom,
                      WHERE_ORDERBY_حد, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_ستنه_دننه );
            إذا( rc==pWInfo->p_ترتیب_By->n_بیان ){
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
        && pWInfo->nOBSat==pWInfo->p_ترتیب_By->n_بیان && nLoop>0
    ){
      Bitmask rev_ماسک = 0;
      صحيح n_ترتیب = where_لار_Satisfies_ترتیب_By(pWInfo, pWInfo->p_ترتیب_By,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &rev_ماسک
      );
      أكد( pWInfo->sorted==0 );
      إذا( n_ترتیب==pWInfo->p_ترتیب_By->n_بیان ){
        pWInfo->sorted = 1;
        pWInfo->rev_ماسک = rev_ماسک;
      }
    }
  }

  pWInfo->n_لیکه_بهر = pFrom->n_لیکه;
#که_تعریف_شوی WHERETRACE_ENABLED
  pWInfo->r_ټول_لګښت = pFrom->r_لګښت;
#د_که_پای

  /* Free temporary memory and return success */
  sqlite3_ډېری_Free_NN(pParse->db, pSpace);
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
#که_تعریف_شوی WHERETRACE_ENABLED
  صحيح once = 0;
#د_که_پای
  لكل(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    إذا( p==0 ) اكسر;
    إذا( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      اكسر;
    }
    إذا( (p->wsFlags & (WHERE_ستنه_EQ|WHERE_ستنه_NULL|WHERE_ستنه_دننه))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      لكل(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_بل_Loop){
        إذا( pLoop->iTab!=iTab ) استمر;
        إذا( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_شاخص))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          استمر;
        }
#که_تعریف_شوی WHERETRACE_ENABLED
        إذا( sqlite3WhereTrace & 0x80 ){
          إذا( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#د_که_پای /* WHERETRACE_ENABLED */
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
  Src_توکی *p_توکی;
  WhereClause *pWC;
  Where_غړی *p_غړی;
  WhereLoop *pLoop;
  صحيح iCur;
  صحيح j;
  جدول *pTab;
  شاخص *pIdx;
  Where_لټون لټون;

  pWInfo = pBuilder->pWInfo;
  إذا( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) أرجع 0;
  أكد( pWInfo->pTabList->nSrc>=1 );
  p_توکی = pWInfo->pTabList->a;
  pTab = p_توکی->pSTab;
  إذا( IsVirtual(pTab) ) أرجع 0;
  إذا( p_توکی->fg.is_شاخصي_By || p_توکی->fg.not_شاخصي ){
    testcase( p_توکی->fg.is_شاخصي_By );
    testcase( p_توکی->fg.not_شاخصي );
    أرجع 0;
  }
  iCur = p_توکی->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  p_غړی = where_لټون_Init(&لټون, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  طالما( p_غړی && p_غړی->prereq_ښی ) p_غړی = where_لټون_بل(&لټون);
  إذا( p_غړی ){
    testcase( p_غړی->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_ستنه_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = p_غړی;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->r_پرلپسې = 33;  /* 33==sqlite3LogEst(10) */
  }وإلا{
    لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
      صحيح op_ماسک;
      أكد( pLoop->aLTermSpace==pLoop->aLTerm );
      إذا( !Is_Unique_شاخص(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_کلی_ستون>Array_اندازه(pLoop->aLTermSpace)
      ) استمر;
      op_ماسک = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      لكل(j=0; j<pIdx->n_کلی_ستون; j++){
        p_غړی = where_لټون_Init(&لټون, pWC, iCur, j, op_ماسک, pIdx);
        طالما( p_غړی && p_غړی->prereq_ښی ) p_غړی = where_لټون_بل(&لټون);
        إذا( p_غړی==0 ) اكسر;
        testcase( p_غړی->eOperator & WO_IS );
        pLoop->aLTerm[j] = p_غړی;
      }
      إذا( j!=pIdx->n_کلی_ستون ) استمر;
      pLoop->wsFlags = WHERE_ستنه_EQ|WHERE_ONEROW|WHERE_شاخصي;
      إذا( pIdx->isCovering || (p_توکی->ستون_کارول_شوی & pIdx->ستون_Not_Idxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_شاخص = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->r_پرلپسې = 39;  /* 39==sqlite3LogEst(15) */
      اكسر;
    }
  }
  إذا( pLoop->wsFlags ){
    pLoop->n_بهر = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    أكد( pWInfo->s_ماسک_سېټ.n==1 && iCur==pWInfo->s_ماسک_سېټ.ix[0] );
    pLoop->ماسک_Self = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->n_لیکه_بهر = 1;
    إذا( pWInfo->p_ترتیب_By ) pWInfo->nOBSat =  pWInfo->p_ترتیب_By->n_بیان;
    إذا( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    إذا( لټون.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#که_تعریف_شوی SQLITE_DEBUG
    pLoop->cId = '0';
#د_که_پای
#که_تعریف_شوی WHERETRACE_ENABLED
    إذا( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#د_که_پای
    أرجع 1;
  }
  أرجع 0;
}

/*
** Helper function for exprIsDeterministic().
*/
ساكن صحيح بیان_غوټه_Is_Deterministic(Walker *pWalker, بیان *p_بیان){
  إذا( p_بیان->op==TK_FUNCTION && بیان_Has_Property(p_بیان, EP_ConstFunc)==0 ){
    pWalker->e_کوډ = 0;
    أرجع WRC_Abort;
  }
  أرجع WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
ساكن صحيح بیان_Is_Deterministic(بیان *p){
  Walker w;
  memset(&w, 0, حجم(w));
  w.e_کوډ = 1;
  w.x_بیان_Callback = بیان_غوټه_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_بیان(&w, p);
  أرجع w.e_کوډ;
}

 
#که_تعریف_شوی WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
ساكن فراغ showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  إذا( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    صحيح i;
    ساكن ثابت حرف zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    لكل(p=pWInfo->pLoops, i=0; p; p=p->p_بل_Loop, i++){
      p->cId = zLabel[i%(حجم(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#وإلا
# define WHERETRACE_ALL_LOOPS(W,C)
#د_که_پای

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
  Bitmask tab_کارول_شوی;
  صحيح has_ښی_Join;

  /* Preconditions checked by the caller */
  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  أكد( pWInfo->p_پایله_سېټ!=0 );
  أكد( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tab_کارول_شوی = sqlite3_Where_بیان_List_Usage(&pWInfo->s_ماسک_سېټ, pWInfo->p_پایله_سېټ);
  إذا( pWInfo->p_ترتیب_By ){
    tab_کارول_شوی |= sqlite3_Where_بیان_List_Usage(&pWInfo->s_ماسک_سېټ, pWInfo->p_ترتیب_By);
  }
  has_ښی_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  لكل(i=pWInfo->nLevel-1; i>=1; i--){
    Where_غړی *p_غړی, *p_پای_ټکی;
    Src_توکی *p_توکی;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    p_توکی = &pWInfo->pTabList->a[pLoop->iTab];
    إذا( (p_توکی->fg.jointype & (JT_کیڼ|JT_ښی))!=JT_کیڼ ) استمر;
    إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      استمر;
    }
    إذا( (tab_کارول_شوی & pLoop->ماسک_Self)!=0 ) استمر;
    p_پای_ټکی = pWInfo->sWC.a + pWInfo->sWC.n_غړی;
    لكل(p_غړی=pWInfo->sWC.a; p_غړی<p_پای_ټکی; p_غړی++){
      إذا( (p_غړی->prereqAll & pLoop->ماسک_Self)!=0 ){
        إذا( !بیان_Has_Property(p_غړی->p_بیان, EP_OuterON)
         || p_غړی->p_بیان->w.iJoin!=p_توکی->iCursor
        ){
          اكسر;
        }
      }
      إذا( has_ښی_Join
       && بیان_Has_Property(p_غړی->p_بیان, EP_InnerON)
       && NEVER(p_غړی->p_بیان->w.iJoin==p_توکی->iCursor)
      ){
        اكسر;  /* restriction (5) */
      }
    }
    إذا( p_غړی<p_پای_ټکی ) استمر;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->rev_ماسک>>1) & ~m1)!=0 );
    pWInfo->rev_ماسک = (m1 & pWInfo->rev_ماسک) | ((pWInfo->rev_ماسک>>1) & ~m1);
    notReady &= ~pLoop->ماسک_Self;
    لكل(p_غړی=pWInfo->sWC.a; p_غړی<p_پای_ټکی; p_غړی++){
      إذا( (p_غړی->prereqAll & pLoop->ماسک_Self)!=0 ){
        p_غړی->wtFlags |= غړی_CODED;
        p_غړی->prereqAll = 0;
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
  LogEst n_پلټنه = 0;

  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  لكل(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ثابت بینښه صحيح reqFlags = (WHERE_SELFCULL|WHERE_ستنه_EQ);
    Src_توکی *p_توکی = &pWInfo->pTabList->a[pLoop->iTab];
    جدول *pTab = p_توکی->pSTab;
    إذا( (pTab->tabFlags & TF_HasStat1)==0 ) اكسر;
    pTab->tabFlags |= TF_MaybeReanalyze;
    إذا( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_پلټنه > pTab->n_لیکه_Log_Est
    ){
      testcase( p_توکی->fg.jointype & JT_کیڼ );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (مزدوج)sqlite3_Log_Est_منزل_Int(n_پلټنه), pTab->zName,
         (مزدوج)sqlite3_Log_Est_منزل_Int(pTab->n_لیکه_Log_Est)));
    }
    n_پلټنه += pLoop->n_بهر;
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
ساكن SQLITE_NOINLINE فراغ where_جمع_کړه_شاخصي_بیان(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  شاخص *pIdx,       /* The index-on-expression that contains the expressions */
  صحيح iIdxCur,       /* Cursor number for pIdx */
  Src_توکی *p_Tab_توکی  /* The FROM clause entry for the table */
){
  صحيح i;
  شاخصي_بیان *p;
  جدول *pTab;
  أكد( pIdx->b_Has_بیان );
  pTab = pIdx->p_جدول;
  لكل(i=0; i<pIdx->n_ستنه; i++){
    بیان *p_بیان;
    صحيح j = pIdx->ai_ستنه[i];
    إذا( j==XN_بیان ){
      p_بیان = pIdx->a_ستون_بیان->a[i].p_بیان;
    }وإلا إذا( j>=0 && (pTab->a_ستون[j].ستون_Flags & COLFLAG_VIRTUAL)!=0 ){
      p_بیان = sqlite3_ستنه_بیان(pTab, &pTab->a_ستون[j]);
    }وإلا{
      استمر;
    }
    إذا( sqlite3_بیان_Is_Constant(0,p_بیان) ) استمر;
    p = sqlite3DbMallocRaw(pParse->db,  حجم(شاخصي_بیان));
    إذا( p==0 ) اكسر;
    p->pIENext = pParse->pIdxEpr;
#که_تعریف_شوی WHERETRACE_ENABLED
    إذا( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      إذا( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_بیان(p_بیان);
    }
#د_که_پای
    p->p_بیان = sqlite3_بیان_Dup(pParse->db, p_بیان, 0);
    p->i_معلومات_Cur = p_Tab_توکی->iCursor;
    p->iIdxCur = iIdxCur;
    p->i_Idx_ستون = i;
    p->b_Maybe_Null_لیکه = (p_Tab_توکی->fg.jointype & (JT_کیڼ|JT_LTORJ|JT_ښی))!=0;
    إذا( sqlite3_شاخص_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->z_ستون_Aff[i];
    }
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#د_که_پای
    pParse->pIdxEpr = p;
    إذا( p->pIENext==0 ){
      فراغ *pArg = (فراغ*)&pParse->pIdxEpr;
      sqlite3_Parser_جمع_کړه_Cleanup(pParse, where_شاخصي_بیان_Cleanup, pArg);
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
ساكن SQLITE_NOINLINE فراغ where_سرچپه_لټون_ترتیب(WhereInfo *pWInfo){
  صحيح ii;
  لكل(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    Src_توکی *p_توکی = &pWInfo->pTabList->a[ii];
    إذا( !p_توکی->fg.isCte
     || p_توکی->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(p_توکی->fg.isSubquery==0)
     || p_توکی->u4.pSubq->pSelect->p_ترتیب_By==0
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
  بیان *pWhere,           /* The WHERE clause */
  بیان_List *p_ترتیب_By,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  بیان_List *p_پایله_سېټ,   /* Query result set.  Req'd for DISTINCT */
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
  Where_ماسک_سېټ *p_ماسک_سېټ;    /* The expression mask set */
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
  testcase( p_ترتیب_By && p_ترتیب_By->n_بیان==BMS-1 );
  إذا( p_ترتیب_By && p_ترتیب_By->n_بیان>=BMS ){
    p_ترتیب_By = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  إذا( pTabList->nSrc>BMS ){
    sqlite3_Error_پیغام(pParse, "at most %d tables in a join", BMS);
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
#د_که_پای
  pWInfo->p_پایله_سېټ = p_پایله_سېټ;
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
  p_ماسک_سېټ = &pWInfo->s_ماسک_سېټ;
  p_ماسک_سېټ->n = 0;
  p_ماسک_سېټ->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((حرف*)pWInfo)+nByteWInfo);
  أكد( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#که_تعریف_شوی SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#د_که_پای

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3_Where_وېشل(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  إذا( nTabList==0 ){
    إذا( p_ترتیب_By ) pWInfo->nOBSat = p_ترتیب_By->n_بیان;
    إذا( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    إذا( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_ارزښت)==0
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
      create_ماسک(p_ماسک_سېټ, pTabList->a[ii].iCursor);
      sqlite3_Where_Tab_Func_دلیلونه(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }طالما( (++ii)<pTabList->nSrc );
  #که_تعریف_شوی SQLITE_DEBUG
    {
      Bitmask mx = 0;
      لكل(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3_Where_Get_ماسک(p_ماسک_سېټ, pTabList->a[ii].iCursor);
        أكد( m>=mx );
        mx = m;
      }
    }
  #د_که_پای
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_بیان_Analyze(pTabList, &pWInfo->sWC);
  إذا( pSelect && pSelect->p_حد ){
    sqlite3_Where_جمع_کړه_حد(&pWInfo->sWC, pSelect);
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
  لكل(ii=0; ii<sWLB.pWC->n_بنسټ; ii++){
    Where_غړی *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    بیان *pX;                          /* The expression of pT */
    إذا( pT->wtFlags & غړی_VIRTUAL ) استمر;
    pX = pT->p_بیان;
    أكد( pX!=0 );
    أكد( pT->prereqAll!=0 || !بیان_Has_Property(pX, EP_OuterON) );
    إذا( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || بیان_Is_Deterministic(pX))   /* Condition (4) */
     && !(بیان_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_بیان_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= غړی_CODED;
    }
  }

  إذا( wctrlFlags & WHERE_WANT_DISTINCT ){
    إذا( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }وإلا إذا( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_پایله_سېټ) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }وإلا إذا( p_ترتیب_By==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->p_ترتیب_By = p_پایله_سېټ;
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
      sSelect.pEList = p_پایله_سېټ;
      sqlite3_ونه_View_Select(0, &sSelect, 0);
    }
    إذا( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#د_که_پای

  إذا( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = where_Loop_جمع_کړه_All(&sWLB);
    إذا( rc ) اذهب whereBeginError;

#که_تعریف_شوی SQLITE_ENABLE_STAT4
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
        pWInfo->pLoops = p->p_بل_Loop;
        whereLoopDelete(db, p);
      }
      rc = where_Loop_جمع_کړه_All(&sWLB);
      إذا( rc ) اذهب whereBeginError;
    }
#د_که_پای
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    where_لار_Solver(pWInfo, 0);
    إذا( db->mallocFailed ) اذهب whereBeginError;
    إذا( pWInfo->p_ترتیب_By ){
       whereInterstageHeuristic(pWInfo);
       where_لار_Solver(pWInfo, pWInfo->n_لیکه_بهر<0 ? 1 : pWInfo->n_لیکه_بهر+1);
       إذا( db->mallocFailed ) اذهب whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    إذا( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->n_لیکه_بهر, pWInfo->n_لیکه_بهر-30));
      pWInfo->n_لیکه_بهر -= 30;
    }

  }
  أكد( pWInfo->pTabList!=0 );
  إذا( pWInfo->p_ترتیب_By==0 && (db->flags & SQLITE_سرچپه_ترتیب)!=0 ){
    where_سرچپه_لټون_ترتیب(pWInfo);
  }
  إذا( pParse->nErr ){
    اذهب whereBeginError;
  }
  أكد( db->mallocFailed==0 );
#که_تعریف_شوی WHERETRACE_ENABLED
  إذا( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_ټول_لګښت, pWInfo->n_لیکه_بهر);
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
#د_که_پای

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
   && p_پایله_سېټ!=0           /* Condition (1) */
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
#د_که_پای
  pWInfo->pParse->nQueryLoop += pWInfo->n_لیکه_بهر;

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
    Src_توکی *p_Tab_توکی;

    p_Tab_توکی = &pTabList->a[pLevel->iFrom];
    pTab = p_Tab_توکی->pSTab;
    iDb = sqlite3_Schema_منزل_شاخص(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    إذا( ii==0 || (p_Tab_توکی[0].fg.jointype & JT_کیڼ)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }وإلا إذا( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }وإلا{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    إذا( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }وإلا
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
    إذا( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ثابت حرف *pVTab = (ثابت حرف *)sqlite3GetVTable(db, pTab);
      صحيح iCur = p_Tab_توکی->iCursor;
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }وإلا إذا( IsVirtual(pTab) ){
      /* noop */
    }وإلا
#د_که_پای
    إذا( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (p_Tab_توکی->fg.jointype & (JT_LTORJ|JT_ښی))!=0
    ){
      صحيح op = OP_OpenRead;
      إذا( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = p_Tab_توکی->iCursor;
      };
      sqlite3_Open_جدول(pParse, p_Tab_توکی->iCursor, iDb, pTab, op);
      أكد( p_Tab_توکی->iCursor==pLevel->iTabCur );
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
        Bitmask b = p_Tab_توکی->ستون_کارول_شوی;
        صحيح n = 0;
        لكل(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_منزل_PTR(n), P4_INT32);
        أكد( n<=pTab->n_ستون );
      }
#که_تعریف_شوی SQLITE_ENABLE_CURSOR_HINTS
      إذا( pLoop->u.btree.p_شاخص!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }وإلا
#د_که_پای
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#که_تعریف_شوی SQLITE_ENABLE_ستنه_کارول_شوی_ماسک
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Columns_کارول_شوی, p_Tab_توکی->iCursor,
                    LOWER32(p_Tab_توکی->ستون_کارول_شوی), UPPER32(p_Tab_توکی->ستون_کارول_شوی));
#د_که_پای
      إذا( ii>=2
       && (p_Tab_توکی[0].fg.jointype & (JT_LTORJ|JT_کیڼ))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_If_تشه, p_Tab_توکی->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }وإلا{
      sqlite3_جدول_قلف(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    إذا( pLoop->wsFlags & WHERE_شاخصي ){
      شاخص *pIx = pLoop->u.btree.p_شاخص;
      صحيح i_شاخص_Cur;
      صحيح op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      أكد( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      إذا( !HasRowid(pTab) && Is_Primary_کلی_شاخص(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_شاخص_Cur = pLevel->iTabCur;
        op = 0;
      }وإلا إذا( pWInfo->eOnePass!=ONEPASS_OFF ){
        شاخص *pJ = p_Tab_توکی->pSTab->p_شاخص;
        i_شاخص_Cur = iAuxArg;
        أكد( wctrlFlags & WHERE_ONEPASS_DESIRED );
        طالما( ALWAYS(pJ) && pJ!=pIx ){
          i_شاخص_Cur++;
          pJ = pJ->p_بل;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_شاخص_Cur;
      }وإلا إذا( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_شاخص_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }وإلا{
        i_شاخص_Cur = pParse->nTab++;
        إذا( pIx->b_Has_بیان && OptimizationEnabled(db, SQLITE_شاخصي_بیان) ){
          where_جمع_کړه_شاخصي_بیان(pParse, pIx, i_شاخص_Cur, p_Tab_توکی);
        }
        إذا( pIx->pPartIdxWhere && (p_Tab_توکی->fg.jointype & JT_ښی)==0 ){
          where_Part_Idx_بیان(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_شاخص_Cur, p_Tab_توکی
          );
        }
      }
      pLevel->iIdxCur = i_شاخص_Cur;
      أكد( pIx!=0 );
      أكد( pIx->pSchema==pTab->pSchema );
      أكد( i_شاخص_Cur>=0 );
      إذا( op ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, op, i_شاخص_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pIx);
        إذا( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_ستنه_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_ترتیبول)==0
         && (pLoop->wsFlags & WHERE_دننه_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#که_تعریف_شوی SQLITE_ENABLE_ستنه_کارول_شوی_ماسک
        {
          u64 ستون_کارول_شوی = 0;
          صحيح ii, jj;
          لكل(ii=0; ii<pIx->n_ستنه; ii++){
            jj = pIx->ai_ستنه[ii];
            إذا( jj<0 ) استمر;
            إذا( jj>63 ) jj = 63;
            إذا( (p_Tab_توکی->ستون_کارول_شوی & MASKBIT(jj))==0 ) استمر;
            ستون_کارول_شوی |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Columns_کارول_شوی, i_شاخص_Cur,
                            LOWER32(ستون_کارول_شوی),UPPER32(ستون_کارول_شوی));
        }
#د_که_پای /* SQLITE_ENABLE_COLUMN_USED_MASK */
#که_تعریف_شوی SQLITE_ENABLE_CURSOR_HINTS
        إذا( HasRowid(pTab) ){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_CursorHint, i_شاخص_Cur, 0, p_Tab_توکی->iCursor);
        }
#د_که_پای
      }
    }
    إذا( iDb>=0 ) sqlite3_کوډ_Verify_Schema(pParse, iDb);
    إذا( (p_Tab_توکی->fg.jointype & JT_ښی)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, حجم(Where_ښی_Join)))!=0
    ){
      Where_ښی_Join *pRJ = pLevel->pRJ;
      صحيح bBloomable = 0;
      pRJ->i_سمون = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Null, 0, pRJ->regReturn);
      أكد( pTab==p_Tab_توکی->pSTab );
      إذا( HasRowid(pTab) ){
        کلی_Info *pInfo;
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenEphemeral, pRJ->i_سمون, 1);
        bBloomable = 1;
        pInfo = sqlite3_کلی_Info_Alloc(pParse->db, 1, 0);
        إذا( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->a_ترتیبول_Flags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }وإلا{
        شاخص *pPk = sqlite3_Primary_کلی_شاخص(pTab);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenEphemeral, pRJ->i_سمون, pPk->n_کلی_ستون);
        sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pPk);
        bBloomable = sqlite3_شاخص_Bloomable(pPk,pPk->n_کلی_ستون);
      }
      إذا( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Blob, 65536, pRJ->regBloom);
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
  pWInfo->i_سر_2 = sqlite3VdbeCurrentAddr(v);
  إذا( db->mallocFailed ) اذهب whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  لكل(ii=0; ii<nTabList; ii++){
    صحيح addrExplain;
    صحيح wsFlags;
    Src_توکی *pSrc;
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
        iOnce = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Once); VdbeCoverage(v);
      }وإلا{
        iOnce = 0;
      }
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      إذا( iOnce )  sqlite3_Vdbe_Jump_دلته(v, iOnce);
    }
    أكد( pTabList == pWInfo->pTabList );
    إذا( (wsFlags & (WHERE_AUTO_شاخص|WHERE_BLOOMFILTER))!=0 ){
      إذا( (wsFlags & WHERE_AUTO_شاخص)!=0 ){
#که_نه_تعریف_شوی SQLITE_OMIT_AUTOMATIC_شاخص
        construct_Automatic_شاخص(pParse, &pWInfo->sWC, notReady, pLevel);
#د_که_پای
      }وإلا{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      إذا( db->mallocFailed ) اذهب whereBeginError;
    }
    addrExplain = sqlite3_Where_Explain_One_لټون(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3_Where_کوډ_One_Loop_پیل(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    إذا( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3_Where_جمع_کړه_لټون_حالت(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->i_پای_ټکی_Where = sqlite3VdbeCurrentAddr(v);
  أرجع pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  إذا( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#که_تعریف_شوی WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#د_که_پای /* WHERETRACE_ENABLED */
  أرجع 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#که_نه_تعریف_شوی SQLITE_DEBUG
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
    sqlite3_Show_Where_غړی(0); /* So compiler won't complain about unused func */
  }
#د_که_پای

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
فراغ sqlite3_Where_پای_ټکی(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  صحيح i_پای_ټکی = sqlite3VdbeCurrentAddr(v);
  صحيح nRJ = 0;
#که_نه_تعریف_شوی SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  صحيح addrSeek = 0;
#د_که_پای

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  لكل(i=pWInfo->nLevel-1; i>=0; i--){
    صحيح addr;
    pLevel = &pWInfo->a[i];
    إذا( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_ښی_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->پای_ټکی_Subrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    إذا( pLevel->op!=OP_Noop ){
#که_نه_تعریف_شوی SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      شاخص *pIdx;
      صحيح n;
      إذا( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_شاخصي)!=0
       && (pIdx = pLoop->u.btree.p_شاخص)->hasStat1
       && (n = pLoop->u.btree.n_Distinct_ستون)>0
       && pIdx->ai_لیکه_Log_Est[n]>=36
      ){
        صحيح r1 = pParse->nMem+1;
        صحيح j, op;
        صحيح addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        إذا( pLevel->i_کیڼ_Join ){
          addrIfNull = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_If_Null_لیکه, pLevel->iIdxCur, r1);
        }
        لكل(j=0; j<n; j++){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3_Vdbe_جمع_کړه_Op4_Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Goto, 1, pLevel->p2);
        إذا( pLevel->i_کیڼ_Join ){
          sqlite3_Vdbe_Jump_دلته(v, addrIfNull);
        }
      }
#د_که_پای /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    إذا( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }وإلا إذا( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      صحيح iBypass = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Goto);
      أكد( pLoop->wsFlags & WHERE_AUTO_شاخص );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3_Vdbe_Jump_دلته(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    إذا( pLevel->op!=OP_Noop ){
      sqlite3_Vdbe_جمع_کړه_Op3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_بل);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#که_نه_تعریف_شوی SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      إذا( addrSeek ){
        sqlite3_Vdbe_Jump_دلته(v, addrSeek);
        addrSeek = 0;
      }
#د_که_پای
      إذا( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
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
#که_نه_تعریف_شوی SQLITE_LIKE_DOESNT_سمون_BLOBS
    أكد( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_دننه_ABLE) && pLevel->u.دننه.n_دننه>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    إذا( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_DecrJumpZero, (صحيح)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#د_که_پای
    إذا( (pLoop->wsFlags & WHERE_دننه_ABLE)!=0 && pLevel->u.دننه.n_دننه>0 ){
      بنية دننه_Loop *p_دننه;
      صحيح j;
      لكل(j=pLevel->u.دننه.n_دننه, p_دننه=&pLevel->u.دننه.a_دننه_Loop[j-1]; j>0; j--, p_دننه--){
        أكد( sqlite3VdbeGetOp(v, p_دننه->addr_دننه_سر+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3_Vdbe_Jump_دلته(v, p_دننه->addr_دننه_سر+1);
        إذا( p_دننه->e_پای_ټکی_Loop_Op!=OP_Noop ){
          إذا( p_دننه->n_مختاړی ){
            صحيح b_Early_بهر =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_دننه_EARLYOUT)!=0;
            إذا( pLevel->i_کیڼ_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3_Vdbe_جمع_کړه_Op3(v, OP_If_Null_لیکه, p_دننه->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + b_Early_بهر, p_دننه->i_بنسټ);
              VdbeCoverage(v);
            }
            إذا( b_Early_بهر ){
              sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  p_دننه->i_بنسټ, p_دننه->n_مختاړی);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3_Vdbe_Jump_دلته(v, p_دننه->addr_دننه_سر+1);
            }
          }
          sqlite3_Vdbe_جمع_کړه_Op2(v, p_دننه->e_پای_ټکی_Loop_Op, p_دننه->iCur, p_دننه->addr_دننه_سر);
          VdbeCoverage(v);
          VdbeCoverageIf(v, p_دننه->e_پای_ټکی_Loop_Op==OP_Prev);
          VdbeCoverageIf(v, p_دننه->e_پای_ټکی_Loop_Op==OP_بل);
        }
        sqlite3_Vdbe_Jump_دلته(v, p_دننه->addr_دننه_سر-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    إذا( pLevel->pRJ ){
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    إذا( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_شاخص->zName));
      sqlite3_Vdbe_Jump_دلته(v, pLevel->addrSkip);
      sqlite3_Vdbe_Jump_دلته(v, pLevel->addrSkip-2);
    }
    إذا( pLevel->i_کیڼ_Join ){
      صحيح ws = pLoop->wsFlags;
      addr = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IfPos, pLevel->i_کیڼ_Join); VdbeCoverage(v);
      أكد( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_شاخصي)!=0 );
      إذا( (ws & WHERE_IDX_ONLY)==0 ){
        Src_توکی *pSrc = &pTabList->a[pLevel->iFrom];
        أكد( pLevel->iTabCur==pSrc->iCursor );
        إذا( pSrc->fg.له_لارې_Coroutine ){
          صحيح m, n;
          أكد( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_پایله;
          أكد( pSrc->pSTab!=0 );
          m = pSrc->pSTab->n_ستون;
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Null_لیکه, pLevel->iTabCur);
      }
      إذا( (ws & WHERE_شاخصي)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        إذا( ws & WHERE_MULTI_OR ){
          شاخص *pIx = pLevel->u.pCoveringIdx;
          صحيح iDb = sqlite3_Schema_منزل_شاخص(db, pIx->pSchema);
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pIx);
        }
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Null_لیکه, pLevel->iIdxCur);
      }
      إذا( ws & WHERE_دننه_ABLE ){
        صحيح m;
        لكل(m=0; m<pLevel->u.دننه.n_دننه; m++){
          بنية دننه_Loop *p_دننه = &pLevel->u.دننه.a_دننه_Loop[m];
          إذا( p_دننه->e_پای_ټکی_Loop_Op!=OP_Noop ){
            sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Null_لیکه, p_دننه->iCur);
          }
        }
      }
      إذا( pLevel->op==OP_Return ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, pLevel->p1, pLevel->addr_لومړی);
      }وإلا{
        sqlite3VdbeGoto(v, pLevel->addr_لومړی);
      }
      sqlite3_Vdbe_Jump_دلته(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  أكد( pWInfo->nLevel<=pTabList->nSrc );
  لكل(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    صحيح k, وروستی_2;
    VdbeOp *pOp, *p_وروستی_Op;
    شاخص *pIdx = 0;
    Src_توکی *p_Tab_توکی = &pTabList->a[pLevel->iFrom];
    جدول *pTab = p_Tab_توکی->pSTab;
    أكد( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    إذا( pLevel->pRJ ){
      sqlite3_Where_ښی_Join_Loop(pWInfo, i, pLevel);
      استمر;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    إذا( p_Tab_توکی->fg.له_لارې_Coroutine ){
      testcase( pParse->db->mallocFailed );
      أكد( p_Tab_توکی->fg.isSubquery );
      أكد( p_Tab_توکی->u4.pSubq->reg_پایله>=0 );
      translate_ستنه_منزل_Copy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            p_Tab_توکی->u4.pSubq->reg_پایله, 0);
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
    إذا( pLoop->wsFlags & (WHERE_شاخصي|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_شاخص;
    }وإلا إذا( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    إذا( pIdx
     && !db->mallocFailed
    ){
      إذا( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->p_جدول) ){
        وروستی_2 = i_پای_ټکی;
      }وإلا{
        وروستی_2 = pWInfo->i_پای_ټکی_Where;
      }
      إذا( pIdx->b_Has_بیان ){
        شاخصي_بیان *p = pParse->pIdxEpr;
        طالما( p ){
          إذا( p->iIdxCur==pLevel->iIdxCur ){
#که_تعریف_شوی WHERETRACE_ENABLED
            إذا( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->i_Idx_ستون);
              إذا( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_بیان(p->p_بیان);
            }
#د_که_پای
            p->i_معلومات_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#که_تعریف_شوی SQLITE_DEBUG
      إذا( db->flags & SQLITE_VdbeAddopTrace ){
        چاپ_کړه("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, وروستی_2-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      أكد( pOp->opcode!=OP_ستنه || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_If_Null_لیکه || pOp->p1!=pLevel->iTabCur );
#د_که_پای
      pOp = sqlite3VdbeGetOp(v, k);
      p_وروستی_Op = pOp + (وروستی_2 - k);
      أكد( pOp<=p_وروستی_Op );
      افعل{
        إذا( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }وإلا إذا( pOp->opcode==OP_ستنه
#که_تعریف_شوی SQLITE_ENABLE_دننه_واټن_SQL_FUNC
         || pOp->opcode==OP_دننه_واټن
#د_که_پای
        ){
          صحيح x = pOp->p2;
          أكد( pIdx->p_جدول==pTab );
#که_تعریف_شوی SQLITE_ENABLE_دننه_واټن_SQL_FUNC
          إذا( pOp->opcode==OP_دننه_واټن ){
            /* Do not need to translate the column number */
          }وإلا
#د_که_پای
          إذا( !HasRowid(pTab) ){
            شاخص *pPk = sqlite3_Primary_کلی_شاخص(pTab);
            x = pPk->ai_ستنه[x];
            أكد( x>=0 );
          }وإلا{
            testcase( x!=sqlite3_Storage_ستنه_منزل_جدول(pTab,x) );
            x = sqlite3_Storage_ستنه_منزل_جدول(pTab,x);
          }
          x = sqlite3_جدول_ستنه_منزل_شاخص(pIdx, x);
          إذا( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }وإلا إذا( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            إذا( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_پیغام(pParse, "internal query planner error");
              pParse->rc = SQLITE_دننی;
            }وإلا{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3_Where_جمع_کړه_Explain_متن(pParse,
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
        }وإلا إذا( pOp->opcode==OP_If_Null_لیکه ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#که_تعریف_شوی SQLITE_DEBUG
        k++;
#د_که_پای
      }طالما( (++pOp)<p_وروستی_Op );
#که_تعریف_شوی SQLITE_DEBUG
      إذا( db->flags & SQLITE_VdbeAddopTrace ) چاپ_کړه("TRANSLATE complete\n");
#د_که_پای
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
