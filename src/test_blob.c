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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/* From test1.c: */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);
ବାହ୍ୟ ଶୂନ୍ୟ *sqlite3TestTextToPtr(ଅଚଳ ଅକ୍ଷର *z);

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  ଅକ୍ଷର *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ଯଦି( n==0 ){
    *ppBlob = 0;
  }ଅନ୍ୟଥା ଯଦି( n>9 && 0==memcmp("incrblob_", z, 9) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    ଯଦି( !channel ) ଫେରନ୍ତୁ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }ଅନ୍ୟଥା{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ସ୍ଥିର ଅକ୍ଷର *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  ଅକ୍ଷର *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ଫେରନ୍ତୁ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ଅଚଳ ଅକ୍ଷର *zDb;
  ଅଚଳ ଅକ୍ଷର *zTable;
  ଅଚଳ ଅକ୍ଷର *zColumn;
  Tcl_WideInt iRowid;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
  ଅଚଳ ଅକ୍ଷର *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=8 ){
    ଅଚଳ ଅକ୍ଷର *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, zUsage);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  ଯଦି( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ଫେରନ୍ତୁ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ଯଦି( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }ଅନ୍ୟଥା{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  ଯଦି( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }ଅନ୍ୟଥା{
    Tcl_SetResult(interp, (ଅକ୍ଷର*)sqlite3ErrName(rc), TCL_VOLATILE);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( blobHandleFromObj(interp, objv[1], &pBlob) ) ଫେରନ୍ତୁ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ଯଦି( rc ){
    Tcl_SetResult(interp, (ଅକ୍ଷର*)sqlite3ErrName(rc), TCL_VOLATILE);
  }ଅନ୍ୟଥା{
    Tcl_ResetResult(interp);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( blobHandleFromObj(interp, objv[1], &pBlob) ) ଫେରନ୍ତୁ TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffset;
  unsigned ଅକ୍ଷର *zBuf = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  
  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "CHANNEL OFFSET N");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( blobHandleFromObj(interp, objv[1], &pBlob) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( nByte>0 ){
    zBuf = (unsigned ଅକ୍ଷର *)Tcl_AttemptAlloc(nByte);
    ଯଦି( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  ଯଦି( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }ଅନ୍ୟଥା{
    Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((ଅକ୍ଷର *)zBuf);

  ଫେରନ୍ତୁ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffset;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  unsigned ଅକ୍ଷର *zBuf;
  Tcl_Size nBuf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  
  ଯଦି( objc!=4 && objc!=5 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( blobHandleFromObj(interp, objv[1], &pBlob) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(nBuf & 0x7fffffff);
  ଯଦି( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ଫେରନ୍ତୁ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  ଫେରନ୍ତୁ TCL_OK;
}
