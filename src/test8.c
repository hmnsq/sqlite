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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

வகைவரை கட்டமைப்பு echo_vtab echo_vtab;
வகைவரை கட்டமைப்பு echo_cursor echo_cursor;

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
கட்டமைப்பு echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  முழு isPattern;
  முழு inTransaction;      /* True if within a transaction */
  எழுத்து *zThis;            /* Name of the echo table */
  எழுத்து *zTableName;       /* Name of the real table */
  எழுத்து *zLogName;         /* Name of the log table */
  முழு nCol;               /* Number of columns in the real table */
  முழு *aIndex;            /* Array of size nCol. True if column has an index */
  எழுத்து **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
கட்டமைப்பு echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

நிலையான முழு simulateVtabError(echo_vtab *p, மாறிலி எழுத்து *zMethod){
  மாறிலி எழுத்து *zErr;
  எழுத்து zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", zMethod, p->zTableName);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  எனில்( zErr ){
    p->base.z_Err_செய்தி = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  திரும்பு (zErr!=0);
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
நிலையான வெறுமை dequoteString(எழுத்து *z){
  முழு quote;
  முழு i, j;
  எனில்( z==0 ) திரும்பு;
  quote = z[0];
  தேர்வு( quote ){
    நிலை '\'':  நிறுத்து;
    நிலை '"':   நிறுத்து;
    நிலை '`':   நிறுத்து;                /* For MySQL compatibility */
    நிலை '[':   quote = ']';  நிறுத்து;  /* For MS SqlServer compatibility */
    இயல்பு:    திரும்பு;
  }
  க்கு(i=1, j=0; z[i]; i++){
    எனில்( z[i]==quote ){
      எனில்( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }இல்லையெனில்{
        z[j++] = 0;
        நிறுத்து;
      }
    }இல்லையெனில்{
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
நிலையான முழு getColumnNames(
  sqlite3 *db, 
  மாறிலி எழுத்து *zTab,
  எழுத்து ***paCol, 
  முழு *pnCol
){
  எழுத்து **aCol = 0;
  எழுத்து *zSql;
  sqlite3_stmt *pStmt = 0;
  முழு rc = SQLITE_OK;
  முழு nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  எனில்( !zSql ){
    rc = SQLITE_NOMEM;
    செல் out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  எனில்( rc==SQLITE_OK ){
    முழு ii;
    முழு nBytes;
    எழுத்து *zSpace;
    nCol = sqlite3_column_மொத்தம்(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = அளவு(எழுத்து *) * nCol;
    க்கு(ii=0; ii<nCol; ii++){
      மாறிலி எழுத்து *zName = sqlite3_column_name(pStmt, ii);
      எனில்( !zName ){
        rc = SQLITE_NOMEM;
        செல் out;
      }
      nBytes += (முழு)strlen(zName)+1;
    }
    aCol = (எழுத்து **)sqlite3MallocZero(nBytes);
    எனில்( !aCol ){
      rc = SQLITE_NOMEM;
      செல் out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (எழுத்து *)(&aCol[nCol]);
    க்கு(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      sqlite3_snprintf(nBytes, zSpace, "%s", sqlite3_column_name(pStmt,ii));
      zSpace += (முழு)strlen(zSpace) + 1;
    }
    ப்ரதிஜ்ஞா( (zSpace-nBytes)==(எழுத்து *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  திரும்பு rc;
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
நிலையான முழு getIndexArray(
  sqlite3 *db,             /* Database connection */
  மாறிலி எழுத்து *zTab,        /* Name of table in database db */
  முழு nCol,
  முழு **paIndex
){
  sqlite3_stmt *pStmt = 0;
  முழு *aIndex = 0;
  முழு rc;
  எழுத்து *zSql;

  /* Allocate space for the index array */
  aIndex = (முழு *)sqlite3MallocZero(அளவு(முழு) * nCol);
  எனில்( !aIndex ){
    rc = SQLITE_NOMEM;
    செல் get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  எனில்( !zSql ){
    rc = SQLITE_NOMEM;
    செல் get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  போது( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    மாறிலி எழுத்து *zIdx = (மாறிலி எழுத்து *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    எனில்( zIdx==0 ) தொடர்;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    எனில்( !zSql ){
      rc = SQLITE_NOMEM;
      செல் get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    எனில்( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      முழு cid = sqlite3_column_int(pStmt2, 1);
      ப்ரதிஜ்ஞா( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    எனில்( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    எனில்( rc!=SQLITE_OK ){
      செல் get_index_array_out;
    }
  }


get_index_array_out:
  எனில்( pStmt ){
    முழு rc2 = sqlite3_finalize(pStmt);
    எனில்( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  எனில்( rc!=SQLITE_OK ){
    sqlite3_free(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  திரும்பு rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
நிலையான வெறுமை appendToEchoModule(Tcl_Interp *interp, மாறிலி எழுத்து *zArg){
  முழு flags = (TCL_APPEND_மதிப்பு | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
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
நிலையான முழு echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  முழு rc = SQLITE_OK;

  எனில்( pVtab->zTableName ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    எனில்( rc==SQLITE_OK ){
      sqlite3_bind_text(pStmt, 1, pVtab->zTableName, -1, 0);
      எனில்( sqlite3_step(pStmt)==SQLITE_ROW ){
        முழு rc2;
        மாறிலி எழுத்து *zCreateTable = (மாறிலி எழுத்து *)sqlite3_column_text(pStmt, 0);
        rc = sqlite3_declare_vtab(db, zCreateTable);
        rc2 = sqlite3_finalize(pStmt);
        எனில்( rc==SQLITE_OK ){
          rc = rc2;
        }
      } இல்லையெனில் {
        rc = sqlite3_finalize(pStmt);
        எனில்( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      எனில்( rc==SQLITE_OK ){
        rc = getColumnNames(db, pVtab->zTableName, &pVtab->aCol, &pVtab->nCol);
      }
      எனில்( rc==SQLITE_OK ){
        rc = getIndexArray(db, pVtab->zTableName, pVtab->nCol, &pVtab->aIndex);
      }
    }
  }

  திரும்பு rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
நிலையான முழு echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->aIndex);
  sqlite3_free(p->aCol);
  sqlite3_free(p->zThis);
  sqlite3_free(p->zTableName);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  திரும்பு 0;
}

வகைவரை கட்டமைப்பு EchoModule EchoModule;
கட்டமைப்பு EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
நிலையான முழு echoConstructor(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  முழு rc;
  முழு i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( அளவு(*pVtab) );
  எனில்( !pVtab ){
    திரும்பு SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  எனில்( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    திரும்பு SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  எனில்( argc>3 ){
    pVtab->zTableName = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->zTableName);
    எனில்( pVtab->zTableName && pVtab->zTableName[0]=='*' ){
      எழுத்து *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->zTableName[1]));
      sqlite3_free(pVtab->zTableName);
      pVtab->zTableName = z;
      pVtab->isPattern = 1;
    }
    எனில்( !pVtab->zTableName ){
      echoDestructor((sqlite3_vtab *)pVtab);
      திரும்பு SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  க்கு(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  எனில்( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    திரும்பு rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  திரும்பு SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
நிலையான முழு echoCreate(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  முழு rc = SQLITE_OK;
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
  எனில்( rc==SQLITE_OK && argc==5 ){
    எழுத்து *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    எனில்( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  எனில்( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  எனில்( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->inTransaction = 1;
  }

  திரும்பு rc;
}

/* 
** Echo virtual table module xConnect method.
*/
நிலையான முழு echoConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  appendToEchoModule(((EchoModule *)pAux)->interp, "xConnect");
  திரும்பு echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
நிலையான முழு echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  திரும்பு echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
நிலையான முழு echoDestroy(sqlite3_vtab *pVtab){
  முழு rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  எனில்( p && p->zLogName ){
    எழுத்து *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  எனில்( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  திரும்பு rc;
}

/* 
** Echo virtual table module xOpen method.
*/
நிலையான முழு echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  எனில்( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    திரும்பு SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(அளவு(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  திரும்பு (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
நிலையான முழு echoClose(sqlite3_vtab_cursor *cur){
  முழு rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  திரும்பு rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
நிலையான முழு echoEof(sqlite3_vtab_cursor *cur){
  திரும்பு (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
நிலையான முழு echoNext(sqlite3_vtab_cursor *cur){
  முழு rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  எனில்( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    திரும்பு SQLITE_ERROR;
  }

  எனில்( pCur->pStmt ){
    rc = sqlite3_step(pCur->pStmt);
    எனில்( rc==SQLITE_ROW ){
      rc = SQLITE_OK;
    }இல்லையெனில்{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  திரும்பு rc;
}

/* 
** Echo virtual table module xColumn method.
*/
நிலையான முழு echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, முழு i){
  முழு iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  எனில்( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    திரும்பு SQLITE_ERROR;
  }

  எனில்( !pStmt ){
    sqlite3_result_null(ctx);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( sqlite3_data_மொத்தம்(pStmt)>iCol );
    sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pStmt, iCol));
  }
  திரும்பு SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
நிலையான முழு echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  எனில்( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    திரும்பு SQLITE_ERROR;
  }

  *pRowid = sqlite3_column_int64(pStmt, 0);
  திரும்பு SQLITE_OK;
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
நிலையான முழு hashString(மாறிலி எழுத்து *zString){
  u32 val = 0;
  முழு ii;
  க்கு(ii=0; zString[ii]; ii++){
    val = (val << 3) + (முழு)zString[ii];
  }
  திரும்பு (முழு)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
நிலையான முழு echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  முழு rc;
  முழு i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  எனில்( simulateVtabError(pVtab, "xFilter") ){
    திரும்பு SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  ப்ரதிஜ்ஞா( idx_இலக்கம்==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  க்கு(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  ப்ரதிஜ்ஞா( pCur->pStmt || rc!=SQLITE_OK );
  க்கு(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_மதிப்பு(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  எனில்( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  திரும்பு rc;
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
நிலையான வெறுமை string_concat(எழுத்து **pzStr, எழுத்து *zAppend, முழு doFree, முழு *pRc){
  எழுத்து *zIn = *pzStr;
  எனில்( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  எனில்( *pRc!=SQLITE_OK ){
    sqlite3_free(zIn);
    zIn = 0;
  }இல்லையெனில்{
    எனில்( zIn ){
      எழுத்து *zTemp = zIn;
      zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
      sqlite3_free(zTemp);
    }இல்லையெனில்{
      zIn = sqlite3_mprintf("%s", zAppend);
    }
    எனில்( !zIn ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = zIn;
  எனில்( doFree ){
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
நிலையான எழுத்து *echoSelectList(echo_vtab *pTab, sqlite3_index_info *pIdxInfo){
  எழுத்து *zRet = 0;
  எனில்( sqlite3_libversion_எண்()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }இல்லையெனில்{
    முழு i;
    க்கு(i=0; i<pTab->nCol; i++){
      எனில்( pIdxInfo->colUsed & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->aCol[i]);
      }இல்லையெனில்{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      எனில்( !zRet ) நிறுத்து;
    }
  }
  திரும்பு zRet;
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
நிலையான முழு echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  முழு ii;
  எழுத்து *zQuery = 0;
  எழுத்து *zCol = 0;
  எழுத்து *zNew;
  முழு nArg = 0;
  மாறிலி எழுத்து *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  முழு nRow = 0;
  முழு useIdx = 0;
  முழு rc = SQLITE_OK;
  முழு useCost = 0;
  இரட்டை cost = 0;
  முழு isIgnoreUsable = 0;
  எனில்( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  எனில்( simulateVtabError(pVtab, "xBestIndex") ){
    திரும்பு SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  எனில்( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } இல்லையெனில் {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    எனில்( !zQuery ){
      திரும்பு SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
  }

  zCol = echoSelectList(pVtab, pIdxInfo);
  எனில்( !zCol ) திரும்பு SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", zCol, pVtab->zTableName);
  எனில்( !zQuery ) திரும்பு SQLITE_NOMEM;

  க்கு(ii=0; ii<pIdxInfo->nConstraint; ii++){
    மாறிலி கட்டமைப்பு sqlite3_index_constraint *pConstraint;
    கட்டமைப்பு sqlite3_index_constraint_usage *pUsage;
    முழு iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    எனில்( !isIgnoreUsable && !pConstraint->usable ) தொடர்;

    iCol = pConstraint->iColumn;
    எனில்( iCol<0 || pVtab->aIndex[iCol] ){
      எழுத்து *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
      எழுத்து *zOp = 0;
      useIdx = 1;
      தேர்வு( pConstraint->op ){
        நிலை SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_MATCH:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_LIKE:
          zOp = "like"; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_GLOB:
          zOp = "glob"; நிறுத்து;
        நிலை SQLITE_INDEX_CONSTRAINT_REGEXP:
          zOp = "regexp"; நிறுத்து;
      }
      எனில்( zOp ){
        எனில்( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, zNewCol);
        } இல்லையெனில் {
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
  எனில்( pIdxInfo->nOrderBy==1 && (
        pIdxInfo->aOrderBy->iColumn<0 ||
        pVtab->aIndex[pIdxInfo->aOrderBy->iColumn]) ){
    முழு iCol = pIdxInfo->aOrderBy->iColumn;
    எழுத்து *zNewCol = iCol>=0 ? pVtab->aCol[iCol] : "rowid";
    எழுத்து *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zNewCol, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  எனில்( !zQuery ){
    திரும்பு rc;
  }
  pIdxInfo->idx_இலக்கம் = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  எனில்( useCost ){
    pIdxInfo->estimatedCost = cost;
  }இல்லையெனில் எனில்( useIdx ){
    /* Approximation of log2(nRow). */
    க்கு( ii=0; ii<(அளவு(முழு)*8)-1; ii++ ){
      எனில்( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (இரட்டை)ii;
      }
    }
  }இல்லையெனில்{
    pIdxInfo->estimatedCost = (இரட்டை)nRow;
  }
  திரும்பு rc;
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
முழு echoUpdate(
  sqlite3_vtab *tab, 
  முழு nData, 
  sqlite3_மதிப்பு **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  முழு rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  எழுத்து *z = 0;               /* SQL statement to execute */
  முழு bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  முழு bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  முழு i;                     /* Counter variable used by for loops */

  ப்ரதிஜ்ஞா( nData==pVtab->nCol+2 || nData==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  ப்ரதிஜ்ஞா( pVtab->inTransaction );

  எனில்( simulateVtabError(pVtab, "xUpdate") ){
    திரும்பு SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  எனில்( nData>1 && sqlite3_மதிப்பு_type(apData[0])==SQLITE_INTEGER ){
    எழுத்து *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);
    எனில்( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (apData[1] && sqlite3_மதிப்பு_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    எனில்( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    க்கு(i=2; i<nData; i++){
      எனில்( apData[i]==0 ) தொடர்;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  இல்லையெனில் எனில்( nData==1 && sqlite3_மதிப்பு_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    எனில்( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  இல்லையெனில் எனில்( nData>2 && sqlite3_மதிப்பு_type(apData[0])==SQLITE_NULL ){
    முழு ii;
    எழுத்து *z_செருகு = 0;
    எழுத்து *z_மதிப்புகள் = 0;
  
    z_செருகு = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    எனில்( !z_செருகு ){
      rc = SQLITE_NOMEM;
    }
    எனில்( sqlite3_மதிப்பு_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_மதிப்புகள் = sqlite3_mprintf("?");
      string_concat(&z_செருகு, "rowid", 0, &rc);
    }

    ப்ரதிஜ்ஞா((pVtab->nCol+2)==nData);
    க்கு(ii=2; ii<nData; ii++){
      string_concat(&z_செருகு, 
          sqlite3_mprintf("%s%Q", z_மதிப்புகள்?", ":"", pVtab->aCol[ii-2]), 1, &rc);
      string_concat(&z_மதிப்புகள், 
          sqlite3_mprintf("%s?%d", z_மதிப்புகள்?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_செருகு, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_மதிப்புகள், 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  இல்லையெனில்{
    ப்ரதிஜ்ஞா(0);
    திரும்பு SQLITE_ERROR;
  }

  எனில்( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  ப்ரதிஜ்ஞா( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  எனில்( rc==SQLITE_OK ) {
    எனில்( bindArgZero ){
      sqlite3_bind_மதிப்பு(pStmt, nData, apData[0]);
    }
    எனில்( bindArgOne ){
      sqlite3_bind_மதிப்பு(pStmt, 1, apData[1]);
    }
    க்கு(i=2; i<nData && rc==SQLITE_OK; i++){
      எனில்( apData[i] ) rc = sqlite3_bind_மதிப்பு(pStmt, i, apData[i]);
    }
    எனில்( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }இல்லையெனில்{
      sqlite3_finalize(pStmt);
    }
  }

  எனில்( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_செருகு_rowid(db);
  }
  எனில்( rc!=SQLITE_OK ){
    tab->z_Err_செய்தி = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  திரும்பு rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
நிலையான முழு echoTransactionCall(sqlite3_vtab *tab, மாறிலி எழுத்து *zCall){
  எழுத்து *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  எனில்( z==0 ) திரும்பு SQLITE_NOMEM;
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  திரும்பு SQLITE_OK;
}
நிலையான முழு echoBegin(sqlite3_vtab *tab){
  முழு rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  மாறிலி எழுத்து *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  ப்ரதிஜ்ஞா( !pVtab->inTransaction );

  எனில்( simulateVtabError(pVtab, "xBegin") ){
    திரும்பு SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  எனில்( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    எனில்( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = SQLITE_ERROR;
    }
  }
  எனில்( rc==SQLITE_OK ){
    pVtab->inTransaction = 1;
  }
  திரும்பு rc;
}
நிலையான முழு echoSync(sqlite3_vtab *tab){
  முழு rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  மாறிலி எழுத்து *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  ப்ரதிஜ்ஞா( pVtab->inTransaction );

  எனில்( simulateVtabError(pVtab, "xSync") ){
    திரும்பு SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  எனில்( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    எனில்( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
      rc = -1;
    }
  }
  திரும்பு rc;
}
நிலையான முழு echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  முழு rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  ப்ரதிஜ்ஞா( pVtab->inTransaction );

  எனில்( simulateVtabError(pVtab, "xCommit") ){
    திரும்பு SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3EndBenignMalloc();
  pVtab->inTransaction = 0;
  திரும்பு rc;
}
நிலையான முழு echoRollback(sqlite3_vtab *tab){
  முழு rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  ப்ரதிஜ்ஞா( pVtab->inTransaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->inTransaction = 0;
  திரும்பு rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
நிலையான வெறுமை overloadedGlobFunction(
  sqlite3_context *pContext,
  முழு nArg,
  sqlite3_மதிப்பு **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  முழு i;
  முழு rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  க்கு(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (எழுத்து*)sqlite3_மதிப்பு_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_மதிப்பு(&str));
  Tcl_DStringFree(&str);
  எனில்( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }இல்லையெனில்{
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
நிலையான முழு echoFindFunction(
  sqlite3_vtab *vtab,
  முழு nArg,
  மாறிலி எழுத்து *zFuncName,
  வெறுமை (**pxFunc)(sqlite3_context*,முழு,sqlite3_மதிப்பு**),
  வெறுமை **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  எனில்( strcmp(zFuncName,"glob")!=0 ){
    திரும்பு 0;
  }
  எனில்( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    திரும்பு 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  திரும்பு 1;
}

நிலையான முழு echoRename(sqlite3_vtab *vtab, மாறிலி எழுத்து *zNewName){
  முழு rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  எனில்( simulateVtabError(p, "xRename") ){
    திரும்பு SQLITE_ERROR;
  }

  எனில்( p->isPattern ){
    முழு nThis = (முழு)strlen(p->zThis);
    எழுத்து *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->zTableName, zNewName, &p->zTableName[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  திரும்பு rc;
}

நிலையான முழு echoSavepoint(sqlite3_vtab *pVTab, முழு iSavepoint){
  ப்ரதிஜ்ஞா( pVTab );
  திரும்பு SQLITE_OK;
}

நிலையான முழு echoRelease(sqlite3_vtab *pVTab, முழு iSavepoint){
  ப்ரதிஜ்ஞா( pVTab );
  திரும்பு SQLITE_OK;
}

நிலையான முழு echoRollbackTo(sqlite3_vtab *pVTab, முழு iSavepoint){
  ப்ரதிஜ்ஞா( pVTab );
  திரும்பு SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
நிலையான sqlite3_module echoModule = {
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

நிலையான sqlite3_module echoModuleV2 = {
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
வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

நிலையான வெறுமை moduleDestroy(வெறுமை *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
நிலையான முழு SQLITE_TCLAPI register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  முழு rc;
  sqlite3 *db;
  EchoModule *pMod;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(அளவு(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (வெறுமை*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  எனில்( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(அளவு(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (வெறுமை*)pMod, moduleDestroy
    );
  }

  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);
  திரும்பு TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
நிலையான முழு SQLITE_TCLAPI declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  முழு rc;
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB SQL");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  எனில்( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (எழுத்து *)sqlite3_errmsg(db), TCL_VOLATILE);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest8_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     வெறுமை *clientData;
  } aObjCmd[] = {
     { "register_echo_module",       register_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  திரும்பு TCL_OK;
}
