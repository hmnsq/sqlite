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

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ಸ್ಥಿರ sqlite3 sDb;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_open(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ rc, nCache;
  ಪೂರ್ಣಾಂಕ n;
  ಅಕ್ಷರ *zFilename;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( Tcl_GetInt(interp, argv[2], &nCache) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  nRefSqlite3++;
  ಆದರೆ( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  ಆದರೆ( zFilename==0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಸ್ಮರಣೆ_ನಕಲು(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(pBt, nCache);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(pBt), ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_close(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pBt = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  nRefSqlite3--;
  ಆದರೆ( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_begin_transaction(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pBt = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_pager_stats(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ *a;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pBt = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
 
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
  ಗಾಗಿ(i=0; i<11; i++){
    ಸ್ಥಿರ ಅಕ್ಷರ *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    ಅಕ್ಷರ zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_cursor(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ;
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ wrFlag;

  ಆದರೆ( argc!=4 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pBt = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &i_ಕೋಷ್ಟಕ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_ಗಾತ್ರ());
  memset(pCur, 0, sqlite3_Btree_Cursor_ಗಾತ್ರ());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3_Btree_ಬೀಗ_ಕೋಷ್ಟಕ(pBt, i_ಕೋಷ್ಟಕ, !!wrFlag);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, i_ಕೋಷ್ಟಕ, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ಆದರೆ( rc ){
    ckfree((ಅಕ್ಷರ *)pCur);
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(pCur), ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_close_cursor(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pCur = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
#ಆದರೆ SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#ಇಲ್ಲದಿದ್ದರೆ
  rc = sqlite3BtreeCloseCursor(pCur);
#ಆದರೆ_ಅಂತ್ಯ
  ckfree((ಅಕ್ಷರ *)pCur);
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_ಮುಂದಿನ_2(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ res = 0;
  ಅಕ್ಷರ zBuf[100];

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pCur = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ಮುಂದಿನ(pCur, 0);
  ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf,"%d",res);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_ಮೊದಲ(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ res = 0;
  ಅಕ್ಷರ zBuf[100];

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pCur = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ಮೊದಲ(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf,"%d",res);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_eof(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ zBuf[50];

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pCur = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf, "%d", rc);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_payload_ಗಾತ್ರ(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  ಅಕ್ಷರ zBuf[50];

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pCur = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_ಗಾತ್ರ(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf, "%u", n);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_varint_test(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  u32 ಆರಂಭ, mult, ಎಣಿಕೆ_2, incr;
  u64 ಒಳ, ಹೊರಹೋಗು;
  ಪೂರ್ಣಾಂಕ n1, n2, i, j;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zBuf[100];
  ಆದರೆ( argc!=5 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[1], (ಪೂರ್ಣಾಂಕ*)&ಆರಂಭ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetInt(interp, argv[2], (ಪೂರ್ಣಾಂಕ*)&mult) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetInt(interp, argv[3], (ಪೂರ್ಣಾಂಕ*)&ಎಣಿಕೆ_2) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetInt(interp, argv[4], (ಪೂರ್ಣಾಂಕ*)&incr) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಒಳ = ಆರಂಭ;
  ಒಳ *= mult;
  ಗಾಗಿ(i=0; i<(ಪೂರ್ಣಾಂಕ)ಎಣಿಕೆ_2; i++){
    ಅಕ್ಷರ zErr[200];
    n1 = sqlite3PutVarint(zBuf, ಒಳ);
    ಆದರೆ( n1>9 || n1<1 ){
      sqlite3_snprintf(ಗಾತ್ರ(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &ಹೊರಹೋಗು);
    ಆದರೆ( n1!=n2 ){
      sqlite3_snprintf(ಗಾತ್ರ(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( ಒಳ!=ಹೊರಹೋಗು ){
      sqlite3_snprintf(ಗಾತ್ರ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", ಒಳ, ಹೊರಹೋಗು);
      Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( (ಒಳ & 0xffffffff)==ಒಳ ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      ಹೊರಹೋಗು = out32;
      ಆದರೆ( n1!=n2 ){
        sqlite3_snprintf(ಗಾತ್ರ(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, ಖಾಲಿ);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( ಒಳ!=ಹೊರಹೋಗು ){
        sqlite3_snprintf(ಗಾತ್ರ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            ಒಳ, ಹೊರಹೋಗು);
        Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, ಖಾಲಿ);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    ಗಾಗಿ(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &ಹೊರಹೋಗು);
    }
    ಒಳ += incr;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_from_db(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ iDb = 0;

  ಆದರೆ( argc!=2 && argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "No such db-handle: \"", argv[1], "\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_ದತ್ತಾಂಶ);
  ಪ್ರತಿಜ್ಞಾ( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ*)sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(pBt), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_ismemdb(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  Btree *pBt;
  ಪೂರ್ಣಾಂಕ res;
  sqlite3_file *pFile;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pBt = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewBooleanObj(res));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_ಗಣ_cache_ಗಾತ್ರ(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ nCache;
  Btree *pBt;
  
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pBt = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[1]);
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &nCache) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ಹಿಂತಿರುಗಿ TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI btree_ಸೇರಿಸು(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ objv[]
){
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ rc;
  BtreePayload x;
  Tcl_ಗಾತ್ರ n;

  ಆದರೆ( objc!=4 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  memset(&x, 0, ಗಾತ್ರ(x));
  ಆದರೆ( objc==4 ){
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &rc) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    x.n_ಕೀಲಿ = rc;
    x.p_ದತ್ತಾಂಶ = (ಶೂನ್ಯ*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_ದತ್ತಾಂಶ = (ಪೂರ್ಣಾಂಕ)n;
  }ಇಲ್ಲದಿದ್ದರೆ{
    x.p_ಕೀಲಿ = (ಶೂನ್ಯ*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_ಕೀಲಿ = (ಪೂರ್ಣಾಂಕ)n;
  }
  pCur = (BtCursor*)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ಸೇರಿಸು(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest3_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_ಮುಂದಿನ_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_ಗಾತ್ರ       },
     { "btree_first",              (Tcl_CmdProc*)btree_ಮೊದಲ              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_ಗಣ_cache_ಗಾತ್ರ     }
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_ಸೇರಿಸು, 0, 0);

  ಹಿಂತಿರುಗಿ TCL_OK;
}
