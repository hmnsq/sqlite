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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
ശൂന്യം sqlite3TestBinToHex(unsigned അക്ഷരം *zBuf, പൂർണ്ണസംഖ്യ N){
  സ്ഥിരം unsigned അക്ഷരം zHex[] = "0123456789ABCDEF";
  പൂർണ്ണസംഖ്യ i, j;
  unsigned അക്ഷരം c;
  i = N*2;
  zBuf[i--] = 0;
  വേണ്ടി(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  പ്രതിജ്ഞാ( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
പൂർണ്ണസംഖ്യ sqlite3TestHexToBin(സ്ഥിരം unsigned അക്ഷരം *zIn, പൂർണ്ണസംഖ്യ N, unsigned അക്ഷരം *aOut){
  സ്ഥിരം unsigned അക്ഷരം aMap[] = {
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
  പൂർണ്ണസംഖ്യ i, j;
  പൂർണ്ണസംഖ്യ hi=1;
  unsigned അക്ഷരം c;

  വേണ്ടി(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    എങ്കിൽ( c==0 ) തുടരുക;
    എങ്കിൽ( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }അല്ലെങ്കിൽ{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  തിരികെ j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI hexio_read(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ offset;
  പൂർണ്ണസംഖ്യ amt, got;
  സ്ഥിരം അക്ഷരം *zFile;
  unsigned അക്ഷരം *zBuf;
  FILE *in;

  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) തിരികെ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  എങ്കിൽ( zBuf==0 ){
    തിരികെ TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  എങ്കിൽ( in==0 ){
    in = fopen(zFile, "r");
  }
  എങ്കിൽ( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (പൂർണ്ണസംഖ്യ)fread(zBuf, 1, amt, in);
  fclose(in);
  എങ്കിൽ( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  sqlite3_free(zBuf);
  തിരികെ TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI hexio_write(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ offset;
  Tcl_Size nIn;
  പൂർണ്ണസംഖ്യ nOut, written;
  സ്ഥിരം അക്ഷരം *zFile;
  സ്ഥിരം unsigned അക്ഷരം *zIn;
  unsigned അക്ഷരം *aOut;
  FILE *out;

  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) തിരികെ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (സ്ഥിരം unsigned അക്ഷരം *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  എങ്കിൽ( aOut==0 ){
    തിരികെ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (പൂർണ്ണസംഖ്യ)nIn, aOut);
  out = fopen(zFile, "r+b");
  എങ്കിൽ( out==0 ){
    out = fopen(zFile, "r+");
  }
  എങ്കിൽ( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (പൂർണ്ണസംഖ്യ)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  തിരികെ TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI hexio_get_int(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ val;
  Tcl_Size nIn;
  പൂർണ്ണസംഖ്യ nOut;
  സ്ഥിരം unsigned അക്ഷരം *zIn;
  unsigned അക്ഷരം *aOut;
  unsigned അക്ഷരം a_അക്കം[4];
  പൂർണ്ണസംഖ്യ bLittle = 0;

  എങ്കിൽ( objc==3 ){
    Tcl_Size n;
    അക്ഷരം *z = Tcl_GetStringFromObj(objv[1], &n);
    എങ്കിൽ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  എങ്കിൽ( (objc-bLittle)!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    തിരികെ TCL_ERROR;
  }
  zIn = (സ്ഥിരം unsigned അക്ഷരം *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  എങ്കിൽ( aOut==0 ){
    തിരികെ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (പൂർണ്ണസംഖ്യ)nIn, aOut);
  എങ്കിൽ( nOut>=4 ){
    memcpy(a_അക്കം, aOut, 4);
  }അല്ലെങ്കിൽ{
    memset(a_അക്കം, 0, വലുപ്പം(a_അക്കം));
    memcpy(&a_അക്കം[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  എങ്കിൽ( bLittle ){
    val = (പൂർണ്ണസംഖ്യ)((u32)a_അക്കം[3]<<24) | (a_അക്കം[2]<<16) | (a_അക്കം[1]<<8) | a_അക്കം[0];
  }അല്ലെങ്കിൽ{
    val = (പൂർണ്ണസംഖ്യ)((u32)a_അക്കം[0]<<24) | (a_അക്കം[1]<<16) | (a_അക്കം[2]<<8) | a_അക്കം[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  തിരികെ TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI hexio_render_int16(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ val;
  unsigned അക്ഷരം a_അക്കം[10];

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "INTEGER");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &val) ) തിരികെ TCL_ERROR;
  a_അക്കം[0] = val>>8;
  a_അക്കം[1] = val;
  sqlite3TestBinToHex(a_അക്കം, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((അക്ഷരം*)a_അക്കം, 4));
  തിരികെ TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI hexio_render_int32(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ val;
  unsigned അക്ഷരം a_അക്കം[10];

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "INTEGER");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &val) ) തിരികെ TCL_ERROR;
  a_അക്കം[0] = val>>24;
  a_അക്കം[1] = val>>16;
  a_അക്കം[2] = val>>8;
  a_അക്കം[3] = val;
  sqlite3TestBinToHex(a_അക്കം, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((അക്ഷരം*)a_അക്കം, 8));
  തിരികെ TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI utf8_to_utf8(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  പൂർണ്ണസംഖ്യ nOut;
  സ്ഥിരം unsigned അക്ഷരം *zOrig;
  unsigned അക്ഷരം *z;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HEX");
    തിരികെ TCL_ERROR;
  }
  zOrig = (unsigned അക്ഷരം *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (പൂർണ്ണസംഖ്യ)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (അക്ഷരം*)z, ശൂന്യ);
  sqlite3_free(z);
  തിരികെ TCL_OK;
#അല്ലെങ്കിൽ
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ശൂന്യ
  );
  തിരികെ TCL_ERROR;
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ getFts3Varint(സ്ഥിരം അക്ഷരം *p, sqlite_int64 *v){
  സ്ഥിരം unsigned അക്ഷരം *q = (സ്ഥിരം unsigned അക്ഷരം *) p;
  sqlite_uint64 x = 0, y = 1;
  ആകുമ്പോൾ( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  തിരികെ (പൂർണ്ണസംഖ്യ) (q - (unsigned അക്ഷരം *)p);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ putFts3Varint(അക്ഷരം *p, sqlite_int64 v){
  unsigned അക്ഷരം *q = (unsigned അക്ഷരം *) p;
  sqlite_uint64 vu = v;
  ചെയ്യുക{
    *q++ = (unsigned അക്ഷരം) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }ആകുമ്പോൾ( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  പ്രതിജ്ഞാ( q - (unsigned അക്ഷരം *)p <= 10 );
  തിരികെ (പൂർണ്ണസംഖ്യ) (q - (unsigned അക്ഷരം *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI read_fts3varint(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned അക്ഷരം *zBlob;
  sqlite3_int64 iVal;
  പൂർണ്ണസംഖ്യ nVal;

  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BLOB VARNAME");
    തിരികെ TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((അക്ഷരം*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  തിരികെ TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI make_fts3record(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned അക്ഷരം *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "LIST");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    തിരികെ TCL_ERROR;
  }

  വേണ്ടി(i=0; i<(പൂർണ്ണസംഖ്യ)nArg; i++){
    Tcl_WideInt iVal;
    എങ്കിൽ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      എങ്കിൽ( nOut+10>nAlloc ){
        പൂർണ്ണസംഖ്യ nNew = nAlloc?nAlloc*2:128;
        unsigned അക്ഷരം *aNew = sqlite3_realloc(aOut, nNew);
        എങ്കിൽ( aNew==0 ){
          sqlite3_free(aOut);
          തിരികെ TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((അക്ഷരം*)&aOut[nOut], iVal);
    }അല്ലെങ്കിൽ{
      Tcl_Size nVal = 0;
      അക്ഷരം *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      ആകുമ്പോൾ( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned അക്ഷരം *aNew = sqlite3_realloc64(aOut, nNew);
        എങ്കിൽ( aNew==0 ){
          sqlite3_free(aOut);
          തിരികെ TCL_ERROR;
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
  തിരികെ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest_hexio_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
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
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  തിരികെ TCL_OK;
}
