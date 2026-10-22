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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
void sqlite3TestBinToHex(unsigned char *zBuf, số_nguyên N){
  const unsigned char zHex[] = "0123456789ABCDEF";
  số_nguyên i, j;
  unsigned char c;
  i = N*2;
  zBuf[i--] = 0;
  cho(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  assert( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
số_nguyên sqlite3TestHexToBin(const unsigned char *zIn, số_nguyên N, unsigned char *aOut){
  const unsigned char aMap[] = {
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
  số_nguyên i, j;
  số_nguyên hi=1;
  unsigned char c;

  cho(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    nếu( c==0 ) continue;
    nếu( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }không_thì{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  trả_về j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
static số_nguyên SQLITE_TCLAPI hexio_read(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên offset;
  số_nguyên amt, got;
  const char *zFile;
  unsigned char *zBuf;
  FILE *in;

  nếu( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET AMT");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[2], &offset) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[3], &amt) ) trả_về TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  nếu( zBuf==0 ){
    trả_về TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  nếu( in==0 ){
    in = fopen(zFile, "r");
  }
  nếu( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, NULL);
    trả_về TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (số_nguyên)fread(zBuf, 1, amt, in);
  fclose(in);
  nếu( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, NULL);
  sqlite3_free(zBuf);
  trả_về TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
static số_nguyên SQLITE_TCLAPI hexio_write(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên offset;
  Tcl_Size nIn;
  số_nguyên nOut, written;
  const char *zFile;
  const unsigned char *zIn;
  unsigned char *aOut;
  FILE *out;

  nếu( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[2], &offset) ) trả_về TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (const unsigned char *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  nếu( aOut==0 ){
    trả_về TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (số_nguyên)nIn, aOut);
  out = fopen(zFile, "r+b");
  nếu( out==0 ){
    out = fopen(zFile, "r+");
  }
  nếu( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, NULL);
    trả_về TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (số_nguyên)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  trả_về TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
static số_nguyên SQLITE_TCLAPI hexio_get_int(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên val;
  Tcl_Size nIn;
  số_nguyên nOut;
  const unsigned char *zIn;
  unsigned char *aOut;
  unsigned char aNum[4];
  số_nguyên bLittle = 0;

  nếu( objc==3 ){
    Tcl_Size n;
    char *z = Tcl_GetStringFromObj(objv[1], &n);
    nếu( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  nếu( (objc-bLittle)!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[-littleendian] HEXDATA");
    trả_về TCL_ERROR;
  }
  zIn = (const unsigned char *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  nếu( aOut==0 ){
    trả_về TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (số_nguyên)nIn, aOut);
  nếu( nOut>=4 ){
    memcpy(aNum, aOut, 4);
  }không_thì{
    memset(aNum, 0, sizeof(aNum));
    memcpy(&aNum[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  nếu( bLittle ){
    val = (số_nguyên)((u32)aNum[3]<<24) | (aNum[2]<<16) | (aNum[1]<<8) | aNum[0];
  }không_thì{
    val = (số_nguyên)((u32)aNum[0]<<24) | (aNum[1]<<16) | (aNum[2]<<8) | aNum[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  trả_về TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
static số_nguyên SQLITE_TCLAPI hexio_render_int16(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên val;
  unsigned char aNum[10];

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &val) ) trả_về TCL_ERROR;
  aNum[0] = val>>8;
  aNum[1] = val;
  sqlite3TestBinToHex(aNum, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((char*)aNum, 4));
  trả_về TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
static số_nguyên SQLITE_TCLAPI hexio_render_int32(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên val;
  unsigned char aNum[10];

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &val) ) trả_về TCL_ERROR;
  aNum[0] = val>>24;
  aNum[1] = val>>16;
  aNum[2] = val>>8;
  aNum[3] = val;
  sqlite3TestBinToHex(aNum, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((char*)aNum, 8));
  trả_về TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
static số_nguyên SQLITE_TCLAPI utf8_to_utf8(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  số_nguyên nOut;
  const unsigned char *zOrig;
  unsigned char *z;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HEX");
    trả_về TCL_ERROR;
  }
  zOrig = (unsigned char *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (số_nguyên)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (char*)z, NULL);
  sqlite3_free(z);
  trả_về TCL_OK;
#không_thì
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", NULL
  );
  trả_về TCL_ERROR;
#endif
}

static số_nguyên getFts3Varint(const char *p, sqlite_int64 *v){
  const unsigned char *q = (const unsigned char *) p;
  sqlite_uint64 x = 0, y = 1;
  trong_khi( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  trả_về (số_nguyên) (q - (unsigned char *)p);
}

static số_nguyên putFts3Varint(char *p, sqlite_int64 v){
  unsigned char *q = (unsigned char *) p;
  sqlite_uint64 vu = v;
  do{
    *q++ = (unsigned char) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }trong_khi( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  assert( q - (unsigned char *)p <= 10 );
  trả_về (số_nguyên) (q - (unsigned char *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
static số_nguyên SQLITE_TCLAPI read_fts3varint(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned char *zBlob;
  sqlite3_int64 iVal;
  số_nguyên nVal;

  nếu( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BLOB VARNAME");
    trả_về TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((char*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  trả_về TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
static số_nguyên SQLITE_TCLAPI make_fts3record(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned char *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  số_nguyên i;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "LIST");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    trả_về TCL_ERROR;
  }

  cho(i=0; i<(số_nguyên)nArg; i++){
    Tcl_WideInt iVal;
    nếu( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      nếu( nOut+10>nAlloc ){
        số_nguyên nNew = nAlloc?nAlloc*2:128;
        unsigned char *aNew = sqlite3_realloc(aOut, nNew);
        nếu( aNew==0 ){
          sqlite3_free(aOut);
          trả_về TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((char*)&aOut[nOut], iVal);
    }không_thì{
      Tcl_Size nVal = 0;
      char *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      trong_khi( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned char *aNew = sqlite3_realloc64(aOut, nNew);
        nếu( aNew==0 ){
          sqlite3_free(aOut);
          trả_về TCL_ERROR;
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
  trả_về TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
số_nguyên Sqlitetest_hexio_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
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
  số_nguyên i;
  cho(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  trả_về TCL_OK;
}
