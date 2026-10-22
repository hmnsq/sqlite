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
void sqlite3TestBinToHex(unsigned var_na *zBuf, ཧྲིལ N){
  const unsigned var_na zHex[] = "0123456789ABCDEF";
  ཧྲིལ i, j;
  unsigned var_na c;
  i = N*2;
  zBuf[i--] = 0;
  ཆེད་དུ(j=N-1; j>=0; j--){
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
ཧྲིལ sqlite3TestHexToBin(const unsigned var_na *zIn, ཧྲིལ N, unsigned var_na *aOut){
  const unsigned var_na aMap[] = {
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
  ཧྲིལ i, j;
  ཧྲིལ hi=1;
  unsigned var_na c;

  ཆེད་དུ(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    གལ་ཏེ( c==0 ) continue;
    གལ་ཏེ( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }དེ་མིན{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  ཕྱིར j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
static ཧྲིལ SQLITE_TCLAPI hexio_read(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ offset;
  ཧྲིལ amt, got;
  const var_na *zFile;
  unsigned var_na *zBuf;
  FILE *in;

  གལ་ཏེ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET AMT");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ཕྱིར TCL_ERROR;
  གལ་ཏེ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ཕྱིར TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  གལ་ཏེ( zBuf==0 ){
    ཕྱིར TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  གལ་ཏེ( in==0 ){
    in = fopen(zFile, "r");
  }
  གལ་ཏེ( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, NULL);
    ཕྱིར TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (ཧྲིལ)fread(zBuf, 1, amt, in);
  fclose(in);
  གལ་ཏེ( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, NULL);
  sqlite3_free(zBuf);
  ཕྱིར TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
static ཧྲིལ SQLITE_TCLAPI hexio_write(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ offset;
  Tcl_Size nIn;
  ཧྲིལ nOut, written;
  const var_na *zFile;
  const unsigned var_na *zIn;
  unsigned var_na *aOut;
  FILE *out;

  གལ་ཏེ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ཕྱིར TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (const unsigned var_na *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  གལ་ཏེ( aOut==0 ){
    ཕྱིར TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ཧྲིལ)nIn, aOut);
  out = fopen(zFile, "r+b");
  གལ་ཏེ( out==0 ){
    out = fopen(zFile, "r+");
  }
  གལ་ཏེ( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, NULL);
    ཕྱིར TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (ཧྲིལ)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  ཕྱིར TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
static ཧྲིལ SQLITE_TCLAPI hexio_get_int(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ val;
  Tcl_Size nIn;
  ཧྲིལ nOut;
  const unsigned var_na *zIn;
  unsigned var_na *aOut;
  unsigned var_na aNum[4];
  ཧྲིལ bLittle = 0;

  གལ་ཏེ( objc==3 ){
    Tcl_Size n;
    var_na *z = Tcl_GetStringFromObj(objv[1], &n);
    གལ་ཏེ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  གལ་ཏེ( (objc-bLittle)!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[-littleendian] HEXDATA");
    ཕྱིར TCL_ERROR;
  }
  zIn = (const unsigned var_na *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  གལ་ཏེ( aOut==0 ){
    ཕྱིར TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ཧྲིལ)nIn, aOut);
  གལ་ཏེ( nOut>=4 ){
    memcpy(aNum, aOut, 4);
  }དེ་མིན{
    memset(aNum, 0, parimaa_na(aNum));
    memcpy(&aNum[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  གལ་ཏེ( bLittle ){
    val = (ཧྲིལ)((u32)aNum[3]<<24) | (aNum[2]<<16) | (aNum[1]<<8) | aNum[0];
  }དེ་མིན{
    val = (ཧྲིལ)((u32)aNum[0]<<24) | (aNum[1]<<16) | (aNum[2]<<8) | aNum[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  ཕྱིར TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
static ཧྲིལ SQLITE_TCLAPI hexio_render_int16(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ val;
  unsigned var_na aNum[10];

  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ཕྱིར TCL_ERROR;
  aNum[0] = val>>8;
  aNum[1] = val;
  sqlite3TestBinToHex(aNum, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((var_na*)aNum, 4));
  ཕྱིར TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
static ཧྲིལ SQLITE_TCLAPI hexio_render_int32(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ val;
  unsigned var_na aNum[10];

  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ཕྱིར TCL_ERROR;
  aNum[0] = val>>24;
  aNum[1] = val>>16;
  aNum[2] = val>>8;
  aNum[3] = val;
  sqlite3TestBinToHex(aNum, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((var_na*)aNum, 8));
  ཕྱིར TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
static ཧྲིལ SQLITE_TCLAPI utf8_to_utf8(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  ཧྲིལ nOut;
  const unsigned var_na *zOrig;
  unsigned var_na *z;
  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HEX");
    ཕྱིར TCL_ERROR;
  }
  zOrig = (unsigned var_na *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (ཧྲིལ)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (var_na*)z, NULL);
  sqlite3_free(z);
  ཕྱིར TCL_OK;
#དེ་མིན
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", NULL
  );
  ཕྱིར TCL_ERROR;
#endif
}

static ཧྲིལ getFts3Varint(const var_na *p, sqlite_int64 *v){
  const unsigned var_na *q = (const unsigned var_na *) p;
  sqlite_uint64 x = 0, y = 1;
  སྐབས( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ཕྱིར (ཧྲིལ) (q - (unsigned var_na *)p);
}

static ཧྲིལ putFts3Varint(var_na *p, sqlite_int64 v){
  unsigned var_na *q = (unsigned var_na *) p;
  sqlite_uint64 vu = v;
  do{
    *q++ = (unsigned var_na) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }སྐབས( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  assert( q - (unsigned var_na *)p <= 10 );
  ཕྱིར (ཧྲིལ) (q - (unsigned var_na *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
static ཧྲིལ SQLITE_TCLAPI read_fts3varint(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned var_na *zBlob;
  sqlite3_int64 iVal;
  ཧྲིལ nVal;

  གལ་ཏེ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BLOB VARNAME");
    ཕྱིར TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((var_na*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  ཕྱིར TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
static ཧྲིལ SQLITE_TCLAPI make_fts3record(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned var_na *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  ཧྲིལ i;

  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "LIST");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ཕྱིར TCL_ERROR;
  }

  ཆེད་དུ(i=0; i<(ཧྲིལ)nArg; i++){
    Tcl_WideInt iVal;
    གལ་ཏེ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      གལ་ཏེ( nOut+10>nAlloc ){
        ཧྲིལ nNew = nAlloc?nAlloc*2:128;
        unsigned var_na *aNew = sqlite3_realloc(aOut, nNew);
        གལ་ཏེ( aNew==0 ){
          sqlite3_free(aOut);
          ཕྱིར TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((var_na*)&aOut[nOut], iVal);
    }དེ་མིན{
      Tcl_Size nVal = 0;
      var_na *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      སྐབས( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned var_na *aNew = sqlite3_realloc64(aOut, nNew);
        གལ་ཏེ( aNew==0 ){
          sqlite3_free(aOut);
          ཕྱིར TCL_ERROR;
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
  ཕྱིར TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ཧྲིལ Sqlitetest_hexio_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
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
  ཧྲིལ i;
  ཆེད་དུ(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ཕྱིར TCL_OK;
}
