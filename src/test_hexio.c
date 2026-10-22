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
تهی sqlite3TestBinToHex(unsigned نویسه *zBuf, صحیح N){
  ثابت unsigned نویسه zHex[] = "0123456789ABCDEF";
  صحیح i, j;
  unsigned نویسه c;
  i = N*2;
  zBuf[i--] = 0;
  برای(j=N-1; j>=0; j--){
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
صحیح sqlite3TestHexToBin(ثابت unsigned نویسه *zIn, صحیح N, unsigned نویسه *aOut){
  ثابت unsigned نویسه aMap[] = {
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
  unsigned نویسه c;

  برای(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    اگر( c==0 ) ادامه;
    اگر( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }وگرنه{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  بازگشت j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ایستا صحیح SQLITE_TCLAPI hexio_read(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح offset;
  صحیح amt, got;
  ثابت نویسه *zFile;
  unsigned نویسه *zBuf;
  FILE *in;

  اگر( objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &offset) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[3], &amt) ) بازگشت TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  اگر( zBuf==0 ){
    بازگشت TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  اگر( in==0 ){
    in = fopen(zFile, "r");
  }
  اگر( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, پوچ);
    بازگشت TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (صحیح)fread(zBuf, 1, amt, in);
  fclose(in);
  اگر( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, پوچ);
  sqlite3_free(zBuf);
  بازگشت TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ایستا صحیح SQLITE_TCLAPI hexio_write(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح offset;
  Tcl_Size nIn;
  صحیح nOut, written;
  ثابت نویسه *zFile;
  ثابت unsigned نویسه *zIn;
  unsigned نویسه *aOut;
  FILE *out;

  اگر( objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &offset) ) بازگشت TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (ثابت unsigned نویسه *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  اگر( aOut==0 ){
    بازگشت TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (صحیح)nIn, aOut);
  out = fopen(zFile, "r+b");
  اگر( out==0 ){
    out = fopen(zFile, "r+");
  }
  اگر( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, پوچ);
    بازگشت TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (صحیح)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  بازگشت TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ایستا صحیح SQLITE_TCLAPI hexio_get_int(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح val;
  Tcl_Size nIn;
  صحیح nOut;
  ثابت unsigned نویسه *zIn;
  unsigned نویسه *aOut;
  unsigned نویسه a_رقم[4];
  صحیح bLittle = 0;

  اگر( objc==3 ){
    Tcl_Size n;
    نویسه *z = Tcl_GetStringFromObj(objv[1], &n);
    اگر( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  اگر( (objc-bLittle)!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    بازگشت TCL_ERROR;
  }
  zIn = (ثابت unsigned نویسه *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  اگر( aOut==0 ){
    بازگشت TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (صحیح)nIn, aOut);
  اگر( nOut>=4 ){
    memcpy(a_رقم, aOut, 4);
  }وگرنه{
    memset(a_رقم, 0, اندازه(a_رقم));
    memcpy(&a_رقم[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  اگر( bLittle ){
    val = (صحیح)((u32)a_رقم[3]<<24) | (a_رقم[2]<<16) | (a_رقم[1]<<8) | a_رقم[0];
  }وگرنه{
    val = (صحیح)((u32)a_رقم[0]<<24) | (a_رقم[1]<<16) | (a_رقم[2]<<8) | a_رقم[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  بازگشت TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ایستا صحیح SQLITE_TCLAPI hexio_render_int16(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح val;
  unsigned نویسه a_رقم[10];

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "INTEGER");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &val) ) بازگشت TCL_ERROR;
  a_رقم[0] = val>>8;
  a_رقم[1] = val;
  sqlite3TestBinToHex(a_رقم, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((نویسه*)a_رقم, 4));
  بازگشت TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ایستا صحیح SQLITE_TCLAPI hexio_render_int32(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح val;
  unsigned نویسه a_رقم[10];

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "INTEGER");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &val) ) بازگشت TCL_ERROR;
  a_رقم[0] = val>>24;
  a_رقم[1] = val>>16;
  a_رقم[2] = val>>8;
  a_رقم[3] = val;
  sqlite3TestBinToHex(a_رقم, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((نویسه*)a_رقم, 8));
  بازگشت TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ایستا صحیح SQLITE_TCLAPI utf8_to_utf8(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  صحیح nOut;
  ثابت unsigned نویسه *zOrig;
  unsigned نویسه *z;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "HEX");
    بازگشت TCL_ERROR;
  }
  zOrig = (unsigned نویسه *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (صحیح)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (نویسه*)z, پوچ);
  sqlite3_free(z);
  بازگشت TCL_OK;
#وگرنه
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", پوچ
  );
  بازگشت TCL_ERROR;
#endif
}

ایستا صحیح getFts3Varint(ثابت نویسه *p, sqlite_int64 *v){
  ثابت unsigned نویسه *q = (ثابت unsigned نویسه *) p;
  sqlite_uint64 x = 0, y = 1;
  وقتی( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  بازگشت (صحیح) (q - (unsigned نویسه *)p);
}

ایستا صحیح putFts3Varint(نویسه *p, sqlite_int64 v){
  unsigned نویسه *q = (unsigned نویسه *) p;
  sqlite_uint64 vu = v;
  انجامبده{
    *q++ = (unsigned نویسه) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }وقتی( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  أكد( q - (unsigned نویسه *)p <= 10 );
  بازگشت (صحیح) (q - (unsigned نویسه *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ایستا صحیح SQLITE_TCLAPI read_fts3varint(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned نویسه *zBlob;
  sqlite3_int64 iVal;
  صحیح nVal;

  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BLOB VARNAME");
    بازگشت TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((نویسه*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  بازگشت TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ایستا صحیح SQLITE_TCLAPI make_fts3record(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned نویسه *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  صحیح i;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "LIST");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    بازگشت TCL_ERROR;
  }

  برای(i=0; i<(صحیح)nArg; i++){
    Tcl_WideInt iVal;
    اگر( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      اگر( nOut+10>nAlloc ){
        صحیح nNew = nAlloc?nAlloc*2:128;
        unsigned نویسه *aNew = sqlite3_realloc(aOut, nNew);
        اگر( aNew==0 ){
          sqlite3_free(aOut);
          بازگشت TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((نویسه*)&aOut[nOut], iVal);
    }وگرنه{
      Tcl_Size nVal = 0;
      نویسه *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      وقتی( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned نویسه *aNew = sqlite3_realloc64(aOut, nNew);
        اگر( aNew==0 ){
          sqlite3_free(aOut);
          بازگشت TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      memcpy(&aOut[nOut], zVal, nVal);
      nOut += nVal;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(aOut, nOut));
  sqlite3_free(aOut);
  بازگشت TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "hexio_read",                   hexio_read            },
     { "hexio_write",                  hexio_write           },
     { "hexio_get_int",                hexio_get_int         },
     { "hexio_render_int16",           hexio_render_int16    },
     { "hexio_render_int32",           hexio_render_int32    },
     { "utf8_to_utf8",                 utf8_to_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  بازگشت TCL_OK;
}
