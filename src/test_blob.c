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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
خارجي ثابت حرف *sqlite3ErrName(صحيح);

/* From test1.c: */
خارجي صحيح getDbPointer(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي فراغ *sqlite3TestTextToPtr(ثابت حرف *z);

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
ساكن صحيح blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  حرف *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ܐܢ( n==0 ){
    *ppBlob = 0;
  }ܐܚܪܢܐ ܐܢ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    صحيح notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    ܐܢ( !channel ) ܦܢܐ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }ܐܚܪܢܐ{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  ܦܢܐ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ساكن حرف *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  حرف *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ܦܢܐ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ساكن صحيح SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  صحيح objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ثابت حرف *zDb;
  ثابت حرف *zTable;
  ثابت حرف *zColumn;
  Tcl_WideInt iRowid;
  صحيح flags;
  ثابت حرف *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  صحيح rc;

  ܐܢ( objc!=8 ){
    ثابت حرف *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_WrongNumArgs(interp, 1, objv, zUsage);
    ܦܢܐ TCL_ERROR;
  }
  ܐܢ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ܦܢܐ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  ܐܢ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ܦܢܐ TCL_ERROR;
  ܐܢ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ܦܢܐ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ܐܢ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }ܐܚܪܢܐ{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  ܐܢ( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }ܐܚܪܢܐ{
    Tcl_SetResult(interp, (حرف*)sqlite3ErrName(rc), TCL_VOLATILE);
    ܦܢܐ TCL_ERROR;
  }
  ܦܢܐ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح rc;
  
  ܐܢ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ܦܢܐ TCL_ERROR;
  }

  ܐܢ( blobHandleFromObj(interp, objv[1], &pBlob) ) ܦܢܐ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ܐܢ( rc ){
    Tcl_SetResult(interp, (حرف*)sqlite3ErrName(rc), TCL_VOLATILE);
  }ܐܚܪܢܐ{
    Tcl_ResetResult(interp);
  }
  ܦܢܐ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح nByte;
  
  ܐܢ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ܦܢܐ TCL_ERROR;
  }

  ܐܢ( blobHandleFromObj(interp, objv[1], &pBlob) ) ܦܢܐ TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  ܦܢܐ TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح nByte;
  صحيح iOffset;
  unsigned حرف *zBuf = 0;
  صحيح rc;
  
  ܐܢ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "CHANNEL OFFSET N");
    ܦܢܐ TCL_ERROR;
  }

  ܐܢ( blobHandleFromObj(interp, objv[1], &pBlob) ) ܦܢܐ TCL_ERROR;
  ܐܢ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ܦܢܐ TCL_ERROR;
  }

  ܐܢ( nByte>0 ){
    zBuf = (unsigned حرف *)Tcl_AttemptAlloc(nByte);
    ܐܢ( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, عدم);
      ܦܢܐ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  ܐܢ( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }ܐܚܪܢܐ{
    Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((حرف *)zBuf);

  ܦܢܐ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ساكن صحيح SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح iOffset;
  صحيح rc;

  unsigned حرف *zBuf;
  Tcl_Size nBuf;
  صحيح n;
  
  ܐܢ( objc!=4 && objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ܦܢܐ TCL_ERROR;
  }

  ܐܢ( blobHandleFromObj(interp, objv[1], &pBlob) ) ܦܢܐ TCL_ERROR;
  ܐܢ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    ܦܢܐ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (صحيح)(nBuf & 0x7fffffff);
  ܐܢ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ܦܢܐ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  ܐܢ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ܦܢܐ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  ܦܢܐ TCL_OK;
}
