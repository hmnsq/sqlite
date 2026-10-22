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
#ضمن "sqliteInt.h"

#جے_بنا_تعریف SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ساكن فراغ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  افعل{
    Upsert *p_اگلا = p->p_اگلا_Upsert;
    sqlite3_اظہاریہ_List_Delete(db, p->p_Upsert_نشانہ);
    sqlite3_اظہاریہ_Delete(db, p->p_Upsert_نشانہ_Where);
    sqlite3_اظہاریہ_List_Delete(db, p->p_Upsert_سیٹ);
    sqlite3_اظہاریہ_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_منزل_Free);
    sqlite3DbFree(db, p);
    p = p_اگلا;
  }طالما( p );
}
فراغ sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  إذا( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  إذا( p==0 ) أرجع 0;
  أرجع sqlite3UpsertNew(db,
           sqlite3_اظہاریہ_List_Dup(db, p->p_Upsert_نشانہ, 0),
           sqlite3_اظہاریہ_Dup(db, p->p_Upsert_نشانہ_Where, 0),
           sqlite3_اظہاریہ_List_Dup(db, p->p_Upsert_سیٹ, 0),
           sqlite3_اظہاریہ_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_اگلا_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  اظہاریہ_List *p_نشانہ,     /* Target argument to ON CONFLICT, or NULL */
  اظہاریہ *p_نشانہ_Where,    /* Optional WHERE clause on the target */
  اظہاریہ_List *p_سیٹ,        /* UPDATE columns, or NULL for a DO NOTHING */
  اظہاریہ *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_اگلا          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, حجم(Upsert));
  إذا( pNew==0 ){
    sqlite3_اظہاریہ_List_Delete(db, p_نشانہ);
    sqlite3_اظہاریہ_Delete(db, p_نشانہ_Where);
    sqlite3_اظہاریہ_List_Delete(db, p_سیٹ);
    sqlite3_اظہاریہ_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_اگلا);
    أرجع 0;
  }وإلا{
    pNew->p_Upsert_نشانہ = p_نشانہ;
    pNew->p_Upsert_نشانہ_Where = p_نشانہ_Where;
    pNew->p_Upsert_سیٹ = p_سیٹ;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_سیٹ!=0;
    pNew->p_اگلا_Upsert = p_اگلا;
  }
  أرجع pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
صحيح sqlite3_Upsert_Analyze_نشانہ(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  جدول *pTab;            /* That table into which we are inserting */
  صحيح rc;                 /* Result code */
  صحيح iCursor;            /* Cursor used by pTab */
  اشاریہ *pIdx;            /* One of the indexes of pTab */
  اظہاریہ_List *p_نشانہ;      /* The conflict-target clause */
  اظہاریہ *p_رکن;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  اظہاریہ s_خانہ[2];           /* Index column converted into an Expr */
  صحيح nClause = 0;        /* Counter of ON CONFLICT clauses */

  أكد( pTabList->nSrc==1 );
  أكد( pTabList->a[0].pSTab!=0 );
  أكد( pUpsert!=0 );
  أكد( pUpsert->p_Upsert_نشانہ!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  لكل(; pUpsert && pUpsert->p_Upsert_نشانہ;
        pUpsert=pUpsert->p_اگلا_Upsert, nClause++){
    rc = sqlite3_Resolve_اظہاریہ_List_ناں(&sNC, pUpsert->p_Upsert_نشانہ);
    إذا( rc ) أرجع rc;
    rc = sqlite3_Resolve_اظہاریہ_ناں(&sNC, pUpsert->p_Upsert_نشانہ_Where);
    إذا( rc ) أرجع rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_نشانہ = pUpsert->p_Upsert_نشانہ;
    iCursor = pTabList->a[0].iCursor;
    إذا( HasRowid(pTab) 
     && p_نشانہ->n_اظہاریہ==1
     && (p_رکن = p_نشانہ->a[0].p_اظہاریہ)->op==TK_کالم
     && p_رکن->i_کالم==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      أكد( pUpsert->pUpsertIdx==0 );
      استمر;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(s_خانہ, 0, حجم(s_خانہ));
    s_خانہ[0].op = TK_COLLATE;
    s_خانہ[0].p_بایاں = &s_خانہ[1];
    s_خانہ[1].op = TK_کالم;
    s_خانہ[1].i_جدول = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      صحيح ii, jj, nn;
      إذا( !Is_Unique_اشاریہ(pIdx) ) استمر;
      إذا( p_نشانہ->n_اظہاریہ!=pIdx->n_کلید_خانہ ) استمر;
      إذا( pIdx->pPartIdxWhere ){
        إذا( pUpsert->p_Upsert_نشانہ_Where==0 ) استمر;
        إذا( sqlite3_اظہاریہ_Compare(pParse, pUpsert->p_Upsert_نشانہ_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          استمر;
        }
      }
      nn = pIdx->n_کلید_خانہ;
      لكل(ii=0; ii<nn; ii++){
        اظہاریہ *p_اظہاریہ;
        s_خانہ[0].u.zToken = (حرف*)pIdx->azColl[ii];
        إذا( pIdx->ai_کالم[ii]==XN_اظہاریہ ){
          أكد( pIdx->a_خانہ_اظہاریہ!=0 );
          أكد( pIdx->a_خانہ_اظہاریہ->n_اظہاریہ>ii );
          أكد( pIdx->b_Has_اظہاریہ );
          p_اظہاریہ = pIdx->a_خانہ_اظہاریہ->a[ii].p_اظہاریہ;
          إذا( p_اظہاریہ->op!=TK_COLLATE ){
            s_خانہ[0].p_بایاں = p_اظہاریہ;
            p_اظہاریہ = &s_خانہ[0];
          }
        }وإلا{
          s_خانہ[0].p_بایاں = &s_خانہ[1];
          s_خانہ[1].i_کالم = pIdx->ai_کالم[ii];
          p_اظہاریہ = &s_خانہ[0];
        }
        لكل(jj=0; jj<nn; jj++){
          إذا( sqlite3_اظہاریہ_Compare(0,p_نشانہ->a[jj].p_اظہاریہ,p_اظہاریہ,iCursor)<2 ){
            اكسر;  /* Column ii of the index matches column jj of target */
          }
        }
        إذا( jj>=nn ){
          /* The target contains no match for column jj of the index */
          اكسر;
        }
      }
      إذا( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        استمر;
      }
      pUpsert->pUpsertIdx = pIdx;
      إذا( sqlite3_Upsert_دا_اشاریہ(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      اكسر;
    }
    إذا( pUpsert->pUpsertIdx==0 ){
      حرف zWhich[16];
      إذا( nClause==0 && pUpsert->p_اگلا_Upsert==0 ){
        zWhich[0] = 0;
      }وإلا{
        sqlite3_snprintf(حجم(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_پیغام(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      أرجع SQLITE_ERROR;
    }
  }
  أرجع SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
صحيح sqlite3_Upsert_اگلا_Is_IPK(Upsert *pUpsert){
  Upsert *p_اگلا;
  إذا( NEVER(pUpsert==0) ) أرجع 0;
  p_اگلا = pUpsert->p_اگلا_Upsert;
  طالما( 1 /*exit-by-return*/ ){
    إذا( p_اگلا==0 ) أرجع 1;
    إذا( p_اگلا->p_Upsert_نشانہ==0 ) أرجع 1;
    إذا( p_اگلا->pUpsertIdx==0 ) أرجع 1;
    إذا( !p_اگلا->isDup ) أرجع 0;
    p_اگلا = p_اگلا->p_اگلا_Upsert;
  }
  أرجع 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_دا_اشاریہ(Upsert *pUpsert, اشاریہ *pIdx){
  طالما(
      pUpsert
   && pUpsert->p_Upsert_نشانہ!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_اگلا_Upsert;
  }
  أرجع pUpsert;
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
فراغ sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  جدول *pTab,          /* The table being updated */
  اشاریہ *pIdx,          /* The UNIQUE constraint that failed */
  صحيح iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  صحيح i_انکڑے_Cur;
  صحيح i;
  Upsert *p_چوٹی = pUpsert;

  أكد( v!=0 );
  أكد( pUpsert!=0 );
  i_انکڑے_Cur = pUpsert->i_انکڑے_Cur;
  pUpsert = sqlite3_Upsert_دا_اشاریہ(p_چوٹی, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  إذا( pIdx && iCur!=i_انکڑے_Cur ){
    إذا( HasRowid(pTab) ){
      صحيح regRowid = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_SeekRowid, i_انکڑے_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_عارضی_Reg(pParse, regRowid);
    }وإلا{
      اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      صحيح nPk = pPk->n_کلید_خانہ;
      صحيح iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      لكل(i=0; i<nPk; i++){
        صحيح k;
        أكد( pPk->ai_کالم[i]>=0 );
        k = sqlite3_جدول_کالم_منزل_اشاریہ(pIdx, pPk->ai_کالم[i]);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_خانہ[pPk->ai_کالم[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_لبھیا, i_انکڑے_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_ایتھے(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_چوٹی->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  لكل(i=0; i<pTab->n_خانہ; i++){
    إذا( pTab->a_خانہ[i].affinity==SQLITE_AFF_REAL ){
      صحيح iStorage = p_چوٹی->reg_انکڑے + sqlite3_جدول_کالم_منزل_Storage(pTab, i);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_اظہاریہ_List_Dup(db,pUpsert->p_Upsert_سیٹ,0),
      sqlite3_اظہاریہ_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#جے_انت /* SQLITE_OMIT_UPSERT */
