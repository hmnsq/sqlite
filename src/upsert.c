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

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  ಕುರು{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->pUpsertTarget);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pUpsertTargetWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->pUpsertSet);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }ಯಾವತ್( p );
}
ಶೂನ್ಯ sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  ಯದಿ( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ sqlite3UpsertNew(db,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  ಅಭಿವ್ಯಕ್ತಿ_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  ಅಭಿವ್ಯಕ್ತಿ *pTargetWhere,    /* Optional WHERE clause on the target */
  ಅಭಿವ್ಯಕ್ತಿ_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Upsert));
  ಯದಿ( pNew==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pTarget);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pTargetWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pSet);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  ಪೂರ್ಣಾಙ್ಕ rc;                 /* Result code */
  ಪೂರ್ಣಾಙ್ಕ iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  ಅಭಿವ್ಯಕ್ತಿ_List *pTarget;      /* The conflict-target clause */
  ಅಭಿವ್ಯಕ್ತಿ *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  ಅಭಿವ್ಯಕ್ತಿ sCol[2];           /* Index column converted into an Expr */
  ಪೂರ್ಣಾಙ್ಕ nClause = 0;        /* Counter of ON CONFLICT clauses */

  ಪ್ರತಿಜ್ಞಾ( pTabList->nSrc==1 );
  ಪ್ರತಿಜ್ಞಾ( pTabList->a[0].pSTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pUpsert!=0 );
  ಪ್ರತಿಜ್ಞಾ( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ಪ್ರತಿ(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pUpsert->pUpsertTarget);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
    rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, pUpsert->pUpsertTargetWhere);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    ಯದಿ( HasRowid(pTab) 
     && pTarget->n_ಅಭಿವ್ಯಕ್ತಿ==1
     && (pTerm = pTarget->a[0].p_ಅಭಿವ್ಯಕ್ತಿ)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      ಪ್ರತಿಜ್ಞಾ( pUpsert->pUpsertIdx==0 );
      ಅನುವರ್ತ;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, ಪರಿಮಾಣ(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_ಎಡ = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ಪೂರ್ಣಾಙ್ಕ ii, jj, nn;
      ಯದಿ( !IsUniqueIndex(pIdx) ) ಅನುವರ್ತ;
      ಯದಿ( pTarget->n_ಅಭಿವ್ಯಕ್ತಿ!=pIdx->n_ಕೀಲಿ_Col ) ಅನುವರ್ತ;
      ಯದಿ( pIdx->pPartIdxWhere ){
        ಯದಿ( pUpsert->pUpsertTargetWhere==0 ) ಅನುವರ್ತ;
        ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          ಅನುವರ್ತ;
        }
      }
      nn = pIdx->n_ಕೀಲಿ_Col;
      ಪ್ರತಿ(ii=0; ii<nn; ii++){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
        sCol[0].u.zToken = (ವರ್ಣ*)pIdx->azColl[ii];
        ಯದಿ( pIdx->aiColumn[ii]==XN_ಅಭಿವ್ಯಕ್ತಿ ){
          ಪ್ರತಿಜ್ಞಾ( pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ!=0 );
          ಪ್ರತಿಜ್ಞಾ( pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ->n_ಅಭಿವ್ಯಕ್ತಿ>ii );
          ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
          p_ಅಭಿವ್ಯಕ್ತಿ = pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
          ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLLATE ){
            sCol[0].p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ;
            p_ಅಭಿವ್ಯಕ್ತಿ = &sCol[0];
          }
        }ಅನ್ಯಥಾ{
          sCol[0].p_ಎಡ = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_ಅಭಿವ್ಯಕ್ತಿ = &sCol[0];
        }
        ಪ್ರತಿ(jj=0; jj<nn; jj++){
          ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pTarget->a[jj].p_ಅಭಿವ್ಯಕ್ತಿ,p_ಅಭಿವ್ಯಕ್ತಿ,iCursor)<2 ){
            ಭಙ್ಗ;  /* Column ii of the index matches column jj of target */
          }
        }
        ಯದಿ( jj>=nn ){
          /* The target contains no match for column jj of the index */
          ಭಙ್ಗ;
        }
      }
      ಯದಿ( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        ಅನುವರ್ತ;
      }
      pUpsert->pUpsertIdx = pIdx;
      ಯದಿ( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      ಭಙ್ಗ;
    }
    ಯದಿ( pUpsert->pUpsertIdx==0 ){
      ವರ್ಣ zWhich[16];
      ಯದಿ( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }ಅನ್ಯಥಾ{
        sqlite3_snprintf(ಪರಿಮಾಣ(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_ಸಂದೇಶ(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  ಯದಿ( NEVER(pUpsert==0) ) ಪ್ರತ್ಯಾಗಮ 0;
  pNext = pUpsert->pNextUpsert;
  ಯಾವತ್( 1 /*exit-by-return*/ ){
    ಯದಿ( pNext==0 ) ಪ್ರತ್ಯಾಗಮ 1;
    ಯದಿ( pNext->pUpsertTarget==0 ) ಪ್ರತ್ಯಾಗಮ 1;
    ಯದಿ( pNext->pUpsertIdx==0 ) ಪ್ರತ್ಯಾಗಮ 1;
    ಯದಿ( !pNext->isDup ) ಪ್ರತ್ಯಾಗಮ 0;
    pNext = pNext->pNextUpsert;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  ಯಾವತ್(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  ಪ್ರತ್ಯಾಗಮ pUpsert;
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
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  ಪೂರ್ಣಾಙ್ಕ iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  ಪೂರ್ಣಾಙ್ಕ iDataCur;
  ಪೂರ್ಣಾಙ್ಕ i;
  Upsert *p_ತುದಿ = pUpsert;

  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_ತುದಿ, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  ಯದಿ( pIdx && iCur!=iDataCur ){
    ಯದಿ( HasRowid(pTab) ){
      ಪೂರ್ಣಾಙ್ಕ regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }ಅನ್ಯಥಾ{
      Index *pPk = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
      ಪೂರ್ಣಾಙ್ಕ nPk = pPk->n_ಕೀಲಿ_Col;
      ಪೂರ್ಣಾಙ್ಕ iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      ಪ್ರತಿ(i=0; i<nPk; i++){
        ಪೂರ್ಣಾಙ್ಕ k;
        ಪ್ರತಿಜ್ಞಾ( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, p_ತುದಿ->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  ಪ್ರತಿ(i=0; i<pTab->nCol; i++){
    ಯದಿ( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      ಪೂರ್ಣಾಙ್ಕ iStorage = p_ತುದಿ->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
