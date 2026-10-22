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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ echo_vtab echo_vtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ echo_cursor echo_cursor;

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
ರಚನೆ echo_vtab {
  sqlite3_vtab ತಳ;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  ಪೂರ್ಣಾಂಕ is_ನಮೂನೆ;
  ಪೂರ್ಣಾಂಕ ಒಳ_Transaction;      /* True if within a transaction */
  ಅಕ್ಷರ *zThis;            /* Name of the echo table */
  ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ_Name;       /* Name of the real table */
  ಅಕ್ಷರ *zLogName;         /* Name of the log table */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;               /* Number of columns in the real table */
  ಪೂರ್ಣಾಂಕ *a_ಸೂಚ್ಯಂಕ;            /* Array of size nCol. True if column has an index */
  ಅಕ್ಷರ **a_ಲಂಬಸಾಲು;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ರಚನೆ echo_cursor {
  sqlite3_vtab_cursor ತಳ;
  sqlite3_stmt *pStmt;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ simulateVtabError(echo_vtab *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ವಿಧಾನ){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr;
  ಅಕ್ಷರ zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", z_ವಿಧಾನ, p->z_ಕೋಷ್ಟಕ_Name);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  ಆದರೆ( zErr ){
    p->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  ಹಿಂತಿರುಗಿ (zErr!=0);
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
ಸ್ಥಿರ ಶೂನ್ಯ dequoteString(ಅಕ್ಷರ *z){
  ಪೂರ್ಣಾಂಕ quote;
  ಪೂರ್ಣಾಂಕ i, j;
  ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ;
  quote = z[0];
  ಆಯ್ಕೆ( quote ){
    ಸ್ಥಿತಿ '\'':  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '"':   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '`':   ನಿಲ್ಲಿಸು;                /* For MySQL compatibility */
    ಸ್ಥಿತಿ '[':   quote = ']';  ನಿಲ್ಲಿಸು;  /* For MS SqlServer compatibility */
    ಪೂರ್ವನಿಯೋಜಿತ:    ಹಿಂತಿರುಗಿ;
  }
  ಗಾಗಿ(i=1, j=0; z[i]; i++){
    ಆದರೆ( z[i]==quote ){
      ಆದರೆ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }ಇಲ್ಲದಿದ್ದರೆ{
        z[j++] = 0;
        ನಿಲ್ಲಿಸು;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(
  sqlite3 *db, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,
  ಅಕ್ಷರ ***pa_ಲಂಬಸಾಲು, 
  ಪೂರ್ಣಾಂಕ *pn_ಲಂಬಸಾಲು
){
  ಅಕ್ಷರ **a_ಲಂಬಸಾಲು = 0;
  ಅಕ್ಷರ *zSql;
  sqlite3_stmt *pStmt = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  ಆದರೆ( !zSql ){
    rc = SQLITE_NOMEM;
    ಹೋಗು ಹೊರಹೋಗು;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ ii;
    ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು;
    ಅಕ್ಷರ *zSpace;
    n_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    n_ಬೈಟುಗಳು = ಗಾತ್ರ(ಅಕ್ಷರ *) * n_ಲಂಬಸಾಲು;
    ಗಾಗಿ(ii=0; ii<n_ಲಂಬಸಾಲು; ii++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = sqlite3_ಸ್ತಂಭ_name(pStmt, ii);
      ಆದರೆ( !zName ){
        rc = SQLITE_NOMEM;
        ಹೋಗು ಹೊರಹೋಗು;
      }
      n_ಬೈಟುಗಳು += (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName)+1;
    }
    a_ಲಂಬಸಾಲು = (ಅಕ್ಷರ **)sqlite3MallocZero(n_ಬೈಟುಗಳು);
    ಆದರೆ( !a_ಲಂಬಸಾಲು ){
      rc = SQLITE_NOMEM;
      ಹೋಗು ಹೊರಹೋಗು;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (ಅಕ್ಷರ *)(&a_ಲಂಬಸಾಲು[n_ಲಂಬಸಾಲು]);
    ಗಾಗಿ(ii=0; ii<n_ಲಂಬಸಾಲು; ii++){
      a_ಲಂಬಸಾಲು[ii] = zSpace;
      sqlite3_snprintf(n_ಬೈಟುಗಳು, zSpace, "%s", sqlite3_ಸ್ತಂಭ_name(pStmt,ii));
      zSpace += (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zSpace) + 1;
    }
    ಪ್ರತಿಜ್ಞಾ( (zSpace-n_ಬೈಟುಗಳು)==(ಅಕ್ಷರ *)a_ಲಂಬಸಾಲು );
  }

  *pa_ಲಂಬಸಾಲು = a_ಲಂಬಸಾಲು;
  *pn_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು;

ಹೊರಹೋಗು:
  sqlite3_finalize(pStmt);
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_ಸೂಚ್ಯಂಕ_Array(
  sqlite3 *db,             /* Database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,        /* Name of table in database db */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು,
  ಪೂರ್ಣಾಂಕ **pa_ಸೂಚ್ಯಂಕ
){
  sqlite3_stmt *pStmt = 0;
  ಪೂರ್ಣಾಂಕ *a_ಸೂಚ್ಯಂಕ = 0;
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ *zSql;

  /* Allocate space for the index array */
  a_ಸೂಚ್ಯಂಕ = (ಪೂರ್ಣಾಂಕ *)sqlite3MallocZero(ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) * n_ಲಂಬಸಾಲು);
  ಆದರೆ( !a_ಸೂಚ್ಯಂಕ ){
    rc = SQLITE_NOMEM;
    ಹೋಗು get_ಸೂಚ್ಯಂಕ_array_ಹೊರಹೋಗು;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  ಆದರೆ( !zSql ){
    rc = SQLITE_NOMEM;
    ಹೋಗು get_ಸೂಚ್ಯಂಕ_array_ಹೊರಹೋಗು;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  ಆಗ( pStmt && sqlite3_ಹೆಜ್ಜೆ(pStmt)==SQLITE_ಅಡ್ಡಸಾಲು ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zIdx = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    ಆದರೆ( zIdx==0 ) ಮುಂದುವರಿ;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    ಆದರೆ( !zSql ){
      rc = SQLITE_NOMEM;
      ಹೋಗು get_ಸೂಚ್ಯಂಕ_array_ಹೊರಹೋಗು;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    ಆದರೆ( pStmt2 && sqlite3_ಹೆಜ್ಜೆ(pStmt2)==SQLITE_ಅಡ್ಡಸಾಲು ){
      ಪೂರ್ಣಾಂಕ cid = sqlite3_ಸ್ತಂಭ_int(pStmt2, 1);
      ಪ್ರತಿಜ್ಞಾ( cid>=0 && cid<n_ಲಂಬಸಾಲು );
      a_ಸೂಚ್ಯಂಕ[cid] = 1;
    }
    ಆದರೆ( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು get_ಸೂಚ್ಯಂಕ_array_ಹೊರಹೋಗು;
    }
  }


get_ಸೂಚ್ಯಂಕ_array_ಹೊರಹೋಗು:
  ಆದರೆ( pStmt ){
    ಪೂರ್ಣಾಂಕ rc2 = sqlite3_finalize(pStmt);
    ಆದರೆ( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3_free(a_ಸೂಚ್ಯಂಕ);
    a_ಸೂಚ್ಯಂಕ = 0;
  }
  *pa_ಸೂಚ್ಯಂಕ = a_ಸೂಚ್ಯಂಕ;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ಸ್ಥಿರ ಶೂನ್ಯ append_ಗಮ್ಯ_Echo_Module(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg){
  ಪೂರ್ಣಾಂಕ flags = (TCL_APPEND_ಮೌಲ್ಯ | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_ಗಣ_Var(interp, "echo_module", (zArg?zArg:""), flags);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಆದರೆ( pVtab->z_ಕೋಷ್ಟಕ_Name ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_bind_ಪಠ್ಯ(pStmt, 1, pVtab->z_ಕೋಷ್ಟಕ_Name, -1, 0);
      ಆದರೆ( sqlite3_ಹೆಜ್ಜೆ(pStmt)==SQLITE_ಅಡ್ಡಸಾಲು ){
        ಪೂರ್ಣಾಂಕ rc2;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_Create_ಕೋಷ್ಟಕ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 0);
        rc = sqlite3_declare_vtab(db, z_Create_ಕೋಷ್ಟಕ);
        rc2 = sqlite3_finalize(pStmt);
        ಆದರೆ( rc==SQLITE_OK ){
          rc = rc2;
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        rc = sqlite3_finalize(pStmt);
        ಆದರೆ( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      ಆದರೆ( rc==SQLITE_OK ){
        rc = get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(db, pVtab->z_ಕೋಷ್ಟಕ_Name, &pVtab->a_ಲಂಬಸಾಲು, &pVtab->n_ಲಂಬಸಾಲು);
      }
      ಆದರೆ( rc==SQLITE_OK ){
        rc = get_ಸೂಚ್ಯಂಕ_Array(db, pVtab->z_ಕೋಷ್ಟಕ_Name, pVtab->n_ಲಂಬಸಾಲು, &pVtab->a_ಸೂಚ್ಯಂಕ);
      }
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_ಸೂಚ್ಯಂಕ);
  sqlite3_free(p->a_ಲಂಬಸಾಲು);
  sqlite3_free(p->zThis);
  sqlite3_free(p->z_ಕೋಷ್ಟಕ_Name);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ 0;
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ EchoModule EchoModule;
ರಚನೆ EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoConstructor(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( ಗಾತ್ರ(*pVtab) );
  ಆದರೆ( !pVtab ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  ಆದರೆ( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  ಆದರೆ( argc>3 ){
    pVtab->z_ಕೋಷ್ಟಕ_Name = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->z_ಕೋಷ್ಟಕ_Name);
    ಆದರೆ( pVtab->z_ಕೋಷ್ಟಕ_Name && pVtab->z_ಕೋಷ್ಟಕ_Name[0]=='*' ){
      ಅಕ್ಷರ *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->z_ಕೋಷ್ಟಕ_Name[1]));
      sqlite3_free(pVtab->z_ಕೋಷ್ಟಕ_Name);
      pVtab->z_ಕೋಷ್ಟಕ_Name = z;
      pVtab->is_ನಮೂನೆ = 1;
    }
    ಆದರೆ( !pVtab->z_ಕೋಷ್ಟಕ_Name ){
      echoDestructor((sqlite3_vtab *)pVtab);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  ಗಾಗಿ(i=0; i<argc; i++){
    append_ಗಮ್ಯ_Echo_Module(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  ಆದರೆ( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    ಹಿಂತಿರುಗಿ rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoCreate(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  append_ಗಮ್ಯ_Echo_Module(((EchoModule *)pAux)->interp, "xCreate");
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
  ಆದರೆ( rc==SQLITE_OK && argc==5 ){
    ಅಕ್ಷರ *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    ಆದರೆ( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  ಆದರೆ( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  ಆದರೆ( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->ಒಳ_Transaction = 1;
  }

  ಹಿಂತಿರುಗಿ rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  append_ಗಮ್ಯ_Echo_Module(((EchoModule *)pAux)->interp, "xConnect");
  ಹಿಂತಿರುಗಿ echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoDisconnect(sqlite3_vtab *pVtab){
  append_ಗಮ್ಯ_Echo_Module(((echo_vtab *)pVtab)->interp, "xDisconnect");
  ಹಿಂತಿರುಗಿ echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoDestroy(sqlite3_vtab *pVtab){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  append_ಗಮ್ಯ_Echo_Module(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  ಆದರೆ( p && p->zLogName ){
    ಅಕ್ಷರ *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  ಆದರೆ( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(ಗಾತ್ರ(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  ಹಿಂತಿರುಗಿ (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoClose(sqlite3_vtab_cursor *cur){
  ಪೂರ್ಣಾಂಕ rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoEof(sqlite3_vtab_cursor *cur){
  ಹಿಂತಿರುಗಿ (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echo_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  ಆದರೆ( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( pCur->pStmt ){
    rc = sqlite3_ಹೆಜ್ಜೆ(pCur->pStmt);
    ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echo_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ಆದರೆ( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( !pStmt ){
    sqlite3_ಫಲಿತಾಂಶ_null(ctx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ದತ್ತಾಂಶ_ಎಣಿಕೆ(pStmt)>i_ಲಂಬಸಾಲು );
    sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pStmt, i_ಲಂಬಸಾಲು));
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  ಆದರೆ( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  *pRowid = sqlite3_ಸ್ತಂಭ_int64(pStmt, 0);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hashString(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zString){
  u32 val = 0;
  ಪೂರ್ಣಾಂಕ ii;
  ಗಾಗಿ(ii=0; zString[ii]; ii++){
    val = (val << 3) + (ಪೂರ್ಣಾಂಕ)zString[ii];
  }
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  ಆದರೆ( simulateVtabError(pVtab, "xFilter") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  ಪ್ರತಿಜ್ಞಾ( idx_ಅಂಕೆ==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  append_ಗಮ್ಯ_Echo_Module(pVtab->interp, "xFilter");
  append_ಗಮ್ಯ_Echo_Module(pVtab->interp, idxStr);
  ಗಾಗಿ(i=0; i<argc; i++){
    append_ಗಮ್ಯ_Echo_Module(pVtab->interp, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  ಪ್ರತಿಜ್ಞಾ( pCur->pStmt || rc!=SQLITE_OK );
  ಗಾಗಿ(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_ಮೌಲ್ಯ(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  ಆದರೆ( rc==SQLITE_OK ){
    rc = echo_ಮುಂದಿನ(pVtabCursor);
  }

  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ string_concat(ಅಕ್ಷರ **pzStr, ಅಕ್ಷರ *zAppend, ಪೂರ್ಣಾಂಕ doFree, ಪೂರ್ಣಾಂಕ *pRc){
  ಅಕ್ಷರ *z_ಒಳ = *pzStr;
  ಆದರೆ( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  ಆದರೆ( *pRc!=SQLITE_OK ){
    sqlite3_free(z_ಒಳ);
    z_ಒಳ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( z_ಒಳ ){
      ಅಕ್ಷರ *z_ತಾತ್ಕಾಲಿಕ = z_ಒಳ;
      z_ಒಳ = sqlite3_mprintf("%s%s", z_ಒಳ, zAppend);
      sqlite3_free(z_ತಾತ್ಕಾಲಿಕ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      z_ಒಳ = sqlite3_mprintf("%s", zAppend);
    }
    ಆದರೆ( !z_ಒಳ ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = z_ಒಳ;
  ಆದರೆ( doFree ){
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
ಸ್ಥಿರ ಅಕ್ಷರ *echoSelectList(echo_vtab *pTab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಅಕ್ಷರ *zRet = 0;
  ಆದರೆ( sqlite3_libversion_ಸಂಖ್ಯೆ()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( pIdxInfo->ಲಂಬಸಾಲು_ಬಳಸಿದ & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->a_ಲಂಬಸಾಲು[i]);
      }ಇಲ್ಲದಿದ್ದರೆ{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      ಆದರೆ( !zRet ) ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ zRet;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echo_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಪೂರ್ಣಾಂಕ ii;
  ಅಕ್ಷರ *zQuery = 0;
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು = 0;
  ಅಕ್ಷರ *zNew;
  ಪೂರ್ಣಾಂಕ nArg = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  ಪೂರ್ಣಾಂಕ n_ಅಡ್ಡಸಾಲು = 0;
  ಪೂರ್ಣಾಂಕ useIdx = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ use_ವೆಚ್ಚ = 0;
  ದ್ವಿಗುಣ ವೆಚ್ಚ = 0;
  ಪೂರ್ಣಾಂಕ isIgnoreUsable = 0;
  ಆದರೆ( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  ಆದರೆ( simulateVtabError(pVtab, "xBestIndex") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  ಆದರೆ( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    ವೆಚ್ಚ = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    use_ವೆಚ್ಚ = 1;
  } ಇಲ್ಲದಿದ್ದರೆ {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->z_ಕೋಷ್ಟಕ_Name);
    ಆದರೆ( !zQuery ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    sqlite3_ಹೆಜ್ಜೆ(pStmt);
    n_ಅಡ್ಡಸಾಲು = sqlite3_ಸ್ತಂಭ_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
  }

  z_ಲಂಬಸಾಲು = echoSelectList(pVtab, pIdxInfo);
  ಆದರೆ( !z_ಲಂಬಸಾಲು ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", z_ಲಂಬಸಾಲು, pVtab->z_ಕೋಷ್ಟಕ_Name);
  ಆದರೆ( !zQuery ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;

  ಗಾಗಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಸ್ಥಿರಾಂಕ ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *pConstraint;
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint_usage *pUsage;
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    ಆದರೆ( !isIgnoreUsable && !pConstraint->usable ) ಮುಂದುವರಿ;

    i_ಲಂಬಸಾಲು = pConstraint->i_ಸ್ತಂಭ;
    ಆದರೆ( i_ಲಂಬಸಾಲು<0 || pVtab->a_ಸೂಚ್ಯಂಕ[i_ಲಂಬಸಾಲು] ){
      ಅಕ್ಷರ *z_New_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು>=0 ? pVtab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು] : "rowid";
      ಅಕ್ಷರ *zOp = 0;
      useIdx = 1;
      ಆಯ್ಕೆ( pConstraint->op ){
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ:
          zOp = "="; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LT:
          zOp = "<"; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GT:
          zOp = ">"; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LE:
          zOp = "<="; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GE:
          zOp = ">="; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಹೊಂದಿಕೆ:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LIKE:
          zOp = "like"; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GLOB:
          zOp = "glob"; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_REGEXP:
          zOp = "regexp"; ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( zOp ){
        ಆದರೆ( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, z_New_ಲಂಬಸಾಲು);
        } ಇಲ್ಲದಿದ್ದರೆ {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, z_New_ಲಂಬಸಾಲು, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_ಸೂಚ್ಯಂಕ = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  ಆದರೆ( pIdxInfo->n_ಕ್ರಮ_By==1 && (
        pIdxInfo->a_ಕ್ರಮ_By->i_ಸ್ತಂಭ<0 ||
        pVtab->a_ಸೂಚ್ಯಂಕ[pIdxInfo->a_ಕ್ರಮ_By->i_ಸ್ತಂಭ]) ){
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pIdxInfo->a_ಕ್ರಮ_By->i_ಸ್ತಂಭ;
    ಅಕ್ಷರ *z_New_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು>=0 ? pVtab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು] : "rowid";
    ಅಕ್ಷರ *zDir = pIdxInfo->a_ಕ್ರಮ_By->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", z_New_ಲಂಬಸಾಲು, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->ಕ್ರಮ_By_Consumed = 1;
  }

  append_ಗಮ್ಯ_Echo_Module(pVtab->interp, "xBestIndex");;
  append_ಗಮ್ಯ_Echo_Module(pVtab->interp, zQuery);

  ಆದರೆ( !zQuery ){
    ಹಿಂತಿರುಗಿ rc;
  }
  pIdxInfo->idx_ಅಂಕೆ = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->need_ಗಮ್ಯ_Free_Idx_Str = 1;
  ಆದರೆ( use_ವೆಚ್ಚ ){
    pIdxInfo->estimated_ವೆಚ್ಚ = ವೆಚ್ಚ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( useIdx ){
    /* Approximation of log2(nRow). */
    ಗಾಗಿ( ii=0; ii<(ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*8)-1; ii++ ){
      ಆದರೆ( n_ಅಡ್ಡಸಾಲು & (1<<ii) ){
        pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)ii;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)n_ಅಡ್ಡಸಾಲು;
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ echoUpdate(
  sqlite3_vtab *tab, 
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ, 
  sqlite3_ಮೌಲ್ಯ **ap_ದತ್ತಾಂಶ, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  ಅಕ್ಷರ *z = 0;               /* SQL statement to execute */
  ಪೂರ್ಣಾಂಕ bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  ಪೂರ್ಣಾಂಕ bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  ಪೂರ್ಣಾಂಕ i;                     /* Counter variable used by for loops */

  ಪ್ರತಿಜ್ಞಾ( n_ದತ್ತಾಂಶ==pVtab->n_ಲಂಬಸಾಲು+2 || n_ದತ್ತಾಂಶ==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  ಪ್ರತಿಜ್ಞಾ( pVtab->ಒಳ_Transaction );

  ಆದರೆ( simulateVtabError(pVtab, "xUpdate") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  ಆದರೆ( n_ದತ್ತಾಂಶ>1 && sqlite3_ಮೌಲ್ಯ_type(ap_ದತ್ತಾಂಶ[0])==SQLITE_INTEGER ){
    ಅಕ್ಷರ *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->z_ಕೋಷ್ಟಕ_Name);
    ಆದರೆ( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_ದತ್ತಾಂಶ[1] && sqlite3_ಮೌಲ್ಯ_type(ap_ದತ್ತಾಂಶ[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    ಆದರೆ( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    ಗಾಗಿ(i=2; i<n_ದತ್ತಾಂಶ; i++){
      ಆದರೆ( ap_ದತ್ತಾಂಶ[i]==0 ) ಮುಂದುವರಿ;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->a_ಲಂಬಸಾಲು[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_ದತ್ತಾಂಶ), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n_ದತ್ತಾಂಶ==1 && sqlite3_ಮೌಲ್ಯ_type(ap_ದತ್ತಾಂಶ[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->z_ಕೋಷ್ಟಕ_Name);
    ಆದರೆ( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n_ದತ್ತಾಂಶ>2 && sqlite3_ಮೌಲ್ಯ_type(ap_ದತ್ತಾಂಶ[0])==SQLITE_NULL ){
    ಪೂರ್ಣಾಂಕ ii;
    ಅಕ್ಷರ *z_ಸೇರಿಸು = 0;
    ಅಕ್ಷರ *z_ಮೌಲ್ಯಗಳು = 0;
  
    z_ಸೇರಿಸು = sqlite3_mprintf("INSERT INTO %Q (", pVtab->z_ಕೋಷ್ಟಕ_Name);
    ಆದರೆ( !z_ಸೇರಿಸು ){
      rc = SQLITE_NOMEM;
    }
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(ap_ದತ್ತಾಂಶ[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_ಮೌಲ್ಯಗಳು = sqlite3_mprintf("?");
      string_concat(&z_ಸೇರಿಸು, "rowid", 0, &rc);
    }

    ಪ್ರತಿಜ್ಞಾ((pVtab->n_ಲಂಬಸಾಲು+2)==n_ದತ್ತಾಂಶ);
    ಗಾಗಿ(ii=2; ii<n_ದತ್ತಾಂಶ; ii++){
      string_concat(&z_ಸೇರಿಸು, 
          sqlite3_mprintf("%s%Q", z_ಮೌಲ್ಯಗಳು?", ":"", pVtab->a_ಲಂಬಸಾಲು[ii-2]), 1, &rc);
      string_concat(&z_ಮೌಲ್ಯಗಳು, 
          sqlite3_mprintf("%s?%d", z_ಮೌಲ್ಯಗಳು?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_ಸೇರಿಸು, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_ಮೌಲ್ಯಗಳು, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ(0);
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  ಆದರೆ( rc==SQLITE_OK ) {
    ಆದರೆ( bindArgZero ){
      sqlite3_bind_ಮೌಲ್ಯ(pStmt, n_ದತ್ತಾಂಶ, ap_ದತ್ತಾಂಶ[0]);
    }
    ಆದರೆ( bindArgOne ){
      sqlite3_bind_ಮೌಲ್ಯ(pStmt, 1, ap_ದತ್ತಾಂಶ[1]);
    }
    ಗಾಗಿ(i=2; i<n_ದತ್ತಾಂಶ && rc==SQLITE_OK; i++){
      ಆದರೆ( ap_ದತ್ತಾಂಶ[i] ) rc = sqlite3_bind_ಮೌಲ್ಯ(pStmt, i, ap_ದತ್ತಾಂಶ[i]);
    }
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_ಹೆಜ್ಜೆ(pStmt);
      rc = sqlite3_finalize(pStmt);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_finalize(pStmt);
    }
  }

  ಆದರೆ( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_ಕೊನೆಯ_ಸೇರಿಸು_rowid(db);
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoTransactionCall(sqlite3_vtab *tab, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCall){
  ಅಕ್ಷರ *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->z_ಕೋಷ್ಟಕ_Name);
  ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  append_ಗಮ್ಯ_Echo_Module(pVtab->interp, zCall);
  append_ಗಮ್ಯ_Echo_Module(pVtab->interp, z);
  sqlite3_free(z);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoBegin(sqlite3_vtab *tab){
  ಪೂರ್ಣಾಂಕ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  ಪ್ರತಿಜ್ಞಾ( !pVtab->ಒಳ_Transaction );

  ಆದರೆ( simulateVtabError(pVtab, "xBegin") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  ಆದರೆ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    ಆದರೆ( zVal && 0==strcmp(zVal, pVtab->z_ಕೋಷ್ಟಕ_Name) ){
      rc = SQLITE_ERROR;
    }
  }
  ಆದರೆ( rc==SQLITE_OK ){
    pVtab->ಒಳ_Transaction = 1;
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoSync(sqlite3_vtab *tab){
  ಪೂರ್ಣಾಂಕ rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  ಪ್ರತಿಜ್ಞಾ( pVtab->ಒಳ_Transaction );

  ಆದರೆ( simulateVtabError(pVtab, "xSync") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  ಆದರೆ( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    ಆದರೆ( zVal && 0==strcmp(zVal, pVtab->z_ಕೋಷ್ಟಕ_Name) ){
      rc = -1;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  ಪೂರ್ಣಾಂಕ rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  ಪ್ರತಿಜ್ಞಾ( pVtab->ಒಳ_Transaction );

  ಆದರೆ( simulateVtabError(pVtab, "xCommit") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3_ಕೊನೆ_Benign_Malloc();
  pVtab->ಒಳ_Transaction = 0;
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoRollback(sqlite3_vtab *tab){
  ಪೂರ್ಣಾಂಕ rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  ಪ್ರತಿಜ್ಞಾ( pVtab->ಒಳ_Transaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->ಒಳ_Transaction = 0;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ಸ್ಥಿರ ಶೂನ್ಯ overloadedGlobFunction(
  sqlite3_context *pContext,
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **apArg
){
  Tcl_Interp *interp = sqlite3_user_ದತ್ತಾಂಶ(pContext);
  Tcl_DString str;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  ಗಾಗಿ(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_ಮೌಲ್ಯ(&str));
  Tcl_DStringFree(&str);
  ಆದರೆ( rc ){
    sqlite3_ಫಲಿತಾಂಶ_error(pContext, Tcl_Get_String_ಫಲಿತಾಂಶ(interp), -1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pContext, Tcl_Get_String_ಫಲಿತಾಂಶ(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echo_ಕಂಡುಹಿಡಿ_Function(
  sqlite3_vtab *vtab,
  ಪೂರ್ಣಾಂಕ nArg,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFuncName,
  ಶೂನ್ಯ (**pxFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  ಆದರೆ( strcmp(zFuncName,"glob")!=0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoRename(sqlite3_vtab *vtab, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNewName){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  ಆದರೆ( simulateVtabError(p, "xRename") ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( p->is_ನಮೂನೆ ){
    ಪೂರ್ಣಾಂಕ nThis = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p->zThis);
    ಅಕ್ಷರ *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->z_ಕೋಷ್ಟಕ_Name, zNewName, &p->z_ಕೋಷ್ಟಕ_Name[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoSavepoint(sqlite3_vtab *pVTab, ಪೂರ್ಣಾಂಕ iSavepoint){
  ಪ್ರತಿಜ್ಞಾ( pVTab );
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echoRelease(sqlite3_vtab *pVTab, ಪೂರ್ಣಾಂಕ iSavepoint){
  ಪ್ರತಿಜ್ಞಾ( pVTab );
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ echo_Rollback_ಗಮ್ಯ(sqlite3_vtab *pVTab, ಪೂರ್ಣಾಂಕ iSavepoint){
  ಪ್ರತಿಜ್ಞಾ( pVTab );
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ಸ್ಥಿರ sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_ಮುಂದಿನ,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_ಸ್ತಂಭ,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ಕಂಡುಹಿಡಿ_Function,          /* xFindFunction - function overloading */
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
  echo_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_ಮುಂದಿನ,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_ಸ್ತಂಭ,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ಕಂಡುಹಿಡಿ_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  echoSavepoint,
  echoRelease,
  echo_Rollback_ಗಮ್ಯ,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

ಸ್ಥಿರ ಶೂನ್ಯ moduleDestroy(ಶೂನ್ಯ *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_echo_module(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3 *db;
  EchoModule *pMod;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(ಗಾತ್ರ(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (ಶೂನ್ಯ*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  ಆದರೆ( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(ಗಾತ್ರ(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (ಶೂನ್ಯ*)pMod, moduleDestroy
    );
  }

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI declare_vtab(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB SQL");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3_errmsg(db), TCL_VOLATILE);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest8_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "register_echo_module",       ರಿಜಿಸ್ಟರ್_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}
