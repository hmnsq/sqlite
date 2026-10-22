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
void sqlite3TestBinToHex(unsigned char *zBuf, цяло N){
  const unsigned char zHex[] = "0123456789ABCDEF";
  цяло i, j;
  unsigned char c;
  i = N*2;
  zBuf[i--] = 0;
  за(j=N-1; j>=0; j--){
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
цяло sqlite3TestHexToBin(const unsigned char *zIn, цяло N, unsigned char *aOut){
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
  цяло i, j;
  цяло hi=1;
  unsigned char c;

  за(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    ако( c==0 ) continue;
    ако( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }иначе{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  върни j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
static цяло SQLITE_TCLAPI hexio_read(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло offset;
  цяло amt, got;
  const char *zFile;
  unsigned char *zBuf;
  FILE *in;

  ако( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET AMT");
    върни TCL_ERROR;
  }
  ако( Tcl_GetIntFromObj(interp, objv[2], &offset) ) върни TCL_ERROR;
  ако( Tcl_GetIntFromObj(interp, objv[3], &amt) ) върни TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ако( zBuf==0 ){
    върни TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  ако( in==0 ){
    in = fopen(zFile, "r");
  }
  ако( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, NULL);
    върни TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (цяло)fread(zBuf, 1, amt, in);
  fclose(in);
  ако( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, NULL);
  sqlite3_free(zBuf);
  върни TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
static цяло SQLITE_TCLAPI hexio_write(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло offset;
  Tcl_Size nIn;
  цяло nOut, written;
  const char *zFile;
  const unsigned char *zIn;
  unsigned char *aOut;
  FILE *out;

  ако( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    върни TCL_ERROR;
  }
  ако( Tcl_GetIntFromObj(interp, objv[2], &offset) ) върни TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (const unsigned char *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ако( aOut==0 ){
    върни TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (цяло)nIn, aOut);
  out = fopen(zFile, "r+b");
  ако( out==0 ){
    out = fopen(zFile, "r+");
  }
  ако( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, NULL);
    върни TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (цяло)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  върни TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
static цяло SQLITE_TCLAPI hexio_get_int(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло val;
  Tcl_Size nIn;
  цяло nOut;
  const unsigned char *zIn;
  unsigned char *aOut;
  unsigned char aNum[4];
  цяло bLittle = 0;

  ако( objc==3 ){
    Tcl_Size n;
    char *z = Tcl_GetStringFromObj(objv[1], &n);
    ако( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ако( (objc-bLittle)!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[-littleendian] HEXDATA");
    върни TCL_ERROR;
  }
  zIn = (const unsigned char *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ако( aOut==0 ){
    върни TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (цяло)nIn, aOut);
  ако( nOut>=4 ){
    memcpy(aNum, aOut, 4);
  }иначе{
    memset(aNum, 0, sizeof(aNum));
    memcpy(&aNum[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  ако( bLittle ){
    val = (цяло)((u32)aNum[3]<<24) | (aNum[2]<<16) | (aNum[1]<<8) | aNum[0];
  }иначе{
    val = (цяло)((u32)aNum[0]<<24) | (aNum[1]<<16) | (aNum[2]<<8) | aNum[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  върни TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
static цяло SQLITE_TCLAPI hexio_render_int16(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло val;
  unsigned char aNum[10];

  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    върни TCL_ERROR;
  }
  ако( Tcl_GetIntFromObj(interp, objv[1], &val) ) върни TCL_ERROR;
  aNum[0] = val>>8;
  aNum[1] = val;
  sqlite3TestBinToHex(aNum, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((char*)aNum, 4));
  върни TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
static цяло SQLITE_TCLAPI hexio_render_int32(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло val;
  unsigned char aNum[10];

  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    върни TCL_ERROR;
  }
  ако( Tcl_GetIntFromObj(interp, objv[1], &val) ) върни TCL_ERROR;
  aNum[0] = val>>24;
  aNum[1] = val>>16;
  aNum[2] = val>>8;
  aNum[3] = val;
  sqlite3TestBinToHex(aNum, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((char*)aNum, 8));
  върни TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
static цяло SQLITE_TCLAPI utf8_to_utf8(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  цяло nOut;
  const unsigned char *zOrig;
  unsigned char *z;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HEX");
    върни TCL_ERROR;
  }
  zOrig = (unsigned char *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (цяло)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (char*)z, NULL);
  sqlite3_free(z);
  върни TCL_OK;
#иначе
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", NULL
  );
  върни TCL_ERROR;
#endif
}

static цяло getFts3Varint(const char *p, sqlite_int64 *v){
  const unsigned char *q = (const unsigned char *) p;
  sqlite_uint64 x = 0, y = 1;
  докато( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  върни (цяло) (q - (unsigned char *)p);
}

static цяло putFts3Varint(char *p, sqlite_int64 v){
  unsigned char *q = (unsigned char *) p;
  sqlite_uint64 vu = v;
  do{
    *q++ = (unsigned char) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }докато( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  assert( q - (unsigned char *)p <= 10 );
  върни (цяло) (q - (unsigned char *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
static цяло SQLITE_TCLAPI read_fts3varint(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned char *zBlob;
  sqlite3_int64 iVal;
  цяло nVal;

  ако( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BLOB VARNAME");
    върни TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((char*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  върни TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
static цяло SQLITE_TCLAPI make_fts3record(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned char *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  цяло i;

  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "LIST");
    върни TCL_ERROR;
  }
  ако( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    върни TCL_ERROR;
  }

  за(i=0; i<(цяло)nArg; i++){
    Tcl_WideInt iVal;
    ако( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ако( nOut+10>nAlloc ){
        цяло nNew = nAlloc?nAlloc*2:128;
        unsigned char *aNew = sqlite3_realloc(aOut, nNew);
        ако( aNew==0 ){
          sqlite3_free(aOut);
          върни TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((char*)&aOut[nOut], iVal);
    }иначе{
      Tcl_Size nVal = 0;
      char *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      докато( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned char *aNew = sqlite3_realloc64(aOut, nNew);
        ако( aNew==0 ){
          sqlite3_free(aOut);
          върни TCL_ERROR;
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
  върни TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
цяло Sqlitetest_hexio_Init(Tcl_Interp *interp){
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
  цяло i;
  за(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  върни TCL_OK;
}
