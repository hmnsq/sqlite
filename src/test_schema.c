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
#অন্যথা
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>

প্রকারনির্দেশ সংরচনা schema_vtab schema_vtab;
প্রকারনির্দেশ সংরচনা schema_cursor schema_cursor;

/* A schema table object */
সংরচনা schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
সংরচনা schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  পূর্ণাঙ্ক rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
স্থির পূর্ণাঙ্ক schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  প্রত্যাগম 0;
}

/*
** Table constructor for the schema module.
*/
স্থির পূর্ণাঙ্ক schemaCreate(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুব বর্ণ *ধ্রুব*argv,
  sqlite3_vtab **ppVtab,
  বর্ণ **pzErr
){
  পূর্ণাঙ্ক rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(পরিমাণ(schema_vtab));
  যদি( pVtab ){
    memset(pVtab, 0, পরিমাণ(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  প্রত্যাগম rc;
}

/*
** Open a new cursor on the schema table.
*/
স্থির পূর্ণাঙ্ক schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  পূর্ণাঙ্ক rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(পরিমাণ(schema_cursor));
  যদি( pCur ){
    memset(pCur, 0, পরিমাণ(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  প্রত্যাগম rc;
}

/*
** Close a schema table cursor.
*/
স্থির পূর্ণাঙ্ক schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  প্রত্যাগম SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
স্থির পূর্ণাঙ্ক schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণাঙ্ক i){
  schema_cursor *pCur = (schema_cursor *)cur;
  বিকল্প( i ){
    পক্ষ 0:
      sqlite3_result_মান(ctx, sqlite3_column_মান(pCur->pDbList, 1));
      ভঙ্গ;
    পক্ষ 1:
      sqlite3_result_মান(ctx, sqlite3_column_মান(pCur->pTableList, 0));
      ভঙ্গ;
    স্বভাব:
      sqlite3_result_মান(ctx, sqlite3_column_মান(pCur->pColumnList, i-2));
      ভঙ্গ;
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
স্থির পূর্ণাঙ্ক schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  প্রত্যাগম SQLITE_OK;
}

স্থির পূর্ণাঙ্ক finalize(sqlite3_stmt **ppStmt){
  পূর্ণাঙ্ক rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  প্রত্যাগম rc;
}

স্থির পূর্ণাঙ্ক schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  প্রত্যাগম (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
স্থির পূর্ণাঙ্ক schemaNext(sqlite3_vtab_cursor *cur){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  বর্ণ *zSql = 0;

  যাবত্( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    যদি( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) গচ্ছ next_exit;

    যাবত্( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      যদি( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) গচ্ছ next_exit;

      প্রতিজ্ঞা(pCur->pDbList);
      যাবত্( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        গচ্ছ next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      যদি( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }অন্যথা{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      যদি( !zSql ){
        rc = SQLITE_NOMEM;
        গচ্ছ next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      যদি( rc!=SQLITE_OK ) গচ্ছ next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    যদি( !zSql ){
      rc = SQLITE_NOMEM;
      গচ্ছ next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    যদি( rc!=SQLITE_OK ) গচ্ছ next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  প্রত্যাগম rc;
}

/*
** Reset a schema table cursor.
*/
স্থির পূর্ণাঙ্ক schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুব বর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  পূর্ণাঙ্ক rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  প্রত্যাগম (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
স্থির পূর্ণাঙ্ক schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  প্রত্যাগম SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
স্থির sqlite3_module schemaModule = {
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
বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুব বর্ণ *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) প্রত্যাগম TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  প্রত্যাগম TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetestschema_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     বর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  পূর্ণাঙ্ক i;
  প্রতি(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  প্রত্যাগম TCL_OK;
}

#অন্যথা

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
পূর্ণাঙ্ক sqlite3_schema_init(
  sqlite3 *db, 
  বর্ণ **pz_Err_বার্তা, 
  ধ্রুব sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  প্রত্যাগম 0;
}

#endif
