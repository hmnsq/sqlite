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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

প্রকারলেখ কাঠামো echo_vtab echo_vtab;
প্রকারলেখ কাঠামো echo_cursor echo_cursor;

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
কাঠামো echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  পূর্ণসংখ্যা isPattern;
  পূর্ণসংখ্যা inTransaction;      /* True if within a transaction */
  অক্ষর *zThis;            /* Name of the echo table */
  অক্ষর *zTableName;       /* Name of the real table */
  অক্ষর *zLogName;         /* Name of the log table */
  পূর্ণসংখ্যা nCol;               /* Number of columns in the real table */
  পূর্ণসংখ্যা *a_সূচক;            /* Array of size nCol. True if column has an index */
  অক্ষর **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
কাঠামো echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

স্থির পূর্ণসংখ্যা simulateVtabError(echo_vtab *p, ধ্রুবক অক্ষর *zMethod){
  ধ্রুবক অক্ষর *zErr;
  অক্ষর zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  যদি( zErr ){
    p->base.z_Err_বার্তা = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  ফেরত (zErr!=0);
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
স্থির শূন্য dequoteString(অক্ষর *z){
  পূর্ণসংখ্যা quote;
  পূর্ণসংখ্যা i, j;
  যদি( z==0 ) ফেরত;
  quote = z[0];
  নির্বাচন( quote ){
    ক্ষেত্র '\'':  বিরতি;
    ক্ষেত্র '"':   বিরতি;
    ক্ষেত্র '`':   বিরতি;                /* For MySQL compatibility */
    ক্ষেত্র '[':   quote = ']';  বিরতি;  /* For MS SqlServer compatibility */
    পূর্বনির্ধারিত:    ফেরত;
  }
  জন্য(i=1, j=0; z[i]; i++){
    যদি( z[i]==quote ){
      যদি( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }অন্যথা{
        z[j++] = 0;
        বিরতি;
      }
    }অন্যথা{
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
স্থির পূর্ণসংখ্যা getColumnNames(
  sqlite3 *db, 
  ধ্রুবক অক্ষর *zTab,
  অক্ষর ***paCol, 
  পূর্ণসংখ্যা *pnCol
){
  অক্ষর **aCol = 0;
  অক্ষর *zSql;
  sqlite3_stmt *pStmt = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  যদি( !zSql ){
    rc = SQLITE_NOMEM;
    যাও out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  যদি( rc==SQLITE_OK ){
    পূর্ণসংখ্যা ii;
    পূর্ণসংখ্যা nBytes;
    অক্ষর *zSpace;
    nCol = sqlite3_column_গুনতি(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = আকার(অক্ষর *) * nCol;
    জন্য(ii=0; ii<nCol; ii++){
      ধ্রুবক অক্ষর *zName = sqlite3_column_name(pStmt, ii);
      যদি( !zName ){
        rc = SQLITE_NOMEM;
        যাও out;
      }
      nBytes += (পূর্ণসংখ্যা)strlen(zName)+1;
    }
    aCol = (অক্ষর **)sqlite3MallocZero(nBytes);
    যদি( !aCol ){
      rc = SQLITE_NOMEM;
      যাও out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (অক্ষর *)(&aCol[nCol]);
    জন্য(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (পূর্ণসংখ্যা)strlen(zSpace) + 1;
    }
    প্রতিজ্ঞা( (zSpace-nBytes)==(অক্ষর *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা get_সূচক_Array(
  sqlite3 *db,             /* Database connection */
  ধ্রুবক অক্ষর *zTab,        /* Name of table in database db */
  পূর্ণসংখ্যা nCol,
  পূর্ণসংখ্যা **pa_সূচক
){
  sqlite3_stmt *pStmt = 0;
  পূর্ণসংখ্যা *a_সূচক = 0;
  পূর্ণসংখ্যা rc;
  অক্ষর *zSql;

  /* Allocate space for the index array */
  a_সূচক = (পূর্ণসংখ্যা *)sqlite3MallocZero(আকার(পূর্ণসংখ্যা) * nCol);
  যদি( !a_সূচক ){
    rc = SQLITE_NOMEM;
    যাও get_সূচক_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  যদি( !zSql ){
    rc = SQLITE_NOMEM;
    যাও get_সূচক_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  যখন( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    ধ্রুবক অক্ষর *zIdx = (ধ্রুবক অক্ষর *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    যদি( zIdx==0 ) চালু;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    যদি( !zSql ){
      rc = SQLITE_NOMEM;
      যাও get_সূচক_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    যদি( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      পূর্ণসংখ্যা cid = sqlite3_column_int(pStmt2, 1);
      প্রতিজ্ঞা( cid>=0 && cid<nCol );
      a_সূচক[cid] = 1;
    }
    যদি( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    যদি( rc!=SQLITE_OK ){
      যাও get_সূচক_array_out;
    }
  }


get_সূচক_array_out:
  যদি( pStmt ){
    পূর্ণসংখ্যা rc2 = sqlite3_finalize(pStmt);
    যদি( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  যদি( rc!=SQLITE_OK ){
    sqlite3_free(a_সূচক);
    a_সূচক = 0;
  }
  *pa_সূচক = a_সূচক;
  ফেরত rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
স্থির শূন্য appendToEchoModule(Tcl_Interp *interp, ধ্রুবক অক্ষর *zArg){
  পূর্ণসংখ্যা flags = (TCL_APPEND_মান | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
স্থির পূর্ণসংখ্যা echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  পূর্ণসংখ্যা rc = SQLITE_OK;

  যদি( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    যদি( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      যদি( sqlite3_step(pStmt)==SQLITE_ROW ){
        পূর্ণসংখ্যা rc2;
        ধ্রুবক অক্ষর *zCreateTable = (ধ্রুবক অক্ষর *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        যদি( rc==SQLITE_OK ){
          rc = rc2;
        }
      } অন্যথা {
        rc = sqlite3_finalize(pStmt);
        যদি( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      যদি( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      যদি( rc==SQLITE_OK ){
        rc = get_সূচক_Array(db, pVtab->zTableName, pVtab->nCol, &pVtab->a_সূচক);
      }
    }
  }

  ফেরত rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
স্থির পূর্ণসংখ্যা echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_সূচক);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  ফেরত 0;
}

প্রকারলেখ কাঠামো EchoModule EchoModule;
কাঠামো EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
স্থির পূর্ণসংখ্যা echoConstructor(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( আকার(*pVtab) );
  যদি( !pVtab ){
    ফেরত SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  যদি( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ফেরত SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  যদি( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    যদি( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      অক্ষর *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    যদি( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      ফেরত SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  জন্য(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  যদি( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ফেরত rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  ফেরত SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
স্থির পূর্ণসংখ্যা echoCreate(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
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
  যদি( rc==SQLITE_OK && argc==5 ){
    অক্ষর *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    যদি( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  যদি( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  যদি( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  ফেরত rc;
}

/* 
** Echo virtual table module xConnect method.
*/
স্থির পূর্ণসংখ্যা echoConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  ফেরত echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
স্থির পূর্ণসংখ্যা echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  ফেরত echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
স্থির পূর্ণসংখ্যা echoDestroy(sqlite3_vtab *pVtab){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  যদি( p && p->zLogName ){
    অক্ষর *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  যদি( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  ফেরত rc;
}

/* 
** Echo virtual table module xOpen method.
*/
স্থির পূর্ণসংখ্যা echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  যদি( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    ফেরত SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(আকার(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  ফেরত (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
স্থির পূর্ণসংখ্যা echoClose(sqlite3_vtab_cursor *cur){
  পূর্ণসংখ্যা rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  ফেরত rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
স্থির পূর্ণসংখ্যা echoEof(sqlite3_vtab_cursor *cur){
  ফেরত (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
স্থির পূর্ণসংখ্যা echo_পরবর্তী(sqlite3_vtab_cursor *cur){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  যদি( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    ফেরত SQLITE_ERROR;
  }

  যদি( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    যদি( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }অন্যথা{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  ফেরত rc;
}

/* 
** Echo virtual table module xColumn method.
*/
স্থির পূর্ণসংখ্যা echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  পূর্ণসংখ্যা iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  যদি( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    ফেরত SQLITE_ERROR;
  }

  যদি( !pStmt ){
    sqlite3_ফলাফল_null(ctx);
  }অন্যথা{
    প্রতিজ্ঞা( sqlite3_তথ্য_গুনতি(pStmt)>iCol );
    sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pStmt, iCol));
  }
  ফেরত SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
স্থির পূর্ণসংখ্যা echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  যদি( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    ফেরত SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা hashString(ধ্রুবক অক্ষর *zString){
  u32 val = 0;
  পূর্ণসংখ্যা ii;
  জন্য(ii=0; zString[ii]; ii++){
    val = (val << 3) + (পূর্ণসংখ্যা)zString[ii];
  }
  ফেরত (পূর্ণসংখ্যা)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
স্থির পূর্ণসংখ্যা echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  যদি( simulateVtabError(pVtab, "xFilter") ){
    ফেরত SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  প্রতিজ্ঞা( idx_অঙ্ক==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  জন্য(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  প্রতিজ্ঞা( pCur->pStmt || rc!=SQLITE_OK );
  জন্য(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_মান(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  যদি( rc==SQLITE_OK ){
    rc = echo_পরবর্তী(pVtabCursor);
  }

  ফেরত rc;
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
স্থির শূন্য string_concat(অক্ষর **pzStr, অক্ষর *zAppend, পূর্ণসংখ্যা doFree, পূর্ণসংখ্যা *pRc){
  অক্ষর *zIn = *pzStr;
  যদি( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  যদি( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }অন্যথা{
    যদি( zIn ){
      অক্ষর *z_অস্থায়ী = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(z_অস্থায়ী);
    }অন্যথা{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    যদি( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  যদি( doFree ){
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
স্থির অক্ষর *echoSelectList(echo_vtab *pTab, sqlite3_সূচক_info *pIdxInfo){
  অক্ষর *zRet = 0;
  যদি( sqlite3_libversion_সংখ্যা()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }অন্যথা{
    পূর্ণসংখ্যা i;
    জন্য(i=0; i<pTab->nCol; i++){
      যদি( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }অন্যথা{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      যদি( !zRet ) বিরতি;
    }
  }
  ফেরত zRet;
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
স্থির পূর্ণসংখ্যা echo_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  পূর্ণসংখ্যা ii;
  অক্ষর *zQuery = 0;
  অক্ষর *zCol = 0;
  অক্ষর *zNew;
  পূর্ণসংখ্যা nArg = 0;
  ধ্রুবক অক্ষর *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  পূর্ণসংখ্যা nRow = 0;
  পূর্ণসংখ্যা useIdx = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা useCost = 0;
  দ্বিগুণ cost = 0;
  পূর্ণসংখ্যা isIgnoreUsable = 0;
  যদি( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  যদি( simulateVtabError(pVtab, "xBestIndex") ){
    ফেরত SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  যদি( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } অন্যথা {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    যদি( !zQuery ){
      ফেরত SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    যদি( rc!=SQLITE_OK ){
      ফেরত rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    যদি( rc!=SQLITE_OK ){
      ফেরত rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  যদি( !zCol ) ফেরত SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  যদি( !zQuery ) ফেরত SQLITE_NOMEM;

  জন্য(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ধ্রুবক কাঠামো sqlite3_সূচক_constraint *pConstraint;
    কাঠামো sqlite3_সূচক_constraint_usage *pUsage;
    পূর্ণসংখ্যা iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    যদি( !isIgnoreUsable && !pConstraint->usable ) চালু;

    iCol = pConstraint->iColumn;
    যদি( iCol<0 || pVtab->a_সূচক[iCol] ){
      অক্ষর *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      অক্ষর *zOp = 0;
      useIdx = 1;
      নির্বাচন( pConstraint->op ){
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_EQ:
          zOp = "="; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LT:
          zOp = "<"; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GT:
          zOp = ">"; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LE:
          zOp = "<="; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GE:
          zOp = ">="; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LIKE:
          zOp = "like"; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GLOB:
          zOp = "glob"; বিরতি;
        ক্ষেত্র SQLITE_সূচক_CONSTRAINT_REGEXP:
          zOp = "regexp"; বিরতি;
      }
      যদি( zOp ){
        যদি( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } অন্যথা {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, zNewCol, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_সূচক = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  যদি( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->a_সূচক[pIdxInfo->aOrderBy->iColumn]) ){
    পূর্ণসংখ্যা iCol = pIdxInfo->aOrderBy->iColumn;
    অক্ষর *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    অক্ষর *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  যদি( !zQuery ){
    ফেরত rc;
  }
  pIdxInfo->idx_অঙ্ক = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  যদি( useCost ){
    pIdxInfo->estimatedCost = cost;
  }অন্যথা যদি( useIdx ){
    /* Approximation of log2(nRow). */
    জন্য( ii=0; ii<(আকার(পূর্ণসংখ্যা)*8)-1; ii++ ){
      যদি( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (দ্বিগুণ)ii;
      }
    }
  }অন্যথা{
    pIdxInfo->estimatedCost = (দ্বিগুণ)nRow;
  }
  ফেরত rc;
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
পূর্ণসংখ্যা echoUpdate(
  sqlite3_vtab *tab, 
  পূর্ণসংখ্যা n_তথ্য, 
  sqlite3_মান **ap_তথ্য, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  অক্ষর *z = 0;               /* SQL statement to execute */
  পূর্ণসংখ্যা bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  পূর্ণসংখ্যা bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  পূর্ণসংখ্যা i;                     /* Counter variable used by for loops */

  প্রতিজ্ঞা( n_তথ্য==pVtab->nCol+2 || n_তথ্য==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  প্রতিজ্ঞা( pVtab->inTransaction );

  যদি( simulateVtabError(pVtab, "xUpdate") ){
    ফেরত SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  যদি( n_তথ্য>1 && sqlite3_মান_type(ap_তথ্য[0])==SQLITE_INTEGER ){
    অক্ষর *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    যদি( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_তথ্য[1] && sqlite3_মান_type(ap_তথ্য[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    যদি( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    জন্য(i=2; i<n_তথ্য; i++){
      যদি( ap_তথ্য[i]==0 ) চালু;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_তথ্য), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  অন্যথা যদি( n_তথ্য==1 && sqlite3_মান_type(ap_তথ্য[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    যদি( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  অন্যথা যদি( n_তথ্য>2 && sqlite3_মান_type(ap_তথ্য[0])==SQLITE_NULL ){
    পূর্ণসংখ্যা ii;
    অক্ষর *z_ঢোকাও = 0;
    অক্ষর *z_মানসমূহ = 0;
  
    z_ঢোকাও = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    যদি( !z_ঢোকাও ){
      rc = SQLITE_NOMEM;
    }
    যদি( sqlite3_মান_type(ap_তথ্য[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_মানসমূহ = sqlite3_mprintf("?");
      string_concat(&z_ঢোকাও, "rowid", 0, &rc);
    }

    প্রতিজ্ঞা((pVtab->nCol+2)==n_তথ্য);
    জন্য(ii=2; ii<n_তথ্য; ii++){
      string_concat(&z_ঢোকাও, 
          sqlite3_mprintf("%s%Q", z_মানসমূহ?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_মানসমূহ, 
          sqlite3_mprintf("%s?%d", z_মানসমূহ?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_ঢোকাও, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_মানসমূহ, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  অন্যথা{
    প্রতিজ্ঞা(0);
    ফেরত SQLITE_ERROR;
  }

  যদি( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  প্রতিজ্ঞা( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  যদি( rc==SQLITE_OK ) {
    যদি( bindArgZero ){
      sqlite3_bind_মান(pStmt, n_তথ্য, ap_তথ্য[0]);
    }
    যদি( bindArgOne ){
      sqlite3_bind_মান(pStmt, 1, ap_তথ্য[1]);
    }
    জন্য(i=2; i<n_তথ্য && rc==SQLITE_OK; i++){
      যদি( ap_তথ্য[i] ) rc = sqlite3_bind_মান(pStmt, i, ap_তথ্য[i]);
    }
    যদি( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }অন্যথা{
      sqlite3_finalize(pStmt);
    }
  }

  যদি( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_ঢোকাও_rowid(db);
  }
  যদি( rc!=SQLITE_OK ){
    tab->z_Err_বার্তা = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  ফেরত rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
স্থির পূর্ণসংখ্যা echoTransactionCall(sqlite3_vtab *tab, ধ্রুবক অক্ষর *zCall){
  অক্ষর *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  যদি( z==0 ) ফেরত SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  ফেরত SQLITE_OK;
}
স্থির পূর্ণসংখ্যা echoBegin(sqlite3_vtab *tab){
  পূর্ণসংখ্যা rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ধ্রুবক অক্ষর *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  প্রতিজ্ঞা( !pVtab->inTransaction );

  যদি( simulateVtabError(pVtab, "xBegin") ){
    ফেরত SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  যদি( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    যদি( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  যদি( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  ফেরত rc;
}
স্থির পূর্ণসংখ্যা echoSync(sqlite3_vtab *tab){
  পূর্ণসংখ্যা rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ধ্রুবক অক্ষর *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  প্রতিজ্ঞা( pVtab->inTransaction );

  যদি( simulateVtabError(pVtab, "xSync") ){
    ফেরত SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  যদি( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    যদি( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  ফেরত rc;
}
স্থির পূর্ণসংখ্যা echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  পূর্ণসংখ্যা rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  প্রতিজ্ঞা( pVtab->inTransaction );

  যদি( simulateVtabError(pVtab, "xCommit") ){
    ফেরত SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  ফেরত rc;
}
স্থির পূর্ণসংখ্যা echoRollback(sqlite3_vtab *tab){
  পূর্ণসংখ্যা rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  প্রতিজ্ঞা( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  ফেরত rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
স্থির শূন্য overloadedGlobFunction(
  sqlite3_context *pContext,
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **apArg
){
  Tcl_Interp *interp = sqlite3_user_তথ্য(pContext);
  Tcl_DString str;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  জন্য(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (অক্ষর*)sqlite3_মান_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_মান(&str));
  Tcl_DStringFree(&str);
  যদি( rc ){
    sqlite3_ফলাফল_error(pContext, Tcl_Get_String_ফলাফল(interp), -1);
  }অন্যথা{
    sqlite3_ফলাফল_text(pContext, Tcl_Get_String_ফলাফল(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_ফলাফল(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
স্থির পূর্ণসংখ্যা echoFindFunction(
  sqlite3_vtab *vtab,
  পূর্ণসংখ্যা nArg,
  ধ্রুবক অক্ষর *zFuncName,
  শূন্য (**pxFunc)(sqlite3_context*,পূর্ণসংখ্যা,sqlite3_মান**),
  শূন্য **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  যদি( strcmp(zFuncName,"glob")!=0 ){
    ফেরত 0;
  }
  যদি( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    ফেরত 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  ফেরত 1;
}

স্থির পূর্ণসংখ্যা echoRename(sqlite3_vtab *vtab, ধ্রুবক অক্ষর *zNewName){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  যদি( simulateVtabError(p, "xRename") ){
    ফেরত SQLITE_ERROR;
  }

  যদি( p->isPattern ){
    পূর্ণসংখ্যা nThis = (পূর্ণসংখ্যা)strlen(p->zThis);
    অক্ষর *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ফেরত rc;
}

স্থির পূর্ণসংখ্যা echoSavepoint(sqlite3_vtab *pVTab, পূর্ণসংখ্যা iSavepoint){
  প্রতিজ্ঞা( pVTab );
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা echoRelease(sqlite3_vtab *pVTab, পূর্ণসংখ্যা iSavepoint){
  প্রতিজ্ঞা( pVTab );
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা echoRollbackTo(sqlite3_vtab *pVTab, পূর্ণসংখ্যা iSavepoint){
  প্রতিজ্ঞা( pVTab );
  ফেরত SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
স্থির sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_Best_সূচক,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_পরবর্তী,                  /* xNext - advance a cursor */
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

স্থির sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_Best_সূচক,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_পরবর্তী,                  /* xNext - advance a cursor */
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
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

স্থির শূন্য moduleDestroy(শূন্য *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_echo_module(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  পূর্ণসংখ্যা rc;
  sqlite3 *db;
  EchoModule *pMod;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(আকার(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (শূন্য*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  যদি( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(আকার(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (শূন্য*)pMod, moduleDestroy
    );
  }

  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_STATIC);
  ফেরত TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI declare_vtab(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  পূর্ণসংখ্যা rc;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB SQL");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  যদি( rc!=SQLITE_OK ){
    Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3_errmsg(db), TCL_VOLATILE);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
#endif
  ফেরত TCL_OK;
}
