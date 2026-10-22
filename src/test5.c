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
स्थिर पूर्णाङ्क SQLITE_TCLAPI binarize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  वर्ण *bytes;
  Tcl_Obj *pRet;
  प्रतिज्ञा(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  पटिनिवत्त TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_मानम्_overhead(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क do_calls;
  पूर्णाङ्क repeat_गणनम्;
  पूर्णाङ्क i;
  Mem val;

  सचे( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", रिक्त);
    पटिनिवत्त TCL_ERROR;
  }

  सचे( Tcl_GetIntFromObj(interp, objv[1], &repeat_गणनम्) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) पटिनिवत्त TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  अत्थाय(i=0; i<repeat_गणनम्; i++){
    सचे( do_calls ){
      sqlite3_मानम्_text(&val);
    }
  }

  पटिनिवत्त TCL_OK;
}

स्थिर u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  संरचना EncName {
    वर्ण *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  संरचना EncName *pEnc;
  वर्ण *z = Tcl_GetString(pObj);
  अत्थाय(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    सचे( 0==sqlite3StrICmp(z, pEnc->zName) ){
      भङ्ग;
    }
  }
  सचे( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, रिक्त);
  }
  सचे( pEnc->enc==SQLITE_UTF16 ){
    पटिनिवत्त SQLITE_UTF16NATIVE;
  }
  पटिनिवत्त pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_translate(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_मानम् *pVal;

  वर्ण *z;
  Tcl_Size len;
  शून्य (*xDel)(शून्य *p) = SQLITE_STATIC;

  सचे( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", रिक्त
    );
    पटिनिवत्त TCL_ERROR;
  }
  सचे( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  सचे( !enc_from ) पटिनिवत्त TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  सचे( !enc_to ) पटिनिवत्त TCL_ERROR;

  pVal = sqlite3_मानम्_New(0);

  सचे( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    सचे( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_मानम्_Set_Str(pVal, -1, z, enc_from, xDel);
  }अञ्ञथा{
    z = (वर्ण*)Tcl_GetByteArrayFromObj(objv[1], &len);
    सचे( objc==5 ){
      वर्ण *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_मानम्_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (वर्ण *)sqlite3_मानम्_Text(pVal, enc_to);
  len = sqlite3_मानम्_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_मानम्_Free(pVal);

  पटिनिवत्त TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
शून्य sqlite3UtfSelfTest(शून्य);
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_translate_selftest(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  पटिनिवत्त SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest5_Init(Tcl_Interp *interp){
  स्थिर संरचना {
    वर्ण *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_मानम्_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  पूर्णाङ्क i;
  अत्थाय(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  पटिनिवत्त SQLITE_OK;
}
