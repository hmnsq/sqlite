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
#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#যদি defined(unix)
#অন্তর্ভাব <sys/resource.h>
#endif

/* Forward declaration */
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
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
ধ্রুব বর্ণ *sqlite3TestInit(Tcl_Interp *interp){
  বাহ্য পূর্ণাঙ্ক Sqlite3_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqliteconfig_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest1_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest2_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest3_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest4_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest5_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest6_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest8_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest9_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_autoext_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_blob_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_demovfs_Init(Tcl_Interp *);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_func_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_hexio_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_init_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_malloc_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_mutex_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestschema_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestsse_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetesttclvar_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestfs_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক SqlitetestThread_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক SqlitetestOnefile_Init();
  বাহ্য পূর্ণাঙ্ক SqlitetestOsinst_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestbackup_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestintarray_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestvfs_Init(Tcl_Interp *);
  বাহ্য পূর্ণাঙ্ক Sqlitetestrtree_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestrtreedoc_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitequota_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitemultiplex_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক SqliteSuperlock_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক SqlitetestSyscall_Init(Tcl_Interp*);
#যদি defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  বাহ্য পূর্ণাঙ্ক TestSession_Init(Tcl_Interp*);
#endif
  বাহ্য পূর্ণাঙ্ক Md5_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Fts5tcl_Init(Tcl_Interp *);
  বাহ্য পূর্ণাঙ্ক SqliteRbu_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetesttcl_Init(Tcl_Interp*);
#যদি defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  বাহ্য পূর্ণাঙ্ক Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  বাহ্য পূর্ণাঙ্ক Zipvfs_Init(Tcl_Interp*);
#endif
  বাহ্য পূর্ণাঙ্ক TestExpert_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetest_window_Init(Tcl_Interp *);
  বাহ্য পূর্ণাঙ্ক Sqlitetestvdbecov_Init(Tcl_Interp *);
  বাহ্য পূর্ণাঙ্ক TestRecover_Init(Tcl_Interp*);
  বাহ্য পূর্ণাঙ্ক Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#যদি defined(unix)
  { সংরচনা rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  যদি( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
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
#যদি defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#যদি defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
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
  প্রত্যাগম 0;
}

/* tclcmd:   load_testfixture_extensions
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SLAVE");
    প্রত্যাগম TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  যদি( !slave ){
    প্রত্যাগম TCL_ERROR;
  }

  (শূন্য)sqlite3TestInit(slave);
  প্রত্যাগম TCL_OK;
}
