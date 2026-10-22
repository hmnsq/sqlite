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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "btreeInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
स्थिर sqlite3 sDb;
स्थिर पूर्णांक nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_open(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक rc, nCache;
  पूर्णांक n;
  अक्षर *zFilename;
  जर( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", रिक्त);
    परत TCL_ERROR;
  }

  जर( Tcl_GetInt(interp, argv[2], &nCache) ) परत TCL_ERROR;
  nRefSqlite3++;
  जर( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (पूर्णांक)म_माप(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  जर( zFilename==0 ) परत TCL_ERROR;
  स_नकल(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  जर( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), रिक्त);
  परत TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_close(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक rc;
  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  जर( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  nRefSqlite3--;
  जर( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  परत TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_begin_transaction(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक rc;
  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  जर( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  परत TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_pager_stats(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक i;
  पूर्णांक *a;

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
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
  साठी(i=0; i<11; i++){
    स्थिर अक्षर *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    अक्षर zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(आकार(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  परत TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_cursor(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक iTable;
  BtCursor *pCur;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक wrFlag;

  जर( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", रिक्त);
    परत TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  जर( Tcl_GetInt(interp, argv[2], &iTable) ) परत TCL_ERROR;
  जर( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) परत TCL_ERROR;
  जर( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  स_रखो(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  जर( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  जर( rc ){
    ckfree((अक्षर *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), रिक्त);
  परत SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_close_cursor(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#जर SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#अन्यथा
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((अक्षर *)pCur);
  जर( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  परत SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_next(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;
  पूर्णांक res = 0;
  अक्षर zBuf[100];

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  जर( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  जर( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  sqlite3_snprintf(आकार(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  परत SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_first(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;
  पूर्णांक res = 0;
  अक्षर zBuf[100];

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  जर( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  sqlite3_snprintf(आकार(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  परत SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_eof(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;
  अक्षर zBuf[50];

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(आकार(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  परत SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_payload_size(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  अक्षर zBuf[50];

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(आकार(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  परत SQLITE_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI btree_varint_test(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  u32 start, mult, मोजणी, incr;
  u64 in, out;
  पूर्णांक n1, n2, i, j;
  अचिन्हित अक्षर zBuf[100];
  जर( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", रिक्त);
    परत TCL_ERROR;
  }
  जर( Tcl_GetInt(interp, argv[1], (पूर्णांक*)&start) ) परत TCL_ERROR;
  जर( Tcl_GetInt(interp, argv[2], (पूर्णांक*)&mult) ) परत TCL_ERROR;
  जर( Tcl_GetInt(interp, argv[3], (पूर्णांक*)&मोजणी) ) परत TCL_ERROR;
  जर( Tcl_GetInt(interp, argv[4], (पूर्णांक*)&incr) ) परत TCL_ERROR;
  in = start;
  in *= mult;
  साठी(i=0; i<(पूर्णांक)मोजणी; i++){
    अक्षर zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    जर( n1>9 || n1<1 ){
      sqlite3_snprintf(आकार(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, रिक्त);
      परत TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    जर( n1!=n2 ){
      sqlite3_snprintf(आकार(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, रिक्त);
      परत TCL_ERROR;
    }
    जर( in!=out ){
      sqlite3_snprintf(आकार(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, रिक्त);
      परत TCL_ERROR;
    }
    जर( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      जर( n1!=n2 ){
        sqlite3_snprintf(आकार(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, रिक्त);
        परत TCL_ERROR;
      }
      जर( in!=out ){
        sqlite3_snprintf(आकार(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, रिक्त);
        परत TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    साठी(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI btree_from_db(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  पूर्णांक iDb = 0;

  जर( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", रिक्त);
    परत TCL_ERROR;
  }

  जर( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", रिक्त);
    परत TCL_ERROR;
  }
  जर( argc==3 ){
    iDb = म_से_प(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  प्रतिज्ञा( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (अक्षर*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_ismemdb(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक res;
  sqlite3_file *pFile;

  जर( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    परत TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  परत SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_set_cache_size(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिरांक अक्षर **argv      /* Text of each argument */
){
  पूर्णांक nCache;
  Btree *pBt;
  
  जर( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", रिक्त);
    परत TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  जर( Tcl_GetInt(interp, argv[2], &nCache) ) परत TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  परत TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
स्थिर पूर्णांक SQLITE_TCLAPI btree_घाला(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *स्थिरांक objv[]
){
  BtCursor *pCur;
  पूर्णांक rc;
  BtreePayload x;
  Tcl_Size n;

  जर( objc!=4 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    परत TCL_ERROR;
  }

  स_रखो(&x, 0, आकार(x));
  जर( objc==4 ){
    जर( Tcl_GetIntFromObj(interp, objv[2], &rc) ) परत TCL_ERROR;
    x.n_किल्ली = rc;
    x.pData = (शून्य*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (पूर्णांक)n;
  }अन्यथा{
    x.p_किल्ली = (शून्य*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_किल्ली = (पूर्णांक)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_घाला(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  जर( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), रिक्त);
    परत TCL_ERROR;
  }
  परत TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest3_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     अक्षर *zName;
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
  पूर्णांक i;

  साठी(i=0; i<आकार(aCmd)/आकार(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_घाला, 0, 0);

  परत TCL_OK;
}
