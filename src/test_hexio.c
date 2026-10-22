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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
శూన్య sqlite3_Test_Bin_గమ్యం_Hex(గుర్తులేని అక్షరం *zBuf, పూర్ణాంకం N){
  స్థిరరాశి గుర్తులేని అక్షరం zHex[] = "0123456789ABCDEF";
  పూర్ణాంకం i, j;
  గుర్తులేని అక్షరం c;
  i = N*2;
  zBuf[i--] = 0;
  కొరకు(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  ప్రతిజ్ఞా( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
పూర్ణాంకం sqlite3_Test_Hex_గమ్యం_Bin(స్థిరరాశి గుర్తులేని అక్షరం *z_లోపల, పూర్ణాంకం N, గుర్తులేని అక్షరం *a_బహిర్గతం){
  స్థిరరాశి గుర్తులేని అక్షరం aMap[] = {
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
  పూర్ణాంకం i, j;
  పూర్ణాంకం hi=1;
  గుర్తులేని అక్షరం c;

  కొరకు(i=j=0; i<N; i++){
    c = aMap[z_లోపల[i]];
    ఒకవేళ( c==0 ) కొనసాగు;
    ఒకవేళ( hi ){
      a_బహిర్గతం[j] = (c-1)<<4;
      hi = 0;
    }లేకపోతే{
      a_బహిర్గతం[j++] |= c-1;
      hi = 1;
    }
  }
  తిరిగి j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI hexio_read(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం స్థానభ్రంశం;
  పూర్ణాంకం amt, got;
  స్థిరరాశి అక్షరం *zFile;
  గుర్తులేని అక్షరం *zBuf;
  FILE *లోపల;

  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME OFFSET AMT");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &స్థానభ్రంశం) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) తిరిగి TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ఒకవేళ( zBuf==0 ){
    తిరిగి TCL_ERROR;
  }
  లోపల = fopen(zFile, "rb");
  ఒకవేళ( లోపల==0 ){
    లోపల = fopen(zFile, "r");
  }
  ఒకవేళ( లోపల==0 ){
    Tcl_Append_ఫలితం(interp, "cannot open input file ", zFile, ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  fseek(లోపల, స్థానభ్రంశం, SEEK_SET);
  got = (పూర్ణాంకం)fread(zBuf, 1, amt, లోపల);
  fclose(లోపల);
  ఒకవేళ( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_గమ్యం_Hex(zBuf, got);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  sqlite3_free(zBuf);
  తిరిగి TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI hexio_write(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం స్థానభ్రంశం;
  Tcl_కొలత n_లోపల;
  పూర్ణాంకం n_బహిర్గతం, written;
  స్థిరరాశి అక్షరం *zFile;
  స్థిరరాశి గుర్తులేని అక్షరం *z_లోపల;
  గుర్తులేని అక్షరం *a_బహిర్గతం;
  FILE *బహిర్గతం;

  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &స్థానభ్రంశం) ) తిరిగి TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_లోపల = (స్థిరరాశి గుర్తులేని అక్షరం *)Tcl_GetStringFromObj(objv[3], &n_లోపల);
  a_బహిర్గతం = sqlite3_malloc64( 1 + n_లోపల/2 );
  ఒకవేళ( a_బహిర్గతం==0 ){
    తిరిగి TCL_ERROR;
  }
  n_బహిర్గతం = sqlite3_Test_Hex_గమ్యం_Bin(z_లోపల, (పూర్ణాంకం)n_లోపల, a_బహిర్గతం);
  బహిర్గతం = fopen(zFile, "r+b");
  ఒకవేళ( బహిర్గతం==0 ){
    బహిర్గతం = fopen(zFile, "r+");
  }
  ఒకవేళ( బహిర్గతం==0 ){
    Tcl_Append_ఫలితం(interp, "cannot open output file ", zFile, ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  fseek(బహిర్గతం, స్థానభ్రంశం, SEEK_SET);
  written = (పూర్ణాంకం)fwrite(a_బహిర్గతం, 1, n_బహిర్గతం, బహిర్గతం);
  sqlite3_free(a_బహిర్గతం);
  fclose(బహిర్గతం);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(written));
  తిరిగి TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI hexio_get_int(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం val;
  Tcl_కొలత n_లోపల;
  పూర్ణాంకం n_బహిర్గతం;
  స్థిరరాశి గుర్తులేని అక్షరం *z_లోపల;
  గుర్తులేని అక్షరం *a_బహిర్గతం;
  గుర్తులేని అక్షరం a_అంకె[4];
  పూర్ణాంకం bLittle = 0;

  ఒకవేళ( objc==3 ){
    Tcl_కొలత n;
    అక్షరం *z = Tcl_GetStringFromObj(objv[1], &n);
    ఒకవేళ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ఒకవేళ( (objc-bLittle)!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "[-littleendian] HEXDATA");
    తిరిగి TCL_ERROR;
  }
  z_లోపల = (స్థిరరాశి గుర్తులేని అక్షరం *)Tcl_GetStringFromObj(objv[1+bLittle], &n_లోపల);
  a_బహిర్గతం = sqlite3_malloc64( 1 + n_లోపల/2 );
  ఒకవేళ( a_బహిర్గతం==0 ){
    తిరిగి TCL_ERROR;
  }
  n_బహిర్గతం = sqlite3_Test_Hex_గమ్యం_Bin(z_లోపల, (పూర్ణాంకం)n_లోపల, a_బహిర్గతం);
  ఒకవేళ( n_బహిర్గతం>=4 ){
    జ్ఞాపకం_నకలు(a_అంకె, a_బహిర్గతం, 4);
  }లేకపోతే{
    memset(a_అంకె, 0, పరిమాణం(a_అంకె));
    జ్ఞాపకం_నకలు(&a_అంకె[4-n_బహిర్గతం], a_బహిర్గతం, n_బహిర్గతం);
  }
  sqlite3_free(a_బహిర్గతం);
  ఒకవేళ( bLittle ){
    val = (పూర్ణాంకం)((u32)a_అంకె[3]<<24) | (a_అంకె[2]<<16) | (a_అంకె[1]<<8) | a_అంకె[0];
  }లేకపోతే{
    val = (పూర్ణాంకం)((u32)a_అంకె[0]<<24) | (a_అంకె[1]<<16) | (a_అంకె[2]<<8) | a_అంకె[3];
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(val));
  తిరిగి TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI hexio_render_int16(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం val;
  గుర్తులేని అక్షరం a_అంకె[10];

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "INTEGER");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &val) ) తిరిగి TCL_ERROR;
  a_అంకె[0] = val>>8;
  a_అంకె[1] = val;
  sqlite3_Test_Bin_గమ్యం_Hex(a_అంకె, 2);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj((అక్షరం*)a_అంకె, 4));
  తిరిగి TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI hexio_render_int32(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం val;
  గుర్తులేని అక్షరం a_అంకె[10];

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "INTEGER");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &val) ) తిరిగి TCL_ERROR;
  a_అంకె[0] = val>>24;
  a_అంకె[1] = val>>16;
  a_అంకె[2] = val>>8;
  a_అంకె[3] = val;
  sqlite3_Test_Bin_గమ్యం_Hex(a_అంకె, 4);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj((అక్షరం*)a_అంకె, 8));
  తిరిగి TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI utf8_గమ్యం_utf8(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
#నిర్వచితమైతే SQLITE_DEBUG
  Tcl_కొలత n;
  పూర్ణాంకం n_బహిర్గతం;
  స్థిరరాశి గుర్తులేని అక్షరం *zOrig;
  గుర్తులేని అక్షరం *z;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HEX");
    తిరిగి TCL_ERROR;
  }
  zOrig = (గుర్తులేని అక్షరం *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_గమ్యం_Bin(zOrig, (పూర్ణాంకం)n, z);
  z[n] = 0;
  n_బహిర్గతం = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_గమ్యం_Hex(z,n_బహిర్గతం);
  Tcl_Append_ఫలితం(interp, (అక్షరం*)z, ఖాళీ);
  sqlite3_free(z);
  తిరిగి TCL_OK;
#లేకపోతే
  Tcl_Append_ఫలితం(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ఖాళీ
  );
  తిరిగి TCL_ERROR;
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం getFts3Varint(స్థిరరాశి అక్షరం *p, sqlite_int64 *v){
  స్థిరరాశి గుర్తులేని అక్షరం *q = (స్థిరరాశి గుర్తులేని అక్షరం *) p;
  sqlite_uint64 x = 0, y = 1;
  అయితే( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  తిరిగి (పూర్ణాంకం) (q - (గుర్తులేని అక్షరం *)p);
}

స్థిర పూర్ణాంకం putFts3Varint(అక్షరం *p, sqlite_int64 v){
  గుర్తులేని అక్షరం *q = (గుర్తులేని అక్షరం *) p;
  sqlite_uint64 vu = v;
  చేయి{
    *q++ = (గుర్తులేని అక్షరం) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }అయితే( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  ప్రతిజ్ఞా( q - (గుర్తులేని అక్షరం *)p <= 10 );
  తిరిగి (పూర్ణాంకం) (q - (గుర్తులేని అక్షరం *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI read_fts3varint(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_కొలత nBlob;
  గుర్తులేని అక్షరం *zBlob;
  sqlite3_int64 iVal;
  పూర్ణాంకం nVal;

  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BLOB VARNAME");
    తిరిగి TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((అక్షరం*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_సమితి_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(nVal));
  తిరిగి TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI make_fts3record(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_కొలత nArg = 0;
  గుర్తులేని అక్షరం *a_బహిర్గతం = 0;
  sqlite3_int64 n_బహిర్గతం = 0;
  sqlite3_int64 nAlloc = 0;
  పూర్ణాంకం i;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "LIST");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    తిరిగి TCL_ERROR;
  }

  కొరకు(i=0; i<(పూర్ణాంకం)nArg; i++){
    Tcl_WideInt iVal;
    ఒకవేళ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ఒకవేళ( n_బహిర్గతం+10>nAlloc ){
        పూర్ణాంకం nNew = nAlloc?nAlloc*2:128;
        గుర్తులేని అక్షరం *aNew = sqlite3_realloc(a_బహిర్గతం, nNew);
        ఒకవేళ( aNew==0 ){
          sqlite3_free(a_బహిర్గతం);
          తిరిగి TCL_ERROR;
        }
        a_బహిర్గతం = aNew;
        nAlloc = nNew;
      }
      n_బహిర్గతం += putFts3Varint((అక్షరం*)&a_బహిర్గతం[n_బహిర్గతం], iVal);
    }లేకపోతే{
      Tcl_కొలత nVal = 0;
      అక్షరం *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      అయితే( (n_బహిర్గతం + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        గుర్తులేని అక్షరం *aNew = sqlite3_realloc64(a_బహిర్గతం, nNew);
        ఒకవేళ( aNew==0 ){
          sqlite3_free(a_బహిర్గతం);
          తిరిగి TCL_ERROR;
        }
        a_బహిర్గతం = aNew;
        nAlloc = nNew;
      }
      జ్ఞాపకం_నకలు(&a_బహిర్గతం[n_బహిర్గతం], zVal, nVal);
      n_బహిర్గతం += nVal;
    }
  }

  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewByteArrayObj(a_బహిర్గతం, n_బహిర్గతం));
  sqlite3_free(a_బహిర్గతం);
  తిరిగి TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest_hexio_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "hexio_read",                   hexio_read            },
     { "hexio_write",                  hexio_write           },
     { "hexio_get_int",                hexio_get_int         },
     { "hexio_render_int16",           hexio_render_int16    },
     { "hexio_render_int32",           hexio_render_int32    },
     { "utf8_to_utf8",                 utf8_గమ్యం_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  తిరిగి TCL_OK;
}
