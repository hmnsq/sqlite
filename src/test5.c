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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "vdbeInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI binarize(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  അക്ഷരം *bytes;
  Tcl_Obj *pRet;
  പ്രതിജ്ഞാ(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_മൂല്യം_overhead(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ do_calls;
  പൂർണ്ണസംഖ്യ repeat_എണ്ണം;
  പൂർണ്ണസംഖ്യ i;
  Mem val;

  എങ്കിൽ( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &repeat_എണ്ണം) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) തിരികെ TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  വേണ്ടി(i=0; i<repeat_എണ്ണം; i++){
    എങ്കിൽ( do_calls ){
      sqlite3_മൂല്യം_text(&val);
    }
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ഘടന EncName {
    അക്ഷരം *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ഘടന EncName *pEnc;
  അക്ഷരം *z = Tcl_GetString(pObj);
  വേണ്ടി(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    എങ്കിൽ( 0==sqlite3StrICmp(z, pEnc->zName) ){
      നിർത്തുക;
    }
  }
  എങ്കിൽ( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, ശൂന്യ);
  }
  എങ്കിൽ( pEnc->enc==SQLITE_UTF16 ){
    തിരികെ SQLITE_UTF16NATIVE;
  }
  തിരികെ pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_translate(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_മൂല്യം *pVal;

  അക്ഷരം *z;
  Tcl_Size len;
  ശൂന്യം (*xDel)(ശൂന്യം *p) = SQLITE_STATIC;

  എങ്കിൽ( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ശൂന്യ
    );
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  എങ്കിൽ( !enc_from ) തിരികെ TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  എങ്കിൽ( !enc_to ) തിരികെ TCL_ERROR;

  pVal = sqlite3_മൂല്യം_New(0);

  എങ്കിൽ( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    എങ്കിൽ( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_മൂല്യം_Set_Str(pVal, -1, z, enc_from, xDel);
  }അല്ലെങ്കിൽ{
    z = (അക്ഷരം*)Tcl_GetByteArrayFromObj(objv[1], &len);
    എങ്കിൽ( objc==5 ){
      അക്ഷരം *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_മൂല്യം_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (അക്ഷരം *)sqlite3_മൂല്യം_Text(pVal, enc_to);
  len = sqlite3_മൂല്യം_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_മൂല്യം_Free(pVal);

  തിരികെ TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
ശൂന്യം sqlite3UtfSelfTest(ശൂന്യം);
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_translate_selftest(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  തിരികെ SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest5_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
    അക്ഷരം *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_മൂല്യം_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  തിരികെ SQLITE_OK;
}
