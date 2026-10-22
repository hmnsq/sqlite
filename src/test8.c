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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

تعریفنوع ساختار echo_vtab echo_vtab;
تعریفنوع ساختار echo_cursor echo_cursor;

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
ساختار echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  صحیح isPattern;
  صحیح inTransaction;      /* True if within a transaction */
  نویسه *zThis;            /* Name of the echo table */
  نویسه *zTableName;       /* Name of the real table */
  نویسه *zLogName;         /* Name of the log table */
  صحیح nCol;               /* Number of columns in the real table */
  صحیح *aIndex;            /* Array of size nCol. True if column has an index */
  نویسه **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ساختار echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

ایستا صحیح simulateVtabError(echo_vtab *p, ثابت نویسه *zMethod){
  ثابت نویسه *zErr;
  نویسه zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  اگر( zErr ){
    p->base.z_Err_پیام = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  بازگشت (zErr!=0);
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
ایستا تهی dequoteString(نویسه *z){
  صحیح quote;
  صحیح i, j;
  اگر( z==0 ) بازگشت;
  quote = z[0];
  انتخاب( quote ){
    حالت '\'':  بشکن;
    حالت '"':   بشکن;
    حالت '`':   بشکن;                /* For MySQL compatibility */
    حالت '[':   quote = ']';  بشکن;  /* For MS SqlServer compatibility */
    پیشفرض:    بازگشت;
  }
  برای(i=1, j=0; z[i]; i++){
    اگر( z[i]==quote ){
      اگر( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }وگرنه{
        z[j++] = 0;
        بشکن;
      }
    }وگرنه{
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
ایستا صحیح getColumnNames(
  sqlite3 *db, 
  ثابت نویسه *zTab,
  نویسه ***paCol, 
  صحیح *pnCol
){
  نویسه **aCol = 0;
  نویسه *zSql;
  sqlite3_stmt *pStmt = 0;
  صحیح rc = SQLITE_OK;
  صحیح nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  اگر( !zSql ){
    rc = SQLITE_NOMEM;
    برو out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  اگر( rc==SQLITE_OK ){
    صحیح ii;
    صحیح nBytes;
    نویسه *zSpace;
    nCol = sqlite3_column_شمار(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = اندازه(نویسه *) * nCol;
    برای(ii=0; ii<nCol; ii++){
      ثابت نویسه *zName = sqlite3_column_name(pStmt, ii);
      اگر( !zName ){
        rc = SQLITE_NOMEM;
        برو out;
      }
      nBytes += (صحیح)strlen(zName)+1;
    }
    aCol = (نویسه **)sqlite3MallocZero(nBytes);
    اگر( !aCol ){
      rc = SQLITE_NOMEM;
      برو out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (نویسه *)(&aCol[nCol]);
    برای(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (صحیح)strlen(zSpace) + 1;
    }
    أكد( (zSpace-nBytes)==(نویسه *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  بازگشت rc;
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
ایستا صحیح getIndexArray(
  sqlite3 *db,             /* Database connection */
  ثابت نویسه *zTab,        /* Name of table in database db */
  صحیح nCol,
  صحیح **paIndex
){
  sqlite3_stmt *pStmt = 0;
  صحیح *aIndex = 0;
  صحیح rc;
  نویسه *zSql;

  /* Allocate space for the index array */
  aIndex = (صحیح *)sqlite3MallocZero(اندازه(صحیح) * nCol);
  اگر( !aIndex ){
    rc = SQLITE_NOMEM;
    برو get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  اگر( !zSql ){
    rc = SQLITE_NOMEM;
    برو get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  وقتی( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    ثابت نویسه *zIdx = (ثابت نویسه *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    اگر( zIdx==0 ) ادامه;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    اگر( !zSql ){
      rc = SQLITE_NOMEM;
      برو get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    اگر( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      صحیح cid = sqlite3_column_int(pStmt2, 1);
      أكد( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    اگر( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    اگر( rc!=SQLITE_OK ){
      برو get_index_array_out;
    }
  }


get_index_array_out:
  اگر( pStmt ){
    صحیح rc2 = sqlite3_finalize(pStmt);
    اگر( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  اگر( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  بازگشت rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ایستا تهی appendToEchoModule(Tcl_Interp *interp, ثابت نویسه *zArg){
  صحیح flags = (TCL_APPEND_مقدار | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
ایستا صحیح echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  صحیح rc = SQLITE_OK;

  اگر( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    اگر( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      اگر( sqlite3_step(pStmt)==SQLITE_ROW ){
        صحیح rc2;
        ثابت نویسه *zCreateTable = (ثابت نویسه *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        اگر( rc==SQLITE_OK ){
          rc = rc2;
        }
      } وگرنه {
        rc = sqlite3_finalize(pStmt);
        اگر( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      اگر( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      اگر( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  بازگشت rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ایستا صحیح echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  بازگشت 0;
}

تعریفنوع ساختار EchoModule EchoModule;
ساختار EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ایستا صحیح echoConstructor(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  صحیح rc;
  صحیح i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( اندازه(*pVtab) );
  اگر( !pVtab ){
    بازگشت SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  اگر( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    بازگشت SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  اگر( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    اگر( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      نویسه *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    اگر( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      بازگشت SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  برای(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  اگر( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    بازگشت rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  بازگشت SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ایستا صحیح echoCreate(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  صحیح rc = SQLITE_OK;
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
  اگر( rc==SQLITE_OK && argc==5 ){
    نویسه *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    اگر( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  اگر( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  اگر( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  بازگشت rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ایستا صحیح echoConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  بازگشت echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ایستا صحیح echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  بازگشت echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ایستا صحیح echoDestroy(sqlite3_vtab *pVtab){
  صحیح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  اگر( p && p->zLogName ){
    نویسه *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  اگر( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  بازگشت rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ایستا صحیح echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  اگر( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    بازگشت SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(اندازه(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  بازگشت (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ایستا صحیح echoClose(sqlite3_vtab_cursor *cur){
  صحیح rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  بازگشت rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ایستا صحیح echoEof(sqlite3_vtab_cursor *cur){
  بازگشت (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ایستا صحیح echoNext(sqlite3_vtab_cursor *cur){
  صحیح rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  اگر( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    بازگشت SQLITE_ERROR;
  }

  اگر( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    اگر( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }وگرنه{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  بازگشت rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ایستا صحیح echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  صحیح iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  اگر( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    بازگشت SQLITE_ERROR;
  }

  اگر( !pStmt ){
    sqlite3_result_null(ctx);
  }وگرنه{
    أكد( sqlite3_data_شمار(pStmt)>iCol );
    sqlite3_result_مقدار(ctx, sqlite3_column_مقدار(pStmt, iCol));
  }
  بازگشت SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ایستا صحیح echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  اگر( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    بازگشت SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  بازگشت SQLITE_OK;
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
ایستا صحیح hashString(ثابت نویسه *zString){
  u32 val = 0;
  صحیح ii;
  برای(ii=0; zString[ii]; ii++){
    val = (val << 3) + (صحیح)zString[ii];
  }
  بازگشت (صحیح)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ایستا صحیح echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  صحیح rc;
  صحیح i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  اگر( simulateVtabError(pVtab, "xFilter") ){
    بازگشت SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  أكد( idx_رقم==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  برای(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (ثابت نویسه*)sqlite3_مقدار_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  أكد( pCur->pStmt || rc!=SQLITE_OK );
  برای(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_مقدار(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  اگر( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  بازگشت rc;
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
ایستا تهی string_concat(نویسه **pzStr, نویسه *zAppend, صحیح doFree, صحیح *pRc){
  نویسه *zIn = *pzStr;
  اگر( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  اگر( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }وگرنه{
    اگر( zIn ){
      نویسه *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }وگرنه{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    اگر( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  اگر( doFree ){
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
ایستا نویسه *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  نویسه *zRet = 0;
  اگر( sqlite3_libversion_عدد()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }وگرنه{
    صحیح i;
    برای(i=0; i<pTab->nCol; i++){
      اگر( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }وگرنه{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      اگر( !zRet ) بشکن;
    }
  }
  بازگشت zRet;
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
ایستا صحیح echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحیح ii;
  نویسه *zQuery = 0;
  نویسه *zCol = 0;
  نویسه *zNew;
  صحیح nArg = 0;
  ثابت نویسه *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  صحیح nRow = 0;
  صحیح useIdx = 0;
  صحیح rc = SQLITE_OK;
  صحیح useCost = 0;
  دوگانه cost = 0;
  صحیح isIgnoreUsable = 0;
  اگر( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  اگر( simulateVtabError(pVtab, "xBestIndex") ){
    بازگشت SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  اگر( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } وگرنه {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    اگر( !zQuery ){
      بازگشت SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    اگر( rc!=SQLITE_OK ){
      بازگشت rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    اگر( rc!=SQLITE_OK ){
      بازگشت rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  اگر( !zCol ) بازگشت SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  اگر( !zQuery ) بازگشت SQLITE_NOMEM;

  برای(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ثابت ساختار sqlite3_index_constraint *pConstraint;
    ساختار sqlite3_index_constraint_usage *pUsage;
    صحیح iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    اگر( !isIgnoreUsable && !pConstraint->usable ) ادامه;

    iCol = pConstraint->iColumn;
    اگر( iCol<0 || pVtab->aIndex[iCol] ){
      نویسه *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      نویسه *zOp = 0;
      useIdx = 1;
      انتخاب( pConstraint->op ){
        حالت SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; بشکن;
        حالت SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; بشکن;
      }
      اگر( zOp ){
        اگر( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } وگرنه {
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
  اگر( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    صحیح iCol = pIdxInfo->aOrderBy->iColumn;
    نویسه *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    نویسه *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  اگر( !zQuery ){
    بازگشت rc;
  }
  pIdxInfo->idx_رقم = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  اگر( useCost ){
    pIdxInfo->estimatedCost = cost;
  }وگرنه اگر( useIdx ){
    /* Approximation of log2(nRow). */
    برای( ii=0; ii<(اندازه(صحیح)*8)-1; ii++ ){
      اگر( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (دوگانه)ii;
      }
    }
  }وگرنه{
    pIdxInfo->estimatedCost = (دوگانه)nRow;
  }
  بازگشت rc;
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
صحیح echoUpdate(
  sqlite3_vtab *tab, 
  صحیح nData, 
  sqlite3_مقدار **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  صحیح rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  نویسه *z = 0;               /* SQL statement to execute */
  صحیح bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  صحیح bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  صحیح i;                     /* Counter variable used by for loops */

  أكد( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  أكد( pVtab->inTransaction );

  اگر( simulateVtabError(pVtab, "xUpdate") ){
    بازگشت SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  اگر( nData>1 && sqlite3_مقدار_type(apData[0])==SQLITE_INTEGER ){
    نویسه *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    اگر( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_مقدار_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    اگر( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    برای(i=2; i<nData; i++){
      اگر( apData[i]==0 ) ادامه;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  وگرنه اگر( nData==1 && sqlite3_مقدار_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    اگر( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  وگرنه اگر( nData>2 && sqlite3_مقدار_type(apData[0])==SQLITE_NULL ){
    صحیح ii;
    نویسه *z_درج = 0;
    نویسه *z_مقادیر = 0;
  
    z_درج = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    اگر( !z_درج ){
      rc = SQLITE_NOMEM;
    }
    اگر( sqlite3_مقدار_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_مقادیر = sqlite3_mprintf("?");
      string_concat(&z_درج, "rowid", 0, &rc);
    }

    أكد((pVtab->nCol+2)==nData);
    برای(ii=2; ii<nData; ii++){
      string_concat(&z_درج, 
          sqlite3_mprintf("%s%Q", z_مقادیر?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_مقادیر, 
          sqlite3_mprintf("%s?%d", z_مقادیر?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_درج, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_مقادیر, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  وگرنه{
    أكد(0);
    بازگشت SQLITE_ERROR;
  }

  اگر( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  أكد( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  اگر( rc==SQLITE_OK ) {
    اگر( bindArgZero ){
      sqlite3_bind_مقدار(pStmt, nData, apData[0]);
    }
    اگر( bindArgOne ){
      sqlite3_bind_مقدار(pStmt, 1, apData[1]);
    }
    برای(i=2; i<nData && rc==SQLITE_OK; i++){
      اگر( apData[i] ) rc = sqlite3_bind_مقدار(pStmt, i, apData[i]);
    }
    اگر( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }وگرنه{
      sqlite3_finalize(pStmt);
    }
  }

  اگر( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_درج_rowid(db);
  }
  اگر( rc!=SQLITE_OK ){
    tab->z_Err_پیام = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  بازگشت rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ایستا صحیح echoTransactionCall(sqlite3_vtab *tab, ثابت نویسه *zCall){
  نویسه *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  اگر( z==0 ) بازگشت SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  بازگشت SQLITE_OK;
}
ایستا صحیح echoBegin(sqlite3_vtab *tab){
  صحیح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت نویسه *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  أكد( !pVtab->inTransaction );

  اگر( simulateVtabError(pVtab, "xBegin") ){
    بازگشت SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  اگر( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    اگر( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  اگر( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  بازگشت rc;
}
ایستا صحیح echoSync(sqlite3_vtab *tab){
  صحیح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت نویسه *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  أكد( pVtab->inTransaction );

  اگر( simulateVtabError(pVtab, "xSync") ){
    بازگشت SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  اگر( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    اگر( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  بازگشت rc;
}
ایستا صحیح echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  صحیح rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  أكد( pVtab->inTransaction );

  اگر( simulateVtabError(pVtab, "xCommit") ){
    بازگشت SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  بازگشت rc;
}
ایستا صحیح echoRollback(sqlite3_vtab *tab){
  صحیح rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  أكد( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  بازگشت rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ایستا تهی overloadedGlobFunction(
  sqlite3_context *pContext,
  صحیح nArg,
  sqlite3_مقدار **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  صحیح i;
  صحیح rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  برای(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (نویسه*)sqlite3_مقدار_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_مقدار(&str));
  Tcl_DStringFree(&str);
  اگر( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }وگرنه{
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
ایستا صحیح echoFindFunction(
  sqlite3_vtab *vtab,
  صحیح nArg,
  ثابت نویسه *zFuncName,
  تهی (**pxFunc)(sqlite3_context*,صحیح,sqlite3_مقدار**),
  تهی **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  اگر( strcmp(zFuncName,"glob")!=0 ){
    بازگشت 0;
  }
  اگر( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    بازگشت 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  بازگشت 1;
}

ایستا صحیح echoRename(sqlite3_vtab *vtab, ثابت نویسه *zNewName){
  صحیح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  اگر( simulateVtabError(p, "xRename") ){
    بازگشت SQLITE_ERROR;
  }

  اگر( p->isPattern ){
    صحیح nThis = (صحیح)strlen(p->zThis);
    نویسه *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  بازگشت rc;
}

ایستا صحیح echoSavepoint(sqlite3_vtab *pVTab, صحیح iSavepoint){
  أكد( pVTab );
  بازگشت SQLITE_OK;
}

ایستا صحیح echoRelease(sqlite3_vtab *pVTab, صحیح iSavepoint){
  أكد( pVTab );
  بازگشت SQLITE_OK;
}

ایستا صحیح echoRollbackTo(sqlite3_vtab *pVTab, صحیح iSavepoint){
  أكد( pVTab );
  بازگشت SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ایستا sqlite3_module echoModule = {
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

ایستا sqlite3_module echoModuleV2 = {
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
بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

ایستا تهی moduleDestroy(تهی *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ایستا صحیح SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحیح rc;
  sqlite3 *db;
  EchoModule *pMod;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(اندازه(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (تهی*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  اگر( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(اندازه(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (تهی*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_STATIC);
  بازگشت TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ایستا صحیح SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  صحیح rc;
  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB SQL");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  اگر( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (نویسه *)sqlite3_errmsg(db), TCL_VOLATILE);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
     تهی *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  بازگشت TCL_OK;
}
