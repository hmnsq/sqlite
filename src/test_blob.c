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
#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
خارجي ثابت حرف *sqlite3ErrName(صحيح);

/* From test1.c: */
خارجي صحيح get_Db_اشاره_ګر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي فراغ *sqlite3_Test_متن_منزل_Ptr(ثابت حرف *z);

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
  Tcl_اندازه n;

  z = Tcl_GetStringFromObj(pObj, &n);
  إذا( n==0 ){
    *ppBlob = 0;
  }وإلا إذا( n>9 && 0==memcmp("incrblob_", z, 9) ){
    صحيح not_کارول_شوی;
    Tcl_Channel channel;
    Client_معلومات instance_معلومات;
    
    channel = Tcl_GetChannel(interp, z, &not_کارول_شوی);
    إذا( !channel ) أرجع TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instance_معلومات = Tcl_Get_Channel_Instance_معلومات(channel);
    *ppBlob = *((sqlite3_blob **)instance_معلومات);
  }وإلا{
    *ppBlob = (sqlite3_blob*)sqlite3_Test_متن_منزل_Ptr(z);
  }

  أرجع TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ساكن حرف *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_اندازه n;
  حرف *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  أرجع (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ساكن صحيح SQLITE_TCLAPI test_blob_open(
  Client_معلومات client_معلومات,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  صحيح objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ثابت حرف *zDb;
  ثابت حرف *z_جدول;
  ثابت حرف *z_ستنه;
  Tcl_WideInt iRowid;
  صحيح flags;
  ثابت حرف *zVarname;
  Tcl_اندازه nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  صحيح rc;

  إذا( objc!=8 ){
    ثابت حرف *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, zUsage);
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  z_جدول = blobStringFromObj(objv[3]);
  z_ستنه = Tcl_GetString(objv[4]);
  إذا( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[6], &flags) ) أرجع TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  إذا( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, z_جدول, z_ستنه, iRowid, flags, &pBlob);
    Tcl_سېټ_Var(interp, zVarname, sqlite3_Test_Ptr_منزل_متن(pBlob), 0);
  }وإلا{
    rc = sqlite3_blob_open(db, zDb, z_جدول, z_ستنه, iRowid, flags, 0);
  }

  إذا( rc==SQLITE_OK ){
    Tcl_Reset_پایله(interp);
  }وإلا{
    Tcl_سېټ_پایله(interp, (حرف*)sqlite3ErrName(rc), TCL_VOLATILE);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_blob_close(
  Client_معلومات client_معلومات, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح rc;
  
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }

  إذا( blobHandleFromObj(interp, objv[1], &pBlob) ) أرجع TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  إذا( rc ){
    Tcl_سېټ_پایله(interp, (حرف*)sqlite3ErrName(rc), TCL_VOLATILE);
  }وإلا{
    Tcl_Reset_پایله(interp);
  }
  أرجع TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_blob_بایټونه(
  Client_معلومات client_معلومات, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح nByte;
  
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }

  إذا( blobHandleFromObj(interp, objv[1], &pBlob) ) أرجع TCL_ERROR;
  nByte = sqlite3_blob_بایټونه(pBlob);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(nByte));

  أرجع TCL_OK;
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
  Client_معلومات client_معلومات, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح nByte;
  صحيح i_دننه_واټن;
  بینښه حرف *zBuf = 0;
  صحيح rc;
  
  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "CHANNEL OFFSET N");
    أرجع TCL_ERROR;
  }

  إذا( blobHandleFromObj(interp, objv[1], &pBlob) ) أرجع TCL_ERROR;
  إذا( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_دننه_واټن)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    أرجع TCL_ERROR;
  }

  إذا( nByte>0 ){
    zBuf = (بینښه حرف *)Tcl_AttemptAlloc(nByte);
    إذا( zBuf==0 ){
      Tcl_Append_پایله(interp, "out of memory in " __FILE__, عدم);
      أرجع TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, i_دننه_واټن);
  إذا( rc==SQLITE_OK ){
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }وإلا{
    Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((حرف *)zBuf);

  أرجع (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
  Client_معلومات client_معلومات, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحيح i_دننه_واټن;
  صحيح rc;

  بینښه حرف *zBuf;
  Tcl_اندازه nBuf;
  صحيح n;
  
  إذا( objc!=4 && objc!=5 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    أرجع TCL_ERROR;
  }

  إذا( blobHandleFromObj(interp, objv[1], &pBlob) ) أرجع TCL_ERROR;
  إذا( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_دننه_واټن) ){ 
    أرجع TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (صحيح)(nBuf & 0x7fffffff);
  إذا( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    أرجع TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, i_دننه_واټن);
  إذا( rc!=SQLITE_OK ){
    Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  أرجع (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#د_که_پای /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest_blob_Init(Tcl_Interp *interp){
#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_بایټونه       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#د_که_پای /* SQLITE_OMIT_INCRBLOB */
  أرجع TCL_OK;
}
