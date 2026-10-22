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
#નહીંતર
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>

પ્રકારવ્યાખ્યા સંરચના schema_vtab schema_vtab;
પ્રકારવ્યાખ્યા સંરચના schema_cursor schema_cursor;

/* A schema table object */
સંરચના schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
સંરચના schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  પૂર્ણાંક rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
સ્થિર પૂર્ણાંક schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા 0;
}

/*
** Table constructor for the schema module.
*/
સ્થિર પૂર્ણાંક schemaCreate(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  પૂર્ણાંક rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(કદ(schema_vtab));
  જો( pVtab ){
    memset(pVtab, 0, કદ(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  પાછા rc;
}

/*
** Open a new cursor on the schema table.
*/
સ્થિર પૂર્ણાંક schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  પૂર્ણાંક rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(કદ(schema_cursor));
  જો( pCur ){
    memset(pCur, 0, કદ(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  પાછા rc;
}

/*
** Close a schema table cursor.
*/
સ્થિર પૂર્ણાંક schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  પાછા SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
સ્થિર પૂર્ણાંક schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  schema_cursor *pCur = (schema_cursor *)cur;
  પસંદગી( i ){
    સ્થિતિ 0:
      sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pCur->pDbList, 1));
      તોડો;
    સ્થિતિ 1:
      sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pCur->pTableList, 0));
      તોડો;
    મૂળભૂત:
      sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pCur->pColumnList, i-2));
      તોડો;
  }
  પાછા SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
સ્થિર પૂર્ણાંક schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક finalize(sqlite3_stmt **ppStmt){
  પૂર્ણાંક rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  પાછા rc;
}

સ્થિર પૂર્ણાંક schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  પાછા (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
સ્થિર પૂર્ણાંક schemaNext(sqlite3_vtab_cursor *cur){
  પૂર્ણાંક rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  અક્ષર *zSql = 0;

  જ્યારે( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    જો( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) જાઓ next_exit;

    જ્યારે( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      જો( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) જાઓ next_exit;

      પ્રતિજ્ઞા(pCur->pDbList);
      જ્યારે( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        જાઓ next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      જો( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }નહીંતર{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      જો( !zSql ){
        rc = SQLITE_NOMEM;
        જાઓ next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      જો( rc!=SQLITE_OK ) જાઓ next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    જો( !zSql ){
      rc = SQLITE_NOMEM;
      જાઓ next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    જો( rc!=SQLITE_OK ) જાઓ next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  પાછા rc;
}

/*
** Reset a schema table cursor.
*/
સ્થિર પૂર્ણાંક schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  પૂર્ણાંક rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  પાછા (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
સ્થિર પૂર્ણાંક schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પાછા SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
સ્થિર sqlite3_module schemaModule = {
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
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  પાછા TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetestschema_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  પાછા TCL_OK;
}

#નહીંતર

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
પૂર્ણાંક sqlite3_schema_init(
  sqlite3 *db, 
  અક્ષર **pz_Err_સંદેશ, 
  અચલ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  પાછા 0;
}

#endif
