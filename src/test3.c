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
#समावेश "sqliteInt.h"
#समावेश "btreeInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>

बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/*
** A bogus sqlite3 connection structure for use in the btree
** tests.
*/
अटल sqlite3 sDb;
अटल पूर्णांक nRefSqlite3 = 0;

/*
** Usage:   btree_open FILENAME NCACHE
**
** Open a new database
*/
अटल पूर्णांक SQLITE_TCLAPI btree_open(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक rc, nCache;
  पूर्णांक n;
  अक्षर *zFilename;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " FILENAME NCACHE FLAGS\"", शून्य);
    वापस TCL_ERROR;
  }

  अगर( Tcl_GetInt(interp, argv[2], &nCache) ) वापस TCL_ERROR;
  nRefSqlite3++;
  अगर( nRefSqlite3==1 ){
    sDb.pVfs = sqlite3_vfs_find(0);
    sDb.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    sqlite3_mutex_enter(sDb.mutex);
  }
  n = (पूर्णांक)म_माप(argv[1]);
  zFilename = sqlite3_malloc( n+2 );
  अगर( zFilename==0 ) वापस TCL_ERROR;
  स_नकल(zFilename, argv[1], n+1);
  zFilename[n+1] = 0;
  rc = sqlite3BtreeOpen(sDb.pVfs, zFilename, &sDb, &pBt, 0, 
     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB);
  sqlite3_free(zFilename);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  sqlite3_Btree_Set_Cache_आकार(pBt, nCache);
  Tcl_Append_परिणाम(interp, sqlite3TestPtrToText(pBt), शून्य);
  वापस TCL_OK;
}

/*
** Usage:   btree_close ID
**
** Close the given database.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_close(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  rc = sqlite3BtreeClose(pBt);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  nRefSqlite3--;
  अगर( nRefSqlite3==0 ){
    sqlite3_mutex_leave(sDb.mutex);
    sqlite3_mutex_free(sDb.mutex);
    sDb.mutex = 0;
    sDb.pVfs = 0;
  }
  वापस TCL_OK;
}


/*
** Usage:   btree_begin_transaction ID
**
** Start a new transaction
*/
अटल पूर्णांक SQLITE_TCLAPI btree_begin_transaction(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक rc;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pBt);
  rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  sqlite3BtreeLeave(pBt);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

/*
** Usage:   btree_pager_stats ID
**
** Returns pager statistics
*/
अटल पूर्णांक SQLITE_TCLAPI btree_pager_stats(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक i;
  पूर्णांक *a;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
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
  क्रम(i=0; i<11; i++){
    अटल अक्षर *zName[] = {
      "ref", "page", "max", "size", "state", "err",
      "hit", "miss", "ovfl", "read", "write"
    };
    अक्षर zBuf[100];
    Tcl_AppendElement(interp, zName[i]);
    sqlite3_snprintf(माप(zBuf), zBuf,"%d",a[i]);
    Tcl_AppendElement(interp, zBuf);
  }
  sqlite3BtreeLeave(pBt);

  /* Release the mutex on the SQLite handle that controls this b-tree */
  sqlite3_mutex_leave(pBt->db->mutex);
  वापस TCL_OK;
}

/*
** Usage:   btree_cursor ID TABLENUM WRITEABLE
**
** Create a new cursor.  Return the ID for the cursor.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_cursor(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक iTable;
  BtCursor *pCur;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक wrFlag;

  अगर( argc!=4 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID TABLENUM WRITEABLE\"", शून्य);
    वापस TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  अगर( Tcl_GetInt(interp, argv[2], &iTable) ) वापस TCL_ERROR;
  अगर( Tcl_GetBoolean(interp, argv[3], &wrFlag) ) वापस TCL_ERROR;
  अगर( wrFlag ) wrFlag = BTREE_WRCSR;
  pCur = (BtCursor *)ckalloc(sqlite3_Btree_Cursor_आकार());
  स_रखो(pCur, 0, sqlite3_Btree_Cursor_आकार());
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
  rc = sqlite3BtreeLockTable(pBt, iTable, !!wrFlag);
#पूर्ण_अगर
  अगर( rc==SQLITE_OK ){
    rc = sqlite3BtreeCursor(pBt, iTable, wrFlag, 0, pCur);
  }
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  अगर( rc ){
    ckfree((अक्षर *)pCur);
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  Tcl_Append_परिणाम(interp, sqlite3TestPtrToText(pCur), शून्य);
  वापस SQLITE_OK;
}

/*
** Usage:   btree_close_cursor ID
**
** Close a cursor opened using btree_cursor.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_close_cursor(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
#अगर SQLITE_THREADSAFE>0
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
#पूर्ण_अगर
  ckfree((अक्षर *)pCur);
  अगर( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस SQLITE_OK;
}

/*
** Usage:   btree_next ID
**
** Move the cursor to the next entry in the table.  Return 0 on success
** or 1 if the cursor was already on the last entry in the table or if
** the table is empty.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_अग्र_2(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;
  पूर्णांक res = 0;
  अक्षर zBuf[100];

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_अग्र(pCur, 0);
  अगर( rc==SQLITE_DONE ){
    res = 1;
    rc = SQLITE_OK;
  }
  sqlite3BtreeLeave(pCur->pBtree);
  अगर( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  sqlite3_snprintf(माप(zBuf),zBuf,"%d",res);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस SQLITE_OK;
}

/*
** Usage:   btree_first ID
**
** Move the cursor to the first entry in the table.  Return 0 if the
** cursor was left point to something and 1 if the table is empty.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_first(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;
  पूर्णांक res = 0;
  अक्षर zBuf[100];

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeFirst(pCur, &res);
  sqlite3BtreeLeave(pCur->pBtree);
  अगर( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  sqlite3_snprintf(माप(zBuf),zBuf,"%d",res);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस SQLITE_OK;
}

/*
** Usage:   btree_eof ID
**
** Return TRUE if the given cursor is not pointing at a valid entry.
** Return FALSE if the cursor does point to a valid entry.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_eof(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  पूर्णांक rc;
  अक्षर zBuf[50];

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3BtreeEof(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(माप(zBuf),zBuf, "%d", rc);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस SQLITE_OK;
}

/*
** Usage:   btree_payload_size ID
**
** Return the number of bytes of payload
*/
अटल पूर्णांक SQLITE_TCLAPI btree_payload_आकार(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  BtCursor *pCur;
  u32 n;
  अक्षर zBuf[50];

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pCur = sqlite3TestTextToPtr(argv[1]);
  sqlite3BtreeEnter(pCur->pBtree);
  n = sqlite3_Btree_Payload_आकार(pCur);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_snprintf(माप(zBuf),zBuf, "%u", n);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस SQLITE_OK;
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
अटल पूर्णांक SQLITE_TCLAPI btree_varint_test(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  u32 start, mult, गिनती, incr;
  u64 in, out;
  पूर्णांक n1, n2, i, j;
  अचिन्हित अक्षर zBuf[100];
  अगर( argc!=5 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " START MULTIPLIER COUNT INCREMENT\"", शून्य);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetInt(interp, argv[1], (पूर्णांक*)&start) ) वापस TCL_ERROR;
  अगर( Tcl_GetInt(interp, argv[2], (पूर्णांक*)&mult) ) वापस TCL_ERROR;
  अगर( Tcl_GetInt(interp, argv[3], (पूर्णांक*)&गिनती) ) वापस TCL_ERROR;
  अगर( Tcl_GetInt(interp, argv[4], (पूर्णांक*)&incr) ) वापस TCL_ERROR;
  in = start;
  in *= mult;
  क्रम(i=0; i<(पूर्णांक)गिनती; i++){
    अक्षर zErr[200];
    n1 = sqlite3PutVarint(zBuf, in);
    अगर( n1>9 || n1<1 ){
      sqlite3_snprintf(माप(zErr), zErr,
         "putVarint returned %d - should be between 1 and 9", n1);
      Tcl_Append_परिणाम(interp, zErr, शून्य);
      वापस TCL_ERROR;
    }
    n2 = sqlite3GetVarint(zBuf, &out);
    अगर( n1!=n2 ){
      sqlite3_snprintf(माप(zErr), zErr,
          "putVarint returned %d and getVarint returned %d", n1, n2);
      Tcl_Append_परिणाम(interp, zErr, शून्य);
      वापस TCL_ERROR;
    }
    अगर( in!=out ){
      sqlite3_snprintf(माप(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx", in, out);
      Tcl_Append_परिणाम(interp, zErr, शून्य);
      वापस TCL_ERROR;
    }
    अगर( (in & 0xffffffff)==in ){
      u32 out32;
      n2 = getVarint32(zBuf, out32);
      out = out32;
      अगर( n1!=n2 ){
        sqlite3_snprintf(माप(zErr), zErr,
          "putVarint returned %d and GetVarint32 returned %d", 
                  n1, n2);
        Tcl_Append_परिणाम(interp, zErr, शून्य);
        वापस TCL_ERROR;
      }
      अगर( in!=out ){
        sqlite3_snprintf(माप(zErr), zErr,
          "Wrote 0x%016llx and got back 0x%016llx from GetVarint32",
            in, out);
        Tcl_Append_परिणाम(interp, zErr, शून्य);
        वापस TCL_ERROR;
      }
    }

    /* In order to get realistic timings, run getVarint 19 more times.
    ** This is because getVarint is called about 20 times more often
    ** than putVarint.
    */
    क्रम(j=0; j<19; j++){
      sqlite3GetVarint(zBuf, &out);
    }
    in += incr;
  }
  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI btree_from_db(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Tcl_CmdInfo info;
  sqlite3 *db;
  Btree *pBt;
  पूर्णांक iDb = 0;

  अगर( argc!=2 && argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " DB-HANDLE ?N?\"", शून्य);
    वापस TCL_ERROR;
  }

  अगर( 1!=Tcl_GetCommandInfo(interp, argv[1], &info) ){
    Tcl_Append_परिणाम(interp, "No such db-handle: \"", argv[1], "\"", शून्य);
    वापस TCL_ERROR;
  }
  अगर( argc==3 ){
    iDb = म_से_प(argv[2]);
  }

  db = *((sqlite3 **)info.obj_Client_तथ्य);
  निश्चित( db );

  pBt = db->aDb[iDb].pBt;
  Tcl_Set_परिणाम(interp, (अक्षर*)sqlite3TestPtrToText(pBt), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** Usage:   btree_ismemdb ID
**
** Return true if the B-Tree is currently stored entirely in memory.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_ismemdb(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  Btree *pBt;
  पूर्णांक res;
  sqlite3_file *pFile;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID\"", शून्य);
    वापस TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  pFile = sqlite3PagerFile(sqlite3BtreePager(pBt));
  res = (pFile->pMethods==0);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewBooleanObj(res));
  वापस SQLITE_OK;
}

/*
** usage:   btree_set_cache_size ID NCACHE
**
** Set the size of the cache used by btree $ID.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_set_cache_आकार(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक nCache;
  Btree *pBt;
  
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(
        interp, "wrong # args: should be \"", argv[0], " BT NCACHE\"", शून्य);
    वापस TCL_ERROR;
  }
  pBt = sqlite3TestTextToPtr(argv[1]);
  अगर( Tcl_GetInt(interp, argv[2], &nCache) ) वापस TCL_ERROR;

  sqlite3_mutex_enter(pBt->db->mutex);
  sqlite3BtreeEnter(pBt);
  sqlite3_Btree_Set_Cache_आकार(pBt, nCache);
  sqlite3BtreeLeave(pBt);
  sqlite3_mutex_leave(pBt->db->mutex);
  वापस TCL_OK;
}      

/*
** usage:   btree_insert CSR ?KEY? VALUE
**
** Set the size of the cache used by btree $ID.
*/
अटल पूर्णांक SQLITE_TCLAPI btree_जोड़ो(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *स्थिर objv[]
){
  BtCursor *pCur;
  पूर्णांक rc;
  BtreePayload x;
  Tcl_आकार n;

  अगर( objc!=4 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?-intkey? CSR KEY VALUE");
    वापस TCL_ERROR;
  }

  स_रखो(&x, 0, माप(x));
  अगर( objc==4 ){
    अगर( Tcl_GetIntFromObj(interp, objv[2], &rc) ) वापस TCL_ERROR;
    x.n_कुंजी = rc;
    x.p_तथ्य = (व्योम*)Tcl_GetByteArrayFromObj(objv[3], &n);
    x.n_तथ्य = (पूर्णांक)n;
  }अन्यथा{
    x.p_कुंजी = (व्योम*)Tcl_GetByteArrayFromObj(objv[2], &n);
    x.n_कुंजी = (पूर्णांक)n;
  }
  pCur = (BtCursor*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));

  sqlite3_mutex_enter(pCur->pBtree->db->mutex);
  sqlite3BtreeEnter(pCur->pBtree);
  rc = sqlite3_Btree_जोड़ो(pCur, &x, 0, 0);
  sqlite3BtreeLeave(pCur->pBtree);
  sqlite3_mutex_leave(pCur->pBtree->db->mutex);

  Tcl_Reset_परिणाम(interp);
  अगर( rc ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), शून्य);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest3_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
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
     { "btree_set_cache_size",     (Tcl_CmdProc*)btree_set_cache_आकार     }
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_CreateObjCommand(interp, "btree_insert", btree_जोड़ो, 0, 0);

  वापस TCL_OK;
}
