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
فراغ sqlite3_Test_Bin_منزل_Hex(بینښه حرف *zBuf, صحيح N){
  ثابت بینښه حرف zHex[] = "0123456789ABCDEF";
  صحيح i, j;
  بینښه حرف c;
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
صحيح sqlite3_Test_Hex_منزل_Bin(ثابت بینښه حرف *z_دننه, صحيح N, بینښه حرف *a_بهر){
  ثابت بینښه حرف aMap[] = {
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
  بینښه حرف c;

  لكل(i=j=0; i<N; i++){
    c = aMap[z_دننه[i]];
    إذا( c==0 ) استمر;
    إذا( hi ){
      a_بهر[j] = (c-1)<<4;
      hi = 0;
    }وإلا{
      a_بهر[j++] |= c-1;
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
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح دننه_واټن;
  صحيح amt, got;
  ثابت حرف *zFile;
  بینښه حرف *zBuf;
  FILE *دننه;

  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME OFFSET AMT");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &دننه_واټن) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[3], &amt) ) أرجع TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  إذا( zBuf==0 ){
    أرجع TCL_ERROR;
  }
  دننه = fopen(zFile, "rb");
  إذا( دننه==0 ){
    دننه = fopen(zFile, "r");
  }
  إذا( دننه==0 ){
    Tcl_Append_پایله(interp, "cannot open input file ", zFile, عدم);
    أرجع TCL_ERROR;
  }
  fseek(دننه, دننه_واټن, SEEK_SET);
  got = (صحيح)fread(zBuf, 1, amt, دننه);
  fclose(دننه);
  إذا( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_منزل_Hex(zBuf, got);
  Tcl_Append_پایله(interp, zBuf, عدم);
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
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح دننه_واټن;
  Tcl_اندازه n_دننه;
  صحيح n_بهر, written;
  ثابت حرف *zFile;
  ثابت بینښه حرف *z_دننه;
  بینښه حرف *a_بهر;
  FILE *بهر;

  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &دننه_واټن) ) أرجع TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_دننه = (ثابت بینښه حرف *)Tcl_GetStringFromObj(objv[3], &n_دننه);
  a_بهر = sqlite3_malloc64( 1 + n_دننه/2 );
  إذا( a_بهر==0 ){
    أرجع TCL_ERROR;
  }
  n_بهر = sqlite3_Test_Hex_منزل_Bin(z_دننه, (صحيح)n_دننه, a_بهر);
  بهر = fopen(zFile, "r+b");
  إذا( بهر==0 ){
    بهر = fopen(zFile, "r+");
  }
  إذا( بهر==0 ){
    Tcl_Append_پایله(interp, "cannot open output file ", zFile, عدم);
    أرجع TCL_ERROR;
  }
  fseek(بهر, دننه_واټن, SEEK_SET);
  written = (صحيح)fwrite(a_بهر, 1, n_بهر, بهر);
  sqlite3_free(a_بهر);
  fclose(بهر);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(written));
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
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح val;
  Tcl_اندازه n_دننه;
  صحيح n_بهر;
  ثابت بینښه حرف *z_دننه;
  بینښه حرف *a_بهر;
  بینښه حرف a_عدد[4];
  صحيح bLittle = 0;

  إذا( objc==3 ){
    Tcl_اندازه n;
    حرف *z = Tcl_GetStringFromObj(objv[1], &n);
    إذا( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  إذا( (objc-bLittle)!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "[-littleendian] HEXDATA");
    أرجع TCL_ERROR;
  }
  z_دننه = (ثابت بینښه حرف *)Tcl_GetStringFromObj(objv[1+bLittle], &n_دننه);
  a_بهر = sqlite3_malloc64( 1 + n_دننه/2 );
  إذا( a_بهر==0 ){
    أرجع TCL_ERROR;
  }
  n_بهر = sqlite3_Test_Hex_منزل_Bin(z_دننه, (صحيح)n_دننه, a_بهر);
  إذا( n_بهر>=4 ){
    حافظه_نقل(a_عدد, a_بهر, 4);
  }وإلا{
    memset(a_عدد, 0, حجم(a_عدد));
    حافظه_نقل(&a_عدد[4-n_بهر], a_بهر, n_بهر);
  }
  sqlite3_free(a_بهر);
  إذا( bLittle ){
    val = (صحيح)((u32)a_عدد[3]<<24) | (a_عدد[2]<<16) | (a_عدد[1]<<8) | a_عدد[0];
  }وإلا{
    val = (صحيح)((u32)a_عدد[0]<<24) | (a_عدد[1]<<16) | (a_عدد[2]<<8) | a_عدد[3];
  }
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(val));
  أرجع TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ساكن صحيح SQLITE_TCLAPI hexio_render_int16(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح val;
  بینښه حرف a_عدد[10];

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "INTEGER");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &val) ) أرجع TCL_ERROR;
  a_عدد[0] = val>>8;
  a_عدد[1] = val;
  sqlite3_Test_Bin_منزل_Hex(a_عدد, 2);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj((حرف*)a_عدد, 4));
  أرجع TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ساكن صحيح SQLITE_TCLAPI hexio_render_int32(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح val;
  بینښه حرف a_عدد[10];

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "INTEGER");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &val) ) أرجع TCL_ERROR;
  a_عدد[0] = val>>24;
  a_عدد[1] = val>>16;
  a_عدد[2] = val>>8;
  a_عدد[3] = val;
  sqlite3_Test_Bin_منزل_Hex(a_عدد, 4);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj((حرف*)a_عدد, 8));
  أرجع TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ساكن صحيح SQLITE_TCLAPI utf8_منزل_utf8(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
#که_تعریف_شوی SQLITE_DEBUG
  Tcl_اندازه n;
  صحيح n_بهر;
  ثابت بینښه حرف *zOrig;
  بینښه حرف *z;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HEX");
    أرجع TCL_ERROR;
  }
  zOrig = (بینښه حرف *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_منزل_Bin(zOrig, (صحيح)n, z);
  z[n] = 0;
  n_بهر = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_منزل_Hex(z,n_بهر);
  Tcl_Append_پایله(interp, (حرف*)z, عدم);
  sqlite3_free(z);
  أرجع TCL_OK;
#وإلا
  Tcl_Append_پایله(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", عدم
  );
  أرجع TCL_ERROR;
#د_که_پای
}

ساكن صحيح getFts3Varint(ثابت حرف *p, sqlite_int64 *v){
  ثابت بینښه حرف *q = (ثابت بینښه حرف *) p;
  sqlite_uint64 x = 0, y = 1;
  طالما( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  أرجع (صحيح) (q - (بینښه حرف *)p);
}

ساكن صحيح putFts3Varint(حرف *p, sqlite_int64 v){
  بینښه حرف *q = (بینښه حرف *) p;
  sqlite_uint64 vu = v;
  افعل{
    *q++ = (بینښه حرف) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }طالما( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  أكد( q - (بینښه حرف *)p <= 10 );
  أرجع (صحيح) (q - (بینښه حرف *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ساكن صحيح SQLITE_TCLAPI read_fts3varint(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_اندازه nBlob;
  بینښه حرف *zBlob;
  sqlite3_int64 iVal;
  صحيح nVal;

  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BLOB VARNAME");
    أرجع TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((حرف*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_سېټ_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(nVal));
  أرجع TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ساكن صحيح SQLITE_TCLAPI make_fts3record(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_اندازه nArg = 0;
  بینښه حرف *a_بهر = 0;
  sqlite3_int64 n_بهر = 0;
  sqlite3_int64 nAlloc = 0;
  صحيح i;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "LIST");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    أرجع TCL_ERROR;
  }

  لكل(i=0; i<(صحيح)nArg; i++){
    Tcl_WideInt iVal;
    إذا( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      إذا( n_بهر+10>nAlloc ){
        صحيح nNew = nAlloc?nAlloc*2:128;
        بینښه حرف *aNew = sqlite3_realloc(a_بهر, nNew);
        إذا( aNew==0 ){
          sqlite3_free(a_بهر);
          أرجع TCL_ERROR;
        }
        a_بهر = aNew;
        nAlloc = nNew;
      }
      n_بهر += putFts3Varint((حرف*)&a_بهر[n_بهر], iVal);
    }وإلا{
      Tcl_اندازه nVal = 0;
      حرف *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      طالما( (n_بهر + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        بینښه حرف *aNew = sqlite3_realloc64(a_بهر, nNew);
        إذا( aNew==0 ){
          sqlite3_free(a_بهر);
          أرجع TCL_ERROR;
        }
        a_بهر = aNew;
        nAlloc = nNew;
      }
      حافظه_نقل(&a_بهر[n_بهر], zVal, nVal);
      n_بهر += nVal;
    }
  }

  Tcl_سېټ_Obj_پایله(interp, Tcl_NewByteArrayObj(a_بهر, n_بهر));
  sqlite3_free(a_بهر);
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
     { "utf8_to_utf8",                 utf8_منزل_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  أرجع TCL_OK;
}
