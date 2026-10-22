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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "vdbeInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
நிலையான முழு SQLITE_TCLAPI binarize(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  எழுத்து *bytes;
  Tcl_Obj *pRet;
  ப்ரதிஜ்ஞா(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI test_மதிப்பு_overhead(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு do_calls;
  முழு repeat_மொத்தம்;
  முழு i;
  Mem val;

  எனில்( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", வெற்று);
    திரும்பு TCL_ERROR;
  }

  எனில்( Tcl_GetIntFromObj(interp, objv[1], &repeat_மொத்தம்) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) திரும்பு TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  க்கு(i=0; i<repeat_மொத்தம்; i++){
    எனில்( do_calls ){
      sqlite3_மதிப்பு_text(&val);
    }
  }

  திரும்பு TCL_OK;
}

நிலையான u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  கட்டமைப்பு EncName {
    எழுத்து *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  கட்டமைப்பு EncName *pEnc;
  எழுத்து *z = Tcl_GetString(pObj);
  க்கு(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    எனில்( 0==sqlite3StrICmp(z, pEnc->zName) ){
      நிறுத்து;
    }
  }
  எனில்( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, வெற்று);
  }
  எனில்( pEnc->enc==SQLITE_UTF16 ){
    திரும்பு SQLITE_UTF16NATIVE;
  }
  திரும்பு pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
நிலையான முழு SQLITE_TCLAPI test_translate(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_மதிப்பு *pVal;

  எழுத்து *z;
  Tcl_Size len;
  வெறுமை (*xDel)(வெறுமை *p) = SQLITE_STATIC;

  எனில்( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", வெற்று
    );
    திரும்பு TCL_ERROR;
  }
  எனில்( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  எனில்( !enc_from ) திரும்பு TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  எனில்( !enc_to ) திரும்பு TCL_ERROR;

  pVal = sqlite3_மதிப்பு_New(0);

  எனில்( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    எனில்( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_மதிப்பு_Set_Str(pVal, -1, z, enc_from, xDel);
  }இல்லையெனில்{
    z = (எழுத்து*)Tcl_GetByteArrayFromObj(objv[1], &len);
    எனில்( objc==5 ){
      எழுத்து *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_மதிப்பு_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (எழுத்து *)sqlite3_மதிப்பு_Text(pVal, enc_to);
  len = sqlite3_மதிப்பு_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_மதிப்பு_Free(pVal);

  திரும்பு TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
வெறுமை sqlite3UtfSelfTest(வெறுமை);
நிலையான முழு SQLITE_TCLAPI test_translate_selftest(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  திரும்பு SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest5_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
    எழுத்து *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_மதிப்பு_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  முழு i;
  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  திரும்பு SQLITE_OK;
}
