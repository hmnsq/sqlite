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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Hidden_ਸੂਚਕਾਂਕ_Info Hidden_ਸੂਚਕਾਂਕ_Info;
ਬਣਤਰ Hidden_ਸੂਚਕਾਂਕ_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  ਪੂਰਨ eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 m_ਵਿੱਚ;                 /* Mask of terms that are <col> IN (...) */
  u32 m_Handle_ਵਿੱਚ;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_ਮੁੱਲ *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_ਸੂਚਕਾਂਕ_Info,aRhs) + (N)*ਆਕਾਰ(sqlite3_ਮੁੱਲ*))

/* Forward declaration of methods */
ਸਥਿਰ ਪੂਰਨ whereLoopResize(sqlite3*, WhereLoop*, ਪੂਰਨ);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_ਪੰਕਤੀ_ਗਿਣਤੀ(WhereInfo *pWInfo){
  ਵਾਪਸ pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
ਪੂਰਨ sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  ਵਾਪਸ pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
ਪੂਰਨ sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  ਵਾਪਸ pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
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
ਪੂਰਨ sqlite3_Where_ਕ੍ਰਮ_By_ਹੱਦਬੰਦੀ_Opt_Label(WhereInfo *pWInfo){
  WhereLevel *pInner;
  ਜੇ( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    ਵਾਪਸ pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  ਪ੍ਰਤਿਜ੍ਞਾ( pInner->addrNxt!=0 );
  ਵਾਪਸ pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
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
ਖਾਲੀ sqlite3_Where_Min_Max_Opt_Early_ਬਾਹਰਲਾ(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  ਪੂਰਨ i;
  ਜੇ( !pWInfo->bOrderedInnerLoop ) ਵਾਪਸ;
  ਜੇ( pWInfo->nOBSat==0 ) ਵਾਪਸ;
  ਲਈ(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    ਜੇ( (pInner->pWLoop->wsFlags & WHERE_ਕਾਲਮ_ਵਿੱਚ)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      ਵਾਪਸ;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
ਪੂਰਨ sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->iContinue!=0 );
  ਵਾਪਸ pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
ਪੂਰਨ sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  ਵਾਪਸ pWInfo->iBreak;
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
ਪੂਰਨ sqlite3WhereOkOnePass(WhereInfo *pWInfo, ਪੂਰਨ *aiCur){
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(aiCur, pWInfo->aiCurOnePass, ਆਕਾਰ(ਪੂਰਨ)*2);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  ਜੇ( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
ਪੂਰਨ sqlite3_Where_ਵਰਤਦਾ_Deferred_Seek(WhereInfo *pWInfo){
  ਵਾਪਸ pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
ਸਥਿਰ ਖਾਲੀ whereOrMove(Where_Or_ਸੈੱਟ *pDest, Where_Or_ਸੈੱਟ *pSrc){
  pDest->n = pSrc->n;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDest->a, pSrc->a, pDest->n*ਆਕਾਰ(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
ਸਥਿਰ ਪੂਰਨ where_Or_ਪਾਓ(
  Where_Or_ਸੈੱਟ *p_ਸੈੱਟ,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst r_ਲੜੀਵਾਰ,           /* Run-cost of the new entry */
  LogEst n_ਬਾਹਰਲਾ            /* Number of outputs for the new entry */
){
  u16 i;
  Where_Or_ਲਾਗਤ *p;
  ਲਈ(i=p_ਸੈੱਟ->n, p=p_ਸੈੱਟ->a; i>0; i--, p++){
    ਜੇ( r_ਲੜੀਵਾਰ<=p->r_ਲੜੀਵਾਰ && (prereq & p->prereq)==prereq ){
      ਜਾਓ where_Or_ਪਾਓ_ਪੂਰਾ;
    }
    ਜੇ( p->r_ਲੜੀਵਾਰ<=r_ਲੜੀਵਾਰ && (p->prereq & prereq)==p->prereq ){
      ਵਾਪਸ 0;
    }
  }
  ਜੇ( p_ਸੈੱਟ->n<N_OR_ਲਾਗਤ ){
    p = &p_ਸੈੱਟ->a[p_ਸੈੱਟ->n++];
    p->n_ਬਾਹਰਲਾ = n_ਬਾਹਰਲਾ;
  }ਨਹੀਂਤਾਂ{
    p = p_ਸੈੱਟ->a;
    ਲਈ(i=1; i<p_ਸੈੱਟ->n; i++){
      ਜੇ( p->r_ਲੜੀਵਾਰ>p_ਸੈੱਟ->a[i].r_ਲੜੀਵਾਰ ) p = p_ਸੈੱਟ->a + i;
    }
    ਜੇ( p->r_ਲੜੀਵਾਰ<=r_ਲੜੀਵਾਰ ) ਵਾਪਸ 0;
  }
where_Or_ਪਾਓ_ਪੂਰਾ:
  p->prereq = prereq;
  p->r_ਲੜੀਵਾਰ = r_ਲੜੀਵਾਰ;
  ਜੇ( p->n_ਬਾਹਰਲਾ>n_ਬਾਹਰਲਾ ) p->n_ਬਾਹਰਲਾ = n_ਬਾਹਰਲਾ;
  ਵਾਪਸ 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3_Where_Get_ਨਕਾਬ(Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ, ਪੂਰਨ iCursor){
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਨਕਾਬ_ਸੈੱਟ->n<=(ਪੂਰਨ)ਆਕਾਰ(Bitmask)*8 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਨਕਾਬ_ਸੈੱਟ->n>0 || p_ਨਕਾਬ_ਸੈੱਟ->ix[0]<0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iCursor>=-1 );
  ਜੇ( p_ਨਕਾਬ_ਸੈੱਟ->ix[0]==iCursor ){
    ਵਾਪਸ 1;
  }
  ਲਈ(i=1; i<p_ਨਕਾਬ_ਸੈੱਟ->n; i++){
    ਜੇ( p_ਨਕਾਬ_ਸੈੱਟ->ix[i]==iCursor ){
      ਵਾਪਸ MASKBIT(i);
    }
  }
  ਵਾਪਸ 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
ਖਾਲੀ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  Where_Mem_ਖੰਡ *p_ਖੰਡ;
  p_ਖੰਡ = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+ਆਕਾਰ(*p_ਖੰਡ));
  ਜੇ( p_ਖੰਡ ){
    p_ਖੰਡ->p_ਅਗਲਾ = pWInfo->p_Mem_ਮੰਜ਼ਿਲ_Free;
    p_ਖੰਡ->sz = nByte;
    pWInfo->p_Mem_ਮੰਜ਼ਿਲ_Free = p_ਖੰਡ;
    p_ਖੰਡ++;
  }
  ਵਾਪਸ (ਖਾਲੀ*)p_ਖੰਡ;
}
ਖਾਲੀ *sqlite3WhereRealloc(WhereInfo *pWInfo, ਖਾਲੀ *pOld, u64 nByte){
  ਖਾਲੀ *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  ਜੇ( pNew && pOld ){
    Where_Mem_ਖੰਡ *pOldBlk = (Where_Mem_ਖੰਡ*)pOld;
    pOldBlk--;
    ਪ੍ਰਤਿਜ੍ਞਾ( pOldBlk->sz<nByte );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew, pOld, pOldBlk->sz);
  }
  ਵਾਪਸ pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
ਸਥਿਰ ਖਾਲੀ create_ਨਕਾਬ(Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ, ਪੂਰਨ iCursor){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਨਕਾਬ_ਸੈੱਟ->n < Array_ਮਾਪ(p_ਨਕਾਬ_ਸੈੱਟ->ix) );
  p_ਨਕਾਬ_ਸੈੱਟ->ix[p_ਨਕਾਬ_ਸੈੱਟ->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
ਸਥਿਰ ਵਿਅੰਜਕ *where_ਸੱਜਾ_Subexpr_Is_ਕਾਲਮ(ਵਿਅੰਜਕ *p){
  p = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p->p_ਸੱਜਾ);
  ਜੇ( ALWAYS(p!=0) && p->op==TK_ਕਾਲਮ && !ਵਿਅੰਜਕ_Has_Property(p, EP_Fixed_ਖਾਨਾ) ){
    ਵਾਪਸ p;
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ SQLITE_NOINLINE ਅਚਲ ਅੱਖਰ *ਸੂਚਕਾਂਕ_ਵਿੱਚ_Affinity_Ok(
  Parse *pParse, 
  Where_ਪਦ *p_ਪਦ, 
  u8 idxaff
){
  ਵਿਅੰਜਕ *pX = p_ਪਦ->p_ਵਿਅੰਜਕ;
  ਵਿਅੰਜਕ inexpr;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->eOperator & WO_ਵਿੱਚ );

  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(pX) ){
    ਪੂਰਨ i_ਖੇਤਰ = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(pX->p_ਖੱਬਾ) ){
      i_ਖੇਤਰ = p_ਪਦ->u.x.i_ਖੇਤਰ - 1;
      inexpr.p_ਖੱਬਾ = pX->p_ਖੱਬਾ->x.pList->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ;
    }ਨਹੀਂਤਾਂ{
      inexpr.p_ਖੱਬਾ = pX->p_ਖੱਬਾ;
    }
    inexpr.p_ਸੱਜਾ = pX->x.pSelect->pEList->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ;
    pX = &inexpr;
  }

  ਜੇ( sqlite3_ਸੂਚਕਾਂਕ_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pX);
    ਵਾਪਸ pRet ? pRet->zName : sqlite3_Str_ਦੁਆਧਾਰੀ;
  }
  ਵਾਪਸ 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
ਸਥਿਰ Where_ਪਦ *where_ਜਾਂਚ_ਅਗਲਾ(Where_ਜਾਂਚ *p_ਜਾਂਚ){
  ਪੂਰਨ iCur;            /* The cursor on the LHS of the term */
  i16 i_ਕਾਲਮ;         /* The column on the LHS of the term.  -1 for IPK */
  ਵਿਅੰਜਕ *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  Where_ਪਦ *p_ਪਦ;    /* The term being tested */
  ਪੂਰਨ k = p_ਜਾਂਚ->k;    /* Where to start scanning */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਜਾਂਚ->iEquiv<=p_ਜਾਂਚ->nEquiv );
  pWC = p_ਜਾਂਚ->pWC;
  ਜਦੋਂ(1){
    i_ਕਾਲਮ = p_ਜਾਂਚ->ai_ਕਾਲਮ[p_ਜਾਂਚ->iEquiv-1];
    iCur = p_ਜਾਂਚ->aiCur[p_ਜਾਂਚ->iEquiv-1];
    ਪ੍ਰਤਿਜ੍ਞਾ( pWC!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( iCur>=0 );
    ਕਰੋ{
      ਲਈ(p_ਪਦ=pWC->a+k; k<pWC->n_ਪਦ; k++, p_ਪਦ++){
        ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 || p_ਪਦ->ਖੱਬਾ_Cursor<0 );
        ਜੇ( p_ਪਦ->ਖੱਬਾ_Cursor==iCur
         && p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ==i_ਕਾਲਮ
         && (i_ਕਾਲਮ!=XN_ਵਿਅੰਜਕ
             || sqlite3_ਵਿਅੰਜਕ_Compare_Skip(p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ,
                                       p_ਜਾਂਚ->p_Idx_ਵਿਅੰਜਕ,iCur)==0)
         && (p_ਜਾਂਚ->iEquiv<=1 || !ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON))
        ){
          ਜੇ( (p_ਪਦ->eOperator & WO_EQUIV)!=0
           && p_ਜਾਂਚ->nEquiv<Array_ਮਾਪ(p_ਜਾਂਚ->aiCur)
           && (pX = where_ਸੱਜਾ_Subexpr_Is_ਕਾਲਮ(p_ਪਦ->p_ਵਿਅੰਜਕ))!=0
          ){
            ਪੂਰਨ j;
            ਲਈ(j=0; j<p_ਜਾਂਚ->nEquiv; j++){
              ਜੇ( p_ਜਾਂਚ->aiCur[j]==pX->i_ਸਾਰਣੀ
               && p_ਜਾਂਚ->ai_ਕਾਲਮ[j]==pX->i_ਕਾਲਮ ){
                  ਰੋਕੋ;
              }
            }
            ਜੇ( j==p_ਜਾਂਚ->nEquiv ){
              p_ਜਾਂਚ->aiCur[j] = pX->i_ਸਾਰਣੀ;
              p_ਜਾਂਚ->ai_ਕਾਲਮ[j] = pX->i_ਕਾਲਮ;
              p_ਜਾਂਚ->nEquiv++;
            }
          }
          ਜੇ( (p_ਪਦ->eOperator & p_ਜਾਂਚ->op_ਨਕਾਬ)!=0 ){
            /* Verify the affinity and collating sequence match */
            ਜੇ( p_ਜਾਂਚ->zCollName && (p_ਪਦ->eOperator & WO_ISNULL)==0 ){
              ਅਚਲ ਅੱਖਰ *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = p_ਪਦ->p_ਵਿਅੰਜਕ;

              ਜੇ( (p_ਪਦ->eOperator & WO_ਵਿੱਚ) ){
                zCollName = ਸੂਚਕਾਂਕ_ਵਿੱਚ_Affinity_Ok(pParse, p_ਪਦ, p_ਜਾਂਚ->idxaff);
                ਜੇ( !zCollName ) ਜਾਰੀ;
              }ਨਹੀਂਤਾਂ{
                CollSeq *pColl;
                ਜੇ( !sqlite3_ਸੂਚਕਾਂਕ_Affinity_Ok(pX, p_ਜਾਂਚ->idxaff) ){
                  ਜਾਰੀ;
                }
                ਪ੍ਰਤਿਜ੍ਞਾ(pX->p_ਖੱਬਾ);
                pColl = sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_ਦੁਆਧਾਰੀ;
              }

              ਜੇ( sqlite3StrICmp(zCollName, p_ਜਾਂਚ->zCollName) ){
                ਜਾਰੀ;
              }
            }
            ਜੇ( (p_ਪਦ->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, ALWAYS(pX!=0))
             && pX->op==TK_ਕਾਲਮ
             && pX->i_ਸਾਰਣੀ==p_ਜਾਂਚ->aiCur[0]
             && pX->i_ਕਾਲਮ==p_ਜਾਂਚ->ai_ਕਾਲਮ[0]
            ){
              testcase( p_ਪਦ->eOperator & WO_IS );
              ਜਾਰੀ;
            }
            p_ਜਾਂਚ->pWC = pWC;
            p_ਜਾਂਚ->k = k+1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
            ਜੇ( (sqlite3WhereTrace & 0x20000)!=0 && p_ਜਾਂਚ->nEquiv>1 ){
              ਪੂਰਨ ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 p_ਜਾਂਚ->aiCur[0], p_ਜਾਂਚ->ai_ਕਾਲਮ[0], p_ਪਦ->i_ਪਦ);
              ਲਈ(ii=1; ii<p_ਜਾਂਚ->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   p_ਜਾਂਚ->aiCur[ii], p_ਜਾਂਚ->ai_ਕਾਲਮ[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#ਜੇ_ਅੰਤ
            ਵਾਪਸ p_ਪਦ;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }ਜਦੋਂ( pWC!=0 );
    ਜੇ( p_ਜਾਂਚ->iEquiv>=p_ਜਾਂਚ->nEquiv ) ਰੋਕੋ;
    pWC = p_ਜਾਂਚ->pOrigWC;
    k = 0;
    p_ਜਾਂਚ->iEquiv++;
  }
  ਵਾਪਸ 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
ਸਥਿਰ SQLITE_NOINLINE Where_ਪਦ *where_ਜਾਂਚ_Init_ਸੂਚਕਾਂਕ_ਵਿਅੰਜਕ(Where_ਜਾਂਚ *p_ਜਾਂਚ){
  p_ਜਾਂਚ->idxaff = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਜਾਂਚ->p_Idx_ਵਿਅੰਜਕ);
  ਵਾਪਸ where_ਜਾਂਚ_ਅਗਲਾ(p_ਜਾਂਚ);
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
ਸਥਿਰ Where_ਪਦ *where_ਜਾਂਚ_Init(
  Where_ਜਾਂਚ *p_ਜਾਂਚ,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  ਪੂਰਨ iCur,               /* Cursor to scan for */
  ਪੂਰਨ i_ਕਾਲਮ,            /* Column to scan for */
  u32 op_ਨਕਾਬ,             /* Operator(s) to scan for */
  ਸੂਚਕਾਂਕ *pIdx             /* Must be compatible with this index */
){
  p_ਜਾਂਚ->pOrigWC = pWC;
  p_ਜਾਂਚ->pWC = pWC;
  p_ਜਾਂਚ->p_Idx_ਵਿਅੰਜਕ = 0;
  p_ਜਾਂਚ->idxaff = 0;
  p_ਜਾਂਚ->zCollName = 0;
  p_ਜਾਂਚ->op_ਨਕਾਬ = op_ਨਕਾਬ;
  p_ਜਾਂਚ->k = 0;
  p_ਜਾਂਚ->aiCur[0] = iCur;
  p_ਜਾਂਚ->nEquiv = 1;
  p_ਜਾਂਚ->iEquiv = 1;
  ਜੇ( pIdx ){
    ਪੂਰਨ j = i_ਕਾਲਮ;
    i_ਕਾਲਮ = pIdx->ai_ਕਾਲਮ[j];
    ਜੇ( i_ਕਾਲਮ==pIdx->p_ਸਾਰਣੀ->iPKey ){
      i_ਕਾਲਮ = XN_ROWID;
    }ਨਹੀਂਤਾਂ ਜੇ( i_ਕਾਲਮ>=0 ){
      p_ਜਾਂਚ->idxaff = pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[i_ਕਾਲਮ].affinity;
      p_ਜਾਂਚ->zCollName = pIdx->azColl[j];
    }ਨਹੀਂਤਾਂ ਜੇ( i_ਕਾਲਮ==XN_ਵਿਅੰਜਕ ){
      p_ਜਾਂਚ->p_Idx_ਵਿਅੰਜਕ = pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[j].p_ਵਿਅੰਜਕ;
      p_ਜਾਂਚ->zCollName = pIdx->azColl[j];
      p_ਜਾਂਚ->ai_ਕਾਲਮ[0] = XN_ਵਿਅੰਜਕ;
      ਵਾਪਸ where_ਜਾਂਚ_Init_ਸੂਚਕਾਂਕ_ਵਿਅੰਜਕ(p_ਜਾਂਚ);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( i_ਕਾਲਮ==XN_ਵਿਅੰਜਕ ){
    ਵਾਪਸ 0;
  }
  p_ਜਾਂਚ->ai_ਕਾਲਮ[0] = i_ਕਾਲਮ;
  ਵਾਪਸ where_ਜਾਂਚ_ਅਗਲਾ(p_ਜਾਂਚ);
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
Where_ਪਦ *sqlite3_Where_ਲੱਭੋ_ਪਦ(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  ਪੂਰਨ iCur,             /* Cursor number of LHS */
  ਪੂਰਨ i_ਕਾਲਮ,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  ਸੂਚਕਾਂਕ *pIdx           /* Must be compatible with this index, if not NULL */
){
  Where_ਪਦ *p_ਨਤੀਜਾ = 0;
  Where_ਪਦ *p;
  Where_ਜਾਂਚ ਜਾਂਚ;

  p = where_ਜਾਂਚ_Init(&ਜਾਂਚ, pWC, iCur, i_ਕਾਲਮ, op, pIdx);
  op &= WO_EQ|WO_IS;
  ਜਦੋਂ( p ){
    ਜੇ( (p->prereq_ਸੱਜਾ & notReady)==0 ){
      ਜੇ( p->prereq_ਸੱਜਾ==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        ਵਾਪਸ p;
      }
      ਜੇ( p_ਨਤੀਜਾ==0 ) p_ਨਤੀਜਾ = p;
    }
    p = where_ਜਾਂਚ_ਅਗਲਾ(&ਜਾਂਚ);
  }
  ਵਾਪਸ p_ਨਤੀਜਾ;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_ਸੂਚਕਾਂਕ_ਖਾਨਾ(
  Parse *pParse,                  /* Parse context */
  ਵਿਅੰਜਕ_List *pList,                /* Expression list to search */
  ਪੂਰਨ i_ਅਧਾਰ,                      /* Cursor for table associated with pIdx */
  ਸੂਚਕਾਂਕ *pIdx,                    /* Index to match column of */
  ਪੂਰਨ i_ਖਾਨਾ                        /* Column of index to match */
){
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zColl = pIdx->azColl[i_ਖਾਨਾ];

  ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pList->a[i].p_ਵਿਅੰਜਕ);
    ਜੇ( ALWAYS(p!=0)
     && (p->op==TK_ਕਾਲਮ || p->op==TK_AGG_ਕਾਲਮ)
     && p->i_ਕਾਲਮ==pIdx->ai_ਕਾਲਮ[i_ਖਾਨਾ]
     && p->i_ਸਾਰਣੀ==i_ਅਧਾਰ
    ){
      CollSeq *pColl = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pParse, pList->a[i].p_ਵਿਅੰਜਕ);
      ਜੇ( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        ਵਾਪਸ i;
      }
    }
  }

  ਵਾਪਸ -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
ਸਥਿਰ ਪੂਰਨ ਸੂਚਕਾਂਕ_ਕਾਲਮ_Not_Null(ਸੂਚਕਾਂਕ *pIdx, ਪੂਰਨ i_ਖਾਨਾ){
  ਪੂਰਨ j;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pIdx->n_ਕਾਲਮ );
  j = pIdx->ai_ਕਾਲਮ[i_ਖਾਨਾ];
  ਜੇ( j>=0 ){
    ਵਾਪਸ pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[j].notNull;
  }ਨਹੀਂਤਾਂ ਜੇ( j==(-1) ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( j==(-2) );
    ਵਾਪਸ 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
ਸਥਿਰ ਪੂਰਨ isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  ਵਿਅੰਜਕ_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  ਸਾਰਣੀ *pTab;
  ਸੂਚਕਾਂਕ *pIdx;
  ਪੂਰਨ i;                         
  ਪੂਰਨ i_ਅਧਾਰ;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  ਜੇ( pTabList->nSrc!=1 ) ਵਾਪਸ 0;
  i_ਅਧਾਰ = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  ਲਈ(i=0; i<pDistinct->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pDistinct->a[i].p_ਵਿਅੰਜਕ);
    ਜੇ( NEVER(p==0) ) ਜਾਰੀ;
    ਜੇ( p->op!=TK_ਕਾਲਮ && p->op!=TK_AGG_ਕਾਲਮ ) ਜਾਰੀ;
    ਜੇ( p->i_ਸਾਰਣੀ==i_ਅਧਾਰ && p->i_ਕਾਲਮ<0 ) ਵਾਪਸ 1;
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
  ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
    ਜੇ( !Is_Unique_ਸੂਚਕਾਂਕ(pIdx) ) ਜਾਰੀ;
    ਜੇ( pIdx->pPartIdxWhere ) ਜਾਰੀ;
    ਲਈ(i=0; i<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
      ਜੇ( 0==sqlite3_Where_ਲੱਭੋ_ਪਦ(pWC, i_ਅਧਾਰ, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        ਜੇ( ਲੱਭੋ_ਸੂਚਕਾਂਕ_ਖਾਨਾ(pParse, pDistinct, i_ਅਧਾਰ, pIdx, i)<0 ) ਰੋਕੋ;
        ਜੇ( ਸੂਚਕਾਂਕ_ਕਾਲਮ_Not_Null(pIdx, i)==0 ) ਰੋਕੋ;
      }
    }
    ਜੇ( i==pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      ਵਾਪਸ 1;
    }
  }

  ਵਾਪਸ 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
ਸਥਿਰ LogEst estLog(LogEst N){
  ਵਾਪਸ N<=10 ? 0 : sqlite3LogEst(N) - 33;
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
ਸਥਿਰ ਖਾਲੀ translate_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Copy(
  Parse *pParse,      /* Parsing context */
  ਪੂਰਨ i_ਸ਼ੁਰੂ,         /* Translate from this opcode to the end */
  ਪੂਰਨ iTabCur,        /* OP_Column/OP_Rowid references to this table */
  ਪੂਰਨ i_ਰਜਿਸਟਰ,      /* The first column is in this register */
  ਪੂਰਨ iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, i_ਸ਼ੁਰੂ);
  ਪੂਰਨ i_ਸਿਰਾ = sqlite3VdbeCurrentAddr(v);
  ਜੇ( pParse->db->mallocFailed ) ਵਾਪਸ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    ਲਿਖੋ("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, i_ਸ਼ੁਰੂ, i_ਸਿਰਾ);
  }
#ਜੇ_ਅੰਤ
  ਲਈ(; i_ਸ਼ੁਰੂ<i_ਸਿਰਾ; i_ਸ਼ੁਰੂ++, pOp++){
    ਜੇ( pOp->p1!=iTabCur ) ਜਾਰੀ;
    ਜੇ( pOp->opcode==OP_ਕਾਲਮ ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      ਜੇ( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        ਲਿਖੋ("TRANSLATE OP_Column to OP_Copy at %d\n", i_ਸ਼ੁਰੂ);
      }
#ਜੇ_ਅੰਤ
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + i_ਰਜਿਸਟਰ;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }ਨਹੀਂਤਾਂ ਜੇ( pOp->opcode==OP_Rowid ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      ਜੇ( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        ਲਿਖੋ("TRANSLATE OP_Rowid to OP_Sequence at %d\n", i_ਸ਼ੁਰੂ);
      }
#ਜੇ_ਅੰਤ
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
      ਜੇ( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#ਜੇ_ਅੰਤ
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#ਜੇ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
ਸਥਿਰ ਖਾਲੀ where_Trace_ਸੂਚਕਾਂਕ_Info_Inputs(
  sqlite3_ਸੂਚਕਾਂਕ_info *p,   /* The IndexInfo object */
  ਸਾਰਣੀ *pTab              /* The TABLE that is the virtual table */
){
  ਪੂਰਨ i;
  ਜੇ( (sqlite3WhereTrace & 0x10)==0 ) ਵਾਪਸ;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  ਲਈ(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].i_ਕਾਲਮ,
       p->aConstraint[i].i_ਪਦ_ਵਿੱਥ,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  ਲਈ(i=0; i<p->n_ਕ੍ਰਮ_By; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->a_ਕ੍ਰਮ_By[i].i_ਕਾਲਮ,
       p->a_ਕ੍ਰਮ_By[i].desc);
  }
}
ਸਥਿਰ ਖਾਲੀ where_Trace_ਸੂਚਕਾਂਕ_Info_Outputs(
  sqlite3_ਸੂਚਕਾਂਕ_info *p,   /* The IndexInfo object */
  ਸਾਰਣੀ *pTab              /* The TABLE that is the virtual table */
){
  ਪੂਰਨ i;
  ਜੇ( (sqlite3WhereTrace & 0x10)==0 ) ਵਾਪਸ;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  ਲਈ(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_ਸੂਚਕਾਂਕ,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_ਅੰਕ);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->ਕ੍ਰਮ_By_Consumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimated_ਲਾਗਤ);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimated_ਪੰਕਤੀਆਂ);
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ where_Trace_ਸੂਚਕਾਂਕ_Info_Inputs(A,B)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ where_Trace_ਸੂਚਕਾਂਕ_Info_Outputs(A,B)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ constraintCompatibleWithOuterJoin(
  ਅਚਲ Where_ਪਦ *p_ਪਦ,       /* WHERE clause term to check */
  ਅਚਲ Src_ਵਸਤੂ *pSrc           /* Table we are trying to access */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( (pSrc->fg.jointype&(JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))==JT_ਖੱਬਾ );
  testcase( (pSrc->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))==JT_LTORJ );
  testcase( ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON) )
  testcase( ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_InnerON) );
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON|EP_InnerON)
   || p_ਪਦ->p_ਵਿਅੰਜਕ->w.iJoin != pSrc->iCursor
  ){
    ਵਾਪਸ 0;
  }
  ਜੇ( (pSrc->fg.jointype & (JT_ਖੱਬਾ|JT_ਸੱਜਾ))!=0
   && NEVER(ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_InnerON))
  ){
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOMATIC_ਸੂਚਕਾਂਕ
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਕਾਲਮ_Is_Good_ਸੂਚਕਾਂਕ_Candidate(
  ਅਚਲ ਸਾਰਣੀ *pTab,
  ਪੂਰਨ i_ਖਾਨਾ
){
  ਅਚਲ ਸੂਚਕਾਂਕ *pIdx;
  ਲਈ(pIdx = pTab->p_ਸੂਚਕਾਂਕ; pIdx!=0; pIdx=pIdx->p_ਅਗਲਾ){
    ਪੂਰਨ j;
    ਲਈ(j=0; j<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; j++){
       ਜੇ( pIdx->ai_ਕਾਲਮ[j]==i_ਖਾਨਾ ){
         ਜੇ( j==0 ) ਵਾਪਸ 0;
         ਜੇ( pIdx->hasStat1 && pIdx->ai_ਪੰਕਤੀ_Log_Est[j+1]>20 ) ਵਾਪਸ 0;
         ਰੋਕੋ;
       }
    }
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_AUTOMATIC_INDEX */



#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOMATIC_ਸੂਚਕਾਂਕ
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
ਸਥਿਰ ਪੂਰਨ ਪਦ_Can_Drive_ਸੂਚਕਾਂਕ(
  ਅਚਲ Where_ਪਦ *p_ਪਦ,        /* WHERE clause term to check */
  ਅਚਲ Src_ਵਸਤੂ *pSrc,           /* Table we are trying to access */
  ਅਚਲ Bitmask notReady         /* Tables in outer loops of the join */
){
  ਅੱਖਰ aff;
  ਪੂਰਨ ਖੱਬਾ_ਖਾਨਾ;
  
  ਜੇ( p_ਪਦ->ਖੱਬਾ_Cursor!=pSrc->iCursor ) ਵਾਪਸ 0;
  ਜੇ( (p_ਪਦ->eOperator & (WO_EQ|WO_IS))==0 ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pSrc->fg.jointype & JT_ਸੱਜਾ)==0 );
  ਜੇ( (pSrc->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))!=0
   && !constraintCompatibleWithOuterJoin(p_ਪਦ,pSrc)
  ){
    ਵਾਪਸ 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  ਜੇ( (p_ਪਦ->prereq_ਸੱਜਾ & notReady)!=0 ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
  ਖੱਬਾ_ਖਾਨਾ = p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ;
  ਜੇ( ਖੱਬਾ_ਖਾਨਾ<0 ) ਵਾਪਸ 0;
  aff = pSrc->pSTab->a_ਖਾਨਾ[ਖੱਬਾ_ਖਾਨਾ].affinity;
  ਜੇ( !sqlite3_ਸੂਚਕਾਂਕ_Affinity_Ok(p_ਪਦ->p_ਵਿਅੰਜਕ, aff) ) ਵਾਪਸ 0;
  testcase( p_ਪਦ->p_ਵਿਅੰਜਕ->op==TK_IS );
  ਵਾਪਸ ਕਾਲਮ_Is_Good_ਸੂਚਕਾਂਕ_Candidate(pSrc->pSTab, ਖੱਬਾ_ਖਾਨਾ);
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOMATIC_ਸੂਚਕਾਂਕ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
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
ਸਥਿਰ ਖਾਲੀ explain_Automatic_ਸੂਚਕਾਂਕ(
  Parse *pParse,
  ਸੂਚਕਾਂਕ *pIdx,                    /* Automatic index to explain */
  ਪੂਰਨ bPartial,                   /* True if pIdx is a partial index */
  ਪੂਰਨ *pAddrExplain               /* OUT: Address of OP_Explain */
){
  ਜੇ( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    ਸਾਰਣੀ *pTab = pIdx->p_ਸਾਰਣੀ;
    ਅਚਲ ਅੱਖਰ *zSep = "";
    ਅੱਖਰ *z_ਪਾਠ = 0;
    ਪੂਰਨ ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->n_ਕਾਲਮ>1 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->ai_ਕਾਲਮ[pIdx->n_ਕਾਲਮ-1]==XN_ROWID || !HasRowid(pTab) );
    ਲਈ(ii=0; ii<(pIdx->n_ਕਾਲਮ-1); ii++){
      ਅਚਲ ਅੱਖਰ *zName = 0;
      ਪੂਰਨ i_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[ii];

      zName = pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    z_ਪਾਠ = sqlite3_str_ਅੰਤ(pStr);
    ਜੇ( z_ਪਾਠ==0 ){
      sqlite3OomFault(pParse->db);
    }ਨਹੀਂਤਾਂ{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", z_ਪਾਠ, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(z_ਪਾਠ);
    }
  }
}
#ਨਹੀਂਤਾਂ
# define explain_Automatic_ਸੂਚਕਾਂਕ(a,b,c,d)
#ਜੇ_ਅੰਤ

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ construct_Automatic_ਸੂਚਕਾਂਕ(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  ਅਚਲ Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  ਪੂਰਨ n_ਕੁੰਜੀ_ਖਾਨਾ;                /* Number of columns in the constructed index */
  Where_ਪਦ *p_ਪਦ;           /* A single term of the WHERE clause */
  Where_ਪਦ *pWCEnd;          /* End of pWC->a[] */
  ਸੂਚਕਾਂਕ *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  ਪੂਰਨ addrInit;               /* Address of the initialization bypass jump */
  ਸਾਰਣੀ *p_ਸਾਰਣੀ;              /* The table being indexed */
  ਪੂਰਨ addr_ਸਿਖਰ;                /* Top of the index fill loop */
  ਪੂਰਨ reg_ਰਿਕਾਰਡ;              /* Register holding an index record */
  ਪੂਰਨ n;                      /* Column counter */
  ਪੂਰਨ i;                      /* Loop counter */
  ਪੂਰਨ mx_ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਾਨਾ;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  ਅੱਖਰ *z_Not_ਵਰਤਿਆ;             /* Extra space on the end of pIdx */
  Bitmask idx_ਖਾਨੇ_ਗਿਣਤੀ;            /* Bitmap of columns used for indexing */
  Bitmask extra_ਖਾਨੇ_ਗਿਣਤੀ;          /* Bitmap of additional columns */
  u8 ਭੇਜਿਆ_Warning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  ਵਿਅੰਜਕ *pPartial = 0;         /* Partial Index Expression */
  ਪੂਰਨ iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  Src_ਵਸਤੂ *pSrc;              /* The FROM clause term to get the next index */
  ਪੂਰਨ addrCounter = 0;        /* Address where integer counter is initialized */
  ਪੂਰਨ reg_ਅਧਾਰ;                /* Array of registers where record is assembled */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  ਪੂਰਨ addrExp = 0;            /* Address of OP_Explain */
#ਜੇ_ਅੰਤ

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  addrInit = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_ਕੁੰਜੀ_ਖਾਨਾ = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  p_ਸਾਰਣੀ = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->n_ਪਦ];
  pLoop = pLevel->pWLoop;
  idx_ਖਾਨੇ_ਗਿਣਤੀ = 0;
  ਲਈ(p_ਪਦ=pWC->a; p_ਪਦ<pWCEnd; p_ਪਦ++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_VIRTUAL)==0
     && sqlite3_ਵਿਅੰਜਕ_Is_Single_ਸਾਰਣੀ_Constraint(p_ਵਿਅੰਜਕ, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_ਵਿਅੰਜਕ_And(pParse, pPartial,
                                sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, p_ਵਿਅੰਜਕ, 0));
    }
    ਜੇ( ਪਦ_Can_Drive_ਸੂਚਕਾਂਕ(p_ਪਦ, pSrc, notReady) ){
      ਪੂਰਨ i_ਖਾਨਾ;
      Bitmask c_ਨਕਾਬ;
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
      i_ਖਾਨਾ = p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ;
      c_ਨਕਾਬ = i_ਖਾਨਾ>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ਖਾਨਾ);
      testcase( i_ਖਾਨਾ==BMS );
      testcase( i_ਖਾਨਾ==BMS-1 );
      ਜੇ( !ਭੇਜਿਆ_Warning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", p_ਸਾਰਣੀ->zName,
            p_ਸਾਰਣੀ->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName);
        ਭੇਜਿਆ_Warning = 1;
      }
      ਜੇ( (idx_ਖਾਨੇ_ਗਿਣਤੀ & c_ਨਕਾਬ)==0 ){
        ਜੇ( whereLoopResize(pParse->db, pLoop, n_ਕੁੰਜੀ_ਖਾਨਾ+1) ){
          ਜਾਓ ਸਿਰਾ_auto_ਸੂਚਕਾਂਕ_create;
        }
        pLoop->aLTerm[n_ਕੁੰਜੀ_ਖਾਨਾ++] = p_ਪਦ;
        idx_ਖਾਨੇ_ਗਿਣਤੀ |= c_ਨਕਾਬ;
      }
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ_ਖਾਨਾ>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_ਕੁੰਜੀ_ਖਾਨਾ;
  pLoop->wsFlags = WHERE_ਕਾਲਮ_EQ | WHERE_IDX_ONLY | WHERE_ਸੂਚਕਾਂਕਿਤ
                     | WHERE_AUTO_ਸੂਚਕਾਂਕ;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  ਜੇ( IsView(p_ਸਾਰਣੀ) ){
    extra_ਖਾਨੇ_ਗਿਣਤੀ = ALLBITS & ~idx_ਖਾਨੇ_ਗਿਣਤੀ;
  }ਨਹੀਂਤਾਂ{
    extra_ਖਾਨੇ_ਗਿਣਤੀ = pSrc->ਖਾਨਾ_ਵਰਤਿਆ & (~idx_ਖਾਨੇ_ਗਿਣਤੀ | MASKBIT(BMS-1));
  }
  ਜੇ( !HasRowid(p_ਸਾਰਣੀ) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    ਲਈ(i=0; i<p_ਸਾਰਣੀ->n_ਖਾਨਾ; i++){
      ਜੇ( (p_ਸਾਰਣੀ->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_PRIMKEY)==0 ) ਜਾਰੀ;
      ਜੇ( i>=BMS-1 ){
        extra_ਖਾਨੇ_ਗਿਣਤੀ |= MASKBIT(BMS-1);
        ਰੋਕੋ;
      }
      ਜੇ( idx_ਖਾਨੇ_ਗਿਣਤੀ & MASKBIT(i) ) ਜਾਰੀ;
      extra_ਖਾਨੇ_ਗਿਣਤੀ |= MASKBIT(i);
    }
  }
  mx_ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਾਨਾ = MIN(BMS-1,p_ਸਾਰਣੀ->n_ਖਾਨਾ);
  testcase( p_ਸਾਰਣੀ->n_ਖਾਨਾ==BMS-1 );
  testcase( p_ਸਾਰਣੀ->n_ਖਾਨਾ==BMS-2 );
  ਲਈ(i=0; i<mx_ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਾਨਾ; i++){
    ਜੇ( extra_ਖਾਨੇ_ਗਿਣਤੀ & MASKBIT(i) ) n_ਕੁੰਜੀ_ਖਾਨਾ++;
  }
  ਜੇ( pSrc->ਖਾਨਾ_ਵਰਤਿਆ & MASKBIT(BMS-1) ){
    n_ਕੁੰਜੀ_ਖਾਨਾ += p_ਸਾਰਣੀ->n_ਖਾਨਾ - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ_ਖਾਨਾ <= p_ਸਾਰਣੀ->n_ਖਾਨਾ + MAX(0, p_ਸਾਰਣੀ->n_ਖਾਨਾ - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_ਸੂਚਕਾਂਕ_Object(pParse->db, n_ਕੁੰਜੀ_ਖਾਨਾ+HasRowid(p_ਸਾਰਣੀ),
                                    0, &z_Not_ਵਰਤਿਆ);
  ਜੇ( pIdx==0 ) ਜਾਓ ਸਿਰਾ_auto_ਸੂਚਕਾਂਕ_create;
  pLoop->u.btree.p_ਸੂਚਕਾਂਕ = pIdx;
  pIdx->zName = "auto-index";
  pIdx->p_ਸਾਰਣੀ = p_ਸਾਰਣੀ;
  n = 0;
  idx_ਖਾਨੇ_ਗਿਣਤੀ = 0;
  ਲਈ(p_ਪਦ=pWC->a; p_ਪਦ<pWCEnd; p_ਪਦ++){
    ਜੇ( ਪਦ_Can_Drive_ਸੂਚਕਾਂਕ(p_ਪਦ, pSrc, notReady) ){
      ਪੂਰਨ i_ਖਾਨਾ;
      Bitmask c_ਨਕਾਬ;
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
      i_ਖਾਨਾ = p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ;
      c_ਨਕਾਬ = i_ਖਾਨਾ>=BMS ? MASKBIT(BMS-1) : MASKBIT(i_ਖਾਨਾ);
      testcase( i_ਖਾਨਾ==BMS-1 );
      testcase( i_ਖਾਨਾ==BMS );
      ਜੇ( (idx_ਖਾਨੇ_ਗਿਣਤੀ & c_ਨਕਾਬ)==0 ){
        ਵਿਅੰਜਕ *pX = p_ਪਦ->p_ਵਿਅੰਜਕ;
        idx_ਖਾਨੇ_ਗਿਣਤੀ |= c_ਨਕਾਬ;
        pIdx->ai_ਕਾਲਮ[n] = p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ;
        pColl = sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pX);
        ਪ੍ਰਤਿਜ੍ਞਾ( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        ਜੇ( !sqlite3_Is_ਦੁਆਧਾਰੀ(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_ਦੁਆਧਾਰੀ;
        n++;
      }
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  ਲਈ(i=0; i<mx_ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਾਨਾ; i++){
    ਜੇ( extra_ਖਾਨੇ_ਗਿਣਤੀ & MASKBIT(i) ){
      pIdx->ai_ਕਾਲਮ[n] = i;
      pIdx->azColl[n] = sqlite3_Str_ਦੁਆਧਾਰੀ;
      n++;
    }
  }
  ਜੇ( pSrc->ਖਾਨਾ_ਵਰਤਿਆ & MASKBIT(BMS-1) ){
    ਲਈ(i=BMS-1; i<p_ਸਾਰਣੀ->n_ਖਾਨਾ; i++){
      pIdx->ai_ਕਾਲਮ[n] = i;
      pIdx->azColl[n] = sqlite3_Str_ਦੁਆਧਾਰੀ;
      n++;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n==n_ਕੁੰਜੀ_ਖਾਨਾ );
  ਜੇ( HasRowid(p_ਸਾਰਣੀ) ){
    pIdx->ai_ਕਾਲਮ[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_ਦੁਆਧਾਰੀ;
  }

  /* Create the automatic index */
  explain_Automatic_ਸੂਚਕਾਂਕ(pParse, pIdx, pPartial!=0, &addrExp);
  ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_ਕੁੰਜੀ_ਖਾਨਾ+1);
  sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIdx);
  VdbeComment((v, "for %s", p_ਸਾਰਣੀ->zName));
  ਜੇ( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  ਜੇ( pSrc->fg.ਰਾਹੀਂ_Coroutine ){
    ਪੂਰਨ regYield;
    Subquery *pSubq;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSubq!=0 );
    regYield = pSubq->regReturn;
    addrCounter = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, 0);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_ਸਿਖਰ =  sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->addrHalt );
    addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  ਜੇ( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  reg_ਰਿਕਾਰਡ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
  reg_ਅਧਾਰ = sqlite3_Generate_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(
      pParse, pIdx, pLevel->iTabCur, reg_ਰਿਕਾਰਡ, 0, 0, 0, 0
  );
  ਜੇ( pLevel->regFilter ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pLevel->regFilter, 0,
                         reg_ਅਧਾਰ, pLoop->u.btree.nEq);
  }
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Counters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Idx_ਪਾਓ, pLevel->iIdxCur, reg_ਰਿਕਾਰਡ);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  ਜੇ( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  ਜੇ( pSrc->fg.ਰਾਹੀਂ_Coroutine ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addrCounter, reg_ਅਧਾਰ+n);
    testcase( pParse->db->mallocFailed );
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->iIdxCur>0 );
    translate_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Copy(pParse, addr_ਸਿਖਰ, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_ਨਤੀਜਾ, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_ਸਿਖਰ);
    pSrc->fg.ਰਾਹੀਂ_Coroutine = 0;
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਸਿਖਰ);
  }ਨਹੀਂਤਾਂ{
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, pLevel->iTabCur, addr_ਸਿਖਰ+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    ਜੇ( (pSrc->fg.jointype & JT_ਖੱਬਾ)!=0 ){
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਸਿਖਰ);
    }
  }
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਰਿਕਾਰਡ);
 
  /* Jump here when skipping the initialization */
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrInit);
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExp, addrExp, -1);

ਸਿਰਾ_auto_ਸੂਚਕਾਂਕ_create:
  sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, pPartial);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_AUTOMATIC_INDEX */

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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  ਪੂਰਨ iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  ਪੂਰਨ addrOnce;                        /* Address of opening OP_Once */
  ਪੂਰਨ addr_ਸਿਖਰ;                         /* Address of OP_Rewind */
  ਪੂਰਨ addrCont;                        /* Jump here to skip a row */
  ਅਚਲ Where_ਪਦ *p_ਪਦ;              /* For looping over WHERE clause terms */
  ਅਚਲ Where_ਪਦ *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  ਪੂਰਨ iCur;                            /* Cursor for table getting the filter */
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *saved_p_Idx_Part_ਵਿਅੰਜਕ;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_ਵਿਅੰਜਕ = pParse->p_Idx_Part_ਵਿਅੰਜਕ;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_ਵਿਅੰਜਕ = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( pLoop!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->wsFlags & WHERE_BLOOMFILTER );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
  ਕਰੋ{
    ਅਚਲ SrcList *pTabList;
    ਅਚਲ Src_ਵਸਤੂ *p_ਵਸਤੂ;
    ਅਚਲ ਸਾਰਣੀ *pTab;
    u64 sz;
    ਪੂਰਨ iSrc;
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
    p_ਵਸਤੂ = &pTabList->a[iSrc];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ!=0 );
    pTab = p_ਵਸਤੂ->pSTab;
    ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
    sz = sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(pTab->n_ਪੰਕਤੀ_Log_Est);
    ਜੇ( sz<10000 ){
      sz = 10000;
    }ਨਹੀਂਤਾਂ ਜੇ( sz>10000000 ){
      sz = 10000000;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Blob, (ਪੂਰਨ)sz, pLevel->regFilter);

    addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.n_ਪਦ];
    ਲਈ(p_ਪਦ=pWInfo->sWC.a; p_ਪਦ<pWCEnd; p_ਪਦ++){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
      ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_VIRTUAL)==0
       && sqlite3_ਵਿਅੰਜਕ_Is_Single_ਸਾਰਣੀ_Constraint(p_ਵਿਅੰਜਕ, pTabList, iSrc, 0)
      ){
        sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਪਦ->p_ਵਿਅੰਜਕ, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    ਜੇ( pLoop->wsFlags & WHERE_IPK ){
      ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iCur, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
    }ਨਹੀਂਤਾਂ{
      ਸੂਚਕਾਂਕ *pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
      ਪੂਰਨ n = pLoop->u.btree.nEq;
      ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n);
      ਪੂਰਨ jj;
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 );
      ਲਈ(jj=0; jj<n; jj++){
        ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->p_ਸਾਰਣੀ==p_ਵਸਤੂ->pSTab );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੱਦੋ_ਸੂਚਕਾਂਕ_ਕਾਲਮ(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, pLevel->iTabCur, addr_ਸਿਖਰ+1);
    VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਸਿਖਰ);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    ਜਦੋਂ( ++iLevel < pWInfo->nLevel ){
      ਅਚਲ Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ;
      pLevel = &pWInfo->a[iLevel];
      p_Tab_ਵਸਤੂ = &pWInfo->pTabList->a[pLevel->iFrom];
      ਜੇ( p_Tab_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ) ) ਜਾਰੀ;
      pLoop = pLevel->pWLoop;
      ਜੇ( NEVER(pLoop==0) ) ਜਾਰੀ;
      ਜੇ( pLoop->prereq & notReady ) ਜਾਰੀ;
      ਜੇ( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_ਕਾਲਮ_ਵਿੱਚ))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        ਰੋਕੋ;
      }
    }
  }ਜਦੋਂ( iLevel < pWInfo->nLevel );
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_ਵਿਅੰਜਕ = saved_p_Idx_Part_ਵਿਅੰਜਕ;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
ਸਥਿਰ Where_ਪਦ *ਪਦ_From_Where_Clause(WhereClause *pWC, ਪੂਰਨ i_ਪਦ){
  WhereClause *p;
  ਲਈ(p=pWC; p; p=p->pOuter){
    ਜੇ( i_ਪਦ<p->n_ਪਦ ) ਵਾਪਸ &p->a[i_ਪਦ];
    i_ਪਦ -= p->n_ਪਦ;
  }
  ਵਾਪਸ 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
ਸਥਿਰ sqlite3_ਸੂਚਕਾਂਕ_info *allocate_ਸੂਚਕਾਂਕ_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  Src_ਵਸਤੂ *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  ਪੂਰਨ i, j;
  ਪੂਰਨ n_ਪਦ;
  Parse *pParse = pWInfo->pParse;
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *pIdxCons;
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_orderby *p_Idx_ਕ੍ਰਮ_By;
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage;
  ਬਣਤਰ Hidden_ਸੂਚਕਾਂਕ_Info *pHidden;
  Where_ਪਦ *p_ਪਦ;
  ਪੂਰਨ n_ਕ੍ਰਮ_By;
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo;
  u16 mNoOmit = 0;
  ਅਚਲ ਸਾਰਣੀ *pTab;
  ਪੂਰਨ eDistinct = 0;
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By = pWInfo->p_ਕ੍ਰਮ_By;
  WhereClause *p;

  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc!=0 );
  pTab = pSrc->pSTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  ਲਈ(p=pWC, n_ਪਦ=0; p; p=p->pOuter){
    ਲਈ(i=0, p_ਪਦ=p->a; i<p->n_ਪਦ; i++, p_ਪਦ++){
      p_ਪਦ->wtFlags &= ~ਪਦ_OK;
      ਜੇ( p_ਪਦ->ਖੱਬਾ_Cursor != pSrc->iCursor ) ਜਾਰੀ;
      ਜੇ( p_ਪਦ->prereq_ਸੱਜਾ & mUnusable ) ਜਾਰੀ;
      ਪ੍ਰਤਿਜ੍ਞਾ( Is_Power_ਦਾ_Two(p_ਪਦ->eOperator & ~WO_EQUIV) );
      testcase( p_ਪਦ->eOperator & WO_ਵਿੱਚ );
      testcase( p_ਪਦ->eOperator & WO_ISNULL );
      testcase( p_ਪਦ->eOperator & WO_IS );
      testcase( p_ਪਦ->eOperator & WO_ALL );
      ਜੇ( (p_ਪਦ->eOperator & ~(WO_EQUIV))==0 ) ਜਾਰੀ;
      ਜੇ( p_ਪਦ->wtFlags & ਪਦ_VNULL ) ਜਾਰੀ;

      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ>=XN_ROWID );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ<pTab->n_ਖਾਨਾ );
      ਜੇ( (pSrc->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))!=0
          && !constraintCompatibleWithOuterJoin(p_ਪਦ,pSrc)
        ){
        ਜਾਰੀ;
      }
      n_ਪਦ++;
      p_ਪਦ->wtFlags |= ਪਦ_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  n_ਕ੍ਰਮ_By = 0;
  ਜੇ( p_ਕ੍ਰਮ_By ){
    ਪੂਰਨ n = p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;
    ਲਈ(i=0; i<n; i++){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ;
      ਵਿਅੰਜਕ *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Constant(0, p_ਵਿਅੰਜਕ) ){
        ਜਾਰੀ;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      ਜੇ( p_ਕ੍ਰਮ_By->a[i].fg.ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_BIGNULL ) ਰੋਕੋ;

      /* First case - a direct column references without a COLLATE operator */
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pSrc->iCursor ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=XN_ROWID && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<pTab->n_ਖਾਨਾ );
        ਜਾਰੀ;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_COLLATE
       && (pE2 = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ)->op==TK_ਕਾਲਮ
       && pE2->i_ਸਾਰਣੀ==pSrc->iCursor
      ){
        ਅਚਲ ਅੱਖਰ *zColl;  /* The collating sequence name */
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pE2->i_ਕਾਲਮ>=XN_ROWID && pE2->i_ਕਾਲਮ<pTab->n_ਖਾਨਾ );
        p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = pE2->i_ਕਾਲਮ;
        ਜੇ( pE2->i_ਕਾਲਮ<0 ) ਜਾਰੀ;  /* Collseq does not matter for rowid */
        zColl = sqlite3_ਕਾਲਮ_Coll(&pTab->a_ਖਾਨਾ[pE2->i_ਕਾਲਮ]);
        ਜੇ( zColl==0 ) zColl = sqlite3_Str_ਦੁਆਧਾਰੀ;
        ਜੇ( sqlite3_stricmp(p_ਵਿਅੰਜਕ->u.zToken, zColl)==0 ) ਜਾਰੀ;
      }

      /* No matches cause a break out of the loop */
      ਰੋਕੋ;
    }
    ਜੇ( i==n ){
      ਪੂਰਨ b_ਛਾਂਟੋ_By_ਸਮੂਹ = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      n_ਕ੍ਰਮ_By = n;
      ਜੇ( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowid_ਵਰਤਿਆ ){
        eDistinct = 2 + b_ਛਾਂਟੋ_By_ਸਮੂਹ;
      }ਨਹੀਂਤਾਂ ਜੇ( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - b_ਛਾਂਟੋ_By_ਸਮੂਹ;
      }ਨਹੀਂਤਾਂ ਜੇ( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, ਆਕਾਰ(*pIdxInfo)
                           + (ਆਕਾਰ(*pIdxCons) + ਆਕਾਰ(*pUsage))*n_ਪਦ
                           + ਆਕਾਰ(*p_Idx_ਕ੍ਰਮ_By)*n_ਕ੍ਰਮ_By
                           + SZ_HIDDENINDEXINFO(n_ਪਦ) );
  ਜੇ( pIdxInfo==0 ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "out of memory");
    ਵਾਪਸ 0;
  }
  pHidden = (ਬਣਤਰ Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  pIdxCons = (ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint*)&pHidden->aRhs[n_ਪਦ];
  p_Idx_ਕ੍ਰਮ_By = (ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_orderby*)&pIdxCons[n_ਪਦ];
  pUsage = (ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage*)&p_Idx_ਕ੍ਰਮ_By[n_ਕ੍ਰਮ_By];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->a_ਕ੍ਰਮ_By = p_Idx_ਕ੍ਰਮ_By;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->ਖਾਨਾ_ਵਰਤਿਆ = (sqlite3_int64)pSrc->ਖਾਨਾ_ਵਰਤਿਆ;
  ਜੇ( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ((ਸਾਰਣੀ*)pTab);
    ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 );
    ਲਈ(i=0; i<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
      ਪੂਰਨ i_ਖਾਨਾ = pPk->ai_ਕਾਲਮ[i];
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 );
      ਜੇ( i_ਖਾਨਾ>=BMS-1 ) i_ਖਾਨਾ = BMS-1;
      pIdxInfo->ਖਾਨਾ_ਵਰਤਿਆ |= MASKBIT(i_ਖਾਨਾ);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->m_ਵਿੱਚ = 0;
  ਲਈ(p=pWC, i=j=0; p; p=p->pOuter){
    ਪੂਰਨ n_ਆਖਰੀ = i+p->n_ਪਦ;;
    ਲਈ(p_ਪਦ=p->a; i<n_ਆਖਰੀ; i++, p_ਪਦ++){
      u16 op;
      ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_OK)==0 ) ਜਾਰੀ;
      pIdxCons[j].i_ਕਾਲਮ = p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ;
      pIdxCons[j].i_ਪਦ_ਵਿੱਥ = i;
      op = p_ਪਦ->eOperator & WO_ALL;
      ਜੇ( op==WO_ਵਿੱਚ ){
        ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_ਟੁਕੜਾ)==0 ){
          pHidden->m_ਵਿੱਚ |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      ਜੇ( op==WO_AUX ){
        pIdxCons[j].op = p_ਪਦ->e_ਮੇਲ_Op;
      }ਨਹੀਂਤਾਂ ਜੇ( op & (WO_ISNULL|WO_IS) ){
        ਜੇ( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNULL;
        }ਨਹੀਂਤਾਂ{
          pIdxCons[j].op = SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_IS;
        }
      }ਨਹੀਂਤਾਂ{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        ਪ੍ਰਤਿਜ੍ਞਾ( WO_EQ==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ );
        ਪ੍ਰਤਿਜ੍ਞਾ( WO_LT==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LT );
        ਪ੍ਰਤਿਜ੍ਞਾ( WO_LE==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LE );
        ਪ੍ਰਤਿਜ੍ਞਾ( WO_GT==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GT );
        ਪ੍ਰਤਿਜ੍ਞਾ( WO_GE==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GE );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->eOperator&(WO_ਵਿੱਚ|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        ਜੇ( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ)
          ){
          testcase( j!=i );
          ਜੇ( j<16 ) mNoOmit |= (1 << j);
          ਜੇ( op==WO_LT ) pIdxCons[j].op = WO_LE;
          ਜੇ( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( j==n_ਪਦ );
  pIdxInfo->nConstraint = j;
  ਲਈ(i=j=0; i<n_ਕ੍ਰਮ_By; i++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Constant(0, p_ਵਿਅੰਜਕ) ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
         || (p_ਵਿਅੰਜਕ->op==TK_COLLATE && p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕਾਲਮ
              && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ==p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->i_ਕਾਲਮ) );
    p_Idx_ਕ੍ਰਮ_By[j].i_ਕਾਲਮ = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
    p_Idx_ਕ੍ਰਮ_By[j].desc = p_ਕ੍ਰਮ_By->a[i].fg.ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_DESC;
    j++;
  }
  pIdxInfo->n_ਕ੍ਰਮ_By = j;

  *pmNoOmit = mNoOmit;
  ਵਾਪਸ pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
ਸਥਿਰ ਖਾਲੀ freeIdxStr(sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਜੇ( pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
ਸਥਿਰ ਖਾਲੀ free_ਸੂਚਕਾਂਕ_Info(sqlite3 *db, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  Hidden_ਸੂਚਕਾਂਕ_Info *pHidden;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdxInfo!=0 );
  pHidden = (Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  ਪ੍ਰਤਿਜ੍ਞਾ( pHidden->pParse!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pHidden->pParse->db==db );
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_ਮੁੱਲ_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
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
ਸਥਿਰ ਪੂਰਨ vtab_ਉੱਤਮ_ਸੂਚਕਾਂਕ(Parse *pParse, ਸਾਰਣੀ *pTab, sqlite3_ਸੂਚਕਾਂਕ_info *p){
  ਪੂਰਨ rc;
  sqlite3_vtab *pVtab;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_ਸੂਚਕਾਂਕ_Info_Inputs(p, pTab);
  pParse->db->n_Schema_ਤਾਲਾ++;
  rc = pVtab->pModule->x_ਉੱਤਮ_ਸੂਚਕਾਂਕ(pVtab, p);
  pParse->db->n_Schema_ਤਾਲਾ--;
  where_Trace_ਸੂਚਕਾਂਕ_Info_Outputs(p, pTab);

  ਜੇ( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    ਜੇ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }ਨਹੀਂਤਾਂ ਜੇ( !pVtab->z_Err_ਸੁਨੇਹਾ ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s", sqlite3ErrStr(rc));
    }ਨਹੀਂਤਾਂ{
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s", pVtab->z_Err_ਸੁਨੇਹਾ);
    }
  }
  ਜੇ( pTab->u.vtab.p->bAllSchemas ){
    sqlite3_Vtab_ਵਰਤਦਾ_All_Schemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_ਸੁਨੇਹਾ);
  pVtab->z_Err_ਸੁਨੇਹਾ = 0;
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
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
ਸਥਿਰ ਪੂਰਨ where_ਕੁੰਜੀ_Stats(
  Parse *pParse,              /* Database connection */
  ਸੂਚਕਾਂਕ *pIdx,                /* Index to consider domain of */
  Unpacked_ਰਿਕਾਰਡ *pRec,       /* Vector of values to consider */
  ਪੂਰਨ ਗੇੜਾ_ਉੱਪਰ,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  ਸੂਚਕਾਂਕ_Sample *aSample = pIdx->aSample;
  ਪੂਰਨ i_ਖਾਨਾ;                   /* Index of required stats in anEq[] etc. */
  ਪੂਰਨ i;                      /* Index of first sample >= pRec */
  ਪੂਰਨ iSample;                /* Smallest sample larger than or equal to pRec */
  ਪੂਰਨ iMin = 0;               /* Smallest sample not yet tested */
  ਪੂਰਨ iTest;                  /* Next sample to test */
  ਪੂਰਨ res;                    /* Result of comparison operation */
  ਪੂਰਨ n_ਖੇਤਰ;                 /* Number of fields in pRec */
  tRowcnt i_ਹੇਠਲਾ = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਅਣਵਰਤਿਆ_PARAMETER( pParse );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( pRec!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->nSample>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pRec->n_ਖੇਤਰ>0 );


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
  ਜੇ( !HasRowid(pIdx->p_ਸਾਰਣੀ) && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ){
    n_ਖੇਤਰ = pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ;
  }ਨਹੀਂਤਾਂ{
    n_ਖੇਤਰ = pIdx->n_ਕਾਲਮ;
  }
  n_ਖੇਤਰ = MIN(pRec->n_ਖੇਤਰ, n_ਖੇਤਰ);
  i_ਖਾਨਾ = 0;
  iSample = pIdx->nSample * n_ਖੇਤਰ;
  ਕਰੋ{
    ਪੂਰਨ iSamp;                    /* Index in aSample[] of test sample */
    ਪੂਰਨ n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / n_ਖੇਤਰ;
    ਜੇ( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      ਲਈ(n=(iTest % n_ਖੇਤਰ) + 1; n<n_ਖੇਤਰ; n++){
        ਜੇ( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ{
      n = iTest + 1;
    }

    pRec->n_ਖੇਤਰ = n;
    res = sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    ਜੇ( res<0 ){
      i_ਹੇਠਲਾ = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }ਨਹੀਂਤਾਂ ਜੇ( res==0 && n<n_ਖੇਤਰ ){
      i_ਹੇਠਲਾ = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }ਨਹੀਂਤਾਂ{
      iSample = iTest;
      i_ਖਾਨਾ = n-1;
    }
  }ਜਦੋਂ( res && iMin<iSample );
  i = iSample / n_ਖੇਤਰ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  ਜੇ( pParse->db->mallocFailed==0 ){
    ਜੇ( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      ਪ੍ਰਤਿਜ੍ਞਾ( i<pIdx->nSample );
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ==n_ਖੇਤਰ-1 );
      pRec->n_ਖੇਤਰ = n_ਖੇਤਰ;
      ਪ੍ਰਤਿਜ੍ਞਾ( 0==sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }ਨਹੀਂਤਾਂ{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      ਪ੍ਰਤਿਜ੍ਞਾ( i<=pIdx->nSample && i>=0 );
      pRec->n_ਖੇਤਰ = i_ਖਾਨਾ+1;
      ਪ੍ਰਤਿਜ੍ਞਾ( i==pIdx->nSample
           || sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      ਜੇ( i_ਖਾਨਾ>0 ){
        pRec->n_ਖੇਤਰ = i_ਖਾਨਾ;
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      ਜੇ( i>0 ){
        pRec->n_ਖੇਤਰ = n_ਖੇਤਰ;
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#ਜੇ_ਅੰਤ /* ifdef SQLITE_DEBUG */

  ਜੇ( res==0 ){
    /* Record pRec is equal to sample i */
    ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ==n_ਖੇਤਰ-1 );
    aStat[0] = aSample[i].anLt[i_ਖਾਨਾ];
    aStat[1] = aSample[i].anEq[i_ਖਾਨਾ];
  }ਨਹੀਂਤਾਂ{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt i_ਉੱਤਲਾ, iGap;
    ਜੇ( i>=pIdx->nSample ){
      i_ਉੱਤਲਾ = pIdx->n_ਪੰਕਤੀ_Est0;
    }ਨਹੀਂਤਾਂ{
      i_ਉੱਤਲਾ = aSample[i].anLt[i_ਖਾਨਾ];
    }

    ਜੇ( i_ਹੇਠਲਾ>=i_ਉੱਤਲਾ ){
      iGap = 0;
    }ਨਹੀਂਤਾਂ{
      iGap = i_ਉੱਤਲਾ - i_ਹੇਠਲਾ;
    }
    ਜੇ( ਗੇੜਾ_ਉੱਪਰ ){
      iGap = (iGap*2)/3;
    }ਨਹੀਂਤਾਂ{
      iGap = iGap/3;
    }
    aStat[0] = i_ਹੇਠਲਾ + iGap;
    aStat[1] = pIdx->aAvgEq[n_ਖੇਤਰ-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->n_ਖੇਤਰ = n_ਖੇਤਰ;
  ਵਾਪਸ i;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

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
ਸਥਿਰ LogEst whereRangeAdjust(Where_ਪਦ *p_ਪਦ, LogEst nNew){
  LogEst nRet = nNew;
  ਜੇ( p_ਪਦ ){
    ਜੇ( p_ਪਦ->truthProb<=0 ){
      nRet += p_ਪਦ->truthProb;
    }ਨਹੀਂਤਾਂ ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_VNULL)==0 ){
      nRet -= 20;        ਪ੍ਰਤਿਜ੍ਞਾ( 20==sqlite3LogEst(4) );
    }
  }
  ਵਾਪਸ nRet;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
ਅੱਖਰ sqlite3_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Affinity(sqlite3 *db, ਸੂਚਕਾਂਕ *pIdx, ਪੂਰਨ i_ਖਾਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pIdx->n_ਕਾਲਮ );
  ਜੇ( !pIdx->z_ਖਾਨਾ_Aff ){
    ਜੇ( sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(db, pIdx)==0 ) ਵਾਪਸ SQLITE_AFF_BLOB;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->z_ਖਾਨਾ_Aff[i_ਖਾਨਾ]!=0 );
  ਵਾਪਸ pIdx->z_ਖਾਨਾ_Aff[i_ਖਾਨਾ];
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
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
ਸਥਿਰ ਪੂਰਨ where_Range_Skip_ਜਾਂਚ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  Where_ਪਦ *p_ਹੇਠਲਾ,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_ਪਦ *p_ਉੱਤਲਾ,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  ਪੂਰਨ *pb_ਪੂਰਾ          /* Set to true if at least one expr. value extracted */
){
  ਸੂਚਕਾਂਕ *p = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
  ਪੂਰਨ nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ n_ਹੇਠਲਾ = -1;
  ਪੂਰਨ n_ਉੱਤਲਾ = p->nSample+1;
  ਪੂਰਨ rc = SQLITE_OK;
  u8 aff = sqlite3_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_ਮੁੱਲ *p1 = 0;          /* Value extracted from pLower */
  sqlite3_ਮੁੱਲ *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_ਮੁੱਲ *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  ਜੇ( p_ਹੇਠਲਾ ){
    rc = sqlite3_Stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(pParse, p_ਹੇਠਲਾ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, aff, &p1);
    n_ਹੇਠਲਾ = 0;
  }
  ਜੇ( p_ਉੱਤਲਾ && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(pParse, p_ਉੱਤਲਾ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, aff, &p2);
    n_ਉੱਤਲਾ = p2 ? 0 : p->nSample;
  }

  ਜੇ( p1 || p2 ){
    ਪੂਰਨ i;
    ਪੂਰਨ nDiff;
    ਲਈ(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3_Stat4_ਕਾਲਮ(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      ਜੇ( rc==SQLITE_OK && p1 ){
        ਪੂਰਨ res = sqlite3MemCompare(p1, pVal, pColl);
        ਜੇ( res>=0 ) n_ਹੇਠਲਾ++;
      }
      ਜੇ( rc==SQLITE_OK && p2 ){
        ਪੂਰਨ res = sqlite3MemCompare(p2, pVal, pColl);
        ਜੇ( res>=0 ) n_ਉੱਤਲਾ++;
      }
    }
    nDiff = (n_ਉੱਤਲਾ - n_ਹੇਠਲਾ);
    ਜੇ( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    ਜੇ( nDiff!=1 || p_ਉੱਤਲਾ==0 || p_ਹੇਠਲਾ==0 ){
      ਪੂਰਨ nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->n_ਬਾਹਰਲਾ -= nAdjust;
      *pb_ਪੂਰਾ = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           n_ਹੇਠਲਾ, n_ਉੱਤਲਾ, nAdjust*-1, pLoop->n_ਬਾਹਰਲਾ));
    }

  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( *pb_ਪੂਰਾ==0 );
  }

  sqlite3_ਮੁੱਲ_Free(p1);
  sqlite3_ਮੁੱਲ_Free(p2);
  sqlite3_ਮੁੱਲ_Free(pVal);

  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

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
ਸਥਿਰ ਪੂਰਨ where_Range_ਜਾਂਚ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Where_ਪਦ *p_ਹੇਠਲਾ,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_ਪਦ *p_ਉੱਤਲਾ,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ n_ਬਾਹਰਲਾ = pLoop->n_ਬਾਹਰਲਾ;
  LogEst nNew;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਸੂਚਕਾਂਕ *p = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
  ਪੂਰਨ nEq = pLoop->u.btree.nEq;

  ਜੇ( p->nSample>0 && ALWAYS(nEq<p->n_Sample_ਖਾਨਾ)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    ਜੇ( nEq==pBuilder->nRecValid ){
      Unpacked_ਰਿਕਾਰਡ *pRec = pBuilder->pRec;
      tRowcnt a[2];
      ਪੂਰਨ nBtm = pLoop->u.btree.nBtm;
      ਪੂਰਨ n_ਸਿਖਰ = pLoop->u.btree.n_ਸਿਖਰ;

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
      tRowcnt i_ਹੇਠਲਾ;     /* Rows less than the lower bound */
      tRowcnt i_ਉੱਤਲਾ;     /* Rows less than the upper bound */
      ਪੂਰਨ iLwrIdx = -2;   /* aSample[] for the lower bound */
      ਪੂਰਨ iUprIdx = -1;   /* aSample[] for the upper bound */

      ਜੇ( pRec ){
        testcase( pRec->n_ਖੇਤਰ!=pBuilder->nRecValid );
        pRec->n_ਖੇਤਰ = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      ਜੇ( nEq==0 ){
        i_ਹੇਠਲਾ = 0;
        i_ਉੱਤਲਾ = p->n_ਪੰਕਤੀ_Est0;
      }ਨਹੀਂਤਾਂ{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_ਕੁੰਜੀ_Stats(pParse, p, pRec, 0, a);
        i_ਹੇਠਲਾ = a[0];
        i_ਉੱਤਲਾ = a[0] + a[1];
      }

      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਹੇਠਲਾ==0 || (p_ਹੇਠਲਾ->eOperator & (WO_GT|WO_GE))!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਉੱਤਲਾ==0 || (p_ਉੱਤਲਾ->eOperator & (WO_LT|WO_LE))!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->a_ਛਾਂਟੋ_ਕ੍ਰਮ!=0 );
      ਜੇ( p->a_ਛਾਂਟੋ_ਕ੍ਰਮ[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        ਅਦਲਾ_ਬਦਲੀ(Where_ਪਦ*, p_ਹੇਠਲਾ, p_ਉੱਤਲਾ);
        ਅਦਲਾ_ਬਦਲੀ(ਪੂਰਨ, nBtm, n_ਸਿਖਰ);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      ਜੇ( p_ਹੇਠਲਾ ){
        ਪੂਰਨ n;                    /* Values extracted from pExpr */
        ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਹੇਠਲਾ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        rc = sqlite3_Stat4_Probe_ਸੈੱਟ_ਮੁੱਲ(pParse, p, &pRec, p_ਵਿਅੰਜਕ, nBtm, nEq, &n);
        ਜੇ( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ਨਕਾਬ = WO_GT|WO_LE;
          ਜੇ( sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ)>n ) ਨਕਾਬ = (WO_LE|WO_LT);
          iLwrIdx = where_ਕੁੰਜੀ_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((p_ਹੇਠਲਾ->eOperator & ਨਕਾਬ) ? a[1] : 0);
          ਜੇ( iNew>i_ਹੇਠਲਾ ) i_ਹੇਠਲਾ = iNew;
          n_ਬਾਹਰਲਾ--;
          p_ਹੇਠਲਾ = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      ਜੇ( p_ਉੱਤਲਾ ){
        ਪੂਰਨ n;                    /* Values extracted from pExpr */
        ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਉੱਤਲਾ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        rc = sqlite3_Stat4_Probe_ਸੈੱਟ_ਮੁੱਲ(pParse, p, &pRec, p_ਵਿਅੰਜਕ, n_ਸਿਖਰ, nEq, &n);
        ਜੇ( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 ਨਕਾਬ = WO_GT|WO_LE;
          ਜੇ( sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ)>n ) ਨਕਾਬ = (WO_LE|WO_LT);
          iUprIdx = where_ਕੁੰਜੀ_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((p_ਉੱਤਲਾ->eOperator & ਨਕਾਬ) ? a[1] : 0);
          ਜੇ( iNew<i_ਉੱਤਲਾ ) i_ਉੱਤਲਾ = iNew;
          n_ਬਾਹਰਲਾ--;
          p_ਉੱਤਲਾ = 0;
        }
      }

      pBuilder->pRec = pRec;
      ਜੇ( rc==SQLITE_OK ){
        ਜੇ( i_ਉੱਤਲਾ>i_ਹੇਠਲਾ ){
          nNew = sqlite3LogEst(i_ਉੱਤਲਾ - i_ਹੇਠਲਾ);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          ਜੇ( iLwrIdx==iUprIdx ){ nNew -= 20; }
          ਪ੍ਰਤਿਜ੍ਞਾ( 20==sqlite3LogEst(4) );
        }ਨਹੀਂਤਾਂ{
          nNew = 10;        ਪ੍ਰਤਿਜ੍ਞਾ( 10==sqlite3LogEst(2) );
        }
        ਜੇ( nNew<n_ਬਾਹਰਲਾ ){
          n_ਬਾਹਰਲਾ = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)i_ਹੇਠਲਾ, (u32)i_ਉੱਤਲਾ, n_ਬਾਹਰਲਾ));
      }
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ b_ਪੂਰਾ = 0;
      rc = where_Range_Skip_ਜਾਂਚ_Est(pParse, p_ਹੇਠਲਾ, p_ਉੱਤਲਾ, pLoop, &b_ਪੂਰਾ);
      ਜੇ( b_ਪੂਰਾ ) ਵਾਪਸ rc;
    }
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(pParse);
  ਅਣਵਰਤਿਆ_PARAMETER(pBuilder);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਹੇਠਲਾ || p_ਉੱਤਲਾ );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਉੱਤਲਾ==0 || (p_ਉੱਤਲਾ->wtFlags & ਪਦ_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(p_ਹੇਠਲਾ, n_ਬਾਹਰਲਾ);
  nNew = whereRangeAdjust(p_ਉੱਤਲਾ, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  ਜੇ( p_ਹੇਠਲਾ && p_ਹੇਠਲਾ->truthProb>0 && p_ਉੱਤਲਾ && p_ਉੱਤਲਾ->truthProb>0 ){
    nNew -= 20;
  }

  n_ਬਾਹਰਲਾ -= (p_ਹੇਠਲਾ!=0) + (p_ਉੱਤਲਾ!=0);
  ਜੇ( nNew<10 ) nNew = 10;
  ਜੇ( nNew<n_ਬਾਹਰਲਾ ) n_ਬਾਹਰਲਾ = nNew;
#ਜੇ defined(WHERETRACE_ENABLED)
  ਜੇ( pLoop->n_ਬਾਹਰਲਾ>n_ਬਾਹਰਲਾ ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->n_ਬਾਹਰਲਾ, n_ਬਾਹਰਲਾ));
  }
#ਜੇ_ਅੰਤ
  pLoop->n_ਬਾਹਰਲਾ = (LogEst)n_ਬਾਹਰਲਾ;
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
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
ਸਥਿਰ ਪੂਰਨ where_Equal_ਜਾਂਚ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pn_ਪੰਕਤੀ       /* Write the revised row estimate here */
){
  ਸੂਚਕਾਂਕ *p = pBuilder->pNew->u.btree.p_ਸੂਚਕਾਂਕ;
  ਪੂਰਨ nEq = pBuilder->pNew->u.btree.nEq;
  Unpacked_ਰਿਕਾਰਡ *pRec = pBuilder->pRec;
  ਪੂਰਨ rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  ਪੂਰਨ bOk;

  ਪ੍ਰਤਿਜ੍ਞਾ( nEq>=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nEq<=p->n_ਕਾਲਮ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->aSample!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nSample>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  ਜੇ( pBuilder->nRecValid<(nEq-1) ){
    ਵਾਪਸ SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  ਜੇ( nEq>=p->n_ਕਾਲਮ ){
    *pn_ਪੰਕਤੀ = 1;
    ਵਾਪਸ SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_ਸੈੱਟ_ਮੁੱਲ(pParse, p, &pRec, p_ਵਿਅੰਜਕ, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  ਜੇ( bOk==0 ) ਵਾਪਸ SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_ਕੁੰਜੀ_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (ਪੂਰਨ)a[1]));
  *pn_ਪੰਕਤੀ = a[1];
 
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
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
ਸਥਿਰ ਪੂਰਨ where_ਵਿੱਚ_ਜਾਂਚ_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  ਵਿਅੰਜਕ_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pn_ਪੰਕਤੀ       /* Write the revised row estimate here */
){
  ਸੂਚਕਾਂਕ *p = pBuilder->pNew->u.btree.p_ਸੂਚਕਾਂਕ;
  i64 nRow0 = sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(p->ai_ਪੰਕਤੀ_Log_Est[0]);
  ਪੂਰਨ nRecValid = pBuilder->nRecValid;
  ਪੂਰਨ rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt n_ਪੰਕਤੀ_Est = 0;    /* New estimate of the number of rows */
  ਪੂਰਨ i;                  /* Loop counter */

  ਪ੍ਰਤਿਜ੍ਞਾ( p->aSample!=0 );
  ਲਈ(i=0; rc==SQLITE_OK && i<pList->n_ਵਿਅੰਜਕ; i++){
    nEst = nRow0;
    rc = where_Equal_ਜਾਂਚ_Est(pParse, pBuilder, pList->a[i].p_ਵਿਅੰਜਕ, &nEst);
    n_ਪੰਕਤੀ_Est += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( n_ਪੰਕਤੀ_Est > (tRowcnt)nRow0 ) n_ਪੰਕਤੀ_Est = nRow0;
    *pn_ਪੰਕਤੀ = n_ਪੰਕਤੀ_Est;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", n_ਪੰਕਤੀ_Est));
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pBuilder->nRecValid==nRecValid );
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */


#ਜੇ defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
ਖਾਲੀ sqlite3_Where_ਪਦ_Print(Where_ਪਦ *p_ਪਦ, ਪੂਰਨ i_ਪਦ){
  ਜੇ( p_ਪਦ==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", i_ਪਦ);
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ zType[8];
    ਅੱਖਰ z_ਖੱਬਾ[50];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zType, "....", 5);
    ਜੇ( p_ਪਦ->wtFlags & ਪਦ_VIRTUAL ) zType[0] = 'V';
    ਜੇ( p_ਪਦ->eOperator & WO_EQUIV  ) zType[1] = 'E';
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON) ) zType[2] = 'L';
    ਜੇ( p_ਪਦ->wtFlags & ਪਦ_CODED  ) zType[3] = 'C';
    ਜੇ( p_ਪਦ->eOperator & WO_SINGLE ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(ਆਕਾਰ(z_ਖੱਬਾ),z_ਖੱਬਾ,"left={%d:%d}",
                       p_ਪਦ->ਖੱਬਾ_Cursor, p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ);
    }ਨਹੀਂਤਾਂ ਜੇ( (p_ਪਦ->eOperator & WO_OR)!=0 && p_ਪਦ->u.pOrInfo!=0 ){
      sqlite3_snprintf(ਆਕਾਰ(z_ਖੱਬਾ),z_ਖੱਬਾ,"indexable=0x%llx",
                       p_ਪਦ->u.pOrInfo->indexable);
    }ਨਹੀਂਤਾਂ{
      sqlite3_snprintf(ਆਕਾਰ(z_ਖੱਬਾ),z_ਖੱਬਾ,"left=%d", p_ਪਦ->ਖੱਬਾ_Cursor);
    }
    i_ਪਦ = p_ਪਦ->i_ਪਦ = MAX(i_ਪਦ,p_ਪਦ->i_ਪਦ);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       i_ਪਦ, p_ਪਦ, zType, z_ਖੱਬਾ, p_ਪਦ->eOperator, p_ਪਦ->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    ਜੇ( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        p_ਪਦ->truthProb, (u64)p_ਪਦ->prereqAll, (u64)p_ਪਦ->prereq_ਸੱਜਾ);
    }
    ਜੇ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 && p_ਪਦ->u.x.i_ਖੇਤਰ ){
      sqlite3DebugPrintf(" iField=%d", p_ਪਦ->u.x.i_ਖੇਤਰ);
    }
    ਜੇ( p_ਪਦ->i_ਮਾਪਾ>=0 ){
      sqlite3DebugPrintf(" iParent=%d", p_ਪਦ->i_ਮਾਪਾ);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, p_ਪਦ->p_ਵਿਅੰਜਕ, 0);
  }
}
ਖਾਲੀ sqlite3_Show_Where_ਪਦ(Where_ਪਦ *p_ਪਦ){
  sqlite3_Where_ਪਦ_Print(p_ਪਦ, 0);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
ਖਾਲੀ sqlite3WhereClausePrint(WhereClause *pWC){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<pWC->n_ਪਦ; i++){
    sqlite3_Where_ਪਦ_Print(&pWC->a[i], i);
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
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
ਖਾਲੀ sqlite3WhereLoopPrint(ਅਚਲ WhereLoop *p, ਅਚਲ WhereClause *pWC){
  WhereInfo *pWInfo;
  ਜੇ( pWC ){
    ਪੂਰਨ nb;
    Src_ਵਸਤੂ *p_ਵਸਤੂ;
    ਸਾਰਣੀ *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    p_ਵਸਤੂ = pWInfo->pTabList->a + p->iTab;
    pTab = p_ਵਸਤੂ->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->ਨਕਾਬ_Self, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       p_ਵਸਤੂ->zAlias ? p_ਵਸਤੂ->zAlias : pTab->zName);
  }ਨਹੀਂਤਾਂ{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->ਨਕਾਬ_Self, p->prereq & 0xfff, p->cId, p->iTab);
  }
  ਜੇ( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ਅਚਲ ਅੱਖਰ *zName;
    ਜੇ( p->u.btree.p_ਸੂਚਕਾਂਕ && (zName = p->u.btree.p_ਸੂਚਕਾਂਕ->zName)!=0 ){
      ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zName, "sqlite_autoindex_", 17)==0 ){
        ਪੂਰਨ i = sqlite3Strlen30(zName) - 1;
        ਜਦੋਂ( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }ਨਹੀਂਤਾਂ{
      sqlite3DebugPrintf("%20s","");
    }
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *z;
    ਜੇ( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_ਅੰਕ, p->u.vtab.idxStr, p->u.vtab.omit_ਨਕਾਬ);
    }ਨਹੀਂਤਾਂ{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_ਅੰਕ, p->u.vtab.omit_ਨਕਾਬ);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  ਜੇ( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }ਨਹੀਂਤਾਂ{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  ਜੇ( pWInfo && pWInfo->b_ਤਾਰਾ_ਵਰਤਿਆ && p->r_ਤਾਰਾ_Delta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->r_ਤਿਆਰੀ, p->r_ਲੜੀਵਾਰ, p->n_ਬਾਹਰਲਾ, p->r_ਤਾਰਾ_Delta);
  }ਨਹੀਂਤਾਂ{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->r_ਤਿਆਰੀ, p->r_ਲੜੀਵਾਰ, p->n_ਬਾਹਰਲਾ);
  }
  ਜੇ( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->nLTerm; i++){
      sqlite3_Where_ਪਦ_Print(p->aLTerm[i], i);
    }
  }
}
ਖਾਲੀ sqlite3ShowWhereLoop(ਅਚਲ WhereLoop *p){
  ਜੇ( p ) sqlite3WhereLoopPrint(p, 0);
}
ਖਾਲੀ sqlite3ShowWhereLoopList(ਅਚਲ WhereLoop *p){
  ਜਦੋਂ( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_ਅਗਲਾ_Loop;
  }
}
#ਜੇ_ਅੰਤ

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
ਸਥਿਰ ਖਾਲੀ whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_ਮਾਪ(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
ਸਥਿਰ ਖਾਲੀ whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  ਜੇ( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_ਸੂਚਕਾਂਕ) ){
    ਜੇ( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( (p->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)!=0 && p->u.btree.p_ਸੂਚਕਾਂਕ!=0 ){
      sqlite3DbFree(db, p->u.btree.p_ਸੂਚਕਾਂਕ->z_ਖਾਨਾ_Aff);
      sqlite3DbFreeNN(db, p->u.btree.p_ਸੂਚਕਾਂਕ);
      p->u.btree.p_ਸੂਚਕਾਂਕ = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
ਸਥਿਰ ਖਾਲੀ whereLoopClear(sqlite3 *db, WhereLoop *p){
  ਜੇ( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_ਮਾਪ(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
ਸਥਿਰ ਪੂਰਨ whereLoopResize(sqlite3 *db, WhereLoop *p, ਪੂਰਨ n){
  Where_ਪਦ **paNew;
  ਜੇ( p->nLSlot>=n ) ਵਾਪਸ SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, ਆਕਾਰ(p->aLTerm[0])*n);
  ਜੇ( paNew==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(paNew, p->aLTerm, ਆਕਾਰ(p->aLTerm[0])*p->nLSlot);
  ਜੇ( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
ਸਥਿਰ ਪੂਰਨ whereLoopXfer(sqlite3 *db, WhereLoop *p_ਮੰਜ਼ਿਲ, WhereLoop *pFrom){
  whereLoopClearUnion(db, p_ਮੰਜ਼ਿਲ);
  ਜੇ( pFrom->nLTerm > p_ਮੰਜ਼ਿਲ->nLSlot
   && whereLoopResize(db, p_ਮੰਜ਼ਿਲ, pFrom->nLTerm)
  ){
    memset(p_ਮੰਜ਼ਿਲ, 0, WHERE_LOOP_XFER_SZ);
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ, pFrom, WHERE_LOOP_XFER_SZ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ->aLTerm, pFrom->aLTerm, p_ਮੰਜ਼ਿਲ->nLTerm*ਆਕਾਰ(p_ਮੰਜ਼ਿਲ->aLTerm[0]));
  ਜੇ( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( (pFrom->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)!=0 ){
    pFrom->u.btree.p_ਸੂਚਕਾਂਕ = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
ਸਥਿਰ ਖਾਲੀ whereLoopDelete(sqlite3 *db, WhereLoop *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
ਸਥਿਰ ਖਾਲੀ whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  ਜਦੋਂ( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_ਅਗਲਾ_Loop;
    whereLoopDelete(db, p);
  }
  ਜਦੋਂ( pWInfo->p_Mem_ਮੰਜ਼ਿਲ_Free ){
    Where_Mem_ਖੰਡ *p_ਅਗਲਾ = pWInfo->p_Mem_ਮੰਜ਼ਿਲ_Free->p_ਅਗਲਾ;
    sqlite3DbNNFreeNN(db, pWInfo->p_Mem_ਮੰਜ਼ਿਲ_Free);
    pWInfo->p_Mem_ਮੰਜ਼ਿਲ_Free = p_ਅਗਲਾ;
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
ਸਥਿਰ ਪੂਰਨ whereLoopCheaperProperSubset(
  ਅਚਲ WhereLoop *pX,       /* First WhereLoop to compare */
  ਅਚਲ WhereLoop *pY        /* Compare against this WhereLoop */
){
  ਪੂਰਨ i, j;
  ਜੇ( pX->r_ਲੜੀਵਾਰ>pY->r_ਲੜੀਵਾਰ && pX->n_ਬਾਹਰਲਾ>pY->n_ਬਾਹਰਲਾ ) ਵਾਪਸ 0; /* (1d) and (2a) */
  ਪ੍ਰਤਿਜ੍ਞਾ( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  ਜੇ( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_ਸੂਚਕਾਂਕ==pY->u.btree.p_ਸੂਚਕਾਂਕ             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    ਵਾਪਸ 1;  /* Case 1 is true */
  }
  ਜੇ( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    ਵਾਪਸ 0;                                            /* (2b) */
  }
  ਜੇ( pY->nSkip > pX->nSkip ) ਵਾਪਸ 0;                  /* (2d) */
  ਲਈ(i=pX->nLTerm-1; i>=0; i--){
    ਜੇ( pX->aLTerm[i]==0 ) ਜਾਰੀ;
    ਲਈ(j=pY->nLTerm-1; j>=0; j--){
      ਜੇ( pY->aLTerm[j]==pX->aLTerm[i] ) ਰੋਕੋ;
    }
    ਜੇ( j<0 ) ਵਾਪਸ 0;                                  /* (2c) */
  }
  ਜੇ( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    ਵਾਪਸ 0;                                            /* (2e) */
  }
  ਵਾਪਸ 1;  /* Case 2 is true */
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
ਸਥਿਰ ਖਾਲੀ where_Loop_Adjust_ਲਾਗਤ(ਅਚਲ WhereLoop *p, WhereLoop *pTemplate){
  ਜੇ( (pTemplate->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)==0 ) ਵਾਪਸ;
  ਲਈ(; p; p=p->p_ਅਗਲਾ_Loop){
    ਜੇ( p->iTab!=pTemplate->iTab ) ਜਾਰੀ;
    ਜੇ( (p->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)==0 ) ਜਾਰੀ;
    ਜੇ( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_ਲੜੀਵਾਰ, pTemplate->n_ਬਾਹਰਲਾ,
                       MIN(p->r_ਲੜੀਵਾਰ, pTemplate->r_ਲੜੀਵਾਰ),
                       MIN(p->n_ਬਾਹਰਲਾ - 1, pTemplate->n_ਬਾਹਰਲਾ)));
      pTemplate->r_ਲੜੀਵਾਰ = MIN(p->r_ਲੜੀਵਾਰ, pTemplate->r_ਲੜੀਵਾਰ);
      pTemplate->n_ਬਾਹਰਲਾ = MIN(p->n_ਬਾਹਰਲਾ - 1, pTemplate->n_ਬਾਹਰਲਾ);
    }ਨਹੀਂਤਾਂ ਜੇ( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_ਲੜੀਵਾਰ, pTemplate->n_ਬਾਹਰਲਾ,
                       MAX(p->r_ਲੜੀਵਾਰ, pTemplate->r_ਲੜੀਵਾਰ),
                       MAX(p->n_ਬਾਹਰਲਾ + 1, pTemplate->n_ਬਾਹਰਲਾ)));
      pTemplate->r_ਲੜੀਵਾਰ = MAX(p->r_ਲੜੀਵਾਰ, pTemplate->r_ਲੜੀਵਾਰ);
      pTemplate->n_ਬਾਹਰਲਾ = MAX(p->n_ਬਾਹਰਲਾ + 1, pTemplate->n_ਬਾਹਰਲਾ);
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
ਸਥਿਰ WhereLoop **where_Loop_ਲੱਭੋ_Lesser(
  WhereLoop **ppPrev,
  ਅਚਲ WhereLoop *pTemplate
){
  WhereLoop *p;
  ਲਈ(p=(*ppPrev); p; ppPrev=&p->p_ਅਗਲਾ_Loop, p=*ppPrev){
    ਜੇ( p->iTab!=pTemplate->iTab || p->i_ਛਾਂਟੋ_Idx!=pTemplate->i_ਛਾਂਟੋ_Idx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      ਜਾਰੀ;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->r_ਤਿਆਰੀ==0 || pTemplate->r_ਤਿਆਰੀ==0
                 || p->r_ਤਿਆਰੀ==pTemplate->r_ਤਿਆਰੀ );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->r_ਤਿਆਰੀ>=pTemplate->r_ਤਿਆਰੀ );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    ਜੇ( (p->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)!=0
     && (pTemplate->wsFlags & WHERE_ਕਾਲਮ_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      ਰੋਕੋ;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    ਜੇ( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->r_ਤਿਆਰੀ<=pTemplate->r_ਤਿਆਰੀ                  /* (2a) */
     && p->r_ਲੜੀਵਾਰ<=pTemplate->r_ਲੜੀਵਾਰ                      /* (2b) */
     && p->n_ਬਾਹਰਲਾ<=pTemplate->n_ਬਾਹਰਲਾ                      /* (2c) */
    ){
      ਵਾਪਸ 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    ਜੇ( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->r_ਲੜੀਵਾਰ>=pTemplate->r_ਲੜੀਵਾਰ                             /* (2a) */
     && p->n_ਬਾਹਰਲਾ>=pTemplate->n_ਬਾਹਰਲਾ                             /* (2b) */
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->r_ਤਿਆਰੀ>=pTemplate->r_ਤਿਆਰੀ ); /* SETUP-INVARIANT above */
      ਰੋਕੋ;   /* Cause p to be overwritten by pTemplate */
    }
  }
  ਵਾਪਸ ppPrev;
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
ਸਥਿਰ ਪੂਰਨ where_Loop_ਪਾਓ(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  ਪੂਰਨ rc;

  /* Stop the search once we hit the query planner search limit */
  ਜੇ( pBuilder->i_Plan_ਹੱਦਬੰਦੀ==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    ਜੇ( pBuilder->p_Or_ਸੈੱਟ ) pBuilder->p_Or_ਸੈੱਟ->n = 0;
    ਵਾਪਸ SQLITE_ਪੂਰਾ;
  }
  pBuilder->i_Plan_ਹੱਦਬੰਦੀ--;

  where_Loop_Adjust_ਲਾਗਤ(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  ਜੇ( pBuilder->p_Or_ਸੈੱਟ!=0 ){
    ਜੇ( pTemplate->nLTerm ){
#ਜੇ WHERETRACE_ENABLED
      u16 n = pBuilder->p_Or_ਸੈੱਟ->n;
      ਪੂਰਨ x =
#ਜੇ_ਅੰਤ
      where_Or_ਪਾਓ(pBuilder->p_Or_ਸੈੱਟ, pTemplate->prereq, pTemplate->r_ਲੜੀਵਾਰ,
                                    pTemplate->n_ਬਾਹਰਲਾ);
#ਜੇ WHERETRACE_ENABLED /* 0x8 */
      ਜੇ( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#ਜੇ_ਅੰਤ
    }
    ਵਾਪਸ SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  ppPrev = where_Loop_ਲੱਭੋ_Lesser(&pWInfo->pLoops, pTemplate);

  ਜੇ( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#ਜੇ WHERETRACE_ENABLED /* 0x8 */
    ਜੇ( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#ਜੇ_ਅੰਤ
    ਵਾਪਸ SQLITE_OK; 
  }ਨਹੀਂਤਾਂ{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#ਜੇ WHERETRACE_ENABLED /* 0x8 */
  ਜੇ( sqlite3WhereTrace & 0x8 ){
    ਜੇ( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }ਨਹੀਂਤਾਂ{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRawNN(db, ਆਕਾਰ(WhereLoop));
    ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_ਅਗਲਾ_Loop = 0;
  }ਨਹੀਂਤਾਂ{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_ਪੂਛ = &p->p_ਅਗਲਾ_Loop;
    WhereLoop *p_ਮੰਜ਼ਿਲ_Del;
    ਜਦੋਂ( *pp_ਪੂਛ ){
      pp_ਪੂਛ = where_Loop_ਲੱਭੋ_Lesser(pp_ਪੂਛ, pTemplate);
      ਜੇ( pp_ਪੂਛ==0 ) ਰੋਕੋ;
      p_ਮੰਜ਼ਿਲ_Del = *pp_ਪੂਛ;
      ਜੇ( p_ਮੰਜ਼ਿਲ_Del==0 ) ਰੋਕੋ;
      *pp_ਪੂਛ = p_ਮੰਜ਼ਿਲ_Del->p_ਅਗਲਾ_Loop;
#ਜੇ WHERETRACE_ENABLED /* 0x8 */
      ਜੇ( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(p_ਮੰਜ਼ਿਲ_Del, pBuilder->pWC);
      }
#ਜੇ_ਅੰਤ
      whereLoopDelete(db, p_ਮੰਜ਼ਿਲ_Del);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  ਜੇ( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ = p->u.btree.p_ਸੂਚਕਾਂਕ;
    ਜੇ( p_ਸੂਚਕਾਂਕ && p_ਸੂਚਕਾਂਕ->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_ਸੂਚਕਾਂਕ = 0;
    }
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਗੰਢ_ਨਮੂਨਾ_ਲੰਮਾਈ_Est(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_STRING ){
    ਪੂਰਨ sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_ਵਿਅੰਜਕ->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    ਜੇ( pWalker->e_ਕੋਡ ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }ਨਹੀਂਤਾਂ{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    ਜਦੋਂ( (c = *(z++))!=0 ){
      ਜੇ( c==c3 ){
        ਜੇ( *z ) z++;
        ਜਦੋਂ( *z && *z!=']' ) z++;
      }ਨਹੀਂਤਾਂ ਜੇ( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    ਜੇ( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਪੂਰਨ est_Like_ਨਮੂਨਾ_ਲੰਮਾਈ(ਵਿਅੰਜਕ *p, u16 e_ਕੋਡ){
  Walker w;
  w.u.sz = 0;
  w.e_ਕੋਡ = e_ਕੋਡ;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਗੰਢ_ਨਮੂਨਾ_ਲੰਮਾਈ_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ਜੇ_ਅੰਤ
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.u.sz;
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
ਸਥਿਰ ਖਾਲੀ whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst n_ਪੰਕਤੀ            /* Number of rows in the entire table */
){
  Where_ਪਦ *p_ਪਦ, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->ਨਕਾਬ_Self);
  ਪੂਰਨ i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  ਜੇ( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->p_ਕ੍ਰਮ_By==0 ){
    ਵਾਪਸ;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)==0 );
  ਲਈ(i=pWC->n_ਅਧਾਰ, p_ਪਦ=pWC->a; i>0; i--, p_ਪਦ++){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
    ਜੇ( (p_ਪਦ->prereqAll & notAllowed)!=0 ) ਜਾਰੀ;
    ਜੇ( (p_ਪਦ->prereqAll & pLoop->ਨਕਾਬ_Self)==0 ) ਜਾਰੀ;
    ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_VIRTUAL)!=0 ) ਜਾਰੀ;
    ਲਈ(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      ਜੇ( pX==0 ) ਜਾਰੀ;
      ਜੇ( pX==p_ਪਦ ) ਰੋਕੋ;
      ਜੇ( pX->i_ਮਾਪਾ>=0 && (&pWC->a[pX->i_ਮਾਪਾ])==p_ਪਦ ) ਰੋਕੋ;
    }
    ਜੇ( j<0 ){
      sqlite3_ਤਰੱਕੀ_Check(pWC->pWInfo->pParse);
      ਜੇ( pLoop->ਨਕਾਬ_Self==p_ਪਦ->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        ਜੇ( (p_ਪਦ->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_ਖੱਬਾ|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      ਜੇ( p_ਪਦ->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->n_ਬਾਹਰਲਾ += p_ਪਦ->truthProb;
      }ਨਹੀਂਤਾਂ{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        ਵਿਅੰਜਕ *p_Op_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
        pLoop->n_ਬਾਹਰਲਾ--;
        ਜੇ( (p_ਪਦ->eOperator&(WO_EQ|WO_IS))!=0
         && (p_ਪਦ->wtFlags & ਪਦ_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_Op_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
          Parse *pParse = pWC->pWInfo->pParse;
          ਪੂਰਨ k = 0;
          testcase( p_Op_ਵਿਅੰਜਕ->op==TK_IS );
          ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(p_ਸੱਜਾ, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }ਨਹੀਂਤਾਂ{
            k = 20;
          }
          ਜੇ( iReduce<k ){
            p_ਪਦ->wtFlags |= ਪਦ_HEURTRUTH;
            iReduce = k;
          }
        }ਨਹੀਂਤਾਂ
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_Op_ਵਿਅੰਜਕ, EP_InfixFunc) 
         && p_Op_ਵਿਅੰਜਕ->op==TK_FUNCTION
        ){
          ਪੂਰਨ eOp;
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_Op_ਵਿਅੰਜਕ) );
          ਪ੍ਰਤਿਜ੍ਞਾ( p_Op_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ>=2 );
          eOp = sqlite3_ਵਿਅੰਜਕ_Is_Like_Operator(p_Op_ਵਿਅੰਜਕ);
          ਜੇ( ALWAYS(eOp>0) ){
            ਪੂਰਨ sz_ਨਮੂਨਾ;
            ਵਿਅੰਜਕ *pRHS = p_Op_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ;
            eOp = eOp==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE;
            sz_ਨਮੂਨਾ = est_Like_ਨਮੂਨਾ_ਲੰਮਾਈ(pRHS, eOp);
            ਜੇ( sz_ਨਮੂਨਾ>0 ){
              pLoop->n_ਬਾਹਰਲਾ -= sz_ਨਮੂਨਾ*2;
            }
          }
        }
      }
    }
  }
  ਜੇ( pLoop->n_ਬਾਹਰਲਾ > n_ਪੰਕਤੀ-iReduce ){
    pLoop->n_ਬਾਹਰਲਾ = n_ਪੰਕਤੀ - iReduce;
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
ਸਥਿਰ ਪੂਰਨ where_Range_ਸਦਿਸ਼_Len(
  Parse *pParse,       /* Parsing context */
  ਪੂਰਨ iCur,            /* Cursor open on pIdx */
  ਸੂਚਕਾਂਕ *pIdx,         /* The index to be used for a inequality constraint */
  ਪੂਰਨ nEq,             /* Number of prior equality constraints on same index */
  Where_ਪਦ *p_ਪਦ     /* The vector inequality constraint */
){
  ਪੂਰਨ nCmp = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
  ਪੂਰਨ i;

  nCmp = MIN(nCmp, (pIdx->n_ਕਾਲਮ - nEq));
  ਲਈ(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    ਅੱਖਰ aff;                     /* Comparison affinity */
    ਅੱਖਰ idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    ਵਿਅੰਜਕ *pLhs, *pRhs;

    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
    pLhs = p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pList->a[i].p_ਵਿਅੰਜਕ;
    pRhs = p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
    ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_ਵਿਅੰਜਕ;
    }ਨਹੀਂਤਾਂ{
      pRhs = pRhs->x.pList->a[i].p_ਵਿਅੰਜਕ;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    ਜੇ( pLhs->op!=TK_ਕਾਲਮ
     || pLhs->i_ਸਾਰਣੀ!=iCur
     || pLhs->i_ਕਾਲਮ!=pIdx->ai_ਕਾਲਮ[i+nEq]
     || pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[i+nEq]!=pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[nEq]
    ){
      ਰੋਕੋ;
    }

    testcase( pLhs->i_ਕਾਲਮ==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_ਵਿਅੰਜਕ_Affinity(pLhs));
    idxaff = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_Affinity(pIdx->p_ਸਾਰਣੀ, pLhs->i_ਕਾਲਮ);
    ਜੇ( aff!=idxaff ) ਰੋਕੋ;

    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_Commuted) ) ਅਦਲਾ_ਬਦਲੀ(ਵਿਅੰਜਕ*, pRhs, pLhs);
    pColl = sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, pLhs, pRhs);
    ਜੇ( pColl==0 ) ਰੋਕੋ;
    ਜੇ( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) ਰੋਕੋ;
  }
  ਵਾਪਸ i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_COSTMULT
# define Apply_ਲਾਗਤ_Multiplier(C,T)  C += T
#ਨਹੀਂਤਾਂ
# define Apply_ਲਾਗਤ_Multiplier(C,T)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ where_Loop_ਜੋੜੋ_Btree_ਸੂਚਕਾਂਕ(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  Src_ਵਸਤੂ *pSrc,                  /* FROM clause term being analyzed */
  ਸੂਚਕਾਂਕ *pProbe,                  /* An index on pSrc */
  LogEst n_ਵਿੱਚ_Mul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  Where_ਪਦ *p_ਪਦ;               /* A WhereTerm under consideration */
  ਪੂਰਨ op_ਨਕਾਬ;                     /* Valid operators for constraints */
  Where_ਜਾਂਚ ਜਾਂਚ;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_ਸਿਖਰ;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_n_ਬਾਹਰਲਾ;              /* Original value of pNew->nOut */
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  LogEst r_ਮਾਪ;                   /* Number of rows in the table */
  LogEst r_Log_ਮਾਪ;                /* Logarithm of table size */
  Where_ਪਦ *p_ਸਿਖਰ = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 || pParse->nErr>0 );
  ਜੇ( pParse->nErr ){
    ਵਾਪਸ pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->p_ਸਾਰਣੀ->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->r_ਲੜੀਵਾਰ));

  ਪ੍ਰਤਿਜ੍ਞਾ( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pNew->wsFlags & WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ)==0 );
  ਜੇ( pNew->wsFlags & WHERE_BTM_ਹੱਦਬੰਦੀ ){
    op_ਨਕਾਬ = WO_LT|WO_LE;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew->u.btree.nBtm==0 );
    op_ਨਕਾਬ = WO_EQ|WO_ਵਿੱਚ|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  ਜੇ( pProbe->bUnordered ){
    op_ਨਕਾਬ &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->u.btree.nEq<pProbe->n_ਕਾਲਮ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->u.btree.nEq<pProbe->n_ਕੁੰਜੀ_ਖਾਨਾ
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_ਸਿਖਰ = pNew->u.btree.n_ਸਿਖਰ;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_n_ਬਾਹਰਲਾ = pNew->n_ਬਾਹਰਲਾ;
  p_ਪਦ = where_ਜਾਂਚ_Init(&ਜਾਂਚ, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        op_ਨਕਾਬ, pProbe);
  pNew->r_ਤਿਆਰੀ = 0;
  r_ਮਾਪ = pProbe->ai_ਪੰਕਤੀ_Log_Est[0];
  r_Log_ਮਾਪ = estLog(r_ਮਾਪ);
  ਲਈ(; rc==SQLITE_OK && p_ਪਦ!=0; p_ਪਦ = where_ਜਾਂਚ_ਅਗਲਾ(&ਜਾਂਚ)){
    u16 eOp = p_ਪਦ->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst r_ਲਾਗਤ_Idx;
    LogEst n_ਬਾਹਰਲਾ_Unadjusted;        /* nOut before IN() and WHERE adjustments */
    ਪੂਰਨ n_ਵਿੱਚ = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਪੂਰਨ nRecValid = pBuilder->nRecValid;
#ਜੇ_ਅੰਤ
    ਜੇ( (eOp==WO_ISNULL || (p_ਪਦ->wtFlags&ਪਦ_VNULL)!=0)
     && ਸੂਚਕਾਂਕ_ਕਾਲਮ_Not_Null(pProbe, saved_nEq)
    ){
      ਜਾਰੀ; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    ਜੇ( p_ਪਦ->prereq_ਸੱਜਾ & pNew->ਨਕਾਬ_Self ) ਜਾਰੀ;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    ਜੇ( p_ਪਦ->wtFlags & ਪਦ_LIKEOPT && p_ਪਦ->eOperator==WO_LT ) ਜਾਰੀ;

    ਜੇ( (pSrc->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))!=0
     && !constraintCompatibleWithOuterJoin(p_ਪਦ,pSrc)
    ){
      ਜਾਰੀ;
    }
    ਜੇ( Is_Unique_ਸੂਚਕਾਂਕ(pProbe) && saved_nEq==pProbe->n_ਕੁੰਜੀ_ਖਾਨਾ-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }ਨਹੀਂਤਾਂ{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_ਸੂਚਕਾਂਕਿਤ;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_ਸਿਖਰ = saved_n_ਸਿਖਰ;
    pNew->nLTerm = saved_nLTerm;
    ਜੇ( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       ਰੋਕੋ; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = p_ਪਦ;
    pNew->prereq = (saved_prereq | p_ਪਦ->prereq_ਸੱਜਾ) & ~pNew->ਨਕਾਬ_Self;

    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਵਿੱਚ_Mul==0
        || (pNew->wsFlags & WHERE_ਕਾਲਮ_NULL)!=0
        || (pNew->wsFlags & WHERE_ਕਾਲਮ_ਵਿੱਚ)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    ਜੇ( eOp & WO_ਵਿੱਚ ){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
      ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        ਪੂਰਨ i;
        ਪੂਰਨ bRedundant = 0;
        n_ਵਿੱਚ = 46;  ਪ੍ਰਤਿਜ੍ਞਾ( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        ਲਈ(i=0; i<pNew->nLTerm-1; i++){
          ਜੇ( pNew->aLTerm[i] && pNew->aLTerm[i]->p_ਵਿਅੰਜਕ==p_ਵਿਅੰਜਕ ){
            n_ਵਿੱਚ = 0;
            ਜੇ( pNew->aLTerm[i]->u.x.i_ਖੇਤਰ == p_ਪਦ->u.x.i_ਖੇਤਰ ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        ਜੇ( bRedundant ){
          pNew->nLTerm--;
          ਜਾਰੀ;
        }
      }ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(p_ਵਿਅੰਜਕ->x.pList && p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ) ){
        /* "x IN (value, value, ...)" */
        n_ਵਿੱਚ = sqlite3LogEst(p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ);
      }
      ਜੇ( pProbe->hasStat1 && r_Log_ਮਾਪ>=10 ){
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
        M = pProbe->ai_ਪੰਕਤੀ_Log_Est[saved_nEq];
        logK = estLog(n_ਵਿੱਚ);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (n_ਵਿੱਚ + r_Log_ਮਾਪ);
        ਜੇ( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, n_ਵਿੱਚ, r_Log_ਮਾਪ, x));
        }ਨਹੀਂਤਾਂ ਜੇ( n_ਵਿੱਚ_Mul<2 && OptimizationEnabled(db, SQLITE_Seek_ਜਾਂਚ) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, n_ਵਿੱਚ, r_Log_ਮਾਪ, x, n_ਵਿੱਚ_Mul));
          pNew->wsFlags |= WHERE_ਵਿੱਚ_SEEKSCAN;
        }ਨਹੀਂਤਾਂ{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, n_ਵਿੱਚ, r_Log_ਮਾਪ, x, n_ਵਿੱਚ_Mul));
          ਜਾਰੀ;
        }
      }
      pNew->wsFlags |= WHERE_ਕਾਲਮ_ਵਿੱਚ;
    }ਨਹੀਂਤਾਂ ਜੇ( eOp & (WO_EQ|WO_IS) ){
      ਪੂਰਨ i_ਖਾਨਾ = pProbe->ai_ਕਾਲਮ[saved_nEq];
      pNew->wsFlags |= WHERE_ਕਾਲਮ_EQ;
      ਪ੍ਰਤਿਜ੍ਞਾ( saved_nEq==pNew->u.btree.nEq );
      ਜੇ( i_ਖਾਨਾ==XN_ROWID
       || (i_ਖਾਨਾ>=0 && n_ਵਿੱਚ_Mul==0 && saved_nEq==pProbe->n_ਕੁੰਜੀ_ਖਾਨਾ-1)
      ){
        ਜੇ( i_ਖਾਨਾ==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_ਕੁੰਜੀ_ਖਾਨਾ==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }ਨਹੀਂਤਾਂ{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      ਜੇ( ਜਾਂਚ.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }ਨਹੀਂਤਾਂ ਜੇ( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_ਕਾਲਮ_NULL;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nVecLen = where_Range_ਸਦਿਸ਼_Len(
          pParse, pSrc->iCursor, pProbe, saved_nEq, p_ਪਦ
      );
      ਜੇ( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_ਕਾਲਮ_RANGE|WHERE_BTM_ਹੱਦਬੰਦੀ;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = p_ਪਦ;
        p_ਸਿਖਰ = 0;
        ਜੇ( p_ਪਦ->wtFlags & ਪਦ_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_ਸਿਖਰ = &p_ਪਦ[1];
          ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਸਿਖਰ-(p_ਪਦ->pWC->a))<p_ਪਦ->pWC->n_ਪਦ );
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਿਖਰ->wtFlags & ਪਦ_LIKEOPT );
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਿਖਰ->eOperator==WO_LT );
          ਜੇ( whereLoopResize(db, pNew, pNew->nLTerm+1) ) ਰੋਕੋ; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_ਸਿਖਰ;
          pNew->wsFlags |= WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ;
          pNew->u.btree.n_ਸਿਖਰ = 1;
        }
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_ਕਾਲਮ_RANGE|WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ;
        pNew->u.btree.n_ਸਿਖਰ = nVecLen;
        p_ਸਿਖਰ = p_ਪਦ;
        pBtm = (pNew->wsFlags & WHERE_BTM_ਹੱਦਬੰਦੀ)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew->n_ਬਾਹਰਲਾ==saved_n_ਬਾਹਰਲਾ );
    ਜੇ( pNew->wsFlags & WHERE_ਕਾਲਮ_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      where_Range_ਜਾਂਚ_Est(pParse, pBuilder, pBtm, p_ਸਿਖਰ, pNew);
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nEq = ++pNew->u.btree.nEq;
      ਪ੍ਰਤਿਜ੍ਞਾ( eOp & (WO_ISNULL|WO_EQ|WO_ਵਿੱਚ|WO_IS) );

      ਪ੍ਰਤਿਜ੍ਞਾ( pNew->n_ਬਾਹਰਲਾ==saved_n_ਬਾਹਰਲਾ );
      ਜੇ( p_ਪਦ->truthProb<=0 && pProbe->ai_ਕਾਲਮ[saved_nEq]>=0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( (eOp & WO_ਵਿੱਚ) || n_ਵਿੱਚ==0 );
        testcase( eOp & WO_ਵਿੱਚ );
        pNew->n_ਬਾਹਰਲਾ += p_ਪਦ->truthProb;
        pNew->n_ਬਾਹਰਲਾ -= n_ਵਿੱਚ;
      }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
        tRowcnt n_ਬਾਹਰਲਾ = 0;
        ਜੇ( n_ਵਿੱਚ_Mul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->n_Sample_ਖਾਨਾ)
         && ((eOp & WO_ਵਿੱਚ)==0 || ਵਿਅੰਜਕ_Use_XList(p_ਪਦ->p_ਵਿਅੰਜਕ))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
          ਜੇ( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = where_Equal_ਜਾਂਚ_Est(pParse, pBuilder, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, &n_ਬਾਹਰਲਾ);
          }ਨਹੀਂਤਾਂ{
            rc = where_ਵਿੱਚ_ਜਾਂਚ_Est(pParse, pBuilder, p_ਵਿਅੰਜਕ->x.pList, &n_ਬਾਹਰਲਾ);
          }
          ਜੇ( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          ਜੇ( rc!=SQLITE_OK ) ਰੋਕੋ;          /* Jump out of the pTerm loop */
          ਜੇ( n_ਬਾਹਰਲਾ ){
            pNew->n_ਬਾਹਰਲਾ = sqlite3LogEst(n_ਬਾਹਰਲਾ);
            ਜੇ( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->n_ਬਾਹਰਲਾ+10 > pProbe->ai_ਪੰਕਤੀ_Log_Est[0]
            ){
#ਜੇ WHERETRACE_ENABLED /* 0x01 */
              ਜੇ( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3_Where_ਪਦ_Print(p_ਪਦ, 999);
              }
#ਜੇ_ਅੰਤ
              p_ਪਦ->wtFlags |= ਪਦ_HIGHTRUTH;
              ਜੇ( p_ਪਦ->wtFlags & ਪਦ_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            ਜੇ( pNew->n_ਬਾਹਰਲਾ>saved_n_ਬਾਹਰਲਾ ) pNew->n_ਬਾਹਰਲਾ = saved_n_ਬਾਹਰਲਾ;
            pNew->n_ਬਾਹਰਲਾ -= n_ਵਿੱਚ;
          }
        }
        ਜੇ( n_ਬਾਹਰਲਾ==0 )
#ਜੇ_ਅੰਤ
        {
          pNew->n_ਬਾਹਰਲਾ += (pProbe->ai_ਪੰਕਤੀ_Log_Est[nEq] - pProbe->ai_ਪੰਕਤੀ_Log_Est[nEq-1]);
          ਜੇ( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->n_ਬਾਹਰਲਾ += 10;
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
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->pSTab->sz_Tab_ਪੰਕਤੀ>0 );
    ਜੇ( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      r_ਲਾਗਤ_Idx = pNew->n_ਬਾਹਰਲਾ + 16;
    }ਨਹੀਂਤਾਂ{
      r_ਲਾਗਤ_Idx = pNew->n_ਬਾਹਰਲਾ + 1 + (15*pProbe->sz_Idx_ਪੰਕਤੀ)/pSrc->pSTab->sz_Tab_ਪੰਕਤੀ;
    }
    r_ਲਾਗਤ_Idx = sqlite3_Log_Est_ਜੋੜੋ(r_Log_ਮਾਪ, r_ਲਾਗਤ_Idx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->r_ਲੜੀਵਾਰ = r_ਲਾਗਤ_Idx;
    ਜੇ( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->r_ਲੜੀਵਾਰ = sqlite3_Log_Est_ਜੋੜੋ(pNew->r_ਲੜੀਵਾਰ, pNew->n_ਬਾਹਰਲਾ + 16);
    }
    Apply_ਲਾਗਤ_Multiplier(pNew->r_ਲੜੀਵਾਰ, pProbe->p_ਸਾਰਣੀ->ਲਾਗਤ_Mult);

    n_ਬਾਹਰਲਾ_Unadjusted = pNew->n_ਬਾਹਰਲਾ;
    pNew->r_ਲੜੀਵਾਰ += n_ਵਿੱਚ_Mul + n_ਵਿੱਚ;
    pNew->n_ਬਾਹਰਲਾ += n_ਵਿੱਚ_Mul + n_ਵਿੱਚ;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_ਮਾਪ);
    ਜੇ( pSrc->fg.fromExists ) pNew->n_ਬਾਹਰਲਾ = 0;
    rc = where_Loop_ਪਾਓ(pBuilder, pNew);

    ਜੇ( pNew->wsFlags & WHERE_ਕਾਲਮ_RANGE ){
      pNew->n_ਬਾਹਰਲਾ = saved_n_ਬਾਹਰਲਾ;
    }ਨਹੀਂਤਾਂ{
      pNew->n_ਬਾਹਰਲਾ = n_ਬਾਹਰਲਾ_Unadjusted;
    }

    ਜੇ( (pNew->wsFlags & WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ)==0
     && pNew->u.btree.nEq<pProbe->n_ਕਾਲਮ
     && (pNew->u.btree.nEq<pProbe->n_ਕੁੰਜੀ_ਖਾਨਾ ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      ਜੇ( pNew->u.btree.nEq>3 ){
        sqlite3_ਤਰੱਕੀ_Check(pParse);
      }
      where_Loop_ਜੋੜੋ_Btree_ਸੂਚਕਾਂਕ(pBuilder, pSrc, pProbe, n_ਵਿੱਚ_Mul+n_ਵਿੱਚ);
    }
    pNew->n_ਬਾਹਰਲਾ = saved_n_ਬਾਹਰਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#ਜੇ_ਅੰਤ
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_ਸਿਖਰ = saved_n_ਸਿਖਰ;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->n_ਬਾਹਰਲਾ = saved_n_ਬਾਹਰਲਾ;
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
  ਪ੍ਰਤਿਜ੍ਞਾ( 42==sqlite3LogEst(18) );
  ਜੇ( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_ਕੁੰਜੀ_ਖਾਨਾ
   && saved_nEq==pNew->nLTerm
   && pProbe->no_Skip_ਜਾਂਚ==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_Skip_ਜਾਂਚ)
   && pProbe->ai_ਪੰਕਤੀ_Log_Est[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->ai_ਪੰਕਤੀ_Log_Est[saved_nEq] - pProbe->ai_ਪੰਕਤੀ_Log_Est[saved_nEq+1];
    pNew->n_ਬਾਹਰਲਾ -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    where_Loop_ਜੋੜੋ_Btree_ਸੂਚਕਾਂਕ(pBuilder, pSrc, pProbe, nIter + n_ਵਿੱਚ_Mul);
    pNew->n_ਬਾਹਰਲਾ = saved_n_ਬਾਹਰਲਾ;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->p_ਸਾਰਣੀ->zName, pProbe->zName, saved_nEq, rc));
  ਵਾਪਸ rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
ਸਥਿਰ ਪੂਰਨ ਸੂਚਕਾਂਕ_Might_Help_With_ਕ੍ਰਮ_By(
  WhereLoopBuilder *pBuilder,
  ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ,
  ਪੂਰਨ iCursor
){
  ਵਿਅੰਜਕ_List *pOB;
  ਵਿਅੰਜਕ_List *a_ਖਾਨਾ_ਵਿਅੰਜਕ;
  ਪੂਰਨ ii, jj;

  ਜੇ( p_ਸੂਚਕਾਂਕ->bUnordered ) ਵਾਪਸ 0;
  ਜੇ( (pOB = pBuilder->pWInfo->p_ਕ੍ਰਮ_By)==0 ) ਵਾਪਸ 0;
  ਲਈ(ii=0; ii<pOB->n_ਵਿਅੰਜਕ; ii++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pOB->a[ii].p_ਵਿਅੰਜਕ);
    ਜੇ( NEVER(p_ਵਿਅੰਜਕ==0) ) ਜਾਰੀ;
    ਜੇ( (p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ || p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ) 
     && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==iCursor 
    ){
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ) ਵਾਪਸ 1;
      ਲਈ(jj=0; jj<p_ਸੂਚਕਾਂਕ->n_ਕੁੰਜੀ_ਖਾਨਾ; jj++){
        ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ==p_ਸੂਚਕਾਂਕ->ai_ਕਾਲਮ[jj] ) ਵਾਪਸ 1;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( (a_ਖਾਨਾ_ਵਿਅੰਜਕ = p_ਸੂਚਕਾਂਕ->a_ਖਾਨਾ_ਵਿਅੰਜਕ)!=0 ){
      ਲਈ(jj=0; jj<p_ਸੂਚਕਾਂਕ->n_ਕੁੰਜੀ_ਖਾਨਾ; jj++){
        ਜੇ( p_ਸੂਚਕਾਂਕ->ai_ਕਾਲਮ[jj]!=XN_ਵਿਅੰਜਕ ) ਜਾਰੀ;
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare_Skip(p_ਵਿਅੰਜਕ,a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[jj].p_ਵਿਅੰਜਕ,iCursor)==0 ){
          ਵਾਪਸ 1;
        }
      }
    }
  }
  ਵਾਪਸ 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
ਸਥਿਰ ਪੂਰਨ where_Usable_Partial_ਸੂਚਕਾਂਕ(
  ਪੂਰਨ iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  ਵਿਅੰਜਕ *pWhere          /* The WHERE clause from the partial index */
){
  ਪੂਰਨ i;
  Where_ਪਦ *p_ਪਦ;
  Parse *pParse;

  ਜੇ( jointype & JT_LTORJ ) ਵਾਪਸ 0;
  pParse = pWC->pWInfo->pParse;
  ਜਦੋਂ( pWhere->op==TK_AND ){
    ਜੇ( !where_Usable_Partial_ਸੂਚਕਾਂਕ(iTab,jointype,pWC,pWhere->p_ਖੱਬਾ) ) ਵਾਪਸ 0;
    pWhere = pWhere->p_ਸੱਜਾ;
  }
  ਲਈ(i=0, p_ਪਦ=pWC->a; i<pWC->n_ਪਦ; i++, p_ਪਦ++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
    p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
    ਜੇ( (!ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) || p_ਵਿਅੰਜਕ->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON))
     && sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(pParse, p_ਵਿਅੰਜਕ, pWhere, iTab)
     && !sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(pParse, p_ਵਿਅੰਜਕ, pWhere, -1)
     && (p_ਪਦ->wtFlags & ਪਦ_VNULL)==0
    ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Is_Covered_By_ਸੂਚਕਾਂਕ(
  ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,
  ਅਚਲ ਸੂਚਕਾਂਕ *pIdx,
  ਪੂਰਨ iTabCur
){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<pIdx->n_ਕਾਲਮ; i++){
    ਜੇ( pIdx->ai_ਕਾਲਮ[i]==XN_ਵਿਅੰਜਕ
     && sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਿਅੰਜਕ, pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ, iTabCur)==0
    ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Covering_ਸੂਚਕਾਂਕ_Check Covering_ਸੂਚਕਾਂਕ_Check;
ਬਣਤਰ Covering_ਸੂਚਕਾਂਕ_Check {
  ਸੂਚਕਾਂਕ *pIdx;       /* The index */
  ਪੂਰਨ iTabCur;       /* Cursor number for the corresponding table */
  u8 b_ਵਿਅੰਜਕ;          /* Uses an indexed expression */
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
ਸਥਿਰ ਪੂਰਨ where_Is_Covering_ਸੂਚਕਾਂਕ_Walk_Callback(Walker *pWalk, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ i;                    /* Loop counter */
  ਅਚਲ ਸੂਚਕਾਂਕ *pIdx;        /* The index of interest */
  ਅਚਲ i16 *ai_ਕਾਲਮ;      /* Columns contained in the index */
  u16 n_ਕਾਲਮ;              /* Number of columns in the index */
  Covering_ਸੂਚਕਾਂਕ_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  ਜੇ( (p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ || p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ!=pCk->iTabCur ) ਵਾਪਸ WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    ai_ਕਾਲਮ = pIdx->ai_ਕਾਲਮ;
    n_ਕਾਲਮ = pIdx->n_ਕਾਲਮ;
    ਲਈ(i=0; i<n_ਕਾਲਮ; i++){
      ਜੇ( ai_ਕਾਲਮ[i]==p_ਵਿਅੰਜਕ->i_ਕਾਲਮ ) ਵਾਪਸ WRC_Continue;
    }
    pCk->bUnidx = 1;
    ਵਾਪਸ WRC_Abort;
  }ਨਹੀਂਤਾਂ ਜੇ( pIdx->b_Has_ਵਿਅੰਜਕ
         && ਵਿਅੰਜਕ_Is_Covered_By_ਸੂਚਕਾਂਕ(p_ਵਿਅੰਜਕ, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_ਵਿਅੰਜਕ = 1;
    ਵਾਪਸ WRC_Prune;
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ SQLITE_NOINLINE u32 where_Is_Covering_ਸੂਚਕਾਂਕ(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  ਸੂਚਕਾਂਕ *pIdx,           /* Index that is being tested */
  ਪੂਰਨ iTabCur            /* Cursor for the table being indexed */
){
  ਪੂਰਨ i, rc;
  ਬਣਤਰ Covering_ਸੂਚਕਾਂਕ_Check ck;
  Walker w;
  ਜੇ( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    ਵਾਪਸ 0;
  }
  ਜੇ( pIdx->b_Has_ਵਿਅੰਜਕ==0 ){
    ਲਈ(i=0; i<pIdx->n_ਕਾਲਮ; i++){
      ਜੇ( pIdx->ai_ਕਾਲਮ[i]>=BMS-1 ) ਰੋਕੋ;
    }
    ਜੇ( i>=pIdx->n_ਕਾਲਮ ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      ਵਾਪਸ 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_ਵਿਅੰਜਕ = 0;
  ck.bUnidx = 0;
  memset(&w, 0, ਆਕਾਰ(w));
  w.x_ਵਿਅੰਜਕ_Callback = where_Is_Covering_ਸੂਚਕਾਂਕ_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  ਜੇ( ck.bUnidx ){
    rc = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( ck.b_ਵਿਅੰਜਕ ){
    rc = WHERE_EXPRIDX;
  }ਨਹੀਂਤਾਂ{
    rc = WHERE_IDX_ONLY;
  }
  ਵਾਪਸ rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
ਸਥਿਰ ਖਾਲੀ where_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Cleanup(sqlite3 *db, ਖਾਲੀ *pObject){
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ **pp = (ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ**)pObject;
  ਜਦੋਂ( *pp!=0 ){
    ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p = *pp;
    *pp = p->pIENext;
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p->p_ਵਿਅੰਜਕ);
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
ਸਥਿਰ ਖਾਲੀ where_Part_Idx_ਵਿਅੰਜਕ(
  Parse *pParse,                  /* Parse context */
  ਸੂਚਕਾਂਕ *pIdx,                    /* Partial index being processed */
  ਵਿਅੰਜਕ *pPart,                    /* WHERE clause being processed */
  Bitmask *p_ਨਕਾਬ,                 /* Mask to clear bits in */
  ਪੂਰਨ iIdxCur,                    /* Cursor number for index */
  Src_ਵਸਤੂ *p_ਵਸਤੂ                  /* The FROM clause entry for the table */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ==0 || (p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਵਸਤੂ==0 || p_ਨਕਾਬ==0) && (p_ਨਕਾਬ!=0 || p_ਵਸਤੂ!=0) );

  ਜੇ( pPart->op==TK_AND ){
    where_Part_Idx_ਵਿਅੰਜਕ(pParse, pIdx, pPart->p_ਸੱਜਾ, p_ਨਕਾਬ, iIdxCur, p_ਵਸਤੂ);
    pPart = pPart->p_ਖੱਬਾ;
  }

  ਜੇ( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = pPart->p_ਖੱਬਾ;
    ਵਿਅੰਜਕ *p_ਸੱਜਾ = pPart->p_ਸੱਜਾ;
    ਸਾਰਣੀ *p_ਖੱਬਾ_Tab;            /* Table of the left operand */
    ਕਾਲਮ *p_ਖੱਬਾ_ਖਾਨਾ;           /* Column of the left operand */
    u8 aff_ਸੱਜਾ, aff_ਖੱਬਾ;

    ਜੇ( p_ਖੱਬਾ->op!=TK_ਕਾਲਮ ) ਵਾਪਸ;
    ਜੇ( !sqlite3_ਵਿਅੰਜਕ_Is_Constant(0, p_ਸੱਜਾ) ) ਵਾਪਸ;
    ਜੇ( !sqlite3_Is_ਦੁਆਧਾਰੀ(sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pPart)) ) ਵਾਪਸ;
    ਜੇ( p_ਖੱਬਾ->i_ਕਾਲਮ<0 ) ਵਾਪਸ;
    p_ਖੱਬਾ_Tab = pIdx->p_ਸਾਰਣੀ;
    p_ਖੱਬਾ_ਖਾਨਾ = &p_ਖੱਬਾ_Tab->a_ਖਾਨਾ[p_ਖੱਬਾ->i_ਕਾਲਮ];
    aff_ਖੱਬਾ = p_ਖੱਬਾ_ਖਾਨਾ->affinity;
    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_AFF_BLOB<SQLITE_AFF_ਪਾਠ );
    ਜੇ( aff_ਖੱਬਾ<SQLITE_AFF_ਪਾਠ ) ਵਾਪਸ;
    aff_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਸੱਜਾ);
    ਜੇ( aff_ਸੱਜਾ!=0 && aff_ਸੱਜਾ!=aff_ਖੱਬਾ ) ਵਾਪਸ;
    ਜੇ( p_ਵਸਤੂ ){
      sqlite3 *db = pParse->db;
      ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p = (ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ*)sqlite3DbMallocRaw(db, ਆਕਾਰ(*p));
      ਜੇ( p ){
        ਪੂਰਨ b_Null_ਪੰਕਤੀ = (p_ਵਸਤੂ->fg.jointype&(JT_ਖੱਬਾ|JT_LTORJ))!=0;
        p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਸੱਜਾ, 0);
        ਜੇ( (p_ਖੱਬਾ_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3_ਕਾਲਮ_Type(p_ਖੱਬਾ_ਖਾਨਾ, 0);
          ਪ੍ਰਤਿਜ੍ਞਾ( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(t.z);
          p->p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_CAST, &t, 1);
          sqlite3_ਵਿਅੰਜਕ_Attach_Subtrees(db, p->p_ਵਿਅੰਜਕ, p_ਸੱਜਾ, 0);
        }ਨਹੀਂਤਾਂ{
          p->p_ਵਿਅੰਜਕ = p_ਸੱਜਾ;
        }
        p->i_ਅੰਕੜੇ_Cur = p_ਵਸਤੂ->iCursor;
        p->iIdxCur = iIdxCur;
        p->i_Idx_ਖਾਨਾ = p_ਖੱਬਾ->i_ਕਾਲਮ;
        p->b_Maybe_Null_ਪੰਕਤੀ = b_Null_ਪੰਕਤੀ;
        p->pIENext = pParse->p_Idx_Part_ਵਿਅੰਜਕ;
        p->aff = aff_ਖੱਬਾ;
        pParse->p_Idx_Part_ਵਿਅੰਜਕ = p;
        ਜੇ( p->pIENext==0 ){
          ਖਾਲੀ *pArg = (ਖਾਲੀ*)&pParse->p_Idx_Part_ਵਿਅੰਜਕ;
          sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, where_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Cleanup, pArg);
        }
      }
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਖੱਬਾ->i_ਕਾਲਮ<(BMS-1) ){
      *p_ਨਕਾਬ &= ~((Bitmask)1 << p_ਖੱਬਾ->i_ਕਾਲਮ);
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
ਸਥਿਰ ਪੂਰਨ where_Loop_ਜੋੜੋ_Btree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  ਸੂਚਕਾਂਕ *pProbe;              /* An index we are evaluating */
  ਸੂਚਕਾਂਕ sPk;                  /* A fake index object for the primary key */
  LogEst ai_ਪੰਕਤੀ_Est_Pk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 ai_ਕਾਲਮ_Pk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  Src_ਵਸਤੂ *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  ਪੂਰਨ rc = SQLITE_OK;         /* Return code */
  ਪੂਰਨ i_ਛਾਂਟੋ_Idx;               /* Index number */
  ਪੂਰਨ b;                      /* A boolean value */
  LogEst r_ਮਾਪ;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  ਸਾਰਣੀ *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsVirtual(pSrc->pSTab) );

  ਜੇ( pSrc->fg.is_ਸੂਚਕਾਂਕਿਤ_By ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }ਨਹੀਂਤਾਂ ਜੇ( !HasRowid(pTab) ){
    ਜੇ( pSrc->fg.not_ਸੂਚਕਾਂਕਿਤ ){
      pProbe = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
    }ਨਹੀਂਤਾਂ{
      pProbe = pTab->p_ਸੂਚਕਾਂਕ;
    }
  }ਨਹੀਂਤਾਂ{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    ਸੂਚਕਾਂਕ *p_ਪਹਿਲਾ;                  /* First of real indices on the table */
    memset(&sPk, 0, ਆਕਾਰ(ਸੂਚਕਾਂਕ));
    sPk.n_ਕੁੰਜੀ_ਖਾਨਾ = 1;
    sPk.n_ਕਾਲਮ = 1;
    sPk.ai_ਕਾਲਮ = &ai_ਕਾਲਮ_Pk;
    sPk.ai_ਪੰਕਤੀ_Log_Est = ai_ਪੰਕਤੀ_Est_Pk;
    sPk.onError = OE_Replace;
    sPk.p_ਸਾਰਣੀ = pTab;
    sPk.sz_Idx_ਪੰਕਤੀ = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    ai_ਪੰਕਤੀ_Est_Pk[0] = pTab->n_ਪੰਕਤੀ_Log_Est;
    ai_ਪੰਕਤੀ_Est_Pk[1] = 0;
    p_ਪਹਿਲਾ = pSrc->pSTab->p_ਸੂਚਕਾਂਕ;
    sPk.p_ਅਗਲਾ = p_ਪਹਿਲਾ;
    pProbe = &sPk;
  }
  r_ਮਾਪ = pTab->n_ਪੰਕਤੀ_Log_Est;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOMATIC_ਸੂਚਕਾਂਕ
  /* Automatic indexes */
  ਜੇ( !pBuilder->p_Or_ਸੈੱਟ      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_ਸੱਜਾ_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_ਸੂਚਕਾਂਕ)!=0
   && !pSrc->fg.is_ਸੂਚਕਾਂਕਿਤ_By  /* Has no INDEXED BY clause */
   && !pSrc->fg.not_ਸੂਚਕਾਂਕਿਤ   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_ਸੱਜਾ)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_ਮਾਪ;         /* Logarithm of the number of rows in the table */
    Where_ਪਦ *p_ਪਦ;
    Where_ਪਦ *pWCEnd = pWC->a + pWC->n_ਪਦ;
    r_Log_ਮਾਪ = estLog(r_ਮਾਪ);
    ਲਈ(p_ਪਦ=pWC->a; rc==SQLITE_OK && p_ਪਦ<pWCEnd; p_ਪਦ++){
      ਜੇ( p_ਪਦ->prereq_ਸੱਜਾ & pNew->ਨਕਾਬ_Self ) ਜਾਰੀ;
      ਜੇ( ਪਦ_Can_Drive_ਸੂਚਕਾਂਕ(p_ਪਦ, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_ਸੂਚਕਾਂਕ = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = p_ਪਦ;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->r_ਤਿਆਰੀ = r_Log_ਮਾਪ + r_ਮਾਪ;
        ਜੇ( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->r_ਤਿਆਰੀ += 28;
        }ਨਹੀਂਤਾਂ{
          pNew->r_ਤਿਆਰੀ -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        Apply_ਲਾਗਤ_Multiplier(pNew->r_ਤਿਆਰੀ, pTab->ਲਾਗਤ_Mult);
        ਜੇ( pNew->r_ਤਿਆਰੀ<0 ) pNew->r_ਤਿਆਰੀ = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->n_ਬਾਹਰਲਾ = 43;  ਪ੍ਰਤਿਜ੍ਞਾ( 43==sqlite3LogEst(20) );
        pNew->r_ਲੜੀਵਾਰ = sqlite3_Log_Est_ਜੋੜੋ(r_Log_ਮਾਪ,pNew->n_ਬਾਹਰਲਾ);
        pNew->wsFlags = WHERE_AUTO_ਸੂਚਕਾਂਕ;
        pNew->prereq = mPrereq | p_ਪਦ->prereq_ਸੱਜਾ;
        rc = where_Loop_ਪਾਓ(pBuilder, pNew);
      }
    }
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  ਲਈ(i_ਛਾਂਟੋ_Idx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.is_ਸੂਚਕਾਂਕਿਤ_By||pSrc->fg.not_ਸੂਚਕਾਂਕਿਤ)?0:pProbe->p_ਅਗਲਾ,
         i_ਛਾਂਟੋ_Idx++
  ){
    ਜੇ( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_ਸੂਚਕਾਂਕ(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      ਜਾਰੀ;  /* Partial index inappropriate for this query */
    }
    ਜੇ( pProbe->bNoQuery ) ਜਾਰੀ;
    r_ਮਾਪ = pProbe->ai_ਪੰਕਤੀ_Log_Est[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_ਸਿਖਰ = 0;
    pNew->u.btree.n_Distinct_ਖਾਨਾ = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->i_ਛਾਂਟੋ_Idx = 0;
    pNew->r_ਤਿਆਰੀ = 0;
    pNew->prereq = mPrereq;
    pNew->n_ਬਾਹਰਲਾ = r_ਮਾਪ;
    pNew->u.btree.p_ਸੂਚਕਾਂਕ = pProbe;
    pNew->u.btree.p_ਕ੍ਰਮ_By = 0;
    b = ਸੂਚਕਾਂਕ_Might_Help_With_ਕ੍ਰਮ_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    ਪ੍ਰਤਿਜ੍ਞਾ( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    ਜੇ( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->i_ਛਾਂਟੋ_Idx = b ? i_ਛਾਂਟੋ_Idx : 0;
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
      pNew->r_ਲੜੀਵਾਰ = r_ਮਾਪ + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#ਨਹੀਂਤਾਂ
      pNew->r_ਲੜੀਵਾਰ = r_ਮਾਪ + 16;
#ਜੇ_ਅੰਤ
      Apply_ਲਾਗਤ_Multiplier(pNew->r_ਲੜੀਵਾਰ, pTab->ਲਾਗਤ_Mult);
      whereLoopOutputAdjust(pWC, pNew, r_ਮਾਪ);
      ਜੇ( pSrc->fg.isSubquery ){
        ਜੇ( pSrc->fg.ਰਾਹੀਂ_Coroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        ਜੇ( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.p_ਕ੍ਰਮ_By = pSrc->u4.pSubq->pSelect->p_ਕ੍ਰਮ_By;
        }
      }ਨਹੀਂਤਾਂ ਜੇ( pSrc->fg.fromExists ){
        pNew->n_ਬਾਹਰਲਾ = 0;
      }
      rc = where_Loop_ਪਾਓ(pBuilder, pNew);
      pNew->n_ਬਾਹਰਲਾ = r_ਮਾਪ;
      ਜੇ( rc ) ਰੋਕੋ;
    }ਨਹੀਂਤਾਂ{
      Bitmask m;
      ਜੇ( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_ਸੂਚਕਾਂਕਿਤ;
      }ਨਹੀਂਤਾਂ{
        m = pSrc->ਖਾਨਾ_ਵਰਤਿਆ & pProbe->ਖਾਨਾ_Not_Idxed;
        ਜੇ( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_ਵਿਅੰਜਕ(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_ਸੂਚਕਾਂਕਿਤ;
        ਜੇ( m==TOPBIT || (pProbe->b_Has_ਵਿਅੰਜਕ && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_ਸੂਚਕਾਂਕ(pWInfo, pProbe, pSrc->iCursor);
          ਜੇ( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            ਪ੍ਰਤਿਜ੍ਞਾ( m!=0 );
          }ਨਹੀਂਤਾਂ{
            m = 0;
            pNew->wsFlags |= isCov;
            ਜੇ( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }ਨਹੀਂਤਾਂ{
              ਪ੍ਰਤਿਜ੍ਞਾ( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }ਨਹੀਂਤਾਂ ਜੇ( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_ਸੂਚਕਾਂਕਿਤ;
        }
      }

      /* Full scan via index */
      ਜੇ( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.is_ਸੂਚਕਾਂਕਿਤ_By
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->sz_Idx_ਪੰਕਤੀ<pTab->sz_Tab_ਪੰਕਤੀ)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_Cover_Idx_ਜਾਂਚ)
          )
      ){
        pNew->i_ਛਾਂਟੋ_Idx = b ? i_ਛਾਂਟੋ_Idx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->r_ਲੜੀਵਾਰ = r_ਮਾਪ + 1 + (15*pProbe->sz_Idx_ਪੰਕਤੀ)/pTab->sz_Tab_ਪੰਕਤੀ;
        ਜੇ( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_ਮਾਪ + 16;  /* Base cost:  N*3 */
          ਪੂਰਨ ii;
          ਪੂਰਨ iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          ਲਈ(ii=0; ii<pWC2->n_ਪਦ; ii++){
            Where_ਪਦ *p_ਪਦ = &pWC2->a[ii];
            ਜੇ( !sqlite3_ਵਿਅੰਜਕ_Covered_By_ਸੂਚਕਾਂਕ(p_ਪਦ->p_ਵਿਅੰਜਕ, iCur, pProbe) ){
              ਰੋਕੋ;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            ਜੇ( p_ਪਦ->truthProb<=0 ){
              nLookup += p_ਪਦ->truthProb;
            }ਨਹੀਂਤਾਂ{
              nLookup--;
              ਜੇ( p_ਪਦ->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->r_ਲੜੀਵਾਰ = sqlite3_Log_Est_ਜੋੜੋ(pNew->r_ਲੜੀਵਾਰ, nLookup);
        }
        Apply_ਲਾਗਤ_Multiplier(pNew->r_ਲੜੀਵਾਰ, pTab->ਲਾਗਤ_Mult);
        whereLoopOutputAdjust(pWC, pNew, r_ਮਾਪ);
        ਜੇ( (pSrc->fg.jointype & JT_ਸੱਜਾ)!=0 && pProbe->a_ਖਾਨਾ_ਵਿਅੰਜਕ ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }ਨਹੀਂਤਾਂ{
          ਜੇ( pSrc->fg.fromExists ) pNew->n_ਬਾਹਰਲਾ = 0;
          rc = where_Loop_ਪਾਓ(pBuilder, pNew);
        }
        pNew->n_ਬਾਹਰਲਾ = r_ਮਾਪ;
        ਜੇ( rc ) ਰੋਕੋ;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_ਜੋੜੋ_Btree_ਸੂਚਕਾਂਕ(pBuilder, pSrc, pProbe, 0);
    ਜੇ( pBuilder->bldFlags1==SQLITE_BLDF1_ਸੂਚਕਾਂਕਿਤ ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ rc;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
ਸਥਿਰ ਪੂਰਨ is_ਹੱਦਬੰਦੀ_ਪਦ(Where_ਪਦ *p_ਪਦ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->eOperator==WO_AUX || p_ਪਦ->e_ਮੇਲ_Op==0 );
  ਵਾਪਸ p_ਪਦ->e_ਮੇਲ_Op>=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਹੱਦਬੰਦੀ
      && p_ਪਦ->e_ਮੇਲ_Op<=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਵਿੱਥ;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
ਸਥਿਰ ਪੂਰਨ all_Constraints_ਵਰਤਿਆ(
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *aUsage, 
  ਪੂਰਨ nCons
){
  ਪੂਰਨ ii;
  ਲਈ(ii=0; ii<nCons; ii++){
    ਜੇ( aUsage[ii].argv_ਸੂਚਕਾਂਕ<=0 ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ where_Loop_ਜੋੜੋ_Virtual_One(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  ਪੂਰਨ *pb_ਵਿੱਚ,                      /* OUT: True if plan uses an IN(...) op */
  ਪੂਰਨ *pb_Retry_ਹੱਦਬੰਦੀ               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_ਸੂਚਕਾਂਕ_Info *pHidden = (Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *pIdxCons;
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  ਪੂਰਨ i;
  ਪੂਰਨ mx_ਪਦ;
  ਪੂਰਨ rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  Src_ਵਸਤੂ *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  ਪੂਰਨ nConstraint = pIdxInfo->nConstraint;

  ਪ੍ਰਤਿਜ੍ਞਾ( (mUsable & mPrereq)==mPrereq );
  *pb_ਵਿੱਚ = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint**)&pIdxInfo->aConstraint;
  ਲਈ(i=0; i<nConstraint; i++, pIdxCons++){
    Where_ਪਦ *p_ਪਦ = ਪਦ_From_Where_Clause(pWC, pIdxCons->i_ਪਦ_ਵਿੱਥ);
    pIdxCons->usable = 0;
    ਜੇ( (p_ਪਦ->prereq_ਸੱਜਾ & mUsable)==p_ਪਦ->prereq_ਸੱਜਾ
     && (p_ਪਦ->eOperator & mExclude)==0
     && (pb_Retry_ਹੱਦਬੰਦੀ || !is_ਹੱਦਬੰਦੀ_ਪਦ(p_ਪਦ))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  memset(pUsage, 0, ਆਕਾਰ(pUsage[0])*nConstraint);
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_ਅੰਕ = 0;
  pIdxInfo->ਕ੍ਰਮ_By_Consumed = 0;
  pIdxInfo->estimated_ਲਾਗਤ = SQLITE_BIG_DBL / (ਦੁੱਗਣਾ)2;
  pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->m_Handle_ਵਿੱਚ = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_ਉੱਤਮ_ਸੂਚਕਾਂਕ(pParse, pSrc->pSTab, pIdxInfo);
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      ਵਾਪਸ SQLITE_OK;
    }
    ਵਾਪਸ rc;
  }

  mx_ਪਦ = -1;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->nLSlot>=nConstraint );
  memset(pNew->aLTerm, 0, ਆਕਾਰ(pNew->aLTerm[0])*nConstraint );
  memset(&pNew->u.vtab, 0, ਆਕਾਰ(pNew->u.vtab));
  pIdxCons = *(ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint**)&pIdxInfo->aConstraint;
  ਲਈ(i=0; i<nConstraint; i++, pIdxCons++){
    ਪੂਰਨ i_ਪਦ;
    ਜੇ( (i_ਪਦ = pUsage[i].argv_ਸੂਚਕਾਂਕ - 1)>=0 ){
      Where_ਪਦ *p_ਪਦ;
      ਪੂਰਨ j = pIdxCons->i_ਪਦ_ਵਿੱਥ;
      ਜੇ( i_ਪਦ>=nConstraint
       || j<0
       || (p_ਪਦ = ਪਦ_From_Where_Clause(pWC, j))==0
       || pNew->aLTerm[i_ਪਦ]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        ਵਾਪਸ SQLITE_ERROR;
      }
      testcase( i_ਪਦ==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->n_ਪਦ-1 );
      pNew->prereq |= p_ਪਦ->prereq_ਸੱਜਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਪਦ<pNew->nLSlot );
      pNew->aLTerm[i_ਪਦ] = p_ਪਦ;
      ਜੇ( i_ਪਦ>mx_ਪਦ ) mx_ਪਦ = i_ਪਦ;
      testcase( i_ਪਦ==15 );
      testcase( i_ਪਦ==16 );
      ਜੇ( pUsage[i].omit ){
        ਜੇ( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=i_ਪਦ );
          pNew->u.vtab.omit_ਨਕਾਬ |= 1<<i_ਪਦ;
        }ਨਹੀਂਤਾਂ{
          testcase( i!=i_ਪਦ );
        }
        ਜੇ( p_ਪਦ->e_ਮੇਲ_Op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਵਿੱਥ ){
          pNew->u.vtab.b_Omit_ਵਿੱਥ = 1;
        }
      }
      ਜੇ( SMASKBIT32(i) & pHidden->m_Handle_ਵਿੱਚ ){
        pNew->u.vtab.m_Handle_ਵਿੱਚ |= MASKBIT32(i_ਪਦ);
      }ਨਹੀਂਤਾਂ ਜੇ( (p_ਪਦ->eOperator & WO_ਵਿੱਚ)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->ਕ੍ਰਮ_By_Consumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_ਸੂਚਕਾਂਕ_ਜਾਂਚ_UNIQUE;
        *pb_ਵਿੱਚ = 1; ਪ੍ਰਤਿਜ੍ਞਾ( (mExclude & WO_ਵਿੱਚ)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      ਪ੍ਰਤਿਜ੍ਞਾ( pb_Retry_ਹੱਦਬੰਦੀ || !is_ਹੱਦਬੰਦੀ_ਪਦ(p_ਪਦ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( !is_ਹੱਦਬੰਦੀ_ਪਦ(p_ਪਦ) || i>=nConstraint-2 );
      ਪ੍ਰਤਿਜ੍ਞਾ( !is_ਹੱਦਬੰਦੀ_ਪਦ(p_ਪਦ) || i==nConstraint-1 || is_ਹੱਦਬੰਦੀ_ਪਦ(p_ਪਦ+1) );

      ਜੇ( is_ਹੱਦਬੰਦੀ_ਪਦ(p_ਪਦ) && (*pb_ਵਿੱਚ || !all_Constraints_ਵਰਤਿਆ(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pb_Retry_ਹੱਦਬੰਦੀ = 1;
        ਵਾਪਸ SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mx_ਪਦ+1;
  ਲਈ(i=0; i<=mx_ਪਦ; i++){
    ਜੇ( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_ਸੁਨੇਹਾ(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      ਵਾਪਸ SQLITE_ERROR;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_ਅੰਕ = pIdxInfo->idx_ਅੰਕ;
  pNew->u.vtab.needFree = pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str;
  pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->ਕ੍ਰਮ_By_Consumed ?
      pIdxInfo->n_ਕ੍ਰਮ_By : 0);
  pNew->u.vtab.b_Idx_ਅੰਕ_Hex = (pIdxInfo->idxFlags&SQLITE_ਸੂਚਕਾਂਕ_ਜਾਂਚ_HEX)!=0;
  pNew->r_ਤਿਆਰੀ = 0;
  pNew->r_ਲੜੀਵਾਰ = sqlite3LogEstFromDouble(pIdxInfo->estimated_ਲਾਗਤ);
  pNew->n_ਬਾਹਰਲਾ = sqlite3LogEst(pIdxInfo->estimated_ਪੰਕਤੀਆਂ);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  ਜੇ( pIdxInfo->idxFlags & SQLITE_ਸੂਚਕਾਂਕ_ਜਾਂਚ_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }ਨਹੀਂਤਾਂ{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_ਪਾਓ(pBuilder, pNew);
  ਜੇ( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pb_ਵਿੱਚ, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  ਵਾਪਸ rc;
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
ਅਚਲ ਅੱਖਰ *sqlite3_vtab_collation(sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo, ਪੂਰਨ iCons){
  Hidden_ਸੂਚਕਾਂਕ_Info *pHidden = (Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  ਅਚਲ ਅੱਖਰ *zRet = 0;
  ਜੇ( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    ਪੂਰਨ i_ਪਦ = pIdxInfo->aConstraint[iCons].i_ਪਦ_ਵਿੱਥ;
    ਵਿਅੰਜਕ *pX = ਪਦ_From_Where_Clause(pHidden->pWC, i_ਪਦ)->p_ਵਿਅੰਜਕ;
    ਜੇ( pX->p_ਖੱਬਾ ){
      pC = sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_ਦੁਆਧਾਰੀ);
  }
  ਵਾਪਸ zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
ਪੂਰਨ sqlite3_vtab_ਵਿੱਚ(sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo, ਪੂਰਨ iCons, ਪੂਰਨ bHandle){
  Hidden_ਸੂਚਕਾਂਕ_Info *pHidden = (Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  ਜੇ( m & pHidden->m_ਵਿੱਚ ){
    ਜੇ( bHandle==0 ){
      pHidden->m_Handle_ਵਿੱਚ &= ~m;
    }ਨਹੀਂਤਾਂ ਜੇ( bHandle>0 ){
      pHidden->m_Handle_ਵਿੱਚ |= m;
    }
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
ਪੂਰਨ sqlite3_vtab_rhs_ਮੁੱਲ(
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  ਪੂਰਨ iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_ਮੁੱਲ **ppVal           /* Write value extracted here */
){
  Hidden_ਸੂਚਕਾਂਕ_Info *pH = (Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  sqlite3_ਮੁੱਲ *pVal = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pH->aRhs[iCons]==0 ){
      Where_ਪਦ *p_ਪਦ = ਪਦ_From_Where_Clause(
          pH->pWC, pIdxInfo->aConstraint[iCons].i_ਪਦ_ਵਿੱਥ
      );
      rc = sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(
          pH->pParse->db, p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  ਜੇ( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  ਵਾਪਸ rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
ਪੂਰਨ sqlite3_vtab_distinct(sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  Hidden_ਸੂਚਕਾਂਕ_Info *pHidden = (Hidden_ਸੂਚਕਾਂਕ_Info*)&pIdxInfo[1];
  ਪ੍ਰਤਿਜ੍ਞਾ( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  ਵਾਪਸ pHidden->eDistinct;
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
ਖਾਲੀ sqlite3_Vtab_ਵਰਤਦਾ_All_Schemas(Parse *pParse){
  ਪੂਰਨ nDb = pParse->db->nDb;
  ਪੂਰਨ i;
  ਲਈ(i=0; i<nDb; i++){
    sqlite3_ਕੋਡ_Verify_Schema(pParse, i);
  }
  ਜੇ( Db_ਨਕਾਬ_Non_Zero(pParse->write_ਨਕਾਬ) ){
    ਲਈ(i=0; i<nDb; i++){
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
ਸਥਿਰ ਪੂਰਨ where_Loop_ਜੋੜੋ_Virtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  ਪੂਰਨ rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  Src_ਵਸਤੂ *pSrc;               /* The FROM clause term to search */
  sqlite3_ਸੂਚਕਾਂਕ_info *p;       /* Object to pass to xBestIndex() */
  ਪੂਰਨ nConstraint;             /* Number of constraints in p */
  ਪੂਰਨ b_ਵਿੱਚ;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask m_ਉੱਤਮ;               /* Tables used by best possible plan */
  u16 mNoOmit;
  ਪੂਰਨ bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  ਪ੍ਰਤਿਜ੍ਞਾ( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pSrc->pSTab) );
  p = allocate_ਸੂਚਕਾਂਕ_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  pNew->r_ਤਿਆਰੀ = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  ਜੇ( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_ਸੂਚਕਾਂਕ_Info(pParse->db, p);
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = where_Loop_ਜੋੜੋ_Virtual_One(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_ਵਿੱਚ, &bRetry
  );
  ਜੇ( bRetry ){
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );
    rc = where_Loop_ਜੋੜੋ_Virtual_One(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_ਵਿੱਚ, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  ਜੇ( rc==SQLITE_OK && ((m_ਉੱਤਮ = (pNew->prereq & ~mPrereq))!=0 || b_ਵਿੱਚ) ){
    ਪੂਰਨ ਵੇਖਿਆ_Zero = 0;             /* True if a plan with no prereqs seen */
    ਪੂਰਨ ਵੇਖਿਆ_Zero_No_ਵਿੱਚ = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask mPrev = 0;
    Bitmask m_ਉੱਤਮ_No_ਵਿੱਚ = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    ਜੇ( b_ਵਿੱਚ ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = where_Loop_ਜੋੜੋ_Virtual_One(
          pBuilder, mPrereq, ALLBITS, WO_ਵਿੱਚ, p, mNoOmit, &b_ਵਿੱਚ, 0);
      ਪ੍ਰਤਿਜ੍ਞਾ( b_ਵਿੱਚ==0 );
      m_ਉੱਤਮ_No_ਵਿੱਚ = pNew->prereq & ~mPrereq;
      ਜੇ( m_ਉੱਤਮ_No_ਵਿੱਚ==0 ){
        ਵੇਖਿਆ_Zero = 1;
        ਵੇਖਿਆ_Zero_No_ਵਿੱਚ = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    ਜਦੋਂ( rc==SQLITE_OK ){
      ਪੂਰਨ i;
      Bitmask m_ਅਗਲਾ = ALLBITS;
      ਪ੍ਰਤਿਜ੍ਞਾ( m_ਅਗਲਾ>0 );
      ਲਈ(i=0; i<nConstraint; i++){
        ਪੂਰਨ i_ਪਦ = p->aConstraint[i].i_ਪਦ_ਵਿੱਥ;
        Bitmask mThis = ਪਦ_From_Where_Clause(pWC, i_ਪਦ)->prereq_ਸੱਜਾ & ~mPrereq;
        ਜੇ( mThis>mPrev && mThis<m_ਅਗਲਾ ) m_ਅਗਲਾ = mThis;
      }
      mPrev = m_ਅਗਲਾ;
      ਜੇ( m_ਅਗਲਾ==ALLBITS ) ਰੋਕੋ;
      ਜੇ( m_ਅਗਲਾ==m_ਉੱਤਮ || m_ਅਗਲਾ==m_ਉੱਤਮ_No_ਵਿੱਚ ) ਜਾਰੀ;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)mPrev, (sqlite3_uint64)m_ਅਗਲਾ));
      rc = where_Loop_ਜੋੜੋ_Virtual_One(
          pBuilder, mPrereq, m_ਅਗਲਾ|mPrereq, 0, p, mNoOmit, &b_ਵਿੱਚ, 0);
      ਜੇ( pNew->prereq==mPrereq ){
        ਵੇਖਿਆ_Zero = 1;
        ਜੇ( b_ਵਿੱਚ==0 ) ਵੇਖਿਆ_Zero_No_ਵਿੱਚ = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    ਜੇ( rc==SQLITE_OK && ਵੇਖਿਆ_Zero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = where_Loop_ਜੋੜੋ_Virtual_One(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &b_ਵਿੱਚ, 0);
      ਜੇ( b_ਵਿੱਚ==0 ) ਵੇਖਿਆ_Zero_No_ਵਿੱਚ = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    ਜੇ( rc==SQLITE_OK && ਵੇਖਿਆ_Zero_No_ਵਿੱਚ==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = where_Loop_ਜੋੜੋ_Virtual_One(
          pBuilder, mPrereq, mPrereq, WO_ਵਿੱਚ, p, mNoOmit, &b_ਵਿੱਚ, 0);
    }
  }

  free_ਸੂਚਕਾਂਕ_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
ਸਥਿਰ ਪੂਰਨ where_Loop_ਜੋੜੋ_Or(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  Where_ਪਦ *p_ਪਦ, *pWCEnd;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iCur;
  WhereClause ਆਰਜ਼ੀ_WC;
  WhereLoopBuilder sSubBuild;
  Where_Or_ਸੈੱਟ s_ਜੋੜ, sCur;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->n_ਪਦ;
  pNew = pBuilder->pNew;
  memset(&s_ਜੋੜ, 0, ਆਕਾਰ(s_ਜੋੜ));
  p_ਵਸਤੂ = pWInfo->pTabList->a + pNew->iTab;
  iCur = p_ਵਸਤੂ->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ ) ਵਾਪਸ SQLITE_OK;

  ਲਈ(p_ਪਦ=pWC->a; p_ਪਦ<pWCEnd && rc==SQLITE_OK; p_ਪਦ++){
    ਜੇ( (p_ਪਦ->eOperator & WO_OR)!=0
     && (p_ਪਦ->u.pOrInfo->indexable & pNew->ਨਕਾਬ_Self)!=0
     && (!ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON)
          || p_ਪਦ->p_ਵਿਅੰਜਕ->w.iJoin==iCur)
    ){
      WhereClause * ਅਚਲ pOrWC = &p_ਪਦ->u.pOrInfo->wc;
      Where_ਪਦ * ਅਚਲ pOrWCEnd = &pOrWC->a[pOrWC->n_ਪਦ];
      Where_ਪਦ *p_Or_ਪਦ;
      ਪੂਰਨ once = 1;
      ਪੂਰਨ i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.p_Or_ਸੈੱਟ = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", p_ਪਦ));
      ਲਈ(p_Or_ਪਦ=pOrWC->a; p_Or_ਪਦ<pOrWCEnd; p_Or_ਪਦ++){
        ਜੇ( (p_Or_ਪਦ->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &p_Or_ਪਦ->u.pAndInfo->wc;
        }ਨਹੀਂਤਾਂ ਜੇ( p_Or_ਪਦ->ਖੱਬਾ_Cursor==iCur ){
          ਆਰਜ਼ੀ_WC.pWInfo = pWC->pWInfo;
          ਆਰਜ਼ੀ_WC.pOuter = pWC;
          ਆਰਜ਼ੀ_WC.op = TK_AND;
          ਆਰਜ਼ੀ_WC.n_ਪਦ = 1;
          ਆਰਜ਼ੀ_WC.n_ਅਧਾਰ = 1;
          ਆਰਜ਼ੀ_WC.a = p_Or_ਪਦ;
          sSubBuild.pWC = &ਆਰਜ਼ੀ_WC;
        }ਨਹੀਂਤਾਂ{
          ਜਾਰੀ;
        }
        sCur.n = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (ਪੂਰਨ)(p_Or_ਪਦ-pOrWC->a), p_ਪਦ, sSubBuild.pWC->n_ਪਦ));
        ਜੇ( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
        ਜੇ( IsVirtual(p_ਵਸਤੂ->pSTab) ){
          rc = where_Loop_ਜੋੜੋ_Virtual(&sSubBuild, mPrereq, mUnusable);
        }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
        {
          rc = where_Loop_ਜੋੜੋ_Btree(&sSubBuild, mPrereq);
        }
        ਜੇ( rc==SQLITE_OK ){
          rc = where_Loop_ਜੋੜੋ_Or(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_ਪੂਰਾ );
        ਜੇ( sCur.n==0 ){
          s_ਜੋੜ.n = 0;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( once ){
          whereOrMove(&s_ਜੋੜ, &sCur);
          once = 0;
        }ਨਹੀਂਤਾਂ{
          Where_Or_ਸੈੱਟ sPrev;
          whereOrMove(&sPrev, &s_ਜੋੜ);
          s_ਜੋੜ.n = 0;
          ਲਈ(i=0; i<sPrev.n; i++){
            ਲਈ(j=0; j<sCur.n; j++){
              where_Or_ਪਾਓ(&s_ਜੋੜ, sPrev.a[i].prereq | sCur.a[j].prereq,
                            sqlite3_Log_Est_ਜੋੜੋ(sPrev.a[i].r_ਲੜੀਵਾਰ, sCur.a[j].r_ਲੜੀਵਾਰ),
                            sqlite3_Log_Est_ਜੋੜੋ(sPrev.a[i].n_ਬਾਹਰਲਾ, sCur.a[j].n_ਬਾਹਰਲਾ));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = p_ਪਦ;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->r_ਤਿਆਰੀ = 0;
      pNew->i_ਛਾਂਟੋ_Idx = 0;
      memset(&pNew->u, 0, ਆਕਾਰ(pNew->u));
      ਲਈ(i=0; rc==SQLITE_OK && i<s_ਜੋੜ.n; i++){
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
        pNew->r_ਲੜੀਵਾਰ = s_ਜੋੜ.a[i].r_ਲੜੀਵਾਰ + 1;
        pNew->n_ਬਾਹਰਲਾ = s_ਜੋੜ.a[i].n_ਬਾਹਰਲਾ;
        pNew->prereq = s_ਜੋੜ.a[i].prereq;
        rc = where_Loop_ਪਾਓ(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", p_ਪਦ));
    }
  }
  ਵਾਪਸ rc;
}

/*
** Add all WhereLoop objects for all tables
*/
ਸਥਿਰ ਪੂਰਨ where_Loop_ਜੋੜੋ_All(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask m_ਪੂਰਵ = 0;
  ਪੂਰਨ iTab;
  SrcList *pTabList = pWInfo->pTabList;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  Src_ਵਸਤੂ *p_ਸਿਰਾ = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ b_ਪਹਿਲਾ_Past_RJ = 0;
  ਪੂਰਨ has_ਸੱਜਾ_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->nLTerm==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->wsFlags==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->nLSlot>=Array_ਮਾਪ(pNew->aLTermSpace) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->aLTerm!=0 );

  pBuilder->i_Plan_ਹੱਦਬੰਦੀ = SQLITE_QUERY_PLANNER_ਹੱਦਬੰਦੀ;
  ਲਈ(iTab=0, p_ਵਸਤੂ=pTabList->a; p_ਵਸਤੂ<p_ਸਿਰਾ; iTab++, p_ਵਸਤੂ++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->i_Plan_ਹੱਦਬੰਦੀ += SQLITE_QUERY_PLANNER_ਹੱਦਬੰਦੀ_INCR;
    pNew->ਨਕਾਬ_Self = sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, p_ਵਸਤੂ->iCursor);
    ਜੇ( b_ਪਹਿਲਾ_Past_RJ
     || (p_ਵਸਤੂ->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
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
      ਜੇ( p_ਵਸਤੂ->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( p_ਵਸਤੂ->fg.jointype & JT_LTORJ );
        testcase( p_ਵਸਤੂ->fg.jointype & JT_CROSS );
        has_ਸੱਜਾ_Cross_Join = 1;
      }
      mPrereq |= m_ਪੂਰਵ;
      b_ਪਹਿਲਾ_Past_RJ = (p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ)!=0;
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      Where_ਪਦ *p_ਪਦ;
      ਪੂਰਨ i;
      ਲਈ(i=pWC->n_ਅਧਾਰ, p_ਪਦ=pWC->a; i>0; i--, p_ਪਦ++){
        ਜੇ( (pNew->ਨਕਾਬ_Self & p_ਪਦ->prereqAll)!=0 ){
          mPrereq |= (p_ਪਦ->prereqAll & (pNew->ਨਕਾਬ_Self-1));
        }
      }
    }ਨਹੀਂਤਾਂ ਜੇ( !has_ਸੱਜਾ_Cross_Join ){
      mPrereq = 0;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਜੇ( IsVirtual(p_ਵਸਤੂ->pSTab) ){
      Src_ਵਸਤੂ *p;
      ਲਈ(p=&p_ਵਸਤੂ[1]; p<p_ਸਿਰਾ; p++){
        ਜੇ( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, p->iCursor);
        }
      }
      rc = where_Loop_ਜੋੜੋ_Virtual(pBuilder, mPrereq, mUnusable);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = where_Loop_ਜੋੜੋ_Btree(pBuilder, mPrereq);
    }
    ਜੇ( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = where_Loop_ਜੋੜੋ_Or(pBuilder, mPrereq, mUnusable);
    }
    m_ਪੂਰਵ |= pNew->ਨਕਾਬ_Self;
    ਜੇ( rc || db->mallocFailed ){
      ਜੇ( rc==SQLITE_ਪੂਰਾ ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }ਨਹੀਂਤਾਂ{
        ਰੋਕੋ;
      }
    }
  }

  whereLoopClear(db, pNew);
  ਵਾਪਸ rc;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ where_ਰਸਤਾ_ਮੇਲ_Subquery_OB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  ਪੂਰਨ iLoop,              /* Which level of the nested loop.  0==outermost */
  ਪੂਰਨ iCur,               /* Cursor used by the this loop */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,     /* The ORDER BY clause on the whole query */
  Bitmask *p_Rev_ਨਕਾਬ,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  ਪੂਰਨ iOB;                /* Index into pOrderBy->a[] */
  ਪੂਰਨ jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  ਵਿਅੰਜਕ *pOBExpr;          /* Current term of outer ORDER BY */
  ਵਿਅੰਜਕ_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.p_ਕ੍ਰਮ_By;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubOB!=0 );
  ਲਈ(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  ਲਈ(jSub=0; jSub<pSubOB->n_ਵਿਅੰਜਕ && iOB<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; jSub++, iOB++){
    ਜੇ( pSubOB->a[jSub].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ==0 ) ਰੋਕੋ;
    pOBExpr = p_ਕ੍ਰਮ_By->a[iOB].p_ਵਿਅੰਜਕ;
    ਜੇ( pOBExpr->op!=TK_ਕਾਲਮ && pOBExpr->op!=TK_AGG_ਕਾਲਮ ) ਰੋਕੋ;
    ਜੇ( pOBExpr->i_ਸਾਰਣੀ!=iCur ) ਰੋਕੋ;
    ਜੇ( pOBExpr->i_ਕਾਲਮ!=pSubOB->a[jSub].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ-1 ) ਰੋਕੋ;
    ਜੇ( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = p_ਕ੍ਰਮ_By->a[iOB].fg.ਛਾਂਟੋ_Flags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.ਛਾਂਟੋ_Flags;   /* sortFlags for jSub */
      ਜੇ( (sfSub & KEYINFO_ਕ੍ਰਮ_BIGNULL) != (sfOB & KEYINFO_ਕ੍ਰਮ_BIGNULL) ){
        ਰੋਕੋ;
      }
      revIdx = sfSub & KEYINFO_ਕ੍ਰਮ_DESC;
      ਜੇ( jSub>0 ){
        ਜੇ( (rev^revIdx)!=(sfOB & KEYINFO_ਕ੍ਰਮ_DESC) ){
          ਰੋਕੋ;
        }
      }ਨਹੀਂਤਾਂ{
        rev = revIdx ^ (sfOB & KEYINFO_ਕ੍ਰਮ_DESC);
        ਜੇ( rev ){
          ਜੇ( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            ਰੋਕੋ;
          }
          *p_Rev_ਨਕਾਬ |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  ਵਾਪਸ jSub>0;
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
ਸਥਿਰ i8 where_ਰਸਤਾ_Satisfies_ਕ੍ਰਮ_By(
  WhereInfo *pWInfo,    /* The WHERE clause */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  Where_ਰਸਤਾ *p_ਰਸਤਾ,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *p_ਆਖਰੀ,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *p_Rev_ਨਕਾਬ     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 rev_ਸੈੱਟ;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 is_ਕ੍ਰਮ_Distinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 is_ਮੇਲ;           /* iColumn matches a term of the ORDER BY clause */
  u16 eq_Op_ਨਕਾਬ;         /* Allowed equality operators */
  u16 n_ਕੁੰਜੀ_ਖਾਨਾ;          /* Number of key columns in pIndex */
  u16 n_ਕਾਲਮ;          /* Total number of ordered columns in the index */
  u16 n_ਕ੍ਰਮ_By;         /* Number terms in the ORDER BY clause */
  ਪੂਰਨ iLoop;            /* Index of WhereLoop in pPath being processed */
  ਪੂਰਨ i, j;             /* Loop counters */
  ਪੂਰਨ iCur;             /* Cursor number for current WhereLoop */
  ਪੂਰਨ i_ਕਾਲਮ;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  Where_ਪਦ *p_ਪਦ;     /* A single term of the WHERE clause */
  ਵਿਅੰਜਕ *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask ob_ਪੂਰਾ;       /* Mask of all ORDER BY terms */
  Bitmask ਕ੍ਰਮ_Distinct_ਨਕਾਬ;  /* Mask of all well-ordered loops */
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

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕ੍ਰਮ_By!=0 );
  ਜੇ( nLoop && OptimizationDisabled(db, SQLITE_ਕ੍ਰਮ_By_Idx_Join) ) ਵਾਪਸ 0;

  n_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;
  testcase( n_ਕ੍ਰਮ_By==BMS-1 );
  ਜੇ( n_ਕ੍ਰਮ_By>BMS-1 ) ਵਾਪਸ 0;  /* Cannot optimize overly large ORDER BYs */
  is_ਕ੍ਰਮ_Distinct = 1;
  ob_ਪੂਰਾ = MASKBIT(n_ਕ੍ਰਮ_By)-1;
  ਕ੍ਰਮ_Distinct_ਨਕਾਬ = 0;
  ready = 0;
  eq_Op_ਨਕਾਬ = WO_EQ | WO_IS | WO_ISNULL;
  ਜੇ( wctrlFlags & (WHERE_ORDERBY_ਹੱਦਬੰਦੀ|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eq_Op_ਨਕਾਬ |= WO_ਵਿੱਚ;
  }
  ਲਈ(iLoop=0; is_ਕ੍ਰਮ_Distinct && obSat<ob_ਪੂਰਾ && iLoop<=nLoop; iLoop++){
    ਜੇ( iLoop>0 ) ready |= pLoop->ਨਕਾਬ_Self;
    ਜੇ( iLoop<nLoop ){
      pLoop = p_ਰਸਤਾ->aLoop[iLoop];
      ਜੇ( wctrlFlags & WHERE_ORDERBY_ਹੱਦਬੰਦੀ ) ਜਾਰੀ;
    }ਨਹੀਂਤਾਂ{
      pLoop = p_ਆਖਰੀ;
    }
    ਜੇ( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      ਜੇ( pLoop->u.vtab.isOrdered && pWInfo->p_ਕ੍ਰਮ_By==p_ਕ੍ਰਮ_By ){
        obSat = ob_ਪੂਰਾ;
      }ਨਹੀਂਤਾਂ{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        is_ਕ੍ਰਮ_Distinct = 0;
      }
      ਰੋਕੋ;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    ਲਈ(i=0; i<n_ਕ੍ਰਮ_By; i++){
      ਜੇ( MASKBIT(i) & obSat ) ਜਾਰੀ;
      pOBExpr = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ);
      ਜੇ( NEVER(pOBExpr==0) ) ਜਾਰੀ;
      ਜੇ( pOBExpr->op!=TK_ਕਾਲਮ && pOBExpr->op!=TK_AGG_ਕਾਲਮ ) ਜਾਰੀ;
      ਜੇ( pOBExpr->i_ਸਾਰਣੀ!=iCur ) ਜਾਰੀ;
      p_ਪਦ = sqlite3_Where_ਲੱਭੋ_ਪਦ(&pWInfo->sWC, iCur, pOBExpr->i_ਕਾਲਮ,
                       ~ready, eq_Op_ਨਕਾਬ, 0);
      ਜੇ( p_ਪਦ==0 ) ਜਾਰੀ;
      ਜੇ( p_ਪਦ->eOperator==WO_ਵਿੱਚ ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        ਪ੍ਰਤਿਜ੍ਞਾ( wctrlFlags &
               (WHERE_ORDERBY_ਹੱਦਬੰਦੀ|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        ਲਈ(j=0; j<pLoop->nLTerm && p_ਪਦ!=pLoop->aLTerm[j]; j++){}
        ਜੇ( j>=pLoop->nLTerm ) ਜਾਰੀ;
      }
      ਜੇ( (p_ਪਦ->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->i_ਕਾਲਮ>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pParse, p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ);
        ਅਚਲ ਵਿਅੰਜਕ *pCExpr = p_ਪਦ->p_ਵਿਅੰਜਕ;
        CollSeq *pColl2 = sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pCExpr);
        ਅੱਖਰ aff_ਸੱਜਾ;
        ਪ੍ਰਤਿਜ੍ਞਾ( pColl1 );
        ਜੇ( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          ਜਾਰੀ;
        }
        aff_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Affinity(pCExpr->p_ਸੱਜਾ);
        /* Left operand must be a column, hence always has affinity */
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਵਿਅੰਜਕ_Affinity(pCExpr->p_ਖੱਬਾ)!=0 );
        ਜੇ( aff_ਸੱਜਾ!=0 && aff_ਸੱਜਾ!=sqlite3_ਵਿਅੰਜਕ_Affinity(pCExpr->p_ਖੱਬਾ) ){
          ਜਾਰੀ;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    ਜੇ( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      ਜੇ( pLoop->wsFlags & WHERE_IPK ){
        ਜੇ( pLoop->u.btree.p_ਕ੍ਰਮ_By
         && OptimizationEnabled(db, SQLITE_ਕ੍ਰਮ_By_Subq)
         &&  where_ਰਸਤਾ_ਮੇਲ_Subquery_OB(pLoop,iLoop,iCur,
                                     p_ਕ੍ਰਮ_By,p_Rev_ਨਕਾਬ, &obSat, wctrlFlags)
        ){
          n_ਕਾਲਮ = 0;
          is_ਕ੍ਰਮ_Distinct = 0;
        }ਨਹੀਂਤਾਂ{
          n_ਕਾਲਮ = 1;
        }
        p_ਸੂਚਕਾਂਕ = 0;
        n_ਕੁੰਜੀ_ਖਾਨਾ = 1;
      }ਨਹੀਂਤਾਂ ਜੇ( (p_ਸੂਚਕਾਂਕ = pLoop->u.btree.p_ਸੂਚਕਾਂਕ)==0 || p_ਸੂਚਕਾਂਕ->bUnordered ){
        ਵਾਪਸ 0;
      }ਨਹੀਂਤਾਂ{
        n_ਕੁੰਜੀ_ਖਾਨਾ = p_ਸੂਚਕਾਂਕ->n_ਕੁੰਜੀ_ਖਾਨਾ;
        n_ਕਾਲਮ = p_ਸੂਚਕਾਂਕ->n_ਕਾਲਮ;
        ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕਾਲਮ==n_ਕੁੰਜੀ_ਖਾਨਾ+1 || !HasRowid(p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ) );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੂਚਕਾਂਕ->ai_ਕਾਲਮ[n_ਕਾਲਮ-1]==XN_ROWID
                          || !HasRowid(p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        is_ਕ੍ਰਮ_Distinct = Is_Unique_ਸੂਚਕਾਂਕ(p_ਸੂਚਕਾਂਕ)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = rev_ਸੈੱਟ = 0;
      distinctColumns = 0;
      ਲਈ(j=0; j<n_ਕਾਲਮ; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        ਪ੍ਰਤਿਜ੍ਞਾ( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        ਜੇ( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
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
          ਜੇ( (eOp & eq_Op_ਨਕਾਬ)!=0 ){
            ਜੇ( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( is_ਕ੍ਰਮ_Distinct );
              is_ਕ੍ਰਮ_Distinct = 0;
            }
            ਜਾਰੀ; 
          }ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(eOp & WO_ਵਿੱਚ) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            ਵਿਅੰਜਕ *pX = pLoop->aLTerm[j]->p_ਵਿਅੰਜਕ;
            ਲਈ(i=j+1; i<pLoop->u.btree.nEq; i++){
              ਜੇ( pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ==pX ){
                ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->aLTerm[i]->eOperator & WO_ਵਿੱਚ) );
                bOnce = 0;
                ਰੋਕੋ;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        ਜੇ( p_ਸੂਚਕਾਂਕ ){
          i_ਕਾਲਮ = p_ਸੂਚਕਾਂਕ->ai_ਕਾਲਮ[j];
          revIdx = p_ਸੂਚਕਾਂਕ->a_ਛਾਂਟੋ_ਕ੍ਰਮ[j] & KEYINFO_ਕ੍ਰਮ_DESC;
          ਜੇ( i_ਕਾਲਮ==p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ->iPKey ) i_ਕਾਲਮ = XN_ROWID;
        }ਨਹੀਂਤਾਂ{
          i_ਕਾਲਮ = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        ਜੇ( is_ਕ੍ਰਮ_Distinct ){
          ਜੇ( i_ਕਾਲਮ>=0
           && j>=pLoop->u.btree.nEq
           && p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ->a_ਖਾਨਾ[i_ਕਾਲਮ].notNull==0
          ){
            is_ਕ੍ਰਮ_Distinct = 0;
          }
          ਜੇ( i_ਕਾਲਮ==XN_ਵਿਅੰਜਕ ){
            is_ਕ੍ਰਮ_Distinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        is_ਮੇਲ = 0;
        ਲਈ(i=0; bOnce && i<n_ਕ੍ਰਮ_By; i++){
          ਜੇ( MASKBIT(i) & obSat ) ਜਾਰੀ;
          pOBExpr = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          ਜੇ( NEVER(pOBExpr==0) ) ਜਾਰੀ;
          ਜੇ( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          ਜੇ( i_ਕਾਲਮ>=XN_ROWID ){
            ਜੇ( pOBExpr->op!=TK_ਕਾਲਮ && pOBExpr->op!=TK_AGG_ਕਾਲਮ ) ਜਾਰੀ;
            ਜੇ( pOBExpr->i_ਸਾਰਣੀ!=iCur ) ਜਾਰੀ;
            ਜੇ( pOBExpr->i_ਕਾਲਮ!=i_ਕਾਲਮ ) ਜਾਰੀ;
          }ਨਹੀਂਤਾਂ{
            ਵਿਅੰਜਕ *p_Ix_ਵਿਅੰਜਕ = p_ਸੂਚਕਾਂਕ->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[j].p_ਵਿਅੰਜਕ;
            ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare_Skip(pOBExpr, p_Ix_ਵਿਅੰਜਕ, iCur) ){
              ਜਾਰੀ;
            }
          }
          ਜੇ( i_ਕਾਲਮ!=XN_ROWID ){
            pColl = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pWInfo->pParse, p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ);
            ਜੇ( sqlite3StrICmp(pColl->zName, p_ਸੂਚਕਾਂਕ->azColl[j])!=0 ) ਜਾਰੀ;
          }
          ਜੇ( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.n_Distinct_ਖਾਨਾ = j+1;
          }
          is_ਮੇਲ = 1;
          ਰੋਕੋ;
        }
        ਜੇ( is_ਮੇਲ && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          ਜੇ( rev_ਸੈੱਟ ){
            ਜੇ( (rev ^ revIdx)
                           != (p_ਕ੍ਰਮ_By->a[i].fg.ਛਾਂਟੋ_Flags&KEYINFO_ਕ੍ਰਮ_DESC)
            ){
              is_ਮੇਲ = 0;
            }
          }ਨਹੀਂਤਾਂ{
            rev = revIdx ^ (p_ਕ੍ਰਮ_By->a[i].fg.ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_DESC);
            ਜੇ( rev ) *p_Rev_ਨਕਾਬ |= MASKBIT(iLoop);
            rev_ਸੈੱਟ = 1;
          }
        }
        ਜੇ( is_ਮੇਲ && (p_ਕ੍ਰਮ_By->a[i].fg.ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_BIGNULL) ){
          ਜੇ( j==pLoop->u.btree.nEq 
           && (j<n_ਕੁੰਜੀ_ਖਾਨਾ||HasRowid(p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ)||!Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(p_ਸੂਚਕਾਂਕ))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_ਛਾਂਟੋ;
          }ਨਹੀਂਤਾਂ{
            is_ਮੇਲ = 0;
          }
        }
        ਜੇ( is_ਮੇਲ ){
          ਜੇ( i_ਕਾਲਮ==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }ਨਹੀਂਤਾਂ{
          /* No match found */
          ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ_ਖਾਨਾ>0 );
          ਜੇ( j<n_ਕੁੰਜੀ_ਖਾਨਾ ){
            testcase( is_ਕ੍ਰਮ_Distinct!=0 );
            is_ਕ੍ਰਮ_Distinct = 0;
          }
          ਰੋਕੋ;
        }
      } /* end Loop over all index columns */
      ਜੇ( distinctColumns ){
        testcase( is_ਕ੍ਰਮ_Distinct==0 );
        is_ਕ੍ਰਮ_Distinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    ਜੇ( is_ਕ੍ਰਮ_Distinct ){
      ਕ੍ਰਮ_Distinct_ਨਕਾਬ |= pLoop->ਨਕਾਬ_Self;
      ਲਈ(i=0; i<n_ਕ੍ਰਮ_By; i++){
        ਵਿਅੰਜਕ *p;
        Bitmask m_ਪਦ;
        ਜੇ( MASKBIT(i) & obSat ) ਜਾਰੀ;
        p = p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ;
        m_ਪਦ = sqlite3_Where_ਵਿਅੰਜਕ_Usage(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ,p);
        ਜੇ( m_ਪਦ==0 && !sqlite3_ਵਿਅੰਜਕ_Is_Constant(0,p) ) ਜਾਰੀ;
        ਜੇ( (m_ਪਦ&~ਕ੍ਰਮ_Distinct_ਨਕਾਬ)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  ਜੇ( obSat==ob_ਪੂਰਾ ) ਵਾਪਸ (i8)n_ਕ੍ਰਮ_By;
  ਜੇ( !is_ਕ੍ਰਮ_Distinct ){
    ਲਈ(i=n_ਕ੍ਰਮ_By-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      ਜੇ( (obSat&m)==m ) ਵਾਪਸ i;
    }
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ -1;
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
ਪੂਰਨ sqlite3WhereIsSorted(WhereInfo *pWInfo){
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  ਵਾਪਸ pWInfo->sorted;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
/* For debugging use only: */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *where_ਰਸਤਾ_Name(Where_ਰਸਤਾ *p_ਰਸਤਾ, ਪੂਰਨ nLoop, WhereLoop *p_ਆਖਰੀ){
  ਸਥਿਰ ਅੱਖਰ zName[65];
  ਪੂਰਨ i;
  ਲਈ(i=0; i<nLoop; i++){ zName[i] = p_ਰਸਤਾ->aLoop[i]->cId; }
  ਜੇ( p_ਆਖਰੀ ) zName[i++] = p_ਆਖਰੀ->cId;
  zName[i] = 0;
  ਵਾਪਸ zName;
}
#ਜੇ_ਅੰਤ

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
ਸਥਿਰ LogEst where_ਛਾਂਟੀ_ਲਾਗਤ(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst n_ਪੰਕਤੀ,       /* Estimated number of rows to sort */
  ਪੂਰਨ n_ਕ੍ਰਮ_By,      /* Number of ORDER BY clause terms */
  ਪੂਰਨ nSorted        /* Number of initial ORDER BY terms naturally in order */
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
  LogEst r_ਛਾਂਟੋ_ਲਾਗਤ, n_ਖਾਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pSelect!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  n_ਖਾਨਾ = sqlite3LogEst((pWInfo->pSelect->pEList->n_ਵਿਅੰਜਕ+59)/30);
  r_ਛਾਂਟੋ_ਲਾਗਤ = n_ਪੰਕਤੀ + n_ਖਾਨਾ;
  ਜੇ( nSorted>0 ){
    /* Scale the result by (Y/X) */
    r_ਛਾਂਟੋ_ਲਾਗਤ += sqlite3LogEst((n_ਕ੍ਰਮ_By-nSorted)*100/n_ਕ੍ਰਮ_By) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  ਜੇ( (pWInfo->wctrlFlags & WHERE_USE_ਹੱਦਬੰਦੀ)!=0 ){
    r_ਛਾਂਟੋ_ਲਾਗਤ += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    ਜੇ( nSorted!=0 ){
      r_ਛਾਂਟੋ_ਲਾਗਤ += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    ਜੇ( pWInfo->i_ਹੱਦਬੰਦੀ<n_ਪੰਕਤੀ ){
      n_ਪੰਕਤੀ = pWInfo->i_ਹੱਦਬੰਦੀ;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    ਜੇ( n_ਪੰਕਤੀ>10 ){ n_ਪੰਕਤੀ -= 10;  ਪ੍ਰਤਿਜ੍ਞਾ( 10==sqlite3LogEst(2) ); }
  }
  r_ਛਾਂਟੋ_ਲਾਗਤ += estLog(n_ਪੰਕਤੀ);
  ਵਾਪਸ r_ਛਾਂਟੋ_ਲਾਗਤ;
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
ਸਥਿਰ ਪੂਰਨ computeMxChoice(WhereInfo *pWInfo){
  ਪੂਰਨ nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  ਲਈ(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
    ਪ੍ਰਤਿਜ੍ਞਾ( pWLoop->ਨਕਾਬ_Self==MASKBIT(pWLoop->iTab) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pWLoop->p_ਅਗਲਾ_Loop==0 || pWLoop->iTab<=pWLoop->p_ਅਗਲਾ_Loop->iTab );
  }
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

  ਜੇ( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->b_ਤਾਰਾ_ਪੂਰਾ
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_ਤਾਰਾ_Query)
  ){
    Src_ਵਸਤੂ *aFromTabs;    /* All terms of the FROM clause */
    ਪੂਰਨ iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *p_ਸ਼ੁਰੂ;     /* Where to start searching for dimension-tables */

    pWInfo->b_ਤਾਰਾ_ਪੂਰਾ = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( !pWInfo->b_ਤਾਰਾ_ਵਰਤਿਆ );
    aFromTabs = pWInfo->pTabList->a;
    p_ਸ਼ੁਰੂ = pWInfo->pLoops;
    ਲਈ(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      ਪੂਰਨ nDep = 0;             /* Number of dimension tables */
      LogEst mx_ਲੜੀਵਾਰ;             /* Maximum SCAN cost of a fact table */
      Bitmask m_ਵੇਖਿਆ = 0;        /* Mask of dimension tables */
      Src_ਵਸਤੂ *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      ਜੇ( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        ਜੇ( iFromIdx+3 > nLoop ){
          ਰੋਕੋ;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        ਜਦੋਂ( p_ਸ਼ੁਰੂ && p_ਸ਼ੁਰੂ->iTab<=iFromIdx ){
          p_ਸ਼ੁਰੂ = p_ਸ਼ੁਰੂ->p_ਅਗਲਾ_Loop;
        }
      }
      ਲਈ(pWLoop=p_ਸ਼ੁਰੂ; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
        ਜੇ( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          ਰੋਕੋ; /* Constraint (bb) */
        }
        ਜੇ( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->ਨਕਾਬ_Self & m_ਵੇਖਿਆ)==0  /* pWInfo not already a dependency */
         && (pWLoop->ਨਕਾਬ_Self & mSelfJoin)==0 /* Not a self-join */
        ){
          ਜੇ( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }ਨਹੀਂਤਾਂ{
            nDep++;
            m_ਵੇਖਿਆ |= pWLoop->ਨਕਾਬ_Self;
          }
        }
      }
      ਜੇ( nDep<=2 ){
        ਜਾਰੀ; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      ਜੇ( !pWInfo->b_ਤਾਰਾ_ਵਰਤਿਆ ){
        ਲਈ(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
          pWLoop->r_ਤਾਰਾ_Delta = 0;
        }
      }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x80000 */
      ਜੇ( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = m_ਵੇਖਿਆ;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        ਲਈ(pWLoop=p_ਸ਼ੁਰੂ; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
          ਜੇ( mShow & pWLoop->ਨਕਾਬ_Self ){
            Src_ਵਸਤੂ *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->ਨਕਾਬ_Self;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#ਜੇ_ਅੰਤ
      pWInfo->b_ਤਾਰਾ_ਵਰਤਿਆ = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mx_ਲੜੀਵਾਰ = LOGEST_MIN;
      ਲਈ(pWLoop=p_ਸ਼ੁਰੂ; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
        ਜੇ( pWLoop->iTab<iFromIdx ) ਜਾਰੀ;
        ਜੇ( pWLoop->iTab>iFromIdx ) ਰੋਕੋ;
        ਜੇ( pWLoop->r_ਲੜੀਵਾਰ>mx_ਲੜੀਵਾਰ ) mx_ਲੜੀਵਾਰ = pWLoop->r_ਲੜੀਵਾਰ;
      }
      ਜੇ( ALWAYS(mx_ਲੜੀਵਾਰ<LOGEST_MAX) ) mx_ਲੜੀਵਾਰ++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      ਲਈ(pWLoop=p_ਸ਼ੁਰੂ; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
        ਜੇ( (pWLoop->ਨਕਾਬ_Self & m_ਵੇਖਿਆ)==0 ) ਜਾਰੀ;
        ਜੇ( pWLoop->nLTerm ) ਜਾਰੀ;
        ਜੇ( pWLoop->r_ਲੜੀਵਾਰ<mx_ਲੜੀਵਾਰ ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x80000 */
          ਜੇ( sqlite3WhereTrace & 0x80000 ){
            Src_ਵਸਤੂ *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mx_ਲੜੀਵਾਰ
            );
          }
          pWLoop->r_ਤਾਰਾ_Delta = mx_ਲੜੀਵਾਰ - pWLoop->r_ਲੜੀਵਾਰ;
#ਜੇ_ਅੰਤ /* WHERETRACE_ENABLED */
          pWLoop->r_ਲੜੀਵਾਰ = mx_ਲੜੀਵਾਰ;
        }
      }
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x80000 */
    ਜੇ( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->b_ਤਾਰਾ_ਵਰਤਿਆ ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      ਲਈ(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
        ਜੇ( pWLoop->r_ਤਾਰਾ_Delta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ pWInfo->b_ਤਾਰਾ_ਵਰਤਿਆ ? 18 : 12;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ whereLoopIsNoBetter(
  ਅਚਲ WhereLoop *pCandidate,
  ਅਚਲ WhereLoop *pBaseline
){
  ਜੇ( (pCandidate->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)==0 ) ਵਾਪਸ 1;
  ਜੇ( (pBaseline->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)==0 ) ਵਾਪਸ 1;
  ਜੇ( pCandidate->u.btree.p_ਸੂਚਕਾਂਕ->sz_Idx_ਪੰਕਤੀ <
        pBaseline->u.btree.p_ਸੂਚਕਾਂਕ->sz_Idx_ਪੰਕਤੀ ) ਵਾਪਸ 0;
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ where_ਰਸਤਾ_Solver(WhereInfo *pWInfo, LogEst n_ਪੰਕਤੀ_Est){
  ਪੂਰਨ mxChoice;             /* Maximum number of simultaneous paths tracked */
  ਪੂਰਨ nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  ਪੂਰਨ iLoop;                /* Loop counter over the terms of the join */
  ਪੂਰਨ ii, jj;               /* Loop counters */
  ਪੂਰਨ mxI = 0;              /* Index of next entry to replace */
  ਪੂਰਨ n_ਕ੍ਰਮ_By;             /* Number of ORDER BY clause terms */
  LogEst mx_ਲਾਗਤ = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  ਪੂਰਨ n_ਮੰਜ਼ਿਲ, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  Where_ਰਸਤਾ *aFrom;         /* All nFrom paths at the previous level */
  Where_ਰਸਤਾ *a_ਮੰਜ਼ਿਲ;           /* The nTo best paths at the current level */
  Where_ਰਸਤਾ *pFrom;         /* An element of aFrom[] that we are working on */
  Where_ਰਸਤਾ *p_ਮੰਜ਼ਿਲ;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *a_ਛਾਂਟੋ_ਲਾਗਤ = 0;    /* Sorting and partial sorting costs */
  ਅੱਖਰ *pSpace;             /* Temporary memory used by this routine */
  ਪੂਰਨ nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     n_ਪੰਕਤੀ_Est, pParse->nQueryLoop));
  /* TUNING: mxChoice is the maximum number of possible paths to preserve
  ** at each step.  Based on the number of loops in the FROM clause:
  **
  **     nLoop      mxChoice
  **     -----      --------
  **       1            1            // the most common case
  **       2            5
  **       3+        12 or 18        // see computeMxChoice()
  */
  ਜੇ( nLoop<=1 ){
    mxChoice = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( nLoop==2 ){
    mxChoice = 5;
  }ਨਹੀਂਤਾਂ ਜੇ( pParse->nErr ){
    mxChoice = 1;
  }ਨਹੀਂਤਾਂ{
    mxChoice = computeMxChoice(pWInfo);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  ਜੇ( pWInfo->p_ਕ੍ਰਮ_By==0 || n_ਪੰਕਤੀ_Est==0 ){
    n_ਕ੍ਰਮ_By = 0;
  }ਨਹੀਂਤਾਂ{
    n_ਕ੍ਰਮ_By = pWInfo->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (ਆਕਾਰ(Where_ਰਸਤਾ)+ਆਕਾਰ(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += ਆਕਾਰ(LogEst) * n_ਕ੍ਰਮ_By;
  pSpace = sqlite3_ਢੇਰ_Alloc_Raw_NN(pParse->db, nSpace);
  ਜੇ( pSpace==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  a_ਮੰਜ਼ਿਲ = (Where_ਰਸਤਾ*)pSpace;
  aFrom = a_ਮੰਜ਼ਿਲ+mxChoice;
  memset(aFrom, 0, ਆਕਾਰ(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  ਲਈ(ii=mxChoice*2, pFrom=a_ਮੰਜ਼ਿਲ; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  ਜੇ( n_ਕ੍ਰਮ_By ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    a_ਛਾਂਟੋ_ਲਾਗਤ = (LogEst*)pX;
    memset(a_ਛਾਂਟੋ_ਲਾਗਤ, 0, ਆਕਾਰ(LogEst) * n_ਕ੍ਰਮ_By);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( a_ਛਾਂਟੋ_ਲਾਗਤ==0 || &pSpace[nSpace]==(ਅੱਖਰ*)&a_ਛਾਂਟੋ_ਲਾਗਤ[n_ਕ੍ਰਮ_By] );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_ਛਾਂਟੋ_ਲਾਗਤ!=0 || &pSpace[nSpace]==(ਅੱਖਰ*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].n_ਪੰਕਤੀ = MIN(pParse->nQueryLoop, 48);  ਪ੍ਰਤਿਜ੍ਞਾ( 48==sqlite3LogEst(28) );
  nFrom = 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( aFrom[0].isOrdered==0 );
  ਜੇ( n_ਕ੍ਰਮ_By ){
    /* If nLoop is zero, then there are no FROM terms in the query. Since
    ** in this case the query may return a maximum of one row, the results
    ** are already in the requested order. Set isOrdered to nOrderBy to
    ** indicate this. Or, if nLoop is greater than zero, set isOrdered to
    ** -1, indicating that the result set may or may not be ordered,
    ** depending on the loops added to the current plan.  */
    aFrom[0].isOrdered = nLoop>0 ? -1 : n_ਕ੍ਰਮ_By;
  }

  /* Compute successively longer WherePaths using the previous generation
  ** of WherePaths as the basis for the next.  Keep track of the mxChoice
  ** best paths at each generation */
  ਲਈ(iLoop=0; iLoop<nLoop; iLoop++){
    n_ਮੰਜ਼ਿਲ = 0;
    ਲਈ(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      ਲਈ(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_ਅਗਲਾ_Loop){
        LogEst n_ਬਾਹਰਲਾ;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst r_ਲਾਗਤ;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask ਨਕਾਬ_New;                  /* Mask of src visited by (..) */
        Bitmask rev_ਨਕਾਬ;                  /* Mask of rev-order loops for (..) */

        ਜੇ( (pWLoop->prereq & ~pFrom->ਨਕਾਬ_Loop)!=0 ) ਜਾਰੀ;
        ਜੇ( (pWLoop->ਨਕਾਬ_Self & pFrom->ਨਕਾਬ_Loop)!=0 ) ਜਾਰੀ;
        ਜੇ( (pWLoop->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)!=0 && pFrom->n_ਪੰਕਤੀ<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          ਪ੍ਰਤਿਜ੍ਞਾ( 10==sqlite3LogEst(2) );
          ਜਾਰੀ;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->r_ਲੜੀਵਾਰ + pFrom->n_ਪੰਕਤੀ;
        ਜੇ( pWLoop->r_ਤਿਆਰੀ ){
          rUnsort = sqlite3_Log_Est_ਜੋੜੋ(pWLoop->r_ਤਿਆਰੀ, rUnsort);
        }
        rUnsort = sqlite3_Log_Est_ਜੋੜੋ(rUnsort, pFrom->rUnsort);
        n_ਬਾਹਰਲਾ = pFrom->n_ਪੰਕਤੀ + pWLoop->n_ਬਾਹਰਲਾ;
        ਨਕਾਬ_New = pFrom->ਨਕਾਬ_Loop | pWLoop->ਨਕਾਬ_Self;
        isOrdered = pFrom->isOrdered;
        ਜੇ( isOrdered<0 ){
          rev_ਨਕਾਬ = 0;
          isOrdered = where_ਰਸਤਾ_Satisfies_ਕ੍ਰਮ_By(pWInfo,
                       pWInfo->p_ਕ੍ਰਮ_By, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &rev_ਨਕਾਬ);
        }ਨਹੀਂਤਾਂ{
          rev_ਨਕਾਬ = pFrom->revLoop;
        }
        ਜੇ( isOrdered>=0 && isOrdered<n_ਕ੍ਰਮ_By ){
          ਜੇ( a_ਛਾਂਟੋ_ਲਾਗਤ[isOrdered]==0 ){
            a_ਛਾਂਟੋ_ਲਾਗਤ[isOrdered] = where_ਛਾਂਟੀ_ਲਾਗਤ(
                pWInfo, n_ਪੰਕਤੀ_Est, n_ਕ੍ਰਮ_By, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          r_ਲਾਗਤ = sqlite3_Log_Est_ਜੋੜੋ(rUnsort, a_ਛਾਂਟੋ_ਲਾਗਤ[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               a_ਛਾਂਟੋ_ਲਾਗਤ[isOrdered], (n_ਕ੍ਰਮ_By-isOrdered), n_ਕ੍ਰਮ_By,
               rUnsort, r_ਲਾਗਤ));
        }ਨਹੀਂਤਾਂ{
          r_ਲਾਗਤ = rUnsort;
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
        testcase( n_ਮੰਜ਼ਿਲ==0 );
        ਲਈ(jj=0, p_ਮੰਜ਼ਿਲ=a_ਮੰਜ਼ਿਲ; jj<n_ਮੰਜ਼ਿਲ; jj++, p_ਮੰਜ਼ਿਲ++){
          ਜੇ( p_ਮੰਜ਼ਿਲ->ਨਕਾਬ_Loop==ਨਕਾਬ_New
           && ( ((p_ਮੰਜ਼ਿਲ->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==n_ਮੰਜ਼ਿਲ-1 );
            ਰੋਕੋ;
          }
        }
        ਜੇ( jj>=n_ਮੰਜ਼ਿਲ ){
          /* None of the existing best-so-far paths match the candidate. */
          ਜੇ( n_ਮੰਜ਼ਿਲ>=mxChoice
           && (r_ਲਾਗਤ>mx_ਲਾਗਤ || (r_ਲਾਗਤ==mx_ਲਾਗਤ && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x4 */
            ਜੇ( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  where_ਰਸਤਾ_Name(pFrom, iLoop, pWLoop), r_ਲਾਗਤ, n_ਬਾਹਰਲਾ, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#ਜੇ_ਅੰਤ
            ਜਾਰੀ;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          ਜੇ( n_ਮੰਜ਼ਿਲ<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = n_ਮੰਜ਼ਿਲ++;
          }ਨਹੀਂਤਾਂ{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          p_ਮੰਜ਼ਿਲ = &a_ਮੰਜ਼ਿਲ[jj];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x4 */
          ਜੇ( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                where_ਰਸਤਾ_Name(pFrom, iLoop, pWLoop), r_ਲਾਗਤ, n_ਬਾਹਰਲਾ, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#ਜੇ_ਅੰਤ
        }ਨਹੀਂਤਾਂ{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          ਜੇ( (p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ<r_ਲਾਗਤ)
           || (p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==r_ਲਾਗਤ && p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ<n_ਬਾਹਰਲਾ)
           || (p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==r_ਲਾਗਤ && p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ==n_ਬਾਹਰਲਾ && p_ਮੰਜ਼ਿਲ->rUnsort<rUnsort)
           || (p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==r_ਲਾਗਤ && p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ==n_ਬਾਹਰਲਾ && p_ਮੰਜ਼ਿਲ->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, p_ਮੰਜ਼ਿਲ->aLoop[iLoop]) )
          ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x4 */
            ਜੇ( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  where_ਰਸਤਾ_Name(pFrom, iLoop, pWLoop), r_ਲਾਗਤ, n_ਬਾਹਰਲਾ, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  where_ਰਸਤਾ_Name(p_ਮੰਜ਼ਿਲ, iLoop+1, 0), p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ, p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ,
                  p_ਮੰਜ਼ਿਲ->rUnsort, p_ਮੰਜ਼ਿਲ->isOrdered>=0 ? p_ਮੰਜ਼ਿਲ->isOrdered+'0' : '?');
            }
#ਜੇ_ਅੰਤ
            /* Discard the candidate path from further consideration */
            testcase( p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==r_ਲਾਗਤ );
            ਜਾਰੀ;
          }
          testcase( p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==r_ਲਾਗਤ+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x4 */
          ਜੇ( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                where_ਰਸਤਾ_Name(pFrom, iLoop, pWLoop), r_ਲਾਗਤ, n_ਬਾਹਰਲਾ, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                where_ਰਸਤਾ_Name(p_ਮੰਜ਼ਿਲ, iLoop+1, 0), p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ, p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ,
                p_ਮੰਜ਼ਿਲ->rUnsort, p_ਮੰਜ਼ਿਲ->isOrdered>=0 ? p_ਮੰਜ਼ਿਲ->isOrdered+'0' : '?');
          }
#ਜੇ_ਅੰਤ
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        p_ਮੰਜ਼ਿਲ->ਨਕਾਬ_Loop = pFrom->ਨਕਾਬ_Loop | pWLoop->ਨਕਾਬ_Self;
        p_ਮੰਜ਼ਿਲ->revLoop = rev_ਨਕਾਬ;
        p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ = n_ਬਾਹਰਲਾ;
        p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ = r_ਲਾਗਤ;
        p_ਮੰਜ਼ਿਲ->rUnsort = rUnsort;
        p_ਮੰਜ਼ਿਲ->isOrdered = isOrdered;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ->aLoop, pFrom->aLoop, ਆਕਾਰ(WhereLoop*)*iLoop);
        p_ਮੰਜ਼ਿਲ->aLoop[iLoop] = pWLoop;
        ਜੇ( n_ਮੰਜ਼ਿਲ>=mxChoice ){
          mxI = 0;
          mx_ਲਾਗਤ = a_ਮੰਜ਼ਿਲ[0].r_ਲਾਗਤ;
          mxUnsort = a_ਮੰਜ਼ਿਲ[0].n_ਪੰਕਤੀ;
          ਲਈ(jj=1, p_ਮੰਜ਼ਿਲ=&a_ਮੰਜ਼ਿਲ[1]; jj<mxChoice; jj++, p_ਮੰਜ਼ਿਲ++){
            ਜੇ( p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ>mx_ਲਾਗਤ
             || (p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==mx_ਲਾਗਤ && p_ਮੰਜ਼ਿਲ->rUnsort>mxUnsort)
            ){
              mx_ਲਾਗਤ = p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ;
              mxUnsort = p_ਮੰਜ਼ਿਲ->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED  /* >=2 */
    ਜੇ( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      ਪੂਰਨ n_ਪੂਰਾ = 0;
      ਪੂਰਨ n_ਤਰੱਕੀ;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      ਕਰੋ{
        n_ਤਰੱਕੀ = 0;
        rMin = 0x7fff;
        ਲਈ(ii=0, p_ਮੰਜ਼ਿਲ=a_ਮੰਜ਼ਿਲ; ii<n_ਮੰਜ਼ਿਲ; ii++, p_ਮੰਜ਼ਿਲ++){
          ਜੇ( p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ>rFloor && p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ<rMin ) rMin = p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ;
        }
        ਲਈ(ii=0, p_ਮੰਜ਼ਿਲ=a_ਮੰਜ਼ਿਲ; ii<n_ਮੰਜ਼ਿਲ; ii++, p_ਮੰਜ਼ਿਲ++){
          ਜੇ( p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               where_ਰਸਤਾ_Name(p_ਮੰਜ਼ਿਲ, iLoop+1, 0), p_ਮੰਜ਼ਿਲ->r_ਲਾਗਤ, p_ਮੰਜ਼ਿਲ->n_ਪੰਕਤੀ,
               p_ਮੰਜ਼ਿਲ->isOrdered>=0 ? (p_ਮੰਜ਼ਿਲ->isOrdered+'0') : '?');
            ਜੇ( p_ਮੰਜ਼ਿਲ->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", p_ਮੰਜ਼ਿਲ->revLoop);
            }ਨਹੀਂਤਾਂ{
              sqlite3DebugPrintf("\n");
            }
            n_ਪੂਰਾ++;
            n_ਤਰੱਕੀ++;
          }
        }
        rFloor = rMin;
      }ਜਦੋਂ( n_ਪੂਰਾ<n_ਮੰਜ਼ਿਲ && n_ਤਰੱਕੀ>0 );
    }
#ਜੇ_ਅੰਤ

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = a_ਮੰਜ਼ਿਲ;
    a_ਮੰਜ਼ਿਲ = aFrom;
    aFrom = pFrom;
    nFrom = n_ਮੰਜ਼ਿਲ;
  }

  ਜੇ( nFrom==0 ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no query solution");
    sqlite3_ਢੇਰ_Free_NN(pParse->db, pSpace);
    ਵਾਪਸ SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  ਪ੍ਰਤਿਜ੍ਞਾ( nFrom==1 );
  pFrom = aFrom;

  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  ਲਈ(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  ਜੇ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && n_ਪੰਕਤੀ_Est
  ){
    Bitmask not_ਵਰਤਿਆ;
    ਪੂਰਨ rc = where_ਰਸਤਾ_Satisfies_ਕ੍ਰਮ_By(pWInfo, pWInfo->p_ਨਤੀਜਾ_ਸੈੱਟ, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &not_ਵਰਤਿਆ);
    ਜੇ( rc==pWInfo->p_ਨਤੀਜਾ_ਸੈੱਟ->n_ਵਿਅੰਜਕ ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  ਜੇ( pWInfo->p_ਕ੍ਰਮ_By ){
    pWInfo->nOBSat = pFrom->isOrdered;
    ਜੇ( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      ਜੇ( pFrom->isOrdered==pWInfo->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pSelect->p_ਕ੍ਰਮ_By==0
           || pWInfo->nOBSat <= pWInfo->pSelect->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ );
    }ਨਹੀਂਤਾਂ{
      pWInfo->rev_ਨਕਾਬ = pFrom->revLoop;
      ਜੇ( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        ਜੇ( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          ਜੇ( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_ਕਾਲਮ_ਵਿੱਚ))!=(WHERE_IPK|WHERE_ਕਾਲਮ_ਵਿੱਚ)
          ){
            Bitmask m = 0;
            ਪੂਰਨ rc = where_ਰਸਤਾ_Satisfies_ਕ੍ਰਮ_By(pWInfo, pWInfo->p_ਕ੍ਰਮ_By, pFrom,
                      WHERE_ORDERBY_ਹੱਦਬੰਦੀ, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_ਕਾਲਮ_ਵਿੱਚ );
            ਜੇ( rc==pWInfo->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->rev_ਨਕਾਬ = m;
            }
          }
        }
      }ਨਹੀਂਤਾਂ ਜੇ( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    ਜੇ( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ && nLoop>0
    ){
      Bitmask rev_ਨਕਾਬ = 0;
      ਪੂਰਨ n_ਕ੍ਰਮ = where_ਰਸਤਾ_Satisfies_ਕ੍ਰਮ_By(pWInfo, pWInfo->p_ਕ੍ਰਮ_By,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &rev_ਨਕਾਬ
      );
      ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->sorted==0 );
      ਜੇ( n_ਕ੍ਰਮ==pWInfo->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ ){
        pWInfo->sorted = 1;
        pWInfo->rev_ਨਕਾਬ = rev_ਨਕਾਬ;
      }
    }
  }

  pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ = pFrom->n_ਪੰਕਤੀ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  pWInfo->r_ਕੁੱਲ_ਲਾਗਤ = pFrom->r_ਲਾਗਤ;
#ਜੇ_ਅੰਤ

  /* Free temporary memory and return success */
  sqlite3_ਢੇਰ_Free_NN(pParse->db, pSpace);
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ whereInterstageHeuristic(WhereInfo *pWInfo){
  ਪੂਰਨ i;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  ਪੂਰਨ once = 0;
#ਜੇ_ਅੰਤ
  ਲਈ(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    ਜੇ( p==0 ) ਰੋਕੋ;
    ਜੇ( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      ਰੋਕੋ;
    }
    ਜੇ( (p->wsFlags & (WHERE_ਕਾਲਮ_EQ|WHERE_ਕਾਲਮ_NULL|WHERE_ਕਾਲਮ_ਵਿੱਚ))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      ਲਈ(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_ਅਗਲਾ_Loop){
        ਜੇ( pLoop->iTab!=iTab ) ਜਾਰੀ;
        ਜੇ( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_ਸੂਚਕਾਂਕ))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          ਜਾਰੀ;
        }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
        ਜੇ( sqlite3WhereTrace & 0x80 ){
          ਜੇ( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#ਜੇ_ਅੰਤ /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }ਨਹੀਂਤਾਂ{
      ਰੋਕੋ;
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
ਸਥਿਰ ਪੂਰਨ whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  WhereClause *pWC;
  Where_ਪਦ *p_ਪਦ;
  WhereLoop *pLoop;
  ਪੂਰਨ iCur;
  ਪੂਰਨ j;
  ਸਾਰਣੀ *pTab;
  ਸੂਚਕਾਂਕ *pIdx;
  Where_ਜਾਂਚ ਜਾਂਚ;

  pWInfo = pBuilder->pWInfo;
  ਜੇ( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pTabList->nSrc>=1 );
  p_ਵਸਤੂ = pWInfo->pTabList->a;
  pTab = p_ਵਸਤੂ->pSTab;
  ਜੇ( IsVirtual(pTab) ) ਵਾਪਸ 0;
  ਜੇ( p_ਵਸਤੂ->fg.is_ਸੂਚਕਾਂਕਿਤ_By || p_ਵਸਤੂ->fg.not_ਸੂਚਕਾਂਕਿਤ ){
    testcase( p_ਵਸਤੂ->fg.is_ਸੂਚਕਾਂਕਿਤ_By );
    testcase( p_ਵਸਤੂ->fg.not_ਸੂਚਕਾਂਕਿਤ );
    ਵਾਪਸ 0;
  }
  iCur = p_ਵਸਤੂ->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  p_ਪਦ = where_ਜਾਂਚ_Init(&ਜਾਂਚ, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  ਜਦੋਂ( p_ਪਦ && p_ਪਦ->prereq_ਸੱਜਾ ) p_ਪਦ = where_ਜਾਂਚ_ਅਗਲਾ(&ਜਾਂਚ);
  ਜੇ( p_ਪਦ ){
    testcase( p_ਪਦ->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_ਕਾਲਮ_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = p_ਪਦ;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->r_ਲੜੀਵਾਰ = 33;  /* 33==sqlite3LogEst(10) */
  }ਨਹੀਂਤਾਂ{
    ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
      ਪੂਰਨ op_ਨਕਾਬ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->aLTermSpace==pLoop->aLTerm );
      ਜੇ( !Is_Unique_ਸੂਚਕਾਂਕ(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ>Array_ਮਾਪ(pLoop->aLTermSpace)
      ) ਜਾਰੀ;
      op_ਨਕਾਬ = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      ਲਈ(j=0; j<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; j++){
        p_ਪਦ = where_ਜਾਂਚ_Init(&ਜਾਂਚ, pWC, iCur, j, op_ਨਕਾਬ, pIdx);
        ਜਦੋਂ( p_ਪਦ && p_ਪਦ->prereq_ਸੱਜਾ ) p_ਪਦ = where_ਜਾਂਚ_ਅਗਲਾ(&ਜਾਂਚ);
        ਜੇ( p_ਪਦ==0 ) ਰੋਕੋ;
        testcase( p_ਪਦ->eOperator & WO_IS );
        pLoop->aLTerm[j] = p_ਪਦ;
      }
      ਜੇ( j!=pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ ) ਜਾਰੀ;
      pLoop->wsFlags = WHERE_ਕਾਲਮ_EQ|WHERE_ONEROW|WHERE_ਸੂਚਕਾਂਕਿਤ;
      ਜੇ( pIdx->isCovering || (p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ & pIdx->ਖਾਨਾ_Not_Idxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_ਸੂਚਕਾਂਕ = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->r_ਲੜੀਵਾਰ = 39;  /* 39==sqlite3LogEst(15) */
      ਰੋਕੋ;
    }
  }
  ਜੇ( pLoop->wsFlags ){
    pLoop->n_ਬਾਹਰਲਾ = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->s_ਨਕਾਬ_ਸੈੱਟ.n==1 && iCur==pWInfo->s_ਨਕਾਬ_ਸੈੱਟ.ix[0] );
    pLoop->ਨਕਾਬ_Self = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ = 1;
    ਜੇ( pWInfo->p_ਕ੍ਰਮ_By ) pWInfo->nOBSat =  pWInfo->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;
    ਜੇ( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    ਜੇ( ਜਾਂਚ.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    pLoop->cId = '0';
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
    ਜੇ( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#ਜੇ_ਅੰਤ
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** Helper function for exprIsDeterministic().
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਗੰਢ_Is_Deterministic(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_ConstFunc)==0 ){
    pWalker->e_ਕੋਡ = 0;
    ਵਾਪਸ WRC_Abort;
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Is_Deterministic(ਵਿਅੰਜਕ *p){
  Walker w;
  memset(&w, 0, ਆਕਾਰ(w));
  w.e_ਕੋਡ = 1;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਗੰਢ_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.e_ਕੋਡ;
}

 
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
ਸਥਿਰ ਖਾਲੀ showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  ਜੇ( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    ਪੂਰਨ i;
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    ਲਈ(p=pWInfo->pLoops, i=0; p; p=p->p_ਅਗਲਾ_Loop, i++){
      p->cId = zLabel[i%(ਆਕਾਰ(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#ਨਹੀਂਤਾਂ
# define WHERETRACE_ALL_LOOPS(W,C)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  ਪੂਰਨ i;
  Bitmask tab_ਵਰਤਿਆ;
  ਪੂਰਨ has_ਸੱਜਾ_Join;

  /* Preconditions checked by the caller */
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->nLevel>=2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->p_ਨਤੀਜਾ_ਸੈੱਟ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tab_ਵਰਤਿਆ = sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, pWInfo->p_ਨਤੀਜਾ_ਸੈੱਟ);
  ਜੇ( pWInfo->p_ਕ੍ਰਮ_By ){
    tab_ਵਰਤਿਆ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, pWInfo->p_ਕ੍ਰਮ_By);
  }
  has_ਸੱਜਾ_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  ਲਈ(i=pWInfo->nLevel-1; i>=1; i--){
    Where_ਪਦ *p_ਪਦ, *p_ਸਿਰਾ;
    Src_ਵਸਤੂ *p_ਵਸਤੂ;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    p_ਵਸਤੂ = &pWInfo->pTabList->a[pLoop->iTab];
    ਜੇ( (p_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_ਸੱਜਾ))!=JT_ਖੱਬਾ ) ਜਾਰੀ;
    ਜੇ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      ਜਾਰੀ;
    }
    ਜੇ( (tab_ਵਰਤਿਆ & pLoop->ਨਕਾਬ_Self)!=0 ) ਜਾਰੀ;
    p_ਸਿਰਾ = pWInfo->sWC.a + pWInfo->sWC.n_ਪਦ;
    ਲਈ(p_ਪਦ=pWInfo->sWC.a; p_ਪਦ<p_ਸਿਰਾ; p_ਪਦ++){
      ਜੇ( (p_ਪਦ->prereqAll & pLoop->ਨਕਾਬ_Self)!=0 ){
        ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON)
         || p_ਪਦ->p_ਵਿਅੰਜਕ->w.iJoin!=p_ਵਸਤੂ->iCursor
        ){
          ਰੋਕੋ;
        }
      }
      ਜੇ( has_ਸੱਜਾ_Join
       && ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_InnerON)
       && NEVER(p_ਪਦ->p_ਵਿਅੰਜਕ->w.iJoin==p_ਵਸਤੂ->iCursor)
      ){
        ਰੋਕੋ;  /* restriction (5) */
      }
    }
    ਜੇ( p_ਪਦ<p_ਸਿਰਾ ) ਜਾਰੀ;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->rev_ਨਕਾਬ>>1) & ~m1)!=0 );
    pWInfo->rev_ਨਕਾਬ = (m1 & pWInfo->rev_ਨਕਾਬ) | ((pWInfo->rev_ਨਕਾਬ>>1) & ~m1);
    notReady &= ~pLoop->ਨਕਾਬ_Self;
    ਲਈ(p_ਪਦ=pWInfo->sWC.a; p_ਪਦ<p_ਸਿਰਾ; p_ਪਦ++){
      ਜੇ( (p_ਪਦ->prereqAll & pLoop->ਨਕਾਬ_Self)!=0 ){
        p_ਪਦ->wtFlags |= ਪਦ_CODED;
        p_ਪਦ->prereqAll = 0;
      }
    }
    ਜੇ( i!=pWInfo->nLevel-1 ){
      ਪੂਰਨ nByte = (pWInfo->nLevel-1-i) * ਆਕਾਰ(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->nLevel>0 );
  }
  ਵਾਪਸ notReady;
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ whereCheckIfBloomFilterIsUseful(
  ਅਚਲ WhereInfo *pWInfo
){
  ਪੂਰਨ i;
  LogEst n_ਖੋਜ = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->nLevel>=2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  ਲਈ(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reqFlags = (WHERE_SELFCULL|WHERE_ਕਾਲਮ_EQ);
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pWInfo->pTabList->a[pLoop->iTab];
    ਸਾਰਣੀ *pTab = p_ਵਸਤੂ->pSTab;
    ਜੇ( (pTab->tabFlags & TF_HasStat1)==0 ) ਰੋਕੋ;
    pTab->tabFlags |= TF_MaybeReanalyze;
    ਜੇ( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_ਖੋਜ > pTab->n_ਪੰਕਤੀ_Log_Est
    ){
      testcase( p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (ਦੁੱਗਣਾ)sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(n_ਖੋਜ), pTab->zName,
         (ਦੁੱਗਣਾ)sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(pTab->n_ਪੰਕਤੀ_Log_Est)));
    }
    n_ਖੋਜ += pLoop->n_ਬਾਹਰਲਾ;
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ where_ਜੋੜੋ_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  ਸੂਚਕਾਂਕ *pIdx,       /* The index-on-expression that contains the expressions */
  ਪੂਰਨ iIdxCur,       /* Cursor number for pIdx */
  Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ  /* The FROM clause entry for the table */
){
  ਪੂਰਨ i;
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p;
  ਸਾਰਣੀ *pTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->b_Has_ਵਿਅੰਜਕ );
  pTab = pIdx->p_ਸਾਰਣੀ;
  ਲਈ(i=0; i<pIdx->n_ਕਾਲਮ; i++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
    ਪੂਰਨ j = pIdx->ai_ਕਾਲਮ[i];
    ਜੇ( j==XN_ਵਿਅੰਜਕ ){
      p_ਵਿਅੰਜਕ = pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ;
    }ਨਹੀਂਤਾਂ ਜੇ( j>=0 && (pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)!=0 ){
      p_ਵਿਅੰਜਕ = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pTab, &pTab->a_ਖਾਨਾ[j]);
    }ਨਹੀਂਤਾਂ{
      ਜਾਰੀ;
    }
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Constant(0,p_ਵਿਅੰਜਕ) ) ਜਾਰੀ;
    p = sqlite3DbMallocRaw(pParse->db,  ਆਕਾਰ(ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ));
    ਜੇ( p==0 ) ਰੋਕੋ;
    p->pIENext = pParse->pIdxEpr;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
    ਜੇ( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      ਜੇ( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_ਵਿਅੰਜਕ(p_ਵਿਅੰਜਕ);
    }
#ਜੇ_ਅੰਤ
    p->p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, p_ਵਿਅੰਜਕ, 0);
    p->i_ਅੰਕੜੇ_Cur = p_Tab_ਵਸਤੂ->iCursor;
    p->iIdxCur = iIdxCur;
    p->i_Idx_ਖਾਨਾ = i;
    p->b_Maybe_Null_ਪੰਕਤੀ = (p_Tab_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ))!=0;
    ਜੇ( sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->z_ਖਾਨਾ_Aff[i];
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#ਜੇ_ਅੰਤ
    pParse->pIdxEpr = p;
    ਜੇ( p->pIENext==0 ){
      ਖਾਲੀ *pArg = (ਖਾਲੀ*)&pParse->pIdxEpr;
      sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, where_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Cleanup, pArg);
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ where_ਉਲਟ_ਜਾਂਚ_ਕ੍ਰਮ(WhereInfo *pWInfo){
  ਪੂਰਨ ii;
  ਲਈ(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pWInfo->pTabList->a[ii];
    ਜੇ( !p_ਵਸਤੂ->fg.isCte
     || p_ਵਸਤੂ->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(p_ਵਸਤੂ->fg.isSubquery==0)
     || p_ਵਸਤੂ->u4.pSubq->pSelect->p_ਕ੍ਰਮ_By==0
    ){
      pWInfo->rev_ਨਕਾਬ |= MASKBIT(ii);
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
  ਵਿਅੰਜਕ *pWhere,           /* The WHERE clause */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  ਵਿਅੰਜਕ_List *p_ਨਤੀਜਾ_ਸੈੱਟ,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  ਪੂਰਨ iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  ਪੂਰਨ nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  ਪੂਰਨ nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  ਪੂਰਨ ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  ਪੂਰਨ rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  ਪ੍ਰਤਿਜ੍ਞਾ( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  ਪ੍ਰਤਿਜ੍ਞਾ( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_ਹੱਦਬੰਦੀ)==0 );

  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, ਆਕਾਰ(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( p_ਕ੍ਰਮ_By && p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ==BMS-1 );
  ਜੇ( p_ਕ੍ਰਮ_By && p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ>=BMS ){
    p_ਕ੍ਰਮ_By = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  ਜੇ( pTabList->nSrc>BMS ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "at most %d tables in a join", BMS);
    ਵਾਪਸ 0;
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
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + ਆਕਾਰ(WhereLoop));
  ਜੇ( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    ਜਾਓ whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
#ਜੇ WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#ਜੇ_ਅੰਤ
  pWInfo->p_ਨਤੀਜਾ_ਸੈੱਟ = p_ਨਤੀਜਾ_ਸੈੱਟ;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->i_ਹੱਦਬੰਦੀ = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  memset(&pWInfo->a[0], 0, ਆਕਾਰ(WhereLoop)+nTabList*ਆਕਾਰ(WhereLevel));
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  p_ਨਕਾਬ_ਸੈੱਟ = &pWInfo->s_ਨਕਾਬ_ਸੈੱਟ;
  p_ਨਕਾਬ_ਸੈੱਟ->n = 0;
  p_ਨਕਾਬ_ਸੈੱਟ->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((ਅੱਖਰ*)pWInfo)+nByteWInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#ਜੇ_ਅੰਤ

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3_Where_ਵੰਡੋ(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  ਜੇ( nTabList==0 ){
    ਜੇ( p_ਕ੍ਰਮ_By ) pWInfo->nOBSat = p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;
    ਜੇ( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    ਜੇ( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_ਮੁੱਲ)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }ਨਹੀਂਤਾਂ{
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
    ਕਰੋ{
      create_ਨਕਾਬ(p_ਨਕਾਬ_ਸੈੱਟ, pTabList->a[ii].iCursor);
      sqlite3_Where_Tab_Func_ਦਲੀਲਾਂ(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }ਜਦੋਂ( (++ii)<pTabList->nSrc );
  #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    {
      Bitmask mx = 0;
      ਲਈ(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3_Where_Get_ਨਕਾਬ(p_ਨਕਾਬ_ਸੈੱਟ, pTabList->a[ii].iCursor);
        ਪ੍ਰਤਿਜ੍ਞਾ( m>=mx );
        mx = m;
      }
    }
  #ਜੇ_ਅੰਤ
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_ਵਿਅੰਜਕ_Analyze(pTabList, &pWInfo->sWC);
  ਜੇ( pSelect && pSelect->p_ਹੱਦਬੰਦੀ ){
    sqlite3_Where_ਜੋੜੋ_ਹੱਦਬੰਦੀ(&pWInfo->sWC, pSelect);
  }
  ਜੇ( pParse->nErr ) ਜਾਓ whereBeginError;

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
  ਲਈ(ii=0; ii<sWLB.pWC->n_ਅਧਾਰ; ii++){
    Where_ਪਦ *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    ਵਿਅੰਜਕ *pX;                          /* The expression of pT */
    ਜੇ( pT->wtFlags & ਪਦ_VIRTUAL ) ਜਾਰੀ;
    pX = pT->p_ਵਿਅੰਜਕ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pX!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pT->prereqAll!=0 || !ਵਿਅੰਜਕ_Has_Property(pX, EP_OuterON) );
    ਜੇ( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || ਵਿਅੰਜਕ_Is_Deterministic(pX))   /* Condition (4) */
     && !(ਵਿਅੰਜਕ_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= ਪਦ_CODED;
    }
  }

  ਜੇ( wctrlFlags & WHERE_WANT_DISTINCT ){
    ਜੇ( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }ਨਹੀਂਤਾਂ ਜੇ( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_ਨਤੀਜਾ_ਸੈੱਟ) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਕ੍ਰਮ_By==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->p_ਕ੍ਰਮ_By = p_ਨਤੀਜਾ_ਸੈੱਟ;
    }
  }

  /* Construct the WhereLoop objects */
#ਜੇ defined(WHERETRACE_ENABLED)
  ਜੇ( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    ਜੇ( wctrlFlags & WHERE_USE_ਹੱਦਬੰਦੀ ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    ਜੇ( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      memset(&sSelect, 0, ਆਕਾਰ(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
      sSelect.pEList = p_ਨਤੀਜਾ_ਸੈੱਟ;
      sqlite3_ਰੁੱਖ_View_Select(0, &sSelect, 0);
    }
    ਜੇ( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#ਜੇ_ਅੰਤ

  ਜੇ( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = where_Loop_ਜੋੜੋ_All(&sWLB);
    ਜੇ( rc ) ਜਾਓ whereBeginError;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    ਜੇ( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      ਜਦੋਂ( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_ਅਗਲਾ_Loop;
        whereLoopDelete(db, p);
      }
      rc = where_Loop_ਜੋੜੋ_All(&sWLB);
      ਜੇ( rc ) ਜਾਓ whereBeginError;
    }
#ਜੇ_ਅੰਤ
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    where_ਰਸਤਾ_Solver(pWInfo, 0);
    ਜੇ( db->mallocFailed ) ਜਾਓ whereBeginError;
    ਜੇ( pWInfo->p_ਕ੍ਰਮ_By ){
       whereInterstageHeuristic(pWInfo);
       where_ਰਸਤਾ_Solver(pWInfo, pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ<0 ? 1 : pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ+1);
       ਜੇ( db->mallocFailed ) ਜਾਓ whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    ਜੇ( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ, pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ-30));
      pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ -= 30;
    }

  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pTabList!=0 );
  ਜੇ( pWInfo->p_ਕ੍ਰਮ_By==0 && (db->flags & SQLITE_ਉਲਟ_ਕ੍ਰਮ)!=0 ){
    where_ਉਲਟ_ਜਾਂਚ_ਕ੍ਰਮ(pWInfo);
  }
  ਜੇ( pParse->nErr ){
    ਜਾਓ whereBeginError;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  ਜੇ( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_ਕੁੱਲ_ਲਾਗਤ, pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ);
    ਜੇ( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->rev_ਨਕਾਬ);
    }
    ਚੋਣ( pWInfo->eDistinct ){
      ਹਾਲਤ WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        ਰੋਕੋ;
      }
      ਹਾਲਤ WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        ਰੋਕੋ;
      }
      ਹਾਲਤ WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        ਰੋਕੋ;
      }
    }
    sqlite3DebugPrintf("\n");
    ਲਈ(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#ਜੇ_ਅੰਤ

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
  ਜੇ( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && p_ਨਤੀਜਾ_ਸੈੱਟ!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    ਪ੍ਰਤਿਜ੍ਞਾ( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  ਜੇ( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#ਜੇ defined(WHERETRACE_ENABLED)
  ਜੇ( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
    sqlite3DebugPrintf("---- WHERE clause at end of analysis:\n");
    sqlite3WhereClausePrint(sWLB.pWC);
  }
  WHERETRACE(0xffffffff,("*** Optimizer Finished ***\n"));
#ਜੇ_ਅੰਤ
  pWInfo->pParse->nQueryLoop += pWInfo->n_ਪੰਕਤੀ_ਬਾਹਰਲਾ;

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
  ਪ੍ਰਤਿਜ੍ਞਾ( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  ਜੇ( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    ਪੂਰਨ wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    ਪੂਰਨ bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    ਪ੍ਰਤਿਜ੍ਞਾ( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    ਜੇ( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      ਜੇ( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        ਜੇ( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  ਲਈ(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    ਸਾਰਣੀ *pTab;     /* Table to open */
    ਪੂਰਨ iDb;         /* Index of database containing table/index */
    Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ;

    p_Tab_ਵਸਤੂ = &pTabList->a[pLevel->iFrom];
    pTab = p_Tab_ਵਸਤੂ->pSTab;
    iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    ਜੇ( ii==0 || (p_Tab_ਵਸਤੂ[0].fg.jointype & JT_ਖੱਬਾ)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }ਨਹੀਂਤਾਂ ਜੇ( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }ਨਹੀਂਤਾਂ{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    ਜੇ( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਜੇ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ਅਚਲ ਅੱਖਰ *pVTab = (ਅਚਲ ਅੱਖਰ *)sqlite3GetVTable(db, pTab);
      ਪੂਰਨ iCur = p_Tab_ਵਸਤੂ->iCursor;
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }ਨਹੀਂਤਾਂ ਜੇ( IsVirtual(pTab) ){
      /* noop */
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    ਜੇ( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (p_Tab_ਵਸਤੂ->fg.jointype & (JT_LTORJ|JT_ਸੱਜਾ))!=0
    ){
      ਪੂਰਨ op = OP_OpenRead;
      ਜੇ( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = p_Tab_ਵਸਤੂ->iCursor;
      };
      sqlite3_Open_ਸਾਰਣੀ(pParse, p_Tab_ਵਸਤੂ->iCursor, iDb, pTab, op);
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Tab_ਵਸਤੂ->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_ਖਾਨਾ==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->n_ਖਾਨਾ==BMS );
      ਜੇ( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->n_ਖਾਨਾ<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_ਸੂਚਕਾਂਕ|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = p_Tab_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ;
        ਪੂਰਨ n = 0;
        ਲਈ(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(n), P4_INT32);
        ਪ੍ਰਤਿਜ੍ਞਾ( n<=pTab->n_ਖਾਨਾ );
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
      ਜੇ( pLoop->u.btree.p_ਸੂਚਕਾਂਕ!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_ਵਰਤਿਆ_ਨਕਾਬ
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Columns_ਵਰਤਿਆ, p_Tab_ਵਸਤੂ->iCursor,
                    LOWER32(p_Tab_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ), UPPER32(p_Tab_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ));
#ਜੇ_ਅੰਤ
      ਜੇ( ii>=2
       && (p_Tab_ਵਸਤੂ[0].fg.jointype & (JT_LTORJ|JT_ਖੱਬਾ))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_If_ਖਾਲੀ_ਥਾਵਾਂ, p_Tab_ਵਸਤੂ->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    ਜੇ( pLoop->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ ){
      ਸੂਚਕਾਂਕ *pIx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
      ਪੂਰਨ i_ਸੂਚਕਾਂਕ_Cur;
      ਪੂਰਨ op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      ਪ੍ਰਤਿਜ੍ਞਾ( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      ਜੇ( !HasRowid(pTab) && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_ਸੂਚਕਾਂਕ_Cur = pLevel->iTabCur;
        op = 0;
      }ਨਹੀਂਤਾਂ ਜੇ( pWInfo->eOnePass!=ONEPASS_OFF ){
        ਸੂਚਕਾਂਕ *pJ = p_Tab_ਵਸਤੂ->pSTab->p_ਸੂਚਕਾਂਕ;
        i_ਸੂਚਕਾਂਕ_Cur = iAuxArg;
        ਪ੍ਰਤਿਜ੍ਞਾ( wctrlFlags & WHERE_ONEPASS_DESIRED );
        ਜਦੋਂ( ALWAYS(pJ) && pJ!=pIx ){
          i_ਸੂਚਕਾਂਕ_Cur++;
          pJ = pJ->p_ਅਗਲਾ;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_ਸੂਚਕਾਂਕ_Cur;
      }ਨਹੀਂਤਾਂ ਜੇ( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_ਸੂਚਕਾਂਕ_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }ਨਹੀਂਤਾਂ{
        i_ਸੂਚਕਾਂਕ_Cur = pParse->nTab++;
        ਜੇ( pIx->b_Has_ਵਿਅੰਜਕ && OptimizationEnabled(db, SQLITE_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ) ){
          where_ਜੋੜੋ_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ(pParse, pIx, i_ਸੂਚਕਾਂਕ_Cur, p_Tab_ਵਸਤੂ);
        }
        ਜੇ( pIx->pPartIdxWhere && (p_Tab_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ)==0 ){
          where_Part_Idx_ਵਿਅੰਜਕ(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_ਸੂਚਕਾਂਕ_Cur, p_Tab_ਵਸਤੂ
          );
        }
      }
      pLevel->iIdxCur = i_ਸੂਚਕਾਂਕ_Cur;
      ਪ੍ਰਤਿਜ੍ਞਾ( pIx!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pIx->pSchema==pTab->pSchema );
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਸੂਚਕਾਂਕ_Cur>=0 );
      ਜੇ( op ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, op, i_ਸੂਚਕਾਂਕ_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIx);
        ਜੇ( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_ਕਾਲਮ_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_ਛਾਂਟੋ)==0
         && (pLoop->wsFlags & WHERE_ਵਿੱਚ_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_ਵਰਤਿਆ_ਨਕਾਬ
        {
          u64 ਖਾਨਾ_ਵਰਤਿਆ = 0;
          ਪੂਰਨ ii, jj;
          ਲਈ(ii=0; ii<pIx->n_ਕਾਲਮ; ii++){
            jj = pIx->ai_ਕਾਲਮ[ii];
            ਜੇ( jj<0 ) ਜਾਰੀ;
            ਜੇ( jj>63 ) jj = 63;
            ਜੇ( (p_Tab_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ & MASKBIT(jj))==0 ) ਜਾਰੀ;
            ਖਾਨਾ_ਵਰਤਿਆ |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Columns_ਵਰਤਿਆ, i_ਸੂਚਕਾਂਕ_Cur,
                            LOWER32(ਖਾਨਾ_ਵਰਤਿਆ),UPPER32(ਖਾਨਾ_ਵਰਤਿਆ));
        }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_COLUMN_USED_MASK */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
        ਜੇ( HasRowid(pTab) ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_CursorHint, i_ਸੂਚਕਾਂਕ_Cur, 0, p_Tab_ਵਸਤੂ->iCursor);
        }
#ਜੇ_ਅੰਤ
      }
    }
    ਜੇ( iDb>=0 ) sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
    ਜੇ( (p_Tab_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, ਆਕਾਰ(Where_ਸੱਜਾ_Join)))!=0
    ){
      Where_ਸੱਜਾ_Join *pRJ = pLevel->pRJ;
      ਪੂਰਨ bBloomable = 0;
      pRJ->i_ਮੇਲ = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, pRJ->regReturn);
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab==p_Tab_ਵਸਤੂ->pSTab );
      ਜੇ( HasRowid(pTab) ){
        ਕੁੰਜੀ_Info *pInfo;
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, pRJ->i_ਮੇਲ, 1);
        bBloomable = 1;
        pInfo = sqlite3_ਕੁੰਜੀ_Info_Alloc(pParse->db, 1, 0);
        ਜੇ( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->a_ਛਾਂਟੋ_Flags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }ਨਹੀਂਤਾਂ{
        ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, pRJ->i_ਮੇਲ, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
        sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pPk);
        bBloomable = sqlite3_ਸੂਚਕਾਂਕ_Bloomable(pPk,pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
      }
      ਜੇ( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Blob, 65536, pRJ->regBloom);
      }ਨਹੀਂਤਾਂ{
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
  pWInfo->i_ਸਿਖਰ = sqlite3VdbeCurrentAddr(v);
  ਜੇ( db->mallocFailed ) ਜਾਓ whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  ਲਈ(ii=0; ii<nTabList; ii++){
    ਪੂਰਨ addrExplain;
    ਪੂਰਨ wsFlags;
    Src_ਵਸਤੂ *pSrc;
    ਜੇ( pParse->nErr ) ਜਾਓ whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    ਜੇ( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      ਪੂਰਨ iOnce = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      ਜੇ( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
      }ਨਹੀਂਤਾਂ{
        iOnce = 0;
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      ਜੇ( iOnce )  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, iOnce);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pTabList == pWInfo->pTabList );
    ਜੇ( (wsFlags & (WHERE_AUTO_ਸੂਚਕਾਂਕ|WHERE_BLOOMFILTER))!=0 ){
      ਜੇ( (wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)!=0 ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOMATIC_ਸੂਚਕਾਂਕ
        construct_Automatic_ਸੂਚਕਾਂਕ(pParse, &pWInfo->sWC, notReady, pLevel);
#ਜੇ_ਅੰਤ
      }ਨਹੀਂਤਾਂ{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      ਜੇ( db->mallocFailed ) ਜਾਓ whereBeginError;
    }
    addrExplain = sqlite3_Where_Explain_One_ਜਾਂਚ(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3_Where_ਕੋਡ_One_Loop_ਸ਼ੁਰੂ(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    ਜੇ( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3_Where_ਜੋੜੋ_ਜਾਂਚ_ਸਥਿਤੀ(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->i_ਸਿਰਾ_Where = sqlite3VdbeCurrentAddr(v);
  ਵਾਪਸ pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  ਜੇ( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#ਜੇ_ਅੰਤ /* WHERETRACE_ENABLED */
  ਵਾਪਸ 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#ਨਹੀਂਤਾਂ
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  ਸਥਿਰ ਖਾਲੀ sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    ਪੂਰਨ pc,
    VdbeOp *pOp
  ){
    ਜੇ( (db->flags & SQLITE_VdbeAddopTrace)==0 ) ਵਾਪਸ;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3_Show_Where_ਪਦ(0); /* So compiler won't complain about unused func */
  }
#ਜੇ_ਅੰਤ

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
ਖਾਲੀ sqlite3_Where_ਸਿਰਾ(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ i_ਸਿਰਾ = sqlite3VdbeCurrentAddr(v);
  ਪੂਰਨ nRJ = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  ਪੂਰਨ addrSeek = 0;
#ਜੇ_ਅੰਤ

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  ਲਈ(i=pWInfo->nLevel-1; i>=0; i--){
    ਪੂਰਨ addr;
    pLevel = &pWInfo->a[i];
    ਜੇ( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_ਸੱਜਾ_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->ਸਿਰਾ_Subrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    ਜੇ( pLevel->op!=OP_Noop ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      ਸੂਚਕਾਂਕ *pIdx;
      ਪੂਰਨ n;
      ਜੇ( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)!=0
       && (pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ)->hasStat1
       && (n = pLoop->u.btree.n_Distinct_ਖਾਨਾ)>0
       && pIdx->ai_ਪੰਕਤੀ_Log_Est[n]>=36
      ){
        ਪੂਰਨ r1 = pParse->nMem+1;
        ਪੂਰਨ j, op;
        ਪੂਰਨ addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        ਜੇ( pLevel->i_ਖੱਬਾ_Join ){
          addrIfNull = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_If_Null_ਪੰਕਤੀ, pLevel->iIdxCur, r1);
        }
        ਲਈ(j=0; j<n; j++){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_Prev ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 1, pLevel->p2);
        ਜੇ( pLevel->i_ਖੱਬਾ_Join ){
          sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrIfNull);
        }
      }
#ਜੇ_ਅੰਤ /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    ਜੇ( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }ਨਹੀਂਤਾਂ ਜੇ( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      ਪੂਰਨ iBypass = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
      ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    ਜੇ( pLevel->op!=OP_Noop ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_ਅਗਲਾ);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      ਜੇ( addrSeek ){
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrSeek);
        addrSeek = 0;
      }
#ਜੇ_ਅੰਤ
      ਜੇ( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    ਜੇ( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
    ਪ੍ਰਤਿਜ੍ਞਾ( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_ਵਿੱਚ_ABLE) && pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    ਜੇ( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_DecrJumpZero, (ਪੂਰਨ)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( (pLoop->wsFlags & WHERE_ਵਿੱਚ_ABLE)!=0 && pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ>0 ){
      ਬਣਤਰ ਵਿੱਚ_Loop *p_ਵਿੱਚ;
      ਪੂਰਨ j;
      ਲਈ(j=pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ, p_ਵਿੱਚ=&pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop[j-1]; j>0; j--, p_ਵਿੱਚ--){
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeGetOp(v, p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ+1);
        ਜੇ( p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op!=OP_Noop ){
          ਜੇ( p_ਵਿੱਚ->n_ਅਗੇਤਰ ){
            ਪੂਰਨ b_Early_ਬਾਹਰਲਾ =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_ਵਿੱਚ_EARLYOUT)!=0;
            ਜੇ( pLevel->i_ਖੱਬਾ_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_If_Null_ਪੰਕਤੀ, p_ਵਿੱਚ->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + b_Early_ਬਾਹਰਲਾ, p_ਵਿੱਚ->i_ਅਧਾਰ);
              VdbeCoverage(v);
            }
            ਜੇ( b_Early_ਬਾਹਰਲਾ ){
              sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  p_ਵਿੱਚ->i_ਅਧਾਰ, p_ਵਿੱਚ->n_ਅਗੇਤਰ);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3_Vdbe_Jump_ਇੱਥੇ(v, p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ+1);
            }
          }
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op, p_ਵਿੱਚ->iCur, p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ);
          VdbeCoverage(v);
          VdbeCoverageIf(v, p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op==OP_Prev);
          VdbeCoverageIf(v, p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op==OP_ਅਗਲਾ);
        }
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    ਜੇ( pLevel->pRJ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    ਜੇ( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_ਸੂਚਕਾਂਕ->zName));
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, pLevel->addrSkip);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, pLevel->addrSkip-2);
    }
    ਜੇ( pLevel->i_ਖੱਬਾ_Join ){
      ਪੂਰਨ ws = pLoop->wsFlags;
      addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IfPos, pLevel->i_ਖੱਬਾ_Join); VdbeCoverage(v);
      ਪ੍ਰਤਿਜ੍ਞਾ( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_ਸੂਚਕਾਂਕਿਤ)!=0 );
      ਜੇ( (ws & WHERE_IDX_ONLY)==0 ){
        Src_ਵਸਤੂ *pSrc = &pTabList->a[pLevel->iFrom];
        ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->iTabCur==pSrc->iCursor );
        ਜੇ( pSrc->fg.ਰਾਹੀਂ_Coroutine ){
          ਪੂਰਨ m, n;
          ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_ਨਤੀਜਾ;
          ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->pSTab!=0 );
          m = pSrc->pSTab->n_ਖਾਨਾ;
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, pLevel->iTabCur);
      }
      ਜੇ( (ws & WHERE_ਸੂਚਕਾਂਕਿਤ)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        ਜੇ( ws & WHERE_MULTI_OR ){
          ਸੂਚਕਾਂਕ *pIx = pLevel->u.pCoveringIdx;
          ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pIx->pSchema);
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIx);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, pLevel->iIdxCur);
      }
      ਜੇ( ws & WHERE_ਵਿੱਚ_ABLE ){
        ਪੂਰਨ m;
        ਲਈ(m=0; m<pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ; m++){
          ਬਣਤਰ ਵਿੱਚ_Loop *p_ਵਿੱਚ = &pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop[m];
          ਜੇ( p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op!=OP_Noop ){
            sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, p_ਵਿੱਚ->iCur);
          }
        }
      }
      ਜੇ( pLevel->op==OP_Return ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, pLevel->p1, pLevel->addr_ਪਹਿਲਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3VdbeGoto(v, pLevel->addr_ਪਹਿਲਾ);
      }
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->nLevel<=pTabList->nSrc );
  ਲਈ(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    ਪੂਰਨ k, ਆਖਰੀ_2;
    VdbeOp *pOp, *p_ਆਖਰੀ_Op;
    ਸੂਚਕਾਂਕ *pIdx = 0;
    Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ = &pTabList->a[pLevel->iFrom];
    ਸਾਰਣੀ *pTab = p_Tab_ਵਸਤੂ->pSTab;
    ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    ਜੇ( pLevel->pRJ ){
      sqlite3_Where_ਸੱਜਾ_Join_Loop(pWInfo, i, pLevel);
      ਜਾਰੀ;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    ਜੇ( p_Tab_ਵਸਤੂ->fg.ਰਾਹੀਂ_Coroutine ){
      testcase( pParse->db->mallocFailed );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Tab_ਵਸਤੂ->fg.isSubquery );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Tab_ਵਸਤੂ->u4.pSubq->reg_ਨਤੀਜਾ>=0 );
      translate_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Copy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            p_Tab_ਵਸਤੂ->u4.pSubq->reg_ਨਤੀਜਾ, 0);
      ਜਾਰੀ;
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
    ਜੇ( pLoop->wsFlags & (WHERE_ਸੂਚਕਾਂਕਿਤ|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
    }ਨਹੀਂਤਾਂ ਜੇ( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    ਜੇ( pIdx
     && !db->mallocFailed
    ){
      ਜੇ( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->p_ਸਾਰਣੀ) ){
        ਆਖਰੀ_2 = i_ਸਿਰਾ;
      }ਨਹੀਂਤਾਂ{
        ਆਖਰੀ_2 = pWInfo->i_ਸਿਰਾ_Where;
      }
      ਜੇ( pIdx->b_Has_ਵਿਅੰਜਕ ){
        ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p = pParse->pIdxEpr;
        ਜਦੋਂ( p ){
          ਜੇ( p->iIdxCur==pLevel->iIdxCur ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
            ਜੇ( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->i_Idx_ਖਾਨਾ);
              ਜੇ( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_ਵਿਅੰਜਕ(p->p_ਵਿਅੰਜਕ);
            }
#ਜੇ_ਅੰਤ
            p->i_ਅੰਕੜੇ_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      ਜੇ( db->flags & SQLITE_VdbeAddopTrace ){
        ਲਿਖੋ("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, ਆਖਰੀ_2-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode!=OP_ਕਾਲਮ || pOp->p1!=pLevel->iTabCur );
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode!=OP_If_Null_ਪੰਕਤੀ || pOp->p1!=pLevel->iTabCur );
#ਜੇ_ਅੰਤ
      pOp = sqlite3VdbeGetOp(v, k);
      p_ਆਖਰੀ_Op = pOp + (ਆਖਰੀ_2 - k);
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp<=p_ਆਖਰੀ_Op );
      ਕਰੋ{
        ਜੇ( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }ਨਹੀਂਤਾਂ ਜੇ( pOp->opcode==OP_ਕਾਲਮ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਵਿੱਥ_SQL_FUNC
         || pOp->opcode==OP_ਵਿੱਥ
#ਜੇ_ਅੰਤ
        ){
          ਪੂਰਨ x = pOp->p2;
          ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->p_ਸਾਰਣੀ==pTab );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਵਿੱਥ_SQL_FUNC
          ਜੇ( pOp->opcode==OP_ਵਿੱਥ ){
            /* Do not need to translate the column number */
          }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
          ਜੇ( !HasRowid(pTab) ){
            ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
            x = pPk->ai_ਕਾਲਮ[x];
            ਪ੍ਰਤਿਜ੍ਞਾ( x>=0 );
          }ਨਹੀਂਤਾਂ{
            testcase( x!=sqlite3_Storage_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸਾਰਣੀ(pTab,x) );
            x = sqlite3_Storage_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸਾਰਣੀ(pTab,x);
          }
          x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pIdx, x);
          ਜੇ( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }ਨਹੀਂਤਾਂ ਜੇ( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            ਜੇ( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_ਸੁਨੇਹਾ(pParse, "internal query planner error");
              pParse->rc = SQLITE_ਅੰਦਰੂਨੀ;
            }ਨਹੀਂਤਾਂ{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3_Where_ਜੋੜੋ_Explain_ਪਾਠ(pParse,
                  pLevel->addrBody-1,
                  pTabList,
                  pLevel,
                  pWInfo->wctrlFlags
              );
            }
          }
        }ਨਹੀਂਤਾਂ ਜੇ( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }ਨਹੀਂਤਾਂ ਜੇ( pOp->opcode==OP_If_Null_ਪੰਕਤੀ ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
        k++;
#ਜੇ_ਅੰਤ
      }ਜਦੋਂ( (++pOp)<p_ਆਖਰੀ_Op );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      ਜੇ( db->flags & SQLITE_VdbeAddopTrace ) ਲਿਖੋ("TRANSLATE complete\n");
#ਜੇ_ਅੰਤ
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
  ਵਾਪਸ;
}
