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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  ಮಾಡು{
    Upsert *p_ಮುಂದಿನ = p->p_ಮುಂದಿನ_Upsert;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->p_Upsert_ಗುರಿ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->p_Upsert_ಗುರಿ_Where);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->p_Upsert_ಗಣ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_ಗಮ್ಯ_Free);
    sqlite3DbFree(db, p);
    p = p_ಮುಂದಿನ;
  }ಆಗ( p );
}
ಶೂನ್ಯ sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  ಆದರೆ( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ sqlite3UpsertNew(db,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->p_Upsert_ಗುರಿ, 0),
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_Upsert_ಗುರಿ_Where, 0),
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->p_Upsert_ಗಣ, 0),
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_ಮುಂದಿನ_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುರಿ,     /* Target argument to ON CONFLICT, or NULL */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಗುರಿ_Where,    /* Optional WHERE clause on the target */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗಣ,        /* UPDATE columns, or NULL for a DO NOTHING */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_ಮುಂದಿನ          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, ಗಾತ್ರ(Upsert));
  ಆದರೆ( pNew==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಗುರಿ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಗುರಿ_Where);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಗಣ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_ಮುಂದಿನ);
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pNew->p_Upsert_ಗುರಿ = p_ಗುರಿ;
    pNew->p_Upsert_ಗುರಿ_Where = p_ಗುರಿ_Where;
    pNew->p_Upsert_ಗಣ = p_ಗಣ;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_ಗಣ!=0;
    pNew->p_ಮುಂದಿನ_Upsert = p_ಮುಂದಿನ;
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Upsert_Analyze_ಗುರಿ(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  ಕೋಷ್ಟಕ *pTab;            /* That table into which we are inserting */
  ಪೂರ್ಣಾಂಕ rc;                 /* Result code */
  ಪೂರ್ಣಾಂಕ iCursor;            /* Cursor used by pTab */
  ಸೂಚ್ಯಂಕ *pIdx;            /* One of the indexes of pTab */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುರಿ;      /* The conflict-target clause */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಪದ;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  ಅಭಿವ್ಯಕ್ತಿ s_ಲಂಬಸಾಲು[2];           /* Index column converted into an Expr */
  ಪೂರ್ಣಾಂಕ nClause = 0;        /* Counter of ON CONFLICT clauses */

  ಪ್ರತಿಜ್ಞಾ( pTabList->nSrc==1 );
  ಪ್ರತಿಜ್ಞಾ( pTabList->a[0].pSTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pUpsert!=0 );
  ಪ್ರತಿಜ್ಞಾ( pUpsert->p_Upsert_ಗುರಿ!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, ಗಾತ್ರ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ಗಾಗಿ(; pUpsert && pUpsert->p_Upsert_ಗುರಿ;
        pUpsert=pUpsert->p_ಮುಂದಿನ_Upsert, nClause++){
    rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pUpsert->p_Upsert_ಗುರಿ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pUpsert->p_Upsert_ಗುರಿ_Where);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_ಗುರಿ = pUpsert->p_Upsert_ಗುರಿ;
    iCursor = pTabList->a[0].iCursor;
    ಆದರೆ( HasRowid(pTab) 
     && p_ಗುರಿ->n_ಅಭಿವ್ಯಕ್ತಿ==1
     && (p_ಪದ = p_ಗುರಿ->a[0].p_ಅಭಿವ್ಯಕ್ತಿ)->op==TK_ಸ್ತಂಭ
     && p_ಪದ->i_ಸ್ತಂಭ==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      ಪ್ರತಿಜ್ಞಾ( pUpsert->pUpsertIdx==0 );
      ಮುಂದುವರಿ;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(s_ಲಂಬಸಾಲು, 0, ಗಾತ್ರ(s_ಲಂಬಸಾಲು));
    s_ಲಂಬಸಾಲು[0].op = TK_COLLATE;
    s_ಲಂಬಸಾಲು[0].p_ಎಡ = &s_ಲಂಬಸಾಲು[1];
    s_ಲಂಬಸಾಲು[1].op = TK_ಸ್ತಂಭ;
    s_ಲಂಬಸಾಲು[1].i_ಕೋಷ್ಟಕ = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
      ಪೂರ್ಣಾಂಕ ii, jj, nn;
      ಆದರೆ( !Is_Unique_ಸೂಚ್ಯಂಕ(pIdx) ) ಮುಂದುವರಿ;
      ಆದರೆ( p_ಗುರಿ->n_ಅಭಿವ್ಯಕ್ತಿ!=pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು ) ಮುಂದುವರಿ;
      ಆದರೆ( pIdx->pPartIdxWhere ){
        ಆದರೆ( pUpsert->p_Upsert_ಗುರಿ_Where==0 ) ಮುಂದುವರಿ;
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pUpsert->p_Upsert_ಗುರಿ_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          ಮುಂದುವರಿ;
        }
      }
      nn = pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
      ಗಾಗಿ(ii=0; ii<nn; ii++){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
        s_ಲಂಬಸಾಲು[0].u.zToken = (ಅಕ್ಷರ*)pIdx->azColl[ii];
        ಆದರೆ( pIdx->ai_ಸ್ತಂಭ[ii]==XN_ಅಭಿವ್ಯಕ್ತಿ ){
          ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0 );
          ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->n_ಅಭಿವ್ಯಕ್ತಿ>ii );
          ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
          p_ಅಭಿವ್ಯಕ್ತಿ = pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
          ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLLATE ){
            s_ಲಂಬಸಾಲು[0].p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ;
            p_ಅಭಿವ್ಯಕ್ತಿ = &s_ಲಂಬಸಾಲು[0];
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          s_ಲಂಬಸಾಲು[0].p_ಎಡ = &s_ಲಂಬಸಾಲು[1];
          s_ಲಂಬಸಾಲು[1].i_ಸ್ತಂಭ = pIdx->ai_ಸ್ತಂಭ[ii];
          p_ಅಭಿವ್ಯಕ್ತಿ = &s_ಲಂಬಸಾಲು[0];
        }
        ಗಾಗಿ(jj=0; jj<nn; jj++){
          ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,p_ಗುರಿ->a[jj].p_ಅಭಿವ್ಯಕ್ತಿ,p_ಅಭಿವ್ಯಕ್ತಿ,iCursor)<2 ){
            ನಿಲ್ಲಿಸು;  /* Column ii of the index matches column jj of target */
          }
        }
        ಆದರೆ( jj>=nn ){
          /* The target contains no match for column jj of the index */
          ನಿಲ್ಲಿಸು;
        }
      }
      ಆದರೆ( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        ಮುಂದುವರಿ;
      }
      pUpsert->pUpsertIdx = pIdx;
      ಆದರೆ( sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( pUpsert->pUpsertIdx==0 ){
      ಅಕ್ಷರ zWhich[16];
      ಆದರೆ( nClause==0 && pUpsert->p_ಮುಂದಿನ_Upsert==0 ){
        zWhich[0] = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_snprintf(ಗಾತ್ರ(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_ಸಂದೇಶ(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Upsert_ಮುಂದಿನ_Is_IPK(Upsert *pUpsert){
  Upsert *p_ಮುಂದಿನ;
  ಆದರೆ( NEVER(pUpsert==0) ) ಹಿಂತಿರುಗಿ 0;
  p_ಮುಂದಿನ = pUpsert->p_ಮುಂದಿನ_Upsert;
  ಆಗ( 1 /*exit-by-return*/ ){
    ಆದರೆ( p_ಮುಂದಿನ==0 ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( p_ಮುಂದಿನ->p_Upsert_ಗುರಿ==0 ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( p_ಮುಂದಿನ->pUpsertIdx==0 ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( !p_ಮುಂದಿನ->isDup ) ಹಿಂತಿರುಗಿ 0;
    p_ಮುಂದಿನ = p_ಮುಂದಿನ->p_ಮುಂದಿನ_Upsert;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(Upsert *pUpsert, ಸೂಚ್ಯಂಕ *pIdx){
  ಆಗ(
      pUpsert
   && pUpsert->p_Upsert_ಗುರಿ!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_ಮುಂದಿನ_Upsert;
  }
  ಹಿಂತಿರುಗಿ pUpsert;
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
ಶೂನ್ಯ sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  ಕೋಷ್ಟಕ *pTab,          /* The table being updated */
  ಸೂಚ್ಯಂಕ *pIdx,          /* The UNIQUE constraint that failed */
  ಪೂರ್ಣಾಂಕ iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur;
  ಪೂರ್ಣಾಂಕ i;
  Upsert *p_ತುದಿ = pUpsert;

  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pUpsert!=0 );
  i_ದತ್ತಾಂಶ_Cur = pUpsert->i_ದತ್ತಾಂಶ_Cur;
  pUpsert = sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(p_ತುದಿ, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  ಆದರೆ( pIdx && iCur!=i_ದತ್ತಾಂಶ_Cur ){
    ಆದರೆ( HasRowid(pTab) ){
      ಪೂರ್ಣಾಂಕ regRowid = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_SeekRowid, i_ದತ್ತಾಂಶ_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regRowid);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      ಪೂರ್ಣಾಂಕ nPk = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
      ಪೂರ್ಣಾಂಕ iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      ಗಾಗಿ(i=0; i<nPk; i++){
        ಪೂರ್ಣಾಂಕ k;
        ಪ್ರತಿಜ್ಞಾ( pPk->ai_ಸ್ತಂಭ[i]>=0 );
        k = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pIdx, pPk->ai_ಸ್ತಂಭ[i]);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_ಲಂಬಸಾಲು[pPk->ai_ಸ್ತಂಭ[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, i_ದತ್ತಾಂಶ_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_ತುದಿ->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
    ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].affinity==SQLITE_AFF_REAL ){
      ಪೂರ್ಣಾಂಕ iStorage = p_ತುದಿ->reg_ದತ್ತಾಂಶ + sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db,pUpsert->p_Upsert_ಗಣ,0),
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UPSERT */
