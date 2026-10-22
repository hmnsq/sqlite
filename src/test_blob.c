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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>
#अगर_अघोषित SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/* From test1.c: */
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);
बाह्य व्योम *sqlite3TestTextToPtr(स्थिर अक्षर *z);

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
अटल पूर्णांक blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  अक्षर *z;
  Tcl_आकार n;

  z = Tcl_GetStringFromObj(pObj, &n);
  अगर( n==0 ){
    *ppBlob = 0;
  }अन्यथा अगर( n>9 && 0==स_भेद("incrblob_", z, 9) ){
    पूर्णांक notUsed;
    Tcl_Channel channel;
    Client_तथ्य instance_तथ्य;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    अगर( !channel ) वापस TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, शुरू_से);

    instance_तथ्य = Tcl_Get_Channel_Instance_तथ्य(channel);
    *ppBlob = *((sqlite3_blob **)instance_तथ्य);
  }अन्यथा{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  वापस TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
अटल अक्षर *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_आकार n;
  अक्षर *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  वापस (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
अटल पूर्णांक SQLITE_TCLAPI test_blob_open(
  Client_तथ्य client_तथ्य,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  पूर्णांक objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  स्थिर अक्षर *zDb;
  स्थिर अक्षर *zTable;
  स्थिर अक्षर *zColumn;
  Tcl_WideInt iRowid;
  पूर्णांक flags;
  स्थिर अक्षर *zVarname;
  Tcl_आकार nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  पूर्णांक rc;

  अगर( objc!=8 ){
    स्थिर अक्षर *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_अंक_Args(interp, 1, objv, zUsage);
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  अगर( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[6], &flags) ) वापस TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  अगर( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }अन्यथा{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  अगर( rc==SQLITE_OK ){
    Tcl_Reset_परिणाम(interp);
  }अन्यथा{
    Tcl_Set_परिणाम(interp, (अक्षर*)sqlite3ErrName(rc), TCL_VOLATILE);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_blob_close(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक rc;
  
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }

  अगर( blobHandleFromObj(interp, objv[1], &pBlob) ) वापस TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  अगर( rc ){
    Tcl_Set_परिणाम(interp, (अक्षर*)sqlite3ErrName(rc), TCL_VOLATILE);
  }अन्यथा{
    Tcl_Reset_परिणाम(interp);
  }
  वापस TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_blob_bytes(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक nByte;
  
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }

  अगर( blobHandleFromObj(interp, objv[1], &pBlob) ) वापस TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(nByte));

  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI test_blob_read(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक nByte;
  पूर्णांक iOffset;
  अचिन्हित अक्षर *zBuf = 0;
  पूर्णांक rc;
  
  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "CHANNEL OFFSET N");
    वापस TCL_ERROR;
  }

  अगर( blobHandleFromObj(interp, objv[1], &pBlob) ) वापस TCL_ERROR;
  अगर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    वापस TCL_ERROR;
  }

  अगर( nByte>0 ){
    zBuf = (अचिन्हित अक्षर *)Tcl_AttemptAlloc(nByte);
    अगर( zBuf==0 ){
      Tcl_Append_परिणाम(interp, "out of memory in " __खाता__, शून्य);
      वापस TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  अगर( rc==SQLITE_OK ){
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }अन्यथा{
    Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((अक्षर *)zBuf);

  वापस (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
अटल पूर्णांक SQLITE_TCLAPI test_blob_write(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  पूर्णांक iOffset;
  पूर्णांक rc;

  अचिन्हित अक्षर *zBuf;
  Tcl_आकार nBuf;
  पूर्णांक n;
  
  अगर( objc!=4 && objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    वापस TCL_ERROR;
  }

  अगर( blobHandleFromObj(interp, objv[1], &pBlob) ) वापस TCL_ERROR;
  अगर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    वापस TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (पूर्णांक)(nBuf & 0x7fffffff);
  अगर( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    वापस TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  अगर( rc!=SQLITE_OK ){
    Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  वापस (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#पूर्ण_अगर /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_blob_Init(Tcl_Interp *interp){
#अगर_अघोषित SQLITE_OMIT_INCRBLOB
  अटल काष्ठा {
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
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#पूर्ण_अगर /* SQLITE_OMIT_INCRBLOB */
  वापस TCL_OK;
}
