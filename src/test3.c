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
#ضمن "sqliteInt.h"
#ضمن "btreeInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>

خارجي ثابت حرف *sqlite3ErrName(صحيح);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
ساكن sqlite3 sDb;
ساكن صحيح nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
ساكن صحيح SQLITE_TCLAPI btree_open(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحيح rc, nCache;
  صحيح n;
  حرف *zFilename;
  إذا( argc!=3 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", عدم);
    أرجع TCL_ERROR;
  }

  إذا( Tcl_GetInt(interp, argv[2], &nCache) ) أرجع TCL_ERROR;
  nRefSqlite3++;
  إذا( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_پیدا_کن(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (صحيح)طول_رشته(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  إذا( zFilename==0 ) أرجع TCL_ERROR;
  کپی_حافظه(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  sqlite3_Btree_مجموعه_Cache_اندازه(pBt, nCache);
  Tcl_Append_نتیجه(interp, sqlite3_Test_Ptr_مقصد_متن(pBt), عدم);
  أرجع TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
ساكن صحيح SQLITE_TCLAPI btree_close(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  nRefSqlite3--;
  إذا( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  أرجع TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
ساكن صحيح SQLITE_TCLAPI btree_begin_transaction(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحيح rc;
  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  إذا( rc!=SQLITE_OK ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
ساكن صحيح SQLITE_TCLAPI btree_pager_stats(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحيح i;
  صحيح *a;

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
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
  لكل(i=0; i<11; i++){
    ساكن حرف *zName[] = {
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
  أرجع TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
ساكن صحيح SQLITE_TCLAPI btree_cursor(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحيح i_جدول;
  BtCursor *pCur;
  صحيح rc = SQLITE_OK;
  صحيح wrFlag;

  إذا( argc!=4 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", عدم);
    أرجع TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  إذا( Tcl_GetInt(interp, argv[2], &i_جدول) ) أرجع TCL_ERROR;
  إذا( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) أرجع TCL_ERROR;
  إذا( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_اندازه());
  memset(pCur, 0, sqlite3_Btree_Cursor_اندازه());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#اگر_تعریف_نشده SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3_Btree_قفل_جدول(pBt, i_جدول, !!wrFlag);
#پایان_اگر
  إذا( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, i_جدول, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  إذا( rc ){
    ckfree((حرف *)pCur);
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  Tcl_Append_نتیجه(interp, sqlite3_Test_Ptr_مقصد_متن(pCur), عدم);
  أرجع SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
ساكن صحيح SQLITE_TCLAPI btree_close_cursor(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحيح rc;

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
#إذا SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#وإلا
  rc = sqlite3BtreeCloseCursor(pCur);
#پایان_اگر
  ckfree((حرف *)pCur);
  إذا( rc ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
ساكن صحيح SQLITE_TCLAPI btree_بعدی_2(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحيح rc;
  صحيح res = 0;
  حرف zBuf[100];

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_بعدی(pCur, 0);
  إذا( rc==SQLITE_انجامشده ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  إذا( rc ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  sqlite3_snprintf(حجم(zBuf),zBuf,"%d",res);
  Tcl_Append_نتیجه(interp, zBuf, عدم);
  أرجع SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
ساكن صحيح SQLITE_TCLAPI btree_اولی(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحيح rc;
  صحيح res = 0;
  حرف zBuf[100];

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_اولی(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  إذا( rc ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  sqlite3_snprintf(حجم(zBuf),zBuf,"%d",res);
  Tcl_Append_نتیجه(interp, zBuf, عدم);
  أرجع SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
ساكن صحيح SQLITE_TCLAPI btree_eof(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  صحيح rc;
  حرف zBuf[50];

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(حجم(zBuf),zBuf, "%d", rc);
  Tcl_Append_نتیجه(interp, zBuf, عدم);
  أرجع SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
ساكن صحيح SQLITE_TCLAPI btree_payload_اندازه(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  حرف zBuf[50];

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pCur = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_اندازه(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(حجم(zBuf),zBuf, "%u", n);
  Tcl_Append_نتیجه(interp, zBuf, عدم);
  أرجع SQLITE_OK;
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
ساكن صحيح SQLITE_TCLAPI btree_varint_test(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  u32 شروع, mult, شمار, incr;
  u64 درون, بیرون;
  صحيح n1, n2, i, j;
  بیعلامت حرف zBuf[100];
  إذا( argc!=5 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", عدم);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetInt(interp, argv[1], (صحيح*)&شروع) ) أرجع TCL_ERROR;
  إذا( Tcl_GetInt(interp, argv[2], (صحيح*)&mult) ) أرجع TCL_ERROR;
  إذا( Tcl_GetInt(interp, argv[3], (صحيح*)&شمار) ) أرجع TCL_ERROR;
  إذا( Tcl_GetInt(interp, argv[4], (صحيح*)&incr) ) أرجع TCL_ERROR;
  درون = شروع;
  درون *= mult;
  لكل(i=0; i<(صحيح)شمار; i++){
    حرف zErr[200];
    n1 = sqlite3PutVarint(zBuf, درون);
    إذا( n1>9 || n1<1 ){
      sqlite3_snprintf(حجم(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_نتیجه(interp, zErr, عدم);
      أرجع TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &بیرون);
    إذا( n1!=n2 ){
      sqlite3_snprintf(حجم(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_نتیجه(interp, zErr, عدم);
      أرجع TCL_ERROR;
    }
    إذا( درون!=بیرون ){
      sqlite3_snprintf(حجم(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", درون, بیرون);
      Tcl_Append_نتیجه(interp, zErr, عدم);
      أرجع TCL_ERROR;
    }
    إذا( (درون & 0xffffffff)==درون ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      بیرون = out32;
      إذا( n1!=n2 ){
        sqlite3_snprintf(حجم(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_نتیجه(interp, zErr, عدم);
        أرجع TCL_ERROR;
      }
      إذا( درون!=بیرون ){
        sqlite3_snprintf(حجم(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            درون, بیرون);
        Tcl_Append_نتیجه(interp, zErr, عدم);
        أرجع TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    لكل(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &بیرون);
    }
    درون += incr;
  }
  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI btree_from_db(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  صحيح iDb = 0;

  إذا( argc!=2 && argc!=3 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", عدم);
    أرجع TCL_ERROR;
  }

  إذا( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_نتیجه(interp, "No such db-handle: \"", argv[1], "\"", عدم);
    أرجع TCL_ERROR;
  }
  إذا( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_داده);
  أكد( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_مجموعه_نتیجه(interp, (حرف*)sqlite3_Test_Ptr_مقصد_متن(pBt), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
ساكن صحيح SQLITE_TCLAPI btree_ismemdb(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  Btree *pBt;
  صحيح res;
  sqlite3_file *pFile;

  إذا( argc!=2 ){
    Tcl_Append_نتیجه(interp, "wrong # args: should be \"", argv[0],
       " ID\"", عدم);
    أرجع TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewBooleanObj(res));
  أرجع SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
ساكن صحيح SQLITE_TCLAPI btree_مجموعه_cache_اندازه(
  فراغ *Not_استفاده_شده,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح nCache;
  Btree *pBt;
  
  إذا( argc!=3 ){
    Tcl_Append_نتیجه(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", عدم);
    أرجع TCL_ERROR;
  }
  pBt = sqlite3_Test_متن_مقصد_Ptr(argv[1]);
  إذا( Tcl_GetInt(interp, argv[2], &nCache) ) أرجع TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_مجموعه_Cache_اندازه(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  أرجع TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
ساكن صحيح SQLITE_TCLAPI btree_درج(
  Client_داده client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *ثابت objv[]
){
  BtCursor *pCur;
  صحيح rc;
  BtreePayload x;
  Tcl_اندازه n;

  إذا( objc!=4 && objc!=3 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    أرجع TCL_ERROR;
  }

  memset(&x, 0, حجم(x));
  إذا( objc==4 ){
    إذا( Tcl_GetIntFromObj(interp, objv[2], &rc) ) أرجع TCL_ERROR;
    x.n_کلید = rc;
    x.p_داده = (فراغ*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_داده = (صحيح)n;
  }وإلا{
    x.p_کلید = (فراغ*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_کلید = (صحيح)n;
  }
  pCur = (BtCursor*)sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_درج(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_نتیجه(interp);
  إذا( rc ){
    Tcl_Append_نتیجه(interp, sqlite3ErrName(rc), عدم);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest3_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_بعدی_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_اندازه       },
     { "btree_first",              (Tcl_CmdProc*)btree_اولی              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_مجموعه_cache_اندازه     }
  };
  صحيح i;

  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_درج, 0, 0);

  أرجع TCL_OK;
}
