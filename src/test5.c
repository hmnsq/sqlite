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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_आकार len;
  वर्ण *अष्टकानि;
  Tcl_Obj *pRet;
  प्रतिज्ञा(objc==2);

  अष्टकानि = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)अष्टकानि, len+1);
  Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);
  प्रत्यागम TCL_OK;
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क do_calls;
  पूर्णाङ्क repeat_गणनम्;
  पूर्णाङ्क i;
  Mem val;

  यदि( objc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", रिक्त);
    प्रत्यागम TCL_ERROR;
  }

  यदि( Tcl_GetIntFromObj(interp, objv[1], &repeat_गणनम्) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) प्रत्यागम TCL_ERROR;

  val.flags = MEM_Str|MEM_पदांशः|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  प्रति(i=0; i<repeat_गणनम्; i++){
    यदि( do_calls ){
      sqlite3_मानम्_text(&val);
    }
  }

  प्रत्यागम TCL_OK;
}

स्थिर u8 name_गन्तव्यम्_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
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
  प्रति(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    यदि( 0==sqlite3StrICmp(z, pEnc->zName) ){
      भङ्ग;
    }
  }
  यदि( !pEnc->enc ){
    Tcl_Append_परिणाम(interp, "No such encoding: ", z, रिक्त);
  }
  यदि( pEnc->enc==SQLITE_UTF16 ){
    प्रत्यागम SQLITE_UTF16NATIVE;
  }
  प्रत्यागम pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_translate(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_गन्तव्यम्;
  sqlite3_मानम् *pVal;

  वर्ण *z;
  Tcl_आकार len;
  शून्य (*xDel)(शून्य *p) = SQLITE_STATIC;

  यदि( objc!=4 && objc!=5 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", रिक्त
    );
    प्रत्यागम TCL_ERROR;
  }
  यदि( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_गन्तव्यम्_enc(interp, objv[2]);
  यदि( !enc_from ) प्रत्यागम TCL_ERROR;
  enc_गन्तव्यम् = name_गन्तव्यम्_enc(interp, objv[3]);
  यदि( !enc_गन्तव्यम् ) प्रत्यागम TCL_ERROR;

  pVal = sqlite3_मानम्_New(0);

  यदि( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    यदि( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_मानम्_समुच्चयः_Str(pVal, -1, z, enc_from, xDel);
  }अन्यथा{
    z = (वर्ण*)Tcl_GetByteArrayFromObj(objv[1], &len);
    यदि( objc==5 ){
      वर्ण *zTmp = z;
      z = sqlite3_malloc64(len);
      स्मृतिं_प्रतिलिख(z, zTmp, len);
    }
    sqlite3_मानम्_समुच्चयः_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (वर्ण *)sqlite3_मानम्_Text(pVal, enc_गन्तव्यम्);
  len = sqlite3_मानम्_अष्टकानि_2(pVal, enc_गन्तव्यम्) + (enc_गन्तव्यम्==SQLITE_UTF8?1:2);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_मानम्_Free(pVal);

  प्रत्यागम TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
शून्य sqlite3UtfSelfTest(शून्य);
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_translate_selftest(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#यदि_अन्तः
  प्रत्यागम SQLITE_OK;
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
  प्रति(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  प्रत्यागम SQLITE_OK;
}
