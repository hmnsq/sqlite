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
#تعریف_کرو SCHEMA \
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
#اگر_تعریف_شدہ SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#ورنہ
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#اگر_ختم

#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>

قسمتعریف ساخت schema_vtab schema_vtab;
قسمتعریف ساخت schema_cursor schema_cursor;

/* A schema table object */
ساخت schema_vtab {
  sqlite3_vtab بنیاد;
  sqlite3 *db;
};

/* A schema table cursor object */
ساخت schema_cursor {
  sqlite3_vtab_cursor بنیاد;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *p_جدول_List;
  sqlite3_stmt *p_عمود_List;
  صحیح rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
ساکن صحیح schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس 0;
}

/*
** Table constructor for the schema module.
*/
ساکن صحیح schemaCreate(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحیح rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(حجم(schema_vtab));
  اگر( pVtab ){
    memset(pVtab, 0, حجم(schema_vtab));
    pVtab->db = db;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#اگر_ختم
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  واپس rc;
}

/*
** Open a new cursor on the schema table.
*/
ساکن صحیح schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  صحیح rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(حجم(schema_cursor));
  اگر( pCur ){
    memset(pCur, 0, حجم(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  واپس rc;
}

/*
** Close a schema table cursor.
*/
ساکن صحیح schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->p_جدول_List);
  sqlite3_finalize(pCur->p_عمود_List);
  sqlite3_free(pCur);
  واپس SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ساکن صحیح schema_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  schema_cursor *pCur = (schema_cursor *)cur;
  انتخاب( i ){
    حالت 0:
      sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pCur->pDbList, 1));
      توڑو;
    حالت 1:
      sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pCur->p_جدول_List, 0));
      توڑو;
    طےشدہ:
      sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pCur->p_عمود_List, i-2));
      توڑو;
  }
  واپس SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ساکن صحیح schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  واپس SQLITE_OK;
}

ساکن صحیح finalize(sqlite3_stmt **ppStmt){
  صحیح rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  واپس rc;
}

ساکن صحیح schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  واپس (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
ساکن صحیح schema_اگلا(sqlite3_vtab_cursor *cur){
  صحیح rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  حرف *zSql = 0;

  جب( !pCur->p_عمود_List || SQLITE_صف!=sqlite3_قدم(pCur->p_عمود_List) ){
    اگر( SQLITE_OK!=(rc = finalize(&pCur->p_عمود_List)) ) جاؤ اگلا_exit;

    جب( !pCur->p_جدول_List || SQLITE_صف!=sqlite3_قدم(pCur->p_جدول_List) ){
      اگر( SQLITE_OK!=(rc = finalize(&pCur->p_جدول_List)) ) جاؤ اگلا_exit;

      أكد(pCur->pDbList);
      جب( SQLITE_صف!=sqlite3_قدم(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        جاؤ اگلا_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      اگر( sqlite3_عمود_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ورنہ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_عمود_متن(pDbList, 1)
        );
      }
      اگر( !zSql ){
        rc = SQLITE_NOMEM;
        جاؤ اگلا_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_جدول_List, 0);
      sqlite3_free(zSql);
      اگر( rc!=SQLITE_OK ) جاؤ اگلا_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_عمود_متن(pCur->pDbList, 1),
        sqlite3_عمود_متن(pCur->p_جدول_List, 0)
    );

    اگر( !zSql ){
      rc = SQLITE_NOMEM;
      جاؤ اگلا_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_عمود_List, 0);
    sqlite3_free(zSql);
    اگر( rc!=SQLITE_OK ) جاؤ اگلا_exit;
  }
  pCur->rowid++;

اگلا_exit:
  /* TODO: Handle rc */
  واپس rc;
}

/*
** Reset a schema table cursor.
*/
ساکن صحیح schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  صحیح rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->p_جدول_List);
  finalize(&pCur->p_عمود_List);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  واپس (rc==SQLITE_OK ? schema_اگلا(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
ساکن صحیح schema_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  واپس SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ساکن sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_بہترین_اشاریہ,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_اگلا,                  /* xNext - advance a cursor */
  schemaEof,                   /* xEof */
  schema_عمود,                /* xColumn - read data */
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

#اگر_ختم /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#اگر_تعریف_شدہ SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
ساکن صحیح SQLITE_TCLAPI رجسٹر_schema_module(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#اگر_ختم
  واپس TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetestschema_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "register_schema_module", رجسٹر_schema_module, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
  واپس TCL_OK;
}

#ورنہ

/*
** Extension load function.
*/
#اگر_تعریف_شدہ _WIN32
__declspec(dllexport)
#اگر_ختم
صحیح sqlite3_schema_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#اگر_ختم
  واپس 0;
}

#اگر_ختم
