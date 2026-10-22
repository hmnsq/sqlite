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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <ctype.h>

வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/*
** Page size and reserved size used for testing.
*/
நிலையான முழு test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
நிலையான வெறுமை pager_test_reiniter(DbPage *pNotUsed){
  திரும்பு;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
நிலையான முழு SQLITE_TCLAPI pager_open(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  முழு nPage;
  முழு rc;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetInt(interp, argv[2], &nPage) ) திரும்பு TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
நிலையான முழு SQLITE_TCLAPI pager_close(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
நிலையான முழு SQLITE_TCLAPI pager_rollback(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
நிலையான முழு SQLITE_TCLAPI pager_commit(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
நிலையான முழு SQLITE_TCLAPI pager_stmt_begin(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
நிலையான முழு SQLITE_TCLAPI pager_stmt_rollback(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
நிலையான முழு SQLITE_TCLAPI pager_stmt_commit(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
நிலையான முழு SQLITE_TCLAPI pager_stats(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு i, *a;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  க்கு(i=0; i<9; i++){
    நிலையான எழுத்து *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    எழுத்து zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(அளவு(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
நிலையான முழு SQLITE_TCLAPI pager_pagecount(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  எழுத்து zBuf[100];
  முழு nPage;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(அளவு(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
நிலையான முழு SQLITE_TCLAPI page_get(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  முழு pgno;
  முழு rc;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  எனில்( Tcl_GetInt(interp, argv[2], &pgno) ) திரும்பு TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
நிலையான முழு SQLITE_TCLAPI page_lookup(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  முழு pgno;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  எனில்( Tcl_GetInt(interp, argv[2], &pgno) ) திரும்பு TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  எனில்( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), வெற்று);
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
நிலையான முழு SQLITE_TCLAPI pager_truncate(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Pager *pPager;
  முழு pgno;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  எனில்( Tcl_GetInt(interp, argv[2], &pgno) ) திரும்பு TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  திரும்பு TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
நிலையான முழு SQLITE_TCLAPI page_unref(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  DbPage *pPage;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  திரும்பு TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
நிலையான முழு SQLITE_TCLAPI page_read(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  எழுத்து zBuf[100];
  DbPage *pPage;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), அளவு(zBuf));
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
நிலையான முழு SQLITE_TCLAPI page_எண்(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  எழுத்து zBuf[100];
  DbPage *pPage;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(அளவு(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
நிலையான முழு SQLITE_TCLAPI page_write(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  DbPage *pPage;
  எழுத்து *pData;
  முழு rc;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI fake_big_file(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  முழு rc;
  முழு n;
  i64 offset;
  எழுத்து *zFile;
  முழு nFile;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetInt(interp, argv[1], &n) ) திரும்பு TCL_ERROR;
#எனில் defined(_WIN32)
  எனில்( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (வெறுமை*)0);
    திரும்பு TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (முழு)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  எனில்( zFile==0 ) திரும்பு TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  எனில்( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), வெற்று);
    sqlite3_free(zFile);
    திரும்பு TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  எனில்( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
நிலையான முழு SQLITE_TCLAPI testPendingByte(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு pbyte;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (வெறுமை*)0);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetInt(interp, argv[1], &pbyte) ) திரும்பு TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
நிலையான Tcl_Interp *faultSimInterp = 0;
நிலையான முழு faultSimScriptSize = 0;
நிலையான எழுத்து *faultSimScript;
நிலையான முழு faultSimCallback(முழு x){
  எழுத்து zInt[30];
  முழு i;
  முழு isNeg;
  முழு rc;
  எனில்( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }இல்லையெனில்{
    /* Convert x to text without using any sqlite3 routines */
    எனில்( x<0 ){
      isNeg = 1;
      x = -x;
    }இல்லையெனில்{
      isNeg = 0;
    }
    zInt[அளவு(zInt)-1] = 0;
    க்கு(i=அளவு(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    எனில்( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, அளவு(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  எனில்( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  திரும்பு rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
நிலையான முழு SQLITE_TCLAPI faultInstallCmd(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  மாறிலி எழுத்து *zScript;
  முழு nScript;
  முழு rc;
  எனில்( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (வெறுமை*)0);
    திரும்பு SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (முழு)strlen(zScript);
  எனில்( faultSimScript ){
    விடுவி(faultSimScript);
    faultSimScript = 0;
  }
  எனில்( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }இல்லையெனில்{
    faultSimScript = malloc( nScript+100 );
    எனில்( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (வெறுமை*)0);
      திரும்பு SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
நிலையான முழு SQLITE_TCLAPI testBitvecBuiltinTest(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு sz, rc;
  முழு nProg = 0;
  முழு aProg[100];
  மாறிலி எழுத்து *z;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (வெறுமை*)0);
  }
  எனில்( Tcl_GetInt(interp, argv[1], &sz) ) திரும்பு TCL_ERROR;
  z = argv[2];
  போது( nProg<99 && *z ){
    போது( *z && !sqlite3Isdigit(*z) ){ z++; }
    எனில்( *z==0 ) நிறுத்து;
    aProg[nProg++] = atoi(z);
    போது( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest2_Init(Tcl_Interp *interp){
  வெளி முழு sqlite3_io_error_persist;
  வெளி முழு sqlite3_io_error_pending;
  வெளி முழு sqlite3_io_error_hit;
  வெளி முழு sqlite3_io_error_hardhit;
  வெளி முழு sqlite3_diskfull_pending;
  வெளி முழு sqlite3_diskfull;
  நிலையான கட்டமைப்பு {
    எழுத்து *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_எண்         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (எழுத்து*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (எழுத்து*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (எழுத்து*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (எழுத்து*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (எழுத்து*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (எழுத்து*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (எழுத்து*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  திரும்பு TCL_OK;
}
