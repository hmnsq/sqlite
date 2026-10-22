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

প্রকারলেখ কাঠামো schema_vtab schema_vtab;
প্রকারলেখ কাঠামো schema_cursor schema_cursor;

/* A schema table object */
কাঠামো schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
কাঠামো schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  পূর্ণসংখ্যা rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
স্থির পূর্ণসংখ্যা schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত 0;
}

/*
** Table constructor for the schema module.
*/
স্থির পূর্ণসংখ্যা schemaCreate(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  পূর্ণসংখ্যা rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(আকার(schema_vtab));
  যদি( pVtab ){
    memset(pVtab, 0, আকার(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ফেরত rc;
}

/*
** Open a new cursor on the schema table.
*/
স্থির পূর্ণসংখ্যা schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  পূর্ণসংখ্যা rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(আকার(schema_cursor));
  যদি( pCur ){
    memset(pCur, 0, আকার(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ফেরত rc;
}

/*
** Close a schema table cursor.
*/
স্থির পূর্ণসংখ্যা schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  ফেরত SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
স্থির পূর্ণসংখ্যা schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  schema_cursor *pCur = (schema_cursor *)cur;
  নির্বাচন( i ){
    ক্ষেত্র 0:
      sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCur->pDbList, 1));
      বিরতি;
    ক্ষেত্র 1:
      sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCur->pTableList, 0));
      বিরতি;
    পূর্বনির্ধারিত:
      sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCur->pColumnList, i-2));
      বিরতি;
  }
  ফেরত SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
স্থির পূর্ণসংখ্যা schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা finalize(sqlite3_stmt **ppStmt){
  পূর্ণসংখ্যা rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ফেরত (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
স্থির পূর্ণসংখ্যা schema_পরবর্তী(sqlite3_vtab_cursor *cur){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  অক্ষর *zSql = 0;

  যখন( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    যদি( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) যাও পরবর্তী_exit;

    যখন( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      যদি( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) যাও পরবর্তী_exit;

      প্রতিজ্ঞা(pCur->pDbList);
      যখন( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        যাও পরবর্তী_exit;
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
        যাও পরবর্তী_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      যদি( rc!=SQLITE_OK ) যাও পরবর্তী_exit;
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
      যাও পরবর্তী_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    যদি( rc!=SQLITE_OK ) যাও পরবর্তী_exit;
  }
  pCur->rowid++;

পরবর্তী_exit:
  /* TODO: Handle rc */
  ফেরত rc;
}

/*
** Reset a schema table cursor.
*/
স্থির পূর্ণসংখ্যা schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  পূর্ণসংখ্যা rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ফেরত (rc==SQLITE_OK ? schema_পরবর্তী(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
স্থির পূর্ণসংখ্যা schema_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  ফেরত SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
স্থির sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_Best_সূচক,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_পরবর্তী,                  /* xNext - advance a cursor */
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
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_schema_module(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ফেরত TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetestschema_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
  ফেরত TCL_OK;
}

#অন্যথা

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
পূর্ণসংখ্যা sqlite3_schema_init(
  sqlite3 *db, 
  অক্ষর **pz_Err_বার্তা, 
  ধ্রুবক sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  ফেরত 0;
}

#endif
