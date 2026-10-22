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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

પ્રકારવ્યાખ્યા સંરચના echo_vtab echo_vtab;
પ્રકારવ્યાખ્યા સંરચના echo_cursor echo_cursor;

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
સંરચના echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  પૂર્ણાંક isPattern;
  પૂર્ણાંક inTransaction;      /* True if within a transaction */
  અક્ષર *zThis;            /* Name of the echo table */
  અક્ષર *zTableName;       /* Name of the real table */
  અક્ષર *zLogName;         /* Name of the log table */
  પૂર્ણાંક nCol;               /* Number of columns in the real table */
  પૂર્ણાંક *aIndex;            /* Array of size nCol. True if column has an index */
  અક્ષર **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
સંરચના echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

સ્થિર પૂર્ણાંક simulateVtabError(echo_vtab *p, અચલ અક્ષર *zMethod){
  અચલ અક્ષર *zErr;
  અક્ષર zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  જો( zErr ){
    p->base.z_Err_સંદેશ = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  પાછા (zErr!=0);
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
સ્થિર શૂન્ય dequoteString(અક્ષર *z){
  પૂર્ણાંક quote;
  પૂર્ણાંક i, j;
  જો( z==0 ) પાછા;
  quote = z[0];
  પસંદગી( quote ){
    સ્થિતિ '\'':  તોડો;
    સ્થિતિ '"':   તોડો;
    સ્થિતિ '`':   તોડો;                /* For MySQL compatibility */
    સ્થિતિ '[':   quote = ']';  તોડો;  /* For MS SqlServer compatibility */
    મૂળભૂત:    પાછા;
  }
  માટે(i=1, j=0; z[i]; i++){
    જો( z[i]==quote ){
      જો( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }નહીંતર{
        z[j++] = 0;
        તોડો;
      }
    }નહીંતર{
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
સ્થિર પૂર્ણાંક getColumnNames(
  sqlite3 *db, 
  અચલ અક્ષર *zTab,
  અક્ષર ***paCol, 
  પૂર્ણાંક *pnCol
){
  અક્ષર **aCol = 0;
  અક્ષર *zSql;
  sqlite3_stmt *pStmt = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  જો( !zSql ){
    rc = SQLITE_NOMEM;
    જાઓ out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  જો( rc==SQLITE_OK ){
    પૂર્ણાંક ii;
    પૂર્ણાંક nBytes;
    અક્ષર *zSpace;
    nCol = sqlite3_column_ગણતરી(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = કદ(અક્ષર *) * nCol;
    માટે(ii=0; ii<nCol; ii++){
      અચલ અક્ષર *zName = sqlite3_column_name(pStmt, ii);
      જો( !zName ){
        rc = SQLITE_NOMEM;
        જાઓ out;
      }
      nBytes += (પૂર્ણાંક)strlen(zName)+1;
    }
    aCol = (અક્ષર **)sqlite3MallocZero(nBytes);
    જો( !aCol ){
      rc = SQLITE_NOMEM;
      જાઓ out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (અક્ષર *)(&aCol[nCol]);
    માટે(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (પૂર્ણાંક)strlen(zSpace) + 1;
    }
    પ્રતિજ્ઞા( (zSpace-nBytes)==(અક્ષર *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  પાછા rc;
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
સ્થિર પૂર્ણાંક getIndexArray(
  sqlite3 *db,             /* Database connection */
  અચલ અક્ષર *zTab,        /* Name of table in database db */
  પૂર્ણાંક nCol,
  પૂર્ણાંક **paIndex
){
  sqlite3_stmt *pStmt = 0;
  પૂર્ણાંક *aIndex = 0;
  પૂર્ણાંક rc;
  અક્ષર *zSql;

  /* Allocate space for the index array */
  aIndex = (પૂર્ણાંક *)sqlite3MallocZero(કદ(પૂર્ણાંક) * nCol);
  જો( !aIndex ){
    rc = SQLITE_NOMEM;
    જાઓ get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  જો( !zSql ){
    rc = SQLITE_NOMEM;
    જાઓ get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  જ્યારે( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    અચલ અક્ષર *zIdx = (અચલ અક્ષર *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    જો( zIdx==0 ) ચાલુ;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    જો( !zSql ){
      rc = SQLITE_NOMEM;
      જાઓ get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    જો( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      પૂર્ણાંક cid = sqlite3_column_int(pStmt2, 1);
      પ્રતિજ્ઞા( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    જો( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    જો( rc!=SQLITE_OK ){
      જાઓ get_index_array_out;
    }
  }


get_index_array_out:
  જો( pStmt ){
    પૂર્ણાંક rc2 = sqlite3_finalize(pStmt);
    જો( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  જો( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  પાછા rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
સ્થિર શૂન્ય appendToEchoModule(Tcl_Interp *interp, અચલ અક્ષર *zArg){
  પૂર્ણાંક flags = (TCL_APPEND_કિંમત | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
સ્થિર પૂર્ણાંક echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  પૂર્ણાંક rc = SQLITE_OK;

  જો( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    જો( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      જો( sqlite3_step(pStmt)==SQLITE_ROW ){
        પૂર્ણાંક rc2;
        અચલ અક્ષર *zCreateTable = (અચલ અક્ષર *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        જો( rc==SQLITE_OK ){
          rc = rc2;
        }
      } નહીંતર {
        rc = sqlite3_finalize(pStmt);
        જો( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      જો( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      જો( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  પાછા rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
સ્થિર પૂર્ણાંક echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  પાછા 0;
}

પ્રકારવ્યાખ્યા સંરચના EchoModule EchoModule;
સંરચના EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
સ્થિર પૂર્ણાંક echoConstructor(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  પૂર્ણાંક rc;
  પૂર્ણાંક i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( કદ(*pVtab) );
  જો( !pVtab ){
    પાછા SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  જો( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    પાછા SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  જો( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    જો( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      અક્ષર *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    જો( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      પાછા SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  માટે(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  જો( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    પાછા rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  પાછા SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
સ્થિર પૂર્ણાંક echoCreate(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  પૂર્ણાંક rc = SQLITE_OK;
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
  જો( rc==SQLITE_OK && argc==5 ){
    અક્ષર *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    જો( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  જો( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  જો( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  પાછા rc;
}

/* 
** Echo virtual table module xConnect method.
*/
સ્થિર પૂર્ણાંક echoConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  પાછા echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
સ્થિર પૂર્ણાંક echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  પાછા echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
સ્થિર પૂર્ણાંક echoDestroy(sqlite3_vtab *pVtab){
  પૂર્ણાંક rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  જો( p && p->zLogName ){
    અક્ષર *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  જો( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  પાછા rc;
}

/* 
** Echo virtual table module xOpen method.
*/
સ્થિર પૂર્ણાંક echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  જો( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    પાછા SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(કદ(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  પાછા (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
સ્થિર પૂર્ણાંક echoClose(sqlite3_vtab_cursor *cur){
  પૂર્ણાંક rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  પાછા rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
સ્થિર પૂર્ણાંક echoEof(sqlite3_vtab_cursor *cur){
  પાછા (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
સ્થિર પૂર્ણાંક echoNext(sqlite3_vtab_cursor *cur){
  પૂર્ણાંક rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  જો( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    પાછા SQLITE_ERROR;
  }

  જો( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    જો( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }નહીંતર{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  પાછા rc;
}

/* 
** Echo virtual table module xColumn method.
*/
સ્થિર પૂર્ણાંક echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  પૂર્ણાંક iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  જો( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    પાછા SQLITE_ERROR;
  }

  જો( !pStmt ){
    sqlite3_result_null(ctx);
  }નહીંતર{
    પ્રતિજ્ઞા( sqlite3_data_ગણતરી(pStmt)>iCol );
    sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pStmt, iCol));
  }
  પાછા SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
સ્થિર પૂર્ણાંક echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  જો( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    પાછા SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક hashString(અચલ અક્ષર *zString){
  u32 val = 0;
  પૂર્ણાંક ii;
  માટે(ii=0; zString[ii]; ii++){
    val = (val << 3) + (પૂર્ણાંક)zString[ii];
  }
  પાછા (પૂર્ણાંક)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
સ્થિર પૂર્ણાંક echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  પૂર્ણાંક rc;
  પૂર્ણાંક i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  જો( simulateVtabError(pVtab, "xFilter") ){
    પાછા SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  પ્રતિજ્ઞા( idx_અંક==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  માટે(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  પ્રતિજ્ઞા( pCur->pStmt || rc!=SQLITE_OK );
  માટે(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_કિંમત(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  જો( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  પાછા rc;
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
સ્થિર શૂન્ય string_concat(અક્ષર **pzStr, અક્ષર *zAppend, પૂર્ણાંક doFree, પૂર્ણાંક *pRc){
  અક્ષર *zIn = *pzStr;
  જો( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  જો( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }નહીંતર{
    જો( zIn ){
      અક્ષર *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }નહીંતર{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    જો( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  જો( doFree ){
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
સ્થિર અક્ષર *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  અક્ષર *zRet = 0;
  જો( sqlite3_libversion_સંખ્યા()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }નહીંતર{
    પૂર્ણાંક i;
    માટે(i=0; i<pTab->nCol; i++){
      જો( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }નહીંતર{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      જો( !zRet ) તોડો;
    }
  }
  પાછા zRet;
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
સ્થિર પૂર્ણાંક echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પૂર્ણાંક ii;
  અક્ષર *zQuery = 0;
  અક્ષર *zCol = 0;
  અક્ષર *zNew;
  પૂર્ણાંક nArg = 0;
  અચલ અક્ષર *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  પૂર્ણાંક nRow = 0;
  પૂર્ણાંક useIdx = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક useCost = 0;
  દ્વિગુણ cost = 0;
  પૂર્ણાંક isIgnoreUsable = 0;
  જો( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  જો( simulateVtabError(pVtab, "xBestIndex") ){
    પાછા SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  જો( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } નહીંતર {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    જો( !zQuery ){
      પાછા SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    જો( rc!=SQLITE_OK ){
      પાછા rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    જો( rc!=SQLITE_OK ){
      પાછા rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  જો( !zCol ) પાછા SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  જો( !zQuery ) પાછા SQLITE_NOMEM;

  માટે(ii=0; ii<pIdxInfo->nConstraint; ii++){
    અચલ સંરચના sqlite3_index_constraint *pConstraint;
    સંરચના sqlite3_index_constraint_usage *pUsage;
    પૂર્ણાંક iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    જો( !isIgnoreUsable && !pConstraint->usable ) ચાલુ;

    iCol = pConstraint->iColumn;
    જો( iCol<0 || pVtab->aIndex[iCol] ){
      અક્ષર *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      અક્ષર *zOp = 0;
      useIdx = 1;
      પસંદગી( pConstraint->op ){
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; તોડો;
        સ્થિતિ SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; તોડો;
      }
      જો( zOp ){
        જો( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } નહીંતર {
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
  જો( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    પૂર્ણાંક iCol = pIdxInfo->aOrderBy->iColumn;
    અક્ષર *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    અક્ષર *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  જો( !zQuery ){
    પાછા rc;
  }
  pIdxInfo->idx_અંક = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  જો( useCost ){
    pIdxInfo->estimatedCost = cost;
  }નહીંતર જો( useIdx ){
    /* Approximation of log2(nRow). */
    માટે( ii=0; ii<(કદ(પૂર્ણાંક)*8)-1; ii++ ){
      જો( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (દ્વિગુણ)ii;
      }
    }
  }નહીંતર{
    pIdxInfo->estimatedCost = (દ્વિગુણ)nRow;
  }
  પાછા rc;
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
પૂર્ણાંક echoUpdate(
  sqlite3_vtab *tab, 
  પૂર્ણાંક nData, 
  sqlite3_કિંમત **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  પૂર્ણાંક rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  અક્ષર *z = 0;               /* SQL statement to execute */
  પૂર્ણાંક bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  પૂર્ણાંક bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  પૂર્ણાંક i;                     /* Counter variable used by for loops */

  પ્રતિજ્ઞા( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  પ્રતિજ્ઞા( pVtab->inTransaction );

  જો( simulateVtabError(pVtab, "xUpdate") ){
    પાછા SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  જો( nData>1 && sqlite3_કિંમત_type(apData[0])==SQLITE_INTEGER ){
    અક્ષર *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    જો( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_કિંમત_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    જો( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    માટે(i=2; i<nData; i++){
      જો( apData[i]==0 ) ચાલુ;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  નહીંતર જો( nData==1 && sqlite3_કિંમત_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    જો( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  નહીંતર જો( nData>2 && sqlite3_કિંમત_type(apData[0])==SQLITE_NULL ){
    પૂર્ણાંક ii;
    અક્ષર *z_ઉમેરો = 0;
    અક્ષર *z_કિંમતો = 0;
  
    z_ઉમેરો = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    જો( !z_ઉમેરો ){
      rc = SQLITE_NOMEM;
    }
    જો( sqlite3_કિંમત_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_કિંમતો = sqlite3_mprintf("?");
      string_concat(&z_ઉમેરો, "rowid", 0, &rc);
    }

    પ્રતિજ્ઞા((pVtab->nCol+2)==nData);
    માટે(ii=2; ii<nData; ii++){
      string_concat(&z_ઉમેરો, 
          sqlite3_mprintf("%s%Q", z_કિંમતો?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_કિંમતો, 
          sqlite3_mprintf("%s?%d", z_કિંમતો?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_ઉમેરો, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_કિંમતો, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  નહીંતર{
    પ્રતિજ્ઞા(0);
    પાછા SQLITE_ERROR;
  }

  જો( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  પ્રતિજ્ઞા( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  જો( rc==SQLITE_OK ) {
    જો( bindArgZero ){
      sqlite3_bind_કિંમત(pStmt, nData, apData[0]);
    }
    જો( bindArgOne ){
      sqlite3_bind_કિંમત(pStmt, 1, apData[1]);
    }
    માટે(i=2; i<nData && rc==SQLITE_OK; i++){
      જો( apData[i] ) rc = sqlite3_bind_કિંમત(pStmt, i, apData[i]);
    }
    જો( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }નહીંતર{
      sqlite3_finalize(pStmt);
    }
  }

  જો( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_ઉમેરો_rowid(db);
  }
  જો( rc!=SQLITE_OK ){
    tab->z_Err_સંદેશ = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  પાછા rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
સ્થિર પૂર્ણાંક echoTransactionCall(sqlite3_vtab *tab, અચલ અક્ષર *zCall){
  અક્ષર *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  જો( z==0 ) પાછા SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  પાછા SQLITE_OK;
}
સ્થિર પૂર્ણાંક echoBegin(sqlite3_vtab *tab){
  પૂર્ણાંક rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  અચલ અક્ષર *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  પ્રતિજ્ઞા( !pVtab->inTransaction );

  જો( simulateVtabError(pVtab, "xBegin") ){
    પાછા SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  જો( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    જો( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  જો( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  પાછા rc;
}
સ્થિર પૂર્ણાંક echoSync(sqlite3_vtab *tab){
  પૂર્ણાંક rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  અચલ અક્ષર *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  પ્રતિજ્ઞા( pVtab->inTransaction );

  જો( simulateVtabError(pVtab, "xSync") ){
    પાછા SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  જો( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    જો( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  પાછા rc;
}
સ્થિર પૂર્ણાંક echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  પૂર્ણાંક rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  પ્રતિજ્ઞા( pVtab->inTransaction );

  જો( simulateVtabError(pVtab, "xCommit") ){
    પાછા SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  પાછા rc;
}
સ્થિર પૂર્ણાંક echoRollback(sqlite3_vtab *tab){
  પૂર્ણાંક rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  પ્રતિજ્ઞા( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  પાછા rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
સ્થિર શૂન્ય overloadedGlobFunction(
  sqlite3_context *pContext,
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  પૂર્ણાંક i;
  પૂર્ણાંક rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  માટે(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (અક્ષર*)sqlite3_કિંમત_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_કિંમત(&str));
  Tcl_DStringFree(&str);
  જો( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }નહીંતર{
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
સ્થિર પૂર્ણાંક echoFindFunction(
  sqlite3_vtab *vtab,
  પૂર્ણાંક nArg,
  અચલ અક્ષર *zFuncName,
  શૂન્ય (**pxFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
  શૂન્ય **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  જો( strcmp(zFuncName,"glob")!=0 ){
    પાછા 0;
  }
  જો( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    પાછા 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  પાછા 1;
}

સ્થિર પૂર્ણાંક echoRename(sqlite3_vtab *vtab, અચલ અક્ષર *zNewName){
  પૂર્ણાંક rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  જો( simulateVtabError(p, "xRename") ){
    પાછા SQLITE_ERROR;
  }

  જો( p->isPattern ){
    પૂર્ણાંક nThis = (પૂર્ણાંક)strlen(p->zThis);
    અક્ષર *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  પાછા rc;
}

સ્થિર પૂર્ણાંક echoSavepoint(sqlite3_vtab *pVTab, પૂર્ણાંક iSavepoint){
  પ્રતિજ્ઞા( pVTab );
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક echoRelease(sqlite3_vtab *pVTab, પૂર્ણાંક iSavepoint){
  પ્રતિજ્ઞા( pVTab );
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક echoRollbackTo(sqlite3_vtab *pVTab, પૂર્ણાંક iSavepoint){
  પ્રતિજ્ઞા( pVTab );
  પાછા SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
સ્થિર sqlite3_module echoModule = {
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

સ્થિર sqlite3_module echoModuleV2 = {
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
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

સ્થિર શૂન્ય moduleDestroy(શૂન્ય *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  પૂર્ણાંક rc;
  sqlite3 *db;
  EchoModule *pMod;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(કદ(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (શૂન્ય*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  જો( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(કદ(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (શૂન્ય*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);
  પાછા TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  પૂર્ણાંક rc;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB SQL");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  જો( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (અક્ષર *)sqlite3_errmsg(db), TCL_VOLATILE);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  પાછા TCL_OK;
}
