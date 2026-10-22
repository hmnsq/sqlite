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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCHEMA \
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#ਨਹੀਂਤਾਂ
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ schema_vtab schema_vtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ schema_cursor schema_cursor;

/* A schema table object */
ਬਣਤਰ schema_vtab {
  sqlite3_vtab ਅਧਾਰ;
  sqlite3 *db;
};

/* A schema table cursor object */
ਬਣਤਰ schema_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *p_ਸਾਰਣੀ_List;
  sqlite3_stmt *p_ਕਾਲਮ_List;
  ਪੂਰਨ rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
ਸਥਿਰ ਪੂਰਨ schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ 0;
}

/*
** Table constructor for the schema module.
*/
ਸਥਿਰ ਪੂਰਨ schemaCreate(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  ਪੂਰਨ rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(ਆਕਾਰ(schema_vtab));
  ਜੇ( pVtab ){
    memset(pVtab, 0, ਆਕਾਰ(schema_vtab));
    pVtab->db = db;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#ਜੇ_ਅੰਤ
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ਵਾਪਸ rc;
}

/*
** Open a new cursor on the schema table.
*/
ਸਥਿਰ ਪੂਰਨ schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ਪੂਰਨ rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(ਆਕਾਰ(schema_cursor));
  ਜੇ( pCur ){
    memset(pCur, 0, ਆਕਾਰ(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ਵਾਪਸ rc;
}

/*
** Close a schema table cursor.
*/
ਸਥਿਰ ਪੂਰਨ schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->p_ਸਾਰਣੀ_List);
  sqlite3_finalize(pCur->p_ਕਾਲਮ_List);
  sqlite3_free(pCur);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ਸਥਿਰ ਪੂਰਨ schema_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  schema_cursor *pCur = (schema_cursor *)cur;
  ਚੋਣ( i ){
    ਹਾਲਤ 0:
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCur->pDbList, 1));
      ਰੋਕੋ;
    ਹਾਲਤ 1:
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCur->p_ਸਾਰਣੀ_List, 0));
      ਰੋਕੋ;
    ਮੂਲ:
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCur->p_ਕਾਲਮ_List, i-2));
      ਰੋਕੋ;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ਸਥਿਰ ਪੂਰਨ schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ finalize(sqlite3_stmt **ppStmt){
  ਪੂਰਨ rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ਵਾਪਸ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
ਸਥਿਰ ਪੂਰਨ schema_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  ਪੂਰਨ rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  ਅੱਖਰ *zSql = 0;

  ਜਦੋਂ( !pCur->p_ਕਾਲਮ_List || SQLITE_ਪੰਕਤੀ!=sqlite3_ਕਦਮ(pCur->p_ਕਾਲਮ_List) ){
    ਜੇ( SQLITE_OK!=(rc = finalize(&pCur->p_ਕਾਲਮ_List)) ) ਜਾਓ ਅਗਲਾ_exit;

    ਜਦੋਂ( !pCur->p_ਸਾਰਣੀ_List || SQLITE_ਪੰਕਤੀ!=sqlite3_ਕਦਮ(pCur->p_ਸਾਰਣੀ_List) ){
      ਜੇ( SQLITE_OK!=(rc = finalize(&pCur->p_ਸਾਰਣੀ_List)) ) ਜਾਓ ਅਗਲਾ_exit;

      ਪ੍ਰਤਿਜ੍ਞਾ(pCur->pDbList);
      ਜਦੋਂ( SQLITE_ਪੰਕਤੀ!=sqlite3_ਕਦਮ(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        ਜਾਓ ਅਗਲਾ_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ਜੇ( sqlite3_ਕਾਲਮ_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ਨਹੀਂਤਾਂ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_ਕਾਲਮ_ਪਾਠ(pDbList, 1)
        );
      }
      ਜੇ( !zSql ){
        rc = SQLITE_NOMEM;
        ਜਾਓ ਅਗਲਾ_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_ਸਾਰਣੀ_List, 0);
      sqlite3_free(zSql);
      ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਅਗਲਾ_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_ਕਾਲਮ_ਪਾਠ(pCur->pDbList, 1),
        sqlite3_ਕਾਲਮ_ਪਾਠ(pCur->p_ਸਾਰਣੀ_List, 0)
    );

    ਜੇ( !zSql ){
      rc = SQLITE_NOMEM;
      ਜਾਓ ਅਗਲਾ_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_ਕਾਲਮ_List, 0);
    sqlite3_free(zSql);
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਅਗਲਾ_exit;
  }
  pCur->rowid++;

ਅਗਲਾ_exit:
  /* TODO: Handle rc */
  ਵਾਪਸ rc;
}

/*
** Reset a schema table cursor.
*/
ਸਥਿਰ ਪੂਰਨ schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->p_ਸਾਰਣੀ_List);
  finalize(&pCur->p_ਕਾਲਮ_List);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ਵਾਪਸ (rc==SQLITE_OK ? schema_ਅਗਲਾ(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
ਸਥਿਰ ਪੂਰਨ schema_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਵਾਪਸ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ਸਥਿਰ sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_ਅਗਲਾ,                  /* xNext - advance a cursor */
  schemaEof,                   /* xEof */
  schema_ਕਾਲਮ,                /* xColumn - read data */
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

#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_schema_module(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetestschema_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "register_schema_module", ਰਜਿਸਟਰ_schema_module, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
  ਵਾਪਸ TCL_OK;
}

#ਨਹੀਂਤਾਂ

/*
** Extension load function.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
__declspec(dllexport)
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_schema_init(
  sqlite3 *db, 
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, 
  ਅਚਲ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ
