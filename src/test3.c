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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "btreeInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
સ્થિર sqlite3 sDb;
સ્થિર પૂર્ણાંક nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_open(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Btree *pBt;
  પૂર્ણાંક rc, nCache;
  પૂર્ણાંક n;
  અક્ષર *zFilename;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", ખાલી);
    પાછા TCL_ERROR;
  }

  જો( Tcl_GetInt(interp, argv[2], &nCache) ) પાછા TCL_ERROR;
  nRefSqlite3++;
  જો( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (પૂર્ણાંક)strlen(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  જો( zFilename==0 ) પાછા TCL_ERROR;
  memcpy(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  sqlite3BtreeSetCacheSize(pBt, nCache);
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pBt), ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_close(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Btree *pBt;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  nRefSqlite3--;
  જો( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  પાછા TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_begin_transaction(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Btree *pBt;
  પૂર્ણાંક rc;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_pager_stats(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Btree *pBt;
  પૂર્ણાંક i;
  પૂર્ણાંક *a;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
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
  માટે(i=0; i<11; i++){
    સ્થિર અક્ષર *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    અક્ષર zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(કદ(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  પાછા TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_cursor(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Btree *pBt;
  પૂર્ણાંક iTable;
  BtCursor *pCur;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક wrFlag;

  જો( argc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  જો( Tcl_GetInt(interp, argv[2], &iTable) ) પાછા TCL_ERROR;
  જો( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) પાછા TCL_ERROR;
  જો( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3BtreeCursorSize());
  memset(pCur, 0, sqlite3BtreeCursorSize());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#ifndef SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#endif
  જો( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  જો( rc ){
    ckfree((અક્ષર *)pCur);
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  Tcl_AppendResult(interp, sqlite3TestPtrToText(pCur), ખાલી);
  પાછા SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_close_cursor(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  BtCursor *pCur;
  પૂર્ણાંક rc;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#જો SQLITE_THREADSAFE>0
  {
    Btree *pBt = pCur->pBtree;
    sqlite3_mutex_enter(pBt->db->mutex);
    sqlite3BtreeEnter(pBt);
    rc = sqlite3BtreeCloseCursor(pCur);
    sqlite3BtreeLeave(pBt);
    sqlite3_mutex_leave(pBt->db->mutex);
  }
#નહીંતર
  rc = sqlite3BtreeCloseCursor(pCur);
#endif
  ckfree((અક્ષર *)pCur);
  જો( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_next(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  BtCursor *pCur;
  પૂર્ણાંક rc;
  પૂર્ણાંક res = 0;
  અક્ષર zBuf[100];

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeNext(pCur, 0);
  જો( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  જો( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  sqlite3_snprintf(કદ(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_first(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  BtCursor *pCur;
  પૂર્ણાંક rc;
  પૂર્ણાંક res = 0;
  અક્ષર zBuf[100];

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  જો( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  sqlite3_snprintf(કદ(zBuf),zBuf,"%d",res);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_eof(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  BtCursor *pCur;
  પૂર્ણાંક rc;
  અક્ષર zBuf[50];

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(કદ(zBuf),zBuf, "%d", rc);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_payload_size(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  અક્ષર zBuf[50];

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3BtreePayloadSize(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(કદ(zBuf),zBuf, "%u", n);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_varint_test(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  u32 start, mult, ગણતરી, incr;
  u64 in, out;
  પૂર્ણાંક n1, n2, i, j;
  unsigned અક્ષર zBuf[100];
  જો( argc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetInt(interp, argv[1], (પૂર્ણાંક*)&start) ) પાછા TCL_ERROR;
  જો( Tcl_GetInt(interp, argv[2], (પૂર્ણાંક*)&mult) ) પાછા TCL_ERROR;
  જો( Tcl_GetInt(interp, argv[3], (પૂર્ણાંક*)&ગણતરી) ) પાછા TCL_ERROR;
  જો( Tcl_GetInt(interp, argv[4], (પૂર્ણાંક*)&incr) ) પાછા TCL_ERROR;
  in = start;
  in *= mult;
  માટે(i=0; i<(પૂર્ણાંક)ગણતરી; i++){
    અક્ષર zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    જો( n1>9 || n1<1 ){
      sqlite3_snprintf(કદ(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_AppendResult(interp, zErr, ખાલી);
      પાછા TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    જો( n1!=n2 ){
      sqlite3_snprintf(કદ(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_AppendResult(interp, zErr, ખાલી);
      પાછા TCL_ERROR;
    }
    જો( in!=out ){
      sqlite3_snprintf(કદ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_AppendResult(interp, zErr, ખાલી);
      પાછા TCL_ERROR;
    }
    જો( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      જો( n1!=n2 ){
        sqlite3_snprintf(કદ(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_AppendResult(interp, zErr, ખાલી);
        પાછા TCL_ERROR;
      }
      જો( in!=out ){
        sqlite3_snprintf(કદ(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_AppendResult(interp, zErr, ખાલી);
        પાછા TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    માટે(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_from_db(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  પૂર્ણાંક iDb = 0;

  જો( argc!=2 && argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", ખાલી);
    પાછા TCL_ERROR;
  }

  જો( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_AppendResult(interp, "No such db-handle: \"", argv[1], "\"", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.objClientData);
  પ્રતિજ્ઞા( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_SetResult(interp, (અક્ષર*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_ismemdb(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  Btree *pBt;
  પૂર્ણાંક res;
  sqlite3_file *pFile;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
  પાછા SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_set_cache_size(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક nCache;
  Btree *pBt;
  
  જો( argc!=3 ){
    Tcl_AppendResult(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", ખાલી);
    પાછા TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  જો( Tcl_GetInt(interp, argv[2], &nCache) ) પાછા TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3BtreeSetCacheSize(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  પાછા TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI btree_ઉમેરો(
  ClientData clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *અચલ objv[]
){
  BtCursor *pCur;
  પૂર્ણાંક rc;
  BtreePayload x;
  Tcl_Size n;

  જો( objc!=4 && objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    પાછા TCL_ERROR;
  }

  memset(&x, 0, કદ(x));
  જો( objc==4 ){
    જો( Tcl_GetIntFromObj(interp, objv[2], &rc) ) પાછા TCL_ERROR;
    x.n_ચાવી = rc;
    x.pData = (શૂન્ય*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.nData = (પૂર્ણાંક)n;
  }નહીંતર{
    x.p_ચાવી = (શૂન્ય*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_ચાવી = (પૂર્ણાંક)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_ઉમેરો(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_ResetResult(interp);
  જો( rc ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), ખાલી);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest3_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
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
  પૂર્ણાંક i;

  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_ઉમેરો, 0, 0);

  પાછા TCL_OK;
}
