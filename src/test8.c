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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE

రకనిర్వచనం నిర్మాణం echo_vtab echo_vtab;
రకనిర్వచనం నిర్మాణం echo_cursor echo_cursor;

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
నిర్మాణం echo_vtab {
  sqlite3_vtab పునాది;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  పూర్ణాంకం is_నమూనా;
  పూర్ణాంకం లోపల_Transaction;      /* True if within a transaction */
  అక్షరం *zThis;            /* Name of the echo table */
  అక్షరం *z_పట్టిక_Name;       /* Name of the real table */
  అక్షరం *zLogName;         /* Name of the log table */
  పూర్ణాంకం n_నిలువు;               /* Number of columns in the real table */
  పూర్ణాంకం *a_సూచీ;            /* Array of size nCol. True if column has an index */
  అక్షరం **a_నిలువు;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
నిర్మాణం echo_cursor {
  sqlite3_vtab_cursor పునాది;
  sqlite3_stmt *pStmt;
};

స్థిర పూర్ణాంకం simulateVtabError(echo_vtab *p, స్థిరరాశి అక్షరం *z_పద్ధతి){
  స్థిరరాశి అక్షరం *zErr;
  అక్షరం zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", z_పద్ధతి, p->z_పట్టిక_Name);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ఒకవేళ( zErr ){
    p->పునాది.z_Err_సందేశం = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  తిరిగి (zErr!=0);
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
స్థిర శూన్య dequoteString(అక్షరం *z){
  పూర్ణాంకం quote;
  పూర్ణాంకం i, j;
  ఒకవేళ( z==0 ) తిరిగి;
  quote = z[0];
  ఎంపిక( quote ){
    స్థితి '\'':  ఆపు;
    స్థితి '"':   ఆపు;
    స్థితి '`':   ఆపు;                /* For MySQL compatibility */
    స్థితి '[':   quote = ']';  ఆపు;  /* For MS SqlServer compatibility */
    అప్రమేయ:    తిరిగి;
  }
  కొరకు(i=1, j=0; z[i]; i++){
    ఒకవేళ( z[i]==quote ){
      ఒకవేళ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }లేకపోతే{
        z[j++] = 0;
        ఆపు;
      }
    }లేకపోతే{
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
స్థిర పూర్ణాంకం get_స్తంభం_పేర్లు(
  sqlite3 *db, 
  స్థిరరాశి అక్షరం *zTab,
  అక్షరం ***pa_నిలువు, 
  పూర్ణాంకం *pn_నిలువు
){
  అక్షరం **a_నిలువు = 0;
  అక్షరం *zSql;
  sqlite3_stmt *pStmt = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం n_నిలువు = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ఒకవేళ( !zSql ){
    rc = SQLITE_NOMEM;
    వెళ్ళు బహిర్గతం;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం ii;
    పూర్ణాంకం n_బైట్లు;
    అక్షరం *zSpace;
    n_నిలువు = sqlite3_స్తంభం_లెక్క(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    n_బైట్లు = పరిమాణం(అక్షరం *) * n_నిలువు;
    కొరకు(ii=0; ii<n_నిలువు; ii++){
      స్థిరరాశి అక్షరం *zName = sqlite3_స్తంభం_name(pStmt, ii);
      ఒకవేళ( !zName ){
        rc = SQLITE_NOMEM;
        వెళ్ళు బహిర్గతం;
      }
      n_బైట్లు += (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zName)+1;
    }
    a_నిలువు = (అక్షరం **)sqlite3MallocZero(n_బైట్లు);
    ఒకవేళ( !a_నిలువు ){
      rc = SQLITE_NOMEM;
      వెళ్ళు బహిర్గతం;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (అక్షరం *)(&a_నిలువు[n_నిలువు]);
    కొరకు(ii=0; ii<n_నిలువు; ii++){
      a_నిలువు[ii] = zSpace;
      sqlite3_snprintf(n_బైట్లు, zSpace, "%s", sqlite3_స్తంభం_name(pStmt,ii));
      zSpace += (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zSpace) + 1;
    }
    ప్రతిజ్ఞా( (zSpace-n_బైట్లు)==(అక్షరం *)a_నిలువు );
  }

  *pa_నిలువు = a_నిలువు;
  *pn_నిలువు = n_నిలువు;

బహిర్గతం:
  sqlite3_finalize(pStmt);
  తిరిగి rc;
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
స్థిర పూర్ణాంకం get_సూచీ_Array(
  sqlite3 *db,             /* Database connection */
  స్థిరరాశి అక్షరం *zTab,        /* Name of table in database db */
  పూర్ణాంకం n_నిలువు,
  పూర్ణాంకం **pa_సూచీ
){
  sqlite3_stmt *pStmt = 0;
  పూర్ణాంకం *a_సూచీ = 0;
  పూర్ణాంకం rc;
  అక్షరం *zSql;

  /* Allocate space for the index array */
  a_సూచీ = (పూర్ణాంకం *)sqlite3MallocZero(పరిమాణం(పూర్ణాంకం) * n_నిలువు);
  ఒకవేళ( !a_సూచీ ){
    rc = SQLITE_NOMEM;
    వెళ్ళు get_సూచీ_array_బహిర్గతం;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ఒకవేళ( !zSql ){
    rc = SQLITE_NOMEM;
    వెళ్ళు get_సూచీ_array_బహిర్గతం;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  అయితే( pStmt && sqlite3_అడుగు(pStmt)==SQLITE_అడ్డువరుస ){
    స్థిరరాశి అక్షరం *zIdx = (స్థిరరాశి అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ఒకవేళ( zIdx==0 ) కొనసాగు;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ఒకవేళ( !zSql ){
      rc = SQLITE_NOMEM;
      వెళ్ళు get_సూచీ_array_బహిర్గతం;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ఒకవేళ( pStmt2 && sqlite3_అడుగు(pStmt2)==SQLITE_అడ్డువరుస ){
      పూర్ణాంకం cid = sqlite3_స్తంభం_int(pStmt2, 1);
      ప్రతిజ్ఞా( cid>=0 && cid<n_నిలువు );
      a_సూచీ[cid] = 1;
    }
    ఒకవేళ( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు get_సూచీ_array_బహిర్గతం;
    }
  }


get_సూచీ_array_బహిర్గతం:
  ఒకవేళ( pStmt ){
    పూర్ణాంకం rc2 = sqlite3_finalize(pStmt);
    ఒకవేళ( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_free(a_సూచీ);
    a_సూచీ = 0;
  }
  *pa_సూచీ = a_సూచీ;
  తిరిగి rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
స్థిర శూన్య append_గమ్యం_Echo_Module(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zArg){
  పూర్ణాంకం flags = (TCL_APPEND_విలువ | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_సమితి_Var(interp, "echo_module", (zArg?zArg:""), flags);
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
స్థిర పూర్ణాంకం echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( pVtab->z_పట్టిక_Name ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_bind_పాఠ్యం(pStmt, 1, pVtab->z_పట్టిక_Name, -1, 0);
      ఒకవేళ( sqlite3_అడుగు(pStmt)==SQLITE_అడ్డువరుస ){
        పూర్ణాంకం rc2;
        స్థిరరాశి అక్షరం *z_Create_పట్టిక = (స్థిరరాశి అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pStmt, 0);
        rc = sqlite3_declare_vtab(db, z_Create_పట్టిక);
        rc2 = sqlite3_finalize(pStmt);
        ఒకవేళ( rc==SQLITE_OK ){
          rc = rc2;
        }
      } లేకపోతే {
        rc = sqlite3_finalize(pStmt);
        ఒకవేళ( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ఒకవేళ( rc==SQLITE_OK ){
        rc = get_స్తంభం_పేర్లు(db, pVtab->z_పట్టిక_Name, &pVtab->a_నిలువు, &pVtab->n_నిలువు);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        rc = get_సూచీ_Array(db, pVtab->z_పట్టిక_Name, pVtab->n_నిలువు, &pVtab->a_సూచీ);
      }
    }
  }

  తిరిగి rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
స్థిర పూర్ణాంకం echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_సూచీ);
  sqlite3_free(p->a_నిలువు);
  sqlite3_free(p->zThis);
  sqlite3_free(p->z_పట్టిక_Name);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  తిరిగి 0;
}

రకనిర్వచనం నిర్మాణం EchoModule EchoModule;
నిర్మాణం EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
స్థిర పూర్ణాంకం echoConstructor(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  పూర్ణాంకం rc;
  పూర్ణాంకం i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( పరిమాణం(*pVtab) );
  ఒకవేళ( !pVtab ){
    తిరిగి SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ఒకవేళ( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    తిరిగి SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ఒకవేళ( argc>3 ){
    pVtab->z_పట్టిక_Name = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->z_పట్టిక_Name);
    ఒకవేళ( pVtab->z_పట్టిక_Name && pVtab->z_పట్టిక_Name[0]=='*' ){
      అక్షరం *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->z_పట్టిక_Name[1]));
      sqlite3_free(pVtab->z_పట్టిక_Name);
      pVtab->z_పట్టిక_Name = z;
      pVtab->is_నమూనా = 1;
    }
    ఒకవేళ( !pVtab->z_పట్టిక_Name ){
      echoDestructor((sqlite3_vtab *)pVtab);
      తిరిగి SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  కొరకు(i=0; i<argc; i++){
    append_గమ్యం_Echo_Module(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ఒకవేళ( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    తిరిగి rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->పునాది;
  తిరిగి SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
స్థిర పూర్ణాంకం echoCreate(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  పూర్ణాంకం rc = SQLITE_OK;
  append_గమ్యం_Echo_Module(((EchoModule *)pAux)->interp, "xCreate");
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
  ఒకవేళ( rc==SQLITE_OK && argc==5 ){
    అక్షరం *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ఒకవేళ( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ఒకవేళ( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->లోపల_Transaction = 1;
  }

  తిరిగి rc;
}

/* 
** Echo virtual table module xConnect method.
*/
స్థిర పూర్ణాంకం echoConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  append_గమ్యం_Echo_Module(((EchoModule *)pAux)->interp, "xConnect");
  తిరిగి echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
స్థిర పూర్ణాంకం echoDisconnect(sqlite3_vtab *pVtab){
  append_గమ్యం_Echo_Module(((echo_vtab *)pVtab)->interp, "xDisconnect");
  తిరిగి echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
స్థిర పూర్ణాంకం echoDestroy(sqlite3_vtab *pVtab){
  పూర్ణాంకం rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  append_గమ్యం_Echo_Module(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ఒకవేళ( p && p->zLogName ){
    అక్షరం *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  తిరిగి rc;
}

/* 
** Echo virtual table module xOpen method.
*/
స్థిర పూర్ణాంకం echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ఒకవేళ( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    తిరిగి SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(పరిమాణం(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  తిరిగి (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
స్థిర పూర్ణాంకం echoClose(sqlite3_vtab_cursor *cur){
  పూర్ణాంకం rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  తిరిగి rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
స్థిర పూర్ణాంకం echoEof(sqlite3_vtab_cursor *cur){
  తిరిగి (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
స్థిర పూర్ణాంకం echo_తదుపరి(sqlite3_vtab_cursor *cur){
  పూర్ణాంకం rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ఒకవేళ( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( pCur->pStmt ){
    rc = sqlite3_అడుగు(pCur->pStmt);
    ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
      rc = SQLITE_OK;
    }లేకపోతే{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  తిరిగి rc;
}

/* 
** Echo virtual table module xColumn method.
*/
స్థిర పూర్ణాంకం echo_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  పూర్ణాంకం i_నిలువు = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ఒకవేళ( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( !pStmt ){
    sqlite3_ఫలితం_null(ctx);
  }లేకపోతే{
    ప్రతిజ్ఞా( sqlite3_దత్తాంశం_లెక్క(pStmt)>i_నిలువు );
    sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pStmt, i_నిలువు));
  }
  తిరిగి SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
స్థిర పూర్ణాంకం echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ఒకవేళ( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    తిరిగి SQLITE_ERROR;
  }

  *pRowid = sqlite3_స్తంభం_int64(pStmt, 0);
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం hashString(స్థిరరాశి అక్షరం *zString){
  u32 val = 0;
  పూర్ణాంకం ii;
  కొరకు(ii=0; zString[ii]; ii++){
    val = (val << 3) + (పూర్ణాంకం)zString[ii];
  }
  తిరిగి (పూర్ణాంకం)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
స్థిర పూర్ణాంకం echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  పూర్ణాంకం rc;
  పూర్ణాంకం i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ఒకవేళ( simulateVtabError(pVtab, "xFilter") ){
    తిరిగి SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  ప్రతిజ్ఞా( idx_అంకె==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  append_గమ్యం_Echo_Module(pVtab->interp, "xFilter");
  append_గమ్యం_Echo_Module(pVtab->interp, idxStr);
  కొరకు(i=0; i<argc; i++){
    append_గమ్యం_Echo_Module(pVtab->interp, (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  ప్రతిజ్ఞా( pCur->pStmt || rc!=SQLITE_OK );
  కొరకు(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_విలువ(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ఒకవేళ( rc==SQLITE_OK ){
    rc = echo_తదుపరి(pVtabCursor);
  }

  తిరిగి rc;
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
స్థిర శూన్య string_concat(అక్షరం **pzStr, అక్షరం *zAppend, పూర్ణాంకం doFree, పూర్ణాంకం *pRc){
  అక్షరం *z_లోపల = *pzStr;
  ఒకవేళ( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ఒకవేళ( *pRc!=SQLITE_OK ){
    sqlite3_free(z_లోపల);
    z_లోపల = 0;
  }లేకపోతే{
    ఒకవేళ( z_లోపల ){
      అక్షరం *z_తాత్కాలిక = z_లోపల;
      z_లోపల = sqlite3_mprintf("%s%s", z_లోపల, zAppend);
      sqlite3_free(z_తాత్కాలిక);
    }లేకపోతే{
      z_లోపల = sqlite3_mprintf("%s", zAppend);
    }
    ఒకవేళ( !z_లోపల ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = z_లోపల;
  ఒకవేళ( doFree ){
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
స్థిర అక్షరం *echoSelectList(echo_vtab *pTab, sqlite3_సూచీ_info *pIdxInfo){
  అక్షరం *zRet = 0;
  ఒకవేళ( sqlite3_libversion_సంఖ్య()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }లేకపోతే{
    పూర్ణాంకం i;
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      ఒకవేళ( pIdxInfo->నిలువు_వాడిన & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->a_నిలువు[i]);
      }లేకపోతే{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ఒకవేళ( !zRet ) ఆపు;
    }
  }
  తిరిగి zRet;
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
స్థిర పూర్ణాంకం echo_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  పూర్ణాంకం ii;
  అక్షరం *zQuery = 0;
  అక్షరం *z_నిలువు = 0;
  అక్షరం *zNew;
  పూర్ణాంకం nArg = 0;
  స్థిరరాశి అక్షరం *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  పూర్ణాంకం n_అడ్డువరుస = 0;
  పూర్ణాంకం useIdx = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం use_ఖర్చు = 0;
  ద్విగుణ ఖర్చు = 0;
  పూర్ణాంకం isIgnoreUsable = 0;
  ఒకవేళ( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ఒకవేళ( simulateVtabError(pVtab, "xBestIndex") ){
    తిరిగి SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ఒకవేళ( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    ఖర్చు = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    use_ఖర్చు = 1;
  } లేకపోతే {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->z_పట్టిక_Name);
    ఒకవేళ( !zQuery ){
      తిరిగి SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    sqlite3_అడుగు(pStmt);
    n_అడ్డువరుస = sqlite3_స్తంభం_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
  }

  z_నిలువు = echoSelectList(pVtab, pIdxInfo);
  ఒకవేళ( !z_నిలువు ) తిరిగి SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", z_నిలువు, pVtab->z_పట్టిక_Name);
  ఒకవేళ( !zQuery ) తిరిగి SQLITE_NOMEM;

  కొరకు(ii=0; ii<pIdxInfo->nConstraint; ii++){
    స్థిరరాశి నిర్మాణం sqlite3_సూచీ_constraint *pConstraint;
    నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage;
    పూర్ణాంకం i_నిలువు;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ఒకవేళ( !isIgnoreUsable && !pConstraint->usable ) కొనసాగు;

    i_నిలువు = pConstraint->i_స్తంభం;
    ఒకవేళ( i_నిలువు<0 || pVtab->a_సూచీ[i_నిలువు] ){
      అక్షరం *z_New_నిలువు = i_నిలువు>=0 ? pVtab->a_నిలువు[i_నిలువు] : "rowid";
      అక్షరం *zOp = 0;
      useIdx = 1;
      ఎంపిక( pConstraint->op ){
        స్థితి SQLITE_సూచీ_CONSTRAINT_EQ:
          zOp = "="; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_LT:
          zOp = "<"; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_GT:
          zOp = ">"; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_LE:
          zOp = "<="; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_GE:
          zOp = ">="; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_సరిపోలిక:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_LIKE:
          zOp = "like"; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_GLOB:
          zOp = "glob"; ఆపు;
        స్థితి SQLITE_సూచీ_CONSTRAINT_REGEXP:
          zOp = "regexp"; ఆపు;
      }
      ఒకవేళ( zOp ){
        ఒకవేళ( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, z_New_నిలువు);
        } లేకపోతే {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, z_New_నిలువు, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_సూచీ = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  ఒకవేళ( pIdxInfo->n_క్రమం_By==1 && (
        pIdxInfo->a_క్రమం_By->i_స్తంభం<0 ||
        pVtab->a_సూచీ[pIdxInfo->a_క్రమం_By->i_స్తంభం]) ){
    పూర్ణాంకం i_నిలువు = pIdxInfo->a_క్రమం_By->i_స్తంభం;
    అక్షరం *z_New_నిలువు = i_నిలువు>=0 ? pVtab->a_నిలువు[i_నిలువు] : "rowid";
    అక్షరం *zDir = pIdxInfo->a_క్రమం_By->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", z_New_నిలువు, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->క్రమం_By_Consumed = 1;
  }

  append_గమ్యం_Echo_Module(pVtab->interp, "xBestIndex");;
  append_గమ్యం_Echo_Module(pVtab->interp, zQuery);

  ఒకవేళ( !zQuery ){
    తిరిగి rc;
  }
  pIdxInfo->idx_అంకె = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->need_గమ్యం_Free_Idx_Str = 1;
  ఒకవేళ( use_ఖర్చు ){
    pIdxInfo->estimated_ఖర్చు = ఖర్చు;
  }లేకపోతే ఒకవేళ( useIdx ){
    /* Approximation of log2(nRow). */
    కొరకు( ii=0; ii<(పరిమాణం(పూర్ణాంకం)*8)-1; ii++ ){
      ఒకవేళ( n_అడ్డువరుస & (1<<ii) ){
        pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)ii;
      }
    }
  }లేకపోతే{
    pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)n_అడ్డువరుస;
  }
  తిరిగి rc;
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
పూర్ణాంకం echoUpdate(
  sqlite3_vtab *tab, 
  పూర్ణాంకం n_దత్తాంశం, 
  sqlite3_విలువ **ap_దత్తాంశం, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  పూర్ణాంకం rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  అక్షరం *z = 0;               /* SQL statement to execute */
  పూర్ణాంకం bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  పూర్ణాంకం bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  పూర్ణాంకం i;                     /* Counter variable used by for loops */

  ప్రతిజ్ఞా( n_దత్తాంశం==pVtab->n_నిలువు+2 || n_దత్తాంశం==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  ప్రతిజ్ఞా( pVtab->లోపల_Transaction );

  ఒకవేళ( simulateVtabError(pVtab, "xUpdate") ){
    తిరిగి SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ఒకవేళ( n_దత్తాంశం>1 && sqlite3_విలువ_type(ap_దత్తాంశం[0])==SQLITE_INTEGER ){
    అక్షరం *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->z_పట్టిక_Name);
    ఒకవేళ( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_దత్తాంశం[1] && sqlite3_విలువ_type(ap_దత్తాంశం[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ఒకవేళ( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    కొరకు(i=2; i<n_దత్తాంశం; i++){
      ఒకవేళ( ap_దత్తాంశం[i]==0 ) కొనసాగు;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->a_నిలువు[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_దత్తాంశం), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  లేకపోతే ఒకవేళ( n_దత్తాంశం==1 && sqlite3_విలువ_type(ap_దత్తాంశం[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->z_పట్టిక_Name);
    ఒకవేళ( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  లేకపోతే ఒకవేళ( n_దత్తాంశం>2 && sqlite3_విలువ_type(ap_దత్తాంశం[0])==SQLITE_NULL ){
    పూర్ణాంకం ii;
    అక్షరం *z_చొప్పించు = 0;
    అక్షరం *z_విలువలు = 0;
  
    z_చొప్పించు = sqlite3_mprintf("INSERT INTO %Q (", pVtab->z_పట్టిక_Name);
    ఒకవేళ( !z_చొప్పించు ){
      rc = SQLITE_NOMEM;
    }
    ఒకవేళ( sqlite3_విలువ_type(ap_దత్తాంశం[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_విలువలు = sqlite3_mprintf("?");
      string_concat(&z_చొప్పించు, "rowid", 0, &rc);
    }

    ప్రతిజ్ఞా((pVtab->n_నిలువు+2)==n_దత్తాంశం);
    కొరకు(ii=2; ii<n_దత్తాంశం; ii++){
      string_concat(&z_చొప్పించు, 
          sqlite3_mprintf("%s%Q", z_విలువలు?", ":"", pVtab->a_నిలువు[ii-2]), 1, &rc);
      string_concat(&z_విలువలు, 
          sqlite3_mprintf("%s?%d", z_విలువలు?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_చొప్పించు, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_విలువలు, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  లేకపోతే{
    ప్రతిజ్ఞా(0);
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  ప్రతిజ్ఞా( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ఒకవేళ( rc==SQLITE_OK ) {
    ఒకవేళ( bindArgZero ){
      sqlite3_bind_విలువ(pStmt, n_దత్తాంశం, ap_దత్తాంశం[0]);
    }
    ఒకవేళ( bindArgOne ){
      sqlite3_bind_విలువ(pStmt, 1, ap_దత్తాంశం[1]);
    }
    కొరకు(i=2; i<n_దత్తాంశం && rc==SQLITE_OK; i++){
      ఒకవేళ( ap_దత్తాంశం[i] ) rc = sqlite3_bind_విలువ(pStmt, i, ap_దత్తాంశం[i]);
    }
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_అడుగు(pStmt);
      rc = sqlite3_finalize(pStmt);
    }లేకపోతే{
      sqlite3_finalize(pStmt);
    }
  }

  ఒకవేళ( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_చివరి_చొప్పించు_rowid(db);
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    tab->z_Err_సందేశం = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  తిరిగి rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
స్థిర పూర్ణాంకం echoTransactionCall(sqlite3_vtab *tab, స్థిరరాశి అక్షరం *zCall){
  అక్షరం *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->z_పట్టిక_Name);
  ఒకవేళ( z==0 ) తిరిగి SQLITE_NOMEM;
  append_గమ్యం_Echo_Module(pVtab->interp, zCall);
  append_గమ్యం_Echo_Module(pVtab->interp, z);
  sqlite3_free(z);
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం echoBegin(sqlite3_vtab *tab){
  పూర్ణాంకం rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  స్థిరరాశి అక్షరం *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  ప్రతిజ్ఞా( !pVtab->లోపల_Transaction );

  ఒకవేళ( simulateVtabError(pVtab, "xBegin") ){
    తిరిగి SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ఒకవేళ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ఒకవేళ( zVal && 0==strcmp(zVal, pVtab->z_పట్టిక_Name) ){
      rc = SQLITE_ERROR;
    }
  }
  ఒకవేళ( rc==SQLITE_OK ){
    pVtab->లోపల_Transaction = 1;
  }
  తిరిగి rc;
}
స్థిర పూర్ణాంకం echoSync(sqlite3_vtab *tab){
  పూర్ణాంకం rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  స్థిరరాశి అక్షరం *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  ప్రతిజ్ఞా( pVtab->లోపల_Transaction );

  ఒకవేళ( simulateVtabError(pVtab, "xSync") ){
    తిరిగి SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ఒకవేళ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ఒకవేళ( zVal && 0==strcmp(zVal, pVtab->z_పట్టిక_Name) ){
      rc = -1;
    }
  }
  తిరిగి rc;
}
స్థిర పూర్ణాంకం echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  పూర్ణాంకం rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  ప్రతిజ్ఞా( pVtab->లోపల_Transaction );

  ఒకవేళ( simulateVtabError(pVtab, "xCommit") ){
    తిరిగి SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3_చివర_Benign_Malloc();
  pVtab->లోపల_Transaction = 0;
  తిరిగి rc;
}
స్థిర పూర్ణాంకం echoRollback(sqlite3_vtab *tab){
  పూర్ణాంకం rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  ప్రతిజ్ఞా( pVtab->లోపల_Transaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->లోపల_Transaction = 0;
  తిరిగి rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
స్థిర శూన్య overloadedGlobFunction(
  sqlite3_context *pContext,
  పూర్ణాంకం nArg,
  sqlite3_విలువ **apArg
){
  Tcl_Interp *interp = sqlite3_user_దత్తాంశం(pContext);
  Tcl_DString str;
  పూర్ణాంకం i;
  పూర్ణాంకం rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  కొరకు(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (అక్షరం*)sqlite3_విలువ_పాఠ్యం(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_విలువ(&str));
  Tcl_DStringFree(&str);
  ఒకవేళ( rc ){
    sqlite3_ఫలితం_error(pContext, Tcl_Get_String_ఫలితం(interp), -1);
  }లేకపోతే{
    sqlite3_ఫలితం_పాఠ్యం(pContext, Tcl_Get_String_ఫలితం(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_ఫలితం(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
స్థిర పూర్ణాంకం echo_కనుగొను_Function(
  sqlite3_vtab *vtab,
  పూర్ణాంకం nArg,
  స్థిరరాశి అక్షరం *zFuncName,
  శూన్య (**pxFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
  శూన్య **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ఒకవేళ( strcmp(zFuncName,"glob")!=0 ){
    తిరిగి 0;
  }
  ఒకవేళ( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    తిరిగి 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  తిరిగి 1;
}

స్థిర పూర్ణాంకం echoRename(sqlite3_vtab *vtab, స్థిరరాశి అక్షరం *zNewName){
  పూర్ణాంకం rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ఒకవేళ( simulateVtabError(p, "xRename") ){
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( p->is_నమూనా ){
    పూర్ణాంకం nThis = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(p->zThis);
    అక్షరం *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->z_పట్టిక_Name, zNewName, &p->z_పట్టిక_Name[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  తిరిగి rc;
}

స్థిర పూర్ణాంకం echoSavepoint(sqlite3_vtab *pVTab, పూర్ణాంకం iSavepoint){
  ప్రతిజ్ఞా( pVTab );
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం echoRelease(sqlite3_vtab *pVTab, పూర్ణాంకం iSavepoint){
  ప్రతిజ్ఞా( pVTab );
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం echo_Rollback_గమ్యం(sqlite3_vtab *pVTab, పూర్ణాంకం iSavepoint){
  ప్రతిజ్ఞా( pVTab );
  తిరిగి SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
స్థిర sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_ఉత్తమ_సూచీ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_తదుపరి,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_స్తంభం,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_కనుగొను_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};

స్థిర sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_ఉత్తమ_సూచీ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_తదుపరి,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_స్తంభం,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_కనుగొను_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  echoSavepoint,
  echoRelease,
  echo_Rollback_గమ్యం,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

స్థిర శూన్య moduleDestroy(శూన్య *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_echo_module(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  పూర్ణాంకం rc;
  sqlite3 *db;
  EchoModule *pMod;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(పరిమాణం(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (శూన్య*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ఒకవేళ( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(పరిమాణం(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (శూన్య*)pMod, moduleDestroy
    );
  }

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
  తిరిగి TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI declare_vtab(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  పూర్ణాంకం rc;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB SQL");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3_errmsg(db), TCL_VOLATILE);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest8_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "register_echo_module",       రిజిస్టరు_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}
