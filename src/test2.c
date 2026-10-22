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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Page size and reserved size used for testing.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ସ୍ଥିର ଶୂନ୍ୟ pager_test_reiniter(DbPage *pNotUsed){
  ଫେରନ୍ତୁ;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_open(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPage;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetInt(interp, argv[2], &nPage) ) ଫେରନ୍ତୁ TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_close(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_rollback(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_commit(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_stmt_begin(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_stmt_rollback(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_stmt_commit(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_stats(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, *a;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  ପାଇଁ(i=0; i<9; i++){
    ସ୍ଥିର ଅକ୍ଷର *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    ଅକ୍ଷର zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ଆକାର(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_pagecount(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ଅକ୍ଷର zBuf[100];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPage;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI page_get(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgno;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  ଯଦି( Tcl_GetInt(interp, argv[2], &pgno) ) ଫେରନ୍ତୁ TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI page_lookup(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgno;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  ଯଦି( Tcl_GetInt(interp, argv[2], &pgno) ) ଫେରନ୍ତୁ TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  ଯଦି( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ଖାଲି);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI pager_truncate(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgno;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  ଯଦି( Tcl_GetInt(interp, argv[2], &pgno) ) ଫେରନ୍ତୁ TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI page_unref(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  DbPage *pPage;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI page_read(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ଅକ୍ଷର zBuf[100];
  DbPage *pPage;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), ଆକାର(zBuf));
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI page_ସଂଖ୍ୟା(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ଅକ୍ଷର zBuf[100];
  DbPage *pPage;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI page_write(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  DbPage *pPage;
  ଅକ୍ଷର *pData;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI fake_big_file(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  i64 offset;
  ଅକ୍ଷର *zFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFile;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetInt(interp, argv[1], &n) ) ଫେରନ୍ତୁ TCL_ERROR;
#ଯଦି defined(_WIN32)
  ଯଦି( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (ଶୂନ୍ୟ*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  ଯଦି( zFile==0 ) ଫେରନ୍ତୁ TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  ଯଦି( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), ଖାଲି);
    sqlite3_free(zFile);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  ଯଦି( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI testPendingByte(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pbyte;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (ଶୂନ୍ୟ*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetInt(interp, argv[1], &pbyte) ) ଫେରନ୍ତୁ TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ସ୍ଥିର Tcl_Interp *faultSimInterp = 0;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultSimScriptSize = 0;
ସ୍ଥିର ଅକ୍ଷର *faultSimScript;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultSimCallback(ପୂର୍ଣ୍ଣସଂଖ୍ୟା x){
  ଅକ୍ଷର zInt[30];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNeg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }ଅନ୍ୟଥା{
    /* Convert x to text without using any sqlite3 routines */
    ଯଦି( x<0 ){
      isNeg = 1;
      x = -x;
    }ଅନ୍ୟଥା{
      isNeg = 0;
    }
    zInt[ଆକାର(zInt)-1] = 0;
    ପାଇଁ(i=ଆକାର(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    ଯଦି( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, ଆକାର(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  ଯଦି( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  ଫେରନ୍ତୁ rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI faultInstallCmd(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ଅଚଳ ଅକ୍ଷର *zScript;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nScript;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (ଶୂନ୍ୟ*)0);
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zScript);
  ଯଦି( faultSimScript ){
    ମୁକ୍ତ(faultSimScript);
    faultSimScript = 0;
  }
  ଯଦି( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }ଅନ୍ୟଥା{
    faultSimScript = malloc( nScript+100 );
    ଯଦି( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (ଶୂନ୍ୟ*)0);
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI testBitvecBuiltinTest(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz, rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nProg = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା aProg[100];
  ଅଚଳ ଅକ୍ଷର *z;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (ଶୂନ୍ୟ*)0);
  }
  ଯଦି( Tcl_GetInt(interp, argv[1], &sz) ) ଫେରନ୍ତୁ TCL_ERROR;
  z = argv[2];
  ଯେତେବେଳେ( nProg<99 && *z ){
    ଯେତେବେଳେ( *z && !sqlite3Isdigit(*z) ){ z++; }
    ଯଦି( *z==0 ) ଭାଙ୍ଗ;
    aProg[nProg++] = atoi(z);
    ଯେତେବେଳେ( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest2_Init(Tcl_Interp *interp){
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_persist;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_pending;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_hit;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_hardhit;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_diskfull_pending;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_diskfull;
  ସ୍ଥିର ସଂରଚନା {
    ଅକ୍ଷର *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_ସଂଖ୍ୟା         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (ଅକ୍ଷର*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (ଅକ୍ଷର*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (ଅକ୍ଷର*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (ଅକ୍ଷର*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (ଅକ୍ଷର*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (ଅକ୍ଷର*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (ଅକ୍ଷର*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  ଫେରନ୍ତୁ TCL_OK;
}
