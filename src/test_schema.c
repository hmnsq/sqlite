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
#నిర్వచించు SCHEMA \
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
#నిర్వచితమైతే SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#లేకపోతే
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#అయితే_ముగింపు

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

రకనిర్వచనం నిర్మాణం schema_vtab schema_vtab;
రకనిర్వచనం నిర్మాణం schema_cursor schema_cursor;

/* A schema table object */
నిర్మాణం schema_vtab {
  sqlite3_vtab పునాది;
  sqlite3 *db;
};

/* A schema table cursor object */
నిర్మాణం schema_cursor {
  sqlite3_vtab_cursor పునాది;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *p_పట్టిక_List;
  sqlite3_stmt *p_స్తంభం_List;
  పూర్ణాంకం rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE

/*
** Table destructor for the schema module.
*/
స్థిర పూర్ణాంకం schemaDestroy(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి 0;
}

/*
** Table constructor for the schema module.
*/
స్థిర పూర్ణాంకం schemaCreate(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  పూర్ణాంకం rc = SQLITE_NOMEM;
  schema_vtab *pVtab = sqlite3_malloc(పరిమాణం(schema_vtab));
  ఒకవేళ( pVtab ){
    memset(pVtab, 0, పరిమాణం(schema_vtab));
    pVtab->db = db;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#అయితే_ముగింపు
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  తిరిగి rc;
}

/*
** Open a new cursor on the schema table.
*/
స్థిర పూర్ణాంకం schemaOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  పూర్ణాంకం rc = SQLITE_NOMEM;
  schema_cursor *pCur;
  pCur = sqlite3_malloc(పరిమాణం(schema_cursor));
  ఒకవేళ( pCur ){
    memset(pCur, 0, పరిమాణం(schema_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  తిరిగి rc;
}

/*
** Close a schema table cursor.
*/
స్థిర పూర్ణాంకం schemaClose(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  sqlite3_finalize(pCur->pDbList);
  sqlite3_finalize(pCur->p_పట్టిక_List);
  sqlite3_finalize(pCur->p_స్తంభం_List);
  sqlite3_free(pCur);
  తిరిగి SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
స్థిర పూర్ణాంకం schema_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  schema_cursor *pCur = (schema_cursor *)cur;
  ఎంపిక( i ){
    స్థితి 0:
      sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCur->pDbList, 1));
      ఆపు;
    స్థితి 1:
      sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCur->p_పట్టిక_List, 0));
      ఆపు;
    అప్రమేయ:
      sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCur->p_స్తంభం_List, i-2));
      ఆపు;
  }
  తిరిగి SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
స్థిర పూర్ణాంకం schemaRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  schema_cursor *pCur = (schema_cursor *)cur;
  *pRowid = pCur->rowid;
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం finalize(sqlite3_stmt **ppStmt){
  పూర్ణాంకం rc = sqlite3_finalize(*ppStmt);
  *ppStmt = 0;
  తిరిగి rc;
}

స్థిర పూర్ణాంకం schemaEof(sqlite3_vtab_cursor *cur){
  schema_cursor *pCur = (schema_cursor *)cur;
  తిరిగి (pCur->pDbList ? 0 : 1);
}

/*
** Advance the cursor to the next row.
*/
స్థిర పూర్ణాంకం schema_తదుపరి(sqlite3_vtab_cursor *cur){
  పూర్ణాంకం rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  అక్షరం *zSql = 0;

  అయితే( !pCur->p_స్తంభం_List || SQLITE_అడ్డువరుస!=sqlite3_అడుగు(pCur->p_స్తంభం_List) ){
    ఒకవేళ( SQLITE_OK!=(rc = finalize(&pCur->p_స్తంభం_List)) ) వెళ్ళు తదుపరి_exit;

    అయితే( !pCur->p_పట్టిక_List || SQLITE_అడ్డువరుస!=sqlite3_అడుగు(pCur->p_పట్టిక_List) ){
      ఒకవేళ( SQLITE_OK!=(rc = finalize(&pCur->p_పట్టిక_List)) ) వెళ్ళు తదుపరి_exit;

      ప్రతిజ్ఞా(pCur->pDbList);
      అయితే( SQLITE_అడ్డువరుస!=sqlite3_అడుగు(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        వెళ్ళు తదుపరి_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      ఒకవేళ( sqlite3_స్తంభం_int(pCur->pDbList, 0)==1 ){
        zSql = sqlite3_mprintf(
            "SELECT name FROM sqlite_temp_schema WHERE type='table'"
        );
      }లేకపోతే{
        sqlite3_stmt *pDbList = pCur->pDbList;
        zSql = sqlite3_mprintf(
            "SELECT name FROM %Q.sqlite_schema WHERE type='table'",
             sqlite3_స్తంభం_పాఠ్యం(pDbList, 1)
        );
      }
      ఒకవేళ( !zSql ){
        rc = SQLITE_NOMEM;
        వెళ్ళు తదుపరి_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_పట్టిక_List, 0);
      sqlite3_free(zSql);
      ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు తదుపరి_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_స్తంభం_పాఠ్యం(pCur->pDbList, 1),
        sqlite3_స్తంభం_పాఠ్యం(pCur->p_పట్టిక_List, 0)
    );

    ఒకవేళ( !zSql ){
      rc = SQLITE_NOMEM;
      వెళ్ళు తదుపరి_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_స్తంభం_List, 0);
    sqlite3_free(zSql);
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు తదుపరి_exit;
  }
  pCur->rowid++;

తదుపరి_exit:
  /* TODO: Handle rc */
  తిరిగి rc;
}

/*
** Reset a schema table cursor.
*/
స్థిర పూర్ణాంకం schemaFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  పూర్ణాంకం rc;
  schema_vtab *pVtab = (schema_vtab *)(pVtabCursor->pVtab);
  schema_cursor *pCur = (schema_cursor *)pVtabCursor;
  pCur->rowid = 0;
  finalize(&pCur->p_పట్టిక_List);
  finalize(&pCur->p_స్తంభం_List);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  తిరిగి (rc==SQLITE_OK ? schema_తదుపరి(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
స్థిర పూర్ణాంకం schema_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  తిరిగి SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
స్థిర sqlite3_module schemaModule = {
  0,                           /* iVersion */
  schemaCreate,
  schemaCreate,
  schema_ఉత్తమ_సూచీ,
  schemaDestroy,
  schemaDestroy,
  schemaOpen,                  /* xOpen - open a cursor */
  schemaClose,                 /* xClose - close a cursor */
  schemaFilter,                /* xFilter - configure scan constraints */
  schema_తదుపరి,                  /* xNext - advance a cursor */
  schemaEof,                   /* xEof */
  schema_స్తంభం,                /* xColumn - read data */
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

#అయితే_ముగింపు /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#నిర్వచితమైతే SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_schema_module(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetestschema_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "register_schema_module", రిజిస్టరు_schema_module, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
  తిరిగి TCL_OK;
}

#లేకపోతే

/*
** Extension load function.
*/
#నిర్వచితమైతే _WIN32
__declspec(dllexport)
#అయితే_ముగింపు
పూర్ణాంకం sqlite3_schema_init(
  sqlite3 *db, 
  అక్షరం **pz_Err_సందేశం, 
  స్థిరరాశి sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#అయితే_ముగింపు
  తిరిగి 0;
}

#అయితే_ముగింపు
