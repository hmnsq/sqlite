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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "vdbeInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

/*
** The first argument is a TCL UTF-8 string. Return the byte array
** object with the encoded representation of the string, including
** the NULL terminator.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI binarize(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size len;
  અક્ષર *bytes;
  Tcl_Obj *pRet;
  પ્રતિજ્ઞા(objc==2);

  bytes = Tcl_GetStringFromObj(objv[1], &len);
  pRet = Tcl_NewByteArrayObj((u8*)bytes, len+1);
  Tcl_SetObjResult(interp, pRet);
  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_કિંમત_overhead(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક do_calls;
  પૂર્ણાંક repeat_ગણતરી;
  પૂર્ણાંક i;
  Mem val;

  જો( objc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), " <repeat-count> <do-calls>", ખાલી);
    પાછા TCL_ERROR;
  }

  જો( Tcl_GetIntFromObj(interp, objv[1], &repeat_ગણતરી) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[2], &do_calls) ) પાછા TCL_ERROR;

  val.flags = MEM_Str|MEM_Term|MEM_Static;
  val.z = "hello world";
  val.enc = SQLITE_UTF8;

  માટે(i=0; i<repeat_ગણતરી; i++){
    જો( do_calls ){
      sqlite3_કિંમત_text(&val);
    }
  }

  પાછા TCL_OK;
}

સ્થિર u8 name_to_enc(Tcl_Interp *interp, Tcl_Obj *pObj){
  સંરચના EncName {
    અક્ષર *zName;
    u8 enc;
  } encnames[] = {
    { "UTF8", SQLITE_UTF8 },
    { "UTF16LE", SQLITE_UTF16LE },
    { "UTF16BE", SQLITE_UTF16BE },
    { "UTF16", SQLITE_UTF16 },
    { 0, 0 }
  };
  સંરચના EncName *pEnc;
  અક્ષર *z = Tcl_GetString(pObj);
  માટે(pEnc=&encnames[0]; pEnc->zName; pEnc++){
    જો( 0==sqlite3StrICmp(z, pEnc->zName) ){
      તોડો;
    }
  }
  જો( !pEnc->enc ){
    Tcl_AppendResult(interp, "No such encoding: ", z, ખાલી);
  }
  જો( pEnc->enc==SQLITE_UTF16 ){
    પાછા SQLITE_UTF16NATIVE;
  }
  પાછા pEnc->enc;
}

/*
** Usage:   test_translate <string/blob> <from enc> <to enc> ?<transient>?
**
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_translate(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  u8 enc_from;
  u8 enc_to;
  sqlite3_કિંમત *pVal;

  અક્ષર *z;
  Tcl_Size len;
  શૂન્ય (*xDel)(શૂન્ય *p) = SQLITE_STATIC;

  જો( objc!=4 && objc!=5 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"",
        Tcl_GetStringFromObj(objv[0], 0), 
        " <string/blob> <from enc> <to enc>", ખાલી
    );
    પાછા TCL_ERROR;
  }
  જો( objc==5 ){
    xDel = sqlite3_free;
  }

  enc_from = name_to_enc(interp, objv[2]);
  જો( !enc_from ) પાછા TCL_ERROR;
  enc_to = name_to_enc(interp, objv[3]);
  જો( !enc_to ) પાછા TCL_ERROR;

  pVal = sqlite3_કિંમત_New(0);

  જો( enc_from==SQLITE_UTF8 ){
    z = Tcl_GetString(objv[1]);
    જો( objc==5 ){
      z = sqlite3_mprintf("%s", z);
    }
    sqlite3_કિંમત_Set_Str(pVal, -1, z, enc_from, xDel);
  }નહીંતર{
    z = (અક્ષર*)Tcl_GetByteArrayFromObj(objv[1], &len);
    જો( objc==5 ){
      અક્ષર *zTmp = z;
      z = sqlite3_malloc64(len);
      memcpy(z, zTmp, len);
    }
    sqlite3_કિંમત_Set_Str(pVal, -1, z, enc_from, xDel);
  }

  z = (અક્ષર *)sqlite3_કિંમત_Text(pVal, enc_to);
  len = sqlite3_કિંમત_Bytes(pVal, enc_to) + (enc_to==SQLITE_UTF8?1:2);
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((u8*)z, len));

  sqlite3_કિંમત_Free(pVal);

  પાછા TCL_OK;
}

/*
** Usage: translate_selftest
**
** Call sqlite3UtfSelfTest() to run the internal tests for unicode
** translation. If there is a problem an assert() will fail.
**/
શૂન્ય sqlite3UtfSelfTest(શૂન્ય);
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_translate_selftest(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3UtfSelfTest();
#endif
  પાછા SQLITE_OK;
}


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest5_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
    અક્ષર *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "binarize",                (Tcl_ObjCmdProc*)binarize },
    { "test_value_overhead",     (Tcl_ObjCmdProc*)test_કિંમત_overhead },
    { "test_translate",          (Tcl_ObjCmdProc*)test_translate     },
    { "translate_selftest",      (Tcl_ObjCmdProc*)test_translate_selftest},
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  પાછા SQLITE_OK;
}
