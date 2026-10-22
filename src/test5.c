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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "vdbeInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI binarize(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  ଅକ୍ଷର *bytes;
  Tcl_Obj *pRet;
  ପ୍ରତିଜ୍ଞା(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_ମୂଲ୍ୟ_overhead(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_calls;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା repeat_ଗଣନା;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Mem val;

  ଯଦି( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &repeat_ଗଣନା) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) ଫେରନ୍ତୁ TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  ପାଇଁ(i=0; i<repeat_ଗଣନା; i++){
    ଯଦି( do_calls ){
      sqlite3_ମୂଲ୍ୟ_text(&val);
    }
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  ସଂରଚନା EncName {
    ଅକ୍ଷର *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  ସଂରଚନା EncName *pEnc;
  ଅକ୍ଷର *z = Tcl_GetString(pObj);
  ପାଇଁ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    ଯଦି( 0==sqlite3StrICmp(z, pEnc->zName) ){
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, ଖାଲି);
  }
  ଯଦି( pEnc->enc==SQLITE_UTF16 ){
    ଫେରନ୍ତୁ SQLITE_UTF16NATIVE;
  }
  ଫେରନ୍ତୁ pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_translate(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_ମୂଲ୍ୟ *pVal;

  ଅକ୍ଷର *z;
  Tcl_Size len;
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *p) = SQLITE_STATIC;

  ଯଦି( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ଖାଲି
    );
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  ଯଦି( !enc_from ) ଫେରନ୍ତୁ TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  ଯଦି( !enc_to ) ଫେରନ୍ତୁ TCL_ERROR;

  pVal = sqlite3_ମୂଲ୍ୟ_New(0);

  ଯଦି( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    ଯଦି( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_ମୂଲ୍ୟ_Set_Str(pVal, -1, z, enc_from, xDel);
  }ଅନ୍ୟଥା{
    z = (ଅକ୍ଷର*)Tcl_GetByteArrayFromObj(objv[1], &len);
    ଯଦି( objc==5 ){
      ଅକ୍ଷର *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_ମୂଲ୍ୟ_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_Text(pVal, enc_to);
  len = sqlite3_ମୂଲ୍ୟ_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_ମୂଲ୍ୟ_Free(pVal);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
ଶୂନ୍ୟ sqlite3UtfSelfTest(ଶୂନ୍ୟ);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_translate_selftest(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  ଫେରନ୍ତୁ SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest5_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
    ଅକ୍ଷର *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_ମୂଲ୍ୟ_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}
