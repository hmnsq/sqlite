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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI binarize(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ਮਾਪ len;
  ਅੱਖਰ *ਬਾਈਟਾਂ;
  Tcl_Obj *pRet;
  ਪ੍ਰਤਿਜ੍ਞਾ(objc==2);

  ਬਾਈਟਾਂ = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)ਬਾਈਟਾਂ, len+1);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_ਮੁੱਲ_overhead(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ do_calls;
  ਪੂਰਨ repeat_ਗਿਣਤੀ;
  ਪੂਰਨ i;
  Mem val;

  ਜੇ( objc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &repeat_ਗਿਣਤੀ) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) ਵਾਪਸ TCL_ERROR;

  val.flags = MEM_Str|MEM_ਪਦ|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  ਲਈ(i=0; i<repeat_ਗਿਣਤੀ; i++){
    ਜੇ( do_calls ){
      sqlite3_ਮੁੱਲ_ਪਾਠ(&val);
    }
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ u8 name_ਮੰਜ਼ਿਲ_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ਬਣਤਰ EncName {
    ਅੱਖਰ *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ਬਣਤਰ EncName *pEnc;
  ਅੱਖਰ *z = Tcl_GetString(pObj);
  ਲਈ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    ਜੇ( 0==sqlite3StrICmp(z, pEnc->zName) ){
      ਰੋਕੋ;
    }
  }
  ਜੇ( !pEnc->enc ){
    Tcl_Append_ਨਤੀਜਾ(interp, "No such encoding: ", z, ਨਿਰਾ);
  }
  ਜੇ( pEnc->enc==SQLITE_UTF16 ){
    ਵਾਪਸ SQLITE_UTF16NATIVE;
  }
  ਵਾਪਸ pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_translate(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_ਮੰਜ਼ਿਲ;
  sqlite3_ਮੁੱਲ *pVal;

  ਅੱਖਰ *z;
  Tcl_ਮਾਪ len;
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *p) = SQLITE_STATIC;

  ਜੇ( objc!=4 && objc!=5 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ਨਿਰਾ
    );
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_ਮੰਜ਼ਿਲ_enc(interp, objv[2]);
  ਜੇ( !enc_from ) ਵਾਪਸ TCL_ERROR;
  enc_ਮੰਜ਼ਿਲ = name_ਮੰਜ਼ਿਲ_enc(interp, objv[3]);
  ਜੇ( !enc_ਮੰਜ਼ਿਲ ) ਵਾਪਸ TCL_ERROR;

  pVal = sqlite3_ਮੁੱਲ_New(0);

  ਜੇ( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    ਜੇ( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(pVal, -1, z, enc_from, xDel);
  }ਨਹੀਂਤਾਂ{
    z = (ਅੱਖਰ*)Tcl_GetByteArrayFromObj(objv[1], &len);
    ਜੇ( objc==5 ){
      ਅੱਖਰ *zTmp = z;
      z = sqlite3_malloc64(len);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(z, zTmp, len);
    }
    sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, enc_ਮੰਜ਼ਿਲ);
  len = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ_2(pVal, enc_ਮੰਜ਼ਿਲ) + (enc_ਮੰਜ਼ਿਲ==SQLITE_UTF8?1:2);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_ਮੁੱਲ_Free(pVal);

  ਵਾਪਸ TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
ਖਾਲੀ sqlite3UtfSelfTest(ਖਾਲੀ);
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_translate_selftest(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest5_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
    ਅੱਖਰ *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_ਮੁੱਲ_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ਵਾਪਸ SQLITE_OK;
}
