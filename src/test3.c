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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "btreeInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
സ്ഥിതം sqlite3 sDb;
സ്ഥിതം പൂർണ്ണസംഖ്യ nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_open(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Btree *pBt;
  പൂർണ്ണസംഖ്യ rc, nCache;
  പൂർണ്ണസംഖ്യ n;
  അക്ഷരം *zFilename;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &nCache) ) തിരികെ TCL_ERROR;
  nRefSqlite3++;
  എങ്കിൽ( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (പൂർണ്ണസംഖ്യ)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  എങ്കിൽ( zFilename==0 ) തിരികെ TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_close(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Btree *pBt;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  nRefSqlite3--;
  എങ്കിൽ( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  തിരികെ TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_begin_transaction(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Btree *pBt;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_pager_stats(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Btree *pBt;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ *a;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
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
  വേണ്ടി(i=0; i<11; i++){
    സ്ഥിതം അക്ഷരം *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    അക്ഷരം zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(വലുപ്പം(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  തിരികെ TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_cursor(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Btree *pBt;
  പൂർണ്ണസംഖ്യ iTable;
  BtCursor *pCur;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ wrFlag;

  എങ്കിൽ( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &iTable) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  memset(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  എങ്കിൽ( rc ){
    ckfree((അക്ഷരം *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), ശൂന്യ);
  തിരികെ SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_close_cursor(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  BtCursor *pCur;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#എങ്കിൽ SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#അല്ലെങ്കിൽ
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((അക്ഷരം *)pCur);
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_next(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  BtCursor *pCur;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ res = 0;
  അക്ഷരം zBuf[100];

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  എങ്കിൽ( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  sqlite3_snprintf(വലുപ്പം(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_first(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  BtCursor *pCur;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ res = 0;
  അക്ഷരം zBuf[100];

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  sqlite3_snprintf(വലുപ്പം(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_eof(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  BtCursor *pCur;
  പൂർണ്ണസംഖ്യ rc;
  അക്ഷരം zBuf[50];

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(വലുപ്പം(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_payload_size(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  അക്ഷരം zBuf[50];

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(വലുപ്പം(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_varint_test(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  u32 start, mult, എണ്ണം, incr;
  u64 in, out;
  പൂർണ്ണസംഖ്യ n1, n2, i, j;
  unsigned അക്ഷരം zBuf[100];
  എങ്കിൽ( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[1], (പൂർണ്ണസംഖ്യ*)&start) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], (പൂർണ്ണസംഖ്യ*)&mult) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetInt(interp, argv[3], (പൂർണ്ണസംഖ്യ*)&എണ്ണം) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetInt(interp, argv[4], (പൂർണ്ണസംഖ്യ*)&incr) ) തിരികെ TCL_ERROR;
  in = start;
  in *= mult;
  വേണ്ടി(i=0; i<(പൂർണ്ണസംഖ്യ)എണ്ണം; i++){
    അക്ഷരം zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    എങ്കിൽ( n1>9 || n1<1 ){
      sqlite3_snprintf(വലുപ്പം(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    എങ്കിൽ( n1!=n2 ){
      sqlite3_snprintf(വലുപ്പം(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( in!=out ){
      sqlite3_snprintf(വലുപ്പം(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      എങ്കിൽ( n1!=n2 ){
        sqlite3_snprintf(വലുപ്പം(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, ശൂന്യ);
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( in!=out ){
        sqlite3_snprintf(വലുപ്പം(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, ശൂന്യ);
        തിരികെ TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    വേണ്ടി(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_from_db(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  പൂർണ്ണസംഖ്യ iDb = 0;

  എങ്കിൽ( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  പ്രതിജ്ഞാ( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (അക്ഷരം*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_ismemdb(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  Btree *pBt;
  പൂർണ്ണസംഖ്യ res;
  sqlite3_file *pFile;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  തിരികെ SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_set_cache_size(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ nCache;
  Btree *pBt;
  
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &nCache) ) തിരികെ TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  തിരികെ TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI btree_തിരുകുക(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *സ്ഥിരം objv[]
){
  BtCursor *pCur;
  പൂർണ്ണസംഖ്യ rc;
  BtreePayload x;
  Tcl_Size n;

  എങ്കിൽ( objc!=4 && objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    തിരികെ TCL_ERROR;
  }

  memset(&x, 0, വലുപ്പം(x));
  എങ്കിൽ( objc==4 ){
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &rc) ) തിരികെ TCL_ERROR;
    x.n_താക്കോൽ = rc;
    x.pData = (ശൂന്യം*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (പൂർണ്ണസംഖ്യ)n;
  }അല്ലെങ്കിൽ{
    x.p_താക്കോൽ = (ശൂന്യം*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_താക്കോൽ = (പൂർണ്ണസംഖ്യ)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_തിരുകുക(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest3_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
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
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_തിരുകുക, 0, 0);

  തിരികെ TCL_OK;
}
