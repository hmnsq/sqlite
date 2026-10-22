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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

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
ਸਾਰਣੀ *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  Src_ਵਸਤੂ *p_ਵਸਤੂ = pSrc->a;
  ਸਾਰਣੀ *pTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ && pSrc->nSrc>=1 );
  pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, p_ਵਸਤੂ);
  ਜੇ( p_ਵਸਤੂ->pSTab ) sqlite3_Delete_ਸਾਰਣੀ(pParse->db, p_ਵਸਤੂ->pSTab);
  p_ਵਸਤੂ->pSTab = pTab;
  p_ਵਸਤੂ->fg.notCte = 1;
  ਜੇ( pTab ){
    pTab->nTabRef++;
    ਜੇ( p_ਵਸਤੂ->fg.is_ਸੂਚਕਾਂਕਿਤ_By && sqlite3_ਸੂਚਕਾਂਕਿਤ_By_Lookup(pParse, p_ਵਸਤੂ) ){
      pTab = 0;
    }
  }
  ਵਾਪਸ pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
ਖਾਲੀ sqlite3_ਕੋਡ_Change_ਗਿਣਤੀ(Vdbe *v, ਪੂਰਨ regCounter, ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ_Name){
  sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_FkCheck);
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, regCounter, 1);
  sqlite3_Vdbe_ਸੈੱਟ_ਅੰਕ_ਖਾਨੇ_ਗਿਣਤੀ(v, 1);
  sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, 0, COLNAME_NAME, z_ਖਾਨਾ_Name, SQLITE_STATIC);
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
ਸਥਿਰ ਪੂਰਨ vtabIsReadOnly(Parse *pParse, ਸਾਰਣੀ *pTab){
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  ਜੇ( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    ਵਾਪਸ 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  ਜੇ( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  ਵਾਪਸ 0;
}
ਸਥਿਰ ਪੂਰਨ tabIsReadOnly(Parse *pParse, ਸਾਰਣੀ *pTab){
  sqlite3 *db;
  ਜੇ( IsVirtual(pTab) ){
    ਵਾਪਸ vtabIsReadOnly(pParse, pTab);
  }
  ਜੇ( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) ਵਾਪਸ 0;
  db = pParse->db;
  ਜੇ( (pTab->tabFlags & TF_Readonly)!=0 ){
    ਵਾਪਸ sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->tabFlags & TF_Shadow );
  ਵਾਪਸ sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
ਪੂਰਨ sqlite3IsReadOnly(Parse *pParse, ਸਾਰਣੀ *pTab, Trigger *pTrigger){
  ਜੇ( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "table %s may not be modified", pTab->zName);
    ਵਾਪਸ 1;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
  ਜੇ( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->p_ਅਗਲਾ==0))
  ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse,"cannot modify %s because it is a view",pTab->zName);
    ਵਾਪਸ 1;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 0;
}


#ਜੇ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
ਖਾਲੀ sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  ਸਾਰਣੀ *pView,        /* View definition */
  ਵਿਅੰਜਕ *pWhere,        /* Optional WHERE clause to be added */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,  /* Optional ORDER BY clause */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ,        /* Optional LIMIT clause */
  ਪੂਰਨ iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pView->pSchema);
  pWhere = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  ਜੇ( pFrom ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->a[0].fg.isUsing==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, p_ਕ੍ਰਮ_By,
                          SF_IncludeHidden, p_ਹੱਦਬੰਦੀ);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#ਜੇ defined(SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
ਵਿਅੰਜਕ *sqlite3_ਹੱਦਬੰਦੀ_Where(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  ਵਿਅੰਜਕ *pWhere,                /* The WHERE clause.  May be null */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,          /* The ORDER BY clause.  May be null */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ,                /* The LIMIT clause.  May be null */
  ਅੱਖਰ *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ *pLhs = ਨਿਰਾ;           /* LHS of IN(SELECT...) operator */
  ਵਿਅੰਜਕ *p_ਵਿੱਚ_Clause = ਨਿਰਾ;      /* WHERE rowid IN ( select ) */
  ਵਿਅੰਜਕ_List *pEList = ਨਿਰਾ;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = ਨਿਰਾ;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = ਨਿਰਾ;      /* Complete SELECT tree */
  ਸਾਰਣੀ *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  ਜੇ( p_ਕ੍ਰਮ_By && p_ਹੱਦਬੰਦੀ==0 ) {
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, pWhere);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(pParse->db, p_ਕ੍ਰਮ_By);
    ਵਾਪਸ 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  ਜੇ( p_ਹੱਦਬੰਦੀ == 0 ) {
    ਵਾਪਸ pWhere;
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
  ਜੇ( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_ਪੰਕਤੀ, 0, 0);
    pEList = sqlite3_ਵਿਅੰਜਕ_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_ਪੰਕਤੀ, 0, 0)
    );
  }ਨਹੀਂਤਾਂ{
    ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
    ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pPk->n_ਕੁੰਜੀ_ਖਾਨਾ>=1 );
    ਜੇ( pPk->n_ਕੁੰਜੀ_ਖਾਨਾ==1 ){
      ਅਚਲ ਅੱਖਰ *zName;
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk->ai_ਕਾਲਮ[0]>=0 && pPk->ai_ਕਾਲਮ[0]<pTab->n_ਖਾਨਾ );
      zName = pTab->a_ਖਾਨਾ[pPk->ai_ਕਾਲਮ[0]].zCnName;
      pLhs = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, zName);
      pEList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, sqlite3_ਵਿਅੰਜਕ(db, TK_ID, zName));
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ i;
      ਲਈ(i=0; i<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
        ਵਿਅੰਜਕ *p;
        ਪ੍ਰਤਿਜ੍ਞਾ( pPk->ai_ਕਾਲਮ[i]>=0 && pPk->ai_ਕਾਲਮ[i]<pTab->n_ਖਾਨਾ );
        p = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, pTab->a_ਖਾਨਾ[pPk->ai_ਕਾਲਮ[i]].zCnName);
        pEList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_ਸਦਿਸ਼, 0, 0);
      ਜੇ( pLhs ){
        pLhs->x.pList = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  ਜੇ( pSrc->a[0].fg.is_ਸੂਚਕਾਂਕਿਤ_By ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.is_ਸੂਚਕਾਂਕਿਤ_By = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.z_ਸੂਚਕਾਂਕਿਤ_By);
  }ਨਹੀਂਤਾਂ ਜੇ( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      p_ਕ੍ਰਮ_By,0,p_ਹੱਦਬੰਦੀ
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  p_ਵਿੱਚ_Clause = sqlite3PExpr(pParse, TK_ਵਿੱਚ, pLhs, 0);
  sqlite3_PExpr_ਜੋੜੋ_Select(pParse, p_ਵਿੱਚ_Clause, pSelect);
  ਵਾਪਸ p_ਵਿੱਚ_Clause;
}
#ਜੇ_ਅੰਤ /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
ਖਾਲੀ sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  ਵਿਅੰਜਕ *pWhere,          /* The WHERE clause.  May be null */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,    /* ORDER BY clause. May be null */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  ਸਾਰਣੀ *pTab;           /* The table from which records will be deleted */
  ਪੂਰਨ i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  ਸੂਚਕਾਂਕ *pIdx;           /* For looping over indices of the table */
  ਪੂਰਨ iTabCur;           /* Cursor number for the table */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur = 0;      /* VDBE cursor for the canonical data source */
  ਪੂਰਨ iIdxCur = 0;       /* Cursor number of the first index */
  ਪੂਰਨ nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  ਪੂਰਨ iDb;               /* Database number */
  ਪੂਰਨ memCnt = 0;        /* Memory cell used for change counting */
  ਪੂਰਨ rcauth;            /* Value returned by authorization callback */
  ਪੂਰਨ eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  ਪੂਰਨ aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *a_ਮੰਜ਼ਿਲ_Open = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  ਸੂਚਕਾਂਕ *pPk;            /* The PRIMARY KEY index on the table */
  ਪੂਰਨ iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  ਪੂਰਨ i_ਕੁੰਜੀ;              /* Memory cell holding key of row to be deleted */
  i16 n_ਕੁੰਜੀ;              /* Number of memory cells in the row key */
  ਪੂਰਨ iEphCur = 0;       /* Ephemeral table holding all primary key values */
  ਪੂਰਨ i_ਪੰਕਤੀ_ਸੈੱਟ = 0;       /* Register for rowset of rows to delete */
  ਪੂਰਨ addrBypass = 0;    /* Address of jump over the delete logic */
  ਪੂਰਨ addrLoop = 0;      /* Top of the delete loop */
  ਪੂਰਨ addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  ਪੂਰਨ bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਪੂਰਨ isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#ਜੇ_ਅੰਤ

  memset(&sContext, 0, ਆਕਾਰ(sContext));
  db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
  ਜੇ( pParse->nErr ){
    ਜਾਓ delete_from_cleanup;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ਜੇ( pTab==0 )  ਜਾਓ delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#ਨਹੀਂਤਾਂ
# define pTrigger 0
# define isView 0
#ਜੇ_ਅੰਤ
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#ਜੇ_ਅੰਤ

#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x10000 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_ਰੁੱਖ_View_Delete(pParse->pWith, pTabList, pWhere,
                          p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ, pTrigger);
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ
  ਜੇ( !isView ){
    pWhere = sqlite3_ਹੱਦਬੰਦੀ_Where(
        pParse, pTabList, pWhere, p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ, "DELETE"
    );
    p_ਕ੍ਰਮ_By = 0;
    p_ਹੱਦਬੰਦੀ = 0;
  }
#ਜੇ_ਅੰਤ

  /* If pTab is really a view, make sure it has been initialized.
  */
  ਜੇ( sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab) ){
    ਜਾਓ delete_from_cleanup;
  }

  ਜੇ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ਜਾਓ delete_from_cleanup;
  }
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  ਪ੍ਰਤਿਜ੍ਞਾ( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  ਜੇ( rcauth==SQLITE_DENY ){
    ਜਾਓ delete_from_cleanup;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  ਲਈ(nIdx=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  ਜੇ( isView ){
    sqlite3_Auth_Context_ਧੱਕੋ(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  ਜੇ( v==0 ){
    ਜਾਓ delete_from_cleanup;
  }
  ਜੇ( pParse->nested==0 ) sqlite3_Vdbe_ਗਿਣਤੀ_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#ਜੇ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ਜੇ( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ, iTabCur
    );
    i_ਅੰਕੜੇ_Cur = iIdxCur = iTabCur;
    p_ਕ੍ਰਮ_By = 0;
    p_ਹੱਦਬੰਦੀ = 0;
  }
#ਜੇ_ਅੰਤ

  /* Resolve the column names in the WHERE clause.
  */
  memset(&sNC, 0, ਆਕਾਰ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pWhere) ){
    ਜਾਓ delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  ਜੇ( (db->flags & SQLITE_ਗਿਣਤੀ_ਪੰਕਤੀਆਂ)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, memCnt);
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRUNCATE_OPTIMIZATION
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
  ਜੇ( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#ਜੇ_ਅੰਤ
  ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !isView );
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, 1, pTab->zName);
    ਜੇ( HasRowid(pTab) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->pSchema==pTab->pSchema );
      ਜੇ( Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) && !HasRowid(pTab) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    ਜੇ( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    ਜੇ( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( nPk==1 );
      i_ਪੰਕਤੀ_ਸੈੱਟ = ++pParse->nMem;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, i_ਪੰਕਤੀ_ਸੈੱਟ);
    }ਨਹੀਂਤਾਂ{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 );
      nPk = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pPk);
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
    ਜੇ( pWInfo==0 ) ਜਾਓ delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    ਜੇ( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    ਜੇ( sqlite3_Where_ਵਰਤਦਾ_Deferred_Seek(pWInfo) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ਅੰਤ_Seek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    ਜੇ( memCnt ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    ਜੇ( pPk ){
      ਲਈ(i=0; i<nPk; i++){
        ਪ੍ਰਤਿਜ੍ਞਾ( pPk->ai_ਕਾਲਮ[i]>=0 );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iTabCur,
                                        pPk->ai_ਕਾਲਮ[i], iPk+i);
      }
      i_ਕੁੰਜੀ = iPk;
    }ਨਹੀਂਤਾਂ{
      i_ਕੁੰਜੀ = ++pParse->nMem;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iTabCur, -1, i_ਕੁੰਜੀ);
    }
 
    ਜੇ( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_ਕੁੰਜੀ = nPk; /* OP_Found will use an unpacked key */
      a_ਮੰਜ਼ਿਲ_Open = sqlite3DbMallocRawNN(db, nIdx+2);
      ਜੇ( a_ਮੰਜ਼ਿਲ_Open==0 ){
        sqlite3_Where_ਸਿਰਾ(pWInfo);
        ਜਾਓ delete_from_cleanup;
      }
      memset(a_ਮੰਜ਼ਿਲ_Open, 1, nIdx+1);
      a_ਮੰਜ਼ਿਲ_Open[nIdx+1] = 0;
      ਜੇ( aiCurOnePass[0]>=0 ) a_ਮੰਜ਼ਿਲ_Open[aiCurOnePass[0]-iTabCur] = 0;
      ਜੇ( aiCurOnePass[1]>=0 ) a_ਮੰਜ਼ਿਲ_Open[aiCurOnePass[1]-iTabCur] = 0;
      ਜੇ( addrEphOpen ) sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }ਨਹੀਂਤਾਂ{
      ਜੇ( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_ਕੁੰਜੀ = ++pParse->nMem;
        n_ਕੁੰਜੀ = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, iPk, nPk, i_ਕੁੰਜੀ,
            sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(pParse->db, pPk), nPk);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iEphCur, i_ਕੁੰਜੀ, iPk, nPk);
      }ਨਹੀਂਤਾਂ{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_ਕੁੰਜੀ = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਪੰਕਤੀ_ਸੈੱਟ_ਜੋੜੋ, i_ਪੰਕਤੀ_ਸੈੱਟ, i_ਕੁੰਜੀ);
      }
      sqlite3_Where_ਸਿਰਾ(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    ਜੇ( !isView ){
      ਪੂਰਨ iAddrOnce = 0;
      ਜੇ( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3_Open_ਸਾਰਣੀ_And_Indices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, a_ਮੰਜ਼ਿਲ_Open, &i_ਅੰਕੜੇ_Cur, &iIdxCur);
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk || IsVirtual(pTab) || i_ਅੰਕੜੇ_Cur==iTabCur );
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk || IsVirtual(pTab) || iIdxCur==i_ਅੰਕੜੇ_Cur+1 );
      ਜੇ( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_ਇੱਥੇ_Or_ਕੱਢੋ_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    ਜੇ( eOnePass!=ONEPASS_OFF ){
      ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ==nPk );  /* OP_Found will use an unpacked key */
      ਜੇ( !IsVirtual(pTab) && a_ਮੰਜ਼ਿਲ_Open[i_ਅੰਕੜੇ_Cur-iTabCur] ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 || IsView(pTab) );
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ, i_ਅੰਕੜੇ_Cur, addrBypass, i_ਕੁੰਜੀ, n_ਕੁੰਜੀ);
        VdbeCoverage(v);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( pPk ){
      addrLoop = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      ਜੇ( IsVirtual(pTab) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iEphCur, 0, i_ਕੁੰਜੀ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਪੰਕਤੀ_ਅੰਕੜੇ, iEphCur, i_ਕੁੰਜੀ);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ==0 );  /* OP_Found will use a composite key */
    }ਨਹੀਂਤਾਂ{
      addrLoop = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪੰਕਤੀ_ਸੈੱਟ_Read, i_ਪੰਕਤੀ_ਸੈੱਟ, 0, i_ਕੁੰਜੀ);
      VdbeCoverage(v);
      ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ==1 );
    } 
 
    /* Delete the row */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਜੇ( IsVirtual(pTab) ){
      ਅਚਲ ਅੱਖਰ *pVTab = (ਅਚਲ ਅੱਖਰ *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      ਪ੍ਰਤਿਜ੍ਞਾ( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      ਜੇ( eOnePass==ONEPASS_SINGLE ){
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Close, iTabCur);
        ਜੇ( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_VUpdate, 0, 1, i_ਕੁੰਜੀ, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਪੂਰਨ ਗਿਣਤੀ_2 = (pParse->nested==0);    /* True to count changes */
      sqlite3_Generate_ਪੰਕਤੀ_Delete(pParse, pTab, pTrigger, i_ਅੰਕੜੇ_Cur, iIdxCur,
          i_ਕੁੰਜੀ, n_ਕੁੰਜੀ, ਗਿਣਤੀ_2, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    ਜੇ( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3_Where_ਸਿਰਾ(pWInfo);
    }ਨਹੀਂਤਾਂ ਜੇ( pPk ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrLoop);
    }ਨਹੀਂਤਾਂ{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ਜੇ( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_ਸਿਰਾ(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ਜੇ( memCnt ){
    sqlite3_ਕੋਡ_Change_ਗਿਣਤੀ(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_ਕੱਢੋ(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
#ਜੇ defined(SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ)
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਕ੍ਰਮ_By);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਹੱਦਬੰਦੀ);
#ਜੇ_ਅੰਤ
  ਜੇ( a_ਮੰਜ਼ਿਲ_Open ) sqlite3DbNNFreeNN(db, a_ਮੰਜ਼ਿਲ_Open);
  ਵਾਪਸ;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ isView
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ isView
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ pTrigger
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ pTrigger
#ਜੇ_ਅੰਤ

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
ਖਾਲੀ sqlite3_Generate_ਪੰਕਤੀ_Delete(
  Parse *pParse,     /* Parsing context */
  ਸਾਰਣੀ *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur,      /* Cursor from which column data is extracted */
  ਪੂਰਨ iIdxCur,       /* First index cursor */
  ਪੂਰਨ iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 ਗਿਣਤੀ_2,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 e_ਬਹੁਲਕ,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  ਪੂਰਨ iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  ਪੂਰਨ iOld = 0;                   /* First register in OLD.* array */
  ਪੂਰਨ iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  ਪ੍ਰਤਿਜ੍ਞਾ( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         i_ਅੰਕੜੇ_Cur, iIdxCur, iPk, (ਪੂਰਨ)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_Not_ਮਿਲਿਆ;
  ਜੇ( e_ਬਹੁਲਕ==ONEPASS_OFF ){
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, opSeek, i_ਅੰਕੜੇ_Cur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_Not_ਮਿਲਿਆ);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  ਜੇ( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 ਨਕਾਬ;                     /* Mask of OLD.* columns in use */
    ਪੂਰਨ i_ਖਾਨਾ;                     /* Iterator used while populating OLD.* */
    ਪੂਰਨ addr_ਸ਼ੁਰੂ;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    ਨਕਾਬ = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    ਨਕਾਬ |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->n_ਖਾਨਾ);

    /* Populate the OLD.* pseudo-table register array. These values will be
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, iPk, iOld);
    ਲਈ(i_ਖਾਨਾ=0; i_ਖਾਨਾ<pTab->n_ਖਾਨਾ; i_ਖਾਨਾ++){
      testcase( ਨਕਾਬ!=0xffffffff && i_ਖਾਨਾ==31 );
      testcase( ਨਕਾਬ!=0xffffffff && i_ਖਾਨਾ==32 );
      ਜੇ( ਨਕਾਬ==0xffffffff || (i_ਖਾਨਾ<=31 && (ਨਕਾਬ & MASKBIT32(i_ਖਾਨਾ))!=0) ){
        ਪੂਰਨ kk = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖਾਨਾ);
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur, i_ਖਾਨਾ, iOld+kk+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addr_ਸ਼ੁਰੂ = sqlite3VdbeCurrentAddr(v);
    sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger,
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
    ਜੇ( addr_ਸ਼ੁਰੂ<sqlite3VdbeCurrentAddr(v) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, opSeek, i_ਅੰਕੜੇ_Cur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_Not_ਮਿਲਿਆ);
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
  ਜੇ( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3_Generate_ਪੰਕਤੀ_ਸੂਚਕਾਂਕ_Delete(pParse, pTab, i_ਅੰਕੜੇ_Cur, iIdxCur,0,iIdxNoSeek);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Delete, i_ਅੰਕੜੇ_Cur, (ਗਿਣਤੀ_2?OPFLAG_NCHANGE:0));
    ਜੇ( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (ਅੱਖਰ*)pTab, P4_ਸਾਰਣੀ);
    }
    ਜੇ( e_ਬਹੁਲਕ!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    ਜੇ( iIdxNoSeek>=0 && iIdxNoSeek!=i_ਅੰਕੜੇ_Cur ){
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Delete, iIdxNoSeek);
    }
    ਜੇ( e_ਬਹੁਲਕ==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  ਜੇ( pTrigger ){
    sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger,
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
ਖਾਲੀ sqlite3_Generate_ਪੰਕਤੀ_ਸੂਚਕਾਂਕ_Delete(
  Parse *pParse,     /* Parsing and code generating context */
  ਸਾਰਣੀ *pTab,       /* Table containing the row to be deleted */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur,      /* Cursor of table holding data. */
  ਪੂਰਨ iIdxCur,       /* First index cursor */
  ਪੂਰਨ *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  ਪੂਰਨ iIdxNoSeek     /* Do not delete from this cursor */
){
  ਪੂਰਨ i;             /* Index loop counter */
  ਪੂਰਨ r1 = -1;       /* Register holding an index key */
  ਪੂਰਨ iPartIdxLabel; /* Jump destination for skipping partial index entries */
  ਸੂਚਕਾਂਕ *pIdx;       /* Current index */
  ਸੂਚਕਾਂਕ *p_ਪੂਰਵ = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  ਸੂਚਕਾਂਕ *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
  ਲਈ(i=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; i++, pIdx=pIdx->p_ਅਗਲਾ){
    ਪੂਰਨ p3 = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( iIdxCur+i!=i_ਅੰਕੜੇ_Cur || pPk==pIdx );
    ਜੇ( aRegIdx!=0 && aRegIdx[i]==0 ) ਜਾਰੀ;
    ਜੇ( pIdx==pPk ) ਜਾਰੀ;
    ਜੇ( iIdxCur+i==iIdxNoSeek ) ਜਾਰੀ;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(pParse, pIdx, i_ਅੰਕੜੇ_Cur, 0, 1,
        &iPartIdxLabel, p_ਪੂਰਵ, r1);
    ਜੇ( pIdx->b_Has_ਵਿਅੰਜਕ && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (ਅਚਲ ਅੱਖਰ*)pIdx, P4_ਸੂਚਕਾਂਕ);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ : pIdx->n_ਕਾਲਮ);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    p_ਪੂਰਵ = pIdx;
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
ਪੂਰਨ sqlite3_Generate_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(
  Parse *pParse,       /* Parsing context */
  ਸੂਚਕਾਂਕ *pIdx,         /* The index for which to generate a key */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur,        /* Cursor number from which to take column data */
  ਪੂਰਨ reg_ਬਾਹਰਲਾ,          /* Put the new key into this register if not 0 */
  ਪੂਰਨ ਅਗੇਤਰ_Only,      /* Compute only a unique prefix of the key */
  ਪੂਰਨ *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  ਸੂਚਕਾਂਕ *p_ਪੂਰਵ,       /* Previously generated index key */
  ਪੂਰਨ reg_ਪੂਰਵ         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ j;
  ਪੂਰਨ reg_ਅਧਾਰ;
  ਪੂਰਨ n_ਖਾਨਾ;

  ਜੇ( piPartIdxLabel ){
    ਜੇ( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = i_ਅੰਕੜੇ_Cur + 1;
      sqlite3_ਵਿਅੰਜਕ_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      p_ਪੂਰਵ = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }ਨਹੀਂਤਾਂ{
      *piPartIdxLabel = 0;
    }
  }
  n_ਖਾਨਾ = (ਅਗੇਤਰ_Only && pIdx->uniqNotNull) ? pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ : pIdx->n_ਕਾਲਮ;
  reg_ਅਧਾਰ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਖਾਨਾ);
  ਜੇ( p_ਪੂਰਵ && (reg_ਅਧਾਰ!=reg_ਪੂਰਵ || p_ਪੂਰਵ->pPartIdxWhere) ) p_ਪੂਰਵ = 0;
  ਲਈ(j=0; j<n_ਖਾਨਾ; j++){
    ਪੂਰਨ i_Tab_ਖਾਨਾ;
    ਜੇ( p_ਪੂਰਵ
     && p_ਪੂਰਵ->ai_ਕਾਲਮ[j]==pIdx->ai_ਕਾਲਮ[j]
     && p_ਪੂਰਵ->ai_ਕਾਲਮ[j]!=XN_ਵਿਅੰਜਕ
    ){
      /* This column was already computed by the previous index */
      ਜਾਰੀ;
    }
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੱਦੋ_ਸੂਚਕਾਂਕ_ਕਾਲਮ(pParse, pIdx, i_ਅੰਕੜੇ_Cur, j, reg_ਅਧਾਰ+j);
    ਜੇ( (i_Tab_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[j])>=0 
     && (pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[i_Tab_ਖਾਨਾ].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)==0
    ){
      /* If the column affinity is REAL but the number is an integer, then it
      ** might be stored in the table as an integer (using a compact
      ** representation) then converted to REAL by an OP_RealAffinity opcode.
      ** But we are getting ready to store this value back into an index, where
      ** it should be converted by to INTEGER again.  So omit the
      ** OP_RealAffinity opcode if it is present */
      sqlite3_Vdbe_Delete_ਪੂਰਵ_Opcode(v, OP_RealAffinity);
    }
  }
  ਜੇ( reg_ਬਾਹਰਲਾ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਅਧਾਰ, n_ਖਾਨਾ, reg_ਬਾਹਰਲਾ);
  }
  sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਅਧਾਰ, n_ਖਾਨਾ);
  ਵਾਪਸ reg_ਅਧਾਰ;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
ਖਾਲੀ sqlite3ResolvePartIdxLabel(Parse *pParse, ਪੂਰਨ iLabel){
  ਜੇ( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
