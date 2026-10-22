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
#இல்லையெனில்
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>

வகைவரை கட்டமைப்பு schema_vtab schema_vtab;
வகைவரை கட்டமைப்பு schema_cursor schema_cursor;

/* A schema table object */
கட்டமைப்பு schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
கட்டமைப்பு schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  முழு rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
நிலையான முழு schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு 0;
}

/*
** Table constructor for the schema module.
*/
நிலையான முழு schemaCreate(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  முழு rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(அளவு(schema_vtab));
  எனில்( pVtab ){
    memset(pVtab, 0, அளவு(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  திரும்பு rc;
}

/*
** Open a new cursor on the schema table.
*/
நிலையான முழு schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  முழு rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(அளவு(schema_cursor));
  எனில்( pCur ){
    memset(pCur, 0, அளவு(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  திரும்பு rc;
}

/*
** Close a schema table cursor.
*/
நிலையான முழு schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  திரும்பு SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
நிலையான முழு schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, முழு i){
  schema_cursor *pCur = (schema_cursor *)cur;
  தேர்வு( i ){
    நிலை 0:
      sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCur->pDbList, 1));
      நிறுத்து;
    நிலை 1:
      sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCur->pTableList, 0));
      நிறுத்து;
    இயல்பு:
      sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCur->pColumnList, i-2));
      நிறுத்து;
  }
  திரும்பு SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
நிலையான முழு schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  திரும்பு SQLITE_OK;
}

நிலையான முழு finalize(sqlite3_stmt **ppStmt){
  முழு rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  திரும்பு rc;
}

நிலையான முழு schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  திரும்பு (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
நிலையான முழு schemaNext(sqlite3_vtab_cursor *cur){
  முழு rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  எழுத்து *zSql = 0;

  போது( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    எனில்( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) செல் next_exit;

    போது( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      எனில்( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) செல் next_exit;

      ப்ரதிஜ்ஞா(pCur->pDbList);
      போது( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        செல் next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      எனில்( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }இல்லையெனில்{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      எனில்( !zSql ){
        rc = SQLITE_NOMEM;
        செல் next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      எனில்( rc!=SQLITE_OK ) செல் next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    எனில்( !zSql ){
      rc = SQLITE_NOMEM;
      செல் next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    எனில்( rc!=SQLITE_OK ) செல் next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  திரும்பு rc;
}

/*
** Reset a schema table cursor.
*/
நிலையான முழு schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  முழு rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  திரும்பு (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
நிலையான முழு schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  திரும்பு SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
நிலையான sqlite3_module schemaModule = {
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
வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
நிலையான முழு SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  திரும்பு TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetestschema_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     வெறுமை *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  திரும்பு TCL_OK;
}

#இல்லையெனில்

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
முழு sqlite3_schema_init(
  sqlite3 *db, 
  எழுத்து **pz_Err_செய்தி, 
  மாறிலி sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  திரும்பு 0;
}

#endif
