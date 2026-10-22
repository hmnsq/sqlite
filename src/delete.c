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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

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
Table *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  SrcItem *pItem = pSrc->a;
  Table *pTab;
  ପ୍ରତିଜ୍ଞା( pItem && pSrc->nSrc>=1 );
  pTab = sqlite3LocateTableItem(pParse, 0, pItem);
  ଯଦି( pItem->pSTab ) sqlite3DeleteTable(pParse->db, pItem->pSTab);
  pItem->pSTab = pTab;
  pItem->fg.notCte = 1;
  ଯଦି( pTab ){
    pTab->nTabRef++;
    ଯଦି( pItem->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pItem) ){
      pTab = 0;
    }
  }
  ଫେରନ୍ତୁ pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
ଶୂନ୍ୟ sqlite3_Code_Change_ଗଣନା(Vdbe *v, ପୂର୍ଣ୍ଣସଂଖ୍ୟା regCounter, ଅଚଳ ଅକ୍ଷର *zColName){
  sqlite3VdbeAddOp0(v, OP_FkCheck);
  sqlite3VdbeAddOp2(v, OP_ResultRow, regCounter, 1);
  sqlite3_Vdbe_Set_ଅଙ୍କ_Cols(v, 1);
  sqlite3VdbeSetColName(v, 0, COLNAME_NAME, zColName, SQLITE_STATIC);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vtabIsReadOnly(Parse *pParse, Table *pTab){
  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );
  ଯଦି( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    ଫେରନ୍ତୁ 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  ଯଦି( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  ଫେରନ୍ତୁ 0;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tabIsReadOnly(Parse *pParse, Table *pTab){
  sqlite3 *db;
  ଯଦି( IsVirtual(pTab) ){
    ଫେରନ୍ତୁ vtabIsReadOnly(pParse, pTab);
  }
  ଯଦି( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) ଫେରନ୍ତୁ 0;
  db = pParse->db;
  ଯଦି( (pTab->tabFlags & TF_Readonly)!=0 ){
    ଫେରନ୍ତୁ sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  ପ୍ରତିଜ୍ଞା( pTab->tabFlags & TF_Shadow );
  ଫେରନ୍ତୁ sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3IsReadOnly(Parse *pParse, Table *pTab, Trigger *pTrigger){
  ଯଦି( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "table %s may not be modified", pTab->zName);
    ଫେରନ୍ତୁ 1;
  }
#ifndef SQLITE_OMIT_VIEW
  ଯଦି( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->pNext==0))
  ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse,"cannot modify %s because it is a view",pTab->zName);
    ଫେରନ୍ତୁ 1;
  }
#endif
  ଫେରନ୍ତୁ 0;
}


#ଯଦି !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
ଶୂନ୍ୟ sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  Table *pView,        /* View definition */
  ପଦସମଷ୍ଟି *pWhere,        /* Optional WHERE clause to be added */
  ପଦସମଷ୍ଟି_List *pOrderBy,  /* Optional ORDER BY clause */
  ପଦସମଷ୍ଟି *pLimit,        /* Optional LIMIT clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(db, pView->pSchema);
  pWhere = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  ଯଦି( pFrom ){
    ପ୍ରତିଜ୍ଞା( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    ପ୍ରତିଜ୍ଞା( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    ପ୍ରତିଜ୍ଞା( pFrom->a[0].fg.isUsing==0 );
    ପ୍ରତିଜ୍ଞା( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, pOrderBy,
                          SF_IncludeHidden, pLimit);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#endif /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#ଯଦି defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
ପଦସମଷ୍ଟି *sqlite3LimitWhere(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  ପଦସମଷ୍ଟି *pWhere,                /* The WHERE clause.  May be null */
  ପଦସମଷ୍ଟି_List *pOrderBy,          /* The ORDER BY clause.  May be null */
  ପଦସମଷ୍ଟି *pLimit,                /* The LIMIT clause.  May be null */
  ଅକ୍ଷର *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  ପଦସମଷ୍ଟି *pLhs = ଖାଲି;           /* LHS of IN(SELECT...) operator */
  ପଦସମଷ୍ଟି *pInClause = ଖାଲି;      /* WHERE rowid IN ( select ) */
  ପଦସମଷ୍ଟି_List *pEList = ଖାଲି;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = ଖାଲି;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = ଖାଲି;      /* Complete SELECT tree */
  Table *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  ଯଦି( pOrderBy && pLimit==0 ) {
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, pWhere);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(pParse->db, pOrderBy);
    ଫେରନ୍ତୁ 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  ଯଦି( pLimit == 0 ) {
    ଫେରନ୍ତୁ pWhere;
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
  ଯଦି( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_ROW, 0, 0);
    pEList = sqlite3_ପଦସମଷ୍ଟି_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_ROW, 0, 0)
    );
  }ଅନ୍ୟଥା{
    Index *pPk = sqlite3_Primary_ଚାବି_Index(pTab);
    ପ୍ରତିଜ୍ଞା( pPk!=0 );
    ପ୍ରତିଜ୍ଞା( pPk->n_ଚାବି_Col>=1 );
    ଯଦି( pPk->n_ଚାବି_Col==1 ){
      ଅଚଳ ଅକ୍ଷର *zName;
      ପ୍ରତିଜ୍ଞା( pPk->aiColumn[0]>=0 && pPk->aiColumn[0]<pTab->nCol );
      zName = pTab->aCol[pPk->aiColumn[0]].zCnName;
      pLhs = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, zName);
      pEList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, 0, sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, zName));
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପାଇଁ(i=0; i<pPk->n_ଚାବି_Col; i++){
        ପଦସମଷ୍ଟି *p;
        ପ୍ରତିଜ୍ଞା( pPk->aiColumn[i]>=0 && pPk->aiColumn[i]<pTab->nCol );
        p = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, pTab->aCol[pPk->aiColumn[i]].zCnName);
        pEList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_VECTOR, 0, 0);
      ଯଦି( pLhs ){
        pLhs->x.pList = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  ଯଦି( pSrc->a[0].fg.isIndexedBy ){
    ପ୍ରତିଜ୍ଞା( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.isIndexedBy = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.zIndexedBy);
  }ଅନ୍ୟଥା ଯଦି( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      pOrderBy,0,pLimit
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  pInClause = sqlite3PExpr(pParse, TK_IN, pLhs, 0);
  sqlite3PExprAddSelect(pParse, pInClause, pSelect);
  ଫେରନ୍ତୁ pInClause;
}
#endif /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
ଶୂନ୍ୟ sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  ପଦସମଷ୍ଟି *pWhere,          /* The WHERE clause.  May be null */
  ପଦସମଷ୍ଟି_List *pOrderBy,    /* ORDER BY clause. May be null */
  ପଦସମଷ୍ଟି *pLimit           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  Table *pTab;           /* The table from which records will be deleted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  Index *pIdx;           /* For looping over indices of the table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTabCur;           /* Cursor number for the table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur = 0;      /* VDBE cursor for the canonical data source */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdxCur = 0;       /* Cursor number of the first index */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;               /* Database number */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା memCnt = 0;        /* Memory cell used for change counting */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcauth;            /* Value returned by authorization callback */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *aToOpen = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  Index *pPk;            /* The PRIMARY KEY index on the table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ଚାବି;              /* Memory cell holding key of row to be deleted */
  i16 n_ଚାବି;              /* Number of memory cells in the row key */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEphCur = 0;       /* Ephemeral table holding all primary key values */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRowSet = 0;       /* Register for rowset of rows to delete */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrBypass = 0;    /* Address of jump over the delete logic */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrLoop = 0;      /* Top of the delete loop */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#ifndef SQLITE_OMIT_TRIGGER
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#endif

  memset(&sContext, 0, ଆକାର(sContext));
  db = pParse->db;
  ପ୍ରତିଜ୍ଞା( db->pParse==pParse );
  ଯଦି( pParse->nErr ){
    ଯାଅ delete_from_cleanup;
  }
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
  ପ୍ରତିଜ୍ଞା( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ଯଦି( pTab==0 )  ଯାଅ delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#ଅନ୍ୟଥା
# define pTrigger 0
# define isView 0
#endif
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif

#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x10000 ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_ବୃକ୍ଷ_View_Delete(pParse->pWith, pTabList, pWhere,
                          pOrderBy, pLimit, pTrigger);
  }
#endif

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  ଯଦି( !isView ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "DELETE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  /* If pTab is really a view, make sure it has been initialized.
  */
  ଯଦି( sqlite3ViewGetColumnNames(pParse, pTab) ){
    ଯାଅ delete_from_cleanup;
  }

  ଯଦି( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ଯାଅ delete_from_cleanup;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  ପ୍ରତିଜ୍ଞା( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  ପ୍ରତିଜ୍ଞା( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  ଯଦି( rcauth==SQLITE_DENY ){
    ଯାଅ delete_from_cleanup;
  }
  ପ୍ରତିଜ୍ଞା(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  ପ୍ରତିଜ୍ଞା( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  ପାଇଁ(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  ଯଦି( isView ){
    sqlite3_Auth_Context_ଠେଲ(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  ଯଦି( v==0 ){
    ଯାଅ delete_from_cleanup;
  }
  ଯଦି( pParse->nested==0 ) sqlite3_Vdbe_ଗଣନା_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#ଯଦି !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ଯଦି( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, pOrderBy, pLimit, iTabCur
    );
    iDataCur = iIdxCur = iTabCur;
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  /* Resolve the column names in the WHERE clause.
  */
  memset(&sNC, 0, ଆକାର(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pWhere) ){
    ଯାଅ delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  ଯଦି( (db->flags & SQLITE_ଗଣନା_Rows)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, memCnt);
  }

#ifndef SQLITE_OMIT_TRUNCATE_OPTIMIZATION
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
  ଯଦି( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#endif
  ){
    ପ୍ରତିଜ୍ଞା( !isView );
    sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);
    ଯଦି( HasRowid(pTab) ){
      sqlite3VdbeAddOp4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    ପାଇଁ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ପ୍ରତିଜ୍ଞା( pIdx->pSchema==pTab->pSchema );
      ଯଦି( Is_Primary_ଚାବି_Index(pIdx) && !HasRowid(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }ଅନ୍ୟଥା
#endif /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    ଯଦି( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    ଯଦି( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      ପ୍ରତିଜ୍ଞା( nPk==1 );
      iRowSet = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, iRowSet);
    }ଅନ୍ୟଥା{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_ଚାବି_Index(pTab);
      ପ୍ରତିଜ୍ଞା( pPk!=0 );
      nPk = pPk->n_ଚାବି_Col;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_Set_P4_ଚାବି_Info(pParse, pPk);
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
    ଯଦି( pWInfo==0 ) ଯାଅ delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    ପ୍ରତିଜ୍ଞା( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    ଯଦି( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    ଯଦି( sqlite3WhereUsesDeferredSeek(pWInfo) ){
      sqlite3VdbeAddOp1(v, OP_FinishSeek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    ଯଦି( memCnt ){
      sqlite3VdbeAddOp2(v, OP_AddImm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    ଯଦି( pPk ){
      ପାଇଁ(i=0; i<nPk; i++){
        ପ୍ରତିଜ୍ଞା( pPk->aiColumn[i]>=0 );
        sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(v, pTab, iTabCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      i_ଚାବି = iPk;
    }ଅନ୍ୟଥା{
      i_ଚାବି = ++pParse->nMem;
      sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(v, pTab, iTabCur, -1, i_ଚାବି);
    }
 
    ଯଦି( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_ଚାବି = nPk; /* OP_Found will use an unpacked key */
      aToOpen = sqlite3DbMallocRawNN(db, nIdx+2);
      ଯଦି( aToOpen==0 ){
        sqlite3WhereEnd(pWInfo);
        ଯାଅ delete_from_cleanup;
      }
      memset(aToOpen, 1, nIdx+1);
      aToOpen[nIdx+1] = 0;
      ଯଦି( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iTabCur] = 0;
      ଯଦି( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iTabCur] = 0;
      ଯଦି( addrEphOpen ) sqlite3VdbeChangeToNoop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }ଅନ୍ୟଥା{
      ଯଦି( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_ଚାବି = ++pParse->nMem;
        n_ଚାବି = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, i_ଚାବି,
            sqlite3IndexAffinityStr(pParse->db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iEphCur, i_ଚାବି, iPk, nPk);
      }ଅନ୍ୟଥା{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_ଚାବି = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3VdbeAddOp2(v, OP_RowSetAdd, iRowSet, i_ଚାବି);
      }
      sqlite3WhereEnd(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    ଯଦି( !isView ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAddrOnce = 0;
      ଯଦି( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, aToOpen, &iDataCur, &iIdxCur);
      ପ୍ରତିଜ୍ଞା( pPk || IsVirtual(pTab) || iDataCur==iTabCur );
      ପ୍ରତିଜ୍ଞା( pPk || IsVirtual(pTab) || iIdxCur==iDataCur+1 );
      ଯଦି( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_Here_Or_କାଢ଼_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    ଯଦି( eOnePass!=ONEPASS_OFF ){
      ପ୍ରତିଜ୍ଞା( n_ଚାବି==nPk );  /* OP_Found will use an unpacked key */
      ଯଦି( !IsVirtual(pTab) && aToOpen[iDataCur-iTabCur] ){
        ପ୍ରତିଜ୍ଞା( pPk!=0 || IsView(pTab) );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, addrBypass, i_ଚାବି, n_ଚାବି);
        VdbeCoverage(v);
      }
    }ଅନ୍ୟଥା ଯଦି( pPk ){
      addrLoop = sqlite3VdbeAddOp1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      ଯଦି( IsVirtual(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Column, iEphCur, 0, i_ଚାବି);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, OP_RowData, iEphCur, i_ଚାବି);
      }
      ପ୍ରତିଜ୍ଞା( n_ଚାବି==0 );  /* OP_Found will use a composite key */
    }ଅନ୍ୟଥା{
      addrLoop = sqlite3VdbeAddOp3(v, OP_RowSetRead, iRowSet, 0, i_ଚାବି);
      VdbeCoverage(v);
      ପ୍ରତିଜ୍ଞା( n_ଚାବି==1 );
    } 
 
    /* Delete the row */
#ifndef SQLITE_OMIT_VIRTUALTABLE
    ଯଦି( IsVirtual(pTab) ){
      ଅଚଳ ଅକ୍ଷର *pVTab = (ଅଚଳ ଅକ୍ଷର *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      ପ୍ରତିଜ୍ଞା( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      ଯଦି( eOnePass==ONEPASS_SINGLE ){
        sqlite3VdbeAddOp1(v, OP_Close, iTabCur);
        ଯଦି( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3VdbeAddOp4(v, OP_VUpdate, 0, 1, i_ଚାବି, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }ଅନ୍ୟଥା
#endif
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = (pParse->nested==0);    /* True to count changes */
      sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur,
          i_ଚାବି, n_ଚାବି, ଗଣନା_2, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    ଯଦି( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3WhereEnd(pWInfo);
    }ଅନ୍ୟଥା ଯଦି( pPk ){
      sqlite3VdbeAddOp2(v, OP_Next, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addrLoop);
    }ଅନ୍ୟଥା{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3VdbeJumpHere(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ଯଦି( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ଯଦି( memCnt ){
    sqlite3_Code_Change_ଗଣନା(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_କାଢ଼(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pWhere);
#ଯଦି defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pOrderBy);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pLimit);
#endif
  ଯଦି( aToOpen ) sqlite3DbNNFreeNN(db, aToOpen);
  ଫେରନ୍ତୁ;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ifdef isView
 #undef isView
#endif
#ifdef pTrigger
 #undef pTrigger
#endif

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
ଶୂନ୍ୟ sqlite3GenerateRowDelete(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur,      /* Cursor from which column data is extracted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdxCur,       /* First index cursor */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 ଗଣନା_2,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 eMode,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOld = 0;                   /* First register in OLD.* array */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  ପ୍ରତିଜ୍ଞା( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         iDataCur, iIdxCur, iPk, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_NotFound;
  ଯଦି( eMode==ONEPASS_OFF ){
    sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_NotFound);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  ଯଦି( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 mask;                     /* Mask of OLD.* columns in use */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;                     /* Iterator used while populating OLD.* */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrStart;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    mask = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    mask |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->nCol);

    /* Populate the OLD.* pseudo-table register array. These values will be
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3VdbeAddOp2(v, OP_Copy, iPk, iOld);
    ପାଇଁ(iCol=0; iCol<pTab->nCol; iCol++){
      testcase( mask!=0xffffffff && iCol==31 );
      testcase( mask!=0xffffffff && iCol==32 );
      ଯଦି( mask==0xffffffff || (iCol<=31 && (mask & MASKBIT32(iCol))!=0) ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା kk = sqlite3TableColumnToStorage(pTab, iCol);
        sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(v, pTab, iDataCur, iCol, iOld+kk+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addrStart = sqlite3VdbeCurrentAddr(v);
    sqlite3CodeRowTrigger(pParse, pTrigger,
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
    ଯଦି( addrStart<sqlite3VdbeCurrentAddr(v) ){
      sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_NotFound);
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
  ଯଦି( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur,0,iIdxNoSeek);
    sqlite3VdbeAddOp2(v, OP_Delete, iDataCur, (ଗଣନା_2?OPFLAG_NCHANGE:0));
    ଯଦି( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (ଅକ୍ଷର*)pTab, P4_TABLE);
    }
    ଯଦି( eMode!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    ଯଦି( iIdxNoSeek>=0 && iIdxNoSeek!=iDataCur ){
      sqlite3VdbeAddOp1(v, OP_Delete, iIdxNoSeek);
    }
    ଯଦି( eMode==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  ଯଦି( pTrigger ){
    sqlite3CodeRowTrigger(pParse, pTrigger,
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
ଶୂନ୍ୟ sqlite3GenerateRowIndexDelete(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* Table containing the row to be deleted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur,      /* Cursor of table holding data. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdxCur,       /* First index cursor */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdxNoSeek     /* Do not delete from this cursor */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;             /* Index loop counter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = -1;       /* Register holding an index key */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPartIdxLabel; /* Jump destination for skipping partial index entries */
  Index *pIdx;       /* Current index */
  Index *pPrior = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  Index *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_ଚାବି_Index(pTab);
  ପାଇଁ(i=0, pIdx=pTab->pIndex; pIdx; i++, pIdx=pIdx->pNext){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା p3 = 0;
    ପ୍ରତିଜ୍ଞା( iIdxCur+i!=iDataCur || pPk==pIdx );
    ଯଦି( aRegIdx!=0 && aRegIdx[i]==0 ) ଜାରି;
    ଯଦି( pIdx==pPk ) ଜାରି;
    ଯଦି( iIdxCur+i==iIdxNoSeek ) ଜାରି;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_Index_ଚାବି(pParse, pIdx, iDataCur, 0, 1,
        &iPartIdxLabel, pPrior, r1);
    ଯଦି( pIdx->b_Has_ପଦସମଷ୍ଟି && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3VdbeAddOp3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (ଅଚଳ ଅକ୍ଷର*)pIdx, P4_INDEX);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_ଚାବି_Col : pIdx->nColumn);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    pPrior = pIdx;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Generate_Index_ଚାବି(
  Parse *pParse,       /* Parsing context */
  Index *pIdx,         /* The index for which to generate a key */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur,        /* Cursor number from which to take column data */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOut,          /* Put the new key into this register if not 0 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା prefixOnly,      /* Compute only a unique prefix of the key */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  Index *pPrior,       /* Previously generated index key */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regPrior         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regBase;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;

  ଯଦି( piPartIdxLabel ){
    ଯଦି( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = iDataCur + 1;
      sqlite3_ପଦସମଷ୍ଟି_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      pPrior = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }ଅନ୍ୟଥା{
      *piPartIdxLabel = 0;
    }
  }
  nCol = (prefixOnly && pIdx->uniqNotNull) ? pIdx->n_ଚାବି_Col : pIdx->nColumn;
  regBase = sqlite3GetTempRange(pParse, nCol);
  ଯଦି( pPrior && (regBase!=regPrior || pPrior->pPartIdxWhere) ) pPrior = 0;
  ପାଇଁ(j=0; j<nCol; j++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTabCol;
    ଯଦି( pPrior
     && pPrior->aiColumn[j]==pIdx->aiColumn[j]
     && pPrior->aiColumn[j]!=XN_ପଦସମଷ୍ଟି
    ){
      /* This column was already computed by the previous index */
      ଜାରି;
    }
    sqlite3_ପଦସମଷ୍ଟି_Code_Load_Index_Column(pParse, pIdx, iDataCur, j, regBase+j);
    ଯଦି( (iTabCol = pIdx->aiColumn[j])>=0 
     && (pIdx->pTable->aCol[iTabCol].colFlags & COLFLAG_VIRTUAL)==0
    ){
      /* If the column affinity is REAL but the number is an integer, then it
      ** might be stored in the table as an integer (using a compact
      ** representation) then converted to REAL by an OP_RealAffinity opcode.
      ** But we are getting ready to store this value back into an index, where
      ** it should be converted by to INTEGER again.  So omit the
      ** OP_RealAffinity opcode if it is present */
      sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
    }
  }
  ଯଦି( regOut ){
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regOut);
  }
  sqlite3ReleaseTempRange(pParse, regBase, nCol);
  ଫେରନ୍ତୁ regBase;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
ଶୂନ୍ୟ sqlite3ResolvePartIdxLabel(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLabel){
  ଯଦି( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
