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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/* From test1.c: */
வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);
வெளி வெறுமை *sqlite3TestTextToPtr(மாறிலி எழுத்து *z);

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
நிலையான முழு blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  எழுத்து *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  எனில்( n==0 ){
    *ppBlob = 0;
  }இல்லையெனில் எனில்( n>9 && 0==memcmp("incrblob_", z, 9) ){
    முழு notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    எனில்( !channel ) திரும்பு TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }இல்லையெனில்{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  திரும்பு TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
நிலையான எழுத்து *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  எழுத்து *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  திரும்பு (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
நிலையான முழு SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  முழு objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  மாறிலி எழுத்து *zDb;
  மாறிலி எழுத்து *zTable;
  மாறிலி எழுத்து *zColumn;
  Tcl_WideInt iRowid;
  முழு flags;
  மாறிலி எழுத்து *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  முழு rc;

  எனில்( objc!=8 ){
    மாறிலி எழுத்து *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, zUsage);
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  எனில்( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[6], &flags) ) திரும்பு TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  எனில்( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }இல்லையெனில்{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  எனில்( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }இல்லையெனில்{
    Tcl_SetResult(interp, (எழுத்து*)sqlite3ErrName(rc), TCL_VOLATILE);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
நிலையான முழு SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  முழு rc;
  
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "HANDLE");
    திரும்பு TCL_ERROR;
  }

  எனில்( blobHandleFromObj(interp, objv[1], &pBlob) ) திரும்பு TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  எனில்( rc ){
    Tcl_SetResult(interp, (எழுத்து*)sqlite3ErrName(rc), TCL_VOLATILE);
  }இல்லையெனில்{
    Tcl_ResetResult(interp);
  }
  திரும்பு TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
நிலையான முழு SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  முழு nByte;
  
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "HANDLE");
    திரும்பு TCL_ERROR;
  }

  எனில்( blobHandleFromObj(interp, objv[1], &pBlob) ) திரும்பு TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  முழு nByte;
  முழு iOffset;
  unsigned எழுத்து *zBuf = 0;
  முழு rc;
  
  எனில்( objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "CHANNEL OFFSET N");
    திரும்பு TCL_ERROR;
  }

  எனில்( blobHandleFromObj(interp, objv[1], &pBlob) ) திரும்பு TCL_ERROR;
  எனில்( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    திரும்பு TCL_ERROR;
  }

  எனில்( nByte>0 ){
    zBuf = (unsigned எழுத்து *)Tcl_AttemptAlloc(nByte);
    எனில்( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, வெற்று);
      திரும்பு TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  எனில்( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }இல்லையெனில்{
    Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((எழுத்து *)zBuf);

  திரும்பு (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
நிலையான முழு SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  முழு iOffset;
  முழு rc;

  unsigned எழுத்து *zBuf;
  Tcl_Size nBuf;
  முழு n;
  
  எனில்( objc!=4 && objc!=5 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    திரும்பு TCL_ERROR;
  }

  எனில்( blobHandleFromObj(interp, objv[1], &pBlob) ) திரும்பு TCL_ERROR;
  எனில்( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    திரும்பு TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (முழு)(nBuf & 0x7fffffff);
  எனில்( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    திரும்பு TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  எனில்( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  திரும்பு (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  திரும்பு TCL_OK;
}
