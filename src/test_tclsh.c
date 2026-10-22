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
#ضمن "sqlite3.h"
#ضمن "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#إذا defined(unix)
#ضمن <sys/resource.h>
#جيڪڏهن_پورو

/* Forward declaration */
ساكن صحيح SQLITE_TCLAPI ڀريو_testfixture_extensions(
  Client_معلومات cd,
  Tcl_Interp *interp,
  صحيح objc,
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
  خارجي صحيح Sqlite3_Init(Tcl_Interp*);
  خارجي صحيح Sqliteconfig_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest1_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest2_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest3_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest4_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest5_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest6_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest8_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest9_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_autoext_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_blob_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_demovfs_Init(Tcl_Interp *);
  خارجي صحيح Sqlitetest_func_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_hexio_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_init_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_malloc_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_mutex_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestschema_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestsse_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetesttclvar_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestfs_Init(Tcl_Interp*);
  خارجي صحيح SqlitetestThread_Init(Tcl_Interp*);
  خارجي صحيح SqlitetestOnefile_Init();
  خارجي صحيح SqlitetestOsinst_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestbackup_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestintarray_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestvfs_Init(Tcl_Interp *);
  خارجي صحيح Sqlitetestrtree_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestrtreedoc_Init(Tcl_Interp*);
  خارجي صحيح Sqlitequota_Init(Tcl_Interp*);
  خارجي صحيح Sqlitemultiplex_Init(Tcl_Interp*);
  خارجي صحيح SqliteSuperlock_Init(Tcl_Interp*);
  خارجي صحيح SqlitetestSyscall_Init(Tcl_Interp*);
#إذا defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  خارجي صحيح TestSession_Init(Tcl_Interp*);
#جيڪڏهن_پورو
  خارجي صحيح Md5_Init(Tcl_Interp*);
  خارجي صحيح Fts5tcl_Init(Tcl_Interp *);
  خارجي صحيح SqliteRbu_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetesttcl_Init(Tcl_Interp*);
#إذا defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  خارجي صحيح Sqlitetestfts3_Init(Tcl_Interp *interp);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ZIPVFS
  خارجي صحيح Zipvfs_Init(Tcl_Interp*);
#جيڪڏهن_پورو
  خارجي صحيح TestExpert_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetest_دري_Init(Tcl_Interp *);
  خارجي صحيح Sqlitetestvdbecov_Init(Tcl_Interp *);
  خارجي صحيح TestRecover_Init(Tcl_Interp*);
  خارجي صحيح Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#إذا defined(unix)
  { بنية rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#جيڪڏهن_پورو /* unix */

  إذا( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
    Sqlite3_Init(interp);
  }
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ZIPVFS
  Zipvfs_Init(interp);
#جيڪڏهن_پورو
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
#إذا defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#جيڪڏهن_پورو
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#إذا defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  Sqlitetestfts3_Init(interp);
#جيڪڏهن_پورو
  TestExpert_Init(interp);
  Sqlitetest_دري_Init(interp);
  Sqlitetestvdbecov_Init(interp);
  TestRecover_Init(interp);
  Sqlitetestintck_Init(interp);

  Tcl_CreateObjCommand(
      interp, "load_testfixture_extensions", ڀريو_testfixture_extensions,0,0
  );
  أرجع 0;
}

/* tclcmd:   load_testfixture_extensions
*/
ساكن صحيح SQLITE_TCLAPI ڀريو_testfixture_extensions(
  Client_معلومات cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  إذا( objc!=2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "SLAVE");
    أرجع TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  إذا( !slave ){
    أرجع TCL_ERROR;
  }

  (فراغ)sqlite3TestInit(slave);
  أرجع TCL_OK;
}
