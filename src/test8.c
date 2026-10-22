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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE

प्रकार काष्ठा echo_vtab echo_vtab;
प्रकार काष्ठा echo_cursor echo_cursor;

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
काष्ठा echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  पूर्णांक isPattern;
  पूर्णांक inTransaction;      /* True if within a transaction */
  अक्षर *zThis;            /* Name of the echo table */
  अक्षर *zTableName;       /* Name of the real table */
  अक्षर *zLogName;         /* Name of the log table */
  पूर्णांक nCol;               /* Number of columns in the real table */
  पूर्णांक *a_सूचकांक;            /* Array of size nCol. True if column has an index */
  अक्षर **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
काष्ठा echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

अटल पूर्णांक simulateVtabError(echo_vtab *p, स्थिर अक्षर *zMethod){
  स्थिर अक्षर *zErr;
  अक्षर zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  अगर( zErr ){
    p->base.z_Err_संदेश = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  वापस (zErr!=0);
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
अटल व्योम dequoteString(अक्षर *z){
  पूर्णांक quote;
  पूर्णांक i, j;
  अगर( z==0 ) वापस;
  quote = z[0];
  चयन( quote ){
    हाल '\'':  अवरोध;
    हाल '"':   अवरोध;
    हाल '`':   अवरोध;                /* For MySQL compatibility */
    हाल '[':   quote = ']';  अवरोध;  /* For MS SqlServer compatibility */
    शेष:    वापस;
  }
  क्रम(i=1, j=0; z[i]; i++){
    अगर( z[i]==quote ){
      अगर( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }अन्यथा{
        z[j++] = 0;
        अवरोध;
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
अटल पूर्णांक getColumnNames(
  sqlite3 *db, 
  स्थिर अक्षर *zTab,
  अक्षर ***paCol, 
  पूर्णांक *pnCol
){
  अक्षर **aCol = 0;
  अक्षर *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  अगर( !zSql ){
    rc = SQLITE_NOMEM;
    जाओ out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  अगर( rc==SQLITE_OK ){
    पूर्णांक ii;
    पूर्णांक nBytes;
    अक्षर *zSpace;
    nCol = sqlite3_column_गिनती(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = माप(अक्षर *) * nCol;
    क्रम(ii=0; ii<nCol; ii++){
      स्थिर अक्षर *zName = sqlite3_column_name(pStmt, ii);
      अगर( !zName ){
        rc = SQLITE_NOMEM;
        जाओ out;
      }
      nBytes += (पूर्णांक)म_माप(zName)+1;
    }
    aCol = (अक्षर **)sqlite3MallocZero(nBytes);
    अगर( !aCol ){
      rc = SQLITE_NOMEM;
      जाओ out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (अक्षर *)(&aCol[nCol]);
    क्रम(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (पूर्णांक)म_माप(zSpace) + 1;
    }
    निश्चित( (zSpace-nBytes)==(अक्षर *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  वापस rc;
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
अटल पूर्णांक get_सूचकांक_Array(
  sqlite3 *db,             /* Database connection */
  स्थिर अक्षर *zTab,        /* Name of table in database db */
  पूर्णांक nCol,
  पूर्णांक **pa_सूचकांक
){
  sqlite3_stmt *pStmt = 0;
  पूर्णांक *a_सूचकांक = 0;
  पूर्णांक rc;
  अक्षर *zSql;

  /* Allocate space for the index array */
  a_सूचकांक = (पूर्णांक *)sqlite3MallocZero(माप(पूर्णांक) * nCol);
  अगर( !a_सूचकांक ){
    rc = SQLITE_NOMEM;
    जाओ get_सूचकांक_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  अगर( !zSql ){
    rc = SQLITE_NOMEM;
    जाओ get_सूचकांक_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  जबतक( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    स्थिर अक्षर *zIdx = (स्थिर अक्षर *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    अगर( zIdx==0 ) जारी;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    अगर( !zSql ){
      rc = SQLITE_NOMEM;
      जाओ get_सूचकांक_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    अगर( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      पूर्णांक cid = sqlite3_column_int(pStmt2, 1);
      निश्चित( cid>=0 && cid<nCol );
      a_सूचकांक[cid] = 1;
    }
    अगर( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    अगर( rc!=SQLITE_OK ){
      जाओ get_सूचकांक_array_out;
    }
  }


get_सूचकांक_array_out:
  अगर( pStmt ){
    पूर्णांक rc2 = sqlite3_finalize(pStmt);
    अगर( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  अगर( rc!=SQLITE_OK ){
    sqlite3_free(a_सूचकांक);
    a_सूचकांक = 0;
  }
  *pa_सूचकांक = a_सूचकांक;
  वापस rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
अटल व्योम appendToEchoModule(Tcl_Interp *interp, स्थिर अक्षर *zArg){
  पूर्णांक flags = (TCL_APPEND_मूल्य | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
अटल पूर्णांक echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  पूर्णांक rc = SQLITE_OK;

  अगर( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    अगर( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      अगर( sqlite3_step(pStmt)==SQLITE_ROW ){
        पूर्णांक rc2;
        स्थिर अक्षर *zCreateTable = (स्थिर अक्षर *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        अगर( rc==SQLITE_OK ){
          rc = rc2;
        }
      } अन्यथा {
        rc = sqlite3_finalize(pStmt);
        अगर( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      अगर( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      अगर( rc==SQLITE_OK ){
        rc = get_सूचकांक_Array(db, pVtab->zTableName, pVtab->nCol, &pVtab->a_सूचकांक);
      }
    }
  }

  वापस rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
अटल पूर्णांक echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_सूचकांक);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  वापस 0;
}

प्रकार काष्ठा EchoModule EchoModule;
काष्ठा EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
अटल पूर्णांक echoConstructor(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  पूर्णांक rc;
  पूर्णांक i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( माप(*pVtab) );
  अगर( !pVtab ){
    वापस SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  अगर( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    वापस SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  अगर( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    अगर( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      अक्षर *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    अगर( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      वापस SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  क्रम(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  अगर( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    वापस rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  वापस SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
अटल पूर्णांक echoCreate(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  पूर्णांक rc = SQLITE_OK;
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
  अगर( rc==SQLITE_OK && argc==5 ){
    अक्षर *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    अगर( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  अगर( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  अगर( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  वापस rc;
}

/* 
** Echo virtual table module xConnect method.
*/
अटल पूर्णांक echoConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  वापस echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
अटल पूर्णांक echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  वापस echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
अटल पूर्णांक echoDestroy(sqlite3_vtab *pVtab){
  पूर्णांक rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  अगर( p && p->zLogName ){
    अक्षर *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  अगर( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  वापस rc;
}

/* 
** Echo virtual table module xOpen method.
*/
अटल पूर्णांक echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  अगर( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    वापस SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(माप(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  वापस (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
अटल पूर्णांक echoClose(sqlite3_vtab_cursor *cur){
  पूर्णांक rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  वापस rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
अटल पूर्णांक echoEof(sqlite3_vtab_cursor *cur){
  वापस (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
अटल पूर्णांक echo_अग्र(sqlite3_vtab_cursor *cur){
  पूर्णांक rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  अगर( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    वापस SQLITE_ERROR;
  }

  अगर( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    अगर( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }अन्यथा{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  वापस rc;
}

/* 
** Echo virtual table module xColumn method.
*/
अटल पूर्णांक echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  पूर्णांक iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  अगर( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    वापस SQLITE_ERROR;
  }

  अगर( !pStmt ){
    sqlite3_परिणाम_null(ctx);
  }अन्यथा{
    निश्चित( sqlite3_तथ्य_गिनती(pStmt)>iCol );
    sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pStmt, iCol));
  }
  वापस SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
अटल पूर्णांक echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  अगर( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    वापस SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  वापस SQLITE_OK;
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
अटल पूर्णांक hashString(स्थिर अक्षर *zString){
  u32 val = 0;
  पूर्णांक ii;
  क्रम(ii=0; zString[ii]; ii++){
    val = (val << 3) + (पूर्णांक)zString[ii];
  }
  वापस (पूर्णांक)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
अटल पूर्णांक echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  पूर्णांक rc;
  पूर्णांक i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  अगर( simulateVtabError(pVtab, "xFilter") ){
    वापस SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  निश्चित( idx_अंक==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  क्रम(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  निश्चित( pCur->pStmt || rc!=SQLITE_OK );
  क्रम(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_मूल्य(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  अगर( rc==SQLITE_OK ){
    rc = echo_अग्र(pVtabCursor);
  }

  वापस rc;
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
अटल व्योम string_concat(अक्षर **pzStr, अक्षर *zAppend, पूर्णांक doFree, पूर्णांक *pRc){
  अक्षर *zIn = *pzStr;
  अगर( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  अगर( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }अन्यथा{
    अगर( zIn ){
      अक्षर *z_अस्थायी = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(z_अस्थायी);
    }अन्यथा{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    अगर( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  अगर( doFree ){
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
अटल अक्षर *echoSelectList(echo_vtab *pTab, sqlite3_सूचकांक_info *pIdxInfo){
  अक्षर *zRet = 0;
  अगर( sqlite3_libversion_संख्या()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }अन्यथा{
    पूर्णांक i;
    क्रम(i=0; i<pTab->nCol; i++){
      अगर( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }अन्यथा{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      अगर( !zRet ) अवरोध;
    }
  }
  वापस zRet;
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
अटल पूर्णांक echo_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णांक ii;
  अक्षर *zQuery = 0;
  अक्षर *zCol = 0;
  अक्षर *zNew;
  पूर्णांक nArg = 0;
  स्थिर अक्षर *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  पूर्णांक nRow = 0;
  पूर्णांक useIdx = 0;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक useCost = 0;
  द्विगुन cost = 0;
  पूर्णांक isIgnoreUsable = 0;
  अगर( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  अगर( simulateVtabError(pVtab, "xBestIndex") ){
    वापस SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  अगर( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = म_से_भ(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } अन्यथा {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    अगर( !zQuery ){
      वापस SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    अगर( rc!=SQLITE_OK ){
      वापस rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    अगर( rc!=SQLITE_OK ){
      वापस rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  अगर( !zCol ) वापस SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  अगर( !zQuery ) वापस SQLITE_NOMEM;

  क्रम(ii=0; ii<pIdxInfo->nConstraint; ii++){
    स्थिर काष्ठा sqlite3_सूचकांक_constraint *pConstraint;
    काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage;
    पूर्णांक iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    अगर( !isIgnoreUsable && !pConstraint->usable ) जारी;

    iCol = pConstraint->iColumn;
    अगर( iCol<0 || pVtab->a_सूचकांक[iCol] ){
      अक्षर *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      अक्षर *zOp = 0;
      useIdx = 1;
      चयन( pConstraint->op ){
        हाल SQLITE_सूचकांक_CONSTRAINT_EQ:
          zOp = "="; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_LT:
          zOp = "<"; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_GT:
          zOp = ">"; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_LE:
          zOp = "<="; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_GE:
          zOp = ">="; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_LIKE:
          zOp = "like"; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_GLOB:
          zOp = "glob"; अवरोध;
        हाल SQLITE_सूचकांक_CONSTRAINT_REGEXP:
          zOp = "regexp"; अवरोध;
      }
      अगर( zOp ){
        अगर( zOp[0]=='L' ){
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
  अगर( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->a_सूचकांक[pIdxInfo->aOrderBy->iColumn]) ){
    पूर्णांक iCol = pIdxInfo->aOrderBy->iColumn;
    अक्षर *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    अक्षर *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  अगर( !zQuery ){
    वापस rc;
  }
  pIdxInfo->idx_अंक = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  अगर( useCost ){
    pIdxInfo->estimatedCost = cost;
  }अन्यथा अगर( useIdx ){
    /* Approximation of log2(nRow). */
    क्रम( ii=0; ii<(माप(पूर्णांक)*8)-1; ii++ ){
      अगर( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (द्विगुन)ii;
      }
    }
  }अन्यथा{
    pIdxInfo->estimatedCost = (द्विगुन)nRow;
  }
  वापस rc;
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
पूर्णांक echoUpdate(
  sqlite3_vtab *tab, 
  पूर्णांक n_तथ्य, 
  sqlite3_मूल्य **ap_तथ्य, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  पूर्णांक rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  अक्षर *z = 0;               /* SQL statement to execute */
  पूर्णांक bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  पूर्णांक bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  पूर्णांक i;                     /* Counter variable used by for loops */

  निश्चित( n_तथ्य==pVtab->nCol+2 || n_तथ्य==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  निश्चित( pVtab->inTransaction );

  अगर( simulateVtabError(pVtab, "xUpdate") ){
    वापस SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  अगर( n_तथ्य>1 && sqlite3_मूल्य_type(ap_तथ्य[0])==SQLITE_INTEGER ){
    अक्षर *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    अगर( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_तथ्य[1] && sqlite3_मूल्य_type(ap_तथ्य[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    अगर( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    क्रम(i=2; i<n_तथ्य; i++){
      अगर( ap_तथ्य[i]==0 ) जारी;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_तथ्य), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  अन्यथा अगर( n_तथ्य==1 && sqlite3_मूल्य_type(ap_तथ्य[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    अगर( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  अन्यथा अगर( n_तथ्य>2 && sqlite3_मूल्य_type(ap_तथ्य[0])==SQLITE_NULL ){
    पूर्णांक ii;
    अक्षर *z_जोड़ो = 0;
    अक्षर *z_मूल्यावली = 0;
  
    z_जोड़ो = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    अगर( !z_जोड़ो ){
      rc = SQLITE_NOMEM;
    }
    अगर( sqlite3_मूल्य_type(ap_तथ्य[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_मूल्यावली = sqlite3_mprintf("?");
      string_concat(&z_जोड़ो, "rowid", 0, &rc);
    }

    निश्चित((pVtab->nCol+2)==n_तथ्य);
    क्रम(ii=2; ii<n_तथ्य; ii++){
      string_concat(&z_जोड़ो, 
          sqlite3_mprintf("%s%Q", z_मूल्यावली?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_मूल्यावली, 
          sqlite3_mprintf("%s?%d", z_मूल्यावली?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_जोड़ो, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_मूल्यावली, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  अन्यथा{
    निश्चित(0);
    वापस SQLITE_ERROR;
  }

  अगर( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  निश्चित( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  अगर( rc==SQLITE_OK ) {
    अगर( bindArgZero ){
      sqlite3_bind_मूल्य(pStmt, n_तथ्य, ap_तथ्य[0]);
    }
    अगर( bindArgOne ){
      sqlite3_bind_मूल्य(pStmt, 1, ap_तथ्य[1]);
    }
    क्रम(i=2; i<n_तथ्य && rc==SQLITE_OK; i++){
      अगर( ap_तथ्य[i] ) rc = sqlite3_bind_मूल्य(pStmt, i, ap_तथ्य[i]);
    }
    अगर( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }अन्यथा{
      sqlite3_finalize(pStmt);
    }
  }

  अगर( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_जोड़ो_rowid(db);
  }
  अगर( rc!=SQLITE_OK ){
    tab->z_Err_संदेश = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  वापस rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
अटल पूर्णांक echoTransactionCall(sqlite3_vtab *tab, स्थिर अक्षर *zCall){
  अक्षर *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  अगर( z==0 ) वापस SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  वापस SQLITE_OK;
}
अटल पूर्णांक echoBegin(sqlite3_vtab *tab){
  पूर्णांक rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  स्थिर अक्षर *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  निश्चित( !pVtab->inTransaction );

  अगर( simulateVtabError(pVtab, "xBegin") ){
    वापस SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  अगर( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    अगर( zVal && 0==म_भेद(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  अगर( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  वापस rc;
}
अटल पूर्णांक echoSync(sqlite3_vtab *tab){
  पूर्णांक rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  स्थिर अक्षर *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  निश्चित( pVtab->inTransaction );

  अगर( simulateVtabError(pVtab, "xSync") ){
    वापस SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  अगर( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    अगर( zVal && 0==म_भेद(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  वापस rc;
}
अटल पूर्णांक echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  पूर्णांक rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  निश्चित( pVtab->inTransaction );

  अगर( simulateVtabError(pVtab, "xCommit") ){
    वापस SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  वापस rc;
}
अटल पूर्णांक echoRollback(sqlite3_vtab *tab){
  पूर्णांक rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  निश्चित( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  वापस rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
अटल व्योम overloadedGlobFunction(
  sqlite3_context *pContext,
  पूर्णांक nArg,
  sqlite3_मूल्य **apArg
){
  Tcl_Interp *interp = sqlite3_user_तथ्य(pContext);
  Tcl_DString str;
  पूर्णांक i;
  पूर्णांक rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  क्रम(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (अक्षर*)sqlite3_मूल्य_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_मूल्य(&str));
  Tcl_DStringFree(&str);
  अगर( rc ){
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
अटल पूर्णांक echoFindFunction(
  sqlite3_vtab *vtab,
  पूर्णांक nArg,
  स्थिर अक्षर *zFuncName,
  व्योम (**pxFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
  व्योम **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  अगर( म_भेद(zFuncName,"glob")!=0 ){
    वापस 0;
  }
  अगर( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    वापस 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  वापस 1;
}

अटल पूर्णांक echoRename(sqlite3_vtab *vtab, स्थिर अक्षर *zNewName){
  पूर्णांक rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  अगर( simulateVtabError(p, "xRename") ){
    वापस SQLITE_ERROR;
  }

  अगर( p->isPattern ){
    पूर्णांक nThis = (पूर्णांक)म_माप(p->zThis);
    अक्षर *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  वापस rc;
}

अटल पूर्णांक echoSavepoint(sqlite3_vtab *pVTab, पूर्णांक iSavepoint){
  निश्चित( pVTab );
  वापस SQLITE_OK;
}

अटल पूर्णांक echoRelease(sqlite3_vtab *pVTab, पूर्णांक iSavepoint){
  निश्चित( pVTab );
  वापस SQLITE_OK;
}

अटल पूर्णांक echoRollbackTo(sqlite3_vtab *pVTab, पूर्णांक iSavepoint){
  निश्चित( pVTab );
  वापस SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
अटल sqlite3_module echoModule = {
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

अटल sqlite3_module echoModuleV2 = {
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
  echoRollbackTo,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

अटल व्योम moduleDestroy(व्योम *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
अटल पूर्णांक SQLITE_TCLAPI register_echo_module(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णांक rc;
  sqlite3 *db;
  EchoModule *pMod;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(माप(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (व्योम*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  अगर( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(माप(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (व्योम*)pMod, moduleDestroy
    );
  }

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
  वापस TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
अटल पूर्णांक SQLITE_TCLAPI declare_vtab(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  पूर्णांक rc;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB SQL");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  अगर( rc!=SQLITE_OK ){
    Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3_errmsg(db), TCL_VOLATILE);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

#पूर्ण_अगर /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest8_Init(Tcl_Interp *interp){
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     व्योम *client_तथ्य;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_तथ्य, 0);
  }
#पूर्ण_अगर
  वापस TCL_OK;
}
