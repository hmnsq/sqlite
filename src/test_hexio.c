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
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ "tclsqlite.h"
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
শূন্য sqlite3TestBinToHex(unsigned ৱর্ণ *zBuf, পূর্ণাঙ্ক N){
  ধ্রুৱ unsigned ৱর্ণ zHex[] = "0123456789ABCDEF";
  পূর্ণাঙ্ক i, j;
  unsigned ৱর্ণ c;
  i = N*2;
  zBuf[i--] = 0;
  বাবে(j=N-1; j>=0; j--){
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
পূর্ণাঙ্ক sqlite3TestHexToBin(ধ্রুৱ unsigned ৱর্ণ *zIn, পূর্ণাঙ্ক N, unsigned ৱর্ণ *aOut){
  ধ্রুৱ unsigned ৱর্ণ aMap[] = {
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
  পূর্ণাঙ্ক i, j;
  পূর্ণাঙ্ক hi=1;
  unsigned ৱর্ণ c;

  বাবে(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    যদি( c==0 ) অনুৱর্ত;
    যদি( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }অন্যথা{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  ঘূৰাই j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI hexio_read(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক offset;
  পূর্ণাঙ্ক amt, got;
  ধ্রুৱ ৱর্ণ *zFile;
  unsigned ৱর্ণ *zBuf;
  FILE *in;

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME OFFSET AMT");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ঘূৰাই TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  যদি( zBuf==0 ){
    ঘূৰাই TCL_ERROR;
  }
  in = fopen(zFile, "rb");
  যদি( in==0 ){
    in = fopen(zFile, "r");
  }
  যদি( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", zFile, রিক্ত);
    ঘূৰাই TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = (পূর্ণাঙ্ক)fread(zBuf, 1, amt, in);
  fclose(in);
  যদি( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, রিক্ত);
  sqlite3_free(zBuf);
  ঘূৰাই TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI hexio_write(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক offset;
  Tcl_Size nIn;
  পূর্ণাঙ্ক nOut, written;
  ধ্রুৱ ৱর্ণ *zFile;
  ধ্রুৱ unsigned ৱর্ণ *zIn;
  unsigned ৱর্ণ *aOut;
  FILE *out;

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &offset) ) ঘূৰাই TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (ধ্রুৱ unsigned ৱর্ণ *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  যদি( aOut==0 ){
    ঘূৰাই TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (পূর্ণাঙ্ক)nIn, aOut);
  out = fopen(zFile, "r+b");
  যদি( out==0 ){
    out = fopen(zFile, "r+");
  }
  যদি( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", zFile, রিক্ত);
    ঘূৰাই TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = (পূর্ণাঙ্ক)fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  ঘূৰাই TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI hexio_get_int(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক val;
  Tcl_Size nIn;
  পূর্ণাঙ্ক nOut;
  ধ্রুৱ unsigned ৱর্ণ *zIn;
  unsigned ৱর্ণ *aOut;
  unsigned ৱর্ণ a_অঙ্ক[4];
  পূর্ণাঙ্ক bLittle = 0;

  যদি( objc==3 ){
    Tcl_Size n;
    ৱর্ণ *z = Tcl_GetStringFromObj(objv[1], &n);
    যদি( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  যদি( (objc-bLittle)!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "[-littleendian] HEXDATA");
    ঘূৰাই TCL_ERROR;
  }
  zIn = (ধ্রুৱ unsigned ৱর্ণ *)Tcl_GetStringFromObj(objv[1+bLittle], &nIn);
  aOut = sqlite3_malloc64( 1 + nIn/2 );
  যদি( aOut==0 ){
    ঘূৰাই TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, (পূর্ণাঙ্ক)nIn, aOut);
  যদি( nOut>=4 ){
    memcpy(a_অঙ্ক, aOut, 4);
  }অন্যথা{
    memset(a_অঙ্ক, 0, পরিমাণ(a_অঙ্ক));
    memcpy(&a_অঙ্ক[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  যদি( bLittle ){
    val = (পূর্ণাঙ্ক)((u32)a_অঙ্ক[3]<<24) | (a_অঙ্ক[2]<<16) | (a_অঙ্ক[1]<<8) | a_অঙ্ক[0];
  }অন্যথা{
    val = (পূর্ণাঙ্ক)((u32)a_অঙ্ক[0]<<24) | (a_অঙ্ক[1]<<16) | (a_অঙ্ক[2]<<8) | a_অঙ্ক[3];
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  ঘূৰাই TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI hexio_render_int16(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক val;
  unsigned ৱর্ণ a_অঙ্ক[10];

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "INTEGER");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &val) ) ঘূৰাই TCL_ERROR;
  a_অঙ্ক[0] = val>>8;
  a_অঙ্ক[1] = val;
  sqlite3TestBinToHex(a_অঙ্ক, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((ৱর্ণ*)a_অঙ্ক, 4));
  ঘূৰাই TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI hexio_render_int32(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক val;
  unsigned ৱর্ণ a_অঙ্ক[10];

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "INTEGER");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &val) ) ঘূৰাই TCL_ERROR;
  a_অঙ্ক[0] = val>>24;
  a_অঙ্ক[1] = val>>16;
  a_অঙ্ক[2] = val>>8;
  a_অঙ্ক[3] = val;
  sqlite3TestBinToHex(a_অঙ্ক, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((ৱর্ণ*)a_অঙ্ক, 8));
  ঘূৰাই TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI utf8_to_utf8(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  Tcl_Size n;
  পূর্ণাঙ্ক nOut;
  ধ্রুৱ unsigned ৱর্ণ *zOrig;
  unsigned ৱর্ণ *z;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HEX");
    ঘূৰাই TCL_ERROR;
  }
  zOrig = (unsigned ৱর্ণ *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3TestHexToBin(zOrig, (পূর্ণাঙ্ক)n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (ৱর্ণ*)z, রিক্ত);
  sqlite3_free(z);
  ঘূৰাই TCL_OK;
#অন্যথা
  Tcl_AppendResult(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", রিক্ত
  );
  ঘূৰাই TCL_ERROR;
#endif
}

স্থির পূর্ণাঙ্ক getFts3Varint(ধ্রুৱ ৱর্ণ *p, sqlite_int64 *v){
  ধ্রুৱ unsigned ৱর্ণ *q = (ধ্রুৱ unsigned ৱর্ণ *) p;
  sqlite_uint64 x = 0, y = 1;
  যেতিয়ালৈ( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ঘূৰাই (পূর্ণাঙ্ক) (q - (unsigned ৱর্ণ *)p);
}

স্থির পূর্ণাঙ্ক putFts3Varint(ৱর্ণ *p, sqlite_int64 v){
  unsigned ৱর্ণ *q = (unsigned ৱর্ণ *) p;
  sqlite_uint64 vu = v;
  কৰা{
    *q++ = (unsigned ৱর্ণ) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }যেতিয়ালৈ( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  প্রতিজ্ঞা( q - (unsigned ৱর্ণ *)p <= 10 );
  ঘূৰাই (পূর্ণাঙ্ক) (q - (unsigned ৱর্ণ *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI read_fts3varint(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Size nBlob;
  unsigned ৱর্ণ *zBlob;
  sqlite3_int64 iVal;
  পূর্ণাঙ্ক nVal;

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BLOB VARNAME");
    ঘূৰাই TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((ৱর্ণ*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_ObjSetVar2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nVal));
  ঘূৰাই TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI make_fts3record(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_Size nArg = 0;
  unsigned ৱর্ণ *aOut = 0;
  sqlite3_int64 nOut = 0;
  sqlite3_int64 nAlloc = 0;
  পূর্ণাঙ্ক i;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "LIST");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ঘূৰাই TCL_ERROR;
  }

  বাবে(i=0; i<(পূর্ণাঙ্ক)nArg; i++){
    Tcl_WideInt iVal;
    যদি( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      যদি( nOut+10>nAlloc ){
        পূর্ণাঙ্ক nNew = nAlloc?nAlloc*2:128;
        unsigned ৱর্ণ *aNew = sqlite3_realloc(aOut, nNew);
        যদি( aNew==0 ){
          sqlite3_free(aOut);
          ঘূৰাই TCL_ERROR;
        }
        aOut = aNew;
        nAlloc = nNew;
      }
      nOut += putFts3Varint((ৱর্ণ*)&aOut[nOut], iVal);
    }অন্যথা{
      Tcl_Size nVal = 0;
      ৱর্ণ *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      যেতিয়ালৈ( (nOut + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        unsigned ৱর্ণ *aNew = sqlite3_realloc64(aOut, nNew);
        যদি( aNew==0 ){
          sqlite3_free(aOut);
          ঘূৰাই TCL_ERROR;
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
  ঘূৰাই TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetest_hexio_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     ৱর্ণ *zName;
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
  পূর্ণাঙ্ক i;
  বাবে(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ঘূৰাই TCL_OK;
}
