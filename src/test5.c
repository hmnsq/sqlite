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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI binarize(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  ವರ್ಣ *bytes;
  Tcl_Obj *pRet;
  ಪ್ರತಿಜ್ಞಾ(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_ಮೌಲ್ಯ_overhead(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ do_calls;
  ಪೂರ್ಣಾಙ್ಕ repeat_ಎಣಿಕೆ;
  ಪೂರ್ಣಾಙ್ಕ i;
  Mem val;

  ಯದಿ( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &repeat_ಎಣಿಕೆ) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  ಪ್ರತಿ(i=0; i<repeat_ಎಣಿಕೆ; i++){
    ಯದಿ( do_calls ){
      sqlite3_ಮೌಲ್ಯ_text(&val);
    }
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ಸಂರಚನಾ EncName {
    ವರ್ಣ *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ಸಂರಚನಾ EncName *pEnc;
  ವರ್ಣ *z = Tcl_GetString(pObj);
  ಪ್ರತಿ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    ಯದಿ( 0==sqlite3StrICmp(z, pEnc->zName) ){
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, ರಿಕ್ತ);
  }
  ಯದಿ( pEnc->enc==SQLITE_UTF16 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_UTF16NATIVE;
  }
  ಪ್ರತ್ಯಾಗಮ pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_translate(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_ಮೌಲ್ಯ *pVal;

  ವರ್ಣ *z;
  Tcl_Size len;
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *p) = SQLITE_STATIC;

  ಯದಿ( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ರಿಕ್ತ
    );
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  ಯದಿ( !enc_from ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  ಯದಿ( !enc_to ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  pVal = sqlite3_ಮೌಲ್ಯ_New(0);

  ಯದಿ( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    ಯದಿ( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_ಮೌಲ್ಯ_Set_Str(pVal, -1, z, enc_from, xDel);
  }ಅನ್ಯಥಾ{
    z = (ವರ್ಣ*)Tcl_GetByteArrayFromObj(objv[1], &len);
    ಯದಿ( objc==5 ){
      ವರ್ಣ *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_ಮೌಲ್ಯ_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (ವರ್ಣ *)sqlite3_ಮೌಲ್ಯ_Text(pVal, enc_to);
  len = sqlite3_ಮೌಲ್ಯ_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_ಮೌಲ್ಯ_Free(pVal);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
ಶೂನ್ಯ sqlite3UtfSelfTest(ಶೂನ್ಯ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_translate_selftest(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest5_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
    ವರ್ಣ *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_ಮೌಲ್ಯ_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
