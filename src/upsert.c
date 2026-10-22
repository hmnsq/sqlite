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

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ایستا تهی SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  انجامبده{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_عبارت_List_Delete(db, p->pUpsertTarget);
    sqlite3_عبارت_Delete(db, p->pUpsertTargetWhere);
    sqlite3_عبارت_List_Delete(db, p->pUpsertSet);
    sqlite3_عبارت_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }وقتی( p );
}
تهی sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  اگر( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  اگر( p==0 ) بازگشت 0;
  بازگشت sqlite3UpsertNew(db,
           sqlite3_عبارت_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_عبارت_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_عبارت_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_عبارت_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  عبارت_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  عبارت *pTargetWhere,    /* Optional WHERE clause on the target */
  عبارت_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  عبارت *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, اندازه(Upsert));
  اگر( pNew==0 ){
    sqlite3_عبارت_List_Delete(db, pTarget);
    sqlite3_عبارت_Delete(db, pTargetWhere);
    sqlite3_عبارت_List_Delete(db, pSet);
    sqlite3_عبارت_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    بازگشت 0;
  }وگرنه{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  بازگشت pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
صحیح sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  صحیح rc;                 /* Result code */
  صحیح iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  عبارت_List *pTarget;      /* The conflict-target clause */
  عبارت *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  عبارت sCol[2];           /* Index column converted into an Expr */
  صحیح nClause = 0;        /* Counter of ON CONFLICT clauses */

  أكد( pTabList->nSrc==1 );
  أكد( pTabList->a[0].pSTab!=0 );
  أكد( pUpsert!=0 );
  أكد( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, اندازه(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  برای(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_عبارت_List_Names(&sNC, pUpsert->pUpsertTarget);
    اگر( rc ) بازگشت rc;
    rc = sqlite3_Resolve_عبارت_Names(&sNC, pUpsert->pUpsertTargetWhere);
    اگر( rc ) بازگشت rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    اگر( HasRowid(pTab) 
     && pTarget->n_عبارت==1
     && (pTerm = pTarget->a[0].p_عبارت)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      أكد( pUpsert->pUpsertIdx==0 );
      ادامه;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, اندازه(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_چپ = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      صحیح ii, jj, nn;
      اگر( !IsUniqueIndex(pIdx) ) ادامه;
      اگر( pTarget->n_عبارت!=pIdx->n_کلید_Col ) ادامه;
      اگر( pIdx->pPartIdxWhere ){
        اگر( pUpsert->pUpsertTargetWhere==0 ) ادامه;
        اگر( sqlite3_عبارت_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          ادامه;
        }
      }
      nn = pIdx->n_کلید_Col;
      برای(ii=0; ii<nn; ii++){
        عبارت *p_عبارت;
        sCol[0].u.zToken = (نویسه*)pIdx->azColl[ii];
        اگر( pIdx->aiColumn[ii]==XN_عبارت ){
          أكد( pIdx->a_Col_عبارت!=0 );
          أكد( pIdx->a_Col_عبارت->n_عبارت>ii );
          أكد( pIdx->b_Has_عبارت );
          p_عبارت = pIdx->a_Col_عبارت->a[ii].p_عبارت;
          اگر( p_عبارت->op!=TK_COLLATE ){
            sCol[0].p_چپ = p_عبارت;
            p_عبارت = &sCol[0];
          }
        }وگرنه{
          sCol[0].p_چپ = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_عبارت = &sCol[0];
        }
        برای(jj=0; jj<nn; jj++){
          اگر( sqlite3_عبارت_Compare(0,pTarget->a[jj].p_عبارت,p_عبارت,iCursor)<2 ){
            بشکن;  /* Column ii of the index matches column jj of target */
          }
        }
        اگر( jj>=nn ){
          /* The target contains no match for column jj of the index */
          بشکن;
        }
      }
      اگر( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        ادامه;
      }
      pUpsert->pUpsertIdx = pIdx;
      اگر( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      بشکن;
    }
    اگر( pUpsert->pUpsertIdx==0 ){
      نویسه zWhich[16];
      اگر( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }وگرنه{
        sqlite3_snprintf(اندازه(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_پیام(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      بازگشت SQLITE_ERROR;
    }
  }
  بازگشت SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
صحیح sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  اگر( NEVER(pUpsert==0) ) بازگشت 0;
  pNext = pUpsert->pNextUpsert;
  وقتی( 1 /*exit-by-return*/ ){
    اگر( pNext==0 ) بازگشت 1;
    اگر( pNext->pUpsertTarget==0 ) بازگشت 1;
    اگر( pNext->pUpsertIdx==0 ) بازگشت 1;
    اگر( !pNext->isDup ) بازگشت 0;
    pNext = pNext->pNextUpsert;
  }
  بازگشت 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  وقتی(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  بازگشت pUpsert;
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
تهی sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  صحیح iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  صحیح iDataCur;
  صحیح i;
  Upsert *p_بالاترین = pUpsert;

  أكد( v!=0 );
  أكد( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_بالاترین, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  اگر( pIdx && iCur!=iDataCur ){
    اگر( HasRowid(pTab) ){
      صحیح regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }وگرنه{
      Index *pPk = sqlite3_Primary_کلید_Index(pTab);
      صحیح nPk = pPk->n_کلید_Col;
      صحیح iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      برای(i=0; i<nPk; i++){
        صحیح k;
        أكد( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, p_بالاترین->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  برای(i=0; i<pTab->nCol; i++){
    اگر( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      صحیح iStorage = p_بالاترین->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_عبارت_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_عبارت_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
