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
#ᰀᰧᰀ
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef sa_nracanaa schema_vtab schema_vtab;
typedef sa_nracanaa schema_cursor schema_cursor;

/* A schema table object */
sa_nracanaa schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
sa_nracanaa schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  puur_naa_nka rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
static puur_naa_nka schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ᰀᰧᰌ 0;
}

/*
** Table constructor for the schema module.
*/
static puur_naa_nka schemaCreate(
  sqlite3 *db,
  void *pAux,
  puur_naa_nka argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  puur_naa_nka rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(parimaa_na(schema_vtab));
  ᰀᰧ( pVtab ){
    memset(pVtab, 0, parimaa_na(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ᰀᰧᰌ rc;
}

/*
** Open a new cursor on the schema table.
*/
static puur_naa_nka schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  puur_naa_nka rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(parimaa_na(schema_cursor));
  ᰀᰧ( pCur ){
    memset(pCur, 0, parimaa_na(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ᰀᰧᰌ rc;
}

/*
** Close a schema table cursor.
*/
static puur_naa_nka schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  ᰀᰧᰌ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
static puur_naa_nka schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, puur_naa_nka i){
  schema_cursor *pCur = (schema_cursor *)cur;
  switch( i ){
    pak_sa 0:
      sqlite3_result_value(ctx, sqlite3_column_value(pCur->pDbList, 1));
      bha_nga;
    pak_sa 1:
      sqlite3_result_value(ctx, sqlite3_column_value(pCur->pTableList, 0));
      bha_nga;
    default:
      sqlite3_result_value(ctx, sqlite3_column_value(pCur->pColumnList, i-2));
      bha_nga;
  }
  ᰀᰧᰌ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
static puur_naa_nka schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ᰀᰧᰌ SQLITE_OK;
}

static puur_naa_nka finalize(sqlite3_stmt **ppStmt){
  puur_naa_nka rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ᰀᰧᰌ rc;
}

static puur_naa_nka schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ᰀᰧᰌ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
static puur_naa_nka schemaNext(sqlite3_vtab_cursor *cur){
  puur_naa_nka rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  var_na *zSql = 0;

  ᰀᰬ( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    ᰀᰧ( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) goto next_exit;

    ᰀᰬ( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      ᰀᰧ( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) goto next_exit;

      assert(pCur->pDbList);
      ᰀᰬ( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        goto next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ᰀᰧ( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ᰀᰧᰀ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      ᰀᰧ( !zSql ){
        rc = SQLITE_NOMEM;
        goto next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      ᰀᰧ( rc!=SQLITE_OK ) goto next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    ᰀᰧ( !zSql ){
      rc = SQLITE_NOMEM;
      goto next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    ᰀᰧ( rc!=SQLITE_OK ) goto next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  ᰀᰧᰌ rc;
}

/*
** Reset a schema table cursor.
*/
static puur_naa_nka schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  puur_naa_nka idxNum, const var_na *idxStr,
  puur_naa_nka argc, sqlite3_value **argv
){
  puur_naa_nka rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ᰀᰧᰌ (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
static puur_naa_nka schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ᰀᰧᰌ SQLITE_OK;
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
extern puur_naa_nka getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
static puur_naa_nka SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ᰀᰧᰌ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ᰀᰧᰌ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
puur_naa_nka Sqlitetestschema_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  puur_naa_nka i;
  for(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ᰀᰧᰌ TCL_OK;
}

#ᰀᰧᰀ

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
puur_naa_nka sqlite3_schema_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ᰀᰧᰌ 0;
}

#endif
