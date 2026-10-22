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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
शून्य sqlite3TestBinToHex(unsigned वर्ण *zBuf, पूर्णाङ्क N){
  ध्रुव unsigned वर्ण zHex[] = "0123456789ABCDEF";
  पूर्णाङ्क i, j;
  unsigned वर्ण c;
  i = N*2;
  zBuf[i--] = 0;
  प्रति(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  प्रतिज्ञा( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
पूर्णाङ्क sqlite3TestHexToBin(ध्रुव unsigned वर्ण *zIn, पूर्णाङ्क N, unsigned वर्ण *aOut){
  ध्रुव unsigned वर्ण aMap[] = {
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
  पूर्णाङ्क i, j;
  पूर्णाङ्क hi=1;
  unsigned वर्ण c;

  प्रति(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    जइ( c==0 ) अनुवर्त;
    जइ( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }अण्णहा{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  प्रत्यागम j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_read(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क offset;
  पूर्णाङ्क amt, got;
  ध्रुव वर्ण *zFile;
  unsigned वर्ण *zBuf;
  FILE *in;

  जइ( objc!=4 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    प्रत्यागम TCL_ERROR;
  }
  जइ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) प्रत्यागम TCL_ERROR;
  जइ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) प्रत्यागम TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  जइ( zBuf==0 ){
    प्रत्यागम TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  जइ( in==0 ){
    in = fopen(zFile, "r");
  }
  जइ( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (पूर्णाङ्क)fread(zBuf, 1, amt, in);
  fclose(in);
  जइ( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  sqlite3_free(zBuf);
  प्रत्यागम TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_write(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क offset;
  Tcl_Size nIn;
  पूर्णाङ्क nOut, written;
  ध्रुव वर्ण *zFile;
  ध्रुव unsigned वर्ण *zIn;
  unsigned वर्ण *aOut;
  FILE *out;

  जइ( objc!=4 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    प्रत्यागम TCL_ERROR;
  }
  जइ( Tcl_GetIntFromObj(interp, objv[2], &offset) ) प्रत्यागम TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (ध्रुव unsigned वर्ण *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  जइ( aOut==0 ){
    प्रत्यागम TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (पूर्णाङ्क)nIn, aOut);
  out = fopen(zFile, "r+b");
  जइ( out==0 ){
    out = fopen(zFile, "r+");
  }
  जइ( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (पूर्णाङ्क)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  प्रत्यागम TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_get_int(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क val;
  Tcl_Size nIn;
  पूर्णाङ्क nOut;
  ध्रुव unsigned वर्ण *zIn;
  unsigned वर्ण *aOut;
  unsigned वर्ण a_अङ्कः[4];
  पूर्णाङ्क bLittle = 0;

  जइ( objc==3 ){
    Tcl_Size n;
    वर्ण *z = Tcl_GetStringFromObj(objv[1], &n);
    जइ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  जइ( (objc-bLittle)!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    प्रत्यागम TCL_ERROR;
  }
  zIn = (ध्रुव unsigned वर्ण *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  जइ( aOut==0 ){
    प्रत्यागम TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (पूर्णाङ्क)nIn, aOut);
  जइ( nOut>=4 ){
    memcpy(a_अङ्कः, aOut, 4);
  }अण्णहा{
    memset(a_अङ्कः, 0, परिमाण(a_अङ्कः));
    memcpy(&a_अङ्कः[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  जइ( bLittle ){
    val = (पूर्णाङ्क)((u32)a_अङ्कः[3]<<24) | (a_अङ्कः[2]<<16) | (a_अङ्कः[1]<<8) | a_अङ्कः[0];
  }अण्णहा{
    val = (पूर्णाङ्क)((u32)a_अङ्कः[0]<<24) | (a_अङ्कः[1]<<16) | (a_अङ्कः[2]<<8) | a_अङ्कः[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  प्रत्यागम TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_render_int16(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क val;
  unsigned वर्ण a_अङ्कः[10];

  जइ( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "INTEGER");
    प्रत्यागम TCL_ERROR;
  }
  जइ( Tcl_GetIntFromObj(interp, objv[1], &val) ) प्रत्यागम TCL_ERROR;
  a_अङ्कः[0] = val>>8;
  a_अङ्कः[1] = val;
  sqlite3TestBinToHex(a_अङ्कः, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((वर्ण*)a_अङ्कः, 4));
  प्रत्यागम TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_render_int32(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क val;
  unsigned वर्ण a_अङ्कः[10];

  जइ( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "INTEGER");
    प्रत्यागम TCL_ERROR;
  }
  जइ( Tcl_GetIntFromObj(interp, objv[1], &val) ) प्रत्यागम TCL_ERROR;
  a_अङ्कः[0] = val>>24;
  a_अङ्कः[1] = val>>16;
  a_अङ्कः[2] = val>>8;
  a_अङ्कः[3] = val;
  sqlite3TestBinToHex(a_अङ्कः, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((वर्ण*)a_अङ्कः, 8));
  प्रत्यागम TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI utf8_to_utf8(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  पूर्णाङ्क nOut;
  ध्रुव unsigned वर्ण *zOrig;
  unsigned वर्ण *z;
  जइ( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "HEX");
    प्रत्यागम TCL_ERROR;
  }
  zOrig = (unsigned वर्ण *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (पूर्णाङ्क)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (वर्ण*)z, रिक्त);
  sqlite3_free(z);
  प्रत्यागम TCL_OK;
#अण्णहा
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", रिक्त
  );
  प्रत्यागम TCL_ERROR;
#endif
}

स्थिर पूर्णाङ्क getFts3Varint(ध्रुव वर्ण *p, sqlite_int64 *v){
  ध्रुव unsigned वर्ण *q = (ध्रुव unsigned वर्ण *) p;
  sqlite_uint64 x = 0, y = 1;
  जाव( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  प्रत्यागम (पूर्णाङ्क) (q - (unsigned वर्ण *)p);
}

स्थिर पूर्णाङ्क putFts3Varint(वर्ण *p, sqlite_int64 v){
  unsigned वर्ण *q = (unsigned वर्ण *) p;
  sqlite_uint64 vu = v;
  करेहि{
    *q++ = (unsigned वर्ण) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }जाव( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  प्रतिज्ञा( q - (unsigned वर्ण *)p <= 10 );
  प्रत्यागम (पूर्णाङ्क) (q - (unsigned वर्ण *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI read_fts3varint(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned वर्ण *zBlob;
  sqlite3_int64 iVal;
  पूर्णाङ्क nVal;

  जइ( objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BLOB VARNAME");
    प्रत्यागम TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((वर्ण*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  प्रत्यागम TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI make_fts3record(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned वर्ण *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  पूर्णाङ्क i;

  जइ( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "LIST");
    प्रत्यागम TCL_ERROR;
  }
  जइ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    प्रत्यागम TCL_ERROR;
  }

  प्रति(i=0; i<(पूर्णाङ्क)nArg; i++){
    Tcl_WideInt iVal;
    जइ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      जइ( nOut+10>nAlloc ){
        पूर्णाङ्क nNew = nAlloc?nAlloc*2:128;
        unsigned वर्ण *aNew = sqlite3_realloc(aOut, nNew);
        जइ( aNew==0 ){
          sqlite3_free(aOut);
          प्रत्यागम TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((वर्ण*)&aOut[nOut], iVal);
    }अण्णहा{
      Tcl_Size nVal = 0;
      वर्ण *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      जाव( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned वर्ण *aNew = sqlite3_realloc64(aOut, nNew);
        जइ( aNew==0 ){
          sqlite3_free(aOut);
          प्रत्यागम TCL_ERROR;
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
  प्रत्यागम TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest_hexio_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
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
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  प्रत्यागम TCL_OK;
}
