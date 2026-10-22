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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE

قسمتعریف ساخت echo_vtab echo_vtab;
قسمتعریف ساخت echo_cursor echo_cursor;

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
ساخت echo_vtab {
  sqlite3_vtab بنیاد;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  صحیح is_نمونہ;
  صحیح اندر_Transaction;      /* True if within a transaction */
  حرف *zThis;            /* Name of the echo table */
  حرف *z_جدول_Name;       /* Name of the real table */
  حرف *zLogName;         /* Name of the log table */
  صحیح n_ستون;               /* Number of columns in the real table */
  صحیح *a_اشاریہ;            /* Array of size nCol. True if column has an index */
  حرف **a_ستون;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
ساخت echo_cursor {
  sqlite3_vtab_cursor بنیاد;
  sqlite3_stmt *pStmt;
};

ساکن صحیح simulateVtabError(echo_vtab *p, ثابت حرف *z_طریقہ){
  ثابت حرف *zErr;
  حرف zVarname[128];
  zVarname[127] = '\0';
  sqlite3_snprintf(127, zVarname, "echo_module_fail(%s,%s)", z_طریقہ, p->z_جدول_Name);
  zErr = Tcl_GetVar(p->interp, zVarname, TCL_GLOBAL_ONLY);
  اگر( zErr ){
    p->بنیاد.z_Err_پیغام = sqlite3_mprintf("echo-vtab-error: %s", zErr);
  }
  واپس (zErr!=0);
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
ساکن خالی dequoteString(حرف *z){
  صحیح quote;
  صحیح i, j;
  اگر( z==0 ) واپس;
  quote = z[0];
  انتخاب( quote ){
    حالت '\'':  توڑو;
    حالت '"':   توڑو;
    حالت '`':   توڑو;                /* For MySQL compatibility */
    حالت '[':   quote = ']';  توڑو;  /* For MS SqlServer compatibility */
    طےشدہ:    واپس;
  }
  برائے(i=1, j=0; z[i]; i++){
    اگر( z[i]==quote ){
      اگر( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }ورنہ{
        z[j++] = 0;
        توڑو;
      }
    }ورنہ{
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
ساکن صحیح get_عمود_نام(
  sqlite3 *db, 
  ثابت حرف *zTab,
  حرف ***pa_ستون, 
  صحیح *pn_ستون
){
  حرف **a_ستون = 0;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحیح rc = SQLITE_OK;
  صحیح n_ستون = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3_mprintf("SELECT * FROM %Q", zTab);
  اگر( !zSql ){
    rc = SQLITE_NOMEM;
    جاؤ برآمد;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  اگر( rc==SQLITE_OK ){
    صحیح ii;
    صحیح n_بائٹیں;
    حرف *zSpace;
    n_ستون = sqlite3_عمود_گنتی(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    n_بائٹیں = حجم(حرف *) * n_ستون;
    برائے(ii=0; ii<n_ستون; ii++){
      ثابت حرف *zName = sqlite3_عمود_name(pStmt, ii);
      اگر( !zName ){
        rc = SQLITE_NOMEM;
        جاؤ برآمد;
      }
      n_بائٹیں += (صحیح)لڑی_لمبائی(zName)+1;
    }
    a_ستون = (حرف **)sqlite3MallocZero(n_بائٹیں);
    اگر( !a_ستون ){
      rc = SQLITE_NOMEM;
      جاؤ برآمد;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (حرف *)(&a_ستون[n_ستون]);
    برائے(ii=0; ii<n_ستون; ii++){
      a_ستون[ii] = zSpace;
      sqlite3_snprintf(n_بائٹیں, zSpace, "%s", sqlite3_عمود_name(pStmt,ii));
      zSpace += (صحیح)لڑی_لمبائی(zSpace) + 1;
    }
    أكد( (zSpace-n_بائٹیں)==(حرف *)a_ستون );
  }

  *pa_ستون = a_ستون;
  *pn_ستون = n_ستون;

برآمد:
  sqlite3_finalize(pStmt);
  واپس rc;
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
ساکن صحیح get_اشاریہ_Array(
  sqlite3 *db,             /* Database connection */
  ثابت حرف *zTab,        /* Name of table in database db */
  صحیح n_ستون,
  صحیح **pa_اشاریہ
){
  sqlite3_stmt *pStmt = 0;
  صحیح *a_اشاریہ = 0;
  صحیح rc;
  حرف *zSql;

  /* Allocate space for the index array */
  a_اشاریہ = (صحیح *)sqlite3MallocZero(حجم(صحیح) * n_ستون);
  اگر( !a_اشاریہ ){
    rc = SQLITE_NOMEM;
    جاؤ get_اشاریہ_array_برآمد;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3_mprintf("PRAGMA index_list(%s)", zTab);
  اگر( !zSql ){
    rc = SQLITE_NOMEM;
    جاؤ get_اشاریہ_array_برآمد;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3_free(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  جب( pStmt && sqlite3_قدم(pStmt)==SQLITE_صف ){
    ثابت حرف *zIdx = (ثابت حرف *)sqlite3_عمود_متن(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    اگر( zIdx==0 ) جاری;
    zSql = sqlite3_mprintf("PRAGMA index_info(%s)", zIdx);
    اگر( !zSql ){
      rc = SQLITE_NOMEM;
      جاؤ get_اشاریہ_array_برآمد;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqlite3_free(zSql);
    اگر( pStmt2 && sqlite3_قدم(pStmt2)==SQLITE_صف ){
      صحیح cid = sqlite3_عمود_int(pStmt2, 1);
      أكد( cid>=0 && cid<n_ستون );
      a_اشاریہ[cid] = 1;
    }
    اگر( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    اگر( rc!=SQLITE_OK ){
      جاؤ get_اشاریہ_array_برآمد;
    }
  }


get_اشاریہ_array_برآمد:
  اگر( pStmt ){
    صحیح rc2 = sqlite3_finalize(pStmt);
    اگر( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  اگر( rc!=SQLITE_OK ){
    sqlite3_free(a_اشاریہ);
    a_اشاریہ = 0;
  }
  *pa_اشاریہ = a_اشاریہ;
  واپس rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
ساکن خالی append_مقصد_Echo_Module(Tcl_Interp *interp, ثابت حرف *zArg){
  صحیح flags = (TCL_APPEND_قدر | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var(interp, "echo_module", (zArg?zArg:""), flags);
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
ساکن صحیح echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db 
){
  صحیح rc = SQLITE_OK;

  اگر( pVtab->z_جدول_Name ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    اگر( rc==SQLITE_OK ){
      sqlite3_bind_متن(pStmt, 1, pVtab->z_جدول_Name, -1, 0);
      اگر( sqlite3_قدم(pStmt)==SQLITE_صف ){
        صحیح rc2;
        ثابت حرف *z_Create_جدول = (ثابت حرف *)sqlite3_عمود_متن(pStmt, 0);
        rc = sqlite3_declare_vtab(db, z_Create_جدول);
        rc2 = sqlite3_finalize(pStmt);
        اگر( rc==SQLITE_OK ){
          rc = rc2;
        }
      } ورنہ {
        rc = sqlite3_finalize(pStmt);
        اگر( rc==SQLITE_OK ){ 
          rc = SQLITE_ERROR;
        }
      }
      اگر( rc==SQLITE_OK ){
        rc = get_عمود_نام(db, pVtab->z_جدول_Name, &pVtab->a_ستون, &pVtab->n_ستون);
      }
      اگر( rc==SQLITE_OK ){
        rc = get_اشاریہ_Array(db, pVtab->z_جدول_Name, pVtab->n_ستون, &pVtab->a_اشاریہ);
      }
    }
  }

  واپس rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
ساکن صحیح echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqlite3_free(p->a_اشاریہ);
  sqlite3_free(p->a_ستون);
  sqlite3_free(p->zThis);
  sqlite3_free(p->z_جدول_Name);
  sqlite3_free(p->zLogName);
  sqlite3_free(p);
  واپس 0;
}

قسمتعریف ساخت EchoModule EchoModule;
ساخت EchoModule {
  Tcl_Interp *interp;
  sqlite3 *db;
};

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
ساکن صحیح echoConstructor(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحیح rc;
  صحیح i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqlite3MallocZero( حجم(*pVtab) );
  اگر( !pVtab ){
    واپس SQLITE_NOMEM;
  }
  pVtab->interp = ((EchoModule *)pAux)->interp;
  pVtab->db = db;

  /* Allocate echo_vtab.zThis */
  pVtab->zThis = sqlite3_mprintf("%s", argv[2]);
  اگر( !pVtab->zThis ){
    echoDestructor((sqlite3_vtab *)pVtab);
    واپس SQLITE_NOMEM;
  }

  /* Allocate echo_vtab.zTableName */
  اگر( argc>3 ){
    pVtab->z_جدول_Name = sqlite3_mprintf("%s", argv[3]);
    dequoteString(pVtab->z_جدول_Name);
    اگر( pVtab->z_جدول_Name && pVtab->z_جدول_Name[0]=='*' ){
      حرف *z = sqlite3_mprintf("%s%s", argv[2], &(pVtab->z_جدول_Name[1]));
      sqlite3_free(pVtab->z_جدول_Name);
      pVtab->z_جدول_Name = z;
      pVtab->is_نمونہ = 1;
    }
    اگر( !pVtab->z_جدول_Name ){
      echoDestructor((sqlite3_vtab *)pVtab);
      واپس SQLITE_NOMEM;
    }
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  برائے(i=0; i<argc; i++){
    append_مقصد_Echo_Module(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  rc = echoDeclareVtab(pVtab, db);
  اگر( rc!=SQLITE_OK ){
    echoDestructor((sqlite3_vtab *)pVtab);
    واپس rc;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->بنیاد;
  واپس SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
ساکن صحیح echoCreate(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  صحیح rc = SQLITE_OK;
  append_مقصد_Echo_Module(((EchoModule *)pAux)->interp, "xCreate");
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
  اگر( rc==SQLITE_OK && argc==5 ){
    حرف *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3_mprintf("%s", argv[4]);
    zSql = sqlite3_mprintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
    اگر( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  اگر( *ppVtab && rc!=SQLITE_OK ){
    echoDestructor(*ppVtab);
    *ppVtab = 0;
  }

  اگر( rc==SQLITE_OK ){
    (*(echo_vtab**)ppVtab)->اندر_Transaction = 1;
  }

  واپس rc;
}

/* 
** Echo virtual table module xConnect method.
*/
ساکن صحیح echoConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  append_مقصد_Echo_Module(((EchoModule *)pAux)->interp, "xConnect");
  واپس echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
ساکن صحیح echoDisconnect(sqlite3_vtab *pVtab){
  append_مقصد_Echo_Module(((echo_vtab *)pVtab)->interp, "xDisconnect");
  واپس echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
ساکن صحیح echoDestroy(sqlite3_vtab *pVtab){
  صحیح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  append_مقصد_Echo_Module(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  اگر( p && p->zLogName ){
    حرف *zSql;
    zSql = sqlite3_mprintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  اگر( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  واپس rc;
}

/* 
** Echo virtual table module xOpen method.
*/
ساکن صحیح echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  اگر( simulateVtabError((echo_vtab *)pVTab, "xOpen") ){
    واپس SQLITE_ERROR;
  }
  pCur = sqlite3MallocZero(حجم(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  واپس (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
ساکن صحیح echoClose(sqlite3_vtab_cursor *cur){
  صحیح rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqlite3_free(pCur);
  rc = sqlite3_finalize(pStmt);
  واپس rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
ساکن صحیح echoEof(sqlite3_vtab_cursor *cur){
  واپس (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
ساکن صحیح echo_اگلا(sqlite3_vtab_cursor *cur){
  صحیح rc = SQLITE_OK;
  echo_cursor *pCur = (echo_cursor *)cur;

  اگر( simulateVtabError((echo_vtab *)(cur->pVtab), "xNext") ){
    واپس SQLITE_ERROR;
  }

  اگر( pCur->pStmt ){
    rc = sqlite3_قدم(pCur->pStmt);
    اگر( rc==SQLITE_صف ){
      rc = SQLITE_OK;
    }ورنہ{
      rc = sqlite3_finalize(pCur->pStmt);
      pCur->pStmt = 0;
    }
  }

  واپس rc;
}

/* 
** Echo virtual table module xColumn method.
*/
ساکن صحیح echo_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  صحیح i_ستون = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  اگر( simulateVtabError((echo_vtab *)(cur->pVtab), "xColumn") ){
    واپس SQLITE_ERROR;
  }

  اگر( !pStmt ){
    sqlite3_نتیجہ_null(ctx);
  }ورنہ{
    أكد( sqlite3_ڈیٹا_گنتی(pStmt)>i_ستون );
    sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pStmt, i_ستون));
  }
  واپس SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
ساکن صحیح echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;

  اگر( simulateVtabError((echo_vtab *)(cur->pVtab), "xRowid") ){
    واپس SQLITE_ERROR;
  }

  *pRowid = sqlite3_عمود_int64(pStmt, 0);
  واپس SQLITE_OK;
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
ساکن صحیح hashString(ثابت حرف *zString){
  u32 val = 0;
  صحیح ii;
  برائے(ii=0; zString[ii]; ii++){
    val = (val << 3) + (صحیح)zString[ii];
  }
  واپس (صحیح)(val&0x7fffffff);
}

/* 
** Echo virtual table module xFilter method.
*/
ساکن صحیح echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  صحیح rc;
  صحیح i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  اگر( simulateVtabError(pVtab, "xFilter") ){
    واپس SQLITE_ERROR;
  }

  /* Check that idxNum matches idxStr */
  أكد( idx_عدد==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  append_مقصد_Echo_Module(pVtab->interp, "xFilter");
  append_مقصد_Echo_Module(pVtab->interp, idxStr);
  برائے(i=0; i<argc; i++){
    append_مقصد_Echo_Module(pVtab->interp, (ثابت حرف*)sqlite3_قدر_متن(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  أكد( pCur->pStmt || rc!=SQLITE_OK );
  برائے(i=0; rc==SQLITE_OK && i<argc; i++){
    rc = sqlite3_bind_قدر(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  اگر( rc==SQLITE_OK ){
    rc = echo_اگلا(pVtabCursor);
  }

  واپس rc;
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
ساکن خالی string_concat(حرف **pzStr, حرف *zAppend, صحیح doFree, صحیح *pRc){
  حرف *z_اندر = *pzStr;
  اگر( !zAppend && doFree && *pRc==SQLITE_OK ){
    *pRc = SQLITE_NOMEM;
  }
  اگر( *pRc!=SQLITE_OK ){
    sqlite3_free(z_اندر);
    z_اندر = 0;
  }ورنہ{
    اگر( z_اندر ){
      حرف *z_عارضی = z_اندر;
      z_اندر = sqlite3_mprintf("%s%s", z_اندر, zAppend);
      sqlite3_free(z_عارضی);
    }ورنہ{
      z_اندر = sqlite3_mprintf("%s", zAppend);
    }
    اگر( !z_اندر ){
      *pRc = SQLITE_NOMEM;
    }
  }
  *pzStr = z_اندر;
  اگر( doFree ){
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
ساکن حرف *echoSelectList(echo_vtab *pTab, sqlite3_اشاریہ_info *pIdxInfo){
  حرف *zRet = 0;
  اگر( sqlite3_libversion_نمبر()<3010000 ){
    zRet = sqlite3_mprintf(", *");
  }ورنہ{
    صحیح i;
    برائے(i=0; i<pTab->n_ستون; i++){
      اگر( pIdxInfo->ستون_مستعمل & ((sqlite3_uint64)1 << (i>=63 ? 63 : i)) ){
        zRet = sqlite3_mprintf("%z, %s", zRet, pTab->a_ستون[i]);
      }ورنہ{
        zRet = sqlite3_mprintf("%z, NULL", zRet);
      }
      اگر( !zRet ) توڑو;
    }
  }
  واپس zRet;
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
ساکن صحیح echo_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحیح ii;
  حرف *zQuery = 0;
  حرف *z_ستون = 0;
  حرف *zNew;
  صحیح nArg = 0;
  ثابت حرف *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  صحیح n_صف = 0;
  صحیح useIdx = 0;
  صحیح rc = SQLITE_OK;
  صحیح use_لاگت = 0;
  دوہرا لاگت = 0;
  صحیح isIgnoreUsable = 0;
  اگر( Tcl_GetVar(interp, "echo_module_ignore_usable", TCL_GLOBAL_ONLY) ){
    isIgnoreUsable = 1;
  }

  اگر( simulateVtabError(pVtab, "xBestIndex") ){
    واپس SQLITE_ERROR;
  }

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  اگر( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    لاگت = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    use_لاگت = 1;
  } ورنہ {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->z_جدول_Name);
    اگر( !zQuery ){
      واپس SQLITE_NOMEM;
    }
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    sqlite3_قدم(pStmt);
    n_صف = sqlite3_عمود_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
  }

  z_ستون = echoSelectList(pVtab, pIdxInfo);
  اگر( !z_ستون ) واپس SQLITE_NOMEM;
  zQuery = sqlite3_mprintf("SELECT rowid%z FROM %Q", z_ستون, pVtab->z_جدول_Name);
  اگر( !zQuery ) واپس SQLITE_NOMEM;

  برائے(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ثابت ساخت sqlite3_اشاریہ_constraint *pConstraint;
    ساخت sqlite3_اشاریہ_constraint_usage *pUsage;
    صحیح i_ستون;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    اگر( !isIgnoreUsable && !pConstraint->usable ) جاری;

    i_ستون = pConstraint->i_عمود;
    اگر( i_ستون<0 || pVtab->a_اشاریہ[i_ستون] ){
      حرف *z_New_ستون = i_ستون>=0 ? pVtab->a_ستون[i_ستون] : "rowid";
      حرف *zOp = 0;
      useIdx = 1;
      انتخاب( pConstraint->op ){
        حالت SQLITE_اشاریہ_CONSTRAINT_EQ:
          zOp = "="; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_LT:
          zOp = "<"; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_GT:
          zOp = ">"; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_LE:
          zOp = "<="; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_GE:
          zOp = ">="; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_مطابقت:
          /* Purposely translate the MATCH operator into a LIKE, which
          ** will be used by the next block of code to construct a new
          ** query.  It should also be noted here that the next block
          ** of code requires the first letter of this operator to be
          ** in upper-case to trigger the special MATCH handling (i.e.
          ** wrapping the bound parameter with literal '%'s).
          */
          zOp = "LIKE"; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_LIKE:
          zOp = "like"; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_GLOB:
          zOp = "glob"; توڑو;
        حالت SQLITE_اشاریہ_CONSTRAINT_REGEXP:
          zOp = "regexp"; توڑو;
      }
      اگر( zOp ){
        اگر( zOp[0]=='L' ){
          zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
              zSep, z_New_ستون);
        } ورنہ {
          zNew = sqlite3_mprintf(" %s %s %s ?", zSep, z_New_ستون, zOp);
        }
        string_concat(&zQuery, zNew, 1, &rc);
        zSep = "AND";
        pUsage->argv_اشاریہ = ++nArg;
        pUsage->omit = 1;
      }
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  اگر( pIdxInfo->n_ترتیب_By==1 && (
        pIdxInfo->a_ترتیب_By->i_عمود<0 ||
        pVtab->a_اشاریہ[pIdxInfo->a_ترتیب_By->i_عمود]) ){
    صحیح i_ستون = pIdxInfo->a_ترتیب_By->i_عمود;
    حرف *z_New_ستون = i_ستون>=0 ? pVtab->a_ستون[i_ستون] : "rowid";
    حرف *zDir = pIdxInfo->a_ترتیب_By->desc?"DESC":"ASC";
    zNew = sqlite3_mprintf(" ORDER BY %s %s", z_New_ستون, zDir);
    string_concat(&zQuery, zNew, 1, &rc);
    pIdxInfo->ترتیب_By_Consumed = 1;
  }

  append_مقصد_Echo_Module(pVtab->interp, "xBestIndex");;
  append_مقصد_Echo_Module(pVtab->interp, zQuery);

  اگر( !zQuery ){
    واپس rc;
  }
  pIdxInfo->idx_عدد = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->need_مقصد_Free_Idx_Str = 1;
  اگر( use_لاگت ){
    pIdxInfo->estimated_لاگت = لاگت;
  }ورنہ اگر( useIdx ){
    /* Approximation of log2(nRow). */
    برائے( ii=0; ii<(حجم(صحیح)*8)-1; ii++ ){
      اگر( n_صف & (1<<ii) ){
        pIdxInfo->estimated_لاگت = (دوہرا)ii;
      }
    }
  }ورنہ{
    pIdxInfo->estimated_لاگت = (دوہرا)n_صف;
  }
  واپس rc;
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
صحیح echoUpdate(
  sqlite3_vtab *tab, 
  صحیح n_ڈیٹا, 
  sqlite3_قدر **ap_ڈیٹا, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  صحیح rc = SQLITE_OK;

  sqlite3_stmt *pStmt = 0;
  حرف *z = 0;               /* SQL statement to execute */
  صحیح bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  صحیح bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  صحیح i;                     /* Counter variable used by for loops */

  أكد( n_ڈیٹا==pVtab->n_ستون+2 || n_ڈیٹا==1 );

  /* Ticket #3083 - make sure we always start a transaction prior to
  ** making any changes to a virtual table */
  أكد( pVtab->اندر_Transaction );

  اگر( simulateVtabError(pVtab, "xUpdate") ){
    واپس SQLITE_ERROR;
  }

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  اگر( n_ڈیٹا>1 && sqlite3_قدر_type(ap_ڈیٹا[0])==SQLITE_INTEGER ){
    حرف *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->z_جدول_Name);
    اگر( !z ){
      rc = SQLITE_NOMEM;
    }

    bindArgOne = (ap_ڈیٹا[1] && sqlite3_قدر_type(ap_ڈیٹا[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    اگر( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0, &rc);
       zSep = ",";
    }
    برائے(i=2; i<n_ڈیٹا; i++){
      اگر( ap_ڈیٹا[i]==0 ) جاری;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->a_ستون[i-2], i), 1, &rc);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", n_ڈیٹا), 1, &rc);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  ورنہ اگر( n_ڈیٹا==1 && sqlite3_قدر_type(ap_ڈیٹا[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->z_جدول_Name);
    اگر( !z ){
      rc = SQLITE_NOMEM;
    }
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  ورنہ اگر( n_ڈیٹا>2 && sqlite3_قدر_type(ap_ڈیٹا[0])==SQLITE_NULL ){
    صحیح ii;
    حرف *z_داخل_کرو = 0;
    حرف *z_اقدار = 0;
  
    z_داخل_کرو = sqlite3_mprintf("INSERT INTO %Q (", pVtab->z_جدول_Name);
    اگر( !z_داخل_کرو ){
      rc = SQLITE_NOMEM;
    }
    اگر( sqlite3_قدر_type(ap_ڈیٹا[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      z_اقدار = sqlite3_mprintf("?");
      string_concat(&z_داخل_کرو, "rowid", 0, &rc);
    }

    أكد((pVtab->n_ستون+2)==n_ڈیٹا);
    برائے(ii=2; ii<n_ڈیٹا; ii++){
      string_concat(&z_داخل_کرو, 
          sqlite3_mprintf("%s%Q", z_اقدار?", ":"", pVtab->a_ستون[ii-2]), 1, &rc);
      string_concat(&z_اقدار, 
          sqlite3_mprintf("%s?%d", z_اقدار?", ":"", ii), 1, &rc);
    }

    string_concat(&z, z_داخل_کرو, 1, &rc);
    string_concat(&z, ") VALUES(", 0, &rc);
    string_concat(&z, z_اقدار, 1, &rc);
    string_concat(&z, ")", 0, &rc);
  }

  /* Anything else is an error */
  ورنہ{
    أكد(0);
    واپس SQLITE_ERROR;
  }

  اگر( rc==SQLITE_OK ){
    rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  }
  أكد( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  اگر( rc==SQLITE_OK ) {
    اگر( bindArgZero ){
      sqlite3_bind_قدر(pStmt, n_ڈیٹا, ap_ڈیٹا[0]);
    }
    اگر( bindArgOne ){
      sqlite3_bind_قدر(pStmt, 1, ap_ڈیٹا[1]);
    }
    برائے(i=2; i<n_ڈیٹا && rc==SQLITE_OK; i++){
      اگر( ap_ڈیٹا[i] ) rc = sqlite3_bind_قدر(pStmt, i, ap_ڈیٹا[i]);
    }
    اگر( rc==SQLITE_OK ){
      sqlite3_قدم(pStmt);
      rc = sqlite3_finalize(pStmt);
    }ورنہ{
      sqlite3_finalize(pStmt);
    }
  }

  اگر( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_آخری_داخل_کرو_rowid(db);
  }
  اگر( rc!=SQLITE_OK ){
    tab->z_Err_پیغام = sqlite3_mprintf("echo-vtab-error: %s", sqlite3_errmsg(db));
  }

  واپس rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
ساکن صحیح echoTransactionCall(sqlite3_vtab *tab, ثابت حرف *zCall){
  حرف *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->z_جدول_Name);
  اگر( z==0 ) واپس SQLITE_NOMEM;
  append_مقصد_Echo_Module(pVtab->interp, zCall);
  append_مقصد_Echo_Module(pVtab->interp, z);
  sqlite3_free(z);
  واپس SQLITE_OK;
}
ساکن صحیح echoBegin(sqlite3_vtab *tab){
  صحیح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت حرف *zVal; 

  /* Ticket #3083 - do not start a transaction if we are already in
  ** a transaction */
  أكد( !pVtab->اندر_Transaction );

  اگر( simulateVtabError(pVtab, "xBegin") ){
    واپس SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xBegin");

  اگر( rc==SQLITE_OK ){
    /* Check if the $::echo_module_begin_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
    اگر( zVal && 0==strcmp(zVal, pVtab->z_جدول_Name) ){
      rc = SQLITE_ERROR;
    }
  }
  اگر( rc==SQLITE_OK ){
    pVtab->اندر_Transaction = 1;
  }
  واپس rc;
}
ساکن صحیح echoSync(sqlite3_vtab *tab){
  صحیح rc;
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  ثابت حرف *zVal; 

  /* Ticket #3083 - Only call xSync if we have previously started a
  ** transaction */
  أكد( pVtab->اندر_Transaction );

  اگر( simulateVtabError(pVtab, "xSync") ){
    واپس SQLITE_ERROR;
  }

  rc = echoTransactionCall(tab, "xSync");

  اگر( rc==SQLITE_OK ){
    /* Check if the $::echo_module_sync_fail variable is defined. If it is,
    ** and it is set to the name of the real table underlying this virtual
    ** echo module table, then cause this xSync operation to fail.
    */
    zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
    اگر( zVal && 0==strcmp(zVal, pVtab->z_جدول_Name) ){
      rc = -1;
    }
  }
  واپس rc;
}
ساکن صحیح echoCommit(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab*)tab;
  صحیح rc;

  /* Ticket #3083 - Only call xCommit if we have previously started
  ** a transaction */
  أكد( pVtab->اندر_Transaction );

  اگر( simulateVtabError(pVtab, "xCommit") ){
    واپس SQLITE_ERROR;
  }

  sqlite3BeginBenignMalloc();
  rc = echoTransactionCall(tab, "xCommit");
  sqlite3_انتہا_Benign_Malloc();
  pVtab->اندر_Transaction = 0;
  واپس rc;
}
ساکن صحیح echoRollback(sqlite3_vtab *tab){
  صحیح rc;
  echo_vtab *pVtab = (echo_vtab*)tab;

  /* Ticket #3083 - Only call xRollback if we have previously started
  ** a transaction */
  أكد( pVtab->اندر_Transaction );

  rc = echoTransactionCall(tab, "xRollback");
  pVtab->اندر_Transaction = 0;
  واپس rc;
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
ساکن خالی overloadedGlobFunction(
  sqlite3_context *pContext,
  صحیح nArg,
  sqlite3_قدر **apArg
){
  Tcl_Interp *interp = sqlite3_user_ڈیٹا(pContext);
  Tcl_DString str;
  صحیح i;
  صحیح rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  برائے(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (حرف*)sqlite3_قدر_متن(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DString_قدر(&str));
  Tcl_DStringFree(&str);
  اگر( rc ){
    sqlite3_نتیجہ_error(pContext, Tcl_Get_String_نتیجہ(interp), -1);
  }ورنہ{
    sqlite3_نتیجہ_متن(pContext, Tcl_Get_String_نتیجہ(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_Reset_نتیجہ(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
ساکن صحیح echo_ڈھونڈو_Function(
  sqlite3_vtab *vtab,
  صحیح nArg,
  ثابت حرف *zFuncName,
  خالی (**pxFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),
  خالی **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  اگر( strcmp(zFuncName,"glob")!=0 ){
    واپس 0;
  }
  اگر( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    واپس 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  واپس 1;
}

ساکن صحیح echoRename(sqlite3_vtab *vtab, ثابت حرف *zNewName){
  صحیح rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)vtab;

  اگر( simulateVtabError(p, "xRename") ){
    واپس SQLITE_ERROR;
  }

  اگر( p->is_نمونہ ){
    صحیح nThis = (صحیح)لڑی_لمبائی(p->zThis);
    حرف *zSql = sqlite3_mprintf("ALTER TABLE %s RENAME TO %s%s", 
        p->z_جدول_Name, zNewName, &p->z_جدول_Name[nThis]
    );
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }

  واپس rc;
}

ساکن صحیح echoSavepoint(sqlite3_vtab *pVTab, صحیح iSavepoint){
  أكد( pVTab );
  واپس SQLITE_OK;
}

ساکن صحیح echoRelease(sqlite3_vtab *pVTab, صحیح iSavepoint){
  أكد( pVTab );
  واپس SQLITE_OK;
}

ساکن صحیح echo_Rollback_مقصد(sqlite3_vtab *pVTab, صحیح iSavepoint){
  أكد( pVTab );
  واپس SQLITE_OK;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
ساکن sqlite3_module echoModule = {
  1,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_بہترین_اشاریہ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_اگلا,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_عمود,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ڈھونڈو_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};

ساکن sqlite3_module echoModuleV2 = {
  2,                         /* iVersion */
  echoCreate,
  echoConnect,
  echo_بہترین_اشاریہ,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echo_اگلا,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echo_عمود,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echo_ڈھونڈو_Function,          /* xFindFunction - function overloading */
  echoRename,                /* xRename - rename the table */
  echoSavepoint,
  echoRelease,
  echo_Rollback_مقصد,
  0,                         /* xShadowName */
  0                          /* xIntegrity  */
};

/*
** Decode a pointer to an sqlite3 object.
*/
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجی ثابت حرف *sqlite3ErrName(صحیح);

ساکن خالی moduleDestroy(خالی *p){
  EchoModule *pMod = (EchoModule*)p;
  sqlite3_create_function(pMod->db, "function_that_does_not_exist_0982ma98",
                          SQLITE_ANY, 1, 0, 0, 0, 0);
  sqlite3_free(p);
}

/*
** Register the echo virtual table module.
*/
ساکن صحیح SQLITE_TCLAPI رجسٹر_echo_module(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحیح rc;
  sqlite3 *db;
  EchoModule *pMod;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;

  /* Virtual table module "echo" */
  pMod = sqlite3_malloc(حجم(EchoModule));
  pMod->interp = interp;
  pMod->db = db;
  rc = sqlite3_create_module_v2(
      db, "echo", &echoModule, (خالی*)pMod, moduleDestroy
  );

  /* Virtual table module "echo_v2" */
  اگر( rc==SQLITE_OK ){
    pMod = sqlite3_malloc(حجم(EchoModule));
    pMod->interp = interp;
    pMod->db = db;
    rc = sqlite3_create_module_v2(db, "echo_v2", 
        &echoModuleV2, (خالی*)pMod, moduleDestroy
    );
  }

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  واپس TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
ساکن صحیح SQLITE_TCLAPI declare_vtab(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  صحیح rc;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB SQL");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3_errmsg(db), TCL_VOLATILE);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

#اگر_ختم /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest8_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "register_echo_module",       رجسٹر_echo_module, 0 },
     { "sqlite3_declare_vtab",       declare_vtab, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
#اگر_ختم
  واپس TCL_OK;
}
