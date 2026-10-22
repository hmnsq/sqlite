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

خارجی ثابت حرف *sqlite3ErrName(صحیح);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ساکن sqlite3 sDb;
ساکن صحیح nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ساکن صحیح SQLITE_TCLAPI btree_open(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح rc, nCache;
  صحیح n;
  حرف *zFilename;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", تہی);
    واپس TCL_ERROR;
  }

  اگر( Tcl_GetInt(interp, argv[2], &nCache) ) واپس TCL_ERROR;
  nRefSqlite3++;
  اگر( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_ڈھونڈو(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (صحیح)لڑی_لمبائی(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  اگر( zFilename==0 ) واپس TCL_ERROR;
  حافظہ_نقل(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  sqlite3_Btree_سیٹ_Cache_سائز(pBt, nCache);
  Tcl_Append_نتیجہ(interp, sqlite3_Test_Ptr_مقصد_متن(pBt), تہی);
  واپس TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ساکن صحیح SQLITE_TCLAPI btree_close(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  nRefSqlite3--;
  اگر( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  واپس TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ساکن صحیح SQLITE_TCLAPI btree_begin_transaction(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح rc;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ساکن صحیح SQLITE_TCLAPI btree_pager_stats(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح i;
  صحیح *a;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
 
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
  برائے(i=0; i<11; i++){
    ساکن حرف *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    حرف zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(حجم(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  واپس TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ساکن صحیح SQLITE_TCLAPI btree_cursor(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح i_جدول;
  BtCursor *pCur;
  صحیح rc = SQLITE_OK;
  صحیح wrFlag;

  اگر( argc!=4 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", تہی);
    واپس TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &i_جدول) ) واپس TCL_ERROR;
  اگر( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) واپس TCL_ERROR;
  اگر( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_سائز());
  memset(pCur, 0, sqlite3_Btree_Cursor_سائز());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3_Btree_تالا_جدول(pBt, i_جدول, !!wrFlag);
#اگر_ختم
  اگر( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, i_جدول, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  اگر( rc ){
    ckfree((حرف *)pCur);
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  Tcl_Append_نتیجہ(interp, sqlite3_Test_Ptr_مقصد_متن(pCur), تہی);
  واپس SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ساکن صحیح SQLITE_TCLAPI btree_close_cursor(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
#اگر SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#ورنہ
  rc = sqlite3BtreeCloseCursor(pCur);
#اگر_ختم
  ckfree((حرف *)pCur);
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ساکن صحیح SQLITE_TCLAPI btree_اگلا_2(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;
  صحیح res = 0;
  حرف zBuf[100];

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_اگلا(pCur, 0);
  اگر( rc==SQLITE_مکمل ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  sqlite3_snprintf(حجم(zBuf),zBuf,"%d",res);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ساکن صحیح SQLITE_TCLAPI btree_پہلا(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;
  صحیح res = 0;
  حرف zBuf[100];

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_پہلا(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  sqlite3_snprintf(حجم(zBuf),zBuf,"%d",res);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ساکن صحیح SQLITE_TCLAPI btree_eof(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحیح rc;
  حرف zBuf[50];

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(حجم(zBuf),zBuf, "%d", rc);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ساکن صحیح SQLITE_TCLAPI btree_payload_سائز(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  حرف zBuf[50];

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_سائز(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(حجم(zBuf),zBuf, "%u", n);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس SQLITE_OK;
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
ساکن صحیح SQLITE_TCLAPI btree_varint_test(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  u32 آغاز, mult, گنتی, incr;
  u64 اندر, برآمد;
  صحیح n1, n2, i, j;
  غیرعلامتی حرف zBuf[100];
  اگر( argc!=5 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", تہی);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[1], (صحیح*)&آغاز) ) واپس TCL_ERROR;
  اگر( Tcl_GetInt(interp, argv[2], (صحیح*)&mult) ) واپس TCL_ERROR;
  اگر( Tcl_GetInt(interp, argv[3], (صحیح*)&گنتی) ) واپس TCL_ERROR;
  اگر( Tcl_GetInt(interp, argv[4], (صحیح*)&incr) ) واپس TCL_ERROR;
  اندر = آغاز;
  اندر *= mult;
  برائے(i=0; i<(صحیح)گنتی; i++){
    حرف zErr[200];
    n1 = sqlite3PutVarint(zBuf, اندر);
    اگر( n1>9 || n1<1 ){
      sqlite3_snprintf(حجم(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_نتیجہ(interp, zErr, تہی);
      واپس TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &برآمد);
    اگر( n1!=n2 ){
      sqlite3_snprintf(حجم(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_نتیجہ(interp, zErr, تہی);
      واپس TCL_ERROR;
    }
    اگر( اندر!=برآمد ){
      sqlite3_snprintf(حجم(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", اندر, برآمد);
      Tcl_Append_نتیجہ(interp, zErr, تہی);
      واپس TCL_ERROR;
    }
    اگر( (اندر & 0xffffffff)==اندر ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      برآمد = out32;
      اگر( n1!=n2 ){
        sqlite3_snprintf(حجم(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_نتیجہ(interp, zErr, تہی);
        واپس TCL_ERROR;
      }
      اگر( اندر!=برآمد ){
        sqlite3_snprintf(حجم(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            اندر, برآمد);
        Tcl_Append_نتیجہ(interp, zErr, تہی);
        واپس TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    برائے(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &برآمد);
    }
    اندر += incr;
  }
  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI btree_from_db(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  صحیح iDb = 0;

  اگر( argc!=2 && argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", تہی);
    واپس TCL_ERROR;
  }

  اگر( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_نتیجہ(interp, "No such db-handle: \"", argv[1], "\"", تہی);
    واپس TCL_ERROR;
  }
  اگر( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_ڈیٹا);
  أكد( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_سیٹ_نتیجہ(interp, (حرف*)sqlite3_Test_Ptr_مقصد_متن(pBt), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ساکن صحیح SQLITE_TCLAPI btree_ismemdb(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحیح res;
  sqlite3_file *pFile;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID\"", تہی);
    واپس TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewBooleanObj(res));
  واپس SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ساکن صحیح SQLITE_TCLAPI btree_سیٹ_cache_سائز(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح nCache;
  Btree *pBt;
  
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", تہی);
    واپس TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  اگر( Tcl_GetInt(interp, argv[2], &nCache) ) واپس TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_سیٹ_Cache_سائز(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  واپس TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ساکن صحیح SQLITE_TCLAPI btree_داخل_کرو(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت objv[]
){
  BtCursor *pCur;
  صحیح rc;
  BtreePayload x;
  Tcl_سائز n;

  اگر( objc!=4 && objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    واپس TCL_ERROR;
  }

  memset(&x, 0, حجم(x));
  اگر( objc==4 ){
    اگر( Tcl_GetIntFromObj(interp, objv[2], &rc) ) واپس TCL_ERROR;
    x.n_کلید = rc;
    x.p_ڈیٹا = (خالی*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_ڈیٹا = (صحیح)n;
  }ورنہ{
    x.p_کلید = (خالی*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_کلید = (صحیح)n;
  }
  pCur = (BtCursor*)sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_داخل_کرو(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_نتیجہ(interp);
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), تہی);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest3_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_اگلا_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_سائز       },
     { "btree_first",              (Tcl_CmdProc*)btree_پہلا              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_سیٹ_cache_سائز     }
  };
  صحیح i;

  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_داخل_کرو, 0, 0);

  واپس TCL_OK;
}
