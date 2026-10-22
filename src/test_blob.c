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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/* From test1.c: */
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);
బాహ్య శూన్య *sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(స్థిరరాశి అక్షరం *z);

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
స్థిర పూర్ణాంకం blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  అక్షరం *z;
  Tcl_కొలత n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ఒకవేళ( n==0 ){
    *ppBlob = 0;
  }లేకపోతే ఒకవేళ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    పూర్ణాంకం not_వాడిన;
    Tcl_Channel channel;
    Client_దత్తాంశం instance_దత్తాంశం;
    
    channel = Tcl_GetChannel(interp, z, &not_వాడిన);
    ఒకవేళ( !channel ) తిరిగి TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instance_దత్తాంశం = Tcl_Get_Channel_Instance_దత్తాంశం(channel);
    *ppBlob = *((sqlite3_blob **)instance_దత్తాంశం);
  }లేకపోతే{
    *ppBlob = (sqlite3_blob*)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(z);
  }

  తిరిగి TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
స్థిర అక్షరం *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_కొలత n;
  అక్షరం *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  తిరిగి (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_blob_open(
  Client_దత్తాంశం client_దత్తాంశం,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  పూర్ణాంకం objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  స్థిరరాశి అక్షరం *zDb;
  స్థిరరాశి అక్షరం *z_పట్టిక;
  స్థిరరాశి అక్షరం *z_స్తంభం;
  Tcl_WideInt iRowid;
  పూర్ణాంకం flags;
  స్థిరరాశి అక్షరం *zVarname;
  Tcl_కొలత nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=8 ){
    స్థిరరాశి అక్షరం *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, zUsage);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  z_పట్టిక = blobStringFromObj(objv[3]);
  z_స్తంభం = Tcl_GetString(objv[4]);
  ఒకవేళ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) తిరిగి TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ఒకవేళ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, z_పట్టిక, z_స్తంభం, iRowid, flags, &pBlob);
    Tcl_సమితి_Var(interp, zVarname, sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(pBlob), 0);
  }లేకపోతే{
    rc = sqlite3_blob_open(db, zDb, z_పట్టిక, z_స్తంభం, iRowid, flags, 0);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    Tcl_Reset_ఫలితం(interp);
  }లేకపోతే{
    Tcl_సమితి_ఫలితం(interp, (అక్షరం*)sqlite3ErrName(rc), TCL_VOLATILE);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_blob_close(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  పూర్ణాంకం rc;
  
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( blobHandleFromObj(interp, objv[1], &pBlob) ) తిరిగి TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ఒకవేళ( rc ){
    Tcl_సమితి_ఫలితం(interp, (అక్షరం*)sqlite3ErrName(rc), TCL_VOLATILE);
  }లేకపోతే{
    Tcl_Reset_ఫలితం(interp);
  }
  తిరిగి TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_blob_బైట్లు(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  పూర్ణాంకం nByte;
  
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( blobHandleFromObj(interp, objv[1], &pBlob) ) తిరిగి TCL_ERROR;
  nByte = sqlite3_blob_బైట్లు(pBlob);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(nByte));

  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_blob_read(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  పూర్ణాంకం nByte;
  పూర్ణాంకం i_స్థానభ్రంశం;
  గుర్తులేని అక్షరం *zBuf = 0;
  పూర్ణాంకం rc;
  
  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "CHANNEL OFFSET N");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( blobHandleFromObj(interp, objv[1], &pBlob) ) తిరిగి TCL_ERROR;
  ఒకవేళ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_స్థానభ్రంశం)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( nByte>0 ){
    zBuf = (గుర్తులేని అక్షరం *)Tcl_AttemptAlloc(nByte);
    ఒకవేళ( zBuf==0 ){
      Tcl_Append_ఫలితం(interp, "out of memory in " __FILE__, ఖాళీ);
      తిరిగి TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, i_స్థానభ్రంశం);
  ఒకవేళ( rc==SQLITE_OK ){
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }లేకపోతే{
    Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((అక్షరం *)zBuf);

  తిరిగి (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_blob_write(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  పూర్ణాంకం i_స్థానభ్రంశం;
  పూర్ణాంకం rc;

  గుర్తులేని అక్షరం *zBuf;
  Tcl_కొలత nBuf;
  పూర్ణాంకం n;
  
  ఒకవేళ( objc!=4 && objc!=5 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( blobHandleFromObj(interp, objv[1], &pBlob) ) తిరిగి TCL_ERROR;
  ఒకవేళ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_స్థానభ్రంశం) ){ 
    తిరిగి TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (పూర్ణాంకం)(nBuf & 0x7fffffff);
  ఒకవేళ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    తిరిగి TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, i_స్థానభ్రంశం);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  తిరిగి (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#అయితే_ముగింపు /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest_blob_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_బైట్లు       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#అయితే_ముగింపు /* SQLITE_OMIT_INCRBLOB */
  తిరిగి TCL_OK;
}
