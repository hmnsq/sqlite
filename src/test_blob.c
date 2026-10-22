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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

/* From test1.c: */
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिरांक अक्षर *zA, sqlite3 **ppDb);
बाह्य शून्य *sqlite3TestTextToPtr(स्थिरांक अक्षर *z);

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
स्थिर पूर्णांक blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  अक्षर *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  जर( n==0 ){
    *ppBlob = 0;
  }अन्यथा जर( n>9 && 0==स_भेद("incrblob_", z, 9) ){
    पूर्णांक notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    जर( !channel ) परत TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, शुरू_से);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }अन्यथा{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  परत TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
स्थिर अक्षर *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  अक्षर *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  परत (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  पूर्णांक objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  स्थिरांक अक्षर *zDb;
  स्थिरांक अक्षर *zTable;
  स्थिरांक अक्षर *zColumn;
  Tcl_WideInt iRowid;
  पूर्णांक flags;
  स्थिरांक अक्षर *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  पूर्णांक rc;

  जर( objc!=8 ){
    स्थिरांक अक्षर *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_अंक_Args(interp, 1, objv, zUsage);
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  जर( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[6], &flags) ) परत TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  जर( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }अन्यथा{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  जर( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }अन्यथा{
    Tcl_SetResult(interp, (अक्षर*)sqlite3ErrName(rc), TCL_VOLATILE);
    परत TCL_ERROR;
  }
  परत TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक rc;
  
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    परत TCL_ERROR;
  }

  जर( blobHandleFromObj(interp, objv[1], &pBlob) ) परत TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  जर( rc ){
    Tcl_SetResult(interp, (अक्षर*)sqlite3ErrName(rc), TCL_VOLATILE);
  }अन्यथा{
    Tcl_ResetResult(interp);
  }
  परत TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक nByte;
  
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    परत TCL_ERROR;
  }

  जर( blobHandleFromObj(interp, objv[1], &pBlob) ) परत TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक nByte;
  पूर्णांक iOffset;
  अचिन्हित अक्षर *zBuf = 0;
  पूर्णांक rc;
  
  जर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "CHANNEL OFFSET N");
    परत TCL_ERROR;
  }

  जर( blobHandleFromObj(interp, objv[1], &pBlob) ) परत TCL_ERROR;
  जर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    परत TCL_ERROR;
  }

  जर( nByte>0 ){
    zBuf = (अचिन्हित अक्षर *)Tcl_AttemptAlloc(nByte);
    जर( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, रिक्त);
      परत TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  जर( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }अन्यथा{
    Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((अक्षर *)zBuf);

  परत (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
स्थिर पूर्णांक SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक iOffset;
  पूर्णांक rc;

  अचिन्हित अक्षर *zBuf;
  Tcl_Size nBuf;
  पूर्णांक n;
  
  जर( objc!=4 && objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    परत TCL_ERROR;
  }

  जर( blobHandleFromObj(interp, objv[1], &pBlob) ) परत TCL_ERROR;
  जर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    परत TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (पूर्णांक)(nBuf & 0x7fffffff);
  जर( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    परत TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  जर( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  परत (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  स्थिर संरचना {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  पूर्णांक i;
  साठी(i=0; i<आकार(aObjCmd)/आकार(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  परत TCL_OK;
}
