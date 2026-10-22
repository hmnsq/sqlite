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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ctype.h>

বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/*
** Page size and reserved size used for testing.
*/
স্থির পূর্ণসংখ্যা test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
স্থির শূন্য pager_test_reiniter(DbPage *pNotUsed){
  ফেরত;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_open(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  u32 page_আকার;
  Pager *pPager;
  পূর্ণসংখ্যা nPage;
  পূর্ণসংখ্যা rc;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetInt(interp, argv[2], &nPage) ) ফেরত TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  page_আকার = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &page_আকার, -1);
  Tcl_Append_ফলাফল(interp, sqlite3TestPtrToText(pPager), ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_close(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_rollback(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_commit(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_stmt_begin(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_stmt_rollback(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_stmt_commit(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_stats(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা i, *a;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  জন্য(i=0; i<9; i++){
    স্থির অক্ষর *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    অক্ষর zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(আকার(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_pagecount(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  অক্ষর zBuf[100];
  পূর্ণসংখ্যা nPage;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(আকার(zBuf), zBuf, "%d", nPage);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI page_get(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  পূর্ণসংখ্যা pgno;
  পূর্ণসংখ্যা rc;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  যদি( Tcl_GetInt(interp, argv[2], &pgno) ) ফেরত TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  যদি( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  Tcl_Append_ফলাফল(interp, sqlite3TestPtrToText(pPage), ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI page_lookup(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  পূর্ণসংখ্যা pgno;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  যদি( Tcl_GetInt(interp, argv[2], &pgno) ) ফেরত TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  যদি( pPage ){
    Tcl_Append_ফলাফল(interp, sqlite3TestPtrToText(pPage), ফাঁকা);
  }
  ফেরত TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI pager_truncate(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Pager *pPager;
  পূর্ণসংখ্যা pgno;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  যদি( Tcl_GetInt(interp, argv[2], &pgno) ) ফেরত TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  ফেরত TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI page_unref(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  DbPage *pPage;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  ফেরত TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI page_read(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  অক্ষর zBuf[100];
  DbPage *pPage;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3_Pager_Get_তথ্য(pPage), আকার(zBuf));
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI page_সংখ্যা(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  অক্ষর zBuf[100];
  DbPage *pPage;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(আকার(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI page_write(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  DbPage *pPage;
  অক্ষর *p_তথ্য;
  পূর্ণসংখ্যা rc;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  p_তথ্য = sqlite3_Pager_Get_তথ্য(pPage);
  strncpy(p_তথ্য, argv[2], test_pagesize-1);
  p_তথ্য[test_pagesize-1] = 0;
  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI fake_big_file(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা n;
  i64 offset;
  অক্ষর *zFile;
  পূর্ণসংখ্যা nFile;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetInt(interp, argv[1], &n) ) ফেরত TCL_ERROR;
#যদি defined(_WIN32)
  যদি( n>2 ){
    Tcl_Append_ফলাফল(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (শূন্য*)0);
    ফেরত TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (পূর্ণসংখ্যা)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  যদি( zFile==0 ) ফেরত TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, "open failed: ", sqlite3ErrName(rc), ফাঁকা);
    sqlite3_free(zFile);
    ফেরত TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, "write failed: ", sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI testPendingByte(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা pbyte;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (শূন্য*)0);
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetInt(interp, argv[1], &pbyte) ) ফেরত TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
স্থির Tcl_Interp *faultSimInterp = 0;
স্থির পূর্ণসংখ্যা fault_Sim_Script_আকার = 0;
স্থির অক্ষর *faultSimScript;
স্থির পূর্ণসংখ্যা faultSimCallback(পূর্ণসংখ্যা x){
  অক্ষর zInt[30];
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা isNeg;
  পূর্ণসংখ্যা rc;
  যদি( x==0 ){
    memcpy(faultSimScript+fault_Sim_Script_আকার, "0", 2);
  }অন্যথা{
    /* Convert x to text without using any sqlite3 routines */
    যদি( x<0 ){
      isNeg = 1;
      x = -x;
    }অন্যথা{
      isNeg = 0;
    }
    zInt[আকার(zInt)-1] = 0;
    জন্য(i=আকার(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    যদি( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+fault_Sim_Script_আকার, zInt+i+1, আকার(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  যদি( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = atoi(Tcl_Get_String_ফলাফল(faultSimInterp));
  }
  Tcl_Reset_ফলাফল(faultSimInterp);
  ফেরত rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI faultInstallCmd(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  ধ্রুবক অক্ষর *zScript;
  পূর্ণসংখ্যা nScript;
  পূর্ণসংখ্যা rc;
  যদি( argc!=1 && argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (শূন্য*)0);
    ফেরত SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (পূর্ণসংখ্যা)strlen(zScript);
  যদি( faultSimScript ){
    মুক্ত(faultSimScript);
    faultSimScript = 0;
  }
  যদি( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }অন্যথা{
    faultSimScript = malloc( nScript+100 );
    যদি( faultSimScript==0 ){
      Tcl_Append_ফলাফল(interp, "out of memory", (শূন্য*)0);
      ফেরত SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_আকার = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI testBitvecBuiltinTest(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা sz, rc;
  পূর্ণসংখ্যা nProg = 0;
  পূর্ণসংখ্যা aProg[100];
  ধ্রুবক অক্ষর *z;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (শূন্য*)0);
  }
  যদি( Tcl_GetInt(interp, argv[1], &sz) ) ফেরত TCL_ERROR;
  z = argv[2];
  যখন( nProg<99 && *z ){
    যখন( *z && !sqlite3Isdigit(*z) ){ z++; }
    যদি( *z==0 ) বিরতি;
    aProg[nProg++] = atoi(z);
    যখন( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest2_Init(Tcl_Interp *interp){
  বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_persist;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_pending;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_hit;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_hardhit;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_diskfull_pending;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_diskfull;
  স্থির কাঠামো {
    অক্ষর *zName;
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
    { "page_number",             (Tcl_CmdProc*)page_সংখ্যা         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aCmd)/আকার(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (অক্ষর*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (অক্ষর*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (অক্ষর*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (অক্ষর*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (অক্ষর*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (অক্ষর*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (অক্ষর*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  ফেরত TCL_OK;
}
