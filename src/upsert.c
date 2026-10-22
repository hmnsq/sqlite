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
#അന്തര്ഭാവ "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
സ്ഥിതം ശൂന്യം SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  ചെയ്യുക{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_വ്യഞ്ജകം_List_Delete(db, p->pUpsertTarget);
    sqlite3_വ്യഞ്ജകം_Delete(db, p->pUpsertTargetWhere);
    sqlite3_വ്യഞ്ജകം_List_Delete(db, p->pUpsertSet);
    sqlite3_വ്യഞ്ജകം_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }ആകുമ്പോൾ( p );
}
ശൂന്യം sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  എങ്കിൽ( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  എങ്കിൽ( p==0 ) തിരികെ 0;
  തിരികെ sqlite3UpsertNew(db,
           sqlite3_വ്യഞ്ജകം_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_വ്യഞ്ജകം_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_വ്യഞ്ജകം_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_വ്യഞ്ജകം_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  വ്യഞ്ജകം_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  വ്യഞ്ജകം *pTargetWhere,    /* Optional WHERE clause on the target */
  വ്യഞ്ജകം_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  വ്യഞ്ജകം *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, വലുപ്പം(Upsert));
  എങ്കിൽ( pNew==0 ){
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pTarget);
    sqlite3_വ്യഞ്ജകം_Delete(db, pTargetWhere);
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pSet);
    sqlite3_വ്യഞ്ജകം_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    തിരികെ 0;
  }അല്ലെങ്കിൽ{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  തിരികെ pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
പൂർണ്ണസംഖ്യ sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  പൂർണ്ണസംഖ്യ rc;                 /* Result code */
  പൂർണ്ണസംഖ്യ iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  വ്യഞ്ജകം_List *pTarget;      /* The conflict-target clause */
  വ്യഞ്ജകം *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  വ്യഞ്ജകം sCol[2];           /* Index column converted into an Expr */
  പൂർണ്ണസംഖ്യ nClause = 0;        /* Counter of ON CONFLICT clauses */

  പ്രതിജ്ഞാ( pTabList->nSrc==1 );
  പ്രതിജ്ഞാ( pTabList->a[0].pSTab!=0 );
  പ്രതിജ്ഞാ( pUpsert!=0 );
  പ്രതിജ്ഞാ( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, വലുപ്പം(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  വേണ്ടി(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, pUpsert->pUpsertTarget);
    എങ്കിൽ( rc ) തിരികെ rc;
    rc = sqlite3_Resolve_വ്യഞ്ജകം_Names(&sNC, pUpsert->pUpsertTargetWhere);
    എങ്കിൽ( rc ) തിരികെ rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    എങ്കിൽ( HasRowid(pTab) 
     && pTarget->n_വ്യഞ്ജകം==1
     && (pTerm = pTarget->a[0].p_വ്യഞ്ജകം)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      പ്രതിജ്ഞാ( pUpsert->pUpsertIdx==0 );
      തുടരുക;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, വലുപ്പം(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_ഇടത് = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    വേണ്ടി(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      പൂർണ്ണസംഖ്യ ii, jj, nn;
      എങ്കിൽ( !IsUniqueIndex(pIdx) ) തുടരുക;
      എങ്കിൽ( pTarget->n_വ്യഞ്ജകം!=pIdx->n_താക്കോൽ_Col ) തുടരുക;
      എങ്കിൽ( pIdx->pPartIdxWhere ){
        എങ്കിൽ( pUpsert->pUpsertTargetWhere==0 ) തുടരുക;
        എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          തുടരുക;
        }
      }
      nn = pIdx->n_താക്കോൽ_Col;
      വേണ്ടി(ii=0; ii<nn; ii++){
        വ്യഞ്ജകം *p_വ്യഞ്ജകം;
        sCol[0].u.zToken = (അക്ഷരം*)pIdx->azColl[ii];
        എങ്കിൽ( pIdx->aiColumn[ii]==XN_വ്യഞ്ജകം ){
          പ്രതിജ്ഞാ( pIdx->a_Col_വ്യഞ്ജകം!=0 );
          പ്രതിജ്ഞാ( pIdx->a_Col_വ്യഞ്ജകം->n_വ്യഞ്ജകം>ii );
          പ്രതിജ്ഞാ( pIdx->b_Has_വ്യഞ്ജകം );
          p_വ്യഞ്ജകം = pIdx->a_Col_വ്യഞ്ജകം->a[ii].p_വ്യഞ്ജകം;
          എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_COLLATE ){
            sCol[0].p_ഇടത് = p_വ്യഞ്ജകം;
            p_വ്യഞ്ജകം = &sCol[0];
          }
        }അല്ലെങ്കിൽ{
          sCol[0].p_ഇടത് = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_വ്യഞ്ജകം = &sCol[0];
        }
        വേണ്ടി(jj=0; jj<nn; jj++){
          എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0,pTarget->a[jj].p_വ്യഞ്ജകം,p_വ്യഞ്ജകം,iCursor)<2 ){
            നിർത്തുക;  /* Column ii of the index matches column jj of target */
          }
        }
        എങ്കിൽ( jj>=nn ){
          /* The target contains no match for column jj of the index */
          നിർത്തുക;
        }
      }
      എങ്കിൽ( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        തുടരുക;
      }
      pUpsert->pUpsertIdx = pIdx;
      എങ്കിൽ( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      നിർത്തുക;
    }
    എങ്കിൽ( pUpsert->pUpsertIdx==0 ){
      അക്ഷരം zWhich[16];
      എങ്കിൽ( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }അല്ലെങ്കിൽ{
        sqlite3_snprintf(വലുപ്പം(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_സന്ദേശം(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      തിരികെ SQLITE_ERROR;
    }
  }
  തിരികെ SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
പൂർണ്ണസംഖ്യ sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  എങ്കിൽ( NEVER(pUpsert==0) ) തിരികെ 0;
  pNext = pUpsert->pNextUpsert;
  ആകുമ്പോൾ( 1 /*exit-by-return*/ ){
    എങ്കിൽ( pNext==0 ) തിരികെ 1;
    എങ്കിൽ( pNext->pUpsertTarget==0 ) തിരികെ 1;
    എങ്കിൽ( pNext->pUpsertIdx==0 ) തിരികെ 1;
    എങ്കിൽ( !pNext->isDup ) തിരികെ 0;
    pNext = pNext->pNextUpsert;
  }
  തിരികെ 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  ആകുമ്പോൾ(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  തിരികെ pUpsert;
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
ശൂന്യം sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  പൂർണ്ണസംഖ്യ iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  പൂർണ്ണസംഖ്യ iDataCur;
  പൂർണ്ണസംഖ്യ i;
  Upsert *p_മുകൾ = pUpsert;

  പ്രതിജ്ഞാ( v!=0 );
  പ്രതിജ്ഞാ( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_മുകൾ, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  എങ്കിൽ( pIdx && iCur!=iDataCur ){
    എങ്കിൽ( HasRowid(pTab) ){
      പൂർണ്ണസംഖ്യ regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }അല്ലെങ്കിൽ{
      Index *pPk = sqlite3_Primary_താക്കോൽ_Index(pTab);
      പൂർണ്ണസംഖ്യ nPk = pPk->n_താക്കോൽ_Col;
      പൂർണ്ണസംഖ്യ iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      വേണ്ടി(i=0; i<nPk; i++){
        പൂർണ്ണസംഖ്യ k;
        പ്രതിജ്ഞാ( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, p_മുകൾ->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  വേണ്ടി(i=0; i<pTab->nCol; i++){
    എങ്കിൽ( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      പൂർണ്ണസംഖ്യ iStorage = p_മുകൾ->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_വ്യഞ്ജകം_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_വ്യഞ്ജകം_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
