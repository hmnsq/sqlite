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

#यदि_अपरिभाषितम् SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
स्थिर शून्य SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  कुरु{
    Upsert *p_अग्र = p->p_अग्र_Upsert;
    sqlite3_व्यञ्जकम्_List_Delete(db, p->pUpsertTarget);
    sqlite3_व्यञ्जकम्_Delete(db, p->pUpsertTargetWhere);
    sqlite3_व्यञ्जकम्_List_Delete(db, p->p_Upsert_समुच्चयः);
    sqlite3_व्यञ्जकम्_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_गन्तव्यम्_Free);
    sqlite3DbFree(db, p);
    p = p_अग्र;
  }यावत्( p );
}
शून्य sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  यदि( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  यदि( p==0 ) प्रत्यागम 0;
  प्रत्यागम sqlite3UpsertNew(db,
           sqlite3_व्यञ्जकम्_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_व्यञ्जकम्_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_व्यञ्जकम्_List_Dup(db, p->p_Upsert_समुच्चयः, 0),
           sqlite3_व्यञ्जकम्_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_अग्र_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  व्यञ्जकम्_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  व्यञ्जकम् *pTargetWhere,    /* Optional WHERE clause on the target */
  व्यञ्जकम्_List *p_समुच्चयः,        /* UPDATE columns, or NULL for a DO NOTHING */
  व्यञ्जकम् *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_अग्र          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, परिमाण(Upsert));
  यदि( pNew==0 ){
    sqlite3_व्यञ्जकम्_List_Delete(db, pTarget);
    sqlite3_व्यञ्जकम्_Delete(db, pTargetWhere);
    sqlite3_व्यञ्जकम्_List_Delete(db, p_समुच्चयः);
    sqlite3_व्यञ्जकम्_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_अग्र);
    प्रत्यागम 0;
  }अन्यथा{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->p_Upsert_समुच्चयः = p_समुच्चयः;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_समुच्चयः!=0;
    pNew->p_अग्र_Upsert = p_अग्र;
  }
  प्रत्यागम pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
पूर्णाङ्क sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  सारणी *pTab;            /* That table into which we are inserting */
  पूर्णाङ्क rc;                 /* Result code */
  पूर्णाङ्क iCursor;            /* Cursor used by pTab */
  सूचकांक *pIdx;            /* One of the indexes of pTab */
  व्यञ्जकम्_List *pTarget;      /* The conflict-target clause */
  व्यञ्जकम् *p_पदांशः;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  व्यञ्जकम् sCol[2];           /* Index column converted into an Expr */
  पूर्णाङ्क nClause = 0;        /* Counter of ON CONFLICT clauses */

  प्रतिज्ञा( pTabList->nSrc==1 );
  प्रतिज्ञा( pTabList->a[0].pSTab!=0 );
  प्रतिज्ञा( pUpsert!=0 );
  प्रतिज्ञा( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  प्रति(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->p_अग्र_Upsert, nClause++){
    rc = sqlite3_Resolve_व्यञ्जकम्_List_नामानि(&sNC, pUpsert->pUpsertTarget);
    यदि( rc ) प्रत्यागम rc;
    rc = sqlite3_Resolve_व्यञ्जकम्_नामानि(&sNC, pUpsert->pUpsertTargetWhere);
    यदि( rc ) प्रत्यागम rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    यदि( HasRowid(pTab) 
     && pTarget->n_व्यञ्जकम्==1
     && (p_पदांशः = pTarget->a[0].p_व्यञ्जकम्)->op==TK_COLUMN
     && p_पदांशः->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      प्रतिज्ञा( pUpsert->pUpsertIdx==0 );
      अनुवर्त;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    स्मृतिं_पूरय(sCol, 0, परिमाण(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_वामः = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].i_सारणी = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
      पूर्णाङ्क ii, jj, nn;
      यदि( !Is_Unique_सूचकांक(pIdx) ) अनुवर्त;
      यदि( pTarget->n_व्यञ्जकम्!=pIdx->n_कुञ्चिका_Col ) अनुवर्त;
      यदि( pIdx->pPartIdxWhere ){
        यदि( pUpsert->pUpsertTargetWhere==0 ) अनुवर्त;
        यदि( sqlite3_व्यञ्जकम्_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          अनुवर्त;
        }
      }
      nn = pIdx->n_कुञ्चिका_Col;
      प्रति(ii=0; ii<nn; ii++){
        व्यञ्जकम् *p_व्यञ्जकम्;
        sCol[0].u.zToken = (वर्ण*)pIdx->azColl[ii];
        यदि( pIdx->aiColumn[ii]==XN_व्यञ्जकम् ){
          प्रतिज्ञा( pIdx->a_Col_व्यञ्जकम्!=0 );
          प्रतिज्ञा( pIdx->a_Col_व्यञ्जकम्->n_व्यञ्जकम्>ii );
          प्रतिज्ञा( pIdx->b_Has_व्यञ्जकम् );
          p_व्यञ्जकम् = pIdx->a_Col_व्यञ्जकम्->a[ii].p_व्यञ्जकम्;
          यदि( p_व्यञ्जकम्->op!=TK_COLLATE ){
            sCol[0].p_वामः = p_व्यञ्जकम्;
            p_व्यञ्जकम् = &sCol[0];
          }
        }अन्यथा{
          sCol[0].p_वामः = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_व्यञ्जकम् = &sCol[0];
        }
        प्रति(jj=0; jj<nn; jj++){
          यदि( sqlite3_व्यञ्जकम्_Compare(0,pTarget->a[jj].p_व्यञ्जकम्,p_व्यञ्जकम्,iCursor)<2 ){
            भङ्ग;  /* Column ii of the index matches column jj of target */
          }
        }
        यदि( jj>=nn ){
          /* The target contains no match for column jj of the index */
          भङ्ग;
        }
      }
      यदि( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        अनुवर्त;
      }
      pUpsert->pUpsertIdx = pIdx;
      यदि( sqlite3_Upsert_सम्बन्धि_सूचकांक(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      भङ्ग;
    }
    यदि( pUpsert->pUpsertIdx==0 ){
      वर्ण zWhich[16];
      यदि( nClause==0 && pUpsert->p_अग्र_Upsert==0 ){
        zWhich[0] = 0;
      }अन्यथा{
        sqlite3_snprintf(परिमाण(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_सन्देशः(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      प्रत्यागम SQLITE_ERROR;
    }
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
पूर्णाङ्क sqlite3_Upsert_अग्र_Is_IPK(Upsert *pUpsert){
  Upsert *p_अग्र;
  यदि( NEVER(pUpsert==0) ) प्रत्यागम 0;
  p_अग्र = pUpsert->p_अग्र_Upsert;
  यावत्( 1 /*exit-by-return*/ ){
    यदि( p_अग्र==0 ) प्रत्यागम 1;
    यदि( p_अग्र->pUpsertTarget==0 ) प्रत्यागम 1;
    यदि( p_अग्र->pUpsertIdx==0 ) प्रत्यागम 1;
    यदि( !p_अग्र->isDup ) प्रत्यागम 0;
    p_अग्र = p_अग्र->p_अग्र_Upsert;
  }
  प्रत्यागम 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_सम्बन्धि_सूचकांक(Upsert *pUpsert, सूचकांक *pIdx){
  यावत्(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_अग्र_Upsert;
  }
  प्रत्यागम pUpsert;
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
  सारणी *pTab,          /* The table being updated */
  सूचकांक *pIdx,          /* The UNIQUE constraint that failed */
  पूर्णाङ्क iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  पूर्णाङ्क i_दत्त_Cur;
  पूर्णाङ्क i;
  Upsert *p_शिखरम् = pUpsert;

  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pUpsert!=0 );
  i_दत्त_Cur = pUpsert->i_दत्त_Cur;
  pUpsert = sqlite3_Upsert_सम्बन्धि_सूचकांक(p_शिखरम्, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  यदि( pIdx && iCur!=i_दत्त_Cur ){
    यदि( HasRowid(pTab) ){
      पूर्णाङ्क regRowid = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_SeekRowid, i_दत्त_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_अस्थायी_Reg(pParse, regRowid);
    }अन्यथा{
      सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      पूर्णाङ्क nPk = pPk->n_कुञ्चिका_Col;
      पूर्णाङ्क iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      प्रति(i=0; i<nPk; i++){
        पूर्णाङ्क k;
        प्रतिज्ञा( pPk->aiColumn[i]>=0 );
        k = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(pIdx, pPk->aiColumn[i]);
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->aCol[pPk->aiColumn[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, i_दत्त_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_अत्र(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_शिखरम्->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  प्रति(i=0; i<pTab->nCol; i++){
    यदि( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      पूर्णाङ्क iStorage = p_शिखरम्->reg_दत्त + sqlite3_सारणी_Column_गन्तव्यम्_Storage(pTab, i);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_व्यञ्जकम्_List_Dup(db,pUpsert->p_Upsert_समुच्चयः,0),
      sqlite3_व्यञ्जकम्_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#यदि_अन्तः /* SQLITE_OMIT_UPSERT */
