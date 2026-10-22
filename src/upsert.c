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

#که_نه_تعریف_شوی SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ساكن فراغ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  افعل{
    Upsert *p_بل = p->p_بل_Upsert;
    sqlite3_بیان_List_Delete(db, p->p_Upsert_موخه);
    sqlite3_بیان_Delete(db, p->p_Upsert_موخه_Where);
    sqlite3_بیان_List_Delete(db, p->p_Upsert_سېټ);
    sqlite3_بیان_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_منزل_Free);
    sqlite3DbFree(db, p);
    p = p_بل;
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
           sqlite3_بیان_List_Dup(db, p->p_Upsert_موخه, 0),
           sqlite3_بیان_Dup(db, p->p_Upsert_موخه_Where, 0),
           sqlite3_بیان_List_Dup(db, p->p_Upsert_سېټ, 0),
           sqlite3_بیان_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_بل_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  بیان_List *p_موخه,     /* Target argument to ON CONFLICT, or NULL */
  بیان *p_موخه_Where,    /* Optional WHERE clause on the target */
  بیان_List *p_سېټ,        /* UPDATE columns, or NULL for a DO NOTHING */
  بیان *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_بل          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, حجم(Upsert));
  إذا( pNew==0 ){
    sqlite3_بیان_List_Delete(db, p_موخه);
    sqlite3_بیان_Delete(db, p_موخه_Where);
    sqlite3_بیان_List_Delete(db, p_سېټ);
    sqlite3_بیان_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_بل);
    أرجع 0;
  }وإلا{
    pNew->p_Upsert_موخه = p_موخه;
    pNew->p_Upsert_موخه_Where = p_موخه_Where;
    pNew->p_Upsert_سېټ = p_سېټ;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_سېټ!=0;
    pNew->p_بل_Upsert = p_بل;
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
صحيح sqlite3_Upsert_Analyze_موخه(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  جدول *pTab;            /* That table into which we are inserting */
  صحيح rc;                 /* Result code */
  صحيح iCursor;            /* Cursor used by pTab */
  شاخص *pIdx;            /* One of the indexes of pTab */
  بیان_List *p_موخه;      /* The conflict-target clause */
  بیان *p_غړی;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  بیان s_ستون[2];           /* Index column converted into an Expr */
  صحيح nClause = 0;        /* Counter of ON CONFLICT clauses */

  أكد( pTabList->nSrc==1 );
  أكد( pTabList->a[0].pSTab!=0 );
  أكد( pUpsert!=0 );
  أكد( pUpsert->p_Upsert_موخه!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  لكل(; pUpsert && pUpsert->p_Upsert_موخه;
        pUpsert=pUpsert->p_بل_Upsert, nClause++){
    rc = sqlite3_Resolve_بیان_List_نومونه(&sNC, pUpsert->p_Upsert_موخه);
    إذا( rc ) أرجع rc;
    rc = sqlite3_Resolve_بیان_نومونه(&sNC, pUpsert->p_Upsert_موخه_Where);
    إذا( rc ) أرجع rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_موخه = pUpsert->p_Upsert_موخه;
    iCursor = pTabList->a[0].iCursor;
    إذا( HasRowid(pTab) 
     && p_موخه->n_بیان==1
     && (p_غړی = p_موخه->a[0].p_بیان)->op==TK_ستنه
     && p_غړی->i_ستنه==XN_ROWID
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
    memset(s_ستون, 0, حجم(s_ستون));
    s_ستون[0].op = TK_COLLATE;
    s_ستون[0].p_کیڼ = &s_ستون[1];
    s_ستون[1].op = TK_ستنه;
    s_ستون[1].i_جدول = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
      صحيح ii, jj, nn;
      إذا( !Is_Unique_شاخص(pIdx) ) استمر;
      إذا( p_موخه->n_بیان!=pIdx->n_کلی_ستون ) استمر;
      إذا( pIdx->pPartIdxWhere ){
        إذا( pUpsert->p_Upsert_موخه_Where==0 ) استمر;
        إذا( sqlite3_بیان_Compare(pParse, pUpsert->p_Upsert_موخه_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          استمر;
        }
      }
      nn = pIdx->n_کلی_ستون;
      لكل(ii=0; ii<nn; ii++){
        بیان *p_بیان;
        s_ستون[0].u.zToken = (حرف*)pIdx->azColl[ii];
        إذا( pIdx->ai_ستنه[ii]==XN_بیان ){
          أكد( pIdx->a_ستون_بیان!=0 );
          أكد( pIdx->a_ستون_بیان->n_بیان>ii );
          أكد( pIdx->b_Has_بیان );
          p_بیان = pIdx->a_ستون_بیان->a[ii].p_بیان;
          إذا( p_بیان->op!=TK_COLLATE ){
            s_ستون[0].p_کیڼ = p_بیان;
            p_بیان = &s_ستون[0];
          }
        }وإلا{
          s_ستون[0].p_کیڼ = &s_ستون[1];
          s_ستون[1].i_ستنه = pIdx->ai_ستنه[ii];
          p_بیان = &s_ستون[0];
        }
        لكل(jj=0; jj<nn; jj++){
          إذا( sqlite3_بیان_Compare(0,p_موخه->a[jj].p_بیان,p_بیان,iCursor)<2 ){
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
      إذا( sqlite3_Upsert_د_شاخص(pAll,pIdx)!=pUpsert ){
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
      إذا( nClause==0 && pUpsert->p_بل_Upsert==0 ){
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
صحيح sqlite3_Upsert_بل_Is_IPK(Upsert *pUpsert){
  Upsert *p_بل;
  إذا( NEVER(pUpsert==0) ) أرجع 0;
  p_بل = pUpsert->p_بل_Upsert;
  طالما( 1 /*exit-by-return*/ ){
    إذا( p_بل==0 ) أرجع 1;
    إذا( p_بل->p_Upsert_موخه==0 ) أرجع 1;
    إذا( p_بل->pUpsertIdx==0 ) أرجع 1;
    إذا( !p_بل->isDup ) أرجع 0;
    p_بل = p_بل->p_بل_Upsert;
  }
  أرجع 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_د_شاخص(Upsert *pUpsert, شاخص *pIdx){
  طالما(
      pUpsert
   && pUpsert->p_Upsert_موخه!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_بل_Upsert;
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
  شاخص *pIdx,          /* The UNIQUE constraint that failed */
  صحيح iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  صحيح i_معلومات_Cur;
  صحيح i;
  Upsert *p_سر_2 = pUpsert;

  أكد( v!=0 );
  أكد( pUpsert!=0 );
  i_معلومات_Cur = pUpsert->i_معلومات_Cur;
  pUpsert = sqlite3_Upsert_د_شاخص(p_سر_2, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  إذا( pIdx && iCur!=i_معلومات_Cur ){
    إذا( HasRowid(pTab) ){
      صحيح regRowid = sqlite3_Get_لنډمهاله_Reg(pParse);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_SeekRowid, i_معلومات_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_لنډمهاله_Reg(pParse, regRowid);
    }وإلا{
      شاخص *pPk = sqlite3_Primary_کلی_شاخص(pTab);
      صحيح nPk = pPk->n_کلی_ستون;
      صحيح iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      لكل(i=0; i<nPk; i++){
        صحيح k;
        أكد( pPk->ai_ستنه[i]>=0 );
        k = sqlite3_جدول_ستنه_منزل_شاخص(pIdx, pPk->ai_ستنه[i]);
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_ستون[pPk->ai_ستنه[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_وموندل, i_معلومات_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_دلته(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_سر_2->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  لكل(i=0; i<pTab->n_ستون; i++){
    إذا( pTab->a_ستون[i].affinity==SQLITE_AFF_REAL ){
      صحيح iStorage = p_سر_2->reg_معلومات + sqlite3_جدول_ستنه_منزل_Storage(pTab, i);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_بیان_List_Dup(db,pUpsert->p_Upsert_سېټ,0),
      sqlite3_بیان_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#د_که_پای /* SQLITE_OMIT_UPSERT */
