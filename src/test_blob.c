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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/* From test1.c: */
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);
ബാഹ്യ ശൂന്യം *sqlite3TestTextToPtr(സ്ഥിരം അക്ഷരം *z);

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
സ്ഥിതം പൂർണ്ണസംഖ്യ blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  അക്ഷരം *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  എങ്കിൽ( n==0 ){
    *ppBlob = 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    പൂർണ്ണസംഖ്യ notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    എങ്കിൽ( !channel ) തിരികെ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }അല്ലെങ്കിൽ{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  തിരികെ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
സ്ഥിതം അക്ഷരം *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  അക്ഷരം *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  തിരികെ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  പൂർണ്ണസംഖ്യ objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  സ്ഥിരം അക്ഷരം *zDb;
  സ്ഥിരം അക്ഷരം *zTable;
  സ്ഥിരം അക്ഷരം *zColumn;
  Tcl_WideInt iRowid;
  പൂർണ്ണസംഖ്യ flags;
  സ്ഥിരം അക്ഷരം *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=8 ){
    സ്ഥിരം അക്ഷരം *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, zUsage);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  എങ്കിൽ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) തിരികെ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  എങ്കിൽ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }അല്ലെങ്കിൽ{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }അല്ലെങ്കിൽ{
    Tcl_SetResult(interp, (അക്ഷരം*)sqlite3ErrName(rc), TCL_VOLATILE);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  പൂർണ്ണസംഖ്യ rc;
  
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( blobHandleFromObj(interp, objv[1], &pBlob) ) തിരികെ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  എങ്കിൽ( rc ){
    Tcl_SetResult(interp, (അക്ഷരം*)sqlite3ErrName(rc), TCL_VOLATILE);
  }അല്ലെങ്കിൽ{
    Tcl_ResetResult(interp);
  }
  തിരികെ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  പൂർണ്ണസംഖ്യ nByte;
  
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( blobHandleFromObj(interp, objv[1], &pBlob) ) തിരികെ TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  പൂർണ്ണസംഖ്യ nByte;
  പൂർണ്ണസംഖ്യ iOffset;
  unsigned അക്ഷരം *zBuf = 0;
  പൂർണ്ണസംഖ്യ rc;
  
  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "CHANNEL OFFSET N");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( blobHandleFromObj(interp, objv[1], &pBlob) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( nByte>0 ){
    zBuf = (unsigned അക്ഷരം *)Tcl_AttemptAlloc(nByte);
    എങ്കിൽ( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  എങ്കിൽ( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }അല്ലെങ്കിൽ{
    Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((അക്ഷരം *)zBuf);

  തിരികെ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  പൂർണ്ണസംഖ്യ iOffset;
  പൂർണ്ണസംഖ്യ rc;

  unsigned അക്ഷരം *zBuf;
  Tcl_Size nBuf;
  പൂർണ്ണസംഖ്യ n;
  
  എങ്കിൽ( objc!=4 && objc!=5 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( blobHandleFromObj(interp, objv[1], &pBlob) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    തിരികെ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (പൂർണ്ണസംഖ്യ)(nBuf & 0x7fffffff);
  എങ്കിൽ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    തിരികെ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  തിരികെ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  തിരികെ TCL_OK;
}
