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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/* From test1.c: */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  ಅಕ್ಷರ *z;
  Tcl_ಗಾತ್ರ n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ಆದರೆ( n==0 ){
    *ppBlob = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    ಪೂರ್ಣಾಂಕ not_ಬಳಸಿದ;
    Tcl_Channel channel;
    Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ;
    
    channel = Tcl_GetChannel(interp, z, &not_ಬಳಸಿದ);
    ಆದರೆ( !channel ) ಹಿಂತಿರುಗಿ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instance_ದತ್ತಾಂಶ = Tcl_Get_Channel_Instance_ದತ್ತಾಂಶ(channel);
    *ppBlob = *((sqlite3_blob **)instance_ದತ್ತಾಂಶ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    *ppBlob = (sqlite3_blob*)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(z);
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
ಸ್ಥಿರ ಅಕ್ಷರ *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_ಗಾತ್ರ n;
  ಅಕ್ಷರ *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ಹಿಂತಿರುಗಿ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_blob_open(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  ಪೂರ್ಣಾಂಕ objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸ್ತಂಭ;
  Tcl_WideInt iRowid;
  ಪೂರ್ಣಾಂಕ flags;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVarname;
  Tcl_ಗಾತ್ರ nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=8 ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, zUsage);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  z_ಕೋಷ್ಟಕ = blobStringFromObj(objv[3]);
  z_ಸ್ತಂಭ = Tcl_GetString(objv[4]);
  ಆದರೆ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ಆದರೆ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, z_ಕೋಷ್ಟಕ, z_ಸ್ತಂಭ, iRowid, flags, &pBlob);
    Tcl_ಗಣ_Var(interp, zVarname, sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(pBlob), 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3_blob_open(db, zDb, z_ಕೋಷ್ಟಕ, z_ಸ್ತಂಭ, iRowid, flags, 0);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ*)sqlite3ErrName(rc), TCL_VOLATILE);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_blob_close(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಂಕ rc;
  
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ಆದರೆ( rc ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ*)sqlite3ErrName(rc), TCL_VOLATILE);
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_blob_ಬೈಟುಗಳು(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಂಕ nByte;
  
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  nByte = sqlite3_blob_ಬೈಟುಗಳು(pBlob);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(nByte));

  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_blob_read(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಂಕ nByte;
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBuf = 0;
  ಪೂರ್ಣಾಂಕ rc;
  
  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "CHANNEL OFFSET N");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_ಸ್ಥಾನಾಂತರ)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( nByte>0 ){
    zBuf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)Tcl_AttemptAlloc(nByte);
    ಆದರೆ( zBuf==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "out of memory in " __FILE__, ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, i_ಸ್ಥಾನಾಂತರ);
  ಆದರೆ( rc==SQLITE_OK ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((ಅಕ್ಷರ *)zBuf);

  ಹಿಂತಿರುಗಿ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_blob_write(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;
  ಪೂರ್ಣಾಂಕ rc;

  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBuf;
  Tcl_ಗಾತ್ರ nBuf;
  ಪೂರ್ಣಾಂಕ n;
  
  ಆದರೆ( objc!=4 && objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( blobHandleFromObj(interp, objv[1], &pBlob) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &i_ಸ್ಥಾನಾಂತರ) ){ 
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (ಪೂರ್ಣಾಂಕ)(nBuf & 0x7fffffff);
  ಆದರೆ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, i_ಸ್ಥಾನಾಂತರ);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ಹಿಂತಿರುಗಿ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest_blob_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_ಬೈಟುಗಳು       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INCRBLOB */
  ಹಿಂತಿರುಗಿ TCL_OK;
}
