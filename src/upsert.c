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
#અન્તર્ભાવ "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
સ્થિર શૂન્ય SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  કરો{
    Upsert *pNext = p->pNextUpsert;
    sqlite3_પદાવલિ_List_Delete(db, p->pUpsertTarget);
    sqlite3_પદાવલિ_Delete(db, p->pUpsertTargetWhere);
    sqlite3_પદાવલિ_List_Delete(db, p->pUpsertSet);
    sqlite3_પદાવલિ_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  }જ્યારે( p );
}
શૂન્ય sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  જો( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  જો( p==0 ) પાછા 0;
  પાછા sqlite3UpsertNew(db,
           sqlite3_પદાવલિ_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_પદાવલિ_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_પદાવલિ_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_પદાવલિ_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->pNextUpsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  પદાવલિ_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  પદાવલિ *pTargetWhere,    /* Optional WHERE clause on the target */
  પદાવલિ_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  પદાવલિ *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *pNext          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, કદ(Upsert));
  જો( pNew==0 ){
    sqlite3_પદાવલિ_List_Delete(db, pTarget);
    sqlite3_પદાવલિ_Delete(db, pTargetWhere);
    sqlite3_પદાવલિ_List_Delete(db, pSet);
    sqlite3_પદાવલિ_Delete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    પાછા 0;
  }નહીંતર{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->pNextUpsert = pNext;
  }
  પાછા pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
પૂર્ણાંક sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  પૂર્ણાંક rc;                 /* Result code */
  પૂર્ણાંક iCursor;            /* Cursor used by pTab */
  Index *pIdx;            /* One of the indexes of pTab */
  પદાવલિ_List *pTarget;      /* The conflict-target clause */
  પદાવલિ *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  પદાવલિ sCol[2];           /* Index column converted into an Expr */
  પૂર્ણાંક nClause = 0;        /* Counter of ON CONFLICT clauses */

  પ્રતિજ્ઞા( pTabList->nSrc==1 );
  પ્રતિજ્ઞા( pTabList->a[0].pSTab!=0 );
  પ્રતિજ્ઞા( pUpsert!=0 );
  પ્રતિજ્ઞા( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, કદ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  માટે(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->pNextUpsert, nClause++){
    rc = sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pUpsert->pUpsertTarget);
    જો( rc ) પાછા rc;
    rc = sqlite3_Resolve_પદાવલિ_Names(&sNC, pUpsert->pUpsertTargetWhere);
    જો( rc ) પાછા rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    જો( HasRowid(pTab) 
     && pTarget->n_પદાવલિ==1
     && (pTerm = pTarget->a[0].p_પદાવલિ)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      પ્રતિજ્ઞા( pUpsert->pUpsertIdx==0 );
      ચાલુ;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, કદ(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_ડાબું = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    માટે(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      પૂર્ણાંક ii, jj, nn;
      જો( !IsUniqueIndex(pIdx) ) ચાલુ;
      જો( pTarget->n_પદાવલિ!=pIdx->n_ચાવી_Col ) ચાલુ;
      જો( pIdx->pPartIdxWhere ){
        જો( pUpsert->pUpsertTargetWhere==0 ) ચાલુ;
        જો( sqlite3_પદાવલિ_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          ચાલુ;
        }
      }
      nn = pIdx->n_ચાવી_Col;
      માટે(ii=0; ii<nn; ii++){
        પદાવલિ *p_પદાવલિ;
        sCol[0].u.zToken = (અક્ષર*)pIdx->azColl[ii];
        જો( pIdx->aiColumn[ii]==XN_પદાવલિ ){
          પ્રતિજ્ઞા( pIdx->a_Col_પદાવલિ!=0 );
          પ્રતિજ્ઞા( pIdx->a_Col_પદાવલિ->n_પદાવલિ>ii );
          પ્રતિજ્ઞા( pIdx->b_Has_પદાવલિ );
          p_પદાવલિ = pIdx->a_Col_પદાવલિ->a[ii].p_પદાવલિ;
          જો( p_પદાવલિ->op!=TK_COLLATE ){
            sCol[0].p_ડાબું = p_પદાવલિ;
            p_પદાવલિ = &sCol[0];
          }
        }નહીંતર{
          sCol[0].p_ડાબું = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_પદાવલિ = &sCol[0];
        }
        માટે(jj=0; jj<nn; jj++){
          જો( sqlite3_પદાવલિ_Compare(0,pTarget->a[jj].p_પદાવલિ,p_પદાવલિ,iCursor)<2 ){
            તોડો;  /* Column ii of the index matches column jj of target */
          }
        }
        જો( jj>=nn ){
          /* The target contains no match for column jj of the index */
          તોડો;
        }
      }
      જો( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        ચાલુ;
      }
      pUpsert->pUpsertIdx = pIdx;
      જો( sqlite3UpsertOfIndex(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      તોડો;
    }
    જો( pUpsert->pUpsertIdx==0 ){
      અક્ષર zWhich[16];
      જો( nClause==0 && pUpsert->pNextUpsert==0 ){
        zWhich[0] = 0;
      }નહીંતર{
        sqlite3_snprintf(કદ(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_સંદેશ(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      પાછા SQLITE_ERROR;
    }
  }
  પાછા SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
પૂર્ણાંક sqlite3UpsertNextIsIPK(Upsert *pUpsert){
  Upsert *pNext;
  જો( NEVER(pUpsert==0) ) પાછા 0;
  pNext = pUpsert->pNextUpsert;
  જ્યારે( 1 /*exit-by-return*/ ){
    જો( pNext==0 ) પાછા 1;
    જો( pNext->pUpsertTarget==0 ) પાછા 1;
    જો( pNext->pUpsertIdx==0 ) પાછા 1;
    જો( !pNext->isDup ) પાછા 0;
    pNext = pNext->pNextUpsert;
  }
  પાછા 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3UpsertOfIndex(Upsert *pUpsert, Index *pIdx){
  જ્યારે(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->pNextUpsert;
  }
  પાછા pUpsert;
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
શૂન્ય sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  Index *pIdx,          /* The UNIQUE constraint that failed */
  પૂર્ણાંક iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  પૂર્ણાંક iDataCur;
  પૂર્ણાંક i;
  Upsert *p_ટોચ = pUpsert;

  પ્રતિજ્ઞા( v!=0 );
  પ્રતિજ્ઞા( pUpsert!=0 );
  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(p_ટોચ, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  જો( pIdx && iCur!=iDataCur ){
    જો( HasRowid(pTab) ){
      પૂર્ણાંક regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, iDataCur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3ReleaseTempReg(pParse, regRowid);
    }નહીંતર{
      Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
      પૂર્ણાંક nPk = pPk->n_ચાવી_Col;
      પૂર્ણાંક iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      માટે(i=0; i<nPk; i++){
        પૂર્ણાંક k;
        પ્રતિજ્ઞા( pPk->aiColumn[i]>=0 );
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
  pSrc = sqlite3SrcListDup(db, p_ટોચ->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  માટે(i=0; i<pTab->nCol; i++){
    જો( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      પૂર્ણાંક iStorage = p_ટોચ->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_પદાવલિ_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_પદાવલિ_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
