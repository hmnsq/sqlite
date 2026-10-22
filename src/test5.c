/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the utf.c module in SQLite.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library. Specifically, the code in this file
** is used for testing the SQLite routines for converting between
** the various supported unicode encodings.
*/
#include "sqliteInt.h"
#include "vdbeInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
static จำนวนเต็ม SQLITE_TCLAPI binarize(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  var_na *bytes;
  Tcl_Obj *pRet;
  assert(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  คืน TCL_OK;
}

/*
** Usage: test_value_overhead <repeat-count> <do-calls>.
**
** This routine is used to test the overhead of calls to
** sqlite3_value_text(), on a value that contains a UTF-8 string. The idea
** is to figure out whether or not it is a problem to use sqlite3_value
** structures with collation sequence functions.
**
** If <do-calls> is 0, then the calls to sqlite3_value_text() are not
** actually made.
*/
static จำนวนเต็ม SQLITE_TCLAPI test_value_overhead(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  จำนวนเต็ม do_calls;
  จำนวนเต็ม repeat_count;
  จำนวนเต็ม i;
  Mem val;

  ถ้า( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", NULL);
    คืน TCL_ERROR;
  }

  ถ้า( Tcl_GetIntFromObj(interp, objv[1], &repeat_count) ) คืน TCL_ERROR;
  ถ้า( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) คืน TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  สำหรับ(i=0; i<repeat_count; i++){
    ถ้า( do_calls ){
      sqlite3_value_text(&val);
    }
  }

  คืน TCL_OK;
}

static u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  sa_nracanaa EncName {
    var_na *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  sa_nracanaa EncName *pEnc;
  var_na *z = Tcl_GetString(pObj);
  สำหรับ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    ถ้า( 0==sqlite3StrICmp(z, pEnc->zName) ){
      bha_nga;
    }
  }
  ถ้า( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, NULL);
  }
  ถ้า( pEnc->enc==SQLITE_UTF16 ){
    คืน SQLITE_UTF16NATIVE;
  }
  คืน pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
static จำนวนเต็ม SQLITE_TCLAPI test_translate(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_value *pVal;

  var_na *z;
  Tcl_Size len;
  void (*xDel)(void *p) = SQLITE_STATIC;

  ถ้า( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", NULL
    );
    คืน TCL_ERROR;
  }
  ถ้า( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  ถ้า( !enc_from ) คืน TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  ถ้า( !enc_to ) คืน TCL_ERROR;

  pVal = sqlite3ValueNew(0);

  ถ้า( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    ถ้า( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3ValueSetStr(pVal, -1, z, enc_from, xDel);
  }มิฉะนั้น{
    z = (var_na*)Tcl_GetByteArrayFromObj(objv[1], &len);
    ถ้า( objc==5 ){
      var_na *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3ValueSetStr(pVal, -1, z, enc_from, xDel);
  }

  z = (var_na *)sqlite3ValueText(pVal, enc_to);
  len = sqlite3ValueBytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3ValueFree(pVal);

  คืน TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
void sqlite3UtfSelfTest(void);
static จำนวนเต็ม SQLITE_TCLAPI test_translate_selftest(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  คืน SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
จำนวนเต็ม Sqlitetest5_Init(Tcl_Interp *interp){
  static sa_nracanaa {
    var_na *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_value_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  จำนวนเต็ม i;
  สำหรับ(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  คืน SQLITE_OK;
}
