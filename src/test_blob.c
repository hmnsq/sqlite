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
extern const char *sqlite3ErrName(целое);

/* From test1.c: */
extern целое getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);
extern void *sqlite3TestTextToPtr(const char *z);

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
static целое blobHandleFromObj(
  Tcl_Interp *interp, 
  Tcl_Obj *pObj,
  sqlite3_blob **ppBlob
){
  char *z;
  Tcl_Size n;

  z = Tcl_GetStringFromObj(pObj, &n);
  если( n==0 ){
    *ppBlob = 0;
  }иначе если( n>9 && 0==memcmp("incrblob_", z, 9) ){
    целое notUsed;
    Tcl_Channel channel;
    ClientData instanceData;
    
    channel = Tcl_GetChannel(interp, z, &notUsed);
    если( !channel ) вернуть TCL_ERROR;

    Tcl_Flush(channel);
    Tcl_Seek(channel, 0, SEEK_SET);

    instanceData = Tcl_GetChannelInstanceData(channel);
    *ppBlob = *((sqlite3_blob **)instanceData);
  }иначе{
    *ppBlob = (sqlite3_blob*)sqlite3TestTextToPtr(z);
  }

  вернуть TCL_OK;
}

/*
** Like Tcl_GetString(), except that if the string is 0 bytes in size, a
** NULL Pointer is returned.
*/
static char *blobStringFromObj(Tcl_Obj *pObj){
  Tcl_Size n;
  char *z;
  z = Tcl_GetStringFromObj(pObj, &n);
  вернуть (n ? z : 0);
}

/*
** sqlite3_blob_open DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME
**
** Tcl test harness for the sqlite3_blob_open() function.
*/
static целое SQLITE_TCLAPI test_blob_open(
  ClientData clientData,          /* Not used */
  Tcl_Interp *interp,             /* Calling TCL interpreter */
  целое objc,                       /* Number of arguments */
  Tcl_Obj *CONST objv[]           /* Command arguments */
){
  sqlite3 *db;
  const char *zDb;
  const char *zTable;
  const char *zColumn;
  Tcl_WideInt iRowid;
  целое flags;
  const char *zVarname;
  Tcl_Size nVarname;

  sqlite3_blob *pBlob = (sqlite3_blob*)&flags;   /* Non-zero initialization */
  целое rc;

  если( objc!=8 ){
    const char *zUsage = "DB DATABASE TABLE COLUMN ROWID FLAGS VARNAME";
    Tcl_WrongNumArgs(interp, 1, objv, zUsage);
    вернуть TCL_ERROR;
  }
  если( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) вернуть TCL_ERROR;
  zDb = Tcl_GetString(objv[2]);
  zTable = blobStringFromObj(objv[3]);
  zColumn = Tcl_GetString(objv[4]);
  если( Tcl_GetWideIntFromObj(interp, objv[5], &iRowid) ) вернуть TCL_ERROR;
  если( Tcl_GetIntFromObj(interp, objv[6], &flags) ) вернуть TCL_ERROR;
  zVarname = Tcl_GetStringFromObj(objv[7], &nVarname);

  если( nVarname>0 ){
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, &pBlob);
    Tcl_SetVar(interp, zVarname, sqlite3TestPtrToText(pBlob), 0);
  }иначе{
    rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRowid, flags, 0);
  }

  если( rc==SQLITE_OK ){
    Tcl_ResetResult(interp);
  }иначе{
    Tcl_SetResult(interp, (char*)sqlite3ErrName(rc), TCL_VOLATILE);
    вернуть TCL_ERROR;
  }
  вернуть TCL_OK;
}


/*
** sqlite3_blob_close  HANDLE
*/
static целое SQLITE_TCLAPI test_blob_close(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  целое rc;
  
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }

  если( blobHandleFromObj(interp, objv[1], &pBlob) ) вернуть TCL_ERROR;
  rc = sqlite3_blob_close(pBlob);

  если( rc ){
    Tcl_SetResult(interp, (char*)sqlite3ErrName(rc), TCL_VOLATILE);
  }иначе{
    Tcl_ResetResult(interp);
  }
  вернуть TCL_OK;
}

/*
** sqlite3_blob_bytes  HANDLE
*/
static целое SQLITE_TCLAPI test_blob_bytes(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  целое nByte;
  
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }

  если( blobHandleFromObj(interp, objv[1], &pBlob) ) вернуть TCL_ERROR;
  nByte = sqlite3_blob_bytes(pBlob);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nByte));

  вернуть TCL_OK;
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
static целое SQLITE_TCLAPI test_blob_read(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  целое nByte;
  целое iOffset;
  unsigned char *zBuf = 0;
  целое rc;
  
  если( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "CHANNEL OFFSET N");
    вернуть TCL_ERROR;
  }

  если( blobHandleFromObj(interp, objv[1], &pBlob) ) вернуть TCL_ERROR;
  если( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset)
   || TCL_OK!=Tcl_GetIntFromObj(interp, objv[3], &nByte)
  ){ 
    вернуть TCL_ERROR;
  }

  если( nByte>0 ){
    zBuf = (unsigned char *)Tcl_AttemptAlloc(nByte);
    если( zBuf==0 ){
      Tcl_AppendResult(interp, "out of memory in " __FILE__, NULL);
      вернуть TCL_ERROR;
    }
  }
  rc = sqlite3_blob_read(pBlob, zBuf, nByte, iOffset);
  если( rc==SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(zBuf, nByte));
  }иначе{
    Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  }
  Tcl_Free((char *)zBuf);

  вернуть (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
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
static целое SQLITE_TCLAPI test_blob_write(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_blob *pBlob;
  целое iOffset;
  целое rc;

  unsigned char *zBuf;
  Tcl_Size nBuf;
  целое n;
  
  если( objc!=4 && objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE OFFSET DATA ?NDATA?");
    вернуть TCL_ERROR;
  }

  если( blobHandleFromObj(interp, objv[1], &pBlob) ) вернуть TCL_ERROR;
  если( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iOffset) ){ 
    вернуть TCL_ERROR;
  }

  zBuf = Tcl_GetByteArrayFromObj(objv[3], &nBuf);
  n = (целое)(nBuf & 0x7fffffff);
  если( objc==5 && Tcl_GetIntFromObj(interp, objv[4], &n) ){
    вернуть TCL_ERROR;
  }
  rc = sqlite3_blob_write(pBlob, zBuf, n, iOffset);
  если( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  }

  вернуть (rc==SQLITE_OK ? TCL_OK : TCL_ERROR);
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Register commands with the TCL interpreter.
*/
целое Sqlitetest_blob_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_INCRBLOB
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "sqlite3_blob_open",            test_blob_open        },
     { "sqlite3_blob_close",           test_blob_close       },
     { "sqlite3_blob_bytes",           test_blob_bytes       },
     { "sqlite3_blob_read",            test_blob_read        },
     { "sqlite3_blob_write",           test_blob_write       },
  };
  целое i;
  для(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
#endif /* SQLITE_OMIT_INCRBLOB */
  вернуть TCL_OK;
}
