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
#include "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
static void SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  do{
    Upsert *pNext = p->pNextUpsert;
    sqlite3ExprListDelete(db, p->pUpsertTarget);
    sqlite3ExprDelete(db, p->pUpsertTargetWhere);
    sqlite3ExprListDelete(db, p->pUpsertSet);
    sqlite3ExprDelete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }ໃນຂະນະທີ່( p );
}
void sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  ຖ້າ( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  ຖ້າ( p==0 ) ສົ່ງຄືນ 0;
  ສົ່ງຄືນ sqlite3UpsertNew(db,
           sqlite3ExprListDup(db, p->pUpsertTarget, 0),
           sqlite3ExprDup(db, p->pUpsertTargetWhere, 0),
           sqlite3ExprListDup(db, p->pUpsertSet, 0),
           sqlite3ExprDup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  ExprList *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  Expr *pTargetWhere,    /* Optional WHERE clause on the target */
  ExprList *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  Expr *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, parimaa_na(Upsert));
  ຖ້າ( pNew==0 ){
    sqlite3ExprListDelete(db, pTarget);
    sqlite3ExprDelete(db, pTargetWhere);
    sqlite3ExprListDelete(db, pSet);
    sqlite3ExprDelete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    ສົ່ງຄືນ 0;
  }ບໍ່ດັ່ງນັ້ນ{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  ສົ່ງຄືນ pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
ຈຳນວນເຕັມ sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  ຈຳນວນເຕັມ rc;                 /* Result code */
  ຈຳນວນເຕັມ iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  ExprList *pTarget;      /* The conflict-target clause */
  Expr *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  Expr sCol[2];           /* Index column converted into an Expr */
  ຈຳນວນເຕັມ nClause = 0;        /* Counter of ON CONFLICT clauses */

  assert( pTabList->nSrc==1 );
  assert( pTabList->a[0].pSTab!=0 );
  assert( pUpsert!=0 );
  assert( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, parimaa_na(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ສຳລັບ(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3ResolveExprListNames(&sNC, pUpsert->pUpsertTarget);
    ຖ້າ( rc ) ສົ່ງຄືນ rc;
    rc = sqlite3ResolveExprNames(&sNC, pUpsert->pUpsertTargetWhere);
    ຖ້າ( rc ) ສົ່ງຄືນ rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    ຖ້າ( HasRowid(pTab) 
     && pTarget->nExpr==1
     && (pTerm = pTarget->a[0].pExpr)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      assert( pUpsert->pUpsertIdx==0 );
      continue;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, parimaa_na(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].pLeft = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    ສຳລັບ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ຈຳນວນເຕັມ ii, jj, nn;
      ຖ້າ( !IsUniqueIndex(pIdx) ) continue;
      ຖ້າ( pTarget->nExpr!=pIdx->nKeyCol ) continue;
      ຖ້າ( pIdx->pPartIdxWhere ){
        ຖ້າ( pUpsert->pUpsertTargetWhere==0 ) continue;
        ຖ້າ( sqlite3ExprCompare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          continue;
        }
      }
      nn = pIdx->nKeyCol;
      ສຳລັບ(ii=0; ii<nn; ii++){
        Expr *pExpr;
        sCol[0].u.zToken = (var_na*)pIdx->azColl[ii];
        ຖ້າ( pIdx->aiColumn[ii]==XN_EXPR ){
          assert( pIdx->aColExpr!=0 );
          assert( pIdx->aColExpr->nExpr>ii );
          assert( pIdx->bHasExpr );
          pExpr = pIdx->aColExpr->a[ii].pExpr;
          ຖ້າ( pExpr->op!=TK_COLLATE ){
            sCol[0].pLeft = pExpr;
            pExpr = &sCol[0];
          }
        }ບໍ່ດັ່ງນັ້ນ{
          sCol[0].pLeft = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          pExpr = &sCol[0];
        }
        ສຳລັບ(jj=0; jj<nn; jj++){
          ຖ້າ( sqlite3ExprCompare(0,pTarget->a[jj].pExpr,pExpr,iCursor)<2 ){
            bha_nga;  /* Column ii of the index matches column jj of target */
          }
        }
        ຖ້າ( jj>=nn ){
          /* The target contains no match for column jj of the index */
          bha_nga;
        }
      }
      ຖ້າ( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        continue;
      }
      pUpsert->pUpsertIdx = pIdx;
      ຖ້າ( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      bha_nga;
    }
    ຖ້າ( pUpsert->pUpsertIdx==0 ){
      var_na zWhich[16];
      ຖ້າ( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }ບໍ່ດັ່ງນັ້ນ{
        sqlite3_snprintf(parimaa_na(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3ErrorMsg(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      ສົ່ງຄືນ SQLITE_ERROR;
    }
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
ຈຳນວນເຕັມ sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  ຖ້າ( NEVER(pUpsert==0) ) ສົ່ງຄືນ 0;
  pNext = pUpsert->pNextUpsert;
  ໃນຂະນະທີ່( 1 /*exit-by-return*/ ){
    ຖ້າ( pNext==0 ) ສົ່ງຄືນ 1;
    ຖ້າ( pNext->pUpsertTarget==0 ) ສົ່ງຄືນ 1;
    ຖ້າ( pNext->pUpsertIdx==0 ) ສົ່ງຄືນ 1;
    ຖ້າ( !pNext->isDup ) ສົ່ງຄືນ 0;
    pNext = pNext->pNextUpsert;
  }
  ສົ່ງຄືນ 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  ໃນຂະນະທີ່(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  ສົ່ງຄືນ pUpsert;
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
void sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  ຈຳນວນເຕັມ iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  ຈຳນວນເຕັມ iDataCur;
  ຈຳນວນເຕັມ i;
  Upsert *pTop = pUpsert;

  assert( v!=0 );
  assert( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(pTop, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  ຖ້າ( pIdx && iCur!=iDataCur ){
    ຖ້າ( HasRowid(pTab) ){
      ຈຳນວນເຕັມ regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }ບໍ່ດັ່ງນັ້ນ{
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      ຈຳນວນເຕັມ nPk = pPk->nKeyCol;
      ຈຳນວນເຕັມ iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      ສຳລັບ(i=0; i<nPk; i++){
        ຈຳນວນເຕັມ k;
        assert( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, pTop->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  ສຳລັບ(i=0; i<pTab->nCol; i++){
    ຖ້າ( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      ຈຳນວນເຕັມ iStorage = pTop->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3ExprListDup(db,pUpsert->pUpsertSet,0),
      sqlite3ExprDup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
