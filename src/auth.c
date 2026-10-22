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
#అన్తర్భావ "sqliteInt.h"

/*
** All of the code in this file may be omitted by defining a single
** macro.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION

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
పూర్ణాంకం sqlite3_సమితి_authorizer(
  sqlite3 *db,
  పూర్ణాంకం (*xAuth)(శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*),
  శూన్య *pArg
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  db->xAuth = (sqlite3_xauth)xAuth;
  db->pAuthArg = pArg;
  sqlite3ExpirePreparedStatements(db, 1);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

/*
** Write an error message into pParse->zErrMsg that explains that the
** user-supplied authorization function returned an illegal value.
*/
స్థిర శూన్య sqlite_Auth_Bad_Return_సంకేతం(Parse *pParse){
  sqlite3_Error_సందేశం(pParse, "authorizer malfunction");
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
పూర్ణాంకం sqlite3_Auth_Read_నిలువు(
  Parse *pParse,                  /* The parser context */
  స్థిరరాశి అక్షరం *zTab,               /* Table name */
  స్థిరరాశి అక్షరం *z_నిలువు,               /* Column name */
  పూర్ణాంకం iDb                         /* Index of containing database. */
){
  sqlite3 *db = pParse->db;          /* Database handle */
  అక్షరం *zDb = db->aDb[iDb].zDbSName; /* Schema name of attached database */
  పూర్ణాంకం rc;                            /* Auth callback return code */

  ఒకవేళ( db->init.busy ) తిరిగి SQLITE_OK;
  rc = db->xAuth(db->pAuthArg, SQLITE_READ, zTab,z_నిలువు,zDb,pParse->zAuthContext);
  ఒకవేళ( rc==SQLITE_DENY ){
    అక్షరం *z = sqlite3_mprintf("%s.%s", zTab, z_నిలువు);
    ఒకవేళ( db->nDb>2 || iDb!=0 ) z = sqlite3_mprintf("%s.%z", zDb, z);
    sqlite3_Error_సందేశం(pParse, "access to %z is prohibited", z);
    pParse->rc = SQLITE_AUTH;
  }లేకపోతే ఒకవేళ( rc!=SQLITE_IGNORE && rc!=SQLITE_OK ){
    sqlite_Auth_Bad_Return_సంకేతం(pParse);
  }
  తిరిగి rc;
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
శూన్య sqlite3AuthRead(
  Parse *pParse,        /* The parser context */
  సమాసం *p_సమాసం,          /* The expression to check authorization on */
  Schema *pSchema,      /* The schema of the expression */
  SrcList *pTabList     /* All table that pExpr might refer to */
){
  పట్టిక *pTab = 0;      /* The table being read */
  స్థిరరాశి అక్షరం *z_నిలువు;     /* Name of the column of the table */
  పూర్ణాంకం iSrc;             /* Index in pTabList->a[] of table being read */
  పూర్ణాంకం iDb;              /* The index of the database the expression refers to */
  పూర్ణాంకం i_నిలువు;             /* Index of column in table */

  ప్రతిజ్ఞా( p_సమాసం->op==TK_స్తంభం || p_సమాసం->op==TK_TRIGGER );
  ప్రతిజ్ఞా( !లోపల_RENAME_OBJECT );
  ప్రతిజ్ఞా( pParse->db->xAuth!=0 );
  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pSchema);
  ఒకవేళ( iDb<0 ){
    /* An attempt to read a column out of a subquery or other
    ** temporary table. */
    తిరిగి;
  }

  ఒకవేళ( p_సమాసం->op==TK_TRIGGER ){
    pTab = pParse->pTriggerTab;
  }లేకపోతే{
    ప్రతిజ్ఞా( pTabList );
    కొరకు(iSrc=0; iSrc<pTabList->nSrc; iSrc++){
      ఒకవేళ( p_సమాసం->i_పట్టిక==pTabList->a[iSrc].iCursor ){
        pTab = pTabList->a[iSrc].pSTab;
        ఆపు;
      }
    }
  }
  i_నిలువు = p_సమాసం->i_స్తంభం;
  ఒకవేళ( pTab==0 ) తిరిగి;

  ఒకవేళ( i_నిలువు>=0 ){
    ప్రతిజ్ఞా( i_నిలువు<pTab->n_నిలువు );
    z_నిలువు = pTab->a_నిలువు[i_నిలువు].zCnName;
  }లేకపోతే ఒకవేళ( pTab->iPKey>=0 ){
    ప్రతిజ్ఞా( pTab->iPKey<pTab->n_నిలువు );
    z_నిలువు = pTab->a_నిలువు[pTab->iPKey].zCnName;
  }లేకపోతే{
    z_నిలువు = "ROWID";
  }
  ప్రతిజ్ఞా( iDb>=0 && iDb<pParse->db->nDb );
  ఒకవేళ( SQLITE_IGNORE==sqlite3_Auth_Read_నిలువు(pParse, pTab->zName, z_నిలువు, iDb) ){
    p_సమాసం->op = TK_NULL;
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
స్థిర పూర్ణాంకం SQLITE_NOINLINE realAuthCheck(
  Parse *pParse,
  పూర్ణాంకం సంకేతం_2,
  స్థిరరాశి అక్షరం *zArg1,
  స్థిరరాశి అక్షరం *zArg2,
  స్థిరరాశి అక్షరం *zArg3
){
  sqlite3 *db = pParse->db;
  పూర్ణాంకం rc;

  /* Don't do any authorization checks if the database is initializing
  ** or if the parser is being invoked from within sqlite3_declare_vtab.
  */
  ప్రతిజ్ఞా( !లోపల_RENAME_OBJECT || db->xAuth==0 );
  ఒకవేళ( లోపల_SPECIAL_PARSE ){
    తిరిగి SQLITE_OK;
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

  rc = db->xAuth(db->pAuthArg,సంకేతం_2,zArg1,zArg2,zArg3,pParse->zAuthContext);
  ఒకవేళ( rc==SQLITE_DENY ){
    sqlite3_Error_సందేశం(pParse, "not authorized");
    pParse->rc = SQLITE_AUTH;
  }లేకపోతే ఒకవేళ( rc!=SQLITE_OK && rc!=SQLITE_IGNORE ){
    rc = SQLITE_DENY;
    sqlite_Auth_Bad_Return_సంకేతం(pParse);
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3AuthCheck(
  Parse *pParse,
  పూర్ణాంకం సంకేతం_2,
  స్థిరరాశి అక్షరం *zArg1,
  స్థిరరాశి అక్షరం *zArg2,
  స్థిరరాశి అక్షరం *zArg3
){
  ఒకవేళ( pParse->db->xAuth!=0 && pParse->db->init.busy==0 ){
    తిరిగి realAuthCheck(pParse,సంకేతం_2,zArg1,zArg2,zArg3);
  }లేకపోతే{
    తిరిగి SQLITE_OK;
  }
}

/*
** Push an authorization context.  After this routine is called, the
** zArg3 argument to authorization callbacks will be zContext until
** popped.  Or if pParse==0, this routine is a no-op.
*/
శూన్య sqlite3_Auth_Context_నెట్టు(
  Parse *pParse,
  AuthContext *pContext, 
  స్థిరరాశి అక్షరం *zContext
){
  ప్రతిజ్ఞా( pParse );
  pContext->pParse = pParse;
  pContext->zAuthContext = pParse->zAuthContext;
  pParse->zAuthContext = zContext;
}

/*
** Pop an authorization context that was previously pushed
** by sqlite3AuthContextPush
*/
శూన్య sqlite3_Auth_Context_తీయి(AuthContext *pContext){
  ఒకవేళ( pContext->pParse ){
    pContext->pParse->zAuthContext = pContext->zAuthContext;
    pContext->pParse = 0;
  }
}

#అయితే_ముగింపు /* SQLITE_OMIT_AUTHORIZATION */
