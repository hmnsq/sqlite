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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE

प्रकारनिर्देश संरचना echo_vtab echo_vtab;
प्रकारनिर्देश संरचना echo_cursor echo_cursor;

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
संरचना echo_vtab {
  sqlite3_vtab आधारः;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  पूर्णाङ्क is_आदर्शः;
  पूर्णाङ्क अन्तर्_Transaction;      /* True if within a transaction */
  वर्ण *zThis;            /* Name of the echo table */
  वर्ण *z_सारणी_Name;       /* Name of the real table */
  वर्ण *zLogName;         /* Name of the log table */
  पूर्णाङ्क nCol;               /* Number of columns in the real table */
  पूर्णाङ्क *a_सूचकांक;            /* Array of size nCol. True if column has an index */
  वर्ण **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
संरचना echo_cursor {
  sqlite3_vtab_cursor आधारः;
  sqlite3_stmt *pStmt;
};

स्थिर पूर्णाङ्क simulateVtabError(echo_vtab *p, ध्रुव वर्ण *z_विधानम्){
  ध्रुव वर्ण *zErr;
  वर्ण zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", z_विधानम्, p->z_सारणी_Name);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  यदि( zErr ){
    p->आधारः.z_Err_सन्देशः = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  प्रत्यागम (zErr!=0);
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
स्थिर शून्य dequoteString(वर्ण *z){
  पूर्णाङ्क quote;
  पूर्णाङ्क i, j;
  यदि( z==0 ) प्रत्यागम;
  quote = z[0];
  विकल्प( quote ){
    पक्ष '\'':  भङ्ग;
    पक्ष '"':   भङ्ग;
    पक्ष '`':   भङ्ग;                /* For MySQL compatibility */
    पक्ष '[':   quote = ']';  भङ्ग;  /* For MS SqlServer compatibility */
    स्वभाव:    प्रत्यागम;
  }
  प्रति(i=1, j=0; z[i]; i++){
    यदि( z[i]==quote ){
      यदि( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }अन्यथा{
        z[j++] = 0;
        भङ्ग;
      }
    }अन्यथा{
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
स्थिर पूर्णाङ्क get_Column_नामानि(
  sqlite3 *db, 
  ध्रुव वर्ण *zTab,
  वर्ण ***paCol, 
  पूर्णाङ्क *pnCol
){
  वर्ण **aCol = 0;
  वर्ण *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  यदि( !zSql ){
    rc = SQLITE_NOMEM;
    गच्छ बहिर्गमनम्;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  यदि( rc==SQLITE_OK ){
    पूर्णाङ्क ii;
    पूर्णाङ्क n_अष्टकानि;
    वर्ण *zSpace;
    nCol = sqlite3_column_गणनम्(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    n_अष्टकानि = परिमाण(वर्ण *) * nCol;
    प्रति(ii=0; ii<nCol; ii++){
      ध्रुव वर्ण *zName = sqlite3_column_name(pStmt, ii);
      यदि( !zName ){
        rc = SQLITE_NOMEM;
        गच्छ बहिर्गमनम्;
      }
      n_अष्टकानि += (पूर्णाङ्क)वाक्यदैर्घ्यम्(zName)+1;
    }
    aCol = (वर्ण **)sqlite3MallocZero(n_अष्टकानि);
    यदि( !aCol ){
      rc = SQLITE_NOMEM;
      गच्छ बहिर्गमनम्;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (वर्ण *)(&aCol[nCol]);
    प्रति(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(n_अष्टकानि, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (पूर्णाङ्क)वाक्यदैर्घ्यम्(zSpace) + 1;
    }
    प्रतिज्ञा( (zSpace-n_अष्टकानि)==(वर्ण *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

बहिर्गमनम्:
  sqlite3_finalize(pStmt);
  प्रत्यागम rc;
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
स्थिर पूर्णाङ्क get_सूचकांक_Array(
  sqlite3 *db,             /* Database connection */
  ध्रुव वर्ण *zTab,        /* Name of table in database db */
  पूर्णाङ्क nCol,
  पूर्णाङ्क **pa_सूचकांक
){
  sqlite3_stmt *pStmt = 0;
  पूर्णाङ्क *a_सूचकांक = 0;
  पूर्णाङ्क rc;
  वर्ण *zSql;

  /* Allocate space for the index array */
  a_सूचकांक = (पूर्णाङ्क *)sqlite3MallocZero(परिमाण(पूर्णाङ्क) * nCol);
  यदि( !a_सूचकांक ){
    rc = SQLITE_NOMEM;
    गच्छ get_सूचकांक_array_बहिर्गमनम्;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  यदि( !zSql ){
    rc = SQLITE_NOMEM;
    गच्छ get_सूचकांक_array_बहिर्गमनम्;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  यावत्( pStmt && sqlite3_चरणः(pStmt)==SQLITE_ROW ){
    ध्रुव वर्ण *zIdx = (ध्रुव वर्ण *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    यदि( zIdx==0 ) अनुवर्त;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    यदि( !zSql ){
      rc = SQLITE_NOMEM;
      गच्छ get_सूचकांक_array_बहिर्गमनम्;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    यदि( pStmt2 && sqlite3_चरणः(pStmt2)==SQLITE_ROW ){
      पूर्णाङ्क cid = sqlite3_column_int(pStmt2, 1);
      प्रतिज्ञा( cid>=0 && cid<nCol );
      a_सूचकांक[cid] = 1;
    }
    यदि( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    यदि( rc!=SQLITE_OK ){
      गच्छ get_सूचकांक_array_बहिर्गमनम्;
    }
  }


get_सूचकांक_array_बहिर्गमनम्:
  यदि( pStmt ){
    पूर्णाङ्क rc2 = sqlite3_finalize(pStmt);
    यदि( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  यदि( rc!=SQLITE_OK ){
    sqlite3_free(a_सूचकांक);
    a_सूचकांक = 0;
  }
  *pa_सूचकांक = a_सूचकांक;
  प्रत्यागम rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
स्थिर शून्य append_गन्तव्यम्_Echo_Module(Tcl_Interp *interp, ध्रुव वर्ण *zArg){
  पूर्णाङ्क flags = (TCL_APPEND_मानम् | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_समुच्चयः_Var(interp, "echo_module", (zArg?zArg:""), flags);
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
स्थिर पूर्णाङ्क echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  पूर्णाङ्क rc = SQLITE_OK;

  यदि( pVtab->z_सारणी_Name ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    यदि( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->z_सारणी_Name, -1, 0);
      यदि( sqlite3_चरणः(pStmt)==SQLITE_ROW ){
        पूर्णाङ्क rc2;
        ध्रुव वर्ण *z_Create_सारणी = (ध्रुव वर्ण *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, z_Create_सारणी);
        rc2 = sqlite3_finalize(pStmt);
        यदि( rc==SQLITE_OK ){
          rc = rc2;
        }
      } अन्यथा {
        rc = sqlite3_finalize(pStmt);
        यदि( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      यदि( rc==SQLITE_OK ){
        rc = get_Column_नामानि(db, pVtab->z_सारणी_Name, &pVtab->aCol, &pVtab->nCol);
      }
      यदि( rc==SQLITE_OK ){
        rc = get_सूचकांक_Array(db, pVtab->z_सारणी_Name, pVtab->nCol, &pVtab->a_सूचकांक);
      }
    }
  }

  प्रत्यागम rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
स्थिर पूर्णाङ्क echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_सूचकांक);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->z_सारणी_Name);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  प्रत्यागम 0;
}

प्रकारनिर्देश संरचना EchoModule EchoModule;
संरचना EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
स्थिर पूर्णाङ्क echoConstructor(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  पूर्णाङ्क rc;
  पूर्णाङ्क i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( परिमाण(*pVtab) );
  यदि( !pVtab ){
    प्रत्यागम SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  यदि( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    प्रत्यागम SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  यदि( argc>3 ){
    pVtab->z_सारणी_Name = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->z_सारणी_Name);
    यदि( pVtab->z_सारणी_Name && pVtab->z_सारणी_Name[0]=='*' ){
      वर्ण *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->z_सारणी_Name[1]));
      sqlite3_free(pVtab->z_सारणी_Name);
      pVtab->z_सारणी_Name = z;
      pVtab->is_आदर्शः = 1;
    }
    यदि( !pVtab->z_सारणी_Name ){
      echoDestructor((sqlite3_vtab *)pVtab);
      प्रत्यागम SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  प्रति(i=0; i<argc; i++){
    append_गन्तव्यम्_Echo_Module(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  यदि( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    प्रत्यागम rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->आधारः;
  प्रत्यागम SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
स्थिर पूर्णाङ्क echoCreate(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  पूर्णाङ्क rc = SQLITE_OK;
  append_गन्तव्यम्_Echo_Module(((EchoModule *)pAux)->interp, "xCreate");
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
  यदि( rc==SQLITE_OK && argc==5 ){
    वर्ण *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    यदि( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  यदि( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  यदि( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->अन्तर्_Transaction = 1;
  }

  प्रत्यागम rc;
}

/* 
** Echo virtual table module xConnect method.
*/
स्थिर पूर्णाङ्क echoConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  append_गन्तव्यम्_Echo_Module(((EchoModule *)pAux)->interp, "xConnect");
  प्रत्यागम echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
स्थिर पूर्णाङ्क echoDisconnect(sqlite3_vtab *pVtab){
  append_गन्तव्यम्_Echo_Module(((echo_vtab *)pVtab)->interp, "xDisconnect");
  प्रत्यागम echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
स्थिर पूर्णाङ्क echoDestroy(sqlite3_vtab *pVtab){
  पूर्णाङ्क rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  append_गन्तव्यम्_Echo_Module(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  यदि( p && p->zLogName ){
    वर्ण *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  यदि( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  प्रत्यागम rc;
}

/* 
** Echo virtual table module xOpen method.
*/
स्थिर पूर्णाङ्क echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  यदि( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    प्रत्यागम SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(परिमाण(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  प्रत्यागम (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
स्थिर पूर्णाङ्क echoClose(sqlite3_vtab_cursor *cur){
  पूर्णाङ्क rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  प्रत्यागम rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
स्थिर पूर्णाङ्क echoEof(sqlite3_vtab_cursor *cur){
  प्रत्यागम (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
स्थिर पूर्णाङ्क echo_अग्र(sqlite3_vtab_cursor *cur){
  पूर्णाङ्क rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  यदि( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( pCur->pStmt ){
    rc = sqlite3_चरणः(pCur->pStmt);
    यदि( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }अन्यथा{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  प्रत्यागम rc;
}

/* 
** Echo virtual table module xColumn method.
*/
स्थिर पूर्णाङ्क echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  पूर्णाङ्क iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  यदि( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( !pStmt ){
    sqlite3_परिणाम_null(ctx);
  }अन्यथा{
    प्रतिज्ञा( sqlite3_दत्त_गणनम्(pStmt)>iCol );
    sqlite3_परिणाम_मानम्(ctx, sqlite3_column_मानम्(pStmt, iCol));
  }
  प्रत्यागम SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
स्थिर पूर्णाङ्क echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  यदि( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    प्रत्यागम SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क hashString(ध्रुव वर्ण *zString){
  u32 val = 0;
  पूर्णाङ्क ii;
  प्रति(ii=0; zString[ii]; ii++){
    val = (val << 3) + (पूर्णाङ्क)zString[ii];
  }
  प्रत्यागम (पूर्णाङ्क)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
स्थिर पूर्णाङ्क echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अङ्कः, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मानम् **argv
){
  पूर्णाङ्क rc;
  पूर्णाङ्क i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  यदि( simulateVtabError(pVtab, "xFilter") ){
    प्रत्यागम SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  प्रतिज्ञा( idx_अङ्कः==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  append_गन्तव्यम्_Echo_Module(pVtab->interp, "xFilter");
  append_गन्तव्यम्_Echo_Module(pVtab->interp, idxStr);
  प्रति(i=0; i<argc; i++){
    append_गन्तव्यम्_Echo_Module(pVtab->interp, (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  प्रतिज्ञा( pCur->pStmt || rc!=SQLITE_OK );
  प्रति(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_मानम्(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  यदि( rc==SQLITE_OK ){
    rc = echo_अग्र(pVtabCursor);
  }

  प्रत्यागम rc;
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
स्थिर शून्य string_concat(वर्ण **pzStr, वर्ण *zAppend, पूर्णाङ्क doFree, पूर्णाङ्क *pRc){
  वर्ण *z_अन्तर् = *pzStr;
  यदि( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  यदि( *pRc!=SQLITE_OK ){
    sqlite3_free(z_अन्तर्);
    z_अन्तर् = 0;
  }अन्यथा{
    यदि( z_अन्तर् ){
      वर्ण *z_अस्थायी = z_अन्तर्;
      z_अन्तर् = sqlite3_mprintf("%s%s", z_अन्तर्, zAppend);
      sqlite3_free(z_अस्थायी);
    }अन्यथा{
      z_अन्तर् = sqlite3_mprintf("%s", zAppend);
    }
    यदि( !z_अन्तर् ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = z_अन्तर्;
  यदि( doFree ){
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
स्थिर वर्ण *echoSelectList(echo_vtab *pTab, sqlite3_सूचकांक_info *pIdxInfo){
  वर्ण *zRet = 0;
  यदि( sqlite3_libversion_सङ्ख्या()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }अन्यथा{
    पूर्णाङ्क i;
    प्रति(i=0; i<pTab->nCol; i++){
      यदि( pIdxInfo->col_प्रयुक्तम् & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }अन्यथा{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      यदि( !zRet ) भङ्ग;
    }
  }
  प्रत्यागम zRet;
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
स्थिर पूर्णाङ्क echo_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णाङ्क ii;
  वर्ण *zQuery = 0;
  वर्ण *zCol = 0;
  वर्ण *zNew;
  पूर्णाङ्क nArg = 0;
  ध्रुव वर्ण *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  पूर्णाङ्क nRow = 0;
  पूर्णाङ्क useIdx = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क useCost = 0;
  द्विगुण cost = 0;
  पूर्णाङ्क isIgnoreUsable = 0;
  यदि( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  यदि( simulateVtabError(pVtab, "xBestIndex") ){
    प्रत्यागम SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  यदि( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } अन्यथा {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->z_सारणी_Name);
    यदि( !zQuery ){
      प्रत्यागम SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    यदि( rc!=SQLITE_OK ){
      प्रत्यागम rc;
    }
    sqlite3_चरणः(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    यदि( rc!=SQLITE_OK ){
      प्रत्यागम rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  यदि( !zCol ) प्रत्यागम SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->z_सारणी_Name);
  यदि( !zQuery ) प्रत्यागम SQLITE_NOMEM;

  प्रति(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ध्रुव संरचना sqlite3_सूचकांक_constraint *pConstraint;
    संरचना sqlite3_सूचकांक_constraint_usage *pUsage;
    पूर्णाङ्क iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    यदि( !isIgnoreUsable && !pConstraint->usable ) अनुवर्त;

    iCol = pConstraint->iColumn;
    यदि( iCol<0 || pVtab->a_सूचकांक[iCol] ){
      वर्ण *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      वर्ण *zOp = 0;
      useIdx = 1;
      विकल्प( pConstraint->op ){
        पक्ष SQLITE_सूचकांक_CONSTRAINT_EQ:
          zOp = "="; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_LT:
          zOp = "<"; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_GT:
          zOp = ">"; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_LE:
          zOp = "<="; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_GE:
          zOp = ">="; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_मेलनम्:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_LIKE:
          zOp = "like"; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_GLOB:
          zOp = "glob"; भङ्ग;
        पक्ष SQLITE_सूचकांक_CONSTRAINT_REGEXP:
          zOp = "regexp"; भङ्ग;
      }
      यदि( zOp ){
        यदि( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } अन्यथा {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, zNewCol, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_सूचकांक = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  यदि( pIdxInfo->n_क्रमः_By==1 && (
        pIdxInfo->a_क्रमः_By->iColumn<0 ||
        pVtab->a_सूचकांक[pIdxInfo->a_क्रमः_By->iColumn]) ){
    पूर्णाङ्क iCol = pIdxInfo->a_क्रमः_By->iColumn;
    वर्ण *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    वर्ण *zDir = pIdxInfo->a_क्रमः_By->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->क्रमः_By_Consumed = 1;
  }

  append_गन्तव्यम्_Echo_Module(pVtab->interp, "xBestIndex");;
  append_गन्तव्यम्_Echo_Module(pVtab->interp, zQuery);

  यदि( !zQuery ){
    प्रत्यागम rc;
  }
  pIdxInfo->idx_अङ्कः = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->need_गन्तव्यम्_Free_Idx_Str = 1;
  यदि( useCost ){
    pIdxInfo->estimatedCost = cost;
  }अन्यथा यदि( useIdx ){
    /* Approximation of log2(nRow). */
    प्रति( ii=0; ii<(परिमाण(पूर्णाङ्क)*8)-1; ii++ ){
      यदि( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (द्विगुण)ii;
      }
    }
  }अन्यथा{
    pIdxInfo->estimatedCost = (द्विगुण)nRow;
  }
  प्रत्यागम rc;
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
पूर्णाङ्क echoUpdate(
  sqlite3_vtab *tab, 
  पूर्णाङ्क n_दत्त, 
  sqlite3_मानम् **ap_दत्त, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  पूर्णाङ्क rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  वर्ण *z = 0;               /* SQL statement to execute */
  पूर्णाङ्क bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  पूर्णाङ्क bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  पूर्णाङ्क i;                     /* Counter variable used by for loops */

  प्रतिज्ञा( n_दत्त==pVtab->nCol+2 || n_दत्त==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  प्रतिज्ञा( pVtab->अन्तर्_Transaction );

  यदि( simulateVtabError(pVtab, "xUpdate") ){
    प्रत्यागम SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  यदि( n_दत्त>1 && sqlite3_मानम्_type(ap_दत्त[0])==SQLITE_INTEGER ){
    वर्ण *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->z_सारणी_Name);
    यदि( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_दत्त[1] && sqlite3_मानम्_type(ap_दत्त[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    यदि( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    प्रति(i=2; i<n_दत्त; i++){
      यदि( ap_दत्त[i]==0 ) अनुवर्त;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_दत्त), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  अन्यथा यदि( n_दत्त==1 && sqlite3_मानम्_type(ap_दत्त[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->z_सारणी_Name);
    यदि( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  अन्यथा यदि( n_दत्त>2 && sqlite3_मानम्_type(ap_दत्त[0])==SQLITE_NULL ){
    पूर्णाङ्क ii;
    वर्ण *z_निवेशय = 0;
    वर्ण *z_मानानि = 0;
  
    z_निवेशय = sqlite3_mprintf("INSERT INTO %Q (", pVtab->z_सारणी_Name);
    यदि( !z_निवेशय ){
      rc = SQLITE_NOMEM;
    }
    यदि( sqlite3_मानम्_type(ap_दत्त[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_मानानि = sqlite3_mprintf("?");
      string_concat(&z_निवेशय, "rowid", 0, &rc);
    }

    प्रतिज्ञा((pVtab->nCol+2)==n_दत्त);
    प्रति(ii=2; ii<n_दत्त; ii++){
      string_concat(&z_निवेशय, 
          sqlite3_mprintf("%s%Q", z_मानानि?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_मानानि, 
          sqlite3_mprintf("%s?%d", z_मानानि?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_निवेशय, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_मानानि, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  अन्यथा{
    प्रतिज्ञा(0);
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  प्रतिज्ञा( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  यदि( rc==SQLITE_OK ) {
    यदि( bindArgZero ){
      sqlite3_bind_मानम्(pStmt, n_दत्त, ap_दत्त[0]);
    }
    यदि( bindArgOne ){
      sqlite3_bind_मानम्(pStmt, 1, ap_दत्त[1]);
    }
    प्रति(i=2; i<n_दत्त && rc==SQLITE_OK; i++){
      यदि( ap_दत्त[i] ) rc = sqlite3_bind_मानम्(pStmt, i, ap_दत्त[i]);
    }
    यदि( rc==SQLITE_OK ){
      sqlite3_चरणः(pStmt);
      rc = sqlite3_finalize(pStmt);
    }अन्यथा{
      sqlite3_finalize(pStmt);
    }
  }

  यदि( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_चरमः_निवेशय_rowid(db);
  }
  यदि( rc!=SQLITE_OK ){
    tab->z_Err_सन्देशः = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  प्रत्यागम rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
स्थिर पूर्णाङ्क echoTransactionCall(sqlite3_vtab *tab, ध्रुव वर्ण *zCall){
  वर्ण *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->z_सारणी_Name);
  यदि( z==0 ) प्रत्यागम SQLITE_NOMEM;
  append_गन्तव्यम्_Echo_Module(pVtab->interp, zCall);
  append_गन्तव्यम्_Echo_Module(pVtab->interp, z);
  sqlite3_free(z);
  प्रत्यागम SQLITE_OK;
}
स्थिर पूर्णाङ्क echoBegin(sqlite3_vtab *tab){
  पूर्णाङ्क rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ध्रुव वर्ण *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  प्रतिज्ञा( !pVtab->अन्तर्_Transaction );

  यदि( simulateVtabError(pVtab, "xBegin") ){
    प्रत्यागम SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  यदि( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    यदि( zVal && 0==वाक्यं_तुलय(zVal, pVtab->z_सारणी_Name) ){
      rc = SQLITE_ERROR;
    }
  }
  यदि( rc==SQLITE_OK ){
    pVtab->अन्तर्_Transaction = 1;
  }
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क echoSync(sqlite3_vtab *tab){
  पूर्णाङ्क rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ध्रुव वर्ण *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  प्रतिज्ञा( pVtab->अन्तर्_Transaction );

  यदि( simulateVtabError(pVtab, "xSync") ){
    प्रत्यागम SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  यदि( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    यदि( zVal && 0==वाक्यं_तुलय(zVal, pVtab->z_सारणी_Name) ){
      rc = -1;
    }
  }
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  पूर्णाङ्क rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  प्रतिज्ञा( pVtab->अन्तर्_Transaction );

  यदि( simulateVtabError(pVtab, "xCommit") ){
    प्रत्यागम SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3_अन्तः_Benign_Malloc();
  pVtab->अन्तर्_Transaction = 0;
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क echoRollback(sqlite3_vtab *tab){
  पूर्णाङ्क rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  प्रतिज्ञा( pVtab->अन्तर्_Transaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->अन्तर्_Transaction = 0;
  प्रत्यागम rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
स्थिर शून्य overloadedGlobFunction(
  sqlite3_context *pContext,
  पूर्णाङ्क nArg,
  sqlite3_मानम् **apArg
){
  Tcl_Interp *interp = sqlite3_user_दत्त(pContext);
  Tcl_DString str;
  पूर्णाङ्क i;
  पूर्णाङ्क rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  प्रति(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (वर्ण*)sqlite3_मानम्_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_मानम्(&str));
  Tcl_DStringFree(&str);
  यदि( rc ){
    sqlite3_परिणाम_error(pContext, Tcl_Get_String_परिणाम(interp), -1);
  }अन्यथा{
    sqlite3_परिणाम_text(pContext, Tcl_Get_String_परिणाम(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_परिणाम(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
स्थिर पूर्णाङ्क echoFindFunction(
  sqlite3_vtab *vtab,
  पूर्णाङ्क nArg,
  ध्रुव वर्ण *zFuncName,
  शून्य (**pxFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
  शून्य **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  यदि( वाक्यं_तुलय(zFuncName,"glob")!=0 ){
    प्रत्यागम 0;
  }
  यदि( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    प्रत्यागम 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  प्रत्यागम 1;
}

स्थिर पूर्णाङ्क echoRename(sqlite3_vtab *vtab, ध्रुव वर्ण *zNewName){
  पूर्णाङ्क rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  यदि( simulateVtabError(p, "xRename") ){
    प्रत्यागम SQLITE_ERROR;
  }

  यदि( p->is_आदर्शः ){
    पूर्णाङ्क nThis = (पूर्णाङ्क)वाक्यदैर्घ्यम्(p->zThis);
    वर्ण *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->z_सारणी_Name, zNewName, &p->z_सारणी_Name[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क echoSavepoint(sqlite3_vtab *pVTab, पूर्णाङ्क iSavepoint){
  प्रतिज्ञा( pVTab );
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क echoRelease(sqlite3_vtab *pVTab, पूर्णाङ्क iSavepoint){
  प्रतिज्ञा( pVTab );
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क echo_Rollback_गन्तव्यम्(sqlite3_vtab *pVTab, पूर्णाङ्क iSavepoint){
  प्रतिज्ञा( pVTab );
  प्रत्यागम SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
स्थिर sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_Best_सूचकांक,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_अग्र,                  /* xNext - advance a cursor */
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

स्थिर sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_Best_सूचकांक,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_अग्र,                  /* xNext - advance a cursor */
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
  echo_Rollback_गन्तव्यम्,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

स्थिर शून्य moduleDestroy(शून्य *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI पञ्जिकाकोशः_echo_module(
  Client_दत्त client_दत्त, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णाङ्क rc;
  sqlite3 *db;
  EchoModule *pMod;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(परिमाण(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (शून्य*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  यदि( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(परिमाण(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (शून्य*)pMod, moduleDestroy
    );
  }

  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  प्रत्यागम TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI declare_vtab(
  Client_दत्त client_दत्त, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  पूर्णाङ्क rc;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB SQL");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3_errmsg(db), TCL_VOLATILE);
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}

#यदि_अन्तः /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest8_Init(Tcl_Interp *interp){
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *client_दत्त;
  } aObjCmd[] = {
     { "register_echo_module",       पञ्जिकाकोशः_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_दत्त, 0);
  }
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}
