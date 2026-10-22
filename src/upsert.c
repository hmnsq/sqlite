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
#شامل "sqliteInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ساکن خالی SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  کرو{
    Upsert *p_اگلا = p->p_اگلا_Upsert;
    sqlite3_اظہاریہ_List_Delete(db, p->p_Upsert_ہدف);
    sqlite3_اظہاریہ_Delete(db, p->p_Upsert_ہدف_Where);
    sqlite3_اظہاریہ_List_Delete(db, p->p_Upsert_سیٹ);
    sqlite3_اظہاریہ_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_مقصد_Free);
    sqlite3DbFree(db, p);
    p = p_اگلا;
  }جب( p );
}
خالی sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  اگر( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  اگر( p==0 ) واپس 0;
  واپس sqlite3UpsertNew(db,
           sqlite3_اظہاریہ_List_Dup(db, p->p_Upsert_ہدف, 0),
           sqlite3_اظہاریہ_Dup(db, p->p_Upsert_ہدف_Where, 0),
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
  اظہاریہ_List *p_ہدف,     /* Target argument to ON CONFLICT, or NULL */
  اظہاریہ *p_ہدف_Where,    /* Optional WHERE clause on the target */
  اظہاریہ_List *p_سیٹ,        /* UPDATE columns, or NULL for a DO NOTHING */
  اظہاریہ *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_اگلا          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, حجم(Upsert));
  اگر( pNew==0 ){
    sqlite3_اظہاریہ_List_Delete(db, p_ہدف);
    sqlite3_اظہاریہ_Delete(db, p_ہدف_Where);
    sqlite3_اظہاریہ_List_Delete(db, p_سیٹ);
    sqlite3_اظہاریہ_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_اگلا);
    واپس 0;
  }ورنہ{
    pNew->p_Upsert_ہدف = p_ہدف;
    pNew->p_Upsert_ہدف_Where = p_ہدف_Where;
    pNew->p_Upsert_سیٹ = p_سیٹ;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_سیٹ!=0;
    pNew->p_اگلا_Upsert = p_اگلا;
  }
  واپس pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
صحیح sqlite3_Upsert_Analyze_ہدف(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  جدول *pTab;            /* That table into which we are inserting */
  صحیح rc;                 /* Result code */
  صحیح iCursor;            /* Cursor used by pTab */
  اشاریہ *pIdx;            /* One of the indexes of pTab */
  اظہاریہ_List *p_ہدف;      /* The conflict-target clause */
  اظہاریہ *p_رکن;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  اظہاریہ s_ستون[2];           /* Index column converted into an Expr */
  صحیح nClause = 0;        /* Counter of ON CONFLICT clauses */

  أكد( pTabList->nSrc==1 );
  أكد( pTabList->a[0].pSTab!=0 );
  أكد( pUpsert!=0 );
  أكد( pUpsert->p_Upsert_ہدف!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  برائے(; pUpsert && pUpsert->p_Upsert_ہدف;
        pUpsert=pUpsert->p_اگلا_Upsert, nClause++){
    rc = sqlite3_Resolve_اظہاریہ_List_نام(&sNC, pUpsert->p_Upsert_ہدف);
    اگر( rc ) واپس rc;
    rc = sqlite3_Resolve_اظہاریہ_نام(&sNC, pUpsert->p_Upsert_ہدف_Where);
    اگر( rc ) واپس rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_ہدف = pUpsert->p_Upsert_ہدف;
    iCursor = pTabList->a[0].iCursor;
    اگر( HasRowid(pTab) 
     && p_ہدف->n_اظہاریہ==1
     && (p_رکن = p_ہدف->a[0].p_اظہاریہ)->op==TK_عمود
     && p_رکن->i_عمود==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      أكد( pUpsert->pUpsertIdx==0 );
      جاری;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(s_ستون, 0, حجم(s_ستون));
    s_ستون[0].op = TK_COLLATE;
    s_ستون[0].p_بایاں = &s_ستون[1];
    s_ستون[1].op = TK_عمود;
    s_ستون[1].i_جدول = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    برائے(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      صحیح ii, jj, nn;
      اگر( !Is_Unique_اشاریہ(pIdx) ) جاری;
      اگر( p_ہدف->n_اظہاریہ!=pIdx->n_کلید_ستون ) جاری;
      اگر( pIdx->pPartIdxWhere ){
        اگر( pUpsert->p_Upsert_ہدف_Where==0 ) جاری;
        اگر( sqlite3_اظہاریہ_Compare(pParse, pUpsert->p_Upsert_ہدف_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          جاری;
        }
      }
      nn = pIdx->n_کلید_ستون;
      برائے(ii=0; ii<nn; ii++){
        اظہاریہ *p_اظہاریہ;
        s_ستون[0].u.zToken = (حرف*)pIdx->azColl[ii];
        اگر( pIdx->ai_عمود[ii]==XN_اظہاریہ ){
          أكد( pIdx->a_ستون_اظہاریہ!=0 );
          أكد( pIdx->a_ستون_اظہاریہ->n_اظہاریہ>ii );
          أكد( pIdx->b_Has_اظہاریہ );
          p_اظہاریہ = pIdx->a_ستون_اظہاریہ->a[ii].p_اظہاریہ;
          اگر( p_اظہاریہ->op!=TK_COLLATE ){
            s_ستون[0].p_بایاں = p_اظہاریہ;
            p_اظہاریہ = &s_ستون[0];
          }
        }ورنہ{
          s_ستون[0].p_بایاں = &s_ستون[1];
          s_ستون[1].i_عمود = pIdx->ai_عمود[ii];
          p_اظہاریہ = &s_ستون[0];
        }
        برائے(jj=0; jj<nn; jj++){
          اگر( sqlite3_اظہاریہ_Compare(0,p_ہدف->a[jj].p_اظہاریہ,p_اظہاریہ,iCursor)<2 ){
            توڑو;  /* Column ii of the index matches column jj of target */
          }
        }
        اگر( jj>=nn ){
          /* The target contains no match for column jj of the index */
          توڑو;
        }
      }
      اگر( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        جاری;
      }
      pUpsert->pUpsertIdx = pIdx;
      اگر( sqlite3_Upsert_کا_اشاریہ(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      توڑو;
    }
    اگر( pUpsert->pUpsertIdx==0 ){
      حرف zWhich[16];
      اگر( nClause==0 && pUpsert->p_اگلا_Upsert==0 ){
        zWhich[0] = 0;
      }ورنہ{
        sqlite3_snprintf(حجم(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_پیغام(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      واپس SQLITE_ERROR;
    }
  }
  واپس SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
صحیح sqlite3_Upsert_اگلا_Is_IPK(Upsert *pUpsert){
  Upsert *p_اگلا;
  اگر( NEVER(pUpsert==0) ) واپس 0;
  p_اگلا = pUpsert->p_اگلا_Upsert;
  جب( 1 /*exit-by-return*/ ){
    اگر( p_اگلا==0 ) واپس 1;
    اگر( p_اگلا->p_Upsert_ہدف==0 ) واپس 1;
    اگر( p_اگلا->pUpsertIdx==0 ) واپس 1;
    اگر( !p_اگلا->isDup ) واپس 0;
    p_اگلا = p_اگلا->p_اگلا_Upsert;
  }
  واپس 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_کا_اشاریہ(Upsert *pUpsert, اشاریہ *pIdx){
  جب(
      pUpsert
   && pUpsert->p_Upsert_ہدف!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_اگلا_Upsert;
  }
  واپس pUpsert;
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
خالی sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  جدول *pTab,          /* The table being updated */
  اشاریہ *pIdx,          /* The UNIQUE constraint that failed */
  صحیح iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  صحیح i_ڈیٹا_Cur;
  صحیح i;
  Upsert *p_چوٹی = pUpsert;

  أكد( v!=0 );
  أكد( pUpsert!=0 );
  i_ڈیٹا_Cur = pUpsert->i_ڈیٹا_Cur;
  pUpsert = sqlite3_Upsert_کا_اشاریہ(p_چوٹی, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  اگر( pIdx && iCur!=i_ڈیٹا_Cur ){
    اگر( HasRowid(pTab) ){
      صحیح regRowid = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_SeekRowid, i_ڈیٹا_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_عارضی_Reg(pParse, regRowid);
    }ورنہ{
      اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      صحیح nPk = pPk->n_کلید_ستون;
      صحیح iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      برائے(i=0; i<nPk; i++){
        صحیح k;
        أكد( pPk->ai_عمود[i]>=0 );
        k = sqlite3_جدول_عمود_مقصد_اشاریہ(pIdx, pPk->ai_عمود[i]);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_ستون[pPk->ai_عمود[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_ملا, i_ڈیٹا_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_یہاں(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_چوٹی->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  برائے(i=0; i<pTab->n_ستون; i++){
    اگر( pTab->a_ستون[i].affinity==SQLITE_AFF_REAL ){
      صحیح iStorage = p_چوٹی->reg_ڈیٹا + sqlite3_جدول_عمود_مقصد_Storage(pTab, i);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_اظہاریہ_List_Dup(db,pUpsert->p_Upsert_سیٹ,0),
      sqlite3_اظہاریہ_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#اگر_ختم /* SQLITE_OMIT_UPSERT */
