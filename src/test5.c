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
static số_nguyên SQLITE_TCLAPI binarize(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  char *bytes;
  Tcl_Obj *pRet;
  assert(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  trả_về TCL_OK;
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
static số_nguyên SQLITE_TCLAPI test_value_overhead(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên do_calls;
  số_nguyên repeat_count;
  số_nguyên i;
  Mem val;

  nếu( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", NULL);
    trả_về TCL_ERROR;
  }

  nếu( Tcl_GetIntFromObj(interp, objv[1], &repeat_count) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) trả_về TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  cho(i=0; i<repeat_count; i++){
    nếu( do_calls ){
      sqlite3_value_text(&val);
    }
  }

  trả_về TCL_OK;
}

static u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  struct EncName {
    char *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  struct EncName *pEnc;
  char *z = Tcl_GetString(pObj);
  cho(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    nếu( 0==sqlite3StrICmp(z, pEnc->zName) ){
      break;
    }
  }
  nếu( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, NULL);
  }
  nếu( pEnc->enc==SQLITE_UTF16 ){
    trả_về SQLITE_UTF16NATIVE;
  }
  trả_về pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
static số_nguyên SQLITE_TCLAPI test_translate(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_value *pVal;

  char *z;
  Tcl_Size len;
  void (*xDel)(void *p) = SQLITE_STATIC;

  nếu( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", NULL
    );
    trả_về TCL_ERROR;
  }
  nếu( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  nếu( !enc_from ) trả_về TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  nếu( !enc_to ) trả_về TCL_ERROR;

  pVal = sqlite3ValueNew(0);

  nếu( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    nếu( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3ValueSetStr(pVal, -1, z, enc_from, xDel);
  }không_thì{
    z = (char*)Tcl_GetByteArrayFromObj(objv[1], &len);
    nếu( objc==5 ){
      char *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3ValueSetStr(pVal, -1, z, enc_from, xDel);
  }

  z = (char *)sqlite3ValueText(pVal, enc_to);
  len = sqlite3ValueBytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3ValueFree(pVal);

  trả_về TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
void sqlite3UtfSelfTest(void);
static số_nguyên SQLITE_TCLAPI test_translate_selftest(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  trả_về SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
số_nguyên Sqlitetest5_Init(Tcl_Interp *interp){
  static struct {
    char *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_value_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  số_nguyên i;
  cho(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  trả_về SQLITE_OK;
}
