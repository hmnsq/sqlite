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
#اگر_ختم

/* Forward declaration */
ساکن صحیح SQLITE_TCLAPI بھرو_testfixture_extensions(
  Client_ڈیٹا cd,
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
ثابت حرف *sqlite3TestInit(Tcl_Interp *interp){
  خارجی صحیح Sqlite3_Init(Tcl_Interp*);
  خارجی صحیح Sqliteconfig_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest1_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest2_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest3_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest4_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest5_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest6_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest8_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest9_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_autoext_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_blob_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_demovfs_Init(Tcl_Interp *);
  خارجی صحیح Sqlitetest_func_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_hexio_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_init_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_malloc_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_mutex_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestschema_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestsse_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetesttclvar_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestfs_Init(Tcl_Interp*);
  خارجی صحیح SqlitetestThread_Init(Tcl_Interp*);
  خارجی صحیح SqlitetestOnefile_Init();
  خارجی صحیح SqlitetestOsinst_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestbackup_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestintarray_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestvfs_Init(Tcl_Interp *);
  خارجی صحیح Sqlitetestrtree_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestrtreedoc_Init(Tcl_Interp*);
  خارجی صحیح Sqlitequota_Init(Tcl_Interp*);
  خارجی صحیح Sqlitemultiplex_Init(Tcl_Interp*);
  خارجی صحیح SqliteSuperlock_Init(Tcl_Interp*);
  خارجی صحیح SqlitetestSyscall_Init(Tcl_Interp*);
#اگر defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  خارجی صحیح TestSession_Init(Tcl_Interp*);
#اگر_ختم
  خارجی صحیح Md5_Init(Tcl_Interp*);
  خارجی صحیح Fts5tcl_Init(Tcl_Interp *);
  خارجی صحیح SqliteRbu_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetesttcl_Init(Tcl_Interp*);
#اگر defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  خارجی صحیح Sqlitetestfts3_Init(Tcl_Interp *interp);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_ZIPVFS
  خارجی صحیح Zipvfs_Init(Tcl_Interp*);
#اگر_ختم
  خارجی صحیح TestExpert_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetest_کھڑکی_Init(Tcl_Interp *);
  خارجی صحیح Sqlitetestvdbecov_Init(Tcl_Interp *);
  خارجی صحیح TestRecover_Init(Tcl_Interp*);
  خارجی صحیح Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#اگر defined(unix)
  { ساخت rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#اگر_ختم /* unix */

  اگر( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
    Sqlite3_Init(interp);
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_ZIPVFS
  Zipvfs_Init(interp);
#اگر_ختم
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
#اگر_ختم
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#اگر defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  Sqlitetestfts3_Init(interp);
#اگر_ختم
  TestExpert_Init(interp);
  Sqlitetest_کھڑکی_Init(interp);
  Sqlitetestvdbecov_Init(interp);
  TestRecover_Init(interp);
  Sqlitetestintck_Init(interp);

  Tcl_CreateObjCommand(
      interp, "load_testfixture_extensions", بھرو_testfixture_extensions,0,0
  );
  واپس 0;
}

/* tclcmd:   load_testfixture_extensions
*/
ساکن صحیح SQLITE_TCLAPI بھرو_testfixture_extensions(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SLAVE");
    واپس TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  اگر( !slave ){
    واپس TCL_ERROR;
  }

  (خالی)sqlite3TestInit(slave);
  واپس TCL_OK;
}
