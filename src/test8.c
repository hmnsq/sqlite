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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

تعريفنوع بنية echo_vtab echo_vtab;
تعريفنوع بنية echo_cursor echo_cursor;

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
بنية echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  صحيح isPattern;
  صحيح inTransaction;      /* True if within a transaction */
  حرف *zThis;            /* Name of the echo table */
  حرف *zTableName;       /* Name of the real table */
  حرف *zLogName;         /* Name of the log table */
  صحيح nCol;               /* Number of columns in the real table */
  صحيح *aIndex;            /* Array of size nCol. True if column has an index */
  حرف **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
بنية echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

ساكن صحيح simulateVtabError(echo_vtab *p, ثابت حرف *zMethod){
  ثابت حرف *zErr;
  حرف zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  אם( zErr ){
    p->base.zErrMsg = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  החזר (zErr!=0);
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
ساكن فراغ dequoteString(حرف *z){
  صحيح quote;
  صحيح i, j;
  אם( z==0 ) החזר;
  quote = z[0];
  بدل( quote ){
    حالة '\'':  اكسر;
    حالة '"':   اكسر;
    حالة '`':   اكسر;                /* For MySQL compatibility */
    حالة '[':   quote = ']';  اكسر;  /* For MS SqlServer compatibility */
    افتراضي:    החזר;
  }
  עבור(i=1, j=0; z[i]; i++){
    אם( z[i]==quote ){
      אם( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }אחרת{
        z[j++] = 0;
        اكسر;
      }
    }אחרת{
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
ساكن صحيح getColumnNames(
  sqlite3 *db, 
  ثابت حرف *zTab,
  حرف ***paCol, 
  صحيح *pnCol
){
  حرف **aCol = 0;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحيح rc = SQLITE_OK;
  صحيح nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  אם( !zSql ){
    rc = SQLITE_NOMEM;
    اذهب out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  אם( rc==SQLITE_OK ){
    صحيح ii;
    صحيح nBytes;
    حرف *zSpace;
    nCol = sqlite3_column_count(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = حجم(حرف *) * nCol;
    עבור(ii=0; ii<nCol; ii++){
      ثابت حرف *zName = sqlite3_column_name(pStmt, ii);
      אם( !zName ){
        rc = SQLITE_NOMEM;
        اذهب out;
      }
      nBytes += (صحيح)strlen(zName)+1;
    }
    aCol = (حرف **)sqlite3MallocZero(nBytes);
    אם( !aCol ){
      rc = SQLITE_NOMEM;
      اذهب out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (حرف *)(&aCol[nCol]);
    עבור(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (صحيح)strlen(zSpace) + 1;
    }
    أكد( (zSpace-nBytes)==(حرف *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  החזר rc;
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
ساكن صحيح getIndexArray(
  sqlite3 *db,             /* Database connection */
  ثابت حرف *zTab,        /* Name of table in database db */
  صحيح nCol,
  صحيح **paIndex
){
  sqlite3_stmt *pStmt = 0;
  صحيح *aIndex = 0;
  صحيح rc;
  حرف *zSql;

  /* Allocate space for the index array */
  aIndex = (صحيح *)sqlite3MallocZero(حجم(صحيح) * nCol);
  אם( !aIndex ){
    rc = SQLITE_NOMEM;
    اذهب get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  אם( !zSql ){
    rc = SQLITE_NOMEM;
    اذهب get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  כלעוד( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    ثابت حرف *zIdx = (ثابت حرف *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    אם( zIdx==0 ) استمر;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    אם( !zSql ){
      rc = SQLITE_NOMEM;
      اذهب get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    אם( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      صحيح cid = sqlite3_column_int(pStmt2, 1);
      أكد( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    אם( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    אם( rc!=SQLITE_OK ){
      اذهب get_index_array_out;
    }
  }


get_index_array_out:
  אם( pStmt ){
    صحيح rc2 = sqlite3_finalize(pStmt);
    אם( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  אם( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  החזר rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ساكن فراغ appendToEchoModule(Tcl_Interp *interp, ثابت حرف *zArg){
  صحيح flags = (TCL_APPEND_VALUE | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
ساكن صحيح echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  صحيح rc = SQLITE_OK;

  אם( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    אם( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      אם( sqlite3_step(pStmt)==SQLITE_ROW ){
        صحيح rc2;
        ثابت حرف *zCreateTable = (ثابت حرف *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        אם( rc==SQLITE_OK ){
          rc = rc2;
        }
      } אחרת {
        rc = sqlite3_finalize(pStmt);
        אם( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      אם( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      אם( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  החזר rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ساكن صحيح echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  החזר 0;
}

تعريفنوع بنية EchoModule EchoModule;
بنية EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ساكن صحيح echoConstructor(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحيح rc;
  صحيح i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( حجم(*pVtab) );
  אם( !pVtab ){
    החזר SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  אם( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    החזר SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  אם( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    אם( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      حرف *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    אם( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      החזר SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  עבור(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  אם( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    החזר rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  החזר SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ساكن صحيح echoCreate(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحيح rc = SQLITE_OK;
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
  אם( rc==SQLITE_OK && argc==5 ){
    حرف *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    אם( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  אם( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  אם( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  החזר rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ساكن صحيح echoConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  החזר echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ساكن صحيح echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  החזר echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ساكن صحيح echoDestroy(sqlite3_vtab *pVtab){
  صحيح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  אם( p && p->zLogName ){
    حرف *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  אם( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  החזר rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ساكن صحيح echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  אם( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    החזר SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(حجم(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  החזר (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ساكن صحيح echoClose(sqlite3_vtab_cursor *cur){
  صحيح rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  החזר rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ساكن صحيح echoEof(sqlite3_vtab_cursor *cur){
  החזר (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ساكن صحيح echoNext(sqlite3_vtab_cursor *cur){
  صحيح rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  אם( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    החזר SQLITE_ERROR;
  }

  אם( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    אם( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }אחרת{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  החזר rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ساكن صحيح echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  صحيح iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  אם( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    החזר SQLITE_ERROR;
  }

  אם( !pStmt ){
    sqlite3_result_null(ctx);
  }אחרת{
    أكد( sqlite3_data_count(pStmt)>iCol );
    sqlite3_result_value(ctx, sqlite3_column_value(pStmt, iCol));
  }
  החזר SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ساكن صحيح echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  אם( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    החזר SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  החזר SQLITE_OK;
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
ساكن صحيح hashString(ثابت حرف *zString){
  u32 val = 0;
  صحيح ii;
  עבור(ii=0; zString[ii]; ii++){
    val = (val << 3) + (صحيح)zString[ii];
  }
  החזר (صحيح)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ساكن صحيح echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idxNum, ثابت حرف *idxStr,
  صحيح argc, sqlite3_value **argv
){
  صحيح rc;
  صحيح i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  אם( simulateVtabError(pVtab, "xFilter") ){
    החזר SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  أكد( idxNum==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  עבור(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (ثابت حرف*)sqlite3_value_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  أكد( pCur->pStmt || rc!=SQLITE_OK );
  עבור(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_value(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  אם( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  החזר rc;
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
ساكن فراغ string_concat(حرف **pzStr, حرف *zAppend, صحيح doFree, صحيح *pRc){
  حرف *zIn = *pzStr;
  אם( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  אם( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }אחרת{
    אם( zIn ){
      حرف *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }אחרת{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    אם( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  אם( doFree ){
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
ساكن حرف *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  حرف *zRet = 0;
  אם( sqlite3_libversion_number()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }אחרת{
    صحيح i;
    עבור(i=0; i<pTab->nCol; i++){
      אם( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }אחרת{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      אם( !zRet ) اكسر;
    }
  }
  החזר zRet;
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
ساكن صحيح echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحيح ii;
  حرف *zQuery = 0;
  حرف *zCol = 0;
  حرف *zNew;
  صحيح nArg = 0;
  ثابت حرف *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  صحيح nRow = 0;
  صحيح useIdx = 0;
  صحيح rc = SQLITE_OK;
  صحيح useCost = 0;
  مزدوج cost = 0;
  صحيح isIgnoreUsable = 0;
  אם( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  אם( simulateVtabError(pVtab, "xBestIndex") ){
    החזר SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  אם( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } אחרת {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    אם( !zQuery ){
      החזר SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    אם( rc!=SQLITE_OK ){
      החזר rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    אם( rc!=SQLITE_OK ){
      החזר rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  אם( !zCol ) החזר SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  אם( !zQuery ) החזר SQLITE_NOMEM;

  עבור(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ثابت بنية sqlite3_index_constraint *pConstraint;
    بنية sqlite3_index_constraint_usage *pUsage;
    صحيح iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    אם( !isIgnoreUsable && !pConstraint->usable ) استمر;

    iCol = pConstraint->iColumn;
    אם( iCol<0 || pVtab->aIndex[iCol] ){
      حرف *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      حرف *zOp = 0;
      useIdx = 1;
      بدل( pConstraint->op ){
        حالة SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; اكسر;
        حالة SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; اكسر;
      }
      אם( zOp ){
        אם( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } אחרת {
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
  אם( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    صحيح iCol = pIdxInfo->aOrderBy->iColumn;
    حرف *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    حرف *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  אם( !zQuery ){
    החזר rc;
  }
  pIdxInfo->idxNum = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  אם( useCost ){
    pIdxInfo->estimatedCost = cost;
  }אחרת אם( useIdx ){
    /* Approximation of log2(nRow). */
    עבור( ii=0; ii<(حجم(صحيح)*8)-1; ii++ ){
      אם( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (مزدوج)ii;
      }
    }
  }אחרת{
    pIdxInfo->estimatedCost = (مزدوج)nRow;
  }
  החזר rc;
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
صحيح echoUpdate(
  sqlite3_vtab *tab, 
  صحيح nData, 
  sqlite3_value **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  صحيح rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  حرف *z = 0;               /* SQL statement to execute */
  صحيح bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  صحيح bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  صحيح i;                     /* Counter variable used by for loops */

  أكد( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  أكد( pVtab->inTransaction );

  אם( simulateVtabError(pVtab, "xUpdate") ){
    החזר SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  אם( nData>1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    حرف *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    אם( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_value_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    אם( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    עבור(i=2; i<nData; i++){
      אם( apData[i]==0 ) استمر;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  אחרת אם( nData==1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    אם( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  אחרת אם( nData>2 && sqlite3_value_type(apData[0])==SQLITE_NULL ){
    صحيح ii;
    حرف *zInsert = 0;
    حرف *zValues = 0;
  
    zInsert = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    אם( !zInsert ){
      rc = SQLITE_NOMEM;
    }
    אם( sqlite3_value_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      zValues = sqlite3_mprintf("?");
      string_concat(&zInsert, "rowid", 0, &rc);
    }

    أكد((pVtab->nCol+2)==nData);
    עבור(ii=2; ii<nData; ii++){
      string_concat(&zInsert, 
          sqlite3_mprintf("%s%Q", zValues?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&zValues, 
          sqlite3_mprintf("%s?%d", zValues?", ":"", ii), 1, &rc);
    }

    string_concat(&z, zInsert, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, zValues, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  אחרת{
    أكد(0);
    החזר SQLITE_ERROR;
  }

  אם( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  أكد( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  אם( rc==SQLITE_OK ) {
    אם( bindArgZero ){
      sqlite3_bind_value(pStmt, nData, apData[0]);
    }
    אם( bindArgOne ){
      sqlite3_bind_value(pStmt, 1, apData[1]);
    }
    עבור(i=2; i<nData && rc==SQLITE_OK; i++){
      אם( apData[i] ) rc = sqlite3_bind_value(pStmt, i, apData[i]);
    }
    אם( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }אחרת{
      sqlite3_finalize(pStmt);
    }
  }

  אם( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_insert_rowid(db);
  }
  אם( rc!=SQLITE_OK ){
    tab->zErrMsg = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  החזר rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ساكن صحيح echoTransactionCall(sqlite3_vtab *tab, ثابت حرف *zCall){
  حرف *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  אם( z==0 ) החזר SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  החזר SQLITE_OK;
}
ساكن صحيح echoBegin(sqlite3_vtab *tab){
  صحيح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت حرف *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  أكد( !pVtab->inTransaction );

  אם( simulateVtabError(pVtab, "xBegin") ){
    החזר SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  אם( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    אם( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  אם( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  החזר rc;
}
ساكن صحيح echoSync(sqlite3_vtab *tab){
  صحيح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت حرف *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  أكد( pVtab->inTransaction );

  אם( simulateVtabError(pVtab, "xSync") ){
    החזר SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  אם( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    אם( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  החזר rc;
}
ساكن صحيح echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  صحيح rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  أكد( pVtab->inTransaction );

  אם( simulateVtabError(pVtab, "xCommit") ){
    החזר SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  החזר rc;
}
ساكن صحيح echoRollback(sqlite3_vtab *tab){
  صحيح rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  أكد( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  החזר rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ساكن فراغ overloadedGlobFunction(
  sqlite3_context *pContext,
  صحيح nArg,
  sqlite3_value **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  صحيح i;
  صحيح rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  עבור(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (حرف*)sqlite3_value_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  אם( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }אחרת{
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
ساكن صحيح echoFindFunction(
  sqlite3_vtab *vtab,
  صحيح nArg,
  ثابت حرف *zFuncName,
  فراغ (**pxFunc)(sqlite3_context*,صحيح,sqlite3_value**),
  فراغ **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  אם( strcmp(zFuncName,"glob")!=0 ){
    החזר 0;
  }
  אם( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    החזר 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  החזר 1;
}

ساكن صحيح echoRename(sqlite3_vtab *vtab, ثابت حرف *zNewName){
  صحيح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  אם( simulateVtabError(p, "xRename") ){
    החזר SQLITE_ERROR;
  }

  אם( p->isPattern ){
    صحيح nThis = (صحيح)strlen(p->zThis);
    حرف *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  החזר rc;
}

ساكن صحيح echoSavepoint(sqlite3_vtab *pVTab, صحيح iSavepoint){
  أكد( pVTab );
  החזר SQLITE_OK;
}

ساكن صحيح echoRelease(sqlite3_vtab *pVTab, صحيح iSavepoint){
  أكد( pVTab );
  החזר SQLITE_OK;
}

ساكن صحيح echoRollbackTo(sqlite3_vtab *pVTab, صحيح iSavepoint){
  أكد( pVTab );
  החזר SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ساكن sqlite3_module echoModule = {
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

ساكن sqlite3_module echoModuleV2 = {
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
خارجي صحيح getDbPointer(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

ساكن فراغ moduleDestroy(فراغ *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحيح rc;
  sqlite3 *db;
  EchoModule *pMod;
  אם( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    החזר TCL_ERROR;
  }
  אם( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) החזר TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(حجم(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (فراغ*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  אם( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(حجم(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (فراغ*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  החזר TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ساكن صحيح SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  صحيح rc;
  אם( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB SQL");
    החזר TCL_ERROR;
  }
  אם( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) החזר TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  אם( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (حرف *)sqlite3_errmsg(db), TCL_VOLATILE);
    החזר TCL_ERROR;
  }
  החזר TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  صحيح i;
  עבור(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  החזר TCL_OK;
}
