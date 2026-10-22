/*
** 2018-04-12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code to implement various aspects of UPSERT
** processing and handling of the Upsert object.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  ਕਰੋ{
    Upsert *p_ਅਗਲਾ = p->p_ਅਗਲਾ_Upsert;
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p->p_Upsert_ਨਿਸ਼ਾਨਾ);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p->p_Upsert_ਨਿਸ਼ਾਨਾ_Where);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p->p_Upsert_ਸੈੱਟ);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_ਮੰਜ਼ਿਲ_Free);
    sqlite3DbFree(db, p);
    p = p_ਅਗਲਾ;
  }ਜਦੋਂ( p );
}
ਖਾਲੀ sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  ਜੇ( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ sqlite3UpsertNew(db,
           sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->p_Upsert_ਨਿਸ਼ਾਨਾ, 0),
           sqlite3_ਵਿਅੰਜਕ_Dup(db, p->p_Upsert_ਨਿਸ਼ਾਨਾ_Where, 0),
           sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->p_Upsert_ਸੈੱਟ, 0),
           sqlite3_ਵਿਅੰਜਕ_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_ਅਗਲਾ_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  ਵਿਅੰਜਕ_List *p_ਨਿਸ਼ਾਨਾ,     /* Target argument to ON CONFLICT, or NULL */
  ਵਿਅੰਜਕ *p_ਨਿਸ਼ਾਨਾ_Where,    /* Optional WHERE clause on the target */
  ਵਿਅੰਜਕ_List *p_ਸੈੱਟ,        /* UPDATE columns, or NULL for a DO NOTHING */
  ਵਿਅੰਜਕ *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_ਅਗਲਾ          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, ਆਕਾਰ(Upsert));
  ਜੇ( pNew==0 ){
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਨਿਸ਼ਾਨਾ);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਨਿਸ਼ਾਨਾ_Where);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਸੈੱਟ);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_ਅਗਲਾ);
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    pNew->p_Upsert_ਨਿਸ਼ਾਨਾ = p_ਨਿਸ਼ਾਨਾ;
    pNew->p_Upsert_ਨਿਸ਼ਾਨਾ_Where = p_ਨਿਸ਼ਾਨਾ_Where;
    pNew->p_Upsert_ਸੈੱਟ = p_ਸੈੱਟ;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_ਸੈੱਟ!=0;
    pNew->p_ਅਗਲਾ_Upsert = p_ਅਗਲਾ;
  }
  ਵਾਪਸ pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
ਪੂਰਨ sqlite3_Upsert_Analyze_ਨਿਸ਼ਾਨਾ(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  ਸਾਰਣੀ *pTab;            /* That table into which we are inserting */
  ਪੂਰਨ rc;                 /* Result code */
  ਪੂਰਨ iCursor;            /* Cursor used by pTab */
  ਸੂਚਕਾਂਕ *pIdx;            /* One of the indexes of pTab */
  ਵਿਅੰਜਕ_List *p_ਨਿਸ਼ਾਨਾ;      /* The conflict-target clause */
  ਵਿਅੰਜਕ *p_ਪਦ;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  ਵਿਅੰਜਕ s_ਖਾਨਾ[2];           /* Index column converted into an Expr */
  ਪੂਰਨ nClause = 0;        /* Counter of ON CONFLICT clauses */

  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList->nSrc==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList->a[0].pSTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pUpsert!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, ਆਕਾਰ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ਲਈ(; pUpsert && pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ;
        pUpsert=pUpsert->p_ਅਗਲਾ_Upsert, nClause++){
    rc = sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ);
    ਜੇ( rc ) ਵਾਪਸ rc;
    rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ_Where);
    ਜੇ( rc ) ਵਾਪਸ rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_ਨਿਸ਼ਾਨਾ = pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ;
    iCursor = pTabList->a[0].iCursor;
    ਜੇ( HasRowid(pTab) 
     && p_ਨਿਸ਼ਾਨਾ->n_ਵਿਅੰਜਕ==1
     && (p_ਪਦ = p_ਨਿਸ਼ਾਨਾ->a[0].p_ਵਿਅੰਜਕ)->op==TK_ਕਾਲਮ
     && p_ਪਦ->i_ਕਾਲਮ==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      ਪ੍ਰਤਿਜ੍ਞਾ( pUpsert->pUpsertIdx==0 );
      ਜਾਰੀ;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(s_ਖਾਨਾ, 0, ਆਕਾਰ(s_ਖਾਨਾ));
    s_ਖਾਨਾ[0].op = TK_COLLATE;
    s_ਖਾਨਾ[0].p_ਖੱਬਾ = &s_ਖਾਨਾ[1];
    s_ਖਾਨਾ[1].op = TK_ਕਾਲਮ;
    s_ਖਾਨਾ[1].i_ਸਾਰਣੀ = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
      ਪੂਰਨ ii, jj, nn;
      ਜੇ( !Is_Unique_ਸੂਚਕਾਂਕ(pIdx) ) ਜਾਰੀ;
      ਜੇ( p_ਨਿਸ਼ਾਨਾ->n_ਵਿਅੰਜਕ!=pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ ) ਜਾਰੀ;
      ਜੇ( pIdx->pPartIdxWhere ){
        ਜੇ( pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ_Where==0 ) ਜਾਰੀ;
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(pParse, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          ਜਾਰੀ;
        }
      }
      nn = pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ;
      ਲਈ(ii=0; ii<nn; ii++){
        ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
        s_ਖਾਨਾ[0].u.zToken = (ਅੱਖਰ*)pIdx->azColl[ii];
        ਜੇ( pIdx->ai_ਕਾਲਮ[ii]==XN_ਵਿਅੰਜਕ ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ!=0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->n_ਵਿਅੰਜਕ>ii );
          ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->b_Has_ਵਿਅੰਜਕ );
          p_ਵਿਅੰਜਕ = pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[ii].p_ਵਿਅੰਜਕ;
          ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_COLLATE ){
            s_ਖਾਨਾ[0].p_ਖੱਬਾ = p_ਵਿਅੰਜਕ;
            p_ਵਿਅੰਜਕ = &s_ਖਾਨਾ[0];
          }
        }ਨਹੀਂਤਾਂ{
          s_ਖਾਨਾ[0].p_ਖੱਬਾ = &s_ਖਾਨਾ[1];
          s_ਖਾਨਾ[1].i_ਕਾਲਮ = pIdx->ai_ਕਾਲਮ[ii];
          p_ਵਿਅੰਜਕ = &s_ਖਾਨਾ[0];
        }
        ਲਈ(jj=0; jj<nn; jj++){
          ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0,p_ਨਿਸ਼ਾਨਾ->a[jj].p_ਵਿਅੰਜਕ,p_ਵਿਅੰਜਕ,iCursor)<2 ){
            ਰੋਕੋ;  /* Column ii of the index matches column jj of target */
          }
        }
        ਜੇ( jj>=nn ){
          /* The target contains no match for column jj of the index */
          ਰੋਕੋ;
        }
      }
      ਜੇ( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        ਜਾਰੀ;
      }
      pUpsert->pUpsertIdx = pIdx;
      ਜੇ( sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      ਰੋਕੋ;
    }
    ਜੇ( pUpsert->pUpsertIdx==0 ){
      ਅੱਖਰ zWhich[16];
      ਜੇ( nClause==0 && pUpsert->p_ਅਗਲਾ_Upsert==0 ){
        zWhich[0] = 0;
      }ਨਹੀਂਤਾਂ{
        sqlite3_snprintf(ਆਕਾਰ(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      ਵਾਪਸ SQLITE_ERROR;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
ਪੂਰਨ sqlite3_Upsert_ਅਗਲਾ_Is_IPK(Upsert *pUpsert){
  Upsert *p_ਅਗਲਾ;
  ਜੇ( NEVER(pUpsert==0) ) ਵਾਪਸ 0;
  p_ਅਗਲਾ = pUpsert->p_ਅਗਲਾ_Upsert;
  ਜਦੋਂ( 1 /*exit-by-return*/ ){
    ਜੇ( p_ਅਗਲਾ==0 ) ਵਾਪਸ 1;
    ਜੇ( p_ਅਗਲਾ->p_Upsert_ਨਿਸ਼ਾਨਾ==0 ) ਵਾਪਸ 1;
    ਜੇ( p_ਅਗਲਾ->pUpsertIdx==0 ) ਵਾਪਸ 1;
    ਜੇ( !p_ਅਗਲਾ->isDup ) ਵਾਪਸ 0;
    p_ਅਗਲਾ = p_ਅਗਲਾ->p_ਅਗਲਾ_Upsert;
  }
  ਵਾਪਸ 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(Upsert *pUpsert, ਸੂਚਕਾਂਕ *pIdx){
  ਜਦੋਂ(
      pUpsert
   && pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_ਅਗਲਾ_Upsert;
  }
  ਵਾਪਸ pUpsert;
}

/*
** Generate bytecode that does an UPDATE as part of an upsert.
**
** If pIdx is NULL, then the UNIQUE constraint that failed was the IPK.
** In this case parameter iCur is a cursor open on the table b-tree that
** currently points to the conflicting table row. Otherwise, if pIdx
** is not NULL, then pIdx is the constraint that failed and iCur is a
** cursor points to the conflicting row.
*/
ਖਾਲੀ sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  ਸਾਰਣੀ *pTab,          /* The table being updated */
  ਸੂਚਕਾਂਕ *pIdx,          /* The UNIQUE constraint that failed */
  ਪੂਰਨ iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur;
  ਪੂਰਨ i;
  Upsert *p_ਸਿਖਰ = pUpsert;

  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pUpsert!=0 );
  i_ਅੰਕੜੇ_Cur = pUpsert->i_ਅੰਕੜੇ_Cur;
  pUpsert = sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(p_ਸਿਖਰ, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  ਜੇ( pIdx && iCur!=i_ਅੰਕੜੇ_Cur ){
    ਜੇ( HasRowid(pTab) ){
      ਪੂਰਨ regRowid = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_SeekRowid, i_ਅੰਕੜੇ_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regRowid);
    }ਨਹੀਂਤਾਂ{
      ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      ਪੂਰਨ nPk = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
      ਪੂਰਨ iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      ਲਈ(i=0; i<nPk; i++){
        ਪੂਰਨ k;
        ਪ੍ਰਤਿਜ੍ਞਾ( pPk->ai_ਕਾਲਮ[i]>=0 );
        k = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pIdx, pPk->ai_ਕਾਲਮ[i]);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_ਖਾਨਾ[pPk->ai_ਕਾਲਮ[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, i_ਅੰਕੜੇ_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_ਸਿਖਰ->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
    ਜੇ( pTab->a_ਖਾਨਾ[i].affinity==SQLITE_AFF_REAL ){
      ਪੂਰਨ iStorage = p_ਸਿਖਰ->reg_ਅੰਕੜੇ + sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_ਵਿਅੰਜਕ_List_Dup(db,pUpsert->p_Upsert_ਸੈੱਟ,0),
      sqlite3_ਵਿਅੰਜਕ_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UPSERT */
