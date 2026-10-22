/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the pager.c module in SQLite.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
*/
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ctype.h>

బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/*
** Page size and reserved size used for testing.
*/
స్థిర పూర్ణాంకం test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
స్థిర శూన్య pager_test_reiniter(Db_పుట *p_Not_వాడిన){
  తిరిగి;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_open(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  u32 పుట_కొలత;
  Pager *pPager;
  పూర్ణాంకం n_పుట;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &n_పుట) ) తిరిగి TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_కనుగొను(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  sqlite3_Pager_సమితి_Cachesize(pPager, n_పుట);
  పుట_కొలత = test_pagesize;
  sqlite3_Pager_సమితి_Pagesize(pPager, &పుట_కొలత, -1);
  Tcl_Append_ఫలితం(interp, sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(pPager), ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_close(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_rollback(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_commit(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_stmt_begin(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_stmt_rollback(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_stmt_commit(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_stats(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం i, *a;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  a = sqlite3PagerStats(pPager);
  కొరకు(i=0; i<9; i++){
    స్థిర అక్షరం *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    అక్షరం zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(పరిమాణం(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_pagecount(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  అక్షరం zBuf[100];
  పూర్ణాంకం n_పుట;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3PagerPagecount(pPager, &n_పుట);
  sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%d", n_పుట);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI పుట_get(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_పుట *p_పుట = 0;
  పూర్ణాంకం pgno;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &pgno) ) తిరిగి TCL_ERROR;
  rc = sqlite3_Pager_Shared_తాళం(pPager);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &p_పుట, 0);
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  Tcl_Append_ఫలితం(interp, sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(p_పుట), ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI పుట_lookup(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  Db_పుట *p_పుట;
  పూర్ణాంకం pgno;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &pgno) ) తిరిగి TCL_ERROR;
  p_పుట = sqlite3PagerLookup(pPager, pgno);
  ఒకవేళ( p_పుట ){
    Tcl_Append_ఫలితం(interp, sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(p_పుట), ఖాళీ);
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI pager_truncate(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Pager *pPager;
  పూర్ణాంకం pgno;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pPager = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &pgno) ) తిరిగి TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  తిరిగి TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI పుట_unref(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Db_పుట *p_పుట;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  p_పుట = (Db_పుట *)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3PagerUnref(p_పుట);
  తిరిగి TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI పుట_read(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  అక్షరం zBuf[100];
  Db_పుట *p_పుట;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  p_పుట = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  జ్ఞాపకం_నకలు(zBuf, sqlite3_Pager_Get_దత్తాంశం(p_పుట), పరిమాణం(zBuf));
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI పుట_సంఖ్య(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  అక్షరం zBuf[100];
  Db_పుట *p_పుట;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  p_పుట = (Db_పుట *)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%d", sqlite3PagerPagenumber(p_పుట));
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI పుట_write(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Db_పుట *p_పుట;
  అక్షరం *p_దత్తాంశం;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  p_పుట = (Db_పుట *)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3PagerWrite(p_పుట);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  p_దత్తాంశం = sqlite3_Pager_Get_దత్తాంశం(p_పుట);
  strncpy(p_దత్తాంశం, argv[2], test_pagesize-1);
  p_దత్తాంశం[test_pagesize-1] = 0;
  తిరిగి TCL_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DISKIO
/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI fake_big_file(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  పూర్ణాంకం rc;
  పూర్ణాంకం n;
  i64 స్థానభ్రంశం;
  అక్షరం *zFile;
  పూర్ణాంకం nFile;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[1], &n) ) తిరిగి TCL_ERROR;
#ఒకవేళ defined(_WIN32)
  ఒకవేళ( n>2 ){
    Tcl_Append_ఫలితం(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (శూన్య*)0);
    తిరిగి TCL_ERROR;
  }
#అయితే_ముగింపు

  pVfs = sqlite3_vfs_కనుగొను(0);
  nFile = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  ఒకవేళ( zFile==0 ) తిరిగి TCL_ERROR;
  జ్ఞాపకం_నకలు(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, "open failed: ", sqlite3ErrName(rc), ఖాళీ);
    sqlite3_free(zFile);
    తిరిగి TCL_ERROR;
  }
  స్థానభ్రంశం = n;
  స్థానభ్రంశం *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, స్థానభ్రంశం);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, "write failed: ", sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}
#అయితే_ముగింపు


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI testPendingByte(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం pbyte;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (శూన్య*)0);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[1], &pbyte) ) తిరిగి TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
స్థిర Tcl_Interp *faultSimInterp = 0;
స్థిర పూర్ణాంకం fault_Sim_Script_కొలత = 0;
స్థిర అక్షరం *faultSimScript;
స్థిర పూర్ణాంకం faultSimCallback(పూర్ణాంకం x){
  అక్షరం zInt[30];
  పూర్ణాంకం i;
  పూర్ణాంకం isNeg;
  పూర్ణాంకం rc;
  ఒకవేళ( x==0 ){
    జ్ఞాపకం_నకలు(faultSimScript+fault_Sim_Script_కొలత, "0", 2);
  }లేకపోతే{
    /* Convert x to text without using any sqlite3 routines */
    ఒకవేళ( x<0 ){
      isNeg = 1;
      x = -x;
    }లేకపోతే{
      isNeg = 0;
    }
    zInt[పరిమాణం(zInt)-1] = 0;
    కొరకు(i=పరిమాణం(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    ఒకవేళ( isNeg ) zInt[i--] = '-';
    జ్ఞాపకం_నకలు(faultSimScript+fault_Sim_Script_కొలత, zInt+i+1, పరిమాణం(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  ఒకవేళ( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }లేకపోతే{
    rc = atoi(Tcl_Get_String_ఫలితం(faultSimInterp));
  }
  Tcl_Reset_ఫలితం(faultSimInterp);
  తిరిగి rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI faultInstallCmd(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  స్థిరరాశి అక్షరం *zScript;
  పూర్ణాంకం nScript;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=1 && argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (శూన్య*)0);
    తిరిగి SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zScript);
  ఒకవేళ( faultSimScript ){
    విడుదల(faultSimScript);
    faultSimScript = 0;
  }
  ఒకవేళ( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }లేకపోతే{
    faultSimScript = జ్ఞాపకం_ఇవ్వు( nScript+100 );
    ఒకవేళ( faultSimScript==0 ){
      Tcl_Append_ఫలితం(interp, "out of memory", (శూన్య*)0);
      తిరిగి SQLITE_ERROR;
    }
    జ్ఞాపకం_నకలు(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_కొలత = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI testBitvecBuiltinTest(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం sz, rc;
  పూర్ణాంకం nProg = 0;
  పూర్ణాంకం aProg[100];
  స్థిరరాశి అక్షరం *z;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (శూన్య*)0);
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[1], &sz) ) తిరిగి TCL_ERROR;
  z = argv[2];
  అయితే( nProg<99 && *z ){
    అయితే( *z && !sqlite3Isdigit(*z) ){ z++; }
    ఒకవేళ( *z==0 ) ఆపు;
    aProg[nProg++] = atoi(z);
    అయితే( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest2_Init(Tcl_Interp *interp){
  బాహ్య పూర్ణాంకం sqlite3_io_error_persist;
  బాహ్య పూర్ణాంకం sqlite3_io_error_pending;
  బాహ్య పూర్ణాంకం sqlite3_io_error_తగిలింది;
  బాహ్య పూర్ణాంకం sqlite3_io_error_hardhit;
  బాహ్య పూర్ణాంకం sqlite3_diskfull_pending;
  బాహ్య పూర్ణాంకం sqlite3_diskfull;
  స్థిర నిర్మాణం {
    అక్షరం *zName;
    Tcl_CmdProc *xProc;
  } aCmd[] = {
    { "pager_open",              (Tcl_CmdProc*)pager_open          },
    { "pager_close",             (Tcl_CmdProc*)pager_close         },
    { "pager_commit",            (Tcl_CmdProc*)pager_commit        },
    { "pager_rollback",          (Tcl_CmdProc*)pager_rollback      },
    { "pager_stmt_begin",        (Tcl_CmdProc*)pager_stmt_begin    },
    { "pager_stmt_commit",       (Tcl_CmdProc*)pager_stmt_commit   },
    { "pager_stmt_rollback",     (Tcl_CmdProc*)pager_stmt_rollback },
    { "pager_stats",             (Tcl_CmdProc*)pager_stats         },
    { "pager_pagecount",         (Tcl_CmdProc*)pager_pagecount     },
    { "page_get",                (Tcl_CmdProc*)పుట_get            },
    { "page_lookup",             (Tcl_CmdProc*)పుట_lookup         },
    { "page_unref",              (Tcl_CmdProc*)పుట_unref          },
    { "page_read",               (Tcl_CmdProc*)పుట_read           },
    { "page_write",              (Tcl_CmdProc*)పుట_write          },
    { "page_number",             (Tcl_CmdProc*)పుట_సంఖ్య         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#అయితే_ముగింపు
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (అక్షరం*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (అక్షరం*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (అక్షరం*)&sqlite3_io_error_తగిలింది, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (అక్షరం*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (అక్షరం*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (అక్షరం*)&sqlite3_diskfull, TCL_LINK_INT);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (అక్షరం*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}
