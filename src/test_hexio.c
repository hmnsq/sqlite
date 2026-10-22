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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
व्योम sqlite3TestBinToHex(अचिन्हित अक्षर *zBuf, पूर्णांक N){
  स्थिर अचिन्हित अक्षर zHex[] = "0123456789ABCDEF";
  पूर्णांक i, j;
  अचिन्हित अक्षर c;
  i = N*2;
  zBuf[i--] = 0;
  क्रम(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  निश्चित( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
पूर्णांक sqlite3TestHexToBin(स्थिर अचिन्हित अक्षर *zIn, पूर्णांक N, अचिन्हित अक्षर *aOut){
  स्थिर अचिन्हित अक्षर aMap[] = {
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

  क्रम(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    अगर( c==0 ) जारी;
    अगर( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }अन्यथा{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  वापस j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
अटल पूर्णांक SQLITE_TCLAPI hexio_read(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक offset;
  पूर्णांक amt, got;
  स्थिर अक्षर *zFile;
  अचिन्हित अक्षर *zBuf;
  खाता *in;

  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[2], &offset) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[3], &amt) ) वापस TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  अगर( zBuf==0 ){
    वापस TCL_ERROR;
  }
  in = ख_खोलो(zFile, "rb");
  अगर( in==0 ){
    in = ख_खोलो(zFile, "r");
  }
  अगर( in==0 ){
    Tcl_Append_परिणाम(interp, "cannot open input file ", zFile, शून्य);
    वापस TCL_ERROR;
  }
  ख_जाओ(in, offset, शुरू_से);
  got = (पूर्णांक)ख_पढ़ो(zBuf, 1, amt, in);
  ख_बंद(in);
  अगर( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  sqlite3_free(zBuf);
  वापस TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
अटल पूर्णांक SQLITE_TCLAPI hexio_write(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक offset;
  Tcl_आकार nIn;
  पूर्णांक nOut, written;
  स्थिर अक्षर *zFile;
  स्थिर अचिन्हित अक्षर *zIn;
  अचिन्हित अक्षर *aOut;
  खाता *out;

  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[2], &offset) ) वापस TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (स्थिर अचिन्हित अक्षर *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  अगर( aOut==0 ){
    वापस TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (पूर्णांक)nIn, aOut);
  out = ख_खोलो(zFile, "r+b");
  अगर( out==0 ){
    out = ख_खोलो(zFile, "r+");
  }
  अगर( out==0 ){
    Tcl_Append_परिणाम(interp, "cannot open output file ", zFile, शून्य);
    वापस TCL_ERROR;
  }
  ख_जाओ(out, offset, शुरू_से);
  written = (पूर्णांक)ख_डालो(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  ख_बंद(out);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(written));
  वापस TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
अटल पूर्णांक SQLITE_TCLAPI hexio_get_int(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक val;
  Tcl_आकार nIn;
  पूर्णांक nOut;
  स्थिर अचिन्हित अक्षर *zIn;
  अचिन्हित अक्षर *aOut;
  अचिन्हित अक्षर a_अंक[4];
  पूर्णांक bLittle = 0;

  अगर( objc==3 ){
    Tcl_आकार n;
    अक्षर *z = Tcl_GetStringFromObj(objv[1], &n);
    अगर( n>=2 && n<=13 && स_भेद(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  अगर( (objc-bLittle)!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    वापस TCL_ERROR;
  }
  zIn = (स्थिर अचिन्हित अक्षर *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  अगर( aOut==0 ){
    वापस TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (पूर्णांक)nIn, aOut);
  अगर( nOut>=4 ){
    स_नकल(a_अंक, aOut, 4);
  }अन्यथा{
    स_रखो(a_अंक, 0, माप(a_अंक));
    स_नकल(&a_अंक[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  अगर( bLittle ){
    val = (पूर्णांक)((u32)a_अंक[3]<<24) | (a_अंक[2]<<16) | (a_अंक[1]<<8) | a_अंक[0];
  }अन्यथा{
    val = (पूर्णांक)((u32)a_अंक[0]<<24) | (a_अंक[1]<<16) | (a_अंक[2]<<8) | a_अंक[3];
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(val));
  वापस TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
अटल पूर्णांक SQLITE_TCLAPI hexio_render_int16(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक val;
  अचिन्हित अक्षर a_अंक[10];

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "INTEGER");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &val) ) वापस TCL_ERROR;
  a_अंक[0] = val>>8;
  a_अंक[1] = val;
  sqlite3TestBinToHex(a_अंक, 2);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj((अक्षर*)a_अंक, 4));
  वापस TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
अटल पूर्णांक SQLITE_TCLAPI hexio_render_int32(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक val;
  अचिन्हित अक्षर a_अंक[10];

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "INTEGER");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &val) ) वापस TCL_ERROR;
  a_अंक[0] = val>>24;
  a_अंक[1] = val>>16;
  a_अंक[2] = val>>8;
  a_अंक[3] = val;
  sqlite3TestBinToHex(a_अंक, 4);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj((अक्षर*)a_अंक, 8));
  वापस TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
अटल पूर्णांक SQLITE_TCLAPI utf8_to_utf8(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#अगर_घोषित SQLITE_DEBUG
  Tcl_आकार n;
  पूर्णांक nOut;
  स्थिर अचिन्हित अक्षर *zOrig;
  अचिन्हित अक्षर *z;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HEX");
    वापस TCL_ERROR;
  }
  zOrig = (अचिन्हित अक्षर *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (पूर्णांक)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_Append_परिणाम(interp, (अक्षर*)z, शून्य);
  sqlite3_free(z);
  वापस TCL_OK;
#अन्यथा
  Tcl_Append_परिणाम(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", शून्य
  );
  वापस TCL_ERROR;
#पूर्ण_अगर
}

अटल पूर्णांक getFts3Varint(स्थिर अक्षर *p, sqlite_int64 *v){
  स्थिर अचिन्हित अक्षर *q = (स्थिर अचिन्हित अक्षर *) p;
  sqlite_uint64 x = 0, y = 1;
  जबतक( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  वापस (पूर्णांक) (q - (अचिन्हित अक्षर *)p);
}

अटल पूर्णांक putFts3Varint(अक्षर *p, sqlite_int64 v){
  अचिन्हित अक्षर *q = (अचिन्हित अक्षर *) p;
  sqlite_uint64 vu = v;
  करो{
    *q++ = (अचिन्हित अक्षर) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }जबतक( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  निश्चित( q - (अचिन्हित अक्षर *)p <= 10 );
  वापस (पूर्णांक) (q - (अचिन्हित अक्षर *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
अटल पूर्णांक SQLITE_TCLAPI read_fts3varint(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_आकार nBlob;
  अचिन्हित अक्षर *zBlob;
  sqlite3_int64 iVal;
  पूर्णांक nVal;

  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BLOB VARNAME");
    वापस TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((अक्षर*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(nVal));
  वापस TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
अटल पूर्णांक SQLITE_TCLAPI make_fts3record(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_आकार nArg = 0;
  अचिन्हित अक्षर *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  पूर्णांक i;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "LIST");
    वापस TCL_ERROR;
  }
  अगर( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    वापस TCL_ERROR;
  }

  क्रम(i=0; i<(पूर्णांक)nArg; i++){
    Tcl_WideInt iVal;
    अगर( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      अगर( nOut+10>nAlloc ){
        पूर्णांक nNew = nAlloc?nAlloc*2:128;
        अचिन्हित अक्षर *aNew = sqlite3_realloc(aOut, nNew);
        अगर( aNew==0 ){
          sqlite3_free(aOut);
          वापस TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((अक्षर*)&aOut[nOut], iVal);
    }अन्यथा{
      Tcl_आकार nVal = 0;
      अक्षर *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      जबतक( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        अचिन्हित अक्षर *aNew = sqlite3_realloc64(aOut, nNew);
        अगर( aNew==0 ){
          sqlite3_free(aOut);
          वापस TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      स_नकल(&aOut[nOut], zVal, nVal);
      nOut += nVal;
    }
  }

  Tcl_Set_Obj_परिणाम(interp, Tcl_NewByteArrayObj(aOut, nOut));
  sqlite3_free(aOut);
  वापस TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_hexio_Init(Tcl_Interp *interp){
  अटल काष्ठा {
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
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  वापस TCL_OK;
}
