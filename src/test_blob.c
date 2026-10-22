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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/* From test1.c: */
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);
বাহ্যিক শূন্য *sqlite3TestTextToPtr(ধ্রুবক অক্ষর *z);

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
স্থির পূর্ণসংখ্যা blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  অক্ষর *z;
  Tcl_আকার n;

  z = Tcl_GetStringFromObj(pObj, &n);
  যদি( n==0 ){
    *ppBlob = 0;
  }অন্যথা যদি( n>9 && 0==memcmp("incrblob_", z, 9) ){
    পূর্ণসংখ্যা notUsed;
    Tcl_Channel channel;
    Client_তথ্য instance_তথ্য;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    যদি( !channel ) ফেরত TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instance_তথ্য = Tcl_Get_Channel_Instance_তথ্য(channel);
    *ppBlob = *((sqlite3_blob **)instance_তথ্য);
  }অন্যথা{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  ফেরত TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
স্থির অক্ষর *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_আকার n;
  অক্ষর *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ফেরত (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_blob_open(
  Client_তথ্য client_তথ্য,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  পূর্ণসংখ্যা objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ধ্রুবক অক্ষর *zDb;
  ধ্রুবক অক্ষর *zTable;
  ধ্রুবক অক্ষর *zColumn;
  Tcl_WideInt iRowid;
  পূর্ণসংখ্যা flags;
  ধ্রুবক অক্ষর *zVarname;
  Tcl_আকার nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  পূর্ণসংখ্যা rc;

  যদি( objc!=8 ){
    ধ্রুবক অক্ষর *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, zUsage);
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  যদি( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ফেরত TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  যদি( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }অন্যথা{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  যদি( rc==SQLITE_OK ){
    Tcl_Reset_ফলাফল(interp);
  }অন্যথা{
    Tcl_Set_ফলাফল(interp, (অক্ষর*)sqlite3ErrName(rc), TCL_VOLATILE);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_blob_close(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণসংখ্যা rc;
  
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    ফেরত TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) ফেরত TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  যদি( rc ){
    Tcl_Set_ফলাফল(interp, (অক্ষর*)sqlite3ErrName(rc), TCL_VOLATILE);
  }অন্যথা{
    Tcl_Reset_ফলাফল(interp);
  }
  ফেরত TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_blob_bytes(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণসংখ্যা nByte;
  
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    ফেরত TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) ফেরত TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(nByte));

  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_blob_read(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণসংখ্যা nByte;
  পূর্ণসংখ্যা iOffset;
  unsigned অক্ষর *zBuf = 0;
  পূর্ণসংখ্যা rc;
  
  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "CHANNEL OFFSET N");
    ফেরত TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) ফেরত TCL_ERROR;
  যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ফেরত TCL_ERROR;
  }

  যদি( nByte>0 ){
    zBuf = (unsigned অক্ষর *)Tcl_AttemptAlloc(nByte);
    যদি( zBuf==0 ){
      Tcl_Append_ফলাফল(interp, "out of memory in " __FILE__, ফাঁকা);
      ফেরত TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  যদি( rc==SQLITE_OK ){
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }অন্যথা{
    Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((অক্ষর *)zBuf);

  ফেরত (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_blob_write(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  পূর্ণসংখ্যা iOffset;
  পূর্ণসংখ্যা rc;

  unsigned অক্ষর *zBuf;
  Tcl_আকার nBuf;
  পূর্ণসংখ্যা n;
  
  যদি( objc!=4 && objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ফেরত TCL_ERROR;
  }

  যদি( blobHandleFromObj(interp, objv[1], &pBlob) ) ফেরত TCL_ERROR;
  যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    ফেরত TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (পূর্ণসংখ্যা)(nBuf & 0x7fffffff);
  যদি( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ফেরত TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  যদি( rc!=SQLITE_OK ){
    Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ফেরত (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  ফেরত TCL_OK;
}
