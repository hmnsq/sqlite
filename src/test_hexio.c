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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
શૂન્ય sqlite3TestBinToHex(unsigned અક્ષર *zBuf, પૂર્ણાંક N){
  અચલ unsigned અક્ષર zHex[] = "0123456789ABCDEF";
  પૂર્ણાંક i, j;
  unsigned અક્ષર c;
  i = N*2;
  zBuf[i--] = 0;
  માટે(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  પ્રતિજ્ઞા( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
પૂર્ણાંક sqlite3TestHexToBin(અચલ unsigned અક્ષર *zIn, પૂર્ણાંક N, unsigned અક્ષર *aOut){
  અચલ unsigned અક્ષર aMap[] = {
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
  પૂર્ણાંક i, j;
  પૂર્ણાંક hi=1;
  unsigned અક્ષર c;

  માટે(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    જો( c==0 ) ચાલુ;
    જો( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }નહીંતર{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  પાછા j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI hexio_read(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક offset;
  પૂર્ણાંક amt, got;
  અચલ અક્ષર *zFile;
  unsigned અક્ષર *zBuf;
  FILE *in;

  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[2], &offset) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[3], &amt) ) પાછા TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  જો( zBuf==0 ){
    પાછા TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  જો( in==0 ){
    in = fopen(zFile, "r");
  }
  જો( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, ખાલી);
    પાછા TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (પૂર્ણાંક)fread(zBuf, 1, amt, in);
  fclose(in);
  જો( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  sqlite3_free(zBuf);
  પાછા TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI hexio_write(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક offset;
  Tcl_Size nIn;
  પૂર્ણાંક nOut, written;
  અચલ અક્ષર *zFile;
  અચલ unsigned અક્ષર *zIn;
  unsigned અક્ષર *aOut;
  FILE *out;

  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[2], &offset) ) પાછા TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (અચલ unsigned અક્ષર *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  જો( aOut==0 ){
    પાછા TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (પૂર્ણાંક)nIn, aOut);
  out = fopen(zFile, "r+b");
  જો( out==0 ){
    out = fopen(zFile, "r+");
  }
  જો( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, ખાલી);
    પાછા TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (પૂર્ણાંક)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  પાછા TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI hexio_get_int(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક val;
  Tcl_Size nIn;
  પૂર્ણાંક nOut;
  અચલ unsigned અક્ષર *zIn;
  unsigned અક્ષર *aOut;
  unsigned અક્ષર a_અંક[4];
  પૂર્ણાંક bLittle = 0;

  જો( objc==3 ){
    Tcl_Size n;
    અક્ષર *z = Tcl_GetStringFromObj(objv[1], &n);
    જો( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  જો( (objc-bLittle)!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    પાછા TCL_ERROR;
  }
  zIn = (અચલ unsigned અક્ષર *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  જો( aOut==0 ){
    પાછા TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (પૂર્ણાંક)nIn, aOut);
  જો( nOut>=4 ){
    memcpy(a_અંક, aOut, 4);
  }નહીંતર{
    memset(a_અંક, 0, કદ(a_અંક));
    memcpy(&a_અંક[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  જો( bLittle ){
    val = (પૂર્ણાંક)((u32)a_અંક[3]<<24) | (a_અંક[2]<<16) | (a_અંક[1]<<8) | a_અંક[0];
  }નહીંતર{
    val = (પૂર્ણાંક)((u32)a_અંક[0]<<24) | (a_અંક[1]<<16) | (a_અંક[2]<<8) | a_અંક[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  પાછા TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI hexio_render_int16(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક val;
  unsigned અક્ષર a_અંક[10];

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "INTEGER");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &val) ) પાછા TCL_ERROR;
  a_અંક[0] = val>>8;
  a_અંક[1] = val;
  sqlite3TestBinToHex(a_અંક, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((અક્ષર*)a_અંક, 4));
  પાછા TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI hexio_render_int32(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક val;
  unsigned અક્ષર a_અંક[10];

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "INTEGER");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &val) ) પાછા TCL_ERROR;
  a_અંક[0] = val>>24;
  a_અંક[1] = val>>16;
  a_અંક[2] = val>>8;
  a_અંક[3] = val;
  sqlite3TestBinToHex(a_અંક, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((અક્ષર*)a_અંક, 8));
  પાછા TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI utf8_to_utf8(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  પૂર્ણાંક nOut;
  અચલ unsigned અક્ષર *zOrig;
  unsigned અક્ષર *z;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HEX");
    પાછા TCL_ERROR;
  }
  zOrig = (unsigned અક્ષર *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (પૂર્ણાંક)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (અક્ષર*)z, ખાલી);
  sqlite3_free(z);
  પાછા TCL_OK;
#નહીંતર
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ખાલી
  );
  પાછા TCL_ERROR;
#endif
}

સ્થિર પૂર્ણાંક getFts3Varint(અચલ અક્ષર *p, sqlite_int64 *v){
  અચલ unsigned અક્ષર *q = (અચલ unsigned અક્ષર *) p;
  sqlite_uint64 x = 0, y = 1;
  જ્યારે( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  પાછા (પૂર્ણાંક) (q - (unsigned અક્ષર *)p);
}

સ્થિર પૂર્ણાંક putFts3Varint(અક્ષર *p, sqlite_int64 v){
  unsigned અક્ષર *q = (unsigned અક્ષર *) p;
  sqlite_uint64 vu = v;
  કરો{
    *q++ = (unsigned અક્ષર) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }જ્યારે( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  પ્રતિજ્ઞા( q - (unsigned અક્ષર *)p <= 10 );
  પાછા (પૂર્ણાંક) (q - (unsigned અક્ષર *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI read_fts3varint(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned અક્ષર *zBlob;
  sqlite3_int64 iVal;
  પૂર્ણાંક nVal;

  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BLOB VARNAME");
    પાછા TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((અક્ષર*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  પાછા TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI make_fts3record(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned અક્ષર *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  પૂર્ણાંક i;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "LIST");
    પાછા TCL_ERROR;
  }
  જો( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    પાછા TCL_ERROR;
  }

  માટે(i=0; i<(પૂર્ણાંક)nArg; i++){
    Tcl_WideInt iVal;
    જો( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      જો( nOut+10>nAlloc ){
        પૂર્ણાંક nNew = nAlloc?nAlloc*2:128;
        unsigned અક્ષર *aNew = sqlite3_realloc(aOut, nNew);
        જો( aNew==0 ){
          sqlite3_free(aOut);
          પાછા TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((અક્ષર*)&aOut[nOut], iVal);
    }નહીંતર{
      Tcl_Size nVal = 0;
      અક્ષર *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      જ્યારે( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned અક્ષર *aNew = sqlite3_realloc64(aOut, nNew);
        જો( aNew==0 ){
          sqlite3_free(aOut);
          પાછા TCL_ERROR;
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
  પાછા TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest_hexio_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
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
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  પાછા TCL_OK;
}
