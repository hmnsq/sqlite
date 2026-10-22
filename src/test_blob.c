/*
** 2014 October 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
*/
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
বাহ্য ধ্রুব বর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

/* From test1.c: */
বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুব বর্ণ *zA, sqlite3 **ppDb);
বাহ্য শূন্য *sqlite3TestTextToPtr(ধ্রুব বর্ণ *z);

/*
** Attempt to extract a blob handle (type sqlite3_blob*) from the Tcl
** object passed as the second argument. If successful, set *ppBlob to
** point to the blob handle and return TCL_OK. Otherwise, store an error
** message in the tcl interpreter and return TCL_ERROR. The final value
** of *ppBlob is undefined in this case.
**
** If the object contains a string that begins with "incrblob_", then it
** is assumed to be the name of a Tcl channel opened using the [db incrblob] 
** command (see tclsqlite.c). Otherwise, it is assumed to be a pointer 
** encoded using the ptrToText() routine or similar.
*/
স্থির পূর্ণাঙ্ক blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  বর্ণ *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  যদি( n==0 ){
    *ppBlob = 0;
  }অন্যথা যদি( n>9 && 0==memcmp("incrblob_", z, 9) ){
    পূর্ণাঙ্ক notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    যদি( !channel ) প্রত্যাগম TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }অন্যথা{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  প্রত্যাগম TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
স্থির বর্ণ *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  বর্ণ *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  প্রত্যাগম (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  পূর্ণাঙ্ক objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ধ্রুব বর্ণ *zDb;
  ধ্রুব বর্ণ *zTable;
  ধ্রুব বর্ণ *zColumn;
  Tcl_WideInt iRowid;
  পূর্ণাঙ্ক flags;
  ধ্রুব বর্ণ *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  পূর্ণাঙ্ক rc;

  যদি( objc!=8 ){
    ধ্রুব বর্ণ *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, zUsage);
    প্রত্যাগম TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) প্রত্যাগম TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  যদি( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) প্রত্যাগম TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[6], &flags) ) প্রত্যাগম TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  যদি( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }অন্যথা{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  যদি( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }অন্যথা{
    Tcl_SetResult(interp, (বর্ণ*)sqlite3ErrName(rc), TCL_VOLATILE);
    প্রত্যাগম TCL_ERROR;
  }
  প্রত্যাগম TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণাঙ্ক rc;
  
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) প্রত্যাগম TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  যদি( rc ){
    Tcl_SetResult(interp, (বর্ণ*)sqlite3ErrName(rc), TCL_VOLATILE);
  }অন্যথা{
    Tcl_ResetResult(interp);
  }
  প্রত্যাগম TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণাঙ্ক nByte;
  
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) প্রত্যাগম TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  প্রত্যাগম TCL_OK;
}

/*
** sqlite3_blob_read  CHANNEL OFFSET N
**
**   This command is used to test the sqlite3_blob_read() in ways that
**   the Tcl channel interface does not. The first argument should
**   be the name of a valid channel created by the [incrblob] method
**   of a database handle. This function calls sqlite3_blob_read()
**   to read N bytes from offset OFFSET from the underlying SQLite
**   blob handle.
**
**   On success, a byte-array object containing the read data is 
**   returned. On failure, the interpreter result is set to the
**   text representation of the returned error code (i.e. "SQLITE_NOMEM")
**   and a Tcl exception is thrown.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণাঙ্ক nByte;
  পূর্ণাঙ্ক iOffset;
  unsigned বর্ণ *zBuf = 0;
  পূর্ণাঙ্ক rc;
  
  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "CHANNEL OFFSET N");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) প্রত্যাগম TCL_ERROR;
  যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    প্রত্যাগম TCL_ERROR;
  }

  যদি( nByte>0 ){
    zBuf = (unsigned বর্ণ *)Tcl_AttemptAlloc(nByte);
    যদি( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, রিক্ত);
      প্রত্যাগম TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  যদি( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }অন্যথা{
    Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((বর্ণ *)zBuf);

  প্রত্যাগম (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}

/*
** sqlite3_blob_write HANDLE OFFSET DATA ?NDATA?
**
**   This command is used to test the sqlite3_blob_write() in ways that
**   the Tcl channel interface does not. The first argument should
**   be the name of a valid channel created by the [incrblob] method
**   of a database handle. This function calls sqlite3_blob_write()
**   to write the DATA byte-array to the underlying SQLite blob handle.
**   at offset OFFSET.
**
**   On success, an empty string is returned. On failure, the interpreter
**   result is set to the text representation of the returned error code 
**   (i.e. "SQLITE_NOMEM") and a Tcl exception is thrown.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণাঙ্ক iOffset;
  পূর্ণাঙ্ক rc;

  unsigned বর্ণ *zBuf;
  Tcl_Size nBuf;
  পূর্ণাঙ্ক n;
  
  যদি( objc!=4 && objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) প্রত্যাগম TCL_ERROR;
  যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    প্রত্যাগম TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (পূর্ণাঙ্ক)(nBuf & 0x7fffffff);
  যদি( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    প্রত্যাগম TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  যদি( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  প্রত্যাগম (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  স্থির সংরচনা {
     বর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  পূর্ণাঙ্ক i;
  প্রতি(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  প্রত্যাগম TCL_OK;
}
