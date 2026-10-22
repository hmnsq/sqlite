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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

തരനിർവചനം ഘടന echo_vtab echo_vtab;
തരനിർവചനം ഘടന echo_cursor echo_cursor;

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
ഘടന echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  പൂർണ്ണസംഖ്യ isPattern;
  പൂർണ്ണസംഖ്യ inTransaction;      /* True if within a transaction */
  അക്ഷരം *zThis;            /* Name of the echo table */
  അക്ഷരം *zTableName;       /* Name of the real table */
  അക്ഷരം *zLogName;         /* Name of the log table */
  പൂർണ്ണസംഖ്യ nCol;               /* Number of columns in the real table */
  പൂർണ്ണസംഖ്യ *aIndex;            /* Array of size nCol. True if column has an index */
  അക്ഷരം **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ഘടന echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ simulateVtabError(echo_vtab *p, സ്ഥിരം അക്ഷരം *zMethod){
  സ്ഥിരം അക്ഷരം *zErr;
  അക്ഷരം zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  എങ്കിൽ( zErr ){
    p->base.z_Err_സന്ദേശം = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  തിരികെ (zErr!=0);
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
സ്ഥിതം ശൂന്യം dequoteString(അക്ഷരം *z){
  പൂർണ്ണസംഖ്യ quote;
  പൂർണ്ണസംഖ്യ i, j;
  എങ്കിൽ( z==0 ) തിരികെ;
  quote = z[0];
  തിരഞ്ഞെടുപ്പ്( quote ){
    അവസ്ഥ '\'':  നിർത്തുക;
    അവസ്ഥ '"':   നിർത്തുക;
    അവസ്ഥ '`':   നിർത്തുക;                /* For MySQL compatibility */
    അവസ്ഥ '[':   quote = ']';  നിർത്തുക;  /* For MS SqlServer compatibility */
    സ്ഥിരസ്ഥിതി:    തിരികെ;
  }
  വേണ്ടി(i=1, j=0; z[i]; i++){
    എങ്കിൽ( z[i]==quote ){
      എങ്കിൽ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }അല്ലെങ്കിൽ{
        z[j++] = 0;
        നിർത്തുക;
      }
    }അല്ലെങ്കിൽ{
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
സ്ഥിതം പൂർണ്ണസംഖ്യ getColumnNames(
  sqlite3 *db, 
  സ്ഥിരം അക്ഷരം *zTab,
  അക്ഷരം ***paCol, 
  പൂർണ്ണസംഖ്യ *pnCol
){
  അക്ഷരം **aCol = 0;
  അക്ഷരം *zSql;
  sqlite3_stmt *pStmt = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  എങ്കിൽ( !zSql ){
    rc = SQLITE_NOMEM;
    പോകുക out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  എങ്കിൽ( rc==SQLITE_OK ){
    പൂർണ്ണസംഖ്യ ii;
    പൂർണ്ണസംഖ്യ nBytes;
    അക്ഷരം *zSpace;
    nCol = sqlite3_column_എണ്ണം(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = വലുപ്പം(അക്ഷരം *) * nCol;
    വേണ്ടി(ii=0; ii<nCol; ii++){
      സ്ഥിരം അക്ഷരം *zName = sqlite3_column_name(pStmt, ii);
      എങ്കിൽ( !zName ){
        rc = SQLITE_NOMEM;
        പോകുക out;
      }
      nBytes += (പൂർണ്ണസംഖ്യ)strlen(zName)+1;
    }
    aCol = (അക്ഷരം **)sqlite3MallocZero(nBytes);
    എങ്കിൽ( !aCol ){
      rc = SQLITE_NOMEM;
      പോകുക out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (അക്ഷരം *)(&aCol[nCol]);
    വേണ്ടി(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (പൂർണ്ണസംഖ്യ)strlen(zSpace) + 1;
    }
    പ്രതിജ്ഞാ( (zSpace-nBytes)==(അക്ഷരം *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  തിരികെ rc;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ getIndexArray(
  sqlite3 *db,             /* Database connection */
  സ്ഥിരം അക്ഷരം *zTab,        /* Name of table in database db */
  പൂർണ്ണസംഖ്യ nCol,
  പൂർണ്ണസംഖ്യ **paIndex
){
  sqlite3_stmt *pStmt = 0;
  പൂർണ്ണസംഖ്യ *aIndex = 0;
  പൂർണ്ണസംഖ്യ rc;
  അക്ഷരം *zSql;

  /* Allocate space for the index array */
  aIndex = (പൂർണ്ണസംഖ്യ *)sqlite3MallocZero(വലുപ്പം(പൂർണ്ണസംഖ്യ) * nCol);
  എങ്കിൽ( !aIndex ){
    rc = SQLITE_NOMEM;
    പോകുക get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  എങ്കിൽ( !zSql ){
    rc = SQLITE_NOMEM;
    പോകുക get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  ആകുമ്പോൾ( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    സ്ഥിരം അക്ഷരം *zIdx = (സ്ഥിരം അക്ഷരം *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    എങ്കിൽ( zIdx==0 ) തുടരുക;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    എങ്കിൽ( !zSql ){
      rc = SQLITE_NOMEM;
      പോകുക get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    എങ്കിൽ( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      പൂർണ്ണസംഖ്യ cid = sqlite3_column_int(pStmt2, 1);
      പ്രതിജ്ഞാ( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    എങ്കിൽ( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    എങ്കിൽ( rc!=SQLITE_OK ){
      പോകുക get_index_array_out;
    }
  }


get_index_array_out:
  എങ്കിൽ( pStmt ){
    പൂർണ്ണസംഖ്യ rc2 = sqlite3_finalize(pStmt);
    എങ്കിൽ( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  എങ്കിൽ( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  തിരികെ rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
സ്ഥിതം ശൂന്യം appendToEchoModule(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zArg){
  പൂർണ്ണസംഖ്യ flags = (TCL_APPEND_മൂല്യം | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  എങ്കിൽ( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    എങ്കിൽ( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      എങ്കിൽ( sqlite3_step(pStmt)==SQLITE_ROW ){
        പൂർണ്ണസംഖ്യ rc2;
        സ്ഥിരം അക്ഷരം *zCreateTable = (സ്ഥിരം അക്ഷരം *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        എങ്കിൽ( rc==SQLITE_OK ){
          rc = rc2;
        }
      } അല്ലെങ്കിൽ {
        rc = sqlite3_finalize(pStmt);
        എങ്കിൽ( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      എങ്കിൽ( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      എങ്കിൽ( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  തിരികെ rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  തിരികെ 0;
}

തരനിർവചനം ഘടന EchoModule EchoModule;
ഘടന EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoConstructor(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( വലുപ്പം(*pVtab) );
  എങ്കിൽ( !pVtab ){
    തിരികെ SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  എങ്കിൽ( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    തിരികെ SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  എങ്കിൽ( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    എങ്കിൽ( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      അക്ഷരം *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    എങ്കിൽ( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      തിരികെ SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  വേണ്ടി(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  എങ്കിൽ( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    തിരികെ rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  തിരികെ SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoCreate(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
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
  എങ്കിൽ( rc==SQLITE_OK && argc==5 ){
    അക്ഷരം *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    എങ്കിൽ( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  എങ്കിൽ( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  തിരികെ rc;
}

/* 
** Echo virtual table module xConnect method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  തിരികെ echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  തിരികെ echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoDestroy(sqlite3_vtab *pVtab){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  എങ്കിൽ( p && p->zLogName ){
    അക്ഷരം *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  തിരികെ rc;
}

/* 
** Echo virtual table module xOpen method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  എങ്കിൽ( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    തിരികെ SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(വലുപ്പം(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  തിരികെ (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoClose(sqlite3_vtab_cursor *cur){
  പൂർണ്ണസംഖ്യ rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  തിരികെ rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoEof(sqlite3_vtab_cursor *cur){
  തിരികെ (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoNext(sqlite3_vtab_cursor *cur){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  എങ്കിൽ( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    തിരികെ SQLITE_ERROR;
  }

  എങ്കിൽ( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    എങ്കിൽ( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }അല്ലെങ്കിൽ{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  തിരികെ rc;
}

/* 
** Echo virtual table module xColumn method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  പൂർണ്ണസംഖ്യ iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  എങ്കിൽ( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    തിരികെ SQLITE_ERROR;
  }

  എങ്കിൽ( !pStmt ){
    sqlite3_result_null(ctx);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( sqlite3_data_എണ്ണം(pStmt)>iCol );
    sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pStmt, iCol));
  }
  തിരികെ SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  എങ്കിൽ( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    തിരികെ SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ hashString(സ്ഥിരം അക്ഷരം *zString){
  u32 val = 0;
  പൂർണ്ണസംഖ്യ ii;
  വേണ്ടി(ii=0; zString[ii]; ii++){
    val = (val << 3) + (പൂർണ്ണസംഖ്യ)zString[ii];
  }
  തിരികെ (പൂർണ്ണസംഖ്യ)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  എങ്കിൽ( simulateVtabError(pVtab, "xFilter") ){
    തിരികെ SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  പ്രതിജ്ഞാ( idx_അക്കം==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  വേണ്ടി(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  പ്രതിജ്ഞാ( pCur->pStmt || rc!=SQLITE_OK );
  വേണ്ടി(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_മൂല്യം(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  തിരികെ rc;
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
സ്ഥിതം ശൂന്യം string_concat(അക്ഷരം **pzStr, അക്ഷരം *zAppend, പൂർണ്ണസംഖ്യ doFree, പൂർണ്ണസംഖ്യ *pRc){
  അക്ഷരം *zIn = *pzStr;
  എങ്കിൽ( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  എങ്കിൽ( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( zIn ){
      അക്ഷരം *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }അല്ലെങ്കിൽ{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    എങ്കിൽ( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  എങ്കിൽ( doFree ){
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
സ്ഥിതം അക്ഷരം *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  അക്ഷരം *zRet = 0;
  എങ്കിൽ( sqlite3_libversion_സംഖ്യ()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<pTab->nCol; i++){
      എങ്കിൽ( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }അല്ലെങ്കിൽ{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      എങ്കിൽ( !zRet ) നിർത്തുക;
    }
  }
  തിരികെ zRet;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  പൂർണ്ണസംഖ്യ ii;
  അക്ഷരം *zQuery = 0;
  അക്ഷരം *zCol = 0;
  അക്ഷരം *zNew;
  പൂർണ്ണസംഖ്യ nArg = 0;
  സ്ഥിരം അക്ഷരം *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  പൂർണ്ണസംഖ്യ nRow = 0;
  പൂർണ്ണസംഖ്യ useIdx = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ useCost = 0;
  ഇരട്ട cost = 0;
  പൂർണ്ണസംഖ്യ isIgnoreUsable = 0;
  എങ്കിൽ( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  എങ്കിൽ( simulateVtabError(pVtab, "xBestIndex") ){
    തിരികെ SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  എങ്കിൽ( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } അല്ലെങ്കിൽ {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    എങ്കിൽ( !zQuery ){
      തിരികെ SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    എങ്കിൽ( rc!=SQLITE_OK ){
      തിരികെ rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    എങ്കിൽ( rc!=SQLITE_OK ){
      തിരികെ rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  എങ്കിൽ( !zCol ) തിരികെ SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  എങ്കിൽ( !zQuery ) തിരികെ SQLITE_NOMEM;

  വേണ്ടി(ii=0; ii<pIdxInfo->nConstraint; ii++){
    സ്ഥിരം ഘടന sqlite3_index_constraint *pConstraint;
    ഘടന sqlite3_index_constraint_usage *pUsage;
    പൂർണ്ണസംഖ്യ iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    എങ്കിൽ( !isIgnoreUsable && !pConstraint->usable ) തുടരുക;

    iCol = pConstraint->iColumn;
    എങ്കിൽ( iCol<0 || pVtab->aIndex[iCol] ){
      അക്ഷരം *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      അക്ഷരം *zOp = 0;
      useIdx = 1;
      തിരഞ്ഞെടുപ്പ്( pConstraint->op ){
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; നിർത്തുക;
        അവസ്ഥ SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; നിർത്തുക;
      }
      എങ്കിൽ( zOp ){
        എങ്കിൽ( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } അല്ലെങ്കിൽ {
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
  എങ്കിൽ( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    പൂർണ്ണസംഖ്യ iCol = pIdxInfo->aOrderBy->iColumn;
    അക്ഷരം *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    അക്ഷരം *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  എങ്കിൽ( !zQuery ){
    തിരികെ rc;
  }
  pIdxInfo->idx_അക്കം = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  എങ്കിൽ( useCost ){
    pIdxInfo->estimatedCost = cost;
  }അല്ലെങ്കിൽ എങ്കിൽ( useIdx ){
    /* Approximation of log2(nRow). */
    വേണ്ടി( ii=0; ii<(വലുപ്പം(പൂർണ്ണസംഖ്യ)*8)-1; ii++ ){
      എങ്കിൽ( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (ഇരട്ട)ii;
      }
    }
  }അല്ലെങ്കിൽ{
    pIdxInfo->estimatedCost = (ഇരട്ട)nRow;
  }
  തിരികെ rc;
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
പൂർണ്ണസംഖ്യ echoUpdate(
  sqlite3_vtab *tab, 
  പൂർണ്ണസംഖ്യ nData, 
  sqlite3_മൂല്യം **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  അക്ഷരം *z = 0;               /* SQL statement to execute */
  പൂർണ്ണസംഖ്യ bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  പൂർണ്ണസംഖ്യ bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  പൂർണ്ണസംഖ്യ i;                     /* Counter variable used by for loops */

  പ്രതിജ്ഞാ( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  പ്രതിജ്ഞാ( pVtab->inTransaction );

  എങ്കിൽ( simulateVtabError(pVtab, "xUpdate") ){
    തിരികെ SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  എങ്കിൽ( nData>1 && sqlite3_മൂല്യം_type(apData[0])==SQLITE_INTEGER ){
    അക്ഷരം *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    എങ്കിൽ( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_മൂല്യം_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    എങ്കിൽ( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    വേണ്ടി(i=2; i<nData; i++){
      എങ്കിൽ( apData[i]==0 ) തുടരുക;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  അല്ലെങ്കിൽ എങ്കിൽ( nData==1 && sqlite3_മൂല്യം_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    എങ്കിൽ( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  അല്ലെങ്കിൽ എങ്കിൽ( nData>2 && sqlite3_മൂല്യം_type(apData[0])==SQLITE_NULL ){
    പൂർണ്ണസംഖ്യ ii;
    അക്ഷരം *z_തിരുകുക = 0;
    അക്ഷരം *z_മൂല്യങ്ങൾ = 0;
  
    z_തിരുകുക = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    എങ്കിൽ( !z_തിരുകുക ){
      rc = SQLITE_NOMEM;
    }
    എങ്കിൽ( sqlite3_മൂല്യം_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_മൂല്യങ്ങൾ = sqlite3_mprintf("?");
      string_concat(&z_തിരുകുക, "rowid", 0, &rc);
    }

    പ്രതിജ്ഞാ((pVtab->nCol+2)==nData);
    വേണ്ടി(ii=2; ii<nData; ii++){
      string_concat(&z_തിരുകുക, 
          sqlite3_mprintf("%s%Q", z_മൂല്യങ്ങൾ?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_മൂല്യങ്ങൾ, 
          sqlite3_mprintf("%s?%d", z_മൂല്യങ്ങൾ?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_തിരുകുക, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_മൂല്യങ്ങൾ, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ(0);
    തിരികെ SQLITE_ERROR;
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  പ്രതിജ്ഞാ( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  എങ്കിൽ( rc==SQLITE_OK ) {
    എങ്കിൽ( bindArgZero ){
      sqlite3_bind_മൂല്യം(pStmt, nData, apData[0]);
    }
    എങ്കിൽ( bindArgOne ){
      sqlite3_bind_മൂല്യം(pStmt, 1, apData[1]);
    }
    വേണ്ടി(i=2; i<nData && rc==SQLITE_OK; i++){
      എങ്കിൽ( apData[i] ) rc = sqlite3_bind_മൂല്യം(pStmt, i, apData[i]);
    }
    എങ്കിൽ( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }അല്ലെങ്കിൽ{
      sqlite3_finalize(pStmt);
    }
  }

  എങ്കിൽ( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_തിരുകുക_rowid(db);
  }
  എങ്കിൽ( rc!=SQLITE_OK ){
    tab->z_Err_സന്ദേശം = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  തിരികെ rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ echoTransactionCall(sqlite3_vtab *tab, സ്ഥിരം അക്ഷരം *zCall){
  അക്ഷരം *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  എങ്കിൽ( z==0 ) തിരികെ SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  തിരികെ SQLITE_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ echoBegin(sqlite3_vtab *tab){
  പൂർണ്ണസംഖ്യ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  സ്ഥിരം അക്ഷരം *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  പ്രതിജ്ഞാ( !pVtab->inTransaction );

  എങ്കിൽ( simulateVtabError(pVtab, "xBegin") ){
    തിരികെ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  എങ്കിൽ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    എങ്കിൽ( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  തിരികെ rc;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ echoSync(sqlite3_vtab *tab){
  പൂർണ്ണസംഖ്യ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  സ്ഥിരം അക്ഷരം *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  പ്രതിജ്ഞാ( pVtab->inTransaction );

  എങ്കിൽ( simulateVtabError(pVtab, "xSync") ){
    തിരികെ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  എങ്കിൽ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    എങ്കിൽ( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  തിരികെ rc;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  പൂർണ്ണസംഖ്യ rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  പ്രതിജ്ഞാ( pVtab->inTransaction );

  എങ്കിൽ( simulateVtabError(pVtab, "xCommit") ){
    തിരികെ SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  തിരികെ rc;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ echoRollback(sqlite3_vtab *tab){
  പൂർണ്ണസംഖ്യ rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  പ്രതിജ്ഞാ( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  തിരികെ rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
സ്ഥിതം ശൂന്യം overloadedGlobFunction(
  sqlite3_context *pContext,
  പൂർണ്ണസംഖ്യ nArg,
  sqlite3_മൂല്യം **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  വേണ്ടി(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (അക്ഷരം*)sqlite3_മൂല്യം_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_മൂല്യം(&str));
  Tcl_DStringFree(&str);
  എങ്കിൽ( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം പൂർണ്ണസംഖ്യ echoFindFunction(
  sqlite3_vtab *vtab,
  പൂർണ്ണസംഖ്യ nArg,
  സ്ഥിരം അക്ഷരം *zFuncName,
  ശൂന്യം (**pxFunc)(sqlite3_context*,പൂർണ്ണസംഖ്യ,sqlite3_മൂല്യം**),
  ശൂന്യം **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  എങ്കിൽ( strcmp(zFuncName,"glob")!=0 ){
    തിരികെ 0;
  }
  എങ്കിൽ( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    തിരികെ 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ echoRename(sqlite3_vtab *vtab, സ്ഥിരം അക്ഷരം *zNewName){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  എങ്കിൽ( simulateVtabError(p, "xRename") ){
    തിരികെ SQLITE_ERROR;
  }

  എങ്കിൽ( p->isPattern ){
    പൂർണ്ണസംഖ്യ nThis = (പൂർണ്ണസംഖ്യ)strlen(p->zThis);
    അക്ഷരം *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ echoSavepoint(sqlite3_vtab *pVTab, പൂർണ്ണസംഖ്യ iSavepoint){
  പ്രതിജ്ഞാ( pVTab );
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ echoRelease(sqlite3_vtab *pVTab, പൂർണ്ണസംഖ്യ iSavepoint){
  പ്രതിജ്ഞാ( pVTab );
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ echoRollbackTo(sqlite3_vtab *pVTab, പൂർണ്ണസംഖ്യ iSavepoint){
  പ്രതിജ്ഞാ( pVTab );
  തിരികെ SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
സ്ഥിതം sqlite3_module echoModule = {
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

സ്ഥിതം sqlite3_module echoModuleV2 = {
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
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

സ്ഥിതം ശൂന്യം moduleDestroy(ശൂന്യം *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3 *db;
  EchoModule *pMod;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(വലുപ്പം(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (ശൂന്യം*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  എങ്കിൽ( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(വലുപ്പം(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (ശൂന്യം*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
  തിരികെ TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB SQL");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (അക്ഷരം *)sqlite3_errmsg(db), TCL_VOLATILE);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  തിരികെ TCL_OK;
}
