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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ସ୍ଥିର ଶୂନ୍ୟ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  କର{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, p->pUpsertTarget);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p->pUpsertTargetWhere);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, p->pUpsertSet);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }ଯେତେବେଳେ( p );
}
ଶୂନ୍ୟ sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  ଯଦି( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ sqlite3UpsertNew(db,
           sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  ପଦସମଷ୍ଟି_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  ପଦସମଷ୍ଟି *pTargetWhere,    /* Optional WHERE clause on the target */
  ପଦସମଷ୍ଟି_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  ପଦସମଷ୍ଟି *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, ଆକାର(Upsert));
  ଯଦି( pNew==0 ){
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pTarget);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, pTargetWhere);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pSet);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  ଫେରନ୍ତୁ pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                 /* Result code */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  ପଦସମଷ୍ଟି_List *pTarget;      /* The conflict-target clause */
  ପଦସମଷ୍ଟି *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  ପଦସମଷ୍ଟି sCol[2];           /* Index column converted into an Expr */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nClause = 0;        /* Counter of ON CONFLICT clauses */

  ପ୍ରତିଜ୍ଞା( pTabList->nSrc==1 );
  ପ୍ରତିଜ୍ଞା( pTabList->a[0].pSTab!=0 );
  ପ୍ରତିଜ୍ଞା( pUpsert!=0 );
  ପ୍ରତିଜ୍ଞା( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, ଆକାର(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ପାଇଁ(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pUpsert->pUpsertTarget);
    ଯଦି( rc ) ଫେରନ୍ତୁ rc;
    rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pUpsert->pUpsertTargetWhere);
    ଯଦି( rc ) ଫେରନ୍ତୁ rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    ଯଦି( HasRowid(pTab) 
     && pTarget->n_ପଦସମଷ୍ଟି==1
     && (pTerm = pTarget->a[0].p_ପଦସମଷ୍ଟି)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      ପ୍ରତିଜ୍ଞା( pUpsert->pUpsertIdx==0 );
      ଜାରି;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, ଆକାର(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_ବାମ = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    ପାଇଁ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii, jj, nn;
      ଯଦି( !IsUniqueIndex(pIdx) ) ଜାରି;
      ଯଦି( pTarget->n_ପଦସମଷ୍ଟି!=pIdx->n_ଚାବି_Col ) ଜାରି;
      ଯଦି( pIdx->pPartIdxWhere ){
        ଯଦି( pUpsert->pUpsertTargetWhere==0 ) ଜାରି;
        ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          ଜାରି;
        }
      }
      nn = pIdx->n_ଚାବି_Col;
      ପାଇଁ(ii=0; ii<nn; ii++){
        ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି;
        sCol[0].u.zToken = (ଅକ୍ଷର*)pIdx->azColl[ii];
        ଯଦି( pIdx->aiColumn[ii]==XN_ପଦସମଷ୍ଟି ){
          ପ୍ରତିଜ୍ଞା( pIdx->a_Col_ପଦସମଷ୍ଟି!=0 );
          ପ୍ରତିଜ୍ଞା( pIdx->a_Col_ପଦସମଷ୍ଟି->n_ପଦସମଷ୍ଟି>ii );
          ପ୍ରତିଜ୍ଞା( pIdx->b_Has_ପଦସମଷ୍ଟି );
          p_ପଦସମଷ୍ଟି = pIdx->a_Col_ପଦସମଷ୍ଟି->a[ii].p_ପଦସମଷ୍ଟି;
          ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_COLLATE ){
            sCol[0].p_ବାମ = p_ପଦସମଷ୍ଟି;
            p_ପଦସମଷ୍ଟି = &sCol[0];
          }
        }ଅନ୍ୟଥା{
          sCol[0].p_ବାମ = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_ପଦସମଷ୍ଟି = &sCol[0];
        }
        ପାଇଁ(jj=0; jj<nn; jj++){
          ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0,pTarget->a[jj].p_ପଦସମଷ୍ଟି,p_ପଦସମଷ୍ଟି,iCursor)<2 ){
            ଭାଙ୍ଗ;  /* Column ii of the index matches column jj of target */
          }
        }
        ଯଦି( jj>=nn ){
          /* The target contains no match for column jj of the index */
          ଭାଙ୍ଗ;
        }
      }
      ଯଦି( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        ଜାରି;
      }
      pUpsert->pUpsertIdx = pIdx;
      ଯଦି( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      ଭାଙ୍ଗ;
    }
    ଯଦି( pUpsert->pUpsertIdx==0 ){
      ଅକ୍ଷର zWhich[16];
      ଯଦି( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }ଅନ୍ୟଥା{
        sqlite3_snprintf(ଆକାର(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  ଯଦି( NEVER(pUpsert==0) ) ଫେରନ୍ତୁ 0;
  pNext = pUpsert->pNextUpsert;
  ଯେତେବେଳେ( 1 /*exit-by-return*/ ){
    ଯଦି( pNext==0 ) ଫେରନ୍ତୁ 1;
    ଯଦି( pNext->pUpsertTarget==0 ) ଫେରନ୍ତୁ 1;
    ଯଦି( pNext->pUpsertIdx==0 ) ଫେରନ୍ତୁ 1;
    ଯଦି( !pNext->isDup ) ଫେରନ୍ତୁ 0;
    pNext = pNext->pNextUpsert;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  ଯେତେବେଳେ(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  ଫେରନ୍ତୁ pUpsert;
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
ଶୂନ୍ୟ sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Upsert *p_ଶୀର୍ଷ = pUpsert;

  ପ୍ରତିଜ୍ଞା( v!=0 );
  ପ୍ରତିଜ୍ଞା( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_ଶୀର୍ଷ, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  ଯଦି( pIdx && iCur!=iDataCur ){
    ଯଦି( HasRowid(pTab) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }ଅନ୍ୟଥା{
      Index *pPk = sqlite3_Primary_ଚାବି_Index(pTab);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPk = pPk->n_ଚାବି_Col;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      ପାଇଁ(i=0; i<nPk; i++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;
        ପ୍ରତିଜ୍ଞା( pPk->aiColumn[i]>=0 );
        k = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[i]);
        sqlite3VdbeAddOp3(v, OP_Column, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->aCol[pPk->aiColumn[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3VdbeAddOp4Int(v, OP_Found, iDataCur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3VdbeJumpHere(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_ଶୀର୍ଷ->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  ପାଇଁ(i=0; i<pTab->nCol; i++){
    ଯଦି( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStorage = p_ଶୀର୍ଷ->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_ପଦସମଷ୍ଟି_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_ପଦସମଷ୍ଟି_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
