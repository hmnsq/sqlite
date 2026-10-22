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

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ساكن فراغ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  افعل{
    Upsert *p_ايندڙ = p->p_ايندڙ_Upsert;
    sqlite3_اظهار_List_Delete(db, p->p_Upsert_هدف);
    sqlite3_اظهار_Delete(db, p->p_Upsert_هدف_Where);
    sqlite3_اظهار_List_Delete(db, p->p_Upsert_سيٽ);
    sqlite3_اظهار_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_منزل_Free);
    sqlite3DbFree(db, p);
    p = p_ايندڙ;
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
           sqlite3_اظهار_List_Dup(db, p->p_Upsert_هدف, 0),
           sqlite3_اظهار_Dup(db, p->p_Upsert_هدف_Where, 0),
           sqlite3_اظهار_List_Dup(db, p->p_Upsert_سيٽ, 0),
           sqlite3_اظهار_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_ايندڙ_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  اظهار_List *p_هدف,     /* Target argument to ON CONFLICT, or NULL */
  اظهار *p_هدف_Where,    /* Optional WHERE clause on the target */
  اظهار_List *p_سيٽ,        /* UPDATE columns, or NULL for a DO NOTHING */
  اظهار *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_ايندڙ          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, حجم(Upsert));
  إذا( pNew==0 ){
    sqlite3_اظهار_List_Delete(db, p_هدف);
    sqlite3_اظهار_Delete(db, p_هدف_Where);
    sqlite3_اظهار_List_Delete(db, p_سيٽ);
    sqlite3_اظهار_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_ايندڙ);
    أرجع 0;
  }وإلا{
    pNew->p_Upsert_هدف = p_هدف;
    pNew->p_Upsert_هدف_Where = p_هدف_Where;
    pNew->p_Upsert_سيٽ = p_سيٽ;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_سيٽ!=0;
    pNew->p_ايندڙ_Upsert = p_ايندڙ;
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
صحيح sqlite3_Upsert_Analyze_هدف(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  جدول *pTab;            /* That table into which we are inserting */
  صحيح rc;                 /* Result code */
  صحيح iCursor;            /* Cursor used by pTab */
  اشاريو *pIdx;            /* One of the indexes of pTab */
  اظهار_List *p_هدف;      /* The conflict-target clause */
  اظهار *p_رڪن;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  اظهار s_کانو[2];           /* Index column converted into an Expr */
  صحيح nClause = 0;        /* Counter of ON CONFLICT clauses */

  أكد( pTabList->nSrc==1 );
  أكد( pTabList->a[0].pSTab!=0 );
  أكد( pUpsert!=0 );
  أكد( pUpsert->p_Upsert_هدف!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  لكل(; pUpsert && pUpsert->p_Upsert_هدف;
        pUpsert=pUpsert->p_ايندڙ_Upsert, nClause++){
    rc = sqlite3_Resolve_اظهار_List_نالا(&sNC, pUpsert->p_Upsert_هدف);
    إذا( rc ) أرجع rc;
    rc = sqlite3_Resolve_اظهار_نالا(&sNC, pUpsert->p_Upsert_هدف_Where);
    إذا( rc ) أرجع rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_هدف = pUpsert->p_Upsert_هدف;
    iCursor = pTabList->a[0].iCursor;
    إذا( HasRowid(pTab) 
     && p_هدف->n_اظهار==1
     && (p_رڪن = p_هدف->a[0].p_اظهار)->op==TK_ڪالم
     && p_رڪن->i_ڪالم==XN_ROWID
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
    memset(s_کانو, 0, حجم(s_کانو));
    s_کانو[0].op = TK_COLLATE;
    s_کانو[0].p_کاٻو = &s_کانو[1];
    s_کانو[1].op = TK_ڪالم;
    s_کانو[1].i_جدول = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
      صحيح ii, jj, nn;
      إذا( !Is_Unique_اشاريو(pIdx) ) استمر;
      إذا( p_هدف->n_اظهار!=pIdx->n_ڪنجي_کانو ) استمر;
      إذا( pIdx->pPartIdxWhere ){
        إذا( pUpsert->p_Upsert_هدف_Where==0 ) استمر;
        إذا( sqlite3_اظهار_Compare(pParse, pUpsert->p_Upsert_هدف_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          استمر;
        }
      }
      nn = pIdx->n_ڪنجي_کانو;
      لكل(ii=0; ii<nn; ii++){
        اظهار *p_اظهار;
        s_کانو[0].u.zToken = (حرف*)pIdx->azColl[ii];
        إذا( pIdx->ai_ڪالم[ii]==XN_اظهار ){
          أكد( pIdx->a_کانو_اظهار!=0 );
          أكد( pIdx->a_کانو_اظهار->n_اظهار>ii );
          أكد( pIdx->b_Has_اظهار );
          p_اظهار = pIdx->a_کانو_اظهار->a[ii].p_اظهار;
          إذا( p_اظهار->op!=TK_COLLATE ){
            s_کانو[0].p_کاٻو = p_اظهار;
            p_اظهار = &s_کانو[0];
          }
        }وإلا{
          s_کانو[0].p_کاٻو = &s_کانو[1];
          s_کانو[1].i_ڪالم = pIdx->ai_ڪالم[ii];
          p_اظهار = &s_کانو[0];
        }
        لكل(jj=0; jj<nn; jj++){
          إذا( sqlite3_اظهار_Compare(0,p_هدف->a[jj].p_اظهار,p_اظهار,iCursor)<2 ){
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
      إذا( sqlite3_Upsert_جو_اشاريو(pAll,pIdx)!=pUpsert ){
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
      إذا( nClause==0 && pUpsert->p_ايندڙ_Upsert==0 ){
        zWhich[0] = 0;
      }وإلا{
        sqlite3_snprintf(حجم(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_نياپو(pParse, "%sON CONFLICT clause does not match any "
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
صحيح sqlite3_Upsert_ايندڙ_Is_IPK(Upsert *pUpsert){
  Upsert *p_ايندڙ;
  إذا( NEVER(pUpsert==0) ) أرجع 0;
  p_ايندڙ = pUpsert->p_ايندڙ_Upsert;
  طالما( 1 /*exit-by-return*/ ){
    إذا( p_ايندڙ==0 ) أرجع 1;
    إذا( p_ايندڙ->p_Upsert_هدف==0 ) أرجع 1;
    إذا( p_ايندڙ->pUpsertIdx==0 ) أرجع 1;
    إذا( !p_ايندڙ->isDup ) أرجع 0;
    p_ايندڙ = p_ايندڙ->p_ايندڙ_Upsert;
  }
  أرجع 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_جو_اشاريو(Upsert *pUpsert, اشاريو *pIdx){
  طالما(
      pUpsert
   && pUpsert->p_Upsert_هدف!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_ايندڙ_Upsert;
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
  اشاريو *pIdx,          /* The UNIQUE constraint that failed */
  صحيح iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  صحيح i_معلومات_Cur;
  صحيح i;
  Upsert *p_چوٽي = pUpsert;

  أكد( v!=0 );
  أكد( pUpsert!=0 );
  i_معلومات_Cur = pUpsert->i_معلومات_Cur;
  pUpsert = sqlite3_Upsert_جو_اشاريو(p_چوٽي, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  إذا( pIdx && iCur!=i_معلومات_Cur ){
    إذا( HasRowid(pTab) ){
      صحيح regRowid = sqlite3_Get_عارضي_Reg(pParse);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_SeekRowid, i_معلومات_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_عارضي_Reg(pParse, regRowid);
    }وإلا{
      اشاريو *pPk = sqlite3_Primary_ڪنجي_اشاريو(pTab);
      صحيح nPk = pPk->n_ڪنجي_کانو;
      صحيح iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      لكل(i=0; i<nPk; i++){
        صحيح k;
        أكد( pPk->ai_ڪالم[i]>=0 );
        k = sqlite3_جدول_ڪالم_منزل_اشاريو(pIdx, pPk->ai_ڪالم[i]);
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_کانو[pPk->ai_ڪالم[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_مليو, i_معلومات_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_هتي(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_چوٽي->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  لكل(i=0; i<pTab->n_کانو; i++){
    إذا( pTab->a_کانو[i].affinity==SQLITE_AFF_REAL ){
      صحيح iStorage = p_چوٽي->reg_معلومات + sqlite3_جدول_ڪالم_منزل_Storage(pTab, i);
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_اظهار_List_Dup(db,pUpsert->p_Upsert_سيٽ,0),
      sqlite3_اظهار_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#جيڪڏهن_پورو /* SQLITE_OMIT_UPSERT */
