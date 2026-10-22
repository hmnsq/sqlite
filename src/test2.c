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

خارجی ثابت حرف *sqlite3ErrName(صحیح);

/*
** Page size and reserved size used for testing.
*/
ساکن صحیح test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
ساکن خالی pager_test_reiniter(Db_صفحہ *p_Not_مستعمل){
  واپس;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ساکن صحیح SQLITE_TCLAPI pager_open(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  u32 صفحہ_سائز;
  Pager *pPager;
  صحیح n_صفحہ;
  صحیح rc;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", تہی);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[2], &n_صفحہ) ) واپس TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_ڈھونڈو(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  sqlite3_Pager_سیٹ_Cachesize(pPager, n_صفحہ);
  صفحہ_سائز = test_pagesize;
  sqlite3_Pager_سیٹ_Pagesize(pPager, &صفحہ_سائز, -1);
  Tcl_Append_نتیجہ(interp, sqlite3_Test_Ptr_مقصد_متن(pPager), تہی);
  واپس TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ساکن صحیح SQLITE_TCLAPI pager_close(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
ساکن صحیح SQLITE_TCLAPI pager_rollback(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
ساکن صحیح SQLITE_TCLAPI pager_commit(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
ساکن صحیح SQLITE_TCLAPI pager_stmt_begin(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
ساکن صحیح SQLITE_TCLAPI pager_stmt_rollback(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
ساکن صحیح SQLITE_TCLAPI pager_stmt_commit(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
ساکن صحیح SQLITE_TCLAPI pager_stats(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح i, *a;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  a = sqlite3PagerStats(pPager);
  برائے(i=0; i<9; i++){
    ساکن حرف *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    حرف zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(حجم(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
ساکن صحیح SQLITE_TCLAPI pager_pagecount(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  حرف zBuf[100];
  صحیح n_صفحہ;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3PagerPagecount(pPager, &n_صفحہ);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", n_صفحہ);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ساکن صحیح SQLITE_TCLAPI صفحہ_get(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_صفحہ *p_صفحہ = 0;
  صحیح pgno;
  صحیح rc;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &pgno) ) واپس TCL_ERROR;
  rc = sqlite3_Pager_Shared_تالا(pPager);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &p_صفحہ, 0);
  }
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  Tcl_Append_نتیجہ(interp, sqlite3_Test_Ptr_مقصد_متن(p_صفحہ), تہی);
  واپس TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ساکن صحیح SQLITE_TCLAPI صفحہ_lookup(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_صفحہ *p_صفحہ;
  صحیح pgno;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &pgno) ) واپس TCL_ERROR;
  p_صفحہ = sqlite3PagerLookup(pPager, pgno);
  اگر( p_صفحہ ){
    Tcl_Append_نتیجہ(interp, sqlite3_Test_Ptr_مقصد_متن(p_صفحہ), تہی);
  }
  واپس TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ساکن صحیح SQLITE_TCLAPI pager_truncate(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحیح pgno;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", تہی);
    واپس TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &pgno) ) واپس TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  واپس TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ساکن صحیح SQLITE_TCLAPI صفحہ_unref(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Db_صفحہ *p_صفحہ;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", تہی);
    واپس TCL_ERROR;
  }
  p_صفحہ = (Db_صفحہ *)sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3PagerUnref(p_صفحہ);
  واپس TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ساکن صحیح SQLITE_TCLAPI صفحہ_read(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  حرف zBuf[100];
  Db_صفحہ *p_صفحہ;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", تہی);
    واپس TCL_ERROR;
  }
  p_صفحہ = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  حافظہ_نقل(zBuf, sqlite3_Pager_Get_ڈیٹا(p_صفحہ), حجم(zBuf));
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ساکن صحیح SQLITE_TCLAPI صفحہ_نمبر(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  حرف zBuf[100];
  Db_صفحہ *p_صفحہ;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", تہی);
    واپس TCL_ERROR;
  }
  p_صفحہ = (Db_صفحہ *)sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", sqlite3PagerPagenumber(p_صفحہ));
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ساکن صحیح SQLITE_TCLAPI صفحہ_write(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Db_صفحہ *p_صفحہ;
  حرف *p_ڈیٹا;
  صحیح rc;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", تہی);
    واپس TCL_ERROR;
  }
  p_صفحہ = (Db_صفحہ *)sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3PagerWrite(p_صفحہ);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  p_ڈیٹا = sqlite3_Pager_Get_ڈیٹا(p_صفحہ);
  strncpy(p_ڈیٹا, argv[2], test_pagesize-1);
  p_ڈیٹا[test_pagesize-1] = 0;
  واپس TCL_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DISKIO
/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
ساکن صحیح SQLITE_TCLAPI fake_big_file(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  صحیح rc;
  صحیح n;
  i64 انحراف;
  حرف *zFile;
  صحیح nFile;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", تہی);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[1], &n) ) واپس TCL_ERROR;
#اگر defined(_WIN32)
  اگر( n>2 ){
    Tcl_Append_نتیجہ(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (خالی*)0);
    واپس TCL_ERROR;
  }
#اگر_ختم

  pVfs = sqlite3_vfs_ڈھونڈو(0);
  nFile = (صحیح)لڑی_لمبائی(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  اگر( zFile==0 ) واپس TCL_ERROR;
  حافظہ_نقل(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, "open failed: ", sqlite3ErrName(rc), تہی);
    sqlite3_free(zFile);
    واپس TCL_ERROR;
  }
  انحراف = n;
  انحراف *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, انحراف);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, "write failed: ", sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}
#اگر_ختم


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ساکن صحیح SQLITE_TCLAPI testPendingByte(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح pbyte;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (خالی*)0);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[1], &pbyte) ) واپس TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ساکن Tcl_Interp *faultSimInterp = 0;
ساکن صحیح fault_Sim_Script_سائز = 0;
ساکن حرف *faultSimScript;
ساکن صحیح faultSimCallback(صحیح x){
  حرف zInt[30];
  صحیح i;
  صحیح isNeg;
  صحیح rc;
  اگر( x==0 ){
    حافظہ_نقل(faultSimScript+fault_Sim_Script_سائز, "0", 2);
  }ورنہ{
    /* Convert x to text without using any sqlite3 routines */
    اگر( x<0 ){
      isNeg = 1;
      x = -x;
    }ورنہ{
      isNeg = 0;
    }
    zInt[حجم(zInt)-1] = 0;
    برائے(i=حجم(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    اگر( isNeg ) zInt[i--] = '-';
    حافظہ_نقل(faultSimScript+fault_Sim_Script_سائز, zInt+i+1, حجم(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  اگر( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }ورنہ{
    rc = atoi(Tcl_Get_String_نتیجہ(faultSimInterp));
  }
  Tcl_Reset_نتیجہ(faultSimInterp);
  واپس rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
ساکن صحیح SQLITE_TCLAPI faultInstallCmd(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  ثابت حرف *zScript;
  صحیح nScript;
  صحیح rc;
  اگر( argc!=1 && argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (خالی*)0);
    واپس SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (صحیح)لڑی_لمبائی(zScript);
  اگر( faultSimScript ){
    آزاد(faultSimScript);
    faultSimScript = 0;
  }
  اگر( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }ورنہ{
    faultSimScript = حافظہ_دو( nScript+100 );
    اگر( faultSimScript==0 ){
      Tcl_Append_نتیجہ(interp, "out of memory", (خالی*)0);
      واپس SQLITE_ERROR;
    }
    حافظہ_نقل(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_سائز = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ساکن صحیح SQLITE_TCLAPI testBitvecBuiltinTest(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح sz, rc;
  صحیح nProg = 0;
  صحیح aProg[100];
  ثابت حرف *z;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (خالی*)0);
  }
  اگر( Tcl_GetInt(interp, argv[1], &sz) ) واپس TCL_ERROR;
  z = argv[2];
  جب( nProg<99 && *z ){
    جب( *z && !sqlite3Isdigit(*z) ){ z++; }
    اگر( *z==0 ) توڑو;
    aProg[nProg++] = atoi(z);
    جب( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest2_Init(Tcl_Interp *interp){
  خارجی صحیح sqlite3_io_error_persist;
  خارجی صحیح sqlite3_io_error_pending;
  خارجی صحیح sqlite3_io_error_کامیابی;
  خارجی صحیح sqlite3_io_error_hardhit;
  خارجی صحیح sqlite3_diskfull_pending;
  خارجی صحیح sqlite3_diskfull;
  ساکن ساخت {
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
    { "page_get",                (Tcl_CmdProc*)صفحہ_get            },
    { "page_lookup",             (Tcl_CmdProc*)صفحہ_lookup         },
    { "page_unref",              (Tcl_CmdProc*)صفحہ_unref          },
    { "page_read",               (Tcl_CmdProc*)صفحہ_read           },
    { "page_write",              (Tcl_CmdProc*)صفحہ_write          },
    { "page_number",             (Tcl_CmdProc*)صفحہ_نمبر         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#اگر_ختم
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  صحیح i;
  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (حرف*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (حرف*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (حرف*)&sqlite3_io_error_کامیابی, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (حرف*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (حرف*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (حرف*)&sqlite3_diskfull, TCL_LINK_INT);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (حرف*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#اگر_ختم
  واپس TCL_OK;
}
