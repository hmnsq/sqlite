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
#परिभाषय SCHEMA \
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
#यदि_परिभाषितम् SQLITE_TEST
#  include "sqliteInt.h"
#  include "tclsqlite.h"
#अन्यथा
#  include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#यदि_अन्तः

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

प्रकारनिर्देश संरचना schema_vtab schema_vtab;
प्रकारनिर्देश संरचना schema_cursor schema_cursor;

/* A schema table object */
संरचना schema_vtab {
  sqlite3_vtab आधारः;
  sqlite3 *db;
};

/* A schema table cursor object */
संरचना schema_cursor {
  sqlite3_vtab_cursor आधारः;
  sqlite3_stmt *pDbList;
  sqlite3_stmt *p_सारणी_List;
  sqlite3_stmt *pColumnList;
  पूर्णाङ्क rowid;
};

/*
** None of this works unless we have virtual tables.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE

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
  यदि( pVtab ){
    स्मृतिं_पूरय(pVtab, 0, परिमाण(schema_vtab));
    pVtab->db = db;
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_declare_vtab(db, SCHEMA);
#यदि_अन्तः
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
  यदि( pCur ){
    स्मृतिं_पूरय(pCur, 0, परिमाण(schema_cursor));
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
  sqlite3_finalize(pCur->p_सारणी_List);
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
      sqlite3_परिणाम_मानम्(ctx, sqlite3_column_मानम्(pCur->pDbList, 1));
      भङ्ग;
    पक्ष 1:
      sqlite3_परिणाम_मानम्(ctx, sqlite3_column_मानम्(pCur->p_सारणी_List, 0));
      भङ्ग;
    स्वभाव:
      sqlite3_परिणाम_मानम्(ctx, sqlite3_column_मानम्(pCur->pColumnList, i-2));
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
स्थिर पूर्णाङ्क schema_अग्र(sqlite3_vtab_cursor *cur){
  पूर्णाङ्क rc = SQLITE_OK;
  schema_cursor *pCur = (schema_cursor *)cur;
  schema_vtab *pVtab = (schema_vtab *)(cur->pVtab);
  वर्ण *zSql = 0;

  यावत्( !pCur->pColumnList || SQLITE_ROW!=sqlite3_चरणः(pCur->pColumnList) ){
    यदि( SQLITE_OK!=(rc = finalize(&pCur->pColumnList)) ) गच्छ अग्र_exit;

    यावत्( !pCur->p_सारणी_List || SQLITE_ROW!=sqlite3_चरणः(pCur->p_सारणी_List) ){
      यदि( SQLITE_OK!=(rc = finalize(&pCur->p_सारणी_List)) ) गच्छ अग्र_exit;

      प्रतिज्ञा(pCur->pDbList);
      यावत्( SQLITE_ROW!=sqlite3_चरणः(pCur->pDbList) ){
        rc = finalize(&pCur->pDbList);
        गच्छ अग्र_exit;
      }

      /* Set zSql to the SQL to pull the list of tables from the 
      ** sqlite_schema (or sqlite_temp_schema) table of the database
      ** identified by the row pointed to by the SQL statement pCur->pDbList
      ** (iterating through a "PRAGMA database_list;" statement).
      */
      यदि( sqlite3_column_int(pCur->pDbList, 0)==1 ){
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
      यदि( !zSql ){
        rc = SQLITE_NOMEM;
        गच्छ अग्र_exit;
      }

      rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->p_सारणी_List, 0);
      sqlite3_free(zSql);
      यदि( rc!=SQLITE_OK ) गच्छ अग्र_exit;
    }

    /* Set zSql to the SQL to the table_info pragma for the table currently
    ** identified by the rows pointed to by statements pCur->pDbList and
    ** pCur->pTableList.
    */
    zSql = sqlite3_mprintf("PRAGMA %Q.table_info(%Q)", 
        sqlite3_column_text(pCur->pDbList, 1),
        sqlite3_column_text(pCur->p_सारणी_List, 0)
    );

    यदि( !zSql ){
      rc = SQLITE_NOMEM;
      गच्छ अग्र_exit;
    }
    rc = sqlite3_prepare(pVtab->db, zSql, -1, &pCur->pColumnList, 0);
    sqlite3_free(zSql);
    यदि( rc!=SQLITE_OK ) गच्छ अग्र_exit;
  }
  pCur->rowid++;

अग्र_exit:
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
  finalize(&pCur->p_सारणी_List);
  finalize(&pCur->pColumnList);
  finalize(&pCur->pDbList);
  rc = sqlite3_prepare(pVtab->db,"PRAGMA database_list", -1, &pCur->pDbList, 0);
  प्रत्यागम (rc==SQLITE_OK ? schema_अग्र(pVtabCursor) : rc);
}

/*
** Analyse the WHERE condition.
*/
स्थिर पूर्णाङ्क schema_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
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

#यदि_अन्तः /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#यदि_परिभाषितम् SQLITE_TEST

/*
** Decode a pointer to an sqlite3 object.
*/
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);

/*
** Register the schema virtual table module.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI पञ्जिकाकोशः_schema_module(
  Client_दत्त client_दत्त, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetestschema_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *client_दत्त;
  } aObjCmd[] = {
     { "register_schema_module", पञ्जिकाकोशः_schema_module, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_दत्त, 0);
  }
  प्रत्यागम TCL_OK;
}

#अन्यथा

/*
** Extension load function.
*/
#यदि_परिभाषितम् _WIN32
__declspec(dllexport)
#यदि_अन्तः
पूर्णाङ्क sqlite3_schema_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "schema", &schemaModule, 0);
#यदि_अन्तः
  प्रत्यागम 0;
}

#यदि_अन्तः
