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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/*
** Page size and reserved size used for testing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ಸ್ಥಿರ ಶೂನ್ಯ pager_test_reiniter(Db_ಪುಟ *p_Not_ಬಳಸಿದ){
  ಹಿಂತಿರುಗಿ;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_open(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  u32 ಪುಟ_ಗಾತ್ರ;
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ n_ಪುಟ;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &n_ಪುಟ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_ಕಂಡುಹಿಡಿ(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_Pager_ಗಣ_Cachesize(pPager, n_ಪುಟ);
  ಪುಟ_ಗಾತ್ರ = test_pagesize;
  sqlite3_Pager_ಗಣ_Pagesize(pPager, &ಪುಟ_ಗಾತ್ರ, -1);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(pPager), ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_close(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_rollback(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_commit(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_stmt_begin(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_stmt_rollback(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_stmt_commit(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_stats(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ i, *a;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  a = sqlite3PagerStats(pPager);
  ಗಾಗಿ(i=0; i<9; i++){
    ಸ್ಥಿರ ಅಕ್ಷರ *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    ಅಕ್ಷರ zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_pagecount(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಅಕ್ಷರ zBuf[100];
  ಪೂರ್ಣಾಂಕ n_ಪುಟ;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3PagerPagecount(pPager, &n_ಪುಟ);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%d", n_ಪುಟ);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ಪುಟ_get(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_ಪುಟ *p_ಪುಟ = 0;
  ಪೂರ್ಣಾಂಕ pgno;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &pgno) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_Pager_Shared_ಬೀಗ(pPager);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &p_ಪುಟ, 0);
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(p_ಪುಟ), ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ಪುಟ_lookup(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_ಪುಟ *p_ಪುಟ;
  ಪೂರ್ಣಾಂಕ pgno;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &pgno) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  p_ಪುಟ = sqlite3PagerLookup(pPager, pgno);
  ಆದರೆ( p_ಪುಟ ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(p_ಪುಟ), ಖಾಲಿ);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI pager_truncate(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ pgno;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pPager = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &pgno) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ಪುಟ_unref(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Db_ಪುಟ *p_ಪುಟ;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p_ಪುಟ = (Db_ಪುಟ *)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3PagerUnref(p_ಪುಟ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ಪುಟ_read(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಅಕ್ಷರ zBuf[100];
  Db_ಪುಟ *p_ಪುಟ;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p_ಪುಟ = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  ಸ್ಮರಣೆ_ನಕಲು(zBuf, sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ), ಗಾತ್ರ(zBuf));
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ಪುಟ_ಸಂಖ್ಯೆ(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಅಕ್ಷರ zBuf[100];
  Db_ಪುಟ *p_ಪುಟ;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p_ಪುಟ = (Db_ಪುಟ *)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%d", sqlite3PagerPagenumber(p_ಪುಟ));
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ಪುಟ_write(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Db_ಪುಟ *p_ಪುಟ;
  ಅಕ್ಷರ *p_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p_ಪುಟ = (Db_ಪುಟ *)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3PagerWrite(p_ಪುಟ);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p_ದತ್ತಾಂಶ = sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ);
  strncpy(p_ದತ್ತಾಂಶ, argv[2], test_pagesize-1);
  p_ದತ್ತಾಂಶ[test_pagesize-1] = 0;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DISKIO
/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI fake_big_file(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ n;
  i64 ಸ್ಥಾನಾಂತರ;
  ಅಕ್ಷರ *zFile;
  ಪೂರ್ಣಾಂಕ nFile;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[1], &n) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಆದರೆ defined(_WIN32)
  ಆದರೆ( n>2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (ಶೂನ್ಯ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
#ಆದರೆ_ಅಂತ್ಯ

  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  nFile = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  ಆದರೆ( zFile==0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಸ್ಮರಣೆ_ನಕಲು(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "open failed: ", sqlite3ErrName(rc), ಖಾಲಿ);
    sqlite3_free(zFile);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಸ್ಥಾನಾಂತರ = n;
  ಸ್ಥಾನಾಂತರ *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, ಸ್ಥಾನಾಂತರ);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "write failed: ", sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI testPendingByte(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ pbyte;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (ಶೂನ್ಯ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[1], &pbyte) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ಸ್ಥಿರ Tcl_Interp *faultSimInterp = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fault_Sim_Script_ಗಾತ್ರ = 0;
ಸ್ಥಿರ ಅಕ್ಷರ *faultSimScript;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ faultSimCallback(ಪೂರ್ಣಾಂಕ x){
  ಅಕ್ಷರ zInt[30];
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ isNeg;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( x==0 ){
    ಸ್ಮರಣೆ_ನಕಲು(faultSimScript+fault_Sim_Script_ಗಾತ್ರ, "0", 2);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Convert x to text without using any sqlite3 routines */
    ಆದರೆ( x<0 ){
      isNeg = 1;
      x = -x;
    }ಇಲ್ಲದಿದ್ದರೆ{
      isNeg = 0;
    }
    zInt[ಗಾತ್ರ(zInt)-1] = 0;
    ಗಾಗಿ(i=ಗಾತ್ರ(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    ಆದರೆ( isNeg ) zInt[i--] = '-';
    ಸ್ಮರಣೆ_ನಕಲು(faultSimScript+fault_Sim_Script_ಗಾತ್ರ, zInt+i+1, ಗಾತ್ರ(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  ಆದರೆ( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = atoi(Tcl_Get_String_ಫಲಿತಾಂಶ(faultSimInterp));
  }
  Tcl_Reset_ಫಲಿತಾಂಶ(faultSimInterp);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI faultInstallCmd(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zScript;
  ಪೂರ್ಣಾಂಕ nScript;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=1 && argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (ಶೂನ್ಯ*)0);
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zScript);
  ಆದರೆ( faultSimScript ){
    ಬಿಡುಗಡೆ(faultSimScript);
    faultSimScript = 0;
  }
  ಆದರೆ( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    faultSimScript = ಸ್ಮರಣೆ_ಕೊಡು( nScript+100 );
    ಆದರೆ( faultSimScript==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "out of memory", (ಶೂನ್ಯ*)0);
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
    ಸ್ಮರಣೆ_ನಕಲು(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_ಗಾತ್ರ = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI testBitvecBuiltinTest(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ sz, rc;
  ಪೂರ್ಣಾಂಕ nProg = 0;
  ಪೂರ್ಣಾಂಕ aProg[100];
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (ಶೂನ್ಯ*)0);
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[1], &sz) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  z = argv[2];
  ಆಗ( nProg<99 && *z ){
    ಆಗ( *z && !sqlite3Isdigit(*z) ){ z++; }
    ಆದರೆ( *z==0 ) ನಿಲ್ಲಿಸು;
    aProg[nProg++] = atoi(z);
    ಆಗ( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest2_Init(Tcl_Interp *interp){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_persist;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_pending;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_ದೊರಕಿತು;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_hardhit;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_diskfull_pending;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_diskfull;
  ಸ್ಥಿರ ರಚನೆ {
    ಅಕ್ಷರ *zName;
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
    { "page_get",                (Tcl_CmdProc*)ಪುಟ_get            },
    { "page_lookup",             (Tcl_CmdProc*)ಪುಟ_lookup         },
    { "page_unref",              (Tcl_CmdProc*)ಪುಟ_unref          },
    { "page_read",               (Tcl_CmdProc*)ಪುಟ_read           },
    { "page_write",              (Tcl_CmdProc*)ಪುಟ_write          },
    { "page_number",             (Tcl_CmdProc*)ಪುಟ_ಸಂಖ್ಯೆ         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#ಆದರೆ_ಅಂತ್ಯ
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (ಅಕ್ಷರ*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (ಅಕ್ಷರ*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (ಅಕ್ಷರ*)&sqlite3_io_error_ದೊರಕಿತು, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (ಅಕ್ಷರ*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (ಅಕ್ಷರ*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (ಅಕ್ಷರ*)&sqlite3_diskfull, TCL_LINK_INT);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (ಅಕ್ಷರ*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}
