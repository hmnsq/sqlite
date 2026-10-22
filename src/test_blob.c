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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef SQLITE_OMIT_INCRBLOB

/* These functions are implemented in main.c. */
extern const var_na *sqlite3ErrName(ចំនួនគត់);

/* From test1.c: */
extern ចំនួនគត់ getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);
extern void *sqlite3TestTextToPtr(const var_na *z);

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
static ចំនួនគត់ blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  var_na *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  ប្រសិនបើ( n==0 ){
    *ppBlob = 0;
  }បើមិនដូច្នេះ ប្រសិនបើ( n>9 && 0==memcmp("incrblob_", z, 9) ){
    ចំនួនគត់ notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    ប្រសិនបើ( !channel ) ត្រឡប់ TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }បើមិនដូច្នេះ{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  ត្រឡប់ TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
static var_na *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  var_na *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  ត្រឡប់ (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
static ចំនួនគត់ SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  ចំនួនគត់ objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  const var_na *zDb;
  const var_na *zTable;
  const var_na *zColumn;
  Tcl_WideInt iRowid;
  ចំនួនគត់ flags;
  const var_na *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  ចំនួនគត់ rc;

  ប្រសិនបើ( objc!=8 ){
    const var_na *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_WrongNumArgs(interp, 1, objv, zUsage);
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ត្រឡប់ TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  ប្រសិនបើ( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) ត្រឡប់ TCL_ERROR;
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[6], &flags) ) ត្រឡប់ TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  ប្រសិនបើ( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }បើមិនដូច្នេះ{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }បើមិនដូច្នេះ{
    Tcl_SetResult(interp, (var_na*)sqlite3ErrName(rc), TCL_VOLATILE);
    ត្រឡប់ TCL_ERROR;
  }
  ត្រឡប់ TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
static ចំនួនគត់ SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ចំនួនគត់ rc;
  
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( blobHandleFromObj(interp, objv[1], &pBlob) ) ត្រឡប់ TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  ប្រសិនបើ( rc ){
    Tcl_SetResult(interp, (var_na*)sqlite3ErrName(rc), TCL_VOLATILE);
  }បើមិនដូច្នេះ{
    Tcl_ResetResult(interp);
  }
  ត្រឡប់ TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
static ចំនួនគត់ SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ចំនួនគត់ nByte;
  
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( blobHandleFromObj(interp, objv[1], &pBlob) ) ត្រឡប់ TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  ត្រឡប់ TCL_OK;
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
static ចំនួនគត់ SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ចំនួនគត់ nByte;
  ចំនួនគត់ iOffset;
  unsigned var_na *zBuf = 0;
  ចំនួនគត់ rc;
  
  ប្រសិនបើ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "CHANNEL OFFSET N");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( blobHandleFromObj(interp, objv[1], &pBlob) ) ត្រឡប់ TCL_ERROR;
  ប្រសិនបើ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( nByte>0 ){
    zBuf = (unsigned var_na *)Tcl_AttemptAlloc(nByte);
    ប្រសិនបើ( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, NULL);
      ត្រឡប់ TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  ប្រសិនបើ( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }បើមិនដូច្នេះ{
    Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((var_na *)zBuf);

  ត្រឡប់ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
static ចំនួនគត់ SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  ចំនួនគត់ iOffset;
  ចំនួនគត់ rc;

  unsigned var_na *zBuf;
  Tcl_Size nBuf;
  ចំនួនគត់ n;
  
  ប្រសិនបើ( objc!=4 && objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( blobHandleFromObj(interp, objv[1], &pBlob) ) ត្រឡប់ TCL_ERROR;
  ប្រសិនបើ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    ត្រឡប់ TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (ចំនួនគត់)(nBuf & 0x7fffffff);
  ប្រសិនបើ( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    ត្រឡប់ TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  ប្រសិនបើ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  ត្រឡប់ (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
ចំនួនគត់ Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  ត្រឡប់ TCL_OK;
}
