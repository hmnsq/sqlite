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
शून्य sqlite3TestBinToHex(अचिन्हित अक्षर *zBuf, पूर्णांक N){
  स्थिरांक अचिन्हित अक्षर zHex[] = "0123456789ABCDEF";
  पूर्णांक i, j;
  अचिन्हित अक्षर c;
  i = N*2;
  zBuf[i--] = 0;
  साठी(j=N-1; j>=0; j--){
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
पूर्णांक sqlite3TestHexToBin(स्थिरांक अचिन्हित अक्षर *zIn, पूर्णांक N, अचिन्हित अक्षर *aOut){
  स्थिरांक अचिन्हित अक्षर aMap[] = {
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
  पूर्णांक i, j;
  पूर्णांक hi=1;
  अचिन्हित अक्षर c;

  साठी(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    जर( c==0 ) सुरूठेवा;
    जर( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }अन्यथा{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  परत j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
स्थिर पूर्णांक SQLITE_TCLAPI hexio_read(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक offset;
  पूर्णांक amt, got;
  स्थिरांक अक्षर *zFile;
  अचिन्हित अक्षर *zBuf;
  खाता *in;

  जर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[2], &offset) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[3], &amt) ) परत TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  जर( zBuf==0 ){
    परत TCL_ERROR;
  }
  in = ख_खोलो(zFile, "rb");
  जर( in==0 ){
    in = ख_खोलो(zFile, "r");
  }
  जर( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, रिक्त);
    परत TCL_ERROR;
  }
  ख_जाओ(in, offset, शुरू_से);
  got = (पूर्णांक)ख_पढ़ो(zBuf, 1, amt, in);
  ख_बंद(in);
  जर( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  sqlite3_free(zBuf);
  परत TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
स्थिर पूर्णांक SQLITE_TCLAPI hexio_write(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक offset;
  Tcl_Size nIn;
  पूर्णांक nOut, written;
  स्थिरांक अक्षर *zFile;
  स्थिरांक अचिन्हित अक्षर *zIn;
  अचिन्हित अक्षर *aOut;
  खाता *out;

  जर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[2], &offset) ) परत TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (स्थिरांक अचिन्हित अक्षर *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  जर( aOut==0 ){
    परत TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (पूर्णांक)nIn, aOut);
  out = ख_खोलो(zFile, "r+b");
  जर( out==0 ){
    out = ख_खोलो(zFile, "r+");
  }
  जर( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, रिक्त);
    परत TCL_ERROR;
  }
  ख_जाओ(out, offset, शुरू_से);
  written = (पूर्णांक)ख_डालो(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  ख_बंद(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  परत TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
स्थिर पूर्णांक SQLITE_TCLAPI hexio_get_int(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक val;
  Tcl_Size nIn;
  पूर्णांक nOut;
  स्थिरांक अचिन्हित अक्षर *zIn;
  अचिन्हित अक्षर *aOut;
  अचिन्हित अक्षर a_अंक[4];
  पूर्णांक bLittle = 0;

  जर( objc==3 ){
    Tcl_Size n;
    अक्षर *z = Tcl_GetStringFromObj(objv[1], &n);
    जर( n>=2 && n<=13 && स_भेद(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  जर( (objc-bLittle)!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    परत TCL_ERROR;
  }
  zIn = (स्थिरांक अचिन्हित अक्षर *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  जर( aOut==0 ){
    परत TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (पूर्णांक)nIn, aOut);
  जर( nOut>=4 ){
    स_नकल(a_अंक, aOut, 4);
  }अन्यथा{
    स_रखो(a_अंक, 0, आकार(a_अंक));
    स_नकल(&a_अंक[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  जर( bLittle ){
    val = (पूर्णांक)((u32)a_अंक[3]<<24) | (a_अंक[2]<<16) | (a_अंक[1]<<8) | a_अंक[0];
  }अन्यथा{
    val = (पूर्णांक)((u32)a_अंक[0]<<24) | (a_अंक[1]<<16) | (a_अंक[2]<<8) | a_अंक[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  परत TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
स्थिर पूर्णांक SQLITE_TCLAPI hexio_render_int16(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक val;
  अचिन्हित अक्षर a_अंक[10];

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "INTEGER");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &val) ) परत TCL_ERROR;
  a_अंक[0] = val>>8;
  a_अंक[1] = val;
  sqlite3TestBinToHex(a_अंक, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((अक्षर*)a_अंक, 4));
  परत TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
स्थिर पूर्णांक SQLITE_TCLAPI hexio_render_int32(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक val;
  अचिन्हित अक्षर a_अंक[10];

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "INTEGER");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &val) ) परत TCL_ERROR;
  a_अंक[0] = val>>24;
  a_अंक[1] = val>>16;
  a_अंक[2] = val>>8;
  a_अंक[3] = val;
  sqlite3TestBinToHex(a_अंक, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((अक्षर*)a_अंक, 8));
  परत TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
स्थिर पूर्णांक SQLITE_TCLAPI utf8_to_utf8(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  पूर्णांक nOut;
  स्थिरांक अचिन्हित अक्षर *zOrig;
  अचिन्हित अक्षर *z;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HEX");
    परत TCL_ERROR;
  }
  zOrig = (अचिन्हित अक्षर *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (पूर्णांक)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (अक्षर*)z, रिक्त);
  sqlite3_free(z);
  परत TCL_OK;
#अन्यथा
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", रिक्त
  );
  परत TCL_ERROR;
#endif
}

स्थिर पूर्णांक getFts3Varint(स्थिरांक अक्षर *p, sqlite_int64 *v){
  स्थिरांक अचिन्हित अक्षर *q = (स्थिरांक अचिन्हित अक्षर *) p;
  sqlite_uint64 x = 0, y = 1;
  पर्यंत( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  परत (पूर्णांक) (q - (अचिन्हित अक्षर *)p);
}

स्थिर पूर्णांक putFts3Varint(अक्षर *p, sqlite_int64 v){
  अचिन्हित अक्षर *q = (अचिन्हित अक्षर *) p;
  sqlite_uint64 vu = v;
  करा{
    *q++ = (अचिन्हित अक्षर) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }पर्यंत( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  प्रतिज्ञा( q - (अचिन्हित अक्षर *)p <= 10 );
  परत (पूर्णांक) (q - (अचिन्हित अक्षर *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
स्थिर पूर्णांक SQLITE_TCLAPI read_fts3varint(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  अचिन्हित अक्षर *zBlob;
  sqlite3_int64 iVal;
  पूर्णांक nVal;

  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BLOB VARNAME");
    परत TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((अक्षर*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  परत TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
स्थिर पूर्णांक SQLITE_TCLAPI make_fts3record(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  अचिन्हित अक्षर *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  पूर्णांक i;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "LIST");
    परत TCL_ERROR;
  }
  जर( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    परत TCL_ERROR;
  }

  साठी(i=0; i<(पूर्णांक)nArg; i++){
    Tcl_WideInt iVal;
    जर( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      जर( nOut+10>nAlloc ){
        पूर्णांक nNew = nAlloc?nAlloc*2:128;
        अचिन्हित अक्षर *aNew = sqlite3_realloc(aOut, nNew);
        जर( aNew==0 ){
          sqlite3_free(aOut);
          परत TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((अक्षर*)&aOut[nOut], iVal);
    }अन्यथा{
      Tcl_Size nVal = 0;
      अक्षर *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      पर्यंत( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        अचिन्हित अक्षर *aNew = sqlite3_realloc64(aOut, nNew);
        जर( aNew==0 ){
          sqlite3_free(aOut);
          परत TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      स_नकल(&aOut[nOut], zVal, nVal);
      nOut += nVal;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(aOut, nOut));
  sqlite3_free(aOut);
  परत TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_hexio_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     अक्षर *zName;
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
  पूर्णांक i;
  साठी(i=0; i<आकार(aObjCmd)/आकार(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  परत TCL_OK;
}
