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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ echo_vtab echo_vtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ echo_cursor echo_cursor;

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
ಸಂರಚನಾ echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  ಪೂರ್ಣಾಙ್ಕ isPattern;
  ಪೂರ್ಣಾಙ್ಕ inTransaction;      /* True if within a transaction */
  ವರ್ಣ *zThis;            /* Name of the echo table */
  ವರ್ಣ *zTableName;       /* Name of the real table */
  ವರ್ಣ *zLogName;         /* Name of the log table */
  ಪೂರ್ಣಾಙ್ಕ nCol;               /* Number of columns in the real table */
  ಪೂರ್ಣಾಙ್ಕ *aIndex;            /* Array of size nCol. True if column has an index */
  ವರ್ಣ **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ಸಂರಚನಾ echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ simulateVtabError(echo_vtab *p, ಧ್ರುವ ವರ್ಣ *zMethod){
  ಧ್ರುವ ವರ್ಣ *zErr;
  ವರ್ಣ zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ಯದಿ( zErr ){
    p->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  ಪ್ರತ್ಯಾಗಮ (zErr!=0);
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
ಸ್ಥಿರ ಶೂನ್ಯ dequoteString(ವರ್ಣ *z){
  ಪೂರ್ಣಾಙ್ಕ quote;
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ;
  quote = z[0];
  ವಿಕಲ್ಪ( quote ){
    ಪಕ್ಷ '\'':  ಭಙ್ಗ;
    ಪಕ್ಷ '"':   ಭಙ್ಗ;
    ಪಕ್ಷ '`':   ಭಙ್ಗ;                /* For MySQL compatibility */
    ಪಕ್ಷ '[':   quote = ']';  ಭಙ್ಗ;  /* For MS SqlServer compatibility */
    ಸ್ವಭಾವ:    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿ(i=1, j=0; z[i]; i++){
    ಯದಿ( z[i]==quote ){
      ಯದಿ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }ಅನ್ಯಥಾ{
        z[j++] = 0;
        ಭಙ್ಗ;
      }
    }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getColumnNames(
  sqlite3 *db, 
  ಧ್ರುವ ವರ್ಣ *zTab,
  ವರ್ಣ ***paCol, 
  ಪೂರ್ಣಾಙ್ಕ *pnCol
){
  ವರ್ಣ **aCol = 0;
  ವರ್ಣ *zSql;
  sqlite3_stmt *pStmt = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ಯದಿ( !zSql ){
    rc = SQLITE_NOMEM;
    ಗಚ್ಛ out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ಯದಿ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಙ್ಕ ii;
    ಪೂರ್ಣಾಙ್ಕ nBytes;
    ವರ್ಣ *zSpace;
    nCol = sqlite3_column_ಎಣಿಕೆ(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = ಪರಿಮಾಣ(ವರ್ಣ *) * nCol;
    ಪ್ರತಿ(ii=0; ii<nCol; ii++){
      ಧ್ರುವ ವರ್ಣ *zName = sqlite3_column_name(pStmt, ii);
      ಯದಿ( !zName ){
        rc = SQLITE_NOMEM;
        ಗಚ್ಛ out;
      }
      nBytes += (ಪೂರ್ಣಾಙ್ಕ)strlen(zName)+1;
    }
    aCol = (ವರ್ಣ **)sqlite3MallocZero(nBytes);
    ಯದಿ( !aCol ){
      rc = SQLITE_NOMEM;
      ಗಚ್ಛ out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (ವರ್ಣ *)(&aCol[nCol]);
    ಪ್ರತಿ(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (ಪೂರ್ಣಾಙ್ಕ)strlen(zSpace) + 1;
    }
    ಪ್ರತಿಜ್ಞಾ( (zSpace-nBytes)==(ವರ್ಣ *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getIndexArray(
  sqlite3 *db,             /* Database connection */
  ಧ್ರುವ ವರ್ಣ *zTab,        /* Name of table in database db */
  ಪೂರ್ಣಾಙ್ಕ nCol,
  ಪೂರ್ಣಾಙ್ಕ **paIndex
){
  sqlite3_stmt *pStmt = 0;
  ಪೂರ್ಣಾಙ್ಕ *aIndex = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ವರ್ಣ *zSql;

  /* Allocate space for the index array */
  aIndex = (ಪೂರ್ಣಾಙ್ಕ *)sqlite3MallocZero(ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) * nCol);
  ಯದಿ( !aIndex ){
    rc = SQLITE_NOMEM;
    ಗಚ್ಛ get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ಯದಿ( !zSql ){
    rc = SQLITE_NOMEM;
    ಗಚ್ಛ get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  ಯಾವತ್( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    ಧ್ರುವ ವರ್ಣ *zIdx = (ಧ್ರುವ ವರ್ಣ *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ಯದಿ( zIdx==0 ) ಅನುವರ್ತ;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ಯದಿ( !zSql ){
      rc = SQLITE_NOMEM;
      ಗಚ್ಛ get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ಯದಿ( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      ಪೂರ್ಣಾಙ್ಕ cid = sqlite3_column_int(pStmt2, 1);
      ಪ್ರತಿಜ್ಞಾ( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    ಯದಿ( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ಯದಿ( rc!=SQLITE_OK ){
      ಗಚ್ಛ get_index_array_out;
    }
  }


get_index_array_out:
  ಯದಿ( pStmt ){
    ಪೂರ್ಣಾಙ್ಕ rc2 = sqlite3_finalize(pStmt);
    ಯದಿ( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ಯದಿ( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ಸ್ಥಿರ ಶೂನ್ಯ appendToEchoModule(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zArg){
  ಪೂರ್ಣಾಙ್ಕ flags = (TCL_APPEND_ಮೌಲ್ಯ | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  ಯದಿ( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      ಯದಿ( sqlite3_step(pStmt)==SQLITE_ROW ){
        ಪೂರ್ಣಾಙ್ಕ rc2;
        ಧ್ರುವ ವರ್ಣ *zCreateTable = (ಧ್ರುವ ವರ್ಣ *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        ಯದಿ( rc==SQLITE_OK ){
          rc = rc2;
        }
      } ಅನ್ಯಥಾ {
        rc = sqlite3_finalize(pStmt);
        ಯದಿ( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ಯದಿ( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      ಯದಿ( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ EchoModule EchoModule;
ಸಂರಚನಾ EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoConstructor(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( ಪರಿಮಾಣ(*pVtab) );
  ಯದಿ( !pVtab ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ಯದಿ( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ಯದಿ( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    ಯದಿ( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      ವರ್ಣ *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    ಯದಿ( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  ಪ್ರತಿ(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ಯದಿ( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ಪ್ರತ್ಯಾಗಮ rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoCreate(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
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
  ಯದಿ( rc==SQLITE_OK && argc==5 ){
    ವರ್ಣ *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ಯದಿ( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ಯದಿ( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ಯದಿ( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  ಪ್ರತ್ಯಾಗಮ echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  ಪ್ರತ್ಯಾಗಮ echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoDestroy(sqlite3_vtab *pVtab){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ಯದಿ( p && p->zLogName ){
    ವರ್ಣ *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ಯದಿ( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ಯದಿ( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(ಪರಿಮಾಣ(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  ಪ್ರತ್ಯಾಗಮ (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoClose(sqlite3_vtab_cursor *cur){
  ಪೂರ್ಣಾಙ್ಕ rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoEof(sqlite3_vtab_cursor *cur){
  ಪ್ರತ್ಯಾಗಮ (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoNext(sqlite3_vtab_cursor *cur){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ಯದಿ( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  ಯದಿ( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    ಯದಿ( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }ಅನ್ಯಥಾ{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  ಪೂರ್ಣಾಙ್ಕ iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ಯದಿ( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  ಯದಿ( !pStmt ){
    sqlite3_result_null(ctx);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( sqlite3_data_ಎಣಿಕೆ(pStmt)>iCol );
    sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pStmt, iCol));
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ಯದಿ( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hashString(ಧ್ರುವ ವರ್ಣ *zString){
  u32 val = 0;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪ್ರತಿ(ii=0; zString[ii]; ii++){
    val = (val << 3) + (ಪೂರ್ಣಾಙ್ಕ)zString[ii];
  }
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ಯದಿ( simulateVtabError(pVtab, "xFilter") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  ಪ್ರತಿಜ್ಞಾ( idx_ಅಂಕೆ==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  ಪ್ರತಿ(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  ಪ್ರತಿಜ್ಞಾ( pCur->pStmt || rc!=SQLITE_OK );
  ಪ್ರತಿ(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_ಮೌಲ್ಯ(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ಯದಿ( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ string_concat(ವರ್ಣ **pzStr, ವರ್ಣ *zAppend, ಪೂರ್ಣಾಙ್ಕ doFree, ಪೂರ್ಣಾಙ್ಕ *pRc){
  ವರ್ಣ *zIn = *pzStr;
  ಯದಿ( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ಯದಿ( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }ಅನ್ಯಥಾ{
    ಯದಿ( zIn ){
      ವರ್ಣ *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }ಅನ್ಯಥಾ{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    ಯದಿ( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  ಯದಿ( doFree ){
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
ಸ್ಥಿರ ವರ್ಣ *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  ವರ್ಣ *zRet = 0;
  ಯದಿ( sqlite3_libversion_ಸಂಖ್ಯೆ()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<pTab->nCol; i++){
      ಯದಿ( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }ಅನ್ಯಥಾ{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ಯದಿ( !zRet ) ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ zRet;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪೂರ್ಣಾಙ್ಕ ii;
  ವರ್ಣ *zQuery = 0;
  ವರ್ಣ *zCol = 0;
  ವರ್ಣ *zNew;
  ಪೂರ್ಣಾಙ್ಕ nArg = 0;
  ಧ್ರುವ ವರ್ಣ *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  ಪೂರ್ಣಾಙ್ಕ nRow = 0;
  ಪೂರ್ಣಾಙ್ಕ useIdx = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ useCost = 0;
  ದ್ವಿಗುಣ cost = 0;
  ಪೂರ್ಣಾಙ್ಕ isIgnoreUsable = 0;
  ಯದಿ( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ಯದಿ( simulateVtabError(pVtab, "xBestIndex") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ಯದಿ( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } ಅನ್ಯಥಾ {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    ಯದಿ( !zQuery ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ಯದಿ( rc!=SQLITE_OK ){
      ಪ್ರತ್ಯಾಗಮ rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ಯದಿ( rc!=SQLITE_OK ){
      ಪ್ರತ್ಯಾಗಮ rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  ಯದಿ( !zCol ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  ಯದಿ( !zQuery ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;

  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಧ್ರುವ ಸಂರಚನಾ sqlite3_index_constraint *pConstraint;
    ಸಂರಚನಾ sqlite3_index_constraint_usage *pUsage;
    ಪೂರ್ಣಾಙ್ಕ iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ಯದಿ( !isIgnoreUsable && !pConstraint->usable ) ಅನುವರ್ತ;

    iCol = pConstraint->iColumn;
    ಯದಿ( iCol<0 || pVtab->aIndex[iCol] ){
      ವರ್ಣ *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      ವರ್ಣ *zOp = 0;
      useIdx = 1;
      ವಿಕಲ್ಪ( pConstraint->op ){
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; ಭಙ್ಗ;
        ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; ಭಙ್ಗ;
      }
      ಯದಿ( zOp ){
        ಯದಿ( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } ಅನ್ಯಥಾ {
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
  ಯದಿ( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    ಪೂರ್ಣಾಙ್ಕ iCol = pIdxInfo->aOrderBy->iColumn;
    ವರ್ಣ *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    ವರ್ಣ *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  ಯದಿ( !zQuery ){
    ಪ್ರತ್ಯಾಗಮ rc;
  }
  pIdxInfo->idx_ಅಂಕೆ = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  ಯದಿ( useCost ){
    pIdxInfo->estimatedCost = cost;
  }ಅನ್ಯಥಾ ಯದಿ( useIdx ){
    /* Approximation of log2(nRow). */
    ಪ್ರತಿ( ii=0; ii<(ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)*8)-1; ii++ ){
      ಯದಿ( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)ii;
      }
    }
  }ಅನ್ಯಥಾ{
    pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)nRow;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಪೂರ್ಣಾಙ್ಕ echoUpdate(
  sqlite3_vtab *tab, 
  ಪೂರ್ಣಾಙ್ಕ nData, 
  sqlite3_ಮೌಲ್ಯ **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  ವರ್ಣ *z = 0;               /* SQL statement to execute */
  ಪೂರ್ಣಾಙ್ಕ bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  ಪೂರ್ಣಾಙ್ಕ bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  ಪೂರ್ಣಾಙ್ಕ i;                     /* Counter variable used by for loops */

  ಪ್ರತಿಜ್ಞಾ( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  ಪ್ರತಿಜ್ಞಾ( pVtab->inTransaction );

  ಯದಿ( simulateVtabError(pVtab, "xUpdate") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ಯದಿ( nData>1 && sqlite3_ಮೌಲ್ಯ_type(apData[0])==SQLITE_INTEGER ){
    ವರ್ಣ *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    ಯದಿ( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_ಮೌಲ್ಯ_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ಯದಿ( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    ಪ್ರತಿ(i=2; i<nData; i++){
      ಯದಿ( apData[i]==0 ) ಅನುವರ್ತ;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  ಅನ್ಯಥಾ ಯದಿ( nData==1 && sqlite3_ಮೌಲ್ಯ_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    ಯದಿ( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  ಅನ್ಯಥಾ ಯದಿ( nData>2 && sqlite3_ಮೌಲ್ಯ_type(apData[0])==SQLITE_NULL ){
    ಪೂರ್ಣಾಙ್ಕ ii;
    ವರ್ಣ *z_ಸೇರಿಸು = 0;
    ವರ್ಣ *z_ಮೌಲ್ಯಗಳು = 0;
  
    z_ಸೇರಿಸು = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    ಯದಿ( !z_ಸೇರಿಸು ){
      rc = SQLITE_NOMEM;
    }
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_ಮೌಲ್ಯಗಳು = sqlite3_mprintf("?");
      string_concat(&z_ಸೇರಿಸು, "rowid", 0, &rc);
    }

    ಪ್ರತಿಜ್ಞಾ((pVtab->nCol+2)==nData);
    ಪ್ರತಿ(ii=2; ii<nData; ii++){
      string_concat(&z_ಸೇರಿಸು, 
          sqlite3_mprintf("%s%Q", z_ಮೌಲ್ಯಗಳು?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_ಮೌಲ್ಯಗಳು, 
          sqlite3_mprintf("%s?%d", z_ಮೌಲ್ಯಗಳು?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_ಸೇರಿಸು, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_ಮೌಲ್ಯಗಳು, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ(0);
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ಯದಿ( rc==SQLITE_OK ) {
    ಯದಿ( bindArgZero ){
      sqlite3_bind_ಮೌಲ್ಯ(pStmt, nData, apData[0]);
    }
    ಯದಿ( bindArgOne ){
      sqlite3_bind_ಮೌಲ್ಯ(pStmt, 1, apData[1]);
    }
    ಪ್ರತಿ(i=2; i<nData && rc==SQLITE_OK; i++){
      ಯದಿ( apData[i] ) rc = sqlite3_bind_ಮೌಲ್ಯ(pStmt, i, apData[i]);
    }
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }ಅನ್ಯಥಾ{
      sqlite3_finalize(pStmt);
    }
  }

  ಯದಿ( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_ಸೇರಿಸು_rowid(db);
  }
  ಯದಿ( rc!=SQLITE_OK ){
    tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoTransactionCall(sqlite3_vtab *tab, ಧ್ರುವ ವರ್ಣ *zCall){
  ವರ್ಣ *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoBegin(sqlite3_vtab *tab){
  ಪೂರ್ಣಾಙ್ಕ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ಧ್ರುವ ವರ್ಣ *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  ಪ್ರತಿಜ್ಞಾ( !pVtab->inTransaction );

  ಯದಿ( simulateVtabError(pVtab, "xBegin") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ಯದಿ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ಯದಿ( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  ಯದಿ( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoSync(sqlite3_vtab *tab){
  ಪೂರ್ಣಾಙ್ಕ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ಧ್ರುವ ವರ್ಣ *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  ಪ್ರತಿಜ್ಞಾ( pVtab->inTransaction );

  ಯದಿ( simulateVtabError(pVtab, "xSync") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ಯದಿ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ಯದಿ( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  ಪೂರ್ಣಾಙ್ಕ rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  ಪ್ರತಿಜ್ಞಾ( pVtab->inTransaction );

  ಯದಿ( simulateVtabError(pVtab, "xCommit") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoRollback(sqlite3_vtab *tab){
  ಪೂರ್ಣಾಙ್ಕ rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  ಪ್ರತಿಜ್ಞಾ( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ಸ್ಥಿರ ಶೂನ್ಯ overloadedGlobFunction(
  sqlite3_context *pContext,
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  ಪ್ರತಿ(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_ಮೌಲ್ಯ(&str));
  Tcl_DStringFree(&str);
  ಯದಿ( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoFindFunction(
  sqlite3_vtab *vtab,
  ಪೂರ್ಣಾಙ್ಕ nArg,
  ಧ್ರುವ ವರ್ಣ *zFuncName,
  ಶೂನ್ಯ (**pxFunc)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ಯದಿ( strcmp(zFuncName,"glob")!=0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoRename(sqlite3_vtab *vtab, ಧ್ರುವ ವರ್ಣ *zNewName){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ಯದಿ( simulateVtabError(p, "xRename") ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  ಯದಿ( p->isPattern ){
    ಪೂರ್ಣಾಙ್ಕ nThis = (ಪೂರ್ಣಾಙ್ಕ)strlen(p->zThis);
    ವರ್ಣ *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoSavepoint(sqlite3_vtab *pVTab, ಪೂರ್ಣಾಙ್ಕ iSavepoint){
  ಪ್ರತಿಜ್ಞಾ( pVTab );
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoRelease(sqlite3_vtab *pVTab, ಪೂರ್ಣಾಙ್ಕ iSavepoint){
  ಪ್ರತಿಜ್ಞಾ( pVTab );
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ echoRollbackTo(sqlite3_vtab *pVTab, ಪೂರ್ಣಾಙ್ಕ iSavepoint){
  ಪ್ರತಿಜ್ಞಾ( pVTab );
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ಸ್ಥಿರ sqlite3_module echoModule = {
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

ಸ್ಥಿರ sqlite3_module echoModuleV2 = {
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

ಸ್ಥಿರ ಶೂನ್ಯ moduleDestroy(ಶೂನ್ಯ *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3 *db;
  EchoModule *pMod;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(ಪರಿಮಾಣ(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (ಶೂನ್ಯ*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ಯದಿ( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(ಪರಿಮಾಣ(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (ಶೂನ್ಯ*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB SQL");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3_errmsg(db), TCL_VOLATILE);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
