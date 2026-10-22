/*
** 2007 April 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing all sorts of SQLite interfaces.  This code
** implements TCL commands for reading and writing the binary
** database files and displaying the content of those files as
** hexadecimal.  We could, in theory, use the built-in "binary"
** command of TCL to do a lot of this, but there are some issues
** with historical versions of the "binary" command.  So it seems
** easier and safer to build our own mechanism.
*/
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
خالی sqlite3_Test_Bin_مقصد_Hex(غیرعلامتی حرف *zBuf, صحیح N){
  ثابت غیرعلامتی حرف zHex[] = "0123456789ABCDEF";
  صحیح i, j;
  غیرعلامتی حرف c;
  i = N*2;
  zBuf[i--] = 0;
  برائے(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  أكد( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
صحیح sqlite3_Test_Hex_مقصد_Bin(ثابت غیرعلامتی حرف *z_اندر, صحیح N, غیرعلامتی حرف *a_برآمد){
  ثابت غیرعلامتی حرف aMap[] = {
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     1, 2, 3, 4, 5, 6, 7, 8,  9,10, 0, 0, 0, 0, 0, 0,
     0,11,12,13,14,15,16, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0,11,12,13,14,15,16, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  };
  صحیح i, j;
  صحیح hi=1;
  غیرعلامتی حرف c;

  برائے(i=j=0; i<N; i++){
    c = aMap[z_اندر[i]];
    اگر( c==0 ) جاری;
    اگر( hi ){
      a_برآمد[j] = (c-1)<<4;
      hi = 0;
    }ورنہ{
      a_برآمد[j++] |= c-1;
      hi = 1;
    }
  }
  واپس j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ساکن صحیح SQLITE_TCLAPI hexio_read(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح انحراف;
  صحیح amt, got;
  ثابت حرف *zFile;
  غیرعلامتی حرف *zBuf;
  FILE *اندر;

  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME OFFSET AMT");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &انحراف) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[3], &amt) ) واپس TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  اگر( zBuf==0 ){
    واپس TCL_ERROR;
  }
  اندر = fopen(zFile, "rb");
  اگر( اندر==0 ){
    اندر = fopen(zFile, "r");
  }
  اگر( اندر==0 ){
    Tcl_Append_نتیجہ(interp, "cannot open input file ", zFile, تہی);
    واپس TCL_ERROR;
  }
  fseek(اندر, انحراف, SEEK_SET);
  got = (صحیح)fread(zBuf, 1, amt, اندر);
  fclose(اندر);
  اگر( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_مقصد_Hex(zBuf, got);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  sqlite3_free(zBuf);
  واپس TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ساکن صحیح SQLITE_TCLAPI hexio_write(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح انحراف;
  Tcl_سائز n_اندر;
  صحیح n_برآمد, written;
  ثابت حرف *zFile;
  ثابت غیرعلامتی حرف *z_اندر;
  غیرعلامتی حرف *a_برآمد;
  FILE *برآمد;

  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &انحراف) ) واپس TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_اندر = (ثابت غیرعلامتی حرف *)Tcl_GetStringFromObj(objv[3], &n_اندر);
  a_برآمد = sqlite3_malloc64( 1 + n_اندر/2 );
  اگر( a_برآمد==0 ){
    واپس TCL_ERROR;
  }
  n_برآمد = sqlite3_Test_Hex_مقصد_Bin(z_اندر, (صحیح)n_اندر, a_برآمد);
  برآمد = fopen(zFile, "r+b");
  اگر( برآمد==0 ){
    برآمد = fopen(zFile, "r+");
  }
  اگر( برآمد==0 ){
    Tcl_Append_نتیجہ(interp, "cannot open output file ", zFile, تہی);
    واپس TCL_ERROR;
  }
  fseek(برآمد, انحراف, SEEK_SET);
  written = (صحیح)fwrite(a_برآمد, 1, n_برآمد, برآمد);
  sqlite3_free(a_برآمد);
  fclose(برآمد);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(written));
  واپس TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ساکن صحیح SQLITE_TCLAPI hexio_get_int(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح val;
  Tcl_سائز n_اندر;
  صحیح n_برآمد;
  ثابت غیرعلامتی حرف *z_اندر;
  غیرعلامتی حرف *a_برآمد;
  غیرعلامتی حرف a_عدد[4];
  صحیح bLittle = 0;

  اگر( objc==3 ){
    Tcl_سائز n;
    حرف *z = Tcl_GetStringFromObj(objv[1], &n);
    اگر( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  اگر( (objc-bLittle)!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "[-littleendian] HEXDATA");
    واپس TCL_ERROR;
  }
  z_اندر = (ثابت غیرعلامتی حرف *)Tcl_GetStringFromObj(objv[1+bLittle], &n_اندر);
  a_برآمد = sqlite3_malloc64( 1 + n_اندر/2 );
  اگر( a_برآمد==0 ){
    واپس TCL_ERROR;
  }
  n_برآمد = sqlite3_Test_Hex_مقصد_Bin(z_اندر, (صحیح)n_اندر, a_برآمد);
  اگر( n_برآمد>=4 ){
    حافظہ_نقل(a_عدد, a_برآمد, 4);
  }ورنہ{
    memset(a_عدد, 0, حجم(a_عدد));
    حافظہ_نقل(&a_عدد[4-n_برآمد], a_برآمد, n_برآمد);
  }
  sqlite3_free(a_برآمد);
  اگر( bLittle ){
    val = (صحیح)((u32)a_عدد[3]<<24) | (a_عدد[2]<<16) | (a_عدد[1]<<8) | a_عدد[0];
  }ورنہ{
    val = (صحیح)((u32)a_عدد[0]<<24) | (a_عدد[1]<<16) | (a_عدد[2]<<8) | a_عدد[3];
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(val));
  واپس TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ساکن صحیح SQLITE_TCLAPI hexio_render_int16(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح val;
  غیرعلامتی حرف a_عدد[10];

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "INTEGER");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &val) ) واپس TCL_ERROR;
  a_عدد[0] = val>>8;
  a_عدد[1] = val;
  sqlite3_Test_Bin_مقصد_Hex(a_عدد, 2);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj((حرف*)a_عدد, 4));
  واپس TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ساکن صحیح SQLITE_TCLAPI hexio_render_int32(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح val;
  غیرعلامتی حرف a_عدد[10];

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "INTEGER");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &val) ) واپس TCL_ERROR;
  a_عدد[0] = val>>24;
  a_عدد[1] = val>>16;
  a_عدد[2] = val>>8;
  a_عدد[3] = val;
  sqlite3_Test_Bin_مقصد_Hex(a_عدد, 4);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj((حرف*)a_عدد, 8));
  واپس TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ساکن صحیح SQLITE_TCLAPI utf8_مقصد_utf8(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_تعریف_شدہ SQLITE_DEBUG
  Tcl_سائز n;
  صحیح n_برآمد;
  ثابت غیرعلامتی حرف *zOrig;
  غیرعلامتی حرف *z;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HEX");
    واپس TCL_ERROR;
  }
  zOrig = (غیرعلامتی حرف *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_مقصد_Bin(zOrig, (صحیح)n, z);
  z[n] = 0;
  n_برآمد = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_مقصد_Hex(z,n_برآمد);
  Tcl_Append_نتیجہ(interp, (حرف*)z, تہی);
  sqlite3_free(z);
  واپس TCL_OK;
#ورنہ
  Tcl_Append_نتیجہ(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", تہی
  );
  واپس TCL_ERROR;
#اگر_ختم
}

ساکن صحیح getFts3Varint(ثابت حرف *p, sqlite_int64 *v){
  ثابت غیرعلامتی حرف *q = (ثابت غیرعلامتی حرف *) p;
  sqlite_uint64 x = 0, y = 1;
  جب( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  واپس (صحیح) (q - (غیرعلامتی حرف *)p);
}

ساکن صحیح putFts3Varint(حرف *p, sqlite_int64 v){
  غیرعلامتی حرف *q = (غیرعلامتی حرف *) p;
  sqlite_uint64 vu = v;
  کرو{
    *q++ = (غیرعلامتی حرف) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }جب( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  أكد( q - (غیرعلامتی حرف *)p <= 10 );
  واپس (صحیح) (q - (غیرعلامتی حرف *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ساکن صحیح SQLITE_TCLAPI read_fts3varint(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_سائز nBlob;
  غیرعلامتی حرف *zBlob;
  sqlite3_int64 iVal;
  صحیح nVal;

  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BLOB VARNAME");
    واپس TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((حرف*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_سیٹ_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(nVal));
  واپس TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ساکن صحیح SQLITE_TCLAPI make_fts3record(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_سائز nArg = 0;
  غیرعلامتی حرف *a_برآمد = 0;
  sqlite3_int64 n_برآمد = 0;
  sqlite3_int64 nAlloc = 0;
  صحیح i;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "LIST");
    واپس TCL_ERROR;
  }
  اگر( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    واپس TCL_ERROR;
  }

  برائے(i=0; i<(صحیح)nArg; i++){
    Tcl_WideInt iVal;
    اگر( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      اگر( n_برآمد+10>nAlloc ){
        صحیح nNew = nAlloc?nAlloc*2:128;
        غیرعلامتی حرف *aNew = sqlite3_realloc(a_برآمد, nNew);
        اگر( aNew==0 ){
          sqlite3_free(a_برآمد);
          واپس TCL_ERROR;
        }
        a_برآمد = aNew;
        nAlloc = nNew;
      }
      n_برآمد += putFts3Varint((حرف*)&a_برآمد[n_برآمد], iVal);
    }ورنہ{
      Tcl_سائز nVal = 0;
      حرف *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      جب( (n_برآمد + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        غیرعلامتی حرف *aNew = sqlite3_realloc64(a_برآمد, nNew);
        اگر( aNew==0 ){
          sqlite3_free(a_برآمد);
          واپس TCL_ERROR;
        }
        a_برآمد = aNew;
        nAlloc = nNew;
      }
      حافظہ_نقل(&a_برآمد[n_برآمد], zVal, nVal);
      n_برآمد += nVal;
    }
  }

  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewByteArrayObj(a_برآمد, n_برآمد));
  sqlite3_free(a_برآمد);
  واپس TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "hexio_read",                   hexio_read            },
     { "hexio_write",                  hexio_write           },
     { "hexio_get_int",                hexio_get_int         },
     { "hexio_render_int16",           hexio_render_int16    },
     { "hexio_render_int32",           hexio_render_int32    },
     { "utf8_to_utf8",                 utf8_مقصد_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  واپس TCL_OK;
}
