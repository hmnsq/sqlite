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
#समावेश "sqliteInt.h"
#समावेश "vdbeInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
अटल पूर्णांक SQLITE_TCLAPI binarize(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_आकार len;
  अक्षर *bytes;
  Tcl_Obj *pRet;
  निश्चित(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_Set_Obj_परिणाम(interp, pRet);
  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI test_मूल्य_overhead(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक do_calls;
  पूर्णांक repeat_गिनती;
  पूर्णांक i;
  Mem val;

  अगर( objc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", शून्य);
    वापस TCL_ERROR;
  }

  अगर( Tcl_GetIntFromObj(interp, objv[1], &repeat_गिनती) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) वापस TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  क्रम(i=0; i<repeat_गिनती; i++){
    अगर( do_calls ){
      sqlite3_मूल्य_text(&val);
    }
  }

  वापस TCL_OK;
}

अटल u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  काष्ठा EncName {
    अक्षर *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  काष्ठा EncName *pEnc;
  अक्षर *z = Tcl_GetString(pObj);
  क्रम(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    अगर( 0==sqlite3StrICmp(z, pEnc->zName) ){
      अवरोध;
    }
  }
  अगर( !pEnc->enc ){
    Tcl_Append_परिणाम(interp, "No such encoding: ", z, शून्य);
  }
  अगर( pEnc->enc==SQLITE_UTF16 ){
    वापस SQLITE_UTF16NATIVE;
  }
  वापस pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
अटल पूर्णांक SQLITE_TCLAPI test_translate(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_मूल्य *pVal;

  अक्षर *z;
  Tcl_आकार len;
  व्योम (*xDel)(व्योम *p) = SQLITE_STATIC;

  अगर( objc!=4 && objc!=5 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", शून्य
    );
    वापस TCL_ERROR;
  }
  अगर( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  अगर( !enc_from ) वापस TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  अगर( !enc_to ) वापस TCL_ERROR;

  pVal = sqlite3_मूल्य_New(0);

  अगर( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    अगर( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_मूल्य_Set_Str(pVal, -1, z, enc_from, xDel);
  }अन्यथा{
    z = (अक्षर*)Tcl_GetByteArrayFromObj(objv[1], &len);
    अगर( objc==5 ){
      अक्षर *zTmp = z;
      z = sqlite3_malloc64(len);
      स_नकल(z, zTmp, len);
    }
    sqlite3_मूल्य_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (अक्षर *)sqlite3_मूल्य_Text(pVal, enc_to);
  len = sqlite3_मूल्य_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_मूल्य_Free(pVal);

  वापस TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
व्योम sqlite3UtfSelfTest(व्योम);
अटल पूर्णांक SQLITE_TCLAPI test_translate_selftest(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#अगर_अघोषित SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#पूर्ण_अगर
  वापस SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest5_Init(Tcl_Interp *interp){
  अटल काष्ठा {
    अक्षर *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_मूल्य_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  वापस SQLITE_OK;
}
