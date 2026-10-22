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
ایستا صحیح SQLITE_TCLAPI binarize(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  نویسه *bytes;
  Tcl_Obj *pRet;
  أكد(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI test_مقدار_overhead(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح do_calls;
  صحیح repeat_شمار;
  صحیح i;
  Mem val;

  اگر( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", پوچ);
    بازگشت TCL_ERROR;
  }

  اگر( Tcl_GetIntFromObj(interp, objv[1], &repeat_شمار) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) بازگشت TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  برای(i=0; i<repeat_شمار; i++){
    اگر( do_calls ){
      sqlite3_مقدار_text(&val);
    }
  }

  بازگشت TCL_OK;
}

ایستا u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ساختار EncName {
    نویسه *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ساختار EncName *pEnc;
  نویسه *z = Tcl_GetString(pObj);
  برای(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    اگر( 0==sqlite3StrICmp(z, pEnc->zName) ){
      بشکن;
    }
  }
  اگر( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, پوچ);
  }
  اگر( pEnc->enc==SQLITE_UTF16 ){
    بازگشت SQLITE_UTF16NATIVE;
  }
  بازگشت pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ایستا صحیح SQLITE_TCLAPI test_translate(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_مقدار *pVal;

  نویسه *z;
  Tcl_Size len;
  تهی (*xDel)(تهی *p) = SQLITE_STATIC;

  اگر( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", پوچ
    );
    بازگشت TCL_ERROR;
  }
  اگر( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  اگر( !enc_from ) بازگشت TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  اگر( !enc_to ) بازگشت TCL_ERROR;

  pVal = sqlite3_مقدار_New(0);

  اگر( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    اگر( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_مقدار_Set_Str(pVal, -1, z, enc_from, xDel);
  }وگرنه{
    z = (نویسه*)Tcl_GetByteArrayFromObj(objv[1], &len);
    اگر( objc==5 ){
      نویسه *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_مقدار_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (نویسه *)sqlite3_مقدار_Text(pVal, enc_to);
  len = sqlite3_مقدار_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_مقدار_Free(pVal);

  بازگشت TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
تهی sqlite3UtfSelfTest(تهی);
ایستا صحیح SQLITE_TCLAPI test_translate_selftest(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  بازگشت SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest5_Init(Tcl_Interp *interp){
  ایستا ساختار {
    نویسه *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_مقدار_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  صحیح i;
  برای(i=0; i<اندازه(aCmd)/اندازه(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  بازگشت SQLITE_OK;
}
