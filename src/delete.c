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
#شامل "sqliteInt.h"

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
  أكد( pItem && pSrc->nSrc>=1 );
  pTab = sqlite3LocateTableItem(pParse, 0, pItem);
  اگر( pItem->pSTab ) sqlite3DeleteTable(pParse->db, pItem->pSTab);
  pItem->pSTab = pTab;
  pItem->fg.notCte = 1;
  اگر( pTab ){
    pTab->nTabRef++;
    اگر( pItem->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pItem) ){
      pTab = 0;
    }
  }
  بازگشت pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
تهی sqlite3_Code_Change_شمار(Vdbe *v, صحیح regCounter, ثابت نویسه *zColName){
  sqlite3VdbeAddOp0(v, OP_FkCheck);
  sqlite3VdbeAddOp2(v, OP_ResultRow, regCounter, 1);
  sqlite3_Vdbe_Set_رقم_Cols(v, 1);
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
ایستا صحیح vtabIsReadOnly(Parse *pParse, Table *pTab){
  أكد( IsVirtual(pTab) );
  اگر( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    بازگشت 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  اگر( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_پیام(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  بازگشت 0;
}
ایستا صحیح tabIsReadOnly(Parse *pParse, Table *pTab){
  sqlite3 *db;
  اگر( IsVirtual(pTab) ){
    بازگشت vtabIsReadOnly(pParse, pTab);
  }
  اگر( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) بازگشت 0;
  db = pParse->db;
  اگر( (pTab->tabFlags & TF_Readonly)!=0 ){
    بازگشت sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  أكد( pTab->tabFlags & TF_Shadow );
  بازگشت sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
صحیح sqlite3IsReadOnly(Parse *pParse, Table *pTab, Trigger *pTrigger){
  اگر( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_پیام(pParse, "table %s may not be modified", pTab->zName);
    بازگشت 1;
  }
#ifndef SQLITE_OMIT_VIEW
  اگر( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->pNext==0))
  ){
    sqlite3_Error_پیام(pParse,"cannot modify %s because it is a view",pTab->zName);
    بازگشت 1;
  }
#endif
  بازگشت 0;
}


#اگر !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
تهی sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  Table *pView,        /* View definition */
  عبارت *pWhere,        /* Optional WHERE clause to be added */
  عبارت_List *pOrderBy,  /* Optional ORDER BY clause */
  عبارت *pLimit,        /* Optional LIMIT clause */
  صحیح iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  صحیح iDb = sqlite3SchemaToIndex(db, pView->pSchema);
  pWhere = sqlite3_عبارت_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  اگر( pFrom ){
    أكد( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    أكد( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    أكد( pFrom->a[0].fg.isUsing==0 );
    أكد( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, pOrderBy,
                          SF_IncludeHidden, pLimit);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#endif /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#اگر defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
عبارت *sqlite3LimitWhere(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  عبارت *pWhere,                /* The WHERE clause.  May be null */
  عبارت_List *pOrderBy,          /* The ORDER BY clause.  May be null */
  عبارت *pLimit,                /* The LIMIT clause.  May be null */
  نویسه *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  عبارت *pLhs = پوچ;           /* LHS of IN(SELECT...) operator */
  عبارت *pInClause = پوچ;      /* WHERE rowid IN ( select ) */
  عبارت_List *pEList = پوچ;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = پوچ;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = پوچ;      /* Complete SELECT tree */
  Table *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  اگر( pOrderBy && pLimit==0 ) {
    sqlite3_Error_پیام(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_عبارت_Delete(pParse->db, pWhere);
    sqlite3_عبارت_List_Delete(pParse->db, pOrderBy);
    بازگشت 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  اگر( pLimit == 0 ) {
    بازگشت pWhere;
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
  اگر( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_ROW, 0, 0);
    pEList = sqlite3_عبارت_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_ROW, 0, 0)
    );
  }وگرنه{
    Index *pPk = sqlite3_Primary_کلید_Index(pTab);
    أكد( pPk!=0 );
    أكد( pPk->n_کلید_Col>=1 );
    اگر( pPk->n_کلید_Col==1 ){
      ثابت نویسه *zName;
      أكد( pPk->aiColumn[0]>=0 && pPk->aiColumn[0]<pTab->nCol );
      zName = pTab->aCol[pPk->aiColumn[0]].zCnName;
      pLhs = sqlite3_عبارت(db, TK_ID, zName);
      pEList = sqlite3_عبارت_List_Append(pParse, 0, sqlite3_عبارت(db, TK_ID, zName));
    }وگرنه{
      صحیح i;
      برای(i=0; i<pPk->n_کلید_Col; i++){
        عبارت *p;
        أكد( pPk->aiColumn[i]>=0 && pPk->aiColumn[i]<pTab->nCol );
        p = sqlite3_عبارت(db, TK_ID, pTab->aCol[pPk->aiColumn[i]].zCnName);
        pEList = sqlite3_عبارت_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_VECTOR, 0, 0);
      اگر( pLhs ){
        pLhs->x.pList = sqlite3_عبارت_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  اگر( pSrc->a[0].fg.isIndexedBy ){
    أكد( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.isIndexedBy = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.zIndexedBy);
  }وگرنه اگر( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      pOrderBy,0,pLimit
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  pInClause = sqlite3PExpr(pParse, TK_IN, pLhs, 0);
  sqlite3PExprAddSelect(pParse, pInClause, pSelect);
  بازگشت pInClause;
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
تهی sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  عبارت *pWhere,          /* The WHERE clause.  May be null */
  عبارت_List *pOrderBy,    /* ORDER BY clause. May be null */
  عبارت *pLimit           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  Table *pTab;           /* The table from which records will be deleted */
  صحیح i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  Index *pIdx;           /* For looping over indices of the table */
  صحیح iTabCur;           /* Cursor number for the table */
  صحیح iDataCur = 0;      /* VDBE cursor for the canonical data source */
  صحیح iIdxCur = 0;       /* Cursor number of the first index */
  صحیح nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  صحیح iDb;               /* Database number */
  صحیح memCnt = 0;        /* Memory cell used for change counting */
  صحیح rcauth;            /* Value returned by authorization callback */
  صحیح eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  صحیح aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *aToOpen = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  Index *pPk;            /* The PRIMARY KEY index on the table */
  صحیح iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  صحیح i_کلید;              /* Memory cell holding key of row to be deleted */
  i16 n_کلید;              /* Number of memory cells in the row key */
  صحیح iEphCur = 0;       /* Ephemeral table holding all primary key values */
  صحیح iRowSet = 0;       /* Register for rowset of rows to delete */
  صحیح addrBypass = 0;    /* Address of jump over the delete logic */
  صحیح addrLoop = 0;      /* Top of the delete loop */
  صحیح addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  صحیح bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#ifndef SQLITE_OMIT_TRIGGER
  صحیح isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#endif

  memset(&sContext, 0, اندازه(sContext));
  db = pParse->db;
  أكد( db->pParse==pParse );
  اگر( pParse->nErr ){
    برو delete_from_cleanup;
  }
  أكد( db->mallocFailed==0 );
  أكد( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  اگر( pTab==0 )  برو delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#وگرنه
# define pTrigger 0
# define isView 0
#endif
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif

#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x10000 ){
    sqlite3_درخت_View_سطر(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_درخت_View_Delete(pParse->pWith, pTabList, pWhere,
                          pOrderBy, pLimit, pTrigger);
  }
#endif

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  اگر( !isView ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "DELETE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  /* If pTab is really a view, make sure it has been initialized.
  */
  اگر( sqlite3ViewGetColumnNames(pParse, pTab) ){
    برو delete_from_cleanup;
  }

  اگر( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    برو delete_from_cleanup;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  أكد( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  أكد( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  اگر( rcauth==SQLITE_DENY ){
    برو delete_from_cleanup;
  }
  أكد(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  أكد( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  برای(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  اگر( isView ){
    sqlite3_Auth_Context_افزودن(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  اگر( v==0 ){
    برو delete_from_cleanup;
  }
  اگر( pParse->nested==0 ) sqlite3_Vdbe_شمار_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#اگر !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  اگر( isView ){
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
  memset(&sNC, 0, اندازه(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  اگر( sqlite3_Resolve_عبارت_Names(&sNC, pWhere) ){
    برو delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  اگر( (db->flags & SQLITE_شمار_Rows)!=0
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
  اگر( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#endif
  ){
    أكد( !isView );
    sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);
    اگر( HasRowid(pTab) ){
      sqlite3VdbeAddOp4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      أكد( pIdx->pSchema==pTab->pSchema );
      اگر( Is_Primary_کلید_Index(pIdx) && !HasRowid(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }وگرنه{
        sqlite3VdbeAddOp2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }وگرنه
#endif /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    اگر( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    اگر( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      أكد( nPk==1 );
      iRowSet = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, iRowSet);
    }وگرنه{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_کلید_Index(pTab);
      أكد( pPk!=0 );
      nPk = pPk->n_کلید_Col;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pPk);
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
    اگر( pWInfo==0 ) برو delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    أكد( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    أكد( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    اگر( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    اگر( sqlite3WhereUsesDeferredSeek(pWInfo) ){
      sqlite3VdbeAddOp1(v, OP_FinishSeek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    اگر( memCnt ){
      sqlite3VdbeAddOp2(v, OP_AddImm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    اگر( pPk ){
      برای(i=0; i<nPk; i++){
        أكد( pPk->aiColumn[i]>=0 );
        sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, iTabCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      i_کلید = iPk;
    }وگرنه{
      i_کلید = ++pParse->nMem;
      sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, iTabCur, -1, i_کلید);
    }
 
    اگر( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_کلید = nPk; /* OP_Found will use an unpacked key */
      aToOpen = sqlite3DbMallocRawNN(db, nIdx+2);
      اگر( aToOpen==0 ){
        sqlite3WhereEnd(pWInfo);
        برو delete_from_cleanup;
      }
      memset(aToOpen, 1, nIdx+1);
      aToOpen[nIdx+1] = 0;
      اگر( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iTabCur] = 0;
      اگر( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iTabCur] = 0;
      اگر( addrEphOpen ) sqlite3VdbeChangeToNoop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }وگرنه{
      اگر( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_کلید = ++pParse->nMem;
        n_کلید = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, i_کلید,
            sqlite3IndexAffinityStr(pParse->db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_درج, iEphCur, i_کلید, iPk, nPk);
      }وگرنه{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_کلید = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3VdbeAddOp2(v, OP_RowSetAdd, iRowSet, i_کلید);
      }
      sqlite3WhereEnd(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    اگر( !isView ){
      صحیح iAddrOnce = 0;
      اگر( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, aToOpen, &iDataCur, &iIdxCur);
      أكد( pPk || IsVirtual(pTab) || iDataCur==iTabCur );
      أكد( pPk || IsVirtual(pTab) || iIdxCur==iDataCur+1 );
      اگر( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_Here_Or_برداشت_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    اگر( eOnePass!=ONEPASS_OFF ){
      أكد( n_کلید==nPk );  /* OP_Found will use an unpacked key */
      اگر( !IsVirtual(pTab) && aToOpen[iDataCur-iTabCur] ){
        أكد( pPk!=0 || IsView(pTab) );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, addrBypass, i_کلید, n_کلید);
        VdbeCoverage(v);
      }
    }وگرنه اگر( pPk ){
      addrLoop = sqlite3VdbeAddOp1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      اگر( IsVirtual(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Column, iEphCur, 0, i_کلید);
      }وگرنه{
        sqlite3VdbeAddOp2(v, OP_RowData, iEphCur, i_کلید);
      }
      أكد( n_کلید==0 );  /* OP_Found will use a composite key */
    }وگرنه{
      addrLoop = sqlite3VdbeAddOp3(v, OP_RowSetRead, iRowSet, 0, i_کلید);
      VdbeCoverage(v);
      أكد( n_کلید==1 );
    } 
 
    /* Delete the row */
#ifndef SQLITE_OMIT_VIRTUALTABLE
    اگر( IsVirtual(pTab) ){
      ثابت نویسه *pVTab = (ثابت نویسه *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      أكد( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      اگر( eOnePass==ONEPASS_SINGLE ){
        sqlite3VdbeAddOp1(v, OP_Close, iTabCur);
        اگر( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3VdbeAddOp4(v, OP_VUpdate, 0, 1, i_کلید, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }وگرنه
#endif
    {
      صحیح شمار = (pParse->nested==0);    /* True to count changes */
      sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur,
          i_کلید, n_کلید, شمار, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    اگر( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3WhereEnd(pWInfo);
    }وگرنه اگر( pPk ){
      sqlite3VdbeAddOp2(v, OP_Next, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addrLoop);
    }وگرنه{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3VdbeJumpHere(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  اگر( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  اگر( memCnt ){
    sqlite3_Code_Change_شمار(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_برداشت(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_عبارت_Delete(db, pWhere);
#اگر defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_عبارت_List_Delete(db, pOrderBy);
  sqlite3_عبارت_Delete(db, pLimit);
#endif
  اگر( aToOpen ) sqlite3DbNNFreeNN(db, aToOpen);
  بازگشت;
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
تهی sqlite3GenerateRowDelete(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  صحیح iDataCur,      /* Cursor from which column data is extracted */
  صحیح iIdxCur,       /* First index cursor */
  صحیح iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 شمار,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 eMode,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  صحیح iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  صحیح iOld = 0;                   /* First register in OLD.* array */
  صحیح iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  أكد( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         iDataCur, iIdxCur, iPk, (صحیح)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_NotFound;
  اگر( eMode==ONEPASS_OFF ){
    sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_NotFound);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  اگر( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 mask;                     /* Mask of OLD.* columns in use */
    صحیح iCol;                     /* Iterator used while populating OLD.* */
    صحیح addrStart;                /* Start of BEFORE trigger programs */

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
    برای(iCol=0; iCol<pTab->nCol; iCol++){
      testcase( mask!=0xffffffff && iCol==31 );
      testcase( mask!=0xffffffff && iCol==32 );
      اگر( mask==0xffffffff || (iCol<=31 && (mask & MASKBIT32(iCol))!=0) ){
        صحیح kk = sqlite3TableColumnToStorage(pTab, iCol);
        sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, iDataCur, iCol, iOld+kk+1);
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
    اگر( addrStart<sqlite3VdbeCurrentAddr(v) ){
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
  اگر( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur,0,iIdxNoSeek);
    sqlite3VdbeAddOp2(v, OP_Delete, iDataCur, (شمار?OPFLAG_NCHANGE:0));
    اگر( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (نویسه*)pTab, P4_TABLE);
    }
    اگر( eMode!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    اگر( iIdxNoSeek>=0 && iIdxNoSeek!=iDataCur ){
      sqlite3VdbeAddOp1(v, OP_Delete, iIdxNoSeek);
    }
    اگر( eMode==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  اگر( pTrigger ){
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
تهی sqlite3GenerateRowIndexDelete(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* Table containing the row to be deleted */
  صحیح iDataCur,      /* Cursor of table holding data. */
  صحیح iIdxCur,       /* First index cursor */
  صحیح *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  صحیح iIdxNoSeek     /* Do not delete from this cursor */
){
  صحیح i;             /* Index loop counter */
  صحیح r1 = -1;       /* Register holding an index key */
  صحیح iPartIdxLabel; /* Jump destination for skipping partial index entries */
  Index *pIdx;       /* Current index */
  Index *pPrior = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  Index *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_کلید_Index(pTab);
  برای(i=0, pIdx=pTab->pIndex; pIdx; i++, pIdx=pIdx->pNext){
    صحیح p3 = 0;
    أكد( iIdxCur+i!=iDataCur || pPk==pIdx );
    اگر( aRegIdx!=0 && aRegIdx[i]==0 ) ادامه;
    اگر( pIdx==pPk ) ادامه;
    اگر( iIdxCur+i==iIdxNoSeek ) ادامه;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_Index_کلید(pParse, pIdx, iDataCur, 0, 1,
        &iPartIdxLabel, pPrior, r1);
    اگر( pIdx->b_Has_عبارت && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3VdbeAddOp3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (ثابت نویسه*)pIdx, P4_INDEX);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_کلید_Col : pIdx->nColumn);
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
صحیح sqlite3_Generate_Index_کلید(
  Parse *pParse,       /* Parsing context */
  Index *pIdx,         /* The index for which to generate a key */
  صحیح iDataCur,        /* Cursor number from which to take column data */
  صحیح regOut,          /* Put the new key into this register if not 0 */
  صحیح prefixOnly,      /* Compute only a unique prefix of the key */
  صحیح *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  Index *pPrior,       /* Previously generated index key */
  صحیح regPrior         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  صحیح j;
  صحیح regBase;
  صحیح nCol;

  اگر( piPartIdxLabel ){
    اگر( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = iDataCur + 1;
      sqlite3_عبارت_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      pPrior = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }وگرنه{
      *piPartIdxLabel = 0;
    }
  }
  nCol = (prefixOnly && pIdx->uniqNotNull) ? pIdx->n_کلید_Col : pIdx->nColumn;
  regBase = sqlite3GetTempRange(pParse, nCol);
  اگر( pPrior && (regBase!=regPrior || pPrior->pPartIdxWhere) ) pPrior = 0;
  برای(j=0; j<nCol; j++){
    صحیح iTabCol;
    اگر( pPrior
     && pPrior->aiColumn[j]==pIdx->aiColumn[j]
     && pPrior->aiColumn[j]!=XN_عبارت
    ){
      /* This column was already computed by the previous index */
      ادامه;
    }
    sqlite3_عبارت_Code_Load_Index_Column(pParse, pIdx, iDataCur, j, regBase+j);
    اگر( (iTabCol = pIdx->aiColumn[j])>=0 
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
  اگر( regOut ){
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regOut);
  }
  sqlite3ReleaseTempRange(pParse, regBase, nCol);
  بازگشت regBase;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
تهی sqlite3ResolvePartIdxLabel(Parse *pParse, صحیح iLabel){
  اگر( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
