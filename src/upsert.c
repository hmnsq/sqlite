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
#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
अटल व्योम SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  करो{
    Upsert *p_अग्र = p->p_अग्र_Upsert;
    sqlite3_व्यंजक_List_Delete(db, p->pUpsertTarget);
    sqlite3_व्यंजक_Delete(db, p->pUpsertTargetWhere);
    sqlite3_व्यंजक_List_Delete(db, p->pUpsertSet);
    sqlite3_व्यंजक_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = p_अग्र;
  }जबतक( p );
}
व्योम sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  अगर( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  अगर( p==0 ) वापस 0;
  वापस sqlite3UpsertNew(db,
           sqlite3_व्यंजक_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_व्यंजक_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_व्यंजक_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_व्यंजक_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_अग्र_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  व्यंजक_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  व्यंजक *pTargetWhere,    /* Optional WHERE clause on the target */
  व्यंजक_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  व्यंजक *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_अग्र          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, माप(Upsert));
  अगर( pNew==0 ){
    sqlite3_व्यंजक_List_Delete(db, pTarget);
    sqlite3_व्यंजक_Delete(db, pTargetWhere);
    sqlite3_व्यंजक_List_Delete(db, pSet);
    sqlite3_व्यंजक_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_अग्र);
    वापस 0;
  }अन्यथा{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->p_अग्र_Upsert = p_अग्र;
  }
  वापस pNew;
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
  सूचकांक *pIdx;            /* One of the indexes of pTab */
  व्यंजक_List *pTarget;      /* The conflict-target clause */
  व्यंजक *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  व्यंजक sCol[2];           /* Index column converted into an Expr */
  पूर्णांक nClause = 0;        /* Counter of ON CONFLICT clauses */

  निश्चित( pTabList->nSrc==1 );
  निश्चित( pTabList->a[0].pSTab!=0 );
  निश्चित( pUpsert!=0 );
  निश्चित( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  स_रखो(&sNC, 0, माप(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  क्रम(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->p_अग्र_Upsert, nClause++){
    rc = sqlite3_Resolve_व्यंजक_List_Names(&sNC, pUpsert->pUpsertTarget);
    अगर( rc ) वापस rc;
    rc = sqlite3_Resolve_व्यंजक_Names(&sNC, pUpsert->pUpsertTargetWhere);
    अगर( rc ) वापस rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    अगर( HasRowid(pTab) 
     && pTarget->n_व्यंजक==1
     && (pTerm = pTarget->a[0].p_व्यंजक)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      निश्चित( pUpsert->pUpsertIdx==0 );
      जारी;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    स_रखो(sCol, 0, माप(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_बायाँ = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
      पूर्णांक ii, jj, nn;
      अगर( !Is_Unique_सूचकांक(pIdx) ) जारी;
      अगर( pTarget->n_व्यंजक!=pIdx->n_कुंजी_Col ) जारी;
      अगर( pIdx->pPartIdxWhere ){
        अगर( pUpsert->pUpsertTargetWhere==0 ) जारी;
        अगर( sqlite3_व्यंजक_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          जारी;
        }
      }
      nn = pIdx->n_कुंजी_Col;
      क्रम(ii=0; ii<nn; ii++){
        व्यंजक *p_व्यंजक;
        sCol[0].u.zToken = (अक्षर*)pIdx->azColl[ii];
        अगर( pIdx->aiColumn[ii]==XN_व्यंजक ){
          निश्चित( pIdx->a_Col_व्यंजक!=0 );
          निश्चित( pIdx->a_Col_व्यंजक->n_व्यंजक>ii );
          निश्चित( pIdx->b_Has_व्यंजक );
          p_व्यंजक = pIdx->a_Col_व्यंजक->a[ii].p_व्यंजक;
          अगर( p_व्यंजक->op!=TK_COLLATE ){
            sCol[0].p_बायाँ = p_व्यंजक;
            p_व्यंजक = &sCol[0];
          }
        }अन्यथा{
          sCol[0].p_बायाँ = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_व्यंजक = &sCol[0];
        }
        क्रम(jj=0; jj<nn; jj++){
          अगर( sqlite3_व्यंजक_Compare(0,pTarget->a[jj].p_व्यंजक,p_व्यंजक,iCursor)<2 ){
            अवरोध;  /* Column ii of the index matches column jj of target */
          }
        }
        अगर( jj>=nn ){
          /* The target contains no match for column jj of the index */
          अवरोध;
        }
      }
      अगर( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        जारी;
      }
      pUpsert->pUpsertIdx = pIdx;
      अगर( sqlite3_Upsert_Of_सूचकांक(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      अवरोध;
    }
    अगर( pUpsert->pUpsertIdx==0 ){
      अक्षर zWhich[16];
      अगर( nClause==0 && pUpsert->p_अग्र_Upsert==0 ){
        zWhich[0] = 0;
      }अन्यथा{
        sqlite3_snprintf(माप(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_संदेश(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      वापस SQLITE_ERROR;
    }
  }
  वापस SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
पूर्णांक sqlite3_Upsert_अग्र_Is_IPK(Upsert *pUpsert){
  Upsert *p_अग्र;
  अगर( NEVER(pUpsert==0) ) वापस 0;
  p_अग्र = pUpsert->p_अग्र_Upsert;
  जबतक( 1 /*exit-by-return*/ ){
    अगर( p_अग्र==0 ) वापस 1;
    अगर( p_अग्र->pUpsertTarget==0 ) वापस 1;
    अगर( p_अग्र->pUpsertIdx==0 ) वापस 1;
    अगर( !p_अग्र->isDup ) वापस 0;
    p_अग्र = p_अग्र->p_अग्र_Upsert;
  }
  वापस 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_Of_सूचकांक(Upsert *pUpsert, सूचकांक *pIdx){
  जबतक(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_अग्र_Upsert;
  }
  वापस pUpsert;
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
व्योम sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  सूचकांक *pIdx,          /* The UNIQUE constraint that failed */
  पूर्णांक iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  पूर्णांक i_तथ्य_Cur;
  पूर्णांक i;
  Upsert *p_शीर्ष_2 = pUpsert;

  निश्चित( v!=0 );
  निश्चित( pUpsert!=0 );
  i_तथ्य_Cur = pUpsert->i_तथ्य_Cur;
  pUpsert = sqlite3_Upsert_Of_सूचकांक(p_शीर्ष_2, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  अगर( pIdx && iCur!=i_तथ्य_Cur ){
    अगर( HasRowid(pTab) ){
      पूर्णांक regRowid = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, i_तथ्य_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_अस्थायी_Reg(pParse, regRowid);
    }अन्यथा{
      सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      पूर्णांक nPk = pPk->n_कुंजी_Col;
      पूर्णांक iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      क्रम(i=0; i<nPk; i++){
        पूर्णांक k;
        निश्चित( pPk->aiColumn[i]>=0 );
        k = sqlite3_Table_Column_To_सूचकांक(pIdx, pPk->aiColumn[i]);
        sqlite3VdbeAddOp3(v, OP_Column, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->aCol[pPk->aiColumn[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3VdbeAddOp4Int(v, OP_Found, i_तथ्य_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3VdbeJumpHere(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_शीर्ष_2->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  क्रम(i=0; i<pTab->nCol; i++){
    अगर( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      पूर्णांक iStorage = p_शीर्ष_2->reg_तथ्य + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_व्यंजक_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_व्यंजक_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#पूर्ण_अगर /* SQLITE_OMIT_UPSERT */
