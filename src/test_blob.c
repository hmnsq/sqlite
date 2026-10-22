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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/* From test1.c: */
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);
બાહ્ય શૂન્ય *sqlite3TestTextToPtr(અચલ અક્ષર *z);

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
સ્થિર પૂર્ણાંક blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  અક્ષર *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  જો( n==0 ){
    *ppBlob = 0;
  }નહીંતર જો( n>9 && 0==memcmp("incrblob_", z, 9) ){
    પૂર્ણાંક notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    જો( !channel ) પાછા TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }નહીંતર{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  પાછા TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
સ્થિર અક્ષર *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  અક્ષર *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  પાછા (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  પૂર્ણાંક objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  અચલ અક્ષર *zDb;
  અચલ અક્ષર *zTable;
  અચલ અક્ષર *zColumn;
  Tcl_WideInt iRowid;
  પૂર્ણાંક flags;
  અચલ અક્ષર *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  પૂર્ણાંક rc;

  જો( objc!=8 ){
    અચલ અક્ષર *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_અંક_Args(interp, 1, objv, zUsage);
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  જો( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[6], &flags) ) પાછા TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  જો( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }નહીંતર{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  જો( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }નહીંતર{
    Tcl_SetResult(interp, (અક્ષર*)sqlite3ErrName(rc), TCL_VOLATILE);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  પૂર્ણાંક rc;
  
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }

  જો( blobHandleFromObj(interp, objv[1], &pBlob) ) પાછા TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  જો( rc ){
    Tcl_SetResult(interp, (અક્ષર*)sqlite3ErrName(rc), TCL_VOLATILE);
  }નહીંતર{
    Tcl_ResetResult(interp);
  }
  પાછા TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  પૂર્ણાંક nByte;
  
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }

  જો( blobHandleFromObj(interp, objv[1], &pBlob) ) પાછા TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  પૂર્ણાંક nByte;
  પૂર્ણાંક iOffset;
  unsigned અક્ષર *zBuf = 0;
  પૂર્ણાંક rc;
  
  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "CHANNEL OFFSET N");
    પાછા TCL_ERROR;
  }

  જો( blobHandleFromObj(interp, objv[1], &pBlob) ) પાછા TCL_ERROR;
  જો( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    પાછા TCL_ERROR;
  }

  જો( nByte>0 ){
    zBuf = (unsigned અક્ષર *)Tcl_AttemptAlloc(nByte);
    જો( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, ખાલી);
      પાછા TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  જો( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }નહીંતર{
    Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((અક્ષર *)zBuf);

  પાછા (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  પૂર્ણાંક iOffset;
  પૂર્ણાંક rc;

  unsigned અક્ષર *zBuf;
  Tcl_Size nBuf;
  પૂર્ણાંક n;
  
  જો( objc!=4 && objc!=5 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    પાછા TCL_ERROR;
  }

  જો( blobHandleFromObj(interp, objv[1], &pBlob) ) પાછા TCL_ERROR;
  જો( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    પાછા TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (પૂર્ણાંક)(nBuf & 0x7fffffff);
  જો( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    પાછા TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  જો( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  પાછા (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  પાછા TCL_OK;
}
