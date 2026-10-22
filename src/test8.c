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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

typedef struct echo_vtab echo_vtab;
typedef struct echo_cursor echo_cursor;

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
struct echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  цяло isPattern;
  цяло inTransaction;      /* True if within a transaction */
  char *zThis;            /* Name of the echo table */
  char *zTableName;       /* Name of the real table */
  char *zLogName;         /* Name of the log table */
  цяло nCol;               /* Number of columns in the real table */
  цяло *aIndex;            /* Array of size nCol. True if column has an index */
  char **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
struct echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

static цяло simulateVtabError(echo_vtab *p, const char *zMethod){
  const char *zErr;
  char zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ако( zErr ){
    p->base.zErrMsg = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  върни (zErr!=0);
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
static void dequoteString(char *z){
  цяло quote;
  цяло i, j;
  ако( z==0 ) върни;
  quote = z[0];
  switch( quote ){
    case '\'':  break;
    case '"':   break;
    case '`':   break;                /* For MySQL compatibility */
    case '[':   quote = ']';  break;  /* For MS SqlServer compatibility */
    default:    върни;
  }
  за(i=1, j=0; z[i]; i++){
    ако( z[i]==quote ){
      ако( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }иначе{
        z[j++] = 0;
        break;
      }
    }иначе{
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
static цяло getColumnNames(
  sqlite3 *db, 
  const char *zTab,
  char ***paCol, 
  цяло *pnCol
){
  char **aCol = 0;
  char *zSql;
  sqlite3_stmt *pStmt = 0;
  цяло rc = SQLITE_OK;
  цяло nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ако( !zSql ){
    rc = SQLITE_NOMEM;
    goto out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ако( rc==SQLITE_OK ){
    цяло ii;
    цяло nBytes;
    char *zSpace;
    nCol = sqlite3_column_count(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = sizeof(char *) * nCol;
    за(ii=0; ii<nCol; ii++){
      const char *zName = sqlite3_column_name(pStmt, ii);
      ако( !zName ){
        rc = SQLITE_NOMEM;
        goto out;
      }
      nBytes += (цяло)strlen(zName)+1;
    }
    aCol = (char **)sqlite3MallocZero(nBytes);
    ако( !aCol ){
      rc = SQLITE_NOMEM;
      goto out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (char *)(&aCol[nCol]);
    за(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (цяло)strlen(zSpace) + 1;
    }
    assert( (zSpace-nBytes)==(char *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  върни rc;
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
static цяло getIndexArray(
  sqlite3 *db,             /* Database connection */
  const char *zTab,        /* Name of table in database db */
  цяло nCol,
  цяло **paIndex
){
  sqlite3_stmt *pStmt = 0;
  цяло *aIndex = 0;
  цяло rc;
  char *zSql;

  /* Allocate space for the index array */
  aIndex = (цяло *)sqlite3MallocZero(sizeof(цяло) * nCol);
  ако( !aIndex ){
    rc = SQLITE_NOMEM;
    goto get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ако( !zSql ){
    rc = SQLITE_NOMEM;
    goto get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  докато( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    const char *zIdx = (const char *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ако( zIdx==0 ) continue;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ако( !zSql ){
      rc = SQLITE_NOMEM;
      goto get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ако( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      цяло cid = sqlite3_column_int(pStmt2, 1);
      assert( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    ако( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ако( rc!=SQLITE_OK ){
      goto get_index_array_out;
    }
  }


get_index_array_out:
  ако( pStmt ){
    цяло rc2 = sqlite3_finalize(pStmt);
    ако( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ако( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  върни rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
static void appendToEchoModule(Tcl_Interp *interp, const char *zArg){
  цяло flags = (TCL_APPEND_VALUE | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
static цяло echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  цяло rc = SQLITE_OK;

  ако( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ако( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      ако( sqlite3_step(pStmt)==SQLITE_ROW ){
        цяло rc2;
        const char *zCreateTable = (const char *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        ако( rc==SQLITE_OK ){
          rc = rc2;
        }
      } иначе {
        rc = sqlite3_finalize(pStmt);
        ако( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ако( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      ако( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  върни rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
static цяло echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  върни 0;
}

typedef struct EchoModule EchoModule;
struct EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
static цяло echoConstructor(
  sqlite3 *db,
  void *pAux,
  цяло argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  цяло rc;
  цяло i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( sizeof(*pVtab) );
  ако( !pVtab ){
    върни SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ако( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    върни SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ако( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    ако( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      char *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    ако( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      върни SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  за(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ако( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    върни rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  върни SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
static цяло echoCreate(
  sqlite3 *db,
  void *pAux,
  цяло argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  цяло rc = SQLITE_OK;
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
  ако( rc==SQLITE_OK && argc==5 ){
    char *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ако( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ако( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ако( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  върни rc;
}

/* 
** Echo virtual table module xConnect method.
*/
static цяло echoConnect(
  sqlite3 *db,
  void *pAux,
  цяло argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  върни echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
static цяло echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  върни echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
static цяло echoDestroy(sqlite3_vtab *pVtab){
  цяло rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ако( p && p->zLogName ){
    char *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ако( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  върни rc;
}

/* 
** Echo virtual table module xOpen method.
*/
static цяло echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ако( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    върни SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(sizeof(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  върни (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
static цяло echoClose(sqlite3_vtab_cursor *cur){
  цяло rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  върни rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
static цяло echoEof(sqlite3_vtab_cursor *cur){
  върни (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
static цяло echoNext(sqlite3_vtab_cursor *cur){
  цяло rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ако( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    върни SQLITE_ERROR;
  }

  ако( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    ако( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }иначе{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  върни rc;
}

/* 
** Echo virtual table module xColumn method.
*/
static цяло echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, цяло i){
  цяло iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ако( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    върни SQLITE_ERROR;
  }

  ако( !pStmt ){
    sqlite3_result_null(ctx);
  }иначе{
    assert( sqlite3_data_count(pStmt)>iCol );
    sqlite3_result_value(ctx, sqlite3_column_value(pStmt, iCol));
  }
  върни SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
static цяло echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ако( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    върни SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  върни SQLITE_OK;
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
static цяло hashString(const char *zString){
  u32 val = 0;
  цяло ii;
  за(ii=0; zString[ii]; ii++){
    val = (val << 3) + (цяло)zString[ii];
  }
  върни (цяло)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
static цяло echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  цяло idxNum, const char *idxStr,
  цяло argc, sqlite3_value **argv
){
  цяло rc;
  цяло i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ако( simulateVtabError(pVtab, "xFilter") ){
    върни SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  assert( idxNum==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  за(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (const char*)sqlite3_value_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  assert( pCur->pStmt || rc!=SQLITE_OK );
  за(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_value(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ако( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  върни rc;
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
static void string_concat(char **pzStr, char *zAppend, цяло doFree, цяло *pRc){
  char *zIn = *pzStr;
  ако( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ако( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }иначе{
    ако( zIn ){
      char *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }иначе{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    ако( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  ако( doFree ){
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
static char *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  char *zRet = 0;
  ако( sqlite3_libversion_number()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }иначе{
    цяло i;
    за(i=0; i<pTab->nCol; i++){
      ако( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }иначе{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ако( !zRet ) break;
    }
  }
  върни zRet;
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
static цяло echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  цяло ii;
  char *zQuery = 0;
  char *zCol = 0;
  char *zNew;
  цяло nArg = 0;
  const char *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  цяло nRow = 0;
  цяло useIdx = 0;
  цяло rc = SQLITE_OK;
  цяло useCost = 0;
  double cost = 0;
  цяло isIgnoreUsable = 0;
  ако( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ако( simulateVtabError(pVtab, "xBestIndex") ){
    върни SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ако( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } иначе {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    ако( !zQuery ){
      върни SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ако( rc!=SQLITE_OK ){
      върни rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ако( rc!=SQLITE_OK ){
      върни rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  ако( !zCol ) върни SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  ако( !zQuery ) върни SQLITE_NOMEM;

  за(ii=0; ii<pIdxInfo->nConstraint; ii++){
    const struct sqlite3_index_constraint *pConstraint;
    struct sqlite3_index_constraint_usage *pUsage;
    цяло iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ако( !isIgnoreUsable && !pConstraint->usable ) continue;

    iCol = pConstraint->iColumn;
    ако( iCol<0 || pVtab->aIndex[iCol] ){
      char *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      char *zOp = 0;
      useIdx = 1;
      switch( pConstraint->op ){
        case SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; break;
        case SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; break;
        case SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; break;
        case SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; break;
        case SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; break;
        case SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; break;
        case SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; break;
        case SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; break;
        case SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; break;
      }
      ако( zOp ){
        ако( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } иначе {
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
  ако( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    цяло iCol = pIdxInfo->aOrderBy->iColumn;
    char *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    char *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  ако( !zQuery ){
    върни rc;
  }
  pIdxInfo->idxNum = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  ако( useCost ){
    pIdxInfo->estimatedCost = cost;
  }иначе ако( useIdx ){
    /* Approximation of log2(nRow). */
    за( ii=0; ii<(sizeof(цяло)*8)-1; ii++ ){
      ако( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (double)ii;
      }
    }
  }иначе{
    pIdxInfo->estimatedCost = (double)nRow;
  }
  върни rc;
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
цяло echoUpdate(
  sqlite3_vtab *tab, 
  цяло nData, 
  sqlite3_value **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  цяло rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  char *z = 0;               /* SQL statement to execute */
  цяло bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  цяло bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  цяло i;                     /* Counter variable used by for loops */

  assert( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  assert( pVtab->inTransaction );

  ако( simulateVtabError(pVtab, "xUpdate") ){
    върни SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ако( nData>1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    char *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    ако( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_value_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ако( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    за(i=2; i<nData; i++){
      ако( apData[i]==0 ) continue;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  иначе ако( nData==1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    ако( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  иначе ако( nData>2 && sqlite3_value_type(apData[0])==SQLITE_NULL ){
    цяло ii;
    char *zInsert = 0;
    char *zValues = 0;
  
    zInsert = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    ако( !zInsert ){
      rc = SQLITE_NOMEM;
    }
    ако( sqlite3_value_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      zValues = sqlite3_mprintf("?");
      string_concat(&zInsert, "rowid", 0, &rc);
    }

    assert((pVtab->nCol+2)==nData);
    за(ii=2; ii<nData; ii++){
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
  иначе{
    assert(0);
    върни SQLITE_ERROR;
  }

  ако( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  assert( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ако( rc==SQLITE_OK ) {
    ако( bindArgZero ){
      sqlite3_bind_value(pStmt, nData, apData[0]);
    }
    ако( bindArgOne ){
      sqlite3_bind_value(pStmt, 1, apData[1]);
    }
    за(i=2; i<nData && rc==SQLITE_OK; i++){
      ако( apData[i] ) rc = sqlite3_bind_value(pStmt, i, apData[i]);
    }
    ако( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }иначе{
      sqlite3_finalize(pStmt);
    }
  }

  ако( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_insert_rowid(db);
  }
  ако( rc!=SQLITE_OK ){
    tab->zErrMsg = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  върни rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
static цяло echoTransactionCall(sqlite3_vtab *tab, const char *zCall){
  char *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  ако( z==0 ) върни SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  върни SQLITE_OK;
}
static цяло echoBegin(sqlite3_vtab *tab){
  цяло rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  const char *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  assert( !pVtab->inTransaction );

  ако( simulateVtabError(pVtab, "xBegin") ){
    върни SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ако( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ако( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  ако( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  върни rc;
}
static цяло echoSync(sqlite3_vtab *tab){
  цяло rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  const char *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  assert( pVtab->inTransaction );

  ако( simulateVtabError(pVtab, "xSync") ){
    върни SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ако( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ако( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  върни rc;
}
static цяло echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  цяло rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  assert( pVtab->inTransaction );

  ако( simulateVtabError(pVtab, "xCommit") ){
    върни SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  върни rc;
}
static цяло echoRollback(sqlite3_vtab *tab){
  цяло rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  assert( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  върни rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
static void overloadedGlobFunction(
  sqlite3_context *pContext,
  цяло nArg,
  sqlite3_value **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  цяло i;
  цяло rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  за(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (char*)sqlite3_value_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  ако( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }иначе{
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
static цяло echoFindFunction(
  sqlite3_vtab *vtab,
  цяло nArg,
  const char *zFuncName,
  void (**pxFunc)(sqlite3_context*,цяло,sqlite3_value**),
  void **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ако( strcmp(zFuncName,"glob")!=0 ){
    върни 0;
  }
  ако( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    върни 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  върни 1;
}

static цяло echoRename(sqlite3_vtab *vtab, const char *zNewName){
  цяло rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ако( simulateVtabError(p, "xRename") ){
    върни SQLITE_ERROR;
  }

  ако( p->isPattern ){
    цяло nThis = (цяло)strlen(p->zThis);
    char *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  върни rc;
}

static цяло echoSavepoint(sqlite3_vtab *pVTab, цяло iSavepoint){
  assert( pVTab );
  върни SQLITE_OK;
}

static цяло echoRelease(sqlite3_vtab *pVTab, цяло iSavepoint){
  assert( pVTab );
  върни SQLITE_OK;
}

static цяло echoRollbackTo(sqlite3_vtab *pVTab, цяло iSavepoint){
  assert( pVTab );
  върни SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
static sqlite3_module echoModule = {
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

static sqlite3_module echoModuleV2 = {
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
extern цяло getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);
extern const char *sqlite3ErrName(цяло);

static void moduleDestroy(void *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
static цяло SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  цяло objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  цяло rc;
  sqlite3 *db;
  EchoModule *pMod;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    върни TCL_ERROR;
  }
  ако( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) върни TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(sizeof(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (void*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ако( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(sizeof(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (void*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
  върни TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
static цяло SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  цяло objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  цяло rc;
  ако( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB SQL");
    върни TCL_ERROR;
  }
  ако( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) върни TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ако( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (char *)sqlite3_errmsg(db), TCL_VOLATILE);
    върни TCL_ERROR;
  }
  върни TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
цяло Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  цяло i;
  за(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  върни TCL_OK;
}
