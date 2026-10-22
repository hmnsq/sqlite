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
#ضمن "sqliteInt.h"

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
جدول *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  Src_توکی *p_توکی = pSrc->a;
  جدول *pTab;
  أكد( p_توکی && pSrc->nSrc>=1 );
  pTab = sqlite3_Locate_جدول_توکی(pParse, 0, p_توکی);
  إذا( p_توکی->pSTab ) sqlite3_Delete_جدول(pParse->db, p_توکی->pSTab);
  p_توکی->pSTab = pTab;
  p_توکی->fg.notCte = 1;
  إذا( pTab ){
    pTab->nTabRef++;
    إذا( p_توکی->fg.is_شاخصي_By && sqlite3_شاخصي_By_Lookup(pParse, p_توکی) ){
      pTab = 0;
    }
  }
  أرجع pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
فراغ sqlite3_کوډ_Change_شمېر(Vdbe *v, صحيح regCounter, ثابت حرف *z_ستون_Name){
  sqlite3_Vdbe_جمع_کړه_Op0(v, OP_FkCheck);
  sqlite3_Vdbe_جمع_کړه_Op2(v, OP_پایله_لیکه, regCounter, 1);
  sqlite3_Vdbe_سېټ_عدد_ستون_شمېر(v, 1);
  sqlite3_Vdbe_سېټ_ستون_Name(v, 0, COLNAME_NAME, z_ستون_Name, SQLITE_STATIC);
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
ساكن صحيح vtabIsReadOnly(Parse *pParse, جدول *pTab){
  أكد( IsVirtual(pTab) );
  إذا( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    أرجع 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  إذا( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_پیغام(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  أرجع 0;
}
ساكن صحيح tabIsReadOnly(Parse *pParse, جدول *pTab){
  sqlite3 *db;
  إذا( IsVirtual(pTab) ){
    أرجع vtabIsReadOnly(pParse, pTab);
  }
  إذا( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) أرجع 0;
  db = pParse->db;
  إذا( (pTab->tabFlags & TF_Readonly)!=0 ){
    أرجع sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  أكد( pTab->tabFlags & TF_Shadow );
  أرجع sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
صحيح sqlite3IsReadOnly(Parse *pParse, جدول *pTab, Trigger *pTrigger){
  إذا( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_پیغام(pParse, "table %s may not be modified", pTab->zName);
    أرجع 1;
  }
#که_نه_تعریف_شوی SQLITE_OMIT_VIEW
  إذا( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->p_بل==0))
  ){
    sqlite3_Error_پیغام(pParse,"cannot modify %s because it is a view",pTab->zName);
    أرجع 1;
  }
#د_که_پای
  أرجع 0;
}


#إذا !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
فراغ sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  جدول *pView,        /* View definition */
  بیان *pWhere,        /* Optional WHERE clause to be added */
  بیان_List *p_ترتیب_By,  /* Optional ORDER BY clause */
  بیان *p_حد,        /* Optional LIMIT clause */
  صحيح iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  صحيح iDb = sqlite3_Schema_منزل_شاخص(db, pView->pSchema);
  pWhere = sqlite3_بیان_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  إذا( pFrom ){
    أكد( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    أكد( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    أكد( pFrom->a[0].fg.isUsing==0 );
    أكد( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, p_ترتیب_By,
                          SF_IncludeHidden, p_حد);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#د_که_پای /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#إذا defined(SQLITE_ENABLE_UPDATE_DELETE_حد) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
بیان *sqlite3_حد_Where(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  بیان *pWhere,                /* The WHERE clause.  May be null */
  بیان_List *p_ترتیب_By,          /* The ORDER BY clause.  May be null */
  بیان *p_حد,                /* The LIMIT clause.  May be null */
  حرف *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  بیان *pLhs = عدم;           /* LHS of IN(SELECT...) operator */
  بیان *p_دننه_Clause = عدم;      /* WHERE rowid IN ( select ) */
  بیان_List *pEList = عدم;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = عدم;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = عدم;      /* Complete SELECT tree */
  جدول *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  إذا( p_ترتیب_By && p_حد==0 ) {
    sqlite3_Error_پیغام(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_بیان_Delete(pParse->db, pWhere);
    sqlite3_بیان_List_Delete(pParse->db, p_ترتیب_By);
    أرجع 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  إذا( p_حد == 0 ) {
    أرجع pWhere;
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
  إذا( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_لیکه, 0, 0);
    pEList = sqlite3_بیان_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_لیکه, 0, 0)
    );
  }وإلا{
    شاخص *pPk = sqlite3_Primary_کلی_شاخص(pTab);
    أكد( pPk!=0 );
    أكد( pPk->n_کلی_ستون>=1 );
    إذا( pPk->n_کلی_ستون==1 ){
      ثابت حرف *zName;
      أكد( pPk->ai_ستنه[0]>=0 && pPk->ai_ستنه[0]<pTab->n_ستون );
      zName = pTab->a_ستون[pPk->ai_ستنه[0]].zCnName;
      pLhs = sqlite3_بیان(db, TK_ID, zName);
      pEList = sqlite3_بیان_List_Append(pParse, 0, sqlite3_بیان(db, TK_ID, zName));
    }وإلا{
      صحيح i;
      لكل(i=0; i<pPk->n_کلی_ستون; i++){
        بیان *p;
        أكد( pPk->ai_ستنه[i]>=0 && pPk->ai_ستنه[i]<pTab->n_ستون );
        p = sqlite3_بیان(db, TK_ID, pTab->a_ستون[pPk->ai_ستنه[i]].zCnName);
        pEList = sqlite3_بیان_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_ویکټور, 0, 0);
      إذا( pLhs ){
        pLhs->x.pList = sqlite3_بیان_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  إذا( pSrc->a[0].fg.is_شاخصي_By ){
    أكد( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.is_شاخصي_By = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.z_شاخصي_By);
  }وإلا إذا( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      p_ترتیب_By,0,p_حد
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  p_دننه_Clause = sqlite3PExpr(pParse, TK_دننه, pLhs, 0);
  sqlite3_PExpr_جمع_کړه_Select(pParse, p_دننه_Clause, pSelect);
  أرجع p_دننه_Clause;
}
#د_که_پای /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
فراغ sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  بیان *pWhere,          /* The WHERE clause.  May be null */
  بیان_List *p_ترتیب_By,    /* ORDER BY clause. May be null */
  بیان *p_حد           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  جدول *pTab;           /* The table from which records will be deleted */
  صحيح i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  شاخص *pIdx;           /* For looping over indices of the table */
  صحيح iTabCur;           /* Cursor number for the table */
  صحيح i_معلومات_Cur = 0;      /* VDBE cursor for the canonical data source */
  صحيح iIdxCur = 0;       /* Cursor number of the first index */
  صحيح nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  صحيح iDb;               /* Database number */
  صحيح memCnt = 0;        /* Memory cell used for change counting */
  صحيح rcauth;            /* Value returned by authorization callback */
  صحيح eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  صحيح aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *a_منزل_Open = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  شاخص *pPk;            /* The PRIMARY KEY index on the table */
  صحيح iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  صحيح i_کلی;              /* Memory cell holding key of row to be deleted */
  i16 n_کلی;              /* Number of memory cells in the row key */
  صحيح iEphCur = 0;       /* Ephemeral table holding all primary key values */
  صحيح i_لیکه_سېټ = 0;       /* Register for rowset of rows to delete */
  صحيح addrBypass = 0;    /* Address of jump over the delete logic */
  صحيح addrLoop = 0;      /* Top of the delete loop */
  صحيح addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  صحيح bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
  صحيح isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#د_که_پای

  memset(&sContext, 0, حجم(sContext));
  db = pParse->db;
  أكد( db->pParse==pParse );
  إذا( pParse->nErr ){
    اذهب delete_from_cleanup;
  }
  أكد( db->mallocFailed==0 );
  أكد( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  إذا( pTab==0 )  اذهب delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#وإلا
# define pTrigger 0
# define isView 0
#د_که_پای
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#که_تعریف_شوی SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#د_که_پای

#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x10000 ){
    sqlite3_ونه_View_کرښه(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_ونه_View_Delete(pParse->pWith, pTabList, pWhere,
                          p_ترتیب_By, p_حد, pTrigger);
  }
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_UPDATE_DELETE_حد
  إذا( !isView ){
    pWhere = sqlite3_حد_Where(
        pParse, pTabList, pWhere, p_ترتیب_By, p_حد, "DELETE"
    );
    p_ترتیب_By = 0;
    p_حد = 0;
  }
#د_که_پای

  /* If pTab is really a view, make sure it has been initialized.
  */
  إذا( sqlite3_View_Get_ستنه_نومونه(pParse, pTab) ){
    اذهب delete_from_cleanup;
  }

  إذا( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    اذهب delete_from_cleanup;
  }
  iDb = sqlite3_Schema_منزل_شاخص(db, pTab->pSchema);
  أكد( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  أكد( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  إذا( rcauth==SQLITE_DENY ){
    اذهب delete_from_cleanup;
  }
  أكد(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  أكد( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  لكل(nIdx=0, pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  إذا( isView ){
    sqlite3_Auth_Context_ټېل(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  إذا( v==0 ){
    اذهب delete_from_cleanup;
  }
  إذا( pParse->nested==0 ) sqlite3_Vdbe_شمېر_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#إذا !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  إذا( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ترتیب_By, p_حد, iTabCur
    );
    i_معلومات_Cur = iIdxCur = iTabCur;
    p_ترتیب_By = 0;
    p_حد = 0;
  }
#د_که_پای

  /* Resolve the column names in the WHERE clause.
  */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  إذا( sqlite3_Resolve_بیان_نومونه(&sNC, pWhere) ){
    اذهب delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  إذا( (db->flags & SQLITE_شمېر_لیکې)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, memCnt);
  }

#که_نه_تعریف_شوی SQLITE_OMIT_TRUNCATE_OPTIMIZATION
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
  إذا( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#د_که_پای
  ){
    أكد( !isView );
    sqlite3_جدول_قلف(pParse, iDb, pTab->tnum, 1, pTab->zName);
    إذا( HasRowid(pTab) ){
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
      أكد( pIdx->pSchema==pTab->pSchema );
      إذا( Is_Primary_کلی_شاخص(pIdx) && !HasRowid(pTab) ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }وإلا{
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }وإلا
#د_که_پای /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    إذا( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    إذا( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      أكد( nPk==1 );
      i_لیکه_سېټ = ++pParse->nMem;
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Null, 0, i_لیکه_سېټ);
    }وإلا{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_کلی_شاخص(pTab);
      أكد( pPk!=0 );
      nPk = pPk->n_کلی_ستون;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pPk);
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
    إذا( pWInfo==0 ) اذهب delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    أكد( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    أكد( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    إذا( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    إذا( sqlite3_Where_کاروي_Deferred_Seek(pWInfo) ){
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_پای_Seek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    إذا( memCnt ){
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_جمع_کړه_Imm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    إذا( pPk ){
      لكل(i=0; i<nPk; i++){
        أكد( pPk->ai_ستنه[i]>=0 );
        sqlite3_بیان_کوډ_Get_ستنه_د_جدول(v, pTab, iTabCur,
                                        pPk->ai_ستنه[i], iPk+i);
      }
      i_کلی = iPk;
    }وإلا{
      i_کلی = ++pParse->nMem;
      sqlite3_بیان_کوډ_Get_ستنه_د_جدول(v, pTab, iTabCur, -1, i_کلی);
    }
 
    إذا( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_کلی = nPk; /* OP_Found will use an unpacked key */
      a_منزل_Open = sqlite3DbMallocRawNN(db, nIdx+2);
      إذا( a_منزل_Open==0 ){
        sqlite3_Where_پای_ټکی(pWInfo);
        اذهب delete_from_cleanup;
      }
      memset(a_منزل_Open, 1, nIdx+1);
      a_منزل_Open[nIdx+1] = 0;
      إذا( aiCurOnePass[0]>=0 ) a_منزل_Open[aiCurOnePass[0]-iTabCur] = 0;
      إذا( aiCurOnePass[1]>=0 ) a_منزل_Open[aiCurOnePass[1]-iTabCur] = 0;
      إذا( addrEphOpen ) sqlite3_Vdbe_Change_منزل_Noop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }وإلا{
      إذا( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_کلی = ++pParse->nMem;
        n_کلی = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Make_ریکارډ, iPk, nPk, i_کلی,
            sqlite3_شاخص_Affinity_Str(pParse->db, pPk), nPk);
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iEphCur, i_کلی, iPk, nPk);
      }وإلا{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_کلی = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_لیکه_سېټ_جمع_کړه, i_لیکه_سېټ, i_کلی);
      }
      sqlite3_Where_پای_ټکی(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    إذا( !isView ){
      صحيح iAddrOnce = 0;
      إذا( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3_Open_جدول_And_Indices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, a_منزل_Open, &i_معلومات_Cur, &iIdxCur);
      أكد( pPk || IsVirtual(pTab) || i_معلومات_Cur==iTabCur );
      أكد( pPk || IsVirtual(pTab) || iIdxCur==i_معلومات_Cur+1 );
      إذا( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_دلته_Or_راوباسه_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    إذا( eOnePass!=ONEPASS_OFF ){
      أكد( n_کلی==nPk );  /* OP_Found will use an unpacked key */
      إذا( !IsVirtual(pTab) && a_منزل_Open[i_معلومات_Cur-iTabCur] ){
        أكد( pPk!=0 || IsView(pTab) );
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Not_وموندل, i_معلومات_Cur, addrBypass, i_کلی, n_کلی);
        VdbeCoverage(v);
      }
    }وإلا إذا( pPk ){
      addrLoop = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      إذا( IsVirtual(pTab) ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, iEphCur, 0, i_کلی);
      }وإلا{
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_لیکه_معلومات, iEphCur, i_کلی);
      }
      أكد( n_کلی==0 );  /* OP_Found will use a composite key */
    }وإلا{
      addrLoop = sqlite3_Vdbe_جمع_کړه_Op3(v, OP_لیکه_سېټ_Read, i_لیکه_سېټ, 0, i_کلی);
      VdbeCoverage(v);
      أكد( n_کلی==1 );
    } 
 
    /* Delete the row */
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
    إذا( IsVirtual(pTab) ){
      ثابت حرف *pVTab = (ثابت حرف *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      أكد( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      إذا( eOnePass==ONEPASS_SINGLE ){
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Close, iTabCur);
        إذا( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_VUpdate, 0, 1, i_کلی, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }وإلا
#د_که_پای
    {
      صحيح شمېر = (pParse->nested==0);    /* True to count changes */
      sqlite3_Generate_لیکه_Delete(pParse, pTab, pTrigger, i_معلومات_Cur, iIdxCur,
          i_کلی, n_کلی, شمېر, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    إذا( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3_Where_پای_ټکی(pWInfo);
    }وإلا إذا( pPk ){
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_دلته(v, addrLoop);
    }وإلا{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3_Vdbe_Jump_دلته(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  إذا( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_پای_ټکی(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  إذا( memCnt ){
    sqlite3_کوډ_Change_شمېر(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_راوباسه(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_بیان_Delete(db, pWhere);
#إذا defined(SQLITE_ENABLE_UPDATE_DELETE_حد)
  sqlite3_بیان_List_Delete(db, p_ترتیب_By);
  sqlite3_بیان_Delete(db, p_حد);
#د_که_پای
  إذا( a_منزل_Open ) sqlite3DbNNFreeNN(db, a_منزل_Open);
  أرجع;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#که_تعریف_شوی isView
 #تعریف_لغوه isView
#د_که_پای
#که_تعریف_شوی pTrigger
 #تعریف_لغوه pTrigger
#د_که_پای

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
فراغ sqlite3_Generate_لیکه_Delete(
  Parse *pParse,     /* Parsing context */
  جدول *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  صحيح i_معلومات_Cur,      /* Cursor from which column data is extracted */
  صحيح iIdxCur,       /* First index cursor */
  صحيح iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 شمېر,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 e_ډېرپېښ,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  صحيح iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  صحيح iOld = 0;                   /* First register in OLD.* array */
  صحيح iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  أكد( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         i_معلومات_Cur, iIdxCur, iPk, (صحيح)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_Not_وموندل;
  إذا( e_ډېرپېښ==ONEPASS_OFF ){
    sqlite3_Vdbe_جمع_کړه_Op4_Int(v, opSeek, i_معلومات_Cur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_Not_وموندل);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  إذا( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 ماسک;                     /* Mask of OLD.* columns in use */
    صحيح i_ستون;                     /* Iterator used while populating OLD.* */
    صحيح addr_پیل;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    ماسک = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    ماسک |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->n_ستون);

    /* Populate the OLD.* pseudo-table register array. These values will be
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Copy, iPk, iOld);
    لكل(i_ستون=0; i_ستون<pTab->n_ستون; i_ستون++){
      testcase( ماسک!=0xffffffff && i_ستون==31 );
      testcase( ماسک!=0xffffffff && i_ستون==32 );
      إذا( ماسک==0xffffffff || (i_ستون<=31 && (ماسک & MASKBIT32(i_ستون))!=0) ){
        صحيح kk = sqlite3_جدول_ستنه_منزل_Storage(pTab, i_ستون);
        sqlite3_بیان_کوډ_Get_ستنه_د_جدول(v, pTab, i_معلومات_Cur, i_ستون, iOld+kk+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addr_پیل = sqlite3VdbeCurrentAddr(v);
    sqlite3_کوډ_لیکه_Trigger(pParse, pTrigger,
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
    إذا( addr_پیل<sqlite3VdbeCurrentAddr(v) ){
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, opSeek, i_معلومات_Cur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_Not_وموندل);
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
  إذا( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3_Generate_لیکه_شاخص_Delete(pParse, pTab, i_معلومات_Cur, iIdxCur,0,iIdxNoSeek);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Delete, i_معلومات_Cur, (شمېر?OPFLAG_NCHANGE:0));
    إذا( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (حرف*)pTab, P4_جدول);
    }
    إذا( e_ډېرپېښ!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    إذا( iIdxNoSeek>=0 && iIdxNoSeek!=i_معلومات_Cur ){
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Delete, iIdxNoSeek);
    }
    إذا( e_ډېرپېښ==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  إذا( pTrigger ){
    sqlite3_کوډ_لیکه_Trigger(pParse, pTrigger,
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
فراغ sqlite3_Generate_لیکه_شاخص_Delete(
  Parse *pParse,     /* Parsing and code generating context */
  جدول *pTab,       /* Table containing the row to be deleted */
  صحيح i_معلومات_Cur,      /* Cursor of table holding data. */
  صحيح iIdxCur,       /* First index cursor */
  صحيح *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  صحيح iIdxNoSeek     /* Do not delete from this cursor */
){
  صحيح i;             /* Index loop counter */
  صحيح r1 = -1;       /* Register holding an index key */
  صحيح iPartIdxLabel; /* Jump destination for skipping partial index entries */
  شاخص *pIdx;       /* Current index */
  شاخص *p_مخکینی = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  شاخص *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_کلی_شاخص(pTab);
  لكل(i=0, pIdx=pTab->p_شاخص; pIdx; i++, pIdx=pIdx->p_بل){
    صحيح p3 = 0;
    أكد( iIdxCur+i!=i_معلومات_Cur || pPk==pIdx );
    إذا( aRegIdx!=0 && aRegIdx[i]==0 ) استمر;
    إذا( pIdx==pPk ) استمر;
    إذا( iIdxCur+i==iIdxNoSeek ) استمر;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_شاخص_کلی(pParse, pIdx, i_معلومات_Cur, 0, 1,
        &iPartIdxLabel, p_مخکینی, r1);
    إذا( pIdx->b_Has_بیان && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pIdx, P4_شاخص);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_کلی_ستون : pIdx->n_ستنه);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    p_مخکینی = pIdx;
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
صحيح sqlite3_Generate_شاخص_کلی(
  Parse *pParse,       /* Parsing context */
  شاخص *pIdx,         /* The index for which to generate a key */
  صحيح i_معلومات_Cur,        /* Cursor number from which to take column data */
  صحيح reg_بهر,          /* Put the new key into this register if not 0 */
  صحيح مختاړی_Only,      /* Compute only a unique prefix of the key */
  صحيح *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  شاخص *p_مخکینی,       /* Previously generated index key */
  صحيح reg_مخکینی         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  صحيح j;
  صحيح reg_بنسټ;
  صحيح n_ستون;

  إذا( piPartIdxLabel ){
    إذا( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = i_معلومات_Cur + 1;
      sqlite3_بیان_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      p_مخکینی = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }وإلا{
      *piPartIdxLabel = 0;
    }
  }
  n_ستون = (مختاړی_Only && pIdx->uniqNotNull) ? pIdx->n_کلی_ستون : pIdx->n_ستنه;
  reg_بنسټ = sqlite3_Get_لنډمهاله_Range(pParse, n_ستون);
  إذا( p_مخکینی && (reg_بنسټ!=reg_مخکینی || p_مخکینی->pPartIdxWhere) ) p_مخکینی = 0;
  لكل(j=0; j<n_ستون; j++){
    صحيح i_Tab_ستون;
    إذا( p_مخکینی
     && p_مخکینی->ai_ستنه[j]==pIdx->ai_ستنه[j]
     && p_مخکینی->ai_ستنه[j]!=XN_بیان
    ){
      /* This column was already computed by the previous index */
      استمر;
    }
    sqlite3_بیان_کوډ_بارول_شاخص_ستنه(pParse, pIdx, i_معلومات_Cur, j, reg_بنسټ+j);
    إذا( (i_Tab_ستون = pIdx->ai_ستنه[j])>=0 
     && (pIdx->p_جدول->a_ستون[i_Tab_ستون].ستون_Flags & COLFLAG_VIRTUAL)==0
    ){
      /* If the column affinity is REAL but the number is an integer, then it
      ** might be stored in the table as an integer (using a compact
      ** representation) then converted to REAL by an OP_RealAffinity opcode.
      ** But we are getting ready to store this value back into an index, where
      ** it should be converted by to INTEGER again.  So omit the
      ** OP_RealAffinity opcode if it is present */
      sqlite3_Vdbe_Delete_مخکینی_Opcode(v, OP_RealAffinity);
    }
  }
  إذا( reg_بهر ){
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_بنسټ, n_ستون, reg_بهر);
  }
  sqlite3_Release_لنډمهاله_Range(pParse, reg_بنسټ, n_ستون);
  أرجع reg_بنسټ;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
فراغ sqlite3ResolvePartIdxLabel(Parse *pParse, صحيح iLabel){
  إذا( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
