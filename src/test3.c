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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "btreeInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
நிலையான sqlite3 sDb;
நிலையான முழு nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
நிலையான முழு SQLITE_TCLAPI btree_open(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Btree *pBt;
  முழு rc, nCache;
  முழு n;
  எழுத்து *zFilename;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", வெற்று);
    திரும்பு TCL_ERROR;
  }

  எனில்( Tcl_GetInt(interp, argv[2], &nCache) ) திரும்பு TCL_ERROR;
  nRefSqlite3++;
  எனில்( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (முழு)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  எனில்( zFilename==0 ) திரும்பு TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
நிலையான முழு SQLITE_TCLAPI btree_close(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Btree *pBt;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  nRefSqlite3--;
  எனில்( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  திரும்பு TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
நிலையான முழு SQLITE_TCLAPI btree_begin_transaction(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Btree *pBt;
  முழு rc;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  எனில்( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
நிலையான முழு SQLITE_TCLAPI btree_pager_stats(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Btree *pBt;
  முழு i;
  முழு *a;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
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
  க்கு(i=0; i<11; i++){
    நிலையான எழுத்து *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    எழுத்து zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(அளவு(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  திரும்பு TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
நிலையான முழு SQLITE_TCLAPI btree_cursor(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Btree *pBt;
  முழு iTable;
  BtCursor *pCur;
  முழு rc = SQLITE_OK;
  முழு wrFlag;

  எனில்( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  எனில்( Tcl_GetInt(interp, argv[2], &iTable) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) திரும்பு TCL_ERROR;
  எனில்( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  memset(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  எனில்( rc ){
    ckfree((எழுத்து *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), வெற்று);
  திரும்பு SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
நிலையான முழு SQLITE_TCLAPI btree_close_cursor(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  BtCursor *pCur;
  முழு rc;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#எனில் SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#இல்லையெனில்
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((எழுத்து *)pCur);
  எனில்( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
நிலையான முழு SQLITE_TCLAPI btree_next(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  BtCursor *pCur;
  முழு rc;
  முழு res = 0;
  எழுத்து zBuf[100];

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  எனில்( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  எனில்( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  sqlite3_snprintf(அளவு(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
நிலையான முழு SQLITE_TCLAPI btree_first(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  BtCursor *pCur;
  முழு rc;
  முழு res = 0;
  எழுத்து zBuf[100];

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  எனில்( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  sqlite3_snprintf(அளவு(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
நிலையான முழு SQLITE_TCLAPI btree_eof(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  BtCursor *pCur;
  முழு rc;
  எழுத்து zBuf[50];

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(அளவு(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
நிலையான முழு SQLITE_TCLAPI btree_payload_size(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  எழுத்து zBuf[50];

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(அளவு(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு SQLITE_OK;
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
நிலையான முழு SQLITE_TCLAPI btree_varint_test(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  u32 start, mult, மொத்தம், incr;
  u64 in, out;
  முழு n1, n2, i, j;
  unsigned எழுத்து zBuf[100];
  எனில்( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetInt(interp, argv[1], (முழு*)&start) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetInt(interp, argv[2], (முழு*)&mult) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetInt(interp, argv[3], (முழு*)&மொத்தம்) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetInt(interp, argv[4], (முழு*)&incr) ) திரும்பு TCL_ERROR;
  in = start;
  in *= mult;
  க்கு(i=0; i<(முழு)மொத்தம்; i++){
    எழுத்து zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    எனில்( n1>9 || n1<1 ){
      sqlite3_snprintf(அளவு(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, வெற்று);
      திரும்பு TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    எனில்( n1!=n2 ){
      sqlite3_snprintf(அளவு(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, வெற்று);
      திரும்பு TCL_ERROR;
    }
    எனில்( in!=out ){
      sqlite3_snprintf(அளவு(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, வெற்று);
      திரும்பு TCL_ERROR;
    }
    எனில்( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      எனில்( n1!=n2 ){
        sqlite3_snprintf(அளவு(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, வெற்று);
        திரும்பு TCL_ERROR;
      }
      எனில்( in!=out ){
        sqlite3_snprintf(அளவு(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, வெற்று);
        திரும்பு TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    க்கு(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI btree_from_db(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  முழு iDb = 0;

  எனில்( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", வெற்று);
    திரும்பு TCL_ERROR;
  }

  எனில்( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  ப்ரதிஜ்ஞா( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (எழுத்து*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
நிலையான முழு SQLITE_TCLAPI btree_ismemdb(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  Btree *pBt;
  முழு res;
  sqlite3_file *pFile;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  திரும்பு SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
நிலையான முழு SQLITE_TCLAPI btree_set_cache_size(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு nCache;
  Btree *pBt;
  
  எனில்( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", வெற்று);
    திரும்பு TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  எனில்( Tcl_GetInt(interp, argv[2], &nCache) ) திரும்பு TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  திரும்பு TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
நிலையான முழு SQLITE_TCLAPI btree_செருகு(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *மாறிலி objv[]
){
  BtCursor *pCur;
  முழு rc;
  BtreePayload x;
  Tcl_Size n;

  எனில்( objc!=4 && objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    திரும்பு TCL_ERROR;
  }

  memset(&x, 0, அளவு(x));
  எனில்( objc==4 ){
    எனில்( Tcl_GetIntFromObj(interp, objv[2], &rc) ) திரும்பு TCL_ERROR;
    x.n_சாவி = rc;
    x.pData = (வெறுமை*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (முழு)n;
  }இல்லையெனில்{
    x.p_சாவி = (வெறுமை*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_சாவி = (முழு)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_செருகு(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  எனில்( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), வெற்று);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest3_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
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
  முழு i;

  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_செருகு, 0, 0);

  திரும்பு TCL_OK;
}
