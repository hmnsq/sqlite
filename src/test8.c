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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

प्रकारनिर्देश संरचना echo_vtab echo_vtab;
प्रकारनिर्देश संरचना echo_cursor echo_cursor;

/*
** The test module defined in this file uses four global Tcl variables to
** communicate with test-scripts:
**
**     $::echo_module
**     $::echo_module_sync_fail
**     $::echo_module_begin_fail
**     $::echo_module_cost
**
** The variable ::echo_module is a list. Each time one of the following
** methods is called, one or more elements are appended to the list.
** This is used for automated testing of virtual table modules.
**
** The ::echo_module_sync_fail variable is set by test scripts and read
** by code in this file. If it is set to the name of a real table in the
** the database, then all xSync operations on echo virtual tables that
** use the named table as a backing store will fail.
*/

/*
** Errors can be provoked within the following echo virtual table methods:
**
**   xBestIndex   xOpen     xFilter   xNext   
**   xColumn      xRowid    xUpdate   xSync   
**   xBegin       xRename
**
** This is done by setting the global tcl variable:
**
**   echo_module_fail($method,$tbl)
**
** where $method is set to the name of the virtual table method to fail
** (i.e. "xBestIndex") and $tbl is the name of the table being echoed (not
** the name of the virtual table, the name of the underlying real table).
*/

/* 
** An echo virtual-table object.
**
** echo.vtab.aIndex is an array of booleans. The nth entry is true if 
** the nth column of the real table is the left-most column of an index
** (implicit or otherwise). In other words, if SQLite can optimize
** a query like "SELECT * FROM real_table WHERE col = ?".
**
** Member variable aCol[] contains copies of the column names of the real
** table.
*/
संरचना echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  पूर्णाङ्क isPattern;
  पूर्णाङ्क inTransaction;      /* True if within a transaction */
  वर्ण *zThis;            /* Name of the echo table */
  वर्ण *zTableName;       /* Name of the real table */
  वर्ण *zLogName;         /* Name of the log table */
  पूर्णाङ्क nCol;               /* Number of columns in the real table */
  पूर्णाङ्क *aIndex;            /* Array of size nCol. True if column has an index */
  वर्ण **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
संरचना echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

स्थिर पूर्णाङ्क simulateVtabError(echo_vtab *p, ध्रुव वर्ण *zMethod){
  ध्रुव वर्ण *zErr;
  वर्ण zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  यदि( zErr ){
    p->base.z_Err_संदेश = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  प्रत्यागम (zErr!=0);
}

/*
** Convert an SQL-style quoted string into a normal string by removing
** the quote characters.  The conversion is done in-place.  If the
** input does not begin with a quote character, then this routine
** is a no-op.
**
** Examples:
**
**     "abc"   becomes   abc
**     'xyz'   becomes   xyz
**     [pqr]   becomes   pqr
**     `mno`   becomes   mno
*/
स्थिर शून्य dequoteString(वर्ण *z){
  पूर्णाङ्क quote;
  पूर्णाङ्क i, j;
  यदि( z==0 ) प्रत्यागम;
  quote = z[0];
  विकल्प( quote ){
    पक्ष '\'':  भङ्ग;
    पक्ष '"':   भङ्ग;
    पक्ष '`':   भङ्ग;                /* For MySQL compatibility */
    पक्ष '[':   quote = ']';  भङ्ग;  /* For MS SqlServer compatibility */
    स्वभाव:    प्रत्यागम;
  }
  प्रति(i=1, j=0; z[i]; i++){
    यदि( z[i]==quote ){
      यदि( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }अन्यथा{
        z[j++] = 0;
        भङ्ग;
      }
    }अन्यथा{
      z[j++] = z[i];
    }
  }
}

/*
** Retrieve the column names for the table named zTab via database
** connection db. SQLITE_OK is returned on success, or an sqlite error
** code otherwise.
**
** If successful, the number of columns is written to *pnCol. *paCol is
** set to point at sqlite3_malloc()'d space containing the array of
** nCol column names. The caller is responsible for calling sqlite3_free
** on *paCol.
*/
स्थिर पूर्णाङ्क getColumnNames(
  sqlite3 *db, 
  ध्रुव वर्ण *zTab,
  वर्ण ***paCol, 
  पूर्णाङ्क *pnCol
){
  वर्ण **aCol = 0;
  वर्ण *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  यदि( !zSql ){
    rc = SQLITE_NOMEM;
    गच्छ out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  यदि( rc==SQLITE_OK ){
    पूर्णाङ्क ii;
    पूर्णाङ्क nBytes;
    वर्ण *zSpace;
    nCol = sqlite3_column_गिनती(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = परिमाण(वर्ण *) * nCol;
    प्रति(ii=0; ii<nCol; ii++){
      ध्रुव वर्ण *zName = sqlite3_column_name(pStmt, ii);
      यदि( !zName ){
        rc = SQLITE_NOMEM;
        गच्छ out;
      }
      nBytes += (पूर्णाङ्क)म_माप(zName)+1;
    }
    aCol = (वर्ण **)sqlite3MallocZero(nBytes);
    यदि( !aCol ){
      rc = SQLITE_NOMEM;
      गच्छ out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (वर्ण *)(&aCol[nCol]);
    प्रति(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (पूर्णाङ्क)म_माप(zSpace) + 1;
    }
    प्रतिज्ञा( (zSpace-nBytes)==(वर्ण *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  प्रत्यागम rc;
}

/*
** Parameter zTab is the name of a table in database db with nCol 
** columns. This function allocates an array of integers nCol in 
** size and populates it according to any implicit or explicit 
** indices on table zTab.
**
** If successful, SQLITE_OK is returned and *paIndex set to point 
** at the allocated array. Otherwise, an error code is returned.
**
** See comments associated with the member variable aIndex above 
** "struct echo_vtab" for details of the contents of the array.
*/
स्थिर पूर्णाङ्क getIndexArray(
  sqlite3 *db,             /* Database connection */
  ध्रुव वर्ण *zTab,        /* Name of table in database db */
  पूर्णाङ्क nCol,
  पूर्णाङ्क **paIndex
){
  sqlite3_stmt *pStmt = 0;
  पूर्णाङ्क *aIndex = 0;
  पूर्णाङ्क rc;
  वर्ण *zSql;

  /* Allocate space for the index array */
  aIndex = (पूर्णाङ्क *)sqlite3MallocZero(परिमाण(पूर्णाङ्क) * nCol);
  यदि( !aIndex ){
    rc = SQLITE_NOMEM;
    गच्छ get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  यदि( !zSql ){
    rc = SQLITE_NOMEM;
    गच्छ get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  यावत्( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    ध्रुव वर्ण *zIdx = (ध्रुव वर्ण *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    यदि( zIdx==0 ) अनुवर्त;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    यदि( !zSql ){
      rc = SQLITE_NOMEM;
      गच्छ get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    यदि( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      पूर्णाङ्क cid = sqlite3_column_int(pStmt2, 1);
      प्रतिज्ञा( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    यदि( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    यदि( rc!=SQLITE_OK ){
      गच्छ get_index_array_out;
    }
  }


get_index_array_out:
  यदि( pStmt ){
    पूर्णाङ्क rc2 = sqlite3_finalize(pStmt);
    यदि( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  यदि( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  प्रत्यागम rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
स्थिर शून्य appendToEchoModule(Tcl_Interp *interp, ध्रुव वर्ण *zArg){
  पूर्णाङ्क flags = (TCL_APPEND_मूल्य | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, "echo_module", (zArg?zArg:""), flags);
}

/*
** This function is called from within the echo-modules xCreate and
** xConnect methods. The argc and argv arguments are copies of those 
** passed to the calling method. This function is responsible for
** calling sqlite3_declare_vtab() to declare the schema of the virtual
** table being created or connected.
**
** If the constructor was passed just one argument, i.e.:
**
**   CREATE TABLE t1 AS echo(t2);
**
** Then t2 is assumed to be the name of a *real* database table. The
** schema of the virtual table is declared by passing a copy of the 
** CREATE TABLE statement for the real table to sqlite3_declare_vtab().
** Hence, the virtual table should have exactly the same column names and 
** types as the real table.
*/
स्थिर पूर्णाङ्क echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  पूर्णाङ्क rc = SQLITE_OK;

  यदि( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    यदि( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      यदि( sqlite3_step(pStmt)==SQLITE_ROW ){
        पूर्णाङ्क rc2;
        ध्रुव वर्ण *zCreateTable = (ध्रुव वर्ण *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        यदि( rc==SQLITE_OK ){
          rc = rc2;
        }
      } अन्यथा {
        rc = sqlite3_finalize(pStmt);
        यदि( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      यदि( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      यदि( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  प्रत्यागम rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
स्थिर पूर्णाङ्क echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  प्रत्यागम 0;
}

प्रकारनिर्देश संरचना EchoModule EchoModule;
संरचना EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
स्थिर पूर्णाङ्क echoConstructor(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  पूर्णाङ्क rc;
  पूर्णाङ्क i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( परिमाण(*pVtab) );
  यदि( !pVtab ){
    प्रत्यागम SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  यदि( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    प्रत्यागम SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  यदि( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    यदि( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      वर्ण *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    यदि( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      प्रत्यागम SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  प्रति(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  यदि( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    प्रत्यागम rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  प्रत्यागम SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
स्थिर पूर्णाङ्क echoCreate(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  पूर्णाङ्क rc = SQLITE_OK;
  appendToEchoModule(((EchoModule *)pAux)->interp, "xCreate");
  rc = echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);

  /* If there were two arguments passed to the module at the SQL level 
  ** (i.e. "CREATE VIRTUAL TABLE tbl USING echo(arg1, arg2)"), then 
  ** the second argument is used as a table name. Attempt to create
  ** such a table with a single column, "logmsg". This table will
  ** be used to log calls to the xUpdate method. It will be deleted
  ** when the virtual table is DROPed.
  **
  ** Note: The main point of this is to test that we can drop tables
  ** from within an xDestroy method call.
  */
  यदि( rc==SQLITE_OK && argc==5 ){
    वर्ण *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    यदि( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  यदि( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  यदि( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  प्रत्यागम rc;
}

/* 
** Echo virtual table module xConnect method.
*/
स्थिर पूर्णाङ्क echoConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  प्रत्यागम echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
स्थिर पूर्णाङ्क echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  प्रत्यागम echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
स्थिर पूर्णाङ्क echoDestroy(sqlite3_vtab *pVtab){
  पूर्णाङ्क rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  यदि( p && p->zLogName ){
    वर्ण *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  यदि( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  प्रत्यागम rc;
}

/* 
** Echo virtual table module xOpen method.
*/
स्थिर पूर्णाङ्क echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  यदि( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    प्रत्यागम SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(परिमाण(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  प्रत्यागम (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
स्थिर पूर्णाङ्क echoClose(sqlite3_vtab_cursor *cur){
  पूर्णाङ्क rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  प्रत्यागम rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
स्थिर पूर्णाङ्क echoEof(sqlite3_vtab_cursor *cur){
  प्रत्यागम (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
स्थिर पूर्णाङ्क echoNext(sqlite3_vtab_cursor *cur){
  पूर्णाङ्क rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  यदि( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    यदि( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }अन्यथा{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  प्रत्यागम rc;
}

/* 
** Echo virtual table module xColumn method.
*/
स्थिर पूर्णाङ्क echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  पूर्णाङ्क iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  यदि( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( !pStmt ){
    sqlite3_result_null(ctx);
  }अन्यथा{
    प्रतिज्ञा( sqlite3_data_गिनती(pStmt)>iCol );
    sqlite3_result_मूल्य(ctx, sqlite3_column_मूल्य(pStmt, iCol));
  }
  प्रत्यागम SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
स्थिर पूर्णाङ्क echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  यदि( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    प्रत्यागम SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  प्रत्यागम SQLITE_OK;
}

/*
** Compute a simple hash of the null terminated string zString.
**
** This module uses only sqlite3_index_info.idxStr, not 
** sqlite3_index_info.idxNum. So to test idxNum, when idxStr is set
** in echoBestIndex(), idxNum is set to the corresponding hash value.
** In echoFilter(), code assert()s that the supplied idxNum value is
** indeed the hash of the supplied idxStr.
*/
स्थिर पूर्णाङ्क hashString(ध्रुव वर्ण *zString){
  u32 val = 0;
  पूर्णाङ्क ii;
  प्रति(ii=0; zString[ii]; ii++){
    val = (val << 3) + (पूर्णाङ्क)zString[ii];
  }
  प्रत्यागम (पूर्णाङ्क)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
स्थिर पूर्णाङ्क echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  पूर्णाङ्क rc;
  पूर्णाङ्क i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  यदि( simulateVtabError(pVtab, "xFilter") ){
    प्रत्यागम SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  प्रतिज्ञा( idx_अंक==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  प्रति(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  प्रतिज्ञा( pCur->pStmt || rc!=SQLITE_OK );
  प्रति(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_मूल्य(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  यदि( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  प्रत्यागम rc;
}


/*
** A helper function used by echoUpdate() and echoBestIndex() for
** manipulating strings in concert with the sqlite3_mprintf() function.
**
** Parameter pzStr points to a pointer to a string allocated with
** sqlite3_mprintf. The second parameter, zAppend, points to another
** string. The two strings are concatenated together and *pzStr
** set to point at the result. The initial buffer pointed to by *pzStr
** is deallocated via sqlite3_free().
**
** If the third argument, doFree, is true, then sqlite3_free() is
** also called to free the buffer pointed to by zAppend.
*/
स्थिर शून्य string_concat(वर्ण **pzStr, वर्ण *zAppend, पूर्णाङ्क doFree, पूर्णाङ्क *pRc){
  वर्ण *zIn = *pzStr;
  यदि( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  यदि( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }अन्यथा{
    यदि( zIn ){
      वर्ण *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }अन्यथा{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    यदि( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  यदि( doFree ){
    sqlite3_free(zAppend);
  }
}

/*
** This function returns a pointer to an sqlite3_malloc()ed buffer 
** containing the select-list (the thing between keywords SELECT and FROM)
** to query the underlying real table with for the scan described by
** argument pIdxInfo.
**
** If the current SQLite version is earlier than 3.10.0, this is just "*"
** (select all columns). Or, for version 3.10.0 and greater, the list of
** columns identified by the pIdxInfo->colUsed mask.
*/
स्थिर वर्ण *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  वर्ण *zRet = 0;
  यदि( sqlite3_libversion_संख्या()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }अन्यथा{
    पूर्णाङ्क i;
    प्रति(i=0; i<pTab->nCol; i++){
      यदि( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }अन्यथा{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      यदि( !zRet ) भङ्ग;
    }
  }
  प्रत्यागम zRet;
}

/*
** The echo module implements the subset of query constraints and sort
** orders that may take advantage of SQLite indices on the underlying
** real table. For example, if the real table is declared as:
**
**     CREATE TABLE real(a, b, c);
**     CREATE INDEX real_index ON real(b);
**
** then the echo module handles WHERE or ORDER BY clauses that refer
** to the column "b", but not "a" or "c". If a multi-column index is
** present, only its left most column is considered. 
**
** This xBestIndex method encodes the proposed search strategy as
** an SQL query on the real table underlying the virtual echo module 
** table and stores the query in sqlite3_index_info.idxStr. The SQL
** statement is of the form:
**
**   SELECT rowid, * FROM <real-table> ?<where-clause>? ?<order-by-clause>?
**
** where the <where-clause> and <order-by-clause> are determined
** by the contents of the structure pointed to by the pIdxInfo argument.
*/
स्थिर पूर्णाङ्क echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णाङ्क ii;
  वर्ण *zQuery = 0;
  वर्ण *zCol = 0;
  वर्ण *zNew;
  पूर्णाङ्क nArg = 0;
  ध्रुव वर्ण *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  पूर्णाङ्क nRow = 0;
  पूर्णाङ्क useIdx = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क useCost = 0;
  द्विगुण cost = 0;
  पूर्णाङ्क isIgnoreUsable = 0;
  यदि( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  यदि( simulateVtabError(pVtab, "xBestIndex") ){
    प्रत्यागम SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  यदि( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = म_से_भ(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } अन्यथा {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    यदि( !zQuery ){
      प्रत्यागम SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    यदि( rc!=SQLITE_OK ){
      प्रत्यागम rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    यदि( rc!=SQLITE_OK ){
      प्रत्यागम rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  यदि( !zCol ) प्रत्यागम SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  यदि( !zQuery ) प्रत्यागम SQLITE_NOMEM;

  प्रति(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ध्रुव संरचना sqlite3_index_constraint *pConstraint;
    संरचना sqlite3_index_constraint_usage *pUsage;
    पूर्णाङ्क iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    यदि( !isIgnoreUsable && !pConstraint->usable ) अनुवर्त;

    iCol = pConstraint->iColumn;
    यदि( iCol<0 || pVtab->aIndex[iCol] ){
      वर्ण *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      वर्ण *zOp = 0;
      useIdx = 1;
      विकल्प( pConstraint->op ){
        पक्ष SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; भङ्ग;
        पक्ष SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; भङ्ग;
      }
      यदि( zOp ){
        यदि( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } अन्यथा {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, zNewCol, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argvIndex = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  यदि( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    पूर्णाङ्क iCol = pIdxInfo->aOrderBy->iColumn;
    वर्ण *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    वर्ण *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  यदि( !zQuery ){
    प्रत्यागम rc;
  }
  pIdxInfo->idx_अंक = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  यदि( useCost ){
    pIdxInfo->estimatedCost = cost;
  }अन्यथा यदि( useIdx ){
    /* Approximation of log2(nRow). */
    प्रति( ii=0; ii<(परिमाण(पूर्णाङ्क)*8)-1; ii++ ){
      यदि( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (द्विगुण)ii;
      }
    }
  }अन्यथा{
    pIdxInfo->estimatedCost = (द्विगुण)nRow;
  }
  प्रत्यागम rc;
}

/*
** The xUpdate method for echo module virtual tables.
** 
**    apData[0]  apData[1]  apData[2..]
**
**    INTEGER                              DELETE            
**
**    INTEGER    NULL       (nCol args)    UPDATE (do not set rowid)
**    INTEGER    INTEGER    (nCol args)    UPDATE (with SET rowid = <arg1>)
**
**    NULL       NULL       (nCol args)    INSERT INTO (automatic rowid value)
**    NULL       INTEGER    (nCol args)    INSERT (incl. rowid value)
**
*/
पूर्णाङ्क echoUpdate(
  sqlite3_vtab *tab, 
  पूर्णाङ्क nData, 
  sqlite3_मूल्य **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  पूर्णाङ्क rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  वर्ण *z = 0;               /* SQL statement to execute */
  पूर्णाङ्क bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  पूर्णाङ्क bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  पूर्णाङ्क i;                     /* Counter variable used by for loops */

  प्रतिज्ञा( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  प्रतिज्ञा( pVtab->inTransaction );

  यदि( simulateVtabError(pVtab, "xUpdate") ){
    प्रत्यागम SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  यदि( nData>1 && sqlite3_मूल्य_type(apData[0])==SQLITE_INTEGER ){
    वर्ण *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    यदि( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_मूल्य_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    यदि( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    प्रति(i=2; i<nData; i++){
      यदि( apData[i]==0 ) अनुवर्त;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  अन्यथा यदि( nData==1 && sqlite3_मूल्य_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    यदि( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  अन्यथा यदि( nData>2 && sqlite3_मूल्य_type(apData[0])==SQLITE_NULL ){
    पूर्णाङ्क ii;
    वर्ण *z_जोड़ो = 0;
    वर्ण *z_मूल्यावली = 0;
  
    z_जोड़ो = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    यदि( !z_जोड़ो ){
      rc = SQLITE_NOMEM;
    }
    यदि( sqlite3_मूल्य_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_मूल्यावली = sqlite3_mprintf("?");
      string_concat(&z_जोड़ो, "rowid", 0, &rc);
    }

    प्रतिज्ञा((pVtab->nCol+2)==nData);
    प्रति(ii=2; ii<nData; ii++){
      string_concat(&z_जोड़ो, 
          sqlite3_mprintf("%s%Q", z_मूल्यावली?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_मूल्यावली, 
          sqlite3_mprintf("%s?%d", z_मूल्यावली?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_जोड़ो, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_मूल्यावली, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  अन्यथा{
    प्रतिज्ञा(0);
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  प्रतिज्ञा( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  यदि( rc==SQLITE_OK ) {
    यदि( bindArgZero ){
      sqlite3_bind_मूल्य(pStmt, nData, apData[0]);
    }
    यदि( bindArgOne ){
      sqlite3_bind_मूल्य(pStmt, 1, apData[1]);
    }
    प्रति(i=2; i<nData && rc==SQLITE_OK; i++){
      यदि( apData[i] ) rc = sqlite3_bind_मूल्य(pStmt, i, apData[i]);
    }
    यदि( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }अन्यथा{
      sqlite3_finalize(pStmt);
    }
  }

  यदि( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_जोड़ो_rowid(db);
  }
  यदि( rc!=SQLITE_OK ){
    tab->z_Err_संदेश = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  प्रत्यागम rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
स्थिर पूर्णाङ्क echoTransactionCall(sqlite3_vtab *tab, ध्रुव वर्ण *zCall){
  वर्ण *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  यदि( z==0 ) प्रत्यागम SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  प्रत्यागम SQLITE_OK;
}
स्थिर पूर्णाङ्क echoBegin(sqlite3_vtab *tab){
  पूर्णाङ्क rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ध्रुव वर्ण *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  प्रतिज्ञा( !pVtab->inTransaction );

  यदि( simulateVtabError(pVtab, "xBegin") ){
    प्रत्यागम SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  यदि( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    यदि( zVal && 0==म_भेद(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  यदि( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क echoSync(sqlite3_vtab *tab){
  पूर्णाङ्क rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ध्रुव वर्ण *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  प्रतिज्ञा( pVtab->inTransaction );

  यदि( simulateVtabError(pVtab, "xSync") ){
    प्रत्यागम SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  यदि( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    यदि( zVal && 0==म_भेद(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  पूर्णाङ्क rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  प्रतिज्ञा( pVtab->inTransaction );

  यदि( simulateVtabError(pVtab, "xCommit") ){
    प्रत्यागम SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क echoRollback(sqlite3_vtab *tab){
  पूर्णाङ्क rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  प्रतिज्ञा( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  प्रत्यागम rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
स्थिर शून्य overloadedGlobFunction(
  sqlite3_context *pContext,
  पूर्णाङ्क nArg,
  sqlite3_मूल्य **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  पूर्णाङ्क i;
  पूर्णाङ्क rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  प्रति(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (वर्ण*)sqlite3_मूल्य_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_मूल्य(&str));
  Tcl_DStringFree(&str);
  यदि( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }अन्यथा{
    sqlite3_result_text(pContext, Tcl_GetStringResult(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_ResetResult(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
स्थिर पूर्णाङ्क echoFindFunction(
  sqlite3_vtab *vtab,
  पूर्णाङ्क nArg,
  ध्रुव वर्ण *zFuncName,
  शून्य (**pxFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मूल्य**),
  शून्य **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  यदि( म_भेद(zFuncName,"glob")!=0 ){
    प्रत्यागम 0;
  }
  यदि( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    प्रत्यागम 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  प्रत्यागम 1;
}

स्थिर पूर्णाङ्क echoRename(sqlite3_vtab *vtab, ध्रुव वर्ण *zNewName){
  पूर्णाङ्क rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  यदि( simulateVtabError(p, "xRename") ){
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( p->isPattern ){
    पूर्णाङ्क nThis = (पूर्णाङ्क)म_माप(p->zThis);
    वर्ण *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क echoSavepoint(sqlite3_vtab *pVTab, पूर्णाङ्क iSavepoint){
  प्रतिज्ञा( pVTab );
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क echoRelease(sqlite3_vtab *pVTab, पूर्णाङ्क iSavepoint){
  प्रतिज्ञा( pVTab );
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क echoRollbackTo(sqlite3_vtab *pVTab, पूर्णाङ्क iSavepoint){
  प्रतिज्ञा( pVTab );
  प्रत्यागम SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
स्थिर sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echoBestIndex,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echoNext,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echoColumn,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echoFindFunction,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};

स्थिर sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echoBestIndex,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echoNext,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echoColumn,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echoFindFunction,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  echoSavepoint,
  echoRelease,
  echoRollbackTo,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

स्थिर शून्य moduleDestroy(शून्य *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णाङ्क rc;
  sqlite3 *db;
  EchoModule *pMod;
  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(परिमाण(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (शून्य*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  यदि( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(परिमाण(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (शून्य*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  प्रत्यागम TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  पूर्णाङ्क rc;
  यदि( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB SQL");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  यदि( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (वर्ण *)sqlite3_errmsg(db), TCL_VOLATILE);
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  प्रत्यागम TCL_OK;
}
