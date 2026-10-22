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
#nếu defined(unix)
#include <sys/resource.h>
#endif

/* Forward declaration */
static số_nguyên SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  số_nguyên objc,
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
const char *sqlite3TestInit(Tcl_Interp *interp){
  extern số_nguyên Sqlite3_Init(Tcl_Interp*);
  extern số_nguyên Sqliteconfig_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest1_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest2_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest3_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest4_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest5_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest6_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest8_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest9_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_autoext_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_blob_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_demovfs_Init(Tcl_Interp *);
  extern số_nguyên Sqlitetest_func_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_hexio_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_init_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_malloc_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_mutex_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestschema_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestsse_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetesttclvar_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestfs_Init(Tcl_Interp*);
  extern số_nguyên SqlitetestThread_Init(Tcl_Interp*);
  extern số_nguyên SqlitetestOnefile_Init();
  extern số_nguyên SqlitetestOsinst_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestbackup_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestintarray_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestvfs_Init(Tcl_Interp *);
  extern số_nguyên Sqlitetestrtree_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestrtreedoc_Init(Tcl_Interp*);
  extern số_nguyên Sqlitequota_Init(Tcl_Interp*);
  extern số_nguyên Sqlitemultiplex_Init(Tcl_Interp*);
  extern số_nguyên SqliteSuperlock_Init(Tcl_Interp*);
  extern số_nguyên SqlitetestSyscall_Init(Tcl_Interp*);
#nếu defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  extern số_nguyên TestSession_Init(Tcl_Interp*);
#endif
  extern số_nguyên Md5_Init(Tcl_Interp*);
  extern số_nguyên Fts5tcl_Init(Tcl_Interp *);
  extern số_nguyên SqliteRbu_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetesttcl_Init(Tcl_Interp*);
#nếu defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
  extern số_nguyên Sqlitetestfts3_Init(Tcl_Interp *interp);
#endif
#ifdef SQLITE_ENABLE_ZIPVFS
  extern số_nguyên Zipvfs_Init(Tcl_Interp*);
#endif
  extern số_nguyên TestExpert_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetest_window_Init(Tcl_Interp *);
  extern số_nguyên Sqlitetestvdbecov_Init(Tcl_Interp *);
  extern số_nguyên TestRecover_Init(Tcl_Interp*);
  extern số_nguyên Sqlitetestintck_Init(Tcl_Interp*);

  Tcl_CmdInfo cmdInfo;

  /* Since the primary use case for this binary is testing of SQLite,
  ** be sure to generate core files if we crash */
#nếu defined(unix)
  { struct rlimit x;
    getrlimit(RLIMIT_CORE, &x);
    x.rlim_cur = x.rlim_max;
    setrlimit(RLIMIT_CORE, &x);
  }
#endif /* unix */

  nếu( Tcl_GetCommandInfo(interp, "sqlite3", &cmdInfo)==0 ){
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
#nếu defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  TestSession_Init(interp);
#endif
  Fts5tcl_Init(interp);
  SqliteRbu_Init(interp);
  Sqlitetesttcl_Init(interp);

#nếu defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)
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
  trả_về 0;
}

/* tclcmd:   load_testfixture_extensions
*/
static số_nguyên SQLITE_TCLAPI load_testfixture_extensions(
  ClientData cd,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){

  Tcl_Interp *slave;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SLAVE");
    trả_về TCL_ERROR;
  }

  slave = Tcl_GetSlave(interp, Tcl_GetString(objv[1]));
  nếu( !slave ){
    trả_về TCL_ERROR;
  }

  (void)sqlite3TestInit(slave);
  trả_về TCL_OK;
}
