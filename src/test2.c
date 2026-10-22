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

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/*
** Page size and reserved size used for testing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ಸ್ಥಿರ ಶೂನ್ಯ pager_test_reiniter(DbPage *pNotUsed){
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_open(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ nPage;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetInt(interp, argv[2], &nPage) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_close(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_rollback(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_commit(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_stmt_begin(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_stmt_rollback(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_stmt_commit(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_stats(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ i, *a;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  ಪ್ರತಿ(i=0; i<9; i++){
    ಸ್ಥಿರ ವರ್ಣ *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    ವರ್ಣ zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_pagecount(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ವರ್ಣ zBuf[100];
  ಪೂರ್ಣಾಙ್ಕ nPage;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI page_get(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  ಪೂರ್ಣಾಙ್ಕ pgno;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  ಯದಿ( Tcl_GetInt(interp, argv[2], &pgno) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI page_lookup(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  ಪೂರ್ಣಾಙ್ಕ pgno;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  ಯದಿ( Tcl_GetInt(interp, argv[2], &pgno) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  ಯದಿ( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ರಿಕ್ತ);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI pager_truncate(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Pager *pPager;
  ಪೂರ್ಣಾಙ್ಕ pgno;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  ಯದಿ( Tcl_GetInt(interp, argv[2], &pgno) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI page_unref(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  DbPage *pPage;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI page_read(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ವರ್ಣ zBuf[100];
  DbPage *pPage;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), ಪರಿಮಾಣ(zBuf));
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI page_ಸಂಖ್ಯೆ(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ವರ್ಣ zBuf[100];
  DbPage *pPage;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI page_write(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  DbPage *pPage;
  ವರ್ಣ *pData;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#ifndef SQLITE_OMIT_DISKIO
/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI fake_big_file(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ n;
  i64 offset;
  ವರ್ಣ *zFile;
  ಪೂರ್ಣಾಙ್ಕ nFile;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetInt(interp, argv[1], &n) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ಯದಿ defined(_WIN32)
  ಯದಿ( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (ಶೂನ್ಯ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (ಪೂರ್ಣಾಙ್ಕ)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  ಯದಿ( zFile==0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), ರಿಕ್ತ);
    sqlite3_free(zFile);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI testPendingByte(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ pbyte;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (ಶೂನ್ಯ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetInt(interp, argv[1], &pbyte) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ಸ್ಥಿರ Tcl_Interp *faultSimInterp = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultSimScriptSize = 0;
ಸ್ಥಿರ ವರ್ಣ *faultSimScript;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultSimCallback(ಪೂರ್ಣಾಙ್ಕ x){
  ವರ್ಣ zInt[30];
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ isNeg;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }ಅನ್ಯಥಾ{
    /* Convert x to text without using any sqlite3 routines */
    ಯದಿ( x<0 ){
      isNeg = 1;
      x = -x;
    }ಅನ್ಯಥಾ{
      isNeg = 0;
    }
    zInt[ಪರಿಮಾಣ(zInt)-1] = 0;
    ಪ್ರತಿ(i=ಪರಿಮಾಣ(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    ಯದಿ( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, ಪರಿಮಾಣ(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  ಯದಿ( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI faultInstallCmd(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಧ್ರುವ ವರ್ಣ *zScript;
  ಪೂರ್ಣಾಙ್ಕ nScript;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (ಶೂನ್ಯ*)0);
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (ಪೂರ್ಣಾಙ್ಕ)strlen(zScript);
  ಯದಿ( faultSimScript ){
    ಮುಕ್ತ(faultSimScript);
    faultSimScript = 0;
  }
  ಯದಿ( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }ಅನ್ಯಥಾ{
    faultSimScript = malloc( nScript+100 );
    ಯದಿ( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (ಶೂನ್ಯ*)0);
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI testBitvecBuiltinTest(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ sz, rc;
  ಪೂರ್ಣಾಙ್ಕ nProg = 0;
  ಪೂರ್ಣಾಙ್ಕ aProg[100];
  ಧ್ರುವ ವರ್ಣ *z;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (ಶೂನ್ಯ*)0);
  }
  ಯದಿ( Tcl_GetInt(interp, argv[1], &sz) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  z = argv[2];
  ಯಾವತ್( nProg<99 && *z ){
    ಯಾವತ್( *z && !sqlite3Isdigit(*z) ){ z++; }
    ಯದಿ( *z==0 ) ಭಙ್ಗ;
    aProg[nProg++] = atoi(z);
    ಯಾವತ್( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest2_Init(Tcl_Interp *interp){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_persist;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_pending;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_hit;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_hardhit;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_diskfull_pending;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_diskfull;
  ಸ್ಥಿರ ಸಂರಚನಾ {
    ವರ್ಣ *zName;
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
    { "page_get",                (Tcl_CmdProc*)page_get            },
    { "page_lookup",             (Tcl_CmdProc*)page_lookup         },
    { "page_unref",              (Tcl_CmdProc*)page_unref          },
    { "page_read",               (Tcl_CmdProc*)page_read           },
    { "page_write",              (Tcl_CmdProc*)page_write          },
    { "page_number",             (Tcl_CmdProc*)page_ಸಂಖ್ಯೆ         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (ವರ್ಣ*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (ವರ್ಣ*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (ವರ್ಣ*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (ವರ್ಣ*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (ವರ್ಣ*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (ವರ್ಣ*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (ವರ್ಣ*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
