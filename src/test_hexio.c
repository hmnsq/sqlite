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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
ಶೂನ್ಯ sqlite3TestBinToHex(unsigned ವರ್ಣ *zBuf, ಪೂರ್ಣಾಙ್ಕ N){
  ಧ್ರುವ unsigned ವರ್ಣ zHex[] = "0123456789ABCDEF";
  ಪೂರ್ಣಾಙ್ಕ i, j;
  unsigned ವರ್ಣ c;
  i = N*2;
  zBuf[i--] = 0;
  ಪ್ರತಿ(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  ಪ್ರತಿಜ್ಞಾ( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3TestHexToBin(ಧ್ರುವ unsigned ವರ್ಣ *zIn, ಪೂರ್ಣಾಙ್ಕ N, unsigned ವರ್ಣ *aOut){
  ಧ್ರುವ unsigned ವರ್ಣ aMap[] = {
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
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪೂರ್ಣಾಙ್ಕ hi=1;
  unsigned ವರ್ಣ c;

  ಪ್ರತಿ(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    ಯದಿ( c==0 ) ಅನುವರ್ತ;
    ಯದಿ( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }ಅನ್ಯಥಾ{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI hexio_read(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ offset;
  ಪೂರ್ಣಾಙ್ಕ amt, got;
  ಧ್ರುವ ವರ್ಣ *zFile;
  unsigned ವರ್ಣ *zBuf;
  FILE *in;

  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ಯದಿ( zBuf==0 ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  ಯದಿ( in==0 ){
    in = fopen(zFile, "r");
  }
  ಯದಿ( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (ಪೂರ್ಣಾಙ್ಕ)fread(zBuf, 1, amt, in);
  fclose(in);
  ಯದಿ( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  sqlite3_free(zBuf);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI hexio_write(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ offset;
  Tcl_Size nIn;
  ಪೂರ್ಣಾಙ್ಕ nOut, written;
  ಧ್ರುವ ವರ್ಣ *zFile;
  ಧ್ರುವ unsigned ವರ್ಣ *zIn;
  unsigned ವರ್ಣ *aOut;
  FILE *out;

  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (ಧ್ರುವ unsigned ವರ್ಣ *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ಯದಿ( aOut==0 ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ಪೂರ್ಣಾಙ್ಕ)nIn, aOut);
  out = fopen(zFile, "r+b");
  ಯದಿ( out==0 ){
    out = fopen(zFile, "r+");
  }
  ಯದಿ( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (ಪೂರ್ಣಾಙ್ಕ)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI hexio_get_int(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ val;
  Tcl_Size nIn;
  ಪೂರ್ಣಾಙ್ಕ nOut;
  ಧ್ರುವ unsigned ವರ್ಣ *zIn;
  unsigned ವರ್ಣ *aOut;
  unsigned ವರ್ಣ a_ಅಂಕೆ[4];
  ಪೂರ್ಣಾಙ್ಕ bLittle = 0;

  ಯದಿ( objc==3 ){
    Tcl_Size n;
    ವರ್ಣ *z = Tcl_GetStringFromObj(objv[1], &n);
    ಯದಿ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ಯದಿ( (objc-bLittle)!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zIn = (ಧ್ರುವ unsigned ವರ್ಣ *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ಯದಿ( aOut==0 ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ಪೂರ್ಣಾಙ್ಕ)nIn, aOut);
  ಯದಿ( nOut>=4 ){
    memcpy(a_ಅಂಕೆ, aOut, 4);
  }ಅನ್ಯಥಾ{
    memset(a_ಅಂಕೆ, 0, ಪರಿಮಾಣ(a_ಅಂಕೆ));
    memcpy(&a_ಅಂಕೆ[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  ಯದಿ( bLittle ){
    val = (ಪೂರ್ಣಾಙ್ಕ)((u32)a_ಅಂಕೆ[3]<<24) | (a_ಅಂಕೆ[2]<<16) | (a_ಅಂಕೆ[1]<<8) | a_ಅಂಕೆ[0];
  }ಅನ್ಯಥಾ{
    val = (ಪೂರ್ಣಾಙ್ಕ)((u32)a_ಅಂಕೆ[0]<<24) | (a_ಅಂಕೆ[1]<<16) | (a_ಅಂಕೆ[2]<<8) | a_ಅಂಕೆ[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI hexio_render_int16(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ val;
  unsigned ವರ್ಣ a_ಅಂಕೆ[10];

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "INTEGER");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  a_ಅಂಕೆ[0] = val>>8;
  a_ಅಂಕೆ[1] = val;
  sqlite3TestBinToHex(a_ಅಂಕೆ, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((ವರ್ಣ*)a_ಅಂಕೆ, 4));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI hexio_render_int32(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ val;
  unsigned ವರ್ಣ a_ಅಂಕೆ[10];

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "INTEGER");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  a_ಅಂಕೆ[0] = val>>24;
  a_ಅಂಕೆ[1] = val>>16;
  a_ಅಂಕೆ[2] = val>>8;
  a_ಅಂಕೆ[3] = val;
  sqlite3TestBinToHex(a_ಅಂಕೆ, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((ವರ್ಣ*)a_ಅಂಕೆ, 8));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI utf8_to_utf8(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  ಪೂರ್ಣಾಙ್ಕ nOut;
  ಧ್ರುವ unsigned ವರ್ಣ *zOrig;
  unsigned ವರ್ಣ *z;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HEX");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zOrig = (unsigned ವರ್ಣ *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (ಪೂರ್ಣಾಙ್ಕ)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (ವರ್ಣ*)z, ರಿಕ್ತ);
  sqlite3_free(z);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
#ಅನ್ಯಥಾ
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ರಿಕ್ತ
  );
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getFts3Varint(ಧ್ರುವ ವರ್ಣ *p, sqlite_int64 *v){
  ಧ್ರುವ unsigned ವರ್ಣ *q = (ಧ್ರುವ unsigned ವರ್ಣ *) p;
  sqlite_uint64 x = 0, y = 1;
  ಯಾವತ್( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ) (q - (unsigned ವರ್ಣ *)p);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ putFts3Varint(ವರ್ಣ *p, sqlite_int64 v){
  unsigned ವರ್ಣ *q = (unsigned ವರ್ಣ *) p;
  sqlite_uint64 vu = v;
  ಕುರು{
    *q++ = (unsigned ವರ್ಣ) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }ಯಾವತ್( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  ಪ್ರತಿಜ್ಞಾ( q - (unsigned ವರ್ಣ *)p <= 10 );
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ) (q - (unsigned ವರ್ಣ *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI read_fts3varint(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned ವರ್ಣ *zBlob;
  sqlite3_int64 iVal;
  ಪೂರ್ಣಾಙ್ಕ nVal;

  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BLOB VARNAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((ವರ್ಣ*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI make_fts3record(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned ವರ್ಣ *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "LIST");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಪ್ರತಿ(i=0; i<(ಪೂರ್ಣಾಙ್ಕ)nArg; i++){
    Tcl_WideInt iVal;
    ಯದಿ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ಯದಿ( nOut+10>nAlloc ){
        ಪೂರ್ಣಾಙ್ಕ nNew = nAlloc?nAlloc*2:128;
        unsigned ವರ್ಣ *aNew = sqlite3_realloc(aOut, nNew);
        ಯದಿ( aNew==0 ){
          sqlite3_free(aOut);
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((ವರ್ಣ*)&aOut[nOut], iVal);
    }ಅನ್ಯಥಾ{
      Tcl_Size nVal = 0;
      ವರ್ಣ *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      ಯಾವತ್( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned ವರ್ಣ *aNew = sqlite3_realloc64(aOut, nNew);
        ಯದಿ( aNew==0 ){
          sqlite3_free(aOut);
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
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
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
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
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
