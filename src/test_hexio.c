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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
فراغ sqlite3_Test_Bin_مقصد_Hex(بیعلامت حرف *zBuf, صحيح N){
  ثابت بیعلامت حرف zHex[] = "0123456789ABCDEF";
  صحيح i, j;
  بیعلامت حرف c;
  i = N*2;
  zBuf[i--] = 0;
  لكل(j=N-1; j>=0; j--){
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
صحيح sqlite3_Test_Hex_مقصد_Bin(ثابت بیعلامت حرف *z_درون, صحيح N, بیعلامت حرف *a_بیرون){
  ثابت بیعلامت حرف aMap[] = {
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
  صحيح i, j;
  صحيح hi=1;
  بیعلامت حرف c;

  لكل(i=j=0; i<N; i++){
    c = aMap[z_درون[i]];
    إذا( c==0 ) استمر;
    إذا( hi ){
      a_بیرون[j] = (c-1)<<4;
      hi = 0;
    }وإلا{
      a_بیرون[j++] |= c-1;
      hi = 1;
    }
  }
  أرجع j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ساكن صحيح SQLITE_TCLAPI hexio_read(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح جابه_جایی;
  صحيح amt, got;
  ثابت حرف *zFile;
  بیعلامت حرف *zBuf;
  FILE *درون;

  إذا( objc!=4 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "FILENAME OFFSET AMT");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &جابه_جایی) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[3], &amt) ) أرجع TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  إذا( zBuf==0 ){
    أرجع TCL_ERROR;
  }
  درون = fopen(zFile, "rb");
  إذا( درون==0 ){
    درون = fopen(zFile, "r");
  }
  إذا( درون==0 ){
    Tcl_Append_نتیجه(interp, "cannot open input file ", zFile, عدم);
    أرجع TCL_ERROR;
  }
  fseek(درون, جابه_جایی, SEEK_SET);
  got = (صحيح)fread(zBuf, 1, amt, درون);
  fclose(درون);
  إذا( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_مقصد_Hex(zBuf, got);
  Tcl_Append_نتیجه(interp, zBuf, عدم);
  sqlite3_free(zBuf);
  أرجع TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ساكن صحيح SQLITE_TCLAPI hexio_write(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح جابه_جایی;
  Tcl_اندازه n_درون;
  صحيح n_بیرون, written;
  ثابت حرف *zFile;
  ثابت بیعلامت حرف *z_درون;
  بیعلامت حرف *a_بیرون;
  FILE *بیرون;

  إذا( objc!=4 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &جابه_جایی) ) أرجع TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_درون = (ثابت بیعلامت حرف *)Tcl_GetStringFromObj(objv[3], &n_درون);
  a_بیرون = sqlite3_malloc64( 1 + n_درون/2 );
  إذا( a_بیرون==0 ){
    أرجع TCL_ERROR;
  }
  n_بیرون = sqlite3_Test_Hex_مقصد_Bin(z_درون, (صحيح)n_درون, a_بیرون);
  بیرون = fopen(zFile, "r+b");
  إذا( بیرون==0 ){
    بیرون = fopen(zFile, "r+");
  }
  إذا( بیرون==0 ){
    Tcl_Append_نتیجه(interp, "cannot open output file ", zFile, عدم);
    أرجع TCL_ERROR;
  }
  fseek(بیرون, جابه_جایی, SEEK_SET);
  written = (صحيح)fwrite(a_بیرون, 1, n_بیرون, بیرون);
  sqlite3_free(a_بیرون);
  fclose(بیرون);
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewIntObj(written));
  أرجع TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ساكن صحيح SQLITE_TCLAPI hexio_get_int(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح val;
  Tcl_اندازه n_درون;
  صحيح n_بیرون;
  ثابت بیعلامت حرف *z_درون;
  بیعلامت حرف *a_بیرون;
  بیعلامت حرف a_رقم[4];
  صحيح bLittle = 0;

  إذا( objc==3 ){
    Tcl_اندازه n;
    حرف *z = Tcl_GetStringFromObj(objv[1], &n);
    إذا( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  إذا( (objc-bLittle)!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "[-littleendian] HEXDATA");
    أرجع TCL_ERROR;
  }
  z_درون = (ثابت بیعلامت حرف *)Tcl_GetStringFromObj(objv[1+bLittle], &n_درون);
  a_بیرون = sqlite3_malloc64( 1 + n_درون/2 );
  إذا( a_بیرون==0 ){
    أرجع TCL_ERROR;
  }
  n_بیرون = sqlite3_Test_Hex_مقصد_Bin(z_درون, (صحيح)n_درون, a_بیرون);
  إذا( n_بیرون>=4 ){
    کپی_حافظه(a_رقم, a_بیرون, 4);
  }وإلا{
    memset(a_رقم, 0, حجم(a_رقم));
    کپی_حافظه(&a_رقم[4-n_بیرون], a_بیرون, n_بیرون);
  }
  sqlite3_free(a_بیرون);
  إذا( bLittle ){
    val = (صحيح)((u32)a_رقم[3]<<24) | (a_رقم[2]<<16) | (a_رقم[1]<<8) | a_رقم[0];
  }وإلا{
    val = (صحيح)((u32)a_رقم[0]<<24) | (a_رقم[1]<<16) | (a_رقم[2]<<8) | a_رقم[3];
  }
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewIntObj(val));
  أرجع TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ساكن صحيح SQLITE_TCLAPI hexio_render_int16(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح val;
  بیعلامت حرف a_رقم[10];

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "INTEGER");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &val) ) أرجع TCL_ERROR;
  a_رقم[0] = val>>8;
  a_رقم[1] = val;
  sqlite3_Test_Bin_مقصد_Hex(a_رقم, 2);
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj((حرف*)a_رقم, 4));
  أرجع TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ساكن صحيح SQLITE_TCLAPI hexio_render_int32(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح val;
  بیعلامت حرف a_رقم[10];

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "INTEGER");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &val) ) أرجع TCL_ERROR;
  a_رقم[0] = val>>24;
  a_رقم[1] = val>>16;
  a_رقم[2] = val>>8;
  a_رقم[3] = val;
  sqlite3_Test_Bin_مقصد_Hex(a_رقم, 4);
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj((حرف*)a_رقم, 8));
  أرجع TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ساكن صحيح SQLITE_TCLAPI utf8_مقصد_utf8(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_تعریف_شده SQLITE_DEBUG
  Tcl_اندازه n;
  صحيح n_بیرون;
  ثابت بیعلامت حرف *zOrig;
  بیعلامت حرف *z;
  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "HEX");
    أرجع TCL_ERROR;
  }
  zOrig = (بیعلامت حرف *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_مقصد_Bin(zOrig, (صحيح)n, z);
  z[n] = 0;
  n_بیرون = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_مقصد_Hex(z,n_بیرون);
  Tcl_Append_نتیجه(interp, (حرف*)z, عدم);
  sqlite3_free(z);
  أرجع TCL_OK;
#وإلا
  Tcl_Append_نتیجه(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", عدم
  );
  أرجع TCL_ERROR;
#پایان_اگر
}

ساكن صحيح getFts3Varint(ثابت حرف *p, sqlite_int64 *v){
  ثابت بیعلامت حرف *q = (ثابت بیعلامت حرف *) p;
  sqlite_uint64 x = 0, y = 1;
  طالما( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  أرجع (صحيح) (q - (بیعلامت حرف *)p);
}

ساكن صحيح putFts3Varint(حرف *p, sqlite_int64 v){
  بیعلامت حرف *q = (بیعلامت حرف *) p;
  sqlite_uint64 vu = v;
  افعل{
    *q++ = (بیعلامت حرف) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }طالما( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  أكد( q - (بیعلامت حرف *)p <= 10 );
  أرجع (صحيح) (q - (بیعلامت حرف *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ساكن صحيح SQLITE_TCLAPI read_fts3varint(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_اندازه nBlob;
  بیعلامت حرف *zBlob;
  sqlite3_int64 iVal;
  صحيح nVal;

  إذا( objc!=3 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "BLOB VARNAME");
    أرجع TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((حرف*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_مجموعه_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewIntObj(nVal));
  أرجع TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ساكن صحيح SQLITE_TCLAPI make_fts3record(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_اندازه nArg = 0;
  بیعلامت حرف *a_بیرون = 0;
  sqlite3_int64 n_بیرون = 0;
  sqlite3_int64 nAlloc = 0;
  صحيح i;

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "LIST");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    أرجع TCL_ERROR;
  }

  لكل(i=0; i<(صحيح)nArg; i++){
    Tcl_WideInt iVal;
    إذا( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      إذا( n_بیرون+10>nAlloc ){
        صحيح nNew = nAlloc?nAlloc*2:128;
        بیعلامت حرف *aNew = sqlite3_realloc(a_بیرون, nNew);
        إذا( aNew==0 ){
          sqlite3_free(a_بیرون);
          أرجع TCL_ERROR;
        }
        a_بیرون = aNew;
        nAlloc = nNew;
      }
      n_بیرون += putFts3Varint((حرف*)&a_بیرون[n_بیرون], iVal);
    }وإلا{
      Tcl_اندازه nVal = 0;
      حرف *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      طالما( (n_بیرون + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        بیعلامت حرف *aNew = sqlite3_realloc64(a_بیرون, nNew);
        إذا( aNew==0 ){
          sqlite3_free(a_بیرون);
          أرجع TCL_ERROR;
        }
        a_بیرون = aNew;
        nAlloc = nNew;
      }
      کپی_حافظه(&a_بیرون[n_بیرون], zVal, nVal);
      n_بیرون += nVal;
    }
  }

  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewByteArrayObj(a_بیرون, n_بیرون));
  sqlite3_free(a_بیرون);
  أرجع TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ساكن بنية {
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
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  أرجع TCL_OK;
}
