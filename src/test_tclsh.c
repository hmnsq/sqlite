/*
** 2017-10-13
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains extensions to the the "tclsqlite.c" module used for
** testing.  Basically, all of the other "test_*.c" modules are linked
** into the enhanced tclsh used for testing (and named "testfixture" or
** "testfixture.exe") using logic encoded by this file.
**
** The code in this file used to be found in tclsqlite3.c, contained within
** #if SQLITE_TEST ... #endif.  It is factored out into this separate module
** in an effort to keep the tclsqlite.c file pure.
*/
#شامل "sqlite3.h"
#شامل "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#اگر defined(unix)
#شامل <sys/resource.h>
#endif

/* Forward declaration */
ایستا صحیح SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
);

/*
** This routine is the primary export of this file.
**
** Configure the interpreter passed as the first argument to have access
** to the commands and linked variables that make up:
**
**   * the [sqlite3] extension itself,
**
**   * If SQLITE_TCLMD5 or SQLITE_TEST is defined, the Md5 commands, and
**
**   * If SQLITE_TEST is set, the various test interfaces used by the Tcl
**     test suite.
*/
ثابت نویسه *sqlite3TestInit(Tcl_Interp *interp){
  بیرونی صحیح Sqlite3_Init(Tcl_Interp*);
  بیرونی صحیح Sqliteconfig_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest1_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest2_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest3_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest4_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest5_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest6_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest8_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest9_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_autoext_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_blob_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_demovfs_Init(Tcl_Interp *);
  بیرونی صحیح Sqlitetest_func_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_hexio_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_init_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_malloc_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_mutex_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestschema_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestsse_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetesttclvar_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestfs_Init(Tcl_Interp*);
  بیرونی صحیح SqlitetestThread_Init(Tcl_Interp*);
  بیرونی صحیح SqlitetestOnefile_Init();
  بیرونی صحیح SqlitetestOsinst_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestbackup_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestintarray_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestvfs_Init(Tcl_Interp *);
  بیرونی صحیح Sqlitetestrtree_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestrtreedoc_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitequota_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitemultiplex_Init(Tcl_Interp*);
  بیرونی صحیح SqliteSuperlock_Init(Tcl_Interp*);
  بیرونی صحیح SqlitetestSyscall_Init(Tcl_Interp*);
#اگر defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  بیرونی صحیح TestSession_Init(Tcl_Interp*);
#endif
  بیرونی صحیح Md5_Init(Tcl_Interp*);
  بیرونی صحیح Fts5tcl_Init(Tcl_Interp *);
  بیرونی صحیح SqliteRbu_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetesttcl_Init(Tcl_Interp*);
#اگر defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  بیرونی صحیح Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  بیرونی صحیح Zipvfs_Init(Tcl_Interp*);
#endif
  بیرونی صحیح TestExpert_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetest_window_Init(Tcl_Interp *);
  بیرونی صحیح Sqlitetestvdbecov_Init(Tcl_Interp *);
  بیرونی صحیح TestRecover_Init(Tcl_Interp*);
  بیرونی صحیح Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#اگر defined(unix)
  { ساختار rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  اگر( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
    Sqlite3_Init(interp);
  }
#ifdef SQLITE_ENABLE_ZIPVFS
  Zipvfs_Init(interp);
#endif
  Md5_Init(interp);
  Sqliteconfig_Init(interp);
  Sqlitetest1_Init(interp);
  Sqlitetest2_Init(interp);
  Sqlitetest3_Init(interp);
  Sqlitetest4_Init(interp);
  Sqlitetest5_Init(interp);
  Sqlitetest6_Init(interp);
  Sqlitetest8_Init(interp);
  Sqlitetest9_Init(interp);
  Sqlitetest_autoext_Init(interp);
  Sqlitetest_blob_Init(interp);
  Sqlitetest_demovfs_Init(interp);
  Sqlitetest_func_Init(interp);
  Sqlitetest_hexio_Init(interp);
  Sqlitetest_init_Init(interp);
  Sqlitetest_malloc_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlitetestschema_Init(interp);
  Sqlitetesttclvar_Init(interp);
  Sqlitetestfs_Init(interp);
  SqlitetestThread_Init(interp);
  SqlitetestOnefile_Init();
  SqlitetestOsinst_Init(interp);
  Sqlitetestbackup_Init(interp);
  Sqlitetestintarray_Init(interp);
  Sqlitetestvfs_Init(interp);
  Sqlitetestrtree_Init(interp);
  Sqlitetestrtreedoc_Init(interp);
  Sqlitequota_Init(interp);
  Sqlitemultiplex_Init(interp);
  SqliteSuperlock_Init(interp);
  SqlitetestSyscall_Init(interp);
#اگر defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#اگر defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  Sqlitetestfts3_Init(interp);
#endif
  TestExpert_Init(interp);
  Sqlitetest_window_Init(interp);
  Sqlitetestvdbecov_Init(interp);
  TestRecover_Init(interp);
  Sqlitetestintck_Init(interp);

  Tcl_CreateObjCommand(
      interp, "load_testfixture_extensions", load_testfixture_extensions,0,0
  );
  بازگشت 0;
}

/* tclcmd:   load_testfixture_extensions
*/
ایستا صحیح SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SLAVE");
    بازگشت TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  اگر( !slave ){
    بازگشت TCL_ERROR;
  }

  (تهی)sqlite3TestInit(slave);
  بازگشت TCL_OK;
}
