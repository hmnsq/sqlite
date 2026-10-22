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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>


/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
ಶೂನ್ಯ sqlite3_Test_Bin_ಗಮ್ಯ_Hex(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBuf, ಪೂರ್ಣಾಂಕ N){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zHex[] = "0123456789ABCDEF";
  ಪೂರ್ಣಾಂಕ i, j;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c;
  i = N*2;
  zBuf[i--] = 0;
  ಗಾಗಿ(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  ಪ್ರತಿಜ್ಞಾ( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Test_Hex_ಗಮ್ಯ_Bin(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಒಳ, ಪೂರ್ಣಾಂಕ N, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಹೊರಹೋಗು){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aMap[] = {
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
  ಪೂರ್ಣಾಂಕ i, j;
  ಪೂರ್ಣಾಂಕ hi=1;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c;

  ಗಾಗಿ(i=j=0; i<N; i++){
    c = aMap[z_ಒಳ[i]];
    ಆದರೆ( c==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( hi ){
      a_ಹೊರಹೋಗು[j] = (c-1)<<4;
      hi = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      a_ಹೊರಹೋಗು[j++] |= c-1;
      hi = 1;
    }
  }
  ಹಿಂತಿರುಗಿ j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI hexio_read(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ;
  ಪೂರ್ಣಾಂಕ amt, got;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBuf;
  FILE *ಒಳ;

  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME OFFSET AMT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &ಸ್ಥಾನಾಂತರ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[3], &amt) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  ಆದರೆ( zBuf==0 ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಒಳ = fopen(zFile, "rb");
  ಆದರೆ( ಒಳ==0 ){
    ಒಳ = fopen(zFile, "r");
  }
  ಆದರೆ( ಒಳ==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "cannot open input file ", zFile, ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  fseek(ಒಳ, ಸ್ಥಾನಾಂತರ, SEEK_SET);
  got = (ಪೂರ್ಣಾಂಕ)fread(zBuf, 1, amt, ಒಳ);
  fclose(ಒಳ);
  ಆದರೆ( got<0 ){
    got = 0;
  }
  sqlite3_Test_Bin_ಗಮ್ಯ_Hex(zBuf, got);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  sqlite3_free(zBuf);
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI hexio_write(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ;
  Tcl_ಗಾತ್ರ n_ಒಳ;
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು, written;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಒಳ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಹೊರಹೋಗು;
  FILE *ಹೊರಹೋಗು;

  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &ಸ್ಥಾನಾಂತರ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)Tcl_GetStringFromObj(objv[3], &n_ಒಳ);
  a_ಹೊರಹೋಗು = sqlite3_malloc64( 1 + n_ಒಳ/2 );
  ಆದರೆ( a_ಹೊರಹೋಗು==0 ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  n_ಹೊರಹೋಗು = sqlite3_Test_Hex_ಗಮ್ಯ_Bin(z_ಒಳ, (ಪೂರ್ಣಾಂಕ)n_ಒಳ, a_ಹೊರಹೋಗು);
  ಹೊರಹೋಗು = fopen(zFile, "r+b");
  ಆದರೆ( ಹೊರಹೋಗು==0 ){
    ಹೊರಹೋಗು = fopen(zFile, "r+");
  }
  ಆದರೆ( ಹೊರಹೋಗು==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "cannot open output file ", zFile, ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  fseek(ಹೊರಹೋಗು, ಸ್ಥಾನಾಂತರ, SEEK_SET);
  written = (ಪೂರ್ಣಾಂಕ)fwrite(a_ಹೊರಹೋಗು, 1, n_ಹೊರಹೋಗು, ಹೊರಹೋಗು);
  sqlite3_free(a_ಹೊರಹೋಗು);
  fclose(ಹೊರಹೋಗು);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(written));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** USAGE:   hexio_get_int [-littleendian] HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI hexio_get_int(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ val;
  Tcl_ಗಾತ್ರ n_ಒಳ;
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಒಳ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಹೊರಹೋಗು;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ a_ಅಂಕೆ[4];
  ಪೂರ್ಣಾಂಕ bLittle = 0;

  ಆದರೆ( objc==3 ){
    Tcl_ಗಾತ್ರ n;
    ಅಕ್ಷರ *z = Tcl_GetStringFromObj(objv[1], &n);
    ಆದರೆ( n>=2 && n<=13 && memcmp(z, "-littleendian", n)==0 ){
      bLittle = 1;
    }
  }
  ಆದರೆ( (objc-bLittle)!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "[-littleendian] HEXDATA");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)Tcl_GetStringFromObj(objv[1+bLittle], &n_ಒಳ);
  a_ಹೊರಹೋಗು = sqlite3_malloc64( 1 + n_ಒಳ/2 );
  ಆದರೆ( a_ಹೊರಹೋಗು==0 ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  n_ಹೊರಹೋಗು = sqlite3_Test_Hex_ಗಮ್ಯ_Bin(z_ಒಳ, (ಪೂರ್ಣಾಂಕ)n_ಒಳ, a_ಹೊರಹೋಗು);
  ಆದರೆ( n_ಹೊರಹೋಗು>=4 ){
    ಸ್ಮರಣೆ_ನಕಲು(a_ಅಂಕೆ, a_ಹೊರಹೋಗು, 4);
  }ಇಲ್ಲದಿದ್ದರೆ{
    memset(a_ಅಂಕೆ, 0, ಗಾತ್ರ(a_ಅಂಕೆ));
    ಸ್ಮರಣೆ_ನಕಲು(&a_ಅಂಕೆ[4-n_ಹೊರಹೋಗು], a_ಹೊರಹೋಗು, n_ಹೊರಹೋಗು);
  }
  sqlite3_free(a_ಹೊರಹೋಗು);
  ಆದರೆ( bLittle ){
    val = (ಪೂರ್ಣಾಂಕ)((u32)a_ಅಂಕೆ[3]<<24) | (a_ಅಂಕೆ[2]<<16) | (a_ಅಂಕೆ[1]<<8) | a_ಅಂಕೆ[0];
  }ಇಲ್ಲದಿದ್ದರೆ{
    val = (ಪೂರ್ಣಾಂಕ)((u32)a_ಅಂಕೆ[0]<<24) | (a_ಅಂಕೆ[1]<<16) | (a_ಅಂಕೆ[2]<<8) | a_ಅಂಕೆ[3];
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(val));
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI hexio_render_int16(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ val;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ a_ಅಂಕೆ[10];

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "INTEGER");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  a_ಅಂಕೆ[0] = val>>8;
  a_ಅಂಕೆ[1] = val;
  sqlite3_Test_Bin_ಗಮ್ಯ_Hex(a_ಅಂಕೆ, 2);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj((ಅಕ್ಷರ*)a_ಅಂಕೆ, 4));
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI hexio_render_int32(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ val;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ a_ಅಂಕೆ[10];

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "INTEGER");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &val) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  a_ಅಂಕೆ[0] = val>>24;
  a_ಅಂಕೆ[1] = val>>16;
  a_ಅಂಕೆ[2] = val>>8;
  a_ಅಂಕೆ[3] = val;
  sqlite3_Test_Bin_ಗಮ್ಯ_Hex(a_ಅಂಕೆ, 4);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj((ಅಕ್ಷರ*)a_ಅಂಕೆ, 8));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI utf8_ಗಮ್ಯ_utf8(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  Tcl_ಗಾತ್ರ n;
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zOrig;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HEX");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zOrig = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc64( n+4 );
  n = sqlite3_Test_Hex_ಗಮ್ಯ_Bin(zOrig, (ಪೂರ್ಣಾಂಕ)n, z);
  z[n] = 0;
  n_ಹೊರಹೋಗು = sqlite3Utf8To8(z);
  sqlite3_Test_Bin_ಗಮ್ಯ_Hex(z,n_ಹೊರಹೋಗು);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ*)z, ಖಾಲಿ);
  sqlite3_free(z);
  ಹಿಂತಿರುಗಿ TCL_OK;
#ಇಲ್ಲದಿದ್ದರೆ
  Tcl_Append_ಫಲಿತಾಂಶ(interp, 
      "[utf8_to_utf8] unavailable - SQLITE_DEBUG not defined", ಖಾಲಿ
  );
  ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getFts3Varint(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, sqlite_int64 *v){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *q = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) p;
  sqlite_uint64 x = 0, y = 1;
  ಆಗ( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ) (q - (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)p);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ putFts3Varint(ಅಕ್ಷರ *p, sqlite_int64 v){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *q = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) p;
  sqlite_uint64 vu = v;
  ಮಾಡು{
    *q++ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }ಆಗ( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  ಪ್ರತಿಜ್ಞಾ( q - (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)p <= 10 );
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ) (q - (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)p);
}

/*
** USAGE:  read_fts3varint BLOB VARNAME
**
** Read a varint from the start of BLOB. Set variable VARNAME to contain
** the interpreted value. Return the number of bytes of BLOB consumed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI read_fts3varint(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ಗಾತ್ರ nBlob;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBlob;
  sqlite3_int64 iVal;
  ಪೂರ್ಣಾಂಕ nVal;

  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BLOB VARNAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zBlob = Tcl_GetByteArrayFromObj(objv[1], &nBlob);

  nVal = getFts3Varint((ಅಕ್ಷರ*)zBlob, (sqlite3_int64 *)(&iVal));
  Tcl_Obj_ಗಣ_Var2(interp, objv[2], 0, Tcl_NewWideIntObj(iVal), 0);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(nVal));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** USAGE:  make_fts3record ARGLIST
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI make_fts3record(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj **aArg = 0;
  Tcl_ಗಾತ್ರ nArg = 0;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಹೊರಹೋಗು = 0;
  sqlite3_int64 n_ಹೊರಹೋಗು = 0;
  sqlite3_int64 nAlloc = 0;
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "LIST");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_ListObjGetElements(interp, objv[1], &nArg, &aArg) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಗಾಗಿ(i=0; i<(ಪೂರ್ಣಾಂಕ)nArg; i++){
    Tcl_WideInt iVal;
    ಆದರೆ( TCL_OK==Tcl_GetWideIntFromObj(0, aArg[i], &iVal) ){
      ಆದರೆ( n_ಹೊರಹೋಗು+10>nAlloc ){
        ಪೂರ್ಣಾಂಕ nNew = nAlloc?nAlloc*2:128;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *aNew = sqlite3_realloc(a_ಹೊರಹೋಗು, nNew);
        ಆದರೆ( aNew==0 ){
          sqlite3_free(a_ಹೊರಹೋಗು);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        a_ಹೊರಹೋಗು = aNew;
        nAlloc = nNew;
      }
      n_ಹೊರಹೋಗು += putFts3Varint((ಅಕ್ಷರ*)&a_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು], iVal);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_ಗಾತ್ರ nVal = 0;
      ಅಕ್ಷರ *zVal = Tcl_GetStringFromObj(aArg[i], &nVal);
      ಆಗ( (n_ಹೊರಹೋಗು + nVal)>nAlloc ){
        sqlite3_int64 nNew = nAlloc?nAlloc*2:128;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *aNew = sqlite3_realloc64(a_ಹೊರಹೋಗು, nNew);
        ಆದರೆ( aNew==0 ){
          sqlite3_free(a_ಹೊರಹೋಗು);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        a_ಹೊರಹೋಗು = aNew;
        nAlloc = nNew;
      }
      ಸ್ಮರಣೆ_ನಕಲು(&a_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು], zVal, nVal);
      n_ಹೊರಹೋಗು += nVal;
    }
  }

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewByteArrayObj(a_ಹೊರಹೋಗು, n_ಹೊರಹೋಗು));
  sqlite3_free(a_ಹೊರಹೋಗು);
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest_hexio_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "hexio_read",                   hexio_read            },
     { "hexio_write",                  hexio_write           },
     { "hexio_get_int",                hexio_get_int         },
     { "hexio_render_int16",           hexio_render_int16    },
     { "hexio_render_int32",           hexio_render_int32    },
     { "utf8_to_utf8",                 utf8_ಗಮ್ಯ_utf8          },
     { "read_fts3varint",              read_fts3varint       },
     { "make_fts3record",              make_fts3record       },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
