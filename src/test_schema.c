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
#ಅನ್ಯಥಾ
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ schema_vtab schema_vtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ schema_cursor schema_cursor;

/* A schema table object */
ಸಂರಚನಾ schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
ಸಂರಚನಾ schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  ಪೂರ್ಣಾಙ್ಕ rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Table constructor for the schema module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaCreate(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(ಪರಿಮಾಣ(schema_vtab));
  ಯದಿ( pVtab ){
    memset(pVtab, 0, ಪರಿಮಾಣ(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Open a new cursor on the schema table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(ಪರಿಮಾಣ(schema_cursor));
  ಯದಿ( pCur ){
    memset(pCur, 0, ಪರಿಮಾಣ(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Close a schema table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  schema_cursor *pCur = (schema_cursor *)cur;
  ವಿಕಲ್ಪ( i ){
    ಪಕ್ಷ 0:
      sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCur->pDbList, 1));
      ಭಙ್ಗ;
    ಪಕ್ಷ 1:
      sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCur->pTableList, 0));
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCur->pColumnList, i-2));
      ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ finalize(sqlite3_stmt **ppStmt){
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ಪ್ರತ್ಯಾಗಮ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaNext(sqlite3_vtab_cursor *cur){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  ವರ್ಣ *zSql = 0;

  ಯಾವತ್( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    ಯದಿ( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) ಗಚ್ಛ next_exit;

    ಯಾವತ್( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      ಯದಿ( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) ಗಚ್ಛ next_exit;

      ಪ್ರತಿಜ್ಞಾ(pCur->pDbList);
      ಯಾವತ್( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        ಗಚ್ಛ next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ಯದಿ( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ಅನ್ಯಥಾ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      ಯದಿ( !zSql ){
        rc = SQLITE_NOMEM;
        ಗಚ್ಛ next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    ಯದಿ( !zSql ){
      rc = SQLITE_NOMEM;
      ಗಚ್ಛ next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Reset a schema table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ಪ್ರತ್ಯಾಗಮ (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ಸ್ಥಿರ sqlite3_module schemaModule = {
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetestschema_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#ಅನ್ಯಥಾ

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ಪೂರ್ಣಾಙ್ಕ sqlite3_schema_init(
  sqlite3 *db, 
  ವರ್ಣ **pz_Err_ಸಂದೇಶ, 
  ಧ್ರುವ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ಪ್ರತ್ಯಾಗಮ 0;
}

#endif
