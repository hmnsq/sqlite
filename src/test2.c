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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>

बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** Page size and reserved size used for testing.
*/
स्थिर पूर्णाङ्क test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
स्थिर शून्य pager_test_reiniter(DbPage *pNotUsed){
  फर्काऊ;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_open(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  पूर्णाङ्क nPage;
  पूर्णाङ्क rc;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetInt(interp, argv[2], &nPage) ) फर्काऊ TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), रिक्त);
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_close(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_rollback(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_commit(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_stmt_begin(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_stmt_rollback(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_stmt_commit(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_stats(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क i, *a;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  लागि(i=0; i<9; i++){
    स्थिर वर्ण *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    वर्ण zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(परिमाण(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_pagecount(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  वर्ण zBuf[100];
  पूर्णाङ्क nPage;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(परिमाण(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  फर्काऊ TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI page_get(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  पूर्णाङ्क pgno;
  पूर्णाङ्क rc;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  यदि( Tcl_GetInt(interp, argv[2], &pgno) ) फर्काऊ TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  यदि( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), रिक्त);
  फर्काऊ TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI page_lookup(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  पूर्णाङ्क pgno;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  यदि( Tcl_GetInt(interp, argv[2], &pgno) ) फर्काऊ TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  यदि( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), रिक्त);
  }
  फर्काऊ TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI pager_truncate(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णाङ्क pgno;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  यदि( Tcl_GetInt(interp, argv[2], &pgno) ) फर्काऊ TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  फर्काऊ TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI page_unref(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  DbPage *pPage;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  फर्काऊ TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI page_read(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  वर्ण zBuf[100];
  DbPage *pPage;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  स_नकल(zBuf, sqlite3PagerGetData(pPage), परिमाण(zBuf));
  Tcl_AppendResult(interp, zBuf, रिक्त);
  फर्काऊ TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI page_सङ्ख्या(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  वर्ण zBuf[100];
  DbPage *pPage;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(परिमाण(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, रिक्त);
  फर्काऊ TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI page_write(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  DbPage *pPage;
  वर्ण *pData;
  पूर्णाङ्क rc;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  म_नकलन(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  फर्काऊ TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI fake_big_file(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  पूर्णाङ्क rc;
  पूर्णाङ्क n;
  i64 offset;
  वर्ण *zFile;
  पूर्णाङ्क nFile;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetInt(interp, argv[1], &n) ) फर्काऊ TCL_ERROR;
#यदि defined(_WIN32)
  यदि( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (शून्य*)0);
    फर्काऊ TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (पूर्णाङ्क)म_माप(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  यदि( zFile==0 ) फर्काऊ TCL_ERROR;
  स_नकल(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  यदि( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), रिक्त);
    sqlite3_free(zFile);
    फर्काऊ TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  यदि( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), रिक्त);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI testPendingByte(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क pbyte;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (शून्य*)0);
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetInt(interp, argv[1], &pbyte) ) फर्काऊ TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
स्थिर Tcl_Interp *faultSimInterp = 0;
स्थिर पूर्णाङ्क faultSimScriptSize = 0;
स्थिर वर्ण *faultSimScript;
स्थिर पूर्णाङ्क faultSimCallback(पूर्णाङ्क x){
  वर्ण zInt[30];
  पूर्णाङ्क i;
  पूर्णाङ्क isNeg;
  पूर्णाङ्क rc;
  यदि( x==0 ){
    स_नकल(faultSimScript+faultSimScriptSize, "0", 2);
  }अन्यथा{
    /* Convert x to text without using any sqlite3 routines */
    यदि( x<0 ){
      isNeg = 1;
      x = -x;
    }अन्यथा{
      isNeg = 0;
    }
    zInt[परिमाण(zInt)-1] = 0;
    लागि(i=परिमाण(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    यदि( isNeg ) zInt[i--] = '-';
    स_नकल(faultSimScript+faultSimScriptSize, zInt+i+1, परिमाण(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  यदि( rc ){
    ख_लिखो(मानक_त्रुटि, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = म_से_प(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  फर्काऊ rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI faultInstallCmd(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  अचर वर्ण *zScript;
  पूर्णाङ्क nScript;
  पूर्णाङ्क rc;
  यदि( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (शून्य*)0);
    फर्काऊ SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (पूर्णाङ्क)म_माप(zScript);
  यदि( faultSimScript ){
    मुक्त(faultSimScript);
    faultSimScript = 0;
  }
  यदि( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }अन्यथा{
    faultSimScript = दो_स्मृति( nScript+100 );
    यदि( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (शून्य*)0);
      फर्काऊ SQLITE_ERROR;
    }
    स_नकल(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI testBitvecBuiltinTest(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  अचर वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क sz, rc;
  पूर्णाङ्क nProg = 0;
  पूर्णाङ्क aProg[100];
  अचर वर्ण *z;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (शून्य*)0);
  }
  यदि( Tcl_GetInt(interp, argv[1], &sz) ) फर्काऊ TCL_ERROR;
  z = argv[2];
  जबसम्म( nProg<99 && *z ){
    जबसम्म( *z && !sqlite3Isdigit(*z) ){ z++; }
    यदि( *z==0 ) रोक;
    aProg[nProg++] = म_से_प(z);
    जबसम्म( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest2_Init(Tcl_Interp *interp){
  बाह्य पूर्णाङ्क sqlite3_io_error_persist;
  बाह्य पूर्णाङ्क sqlite3_io_error_pending;
  बाह्य पूर्णाङ्क sqlite3_io_error_hit;
  बाह्य पूर्णाङ्क sqlite3_io_error_hardhit;
  बाह्य पूर्णाङ्क sqlite3_diskfull_pending;
  बाह्य पूर्णाङ्क sqlite3_diskfull;
  स्थिर संरचना {
    वर्ण *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_सङ्ख्या         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  पूर्णाङ्क i;
  लागि(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (वर्ण*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (वर्ण*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (वर्ण*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (वर्ण*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (वर्ण*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (वर्ण*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (वर्ण*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  फर्काऊ TCL_OK;
}
