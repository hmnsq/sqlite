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
#अण्णहा
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#endif

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

प्रकारनिर्देश संरचना schema_vtab schema_vtab;
प्रकारनिर्देश संरचना schema_cursor schema_cursor;

/* A schema table object */
संरचना schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
संरचना schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  पूर्णाङ्क rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
स्थिर पूर्णाङ्क schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम 0;
}

/*
** Table constructor for the schema module.
*/
स्थिर पूर्णाङ्क schemaCreate(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  पूर्णाङ्क rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(परिमाण(schema_vtab));
  जइ( pVtab ){
    memset(pVtab, 0, परिमाण(schema_vtab));
    pVtab->db = db;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#endif
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  प्रत्यागम rc;
}

/*
** Open a new cursor on the schema table.
*/
स्थिर पूर्णाङ्क schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  पूर्णाङ्क rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(परिमाण(schema_cursor));
  जइ( pCur ){
    memset(pCur, 0, परिमाण(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  प्रत्यागम rc;
}

/*
** Close a schema table cursor.
*/
स्थिर पूर्णाङ्क schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  प्रत्यागम SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
स्थिर पूर्णाङ्क schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  schema_cursor *pCur = (schema_cursor *)cur;
  विकल्प( i ){
    पक्ष 0:
      sqlite3_result_मानम्(ctx, sqlite3_column_मानम्(pCur->pDbList, 1));
      भङ्ग;
    पक्ष 1:
      sqlite3_result_मानम्(ctx, sqlite3_column_मानम्(pCur->pTableList, 0));
      भङ्ग;
    स्वभाव:
      sqlite3_result_मानम्(ctx, sqlite3_column_मानम्(pCur->pColumnList, i-2));
      भङ्ग;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
स्थिर पूर्णाङ्क schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क finalize(sqlite3_stmt **ppStmt){
  पूर्णाङ्क rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  प्रत्यागम (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
स्थिर पूर्णाङ्क schemaNext(sqlite3_vtab_cursor *cur){
  पूर्णाङ्क rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  वर्ण *zSql = 0;

  जाव( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    जइ( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) गच्छ next_exit;

    जाव( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      जइ( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) गच्छ next_exit;

      प्रतिज्ञा(pCur->pDbList);
      जाव( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        गच्छ next_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      जइ( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }अण्णहा{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      जइ( !zSql ){
        rc = SQLITE_NOMEM;
        गच्छ next_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      जइ( rc!=SQLITE_OK ) गच्छ next_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    जइ( !zSql ){
      rc = SQLITE_NOMEM;
      गच्छ next_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    जइ( rc!=SQLITE_OK ) गच्छ next_exit;
  }
  pCur->rowid++;

next_exit:
  /* TODO: Handle rc */
  प्रत्यागम rc;
}

/*
** Reset a schema table cursor.
*/
स्थिर पूर्णाङ्क schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अङ्कः, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मानम् **argv
){
  पूर्णाङ्क rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  प्रत्यागम (rc==SQLITE_OK ? schemaNext(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
स्थिर पूर्णाङ्क schemaBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  प्रत्यागम SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
स्थिर sqlite3_module schemaModule = {
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
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI register_schema_module(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  जइ( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  जइ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  प्रत्यागम TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetestschema_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  प्रत्यागम TCL_OK;
}

#अण्णहा

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
पूर्णाङ्क sqlite3_schema_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#endif
  प्रत्यागम 0;
}

#endif
