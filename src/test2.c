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
ساكن فراغ pager_test_reiniter(Db_صفحو *p_Not_استعمال_ٿيل){
  أرجع;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
ساكن صحيح SQLITE_TCLAPI pager_open(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  u32 صفحو_سائيز;
  Pager *pPager;
  صحيح n_صفحو;
  صحيح rc;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetInt(interp, argv[2], &n_صفحو) ) أرجع TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_ڳوليو(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  sqlite3_Pager_سيٽ_Cachesize(pPager, n_صفحو);
  صفحو_سائيز = test_pagesize;
  sqlite3_Pager_سيٽ_Pagesize(pPager, &صفحو_سائيز, -1);
  Tcl_Append_نتيجو(interp, sqlite3_Test_Ptr_منزل_متن(pPager), عدم);
  أرجع TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
ساكن صحيح SQLITE_TCLAPI pager_close(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح i, *a;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  حرف zBuf[100];
  صحيح n_صفحو;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  sqlite3PagerPagecount(pPager, &n_صفحو);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", n_صفحو);
  Tcl_Append_نتيجو(interp, zBuf, عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
ساكن صحيح SQLITE_TCLAPI صفحو_get(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_صفحو *p_صفحو = 0;
  صحيح pgno;
  صحيح rc;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  إذا( Tcl_GetInt(interp, argv[2], &pgno) ) أرجع TCL_ERROR;
  rc = sqlite3_Pager_Shared_تالو(pPager);
  إذا( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &p_صفحو, 0);
  }
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  Tcl_Append_نتيجو(interp, sqlite3_Test_Ptr_منزل_متن(p_صفحو), عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
ساكن صحيح SQLITE_TCLAPI صفحو_lookup(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_صفحو *p_صفحو;
  صحيح pgno;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  إذا( Tcl_GetInt(interp, argv[2], &pgno) ) أرجع TCL_ERROR;
  p_صفحو = sqlite3PagerLookup(pPager, pgno);
  إذا( p_صفحو ){
    Tcl_Append_نتيجو(interp, sqlite3_Test_Ptr_منزل_متن(p_صفحو), عدم);
  }
  أرجع TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
ساكن صحيح SQLITE_TCLAPI pager_truncate(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Pager *pPager;
  صحيح pgno;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", عدم);
    أرجع TCL_ERROR;
  }
  pPager = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  إذا( Tcl_GetInt(interp, argv[2], &pgno) ) أرجع TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  أرجع TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
ساكن صحيح SQLITE_TCLAPI صفحو_unref(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Db_صفحو *p_صفحو;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  p_صفحو = (Db_صفحو *)sqlite3_Test_متن_منزل_Ptr(argv[1]);
  sqlite3PagerUnref(p_صفحو);
  أرجع TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
ساكن صحيح SQLITE_TCLAPI صفحو_read(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  حرف zBuf[100];
  Db_صفحو *p_صفحو;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  p_صفحو = sqlite3_Test_متن_منزل_Ptr(argv[1]);
  يادگيري_نقل(zBuf, sqlite3_Pager_Get_معلومات(p_صفحو), حجم(zBuf));
  Tcl_Append_نتيجو(interp, zBuf, عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
ساكن صحيح SQLITE_TCLAPI صفحو_عدد(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  حرف zBuf[100];
  Db_صفحو *p_صفحو;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", عدم);
    أرجع TCL_ERROR;
  }
  p_صفحو = (Db_صفحو *)sqlite3_Test_متن_منزل_Ptr(argv[1]);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", sqlite3PagerPagenumber(p_صفحو));
  Tcl_Append_نتيجو(interp, zBuf, عدم);
  أرجع TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
ساكن صحيح SQLITE_TCLAPI صفحو_write(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Db_صفحو *p_صفحو;
  حرف *p_معلومات;
  صحيح rc;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", عدم);
    أرجع TCL_ERROR;
  }
  p_صفحو = (Db_صفحو *)sqlite3_Test_متن_منزل_Ptr(argv[1]);
  rc = sqlite3PagerWrite(p_صفحو);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتيجو(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  p_معلومات = sqlite3_Pager_Get_معلومات(p_صفحو);
  strncpy(p_معلومات, argv[2], test_pagesize-1);
  p_معلومات[test_pagesize-1] = 0;
  أرجع TCL_OK;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DISKIO
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  صحيح rc;
  صحيح n;
  i64 وٿي;
  حرف *zFile;
  صحيح nFile;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", عدم);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetInt(interp, argv[1], &n) ) أرجع TCL_ERROR;
#إذا defined(_WIN32)
  إذا( n>2 ){
    Tcl_Append_نتيجو(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (فراغ*)0);
    أرجع TCL_ERROR;
  }
#جيڪڏهن_پورو

  pVfs = sqlite3_vfs_ڳوليو(0);
  nFile = (صحيح)لڙي_ڊيگهه(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  إذا( zFile==0 ) أرجع TCL_ERROR;
  يادگيري_نقل(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  إذا( rc ){
    Tcl_Append_نتيجو(interp, "open failed: ", sqlite3ErrName(rc), عدم);
    sqlite3_free(zFile);
    أرجع TCL_ERROR;
  }
  وٿي = n;
  وٿي *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, وٿي);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  إذا( rc ){
    Tcl_Append_نتيجو(interp, "write failed: ", sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}
#جيڪڏهن_پورو


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
ساكن صحيح SQLITE_TCLAPI testPendingByte(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح pbyte;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (فراغ*)0);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetInt(interp, argv[1], &pbyte) ) أرجع TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
ساكن Tcl_Interp *faultSimInterp = 0;
ساكن صحيح fault_Sim_Script_سائيز = 0;
ساكن حرف *faultSimScript;
ساكن صحيح faultSimCallback(صحيح x){
  حرف zInt[30];
  صحيح i;
  صحيح isNeg;
  صحيح rc;
  إذا( x==0 ){
    يادگيري_نقل(faultSimScript+fault_Sim_Script_سائيز, "0", 2);
  }وإلا{
    /* Convert x to text without using any sqlite3 routines */
    إذا( x<0 ){
      isNeg = 1;
      x = -x;
    }وإلا{
      isNeg = 0;
    }
    zInt[حجم(zInt)-1] = 0;
    لكل(i=حجم(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    إذا( isNeg ) zInt[i--] = '-';
    يادگيري_نقل(faultSimScript+fault_Sim_Script_سائيز, zInt+i+1, حجم(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  إذا( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }وإلا{
    rc = atoi(Tcl_Get_String_نتيجو(faultSimInterp));
  }
  Tcl_Reset_نتيجو(faultSimInterp);
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
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  ثابت حرف *zScript;
  صحيح nScript;
  صحيح rc;
  إذا( argc!=1 && argc!=2 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (فراغ*)0);
    أرجع SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (صحيح)لڙي_ڊيگهه(zScript);
  إذا( faultSimScript ){
    حرر(faultSimScript);
    faultSimScript = 0;
  }
  إذا( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }وإلا{
    faultSimScript = يادگيري_ڏيو( nScript+100 );
    إذا( faultSimScript==0 ){
      Tcl_Append_نتيجو(interp, "out of memory", (فراغ*)0);
      أرجع SQLITE_ERROR;
    }
    يادگيري_نقل(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_سائيز = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
ساكن صحيح SQLITE_TCLAPI testBitvecBuiltinTest(
  فراغ *Not_استعمال_ٿيل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح sz, rc;
  صحيح nProg = 0;
  صحيح aProg[100];
  ثابت حرف *z;
  إذا( argc!=3 ){
    Tcl_Append_نتيجو(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (فراغ*)0);
  }
  إذا( Tcl_GetInt(interp, argv[1], &sz) ) أرجع TCL_ERROR;
  z = argv[2];
  طالما( nProg<99 && *z ){
    طالما( *z && !sqlite3Isdigit(*z) ){ z++; }
    إذا( *z==0 ) اكسر;
    aProg[nProg++] = atoi(z);
    طالما( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest2_Init(Tcl_Interp *interp){
  خارجي صحيح sqlite3_io_error_persist;
  خارجي صحيح sqlite3_io_error_pending;
  خارجي صحيح sqlite3_io_error_ڪاميابي;
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
    { "page_get",                (Tcl_CmdProc*)صفحو_get            },
    { "page_lookup",             (Tcl_CmdProc*)صفحو_lookup         },
    { "page_unref",              (Tcl_CmdProc*)صفحو_unref          },
    { "page_read",               (Tcl_CmdProc*)صفحو_read           },
    { "page_write",              (Tcl_CmdProc*)صفحو_write          },
    { "page_number",             (Tcl_CmdProc*)صفحو_عدد         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#جيڪڏهن_پورو
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
     (حرف*)&sqlite3_io_error_ڪاميابي, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (حرف*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (حرف*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (حرف*)&sqlite3_diskfull, TCL_LINK_INT);
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (حرف*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#جيڪڏهن_پورو
  أرجع TCL_OK;
}
