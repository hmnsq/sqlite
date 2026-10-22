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

typedef sa_nracanaa echo_vtab echo_vtab;
typedef sa_nracanaa echo_cursor echo_cursor;

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
sa_nracanaa echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  ចំនួនគត់ isPattern;
  ចំនួនគត់ inTransaction;      /* True if within a transaction */
  var_na *zThis;            /* Name of the echo table */
  var_na *zTableName;       /* Name of the real table */
  var_na *zLogName;         /* Name of the log table */
  ចំនួនគត់ nCol;               /* Number of columns in the real table */
  ចំនួនគត់ *aIndex;            /* Array of size nCol. True if column has an index */
  var_na **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
sa_nracanaa echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

static ចំនួនគត់ simulateVtabError(echo_vtab *p, const var_na *zMethod){
  const var_na *zErr;
  var_na zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ប្រសិនបើ( zErr ){
    p->base.zErrMsg = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  ត្រឡប់ (zErr!=0);
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
static void dequoteString(var_na *z){
  ចំនួនគត់ quote;
  ចំនួនគត់ i, j;
  ប្រសិនបើ( z==0 ) ត្រឡប់;
  quote = z[0];
  switch( quote ){
    pak_sa '\'':  bha_nga;
    pak_sa '"':   bha_nga;
    pak_sa '`':   bha_nga;                /* For MySQL compatibility */
    pak_sa '[':   quote = ']';  bha_nga;  /* For MS SqlServer compatibility */
    default:    ត្រឡប់;
  }
  សម្រាប់(i=1, j=0; z[i]; i++){
    ប្រសិនបើ( z[i]==quote ){
      ប្រសិនបើ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }បើមិនដូច្នេះ{
        z[j++] = 0;
        bha_nga;
      }
    }បើមិនដូច្នេះ{
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
static ចំនួនគត់ getColumnNames(
  sqlite3 *db, 
  const var_na *zTab,
  var_na ***paCol, 
  ចំនួនគត់ *pnCol
){
  var_na **aCol = 0;
  var_na *zSql;
  sqlite3_stmt *pStmt = 0;
  ចំនួនគត់ rc = SQLITE_OK;
  ចំនួនគត់ nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ប្រសិនបើ( !zSql ){
    rc = SQLITE_NOMEM;
    goto out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ប្រសិនបើ( rc==SQLITE_OK ){
    ចំនួនគត់ ii;
    ចំនួនគត់ nBytes;
    var_na *zSpace;
    nCol = sqlite3_column_count(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = parimaa_na(var_na *) * nCol;
    សម្រាប់(ii=0; ii<nCol; ii++){
      const var_na *zName = sqlite3_column_name(pStmt, ii);
      ប្រសិនបើ( !zName ){
        rc = SQLITE_NOMEM;
        goto out;
      }
      nBytes += (ចំនួនគត់)strlen(zName)+1;
    }
    aCol = (var_na **)sqlite3MallocZero(nBytes);
    ប្រសិនបើ( !aCol ){
      rc = SQLITE_NOMEM;
      goto out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (var_na *)(&aCol[nCol]);
    សម្រាប់(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (ចំនួនគត់)strlen(zSpace) + 1;
    }
    assert( (zSpace-nBytes)==(var_na *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  ត្រឡប់ rc;
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
static ចំនួនគត់ getIndexArray(
  sqlite3 *db,             /* Database connection */
  const var_na *zTab,        /* Name of table in database db */
  ចំនួនគត់ nCol,
  ចំនួនគត់ **paIndex
){
  sqlite3_stmt *pStmt = 0;
  ចំនួនគត់ *aIndex = 0;
  ចំនួនគត់ rc;
  var_na *zSql;

  /* Allocate space for the index array */
  aIndex = (ចំនួនគត់ *)sqlite3MallocZero(parimaa_na(ចំនួនគត់) * nCol);
  ប្រសិនបើ( !aIndex ){
    rc = SQLITE_NOMEM;
    goto get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ប្រសិនបើ( !zSql ){
    rc = SQLITE_NOMEM;
    goto get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  ខណៈពេល( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    const var_na *zIdx = (const var_na *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ប្រសិនបើ( zIdx==0 ) continue;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ប្រសិនបើ( !zSql ){
      rc = SQLITE_NOMEM;
      goto get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ប្រសិនបើ( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      ចំនួនគត់ cid = sqlite3_column_int(pStmt2, 1);
      assert( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    ប្រសិនបើ( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ប្រសិនបើ( rc!=SQLITE_OK ){
      goto get_index_array_out;
    }
  }


get_index_array_out:
  ប្រសិនបើ( pStmt ){
    ចំនួនគត់ rc2 = sqlite3_finalize(pStmt);
    ប្រសិនបើ( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ប្រសិនបើ( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  ត្រឡប់ rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
static void appendToEchoModule(Tcl_Interp *interp, const var_na *zArg){
  ចំនួនគត់ flags = (TCL_APPEND_VALUE | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
static ចំនួនគត់ echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  ចំនួនគត់ rc = SQLITE_OK;

  ប្រសិនបើ( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ប្រសិនបើ( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      ប្រសិនបើ( sqlite3_step(pStmt)==SQLITE_ROW ){
        ចំនួនគត់ rc2;
        const var_na *zCreateTable = (const var_na *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        ប្រសិនបើ( rc==SQLITE_OK ){
          rc = rc2;
        }
      } បើមិនដូច្នេះ {
        rc = sqlite3_finalize(pStmt);
        ប្រសិនបើ( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ប្រសិនបើ( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      ប្រសិនបើ( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  ត្រឡប់ rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
static ចំនួនគត់ echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  ត្រឡប់ 0;
}

typedef sa_nracanaa EchoModule EchoModule;
sa_nracanaa EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
static ចំនួនគត់ echoConstructor(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  ចំនួនគត់ rc;
  ចំនួនគត់ i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( parimaa_na(*pVtab) );
  ប្រសិនបើ( !pVtab ){
    ត្រឡប់ SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ប្រសិនបើ( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ត្រឡប់ SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ប្រសិនបើ( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    ប្រសិនបើ( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      var_na *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    ប្រសិនបើ( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      ត្រឡប់ SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  សម្រាប់(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ប្រសិនបើ( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ត្រឡប់ rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  ត្រឡប់ SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
static ចំនួនគត់ echoCreate(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  ចំនួនគត់ rc = SQLITE_OK;
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
  ប្រសិនបើ( rc==SQLITE_OK && argc==5 ){
    var_na *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ប្រសិនបើ( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ប្រសិនបើ( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  ត្រឡប់ rc;
}

/* 
** Echo virtual table module xConnect method.
*/
static ចំនួនគត់ echoConnect(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  ត្រឡប់ echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
static ចំនួនគត់ echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  ត្រឡប់ echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
static ចំនួនគត់ echoDestroy(sqlite3_vtab *pVtab){
  ចំនួនគត់ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ប្រសិនបើ( p && p->zLogName ){
    var_na *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  ត្រឡប់ rc;
}

/* 
** Echo virtual table module xOpen method.
*/
static ចំនួនគត់ echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ប្រសិនបើ( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    ត្រឡប់ SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(parimaa_na(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  ត្រឡប់ (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
static ចំនួនគត់ echoClose(sqlite3_vtab_cursor *cur){
  ចំនួនគត់ rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  ត្រឡប់ rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
static ចំនួនគត់ echoEof(sqlite3_vtab_cursor *cur){
  ត្រឡប់ (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
static ចំនួនគត់ echoNext(sqlite3_vtab_cursor *cur){
  ចំនួនគត់ rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ប្រសិនបើ( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  ប្រសិនបើ( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    ប្រសិនបើ( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }បើមិនដូច្នេះ{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  ត្រឡប់ rc;
}

/* 
** Echo virtual table module xColumn method.
*/
static ចំនួនគត់ echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ចំនួនគត់ i){
  ចំនួនគត់ iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ប្រសិនបើ( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  ប្រសិនបើ( !pStmt ){
    sqlite3_result_null(ctx);
  }បើមិនដូច្នេះ{
    assert( sqlite3_data_count(pStmt)>iCol );
    sqlite3_result_value(ctx, sqlite3_column_value(pStmt, iCol));
  }
  ត្រឡប់ SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
static ចំនួនគត់ echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ប្រសិនបើ( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  ត្រឡប់ SQLITE_OK;
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
static ចំនួនគត់ hashString(const var_na *zString){
  u32 val = 0;
  ចំនួនគត់ ii;
  សម្រាប់(ii=0; zString[ii]; ii++){
    val = (val << 3) + (ចំនួនគត់)zString[ii];
  }
  ត្រឡប់ (ចំនួនគត់)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
static ចំនួនគត់ echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ចំនួនគត់ idxNum, const var_na *idxStr,
  ចំនួនគត់ argc, sqlite3_value **argv
){
  ចំនួនគត់ rc;
  ចំនួនគត់ i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ប្រសិនបើ( simulateVtabError(pVtab, "xFilter") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  assert( idxNum==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  សម្រាប់(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (const var_na*)sqlite3_value_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  assert( pCur->pStmt || rc!=SQLITE_OK );
  សម្រាប់(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_value(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ប្រសិនបើ( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  ត្រឡប់ rc;
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
static void string_concat(var_na **pzStr, var_na *zAppend, ចំនួនគត់ doFree, ចំនួនគត់ *pRc){
  var_na *zIn = *pzStr;
  ប្រសិនបើ( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ប្រសិនបើ( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }បើមិនដូច្នេះ{
    ប្រសិនបើ( zIn ){
      var_na *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }បើមិនដូច្នេះ{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    ប្រសិនបើ( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  ប្រសិនបើ( doFree ){
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
static var_na *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  var_na *zRet = 0;
  ប្រសិនបើ( sqlite3_libversion_number()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }បើមិនដូច្នេះ{
    ចំនួនគត់ i;
    សម្រាប់(i=0; i<pTab->nCol; i++){
      ប្រសិនបើ( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }បើមិនដូច្នេះ{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ប្រសិនបើ( !zRet ) bha_nga;
    }
  }
  ត្រឡប់ zRet;
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
static ចំនួនគត់ echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ចំនួនគត់ ii;
  var_na *zQuery = 0;
  var_na *zCol = 0;
  var_na *zNew;
  ចំនួនគត់ nArg = 0;
  const var_na *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  ចំនួនគត់ nRow = 0;
  ចំនួនគត់ useIdx = 0;
  ចំនួនគត់ rc = SQLITE_OK;
  ចំនួនគត់ useCost = 0;
  dviguu_na cost = 0;
  ចំនួនគត់ isIgnoreUsable = 0;
  ប្រសិនបើ( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ប្រសិនបើ( simulateVtabError(pVtab, "xBestIndex") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ប្រសិនបើ( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } បើមិនដូច្នេះ {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    ប្រសិនបើ( !zQuery ){
      ត្រឡប់ SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ប្រសិនបើ( rc!=SQLITE_OK ){
      ត្រឡប់ rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ប្រសិនបើ( rc!=SQLITE_OK ){
      ត្រឡប់ rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  ប្រសិនបើ( !zCol ) ត្រឡប់ SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  ប្រសិនបើ( !zQuery ) ត្រឡប់ SQLITE_NOMEM;

  សម្រាប់(ii=0; ii<pIdxInfo->nConstraint; ii++){
    const sa_nracanaa sqlite3_index_constraint *pConstraint;
    sa_nracanaa sqlite3_index_constraint_usage *pUsage;
    ចំនួនគត់ iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ប្រសិនបើ( !isIgnoreUsable && !pConstraint->usable ) continue;

    iCol = pConstraint->iColumn;
    ប្រសិនបើ( iCol<0 || pVtab->aIndex[iCol] ){
      var_na *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      var_na *zOp = 0;
      useIdx = 1;
      switch( pConstraint->op ){
        pak_sa SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; bha_nga;
        pak_sa SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; bha_nga;
      }
      ប្រសិនបើ( zOp ){
        ប្រសិនបើ( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } បើមិនដូច្នេះ {
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
  ប្រសិនបើ( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    ចំនួនគត់ iCol = pIdxInfo->aOrderBy->iColumn;
    var_na *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    var_na *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  ប្រសិនបើ( !zQuery ){
    ត្រឡប់ rc;
  }
  pIdxInfo->idxNum = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  ប្រសិនបើ( useCost ){
    pIdxInfo->estimatedCost = cost;
  }បើមិនដូច្នេះ ប្រសិនបើ( useIdx ){
    /* Approximation of log2(nRow). */
    សម្រាប់( ii=0; ii<(parimaa_na(ចំនួនគត់)*8)-1; ii++ ){
      ប្រសិនបើ( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (dviguu_na)ii;
      }
    }
  }បើមិនដូច្នេះ{
    pIdxInfo->estimatedCost = (dviguu_na)nRow;
  }
  ត្រឡប់ rc;
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
ចំនួនគត់ echoUpdate(
  sqlite3_vtab *tab, 
  ចំនួនគត់ nData, 
  sqlite3_value **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  ចំនួនគត់ rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  var_na *z = 0;               /* SQL statement to execute */
  ចំនួនគត់ bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  ចំនួនគត់ bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  ចំនួនគត់ i;                     /* Counter variable used by for loops */

  assert( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  assert( pVtab->inTransaction );

  ប្រសិនបើ( simulateVtabError(pVtab, "xUpdate") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ប្រសិនបើ( nData>1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    var_na *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    ប្រសិនបើ( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_value_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ប្រសិនបើ( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    សម្រាប់(i=2; i<nData; i++){
      ប្រសិនបើ( apData[i]==0 ) continue;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  បើមិនដូច្នេះ ប្រសិនបើ( nData==1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    ប្រសិនបើ( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  បើមិនដូច្នេះ ប្រសិនបើ( nData>2 && sqlite3_value_type(apData[0])==SQLITE_NULL ){
    ចំនួនគត់ ii;
    var_na *zInsert = 0;
    var_na *zValues = 0;
  
    zInsert = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    ប្រសិនបើ( !zInsert ){
      rc = SQLITE_NOMEM;
    }
    ប្រសិនបើ( sqlite3_value_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      zValues = sqlite3_mprintf("?");
      string_concat(&zInsert, "rowid", 0, &rc);
    }

    assert((pVtab->nCol+2)==nData);
    សម្រាប់(ii=2; ii<nData; ii++){
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
  បើមិនដូច្នេះ{
    assert(0);
    ត្រឡប់ SQLITE_ERROR;
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  assert( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ប្រសិនបើ( rc==SQLITE_OK ) {
    ប្រសិនបើ( bindArgZero ){
      sqlite3_bind_value(pStmt, nData, apData[0]);
    }
    ប្រសិនបើ( bindArgOne ){
      sqlite3_bind_value(pStmt, 1, apData[1]);
    }
    សម្រាប់(i=2; i<nData && rc==SQLITE_OK; i++){
      ប្រសិនបើ( apData[i] ) rc = sqlite3_bind_value(pStmt, i, apData[i]);
    }
    ប្រសិនបើ( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }បើមិនដូច្នេះ{
      sqlite3_finalize(pStmt);
    }
  }

  ប្រសិនបើ( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_insert_rowid(db);
  }
  ប្រសិនបើ( rc!=SQLITE_OK ){
    tab->zErrMsg = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  ត្រឡប់ rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
static ចំនួនគត់ echoTransactionCall(sqlite3_vtab *tab, const var_na *zCall){
  var_na *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  ប្រសិនបើ( z==0 ) ត្រឡប់ SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  ត្រឡប់ SQLITE_OK;
}
static ចំនួនគត់ echoBegin(sqlite3_vtab *tab){
  ចំនួនគត់ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  const var_na *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  assert( !pVtab->inTransaction );

  ប្រសិនបើ( simulateVtabError(pVtab, "xBegin") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ប្រសិនបើ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ប្រសិនបើ( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  ប្រសិនបើ( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  ត្រឡប់ rc;
}
static ចំនួនគត់ echoSync(sqlite3_vtab *tab){
  ចំនួនគត់ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  const var_na *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  assert( pVtab->inTransaction );

  ប្រសិនបើ( simulateVtabError(pVtab, "xSync") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ប្រសិនបើ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ប្រសិនបើ( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  ត្រឡប់ rc;
}
static ចំនួនគត់ echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  ចំនួនគត់ rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  assert( pVtab->inTransaction );

  ប្រសិនបើ( simulateVtabError(pVtab, "xCommit") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  ត្រឡប់ rc;
}
static ចំនួនគត់ echoRollback(sqlite3_vtab *tab){
  ចំនួនគត់ rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  assert( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  ត្រឡប់ rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
static void overloadedGlobFunction(
  sqlite3_context *pContext,
  ចំនួនគត់ nArg,
  sqlite3_value **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  ចំនួនគត់ i;
  ចំនួនគត់ rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  សម្រាប់(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (var_na*)sqlite3_value_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  ប្រសិនបើ( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }បើមិនដូច្នេះ{
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
static ចំនួនគត់ echoFindFunction(
  sqlite3_vtab *vtab,
  ចំនួនគត់ nArg,
  const var_na *zFuncName,
  void (**pxFunc)(sqlite3_context*,ចំនួនគត់,sqlite3_value**),
  void **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ប្រសិនបើ( strcmp(zFuncName,"glob")!=0 ){
    ត្រឡប់ 0;
  }
  ប្រសិនបើ( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    ត្រឡប់ 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  ត្រឡប់ 1;
}

static ចំនួនគត់ echoRename(sqlite3_vtab *vtab, const var_na *zNewName){
  ចំនួនគត់ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ប្រសិនបើ( simulateVtabError(p, "xRename") ){
    ត្រឡប់ SQLITE_ERROR;
  }

  ប្រសិនបើ( p->isPattern ){
    ចំនួនគត់ nThis = (ចំនួនគត់)strlen(p->zThis);
    var_na *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ត្រឡប់ rc;
}

static ចំនួនគត់ echoSavepoint(sqlite3_vtab *pVTab, ចំនួនគត់ iSavepoint){
  assert( pVTab );
  ត្រឡប់ SQLITE_OK;
}

static ចំនួនគត់ echoRelease(sqlite3_vtab *pVTab, ចំនួនគត់ iSavepoint){
  assert( pVTab );
  ត្រឡប់ SQLITE_OK;
}

static ចំនួនគត់ echoRollbackTo(sqlite3_vtab *pVTab, ចំនួនគត់ iSavepoint){
  assert( pVTab );
  ត្រឡប់ SQLITE_OK;
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
extern ចំនួនគត់ getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);
extern const var_na *sqlite3ErrName(ចំនួនគត់);

static void moduleDestroy(void *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
static ចំនួនគត់ SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ចំនួនគត់ rc;
  sqlite3 *db;
  EchoModule *pMod;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ត្រឡប់ TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(parimaa_na(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (void*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ប្រសិនបើ( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(parimaa_na(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (void*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
  ត្រឡប់ TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
static ចំនួនគត់ SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ចំនួនគត់ rc;
  ប្រសិនបើ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB SQL");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ត្រឡប់ TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ប្រសិនបើ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(db), TCL_VOLATILE);
    ត្រឡប់ TCL_ERROR;
  }
  ត្រឡប់ TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
ចំនួនគត់ Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ត្រឡប់ TCL_OK;
}
