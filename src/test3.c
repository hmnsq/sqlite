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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "btreeInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ಸ್ಥಿರ sqlite3 sDb;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_open(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ rc, nCache;
  ಪೂರ್ಣಾಙ್ಕ n;
  ವರ್ಣ *zFilename;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( Tcl_GetInt(interp, argv[2], &nCache) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  nRefSqlite3++;
  ಯದಿ( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (ಪೂರ್ಣಾಙ್ಕ)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  ಯದಿ( zFilename==0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_close(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  nRefSqlite3--;
  ಯದಿ( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_begin_transaction(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_pager_stats(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ *a;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
 
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
  ಪ್ರತಿ(i=0; i<11; i++){
    ಸ್ಥಿರ ವರ್ಣ *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    ವರ್ಣ zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_cursor(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ iTable;
  BtCursor *pCur;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ wrFlag;

  ಯದಿ( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  ಯದಿ( Tcl_GetInt(interp, argv[2], &iTable) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  memset(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ಯದಿ( rc ){
    ckfree((ವರ್ಣ *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_close_cursor(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#ಯದಿ SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#ಅನ್ಯಥಾ
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((ವರ್ಣ *)pCur);
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_next(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ res = 0;
  ವರ್ಣ zBuf[100];

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  ಯದಿ( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_first(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ res = 0;
  ವರ್ಣ zBuf[100];

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_eof(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ವರ್ಣ zBuf[50];

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_payload_size(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  ವರ್ಣ zBuf[50];

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_varint_test(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  u32 start, mult, ಎಣಿಕೆ, incr;
  u64 in, out;
  ಪೂರ್ಣಾಙ್ಕ n1, n2, i, j;
  unsigned ವರ್ಣ zBuf[100];
  ಯದಿ( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetInt(interp, argv[1], (ಪೂರ್ಣಾಙ್ಕ*)&start) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetInt(interp, argv[2], (ಪೂರ್ಣಾಙ್ಕ*)&mult) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetInt(interp, argv[3], (ಪೂರ್ಣಾಙ್ಕ*)&ಎಣಿಕೆ) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetInt(interp, argv[4], (ಪೂರ್ಣಾಙ್ಕ*)&incr) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  in = start;
  in *= mult;
  ಪ್ರತಿ(i=0; i<(ಪೂರ್ಣಾಙ್ಕ)ಎಣಿಕೆ; i++){
    ವರ್ಣ zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    ಯದಿ( n1>9 || n1<1 ){
      sqlite3_snprintf(ಪರಿಮಾಣ(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    ಯದಿ( n1!=n2 ){
      sqlite3_snprintf(ಪರಿಮಾಣ(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    ಯದಿ( in!=out ){
      sqlite3_snprintf(ಪರಿಮಾಣ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    ಯದಿ( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      ಯದಿ( n1!=n2 ){
        sqlite3_snprintf(ಪರಿಮಾಣ(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, ರಿಕ್ತ);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( in!=out ){
        sqlite3_snprintf(ಪರಿಮಾಣ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, ರಿಕ್ತ);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    ಪ್ರತಿ(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_from_db(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ iDb = 0;

  ಯದಿ( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  ಪ್ರತಿಜ್ಞಾ( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (ವರ್ಣ*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_ismemdb(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಙ್ಕ res;
  sqlite3_file *pFile;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_set_cache_size(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ nCache;
  Btree *pBt;
  
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  ಯದಿ( Tcl_GetInt(interp, argv[2], &nCache) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI btree_ಸೇರಿಸು(
  ClientData clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *ಧ್ರುವ objv[]
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಙ್ಕ rc;
  BtreePayload x;
  Tcl_Size n;

  ಯದಿ( objc!=4 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  memset(&x, 0, ಪರಿಮಾಣ(x));
  ಯದಿ( objc==4 ){
    ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &rc) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    x.n_ಕೀಲಿ = rc;
    x.pData = (ಶೂನ್ಯ*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (ಪೂರ್ಣಾಙ್ಕ)n;
  }ಅನ್ಯಥಾ{
    x.p_ಕೀಲಿ = (ಶೂನ್ಯ*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_ಕೀಲಿ = (ಪೂರ್ಣಾಙ್ಕ)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ಸೇರಿಸು(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest3_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_next               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_size       },
     { "btree_first",              (Tcl_CmdProc*)btree_first              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_set_cache_size     }
  };
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_ಸೇರಿಸು, 0, 0);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
