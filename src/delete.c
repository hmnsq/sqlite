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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

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
ಕೋಷ್ಟಕ *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  Src_ಅಂಶ *p_ಅಂಶ = pSrc->a;
  ಕೋಷ್ಟಕ *pTab;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ && pSrc->nSrc>=1 );
  pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, p_ಅಂಶ);
  ಆದರೆ( p_ಅಂಶ->pSTab ) sqlite3_Delete_ಕೋಷ್ಟಕ(pParse->db, p_ಅಂಶ->pSTab);
  p_ಅಂಶ->pSTab = pTab;
  p_ಅಂಶ->fg.notCte = 1;
  ಆದರೆ( pTab ){
    pTab->nTabRef++;
    ಆದರೆ( p_ಅಂಶ->fg.is_ಸೂಚ್ಯಂಕಿತ_By && sqlite3_ಸೂಚ್ಯಂಕಿತ_By_Lookup(pParse, p_ಅಂಶ) ){
      pTab = 0;
    }
  }
  ಹಿಂತಿರುಗಿ pTab;
}

/* Generate byte-code that will report the number of rows modified
** by a DELETE, INSERT, or UPDATE statement.
*/
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Change_ಎಣಿಕೆ(Vdbe *v, ಪೂರ್ಣಾಂಕ regCounter, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು_Name){
  sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_FkCheck);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, regCounter, 1);
  sqlite3_Vdbe_ಗಣ_ಅಂಕೆ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ(v, 1);
  sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, 0, COLNAME_NAME, z_ಲಂಬಸಾಲು_Name, SQLITE_STATIC);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vtabIsReadOnly(Parse *pParse, ಕೋಷ್ಟಕ *pTab){
  ಪ್ರತಿಜ್ಞಾ( IsVirtual(pTab) );
  ಆದರೆ( sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 ){
    ಹಿಂತಿರುಗಿ 1;
  }

  /* Within triggers:
  **   *  Do not allow DELETE, INSERT, or UPDATE of SQLITE_VTAB_DIRECTONLY
  **      virtual tables
  **   *  Only allow DELETE, INSERT, or UPDATE of non-SQLITE_VTAB_INNOCUOUS
  **      virtual tables if PRAGMA trusted_schema=ON.
  */
  ಆದರೆ( (pParse->pToplevel!=0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
   && pTab->u.vtab.p->eVtabRisk >
           ((pParse->db->flags & SQLITE_TrustedSchema)!=0)
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "unsafe use of virtual table \"%s\"",
      pTab->zName);
  }
  ಹಿಂತಿರುಗಿ 0;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tabIsReadOnly(Parse *pParse, ಕೋಷ್ಟಕ *pTab){
  sqlite3 *db;
  ಆದರೆ( IsVirtual(pTab) ){
    ಹಿಂತಿರುಗಿ vtabIsReadOnly(pParse, pTab);
  }
  ಆದರೆ( (pTab->tabFlags & (TF_Readonly|TF_Shadow))==0 ) ಹಿಂತಿರುಗಿ 0;
  db = pParse->db;
  ಆದರೆ( (pTab->tabFlags & TF_Readonly)!=0 ){
    ಹಿಂತಿರುಗಿ sqlite3WritableSchema(db)==0 && pParse->nested==0;
  }
  ಪ್ರತಿಜ್ಞಾ( pTab->tabFlags & TF_Shadow );
  ಹಿಂತಿರುಗಿ sqlite3ReadOnlyShadowTables(db);
}

/*
** Check to make sure the given table is writable.
**
** If pTab is not writable  ->  generate an error message and return 1.
** If pTab is writable but other errors have occurred -> return 1.
** If pTab is writable and no prior errors -> return 0;
*/
ಪೂರ್ಣಾಂಕ sqlite3IsReadOnly(Parse *pParse, ಕೋಷ್ಟಕ *pTab, Trigger *pTrigger){
  ಆದರೆ( tabIsReadOnly(pParse, pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "table %s may not be modified", pTab->zName);
    ಹಿಂತಿರುಗಿ 1;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIEW
  ಆದರೆ( IsView(pTab)
   && (pTrigger==0 || (pTrigger->bReturning && pTrigger->p_ಮುಂದಿನ==0))
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse,"cannot modify %s because it is a view",pTab->zName);
    ಹಿಂತಿರುಗಿ 1;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 0;
}


#ಆದರೆ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
ಶೂನ್ಯ sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  ಕೋಷ್ಟಕ *pView,        /* View definition */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,        /* Optional WHERE clause to be added */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,  /* Optional ORDER BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ,        /* Optional LIMIT clause */
  ಪೂರ್ಣಾಂಕ iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pView->pSchema);
  pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  ಆದರೆ( pFrom ){
    ಪ್ರತಿಜ್ಞಾ( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    ಪ್ರತಿಜ್ಞಾ( pFrom->a[0].fg.fixedSchema==0 && pFrom->a[0].fg.isSubquery==0 );
    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
    ಪ್ರತಿಜ್ಞಾ( pFrom->a[0].fg.isUsing==0 );
    ಪ್ರತಿಜ್ಞಾ( pFrom->a[0].u3.pOn==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, p_ಕ್ರಮ_By,
                          SF_IncludeHidden, p_ಎಲ್ಲೆ);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#ಆದರೆ defined(SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಎಲ್ಲೆ_Where(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,                /* The WHERE clause.  May be null */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,          /* The ORDER BY clause.  May be null */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ,                /* The LIMIT clause.  May be null */
  ಅಕ್ಷರ *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pLhs = ಖಾಲಿ;           /* LHS of IN(SELECT...) operator */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಒಳ_Clause = ಖಾಲಿ;      /* WHERE rowid IN ( select ) */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList = ಖಾಲಿ;     /* Expression list containing only pSelectRowid*/
  SrcList *pSelectSrc = ಖಾಲಿ;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = ಖಾಲಿ;      /* Complete SELECT tree */
  ಕೋಷ್ಟಕ *pTab;

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  ಆದರೆ( p_ಕ್ರಮ_By && p_ಎಲ್ಲೆ==0 ) {
    sqlite3_Error_ಸಂದೇಶ(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, p_ಕ್ರಮ_By);
    ಹಿಂತಿರುಗಿ 0;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  ಆದರೆ( p_ಎಲ್ಲೆ == 0 ) {
    ಹಿಂತಿರುಗಿ pWhere;
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
  ಆದರೆ( HasRowid(pTab) ){
    pLhs = sqlite3PExpr(pParse, TK_ಅಡ್ಡಸಾಲು, 0, 0);
    pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(
        pParse, 0, sqlite3PExpr(pParse, TK_ಅಡ್ಡಸಾಲು, 0, 0)
    );
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
    ಪ್ರತಿಜ್ಞಾ( pPk!=0 );
    ಪ್ರತಿಜ್ಞಾ( pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು>=1 );
    ಆದರೆ( pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು==1 ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
      ಪ್ರತಿಜ್ಞಾ( pPk->ai_ಸ್ತಂಭ[0]>=0 && pPk->ai_ಸ್ತಂಭ[0]<pTab->n_ಲಂಬಸಾಲು );
      zName = pTab->a_ಲಂಬಸಾಲು[pPk->ai_ಸ್ತಂಭ[0]].zCnName;
      pLhs = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zName);
      pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zName));
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ(i=0; i<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
        ಅಭಿವ್ಯಕ್ತಿ *p;
        ಪ್ರತಿಜ್ಞಾ( pPk->ai_ಸ್ತಂಭ[i]>=0 && pPk->ai_ಸ್ತಂಭ[i]<pTab->n_ಲಂಬಸಾಲು );
        p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, pTab->a_ಲಂಬಸಾಲು[pPk->ai_ಸ್ತಂಭ[i]].zCnName);
        pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pEList, p);
      }
      pLhs = sqlite3PExpr(pParse, TK_ಸದಿಶ, 0, 0);
      ಆದರೆ( pLhs ){
        pLhs->x.pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, 0);
      }
    }
  }

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSrc->a[0].pSTab = 0;
  pSelectSrc = sqlite3SrcListDup(db, pSrc, 0);
  pSrc->a[0].pSTab = pTab;
  ಆದರೆ( pSrc->a[0].fg.is_ಸೂಚ್ಯಂಕಿತ_By ){
    ಪ್ರತಿಜ್ಞಾ( pSrc->a[0].fg.isCte==0 );
    pSrc->a[0].u2.pIBIndex = 0;
    pSrc->a[0].fg.is_ಸೂಚ್ಯಂಕಿತ_By = 0;
    sqlite3DbFree(db, pSrc->a[0].u1.z_ಸೂಚ್ಯಂಕಿತ_By);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSrc->a[0].fg.isCte ){
    pSrc->a[0].u2.pCteUse->nUse++;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse, pEList, pSelectSrc, pWhere, 0 ,0,
      p_ಕ್ರಮ_By,0,p_ಎಲ್ಲೆ
  );

  /* now generate the new WHERE rowid IN clause for the DELETE/UPDATE */
  p_ಒಳ_Clause = sqlite3PExpr(pParse, TK_ಒಳ, pLhs, 0);
  sqlite3_PExpr_ಕೂಡಿಸು_Select(pParse, p_ಒಳ_Clause, pSelect);
  ಹಿಂತಿರುಗಿ p_ಒಳ_Clause;
}
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
ಶೂನ್ಯ sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,          /* The WHERE clause.  May be null */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,    /* ORDER BY clause. May be null */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ           /* LIMIT clause. May be null */
){
  Vdbe *v;               /* The virtual database engine */
  ಕೋಷ್ಟಕ *pTab;           /* The table from which records will be deleted */
  ಪೂರ್ಣಾಂಕ i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  ಸೂಚ್ಯಂಕ *pIdx;           /* For looping over indices of the table */
  ಪೂರ್ಣಾಂಕ iTabCur;           /* Cursor number for the table */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur = 0;      /* VDBE cursor for the canonical data source */
  ಪೂರ್ಣಾಂಕ iIdxCur = 0;       /* Cursor number of the first index */
  ಪೂರ್ಣಾಂಕ nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  ಪೂರ್ಣಾಂಕ iDb;               /* Database number */
  ಪೂರ್ಣಾಂಕ memCnt = 0;        /* Memory cell used for change counting */
  ಪೂರ್ಣಾಂಕ rcauth;            /* Value returned by authorization callback */
  ಪೂರ್ಣಾಂಕ eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  ಪೂರ್ಣಾಂಕ aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *a_ಗಮ್ಯ_Open = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  ಸೂಚ್ಯಂಕ *pPk;            /* The PRIMARY KEY index on the table */
  ಪೂರ್ಣಾಂಕ iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ;              /* Memory cell holding key of row to be deleted */
  i16 n_ಕೀಲಿ;              /* Number of memory cells in the row key */
  ಪೂರ್ಣಾಂಕ iEphCur = 0;       /* Ephemeral table holding all primary key values */
  ಪೂರ್ಣಾಂಕ i_ಅಡ್ಡಸಾಲು_ಗಣ = 0;       /* Register for rowset of rows to delete */
  ಪೂರ್ಣಾಂಕ addrBypass = 0;    /* Address of jump over the delete logic */
  ಪೂರ್ಣಾಂಕ addrLoop = 0;      /* Top of the delete loop */
  ಪೂರ್ಣಾಂಕ addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  ಪೂರ್ಣಾಂಕ bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಪೂರ್ಣಾಂಕ isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#ಆದರೆ_ಅಂತ್ಯ

  memset(&sContext, 0, ಗಾತ್ರ(sContext));
  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಆದರೆ( pParse->nErr ){
    ಹೋಗು delete_from_cleanup;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  ಪ್ರತಿಜ್ಞಾ( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ಆದರೆ( pTab==0 )  ಹೋಗು delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = IsView(pTab);
#ಇಲ್ಲದಿದ್ದರೆ
# define pTrigger 0
# define isView 0
#ಆದರೆ_ಅಂತ್ಯ
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x10000 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(0, "In sqlite3Delete() at %s:%d", __FILE__, __LINE__);
    sqlite3_ವೃಕ್ಷ_View_Delete(pParse->pWith, pTabList, pWhere,
                          p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ, pTrigger);
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ
  ಆದರೆ( !isView ){
    pWhere = sqlite3_ಎಲ್ಲೆ_Where(
        pParse, pTabList, pWhere, p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ, "DELETE"
    );
    p_ಕ್ರಮ_By = 0;
    p_ಎಲ್ಲೆ = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If pTab is really a view, make sure it has been initialized.
  */
  ಆದರೆ( sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab) ){
    ಹೋಗು delete_from_cleanup;
  }

  ಆದರೆ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ಹೋಗು delete_from_cleanup;
  }
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb<db->nDb );
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0,
                            db->aDb[iDb].zDbSName);
  ಪ್ರತಿಜ್ಞಾ( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  ಆದರೆ( rcauth==SQLITE_DENY ){
    ಹೋಗು delete_from_cleanup;
  }
  ಪ್ರತಿಜ್ಞಾ(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  ಪ್ರತಿಜ್ಞಾ( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  ಗಾಗಿ(nIdx=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  ಆದರೆ( isView ){
    sqlite3_Auth_Context_ತಳ್ಳು(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v==0 ){
    ಹೋಗು delete_from_cleanup;
  }
  ಆದರೆ( pParse->nested==0 ) sqlite3_Vdbe_ಎಣಿಕೆ_Changes(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ಆದರೆ( isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ, iTabCur
    );
    i_ದತ್ತಾಂಶ_Cur = iIdxCur = iTabCur;
    p_ಕ್ರಮ_By = 0;
    p_ಎಲ್ಲೆ = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Resolve the column names in the WHERE clause.
  */
  memset(&sNC, 0, ಗಾತ್ರ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pWhere) ){
    ಹೋಗು delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  ಆದರೆ( (db->flags & SQLITE_ಎಣಿಕೆ_ಅಡ್ಡಸಾಲುಗಳು)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    memCnt = ++pParse->nMem;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, memCnt);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRUNCATE_OPTIMIZATION
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
  ಆದರೆ( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#ಆದರೆ_ಅಂತ್ಯ
  ){
    ಪ್ರತಿಜ್ಞಾ( !isView );
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum, 1, pTab->zName);
    ಆದರೆ( HasRowid(pTab) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Clear, pTab->tnum, iDb, memCnt ? memCnt : -1,
                        pTab->zName, P4_STATIC);
    }
    ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
      ಪ್ರತಿಜ್ಞಾ( pIdx->pSchema==pTab->pSchema );
      ಆದರೆ( Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) && !HasRowid(pTab) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Clear, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Clear, pIdx->tnum, iDb);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK;
    ಆದರೆ( sNC.ncFlags & NC_Subquery ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    ಆದರೆ( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      ಪ್ರತಿಜ್ಞಾ( nPk==1 );
      i_ಅಡ್ಡಸಾಲು_ಗಣ = ++pParse->nMem;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, i_ಅಡ್ಡಸಾಲು_ಗಣ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      ಪ್ರತಿಜ್ಞಾ( pPk!=0 );
      nPk = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pPk);
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
    ಆದರೆ( pWInfo==0 ) ಹೋಗು delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    ಪ್ರತಿಜ್ಞಾ( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    ಪ್ರತಿಜ್ಞಾ( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF
            || OptimizationDisabled(db, SQLITE_OnePass) );
    ಆದರೆ( eOnePass!=ONEPASS_SINGLE ) sqlite3MultiWrite(pParse);
    ಆದರೆ( sqlite3_Where_ಬಳಸುತ್ತದೆ_Deferred_Seek(pWInfo) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ಮುಕ್ತಾಯ_Seek, iTabCur);
    }
 
    /* Keep track of the number of rows to be deleted */
    ಆದರೆ( memCnt ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, memCnt, 1);
    }
 
    /* Extract the rowid or primary key for the current row */
    ಆದರೆ( pPk ){
      ಗಾಗಿ(i=0; i<nPk; i++){
        ಪ್ರತಿಜ್ಞಾ( pPk->ai_ಸ್ತಂಭ[i]>=0 );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iTabCur,
                                        pPk->ai_ಸ್ತಂಭ[i], iPk+i);
      }
      i_ಕೀಲಿ = iPk;
    }ಇಲ್ಲದಿದ್ದರೆ{
      i_ಕೀಲಿ = ++pParse->nMem;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iTabCur, -1, i_ಕೀಲಿ);
    }
 
    ಆದರೆ( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      n_ಕೀಲಿ = nPk; /* OP_Found will use an unpacked key */
      a_ಗಮ್ಯ_Open = sqlite3DbMallocRawNN(db, nIdx+2);
      ಆದರೆ( a_ಗಮ್ಯ_Open==0 ){
        sqlite3_Where_ಕೊನೆ(pWInfo);
        ಹೋಗು delete_from_cleanup;
      }
      memset(a_ಗಮ್ಯ_Open, 1, nIdx+1);
      a_ಗಮ್ಯ_Open[nIdx+1] = 0;
      ಆದರೆ( aiCurOnePass[0]>=0 ) a_ಗಮ್ಯ_Open[aiCurOnePass[0]-iTabCur] = 0;
      ಆದರೆ( aiCurOnePass[1]>=0 ) a_ಗಮ್ಯ_Open[aiCurOnePass[1]-iTabCur] = 0;
      ಆದರೆ( addrEphOpen ) sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( pPk ){
        /* Add the PK key for this row to the temporary table */
        i_ಕೀಲಿ = ++pParse->nMem;
        n_ಕೀಲಿ = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, iPk, nPk, i_ಕೀಲಿ,
            sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(pParse->db, pPk), nPk);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iEphCur, i_ಕೀಲಿ, iPk, nPk);
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Add the rowid of the row to be deleted to the RowSet */
        n_ಕೀಲಿ = 1;  /* OP_DeferredSeek always uses a single rowid */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಅಡ್ಡಸಾಲು_ಗಣ_ಕೂಡಿಸು, i_ಅಡ್ಡಸಾಲು_ಗಣ, i_ಕೀಲಿ);
      }
      sqlite3_Where_ಕೊನೆ(pWInfo);
    }
 
    /* Unless this is a view, open cursors for the table we are
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF
    ** triggers.
    */
    ಆದರೆ( !isView ){
      ಪೂರ್ಣಾಂಕ iAddrOnce = 0;
      ಆದರೆ( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3_Open_ಕೋಷ್ಟಕ_And_Indices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, a_ಗಮ್ಯ_Open, &i_ದತ್ತಾಂಶ_Cur, &iIdxCur);
      ಪ್ರತಿಜ್ಞಾ( pPk || IsVirtual(pTab) || i_ದತ್ತಾಂಶ_Cur==iTabCur );
      ಪ್ರತಿಜ್ಞಾ( pPk || IsVirtual(pTab) || iIdxCur==i_ದತ್ತಾಂಶ_Cur+1 );
      ಆದರೆ( eOnePass==ONEPASS_MULTI ){
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ_Or_ತೆಗೆ_Inst(v, iAddrOnce);
      }
    }
 
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    ಆದರೆ( eOnePass!=ONEPASS_OFF ){
      ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ==nPk );  /* OP_Found will use an unpacked key */
      ಆದರೆ( !IsVirtual(pTab) && a_ಗಮ್ಯ_Open[i_ದತ್ತಾಂಶ_Cur-iTabCur] ){
        ಪ್ರತಿಜ್ಞಾ( pPk!=0 || IsView(pTab) );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು, i_ದತ್ತಾಂಶ_Cur, addrBypass, i_ಕೀಲಿ, n_ಕೀಲಿ);
        VdbeCoverage(v);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pPk ){
      addrLoop = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      ಆದರೆ( IsVirtual(pTab) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iEphCur, 0, i_ಕೀಲಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಅಡ್ಡಸಾಲು_ದತ್ತಾಂಶ, iEphCur, i_ಕೀಲಿ);
      }
      ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ==0 );  /* OP_Found will use a composite key */
    }ಇಲ್ಲದಿದ್ದರೆ{
      addrLoop = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಅಡ್ಡಸಾಲು_ಗಣ_Read, i_ಅಡ್ಡಸಾಲು_ಗಣ, 0, i_ಕೀಲಿ);
      VdbeCoverage(v);
      ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ==1 );
    } 
 
    /* Delete the row */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    ಆದರೆ( IsVirtual(pTab) ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pVTab = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      ಪ್ರತಿಜ್ಞಾ( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      ಆದರೆ( eOnePass==ONEPASS_SINGLE ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Close, iTabCur);
        ಆದರೆ( sqlite3IsToplevel(pParse) ){
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_VUpdate, 0, 1, i_ಕೀಲಿ, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = (pParse->nested==0);    /* True to count changes */
      sqlite3_Generate_ಅಡ್ಡಸಾಲು_Delete(pParse, pTab, pTrigger, i_ದತ್ತಾಂಶ_Cur, iIdxCur,
          i_ಕೀಲಿ, n_ಕೀಲಿ, ಎಣಿಕೆ_2, OE_Default, eOnePass, aiCurOnePass[1]);
    }
 
    /* End of the loop over all rowids/primary-keys. */
    ಆದರೆ( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3_Where_ಕೊನೆ(pWInfo);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pPk ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrLoop);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrLoop);
    }    
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ಆದರೆ( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_ಕೊನೆ(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ಆದರೆ( memCnt ){
    sqlite3_ಸಂಕೇತ_Change_ಎಣಿಕೆ(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3_Auth_Context_ತೆಗೆ(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
#ಆದರೆ defined(SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ)
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಕ್ರಮ_By);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಎಲ್ಲೆ);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( a_ಗಮ್ಯ_Open ) sqlite3DbNNFreeNN(db, a_ಗಮ್ಯ_Open);
  ಹಿಂತಿರುಗಿ;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ isView
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು isView
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ pTrigger
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು pTrigger
#ಆದರೆ_ಅಂತ್ಯ

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
ಶೂನ್ಯ sqlite3_Generate_ಅಡ್ಡಸಾಲು_Delete(
  Parse *pParse,     /* Parsing context */
  ಕೋಷ್ಟಕ *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur,      /* Cursor from which column data is extracted */
  ಪೂರ್ಣಾಂಕ iIdxCur,       /* First index cursor */
  ಪೂರ್ಣಾಂಕ iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 ಎಣಿಕೆ_2,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 e_ಬಹುಲಕ,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  ಪೂರ್ಣಾಂಕ iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  ಪೂರ್ಣಾಂಕ iOld = 0;                   /* First register in OLD.* array */
  ಪೂರ್ಣಾಂಕ iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  ಪ್ರತಿಜ್ಞಾ( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         i_ದತ್ತಾಂಶ_Cur, iIdxCur, iPk, (ಪೂರ್ಣಾಂಕ)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_Not_ಸಿಕ್ಕಿತು;
  ಆದರೆ( e_ಬಹುಲಕ==ONEPASS_OFF ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, opSeek, i_ದತ್ತಾಂಶ_Cur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_Not_ಸಿಕ್ಕಿತು);
  }

  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  ಆದರೆ( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 ಮುಖವಾಡ;                     /* Mask of OLD.* columns in use */
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                     /* Iterator used while populating OLD.* */
    ಪೂರ್ಣಾಂಕ addr_ಆರಂಭ;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    ಮುಖವಾಡ = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    ಮುಖವಾಡ |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->n_ಲಂಬಸಾಲು);

    /* Populate the OLD.* pseudo-table register array. These values will be
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, iPk, iOld);
    ಗಾಗಿ(i_ಲಂಬಸಾಲು=0; i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು; i_ಲಂಬಸಾಲು++){
      testcase( ಮುಖವಾಡ!=0xffffffff && i_ಲಂಬಸಾಲು==31 );
      testcase( ಮುಖವಾಡ!=0xffffffff && i_ಲಂಬಸಾಲು==32 );
      ಆದರೆ( ಮುಖವಾಡ==0xffffffff || (i_ಲಂಬಸಾಲು<=31 && (ಮುಖವಾಡ & MASKBIT32(i_ಲಂಬಸಾಲು))!=0) ){
        ಪೂರ್ಣಾಂಕ kk = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಲಂಬಸಾಲು);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur, i_ಲಂಬಸಾಲು, iOld+kk+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addr_ಆರಂಭ = sqlite3VdbeCurrentAddr(v);
    sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger,
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
    ಆದರೆ( addr_ಆರಂಭ<sqlite3VdbeCurrentAddr(v) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, opSeek, i_ದತ್ತಾಂಶ_Cur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_Not_ಸಿಕ್ಕಿತು);
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
  ಆದರೆ( !IsView(pTab) ){
    u8 p5 = 0;
    sqlite3_Generate_ಅಡ್ಡಸಾಲು_ಸೂಚ್ಯಂಕ_Delete(pParse, pTab, i_ದತ್ತಾಂಶ_Cur, iIdxCur,0,iIdxNoSeek);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Delete, i_ದತ್ತಾಂಶ_Cur, (ಎಣಿಕೆ_2?OPFLAG_NCHANGE:0));
    ಆದರೆ( pParse->nested==0 || 0==sqlite3_stricmp(pTab->zName, "sqlite_stat1") ){
      sqlite3VdbeAppendP4(v, (ಅಕ್ಷರ*)pTab, P4_ಕೋಷ್ಟಕ);
    }
    ಆದರೆ( e_ಬಹುಲಕ!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    ಆದರೆ( iIdxNoSeek>=0 && iIdxNoSeek!=i_ದತ್ತಾಂಶ_Cur ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Delete, iIdxNoSeek);
    }
    ಆದರೆ( e_ಬಹುಲಕ==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  ಆದರೆ( pTrigger ){
    sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger,
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
ಶೂನ್ಯ sqlite3_Generate_ಅಡ್ಡಸಾಲು_ಸೂಚ್ಯಂಕ_Delete(
  Parse *pParse,     /* Parsing and code generating context */
  ಕೋಷ್ಟಕ *pTab,       /* Table containing the row to be deleted */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur,      /* Cursor of table holding data. */
  ಪೂರ್ಣಾಂಕ iIdxCur,       /* First index cursor */
  ಪೂರ್ಣಾಂಕ *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  ಪೂರ್ಣಾಂಕ iIdxNoSeek     /* Do not delete from this cursor */
){
  ಪೂರ್ಣಾಂಕ i;             /* Index loop counter */
  ಪೂರ್ಣಾಂಕ r1 = -1;       /* Register holding an index key */
  ಪೂರ್ಣಾಂಕ iPartIdxLabel; /* Jump destination for skipping partial index entries */
  ಸೂಚ್ಯಂಕ *pIdx;       /* Current index */
  ಸೂಚ್ಯಂಕ *p_ಪೂರ್ವ = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  ಸೂಚ್ಯಂಕ *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
  ಗಾಗಿ(i=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; i++, pIdx=pIdx->p_ಮುಂದಿನ){
    ಪೂರ್ಣಾಂಕ p3 = 0;
    ಪ್ರತಿಜ್ಞಾ( iIdxCur+i!=i_ದತ್ತಾಂಶ_Cur || pPk==pIdx );
    ಆದರೆ( aRegIdx!=0 && aRegIdx[i]==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pIdx==pPk ) ಮುಂದುವರಿ;
    ಆದರೆ( iIdxCur+i==iIdxNoSeek ) ಮುಂದುವರಿ;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3_Generate_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(pParse, pIdx, i_ದತ್ತಾಂಶ_Cur, 0, 1,
        &iPartIdxLabel, p_ಪೂರ್ವ, r1);
    ಆದರೆ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ && aRegIdx ){
      p3 = aRegIdx[i];
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_IdxDelete, iIdxCur+i, r1, p3);
    sqlite3VdbeChangeP4(v, -1, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)pIdx, P4_ಸೂಚ್ಯಂಕ);
    sqlite3VdbeChangeP5(v, pIdx->uniqNotNull ? pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು : pIdx->n_ಸ್ತಂಭ);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    p_ಪೂರ್ವ = pIdx;
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
ಪೂರ್ಣಾಂಕ sqlite3_Generate_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(
  Parse *pParse,       /* Parsing context */
  ಸೂಚ್ಯಂಕ *pIdx,         /* The index for which to generate a key */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur,        /* Cursor number from which to take column data */
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು,          /* Put the new key into this register if not 0 */
  ಪೂರ್ಣಾಂಕ ಪೂರ್ವಪ್ರತ್ಯಯ_Only,      /* Compute only a unique prefix of the key */
  ಪೂರ್ಣಾಂಕ *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  ಸೂಚ್ಯಂಕ *p_ಪೂರ್ವ,       /* Previously generated index key */
  ಪೂರ್ಣಾಂಕ reg_ಪೂರ್ವ         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ j;
  ಪೂರ್ಣಾಂಕ reg_ತಳ;
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;

  ಆದರೆ( piPartIdxLabel ){
    ಆದರೆ( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = i_ದತ್ತಾಂಶ_Cur + 1;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False_Dup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
      p_ಪೂರ್ವ = 0; /* Ticket a9efb42811fa41ee 2019-11-02;
                  ** pPartIdxWhere may have corrupted regPrior registers */
    }ಇಲ್ಲದಿದ್ದರೆ{
      *piPartIdxLabel = 0;
    }
  }
  n_ಲಂಬಸಾಲು = (ಪೂರ್ವಪ್ರತ್ಯಯ_Only && pIdx->uniqNotNull) ? pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು : pIdx->n_ಸ್ತಂಭ;
  reg_ತಳ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಲಂಬಸಾಲು);
  ಆದರೆ( p_ಪೂರ್ವ && (reg_ತಳ!=reg_ಪೂರ್ವ || p_ಪೂರ್ವ->pPartIdxWhere) ) p_ಪೂರ್ವ = 0;
  ಗಾಗಿ(j=0; j<n_ಲಂಬಸಾಲು; j++){
    ಪೂರ್ಣಾಂಕ i_Tab_ಲಂಬಸಾಲು;
    ಆದರೆ( p_ಪೂರ್ವ
     && p_ಪೂರ್ವ->ai_ಸ್ತಂಭ[j]==pIdx->ai_ಸ್ತಂಭ[j]
     && p_ಪೂರ್ವ->ai_ಸ್ತಂಭ[j]!=XN_ಅಭಿವ್ಯಕ್ತಿ
    ){
      /* This column was already computed by the previous index */
      ಮುಂದುವರಿ;
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತುಂಬು_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ(pParse, pIdx, i_ದತ್ತಾಂಶ_Cur, j, reg_ತಳ+j);
    ಆದರೆ( (i_Tab_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[j])>=0 
     && (pIdx->p_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i_Tab_ಲಂಬಸಾಲು].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)==0
    ){
      /* If the column affinity is REAL but the number is an integer, then it
      ** might be stored in the table as an integer (using a compact
      ** representation) then converted to REAL by an OP_RealAffinity opcode.
      ** But we are getting ready to store this value back into an index, where
      ** it should be converted by to INTEGER again.  So omit the
      ** OP_RealAffinity opcode if it is present */
      sqlite3_Vdbe_Delete_ಪೂರ್ವ_Opcode(v, OP_RealAffinity);
    }
  }
  ಆದರೆ( reg_ಹೊರಹೋಗು ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ತಳ, n_ಲಂಬಸಾಲು, reg_ಹೊರಹೋಗು);
  }
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ತಳ, n_ಲಂಬಸಾಲು);
  ಹಿಂತಿರುಗಿ reg_ತಳ;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
ಶೂನ್ಯ sqlite3ResolvePartIdxLabel(Parse *pParse, ಪೂರ್ಣಾಂಕ iLabel){
  ಆದರೆ( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}
