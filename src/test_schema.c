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
#ವ್ಯಾಖ್ಯಾನಿಸು SCHEMA \
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#ಇಲ್ಲದಿದ್ದರೆ
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ schema_vtab schema_vtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ schema_cursor schema_cursor;

/* A schema table object */
ರಚನೆ schema_vtab {
  sqlite3_vtab ತಳ;
  sqlite3 *db;
};

/* A schema table cursor object */
ರಚನೆ schema_cursor {
  sqlite3_vtab_cursor ತಳ;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *p_ಕೋಷ್ಟಕ_List;
  sqlite3_stmt *p_ಸ್ತಂಭ_List;
  ಪೂರ್ಣಾಂಕ rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Table constructor for the schema module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaCreate(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(ಗಾತ್ರ(schema_vtab));
  ಆದರೆ( pVtab ){
    memset(pVtab, 0, ಗಾತ್ರ(schema_vtab));
    pVtab->db = db;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#ಆದರೆ_ಅಂತ್ಯ
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Open a new cursor on the schema table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(ಗಾತ್ರ(schema_cursor));
  ಆದರೆ( pCur ){
    memset(pCur, 0, ಗಾತ್ರ(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close a schema table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->p_ಕೋಷ್ಟಕ_List);
  sqlite3_finalize(pCur->p_ಸ್ತಂಭ_List);
  sqlite3_free(pCur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schema_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  schema_cursor *pCur = (schema_cursor *)cur;
  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ 0:
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCur->pDbList, 1));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1:
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCur->p_ಕೋಷ್ಟಕ_List, 0));
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCur->p_ಸ್ತಂಭ_List, i-2));
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ finalize(sqlite3_stmt **ppStmt){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  ಹಿಂತಿರುಗಿ (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schema_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  ಅಕ್ಷರ *zSql = 0;

  ಆಗ( !pCur->p_ಸ್ತಂಭ_List || SQLITE_ಅಡ್ಡಸಾಲು!=sqlite3_ಹೆಜ್ಜೆ(pCur->p_ಸ್ತಂಭ_List) ){
    ಆದರೆ( SQLITE_OK!=(rc = finalize(&pCur->p_ಸ್ತಂಭ_List)) ) ಹೋಗು ಮುಂದಿನ_exit;

    ಆಗ( !pCur->p_ಕೋಷ್ಟಕ_List || SQLITE_ಅಡ್ಡಸಾಲು!=sqlite3_ಹೆಜ್ಜೆ(pCur->p_ಕೋಷ್ಟಕ_List) ){
      ಆದರೆ( SQLITE_OK!=(rc = finalize(&pCur->p_ಕೋಷ್ಟಕ_List)) ) ಹೋಗು ಮುಂದಿನ_exit;

      ಪ್ರತಿಜ್ಞಾ(pCur->pDbList);
      ಆಗ( SQLITE_ಅಡ್ಡಸಾಲು!=sqlite3_ಹೆಜ್ಜೆ(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        ಹೋಗು ಮುಂದಿನ_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ಆದರೆ( sqlite3_ಸ್ತಂಭ_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pDbList, 1)
        );
      }
      ಆದರೆ( !zSql ){
        rc = SQLITE_NOMEM;
        ಹೋಗು ಮುಂದಿನ_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_ಕೋಷ್ಟಕ_List, 0);
      sqlite3_free(zSql);
      ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಮುಂದಿನ_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pCur->pDbList, 1),
        sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pCur->p_ಕೋಷ್ಟಕ_List, 0)
    );

    ಆದರೆ( !zSql ){
      rc = SQLITE_NOMEM;
      ಹೋಗು ಮುಂದಿನ_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_ಸ್ತಂಭ_List, 0);
    sqlite3_free(zSql);
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಮುಂದಿನ_exit;
  }
  pCur->rowid++;

ಮುಂದಿನ_exit:
  /* TODO: Handle rc */
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Reset a schema table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->p_ಕೋಷ್ಟಕ_List);
  finalize(&pCur->p_ಸ್ತಂಭ_List);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  ಹಿಂತಿರುಗಿ (rc==SQLITE_OK ? schema_ಮುಂದಿನ(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ schema_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ಸ್ಥಿರ sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_ಮುಂದಿನ,                  /* xNext - advance a cursor */
  schemaEof,                   /* xEof */
  schema_ಸ್ತಂಭ,                /* xColumn - read data */
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

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_schema_module(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetestschema_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "register_schema_module", ರಿಜಿಸ್ಟರ್_schema_module, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಇಲ್ಲದಿದ್ದರೆ

/*
** Extension load function.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
__declspec(dllexport)
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_schema_init(
  sqlite3 *db, 
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, 
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ
