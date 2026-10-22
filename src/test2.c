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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <ctype.h>

બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/*
** Page size and reserved size used for testing.
*/
સ્થિર પૂર્ણાંક test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
સ્થિર શૂન્ય pager_test_reiniter(DbPage *pNotUsed){
  પાછા;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_open(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  પૂર્ણાંક nPage;
  પૂર્ણાંક rc;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetInt(interp, argv[2], &nPage) ) પાછા TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_close(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_rollback(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_commit(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_stmt_begin(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_stmt_rollback(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_stmt_commit(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_stats(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક i, *a;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  માટે(i=0; i<9; i++){
    સ્થિર અક્ષર *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    અક્ષર zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(કદ(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_pagecount(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  અક્ષર zBuf[100];
  પૂર્ણાંક nPage;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(કદ(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI page_get(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  પૂર્ણાંક pgno;
  પૂર્ણાંક rc;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  જો( Tcl_GetInt(interp, argv[2], &pgno) ) પાછા TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  જો( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI page_lookup(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  પૂર્ણાંક pgno;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  જો( Tcl_GetInt(interp, argv[2], &pgno) ) પાછા TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  જો( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), ખાલી);
  }
  પાછા TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI pager_truncate(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Pager *pPager;
  પૂર્ણાંક pgno;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  જો( Tcl_GetInt(interp, argv[2], &pgno) ) પાછા TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  પાછા TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI page_unref(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  DbPage *pPage;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  પાછા TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI page_read(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  અક્ષર zBuf[100];
  DbPage *pPage;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), કદ(zBuf));
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI page_સંખ્યા(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  અક્ષર zBuf[100];
  DbPage *pPage;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(કદ(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI page_write(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  DbPage *pPage;
  અક્ષર *pData;
  પૂર્ણાંક rc;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI fake_big_file(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  પૂર્ણાંક rc;
  પૂર્ણાંક n;
  i64 offset;
  અક્ષર *zFile;
  પૂર્ણાંક nFile;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetInt(interp, argv[1], &n) ) પાછા TCL_ERROR;
#જો defined(_WIN32)
  જો( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (શૂન્ય*)0);
    પાછા TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (પૂર્ણાંક)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  જો( zFile==0 ) પાછા TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  જો( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), ખાલી);
    sqlite3_free(zFile);
    પાછા TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  જો( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI testPendingByte(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક pbyte;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (શૂન્ય*)0);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetInt(interp, argv[1], &pbyte) ) પાછા TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
સ્થિર Tcl_Interp *faultSimInterp = 0;
સ્થિર પૂર્ણાંક faultSimScriptSize = 0;
સ્થિર અક્ષર *faultSimScript;
સ્થિર પૂર્ણાંક faultSimCallback(પૂર્ણાંક x){
  અક્ષર zInt[30];
  પૂર્ણાંક i;
  પૂર્ણાંક isNeg;
  પૂર્ણાંક rc;
  જો( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }નહીંતર{
    /* Convert x to text without using any sqlite3 routines */
    જો( x<0 ){
      isNeg = 1;
      x = -x;
    }નહીંતર{
      isNeg = 0;
    }
    zInt[કદ(zInt)-1] = 0;
    માટે(i=કદ(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    જો( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, કદ(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  જો( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }નહીંતર{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  પાછા rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI faultInstallCmd(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  અચલ અક્ષર *zScript;
  પૂર્ણાંક nScript;
  પૂર્ણાંક rc;
  જો( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (શૂન્ય*)0);
    પાછા SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (પૂર્ણાંક)strlen(zScript);
  જો( faultSimScript ){
    મુક્ત(faultSimScript);
    faultSimScript = 0;
  }
  જો( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }નહીંતર{
    faultSimScript = malloc( nScript+100 );
    જો( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (શૂન્ય*)0);
      પાછા SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI testBitvecBuiltinTest(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક sz, rc;
  પૂર્ણાંક nProg = 0;
  પૂર્ણાંક aProg[100];
  અચલ અક્ષર *z;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (શૂન્ય*)0);
  }
  જો( Tcl_GetInt(interp, argv[1], &sz) ) પાછા TCL_ERROR;
  z = argv[2];
  જ્યારે( nProg<99 && *z ){
    જ્યારે( *z && !sqlite3Isdigit(*z) ){ z++; }
    જો( *z==0 ) તોડો;
    aProg[nProg++] = atoi(z);
    જ્યારે( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest2_Init(Tcl_Interp *interp){
  બાહ્ય પૂર્ણાંક sqlite3_io_error_persist;
  બાહ્ય પૂર્ણાંક sqlite3_io_error_pending;
  બાહ્ય પૂર્ણાંક sqlite3_io_error_hit;
  બાહ્ય પૂર્ણાંક sqlite3_io_error_hardhit;
  બાહ્ય પૂર્ણાંક sqlite3_diskfull_pending;
  બાહ્ય પૂર્ણાંક sqlite3_diskfull;
  સ્થિર સંરચના {
    અક્ષર *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_સંખ્યા         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (અક્ષર*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (અક્ષર*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (અક્ષર*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (અક્ષર*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (અક્ષર*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (અક્ષર*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (અક્ષર*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  પાછા TCL_OK;
}
