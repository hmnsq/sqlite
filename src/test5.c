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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI binarize(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ಗಾತ್ರ len;
  ಅಕ್ಷರ *ಬೈಟುಗಳು;
  Tcl_Obj *pRet;
  ಪ್ರತಿಜ್ಞಾ(objc==2);

  ಬೈಟುಗಳು = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)ಬೈಟುಗಳು, len+1);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_ಮೌಲ್ಯ_overhead(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ do_calls;
  ಪೂರ್ಣಾಂಕ repeat_ಎಣಿಕೆ;
  ಪೂರ್ಣಾಂಕ i;
  Mem val;

  ಆದರೆ( objc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &repeat_ಎಣಿಕೆ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  val.flags = MEM_Str|MEM_ಪದ|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  ಗಾಗಿ(i=0; i<repeat_ಎಣಿಕೆ; i++){
    ಆದರೆ( do_calls ){
      sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(&val);
    }
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ u8 name_ಗಮ್ಯ_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ರಚನೆ EncName {
    ಅಕ್ಷರ *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ರಚನೆ EncName *pEnc;
  ಅಕ್ಷರ *z = Tcl_GetString(pObj);
  ಗಾಗಿ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    ಆದರೆ( 0==sqlite3StrICmp(z, pEnc->zName) ){
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( !pEnc->enc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "No such encoding: ", z, ಖಾಲಿ);
  }
  ಆದರೆ( pEnc->enc==SQLITE_UTF16 ){
    ಹಿಂತಿರುಗಿ SQLITE_UTF16NATIVE;
  }
  ಹಿಂತಿರುಗಿ pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_translate(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_ಗಮ್ಯ;
  sqlite3_ಮೌಲ್ಯ *pVal;

  ಅಕ್ಷರ *z;
  Tcl_ಗಾತ್ರ len;
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *p) = SQLITE_STATIC;

  ಆದರೆ( objc!=4 && objc!=5 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ಖಾಲಿ
    );
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_ಗಮ್ಯ_enc(interp, objv[2]);
  ಆದರೆ( !enc_from ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  enc_ಗಮ್ಯ = name_ಗಮ್ಯ_enc(interp, objv[3]);
  ಆದರೆ( !enc_ಗಮ್ಯ ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  pVal = sqlite3_ಮೌಲ್ಯ_New(0);

  ಆದರೆ( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    ಆದರೆ( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(pVal, -1, z, enc_from, xDel);
  }ಇಲ್ಲದಿದ್ದರೆ{
    z = (ಅಕ್ಷರ*)Tcl_GetByteArrayFromObj(objv[1], &len);
    ಆದರೆ( objc==5 ){
      ಅಕ್ಷರ *zTmp = z;
      z = sqlite3_malloc64(len);
      ಸ್ಮರಣೆ_ನಕಲು(z, zTmp, len);
    }
    sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, enc_ಗಮ್ಯ);
  len = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(pVal, enc_ಗಮ್ಯ) + (enc_ಗಮ್ಯ==SQLITE_UTF8?1:2);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_ಮೌಲ್ಯ_Free(pVal);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
ಶೂನ್ಯ sqlite3UtfSelfTest(ಶೂನ್ಯ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_translate_selftest(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest5_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
    ಅಕ್ಷರ *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_ಮೌಲ್ಯ_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
