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
#تعریف_کړه SCHEMA \
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
#که_تعریف_شوی SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#وإلا
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#د_که_پای

#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>

تعريفنوع بنية schema_vtab schema_vtab;
تعريفنوع بنية schema_cursor schema_cursor;

/* A schema table object */
بنية schema_vtab {
  sqlite3_vtab بنسټ;
  sqlite3 *db;
};

/* A schema table cursor object */
بنية schema_cursor {
  sqlite3_vtab_cursor بنسټ;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *p_جدول_List;
  sqlite3_stmt *p_ستنه_List;
  صحيح rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
ساكن صحيح schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع 0;
}

/*
** Table constructor for the schema module.
*/
ساكن صحيح schemaCreate(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحيح rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(حجم(schema_vtab));
  إذا( pVtab ){
    memset(pVtab, 0, حجم(schema_vtab));
    pVtab->db = db;
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#د_که_پای
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  أرجع rc;
}

/*
** Open a new cursor on the schema table.
*/
ساكن صحيح schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  صحيح rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(حجم(schema_cursor));
  إذا( pCur ){
    memset(pCur, 0, حجم(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  أرجع rc;
}

/*
** Close a schema table cursor.
*/
ساكن صحيح schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->p_جدول_List);
  sqlite3_finalize(pCur->p_ستنه_List);
  sqlite3_free(pCur);
  أرجع SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ساكن صحيح schema_ستنه(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  schema_cursor *pCur = (schema_cursor *)cur;
  بدل( i ){
    حالة 0:
      sqlite3_پایله_ارزښت(ctx, sqlite3_ستنه_ارزښت(pCur->pDbList, 1));
      اكسر;
    حالة 1:
      sqlite3_پایله_ارزښت(ctx, sqlite3_ستنه_ارزښت(pCur->p_جدول_List, 0));
      اكسر;
    افتراضي:
      sqlite3_پایله_ارزښت(ctx, sqlite3_ستنه_ارزښت(pCur->p_ستنه_List, i-2));
      اكسر;
  }
  أرجع SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ساكن صحيح schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  أرجع SQLITE_OK;
}

ساكن صحيح finalize(sqlite3_stmt **ppStmt){
  صحيح rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  أرجع rc;
}

ساكن صحيح schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  أرجع (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
ساكن صحيح schema_بل(sqlite3_vtab_cursor *cur){
  صحيح rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  حرف *zSql = 0;

  طالما( !pCur->p_ستنه_List || SQLITE_لیکه!=sqlite3_ګام(pCur->p_ستنه_List) ){
    إذا( SQLITE_OK!=(rc = finalize(&pCur->p_ستنه_List)) ) اذهب بل_exit;

    طالما( !pCur->p_جدول_List || SQLITE_لیکه!=sqlite3_ګام(pCur->p_جدول_List) ){
      إذا( SQLITE_OK!=(rc = finalize(&pCur->p_جدول_List)) ) اذهب بل_exit;

      أكد(pCur->pDbList);
      طالما( SQLITE_لیکه!=sqlite3_ګام(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        اذهب بل_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      إذا( sqlite3_ستنه_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }وإلا{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_ستنه_متن(pDbList, 1)
        );
      }
      إذا( !zSql ){
        rc = SQLITE_NOMEM;
        اذهب بل_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_جدول_List, 0);
      sqlite3_free(zSql);
      إذا( rc!=SQLITE_OK ) اذهب بل_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_ستنه_متن(pCur->pDbList, 1),
        sqlite3_ستنه_متن(pCur->p_جدول_List, 0)
    );

    إذا( !zSql ){
      rc = SQLITE_NOMEM;
      اذهب بل_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_ستنه_List, 0);
    sqlite3_free(zSql);
    إذا( rc!=SQLITE_OK ) اذهب بل_exit;
  }
  pCur->rowid++;

بل_exit:
  /* TODO: Handle rc */
  أرجع rc;
}

/*
** Reset a schema table cursor.
*/
ساكن صحيح schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_ارزښت **argv
){
  صحيح rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->p_جدول_List);
  finalize(&pCur->p_ستنه_List);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  أرجع (rc==SQLITE_OK ? schema_بل(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
ساكن صحيح schema_غوره_شاخص(sqlite3_vtab *tab, sqlite3_شاخص_info *pIdxInfo){
  أرجع SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ساكن sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_غوره_شاخص,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_بل,                  /* xNext - advance a cursor */
  schemaEof,                   /* xEof */
  schema_ستنه,                /* xColumn - read data */
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

#د_که_پای /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#که_تعریف_شوی SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
خارجي صحيح get_Db_اشاره_ګر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI ثبتوونکی_schema_module(
  Client_معلومات client_معلومات, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#د_که_پای
  أرجع TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetestschema_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *client_معلومات;
  } aObjCmd[] = {
     { "register_schema_module", ثبتوونکی_schema_module, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_معلومات, 0);
  }
  أرجع TCL_OK;
}

#وإلا

/*
** Extension load function.
*/
#که_تعریف_شوی _WIN32
__declspec(dllexport)
#د_که_پای
صحيح sqlite3_schema_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#د_که_پای
  أرجع 0;
}

#د_که_پای
