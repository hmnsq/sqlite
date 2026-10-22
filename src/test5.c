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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI binarize(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_కొలత len;
  అక్షరం *బైట్లు;
  Tcl_Obj *pRet;
  ప్రతిజ్ఞా(objc==2);

  బైట్లు = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)బైట్లు, len+1);
  Tcl_సమితి_Obj_ఫలితం(interp, pRet);
  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_విలువ_overhead(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం do_calls;
  పూర్ణాంకం repeat_లెక్క;
  పూర్ణాంకం i;
  Mem val;

  ఒకవేళ( objc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &repeat_లెక్క) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) తిరిగి TCL_ERROR;

  val.flags = MEM_Str|MEM_పదం|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  కొరకు(i=0; i<repeat_లెక్క; i++){
    ఒకవేళ( do_calls ){
      sqlite3_విలువ_పాఠ్యం(&val);
    }
  }

  తిరిగి TCL_OK;
}

స్థిర u8 name_గమ్యం_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  నిర్మాణం EncName {
    అక్షరం *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  నిర్మాణం EncName *pEnc;
  అక్షరం *z = Tcl_GetString(pObj);
  కొరకు(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    ఒకవేళ( 0==sqlite3StrICmp(z, pEnc->zName) ){
      ఆపు;
    }
  }
  ఒకవేళ( !pEnc->enc ){
    Tcl_Append_ఫలితం(interp, "No such encoding: ", z, ఖాళీ);
  }
  ఒకవేళ( pEnc->enc==SQLITE_UTF16 ){
    తిరిగి SQLITE_UTF16NATIVE;
  }
  తిరిగి pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_translate(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_గమ్యం;
  sqlite3_విలువ *pVal;

  అక్షరం *z;
  Tcl_కొలత len;
  శూన్య (*xDel)(శూన్య *p) = SQLITE_STATIC;

  ఒకవేళ( objc!=4 && objc!=5 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ఖాళీ
    );
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_గమ్యం_enc(interp, objv[2]);
  ఒకవేళ( !enc_from ) తిరిగి TCL_ERROR;
  enc_గమ్యం = name_గమ్యం_enc(interp, objv[3]);
  ఒకవేళ( !enc_గమ్యం ) తిరిగి TCL_ERROR;

  pVal = sqlite3_విలువ_New(0);

  ఒకవేళ( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    ఒకవేళ( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_విలువ_సమితి_Str(pVal, -1, z, enc_from, xDel);
  }లేకపోతే{
    z = (అక్షరం*)Tcl_GetByteArrayFromObj(objv[1], &len);
    ఒకవేళ( objc==5 ){
      అక్షరం *zTmp = z;
      z = sqlite3_malloc64(len);
      జ్ఞాపకం_నకలు(z, zTmp, len);
    }
    sqlite3_విలువ_సమితి_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (అక్షరం *)sqlite3_విలువ_పాఠ్యం_2(pVal, enc_గమ్యం);
  len = sqlite3_విలువ_బైట్లు_2(pVal, enc_గమ్యం) + (enc_గమ్యం==SQLITE_UTF8?1:2);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_విలువ_Free(pVal);

  తిరిగి TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
శూన్య sqlite3UtfSelfTest(శూన్య);
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_translate_selftest(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest5_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
    అక్షరం *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_విలువ_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  తిరిగి SQLITE_OK;
}
