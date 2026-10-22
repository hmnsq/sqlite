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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern const var_na *sqlite3ErrName(පූර්ණ_සංඛ්‍යා);

/*
** Page size and reserved size used for testing.
*/
static පූර්ණ_සංඛ්‍යා test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
static void pager_test_reiniter(DbPage *pNotUsed){
  ආපසු;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_open(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  u32 pageSize;
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා nPage;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", NULL);
    ආපසු TCL_ERROR;
  }
  නම්( Tcl_GetInt(interp, argv[2], &nPage) ) ආපසු TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  pageSize = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &pageSize, -1);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPager), NULL);
  ආපසු TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_close(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_rollback(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_commit(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_stmt_begin(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_stmt_rollback(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_stmt_commit(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_stats(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා i, *a;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  සඳහා(i=0; i<9; i++){
    static var_na *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    var_na zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(parimaa_na(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_pagecount(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  var_na zBuf[100];
  පූර්ණ_සංඛ්‍යා nPage;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%d", nPage);
  Tcl_AppendResult(interp, zBuf, NULL);
  ආපසු TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI page_get(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  පූර්ණ_සංඛ්‍යා pgno;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  නම්( Tcl_GetInt(interp, argv[2], &pgno) ) ආපසු TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  නම්( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), NULL);
  ආපසු TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI page_lookup(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  පූර්ණ_සංඛ්‍යා pgno;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  නම්( Tcl_GetInt(interp, argv[2], &pgno) ) ආපසු TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  නම්( pPage ){
    Tcl_AppendResult(interp, sqlite3TestPtrToText(pPage), NULL);
  }
  ආපසු TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI pager_truncate(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  Pager *pPager;
  පූර්ණ_සංඛ්‍යා pgno;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  නම්( Tcl_GetInt(interp, argv[2], &pgno) ) ආපසු TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  ආපසු TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI page_unref(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  DbPage *pPage;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  ආපසු TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI page_read(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  var_na zBuf[100];
  DbPage *pPage;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  memcpy(zBuf, sqlite3PagerGetData(pPage), parimaa_na(zBuf));
  Tcl_AppendResult(interp, zBuf, NULL);
  ආපසු TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI page_number(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  var_na zBuf[100];
  DbPage *pPage;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_AppendResult(interp, zBuf, NULL);
  ආපසු TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI page_write(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  DbPage *pPage;
  var_na *pData;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", NULL);
    ආපසු TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  නම්( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  pData = sqlite3PagerGetData(pPage);
  strncpy(pData, argv[2], test_pagesize-1);
  pData[test_pagesize-1] = 0;
  ආපසු TCL_OK;
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
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI fake_big_file(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  පූර්ණ_සංඛ්‍යා rc;
  පූර්ණ_සංඛ්‍යා n;
  i64 offset;
  var_na *zFile;
  පූර්ණ_සංඛ්‍යා nFile;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", NULL);
    ආපසු TCL_ERROR;
  }
  නම්( Tcl_GetInt(interp, argv[1], &n) ) ආපසු TCL_ERROR;
#නම් defined(_WIN32)
  නම්( n>2 ){
    Tcl_AppendResult(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (void*)0);
    ආපසු TCL_ERROR;
  }
#endif

  pVfs = sqlite3_vfs_find(0);
  nFile = (පූර්ණ_සංඛ්‍යා)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  නම්( zFile==0 ) ආපසු TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  නම්( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), NULL);
    sqlite3_free(zFile);
    ආපසු TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  නම්( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), NULL);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}
#endif


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI testPendingByte(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  පූර්ණ_සංඛ්‍යා pbyte;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (void*)0);
    ආපසු TCL_ERROR;
  }
  නම්( Tcl_GetInt(interp, argv[1], &pbyte) ) ආපසු TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ආපසු TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
static Tcl_Interp *faultSimInterp = 0;
static පූර්ණ_සංඛ්‍යා faultSimScriptSize = 0;
static var_na *faultSimScript;
static පූර්ණ_සංඛ්‍යා faultSimCallback(පූර්ණ_සංඛ්‍යා x){
  var_na zInt[30];
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා isNeg;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }නැතහොත්{
    /* Convert x to text without using any sqlite3 routines */
    නම්( x<0 ){
      isNeg = 1;
      x = -x;
    }නැතහොත්{
      isNeg = 0;
    }
    zInt[parimaa_na(zInt)-1] = 0;
    සඳහා(i=parimaa_na(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    නම්( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, parimaa_na(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  නම්( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }නැතහොත්{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  ආපසු rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI faultInstallCmd(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  const var_na *zScript;
  පූර්ණ_සංඛ්‍යා nScript;
  පූර්ණ_සංඛ්‍යා rc;
  නම්( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (void*)0);
    ආපසු SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (පූර්ණ_සංඛ්‍යා)strlen(zScript);
  නම්( faultSimScript ){
    free(faultSimScript);
    faultSimScript = 0;
  }
  නම්( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }නැතහොත්{
    faultSimScript = malloc( nScript+100 );
    නම්( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (void*)0);
      ආපසු SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ආපසු SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI testBitvecBuiltinTest(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා argc,              /* Number of arguments */
  const var_na **argv      /* Text of each argument */
){
  පූර්ණ_සංඛ්‍යා sz, rc;
  පූර්ණ_සංඛ්‍යා nProg = 0;
  පූර්ණ_සංඛ්‍යා aProg[100];
  const var_na *z;
  නම්( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (void*)0);
  }
  නම්( Tcl_GetInt(interp, argv[1], &sz) ) ආපසු TCL_ERROR;
  z = argv[2];
  අතරතුර( nProg<99 && *z ){
    අතරතුර( *z && !sqlite3Isdigit(*z) ){ z++; }
    නම්( *z==0 ) bha_nga;
    aProg[nProg++] = atoi(z);
    අතරතුර( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ආපසු TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
පූර්ණ_සංඛ්‍යා Sqlitetest2_Init(Tcl_Interp *interp){
  extern පූර්ණ_සංඛ්‍යා sqlite3_io_error_persist;
  extern පූර්ණ_සංඛ්‍යා sqlite3_io_error_pending;
  extern පූර්ණ_සංඛ්‍යා sqlite3_io_error_hit;
  extern පූර්ණ_සංඛ්‍යා sqlite3_io_error_hardhit;
  extern පූර්ණ_සංඛ්‍යා sqlite3_diskfull_pending;
  extern පූර්ණ_සංඛ්‍යා sqlite3_diskfull;
  static sa_nracanaa {
    var_na *zName;
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
  පූර්ණ_සංඛ්‍යා i;
  සඳහා(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (var_na*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (var_na*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (var_na*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (var_na*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (var_na*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (var_na*)&sqlite3_diskfull, TCL_LINK_INT);
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (var_na*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  ආපසු TCL_OK;
}
