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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା echo_vtab echo_vtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା echo_cursor echo_cursor;

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
ସଂରଚନା echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isPattern;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା inTransaction;      /* True if within a transaction */
  ଅକ୍ଷର *zThis;            /* Name of the echo table */
  ଅକ୍ଷର *zTableName;       /* Name of the real table */
  ଅକ୍ଷର *zLogName;         /* Name of the log table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;               /* Number of columns in the real table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aIndex;            /* Array of size nCol. True if column has an index */
  ଅକ୍ଷର **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ସଂରଚନା echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା simulateVtabError(echo_vtab *p, ଅଚଳ ଅକ୍ଷର *zMethod){
  ଅଚଳ ଅକ୍ଷର *zErr;
  ଅକ୍ଷର zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ଯଦି( zErr ){
    p->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  ଫେରନ୍ତୁ (zErr!=0);
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
ସ୍ଥିର ଶୂନ୍ୟ dequoteString(ଅକ୍ଷର *z){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା quote;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ଯଦି( z==0 ) ଫେରନ୍ତୁ;
  quote = z[0];
  ବାଛ( quote ){
    ସ୍ଥିତି '\'':  ଭାଙ୍ଗ;
    ସ୍ଥିତି '"':   ଭାଙ୍ଗ;
    ସ୍ଥିତି '`':   ଭାଙ୍ଗ;                /* For MySQL compatibility */
    ସ୍ଥିତି '[':   quote = ']';  ଭାଙ୍ଗ;  /* For MS SqlServer compatibility */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:    ଫେରନ୍ତୁ;
  }
  ପାଇଁ(i=1, j=0; z[i]; i++){
    ଯଦି( z[i]==quote ){
      ଯଦି( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }ଅନ୍ୟଥା{
        z[j++] = 0;
        ଭାଙ୍ଗ;
      }
    }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getColumnNames(
  sqlite3 *db, 
  ଅଚଳ ଅକ୍ଷର *zTab,
  ଅକ୍ଷର ***paCol, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnCol
){
  ଅକ୍ଷର **aCol = 0;
  ଅକ୍ଷର *zSql;
  sqlite3_stmt *pStmt = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ଯଦି( !zSql ){
    rc = SQLITE_NOMEM;
    ଯାଅ out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ଯଦି( rc==SQLITE_OK ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes;
    ଅକ୍ଷର *zSpace;
    nCol = sqlite3_column_ଗଣନା(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = ଆକାର(ଅକ୍ଷର *) * nCol;
    ପାଇଁ(ii=0; ii<nCol; ii++){
      ଅଚଳ ଅକ୍ଷର *zName = sqlite3_column_name(pStmt, ii);
      ଯଦି( !zName ){
        rc = SQLITE_NOMEM;
        ଯାଅ out;
      }
      nBytes += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zName)+1;
    }
    aCol = (ଅକ୍ଷର **)sqlite3MallocZero(nBytes);
    ଯଦି( !aCol ){
      rc = SQLITE_NOMEM;
      ଯାଅ out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (ଅକ୍ଷର *)(&aCol[nCol]);
    ପାଇଁ(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zSpace) + 1;
    }
    ପ୍ରତିଜ୍ଞା( (zSpace-nBytes)==(ଅକ୍ଷର *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getIndexArray(
  sqlite3 *db,             /* Database connection */
  ଅଚଳ ଅକ୍ଷର *zTab,        /* Name of table in database db */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା **paIndex
){
  sqlite3_stmt *pStmt = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aIndex = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅକ୍ଷର *zSql;

  /* Allocate space for the index array */
  aIndex = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)sqlite3MallocZero(ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) * nCol);
  ଯଦି( !aIndex ){
    rc = SQLITE_NOMEM;
    ଯାଅ get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ଯଦି( !zSql ){
    rc = SQLITE_NOMEM;
    ଯାଅ get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  ଯେତେବେଳେ( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    ଅଚଳ ଅକ୍ଷର *zIdx = (ଅଚଳ ଅକ୍ଷର *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ଯଦି( zIdx==0 ) ଜାରି;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ଯଦି( !zSql ){
      rc = SQLITE_NOMEM;
      ଯାଅ get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ଯଦି( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା cid = sqlite3_column_int(pStmt2, 1);
      ପ୍ରତିଜ୍ଞା( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    ଯଦି( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ଯଦି( rc!=SQLITE_OK ){
      ଯାଅ get_index_array_out;
    }
  }


get_index_array_out:
  ଯଦି( pStmt ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc2 = sqlite3_finalize(pStmt);
    ଯଦି( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ଯଦି( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  ଫେରନ୍ତୁ rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ସ୍ଥିର ଶୂନ୍ୟ appendToEchoModule(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zArg){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = (TCL_APPEND_ମୂଲ୍ୟ | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ଯଦି( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ଯଦି( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      ଯଦି( sqlite3_step(pStmt)==SQLITE_ROW ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc2;
        ଅଚଳ ଅକ୍ଷର *zCreateTable = (ଅଚଳ ଅକ୍ଷର *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        ଯଦି( rc==SQLITE_OK ){
          rc = rc2;
        }
      } ଅନ୍ୟଥା {
        rc = sqlite3_finalize(pStmt);
        ଯଦି( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ଯଦି( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      ଯଦି( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  ଫେରନ୍ତୁ rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  ଫେରନ୍ତୁ 0;
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା EchoModule EchoModule;
ସଂରଚନା EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoConstructor(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( ଆକାର(*pVtab) );
  ଯଦି( !pVtab ){
    ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ଯଦି( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ଫେରନ୍ତୁ SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ଯଦି( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    ଯଦି( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      ଅକ୍ଷର *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    ଯଦି( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  ପାଇଁ(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ଯଦି( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ଫେରନ୍ତୁ rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoCreate(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
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
  ଯଦି( rc==SQLITE_OK && argc==5 ){
    ଅକ୍ଷର *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ଯଦି( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ଯଦି( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ଯଦି( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  ଫେରନ୍ତୁ rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  ଫେରନ୍ତୁ echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  ଫେରନ୍ତୁ echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoDestroy(sqlite3_vtab *pVtab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ଯଦି( p && p->zLogName ){
    ଅକ୍ଷର *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ଯଦି( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  ଫେରନ୍ତୁ rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ଯଦି( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(ଆକାର(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  ଫେରନ୍ତୁ (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoClose(sqlite3_vtab_cursor *cur){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  ଫେରନ୍ତୁ rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoEof(sqlite3_vtab_cursor *cur){
  ଫେରନ୍ତୁ (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoNext(sqlite3_vtab_cursor *cur){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ଯଦି( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  ଯଦି( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    ଯଦି( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }ଅନ୍ୟଥା{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  ଫେରନ୍ତୁ rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ଯଦି( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  ଯଦି( !pStmt ){
    sqlite3_result_null(ctx);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( sqlite3_data_ଗଣନା(pStmt)>iCol );
    sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pStmt, iCol));
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ଯଦି( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hashString(ଅଚଳ ଅକ୍ଷର *zString){
  u32 val = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପାଇଁ(ii=0; zString[ii]; ii++){
    val = (val << 3) + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)zString[ii];
  }
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ଯଦି( simulateVtabError(pVtab, "xFilter") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  ପ୍ରତିଜ୍ଞା( idx_ଅଙ୍କ==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  ପାଇଁ(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  ପ୍ରତିଜ୍ଞା( pCur->pStmt || rc!=SQLITE_OK );
  ପାଇଁ(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_ମୂଲ୍ୟ(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ଯଦି( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ଶୂନ୍ୟ string_concat(ଅକ୍ଷର **pzStr, ଅକ୍ଷର *zAppend, ପୂର୍ଣ୍ଣସଂଖ୍ୟା doFree, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pRc){
  ଅକ୍ଷର *zIn = *pzStr;
  ଯଦି( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ଯଦି( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }ଅନ୍ୟଥା{
    ଯଦି( zIn ){
      ଅକ୍ଷର *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }ଅନ୍ୟଥା{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    ଯଦି( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  ଯଦି( doFree ){
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
ସ୍ଥିର ଅକ୍ଷର *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  ଅକ୍ଷର *zRet = 0;
  ଯଦି( sqlite3_libversion_ସଂଖ୍ୟା()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<pTab->nCol; i++){
      ଯଦି( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }ଅନ୍ୟଥା{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ଯଦି( !zRet ) ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ zRet;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ଅକ୍ଷର *zQuery = 0;
  ଅକ୍ଷର *zCol = 0;
  ଅକ୍ଷର *zNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg = 0;
  ଅଚଳ ଅକ୍ଷର *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRow = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା useIdx = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା useCost = 0;
  ଦ୍ୱିଗୁଣ cost = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isIgnoreUsable = 0;
  ଯଦି( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ଯଦି( simulateVtabError(pVtab, "xBestIndex") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ଯଦି( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } ଅନ୍ୟଥା {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    ଯଦି( !zQuery ){
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ଯଦି( rc!=SQLITE_OK ){
      ଫେରନ୍ତୁ rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ଯଦି( rc!=SQLITE_OK ){
      ଫେରନ୍ତୁ rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  ଯଦି( !zCol ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  ଯଦି( !zQuery ) ଫେରନ୍ତୁ SQLITE_NOMEM;

  ପାଇଁ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ଅଚଳ ସଂରଚନା sqlite3_index_constraint *pConstraint;
    ସଂରଚନା sqlite3_index_constraint_usage *pUsage;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ଯଦି( !isIgnoreUsable && !pConstraint->usable ) ଜାରି;

    iCol = pConstraint->iColumn;
    ଯଦି( iCol<0 || pVtab->aIndex[iCol] ){
      ଅକ୍ଷର *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      ଅକ୍ଷର *zOp = 0;
      useIdx = 1;
      ବାଛ( pConstraint->op ){
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; ଭାଙ୍ଗ;
        ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; ଭାଙ୍ଗ;
      }
      ଯଦି( zOp ){
        ଯଦି( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } ଅନ୍ୟଥା {
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
  ଯଦି( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = pIdxInfo->aOrderBy->iColumn;
    ଅକ୍ଷର *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    ଅକ୍ଷର *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  ଯଦି( !zQuery ){
    ଫେରନ୍ତୁ rc;
  }
  pIdxInfo->idx_ଅଙ୍କ = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  ଯଦି( useCost ){
    pIdxInfo->estimatedCost = cost;
  }ଅନ୍ୟଥା ଯଦି( useIdx ){
    /* Approximation of log2(nRow). */
    ପାଇଁ( ii=0; ii<(ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*8)-1; ii++ ){
      ଯଦି( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (ଦ୍ୱିଗୁଣ)ii;
      }
    }
  }ଅନ୍ୟଥା{
    pIdxInfo->estimatedCost = (ଦ୍ୱିଗୁଣ)nRow;
  }
  ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoUpdate(
  sqlite3_vtab *tab, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData, 
  sqlite3_ମୂଲ୍ୟ **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  ଅକ୍ଷର *z = 0;               /* SQL statement to execute */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                     /* Counter variable used by for loops */

  ପ୍ରତିଜ୍ଞା( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  ପ୍ରତିଜ୍ଞା( pVtab->inTransaction );

  ଯଦି( simulateVtabError(pVtab, "xUpdate") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ଯଦି( nData>1 && sqlite3_ମୂଲ୍ୟ_type(apData[0])==SQLITE_INTEGER ){
    ଅକ୍ଷର *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    ଯଦି( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_ମୂଲ୍ୟ_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ଯଦି( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    ପାଇଁ(i=2; i<nData; i++){
      ଯଦି( apData[i]==0 ) ଜାରି;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  ଅନ୍ୟଥା ଯଦି( nData==1 && sqlite3_ମୂଲ୍ୟ_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    ଯଦି( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  ଅନ୍ୟଥା ଯଦି( nData>2 && sqlite3_ମୂଲ୍ୟ_type(apData[0])==SQLITE_NULL ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ଅକ୍ଷର *z_ଭର୍ତ୍ତି = 0;
    ଅକ୍ଷର *z_ମୂଲ୍ୟଗୁଡ଼ିକ = 0;
  
    z_ଭର୍ତ୍ତି = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    ଯଦି( !z_ଭର୍ତ୍ତି ){
      rc = SQLITE_NOMEM;
    }
    ଯଦି( sqlite3_ମୂଲ୍ୟ_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_ମୂଲ୍ୟଗୁଡ଼ିକ = sqlite3_mprintf("?");
      string_concat(&z_ଭର୍ତ୍ତି, "rowid", 0, &rc);
    }

    ପ୍ରତିଜ୍ଞା((pVtab->nCol+2)==nData);
    ପାଇଁ(ii=2; ii<nData; ii++){
      string_concat(&z_ଭର୍ତ୍ତି, 
          sqlite3_mprintf("%s%Q", z_ମୂଲ୍ୟଗୁଡ଼ିକ?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_ମୂଲ୍ୟଗୁଡ଼ିକ, 
          sqlite3_mprintf("%s?%d", z_ମୂଲ୍ୟଗୁଡ଼ିକ?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_ଭର୍ତ୍ତି, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_ମୂଲ୍ୟଗୁଡ଼ିକ, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା(0);
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  ପ୍ରତିଜ୍ଞା( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ଯଦି( rc==SQLITE_OK ) {
    ଯଦି( bindArgZero ){
      sqlite3_bind_ମୂଲ୍ୟ(pStmt, nData, apData[0]);
    }
    ଯଦି( bindArgOne ){
      sqlite3_bind_ମୂଲ୍ୟ(pStmt, 1, apData[1]);
    }
    ପାଇଁ(i=2; i<nData && rc==SQLITE_OK; i++){
      ଯଦି( apData[i] ) rc = sqlite3_bind_ମୂଲ୍ୟ(pStmt, i, apData[i]);
    }
    ଯଦି( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }ଅନ୍ୟଥା{
      sqlite3_finalize(pStmt);
    }
  }

  ଯଦି( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_ଭର୍ତ୍ତି_rowid(db);
  }
  ଯଦି( rc!=SQLITE_OK ){
    tab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  ଫେରନ୍ତୁ rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoTransactionCall(sqlite3_vtab *tab, ଅଚଳ ଅକ୍ଷର *zCall){
  ଅକ୍ଷର *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  ଯଦି( z==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  ଫେରନ୍ତୁ SQLITE_OK;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoBegin(sqlite3_vtab *tab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ଅଚଳ ଅକ୍ଷର *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  ପ୍ରତିଜ୍ଞା( !pVtab->inTransaction );

  ଯଦି( simulateVtabError(pVtab, "xBegin") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ଯଦି( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ଯଦି( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  ଯଦି( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoSync(sqlite3_vtab *tab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ଅଚଳ ଅକ୍ଷର *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  ପ୍ରତିଜ୍ଞା( pVtab->inTransaction );

  ଯଦି( simulateVtabError(pVtab, "xSync") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ଯଦି( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ଯଦି( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  ପ୍ରତିଜ୍ଞା( pVtab->inTransaction );

  ଯଦି( simulateVtabError(pVtab, "xCommit") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoRollback(sqlite3_vtab *tab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  ପ୍ରତିଜ୍ଞା( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  ଫେରନ୍ତୁ rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ସ୍ଥିର ଶୂନ୍ୟ overloadedGlobFunction(
  sqlite3_context *pContext,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  ପାଇଁ(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_ମୂଲ୍ୟ(&str));
  Tcl_DStringFree(&str);
  ଯଦି( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoFindFunction(
  sqlite3_vtab *vtab,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  ଅଚଳ ଅକ୍ଷର *zFuncName,
  ଶୂନ୍ୟ (**pxFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
  ଶୂନ୍ୟ **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ଯଦି( strcmp(zFuncName,"glob")!=0 ){
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    ଫେରନ୍ତୁ 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoRename(sqlite3_vtab *vtab, ଅଚଳ ଅକ୍ଷର *zNewName){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ଯଦି( simulateVtabError(p, "xRename") ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  ଯଦି( p->isPattern ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nThis = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(p->zThis);
    ଅକ୍ଷର *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoSavepoint(sqlite3_vtab *pVTab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSavepoint){
  ପ୍ରତିଜ୍ଞା( pVTab );
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoRelease(sqlite3_vtab *pVTab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSavepoint){
  ପ୍ରତିଜ୍ଞା( pVTab );
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା echoRollbackTo(sqlite3_vtab *pVTab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSavepoint){
  ପ୍ରତିଜ୍ଞା( pVTab );
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ସ୍ଥିର sqlite3_module echoModule = {
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

ସ୍ଥିର sqlite3_module echoModuleV2 = {
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
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

ସ୍ଥିର ଶୂନ୍ୟ moduleDestroy(ଶୂନ୍ୟ *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3 *db;
  EchoModule *pMod;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(ଆକାର(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (ଶୂନ୍ୟ*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ଯଦି( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(ଆକାର(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (ଶୂନ୍ୟ*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB SQL");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3_errmsg(db), TCL_VOLATILE);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ଶୂନ୍ୟ *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}
