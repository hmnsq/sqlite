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
#समावेश "sqliteInt.h"

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
  निश्चित( pItem && pSrc->nSrc>=1 );
  pTab = sqlite3LocateTableItem(pParse, 0, pItem);
  अगर( pItem->pSTab ) sqlite3DeleteTable(pParse->db, pItem->pSTab);
  pItem->pSTab = pTab;
  pItem->fg.notCte = 1;
  अगर( pTab ){
    pTab->nTabRef++;
    अगर( pItem->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pItem) ){
      pTab = 0;
    }
  }
  वापस pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
व्योम sqlite3_Code_Change_गिनती(Vdbe *v, पूर्णांक reg_गणक, स्थिर अक्षर *zColName){
  sqlite3VdbeAddOp0(v, OP_FkCheck);
  sqlite3VdbeAddOp2(v, OP_परिणाम_Row, reg_गणक, 1);
  sqlite3_Vdbe_Set_अंक_Cols(v, 1);
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
अटल पूर्णांक vtabIsReadOnly(Parse *pParse, Table *pTab){
  निश्चित( IsVirtual(pTab) );
  अगर( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    वापस 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  अगर( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_संदेश(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  वापस 0;
}
अटल पूर्णांक tabIsReadOnly(Parse *pParse, Table *pTab){
  sqlite3 *db;
  अगर( IsVirtual(pTab) ){
    वापस vtabIsReadOnly(pParse, pTab);
  }
  अगर( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) वापस 0;
  db = pParse->db;
  अगर( (pTab->tabFlags & TF_Readonly)!=0 ){
    वापस sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  निश्चित( pTab->tabFlags & TF_Shadow );
  वापस sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
पूर्णांक sqlite3IsReadOnly(Parse *pParse, Table *pTab, Trigger *pTrigger){
  अगर( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_संदेश(pParse, "table %s may not be modified", pTab->zName);
    वापस 1;
  }
#अगर_अघोषित SQLITE_OMIT_VIEW
  अगर( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->p_अग्र==0))
  ){
    sqlite3_Error_संदेश(pParse,"cannot modify %s because it is a view",pTab->zName);
    वापस 1;
  }
#पूर्ण_अगर
  वापस 0;
}


#अगर !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
व्योम sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  Table *pView,        /* View definition */
  व्यंजक *pWhere,        /* Optional WHERE clause to be added */
  व्यंजक_List *pOrderBy,  /* Optional ORDER BY clause */
  व्यंजक *pLimit,        /* Optional LIMIT clause */
  पूर्णांक iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  पूर्णांक iDb = sqlite3_Schema_To_सूचकांक(db, pView->pSchema);
  pWhere = sqlite3_व्यंजक_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  अगर( pFrom ){
    निश्चित( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    निश्चित( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    निश्चित( pFrom->a[0].fg.isUsing==0 );
    निश्चित( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, pOrderBy,
                          SF_IncludeHidden, pLimit);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#पूर्ण_अगर /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#अगर defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
व्यंजक *sqlite3LimitWhere(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  व्यंजक *pWhere,                /* The WHERE clause.  May be null */
  व्यंजक_List *pOrderBy,          /* The ORDER BY clause.  May be null */
  व्यंजक *pLimit,                /* The LIMIT clause.  May be null */
  अक्षर *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  व्यंजक *pLhs = शून्य;           /* LHS of IN(SELECT...) operator */
  व्यंजक *pInClause = शून्य;      /* WHERE rowid IN ( select ) */
  व्यंजक_List *pEList = शून्य;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = शून्य;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = शून्य;      /* Complete SELECT tree */
  Table *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  अगर( pOrderBy && pLimit==0 ) {
    sqlite3_Error_संदेश(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_व्यंजक_Delete(pParse->db, pWhere);
    sqlite3_व्यंजक_List_Delete(pParse->db, pOrderBy);
    वापस 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  अगर( pLimit == 0 ) {
    वापस pWhere;
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
  अगर( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_ROW, 0, 0);
    pEList = sqlite3_व्यंजक_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_ROW, 0, 0)
    );
  }अन्यथा{
    सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
    निश्चित( pPk!=0 );
    निश्चित( pPk->n_कुंजी_Col>=1 );
    अगर( pPk->n_कुंजी_Col==1 ){
      स्थिर अक्षर *zName;
      निश्चित( pPk->aiColumn[0]>=0 && pPk->aiColumn[0]<pTab->nCol );
      zName = pTab->aCol[pPk->aiColumn[0]].zCnName;
      pLhs = sqlite3_व्यंजक(db, TK_ID, zName);
      pEList = sqlite3_व्यंजक_List_Append(pParse, 0, sqlite3_व्यंजक(db, TK_ID, zName));
    }अन्यथा{
      पूर्णांक i;
      क्रम(i=0; i<pPk->n_कुंजी_Col; i++){
        व्यंजक *p;
        निश्चित( pPk->aiColumn[i]>=0 && pPk->aiColumn[i]<pTab->nCol );
        p = sqlite3_व्यंजक(db, TK_ID, pTab->aCol[pPk->aiColumn[i]].zCnName);
        pEList = sqlite3_व्यंजक_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_VECTOR, 0, 0);
      अगर( pLhs ){
        pLhs->x.pList = sqlite3_व्यंजक_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  अगर( pSrc->a[0].fg.isIndexedBy ){
    निश्चित( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.isIndexedBy = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.zIndexedBy);
  }अन्यथा अगर( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      pOrderBy,0,pLimit
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  pInClause = sqlite3PExpr(pParse, TK_IN, pLhs, 0);
  sqlite3PExprAddSelect(pParse, pInClause, pSelect);
  वापस pInClause;
}
#पूर्ण_अगर /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
व्योम sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  व्यंजक *pWhere,          /* The WHERE clause.  May be null */
  व्यंजक_List *pOrderBy,    /* ORDER BY clause. May be null */
  व्यंजक *pLimit           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  Table *pTab;           /* The table from which records will be deleted */
  पूर्णांक i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  सूचकांक *pIdx;           /* For looping over indices of the table */
  पूर्णांक iTabCur;           /* Cursor number for the table */
  पूर्णांक i_तथ्य_Cur = 0;      /* VDBE cursor for the canonical data source */
  पूर्णांक iIdxCur = 0;       /* Cursor number of the first index */
  पूर्णांक nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  पूर्णांक iDb;               /* Database number */
  पूर्णांक memCnt = 0;        /* Memory cell used for change counting */
  पूर्णांक rcauth;            /* Value returned by authorization callback */
  पूर्णांक eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  पूर्णांक aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *aToOpen = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  सूचकांक *pPk;            /* The PRIMARY KEY index on the table */
  पूर्णांक iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  पूर्णांक i_कुंजी;              /* Memory cell holding key of row to be deleted */
  i16 n_कुंजी;              /* Number of memory cells in the row key */
  पूर्णांक iEphCur = 0;       /* Ephemeral table holding all primary key values */
  पूर्णांक iRowSet = 0;       /* Register for rowset of rows to delete */
  पूर्णांक addrBypass = 0;    /* Address of jump over the delete logic */
  पूर्णांक addrLoop = 0;      /* Top of the delete loop */
  पूर्णांक addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  पूर्णांक bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#अगर_अघोषित SQLITE_OMIT_TRIGGER
  पूर्णांक isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#पूर्ण_अगर

  स_रखो(&sContext, 0, माप(sContext));
  db = pParse->db;
  निश्चित( db->pParse==pParse );
  अगर( pParse->nErr ){
    जाओ delete_from_cleanup;
  }
  निश्चित( db->mallocFailed==0 );
  निश्चित( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  अगर( pTab==0 )  जाओ delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#अगर_अघोषित SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#अन्यथा
# define pTrigger 0
# define isView 0
#पूर्ण_अगर
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#अगर_घोषित SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#पूर्ण_अगर

#अगर TREETRACE_ENABLED
  अगर( sqlite3_वृक्ष_Trace & 0x10000 ){
    sqlite3_वृक्ष_View_पंक्ति(0, "In sqlite3Delete() at %s:%d", __खाता__, __LINE__);
    sqlite3_वृक्ष_View_Delete(pParse->pWith, pTabList, pWhere,
                          pOrderBy, pLimit, pTrigger);
  }
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  अगर( !isView ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "DELETE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#पूर्ण_अगर

  /* If pTab is really a view, make sure it has been initialized.
  */
  अगर( sqlite3ViewGetColumnNames(pParse, pTab) ){
    जाओ delete_from_cleanup;
  }

  अगर( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    जाओ delete_from_cleanup;
  }
  iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
  निश्चित( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  निश्चित( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  अगर( rcauth==SQLITE_DENY ){
    जाओ delete_from_cleanup;
  }
  निश्चित(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  निश्चित( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  क्रम(nIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  अगर( isView ){
    sqlite3_Auth_Context_धकेलो(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  अगर( v==0 ){
    जाओ delete_from_cleanup;
  }
  अगर( pParse->nested==0 ) sqlite3_Vdbe_गिनती_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#अगर !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  अगर( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, pOrderBy, pLimit, iTabCur
    );
    i_तथ्य_Cur = iIdxCur = iTabCur;
    pOrderBy = 0;
    pLimit = 0;
  }
#पूर्ण_अगर

  /* Resolve the column names in the WHERE clause.
  */
  स_रखो(&sNC, 0, माप(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  अगर( sqlite3_Resolve_व्यंजक_Names(&sNC, pWhere) ){
    जाओ delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  अगर( (db->flags & SQLITE_गिनती_Rows)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, memCnt);
  }

#अगर_अघोषित SQLITE_OMIT_TRUNCATE_OPTIMIZATION
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
  अगर( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#पूर्ण_अगर
  ){
    निश्चित( !isView );
    sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);
    अगर( HasRowid(pTab) ){
      sqlite3VdbeAddOp4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
      निश्चित( pIdx->pSchema==pTab->pSchema );
      अगर( Is_Primary_कुंजी_सूचकांक(pIdx) && !HasRowid(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }अन्यथा
#पूर्ण_अगर /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    अगर( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    अगर( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      निश्चित( nPk==1 );
      iRowSet = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, iRowSet);
    }अन्यथा{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      निश्चित( pPk!=0 );
      nPk = pPk->n_कुंजी_Col;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pPk);
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
    अगर( pWInfo==0 ) जाओ delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    निश्चित( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    निश्चित( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    अगर( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    अगर( sqlite3WhereUsesDeferredSeek(pWInfo) ){
      sqlite3VdbeAddOp1(v, OP_FinishSeek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    अगर( memCnt ){
      sqlite3VdbeAddOp2(v, OP_AddImm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    अगर( pPk ){
      क्रम(i=0; i<nPk; i++){
        निश्चित( pPk->aiColumn[i]>=0 );
        sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, iTabCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      i_कुंजी = iPk;
    }अन्यथा{
      i_कुंजी = ++pParse->nMem;
      sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, iTabCur, -1, i_कुंजी);
    }
 
    अगर( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_कुंजी = nPk; /* OP_Found will use an unpacked key */
      aToOpen = sqlite3DbMallocRawNN(db, nIdx+2);
      अगर( aToOpen==0 ){
        sqlite3WhereEnd(pWInfo);
        जाओ delete_from_cleanup;
      }
      स_रखो(aToOpen, 1, nIdx+1);
      aToOpen[nIdx+1] = 0;
      अगर( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iTabCur] = 0;
      अगर( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iTabCur] = 0;
      अगर( addrEphOpen ) sqlite3VdbeChangeToNoop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }अन्यथा{
      अगर( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_कुंजी = ++pParse->nMem;
        n_कुंजी = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, i_कुंजी,
            sqlite3_सूचकांक_Affinity_Str(pParse->db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_जोड़ो, iEphCur, i_कुंजी, iPk, nPk);
      }अन्यथा{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_कुंजी = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3VdbeAddOp2(v, OP_RowSetAdd, iRowSet, i_कुंजी);
      }
      sqlite3WhereEnd(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    अगर( !isView ){
      पूर्णांक iAddrOnce = 0;
      अगर( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, aToOpen, &i_तथ्य_Cur, &iIdxCur);
      निश्चित( pPk || IsVirtual(pTab) || i_तथ्य_Cur==iTabCur );
      निश्चित( pPk || IsVirtual(pTab) || iIdxCur==i_तथ्य_Cur+1 );
      अगर( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_Here_Or_निकालो_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    अगर( eOnePass!=ONEPASS_OFF ){
      निश्चित( n_कुंजी==nPk );  /* OP_Found will use an unpacked key */
      अगर( !IsVirtual(pTab) && aToOpen[i_तथ्य_Cur-iTabCur] ){
        निश्चित( pPk!=0 || IsView(pTab) );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, i_तथ्य_Cur, addrBypass, i_कुंजी, n_कुंजी);
        VdbeCoverage(v);
      }
    }अन्यथा अगर( pPk ){
      addrLoop = sqlite3VdbeAddOp1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      अगर( IsVirtual(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Column, iEphCur, 0, i_कुंजी);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Row_तथ्य, iEphCur, i_कुंजी);
      }
      निश्चित( n_कुंजी==0 );  /* OP_Found will use a composite key */
    }अन्यथा{
      addrLoop = sqlite3VdbeAddOp3(v, OP_RowSetRead, iRowSet, 0, i_कुंजी);
      VdbeCoverage(v);
      निश्चित( n_कुंजी==1 );
    } 
 
    /* Delete the row */
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    अगर( IsVirtual(pTab) ){
      स्थिर अक्षर *pVTab = (स्थिर अक्षर *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      निश्चित( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      अगर( eOnePass==ONEPASS_SINGLE ){
        sqlite3VdbeAddOp1(v, OP_Close, iTabCur);
        अगर( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3VdbeAddOp4(v, OP_VUpdate, 0, 1, i_कुंजी, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }अन्यथा
#पूर्ण_अगर
    {
      पूर्णांक गिनती = (pParse->nested==0);    /* True to count changes */
      sqlite3GenerateRowDelete(pParse, pTab, pTrigger, i_तथ्य_Cur, iIdxCur,
          i_कुंजी, n_कुंजी, गिनती, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    अगर( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3WhereEnd(pWInfo);
    }अन्यथा अगर( pPk ){
      sqlite3VdbeAddOp2(v, OP_अग्र, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addrLoop);
    }अन्यथा{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3VdbeJumpHere(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  अगर( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  अगर( memCnt ){
    sqlite3_Code_Change_गिनती(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_निकालो(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_व्यंजक_Delete(db, pWhere);
#अगर defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_व्यंजक_List_Delete(db, pOrderBy);
  sqlite3_व्यंजक_Delete(db, pLimit);
#पूर्ण_अगर
  अगर( aToOpen ) sqlite3DbNNFreeNN(db, aToOpen);
  वापस;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#अगर_घोषित isView
 #अघोषित isView
#पूर्ण_अगर
#अगर_घोषित pTrigger
 #अघोषित pTrigger
#पूर्ण_अगर

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
व्योम sqlite3GenerateRowDelete(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  पूर्णांक i_तथ्य_Cur,      /* Cursor from which column data is extracted */
  पूर्णांक iIdxCur,       /* First index cursor */
  पूर्णांक iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 गिनती,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 eMode,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  पूर्णांक iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  पूर्णांक iOld = 0;                   /* First register in OLD.* array */
  पूर्णांक iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  निश्चित( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         i_तथ्य_Cur, iIdxCur, iPk, (पूर्णांक)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_NotFound;
  अगर( eMode==ONEPASS_OFF ){
    sqlite3VdbeAddOp4Int(v, opSeek, i_तथ्य_Cur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_NotFound);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  अगर( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 mask;                     /* Mask of OLD.* columns in use */
    पूर्णांक iCol;                     /* Iterator used while populating OLD.* */
    पूर्णांक addrStart;                /* Start of BEFORE trigger programs */

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
    क्रम(iCol=0; iCol<pTab->nCol; iCol++){
      testcase( mask!=0xffffffff && iCol==31 );
      testcase( mask!=0xffffffff && iCol==32 );
      अगर( mask==0xffffffff || (iCol<=31 && (mask & MASKBIT32(iCol))!=0) ){
        पूर्णांक kk = sqlite3TableColumnToStorage(pTab, iCol);
        sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur, iCol, iOld+kk+1);
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
    अगर( addrStart<sqlite3VdbeCurrentAddr(v) ){
      sqlite3VdbeAddOp4Int(v, opSeek, i_तथ्य_Cur, iLabel, iPk, nPk);
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
  अगर( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3_Generate_Row_सूचकांक_Delete(pParse, pTab, i_तथ्य_Cur, iIdxCur,0,iIdxNoSeek);
    sqlite3VdbeAddOp2(v, OP_Delete, i_तथ्य_Cur, (गिनती?OPFLAG_NCHANGE:0));
    अगर( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (अक्षर*)pTab, P4_TABLE);
    }
    अगर( eMode!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    अगर( iIdxNoSeek>=0 && iIdxNoSeek!=i_तथ्य_Cur ){
      sqlite3VdbeAddOp1(v, OP_Delete, iIdxNoSeek);
    }
    अगर( eMode==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  अगर( pTrigger ){
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
व्योम sqlite3_Generate_Row_सूचकांक_Delete(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* Table containing the row to be deleted */
  पूर्णांक i_तथ्य_Cur,      /* Cursor of table holding data. */
  पूर्णांक iIdxCur,       /* First index cursor */
  पूर्णांक *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  पूर्णांक iIdxNoSeek     /* Do not delete from this cursor */
){
  पूर्णांक i;             /* Index loop counter */
  पूर्णांक r1 = -1;       /* Register holding an index key */
  पूर्णांक iPartIdxLabel; /* Jump destination for skipping partial index entries */
  सूचकांक *pIdx;       /* Current index */
  सूचकांक *pPrior = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  सूचकांक *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_कुंजी_सूचकांक(pTab);
  क्रम(i=0, pIdx=pTab->p_सूचकांक; pIdx; i++, pIdx=pIdx->p_अग्र){
    पूर्णांक p3 = 0;
    निश्चित( iIdxCur+i!=i_तथ्य_Cur || pPk==pIdx );
    अगर( aRegIdx!=0 && aRegIdx[i]==0 ) जारी;
    अगर( pIdx==pPk ) जारी;
    अगर( iIdxCur+i==iIdxNoSeek ) जारी;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_सूचकांक_कुंजी(pParse, pIdx, i_तथ्य_Cur, 0, 1,
        &iPartIdxLabel, pPrior, r1);
    अगर( pIdx->b_Has_व्यंजक && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3VdbeAddOp3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (स्थिर अक्षर*)pIdx, P4_सूचकांक);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_कुंजी_Col : pIdx->nColumn);
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
पूर्णांक sqlite3_Generate_सूचकांक_कुंजी(
  Parse *pParse,       /* Parsing context */
  सूचकांक *pIdx,         /* The index for which to generate a key */
  पूर्णांक i_तथ्य_Cur,        /* Cursor number from which to take column data */
  पूर्णांक regOut,          /* Put the new key into this register if not 0 */
  पूर्णांक prefixOnly,      /* Compute only a unique prefix of the key */
  पूर्णांक *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  सूचकांक *pPrior,       /* Previously generated index key */
  पूर्णांक regPrior         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णांक j;
  पूर्णांक regBase;
  पूर्णांक nCol;

  अगर( piPartIdxLabel ){
    अगर( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = i_तथ्य_Cur + 1;
      sqlite3_व्यंजक_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      pPrior = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }अन्यथा{
      *piPartIdxLabel = 0;
    }
  }
  nCol = (prefixOnly && pIdx->uniqNotNull) ? pIdx->n_कुंजी_Col : pIdx->nColumn;
  regBase = sqlite3_Get_अस्थायी_Range(pParse, nCol);
  अगर( pPrior && (regBase!=regPrior || pPrior->pPartIdxWhere) ) pPrior = 0;
  क्रम(j=0; j<nCol; j++){
    पूर्णांक iTabCol;
    अगर( pPrior
     && pPrior->aiColumn[j]==pIdx->aiColumn[j]
     && pPrior->aiColumn[j]!=XN_व्यंजक
    ){
      /* This column was already computed by the previous index */
      जारी;
    }
    sqlite3_व्यंजक_Code_Load_सूचकांक_Column(pParse, pIdx, i_तथ्य_Cur, j, regBase+j);
    अगर( (iTabCol = pIdx->aiColumn[j])>=0 
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
  अगर( regOut ){
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regOut);
  }
  sqlite3_Release_अस्थायी_Range(pParse, regBase, nCol);
  वापस regBase;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
व्योम sqlite3ResolvePartIdxLabel(Parse *pParse, पूर्णांक iLabel){
  अगर( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
