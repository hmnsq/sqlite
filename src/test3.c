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

बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
स्थिर sqlite3 sDb;
स्थिर पूर्णाङ्क nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_open(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णाङ्क rc, nCache;
  पूर्णाङ्क n;
  वर्ण *zFilename;
  यदि( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }

  यदि( Tcl_GetInt(interp, argv[2], &nCache) ) प्रत्यागम TCL_ERROR;
  nRefSqlite3++;
  यदि( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (पूर्णाङ्क)वाक्यदैर्घ्यम्(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  यदि( zFilename==0 ) प्रत्यागम TCL_ERROR;
  स्मृतिं_प्रतिलिख(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  यदि( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  sqlite3_Btree_समुच्चयः_Cache_आकार(pBt, nCache);
  Tcl_Append_परिणाम(interp, sqlite3_Test_Ptr_गन्तव्यम्_Text(pBt), रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_close(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pBt = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  यदि( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  nRefSqlite3--;
  यदि( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  प्रत्यागम TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_begin_transaction(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णाङ्क rc;
  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pBt = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  यदि( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_pager_stats(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णाङ्क i;
  पूर्णाङ्क *a;

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pBt = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
 
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
  प्रति(i=0; i<11; i++){
    स्थिर वर्ण *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    वर्ण zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(परिमाण(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  प्रत्यागम TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_cursor(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णाङ्क i_सारणी;
  BtCursor *pCur;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क wrFlag;

  यदि( argc!=4 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pBt = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  यदि( Tcl_GetInt(interp, argv[2], &i_सारणी) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) प्रत्यागम TCL_ERROR;
  यदि( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_आकार());
  स्मृतिं_पूरय(pCur, 0, sqlite3_Btree_Cursor_आकार());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3_Btree_तालकम्_सारणी(pBt, i_सारणी, !!wrFlag);
#यदि_अन्तः
  यदि( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, i_सारणी, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  यदि( rc ){
    ckfree((वर्ण *)pCur);
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  Tcl_Append_परिणाम(interp, sqlite3_Test_Ptr_गन्तव्यम्_Text(pCur), रिक्त);
  प्रत्यागम SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_close_cursor(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णाङ्क rc;

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pCur = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
#यदि SQLITE_THREADSAFE>0
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
#यदि_अन्तः
  ckfree((वर्ण *)pCur);
  यदि( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_अग्र_2(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णाङ्क rc;
  पूर्णाङ्क res = 0;
  वर्ण zBuf[100];

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pCur = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_अग्र(pCur, 0);
  यदि( rc==SQLITE_समाप्तम् ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  यदि( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  sqlite3_snprintf(परिमाण(zBuf),zBuf,"%d",res);
  Tcl_Append_परिणाम(interp, zBuf, रिक्त);
  प्रत्यागम SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_first(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णाङ्क rc;
  पूर्णाङ्क res = 0;
  वर्ण zBuf[100];

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pCur = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  यदि( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  sqlite3_snprintf(परिमाण(zBuf),zBuf,"%d",res);
  Tcl_Append_परिणाम(interp, zBuf, रिक्त);
  प्रत्यागम SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_eof(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णाङ्क rc;
  वर्ण zBuf[50];

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pCur = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(परिमाण(zBuf),zBuf, "%d", rc);
  Tcl_Append_परिणाम(interp, zBuf, रिक्त);
  प्रत्यागम SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_payload_आकार(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  वर्ण zBuf[50];

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pCur = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_आकार(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(परिमाण(zBuf),zBuf, "%u", n);
  Tcl_Append_परिणाम(interp, zBuf, रिक्त);
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_varint_test(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  u32 start, mult, गणनम्, incr;
  u64 अन्तर्, बहिर्गमनम्;
  पूर्णाङ्क n1, n2, i, j;
  अचिह्नित वर्ण zBuf[100];
  यदि( argc!=5 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetInt(interp, argv[1], (पूर्णाङ्क*)&start) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetInt(interp, argv[2], (पूर्णाङ्क*)&mult) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetInt(interp, argv[3], (पूर्णाङ्क*)&गणनम्) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetInt(interp, argv[4], (पूर्णाङ्क*)&incr) ) प्रत्यागम TCL_ERROR;
  अन्तर् = start;
  अन्तर् *= mult;
  प्रति(i=0; i<(पूर्णाङ्क)गणनम्; i++){
    वर्ण zErr[200];
    n1 = sqlite3PutVarint(zBuf, अन्तर्);
    यदि( n1>9 || n1<1 ){
      sqlite3_snprintf(परिमाण(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_परिणाम(interp, zErr, रिक्त);
      प्रत्यागम TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &बहिर्गमनम्);
    यदि( n1!=n2 ){
      sqlite3_snprintf(परिमाण(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_परिणाम(interp, zErr, रिक्त);
      प्रत्यागम TCL_ERROR;
    }
    यदि( अन्तर्!=बहिर्गमनम् ){
      sqlite3_snprintf(परिमाण(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", अन्तर्, बहिर्गमनम्);
      Tcl_Append_परिणाम(interp, zErr, रिक्त);
      प्रत्यागम TCL_ERROR;
    }
    यदि( (अन्तर् & 0xffffffff)==अन्तर् ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      बहिर्गमनम् = out32;
      यदि( n1!=n2 ){
        sqlite3_snprintf(परिमाण(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_परिणाम(interp, zErr, रिक्त);
        प्रत्यागम TCL_ERROR;
      }
      यदि( अन्तर्!=बहिर्गमनम् ){
        sqlite3_snprintf(परिमाण(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            अन्तर्, बहिर्गमनम्);
        Tcl_Append_परिणाम(interp, zErr, रिक्त);
        प्रत्यागम TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    प्रति(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &बहिर्गमनम्);
    }
    अन्तर् += incr;
  }
  प्रत्यागम TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_from_db(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  पूर्णाङ्क iDb = 0;

  यदि( argc!=2 && argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }

  यदि( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_परिणाम(interp, "No such db-handle: \"", argv[1], "\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  यदि( argc==3 ){
    iDb = atoi(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_दत्त);
  प्रतिज्ञा( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण*)sqlite3_Test_Ptr_गन्तव्यम्_Text(pBt), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_ismemdb(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णाङ्क res;
  sqlite3_file *pFile;

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pBt = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewBooleanObj(res));
  प्रत्यागम SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_समुच्चयः_cache_आकार(
  शून्य *Not_प्रयुक्तम्,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क nCache;
  Btree *pBt;
  
  यदि( argc!=3 ){
    Tcl_Append_परिणाम(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  pBt = sqlite3_Test_Text_गन्तव्यम्_Ptr(argv[1]);
  यदि( Tcl_GetInt(interp, argv[2], &nCache) ) प्रत्यागम TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_समुच्चयः_Cache_आकार(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  प्रत्यागम TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI btree_निवेशय(
  Client_दत्त client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *ध्रुव objv[]
){
  BtCursor *pCur;
  पूर्णाङ्क rc;
  BtreePayload x;
  Tcl_आकार n;

  यदि( objc!=4 && objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    प्रत्यागम TCL_ERROR;
  }

  स्मृतिं_पूरय(&x, 0, परिमाण(x));
  यदि( objc==4 ){
    यदि( Tcl_GetIntFromObj(interp, objv[2], &rc) ) प्रत्यागम TCL_ERROR;
    x.n_कुञ्चिका = rc;
    x.p_दत्त = (शून्य*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_दत्त = (पूर्णाङ्क)n;
  }अन्यथा{
    x.p_कुञ्चिका = (शून्य*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_कुञ्चिका = (पूर्णाङ्क)n;
  }
  pCur = (BtCursor*)sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_निवेशय(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_परिणाम(interp);
  यदि( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest3_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "btree_open",               (Tcl_CmdProc*)btree_open               },
     { "btree_close",              (Tcl_CmdProc*)btree_close              },
     { "btree_begin_transaction",  (Tcl_CmdProc*)btree_begin_transaction  },
     { "btree_pager_stats",        (Tcl_CmdProc*)btree_pager_stats        },
     { "btree_cursor",             (Tcl_CmdProc*)btree_cursor             },
     { "btree_close_cursor",       (Tcl_CmdProc*)btree_close_cursor       },
     { "btree_next",               (Tcl_CmdProc*)btree_अग्र_2               },
     { "btree_eof",                (Tcl_CmdProc*)btree_eof                },
     { "btree_payload_size",       (Tcl_CmdProc*)btree_payload_आकार       },
     { "btree_first",              (Tcl_CmdProc*)btree_first              },
     { "btree_varint_test",        (Tcl_CmdProc*)btree_varint_test        },
     { "btree_from_db",            (Tcl_CmdProc*)btree_from_db            },
     { "btree_ismemdb",            (Tcl_CmdProc*)btree_ismemdb            },
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_समुच्चयः_cache_आकार     }
  };
  पूर्णाङ्क i;

  प्रति(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_निवेशय, 0, 0);

  प्रत्यागम TCL_OK;
}
