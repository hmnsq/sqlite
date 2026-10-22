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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
రకనిర్వచనం నిర్మాణం Hidden_సూచీ_Info Hidden_సూచీ_Info;
నిర్మాణం Hidden_సూచీ_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  పూర్ణాంకం eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 m_లోపల;                 /* Mask of terms that are <col> IN (...) */
  u32 m_Handle_లోపల;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_విలువ *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#నిర్వచించు SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_సూచీ_Info,aRhs) + (N)*పరిమాణం(sqlite3_విలువ*))

/* Forward declaration of methods */
స్థిర పూర్ణాంకం whereLoopResize(sqlite3*, WhereLoop*, పూర్ణాంకం);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_అడ్డువరుస_లెక్క(WhereInfo *pWInfo){
  తిరిగి pWInfo->n_అడ్డువరుస_బహిర్గతం;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
పూర్ణాంకం sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  తిరిగి pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
పూర్ణాంకం sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  తిరిగి pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
పూర్ణాంకం sqlite3_Where_క్రమం_By_హద్దు_Opt_Label(WhereInfo *pWInfo){
  WhereLevel *pInner;
  ఒకవేళ( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    తిరిగి pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  ప్రతిజ్ఞా( pInner->addrNxt!=0 );
  తిరిగి pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
శూన్య sqlite3_Where_Min_Max_Opt_Early_బహిర్గతం(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  పూర్ణాంకం i;
  ఒకవేళ( !pWInfo->bOrderedInnerLoop ) తిరిగి;
  ఒకవేళ( pWInfo->nOBSat==0 ) తిరిగి;
  కొరకు(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    ఒకవేళ( (pInner->pWLoop->wsFlags & WHERE_స్తంభం_లోపల)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      తిరిగి;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
పూర్ణాంకం sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  ప్రతిజ్ఞా( pWInfo->iContinue!=0 );
  తిరిగి pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
పూర్ణాంకం sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  తిరిగి pWInfo->iBreak;
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
పూర్ణాంకం sqlite3WhereOkOnePass(WhereInfo *pWInfo, పూర్ణాంకం *aiCur){
  జ్ఞాపకం_నకలు(aiCur, pWInfo->aiCurOnePass, పరిమాణం(పూర్ణాంకం)*2);
#నిర్వచితమైతే WHERETRACE_ENABLED
  ఒకవేళ( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#అయితే_ముగింపు
  తిరిగి pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
పూర్ణాంకం sqlite3_Where_ఉపయోగిస్తుంది_Deferred_Seek(WhereInfo *pWInfo){
  తిరిగి pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
స్థిర శూన్య whereOrMove(Where_Or_సమితి *pDest, Where_Or_సమితి *pSrc){
  pDest->n = pSrc->n;
  జ్ఞాపకం_నకలు(pDest->a, pSrc->a, pDest->n*పరిమాణం(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
స్థిర పూర్ణాంకం where_Or_చొప్పించు(
  Where_Or_సమితి *p_సమితి,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst r_నిరంతరం,           /* Run-cost of the new entry */
  LogEst n_బహిర్గతం            /* Number of outputs for the new entry */
){
  u16 i;
  Where_Or_ఖర్చు *p;
  కొరకు(i=p_సమితి->n, p=p_సమితి->a; i>0; i--, p++){
    ఒకవేళ( r_నిరంతరం<=p->r_నిరంతరం && (prereq & p->prereq)==prereq ){
      వెళ్ళు where_Or_చొప్పించు_పూర్తి;
    }
    ఒకవేళ( p->r_నిరంతరం<=r_నిరంతరం && (p->prereq & prereq)==p->prereq ){
      తిరిగి 0;
    }
  }
  ఒకవేళ( p_సమితి->n<N_OR_ఖర్చు ){
    p = &p_సమితి->a[p_సమితి->n++];
    p->n_బహిర్గతం = n_బహిర్గతం;
  }లేకపోతే{
    p = p_సమితి->a;
    కొరకు(i=1; i<p_సమితి->n; i++){
      ఒకవేళ( p->r_నిరంతరం>p_సమితి->a[i].r_నిరంతరం ) p = p_సమితి->a + i;
    }
    ఒకవేళ( p->r_నిరంతరం<=r_నిరంతరం ) తిరిగి 0;
  }
where_Or_చొప్పించు_పూర్తి:
  p->prereq = prereq;
  p->r_నిరంతరం = r_నిరంతరం;
  ఒకవేళ( p->n_బహిర్గతం>n_బహిర్గతం ) p->n_బహిర్గతం = n_బహిర్గతం;
  తిరిగి 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3_Where_Get_ముసుగు(Where_ముసుగు_సమితి *p_ముసుగు_సమితి, పూర్ణాంకం iCursor){
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( p_ముసుగు_సమితి->n<=(పూర్ణాంకం)పరిమాణం(Bitmask)*8 );
  ప్రతిజ్ఞా( p_ముసుగు_సమితి->n>0 || p_ముసుగు_సమితి->ix[0]<0 );
  ప్రతిజ్ఞా( iCursor>=-1 );
  ఒకవేళ( p_ముసుగు_సమితి->ix[0]==iCursor ){
    తిరిగి 1;
  }
  కొరకు(i=1; i<p_ముసుగు_సమితి->n; i++){
    ఒకవేళ( p_ముసుగు_సమితి->ix[i]==iCursor ){
      తిరిగి MASKBIT(i);
    }
  }
  తిరిగి 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
శూన్య *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  Where_Mem_ఖండం *p_ఖండం;
  p_ఖండం = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+పరిమాణం(*p_ఖండం));
  ఒకవేళ( p_ఖండం ){
    p_ఖండం->p_తదుపరి = pWInfo->p_Mem_గమ్యం_Free;
    p_ఖండం->sz = nByte;
    pWInfo->p_Mem_గమ్యం_Free = p_ఖండం;
    p_ఖండం++;
  }
  తిరిగి (శూన్య*)p_ఖండం;
}
శూన్య *sqlite3WhereRealloc(WhereInfo *pWInfo, శూన్య *pOld, u64 nByte){
  శూన్య *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  ఒకవేళ( pNew && pOld ){
    Where_Mem_ఖండం *pOldBlk = (Where_Mem_ఖండం*)pOld;
    pOldBlk--;
    ప్రతిజ్ఞా( pOldBlk->sz<nByte );
    జ్ఞాపకం_నకలు(pNew, pOld, pOldBlk->sz);
  }
  తిరిగి pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
స్థిర శూన్య create_ముసుగు(Where_ముసుగు_సమితి *p_ముసుగు_సమితి, పూర్ణాంకం iCursor){
  ప్రతిజ్ఞా( p_ముసుగు_సమితి->n < Array_కొలత(p_ముసుగు_సమితి->ix) );
  p_ముసుగు_సమితి->ix[p_ముసుగు_సమితి->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
స్థిర సమాసం *where_కుడి_Subexpr_Is_స్తంభం(సమాసం *p){
  p = sqlite3_సమాసం_Skip_Collate_And_Likely(p->p_కుడి);
  ఒకవేళ( ALWAYS(p!=0) && p->op==TK_స్తంభం && !సమాసం_Has_Property(p, EP_Fixed_నిలువు) ){
    తిరిగి p;
  }
  తిరిగి 0;
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
స్థిర SQLITE_NOINLINE స్థిరరాశి అక్షరం *సూచీ_లోపల_Affinity_Ok(
  Parse *pParse, 
  Where_పదం *p_పదం, 
  u8 idxaff
){
  సమాసం *pX = p_పదం->p_సమాసం;
  సమాసం inexpr;

  ప్రతిజ్ఞా( p_పదం->eOperator & WO_లోపల );

  ఒకవేళ( సమాసం_Use_XSelect(pX) ){
    పూర్ణాంకం i_క్షేత్రం = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(pX->p_ఎడమ) ){
      i_క్షేత్రం = p_పదం->u.x.i_క్షేత్రం - 1;
      inexpr.p_ఎడమ = pX->p_ఎడమ->x.pList->a[i_క్షేత్రం].p_సమాసం;
    }లేకపోతే{
      inexpr.p_ఎడమ = pX->p_ఎడమ;
    }
    inexpr.p_కుడి = pX->x.pSelect->pEList->a[i_క్షేత్రం].p_సమాసం;
    pX = &inexpr;
  }

  ఒకవేళ( sqlite3_సూచీ_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_సమాసం_Compare_Coll_Seq(pParse, pX);
    తిరిగి pRet ? pRet->zName : sqlite3_Str_ద్విమాన;
  }
  తిరిగి 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
స్థిర Where_పదం *where_పరిశీలన_తదుపరి(Where_పరిశీలన *p_పరిశీలన){
  పూర్ణాంకం iCur;            /* The cursor on the LHS of the term */
  i16 i_స్తంభం;         /* The column on the LHS of the term.  -1 for IPK */
  సమాసం *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  Where_పదం *p_పదం;    /* The term being tested */
  పూర్ణాంకం k = p_పరిశీలన->k;    /* Where to start scanning */

  ప్రతిజ్ఞా( p_పరిశీలన->iEquiv<=p_పరిశీలన->nEquiv );
  pWC = p_పరిశీలన->pWC;
  అయితే(1){
    i_స్తంభం = p_పరిశీలన->ai_స్తంభం[p_పరిశీలన->iEquiv-1];
    iCur = p_పరిశీలన->aiCur[p_పరిశీలన->iEquiv-1];
    ప్రతిజ్ఞా( pWC!=0 );
    ప్రతిజ్ఞా( iCur>=0 );
    చేయి{
      కొరకు(p_పదం=pWC->a+k; k<pWC->n_పదం; k++, p_పదం++){
        ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 || p_పదం->ఎడమ_Cursor<0 );
        ఒకవేళ( p_పదం->ఎడమ_Cursor==iCur
         && p_పదం->u.x.ఎడమ_స్తంభం==i_స్తంభం
         && (i_స్తంభం!=XN_సమాసం
             || sqlite3_సమాసం_Compare_Skip(p_పదం->p_సమాసం->p_ఎడమ,
                                       p_పరిశీలన->p_Idx_సమాసం,iCur)==0)
         && (p_పరిశీలన->iEquiv<=1 || !సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON))
        ){
          ఒకవేళ( (p_పదం->eOperator & WO_EQUIV)!=0
           && p_పరిశీలన->nEquiv<Array_కొలత(p_పరిశీలన->aiCur)
           && (pX = where_కుడి_Subexpr_Is_స్తంభం(p_పదం->p_సమాసం))!=0
          ){
            పూర్ణాంకం j;
            కొరకు(j=0; j<p_పరిశీలన->nEquiv; j++){
              ఒకవేళ( p_పరిశీలన->aiCur[j]==pX->i_పట్టిక
               && p_పరిశీలన->ai_స్తంభం[j]==pX->i_స్తంభం ){
                  ఆపు;
              }
            }
            ఒకవేళ( j==p_పరిశీలన->nEquiv ){
              p_పరిశీలన->aiCur[j] = pX->i_పట్టిక;
              p_పరిశీలన->ai_స్తంభం[j] = pX->i_స్తంభం;
              p_పరిశీలన->nEquiv++;
            }
          }
          ఒకవేళ( (p_పదం->eOperator & p_పరిశీలన->op_ముసుగు)!=0 ){
            /* Verify the affinity and collating sequence match */
            ఒకవేళ( p_పరిశీలన->zCollName && (p_పదం->eOperator & WO_ISNULL)==0 ){
              స్థిరరాశి అక్షరం *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = p_పదం->p_సమాసం;

              ఒకవేళ( (p_పదం->eOperator & WO_లోపల) ){
                zCollName = సూచీ_లోపల_Affinity_Ok(pParse, p_పదం, p_పరిశీలన->idxaff);
                ఒకవేళ( !zCollName ) కొనసాగు;
              }లేకపోతే{
                CollSeq *pColl;
                ఒకవేళ( !sqlite3_సూచీ_Affinity_Ok(pX, p_పరిశీలన->idxaff) ){
                  కొనసాగు;
                }
                ప్రతిజ్ఞా(pX->p_ఎడమ);
                pColl = sqlite3_సమాసం_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_ద్విమాన;
              }

              ఒకవేళ( sqlite3StrICmp(zCollName, p_పరిశీలన->zCollName) ){
                కొనసాగు;
              }
            }
            ఒకవేళ( (p_పదం->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = p_పదం->p_సమాసం->p_కుడి, ALWAYS(pX!=0))
             && pX->op==TK_స్తంభం
             && pX->i_పట్టిక==p_పరిశీలన->aiCur[0]
             && pX->i_స్తంభం==p_పరిశీలన->ai_స్తంభం[0]
            ){
              testcase( p_పదం->eOperator & WO_IS );
              కొనసాగు;
            }
            p_పరిశీలన->pWC = pWC;
            p_పరిశీలన->k = k+1;
#నిర్వచితమైతే WHERETRACE_ENABLED
            ఒకవేళ( (sqlite3WhereTrace & 0x20000)!=0 && p_పరిశీలన->nEquiv>1 ){
              పూర్ణాంకం ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 p_పరిశీలన->aiCur[0], p_పరిశీలన->ai_స్తంభం[0], p_పదం->i_పదం);
              కొరకు(ii=1; ii<p_పరిశీలన->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   p_పరిశీలన->aiCur[ii], p_పరిశీలన->ai_స్తంభం[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#అయితే_ముగింపు
            తిరిగి p_పదం;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }అయితే( pWC!=0 );
    ఒకవేళ( p_పరిశీలన->iEquiv>=p_పరిశీలన->nEquiv ) ఆపు;
    pWC = p_పరిశీలన->pOrigWC;
    k = 0;
    p_పరిశీలన->iEquiv++;
  }
  తిరిగి 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
స్థిర SQLITE_NOINLINE Where_పదం *where_పరిశీలన_Init_సూచీ_సమాసం(Where_పరిశీలన *p_పరిశీలన){
  p_పరిశీలన->idxaff = sqlite3_సమాసం_Affinity(p_పరిశీలన->p_Idx_సమాసం);
  తిరిగి where_పరిశీలన_తదుపరి(p_పరిశీలన);
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
స్థిర Where_పదం *where_పరిశీలన_Init(
  Where_పరిశీలన *p_పరిశీలన,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  పూర్ణాంకం iCur,               /* Cursor to scan for */
  పూర్ణాంకం i_స్తంభం,            /* Column to scan for */
  u32 op_ముసుగు,             /* Operator(s) to scan for */
  సూచీ *pIdx             /* Must be compatible with this index */
){
  p_పరిశీలన->pOrigWC = pWC;
  p_పరిశీలన->pWC = pWC;
  p_పరిశీలన->p_Idx_సమాసం = 0;
  p_పరిశీలన->idxaff = 0;
  p_పరిశీలన->zCollName = 0;
  p_పరిశీలన->op_ముసుగు = op_ముసుగు;
  p_పరిశీలన->k = 0;
  p_పరిశీలన->aiCur[0] = iCur;
  p_పరిశీలన->nEquiv = 1;
  p_పరిశీలన->iEquiv = 1;
  ఒకవేళ( pIdx ){
    పూర్ణాంకం j = i_స్తంభం;
    i_స్తంభం = pIdx->ai_స్తంభం[j];
    ఒకవేళ( i_స్తంభం==pIdx->p_పట్టిక->iPKey ){
      i_స్తంభం = XN_ROWID;
    }లేకపోతే ఒకవేళ( i_స్తంభం>=0 ){
      p_పరిశీలన->idxaff = pIdx->p_పట్టిక->a_నిలువు[i_స్తంభం].affinity;
      p_పరిశీలన->zCollName = pIdx->azColl[j];
    }లేకపోతే ఒకవేళ( i_స్తంభం==XN_సమాసం ){
      p_పరిశీలన->p_Idx_సమాసం = pIdx->a_నిలువు_సమాసం->a[j].p_సమాసం;
      p_పరిశీలన->zCollName = pIdx->azColl[j];
      p_పరిశీలన->ai_స్తంభం[0] = XN_సమాసం;
      తిరిగి where_పరిశీలన_Init_సూచీ_సమాసం(p_పరిశీలన);
    }
  }లేకపోతే ఒకవేళ( i_స్తంభం==XN_సమాసం ){
    తిరిగి 0;
  }
  p_పరిశీలన->ai_స్తంభం[0] = i_స్తంభం;
  తిరిగి where_పరిశీలన_తదుపరి(p_పరిశీలన);
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
Where_పదం *sqlite3_Where_కనుగొను_పదం(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  పూర్ణాంకం iCur,             /* Cursor number of LHS */
  పూర్ణాంకం i_స్తంభం,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  సూచీ *pIdx           /* Must be compatible with this index, if not NULL */
){
  Where_పదం *p_ఫలితం = 0;
  Where_పదం *p;
  Where_పరిశీలన పరిశీలన;

  p = where_పరిశీలన_Init(&పరిశీలన, pWC, iCur, i_స్తంభం, op, pIdx);
  op &= WO_EQ|WO_IS;
  అయితే( p ){
    ఒకవేళ( (p->prereq_కుడి & notReady)==0 ){
      ఒకవేళ( p->prereq_కుడి==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        తిరిగి p;
      }
      ఒకవేళ( p_ఫలితం==0 ) p_ఫలితం = p;
    }
    p = where_పరిశీలన_తదుపరి(&పరిశీలన);
  }
  తిరిగి p_ఫలితం;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
స్థిర పూర్ణాంకం కనుగొను_సూచీ_నిలువు(
  Parse *pParse,                  /* Parse context */
  సమాసం_List *pList,                /* Expression list to search */
  పూర్ణాంకం i_పునాది,                      /* Cursor for table associated with pIdx */
  సూచీ *pIdx,                    /* Index to match column of */
  పూర్ణాంకం i_నిలువు                        /* Column of index to match */
){
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zColl = pIdx->azColl[i_నిలువు];

  కొరకు(i=0; i<pList->n_సమాసం; i++){
    సమాసం *p = sqlite3_సమాసం_Skip_Collate_And_Likely(pList->a[i].p_సమాసం);
    ఒకవేళ( ALWAYS(p!=0)
     && (p->op==TK_స్తంభం || p->op==TK_AGG_స్తంభం)
     && p->i_స్తంభం==pIdx->ai_స్తంభం[i_నిలువు]
     && p->i_పట్టిక==i_పునాది
    ){
      CollSeq *pColl = sqlite3_సమాసం_NNColl_Seq(pParse, pList->a[i].p_సమాసం);
      ఒకవేళ( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        తిరిగి i;
      }
    }
  }

  తిరిగి -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
స్థిర పూర్ణాంకం సూచీ_స్తంభం_Not_Null(సూచీ *pIdx, పూర్ణాంకం i_నిలువు){
  పూర్ణాంకం j;
  ప్రతిజ్ఞా( pIdx!=0 );
  ప్రతిజ్ఞా( i_నిలువు>=0 && i_నిలువు<pIdx->n_స్తంభం );
  j = pIdx->ai_స్తంభం[i_నిలువు];
  ఒకవేళ( j>=0 ){
    తిరిగి pIdx->p_పట్టిక->a_నిలువు[j].notNull;
  }లేకపోతే ఒకవేళ( j==(-1) ){
    తిరిగి 1;
  }లేకపోతే{
    ప్రతిజ్ఞా( j==(-2) );
    తిరిగి 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
స్థిర పూర్ణాంకం isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  సమాసం_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  పట్టిక *pTab;
  సూచీ *pIdx;
  పూర్ణాంకం i;                         
  పూర్ణాంకం i_పునాది;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  ఒకవేళ( pTabList->nSrc!=1 ) తిరిగి 0;
  i_పునాది = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  కొరకు(i=0; i<pDistinct->n_సమాసం; i++){
    సమాసం *p = sqlite3_సమాసం_Skip_Collate_And_Likely(pDistinct->a[i].p_సమాసం);
    ఒకవేళ( NEVER(p==0) ) కొనసాగు;
    ఒకవేళ( p->op!=TK_స్తంభం && p->op!=TK_AGG_స్తంభం ) కొనసాగు;
    ఒకవేళ( p->i_పట్టిక==i_పునాది && p->i_స్తంభం<0 ) తిరిగి 1;
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
  కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
    ఒకవేళ( !Is_Unique_సూచీ(pIdx) ) కొనసాగు;
    ఒకవేళ( pIdx->pPartIdxWhere ) కొనసాగు;
    కొరకు(i=0; i<pIdx->n_కీలకం_నిలువు; i++){
      ఒకవేళ( 0==sqlite3_Where_కనుగొను_పదం(pWC, i_పునాది, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        ఒకవేళ( కనుగొను_సూచీ_నిలువు(pParse, pDistinct, i_పునాది, pIdx, i)<0 ) ఆపు;
        ఒకవేళ( సూచీ_స్తంభం_Not_Null(pIdx, i)==0 ) ఆపు;
      }
    }
    ఒకవేళ( i==pIdx->n_కీలకం_నిలువు ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      తిరిగి 1;
    }
  }

  తిరిగి 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
స్థిర LogEst estLog(LogEst N){
  తిరిగి N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
స్థిర శూన్య translate_స్తంభం_గమ్యం_Copy(
  Parse *pParse,      /* Parsing context */
  పూర్ణాంకం i_ప్రారంభం,         /* Translate from this opcode to the end */
  పూర్ణాంకం iTabCur,        /* OP_Column/OP_Rowid references to this table */
  పూర్ణాంకం i_రిజిస్టరు,      /* The first column is in this register */
  పూర్ణాంకం iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, i_ప్రారంభం);
  పూర్ణాంకం i_చివర = sqlite3VdbeCurrentAddr(v);
  ఒకవేళ( pParse->db->mallocFailed ) తిరిగి;
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    రాయి("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, i_ప్రారంభం, i_చివర);
  }
#అయితే_ముగింపు
  కొరకు(; i_ప్రారంభం<i_చివర; i_ప్రారంభం++, pOp++){
    ఒకవేళ( pOp->p1!=iTabCur ) కొనసాగు;
    ఒకవేళ( pOp->opcode==OP_స్తంభం ){
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        రాయి("TRANSLATE OP_Column to OP_Copy at %d\n", i_ప్రారంభం);
      }
#అయితే_ముగింపు
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + i_రిజిస్టరు;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }లేకపోతే ఒకవేళ( pOp->opcode==OP_Rowid ){
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        రాయి("TRANSLATE OP_Rowid to OP_Sequence at %d\n", i_ప్రారంభం);
      }
#అయితే_ముగింపు
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
      ఒకవేళ( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#అయితే_ముగింపు
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#ఒకవేళ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
స్థిర శూన్య where_Trace_సూచీ_Info_Inputs(
  sqlite3_సూచీ_info *p,   /* The IndexInfo object */
  పట్టిక *pTab              /* The TABLE that is the virtual table */
){
  పూర్ణాంకం i;
  ఒకవేళ( (sqlite3WhereTrace & 0x10)==0 ) తిరిగి;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  కొరకు(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].i_స్తంభం,
       p->aConstraint[i].i_పదం_స్థానభ్రంశం,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  కొరకు(i=0; i<p->n_క్రమం_By; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->a_క్రమం_By[i].i_స్తంభం,
       p->a_క్రమం_By[i].desc);
  }
}
స్థిర శూన్య where_Trace_సూచీ_Info_Outputs(
  sqlite3_సూచీ_info *p,   /* The IndexInfo object */
  పట్టిక *pTab              /* The TABLE that is the virtual table */
){
  పూర్ణాంకం i;
  ఒకవేళ( (sqlite3WhereTrace & 0x10)==0 ) తిరిగి;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  కొరకు(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_సూచీ,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_అంకె);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->క్రమం_By_Consumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimated_ఖర్చు);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimated_అడ్డువరుసలు);
}
#లేకపోతే
#నిర్వచించు where_Trace_సూచీ_Info_Inputs(A,B)
#నిర్వచించు where_Trace_సూచీ_Info_Outputs(A,B)
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం constraintCompatibleWithOuterJoin(
  స్థిరరాశి Where_పదం *p_పదం,       /* WHERE clause term to check */
  స్థిరరాశి Src_అంశం *pSrc           /* Table we are trying to access */
){
  ప్రతిజ్ఞా( (pSrc->fg.jointype&(JT_ఎడమ|JT_LTORJ|JT_కుడి))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి))==JT_ఎడమ );
  testcase( (pSrc->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి))==JT_LTORJ );
  testcase( సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON) )
  testcase( సమాసం_Has_Property(p_పదం->p_సమాసం, EP_InnerON) );
  ఒకవేళ( !సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON|EP_InnerON)
   || p_పదం->p_సమాసం->w.iJoin != pSrc->iCursor
  ){
    తిరిగి 0;
  }
  ఒకవేళ( (pSrc->fg.jointype & (JT_ఎడమ|JT_కుడి))!=0
   && NEVER(సమాసం_Has_Property(p_పదం->p_సమాసం, EP_InnerON))
  ){
    తిరిగి 0;
  }
  తిరిగి 1;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOMATIC_సూచీ
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం స్తంభం_Is_Good_సూచీ_Candidate(
  స్థిరరాశి పట్టిక *pTab,
  పూర్ణాంకం i_నిలువు
){
  స్థిరరాశి సూచీ *pIdx;
  కొరకు(pIdx = pTab->p_సూచీ; pIdx!=0; pIdx=pIdx->p_తదుపరి){
    పూర్ణాంకం j;
    కొరకు(j=0; j<pIdx->n_కీలకం_నిలువు; j++){
       ఒకవేళ( pIdx->ai_స్తంభం[j]==i_నిలువు ){
         ఒకవేళ( j==0 ) తిరిగి 0;
         ఒకవేళ( pIdx->hasStat1 && pIdx->ai_అడ్డువరుస_Log_Est[j+1]>20 ) తిరిగి 0;
         ఆపు;
       }
    }
  }
  తిరిగి 1;
}
#అయితే_ముగింపు /* SQLITE_OMIT_AUTOMATIC_INDEX */



#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOMATIC_సూచీ
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
స్థిర పూర్ణాంకం పదం_Can_Drive_సూచీ(
  స్థిరరాశి Where_పదం *p_పదం,        /* WHERE clause term to check */
  స్థిరరాశి Src_అంశం *pSrc,           /* Table we are trying to access */
  స్థిరరాశి Bitmask notReady         /* Tables in outer loops of the join */
){
  అక్షరం aff;
  పూర్ణాంకం ఎడమ_నిలువు;
  
  ఒకవేళ( p_పదం->ఎడమ_Cursor!=pSrc->iCursor ) తిరిగి 0;
  ఒకవేళ( (p_పదం->eOperator & (WO_EQ|WO_IS))==0 ) తిరిగి 0;
  ప్రతిజ్ఞా( (pSrc->fg.jointype & JT_కుడి)==0 );
  ఒకవేళ( (pSrc->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి))!=0
   && !constraintCompatibleWithOuterJoin(p_పదం,pSrc)
  ){
    తిరిగి 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  ఒకవేళ( (p_పదం->prereq_కుడి & notReady)!=0 ) తిరిగి 0;
  ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
  ఎడమ_నిలువు = p_పదం->u.x.ఎడమ_స్తంభం;
  ఒకవేళ( ఎడమ_నిలువు<0 ) తిరిగి 0;
  aff = pSrc->pSTab->a_నిలువు[ఎడమ_నిలువు].affinity;
  ఒకవేళ( !sqlite3_సూచీ_Affinity_Ok(p_పదం->p_సమాసం, aff) ) తిరిగి 0;
  testcase( p_పదం->p_సమాసం->op==TK_IS );
  తిరిగి స్తంభం_Is_Good_సూచీ_Candidate(pSrc->pSTab, ఎడమ_నిలువు);
}
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOMATIC_సూచీ

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
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
స్థిర శూన్య explain_Automatic_సూచీ(
  Parse *pParse,
  సూచీ *pIdx,                    /* Automatic index to explain */
  పూర్ణాంకం bPartial,                   /* True if pIdx is a partial index */
  పూర్ణాంకం *pAddrExplain               /* OUT: Address of OP_Explain */
){
  ఒకవేళ( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    పట్టిక *pTab = pIdx->p_పట్టిక;
    స్థిరరాశి అక్షరం *zSep = "";
    అక్షరం *z_పాఠ్యం = 0;
    పూర్ణాంకం ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    ప్రతిజ్ఞా( pIdx->n_స్తంభం>1 );
    ప్రతిజ్ఞా( pIdx->ai_స్తంభం[pIdx->n_స్తంభం-1]==XN_ROWID || !HasRowid(pTab) );
    కొరకు(ii=0; ii<(pIdx->n_స్తంభం-1); ii++){
      స్థిరరాశి అక్షరం *zName = 0;
      పూర్ణాంకం i_నిలువు = pIdx->ai_స్తంభం[ii];

      zName = pTab->a_నిలువు[i_నిలువు].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    z_పాఠ్యం = sqlite3_str_ముగింపు(pStr);
    ఒకవేళ( z_పాఠ్యం==0 ){
      sqlite3OomFault(pParse->db);
    }లేకపోతే{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", z_పాఠ్యం, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(z_పాఠ్యం);
    }
  }
}
#లేకపోతే
# define explain_Automatic_సూచీ(a,b,c,d)
#అయితే_ముగింపు

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
స్థిర SQLITE_NOINLINE శూన్య construct_Automatic_సూచీ(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  స్థిరరాశి Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  పూర్ణాంకం n_కీలకం_నిలువు;                /* Number of columns in the constructed index */
  Where_పదం *p_పదం;           /* A single term of the WHERE clause */
  Where_పదం *pWCEnd;          /* End of pWC->a[] */
  సూచీ *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  పూర్ణాంకం addrInit;               /* Address of the initialization bypass jump */
  పట్టిక *p_పట్టిక;              /* The table being indexed */
  పూర్ణాంకం addr_శిఖరం;                /* Top of the index fill loop */
  పూర్ణాంకం reg_రికార్డు;              /* Register holding an index record */
  పూర్ణాంకం n;                      /* Column counter */
  పూర్ణాంకం i;                      /* Loop counter */
  పూర్ణాంకం mx_బిట్టు_నిలువు;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  అక్షరం *z_Not_వాడిన;             /* Extra space on the end of pIdx */
  Bitmask idx_నిలువుల_సంఖ్య;            /* Bitmap of columns used for indexing */
  Bitmask extra_నిలువుల_సంఖ్య;          /* Bitmap of additional columns */
  u8 పంపిన_Warning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  సమాసం *pPartial = 0;         /* Partial Index Expression */
  పూర్ణాంకం iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  Src_అంశం *pSrc;              /* The FROM clause term to get the next index */
  పూర్ణాంకం addrCounter = 0;        /* Address where integer counter is initialized */
  పూర్ణాంకం reg_పునాది;                /* Array of registers where record is assembled */
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  పూర్ణాంకం addrExp = 0;            /* Address of OP_Explain */
#అయితే_ముగింపు

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  addrInit = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_కీలకం_నిలువు = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  p_పట్టిక = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->n_పదం];
  pLoop = pLevel->pWLoop;
  idx_నిలువుల_సంఖ్య = 0;
  కొరకు(p_పదం=pWC->a; p_పదం<pWCEnd; p_పదం++){
    సమాసం *p_సమాసం = p_పదం->p_సమాసం;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    ఒకవేళ( (p_పదం->wtFlags & పదం_VIRTUAL)==0
     && sqlite3_సమాసం_Is_Single_పట్టిక_Constraint(p_సమాసం, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_సమాసం_And(pParse, pPartial,
                                sqlite3_సమాసం_Dup(pParse->db, p_సమాసం, 0));
    }
    ఒకవేళ( పదం_Can_Drive_సూచీ(p_పదం, pSrc, notReady) ){
      పూర్ణాంకం i_నిలువు;
      Bitmask c_ముసుగు;
      ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
      i_నిలువు = p_పదం->u.x.ఎడమ_స్తంభం;
      c_ముసుగు = i_నిలువు>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_నిలువు);
      testcase( i_నిలువు==BMS );
      testcase( i_నిలువు==BMS-1 );
      ఒకవేళ( !పంపిన_Warning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", p_పట్టిక->zName,
            p_పట్టిక->a_నిలువు[i_నిలువు].zCnName);
        పంపిన_Warning = 1;
      }
      ఒకవేళ( (idx_నిలువుల_సంఖ్య & c_ముసుగు)==0 ){
        ఒకవేళ( whereLoopResize(pParse->db, pLoop, n_కీలకం_నిలువు+1) ){
          వెళ్ళు చివర_auto_సూచీ_create;
        }
        pLoop->aLTerm[n_కీలకం_నిలువు++] = p_పదం;
        idx_నిలువుల_సంఖ్య |= c_ముసుగు;
      }
    }
  }
  ప్రతిజ్ఞా( n_కీలకం_నిలువు>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_కీలకం_నిలువు;
  pLoop->wsFlags = WHERE_స్తంభం_EQ | WHERE_IDX_ONLY | WHERE_సూచీకృత
                     | WHERE_AUTO_సూచీ;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  ఒకవేళ( IsView(p_పట్టిక) ){
    extra_నిలువుల_సంఖ్య = ALLBITS & ~idx_నిలువుల_సంఖ్య;
  }లేకపోతే{
    extra_నిలువుల_సంఖ్య = pSrc->నిలువు_వాడిన & (~idx_నిలువుల_సంఖ్య | MASKBIT(BMS-1));
  }
  ఒకవేళ( !HasRowid(p_పట్టిక) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    కొరకు(i=0; i<p_పట్టిక->n_నిలువు; i++){
      ఒకవేళ( (p_పట్టిక->a_నిలువు[i].నిలువు_Flags & COLFLAG_PRIMKEY)==0 ) కొనసాగు;
      ఒకవేళ( i>=BMS-1 ){
        extra_నిలువుల_సంఖ్య |= MASKBIT(BMS-1);
        ఆపు;
      }
      ఒకవేళ( idx_నిలువుల_సంఖ్య & MASKBIT(i) ) కొనసాగు;
      extra_నిలువుల_సంఖ్య |= MASKBIT(i);
    }
  }
  mx_బిట్టు_నిలువు = MIN(BMS-1,p_పట్టిక->n_నిలువు);
  testcase( p_పట్టిక->n_నిలువు==BMS-1 );
  testcase( p_పట్టిక->n_నిలువు==BMS-2 );
  కొరకు(i=0; i<mx_బిట్టు_నిలువు; i++){
    ఒకవేళ( extra_నిలువుల_సంఖ్య & MASKBIT(i) ) n_కీలకం_నిలువు++;
  }
  ఒకవేళ( pSrc->నిలువు_వాడిన & MASKBIT(BMS-1) ){
    n_కీలకం_నిలువు += p_పట్టిక->n_నిలువు - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  ప్రతిజ్ఞా( n_కీలకం_నిలువు <= p_పట్టిక->n_నిలువు + MAX(0, p_పట్టిక->n_నిలువు - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_సూచీ_Object(pParse->db, n_కీలకం_నిలువు+HasRowid(p_పట్టిక),
                                    0, &z_Not_వాడిన);
  ఒకవేళ( pIdx==0 ) వెళ్ళు చివర_auto_సూచీ_create;
  pLoop->u.btree.p_సూచీ = pIdx;
  pIdx->zName = "auto-index";
  pIdx->p_పట్టిక = p_పట్టిక;
  n = 0;
  idx_నిలువుల_సంఖ్య = 0;
  కొరకు(p_పదం=pWC->a; p_పదం<pWCEnd; p_పదం++){
    ఒకవేళ( పదం_Can_Drive_సూచీ(p_పదం, pSrc, notReady) ){
      పూర్ణాంకం i_నిలువు;
      Bitmask c_ముసుగు;
      ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
      i_నిలువు = p_పదం->u.x.ఎడమ_స్తంభం;
      c_ముసుగు = i_నిలువు>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_నిలువు);
      testcase( i_నిలువు==BMS-1 );
      testcase( i_నిలువు==BMS );
      ఒకవేళ( (idx_నిలువుల_సంఖ్య & c_ముసుగు)==0 ){
        సమాసం *pX = p_పదం->p_సమాసం;
        idx_నిలువుల_సంఖ్య |= c_ముసుగు;
        pIdx->ai_స్తంభం[n] = p_పదం->u.x.ఎడమ_స్తంభం;
        pColl = sqlite3_సమాసం_Compare_Coll_Seq(pParse, pX);
        ప్రతిజ్ఞా( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        ఒకవేళ( !sqlite3_Is_ద్విమాన(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_ద్విమాన;
        n++;
      }
    }
  }
  ప్రతిజ్ఞా( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  కొరకు(i=0; i<mx_బిట్టు_నిలువు; i++){
    ఒకవేళ( extra_నిలువుల_సంఖ్య & MASKBIT(i) ){
      pIdx->ai_స్తంభం[n] = i;
      pIdx->azColl[n] = sqlite3_Str_ద్విమాన;
      n++;
    }
  }
  ఒకవేళ( pSrc->నిలువు_వాడిన & MASKBIT(BMS-1) ){
    కొరకు(i=BMS-1; i<p_పట్టిక->n_నిలువు; i++){
      pIdx->ai_స్తంభం[n] = i;
      pIdx->azColl[n] = sqlite3_Str_ద్విమాన;
      n++;
    }
  }
  ప్రతిజ్ఞా( n==n_కీలకం_నిలువు );
  ఒకవేళ( HasRowid(p_పట్టిక) ){
    pIdx->ai_స్తంభం[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_ద్విమాన;
  }

  /* Create the automatic index */
  explain_Automatic_సూచీ(pParse, pIdx, pPartial!=0, &addrExp);
  ప్రతిజ్ఞా( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3_Vdbe_కూడు_Op2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_కీలకం_నిలువు+1);
  sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIdx);
  VdbeComment((v, "for %s", p_పట్టిక->zName));
  ఒకవేళ( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  ప్రతిజ్ఞా( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  ఒకవేళ( pSrc->fg.ద్వారా_Coroutine ){
    పూర్ణాంకం regYield;
    Subquery *pSubq;
    ప్రతిజ్ఞా( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    ప్రతిజ్ఞా( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, 0);
    sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_శిఖరం =  sqlite3_Vdbe_కూడు_Op1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }లేకపోతే{
    ప్రతిజ్ఞా( pLevel->addrHalt );
    addr_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  ఒకవేళ( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_సమాసం_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  reg_రికార్డు = sqlite3_Get_తాత్కాలిక_Reg(pParse);
  reg_పునాది = sqlite3_Generate_సూచీ_కీలకం(
      pParse, pIdx, pLevel->iTabCur, reg_రికార్డు, 0, 0, 0, 0
  );
  ఒకవేళ( pLevel->regFilter ){
    ప్రతిజ్ఞా( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pLevel->regFilter, 0,
                         reg_పునాది, pLoop->u.btree.nEq);
  }
  sqlite3_Vdbe_పరిశీలన_స్థితి_Counters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3_Vdbe_కూడు_Op2(v, OP_Idx_చొప్పించు, pLevel->iIdxCur, reg_రికార్డు);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  ఒకవేళ( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  ఒకవేళ( pSrc->fg.ద్వారా_Coroutine ){
    ప్రతిజ్ఞా( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, reg_పునాది+n);
    testcase( pParse->db->mallocFailed );
    ప్రతిజ్ఞా( pLevel->iIdxCur>0 );
    translate_స్తంభం_గమ్యం_Copy(pParse, addr_శిఖరం, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_ఫలితం, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_శిఖరం);
    pSrc->fg.ద్వారా_Coroutine = 0;
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_శిఖరం);
  }లేకపోతే{
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, pLevel->iTabCur, addr_శిఖరం+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    ఒకవేళ( (pSrc->fg.jointype & JT_ఎడమ)!=0 ){
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_శిఖరం);
    }
  }
  sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_రికార్డు);
 
  /* Jump here when skipping the initialization */
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addrInit);
  sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExp, addrExp, -1);

చివర_auto_సూచీ_create:
  sqlite3_సమాసం_Delete(pParse->db, pPartial);
}
#అయితే_ముగింపు /* SQLITE_OMIT_AUTOMATIC_INDEX */

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
స్థిర SQLITE_NOINLINE శూన్య sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  పూర్ణాంకం iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  పూర్ణాంకం addrOnce;                        /* Address of opening OP_Once */
  పూర్ణాంకం addr_శిఖరం;                         /* Address of OP_Rewind */
  పూర్ణాంకం addrCont;                        /* Jump here to skip a row */
  స్థిరరాశి Where_పదం *p_పదం;              /* For looping over WHERE clause terms */
  స్థిరరాశి Where_పదం *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  పూర్ణాంకం iCur;                            /* Cursor for table getting the filter */
  సూచీకృత_సమాసం *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  సూచీకృత_సమాసం *saved_p_Idx_Part_సమాసం;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_సమాసం = pParse->p_Idx_Part_సమాసం;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_సమాసం = 0;

  ప్రతిజ్ఞా( pLoop!=0 );
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pLoop->wsFlags & WHERE_BLOOMFILTER );
  ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
  చేయి{
    స్థిరరాశి SrcList *pTabList;
    స్థిరరాశి Src_అంశం *p_అంశం;
    స్థిరరాశి పట్టిక *pTab;
    u64 sz;
    పూర్ణాంకం iSrc;
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
    p_అంశం = &pTabList->a[iSrc];
    ప్రతిజ్ఞా( p_అంశం!=0 );
    pTab = p_అంశం->pSTab;
    ప్రతిజ్ఞా( pTab!=0 );
    sz = sqlite3_Log_Est_గమ్యం_Int(pTab->n_అడ్డువరుస_Log_Est);
    ఒకవేళ( sz<10000 ){
      sz = 10000;
    }లేకపోతే ఒకవేళ( sz>10000000 ){
      sz = 10000000;
    }
    ప్రతిజ్ఞా( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_కూడు_Op2(v, OP_Blob, (పూర్ణాంకం)sz, pLevel->regFilter);

    addr_శిఖరం = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.n_పదం];
    కొరకు(p_పదం=pWInfo->sWC.a; p_పదం<pWCEnd; p_పదం++){
      సమాసం *p_సమాసం = p_పదం->p_సమాసం;
      ఒకవేళ( (p_పదం->wtFlags & పదం_VIRTUAL)==0
       && sqlite3_సమాసం_Is_Single_పట్టిక_Constraint(p_సమాసం, pTabList, iSrc, 0)
      ){
        sqlite3_సమాసం_If_False(pParse, p_పదం->p_సమాసం, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    ఒకవేళ( pLoop->wsFlags & WHERE_IPK ){
      పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iCur, r1);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
    }లేకపోతే{
      సూచీ *pIdx = pLoop->u.btree.p_సూచీ;
      పూర్ణాంకం n = pLoop->u.btree.nEq;
      పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Range(pParse, n);
      పూర్ణాంకం jj;
      ప్రతిజ్ఞా( pIdx!=0 );
      కొరకు(jj=0; jj<n; jj++){
        ప్రతిజ్ఞా( pIdx->p_పట్టిక==p_అంశం->pSTab );
        sqlite3_సమాసం_సంకేతం_ఎక్కించు_సూచీ_స్తంభం(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_తాత్కాలిక_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, pLevel->iTabCur, addr_శిఖరం+1);
    VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_శిఖరం);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    అయితే( ++iLevel < pWInfo->nLevel ){
      స్థిరరాశి Src_అంశం *p_Tab_అంశం;
      pLevel = &pWInfo->a[iLevel];
      p_Tab_అంశం = &pWInfo->pTabList->a[pLevel->iFrom];
      ఒకవేళ( p_Tab_అంశం->fg.jointype & (JT_ఎడమ|JT_LTORJ) ) కొనసాగు;
      pLoop = pLevel->pWLoop;
      ఒకవేళ( NEVER(pLoop==0) ) కొనసాగు;
      ఒకవేళ( pLoop->prereq & notReady ) కొనసాగు;
      ఒకవేళ( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_స్తంభం_లోపల))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        ఆపు;
      }
    }
  }అయితే( iLevel < pWInfo->nLevel );
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_సమాసం = saved_p_Idx_Part_సమాసం;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
స్థిర Where_పదం *పదం_From_Where_Clause(WhereClause *pWC, పూర్ణాంకం i_పదం){
  WhereClause *p;
  కొరకు(p=pWC; p; p=p->pOuter){
    ఒకవేళ( i_పదం<p->n_పదం ) తిరిగి &p->a[i_పదం];
    i_పదం -= p->n_పదం;
  }
  తిరిగి 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
స్థిర sqlite3_సూచీ_info *allocate_సూచీ_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  Src_అంశం *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  పూర్ణాంకం i, j;
  పూర్ణాంకం n_పదం;
  Parse *pParse = pWInfo->pParse;
  నిర్మాణం sqlite3_సూచీ_constraint *pIdxCons;
  నిర్మాణం sqlite3_సూచీ_orderby *p_Idx_క్రమం_By;
  నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage;
  నిర్మాణం Hidden_సూచీ_Info *pHidden;
  Where_పదం *p_పదం;
  పూర్ణాంకం n_క్రమం_By;
  sqlite3_సూచీ_info *pIdxInfo;
  u16 mNoOmit = 0;
  స్థిరరాశి పట్టిక *pTab;
  పూర్ణాంకం eDistinct = 0;
  సమాసం_List *p_క్రమం_By = pWInfo->p_క్రమం_By;
  WhereClause *p;

  ప్రతిజ్ఞా( pSrc!=0 );
  pTab = pSrc->pSTab;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  కొరకు(p=pWC, n_పదం=0; p; p=p->pOuter){
    కొరకు(i=0, p_పదం=p->a; i<p->n_పదం; i++, p_పదం++){
      p_పదం->wtFlags &= ~పదం_OK;
      ఒకవేళ( p_పదం->ఎడమ_Cursor != pSrc->iCursor ) కొనసాగు;
      ఒకవేళ( p_పదం->prereq_కుడి & mUnusable ) కొనసాగు;
      ప్రతిజ్ఞా( Is_Power_యొక్క_Two(p_పదం->eOperator & ~WO_EQUIV) );
      testcase( p_పదం->eOperator & WO_లోపల );
      testcase( p_పదం->eOperator & WO_ISNULL );
      testcase( p_పదం->eOperator & WO_IS );
      testcase( p_పదం->eOperator & WO_ALL );
      ఒకవేళ( (p_పదం->eOperator & ~(WO_EQUIV))==0 ) కొనసాగు;
      ఒకవేళ( p_పదం->wtFlags & పదం_VNULL ) కొనసాగు;

      ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
      ప్రతిజ్ఞా( p_పదం->u.x.ఎడమ_స్తంభం>=XN_ROWID );
      ప్రతిజ్ఞా( p_పదం->u.x.ఎడమ_స్తంభం<pTab->n_నిలువు );
      ఒకవేళ( (pSrc->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి))!=0
          && !constraintCompatibleWithOuterJoin(p_పదం,pSrc)
        ){
        కొనసాగు;
      }
      n_పదం++;
      p_పదం->wtFlags |= పదం_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  n_క్రమం_By = 0;
  ఒకవేళ( p_క్రమం_By ){
    పూర్ణాంకం n = p_క్రమం_By->n_సమాసం;
    కొరకు(i=0; i<n; i++){
      సమాసం *p_సమాసం = p_క్రమం_By->a[i].p_సమాసం;
      సమాసం *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      ఒకవేళ( sqlite3_సమాసం_Is_Constant(0, p_సమాసం) ){
        కొనసాగు;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      ఒకవేళ( p_క్రమం_By->a[i].fg.క్రమీకరించు_Flags & KEYINFO_క్రమం_BIGNULL ) ఆపు;

      /* First case - a direct column references without a COLLATE operator */
      ఒకవేళ( p_సమాసం->op==TK_స్తంభం && p_సమాసం->i_పట్టిక==pSrc->iCursor ){
        ప్రతిజ్ఞా( p_సమాసం->i_స్తంభం>=XN_ROWID && p_సమాసం->i_స్తంభం<pTab->n_నిలువు );
        కొనసాగు;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      ఒకవేళ( p_సమాసం->op==TK_COLLATE
       && (pE2 = p_సమాసం->p_ఎడమ)->op==TK_స్తంభం
       && pE2->i_పట్టిక==pSrc->iCursor
      ){
        స్థిరరాశి అక్షరం *zColl;  /* The collating sequence name */
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
        ప్రతిజ్ఞా( p_సమాసం->u.zToken!=0 );
        ప్రతిజ్ఞా( pE2->i_స్తంభం>=XN_ROWID && pE2->i_స్తంభం<pTab->n_నిలువు );
        p_సమాసం->i_స్తంభం = pE2->i_స్తంభం;
        ఒకవేళ( pE2->i_స్తంభం<0 ) కొనసాగు;  /* Collseq does not matter for rowid */
        zColl = sqlite3_స్తంభం_Coll(&pTab->a_నిలువు[pE2->i_స్తంభం]);
        ఒకవేళ( zColl==0 ) zColl = sqlite3_Str_ద్విమాన;
        ఒకవేళ( sqlite3_stricmp(p_సమాసం->u.zToken, zColl)==0 ) కొనసాగు;
      }

      /* No matches cause a break out of the loop */
      ఆపు;
    }
    ఒకవేళ( i==n ){
      పూర్ణాంకం b_క్రమీకరించు_By_సమూహం = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      n_క్రమం_By = n;
      ఒకవేళ( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowid_వాడిన ){
        eDistinct = 2 + b_క్రమీకరించు_By_సమూహం;
      }లేకపోతే ఒకవేళ( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - b_క్రమీకరించు_By_సమూహం;
      }లేకపోతే ఒకవేళ( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, పరిమాణం(*pIdxInfo)
                           + (పరిమాణం(*pIdxCons) + పరిమాణం(*pUsage))*n_పదం
                           + పరిమాణం(*p_Idx_క్రమం_By)*n_క్రమం_By
                           + SZ_HIDDENINDEXINFO(n_పదం) );
  ఒకవేళ( pIdxInfo==0 ){
    sqlite3_Error_సందేశం(pParse, "out of memory");
    తిరిగి 0;
  }
  pHidden = (నిర్మాణం Hidden_సూచీ_Info*)&pIdxInfo[1];
  pIdxCons = (నిర్మాణం sqlite3_సూచీ_constraint*)&pHidden->aRhs[n_పదం];
  p_Idx_క్రమం_By = (నిర్మాణం sqlite3_సూచీ_orderby*)&pIdxCons[n_పదం];
  pUsage = (నిర్మాణం sqlite3_సూచీ_constraint_usage*)&p_Idx_క్రమం_By[n_క్రమం_By];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->a_క్రమం_By = p_Idx_క్రమం_By;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->నిలువు_వాడిన = (sqlite3_int64)pSrc->నిలువు_వాడిన;
  ఒకవేళ( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ((పట్టిక*)pTab);
    ప్రతిజ్ఞా( pPk!=0 );
    కొరకు(i=0; i<pPk->n_కీలకం_నిలువు; i++){
      పూర్ణాంకం i_నిలువు = pPk->ai_స్తంభం[i];
      ప్రతిజ్ఞా( i_నిలువు>=0 );
      ఒకవేళ( i_నిలువు>=BMS-1 ) i_నిలువు = BMS-1;
      pIdxInfo->నిలువు_వాడిన |= MASKBIT(i_నిలువు);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->m_లోపల = 0;
  కొరకు(p=pWC, i=j=0; p; p=p->pOuter){
    పూర్ణాంకం n_చివరి = i+p->n_పదం;;
    కొరకు(p_పదం=p->a; i<n_చివరి; i++, p_పదం++){
      u16 op;
      ఒకవేళ( (p_పదం->wtFlags & పదం_OK)==0 ) కొనసాగు;
      pIdxCons[j].i_స్తంభం = p_పదం->u.x.ఎడమ_స్తంభం;
      pIdxCons[j].i_పదం_స్థానభ్రంశం = i;
      op = p_పదం->eOperator & WO_ALL;
      ఒకవేళ( op==WO_లోపల ){
        ఒకవేళ( (p_పదం->wtFlags & పదం_ముక్క)==0 ){
          pHidden->m_లోపల |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      ఒకవేళ( op==WO_AUX ){
        pIdxCons[j].op = p_పదం->e_సరిపోలిక_Op;
      }లేకపోతే ఒకవేళ( op & (WO_ISNULL|WO_IS) ){
        ఒకవేళ( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_సూచీ_CONSTRAINT_ISNULL;
        }లేకపోతే{
          pIdxCons[j].op = SQLITE_సూచీ_CONSTRAINT_IS;
        }
      }లేకపోతే{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        ప్రతిజ్ఞా( WO_EQ==SQLITE_సూచీ_CONSTRAINT_EQ );
        ప్రతిజ్ఞా( WO_LT==SQLITE_సూచీ_CONSTRAINT_LT );
        ప్రతిజ్ఞా( WO_LE==SQLITE_సూచీ_CONSTRAINT_LE );
        ప్రతిజ్ఞా( WO_GT==SQLITE_సూచీ_CONSTRAINT_GT );
        ప్రతిజ్ఞా( WO_GE==SQLITE_సూచీ_CONSTRAINT_GE );
        ప్రతిజ్ఞా( p_పదం->eOperator&(WO_లోపల|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        ఒకవేళ( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_సమాసం_Is_సదిశ(p_పదం->p_సమాసం->p_కుడి)
          ){
          testcase( j!=i );
          ఒకవేళ( j<16 ) mNoOmit |= (1 << j);
          ఒకవేళ( op==WO_LT ) pIdxCons[j].op = WO_LE;
          ఒకవేళ( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  ప్రతిజ్ఞా( j==n_పదం );
  pIdxInfo->nConstraint = j;
  కొరకు(i=j=0; i<n_క్రమం_By; i++){
    సమాసం *p_సమాసం = p_క్రమం_By->a[i].p_సమాసం;
    ఒకవేళ( sqlite3_సమాసం_Is_Constant(0, p_సమాసం) ) కొనసాగు;
    ప్రతిజ్ఞా( p_సమాసం->op==TK_స్తంభం
         || (p_సమాసం->op==TK_COLLATE && p_సమాసం->p_ఎడమ->op==TK_స్తంభం
              && p_సమాసం->i_స్తంభం==p_సమాసం->p_ఎడమ->i_స్తంభం) );
    p_Idx_క్రమం_By[j].i_స్తంభం = p_సమాసం->i_స్తంభం;
    p_Idx_క్రమం_By[j].desc = p_క్రమం_By->a[i].fg.క్రమీకరించు_Flags & KEYINFO_క్రమం_DESC;
    j++;
  }
  pIdxInfo->n_క్రమం_By = j;

  *pmNoOmit = mNoOmit;
  తిరిగి pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
స్థిర శూన్య freeIdxStr(sqlite3_సూచీ_info *pIdxInfo){
  ఒకవేళ( pIdxInfo->need_గమ్యం_Free_Idx_Str ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->need_గమ్యం_Free_Idx_Str = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
స్థిర శూన్య free_సూచీ_Info(sqlite3 *db, sqlite3_సూచీ_info *pIdxInfo){
  Hidden_సూచీ_Info *pHidden;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( pIdxInfo!=0 );
  pHidden = (Hidden_సూచీ_Info*)&pIdxInfo[1];
  ప్రతిజ్ఞా( pHidden->pParse!=0 );
  ప్రతిజ్ఞా( pHidden->pParse->db==db );
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_విలువ_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
స్థిర పూర్ణాంకం vtab_ఉత్తమ_సూచీ(Parse *pParse, పట్టిక *pTab, sqlite3_సూచీ_info *p){
  పూర్ణాంకం rc;
  sqlite3_vtab *pVtab;

  ప్రతిజ్ఞా( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_సూచీ_Info_Inputs(p, pTab);
  pParse->db->n_Schema_తాళం++;
  rc = pVtab->pModule->x_ఉత్తమ_సూచీ(pVtab, p);
  pParse->db->n_Schema_తాళం--;
  where_Trace_సూచీ_Info_Outputs(p, pTab);

  ఒకవేళ( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    ఒకవేళ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }లేకపోతే ఒకవేళ( !pVtab->z_Err_సందేశం ){
      sqlite3_Error_సందేశం(pParse, "%s", sqlite3ErrStr(rc));
    }లేకపోతే{
      sqlite3_Error_సందేశం(pParse, "%s", pVtab->z_Err_సందేశం);
    }
  }
  ఒకవేళ( pTab->u.vtab.p->bAllSchemas ){
    sqlite3_Vtab_ఉపయోగిస్తుంది_All_Schemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_సందేశం);
  pVtab->z_Err_సందేశం = 0;
  తిరిగి rc;
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
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
స్థిర పూర్ణాంకం where_కీలకం_Stats(
  Parse *pParse,              /* Database connection */
  సూచీ *pIdx,                /* Index to consider domain of */
  Unpacked_రికార్డు *pRec,       /* Vector of values to consider */
  పూర్ణాంకం విడత_పైకి,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  సూచీ_Sample *aSample = pIdx->aSample;
  పూర్ణాంకం i_నిలువు;                   /* Index of required stats in anEq[] etc. */
  పూర్ణాంకం i;                      /* Index of first sample >= pRec */
  పూర్ణాంకం iSample;                /* Smallest sample larger than or equal to pRec */
  పూర్ణాంకం iMin = 0;               /* Smallest sample not yet tested */
  పూర్ణాంకం iTest;                  /* Next sample to test */
  పూర్ణాంకం res;                    /* Result of comparison operation */
  పూర్ణాంకం n_క్షేత్రం;                 /* Number of fields in pRec */
  tRowcnt i_క్రింది = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#నిర్వచితం_కాకపోతే SQLITE_DEBUG
  వాడని_PARAMETER( pParse );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pRec!=0 );
  ప్రతిజ్ఞా( pIdx->nSample>0 );
  ప్రతిజ్ఞా( pRec->n_క్షేత్రం>0 );


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
  ఒకవేళ( !HasRowid(pIdx->p_పట్టిక) && Is_Primary_కీలకం_సూచీ(pIdx) ){
    n_క్షేత్రం = pIdx->n_కీలకం_నిలువు;
  }లేకపోతే{
    n_క్షేత్రం = pIdx->n_స్తంభం;
  }
  n_క్షేత్రం = MIN(pRec->n_క్షేత్రం, n_క్షేత్రం);
  i_నిలువు = 0;
  iSample = pIdx->nSample * n_క్షేత్రం;
  చేయి{
    పూర్ణాంకం iSamp;                    /* Index in aSample[] of test sample */
    పూర్ణాంకం n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / n_క్షేత్రం;
    ఒకవేళ( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      కొరకు(n=(iTest % n_క్షేత్రం) + 1; n<n_క్షేత్రం; n++){
        ఒకవేళ( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) ఆపు;
      }
    }లేకపోతే{
      n = iTest + 1;
    }

    pRec->n_క్షేత్రం = n;
    res = sqlite3_Vdbe_రికార్డు_Compare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    ఒకవేళ( res<0 ){
      i_క్రింది = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }లేకపోతే ఒకవేళ( res==0 && n<n_క్షేత్రం ){
      i_క్రింది = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }లేకపోతే{
      iSample = iTest;
      i_నిలువు = n-1;
    }
  }అయితే( res && iMin<iSample );
  i = iSample / n_క్షేత్రం;

#నిర్వచితమైతే SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  ఒకవేళ( pParse->db->mallocFailed==0 ){
    ఒకవేళ( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      ప్రతిజ్ఞా( i<pIdx->nSample );
      ప్రతిజ్ఞా( i_నిలువు==n_క్షేత్రం-1 );
      pRec->n_క్షేత్రం = n_క్షేత్రం;
      ప్రతిజ్ఞా( 0==sqlite3_Vdbe_రికార్డు_Compare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }లేకపోతే{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      ప్రతిజ్ఞా( i<=pIdx->nSample && i>=0 );
      pRec->n_క్షేత్రం = i_నిలువు+1;
      ప్రతిజ్ఞా( i==pIdx->nSample
           || sqlite3_Vdbe_రికార్డు_Compare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      ఒకవేళ( i_నిలువు>0 ){
        pRec->n_క్షేత్రం = i_నిలువు;
        ప్రతిజ్ఞా( sqlite3_Vdbe_రికార్డు_Compare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      ఒకవేళ( i>0 ){
        pRec->n_క్షేత్రం = n_క్షేత్రం;
        ప్రతిజ్ఞా( sqlite3_Vdbe_రికార్డు_Compare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#అయితే_ముగింపు /* ifdef SQLITE_DEBUG */

  ఒకవేళ( res==0 ){
    /* Record pRec is equal to sample i */
    ప్రతిజ్ఞా( i_నిలువు==n_క్షేత్రం-1 );
    aStat[0] = aSample[i].anLt[i_నిలువు];
    aStat[1] = aSample[i].anEq[i_నిలువు];
  }లేకపోతే{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt i_పై, iGap;
    ఒకవేళ( i>=pIdx->nSample ){
      i_పై = pIdx->n_అడ్డువరుస_Est0;
    }లేకపోతే{
      i_పై = aSample[i].anLt[i_నిలువు];
    }

    ఒకవేళ( i_క్రింది>=i_పై ){
      iGap = 0;
    }లేకపోతే{
      iGap = i_పై - i_క్రింది;
    }
    ఒకవేళ( విడత_పైకి ){
      iGap = (iGap*2)/3;
    }లేకపోతే{
      iGap = iGap/3;
    }
    aStat[0] = i_క్రింది + iGap;
    aStat[1] = pIdx->aAvgEq[n_క్షేత్రం-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->n_క్షేత్రం = n_క్షేత్రం;
  తిరిగి i;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

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
స్థిర LogEst whereRangeAdjust(Where_పదం *p_పదం, LogEst nNew){
  LogEst nRet = nNew;
  ఒకవేళ( p_పదం ){
    ఒకవేళ( p_పదం->truthProb<=0 ){
      nRet += p_పదం->truthProb;
    }లేకపోతే ఒకవేళ( (p_పదం->wtFlags & పదం_VNULL)==0 ){
      nRet -= 20;        ప్రతిజ్ఞా( 20==sqlite3LogEst(4) );
    }
  }
  తిరిగి nRet;
}


#నిర్వచితమైతే SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
అక్షరం sqlite3_సూచీ_స్తంభం_Affinity(sqlite3 *db, సూచీ *pIdx, పూర్ణాంకం i_నిలువు){
  ప్రతిజ్ఞా( i_నిలువు>=0 && i_నిలువు<pIdx->n_స్తంభం );
  ఒకవేళ( !pIdx->z_నిలువు_Aff ){
    ఒకవేళ( sqlite3_సూచీ_Affinity_Str(db, pIdx)==0 ) తిరిగి SQLITE_AFF_BLOB;
  }
  ప్రతిజ్ఞా( pIdx->z_నిలువు_Aff[i_నిలువు]!=0 );
  తిరిగి pIdx->z_నిలువు_Aff[i_నిలువు];
}
#అయితే_ముగింపు


#నిర్వచితమైతే SQLITE_ENABLE_STAT4
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
స్థిర పూర్ణాంకం where_Range_Skip_పరిశీలన_Est(
  Parse *pParse,       /* Parsing & code generating context */
  Where_పదం *p_క్రింది,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_పదం *p_పై,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  పూర్ణాంకం *pb_పూర్తి          /* Set to true if at least one expr. value extracted */
){
  సూచీ *p = pLoop->u.btree.p_సూచీ;
  పూర్ణాంకం nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం n_క్రింది = -1;
  పూర్ణాంకం n_పై = p->nSample+1;
  పూర్ణాంకం rc = SQLITE_OK;
  u8 aff = sqlite3_సూచీ_స్తంభం_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_విలువ *p1 = 0;          /* Value extracted from pLower */
  sqlite3_విలువ *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_విలువ *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  ఒకవేళ( p_క్రింది ){
    rc = sqlite3_Stat4_విలువ_From_సమాసం(pParse, p_క్రింది->p_సమాసం->p_కుడి, aff, &p1);
    n_క్రింది = 0;
  }
  ఒకవేళ( p_పై && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_విలువ_From_సమాసం(pParse, p_పై->p_సమాసం->p_కుడి, aff, &p2);
    n_పై = p2 ? 0 : p->nSample;
  }

  ఒకవేళ( p1 || p2 ){
    పూర్ణాంకం i;
    పూర్ణాంకం nDiff;
    కొరకు(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3_Stat4_స్తంభం(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      ఒకవేళ( rc==SQLITE_OK && p1 ){
        పూర్ణాంకం res = sqlite3MemCompare(p1, pVal, pColl);
        ఒకవేళ( res>=0 ) n_క్రింది++;
      }
      ఒకవేళ( rc==SQLITE_OK && p2 ){
        పూర్ణాంకం res = sqlite3MemCompare(p2, pVal, pColl);
        ఒకవేళ( res>=0 ) n_పై++;
      }
    }
    nDiff = (n_పై - n_క్రింది);
    ఒకవేళ( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    ఒకవేళ( nDiff!=1 || p_పై==0 || p_క్రింది==0 ){
      పూర్ణాంకం nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->n_బహిర్గతం -= nAdjust;
      *pb_పూర్తి = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           n_క్రింది, n_పై, nAdjust*-1, pLoop->n_బహిర్గతం));
    }

  }లేకపోతే{
    ప్రతిజ్ఞా( *pb_పూర్తి==0 );
  }

  sqlite3_విలువ_Free(p1);
  sqlite3_విలువ_Free(p2);
  sqlite3_విలువ_Free(pVal);

  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

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
స్థిర పూర్ణాంకం where_Range_పరిశీలన_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Where_పదం *p_క్రింది,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_పదం *p_పై,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం n_బహిర్గతం = pLoop->n_బహిర్గతం;
  LogEst nNew;

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  సూచీ *p = pLoop->u.btree.p_సూచీ;
  పూర్ణాంకం nEq = pLoop->u.btree.nEq;

  ఒకవేళ( p->nSample>0 && ALWAYS(nEq<p->n_Sample_నిలువు)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    ఒకవేళ( nEq==pBuilder->nRecValid ){
      Unpacked_రికార్డు *pRec = pBuilder->pRec;
      tRowcnt a[2];
      పూర్ణాంకం nBtm = pLoop->u.btree.nBtm;
      పూర్ణాంకం n_శిఖరం = pLoop->u.btree.n_శిఖరం;

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
      tRowcnt i_క్రింది;     /* Rows less than the lower bound */
      tRowcnt i_పై;     /* Rows less than the upper bound */
      పూర్ణాంకం iLwrIdx = -2;   /* aSample[] for the lower bound */
      పూర్ణాంకం iUprIdx = -1;   /* aSample[] for the upper bound */

      ఒకవేళ( pRec ){
        testcase( pRec->n_క్షేత్రం!=pBuilder->nRecValid );
        pRec->n_క్షేత్రం = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      ఒకవేళ( nEq==0 ){
        i_క్రింది = 0;
        i_పై = p->n_అడ్డువరుస_Est0;
      }లేకపోతే{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_కీలకం_Stats(pParse, p, pRec, 0, a);
        i_క్రింది = a[0];
        i_పై = a[0] + a[1];
      }

      ప్రతిజ్ఞా( p_క్రింది==0 || (p_క్రింది->eOperator & (WO_GT|WO_GE))!=0 );
      ప్రతిజ్ఞా( p_పై==0 || (p_పై->eOperator & (WO_LT|WO_LE))!=0 );
      ప్రతిజ్ఞా( p->a_క్రమీకరించు_క్రమం!=0 );
      ఒకవేళ( p->a_క్రమీకరించు_క్రమం[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        మార్పిడి(Where_పదం*, p_క్రింది, p_పై);
        మార్పిడి(పూర్ణాంకం, nBtm, n_శిఖరం);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      ఒకవేళ( p_క్రింది ){
        పూర్ణాంకం n;                    /* Values extracted from pExpr */
        సమాసం *p_సమాసం = p_క్రింది->p_సమాసం->p_కుడి;
        rc = sqlite3_Stat4_Probe_సమితి_విలువ(pParse, p, &pRec, p_సమాసం, nBtm, nEq, &n);
        ఒకవేళ( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ముసుగు = WO_GT|WO_LE;
          ఒకవేళ( sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం)>n ) ముసుగు = (WO_LE|WO_LT);
          iLwrIdx = where_కీలకం_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((p_క్రింది->eOperator & ముసుగు) ? a[1] : 0);
          ఒకవేళ( iNew>i_క్రింది ) i_క్రింది = iNew;
          n_బహిర్గతం--;
          p_క్రింది = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      ఒకవేళ( p_పై ){
        పూర్ణాంకం n;                    /* Values extracted from pExpr */
        సమాసం *p_సమాసం = p_పై->p_సమాసం->p_కుడి;
        rc = sqlite3_Stat4_Probe_సమితి_విలువ(pParse, p, &pRec, p_సమాసం, n_శిఖరం, nEq, &n);
        ఒకవేళ( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ముసుగు = WO_GT|WO_LE;
          ఒకవేళ( sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం)>n ) ముసుగు = (WO_LE|WO_LT);
          iUprIdx = where_కీలకం_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((p_పై->eOperator & ముసుగు) ? a[1] : 0);
          ఒకవేళ( iNew<i_పై ) i_పై = iNew;
          n_బహిర్గతం--;
          p_పై = 0;
        }
      }

      pBuilder->pRec = pRec;
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( i_పై>i_క్రింది ){
          nNew = sqlite3LogEst(i_పై - i_క్రింది);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          ఒకవేళ( iLwrIdx==iUprIdx ){ nNew -= 20; }
          ప్రతిజ్ఞా( 20==sqlite3LogEst(4) );
        }లేకపోతే{
          nNew = 10;        ప్రతిజ్ఞా( 10==sqlite3LogEst(2) );
        }
        ఒకవేళ( nNew<n_బహిర్గతం ){
          n_బహిర్గతం = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)i_క్రింది, (u32)i_పై, n_బహిర్గతం));
      }
    }లేకపోతే{
      పూర్ణాంకం b_పూర్తి = 0;
      rc = where_Range_Skip_పరిశీలన_Est(pParse, p_క్రింది, p_పై, pLoop, &b_పూర్తి);
      ఒకవేళ( b_పూర్తి ) తిరిగి rc;
    }
  }
#లేకపోతే
  వాడని_PARAMETER(pParse);
  వాడని_PARAMETER(pBuilder);
  ప్రతిజ్ఞా( p_క్రింది || p_పై );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p_పై==0 || (p_పై->wtFlags & పదం_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(p_క్రింది, n_బహిర్గతం);
  nNew = whereRangeAdjust(p_పై, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  ఒకవేళ( p_క్రింది && p_క్రింది->truthProb>0 && p_పై && p_పై->truthProb>0 ){
    nNew -= 20;
  }

  n_బహిర్గతం -= (p_క్రింది!=0) + (p_పై!=0);
  ఒకవేళ( nNew<10 ) nNew = 10;
  ఒకవేళ( nNew<n_బహిర్గతం ) n_బహిర్గతం = nNew;
#ఒకవేళ defined(WHERETRACE_ENABLED)
  ఒకవేళ( pLoop->n_బహిర్గతం>n_బహిర్గతం ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->n_బహిర్గతం, n_బహిర్గతం));
  }
#అయితే_ముగింపు
  pLoop->n_బహిర్గతం = (LogEst)n_బహిర్గతం;
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
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
స్థిర పూర్ణాంకం where_Equal_పరిశీలన_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  సమాసం *p_సమాసం,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pn_అడ్డువరుస       /* Write the revised row estimate here */
){
  సూచీ *p = pBuilder->pNew->u.btree.p_సూచీ;
  పూర్ణాంకం nEq = pBuilder->pNew->u.btree.nEq;
  Unpacked_రికార్డు *pRec = pBuilder->pRec;
  పూర్ణాంకం rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  పూర్ణాంకం bOk;

  ప్రతిజ్ఞా( nEq>=1 );
  ప్రతిజ్ఞా( nEq<=p->n_స్తంభం );
  ప్రతిజ్ఞా( p->aSample!=0 );
  ప్రతిజ్ఞా( p->nSample>0 );
  ప్రతిజ్ఞా( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  ఒకవేళ( pBuilder->nRecValid<(nEq-1) ){
    తిరిగి SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  ఒకవేళ( nEq>=p->n_స్తంభం ){
    *pn_అడ్డువరుస = 1;
    తిరిగి SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_సమితి_విలువ(pParse, p, &pRec, p_సమాసం, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  ఒకవేళ( bOk==0 ) తిరిగి SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_కీలకం_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (పూర్ణాంకం)a[1]));
  *pn_అడ్డువరుస = a[1];
 
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
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
స్థిర పూర్ణాంకం where_లోపల_పరిశీలన_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  సమాసం_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pn_అడ్డువరుస       /* Write the revised row estimate here */
){
  సూచీ *p = pBuilder->pNew->u.btree.p_సూచీ;
  i64 nRow0 = sqlite3_Log_Est_గమ్యం_Int(p->ai_అడ్డువరుస_Log_Est[0]);
  పూర్ణాంకం nRecValid = pBuilder->nRecValid;
  పూర్ణాంకం rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt n_అడ్డువరుస_Est = 0;    /* New estimate of the number of rows */
  పూర్ణాంకం i;                  /* Loop counter */

  ప్రతిజ్ఞా( p->aSample!=0 );
  కొరకు(i=0; rc==SQLITE_OK && i<pList->n_సమాసం; i++){
    nEst = nRow0;
    rc = where_Equal_పరిశీలన_Est(pParse, pBuilder, pList->a[i].p_సమాసం, &nEst);
    n_అడ్డువరుస_Est += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( n_అడ్డువరుస_Est > (tRowcnt)nRow0 ) n_అడ్డువరుస_Est = nRow0;
    *pn_అడ్డువరుస = n_అడ్డువరుస_Est;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", n_అడ్డువరుస_Est));
  }
  ప్రతిజ్ఞా( pBuilder->nRecValid==nRecValid );
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */


#ఒకవేళ defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
శూన్య sqlite3_Where_పదం_Print(Where_పదం *p_పదం, పూర్ణాంకం i_పదం){
  ఒకవేళ( p_పదం==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", i_పదం);
  }లేకపోతే{
    అక్షరం zType[8];
    అక్షరం z_ఎడమ[50];
    జ్ఞాపకం_నకలు(zType, "....", 5);
    ఒకవేళ( p_పదం->wtFlags & పదం_VIRTUAL ) zType[0] = 'V';
    ఒకవేళ( p_పదం->eOperator & WO_EQUIV  ) zType[1] = 'E';
    ఒకవేళ( సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON) ) zType[2] = 'L';
    ఒకవేళ( p_పదం->wtFlags & పదం_CODED  ) zType[3] = 'C';
    ఒకవేళ( p_పదం->eOperator & WO_SINGLE ){
      ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(పరిమాణం(z_ఎడమ),z_ఎడమ,"left={%d:%d}",
                       p_పదం->ఎడమ_Cursor, p_పదం->u.x.ఎడమ_స్తంభం);
    }లేకపోతే ఒకవేళ( (p_పదం->eOperator & WO_OR)!=0 && p_పదం->u.pOrInfo!=0 ){
      sqlite3_snprintf(పరిమాణం(z_ఎడమ),z_ఎడమ,"indexable=0x%llx",
                       p_పదం->u.pOrInfo->indexable);
    }లేకపోతే{
      sqlite3_snprintf(పరిమాణం(z_ఎడమ),z_ఎడమ,"left=%d", p_పదం->ఎడమ_Cursor);
    }
    i_పదం = p_పదం->i_పదం = MAX(i_పదం,p_పదం->i_పదం);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       i_పదం, p_పదం, zType, z_ఎడమ, p_పదం->eOperator, p_పదం->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    ఒకవేళ( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        p_పదం->truthProb, (u64)p_పదం->prereqAll, (u64)p_పదం->prereq_కుడి);
    }
    ఒకవేళ( (p_పదం->eOperator & (WO_OR|WO_AND))==0 && p_పదం->u.x.i_క్షేత్రం ){
      sqlite3DebugPrintf(" iField=%d", p_పదం->u.x.i_క్షేత్రం);
    }
    ఒకవేళ( p_పదం->i_మాతృ>=0 ){
      sqlite3DebugPrintf(" iParent=%d", p_పదం->i_మాతృ);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_వృక్షం_View_సమాసం(0, p_పదం->p_సమాసం, 0);
  }
}
శూన్య sqlite3_Show_Where_పదం(Where_పదం *p_పదం){
  sqlite3_Where_పదం_Print(p_పదం, 0);
}
#అయితే_ముగింపు

#నిర్వచితమైతే WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
శూన్య sqlite3WhereClausePrint(WhereClause *pWC){
  పూర్ణాంకం i;
  కొరకు(i=0; i<pWC->n_పదం; i++){
    sqlite3_Where_పదం_Print(&pWC->a[i], i);
  }
}
#అయితే_ముగింపు

#నిర్వచితమైతే WHERETRACE_ENABLED
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
శూన్య sqlite3WhereLoopPrint(స్థిరరాశి WhereLoop *p, స్థిరరాశి WhereClause *pWC){
  WhereInfo *pWInfo;
  ఒకవేళ( pWC ){
    పూర్ణాంకం nb;
    Src_అంశం *p_అంశం;
    పట్టిక *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    p_అంశం = pWInfo->pTabList->a + p->iTab;
    pTab = p_అంశం->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->ముసుగు_Self, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       p_అంశం->zAlias ? p_అంశం->zAlias : pTab->zName);
  }లేకపోతే{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->ముసుగు_Self, p->prereq & 0xfff, p->cId, p->iTab);
  }
  ఒకవేళ( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    స్థిరరాశి అక్షరం *zName;
    ఒకవేళ( p->u.btree.p_సూచీ && (zName = p->u.btree.p_సూచీ->zName)!=0 ){
      ఒకవేళ( అక్షరశ్రేణి_పోలిక(zName, "sqlite_autoindex_", 17)==0 ){
        పూర్ణాంకం i = sqlite3Strlen30(zName) - 1;
        అయితే( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }లేకపోతే{
      sqlite3DebugPrintf("%20s","");
    }
  }లేకపోతే{
    అక్షరం *z;
    ఒకవేళ( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_అంకె, p->u.vtab.idxStr, p->u.vtab.omit_ముసుగు);
    }లేకపోతే{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_అంకె, p->u.vtab.omit_ముసుగు);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  ఒకవేళ( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }లేకపోతే{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  ఒకవేళ( pWInfo && pWInfo->b_నక్షత్రం_వాడిన && p->r_నక్షత్రం_Delta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->r_అమరిక, p->r_నిరంతరం, p->n_బహిర్గతం, p->r_నక్షత్రం_Delta);
  }లేకపోతే{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->r_అమరిక, p->r_నిరంతరం, p->n_బహిర్గతం);
  }
  ఒకవేళ( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->nLTerm; i++){
      sqlite3_Where_పదం_Print(p->aLTerm[i], i);
    }
  }
}
శూన్య sqlite3ShowWhereLoop(స్థిరరాశి WhereLoop *p){
  ఒకవేళ( p ) sqlite3WhereLoopPrint(p, 0);
}
శూన్య sqlite3ShowWhereLoopList(స్థిరరాశి WhereLoop *p){
  అయితే( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_తదుపరి_Loop;
  }
}
#అయితే_ముగింపు

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
స్థిర శూన్య whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_కొలత(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
స్థిర శూన్య whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  ఒకవేళ( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_సూచీ) ){
    ఒకవేళ( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }లేకపోతే ఒకవేళ( (p->wsFlags & WHERE_AUTO_సూచీ)!=0 && p->u.btree.p_సూచీ!=0 ){
      sqlite3DbFree(db, p->u.btree.p_సూచీ->z_నిలువు_Aff);
      sqlite3DbFreeNN(db, p->u.btree.p_సూచీ);
      p->u.btree.p_సూచీ = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
స్థిర శూన్య whereLoopClear(sqlite3 *db, WhereLoop *p){
  ఒకవేళ( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_కొలత(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
స్థిర పూర్ణాంకం whereLoopResize(sqlite3 *db, WhereLoop *p, పూర్ణాంకం n){
  Where_పదం **paNew;
  ఒకవేళ( p->nLSlot>=n ) తిరిగి SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, పరిమాణం(p->aLTerm[0])*n);
  ఒకవేళ( paNew==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  జ్ఞాపకం_నకలు(paNew, p->aLTerm, పరిమాణం(p->aLTerm[0])*p->nLSlot);
  ఒకవేళ( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  తిరిగి SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
స్థిర పూర్ణాంకం whereLoopXfer(sqlite3 *db, WhereLoop *p_గమ్యం, WhereLoop *pFrom){
  whereLoopClearUnion(db, p_గమ్యం);
  ఒకవేళ( pFrom->nLTerm > p_గమ్యం->nLSlot
   && whereLoopResize(db, p_గమ్యం, pFrom->nLTerm)
  ){
    memset(p_గమ్యం, 0, WHERE_LOOP_XFER_SZ);
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  జ్ఞాపకం_నకలు(p_గమ్యం, pFrom, WHERE_LOOP_XFER_SZ);
  జ్ఞాపకం_నకలు(p_గమ్యం->aLTerm, pFrom->aLTerm, p_గమ్యం->nLTerm*పరిమాణం(p_గమ్యం->aLTerm[0]));
  ఒకవేళ( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }లేకపోతే ఒకవేళ( (pFrom->wsFlags & WHERE_AUTO_సూచీ)!=0 ){
    pFrom->u.btree.p_సూచీ = 0;
  }
  తిరిగి SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
స్థిర శూన్య whereLoopDelete(sqlite3 *db, WhereLoop *p){
  ప్రతిజ్ఞా( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
స్థిర శూన్య whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  ప్రతిజ్ఞా( pWInfo!=0 );
  ప్రతిజ్ఞా( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  అయితే( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_తదుపరి_Loop;
    whereLoopDelete(db, p);
  }
  అయితే( pWInfo->p_Mem_గమ్యం_Free ){
    Where_Mem_ఖండం *p_తదుపరి = pWInfo->p_Mem_గమ్యం_Free->p_తదుపరి;
    sqlite3DbNNFreeNN(db, pWInfo->p_Mem_గమ్యం_Free);
    pWInfo->p_Mem_గమ్యం_Free = p_తదుపరి;
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
స్థిర పూర్ణాంకం whereLoopCheaperProperSubset(
  స్థిరరాశి WhereLoop *pX,       /* First WhereLoop to compare */
  స్థిరరాశి WhereLoop *pY        /* Compare against this WhereLoop */
){
  పూర్ణాంకం i, j;
  ఒకవేళ( pX->r_నిరంతరం>pY->r_నిరంతరం && pX->n_బహిర్గతం>pY->n_బహిర్గతం ) తిరిగి 0; /* (1d) and (2a) */
  ప్రతిజ్ఞా( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  ప్రతిజ్ఞా( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  ఒకవేళ( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_సూచీ==pY->u.btree.p_సూచీ             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    తిరిగి 1;  /* Case 1 is true */
  }
  ఒకవేళ( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    తిరిగి 0;                                            /* (2b) */
  }
  ఒకవేళ( pY->nSkip > pX->nSkip ) తిరిగి 0;                  /* (2d) */
  కొరకు(i=pX->nLTerm-1; i>=0; i--){
    ఒకవేళ( pX->aLTerm[i]==0 ) కొనసాగు;
    కొరకు(j=pY->nLTerm-1; j>=0; j--){
      ఒకవేళ( pY->aLTerm[j]==pX->aLTerm[i] ) ఆపు;
    }
    ఒకవేళ( j<0 ) తిరిగి 0;                                  /* (2c) */
  }
  ఒకవేళ( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    తిరిగి 0;                                            /* (2e) */
  }
  తిరిగి 1;  /* Case 2 is true */
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
స్థిర శూన్య where_Loop_Adjust_ఖర్చు(స్థిరరాశి WhereLoop *p, WhereLoop *pTemplate){
  ఒకవేళ( (pTemplate->wsFlags & WHERE_సూచీకృత)==0 ) తిరిగి;
  కొరకు(; p; p=p->p_తదుపరి_Loop){
    ఒకవేళ( p->iTab!=pTemplate->iTab ) కొనసాగు;
    ఒకవేళ( (p->wsFlags & WHERE_సూచీకృత)==0 ) కొనసాగు;
    ఒకవేళ( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_నిరంతరం, pTemplate->n_బహిర్గతం,
                       MIN(p->r_నిరంతరం, pTemplate->r_నిరంతరం),
                       MIN(p->n_బహిర్గతం - 1, pTemplate->n_బహిర్గతం)));
      pTemplate->r_నిరంతరం = MIN(p->r_నిరంతరం, pTemplate->r_నిరంతరం);
      pTemplate->n_బహిర్గతం = MIN(p->n_బహిర్గతం - 1, pTemplate->n_బహిర్గతం);
    }లేకపోతే ఒకవేళ( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_నిరంతరం, pTemplate->n_బహిర్గతం,
                       MAX(p->r_నిరంతరం, pTemplate->r_నిరంతరం),
                       MAX(p->n_బహిర్గతం + 1, pTemplate->n_బహిర్గతం)));
      pTemplate->r_నిరంతరం = MAX(p->r_నిరంతరం, pTemplate->r_నిరంతరం);
      pTemplate->n_బహిర్గతం = MAX(p->n_బహిర్గతం + 1, pTemplate->n_బహిర్గతం);
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
స్థిర WhereLoop **where_Loop_కనుగొను_Lesser(
  WhereLoop **ppPrev,
  స్థిరరాశి WhereLoop *pTemplate
){
  WhereLoop *p;
  కొరకు(p=(*ppPrev); p; ppPrev=&p->p_తదుపరి_Loop, p=*ppPrev){
    ఒకవేళ( p->iTab!=pTemplate->iTab || p->i_క్రమీకరించు_Idx!=pTemplate->i_క్రమీకరించు_Idx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      కొనసాగు;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    ప్రతిజ్ఞా( p->r_అమరిక==0 || pTemplate->r_అమరిక==0
                 || p->r_అమరిక==pTemplate->r_అమరిక );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    ప్రతిజ్ఞా( p->r_అమరిక>=pTemplate->r_అమరిక );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    ఒకవేళ( (p->wsFlags & WHERE_AUTO_సూచీ)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_సూచీకృత)!=0
     && (pTemplate->wsFlags & WHERE_స్తంభం_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      ఆపు;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    ఒకవేళ( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->r_అమరిక<=pTemplate->r_అమరిక                  /* (2a) */
     && p->r_నిరంతరం<=pTemplate->r_నిరంతరం                      /* (2b) */
     && p->n_బహిర్గతం<=pTemplate->n_బహిర్గతం                      /* (2c) */
    ){
      తిరిగి 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    ఒకవేళ( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->r_నిరంతరం>=pTemplate->r_నిరంతరం                             /* (2a) */
     && p->n_బహిర్గతం>=pTemplate->n_బహిర్గతం                             /* (2b) */
    ){
      ప్రతిజ్ఞా( p->r_అమరిక>=pTemplate->r_అమరిక ); /* SETUP-INVARIANT above */
      ఆపు;   /* Cause p to be overwritten by pTemplate */
    }
  }
  తిరిగి ppPrev;
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
స్థిర పూర్ణాంకం where_Loop_చొప్పించు(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  పూర్ణాంకం rc;

  /* Stop the search once we hit the query planner search limit */
  ఒకవేళ( pBuilder->i_Plan_హద్దు==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    ఒకవేళ( pBuilder->p_Or_సమితి ) pBuilder->p_Or_సమితి->n = 0;
    తిరిగి SQLITE_పూర్తి;
  }
  pBuilder->i_Plan_హద్దు--;

  where_Loop_Adjust_ఖర్చు(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  ఒకవేళ( pBuilder->p_Or_సమితి!=0 ){
    ఒకవేళ( pTemplate->nLTerm ){
#ఒకవేళ WHERETRACE_ENABLED
      u16 n = pBuilder->p_Or_సమితి->n;
      పూర్ణాంకం x =
#అయితే_ముగింపు
      where_Or_చొప్పించు(pBuilder->p_Or_సమితి, pTemplate->prereq, pTemplate->r_నిరంతరం,
                                    pTemplate->n_బహిర్గతం);
#ఒకవేళ WHERETRACE_ENABLED /* 0x8 */
      ఒకవేళ( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#అయితే_ముగింపు
    }
    తిరిగి SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = where_Loop_కనుగొను_Lesser(&pWInfo->pLoops, pTemplate);

  ఒకవేళ( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#ఒకవేళ WHERETRACE_ENABLED /* 0x8 */
    ఒకవేళ( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#అయితే_ముగింపు
    తిరిగి SQLITE_OK; 
  }లేకపోతే{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#ఒకవేళ WHERETRACE_ENABLED /* 0x8 */
  ఒకవేళ( sqlite3WhereTrace & 0x8 ){
    ఒకవేళ( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }లేకపోతే{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#అయితే_ముగింపు
  ఒకవేళ( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, పరిమాణం(WhereLoop));
    ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_తదుపరి_Loop = 0;
  }లేకపోతే{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_తోక = &p->p_తదుపరి_Loop;
    WhereLoop *p_గమ్యం_Del;
    అయితే( *pp_తోక ){
      pp_తోక = where_Loop_కనుగొను_Lesser(pp_తోక, pTemplate);
      ఒకవేళ( pp_తోక==0 ) ఆపు;
      p_గమ్యం_Del = *pp_తోక;
      ఒకవేళ( p_గమ్యం_Del==0 ) ఆపు;
      *pp_తోక = p_గమ్యం_Del->p_తదుపరి_Loop;
#ఒకవేళ WHERETRACE_ENABLED /* 0x8 */
      ఒకవేళ( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(p_గమ్యం_Del, pBuilder->pWC);
      }
#అయితే_ముగింపు
      whereLoopDelete(db, p_గమ్యం_Del);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  ఒకవేళ( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    సూచీ *p_సూచీ = p->u.btree.p_సూచీ;
    ఒకవేళ( p_సూచీ && p_సూచీ->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_సూచీ = 0;
    }
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం సమాసం_కణుపు_నమూనా_నిడివి_Est(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_STRING ){
    పూర్ణాంకం sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_సమాసం->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    ఒకవేళ( pWalker->e_సంకేతం ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }లేకపోతే{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    అయితే( (c = *(z++))!=0 ){
      ఒకవేళ( c==c3 ){
        ఒకవేళ( *z ) z++;
        అయితే( *z && *z!=']' ) z++;
      }లేకపోతే ఒకవేళ( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    ఒకవేళ( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  తిరిగి WRC_Continue;
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
స్థిర పూర్ణాంకం est_Like_నమూనా_నిడివి(సమాసం *p, u16 e_సంకేతం){
  Walker w;
  w.u.sz = 0;
  w.e_సంకేతం = e_సంకేతం;
  w.x_సమాసం_Callback = సమాసం_కణుపు_నమూనా_నిడివి_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#నిర్వచితమైతే SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#అయితే_ముగింపు
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.u.sz;
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
స్థిర శూన్య whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst n_అడ్డువరుస            /* Number of rows in the entire table */
){
  Where_పదం *p_పదం, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->ముసుగు_Self);
  పూర్ణాంకం i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  ఒకవేళ( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->p_క్రమం_By==0 ){
    తిరిగి;
  }

  ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_AUTO_సూచీ)==0 );
  కొరకు(i=pWC->n_పునాది, p_పదం=pWC->a; i>0; i--, p_పదం++){
    ప్రతిజ్ఞా( p_పదం!=0 );
    ఒకవేళ( (p_పదం->prereqAll & notAllowed)!=0 ) కొనసాగు;
    ఒకవేళ( (p_పదం->prereqAll & pLoop->ముసుగు_Self)==0 ) కొనసాగు;
    ఒకవేళ( (p_పదం->wtFlags & పదం_VIRTUAL)!=0 ) కొనసాగు;
    కొరకు(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      ఒకవేళ( pX==0 ) కొనసాగు;
      ఒకవేళ( pX==p_పదం ) ఆపు;
      ఒకవేళ( pX->i_మాతృ>=0 && (&pWC->a[pX->i_మాతృ])==p_పదం ) ఆపు;
    }
    ఒకవేళ( j<0 ){
      sqlite3_పురోగతి_Check(pWC->pWInfo->pParse);
      ఒకవేళ( pLoop->ముసుగు_Self==p_పదం->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        ఒకవేళ( (p_పదం->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_ఎడమ|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      ఒకవేళ( p_పదం->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->n_బహిర్గతం += p_పదం->truthProb;
      }లేకపోతే{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        సమాసం *p_Op_సమాసం = p_పదం->p_సమాసం;
        pLoop->n_బహిర్గతం--;
        ఒకవేళ( (p_పదం->eOperator&(WO_EQ|WO_IS))!=0
         && (p_పదం->wtFlags & పదం_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          సమాసం *p_కుడి = p_Op_సమాసం->p_కుడి;
          Parse *pParse = pWC->pWInfo->pParse;
          పూర్ణాంకం k = 0;
          testcase( p_Op_సమాసం->op==TK_IS );
          ఒకవేళ( sqlite3_సమాసం_Is_Integer(p_కుడి, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }లేకపోతే{
            k = 20;
          }
          ఒకవేళ( iReduce<k ){
            p_పదం->wtFlags |= పదం_HEURTRUTH;
            iReduce = k;
          }
        }లేకపోతే
        ఒకవేళ( సమాసం_Has_Property(p_Op_సమాసం, EP_InfixFunc) 
         && p_Op_సమాసం->op==TK_FUNCTION
        ){
          పూర్ణాంకం eOp;
          ప్రతిజ్ఞా( సమాసం_Use_XList(p_Op_సమాసం) );
          ప్రతిజ్ఞా( p_Op_సమాసం->x.pList->n_సమాసం>=2 );
          eOp = sqlite3_సమాసం_Is_Like_Operator(p_Op_సమాసం);
          ఒకవేళ( ALWAYS(eOp>0) ){
            పూర్ణాంకం sz_నమూనా;
            సమాసం *pRHS = p_Op_సమాసం->x.pList->a[0].p_సమాసం;
            eOp = eOp==SQLITE_సూచీ_CONSTRAINT_LIKE;
            sz_నమూనా = est_Like_నమూనా_నిడివి(pRHS, eOp);
            ఒకవేళ( sz_నమూనా>0 ){
              pLoop->n_బహిర్గతం -= sz_నమూనా*2;
            }
          }
        }
      }
    }
  }
  ఒకవేళ( pLoop->n_బహిర్గతం > n_అడ్డువరుస-iReduce ){
    pLoop->n_బహిర్గతం = n_అడ్డువరుస - iReduce;
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
స్థిర పూర్ణాంకం where_Range_సదిశ_Len(
  Parse *pParse,       /* Parsing context */
  పూర్ణాంకం iCur,            /* Cursor open on pIdx */
  సూచీ *pIdx,         /* The index to be used for a inequality constraint */
  పూర్ణాంకం nEq,             /* Number of prior equality constraints on same index */
  Where_పదం *p_పదం     /* The vector inequality constraint */
){
  పూర్ణాంకం nCmp = sqlite3_సమాసం_సదిశ_కొలత(p_పదం->p_సమాసం->p_ఎడమ);
  పూర్ణాంకం i;

  nCmp = MIN(nCmp, (pIdx->n_స్తంభం - nEq));
  కొరకు(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    అక్షరం aff;                     /* Comparison affinity */
    అక్షరం idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    సమాసం *pLhs, *pRhs;

    ప్రతిజ్ఞా( సమాసం_Use_XList(p_పదం->p_సమాసం->p_ఎడమ) );
    pLhs = p_పదం->p_సమాసం->p_ఎడమ->x.pList->a[i].p_సమాసం;
    pRhs = p_పదం->p_సమాసం->p_కుడి;
    ఒకవేళ( సమాసం_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_సమాసం;
    }లేకపోతే{
      pRhs = pRhs->x.pList->a[i].p_సమాసం;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    ఒకవేళ( pLhs->op!=TK_స్తంభం
     || pLhs->i_పట్టిక!=iCur
     || pLhs->i_స్తంభం!=pIdx->ai_స్తంభం[i+nEq]
     || pIdx->a_క్రమీకరించు_క్రమం[i+nEq]!=pIdx->a_క్రమీకరించు_క్రమం[nEq]
    ){
      ఆపు;
    }

    testcase( pLhs->i_స్తంభం==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_సమాసం_Affinity(pLhs));
    idxaff = sqlite3_పట్టిక_స్తంభం_Affinity(pIdx->p_పట్టిక, pLhs->i_స్తంభం);
    ఒకవేళ( aff!=idxaff ) ఆపు;

    ఒకవేళ( సమాసం_Has_Property(p_పదం->p_సమాసం, EP_Commuted) ) మార్పిడి(సమాసం*, pRhs, pLhs);
    pColl = sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, pLhs, pRhs);
    ఒకవేళ( pColl==0 ) ఆపు;
    ఒకవేళ( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) ఆపు;
  }
  తిరిగి i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#నిర్వచితమైతే SQLITE_ENABLE_COSTMULT
# define Apply_ఖర్చు_Multiplier(C,T)  C += T
#లేకపోతే
# define Apply_ఖర్చు_Multiplier(C,T)
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం where_Loop_కూడు_Btree_సూచీ(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  Src_అంశం *pSrc,                  /* FROM clause term being analyzed */
  సూచీ *pProbe,                  /* An index on pSrc */
  LogEst n_లోపల_Mul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  Where_పదం *p_పదం;               /* A WhereTerm under consideration */
  పూర్ణాంకం op_ముసుగు;                     /* Valid operators for constraints */
  Where_పరిశీలన పరిశీలన;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_శిఖరం;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_n_బహిర్గతం;              /* Original value of pNew->nOut */
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  LogEst r_కొలత;                   /* Number of rows in the table */
  LogEst r_Log_కొలత;                /* Logarithm of table size */
  Where_పదం *p_శిఖరం = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  ప్రతిజ్ఞా( db->mallocFailed==0 || pParse->nErr>0 );
  ఒకవేళ( pParse->nErr ){
    తిరిగి pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->p_పట్టిక->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->r_నిరంతరం));

  ప్రతిజ్ఞా( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  ప్రతిజ్ఞా( (pNew->wsFlags & WHERE_శిఖరం_హద్దు)==0 );
  ఒకవేళ( pNew->wsFlags & WHERE_BTM_హద్దు ){
    op_ముసుగు = WO_LT|WO_LE;
  }లేకపోతే{
    ప్రతిజ్ఞా( pNew->u.btree.nBtm==0 );
    op_ముసుగు = WO_EQ|WO_లోపల|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  ఒకవేళ( pProbe->bUnordered ){
    op_ముసుగు &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  ప్రతిజ్ఞా( pNew->u.btree.nEq<pProbe->n_స్తంభం );
  ప్రతిజ్ఞా( pNew->u.btree.nEq<pProbe->n_కీలకం_నిలువు
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_శిఖరం = pNew->u.btree.n_శిఖరం;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_n_బహిర్గతం = pNew->n_బహిర్గతం;
  p_పదం = where_పరిశీలన_Init(&పరిశీలన, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        op_ముసుగు, pProbe);
  pNew->r_అమరిక = 0;
  r_కొలత = pProbe->ai_అడ్డువరుస_Log_Est[0];
  r_Log_కొలత = estLog(r_కొలత);
  కొరకు(; rc==SQLITE_OK && p_పదం!=0; p_పదం = where_పరిశీలన_తదుపరి(&పరిశీలన)){
    u16 eOp = p_పదం->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst r_ఖర్చు_Idx;
    LogEst n_బహిర్గతం_Unadjusted;        /* nOut before IN() and WHERE adjustments */
    పూర్ణాంకం n_లోపల = 0;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    పూర్ణాంకం nRecValid = pBuilder->nRecValid;
#అయితే_ముగింపు
    ఒకవేళ( (eOp==WO_ISNULL || (p_పదం->wtFlags&పదం_VNULL)!=0)
     && సూచీ_స్తంభం_Not_Null(pProbe, saved_nEq)
    ){
      కొనసాగు; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    ఒకవేళ( p_పదం->prereq_కుడి & pNew->ముసుగు_Self ) కొనసాగు;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    ఒకవేళ( p_పదం->wtFlags & పదం_LIKEOPT && p_పదం->eOperator==WO_LT ) కొనసాగు;

    ఒకవేళ( (pSrc->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి))!=0
     && !constraintCompatibleWithOuterJoin(p_పదం,pSrc)
    ){
      కొనసాగు;
    }
    ఒకవేళ( Is_Unique_సూచీ(pProbe) && saved_nEq==pProbe->n_కీలకం_నిలువు-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }లేకపోతే{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_సూచీకృత;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_శిఖరం = saved_n_శిఖరం;
    pNew->nLTerm = saved_nLTerm;
    ఒకవేళ( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       ఆపు; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = p_పదం;
    pNew->prereq = (saved_prereq | p_పదం->prereq_కుడి) & ~pNew->ముసుగు_Self;

    ప్రతిజ్ఞా( n_లోపల_Mul==0
        || (pNew->wsFlags & WHERE_స్తంభం_NULL)!=0
        || (pNew->wsFlags & WHERE_స్తంభం_లోపల)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    ఒకవేళ( eOp & WO_లోపల ){
      సమాసం *p_సమాసం = p_పదం->p_సమాసం;
      ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        పూర్ణాంకం i;
        పూర్ణాంకం bRedundant = 0;
        n_లోపల = 46;  ప్రతిజ్ఞా( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        కొరకు(i=0; i<pNew->nLTerm-1; i++){
          ఒకవేళ( pNew->aLTerm[i] && pNew->aLTerm[i]->p_సమాసం==p_సమాసం ){
            n_లోపల = 0;
            ఒకవేళ( pNew->aLTerm[i]->u.x.i_క్షేత్రం == p_పదం->u.x.i_క్షేత్రం ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        ఒకవేళ( bRedundant ){
          pNew->nLTerm--;
          కొనసాగు;
        }
      }లేకపోతే ఒకవేళ( ALWAYS(p_సమాసం->x.pList && p_సమాసం->x.pList->n_సమాసం) ){
        /* "x IN (value, value, ...)" */
        n_లోపల = sqlite3LogEst(p_సమాసం->x.pList->n_సమాసం);
      }
      ఒకవేళ( pProbe->hasStat1 && r_Log_కొలత>=10 ){
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
        M = pProbe->ai_అడ్డువరుస_Log_Est[saved_nEq];
        logK = estLog(n_లోపల);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (n_లోపల + r_Log_కొలత);
        ఒకవేళ( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, n_లోపల, r_Log_కొలత, x));
        }లేకపోతే ఒకవేళ( n_లోపల_Mul<2 && OptimizationEnabled(db, SQLITE_Seek_పరిశీలన) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, n_లోపల, r_Log_కొలత, x, n_లోపల_Mul));
          pNew->wsFlags |= WHERE_లోపల_SEEKSCAN;
        }లేకపోతే{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, n_లోపల, r_Log_కొలత, x, n_లోపల_Mul));
          కొనసాగు;
        }
      }
      pNew->wsFlags |= WHERE_స్తంభం_లోపల;
    }లేకపోతే ఒకవేళ( eOp & (WO_EQ|WO_IS) ){
      పూర్ణాంకం i_నిలువు = pProbe->ai_స్తంభం[saved_nEq];
      pNew->wsFlags |= WHERE_స్తంభం_EQ;
      ప్రతిజ్ఞా( saved_nEq==pNew->u.btree.nEq );
      ఒకవేళ( i_నిలువు==XN_ROWID
       || (i_నిలువు>=0 && n_లోపల_Mul==0 && saved_nEq==pProbe->n_కీలకం_నిలువు-1)
      ){
        ఒకవేళ( i_నిలువు==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_కీలకం_నిలువు==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }లేకపోతే{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      ఒకవేళ( పరిశీలన.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }లేకపోతే ఒకవేళ( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_స్తంభం_NULL;
    }లేకపోతే{
      పూర్ణాంకం nVecLen = where_Range_సదిశ_Len(
          pParse, pSrc->iCursor, pProbe, saved_nEq, p_పదం
      );
      ఒకవేళ( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_స్తంభం_RANGE|WHERE_BTM_హద్దు;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = p_పదం;
        p_శిఖరం = 0;
        ఒకవేళ( p_పదం->wtFlags & పదం_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_శిఖరం = &p_పదం[1];
          ప్రతిజ్ఞా( (p_శిఖరం-(p_పదం->pWC->a))<p_పదం->pWC->n_పదం );
          ప్రతిజ్ఞా( p_శిఖరం->wtFlags & పదం_LIKEOPT );
          ప్రతిజ్ఞా( p_శిఖరం->eOperator==WO_LT );
          ఒకవేళ( whereLoopResize(db, pNew, pNew->nLTerm+1) ) ఆపు; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_శిఖరం;
          pNew->wsFlags |= WHERE_శిఖరం_హద్దు;
          pNew->u.btree.n_శిఖరం = 1;
        }
      }లేకపోతే{
        ప్రతిజ్ఞా( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_స్తంభం_RANGE|WHERE_శిఖరం_హద్దు;
        pNew->u.btree.n_శిఖరం = nVecLen;
        p_శిఖరం = p_పదం;
        pBtm = (pNew->wsFlags & WHERE_BTM_హద్దు)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    ప్రతిజ్ఞా( pNew->n_బహిర్గతం==saved_n_బహిర్గతం );
    ఒకవేళ( pNew->wsFlags & WHERE_స్తంభం_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      where_Range_పరిశీలన_Est(pParse, pBuilder, pBtm, p_శిఖరం, pNew);
    }లేకపోతే{
      పూర్ణాంకం nEq = ++pNew->u.btree.nEq;
      ప్రతిజ్ఞా( eOp & (WO_ISNULL|WO_EQ|WO_లోపల|WO_IS) );

      ప్రతిజ్ఞా( pNew->n_బహిర్గతం==saved_n_బహిర్గతం );
      ఒకవేళ( p_పదం->truthProb<=0 && pProbe->ai_స్తంభం[saved_nEq]>=0 ){
        ప్రతిజ్ఞా( (eOp & WO_లోపల) || n_లోపల==0 );
        testcase( eOp & WO_లోపల );
        pNew->n_బహిర్గతం += p_పదం->truthProb;
        pNew->n_బహిర్గతం -= n_లోపల;
      }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
        tRowcnt n_బహిర్గతం = 0;
        ఒకవేళ( n_లోపల_Mul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->n_Sample_నిలువు)
         && ((eOp & WO_లోపల)==0 || సమాసం_Use_XList(p_పదం->p_సమాసం))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          సమాసం *p_సమాసం = p_పదం->p_సమాసం;
          ఒకవేళ( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = where_Equal_పరిశీలన_Est(pParse, pBuilder, p_సమాసం->p_కుడి, &n_బహిర్గతం);
          }లేకపోతే{
            rc = where_లోపల_పరిశీలన_Est(pParse, pBuilder, p_సమాసం->x.pList, &n_బహిర్గతం);
          }
          ఒకవేళ( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          ఒకవేళ( rc!=SQLITE_OK ) ఆపు;          /* Jump out of the pTerm loop */
          ఒకవేళ( n_బహిర్గతం ){
            pNew->n_బహిర్గతం = sqlite3LogEst(n_బహిర్గతం);
            ఒకవేళ( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->n_బహిర్గతం+10 > pProbe->ai_అడ్డువరుస_Log_Est[0]
            ){
#ఒకవేళ WHERETRACE_ENABLED /* 0x01 */
              ఒకవేళ( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3_Where_పదం_Print(p_పదం, 999);
              }
#అయితే_ముగింపు
              p_పదం->wtFlags |= పదం_HIGHTRUTH;
              ఒకవేళ( p_పదం->wtFlags & పదం_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            ఒకవేళ( pNew->n_బహిర్గతం>saved_n_బహిర్గతం ) pNew->n_బహిర్గతం = saved_n_బహిర్గతం;
            pNew->n_బహిర్గతం -= n_లోపల;
          }
        }
        ఒకవేళ( n_బహిర్గతం==0 )
#అయితే_ముగింపు
        {
          pNew->n_బహిర్గతం += (pProbe->ai_అడ్డువరుస_Log_Est[nEq] - pProbe->ai_అడ్డువరుస_Log_Est[nEq-1]);
          ఒకవేళ( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->n_బహిర్గతం += 10;
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
    ప్రతిజ్ఞా( pSrc->pSTab->sz_Tab_అడ్డువరుస>0 );
    ఒకవేళ( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      r_ఖర్చు_Idx = pNew->n_బహిర్గతం + 16;
    }లేకపోతే{
      r_ఖర్చు_Idx = pNew->n_బహిర్గతం + 1 + (15*pProbe->sz_Idx_అడ్డువరుస)/pSrc->pSTab->sz_Tab_అడ్డువరుస;
    }
    r_ఖర్చు_Idx = sqlite3_Log_Est_కూడు(r_Log_కొలత, r_ఖర్చు_Idx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->r_నిరంతరం = r_ఖర్చు_Idx;
    ఒకవేళ( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->r_నిరంతరం = sqlite3_Log_Est_కూడు(pNew->r_నిరంతరం, pNew->n_బహిర్గతం + 16);
    }
    Apply_ఖర్చు_Multiplier(pNew->r_నిరంతరం, pProbe->p_పట్టిక->ఖర్చు_Mult);

    n_బహిర్గతం_Unadjusted = pNew->n_బహిర్గతం;
    pNew->r_నిరంతరం += n_లోపల_Mul + n_లోపల;
    pNew->n_బహిర్గతం += n_లోపల_Mul + n_లోపల;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_కొలత);
    ఒకవేళ( pSrc->fg.fromExists ) pNew->n_బహిర్గతం = 0;
    rc = where_Loop_చొప్పించు(pBuilder, pNew);

    ఒకవేళ( pNew->wsFlags & WHERE_స్తంభం_RANGE ){
      pNew->n_బహిర్గతం = saved_n_బహిర్గతం;
    }లేకపోతే{
      pNew->n_బహిర్గతం = n_బహిర్గతం_Unadjusted;
    }

    ఒకవేళ( (pNew->wsFlags & WHERE_శిఖరం_హద్దు)==0
     && pNew->u.btree.nEq<pProbe->n_స్తంభం
     && (pNew->u.btree.nEq<pProbe->n_కీలకం_నిలువు ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      ఒకవేళ( pNew->u.btree.nEq>3 ){
        sqlite3_పురోగతి_Check(pParse);
      }
      where_Loop_కూడు_Btree_సూచీ(pBuilder, pSrc, pProbe, n_లోపల_Mul+n_లోపల);
    }
    pNew->n_బహిర్గతం = saved_n_బహిర్గతం;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#అయితే_ముగింపు
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_శిఖరం = saved_n_శిఖరం;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->n_బహిర్గతం = saved_n_బహిర్గతం;
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
  ప్రతిజ్ఞా( 42==sqlite3LogEst(18) );
  ఒకవేళ( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_కీలకం_నిలువు
   && saved_nEq==pNew->nLTerm
   && pProbe->no_Skip_పరిశీలన==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_Skip_పరిశీలన)
   && pProbe->ai_అడ్డువరుస_Log_Est[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->ai_అడ్డువరుస_Log_Est[saved_nEq] - pProbe->ai_అడ్డువరుస_Log_Est[saved_nEq+1];
    pNew->n_బహిర్గతం -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    where_Loop_కూడు_Btree_సూచీ(pBuilder, pSrc, pProbe, nIter + n_లోపల_Mul);
    pNew->n_బహిర్గతం = saved_n_బహిర్గతం;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->p_పట్టిక->zName, pProbe->zName, saved_nEq, rc));
  తిరిగి rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
స్థిర పూర్ణాంకం సూచీ_Might_Help_With_క్రమం_By(
  WhereLoopBuilder *pBuilder,
  సూచీ *p_సూచీ,
  పూర్ణాంకం iCursor
){
  సమాసం_List *pOB;
  సమాసం_List *a_నిలువు_సమాసం;
  పూర్ణాంకం ii, jj;

  ఒకవేళ( p_సూచీ->bUnordered ) తిరిగి 0;
  ఒకవేళ( (pOB = pBuilder->pWInfo->p_క్రమం_By)==0 ) తిరిగి 0;
  కొరకు(ii=0; ii<pOB->n_సమాసం; ii++){
    సమాసం *p_సమాసం = sqlite3_సమాసం_Skip_Collate_And_Likely(pOB->a[ii].p_సమాసం);
    ఒకవేళ( NEVER(p_సమాసం==0) ) కొనసాగు;
    ఒకవేళ( (p_సమాసం->op==TK_స్తంభం || p_సమాసం->op==TK_AGG_స్తంభం) 
     && p_సమాసం->i_పట్టిక==iCursor 
    ){
      ఒకవేళ( p_సమాసం->i_స్తంభం<0 ) తిరిగి 1;
      కొరకు(jj=0; jj<p_సూచీ->n_కీలకం_నిలువు; jj++){
        ఒకవేళ( p_సమాసం->i_స్తంభం==p_సూచీ->ai_స్తంభం[jj] ) తిరిగి 1;
      }
    }లేకపోతే ఒకవేళ( (a_నిలువు_సమాసం = p_సూచీ->a_నిలువు_సమాసం)!=0 ){
      కొరకు(jj=0; jj<p_సూచీ->n_కీలకం_నిలువు; jj++){
        ఒకవేళ( p_సూచీ->ai_స్తంభం[jj]!=XN_సమాసం ) కొనసాగు;
        ఒకవేళ( sqlite3_సమాసం_Compare_Skip(p_సమాసం,a_నిలువు_సమాసం->a[jj].p_సమాసం,iCursor)==0 ){
          తిరిగి 1;
        }
      }
    }
  }
  తిరిగి 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
స్థిర పూర్ణాంకం where_Usable_Partial_సూచీ(
  పూర్ణాంకం iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  సమాసం *pWhere          /* The WHERE clause from the partial index */
){
  పూర్ణాంకం i;
  Where_పదం *p_పదం;
  Parse *pParse;

  ఒకవేళ( jointype & JT_LTORJ ) తిరిగి 0;
  pParse = pWC->pWInfo->pParse;
  అయితే( pWhere->op==TK_AND ){
    ఒకవేళ( !where_Usable_Partial_సూచీ(iTab,jointype,pWC,pWhere->p_ఎడమ) ) తిరిగి 0;
    pWhere = pWhere->p_కుడి;
  }
  కొరకు(i=0, p_పదం=pWC->a; i<pWC->n_పదం; i++, p_పదం++){
    సమాసం *p_సమాసం;
    p_సమాసం = p_పదం->p_సమాసం;
    ఒకవేళ( (!సమాసం_Has_Property(p_సమాసం, EP_OuterON) || p_సమాసం->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || సమాసం_Has_Property(p_సమాసం, EP_OuterON))
     && sqlite3_సమాసం_సూచిస్తుంది_సమాసం(pParse, p_సమాసం, pWhere, iTab)
     && !sqlite3_సమాసం_సూచిస్తుంది_సమాసం(pParse, p_సమాసం, pWhere, -1)
     && (p_పదం->wtFlags & పదం_VNULL)==0
    ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
స్థిర పూర్ణాంకం సమాసం_Is_Covered_By_సూచీ(
  స్థిరరాశి సమాసం *p_సమాసం,
  స్థిరరాశి సూచీ *pIdx,
  పూర్ణాంకం iTabCur
){
  పూర్ణాంకం i;
  కొరకు(i=0; i<pIdx->n_స్తంభం; i++){
    ఒకవేళ( pIdx->ai_స్తంభం[i]==XN_సమాసం
     && sqlite3_సమాసం_Compare(0, p_సమాసం, pIdx->a_నిలువు_సమాసం->a[i].p_సమాసం, iTabCur)==0
    ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
రకనిర్వచనం నిర్మాణం Covering_సూచీ_Check Covering_సూచీ_Check;
నిర్మాణం Covering_సూచీ_Check {
  సూచీ *pIdx;       /* The index */
  పూర్ణాంకం iTabCur;       /* Cursor number for the corresponding table */
  u8 b_సమాసం;          /* Uses an indexed expression */
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
స్థిర పూర్ణాంకం where_Is_Covering_సూచీ_Walk_Callback(Walker *pWalk, సమాసం *p_సమాసం){
  పూర్ణాంకం i;                    /* Loop counter */
  స్థిరరాశి సూచీ *pIdx;        /* The index of interest */
  స్థిరరాశి i16 *ai_స్తంభం;      /* Columns contained in the index */
  u16 n_స్తంభం;              /* Number of columns in the index */
  Covering_సూచీ_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  ఒకవేళ( (p_సమాసం->op==TK_స్తంభం || p_సమాసం->op==TK_AGG_స్తంభం) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    ఒకవేళ( p_సమాసం->i_పట్టిక!=pCk->iTabCur ) తిరిగి WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    ai_స్తంభం = pIdx->ai_స్తంభం;
    n_స్తంభం = pIdx->n_స్తంభం;
    కొరకు(i=0; i<n_స్తంభం; i++){
      ఒకవేళ( ai_స్తంభం[i]==p_సమాసం->i_స్తంభం ) తిరిగి WRC_Continue;
    }
    pCk->bUnidx = 1;
    తిరిగి WRC_Abort;
  }లేకపోతే ఒకవేళ( pIdx->b_Has_సమాసం
         && సమాసం_Is_Covered_By_సూచీ(p_సమాసం, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_సమాసం = 1;
    తిరిగి WRC_Prune;
  }
  తిరిగి WRC_Continue;
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
స్థిర SQLITE_NOINLINE u32 where_Is_Covering_సూచీ(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  సూచీ *pIdx,           /* Index that is being tested */
  పూర్ణాంకం iTabCur            /* Cursor for the table being indexed */
){
  పూర్ణాంకం i, rc;
  నిర్మాణం Covering_సూచీ_Check ck;
  Walker w;
  ఒకవేళ( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    తిరిగి 0;
  }
  ఒకవేళ( pIdx->b_Has_సమాసం==0 ){
    కొరకు(i=0; i<pIdx->n_స్తంభం; i++){
      ఒకవేళ( pIdx->ai_స్తంభం[i]>=BMS-1 ) ఆపు;
    }
    ఒకవేళ( i>=pIdx->n_స్తంభం ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      తిరిగి 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_సమాసం = 0;
  ck.bUnidx = 0;
  memset(&w, 0, పరిమాణం(w));
  w.x_సమాసం_Callback = where_Is_Covering_సూచీ_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  ఒకవేళ( ck.bUnidx ){
    rc = 0;
  }లేకపోతే ఒకవేళ( ck.b_సమాసం ){
    rc = WHERE_EXPRIDX;
  }లేకపోతే{
    rc = WHERE_IDX_ONLY;
  }
  తిరిగి rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
స్థిర శూన్య where_సూచీకృత_సమాసం_Cleanup(sqlite3 *db, శూన్య *pObject){
  సూచీకృత_సమాసం **pp = (సూచీకృత_సమాసం**)pObject;
  అయితే( *pp!=0 ){
    సూచీకృత_సమాసం *p = *pp;
    *pp = p->pIENext;
    sqlite3_సమాసం_Delete(db, p->p_సమాసం);
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
స్థిర శూన్య where_Part_Idx_సమాసం(
  Parse *pParse,                  /* Parse context */
  సూచీ *pIdx,                    /* Partial index being processed */
  సమాసం *pPart,                    /* WHERE clause being processed */
  Bitmask *p_ముసుగు,                 /* Mask to clear bits in */
  పూర్ణాంకం iIdxCur,                    /* Cursor number for index */
  Src_అంశం *p_అంశం                  /* The FROM clause entry for the table */
){
  ప్రతిజ్ఞా( p_అంశం==0 || (p_అంశం->fg.jointype & JT_కుడి)==0 );
  ప్రతిజ్ఞా( (p_అంశం==0 || p_ముసుగు==0) && (p_ముసుగు!=0 || p_అంశం!=0) );

  ఒకవేళ( pPart->op==TK_AND ){
    where_Part_Idx_సమాసం(pParse, pIdx, pPart->p_కుడి, p_ముసుగు, iIdxCur, p_అంశం);
    pPart = pPart->p_ఎడమ;
  }

  ఒకవేళ( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    సమాసం *p_ఎడమ = pPart->p_ఎడమ;
    సమాసం *p_కుడి = pPart->p_కుడి;
    పట్టిక *p_ఎడమ_Tab;            /* Table of the left operand */
    స్తంభం *p_ఎడమ_నిలువు;           /* Column of the left operand */
    u8 aff_కుడి, aff_ఎడమ;

    ఒకవేళ( p_ఎడమ->op!=TK_స్తంభం ) తిరిగి;
    ఒకవేళ( !sqlite3_సమాసం_Is_Constant(0, p_కుడి) ) తిరిగి;
    ఒకవేళ( !sqlite3_Is_ద్విమాన(sqlite3_సమాసం_Compare_Coll_Seq(pParse, pPart)) ) తిరిగి;
    ఒకవేళ( p_ఎడమ->i_స్తంభం<0 ) తిరిగి;
    p_ఎడమ_Tab = pIdx->p_పట్టిక;
    p_ఎడమ_నిలువు = &p_ఎడమ_Tab->a_నిలువు[p_ఎడమ->i_స్తంభం];
    aff_ఎడమ = p_ఎడమ_నిలువు->affinity;
    ప్రతిజ్ఞా( SQLITE_AFF_BLOB<SQLITE_AFF_పాఠ్యం );
    ఒకవేళ( aff_ఎడమ<SQLITE_AFF_పాఠ్యం ) తిరిగి;
    aff_కుడి = sqlite3_సమాసం_Affinity(p_కుడి);
    ఒకవేళ( aff_కుడి!=0 && aff_కుడి!=aff_ఎడమ ) తిరిగి;
    ఒకవేళ( p_అంశం ){
      sqlite3 *db = pParse->db;
      సూచీకృత_సమాసం *p = (సూచీకృత_సమాసం*)sqlite3DbMallocRaw(db, పరిమాణం(*p));
      ఒకవేళ( p ){
        పూర్ణాంకం b_Null_అడ్డువరుస = (p_అంశం->fg.jointype&(JT_ఎడమ|JT_LTORJ))!=0;
        p_కుడి = sqlite3_సమాసం_Dup(db, p_కుడి, 0);
        ఒకవేళ( (p_ఎడమ_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3_స్తంభం_Type(p_ఎడమ_నిలువు, 0);
          ప్రతిజ్ఞా( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(t.z);
          p->p_సమాసం = sqlite3_సమాసం_Alloc(db, TK_CAST, &t, 1);
          sqlite3_సమాసం_Attach_Subtrees(db, p->p_సమాసం, p_కుడి, 0);
        }లేకపోతే{
          p->p_సమాసం = p_కుడి;
        }
        p->i_దత్తాంశం_Cur = p_అంశం->iCursor;
        p->iIdxCur = iIdxCur;
        p->i_Idx_నిలువు = p_ఎడమ->i_స్తంభం;
        p->b_Maybe_Null_అడ్డువరుస = b_Null_అడ్డువరుస;
        p->pIENext = pParse->p_Idx_Part_సమాసం;
        p->aff = aff_ఎడమ;
        pParse->p_Idx_Part_సమాసం = p;
        ఒకవేళ( p->pIENext==0 ){
          శూన్య *pArg = (శూన్య*)&pParse->p_Idx_Part_సమాసం;
          sqlite3_Parser_కూడు_Cleanup(pParse, where_సూచీకృత_సమాసం_Cleanup, pArg);
        }
      }
    }లేకపోతే ఒకవేళ( p_ఎడమ->i_స్తంభం<(BMS-1) ){
      *p_ముసుగు &= ~((Bitmask)1 << p_ఎడమ->i_స్తంభం);
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
స్థిర పూర్ణాంకం where_Loop_కూడు_Btree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  సూచీ *pProbe;              /* An index we are evaluating */
  సూచీ sPk;                  /* A fake index object for the primary key */
  LogEst ai_అడ్డువరుస_Est_Pk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 ai_స్తంభం_Pk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  Src_అంశం *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  పూర్ణాంకం rc = SQLITE_OK;         /* Return code */
  పూర్ణాంకం i_క్రమీకరించు_Idx;               /* Index number */
  పూర్ణాంకం b;                      /* A boolean value */
  LogEst r_కొలత;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  పట్టిక *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  ప్రతిజ్ఞా( !IsVirtual(pSrc->pSTab) );

  ఒకవేళ( pSrc->fg.is_సూచీకృత_By ){
    ప్రతిజ్ఞా( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }లేకపోతే ఒకవేళ( !HasRowid(pTab) ){
    ఒకవేళ( pSrc->fg.not_సూచీకృత ){
      pProbe = sqlite3_Primary_కీలకం_సూచీ(pTab);
    }లేకపోతే{
      pProbe = pTab->p_సూచీ;
    }
  }లేకపోతే{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    సూచీ *p_మొదటి;                  /* First of real indices on the table */
    memset(&sPk, 0, పరిమాణం(సూచీ));
    sPk.n_కీలకం_నిలువు = 1;
    sPk.n_స్తంభం = 1;
    sPk.ai_స్తంభం = &ai_స్తంభం_Pk;
    sPk.ai_అడ్డువరుస_Log_Est = ai_అడ్డువరుస_Est_Pk;
    sPk.onError = OE_Replace;
    sPk.p_పట్టిక = pTab;
    sPk.sz_Idx_అడ్డువరుస = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    ai_అడ్డువరుస_Est_Pk[0] = pTab->n_అడ్డువరుస_Log_Est;
    ai_అడ్డువరుస_Est_Pk[1] = 0;
    p_మొదటి = pSrc->pSTab->p_సూచీ;
    sPk.p_తదుపరి = p_మొదటి;
    pProbe = &sPk;
  }
  r_కొలత = pTab->n_అడ్డువరుస_Log_Est;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOMATIC_సూచీ
  /* Automatic indexes */
  ఒకవేళ( !pBuilder->p_Or_సమితి      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_కుడి_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_సూచీ)!=0
   && !pSrc->fg.is_సూచీకృత_By  /* Has no INDEXED BY clause */
   && !pSrc->fg.not_సూచీకృత   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_కుడి)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_కొలత;         /* Logarithm of the number of rows in the table */
    Where_పదం *p_పదం;
    Where_పదం *pWCEnd = pWC->a + pWC->n_పదం;
    r_Log_కొలత = estLog(r_కొలత);
    కొరకు(p_పదం=pWC->a; rc==SQLITE_OK && p_పదం<pWCEnd; p_పదం++){
      ఒకవేళ( p_పదం->prereq_కుడి & pNew->ముసుగు_Self ) కొనసాగు;
      ఒకవేళ( పదం_Can_Drive_సూచీ(p_పదం, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_సూచీ = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = p_పదం;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->r_అమరిక = r_Log_కొలత + r_కొలత;
        ఒకవేళ( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->r_అమరిక += 28;
        }లేకపోతే{
          pNew->r_అమరిక -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        Apply_ఖర్చు_Multiplier(pNew->r_అమరిక, pTab->ఖర్చు_Mult);
        ఒకవేళ( pNew->r_అమరిక<0 ) pNew->r_అమరిక = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->n_బహిర్గతం = 43;  ప్రతిజ్ఞా( 43==sqlite3LogEst(20) );
        pNew->r_నిరంతరం = sqlite3_Log_Est_కూడు(r_Log_కొలత,pNew->n_బహిర్గతం);
        pNew->wsFlags = WHERE_AUTO_సూచీ;
        pNew->prereq = mPrereq | p_పదం->prereq_కుడి;
        rc = where_Loop_చొప్పించు(pBuilder, pNew);
      }
    }
  }
#అయితే_ముగింపు /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  కొరకు(i_క్రమీకరించు_Idx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.is_సూచీకృత_By||pSrc->fg.not_సూచీకృత)?0:pProbe->p_తదుపరి,
         i_క్రమీకరించు_Idx++
  ){
    ఒకవేళ( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_సూచీ(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      కొనసాగు;  /* Partial index inappropriate for this query */
    }
    ఒకవేళ( pProbe->bNoQuery ) కొనసాగు;
    r_కొలత = pProbe->ai_అడ్డువరుస_Log_Est[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_శిఖరం = 0;
    pNew->u.btree.n_Distinct_నిలువు = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->i_క్రమీకరించు_Idx = 0;
    pNew->r_అమరిక = 0;
    pNew->prereq = mPrereq;
    pNew->n_బహిర్గతం = r_కొలత;
    pNew->u.btree.p_సూచీ = pProbe;
    pNew->u.btree.p_క్రమం_By = 0;
    b = సూచీ_Might_Help_With_క్రమం_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    ప్రతిజ్ఞా( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    ఒకవేళ( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->i_క్రమీకరించు_Idx = b ? i_క్రమీకరించు_Idx : 0;
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
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
      pNew->r_నిరంతరం = r_కొలత + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#లేకపోతే
      pNew->r_నిరంతరం = r_కొలత + 16;
#అయితే_ముగింపు
      Apply_ఖర్చు_Multiplier(pNew->r_నిరంతరం, pTab->ఖర్చు_Mult);
      whereLoopOutputAdjust(pWC, pNew, r_కొలత);
      ఒకవేళ( pSrc->fg.isSubquery ){
        ఒకవేళ( pSrc->fg.ద్వారా_Coroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        ఒకవేళ( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.p_క్రమం_By = pSrc->u4.pSubq->pSelect->p_క్రమం_By;
        }
      }లేకపోతే ఒకవేళ( pSrc->fg.fromExists ){
        pNew->n_బహిర్గతం = 0;
      }
      rc = where_Loop_చొప్పించు(pBuilder, pNew);
      pNew->n_బహిర్గతం = r_కొలత;
      ఒకవేళ( rc ) ఆపు;
    }లేకపోతే{
      Bitmask m;
      ఒకవేళ( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_సూచీకృత;
      }లేకపోతే{
        m = pSrc->నిలువు_వాడిన & pProbe->నిలువు_Not_Idxed;
        ఒకవేళ( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_సమాసం(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_సూచీకృత;
        ఒకవేళ( m==TOPBIT || (pProbe->b_Has_సమాసం && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_సూచీ(pWInfo, pProbe, pSrc->iCursor);
          ఒకవేళ( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            ప్రతిజ్ఞా( m!=0 );
          }లేకపోతే{
            m = 0;
            pNew->wsFlags |= isCov;
            ఒకవేళ( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }లేకపోతే{
              ప్రతిజ్ఞా( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }లేకపోతే ఒకవేళ( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_సూచీకృత;
        }
      }

      /* Full scan via index */
      ఒకవేళ( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.is_సూచీకృత_By
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->sz_Idx_అడ్డువరుస<pTab->sz_Tab_అడ్డువరుస)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_Cover_Idx_పరిశీలన)
          )
      ){
        pNew->i_క్రమీకరించు_Idx = b ? i_క్రమీకరించు_Idx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->r_నిరంతరం = r_కొలత + 1 + (15*pProbe->sz_Idx_అడ్డువరుస)/pTab->sz_Tab_అడ్డువరుస;
        ఒకవేళ( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_కొలత + 16;  /* Base cost:  N*3 */
          పూర్ణాంకం ii;
          పూర్ణాంకం iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          కొరకు(ii=0; ii<pWC2->n_పదం; ii++){
            Where_పదం *p_పదం = &pWC2->a[ii];
            ఒకవేళ( !sqlite3_సమాసం_Covered_By_సూచీ(p_పదం->p_సమాసం, iCur, pProbe) ){
              ఆపు;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            ఒకవేళ( p_పదం->truthProb<=0 ){
              nLookup += p_పదం->truthProb;
            }లేకపోతే{
              nLookup--;
              ఒకవేళ( p_పదం->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->r_నిరంతరం = sqlite3_Log_Est_కూడు(pNew->r_నిరంతరం, nLookup);
        }
        Apply_ఖర్చు_Multiplier(pNew->r_నిరంతరం, pTab->ఖర్చు_Mult);
        whereLoopOutputAdjust(pWC, pNew, r_కొలత);
        ఒకవేళ( (pSrc->fg.jointype & JT_కుడి)!=0 && pProbe->a_నిలువు_సమాసం ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }లేకపోతే{
          ఒకవేళ( pSrc->fg.fromExists ) pNew->n_బహిర్గతం = 0;
          rc = where_Loop_చొప్పించు(pBuilder, pNew);
        }
        pNew->n_బహిర్గతం = r_కొలత;
        ఒకవేళ( rc ) ఆపు;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_కూడు_Btree_సూచీ(pBuilder, pSrc, pProbe, 0);
    ఒకవేళ( pBuilder->bldFlags1==SQLITE_BLDF1_సూచీకృత ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#అయితే_ముగింపు
  }
  తిరిగి rc;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
స్థిర పూర్ణాంకం is_హద్దు_పదం(Where_పదం *p_పదం){
  ప్రతిజ్ఞా( p_పదం->eOperator==WO_AUX || p_పదం->e_సరిపోలిక_Op==0 );
  తిరిగి p_పదం->e_సరిపోలిక_Op>=SQLITE_సూచీ_CONSTRAINT_హద్దు
      && p_పదం->e_సరిపోలిక_Op<=SQLITE_సూచీ_CONSTRAINT_స్థానభ్రంశం;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
స్థిర పూర్ణాంకం all_Constraints_వాడిన(
  నిర్మాణం sqlite3_సూచీ_constraint_usage *aUsage, 
  పూర్ణాంకం nCons
){
  పూర్ణాంకం ii;
  కొరకు(ii=0; ii<nCons; ii++){
    ఒకవేళ( aUsage[ii].argv_సూచీ<=0 ) తిరిగి 0;
  }
  తిరిగి 1;
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
స్థిర పూర్ణాంకం where_Loop_కూడు_Virtual_One(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_సూచీ_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  పూర్ణాంకం *pb_లోపల,                      /* OUT: True if plan uses an IN(...) op */
  పూర్ణాంకం *pb_Retry_హద్దు               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_సూచీ_Info *pHidden = (Hidden_సూచీ_Info*)&pIdxInfo[1];
  నిర్మాణం sqlite3_సూచీ_constraint *pIdxCons;
  నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  పూర్ణాంకం i;
  పూర్ణాంకం mx_పదం;
  పూర్ణాంకం rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  Src_అంశం *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  పూర్ణాంకం nConstraint = pIdxInfo->nConstraint;

  ప్రతిజ్ఞా( (mUsable & mPrereq)==mPrereq );
  *pb_లోపల = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(నిర్మాణం sqlite3_సూచీ_constraint**)&pIdxInfo->aConstraint;
  కొరకు(i=0; i<nConstraint; i++, pIdxCons++){
    Where_పదం *p_పదం = పదం_From_Where_Clause(pWC, pIdxCons->i_పదం_స్థానభ్రంశం);
    pIdxCons->usable = 0;
    ఒకవేళ( (p_పదం->prereq_కుడి & mUsable)==p_పదం->prereq_కుడి
     && (p_పదం->eOperator & mExclude)==0
     && (pb_Retry_హద్దు || !is_హద్దు_పదం(p_పదం))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, పరిమాణం(pUsage[0])*nConstraint);
  ప్రతిజ్ఞా( pIdxInfo->need_గమ్యం_Free_Idx_Str==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_అంకె = 0;
  pIdxInfo->క్రమం_By_Consumed = 0;
  pIdxInfo->estimated_ఖర్చు = SQLITE_BIG_DBL / (ద్విగుణ)2;
  pIdxInfo->estimated_అడ్డువరుసలు = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->m_Handle_లోపల = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_ఉత్తమ_సూచీ(pParse, pSrc->pSTab, pIdxInfo);
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      తిరిగి SQLITE_OK;
    }
    తిరిగి rc;
  }

  mx_పదం = -1;
  ప్రతిజ్ఞా( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, పరిమాణం(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, పరిమాణం(pNew->u.vtab));
  pIdxCons = *(నిర్మాణం sqlite3_సూచీ_constraint**)&pIdxInfo->aConstraint;
  కొరకు(i=0; i<nConstraint; i++, pIdxCons++){
    పూర్ణాంకం i_పదం;
    ఒకవేళ( (i_పదం = pUsage[i].argv_సూచీ - 1)>=0 ){
      Where_పదం *p_పదం;
      పూర్ణాంకం j = pIdxCons->i_పదం_స్థానభ్రంశం;
      ఒకవేళ( i_పదం>=nConstraint
       || j<0
       || (p_పదం = పదం_From_Where_Clause(pWC, j))==0
       || pNew->aLTerm[i_పదం]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_సందేశం(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        తిరిగి SQLITE_ERROR;
      }
      testcase( i_పదం==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->n_పదం-1 );
      pNew->prereq |= p_పదం->prereq_కుడి;
      ప్రతిజ్ఞా( i_పదం<pNew->nLSlot );
      pNew->aLTerm[i_పదం] = p_పదం;
      ఒకవేళ( i_పదం>mx_పదం ) mx_పదం = i_పదం;
      testcase( i_పదం==15 );
      testcase( i_పదం==16 );
      ఒకవేళ( pUsage[i].omit ){
        ఒకవేళ( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=i_పదం );
          pNew->u.vtab.omit_ముసుగు |= 1<<i_పదం;
        }లేకపోతే{
          testcase( i!=i_పదం );
        }
        ఒకవేళ( p_పదం->e_సరిపోలిక_Op==SQLITE_సూచీ_CONSTRAINT_స్థానభ్రంశం ){
          pNew->u.vtab.b_Omit_స్థానభ్రంశం = 1;
        }
      }
      ఒకవేళ( SMASKBIT32(i) & pHidden->m_Handle_లోపల ){
        pNew->u.vtab.m_Handle_లోపల |= MASKBIT32(i_పదం);
      }లేకపోతే ఒకవేళ( (p_పదం->eOperator & WO_లోపల)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->క్రమం_By_Consumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_సూచీ_పరిశీలన_UNIQUE;
        *pb_లోపల = 1; ప్రతిజ్ఞా( (mExclude & WO_లోపల)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      ప్రతిజ్ఞా( pb_Retry_హద్దు || !is_హద్దు_పదం(p_పదం) );
      ప్రతిజ్ఞా( !is_హద్దు_పదం(p_పదం) || i>=nConstraint-2 );
      ప్రతిజ్ఞా( !is_హద్దు_పదం(p_పదం) || i==nConstraint-1 || is_హద్దు_పదం(p_పదం+1) );

      ఒకవేళ( is_హద్దు_పదం(p_పదం) && (*pb_లోపల || !all_Constraints_వాడిన(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pb_Retry_హద్దు = 1;
        తిరిగి SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mx_పదం+1;
  కొరకు(i=0; i<=mx_పదం; i++){
    ఒకవేళ( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_సందేశం(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      తిరిగి SQLITE_ERROR;
    }
  }
  ప్రతిజ్ఞా( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_అంకె = pIdxInfo->idx_అంకె;
  pNew->u.vtab.needFree = pIdxInfo->need_గమ్యం_Free_Idx_Str;
  pIdxInfo->need_గమ్యం_Free_Idx_Str = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->క్రమం_By_Consumed ?
      pIdxInfo->n_క్రమం_By : 0);
  pNew->u.vtab.b_Idx_అంకె_Hex = (pIdxInfo->idxFlags&SQLITE_సూచీ_పరిశీలన_HEX)!=0;
  pNew->r_అమరిక = 0;
  pNew->r_నిరంతరం = sqlite3LogEstFromDouble(pIdxInfo->estimated_ఖర్చు);
  pNew->n_బహిర్గతం = sqlite3LogEst(pIdxInfo->estimated_అడ్డువరుసలు);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  ఒకవేళ( pIdxInfo->idxFlags & SQLITE_సూచీ_పరిశీలన_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }లేకపోతే{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_చొప్పించు(pBuilder, pNew);
  ఒకవేళ( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pb_లోపల, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  తిరిగి rc;
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
స్థిరరాశి అక్షరం *sqlite3_vtab_collation(sqlite3_సూచీ_info *pIdxInfo, పూర్ణాంకం iCons){
  Hidden_సూచీ_Info *pHidden = (Hidden_సూచీ_Info*)&pIdxInfo[1];
  స్థిరరాశి అక్షరం *zRet = 0;
  ఒకవేళ( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    పూర్ణాంకం i_పదం = pIdxInfo->aConstraint[iCons].i_పదం_స్థానభ్రంశం;
    సమాసం *pX = పదం_From_Where_Clause(pHidden->pWC, i_పదం)->p_సమాసం;
    ఒకవేళ( pX->p_ఎడమ ){
      pC = sqlite3_సమాసం_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_ద్విమాన);
  }
  తిరిగి zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
పూర్ణాంకం sqlite3_vtab_లోపల(sqlite3_సూచీ_info *pIdxInfo, పూర్ణాంకం iCons, పూర్ణాంకం bHandle){
  Hidden_సూచీ_Info *pHidden = (Hidden_సూచీ_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  ఒకవేళ( m & pHidden->m_లోపల ){
    ఒకవేళ( bHandle==0 ){
      pHidden->m_Handle_లోపల &= ~m;
    }లేకపోతే ఒకవేళ( bHandle>0 ){
      pHidden->m_Handle_లోపల |= m;
    }
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
పూర్ణాంకం sqlite3_vtab_rhs_విలువ(
  sqlite3_సూచీ_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  పూర్ణాంకం iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_విలువ **ppVal           /* Write value extracted here */
){
  Hidden_సూచీ_Info *pH = (Hidden_సూచీ_Info*)&pIdxInfo[1];
  sqlite3_విలువ *pVal = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }లేకపోతే{
    ఒకవేళ( pH->aRhs[iCons]==0 ){
      Where_పదం *p_పదం = పదం_From_Where_Clause(
          pH->pWC, pIdxInfo->aConstraint[iCons].i_పదం_స్థానభ్రంశం
      );
      rc = sqlite3_విలువ_From_సమాసం(
          pH->pParse->db, p_పదం->p_సమాసం->p_కుడి, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  ఒకవేళ( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  తిరిగి rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
పూర్ణాంకం sqlite3_vtab_distinct(sqlite3_సూచీ_info *pIdxInfo){
  Hidden_సూచీ_Info *pHidden = (Hidden_సూచీ_Info*)&pIdxInfo[1];
  ప్రతిజ్ఞా( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  తిరిగి pHidden->eDistinct;
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
శూన్య sqlite3_Vtab_ఉపయోగిస్తుంది_All_Schemas(Parse *pParse){
  పూర్ణాంకం nDb = pParse->db->nDb;
  పూర్ణాంకం i;
  కొరకు(i=0; i<nDb; i++){
    sqlite3_సంకేతం_Verify_Schema(pParse, i);
  }
  ఒకవేళ( Db_ముసుగు_Non_Zero(pParse->write_ముసుగు) ){
    కొరకు(i=0; i<nDb; i++){
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
స్థిర పూర్ణాంకం where_Loop_కూడు_Virtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  పూర్ణాంకం rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  Src_అంశం *pSrc;               /* The FROM clause term to search */
  sqlite3_సూచీ_info *p;       /* Object to pass to xBestIndex() */
  పూర్ణాంకం nConstraint;             /* Number of constraints in p */
  పూర్ణాంకం b_లోపల;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask m_ఉత్తమ;               /* Tables used by best possible plan */
  u16 mNoOmit;
  పూర్ణాంకం bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  ప్రతిజ్ఞా( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  ప్రతిజ్ఞా( IsVirtual(pSrc->pSTab) );
  p = allocate_సూచీ_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  pNew->r_అమరిక = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  ఒకవేళ( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_సూచీ_Info(pParse->db, p);
    తిరిగి SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = where_Loop_కూడు_Virtual_One(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_లోపల, &bRetry
  );
  ఒకవేళ( bRetry ){
    ప్రతిజ్ఞా( rc==SQLITE_OK );
    rc = where_Loop_కూడు_Virtual_One(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_లోపల, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  ఒకవేళ( rc==SQLITE_OK && ((m_ఉత్తమ = (pNew->prereq & ~mPrereq))!=0 || b_లోపల) ){
    పూర్ణాంకం చూసిన_Zero = 0;             /* True if a plan with no prereqs seen */
    పూర్ణాంకం చూసిన_Zero_No_లోపల = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask m_ఉత్తమ_No_లోపల = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    ఒకవేళ( b_లోపల ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = where_Loop_కూడు_Virtual_One(
          pBuilder, mPrereq, ALLBITS, WO_లోపల, p, mNoOmit, &b_లోపల, 0);
      ప్రతిజ్ఞా( b_లోపల==0 );
      m_ఉత్తమ_No_లోపల = pNew->prereq & ~mPrereq;
      ఒకవేళ( m_ఉత్తమ_No_లోపల==0 ){
        చూసిన_Zero = 1;
        చూసిన_Zero_No_లోపల = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    అయితే( rc==SQLITE_OK ){
      పూర్ణాంకం i;
      Bitmask m_తదుపరి = ALLBITS;
      ప్రతిజ్ఞా( m_తదుపరి>0 );
      కొరకు(i=0; i<nConstraint; i++){
        పూర్ణాంకం i_పదం = p->aConstraint[i].i_పదం_స్థానభ్రంశం;
        Bitmask mThis = పదం_From_Where_Clause(pWC, i_పదం)->prereq_కుడి & ~mPrereq;
        ఒకవేళ( mThis>mPrev && mThis<m_తదుపరి ) m_తదుపరి = mThis;
      }
      mPrev = m_తదుపరి;
      ఒకవేళ( m_తదుపరి==ALLBITS ) ఆపు;
      ఒకవేళ( m_తదుపరి==m_ఉత్తమ || m_తదుపరి==m_ఉత్తమ_No_లోపల ) కొనసాగు;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)m_తదుపరి));
      rc = where_Loop_కూడు_Virtual_One(
          pBuilder, mPrereq, m_తదుపరి|mPrereq, 0, p, mNoOmit, &b_లోపల, 0);
      ఒకవేళ( pNew->prereq==mPrereq ){
        చూసిన_Zero = 1;
        ఒకవేళ( b_లోపల==0 ) చూసిన_Zero_No_లోపల = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    ఒకవేళ( rc==SQLITE_OK && చూసిన_Zero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = where_Loop_కూడు_Virtual_One(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &b_లోపల, 0);
      ఒకవేళ( b_లోపల==0 ) చూసిన_Zero_No_లోపల = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    ఒకవేళ( rc==SQLITE_OK && చూసిన_Zero_No_లోపల==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = where_Loop_కూడు_Virtual_One(
          pBuilder, mPrereq, mPrereq, WO_లోపల, p, mNoOmit, &b_లోపల, 0);
    }
  }

  free_సూచీ_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
స్థిర పూర్ణాంకం where_Loop_కూడు_Or(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  Where_పదం *p_పదం, *pWCEnd;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iCur;
  WhereClause తాత్కాలిక_WC;
  WhereLoopBuilder sSubBuild;
  Where_Or_సమితి s_కూడిక, sCur;
  Src_అంశం *p_అంశం;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->n_పదం;
  pNew = pBuilder->pNew;
  memset(&s_కూడిక, 0, పరిమాణం(s_కూడిక));
  p_అంశం = pWInfo->pTabList->a + pNew->iTab;
  iCur = p_అంశం->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  ఒకవేళ( p_అంశం->fg.jointype & JT_కుడి ) తిరిగి SQLITE_OK;

  కొరకు(p_పదం=pWC->a; p_పదం<pWCEnd && rc==SQLITE_OK; p_పదం++){
    ఒకవేళ( (p_పదం->eOperator & WO_OR)!=0
     && (p_పదం->u.pOrInfo->indexable & pNew->ముసుగు_Self)!=0
     && (!సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON)
          || p_పదం->p_సమాసం->w.iJoin==iCur)
    ){
      WhereClause * స్థిరరాశి pOrWC = &p_పదం->u.pOrInfo->wc;
      Where_పదం * స్థిరరాశి pOrWCEnd = &pOrWC->a[pOrWC->n_పదం];
      Where_పదం *p_Or_పదం;
      పూర్ణాంకం once = 1;
      పూర్ణాంకం i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.p_Or_సమితి = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", p_పదం));
      కొరకు(p_Or_పదం=pOrWC->a; p_Or_పదం<pOrWCEnd; p_Or_పదం++){
        ఒకవేళ( (p_Or_పదం->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &p_Or_పదం->u.pAndInfo->wc;
        }లేకపోతే ఒకవేళ( p_Or_పదం->ఎడమ_Cursor==iCur ){
          తాత్కాలిక_WC.pWInfo = pWC->pWInfo;
          తాత్కాలిక_WC.pOuter = pWC;
          తాత్కాలిక_WC.op = TK_AND;
          తాత్కాలిక_WC.n_పదం = 1;
          తాత్కాలిక_WC.n_పునాది = 1;
          తాత్కాలిక_WC.a = p_Or_పదం;
          sSubBuild.pWC = &తాత్కాలిక_WC;
        }లేకపోతే{
          కొనసాగు;
        }
        sCur.n = 0;
#నిర్వచితమైతే WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (పూర్ణాంకం)(p_Or_పదం-pOrWC->a), p_పదం, sSubBuild.pWC->n_పదం));
        ఒకవేళ( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
        ఒకవేళ( IsVirtual(p_అంశం->pSTab) ){
          rc = where_Loop_కూడు_Virtual(&sSubBuild, mPrereq, mUnusable);
        }లేకపోతే
#అయితే_ముగింపు
        {
          rc = where_Loop_కూడు_Btree(&sSubBuild, mPrereq);
        }
        ఒకవేళ( rc==SQLITE_OK ){
          rc = where_Loop_కూడు_Or(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_పూర్తి );
        ఒకవేళ( sCur.n==0 ){
          s_కూడిక.n = 0;
          ఆపు;
        }లేకపోతే ఒకవేళ( once ){
          whereOrMove(&s_కూడిక, &sCur);
          once = 0;
        }లేకపోతే{
          Where_Or_సమితి sPrev;
          whereOrMove(&sPrev, &s_కూడిక);
          s_కూడిక.n = 0;
          కొరకు(i=0; i<sPrev.n; i++){
            కొరకు(j=0; j<sCur.n; j++){
              where_Or_చొప్పించు(&s_కూడిక, sPrev.a[i].prereq | sCur.a[j].prereq,
                            sqlite3_Log_Est_కూడు(sPrev.a[i].r_నిరంతరం, sCur.a[j].r_నిరంతరం),
                            sqlite3_Log_Est_కూడు(sPrev.a[i].n_బహిర్గతం, sCur.a[j].n_బహిర్గతం));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = p_పదం;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->r_అమరిక = 0;
      pNew->i_క్రమీకరించు_Idx = 0;
      memset(&pNew->u, 0, పరిమాణం(pNew->u));
      కొరకు(i=0; rc==SQLITE_OK && i<s_కూడిక.n; i++){
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
        pNew->r_నిరంతరం = s_కూడిక.a[i].r_నిరంతరం + 1;
        pNew->n_బహిర్గతం = s_కూడిక.a[i].n_బహిర్గతం;
        pNew->prereq = s_కూడిక.a[i].prereq;
        rc = where_Loop_చొప్పించు(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", p_పదం));
    }
  }
  తిరిగి rc;
}

/*
** Add all WhereLoop objects for all tables
*/
స్థిర పూర్ణాంకం where_Loop_కూడు_All(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask m_పూర్వ = 0;
  పూర్ణాంకం iTab;
  SrcList *pTabList = pWInfo->pTabList;
  Src_అంశం *p_అంశం;
  Src_అంశం *p_చివర = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం b_మొదటి_Past_RJ = 0;
  పూర్ణాంకం has_కుడి_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  ప్రతిజ్ఞా( pNew->nLTerm==0 );
  ప్రతిజ్ఞా( pNew->wsFlags==0 );
  ప్రతిజ్ఞా( pNew->nLSlot>=Array_కొలత(pNew->aLTermSpace) );
  ప్రతిజ్ఞా( pNew->aLTerm!=0 );

  pBuilder->i_Plan_హద్దు = SQLITE_QUERY_PLANNER_హద్దు;
  కొరకు(iTab=0, p_అంశం=pTabList->a; p_అంశం<p_చివర; iTab++, p_అంశం++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->i_Plan_హద్దు += SQLITE_QUERY_PLANNER_హద్దు_INCR;
    pNew->ముసుగు_Self = sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, p_అంశం->iCursor);
    ఒకవేళ( b_మొదటి_Past_RJ
     || (p_అంశం->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
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
      ఒకవేళ( p_అంశం->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( p_అంశం->fg.jointype & JT_LTORJ );
        testcase( p_అంశం->fg.jointype & JT_CROSS );
        has_కుడి_Cross_Join = 1;
      }
      mPrereq |= m_పూర్వ;
      b_మొదటి_Past_RJ = (p_అంశం->fg.jointype & JT_కుడి)!=0;
    }లేకపోతే ఒకవేళ( p_అంశం->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      Where_పదం *p_పదం;
      పూర్ణాంకం i;
      కొరకు(i=pWC->n_పునాది, p_పదం=pWC->a; i>0; i--, p_పదం++){
        ఒకవేళ( (pNew->ముసుగు_Self & p_పదం->prereqAll)!=0 ){
          mPrereq |= (p_పదం->prereqAll & (pNew->ముసుగు_Self-1));
        }
      }
    }లేకపోతే ఒకవేళ( !has_కుడి_Cross_Join ){
      mPrereq = 0;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    ఒకవేళ( IsVirtual(p_అంశం->pSTab) ){
      Src_అంశం *p;
      కొరకు(p=&p_అంశం[1]; p<p_చివర; p++){
        ఒకవేళ( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, p->iCursor);
        }
      }
      rc = where_Loop_కూడు_Virtual(pBuilder, mPrereq, mUnusable);
    }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = where_Loop_కూడు_Btree(pBuilder, mPrereq);
    }
    ఒకవేళ( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = where_Loop_కూడు_Or(pBuilder, mPrereq, mUnusable);
    }
    m_పూర్వ |= pNew->ముసుగు_Self;
    ఒకవేళ( rc || db->mallocFailed ){
      ఒకవేళ( rc==SQLITE_పూర్తి ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }లేకపోతే{
        ఆపు;
      }
    }
  }

  whereLoopClear(db, pNew);
  తిరిగి rc;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం where_మార్గం_సరిపోలిక_Subquery_OB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  పూర్ణాంకం iLoop,              /* Which level of the nested loop.  0==outermost */
  పూర్ణాంకం iCur,               /* Cursor used by the this loop */
  సమాసం_List *p_క్రమం_By,     /* The ORDER BY clause on the whole query */
  Bitmask *p_Rev_ముసుగు,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  పూర్ణాంకం iOB;                /* Index into pOrderBy->a[] */
  పూర్ణాంకం jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  సమాసం *pOBExpr;          /* Current term of outer ORDER BY */
  సమాసం_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.p_క్రమం_By;
  ప్రతిజ్ఞా( pSubOB!=0 );
  కొరకు(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  కొరకు(jSub=0; jSub<pSubOB->n_సమాసం && iOB<p_క్రమం_By->n_సమాసం; jSub++, iOB++){
    ఒకవేళ( pSubOB->a[jSub].u.x.i_క్రమం_By_నిలువు==0 ) ఆపు;
    pOBExpr = p_క్రమం_By->a[iOB].p_సమాసం;
    ఒకవేళ( pOBExpr->op!=TK_స్తంభం && pOBExpr->op!=TK_AGG_స్తంభం ) ఆపు;
    ఒకవేళ( pOBExpr->i_పట్టిక!=iCur ) ఆపు;
    ఒకవేళ( pOBExpr->i_స్తంభం!=pSubOB->a[jSub].u.x.i_క్రమం_By_నిలువు-1 ) ఆపు;
    ఒకవేళ( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = p_క్రమం_By->a[iOB].fg.క్రమీకరించు_Flags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.క్రమీకరించు_Flags;   /* sortFlags for jSub */
      ఒకవేళ( (sfSub & KEYINFO_క్రమం_BIGNULL) != (sfOB & KEYINFO_క్రమం_BIGNULL) ){
        ఆపు;
      }
      revIdx = sfSub & KEYINFO_క్రమం_DESC;
      ఒకవేళ( jSub>0 ){
        ఒకవేళ( (rev^revIdx)!=(sfOB & KEYINFO_క్రమం_DESC) ){
          ఆపు;
        }
      }లేకపోతే{
        rev = revIdx ^ (sfOB & KEYINFO_క్రమం_DESC);
        ఒకవేళ( rev ){
          ఒకవేళ( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            ఆపు;
          }
          *p_Rev_ముసుగు |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  తిరిగి jSub>0;
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
స్థిర i8 where_మార్గం_Satisfies_క్రమం_By(
  WhereInfo *pWInfo,    /* The WHERE clause */
  సమాసం_List *p_క్రమం_By,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  Where_మార్గం *p_మార్గం,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *p_చివరి,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *p_Rev_ముసుగు     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 rev_సమితి;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 is_క్రమం_Distinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 is_సరిపోలిక;           /* iColumn matches a term of the ORDER BY clause */
  u16 eq_Op_ముసుగు;         /* Allowed equality operators */
  u16 n_కీలకం_నిలువు;          /* Number of key columns in pIndex */
  u16 n_స్తంభం;          /* Total number of ordered columns in the index */
  u16 n_క్రమం_By;         /* Number terms in the ORDER BY clause */
  పూర్ణాంకం iLoop;            /* Index of WhereLoop in pPath being processed */
  పూర్ణాంకం i, j;             /* Loop counters */
  పూర్ణాంకం iCur;             /* Cursor number for current WhereLoop */
  పూర్ణాంకం i_స్తంభం;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  Where_పదం *p_పదం;     /* A single term of the WHERE clause */
  సమాసం *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  సూచీ *p_సూచీ;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask ob_పూర్తి;       /* Mask of all ORDER BY terms */
  Bitmask క్రమం_Distinct_ముసుగు;  /* Mask of all well-ordered loops */
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

  ప్రతిజ్ఞా( p_క్రమం_By!=0 );
  ఒకవేళ( nLoop && OptimizationDisabled(db, SQLITE_క్రమం_By_Idx_Join) ) తిరిగి 0;

  n_క్రమం_By = p_క్రమం_By->n_సమాసం;
  testcase( n_క్రమం_By==BMS-1 );
  ఒకవేళ( n_క్రమం_By>BMS-1 ) తిరిగి 0;  /* Cannot optimize overly large ORDER BYs */
  is_క్రమం_Distinct = 1;
  ob_పూర్తి = MASKBIT(n_క్రమం_By)-1;
  క్రమం_Distinct_ముసుగు = 0;
  ready = 0;
  eq_Op_ముసుగు = WO_EQ | WO_IS | WO_ISNULL;
  ఒకవేళ( wctrlFlags & (WHERE_ORDERBY_హద్దు|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eq_Op_ముసుగు |= WO_లోపల;
  }
  కొరకు(iLoop=0; is_క్రమం_Distinct && obSat<ob_పూర్తి && iLoop<=nLoop; iLoop++){
    ఒకవేళ( iLoop>0 ) ready |= pLoop->ముసుగు_Self;
    ఒకవేళ( iLoop<nLoop ){
      pLoop = p_మార్గం->aLoop[iLoop];
      ఒకవేళ( wctrlFlags & WHERE_ORDERBY_హద్దు ) కొనసాగు;
    }లేకపోతే{
      pLoop = p_చివరి;
    }
    ఒకవేళ( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      ఒకవేళ( pLoop->u.vtab.isOrdered && pWInfo->p_క్రమం_By==p_క్రమం_By ){
        obSat = ob_పూర్తి;
      }లేకపోతే{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        is_క్రమం_Distinct = 0;
      }
      ఆపు;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    కొరకు(i=0; i<n_క్రమం_By; i++){
      ఒకవేళ( MASKBIT(i) & obSat ) కొనసాగు;
      pOBExpr = sqlite3_సమాసం_Skip_Collate_And_Likely(p_క్రమం_By->a[i].p_సమాసం);
      ఒకవేళ( NEVER(pOBExpr==0) ) కొనసాగు;
      ఒకవేళ( pOBExpr->op!=TK_స్తంభం && pOBExpr->op!=TK_AGG_స్తంభం ) కొనసాగు;
      ఒకవేళ( pOBExpr->i_పట్టిక!=iCur ) కొనసాగు;
      p_పదం = sqlite3_Where_కనుగొను_పదం(&pWInfo->sWC, iCur, pOBExpr->i_స్తంభం,
                       ~ready, eq_Op_ముసుగు, 0);
      ఒకవేళ( p_పదం==0 ) కొనసాగు;
      ఒకవేళ( p_పదం->eOperator==WO_లోపల ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        ప్రతిజ్ఞా( wctrlFlags &
               (WHERE_ORDERBY_హద్దు|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        కొరకు(j=0; j<pLoop->nLTerm && p_పదం!=pLoop->aLTerm[j]; j++){}
        ఒకవేళ( j>=pLoop->nLTerm ) కొనసాగు;
      }
      ఒకవేళ( (p_పదం->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->i_స్తంభం>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_సమాసం_NNColl_Seq(pParse, p_క్రమం_By->a[i].p_సమాసం);
        స్థిరరాశి సమాసం *pCExpr = p_పదం->p_సమాసం;
        CollSeq *pColl2 = sqlite3_సమాసం_Compare_Coll_Seq(pParse, pCExpr);
        అక్షరం aff_కుడి;
        ప్రతిజ్ఞా( pColl1 );
        ఒకవేళ( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          కొనసాగు;
        }
        aff_కుడి = sqlite3_సమాసం_Affinity(pCExpr->p_కుడి);
        /* Left operand must be a column, hence always has affinity */
        ప్రతిజ్ఞా( sqlite3_సమాసం_Affinity(pCExpr->p_ఎడమ)!=0 );
        ఒకవేళ( aff_కుడి!=0 && aff_కుడి!=sqlite3_సమాసం_Affinity(pCExpr->p_ఎడమ) ){
          కొనసాగు;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    ఒకవేళ( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      ఒకవేళ( pLoop->wsFlags & WHERE_IPK ){
        ఒకవేళ( pLoop->u.btree.p_క్రమం_By
         && OptimizationEnabled(db, SQLITE_క్రమం_By_Subq)
         &&  where_మార్గం_సరిపోలిక_Subquery_OB(pLoop,iLoop,iCur,
                                     p_క్రమం_By,p_Rev_ముసుగు, &obSat, wctrlFlags)
        ){
          n_స్తంభం = 0;
          is_క్రమం_Distinct = 0;
        }లేకపోతే{
          n_స్తంభం = 1;
        }
        p_సూచీ = 0;
        n_కీలకం_నిలువు = 1;
      }లేకపోతే ఒకవేళ( (p_సూచీ = pLoop->u.btree.p_సూచీ)==0 || p_సూచీ->bUnordered ){
        తిరిగి 0;
      }లేకపోతే{
        n_కీలకం_నిలువు = p_సూచీ->n_కీలకం_నిలువు;
        n_స్తంభం = p_సూచీ->n_స్తంభం;
        ప్రతిజ్ఞా( n_స్తంభం==n_కీలకం_నిలువు+1 || !HasRowid(p_సూచీ->p_పట్టిక) );
        ప్రతిజ్ఞా( p_సూచీ->ai_స్తంభం[n_స్తంభం-1]==XN_ROWID
                          || !HasRowid(p_సూచీ->p_పట్టిక));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        is_క్రమం_Distinct = Is_Unique_సూచీ(p_సూచీ)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = rev_సమితి = 0;
      distinctColumns = 0;
      కొరకు(j=0; j<n_స్తంభం; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        ప్రతిజ్ఞా( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        ఒకవేళ( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          ఒకవేళ( (eOp & eq_Op_ముసుగు)!=0 ){
            ఒకవేళ( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( is_క్రమం_Distinct );
              is_క్రమం_Distinct = 0;
            }
            కొనసాగు; 
          }లేకపోతే ఒకవేళ( ALWAYS(eOp & WO_లోపల) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            సమాసం *pX = pLoop->aLTerm[j]->p_సమాసం;
            కొరకు(i=j+1; i<pLoop->u.btree.nEq; i++){
              ఒకవేళ( pLoop->aLTerm[i]->p_సమాసం==pX ){
                ప్రతిజ్ఞా( (pLoop->aLTerm[i]->eOperator & WO_లోపల) );
                bOnce = 0;
                ఆపు;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        ఒకవేళ( p_సూచీ ){
          i_స్తంభం = p_సూచీ->ai_స్తంభం[j];
          revIdx = p_సూచీ->a_క్రమీకరించు_క్రమం[j] & KEYINFO_క్రమం_DESC;
          ఒకవేళ( i_స్తంభం==p_సూచీ->p_పట్టిక->iPKey ) i_స్తంభం = XN_ROWID;
        }లేకపోతే{
          i_స్తంభం = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        ఒకవేళ( is_క్రమం_Distinct ){
          ఒకవేళ( i_స్తంభం>=0
           && j>=pLoop->u.btree.nEq
           && p_సూచీ->p_పట్టిక->a_నిలువు[i_స్తంభం].notNull==0
          ){
            is_క్రమం_Distinct = 0;
          }
          ఒకవేళ( i_స్తంభం==XN_సమాసం ){
            is_క్రమం_Distinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        is_సరిపోలిక = 0;
        కొరకు(i=0; bOnce && i<n_క్రమం_By; i++){
          ఒకవేళ( MASKBIT(i) & obSat ) కొనసాగు;
          pOBExpr = sqlite3_సమాసం_Skip_Collate_And_Likely(p_క్రమం_By->a[i].p_సమాసం);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          ఒకవేళ( NEVER(pOBExpr==0) ) కొనసాగు;
          ఒకవేళ( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          ఒకవేళ( i_స్తంభం>=XN_ROWID ){
            ఒకవేళ( pOBExpr->op!=TK_స్తంభం && pOBExpr->op!=TK_AGG_స్తంభం ) కొనసాగు;
            ఒకవేళ( pOBExpr->i_పట్టిక!=iCur ) కొనసాగు;
            ఒకవేళ( pOBExpr->i_స్తంభం!=i_స్తంభం ) కొనసాగు;
          }లేకపోతే{
            సమాసం *p_Ix_సమాసం = p_సూచీ->a_నిలువు_సమాసం->a[j].p_సమాసం;
            ఒకవేళ( sqlite3_సమాసం_Compare_Skip(pOBExpr, p_Ix_సమాసం, iCur) ){
              కొనసాగు;
            }
          }
          ఒకవేళ( i_స్తంభం!=XN_ROWID ){
            pColl = sqlite3_సమాసం_NNColl_Seq(pWInfo->pParse, p_క్రమం_By->a[i].p_సమాసం);
            ఒకవేళ( sqlite3StrICmp(pColl->zName, p_సూచీ->azColl[j])!=0 ) కొనసాగు;
          }
          ఒకవేళ( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.n_Distinct_నిలువు = j+1;
          }
          is_సరిపోలిక = 1;
          ఆపు;
        }
        ఒకవేళ( is_సరిపోలిక && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          ఒకవేళ( rev_సమితి ){
            ఒకవేళ( (rev ^ revIdx)
                           != (p_క్రమం_By->a[i].fg.క్రమీకరించు_Flags&KEYINFO_క్రమం_DESC)
            ){
              is_సరిపోలిక = 0;
            }
          }లేకపోతే{
            rev = revIdx ^ (p_క్రమం_By->a[i].fg.క్రమీకరించు_Flags & KEYINFO_క్రమం_DESC);
            ఒకవేళ( rev ) *p_Rev_ముసుగు |= MASKBIT(iLoop);
            rev_సమితి = 1;
          }
        }
        ఒకవేళ( is_సరిపోలిక && (p_క్రమం_By->a[i].fg.క్రమీకరించు_Flags & KEYINFO_క్రమం_BIGNULL) ){
          ఒకవేళ( j==pLoop->u.btree.nEq 
           && (j<n_కీలకం_నిలువు||HasRowid(p_సూచీ->p_పట్టిక)||!Is_Primary_కీలకం_సూచీ(p_సూచీ))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_క్రమీకరించు;
          }లేకపోతే{
            is_సరిపోలిక = 0;
          }
        }
        ఒకవేళ( is_సరిపోలిక ){
          ఒకవేళ( i_స్తంభం==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }లేకపోతే{
          /* No match found */
          ప్రతిజ్ఞా( n_కీలకం_నిలువు>0 );
          ఒకవేళ( j<n_కీలకం_నిలువు ){
            testcase( is_క్రమం_Distinct!=0 );
            is_క్రమం_Distinct = 0;
          }
          ఆపు;
        }
      } /* end Loop over all index columns */
      ఒకవేళ( distinctColumns ){
        testcase( is_క్రమం_Distinct==0 );
        is_క్రమం_Distinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    ఒకవేళ( is_క్రమం_Distinct ){
      క్రమం_Distinct_ముసుగు |= pLoop->ముసుగు_Self;
      కొరకు(i=0; i<n_క్రమం_By; i++){
        సమాసం *p;
        Bitmask m_పదం;
        ఒకవేళ( MASKBIT(i) & obSat ) కొనసాగు;
        p = p_క్రమం_By->a[i].p_సమాసం;
        m_పదం = sqlite3_Where_సమాసం_Usage(&pWInfo->s_ముసుగు_సమితి,p);
        ఒకవేళ( m_పదం==0 && !sqlite3_సమాసం_Is_Constant(0,p) ) కొనసాగు;
        ఒకవేళ( (m_పదం&~క్రమం_Distinct_ముసుగు)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  ఒకవేళ( obSat==ob_పూర్తి ) తిరిగి (i8)n_క్రమం_By;
  ఒకవేళ( !is_క్రమం_Distinct ){
    కొరకు(i=n_క్రమం_By-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      ఒకవేళ( (obSat&m)==m ) తిరిగి i;
    }
    తిరిగి 0;
  }
  తిరిగి -1;
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
పూర్ణాంకం sqlite3WhereIsSorted(WhereInfo *pWInfo){
  ప్రతిజ్ఞా( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  ప్రతిజ్ఞా( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  తిరిగి pWInfo->sorted;
}

#నిర్వచితమైతే WHERETRACE_ENABLED
/* For debugging use only: */
స్థిర స్థిరరాశి అక్షరం *where_మార్గం_Name(Where_మార్గం *p_మార్గం, పూర్ణాంకం nLoop, WhereLoop *p_చివరి){
  స్థిర అక్షరం zName[65];
  పూర్ణాంకం i;
  కొరకు(i=0; i<nLoop; i++){ zName[i] = p_మార్గం->aLoop[i]->cId; }
  ఒకవేళ( p_చివరి ) zName[i++] = p_చివరి->cId;
  zName[i] = 0;
  తిరిగి zName;
}
#అయితే_ముగింపు

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
స్థిర LogEst where_క్రమీకరణ_ఖర్చు(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst n_అడ్డువరుస,       /* Estimated number of rows to sort */
  పూర్ణాంకం n_క్రమం_By,      /* Number of ORDER BY clause terms */
  పూర్ణాంకం nSorted        /* Number of initial ORDER BY terms naturally in order */
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
  LogEst r_క్రమీకరించు_ఖర్చు, n_నిలువు;
  ప్రతిజ్ఞా( pWInfo->pSelect!=0 );
  ప్రతిజ్ఞా( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  n_నిలువు = sqlite3LogEst((pWInfo->pSelect->pEList->n_సమాసం+59)/30);
  r_క్రమీకరించు_ఖర్చు = n_అడ్డువరుస + n_నిలువు;
  ఒకవేళ( nSorted>0 ){
    /* Scale the result by (Y/X) */
    r_క్రమీకరించు_ఖర్చు += sqlite3LogEst((n_క్రమం_By-nSorted)*100/n_క్రమం_By) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  ఒకవేళ( (pWInfo->wctrlFlags & WHERE_USE_హద్దు)!=0 ){
    r_క్రమీకరించు_ఖర్చు += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    ఒకవేళ( nSorted!=0 ){
      r_క్రమీకరించు_ఖర్చు += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    ఒకవేళ( pWInfo->i_హద్దు<n_అడ్డువరుస ){
      n_అడ్డువరుస = pWInfo->i_హద్దు;
    }
  }లేకపోతే ఒకవేళ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    ఒకవేళ( n_అడ్డువరుస>10 ){ n_అడ్డువరుస -= 10;  ప్రతిజ్ఞా( 10==sqlite3LogEst(2) ); }
  }
  r_క్రమీకరించు_ఖర్చు += estLog(n_అడ్డువరుస);
  తిరిగి r_క్రమీకరించు_ఖర్చు;
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
స్థిర పూర్ణాంకం computeMxChoice(WhereInfo *pWInfo){
  పూర్ణాంకం nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#నిర్వచితమైతే SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  కొరకు(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
    ప్రతిజ్ఞా( pWLoop->ముసుగు_Self==MASKBIT(pWLoop->iTab) );
    ప్రతిజ్ఞా( pWLoop->p_తదుపరి_Loop==0 || pWLoop->iTab<=pWLoop->p_తదుపరి_Loop->iTab );
  }
#అయితే_ముగింపు /* SQLITE_DEBUG */

  ఒకవేళ( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->b_నక్షత్రం_పూర్తి
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_నక్షత్రం_Query)
  ){
    Src_అంశం *aFromTabs;    /* All terms of the FROM clause */
    పూర్ణాంకం iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *p_ప్రారంభం;     /* Where to start searching for dimension-tables */

    pWInfo->b_నక్షత్రం_పూర్తి = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    ప్రతిజ్ఞా( !pWInfo->b_నక్షత్రం_వాడిన );
    aFromTabs = pWInfo->pTabList->a;
    p_ప్రారంభం = pWInfo->pLoops;
    కొరకు(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      పూర్ణాంకం nDep = 0;             /* Number of dimension tables */
      LogEst mx_నిరంతరం;             /* Maximum SCAN cost of a fact table */
      Bitmask m_చూసిన = 0;        /* Mask of dimension tables */
      Src_అంశం *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      ఒకవేళ( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        ఒకవేళ( iFromIdx+3 > nLoop ){
          ఆపు;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        అయితే( p_ప్రారంభం && p_ప్రారంభం->iTab<=iFromIdx ){
          p_ప్రారంభం = p_ప్రారంభం->p_తదుపరి_Loop;
        }
      }
      కొరకు(pWLoop=p_ప్రారంభం; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
        ఒకవేళ( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          ఆపు; /* Constraint (bb) */
        }
        ఒకవేళ( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->ముసుగు_Self & m_చూసిన)==0  /* pWInfo not already a dependency */
         && (pWLoop->ముసుగు_Self & mSelfJoin)==0 /* Not a self-join */
        ){
          ఒకవేళ( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }లేకపోతే{
            nDep++;
            m_చూసిన |= pWLoop->ముసుగు_Self;
          }
        }
      }
      ఒకవేళ( nDep<=2 ){
        కొనసాగు; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#నిర్వచితమైతే WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      ఒకవేళ( !pWInfo->b_నక్షత్రం_వాడిన ){
        కొరకు(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
          pWLoop->r_నక్షత్రం_Delta = 0;
        }
      }
#అయితే_ముగింపు
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x80000 */
      ఒకవేళ( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = m_చూసిన;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        కొరకు(pWLoop=p_ప్రారంభం; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
          ఒకవేళ( mShow & pWLoop->ముసుగు_Self ){
            Src_అంశం *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->ముసుగు_Self;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#అయితే_ముగింపు
      pWInfo->b_నక్షత్రం_వాడిన = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mx_నిరంతరం = LOGEST_MIN;
      కొరకు(pWLoop=p_ప్రారంభం; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
        ఒకవేళ( pWLoop->iTab<iFromIdx ) కొనసాగు;
        ఒకవేళ( pWLoop->iTab>iFromIdx ) ఆపు;
        ఒకవేళ( pWLoop->r_నిరంతరం>mx_నిరంతరం ) mx_నిరంతరం = pWLoop->r_నిరంతరం;
      }
      ఒకవేళ( ALWAYS(mx_నిరంతరం<LOGEST_MAX) ) mx_నిరంతరం++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      కొరకు(pWLoop=p_ప్రారంభం; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
        ఒకవేళ( (pWLoop->ముసుగు_Self & m_చూసిన)==0 ) కొనసాగు;
        ఒకవేళ( pWLoop->nLTerm ) కొనసాగు;
        ఒకవేళ( pWLoop->r_నిరంతరం<mx_నిరంతరం ){
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x80000 */
          ఒకవేళ( sqlite3WhereTrace & 0x80000 ){
            Src_అంశం *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mx_నిరంతరం
            );
          }
          pWLoop->r_నక్షత్రం_Delta = mx_నిరంతరం - pWLoop->r_నిరంతరం;
#అయితే_ముగింపు /* WHERETRACE_ENABLED */
          pWLoop->r_నిరంతరం = mx_నిరంతరం;
        }
      }
    }
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x80000 */
    ఒకవేళ( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->b_నక్షత్రం_వాడిన ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      కొరకు(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
        ఒకవేళ( pWLoop->r_నక్షత్రం_Delta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#అయితే_ముగింపు
  }
  తిరిగి pWInfo->b_నక్షత్రం_వాడిన ? 18 : 12;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం whereLoopIsNoBetter(
  స్థిరరాశి WhereLoop *pCandidate,
  స్థిరరాశి WhereLoop *pBaseline
){
  ఒకవేళ( (pCandidate->wsFlags & WHERE_సూచీకృత)==0 ) తిరిగి 1;
  ఒకవేళ( (pBaseline->wsFlags & WHERE_సూచీకృత)==0 ) తిరిగి 1;
  ఒకవేళ( pCandidate->u.btree.p_సూచీ->sz_Idx_అడ్డువరుస <
        pBaseline->u.btree.p_సూచీ->sz_Idx_అడ్డువరుస ) తిరిగి 0;
  తిరిగి 1;
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
స్థిర పూర్ణాంకం where_మార్గం_Solver(WhereInfo *pWInfo, LogEst n_అడ్డువరుస_Est){
  పూర్ణాంకం mxChoice;             /* Maximum number of simultaneous paths tracked */
  పూర్ణాంకం nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  పూర్ణాంకం iLoop;                /* Loop counter over the terms of the join */
  పూర్ణాంకం ii, jj;               /* Loop counters */
  పూర్ణాంకం mxI = 0;              /* Index of next entry to replace */
  పూర్ణాంకం n_క్రమం_By;             /* Number of ORDER BY clause terms */
  LogEst mx_ఖర్చు = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  పూర్ణాంకం n_గమ్యం, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  Where_మార్గం *aFrom;         /* All nFrom paths at the previous level */
  Where_మార్గం *a_గమ్యం;           /* The nTo best paths at the current level */
  Where_మార్గం *pFrom;         /* An element of aFrom[] that we are working on */
  Where_మార్గం *p_గమ్యం;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *a_క్రమీకరించు_ఖర్చు = 0;    /* Sorting and partial sorting costs */
  అక్షరం *pSpace;             /* Temporary memory used by this routine */
  పూర్ణాంకం nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     n_అడ్డువరుస_Est, pParse->nQueryLoop));
  /* TUNING: mxChoice is the maximum number of possible paths to preserve
  ** at each step.  Based on the number of loops in the FROM clause:
  **
  **     nLoop      mxChoice
  **     -----      --------
  **       1            1            // the most common case
  **       2            5
  **       3+        12 or 18        // see computeMxChoice()
  */
  ఒకవేళ( nLoop<=1 ){
    mxChoice = 1;
  }లేకపోతే ఒకవేళ( nLoop==2 ){
    mxChoice = 5;
  }లేకపోతే ఒకవేళ( pParse->nErr ){
    mxChoice = 1;
  }లేకపోతే{
    mxChoice = computeMxChoice(pWInfo);
  }
  ప్రతిజ్ఞా( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  ఒకవేళ( pWInfo->p_క్రమం_By==0 || n_అడ్డువరుస_Est==0 ){
    n_క్రమం_By = 0;
  }లేకపోతే{
    n_క్రమం_By = pWInfo->p_క్రమం_By->n_సమాసం;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (పరిమాణం(Where_మార్గం)+పరిమాణం(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += పరిమాణం(LogEst) * n_క్రమం_By;
  pSpace = sqlite3_దొంతర_Alloc_Raw_NN(pParse->db, nSpace);
  ఒకవేళ( pSpace==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  a_గమ్యం = (Where_మార్గం*)pSpace;
  aFrom = a_గమ్యం+mxChoice;
  memset(aFrom, 0, పరిమాణం(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  కొరకు(ii=mxChoice*2, pFrom=a_గమ్యం; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  ఒకవేళ( n_క్రమం_By ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    a_క్రమీకరించు_ఖర్చు = (LogEst*)pX;
    memset(a_క్రమీకరించు_ఖర్చు, 0, పరిమాణం(LogEst) * n_క్రమం_By);
  }
  ప్రతిజ్ఞా( a_క్రమీకరించు_ఖర్చు==0 || &pSpace[nSpace]==(అక్షరం*)&a_క్రమీకరించు_ఖర్చు[n_క్రమం_By] );
  ప్రతిజ్ఞా( a_క్రమీకరించు_ఖర్చు!=0 || &pSpace[nSpace]==(అక్షరం*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].n_అడ్డువరుస = MIN(pParse->nQueryLoop, 48);  ప్రతిజ్ఞా( 48==sqlite3LogEst(28) );
  nFrom = 1;
  ప్రతిజ్ఞా( aFrom[0].isOrdered==0 );
  ఒకవేళ( n_క్రమం_By ){
    /* If nLoop is zero, then there are no FROM terms in the query. Since
    ** in this case the query may return a maximum of one row, the results
    ** are already in the requested order. Set isOrdered to nOrderBy to
    ** indicate this. Or, if nLoop is greater than zero, set isOrdered to
    ** -1, indicating that the result set may or may not be ordered,
    ** depending on the loops added to the current plan.  */
    aFrom[0].isOrdered = nLoop>0 ? -1 : n_క్రమం_By;
  }

  /* Compute successively longer WherePaths using the previous generation
  ** of WherePaths as the basis for the next.  Keep track of the mxChoice
  ** best paths at each generation */
  కొరకు(iLoop=0; iLoop<nLoop; iLoop++){
    n_గమ్యం = 0;
    కొరకు(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      కొరకు(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_తదుపరి_Loop){
        LogEst n_బహిర్గతం;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst r_ఖర్చు;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask ముసుగు_New;                  /* Mask of src visited by (..) */
        Bitmask rev_ముసుగు;                  /* Mask of rev-order loops for (..) */

        ఒకవేళ( (pWLoop->prereq & ~pFrom->ముసుగు_Loop)!=0 ) కొనసాగు;
        ఒకవేళ( (pWLoop->ముసుగు_Self & pFrom->ముసుగు_Loop)!=0 ) కొనసాగు;
        ఒకవేళ( (pWLoop->wsFlags & WHERE_AUTO_సూచీ)!=0 && pFrom->n_అడ్డువరుస<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          ప్రతిజ్ఞా( 10==sqlite3LogEst(2) );
          కొనసాగు;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->r_నిరంతరం + pFrom->n_అడ్డువరుస;
        ఒకవేళ( pWLoop->r_అమరిక ){
          rUnsort = sqlite3_Log_Est_కూడు(pWLoop->r_అమరిక, rUnsort);
        }
        rUnsort = sqlite3_Log_Est_కూడు(rUnsort, pFrom->rUnsort);
        n_బహిర్గతం = pFrom->n_అడ్డువరుస + pWLoop->n_బహిర్గతం;
        ముసుగు_New = pFrom->ముసుగు_Loop | pWLoop->ముసుగు_Self;
        isOrdered = pFrom->isOrdered;
        ఒకవేళ( isOrdered<0 ){
          rev_ముసుగు = 0;
          isOrdered = where_మార్గం_Satisfies_క్రమం_By(pWInfo,
                       pWInfo->p_క్రమం_By, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &rev_ముసుగు);
        }లేకపోతే{
          rev_ముసుగు = pFrom->revLoop;
        }
        ఒకవేళ( isOrdered>=0 && isOrdered<n_క్రమం_By ){
          ఒకవేళ( a_క్రమీకరించు_ఖర్చు[isOrdered]==0 ){
            a_క్రమీకరించు_ఖర్చు[isOrdered] = where_క్రమీకరణ_ఖర్చు(
                pWInfo, n_అడ్డువరుస_Est, n_క్రమం_By, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          r_ఖర్చు = sqlite3_Log_Est_కూడు(rUnsort, a_క్రమీకరించు_ఖర్చు[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               a_క్రమీకరించు_ఖర్చు[isOrdered], (n_క్రమం_By-isOrdered), n_క్రమం_By,
               rUnsort, r_ఖర్చు));
        }లేకపోతే{
          r_ఖర్చు = rUnsort;
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
        testcase( n_గమ్యం==0 );
        కొరకు(jj=0, p_గమ్యం=a_గమ్యం; jj<n_గమ్యం; jj++, p_గమ్యం++){
          ఒకవేళ( p_గమ్యం->ముసుగు_Loop==ముసుగు_New
           && ( ((p_గమ్యం->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==n_గమ్యం-1 );
            ఆపు;
          }
        }
        ఒకవేళ( jj>=n_గమ్యం ){
          /* None of the existing best-so-far paths match the candidate. */
          ఒకవేళ( n_గమ్యం>=mxChoice
           && (r_ఖర్చు>mx_ఖర్చు || (r_ఖర్చు==mx_ఖర్చు && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x4 */
            ఒకవేళ( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  where_మార్గం_Name(pFrom, iLoop, pWLoop), r_ఖర్చు, n_బహిర్గతం, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#అయితే_ముగింపు
            కొనసాగు;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          ఒకవేళ( n_గమ్యం<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = n_గమ్యం++;
          }లేకపోతే{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          p_గమ్యం = &a_గమ్యం[jj];
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x4 */
          ఒకవేళ( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                where_మార్గం_Name(pFrom, iLoop, pWLoop), r_ఖర్చు, n_బహిర్గతం, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#అయితే_ముగింపు
        }లేకపోతే{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          ఒకవేళ( (p_గమ్యం->r_ఖర్చు<r_ఖర్చు)
           || (p_గమ్యం->r_ఖర్చు==r_ఖర్చు && p_గమ్యం->n_అడ్డువరుస<n_బహిర్గతం)
           || (p_గమ్యం->r_ఖర్చు==r_ఖర్చు && p_గమ్యం->n_అడ్డువరుస==n_బహిర్గతం && p_గమ్యం->rUnsort<rUnsort)
           || (p_గమ్యం->r_ఖర్చు==r_ఖర్చు && p_గమ్యం->n_అడ్డువరుస==n_బహిర్గతం && p_గమ్యం->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, p_గమ్యం->aLoop[iLoop]) )
          ){
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x4 */
            ఒకవేళ( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  where_మార్గం_Name(pFrom, iLoop, pWLoop), r_ఖర్చు, n_బహిర్గతం, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  where_మార్గం_Name(p_గమ్యం, iLoop+1, 0), p_గమ్యం->r_ఖర్చు, p_గమ్యం->n_అడ్డువరుస,
                  p_గమ్యం->rUnsort, p_గమ్యం->isOrdered>=0 ? p_గమ్యం->isOrdered+'0' : '?');
            }
#అయితే_ముగింపు
            /* Discard the candidate path from further consideration */
            testcase( p_గమ్యం->r_ఖర్చు==r_ఖర్చు );
            కొనసాగు;
          }
          testcase( p_గమ్యం->r_ఖర్చు==r_ఖర్చు+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x4 */
          ఒకవేళ( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                where_మార్గం_Name(pFrom, iLoop, pWLoop), r_ఖర్చు, n_బహిర్గతం, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                where_మార్గం_Name(p_గమ్యం, iLoop+1, 0), p_గమ్యం->r_ఖర్చు, p_గమ్యం->n_అడ్డువరుస,
                p_గమ్యం->rUnsort, p_గమ్యం->isOrdered>=0 ? p_గమ్యం->isOrdered+'0' : '?');
          }
#అయితే_ముగింపు
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        p_గమ్యం->ముసుగు_Loop = pFrom->ముసుగు_Loop | pWLoop->ముసుగు_Self;
        p_గమ్యం->revLoop = rev_ముసుగు;
        p_గమ్యం->n_అడ్డువరుస = n_బహిర్గతం;
        p_గమ్యం->r_ఖర్చు = r_ఖర్చు;
        p_గమ్యం->rUnsort = rUnsort;
        p_గమ్యం->isOrdered = isOrdered;
        జ్ఞాపకం_నకలు(p_గమ్యం->aLoop, pFrom->aLoop, పరిమాణం(WhereLoop*)*iLoop);
        p_గమ్యం->aLoop[iLoop] = pWLoop;
        ఒకవేళ( n_గమ్యం>=mxChoice ){
          mxI = 0;
          mx_ఖర్చు = a_గమ్యం[0].r_ఖర్చు;
          mxUnsort = a_గమ్యం[0].n_అడ్డువరుస;
          కొరకు(jj=1, p_గమ్యం=&a_గమ్యం[1]; jj<mxChoice; jj++, p_గమ్యం++){
            ఒకవేళ( p_గమ్యం->r_ఖర్చు>mx_ఖర్చు
             || (p_గమ్యం->r_ఖర్చు==mx_ఖర్చు && p_గమ్యం->rUnsort>mxUnsort)
            ){
              mx_ఖర్చు = p_గమ్యం->r_ఖర్చు;
              mxUnsort = p_గమ్యం->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#నిర్వచితమైతే WHERETRACE_ENABLED  /* >=2 */
    ఒకవేళ( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      పూర్ణాంకం n_పూర్తి = 0;
      పూర్ణాంకం n_పురోగతి;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      చేయి{
        n_పురోగతి = 0;
        rMin = 0x7fff;
        కొరకు(ii=0, p_గమ్యం=a_గమ్యం; ii<n_గమ్యం; ii++, p_గమ్యం++){
          ఒకవేళ( p_గమ్యం->r_ఖర్చు>rFloor && p_గమ్యం->r_ఖర్చు<rMin ) rMin = p_గమ్యం->r_ఖర్చు;
        }
        కొరకు(ii=0, p_గమ్యం=a_గమ్యం; ii<n_గమ్యం; ii++, p_గమ్యం++){
          ఒకవేళ( p_గమ్యం->r_ఖర్చు==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               where_మార్గం_Name(p_గమ్యం, iLoop+1, 0), p_గమ్యం->r_ఖర్చు, p_గమ్యం->n_అడ్డువరుస,
               p_గమ్యం->isOrdered>=0 ? (p_గమ్యం->isOrdered+'0') : '?');
            ఒకవేళ( p_గమ్యం->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", p_గమ్యం->revLoop);
            }లేకపోతే{
              sqlite3DebugPrintf("\n");
            }
            n_పూర్తి++;
            n_పురోగతి++;
          }
        }
        rFloor = rMin;
      }అయితే( n_పూర్తి<n_గమ్యం && n_పురోగతి>0 );
    }
#అయితే_ముగింపు

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = a_గమ్యం;
    a_గమ్యం = aFrom;
    aFrom = pFrom;
    nFrom = n_గమ్యం;
  }

  ఒకవేళ( nFrom==0 ){
    sqlite3_Error_సందేశం(pParse, "no query solution");
    sqlite3_దొంతర_Free_NN(pParse->db, pSpace);
    తిరిగి SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  ప్రతిజ్ఞా( nFrom==1 );
  pFrom = aFrom;

  ప్రతిజ్ఞా( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  కొరకు(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  ఒకవేళ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && n_అడ్డువరుస_Est
  ){
    Bitmask not_వాడిన;
    పూర్ణాంకం rc = where_మార్గం_Satisfies_క్రమం_By(pWInfo, pWInfo->p_ఫలితం_సమితి, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &not_వాడిన);
    ఒకవేళ( rc==pWInfo->p_ఫలితం_సమితి->n_సమాసం ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  ఒకవేళ( pWInfo->p_క్రమం_By ){
    pWInfo->nOBSat = pFrom->isOrdered;
    ఒకవేళ( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      ఒకవేళ( pFrom->isOrdered==pWInfo->p_క్రమం_By->n_సమాసం ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      ప్రతిజ్ఞా( pWInfo->pSelect->p_క్రమం_By==0
           || pWInfo->nOBSat <= pWInfo->pSelect->p_క్రమం_By->n_సమాసం );
    }లేకపోతే{
      pWInfo->rev_ముసుగు = pFrom->revLoop;
      ఒకవేళ( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        ఒకవేళ( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          ఒకవేళ( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_స్తంభం_లోపల))!=(WHERE_IPK|WHERE_స్తంభం_లోపల)
          ){
            Bitmask m = 0;
            పూర్ణాంకం rc = where_మార్గం_Satisfies_క్రమం_By(pWInfo, pWInfo->p_క్రమం_By, pFrom,
                      WHERE_ORDERBY_హద్దు, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_స్తంభం_లోపల );
            ఒకవేళ( rc==pWInfo->p_క్రమం_By->n_సమాసం ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->rev_ముసుగు = m;
            }
          }
        }
      }లేకపోతే ఒకవేళ( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    ఒకవేళ( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->p_క్రమం_By->n_సమాసం && nLoop>0
    ){
      Bitmask rev_ముసుగు = 0;
      పూర్ణాంకం n_క్రమం = where_మార్గం_Satisfies_క్రమం_By(pWInfo, pWInfo->p_క్రమం_By,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &rev_ముసుగు
      );
      ప్రతిజ్ఞా( pWInfo->sorted==0 );
      ఒకవేళ( n_క్రమం==pWInfo->p_క్రమం_By->n_సమాసం ){
        pWInfo->sorted = 1;
        pWInfo->rev_ముసుగు = rev_ముసుగు;
      }
    }
  }

  pWInfo->n_అడ్డువరుస_బహిర్గతం = pFrom->n_అడ్డువరుస;
#నిర్వచితమైతే WHERETRACE_ENABLED
  pWInfo->r_మొత్తం_ఖర్చు = pFrom->r_ఖర్చు;
#అయితే_ముగింపు

  /* Free temporary memory and return success */
  sqlite3_దొంతర_Free_NN(pParse->db, pSpace);
  తిరిగి SQLITE_OK;
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
స్థిర SQLITE_NOINLINE శూన్య whereInterstageHeuristic(WhereInfo *pWInfo){
  పూర్ణాంకం i;
#నిర్వచితమైతే WHERETRACE_ENABLED
  పూర్ణాంకం once = 0;
#అయితే_ముగింపు
  కొరకు(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    ఒకవేళ( p==0 ) ఆపు;
    ఒకవేళ( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      ఆపు;
    }
    ఒకవేళ( (p->wsFlags & (WHERE_స్తంభం_EQ|WHERE_స్తంభం_NULL|WHERE_స్తంభం_లోపల))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      కొరకు(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_తదుపరి_Loop){
        ఒకవేళ( pLoop->iTab!=iTab ) కొనసాగు;
        ఒకవేళ( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_సూచీ))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          కొనసాగు;
        }
#నిర్వచితమైతే WHERETRACE_ENABLED
        ఒకవేళ( sqlite3WhereTrace & 0x80 ){
          ఒకవేళ( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#అయితే_ముగింపు /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }లేకపోతే{
      ఆపు;
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
స్థిర పూర్ణాంకం whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  Src_అంశం *p_అంశం;
  WhereClause *pWC;
  Where_పదం *p_పదం;
  WhereLoop *pLoop;
  పూర్ణాంకం iCur;
  పూర్ణాంకం j;
  పట్టిక *pTab;
  సూచీ *pIdx;
  Where_పరిశీలన పరిశీలన;

  pWInfo = pBuilder->pWInfo;
  ఒకవేళ( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) తిరిగి 0;
  ప్రతిజ్ఞా( pWInfo->pTabList->nSrc>=1 );
  p_అంశం = pWInfo->pTabList->a;
  pTab = p_అంశం->pSTab;
  ఒకవేళ( IsVirtual(pTab) ) తిరిగి 0;
  ఒకవేళ( p_అంశం->fg.is_సూచీకృత_By || p_అంశం->fg.not_సూచీకృత ){
    testcase( p_అంశం->fg.is_సూచీకృత_By );
    testcase( p_అంశం->fg.not_సూచీకృత );
    తిరిగి 0;
  }
  iCur = p_అంశం->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  p_పదం = where_పరిశీలన_Init(&పరిశీలన, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  అయితే( p_పదం && p_పదం->prereq_కుడి ) p_పదం = where_పరిశీలన_తదుపరి(&పరిశీలన);
  ఒకవేళ( p_పదం ){
    testcase( p_పదం->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_స్తంభం_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = p_పదం;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->r_నిరంతరం = 33;  /* 33==sqlite3LogEst(10) */
  }లేకపోతే{
    కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
      పూర్ణాంకం op_ముసుగు;
      ప్రతిజ్ఞా( pLoop->aLTermSpace==pLoop->aLTerm );
      ఒకవేళ( !Is_Unique_సూచీ(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_కీలకం_నిలువు>Array_కొలత(pLoop->aLTermSpace)
      ) కొనసాగు;
      op_ముసుగు = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      కొరకు(j=0; j<pIdx->n_కీలకం_నిలువు; j++){
        p_పదం = where_పరిశీలన_Init(&పరిశీలన, pWC, iCur, j, op_ముసుగు, pIdx);
        అయితే( p_పదం && p_పదం->prereq_కుడి ) p_పదం = where_పరిశీలన_తదుపరి(&పరిశీలన);
        ఒకవేళ( p_పదం==0 ) ఆపు;
        testcase( p_పదం->eOperator & WO_IS );
        pLoop->aLTerm[j] = p_పదం;
      }
      ఒకవేళ( j!=pIdx->n_కీలకం_నిలువు ) కొనసాగు;
      pLoop->wsFlags = WHERE_స్తంభం_EQ|WHERE_ONEROW|WHERE_సూచీకృత;
      ఒకవేళ( pIdx->isCovering || (p_అంశం->నిలువు_వాడిన & pIdx->నిలువు_Not_Idxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_సూచీ = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->r_నిరంతరం = 39;  /* 39==sqlite3LogEst(15) */
      ఆపు;
    }
  }
  ఒకవేళ( pLoop->wsFlags ){
    pLoop->n_బహిర్గతం = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    ప్రతిజ్ఞా( pWInfo->s_ముసుగు_సమితి.n==1 && iCur==pWInfo->s_ముసుగు_సమితి.ix[0] );
    pLoop->ముసుగు_Self = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->n_అడ్డువరుస_బహిర్గతం = 1;
    ఒకవేళ( pWInfo->p_క్రమం_By ) pWInfo->nOBSat =  pWInfo->p_క్రమం_By->n_సమాసం;
    ఒకవేళ( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    ఒకవేళ( పరిశీలన.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#నిర్వచితమైతే SQLITE_DEBUG
    pLoop->cId = '0';
#అయితే_ముగింపు
#నిర్వచితమైతే WHERETRACE_ENABLED
    ఒకవేళ( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#అయితే_ముగింపు
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** Helper function for exprIsDeterministic().
*/
స్థిర పూర్ణాంకం సమాసం_కణుపు_Is_Deterministic(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_FUNCTION && సమాసం_Has_Property(p_సమాసం, EP_ConstFunc)==0 ){
    pWalker->e_సంకేతం = 0;
    తిరిగి WRC_Abort;
  }
  తిరిగి WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
స్థిర పూర్ణాంకం సమాసం_Is_Deterministic(సమాసం *p){
  Walker w;
  memset(&w, 0, పరిమాణం(w));
  w.e_సంకేతం = 1;
  w.x_సమాసం_Callback = సమాసం_కణుపు_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.e_సంకేతం;
}

 
#నిర్వచితమైతే WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
స్థిర శూన్య showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  ఒకవేళ( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    పూర్ణాంకం i;
    స్థిర స్థిరరాశి అక్షరం zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    కొరకు(p=pWInfo->pLoops, i=0; p; p=p->p_తదుపరి_Loop, i++){
      p->cId = zLabel[i%(పరిమాణం(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#లేకపోతే
# define WHERETRACE_ALL_LOOPS(W,C)
#అయితే_ముగింపు

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
స్థిర SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  పూర్ణాంకం i;
  Bitmask tab_వాడిన;
  పూర్ణాంకం has_కుడి_Join;

  /* Preconditions checked by the caller */
  ప్రతిజ్ఞా( pWInfo->nLevel>=2 );
  ప్రతిజ్ఞా( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  ప్రతిజ్ఞా( pWInfo->p_ఫలితం_సమితి!=0 );
  ప్రతిజ్ఞా( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tab_వాడిన = sqlite3_Where_సమాసం_List_Usage(&pWInfo->s_ముసుగు_సమితి, pWInfo->p_ఫలితం_సమితి);
  ఒకవేళ( pWInfo->p_క్రమం_By ){
    tab_వాడిన |= sqlite3_Where_సమాసం_List_Usage(&pWInfo->s_ముసుగు_సమితి, pWInfo->p_క్రమం_By);
  }
  has_కుడి_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  కొరకు(i=pWInfo->nLevel-1; i>=1; i--){
    Where_పదం *p_పదం, *p_చివర;
    Src_అంశం *p_అంశం;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    p_అంశం = &pWInfo->pTabList->a[pLoop->iTab];
    ఒకవేళ( (p_అంశం->fg.jointype & (JT_ఎడమ|JT_కుడి))!=JT_ఎడమ ) కొనసాగు;
    ఒకవేళ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      కొనసాగు;
    }
    ఒకవేళ( (tab_వాడిన & pLoop->ముసుగు_Self)!=0 ) కొనసాగు;
    p_చివర = pWInfo->sWC.a + pWInfo->sWC.n_పదం;
    కొరకు(p_పదం=pWInfo->sWC.a; p_పదం<p_చివర; p_పదం++){
      ఒకవేళ( (p_పదం->prereqAll & pLoop->ముసుగు_Self)!=0 ){
        ఒకవేళ( !సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON)
         || p_పదం->p_సమాసం->w.iJoin!=p_అంశం->iCursor
        ){
          ఆపు;
        }
      }
      ఒకవేళ( has_కుడి_Join
       && సమాసం_Has_Property(p_పదం->p_సమాసం, EP_InnerON)
       && NEVER(p_పదం->p_సమాసం->w.iJoin==p_అంశం->iCursor)
      ){
        ఆపు;  /* restriction (5) */
      }
    }
    ఒకవేళ( p_పదం<p_చివర ) కొనసాగు;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->rev_ముసుగు>>1) & ~m1)!=0 );
    pWInfo->rev_ముసుగు = (m1 & pWInfo->rev_ముసుగు) | ((pWInfo->rev_ముసుగు>>1) & ~m1);
    notReady &= ~pLoop->ముసుగు_Self;
    కొరకు(p_పదం=pWInfo->sWC.a; p_పదం<p_చివర; p_పదం++){
      ఒకవేళ( (p_పదం->prereqAll & pLoop->ముసుగు_Self)!=0 ){
        p_పదం->wtFlags |= పదం_CODED;
        p_పదం->prereqAll = 0;
      }
    }
    ఒకవేళ( i!=pWInfo->nLevel-1 ){
      పూర్ణాంకం nByte = (pWInfo->nLevel-1-i) * పరిమాణం(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    ప్రతిజ్ఞా( pWInfo->nLevel>0 );
  }
  తిరిగి notReady;
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
స్థిర SQLITE_NOINLINE శూన్య whereCheckIfBloomFilterIsUseful(
  స్థిరరాశి WhereInfo *pWInfo
){
  పూర్ణాంకం i;
  LogEst n_శోధన = 0;

  ప్రతిజ్ఞా( pWInfo->nLevel>=2 );
  ప్రతిజ్ఞా( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  కొరకు(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    స్థిరరాశి గుర్తులేని పూర్ణాంకం reqFlags = (WHERE_SELFCULL|WHERE_స్తంభం_EQ);
    Src_అంశం *p_అంశం = &pWInfo->pTabList->a[pLoop->iTab];
    పట్టిక *pTab = p_అంశం->pSTab;
    ఒకవేళ( (pTab->tabFlags & TF_HasStat1)==0 ) ఆపు;
    pTab->tabFlags |= TF_MaybeReanalyze;
    ఒకవేళ( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_శోధన > pTab->n_అడ్డువరుస_Log_Est
    ){
      testcase( p_అంశం->fg.jointype & JT_ఎడమ );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (ద్విగుణ)sqlite3_Log_Est_గమ్యం_Int(n_శోధన), pTab->zName,
         (ద్విగుణ)sqlite3_Log_Est_గమ్యం_Int(pTab->n_అడ్డువరుస_Log_Est)));
    }
    n_శోధన += pLoop->n_బహిర్గతం;
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
స్థిర SQLITE_NOINLINE శూన్య where_కూడు_సూచీకృత_సమాసం(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  సూచీ *pIdx,       /* The index-on-expression that contains the expressions */
  పూర్ణాంకం iIdxCur,       /* Cursor number for pIdx */
  Src_అంశం *p_Tab_అంశం  /* The FROM clause entry for the table */
){
  పూర్ణాంకం i;
  సూచీకృత_సమాసం *p;
  పట్టిక *pTab;
  ప్రతిజ్ఞా( pIdx->b_Has_సమాసం );
  pTab = pIdx->p_పట్టిక;
  కొరకు(i=0; i<pIdx->n_స్తంభం; i++){
    సమాసం *p_సమాసం;
    పూర్ణాంకం j = pIdx->ai_స్తంభం[i];
    ఒకవేళ( j==XN_సమాసం ){
      p_సమాసం = pIdx->a_నిలువు_సమాసం->a[i].p_సమాసం;
    }లేకపోతే ఒకవేళ( j>=0 && (pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_VIRTUAL)!=0 ){
      p_సమాసం = sqlite3_స్తంభం_సమాసం(pTab, &pTab->a_నిలువు[j]);
    }లేకపోతే{
      కొనసాగు;
    }
    ఒకవేళ( sqlite3_సమాసం_Is_Constant(0,p_సమాసం) ) కొనసాగు;
    p = sqlite3DbMallocRaw(pParse->db,  పరిమాణం(సూచీకృత_సమాసం));
    ఒకవేళ( p==0 ) ఆపు;
    p->pIENext = pParse->pIdxEpr;
#నిర్వచితమైతే WHERETRACE_ENABLED
    ఒకవేళ( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      ఒకవేళ( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_సమాసం(p_సమాసం);
    }
#అయితే_ముగింపు
    p->p_సమాసం = sqlite3_సమాసం_Dup(pParse->db, p_సమాసం, 0);
    p->i_దత్తాంశం_Cur = p_Tab_అంశం->iCursor;
    p->iIdxCur = iIdxCur;
    p->i_Idx_నిలువు = i;
    p->b_Maybe_Null_అడ్డువరుస = (p_Tab_అంశం->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి))!=0;
    ఒకవేళ( sqlite3_సూచీ_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->z_నిలువు_Aff[i];
    }
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#అయితే_ముగింపు
    pParse->pIdxEpr = p;
    ఒకవేళ( p->pIENext==0 ){
      శూన్య *pArg = (శూన్య*)&pParse->pIdxEpr;
      sqlite3_Parser_కూడు_Cleanup(pParse, where_సూచీకృత_సమాసం_Cleanup, pArg);
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
స్థిర SQLITE_NOINLINE శూన్య where_తిరగేయి_పరిశీలన_క్రమం(WhereInfo *pWInfo){
  పూర్ణాంకం ii;
  కొరకు(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    Src_అంశం *p_అంశం = &pWInfo->pTabList->a[ii];
    ఒకవేళ( !p_అంశం->fg.isCte
     || p_అంశం->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(p_అంశం->fg.isSubquery==0)
     || p_అంశం->u4.pSubq->pSelect->p_క్రమం_By==0
    ){
      pWInfo->rev_ముసుగు |= MASKBIT(ii);
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
  సమాసం *pWhere,           /* The WHERE clause */
  సమాసం_List *p_క్రమం_By,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  సమాసం_List *p_ఫలితం_సమితి,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  పూర్ణాంకం iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  పూర్ణాంకం nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  పూర్ణాంకం nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  Where_ముసుగు_సమితి *p_ముసుగు_సమితి;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  పూర్ణాంకం ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  పూర్ణాంకం rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  ప్రతిజ్ఞా( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  ప్రతిజ్ఞా( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_హద్దు)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, పరిమాణం(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( p_క్రమం_By && p_క్రమం_By->n_సమాసం==BMS-1 );
  ఒకవేళ( p_క్రమం_By && p_క్రమం_By->n_సమాసం>=BMS ){
    p_క్రమం_By = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  ఒకవేళ( pTabList->nSrc>BMS ){
    sqlite3_Error_సందేశం(pParse, "at most %d tables in a join", BMS);
    తిరిగి 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + పరిమాణం(WhereLoop));
  ఒకవేళ( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    వెళ్ళు whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->p_క్రమం_By = p_క్రమం_By;
#ఒకవేళ WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#అయితే_ముగింపు
  pWInfo->p_ఫలితం_సమితి = p_ఫలితం_సమితి;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->i_హద్దు = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  memset(&pWInfo->a[0], 0, పరిమాణం(WhereLoop)+nTabList*పరిమాణం(WhereLevel));
  ప్రతిజ్ఞా( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  p_ముసుగు_సమితి = &pWInfo->s_ముసుగు_సమితి;
  p_ముసుగు_సమితి->n = 0;
  p_ముసుగు_సమితి->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((అక్షరం*)pWInfo)+nByteWInfo);
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#నిర్వచితమైతే SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#అయితే_ముగింపు

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3_Where_విభజించు(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  ఒకవేళ( nTabList==0 ){
    ఒకవేళ( p_క్రమం_By ) pWInfo->nOBSat = p_క్రమం_By->n_సమాసం;
    ఒకవేళ( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    ఒకవేళ( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_విలువ)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }లేకపోతే{
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
    చేయి{
      create_ముసుగు(p_ముసుగు_సమితి, pTabList->a[ii].iCursor);
      sqlite3_Where_Tab_Func_వాదనలు(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }అయితే( (++ii)<pTabList->nSrc );
  #నిర్వచితమైతే SQLITE_DEBUG
    {
      Bitmask mx = 0;
      కొరకు(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3_Where_Get_ముసుగు(p_ముసుగు_సమితి, pTabList->a[ii].iCursor);
        ప్రతిజ్ఞా( m>=mx );
        mx = m;
      }
    }
  #అయితే_ముగింపు
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_సమాసం_Analyze(pTabList, &pWInfo->sWC);
  ఒకవేళ( pSelect && pSelect->p_హద్దు ){
    sqlite3_Where_కూడు_హద్దు(&pWInfo->sWC, pSelect);
  }
  ఒకవేళ( pParse->nErr ) వెళ్ళు whereBeginError;

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
  కొరకు(ii=0; ii<sWLB.pWC->n_పునాది; ii++){
    Where_పదం *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    సమాసం *pX;                          /* The expression of pT */
    ఒకవేళ( pT->wtFlags & పదం_VIRTUAL ) కొనసాగు;
    pX = pT->p_సమాసం;
    ప్రతిజ్ఞా( pX!=0 );
    ప్రతిజ్ఞా( pT->prereqAll!=0 || !సమాసం_Has_Property(pX, EP_OuterON) );
    ఒకవేళ( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || సమాసం_Is_Deterministic(pX))   /* Condition (4) */
     && !(సమాసం_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_సమాసం_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= పదం_CODED;
    }
  }

  ఒకవేళ( wctrlFlags & WHERE_WANT_DISTINCT ){
    ఒకవేళ( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }లేకపోతే ఒకవేళ( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_ఫలితం_సమితి) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }లేకపోతే ఒకవేళ( p_క్రమం_By==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->p_క్రమం_By = p_ఫలితం_సమితి;
    }
  }

  /* Construct the WhereLoop objects */
#ఒకవేళ defined(WHERETRACE_ENABLED)
  ఒకవేళ( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    ఒకవేళ( wctrlFlags & WHERE_USE_హద్దు ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    ఒకవేళ( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, పరిమాణం(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.p_క్రమం_By = p_క్రమం_By;
      sSelect.pEList = p_ఫలితం_సమితి;
      sqlite3_వృక్షం_View_Select(0, &sSelect, 0);
    }
    ఒకవేళ( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#అయితే_ముగింపు

  ఒకవేళ( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = where_Loop_కూడు_All(&sWLB);
    ఒకవేళ( rc ) వెళ్ళు whereBeginError;

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    ఒకవేళ( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      అయితే( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_తదుపరి_Loop;
        whereLoopDelete(db, p);
      }
      rc = where_Loop_కూడు_All(&sWLB);
      ఒకవేళ( rc ) వెళ్ళు whereBeginError;
    }
#అయితే_ముగింపు
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    where_మార్గం_Solver(pWInfo, 0);
    ఒకవేళ( db->mallocFailed ) వెళ్ళు whereBeginError;
    ఒకవేళ( pWInfo->p_క్రమం_By ){
       whereInterstageHeuristic(pWInfo);
       where_మార్గం_Solver(pWInfo, pWInfo->n_అడ్డువరుస_బహిర్గతం<0 ? 1 : pWInfo->n_అడ్డువరుస_బహిర్గతం+1);
       ఒకవేళ( db->mallocFailed ) వెళ్ళు whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    ఒకవేళ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->n_అడ్డువరుస_బహిర్గతం, pWInfo->n_అడ్డువరుస_బహిర్గతం-30));
      pWInfo->n_అడ్డువరుస_బహిర్గతం -= 30;
    }

  }
  ప్రతిజ్ఞా( pWInfo->pTabList!=0 );
  ఒకవేళ( pWInfo->p_క్రమం_By==0 && (db->flags & SQLITE_తిరగేయి_క్రమం)!=0 ){
    where_తిరగేయి_పరిశీలన_క్రమం(pWInfo);
  }
  ఒకవేళ( pParse->nErr ){
    వెళ్ళు whereBeginError;
  }
  ప్రతిజ్ఞా( db->mallocFailed==0 );
#నిర్వచితమైతే WHERETRACE_ENABLED
  ఒకవేళ( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_మొత్తం_ఖర్చు, pWInfo->n_అడ్డువరుస_బహిర్గతం);
    ఒకవేళ( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->rev_ముసుగు);
    }
    ఎంపిక( pWInfo->eDistinct ){
      స్థితి WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        ఆపు;
      }
      స్థితి WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        ఆపు;
      }
      స్థితి WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        ఆపు;
      }
    }
    sqlite3DebugPrintf("\n");
    కొరకు(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#అయితే_ముగింపు

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
  ఒకవేళ( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && p_ఫలితం_సమితి!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    ప్రతిజ్ఞా( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  ఒకవేళ( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#ఒకవేళ defined(WHERETRACE_ENABLED)
  ఒకవేళ( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
    sqlite3DebugPrintf("---- WHERE clause at end of analysis:\n");
    sqlite3WhereClausePrint(sWLB.pWC);
  }
  WHERETRACE(0xffffffff,("*** Optimizer Finished ***\n"));
#అయితే_ముగింపు
  pWInfo->pParse->nQueryLoop += pWInfo->n_అడ్డువరుస_బహిర్గతం;

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
  ప్రతిజ్ఞా( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  ఒకవేళ( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    పూర్ణాంకం wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    పూర్ణాంకం bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    ప్రతిజ్ఞా( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    ఒకవేళ( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      ఒకవేళ( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        ఒకవేళ( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  కొరకు(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    పట్టిక *pTab;     /* Table to open */
    పూర్ణాంకం iDb;         /* Index of database containing table/index */
    Src_అంశం *p_Tab_అంశం;

    p_Tab_అంశం = &pTabList->a[pLevel->iFrom];
    pTab = p_Tab_అంశం->pSTab;
    iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    ఒకవేళ( ii==0 || (p_Tab_అంశం[0].fg.jointype & JT_ఎడమ)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }లేకపోతే ఒకవేళ( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }లేకపోతే{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    ఒకవేళ( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }లేకపోతే
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    ఒకవేళ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      స్థిరరాశి అక్షరం *pVTab = (స్థిరరాశి అక్షరం *)sqlite3GetVTable(db, pTab);
      పూర్ణాంకం iCur = p_Tab_అంశం->iCursor;
      sqlite3_Vdbe_కూడు_Op4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }లేకపోతే ఒకవేళ( IsVirtual(pTab) ){
      /* noop */
    }లేకపోతే
#అయితే_ముగింపు
    ఒకవేళ( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (p_Tab_అంశం->fg.jointype & (JT_LTORJ|JT_కుడి))!=0
    ){
      పూర్ణాంకం op = OP_OpenRead;
      ఒకవేళ( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = p_Tab_అంశం->iCursor;
      };
      sqlite3_Open_పట్టిక(pParse, p_Tab_అంశం->iCursor, iDb, pTab, op);
      ప్రతిజ్ఞా( p_Tab_అంశం->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_నిలువు==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_నిలువు==BMS );
      ఒకవేళ( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->n_నిలువు<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_సూచీ|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = p_Tab_అంశం->నిలువు_వాడిన;
        పూర్ణాంకం n = 0;
        కొరకు(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_గమ్యం_PTR(n), P4_INT32);
        ప్రతిజ్ఞా( n<=pTab->n_నిలువు );
      }
#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
      ఒకవేళ( pLoop->u.btree.p_సూచీ!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }లేకపోతే
#అయితే_ముగింపు
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_వాడిన_ముసుగు
      sqlite3_Vdbe_కూడు_Op3(v, OP_Columns_వాడిన, p_Tab_అంశం->iCursor,
                    LOWER32(p_Tab_అంశం->నిలువు_వాడిన), UPPER32(p_Tab_అంశం->నిలువు_వాడిన));
#అయితే_ముగింపు
      ఒకవేళ( ii>=2
       && (p_Tab_అంశం[0].fg.jointype & (JT_LTORJ|JT_ఎడమ))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_If_ఖాళీలు, p_Tab_అంశం->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }లేకపోతే{
      sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    ఒకవేళ( pLoop->wsFlags & WHERE_సూచీకృత ){
      సూచీ *pIx = pLoop->u.btree.p_సూచీ;
      పూర్ణాంకం i_సూచీ_Cur;
      పూర్ణాంకం op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      ప్రతిజ్ఞా( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      ఒకవేళ( !HasRowid(pTab) && Is_Primary_కీలకం_సూచీ(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_సూచీ_Cur = pLevel->iTabCur;
        op = 0;
      }లేకపోతే ఒకవేళ( pWInfo->eOnePass!=ONEPASS_OFF ){
        సూచీ *pJ = p_Tab_అంశం->pSTab->p_సూచీ;
        i_సూచీ_Cur = iAuxArg;
        ప్రతిజ్ఞా( wctrlFlags & WHERE_ONEPASS_DESIRED );
        అయితే( ALWAYS(pJ) && pJ!=pIx ){
          i_సూచీ_Cur++;
          pJ = pJ->p_తదుపరి;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_సూచీ_Cur;
      }లేకపోతే ఒకవేళ( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_సూచీ_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }లేకపోతే{
        i_సూచీ_Cur = pParse->nTab++;
        ఒకవేళ( pIx->b_Has_సమాసం && OptimizationEnabled(db, SQLITE_సూచీకృత_సమాసం) ){
          where_కూడు_సూచీకృత_సమాసం(pParse, pIx, i_సూచీ_Cur, p_Tab_అంశం);
        }
        ఒకవేళ( pIx->pPartIdxWhere && (p_Tab_అంశం->fg.jointype & JT_కుడి)==0 ){
          where_Part_Idx_సమాసం(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_సూచీ_Cur, p_Tab_అంశం
          );
        }
      }
      pLevel->iIdxCur = i_సూచీ_Cur;
      ప్రతిజ్ఞా( pIx!=0 );
      ప్రతిజ్ఞా( pIx->pSchema==pTab->pSchema );
      ప్రతిజ్ఞా( i_సూచీ_Cur>=0 );
      ఒకవేళ( op ){
        sqlite3_Vdbe_కూడు_Op3(v, op, i_సూచీ_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIx);
        ఒకవేళ( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_స్తంభం_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_క్రమీకరించు)==0
         && (pLoop->wsFlags & WHERE_లోపల_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_వాడిన_ముసుగు
        {
          u64 నిలువు_వాడిన = 0;
          పూర్ణాంకం ii, jj;
          కొరకు(ii=0; ii<pIx->n_స్తంభం; ii++){
            jj = pIx->ai_స్తంభం[ii];
            ఒకవేళ( jj<0 ) కొనసాగు;
            ఒకవేళ( jj>63 ) jj = 63;
            ఒకవేళ( (p_Tab_అంశం->నిలువు_వాడిన & MASKBIT(jj))==0 ) కొనసాగు;
            నిలువు_వాడిన |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3_Vdbe_కూడు_Op3(v, OP_Columns_వాడిన, i_సూచీ_Cur,
                            LOWER32(నిలువు_వాడిన),UPPER32(నిలువు_వాడిన));
        }
#అయితే_ముగింపు /* SQLITE_ENABLE_COLUMN_USED_MASK */
#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
        ఒకవేళ( HasRowid(pTab) ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_CursorHint, i_సూచీ_Cur, 0, p_Tab_అంశం->iCursor);
        }
#అయితే_ముగింపు
      }
    }
    ఒకవేళ( iDb>=0 ) sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
    ఒకవేళ( (p_Tab_అంశం->fg.jointype & JT_కుడి)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, పరిమాణం(Where_కుడి_Join)))!=0
    ){
      Where_కుడి_Join *pRJ = pLevel->pRJ;
      పూర్ణాంకం bBloomable = 0;
      pRJ->i_సరిపోలిక = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, pRJ->regReturn);
      ప్రతిజ్ఞా( pTab==p_Tab_అంశం->pSTab );
      ఒకవేళ( HasRowid(pTab) ){
        కీలకం_Info *pInfo;
        sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, pRJ->i_సరిపోలిక, 1);
        bBloomable = 1;
        pInfo = sqlite3_కీలకం_Info_Alloc(pParse->db, 1, 0);
        ఒకవేళ( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->a_క్రమీకరించు_Flags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }లేకపోతే{
        సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
        sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, pRJ->i_సరిపోలిక, pPk->n_కీలకం_నిలువు);
        sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pPk);
        bBloomable = sqlite3_సూచీ_Bloomable(pPk,pPk->n_కీలకం_నిలువు);
      }
      ఒకవేళ( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3_Vdbe_కూడు_Op2(v, OP_Blob, 65536, pRJ->regBloom);
      }లేకపోతే{
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
  pWInfo->i_శిఖరం = sqlite3VdbeCurrentAddr(v);
  ఒకవేళ( db->mallocFailed ) వెళ్ళు whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  కొరకు(ii=0; ii<nTabList; ii++){
    పూర్ణాంకం addrExplain;
    పూర్ణాంకం wsFlags;
    Src_అంశం *pSrc;
    ఒకవేళ( pParse->nErr ) వెళ్ళు whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    ఒకవేళ( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      పూర్ణాంకం iOnce = 0;
      ప్రతిజ్ఞా( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      ఒకవేళ( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
      }లేకపోతే{
        iOnce = 0;
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      ఒకవేళ( iOnce )  sqlite3_Vdbe_Jump_ఇక్కడ(v, iOnce);
    }
    ప్రతిజ్ఞా( pTabList == pWInfo->pTabList );
    ఒకవేళ( (wsFlags & (WHERE_AUTO_సూచీ|WHERE_BLOOMFILTER))!=0 ){
      ఒకవేళ( (wsFlags & WHERE_AUTO_సూచీ)!=0 ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOMATIC_సూచీ
        construct_Automatic_సూచీ(pParse, &pWInfo->sWC, notReady, pLevel);
#అయితే_ముగింపు
      }లేకపోతే{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      ఒకవేళ( db->mallocFailed ) వెళ్ళు whereBeginError;
    }
    addrExplain = sqlite3_Where_Explain_One_పరిశీలన(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3_Where_సంకేతం_One_Loop_ప్రారంభం(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    ఒకవేళ( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3_Where_కూడు_పరిశీలన_స్థితి(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->i_చివర_Where = sqlite3VdbeCurrentAddr(v);
  తిరిగి pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  ఒకవేళ( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#నిర్వచితమైతే WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#అయితే_ముగింపు /* WHERETRACE_ENABLED */
  తిరిగి 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#నిర్వచితం_కాకపోతే SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#లేకపోతే
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  స్థిర శూన్య sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    పూర్ణాంకం pc,
    VdbeOp *pOp
  ){
    ఒకవేళ( (db->flags & SQLITE_VdbeAddopTrace)==0 ) తిరిగి;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3_Show_Where_పదం(0); /* So compiler won't complain about unused func */
  }
#అయితే_ముగింపు

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
శూన్య sqlite3_Where_చివర(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం i_చివర = sqlite3VdbeCurrentAddr(v);
  పూర్ణాంకం nRJ = 0;
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  పూర్ణాంకం addrSeek = 0;
#అయితే_ముగింపు

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  కొరకు(i=pWInfo->nLevel-1; i>=0; i--){
    పూర్ణాంకం addr;
    pLevel = &pWInfo->a[i];
    ఒకవేళ( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_కుడి_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->చివర_Subrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    ఒకవేళ( pLevel->op!=OP_Noop ){
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      సూచీ *pIdx;
      పూర్ణాంకం n;
      ఒకవేళ( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_సూచీకృత)!=0
       && (pIdx = pLoop->u.btree.p_సూచీ)->hasStat1
       && (n = pLoop->u.btree.n_Distinct_నిలువు)>0
       && pIdx->ai_అడ్డువరుస_Log_Est[n]>=36
      ){
        పూర్ణాంకం r1 = pParse->nMem+1;
        పూర్ణాంకం j, op;
        పూర్ణాంకం addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        ఒకవేళ( pLevel->i_ఎడమ_Join ){
          addrIfNull = sqlite3_Vdbe_కూడు_Op2(v, OP_If_Null_అడ్డువరుస, pLevel->iIdxCur, r1);
        }
        కొరకు(j=0; j<n; j++){
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3_Vdbe_కూడు_Op4_Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 1, pLevel->p2);
        ఒకవేళ( pLevel->i_ఎడమ_Join ){
          sqlite3_Vdbe_Jump_ఇక్కడ(v, addrIfNull);
        }
      }
#అయితే_ముగింపు /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    ఒకవేళ( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }లేకపోతే ఒకవేళ( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      పూర్ణాంకం iBypass = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
      ప్రతిజ్ఞా( pLoop->wsFlags & WHERE_AUTO_సూచీ );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3_Vdbe_కూడు_Op1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3_Vdbe_Jump_ఇక్కడ(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    ఒకవేళ( pLevel->op!=OP_Noop ){
      sqlite3_Vdbe_కూడు_Op3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_తదుపరి);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      ఒకవేళ( addrSeek ){
        sqlite3_Vdbe_Jump_ఇక్కడ(v, addrSeek);
        addrSeek = 0;
      }
#అయితే_ముగింపు
      ఒకవేళ( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3_Vdbe_కూడు_Op2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    ఒకవేళ( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#నిర్వచితం_కాకపోతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
    ప్రతిజ్ఞా( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_లోపల_ABLE) && pLevel->u.లోపల.n_లోపల>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    ఒకవేళ( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3_Vdbe_కూడు_Op2(v, OP_DecrJumpZero, (పూర్ణాంకం)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#అయితే_ముగింపు
    ఒకవేళ( (pLoop->wsFlags & WHERE_లోపల_ABLE)!=0 && pLevel->u.లోపల.n_లోపల>0 ){
      నిర్మాణం లోపల_Loop *p_లోపల;
      పూర్ణాంకం j;
      కొరకు(j=pLevel->u.లోపల.n_లోపల, p_లోపల=&pLevel->u.లోపల.a_లోపల_Loop[j-1]; j>0; j--, p_లోపల--){
        ప్రతిజ్ఞా( sqlite3VdbeGetOp(v, p_లోపల->addr_లోపల_శిఖరం+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3_Vdbe_Jump_ఇక్కడ(v, p_లోపల->addr_లోపల_శిఖరం+1);
        ఒకవేళ( p_లోపల->e_చివర_Loop_Op!=OP_Noop ){
          ఒకవేళ( p_లోపల->n_ఉపసర్గ ){
            పూర్ణాంకం b_Early_బహిర్గతం =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_లోపల_EARLYOUT)!=0;
            ఒకవేళ( pLevel->i_ఎడమ_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3_Vdbe_కూడు_Op3(v, OP_If_Null_అడ్డువరుస, p_లోపల->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + b_Early_బహిర్గతం, p_లోపల->i_పునాది);
              VdbeCoverage(v);
            }
            ఒకవేళ( b_Early_బహిర్గతం ){
              sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  p_లోపల->i_పునాది, p_లోపల->n_ఉపసర్గ);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3_Vdbe_Jump_ఇక్కడ(v, p_లోపల->addr_లోపల_శిఖరం+1);
            }
          }
          sqlite3_Vdbe_కూడు_Op2(v, p_లోపల->e_చివర_Loop_Op, p_లోపల->iCur, p_లోపల->addr_లోపల_శిఖరం);
          VdbeCoverage(v);
          VdbeCoverageIf(v, p_లోపల->e_చివర_Loop_Op==OP_Prev);
          VdbeCoverageIf(v, p_లోపల->e_చివర_Loop_Op==OP_తదుపరి);
        }
        sqlite3_Vdbe_Jump_ఇక్కడ(v, p_లోపల->addr_లోపల_శిఖరం-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    ఒకవేళ( pLevel->pRJ ){
      sqlite3_Vdbe_కూడు_Op3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    ఒకవేళ( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_సూచీ->zName));
      sqlite3_Vdbe_Jump_ఇక్కడ(v, pLevel->addrSkip);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, pLevel->addrSkip-2);
    }
    ఒకవేళ( pLevel->i_ఎడమ_Join ){
      పూర్ణాంకం ws = pLoop->wsFlags;
      addr = sqlite3_Vdbe_కూడు_Op1(v, OP_IfPos, pLevel->i_ఎడమ_Join); VdbeCoverage(v);
      ప్రతిజ్ఞా( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_సూచీకృత)!=0 );
      ఒకవేళ( (ws & WHERE_IDX_ONLY)==0 ){
        Src_అంశం *pSrc = &pTabList->a[pLevel->iFrom];
        ప్రతిజ్ఞా( pLevel->iTabCur==pSrc->iCursor );
        ఒకవేళ( pSrc->fg.ద్వారా_Coroutine ){
          పూర్ణాంకం m, n;
          ప్రతిజ్ఞా( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_ఫలితం;
          ప్రతిజ్ఞా( pSrc->pSTab!=0 );
          m = pSrc->pSTab->n_నిలువు;
          sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, pLevel->iTabCur);
      }
      ఒకవేళ( (ws & WHERE_సూచీకృత)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        ఒకవేళ( ws & WHERE_MULTI_OR ){
          సూచీ *pIx = pLevel->u.pCoveringIdx;
          పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pIx->pSchema);
          sqlite3_Vdbe_కూడు_Op3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIx);
        }
        sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, pLevel->iIdxCur);
      }
      ఒకవేళ( ws & WHERE_లోపల_ABLE ){
        పూర్ణాంకం m;
        కొరకు(m=0; m<pLevel->u.లోపల.n_లోపల; m++){
          నిర్మాణం లోపల_Loop *p_లోపల = &pLevel->u.లోపల.a_లోపల_Loop[m];
          ఒకవేళ( p_లోపల->e_చివర_Loop_Op!=OP_Noop ){
            sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, p_లోపల->iCur);
          }
        }
      }
      ఒకవేళ( pLevel->op==OP_Return ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, pLevel->p1, pLevel->addr_మొదటి);
      }లేకపోతే{
        sqlite3VdbeGoto(v, pLevel->addr_మొదటి);
      }
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  ప్రతిజ్ఞా( pWInfo->nLevel<=pTabList->nSrc );
  కొరకు(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    పూర్ణాంకం k, చివరి_2;
    VdbeOp *pOp, *p_చివరి_Op;
    సూచీ *pIdx = 0;
    Src_అంశం *p_Tab_అంశం = &pTabList->a[pLevel->iFrom];
    పట్టిక *pTab = p_Tab_అంశం->pSTab;
    ప్రతిజ్ఞా( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    ఒకవేళ( pLevel->pRJ ){
      sqlite3_Where_కుడి_Join_Loop(pWInfo, i, pLevel);
      కొనసాగు;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    ఒకవేళ( p_Tab_అంశం->fg.ద్వారా_Coroutine ){
      testcase( pParse->db->mallocFailed );
      ప్రతిజ్ఞా( p_Tab_అంశం->fg.isSubquery );
      ప్రతిజ్ఞా( p_Tab_అంశం->u4.pSubq->reg_ఫలితం>=0 );
      translate_స్తంభం_గమ్యం_Copy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            p_Tab_అంశం->u4.pSubq->reg_ఫలితం, 0);
      కొనసాగు;
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
    ఒకవేళ( pLoop->wsFlags & (WHERE_సూచీకృత|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_సూచీ;
    }లేకపోతే ఒకవేళ( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    ఒకవేళ( pIdx
     && !db->mallocFailed
    ){
      ఒకవేళ( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->p_పట్టిక) ){
        చివరి_2 = i_చివర;
      }లేకపోతే{
        చివరి_2 = pWInfo->i_చివర_Where;
      }
      ఒకవేళ( pIdx->b_Has_సమాసం ){
        సూచీకృత_సమాసం *p = pParse->pIdxEpr;
        అయితే( p ){
          ఒకవేళ( p->iIdxCur==pLevel->iIdxCur ){
#నిర్వచితమైతే WHERETRACE_ENABLED
            ఒకవేళ( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->i_Idx_నిలువు);
              ఒకవేళ( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_సమాసం(p->p_సమాసం);
            }
#అయితే_ముగింపు
            p->i_దత్తాంశం_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( db->flags & SQLITE_VdbeAddopTrace ){
        రాయి("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, చివరి_2-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      ప్రతిజ్ఞా( pOp->opcode!=OP_స్తంభం || pOp->p1!=pLevel->iTabCur );
      ప్రతిజ్ఞా( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      ప్రతిజ్ఞా( pOp->opcode!=OP_If_Null_అడ్డువరుస || pOp->p1!=pLevel->iTabCur );
#అయితే_ముగింపు
      pOp = sqlite3VdbeGetOp(v, k);
      p_చివరి_Op = pOp + (చివరి_2 - k);
      ప్రతిజ్ఞా( pOp<=p_చివరి_Op );
      చేయి{
        ఒకవేళ( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }లేకపోతే ఒకవేళ( pOp->opcode==OP_స్తంభం
#నిర్వచితమైతే SQLITE_ENABLE_స్థానభ్రంశం_SQL_FUNC
         || pOp->opcode==OP_స్థానభ్రంశం
#అయితే_ముగింపు
        ){
          పూర్ణాంకం x = pOp->p2;
          ప్రతిజ్ఞా( pIdx->p_పట్టిక==pTab );
#నిర్వచితమైతే SQLITE_ENABLE_స్థానభ్రంశం_SQL_FUNC
          ఒకవేళ( pOp->opcode==OP_స్థానభ్రంశం ){
            /* Do not need to translate the column number */
          }లేకపోతే
#అయితే_ముగింపు
          ఒకవేళ( !HasRowid(pTab) ){
            సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
            x = pPk->ai_స్తంభం[x];
            ప్రతిజ్ఞా( x>=0 );
          }లేకపోతే{
            testcase( x!=sqlite3_Storage_స్తంభం_గమ్యం_పట్టిక(pTab,x) );
            x = sqlite3_Storage_స్తంభం_గమ్యం_పట్టిక(pTab,x);
          }
          x = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pIdx, x);
          ఒకవేళ( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }లేకపోతే ఒకవేళ( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            ఒకవేళ( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_సందేశం(pParse, "internal query planner error");
              pParse->rc = SQLITE_అంతర్గత;
            }లేకపోతే{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3_Where_కూడు_Explain_పాఠ్యం(pParse,
                  pLevel->addrBody-1,
                  pTabList,
                  pLevel,
                  pWInfo->wctrlFlags
              );
            }
          }
        }లేకపోతే ఒకవేళ( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }లేకపోతే ఒకవేళ( pOp->opcode==OP_If_Null_అడ్డువరుస ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#నిర్వచితమైతే SQLITE_DEBUG
        k++;
#అయితే_ముగింపు
      }అయితే( (++pOp)<p_చివరి_Op );
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( db->flags & SQLITE_VdbeAddopTrace ) రాయి("TRANSLATE complete\n");
#అయితే_ముగింపు
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
  తిరిగి;
}
