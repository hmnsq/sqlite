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
शून्य sqlite3_Test_Bin_गन्तव्यम्_Hex(अचिह्नित वर्ण *zBuf, पूर्णाङ्क N){
  ध्रुव अचिह्नित वर्ण zHex[] = "0123456789ABCDEF";
  पूर्णाङ्क i, j;
  अचिह्नित वर्ण c;
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
पूर्णाङ्क sqlite3_Test_Hex_गन्तव्यम्_Bin(ध्रुव अचिह्नित वर्ण *z_अन्तर्, पूर्णाङ्क N, अचिह्नित वर्ण *a_बहिर्गमनम्){
  ध्रुव अचिह्नित वर्ण aMap[] = {
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
  अचिह्नित वर्ण c;

  प्रति(i=j=0; i<N; i++){
    c = aMap[z_अन्तर्[i]];
    यदि( c==0 ) अनुवर्त;
    यदि( hi ){
      a_बहिर्गमनम्[j] = (c-1)<<4;
      hi = 0;
    }अन्यथा{
      a_बहिर्गमनम्[j++] |= c-1;
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क विस्थापनम्;
  पूर्णाङ्क amt, got;
  ध्रुव वर्ण *zFile;
  अचिह्नित वर्ण *zBuf;
  FILE *अन्तर्;

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME OFFSET AMT");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &विस्थापनम्) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &amt) ) प्रत्यागम TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  यदि( zBuf==0 ){
    प्रत्यागम TCL_ERROR;
  }
  अन्तर् = fopen(zFile, "rb");
  यदि( अन्तर्==0 ){
    अन्तर् = fopen(zFile, "r");
  }
  यदि( अन्तर्==0 ){
    Tcl_Append_परिणाम(interp, "cannot open input file ", zFile, रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  fseek(अन्तर्, विस्थापनम्, SEEK_SET);
  got = (पूर्णाङ्क)fread(zBuf, 1, amt, अन्तर्);
  fclose(अन्तर्);
  यदि( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_गन्तव्यम्_Hex(zBuf, got);
  Tcl_Append_परिणाम(interp, zBuf, रिक्त);
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क विस्थापनम्;
  Tcl_आकार n_अन्तर्;
  पूर्णाङ्क n_बहिर्गमनम्, written;
  ध्रुव वर्ण *zFile;
  ध्रुव अचिह्नित वर्ण *z_अन्तर्;
  अचिह्नित वर्ण *a_बहिर्गमनम्;
  FILE *बहिर्गमनम्;

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &विस्थापनम्) ) प्रत्यागम TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_अन्तर् = (ध्रुव अचिह्नित वर्ण *)Tcl_GetStringFromObj(objv[3], &n_अन्तर्);
  a_बहिर्गमनम् = sqlite3_malloc64( 1 + n_अन्तर्/2 );
  यदि( a_बहिर्गमनम्==0 ){
    प्रत्यागम TCL_ERROR;
  }
  n_बहिर्गमनम् = sqlite3_Test_Hex_गन्तव्यम्_Bin(z_अन्तर्, (पूर्णाङ्क)n_अन्तर्, a_बहिर्गमनम्);
  बहिर्गमनम् = fopen(zFile, "r+b");
  यदि( बहिर्गमनम्==0 ){
    बहिर्गमनम् = fopen(zFile, "r+");
  }
  यदि( बहिर्गमनम्==0 ){
    Tcl_Append_परिणाम(interp, "cannot open output file ", zFile, रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  fseek(बहिर्गमनम्, विस्थापनम्, SEEK_SET);
  written = (पूर्णाङ्क)fwrite(a_बहिर्गमनम्, 1, n_बहिर्गमनम्, बहिर्गमनम्);
  sqlite3_free(a_बहिर्गमनम्);
  fclose(बहिर्गमनम्);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(written));
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क val;
  Tcl_आकार n_अन्तर्;
  पूर्णाङ्क n_बहिर्गमनम्;
  ध्रुव अचिह्नित वर्ण *z_अन्तर्;
  अचिह्नित वर्ण *a_बहिर्गमनम्;
  अचिह्नित वर्ण a_अङ्कः[4];
  पूर्णाङ्क bLittle = 0;

  यदि( objc==3 ){
    Tcl_आकार n;
    वर्ण *z = Tcl_GetStringFromObj(objv[1], &n);
    यदि( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  यदि( (objc-bLittle)!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "[-littleendian] HEXDATA");
    प्रत्यागम TCL_ERROR;
  }
  z_अन्तर् = (ध्रुव अचिह्नित वर्ण *)Tcl_GetStringFromObj(objv[1+bLittle], &n_अन्तर्);
  a_बहिर्गमनम् = sqlite3_malloc64( 1 + n_अन्तर्/2 );
  यदि( a_बहिर्गमनम्==0 ){
    प्रत्यागम TCL_ERROR;
  }
  n_बहिर्गमनम् = sqlite3_Test_Hex_गन्तव्यम्_Bin(z_अन्तर्, (पूर्णाङ्क)n_अन्तर्, a_बहिर्गमनम्);
  यदि( n_बहिर्गमनम्>=4 ){
    स्मृतिं_प्रतिलिख(a_अङ्कः, a_बहिर्गमनम्, 4);
  }अन्यथा{
    स्मृतिं_पूरय(a_अङ्कः, 0, परिमाण(a_अङ्कः));
    स्मृतिं_प्रतिलिख(&a_अङ्कः[4-n_बहिर्गमनम्], a_बहिर्गमनम्, n_बहिर्गमनम्);
  }
  sqlite3_free(a_बहिर्गमनम्);
  यदि( bLittle ){
    val = (पूर्णाङ्क)((u32)a_अङ्कः[3]<<24) | (a_अङ्कः[2]<<16) | (a_अङ्कः[1]<<8) | a_अङ्कः[0];
  }अन्यथा{
    val = (पूर्णाङ्क)((u32)a_अङ्कः[0]<<24) | (a_अङ्कः[1]<<16) | (a_अङ्कः[2]<<8) | a_अङ्कः[3];
  }
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(val));
  प्रत्यागम TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_render_int16(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क val;
  अचिह्नित वर्ण a_अङ्कः[10];

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "INTEGER");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &val) ) प्रत्यागम TCL_ERROR;
  a_अङ्कः[0] = val>>8;
  a_अङ्कः[1] = val;
  sqlite3_Test_Bin_गन्तव्यम्_Hex(a_अङ्कः, 2);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj((वर्ण*)a_अङ्कः, 4));
  प्रत्यागम TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI hexio_render_int32(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क val;
  अचिह्नित वर्ण a_अङ्कः[10];

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "INTEGER");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &val) ) प्रत्यागम TCL_ERROR;
  a_अङ्कः[0] = val>>24;
  a_अङ्कः[1] = val>>16;
  a_अङ्कः[2] = val>>8;
  a_अङ्कः[3] = val;
  sqlite3_Test_Bin_गन्तव्यम्_Hex(a_अङ्कः, 4);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj((वर्ण*)a_अङ्कः, 8));
  प्रत्यागम TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI utf8_गन्तव्यम्_utf8(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#यदि_परिभाषितम् SQLITE_DEBUG
  Tcl_आकार n;
  पूर्णाङ्क n_बहिर्गमनम्;
  ध्रुव अचिह्नित वर्ण *zOrig;
  अचिह्नित वर्ण *z;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HEX");
    प्रत्यागम TCL_ERROR;
  }
  zOrig = (अचिह्नित वर्ण *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_गन्तव्यम्_Bin(zOrig, (पूर्णाङ्क)n, z);
  z[n] = 0;
  n_बहिर्गमनम् = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_गन्तव्यम्_Hex(z,n_बहिर्गमनम्);
  Tcl_Append_परिणाम(interp, (वर्ण*)z, रिक्त);
  sqlite3_free(z);
  प्रत्यागम TCL_OK;
#अन्यथा
  Tcl_Append_परिणाम(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", रिक्त
  );
  प्रत्यागम TCL_ERROR;
#यदि_अन्तः
}

स्थिर पूर्णाङ्क getFts3Varint(ध्रुव वर्ण *p, sqlite_int64 *v){
  ध्रुव अचिह्नित वर्ण *q = (ध्रुव अचिह्नित वर्ण *) p;
  sqlite_uint64 x = 0, y = 1;
  यावत्( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  प्रत्यागम (पूर्णाङ्क) (q - (अचिह्नित वर्ण *)p);
}

स्थिर पूर्णाङ्क putFts3Varint(वर्ण *p, sqlite_int64 v){
  अचिह्नित वर्ण *q = (अचिह्नित वर्ण *) p;
  sqlite_uint64 vu = v;
  कुरु{
    *q++ = (अचिह्नित वर्ण) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }यावत्( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  प्रतिज्ञा( q - (अचिह्नित वर्ण *)p <= 10 );
  प्रत्यागम (पूर्णाङ्क) (q - (अचिह्नित वर्ण *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI read_fts3varint(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_आकार nBlob;
  अचिह्नित वर्ण *zBlob;
  sqlite3_int64 iVal;
  पूर्णाङ्क nVal;

  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BLOB VARNAME");
    प्रत्यागम TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((वर्ण*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_समुच्चयः_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(nVal));
  प्रत्यागम TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI make_fts3record(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_आकार nArg = 0;
  अचिह्नित वर्ण *a_बहिर्गमनम् = 0;
  sqlite3_int64 n_बहिर्गमनम् = 0;
  sqlite3_int64 nAlloc = 0;
  पूर्णाङ्क i;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "LIST");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    प्रत्यागम TCL_ERROR;
  }

  प्रति(i=0; i<(पूर्णाङ्क)nArg; i++){
    Tcl_WideInt iVal;
    यदि( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      यदि( n_बहिर्गमनम्+10>nAlloc ){
        पूर्णाङ्क nNew = nAlloc?nAlloc*2:128;
        अचिह्नित वर्ण *aNew = sqlite3_realloc(a_बहिर्गमनम्, nNew);
        यदि( aNew==0 ){
          sqlite3_free(a_बहिर्गमनम्);
          प्रत्यागम TCL_ERROR;
        }
        a_बहिर्गमनम् = aNew;
        nAlloc = nNew;
      }
      n_बहिर्गमनम् += putFts3Varint((वर्ण*)&a_बहिर्गमनम्[n_बहिर्गमनम्], iVal);
    }अन्यथा{
      Tcl_आकार nVal = 0;
      वर्ण *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      यावत्( (n_बहिर्गमनम् + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        अचिह्नित वर्ण *aNew = sqlite3_realloc64(a_बहिर्गमनम्, nNew);
        यदि( aNew==0 ){
          sqlite3_free(a_बहिर्गमनम्);
          प्रत्यागम TCL_ERROR;
        }
        a_बहिर्गमनम् = aNew;
        nAlloc = nNew;
      }
      स्मृतिं_प्रतिलिख(&a_बहिर्गमनम्[n_बहिर्गमनम्], zVal, nVal);
      n_बहिर्गमनम् += nVal;
    }
  }

  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewByteArrayObj(a_बहिर्गमनम्, n_बहिर्गमनम्));
  sqlite3_free(a_बहिर्गमनम्);
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
     { "utf8_to_utf8",                 utf8_गन्तव्यम्_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  प्रत्यागम TCL_OK;
}
