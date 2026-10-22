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
#شامل "sqliteInt.h"
#شامل "btreeInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ایستا sqlite3 sDb;
ایستا صحیح nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ایستا صحیح SQLITE_TCLAPI btree_open(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح rc, nCache;
  صحیح n;
  نویسه *zFilename;
  اگر( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", پوچ);
    بازگشت TCL_ERROR;
  }

  اگر( Tcl_GetInt(interp, argv[2], &nCache) ) بازگشت TCL_ERROR;
  nRefSqlite3++;
  اگر( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (صحیح)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  اگر( zFilename==0 ) بازگشت TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ایستا صحیح SQLITE_TCLAPI btree_close(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  nRefSqlite3--;
  اگر( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  بازگشت TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ایستا صحیح SQLITE_TCLAPI btree_begin_transaction(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  اگر( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ایستا صحیح SQLITE_TCLAPI btree_pager_stats(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح i;
  صحیح *a;

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
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
  برای(i=0; i<11; i++){
    ایستا نویسه *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    نویسه zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(اندازه(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  بازگشت TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ایستا صحیح SQLITE_TCLAPI btree_cursor(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح iTable;
  BtCursor *pCur;
  صحیح rc = SQLITE_OK;
  صحیح wrFlag;

  اگر( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &iTable) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) بازگشت TCL_ERROR;
  اگر( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  memset(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  اگر( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  اگر( rc ){
    ckfree((نویسه *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), پوچ);
  بازگشت SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ایستا صحیح SQLITE_TCLAPI btree_close_cursor(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#اگر SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#وگرنه
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((نویسه *)pCur);
  اگر( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ایستا صحیح SQLITE_TCLAPI btree_next(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;
  صحیح res = 0;
  نویسه zBuf[100];

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  اگر( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  اگر( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  sqlite3_snprintf(اندازه(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ایستا صحیح SQLITE_TCLAPI btree_first(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;
  صحیح res = 0;
  نویسه zBuf[100];

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  اگر( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  sqlite3_snprintf(اندازه(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ایستا صحیح SQLITE_TCLAPI btree_eof(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;
  نویسه zBuf[50];

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(اندازه(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ایستا صحیح SQLITE_TCLAPI btree_payload_size(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  نویسه zBuf[50];

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(اندازه(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, پوچ);
  بازگشت SQLITE_OK;
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
ایستا صحیح SQLITE_TCLAPI btree_varint_test(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  u32 start, mult, شمار, incr;
  u64 in, out;
  صحیح n1, n2, i, j;
  unsigned نویسه zBuf[100];
  اگر( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", پوچ);
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[1], (صحیح*)&start) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetInt(interp, argv[2], (صحیح*)&mult) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetInt(interp, argv[3], (صحیح*)&شمار) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetInt(interp, argv[4], (صحیح*)&incr) ) بازگشت TCL_ERROR;
  in = start;
  in *= mult;
  برای(i=0; i<(صحیح)شمار; i++){
    نویسه zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    اگر( n1>9 || n1<1 ){
      sqlite3_snprintf(اندازه(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, پوچ);
      بازگشت TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    اگر( n1!=n2 ){
      sqlite3_snprintf(اندازه(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, پوچ);
      بازگشت TCL_ERROR;
    }
    اگر( in!=out ){
      sqlite3_snprintf(اندازه(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, پوچ);
      بازگشت TCL_ERROR;
    }
    اگر( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      اگر( n1!=n2 ){
        sqlite3_snprintf(اندازه(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, پوچ);
        بازگشت TCL_ERROR;
      }
      اگر( in!=out ){
        sqlite3_snprintf(اندازه(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, پوچ);
        بازگشت TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    برای(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI btree_from_db(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  صحیح iDb = 0;

  اگر( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", پوچ);
    بازگشت TCL_ERROR;
  }

  اگر( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", پوچ);
    بازگشت TCL_ERROR;
  }
  اگر( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  أكد( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (نویسه*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ایستا صحیح SQLITE_TCLAPI btree_ismemdb(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح res;
  sqlite3_file *pFile;

  اگر( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  بازگشت SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ایستا صحیح SQLITE_TCLAPI btree_set_cache_size(
  تهی *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت نویسه **argv      /* Text of each argument */
){
  صحیح nCache;
  Btree *pBt;
  
  اگر( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", پوچ);
    بازگشت TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &nCache) ) بازگشت TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  بازگشت TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ایستا صحیح SQLITE_TCLAPI btree_درج(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت objv[]
){
  BtCursor *pCur;
  صحیح rc;
  BtreePayload x;
  Tcl_Size n;

  اگر( objc!=4 && objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    بازگشت TCL_ERROR;
  }

  memset(&x, 0, اندازه(x));
  اگر( objc==4 ){
    اگر( Tcl_GetIntFromObj(interp, objv[2], &rc) ) بازگشت TCL_ERROR;
    x.n_کلید = rc;
    x.pData = (تهی*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (صحیح)n;
  }وگرنه{
    x.p_کلید = (تهی*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_کلید = (صحیح)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_درج(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  اگر( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), پوچ);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest3_Init(Tcl_Interp *interp){
  ایستا ساختار {
     نویسه *zName;
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
  صحیح i;

  برای(i=0; i<اندازه(aCmd)/اندازه(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_درج, 0, 0);

  بازگشت TCL_OK;
}
