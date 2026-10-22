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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE

تعريفنوع بنية echo_vtab echo_vtab;
تعريفنوع بنية echo_cursor echo_cursor;

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
بنية echo_vtab {
  sqlite3_vtab بنياد;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  صحيح is_نمونو;
  صحيح اندريون_Transaction;      /* True if within a transaction */
  حرف *zThis;            /* Name of the echo table */
  حرف *z_جدول_Name;       /* Name of the real table */
  حرف *zLogName;         /* Name of the log table */
  صحيح n_کانو;               /* Number of columns in the real table */
  صحيح *a_اشاريو;            /* Array of size nCol. True if column has an index */
  حرف **a_کانو;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
بنية echo_cursor {
  sqlite3_vtab_cursor بنياد;
  sqlite3_stmt *pStmt;
};

ساكن صحيح simulateVtabError(echo_vtab *p, ثابت حرف *z_طريقو){
  ثابت حرف *zErr;
  حرف zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", z_طريقو, p->z_جدول_Name);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  إذا( zErr ){
    p->بنياد.z_Err_نياپو = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  أرجع (zErr!=0);
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
ساكن فراغ dequoteString(حرف *z){
  صحيح quote;
  صحيح i, j;
  إذا( z==0 ) أرجع;
  quote = z[0];
  بدل( quote ){
    حالة '\'':  اكسر;
    حالة '"':   اكسر;
    حالة '`':   اكسر;                /* For MySQL compatibility */
    حالة '[':   quote = ']';  اكسر;  /* For MS SqlServer compatibility */
    افتراضي:    أرجع;
  }
  لكل(i=1, j=0; z[i]; i++){
    إذا( z[i]==quote ){
      إذا( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }وإلا{
        z[j++] = 0;
        اكسر;
      }
    }وإلا{
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
ساكن صحيح get_ڪالم_نالا(
  sqlite3 *db, 
  ثابت حرف *zTab,
  حرف ***pa_کانو, 
  صحيح *pn_کانو
){
  حرف **a_کانو = 0;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحيح rc = SQLITE_OK;
  صحيح n_کانو = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  إذا( !zSql ){
    rc = SQLITE_NOMEM;
    اذهب ٻاهريون;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  إذا( rc==SQLITE_OK ){
    صحيح ii;
    صحيح n_بائيٽون;
    حرف *zSpace;
    n_کانو = sqlite3_ڪالم_ڳڻپ(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    n_بائيٽون = حجم(حرف *) * n_کانو;
    لكل(ii=0; ii<n_کانو; ii++){
      ثابت حرف *zName = sqlite3_ڪالم_name(pStmt, ii);
      إذا( !zName ){
        rc = SQLITE_NOMEM;
        اذهب ٻاهريون;
      }
      n_بائيٽون += (صحيح)لڙي_ڊيگهه(zName)+1;
    }
    a_کانو = (حرف **)sqlite3MallocZero(n_بائيٽون);
    إذا( !a_کانو ){
      rc = SQLITE_NOMEM;
      اذهب ٻاهريون;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (حرف *)(&a_کانو[n_کانو]);
    لكل(ii=0; ii<n_کانو; ii++){
      a_کانو[ii] = zSpace;
      sqlite3_snprintf(n_بائيٽون, zSpace, "%s", sqlite3_ڪالم_name(pStmt,ii));
      zSpace += (صحيح)لڙي_ڊيگهه(zSpace) + 1;
    }
    أكد( (zSpace-n_بائيٽون)==(حرف *)a_کانو );
  }

  *pa_کانو = a_کانو;
  *pn_کانو = n_کانو;

ٻاهريون:
  sqlite3_finalize(pStmt);
  أرجع rc;
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
ساكن صحيح get_اشاريو_Array(
  sqlite3 *db,             /* Database connection */
  ثابت حرف *zTab,        /* Name of table in database db */
  صحيح n_کانو,
  صحيح **pa_اشاريو
){
  sqlite3_stmt *pStmt = 0;
  صحيح *a_اشاريو = 0;
  صحيح rc;
  حرف *zSql;

  /* Allocate space for the index array */
  a_اشاريو = (صحيح *)sqlite3MallocZero(حجم(صحيح) * n_کانو);
  إذا( !a_اشاريو ){
    rc = SQLITE_NOMEM;
    اذهب get_اشاريو_array_ٻاهريون;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  إذا( !zSql ){
    rc = SQLITE_NOMEM;
    اذهب get_اشاريو_array_ٻاهريون;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  طالما( pStmt && sqlite3_قدم(pStmt)==SQLITE_صف ){
    ثابت حرف *zIdx = (ثابت حرف *)sqlite3_ڪالم_متن(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    إذا( zIdx==0 ) استمر;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    إذا( !zSql ){
      rc = SQLITE_NOMEM;
      اذهب get_اشاريو_array_ٻاهريون;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    إذا( pStmt2 && sqlite3_قدم(pStmt2)==SQLITE_صف ){
      صحيح cid = sqlite3_ڪالم_int(pStmt2, 1);
      أكد( cid>=0 && cid<n_کانو );
      a_اشاريو[cid] = 1;
    }
    إذا( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    إذا( rc!=SQLITE_OK ){
      اذهب get_اشاريو_array_ٻاهريون;
    }
  }


get_اشاريو_array_ٻاهريون:
  إذا( pStmt ){
    صحيح rc2 = sqlite3_finalize(pStmt);
    إذا( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  إذا( rc!=SQLITE_OK ){
    sqlite3_free(a_اشاريو);
    a_اشاريو = 0;
  }
  *pa_اشاريو = a_اشاريو;
  أرجع rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ساكن فراغ append_منزل_Echo_Module(Tcl_Interp *interp, ثابت حرف *zArg){
  صحيح flags = (TCL_APPEND_قدر | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_سيٽ_Var(interp, "echo_module", (zArg?zArg:""), flags);
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
ساكن صحيح echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  صحيح rc = SQLITE_OK;

  إذا( pVtab->z_جدول_Name ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    إذا( rc==SQLITE_OK ){
      sqlite3_bind_متن(pStmt, 1, pVtab->z_جدول_Name, -1, 0);
      إذا( sqlite3_قدم(pStmt)==SQLITE_صف ){
        صحيح rc2;
        ثابت حرف *z_Create_جدول = (ثابت حرف *)sqlite3_ڪالم_متن(pStmt, 0);
        rc = sqlite3_declare_vtab(db, z_Create_جدول);
        rc2 = sqlite3_finalize(pStmt);
        إذا( rc==SQLITE_OK ){
          rc = rc2;
        }
      } وإلا {
        rc = sqlite3_finalize(pStmt);
        إذا( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      إذا( rc==SQLITE_OK ){
        rc = get_ڪالم_نالا(db, pVtab->z_جدول_Name, &pVtab->a_کانو, &pVtab->n_کانو);
      }
      إذا( rc==SQLITE_OK ){
        rc = get_اشاريو_Array(db, pVtab->z_جدول_Name, pVtab->n_کانو, &pVtab->a_اشاريو);
      }
    }
  }

  أرجع rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ساكن صحيح echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_اشاريو);
  sqlite3_free(p->a_کانو);
  sqlite3_free(p->zThis);
  sqlite3_free(p->z_جدول_Name);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  أرجع 0;
}

تعريفنوع بنية EchoModule EchoModule;
بنية EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ساكن صحيح echoConstructor(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحيح rc;
  صحيح i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( حجم(*pVtab) );
  إذا( !pVtab ){
    أرجع SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  إذا( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    أرجع SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  إذا( argc>3 ){
    pVtab->z_جدول_Name = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->z_جدول_Name);
    إذا( pVtab->z_جدول_Name && pVtab->z_جدول_Name[0]=='*' ){
      حرف *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->z_جدول_Name[1]));
      sqlite3_free(pVtab->z_جدول_Name);
      pVtab->z_جدول_Name = z;
      pVtab->is_نمونو = 1;
    }
    إذا( !pVtab->z_جدول_Name ){
      echoDestructor((sqlite3_vtab *)pVtab);
      أرجع SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  لكل(i=0; i<argc; i++){
    append_منزل_Echo_Module(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  إذا( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    أرجع rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->بنياد;
  أرجع SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ساكن صحيح echoCreate(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحيح rc = SQLITE_OK;
  append_منزل_Echo_Module(((EchoModule *)pAux)->interp, "xCreate");
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
  إذا( rc==SQLITE_OK && argc==5 ){
    حرف *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    إذا( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  إذا( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  إذا( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->اندريون_Transaction = 1;
  }

  أرجع rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ساكن صحيح echoConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  append_منزل_Echo_Module(((EchoModule *)pAux)->interp, "xConnect");
  أرجع echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ساكن صحيح echoDisconnect(sqlite3_vtab *pVtab){
  append_منزل_Echo_Module(((echo_vtab *)pVtab)->interp, "xDisconnect");
  أرجع echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ساكن صحيح echoDestroy(sqlite3_vtab *pVtab){
  صحيح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  append_منزل_Echo_Module(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  إذا( p && p->zLogName ){
    حرف *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  إذا( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  أرجع rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ساكن صحيح echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  إذا( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    أرجع SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(حجم(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  أرجع (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ساكن صحيح echoClose(sqlite3_vtab_cursor *cur){
  صحيح rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  أرجع rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ساكن صحيح echoEof(sqlite3_vtab_cursor *cur){
  أرجع (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ساكن صحيح echo_ايندڙ(sqlite3_vtab_cursor *cur){
  صحيح rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  إذا( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    أرجع SQLITE_ERROR;
  }

  إذا( pCur->pStmt ){
    rc = sqlite3_قدم(pCur->pStmt);
    إذا( rc==SQLITE_صف ){
      rc = SQLITE_OK;
    }وإلا{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  أرجع rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ساكن صحيح echo_ڪالم(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  صحيح i_کانو = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  إذا( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    أرجع SQLITE_ERROR;
  }

  إذا( !pStmt ){
    sqlite3_نتيجو_null(ctx);
  }وإلا{
    أكد( sqlite3_معلومات_ڳڻپ(pStmt)>i_کانو );
    sqlite3_نتيجو_قدر(ctx, sqlite3_ڪالم_قدر(pStmt, i_کانو));
  }
  أرجع SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ساكن صحيح echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  إذا( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    أرجع SQLITE_ERROR;
  }

  *pRowid = sqlite3_ڪالم_int64(pStmt, 0);
  أرجع SQLITE_OK;
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
ساكن صحيح hashString(ثابت حرف *zString){
  u32 val = 0;
  صحيح ii;
  لكل(ii=0; zString[ii]; ii++){
    val = (val << 3) + (صحيح)zString[ii];
  }
  أرجع (صحيح)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ساكن صحيح echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_انگ, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  صحيح rc;
  صحيح i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  إذا( simulateVtabError(pVtab, "xFilter") ){
    أرجع SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  أكد( idx_انگ==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  append_منزل_Echo_Module(pVtab->interp, "xFilter");
  append_منزل_Echo_Module(pVtab->interp, idxStr);
  لكل(i=0; i<argc; i++){
    append_منزل_Echo_Module(pVtab->interp, (ثابت حرف*)sqlite3_قدر_متن(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  أكد( pCur->pStmt || rc!=SQLITE_OK );
  لكل(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_قدر(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  إذا( rc==SQLITE_OK ){
    rc = echo_ايندڙ(pVtabCursor);
  }

  أرجع rc;
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
ساكن فراغ string_concat(حرف **pzStr, حرف *zAppend, صحيح doFree, صحيح *pRc){
  حرف *z_اندريون = *pzStr;
  إذا( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  إذا( *pRc!=SQLITE_OK ){
    sqlite3_free(z_اندريون);
    z_اندريون = 0;
  }وإلا{
    إذا( z_اندريون ){
      حرف *z_عارضي = z_اندريون;
      z_اندريون = sqlite3_mprintf("%s%s", z_اندريون, zAppend);
      sqlite3_free(z_عارضي);
    }وإلا{
      z_اندريون = sqlite3_mprintf("%s", zAppend);
    }
    إذا( !z_اندريون ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = z_اندريون;
  إذا( doFree ){
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
ساكن حرف *echoSelectList(echo_vtab *pTab, sqlite3_اشاريو_info *pIdxInfo){
  حرف *zRet = 0;
  إذا( sqlite3_libversion_عدد()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }وإلا{
    صحيح i;
    لكل(i=0; i<pTab->n_کانو; i++){
      إذا( pIdxInfo->کانو_استعمال_ٿيل & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->a_کانو[i]);
      }وإلا{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      إذا( !zRet ) اكسر;
    }
  }
  أرجع zRet;
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
ساكن صحيح echo_بهترين_اشاريو(sqlite3_vtab *tab, sqlite3_اشاريو_info *pIdxInfo){
  صحيح ii;
  حرف *zQuery = 0;
  حرف *z_کانو = 0;
  حرف *zNew;
  صحيح nArg = 0;
  ثابت حرف *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  صحيح n_صف = 0;
  صحيح useIdx = 0;
  صحيح rc = SQLITE_OK;
  صحيح use_لاڳت = 0;
  مزدوج لاڳت = 0;
  صحيح isIgnoreUsable = 0;
  إذا( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  إذا( simulateVtabError(pVtab, "xBestIndex") ){
    أرجع SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  إذا( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    لاڳت = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    use_لاڳت = 1;
  } وإلا {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->z_جدول_Name);
    إذا( !zQuery ){
      أرجع SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    sqlite3_قدم(pStmt);
    n_صف = sqlite3_ڪالم_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
  }

  z_کانو = echoSelectList(pVtab, pIdxInfo);
  إذا( !z_کانو ) أرجع SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", z_کانو, pVtab->z_جدول_Name);
  إذا( !zQuery ) أرجع SQLITE_NOMEM;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ثابت بنية sqlite3_اشاريو_constraint *pConstraint;
    بنية sqlite3_اشاريو_constraint_usage *pUsage;
    صحيح i_کانو;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    إذا( !isIgnoreUsable && !pConstraint->usable ) استمر;

    i_کانو = pConstraint->i_ڪالم;
    إذا( i_کانو<0 || pVtab->a_اشاريو[i_کانو] ){
      حرف *z_New_کانو = i_کانو>=0 ? pVtab->a_کانو[i_کانو] : "rowid";
      حرف *zOp = 0;
      useIdx = 1;
      بدل( pConstraint->op ){
        حالة SQLITE_اشاريو_CONSTRAINT_EQ:
          zOp = "="; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_LT:
          zOp = "<"; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_GT:
          zOp = ">"; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_LE:
          zOp = "<="; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_GE:
          zOp = ">="; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_ميلاپ:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_LIKE:
          zOp = "like"; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_GLOB:
          zOp = "glob"; اكسر;
        حالة SQLITE_اشاريو_CONSTRAINT_REGEXP:
          zOp = "regexp"; اكسر;
      }
      إذا( zOp ){
        إذا( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, z_New_کانو);
        } وإلا {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, z_New_کانو, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_اشاريو = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  إذا( pIdxInfo->n_ترتيب_By==1 && (
        pIdxInfo->a_ترتيب_By->i_ڪالم<0 ||
        pVtab->a_اشاريو[pIdxInfo->a_ترتيب_By->i_ڪالم]) ){
    صحيح i_کانو = pIdxInfo->a_ترتيب_By->i_ڪالم;
    حرف *z_New_کانو = i_کانو>=0 ? pVtab->a_کانو[i_کانو] : "rowid";
    حرف *zDir = pIdxInfo->a_ترتيب_By->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", z_New_کانو, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->ترتيب_By_Consumed = 1;
  }

  append_منزل_Echo_Module(pVtab->interp, "xBestIndex");;
  append_منزل_Echo_Module(pVtab->interp, zQuery);

  إذا( !zQuery ){
    أرجع rc;
  }
  pIdxInfo->idx_انگ = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->need_منزل_Free_Idx_Str = 1;
  إذا( use_لاڳت ){
    pIdxInfo->estimated_لاڳت = لاڳت;
  }وإلا إذا( useIdx ){
    /* Approximation of log2(nRow). */
    لكل( ii=0; ii<(حجم(صحيح)*8)-1; ii++ ){
      إذا( n_صف & (1<<ii) ){
        pIdxInfo->estimated_لاڳت = (مزدوج)ii;
      }
    }
  }وإلا{
    pIdxInfo->estimated_لاڳت = (مزدوج)n_صف;
  }
  أرجع rc;
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
صحيح echoUpdate(
  sqlite3_vtab *tab, 
  صحيح n_معلومات, 
  sqlite3_قدر **ap_معلومات, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  صحيح rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  حرف *z = 0;               /* SQL statement to execute */
  صحيح bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  صحيح bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  صحيح i;                     /* Counter variable used by for loops */

  أكد( n_معلومات==pVtab->n_کانو+2 || n_معلومات==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  أكد( pVtab->اندريون_Transaction );

  إذا( simulateVtabError(pVtab, "xUpdate") ){
    أرجع SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  إذا( n_معلومات>1 && sqlite3_قدر_type(ap_معلومات[0])==SQLITE_INTEGER ){
    حرف *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->z_جدول_Name);
    إذا( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_معلومات[1] && sqlite3_قدر_type(ap_معلومات[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    إذا( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    لكل(i=2; i<n_معلومات; i++){
      إذا( ap_معلومات[i]==0 ) استمر;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->a_کانو[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_معلومات), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  وإلا إذا( n_معلومات==1 && sqlite3_قدر_type(ap_معلومات[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->z_جدول_Name);
    إذا( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  وإلا إذا( n_معلومات>2 && sqlite3_قدر_type(ap_معلومات[0])==SQLITE_NULL ){
    صحيح ii;
    حرف *z_وجهو = 0;
    حرف *z_قدرون = 0;
  
    z_وجهو = sqlite3_mprintf("INSERT INTO %Q (", pVtab->z_جدول_Name);
    إذا( !z_وجهو ){
      rc = SQLITE_NOMEM;
    }
    إذا( sqlite3_قدر_type(ap_معلومات[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_قدرون = sqlite3_mprintf("?");
      string_concat(&z_وجهو, "rowid", 0, &rc);
    }

    أكد((pVtab->n_کانو+2)==n_معلومات);
    لكل(ii=2; ii<n_معلومات; ii++){
      string_concat(&z_وجهو, 
          sqlite3_mprintf("%s%Q", z_قدرون?", ":"", pVtab->a_کانو[ii-2]), 1, &rc);
      string_concat(&z_قدرون, 
          sqlite3_mprintf("%s?%d", z_قدرون?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_وجهو, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_قدرون, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  وإلا{
    أكد(0);
    أرجع SQLITE_ERROR;
  }

  إذا( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  أكد( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  إذا( rc==SQLITE_OK ) {
    إذا( bindArgZero ){
      sqlite3_bind_قدر(pStmt, n_معلومات, ap_معلومات[0]);
    }
    إذا( bindArgOne ){
      sqlite3_bind_قدر(pStmt, 1, ap_معلومات[1]);
    }
    لكل(i=2; i<n_معلومات && rc==SQLITE_OK; i++){
      إذا( ap_معلومات[i] ) rc = sqlite3_bind_قدر(pStmt, i, ap_معلومات[i]);
    }
    إذا( rc==SQLITE_OK ){
      sqlite3_قدم(pStmt);
      rc = sqlite3_finalize(pStmt);
    }وإلا{
      sqlite3_finalize(pStmt);
    }
  }

  إذا( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_آخري_وجهو_rowid(db);
  }
  إذا( rc!=SQLITE_OK ){
    tab->z_Err_نياپو = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  أرجع rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ساكن صحيح echoTransactionCall(sqlite3_vtab *tab, ثابت حرف *zCall){
  حرف *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->z_جدول_Name);
  إذا( z==0 ) أرجع SQLITE_NOMEM;
  append_منزل_Echo_Module(pVtab->interp, zCall);
  append_منزل_Echo_Module(pVtab->interp, z);
  sqlite3_free(z);
  أرجع SQLITE_OK;
}
ساكن صحيح echoBegin(sqlite3_vtab *tab){
  صحيح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت حرف *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  أكد( !pVtab->اندريون_Transaction );

  إذا( simulateVtabError(pVtab, "xBegin") ){
    أرجع SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  إذا( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    إذا( zVal && 0==strcmp(zVal, pVtab->z_جدول_Name) ){
      rc = SQLITE_ERROR;
    }
  }
  إذا( rc==SQLITE_OK ){
    pVtab->اندريون_Transaction = 1;
  }
  أرجع rc;
}
ساكن صحيح echoSync(sqlite3_vtab *tab){
  صحيح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت حرف *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  أكد( pVtab->اندريون_Transaction );

  إذا( simulateVtabError(pVtab, "xSync") ){
    أرجع SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  إذا( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    إذا( zVal && 0==strcmp(zVal, pVtab->z_جدول_Name) ){
      rc = -1;
    }
  }
  أرجع rc;
}
ساكن صحيح echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  صحيح rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  أكد( pVtab->اندريون_Transaction );

  إذا( simulateVtabError(pVtab, "xCommit") ){
    أرجع SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3_پڇاڙي_Benign_Malloc();
  pVtab->اندريون_Transaction = 0;
  أرجع rc;
}
ساكن صحيح echoRollback(sqlite3_vtab *tab){
  صحيح rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  أكد( pVtab->اندريون_Transaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->اندريون_Transaction = 0;
  أرجع rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ساكن فراغ overloadedGlobFunction(
  sqlite3_context *pContext,
  صحيح nArg,
  sqlite3_قدر **apArg
){
  Tcl_Interp *interp = sqlite3_user_معلومات(pContext);
  Tcl_DString str;
  صحيح i;
  صحيح rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  لكل(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (حرف*)sqlite3_قدر_متن(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_قدر(&str));
  Tcl_DStringFree(&str);
  إذا( rc ){
    sqlite3_نتيجو_error(pContext, Tcl_Get_String_نتيجو(interp), -1);
  }وإلا{
    sqlite3_نتيجو_متن(pContext, Tcl_Get_String_نتيجو(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_نتيجو(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
ساكن صحيح echo_ڳوليو_Function(
  sqlite3_vtab *vtab,
  صحيح nArg,
  ثابت حرف *zFuncName,
  فراغ (**pxFunc)(sqlite3_context*,صحيح,sqlite3_قدر**),
  فراغ **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  إذا( strcmp(zFuncName,"glob")!=0 ){
    أرجع 0;
  }
  إذا( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    أرجع 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  أرجع 1;
}

ساكن صحيح echoRename(sqlite3_vtab *vtab, ثابت حرف *zNewName){
  صحيح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  إذا( simulateVtabError(p, "xRename") ){
    أرجع SQLITE_ERROR;
  }

  إذا( p->is_نمونو ){
    صحيح nThis = (صحيح)لڙي_ڊيگهه(p->zThis);
    حرف *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->z_جدول_Name, zNewName, &p->z_جدول_Name[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  أرجع rc;
}

ساكن صحيح echoSavepoint(sqlite3_vtab *pVTab, صحيح iSavepoint){
  أكد( pVTab );
  أرجع SQLITE_OK;
}

ساكن صحيح echoRelease(sqlite3_vtab *pVTab, صحيح iSavepoint){
  أكد( pVTab );
  أرجع SQLITE_OK;
}

ساكن صحيح echo_Rollback_منزل(sqlite3_vtab *pVTab, صحيح iSavepoint){
  أكد( pVTab );
  أرجع SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ساكن sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_بهترين_اشاريو,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_ايندڙ,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_ڪالم,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ڳوليو_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};

ساكن sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_بهترين_اشاريو,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_ايندڙ,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_ڪالم,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ڳوليو_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  echoSavepoint,
  echoRelease,
  echo_Rollback_منزل,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
خارجي صحيح get_Db_اشارو(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

ساكن فراغ moduleDestroy(فراغ *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI رجسٽر_echo_module(
  Client_معلومات client_معلومات, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحيح rc;
  sqlite3 *db;
  EchoModule *pMod;
  إذا( objc!=2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(حجم(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (فراغ*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  إذا( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(حجم(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (فراغ*)pMod, moduleDestroy
    );
  }

  Tcl_سيٽ_نتيجو(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  أرجع TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ساكن صحيح SQLITE_TCLAPI declare_vtab(
  Client_معلومات client_معلومات, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  صحيح rc;
  إذا( objc!=3 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB SQL");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  إذا( rc!=SQLITE_OK ){
    Tcl_سيٽ_نتيجو(interp, (حرف *)sqlite3_errmsg(db), TCL_VOLATILE);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

#جيڪڏهن_پورو /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest8_Init(Tcl_Interp *interp){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *client_معلومات;
  } aObjCmd[] = {
     { "register_echo_module",       رجسٽر_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_معلومات, 0);
  }
#جيڪڏهن_پورو
  أرجع TCL_OK;
}
