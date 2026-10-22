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
#അല്ലെങ്കിൽ
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>

തരനിർവചനം ഘടന schema_vtab schema_vtab;
തരനിർവചനം ഘടന schema_cursor schema_cursor;

/* A schema table object */
ഘടന schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
ഘടന schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  പൂർണ്ണസംഖ്യ rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ 0;
}

/*
** Table constructor for the schema module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaCreate(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(വലുപ്പം(schema_vtab));
  എങ്കിൽ( pVtab ){
    memset(pVtab, 0, വലുപ്പം(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  തിരികെ rc;
}

/*
** Open a new cursor on the schema table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  പൂർണ്ണസംഖ്യ rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(വലുപ്പം(schema_cursor));
  എങ്കിൽ( pCur ){
    memset(pCur, 0, വലുപ്പം(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  തിരികെ rc;
}

/*
** Close a schema table cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  തിരികെ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  schema_cursor *pCur = (schema_cursor *)cur;
  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ 0:
      sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCur->pDbList, 1));
      നിർത്തുക;
    അവസ്ഥ 1:
      sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCur->pTableList, 0));
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCur->pColumnList, i-2));
      നിർത്തുക;
  }
  തിരികെ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ finalize(sqlite3_stmt **ppStmt){
  പൂർണ്ണസംഖ്യ rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  തിരികെ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaNext(sqlite3_vtab_cursor *cur){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  അക്ഷരം *zSql = 0;

  ആകുമ്പോൾ( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    എങ്കിൽ( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) പോകുക next_exit;

    ആകുമ്പോൾ( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      എങ്കിൽ( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) പോകുക next_exit;

      പ്രതിജ്ഞാ(pCur->pDbList);
      ആകുമ്പോൾ( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        പോകുക next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      എങ്കിൽ( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }അല്ലെങ്കിൽ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      എങ്കിൽ( !zSql ){
        rc = SQLITE_NOMEM;
        പോകുക next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      എങ്കിൽ( rc!=SQLITE_OK ) പോകുക next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    എങ്കിൽ( !zSql ){
      rc = SQLITE_NOMEM;
      പോകുക next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    എങ്കിൽ( rc!=SQLITE_OK ) പോകുക next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  തിരികെ rc;
}

/*
** Reset a schema table cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  തിരികെ (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  തിരികെ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
സ്ഥിതം sqlite3_module schemaModule = {
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
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  തിരികെ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetestschema_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  തിരികെ TCL_OK;
}

#അല്ലെങ്കിൽ

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
പൂർണ്ണസംഖ്യ sqlite3_schema_init(
  sqlite3 *db, 
  അക്ഷരം **pz_Err_സന്ദേശം, 
  സ്ഥിരം sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  തിരികെ 0;
}

#endif
