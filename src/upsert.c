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
#அந்தர்பாவ "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
நிலையான வெறுமை SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  செய்{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_கோவை_List_Delete(db, p->pUpsertTarget);
    sqlite3_கோவை_Delete(db, p->pUpsertTargetWhere);
    sqlite3_கோவை_List_Delete(db, p->pUpsertSet);
    sqlite3_கோவை_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }போது( p );
}
வெறுமை sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  எனில்( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  எனில்( p==0 ) திரும்பு 0;
  திரும்பு sqlite3UpsertNew(db,
           sqlite3_கோவை_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_கோவை_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_கோவை_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_கோவை_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  கோவை_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  கோவை *pTargetWhere,    /* Optional WHERE clause on the target */
  கோவை_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  கோவை *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, அளவு(Upsert));
  எனில்( pNew==0 ){
    sqlite3_கோவை_List_Delete(db, pTarget);
    sqlite3_கோவை_Delete(db, pTargetWhere);
    sqlite3_கோவை_List_Delete(db, pSet);
    sqlite3_கோவை_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    திரும்பு 0;
  }இல்லையெனில்{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  திரும்பு pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
முழு sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  முழு rc;                 /* Result code */
  முழு iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  கோவை_List *pTarget;      /* The conflict-target clause */
  கோவை *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  கோவை sCol[2];           /* Index column converted into an Expr */
  முழு nClause = 0;        /* Counter of ON CONFLICT clauses */

  ப்ரதிஜ்ஞா( pTabList->nSrc==1 );
  ப்ரதிஜ்ஞா( pTabList->a[0].pSTab!=0 );
  ப்ரதிஜ்ஞா( pUpsert!=0 );
  ப்ரதிஜ்ஞா( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, அளவு(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  க்கு(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_கோவை_List_Names(&sNC, pUpsert->pUpsertTarget);
    எனில்( rc ) திரும்பு rc;
    rc = sqlite3_Resolve_கோவை_Names(&sNC, pUpsert->pUpsertTargetWhere);
    எனில்( rc ) திரும்பு rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    எனில்( HasRowid(pTab) 
     && pTarget->n_கோவை==1
     && (pTerm = pTarget->a[0].p_கோவை)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      ப்ரதிஜ்ஞா( pUpsert->pUpsertIdx==0 );
      தொடர்;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, அளவு(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_இடது = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      முழு ii, jj, nn;
      எனில்( !IsUniqueIndex(pIdx) ) தொடர்;
      எனில்( pTarget->n_கோவை!=pIdx->n_சாவி_Col ) தொடர்;
      எனில்( pIdx->pPartIdxWhere ){
        எனில்( pUpsert->pUpsertTargetWhere==0 ) தொடர்;
        எனில்( sqlite3_கோவை_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          தொடர்;
        }
      }
      nn = pIdx->n_சாவி_Col;
      க்கு(ii=0; ii<nn; ii++){
        கோவை *p_கோவை;
        sCol[0].u.zToken = (எழுத்து*)pIdx->azColl[ii];
        எனில்( pIdx->aiColumn[ii]==XN_கோவை ){
          ப்ரதிஜ்ஞா( pIdx->a_Col_கோவை!=0 );
          ப்ரதிஜ்ஞா( pIdx->a_Col_கோவை->n_கோவை>ii );
          ப்ரதிஜ்ஞா( pIdx->b_Has_கோவை );
          p_கோவை = pIdx->a_Col_கோவை->a[ii].p_கோவை;
          எனில்( p_கோவை->op!=TK_COLLATE ){
            sCol[0].p_இடது = p_கோவை;
            p_கோவை = &sCol[0];
          }
        }இல்லையெனில்{
          sCol[0].p_இடது = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_கோவை = &sCol[0];
        }
        க்கு(jj=0; jj<nn; jj++){
          எனில்( sqlite3_கோவை_Compare(0,pTarget->a[jj].p_கோவை,p_கோவை,iCursor)<2 ){
            நிறுத்து;  /* Column ii of the index matches column jj of target */
          }
        }
        எனில்( jj>=nn ){
          /* The target contains no match for column jj of the index */
          நிறுத்து;
        }
      }
      எனில்( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        தொடர்;
      }
      pUpsert->pUpsertIdx = pIdx;
      எனில்( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      நிறுத்து;
    }
    எனில்( pUpsert->pUpsertIdx==0 ){
      எழுத்து zWhich[16];
      எனில்( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }இல்லையெனில்{
        sqlite3_snprintf(அளவு(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_செய்தி(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      திரும்பு SQLITE_ERROR;
    }
  }
  திரும்பு SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
முழு sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  எனில்( NEVER(pUpsert==0) ) திரும்பு 0;
  pNext = pUpsert->pNextUpsert;
  போது( 1 /*exit-by-return*/ ){
    எனில்( pNext==0 ) திரும்பு 1;
    எனில்( pNext->pUpsertTarget==0 ) திரும்பு 1;
    எனில்( pNext->pUpsertIdx==0 ) திரும்பு 1;
    எனில்( !pNext->isDup ) திரும்பு 0;
    pNext = pNext->pNextUpsert;
  }
  திரும்பு 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  போது(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  திரும்பு pUpsert;
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
வெறுமை sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  முழு iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  முழு iDataCur;
  முழு i;
  Upsert *p_உச்சி = pUpsert;

  ப்ரதிஜ்ஞா( v!=0 );
  ப்ரதிஜ்ஞா( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_உச்சி, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  எனில்( pIdx && iCur!=iDataCur ){
    எனில்( HasRowid(pTab) ){
      முழு regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }இல்லையெனில்{
      Index *pPk = sqlite3_Primary_சாவி_Index(pTab);
      முழு nPk = pPk->n_சாவி_Col;
      முழு iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      க்கு(i=0; i<nPk; i++){
        முழு k;
        ப்ரதிஜ்ஞா( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, p_உச்சி->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  க்கு(i=0; i<pTab->nCol; i++){
    எனில்( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      முழு iStorage = p_உச்சி->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_கோவை_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_கோவை_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
