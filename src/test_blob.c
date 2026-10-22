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
बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);

/* From test1.c: */
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, अचर वर्ण *zA, sqlite3 **ppDb);
बाह्य शून्य *sqlite3TestTextToPtr(अचर वर्ण *z);

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
स्थिर पूर्णाङ्क blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  वर्ण *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  यदि( n==0 ){
    *ppBlob = 0;
  }अन्यथा यदि( n>9 && 0==स_भेद("incrblob_", z, 9) ){
    पूर्णाङ्क notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    यदि( !channel ) फर्काऊ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, शुरू_से);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }अन्यथा{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  फर्काऊ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
स्थिर वर्ण *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  वर्ण *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  फर्काऊ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  पूर्णाङ्क objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  अचर वर्ण *zDb;
  अचर वर्ण *zTable;
  अचर वर्ण *zColumn;
  Tcl_WideInt iRowid;
  पूर्णाङ्क flags;
  अचर वर्ण *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  पूर्णाङ्क rc;

  यदि( objc!=8 ){
    अचर वर्ण *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, zUsage);
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  यदि( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[6], &flags) ) फर्काऊ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  यदि( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }अन्यथा{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  यदि( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }अन्यथा{
    Tcl_SetResult(interp, (वर्ण*)sqlite3ErrName(rc), TCL_VOLATILE);
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णाङ्क rc;
  
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }

  यदि( blobHandleFromObj(interp, objv[1], &pBlob) ) फर्काऊ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  यदि( rc ){
    Tcl_SetResult(interp, (वर्ण*)sqlite3ErrName(rc), TCL_VOLATILE);
  }अन्यथा{
    Tcl_ResetResult(interp);
  }
  फर्काऊ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णाङ्क nByte;
  
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }

  यदि( blobHandleFromObj(interp, objv[1], &pBlob) ) फर्काऊ TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  फर्काऊ TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णाङ्क nByte;
  पूर्णाङ्क iOffset;
  अचिन्हित वर्ण *zBuf = 0;
  पूर्णाङ्क rc;
  
  यदि( objc!=4 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "CHANNEL OFFSET N");
    फर्काऊ TCL_ERROR;
  }

  यदि( blobHandleFromObj(interp, objv[1], &pBlob) ) फर्काऊ TCL_ERROR;
  यदि( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    फर्काऊ TCL_ERROR;
  }

  यदि( nByte>0 ){
    zBuf = (अचिन्हित वर्ण *)Tcl_AttemptAlloc(nByte);
    यदि( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, रिक्त);
      फर्काऊ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  यदि( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }अन्यथा{
    Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((वर्ण *)zBuf);

  फर्काऊ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णाङ्क iOffset;
  पूर्णाङ्क rc;

  अचिन्हित वर्ण *zBuf;
  Tcl_Size nBuf;
  पूर्णाङ्क n;
  
  यदि( objc!=4 && objc!=5 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    फर्काऊ TCL_ERROR;
  }

  यदि( blobHandleFromObj(interp, objv[1], &pBlob) ) फर्काऊ TCL_ERROR;
  यदि( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    फर्काऊ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (पूर्णाङ्क)(nBuf & 0x7fffffff);
  यदि( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    फर्काऊ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  यदि( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  फर्काऊ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  पूर्णाङ्क i;
  लागि(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  फर्काऊ TCL_OK;
}
