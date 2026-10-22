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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
स्थिर पूर्णांक SQLITE_TCLAPI binarize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  अक्षर *bytes;
  Tcl_Obj *pRet;
  प्रतिज्ञा(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI test_मूल्य_overhead(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक do_calls;
  पूर्णांक repeat_मोजणी;
  पूर्णांक i;
  Mem val;

  जर( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", रिक्त);
    परत TCL_ERROR;
  }

  जर( Tcl_GetIntFromObj(interp, objv[1], &repeat_मोजणी) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) परत TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  साठी(i=0; i<repeat_मोजणी; i++){
    जर( do_calls ){
      sqlite3_मूल्य_text(&val);
    }
  }

  परत TCL_OK;
}

स्थिर u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  संरचना EncName {
    अक्षर *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  संरचना EncName *pEnc;
  अक्षर *z = Tcl_GetString(pObj);
  साठी(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    जर( 0==sqlite3StrICmp(z, pEnc->zName) ){
      थांबा;
    }
  }
  जर( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, रिक्त);
  }
  जर( pEnc->enc==SQLITE_UTF16 ){
    परत SQLITE_UTF16NATIVE;
  }
  परत pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_translate(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_मूल्य *pVal;

  अक्षर *z;
  Tcl_Size len;
  शून्य (*xDel)(शून्य *p) = SQLITE_STATIC;

  जर( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", रिक्त
    );
    परत TCL_ERROR;
  }
  जर( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  जर( !enc_from ) परत TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  जर( !enc_to ) परत TCL_ERROR;

  pVal = sqlite3_मूल्य_New(0);

  जर( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    जर( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_मूल्य_Set_Str(pVal, -1, z, enc_from, xDel);
  }अन्यथा{
    z = (अक्षर*)Tcl_GetByteArrayFromObj(objv[1], &len);
    जर( objc==5 ){
      अक्षर *zTmp = z;
      z = sqlite3_malloc64(len);
      स_नकल(z, zTmp, len);
    }
    sqlite3_मूल्य_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (अक्षर *)sqlite3_मूल्य_Text(pVal, enc_to);
  len = sqlite3_मूल्य_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_मूल्य_Free(pVal);

  परत TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
शून्य sqlite3UtfSelfTest(शून्य);
स्थिर पूर्णांक SQLITE_TCLAPI test_translate_selftest(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  परत SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest5_Init(Tcl_Interp *interp){
  स्थिर संरचना {
    अक्षर *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_मूल्य_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  पूर्णांक i;
  साठी(i=0; i<आकार(aCmd)/आकार(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  परत SQLITE_OK;
}
