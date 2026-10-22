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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "vdbeInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI binarize(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_আকার len;
  অক্ষর *bytes;
  Tcl_Obj *pRet;
  প্রতিজ্ঞা(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_Set_Obj_ফলাফল(interp, pRet);
  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_মান_overhead(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা do_calls;
  পূর্ণসংখ্যা repeat_গুনতি;
  পূর্ণসংখ্যা i;
  Mem val;

  যদি( objc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  যদি( Tcl_GetIntFromObj(interp, objv[1], &repeat_গুনতি) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) ফেরত TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  জন্য(i=0; i<repeat_গুনতি; i++){
    যদি( do_calls ){
      sqlite3_মান_text(&val);
    }
  }

  ফেরত TCL_OK;
}

স্থির u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  কাঠামো EncName {
    অক্ষর *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  কাঠামো EncName *pEnc;
  অক্ষর *z = Tcl_GetString(pObj);
  জন্য(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    যদি( 0==sqlite3StrICmp(z, pEnc->zName) ){
      বিরতি;
    }
  }
  যদি( !pEnc->enc ){
    Tcl_Append_ফলাফল(interp, "No such encoding: ", z, ফাঁকা);
  }
  যদি( pEnc->enc==SQLITE_UTF16 ){
    ফেরত SQLITE_UTF16NATIVE;
  }
  ফেরত pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_translate(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_মান *pVal;

  অক্ষর *z;
  Tcl_আকার len;
  শূন্য (*xDel)(শূন্য *p) = SQLITE_STATIC;

  যদি( objc!=4 && objc!=5 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ফাঁকা
    );
    ফেরত TCL_ERROR;
  }
  যদি( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  যদি( !enc_from ) ফেরত TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  যদি( !enc_to ) ফেরত TCL_ERROR;

  pVal = sqlite3_মান_New(0);

  যদি( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    যদি( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_মান_Set_Str(pVal, -1, z, enc_from, xDel);
  }অন্যথা{
    z = (অক্ষর*)Tcl_GetByteArrayFromObj(objv[1], &len);
    যদি( objc==5 ){
      অক্ষর *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_মান_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (অক্ষর *)sqlite3_মান_Text(pVal, enc_to);
  len = sqlite3_মান_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_মান_Free(pVal);

  ফেরত TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
শূন্য sqlite3UtfSelfTest(শূন্য);
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_translate_selftest(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  ফেরত SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest5_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
    অক্ষর *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_মান_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aCmd)/আকার(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ফেরত SQLITE_OK;
}
