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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/* From test1.c: */
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);
ਬਾਹਰੀ ਖਾਲੀ *sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(ਅਚਲ ਅੱਖਰ *z);

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
ਸਥਿਰ ਪੂਰਨ blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  ਅੱਖਰ *z;
  Tcl_ਮਾਪ n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ਜੇ( n==0 ){
    *ppBlob = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    ਪੂਰਨ not_ਵਰਤਿਆ;
    Tcl_Channel channel;
    Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ;
    
    channel = Tcl_GetChannel(interp, z, &not_ਵਰਤਿਆ);
    ਜੇ( !channel ) ਵਾਪਸ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instance_ਅੰਕੜੇ = Tcl_Get_Channel_Instance_ਅੰਕੜੇ(channel);
    *ppBlob = *((sqlite3_blob **)instance_ਅੰਕੜੇ);
  }ਨਹੀਂਤਾਂ{
    *ppBlob = (sqlite3_blob*)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(z);
  }

  ਵਾਪਸ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ਸਥਿਰ ਅੱਖਰ *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_ਮਾਪ n;
  ਅੱਖਰ *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ਵਾਪਸ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_blob_open(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  ਪੂਰਨ objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ਅਚਲ ਅੱਖਰ *zDb;
  ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ;
  ਅਚਲ ਅੱਖਰ *z_ਕਾਲਮ;
  Tcl_WideInt iRowid;
  ਪੂਰਨ flags;
  ਅਚਲ ਅੱਖਰ *zVarname;
  Tcl_ਮਾਪ nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  ਪੂਰਨ rc;

  ਜੇ( objc!=8 ){
    ਅਚਲ ਅੱਖਰ *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, zUsage);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  z_ਸਾਰਣੀ = blobStringFromObj(objv[3]);
  z_ਕਾਲਮ = Tcl_GetString(objv[4]);
  ਜੇ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ਵਾਪਸ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ਜੇ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, z_ਸਾਰਣੀ, z_ਕਾਲਮ, iRowid, flags, &pBlob);
    Tcl_ਸੈੱਟ_Var(interp, zVarname, sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(pBlob), 0);
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3_blob_open(db, zDb, z_ਸਾਰਣੀ, z_ਕਾਲਮ, iRowid, flags, 0);
  }

  ਜੇ( rc==SQLITE_OK ){
    Tcl_Reset_ਨਤੀਜਾ(interp);
  }ਨਹੀਂਤਾਂ{
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ*)sqlite3ErrName(rc), TCL_VOLATILE);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_blob_close(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ਪੂਰਨ rc;
  
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( blobHandleFromObj(interp, objv[1], &pBlob) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ਜੇ( rc ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ*)sqlite3ErrName(rc), TCL_VOLATILE);
  }ਨਹੀਂਤਾਂ{
    Tcl_Reset_ਨਤੀਜਾ(interp);
  }
  ਵਾਪਸ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_blob_ਬਾਈਟਾਂ(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ਪੂਰਨ nByte;
  
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( blobHandleFromObj(interp, objv[1], &pBlob) ) ਵਾਪਸ TCL_ERROR;
  nByte = sqlite3_blob_ਬਾਈਟਾਂ(pBlob);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(nByte));

  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_blob_read(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ਪੂਰਨ nByte;
  ਪੂਰਨ i_ਵਿੱਥ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBuf = 0;
  ਪੂਰਨ rc;
  
  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "CHANNEL OFFSET N");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( blobHandleFromObj(interp, objv[1], &pBlob) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_ਵਿੱਥ)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( nByte>0 ){
    zBuf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)Tcl_AttemptAlloc(nByte);
    ਜੇ( zBuf==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "out of memory in " __FILE__, ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, i_ਵਿੱਥ);
  ਜੇ( rc==SQLITE_OK ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }ਨਹੀਂਤਾਂ{
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((ਅੱਖਰ *)zBuf);

  ਵਾਪਸ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_blob_write(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ਪੂਰਨ i_ਵਿੱਥ;
  ਪੂਰਨ rc;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBuf;
  Tcl_ਮਾਪ nBuf;
  ਪੂਰਨ n;
  
  ਜੇ( objc!=4 && objc!=5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( blobHandleFromObj(interp, objv[1], &pBlob) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_ਵਿੱਥ) ){ 
    ਵਾਪਸ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (ਪੂਰਨ)(nBuf & 0x7fffffff);
  ਜੇ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ਵਾਪਸ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, i_ਵਿੱਥ);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ਵਾਪਸ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest_blob_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_ਬਾਈਟਾਂ       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INCRBLOB */
  ਵਾਪਸ TCL_OK;
}
