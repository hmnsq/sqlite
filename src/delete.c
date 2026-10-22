/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that are called by the parser
** in order to generate code for DELETE FROM statements.
*/
#అన్తర్భావ "sqliteInt.h"

/*
** While a SrcList can in general represent multiple tables and subqueries
** (as in the FROM clause of a SELECT statement) in this case it contains
** the name of a single table, as one might find in an INSERT, DELETE,
** or UPDATE statement.  Look up that table in the symbol table and
** return a pointer.  Set an error message and return NULL if the table
** name is not found or if any other error occurs.
**
** The following fields are initialized appropriate in pSrc:
**
**    pSrc->a[0].spTab        Pointer to the Table object
**    pSrc->a[0].u2.pIBIndex  Pointer to the INDEXED BY index, if there is one
**
*/
పట్టిక *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  Src_అంశం *p_అంశం = pSrc->a;
  పట్టిక *pTab;
  ప్రతిజ్ఞా( p_అంశం && pSrc->nSrc>=1 );
  pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, p_అంశం);
  ఒకవేళ( p_అంశం->pSTab ) sqlite3_Delete_పట్టిక(pParse->db, p_అంశం->pSTab);
  p_అంశం->pSTab = pTab;
  p_అంశం->fg.notCte = 1;
  ఒకవేళ( pTab ){
    pTab->nTabRef++;
    ఒకవేళ( p_అంశం->fg.is_సూచీకృత_By && sqlite3_సూచీకృత_By_Lookup(pParse, p_అంశం) ){
      pTab = 0;
    }
  }
  తిరిగి pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
శూన్య sqlite3_సంకేతం_Change_లెక్క(Vdbe *v, పూర్ణాంకం regCounter, స్థిరరాశి అక్షరం *z_నిలువు_Name){
  sqlite3_Vdbe_కూడు_Op0(v, OP_FkCheck);
  sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, regCounter, 1);
  sqlite3_Vdbe_సమితి_అంకె_నిలువుల_సంఖ్య(v, 1);
  sqlite3_Vdbe_సమితి_నిలువు_Name(v, 0, COLNAME_NAME, z_నిలువు_Name, SQLITE_STATIC);
}

/* Return true if table pTab is read-only.
**
** A table is read-only if any of the following are true:
**
**   1) It is a virtual table and no implementation of the xUpdate method
**      has been provided
**
**   2) A trigger is currently being coded and the table is a virtual table
**      that is SQLITE_VTAB_DIRECTONLY or if PRAGMA trusted_schema=OFF and
**      the table is not SQLITE_VTAB_INNOCUOUS.
**
**   3) It is a system table (i.e. sqlite_schema), this call is not
**      part of a nested parse and writable_schema pragma has not
**      been specified
**
**   4) The table is a shadow table, the database connection is in
**      defensive mode, and the current sqlite3_prepare()
**      is for a top-level SQL statement.
*/
స్థిర పూర్ణాంకం vtabIsReadOnly(Parse *pParse, పట్టిక *pTab){
  ప్రతిజ్ఞా( IsVirtual(pTab) );
  ఒకవేళ( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    తిరిగి 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  ఒకవేళ( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_సందేశం(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  తిరిగి 0;
}
స్థిర పూర్ణాంకం tabIsReadOnly(Parse *pParse, పట్టిక *pTab){
  sqlite3 *db;
  ఒకవేళ( IsVirtual(pTab) ){
    తిరిగి vtabIsReadOnly(pParse, pTab);
  }
  ఒకవేళ( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) తిరిగి 0;
  db = pParse->db;
  ఒకవేళ( (pTab->tabFlags & TF_Readonly)!=0 ){
    తిరిగి sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  ప్రతిజ్ఞా( pTab->tabFlags & TF_Shadow );
  తిరిగి sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
పూర్ణాంకం sqlite3IsReadOnly(Parse *pParse, పట్టిక *pTab, Trigger *pTrigger){
  ఒకవేళ( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_సందేశం(pParse, "table %s may not be modified", pTab->zName);
    తిరిగి 1;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIEW
  ఒకవేళ( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->p_తదుపరి==0))
  ){
    sqlite3_Error_సందేశం(pParse,"cannot modify %s because it is a view",pTab->zName);
    తిరిగి 1;
  }
#అయితే_ముగింపు
  తిరిగి 0;
}


#ఒకవేళ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
శూన్య sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  పట్టిక *pView,        /* View definition */
  సమాసం *pWhere,        /* Optional WHERE clause to be added */
  సమాసం_List *p_క్రమం_By,  /* Optional ORDER BY clause */
  సమాసం *p_హద్దు,        /* Optional LIMIT clause */
  పూర్ణాంకం iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pView->pSchema);
  pWhere = sqlite3_సమాసం_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  ఒకవేళ( pFrom ){
    ప్రతిజ్ఞా( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    ప్రతిజ్ఞా( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    ప్రతిజ్ఞా( pFrom->a[0].fg.isUsing==0 );
    ప్రతిజ్ఞా( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, p_క్రమం_By,
                          SF_IncludeHidden, p_హద్దు);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#ఒకవేళ defined(SQLITE_ENABLE_UPDATE_DELETE_హద్దు) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
సమాసం *sqlite3_హద్దు_Where(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  సమాసం *pWhere,                /* The WHERE clause.  May be null */
  సమాసం_List *p_క్రమం_By,          /* The ORDER BY clause.  May be null */
  సమాసం *p_హద్దు,                /* The LIMIT clause.  May be null */
  అక్షరం *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  సమాసం *pLhs = ఖాళీ;           /* LHS of IN(SELECT...) operator */
  సమాసం *p_లోపల_Clause = ఖాళీ;      /* WHERE rowid IN ( select ) */
  సమాసం_List *pEList = ఖాళీ;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = ఖాళీ;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = ఖాళీ;      /* Complete SELECT tree */
  పట్టిక *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  ఒకవేళ( p_క్రమం_By && p_హద్దు==0 ) {
    sqlite3_Error_సందేశం(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_సమాసం_Delete(pParse->db, pWhere);
    sqlite3_సమాసం_List_Delete(pParse->db, p_క్రమం_By);
    తిరిగి 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  ఒకవేళ( p_హద్దు == 0 ) {
    తిరిగి pWhere;
  }

  /* Generate a select expression tree to enforce the limit/offset
  ** term for the DELETE or UPDATE statement.  For example:
  **   DELETE FROM table_a WHERE col1=1 ORDER BY col2 LIMIT 1 OFFSET 1
  ** becomes:
  **   DELETE FROM table_a WHERE rowid IN (
  **     SELECT rowid FROM table_a WHERE col1=1 ORDER BY col2 LIMIT 1 OFFSET 1
  **   );
  */

  pTab = pSrc->a[0].pSTab;
  ఒకవేళ( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_అడ్డువరుస, 0, 0);
    pEList = sqlite3_సమాసం_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_అడ్డువరుస, 0, 0)
    );
  }లేకపోతే{
    సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
    ప్రతిజ్ఞా( pPk!=0 );
    ప్రతిజ్ఞా( pPk->n_కీలకం_నిలువు>=1 );
    ఒకవేళ( pPk->n_కీలకం_నిలువు==1 ){
      స్థిరరాశి అక్షరం *zName;
      ప్రతిజ్ఞా( pPk->ai_స్తంభం[0]>=0 && pPk->ai_స్తంభం[0]<pTab->n_నిలువు );
      zName = pTab->a_నిలువు[pPk->ai_స్తంభం[0]].zCnName;
      pLhs = sqlite3_సమాసం(db, TK_ID, zName);
      pEList = sqlite3_సమాసం_List_Append(pParse, 0, sqlite3_సమాసం(db, TK_ID, zName));
    }లేకపోతే{
      పూర్ణాంకం i;
      కొరకు(i=0; i<pPk->n_కీలకం_నిలువు; i++){
        సమాసం *p;
        ప్రతిజ్ఞా( pPk->ai_స్తంభం[i]>=0 && pPk->ai_స్తంభం[i]<pTab->n_నిలువు );
        p = sqlite3_సమాసం(db, TK_ID, pTab->a_నిలువు[pPk->ai_స్తంభం[i]].zCnName);
        pEList = sqlite3_సమాసం_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_సదిశ, 0, 0);
      ఒకవేళ( pLhs ){
        pLhs->x.pList = sqlite3_సమాసం_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  ఒకవేళ( pSrc->a[0].fg.is_సూచీకృత_By ){
    ప్రతిజ్ఞా( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.is_సూచీకృత_By = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.z_సూచీకృత_By);
  }లేకపోతే ఒకవేళ( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      p_క్రమం_By,0,p_హద్దు
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  p_లోపల_Clause = sqlite3PExpr(pParse, TK_లోపల, pLhs, 0);
  sqlite3_PExpr_కూడు_Select(pParse, p_లోపల_Clause, pSelect);
  తిరిగి p_లోపల_Clause;
}
#అయితే_ముగింపు /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
శూన్య sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  సమాసం *pWhere,          /* The WHERE clause.  May be null */
  సమాసం_List *p_క్రమం_By,    /* ORDER BY clause. May be null */
  సమాసం *p_హద్దు           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  పట్టిక *pTab;           /* The table from which records will be deleted */
  పూర్ణాంకం i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  సూచీ *pIdx;           /* For looping over indices of the table */
  పూర్ణాంకం iTabCur;           /* Cursor number for the table */
  పూర్ణాంకం i_దత్తాంశం_Cur = 0;      /* VDBE cursor for the canonical data source */
  పూర్ణాంకం iIdxCur = 0;       /* Cursor number of the first index */
  పూర్ణాంకం nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  పూర్ణాంకం iDb;               /* Database number */
  పూర్ణాంకం memCnt = 0;        /* Memory cell used for change counting */
  పూర్ణాంకం rcauth;            /* Value returned by authorization callback */
  పూర్ణాంకం eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  పూర్ణాంకం aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *a_గమ్యం_Open = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  సూచీ *pPk;            /* The PRIMARY KEY index on the table */
  పూర్ణాంకం iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  పూర్ణాంకం i_కీలకం;              /* Memory cell holding key of row to be deleted */
  i16 n_కీలకం;              /* Number of memory cells in the row key */
  పూర్ణాంకం iEphCur = 0;       /* Ephemeral table holding all primary key values */
  పూర్ణాంకం i_అడ్డువరుస_సమితి = 0;       /* Register for rowset of rows to delete */
  పూర్ణాంకం addrBypass = 0;    /* Address of jump over the delete logic */
  పూర్ణాంకం addrLoop = 0;      /* Top of the delete loop */
  పూర్ణాంకం addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  పూర్ణాంకం bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  పూర్ణాంకం isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#అయితే_ముగింపు

  memset(&sContext, 0, పరిమాణం(sContext));
  db = pParse->db;
  ప్రతిజ్ఞా( db->pParse==pParse );
  ఒకవేళ( pParse->nErr ){
    వెళ్ళు delete_from_cleanup;
  }
  ప్రతిజ్ఞా( db->mallocFailed==0 );
  ప్రతిజ్ఞా( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ఒకవేళ( pTab==0 )  వెళ్ళు delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#లేకపోతే
# define pTrigger 0
# define isView 0
#అయితే_ముగింపు
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#నిర్వచితమైతే SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#అయితే_ముగింపు

#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x10000 ){
    sqlite3_వృక్షం_View_పంక్తి(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_వృక్షం_View_Delete(pParse->pWith, pTabList, pWhere,
                          p_క్రమం_By, p_హద్దు, pTrigger);
  }
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_UPDATE_DELETE_హద్దు
  ఒకవేళ( !isView ){
    pWhere = sqlite3_హద్దు_Where(
        pParse, pTabList, pWhere, p_క్రమం_By, p_హద్దు, "DELETE"
    );
    p_క్రమం_By = 0;
    p_హద్దు = 0;
  }
#అయితే_ముగింపు

  /* If pTab is really a view, make sure it has been initialized.
  */
  ఒకవేళ( sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab) ){
    వెళ్ళు delete_from_cleanup;
  }

  ఒకవేళ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    వెళ్ళు delete_from_cleanup;
  }
  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  ప్రతిజ్ఞా( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  ప్రతిజ్ఞా( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  ఒకవేళ( rcauth==SQLITE_DENY ){
    వెళ్ళు delete_from_cleanup;
  }
  ప్రతిజ్ఞా(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  ప్రతిజ్ఞా( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  కొరకు(nIdx=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  ఒకవేళ( isView ){
    sqlite3_Auth_Context_నెట్టు(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v==0 ){
    వెళ్ళు delete_from_cleanup;
  }
  ఒకవేళ( pParse->nested==0 ) sqlite3_Vdbe_లెక్క_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ఒకవేళ( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_క్రమం_By, p_హద్దు, iTabCur
    );
    i_దత్తాంశం_Cur = iIdxCur = iTabCur;
    p_క్రమం_By = 0;
    p_హద్దు = 0;
  }
#అయితే_ముగింపు

  /* Resolve the column names in the WHERE clause.
  */
  memset(&sNC, 0, పరిమాణం(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ఒకవేళ( sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pWhere) ){
    వెళ్ళు delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  ఒకవేళ( (db->flags & SQLITE_లెక్క_అడ్డువరుసలు)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, memCnt);
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRUNCATE_OPTIMIZATION
  /* Special case: A DELETE without a WHERE clause deletes everything.
  ** It is easier just to erase the whole table. Prior to version 3.6.5,
  ** this optimization caused the row change count (the value returned by
  ** API function sqlite3_count_changes) to be set incorrectly.
  **
  ** The "rcauth==SQLITE_OK" terms is the
  ** IMPLEMENTATION-OF: R-17228-37124 If the action code is SQLITE_DELETE and
  ** the callback returns SQLITE_IGNORE then the DELETE operation proceeds but
  ** the truncate optimization is disabled and all rows are deleted
  ** individually.
  */
  ఒకవేళ( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#అయితే_ముగింపు
  ){
    ప్రతిజ్ఞా( !isView );
    sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, 1, pTab->zName);
    ఒకవేళ( HasRowid(pTab) ){
      sqlite3_Vdbe_కూడు_Op4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
      ప్రతిజ్ఞా( pIdx->pSchema==pTab->pSchema );
      ఒకవేళ( Is_Primary_కీలకం_సూచీ(pIdx) && !HasRowid(pTab) ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    ఒకవేళ( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    ఒకవేళ( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      ప్రతిజ్ఞా( nPk==1 );
      i_అడ్డువరుస_సమితి = ++pParse->nMem;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, i_అడ్డువరుస_సమితి);
    }లేకపోతే{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      ప్రతిజ్ఞా( pPk!=0 );
      nPk = pPk->n_కీలకం_నిలువు;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pPk);
    }
 
    /* Construct a query to find the rowid or primary key for every row
    ** to be deleted, based on the WHERE clause. Set variable eOnePass
    ** to indicate the strategy used to implement this delete:
    **
    **  ONEPASS_OFF:    Two-pass approach - use a FIFO for rowids/PK values.
    **  ONEPASS_SINGLE: One-pass approach - at most one row deleted.
    **  ONEPASS_MULTI:  One-pass approach - any number of rows may be deleted.
    */
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, 0, 0,0,wcf,iTabCur+1);
    ఒకవేళ( pWInfo==0 ) వెళ్ళు delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    ప్రతిజ్ఞా( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    ప్రతిజ్ఞా( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    ఒకవేళ( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    ఒకవేళ( sqlite3_Where_ఉపయోగిస్తుంది_Deferred_Seek(pWInfo) ){
      sqlite3_Vdbe_కూడు_Op1(v, OP_ముగింపు_Seek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    ఒకవేళ( memCnt ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    ఒకవేళ( pPk ){
      కొరకు(i=0; i<nPk; i++){
        ప్రతిజ్ఞా( pPk->ai_స్తంభం[i]>=0 );
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iTabCur,
                                        pPk->ai_స్తంభం[i], iPk+i);
      }
      i_కీలకం = iPk;
    }లేకపోతే{
      i_కీలకం = ++pParse->nMem;
      sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iTabCur, -1, i_కీలకం);
    }
 
    ఒకవేళ( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_కీలకం = nPk; /* OP_Found will use an unpacked key */
      a_గమ్యం_Open = sqlite3DbMallocRawNN(db, nIdx+2);
      ఒకవేళ( a_గమ్యం_Open==0 ){
        sqlite3_Where_చివర(pWInfo);
        వెళ్ళు delete_from_cleanup;
      }
      memset(a_గమ్యం_Open, 1, nIdx+1);
      a_గమ్యం_Open[nIdx+1] = 0;
      ఒకవేళ( aiCurOnePass[0]>=0 ) a_గమ్యం_Open[aiCurOnePass[0]-iTabCur] = 0;
      ఒకవేళ( aiCurOnePass[1]>=0 ) a_గమ్యం_Open[aiCurOnePass[1]-iTabCur] = 0;
      ఒకవేళ( addrEphOpen ) sqlite3_Vdbe_Change_గమ్యం_Noop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }లేకపోతే{
      ఒకవేళ( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_కీలకం = ++pParse->nMem;
        n_కీలకం = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, iPk, nPk, i_కీలకం,
            sqlite3_సూచీ_Affinity_Str(pParse->db, pPk), nPk);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iEphCur, i_కీలకం, iPk, nPk);
      }లేకపోతే{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_కీలకం = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3_Vdbe_కూడు_Op2(v, OP_అడ్డువరుస_సమితి_కూడు, i_అడ్డువరుస_సమితి, i_కీలకం);
      }
      sqlite3_Where_చివర(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    ఒకవేళ( !isView ){
      పూర్ణాంకం iAddrOnce = 0;
      ఒకవేళ( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3_Open_పట్టిక_And_Indices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, a_గమ్యం_Open, &i_దత్తాంశం_Cur, &iIdxCur);
      ప్రతిజ్ఞా( pPk || IsVirtual(pTab) || i_దత్తాంశం_Cur==iTabCur );
      ప్రతిజ్ఞా( pPk || IsVirtual(pTab) || iIdxCur==i_దత్తాంశం_Cur+1 );
      ఒకవేళ( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_ఇక్కడ_Or_తీయి_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    ఒకవేళ( eOnePass!=ONEPASS_OFF ){
      ప్రతిజ్ఞా( n_కీలకం==nPk );  /* OP_Found will use an unpacked key */
      ఒకవేళ( !IsVirtual(pTab) && a_గమ్యం_Open[i_దత్తాంశం_Cur-iTabCur] ){
        ప్రతిజ్ఞా( pPk!=0 || IsView(pTab) );
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది, i_దత్తాంశం_Cur, addrBypass, i_కీలకం, n_కీలకం);
        VdbeCoverage(v);
      }
    }లేకపోతే ఒకవేళ( pPk ){
      addrLoop = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      ఒకవేళ( IsVirtual(pTab) ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iEphCur, 0, i_కీలకం);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_అడ్డువరుస_దత్తాంశం, iEphCur, i_కీలకం);
      }
      ప్రతిజ్ఞా( n_కీలకం==0 );  /* OP_Found will use a composite key */
    }లేకపోతే{
      addrLoop = sqlite3_Vdbe_కూడు_Op3(v, OP_అడ్డువరుస_సమితి_Read, i_అడ్డువరుస_సమితి, 0, i_కీలకం);
      VdbeCoverage(v);
      ప్రతిజ్ఞా( n_కీలకం==1 );
    } 
 
    /* Delete the row */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    ఒకవేళ( IsVirtual(pTab) ){
      స్థిరరాశి అక్షరం *pVTab = (స్థిరరాశి అక్షరం *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      ప్రతిజ్ఞా( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      ఒకవేళ( eOnePass==ONEPASS_SINGLE ){
        sqlite3_Vdbe_కూడు_Op1(v, OP_Close, iTabCur);
        ఒకవేళ( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3_Vdbe_కూడు_Op4(v, OP_VUpdate, 0, 1, i_కీలకం, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }లేకపోతే
#అయితే_ముగింపు
    {
      పూర్ణాంకం లెక్క = (pParse->nested==0);    /* True to count changes */
      sqlite3_Generate_అడ్డువరుస_Delete(pParse, pTab, pTrigger, i_దత్తాంశం_Cur, iIdxCur,
          i_కీలకం, n_కీలకం, లెక్క, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    ఒకవేళ( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3_Where_చివర(pWInfo);
    }లేకపోతే ఒకవేళ( pPk ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrLoop);
    }లేకపోతే{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ఒకవేళ( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_చివర(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ఒకవేళ( memCnt ){
    sqlite3_సంకేతం_Change_లెక్క(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_తీయి(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_సమాసం_Delete(db, pWhere);
#ఒకవేళ defined(SQLITE_ENABLE_UPDATE_DELETE_హద్దు)
  sqlite3_సమాసం_List_Delete(db, p_క్రమం_By);
  sqlite3_సమాసం_Delete(db, p_హద్దు);
#అయితే_ముగింపు
  ఒకవేళ( a_గమ్యం_Open ) sqlite3DbNNFreeNN(db, a_గమ్యం_Open);
  తిరిగి;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#నిర్వచితమైతే isView
 #నిర్వచనం_రద్దు isView
#అయితే_ముగింపు
#నిర్వచితమైతే pTrigger
 #నిర్వచనం_రద్దు pTrigger
#అయితే_ముగింపు

/*
** This routine generates VDBE code that causes a single row of a
** single table to be deleted.  Both the original table entry and
** all indices are removed.
**
** Preconditions:
**
**   1.  iDataCur is an open cursor on the btree that is the canonical data
**       store for the table.  (This will be either the table itself,
**       in the case of a rowid table, or the PRIMARY KEY index in the case
**       of a WITHOUT ROWID table.)
**
**   2.  Read/write cursors for all indices of pTab must be open as
**       cursor number iIdxCur+i for the i-th index.
**
**   3.  The primary key for the row to be deleted must be stored in a
**       sequence of nPk memory cells starting at iPk.  If nPk==0 that means
**       that a search record formed from OP_MakeRecord is contained in the
**       single memory location iPk.
**
** eMode:
**   Parameter eMode may be passed either ONEPASS_OFF (0), ONEPASS_SINGLE, or
**   ONEPASS_MULTI.  If eMode is not ONEPASS_OFF, then the cursor
**   iDataCur already points to the row to delete. If eMode is ONEPASS_OFF
**   then this function must seek iDataCur to the entry identified by iPk
**   and nPk before reading from it.
**
**   If eMode is ONEPASS_MULTI, then this call is being made as part
**   of a ONEPASS delete that affects multiple rows. In this case, if
**   iIdxNoSeek is a valid cursor number (>=0) and is not the same as
**   iDataCur, then its position should be preserved following the delete
**   operation. Or, if iIdxNoSeek is not a valid cursor number, the
**   position of iDataCur should be preserved instead.
**
** iIdxNoSeek:
**   If iIdxNoSeek is a valid cursor number (>=0) not equal to iDataCur,
**   then it identifies an index cursor (from within array of cursors
**   starting at iIdxCur) that already points to the index entry to be deleted.
**   Except, this optimization is disabled if there are BEFORE triggers since
**   the trigger body might have moved the cursor.
*/
శూన్య sqlite3_Generate_అడ్డువరుస_Delete(
  Parse *pParse,     /* Parsing context */
  పట్టిక *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  పూర్ణాంకం i_దత్తాంశం_Cur,      /* Cursor from which column data is extracted */
  పూర్ణాంకం iIdxCur,       /* First index cursor */
  పూర్ణాంకం iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 లెక్క,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 e_బహుళకం,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  పూర్ణాంకం iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  పూర్ణాంకం iOld = 0;                   /* First register in OLD.* array */
  పూర్ణాంకం iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  ప్రతిజ్ఞా( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         i_దత్తాంశం_Cur, iIdxCur, iPk, (పూర్ణాంకం)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_Not_దొరికింది;
  ఒకవేళ( e_బహుళకం==ONEPASS_OFF ){
    sqlite3_Vdbe_కూడు_Op4_Int(v, opSeek, i_దత్తాంశం_Cur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_Not_దొరికింది);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  ఒకవేళ( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 ముసుగు;                     /* Mask of OLD.* columns in use */
    పూర్ణాంకం i_నిలువు;                     /* Iterator used while populating OLD.* */
    పూర్ణాంకం addr_ప్రారంభం;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    ముసుగు = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    ముసుగు |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->n_నిలువు);

    /* Populate the OLD.* pseudo-table register array. These values will be
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, iPk, iOld);
    కొరకు(i_నిలువు=0; i_నిలువు<pTab->n_నిలువు; i_నిలువు++){
      testcase( ముసుగు!=0xffffffff && i_నిలువు==31 );
      testcase( ముసుగు!=0xffffffff && i_నిలువు==32 );
      ఒకవేళ( ముసుగు==0xffffffff || (i_నిలువు<=31 && (ముసుగు & MASKBIT32(i_నిలువు))!=0) ){
        పూర్ణాంకం kk = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_నిలువు);
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur, i_నిలువు, iOld+kk+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addr_ప్రారంభం = sqlite3VdbeCurrentAddr(v);
    sqlite3_సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger,
        TK_DELETE, 0, TRIGGER_BEFORE, pTab, iOld, onconf, iLabel
    );

    /* If any BEFORE triggers were coded, then seek the cursor to the
    ** row to be deleted again. It may be that the BEFORE triggers moved
    ** the cursor or already deleted the row that the cursor was
    ** pointing to.
    **
    ** Also disable the iIdxNoSeek optimization since the BEFORE trigger
    ** may have moved that cursor.
    */
    ఒకవేళ( addr_ప్రారంభం<sqlite3VdbeCurrentAddr(v) ){
      sqlite3_Vdbe_కూడు_Op4_Int(v, opSeek, i_దత్తాంశం_Cur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_Not_దొరికింది);
      testcase( iIdxNoSeek>=0 );
      iIdxNoSeek = -1;
    }

    /* Do FK processing. This call checks that any FK constraints that
    ** refer to this table (i.e. constraints attached to other tables)
    ** are not violated by deleting this row.  */
    sqlite3FkCheck(pParse, pTab, iOld, 0, 0, 0);
  }

  /* Delete the index and table entries. Skip this step if pTab is really
  ** a view (in which case the only effect of the DELETE statement is to
  ** fire the INSTEAD OF triggers). 
  **
  ** If variable 'count' is non-zero, then this OP_Delete instruction should
  ** invoke the update-hook. The pre-update-hook, on the other hand should
  ** be invoked unless table pTab is a system table. The difference is that
  ** the update-hook is not invoked for rows removed by REPLACE, but the
  ** pre-update-hook is.
  */
  ఒకవేళ( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3_Generate_అడ్డువరుస_సూచీ_Delete(pParse, pTab, i_దత్తాంశం_Cur, iIdxCur,0,iIdxNoSeek);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Delete, i_దత్తాంశం_Cur, (లెక్క?OPFLAG_NCHANGE:0));
    ఒకవేళ( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (అక్షరం*)pTab, P4_పట్టిక);
    }
    ఒకవేళ( e_బహుళకం!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    ఒకవేళ( iIdxNoSeek>=0 && iIdxNoSeek!=i_దత్తాంశం_Cur ){
      sqlite3_Vdbe_కూడు_Op1(v, OP_Delete, iIdxNoSeek);
    }
    ఒకవేళ( e_బహుళకం==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  ఒకవేళ( pTrigger ){
    sqlite3_సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger,
        TK_DELETE, 0, TRIGGER_AFTER, pTab, iOld, onconf, iLabel
    );
  }

  /* Jump here if the row had already been deleted before any BEFORE
  ** trigger programs were invoked. Or if a trigger program throws a
  ** RAISE(IGNORE) exception.  */
  sqlite3VdbeResolveLabel(v, iLabel);
  VdbeModuleComment((v, "END: GenRowDel()"));
}

/*
** This routine generates VDBE code that causes the deletion of all
** index entries associated with a single row of a single table, pTab
**
** Preconditions:
**
**   1.  A read/write cursor "iDataCur" must be open on the canonical storage
**       btree for the table pTab.  (This will be either the table itself
**       for rowid tables or to the primary key index for WITHOUT ROWID
**       tables.)
**
**   2.  Read/write cursors for all indices of pTab must be open as
**       cursor number iIdxCur+i for the i-th index.  (The pTab->pIndex
**       index is the 0-th index.)
**
**   3.  The "iDataCur" cursor must be already be positioned on the row
**       that is to be deleted.
*/
శూన్య sqlite3_Generate_అడ్డువరుస_సూచీ_Delete(
  Parse *pParse,     /* Parsing and code generating context */
  పట్టిక *pTab,       /* Table containing the row to be deleted */
  పూర్ణాంకం i_దత్తాంశం_Cur,      /* Cursor of table holding data. */
  పూర్ణాంకం iIdxCur,       /* First index cursor */
  పూర్ణాంకం *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  పూర్ణాంకం iIdxNoSeek     /* Do not delete from this cursor */
){
  పూర్ణాంకం i;             /* Index loop counter */
  పూర్ణాంకం r1 = -1;       /* Register holding an index key */
  పూర్ణాంకం iPartIdxLabel; /* Jump destination for skipping partial index entries */
  సూచీ *pIdx;       /* Current index */
  సూచీ *p_పూర్వ = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  సూచీ *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_కీలకం_సూచీ(pTab);
  కొరకు(i=0, pIdx=pTab->p_సూచీ; pIdx; i++, pIdx=pIdx->p_తదుపరి){
    పూర్ణాంకం p3 = 0;
    ప్రతిజ్ఞా( iIdxCur+i!=i_దత్తాంశం_Cur || pPk==pIdx );
    ఒకవేళ( aRegIdx!=0 && aRegIdx[i]==0 ) కొనసాగు;
    ఒకవేళ( pIdx==pPk ) కొనసాగు;
    ఒకవేళ( iIdxCur+i==iIdxNoSeek ) కొనసాగు;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_సూచీ_కీలకం(pParse, pIdx, i_దత్తాంశం_Cur, 0, 1,
        &iPartIdxLabel, p_పూర్వ, r1);
    ఒకవేళ( pIdx->b_Has_సమాసం && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (స్థిరరాశి అక్షరం*)pIdx, P4_సూచీ);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_కీలకం_నిలువు : pIdx->n_స్తంభం);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    p_పూర్వ = pIdx;
  }
}

/*
** Generate code that will assemble an index key and stores it in register
** regOut.  The key with be for index pIdx which is an index on pTab.
** iCur is the index of a cursor open on the pTab table and pointing to
** the entry that needs indexing.  If pTab is a WITHOUT ROWID table, then
** iCur must be the cursor of the PRIMARY KEY index.
**
** Return a register number which is the first in a block of
** registers that holds the elements of the index key.  The
** block of registers has already been deallocated by the time
** this routine returns.
**
** If *piPartIdxLabel is not NULL, fill it in with a label and jump
** to that label if pIdx is a partial index that should be skipped.
** The label should be resolved using sqlite3ResolvePartIdxLabel().
** A partial index should be skipped if its WHERE clause evaluates
** to false or null.  If pIdx is not a partial index, *piPartIdxLabel
** will be set to zero which is an empty label that is ignored by
** sqlite3ResolvePartIdxLabel().
**
** The pPrior and regPrior parameters are used to implement a cache to
** avoid unnecessary register loads.  If pPrior is not NULL, then it is
** a pointer to a different index for which an index key has just been
** computed into register regPrior.  If the current pIdx index is generating
** its key into the same sequence of registers and if pPrior and pIdx share
** a column in common, then the register corresponding to that column already
** holds the correct value and the loading of that register is skipped.
** This optimization is helpful when doing a DELETE or an INTEGRITY_CHECK
** on a table with multiple indices, and especially with the ROWID or
** PRIMARY KEY columns of the index.
*/
పూర్ణాంకం sqlite3_Generate_సూచీ_కీలకం(
  Parse *pParse,       /* Parsing context */
  సూచీ *pIdx,         /* The index for which to generate a key */
  పూర్ణాంకం i_దత్తాంశం_Cur,        /* Cursor number from which to take column data */
  పూర్ణాంకం reg_బహిర్గతం,          /* Put the new key into this register if not 0 */
  పూర్ణాంకం ఉపసర్గ_Only,      /* Compute only a unique prefix of the key */
  పూర్ణాంకం *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  సూచీ *p_పూర్వ,       /* Previously generated index key */
  పూర్ణాంకం reg_పూర్వ         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం j;
  పూర్ణాంకం reg_పునాది;
  పూర్ణాంకం n_నిలువు;

  ఒకవేళ( piPartIdxLabel ){
    ఒకవేళ( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = i_దత్తాంశం_Cur + 1;
      sqlite3_సమాసం_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      p_పూర్వ = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }లేకపోతే{
      *piPartIdxLabel = 0;
    }
  }
  n_నిలువు = (ఉపసర్గ_Only && pIdx->uniqNotNull) ? pIdx->n_కీలకం_నిలువు : pIdx->n_స్తంభం;
  reg_పునాది = sqlite3_Get_తాత్కాలిక_Range(pParse, n_నిలువు);
  ఒకవేళ( p_పూర్వ && (reg_పునాది!=reg_పూర్వ || p_పూర్వ->pPartIdxWhere) ) p_పూర్వ = 0;
  కొరకు(j=0; j<n_నిలువు; j++){
    పూర్ణాంకం i_Tab_నిలువు;
    ఒకవేళ( p_పూర్వ
     && p_పూర్వ->ai_స్తంభం[j]==pIdx->ai_స్తంభం[j]
     && p_పూర్వ->ai_స్తంభం[j]!=XN_సమాసం
    ){
      /* This column was already computed by the previous index */
      కొనసాగు;
    }
    sqlite3_సమాసం_సంకేతం_ఎక్కించు_సూచీ_స్తంభం(pParse, pIdx, i_దత్తాంశం_Cur, j, reg_పునాది+j);
    ఒకవేళ( (i_Tab_నిలువు = pIdx->ai_స్తంభం[j])>=0 
     && (pIdx->p_పట్టిక->a_నిలువు[i_Tab_నిలువు].నిలువు_Flags & COLFLAG_VIRTUAL)==0
    ){
      /* If the column affinity is REAL but the number is an integer, then it
      ** might be stored in the table as an integer (using a compact
      ** representation) then converted to REAL by an OP_RealAffinity opcode.
      ** But we are getting ready to store this value back into an index, where
      ** it should be converted by to INTEGER again.  So omit the
      ** OP_RealAffinity opcode if it is present */
      sqlite3_Vdbe_Delete_పూర్వ_Opcode(v, OP_RealAffinity);
    }
  }
  ఒకవేళ( reg_బహిర్గతం ){
    sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_పునాది, n_నిలువు, reg_బహిర్గతం);
  }
  sqlite3_Release_తాత్కాలిక_Range(pParse, reg_పునాది, n_నిలువు);
  తిరిగి reg_పునాది;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
శూన్య sqlite3ResolvePartIdxLabel(Parse *pParse, పూర్ణాంకం iLabel){
  ఒకవేళ( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
