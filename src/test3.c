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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "btreeInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ਸਥਿਰ sqlite3 sDb;
ਸਥਿਰ ਪੂਰਨ nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_open(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Btree *pBt;
  ਪੂਰਨ rc, nCache;
  ਪੂਰਨ n;
  ਅੱਖਰ *zFilename;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( Tcl_GetInt(interp, argv[2], &nCache) ) ਵਾਪਸ TCL_ERROR;
  nRefSqlite3++;
  ਜੇ( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_ਲੱਭੋ(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  ਜੇ( zFilename==0 ) ਵਾਪਸ TCL_ERROR;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(pBt, nCache);
  Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(pBt), ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_close(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Btree *pBt;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pBt = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  nRefSqlite3--;
  ਜੇ( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  ਵਾਪਸ TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_begin_transaction(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Btree *pBt;
  ਪੂਰਨ rc;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pBt = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_pager_stats(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Btree *pBt;
  ਪੂਰਨ i;
  ਪੂਰਨ *a;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pBt = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
 
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
  ਲਈ(i=0; i<11; i++){
    ਸਥਿਰ ਅੱਖਰ *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    ਅੱਖਰ zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_cursor(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Btree *pBt;
  ਪੂਰਨ i_ਸਾਰਣੀ;
  BtCursor *pCur;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ wrFlag;

  ਜੇ( argc!=4 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pBt = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  ਜੇ( Tcl_GetInt(interp, argv[2], &i_ਸਾਰਣੀ) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_ਮਾਪ());
  memset(pCur, 0, sqlite3_Btree_Cursor_ਮਾਪ());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3_Btree_ਤਾਲਾ_ਸਾਰਣੀ(pBt, i_ਸਾਰਣੀ, !!wrFlag);
#ਜੇ_ਅੰਤ
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, i_ਸਾਰਣੀ, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ਜੇ( rc ){
    ckfree((ਅੱਖਰ *)pCur);
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(pCur), ਨਿਰਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_close_cursor(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ਪੂਰਨ rc;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pCur = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
#ਜੇ SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#ਨਹੀਂਤਾਂ
  rc = sqlite3BtreeCloseCursor(pCur);
#ਜੇ_ਅੰਤ
  ckfree((ਅੱਖਰ *)pCur);
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_ਅਗਲਾ_2(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ਪੂਰਨ rc;
  ਪੂਰਨ res = 0;
  ਅੱਖਰ zBuf[100];

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pCur = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ਅਗਲਾ(pCur, 0);
  ਜੇ( rc==SQLITE_ਪੂਰਾ ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf,"%d",res);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_ਪਹਿਲਾ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ਪੂਰਨ rc;
  ਪੂਰਨ res = 0;
  ਅੱਖਰ zBuf[100];

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pCur = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ਪਹਿਲਾ(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf,"%d",res);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_eof(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ਪੂਰਨ rc;
  ਅੱਖਰ zBuf[50];

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pCur = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf, "%d", rc);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_payload_ਮਾਪ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  ਅੱਖਰ zBuf[50];

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pCur = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_ਮਾਪ(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf, "%u", n);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_varint_test(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  u32 ਸ਼ੁਰੂ, mult, ਗਿਣਤੀ_2, incr;
  u64 ਵਿੱਚ, ਬਾਹਰਲਾ;
  ਪੂਰਨ n1, n2, i, j;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zBuf[100];
  ਜੇ( argc!=5 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetInt(interp, argv[1], (ਪੂਰਨ*)&ਸ਼ੁਰੂ) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetInt(interp, argv[2], (ਪੂਰਨ*)&mult) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetInt(interp, argv[3], (ਪੂਰਨ*)&ਗਿਣਤੀ_2) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetInt(interp, argv[4], (ਪੂਰਨ*)&incr) ) ਵਾਪਸ TCL_ERROR;
  ਵਿੱਚ = ਸ਼ੁਰੂ;
  ਵਿੱਚ *= mult;
  ਲਈ(i=0; i<(ਪੂਰਨ)ਗਿਣਤੀ_2; i++){
    ਅੱਖਰ zErr[200];
    n1 = sqlite3PutVarint(zBuf, ਵਿੱਚ);
    ਜੇ( n1>9 || n1<1 ){
      sqlite3_snprintf(ਆਕਾਰ(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_ਨਤੀਜਾ(interp, zErr, ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &ਬਾਹਰਲਾ);
    ਜੇ( n1!=n2 ){
      sqlite3_snprintf(ਆਕਾਰ(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_ਨਤੀਜਾ(interp, zErr, ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( ਵਿੱਚ!=ਬਾਹਰਲਾ ){
      sqlite3_snprintf(ਆਕਾਰ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", ਵਿੱਚ, ਬਾਹਰਲਾ);
      Tcl_Append_ਨਤੀਜਾ(interp, zErr, ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( (ਵਿੱਚ & 0xffffffff)==ਵਿੱਚ ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      ਬਾਹਰਲਾ = out32;
      ਜੇ( n1!=n2 ){
        sqlite3_snprintf(ਆਕਾਰ(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_ਨਤੀਜਾ(interp, zErr, ਨਿਰਾ);
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( ਵਿੱਚ!=ਬਾਹਰਲਾ ){
        sqlite3_snprintf(ਆਕਾਰ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            ਵਿੱਚ, ਬਾਹਰਲਾ);
        Tcl_Append_ਨਤੀਜਾ(interp, zErr, ਨਿਰਾ);
        ਵਾਪਸ TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    ਲਈ(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &ਬਾਹਰਲਾ);
    }
    ਵਿੱਚ += incr;
  }
  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_from_db(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  ਪੂਰਨ iDb = 0;

  ਜੇ( argc!=2 && argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "No such db-handle: \"", argv[1], "\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_ਅੰਕੜੇ);
  ਪ੍ਰਤਿਜ੍ਞਾ( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ*)sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(pBt), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_ismemdb(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  Btree *pBt;
  ਪੂਰਨ res;
  sqlite3_file *pFile;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pBt = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewBooleanObj(res));
  ਵਾਪਸ SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_ਸੈੱਟ_cache_ਮਾਪ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ nCache;
  Btree *pBt;
  
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  pBt = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[1]);
  ਜੇ( Tcl_GetInt(interp, argv[2], &nCache) ) ਵਾਪਸ TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ਵਾਪਸ TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI btree_ਪਾਓ(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *ਅਚਲ objv[]
){
  BtCursor *pCur;
  ਪੂਰਨ rc;
  BtreePayload x;
  Tcl_ਮਾਪ n;

  ਜੇ( objc!=4 && objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    ਵਾਪਸ TCL_ERROR;
  }

  memset(&x, 0, ਆਕਾਰ(x));
  ਜੇ( objc==4 ){
    ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &rc) ) ਵਾਪਸ TCL_ERROR;
    x.n_ਕੁੰਜੀ = rc;
    x.p_ਅੰਕੜੇ = (ਖਾਲੀ*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_ਅੰਕੜੇ = (ਪੂਰਨ)n;
  }ਨਹੀਂਤਾਂ{
    x.p_ਕੁੰਜੀ = (ਖਾਲੀ*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_ਕੁੰਜੀ = (ਪੂਰਨ)n;
  }
  pCur = (BtCursor*)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ਪਾਓ(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_ਨਤੀਜਾ(interp);
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest3_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_ਅਗਲਾ_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_ਮਾਪ       },
     { "btree_first",              (Tcl_CmdProc*)btree_ਪਹਿਲਾ              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_ਸੈੱਟ_cache_ਮਾਪ     }
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_ਪਾਓ, 0, 0);

  ਵਾਪਸ TCL_OK;
}
