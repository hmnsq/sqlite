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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
ساكن صحيح SQLITE_TCLAPI binarize(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  حرف *bytes;
  Tcl_Obj *pRet;
  أكد(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI test_value_overhead(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح do_calls;
  صحيح repeat_count;
  صحيح i;
  Mem val;

  𐤀𐤌( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", عدم);
    أرجع TCL_ERROR;
  }

  𐤀𐤌( Tcl_GetIntFromObj(interp, objv[1], &repeat_count) ) أرجع TCL_ERROR;
  𐤀𐤌( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) أرجع TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  لكل(i=0; i<repeat_count; i++){
    𐤀𐤌( do_calls ){
      sqlite3_value_text(&val);
    }
  }

  أرجع TCL_OK;
}

ساكن u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  بنية EncName {
    حرف *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  بنية EncName *pEnc;
  حرف *z = Tcl_GetString(pObj);
  لكل(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    𐤀𐤌( 0==sqlite3StrICmp(z, pEnc->zName) ){
      اكسر;
    }
  }
  𐤀𐤌( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, عدم);
  }
  𐤀𐤌( pEnc->enc==SQLITE_UTF16 ){
    أرجع SQLITE_UTF16NATIVE;
  }
  أرجع pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
ساكن صحيح SQLITE_TCLAPI test_translate(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_value *pVal;

  حرف *z;
  Tcl_Size len;
  فراغ (*xDel)(فراغ *p) = SQLITE_STATIC;

  𐤀𐤌( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", عدم
    );
    أرجع TCL_ERROR;
  }
  𐤀𐤌( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  𐤀𐤌( !enc_from ) أرجع TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  𐤀𐤌( !enc_to ) أرجع TCL_ERROR;

  pVal = sqlite3ValueNew(0);

  𐤀𐤌( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    𐤀𐤌( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3ValueSetStr(pVal, -1, z, enc_from, xDel);
  }وإلا{
    z = (حرف*)Tcl_GetByteArrayFromObj(objv[1], &len);
    𐤀𐤌( objc==5 ){
      حرف *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3ValueSetStr(pVal, -1, z, enc_from, xDel);
  }

  z = (حرف *)sqlite3ValueText(pVal, enc_to);
  len = sqlite3ValueBytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3ValueFree(pVal);

  أرجع TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
فراغ sqlite3UtfSelfTest(فراغ);
ساكن صحيح SQLITE_TCLAPI test_translate_selftest(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  أرجع SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest5_Init(Tcl_Interp *interp){
  ساكن بنية {
    حرف *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_value_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  صحيح i;
  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  أرجع SQLITE_OK;
}
