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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/* From test1.c: */
بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);
بیرونی تهی *sqlite3TestTextToPtr(ثابت نویسه *z);

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
ایستا صحیح blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  نویسه *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  اگر( n==0 ){
    *ppBlob = 0;
  }وگرنه اگر( n>9 && 0==memcmp("incrblob_", z, 9) ){
    صحیح notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    اگر( !channel ) بازگشت TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }وگرنه{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  بازگشت TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ایستا نویسه *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  نویسه *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  بازگشت (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ایستا صحیح SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  صحیح objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ثابت نویسه *zDb;
  ثابت نویسه *zTable;
  ثابت نویسه *zColumn;
  Tcl_WideInt iRowid;
  صحیح flags;
  ثابت نویسه *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  صحیح rc;

  اگر( objc!=8 ){
    ثابت نویسه *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_رقم_Args(interp, 1, objv, zUsage);
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  اگر( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[6], &flags) ) بازگشت TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  اگر( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }وگرنه{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  اگر( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }وگرنه{
    Tcl_SetResult(interp, (نویسه*)sqlite3ErrName(rc), TCL_VOLATILE);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ایستا صحیح SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح rc;
  
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "HANDLE");
    بازگشت TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) بازگشت TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  اگر( rc ){
    Tcl_SetResult(interp, (نویسه*)sqlite3ErrName(rc), TCL_VOLATILE);
  }وگرنه{
    Tcl_ResetResult(interp);
  }
  بازگشت TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ایستا صحیح SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح nByte;
  
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "HANDLE");
    بازگشت TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) بازگشت TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح nByte;
  صحیح iOffset;
  unsigned نویسه *zBuf = 0;
  صحیح rc;
  
  اگر( objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "CHANNEL OFFSET N");
    بازگشت TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) بازگشت TCL_ERROR;
  اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    بازگشت TCL_ERROR;
  }

  اگر( nByte>0 ){
    zBuf = (unsigned نویسه *)Tcl_AttemptAlloc(nByte);
    اگر( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, پوچ);
      بازگشت TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  اگر( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }وگرنه{
    Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((نویسه *)zBuf);

  بازگشت (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ایستا صحیح SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح iOffset;
  صحیح rc;

  unsigned نویسه *zBuf;
  Tcl_Size nBuf;
  صحیح n;
  
  اگر( objc!=4 && objc!=5 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    بازگشت TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) بازگشت TCL_ERROR;
  اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    بازگشت TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (صحیح)(nBuf & 0x7fffffff);
  اگر( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    بازگشت TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  اگر( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  بازگشت (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  بازگشت TCL_OK;
}
