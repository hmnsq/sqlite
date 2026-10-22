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
#घोषणा SCHEMA \
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
#अगर_घोषित SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#अन्यथा
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#पूर्ण_अगर

#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>

प्रकार काष्ठा schema_vtab schema_vtab;
प्रकार काष्ठा schema_cursor schema_cursor;

/* A schema table object */
काष्ठा schema_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

/* A schema table cursor object */
काष्ठा schema_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *pTableList;
  sqlite3_stmt *pColumnList;
  पूर्णांक rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
अटल पूर्णांक schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस 0;
}

/*
** Table constructor for the schema module.
*/
अटल पूर्णांक schemaCreate(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  पूर्णांक rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(माप(schema_vtab));
  अगर( pVtab ){
    स_रखो(pVtab, 0, माप(schema_vtab));
    pVtab->db = db;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#पूर्ण_अगर
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  वापस rc;
}

/*
** Open a new cursor on the schema table.
*/
अटल पूर्णांक schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  पूर्णांक rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(माप(schema_cursor));
  अगर( pCur ){
    स_रखो(pCur, 0, माप(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  वापस rc;
}

/*
** Close a schema table cursor.
*/
अटल पूर्णांक schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->pTableList);
  sqlite3_finalize(pCur->pColumnList);
  sqlite3_free(pCur);
  वापस SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
अटल पूर्णांक schemaColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  schema_cursor *pCur = (schema_cursor *)cur;
  चयन( i ){
    हाल 0:
      sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pCur->pDbList, 1));
      अवरोध;
    हाल 1:
      sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pCur->pTableList, 0));
      अवरोध;
    शेष:
      sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pCur->pColumnList, i-2));
      अवरोध;
  }
  वापस SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
अटल पूर्णांक schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  वापस SQLITE_OK;
}

अटल पूर्णांक finalize(sqlite3_stmt **ppStmt){
  पूर्णांक rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  वापस rc;
}

अटल पूर्णांक schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  वापस (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
अटल पूर्णांक schema_अग्र(sqlite3_vtab_cursor *cur){
  पूर्णांक rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  अक्षर *zSql = 0;

  जबतक( !pCur->pColumnList || SQLITE_ROW!=sqlite3_step(pCur->pColumnList) ){
    अगर( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) जाओ अग्र_exit;

    जबतक( !pCur->pTableList || SQLITE_ROW!=sqlite3_step(pCur->pTableList) ){
      अगर( SQLITE_OK!=(rc = finalize(&pCur->pTableList)) ) जाओ अग्र_exit;

      निश्चित(pCur->pDbList);
      जबतक( SQLITE_ROW!=sqlite3_step(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        जाओ अग्र_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      अगर( sqlite3_column_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }अन्यथा{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_column_text(pDbList, 1)
        );
      }
      अगर( !zSql ){
        rc = SQLITE_NOMEM;
        जाओ अग्र_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pTableList, 0);
      sqlite3_free(zSql);
      अगर( rc!=SQLITE_OK ) जाओ अग्र_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->pTableList, 0)
    );

    अगर( !zSql ){
      rc = SQLITE_NOMEM;
      जाओ अग्र_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    अगर( rc!=SQLITE_OK ) जाओ अग्र_exit;
  }
  pCur->rowid++;

अग्र_exit:
  /* TODO: Handle rc */
  वापस rc;
}

/*
** Reset a schema table cursor.
*/
अटल पूर्णांक schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  पूर्णांक rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->pTableList);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  वापस (rc==SQLITE_OK ? schema_अग्र(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
अटल पूर्णांक schema_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  वापस SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
अटल sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_Best_सूचकांक,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_अग्र,                  /* xNext - advance a cursor */
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

#पूर्ण_अगर /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#अगर_घोषित SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
अटल पूर्णांक SQLITE_TCLAPI register_schema_module(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#पूर्ण_अगर
  वापस TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetestschema_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     व्योम *client_तथ्य;
  } aObjCmd[] = {
     { "register_schema_module", register_schema_module, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_तथ्य, 0);
  }
  वापस TCL_OK;
}

#अन्यथा

/*
** Extension load function.
*/
#अगर_घोषित _WIN32
__declspec(dllexport)
#पूर्ण_अगर
पूर्णांक sqlite3_schema_init(
  sqlite3 *db, 
  अक्षर **pz_Err_संदेश, 
  स्थिर sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#पूर्ण_अगर
  वापस 0;
}

#पूर्ण_अगर
