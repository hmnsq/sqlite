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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <ctype.h>

बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/*
** Page size and reserved size used for testing.
*/
अटल पूर्णांक test_pagesize = 1024;

/*
** Dummy page reinitializer
*/
अटल व्योम pager_test_reiniter(DbPage *pNotUsed){
  वापस;
}

/*
** Usage:   pager_open FILENAME N-PAGE
**
** Open a new pager
*/
अटल पूर्णांक SQLITE_TCLAPI pager_open(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  u32 page_आकार;
  Pager *pPager;
  पूर्णांक nPage;
  पूर्णांक rc;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " FILENAME N-PAGE\"", शून्य);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetInt(interp, argv[2], &nPage) ) वापस TCL_ERROR;
  rc = sqlite3PagerOpen(sqlite3_vfs_find(0), &pPager, argv[1], 0, 0,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB,
      pager_test_reiniter);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  sqlite3PagerSetCachesize(pPager, nPage);
  page_आकार = test_pagesize;
  sqlite3PagerSetPagesize(pPager, &page_आकार, -1);
  Tcl_Append_परिणाम(interp, sqlite3TestPtrToText(pPager), शून्य);
  वापस TCL_OK;
}

/*
** Usage:   pager_close ID
**
** Close the given pager.
*/
अटल पूर्णांक SQLITE_TCLAPI pager_close(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerClose(pPager, 0);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_rollback ID
**
** Rollback changes
*/
अटल पूर्णांक SQLITE_TCLAPI pager_rollback(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerRollback(pPager);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_commit ID
**
** Commit all changes
*/
अटल पूर्णांक SQLITE_TCLAPI pager_commit(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerCommitPhaseOne(pPager, 0, 0);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  rc = sqlite3PagerCommitPhaseTwo(pPager);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_stmt_begin ID
**
** Start a new checkpoint.
*/
अटल पूर्णांक SQLITE_TCLAPI pager_stmt_begin(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerOpenSavepoint(pPager, 1);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_stmt_rollback ID
**
** Rollback changes to a checkpoint
*/
अटल पूर्णांक SQLITE_TCLAPI pager_stmt_rollback(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_ROLLBACK, 0);
  sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_stmt_commit ID
**
** Commit changes to a checkpoint
*/
अटल पूर्णांक SQLITE_TCLAPI pager_stmt_commit(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerSavepoint(pPager, SAVEPOINT_RELEASE, 0);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_stats ID
**
** Return pager statistics.
*/
अटल पूर्णांक SQLITE_TCLAPI pager_stats(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक i, *a;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  a = sqlite3PagerStats(pPager);
  क्रम(i=0; i<9; i++){
    अटल अक्षर *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl",
    };
    अक्षर zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(माप(zBuf),zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_pagecount ID
**
** Return the size of the database file.
*/
अटल पूर्णांक SQLITE_TCLAPI pager_pagecount(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  अक्षर zBuf[100];
  पूर्णांक nPage;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerPagecount(pPager, &nPage);
  sqlite3_snprintf(माप(zBuf), zBuf, "%d", nPage);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस TCL_OK;
}

/*
** Usage:   page_get ID PGNO
**
** Return a pointer to a page from the database.
*/
अटल पूर्णांक SQLITE_TCLAPI page_get(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage = 0;
  पूर्णांक pgno;
  पूर्णांक rc;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  अगर( Tcl_GetInt(interp, argv[2], &pgno) ) वापस TCL_ERROR;
  rc = sqlite3PagerSharedLock(pPager);
  अगर( rc==SQLITE_OK ){
    rc = sqlite3PagerGet(pPager, pgno, &pPage, 0);
  }
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  Tcl_Append_परिणाम(interp, sqlite3TestPtrToText(pPage), शून्य);
  वापस TCL_OK;
}

/*
** Usage:   page_lookup ID PGNO
**
** Return a pointer to a page if the page is already in cache.
** If not in cache, return an empty string.
*/
अटल पूर्णांक SQLITE_TCLAPI page_lookup(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  DbPage *pPage;
  पूर्णांक pgno;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  अगर( Tcl_GetInt(interp, argv[2], &pgno) ) वापस TCL_ERROR;
  pPage = sqlite3PagerLookup(pPager, pgno);
  अगर( pPage ){
    Tcl_Append_परिणाम(interp, sqlite3TestPtrToText(pPage), शून्य);
  }
  वापस TCL_OK;
}

/*
** Usage:   pager_truncate ID PGNO
*/
अटल पूर्णांक SQLITE_TCLAPI pager_truncate(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Pager *pPager;
  पूर्णांक pgno;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID PGNO\"", शून्य);
    वापस TCL_ERROR;
  }
  pPager = sqlite3TestTextToPtr(argv[1]);
  अगर( Tcl_GetInt(interp, argv[2], &pgno) ) वापस TCL_ERROR;
  sqlite3PagerTruncateImage(pPager, pgno);
  वापस TCL_OK;
}


/*
** Usage:   page_unref PAGE
**
** Drop a pointer to a page.
*/
अटल पूर्णांक SQLITE_TCLAPI page_unref(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  DbPage *pPage;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", शून्य);
    वापस TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3PagerUnref(pPage);
  वापस TCL_OK;
}

/*
** Usage:   page_read PAGE
**
** Return the content of a page
*/
अटल पूर्णांक SQLITE_TCLAPI page_read(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  अक्षर zBuf[100];
  DbPage *pPage;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", शून्य);
    वापस TCL_ERROR;
  }
  pPage = sqlite3TestTextToPtr(argv[1]);
  स_नकल(zBuf, sqlite3_Pager_Get_तथ्य(pPage), माप(zBuf));
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस TCL_OK;
}

/*
** Usage:   page_number PAGE
**
** Return the page number for a page.
*/
अटल पूर्णांक SQLITE_TCLAPI page_संख्या(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  अक्षर zBuf[100];
  DbPage *pPage;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " PAGE\"", शून्य);
    वापस TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  sqlite3_snprintf(माप(zBuf), zBuf, "%d", sqlite3PagerPagenumber(pPage));
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस TCL_OK;
}

/*
** Usage:   page_write PAGE DATA
**
** Write something into a page.
*/
अटल पूर्णांक SQLITE_TCLAPI page_write(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  DbPage *pPage;
  अक्षर *p_तथ्य;
  पूर्णांक rc;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " PAGE DATA\"", शून्य);
    वापस TCL_ERROR;
  }
  pPage = (DbPage *)sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3PagerWrite(pPage);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  p_तथ्य = sqlite3_Pager_Get_तथ्य(pPage);
  म_नकलन(p_तथ्य, argv[2], test_pagesize-1);
  p_तथ्य[test_pagesize-1] = 0;
  वापस TCL_OK;
}

#अगर_अघोषित SQLITE_OMIT_DISKIO
/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
अटल पूर्णांक SQLITE_TCLAPI fake_big_file(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  पूर्णांक rc;
  पूर्णांक n;
  i64 offset;
  अक्षर *zFile;
  पूर्णांक nFile;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", शून्य);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetInt(interp, argv[1], &n) ) वापस TCL_ERROR;
#अगर defined(_WIN32)
  अगर( n>2 ){
    Tcl_Append_परिणाम(interp, "cannot create ", argv[1],
       "MB file because Windows "
       "does not support sparse files", (व्योम*)0);
    वापस TCL_ERROR;
  }
#पूर्ण_अगर

  pVfs = sqlite3_vfs_find(0);
  nFile = (पूर्णांक)म_माप(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  अगर( zFile==0 ) वापस TCL_ERROR;
  स_नकल(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  अगर( rc ){
    Tcl_Append_परिणाम(interp, "open failed: ", sqlite3ErrName(rc), शून्य);
    sqlite3_free(zFile);
    वापस TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  अगर( rc ){
    Tcl_Append_परिणाम(interp, "write failed: ", sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}
#पूर्ण_अगर


/*
** test_control_pending_byte  PENDING_BYTE
**
** Set the PENDING_BYTE using the sqlite3_test_control() interface.
*/
अटल पूर्णांक SQLITE_TCLAPI testPendingByte(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक pbyte;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
                     " PENDING-BYTE\"", (व्योम*)0);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetInt(interp, argv[1], &pbyte) ) वापस TCL_ERROR;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, pbyte);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** The sqlite3FaultSim() callback:
*/
अटल Tcl_Interp *faultSimInterp = 0;
अटल पूर्णांक fault_Sim_Script_आकार = 0;
अटल अक्षर *faultSimScript;
अटल पूर्णांक faultSimCallback(पूर्णांक x){
  अक्षर zInt[30];
  पूर्णांक i;
  पूर्णांक isNeg;
  पूर्णांक rc;
  अगर( x==0 ){
    स_नकल(faultSimScript+fault_Sim_Script_आकार, "0", 2);
  }अन्यथा{
    /* Convert x to text without using any sqlite3 routines */
    अगर( x<0 ){
      isNeg = 1;
      x = -x;
    }अन्यथा{
      isNeg = 0;
    }
    zInt[माप(zInt)-1] = 0;
    क्रम(i=माप(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    अगर( isNeg ) zInt[i--] = '-';
    स_नकल(faultSimScript+fault_Sim_Script_आकार, zInt+i+1, माप(zInt)-i-1);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  अगर( rc ){
    ख_लिखो(मानक_त्रुटि, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = म_से_प(Tcl_Get_String_परिणाम(faultSimInterp));
  }
  Tcl_Reset_परिणाम(faultSimInterp);
  वापस rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
अटल पूर्णांक SQLITE_TCLAPI faultInstallCmd(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  स्थिर अक्षर *zScript;
  पूर्णांक nScript;
  पूर्णांक rc;
  अगर( argc!=1 && argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (व्योम*)0);
    वापस SQLITE_ERROR;
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (पूर्णांक)म_माप(zScript);
  अगर( faultSimScript ){
    मुक्त(faultSimScript);
    faultSimScript = 0;
  }
  अगर( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }अन्यथा{
    faultSimScript = दो_स्मृति( nScript+100 );
    अगर( faultSimScript==0 ){
      Tcl_Append_परिणाम(interp, "out of memory", (व्योम*)0);
      वापस SQLITE_ERROR;
    }
    स_नकल(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    fault_Sim_Script_आकार = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
अटल पूर्णांक SQLITE_TCLAPI testBitvecBuiltinTest(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक sz, rc;
  पूर्णांक nProg = 0;
  पूर्णांक aProg[100];
  स्थिर अक्षर *z;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (व्योम*)0);
  }
  अगर( Tcl_GetInt(interp, argv[1], &sz) ) वापस TCL_ERROR;
  z = argv[2];
  जबतक( nProg<99 && *z ){
    जबतक( *z && !sqlite3Isdigit(*z) ){ z++; }
    अगर( *z==0 ) अवरोध;
    aProg[nProg++] = म_से_प(z);
    जबतक( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}  

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest2_Init(Tcl_Interp *interp){
  बाह्य पूर्णांक sqlite3_io_error_persist;
  बाह्य पूर्णांक sqlite3_io_error_pending;
  बाह्य पूर्णांक sqlite3_io_error_hit;
  बाह्य पूर्णांक sqlite3_io_error_hardhit;
  बाह्य पूर्णांक sqlite3_diskfull_pending;
  बाह्य पूर्णांक sqlite3_diskfull;
  अटल काष्ठा {
    अक्षर *zName;
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
    { "page_get",                (Tcl_CmdProc*)page_get            },
    { "page_lookup",             (Tcl_CmdProc*)page_lookup         },
    { "page_unref",              (Tcl_CmdProc*)page_unref          },
    { "page_read",               (Tcl_CmdProc*)page_read           },
    { "page_write",              (Tcl_CmdProc*)page_write          },
    { "page_number",             (Tcl_CmdProc*)page_संख्या         },
    { "pager_truncate",          (Tcl_CmdProc*)pager_truncate      },
#अगर_अघोषित SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#पूर्ण_अगर
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte",  (Tcl_CmdProc*)testPendingByte },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  Tcl_LinkVar(interp, "sqlite_io_error_pending",
     (अक्षर*)&sqlite3_io_error_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_persist",
     (अक्षर*)&sqlite3_io_error_persist, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hit",
     (अक्षर*)&sqlite3_io_error_hit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_io_error_hardhit",
     (अक्षर*)&sqlite3_io_error_hardhit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull_pending",
     (अक्षर*)&sqlite3_diskfull_pending, TCL_LINK_INT);
  Tcl_LinkVar(interp, "sqlite_diskfull",
     (अक्षर*)&sqlite3_diskfull, TCL_LINK_INT);
#अगर_अघोषित SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (अक्षर*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#पूर्ण_अगर
  वापस TCL_OK;
}
