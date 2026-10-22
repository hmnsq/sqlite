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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI load_testfixture_extensions(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
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
ধ্রুবক অক্ষর *sqlite3TestInit(Tcl_Interp *interp){
  বাহ্যিক পূর্ণসংখ্যা Sqlite3_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqliteconfig_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest1_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest2_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest3_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest4_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest5_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest6_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest8_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest9_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_autoext_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_blob_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_demovfs_Init(Tcl_Interp *);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_func_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_hexio_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_init_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_malloc_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_mutex_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestschema_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestsse_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetesttclvar_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestfs_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা SqlitetestThread_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা SqlitetestOnefile_Init();
  বাহ্যিক পূর্ণসংখ্যা SqlitetestOsinst_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestbackup_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestintarray_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestvfs_Init(Tcl_Interp *);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestrtree_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestrtreedoc_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitequota_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitemultiplex_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা SqliteSuperlock_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা SqlitetestSyscall_Init(Tcl_Interp*);
#যদি defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  বাহ্যিক পূর্ণসংখ্যা TestSession_Init(Tcl_Interp*);
#endif
  বাহ্যিক পূর্ণসংখ্যা Md5_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Fts5tcl_Init(Tcl_Interp *);
  বাহ্যিক পূর্ণসংখ্যা SqliteRbu_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetesttcl_Init(Tcl_Interp*);
#যদি defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  বাহ্যিক পূর্ণসংখ্যা Zipvfs_Init(Tcl_Interp*);
#endif
  বাহ্যিক পূর্ণসংখ্যা TestExpert_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_window_Init(Tcl_Interp *);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestvdbecov_Init(Tcl_Interp *);
  বাহ্যিক পূর্ণসংখ্যা TestRecover_Init(Tcl_Interp*);
  বাহ্যিক পূর্ণসংখ্যা Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#যদি defined(unix)
  { কাঠামো rlimit x;
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
  ফেরত 0;
}

/* tclcmd:   load_testfixture_extensions
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI load_testfixture_extensions(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SLAVE");
    ফেরত TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  যদি( !slave ){
    ফেরত TCL_ERROR;
  }

  (শূন্য)sqlite3TestInit(slave);
  ফেরত TCL_OK;
}
