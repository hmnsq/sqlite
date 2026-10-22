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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "btreeInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ସ୍ଥିର sqlite3 sDb;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_open(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, nCache;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅକ୍ଷର *zFilename;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( Tcl_GetInt(interp, argv[2], &nCache) ) ଫେରନ୍ତୁ TCL_ERROR;
  nRefSqlite3++;
  ଯଦି( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  ଯଦି( zFilename==0 ) ଫେରନ୍ତୁ TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_close(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  nRefSqlite3--;
  ଯଦି( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_begin_transaction(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_pager_stats(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *a;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
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
  ପାଇଁ(i=0; i<11; i++){
    ସ୍ଥିର ଅକ୍ଷର *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    ଅକ୍ଷର zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ଆକାର(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_cursor(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTable;
  BtCursor *pCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrFlag;

  ଯଦି( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  ଯଦି( Tcl_GetInt(interp, argv[2], &iTable) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  memset(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ଯଦି( rc ){
    ckfree((ଅକ୍ଷର *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), ଖାଲି);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_close_cursor(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#ଯଦି SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#ଅନ୍ୟଥା
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((ଅକ୍ଷର *)pCur);
  ଯଦି( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_next(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  ଅକ୍ଷର zBuf[100];

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  ଯଦି( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  ଯଦି( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_snprintf(ଆକାର(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_first(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  ଅକ୍ଷର zBuf[100];

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  ଯଦି( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_snprintf(ଆକାର(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_eof(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅକ୍ଷର zBuf[50];

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ଆକାର(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_payload_size(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  ଅକ୍ଷର zBuf[50];

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ଆକାର(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_varint_test(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  u32 start, mult, ଗଣନା_2, incr;
  u64 in, out;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n1, n2, i, j;
  unsigned ଅକ୍ଷର zBuf[100];
  ଯଦି( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetInt(interp, argv[1], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)&start) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetInt(interp, argv[2], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)&mult) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetInt(interp, argv[3], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)&ଗଣନା_2) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetInt(interp, argv[4], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)&incr) ) ଫେରନ୍ତୁ TCL_ERROR;
  in = start;
  in *= mult;
  ପାଇଁ(i=0; i<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଗଣନା_2; i++){
    ଅକ୍ଷର zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    ଯଦି( n1>9 || n1<1 ){
      sqlite3_snprintf(ଆକାର(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    ଯଦି( n1!=n2 ){
      sqlite3_snprintf(ଆକାର(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( in!=out ){
      sqlite3_snprintf(ଆକାର(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      ଯଦି( n1!=n2 ){
        sqlite3_snprintf(ଆକାର(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, ଖାଲି);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( in!=out ){
        sqlite3_snprintf(ଆକାର(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, ଖାଲି);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    ପାଇଁ(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_from_db(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = 0;

  ଯଦି( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  ପ୍ରତିଜ୍ଞା( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (ଅକ୍ଷର*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_ismemdb(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  Btree *pBt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  sqlite3_file *pFile;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_set_cache_size(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCache;
  Btree *pBt;
  
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  ଯଦି( Tcl_GetInt(interp, argv[2], &nCache) ) ଫେରନ୍ତୁ TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ଫେରନ୍ତୁ TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI btree_ଭର୍ତ୍ତି(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *ଅଚଳ objv[]
){
  BtCursor *pCur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  BtreePayload x;
  Tcl_Size n;

  ଯଦି( objc!=4 && objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  memset(&x, 0, ଆକାର(x));
  ଯଦି( objc==4 ){
    ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &rc) ) ଫେରନ୍ତୁ TCL_ERROR;
    x.n_ଚାବି = rc;
    x.pData = (ଶୂନ୍ୟ*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
  }ଅନ୍ୟଥା{
    x.p_ଚାବି = (ଶୂନ୍ୟ*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_ଚାବି = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ଭର୍ତ୍ତି(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  ଯଦି( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest3_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_ଭର୍ତ୍ତି, 0, 0);

  ଫେରନ୍ତୁ TCL_OK;
}
