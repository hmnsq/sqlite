/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the pager.c module in SQLite.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/*
** Page size and reserved size used for testing.
*/
ਸਥਿਰ ਪੂਰਨ test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ਸਥਿਰ ਖਾਲੀ pager_test_reiniter(Db_ਪੰਨਾ *p_Not_ਵਰਤਿਆ){
  ਵਾਪਸ;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_open(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  u32 ਪੰਨਾ_ਮਾਪ;
  Pager *pPager;
  ਪੂਰਨ n_ਪੰਨਾ;
  ਪੂਰਨ rc;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetInt(interp, argv[2], &n_ਪੰਨਾ) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_ਲੱਭੋ(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_Pager_ਸੈੱਟ_Cachesize(pPager, n_ਪੰਨਾ);
  ਪੰਨਾ_ਮਾਪ = test_pagesize;
  sqlite3_Pager_ਸੈੱਟ_Pagesize(pPager, &ਪੰਨਾ_ਮਾਪ, -1);
  Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(pPager), ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_close(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_rollback(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_commit(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_stmt_begin(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_stmt_rollback(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_stmt_commit(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_stats(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ i, *a;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  a = sqlite3PagerStats(pPager);
  ਲਈ(i=0; i<9; i++){
    ਸਥਿਰ ਅੱਖਰ *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    ਅੱਖਰ zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_pagecount(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਅੱਖਰ zBuf[100];
  ਪੂਰਨ n_ਪੰਨਾ;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3PagerPagecount(pPager, &n_ਪੰਨਾ);
  sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%d", n_ਪੰਨਾ);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਪੰਨਾ_get(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_ਪੰਨਾ *p_ਪੰਨਾ = 0;
  ਪੂਰਨ pgno;
  ਪੂਰਨ rc;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  ਜੇ( Tcl_GetInt(interp, argv[2], &pgno) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_Pager_Shared_ਤਾਲਾ(pPager);
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &p_ਪੰਨਾ, 0);
  }
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(p_ਪੰਨਾ), ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਪੰਨਾ_lookup(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_ਪੰਨਾ *p_ਪੰਨਾ;
  ਪੂਰਨ pgno;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  ਜੇ( Tcl_GetInt(interp, argv[2], &pgno) ) ਵਾਪਸ TCL_ERROR;
  p_ਪੰਨਾ = sqlite3PagerLookup(pPager, pgno);
  ਜੇ( p_ਪੰਨਾ ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(p_ਪੰਨਾ), ਨਿਰਾ);
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI pager_truncate(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Pager *pPager;
  ਪੂਰਨ pgno;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pPager = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  ਜੇ( Tcl_GetInt(interp, argv[2], &pgno) ) ਵਾਪਸ TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  ਵਾਪਸ TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਪੰਨਾ_unref(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Db_ਪੰਨਾ *p_ਪੰਨਾ;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  p_ਪੰਨਾ = (Db_ਪੰਨਾ *)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3PagerUnref(p_ਪੰਨਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਪੰਨਾ_read(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਅੱਖਰ zBuf[100];
  Db_ਪੰਨਾ *p_ਪੰਨਾ;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  p_ਪੰਨਾ = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ), ਆਕਾਰ(zBuf));
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਪੰਨਾ_ਸੰਖਿਆ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਅੱਖਰ zBuf[100];
  Db_ਪੰਨਾ *p_ਪੰਨਾ;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  p_ਪੰਨਾ = (Db_ਪੰਨਾ *)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%d", sqlite3PagerPagenumber(p_ਪੰਨਾ));
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਪੰਨਾ_write(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Db_ਪੰਨਾ *p_ਪੰਨਾ;
  ਅੱਖਰ *p_ਅੰਕੜੇ;
  ਪੂਰਨ rc;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  p_ਪੰਨਾ = (Db_ਪੰਨਾ *)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3PagerWrite(p_ਪੰਨਾ);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  p_ਅੰਕੜੇ = sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ);
  strncpy(p_ਅੰਕੜੇ, argv[2], test_pagesize-1);
  p_ਅੰਕੜੇ[test_pagesize-1] = 0;
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DISKIO
/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI fake_big_file(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  ਪੂਰਨ rc;
  ਪੂਰਨ n;
  i64 ਵਿੱਥ;
  ਅੱਖਰ *zFile;
  ਪੂਰਨ nFile;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetInt(interp, argv[1], &n) ) ਵਾਪਸ TCL_ERROR;
#ਜੇ defined(_WIN32)
  ਜੇ( n>2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (ਖਾਲੀ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
#ਜੇ_ਅੰਤ

  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  nFile = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  ਜੇ( zFile==0 ) ਵਾਪਸ TCL_ERROR;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, "open failed: ", sqlite3ErrName(rc), ਨਿਰਾ);
    sqlite3_free(zFile);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਿੱਥ = n;
  ਵਿੱਥ *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, ਵਿੱਥ);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, "write failed: ", sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI testPendingByte(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ pbyte;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (ਖਾਲੀ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetInt(interp, argv[1], &pbyte) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ਸਥਿਰ Tcl_Interp *faultSimInterp = 0;
ਸਥਿਰ ਪੂਰਨ fault_Sim_Script_ਮਾਪ = 0;
ਸਥਿਰ ਅੱਖਰ *faultSimScript;
ਸਥਿਰ ਪੂਰਨ faultSimCallback(ਪੂਰਨ x){
  ਅੱਖਰ zInt[30];
  ਪੂਰਨ i;
  ਪੂਰਨ isNeg;
  ਪੂਰਨ rc;
  ਜੇ( x==0 ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(faultSimScript+fault_Sim_Script_ਮਾਪ, "0", 2);
  }ਨਹੀਂਤਾਂ{
    /* Convert x to text without using any sqlite3 routines */
    ਜੇ( x<0 ){
      isNeg = 1;
      x = -x;
    }ਨਹੀਂਤਾਂ{
      isNeg = 0;
    }
    zInt[ਆਕਾਰ(zInt)-1] = 0;
    ਲਈ(i=ਆਕਾਰ(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    ਜੇ( isNeg ) zInt[i--] = '-';
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(faultSimScript+fault_Sim_Script_ਮਾਪ, zInt+i+1, ਆਕਾਰ(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  ਜੇ( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    rc = atoi(Tcl_Get_String_ਨਤੀਜਾ(faultSimInterp));
  }
  Tcl_Reset_ਨਤੀਜਾ(faultSimInterp);
  ਵਾਪਸ rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI faultInstallCmd(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਅਚਲ ਅੱਖਰ *zScript;
  ਪੂਰਨ nScript;
  ਪੂਰਨ rc;
  ਜੇ( argc!=1 && argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (ਖਾਲੀ*)0);
    ਵਾਪਸ SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zScript);
  ਜੇ( faultSimScript ){
    ਮੁਕਤ(faultSimScript);
    faultSimScript = 0;
  }
  ਜੇ( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }ਨਹੀਂਤਾਂ{
    faultSimScript = ਸਮ੍ਰਿਤੀ_ਦਿਓ( nScript+100 );
    ਜੇ( faultSimScript==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "out of memory", (ਖਾਲੀ*)0);
      ਵਾਪਸ SQLITE_ERROR;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_ਮਾਪ = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI testBitvecBuiltinTest(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ sz, rc;
  ਪੂਰਨ nProg = 0;
  ਪੂਰਨ aProg[100];
  ਅਚਲ ਅੱਖਰ *z;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (ਖਾਲੀ*)0);
  }
  ਜੇ( Tcl_GetInt(interp, argv[1], &sz) ) ਵਾਪਸ TCL_ERROR;
  z = argv[2];
  ਜਦੋਂ( nProg<99 && *z ){
    ਜਦੋਂ( *z && !sqlite3Isdigit(*z) ){ z++; }
    ਜੇ( *z==0 ) ਰੋਕੋ;
    aProg[nProg++] = atoi(z);
    ਜਦੋਂ( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest2_Init(Tcl_Interp *interp){
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_persist;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_pending;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_ਪ੍ਰਾਪਤੀ;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_hardhit;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_diskfull_pending;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_diskfull;
  ਸਥਿਰ ਬਣਤਰ {
    ਅੱਖਰ *zName;
    Tcl_CmdProc *xProc;
  } aCmd[] = {
    { "pager_open",              (Tcl_CmdProc*)pager_open          },
    { "pager_close",             (Tcl_CmdProc*)pager_close         },
    { "pager_commit",            (Tcl_CmdProc*)pager_commit        },
    { "pager_rollback",          (Tcl_CmdProc*)pager_rollback      },
    { "pager_stmt_begin",        (Tcl_CmdProc*)pager_stmt_begin    },
    { "pager_stmt_commit",       (Tcl_CmdProc*)pager_stmt_commit   },
    { "pager_stmt_rollback",     (Tcl_CmdProc*)pager_stmt_rollback },
    { "pager_stats",             (Tcl_CmdProc*)pager_stats         },
    { "pager_pagecount",         (Tcl_CmdProc*)pager_pagecount     },
    { "page_get",                (Tcl_CmdProc*)ਪੰਨਾ_get            },
    { "page_lookup",             (Tcl_CmdProc*)ਪੰਨਾ_lookup         },
    { "page_unref",              (Tcl_CmdProc*)ਪੰਨਾ_unref          },
    { "page_read",               (Tcl_CmdProc*)ਪੰਨਾ_read           },
    { "page_write",              (Tcl_CmdProc*)ਪੰਨਾ_write          },
    { "page_number",             (Tcl_CmdProc*)ਪੰਨਾ_ਸੰਖਿਆ         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#ਜੇ_ਅੰਤ
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (ਅੱਖਰ*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (ਅੱਖਰ*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (ਅੱਖਰ*)&sqlite3_io_error_ਪ੍ਰਾਪਤੀ, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (ਅੱਖਰ*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (ਅੱਖਰ*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (ਅੱਖਰ*)&sqlite3_diskfull, TCL_LINK_INT);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (ਅੱਖਰ*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}
