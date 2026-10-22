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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
শূন্য sqlite3TestBinToHex(unsigned অক্ষর *zBuf, পূর্ণসংখ্যা N){
  ধ্রুবক unsigned অক্ষর zHex[] = "0123456789ABCDEF";
  পূর্ণসংখ্যা i, j;
  unsigned অক্ষর c;
  i = N*2;
  zBuf[i--] = 0;
  জন্য(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  প্রতিজ্ঞা( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
পূর্ণসংখ্যা sqlite3TestHexToBin(ধ্রুবক unsigned অক্ষর *zIn, পূর্ণসংখ্যা N, unsigned অক্ষর *aOut){
  ধ্রুবক unsigned অক্ষর aMap[] = {
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
  পূর্ণসংখ্যা i, j;
  পূর্ণসংখ্যা hi=1;
  unsigned অক্ষর c;

  জন্য(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    যদি( c==0 ) চালু;
    যদি( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }অন্যথা{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  ফেরত j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI hexio_read(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা offset;
  পূর্ণসংখ্যা amt, got;
  ধ্রুবক অক্ষর *zFile;
  unsigned অক্ষর *zBuf;
  FILE *in;

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ফেরত TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ফেরত TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  যদি( zBuf==0 ){
    ফেরত TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  যদি( in==0 ){
    in = fopen(zFile, "r");
  }
  যদি( in==0 ){
    Tcl_Append_ফলাফল(interp, "cannot open input file ", zFile, ফাঁকা);
    ফেরত TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (পূর্ণসংখ্যা)fread(zBuf, 1, amt, in);
  fclose(in);
  যদি( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  sqlite3_free(zBuf);
  ফেরত TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI hexio_write(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা offset;
  Tcl_আকার nIn;
  পূর্ণসংখ্যা nOut, written;
  ধ্রুবক অক্ষর *zFile;
  ধ্রুবক unsigned অক্ষর *zIn;
  unsigned অক্ষর *aOut;
  FILE *out;

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ফেরত TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (ধ্রুবক unsigned অক্ষর *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  যদি( aOut==0 ){
    ফেরত TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (পূর্ণসংখ্যা)nIn, aOut);
  out = fopen(zFile, "r+b");
  যদি( out==0 ){
    out = fopen(zFile, "r+");
  }
  যদি( out==0 ){
    Tcl_Append_ফলাফল(interp, "cannot open output file ", zFile, ফাঁকা);
    ফেরত TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (পূর্ণসংখ্যা)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(written));
  ফেরত TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI hexio_get_int(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা val;
  Tcl_আকার nIn;
  পূর্ণসংখ্যা nOut;
  ধ্রুবক unsigned অক্ষর *zIn;
  unsigned অক্ষর *aOut;
  unsigned অক্ষর a_অঙ্ক[4];
  পূর্ণসংখ্যা bLittle = 0;

  যদি( objc==3 ){
    Tcl_আকার n;
    অক্ষর *z = Tcl_GetStringFromObj(objv[1], &n);
    যদি( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  যদি( (objc-bLittle)!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    ফেরত TCL_ERROR;
  }
  zIn = (ধ্রুবক unsigned অক্ষর *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  যদি( aOut==0 ){
    ফেরত TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (পূর্ণসংখ্যা)nIn, aOut);
  যদি( nOut>=4 ){
    memcpy(a_অঙ্ক, aOut, 4);
  }অন্যথা{
    memset(a_অঙ্ক, 0, আকার(a_অঙ্ক));
    memcpy(&a_অঙ্ক[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  যদি( bLittle ){
    val = (পূর্ণসংখ্যা)((u32)a_অঙ্ক[3]<<24) | (a_অঙ্ক[2]<<16) | (a_অঙ্ক[1]<<8) | a_অঙ্ক[0];
  }অন্যথা{
    val = (পূর্ণসংখ্যা)((u32)a_অঙ্ক[0]<<24) | (a_অঙ্ক[1]<<16) | (a_অঙ্ক[2]<<8) | a_অঙ্ক[3];
  }
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(val));
  ফেরত TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI hexio_render_int16(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা val;
  unsigned অক্ষর a_অঙ্ক[10];

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "INTEGER");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &val) ) ফেরত TCL_ERROR;
  a_অঙ্ক[0] = val>>8;
  a_অঙ্ক[1] = val;
  sqlite3TestBinToHex(a_অঙ্ক, 2);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj((অক্ষর*)a_অঙ্ক, 4));
  ফেরত TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI hexio_render_int32(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা val;
  unsigned অক্ষর a_অঙ্ক[10];

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "INTEGER");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &val) ) ফেরত TCL_ERROR;
  a_অঙ্ক[0] = val>>24;
  a_অঙ্ক[1] = val>>16;
  a_অঙ্ক[2] = val>>8;
  a_অঙ্ক[3] = val;
  sqlite3TestBinToHex(a_অঙ্ক, 4);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj((অক্ষর*)a_অঙ্ক, 8));
  ফেরত TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI utf8_to_utf8(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_আকার n;
  পূর্ণসংখ্যা nOut;
  ধ্রুবক unsigned অক্ষর *zOrig;
  unsigned অক্ষর *z;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HEX");
    ফেরত TCL_ERROR;
  }
  zOrig = (unsigned অক্ষর *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (পূর্ণসংখ্যা)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_Append_ফলাফল(interp, (অক্ষর*)z, ফাঁকা);
  sqlite3_free(z);
  ফেরত TCL_OK;
#অন্যথা
  Tcl_Append_ফলাফল(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ফাঁকা
  );
  ফেরত TCL_ERROR;
#endif
}

স্থির পূর্ণসংখ্যা getFts3Varint(ধ্রুবক অক্ষর *p, sqlite_int64 *v){
  ধ্রুবক unsigned অক্ষর *q = (ধ্রুবক unsigned অক্ষর *) p;
  sqlite_uint64 x = 0, y = 1;
  যখন( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ফেরত (পূর্ণসংখ্যা) (q - (unsigned অক্ষর *)p);
}

স্থির পূর্ণসংখ্যা putFts3Varint(অক্ষর *p, sqlite_int64 v){
  unsigned অক্ষর *q = (unsigned অক্ষর *) p;
  sqlite_uint64 vu = v;
  করো{
    *q++ = (unsigned অক্ষর) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }যখন( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  প্রতিজ্ঞা( q - (unsigned অক্ষর *)p <= 10 );
  ফেরত (পূর্ণসংখ্যা) (q - (unsigned অক্ষর *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI read_fts3varint(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_আকার nBlob;
  unsigned অক্ষর *zBlob;
  sqlite3_int64 iVal;
  পূর্ণসংখ্যা nVal;

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BLOB VARNAME");
    ফেরত TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((অক্ষর*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(nVal));
  ফেরত TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI make_fts3record(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_আকার nArg = 0;
  unsigned অক্ষর *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  পূর্ণসংখ্যা i;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "LIST");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ফেরত TCL_ERROR;
  }

  জন্য(i=0; i<(পূর্ণসংখ্যা)nArg; i++){
    Tcl_WideInt iVal;
    যদি( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      যদি( nOut+10>nAlloc ){
        পূর্ণসংখ্যা nNew = nAlloc?nAlloc*2:128;
        unsigned অক্ষর *aNew = sqlite3_realloc(aOut, nNew);
        যদি( aNew==0 ){
          sqlite3_free(aOut);
          ফেরত TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((অক্ষর*)&aOut[nOut], iVal);
    }অন্যথা{
      Tcl_আকার nVal = 0;
      অক্ষর *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      যখন( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned অক্ষর *aNew = sqlite3_realloc64(aOut, nNew);
        যদি( aNew==0 ){
          sqlite3_free(aOut);
          ফেরত TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      memcpy(&aOut[nOut], zVal, nVal);
      nOut += nVal;
    }
  }

  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewByteArrayObj(aOut, nOut));
  sqlite3_free(aOut);
  ফেরত TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest_hexio_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
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
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ফেরত TCL_OK;
}
