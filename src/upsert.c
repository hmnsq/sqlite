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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
/*
** Free a list of Upsert objects
*/
స్థిర శూన్య SQLITE_NOINLINE upsertDelete(sqlite3 *db, Upsert *p){
  చేయి{
    Upsert *p_తదుపరి = p->p_తదుపరి_Upsert;
    sqlite3_సమాసం_List_Delete(db, p->p_Upsert_లక్ష్యం);
    sqlite3_సమాసం_Delete(db, p->p_Upsert_లక్ష్యం_Where);
    sqlite3_సమాసం_List_Delete(db, p->p_Upsert_సమితి);
    sqlite3_సమాసం_Delete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->p_గమ్యం_Free);
    sqlite3DbFree(db, p);
    p = p_తదుపరి;
  }అయితే( p );
}
శూన్య sqlite3UpsertDelete(sqlite3 *db, Upsert *p){
  ఒకవేళ( p ) upsertDelete(db, p);
}


/*
** Duplicate an Upsert object.
*/
Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p){
  ఒకవేళ( p==0 ) తిరిగి 0;
  తిరిగి sqlite3UpsertNew(db,
           sqlite3_సమాసం_List_Dup(db, p->p_Upsert_లక్ష్యం, 0),
           sqlite3_సమాసం_Dup(db, p->p_Upsert_లక్ష్యం_Where, 0),
           sqlite3_సమాసం_List_Dup(db, p->p_Upsert_సమితి, 0),
           sqlite3_సమాసం_Dup(db, p->pUpsertWhere, 0),
           sqlite3UpsertDup(db, p->p_తదుపరి_Upsert)
         );
}

/*
** Create a new Upsert object.
*/
Upsert *sqlite3UpsertNew(
  sqlite3 *db,           /* Determines which memory allocator to use */
  సమాసం_List *p_లక్ష్యం,     /* Target argument to ON CONFLICT, or NULL */
  సమాసం *p_లక్ష్యం_Where,    /* Optional WHERE clause on the target */
  సమాసం_List *p_సమితి,        /* UPDATE columns, or NULL for a DO NOTHING */
  సమాసం *pWhere,          /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_తదుపరి          /* Next ON CONFLICT clause in the list */
){
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, పరిమాణం(Upsert));
  ఒకవేళ( pNew==0 ){
    sqlite3_సమాసం_List_Delete(db, p_లక్ష్యం);
    sqlite3_సమాసం_Delete(db, p_లక్ష్యం_Where);
    sqlite3_సమాసం_List_Delete(db, p_సమితి);
    sqlite3_సమాసం_Delete(db, pWhere);
    sqlite3UpsertDelete(db, p_తదుపరి);
    తిరిగి 0;
  }లేకపోతే{
    pNew->p_Upsert_లక్ష్యం = p_లక్ష్యం;
    pNew->p_Upsert_లక్ష్యం_Where = p_లక్ష్యం_Where;
    pNew->p_Upsert_సమితి = p_సమితి;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = p_సమితి!=0;
    pNew->p_తదుపరి_Upsert = p_తదుపరి;
  }
  తిరిగి pNew;
}

/*
** Analyze the ON CONFLICT clause described by pUpsert.  Resolve all
** symbols in the conflict-target.
**
** Return SQLITE_OK if everything works, or an error code is something
** is wrong.
*/
పూర్ణాంకం sqlite3_Upsert_Analyze_లక్ష్యం(
  Parse *pParse,     /* The parsing context */
  SrcList *pTabList, /* Table into which we are inserting */
  Upsert *pUpsert,   /* The ON CONFLICT clauses */
  Upsert *pAll       /* Complete list of all ON CONFLICT clauses */
){
  పట్టిక *pTab;            /* That table into which we are inserting */
  పూర్ణాంకం rc;                 /* Result code */
  పూర్ణాంకం iCursor;            /* Cursor used by pTab */
  సూచీ *pIdx;            /* One of the indexes of pTab */
  సమాసం_List *p_లక్ష్యం;      /* The conflict-target clause */
  సమాసం *p_పదం;            /* One term of the conflict-target clause */
  NameContext sNC;        /* Context for resolving symbolic names */
  సమాసం s_నిలువు[2];           /* Index column converted into an Expr */
  పూర్ణాంకం nClause = 0;        /* Counter of ON CONFLICT clauses */

  ప్రతిజ్ఞా( pTabList->nSrc==1 );
  ప్రతిజ్ఞా( pTabList->a[0].pSTab!=0 );
  ప్రతిజ్ఞా( pUpsert!=0 );
  ప్రతిజ్ఞా( pUpsert->p_Upsert_లక్ష్యం!=0 );

  /* Resolve all symbolic names in the conflict-target clause, which
  ** includes both the list of columns and the optional partial-index
  ** WHERE clause.
  */
  memset(&sNC, 0, పరిమాణం(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  కొరకు(; pUpsert && pUpsert->p_Upsert_లక్ష్యం;
        pUpsert=pUpsert->p_తదుపరి_Upsert, nClause++){
    rc = sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pUpsert->p_Upsert_లక్ష్యం);
    ఒకవేళ( rc ) తిరిగి rc;
    rc = sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pUpsert->p_Upsert_లక్ష్యం_Where);
    ఒకవేళ( rc ) తిరిగి rc;
  
    /* Check to see if the conflict target matches the rowid. */  
    pTab = pTabList->a[0].pSTab;
    p_లక్ష్యం = pUpsert->p_Upsert_లక్ష్యం;
    iCursor = pTabList->a[0].iCursor;
    ఒకవేళ( HasRowid(pTab) 
     && p_లక్ష్యం->n_సమాసం==1
     && (p_పదం = p_లక్ష్యం->a[0].p_సమాసం)->op==TK_స్తంభం
     && p_పదం->i_స్తంభం==XN_ROWID
    ){
      /* The conflict-target is the rowid of the primary table */
      ప్రతిజ్ఞా( pUpsert->pUpsertIdx==0 );
      కొనసాగు;
    }
  
    /* Initialize sCol[0..1] to be an expression parse tree for a
    ** single column of an index.  The sCol[0] node will be the TK_COLLATE
    ** operator and sCol[1] will be the TK_COLUMN operator.  Code below
    ** will populate the specific collation and column number values
    ** prior to comparing against the conflict-target expression.
    */
    memset(s_నిలువు, 0, పరిమాణం(s_నిలువు));
    s_నిలువు[0].op = TK_COLLATE;
    s_నిలువు[0].p_ఎడమ = &s_నిలువు[1];
    s_నిలువు[1].op = TK_స్తంభం;
    s_నిలువు[1].i_పట్టిక = pTabList->a[0].iCursor;
  
    /* Check for matches against other indexes */
    కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
      పూర్ణాంకం ii, jj, nn;
      ఒకవేళ( !Is_Unique_సూచీ(pIdx) ) కొనసాగు;
      ఒకవేళ( p_లక్ష్యం->n_సమాసం!=pIdx->n_కీలకం_నిలువు ) కొనసాగు;
      ఒకవేళ( pIdx->pPartIdxWhere ){
        ఒకవేళ( pUpsert->p_Upsert_లక్ష్యం_Where==0 ) కొనసాగు;
        ఒకవేళ( sqlite3_సమాసం_Compare(pParse, pUpsert->p_Upsert_లక్ష్యం_Where,
                               pIdx->pPartIdxWhere, iCursor)!=0 ){
          కొనసాగు;
        }
      }
      nn = pIdx->n_కీలకం_నిలువు;
      కొరకు(ii=0; ii<nn; ii++){
        సమాసం *p_సమాసం;
        s_నిలువు[0].u.zToken = (అక్షరం*)pIdx->azColl[ii];
        ఒకవేళ( pIdx->ai_స్తంభం[ii]==XN_సమాసం ){
          ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం!=0 );
          ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం->n_సమాసం>ii );
          ప్రతిజ్ఞా( pIdx->b_Has_సమాసం );
          p_సమాసం = pIdx->a_నిలువు_సమాసం->a[ii].p_సమాసం;
          ఒకవేళ( p_సమాసం->op!=TK_COLLATE ){
            s_నిలువు[0].p_ఎడమ = p_సమాసం;
            p_సమాసం = &s_నిలువు[0];
          }
        }లేకపోతే{
          s_నిలువు[0].p_ఎడమ = &s_నిలువు[1];
          s_నిలువు[1].i_స్తంభం = pIdx->ai_స్తంభం[ii];
          p_సమాసం = &s_నిలువు[0];
        }
        కొరకు(jj=0; jj<nn; jj++){
          ఒకవేళ( sqlite3_సమాసం_Compare(0,p_లక్ష్యం->a[jj].p_సమాసం,p_సమాసం,iCursor)<2 ){
            ఆపు;  /* Column ii of the index matches column jj of target */
          }
        }
        ఒకవేళ( jj>=nn ){
          /* The target contains no match for column jj of the index */
          ఆపు;
        }
      }
      ఒకవేళ( ii<nn ){
        /* Column ii of the index did not match any term of the conflict target.
        ** Continue the search with the next index. */
        కొనసాగు;
      }
      pUpsert->pUpsertIdx = pIdx;
      ఒకవేళ( sqlite3_Upsert_యొక్క_సూచీ(pAll,pIdx)!=pUpsert ){
        /* Really this should be an error.  The isDup ON CONFLICT clause will
        ** never fire.  But this problem was not discovered until three years
        ** after multi-CONFLICT upsert was added, and so we silently ignore
        ** the problem to prevent breaking applications that might actually
        ** have redundant ON CONFLICT clauses. */
        pUpsert->isDup = 1;
      }
      ఆపు;
    }
    ఒకవేళ( pUpsert->pUpsertIdx==0 ){
      అక్షరం zWhich[16];
      ఒకవేళ( nClause==0 && pUpsert->p_తదుపరి_Upsert==0 ){
        zWhich[0] = 0;
      }లేకపోతే{
        sqlite3_snprintf(పరిమాణం(zWhich),zWhich,"%r ", nClause+1);
      }
      sqlite3_Error_సందేశం(pParse, "%sON CONFLICT clause does not match any "
                              "PRIMARY KEY or UNIQUE constraint", zWhich);
      తిరిగి SQLITE_ERROR;
    }
  }
  తిరిగి SQLITE_OK;
}

/*
** Return true if pUpsert is the last ON CONFLICT clause with a
** conflict target, or if pUpsert is followed by another ON CONFLICT
** clause that targets the INTEGER PRIMARY KEY.
*/
పూర్ణాంకం sqlite3_Upsert_తదుపరి_Is_IPK(Upsert *pUpsert){
  Upsert *p_తదుపరి;
  ఒకవేళ( NEVER(pUpsert==0) ) తిరిగి 0;
  p_తదుపరి = pUpsert->p_తదుపరి_Upsert;
  అయితే( 1 /*exit-by-return*/ ){
    ఒకవేళ( p_తదుపరి==0 ) తిరిగి 1;
    ఒకవేళ( p_తదుపరి->p_Upsert_లక్ష్యం==0 ) తిరిగి 1;
    ఒకవేళ( p_తదుపరి->pUpsertIdx==0 ) తిరిగి 1;
    ఒకవేళ( !p_తదుపరి->isDup ) తిరిగి 0;
    p_తదుపరి = p_తదుపరి->p_తదుపరి_Upsert;
  }
  తిరిగి 0;
}

/*
** Given the list of ON CONFLICT clauses described by pUpsert, and
** a particular index pIdx, return a pointer to the particular ON CONFLICT
** clause that applies to the index.  Or, if the index is not subject to
** any ON CONFLICT clause, return NULL.
*/
Upsert *sqlite3_Upsert_యొక్క_సూచీ(Upsert *pUpsert, సూచీ *pIdx){
  అయితే(
      pUpsert
   && pUpsert->p_Upsert_లక్ష్యం!=0
   && pUpsert->pUpsertIdx!=pIdx
  ){
     pUpsert = pUpsert->p_తదుపరి_Upsert;
  }
  తిరిగి pUpsert;
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
శూన్య sqlite3UpsertDoUpdate(
  Parse *pParse,        /* The parsing and code-generating context */
  Upsert *pUpsert,      /* The ON CONFLICT clause for the upsert */
  పట్టిక *pTab,          /* The table being updated */
  సూచీ *pIdx,          /* The UNIQUE constraint that failed */
  పూర్ణాంకం iCur              /* Cursor for pIdx (or pTab if pIdx==NULL) */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;            /* FROM clause for the UPDATE */
  పూర్ణాంకం i_దత్తాంశం_Cur;
  పూర్ణాంకం i;
  Upsert *p_శిఖరం = pUpsert;

  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pUpsert!=0 );
  i_దత్తాంశం_Cur = pUpsert->i_దత్తాంశం_Cur;
  pUpsert = sqlite3_Upsert_యొక్క_సూచీ(p_శిఖరం, pIdx);
  VdbeNoopComment((v, "Begin DO UPDATE of UPSERT"));
  ఒకవేళ( pIdx && iCur!=i_దత్తాంశం_Cur ){
    ఒకవేళ( HasRowid(pTab) ){
      పూర్ణాంకం regRowid = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_IdxRowid, iCur, regRowid);
      sqlite3_Vdbe_కూడు_Op3(v, OP_SeekRowid, i_దత్తాంశం_Cur, 0, regRowid);
      VdbeCoverage(v);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, regRowid);
    }లేకపోతే{
      సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      పూర్ణాంకం nPk = pPk->n_కీలకం_నిలువు;
      పూర్ణాంకం iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      కొరకు(i=0; i<nPk; i++){
        పూర్ణాంకం k;
        ప్రతిజ్ఞా( pPk->ai_స్తంభం[i]>=0 );
        k = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pIdx, pPk->ai_స్తంభం[i]);
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iCur, k, iPk+i);
        VdbeComment((v, "%s.%s", pIdx->zName,
                    pTab->a_నిలువు[pPk->ai_స్తంభం[i]].zCnName));
      }
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      i = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, i_దత్తాంశం_Cur, 0, iPk, nPk);
      VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op4(v, OP_Halt, SQLITE_CORRUPT, OE_Abort, 0, 
            "corrupt database", P4_STATIC);
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, i);
    }
  }
  /* pUpsert does not own pTop->pUpsertSrc - the outer INSERT statement does.
  ** So we have to make a copy before passing it down into sqlite3Update() */
  pSrc = sqlite3SrcListDup(db, p_శిఖరం->pUpsertSrc, 0);
  /* excluded.* columns of type REAL need to be converted to a hard real */
  కొరకు(i=0; i<pTab->n_నిలువు; i++){
    ఒకవేళ( pTab->a_నిలువు[i].affinity==SQLITE_AFF_REAL ){
      పూర్ణాంకం iStorage = p_శిఖరం->reg_దత్తాంశం + sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i);
      sqlite3_Vdbe_కూడు_Op1(v, OP_RealAffinity, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3_సమాసం_List_Dup(db,pUpsert->p_Upsert_సమితి,0),
      sqlite3_సమాసం_Dup(db,pUpsert->pUpsertWhere,0), OE_Abort, 0, 0, pUpsert);
  VdbeNoopComment((v, "End DO UPDATE of UPSERT"));
}

#అయితే_ముగింపు /* SQLITE_OMIT_UPSERT */
