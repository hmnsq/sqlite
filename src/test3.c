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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "btreeInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
স্থির sqlite3 sDb;
স্থির পূর্ণসংখ্যা nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_open(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Btree *pBt;
  পূর্ণসংখ্যা rc, nCache;
  পূর্ণসংখ্যা n;
  অক্ষর *zFilename;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  যদি( Tcl_GetInt(interp, argv[2], &nCache) ) ফেরত TCL_ERROR;
  nRefSqlite3++;
  যদি( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (পূর্ণসংখ্যা)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  যদি( zFilename==0 ) ফেরত TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  sqlite3_Btree_Set_Cache_আকার(pBt, nCache);
  Tcl_Append_ফলাফল(interp, sqlite3TestPtrToText(pBt), ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_close(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Btree *pBt;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  nRefSqlite3--;
  যদি( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  ফেরত TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_begin_transaction(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Btree *pBt;
  পূর্ণসংখ্যা rc;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_pager_stats(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Btree *pBt;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা *a;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
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
  জন্য(i=0; i<11; i++){
    স্থির অক্ষর *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    অক্ষর zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(আকার(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  ফেরত TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_cursor(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Btree *pBt;
  পূর্ণসংখ্যা iTable;
  BtCursor *pCur;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা wrFlag;

  যদি( argc!=4 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  যদি( Tcl_GetInt(interp, argv[2], &iTable) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) ফেরত TCL_ERROR;
  যদি( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_আকার());
  memset(pCur, 0, sqlite3_Btree_Cursor_আকার());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  যদি( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  যদি( rc ){
    ckfree((অক্ষর *)pCur);
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  Tcl_Append_ফলাফল(interp, sqlite3TestPtrToText(pCur), ফাঁকা);
  ফেরত SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_close_cursor(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  BtCursor *pCur;
  পূর্ণসংখ্যা rc;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#যদি SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#অন্যথা
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((অক্ষর *)pCur);
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_পরবর্তী_2(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  BtCursor *pCur;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা res = 0;
  অক্ষর zBuf[100];

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_পরবর্তী(pCur, 0);
  যদি( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  sqlite3_snprintf(আকার(zBuf),zBuf,"%d",res);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_first(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  BtCursor *pCur;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা res = 0;
  অক্ষর zBuf[100];

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  sqlite3_snprintf(আকার(zBuf),zBuf,"%d",res);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_eof(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  BtCursor *pCur;
  পূর্ণসংখ্যা rc;
  অক্ষর zBuf[50];

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(আকার(zBuf),zBuf, "%d", rc);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_payload_আকার(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  অক্ষর zBuf[50];

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_আকার(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(আকার(zBuf),zBuf, "%u", n);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_varint_test(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  u32 start, mult, গুনতি, incr;
  u64 in, out;
  পূর্ণসংখ্যা n1, n2, i, j;
  unsigned অক্ষর zBuf[100];
  যদি( argc!=5 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetInt(interp, argv[1], (পূর্ণসংখ্যা*)&start) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetInt(interp, argv[2], (পূর্ণসংখ্যা*)&mult) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetInt(interp, argv[3], (পূর্ণসংখ্যা*)&গুনতি) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetInt(interp, argv[4], (পূর্ণসংখ্যা*)&incr) ) ফেরত TCL_ERROR;
  in = start;
  in *= mult;
  জন্য(i=0; i<(পূর্ণসংখ্যা)গুনতি; i++){
    অক্ষর zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    যদি( n1>9 || n1<1 ){
      sqlite3_snprintf(আকার(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_ফলাফল(interp, zErr, ফাঁকা);
      ফেরত TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    যদি( n1!=n2 ){
      sqlite3_snprintf(আকার(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_ফলাফল(interp, zErr, ফাঁকা);
      ফেরত TCL_ERROR;
    }
    যদি( in!=out ){
      sqlite3_snprintf(আকার(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_Append_ফলাফল(interp, zErr, ফাঁকা);
      ফেরত TCL_ERROR;
    }
    যদি( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      যদি( n1!=n2 ){
        sqlite3_snprintf(আকার(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_ফলাফল(interp, zErr, ফাঁকা);
        ফেরত TCL_ERROR;
      }
      যদি( in!=out ){
        sqlite3_snprintf(আকার(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_Append_ফলাফল(interp, zErr, ফাঁকা);
        ফেরত TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    জন্য(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_from_db(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  পূর্ণসংখ্যা iDb = 0;

  যদি( argc!=2 && argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  যদি( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_ফলাফল(interp, "No such db-handle: \"", argv[1], "\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_তথ্য);
  প্রতিজ্ঞা( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_Set_ফলাফল(interp, (অক্ষর*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  ফেরত TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_ismemdb(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  Btree *pBt;
  পূর্ণসংখ্যা res;
  sqlite3_file *pFile;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewBooleanObj(res));
  ফেরত SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_set_cache_আকার(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা nCache;
  Btree *pBt;
  
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  যদি( Tcl_GetInt(interp, argv[2], &nCache) ) ফেরত TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_Set_Cache_আকার(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  ফেরত TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI btree_ঢোকাও(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *ধ্রুবক objv[]
){
  BtCursor *pCur;
  পূর্ণসংখ্যা rc;
  BtreePayload x;
  Tcl_আকার n;

  যদি( objc!=4 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    ফেরত TCL_ERROR;
  }

  memset(&x, 0, আকার(x));
  যদি( objc==4 ){
    যদি( Tcl_GetIntFromObj(interp, objv[2], &rc) ) ফেরত TCL_ERROR;
    x.n_চাবি = rc;
    x.p_তথ্য = (শূন্য*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_তথ্য = (পূর্ণসংখ্যা)n;
  }অন্যথা{
    x.p_চাবি = (শূন্য*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_চাবি = (পূর্ণসংখ্যা)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ঢোকাও(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_ফলাফল(interp);
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest3_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_পরবর্তী_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_আকার       },
     { "btree_first",              (Tcl_CmdProc*)btree_first              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_set_cache_আকার     }
  };
  পূর্ণসংখ্যা i;

  জন্য(i=0; i<আকার(aCmd)/আকার(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_ঢোকাও, 0, 0);

  ফেরত TCL_OK;
}
