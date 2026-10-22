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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <ctype.h>

بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/*
** Page size and reserved size used for testing.
*/
ایستا صحیح test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ایستا تهی pager_test_reiniter(DbPage *pNotUsed){
  بازگشت;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ایستا صحیح SQLITE_TCLAPI pager_open(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  صحیح nPage;
  صحیح rc;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[2], &nPage) ) بازگشت TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ایستا صحیح SQLITE_TCLAPI pager_close(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ایستا صحیح SQLITE_TCLAPI pager_rollback(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ایستا صحیح SQLITE_TCLAPI pager_commit(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ایستا صحیح SQLITE_TCLAPI pager_stmt_begin(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ایستا صحیح SQLITE_TCLAPI pager_stmt_rollback(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ایستا صحیح SQLITE_TCLAPI pager_stmt_commit(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ایستا صحیح SQLITE_TCLAPI pager_stats(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح i, *a;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  برای(i=0; i<9; i++){
    ایستا نویسه *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    نویسه zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(اندازه(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ایستا صحیح SQLITE_TCLAPI pager_pagecount(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  نویسه zBuf[100];
  صحیح nPage;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(اندازه(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ایستا صحیح SQLITE_TCLAPI page_get(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  صحیح pgno;
  صحیح rc;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &pgno) ) بازگشت TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ایستا صحیح SQLITE_TCLAPI page_lookup(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  صحیح pgno;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &pgno) ) بازگشت TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  اگر( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), پوچ);
  }
  بازگشت TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ایستا صحیح SQLITE_TCLAPI pager_truncate(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح pgno;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &pgno) ) بازگشت TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  بازگشت TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ایستا صحیح SQLITE_TCLAPI page_unref(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  DbPage *pPage;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  بازگشت TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ایستا صحیح SQLITE_TCLAPI page_read(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  نویسه zBuf[100];
  DbPage *pPage;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), اندازه(zBuf));
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ایستا صحیح SQLITE_TCLAPI page_عدد(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  نویسه zBuf[100];
  DbPage *pPage;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(اندازه(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ایستا صحیح SQLITE_TCLAPI page_write(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  DbPage *pPage;
  نویسه *pData;
  صحیح rc;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI fake_big_file(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  صحیح rc;
  صحیح n;
  i64 offset;
  نویسه *zFile;
  صحیح nFile;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[1], &n) ) بازگشت TCL_ERROR;
#اگر defined(_WIN32)
  اگر( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (تهی*)0);
    بازگشت TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (صحیح)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  اگر( zFile==0 ) بازگشت TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  اگر( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), پوچ);
    sqlite3_free(zFile);
    بازگشت TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  اگر( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ایستا صحیح SQLITE_TCLAPI testPendingByte(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  صحیح pbyte;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (تهی*)0);
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[1], &pbyte) ) بازگشت TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ایستا Tcl_Interp *faultSimInterp = 0;
ایستا صحیح faultSimScriptSize = 0;
ایستا نویسه *faultSimScript;
ایستا صحیح faultSimCallback(صحیح x){
  نویسه zInt[30];
  صحیح i;
  صحیح isNeg;
  صحیح rc;
  اگر( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }وگرنه{
    /* Convert x to text without using any sqlite3 routines */
    اگر( x<0 ){
      isNeg = 1;
      x = -x;
    }وگرنه{
      isNeg = 0;
    }
    zInt[اندازه(zInt)-1] = 0;
    برای(i=اندازه(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    اگر( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, اندازه(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  اگر( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }وگرنه{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  بازگشت rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ایستا صحیح SQLITE_TCLAPI faultInstallCmd(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  ثابت نویسه *zScript;
  صحیح nScript;
  صحیح rc;
  اگر( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (تهی*)0);
    بازگشت SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (صحیح)strlen(zScript);
  اگر( faultSimScript ){
    آزاد(faultSimScript);
    faultSimScript = 0;
  }
  اگر( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }وگرنه{
    faultSimScript = malloc( nScript+100 );
    اگر( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (تهی*)0);
      بازگشت SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ایستا صحیح SQLITE_TCLAPI testBitvecBuiltinTest(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  صحیح sz, rc;
  صحیح nProg = 0;
  صحیح aProg[100];
  ثابت نویسه *z;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (تهی*)0);
  }
  اگر( Tcl_GetInt(interp, argv[1], &sz) ) بازگشت TCL_ERROR;
  z = argv[2];
  وقتی( nProg<99 && *z ){
    وقتی( *z && !sqlite3Isdigit(*z) ){ z++; }
    اگر( *z==0 ) بشکن;
    aProg[nProg++] = atoi(z);
    وقتی( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest2_Init(Tcl_Interp *interp){
  بیرونی صحیح sqlite3_io_error_persist;
  بیرونی صحیح sqlite3_io_error_pending;
  بیرونی صحیح sqlite3_io_error_hit;
  بیرونی صحیح sqlite3_io_error_hardhit;
  بیرونی صحیح sqlite3_diskfull_pending;
  بیرونی صحیح sqlite3_diskfull;
  ایستا ساختار {
    نویسه *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_عدد         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  صحیح i;
  برای(i=0; i<اندازه(aCmd)/اندازه(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (نویسه*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (نویسه*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (نویسه*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (نویسه*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (نویسه*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (نویسه*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (نویسه*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  بازگشت TCL_OK;
}
