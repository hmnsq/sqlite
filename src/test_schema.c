/*
** 2006 June 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the virtual table interfaces.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
*/

/* The code in this file defines a sqlite3 virtual-table module that
** provides a read-only view of the current database schema. There is one
** row in the schema table for each column in the database schema.
*/
#define SCHEMA \
"CREATE TABLE x("                                                            \
  "database,"          /* Name of database (i.e. main, temp etc.) */         \
  "tablename,"         /* Name of table */                                   \
  "cid,"               /* Column number (from left-to-right, 0 upward) */    \
  "name,"              /* Column name */                                     \
  "type,"              /* Specified type (i.e. VARCHAR(32)) */               \
  "not_null,"          /* Boolean. True if NOT NULL was specified */         \
  "dflt_value,"        /* Default value for this column */                   \
  "pk"                 /* True if this column is part of the primary key */  \
")"

/* If SQLITE_TEST is defined this code is preprocessed for use as part
** of the sqlite test binary "testfixture". Otherwise it is preprocessed
** to be compiled into an sqlite dynamic extension.
*/
#ifdef SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#không_thì
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct schema_vtab schema_vtab;
typedef struct schema_cursor schema_cursor;

/* A schema table object */
struct schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
struct schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  số_nguyên rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
static số_nguyên schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  trả_về 0;
}

/*
** Table constructor for the schema module.
*/
static số_nguyên schemaCreate(
  sqlite3 *db,
  void *pAux,
  số_nguyên argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  số_nguyên rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(sizeof(schema_vtab));
  nếu( pVtab ){
    memset(pVtab, 0, sizeof(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  trả_về rc;
}

/*
** Open a new cursor on the schema table.
*/
static số_nguyên schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  số_nguyên rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(sizeof(schema_cursor));
  nếu( pCur ){
    memset(pCur, 0, sizeof(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  trả_về rc;
}

/*
** Close a schema table cursor.
*/
static số_nguyên schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  trả_về SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
static số_nguyên schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, số_nguyên i){
  schema_cursor *pCur = (schema_cursor *)cur;
  switch( i ){
    case 0:
      sqlite3_result_value(ctx, sqlite3_column_value(pCur->pDbList, 1));
      break;
    case 1:
      sqlite3_result_value(ctx, sqlite3_column_value(pCur->pTableList, 0));
      break;
    default:
      sqlite3_result_value(ctx, sqlite3_column_value(pCur->pColumnList, i-2));
      break;
  }
  trả_về SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
static số_nguyên schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  trả_về SQLITE_OK;
}

static số_nguyên finalize(sqlite3_stmt **ppStmt){
  số_nguyên rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  trả_về rc;
}

static số_nguyên schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  trả_về (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
static số_nguyên schemaNext(sqlite3_vtab_cursor *cur){
  số_nguyên rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  char *zSql = 0;

  trong_khi( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    nếu( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) goto next_exit;

    trong_khi( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      nếu( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) goto next_exit;

      assert(pCur->pDbList);
      trong_khi( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        goto next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      nếu( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }không_thì{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      nếu( !zSql ){
        rc = SQLITE_NOMEM;
        goto next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      nếu( rc!=SQLITE_OK ) goto next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    nếu( !zSql ){
      rc = SQLITE_NOMEM;
      goto next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    nếu( rc!=SQLITE_OK ) goto next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  trả_về rc;
}

/*
** Reset a schema table cursor.
*/
static số_nguyên schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  số_nguyên idxNum, const char *idxStr,
  số_nguyên argc, sqlite3_value **argv
){
  số_nguyên rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  trả_về (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
static số_nguyên schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  trả_về SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
static sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schemaBestIndex,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schemaNext,                  /* xNext - advance a cursor */
  schemaEof,                   /* xEof */
  schemaColumn,                /* xColumn - read data */
  schemaRowid,                 /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  0,                           /* xFindMethod */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ifdef SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
extern số_nguyên getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
static số_nguyên SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  trả_về TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
số_nguyên Sqlitetestschema_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  số_nguyên i;
  cho(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  trả_về TCL_OK;
}

#không_thì

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
số_nguyên sqlite3_schema_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  trả_về 0;
}

#endif
