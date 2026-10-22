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
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#എങ്കിൽ defined(unix)
#അന്തര്ഭാവ <sys/resource.h>
#endif

/* Forward declaration */
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
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
സ്ഥിരം അക്ഷരം *sqlite3TestInit(Tcl_Interp *interp){
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlite3_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqliteconfig_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest1_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest2_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest3_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest4_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest5_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest6_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest8_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest9_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_autoext_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_blob_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_demovfs_Init(Tcl_Interp *);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_func_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_hexio_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_init_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_malloc_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_mutex_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestschema_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestsse_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetesttclvar_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestfs_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ SqlitetestThread_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ SqlitetestOnefile_Init();
  ബാഹ്യ പൂർണ്ണസംഖ്യ SqlitetestOsinst_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestbackup_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestintarray_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestvfs_Init(Tcl_Interp *);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestrtree_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestrtreedoc_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitequota_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitemultiplex_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ SqliteSuperlock_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ SqlitetestSyscall_Init(Tcl_Interp*);
#എങ്കിൽ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  ബാഹ്യ പൂർണ്ണസംഖ്യ TestSession_Init(Tcl_Interp*);
#endif
  ബാഹ്യ പൂർണ്ണസംഖ്യ Md5_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Fts5tcl_Init(Tcl_Interp *);
  ബാഹ്യ പൂർണ്ണസംഖ്യ SqliteRbu_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetesttcl_Init(Tcl_Interp*);
#എങ്കിൽ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  ബാഹ്യ പൂർണ്ണസംഖ്യ Zipvfs_Init(Tcl_Interp*);
#endif
  ബാഹ്യ പൂർണ്ണസംഖ്യ TestExpert_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_window_Init(Tcl_Interp *);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestvdbecov_Init(Tcl_Interp *);
  ബാഹ്യ പൂർണ്ണസംഖ്യ TestRecover_Init(Tcl_Interp*);
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#എങ്കിൽ defined(unix)
  { ഘടന rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  എങ്കിൽ( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
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
#എങ്കിൽ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#എങ്കിൽ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
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
  തിരികെ 0;
}

/* tclcmd:   load_testfixture_extensions
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SLAVE");
    തിരികെ TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  എങ്കിൽ( !slave ){
    തിരികെ TCL_ERROR;
  }

  (ശൂന്യം)sqlite3TestInit(slave);
  തിരികെ TCL_OK;
}
