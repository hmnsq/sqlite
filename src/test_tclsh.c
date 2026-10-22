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
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#ఒకవేళ defined(unix)
#అన్తర్భావ <sys/resource.h>
#అయితే_ముగింపు

/* Forward declaration */
స్థిర పూర్ణాంకం SQLITE_TCLAPI ఎక్కించు_testfixture_extensions(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
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
స్థిరరాశి అక్షరం *sqlite3TestInit(Tcl_Interp *interp){
  బాహ్య పూర్ణాంకం Sqlite3_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqliteconfig_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest1_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest2_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest3_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest4_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest5_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest6_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest8_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest9_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_autoext_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_blob_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_demovfs_Init(Tcl_Interp *);
  బాహ్య పూర్ణాంకం Sqlitetest_func_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_hexio_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_init_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_malloc_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_mutex_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestschema_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestsse_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetesttclvar_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestfs_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం SqlitetestThread_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం SqlitetestOnefile_Init();
  బాహ్య పూర్ణాంకం SqlitetestOsinst_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestbackup_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestintarray_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestvfs_Init(Tcl_Interp *);
  బాహ్య పూర్ణాంకం Sqlitetestrtree_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestrtreedoc_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitequota_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitemultiplex_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం SqliteSuperlock_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం SqlitetestSyscall_Init(Tcl_Interp*);
#ఒకవేళ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  బాహ్య పూర్ణాంకం TestSession_Init(Tcl_Interp*);
#అయితే_ముగింపు
  బాహ్య పూర్ణాంకం Md5_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Fts5tcl_Init(Tcl_Interp *);
  బాహ్య పూర్ణాంకం SqliteRbu_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetesttcl_Init(Tcl_Interp*);
#ఒకవేళ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  బాహ్య పూర్ణాంకం Sqlitetestfts3_Init(Tcl_Interp *interp);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_ZIPVFS
  బాహ్య పూర్ణాంకం Zipvfs_Init(Tcl_Interp*);
#అయితే_ముగింపు
  బాహ్య పూర్ణాంకం TestExpert_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetest_కిటికీ_Init(Tcl_Interp *);
  బాహ్య పూర్ణాంకం Sqlitetestvdbecov_Init(Tcl_Interp *);
  బాహ్య పూర్ణాంకం TestRecover_Init(Tcl_Interp*);
  బాహ్య పూర్ణాంకం Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#ఒకవేళ defined(unix)
  { నిర్మాణం rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#అయితే_ముగింపు /* unix */

  ఒకవేళ( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
    Sqlite3_Init(interp);
  }
#నిర్వచితమైతే SQLITE_ENABLE_ZIPVFS
  Zipvfs_Init(interp);
#అయితే_ముగింపు
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
#ఒకవేళ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#అయితే_ముగింపు
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#ఒకవేళ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  Sqlitetestfts3_Init(interp);
#అయితే_ముగింపు
  TestExpert_Init(interp);
  Sqlitetest_కిటికీ_Init(interp);
  Sqlitetestvdbecov_Init(interp);
  TestRecover_Init(interp);
  Sqlitetestintck_Init(interp);

  Tcl_CreateObjCommand(
      interp, "load_testfixture_extensions", ఎక్కించు_testfixture_extensions,0,0
  );
  తిరిగి 0;
}

/* tclcmd:   load_testfixture_extensions
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI ఎక్కించు_testfixture_extensions(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SLAVE");
    తిరిగి TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  ఒకవేళ( !slave ){
    తిరిగి TCL_ERROR;
  }

  (శూన్య)sqlite3TestInit(slave);
  తిరిగి TCL_OK;
}
