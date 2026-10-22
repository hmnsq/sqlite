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
#include "sqliteInt.h"

/*
** All of the code in this file may be omitted by defining a single
** macro.
*/
#ifndef SQLITE_OMIT_AUTHORIZATION

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
ကိန်းပြည့် sqlite3_set_authorizer(
  sqlite3 *db,
  ကိန်းပြည့် (*xAuth)(void*,ကိန်းပြည့်,const var_na*,const var_na*,const var_na*,const var_na*),
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xAuth = (sqlite3_xauth)xAuth;
  db->pAuthArg = pArg;
  sqlite3ExpirePreparedStatements(db, 1);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}

/*
** Write an error message into pParse->zErrMsg that explains that the
** user-supplied authorization function returned an illegal value.
*/
static void sqliteAuthBadReturnCode(Parse *pParse){
  sqlite3ErrorMsg(pParse, "authorizer malfunction");
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
ကိန်းပြည့် sqlite3AuthReadCol(
  Parse *pParse,                  /* The parser context */
  const var_na *zTab,               /* Table name */
  const var_na *zCol,               /* Column name */
  ကိန်းပြည့် iDb                         /* Index of containing database. */
){
  sqlite3 *db = pParse->db;          /* Database handle */
  var_na *zDb = db->aDb[iDb].zDbSName; /* Schema name of attached database */
  ကိန်းပြည့် rc;                            /* Auth callback return code */

  အကယ်၍( db->init.busy ) ပြန်ပေး SQLITE_OK;
  rc = db->xAuth(db->pAuthArg, SQLITE_READ, zTab,zCol,zDb,pParse->zAuthContext);
  အကယ်၍( rc==SQLITE_DENY ){
    var_na *z = sqlite3_mprintf("%s.%s", zTab, zCol);
    အကယ်၍( db->nDb>2 || iDb!=0 ) z = sqlite3_mprintf("%s.%z", zDb, z);
    sqlite3ErrorMsg(pParse, "access to %z is prohibited", z);
    pParse->rc = SQLITE_AUTH;
  }မဟုတ်ပါက အကယ်၍( rc!=SQLITE_IGNORE && rc!=SQLITE_OK ){
    sqliteAuthBadReturnCode(pParse);
  }
  ပြန်ပေး rc;
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
void sqlite3AuthRead(
  Parse *pParse,        /* The parser context */
  Expr *pExpr,          /* The expression to check authorization on */
  Schema *pSchema,      /* The schema of the expression */
  SrcList *pTabList     /* All table that pExpr might refer to */
){
  Table *pTab = 0;      /* The table being read */
  const var_na *zCol;     /* Name of the column of the table */
  ကိန်းပြည့် iSrc;             /* Index in pTabList->a[] of table being read */
  ကိန်းပြည့် iDb;              /* The index of the database the expression refers to */
  ကိန်းပြည့် iCol;             /* Index of column in table */

  assert( pExpr->op==TK_COLUMN || pExpr->op==TK_TRIGGER );
  assert( !IN_RENAME_OBJECT );
  assert( pParse->db->xAuth!=0 );
  iDb = sqlite3SchemaToIndex(pParse->db, pSchema);
  အကယ်၍( iDb<0 ){
    /* An attempt to read a column out of a subquery or other
    ** temporary table. */
    ပြန်ပေး;
  }

  အကယ်၍( pExpr->op==TK_TRIGGER ){
    pTab = pParse->pTriggerTab;
  }မဟုတ်ပါက{
    assert( pTabList );
    အတွက်(iSrc=0; iSrc<pTabList->nSrc; iSrc++){
      အကယ်၍( pExpr->iTable==pTabList->a[iSrc].iCursor ){
        pTab = pTabList->a[iSrc].pSTab;
        bha_nga;
      }
    }
  }
  iCol = pExpr->iColumn;
  အကယ်၍( pTab==0 ) ပြန်ပေး;

  အကယ်၍( iCol>=0 ){
    assert( iCol<pTab->nCol );
    zCol = pTab->aCol[iCol].zCnName;
  }မဟုတ်ပါက အကယ်၍( pTab->iPKey>=0 ){
    assert( pTab->iPKey<pTab->nCol );
    zCol = pTab->aCol[pTab->iPKey].zCnName;
  }မဟုတ်ပါက{
    zCol = "ROWID";
  }
  assert( iDb>=0 && iDb<pParse->db->nDb );
  အကယ်၍( SQLITE_IGNORE==sqlite3AuthReadCol(pParse, pTab->zName, zCol, iDb) ){
    pExpr->op = TK_NULL;
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
static ကိန်းပြည့် SQLITE_NOINLINE realAuthCheck(
  Parse *pParse,
  ကိန်းပြည့် code,
  const var_na *zArg1,
  const var_na *zArg2,
  const var_na *zArg3
){
  sqlite3 *db = pParse->db;
  ကိန်းပြည့် rc;

  /* Don't do any authorization checks if the database is initializing
  ** or if the parser is being invoked from within sqlite3_declare_vtab.
  */
  assert( !IN_RENAME_OBJECT || db->xAuth==0 );
  အကယ်၍( IN_SPECIAL_PARSE ){
    ပြန်ပေး SQLITE_OK;
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

  rc = db->xAuth(db->pAuthArg,code,zArg1,zArg2,zArg3,pParse->zAuthContext);
  အကယ်၍( rc==SQLITE_DENY ){
    sqlite3ErrorMsg(pParse, "not authorized");
    pParse->rc = SQLITE_AUTH;
  }မဟုတ်ပါက အကယ်၍( rc!=SQLITE_OK && rc!=SQLITE_IGNORE ){
    rc = SQLITE_DENY;
    sqliteAuthBadReturnCode(pParse);
  }
  ပြန်ပေး rc;
}
ကိန်းပြည့် sqlite3AuthCheck(
  Parse *pParse,
  ကိန်းပြည့် code,
  const var_na *zArg1,
  const var_na *zArg2,
  const var_na *zArg3
){
  အကယ်၍( pParse->db->xAuth!=0 && pParse->db->init.busy==0 ){
    ပြန်ပေး realAuthCheck(pParse,code,zArg1,zArg2,zArg3);
  }မဟုတ်ပါက{
    ပြန်ပေး SQLITE_OK;
  }
}

/*
** Push an authorization context.  After this routine is called, the
** zArg3 argument to authorization callbacks will be zContext until
** popped.  Or if pParse==0, this routine is a no-op.
*/
void sqlite3AuthContextPush(
  Parse *pParse,
  AuthContext *pContext, 
  const var_na *zContext
){
  assert( pParse );
  pContext->pParse = pParse;
  pContext->zAuthContext = pParse->zAuthContext;
  pParse->zAuthContext = zContext;
}

/*
** Pop an authorization context that was previously pushed
** by sqlite3AuthContextPush
*/
void sqlite3AuthContextPop(AuthContext *pContext){
  အကယ်၍( pContext->pParse ){
    pContext->pParse->zAuthContext = pContext->zAuthContext;
    pContext->pParse = 0;
  }
}

#endif /* SQLITE_OMIT_AUTHORIZATION */
