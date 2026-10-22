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
#অন্তর্ভাব "sqliteInt.h"

#ifndef SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
স্থির শূন্য SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  করো{
    Upsert *p_পরবর্তী = p->p_পরবর্তী_Upsert;
    sqlite3_রাশিমালা_List_Delete(db, p->pUpsertTarget);
    sqlite3_রাশিমালা_Delete(db, p->pUpsertTargetWhere);
    sqlite3_রাশিমালা_List_Delete(db, p->pUpsertSet);
    sqlite3_রাশিমালা_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = p_পরবর্তী;
  }যখন( p );
}
শূন্য sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  যদি( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  যদি( p==0 ) ফেরত 0;
  ফেরত sqlite3UpsertNew(db,
           sqlite3_রাশিমালা_List_Dup(db, p->pUpsertTarget, 0),
           sqlite3_রাশিমালা_Dup(db, p->pUpsertTargetWhere, 0),
           sqlite3_রাশিমালা_List_Dup(db, p->pUpsertSet, 0),
           sqlite3_রাশিমালা_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_পরবর্তী_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  রাশিমালা_List *pTarget,     /* Target argument to ON CONFLICT, or NULL */
  রাশিমালা *pTargetWhere,    /* Optional WHERE clause on the target */
  রাশিমালা_List *pSet,        /* UPDATE columns, or NULL for a DO NOTHING */
  রাশিমালা *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_পরবর্তী          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, আকার(Upsert));
  যদি( pNew==0 ){
    sqlite3_রাশিমালা_List_Delete(db, pTarget);
    sqlite3_রাশিমালা_Delete(db, pTargetWhere);
    sqlite3_রাশিমালা_List_Delete(db, pSet);
    sqlite3_রাশিমালা_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_পরবর্তী);
    ফেরত 0;
  }অন্যথা{
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet!=0;
    pNew->p_পরবর্তী_Upsert = p_পরবর্তী;
  }
  ফেরত pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
পূর্ণসংখ্যা sqlite3UpsertAnalyzeTarget(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  Table *pTab;            /* That table into which we are inserting */
  পূর্ণসংখ্যা rc;                 /* Result code */
  পূর্ণসংখ্যা iCursor;            /* Cursor used by pTab */
  সূচক *pIdx;            /* One of the indexes of pTab */
  রাশিমালা_List *pTarget;      /* The conflict-target clause */
  রাশিমালা *pTerm;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  রাশিমালা sCol[2];           /* Index column converted into an Expr */
  পূর্ণসংখ্যা nClause = 0;        /* Counter of ON CONFLICT clauses */

  প্রতিজ্ঞা( pTabList->nSrc==1 );
  প্রতিজ্ঞা( pTabList->a[0].pSTab!=0 );
  প্রতিজ্ঞা( pUpsert!=0 );
  প্রতিজ্ঞা( pUpsert->pUpsertTarget!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, আকার(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  জন্য(; pUpsert && pUpsert->pUpsertTarget;
        pUpsert=pUpsert->p_পরবর্তী_Upsert, nClause++){
    rc = sqlite3_Resolve_রাশিমালা_List_Names(&sNC, pUpsert->pUpsertTarget);
    যদি( rc ) ফেরত rc;
    rc = sqlite3_Resolve_রাশিমালা_Names(&sNC, pUpsert->pUpsertTargetWhere);
    যদি( rc ) ফেরত rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    যদি( HasRowid(pTab) 
     && pTarget->n_রাশিমালা==1
     && (pTerm = pTarget->a[0].p_রাশিমালা)->op==TK_COLUMN
     && pTerm->iColumn==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      প্রতিজ্ঞা( pUpsert->pUpsertIdx==0 );
      চালু;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(sCol, 0, আকার(sCol));
    sCol[0].op = TK_COLLATE;
    sCol[0].p_বাম = &sCol[1];
    sCol[1].op = TK_COLUMN;
    sCol[1].iTable = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
      পূর্ণসংখ্যা ii, jj, nn;
      যদি( !Is_Unique_সূচক(pIdx) ) চালু;
      যদি( pTarget->n_রাশিমালা!=pIdx->n_চাবি_Col ) চালু;
      যদি( pIdx->pPartIdxWhere ){
        যদি( pUpsert->pUpsertTargetWhere==0 ) চালু;
        যদি( sqlite3_রাশিমালা_Compare(pParse, pUpsert->pUpsertTargetWhere,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          চালু;
        }
      }
      nn = pIdx->n_চাবি_Col;
      জন্য(ii=0; ii<nn; ii++){
        রাশিমালা *p_রাশিমালা;
        sCol[0].u.zToken = (অক্ষর*)pIdx->azColl[ii];
        যদি( pIdx->aiColumn[ii]==XN_রাশিমালা ){
          প্রতিজ্ঞা( pIdx->a_Col_রাশিমালা!=0 );
          প্রতিজ্ঞা( pIdx->a_Col_রাশিমালা->n_রাশিমালা>ii );
          প্রতিজ্ঞা( pIdx->b_Has_রাশিমালা );
          p_রাশিমালা = pIdx->a_Col_রাশিমালা->a[ii].p_রাশিমালা;
          যদি( p_রাশিমালা->op!=TK_COLLATE ){
            sCol[0].p_বাম = p_রাশিমালা;
            p_রাশিমালা = &sCol[0];
          }
        }অন্যথা{
          sCol[0].p_বাম = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          p_রাশিমালা = &sCol[0];
        }
        জন্য(jj=0; jj<nn; jj++){
          যদি( sqlite3_রাশিমালা_Compare(0,pTarget->a[jj].p_রাশিমালা,p_রাশিমালা,iCursor)<2 ){
            বিরতি;  /* Column ii of the index matches column jj of target */
          }
        }
        যদি( jj>=nn ){
          /* The target contains no match for column jj of the index */
          বিরতি;
        }
      }
      যদি( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        চালু;
      }
      pUpsert->pUpsertIdx = pIdx;
      যদি( sqlite3_Upsert_Of_সূচক(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      বিরতি;
    }
    যদি( pUpsert->pUpsertIdx==0 ){
      অক্ষর zWhich[16];
      যদি( nClause==0 && pUpsert->p_পরবর্তী_Upsert==0 ){
        zWhich[0] = 0;
      }অন্যথা{
        sqlite3_snprintf(আকার(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_বার্তা(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      ফেরত SQLITE_ERROR;
    }
  }
  ফেরত SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
পূর্ণসংখ্যা sqlite3_Upsert_পরবর্তী_Is_IPK(Upsert *pUpsert){
  Upsert *p_পরবর্তী;
  যদি( NEVER(pUpsert==0) ) ফেরত 0;
  p_পরবর্তী = pUpsert->p_পরবর্তী_Upsert;
  যখন( 1 /*exit-by-return*/ ){
    যদি( p_পরবর্তী==0 ) ফেরত 1;
    যদি( p_পরবর্তী->pUpsertTarget==0 ) ফেরত 1;
    যদি( p_পরবর্তী->pUpsertIdx==0 ) ফেরত 1;
    যদি( !p_পরবর্তী->isDup ) ফেরত 0;
    p_পরবর্তী = p_পরবর্তী->p_পরবর্তী_Upsert;
  }
  ফেরত 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_Of_সূচক(Upsert *pUpsert, সূচক *pIdx){
  যখন(
      pUpsert
   && pUpsert->pUpsertTarget!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_পরবর্তী_Upsert;
  }
  ফেরত pUpsert;
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
শূন্য sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  Table *pTab,          /* The table being updated */
  সূচক *pIdx,          /* The UNIQUE constraint that failed */
  পূর্ণসংখ্যা iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  পূর্ণসংখ্যা i_তথ্য_Cur;
  পূর্ণসংখ্যা i;
  Upsert *p_শীর্ষ_2 = pUpsert;

  প্রতিজ্ঞা( v!=0 );
  প্রতিজ্ঞা( pUpsert!=0 );
  i_তথ্য_Cur = pUpsert->i_তথ্য_Cur;
  pUpsert = sqlite3_Upsert_Of_সূচক(p_শীর্ষ_2, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  যদি( pIdx && iCur!=i_তথ্য_Cur ){
    যদি( HasRowid(pTab) ){
      পূর্ণসংখ্যা regRowid = sqlite3_Get_অস্থায়ী_Reg(pParse);
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3VdbeAddOp3(v, OP_SeekRowid, i_তথ্য_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_অস্থায়ী_Reg(pParse, regRowid);
    }অন্যথা{
      সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
      পূর্ণসংখ্যা nPk = pPk->n_চাবি_Col;
      পূর্ণসংখ্যা iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      জন্য(i=0; i<nPk; i++){
        পূর্ণসংখ্যা k;
        প্রতিজ্ঞা( pPk->aiColumn[i]>=0 );
        k = sqlite3_Table_Column_To_সূচক(pIdx, pPk->aiColumn[i]);
        sqlite3VdbeAddOp3(v, OP_Column, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->aCol[pPk->aiColumn[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3VdbeAddOp4Int(v, OP_Found, i_তথ্য_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3VdbeJumpHere(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_শীর্ষ_2->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  জন্য(i=0; i<pTab->nCol; i++){
    যদি( pTab->aCol[i].affinity==SQLITE_AFF_REAL ){
      পূর্ণসংখ্যা iStorage = p_শীর্ষ_2->reg_তথ্য + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_রাশিমালা_List_Dup(db,pUpsert->pUpsertSet,0),
      sqlite3_রাশিমালা_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#endif /* SQLITE_OMIT_UPSERT */
