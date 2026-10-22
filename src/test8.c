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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ echo_vtab echo_vtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ echo_cursor echo_cursor;

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
ਬਣਤਰ echo_vtab {
  sqlite3_vtab ਅਧਾਰ;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  ਪੂਰਨ is_ਨਮੂਨਾ;
  ਪੂਰਨ ਵਿੱਚ_Transaction;      /* True if within a transaction */
  ਅੱਖਰ *zThis;            /* Name of the echo table */
  ਅੱਖਰ *z_ਸਾਰਣੀ_Name;       /* Name of the real table */
  ਅੱਖਰ *zLogName;         /* Name of the log table */
  ਪੂਰਨ n_ਖਾਨਾ;               /* Number of columns in the real table */
  ਪੂਰਨ *a_ਸੂਚਕਾਂਕ;            /* Array of size nCol. True if column has an index */
  ਅੱਖਰ **a_ਖਾਨਾ;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ਬਣਤਰ echo_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;
  sqlite3_stmt *pStmt;
};

ਸਥਿਰ ਪੂਰਨ simulateVtabError(echo_vtab *p, ਅਚਲ ਅੱਖਰ *z_ਵਿਧੀ){
  ਅਚਲ ਅੱਖਰ *zErr;
  ਅੱਖਰ zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", z_ਵਿਧੀ, p->z_ਸਾਰਣੀ_Name);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ਜੇ( zErr ){
    p->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  ਵਾਪਸ (zErr!=0);
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
ਸਥਿਰ ਖਾਲੀ dequoteString(ਅੱਖਰ *z){
  ਪੂਰਨ quote;
  ਪੂਰਨ i, j;
  ਜੇ( z==0 ) ਵਾਪਸ;
  quote = z[0];
  ਚੋਣ( quote ){
    ਹਾਲਤ '\'':  ਰੋਕੋ;
    ਹਾਲਤ '"':   ਰੋਕੋ;
    ਹਾਲਤ '`':   ਰੋਕੋ;                /* For MySQL compatibility */
    ਹਾਲਤ '[':   quote = ']';  ਰੋਕੋ;  /* For MS SqlServer compatibility */
    ਮੂਲ:    ਵਾਪਸ;
  }
  ਲਈ(i=1, j=0; z[i]; i++){
    ਜੇ( z[i]==quote ){
      ਜੇ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }ਨਹੀਂਤਾਂ{
        z[j++] = 0;
        ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ{
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
ਸਥਿਰ ਪੂਰਨ get_ਕਾਲਮ_ਨਾਂ(
  sqlite3 *db, 
  ਅਚਲ ਅੱਖਰ *zTab,
  ਅੱਖਰ ***pa_ਖਾਨਾ, 
  ਪੂਰਨ *pn_ਖਾਨਾ
){
  ਅੱਖਰ **a_ਖਾਨਾ = 0;
  ਅੱਖਰ *zSql;
  sqlite3_stmt *pStmt = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ n_ਖਾਨਾ = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ਜੇ( !zSql ){
    rc = SQLITE_NOMEM;
    ਜਾਓ ਬਾਹਰਲਾ;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ ii;
    ਪੂਰਨ n_ਬਾਈਟਾਂ;
    ਅੱਖਰ *zSpace;
    n_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਗਿਣਤੀ(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    n_ਬਾਈਟਾਂ = ਆਕਾਰ(ਅੱਖਰ *) * n_ਖਾਨਾ;
    ਲਈ(ii=0; ii<n_ਖਾਨਾ; ii++){
      ਅਚਲ ਅੱਖਰ *zName = sqlite3_ਕਾਲਮ_name(pStmt, ii);
      ਜੇ( !zName ){
        rc = SQLITE_NOMEM;
        ਜਾਓ ਬਾਹਰਲਾ;
      }
      n_ਬਾਈਟਾਂ += (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zName)+1;
    }
    a_ਖਾਨਾ = (ਅੱਖਰ **)sqlite3MallocZero(n_ਬਾਈਟਾਂ);
    ਜੇ( !a_ਖਾਨਾ ){
      rc = SQLITE_NOMEM;
      ਜਾਓ ਬਾਹਰਲਾ;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (ਅੱਖਰ *)(&a_ਖਾਨਾ[n_ਖਾਨਾ]);
    ਲਈ(ii=0; ii<n_ਖਾਨਾ; ii++){
      a_ਖਾਨਾ[ii] = zSpace;
      sqlite3_snprintf(n_ਬਾਈਟਾਂ, zSpace, "%s", sqlite3_ਕਾਲਮ_name(pStmt,ii));
      zSpace += (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zSpace) + 1;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( (zSpace-n_ਬਾਈਟਾਂ)==(ਅੱਖਰ *)a_ਖਾਨਾ );
  }

  *pa_ਖਾਨਾ = a_ਖਾਨਾ;
  *pn_ਖਾਨਾ = n_ਖਾਨਾ;

ਬਾਹਰਲਾ:
  sqlite3_finalize(pStmt);
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ get_ਸੂਚਕਾਂਕ_Array(
  sqlite3 *db,             /* Database connection */
  ਅਚਲ ਅੱਖਰ *zTab,        /* Name of table in database db */
  ਪੂਰਨ n_ਖਾਨਾ,
  ਪੂਰਨ **pa_ਸੂਚਕਾਂਕ
){
  sqlite3_stmt *pStmt = 0;
  ਪੂਰਨ *a_ਸੂਚਕਾਂਕ = 0;
  ਪੂਰਨ rc;
  ਅੱਖਰ *zSql;

  /* Allocate space for the index array */
  a_ਸੂਚਕਾਂਕ = (ਪੂਰਨ *)sqlite3MallocZero(ਆਕਾਰ(ਪੂਰਨ) * n_ਖਾਨਾ);
  ਜੇ( !a_ਸੂਚਕਾਂਕ ){
    rc = SQLITE_NOMEM;
    ਜਾਓ get_ਸੂਚਕਾਂਕ_array_ਬਾਹਰਲਾ;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ਜੇ( !zSql ){
    rc = SQLITE_NOMEM;
    ਜਾਓ get_ਸੂਚਕਾਂਕ_array_ਬਾਹਰਲਾ;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  ਜਦੋਂ( pStmt && sqlite3_ਕਦਮ(pStmt)==SQLITE_ਪੰਕਤੀ ){
    ਅਚਲ ਅੱਖਰ *zIdx = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ਜੇ( zIdx==0 ) ਜਾਰੀ;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ਜੇ( !zSql ){
      rc = SQLITE_NOMEM;
      ਜਾਓ get_ਸੂਚਕਾਂਕ_array_ਬਾਹਰਲਾ;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ਜੇ( pStmt2 && sqlite3_ਕਦਮ(pStmt2)==SQLITE_ਪੰਕਤੀ ){
      ਪੂਰਨ cid = sqlite3_ਕਾਲਮ_int(pStmt2, 1);
      ਪ੍ਰਤਿਜ੍ਞਾ( cid>=0 && cid<n_ਖਾਨਾ );
      a_ਸੂਚਕਾਂਕ[cid] = 1;
    }
    ਜੇ( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ get_ਸੂਚਕਾਂਕ_array_ਬਾਹਰਲਾ;
    }
  }


get_ਸੂਚਕਾਂਕ_array_ਬਾਹਰਲਾ:
  ਜੇ( pStmt ){
    ਪੂਰਨ rc2 = sqlite3_finalize(pStmt);
    ਜੇ( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_free(a_ਸੂਚਕਾਂਕ);
    a_ਸੂਚਕਾਂਕ = 0;
  }
  *pa_ਸੂਚਕਾਂਕ = a_ਸੂਚਕਾਂਕ;
  ਵਾਪਸ rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ਸਥਿਰ ਖਾਲੀ append_ਮੰਜ਼ਿਲ_Echo_Module(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zArg){
  ਪੂਰਨ flags = (TCL_APPEND_ਮੁੱਲ | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_ਸੈੱਟ_Var(interp, "echo_module", (zArg?zArg:""), flags);
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
ਸਥਿਰ ਪੂਰਨ echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( pVtab->z_ਸਾਰਣੀ_Name ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_bind_ਪਾਠ(pStmt, 1, pVtab->z_ਸਾਰਣੀ_Name, -1, 0);
      ਜੇ( sqlite3_ਕਦਮ(pStmt)==SQLITE_ਪੰਕਤੀ ){
        ਪੂਰਨ rc2;
        ਅਚਲ ਅੱਖਰ *z_Create_ਸਾਰਣੀ = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 0);
        rc = sqlite3_declare_vtab(db, z_Create_ਸਾਰਣੀ);
        rc2 = sqlite3_finalize(pStmt);
        ਜੇ( rc==SQLITE_OK ){
          rc = rc2;
        }
      } ਨਹੀਂਤਾਂ {
        rc = sqlite3_finalize(pStmt);
        ਜੇ( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ਜੇ( rc==SQLITE_OK ){
        rc = get_ਕਾਲਮ_ਨਾਂ(db, pVtab->z_ਸਾਰਣੀ_Name, &pVtab->a_ਖਾਨਾ, &pVtab->n_ਖਾਨਾ);
      }
      ਜੇ( rc==SQLITE_OK ){
        rc = get_ਸੂਚਕਾਂਕ_Array(db, pVtab->z_ਸਾਰਣੀ_Name, pVtab->n_ਖਾਨਾ, &pVtab->a_ਸੂਚਕਾਂਕ);
      }
    }
  }

  ਵਾਪਸ rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ਸਥਿਰ ਪੂਰਨ echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_ਸੂਚਕਾਂਕ);
  sqlite3_free(p->a_ਖਾਨਾ);
  sqlite3_free(p->zThis);
  sqlite3_free(p->z_ਸਾਰਣੀ_Name);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  ਵਾਪਸ 0;
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ EchoModule EchoModule;
ਬਣਤਰ EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ਸਥਿਰ ਪੂਰਨ echoConstructor(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  ਪੂਰਨ rc;
  ਪੂਰਨ i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( ਆਕਾਰ(*pVtab) );
  ਜੇ( !pVtab ){
    ਵਾਪਸ SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ਜੇ( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ਵਾਪਸ SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ਜੇ( argc>3 ){
    pVtab->z_ਸਾਰਣੀ_Name = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->z_ਸਾਰਣੀ_Name);
    ਜੇ( pVtab->z_ਸਾਰਣੀ_Name && pVtab->z_ਸਾਰਣੀ_Name[0]=='*' ){
      ਅੱਖਰ *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->z_ਸਾਰਣੀ_Name[1]));
      sqlite3_free(pVtab->z_ਸਾਰਣੀ_Name);
      pVtab->z_ਸਾਰਣੀ_Name = z;
      pVtab->is_ਨਮੂਨਾ = 1;
    }
    ਜੇ( !pVtab->z_ਸਾਰਣੀ_Name ){
      echoDestructor((sqlite3_vtab *)pVtab);
      ਵਾਪਸ SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  ਲਈ(i=0; i<argc; i++){
    append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ਜੇ( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ਵਾਪਸ rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ਸਥਿਰ ਪੂਰਨ echoCreate(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  ਪੂਰਨ rc = SQLITE_OK;
  append_ਮੰਜ਼ਿਲ_Echo_Module(((EchoModule *)pAux)->interp, "xCreate");
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
  ਜੇ( rc==SQLITE_OK && argc==5 ){
    ਅੱਖਰ *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ਜੇ( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ਜੇ( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ਜੇ( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->ਵਿੱਚ_Transaction = 1;
  }

  ਵਾਪਸ rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ਸਥਿਰ ਪੂਰਨ echoConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  append_ਮੰਜ਼ਿਲ_Echo_Module(((EchoModule *)pAux)->interp, "xConnect");
  ਵਾਪਸ echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ਸਥਿਰ ਪੂਰਨ echoDisconnect(sqlite3_vtab *pVtab){
  append_ਮੰਜ਼ਿਲ_Echo_Module(((echo_vtab *)pVtab)->interp, "xDisconnect");
  ਵਾਪਸ echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ਸਥਿਰ ਪੂਰਨ echoDestroy(sqlite3_vtab *pVtab){
  ਪੂਰਨ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  append_ਮੰਜ਼ਿਲ_Echo_Module(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ਜੇ( p && p->zLogName ){
    ਅੱਖਰ *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ਜੇ( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  ਵਾਪਸ rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ਸਥਿਰ ਪੂਰਨ echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ਜੇ( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    ਵਾਪਸ SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(ਆਕਾਰ(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  ਵਾਪਸ (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ਸਥਿਰ ਪੂਰਨ echoClose(sqlite3_vtab_cursor *cur){
  ਪੂਰਨ rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  ਵਾਪਸ rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ਸਥਿਰ ਪੂਰਨ echoEof(sqlite3_vtab_cursor *cur){
  ਵਾਪਸ (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ਸਥਿਰ ਪੂਰਨ echo_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  ਪੂਰਨ rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ਜੇ( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( pCur->pStmt ){
    rc = sqlite3_ਕਦਮ(pCur->pStmt);
    ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  ਵਾਪਸ rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ਸਥਿਰ ਪੂਰਨ echo_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  ਪੂਰਨ i_ਖਾਨਾ = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ਜੇ( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( !pStmt ){
    sqlite3_ਨਤੀਜਾ_null(ctx);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਅੰਕੜੇ_ਗਿਣਤੀ(pStmt)>i_ਖਾਨਾ );
    sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pStmt, i_ਖਾਨਾ));
  }
  ਵਾਪਸ SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ਸਥਿਰ ਪੂਰਨ echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ਜੇ( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  *pRowid = sqlite3_ਕਾਲਮ_int64(pStmt, 0);
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ hashString(ਅਚਲ ਅੱਖਰ *zString){
  u32 val = 0;
  ਪੂਰਨ ii;
  ਲਈ(ii=0; zString[ii]; ii++){
    val = (val << 3) + (ਪੂਰਨ)zString[ii];
  }
  ਵਾਪਸ (ਪੂਰਨ)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ਸਥਿਰ ਪੂਰਨ echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ rc;
  ਪੂਰਨ i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ਜੇ( simulateVtabError(pVtab, "xFilter") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  ਪ੍ਰਤਿਜ੍ਞਾ( idx_ਅੰਕ==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, "xFilter");
  append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, idxStr);
  ਲਈ(i=0; i<argc; i++){
    append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pStmt || rc!=SQLITE_OK );
  ਲਈ(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_ਮੁੱਲ(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ਜੇ( rc==SQLITE_OK ){
    rc = echo_ਅਗਲਾ(pVtabCursor);
  }

  ਵਾਪਸ rc;
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
ਸਥਿਰ ਖਾਲੀ string_concat(ਅੱਖਰ **pzStr, ਅੱਖਰ *zAppend, ਪੂਰਨ doFree, ਪੂਰਨ *pRc){
  ਅੱਖਰ *z_ਵਿੱਚ = *pzStr;
  ਜੇ( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ਜੇ( *pRc!=SQLITE_OK ){
    sqlite3_free(z_ਵਿੱਚ);
    z_ਵਿੱਚ = 0;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( z_ਵਿੱਚ ){
      ਅੱਖਰ *z_ਆਰਜ਼ੀ = z_ਵਿੱਚ;
      z_ਵਿੱਚ = sqlite3_mprintf("%s%s", z_ਵਿੱਚ, zAppend);
      sqlite3_free(z_ਆਰਜ਼ੀ);
    }ਨਹੀਂਤਾਂ{
      z_ਵਿੱਚ = sqlite3_mprintf("%s", zAppend);
    }
    ਜੇ( !z_ਵਿੱਚ ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = z_ਵਿੱਚ;
  ਜੇ( doFree ){
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
ਸਥਿਰ ਅੱਖਰ *echoSelectList(echo_vtab *pTab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਅੱਖਰ *zRet = 0;
  ਜੇ( sqlite3_libversion_ਸੰਖਿਆ()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      ਜੇ( pIdxInfo->ਖਾਨਾ_ਵਰਤਿਆ & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->a_ਖਾਨਾ[i]);
      }ਨਹੀਂਤਾਂ{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ਜੇ( !zRet ) ਰੋਕੋ;
    }
  }
  ਵਾਪਸ zRet;
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
ਸਥਿਰ ਪੂਰਨ echo_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਪੂਰਨ ii;
  ਅੱਖਰ *zQuery = 0;
  ਅੱਖਰ *z_ਖਾਨਾ = 0;
  ਅੱਖਰ *zNew;
  ਪੂਰਨ nArg = 0;
  ਅਚਲ ਅੱਖਰ *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  ਪੂਰਨ n_ਪੰਕਤੀ = 0;
  ਪੂਰਨ useIdx = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ use_ਲਾਗਤ = 0;
  ਦੁੱਗਣਾ ਲਾਗਤ = 0;
  ਪੂਰਨ isIgnoreUsable = 0;
  ਜੇ( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ਜੇ( simulateVtabError(pVtab, "xBestIndex") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ਜੇ( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    ਲਾਗਤ = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    use_ਲਾਗਤ = 1;
  } ਨਹੀਂਤਾਂ {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->z_ਸਾਰਣੀ_Name);
    ਜੇ( !zQuery ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    sqlite3_ਕਦਮ(pStmt);
    n_ਪੰਕਤੀ = sqlite3_ਕਾਲਮ_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
  }

  z_ਖਾਨਾ = echoSelectList(pVtab, pIdxInfo);
  ਜੇ( !z_ਖਾਨਾ ) ਵਾਪਸ SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", z_ਖਾਨਾ, pVtab->z_ਸਾਰਣੀ_Name);
  ਜੇ( !zQuery ) ਵਾਪਸ SQLITE_NOMEM;

  ਲਈ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ਅਚਲ ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *pConstraint;
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage;
    ਪੂਰਨ i_ਖਾਨਾ;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ਜੇ( !isIgnoreUsable && !pConstraint->usable ) ਜਾਰੀ;

    i_ਖਾਨਾ = pConstraint->i_ਕਾਲਮ;
    ਜੇ( i_ਖਾਨਾ<0 || pVtab->a_ਸੂਚਕਾਂਕ[i_ਖਾਨਾ] ){
      ਅੱਖਰ *z_New_ਖਾਨਾ = i_ਖਾਨਾ>=0 ? pVtab->a_ਖਾਨਾ[i_ਖਾਨਾ] : "rowid";
      ਅੱਖਰ *zOp = 0;
      useIdx = 1;
      ਚੋਣ( pConstraint->op ){
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ:
          zOp = "="; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LT:
          zOp = "<"; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GT:
          zOp = ">"; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LE:
          zOp = "<="; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GE:
          zOp = ">="; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਮੇਲ:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE:
          zOp = "like"; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GLOB:
          zOp = "glob"; ਰੋਕੋ;
        ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_REGEXP:
          zOp = "regexp"; ਰੋਕੋ;
      }
      ਜੇ( zOp ){
        ਜੇ( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, z_New_ਖਾਨਾ);
        } ਨਹੀਂਤਾਂ {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, z_New_ਖਾਨਾ, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_ਸੂਚਕਾਂਕ = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  ਜੇ( pIdxInfo->n_ਕ੍ਰਮ_By==1 && (
        pIdxInfo->a_ਕ੍ਰਮ_By->i_ਕਾਲਮ<0 ||
        pVtab->a_ਸੂਚਕਾਂਕ[pIdxInfo->a_ਕ੍ਰਮ_By->i_ਕਾਲਮ]) ){
    ਪੂਰਨ i_ਖਾਨਾ = pIdxInfo->a_ਕ੍ਰਮ_By->i_ਕਾਲਮ;
    ਅੱਖਰ *z_New_ਖਾਨਾ = i_ਖਾਨਾ>=0 ? pVtab->a_ਖਾਨਾ[i_ਖਾਨਾ] : "rowid";
    ਅੱਖਰ *zDir = pIdxInfo->a_ਕ੍ਰਮ_By->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", z_New_ਖਾਨਾ, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->ਕ੍ਰਮ_By_Consumed = 1;
  }

  append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, "xBestIndex");;
  append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, zQuery);

  ਜੇ( !zQuery ){
    ਵਾਪਸ rc;
  }
  pIdxInfo->idx_ਅੰਕ = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str = 1;
  ਜੇ( use_ਲਾਗਤ ){
    pIdxInfo->estimated_ਲਾਗਤ = ਲਾਗਤ;
  }ਨਹੀਂਤਾਂ ਜੇ( useIdx ){
    /* Approximation of log2(nRow). */
    ਲਈ( ii=0; ii<(ਆਕਾਰ(ਪੂਰਨ)*8)-1; ii++ ){
      ਜੇ( n_ਪੰਕਤੀ & (1<<ii) ){
        pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)ii;
      }
    }
  }ਨਹੀਂਤਾਂ{
    pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)n_ਪੰਕਤੀ;
  }
  ਵਾਪਸ rc;
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
ਪੂਰਨ echoUpdate(
  sqlite3_vtab *tab, 
  ਪੂਰਨ n_ਅੰਕੜੇ, 
  sqlite3_ਮੁੱਲ **ap_ਅੰਕੜੇ, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  ਪੂਰਨ rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  ਅੱਖਰ *z = 0;               /* SQL statement to execute */
  ਪੂਰਨ bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  ਪੂਰਨ bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  ਪੂਰਨ i;                     /* Counter variable used by for loops */

  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਅੰਕੜੇ==pVtab->n_ਖਾਨਾ+2 || n_ਅੰਕੜੇ==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  ਪ੍ਰਤਿਜ੍ਞਾ( pVtab->ਵਿੱਚ_Transaction );

  ਜੇ( simulateVtabError(pVtab, "xUpdate") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ਜੇ( n_ਅੰਕੜੇ>1 && sqlite3_ਮੁੱਲ_type(ap_ਅੰਕੜੇ[0])==SQLITE_INTEGER ){
    ਅੱਖਰ *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->z_ਸਾਰਣੀ_Name);
    ਜੇ( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_ਅੰਕੜੇ[1] && sqlite3_ਮੁੱਲ_type(ap_ਅੰਕੜੇ[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ਜੇ( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    ਲਈ(i=2; i<n_ਅੰਕੜੇ; i++){
      ਜੇ( ap_ਅੰਕੜੇ[i]==0 ) ਜਾਰੀ;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->a_ਖਾਨਾ[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_ਅੰਕੜੇ), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  ਨਹੀਂਤਾਂ ਜੇ( n_ਅੰਕੜੇ==1 && sqlite3_ਮੁੱਲ_type(ap_ਅੰਕੜੇ[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->z_ਸਾਰਣੀ_Name);
    ਜੇ( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  ਨਹੀਂਤਾਂ ਜੇ( n_ਅੰਕੜੇ>2 && sqlite3_ਮੁੱਲ_type(ap_ਅੰਕੜੇ[0])==SQLITE_NULL ){
    ਪੂਰਨ ii;
    ਅੱਖਰ *z_ਪਾਓ = 0;
    ਅੱਖਰ *z_ਮੁੱਲਾਂ = 0;
  
    z_ਪਾਓ = sqlite3_mprintf("INSERT INTO %Q (", pVtab->z_ਸਾਰਣੀ_Name);
    ਜੇ( !z_ਪਾਓ ){
      rc = SQLITE_NOMEM;
    }
    ਜੇ( sqlite3_ਮੁੱਲ_type(ap_ਅੰਕੜੇ[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_ਮੁੱਲਾਂ = sqlite3_mprintf("?");
      string_concat(&z_ਪਾਓ, "rowid", 0, &rc);
    }

    ਪ੍ਰਤਿਜ੍ਞਾ((pVtab->n_ਖਾਨਾ+2)==n_ਅੰਕੜੇ);
    ਲਈ(ii=2; ii<n_ਅੰਕੜੇ; ii++){
      string_concat(&z_ਪਾਓ, 
          sqlite3_mprintf("%s%Q", z_ਮੁੱਲਾਂ?", ":"", pVtab->a_ਖਾਨਾ[ii-2]), 1, &rc);
      string_concat(&z_ਮੁੱਲਾਂ, 
          sqlite3_mprintf("%s?%d", z_ਮੁੱਲਾਂ?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_ਪਾਓ, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_ਮੁੱਲਾਂ, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ(0);
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ਜੇ( rc==SQLITE_OK ) {
    ਜੇ( bindArgZero ){
      sqlite3_bind_ਮੁੱਲ(pStmt, n_ਅੰਕੜੇ, ap_ਅੰਕੜੇ[0]);
    }
    ਜੇ( bindArgOne ){
      sqlite3_bind_ਮੁੱਲ(pStmt, 1, ap_ਅੰਕੜੇ[1]);
    }
    ਲਈ(i=2; i<n_ਅੰਕੜੇ && rc==SQLITE_OK; i++){
      ਜੇ( ap_ਅੰਕੜੇ[i] ) rc = sqlite3_bind_ਮੁੱਲ(pStmt, i, ap_ਅੰਕੜੇ[i]);
    }
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_ਕਦਮ(pStmt);
      rc = sqlite3_finalize(pStmt);
    }ਨਹੀਂਤਾਂ{
      sqlite3_finalize(pStmt);
    }
  }

  ਜੇ( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_ਆਖਰੀ_ਪਾਓ_rowid(db);
  }
  ਜੇ( rc!=SQLITE_OK ){
    tab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  ਵਾਪਸ rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ਸਥਿਰ ਪੂਰਨ echoTransactionCall(sqlite3_vtab *tab, ਅਚਲ ਅੱਖਰ *zCall){
  ਅੱਖਰ *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->z_ਸਾਰਣੀ_Name);
  ਜੇ( z==0 ) ਵਾਪਸ SQLITE_NOMEM;
  append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, zCall);
  append_ਮੰਜ਼ਿਲ_Echo_Module(pVtab->interp, z);
  sqlite3_free(z);
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ echoBegin(sqlite3_vtab *tab){
  ਪੂਰਨ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ਅਚਲ ਅੱਖਰ *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  ਪ੍ਰਤਿਜ੍ਞਾ( !pVtab->ਵਿੱਚ_Transaction );

  ਜੇ( simulateVtabError(pVtab, "xBegin") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ਜੇ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ਜੇ( zVal && 0==strcmp(zVal, pVtab->z_ਸਾਰਣੀ_Name) ){
      rc = SQLITE_ERROR;
    }
  }
  ਜੇ( rc==SQLITE_OK ){
    pVtab->ਵਿੱਚ_Transaction = 1;
  }
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ echoSync(sqlite3_vtab *tab){
  ਪੂਰਨ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ਅਚਲ ਅੱਖਰ *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  ਪ੍ਰਤਿਜ੍ਞਾ( pVtab->ਵਿੱਚ_Transaction );

  ਜੇ( simulateVtabError(pVtab, "xSync") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ਜੇ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ਜੇ( zVal && 0==strcmp(zVal, pVtab->z_ਸਾਰਣੀ_Name) ){
      rc = -1;
    }
  }
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  ਪੂਰਨ rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  ਪ੍ਰਤਿਜ੍ਞਾ( pVtab->ਵਿੱਚ_Transaction );

  ਜੇ( simulateVtabError(pVtab, "xCommit") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3_ਸਿਰਾ_Benign_Malloc();
  pVtab->ਵਿੱਚ_Transaction = 0;
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ echoRollback(sqlite3_vtab *tab){
  ਪੂਰਨ rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  ਪ੍ਰਤਿਜ੍ਞਾ( pVtab->ਵਿੱਚ_Transaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->ਵਿੱਚ_Transaction = 0;
  ਵਾਪਸ rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ਸਥਿਰ ਖਾਲੀ overloadedGlobFunction(
  sqlite3_context *pContext,
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **apArg
){
  Tcl_Interp *interp = sqlite3_user_ਅੰਕੜੇ(pContext);
  Tcl_DString str;
  ਪੂਰਨ i;
  ਪੂਰਨ rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  ਲਈ(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_ਮੁੱਲ(&str));
  Tcl_DStringFree(&str);
  ਜੇ( rc ){
    sqlite3_ਨਤੀਜਾ_error(pContext, Tcl_Get_String_ਨਤੀਜਾ(interp), -1);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(pContext, Tcl_Get_String_ਨਤੀਜਾ(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_ਨਤੀਜਾ(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
ਸਥਿਰ ਪੂਰਨ echo_ਲੱਭੋ_Function(
  sqlite3_vtab *vtab,
  ਪੂਰਨ nArg,
  ਅਚਲ ਅੱਖਰ *zFuncName,
  ਖਾਲੀ (**pxFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
  ਖਾਲੀ **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ਜੇ( strcmp(zFuncName,"glob")!=0 ){
    ਵਾਪਸ 0;
  }
  ਜੇ( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    ਵਾਪਸ 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ echoRename(sqlite3_vtab *vtab, ਅਚਲ ਅੱਖਰ *zNewName){
  ਪੂਰਨ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ਜੇ( simulateVtabError(p, "xRename") ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( p->is_ਨਮੂਨਾ ){
    ਪੂਰਨ nThis = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(p->zThis);
    ਅੱਖਰ *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->z_ਸਾਰਣੀ_Name, zNewName, &p->z_ਸਾਰਣੀ_Name[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ echoSavepoint(sqlite3_vtab *pVTab, ਪੂਰਨ iSavepoint){
  ਪ੍ਰਤਿਜ੍ਞਾ( pVTab );
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ echoRelease(sqlite3_vtab *pVTab, ਪੂਰਨ iSavepoint){
  ਪ੍ਰਤਿਜ੍ਞਾ( pVTab );
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ echo_Rollback_ਮੰਜ਼ਿਲ(sqlite3_vtab *pVTab, ਪੂਰਨ iSavepoint){
  ਪ੍ਰਤਿਜ੍ਞਾ( pVTab );
  ਵਾਪਸ SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ਸਥਿਰ sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_ਅਗਲਾ,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_ਕਾਲਮ,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ਲੱਭੋ_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};

ਸਥਿਰ sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_ਅਗਲਾ,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_ਕਾਲਮ,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ਲੱਭੋ_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  echoSavepoint,
  echoRelease,
  echo_Rollback_ਮੰਜ਼ਿਲ,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

ਸਥਿਰ ਖਾਲੀ moduleDestroy(ਖਾਲੀ *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_echo_module(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਪੂਰਨ rc;
  sqlite3 *db;
  EchoModule *pMod;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(ਆਕਾਰ(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (ਖਾਲੀ*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ਜੇ( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(ਆਕਾਰ(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (ਖਾਲੀ*)pMod, moduleDestroy
    );
  }

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
  ਵਾਪਸ TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI declare_vtab(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ਪੂਰਨ rc;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB SQL");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3_errmsg(db), TCL_VOLATILE);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest8_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "register_echo_module",       ਰਜਿਸਟਰ_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}
