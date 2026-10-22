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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <ctype.h>

خارجي ثابت حرف *sqlite3ErrName(صحيح);

/*
** Page size and reserved size used for testing.
*/
ساكن صحيح test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ساكن فراغ pager_test_reiniter(DbPage *pNotUsed){
  أرجع;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ساكن صحيح SQLITE_TCLAPI pager_open(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  صحيح nPage;
  صحيح rc;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  𐤀𐤌( Tcl_GetInt(interp, argv[2], &nPage) ) أرجع TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), عدم);
  أرجع TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ساكن صحيح SQLITE_TCLAPI pager_close(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ساكن صحيح SQLITE_TCLAPI pager_rollback(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ساكن صحيح SQLITE_TCLAPI pager_commit(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ساكن صحيح SQLITE_TCLAPI pager_stmt_begin(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ساكن صحيح SQLITE_TCLAPI pager_stmt_rollback(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ساكن صحيح SQLITE_TCLAPI pager_stmt_commit(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ساكن صحيح SQLITE_TCLAPI pager_stats(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح i, *a;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  لكل(i=0; i<9; i++){
    ساكن حرف *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    حرف zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(حجم(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ساكن صحيح SQLITE_TCLAPI pager_pagecount(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  حرف zBuf[100];
  صحيح nPage;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ساكن صحيح SQLITE_TCLAPI page_get(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  صحيح pgno;
  صحيح rc;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  𐤀𐤌( Tcl_GetInt(interp, argv[2], &pgno) ) أرجع TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  𐤀𐤌( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ساكن صحيح SQLITE_TCLAPI page_lookup(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  صحيح pgno;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  𐤀𐤌( Tcl_GetInt(interp, argv[2], &pgno) ) أرجع TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  𐤀𐤌( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), عدم);
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ساكن صحيح SQLITE_TCLAPI pager_truncate(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح pgno;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  𐤀𐤌( Tcl_GetInt(interp, argv[2], &pgno) ) أرجع TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  أرجع TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ساكن صحيح SQLITE_TCLAPI page_unref(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  DbPage *pPage;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  أرجع TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ساكن صحيح SQLITE_TCLAPI page_read(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  حرف zBuf[100];
  DbPage *pPage;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), حجم(zBuf));
  Tcl_AppendResult(interp, zBuf, عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ساكن صحيح SQLITE_TCLAPI page_number(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  حرف zBuf[100];
  DbPage *pPage;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ساكن صحيح SQLITE_TCLAPI page_write(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  DbPage *pPage;
  حرف *pData;
  صحيح rc;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", عدم);
    أرجع TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  𐤀𐤌( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI fake_big_file(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  صحيح rc;
  صحيح n;
  i64 offset;
  حرف *zFile;
  صحيح nFile;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", عدم);
    أرجع TCL_ERROR;
  }
  𐤀𐤌( Tcl_GetInt(interp, argv[1], &n) ) أرجع TCL_ERROR;
#𐤀𐤌 defined(_WIN32)
  𐤀𐤌( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (فراغ*)0);
    أرجع TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (صحيح)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  𐤀𐤌( zFile==0 ) أرجع TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  𐤀𐤌( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), عدم);
    sqlite3_free(zFile);
    أرجع TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  𐤀𐤌( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ساكن صحيح SQLITE_TCLAPI testPendingByte(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح pbyte;
  صحيح rc;
  𐤀𐤌( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (فراغ*)0);
    أرجع TCL_ERROR;
  }
  𐤀𐤌( Tcl_GetInt(interp, argv[1], &pbyte) ) أرجع TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ساكن Tcl_Interp *faultSimInterp = 0;
ساكن صحيح faultSimScriptSize = 0;
ساكن حرف *faultSimScript;
ساكن صحيح faultSimCallback(صحيح x){
  حرف zInt[30];
  صحيح i;
  صحيح isNeg;
  صحيح rc;
  𐤀𐤌( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }وإلا{
    /* Convert x to text without using any sqlite3 routines */
    𐤀𐤌( x<0 ){
      isNeg = 1;
      x = -x;
    }وإلا{
      isNeg = 0;
    }
    zInt[حجم(zInt)-1] = 0;
    لكل(i=حجم(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    𐤀𐤌( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, حجم(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  𐤀𐤌( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }وإلا{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  أرجع rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ساكن صحيح SQLITE_TCLAPI faultInstallCmd(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  ثابت حرف *zScript;
  صحيح nScript;
  صحيح rc;
  𐤀𐤌( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (فراغ*)0);
    أرجع SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (صحيح)strlen(zScript);
  𐤀𐤌( faultSimScript ){
    حرر(faultSimScript);
    faultSimScript = 0;
  }
  𐤀𐤌( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }وإلا{
    faultSimScript = malloc( nScript+100 );
    𐤀𐤌( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (فراغ*)0);
      أرجع SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ساكن صحيح SQLITE_TCLAPI testBitvecBuiltinTest(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح sz, rc;
  صحيح nProg = 0;
  صحيح aProg[100];
  ثابت حرف *z;
  𐤀𐤌( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (فراغ*)0);
  }
  𐤀𐤌( Tcl_GetInt(interp, argv[1], &sz) ) أرجع TCL_ERROR;
  z = argv[2];
  طالما( nProg<99 && *z ){
    طالما( *z && !sqlite3Isdigit(*z) ){ z++; }
    𐤀𐤌( *z==0 ) اكسر;
    aProg[nProg++] = atoi(z);
    طالما( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest2_Init(Tcl_Interp *interp){
  خارجي صحيح sqlite3_io_error_persist;
  خارجي صحيح sqlite3_io_error_pending;
  خارجي صحيح sqlite3_io_error_hit;
  خارجي صحيح sqlite3_io_error_hardhit;
  خارجي صحيح sqlite3_diskfull_pending;
  خارجي صحيح sqlite3_diskfull;
  ساكن بنية {
    حرف *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_number         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  صحيح i;
  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (حرف*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (حرف*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (حرف*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (حرف*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (حرف*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (حرف*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (حرف*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  أرجع TCL_OK;
}
