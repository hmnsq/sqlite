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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
ଶୂନ୍ୟ sqlite3TestBinToHex(unsigned ଅକ୍ଷର *zBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଅଚଳ unsigned ଅକ୍ଷର zHex[] = "0123456789ABCDEF";
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  unsigned ଅକ୍ଷର c;
  i = N*2;
  zBuf[i--] = 0;
  ପାଇଁ(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  ପ୍ରତିଜ୍ଞା( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestHexToBin(ଅଚଳ unsigned ଅକ୍ଷର *zIn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, unsigned ଅକ୍ଷର *aOut){
  ଅଚଳ unsigned ଅକ୍ଷର aMap[] = {
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hi=1;
  unsigned ଅକ୍ଷର c;

  ପାଇଁ(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    ଯଦି( c==0 ) ଜାରି;
    ଯଦି( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }ଅନ୍ୟଥା{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  ଫେରନ୍ତୁ j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI hexio_read(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା amt, got;
  ଅଚଳ ଅକ୍ଷର *zFile;
  unsigned ଅକ୍ଷର *zBuf;
  FILE *in;

  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ଫେରନ୍ତୁ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ଯଦି( zBuf==0 ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  ଯଦି( in==0 ){
    in = fopen(zFile, "r");
  }
  ଯଦି( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fread(zBuf, 1, amt, in);
  fclose(in);
  ଯଦି( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  sqlite3_free(zBuf);
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI hexio_write(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset;
  Tcl_Size nIn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut, written;
  ଅଚଳ ଅକ୍ଷର *zFile;
  ଅଚଳ unsigned ଅକ୍ଷର *zIn;
  unsigned ଅକ୍ଷର *aOut;
  FILE *out;

  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ଫେରନ୍ତୁ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (ଅଚଳ unsigned ଅକ୍ଷର *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ଯଦି( aOut==0 ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nIn, aOut);
  out = fopen(zFile, "r+b");
  ଯଦି( out==0 ){
    out = fopen(zFile, "r+");
  }
  ଯଦି( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI hexio_get_int(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା val;
  Tcl_Size nIn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut;
  ଅଚଳ unsigned ଅକ୍ଷର *zIn;
  unsigned ଅକ୍ଷର *aOut;
  unsigned ଅକ୍ଷର a_ଅଙ୍କ[4];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bLittle = 0;

  ଯଦି( objc==3 ){
    Tcl_Size n;
    ଅକ୍ଷର *z = Tcl_GetStringFromObj(objv[1], &n);
    ଯଦି( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ଯଦି( (objc-bLittle)!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zIn = (ଅଚଳ unsigned ଅକ୍ଷର *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  ଯଦି( aOut==0 ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nIn, aOut);
  ଯଦି( nOut>=4 ){
    memcpy(a_ଅଙ୍କ, aOut, 4);
  }ଅନ୍ୟଥା{
    memset(a_ଅଙ୍କ, 0, ଆକାର(a_ଅଙ୍କ));
    memcpy(&a_ଅଙ୍କ[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  ଯଦି( bLittle ){
    val = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((u32)a_ଅଙ୍କ[3]<<24) | (a_ଅଙ୍କ[2]<<16) | (a_ଅଙ୍କ[1]<<8) | a_ଅଙ୍କ[0];
  }ଅନ୍ୟଥା{
    val = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((u32)a_ଅଙ୍କ[0]<<24) | (a_ଅଙ୍କ[1]<<16) | (a_ଅଙ୍କ[2]<<8) | a_ଅଙ୍କ[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI hexio_render_int16(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା val;
  unsigned ଅକ୍ଷର a_ଅଙ୍କ[10];

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "INTEGER");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &val) ) ଫେରନ୍ତୁ TCL_ERROR;
  a_ଅଙ୍କ[0] = val>>8;
  a_ଅଙ୍କ[1] = val;
  sqlite3TestBinToHex(a_ଅଙ୍କ, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((ଅକ୍ଷର*)a_ଅଙ୍କ, 4));
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI hexio_render_int32(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା val;
  unsigned ଅକ୍ଷର a_ଅଙ୍କ[10];

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "INTEGER");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &val) ) ଫେରନ୍ତୁ TCL_ERROR;
  a_ଅଙ୍କ[0] = val>>24;
  a_ଅଙ୍କ[1] = val>>16;
  a_ଅଙ୍କ[2] = val>>8;
  a_ଅଙ୍କ[3] = val;
  sqlite3TestBinToHex(a_ଅଙ୍କ, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((ଅକ୍ଷର*)a_ଅଙ୍କ, 8));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI utf8_to_utf8(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut;
  ଅଚଳ unsigned ଅକ୍ଷର *zOrig;
  unsigned ଅକ୍ଷର *z;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HEX");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zOrig = (unsigned ଅକ୍ଷର *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (ଅକ୍ଷର*)z, ଖାଲି);
  sqlite3_free(z);
  ଫେରନ୍ତୁ TCL_OK;
#ଅନ୍ୟଥା
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ଖାଲି
  );
  ଫେରନ୍ତୁ TCL_ERROR;
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getFts3Varint(ଅଚଳ ଅକ୍ଷର *p, sqlite_int64 *v){
  ଅଚଳ unsigned ଅକ୍ଷର *q = (ଅଚଳ unsigned ଅକ୍ଷର *) p;
  sqlite_uint64 x = 0, y = 1;
  ଯେତେବେଳେ( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (q - (unsigned ଅକ୍ଷର *)p);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା putFts3Varint(ଅକ୍ଷର *p, sqlite_int64 v){
  unsigned ଅକ୍ଷର *q = (unsigned ଅକ୍ଷର *) p;
  sqlite_uint64 vu = v;
  କର{
    *q++ = (unsigned ଅକ୍ଷର) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }ଯେତେବେଳେ( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  ପ୍ରତିଜ୍ଞା( q - (unsigned ଅକ୍ଷର *)p <= 10 );
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (q - (unsigned ଅକ୍ଷର *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI read_fts3varint(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned ଅକ୍ଷର *zBlob;
  sqlite3_int64 iVal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVal;

  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BLOB VARNAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((ଅକ୍ଷର*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI make_fts3record(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned ଅକ୍ଷର *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "LIST");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ପାଇଁ(i=0; i<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nArg; i++){
    Tcl_WideInt iVal;
    ଯଦି( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ଯଦି( nOut+10>nAlloc ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNew = nAlloc?nAlloc*2:128;
        unsigned ଅକ୍ଷର *aNew = sqlite3_realloc(aOut, nNew);
        ଯଦି( aNew==0 ){
          sqlite3_free(aOut);
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((ଅକ୍ଷର*)&aOut[nOut], iVal);
    }ଅନ୍ୟଥା{
      Tcl_Size nVal = 0;
      ଅକ୍ଷର *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      ଯେତେବେଳେ( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned ଅକ୍ଷର *aNew = sqlite3_realloc64(aOut, nNew);
        ଯଦି( aNew==0 ){
          sqlite3_free(aOut);
          ଫେରନ୍ତୁ TCL_ERROR;
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
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}
