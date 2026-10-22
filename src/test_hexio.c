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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
ਖਾਲੀ sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBuf, ਪੂਰਨ N){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zHex[] = "0123456789ABCDEF";
  ਪੂਰਨ i, j;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c;
  i = N*2;
  zBuf[i--] = 0;
  ਲਈ(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
ਪੂਰਨ sqlite3_Test_Hex_ਮੰਜ਼ਿਲ_Bin(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਵਿੱਚ, ਪੂਰਨ N, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਬਾਹਰਲਾ){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aMap[] = {
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
  ਪੂਰਨ i, j;
  ਪੂਰਨ hi=1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c;

  ਲਈ(i=j=0; i<N; i++){
    c = aMap[z_ਵਿੱਚ[i]];
    ਜੇ( c==0 ) ਜਾਰੀ;
    ਜੇ( hi ){
      a_ਬਾਹਰਲਾ[j] = (c-1)<<4;
      hi = 0;
    }ਨਹੀਂਤਾਂ{
      a_ਬਾਹਰਲਾ[j++] |= c-1;
      hi = 1;
    }
  }
  ਵਾਪਸ j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI hexio_read(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ ਵਿੱਥ;
  ਪੂਰਨ amt, got;
  ਅਚਲ ਅੱਖਰ *zFile;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBuf;
  FILE *ਵਿੱਚ;

  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME OFFSET AMT");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &ਵਿੱਥ) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ਵਾਪਸ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ਜੇ( zBuf==0 ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਿੱਚ = fopen(zFile, "rb");
  ਜੇ( ਵਿੱਚ==0 ){
    ਵਿੱਚ = fopen(zFile, "r");
  }
  ਜੇ( ਵਿੱਚ==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "cannot open input file ", zFile, ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  fseek(ਵਿੱਚ, ਵਿੱਥ, SEEK_SET);
  got = (ਪੂਰਨ)fread(zBuf, 1, amt, ਵਿੱਚ);
  fclose(ਵਿੱਚ);
  ਜੇ( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(zBuf, got);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  sqlite3_free(zBuf);
  ਵਾਪਸ TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI hexio_write(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ ਵਿੱਥ;
  Tcl_ਮਾਪ n_ਵਿੱਚ;
  ਪੂਰਨ n_ਬਾਹਰਲਾ, written;
  ਅਚਲ ਅੱਖਰ *zFile;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਵਿੱਚ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਬਾਹਰਲਾ;
  FILE *ਬਾਹਰਲਾ;

  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &ਵਿੱਥ) ) ਵਾਪਸ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_ਵਿੱਚ = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)Tcl_GetStringFromObj(objv[3], &n_ਵਿੱਚ);
  a_ਬਾਹਰਲਾ = sqlite3_malloc64( 1 + n_ਵਿੱਚ/2 );
  ਜੇ( a_ਬਾਹਰਲਾ==0 ){
    ਵਾਪਸ TCL_ERROR;
  }
  n_ਬਾਹਰਲਾ = sqlite3_Test_Hex_ਮੰਜ਼ਿਲ_Bin(z_ਵਿੱਚ, (ਪੂਰਨ)n_ਵਿੱਚ, a_ਬਾਹਰਲਾ);
  ਬਾਹਰਲਾ = fopen(zFile, "r+b");
  ਜੇ( ਬਾਹਰਲਾ==0 ){
    ਬਾਹਰਲਾ = fopen(zFile, "r+");
  }
  ਜੇ( ਬਾਹਰਲਾ==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "cannot open output file ", zFile, ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  fseek(ਬਾਹਰਲਾ, ਵਿੱਥ, SEEK_SET);
  written = (ਪੂਰਨ)fwrite(a_ਬਾਹਰਲਾ, 1, n_ਬਾਹਰਲਾ, ਬਾਹਰਲਾ);
  sqlite3_free(a_ਬਾਹਰਲਾ);
  fclose(ਬਾਹਰਲਾ);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(written));
  ਵਾਪਸ TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI hexio_get_int(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ val;
  Tcl_ਮਾਪ n_ਵਿੱਚ;
  ਪੂਰਨ n_ਬਾਹਰਲਾ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਵਿੱਚ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਬਾਹਰਲਾ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ a_ਅੰਕ[4];
  ਪੂਰਨ bLittle = 0;

  ਜੇ( objc==3 ){
    Tcl_ਮਾਪ n;
    ਅੱਖਰ *z = Tcl_GetStringFromObj(objv[1], &n);
    ਜੇ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ਜੇ( (objc-bLittle)!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "[-littleendian] HEXDATA");
    ਵਾਪਸ TCL_ERROR;
  }
  z_ਵਿੱਚ = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)Tcl_GetStringFromObj(objv[1+bLittle], &n_ਵਿੱਚ);
  a_ਬਾਹਰਲਾ = sqlite3_malloc64( 1 + n_ਵਿੱਚ/2 );
  ਜੇ( a_ਬਾਹਰਲਾ==0 ){
    ਵਾਪਸ TCL_ERROR;
  }
  n_ਬਾਹਰਲਾ = sqlite3_Test_Hex_ਮੰਜ਼ਿਲ_Bin(z_ਵਿੱਚ, (ਪੂਰਨ)n_ਵਿੱਚ, a_ਬਾਹਰਲਾ);
  ਜੇ( n_ਬਾਹਰਲਾ>=4 ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(a_ਅੰਕ, a_ਬਾਹਰਲਾ, 4);
  }ਨਹੀਂਤਾਂ{
    memset(a_ਅੰਕ, 0, ਆਕਾਰ(a_ਅੰਕ));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਅੰਕ[4-n_ਬਾਹਰਲਾ], a_ਬਾਹਰਲਾ, n_ਬਾਹਰਲਾ);
  }
  sqlite3_free(a_ਬਾਹਰਲਾ);
  ਜੇ( bLittle ){
    val = (ਪੂਰਨ)((u32)a_ਅੰਕ[3]<<24) | (a_ਅੰਕ[2]<<16) | (a_ਅੰਕ[1]<<8) | a_ਅੰਕ[0];
  }ਨਹੀਂਤਾਂ{
    val = (ਪੂਰਨ)((u32)a_ਅੰਕ[0]<<24) | (a_ਅੰਕ[1]<<16) | (a_ਅੰਕ[2]<<8) | a_ਅੰਕ[3];
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(val));
  ਵਾਪਸ TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI hexio_render_int16(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ val;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ a_ਅੰਕ[10];

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "INTEGER");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ਵਾਪਸ TCL_ERROR;
  a_ਅੰਕ[0] = val>>8;
  a_ਅੰਕ[1] = val;
  sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(a_ਅੰਕ, 2);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj((ਅੱਖਰ*)a_ਅੰਕ, 4));
  ਵਾਪਸ TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI hexio_render_int32(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ val;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ a_ਅੰਕ[10];

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "INTEGER");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ਵਾਪਸ TCL_ERROR;
  a_ਅੰਕ[0] = val>>24;
  a_ਅੰਕ[1] = val>>16;
  a_ਅੰਕ[2] = val>>8;
  a_ਅੰਕ[3] = val;
  sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(a_ਅੰਕ, 4);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj((ਅੱਖਰ*)a_ਅੰਕ, 8));
  ਵਾਪਸ TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI utf8_ਮੰਜ਼ਿਲ_utf8(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  Tcl_ਮਾਪ n;
  ਪੂਰਨ n_ਬਾਹਰਲਾ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zOrig;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HEX");
    ਵਾਪਸ TCL_ERROR;
  }
  zOrig = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_ਮੰਜ਼ਿਲ_Bin(zOrig, (ਪੂਰਨ)n, z);
  z[n] = 0;
  n_ਬਾਹਰਲਾ = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(z,n_ਬਾਹਰਲਾ);
  Tcl_Append_ਨਤੀਜਾ(interp, (ਅੱਖਰ*)z, ਨਿਰਾ);
  sqlite3_free(z);
  ਵਾਪਸ TCL_OK;
#ਨਹੀਂਤਾਂ
  Tcl_Append_ਨਤੀਜਾ(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ਨਿਰਾ
  );
  ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ getFts3Varint(ਅਚਲ ਅੱਖਰ *p, sqlite_int64 *v){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *q = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) p;
  sqlite_uint64 x = 0, y = 1;
  ਜਦੋਂ( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ਵਾਪਸ (ਪੂਰਨ) (q - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)p);
}

ਸਥਿਰ ਪੂਰਨ putFts3Varint(ਅੱਖਰ *p, sqlite_int64 v){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *q = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) p;
  sqlite_uint64 vu = v;
  ਕਰੋ{
    *q++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }ਜਦੋਂ( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  ਪ੍ਰਤਿਜ੍ਞਾ( q - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)p <= 10 );
  ਵਾਪਸ (ਪੂਰਨ) (q - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI read_fts3varint(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ਮਾਪ nBlob;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBlob;
  sqlite3_int64 iVal;
  ਪੂਰਨ nVal;

  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BLOB VARNAME");
    ਵਾਪਸ TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((ਅੱਖਰ*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_ਸੈੱਟ_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(nVal));
  ਵਾਪਸ TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI make_fts3record(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_ਮਾਪ nArg = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਬਾਹਰਲਾ = 0;
  sqlite3_int64 n_ਬਾਹਰਲਾ = 0;
  sqlite3_int64 nAlloc = 0;
  ਪੂਰਨ i;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "LIST");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਲਈ(i=0; i<(ਪੂਰਨ)nArg; i++){
    Tcl_WideInt iVal;
    ਜੇ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ਜੇ( n_ਬਾਹਰਲਾ+10>nAlloc ){
        ਪੂਰਨ nNew = nAlloc?nAlloc*2:128;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *aNew = sqlite3_realloc(a_ਬਾਹਰਲਾ, nNew);
        ਜੇ( aNew==0 ){
          sqlite3_free(a_ਬਾਹਰਲਾ);
          ਵਾਪਸ TCL_ERROR;
        }
        a_ਬਾਹਰਲਾ = aNew;
        nAlloc = nNew;
      }
      n_ਬਾਹਰਲਾ += putFts3Varint((ਅੱਖਰ*)&a_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ], iVal);
    }ਨਹੀਂਤਾਂ{
      Tcl_ਮਾਪ nVal = 0;
      ਅੱਖਰ *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      ਜਦੋਂ( (n_ਬਾਹਰਲਾ + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *aNew = sqlite3_realloc64(a_ਬਾਹਰਲਾ, nNew);
        ਜੇ( aNew==0 ){
          sqlite3_free(a_ਬਾਹਰਲਾ);
          ਵਾਪਸ TCL_ERROR;
        }
        a_ਬਾਹਰਲਾ = aNew;
        nAlloc = nNew;
      }
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ], zVal, nVal);
      n_ਬਾਹਰਲਾ += nVal;
    }
  }

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewByteArrayObj(a_ਬਾਹਰਲਾ, n_ਬਾਹਰਲਾ));
  sqlite3_free(a_ਬਾਹਰਲਾ);
  ਵਾਪਸ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "hexio_read",                   hexio_read            },
     { "hexio_write",                  hexio_write           },
     { "hexio_get_int",                hexio_get_int         },
     { "hexio_render_int16",           hexio_render_int16    },
     { "hexio_render_int32",           hexio_render_int32    },
     { "utf8_to_utf8",                 utf8_ਮੰਜ਼ਿਲ_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ਵਾਪਸ TCL_OK;
}
