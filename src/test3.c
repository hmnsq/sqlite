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
** Code for testing the btree.c module in SQLite.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
*/
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "btreeInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
స్థిర sqlite3 sDb;
స్థిర పూర్ణాంకం nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_open(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Btree *pBt;
  పూర్ణాంకం rc, nCache;
  పూర్ణాంకం n;
  అక్షరం *zFilename;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( Tcl_GetInt(interp, argv[2], &nCache) ) తిరిగి TCL_ERROR;
  nRefSqlite3++;
  ఒకవేళ( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_కనుగొను(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  ఒకవేళ( zFilename==0 ) తిరిగి TCL_ERROR;
  జ్ఞాపకం_నకలు(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  sqlite3_Btree_సమితి_Cache_కొలత(pBt, nCache);
  Tcl_Append_ఫలితం(interp, sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(pBt), ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_close(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Btree *pBt;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pBt = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  nRefSqlite3--;
  ఒకవేళ( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  తిరిగి TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_begin_transaction(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Btree *pBt;
  పూర్ణాంకం rc;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pBt = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_pager_stats(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Btree *pBt;
  పూర్ణాంకం i;
  పూర్ణాంకం *a;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pBt = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
 
  /* Normally in this file, with a b-tree handle opened using the 
  ** [btree_open] command it is safe to call sqlite3BtreeEnter() directly.
  ** But this function is sometimes called with a btree handle obtained
  ** from an open SQLite connection (using [btree_from_db]). In this case
  ** we need to obtain the mutex for the controlling SQLite handle before
  ** it is safe to call sqlite3BtreeEnter().
  */
  sqlite3_mutex_enter(pBt->db->mutex);

  sqlite3BtreeEnter(pBt);
  a = sqlite3PagerStats(sqlite3BtreePager(pBt));
  కొరకు(i=0; i<11; i++){
    స్థిర అక్షరం *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    అక్షరం zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(పరిమాణం(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  తిరిగి TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_cursor(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Btree *pBt;
  పూర్ణాంకం i_పట్టిక;
  BtCursor *pCur;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం wrFlag;

  ఒకవేళ( argc!=4 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pBt = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &i_పట్టిక) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) తిరిగి TCL_ERROR;
  ఒకవేళ( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_కొలత());
  memset(pCur, 0, sqlite3_Btree_Cursor_కొలత());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3_Btree_తాళం_పట్టిక(pBt, i_పట్టిక, !!wrFlag);
#అయితే_ముగింపు
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, i_పట్టిక, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ఒకవేళ( rc ){
    ckfree((అక్షరం *)pCur);
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  Tcl_Append_ఫలితం(interp, sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(pCur), ఖాళీ);
  తిరిగి SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_close_cursor(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  BtCursor *pCur;
  పూర్ణాంకం rc;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pCur = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
#ఒకవేళ SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#లేకపోతే
  rc = sqlite3BtreeCloseCursor(pCur);
#అయితే_ముగింపు
  ckfree((అక్షరం *)pCur);
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_తదుపరి_2(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  BtCursor *pCur;
  పూర్ణాంకం rc;
  పూర్ణాంకం res = 0;
  అక్షరం zBuf[100];

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pCur = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_తదుపరి(pCur, 0);
  ఒకవేళ( rc==SQLITE_పూర్తి ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  sqlite3_snprintf(పరిమాణం(zBuf),zBuf,"%d",res);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_మొదటి(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  BtCursor *pCur;
  పూర్ణాంకం rc;
  పూర్ణాంకం res = 0;
  అక్షరం zBuf[100];

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pCur = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_మొదటి(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  sqlite3_snprintf(పరిమాణం(zBuf),zBuf,"%d",res);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_eof(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  BtCursor *pCur;
  పూర్ణాంకం rc;
  అక్షరం zBuf[50];

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pCur = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(పరిమాణం(zBuf),zBuf, "%d", rc);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_payload_కొలత(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  అక్షరం zBuf[50];

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pCur = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_కొలత(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(పరిమాణం(zBuf),zBuf, "%u", n);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి SQLITE_OK;
}

/*
** usage:   varint_test  START  MULTIPLIER  COUNT  INCREMENT
**
** This command tests the sqlite3PutVarint() and sqlite3GetVarint()
** routines, both for accuracy and for speed.
**
** An integer is written using sqlite3PutVarint() and read back with
** sqlite3GetVarint() and verified to be unchanged.  This repeats COUNT
** times.  The first integer is START*MULTIPLIER.  Each iteration
** increases the integer by INCREMENT.
**
** This command returns nothing if it works.  It returns an error message
** if something goes wrong.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_varint_test(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  u32 ప్రారంభం, mult, లెక్క, incr;
  u64 లోపల, బహిర్గతం;
  పూర్ణాంకం n1, n2, i, j;
  గుర్తులేని అక్షరం zBuf[100];
  ఒకవేళ( argc!=5 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[1], (పూర్ణాంకం*)&ప్రారంభం) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetInt(interp, argv[2], (పూర్ణాంకం*)&mult) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetInt(interp, argv[3], (పూర్ణాంకం*)&లెక్క) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetInt(interp, argv[4], (పూర్ణాంకం*)&incr) ) తిరిగి TCL_ERROR;
  లోపల = ప్రారంభం;
  లోపల *= mult;
  కొరకు(i=0; i<(పూర్ణాంకం)లెక్క; i++){
    అక్షరం zErr[200];
    n1 = sqlite3PutVarint(zBuf, లోపల);
    ఒకవేళ( n1>9 || n1<1 ){
      sqlite3_snprintf(పరిమాణం(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_ఫలితం(interp, zErr, ఖాళీ);
      తిరిగి TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &బహిర్గతం);
    ఒకవేళ( n1!=n2 ){
      sqlite3_snprintf(పరిమాణం(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_ఫలితం(interp, zErr, ఖాళీ);
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( లోపల!=బహిర్గతం ){
      sqlite3_snprintf(పరిమాణం(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", లోపల, బహిర్గతం);
      Tcl_Append_ఫలితం(interp, zErr, ఖాళీ);
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( (లోపల & 0xffffffff)==లోపల ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      బహిర్గతం = out32;
      ఒకవేళ( n1!=n2 ){
        sqlite3_snprintf(పరిమాణం(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_ఫలితం(interp, zErr, ఖాళీ);
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( లోపల!=బహిర్గతం ){
        sqlite3_snprintf(పరిమాణం(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            లోపల, బహిర్గతం);
        Tcl_Append_ఫలితం(interp, zErr, ఖాళీ);
        తిరిగి TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    కొరకు(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &బహిర్గతం);
    }
    లోపల += incr;
  }
  తిరిగి TCL_OK;
}

/*
** usage:   btree_from_db  DB-HANDLE
**
** This command returns the btree handle for the main database associated
** with the database-handle passed as the argument. Example usage:
**
** sqlite3 db test.db
** set bt [btree_from_db db]
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_from_db(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  పూర్ణాంకం iDb = 0;

  ఒకవేళ( argc!=2 && argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_ఫలితం(interp, "No such db-handle: \"", argv[1], "\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_దత్తాంశం);
  ప్రతిజ్ఞా( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_సమితి_ఫలితం(interp, (అక్షరం*)sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(pBt), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_ismemdb(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  Btree *pBt;
  పూర్ణాంకం res;
  sqlite3_file *pFile;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pBt = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewBooleanObj(res));
  తిరిగి SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_సమితి_cache_కొలత(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం nCache;
  Btree *pBt;
  
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  pBt = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[1]);
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &nCache) ) తిరిగి TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_సమితి_Cache_కొలత(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  తిరిగి TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI btree_చొప్పించు(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *స్థిరరాశి objv[]
){
  BtCursor *pCur;
  పూర్ణాంకం rc;
  BtreePayload x;
  Tcl_కొలత n;

  ఒకవేళ( objc!=4 && objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    తిరిగి TCL_ERROR;
  }

  memset(&x, 0, పరిమాణం(x));
  ఒకవేళ( objc==4 ){
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &rc) ) తిరిగి TCL_ERROR;
    x.n_కీలకం = rc;
    x.p_దత్తాంశం = (శూన్య*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_దత్తాంశం = (పూర్ణాంకం)n;
  }లేకపోతే{
    x.p_కీలకం = (శూన్య*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_కీలకం = (పూర్ణాంకం)n;
  }
  pCur = (BtCursor*)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_చొప్పించు(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_ఫలితం(interp);
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest3_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_తదుపరి_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_కొలత       },
     { "btree_first",              (Tcl_CmdProc*)btree_మొదటి              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_సమితి_cache_కొలత     }
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_చొప్పించు, 0, 0);

  తిరిగి TCL_OK;
}
