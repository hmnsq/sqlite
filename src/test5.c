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
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
ساکن صحیح SQLITE_TCLAPI binarize(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_سائز len;
  حرف *بائٹیں;
  Tcl_Obj *pRet;
  أكد(objc==2);

  بائٹیں = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)بائٹیں, len+1);
  Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI test_قدر_overhead(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح do_calls;
  صحیح repeat_گنتی;
  صحیح i;
  Mem val;

  اگر( objc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", تہی);
    واپس TCL_ERROR;
  }

  اگر( Tcl_GetIntFromObj(interp, objv[1], &repeat_گنتی) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) واپس TCL_ERROR;

  val.flags = MEM_Str|MEM_رکن|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  برائے(i=0; i<repeat_گنتی; i++){
    اگر( do_calls ){
      sqlite3_قدر_متن(&val);
    }
  }

  واپس TCL_OK;
}

ساکن u8 name_مقصد_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ساخت EncName {
    حرف *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ساخت EncName *pEnc;
  حرف *z = Tcl_GetString(pObj);
  برائے(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    اگر( 0==sqlite3StrICmp(z, pEnc->zName) ){
      توڑو;
    }
  }
  اگر( !pEnc->enc ){
    Tcl_Append_نتیجہ(interp, "No such encoding: ", z, تہی);
  }
  اگر( pEnc->enc==SQLITE_UTF16 ){
    واپس SQLITE_UTF16NATIVE;
  }
  واپس pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ساکن صحیح SQLITE_TCLAPI test_translate(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_مقصد;
  sqlite3_قدر *pVal;

  حرف *z;
  Tcl_سائز len;
  خالی (*xDel)(خالی *p) = SQLITE_STATIC;

  اگر( objc!=4 && objc!=5 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", تہی
    );
    واپس TCL_ERROR;
  }
  اگر( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_مقصد_enc(interp, objv[2]);
  اگر( !enc_from ) واپس TCL_ERROR;
  enc_مقصد = name_مقصد_enc(interp, objv[3]);
  اگر( !enc_مقصد ) واپس TCL_ERROR;

  pVal = sqlite3_قدر_New(0);

  اگر( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    اگر( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_قدر_سیٹ_Str(pVal, -1, z, enc_from, xDel);
  }ورنہ{
    z = (حرف*)Tcl_GetByteArrayFromObj(objv[1], &len);
    اگر( objc==5 ){
      حرف *zTmp = z;
      z = sqlite3_malloc64(len);
      حافظہ_نقل(z, zTmp, len);
    }
    sqlite3_قدر_سیٹ_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (حرف *)sqlite3_قدر_متن_2(pVal, enc_مقصد);
  len = sqlite3_قدر_بائٹیں_2(pVal, enc_مقصد) + (enc_مقصد==SQLITE_UTF8?1:2);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_قدر_Free(pVal);

  واپس TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
خالی sqlite3UtfSelfTest(خالی);
ساکن صحیح SQLITE_TCLAPI test_translate_selftest(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#اگر_ختم
  واپس SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest5_Init(Tcl_Interp *interp){
  ساکن ساخت {
    حرف *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_قدر_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  صحیح i;
  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  واپس SQLITE_OK;
}
