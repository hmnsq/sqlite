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
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#ಯದಿ defined(unix)
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#endif

/* Forward declaration */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
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
ಧ್ರುವ ವರ್ಣ *sqlite3TestInit(Tcl_Interp *interp){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlite3_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqliteconfig_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest1_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest2_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest3_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest4_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest5_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest6_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest8_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest9_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_autoext_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_blob_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_demovfs_Init(Tcl_Interp *);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_func_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_hexio_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_init_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_malloc_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_mutex_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestschema_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestsse_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetesttclvar_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestfs_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ SqlitetestThread_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ SqlitetestOnefile_Init();
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ SqlitetestOsinst_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestbackup_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestintarray_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestvfs_Init(Tcl_Interp *);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestrtree_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestrtreedoc_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitequota_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitemultiplex_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ SqliteSuperlock_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ SqlitetestSyscall_Init(Tcl_Interp*);
#ಯದಿ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ TestSession_Init(Tcl_Interp*);
#endif
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Md5_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Fts5tcl_Init(Tcl_Interp *);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ SqliteRbu_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetesttcl_Init(Tcl_Interp*);
#ಯದಿ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Zipvfs_Init(Tcl_Interp*);
#endif
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ TestExpert_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetest_window_Init(Tcl_Interp *);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestvdbecov_Init(Tcl_Interp *);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ TestRecover_Init(Tcl_Interp*);
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#ಯದಿ defined(unix)
  { ಸಂರಚನಾ rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  ಯದಿ( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
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
#ಯದಿ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#ಯದಿ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
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
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* tclcmd:   load_testfixture_extensions
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SLAVE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  ಯದಿ( !slave ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  (ಶೂನ್ಯ)sqlite3TestInit(slave);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
