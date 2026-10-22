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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
خارجی ثابت حرف *sqlite3ErrName(صحیح);

/* From test1.c: */
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجی خالی *sqlite3_Test_متن_مقصد_Ptr(ثابت حرف *z);

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
ساکن صحیح blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  حرف *z;
  Tcl_سائز n;

  z = Tcl_GetStringFromObj(pObj, &n);
  اگر( n==0 ){
    *ppBlob = 0;
  }ورنہ اگر( n>9 && 0==memcmp("incrblob_", z, 9) ){
    صحیح not_مستعمل;
    Tcl_Channel channel;
    Client_ڈیٹا instance_ڈیٹا;
    
    channel = Tcl_GetChannel(interp, z, &not_مستعمل);
    اگر( !channel ) واپس TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instance_ڈیٹا = Tcl_Get_Channel_Instance_ڈیٹا(channel);
    *ppBlob = *((sqlite3_blob **)instance_ڈیٹا);
  }ورنہ{
    *ppBlob = (sqlite3_blob*)sqlite3_Test_متن_مقصد_Ptr(z);
  }

  واپس TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ساکن حرف *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_سائز n;
  حرف *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  واپس (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ساکن صحیح SQLITE_TCLAPI test_blob_open(
  Client_ڈیٹا client_ڈیٹا,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  صحیح objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ثابت حرف *zDb;
  ثابت حرف *z_جدول;
  ثابت حرف *z_عمود;
  Tcl_WideInt iRowid;
  صحیح flags;
  ثابت حرف *zVarname;
  Tcl_سائز nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  صحیح rc;

  اگر( objc!=8 ){
    ثابت حرف *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, zUsage);
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  z_جدول = blobStringFromObj(objv[3]);
  z_عمود = Tcl_GetString(objv[4]);
  اگر( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[6], &flags) ) واپس TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  اگر( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, z_جدول, z_عمود, iRowid, flags, &pBlob);
    Tcl_سیٹ_Var(interp, zVarname, sqlite3_Test_Ptr_مقصد_متن(pBlob), 0);
  }ورنہ{
    rc = sqlite3_blob_open(db, zDb, z_جدول, z_عمود, iRowid, flags, 0);
  }

  اگر( rc==SQLITE_OK ){
    Tcl_Reset_نتیجہ(interp);
  }ورنہ{
    Tcl_سیٹ_نتیجہ(interp, (حرف*)sqlite3ErrName(rc), TCL_VOLATILE);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_blob_close(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح rc;
  
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) واپس TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  اگر( rc ){
    Tcl_سیٹ_نتیجہ(interp, (حرف*)sqlite3ErrName(rc), TCL_VOLATILE);
  }ورنہ{
    Tcl_Reset_نتیجہ(interp);
  }
  واپس TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_blob_بائٹیں(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح nByte;
  
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) واپس TCL_ERROR;
  nByte = sqlite3_blob_بائٹیں(pBlob);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(nByte));

  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI test_blob_read(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح nByte;
  صحیح i_انحراف;
  غیرعلامتی حرف *zBuf = 0;
  صحیح rc;
  
  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "CHANNEL OFFSET N");
    واپس TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) واپس TCL_ERROR;
  اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_انحراف)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    واپس TCL_ERROR;
  }

  اگر( nByte>0 ){
    zBuf = (غیرعلامتی حرف *)Tcl_AttemptAlloc(nByte);
    اگر( zBuf==0 ){
      Tcl_Append_نتیجہ(interp, "out of memory in " __FILE__, تہی);
      واپس TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, i_انحراف);
  اگر( rc==SQLITE_OK ){
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }ورنہ{
    Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((حرف *)zBuf);

  واپس (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ساکن صحیح SQLITE_TCLAPI test_blob_write(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  صحیح i_انحراف;
  صحیح rc;

  غیرعلامتی حرف *zBuf;
  Tcl_سائز nBuf;
  صحیح n;
  
  اگر( objc!=4 && objc!=5 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    واپس TCL_ERROR;
  }

  اگر( blobHandleFromObj(interp, objv[1], &pBlob) ) واپس TCL_ERROR;
  اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_انحراف) ){ 
    واپس TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (صحیح)(nBuf & 0x7fffffff);
  اگر( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    واپس TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, i_انحراف);
  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  واپس (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#اگر_ختم /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_blob_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_بائٹیں       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#اگر_ختم /* SQLITE_OMIT_INCRBLOB */
  واپس TCL_OK;
}
