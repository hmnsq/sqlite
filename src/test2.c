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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ctype.h>

ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/*
** Page size and reserved size used for testing.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
സ്ഥിതം ശൂന്യം pager_test_reiniter(DbPage *pNotUsed){
  തിരികെ;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_open(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  പൂർണ്ണസംഖ്യ nPage;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &nPage) ) തിരികെ TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_close(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_rollback(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_commit(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_stmt_begin(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_stmt_rollback(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_stmt_commit(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_stats(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ i, *a;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  വേണ്ടി(i=0; i<9; i++){
    സ്ഥിതം അക്ഷരം *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    അക്ഷരം zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(വലുപ്പം(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_pagecount(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  അക്ഷരം zBuf[100];
  പൂർണ്ണസംഖ്യ nPage;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI page_get(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  പൂർണ്ണസംഖ്യ pgno;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &pgno) ) തിരികെ TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI page_lookup(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  പൂർണ്ണസംഖ്യ pgno;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &pgno) ) തിരികെ TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  എങ്കിൽ( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ശൂന്യ);
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI pager_truncate(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Pager *pPager;
  പൂർണ്ണസംഖ്യ pgno;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &pgno) ) തിരികെ TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  തിരികെ TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI page_unref(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  DbPage *pPage;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  തിരികെ TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI page_read(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  അക്ഷരം zBuf[100];
  DbPage *pPage;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), വലുപ്പം(zBuf));
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI page_സംഖ്യ(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  അക്ഷരം zBuf[100];
  DbPage *pPage;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI page_write(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  DbPage *pPage;
  അക്ഷരം *pData;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI fake_big_file(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ n;
  i64 offset;
  അക്ഷരം *zFile;
  പൂർണ്ണസംഖ്യ nFile;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[1], &n) ) തിരികെ TCL_ERROR;
#എങ്കിൽ defined(_WIN32)
  എങ്കിൽ( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (ശൂന്യം*)0);
    തിരികെ TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (പൂർണ്ണസംഖ്യ)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  എങ്കിൽ( zFile==0 ) തിരികെ TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), ശൂന്യ);
    sqlite3_free(zFile);
    തിരികെ TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI testPendingByte(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ pbyte;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (ശൂന്യം*)0);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[1], &pbyte) ) തിരികെ TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
സ്ഥിതം Tcl_Interp *faultSimInterp = 0;
സ്ഥിതം പൂർണ്ണസംഖ്യ faultSimScriptSize = 0;
സ്ഥിതം അക്ഷരം *faultSimScript;
സ്ഥിതം പൂർണ്ണസംഖ്യ faultSimCallback(പൂർണ്ണസംഖ്യ x){
  അക്ഷരം zInt[30];
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ isNeg;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }അല്ലെങ്കിൽ{
    /* Convert x to text without using any sqlite3 routines */
    എങ്കിൽ( x<0 ){
      isNeg = 1;
      x = -x;
    }അല്ലെങ്കിൽ{
      isNeg = 0;
    }
    zInt[വലുപ്പം(zInt)-1] = 0;
    വേണ്ടി(i=വലുപ്പം(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    എങ്കിൽ( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, വലുപ്പം(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  എങ്കിൽ( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  തിരികെ rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI faultInstallCmd(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  സ്ഥിരം അക്ഷരം *zScript;
  പൂർണ്ണസംഖ്യ nScript;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (ശൂന്യം*)0);
    തിരികെ SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (പൂർണ്ണസംഖ്യ)strlen(zScript);
  എങ്കിൽ( faultSimScript ){
    സ്വതന്ത്രം(faultSimScript);
    faultSimScript = 0;
  }
  എങ്കിൽ( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }അല്ലെങ്കിൽ{
    faultSimScript = malloc( nScript+100 );
    എങ്കിൽ( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (ശൂന്യം*)0);
      തിരികെ SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI testBitvecBuiltinTest(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ sz, rc;
  പൂർണ്ണസംഖ്യ nProg = 0;
  പൂർണ്ണസംഖ്യ aProg[100];
  സ്ഥിരം അക്ഷരം *z;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (ശൂന്യം*)0);
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[1], &sz) ) തിരികെ TCL_ERROR;
  z = argv[2];
  ആകുമ്പോൾ( nProg<99 && *z ){
    ആകുമ്പോൾ( *z && !sqlite3Isdigit(*z) ){ z++; }
    എങ്കിൽ( *z==0 ) നിർത്തുക;
    aProg[nProg++] = atoi(z);
    ആകുമ്പോൾ( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest2_Init(Tcl_Interp *interp){
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_io_error_persist;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_io_error_pending;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_io_error_hit;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_io_error_hardhit;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_diskfull_pending;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_diskfull;
  സ്ഥിതം ഘടന {
    അക്ഷരം *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_സംഖ്യ         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (അക്ഷരം*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (അക്ഷരം*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (അക്ഷരം*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (അക്ഷരം*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (അക്ഷരം*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (അക്ഷരം*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (അക്ഷരം*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  തിരികെ TCL_OK;
}
