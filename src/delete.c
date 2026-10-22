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
#अन्तर्भाव "sqliteInt.h"

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
सारणी *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  SrcItem *pItem = pSrc->a;
  सारणी *pTab;
  प्रतिज्ञा( pItem && pSrc->nSrc>=1 );
  pTab = sqlite3_Locate_सारणी_Item(pParse, 0, pItem);
  यदि( pItem->pSTab ) sqlite3_Delete_सारणी(pParse->db, pItem->pSTab);
  pItem->pSTab = pTab;
  pItem->fg.notCte = 1;
  यदि( pTab ){
    pTab->nTabRef++;
    यदि( pItem->fg.is_अनुक्रमितम्_By && sqlite3_अनुक्रमितम्_By_Lookup(pParse, pItem) ){
      pTab = 0;
    }
  }
  प्रत्यागम pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
शून्य sqlite3_संहिता_Change_गणनम्(Vdbe *v, पूर्णाङ्क reg_गणक, ध्रुव वर्ण *zColName){
  sqlite3_Vdbe_सङ्कलय_Op0(v, OP_FkCheck);
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, reg_गणक, 1);
  sqlite3_Vdbe_समुच्चयः_अङ्कः_स्तम्भसङ्ख्या(v, 1);
  sqlite3_Vdbe_समुच्चयः_Col_Name(v, 0, COLNAME_NAME, zColName, SQLITE_STATIC);
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
स्थिर पूर्णाङ्क vtabIsReadOnly(Parse *pParse, सारणी *pTab){
  प्रतिज्ञा( IsVirtual(pTab) );
  यदि( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    प्रत्यागम 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  यदि( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_सन्देशः(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  प्रत्यागम 0;
}
स्थिर पूर्णाङ्क tabIsReadOnly(Parse *pParse, सारणी *pTab){
  sqlite3 *db;
  यदि( IsVirtual(pTab) ){
    प्रत्यागम vtabIsReadOnly(pParse, pTab);
  }
  यदि( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) प्रत्यागम 0;
  db = pParse->db;
  यदि( (pTab->tabFlags & TF_Readonly)!=0 ){
    प्रत्यागम sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  प्रतिज्ञा( pTab->tabFlags & TF_Shadow );
  प्रत्यागम sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
पूर्णाङ्क sqlite3IsReadOnly(Parse *pParse, सारणी *pTab, Trigger *pTrigger){
  यदि( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_सन्देशः(pParse, "table %s may not be modified", pTab->zName);
    प्रत्यागम 1;
  }
#यदि_अपरिभाषितम् SQLITE_OMIT_VIEW
  यदि( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->p_अग्र==0))
  ){
    sqlite3_Error_सन्देशः(pParse,"cannot modify %s because it is a view",pTab->zName);
    प्रत्यागम 1;
  }
#यदि_अन्तः
  प्रत्यागम 0;
}


#यदि !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
शून्य sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  सारणी *pView,        /* View definition */
  व्यञ्जकम् *pWhere,        /* Optional WHERE clause to be added */
  व्यञ्जकम्_List *p_क्रमः_By,  /* Optional ORDER BY clause */
  व्यञ्जकम् *p_सीमा,        /* Optional LIMIT clause */
  पूर्णाङ्क iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pView->pSchema);
  pWhere = sqlite3_व्यञ्जकम्_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  यदि( pFrom ){
    प्रतिज्ञा( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    प्रतिज्ञा( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    प्रतिज्ञा( pFrom->a[0].fg.isUsing==0 );
    प्रतिज्ञा( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, p_क्रमः_By,
                          SF_IncludeHidden, p_सीमा);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#यदि defined(SQLITE_ENABLE_UPDATE_DELETE_सीमा) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
व्यञ्जकम् *sqlite3_सीमा_Where(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  व्यञ्जकम् *pWhere,                /* The WHERE clause.  May be null */
  व्यञ्जकम्_List *p_क्रमः_By,          /* The ORDER BY clause.  May be null */
  व्यञ्जकम् *p_सीमा,                /* The LIMIT clause.  May be null */
  वर्ण *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  व्यञ्जकम् *pLhs = रिक्त;           /* LHS of IN(SELECT...) operator */
  व्यञ्जकम् *p_अन्तर्_Clause = रिक्त;      /* WHERE rowid IN ( select ) */
  व्यञ्जकम्_List *pEList = रिक्त;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = रिक्त;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = रिक्त;      /* Complete SELECT tree */
  सारणी *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  यदि( p_क्रमः_By && p_सीमा==0 ) {
    sqlite3_Error_सन्देशः(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_व्यञ्जकम्_Delete(pParse->db, pWhere);
    sqlite3_व्यञ्जकम्_List_Delete(pParse->db, p_क्रमः_By);
    प्रत्यागम 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  यदि( p_सीमा == 0 ) {
    प्रत्यागम pWhere;
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
  यदि( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_ROW, 0, 0);
    pEList = sqlite3_व्यञ्जकम्_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_ROW, 0, 0)
    );
  }अन्यथा{
    सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
    प्रतिज्ञा( pPk!=0 );
    प्रतिज्ञा( pPk->n_कुञ्चिका_Col>=1 );
    यदि( pPk->n_कुञ्चिका_Col==1 ){
      ध्रुव वर्ण *zName;
      प्रतिज्ञा( pPk->aiColumn[0]>=0 && pPk->aiColumn[0]<pTab->nCol );
      zName = pTab->aCol[pPk->aiColumn[0]].zCnName;
      pLhs = sqlite3_व्यञ्जकम्(db, TK_ID, zName);
      pEList = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, sqlite3_व्यञ्जकम्(db, TK_ID, zName));
    }अन्यथा{
      पूर्णाङ्क i;
      प्रति(i=0; i<pPk->n_कुञ्चिका_Col; i++){
        व्यञ्जकम् *p;
        प्रतिज्ञा( pPk->aiColumn[i]>=0 && pPk->aiColumn[i]<pTab->nCol );
        p = sqlite3_व्यञ्जकम्(db, TK_ID, pTab->aCol[pPk->aiColumn[i]].zCnName);
        pEList = sqlite3_व्यञ्जकम्_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_सदिशम्, 0, 0);
      यदि( pLhs ){
        pLhs->x.pList = sqlite3_व्यञ्जकम्_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  यदि( pSrc->a[0].fg.is_अनुक्रमितम्_By ){
    प्रतिज्ञा( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.is_अनुक्रमितम्_By = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.z_अनुक्रमितम्_By);
  }अन्यथा यदि( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      p_क्रमः_By,0,p_सीमा
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  p_अन्तर्_Clause = sqlite3PExpr(pParse, TK_अन्तर्, pLhs, 0);
  sqlite3_PExpr_सङ्कलय_Select(pParse, p_अन्तर्_Clause, pSelect);
  प्रत्यागम p_अन्तर्_Clause;
}
#यदि_अन्तः /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
शून्य sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  व्यञ्जकम् *pWhere,          /* The WHERE clause.  May be null */
  व्यञ्जकम्_List *p_क्रमः_By,    /* ORDER BY clause. May be null */
  व्यञ्जकम् *p_सीमा           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  सारणी *pTab;           /* The table from which records will be deleted */
  पूर्णाङ्क i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  सूचकांक *pIdx;           /* For looping over indices of the table */
  पूर्णाङ्क iTabCur;           /* Cursor number for the table */
  पूर्णाङ्क i_दत्त_Cur = 0;      /* VDBE cursor for the canonical data source */
  पूर्णाङ्क iIdxCur = 0;       /* Cursor number of the first index */
  पूर्णाङ्क nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  पूर्णाङ्क iDb;               /* Database number */
  पूर्णाङ्क memCnt = 0;        /* Memory cell used for change counting */
  पूर्णाङ्क rcauth;            /* Value returned by authorization callback */
  पूर्णाङ्क eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  पूर्णाङ्क aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *a_गन्तव्यम्_Open = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  सूचकांक *pPk;            /* The PRIMARY KEY index on the table */
  पूर्णाङ्क iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  पूर्णाङ्क i_कुञ्चिका;              /* Memory cell holding key of row to be deleted */
  i16 n_कुञ्चिका;              /* Number of memory cells in the row key */
  पूर्णाङ्क iEphCur = 0;       /* Ephemeral table holding all primary key values */
  पूर्णाङ्क i_Row_समुच्चयः = 0;       /* Register for rowset of rows to delete */
  पूर्णाङ्क addrBypass = 0;    /* Address of jump over the delete logic */
  पूर्णाङ्क addrLoop = 0;      /* Top of the delete loop */
  पूर्णाङ्क addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  पूर्णाङ्क bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  पूर्णाङ्क isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#यदि_अन्तः

  स्मृतिं_पूरय(&sContext, 0, परिमाण(sContext));
  db = pParse->db;
  प्रतिज्ञा( db->pParse==pParse );
  यदि( pParse->nErr ){
    गच्छ delete_from_cleanup;
  }
  प्रतिज्ञा( db->mallocFailed==0 );
  प्रतिज्ञा( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  यदि( pTab==0 )  गच्छ delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#अन्यथा
# define pTrigger 0
# define isView 0
#यदि_अन्तः
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#यदि_परिभाषितम् SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#यदि_अन्तः

#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x10000 ){
    sqlite3_वृक्षः_View_पङ्क्तिः(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_वृक्षः_View_Delete(pParse->pWith, pTabList, pWhere,
                          p_क्रमः_By, p_सीमा, pTrigger);
  }
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_UPDATE_DELETE_सीमा
  यदि( !isView ){
    pWhere = sqlite3_सीमा_Where(
        pParse, pTabList, pWhere, p_क्रमः_By, p_सीमा, "DELETE"
    );
    p_क्रमः_By = 0;
    p_सीमा = 0;
  }
#यदि_अन्तः

  /* If pTab is really a view, make sure it has been initialized.
  */
  यदि( sqlite3_View_Get_Column_नामानि(pParse, pTab) ){
    गच्छ delete_from_cleanup;
  }

  यदि( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    गच्छ delete_from_cleanup;
  }
  iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
  प्रतिज्ञा( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  प्रतिज्ञा( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  यदि( rcauth==SQLITE_DENY ){
    गच्छ delete_from_cleanup;
  }
  प्रतिज्ञा(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  प्रतिज्ञा( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  प्रति(nIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  यदि( isView ){
    sqlite3_Auth_Context_प्रक्षिप(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  यदि( v==0 ){
    गच्छ delete_from_cleanup;
  }
  यदि( pParse->nested==0 ) sqlite3_Vdbe_गणनम्_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#यदि !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  यदि( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_क्रमः_By, p_सीमा, iTabCur
    );
    i_दत्त_Cur = iIdxCur = iTabCur;
    p_क्रमः_By = 0;
    p_सीमा = 0;
  }
#यदि_अन्तः

  /* Resolve the column names in the WHERE clause.
  */
  स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  यदि( sqlite3_Resolve_व्यञ्जकम्_नामानि(&sNC, pWhere) ){
    गच्छ delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  यदि( (db->flags & SQLITE_गणनम्_पङ्क्तिकाः)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, memCnt);
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_TRUNCATE_OPTIMIZATION
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
  यदि( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#यदि_अन्तः
  ){
    प्रतिज्ञा( !isView );
    sqlite3_सारणी_तालकम्(pParse, iDb, pTab->tnum, 1, pTab->zName);
    यदि( HasRowid(pTab) ){
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
      प्रतिज्ञा( pIdx->pSchema==pTab->pSchema );
      यदि( Is_Primary_कुञ्चिका_सूचकांक(pIdx) && !HasRowid(pTab) ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }अन्यथा
#यदि_अन्तः /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    यदि( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    यदि( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      प्रतिज्ञा( nPk==1 );
      i_Row_समुच्चयः = ++pParse->nMem;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, i_Row_समुच्चयः);
    }अन्यथा{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      प्रतिज्ञा( pPk!=0 );
      nPk = pPk->n_कुञ्चिका_Col;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pPk);
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
    यदि( pWInfo==0 ) गच्छ delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    प्रतिज्ञा( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    प्रतिज्ञा( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    यदि( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    यदि( sqlite3_Where_उपयुङ्क्ते_Deferred_Seek(pWInfo) ){
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_FinishSeek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    यदि( memCnt ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_सङ्कलय_Imm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    यदि( pPk ){
      प्रति(i=0; i<nPk; i++){
        प्रतिज्ञा( pPk->aiColumn[i]>=0 );
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iTabCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      i_कुञ्चिका = iPk;
    }अन्यथा{
      i_कुञ्चिका = ++pParse->nMem;
      sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iTabCur, -1, i_कुञ्चिका);
    }
 
    यदि( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_कुञ्चिका = nPk; /* OP_Found will use an unpacked key */
      a_गन्तव्यम्_Open = sqlite3DbMallocRawNN(db, nIdx+2);
      यदि( a_गन्तव्यम्_Open==0 ){
        sqlite3_Where_अन्तः(pWInfo);
        गच्छ delete_from_cleanup;
      }
      स्मृतिं_पूरय(a_गन्तव्यम्_Open, 1, nIdx+1);
      a_गन्तव्यम्_Open[nIdx+1] = 0;
      यदि( aiCurOnePass[0]>=0 ) a_गन्तव्यम्_Open[aiCurOnePass[0]-iTabCur] = 0;
      यदि( aiCurOnePass[1]>=0 ) a_गन्तव्यम्_Open[aiCurOnePass[1]-iTabCur] = 0;
      यदि( addrEphOpen ) sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }अन्यथा{
      यदि( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_कुञ्चिका = ++pParse->nMem;
        n_कुञ्चिका = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Make_लेखः, iPk, nPk, i_कुञ्चिका,
            sqlite3_सूचकांक_Affinity_Str(pParse->db, pPk), nPk);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iEphCur, i_कुञ्चिका, iPk, nPk);
      }अन्यथा{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_कुञ्चिका = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Row_समुच्चयः_सङ्कलय, i_Row_समुच्चयः, i_कुञ्चिका);
      }
      sqlite3_Where_अन्तः(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    यदि( !isView ){
      पूर्णाङ्क iAddrOnce = 0;
      यदि( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3_Open_सारणी_And_Indices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, a_गन्तव्यम्_Open, &i_दत्त_Cur, &iIdxCur);
      प्रतिज्ञा( pPk || IsVirtual(pTab) || i_दत्त_Cur==iTabCur );
      प्रतिज्ञा( pPk || IsVirtual(pTab) || iIdxCur==i_दत्त_Cur+1 );
      यदि( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_अत्र_Or_निष्कासय_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    यदि( eOnePass!=ONEPASS_OFF ){
      प्रतिज्ञा( n_कुञ्चिका==nPk );  /* OP_Found will use an unpacked key */
      यदि( !IsVirtual(pTab) && a_गन्तव्यम्_Open[i_दत्त_Cur-iTabCur] ){
        प्रतिज्ञा( pPk!=0 || IsView(pTab) );
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Not_प्राप्तम्, i_दत्त_Cur, addrBypass, i_कुञ्चिका, n_कुञ्चिका);
        VdbeCoverage(v);
      }
    }अन्यथा यदि( pPk ){
      addrLoop = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      यदि( IsVirtual(pTab) ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iEphCur, 0, i_कुञ्चिका);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Row_दत्त, iEphCur, i_कुञ्चिका);
      }
      प्रतिज्ञा( n_कुञ्चिका==0 );  /* OP_Found will use a composite key */
    }अन्यथा{
      addrLoop = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Row_समुच्चयः_Read, i_Row_समुच्चयः, 0, i_कुञ्चिका);
      VdbeCoverage(v);
      प्रतिज्ञा( n_कुञ्चिका==1 );
    } 
 
    /* Delete the row */
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
    यदि( IsVirtual(pTab) ){
      ध्रुव वर्ण *pVTab = (ध्रुव वर्ण *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      प्रतिज्ञा( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      यदि( eOnePass==ONEPASS_SINGLE ){
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Close, iTabCur);
        यदि( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_VUpdate, 0, 1, i_कुञ्चिका, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }अन्यथा
#यदि_अन्तः
    {
      पूर्णाङ्क गणनम् = (pParse->nested==0);    /* True to count changes */
      sqlite3GenerateRowDelete(pParse, pTab, pTrigger, i_दत्त_Cur, iIdxCur,
          i_कुञ्चिका, n_कुञ्चिका, गणनम्, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    यदि( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3_Where_अन्तः(pWInfo);
    }अन्यथा यदि( pPk ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_अत्र(v, addrLoop);
    }अन्यथा{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3_Vdbe_Jump_अत्र(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  यदि( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_अन्तः(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  यदि( memCnt ){
    sqlite3_संहिता_Change_गणनम्(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_निष्कासय(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_व्यञ्जकम्_Delete(db, pWhere);
#यदि defined(SQLITE_ENABLE_UPDATE_DELETE_सीमा)
  sqlite3_व्यञ्जकम्_List_Delete(db, p_क्रमः_By);
  sqlite3_व्यञ्जकम्_Delete(db, p_सीमा);
#यदि_अन्तः
  यदि( a_गन्तव्यम्_Open ) sqlite3DbNNFreeNN(db, a_गन्तव्यम्_Open);
  प्रत्यागम;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#यदि_परिभाषितम् isView
 #अपरिभाषय isView
#यदि_अन्तः
#यदि_परिभाषितम् pTrigger
 #अपरिभाषय pTrigger
#यदि_अन्तः

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
शून्य sqlite3GenerateRowDelete(
  Parse *pParse,     /* Parsing context */
  सारणी *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  पूर्णाङ्क i_दत्त_Cur,      /* Cursor from which column data is extracted */
  पूर्णाङ्क iIdxCur,       /* First index cursor */
  पूर्णाङ्क iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 गणनम्,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 e_बहुलकम्,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  पूर्णाङ्क iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  पूर्णाङ्क iOld = 0;                   /* First register in OLD.* array */
  पूर्णाङ्क iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  प्रतिज्ञा( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         i_दत्त_Cur, iIdxCur, iPk, (पूर्णाङ्क)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_Not_प्राप्तम्;
  यदि( e_बहुलकम्==ONEPASS_OFF ){
    sqlite3_Vdbe_सङ्कलय_Op4_Int(v, opSeek, i_दत्त_Cur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_Not_प्राप्तम्);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  यदि( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 आवरणम्;                     /* Mask of OLD.* columns in use */
    पूर्णाङ्क iCol;                     /* Iterator used while populating OLD.* */
    पूर्णाङ्क addrStart;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    आवरणम् = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    आवरणम् |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->nCol);

    /* Populate the OLD.* pseudo-table register array. These values will be
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Copy, iPk, iOld);
    प्रति(iCol=0; iCol<pTab->nCol; iCol++){
      testcase( आवरणम्!=0xffffffff && iCol==31 );
      testcase( आवरणम्!=0xffffffff && iCol==32 );
      यदि( आवरणम्==0xffffffff || (iCol<=31 && (आवरणम् & MASKBIT32(iCol))!=0) ){
        पूर्णाङ्क kk = sqlite3_सारणी_Column_गन्तव्यम्_Storage(pTab, iCol);
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur, iCol, iOld+kk+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addrStart = sqlite3VdbeCurrentAddr(v);
    sqlite3_संहिता_Row_Trigger(pParse, pTrigger,
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
    यदि( addrStart<sqlite3VdbeCurrentAddr(v) ){
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, opSeek, i_दत्त_Cur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_Not_प्राप्तम्);
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
  यदि( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3_Generate_Row_सूचकांक_Delete(pParse, pTab, i_दत्त_Cur, iIdxCur,0,iIdxNoSeek);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Delete, i_दत्त_Cur, (गणनम्?OPFLAG_NCHANGE:0));
    यदि( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (वर्ण*)pTab, P4_सारणी);
    }
    यदि( e_बहुलकम्!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    यदि( iIdxNoSeek>=0 && iIdxNoSeek!=i_दत्त_Cur ){
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Delete, iIdxNoSeek);
    }
    यदि( e_बहुलकम्==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  यदि( pTrigger ){
    sqlite3_संहिता_Row_Trigger(pParse, pTrigger,
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
शून्य sqlite3_Generate_Row_सूचकांक_Delete(
  Parse *pParse,     /* Parsing and code generating context */
  सारणी *pTab,       /* Table containing the row to be deleted */
  पूर्णाङ्क i_दत्त_Cur,      /* Cursor of table holding data. */
  पूर्णाङ्क iIdxCur,       /* First index cursor */
  पूर्णाङ्क *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  पूर्णाङ्क iIdxNoSeek     /* Do not delete from this cursor */
){
  पूर्णाङ्क i;             /* Index loop counter */
  पूर्णाङ्क r1 = -1;       /* Register holding an index key */
  पूर्णाङ्क iPartIdxLabel; /* Jump destination for skipping partial index entries */
  सूचकांक *pIdx;       /* Current index */
  सूचकांक *p_प्राक् = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  सूचकांक *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
  प्रति(i=0, pIdx=pTab->p_सूचकांक; pIdx; i++, pIdx=pIdx->p_अग्र){
    पूर्णाङ्क p3 = 0;
    प्रतिज्ञा( iIdxCur+i!=i_दत्त_Cur || pPk==pIdx );
    यदि( aRegIdx!=0 && aRegIdx[i]==0 ) अनुवर्त;
    यदि( pIdx==pPk ) अनुवर्त;
    यदि( iIdxCur+i==iIdxNoSeek ) अनुवर्त;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_सूचकांक_कुञ्चिका(pParse, pIdx, i_दत्त_Cur, 0, 1,
        &iPartIdxLabel, p_प्राक्, r1);
    यदि( pIdx->b_Has_व्यञ्जकम् && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (ध्रुव वर्ण*)pIdx, P4_सूचकांक);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_कुञ्चिका_Col : pIdx->nColumn);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    p_प्राक् = pIdx;
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
पूर्णाङ्क sqlite3_Generate_सूचकांक_कुञ्चिका(
  Parse *pParse,       /* Parsing context */
  सूचकांक *pIdx,         /* The index for which to generate a key */
  पूर्णाङ्क i_दत्त_Cur,        /* Cursor number from which to take column data */
  पूर्णाङ्क reg_बहिर्गमनम्,          /* Put the new key into this register if not 0 */
  पूर्णाङ्क उपसर्गः_Only,      /* Compute only a unique prefix of the key */
  पूर्णाङ्क *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  सूचकांक *p_प्राक्,       /* Previously generated index key */
  पूर्णाङ्क reg_प्राक्         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क j;
  पूर्णाङ्क reg_आधारः;
  पूर्णाङ्क nCol;

  यदि( piPartIdxLabel ){
    यदि( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = i_दत्त_Cur + 1;
      sqlite3_व्यञ्जकम्_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      p_प्राक् = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }अन्यथा{
      *piPartIdxLabel = 0;
    }
  }
  nCol = (उपसर्गः_Only && pIdx->uniqNotNull) ? pIdx->n_कुञ्चिका_Col : pIdx->nColumn;
  reg_आधारः = sqlite3_Get_अस्थायी_Range(pParse, nCol);
  यदि( p_प्राक् && (reg_आधारः!=reg_प्राक् || p_प्राक्->pPartIdxWhere) ) p_प्राक् = 0;
  प्रति(j=0; j<nCol; j++){
    पूर्णाङ्क iTabCol;
    यदि( p_प्राक्
     && p_प्राक्->aiColumn[j]==pIdx->aiColumn[j]
     && p_प्राक्->aiColumn[j]!=XN_व्यञ्जकम्
    ){
      /* This column was already computed by the previous index */
      अनुवर्त;
    }
    sqlite3_व्यञ्जकम्_संहिता_आरोपय_सूचकांक_Column(pParse, pIdx, i_दत्त_Cur, j, reg_आधारः+j);
    यदि( (iTabCol = pIdx->aiColumn[j])>=0 
     && (pIdx->p_सारणी->aCol[iTabCol].colFlags & COLFLAG_VIRTUAL)==0
    ){
      /* If the column affinity is REAL but the number is an integer, then it
      ** might be stored in the table as an integer (using a compact
      ** representation) then converted to REAL by an OP_RealAffinity opcode.
      ** But we are getting ready to store this value back into an index, where
      ** it should be converted by to INTEGER again.  So omit the
      ** OP_RealAffinity opcode if it is present */
      sqlite3_Vdbe_Delete_प्राक्_Opcode(v, OP_RealAffinity);
    }
  }
  यदि( reg_बहिर्गमनम् ){
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, reg_आधारः, nCol, reg_बहिर्गमनम्);
  }
  sqlite3_Release_अस्थायी_Range(pParse, reg_आधारः, nCol);
  प्रत्यागम reg_आधारः;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
शून्य sqlite3ResolvePartIdxLabel(Parse *pParse, पूर्णाङ्क iLabel){
  यदि( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
