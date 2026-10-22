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
#अन्तर्भाव "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
स्थिर शून्य SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  करा{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_अभिव्यक्ती_List_Delete(db, p->pUpsertTarget);
    sqlite3_अभिव्यक्ती_Delete(db, p->pUpsertTargetWhere);
    sqlite3_अभिव्यक्ती_List_Delete(db, p->pUpsertSet);
    sqlite3_अभिव्यक्ती_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }पर्यंत( p );
}
शून्य sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  जर( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  जर( p==0 ) परत 0;
  परत sqlite3UpsertNew(db,
           sqlite3_अभिव्यक्ती_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_अभिव्यक्ती_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_अभिव्यक्ती_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_अभिव्यक्ती_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  अभिव्यक्ती_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  अभिव्यक्ती *pTargetWhere,    /* Optional WHERE clause on the target */
  अभिव्यक्ती_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  अभिव्यक्ती *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, आकार(Upsert));
  जर( pNew==0 ){
    sqlite3_अभिव्यक्ती_List_Delete(db, pTarget);
    sqlite3_अभिव्यक्ती_Delete(db, pTargetWhere);
    sqlite3_अभिव्यक्ती_List_Delete(db, pSet);
    sqlite3_अभिव्यक्ती_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    परत 0;
  }अन्यथा{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  परत pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
पूर्णांक sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  पूर्णांक rc;                 /* Result code */
  पूर्णांक iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  अभिव्यक्ती_List *pTarget;      /* The conflict-target clause */
  अभिव्यक्ती *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  अभिव्यक्ती sCol[2];           /* Index column converted into an Expr */
  पूर्णांक nClause = 0;        /* Counter of ON CONFLICT clauses */

  प्रतिज्ञा( pTabList->nSrc==1 );
  प्रतिज्ञा( pTabList->a[0].pSTab!=0 );
  प्रतिज्ञा( pUpsert!=0 );
  प्रतिज्ञा( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  स_रखो(&sNC, 0, आकार(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  साठी(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_अभिव्यक्ती_List_Names(&sNC, pUpsert->pUpsertTarget);
    जर( rc ) परत rc;
    rc = sqlite3_Resolve_अभिव्यक्ती_Names(&sNC, pUpsert->pUpsertTargetWhere);
    जर( rc ) परत rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    जर( HasRowid(pTab) 
     && pTarget->n_अभिव्यक्ती==1
     && (pTerm = pTarget->a[0].p_अभिव्यक्ती)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      प्रतिज्ञा( pUpsert->pUpsertIdx==0 );
      सुरूठेवा;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    स_रखो(sCol, 0, आकार(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_डावा = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    साठी(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      पूर्णांक ii, jj, nn;
      जर( !IsUniqueIndex(pIdx) ) सुरूठेवा;
      जर( pTarget->n_अभिव्यक्ती!=pIdx->n_किल्ली_Col ) सुरूठेवा;
      जर( pIdx->pPartIdxWhere ){
        जर( pUpsert->pUpsertTargetWhere==0 ) सुरूठेवा;
        जर( sqlite3_अभिव्यक्ती_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          सुरूठेवा;
        }
      }
      nn = pIdx->n_किल्ली_Col;
      साठी(ii=0; ii<nn; ii++){
        अभिव्यक्ती *p_अभिव्यक्ती;
        sCol[0].u.zToken = (अक्षर*)pIdx->azColl[ii];
        जर( pIdx->aiColumn[ii]==XN_अभिव्यक्ती ){
          प्रतिज्ञा( pIdx->a_Col_अभिव्यक्ती!=0 );
          प्रतिज्ञा( pIdx->a_Col_अभिव्यक्ती->n_अभिव्यक्ती>ii );
          प्रतिज्ञा( pIdx->b_Has_अभिव्यक्ती );
          p_अभिव्यक्ती = pIdx->a_Col_अभिव्यक्ती->a[ii].p_अभिव्यक्ती;
          जर( p_अभिव्यक्ती->op!=TK_COLLATE ){
            sCol[0].p_डावा = p_अभिव्यक्ती;
            p_अभिव्यक्ती = &sCol[0];
          }
        }अन्यथा{
          sCol[0].p_डावा = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_अभिव्यक्ती = &sCol[0];
        }
        साठी(jj=0; jj<nn; jj++){
          जर( sqlite3_अभिव्यक्ती_Compare(0,pTarget->a[jj].p_अभिव्यक्ती,p_अभिव्यक्ती,iCursor)<2 ){
            थांबा;  /* Column ii of the index matches column jj of target */
          }
        }
        जर( jj>=nn ){
          /* The target contains no match for column jj of the index */
          थांबा;
        }
      }
      जर( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        सुरूठेवा;
      }
      pUpsert->pUpsertIdx = pIdx;
      जर( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      थांबा;
    }
    जर( pUpsert->pUpsertIdx==0 ){
      अक्षर zWhich[16];
      जर( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }अन्यथा{
        sqlite3_snprintf(आकार(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_संदेश(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      परत SQLITE_ERROR;
    }
  }
  परत SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
पूर्णांक sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  जर( NEVER(pUpsert==0) ) परत 0;
  pNext = pUpsert->pNextUpsert;
  पर्यंत( 1 /*exit-by-return*/ ){
    जर( pNext==0 ) परत 1;
    जर( pNext->pUpsertTarget==0 ) परत 1;
    जर( pNext->pUpsertIdx==0 ) परत 1;
    जर( !pNext->isDup ) परत 0;
    pNext = pNext->pNextUpsert;
  }
  परत 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  पर्यंत(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  परत pUpsert;
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
शून्य sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  पूर्णांक iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  पूर्णांक iDataCur;
  पूर्णांक i;
  Upsert *p_शीर्ष = pUpsert;

  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_शीर्ष, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  जर( pIdx && iCur!=iDataCur ){
    जर( HasRowid(pTab) ){
      पूर्णांक regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }अन्यथा{
      Index *pPk = sqlite3_Primary_किल्ली_Index(pTab);
      पूर्णांक nPk = pPk->n_किल्ली_Col;
      पूर्णांक iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      साठी(i=0; i<nPk; i++){
        पूर्णांक k;
        प्रतिज्ञा( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, p_शीर्ष->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  साठी(i=0; i<pTab->nCol; i++){
    जर( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      पूर्णांक iStorage = p_शीर्ष->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_अभिव्यक्ती_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_अभिव्यक्ती_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
