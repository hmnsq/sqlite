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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
வெறுமை sqlite3TestBinToHex(unsigned எழுத்து *zBuf, முழு N){
  மாறிலி unsigned எழுத்து zHex[] = "0123456789ABCDEF";
  முழு i, j;
  unsigned எழுத்து c;
  i = N*2;
  zBuf[i--] = 0;
  க்கு(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  ப்ரதிஜ்ஞா( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
முழு sqlite3TestHexToBin(மாறிலி unsigned எழுத்து *zIn, முழு N, unsigned எழுத்து *aOut){
  மாறிலி unsigned எழுத்து aMap[] = {
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
  முழு i, j;
  முழு hi=1;
  unsigned எழுத்து c;

  க்கு(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    எனில்( c==0 ) தொடர்;
    எனில்( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }இல்லையெனில்{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  திரும்பு j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
நிலையான முழு SQLITE_TCLAPI hexio_read(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு offset;
  முழு amt, got;
  மாறிலி எழுத்து *zFile;
  unsigned எழுத்து *zBuf;
  FILE *in;

  எனில்( objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &offset) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[3], &amt) ) திரும்பு TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  எனில்( zBuf==0 ){
    திரும்பு TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  எனில்( in==0 ){
    in = fopen(zFile, "r");
  }
  எனில்( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, வெற்று);
    திரும்பு TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (முழு)fread(zBuf, 1, amt, in);
  fclose(in);
  எனில்( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  sqlite3_free(zBuf);
  திரும்பு TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
நிலையான முழு SQLITE_TCLAPI hexio_write(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு offset;
  Tcl_Size nIn;
  முழு nOut, written;
  மாறிலி எழுத்து *zFile;
  மாறிலி unsigned எழுத்து *zIn;
  unsigned எழுத்து *aOut;
  FILE *out;

  எனில்( objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &offset) ) திரும்பு TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (மாறிலி unsigned எழுத்து *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  எனில்( aOut==0 ){
    திரும்பு TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (முழு)nIn, aOut);
  out = fopen(zFile, "r+b");
  எனில்( out==0 ){
    out = fopen(zFile, "r+");
  }
  எனில்( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, வெற்று);
    திரும்பு TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (முழு)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  திரும்பு TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
நிலையான முழு SQLITE_TCLAPI hexio_get_int(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு val;
  Tcl_Size nIn;
  முழு nOut;
  மாறிலி unsigned எழுத்து *zIn;
  unsigned எழுத்து *aOut;
  unsigned எழுத்து a_இலக்கம்[4];
  முழு bLittle = 0;

  எனில்( objc==3 ){
    Tcl_Size n;
    எழுத்து *z = Tcl_GetStringFromObj(objv[1], &n);
    எனில்( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  எனில்( (objc-bLittle)!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    திரும்பு TCL_ERROR;
  }
  zIn = (மாறிலி unsigned எழுத்து *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  எனில்( aOut==0 ){
    திரும்பு TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (முழு)nIn, aOut);
  எனில்( nOut>=4 ){
    memcpy(a_இலக்கம், aOut, 4);
  }இல்லையெனில்{
    memset(a_இலக்கம், 0, அளவு(a_இலக்கம்));
    memcpy(&a_இலக்கம்[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  எனில்( bLittle ){
    val = (முழு)((u32)a_இலக்கம்[3]<<24) | (a_இலக்கம்[2]<<16) | (a_இலக்கம்[1]<<8) | a_இலக்கம்[0];
  }இல்லையெனில்{
    val = (முழு)((u32)a_இலக்கம்[0]<<24) | (a_இலக்கம்[1]<<16) | (a_இலக்கம்[2]<<8) | a_இலக்கம்[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  திரும்பு TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
நிலையான முழு SQLITE_TCLAPI hexio_render_int16(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு val;
  unsigned எழுத்து a_இலக்கம்[10];

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "INTEGER");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &val) ) திரும்பு TCL_ERROR;
  a_இலக்கம்[0] = val>>8;
  a_இலக்கம்[1] = val;
  sqlite3TestBinToHex(a_இலக்கம், 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((எழுத்து*)a_இலக்கம், 4));
  திரும்பு TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
நிலையான முழு SQLITE_TCLAPI hexio_render_int32(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு val;
  unsigned எழுத்து a_இலக்கம்[10];

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "INTEGER");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &val) ) திரும்பு TCL_ERROR;
  a_இலக்கம்[0] = val>>24;
  a_இலக்கம்[1] = val>>16;
  a_இலக்கம்[2] = val>>8;
  a_இலக்கம்[3] = val;
  sqlite3TestBinToHex(a_இலக்கம், 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((எழுத்து*)a_இலக்கம், 8));
  திரும்பு TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
நிலையான முழு SQLITE_TCLAPI utf8_to_utf8(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  முழு nOut;
  மாறிலி unsigned எழுத்து *zOrig;
  unsigned எழுத்து *z;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "HEX");
    திரும்பு TCL_ERROR;
  }
  zOrig = (unsigned எழுத்து *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (முழு)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (எழுத்து*)z, வெற்று);
  sqlite3_free(z);
  திரும்பு TCL_OK;
#இல்லையெனில்
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", வெற்று
  );
  திரும்பு TCL_ERROR;
#endif
}

நிலையான முழு getFts3Varint(மாறிலி எழுத்து *p, sqlite_int64 *v){
  மாறிலி unsigned எழுத்து *q = (மாறிலி unsigned எழுத்து *) p;
  sqlite_uint64 x = 0, y = 1;
  போது( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  திரும்பு (முழு) (q - (unsigned எழுத்து *)p);
}

நிலையான முழு putFts3Varint(எழுத்து *p, sqlite_int64 v){
  unsigned எழுத்து *q = (unsigned எழுத்து *) p;
  sqlite_uint64 vu = v;
  செய்{
    *q++ = (unsigned எழுத்து) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }போது( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  ப்ரதிஜ்ஞா( q - (unsigned எழுத்து *)p <= 10 );
  திரும்பு (முழு) (q - (unsigned எழுத்து *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
நிலையான முழு SQLITE_TCLAPI read_fts3varint(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned எழுத்து *zBlob;
  sqlite3_int64 iVal;
  முழு nVal;

  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BLOB VARNAME");
    திரும்பு TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((எழுத்து*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  திரும்பு TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
நிலையான முழு SQLITE_TCLAPI make_fts3record(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned எழுத்து *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  முழு i;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "LIST");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    திரும்பு TCL_ERROR;
  }

  க்கு(i=0; i<(முழு)nArg; i++){
    Tcl_WideInt iVal;
    எனில்( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      எனில்( nOut+10>nAlloc ){
        முழு nNew = nAlloc?nAlloc*2:128;
        unsigned எழுத்து *aNew = sqlite3_realloc(aOut, nNew);
        எனில்( aNew==0 ){
          sqlite3_free(aOut);
          திரும்பு TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((எழுத்து*)&aOut[nOut], iVal);
    }இல்லையெனில்{
      Tcl_Size nVal = 0;
      எழுத்து *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      போது( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned எழுத்து *aNew = sqlite3_realloc64(aOut, nNew);
        எனில்( aNew==0 ){
          sqlite3_free(aOut);
          திரும்பு TCL_ERROR;
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
  திரும்பு TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest_hexio_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
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
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  திரும்பு TCL_OK;
}
