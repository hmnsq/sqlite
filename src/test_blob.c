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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/* From test1.c: */
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3TestTextToPtr(ಧ್ರುವ ವರ್ಣ *z);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  ವರ್ಣ *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ಯದಿ( n==0 ){
    *ppBlob = 0;
  }ಅನ್ಯಥಾ ಯದಿ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    ಪೂರ್ಣಾಙ್ಕ notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    ಯದಿ( !channel ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }ಅನ್ಯಥಾ{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ಸ್ಥಿರ ವರ್ಣ *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  ವರ್ಣ *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ಪ್ರತ್ಯಾಗಮ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  ಪೂರ್ಣಾಙ್ಕ objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ಧ್ರುವ ವರ್ಣ *zDb;
  ಧ್ರುವ ವರ್ಣ *zTable;
  ಧ್ರುವ ವರ್ಣ *zColumn;
  Tcl_WideInt iRowid;
  ಪೂರ್ಣಾಙ್ಕ flags;
  ಧ್ರುವ ವರ್ಣ *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=8 ){
    ಧ್ರುವ ವರ್ಣ *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, zUsage);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  ಯದಿ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ಯದಿ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }ಅನ್ಯಥಾ{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  ಯದಿ( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }ಅನ್ಯಥಾ{
    Tcl_SetResult(interp, (ವರ್ಣ*)sqlite3ErrName(rc), TCL_VOLATILE);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಙ್ಕ rc;
  
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ಯದಿ( rc ){
    Tcl_SetResult(interp, (ವರ್ಣ*)sqlite3ErrName(rc), TCL_VOLATILE);
  }ಅನ್ಯಥಾ{
    Tcl_ResetResult(interp);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಙ್ಕ nByte;
  
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ಪೂರ್ಣಾಙ್ಕ iOffset;
  unsigned ವರ್ಣ *zBuf = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  
  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "CHANNEL OFFSET N");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( nByte>0 ){
    zBuf = (unsigned ವರ್ಣ *)Tcl_AttemptAlloc(nByte);
    ಯದಿ( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  ಯದಿ( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }ಅನ್ಯಥಾ{
    Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((ವರ್ಣ *)zBuf);

  ಪ್ರತ್ಯಾಗಮ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಙ್ಕ iOffset;
  ಪೂರ್ಣಾಙ್ಕ rc;

  unsigned ವರ್ಣ *zBuf;
  Tcl_Size nBuf;
  ಪೂರ್ಣಾಙ್ಕ n;
  
  ಯದಿ( objc!=4 && objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (ಪೂರ್ಣಾಙ್ಕ)(nBuf & 0x7fffffff);
  ಯದಿ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ಪ್ರತ್ಯಾಗಮ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
