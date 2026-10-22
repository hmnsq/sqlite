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
#include "sqlite3.h"
#include "tclsqlite.h"

/* Needed for the setrlimit() system call on unix */
#ຖ້າ defined(unix)
#include <sys/resource.h>
#endif

/* Forward declaration */
static ຈຳນວນເຕັມ SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
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
const var_na *sqlite3TestInit(Tcl_Interp *interp){
  extern ຈຳນວນເຕັມ Sqlite3_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqliteconfig_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest1_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest2_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest3_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest4_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest5_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest6_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest8_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest9_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_autoext_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_blob_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_demovfs_Init(Tcl_Interp *);
  extern ຈຳນວນເຕັມ Sqlitetest_func_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_hexio_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_init_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_malloc_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_mutex_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestschema_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestsse_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetesttclvar_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestfs_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ SqlitetestThread_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ SqlitetestOnefile_Init();
  extern ຈຳນວນເຕັມ SqlitetestOsinst_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestbackup_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestintarray_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestvfs_Init(Tcl_Interp *);
  extern ຈຳນວນເຕັມ Sqlitetestrtree_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestrtreedoc_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitequota_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitemultiplex_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ SqliteSuperlock_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ SqlitetestSyscall_Init(Tcl_Interp*);
#ຖ້າ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  extern ຈຳນວນເຕັມ TestSession_Init(Tcl_Interp*);
#endif
  extern ຈຳນວນເຕັມ Md5_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Fts5tcl_Init(Tcl_Interp *);
  extern ຈຳນວນເຕັມ SqliteRbu_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetesttcl_Init(Tcl_Interp*);
#ຖ້າ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  extern ຈຳນວນເຕັມ Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  extern ຈຳນວນເຕັມ Zipvfs_Init(Tcl_Interp*);
#endif
  extern ຈຳນວນເຕັມ TestExpert_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetest_window_Init(Tcl_Interp *);
  extern ຈຳນວນເຕັມ Sqlitetestvdbecov_Init(Tcl_Interp *);
  extern ຈຳນວນເຕັມ TestRecover_Init(Tcl_Interp*);
  extern ຈຳນວນເຕັມ Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#ຖ້າ defined(unix)
  { sa_nracanaa rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  ຖ້າ( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
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
#ຖ້າ defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#ຖ້າ defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
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
  ສົ່ງຄືນ 0;
}

/* tclcmd:   load_testfixture_extensions
*/
static ຈຳນວນເຕັມ SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  ຖ້າ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SLAVE");
    ສົ່ງຄືນ TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  ຖ້າ( !slave ){
    ສົ່ງຄືນ TCL_ERROR;
  }

  (void)sqlite3TestInit(slave);
  ສົ່ງຄືນ TCL_OK;
}
