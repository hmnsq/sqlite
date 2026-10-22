/*
** 2002 February 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C-language implementations for many of the SQL
** functions of SQLite.  (Some function, and in particular the date and
** time functions, are implemented separately.)
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
#ಅನ್ತರ್ಭಾವ <math.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
ಸ್ಥಿರ CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  ಪ್ರತಿಜ್ಞಾ( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_CollSeq );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_COLLSEQ );
  ಹಿಂತಿರುಗಿ pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_Skip_ಸಂಚಯಕ_ತುಂಬು(sqlite3_context *context){
  ಪ್ರತಿಜ್ಞಾ( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
ಸ್ಥಿರ ಶೂನ್ಯ minmaxFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ;    /* 0 for min() or 0xffffffff for max() */
  ಪೂರ್ಣಾಂಕ i_ಉತ್ತಮ;
  CollSeq *pColl;

  ಪ್ರತಿಜ್ಞಾ( argc>1 );
  ಮುಖವಾಡ = sqlite3_user_ದತ್ತಾಂಶ(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  ಪ್ರತಿಜ್ಞಾ( pColl );
  ಪ್ರತಿಜ್ಞಾ( ಮುಖವಾಡ==-1 || ಮುಖವಾಡ==0 );
  i_ಉತ್ತಮ = 0;
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=1; i<argc; i++){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
    ಆದರೆ( (sqlite3MemCompare(argv[i_ಉತ್ತಮ], argv[i], pColl)^ಮುಖವಾಡ)>=0 ){
      testcase( ಮುಖವಾಡ==0 );
      i_ಉತ್ತಮ = i;
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, argv[i_ಉತ್ತಮ]);
}

/*
** Return the type of the argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ typeofFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azType[] = { "integer", "real", "text", "blob", "null" };
  ಪೂರ್ಣಾಂಕ i = sqlite3_ಮೌಲ್ಯ_type(argv[0]) - 1;
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<Array_ಗಾತ್ರ(azType) );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_INTEGER==1 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_FLOAT==2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_ಪಠ್ಯ==3 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_BLOB==4 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
ಸ್ಥಿರ ಶೂನ್ಯ subtypeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಬಳಸದ_PARAMETER(argc);
  sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3_ಮೌಲ್ಯ_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಉದ್ದಳತೆ_Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಸ್ಥಿತಿ SQLITE_BLOB:
    ಸ್ಥಿತಿ SQLITE_INTEGER:
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
      sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ: {
      ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
      ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z0;
      ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ;
      z0 = z;
      ಆಗ( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        ಆದರೆ( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]==0 ){
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ{
          z++;
          ಆಗ( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_ಫಲಿತಾಂಶ_int(context, (ಪೂರ್ಣಾಂಕ)(z-z0));
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      sqlite3_ಫಲಿತಾಂಶ_null(context);
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
ಸ್ಥಿರ ಶೂನ್ಯ bytelengthFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಸ್ಥಿತಿ SQLITE_BLOB: {
      sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_INTEGER:
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_ಫಲಿತಾಂಶ_int64(context, sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0])*m);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ: {
      ಆದರೆ( sqlite3_ಮೌಲ್ಯ_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]));
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3_ಮೌಲ್ಯ_bytes16(argv[0]));
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      sqlite3_ಫಲಿತಾಂಶ_null(context);
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
ಸ್ಥಿರ ಶೂನ್ಯ absFunc(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
      i64 iVal = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
      ಆದರೆ( iVal<0 ){
        ಆದರೆ( iVal==ಅತಿಚಿಕ್ಕ_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_ಫಲಿತಾಂಶ_error(context, "integer overflow", -1);
          ಹಿಂತಿರುಗಿ;
        }
        iVal = -iVal;
      }
      sqlite3_ಫಲಿತಾಂಶ_int64(context, iVal);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_ಫಲಿತಾಂಶ_null(context);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      ದ್ವಿಗುಣ rVal = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
      ಆದರೆ( rVal<0 ) rVal = -rVal;
      sqlite3_ಫಲಿತಾಂಶ_double(context, rVal);
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Implementation of the instr() function.
**
** instr(haystack,needle) finds the first occurrence of needle
** in haystack and returns the number of previous characters plus 1,
** or 0 if needle does not occur within haystack.
**
** If both haystack and needle are BLOBs, then the result is one more than
** the number of bytes in haystack prior to the first occurrence of needle,
** or 0 if needle never occurs in haystack.
*/
ಸ್ಥಿರ ಶೂನ್ಯ instrFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zHaystack;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zNeedle;
  ಪೂರ್ಣಾಂಕ nHaystack;
  ಪೂರ್ಣಾಂಕ nNeedle;
  ಪೂರ್ಣಾಂಕ typeHaystack, typeNeedle;
  ಪೂರ್ಣಾಂಕ N = 1;
  ಪೂರ್ಣಾಂಕ is_ಪಠ್ಯ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಮೊದಲ_Char;
  sqlite3_ಮೌಲ್ಯ *pC1 = 0;
  sqlite3_ಮೌಲ್ಯ *pC2 = 0;

  ಬಳಸದ_PARAMETER(argc);
  typeHaystack = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  typeNeedle = sqlite3_ಮೌಲ್ಯ_type(argv[1]);
  ಆದರೆ( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  nHaystack = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  nNeedle = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[1]);
  ಆದರೆ( nNeedle>0 ){
    ಆದರೆ( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
      zNeedle = sqlite3_ಮೌಲ್ಯ_blob(argv[1]);
      is_ಪಠ್ಯ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
      zNeedle = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
      is_ಪಠ್ಯ = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pC1 = sqlite3_ಮೌಲ್ಯ_dup(argv[0]);
      zHaystack = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pC1);
      ಆದರೆ( zHaystack==0 ) ಹೋಗು ಕೊನೆ_Instr_OOM;
      nHaystack = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pC1);
      pC2 = sqlite3_ಮೌಲ್ಯ_dup(argv[1]);
      zNeedle = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pC2);
      ಆದರೆ( zNeedle==0 ) ಹೋಗು ಕೊನೆ_Instr_OOM;
      nNeedle = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pC2);
      is_ಪಠ್ಯ = 1;
    }
    ಆದರೆ( zNeedle==0 || (nHaystack && zHaystack==0) ) ಹೋಗು ಕೊನೆ_Instr_OOM;
    ಮೊದಲ_Char = zNeedle[0];
    ಆಗ( nNeedle<=nHaystack
       && (zHaystack[0]!=ಮೊದಲ_Char || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      ಮಾಡು{
        nHaystack--;
        zHaystack++;
      }ಆಗ( is_ಪಠ್ಯ && (zHaystack[0]&0xc0)==0x80 );
    }
    ಆದರೆ( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_ಫಲಿತಾಂಶ_int(context, N);
ಕೊನೆ_Instr:
  sqlite3_ಮೌಲ್ಯ_free(pC1);
  sqlite3_ಮೌಲ್ಯ_free(pC2);
  ಹಿಂತಿರುಗಿ;
ಕೊನೆ_Instr_OOM:
  sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
  ಹೋಗು ಕೊನೆ_Instr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ printfFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  PrintfArguments x;
  StrAccum str;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  ಆದರೆ( argc>=1 && (zFormat = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.n_ಬಳಸಿದ = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_ಫಲಿತಾಂಶ_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
ಸ್ಥಿರ SQLITE_INLINE ಪೂರ್ಣಾಂಕ ಒಳಗೊಂಡಿದೆ_Nul_Or_Non_Ascii(u64 ಶಬ್ದ){
  ಸ್ಥಿರಾಂಕ u64 byteOnes = UINT64_C(0x0101010101010101);
  ಸ್ಥಿರಾಂಕ u64 byte_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  ಹಿಂತಿರುಗಿ ((ಶಬ್ದ | (ಶಬ್ದ - byteOnes)) & byte_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು)!=0;
}

/*
** Implementation of the substr() function.
**
** substr(x,p1,p2)  returns p2 characters of x[] beginning with p1.
** p1 is 1-indexed.  So substr(x,1,1) returns the first character
** of x.  If x is text, then we actually count UTF-8 characters.
** If x is a blob, then we count bytes.
**
** If p1 is negative, then we begin abs(p1) from the end of x[].
**
** If p2 is negative, return the p2 characters preceding p1.
*/
ಸ್ಥಿರ ಶೂನ್ಯ substrFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z2;
  ಪೂರ್ಣಾಂಕ len;
  ಪೂರ್ಣಾಂಕ p0type;
  i64 p1, p2;

  ಪ್ರತಿಜ್ಞಾ( argc==3 || argc==2 );
  p0type = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  p1 = sqlite3_ಮೌಲ್ಯ_int64(argv[1]);
  ಆದರೆ( p0type==SQLITE_BLOB ){
    len = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
    z = sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
    ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ;
    ಪ್ರತಿಜ್ಞಾ( len==sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]) );
  }ಇಲ್ಲದಿದ್ದರೆ{
    z = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ;
    len = 0;
    ಆದರೆ( p1<0 ){
      ಗಾಗಿ(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  ಆದರೆ( argc==3 ){
    p2 = sqlite3_ಮೌಲ್ಯ_int64(argv[2]);
    ಆದರೆ( p2==0 && sqlite3_ಮೌಲ್ಯ_type(argv[2])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p2 = ಅತಿದೊಡ್ಡ_INT64;
  }
  ಆದರೆ( p1==0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p1<0 ){
    p1 += len;
    ಆದರೆ( p1<0 ){
      ಆದರೆ( p2<0 ){
        p2 = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p2 += p1;
      }
      p1 = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p1>0 ){
    p1--;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p2>0 ){
    p2--;
  }
  ಆದರೆ( p2<0 ){
    ಆದರೆ( p2<-p1 ){
      p2 = p1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p2 = -p2;
    }
    p1 -= p2;
  }
  ಪ್ರತಿಜ್ಞಾ( p1>=0 && p2>=0 );
  ಆದರೆ( p0type!=SQLITE_BLOB ){
    ಆದರೆ( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಕೊನೆ = z + sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
      ಆಗ( p1>=8 && z_ಕೊನೆ-z>=8 ){
        u64 ಶಬ್ದ;
        ಸ್ಮರಣೆ_ನಕಲು(&ಶಬ್ದ, z, ಗಾತ್ರ(ಶಬ್ದ));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        ಆದರೆ( ಒಳಗೊಂಡಿದೆ_Nul_Or_Non_Ascii(ಶಬ್ದ) ){
          ನಿಲ್ಲಿಸು;
        }
        z += 8;
        p1 -= 8;
      }
    }
    ಗಾಗಿ( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      ಆದರೆ( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]==0 ){
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಮಾಡು{ z++; }ಆಗ( (z[0]&0xc0)==0x80 );
      }
    }
    ಗಾಗಿ(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      ಆದರೆ( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z2[0]==0 ){
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಮಾಡು{ z2++; }ಆಗ( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_ಫಲಿತಾಂಶ_text64(context, (ಅಕ್ಷರ*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( p1>=len ){
      p1 = p2 = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p2>len-p1 ){
      p2 = len-p1;
      ಪ್ರತಿಜ್ಞಾ( p2>0 );
    }
    sqlite3_ಫಲಿತಾಂಶ_blob64(context, (ಅಕ್ಷರ*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
ಸ್ಥಿರ ಶೂನ್ಯ ಸುತ್ತು_Func(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  i64 n = 0;
  ದ್ವಿಗುಣ r;
  ಅಕ್ಷರ *zBuf;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಆದರೆ( argc==2 ){
    ಆದರೆ( SQLITE_NULL==sqlite3_ಮೌಲ್ಯ_type(argv[1]) ) ಹಿಂತಿರುಗಿ;
    n = sqlite3_ಮೌಲ್ಯ_int64(argv[1]);
    ಆದರೆ( n>30 ) n = 30;
    ಆದರೆ( n<0 ) n = 0;
  }
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  r = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  ಆದರೆ( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n==0 ){ 
    r = (ದ್ವಿಗುಣ)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }ಇಲ್ಲದಿದ್ದರೆ{
    zBuf = sqlite3_mprintf("%!.*f",(ಪೂರ್ಣಾಂಕ)n,r);
    ಆದರೆ( zBuf==0 ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
      ಹಿಂತಿರುಗಿ;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_ಫಲಿತಾಂಶ_double(context, r);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *contextMalloc(sqlite3_context *context, i64 nByte){
  ಅಕ್ಷರ *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪ್ರತಿಜ್ಞಾ( nByte>0 );
  testcase( nByte==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] );
  testcase( nByte==(i64)db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]+1 );
  ಆದರೆ( nByte>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    sqlite3_ಫಲಿತಾಂಶ_error_toobig(context);
    z = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    z = sqlite3Malloc(nByte);
    ಆದರೆ( !z ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    }
  }
  ಹಿಂತಿರುಗಿ z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಊರ್ಧ್ವ_Func(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಅಕ್ಷರ *z1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z2;
  ಪೂರ್ಣಾಂಕ i, n;
  ಬಳಸದ_PARAMETER(argc);
  z2 = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ಪ್ರತಿಜ್ಞಾ( z2==(ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]) );
  ಆದರೆ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ಆದರೆ( z1 ){
      ಗಾಗಿ(i=0; i<n; i++){
        z1[i] = (ಅಕ್ಷರ)sqlite3Toupper(z2[i]);
      }
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, z1, n, sqlite3_free);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ ಅಧೋ_Func(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಅಕ್ಷರ *z1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z2;
  ಪೂರ್ಣಾಂಕ i, n;
  ಬಳಸದ_PARAMETER(argc);
  z2 = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ಪ್ರತಿಜ್ಞಾ( z2==(ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]) );
  ಆದರೆ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ಆದರೆ( z1 ){
      ಗಾಗಿ(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, z1, n, sqlite3_free);
    }
  }
}

/*
** Some functions like COALESCE() and IFNULL() and UNLIKELY() are implemented
** as VDBE code so that unused argument values do not have to be computed.
** However, we still need some kind of function implementation for this
** routines in the function table.  The noopFunc macro provides this.
** noopFunc will never be called so it doesn't matter what the implementation
** is.  We might as well use the "version()" function as a substitute.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು noopFunc ಆವೃತ್ತಿ_Func   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
ಸ್ಥಿರ ಶೂನ್ಯ randomFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite_int64 r;
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  sqlite3_randomness(ಗಾತ್ರ(r), &r);
  ಆದರೆ( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & ಅತಿದೊಡ್ಡ_INT64);
  }
  sqlite3_ಫಲಿತಾಂಶ_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
ಸ್ಥಿರ ಶೂನ್ಯ randomBlob(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_int64 n;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  n = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
  ಆದರೆ( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  ಆದರೆ( p ){
    sqlite3_randomness(n, p);
    sqlite3_ಫಲಿತಾಂಶ_blob(context, (ಅಕ್ಷರ*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕೊನೆಯ_ಸೇರಿಸು_rowid(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_ಫಲಿತಾಂಶ_int64(context, sqlite3_ಕೊನೆಯ_ಸೇರಿಸು_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ changes(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  sqlite3_ಫಲಿತಾಂಶ_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಒಟ್ಟು_changes(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_ಫಲಿತಾಂಶ_int64(context, sqlite3_ಒಟ್ಟು_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
ರಚನೆ compareInfo {
  u8 ಹೊಂದಿಕೆ_All;          /* "*" or "%" */
  u8 ಹೊಂದಿಕೆ_One;          /* "?" or "_" */
  u8 ಹೊಂದಿಕೆ_ಗಣ;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#ಆದರೆ defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#ಇಲ್ಲದಿದ್ದರೆ
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಹೊಂದಿಕೆ             0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NOMATCH           1
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NOWILDCARDMATCH   2

/*
** Compare two UTF-8 strings for equality where the first string is
** a GLOB or LIKE expression.  Return values:
**
**    SQLITE_MATCH:            Match
**    SQLITE_NOMATCH:          No match
**    SQLITE_NOWILDCARDMATCH:  No match in spite of having * or % wildcards.
**
** Globbing rules:
**
**      '*'       Matches any sequence of zero or more characters.
**
**      '?'       Matches exactly one character.
**
**     [...]      Matches one character from the enclosed list of
**                characters.
**
**     [^...]     Matches one character not in the enclosed list.
**
** With the [...] and [^...] matching, a ']' character can be included
** in the list by making it the first character after '[' or '^'.  A
** range of characters can be specified using '-'.  Example:
** "[a-z]" matches any single lower-case letter.  To match a '-', make
** it the last character in the list.
**
** Like matching rules:
**
**      '%'       Matches any sequence of zero or more characters
**
***     '_'       Matches any one character
**
**      Ec        Where E is the "esc" character and c is any other
**                character, including '%', '_', and esc, match exactly c.
**
** The comments within this routine usually assume glob matching.
**
** This routine is usually quick, but can be N**2 in the worst case.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ನಮೂನೆ_Compare(
  ಸ್ಥಿರಾಂಕ u8 *z_ನಮೂನೆ,              /* The glob pattern */
  ಸ್ಥಿರಾಂಕ u8 *zString,               /* The string to compare against the glob */
  ಸ್ಥಿರಾಂಕ ರಚನೆ compareInfo *pInfo, /* Information about how to do the compare */
  u32 ಹೊಂದಿಕೆ_Other                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 ಹೊಂದಿಕೆ_One = pInfo->ಹೊಂದಿಕೆ_One;  /* "?" or "_" */
  u32 ಹೊಂದಿಕೆ_All = pInfo->ಹೊಂದಿಕೆ_All;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ಸ್ಥಿರಾಂಕ u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  ಆಗ( (c = Utf8Read(z_ನಮೂನೆ))!=0 ){
    ಆದರೆ( c==ಹೊಂದಿಕೆ_All ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      ಆಗ( (c=Utf8Read(z_ನಮೂನೆ)) == ಹೊಂದಿಕೆ_All
             || (c == ಹೊಂದಿಕೆ_One && ಹೊಂದಿಕೆ_One!=0) ){
        ಆದರೆ( c==ಹೊಂದಿಕೆ_One && sqlite3Utf8Read(&zString)==0 ){
          ಹಿಂತಿರುಗಿ SQLITE_NOWILDCARDMATCH;
        }
      }
      ಆದರೆ( c==0 ){
        ಹಿಂತಿರುಗಿ SQLITE_ಹೊಂದಿಕೆ;   /* "*" at the end of the pattern matches */
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c==ಹೊಂದಿಕೆ_Other ){
        ಆದರೆ( pInfo->ಹೊಂದಿಕೆ_ಗಣ==0 ){
          c = sqlite3Utf8Read(&z_ನಮೂನೆ);
          ಆದರೆ( c==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOWILDCARDMATCH;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          ಪ್ರತಿಜ್ಞಾ( ಹೊಂದಿಕೆ_Other<0x80 );  /* '[' is a single-byte character */
          ಆಗ( *zString ){
            ಪೂರ್ಣಾಂಕ b_ಹೊಂದಿಕೆ = ನಮೂನೆ_Compare(&z_ನಮೂನೆ[-1],zString,pInfo,ಹೊಂದಿಕೆ_Other);
            ಆದರೆ( b_ಹೊಂದಿಕೆ!=SQLITE_NOMATCH ) ಹಿಂತಿರುಗಿ b_ಹೊಂದಿಕೆ;
            SQLITE_SKIP_UTF8(zString);
          }
          ಹಿಂತಿರುಗಿ SQLITE_NOWILDCARDMATCH;
        }
      }

      /* At this point variable c contains the first character of the
      ** pattern string past the "*".  Search in the input string for the
      ** first matching character and recursively continue the match from
      ** that point.
      **
      ** For a case-insensitive search, set variable cx to be the same as
      ** c but in the other case and search the input string for either
      ** c or cx.
      */
      ಆದರೆ( c<0x80 ){
        ಅಕ್ಷರ zStop[3];
        ಪೂರ್ಣಾಂಕ b_ಹೊಂದಿಕೆ;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = z_ನಮೂನೆ[0], ny = nx;
        ಆದರೆ( nx>=0x80 || nx==ಹೊಂದಿಕೆ_All || nx==ಹೊಂದಿಕೆ_One || nx==ಹೊಂದಿಕೆ_Other ){
          nx = 0;
        }
        ಆದರೆ( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }ಇಲ್ಲದಿದ್ದರೆ{
          zStop[0] = c;
          zStop[1] = 0;
        }
        ಆಗ(1){
          zString += strcspn((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zString, zStop);
          ಆದರೆ( zString[0]==0 ) ನಿಲ್ಲಿಸು;
          zString++;
          ಆದರೆ( nx && zString[0]!=nx && zString[0]!=ny ) ಮುಂದುವರಿ;
          b_ಹೊಂದಿಕೆ = ನಮೂನೆ_Compare(z_ನಮೂನೆ,zString,pInfo,ಹೊಂದಿಕೆ_Other);
          ಆದರೆ( b_ಹೊಂದಿಕೆ!=SQLITE_NOMATCH ) ಹಿಂತಿರುಗಿ b_ಹೊಂದಿಕೆ;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ b_ಹೊಂದಿಕೆ;
        ಆಗ( (c2 = Utf8Read(zString))!=0 ){
          ಆದರೆ( c2!=c ) ಮುಂದುವರಿ;
          b_ಹೊಂದಿಕೆ = ನಮೂನೆ_Compare(z_ನಮೂನೆ,zString,pInfo,ಹೊಂದಿಕೆ_Other);
          ಆದರೆ( b_ಹೊಂದಿಕೆ!=SQLITE_NOMATCH ) ಹಿಂತಿರುಗಿ b_ಹೊಂದಿಕೆ;
        }
      }
      ಹಿಂತಿರುಗಿ SQLITE_NOWILDCARDMATCH;
    }
    ಆದರೆ( c==ಹೊಂದಿಕೆ_Other ){
      ಆದರೆ( pInfo->ಹೊಂದಿಕೆ_ಗಣ==0 ){
        c = sqlite3Utf8Read(&z_ನಮೂನೆ);
        ಆದರೆ( c==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMATCH;
        zEscaped = z_ನಮೂನೆ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        u32 ಪೂರ್ವ_c = 0;
        ಪೂರ್ಣಾಂಕ ನೋಡಿದ = 0;
        ಪೂರ್ಣಾಂಕ invert = 0;
        c = sqlite3Utf8Read(&zString);
        ಆದರೆ( c==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&z_ನಮೂನೆ);
        ಆದರೆ( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&z_ನಮೂನೆ);
        }
        ಆದರೆ( c2==']' ){
          ಆದರೆ( c==']' ) ನೋಡಿದ = 1;
          c2 = sqlite3Utf8Read(&z_ನಮೂನೆ);
        }
        ಆಗ( c2 && c2!=']' ){
          ಆದರೆ( c2=='-' && z_ನಮೂನೆ[0]!=']' && z_ನಮೂನೆ[0]!=0 && ಪೂರ್ವ_c>0 ){
            c2 = sqlite3Utf8Read(&z_ನಮೂನೆ);
            ಆದರೆ( c>=ಪೂರ್ವ_c && c<=c2 ) ನೋಡಿದ = 1;
            ಪೂರ್ವ_c = 0;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಆದರೆ( c==c2 ){
              ನೋಡಿದ = 1;
            }
            ಪೂರ್ವ_c = c2;
          }
          c2 = sqlite3Utf8Read(&z_ನಮೂನೆ);
        }
        ಆದರೆ( c2==0 || (ನೋಡಿದ ^ invert)==0 ){
          ಹಿಂತಿರುಗಿ SQLITE_NOMATCH;
        }
        ಮುಂದುವರಿ;
      }
    }
    c2 = Utf8Read(zString);
    ಆದರೆ( c==c2 ) ಮುಂದುವರಿ;
    ಆದರೆ( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( c==ಹೊಂದಿಕೆ_One && z_ನಮೂನೆ!=zEscaped && c2!=0 ) ಮುಂದುವರಿ;
    ಹಿಂತಿರುಗಿ SQLITE_NOMATCH;
  }
  ಹಿಂತಿರುಗಿ *zString==0 ? SQLITE_ಹೊಂದಿಕೆ : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
ಪೂರ್ಣಾಂಕ sqlite3_strglob(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_Glob_ನಮೂನೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zString){
  ಆದರೆ( zString==0 ){
    ಹಿಂತಿರುಗಿ z_Glob_ನಮೂನೆ!=0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_Glob_ನಮೂನೆ==0 ){
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ ನಮೂನೆ_Compare((u8*)z_Glob_ನಮೂನೆ, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
ಪೂರ್ಣಾಂಕ sqlite3_strlike(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ನಮೂನೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zStr, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ esc){
  ಆದರೆ( zStr==0 ){
    ಹಿಂತಿರುಗಿ z_ನಮೂನೆ!=0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ನಮೂನೆ==0 ){
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ ನಮೂನೆ_Compare((u8*)z_ನಮೂನೆ, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_like_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ


/*
** Implementation of the like() SQL function.  This function implements
** the built-in LIKE operator.  The first argument to the function is the
** pattern and the second argument is the string.  So, the SQL statements:
**
**       A LIKE B
**
** is implemented as like(B,A).
**
** This same function (with a different compareInfo structure) computes
** the GLOB operator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ likeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zA, *zB;
  u32 escape;
  ಪೂರ್ಣಾಂಕ nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ರಚನೆ compareInfo *pInfo = sqlite3_user_ದತ್ತಾಂಶ(context);
  ರಚನೆ compareInfo backupInfo;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_BLOB
   || sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_BLOB
  ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
    sqlite3_like_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_ಫಲಿತಾಂಶ_int(context, 0);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  testcase( nPat==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ] );
  testcase( nPat==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ]+1 );
  ಆದರೆ( nPat > db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ] ){
    sqlite3_ಫಲಿತಾಂಶ_error(context, "LIKE or GLOB pattern too complex", -1);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zEsc = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[2]);
    ಆದರೆ( zEsc==0 ) ಹಿಂತಿರುಗಿ;
    ಆದರೆ( sqlite3Utf8CharLen((ಅಕ್ಷರ*)zEsc, -1)!=1 ){
      sqlite3_ಫಲಿತಾಂಶ_error(context,
          "ESCAPE expression must be a single character", -1);
      ಹಿಂತಿರುಗಿ;
    }
    escape = sqlite3Utf8Read(&zEsc);
    ಆದರೆ( escape==pInfo->ಹೊಂದಿಕೆ_All || escape==pInfo->ಹೊಂದಿಕೆ_One ){
      ಸ್ಮರಣೆ_ನಕಲು(&backupInfo, pInfo, ಗಾತ್ರ(backupInfo));
      pInfo = &backupInfo;
      ಆದರೆ( escape==pInfo->ಹೊಂದಿಕೆ_All ) pInfo->ಹೊಂದಿಕೆ_All = 0;
      ಆದರೆ( escape==pInfo->ಹೊಂದಿಕೆ_One ) pInfo->ಹೊಂದಿಕೆ_One = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    escape = pInfo->ಹೊಂದಿಕೆ_ಗಣ;
  }
  zB = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  zA = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  ಆದರೆ( zA && zB ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
    sqlite3_like_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_ಫಲಿತಾಂಶ_int(context,
                      ನಮೂನೆ_Compare(zB, zA, pInfo, escape)==SQLITE_ಹೊಂದಿಕೆ);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
ಸ್ಥಿರ ಶೂನ್ಯ nullifFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಆದರೆ( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಆವೃತ್ತಿ_Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sourceidFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
ಸ್ಥಿರ ಶೂನ್ಯ errlogFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಬಳಸದ_PARAMETER(argc);
  ಬಳಸದ_PARAMETER(context);
  sqlite3_log(sqlite3_ಮೌಲ್ಯ_int(argv[0]), "%s", sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
ಸ್ಥಿರ ಶೂನ್ಯ compileoptionusedFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOptName;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  ಆದರೆ( (zOptName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]))!=0 ){
    sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3_compileoption_ಬಳಸಿದ(zOptName));
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
ಸ್ಥಿರ ಶೂನ್ಯ compileoptiongetFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  i64 n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRes;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((ಪೂರ್ಣಾಂಕ)n) : 0;
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, zRes, -1, SQLITE_STATIC);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
ಶೂನ್ಯ sqlite3_Quote_ಮೌಲ್ಯ(StrAccum *pStr, sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  ಪ್ರತಿಜ್ಞಾ( pStr!=0 && pStr->nChar==0 );

  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(p_ಮೌಲ್ಯ) ){
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_ಮೌಲ್ಯ_double(p_ಮೌಲ್ಯ));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_ಮೌಲ್ಯ_int64(p_ಮೌಲ್ಯ));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_BLOB: {
      ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zBlob = sqlite3_ಮೌಲ್ಯ_blob(p_ಮೌಲ್ಯ);
      i64 nBlob = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(p_ಮೌಲ್ಯ);
      ಪ್ರತಿಜ್ಞಾ( zBlob==sqlite3_ಮೌಲ್ಯ_blob(p_ಮೌಲ್ಯ) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      ಆದರೆ( pStr->accError==0 ){
        ಅಕ್ಷರ *z_ಪಠ್ಯ = pStr->z_ಪಠ್ಯ;
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ(i=0; i<nBlob; i++){
          z_ಪಠ್ಯ[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          z_ಪಠ್ಯ[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        z_ಪಠ್ಯ[(nBlob*2)+2] = '\'';
        z_ಪಠ್ಯ[(nBlob*2)+3] = '\0';
        z_ಪಠ್ಯ[0] = 'X';
        z_ಪಠ್ಯ[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ: {
      ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zArg = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಮೌಲ್ಯ);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಮೌಲ್ಯ_type(p_ಮೌಲ್ಯ)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isNHex(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ N, u32 *pVal){
  ಪೂರ್ಣಾಂಕ i;
  u32 v = 0;
  ಗಾಗಿ(i=0; i<N; i++){
    ಆದರೆ( !sqlite3Isxdigit(z[i]) ) ಹಿಂತಿರುಗಿ 0;
    v = (v<<4) + sqlite3_Hex_ಗಮ್ಯ_Int(z[i]);
  }
  *pVal = v;
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Implementation of the UNISTR() function.
**
** This is intended to be a work-alike of the UNISTR() function in
** PostgreSQL.  Quoting from the PG documentation (PostgreSQL 17 -
** scraped on 2025-02-22):
**
**    Evaluate escaped Unicode characters in the argument. Unicode
**    characters can be specified as \XXXX (4 hexadecimal digits),
**    \+XXXXXX (6 hexadecimal digits), \uXXXX (4 hexadecimal digits),
**    or \UXXXXXXXX (8 hexadecimal digits). To specify a backslash,
**    write two backslashes. All other characters are taken literally.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unistrFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ;
  ಪೂರ್ಣಾಂಕ n_ಒಳ;
  ಪೂರ್ಣಾಂಕ i, j, n;
  u32 v;

  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER( argc );
  z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಆದರೆ( z_ಒಳ==0 ) ಹಿಂತಿರುಗಿ;
  n_ಒಳ = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  z_ಹೊರಹೋಗು = sqlite3_malloc64(n_ಒಳ+1);
  ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    ಹಿಂತಿರುಗಿ;
  }
  i = j = 0;
  ಆಗ( i<n_ಒಳ ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = strchr(&z_ಒಳ[i],'\\');
    ಆದರೆ( z==0 ){
      n = n_ಒಳ - i;
      memmove(&z_ಹೊರಹೋಗು[j], &z_ಒಳ[i], n);
      j += n;
      ನಿಲ್ಲಿಸು;
    }
    n = z - &z_ಒಳ[i];
    ಆದರೆ( n>0 ){
      memmove(&z_ಹೊರಹೋಗು[j], &z_ಒಳ[i], n);
      j += n;
      i += n;
    }
    ಆದರೆ( z_ಒಳ[i+1]=='\\' ){
      i += 2;
      z_ಹೊರಹೋಗು[j++] = '\\';
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3Isxdigit(z_ಒಳ[i+1]) ){
      ಆದರೆ( !isNHex(&z_ಒಳ[i+1], 4, &v) ) ಹೋಗು unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&z_ಹೊರಹೋಗು[j], v);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಒಳ[i+1]=='+' ){
      ಆದರೆ( !isNHex(&z_ಒಳ[i+2], 6, &v) ) ಹೋಗು unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&z_ಹೊರಹೋಗು[j], v);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಒಳ[i+1]=='u' ){
      ಆದರೆ( !isNHex(&z_ಒಳ[i+2], 4, &v) ) ಹೋಗು unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&z_ಹೊರಹೋಗು[j], v);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಒಳ[i+1]=='U' ){
      ಆದರೆ( !isNHex(&z_ಒಳ[i+2], 8, &v) ) ಹೋಗು unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&z_ಹೊರಹೋಗು[j], v);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹೋಗು unistr_error;
    }
  }
  z_ಹೊರಹೋಗು[j] = 0;
  sqlite3_ಫಲಿತಾಂಶ_text64(context, z_ಹೊರಹೋಗು, j, sqlite3_free, SQLITE_UTF8_ZT);
  ಹಿಂತಿರುಗಿ;

unistr_error:
  sqlite3_free(z_ಹೊರಹೋಗು);
  sqlite3_ಫಲಿತಾಂಶ_error(context, "invalid Unicode escape", -1);
  ಹಿಂತಿರುಗಿ;
}


/*
** Implementation of the QUOTE() function. 
**
** The quote(X) function returns the text of an SQL literal which is the
** value of its argument suitable for inclusion into an SQL statement.
** Strings are surrounded by single-quotes with escapes on interior quotes
** as needed. BLOBs are encoded as hexadecimal literals. Strings with
** embedded NUL characters cannot be represented as string literals in SQL
** and hence the returned string literal is truncated prior to the first NUL.
**
** If sqlite3_user_data() is non-zero, then the UNISTR_QUOTE() function is
** implemented instead.  The difference is that UNISTR_QUOTE() uses the
** UNISTR() function to escape control characters.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quoteFunc(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]);
  sqlite3_Quote_ಮೌಲ್ಯ(&str,argv[0],SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(context)));
  sqlite3_ಫಲಿತಾಂಶ_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unicodeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  (ಶೂನ್ಯ)argc;
  ಆದರೆ( z && z[0] ) sqlite3_ಫಲಿತಾಂಶ_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ charFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z, *z_ಹೊರಹೋಗು;
  ಪೂರ್ಣಾಂಕ i;
  z_ಹೊರಹೋಗು = z = sqlite3_malloc64( argc*4+1 );
  ಆದರೆ( z==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    ಹಿಂತಿರುಗಿ;
  }
  ಗಾಗಿ(i=0; i<argc; i++){
    sqlite3_int64 x;
    ಚಿಹ್ನೆರಹಿತ c;
    x = sqlite3_ಮೌಲ್ಯ_int64(argv[i]);
    ಆದರೆ( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (ಚಿಹ್ನೆರಹಿತ)(x & 0x1fffff);
    ಆದರೆ( c<0x00080 ){
      *z_ಹೊರಹೋಗು++ = (u8)(c&0xFF);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c<0x00800 ){
      *z_ಹೊರಹೋಗು++ = 0xC0 + (u8)((c>>6)&0x1F);
      *z_ಹೊರಹೋಗು++ = 0x80 + (u8)(c & 0x3F);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c<0x10000 ){
      *z_ಹೊರಹೋಗು++ = 0xE0 + (u8)((c>>12)&0x0F);
      *z_ಹೊರಹೋಗು++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_ಹೊರಹೋಗು++ = 0x80 + (u8)(c & 0x3F);
    }ಇಲ್ಲದಿದ್ದರೆ{
      *z_ಹೊರಹೋಗು++ = 0xF0 + (u8)((c>>18) & 0x07);
      *z_ಹೊರಹೋಗು++ = 0x80 + (u8)((c>>12) & 0x3F);
      *z_ಹೊರಹೋಗು++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_ಹೊರಹೋಗು++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *z_ಹೊರಹೋಗು = 0;
  sqlite3_ಫಲಿತಾಂಶ_text64(context, (ಅಕ್ಷರ*)z, z_ಹೊರಹೋಗು-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
ಸ್ಥಿರ ಶೂನ್ಯ hexFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i, n;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pBlob;
  ಅಕ್ಷರ *zHex, *z;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  pBlob = sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
  n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( pBlob==sqlite3_ಮೌಲ್ಯ_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  ಆದರೆ( zHex ){
    ಗಾಗಿ(i=0; i<n; i++, pBlob++){
      ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_ಫಲಿತಾಂಶ_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಒಳಗೊಂಡಿದೆ_Char(ಸ್ಥಿರಾಂಕ u8 *zStr, ಪೂರ್ಣಾಂಕ nStr, u32 ch){
  ಸ್ಥಿರಾಂಕ u8 *z_ಕೊನೆ = &zStr[nStr];
  ಸ್ಥಿರಾಂಕ u8 *z = zStr;
  ಆಗ( z<z_ಕೊನೆ ){
    u32 tst = Utf8Read(z);
    ಆದರೆ( tst==ch ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** The unhex() function. This function may be invoked with either one or
** two arguments. In both cases the first argument is interpreted as text
** a text value containing a set of pairs of hexadecimal digits which are
** decoded and returned as a blob.
**
** If there is only a single argument, then it must consist only of an
** even number of hexadecimal digits. Otherwise, return NULL.
**
** Or, if there is a second argument, then any character that appears in
** the second argument is also allowed to appear between pairs of hexadecimal
** digits in the first argument. If any other character appears in the
** first argument, or if one of the allowed characters appears between
** two hexadecimal digits that make up a single byte, NULL is returned.
**
** The following expressions are all true:
**
**     unhex('ABCD')       IS x'ABCD'
**     unhex('AB CD')      IS NULL
**     unhex('AB CD', ' ') IS x'ABCD'
**     unhex('A BCD', ' ') IS NULL
*/
ಸ್ಥಿರ ಶೂನ್ಯ unhexFunc(
  sqlite3_context *pCtx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ u8 *zPass = (ಸ್ಥಿರಾಂಕ u8*)"";
  ಪೂರ್ಣಾಂಕ nPass = 0;
  ಸ್ಥಿರಾಂಕ u8 *zHex = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಪೂರ್ಣಾಂಕ nHex = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಸ್ಥಿರಾಂಕ u8 *z_ಕೊನೆ = zHex ? &zHex[nHex] : 0;
#ಆದರೆ_ಅಂತ್ಯ
  u8 *pBlob = 0;
  u8 *p = 0;

  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಆದರೆ( argc==2 ){
    zPass = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
    nPass = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[1]);
  }
  ಆದರೆ( !zHex || !zPass ) ಹಿಂತಿರುಗಿ;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  ಆದರೆ( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    ಆಗ( (c = *zHex)!=0x00 ){
      ಆಗ( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        ಪ್ರತಿಜ್ಞಾ( zHex<=z_ಕೊನೆ );
        ಆದರೆ( !str_ಒಳಗೊಂಡಿದೆ_Char(zPass, nPass, ch) ) ಹೋಗು unhex_null;
        c = *zHex;
        ಆದರೆ( c==0x00 ) ಹೋಗು unhex_ಮುಗಿದ;
      }
      zHex++;
      ಪ್ರತಿಜ್ಞಾ( *z_ಕೊನೆ==0x00 );
      ಪ್ರತಿಜ್ಞಾ( zHex<=z_ಕೊನೆ );
      d = *(zHex++);
      ಆದರೆ( !sqlite3Isxdigit(d) ) ಹೋಗು unhex_null;
      *(p++) = (sqlite3_Hex_ಗಮ್ಯ_Int(c)<<4) | sqlite3_Hex_ಗಮ್ಯ_Int(d);
    }
  }

 unhex_ಮುಗಿದ:
  sqlite3_ಫಲಿತಾಂಶ_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  ಹಿಂತಿರುಗಿ;

 unhex_null:
  sqlite3_free(pBlob);
  ಹಿಂತಿರುಗಿ;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ zeroblobFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  i64 n;
  ಪೂರ್ಣಾಂಕ rc;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  n = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
  ಆದರೆ( n<0 ) n = 0;
  rc = sqlite3_ಫಲಿತಾಂಶ_zeroblob64(context, n); /* IMP: R-00293-64994 */
  ಆದರೆ( rc ){
    sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
ಸ್ಥಿರ ಶೂನ್ಯ replaceFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zStr;        /* The input string A */
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ನಮೂನೆ;    /* The pattern string B */
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zRep;        /* The replacement string C */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಹೊರಹೋಗು;              /* The output */
  ಪೂರ್ಣಾಂಕ nStr;                /* Size of zStr */
  ಪೂರ್ಣಾಂಕ n_ನಮೂನೆ;            /* Size of zPattern */
  ಪೂರ್ಣಾಂಕ nRep;                /* Size of zRep */
  i64 n_ಹೊರಹೋಗು;                /* Maximum size of zOut */
  ಪೂರ್ಣಾಂಕ loop_ಎಲ್ಲೆ;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  ಚಿಹ್ನೆರಹಿತ cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  ಪ್ರತಿಜ್ಞಾ( argc==3 );
  ಬಳಸದ_PARAMETER(argc);
  zStr = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಆದರೆ( zStr==0 ) ಹಿಂತಿರುಗಿ;
  nStr = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( zStr==sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]) );  /* No encoding change */
  z_ನಮೂನೆ = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  ಆದರೆ( z_ನಮೂನೆ==0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( z_ನಮೂನೆ[0]==0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಮೌಲ್ಯ_type(argv[1])!=SQLITE_NULL );
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zStr, nStr, SQLITE_TRANSIENT);
    ಹಿಂತಿರುಗಿ;
  }
  n_ನಮೂನೆ = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[1]);
  ಪ್ರತಿಜ್ಞಾ( z_ನಮೂನೆ==sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]) );  /* No encoding change */
  zRep = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[2]);
  ಆದರೆ( zRep==0 ) ಹಿಂತಿರುಗಿ;
  nRep = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[2]);
  ಪ್ರತಿಜ್ಞಾ( zRep==sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[2]) );
  n_ಹೊರಹೋಗು = nStr + 1;
  ಪ್ರತಿಜ್ಞಾ( n_ಹೊರಹೋಗು<SQLITE_MAX_ಉದ್ದಳತೆ );
  z_ಹೊರಹೋಗು = contextMalloc(context, n_ಹೊರಹೋಗು);
  ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
    ಹಿಂತಿರುಗಿ;
  }
  loop_ಎಲ್ಲೆ = nStr - n_ನಮೂನೆ; 
  cntExpand = 0;
  ಗಾಗಿ(i=j=0; i<=loop_ಎಲ್ಲೆ; i++){
    ಆದರೆ( zStr[i]!=z_ನಮೂನೆ[0] || memcmp(&zStr[i], z_ನಮೂನೆ, n_ನಮೂನೆ) ){
      z_ಹೊರಹೋಗು[j++] = zStr[i];
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( nRep>n_ನಮೂನೆ ){
        n_ಹೊರಹೋಗು += nRep - n_ನಮೂನೆ;
        testcase( n_ಹೊರಹೋಗು-1==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] );
        testcase( n_ಹೊರಹೋಗು-2==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] );
        ಆದರೆ( n_ಹೊರಹೋಗು-1>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
          sqlite3_ಫಲಿತಾಂಶ_error_toobig(context);
          sqlite3_free(z_ಹೊರಹೋಗು);
          ಹಿಂತಿರುಗಿ;
        }
        cntExpand++;
        ಆದರೆ( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = z_ಹೊರಹೋಗು;
          z_ಹೊರಹೋಗು = sqlite3Realloc(z_ಹೊರಹೋಗು, (ಪೂರ್ಣಾಂಕ)n_ಹೊರಹೋಗು + (n_ಹೊರಹೋಗು - nStr - 1));
          ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
            sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
            sqlite3_free(zOld);
            ಹಿಂತಿರುಗಿ;
          }
        }
      }
      ಸ್ಮರಣೆ_ನಕಲು(&z_ಹೊರಹೋಗು[j], zRep, nRep);
      j += nRep;
      i += n_ನಮೂನೆ-1;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( j+nStr-i+1<=n_ಹೊರಹೋಗು );
  ಸ್ಮರಣೆ_ನಕಲು(&z_ಹೊರಹೋಗು[j], &zStr[i], nStr-i);
  j += nStr - i;
  ಪ್ರತಿಜ್ಞಾ( j<=n_ಹೊರಹೋಗು );
  z_ಹೊರಹೋಗು[j] = 0;
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, (ಅಕ್ಷರ*)z_ಹೊರಹೋಗು, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
ಸ್ಥಿರ ಶೂನ್ಯ trimFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಒಳ;         /* Input string */
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_Char_ಗಣ;    /* Set of characters to trim */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_ಒಳ;                 /* Number of bytes in input */
  ಪೂರ್ಣಾಂಕ flags;                        /* 1: trimleft  2: trimright  3: trim */
  ಪೂರ್ಣಾಂಕ i;                            /* Loop counter */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *aLen = 0;           /* Length of each character in zCharSet */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **azChar = 0;       /* Individual characters in zCharSet */
  ಪೂರ್ಣಾಂಕ nChar;                        /* Number of characters in zCharSet */

  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ){
    ಹಿಂತಿರುಗಿ;
  }
  z_ಒಳ = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಆದರೆ( z_ಒಳ==0 ) ಹಿಂತಿರುಗಿ;
  n_ಒಳ = (ಚಿಹ್ನೆರಹಿತ)sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( z_ಒಳ==sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]) );
  ಆದರೆ( argc==1 ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ lenOne[] = { 1 };
    ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (ಚಿಹ್ನೆರಹಿತ*)lenOne;
    azChar = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **)azOne;
    z_Char_ಗಣ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (z_Char_ಗಣ = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]))==0 ){
    ಹಿಂತಿರುಗಿ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z;
    ಗಾಗಿ(z=z_Char_ಗಣ, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    ಆದರೆ( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(ಗಾತ್ರ(ಅಕ್ಷರ*)+ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ)));
      ಆದರೆ( azChar==0 ){
        ಹಿಂತಿರುಗಿ;
      }
      aLen = (ಚಿಹ್ನೆರಹಿತ*)&azChar[nChar];
      ಗಾಗಿ(z=z_Char_ಗಣ, nChar=0; *z; nChar++){
        azChar[nChar] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (ಚಿಹ್ನೆರಹಿತ)(z - azChar[nChar]);
      }
    }
  }
  ಆದರೆ( nChar>0 ){
    flags = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(context));
    ಆದರೆ( flags & 1 ){
      ಆಗ( n_ಒಳ>0 ){
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len = 0;
        ಗಾಗಿ(i=0; i<nChar; i++){
          len = aLen[i];
          ಆದರೆ( len<=n_ಒಳ && memcmp(z_ಒಳ, azChar[i], len)==0 ) ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( i>=nChar ) ನಿಲ್ಲಿಸು;
        z_ಒಳ += len;
        n_ಒಳ -= len;
      }
    }
    ಆದರೆ( flags & 2 ){
      ಆಗ( n_ಒಳ>0 ){
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len = 0;
        ಗಾಗಿ(i=0; i<nChar; i++){
          len = aLen[i];
          ಆದರೆ( len<=n_ಒಳ && memcmp(&z_ಒಳ[n_ಒಳ-len],azChar[i],len)==0 ) ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( i>=nChar ) ನಿಲ್ಲಿಸು;
        n_ಒಳ -= len;
      }
    }
    ಆದರೆ( z_Char_ಗಣ ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, (ಅಕ್ಷರ*)z_ಒಳ, n_ಒಳ, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ concatFuncCore(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  ಪೂರ್ಣಾಂಕ nSep,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSep
){
  i64 j, n = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ bNotNull = 0;   /* True after at least NOT NULL argument seen */
  ಅಕ್ಷರ *z;
  ಗಾಗಿ(i=0; i<argc; i++){
    n += sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  ಆದರೆ( z==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    ಹಿಂತಿರುಗಿ;
  }
  j = 0;
  ಗಾಗಿ(i=0; i<argc; i++){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[i])!=SQLITE_NULL ){
      ಪೂರ್ಣಾಂಕ k = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[i]);
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *v = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
      ಆದರೆ( v!=0 ){
        ಆದರೆ( bNotNull && nSep>0 ){
          ಸ್ಮರಣೆ_ನಕಲು(&z[j], zSep, nSep);
          j += nSep;
        }
        ಸ್ಮರಣೆ_ನಕಲು(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  ಪ್ರತಿಜ್ಞಾ( j<=n );
  sqlite3_ಫಲಿತಾಂಶ_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
ಸ್ಥಿರ ಶೂನ್ಯ concatFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  concatFuncCore(context, argc, argv, 0, "");
}

/*
** The CONCAT_WS(separator, ...) function.
**
** Generate a string that is the concatenation of 2nd through the Nth
** argument.  Use the first argument (which must be non-NULL) as the
** separator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ concatwsFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ nSep = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSep = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಆದರೆ( zSep==0 ) ಹಿಂತಿರುಗಿ;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unknownFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  /* no-op */
  (ಶೂನ್ಯ)context;
  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)argv;
}
#ಆದರೆ_ಅಂತ್ಯ /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
ಸ್ಥಿರ ಶೂನ್ಯ soundexFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಅಕ್ಷರ z_ಫಲಿತಾಂಶ[8];
  ಸ್ಥಿರಾಂಕ u8 *z_ಒಳ;
  ಪೂರ್ಣಾಂಕ i, j;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ i_ಸಂಕೇತ[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  z_ಒಳ = (u8*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಆದರೆ( z_ಒಳ==0 ) z_ಒಳ = (u8*)"";
  ಗಾಗಿ(i=0; z_ಒಳ[i] && !sqlite3Isalpha(z_ಒಳ[i]); i++){}
  ಆದರೆ( z_ಒಳ[i] ){
    u8 prevcode = i_ಸಂಕೇತ[z_ಒಳ[i]&0x7f];
    z_ಫಲಿತಾಂಶ[0] = sqlite3Toupper(z_ಒಳ[i]);
    ಗಾಗಿ(j=1; j<4 && z_ಒಳ[i]; i++){
      ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = i_ಸಂಕೇತ[z_ಒಳ[i]&0x7f];
      ಆದರೆ( ಸಂಕೇತ_2>0 ){
        ಆದರೆ( ಸಂಕೇತ_2!=prevcode ){
          prevcode = ಸಂಕೇತ_2;
          z_ಫಲಿತಾಂಶ[j++] = ಸಂಕೇತ_2 + '0';
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        prevcode = 0;
      }
    }
    ಆಗ( j<4 ){
      z_ಫಲಿತಾಂಶ[j++] = '0';
    }
    z_ಫಲಿತಾಂಶ[j] = 0;
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, z_ಫಲಿತಾಂಶ, 4, SQLITE_TRANSIENT);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, "?000", 4, SQLITE_STATIC);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_SOUNDEX */

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
ಸ್ಥಿರ ಶೂನ್ಯ oomFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Ext(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  ಆದರೆ( (db->flags & SQLITE_ತುಂಬು_Ext_Func)==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error(context, "not authorized", -1);
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ( argc==2 ){
    zProc = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  }ಇಲ್ಲದಿದ್ದರೆ{
    zProc = 0;
  }
  ಆದರೆ( zFile && sqlite3_ತುಂಬು_extension(db, zFile, zProc, &z_Err_ಸಂದೇಶ) ){
    sqlite3_ಫಲಿತಾಂಶ_error(context, z_Err_ಸಂದೇಶ, -1);
    sqlite3_free(z_Err_ಸಂದೇಶ);
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಮೊತ್ತ_Ctx ಮೊತ್ತ_Ctx;
ರಚನೆ ಮೊತ್ತ_Ctx {
  ದ್ವಿಗುಣ r_ಮೊತ್ತ;      /* Running sum as as a double */
  ದ್ವಿಗುಣ rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 i_ಮೊತ್ತ;         /* Running sum as a signed integer */
  i64 cnt;          /* Number of elements summed */
  u8 approx;        /* True if any non-integer value was input to the sum */
  u8 ovrfl;         /* Integer overflow seen */
};

/*
** Do one step of the Kahan-Babushka-Neumaier summation.
**
** https://en.wikipedia.org/wiki/Kahan_summation_algorithm
**
** Variables are marked "volatile" to defeat c89 x86 floating point
** optimizations can mess up this algorithm.
*/
ಸ್ಥಿರ ಶೂನ್ಯ kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(
  ಅಸ್ಥಿರ ಮೊತ್ತ_Ctx *p_ಮೊತ್ತ,
  ಅಸ್ಥಿರ ದ್ವಿಗುಣ r
){
  ಅಸ್ಥಿರ ದ್ವಿಗುಣ s = p_ಮೊತ್ತ->r_ಮೊತ್ತ;
  ಅಸ್ಥಿರ ದ್ವಿಗುಣ t = s + r;
  ಆದರೆ( fabs(s) > fabs(r) ){
    p_ಮೊತ್ತ->rErr += (s - t) + r;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಮೊತ್ತ->rErr += (r - t) + s;
  }
  p_ಮೊತ್ತ->r_ಮೊತ್ತ = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
ಸ್ಥಿರ ಶೂನ್ಯ kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ_Int64(ಅಸ್ಥಿರ ಮೊತ್ತ_Ctx *p_ಮೊತ್ತ, i64 iVal){
  ಆದರೆ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(p_ಮೊತ್ತ, iBig);
    kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(p_ಮೊತ್ತ, iSm);
  }ಇಲ್ಲದಿದ್ದರೆ{
    kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(p_ಮೊತ್ತ, (ದ್ವಿಗುಣ)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
ಸ್ಥಿರ ಶೂನ್ಯ kahanBabuskaNeumaierInit(
  ಅಸ್ಥಿರ ಮೊತ್ತ_Ctx *p,
  i64 iVal
){
  ಆದರೆ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_ಮೊತ್ತ = (ದ್ವಿಗುಣ)(iVal - iSm);
    p->rErr = (ದ್ವಿಗುಣ)iSm;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->r_ಮೊತ್ತ = (ದ್ವಿಗುಣ)iVal;
    p->rErr = 0.0;
  }
}

/*
** Routines used to compute the sum, average, and total.
**
** The SUM() function follows the (broken) SQL standard which means
** that it returns NULL if it sums over no inputs.  TOTAL returns
** 0.0 in that case.  In addition, TOTAL always returns a float where
** SUM might return an integer if it never encounters a floating point
** value.  TOTAL never fails, but SUM might throw an exception if
** it overflows an integer.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಮೊತ್ತ_ಹೆಜ್ಜೆ(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಮೊತ್ತ_Ctx *p;
  ಪೂರ್ಣಾಂಕ type;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, ಗಾತ್ರ(*p));
  type = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಆದರೆ( p && type!=SQLITE_NULL ){
    p->cnt++;
    ಆದರೆ( p->approx==0 ){
      ಆದರೆ( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_ಮೊತ್ತ);
        p->approx = 1;
        kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(p, sqlite3_ಮೌಲ್ಯ_double(argv[0]));
      }ಇಲ್ಲದಿದ್ದರೆ{
        i64 x = p->i_ಮೊತ್ತ;
        ಆದರೆ( sqlite3_ಕೂಡಿಸು_Int64(&x, sqlite3_ಮೌಲ್ಯ_int64(argv[0]))==0 ){
          p->i_ಮೊತ್ತ = x;
        }ಇಲ್ಲದಿದ್ದರೆ{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_ಮೊತ್ತ);
          p->approx = 1;
          kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ_Int64(p, sqlite3_ಮೌಲ್ಯ_int64(argv[0]));
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( type==SQLITE_INTEGER ){
        kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ_Int64(p, sqlite3_ಮೌಲ್ಯ_int64(argv[0]));
      }ಇಲ್ಲದಿದ್ದರೆ{
        p->ovrfl = 0;
        kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(p, sqlite3_ಮೌಲ್ಯ_double(argv[0]));
      }
    }
  }
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ ಮೊತ್ತ_Inverse(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ**argv){
  ಮೊತ್ತ_Ctx *p;
  ಪೂರ್ಣಾಂಕ type;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, ಗಾತ್ರ(*p));
  type = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  ಆದರೆ( ALWAYS(p) && type!=SQLITE_NULL ){
    ಪ್ರತಿಜ್ಞಾ( p->cnt>0 );
    p->cnt--;
    ಆದರೆ( !p->approx ){
      i64 x = p->i_ಮೊತ್ತ;
      ಆದರೆ( sqlite3SubInt64(&x, sqlite3_ಮೌಲ್ಯ_int64(argv[0]))==0 ){
        p->i_ಮೊತ್ತ = x;
        ಹಿಂತಿರುಗಿ;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_ಮೊತ್ತ);
    }
    ಆದರೆ( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
      ಆದರೆ( iVal!=ಅತಿಚಿಕ್ಕ_INT64 ){
        kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ_Int64(p, -iVal);
      }ಇಲ್ಲದಿದ್ದರೆ{
        kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ_Int64(p, ಅತಿದೊಡ್ಡ_INT64);
        kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ_Int64(p, 1);
      }       
    }ಇಲ್ಲದಿದ್ದರೆ{
      kahan_Babuska_Neumaier_ಹೆಜ್ಜೆ(p, -sqlite3_ಮೌಲ್ಯ_double(argv[0]));
    }
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಮೊತ್ತ_Inverse 0
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
ಸ್ಥಿರ ಶೂನ್ಯ ಮೊತ್ತ_Finalize(sqlite3_context *context){
  ಮೊತ್ತ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  ಆದರೆ( p && p->cnt>0 ){
    ಆದರೆ( p->approx ){
      ಆದರೆ( p->ovrfl ){
        sqlite3_ಫಲಿತಾಂಶ_error(context,"integer overflow",-1);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !sqlite3_Is_ಅತಿಪ್ರವಾಹ(p->rErr) ){
        sqlite3_ಫಲಿತಾಂಶ_double(context, p->r_ಮೊತ್ತ+p->rErr);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_double(context, p->r_ಮೊತ್ತ);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_int64(context, p->i_ಮೊತ್ತ);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ avgFinalize(sqlite3_context *context){
  ಮೊತ್ತ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  ಆದರೆ( p && p->cnt>0 ){
    ದ್ವಿಗುಣ r;
    ಆದರೆ( p->approx ){
      r = p->r_ಮೊತ್ತ;
      ಆದರೆ( !sqlite3_Is_ಅತಿಪ್ರವಾಹ(p->rErr) ) r += p->rErr;
    }ಇಲ್ಲದಿದ್ದರೆ{
      r = (ದ್ವಿಗುಣ)(p->i_ಮೊತ್ತ);
    }
    sqlite3_ಫಲಿತಾಂಶ_double(context, r/(ದ್ವಿಗುಣ)p->cnt);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ ಒಟ್ಟು_Finalize(sqlite3_context *context){
  ಮೊತ್ತ_Ctx *p;
  ದ್ವಿಗುಣ r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  ಆದರೆ( p ){
    ಆದರೆ( p->approx ){
      r = p->r_ಮೊತ್ತ;
      ಆದರೆ( !sqlite3_Is_ಅತಿಪ್ರವಾಹ(p->rErr) ) r += p->rErr;
    }ಇಲ್ಲದಿದ್ದರೆ{
      r = (ದ್ವಿಗುಣ)(p->i_ಮೊತ್ತ);
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಎಣಿಕೆ_Ctx ಎಣಿಕೆ_Ctx;
ರಚನೆ ಎಣಿಕೆ_Ctx {
  i64 n;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪೂರ್ಣಾಂಕ bInverse;                   /* True if xInverse() ever called */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** Routines to implement the count() aggregate function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಎಣಿಕೆ_ಹೆಜ್ಜೆ(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಎಣಿಕೆ_Ctx *p;
  p = sqlite3_aggregate_context(context, ಗಾತ್ರ(*p));
  ಆದರೆ( (argc==0 || SQLITE_NULL!=sqlite3_ಮೌಲ್ಯ_type(argv[0])) && p ){
    p->n++;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  ಪ್ರತಿಜ್ಞಾ( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_ಎಣಿಕೆ(context) );
#ಆದರೆ_ಅಂತ್ಯ
}  
ಸ್ಥಿರ ಶೂನ್ಯ ಎಣಿಕೆ_Finalize(sqlite3_context *context){
  ಎಣಿಕೆ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_ಫಲಿತಾಂಶ_int64(context, p ? p->n : 0);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ ಎಣಿಕೆ_Inverse(sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಎಣಿಕೆ_Ctx *p;
  p = sqlite3_aggregate_context(ctx, ಗಾತ್ರ(*p));
  /* p is always non-NULL since countStep() will have been called first */
  ಆದರೆ( (argc==0 || SQLITE_NULL!=sqlite3_ಮೌಲ್ಯ_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    p->bInverse = 1;
#ಆದರೆ_ಅಂತ್ಯ
  }
}  
#ಇಲ್ಲದಿದ್ದರೆ
# define ಎಣಿಕೆ_Inverse 0
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
ಸ್ಥಿರ ಶೂನ್ಯ minmax_ಹೆಜ್ಜೆ(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *p_ಉತ್ತಮ;
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  p_ಉತ್ತಮ = (Mem *)sqlite3_aggregate_context(context, ಗಾತ್ರ(*p_ಉತ್ತಮ));
  ಆದರೆ( !p_ಉತ್ತಮ ) ಹಿಂತಿರುಗಿ;

  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(pArg)==SQLITE_NULL ){
    ಆದರೆ( p_ಉತ್ತಮ->flags ) sqlite3_Skip_ಸಂಚಯಕ_ತುಂಬು(context);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಉತ್ತಮ->flags ){
    ಪೂರ್ಣಾಂಕ max;
    ಪೂರ್ಣಾಂಕ cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_ದತ್ತಾಂಶ(context)!=0;
    cmp = sqlite3MemCompare(p_ಉತ್ತಮ, pArg, pColl);
    ಆದರೆ( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(p_ಉತ್ತಮ, pArg);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Skip_ಸಂಚಯಕ_ತುಂಬು(context);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಉತ್ತಮ->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(p_ಉತ್ತಮ, pArg);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ min_Max_ಮೌಲ್ಯ_Finalize(sqlite3_context *context, ಪೂರ್ಣಾಂಕ b_ಮೌಲ್ಯ){
  sqlite3_ಮೌಲ್ಯ *pRes;
  pRes = (sqlite3_ಮೌಲ್ಯ *)sqlite3_aggregate_context(context, 0);
  ಆದರೆ( pRes ){
    ಆದರೆ( pRes->flags ){
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, pRes);
    }
    ಆದರೆ( b_ಮೌಲ್ಯ==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ min_Max_ಮೌಲ್ಯ(sqlite3_context *context){
  min_Max_ಮೌಲ್ಯ_Finalize(context, 1);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define min_Max_ಮೌಲ್ಯ 0
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
ಸ್ಥಿರ ಶೂನ್ಯ minMaxFinalize(sqlite3_context *context){
  min_Max_ಮೌಲ್ಯ_Finalize(context, 0);
}

/*
** group_concat(EXPR, ?SEPARATOR?)
** string_agg(EXPR, SEPARATOR)
**
** Content is accumulated in GroupConcatCtx.str with the SEPARATOR
** coming before the EXPR value, except for the first entry which
** omits the SEPARATOR.
**
** It is tragic that the SEPARATOR goes before the EXPR string.  The
** groupConcatInverse() implementation would have been easier if the
** SEPARATOR were appended after EXPR.  And the order is undocumented,
** so we could change it, in theory.  But the old behavior has been
** around for so long that we dare not, for fear of breaking something.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  StrAccum str;          /* The accumulated concatenation */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಪೂರ್ಣಾಂಕ nAccum;            /* Number of strings presently concatenated */
  ಪೂರ್ಣಾಂಕ n_ಮೊದಲ_Sep_ಉದ್ದಳತೆ;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  ಪೂರ್ಣಾಂಕ *pnSepLengths;
#ಆದರೆ_ಅಂತ್ಯ
} ಗುಂಪು_Concat_Ctx;

ಸ್ಥಿರ ಶೂನ್ಯ ಗುಂಪು_Concat_ಹೆಜ್ಜೆ(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal;
  ಗುಂಪು_Concat_Ctx *pGCC;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSep;
  ಪೂರ್ಣಾಂಕ nVal, nSep;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  pGCC = (ಗುಂಪು_Concat_Ctx*)sqlite3_aggregate_context(context, ಗಾತ್ರ(*pGCC));
  ಆದರೆ( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    ಪೂರ್ಣಾಂಕ ಮೊದಲ_ಪದ = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ];
    ಆದರೆ( argc==1 ){
      ಆದರೆ( !ಮೊದಲ_ಪದ ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಇಲ್ಲದಿದ್ದರೆ{
        pGCC->n_ಮೊದಲ_Sep_ಉದ್ದಳತೆ = 1;
      }
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !ಮೊದಲ_ಪದ ){
      zSep = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
      nSep = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[1]);
      ಆದರೆ( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಇಲ್ಲದಿದ್ದರೆ{
        nSep = 0;
      }
      ಆದರೆ( nSep != pGCC->n_ಮೊದಲ_Sep_ಉದ್ದಳತೆ || pGCC->pnSepLengths != 0 ){
        ಪೂರ್ಣಾಂಕ *pnsl = pGCC->pnSepLengths;
        ಆದರೆ( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (ಪೂರ್ಣಾಂಕ*)sqlite3_malloc64((pGCC->nAccum+1) * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
          ಆದರೆ( pnsl!=0 ){
            ಪೂರ್ಣಾಂಕ i = 0, nA = pGCC->nAccum-1;
            ಆಗ( i<nA ) pnsl[i++] = pGCC->n_ಮೊದಲ_Sep_ಉದ್ದಳತೆ;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          pnsl = (ಪೂರ್ಣಾಂಕ*)sqlite3_realloc64(pnsl, pGCC->nAccum * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
        }
        ಆದರೆ( pnsl!=0 ){
          ಆದರೆ( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Str_Accum_ಗಣ_Error(&pGCC->str, SQLITE_NOMEM);
        }
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಇಲ್ಲದಿದ್ದರೆ{
      pGCC->n_ಮೊದಲ_Sep_ಉದ್ದಳತೆ = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[1]);
    }
    pGCC->nAccum += 1;
#ಆದರೆ_ಅಂತ್ಯ
    zVal = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    nVal = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
    ಆದರೆ( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ ಗುಂಪು_Concat_Inverse(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಗುಂಪು_Concat_Ctx *pGCC;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  (ಶೂನ್ಯ)argc;  /* Suppress unused parameter warning */
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  pGCC = (ಗುಂಪು_Concat_Ctx*)sqlite3_aggregate_context(context, ಗಾತ್ರ(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  ಆದರೆ( ALWAYS(pGCC) ){
    ಪೂರ್ಣಾಂಕ nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (ಶೂನ್ಯ)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    nVS = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
    pGCC->nAccum -= 1;
    ಆದರೆ( pGCC->pnSepLengths!=0 ){
      ಪ್ರತಿಜ್ಞಾ(pGCC->nAccum >= 0);
      ಆದರೆ( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_ಮೊದಲ_Sep_ಉದ್ದಳತೆ;
    }
    ಆದರೆ( nVS>=(ಪೂರ್ಣಾಂಕ)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.z_ಪಠ್ಯ, &pGCC->str.z_ಪಠ್ಯ[nVS], pGCC->str.nChar);
    }
    ಆದರೆ( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಗುಂಪು_Concat_Inverse 0
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
ಸ್ಥಿರ ಶೂನ್ಯ ಗುಂಪು_Concat_Finalize(sqlite3_context *context){
  ಗುಂಪು_Concat_Ctx *pGCC
    = (ಗುಂಪು_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  ಆದರೆ( pGCC ){
    sqlite3_ಫಲಿತಾಂಶ_str(context, &pGCC->str, SQLITE_XFER);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#ಆದರೆ_ಅಂತ್ಯ
  }
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ ಗುಂಪು_Concat_ಮೌಲ್ಯ(sqlite3_context *context){
  ಗುಂಪು_Concat_Ctx *pGCC
    = (ಗುಂಪು_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  ಆದರೆ( pGCC && pGCC->nAccum>0 ){
    sqlite3_ಫಲಿತಾಂಶ_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಗುಂಪು_Concat_ಮೌಲ್ಯ 0
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_ಪ್ರತಿ_Connection_Builtin_Functions(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_overload_function(db, "MATCH", 2);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  ಆದರೆ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Like_Functions(sqlite3 *db, ಪೂರ್ಣಾಂಕ caseSensitive){
  FuncDef *pDef;
  ರಚನೆ compareInfo *pInfo;
  ಪೂರ್ಣಾಂಕ flags;
  ಪೂರ್ಣಾಂಕ nArg;
  ಆದರೆ( caseSensitive ){
    pInfo = (ರಚನೆ compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pInfo = (ರಚನೆ compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  ಗಾಗಿ(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, "like", nArg, SQLITE_UTF8, 0);
    ಪ್ರತಿಜ್ಞಾ( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
                       ** because the "like" SQL-function already exists */
    pDef->funcFlags |= flags;
    pDef->funcFlags &= ~SQLITE_FUNC_UNSAFE;
  }
}

/*
** pExpr points to an expression which implements a function.  If
** it is appropriate to apply the LIKE optimization to that function
** then set aWc[0] through aWc[2] to the wildcard characters and the
** escape character and then return TRUE.  If the function is not a
** LIKE-style function then return FALSE.
**
** The expression "a LIKE b ESCAPE c" is only considered a valid LIKE
** operator if c is a string literal that is exactly one byte in length.
** That one byte is stored in aWc[3].  aWc[3] is set to zero if there is
** no ESCAPE clause.
**
** *pIsNocase is set to true if uppercase and lowercase are equivalent for
** the function (default for LIKE).  If the function makes the distinction
** between uppercase and lowercase (as does GLOB) then *pIsNocase is set to
** false.
*/
ಪೂರ್ಣಾಂಕ sqlite3IsLikeFunction(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ *pIsNocase, ಅಕ್ಷರ *aWc){
  FuncDef *pDef;
  ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಆದರೆ( !p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ){
    ಹಿಂತಿರುಗಿ 0;
  }
  n_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
  pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, n_ಅಭಿವ್ಯಕ್ತಿ, SQLITE_UTF8, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ಆದರೆ( pDef==0 ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  ಸ್ಮರಣೆ_ನಕಲು(aWc, pDef->p_User_ದತ್ತಾಂಶ, 3);
  ಪ್ರತಿಜ್ಞಾ( (ಅಕ್ಷರ*)&likeInfoAlt == (ಅಕ್ಷರ*)&likeInfoAlt.ಹೊಂದಿಕೆ_All );
  ಪ್ರತಿಜ್ಞಾ( &((ಅಕ್ಷರ*)&likeInfoAlt)[1] == (ಅಕ್ಷರ*)&likeInfoAlt.ಹೊಂದಿಕೆ_One );
  ಪ್ರತಿಜ್ಞಾ( &((ಅಕ್ಷರ*)&likeInfoAlt)[2] == (ಅಕ್ಷರ*)&likeInfoAlt.ಹೊಂದಿಕೆ_ಗಣ );

  ಆದರೆ( n_ಅಭಿವ್ಯಕ್ತಿ<3 ){
    aWc[3] = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಭಿವ್ಯಕ್ತಿ *pEscape = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[2].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಕ್ಷರ *zEscape;
    ಆದರೆ( pEscape->op!=TK_STRING ) ಹಿಂತಿರುಗಿ 0;
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pEscape, EP_Int_ಮೌಲ್ಯ) );
    zEscape = pEscape->u.zToken;
    ಆದರೆ( zEscape[0]==0 || zEscape[1]!=0 ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( zEscape[0]==aWc[0] ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( zEscape[0]==aWc[1] ) ಹಿಂತಿರುಗಿ 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  ಹಿಂತಿರುಗಿ 1;
}

/* Mathematical Constants */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ M_PI
# define M_PI   3.141592653589793238462643383279502884
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#ಆದರೆ_ಅಂತ್ಯ


/* Extra math functions that require linking with -lm
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MATH_FUNCTIONS
/*
** Implementation SQL functions:
**
**   ceil(X)
**   ceiling(X)
**   floor(X)
**
** The sqlite3_user_data() pointer is a pointer to the libm implementation
** of the underlying C function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ceilingFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]) ){
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
       sqlite3_ಫಲಿತಾಂಶ_int64(context, sqlite3_ಮೌಲ್ಯ_int64(argv[0]));
       ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
       ದ್ವಿಗುಣ (*x)(ದ್ವಿಗುಣ) = (ದ್ವಿಗುಣ(*)(ದ್ವಿಗುಣ))sqlite3_user_ದತ್ತಾಂಶ(context);
       sqlite3_ಫಲಿತಾಂಶ_double(context, x(sqlite3_ಮೌಲ್ಯ_double(argv[0])));
       ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
       ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
ಸ್ಥಿರ ದ್ವಿಗುಣ xCeil(ದ್ವಿಗುಣ x){ ಹಿಂತಿರುಗಿ ceil(x); }
ಸ್ಥಿರ ದ್ವಿಗುಣ xFloor(ದ್ವಿಗುಣ x){ ಹಿಂತಿರುಗಿ floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#ಆದರೆ defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*ಲಘುಗಣಕ(X))
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*ಲಘುಗಣಕ(X))
#ಆದರೆ_ಅಂತ್ಯ


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
ಸ್ಥಿರ ಶೂನ್ಯ logFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ದ್ವಿಗುಣ x, b, ans;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]) ){
    ಸ್ಥಿತಿ SQLITE_INTEGER:
    ಸ್ಥಿತಿ SQLITE_FLOAT:
      x = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
      ಆದರೆ( x<=0.0 ) ಹಿಂತಿರುಗಿ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( argc==2 ){
    ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_numeric_type(argv[1]) ){
      ಸ್ಥಿತಿ SQLITE_INTEGER:
      ಸ್ಥಿತಿ SQLITE_FLOAT:
        b = ಲಘುಗಣಕ(x);
        ಆದರೆ( b<=0.0 ) ಹಿಂತಿರುಗಿ;
        x = sqlite3_ಮೌಲ್ಯ_double(argv[1]);
        ಆದರೆ( x<=0.0 ) ಹಿಂತಿರುಗಿ;
        ನಿಲ್ಲಿಸು;
     ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ;
    }
    ans = ಲಘುಗಣಕ(x)/b;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಯ್ಕೆ( SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(context)) ){
      ಸ್ಥಿತಿ 1:
        ans = log10(x);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 2:
        ans = log2(x);
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        ans = ಲಘುಗಣಕ(x);
        ನಿಲ್ಲಿಸು;
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
ಸ್ಥಿರ ದ್ವಿಗುಣ deg_ಗಮ್ಯ_Rad(ದ್ವಿಗುಣ x){ ಹಿಂತಿರುಗಿ x*(M_PI/180.0); }
ಸ್ಥಿರ ದ್ವಿಗುಣ rad_ಗಮ್ಯ_Deg(ದ್ವಿಗುಣ x){ ಹಿಂತಿರುಗಿ x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
ಸ್ಥಿರ ಶೂನ್ಯ math1Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ type0;
  ದ್ವಿಗುಣ v0, ans;
  ದ್ವಿಗುಣ (*x)(ದ್ವಿಗುಣ);
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  type0 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಆದರೆ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ಹಿಂತಿರುಗಿ;
  v0 = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  x = (ದ್ವಿಗುಣ(*)(ದ್ವಿಗುಣ))sqlite3_user_ದತ್ತಾಂಶ(context);
  ans = x(v0);
  sqlite3_ಫಲಿತಾಂಶ_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
ಸ್ಥಿರ ಶೂನ್ಯ math2Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ type0, type1;
  ದ್ವಿಗುಣ v0, v1, ans;
  ದ್ವಿಗುಣ (*x)(ದ್ವಿಗುಣ,ದ್ವಿಗುಣ);
  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  type0 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಆದರೆ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ಹಿಂತಿರುಗಿ;
  type1 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[1]);
  ಆದರೆ( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) ಹಿಂತಿರುಗಿ;
  v0 = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  v1 = sqlite3_ಮೌಲ್ಯ_double(argv[1]);
  x = (ದ್ವಿಗುಣ(*)(ದ್ವಿಗುಣ,ದ್ವಿಗುಣ))sqlite3_user_ದತ್ತಾಂಶ(context);
  ans = x(v0, v1);
  sqlite3_ಫಲಿತಾಂಶ_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ piFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==0 );
  (ಶೂನ್ಯ)argv;
  sqlite3_ಫಲಿತಾಂಶ_double(context, M_PI);
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ signFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ type0;
  ದ್ವಿಗುಣ x;
  ಬಳಸದ_PARAMETER(argc);
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  type0 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಆದರೆ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ಹಿಂತಿರುಗಿ;
  x = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  sqlite3_ಫಲಿತಾಂಶ_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#ಆದರೆ defined(SQLITE_ENABLE_PERCENTILE)
/***********************************************************************
** This section implements the percentile(Y,P) SQL function and similar.
** Requirements:
**
**   (1)  The percentile(Y,P) function is an aggregate function taking
**        exactly two arguments.
**
**   (2)  If the P argument to percentile(Y,P) is not the same for every
**        row in the aggregate then an error is thrown.  The word "same"
**        in the previous sentence means that the value differ by less
**        than 0.001.
**
**   (3)  If the P argument to percentile(Y,P) evaluates to anything other
**        than a number in the range of 0.0 to 100.0 inclusive then an
**        error is thrown.
**
**   (4)  If any Y argument to percentile(Y,P) evaluates to a value that
**        is not NULL and is not numeric then an error is thrown.
**
**   (5)  If any Y argument to percentile(Y,P) evaluates to plus or minus
**        infinity then an error is thrown.  (SQLite always interprets NaN
**        values as NULL.)
**
**   (6)  Both Y and P in percentile(Y,P) can be arbitrary expressions,
**        including CASE WHEN expressions.
**
**   (7)  The percentile(Y,P) aggregate is able to handle inputs of at least
**        one million (1,000,000) rows.
**
**   (8)  If there are no non-NULL values for Y, then percentile(Y,P)
**        returns NULL.
**
**   (9)  If there is exactly one non-NULL value for Y, the percentile(Y,P)
**        returns the one Y value.
**
**  (10)  If there N non-NULL values of Y where N is two or more and
**        the Y values are ordered from least to greatest and a graph is
**        drawn from 0 to N-1 such that the height of the graph at J is
**        the J-th Y value and such that straight lines are drawn between
**        adjacent Y values, then the percentile(Y,P) function returns
**        the height of the graph at P*(N-1)/100.
**
**  (11)  The percentile(Y,P) function always returns either a floating
**        point number or NULL.
**
**  (12)  The percentile(Y,P) is implemented as a single C99 source-code
**        file that compiles into a shared-library or DLL that can be loaded
**        into SQLite using the sqlite3_load_extension() interface.
**
**  (13)  A separate median(Y) function is the equivalent percentile(Y,50).
**
**  (14)  A separate percentile_cont(Y,P) function is equivalent to
**        percentile(Y,P/100.0).  In other words, the fraction value in
**        the second argument is in the range of 0 to 1 instead of 0 to 100.
**
**  (15)  A separate percentile_disc(Y,P) function is like
**        percentile_cont(Y,P) except that instead of returning the weighted
**        average of the nearest two input values, it returns the next lower
**        value.  So the percentile_disc(Y,P) will always return a value
**        that was one of the inputs.
**
**  (16)  All of median(), percentile(Y,P), percentile_cont(Y,P) and
**        percentile_disc(Y,P) can be used as window functions.
**
** Differences from standard SQL:
**
**  *  The percentile_cont(X,P) function is equivalent to the following in
**     standard SQL:
**
**         (percentile_cont(P) WITHIN GROUP (ORDER BY X))
**
**     The SQLite syntax is much more compact.  The standard SQL syntax
**     is also supported if SQLite is compiled with the
**     -DSQLITE_ENABLE_ORDERED_SET_AGGREGATES option.
**
**  *  No median(X) function exists in the SQL standard.  App developers
**     are expected to write "percentile_cont(0.5)WITHIN GROUP(ORDER BY X)".
**
**  *  No percentile(Y,P) function exists in the SQL standard.  Instead of
**     percential(Y,P), developers must write this:
**     "percentile_cont(P/100.0) WITHIN GROUP (ORDER BY Y)".  Note that
**     the fraction parameter to percentile() goes from 0 to 100 whereas
**     the fraction parameter in SQL standard percentile_cont() goes from
**     0 to 1.
**
** Implementation notes as of 2024-08-31:
**
**  *  The regular aggregate-function versions of these routines work
**     by accumulating all values in an array of doubles, then sorting
**     that array using quicksort before computing the answer. Thus
**     the runtime is O(NlogN) where N is the number of rows of input.
**
**  *  For the window-function versions of these routines, the array of
**     inputs is sorted as soon as the first value is computed.  Thereafter,
**     the array is kept in sorted order using an insert-sort.  This
**     results in O(N*K) performance where K is the size of the window.
**     One can imagine alternative implementations that give O(N*logN*logK)
**     performance, but they require more complex logic and data structures.
**     The developers have elected to keep the asymptotically slower
**     algorithm for now, for simplicity, under the theory that window
**     functions are seldom used and when they are, the window size K is
**     often small.  The developers might revisit that decision later,
**     should the need arise.
*/

/* The following object is the group context for a single percentile()
** aggregate.  Remember all input Y values until the very end.
** Those values are accumulated in the Percentile.a[] array.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Percentile Percentile;
ರಚನೆ Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 n_ಬಳಸಿದ;           /* Number of slots actually used in a[] */
  ಅಕ್ಷರ bSorted;        /* True if a[] is already in sorted order */
  ಅಕ್ಷರ bKeepSorted;    /* True if advantageous to keep a[] sorted */
  ಅಕ್ಷರ bPctValid;      /* True if rPct is valid */
  ದ್ವಿಗುಣ rPct;         /* Fraction.  0.0 to 1.0 */
  ದ್ವಿಗುಣ *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ percentIsInfinity(ದ್ವಿಗುಣ r){
  sqlite3_uint64 u;
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(u)==ಗಾತ್ರ(r) );
  ಸ್ಮರಣೆ_ನಕಲು(&u, &r, ಗಾತ್ರ(u));
  ಹಿಂತಿರುಗಿ ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ percent_Same_ಮೌಲ್ಯ(ದ್ವಿಗುಣ a, ದ್ವಿಗುಣ b){
  a -= b;
  ಹಿಂತಿರುಗಿ a>=-0.001 && a<=0.001;
}

/*
** Search p (which must have p->bSorted) looking for an entry with
** value y.  Return the index of that entry.
**
** If bExact is true, return -1 if the entry is not found.
**
** If bExact is false, return the index at which a new entry with
** value y should be insert in order to keep the values in sorted
** order.  The smallest return value in this case will be 0, and
** the largest return value will be p->nUsed.
*/
ಸ್ಥಿರ i64 percent_ದ್ವಿಮಾನ_ಹುಡುಕು(Percentile *p, ದ್ವಿಗುಣ y, ಪೂರ್ಣಾಂಕ bExact){
  i64 i_ಮೊದಲ = 0;                   /* First element of search range */
  i64 i_ಕೊನೆಯ = (i64)p->n_ಬಳಸಿದ - 1;    /* Last element of search range */
  ಆಗ( i_ಕೊನೆಯ>=i_ಮೊದಲ ){
    i64 iMid = (i_ಮೊದಲ+i_ಕೊನೆಯ)/2;
    ದ್ವಿಗುಣ x = p->a[iMid];
    ಆದರೆ( x<y ){
      i_ಮೊದಲ = iMid + 1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x>y ){
      i_ಕೊನೆಯ = iMid - 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ iMid;
    }
  }
  ಆದರೆ( bExact ) ಹಿಂತಿರುಗಿ -1;
  ಹಿಂತಿರುಗಿ i_ಮೊದಲ;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentError(sqlite3_context *pCtx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  ಅಕ್ಷರ *zMsg1;
  ಅಕ್ಷರ *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_ಫಲಿತಾಂಶ_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percent_ಹೆಜ್ಜೆ(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  Percentile *p;
  ದ್ವಿಗುಣ rPct;
  ಪೂರ್ಣಾಂಕ eType;
  ದ್ವಿಗುಣ y;
  ಪ್ರತಿಜ್ಞಾ( argc==2 || argc==1 );

  ಆದರೆ( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    ದ್ವಿಗುಣ mxFrac = (SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[1]);
    rPct = sqlite3_ಮೌಲ್ಯ_double(argv[1])/mxFrac;
    ಆದರೆ( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (ದ್ವಿಗುಣ)mxFrac);
      ಹಿಂತಿರುಗಿ;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  ಆದರೆ( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !percent_Same_ಮೌಲ್ಯ(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    ಹಿಂತಿರುಗಿ;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  ಆದರೆ( eType==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ಆದರೆ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    ಹಿಂತಿರುಗಿ;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  ಆದರೆ( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    ಹಿಂತಿರುಗಿ;
  }

  /* Allocate and store the Y */
  ಆದರೆ( p->n_ಬಳಸಿದ>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    ದ್ವಿಗುಣ *a = sqlite3_realloc64(p->a, ಗಾತ್ರ(ದ್ವಿಗುಣ)*n);
    ಆದರೆ( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, ಗಾತ್ರ(*p));
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
      ಹಿಂತಿರುಗಿ;
    }
    p->nAlloc = n;
    p->a = a;
  }
  ಆದರೆ( p->n_ಬಳಸಿದ==0 ){
    p->a[p->n_ಬಳಸಿದ++] = y;
    p->bSorted = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !p->bSorted || y>=p->a[p->n_ಬಳಸಿದ-1] ){
    p->a[p->n_ಬಳಸಿದ++] = y;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->bKeepSorted ){
    i64 i;
    i = percent_ದ್ವಿಮಾನ_ಹುಡುಕು(p, y, 0);
    ಆದರೆ( i<(ಪೂರ್ಣಾಂಕ)p->n_ಬಳಸಿದ ){
      memmove(&p->a[i+1], &p->a[i], (p->n_ಬಳಸಿದ-i)*ಗಾತ್ರ(p->a[0]));
    }
    p->a[i] = y;
    p->n_ಬಳಸಿದ++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->a[p->n_ಬಳಸಿದ++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಅದಲುಬದಲು_DOUBLE(X,Y)  {ದ್ವಿಗುಣ ttt=(X);(X)=(Y);(Y)=ttt;}

/*
** Sort an array of doubles.
**
** Algorithm: quicksort
**
** This is implemented separately rather than using the qsort() routine
** from the standard library because:
**
**    (1)  To avoid a dependency on qsort()
**    (2)  To avoid the function call to the comparison routine for each
**         comparison.
**
** If parameter iReq is non-negative, then the caller will only access
** elements a[iReq] and a[iReq+1] (if it exists) of the sorted array and
** so it is not necessary to position any other elements. Or if iReq is
** negative, then the final array must be fully sorted.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percent_ವಿಂಗಡಿಸು(
  ದ್ವಿಗುಣ *a,                      /* Array to sort */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n,                 /* Number of elements in array a[] */
  ಪೂರ್ಣಾಂಕ iReq                        /* Element caller cares about (or -ve) */
){
  ಪೂರ್ಣಾಂಕ iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  ಪೂರ್ಣಾಂಕ iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  ಪೂರ್ಣಾಂಕ i;         /* Loop counter */
  ದ್ವಿಗುಣ r_ಅಚ್ಚು; /* The pivot value */

  ಪ್ರತಿಜ್ಞಾ( n>=2 );
  ಮಾಡು{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    ಆದರೆ( a[0]>a[n-1] ){
      ಅದಲುಬದಲು_DOUBLE(a[0],a[n-1])
    }
    ಆದರೆ( n==2 ) ಹಿಂತಿರುಗಿ;
    iGt = n-1;
    i = n/2;
    ಆದರೆ( a[0]>a[i] ){
      ಅದಲುಬದಲು_DOUBLE(a[0],a[i])
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( a[i]>a[iGt] ){
      ಅದಲುಬದಲು_DOUBLE(a[i],a[iGt])
    }
    ಆದರೆ( n==3 ) ಹಿಂತಿರುಗಿ;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    r_ಅಚ್ಚು = a[i];
    iLt = i = 1;
    ಮಾಡು{
      ಆದರೆ( a[i]<r_ಅಚ್ಚು ){
        ಆದರೆ( i>iLt ) ಅದಲುಬದಲು_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( a[i]>r_ಅಚ್ಚು ){
        ಮಾಡು{
          iGt--;
        }ಆಗ( iGt>i && a[iGt]>r_ಅಚ್ಚು );
        ಅದಲುಬದಲು_DOUBLE(a[i],a[iGt])
      }ಇಲ್ಲದಿದ್ದರೆ{
        i++;
      }
    }ಆಗ( i<iGt );

    ಪ್ರತಿಜ್ಞಾ( iLt>0 && iLt<iGt && (ಚಿಹ್ನೆರಹಿತ)iGt<n );
    testcase( iGt>iLt+1 );
    ಪ್ರತಿಜ್ಞಾ( a[iLt]==r_ಅಚ್ಚು );
    ಪ್ರತಿಜ್ಞಾ( a[iLt-1]<=r_ಅಚ್ಚು );
    ಪ್ರತಿಜ್ಞಾ( a[iGt]>=r_ಅಚ್ಚು );
    ಪ್ರತಿಜ್ಞಾ( a[iLt+1]>=r_ಅಚ್ಚು );

    ಆದರೆ( iReq>=0 ){
      /* In this case, the only elements that the caller requires sorted into 
      ** the correct positions are elements a[iReq] and a[iReq+1]. At this
      ** point we know that element a[iLt] is in the correct position and
      ** all elements smaller than a[iLt] are in the left-hand partition.
      ** So if (iReq<iLt), then it is only necessary to sort the left
      ** partition.
      **
      ** If (iReq>=iLt), then elements iReq and iReq+1 are either in the
      ** right partition or the equal partition (elements for which 
      ** iLt<=iElem<iGt). Therefore it is always sufficient to sort only
      ** the right partition in this case.  */
      ಆದರೆ( iReq<iLt ){
        n = iLt;
      }ಇಲ್ಲದಿದ್ದರೆ{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      ಆದರೆ( iLt>(ಪೂರ್ಣಾಂಕ)(n/2) ){
        ಆದರೆ( n-iGt>=2 ) percent_ವಿಂಗಡಿಸು(a+iGt, n-iGt, -1);
        n = iLt;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( iLt>=2 ) percent_ವಿಂಗಡಿಸು(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }ಆಗ( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentInverse(sqlite3_context *pCtx,ಪೂರ್ಣಾಂಕ argc,sqlite3_ಮೌಲ್ಯ **argv){
  Percentile *p;
  ಪೂರ್ಣಾಂಕ eType;
  ದ್ವಿಗುಣ y;
  i64 i;
  ಪ್ರತಿಜ್ಞಾ( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, ಗಾತ್ರ(*p));
  ಪ್ರತಿಜ್ಞಾ( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  ಆದರೆ( eType==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ಆದರೆ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    ಹಿಂತಿರುಗಿ;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  ಆದರೆ( percentIsInfinity(y) ){
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p->bSorted==0 ){
    ಪ್ರತಿಜ್ಞಾ( p->n_ಬಳಸಿದ>1 );
    percent_ವಿಂಗಡಿಸು(p->a, p->n_ಬಳಸಿದ, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_ದ್ವಿಮಾನ_ಹುಡುಕು(p, y, 1);
  ಆದರೆ( i>=0 ){
    p->n_ಬಳಸಿದ--;
    ಆದರೆ( i<(ಪೂರ್ಣಾಂಕ)p->n_ಬಳಸಿದ ){
      memmove(&p->a[i], &p->a[i+1], (p->n_ಬಳಸಿದ - i)*ಗಾತ್ರ(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentCompute(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ bIsFinal){
  Percentile *p;
  ಪೂರ್ಣಾಂಕ settings = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(pCtx))&1; /* Discrete? */
  ಚಿಹ್ನೆರಹಿತ i1, i2;
  ದ್ವಿಗುಣ v1, v2;
  ದ್ವಿಗುಣ ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p->a==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p->n_ಬಳಸಿದ ){
    ix = p->rPct*(p->n_ಬಳಸಿದ-1);
    i1 = (ಚಿಹ್ನೆರಹಿತ)ix;
    ಆದರೆ( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      ಪ್ರತಿಜ್ಞಾ( p->n_ಬಳಸಿದ>1 );
      percent_ವಿಂಗಡಿಸು(p->a, p->n_ಬಳಸಿದ, (bIsFinal ? (ಪೂರ್ಣಾಂಕ)i1 : -1));
      p->bSorted = 1;
    }
    ಆದರೆ( settings & 1 ){
      vx = p->a[i1];
    }ಇಲ್ಲದಿದ್ದರೆ{
      i2 = ix==(ದ್ವಿಗುಣ)i1 || i1==p->n_ಬಳಸಿದ-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_ಫಲಿತಾಂಶ_double(pCtx, vx);
  }
  ಆದರೆ( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, ಗಾತ್ರ(*p));
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->bKeepSorted = 1;
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
ಸ್ಥಿರ ಶೂನ್ಯ percent_ಮೌಲ್ಯ(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PERCENTILE */

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
ಸ್ಥಿರ ಶೂನ್ಯ filestatFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  pBtree = sqlite3_Db_Name_ಗಮ್ಯ_Btree(db, zDbName);
  ಆದರೆ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    ಪೂರ್ಣಾಂಕ rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ಪ್ರತಿಜ್ಞಾ( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    ಆದರೆ( sqlite3_str_errcode(pStr) ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      ಆದರೆ( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      ಆದರೆ( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        ಆದರೆ( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_ಫಲಿತಾಂಶ_str(context, pStr, SQLITE_ಮುಕ್ತಾಯ);
    }
    sqlite3BtreeLeave(pBtree);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, "{}", 2, SQLITE_STATIC);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
ಸ್ಥಿರ ಶೂನ್ಯ walstatFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  pBtree = sqlite3_Db_Name_ಗಮ್ಯ_Btree(db, zDbName);
  ಆದರೆ( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ಪ್ರತಿಜ್ಞಾ( pPager!=0 );
    pStr = sqlite3_str_new(db);
    ಆದರೆ( sqlite3_str_errcode(pStr) ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_ಫಲಿತಾಂಶ_str(context, pStr, SQLITE_ಮುಕ್ತಾಯ);
    }
    sqlite3BtreeLeave(pBtree);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, "{}", 2, SQLITE_STATIC);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Implementation of fpdecode(x,y,z) function.
**
** x is a real number that is to be decoded.  y is the precision.
** z is the maximum real precision.  Return a string that shows the
** results of the sqlite3FpDecode() function.
**
** Used for testing and debugging only, specifically testing and debugging
** of the sqlite3FpDecode() function.  This SQL function does not appear
** in production builds.  This function is not an API and is subject to
** modification or removal in future versions of SQLite.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fpdecodeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  FpDecode s;
  ದ್ವಿಗುಣ x;
  ಪೂರ್ಣಾಂಕ y, z;
  ಅಕ್ಷರ zBuf[100];
  ಬಳಸದ_PARAMETER(argc);
  ಪ್ರತಿಜ್ಞಾ( argc==3 );
  x = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  y = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  z = sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  ಆದರೆ( z<=0 ) z = 1;
  ಆದರೆ( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  ಆದರೆ( s.isSpecial==2 ){
    sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "NaN");
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, zBuf, -1, SQLITE_TRANSIENT);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Implementation of parseuri(uri,flags) function.
**
** Required Arguments:
**    "uri"        The URI to parse.
**    "flags"      Bitmask of flags, as if to sqlite3_open_v2().
**
** Additional arguments beyond the first two make calls to
** sqlite3_uri_key() for integers and sqlite3_uri_parameter for
** anything else.
**
** The result is a string showing the results of calling sqlite3ParseUri().
**
** Used for testing and debugging only, specifically testing and debugging
** of the sqlite3ParseUri() function.  This SQL function does not appear
** in production builds.  This function is not an API and is subject to
** modification or removal in future versions of SQLite.
*/
ಸ್ಥಿರ ಶೂನ್ಯ parseuriFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_str *p_ಫಲಿತಾಂಶ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUri;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flgs;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_vfs *pVfs = 0;
  ಅಕ್ಷರ *zFile = 0;
  ಅಕ್ಷರ *zErr = 0;

  ಆದರೆ( argc<2 ) ಹಿಂತಿರುಗಿ;
  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಪ್ರತಿಜ್ಞಾ( pVfs );
  zVfs = pVfs->zName;
  zUri = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಆದರೆ( zUri==0 ) ಹಿಂತಿರುಗಿ;
  flgs = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_ಫಲಿತಾಂಶ = sqlite3_str_new(0);
  ಆದರೆ( !sqlite3_str_errcode(p_ಫಲಿತಾಂಶ) ){
    ಪೂರ್ಣಾಂಕ i;
    sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, "rc=%d", rc);
    sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", err=%Q", zErr);
    sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", file=%Q", zFile);
    ಆದರೆ( zFile ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = zFile;
      z += sqlite3Strlen30(z)+1;
      ಆಗ( z[0] ){
        sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      ಗಾಗಿ(i=2; i<argc; i++){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg;
        ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_INTEGER ){
          ಪೂರ್ಣಾಂಕ k = sqlite3_ಮೌಲ್ಯ_int(argv[i]);
          sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", '%d:%q'",k,sqlite3_uri_ಕೀಲಿ(zFile, k));
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (zArg = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]))!=0 ){
          sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_str_appendf(p_ಫಲಿತಾಂಶ, ", NULL");
        }
      }
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_str(ctx, p_ಫಲಿತಾಂಶ, SQLITE_ಮುಕ್ತಾಯ);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Builtin_Functions(ಶೂನ್ಯ){
  /*
  ** The following array holds FuncDef structures for all of the functions
  ** defined in this file.
  **
  ** The array cannot be constant since changes are made to the
  ** FuncDef.pHash elements at start-time.  The elements of this array
  ** are read-only after initialization is complete.
  **
  ** For peak efficiency, put the most frequently used function last.
  */
  ಸ್ಥಿರ FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#ಆದರೆ !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(ಸೂಚಿಸುತ್ತದೆ_nonnull_ಅಡ್ಡಸಾಲು, 2, INLINEFUNC_ಸೂಚಿಸುತ್ತದೆ_nonnull_ಅಡ್ಡಸಾಲು, 0),
    TEST_FUNC(ಅಭಿವ್ಯಕ್ತಿ_compare,        2, INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_compare,        0),
    TEST_FUNC(ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ,   2, INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
    SFUNCTION(ತುಂಬು_extension,    1, 0, 0, ತುಂಬು_Ext          ),
    SFUNCTION(ತುಂಬು_extension,    2, 0, 0, ತುಂಬು_Ext          ),
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_ಬಳಸಿದ,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಥಾನಾಂತರ_SQL_FUNC
    INLINE_FUNC(sqlite_ಸ್ಥಾನಾಂತರ,   1, INLINEFUNC_sqlite_ಸ್ಥಾನಾಂತರ, 0 ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#ಆದರೆ_ಅಂತ್ಯ
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmax_ಹೆಜ್ಜೆ, minMaxFinalize, min_Max_ಮೌಲ್ಯ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmax_ಹೆಜ್ಜೆ, minMaxFinalize, min_Max_ಮೌಲ್ಯ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(ಉದ್ದಳತೆ,            1, 0, 0, ಉದ್ದಳತೆ_Func,  SQLITE_FUNC_ಉದ್ದಳತೆ),
    FUNCTION2(octet_ಉದ್ದಳತೆ,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(ಬರೆ,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(ಅಕ್ಷರ,              -1, 0, 0, charFunc         ),
    FUNCTION(ನಿರಪೇಕ್ಷ,                1, 0, 0, absFunc          ),
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
    FUNCTION(ಸುತ್ತು,              1, 0, 0, ಸುತ್ತು_Func        ),
    FUNCTION(ಸುತ್ತು,              2, 0, 0, ಸುತ್ತು_Func        ),
#ಆದರೆ_ಅಂತ್ಯ
    FUNCTION(ಊರ್ಧ್ವ,              1, 0, 0, ಊರ್ಧ್ವ_Func        ),
    FUNCTION(ಅಧೋ,              1, 0, 0, ಅಧೋ_Func        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_ಆವೃತ್ತಿ,    0, 0, 0, ಆವೃತ್ತಿ_Func      ),
    DFUNCTION(sqlite_ಮೂಲ_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(ಕೊನೆಯ_ಸೇರಿಸು_rowid, 0, 0, 0, ಕೊನೆಯ_ಸೇರಿಸು_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(ಒಟ್ಟು_changes,     0, 0, 0, ಒಟ್ಟು_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(ಮೊತ್ತ,   1,0,0, ಮೊತ್ತ_ಹೆಜ್ಜೆ, ಮೊತ್ತ_Finalize, ಮೊತ್ತ_Finalize, ಮೊತ್ತ_Inverse, 0),
    WAGGREGATE(ಒಟ್ಟು, 1,0,0, ಮೊತ್ತ_ಹೆಜ್ಜೆ,ಒಟ್ಟು_Finalize,ಒಟ್ಟು_Finalize,ಮೊತ್ತ_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, ಮೊತ್ತ_ಹೆಜ್ಜೆ, avgFinalize, avgFinalize, ಮೊತ್ತ_Inverse, 0),
    WAGGREGATE(ಎಣಿಕೆ_2, 0,0,0, ಎಣಿಕೆ_ಹೆಜ್ಜೆ,
        ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Inverse,
        SQLITE_FUNC_ಎಣಿಕೆ|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(ಎಣಿಕೆ_2, 1,0,0, ಎಣಿಕೆ_ಹೆಜ್ಜೆ,
        ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(ಗುಂಪು_concat, 1, 0, 0, ಗುಂಪು_Concat_ಹೆಜ್ಜೆ,
        ಗುಂಪು_Concat_Finalize, ಗುಂಪು_Concat_ಮೌಲ್ಯ, ಗುಂಪು_Concat_Inverse, 0),
    WAGGREGATE(ಗುಂಪು_concat, 2, 0, 0, ಗುಂಪು_Concat_ಹೆಜ್ಜೆ,
        ಗುಂಪು_Concat_Finalize, ಗುಂಪು_Concat_ಮೌಲ್ಯ, ಗುಂಪು_Concat_Inverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, ಗುಂಪು_Concat_ಹೆಜ್ಜೆ,
        ಗುಂಪು_Concat_Finalize, ಗುಂಪು_Concat_ಮೌಲ್ಯ, ಗುಂಪು_Concat_Inverse, 0),

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percent_ಹೆಜ್ಜೆ,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percent_ಹೆಜ್ಜೆ,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percent_ಹೆಜ್ಜೆ,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percent_ಹೆಜ್ಜೆ,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ಇಲ್ಲದಿದ್ದರೆ
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#ಆದರೆ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#ಆದರೆ_ಅಂತ್ಯ
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(ಲಘುಗಣಕ,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(ಲಘುಗಣಕ,                2, 0, 0,      logFunc     ),
    MFUNCTION(ಘಾತಾಂಕ,               1, ಘಾತಾಂಕ,       math1Func   ),
    MFUNCTION(ಘಾತ,               2, ಘಾತ,       math2Func   ),
    MFUNCTION(power,             2, ಘಾತ,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(ಕೋಜ್ಯಾ,               1, ಕೋಜ್ಯಾ,       math1Func   ),
    MFUNCTION(ಜ್ಯಾ,               1, ಜ್ಯಾ,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#ಆದರೆ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#ಆದರೆ_ಅಂತ್ಯ
    MFUNCTION(ವರ್ಗಮೂಲ,              1, ವರ್ಗಮೂಲ,      math1Func   ),
    MFUNCTION(radians,           1, deg_ಗಮ್ಯ_Rad,  math1Func   ),
    MFUNCTION(degrees,           1, rad_ಗಮ್ಯ_Deg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(ಆದರೆ,             -4, INLINEFUNC_iif,      0 ),
  };
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_ಕಿಟಕಿ_Functions();
  sqlite3_ರಿಜಿಸ್ಟರ್_Date_ಸಮಯ_Functions();
  sqlite3_ರಿಜಿಸ್ಟರ್_Json_Functions();
  sqlite3_ಸೇರಿಸು_Builtin_Funcs(aBuiltinFunc, Array_ಗಾತ್ರ(aBuiltinFunc));

#ಆದರೆ 0  /* Enable to print out how the built-in functions are hashed */
  {
    ಪೂರ್ಣಾಂಕ i;
    FuncDef *p;
    ಗಾಗಿ(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      ಬರೆ("FUNC-HASH %02d:", i);
      ಗಾಗಿ(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        ಪೂರ್ಣಾಂಕ n = sqlite3Strlen30(p->zName);
        ಪೂರ್ಣಾಂಕ h = p->zName[0] + n;
        ಪ್ರತಿಜ್ಞಾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        ಬರೆ(" %s(%d)", p->zName, h);
      }
      ಬರೆ("\n");
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
}
