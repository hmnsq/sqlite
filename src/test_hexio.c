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
void sqlite3TestBinToHex(unsigned var_na *zBuf, ចំនួនគត់ N){
  const unsigned var_na zHex[] = "0123456789ABCDEF";
  ចំនួនគត់ i, j;
  unsigned var_na c;
  i = N*2;
  zBuf[i--] = 0;
  សម្រាប់(j=N-1; j>=0; j--){
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
ចំនួនគត់ sqlite3TestHexToBin(const unsigned var_na *zIn, ចំនួនគត់ N, unsigned var_na *aOut){
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
  ចំនួនគត់ i, j;
  ចំនួនគត់ hi=1;
  unsigned var_na c;

  សម្រាប់(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    ប្រសិនបើ( c==0 ) continue;
    ប្រសិនបើ( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }បើមិនដូច្នេះ{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  ត្រឡប់ j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
static ចំនួនគត់ SQLITE_TCLAPI hexio_read(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ offset;
  ចំនួនគត់ amt, got;
  const var_na *zFile;
  unsigned var_na *zBuf;
  FILE *in;

  ប្រសិនបើ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET AMT");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ត្រឡប់ TCL_ERROR;
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ត្រឡប់ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ប្រសិនបើ( zBuf==0 ){
    ត្រឡប់ TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  ប្រសិនបើ( in==0 ){
    in = fopen(zFile, "r");
  }
  ប្រសិនបើ( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, NULL);
    ត្រឡប់ TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (ចំនួនគត់)fread(zBuf, 1, amt, in);
  fclose(in);
  ប្រសិនបើ( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, NULL);
  sqlite3_free(zBuf);
  ត្រឡប់ TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
static ចំនួនគត់ SQLITE_TCLAPI hexio_write(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ offset;
  Tcl_Size nIn;
  ចំនួនគត់ nOut, written;
  const var_na *zFile;
  const unsigned var_na *zIn;
  unsigned var_na *aOut;
  FILE *out;

  ប្រសិនបើ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ត្រឡប់ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (const unsigned var_na *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ប្រសិនបើ( aOut==0 ){
    ត្រឡប់ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ចំនួនគត់)nIn, aOut);
  out = fopen(zFile, "r+b");
  ប្រសិនបើ( out==0 ){
    out = fopen(zFile, "r+");
  }
  ប្រសិនបើ( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, NULL);
    ត្រឡប់ TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (ចំនួនគត់)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  ត្រឡប់ TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
static ចំនួនគត់ SQLITE_TCLAPI hexio_get_int(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ val;
  Tcl_Size nIn;
  ចំនួនគត់ nOut;
  const unsigned var_na *zIn;
  unsigned var_na *aOut;
  unsigned var_na aNum[4];
  ចំនួនគត់ bLittle = 0;

  ប្រសិនបើ( objc==3 ){
    Tcl_Size n;
    var_na *z = Tcl_GetStringFromObj(objv[1], &n);
    ប្រសិនបើ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ប្រសិនបើ( (objc-bLittle)!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[-littleendian] HEXDATA");
    ត្រឡប់ TCL_ERROR;
  }
  zIn = (const unsigned var_na *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ប្រសិនបើ( aOut==0 ){
    ត្រឡប់ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ចំនួនគត់)nIn, aOut);
  ប្រសិនបើ( nOut>=4 ){
    memcpy(aNum, aOut, 4);
  }បើមិនដូច្នេះ{
    memset(aNum, 0, parimaa_na(aNum));
    memcpy(&aNum[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  ប្រសិនបើ( bLittle ){
    val = (ចំនួនគត់)((u32)aNum[3]<<24) | (aNum[2]<<16) | (aNum[1]<<8) | aNum[0];
  }បើមិនដូច្នេះ{
    val = (ចំនួនគត់)((u32)aNum[0]<<24) | (aNum[1]<<16) | (aNum[2]<<8) | aNum[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  ត្រឡប់ TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
static ចំនួនគត់ SQLITE_TCLAPI hexio_render_int16(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ val;
  unsigned var_na aNum[10];

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ត្រឡប់ TCL_ERROR;
  aNum[0] = val>>8;
  aNum[1] = val;
  sqlite3TestBinToHex(aNum, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((var_na*)aNum, 4));
  ត្រឡប់ TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
static ចំនួនគត់ SQLITE_TCLAPI hexio_render_int32(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ val;
  unsigned var_na aNum[10];

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ត្រឡប់ TCL_ERROR;
  aNum[0] = val>>24;
  aNum[1] = val>>16;
  aNum[2] = val>>8;
  aNum[3] = val;
  sqlite3TestBinToHex(aNum, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((var_na*)aNum, 8));
  ត្រឡប់ TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
static ចំនួនគត់ SQLITE_TCLAPI utf8_to_utf8(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  ចំនួនគត់ nOut;
  const unsigned var_na *zOrig;
  unsigned var_na *z;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HEX");
    ត្រឡប់ TCL_ERROR;
  }
  zOrig = (unsigned var_na *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (ចំនួនគត់)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (var_na*)z, NULL);
  sqlite3_free(z);
  ត្រឡប់ TCL_OK;
#បើមិនដូច្នេះ
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", NULL
  );
  ត្រឡប់ TCL_ERROR;
#endif
}

static ចំនួនគត់ getFts3Varint(const var_na *p, sqlite_int64 *v){
  const unsigned var_na *q = (const unsigned var_na *) p;
  sqlite_uint64 x = 0, y = 1;
  ខណៈពេល( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ត្រឡប់ (ចំនួនគត់) (q - (unsigned var_na *)p);
}

static ចំនួនគត់ putFts3Varint(var_na *p, sqlite_int64 v){
  unsigned var_na *q = (unsigned var_na *) p;
  sqlite_uint64 vu = v;
  do{
    *q++ = (unsigned var_na) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }ខណៈពេល( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  assert( q - (unsigned var_na *)p <= 10 );
  ត្រឡប់ (ចំនួនគត់) (q - (unsigned var_na *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
static ចំនួនគត់ SQLITE_TCLAPI read_fts3varint(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned var_na *zBlob;
  sqlite3_int64 iVal;
  ចំនួនគត់ nVal;

  ប្រសិនបើ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BLOB VARNAME");
    ត្រឡប់ TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((var_na*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  ត្រឡប់ TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
static ចំនួនគត់ SQLITE_TCLAPI make_fts3record(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned var_na *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  ចំនួនគត់ i;

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "LIST");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ត្រឡប់ TCL_ERROR;
  }

  សម្រាប់(i=0; i<(ចំនួនគត់)nArg; i++){
    Tcl_WideInt iVal;
    ប្រសិនបើ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ប្រសិនបើ( nOut+10>nAlloc ){
        ចំនួនគត់ nNew = nAlloc?nAlloc*2:128;
        unsigned var_na *aNew = sqlite3_realloc(aOut, nNew);
        ប្រសិនបើ( aNew==0 ){
          sqlite3_free(aOut);
          ត្រឡប់ TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((var_na*)&aOut[nOut], iVal);
    }បើមិនដូច្នេះ{
      Tcl_Size nVal = 0;
      var_na *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      ខណៈពេល( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned var_na *aNew = sqlite3_realloc64(aOut, nNew);
        ប្រសិនបើ( aNew==0 ){
          sqlite3_free(aOut);
          ត្រឡប់ TCL_ERROR;
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
  ត្រឡប់ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ចំនួនគត់ Sqlitetest_hexio_Init(Tcl_Interp *interp){
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
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ត្រឡប់ TCL_OK;
}
