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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI binarize(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  বর্ণ *bytes;
  Tcl_Obj *pRet;
  প্রতিজ্ঞা(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  প্রত্যাগম TCL_OK;
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_মান_overhead(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক do_calls;
  পূর্ণাঙ্ক repeat_গুনতি;
  পূর্ণাঙ্ক i;
  Mem val;

  যদি( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", রিক্ত);
    প্রত্যাগম TCL_ERROR;
  }

  যদি( Tcl_GetIntFromObj(interp, objv[1], &repeat_গুনতি) ) প্রত্যাগম TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) প্রত্যাগম TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  প্রতি(i=0; i<repeat_গুনতি; i++){
    যদি( do_calls ){
      sqlite3_মান_text(&val);
    }
  }

  প্রত্যাগম TCL_OK;
}

স্থির u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  সংরচনা EncName {
    বর্ণ *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  সংরচনা EncName *pEnc;
  বর্ণ *z = Tcl_GetString(pObj);
  প্রতি(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    যদি( 0==sqlite3StrICmp(z, pEnc->zName) ){
      ভঙ্গ;
    }
  }
  যদি( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, রিক্ত);
  }
  যদি( pEnc->enc==SQLITE_UTF16 ){
    প্রত্যাগম SQLITE_UTF16NATIVE;
  }
  প্রত্যাগম pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_translate(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_মান *pVal;

  বর্ণ *z;
  Tcl_Size len;
  শূন্য (*xDel)(শূন্য *p) = SQLITE_STATIC;

  যদি( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", রিক্ত
    );
    প্রত্যাগম TCL_ERROR;
  }
  যদি( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  যদি( !enc_from ) প্রত্যাগম TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  যদি( !enc_to ) প্রত্যাগম TCL_ERROR;

  pVal = sqlite3_মান_New(0);

  যদি( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    যদি( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_মান_Set_Str(pVal, -1, z, enc_from, xDel);
  }অন্যথা{
    z = (বর্ণ*)Tcl_GetByteArrayFromObj(objv[1], &len);
    যদি( objc==5 ){
      বর্ণ *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_মান_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (বর্ণ *)sqlite3_মান_Text(pVal, enc_to);
  len = sqlite3_মান_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_মান_Free(pVal);

  প্রত্যাগম TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
শূন্য sqlite3UtfSelfTest(শূন্য);
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_translate_selftest(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  প্রত্যাগম SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetest5_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
    বর্ণ *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_মান_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  পূর্ণাঙ্ক i;
  প্রতি(i=0; i<পরিমাণ(aCmd)/পরিমাণ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  প্রত্যাগম SQLITE_OK;
}
