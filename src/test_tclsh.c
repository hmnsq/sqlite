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
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#सचे defined(unix)
#अन्तर्भाव <sys/resource.h>
#endif

/* Forward declaration */
स्थिर पूर्णाङ्क SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
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
ध्रुव वर्ण *sqlite3TestInit(Tcl_Interp *interp){
  बाह्य पूर्णाङ्क Sqlite3_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqliteconfig_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest1_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest2_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest3_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest4_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest5_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest6_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest8_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest9_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_autoext_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_blob_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_demovfs_Init(Tcl_Interp *);
  बाह्य पूर्णाङ्क Sqlitetest_func_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_hexio_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_init_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_malloc_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_mutex_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestschema_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestsse_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetesttclvar_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestfs_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क SqlitetestThread_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क SqlitetestOnefile_Init();
  बाह्य पूर्णाङ्क SqlitetestOsinst_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestbackup_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestintarray_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestvfs_Init(Tcl_Interp *);
  बाह्य पूर्णाङ्क Sqlitetestrtree_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestrtreedoc_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitequota_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitemultiplex_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क SqliteSuperlock_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क SqlitetestSyscall_Init(Tcl_Interp*);
#सचे defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  बाह्य पूर्णाङ्क TestSession_Init(Tcl_Interp*);
#endif
  बाह्य पूर्णाङ्क Md5_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Fts5tcl_Init(Tcl_Interp *);
  बाह्य पूर्णाङ्क SqliteRbu_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetesttcl_Init(Tcl_Interp*);
#सचे defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  बाह्य पूर्णाङ्क Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  बाह्य पूर्णाङ्क Zipvfs_Init(Tcl_Interp*);
#endif
  बाह्य पूर्णाङ्क TestExpert_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetest_window_Init(Tcl_Interp *);
  बाह्य पूर्णाङ्क Sqlitetestvdbecov_Init(Tcl_Interp *);
  बाह्य पूर्णाङ्क TestRecover_Init(Tcl_Interp*);
  बाह्य पूर्णाङ्क Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#सचे defined(unix)
  { संरचना rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  सचे( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
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
#सचे defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#सचे defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
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
  पटिनिवत्त 0;
}

/* tclcmd:   load_testfixture_extensions
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "SLAVE");
    पटिनिवत्त TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  सचे( !slave ){
    पटिनिवत्त TCL_ERROR;
  }

  (शून्य)sqlite3TestInit(slave);
  पटिनिवत्त TCL_OK;
}
