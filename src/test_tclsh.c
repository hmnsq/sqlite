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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#ਜੇ defined(unix)
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/resource.h>
#ਜੇ_ਅੰਤ

/* Forward declaration */
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਲੱਦੋ_testfixture_extensions(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
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
ਅਚਲ ਅੱਖਰ *sqlite3TestInit(Tcl_Interp *interp){
  ਬਾਹਰੀ ਪੂਰਨ Sqlite3_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqliteconfig_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest1_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest2_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest3_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest4_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest5_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest6_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest8_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest9_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_autoext_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_blob_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_demovfs_Init(Tcl_Interp *);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_func_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_hexio_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_init_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_malloc_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_mutex_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestschema_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestsse_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetesttclvar_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestfs_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ SqlitetestThread_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ SqlitetestOnefile_Init();
  ਬਾਹਰੀ ਪੂਰਨ SqlitetestOsinst_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestbackup_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestintarray_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestvfs_Init(Tcl_Interp *);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestrtree_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestrtreedoc_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitequota_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitemultiplex_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ SqliteSuperlock_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ SqlitetestSyscall_Init(Tcl_Interp*);
#ਜੇ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  ਬਾਹਰੀ ਪੂਰਨ TestSession_Init(Tcl_Interp*);
#ਜੇ_ਅੰਤ
  ਬਾਹਰੀ ਪੂਰਨ Md5_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Fts5tcl_Init(Tcl_Interp *);
  ਬਾਹਰੀ ਪੂਰਨ SqliteRbu_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetesttcl_Init(Tcl_Interp*);
#ਜੇ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestfts3_Init(Tcl_Interp *interp);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ZIPVFS
  ਬਾਹਰੀ ਪੂਰਨ Zipvfs_Init(Tcl_Interp*);
#ਜੇ_ਅੰਤ
  ਬਾਹਰੀ ਪੂਰਨ TestExpert_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_ਖਿੜਕੀ_Init(Tcl_Interp *);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestvdbecov_Init(Tcl_Interp *);
  ਬਾਹਰੀ ਪੂਰਨ TestRecover_Init(Tcl_Interp*);
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#ਜੇ defined(unix)
  { ਬਣਤਰ rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#ਜੇ_ਅੰਤ /* unix */

  ਜੇ( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
    Sqlite3_Init(interp);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ZIPVFS
  Zipvfs_Init(interp);
#ਜੇ_ਅੰਤ
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
#ਜੇ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#ਜੇ_ਅੰਤ
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#ਜੇ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  Sqlitetestfts3_Init(interp);
#ਜੇ_ਅੰਤ
  TestExpert_Init(interp);
  Sqlitetest_ਖਿੜਕੀ_Init(interp);
  Sqlitetestvdbecov_Init(interp);
  TestRecover_Init(interp);
  Sqlitetestintck_Init(interp);

  Tcl_CreateObjCommand(
      interp, "load_testfixture_extensions", ਲੱਦੋ_testfixture_extensions,0,0
  );
  ਵਾਪਸ 0;
}

/* tclcmd:   load_testfixture_extensions
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਲੱਦੋ_testfixture_extensions(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SLAVE");
    ਵਾਪਸ TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  ਜੇ( !slave ){
    ਵਾਪਸ TCL_ERROR;
  }

  (ਖਾਲੀ)sqlite3TestInit(slave);
  ਵਾਪਸ TCL_OK;
}
