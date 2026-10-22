/*
** 2003 January 11
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used to implement the sqlite3_set_authorizer()
** API.  This facility is an optional feature of the library.  Embedded
** systems that do not need this facility may omit it by recompiling
** the library with -DSQLITE_OMIT_AUTHORIZATION=1
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** All of the code in this file may be omitted by defining a single
** macro.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION

/*
** Set or clear the access authorization function.
**
** The access authorization function is be called during the compilation
** phase to verify that the user has read and/or write access permission on
** various fields of the database.  The first argument to the auth function
** is a copy of the 3rd argument to this routine.  The second argument
** to the auth function is one of these constants:
**
**       SQLITE_CREATE_INDEX
**       SQLITE_CREATE_TABLE
**       SQLITE_CREATE_TEMP_INDEX
**       SQLITE_CREATE_TEMP_TABLE
**       SQLITE_CREATE_TEMP_TRIGGER
**       SQLITE_CREATE_TEMP_VIEW
**       SQLITE_CREATE_TRIGGER
**       SQLITE_CREATE_VIEW
**       SQLITE_DELETE
**       SQLITE_DROP_INDEX
**       SQLITE_DROP_TABLE
**       SQLITE_DROP_TEMP_INDEX
**       SQLITE_DROP_TEMP_TABLE
**       SQLITE_DROP_TEMP_TRIGGER
**       SQLITE_DROP_TEMP_VIEW
**       SQLITE_DROP_TRIGGER
**       SQLITE_DROP_VIEW
**       SQLITE_INSERT
**       SQLITE_PRAGMA
**       SQLITE_READ
**       SQLITE_SELECT
**       SQLITE_TRANSACTION
**       SQLITE_UPDATE
**
** The third and fourth arguments to the auth function are the name of
** the table and the column that are being accessed.  The auth function
** should return either SQLITE_OK, SQLITE_DENY, or SQLITE_IGNORE.  If
** SQLITE_OK is returned, it means that access is allowed.  SQLITE_DENY
** means that the SQL statement will never-run - the sqlite3_exec() call
** will return with an error.  SQLITE_IGNORE means that the SQL statement
** should run but attempts to read the specified column will return NULL
** and attempts to write the column will be ignored.
**
** Setting the auth function to NULL disables this hook.  The default
** setting of the auth function is NULL.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಗಣ_authorizer(
  sqlite3 *db,
  ಪೂರ್ಣಾಂಕ (*xAuth)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*),
  ಶೂನ್ಯ *pArg
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  db->xAuth = (sqlite3_xauth)xAuth;
  db->pAuthArg = pArg;
  sqlite3ExpirePreparedStatements(db, 1);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Write an error message into pParse->zErrMsg that explains that the
** user-supplied authorization function returned an illegal value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite_Auth_Bad_Return_ಸಂಕೇತ(Parse *pParse){
  sqlite3_Error_ಸಂದೇಶ(pParse, "authorizer malfunction");
  pParse->rc = SQLITE_ERROR;
}

/*
** Invoke the authorization callback for permission to read column zCol from
** table zTab in database zDb. This function assumes that an authorization
** callback has been registered (i.e. that sqlite3.xAuth is not NULL).
**
** If SQLITE_IGNORE is returned and pExpr is not NULL, then pExpr is changed
** to an SQL NULL expression. Otherwise, if pExpr is NULL, then SQLITE_IGNORE
** is treated as SQLITE_DENY. In this case an error is left in pParse.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Auth_Read_ಲಂಬಸಾಲು(
  Parse *pParse,                  /* The parser context */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,               /* Table name */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು,               /* Column name */
  ಪೂರ್ಣಾಂಕ iDb                         /* Index of containing database. */
){
  sqlite3 *db = pParse->db;          /* Database handle */
  ಅಕ್ಷರ *zDb = db->aDb[iDb].zDbSName; /* Schema name of attached database */
  ಪೂರ್ಣಾಂಕ rc;                            /* Auth callback return code */

  ಆದರೆ( db->init.busy ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  rc = db->xAuth(db->pAuthArg, SQLITE_READ, zTab,z_ಲಂಬಸಾಲು,zDb,pParse->zAuthContext);
  ಆದರೆ( rc==SQLITE_DENY ){
    ಅಕ್ಷರ *z = sqlite3_mprintf("%s.%s", zTab, z_ಲಂಬಸಾಲು);
    ಆದರೆ( db->nDb>2 || iDb!=0 ) z = sqlite3_mprintf("%s.%z", zDb, z);
    sqlite3_Error_ಸಂದೇಶ(pParse, "access to %z is prohibited", z);
    pParse->rc = SQLITE_AUTH;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_IGNORE && rc!=SQLITE_OK ){
    sqlite_Auth_Bad_Return_ಸಂಕೇತ(pParse);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** The pExpr should be a TK_COLUMN expression.  The table referred to
** is in pTabList or else it is the NEW or OLD table of a trigger.  
** Check to see if it is OK to read this particular column.
**
** If the auth function returns SQLITE_IGNORE, change the TK_COLUMN 
** instruction into a TK_NULL.  If the auth function returns SQLITE_DENY,
** then generate an error.
*/
ಶೂನ್ಯ sqlite3AuthRead(
  Parse *pParse,        /* The parser context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The expression to check authorization on */
  Schema *pSchema,      /* The schema of the expression */
  SrcList *pTabList     /* All table that pExpr might refer to */
){
  ಕೋಷ್ಟಕ *pTab = 0;      /* The table being read */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು;     /* Name of the column of the table */
  ಪೂರ್ಣಾಂಕ iSrc;             /* Index in pTabList->a[] of table being read */
  ಪೂರ್ಣಾಂಕ iDb;              /* The index of the database the expression refers to */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;             /* Index of column in table */

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRIGGER );
  ಪ್ರತಿಜ್ಞಾ( !ಒಳ_RENAME_OBJECT );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->xAuth!=0 );
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pSchema);
  ಆದರೆ( iDb<0 ){
    /* An attempt to read a column out of a subquery or other
    ** temporary table. */
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRIGGER ){
    pTab = pParse->pTriggerTab;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pTabList );
    ಗಾಗಿ(iSrc=0; iSrc<pTabList->nSrc; iSrc++){
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pTabList->a[iSrc].iCursor ){
        pTab = pTabList->a[iSrc].pSTab;
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  i_ಲಂಬಸಾಲು = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
  ಆದರೆ( pTab==0 ) ಹಿಂತಿರುಗಿ;

  ಆದರೆ( i_ಲಂಬಸಾಲು>=0 ){
    ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು );
    z_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTab->iPKey>=0 ){
    ಪ್ರತಿಜ್ಞಾ( pTab->iPKey<pTab->n_ಲಂಬಸಾಲು );
    z_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು[pTab->iPKey].zCnName;
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ಲಂಬಸಾಲು = "ROWID";
  }
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<pParse->db->nDb );
  ಆದರೆ( SQLITE_IGNORE==sqlite3_Auth_Read_ಲಂಬಸಾಲು(pParse, pTab->zName, z_ಲಂಬಸಾಲು, iDb) ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
  }
}

/*
** Do an authorization check using the code and arguments given.  Return
** either SQLITE_OK (zero) or SQLITE_IGNORE or SQLITE_DENY.  If SQLITE_DENY
** is returned, then the error count and error message in pParse are
** modified appropriately.
**
** Divided into two routines.  realAuthCheck() does the work.  The
** sqlite3AuthCheck() routine is usually a fast no-op but invokes
** realAuthCheck() (and spends time doing some stack pushes and pops
** as a result) in the uncommon case where an authorization check is
** actually needed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE realAuthCheck(
  Parse *pParse,
  ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg1,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg2,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg3
){
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ rc;

  /* Don't do any authorization checks if the database is initializing
  ** or if the parser is being invoked from within sqlite3_declare_vtab.
  */
  ಪ್ರತಿಜ್ಞಾ( !ಒಳ_RENAME_OBJECT || db->xAuth==0 );
  ಆದರೆ( ಒಳ_SPECIAL_PARSE ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  /* EVIDENCE-OF: R-43249-19882 The third through sixth parameters to the
  ** callback are either NULL pointers or zero-terminated strings that
  ** contain additional details about the action to be authorized.
  **
  ** The following testcase() macros show that any of the 3rd through 6th
  ** parameters can be either NULL or a string. */
  testcase( zArg1==0 );
  testcase( zArg2==0 );
  testcase( zArg3==0 );
  testcase( pParse->zAuthContext==0 );

  rc = db->xAuth(db->pAuthArg,ಸಂಕೇತ_2,zArg1,zArg2,zArg3,pParse->zAuthContext);
  ಆದರೆ( rc==SQLITE_DENY ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "not authorized");
    pParse->rc = SQLITE_AUTH;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_OK && rc!=SQLITE_IGNORE ){
    rc = SQLITE_DENY;
    sqlite_Auth_Bad_Return_ಸಂಕೇತ(pParse);
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3AuthCheck(
  Parse *pParse,
  ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg1,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg2,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg3
){
  ಆದರೆ( pParse->db->xAuth!=0 && pParse->db->init.busy==0 ){
    ಹಿಂತಿರುಗಿ realAuthCheck(pParse,ಸಂಕೇತ_2,zArg1,zArg2,zArg3);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
}

/*
** Push an authorization context.  After this routine is called, the
** zArg3 argument to authorization callbacks will be zContext until
** popped.  Or if pParse==0, this routine is a no-op.
*/
ಶೂನ್ಯ sqlite3_Auth_Context_ತಳ್ಳು(
  Parse *pParse,
  AuthContext *pContext, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zContext
){
  ಪ್ರತಿಜ್ಞಾ( pParse );
  pContext->pParse = pParse;
  pContext->zAuthContext = pParse->zAuthContext;
  pParse->zAuthContext = zContext;
}

/*
** Pop an authorization context that was previously pushed
** by sqlite3AuthContextPush
*/
ಶೂನ್ಯ sqlite3_Auth_Context_ತೆಗೆ(AuthContext *pContext){
  ಆದರೆ( pContext->pParse ){
    pContext->pParse->zAuthContext = pContext->zAuthContext;
    pContext->pParse = 0;
  }
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_AUTHORIZATION */
