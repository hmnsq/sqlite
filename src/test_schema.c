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
#ບໍ່ດັ່ງນັ້ນ
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
  ຈຳນວນເຕັມ rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
static ຈຳນວນເຕັມ schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ສົ່ງຄືນ 0;
}

/*
** Table constructor for the schema module.
*/
static ຈຳນວນເຕັມ schemaCreate(
  sqlite3 *db,
  void *pAux,
  ຈຳນວນເຕັມ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  ຈຳນວນເຕັມ rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(parimaa_na(schema_vtab));
  ຖ້າ( pVtab ){
    memset(pVtab, 0, parimaa_na(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ສົ່ງຄືນ rc;
}

/*
** Open a new cursor on the schema table.
*/
static ຈຳນວນເຕັມ schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ຈຳນວນເຕັມ rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(parimaa_na(schema_cursor));
  ຖ້າ( pCur ){
    memset(pCur, 0, parimaa_na(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ສົ່ງຄືນ rc;
}

/*
** Close a schema table cursor.
*/
static ຈຳນວນເຕັມ schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
static ຈຳນວນເຕັມ schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ຈຳນວນເຕັມ i){
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
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
static ຈຳນວນເຕັມ schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ສົ່ງຄືນ SQLITE_OK;
}

static ຈຳນວນເຕັມ finalize(sqlite3_stmt **ppStmt){
  ຈຳນວນເຕັມ rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ສົ່ງຄືນ rc;
}

static ຈຳນວນເຕັມ schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ສົ່ງຄືນ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
static ຈຳນວນເຕັມ schemaNext(sqlite3_vtab_cursor *cur){
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  var_na *zSql = 0;

  ໃນຂະນະທີ່( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    ຖ້າ( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) goto next_exit;

    ໃນຂະນະທີ່( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      ຖ້າ( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) goto next_exit;

      assert(pCur->pDbList);
      ໃນຂະນະທີ່( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        goto next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ຖ້າ( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ບໍ່ດັ່ງນັ້ນ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      ຖ້າ( !zSql ){
        rc = SQLITE_NOMEM;
        goto next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      ຖ້າ( rc!=SQLITE_OK ) goto next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    ຖ້າ( !zSql ){
      rc = SQLITE_NOMEM;
      goto next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    ຖ້າ( rc!=SQLITE_OK ) goto next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  ສົ່ງຄືນ rc;
}

/*
** Reset a schema table cursor.
*/
static ຈຳນວນເຕັມ schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ຈຳນວນເຕັມ idxNum, const var_na *idxStr,
  ຈຳນວນເຕັມ argc, sqlite3_value **argv
){
  ຈຳນວນເຕັມ rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ສົ່ງຄືນ (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
static ຈຳນວນເຕັມ schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ສົ່ງຄືນ SQLITE_OK;
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
extern ຈຳນວນເຕັມ getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
static ຈຳນວນເຕັມ SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ຈຳນວນເຕັມ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ຖ້າ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ສົ່ງຄືນ TCL_ERROR;
  }
  ຖ້າ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ສົ່ງຄືນ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ສົ່ງຄືນ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ຈຳນວນເຕັມ Sqlitetestschema_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  ຈຳນວນເຕັມ i;
  ສຳລັບ(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ສົ່ງຄືນ TCL_OK;
}

#ບໍ່ດັ່ງນັ້ນ

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ຈຳນວນເຕັມ sqlite3_schema_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ສົ່ງຄືນ 0;
}

#endif
