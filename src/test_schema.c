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
#ଅନ୍ୟଥା
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା schema_vtab schema_vtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା schema_cursor schema_cursor;

/* A schema table object */
ସଂରଚନା schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
ସଂରଚନା schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ 0;
}

/*
** Table constructor for the schema module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaCreate(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(ଆକାର(schema_vtab));
  ଯଦି( pVtab ){
    memset(pVtab, 0, ଆକାର(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ଫେରନ୍ତୁ rc;
}

/*
** Open a new cursor on the schema table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(ଆକାର(schema_cursor));
  ଯଦି( pCur ){
    memset(pCur, 0, ଆକାର(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Close a schema table cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  schema_cursor *pCur = (schema_cursor *)cur;
  ବାଛ( i ){
    ସ୍ଥିତି 0:
      sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pCur->pDbList, 1));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 1:
      sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pCur->pTableList, 0));
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pCur->pColumnList, i-2));
      ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା finalize(sqlite3_stmt **ppStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ଫେରନ୍ତୁ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaNext(sqlite3_vtab_cursor *cur){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  ଅକ୍ଷର *zSql = 0;

  ଯେତେବେଳେ( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    ଯଦି( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) ଯାଅ next_exit;

    ଯେତେବେଳେ( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      ଯଦି( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) ଯାଅ next_exit;

      ପ୍ରତିଜ୍ଞା(pCur->pDbList);
      ଯେତେବେଳେ( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        ଯାଅ next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ଯଦି( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ଅନ୍ୟଥା{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      ଯଦି( !zSql ){
        rc = SQLITE_NOMEM;
        ଯାଅ next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      ଯଦି( rc!=SQLITE_OK ) ଯାଅ next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    ଯଦି( !zSql ){
      rc = SQLITE_NOMEM;
      ଯାଅ next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    ଯଦି( rc!=SQLITE_OK ) ଯାଅ next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  ଫେରନ୍ତୁ rc;
}

/*
** Reset a schema table cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ଫେରନ୍ତୁ (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ସ୍ଥିର sqlite3_module schemaModule = {
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
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetestschema_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ଶୂନ୍ୟ *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

#ଅନ୍ୟଥା

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_schema_init(
  sqlite3 *db, 
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ, 
  ଅଚଳ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ଫେରନ୍ତୁ 0;
}

#endif
