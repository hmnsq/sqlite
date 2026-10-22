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

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
ساكن فراغ SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  افعل{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_دەربڕین_List_Delete(db, p->pUpsertTarget);
    sqlite3_دەربڕین_Delete(db, p->pUpsertTargetWhere);
    sqlite3_دەربڕین_List_Delete(db, p->pUpsertSet);
    sqlite3_دەربڕین_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
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
           sqlite3_دەربڕین_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_دەربڕین_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_دەربڕین_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_دەربڕین_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  دەربڕین_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  دەربڕین *pTargetWhere,    /* Optional WHERE clause on the target */
  دەربڕین_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  دەربڕین *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, حجم(Upsert));
  إذا( pNew==0 ){
    sqlite3_دەربڕین_List_Delete(db, pTarget);
    sqlite3_دەربڕین_Delete(db, pTargetWhere);
    sqlite3_دەربڕین_List_Delete(db, pSet);
    sqlite3_دەربڕین_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    أرجع 0;
  }وإلا{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
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
صحيح sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  صحيح rc;                 /* Result code */
  صحيح iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  دەربڕین_List *pTarget;      /* The conflict-target clause */
  دەربڕین *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  دەربڕین sCol[2];           /* Index column converted into an Expr */
  صحيح nClause = 0;        /* Counter of ON CONFLICT clauses */

  أكد( pTabList->nSrc==1 );
  أكد( pTabList->a[0].pSTab!=0 );
  أكد( pUpsert!=0 );
  أكد( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  لكل(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_دەربڕین_List_Names(&sNC, pUpsert->pUpsertTarget);
    إذا( rc ) أرجع rc;
    rc = sqlite3_Resolve_دەربڕین_Names(&sNC, pUpsert->pUpsertTargetWhere);
    إذا( rc ) أرجع rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    إذا( HasRowid(pTab) 
     && pTarget->n_دەربڕین==1
     && (pTerm = pTarget->a[0].p_دەربڕین)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
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
    memset(sCol, 0, حجم(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_چەپ = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    لكل(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      صحيح ii, jj, nn;
      إذا( !IsUniqueIndex(pIdx) ) استمر;
      إذا( pTarget->n_دەربڕین!=pIdx->n_کلیل_Col ) استمر;
      إذا( pIdx->pPartIdxWhere ){
        إذا( pUpsert->pUpsertTargetWhere==0 ) استمر;
        إذا( sqlite3_دەربڕین_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          استمر;
        }
      }
      nn = pIdx->n_کلیل_Col;
      لكل(ii=0; ii<nn; ii++){
        دەربڕین *p_دەربڕین;
        sCol[0].u.zToken = (حرف*)pIdx->azColl[ii];
        إذا( pIdx->aiColumn[ii]==XN_دەربڕین ){
          أكد( pIdx->a_Col_دەربڕین!=0 );
          أكد( pIdx->a_Col_دەربڕین->n_دەربڕین>ii );
          أكد( pIdx->b_Has_دەربڕین );
          p_دەربڕین = pIdx->a_Col_دەربڕین->a[ii].p_دەربڕین;
          إذا( p_دەربڕین->op!=TK_COLLATE ){
            sCol[0].p_چەپ = p_دەربڕین;
            p_دەربڕین = &sCol[0];
          }
        }وإلا{
          sCol[0].p_چەپ = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_دەربڕین = &sCol[0];
        }
        لكل(jj=0; jj<nn; jj++){
          إذا( sqlite3_دەربڕین_Compare(0,pTarget->a[jj].p_دەربڕین,p_دەربڕین,iCursor)<2 ){
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
      إذا( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
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
      إذا( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }وإلا{
        sqlite3_snprintf(حجم(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_پەیام(pParse, "%sON CONFLICT clause does not match any "
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
صحيح sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  إذا( NEVER(pUpsert==0) ) أرجع 0;
  pNext = pUpsert->pNextUpsert;
  طالما( 1 /*exit-by-return*/ ){
    إذا( pNext==0 ) أرجع 1;
    إذا( pNext->pUpsertTarget==0 ) أرجع 1;
    إذا( pNext->pUpsertIdx==0 ) أرجع 1;
    إذا( !pNext->isDup ) أرجع 0;
    pNext = pNext->pNextUpsert;
  }
  أرجع 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  طالما(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
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
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  صحيح iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  صحيح iDataCur;
  صحيح i;
  Upsert *p_لووتکە = pUpsert;

  أكد( v!=0 );
  أكد( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_لووتکە, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  إذا( pIdx && iCur!=iDataCur ){
    إذا( HasRowid(pTab) ){
      صحيح regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }وإلا{
      Index *pPk = sqlite3_Primary_کلیل_Index(pTab);
      صحيح nPk = pPk->n_کلیل_Col;
      صحيح iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      لكل(i=0; i<nPk; i++){
        صحيح k;
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
  pSrc = sqlite3SrcListDup(db, p_لووتکە->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  لكل(i=0; i<pTab->nCol; i++){
    إذا( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      صحيح iStorage = p_لووتکە->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_دەربڕین_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_دەربڕین_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
