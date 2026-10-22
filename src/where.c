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
قسمتعریف ساخت Hidden_اشاریہ_Info Hidden_اشاریہ_Info;
ساخت Hidden_اشاریہ_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  صحیح eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 m_اندر;                 /* Mask of terms that are <col> IN (...) */
  u32 m_Handle_اندر;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_قدر *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#تعریف_کرو SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_اشاریہ_Info,aRhs) + (N)*حجم(sqlite3_قدر*))

/* Forward declaration of methods */
ساکن صحیح whereLoopResize(sqlite3*, WhereLoop*, صحیح);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_صف_گنتی(WhereInfo *pWInfo){
  واپس pWInfo->n_صف_برآمد;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
صحیح sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  واپس pWInfo->eDistinct;
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
  واپس pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
صحیح sqlite3_Where_ترتیب_By_حد_بندی_Opt_Label(WhereInfo *pWInfo){
  WhereLevel *pInner;
  اگر( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    واپس pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  أكد( pInner->addrNxt!=0 );
  واپس pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
خالی sqlite3_Where_Min_Max_Opt_Early_برآمد(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  صحیح i;
  اگر( !pWInfo->bOrderedInnerLoop ) واپس;
  اگر( pWInfo->nOBSat==0 ) واپس;
  برائے(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    اگر( (pInner->pWLoop->wsFlags & WHERE_عمود_اندر)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      واپس;
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
  واپس pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
صحیح sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  واپس pWInfo->iBreak;
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
  حافظہ_نقل(aiCur, pWInfo->aiCurOnePass, حجم(صحیح)*2);
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  اگر( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#اگر_ختم
  واپس pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
صحیح sqlite3_Where_استعمال_کرتا_Deferred_Seek(WhereInfo *pWInfo){
  واپس pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
ساکن خالی whereOrMove(Where_Or_سیٹ *pDest, Where_Or_سیٹ *pSrc){
  pDest->n = pSrc->n;
  حافظہ_نقل(pDest->a, pSrc->a, pDest->n*حجم(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
ساکن صحیح where_Or_داخل_کرو(
  Where_Or_سیٹ *p_سیٹ,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst r_تسلسل,           /* Run-cost of the new entry */
  LogEst n_برآمد            /* Number of outputs for the new entry */
){
  u16 i;
  Where_Or_لاگت *p;
  برائے(i=p_سیٹ->n, p=p_سیٹ->a; i>0; i--, p++){
    اگر( r_تسلسل<=p->r_تسلسل && (prereq & p->prereq)==prereq ){
      جاؤ where_Or_داخل_کرو_مکمل;
    }
    اگر( p->r_تسلسل<=r_تسلسل && (p->prereq & prereq)==p->prereq ){
      واپس 0;
    }
  }
  اگر( p_سیٹ->n<N_OR_لاگت ){
    p = &p_سیٹ->a[p_سیٹ->n++];
    p->n_برآمد = n_برآمد;
  }ورنہ{
    p = p_سیٹ->a;
    برائے(i=1; i<p_سیٹ->n; i++){
      اگر( p->r_تسلسل>p_سیٹ->a[i].r_تسلسل ) p = p_سیٹ->a + i;
    }
    اگر( p->r_تسلسل<=r_تسلسل ) واپس 0;
  }
where_Or_داخل_کرو_مکمل:
  p->prereq = prereq;
  p->r_تسلسل = r_تسلسل;
  اگر( p->n_برآمد>n_برآمد ) p->n_برآمد = n_برآمد;
  واپس 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3_Where_Get_نقاب(Where_نقاب_سیٹ *p_نقاب_سیٹ, صحیح iCursor){
  صحیح i;
  أكد( p_نقاب_سیٹ->n<=(صحیح)حجم(Bitmask)*8 );
  أكد( p_نقاب_سیٹ->n>0 || p_نقاب_سیٹ->ix[0]<0 );
  أكد( iCursor>=-1 );
  اگر( p_نقاب_سیٹ->ix[0]==iCursor ){
    واپس 1;
  }
  برائے(i=1; i<p_نقاب_سیٹ->n; i++){
    اگر( p_نقاب_سیٹ->ix[i]==iCursor ){
      واپس MASKBIT(i);
    }
  }
  واپس 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
خالی *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  Where_Mem_بلاک *p_بلاک;
  p_بلاک = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+حجم(*p_بلاک));
  اگر( p_بلاک ){
    p_بلاک->p_اگلا = pWInfo->p_Mem_مقصد_Free;
    p_بلاک->sz = nByte;
    pWInfo->p_Mem_مقصد_Free = p_بلاک;
    p_بلاک++;
  }
  واپس (خالی*)p_بلاک;
}
خالی *sqlite3WhereRealloc(WhereInfo *pWInfo, خالی *pOld, u64 nByte){
  خالی *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  اگر( pNew && pOld ){
    Where_Mem_بلاک *pOldBlk = (Where_Mem_بلاک*)pOld;
    pOldBlk--;
    أكد( pOldBlk->sz<nByte );
    حافظہ_نقل(pNew, pOld, pOldBlk->sz);
  }
  واپس pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
ساکن خالی create_نقاب(Where_نقاب_سیٹ *p_نقاب_سیٹ, صحیح iCursor){
  أكد( p_نقاب_سیٹ->n < Array_سائز(p_نقاب_سیٹ->ix) );
  p_نقاب_سیٹ->ix[p_نقاب_سیٹ->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
ساکن اظہاریہ *where_دایاں_Subexpr_Is_عمود(اظہاریہ *p){
  p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p->p_دایاں);
  اگر( ALWAYS(p!=0) && p->op==TK_عمود && !اظہاریہ_Has_Property(p, EP_Fixed_ستون) ){
    واپس p;
  }
  واپس 0;
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
ساکن SQLITE_NOINLINE ثابت حرف *اشاریہ_اندر_Affinity_Ok(
  Parse *pParse, 
  Where_رکن *p_رکن, 
  u8 idxaff
){
  اظہاریہ *pX = p_رکن->p_اظہاریہ;
  اظہاریہ inexpr;

  أكد( p_رکن->eOperator & WO_اندر );

  اگر( اظہاریہ_Use_XSelect(pX) ){
    صحیح i_میدان = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    اگر( sqlite3_اظہاریہ_Is_سمتیہ(pX->p_بایاں) ){
      i_میدان = p_رکن->u.x.i_میدان - 1;
      inexpr.p_بایاں = pX->p_بایاں->x.pList->a[i_میدان].p_اظہاریہ;
    }ورنہ{
      inexpr.p_بایاں = pX->p_بایاں;
    }
    inexpr.p_دایاں = pX->x.pSelect->pEList->a[i_میدان].p_اظہاریہ;
    pX = &inexpr;
  }

  اگر( sqlite3_اشاریہ_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pX);
    واپس pRet ? pRet->zName : sqlite3_Str_ثنائی;
  }
  واپس 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
ساکن Where_رکن *where_جائزہ_اگلا(Where_جائزہ *p_جائزہ){
  صحیح iCur;            /* The cursor on the LHS of the term */
  i16 i_عمود;         /* The column on the LHS of the term.  -1 for IPK */
  اظہاریہ *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  Where_رکن *p_رکن;    /* The term being tested */
  صحیح k = p_جائزہ->k;    /* Where to start scanning */

  أكد( p_جائزہ->iEquiv<=p_جائزہ->nEquiv );
  pWC = p_جائزہ->pWC;
  جب(1){
    i_عمود = p_جائزہ->ai_عمود[p_جائزہ->iEquiv-1];
    iCur = p_جائزہ->aiCur[p_جائزہ->iEquiv-1];
    أكد( pWC!=0 );
    أكد( iCur>=0 );
    کرو{
      برائے(p_رکن=pWC->a+k; k<pWC->n_رکن; k++, p_رکن++){
        أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 || p_رکن->بایاں_Cursor<0 );
        اگر( p_رکن->بایاں_Cursor==iCur
         && p_رکن->u.x.بایاں_عمود==i_عمود
         && (i_عمود!=XN_اظہاریہ
             || sqlite3_اظہاریہ_Compare_Skip(p_رکن->p_اظہاریہ->p_بایاں,
                                       p_جائزہ->p_Idx_اظہاریہ,iCur)==0)
         && (p_جائزہ->iEquiv<=1 || !اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_OuterON))
        ){
          اگر( (p_رکن->eOperator & WO_EQUIV)!=0
           && p_جائزہ->nEquiv<Array_سائز(p_جائزہ->aiCur)
           && (pX = where_دایاں_Subexpr_Is_عمود(p_رکن->p_اظہاریہ))!=0
          ){
            صحیح j;
            برائے(j=0; j<p_جائزہ->nEquiv; j++){
              اگر( p_جائزہ->aiCur[j]==pX->i_جدول
               && p_جائزہ->ai_عمود[j]==pX->i_عمود ){
                  توڑو;
              }
            }
            اگر( j==p_جائزہ->nEquiv ){
              p_جائزہ->aiCur[j] = pX->i_جدول;
              p_جائزہ->ai_عمود[j] = pX->i_عمود;
              p_جائزہ->nEquiv++;
            }
          }
          اگر( (p_رکن->eOperator & p_جائزہ->op_نقاب)!=0 ){
            /* Verify the affinity and collating sequence match */
            اگر( p_جائزہ->zCollName && (p_رکن->eOperator & WO_ISNULL)==0 ){
              ثابت حرف *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = p_رکن->p_اظہاریہ;

              اگر( (p_رکن->eOperator & WO_اندر) ){
                zCollName = اشاریہ_اندر_Affinity_Ok(pParse, p_رکن, p_جائزہ->idxaff);
                اگر( !zCollName ) جاری;
              }ورنہ{
                CollSeq *pColl;
                اگر( !sqlite3_اشاریہ_Affinity_Ok(pX, p_جائزہ->idxaff) ){
                  جاری;
                }
                أكد(pX->p_بایاں);
                pColl = sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_ثنائی;
              }

              اگر( sqlite3StrICmp(zCollName, p_جائزہ->zCollName) ){
                جاری;
              }
            }
            اگر( (p_رکن->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = p_رکن->p_اظہاریہ->p_دایاں, ALWAYS(pX!=0))
             && pX->op==TK_عمود
             && pX->i_جدول==p_جائزہ->aiCur[0]
             && pX->i_عمود==p_جائزہ->ai_عمود[0]
            ){
              testcase( p_رکن->eOperator & WO_IS );
              جاری;
            }
            p_جائزہ->pWC = pWC;
            p_جائزہ->k = k+1;
#اگر_تعریف_شدہ WHERETRACE_ENABLED
            اگر( (sqlite3WhereTrace & 0x20000)!=0 && p_جائزہ->nEquiv>1 ){
              صحیح ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 p_جائزہ->aiCur[0], p_جائزہ->ai_عمود[0], p_رکن->i_رکن);
              برائے(ii=1; ii<p_جائزہ->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   p_جائزہ->aiCur[ii], p_جائزہ->ai_عمود[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#اگر_ختم
            واپس p_رکن;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }جب( pWC!=0 );
    اگر( p_جائزہ->iEquiv>=p_جائزہ->nEquiv ) توڑو;
    pWC = p_جائزہ->pOrigWC;
    k = 0;
    p_جائزہ->iEquiv++;
  }
  واپس 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
ساکن SQLITE_NOINLINE Where_رکن *where_جائزہ_Init_اشاریہ_اظہاریہ(Where_جائزہ *p_جائزہ){
  p_جائزہ->idxaff = sqlite3_اظہاریہ_Affinity(p_جائزہ->p_Idx_اظہاریہ);
  واپس where_جائزہ_اگلا(p_جائزہ);
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
ساکن Where_رکن *where_جائزہ_Init(
  Where_جائزہ *p_جائزہ,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  صحیح iCur,               /* Cursor to scan for */
  صحیح i_عمود,            /* Column to scan for */
  u32 op_نقاب,             /* Operator(s) to scan for */
  اشاریہ *pIdx             /* Must be compatible with this index */
){
  p_جائزہ->pOrigWC = pWC;
  p_جائزہ->pWC = pWC;
  p_جائزہ->p_Idx_اظہاریہ = 0;
  p_جائزہ->idxaff = 0;
  p_جائزہ->zCollName = 0;
  p_جائزہ->op_نقاب = op_نقاب;
  p_جائزہ->k = 0;
  p_جائزہ->aiCur[0] = iCur;
  p_جائزہ->nEquiv = 1;
  p_جائزہ->iEquiv = 1;
  اگر( pIdx ){
    صحیح j = i_عمود;
    i_عمود = pIdx->ai_عمود[j];
    اگر( i_عمود==pIdx->p_جدول->iPKey ){
      i_عمود = XN_ROWID;
    }ورنہ اگر( i_عمود>=0 ){
      p_جائزہ->idxaff = pIdx->p_جدول->a_ستون[i_عمود].affinity;
      p_جائزہ->zCollName = pIdx->azColl[j];
    }ورنہ اگر( i_عمود==XN_اظہاریہ ){
      p_جائزہ->p_Idx_اظہاریہ = pIdx->a_ستون_اظہاریہ->a[j].p_اظہاریہ;
      p_جائزہ->zCollName = pIdx->azColl[j];
      p_جائزہ->ai_عمود[0] = XN_اظہاریہ;
      واپس where_جائزہ_Init_اشاریہ_اظہاریہ(p_جائزہ);
    }
  }ورنہ اگر( i_عمود==XN_اظہاریہ ){
    واپس 0;
  }
  p_جائزہ->ai_عمود[0] = i_عمود;
  واپس where_جائزہ_اگلا(p_جائزہ);
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
Where_رکن *sqlite3_Where_ڈھونڈو_رکن(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  صحیح iCur,             /* Cursor number of LHS */
  صحیح i_عمود,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  اشاریہ *pIdx           /* Must be compatible with this index, if not NULL */
){
  Where_رکن *p_نتیجہ = 0;
  Where_رکن *p;
  Where_جائزہ جائزہ;

  p = where_جائزہ_Init(&جائزہ, pWC, iCur, i_عمود, op, pIdx);
  op &= WO_EQ|WO_IS;
  جب( p ){
    اگر( (p->prereq_دایاں & notReady)==0 ){
      اگر( p->prereq_دایاں==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        واپس p;
      }
      اگر( p_نتیجہ==0 ) p_نتیجہ = p;
    }
    p = where_جائزہ_اگلا(&جائزہ);
  }
  واپس p_نتیجہ;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
ساکن صحیح ڈھونڈو_اشاریہ_ستون(
  Parse *pParse,                  /* Parse context */
  اظہاریہ_List *pList,                /* Expression list to search */
  صحیح i_بنیاد,                      /* Cursor for table associated with pIdx */
  اشاریہ *pIdx,                    /* Index to match column of */
  صحیح i_ستون                        /* Column of index to match */
){
  صحیح i;
  ثابت حرف *zColl = pIdx->azColl[i_ستون];

  برائے(i=0; i<pList->n_اظہاریہ; i++){
    اظہاریہ *p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pList->a[i].p_اظہاریہ);
    اگر( ALWAYS(p!=0)
     && (p->op==TK_عمود || p->op==TK_AGG_عمود)
     && p->i_عمود==pIdx->ai_عمود[i_ستون]
     && p->i_جدول==i_بنیاد
    ){
      CollSeq *pColl = sqlite3_اظہاریہ_NNColl_Seq(pParse, pList->a[i].p_اظہاریہ);
      اگر( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        واپس i;
      }
    }
  }

  واپس -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
ساکن صحیح اشاریہ_عمود_Not_Null(اشاریہ *pIdx, صحیح i_ستون){
  صحیح j;
  أكد( pIdx!=0 );
  أكد( i_ستون>=0 && i_ستون<pIdx->n_عمود );
  j = pIdx->ai_عمود[i_ستون];
  اگر( j>=0 ){
    واپس pIdx->p_جدول->a_ستون[j].notNull;
  }ورنہ اگر( j==(-1) ){
    واپس 1;
  }ورنہ{
    أكد( j==(-2) );
    واپس 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
ساکن صحیح isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  اظہاریہ_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  جدول *pTab;
  اشاریہ *pIdx;
  صحیح i;                         
  صحیح i_بنیاد;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  اگر( pTabList->nSrc!=1 ) واپس 0;
  i_بنیاد = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  برائے(i=0; i<pDistinct->n_اظہاریہ; i++){
    اظہاریہ *p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pDistinct->a[i].p_اظہاریہ);
    اگر( NEVER(p==0) ) جاری;
    اگر( p->op!=TK_عمود && p->op!=TK_AGG_عمود ) جاری;
    اگر( p->i_جدول==i_بنیاد && p->i_عمود<0 ) واپس 1;
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
  برائے(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
    اگر( !Is_Unique_اشاریہ(pIdx) ) جاری;
    اگر( pIdx->pPartIdxWhere ) جاری;
    برائے(i=0; i<pIdx->n_کلید_ستون; i++){
      اگر( 0==sqlite3_Where_ڈھونڈو_رکن(pWC, i_بنیاد, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        اگر( ڈھونڈو_اشاریہ_ستون(pParse, pDistinct, i_بنیاد, pIdx, i)<0 ) توڑو;
        اگر( اشاریہ_عمود_Not_Null(pIdx, i)==0 ) توڑو;
      }
    }
    اگر( i==pIdx->n_کلید_ستون ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      واپس 1;
    }
  }

  واپس 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
ساکن LogEst estLog(LogEst N){
  واپس N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
ساکن خالی translate_عمود_مقصد_Copy(
  Parse *pParse,      /* Parsing context */
  صحیح i_آغاز,         /* Translate from this opcode to the end */
  صحیح iTabCur,        /* OP_Column/OP_Rowid references to this table */
  صحیح i_رجسٹر,      /* The first column is in this register */
  صحیح iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, i_آغاز);
  صحیح i_انتہا = sqlite3VdbeCurrentAddr(v);
  اگر( pParse->db->mallocFailed ) واپس;
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    لکھو("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, i_آغاز, i_انتہا);
  }
#اگر_ختم
  برائے(; i_آغاز<i_انتہا; i_آغاز++, pOp++){
    اگر( pOp->p1!=iTabCur ) جاری;
    اگر( pOp->opcode==OP_عمود ){
#اگر_تعریف_شدہ SQLITE_DEBUG
      اگر( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        لکھو("TRANSLATE OP_Column to OP_Copy at %d\n", i_آغاز);
      }
#اگر_ختم
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + i_رجسٹر;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }ورنہ اگر( pOp->opcode==OP_Rowid ){
#اگر_تعریف_شدہ SQLITE_DEBUG
      اگر( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        لکھو("TRANSLATE OP_Rowid to OP_Sequence at %d\n", i_آغاز);
      }
#اگر_ختم
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
      اگر( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#اگر_ختم
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
ساکن خالی where_Trace_اشاریہ_Info_Inputs(
  sqlite3_اشاریہ_info *p,   /* The IndexInfo object */
  جدول *pTab              /* The TABLE that is the virtual table */
){
  صحیح i;
  اگر( (sqlite3WhereTrace & 0x10)==0 ) واپس;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  برائے(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].i_عمود,
       p->aConstraint[i].i_رکن_انحراف,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  برائے(i=0; i<p->n_ترتیب_By; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->a_ترتیب_By[i].i_عمود,
       p->a_ترتیب_By[i].desc);
  }
}
ساکن خالی where_Trace_اشاریہ_Info_Outputs(
  sqlite3_اشاریہ_info *p,   /* The IndexInfo object */
  جدول *pTab              /* The TABLE that is the virtual table */
){
  صحیح i;
  اگر( (sqlite3WhereTrace & 0x10)==0 ) واپس;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  برائے(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_اشاریہ,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_عدد);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->ترتیب_By_Consumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimated_لاگت);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimated_صفیں);
}
#ورنہ
#تعریف_کرو where_Trace_اشاریہ_Info_Inputs(A,B)
#تعریف_کرو where_Trace_اشاریہ_Info_Outputs(A,B)
#اگر_ختم

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
ساکن صحیح constraintCompatibleWithOuterJoin(
  ثابت Where_رکن *p_رکن,       /* WHERE clause term to check */
  ثابت Src_شے *pSrc           /* Table we are trying to access */
){
  أكد( (pSrc->fg.jointype&(JT_بایاں|JT_LTORJ|JT_دایاں))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_بایاں|JT_LTORJ|JT_دایاں))==JT_بایاں );
  testcase( (pSrc->fg.jointype & (JT_بایاں|JT_LTORJ|JT_دایاں))==JT_LTORJ );
  testcase( اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_OuterON) )
  testcase( اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_InnerON) );
  اگر( !اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_OuterON|EP_InnerON)
   || p_رکن->p_اظہاریہ->w.iJoin != pSrc->iCursor
  ){
    واپس 0;
  }
  اگر( (pSrc->fg.jointype & (JT_بایاں|JT_دایاں))!=0
   && NEVER(اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_InnerON))
  ){
    واپس 0;
  }
  واپس 1;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOMATIC_اشاریہ
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
ساکن SQLITE_NOINLINE صحیح عمود_Is_Good_اشاریہ_Candidate(
  ثابت جدول *pTab,
  صحیح i_ستون
){
  ثابت اشاریہ *pIdx;
  برائے(pIdx = pTab->p_اشاریہ; pIdx!=0; pIdx=pIdx->p_اگلا){
    صحیح j;
    برائے(j=0; j<pIdx->n_کلید_ستون; j++){
       اگر( pIdx->ai_عمود[j]==i_ستون ){
         اگر( j==0 ) واپس 0;
         اگر( pIdx->hasStat1 && pIdx->ai_صف_Log_Est[j+1]>20 ) واپس 0;
         توڑو;
       }
    }
  }
  واپس 1;
}
#اگر_ختم /* SQLITE_OMIT_AUTOMATIC_INDEX */



#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOMATIC_اشاریہ
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
ساکن صحیح رکن_Can_Drive_اشاریہ(
  ثابت Where_رکن *p_رکن,        /* WHERE clause term to check */
  ثابت Src_شے *pSrc,           /* Table we are trying to access */
  ثابت Bitmask notReady         /* Tables in outer loops of the join */
){
  حرف aff;
  صحیح بایاں_ستون;
  
  اگر( p_رکن->بایاں_Cursor!=pSrc->iCursor ) واپس 0;
  اگر( (p_رکن->eOperator & (WO_EQ|WO_IS))==0 ) واپس 0;
  أكد( (pSrc->fg.jointype & JT_دایاں)==0 );
  اگر( (pSrc->fg.jointype & (JT_بایاں|JT_LTORJ|JT_دایاں))!=0
   && !constraintCompatibleWithOuterJoin(p_رکن,pSrc)
  ){
    واپس 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  اگر( (p_رکن->prereq_دایاں & notReady)!=0 ) واپس 0;
  أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
  بایاں_ستون = p_رکن->u.x.بایاں_عمود;
  اگر( بایاں_ستون<0 ) واپس 0;
  aff = pSrc->pSTab->a_ستون[بایاں_ستون].affinity;
  اگر( !sqlite3_اشاریہ_Affinity_Ok(p_رکن->p_اظہاریہ, aff) ) واپس 0;
  testcase( p_رکن->p_اظہاریہ->op==TK_IS );
  واپس عمود_Is_Good_اشاریہ_Candidate(pSrc->pSTab, بایاں_ستون);
}
#اگر_ختم


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOMATIC_اشاریہ

#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
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
ساکن خالی explain_Automatic_اشاریہ(
  Parse *pParse,
  اشاریہ *pIdx,                    /* Automatic index to explain */
  صحیح bPartial,                   /* True if pIdx is a partial index */
  صحیح *pAddrExplain               /* OUT: Address of OP_Explain */
){
  اگر( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    جدول *pTab = pIdx->p_جدول;
    ثابت حرف *zSep = "";
    حرف *z_متن = 0;
    صحیح ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    أكد( pIdx->n_عمود>1 );
    أكد( pIdx->ai_عمود[pIdx->n_عمود-1]==XN_ROWID || !HasRowid(pTab) );
    برائے(ii=0; ii<(pIdx->n_عمود-1); ii++){
      ثابت حرف *zName = 0;
      صحیح i_ستون = pIdx->ai_عمود[ii];

      zName = pTab->a_ستون[i_ستون].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    z_متن = sqlite3_str_اختتام(pStr);
    اگر( z_متن==0 ){
      sqlite3OomFault(pParse->db);
    }ورنہ{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", z_متن, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(z_متن);
    }
  }
}
#ورنہ
# define explain_Automatic_اشاریہ(a,b,c,d)
#اگر_ختم

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
ساکن SQLITE_NOINLINE خالی construct_Automatic_اشاریہ(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  ثابت Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  صحیح n_کلید_ستون;                /* Number of columns in the constructed index */
  Where_رکن *p_رکن;           /* A single term of the WHERE clause */
  Where_رکن *pWCEnd;          /* End of pWC->a[] */
  اشاریہ *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  صحیح addrInit;               /* Address of the initialization bypass jump */
  جدول *p_جدول;              /* The table being indexed */
  صحیح addr_چوٹی;                /* Top of the index fill loop */
  صحیح reg_ریکارڈ;              /* Register holding an index record */
  صحیح n;                      /* Column counter */
  صحیح i;                      /* Loop counter */
  صحیح mx_ثنائی_ہندسہ_ستون;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  حرف *z_Not_مستعمل;             /* Extra space on the end of pIdx */
  Bitmask idx_ستون_تعداد;            /* Bitmap of columns used for indexing */
  Bitmask extra_ستون_تعداد;          /* Bitmap of additional columns */
  u8 بھیجا_Warning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  اظہاریہ *pPartial = 0;         /* Partial Index Expression */
  صحیح iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  Src_شے *pSrc;              /* The FROM clause term to get the next index */
  صحیح addr_شمار = 0;        /* Address where integer counter is initialized */
  صحیح reg_بنیاد;                /* Array of registers where record is assembled */
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addrExp = 0;            /* Address of OP_Explain */
#اگر_ختم

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  أكد( v!=0 );
  addrInit = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_کلید_ستون = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  p_جدول = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->n_رکن];
  pLoop = pLevel->pWLoop;
  idx_ستون_تعداد = 0;
  برائے(p_رکن=pWC->a; p_رکن<pWCEnd; p_رکن++){
    اظہاریہ *p_اظہاریہ = p_رکن->p_اظہاریہ;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    اگر( (p_رکن->wtFlags & رکن_VIRTUAL)==0
     && sqlite3_اظہاریہ_Is_Single_جدول_Constraint(p_اظہاریہ, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_اظہاریہ_And(pParse, pPartial,
                                sqlite3_اظہاریہ_Dup(pParse->db, p_اظہاریہ, 0));
    }
    اگر( رکن_Can_Drive_اشاریہ(p_رکن, pSrc, notReady) ){
      صحیح i_ستون;
      Bitmask c_نقاب;
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      i_ستون = p_رکن->u.x.بایاں_عمود;
      c_نقاب = i_ستون>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ستون);
      testcase( i_ستون==BMS );
      testcase( i_ستون==BMS-1 );
      اگر( !بھیجا_Warning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", p_جدول->zName,
            p_جدول->a_ستون[i_ستون].zCnName);
        بھیجا_Warning = 1;
      }
      اگر( (idx_ستون_تعداد & c_نقاب)==0 ){
        اگر( whereLoopResize(pParse->db, pLoop, n_کلید_ستون+1) ){
          جاؤ انتہا_auto_اشاریہ_create;
        }
        pLoop->aLTerm[n_کلید_ستون++] = p_رکن;
        idx_ستون_تعداد |= c_نقاب;
      }
    }
  }
  أكد( n_کلید_ستون>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_کلید_ستون;
  pLoop->wsFlags = WHERE_عمود_EQ | WHERE_IDX_ONLY | WHERE_اشاریہ_دار
                     | WHERE_AUTO_اشاریہ;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  اگر( IsView(p_جدول) ){
    extra_ستون_تعداد = ALLBITS & ~idx_ستون_تعداد;
  }ورنہ{
    extra_ستون_تعداد = pSrc->ستون_مستعمل & (~idx_ستون_تعداد | MASKBIT(BMS-1));
  }
  اگر( !HasRowid(p_جدول) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    برائے(i=0; i<p_جدول->n_ستون; i++){
      اگر( (p_جدول->a_ستون[i].ستون_Flags & COLFLAG_PRIMKEY)==0 ) جاری;
      اگر( i>=BMS-1 ){
        extra_ستون_تعداد |= MASKBIT(BMS-1);
        توڑو;
      }
      اگر( idx_ستون_تعداد & MASKBIT(i) ) جاری;
      extra_ستون_تعداد |= MASKBIT(i);
    }
  }
  mx_ثنائی_ہندسہ_ستون = MIN(BMS-1,p_جدول->n_ستون);
  testcase( p_جدول->n_ستون==BMS-1 );
  testcase( p_جدول->n_ستون==BMS-2 );
  برائے(i=0; i<mx_ثنائی_ہندسہ_ستون; i++){
    اگر( extra_ستون_تعداد & MASKBIT(i) ) n_کلید_ستون++;
  }
  اگر( pSrc->ستون_مستعمل & MASKBIT(BMS-1) ){
    n_کلید_ستون += p_جدول->n_ستون - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  أكد( n_کلید_ستون <= p_جدول->n_ستون + MAX(0, p_جدول->n_ستون - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_اشاریہ_Object(pParse->db, n_کلید_ستون+HasRowid(p_جدول),
                                    0, &z_Not_مستعمل);
  اگر( pIdx==0 ) جاؤ انتہا_auto_اشاریہ_create;
  pLoop->u.btree.p_اشاریہ = pIdx;
  pIdx->zName = "auto-index";
  pIdx->p_جدول = p_جدول;
  n = 0;
  idx_ستون_تعداد = 0;
  برائے(p_رکن=pWC->a; p_رکن<pWCEnd; p_رکن++){
    اگر( رکن_Can_Drive_اشاریہ(p_رکن, pSrc, notReady) ){
      صحیح i_ستون;
      Bitmask c_نقاب;
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      i_ستون = p_رکن->u.x.بایاں_عمود;
      c_نقاب = i_ستون>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ستون);
      testcase( i_ستون==BMS-1 );
      testcase( i_ستون==BMS );
      اگر( (idx_ستون_تعداد & c_نقاب)==0 ){
        اظہاریہ *pX = p_رکن->p_اظہاریہ;
        idx_ستون_تعداد |= c_نقاب;
        pIdx->ai_عمود[n] = p_رکن->u.x.بایاں_عمود;
        pColl = sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pX);
        أكد( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        اگر( !sqlite3_Is_ثنائی(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_ثنائی;
        n++;
      }
    }
  }
  أكد( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  برائے(i=0; i<mx_ثنائی_ہندسہ_ستون; i++){
    اگر( extra_ستون_تعداد & MASKBIT(i) ){
      pIdx->ai_عمود[n] = i;
      pIdx->azColl[n] = sqlite3_Str_ثنائی;
      n++;
    }
  }
  اگر( pSrc->ستون_مستعمل & MASKBIT(BMS-1) ){
    برائے(i=BMS-1; i<p_جدول->n_ستون; i++){
      pIdx->ai_عمود[n] = i;
      pIdx->azColl[n] = sqlite3_Str_ثنائی;
      n++;
    }
  }
  أكد( n==n_کلید_ستون );
  اگر( HasRowid(p_جدول) ){
    pIdx->ai_عمود[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_ثنائی;
  }

  /* Create the automatic index */
  explain_Automatic_اشاریہ(pParse, pIdx, pPartial!=0, &addrExp);
  أكد( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_کلید_ستون+1);
  sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIdx);
  VdbeComment((v, "for %s", p_جدول->zName));
  اگر( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  أكد( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  اگر( pSrc->fg.بذریعہ_Coroutine ){
    صحیح regYield;
    Subquery *pSubq;
    أكد( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    أكد( pSubq!=0 );
    regYield = pSubq->regReturn;
    addr_شمار = sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, 0);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_چوٹی =  sqlite3_Vdbe_جوڑو_Op1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }ورنہ{
    أكد( pLevel->addrHalt );
    addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  اگر( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_اظہاریہ_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  reg_ریکارڈ = sqlite3_Get_عارضی_Reg(pParse);
  reg_بنیاد = sqlite3_Generate_اشاریہ_کلید(
      pParse, pIdx, pLevel->iTabCur, reg_ریکارڈ, 0, 0, 0, 0
  );
  اگر( pLevel->regFilter ){
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter_جوڑو, pLevel->regFilter, 0,
                         reg_بنیاد, pLoop->u.btree.nEq);
  }
  sqlite3_Vdbe_جائزہ_حالت_Counters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Idx_داخل_کرو, pLevel->iIdxCur, reg_ریکارڈ);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  اگر( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  اگر( pSrc->fg.بذریعہ_Coroutine ){
    أكد( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addr_شمار, reg_بنیاد+n);
    testcase( pParse->db->mallocFailed );
    أكد( pLevel->iIdxCur>0 );
    translate_عمود_مقصد_Copy(pParse, addr_چوٹی, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_نتیجہ, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_چوٹی);
    pSrc->fg.بذریعہ_Coroutine = 0;
    sqlite3_Vdbe_Jump_یہاں(v, addr_چوٹی);
  }ورنہ{
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, pLevel->iTabCur, addr_چوٹی+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    اگر( (pSrc->fg.jointype & JT_بایاں)!=0 ){
      sqlite3_Vdbe_Jump_یہاں(v, addr_چوٹی);
    }
  }
  sqlite3_Release_عارضی_Reg(pParse, reg_ریکارڈ);
 
  /* Jump here when skipping the initialization */
  sqlite3_Vdbe_Jump_یہاں(v, addrInit);
  sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExp, addrExp, -1);

انتہا_auto_اشاریہ_create:
  sqlite3_اظہاریہ_Delete(pParse->db, pPartial);
}
#اگر_ختم /* SQLITE_OMIT_AUTOMATIC_INDEX */

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
ساکن SQLITE_NOINLINE خالی sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  صحیح iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  صحیح addrOnce;                        /* Address of opening OP_Once */
  صحیح addr_چوٹی;                         /* Address of OP_Rewind */
  صحیح addrCont;                        /* Jump here to skip a row */
  ثابت Where_رکن *p_رکن;              /* For looping over WHERE clause terms */
  ثابت Where_رکن *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  صحیح iCur;                            /* Cursor for table getting the filter */
  اشاریہ_دار_اظہاریہ *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  اشاریہ_دار_اظہاریہ *saved_p_Idx_Part_اظہاریہ;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_اظہاریہ = pParse->p_Idx_Part_اظہاریہ;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_اظہاریہ = 0;

  أكد( pLoop!=0 );
  أكد( v!=0 );
  أكد( pLoop->wsFlags & WHERE_BLOOMFILTER );
  أكد( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
  کرو{
    ثابت SrcList *pTabList;
    ثابت Src_شے *p_شے;
    ثابت جدول *pTab;
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
    p_شے = &pTabList->a[iSrc];
    أكد( p_شے!=0 );
    pTab = p_شے->pSTab;
    أكد( pTab!=0 );
    sz = sqlite3_Log_Est_مقصد_Int(pTab->n_صف_Log_Est);
    اگر( sz<10000 ){
      sz = 10000;
    }ورنہ اگر( sz>10000000 ){
      sz = 10000000;
    }
    أكد( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Blob, (صحیح)sz, pLevel->regFilter);

    addr_چوٹی = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.n_رکن];
    برائے(p_رکن=pWInfo->sWC.a; p_رکن<pWCEnd; p_رکن++){
      اظہاریہ *p_اظہاریہ = p_رکن->p_اظہاریہ;
      اگر( (p_رکن->wtFlags & رکن_VIRTUAL)==0
       && sqlite3_اظہاریہ_Is_Single_جدول_Constraint(p_اظہاریہ, pTabList, iSrc, 0)
      ){
        sqlite3_اظہاریہ_If_False(pParse, p_رکن->p_اظہاریہ, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    اگر( pLoop->wsFlags & WHERE_IPK ){
      صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCur, r1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter_جوڑو, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_عارضی_Reg(pParse, r1);
    }ورنہ{
      اشاریہ *pIdx = pLoop->u.btree.p_اشاریہ;
      صحیح n = pLoop->u.btree.nEq;
      صحیح r1 = sqlite3_Get_عارضی_Range(pParse, n);
      صحیح jj;
      أكد( pIdx!=0 );
      برائے(jj=0; jj<n; jj++){
        أكد( pIdx->p_جدول==p_شے->pSTab );
        sqlite3_اظہاریہ_رمز_بھرو_اشاریہ_عمود(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter_جوڑو, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_عارضی_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, pLevel->iTabCur, addr_چوٹی+1);
    VdbeCoverage(v);
    sqlite3_Vdbe_Jump_یہاں(v, addr_چوٹی);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    جب( ++iLevel < pWInfo->nLevel ){
      ثابت Src_شے *p_Tab_شے;
      pLevel = &pWInfo->a[iLevel];
      p_Tab_شے = &pWInfo->pTabList->a[pLevel->iFrom];
      اگر( p_Tab_شے->fg.jointype & (JT_بایاں|JT_LTORJ) ) جاری;
      pLoop = pLevel->pWLoop;
      اگر( NEVER(pLoop==0) ) جاری;
      اگر( pLoop->prereq & notReady ) جاری;
      اگر( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_عمود_اندر))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        توڑو;
      }
    }
  }جب( iLevel < pWInfo->nLevel );
  sqlite3_Vdbe_Jump_یہاں(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_اظہاریہ = saved_p_Idx_Part_اظہاریہ;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
ساکن Where_رکن *رکن_From_Where_Clause(WhereClause *pWC, صحیح i_رکن){
  WhereClause *p;
  برائے(p=pWC; p; p=p->pOuter){
    اگر( i_رکن<p->n_رکن ) واپس &p->a[i_رکن];
    i_رکن -= p->n_رکن;
  }
  واپس 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
ساکن sqlite3_اشاریہ_info *allocate_اشاریہ_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  Src_شے *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  صحیح i, j;
  صحیح n_رکن;
  Parse *pParse = pWInfo->pParse;
  ساخت sqlite3_اشاریہ_constraint *pIdxCons;
  ساخت sqlite3_اشاریہ_orderby *p_Idx_ترتیب_By;
  ساخت sqlite3_اشاریہ_constraint_usage *pUsage;
  ساخت Hidden_اشاریہ_Info *pHidden;
  Where_رکن *p_رکن;
  صحیح n_ترتیب_By;
  sqlite3_اشاریہ_info *pIdxInfo;
  u16 mNoOmit = 0;
  ثابت جدول *pTab;
  صحیح eDistinct = 0;
  اظہاریہ_List *p_ترتیب_By = pWInfo->p_ترتیب_By;
  WhereClause *p;

  أكد( pSrc!=0 );
  pTab = pSrc->pSTab;
  أكد( pTab!=0 );
  أكد( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  برائے(p=pWC, n_رکن=0; p; p=p->pOuter){
    برائے(i=0, p_رکن=p->a; i<p->n_رکن; i++, p_رکن++){
      p_رکن->wtFlags &= ~رکن_OK;
      اگر( p_رکن->بایاں_Cursor != pSrc->iCursor ) جاری;
      اگر( p_رکن->prereq_دایاں & mUnusable ) جاری;
      أكد( Is_Power_کا_Two(p_رکن->eOperator & ~WO_EQUIV) );
      testcase( p_رکن->eOperator & WO_اندر );
      testcase( p_رکن->eOperator & WO_ISNULL );
      testcase( p_رکن->eOperator & WO_IS );
      testcase( p_رکن->eOperator & WO_ALL );
      اگر( (p_رکن->eOperator & ~(WO_EQUIV))==0 ) جاری;
      اگر( p_رکن->wtFlags & رکن_VNULL ) جاری;

      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      أكد( p_رکن->u.x.بایاں_عمود>=XN_ROWID );
      أكد( p_رکن->u.x.بایاں_عمود<pTab->n_ستون );
      اگر( (pSrc->fg.jointype & (JT_بایاں|JT_LTORJ|JT_دایاں))!=0
          && !constraintCompatibleWithOuterJoin(p_رکن,pSrc)
        ){
        جاری;
      }
      n_رکن++;
      p_رکن->wtFlags |= رکن_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  n_ترتیب_By = 0;
  اگر( p_ترتیب_By ){
    صحیح n = p_ترتیب_By->n_اظہاریہ;
    برائے(i=0; i<n; i++){
      اظہاریہ *p_اظہاریہ = p_ترتیب_By->a[i].p_اظہاریہ;
      اظہاریہ *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      اگر( sqlite3_اظہاریہ_Is_Constant(0, p_اظہاریہ) ){
        جاری;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      اگر( p_ترتیب_By->a[i].fg.ترتیب_دو_Flags & KEYINFO_ترتیب_BIGNULL ) توڑو;

      /* First case - a direct column references without a COLLATE operator */
      اگر( p_اظہاریہ->op==TK_عمود && p_اظہاریہ->i_جدول==pSrc->iCursor ){
        أكد( p_اظہاریہ->i_عمود>=XN_ROWID && p_اظہاریہ->i_عمود<pTab->n_ستون );
        جاری;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      اگر( p_اظہاریہ->op==TK_COLLATE
       && (pE2 = p_اظہاریہ->p_بایاں)->op==TK_عمود
       && pE2->i_جدول==pSrc->iCursor
      ){
        ثابت حرف *zColl;  /* The collating sequence name */
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
        أكد( p_اظہاریہ->u.zToken!=0 );
        أكد( pE2->i_عمود>=XN_ROWID && pE2->i_عمود<pTab->n_ستون );
        p_اظہاریہ->i_عمود = pE2->i_عمود;
        اگر( pE2->i_عمود<0 ) جاری;  /* Collseq does not matter for rowid */
        zColl = sqlite3_عمود_Coll(&pTab->a_ستون[pE2->i_عمود]);
        اگر( zColl==0 ) zColl = sqlite3_Str_ثنائی;
        اگر( sqlite3_stricmp(p_اظہاریہ->u.zToken, zColl)==0 ) جاری;
      }

      /* No matches cause a break out of the loop */
      توڑو;
    }
    اگر( i==n ){
      صحیح b_ترتیب_دو_By_گروہ = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      n_ترتیب_By = n;
      اگر( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowid_مستعمل ){
        eDistinct = 2 + b_ترتیب_دو_By_گروہ;
      }ورنہ اگر( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - b_ترتیب_دو_By_گروہ;
      }ورنہ اگر( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, حجم(*pIdxInfo)
                           + (حجم(*pIdxCons) + حجم(*pUsage))*n_رکن
                           + حجم(*p_Idx_ترتیب_By)*n_ترتیب_By
                           + SZ_HIDDENINDEXINFO(n_رکن) );
  اگر( pIdxInfo==0 ){
    sqlite3_Error_پیغام(pParse, "out of memory");
    واپس 0;
  }
  pHidden = (ساخت Hidden_اشاریہ_Info*)&pIdxInfo[1];
  pIdxCons = (ساخت sqlite3_اشاریہ_constraint*)&pHidden->aRhs[n_رکن];
  p_Idx_ترتیب_By = (ساخت sqlite3_اشاریہ_orderby*)&pIdxCons[n_رکن];
  pUsage = (ساخت sqlite3_اشاریہ_constraint_usage*)&p_Idx_ترتیب_By[n_ترتیب_By];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->a_ترتیب_By = p_Idx_ترتیب_By;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->ستون_مستعمل = (sqlite3_int64)pSrc->ستون_مستعمل;
  اگر( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ((جدول*)pTab);
    أكد( pPk!=0 );
    برائے(i=0; i<pPk->n_کلید_ستون; i++){
      صحیح i_ستون = pPk->ai_عمود[i];
      أكد( i_ستون>=0 );
      اگر( i_ستون>=BMS-1 ) i_ستون = BMS-1;
      pIdxInfo->ستون_مستعمل |= MASKBIT(i_ستون);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->m_اندر = 0;
  برائے(p=pWC, i=j=0; p; p=p->pOuter){
    صحیح n_آخری = i+p->n_رکن;;
    برائے(p_رکن=p->a; i<n_آخری; i++, p_رکن++){
      u16 op;
      اگر( (p_رکن->wtFlags & رکن_OK)==0 ) جاری;
      pIdxCons[j].i_عمود = p_رکن->u.x.بایاں_عمود;
      pIdxCons[j].i_رکن_انحراف = i;
      op = p_رکن->eOperator & WO_ALL;
      اگر( op==WO_اندر ){
        اگر( (p_رکن->wtFlags & رکن_قاش)==0 ){
          pHidden->m_اندر |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      اگر( op==WO_AUX ){
        pIdxCons[j].op = p_رکن->e_مطابقت_Op;
      }ورنہ اگر( op & (WO_ISNULL|WO_IS) ){
        اگر( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_اشاریہ_CONSTRAINT_ISNULL;
        }ورنہ{
          pIdxCons[j].op = SQLITE_اشاریہ_CONSTRAINT_IS;
        }
      }ورنہ{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        أكد( WO_EQ==SQLITE_اشاریہ_CONSTRAINT_EQ );
        أكد( WO_LT==SQLITE_اشاریہ_CONSTRAINT_LT );
        أكد( WO_LE==SQLITE_اشاریہ_CONSTRAINT_LE );
        أكد( WO_GT==SQLITE_اشاریہ_CONSTRAINT_GT );
        أكد( WO_GE==SQLITE_اشاریہ_CONSTRAINT_GE );
        أكد( p_رکن->eOperator&(WO_اندر|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        اگر( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_اظہاریہ_Is_سمتیہ(p_رکن->p_اظہاریہ->p_دایاں)
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
  أكد( j==n_رکن );
  pIdxInfo->nConstraint = j;
  برائے(i=j=0; i<n_ترتیب_By; i++){
    اظہاریہ *p_اظہاریہ = p_ترتیب_By->a[i].p_اظہاریہ;
    اگر( sqlite3_اظہاریہ_Is_Constant(0, p_اظہاریہ) ) جاری;
    أكد( p_اظہاریہ->op==TK_عمود
         || (p_اظہاریہ->op==TK_COLLATE && p_اظہاریہ->p_بایاں->op==TK_عمود
              && p_اظہاریہ->i_عمود==p_اظہاریہ->p_بایاں->i_عمود) );
    p_Idx_ترتیب_By[j].i_عمود = p_اظہاریہ->i_عمود;
    p_Idx_ترتیب_By[j].desc = p_ترتیب_By->a[i].fg.ترتیب_دو_Flags & KEYINFO_ترتیب_DESC;
    j++;
  }
  pIdxInfo->n_ترتیب_By = j;

  *pmNoOmit = mNoOmit;
  واپس pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
ساکن خالی freeIdxStr(sqlite3_اشاریہ_info *pIdxInfo){
  اگر( pIdxInfo->need_مقصد_Free_Idx_Str ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->need_مقصد_Free_Idx_Str = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
ساکن خالی free_اشاریہ_Info(sqlite3 *db, sqlite3_اشاریہ_info *pIdxInfo){
  Hidden_اشاریہ_Info *pHidden;
  صحیح i;
  أكد( pIdxInfo!=0 );
  pHidden = (Hidden_اشاریہ_Info*)&pIdxInfo[1];
  أكد( pHidden->pParse!=0 );
  أكد( pHidden->pParse->db==db );
  برائے(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_قدر_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
ساکن صحیح vtab_بہترین_اشاریہ(Parse *pParse, جدول *pTab, sqlite3_اشاریہ_info *p){
  صحیح rc;
  sqlite3_vtab *pVtab;

  أكد( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_اشاریہ_Info_Inputs(p, pTab);
  pParse->db->n_Schema_تالا++;
  rc = pVtab->pModule->x_بہترین_اشاریہ(pVtab, p);
  pParse->db->n_Schema_تالا--;
  where_Trace_اشاریہ_Info_Outputs(p, pTab);

  اگر( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    اگر( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }ورنہ اگر( !pVtab->z_Err_پیغام ){
      sqlite3_Error_پیغام(pParse, "%s", sqlite3ErrStr(rc));
    }ورنہ{
      sqlite3_Error_پیغام(pParse, "%s", pVtab->z_Err_پیغام);
    }
  }
  اگر( pTab->u.vtab.p->bAllSchemas ){
    sqlite3_Vtab_استعمال_کرتا_All_Schemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_پیغام);
  pVtab->z_Err_پیغام = 0;
  واپس rc;
}
#اگر_ختم /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
ساکن صحیح where_کلید_Stats(
  Parse *pParse,              /* Database connection */
  اشاریہ *pIdx,                /* Index to consider domain of */
  Unpacked_ریکارڈ *pRec,       /* Vector of values to consider */
  صحیح باری_اوپر,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  اشاریہ_Sample *aSample = pIdx->aSample;
  صحیح i_ستون;                   /* Index of required stats in anEq[] etc. */
  صحیح i;                      /* Index of first sample >= pRec */
  صحیح iSample;                /* Smallest sample larger than or equal to pRec */
  صحیح iMin = 0;               /* Smallest sample not yet tested */
  صحیح iTest;                  /* Next sample to test */
  صحیح res;                    /* Result of comparison operation */
  صحیح n_میدان;                 /* Number of fields in pRec */
  tRowcnt i_زیریں = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#اگر_غیر_تعریف_شدہ SQLITE_DEBUG
  غیر_مستعمل_PARAMETER( pParse );
#اگر_ختم
  أكد( pRec!=0 );
  أكد( pIdx->nSample>0 );
  أكد( pRec->n_میدان>0 );


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
  اگر( !HasRowid(pIdx->p_جدول) && Is_Primary_کلید_اشاریہ(pIdx) ){
    n_میدان = pIdx->n_کلید_ستون;
  }ورنہ{
    n_میدان = pIdx->n_عمود;
  }
  n_میدان = MIN(pRec->n_میدان, n_میدان);
  i_ستون = 0;
  iSample = pIdx->nSample * n_میدان;
  کرو{
    صحیح iSamp;                    /* Index in aSample[] of test sample */
    صحیح n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / n_میدان;
    اگر( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      برائے(n=(iTest % n_میدان) + 1; n<n_میدان; n++){
        اگر( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) توڑو;
      }
    }ورنہ{
      n = iTest + 1;
    }

    pRec->n_میدان = n;
    res = sqlite3_Vdbe_ریکارڈ_Compare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    اگر( res<0 ){
      i_زیریں = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }ورنہ اگر( res==0 && n<n_میدان ){
      i_زیریں = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }ورنہ{
      iSample = iTest;
      i_ستون = n-1;
    }
  }جب( res && iMin<iSample );
  i = iSample / n_میدان;

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  اگر( pParse->db->mallocFailed==0 ){
    اگر( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      أكد( i<pIdx->nSample );
      أكد( i_ستون==n_میدان-1 );
      pRec->n_میدان = n_میدان;
      أكد( 0==sqlite3_Vdbe_ریکارڈ_Compare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }ورنہ{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      أكد( i<=pIdx->nSample && i>=0 );
      pRec->n_میدان = i_ستون+1;
      أكد( i==pIdx->nSample
           || sqlite3_Vdbe_ریکارڈ_Compare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      اگر( i_ستون>0 ){
        pRec->n_میدان = i_ستون;
        أكد( sqlite3_Vdbe_ریکارڈ_Compare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      اگر( i>0 ){
        pRec->n_میدان = n_میدان;
        أكد( sqlite3_Vdbe_ریکارڈ_Compare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#اگر_ختم /* ifdef SQLITE_DEBUG */

  اگر( res==0 ){
    /* Record pRec is equal to sample i */
    أكد( i_ستون==n_میدان-1 );
    aStat[0] = aSample[i].anLt[i_ستون];
    aStat[1] = aSample[i].anEq[i_ستون];
  }ورنہ{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt i_بالائی, iGap;
    اگر( i>=pIdx->nSample ){
      i_بالائی = pIdx->n_صف_Est0;
    }ورنہ{
      i_بالائی = aSample[i].anLt[i_ستون];
    }

    اگر( i_زیریں>=i_بالائی ){
      iGap = 0;
    }ورنہ{
      iGap = i_بالائی - i_زیریں;
    }
    اگر( باری_اوپر ){
      iGap = (iGap*2)/3;
    }ورنہ{
      iGap = iGap/3;
    }
    aStat[0] = i_زیریں + iGap;
    aStat[1] = pIdx->aAvgEq[n_میدان-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->n_میدان = n_میدان;
  واپس i;
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

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
ساکن LogEst whereRangeAdjust(Where_رکن *p_رکن, LogEst nNew){
  LogEst nRet = nNew;
  اگر( p_رکن ){
    اگر( p_رکن->truthProb<=0 ){
      nRet += p_رکن->truthProb;
    }ورنہ اگر( (p_رکن->wtFlags & رکن_VNULL)==0 ){
      nRet -= 20;        أكد( 20==sqlite3LogEst(4) );
    }
  }
  واپس nRet;
}


#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
حرف sqlite3_اشاریہ_عمود_Affinity(sqlite3 *db, اشاریہ *pIdx, صحیح i_ستون){
  أكد( i_ستون>=0 && i_ستون<pIdx->n_عمود );
  اگر( !pIdx->z_ستون_Aff ){
    اگر( sqlite3_اشاریہ_Affinity_Str(db, pIdx)==0 ) واپس SQLITE_AFF_BLOB;
  }
  أكد( pIdx->z_ستون_Aff[i_ستون]!=0 );
  واپس pIdx->z_ستون_Aff[i_ستون];
}
#اگر_ختم


#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
ساکن صحیح where_Range_Skip_جائزہ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  Where_رکن *p_زیریں,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_رکن *p_بالائی,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  صحیح *pb_مکمل          /* Set to true if at least one expr. value extracted */
){
  اشاریہ *p = pLoop->u.btree.p_اشاریہ;
  صحیح nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  صحیح n_زیریں = -1;
  صحیح n_بالائی = p->nSample+1;
  صحیح rc = SQLITE_OK;
  u8 aff = sqlite3_اشاریہ_عمود_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_قدر *p1 = 0;          /* Value extracted from pLower */
  sqlite3_قدر *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_قدر *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  اگر( p_زیریں ){
    rc = sqlite3_Stat4_قدر_From_اظہاریہ(pParse, p_زیریں->p_اظہاریہ->p_دایاں, aff, &p1);
    n_زیریں = 0;
  }
  اگر( p_بالائی && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_قدر_From_اظہاریہ(pParse, p_بالائی->p_اظہاریہ->p_دایاں, aff, &p2);
    n_بالائی = p2 ? 0 : p->nSample;
  }

  اگر( p1 || p2 ){
    صحیح i;
    صحیح nDiff;
    برائے(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3_Stat4_عمود(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      اگر( rc==SQLITE_OK && p1 ){
        صحیح res = sqlite3MemCompare(p1, pVal, pColl);
        اگر( res>=0 ) n_زیریں++;
      }
      اگر( rc==SQLITE_OK && p2 ){
        صحیح res = sqlite3MemCompare(p2, pVal, pColl);
        اگر( res>=0 ) n_بالائی++;
      }
    }
    nDiff = (n_بالائی - n_زیریں);
    اگر( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    اگر( nDiff!=1 || p_بالائی==0 || p_زیریں==0 ){
      صحیح nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->n_برآمد -= nAdjust;
      *pb_مکمل = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           n_زیریں, n_بالائی, nAdjust*-1, pLoop->n_برآمد));
    }

  }ورنہ{
    أكد( *pb_مکمل==0 );
  }

  sqlite3_قدر_Free(p1);
  sqlite3_قدر_Free(p2);
  sqlite3_قدر_Free(pVal);

  واپس rc;
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

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
ساکن صحیح where_Range_جائزہ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Where_رکن *p_زیریں,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_رکن *p_بالائی,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  صحیح rc = SQLITE_OK;
  صحیح n_برآمد = pLoop->n_برآمد;
  LogEst nNew;

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اشاریہ *p = pLoop->u.btree.p_اشاریہ;
  صحیح nEq = pLoop->u.btree.nEq;

  اگر( p->nSample>0 && ALWAYS(nEq<p->n_Sample_ستون)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    اگر( nEq==pBuilder->nRecValid ){
      Unpacked_ریکارڈ *pRec = pBuilder->pRec;
      tRowcnt a[2];
      صحیح nBtm = pLoop->u.btree.nBtm;
      صحیح n_چوٹی = pLoop->u.btree.n_چوٹی;

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
      tRowcnt i_زیریں;     /* Rows less than the lower bound */
      tRowcnt i_بالائی;     /* Rows less than the upper bound */
      صحیح iLwrIdx = -2;   /* aSample[] for the lower bound */
      صحیح iUprIdx = -1;   /* aSample[] for the upper bound */

      اگر( pRec ){
        testcase( pRec->n_میدان!=pBuilder->nRecValid );
        pRec->n_میدان = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      اگر( nEq==0 ){
        i_زیریں = 0;
        i_بالائی = p->n_صف_Est0;
      }ورنہ{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_کلید_Stats(pParse, p, pRec, 0, a);
        i_زیریں = a[0];
        i_بالائی = a[0] + a[1];
      }

      أكد( p_زیریں==0 || (p_زیریں->eOperator & (WO_GT|WO_GE))!=0 );
      أكد( p_بالائی==0 || (p_بالائی->eOperator & (WO_LT|WO_LE))!=0 );
      أكد( p->a_ترتیب_دو_ترتیب!=0 );
      اگر( p->a_ترتیب_دو_ترتیب[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        تبادلہ(Where_رکن*, p_زیریں, p_بالائی);
        تبادلہ(صحیح, nBtm, n_چوٹی);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      اگر( p_زیریں ){
        صحیح n;                    /* Values extracted from pExpr */
        اظہاریہ *p_اظہاریہ = p_زیریں->p_اظہاریہ->p_دایاں;
        rc = sqlite3_Stat4_Probe_سیٹ_قدر(pParse, p, &pRec, p_اظہاریہ, nBtm, nEq, &n);
        اگر( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 نقاب = WO_GT|WO_LE;
          اگر( sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ)>n ) نقاب = (WO_LE|WO_LT);
          iLwrIdx = where_کلید_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((p_زیریں->eOperator & نقاب) ? a[1] : 0);
          اگر( iNew>i_زیریں ) i_زیریں = iNew;
          n_برآمد--;
          p_زیریں = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      اگر( p_بالائی ){
        صحیح n;                    /* Values extracted from pExpr */
        اظہاریہ *p_اظہاریہ = p_بالائی->p_اظہاریہ->p_دایاں;
        rc = sqlite3_Stat4_Probe_سیٹ_قدر(pParse, p, &pRec, p_اظہاریہ, n_چوٹی, nEq, &n);
        اگر( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 نقاب = WO_GT|WO_LE;
          اگر( sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ)>n ) نقاب = (WO_LE|WO_LT);
          iUprIdx = where_کلید_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((p_بالائی->eOperator & نقاب) ? a[1] : 0);
          اگر( iNew<i_بالائی ) i_بالائی = iNew;
          n_برآمد--;
          p_بالائی = 0;
        }
      }

      pBuilder->pRec = pRec;
      اگر( rc==SQLITE_OK ){
        اگر( i_بالائی>i_زیریں ){
          nNew = sqlite3LogEst(i_بالائی - i_زیریں);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          اگر( iLwrIdx==iUprIdx ){ nNew -= 20; }
          أكد( 20==sqlite3LogEst(4) );
        }ورنہ{
          nNew = 10;        أكد( 10==sqlite3LogEst(2) );
        }
        اگر( nNew<n_برآمد ){
          n_برآمد = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)i_زیریں, (u32)i_بالائی, n_برآمد));
      }
    }ورنہ{
      صحیح b_مکمل = 0;
      rc = where_Range_Skip_جائزہ_Est(pParse, p_زیریں, p_بالائی, pLoop, &b_مکمل);
      اگر( b_مکمل ) واپس rc;
    }
  }
#ورنہ
  غیر_مستعمل_PARAMETER(pParse);
  غیر_مستعمل_PARAMETER(pBuilder);
  أكد( p_زیریں || p_بالائی );
#اگر_ختم
  أكد( p_بالائی==0 || (p_بالائی->wtFlags & رکن_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(p_زیریں, n_برآمد);
  nNew = whereRangeAdjust(p_بالائی, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  اگر( p_زیریں && p_زیریں->truthProb>0 && p_بالائی && p_بالائی->truthProb>0 ){
    nNew -= 20;
  }

  n_برآمد -= (p_زیریں!=0) + (p_بالائی!=0);
  اگر( nNew<10 ) nNew = 10;
  اگر( nNew<n_برآمد ) n_برآمد = nNew;
#اگر defined(WHERETRACE_ENABLED)
  اگر( pLoop->n_برآمد>n_برآمد ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->n_برآمد, n_برآمد));
  }
#اگر_ختم
  pLoop->n_برآمد = (LogEst)n_برآمد;
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
ساکن صحیح where_Equal_جائزہ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  اظہاریہ *p_اظہاریہ,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pn_صف       /* Write the revised row estimate here */
){
  اشاریہ *p = pBuilder->pNew->u.btree.p_اشاریہ;
  صحیح nEq = pBuilder->pNew->u.btree.nEq;
  Unpacked_ریکارڈ *pRec = pBuilder->pRec;
  صحیح rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  صحیح bOk;

  أكد( nEq>=1 );
  أكد( nEq<=p->n_عمود );
  أكد( p->aSample!=0 );
  أكد( p->nSample>0 );
  أكد( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  اگر( pBuilder->nRecValid<(nEq-1) ){
    واپس SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  اگر( nEq>=p->n_عمود ){
    *pn_صف = 1;
    واپس SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_سیٹ_قدر(pParse, p, &pRec, p_اظہاریہ, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  اگر( rc!=SQLITE_OK ) واپس rc;
  اگر( bOk==0 ) واپس SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_کلید_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (صحیح)a[1]));
  *pn_صف = a[1];
 
  واپس rc;
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
ساکن صحیح where_اندر_جائزہ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  اظہاریہ_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pn_صف       /* Write the revised row estimate here */
){
  اشاریہ *p = pBuilder->pNew->u.btree.p_اشاریہ;
  i64 nRow0 = sqlite3_Log_Est_مقصد_Int(p->ai_صف_Log_Est[0]);
  صحیح nRecValid = pBuilder->nRecValid;
  صحیح rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt n_صف_Est = 0;    /* New estimate of the number of rows */
  صحیح i;                  /* Loop counter */

  أكد( p->aSample!=0 );
  برائے(i=0; rc==SQLITE_OK && i<pList->n_اظہاریہ; i++){
    nEst = nRow0;
    rc = where_Equal_جائزہ_Est(pParse, pBuilder, pList->a[i].p_اظہاریہ, &nEst);
    n_صف_Est += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  اگر( rc==SQLITE_OK ){
    اگر( n_صف_Est > (tRowcnt)nRow0 ) n_صف_Est = nRow0;
    *pn_صف = n_صف_Est;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", n_صف_Est));
  }
  أكد( pBuilder->nRecValid==nRecValid );
  واپس rc;
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */


#اگر defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
خالی sqlite3_Where_رکن_Print(Where_رکن *p_رکن, صحیح i_رکن){
  اگر( p_رکن==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", i_رکن);
  }ورنہ{
    حرف zType[8];
    حرف z_بایاں[50];
    حافظہ_نقل(zType, "....", 5);
    اگر( p_رکن->wtFlags & رکن_VIRTUAL ) zType[0] = 'V';
    اگر( p_رکن->eOperator & WO_EQUIV  ) zType[1] = 'E';
    اگر( اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_OuterON) ) zType[2] = 'L';
    اگر( p_رکن->wtFlags & رکن_CODED  ) zType[3] = 'C';
    اگر( p_رکن->eOperator & WO_SINGLE ){
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(حجم(z_بایاں),z_بایاں,"left={%d:%d}",
                       p_رکن->بایاں_Cursor, p_رکن->u.x.بایاں_عمود);
    }ورنہ اگر( (p_رکن->eOperator & WO_OR)!=0 && p_رکن->u.pOrInfo!=0 ){
      sqlite3_snprintf(حجم(z_بایاں),z_بایاں,"indexable=0x%llx",
                       p_رکن->u.pOrInfo->indexable);
    }ورنہ{
      sqlite3_snprintf(حجم(z_بایاں),z_بایاں,"left=%d", p_رکن->بایاں_Cursor);
    }
    i_رکن = p_رکن->i_رکن = MAX(i_رکن,p_رکن->i_رکن);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       i_رکن, p_رکن, zType, z_بایاں, p_رکن->eOperator, p_رکن->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    اگر( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        p_رکن->truthProb, (u64)p_رکن->prereqAll, (u64)p_رکن->prereq_دایاں);
    }
    اگر( (p_رکن->eOperator & (WO_OR|WO_AND))==0 && p_رکن->u.x.i_میدان ){
      sqlite3DebugPrintf(" iField=%d", p_رکن->u.x.i_میدان);
    }
    اگر( p_رکن->i_والد>=0 ){
      sqlite3DebugPrintf(" iParent=%d", p_رکن->i_والد);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_درخت_View_اظہاریہ(0, p_رکن->p_اظہاریہ, 0);
  }
}
خالی sqlite3_Show_Where_رکن(Where_رکن *p_رکن){
  sqlite3_Where_رکن_Print(p_رکن, 0);
}
#اگر_ختم

#اگر_تعریف_شدہ WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
خالی sqlite3WhereClausePrint(WhereClause *pWC){
  صحیح i;
  برائے(i=0; i<pWC->n_رکن; i++){
    sqlite3_Where_رکن_Print(&pWC->a[i], i);
  }
}
#اگر_ختم

#اگر_تعریف_شدہ WHERETRACE_ENABLED
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
خالی sqlite3WhereLoopPrint(ثابت WhereLoop *p, ثابت WhereClause *pWC){
  WhereInfo *pWInfo;
  اگر( pWC ){
    صحیح nb;
    Src_شے *p_شے;
    جدول *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    p_شے = pWInfo->pTabList->a + p->iTab;
    pTab = p_شے->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->نقاب_Self, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       p_شے->zAlias ? p_شے->zAlias : pTab->zName);
  }ورنہ{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->نقاب_Self, p->prereq & 0xfff, p->cId, p->iTab);
  }
  اگر( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ثابت حرف *zName;
    اگر( p->u.btree.p_اشاریہ && (zName = p->u.btree.p_اشاریہ->zName)!=0 ){
      اگر( لڑی_موازنہ(zName, "sqlite_autoindex_", 17)==0 ){
        صحیح i = sqlite3Strlen30(zName) - 1;
        جب( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }ورنہ{
      sqlite3DebugPrintf("%20s","");
    }
  }ورنہ{
    حرف *z;
    اگر( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_عدد, p->u.vtab.idxStr, p->u.vtab.omit_نقاب);
    }ورنہ{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_عدد, p->u.vtab.omit_نقاب);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  اگر( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }ورنہ{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  اگر( pWInfo && pWInfo->b_ستارہ_مستعمل && p->r_ستارہ_Delta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->r_تیاری, p->r_تسلسل, p->n_برآمد, p->r_ستارہ_Delta);
  }ورنہ{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->r_تیاری, p->r_تسلسل, p->n_برآمد);
  }
  اگر( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    صحیح i;
    برائے(i=0; i<p->nLTerm; i++){
      sqlite3_Where_رکن_Print(p->aLTerm[i], i);
    }
  }
}
خالی sqlite3ShowWhereLoop(ثابت WhereLoop *p){
  اگر( p ) sqlite3WhereLoopPrint(p, 0);
}
خالی sqlite3ShowWhereLoopList(ثابت WhereLoop *p){
  جب( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_اگلا_Loop;
  }
}
#اگر_ختم

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
ساکن خالی whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_سائز(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
ساکن خالی whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  اگر( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_اشاریہ) ){
    اگر( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }ورنہ اگر( (p->wsFlags & WHERE_AUTO_اشاریہ)!=0 && p->u.btree.p_اشاریہ!=0 ){
      sqlite3DbFree(db, p->u.btree.p_اشاریہ->z_ستون_Aff);
      sqlite3DbFreeNN(db, p->u.btree.p_اشاریہ);
      p->u.btree.p_اشاریہ = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
ساکن خالی whereLoopClear(sqlite3 *db, WhereLoop *p){
  اگر( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_سائز(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
ساکن صحیح whereLoopResize(sqlite3 *db, WhereLoop *p, صحیح n){
  Where_رکن **paNew;
  اگر( p->nLSlot>=n ) واپس SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, حجم(p->aLTerm[0])*n);
  اگر( paNew==0 ) واپس SQLITE_NOMEM_BKPT;
  حافظہ_نقل(paNew, p->aLTerm, حجم(p->aLTerm[0])*p->nLSlot);
  اگر( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  واپس SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
ساکن صحیح whereLoopXfer(sqlite3 *db, WhereLoop *p_مقصد, WhereLoop *pFrom){
  whereLoopClearUnion(db, p_مقصد);
  اگر( pFrom->nLTerm > p_مقصد->nLSlot
   && whereLoopResize(db, p_مقصد, pFrom->nLTerm)
  ){
    memset(p_مقصد, 0, WHERE_LOOP_XFER_SZ);
    واپس SQLITE_NOMEM_BKPT;
  }
  حافظہ_نقل(p_مقصد, pFrom, WHERE_LOOP_XFER_SZ);
  حافظہ_نقل(p_مقصد->aLTerm, pFrom->aLTerm, p_مقصد->nLTerm*حجم(p_مقصد->aLTerm[0]));
  اگر( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }ورنہ اگر( (pFrom->wsFlags & WHERE_AUTO_اشاریہ)!=0 ){
    pFrom->u.btree.p_اشاریہ = 0;
  }
  واپس SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
ساکن خالی whereLoopDelete(sqlite3 *db, WhereLoop *p){
  أكد( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
ساکن خالی whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  أكد( pWInfo!=0 );
  أكد( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  جب( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_اگلا_Loop;
    whereLoopDelete(db, p);
  }
  جب( pWInfo->p_Mem_مقصد_Free ){
    Where_Mem_بلاک *p_اگلا = pWInfo->p_Mem_مقصد_Free->p_اگلا;
    sqlite3DbNNFreeNN(db, pWInfo->p_Mem_مقصد_Free);
    pWInfo->p_Mem_مقصد_Free = p_اگلا;
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
ساکن صحیح whereLoopCheaperProperSubset(
  ثابت WhereLoop *pX,       /* First WhereLoop to compare */
  ثابت WhereLoop *pY        /* Compare against this WhereLoop */
){
  صحیح i, j;
  اگر( pX->r_تسلسل>pY->r_تسلسل && pX->n_برآمد>pY->n_برآمد ) واپس 0; /* (1d) and (2a) */
  أكد( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  اگر( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_اشاریہ==pY->u.btree.p_اشاریہ             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    واپس 1;  /* Case 1 is true */
  }
  اگر( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    واپس 0;                                            /* (2b) */
  }
  اگر( pY->nSkip > pX->nSkip ) واپس 0;                  /* (2d) */
  برائے(i=pX->nLTerm-1; i>=0; i--){
    اگر( pX->aLTerm[i]==0 ) جاری;
    برائے(j=pY->nLTerm-1; j>=0; j--){
      اگر( pY->aLTerm[j]==pX->aLTerm[i] ) توڑو;
    }
    اگر( j<0 ) واپس 0;                                  /* (2c) */
  }
  اگر( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    واپس 0;                                            /* (2e) */
  }
  واپس 1;  /* Case 2 is true */
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
ساکن خالی where_Loop_Adjust_لاگت(ثابت WhereLoop *p, WhereLoop *pTemplate){
  اگر( (pTemplate->wsFlags & WHERE_اشاریہ_دار)==0 ) واپس;
  برائے(; p; p=p->p_اگلا_Loop){
    اگر( p->iTab!=pTemplate->iTab ) جاری;
    اگر( (p->wsFlags & WHERE_اشاریہ_دار)==0 ) جاری;
    اگر( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_تسلسل, pTemplate->n_برآمد,
                       MIN(p->r_تسلسل, pTemplate->r_تسلسل),
                       MIN(p->n_برآمد - 1, pTemplate->n_برآمد)));
      pTemplate->r_تسلسل = MIN(p->r_تسلسل, pTemplate->r_تسلسل);
      pTemplate->n_برآمد = MIN(p->n_برآمد - 1, pTemplate->n_برآمد);
    }ورنہ اگر( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_تسلسل, pTemplate->n_برآمد,
                       MAX(p->r_تسلسل, pTemplate->r_تسلسل),
                       MAX(p->n_برآمد + 1, pTemplate->n_برآمد)));
      pTemplate->r_تسلسل = MAX(p->r_تسلسل, pTemplate->r_تسلسل);
      pTemplate->n_برآمد = MAX(p->n_برآمد + 1, pTemplate->n_برآمد);
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
ساکن WhereLoop **where_Loop_ڈھونڈو_Lesser(
  WhereLoop **pp_پچھلا,
  ثابت WhereLoop *pTemplate
){
  WhereLoop *p;
  برائے(p=(*pp_پچھلا); p; pp_پچھلا=&p->p_اگلا_Loop, p=*pp_پچھلا){
    اگر( p->iTab!=pTemplate->iTab || p->i_ترتیب_دو_Idx!=pTemplate->i_ترتیب_دو_Idx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      جاری;
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
    اگر( (p->wsFlags & WHERE_AUTO_اشاریہ)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_اشاریہ_دار)!=0
     && (pTemplate->wsFlags & WHERE_عمود_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      توڑو;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    اگر( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->r_تیاری<=pTemplate->r_تیاری                  /* (2a) */
     && p->r_تسلسل<=pTemplate->r_تسلسل                      /* (2b) */
     && p->n_برآمد<=pTemplate->n_برآمد                      /* (2c) */
    ){
      واپس 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    اگر( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->r_تسلسل>=pTemplate->r_تسلسل                             /* (2a) */
     && p->n_برآمد>=pTemplate->n_برآمد                             /* (2b) */
    ){
      أكد( p->r_تیاری>=pTemplate->r_تیاری ); /* SETUP-INVARIANT above */
      توڑو;   /* Cause p to be overwritten by pTemplate */
    }
  }
  واپس pp_پچھلا;
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
ساکن صحیح where_Loop_داخل_کرو(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **pp_پچھلا, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  صحیح rc;

  /* Stop the search once we hit the query planner search limit */
  اگر( pBuilder->i_Plan_حد_بندی==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    اگر( pBuilder->p_Or_سیٹ ) pBuilder->p_Or_سیٹ->n = 0;
    واپس SQLITE_مکمل;
  }
  pBuilder->i_Plan_حد_بندی--;

  where_Loop_Adjust_لاگت(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  اگر( pBuilder->p_Or_سیٹ!=0 ){
    اگر( pTemplate->nLTerm ){
#اگر WHERETRACE_ENABLED
      u16 n = pBuilder->p_Or_سیٹ->n;
      صحیح x =
#اگر_ختم
      where_Or_داخل_کرو(pBuilder->p_Or_سیٹ, pTemplate->prereq, pTemplate->r_تسلسل,
                                    pTemplate->n_برآمد);
#اگر WHERETRACE_ENABLED /* 0x8 */
      اگر( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#اگر_ختم
    }
    واپس SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  pp_پچھلا = where_Loop_ڈھونڈو_Lesser(&pWInfo->pLoops, pTemplate);

  اگر( pp_پچھلا==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#اگر WHERETRACE_ENABLED /* 0x8 */
    اگر( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#اگر_ختم
    واپس SQLITE_OK; 
  }ورنہ{
    p = *pp_پچھلا;
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
    }ورنہ{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#اگر_ختم
  اگر( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *pp_پچھلا = p = sqlite3DbMallocRawNN(db, حجم(WhereLoop));
    اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_اگلا_Loop = 0;
  }ورنہ{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_دم = &p->p_اگلا_Loop;
    WhereLoop *p_مقصد_Del;
    جب( *pp_دم ){
      pp_دم = where_Loop_ڈھونڈو_Lesser(pp_دم, pTemplate);
      اگر( pp_دم==0 ) توڑو;
      p_مقصد_Del = *pp_دم;
      اگر( p_مقصد_Del==0 ) توڑو;
      *pp_دم = p_مقصد_Del->p_اگلا_Loop;
#اگر WHERETRACE_ENABLED /* 0x8 */
      اگر( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(p_مقصد_Del, pBuilder->pWC);
      }
#اگر_ختم
      whereLoopDelete(db, p_مقصد_Del);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  اگر( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    اشاریہ *p_اشاریہ = p->u.btree.p_اشاریہ;
    اگر( p_اشاریہ && p_اشاریہ->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_اشاریہ = 0;
    }
  }
  واپس rc;
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
ساکن صحیح اظہاریہ_گرہ_نمونہ_لمبائی_Est(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_STRING ){
    صحیح sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_اظہاریہ->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    اگر( pWalker->e_رمز ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }ورنہ{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    جب( (c = *(z++))!=0 ){
      اگر( c==c3 ){
        اگر( *z ) z++;
        جب( *z && *z!=']' ) z++;
      }ورنہ اگر( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    اگر( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  واپس WRC_Continue;
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
ساکن صحیح est_Like_نمونہ_لمبائی(اظہاریہ *p, u16 e_رمز){
  Walker w;
  w.u.sz = 0;
  w.e_رمز = e_رمز;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_نمونہ_لمبائی_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شدہ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#اگر_ختم
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.u.sz;
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
ساکن خالی whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst n_صف            /* Number of rows in the entire table */
){
  Where_رکن *p_رکن, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->نقاب_Self);
  صحیح i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  اگر( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->p_ترتیب_By==0 ){
    واپس;
  }

  أكد( (pLoop->wsFlags & WHERE_AUTO_اشاریہ)==0 );
  برائے(i=pWC->n_بنیاد, p_رکن=pWC->a; i>0; i--, p_رکن++){
    أكد( p_رکن!=0 );
    اگر( (p_رکن->prereqAll & notAllowed)!=0 ) جاری;
    اگر( (p_رکن->prereqAll & pLoop->نقاب_Self)==0 ) جاری;
    اگر( (p_رکن->wtFlags & رکن_VIRTUAL)!=0 ) جاری;
    برائے(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      اگر( pX==0 ) جاری;
      اگر( pX==p_رکن ) توڑو;
      اگر( pX->i_والد>=0 && (&pWC->a[pX->i_والد])==p_رکن ) توڑو;
    }
    اگر( j<0 ){
      sqlite3_پیش_رفت_Check(pWC->pWInfo->pParse);
      اگر( pLoop->نقاب_Self==p_رکن->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        اگر( (p_رکن->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_بایاں|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      اگر( p_رکن->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->n_برآمد += p_رکن->truthProb;
      }ورنہ{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        اظہاریہ *p_Op_اظہاریہ = p_رکن->p_اظہاریہ;
        pLoop->n_برآمد--;
        اگر( (p_رکن->eOperator&(WO_EQ|WO_IS))!=0
         && (p_رکن->wtFlags & رکن_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          اظہاریہ *p_دایاں = p_Op_اظہاریہ->p_دایاں;
          Parse *pParse = pWC->pWInfo->pParse;
          صحیح k = 0;
          testcase( p_Op_اظہاریہ->op==TK_IS );
          اگر( sqlite3_اظہاریہ_Is_Integer(p_دایاں, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }ورنہ{
            k = 20;
          }
          اگر( iReduce<k ){
            p_رکن->wtFlags |= رکن_HEURTRUTH;
            iReduce = k;
          }
        }ورنہ
        اگر( اظہاریہ_Has_Property(p_Op_اظہاریہ, EP_InfixFunc) 
         && p_Op_اظہاریہ->op==TK_FUNCTION
        ){
          صحیح eOp;
          أكد( اظہاریہ_Use_XList(p_Op_اظہاریہ) );
          أكد( p_Op_اظہاریہ->x.pList->n_اظہاریہ>=2 );
          eOp = sqlite3_اظہاریہ_Is_Like_Operator(p_Op_اظہاریہ);
          اگر( ALWAYS(eOp>0) ){
            صحیح sz_نمونہ;
            اظہاریہ *pRHS = p_Op_اظہاریہ->x.pList->a[0].p_اظہاریہ;
            eOp = eOp==SQLITE_اشاریہ_CONSTRAINT_LIKE;
            sz_نمونہ = est_Like_نمونہ_لمبائی(pRHS, eOp);
            اگر( sz_نمونہ>0 ){
              pLoop->n_برآمد -= sz_نمونہ*2;
            }
          }
        }
      }
    }
  }
  اگر( pLoop->n_برآمد > n_صف-iReduce ){
    pLoop->n_برآمد = n_صف - iReduce;
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
ساکن صحیح where_Range_سمتیہ_Len(
  Parse *pParse,       /* Parsing context */
  صحیح iCur,            /* Cursor open on pIdx */
  اشاریہ *pIdx,         /* The index to be used for a inequality constraint */
  صحیح nEq,             /* Number of prior equality constraints on same index */
  Where_رکن *p_رکن     /* The vector inequality constraint */
){
  صحیح nCmp = sqlite3_اظہاریہ_سمتیہ_سائز(p_رکن->p_اظہاریہ->p_بایاں);
  صحیح i;

  nCmp = MIN(nCmp, (pIdx->n_عمود - nEq));
  برائے(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    حرف aff;                     /* Comparison affinity */
    حرف idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    اظہاریہ *pLhs, *pRhs;

    أكد( اظہاریہ_Use_XList(p_رکن->p_اظہاریہ->p_بایاں) );
    pLhs = p_رکن->p_اظہاریہ->p_بایاں->x.pList->a[i].p_اظہاریہ;
    pRhs = p_رکن->p_اظہاریہ->p_دایاں;
    اگر( اظہاریہ_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_اظہاریہ;
    }ورنہ{
      pRhs = pRhs->x.pList->a[i].p_اظہاریہ;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    اگر( pLhs->op!=TK_عمود
     || pLhs->i_جدول!=iCur
     || pLhs->i_عمود!=pIdx->ai_عمود[i+nEq]
     || pIdx->a_ترتیب_دو_ترتیب[i+nEq]!=pIdx->a_ترتیب_دو_ترتیب[nEq]
    ){
      توڑو;
    }

    testcase( pLhs->i_عمود==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_اظہاریہ_Affinity(pLhs));
    idxaff = sqlite3_جدول_عمود_Affinity(pIdx->p_جدول, pLhs->i_عمود);
    اگر( aff!=idxaff ) توڑو;

    اگر( اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_Commuted) ) تبادلہ(اظہاریہ*, pRhs, pLhs);
    pColl = sqlite3_ثنائی_Compare_Coll_Seq(pParse, pLhs, pRhs);
    اگر( pColl==0 ) توڑو;
    اگر( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) توڑو;
  }
  واپس i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_COSTMULT
# define Apply_لاگت_Multiplier(C,T)  C += T
#ورنہ
# define Apply_لاگت_Multiplier(C,T)
#اگر_ختم

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
ساکن صحیح where_Loop_جوڑو_Btree_اشاریہ(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  Src_شے *pSrc,                  /* FROM clause term being analyzed */
  اشاریہ *pProbe,                  /* An index on pSrc */
  LogEst n_اندر_Mul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  Where_رکن *p_رکن;               /* A WhereTerm under consideration */
  صحیح op_نقاب;                     /* Valid operators for constraints */
  Where_جائزہ جائزہ;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_چوٹی;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_n_برآمد;              /* Original value of pNew->nOut */
  صحیح rc = SQLITE_OK;             /* Return code */
  LogEst r_سائز;                   /* Number of rows in the table */
  LogEst r_Log_سائز;                /* Logarithm of table size */
  Where_رکن *p_چوٹی = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  أكد( db->mallocFailed==0 || pParse->nErr>0 );
  اگر( pParse->nErr ){
    واپس pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->p_جدول->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->r_تسلسل));

  أكد( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  أكد( (pNew->wsFlags & WHERE_چوٹی_حد_بندی)==0 );
  اگر( pNew->wsFlags & WHERE_BTM_حد_بندی ){
    op_نقاب = WO_LT|WO_LE;
  }ورنہ{
    أكد( pNew->u.btree.nBtm==0 );
    op_نقاب = WO_EQ|WO_اندر|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  اگر( pProbe->bUnordered ){
    op_نقاب &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  أكد( pNew->u.btree.nEq<pProbe->n_عمود );
  أكد( pNew->u.btree.nEq<pProbe->n_کلید_ستون
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_چوٹی = pNew->u.btree.n_چوٹی;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_n_برآمد = pNew->n_برآمد;
  p_رکن = where_جائزہ_Init(&جائزہ, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        op_نقاب, pProbe);
  pNew->r_تیاری = 0;
  r_سائز = pProbe->ai_صف_Log_Est[0];
  r_Log_سائز = estLog(r_سائز);
  برائے(; rc==SQLITE_OK && p_رکن!=0; p_رکن = where_جائزہ_اگلا(&جائزہ)){
    u16 eOp = p_رکن->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst r_لاگت_Idx;
    LogEst n_برآمد_Unadjusted;        /* nOut before IN() and WHERE adjustments */
    صحیح n_اندر = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    صحیح nRecValid = pBuilder->nRecValid;
#اگر_ختم
    اگر( (eOp==WO_ISNULL || (p_رکن->wtFlags&رکن_VNULL)!=0)
     && اشاریہ_عمود_Not_Null(pProbe, saved_nEq)
    ){
      جاری; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    اگر( p_رکن->prereq_دایاں & pNew->نقاب_Self ) جاری;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    اگر( p_رکن->wtFlags & رکن_LIKEOPT && p_رکن->eOperator==WO_LT ) جاری;

    اگر( (pSrc->fg.jointype & (JT_بایاں|JT_LTORJ|JT_دایاں))!=0
     && !constraintCompatibleWithOuterJoin(p_رکن,pSrc)
    ){
      جاری;
    }
    اگر( Is_Unique_اشاریہ(pProbe) && saved_nEq==pProbe->n_کلید_ستون-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }ورنہ{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_اشاریہ_دار;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_چوٹی = saved_n_چوٹی;
    pNew->nLTerm = saved_nLTerm;
    اگر( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       توڑو; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = p_رکن;
    pNew->prereq = (saved_prereq | p_رکن->prereq_دایاں) & ~pNew->نقاب_Self;

    أكد( n_اندر_Mul==0
        || (pNew->wsFlags & WHERE_عمود_NULL)!=0
        || (pNew->wsFlags & WHERE_عمود_اندر)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    اگر( eOp & WO_اندر ){
      اظہاریہ *p_اظہاریہ = p_رکن->p_اظہاریہ;
      اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        صحیح i;
        صحیح bRedundant = 0;
        n_اندر = 46;  أكد( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        برائے(i=0; i<pNew->nLTerm-1; i++){
          اگر( pNew->aLTerm[i] && pNew->aLTerm[i]->p_اظہاریہ==p_اظہاریہ ){
            n_اندر = 0;
            اگر( pNew->aLTerm[i]->u.x.i_میدان == p_رکن->u.x.i_میدان ){
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
          جاری;
        }
      }ورنہ اگر( ALWAYS(p_اظہاریہ->x.pList && p_اظہاریہ->x.pList->n_اظہاریہ) ){
        /* "x IN (value, value, ...)" */
        n_اندر = sqlite3LogEst(p_اظہاریہ->x.pList->n_اظہاریہ);
      }
      اگر( pProbe->hasStat1 && r_Log_سائز>=10 ){
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
        M = pProbe->ai_صف_Log_Est[saved_nEq];
        logK = estLog(n_اندر);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (n_اندر + r_Log_سائز);
        اگر( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, n_اندر, r_Log_سائز, x));
        }ورنہ اگر( n_اندر_Mul<2 && OptimizationEnabled(db, SQLITE_Seek_جائزہ) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, n_اندر, r_Log_سائز, x, n_اندر_Mul));
          pNew->wsFlags |= WHERE_اندر_SEEKSCAN;
        }ورنہ{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, n_اندر, r_Log_سائز, x, n_اندر_Mul));
          جاری;
        }
      }
      pNew->wsFlags |= WHERE_عمود_اندر;
    }ورنہ اگر( eOp & (WO_EQ|WO_IS) ){
      صحیح i_ستون = pProbe->ai_عمود[saved_nEq];
      pNew->wsFlags |= WHERE_عمود_EQ;
      أكد( saved_nEq==pNew->u.btree.nEq );
      اگر( i_ستون==XN_ROWID
       || (i_ستون>=0 && n_اندر_Mul==0 && saved_nEq==pProbe->n_کلید_ستون-1)
      ){
        اگر( i_ستون==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_کلید_ستون==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }ورنہ{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      اگر( جائزہ.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }ورنہ اگر( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_عمود_NULL;
    }ورنہ{
      صحیح nVecLen = where_Range_سمتیہ_Len(
          pParse, pSrc->iCursor, pProbe, saved_nEq, p_رکن
      );
      اگر( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_عمود_RANGE|WHERE_BTM_حد_بندی;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = p_رکن;
        p_چوٹی = 0;
        اگر( p_رکن->wtFlags & رکن_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_چوٹی = &p_رکن[1];
          أكد( (p_چوٹی-(p_رکن->pWC->a))<p_رکن->pWC->n_رکن );
          أكد( p_چوٹی->wtFlags & رکن_LIKEOPT );
          أكد( p_چوٹی->eOperator==WO_LT );
          اگر( whereLoopResize(db, pNew, pNew->nLTerm+1) ) توڑو; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_چوٹی;
          pNew->wsFlags |= WHERE_چوٹی_حد_بندی;
          pNew->u.btree.n_چوٹی = 1;
        }
      }ورنہ{
        أكد( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_عمود_RANGE|WHERE_چوٹی_حد_بندی;
        pNew->u.btree.n_چوٹی = nVecLen;
        p_چوٹی = p_رکن;
        pBtm = (pNew->wsFlags & WHERE_BTM_حد_بندی)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    أكد( pNew->n_برآمد==saved_n_برآمد );
    اگر( pNew->wsFlags & WHERE_عمود_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      where_Range_جائزہ_Est(pParse, pBuilder, pBtm, p_چوٹی, pNew);
    }ورنہ{
      صحیح nEq = ++pNew->u.btree.nEq;
      أكد( eOp & (WO_ISNULL|WO_EQ|WO_اندر|WO_IS) );

      أكد( pNew->n_برآمد==saved_n_برآمد );
      اگر( p_رکن->truthProb<=0 && pProbe->ai_عمود[saved_nEq]>=0 ){
        أكد( (eOp & WO_اندر) || n_اندر==0 );
        testcase( eOp & WO_اندر );
        pNew->n_برآمد += p_رکن->truthProb;
        pNew->n_برآمد -= n_اندر;
      }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
        tRowcnt n_برآمد = 0;
        اگر( n_اندر_Mul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->n_Sample_ستون)
         && ((eOp & WO_اندر)==0 || اظہاریہ_Use_XList(p_رکن->p_اظہاریہ))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          اظہاریہ *p_اظہاریہ = p_رکن->p_اظہاریہ;
          اگر( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = where_Equal_جائزہ_Est(pParse, pBuilder, p_اظہاریہ->p_دایاں, &n_برآمد);
          }ورنہ{
            rc = where_اندر_جائزہ_Est(pParse, pBuilder, p_اظہاریہ->x.pList, &n_برآمد);
          }
          اگر( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          اگر( rc!=SQLITE_OK ) توڑو;          /* Jump out of the pTerm loop */
          اگر( n_برآمد ){
            pNew->n_برآمد = sqlite3LogEst(n_برآمد);
            اگر( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->n_برآمد+10 > pProbe->ai_صف_Log_Est[0]
            ){
#اگر WHERETRACE_ENABLED /* 0x01 */
              اگر( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3_Where_رکن_Print(p_رکن, 999);
              }
#اگر_ختم
              p_رکن->wtFlags |= رکن_HIGHTRUTH;
              اگر( p_رکن->wtFlags & رکن_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            اگر( pNew->n_برآمد>saved_n_برآمد ) pNew->n_برآمد = saved_n_برآمد;
            pNew->n_برآمد -= n_اندر;
          }
        }
        اگر( n_برآمد==0 )
#اگر_ختم
        {
          pNew->n_برآمد += (pProbe->ai_صف_Log_Est[nEq] - pProbe->ai_صف_Log_Est[nEq-1]);
          اگر( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->n_برآمد += 10;
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
    أكد( pSrc->pSTab->sz_Tab_صف>0 );
    اگر( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      r_لاگت_Idx = pNew->n_برآمد + 16;
    }ورنہ{
      r_لاگت_Idx = pNew->n_برآمد + 1 + (15*pProbe->sz_Idx_صف)/pSrc->pSTab->sz_Tab_صف;
    }
    r_لاگت_Idx = sqlite3_Log_Est_جوڑو(r_Log_سائز, r_لاگت_Idx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->r_تسلسل = r_لاگت_Idx;
    اگر( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->r_تسلسل = sqlite3_Log_Est_جوڑو(pNew->r_تسلسل, pNew->n_برآمد + 16);
    }
    Apply_لاگت_Multiplier(pNew->r_تسلسل, pProbe->p_جدول->لاگت_Mult);

    n_برآمد_Unadjusted = pNew->n_برآمد;
    pNew->r_تسلسل += n_اندر_Mul + n_اندر;
    pNew->n_برآمد += n_اندر_Mul + n_اندر;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_سائز);
    اگر( pSrc->fg.fromExists ) pNew->n_برآمد = 0;
    rc = where_Loop_داخل_کرو(pBuilder, pNew);

    اگر( pNew->wsFlags & WHERE_عمود_RANGE ){
      pNew->n_برآمد = saved_n_برآمد;
    }ورنہ{
      pNew->n_برآمد = n_برآمد_Unadjusted;
    }

    اگر( (pNew->wsFlags & WHERE_چوٹی_حد_بندی)==0
     && pNew->u.btree.nEq<pProbe->n_عمود
     && (pNew->u.btree.nEq<pProbe->n_کلید_ستون ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      اگر( pNew->u.btree.nEq>3 ){
        sqlite3_پیش_رفت_Check(pParse);
      }
      where_Loop_جوڑو_Btree_اشاریہ(pBuilder, pSrc, pProbe, n_اندر_Mul+n_اندر);
    }
    pNew->n_برآمد = saved_n_برآمد;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#اگر_ختم
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_چوٹی = saved_n_چوٹی;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->n_برآمد = saved_n_برآمد;
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
   && saved_nEq+1<pProbe->n_کلید_ستون
   && saved_nEq==pNew->nLTerm
   && pProbe->no_Skip_جائزہ==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_Skip_جائزہ)
   && pProbe->ai_صف_Log_Est[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->ai_صف_Log_Est[saved_nEq] - pProbe->ai_صف_Log_Est[saved_nEq+1];
    pNew->n_برآمد -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    where_Loop_جوڑو_Btree_اشاریہ(pBuilder, pSrc, pProbe, nIter + n_اندر_Mul);
    pNew->n_برآمد = saved_n_برآمد;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->p_جدول->zName, pProbe->zName, saved_nEq, rc));
  واپس rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
ساکن صحیح اشاریہ_Might_Help_With_ترتیب_By(
  WhereLoopBuilder *pBuilder,
  اشاریہ *p_اشاریہ,
  صحیح iCursor
){
  اظہاریہ_List *pOB;
  اظہاریہ_List *a_ستون_اظہاریہ;
  صحیح ii, jj;

  اگر( p_اشاریہ->bUnordered ) واپس 0;
  اگر( (pOB = pBuilder->pWInfo->p_ترتیب_By)==0 ) واپس 0;
  برائے(ii=0; ii<pOB->n_اظہاریہ; ii++){
    اظہاریہ *p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pOB->a[ii].p_اظہاریہ);
    اگر( NEVER(p_اظہاریہ==0) ) جاری;
    اگر( (p_اظہاریہ->op==TK_عمود || p_اظہاریہ->op==TK_AGG_عمود) 
     && p_اظہاریہ->i_جدول==iCursor 
    ){
      اگر( p_اظہاریہ->i_عمود<0 ) واپس 1;
      برائے(jj=0; jj<p_اشاریہ->n_کلید_ستون; jj++){
        اگر( p_اظہاریہ->i_عمود==p_اشاریہ->ai_عمود[jj] ) واپس 1;
      }
    }ورنہ اگر( (a_ستون_اظہاریہ = p_اشاریہ->a_ستون_اظہاریہ)!=0 ){
      برائے(jj=0; jj<p_اشاریہ->n_کلید_ستون; jj++){
        اگر( p_اشاریہ->ai_عمود[jj]!=XN_اظہاریہ ) جاری;
        اگر( sqlite3_اظہاریہ_Compare_Skip(p_اظہاریہ,a_ستون_اظہاریہ->a[jj].p_اظہاریہ,iCursor)==0 ){
          واپس 1;
        }
      }
    }
  }
  واپس 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
ساکن صحیح where_Usable_Partial_اشاریہ(
  صحیح iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  اظہاریہ *pWhere          /* The WHERE clause from the partial index */
){
  صحیح i;
  Where_رکن *p_رکن;
  Parse *pParse;

  اگر( jointype & JT_LTORJ ) واپس 0;
  pParse = pWC->pWInfo->pParse;
  جب( pWhere->op==TK_AND ){
    اگر( !where_Usable_Partial_اشاریہ(iTab,jointype,pWC,pWhere->p_بایاں) ) واپس 0;
    pWhere = pWhere->p_دایاں;
  }
  برائے(i=0, p_رکن=pWC->a; i<pWC->n_رکن; i++, p_رکن++){
    اظہاریہ *p_اظہاریہ;
    p_اظہاریہ = p_رکن->p_اظہاریہ;
    اگر( (!اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) || p_اظہاریہ->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON))
     && sqlite3_اظہاریہ_مستلزم_اظہاریہ(pParse, p_اظہاریہ, pWhere, iTab)
     && !sqlite3_اظہاریہ_مستلزم_اظہاریہ(pParse, p_اظہاریہ, pWhere, -1)
     && (p_رکن->wtFlags & رکن_VNULL)==0
    ){
      واپس 1;
    }
  }
  واپس 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
ساکن صحیح اظہاریہ_Is_Covered_By_اشاریہ(
  ثابت اظہاریہ *p_اظہاریہ,
  ثابت اشاریہ *pIdx,
  صحیح iTabCur
){
  صحیح i;
  برائے(i=0; i<pIdx->n_عمود; i++){
    اگر( pIdx->ai_عمود[i]==XN_اظہاریہ
     && sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, pIdx->a_ستون_اظہاریہ->a[i].p_اظہاریہ, iTabCur)==0
    ){
      واپس 1;
    }
  }
  واپس 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
قسمتعریف ساخت Covering_اشاریہ_Check Covering_اشاریہ_Check;
ساخت Covering_اشاریہ_Check {
  اشاریہ *pIdx;       /* The index */
  صحیح iTabCur;       /* Cursor number for the corresponding table */
  u8 b_اظہاریہ;          /* Uses an indexed expression */
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
ساکن صحیح where_Is_Covering_اشاریہ_Walk_Callback(Walker *pWalk, اظہاریہ *p_اظہاریہ){
  صحیح i;                    /* Loop counter */
  ثابت اشاریہ *pIdx;        /* The index of interest */
  ثابت i16 *ai_عمود;      /* Columns contained in the index */
  u16 n_عمود;              /* Number of columns in the index */
  Covering_اشاریہ_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  اگر( (p_اظہاریہ->op==TK_عمود || p_اظہاریہ->op==TK_AGG_عمود) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    اگر( p_اظہاریہ->i_جدول!=pCk->iTabCur ) واپس WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    ai_عمود = pIdx->ai_عمود;
    n_عمود = pIdx->n_عمود;
    برائے(i=0; i<n_عمود; i++){
      اگر( ai_عمود[i]==p_اظہاریہ->i_عمود ) واپس WRC_Continue;
    }
    pCk->bUnidx = 1;
    واپس WRC_Abort;
  }ورنہ اگر( pIdx->b_Has_اظہاریہ
         && اظہاریہ_Is_Covered_By_اشاریہ(p_اظہاریہ, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_اظہاریہ = 1;
    واپس WRC_Prune;
  }
  واپس WRC_Continue;
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
ساکن SQLITE_NOINLINE u32 where_Is_Covering_اشاریہ(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  اشاریہ *pIdx,           /* Index that is being tested */
  صحیح iTabCur            /* Cursor for the table being indexed */
){
  صحیح i, rc;
  ساخت Covering_اشاریہ_Check ck;
  Walker w;
  اگر( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    واپس 0;
  }
  اگر( pIdx->b_Has_اظہاریہ==0 ){
    برائے(i=0; i<pIdx->n_عمود; i++){
      اگر( pIdx->ai_عمود[i]>=BMS-1 ) توڑو;
    }
    اگر( i>=pIdx->n_عمود ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      واپس 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_اظہاریہ = 0;
  ck.bUnidx = 0;
  memset(&w, 0, حجم(w));
  w.x_اظہاریہ_Callback = where_Is_Covering_اشاریہ_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  اگر( ck.bUnidx ){
    rc = 0;
  }ورنہ اگر( ck.b_اظہاریہ ){
    rc = WHERE_EXPRIDX;
  }ورنہ{
    rc = WHERE_IDX_ONLY;
  }
  واپس rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
ساکن خالی where_اشاریہ_دار_اظہاریہ_Cleanup(sqlite3 *db, خالی *pObject){
  اشاریہ_دار_اظہاریہ **pp = (اشاریہ_دار_اظہاریہ**)pObject;
  جب( *pp!=0 ){
    اشاریہ_دار_اظہاریہ *p = *pp;
    *pp = p->pIENext;
    sqlite3_اظہاریہ_Delete(db, p->p_اظہاریہ);
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
ساکن خالی where_Part_Idx_اظہاریہ(
  Parse *pParse,                  /* Parse context */
  اشاریہ *pIdx,                    /* Partial index being processed */
  اظہاریہ *pPart,                    /* WHERE clause being processed */
  Bitmask *p_نقاب,                 /* Mask to clear bits in */
  صحیح iIdxCur,                    /* Cursor number for index */
  Src_شے *p_شے                  /* The FROM clause entry for the table */
){
  أكد( p_شے==0 || (p_شے->fg.jointype & JT_دایاں)==0 );
  أكد( (p_شے==0 || p_نقاب==0) && (p_نقاب!=0 || p_شے!=0) );

  اگر( pPart->op==TK_AND ){
    where_Part_Idx_اظہاریہ(pParse, pIdx, pPart->p_دایاں, p_نقاب, iIdxCur, p_شے);
    pPart = pPart->p_بایاں;
  }

  اگر( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    اظہاریہ *p_بایاں = pPart->p_بایاں;
    اظہاریہ *p_دایاں = pPart->p_دایاں;
    جدول *p_بایاں_Tab;            /* Table of the left operand */
    عمود *p_بایاں_ستون;           /* Column of the left operand */
    u8 aff_دایاں, aff_بایاں;

    اگر( p_بایاں->op!=TK_عمود ) واپس;
    اگر( !sqlite3_اظہاریہ_Is_Constant(0, p_دایاں) ) واپس;
    اگر( !sqlite3_Is_ثنائی(sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pPart)) ) واپس;
    اگر( p_بایاں->i_عمود<0 ) واپس;
    p_بایاں_Tab = pIdx->p_جدول;
    p_بایاں_ستون = &p_بایاں_Tab->a_ستون[p_بایاں->i_عمود];
    aff_بایاں = p_بایاں_ستون->affinity;
    أكد( SQLITE_AFF_BLOB<SQLITE_AFF_متن );
    اگر( aff_بایاں<SQLITE_AFF_متن ) واپس;
    aff_دایاں = sqlite3_اظہاریہ_Affinity(p_دایاں);
    اگر( aff_دایاں!=0 && aff_دایاں!=aff_بایاں ) واپس;
    اگر( p_شے ){
      sqlite3 *db = pParse->db;
      اشاریہ_دار_اظہاریہ *p = (اشاریہ_دار_اظہاریہ*)sqlite3DbMallocRaw(db, حجم(*p));
      اگر( p ){
        صحیح b_Null_صف = (p_شے->fg.jointype&(JT_بایاں|JT_LTORJ))!=0;
        p_دایاں = sqlite3_اظہاریہ_Dup(db, p_دایاں, 0);
        اگر( (p_بایاں_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3_عمود_Type(p_بایاں_ستون, 0);
          أكد( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (صحیح)لڑی_لمبائی(t.z);
          p->p_اظہاریہ = sqlite3_اظہاریہ_Alloc(db, TK_CAST, &t, 1);
          sqlite3_اظہاریہ_Attach_Subtrees(db, p->p_اظہاریہ, p_دایاں, 0);
        }ورنہ{
          p->p_اظہاریہ = p_دایاں;
        }
        p->i_ڈیٹا_Cur = p_شے->iCursor;
        p->iIdxCur = iIdxCur;
        p->i_Idx_ستون = p_بایاں->i_عمود;
        p->b_Maybe_Null_صف = b_Null_صف;
        p->pIENext = pParse->p_Idx_Part_اظہاریہ;
        p->aff = aff_بایاں;
        pParse->p_Idx_Part_اظہاریہ = p;
        اگر( p->pIENext==0 ){
          خالی *pArg = (خالی*)&pParse->p_Idx_Part_اظہاریہ;
          sqlite3_Parser_جوڑو_Cleanup(pParse, where_اشاریہ_دار_اظہاریہ_Cleanup, pArg);
        }
      }
    }ورنہ اگر( p_بایاں->i_عمود<(BMS-1) ){
      *p_نقاب &= ~((Bitmask)1 << p_بایاں->i_عمود);
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
ساکن صحیح where_Loop_جوڑو_Btree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  اشاریہ *pProbe;              /* An index we are evaluating */
  اشاریہ sPk;                  /* A fake index object for the primary key */
  LogEst ai_صف_Est_Pk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 ai_عمود_Pk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  Src_شے *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  صحیح rc = SQLITE_OK;         /* Return code */
  صحیح i_ترتیب_دو_Idx;               /* Index number */
  صحیح b;                      /* A boolean value */
  LogEst r_سائز;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  جدول *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  أكد( !IsVirtual(pSrc->pSTab) );

  اگر( pSrc->fg.is_اشاریہ_دار_By ){
    أكد( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }ورنہ اگر( !HasRowid(pTab) ){
    اگر( pSrc->fg.not_اشاریہ_دار ){
      pProbe = sqlite3_Primary_کلید_اشاریہ(pTab);
    }ورنہ{
      pProbe = pTab->p_اشاریہ;
    }
  }ورنہ{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    اشاریہ *p_پہلا;                  /* First of real indices on the table */
    memset(&sPk, 0, حجم(اشاریہ));
    sPk.n_کلید_ستون = 1;
    sPk.n_عمود = 1;
    sPk.ai_عمود = &ai_عمود_Pk;
    sPk.ai_صف_Log_Est = ai_صف_Est_Pk;
    sPk.onError = OE_Replace;
    sPk.p_جدول = pTab;
    sPk.sz_Idx_صف = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    ai_صف_Est_Pk[0] = pTab->n_صف_Log_Est;
    ai_صف_Est_Pk[1] = 0;
    p_پہلا = pSrc->pSTab->p_اشاریہ;
    sPk.p_اگلا = p_پہلا;
    pProbe = &sPk;
  }
  r_سائز = pTab->n_صف_Log_Est;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOMATIC_اشاریہ
  /* Automatic indexes */
  اگر( !pBuilder->p_Or_سیٹ      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_دایاں_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_اشاریہ)!=0
   && !pSrc->fg.is_اشاریہ_دار_By  /* Has no INDEXED BY clause */
   && !pSrc->fg.not_اشاریہ_دار   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_دایاں)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_سائز;         /* Logarithm of the number of rows in the table */
    Where_رکن *p_رکن;
    Where_رکن *pWCEnd = pWC->a + pWC->n_رکن;
    r_Log_سائز = estLog(r_سائز);
    برائے(p_رکن=pWC->a; rc==SQLITE_OK && p_رکن<pWCEnd; p_رکن++){
      اگر( p_رکن->prereq_دایاں & pNew->نقاب_Self ) جاری;
      اگر( رکن_Can_Drive_اشاریہ(p_رکن, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_اشاریہ = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = p_رکن;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->r_تیاری = r_Log_سائز + r_سائز;
        اگر( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->r_تیاری += 28;
        }ورنہ{
          pNew->r_تیاری -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        Apply_لاگت_Multiplier(pNew->r_تیاری, pTab->لاگت_Mult);
        اگر( pNew->r_تیاری<0 ) pNew->r_تیاری = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->n_برآمد = 43;  أكد( 43==sqlite3LogEst(20) );
        pNew->r_تسلسل = sqlite3_Log_Est_جوڑو(r_Log_سائز,pNew->n_برآمد);
        pNew->wsFlags = WHERE_AUTO_اشاریہ;
        pNew->prereq = mPrereq | p_رکن->prereq_دایاں;
        rc = where_Loop_داخل_کرو(pBuilder, pNew);
      }
    }
  }
#اگر_ختم /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  برائے(i_ترتیب_دو_Idx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.is_اشاریہ_دار_By||pSrc->fg.not_اشاریہ_دار)?0:pProbe->p_اگلا,
         i_ترتیب_دو_Idx++
  ){
    اگر( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_اشاریہ(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      جاری;  /* Partial index inappropriate for this query */
    }
    اگر( pProbe->bNoQuery ) جاری;
    r_سائز = pProbe->ai_صف_Log_Est[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_چوٹی = 0;
    pNew->u.btree.n_Distinct_ستون = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->i_ترتیب_دو_Idx = 0;
    pNew->r_تیاری = 0;
    pNew->prereq = mPrereq;
    pNew->n_برآمد = r_سائز;
    pNew->u.btree.p_اشاریہ = pProbe;
    pNew->u.btree.p_ترتیب_By = 0;
    b = اشاریہ_Might_Help_With_ترتیب_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    أكد( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    اگر( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->i_ترتیب_دو_Idx = b ? i_ترتیب_دو_Idx : 0;
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
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
      pNew->r_تسلسل = r_سائز + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#ورنہ
      pNew->r_تسلسل = r_سائز + 16;
#اگر_ختم
      Apply_لاگت_Multiplier(pNew->r_تسلسل, pTab->لاگت_Mult);
      whereLoopOutputAdjust(pWC, pNew, r_سائز);
      اگر( pSrc->fg.isSubquery ){
        اگر( pSrc->fg.بذریعہ_Coroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        اگر( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.p_ترتیب_By = pSrc->u4.pSubq->pSelect->p_ترتیب_By;
        }
      }ورنہ اگر( pSrc->fg.fromExists ){
        pNew->n_برآمد = 0;
      }
      rc = where_Loop_داخل_کرو(pBuilder, pNew);
      pNew->n_برآمد = r_سائز;
      اگر( rc ) توڑو;
    }ورنہ{
      Bitmask m;
      اگر( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_اشاریہ_دار;
      }ورنہ{
        m = pSrc->ستون_مستعمل & pProbe->ستون_Not_Idxed;
        اگر( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_اظہاریہ(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_اشاریہ_دار;
        اگر( m==TOPBIT || (pProbe->b_Has_اظہاریہ && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_اشاریہ(pWInfo, pProbe, pSrc->iCursor);
          اگر( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            أكد( m!=0 );
          }ورنہ{
            m = 0;
            pNew->wsFlags |= isCov;
            اگر( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }ورنہ{
              أكد( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }ورنہ اگر( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_اشاریہ_دار;
        }
      }

      /* Full scan via index */
      اگر( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.is_اشاریہ_دار_By
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->sz_Idx_صف<pTab->sz_Tab_صف)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_Cover_Idx_جائزہ)
          )
      ){
        pNew->i_ترتیب_دو_Idx = b ? i_ترتیب_دو_Idx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->r_تسلسل = r_سائز + 1 + (15*pProbe->sz_Idx_صف)/pTab->sz_Tab_صف;
        اگر( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_سائز + 16;  /* Base cost:  N*3 */
          صحیح ii;
          صحیح iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          برائے(ii=0; ii<pWC2->n_رکن; ii++){
            Where_رکن *p_رکن = &pWC2->a[ii];
            اگر( !sqlite3_اظہاریہ_Covered_By_اشاریہ(p_رکن->p_اظہاریہ, iCur, pProbe) ){
              توڑو;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            اگر( p_رکن->truthProb<=0 ){
              nLookup += p_رکن->truthProb;
            }ورنہ{
              nLookup--;
              اگر( p_رکن->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->r_تسلسل = sqlite3_Log_Est_جوڑو(pNew->r_تسلسل, nLookup);
        }
        Apply_لاگت_Multiplier(pNew->r_تسلسل, pTab->لاگت_Mult);
        whereLoopOutputAdjust(pWC, pNew, r_سائز);
        اگر( (pSrc->fg.jointype & JT_دایاں)!=0 && pProbe->a_ستون_اظہاریہ ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }ورنہ{
          اگر( pSrc->fg.fromExists ) pNew->n_برآمد = 0;
          rc = where_Loop_داخل_کرو(pBuilder, pNew);
        }
        pNew->n_برآمد = r_سائز;
        اگر( rc ) توڑو;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_جوڑو_Btree_اشاریہ(pBuilder, pSrc, pProbe, 0);
    اگر( pBuilder->bldFlags1==SQLITE_BLDF1_اشاریہ_دار ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#اگر_ختم
  }
  واپس rc;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
ساکن صحیح is_حد_بندی_رکن(Where_رکن *p_رکن){
  أكد( p_رکن->eOperator==WO_AUX || p_رکن->e_مطابقت_Op==0 );
  واپس p_رکن->e_مطابقت_Op>=SQLITE_اشاریہ_CONSTRAINT_حد_بندی
      && p_رکن->e_مطابقت_Op<=SQLITE_اشاریہ_CONSTRAINT_انحراف;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
ساکن صحیح all_Constraints_مستعمل(
  ساخت sqlite3_اشاریہ_constraint_usage *aUsage, 
  صحیح nCons
){
  صحیح ii;
  برائے(ii=0; ii<nCons; ii++){
    اگر( aUsage[ii].argv_اشاریہ<=0 ) واپس 0;
  }
  واپس 1;
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
ساکن صحیح where_Loop_جوڑو_Virtual_One(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_اشاریہ_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  صحیح *pb_اندر,                      /* OUT: True if plan uses an IN(...) op */
  صحیح *pb_Retry_حد_بندی               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_اشاریہ_Info *pHidden = (Hidden_اشاریہ_Info*)&pIdxInfo[1];
  ساخت sqlite3_اشاریہ_constraint *pIdxCons;
  ساخت sqlite3_اشاریہ_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  صحیح i;
  صحیح mx_رکن;
  صحیح rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  Src_شے *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  صحیح nConstraint = pIdxInfo->nConstraint;

  أكد( (mUsable & mPrereq)==mPrereq );
  *pb_اندر = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(ساخت sqlite3_اشاریہ_constraint**)&pIdxInfo->aConstraint;
  برائے(i=0; i<nConstraint; i++, pIdxCons++){
    Where_رکن *p_رکن = رکن_From_Where_Clause(pWC, pIdxCons->i_رکن_انحراف);
    pIdxCons->usable = 0;
    اگر( (p_رکن->prereq_دایاں & mUsable)==p_رکن->prereq_دایاں
     && (p_رکن->eOperator & mExclude)==0
     && (pb_Retry_حد_بندی || !is_حد_بندی_رکن(p_رکن))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, حجم(pUsage[0])*nConstraint);
  أكد( pIdxInfo->need_مقصد_Free_Idx_Str==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_عدد = 0;
  pIdxInfo->ترتیب_By_Consumed = 0;
  pIdxInfo->estimated_لاگت = SQLITE_BIG_DBL / (دوہرا)2;
  pIdxInfo->estimated_صفیں = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->m_Handle_اندر = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_بہترین_اشاریہ(pParse, pSrc->pSTab, pIdxInfo);
  اگر( rc ){
    اگر( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      واپس SQLITE_OK;
    }
    واپس rc;
  }

  mx_رکن = -1;
  أكد( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, حجم(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, حجم(pNew->u.vtab));
  pIdxCons = *(ساخت sqlite3_اشاریہ_constraint**)&pIdxInfo->aConstraint;
  برائے(i=0; i<nConstraint; i++, pIdxCons++){
    صحیح i_رکن;
    اگر( (i_رکن = pUsage[i].argv_اشاریہ - 1)>=0 ){
      Where_رکن *p_رکن;
      صحیح j = pIdxCons->i_رکن_انحراف;
      اگر( i_رکن>=nConstraint
       || j<0
       || (p_رکن = رکن_From_Where_Clause(pWC, j))==0
       || pNew->aLTerm[i_رکن]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_پیغام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        واپس SQLITE_ERROR;
      }
      testcase( i_رکن==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->n_رکن-1 );
      pNew->prereq |= p_رکن->prereq_دایاں;
      أكد( i_رکن<pNew->nLSlot );
      pNew->aLTerm[i_رکن] = p_رکن;
      اگر( i_رکن>mx_رکن ) mx_رکن = i_رکن;
      testcase( i_رکن==15 );
      testcase( i_رکن==16 );
      اگر( pUsage[i].omit ){
        اگر( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=i_رکن );
          pNew->u.vtab.omit_نقاب |= 1<<i_رکن;
        }ورنہ{
          testcase( i!=i_رکن );
        }
        اگر( p_رکن->e_مطابقت_Op==SQLITE_اشاریہ_CONSTRAINT_انحراف ){
          pNew->u.vtab.b_Omit_انحراف = 1;
        }
      }
      اگر( SMASKBIT32(i) & pHidden->m_Handle_اندر ){
        pNew->u.vtab.m_Handle_اندر |= MASKBIT32(i_رکن);
      }ورنہ اگر( (p_رکن->eOperator & WO_اندر)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->ترتیب_By_Consumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_اشاریہ_جائزہ_UNIQUE;
        *pb_اندر = 1; أكد( (mExclude & WO_اندر)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      أكد( pb_Retry_حد_بندی || !is_حد_بندی_رکن(p_رکن) );
      أكد( !is_حد_بندی_رکن(p_رکن) || i>=nConstraint-2 );
      أكد( !is_حد_بندی_رکن(p_رکن) || i==nConstraint-1 || is_حد_بندی_رکن(p_رکن+1) );

      اگر( is_حد_بندی_رکن(p_رکن) && (*pb_اندر || !all_Constraints_مستعمل(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pb_Retry_حد_بندی = 1;
        واپس SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mx_رکن+1;
  برائے(i=0; i<=mx_رکن; i++){
    اگر( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_پیغام(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      واپس SQLITE_ERROR;
    }
  }
  أكد( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_عدد = pIdxInfo->idx_عدد;
  pNew->u.vtab.needFree = pIdxInfo->need_مقصد_Free_Idx_Str;
  pIdxInfo->need_مقصد_Free_Idx_Str = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->ترتیب_By_Consumed ?
      pIdxInfo->n_ترتیب_By : 0);
  pNew->u.vtab.b_Idx_عدد_Hex = (pIdxInfo->idxFlags&SQLITE_اشاریہ_جائزہ_HEX)!=0;
  pNew->r_تیاری = 0;
  pNew->r_تسلسل = sqlite3LogEstFromDouble(pIdxInfo->estimated_لاگت);
  pNew->n_برآمد = sqlite3LogEst(pIdxInfo->estimated_صفیں);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  اگر( pIdxInfo->idxFlags & SQLITE_اشاریہ_جائزہ_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }ورنہ{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_داخل_کرو(pBuilder, pNew);
  اگر( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pb_اندر, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  واپس rc;
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
ثابت حرف *sqlite3_vtab_collation(sqlite3_اشاریہ_info *pIdxInfo, صحیح iCons){
  Hidden_اشاریہ_Info *pHidden = (Hidden_اشاریہ_Info*)&pIdxInfo[1];
  ثابت حرف *zRet = 0;
  اگر( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    صحیح i_رکن = pIdxInfo->aConstraint[iCons].i_رکن_انحراف;
    اظہاریہ *pX = رکن_From_Where_Clause(pHidden->pWC, i_رکن)->p_اظہاریہ;
    اگر( pX->p_بایاں ){
      pC = sqlite3_اظہاریہ_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_ثنائی);
  }
  واپس zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
صحیح sqlite3_vtab_اندر(sqlite3_اشاریہ_info *pIdxInfo, صحیح iCons, صحیح bHandle){
  Hidden_اشاریہ_Info *pHidden = (Hidden_اشاریہ_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  اگر( m & pHidden->m_اندر ){
    اگر( bHandle==0 ){
      pHidden->m_Handle_اندر &= ~m;
    }ورنہ اگر( bHandle>0 ){
      pHidden->m_Handle_اندر |= m;
    }
    واپس 1;
  }
  واپس 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
صحیح sqlite3_vtab_rhs_قدر(
  sqlite3_اشاریہ_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  صحیح iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_قدر **ppVal           /* Write value extracted here */
){
  Hidden_اشاریہ_Info *pH = (Hidden_اشاریہ_Info*)&pIdxInfo[1];
  sqlite3_قدر *pVal = 0;
  صحیح rc = SQLITE_OK;
  اگر( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }ورنہ{
    اگر( pH->aRhs[iCons]==0 ){
      Where_رکن *p_رکن = رکن_From_Where_Clause(
          pH->pWC, pIdxInfo->aConstraint[iCons].i_رکن_انحراف
      );
      rc = sqlite3_قدر_From_اظہاریہ(
          pH->pParse->db, p_رکن->p_اظہاریہ->p_دایاں, ENC(pH->pParse->db),
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

  واپس rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
صحیح sqlite3_vtab_distinct(sqlite3_اشاریہ_info *pIdxInfo){
  Hidden_اشاریہ_Info *pHidden = (Hidden_اشاریہ_Info*)&pIdxInfo[1];
  أكد( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  واپس pHidden->eDistinct;
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
خالی sqlite3_Vtab_استعمال_کرتا_All_Schemas(Parse *pParse){
  صحیح nDb = pParse->db->nDb;
  صحیح i;
  برائے(i=0; i<nDb; i++){
    sqlite3_رمز_Verify_Schema(pParse, i);
  }
  اگر( Db_نقاب_Non_Zero(pParse->write_نقاب) ){
    برائے(i=0; i<nDb; i++){
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
ساکن صحیح where_Loop_جوڑو_Virtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  صحیح rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  Src_شے *pSrc;               /* The FROM clause term to search */
  sqlite3_اشاریہ_info *p;       /* Object to pass to xBestIndex() */
  صحیح nConstraint;             /* Number of constraints in p */
  صحیح b_اندر;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask m_بہترین;               /* Tables used by best possible plan */
  u16 mNoOmit;
  صحیح bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  أكد( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  أكد( IsVirtual(pSrc->pSTab) );
  p = allocate_اشاریہ_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
  pNew->r_تیاری = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  اگر( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_اشاریہ_Info(pParse->db, p);
    واپس SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = where_Loop_جوڑو_Virtual_One(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_اندر, &bRetry
  );
  اگر( bRetry ){
    أكد( rc==SQLITE_OK );
    rc = where_Loop_جوڑو_Virtual_One(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_اندر, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  اگر( rc==SQLITE_OK && ((m_بہترین = (pNew->prereq & ~mPrereq))!=0 || b_اندر) ){
    صحیح نظر_آیا_Zero = 0;             /* True if a plan with no prereqs seen */
    صحیح نظر_آیا_Zero_No_اندر = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask m_پچھلا = 0;
    Bitmask m_بہترین_No_اندر = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    اگر( b_اندر ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = where_Loop_جوڑو_Virtual_One(
          pBuilder, mPrereq, ALLBITS, WO_اندر, p, mNoOmit, &b_اندر, 0);
      أكد( b_اندر==0 );
      m_بہترین_No_اندر = pNew->prereq & ~mPrereq;
      اگر( m_بہترین_No_اندر==0 ){
        نظر_آیا_Zero = 1;
        نظر_آیا_Zero_No_اندر = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    جب( rc==SQLITE_OK ){
      صحیح i;
      Bitmask m_اگلا = ALLBITS;
      أكد( m_اگلا>0 );
      برائے(i=0; i<nConstraint; i++){
        صحیح i_رکن = p->aConstraint[i].i_رکن_انحراف;
        Bitmask mThis = رکن_From_Where_Clause(pWC, i_رکن)->prereq_دایاں & ~mPrereq;
        اگر( mThis>m_پچھلا && mThis<m_اگلا ) m_اگلا = mThis;
      }
      m_پچھلا = m_اگلا;
      اگر( m_اگلا==ALLBITS ) توڑو;
      اگر( m_اگلا==m_بہترین || m_اگلا==m_بہترین_No_اندر ) جاری;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)m_پچھلا, (sqlite3_uint64)m_اگلا));
      rc = where_Loop_جوڑو_Virtual_One(
          pBuilder, mPrereq, m_اگلا|mPrereq, 0, p, mNoOmit, &b_اندر, 0);
      اگر( pNew->prereq==mPrereq ){
        نظر_آیا_Zero = 1;
        اگر( b_اندر==0 ) نظر_آیا_Zero_No_اندر = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    اگر( rc==SQLITE_OK && نظر_آیا_Zero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = where_Loop_جوڑو_Virtual_One(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &b_اندر, 0);
      اگر( b_اندر==0 ) نظر_آیا_Zero_No_اندر = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    اگر( rc==SQLITE_OK && نظر_آیا_Zero_No_اندر==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = where_Loop_جوڑو_Virtual_One(
          pBuilder, mPrereq, mPrereq, WO_اندر, p, mNoOmit, &b_اندر, 0);
    }
  }

  free_اشاریہ_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
ساکن صحیح where_Loop_جوڑو_Or(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  Where_رکن *p_رکن, *pWCEnd;
  صحیح rc = SQLITE_OK;
  صحیح iCur;
  WhereClause عارضی_WC;
  WhereLoopBuilder sSubBuild;
  Where_Or_سیٹ s_مجموع, sCur;
  Src_شے *p_شے;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->n_رکن;
  pNew = pBuilder->pNew;
  memset(&s_مجموع, 0, حجم(s_مجموع));
  p_شے = pWInfo->pTabList->a + pNew->iTab;
  iCur = p_شے->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  اگر( p_شے->fg.jointype & JT_دایاں ) واپس SQLITE_OK;

  برائے(p_رکن=pWC->a; p_رکن<pWCEnd && rc==SQLITE_OK; p_رکن++){
    اگر( (p_رکن->eOperator & WO_OR)!=0
     && (p_رکن->u.pOrInfo->indexable & pNew->نقاب_Self)!=0
     && (!اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_OuterON)
          || p_رکن->p_اظہاریہ->w.iJoin==iCur)
    ){
      WhereClause * ثابت pOrWC = &p_رکن->u.pOrInfo->wc;
      Where_رکن * ثابت pOrWCEnd = &pOrWC->a[pOrWC->n_رکن];
      Where_رکن *p_Or_رکن;
      صحیح once = 1;
      صحیح i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.p_Or_سیٹ = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", p_رکن));
      برائے(p_Or_رکن=pOrWC->a; p_Or_رکن<pOrWCEnd; p_Or_رکن++){
        اگر( (p_Or_رکن->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &p_Or_رکن->u.pAndInfo->wc;
        }ورنہ اگر( p_Or_رکن->بایاں_Cursor==iCur ){
          عارضی_WC.pWInfo = pWC->pWInfo;
          عارضی_WC.pOuter = pWC;
          عارضی_WC.op = TK_AND;
          عارضی_WC.n_رکن = 1;
          عارضی_WC.n_بنیاد = 1;
          عارضی_WC.a = p_Or_رکن;
          sSubBuild.pWC = &عارضی_WC;
        }ورنہ{
          جاری;
        }
        sCur.n = 0;
#اگر_تعریف_شدہ WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (صحیح)(p_Or_رکن-pOrWC->a), p_رکن, sSubBuild.pWC->n_رکن));
        اگر( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
        اگر( IsVirtual(p_شے->pSTab) ){
          rc = where_Loop_جوڑو_Virtual(&sSubBuild, mPrereq, mUnusable);
        }ورنہ
#اگر_ختم
        {
          rc = where_Loop_جوڑو_Btree(&sSubBuild, mPrereq);
        }
        اگر( rc==SQLITE_OK ){
          rc = where_Loop_جوڑو_Or(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_مکمل );
        اگر( sCur.n==0 ){
          s_مجموع.n = 0;
          توڑو;
        }ورنہ اگر( once ){
          whereOrMove(&s_مجموع, &sCur);
          once = 0;
        }ورنہ{
          Where_Or_سیٹ s_پچھلا;
          whereOrMove(&s_پچھلا, &s_مجموع);
          s_مجموع.n = 0;
          برائے(i=0; i<s_پچھلا.n; i++){
            برائے(j=0; j<sCur.n; j++){
              where_Or_داخل_کرو(&s_مجموع, s_پچھلا.a[i].prereq | sCur.a[j].prereq,
                            sqlite3_Log_Est_جوڑو(s_پچھلا.a[i].r_تسلسل, sCur.a[j].r_تسلسل),
                            sqlite3_Log_Est_جوڑو(s_پچھلا.a[i].n_برآمد, sCur.a[j].n_برآمد));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = p_رکن;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->r_تیاری = 0;
      pNew->i_ترتیب_دو_Idx = 0;
      memset(&pNew->u, 0, حجم(pNew->u));
      برائے(i=0; rc==SQLITE_OK && i<s_مجموع.n; i++){
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
        pNew->r_تسلسل = s_مجموع.a[i].r_تسلسل + 1;
        pNew->n_برآمد = s_مجموع.a[i].n_برآمد;
        pNew->prereq = s_مجموع.a[i].prereq;
        rc = where_Loop_داخل_کرو(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", p_رکن));
    }
  }
  واپس rc;
}

/*
** Add all WhereLoop objects for all tables
*/
ساکن صحیح where_Loop_جوڑو_All(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask m_پیشگی = 0;
  صحیح iTab;
  SrcList *pTabList = pWInfo->pTabList;
  Src_شے *p_شے;
  Src_شے *p_انتہا = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  صحیح rc = SQLITE_OK;
  صحیح b_پہلا_Past_RJ = 0;
  صحیح has_دایاں_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  أكد( pNew->nLTerm==0 );
  أكد( pNew->wsFlags==0 );
  أكد( pNew->nLSlot>=Array_سائز(pNew->aLTermSpace) );
  أكد( pNew->aLTerm!=0 );

  pBuilder->i_Plan_حد_بندی = SQLITE_QUERY_PLANNER_حد_بندی;
  برائے(iTab=0, p_شے=pTabList->a; p_شے<p_انتہا; iTab++, p_شے++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->i_Plan_حد_بندی += SQLITE_QUERY_PLANNER_حد_بندی_INCR;
    pNew->نقاب_Self = sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, p_شے->iCursor);
    اگر( b_پہلا_Past_RJ
     || (p_شے->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
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
      اگر( p_شے->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( p_شے->fg.jointype & JT_LTORJ );
        testcase( p_شے->fg.jointype & JT_CROSS );
        has_دایاں_Cross_Join = 1;
      }
      mPrereq |= m_پیشگی;
      b_پہلا_Past_RJ = (p_شے->fg.jointype & JT_دایاں)!=0;
    }ورنہ اگر( p_شے->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      Where_رکن *p_رکن;
      صحیح i;
      برائے(i=pWC->n_بنیاد, p_رکن=pWC->a; i>0; i--, p_رکن++){
        اگر( (pNew->نقاب_Self & p_رکن->prereqAll)!=0 ){
          mPrereq |= (p_رکن->prereqAll & (pNew->نقاب_Self-1));
        }
      }
    }ورنہ اگر( !has_دایاں_Cross_Join ){
      mPrereq = 0;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
    اگر( IsVirtual(p_شے->pSTab) ){
      Src_شے *p;
      برائے(p=&p_شے[1]; p<p_انتہا; p++){
        اگر( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, p->iCursor);
        }
      }
      rc = where_Loop_جوڑو_Virtual(pBuilder, mPrereq, mUnusable);
    }ورنہ
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = where_Loop_جوڑو_Btree(pBuilder, mPrereq);
    }
    اگر( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = where_Loop_جوڑو_Or(pBuilder, mPrereq, mUnusable);
    }
    m_پیشگی |= pNew->نقاب_Self;
    اگر( rc || db->mallocFailed ){
      اگر( rc==SQLITE_مکمل ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }ورنہ{
        توڑو;
      }
    }
  }

  whereLoopClear(db, pNew);
  واپس rc;
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
ساکن SQLITE_NOINLINE صحیح where_راستہ_مطابقت_Subquery_OB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  صحیح iLoop,              /* Which level of the nested loop.  0==outermost */
  صحیح iCur,               /* Cursor used by the this loop */
  اظہاریہ_List *p_ترتیب_By,     /* The ORDER BY clause on the whole query */
  Bitmask *p_Rev_نقاب,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  صحیح iOB;                /* Index into pOrderBy->a[] */
  صحیح jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  اظہاریہ *pOBExpr;          /* Current term of outer ORDER BY */
  اظہاریہ_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.p_ترتیب_By;
  أكد( pSubOB!=0 );
  برائے(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  برائے(jSub=0; jSub<pSubOB->n_اظہاریہ && iOB<p_ترتیب_By->n_اظہاریہ; jSub++, iOB++){
    اگر( pSubOB->a[jSub].u.x.i_ترتیب_By_ستون==0 ) توڑو;
    pOBExpr = p_ترتیب_By->a[iOB].p_اظہاریہ;
    اگر( pOBExpr->op!=TK_عمود && pOBExpr->op!=TK_AGG_عمود ) توڑو;
    اگر( pOBExpr->i_جدول!=iCur ) توڑو;
    اگر( pOBExpr->i_عمود!=pSubOB->a[jSub].u.x.i_ترتیب_By_ستون-1 ) توڑو;
    اگر( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = p_ترتیب_By->a[iOB].fg.ترتیب_دو_Flags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.ترتیب_دو_Flags;   /* sortFlags for jSub */
      اگر( (sfSub & KEYINFO_ترتیب_BIGNULL) != (sfOB & KEYINFO_ترتیب_BIGNULL) ){
        توڑو;
      }
      revIdx = sfSub & KEYINFO_ترتیب_DESC;
      اگر( jSub>0 ){
        اگر( (rev^revIdx)!=(sfOB & KEYINFO_ترتیب_DESC) ){
          توڑو;
        }
      }ورنہ{
        rev = revIdx ^ (sfOB & KEYINFO_ترتیب_DESC);
        اگر( rev ){
          اگر( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            توڑو;
          }
          *p_Rev_نقاب |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  واپس jSub>0;
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
ساکن i8 where_راستہ_Satisfies_ترتیب_By(
  WhereInfo *pWInfo,    /* The WHERE clause */
  اظہاریہ_List *p_ترتیب_By,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  Where_راستہ *p_راستہ,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *p_آخری,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *p_Rev_نقاب     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 rev_سیٹ;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 is_ترتیب_Distinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 is_مطابقت;           /* iColumn matches a term of the ORDER BY clause */
  u16 eq_Op_نقاب;         /* Allowed equality operators */
  u16 n_کلید_ستون;          /* Number of key columns in pIndex */
  u16 n_عمود;          /* Total number of ordered columns in the index */
  u16 n_ترتیب_By;         /* Number terms in the ORDER BY clause */
  صحیح iLoop;            /* Index of WhereLoop in pPath being processed */
  صحیح i, j;             /* Loop counters */
  صحیح iCur;             /* Cursor number for current WhereLoop */
  صحیح i_عمود;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  Where_رکن *p_رکن;     /* A single term of the WHERE clause */
  اظہاریہ *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  اشاریہ *p_اشاریہ;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask ob_مکمل;       /* Mask of all ORDER BY terms */
  Bitmask ترتیب_Distinct_نقاب;  /* Mask of all well-ordered loops */
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
  اگر( nLoop && OptimizationDisabled(db, SQLITE_ترتیب_By_Idx_Join) ) واپس 0;

  n_ترتیب_By = p_ترتیب_By->n_اظہاریہ;
  testcase( n_ترتیب_By==BMS-1 );
  اگر( n_ترتیب_By>BMS-1 ) واپس 0;  /* Cannot optimize overly large ORDER BYs */
  is_ترتیب_Distinct = 1;
  ob_مکمل = MASKBIT(n_ترتیب_By)-1;
  ترتیب_Distinct_نقاب = 0;
  ready = 0;
  eq_Op_نقاب = WO_EQ | WO_IS | WO_ISNULL;
  اگر( wctrlFlags & (WHERE_ORDERBY_حد_بندی|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eq_Op_نقاب |= WO_اندر;
  }
  برائے(iLoop=0; is_ترتیب_Distinct && obSat<ob_مکمل && iLoop<=nLoop; iLoop++){
    اگر( iLoop>0 ) ready |= pLoop->نقاب_Self;
    اگر( iLoop<nLoop ){
      pLoop = p_راستہ->aLoop[iLoop];
      اگر( wctrlFlags & WHERE_ORDERBY_حد_بندی ) جاری;
    }ورنہ{
      pLoop = p_آخری;
    }
    اگر( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      اگر( pLoop->u.vtab.isOrdered && pWInfo->p_ترتیب_By==p_ترتیب_By ){
        obSat = ob_مکمل;
      }ورنہ{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        is_ترتیب_Distinct = 0;
      }
      توڑو;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    برائے(i=0; i<n_ترتیب_By; i++){
      اگر( MASKBIT(i) & obSat ) جاری;
      pOBExpr = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_ترتیب_By->a[i].p_اظہاریہ);
      اگر( NEVER(pOBExpr==0) ) جاری;
      اگر( pOBExpr->op!=TK_عمود && pOBExpr->op!=TK_AGG_عمود ) جاری;
      اگر( pOBExpr->i_جدول!=iCur ) جاری;
      p_رکن = sqlite3_Where_ڈھونڈو_رکن(&pWInfo->sWC, iCur, pOBExpr->i_عمود,
                       ~ready, eq_Op_نقاب, 0);
      اگر( p_رکن==0 ) جاری;
      اگر( p_رکن->eOperator==WO_اندر ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        أكد( wctrlFlags &
               (WHERE_ORDERBY_حد_بندی|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        برائے(j=0; j<pLoop->nLTerm && p_رکن!=pLoop->aLTerm[j]; j++){}
        اگر( j>=pLoop->nLTerm ) جاری;
      }
      اگر( (p_رکن->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->i_عمود>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_اظہاریہ_NNColl_Seq(pParse, p_ترتیب_By->a[i].p_اظہاریہ);
        ثابت اظہاریہ *pCExpr = p_رکن->p_اظہاریہ;
        CollSeq *pColl2 = sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pCExpr);
        حرف aff_دایاں;
        أكد( pColl1 );
        اگر( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          جاری;
        }
        aff_دایاں = sqlite3_اظہاریہ_Affinity(pCExpr->p_دایاں);
        /* Left operand must be a column, hence always has affinity */
        أكد( sqlite3_اظہاریہ_Affinity(pCExpr->p_بایاں)!=0 );
        اگر( aff_دایاں!=0 && aff_دایاں!=sqlite3_اظہاریہ_Affinity(pCExpr->p_بایاں) ){
          جاری;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    اگر( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      اگر( pLoop->wsFlags & WHERE_IPK ){
        اگر( pLoop->u.btree.p_ترتیب_By
         && OptimizationEnabled(db, SQLITE_ترتیب_By_Subq)
         &&  where_راستہ_مطابقت_Subquery_OB(pLoop,iLoop,iCur,
                                     p_ترتیب_By,p_Rev_نقاب, &obSat, wctrlFlags)
        ){
          n_عمود = 0;
          is_ترتیب_Distinct = 0;
        }ورنہ{
          n_عمود = 1;
        }
        p_اشاریہ = 0;
        n_کلید_ستون = 1;
      }ورنہ اگر( (p_اشاریہ = pLoop->u.btree.p_اشاریہ)==0 || p_اشاریہ->bUnordered ){
        واپس 0;
      }ورنہ{
        n_کلید_ستون = p_اشاریہ->n_کلید_ستون;
        n_عمود = p_اشاریہ->n_عمود;
        أكد( n_عمود==n_کلید_ستون+1 || !HasRowid(p_اشاریہ->p_جدول) );
        أكد( p_اشاریہ->ai_عمود[n_عمود-1]==XN_ROWID
                          || !HasRowid(p_اشاریہ->p_جدول));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        is_ترتیب_Distinct = Is_Unique_اشاریہ(p_اشاریہ)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = rev_سیٹ = 0;
      distinctColumns = 0;
      برائے(j=0; j<n_عمود; j++){
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
          اگر( (eOp & eq_Op_نقاب)!=0 ){
            اگر( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( is_ترتیب_Distinct );
              is_ترتیب_Distinct = 0;
            }
            جاری; 
          }ورنہ اگر( ALWAYS(eOp & WO_اندر) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            اظہاریہ *pX = pLoop->aLTerm[j]->p_اظہاریہ;
            برائے(i=j+1; i<pLoop->u.btree.nEq; i++){
              اگر( pLoop->aLTerm[i]->p_اظہاریہ==pX ){
                أكد( (pLoop->aLTerm[i]->eOperator & WO_اندر) );
                bOnce = 0;
                توڑو;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        اگر( p_اشاریہ ){
          i_عمود = p_اشاریہ->ai_عمود[j];
          revIdx = p_اشاریہ->a_ترتیب_دو_ترتیب[j] & KEYINFO_ترتیب_DESC;
          اگر( i_عمود==p_اشاریہ->p_جدول->iPKey ) i_عمود = XN_ROWID;
        }ورنہ{
          i_عمود = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        اگر( is_ترتیب_Distinct ){
          اگر( i_عمود>=0
           && j>=pLoop->u.btree.nEq
           && p_اشاریہ->p_جدول->a_ستون[i_عمود].notNull==0
          ){
            is_ترتیب_Distinct = 0;
          }
          اگر( i_عمود==XN_اظہاریہ ){
            is_ترتیب_Distinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        is_مطابقت = 0;
        برائے(i=0; bOnce && i<n_ترتیب_By; i++){
          اگر( MASKBIT(i) & obSat ) جاری;
          pOBExpr = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_ترتیب_By->a[i].p_اظہاریہ);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          اگر( NEVER(pOBExpr==0) ) جاری;
          اگر( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          اگر( i_عمود>=XN_ROWID ){
            اگر( pOBExpr->op!=TK_عمود && pOBExpr->op!=TK_AGG_عمود ) جاری;
            اگر( pOBExpr->i_جدول!=iCur ) جاری;
            اگر( pOBExpr->i_عمود!=i_عمود ) جاری;
          }ورنہ{
            اظہاریہ *p_Ix_اظہاریہ = p_اشاریہ->a_ستون_اظہاریہ->a[j].p_اظہاریہ;
            اگر( sqlite3_اظہاریہ_Compare_Skip(pOBExpr, p_Ix_اظہاریہ, iCur) ){
              جاری;
            }
          }
          اگر( i_عمود!=XN_ROWID ){
            pColl = sqlite3_اظہاریہ_NNColl_Seq(pWInfo->pParse, p_ترتیب_By->a[i].p_اظہاریہ);
            اگر( sqlite3StrICmp(pColl->zName, p_اشاریہ->azColl[j])!=0 ) جاری;
          }
          اگر( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.n_Distinct_ستون = j+1;
          }
          is_مطابقت = 1;
          توڑو;
        }
        اگر( is_مطابقت && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          اگر( rev_سیٹ ){
            اگر( (rev ^ revIdx)
                           != (p_ترتیب_By->a[i].fg.ترتیب_دو_Flags&KEYINFO_ترتیب_DESC)
            ){
              is_مطابقت = 0;
            }
          }ورنہ{
            rev = revIdx ^ (p_ترتیب_By->a[i].fg.ترتیب_دو_Flags & KEYINFO_ترتیب_DESC);
            اگر( rev ) *p_Rev_نقاب |= MASKBIT(iLoop);
            rev_سیٹ = 1;
          }
        }
        اگر( is_مطابقت && (p_ترتیب_By->a[i].fg.ترتیب_دو_Flags & KEYINFO_ترتیب_BIGNULL) ){
          اگر( j==pLoop->u.btree.nEq 
           && (j<n_کلید_ستون||HasRowid(p_اشاریہ->p_جدول)||!Is_Primary_کلید_اشاریہ(p_اشاریہ))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_ترتیب_دو;
          }ورنہ{
            is_مطابقت = 0;
          }
        }
        اگر( is_مطابقت ){
          اگر( i_عمود==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }ورنہ{
          /* No match found */
          أكد( n_کلید_ستون>0 );
          اگر( j<n_کلید_ستون ){
            testcase( is_ترتیب_Distinct!=0 );
            is_ترتیب_Distinct = 0;
          }
          توڑو;
        }
      } /* end Loop over all index columns */
      اگر( distinctColumns ){
        testcase( is_ترتیب_Distinct==0 );
        is_ترتیب_Distinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    اگر( is_ترتیب_Distinct ){
      ترتیب_Distinct_نقاب |= pLoop->نقاب_Self;
      برائے(i=0; i<n_ترتیب_By; i++){
        اظہاریہ *p;
        Bitmask m_رکن;
        اگر( MASKBIT(i) & obSat ) جاری;
        p = p_ترتیب_By->a[i].p_اظہاریہ;
        m_رکن = sqlite3_Where_اظہاریہ_Usage(&pWInfo->s_نقاب_سیٹ,p);
        اگر( m_رکن==0 && !sqlite3_اظہاریہ_Is_Constant(0,p) ) جاری;
        اگر( (m_رکن&~ترتیب_Distinct_نقاب)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  اگر( obSat==ob_مکمل ) واپس (i8)n_ترتیب_By;
  اگر( !is_ترتیب_Distinct ){
    برائے(i=n_ترتیب_By-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      اگر( (obSat&m)==m ) واپس i;
    }
    واپس 0;
  }
  واپس -1;
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
  واپس pWInfo->sorted;
}

#اگر_تعریف_شدہ WHERETRACE_ENABLED
/* For debugging use only: */
ساکن ثابت حرف *where_راستہ_Name(Where_راستہ *p_راستہ, صحیح nLoop, WhereLoop *p_آخری){
  ساکن حرف zName[65];
  صحیح i;
  برائے(i=0; i<nLoop; i++){ zName[i] = p_راستہ->aLoop[i]->cId; }
  اگر( p_آخری ) zName[i++] = p_آخری->cId;
  zName[i] = 0;
  واپس zName;
}
#اگر_ختم

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
ساکن LogEst where_ترتیب_کاری_لاگت(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst n_صف,       /* Estimated number of rows to sort */
  صحیح n_ترتیب_By,      /* Number of ORDER BY clause terms */
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
  LogEst r_ترتیب_دو_لاگت, n_ستون;
  أكد( pWInfo->pSelect!=0 );
  أكد( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  n_ستون = sqlite3LogEst((pWInfo->pSelect->pEList->n_اظہاریہ+59)/30);
  r_ترتیب_دو_لاگت = n_صف + n_ستون;
  اگر( nSorted>0 ){
    /* Scale the result by (Y/X) */
    r_ترتیب_دو_لاگت += sqlite3LogEst((n_ترتیب_By-nSorted)*100/n_ترتیب_By) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  اگر( (pWInfo->wctrlFlags & WHERE_USE_حد_بندی)!=0 ){
    r_ترتیب_دو_لاگت += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    اگر( nSorted!=0 ){
      r_ترتیب_دو_لاگت += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    اگر( pWInfo->i_حد_بندی<n_صف ){
      n_صف = pWInfo->i_حد_بندی;
    }
  }ورنہ اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    اگر( n_صف>10 ){ n_صف -= 10;  أكد( 10==sqlite3LogEst(2) ); }
  }
  r_ترتیب_دو_لاگت += estLog(n_صف);
  واپس r_ترتیب_دو_لاگت;
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
ساکن صحیح computeMxChoice(WhereInfo *pWInfo){
  صحیح nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  برائے(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
    أكد( pWLoop->نقاب_Self==MASKBIT(pWLoop->iTab) );
    أكد( pWLoop->p_اگلا_Loop==0 || pWLoop->iTab<=pWLoop->p_اگلا_Loop->iTab );
  }
#اگر_ختم /* SQLITE_DEBUG */

  اگر( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->b_ستارہ_مکمل
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_ستارہ_Query)
  ){
    Src_شے *aFromTabs;    /* All terms of the FROM clause */
    صحیح iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *p_آغاز;     /* Where to start searching for dimension-tables */

    pWInfo->b_ستارہ_مکمل = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    أكد( !pWInfo->b_ستارہ_مستعمل );
    aFromTabs = pWInfo->pTabList->a;
    p_آغاز = pWInfo->pLoops;
    برائے(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      صحیح nDep = 0;             /* Number of dimension tables */
      LogEst mx_تسلسل;             /* Maximum SCAN cost of a fact table */
      Bitmask m_نظر_آیا = 0;        /* Mask of dimension tables */
      Src_شے *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      اگر( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        اگر( iFromIdx+3 > nLoop ){
          توڑو;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        جب( p_آغاز && p_آغاز->iTab<=iFromIdx ){
          p_آغاز = p_آغاز->p_اگلا_Loop;
        }
      }
      برائے(pWLoop=p_آغاز; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
        اگر( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          توڑو; /* Constraint (bb) */
        }
        اگر( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->نقاب_Self & m_نظر_آیا)==0  /* pWInfo not already a dependency */
         && (pWLoop->نقاب_Self & mSelfJoin)==0 /* Not a self-join */
        ){
          اگر( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }ورنہ{
            nDep++;
            m_نظر_آیا |= pWLoop->نقاب_Self;
          }
        }
      }
      اگر( nDep<=2 ){
        جاری; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#اگر_تعریف_شدہ WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      اگر( !pWInfo->b_ستارہ_مستعمل ){
        برائے(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
          pWLoop->r_ستارہ_Delta = 0;
        }
      }
#اگر_ختم
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x80000 */
      اگر( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = m_نظر_آیا;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        برائے(pWLoop=p_آغاز; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
          اگر( mShow & pWLoop->نقاب_Self ){
            Src_شے *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->نقاب_Self;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#اگر_ختم
      pWInfo->b_ستارہ_مستعمل = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mx_تسلسل = LOGEST_MIN;
      برائے(pWLoop=p_آغاز; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
        اگر( pWLoop->iTab<iFromIdx ) جاری;
        اگر( pWLoop->iTab>iFromIdx ) توڑو;
        اگر( pWLoop->r_تسلسل>mx_تسلسل ) mx_تسلسل = pWLoop->r_تسلسل;
      }
      اگر( ALWAYS(mx_تسلسل<LOGEST_MAX) ) mx_تسلسل++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      برائے(pWLoop=p_آغاز; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
        اگر( (pWLoop->نقاب_Self & m_نظر_آیا)==0 ) جاری;
        اگر( pWLoop->nLTerm ) جاری;
        اگر( pWLoop->r_تسلسل<mx_تسلسل ){
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x80000 */
          اگر( sqlite3WhereTrace & 0x80000 ){
            Src_شے *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mx_تسلسل
            );
          }
          pWLoop->r_ستارہ_Delta = mx_تسلسل - pWLoop->r_تسلسل;
#اگر_ختم /* WHERETRACE_ENABLED */
          pWLoop->r_تسلسل = mx_تسلسل;
        }
      }
    }
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x80000 */
    اگر( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->b_ستارہ_مستعمل ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      برائے(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
        اگر( pWLoop->r_ستارہ_Delta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#اگر_ختم
  }
  واپس pWInfo->b_ستارہ_مستعمل ? 18 : 12;
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
ساکن SQLITE_NOINLINE صحیح whereLoopIsNoBetter(
  ثابت WhereLoop *pCandidate,
  ثابت WhereLoop *pBaseline
){
  اگر( (pCandidate->wsFlags & WHERE_اشاریہ_دار)==0 ) واپس 1;
  اگر( (pBaseline->wsFlags & WHERE_اشاریہ_دار)==0 ) واپس 1;
  اگر( pCandidate->u.btree.p_اشاریہ->sz_Idx_صف <
        pBaseline->u.btree.p_اشاریہ->sz_Idx_صف ) واپس 0;
  واپس 1;
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
ساکن صحیح where_راستہ_Solver(WhereInfo *pWInfo, LogEst n_صف_Est){
  صحیح mxChoice;             /* Maximum number of simultaneous paths tracked */
  صحیح nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  صحیح iLoop;                /* Loop counter over the terms of the join */
  صحیح ii, jj;               /* Loop counters */
  صحیح mxI = 0;              /* Index of next entry to replace */
  صحیح n_ترتیب_By;             /* Number of ORDER BY clause terms */
  LogEst mx_لاگت = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  صحیح n_مقصد, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  Where_راستہ *aFrom;         /* All nFrom paths at the previous level */
  Where_راستہ *a_مقصد;           /* The nTo best paths at the current level */
  Where_راستہ *pFrom;         /* An element of aFrom[] that we are working on */
  Where_راستہ *p_مقصد;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *a_ترتیب_دو_لاگت = 0;    /* Sorting and partial sorting costs */
  حرف *pSpace;             /* Temporary memory used by this routine */
  صحیح nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     n_صف_Est, pParse->nQueryLoop));
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
  }ورنہ اگر( nLoop==2 ){
    mxChoice = 5;
  }ورنہ اگر( pParse->nErr ){
    mxChoice = 1;
  }ورنہ{
    mxChoice = computeMxChoice(pWInfo);
  }
  أكد( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  اگر( pWInfo->p_ترتیب_By==0 || n_صف_Est==0 ){
    n_ترتیب_By = 0;
  }ورنہ{
    n_ترتیب_By = pWInfo->p_ترتیب_By->n_اظہاریہ;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (حجم(Where_راستہ)+حجم(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += حجم(LogEst) * n_ترتیب_By;
  pSpace = sqlite3_ڈھیر_Alloc_Raw_NN(pParse->db, nSpace);
  اگر( pSpace==0 ) واپس SQLITE_NOMEM_BKPT;
  a_مقصد = (Where_راستہ*)pSpace;
  aFrom = a_مقصد+mxChoice;
  memset(aFrom, 0, حجم(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  برائے(ii=mxChoice*2, pFrom=a_مقصد; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  اگر( n_ترتیب_By ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    a_ترتیب_دو_لاگت = (LogEst*)pX;
    memset(a_ترتیب_دو_لاگت, 0, حجم(LogEst) * n_ترتیب_By);
  }
  أكد( a_ترتیب_دو_لاگت==0 || &pSpace[nSpace]==(حرف*)&a_ترتیب_دو_لاگت[n_ترتیب_By] );
  أكد( a_ترتیب_دو_لاگت!=0 || &pSpace[nSpace]==(حرف*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].n_صف = MIN(pParse->nQueryLoop, 48);  أكد( 48==sqlite3LogEst(28) );
  nFrom = 1;
  أكد( aFrom[0].isOrdered==0 );
  اگر( n_ترتیب_By ){
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
  برائے(iLoop=0; iLoop<nLoop; iLoop++){
    n_مقصد = 0;
    برائے(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      برائے(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_اگلا_Loop){
        LogEst n_برآمد;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst r_لاگت;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask نقاب_New;                  /* Mask of src visited by (..) */
        Bitmask rev_نقاب;                  /* Mask of rev-order loops for (..) */

        اگر( (pWLoop->prereq & ~pFrom->نقاب_Loop)!=0 ) جاری;
        اگر( (pWLoop->نقاب_Self & pFrom->نقاب_Loop)!=0 ) جاری;
        اگر( (pWLoop->wsFlags & WHERE_AUTO_اشاریہ)!=0 && pFrom->n_صف<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          أكد( 10==sqlite3LogEst(2) );
          جاری;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->r_تسلسل + pFrom->n_صف;
        اگر( pWLoop->r_تیاری ){
          rUnsort = sqlite3_Log_Est_جوڑو(pWLoop->r_تیاری, rUnsort);
        }
        rUnsort = sqlite3_Log_Est_جوڑو(rUnsort, pFrom->rUnsort);
        n_برآمد = pFrom->n_صف + pWLoop->n_برآمد;
        نقاب_New = pFrom->نقاب_Loop | pWLoop->نقاب_Self;
        isOrdered = pFrom->isOrdered;
        اگر( isOrdered<0 ){
          rev_نقاب = 0;
          isOrdered = where_راستہ_Satisfies_ترتیب_By(pWInfo,
                       pWInfo->p_ترتیب_By, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &rev_نقاب);
        }ورنہ{
          rev_نقاب = pFrom->revLoop;
        }
        اگر( isOrdered>=0 && isOrdered<n_ترتیب_By ){
          اگر( a_ترتیب_دو_لاگت[isOrdered]==0 ){
            a_ترتیب_دو_لاگت[isOrdered] = where_ترتیب_کاری_لاگت(
                pWInfo, n_صف_Est, n_ترتیب_By, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          r_لاگت = sqlite3_Log_Est_جوڑو(rUnsort, a_ترتیب_دو_لاگت[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               a_ترتیب_دو_لاگت[isOrdered], (n_ترتیب_By-isOrdered), n_ترتیب_By,
               rUnsort, r_لاگت));
        }ورنہ{
          r_لاگت = rUnsort;
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
        برائے(jj=0, p_مقصد=a_مقصد; jj<n_مقصد; jj++, p_مقصد++){
          اگر( p_مقصد->نقاب_Loop==نقاب_New
           && ( ((p_مقصد->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==n_مقصد-1 );
            توڑو;
          }
        }
        اگر( jj>=n_مقصد ){
          /* None of the existing best-so-far paths match the candidate. */
          اگر( n_مقصد>=mxChoice
           && (r_لاگت>mx_لاگت || (r_لاگت==mx_لاگت && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x4 */
            اگر( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  where_راستہ_Name(pFrom, iLoop, pWLoop), r_لاگت, n_برآمد, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#اگر_ختم
            جاری;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          اگر( n_مقصد<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = n_مقصد++;
          }ورنہ{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          p_مقصد = &a_مقصد[jj];
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x4 */
          اگر( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                where_راستہ_Name(pFrom, iLoop, pWLoop), r_لاگت, n_برآمد, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#اگر_ختم
        }ورنہ{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          اگر( (p_مقصد->r_لاگت<r_لاگت)
           || (p_مقصد->r_لاگت==r_لاگت && p_مقصد->n_صف<n_برآمد)
           || (p_مقصد->r_لاگت==r_لاگت && p_مقصد->n_صف==n_برآمد && p_مقصد->rUnsort<rUnsort)
           || (p_مقصد->r_لاگت==r_لاگت && p_مقصد->n_صف==n_برآمد && p_مقصد->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, p_مقصد->aLoop[iLoop]) )
          ){
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x4 */
            اگر( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  where_راستہ_Name(pFrom, iLoop, pWLoop), r_لاگت, n_برآمد, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  where_راستہ_Name(p_مقصد, iLoop+1, 0), p_مقصد->r_لاگت, p_مقصد->n_صف,
                  p_مقصد->rUnsort, p_مقصد->isOrdered>=0 ? p_مقصد->isOrdered+'0' : '?');
            }
#اگر_ختم
            /* Discard the candidate path from further consideration */
            testcase( p_مقصد->r_لاگت==r_لاگت );
            جاری;
          }
          testcase( p_مقصد->r_لاگت==r_لاگت+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#اگر_تعریف_شدہ WHERETRACE_ENABLED /* 0x4 */
          اگر( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                where_راستہ_Name(pFrom, iLoop, pWLoop), r_لاگت, n_برآمد, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                where_راستہ_Name(p_مقصد, iLoop+1, 0), p_مقصد->r_لاگت, p_مقصد->n_صف,
                p_مقصد->rUnsort, p_مقصد->isOrdered>=0 ? p_مقصد->isOrdered+'0' : '?');
          }
#اگر_ختم
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        p_مقصد->نقاب_Loop = pFrom->نقاب_Loop | pWLoop->نقاب_Self;
        p_مقصد->revLoop = rev_نقاب;
        p_مقصد->n_صف = n_برآمد;
        p_مقصد->r_لاگت = r_لاگت;
        p_مقصد->rUnsort = rUnsort;
        p_مقصد->isOrdered = isOrdered;
        حافظہ_نقل(p_مقصد->aLoop, pFrom->aLoop, حجم(WhereLoop*)*iLoop);
        p_مقصد->aLoop[iLoop] = pWLoop;
        اگر( n_مقصد>=mxChoice ){
          mxI = 0;
          mx_لاگت = a_مقصد[0].r_لاگت;
          mxUnsort = a_مقصد[0].n_صف;
          برائے(jj=1, p_مقصد=&a_مقصد[1]; jj<mxChoice; jj++, p_مقصد++){
            اگر( p_مقصد->r_لاگت>mx_لاگت
             || (p_مقصد->r_لاگت==mx_لاگت && p_مقصد->rUnsort>mxUnsort)
            ){
              mx_لاگت = p_مقصد->r_لاگت;
              mxUnsort = p_مقصد->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#اگر_تعریف_شدہ WHERETRACE_ENABLED  /* >=2 */
    اگر( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      صحیح n_مکمل = 0;
      صحیح n_پیش_رفت;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      کرو{
        n_پیش_رفت = 0;
        rMin = 0x7fff;
        برائے(ii=0, p_مقصد=a_مقصد; ii<n_مقصد; ii++, p_مقصد++){
          اگر( p_مقصد->r_لاگت>rFloor && p_مقصد->r_لاگت<rMin ) rMin = p_مقصد->r_لاگت;
        }
        برائے(ii=0, p_مقصد=a_مقصد; ii<n_مقصد; ii++, p_مقصد++){
          اگر( p_مقصد->r_لاگت==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               where_راستہ_Name(p_مقصد, iLoop+1, 0), p_مقصد->r_لاگت, p_مقصد->n_صف,
               p_مقصد->isOrdered>=0 ? (p_مقصد->isOrdered+'0') : '?');
            اگر( p_مقصد->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", p_مقصد->revLoop);
            }ورنہ{
              sqlite3DebugPrintf("\n");
            }
            n_مکمل++;
            n_پیش_رفت++;
          }
        }
        rFloor = rMin;
      }جب( n_مکمل<n_مقصد && n_پیش_رفت>0 );
    }
#اگر_ختم

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = a_مقصد;
    a_مقصد = aFrom;
    aFrom = pFrom;
    nFrom = n_مقصد;
  }

  اگر( nFrom==0 ){
    sqlite3_Error_پیغام(pParse, "no query solution");
    sqlite3_ڈھیر_Free_NN(pParse->db, pSpace);
    واپس SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  أكد( nFrom==1 );
  pFrom = aFrom;

  أكد( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  برائے(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && n_صف_Est
  ){
    Bitmask not_مستعمل;
    صحیح rc = where_راستہ_Satisfies_ترتیب_By(pWInfo, pWInfo->p_نتیجہ_سیٹ, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &not_مستعمل);
    اگر( rc==pWInfo->p_نتیجہ_سیٹ->n_اظہاریہ ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  اگر( pWInfo->p_ترتیب_By ){
    pWInfo->nOBSat = pFrom->isOrdered;
    اگر( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      اگر( pFrom->isOrdered==pWInfo->p_ترتیب_By->n_اظہاریہ ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      أكد( pWInfo->pSelect->p_ترتیب_By==0
           || pWInfo->nOBSat <= pWInfo->pSelect->p_ترتیب_By->n_اظہاریہ );
    }ورنہ{
      pWInfo->rev_نقاب = pFrom->revLoop;
      اگر( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        اگر( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          اگر( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_عمود_اندر))!=(WHERE_IPK|WHERE_عمود_اندر)
          ){
            Bitmask m = 0;
            صحیح rc = where_راستہ_Satisfies_ترتیب_By(pWInfo, pWInfo->p_ترتیب_By, pFrom,
                      WHERE_ORDERBY_حد_بندی, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_عمود_اندر );
            اگر( rc==pWInfo->p_ترتیب_By->n_اظہاریہ ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->rev_نقاب = m;
            }
          }
        }
      }ورنہ اگر( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    اگر( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->p_ترتیب_By->n_اظہاریہ && nLoop>0
    ){
      Bitmask rev_نقاب = 0;
      صحیح n_ترتیب = where_راستہ_Satisfies_ترتیب_By(pWInfo, pWInfo->p_ترتیب_By,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &rev_نقاب
      );
      أكد( pWInfo->sorted==0 );
      اگر( n_ترتیب==pWInfo->p_ترتیب_By->n_اظہاریہ ){
        pWInfo->sorted = 1;
        pWInfo->rev_نقاب = rev_نقاب;
      }
    }
  }

  pWInfo->n_صف_برآمد = pFrom->n_صف;
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  pWInfo->r_کل_لاگت = pFrom->r_لاگت;
#اگر_ختم

  /* Free temporary memory and return success */
  sqlite3_ڈھیر_Free_NN(pParse->db, pSpace);
  واپس SQLITE_OK;
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
ساکن SQLITE_NOINLINE خالی whereInterstageHeuristic(WhereInfo *pWInfo){
  صحیح i;
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  صحیح once = 0;
#اگر_ختم
  برائے(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    اگر( p==0 ) توڑو;
    اگر( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      توڑو;
    }
    اگر( (p->wsFlags & (WHERE_عمود_EQ|WHERE_عمود_NULL|WHERE_عمود_اندر))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      برائے(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_اگلا_Loop){
        اگر( pLoop->iTab!=iTab ) جاری;
        اگر( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_اشاریہ))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          جاری;
        }
#اگر_تعریف_شدہ WHERETRACE_ENABLED
        اگر( sqlite3WhereTrace & 0x80 ){
          اگر( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#اگر_ختم /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }ورنہ{
      توڑو;
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
ساکن صحیح whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  Src_شے *p_شے;
  WhereClause *pWC;
  Where_رکن *p_رکن;
  WhereLoop *pLoop;
  صحیح iCur;
  صحیح j;
  جدول *pTab;
  اشاریہ *pIdx;
  Where_جائزہ جائزہ;

  pWInfo = pBuilder->pWInfo;
  اگر( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) واپس 0;
  أكد( pWInfo->pTabList->nSrc>=1 );
  p_شے = pWInfo->pTabList->a;
  pTab = p_شے->pSTab;
  اگر( IsVirtual(pTab) ) واپس 0;
  اگر( p_شے->fg.is_اشاریہ_دار_By || p_شے->fg.not_اشاریہ_دار ){
    testcase( p_شے->fg.is_اشاریہ_دار_By );
    testcase( p_شے->fg.not_اشاریہ_دار );
    واپس 0;
  }
  iCur = p_شے->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  p_رکن = where_جائزہ_Init(&جائزہ, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  جب( p_رکن && p_رکن->prereq_دایاں ) p_رکن = where_جائزہ_اگلا(&جائزہ);
  اگر( p_رکن ){
    testcase( p_رکن->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_عمود_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = p_رکن;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->r_تسلسل = 33;  /* 33==sqlite3LogEst(10) */
  }ورنہ{
    برائے(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      صحیح op_نقاب;
      أكد( pLoop->aLTermSpace==pLoop->aLTerm );
      اگر( !Is_Unique_اشاریہ(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_کلید_ستون>Array_سائز(pLoop->aLTermSpace)
      ) جاری;
      op_نقاب = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      برائے(j=0; j<pIdx->n_کلید_ستون; j++){
        p_رکن = where_جائزہ_Init(&جائزہ, pWC, iCur, j, op_نقاب, pIdx);
        جب( p_رکن && p_رکن->prereq_دایاں ) p_رکن = where_جائزہ_اگلا(&جائزہ);
        اگر( p_رکن==0 ) توڑو;
        testcase( p_رکن->eOperator & WO_IS );
        pLoop->aLTerm[j] = p_رکن;
      }
      اگر( j!=pIdx->n_کلید_ستون ) جاری;
      pLoop->wsFlags = WHERE_عمود_EQ|WHERE_ONEROW|WHERE_اشاریہ_دار;
      اگر( pIdx->isCovering || (p_شے->ستون_مستعمل & pIdx->ستون_Not_Idxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_اشاریہ = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->r_تسلسل = 39;  /* 39==sqlite3LogEst(15) */
      توڑو;
    }
  }
  اگر( pLoop->wsFlags ){
    pLoop->n_برآمد = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    أكد( pWInfo->s_نقاب_سیٹ.n==1 && iCur==pWInfo->s_نقاب_سیٹ.ix[0] );
    pLoop->نقاب_Self = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->n_صف_برآمد = 1;
    اگر( pWInfo->p_ترتیب_By ) pWInfo->nOBSat =  pWInfo->p_ترتیب_By->n_اظہاریہ;
    اگر( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    اگر( جائزہ.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#اگر_تعریف_شدہ SQLITE_DEBUG
    pLoop->cId = '0';
#اگر_ختم
#اگر_تعریف_شدہ WHERETRACE_ENABLED
    اگر( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#اگر_ختم
    واپس 1;
  }
  واپس 0;
}

/*
** Helper function for exprIsDeterministic().
*/
ساکن صحیح اظہاریہ_گرہ_Is_Deterministic(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_FUNCTION && اظہاریہ_Has_Property(p_اظہاریہ, EP_ConstFunc)==0 ){
    pWalker->e_رمز = 0;
    واپس WRC_Abort;
  }
  واپس WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
ساکن صحیح اظہاریہ_Is_Deterministic(اظہاریہ *p){
  Walker w;
  memset(&w, 0, حجم(w));
  w.e_رمز = 1;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.e_رمز;
}

 
#اگر_تعریف_شدہ WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
ساکن خالی showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  اگر( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    صحیح i;
    ساکن ثابت حرف zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    برائے(p=pWInfo->pLoops, i=0; p; p=p->p_اگلا_Loop, i++){
      p->cId = zLabel[i%(حجم(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#ورنہ
# define WHERETRACE_ALL_LOOPS(W,C)
#اگر_ختم

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
ساکن SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  صحیح i;
  Bitmask tab_مستعمل;
  صحیح has_دایاں_Join;

  /* Preconditions checked by the caller */
  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  أكد( pWInfo->p_نتیجہ_سیٹ!=0 );
  أكد( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tab_مستعمل = sqlite3_Where_اظہاریہ_List_Usage(&pWInfo->s_نقاب_سیٹ, pWInfo->p_نتیجہ_سیٹ);
  اگر( pWInfo->p_ترتیب_By ){
    tab_مستعمل |= sqlite3_Where_اظہاریہ_List_Usage(&pWInfo->s_نقاب_سیٹ, pWInfo->p_ترتیب_By);
  }
  has_دایاں_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  برائے(i=pWInfo->nLevel-1; i>=1; i--){
    Where_رکن *p_رکن, *p_انتہا;
    Src_شے *p_شے;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    p_شے = &pWInfo->pTabList->a[pLoop->iTab];
    اگر( (p_شے->fg.jointype & (JT_بایاں|JT_دایاں))!=JT_بایاں ) جاری;
    اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      جاری;
    }
    اگر( (tab_مستعمل & pLoop->نقاب_Self)!=0 ) جاری;
    p_انتہا = pWInfo->sWC.a + pWInfo->sWC.n_رکن;
    برائے(p_رکن=pWInfo->sWC.a; p_رکن<p_انتہا; p_رکن++){
      اگر( (p_رکن->prereqAll & pLoop->نقاب_Self)!=0 ){
        اگر( !اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_OuterON)
         || p_رکن->p_اظہاریہ->w.iJoin!=p_شے->iCursor
        ){
          توڑو;
        }
      }
      اگر( has_دایاں_Join
       && اظہاریہ_Has_Property(p_رکن->p_اظہاریہ, EP_InnerON)
       && NEVER(p_رکن->p_اظہاریہ->w.iJoin==p_شے->iCursor)
      ){
        توڑو;  /* restriction (5) */
      }
    }
    اگر( p_رکن<p_انتہا ) جاری;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->rev_نقاب>>1) & ~m1)!=0 );
    pWInfo->rev_نقاب = (m1 & pWInfo->rev_نقاب) | ((pWInfo->rev_نقاب>>1) & ~m1);
    notReady &= ~pLoop->نقاب_Self;
    برائے(p_رکن=pWInfo->sWC.a; p_رکن<p_انتہا; p_رکن++){
      اگر( (p_رکن->prereqAll & pLoop->نقاب_Self)!=0 ){
        p_رکن->wtFlags |= رکن_CODED;
        p_رکن->prereqAll = 0;
      }
    }
    اگر( i!=pWInfo->nLevel-1 ){
      صحیح nByte = (pWInfo->nLevel-1-i) * حجم(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    أكد( pWInfo->nLevel>0 );
  }
  واپس notReady;
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
ساکن SQLITE_NOINLINE خالی whereCheckIfBloomFilterIsUseful(
  ثابت WhereInfo *pWInfo
){
  صحیح i;
  LogEst n_تلاش = 0;

  أكد( pWInfo->nLevel>=2 );
  أكد( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  برائے(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ثابت غیرعلامتی صحیح reqFlags = (WHERE_SELFCULL|WHERE_عمود_EQ);
    Src_شے *p_شے = &pWInfo->pTabList->a[pLoop->iTab];
    جدول *pTab = p_شے->pSTab;
    اگر( (pTab->tabFlags & TF_HasStat1)==0 ) توڑو;
    pTab->tabFlags |= TF_MaybeReanalyze;
    اگر( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_تلاش > pTab->n_صف_Log_Est
    ){
      testcase( p_شے->fg.jointype & JT_بایاں );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (دوہرا)sqlite3_Log_Est_مقصد_Int(n_تلاش), pTab->zName,
         (دوہرا)sqlite3_Log_Est_مقصد_Int(pTab->n_صف_Log_Est)));
    }
    n_تلاش += pLoop->n_برآمد;
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
ساکن SQLITE_NOINLINE خالی where_جوڑو_اشاریہ_دار_اظہاریہ(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  اشاریہ *pIdx,       /* The index-on-expression that contains the expressions */
  صحیح iIdxCur,       /* Cursor number for pIdx */
  Src_شے *p_Tab_شے  /* The FROM clause entry for the table */
){
  صحیح i;
  اشاریہ_دار_اظہاریہ *p;
  جدول *pTab;
  أكد( pIdx->b_Has_اظہاریہ );
  pTab = pIdx->p_جدول;
  برائے(i=0; i<pIdx->n_عمود; i++){
    اظہاریہ *p_اظہاریہ;
    صحیح j = pIdx->ai_عمود[i];
    اگر( j==XN_اظہاریہ ){
      p_اظہاریہ = pIdx->a_ستون_اظہاریہ->a[i].p_اظہاریہ;
    }ورنہ اگر( j>=0 && (pTab->a_ستون[j].ستون_Flags & COLFLAG_VIRTUAL)!=0 ){
      p_اظہاریہ = sqlite3_عمود_اظہاریہ(pTab, &pTab->a_ستون[j]);
    }ورنہ{
      جاری;
    }
    اگر( sqlite3_اظہاریہ_Is_Constant(0,p_اظہاریہ) ) جاری;
    p = sqlite3DbMallocRaw(pParse->db,  حجم(اشاریہ_دار_اظہاریہ));
    اگر( p==0 ) توڑو;
    p->pIENext = pParse->pIdxEpr;
#اگر_تعریف_شدہ WHERETRACE_ENABLED
    اگر( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      اگر( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_اظہاریہ(p_اظہاریہ);
    }
#اگر_ختم
    p->p_اظہاریہ = sqlite3_اظہاریہ_Dup(pParse->db, p_اظہاریہ, 0);
    p->i_ڈیٹا_Cur = p_Tab_شے->iCursor;
    p->iIdxCur = iIdxCur;
    p->i_Idx_ستون = i;
    p->b_Maybe_Null_صف = (p_Tab_شے->fg.jointype & (JT_بایاں|JT_LTORJ|JT_دایاں))!=0;
    اگر( sqlite3_اشاریہ_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->z_ستون_Aff[i];
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#اگر_ختم
    pParse->pIdxEpr = p;
    اگر( p->pIENext==0 ){
      خالی *pArg = (خالی*)&pParse->pIdxEpr;
      sqlite3_Parser_جوڑو_Cleanup(pParse, where_اشاریہ_دار_اظہاریہ_Cleanup, pArg);
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
ساکن SQLITE_NOINLINE خالی where_الٹ_جائزہ_ترتیب(WhereInfo *pWInfo){
  صحیح ii;
  برائے(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    Src_شے *p_شے = &pWInfo->pTabList->a[ii];
    اگر( !p_شے->fg.isCte
     || p_شے->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(p_شے->fg.isSubquery==0)
     || p_شے->u4.pSubq->pSelect->p_ترتیب_By==0
    ){
      pWInfo->rev_نقاب |= MASKBIT(ii);
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
  اظہاریہ *pWhere,           /* The WHERE clause */
  اظہاریہ_List *p_ترتیب_By,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  اظہاریہ_List *p_نتیجہ_سیٹ,   /* Query result set.  Req'd for DISTINCT */
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
  Where_نقاب_سیٹ *p_نقاب_سیٹ;    /* The expression mask set */
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
            || (wctrlFlags & WHERE_USE_حد_بندی)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, حجم(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( p_ترتیب_By && p_ترتیب_By->n_اظہاریہ==BMS-1 );
  اگر( p_ترتیب_By && p_ترتیب_By->n_اظہاریہ>=BMS ){
    p_ترتیب_By = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  اگر( pTabList->nSrc>BMS ){
    sqlite3_Error_پیغام(pParse, "at most %d tables in a join", BMS);
    واپس 0;
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
  اگر( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    جاؤ whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->p_ترتیب_By = p_ترتیب_By;
#اگر WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#اگر_ختم
  pWInfo->p_نتیجہ_سیٹ = p_نتیجہ_سیٹ;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->i_حد_بندی = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  memset(&pWInfo->a[0], 0, حجم(WhereLoop)+nTabList*حجم(WhereLevel));
  أكد( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  p_نقاب_سیٹ = &pWInfo->s_نقاب_سیٹ;
  p_نقاب_سیٹ->n = 0;
  p_نقاب_سیٹ->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((حرف*)pWInfo)+nByteWInfo);
  أكد( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#اگر_تعریف_شدہ SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#اگر_ختم

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3_Where_بانٹو(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  اگر( nTabList==0 ){
    اگر( p_ترتیب_By ) pWInfo->nOBSat = p_ترتیب_By->n_اظہاریہ;
    اگر( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    اگر( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_قدر)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }ورنہ{
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
    کرو{
      create_نقاب(p_نقاب_سیٹ, pTabList->a[ii].iCursor);
      sqlite3_Where_Tab_Func_دلائل(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }جب( (++ii)<pTabList->nSrc );
  #اگر_تعریف_شدہ SQLITE_DEBUG
    {
      Bitmask mx = 0;
      برائے(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, pTabList->a[ii].iCursor);
        أكد( m>=mx );
        mx = m;
      }
    }
  #اگر_ختم
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_اظہاریہ_Analyze(pTabList, &pWInfo->sWC);
  اگر( pSelect && pSelect->p_حد_بندی ){
    sqlite3_Where_جوڑو_حد_بندی(&pWInfo->sWC, pSelect);
  }
  اگر( pParse->nErr ) جاؤ whereBeginError;

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
  برائے(ii=0; ii<sWLB.pWC->n_بنیاد; ii++){
    Where_رکن *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    اظہاریہ *pX;                          /* The expression of pT */
    اگر( pT->wtFlags & رکن_VIRTUAL ) جاری;
    pX = pT->p_اظہاریہ;
    أكد( pX!=0 );
    أكد( pT->prereqAll!=0 || !اظہاریہ_Has_Property(pX, EP_OuterON) );
    اگر( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || اظہاریہ_Is_Deterministic(pX))   /* Condition (4) */
     && !(اظہاریہ_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_اظہاریہ_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= رکن_CODED;
    }
  }

  اگر( wctrlFlags & WHERE_WANT_DISTINCT ){
    اگر( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }ورنہ اگر( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_نتیجہ_سیٹ) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }ورنہ اگر( p_ترتیب_By==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->p_ترتیب_By = p_نتیجہ_سیٹ;
    }
  }

  /* Construct the WhereLoop objects */
#اگر defined(WHERETRACE_ENABLED)
  اگر( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    اگر( wctrlFlags & WHERE_USE_حد_بندی ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    اگر( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, حجم(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.p_ترتیب_By = p_ترتیب_By;
      sSelect.pEList = p_نتیجہ_سیٹ;
      sqlite3_درخت_View_Select(0, &sSelect, 0);
    }
    اگر( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#اگر_ختم

  اگر( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = where_Loop_جوڑو_All(&sWLB);
    اگر( rc ) جاؤ whereBeginError;

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
      جب( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_اگلا_Loop;
        whereLoopDelete(db, p);
      }
      rc = where_Loop_جوڑو_All(&sWLB);
      اگر( rc ) جاؤ whereBeginError;
    }
#اگر_ختم
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    where_راستہ_Solver(pWInfo, 0);
    اگر( db->mallocFailed ) جاؤ whereBeginError;
    اگر( pWInfo->p_ترتیب_By ){
       whereInterstageHeuristic(pWInfo);
       where_راستہ_Solver(pWInfo, pWInfo->n_صف_برآمد<0 ? 1 : pWInfo->n_صف_برآمد+1);
       اگر( db->mallocFailed ) جاؤ whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    اگر( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->n_صف_برآمد, pWInfo->n_صف_برآمد-30));
      pWInfo->n_صف_برآمد -= 30;
    }

  }
  أكد( pWInfo->pTabList!=0 );
  اگر( pWInfo->p_ترتیب_By==0 && (db->flags & SQLITE_الٹ_ترتیب)!=0 ){
    where_الٹ_جائزہ_ترتیب(pWInfo);
  }
  اگر( pParse->nErr ){
    جاؤ whereBeginError;
  }
  أكد( db->mallocFailed==0 );
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  اگر( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_کل_لاگت, pWInfo->n_صف_برآمد);
    اگر( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->rev_نقاب);
    }
    انتخاب( pWInfo->eDistinct ){
      حالت WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        توڑو;
      }
      حالت WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        توڑو;
      }
      حالت WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        توڑو;
      }
    }
    sqlite3DebugPrintf("\n");
    برائے(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#اگر_ختم

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
   && p_نتیجہ_سیٹ!=0           /* Condition (1) */
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
#اگر_ختم
  pWInfo->pParse->nQueryLoop += pWInfo->n_صف_برآمد;

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
  برائے(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    جدول *pTab;     /* Table to open */
    صحیح iDb;         /* Index of database containing table/index */
    Src_شے *p_Tab_شے;

    p_Tab_شے = &pTabList->a[pLevel->iFrom];
    pTab = p_Tab_شے->pSTab;
    iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    اگر( ii==0 || (p_Tab_شے[0].fg.jointype & JT_بایاں)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }ورنہ اگر( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }ورنہ{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    اگر( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }ورنہ
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
    اگر( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ثابت حرف *pVTab = (ثابت حرف *)sqlite3GetVTable(db, pTab);
      صحیح iCur = p_Tab_شے->iCursor;
      sqlite3_Vdbe_جوڑو_Op4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }ورنہ اگر( IsVirtual(pTab) ){
      /* noop */
    }ورنہ
#اگر_ختم
    اگر( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (p_Tab_شے->fg.jointype & (JT_LTORJ|JT_دایاں))!=0
    ){
      صحیح op = OP_OpenRead;
      اگر( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = p_Tab_شے->iCursor;
      };
      sqlite3_Open_جدول(pParse, p_Tab_شے->iCursor, iDb, pTab, op);
      أكد( p_Tab_شے->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_ستون==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_ستون==BMS );
      اگر( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->n_ستون<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_اشاریہ|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = p_Tab_شے->ستون_مستعمل;
        صحیح n = 0;
        برائے(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_مقصد_PTR(n), P4_INT32);
        أكد( n<=pTab->n_ستون );
      }
#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
      اگر( pLoop->u.btree.p_اشاریہ!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }ورنہ
#اگر_ختم
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_مستعمل_نقاب
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Columns_مستعمل, p_Tab_شے->iCursor,
                    LOWER32(p_Tab_شے->ستون_مستعمل), UPPER32(p_Tab_شے->ستون_مستعمل));
#اگر_ختم
      اگر( ii>=2
       && (p_Tab_شے[0].fg.jointype & (JT_LTORJ|JT_بایاں))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_If_خالی_خانے, p_Tab_شے->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }ورنہ{
      sqlite3_جدول_تالا(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    اگر( pLoop->wsFlags & WHERE_اشاریہ_دار ){
      اشاریہ *pIx = pLoop->u.btree.p_اشاریہ;
      صحیح i_اشاریہ_Cur;
      صحیح op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      أكد( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      اگر( !HasRowid(pTab) && Is_Primary_کلید_اشاریہ(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_اشاریہ_Cur = pLevel->iTabCur;
        op = 0;
      }ورنہ اگر( pWInfo->eOnePass!=ONEPASS_OFF ){
        اشاریہ *pJ = p_Tab_شے->pSTab->p_اشاریہ;
        i_اشاریہ_Cur = iAuxArg;
        أكد( wctrlFlags & WHERE_ONEPASS_DESIRED );
        جب( ALWAYS(pJ) && pJ!=pIx ){
          i_اشاریہ_Cur++;
          pJ = pJ->p_اگلا;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_اشاریہ_Cur;
      }ورنہ اگر( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_اشاریہ_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }ورنہ{
        i_اشاریہ_Cur = pParse->nTab++;
        اگر( pIx->b_Has_اظہاریہ && OptimizationEnabled(db, SQLITE_اشاریہ_دار_اظہاریہ) ){
          where_جوڑو_اشاریہ_دار_اظہاریہ(pParse, pIx, i_اشاریہ_Cur, p_Tab_شے);
        }
        اگر( pIx->pPartIdxWhere && (p_Tab_شے->fg.jointype & JT_دایاں)==0 ){
          where_Part_Idx_اظہاریہ(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_اشاریہ_Cur, p_Tab_شے
          );
        }
      }
      pLevel->iIdxCur = i_اشاریہ_Cur;
      أكد( pIx!=0 );
      أكد( pIx->pSchema==pTab->pSchema );
      أكد( i_اشاریہ_Cur>=0 );
      اگر( op ){
        sqlite3_Vdbe_جوڑو_Op3(v, op, i_اشاریہ_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIx);
        اگر( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_عمود_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_ترتیب_دو)==0
         && (pLoop->wsFlags & WHERE_اندر_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_مستعمل_نقاب
        {
          u64 ستون_مستعمل = 0;
          صحیح ii, jj;
          برائے(ii=0; ii<pIx->n_عمود; ii++){
            jj = pIx->ai_عمود[ii];
            اگر( jj<0 ) جاری;
            اگر( jj>63 ) jj = 63;
            اگر( (p_Tab_شے->ستون_مستعمل & MASKBIT(jj))==0 ) جاری;
            ستون_مستعمل |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Columns_مستعمل, i_اشاریہ_Cur,
                            LOWER32(ستون_مستعمل),UPPER32(ستون_مستعمل));
        }
#اگر_ختم /* SQLITE_ENABLE_COLUMN_USED_MASK */
#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
        اگر( HasRowid(pTab) ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_CursorHint, i_اشاریہ_Cur, 0, p_Tab_شے->iCursor);
        }
#اگر_ختم
      }
    }
    اگر( iDb>=0 ) sqlite3_رمز_Verify_Schema(pParse, iDb);
    اگر( (p_Tab_شے->fg.jointype & JT_دایاں)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, حجم(Where_دایاں_Join)))!=0
    ){
      Where_دایاں_Join *pRJ = pLevel->pRJ;
      صحیح bBloomable = 0;
      pRJ->i_مطابقت = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, pRJ->regReturn);
      أكد( pTab==p_Tab_شے->pSTab );
      اگر( HasRowid(pTab) ){
        کلید_Info *pInfo;
        sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, pRJ->i_مطابقت, 1);
        bBloomable = 1;
        pInfo = sqlite3_کلید_Info_Alloc(pParse->db, 1, 0);
        اگر( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->a_ترتیب_دو_Flags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }ورنہ{
        اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, pRJ->i_مطابقت, pPk->n_کلید_ستون);
        sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pPk);
        bBloomable = sqlite3_اشاریہ_Bloomable(pPk,pPk->n_کلید_ستون);
      }
      اگر( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Blob, 65536, pRJ->regBloom);
      }ورنہ{
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
  pWInfo->i_چوٹی = sqlite3VdbeCurrentAddr(v);
  اگر( db->mallocFailed ) جاؤ whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  برائے(ii=0; ii<nTabList; ii++){
    صحیح addrExplain;
    صحیح wsFlags;
    Src_شے *pSrc;
    اگر( pParse->nErr ) جاؤ whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    اگر( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      صحیح iOnce = 0;
      أكد( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      اگر( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
      }ورنہ{
        iOnce = 0;
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      اگر( iOnce )  sqlite3_Vdbe_Jump_یہاں(v, iOnce);
    }
    أكد( pTabList == pWInfo->pTabList );
    اگر( (wsFlags & (WHERE_AUTO_اشاریہ|WHERE_BLOOMFILTER))!=0 ){
      اگر( (wsFlags & WHERE_AUTO_اشاریہ)!=0 ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOMATIC_اشاریہ
        construct_Automatic_اشاریہ(pParse, &pWInfo->sWC, notReady, pLevel);
#اگر_ختم
      }ورنہ{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      اگر( db->mallocFailed ) جاؤ whereBeginError;
    }
    addrExplain = sqlite3_Where_Explain_One_جائزہ(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3_Where_رمز_One_Loop_آغاز(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    اگر( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3_Where_جوڑو_جائزہ_حالت(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->i_انتہا_Where = sqlite3VdbeCurrentAddr(v);
  واپس pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  اگر( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#اگر_ختم /* WHERETRACE_ENABLED */
  واپس 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#اگر_غیر_تعریف_شدہ SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#ورنہ
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  ساکن خالی sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    صحیح pc,
    VdbeOp *pOp
  ){
    اگر( (db->flags & SQLITE_VdbeAddopTrace)==0 ) واپس;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3_Show_Where_رکن(0); /* So compiler won't complain about unused func */
  }
#اگر_ختم

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
خالی sqlite3_Where_انتہا(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  صحیح i_انتہا = sqlite3VdbeCurrentAddr(v);
  صحیح nRJ = 0;
#اگر_غیر_تعریف_شدہ SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  صحیح addrSeek = 0;
#اگر_ختم

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  برائے(i=pWInfo->nLevel-1; i>=0; i--){
    صحیح addr;
    pLevel = &pWInfo->a[i];
    اگر( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_دایاں_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->انتہا_Subrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    اگر( pLevel->op!=OP_Noop ){
#اگر_غیر_تعریف_شدہ SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      اشاریہ *pIdx;
      صحیح n;
      اگر( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_اشاریہ_دار)!=0
       && (pIdx = pLoop->u.btree.p_اشاریہ)->hasStat1
       && (n = pLoop->u.btree.n_Distinct_ستون)>0
       && pIdx->ai_صف_Log_Est[n]>=36
      ){
        صحیح r1 = pParse->nMem+1;
        صحیح j, op;
        صحیح addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        اگر( pLevel->i_بایاں_Join ){
          addrIfNull = sqlite3_Vdbe_جوڑو_Op2(v, OP_If_Null_صف, pLevel->iIdxCur, r1);
        }
        برائے(j=0; j<n; j++){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_پچھلا ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3_Vdbe_جوڑو_Op4_Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 1, pLevel->p2);
        اگر( pLevel->i_بایاں_Join ){
          sqlite3_Vdbe_Jump_یہاں(v, addrIfNull);
        }
      }
#اگر_ختم /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    اگر( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }ورنہ اگر( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      صحیح iBypass = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
      أكد( pLoop->wsFlags & WHERE_AUTO_اشاریہ );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3_Vdbe_Jump_یہاں(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    اگر( pLevel->op!=OP_Noop ){
      sqlite3_Vdbe_جوڑو_Op3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_اگلا);
      VdbeCoverageIf(v, pLevel->op==OP_پچھلا);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#اگر_غیر_تعریف_شدہ SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      اگر( addrSeek ){
        sqlite3_Vdbe_Jump_یہاں(v, addrSeek);
        addrSeek = 0;
      }
#اگر_ختم
      اگر( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
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
#اگر_غیر_تعریف_شدہ SQLITE_LIKE_DOESNT_مطابقت_BLOBS
    أكد( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_اندر_ABLE) && pLevel->u.اندر.n_اندر>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    اگر( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_DecrJumpZero, (صحیح)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#اگر_ختم
    اگر( (pLoop->wsFlags & WHERE_اندر_ABLE)!=0 && pLevel->u.اندر.n_اندر>0 ){
      ساخت اندر_Loop *p_اندر;
      صحیح j;
      برائے(j=pLevel->u.اندر.n_اندر, p_اندر=&pLevel->u.اندر.a_اندر_Loop[j-1]; j>0; j--, p_اندر--){
        أكد( sqlite3VdbeGetOp(v, p_اندر->addr_اندر_چوٹی+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3_Vdbe_Jump_یہاں(v, p_اندر->addr_اندر_چوٹی+1);
        اگر( p_اندر->e_انتہا_Loop_Op!=OP_Noop ){
          اگر( p_اندر->n_سابقہ ){
            صحیح b_Early_برآمد =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_اندر_EARLYOUT)!=0;
            اگر( pLevel->i_بایاں_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, p_اندر->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + b_Early_برآمد, p_اندر->i_بنیاد);
              VdbeCoverage(v);
            }
            اگر( b_Early_برآمد ){
              sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  p_اندر->i_بنیاد, p_اندر->n_سابقہ);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3_Vdbe_Jump_یہاں(v, p_اندر->addr_اندر_چوٹی+1);
            }
          }
          sqlite3_Vdbe_جوڑو_Op2(v, p_اندر->e_انتہا_Loop_Op, p_اندر->iCur, p_اندر->addr_اندر_چوٹی);
          VdbeCoverage(v);
          VdbeCoverageIf(v, p_اندر->e_انتہا_Loop_Op==OP_پچھلا);
          VdbeCoverageIf(v, p_اندر->e_انتہا_Loop_Op==OP_اگلا);
        }
        sqlite3_Vdbe_Jump_یہاں(v, p_اندر->addr_اندر_چوٹی-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    اگر( pLevel->pRJ ){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    اگر( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_اشاریہ->zName));
      sqlite3_Vdbe_Jump_یہاں(v, pLevel->addrSkip);
      sqlite3_Vdbe_Jump_یہاں(v, pLevel->addrSkip-2);
    }
    اگر( pLevel->i_بایاں_Join ){
      صحیح ws = pLoop->wsFlags;
      addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_IfPos, pLevel->i_بایاں_Join); VdbeCoverage(v);
      أكد( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_اشاریہ_دار)!=0 );
      اگر( (ws & WHERE_IDX_ONLY)==0 ){
        Src_شے *pSrc = &pTabList->a[pLevel->iFrom];
        أكد( pLevel->iTabCur==pSrc->iCursor );
        اگر( pSrc->fg.بذریعہ_Coroutine ){
          صحیح m, n;
          أكد( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_نتیجہ;
          أكد( pSrc->pSTab!=0 );
          m = pSrc->pSTab->n_ستون;
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, pLevel->iTabCur);
      }
      اگر( (ws & WHERE_اشاریہ_دار)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        اگر( ws & WHERE_MULTI_OR ){
          اشاریہ *pIx = pLevel->u.pCoveringIdx;
          صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(db, pIx->pSchema);
          sqlite3_Vdbe_جوڑو_Op3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIx);
        }
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, pLevel->iIdxCur);
      }
      اگر( ws & WHERE_اندر_ABLE ){
        صحیح m;
        برائے(m=0; m<pLevel->u.اندر.n_اندر; m++){
          ساخت اندر_Loop *p_اندر = &pLevel->u.اندر.a_اندر_Loop[m];
          اگر( p_اندر->e_انتہا_Loop_Op!=OP_Noop ){
            sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, p_اندر->iCur);
          }
        }
      }
      اگر( pLevel->op==OP_Return ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, pLevel->p1, pLevel->addr_پہلا);
      }ورنہ{
        sqlite3VdbeGoto(v, pLevel->addr_پہلا);
      }
      sqlite3_Vdbe_Jump_یہاں(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  أكد( pWInfo->nLevel<=pTabList->nSrc );
  برائے(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    صحیح k, آخری_2;
    VdbeOp *pOp, *p_آخری_Op;
    اشاریہ *pIdx = 0;
    Src_شے *p_Tab_شے = &pTabList->a[pLevel->iFrom];
    جدول *pTab = p_Tab_شے->pSTab;
    أكد( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    اگر( pLevel->pRJ ){
      sqlite3_Where_دایاں_Join_Loop(pWInfo, i, pLevel);
      جاری;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    اگر( p_Tab_شے->fg.بذریعہ_Coroutine ){
      testcase( pParse->db->mallocFailed );
      أكد( p_Tab_شے->fg.isSubquery );
      أكد( p_Tab_شے->u4.pSubq->reg_نتیجہ>=0 );
      translate_عمود_مقصد_Copy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            p_Tab_شے->u4.pSubq->reg_نتیجہ, 0);
      جاری;
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
    اگر( pLoop->wsFlags & (WHERE_اشاریہ_دار|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_اشاریہ;
    }ورنہ اگر( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    اگر( pIdx
     && !db->mallocFailed
    ){
      اگر( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->p_جدول) ){
        آخری_2 = i_انتہا;
      }ورنہ{
        آخری_2 = pWInfo->i_انتہا_Where;
      }
      اگر( pIdx->b_Has_اظہاریہ ){
        اشاریہ_دار_اظہاریہ *p = pParse->pIdxEpr;
        جب( p ){
          اگر( p->iIdxCur==pLevel->iIdxCur ){
#اگر_تعریف_شدہ WHERETRACE_ENABLED
            اگر( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->i_Idx_ستون);
              اگر( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_اظہاریہ(p->p_اظہاریہ);
            }
#اگر_ختم
            p->i_ڈیٹا_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#اگر_تعریف_شدہ SQLITE_DEBUG
      اگر( db->flags & SQLITE_VdbeAddopTrace ){
        لکھو("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, آخری_2-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      أكد( pOp->opcode!=OP_عمود || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      أكد( pOp->opcode!=OP_If_Null_صف || pOp->p1!=pLevel->iTabCur );
#اگر_ختم
      pOp = sqlite3VdbeGetOp(v, k);
      p_آخری_Op = pOp + (آخری_2 - k);
      أكد( pOp<=p_آخری_Op );
      کرو{
        اگر( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }ورنہ اگر( pOp->opcode==OP_عمود
#اگر_تعریف_شدہ SQLITE_ENABLE_انحراف_SQL_FUNC
         || pOp->opcode==OP_انحراف
#اگر_ختم
        ){
          صحیح x = pOp->p2;
          أكد( pIdx->p_جدول==pTab );
#اگر_تعریف_شدہ SQLITE_ENABLE_انحراف_SQL_FUNC
          اگر( pOp->opcode==OP_انحراف ){
            /* Do not need to translate the column number */
          }ورنہ
#اگر_ختم
          اگر( !HasRowid(pTab) ){
            اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
            x = pPk->ai_عمود[x];
            أكد( x>=0 );
          }ورنہ{
            testcase( x!=sqlite3_Storage_عمود_مقصد_جدول(pTab,x) );
            x = sqlite3_Storage_عمود_مقصد_جدول(pTab,x);
          }
          x = sqlite3_جدول_عمود_مقصد_اشاریہ(pIdx, x);
          اگر( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }ورنہ اگر( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            اگر( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_پیغام(pParse, "internal query planner error");
              pParse->rc = SQLITE_اندرونی;
            }ورنہ{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3_Where_جوڑو_Explain_متن(pParse,
                  pLevel->addrBody-1,
                  pTabList,
                  pLevel,
                  pWInfo->wctrlFlags
              );
            }
          }
        }ورنہ اگر( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }ورنہ اگر( pOp->opcode==OP_If_Null_صف ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#اگر_تعریف_شدہ SQLITE_DEBUG
        k++;
#اگر_ختم
      }جب( (++pOp)<p_آخری_Op );
#اگر_تعریف_شدہ SQLITE_DEBUG
      اگر( db->flags & SQLITE_VdbeAddopTrace ) لکھو("TRANSLATE complete\n");
#اگر_ختم
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
  واپس;
}
