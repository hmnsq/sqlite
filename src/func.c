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
#ifndef SQLITE_OMIT_FLOATING_POINT
#ಅನ್ತರ್ಭಾವ <math.h>
#endif
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
  ಪ್ರತ್ಯಾಗಮ pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  ಪ್ರತಿಜ್ಞಾ( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
ಸ್ಥಿರ ಶೂನ್ಯ minmaxFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ mask;    /* 0 for min() or 0xffffffff for max() */
  ಪೂರ್ಣಾಙ್ಕ iBest;
  CollSeq *pColl;

  ಪ್ರತಿಜ್ಞಾ( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  ಪ್ರತಿಜ್ಞಾ( pColl );
  ಪ್ರತಿಜ್ಞಾ( mask==-1 || mask==0 );
  iBest = 0;
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(i=1; i<argc; i++){
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_ಮೌಲ್ಯ(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ typeofFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *azType[] = { "integer", "real", "text", "blob", "null" };
  ಪೂರ್ಣಾಙ್ಕ i = sqlite3_ಮೌಲ್ಯ_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<ArraySize(azType) );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_INTEGER==1 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_FLOAT==2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_TEXT==3 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_BLOB==4 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_result_text(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
ಸ್ಥಿರ ಶೂನ್ಯ subtypeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_result_int(context, sqlite3_ಮೌಲ್ಯ_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
ಸ್ಥಿರ ಶೂನ್ಯ lengthFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಪಕ್ಷ SQLITE_BLOB:
    ಪಕ್ಷ SQLITE_INTEGER:
    ಪಕ್ಷ SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_ಮೌಲ್ಯ_bytes(argv[0]));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_TEXT: {
      ಧ್ರುವ unsigned ವರ್ಣ *z = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
      ಧ್ರುವ unsigned ವರ್ಣ *z0;
      ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ;
      z0 = z;
      ಯಾವತ್( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        ಯದಿ( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }ಅನ್ಯಥಾ ಯದಿ( z[0]==0 ){
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ{
          z++;
          ಯಾವತ್( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_result_int(context, (ಪೂರ್ಣಾಙ್ಕ)(z-z0));
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      sqlite3_result_null(context);
      ಭಙ್ಗ;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
ಸ್ಥಿರ ಶೂನ್ಯ bytelengthFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಪಕ್ಷ SQLITE_BLOB: {
      sqlite3_result_int(context, sqlite3_ಮೌಲ್ಯ_bytes(argv[0]));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_INTEGER:
    ಪಕ್ಷ SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_result_int64(context, sqlite3_ಮೌಲ್ಯ_bytes(argv[0])*m);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_TEXT: {
      ಯದಿ( sqlite3_ಮೌಲ್ಯ_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_result_int(context, sqlite3_ಮೌಲ್ಯ_bytes(argv[0]));
      }ಅನ್ಯಥಾ{
        sqlite3_result_int(context, sqlite3_ಮೌಲ್ಯ_bytes16(argv[0]));
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      sqlite3_result_null(context);
      ಭಙ್ಗ;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
ಸ್ಥಿರ ಶೂನ್ಯ absFunc(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಪಕ್ಷ SQLITE_INTEGER: {
      i64 iVal = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
      ಯದಿ( iVal<0 ){
        ಯದಿ( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          ಪ್ರತ್ಯಾಗಮ;
        }
        iVal = -iVal;
      }
      sqlite3_result_int64(context, iVal);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      ದ್ವಿಗುಣ rVal = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
      ಯದಿ( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      ಭಙ್ಗ;
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ unsigned ವರ್ಣ *zHaystack;
  ಧ್ರುವ unsigned ವರ್ಣ *zNeedle;
  ಪೂರ್ಣಾಙ್ಕ nHaystack;
  ಪೂರ್ಣಾಙ್ಕ nNeedle;
  ಪೂರ್ಣಾಙ್ಕ typeHaystack, typeNeedle;
  ಪೂರ್ಣಾಙ್ಕ N = 1;
  ಪೂರ್ಣಾಙ್ಕ isText;
  unsigned ವರ್ಣ firstChar;
  sqlite3_ಮೌಲ್ಯ *pC1 = 0;
  sqlite3_ಮೌಲ್ಯ *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  typeNeedle = sqlite3_ಮೌಲ್ಯ_type(argv[1]);
  ಯದಿ( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  nHaystack = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  nNeedle = sqlite3_ಮೌಲ್ಯ_bytes(argv[1]);
  ಯದಿ( nNeedle>0 ){
    ಯದಿ( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
      zNeedle = sqlite3_ಮೌಲ್ಯ_blob(argv[1]);
      isText = 0;
    }ಅನ್ಯಥಾ ಯದಿ( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
      zNeedle = sqlite3_ಮೌಲ್ಯ_text(argv[1]);
      isText = 1;
    }ಅನ್ಯಥಾ{
      pC1 = sqlite3_ಮೌಲ್ಯ_dup(argv[0]);
      zHaystack = sqlite3_ಮೌಲ್ಯ_text(pC1);
      ಯದಿ( zHaystack==0 ) ಗಚ್ಛ endInstrOOM;
      nHaystack = sqlite3_ಮೌಲ್ಯ_bytes(pC1);
      pC2 = sqlite3_ಮೌಲ್ಯ_dup(argv[1]);
      zNeedle = sqlite3_ಮೌಲ್ಯ_text(pC2);
      ಯದಿ( zNeedle==0 ) ಗಚ್ಛ endInstrOOM;
      nNeedle = sqlite3_ಮೌಲ್ಯ_bytes(pC2);
      isText = 1;
    }
    ಯದಿ( zNeedle==0 || (nHaystack && zHaystack==0) ) ಗಚ್ಛ endInstrOOM;
    firstChar = zNeedle[0];
    ಯಾವತ್( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      ಕುರು{
        nHaystack--;
        zHaystack++;
      }ಯಾವತ್( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    ಯದಿ( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_ಮೌಲ್ಯ_free(pC1);
  sqlite3_ಮೌಲ್ಯ_free(pC2);
  ಪ್ರತ್ಯಾಗಮ;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  ಗಚ್ಛ endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ printfFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  PrintfArguments x;
  StrAccum str;
  ಧ್ರುವ ವರ್ಣ *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  ಯದಿ( argc>=1 && (zFormat = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.nUsed = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_result_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
ಸ್ಥಿರ SQLITE_INLINE ಪೂರ್ಣಾಙ್ಕ containsNulOrNonAscii(u64 word){
  ಧ್ರುವ u64 byteOnes = UINT64_C(0x0101010101010101);
  ಧ್ರುವ u64 byte_ಮೇಲಿನ_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  ಪ್ರತ್ಯಾಗಮ ((word | (word - byteOnes)) & byte_ಮೇಲಿನ_Bits)!=0;
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ unsigned ವರ್ಣ *z;
  ಧ್ರುವ unsigned ವರ್ಣ *z2;
  ಪೂರ್ಣಾಙ್ಕ len;
  ಪೂರ್ಣಾಙ್ಕ p0type;
  i64 p1, p2;

  ಪ್ರತಿಜ್ಞಾ( argc==3 || argc==2 );
  p0type = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  p1 = sqlite3_ಮೌಲ್ಯ_int64(argv[1]);
  ಯದಿ( p0type==SQLITE_BLOB ){
    len = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
    z = sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
    ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ;
    ಪ್ರತಿಜ್ಞಾ( len==sqlite3_ಮೌಲ್ಯ_bytes(argv[0]) );
  }ಅನ್ಯಥಾ{
    z = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ;
    len = 0;
    ಯದಿ( p1<0 ){
      ಪ್ರತಿ(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  ಯದಿ( argc==3 ){
    p2 = sqlite3_ಮೌಲ್ಯ_int64(argv[2]);
    ಯದಿ( p2==0 && sqlite3_ಮೌಲ್ಯ_type(argv[2])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  }ಅನ್ಯಥಾ{
    p2 = LARGEST_INT64;
  }
  ಯದಿ( p1==0 ){
#ifdef SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#endif
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p1<0 ){
    p1 += len;
    ಯದಿ( p1<0 ){
      ಯದಿ( p2<0 ){
        p2 = 0;
      }ಅನ್ಯಥಾ{
        p2 += p1;
      }
      p1 = 0;
    }
  }ಅನ್ಯಥಾ ಯದಿ( p1>0 ){
    p1--;
  }ಅನ್ಯಥಾ ಯದಿ( p2>0 ){
    p2--;
  }
  ಯದಿ( p2<0 ){
    ಯದಿ( p2<-p1 ){
      p2 = p1;
    }ಅನ್ಯಥಾ{
      p2 = -p2;
    }
    p1 -= p2;
  }
  ಪ್ರತಿಜ್ಞಾ( p1>=0 && p2>=0 );
  ಯದಿ( p0type!=SQLITE_BLOB ){
    ಯದಿ( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ಧ್ರುವ unsigned ವರ್ಣ *zEnd = z + sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
      ಯಾವತ್( p1>=8 && zEnd-z>=8 ){
        u64 word;
        memcpy(&word, z, ಪರಿಮಾಣ(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        ಯದಿ( containsNulOrNonAscii(word) ){
          ಭಙ್ಗ;
        }
        z += 8;
        p1 -= 8;
      }
    }
    ಪ್ರತಿ( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      ಯದಿ( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }ಅನ್ಯಥಾ ಯದಿ( z[0]==0 ){
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ{
        ಕುರು{ z++; }ಯಾವತ್( (z[0]&0xc0)==0x80 );
      }
    }
    ಪ್ರತಿ(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      ಯದಿ( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }ಅನ್ಯಥಾ ಯದಿ( z2[0]==0 ){
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ{
        ಕುರು{ z2++; }ಯಾವತ್( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_result_text64(context, (ವರ್ಣ*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }ಅನ್ಯಥಾ{
    ಯದಿ( p1>=len ){
      p1 = p2 = 0;
    }ಅನ್ಯಥಾ ಯದಿ( p2>len-p1 ){
      p2 = len-p1;
      ಪ್ರತಿಜ್ಞಾ( p2>0 );
    }
    sqlite3_result_blob64(context, (ವರ್ಣ*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
ಸ್ಥಿರ ಶೂನ್ಯ roundFunc(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  i64 n = 0;
  ದ್ವಿಗುಣ r;
  ವರ್ಣ *zBuf;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಯದಿ( argc==2 ){
    ಯದಿ( SQLITE_NULL==sqlite3_ಮೌಲ್ಯ_type(argv[1]) ) ಪ್ರತ್ಯಾಗಮ;
    n = sqlite3_ಮೌಲ್ಯ_int64(argv[1]);
    ಯದಿ( n>30 ) n = 30;
    ಯದಿ( n<0 ) n = 0;
  }
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  r = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  ಯದಿ( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }ಅನ್ಯಥಾ ಯದಿ( n==0 ){ 
    r = (ದ್ವಿಗುಣ)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }ಅನ್ಯಥಾ{
    zBuf = sqlite3_mprintf("%!.*f",(ಪೂರ್ಣಾಙ್ಕ)n,r);
    ಯದಿ( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      ಪ್ರತ್ಯಾಗಮ;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_result_double(context, r);
}
#endif

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *contextMalloc(sqlite3_context *context, i64 nByte){
  ವರ್ಣ *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪ್ರತಿಜ್ಞಾ( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  ಯದಿ( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }ಅನ್ಯಥಾ{
    z = sqlite3Malloc(nByte);
    ಯದಿ( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  ಪ್ರತ್ಯಾಗಮ z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
ಸ್ಥಿರ ಶೂನ್ಯ upperFunc(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ವರ್ಣ *z1;
  ಧ್ರುವ ವರ್ಣ *z2;
  ಪೂರ್ಣಾಙ್ಕ i, n;
  UNUSED_PARAMETER(argc);
  z2 = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  n = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ಪ್ರತಿಜ್ಞಾ( z2==(ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]) );
  ಯದಿ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ಯದಿ( z1 ){
      ಪ್ರತಿ(i=0; i<n; i++){
        z1[i] = (ವರ್ಣ)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ lowerFunc(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ವರ್ಣ *z1;
  ಧ್ರುವ ವರ್ಣ *z2;
  ಪೂರ್ಣಾಙ್ಕ i, n;
  UNUSED_PARAMETER(argc);
  z2 = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  n = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ಪ್ರತಿಜ್ಞಾ( z2==(ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]) );
  ಯದಿ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ಯದಿ( z1 ){
      ಪ್ರತಿ(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
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
#define noopFunc versionFunc   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
ಸ್ಥಿರ ಶೂನ್ಯ randomFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(ಪರಿಮಾಣ(r), &r);
  ಯದಿ( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & LARGEST_INT64);
  }
  sqlite3_result_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
ಸ್ಥಿರ ಶೂನ್ಯ randomBlob(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_int64 n;
  unsigned ವರ್ಣ *p;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
  ಯದಿ( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  ಯದಿ( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (ವರ್ಣ*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ last_ಸೇರಿಸು_rowid(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_ಸೇರಿಸು_rowid(db));
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
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ total_changes(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_result_int64(context, sqlite3_total_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
ಸಂರಚನಾ compareInfo {
  u8 matchAll;          /* "*" or "%" */
  u8 matchOne;          /* "?" or "_" */
  u8 matchSet;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#ಯದಿ defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#ಅನ್ಯಥಾ
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#define SQLITE_MATCH             0
#define SQLITE_NOMATCH           1
#define SQLITE_NOWILDCARDMATCH   2

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ patternCompare(
  ಧ್ರುವ u8 *zPattern,              /* The glob pattern */
  ಧ್ರುವ u8 *zString,               /* The string to compare against the glob */
  ಧ್ರುವ ಸಂರಚನಾ compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ಧ್ರುವ u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  ಯಾವತ್( (c = Utf8Read(zPattern))!=0 ){
    ಯದಿ( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      ಯಾವತ್( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        ಯದಿ( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          ಪ್ರತ್ಯಾಗಮ SQLITE_NOWILDCARDMATCH;
        }
      }
      ಯದಿ( c==0 ){
        ಪ್ರತ್ಯಾಗಮ SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }ಅನ್ಯಥಾ ಯದಿ( c==matchOther ){
        ಯದಿ( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          ಯದಿ( c==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOWILDCARDMATCH;
        }ಅನ್ಯಥಾ{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          ಪ್ರತಿಜ್ಞಾ( matchOther<0x80 );  /* '[' is a single-byte character */
          ಯಾವತ್( *zString ){
            ಪೂರ್ಣಾಙ್ಕ bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            ಯದಿ( bMatch!=SQLITE_NOMATCH ) ಪ್ರತ್ಯಾಗಮ bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          ಪ್ರತ್ಯಾಗಮ SQLITE_NOWILDCARDMATCH;
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
      ಯದಿ( c<0x80 ){
        ವರ್ಣ zStop[3];
        ಪೂರ್ಣಾಙ್ಕ bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        ಯದಿ( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        ಯದಿ( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }ಅನ್ಯಥಾ{
          zStop[0] = c;
          zStop[1] = 0;
        }
        ಯಾವತ್(1){
          zString += strcspn((ಧ್ರುವ ವರ್ಣ*)zString, zStop);
          ಯದಿ( zString[0]==0 ) ಭಙ್ಗ;
          zString++;
          ಯದಿ( nx && zString[0]!=nx && zString[0]!=ny ) ಅನುವರ್ತ;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          ಯದಿ( bMatch!=SQLITE_NOMATCH ) ಪ್ರತ್ಯಾಗಮ bMatch;
        }
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ bMatch;
        ಯಾವತ್( (c2 = Utf8Read(zString))!=0 ){
          ಯದಿ( c2!=c ) ಅನುವರ್ತ;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          ಯದಿ( bMatch!=SQLITE_NOMATCH ) ಪ್ರತ್ಯಾಗಮ bMatch;
        }
      }
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOWILDCARDMATCH;
    }
    ಯದಿ( c==matchOther ){
      ಯದಿ( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        ಯದಿ( c==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMATCH;
        zEscaped = zPattern;
      }ಅನ್ಯಥಾ{
        u32 prior_c = 0;
        ಪೂರ್ಣಾಙ್ಕ seen = 0;
        ಪೂರ್ಣಾಙ್ಕ invert = 0;
        c = sqlite3Utf8Read(&zString);
        ಯದಿ( c==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        ಯದಿ( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        ಯದಿ( c2==']' ){
          ಯದಿ( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        ಯಾವತ್( c2 && c2!=']' ){
          ಯದಿ( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            ಯದಿ( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }ಅನ್ಯಥಾ{
            ಯದಿ( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        ಯದಿ( c2==0 || (seen ^ invert)==0 ){
          ಪ್ರತ್ಯಾಗಮ SQLITE_NOMATCH;
        }
        ಅನುವರ್ತ;
      }
    }
    c2 = Utf8Read(zString);
    ಯದಿ( c==c2 ) ಅನುವರ್ತ;
    ಯದಿ( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      ಅನುವರ್ತ;
    }
    ಯದಿ( c==matchOne && zPattern!=zEscaped && c2!=0 ) ಅನುವರ್ತ;
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMATCH;
  }
  ಪ್ರತ್ಯಾಗಮ *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_strglob(ಧ್ರುವ ವರ್ಣ *zGlobPattern, ಧ್ರುವ ವರ್ಣ *zString){
  ಯದಿ( zString==0 ){
    ಪ್ರತ್ಯಾಗಮ zGlobPattern!=0;
  }ಅನ್ಯಥಾ ಯದಿ( zGlobPattern==0 ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ {
    ಪ್ರತ್ಯಾಗಮ patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_strlike(ಧ್ರುವ ವರ್ಣ *zPattern, ಧ್ರುವ ವರ್ಣ *zStr, unsigned ಪೂರ್ಣಾಙ್ಕ esc){
  ಯದಿ( zStr==0 ){
    ಪ್ರತ್ಯಾಗಮ zPattern!=0;
  }ಅನ್ಯಥಾ ಯದಿ( zPattern==0 ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
ಪೂರ್ಣಾಙ್ಕ sqlite3_like_ಎಣಿಕೆ = 0;
#endif


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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ unsigned ವರ್ಣ *zA, *zB;
  u32 escape;
  ಪೂರ್ಣಾಙ್ಕ nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಸಂರಚನಾ compareInfo *pInfo = sqlite3_user_data(context);
  ಸಂರಚನಾ compareInfo backupInfo;

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_BLOB
   || sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_ಎಣಿಕೆ++;
#endif
    sqlite3_result_int(context, 0);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  ಯದಿ( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ಧ್ರುವ unsigned ವರ್ಣ *zEsc = sqlite3_ಮೌಲ್ಯ_text(argv[2]);
    ಯದಿ( zEsc==0 ) ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ( sqlite3Utf8CharLen((ವರ್ಣ*)zEsc, -1)!=1 ){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      ಪ್ರತ್ಯಾಗಮ;
    }
    escape = sqlite3Utf8Read(&zEsc);
    ಯದಿ( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      memcpy(&backupInfo, pInfo, ಪರಿಮಾಣ(backupInfo));
      pInfo = &backupInfo;
      ಯದಿ( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      ಯದಿ( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }ಅನ್ಯಥಾ{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  zA = sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  ಯದಿ( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_ಎಣಿಕೆ++;
#endif
    sqlite3_result_int(context,
                      patternCompare(zB, zA, pInfo, escape)==SQLITE_MATCH);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
ಸ್ಥಿರ ಶೂನ್ಯ nullifFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  ಯದಿ( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_ಮೌಲ್ಯ(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
ಸ್ಥಿರ ಶೂನ್ಯ versionFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_result_text(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sourceidFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_result_text(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
ಸ್ಥಿರ ಶೂನ್ಯ errlogFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_ಮೌಲ್ಯ_int(argv[0]), "%s", sqlite3_ಮೌಲ್ಯ_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
ಸ್ಥಿರ ಶೂನ್ಯ compileoptionusedFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ ವರ್ಣ *zOptName;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  ಯದಿ( (zOptName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]))!=0 ){
    sqlite3_result_int(context, sqlite3_compileoption_used(zOptName));
  }
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
ಸ್ಥಿರ ಶೂನ್ಯ compileoptiongetFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  i64 n;
  ಧ್ರುವ ವರ್ಣ *zRes;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((ಪೂರ್ಣಾಙ್ಕ)n) : 0;
  sqlite3_result_text(context, zRes, -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
ಶೂನ್ಯ sqlite3_Quote_ಮೌಲ್ಯ(StrAccum *pStr, sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ, ಪೂರ್ಣಾಙ್ಕ bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  ಪ್ರತಿಜ್ಞಾ( pStr!=0 && pStr->nChar==0 );

  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(p_ಮೌಲ್ಯ) ){
    ಪಕ್ಷ SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_ಮೌಲ್ಯ_double(p_ಮೌಲ್ಯ));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_ಮೌಲ್ಯ_int64(p_ಮೌಲ್ಯ));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_BLOB: {
      ವರ್ಣ ಧ್ರುವ *zBlob = sqlite3_ಮೌಲ್ಯ_blob(p_ಮೌಲ್ಯ);
      i64 nBlob = sqlite3_ಮೌಲ್ಯ_bytes(p_ಮೌಲ್ಯ);
      ಪ್ರತಿಜ್ಞಾ( zBlob==sqlite3_ಮೌಲ್ಯ_blob(p_ಮೌಲ್ಯ) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      ಯದಿ( pStr->accError==0 ){
        ವರ್ಣ *zText = pStr->zText;
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_TEXT: {
      ಧ್ರುವ unsigned ವರ್ಣ *zArg = sqlite3_ಮೌಲ್ಯ_text(p_ಮೌಲ್ಯ);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಮೌಲ್ಯ_type(p_ಮೌಲ್ಯ)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      ಭಙ್ಗ;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isNHex(ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ N, u32 *pVal){
  ಪೂರ್ಣಾಙ್ಕ i;
  u32 v = 0;
  ಪ್ರತಿ(i=0; i<N; i++){
    ಯದಿ( !sqlite3Isxdigit(z[i]) ) ಪ್ರತ್ಯಾಗಮ 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  ಪ್ರತ್ಯಾಗಮ 1;
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ವರ್ಣ *zOut;
  ಧ್ರುವ ವರ್ಣ *zIn;
  ಪೂರ್ಣಾಙ್ಕ nIn;
  ಪೂರ್ಣಾಙ್ಕ i, j, n;
  u32 v;

  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಯದಿ( zIn==0 ) ಪ್ರತ್ಯಾಗಮ;
  nIn = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  ಯದಿ( zOut==0 ){
    sqlite3_result_error_nomem(context);
    ಪ್ರತ್ಯಾಗಮ;
  }
  i = j = 0;
  ಯಾವತ್( i<nIn ){
    ಧ್ರುವ ವರ್ಣ *z = strchr(&zIn[i],'\\');
    ಯದಿ( z==0 ){
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      ಭಙ್ಗ;
    }
    n = z - &zIn[i];
    ಯದಿ( n>0 ){
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    ಯದಿ( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }ಅನ್ಯಥಾ ಯದಿ( sqlite3Isxdigit(zIn[i+1]) ){
      ಯದಿ( !isNHex(&zIn[i+1], 4, &v) ) ಗಚ್ಛ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }ಅನ್ಯಥಾ ಯದಿ( zIn[i+1]=='+' ){
      ಯದಿ( !isNHex(&zIn[i+2], 6, &v) ) ಗಚ್ಛ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }ಅನ್ಯಥಾ ಯದಿ( zIn[i+1]=='u' ){
      ಯದಿ( !isNHex(&zIn[i+2], 4, &v) ) ಗಚ್ಛ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }ಅನ್ಯಥಾ ಯದಿ( zIn[i+1]=='U' ){
      ಯದಿ( !isNHex(&zIn[i+2], 8, &v) ) ಗಚ್ಛ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }ಅನ್ಯಥಾ{
      ಗಚ್ಛ unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  ಪ್ರತ್ಯಾಗಮ;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಶೂನ್ಯ quoteFunc(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  sqlite3_Quote_ಮೌಲ್ಯ(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_data(context)));
  sqlite3_result_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unicodeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ unsigned ವರ್ಣ *z = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  (ಶೂನ್ಯ)argc;
  ಯದಿ( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ charFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  unsigned ವರ್ಣ *z, *zOut;
  ಪೂರ್ಣಾಙ್ಕ i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  ಯದಿ( z==0 ){
    sqlite3_result_error_nomem(context);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿ(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_ಮೌಲ್ಯ_int64(argv[i]);
    ಯದಿ( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    ಯದಿ( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }ಅನ್ಯಥಾ ಯದಿ( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }ಅನ್ಯಥಾ ಯದಿ( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }ಅನ್ಯಥಾ{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_result_text64(context, (ವರ್ಣ*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
ಸ್ಥಿರ ಶೂನ್ಯ hexFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ i, n;
  ಧ್ರುವ unsigned ವರ್ಣ *pBlob;
  ವರ್ಣ *zHex, *z;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
  n = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( pBlob==sqlite3_ಮೌಲ್ಯ_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  ಯದಿ( zHex ){
    ಪ್ರತಿ(i=0; i<n; i++, pBlob++){
      unsigned ವರ್ಣ c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_result_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ strContainsChar(ಧ್ರುವ u8 *zStr, ಪೂರ್ಣಾಙ್ಕ nStr, u32 ch){
  ಧ್ರುವ u8 *zEnd = &zStr[nStr];
  ಧ್ರುವ u8 *z = zStr;
  ಯಾವತ್( z<zEnd ){
    u32 tst = Utf8Read(z);
    ಯದಿ( tst==ch ) ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ u8 *zPass = (ಧ್ರುವ u8*)"";
  ಪೂರ್ಣಾಙ್ಕ nPass = 0;
  ಧ್ರುವ u8 *zHex = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಪೂರ್ಣಾಙ್ಕ nHex = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
#ifdef SQLITE_DEBUG
  ಧ್ರುವ u8 *zEnd = zHex ? &zHex[nHex] : 0;
#endif
  u8 *pBlob = 0;
  u8 *p = 0;

  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಯದಿ( argc==2 ){
    zPass = sqlite3_ಮೌಲ್ಯ_text(argv[1]);
    nPass = sqlite3_ಮೌಲ್ಯ_bytes(argv[1]);
  }
  ಯದಿ( !zHex || !zPass ) ಪ್ರತ್ಯಾಗಮ;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  ಯದಿ( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    ಯಾವತ್( (c = *zHex)!=0x00 ){
      ಯಾವತ್( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        ಪ್ರತಿಜ್ಞಾ( zHex<=zEnd );
        ಯದಿ( !strContainsChar(zPass, nPass, ch) ) ಗಚ್ಛ unhex_null;
        c = *zHex;
        ಯದಿ( c==0x00 ) ಗಚ್ಛ unhex_done;
      }
      zHex++;
      ಪ್ರತಿಜ್ಞಾ( *zEnd==0x00 );
      ಪ್ರತಿಜ್ಞಾ( zHex<=zEnd );
      d = *(zHex++);
      ಯದಿ( !sqlite3Isxdigit(d) ) ಗಚ್ಛ unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  ಪ್ರತ್ಯಾಗಮ;

 unhex_null:
  sqlite3_free(pBlob);
  ಪ್ರತ್ಯಾಗಮ;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ zeroblobFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  i64 n;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
  ಯದಿ( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  ಯದಿ( rc ){
    sqlite3_result_error_code(context, rc);
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ unsigned ವರ್ಣ *zStr;        /* The input string A */
  ಧ್ರುವ unsigned ವರ್ಣ *zPattern;    /* The pattern string B */
  ಧ್ರುವ unsigned ವರ್ಣ *zRep;        /* The replacement string C */
  unsigned ವರ್ಣ *zOut;              /* The output */
  ಪೂರ್ಣಾಙ್ಕ nStr;                /* Size of zStr */
  ಪೂರ್ಣಾಙ್ಕ nPattern;            /* Size of zPattern */
  ಪೂರ್ಣಾಙ್ಕ nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  ಪೂರ್ಣಾಙ್ಕ loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  unsigned cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  ಪ್ರತಿಜ್ಞಾ( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಯದಿ( zStr==0 ) ಪ್ರತ್ಯಾಗಮ;
  nStr = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( zStr==sqlite3_ಮೌಲ್ಯ_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  ಯದಿ( zPattern==0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( zPattern[0]==0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಮೌಲ್ಯ_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_text(context, (ಧ್ರುವ ವರ್ಣ*)zStr, nStr, SQLITE_TRANSIENT);
    ಪ್ರತ್ಯಾಗಮ;
  }
  nPattern = sqlite3_ಮೌಲ್ಯ_bytes(argv[1]);
  ಪ್ರತಿಜ್ಞಾ( zPattern==sqlite3_ಮೌಲ್ಯ_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_ಮೌಲ್ಯ_text(argv[2]);
  ಯದಿ( zRep==0 ) ಪ್ರತ್ಯಾಗಮ;
  nRep = sqlite3_ಮೌಲ್ಯ_bytes(argv[2]);
  ಪ್ರತಿಜ್ಞಾ( zRep==sqlite3_ಮೌಲ್ಯ_text(argv[2]) );
  nOut = nStr + 1;
  ಪ್ರತಿಜ್ಞಾ( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, nOut);
  ಯದಿ( zOut==0 ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  ಪ್ರತಿ(i=j=0; i<=loopLimit; i++){
    ಯದಿ( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }ಅನ್ಯಥಾ{
      ಯದಿ( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
        ಯದಿ( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          ಪ್ರತ್ಯಾಗಮ;
        }
        cntExpand++;
        ಯದಿ( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (ಪೂರ್ಣಾಙ್ಕ)nOut + (nOut - nStr - 1));
          ಯದಿ( zOut==0 ){
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            ಪ್ರತ್ಯಾಗಮ;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( j+nStr-i+1<=nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  ಪ್ರತಿಜ್ಞಾ( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (ವರ್ಣ*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
ಸ್ಥಿರ ಶೂನ್ಯ trimFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ unsigned ವರ್ಣ *zIn;         /* Input string */
  ಧ್ರುವ unsigned ವರ್ಣ *zCharSet;    /* Set of characters to trim */
  unsigned ಪೂರ್ಣಾಙ್ಕ nIn;                 /* Number of bytes in input */
  ಪೂರ್ಣಾಙ್ಕ flags;                        /* 1: trimleft  2: trimright  3: trim */
  ಪೂರ್ಣಾಙ್ಕ i;                            /* Loop counter */
  unsigned ಪೂರ್ಣಾಙ್ಕ *aLen = 0;           /* Length of each character in zCharSet */
  unsigned ವರ್ಣ **azChar = 0;       /* Individual characters in zCharSet */
  ಪೂರ್ಣಾಙ್ಕ nChar;                        /* Number of characters in zCharSet */

  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  zIn = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಯದಿ( zIn==0 ) ಪ್ರತ್ಯಾಗಮ;
  nIn = (unsigned)sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( zIn==sqlite3_ಮೌಲ್ಯ_text(argv[0]) );
  ಯದಿ( argc==1 ){
    ಸ್ಥಿರ ಧ್ರುವ unsigned lenOne[] = { 1 };
    ಸ್ಥಿರ unsigned ವರ್ಣ * ಧ್ರುವ azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (unsigned*)lenOne;
    azChar = (unsigned ವರ್ಣ **)azOne;
    zCharSet = 0;
  }ಅನ್ಯಥಾ ಯದಿ( (zCharSet = sqlite3_ಮೌಲ್ಯ_text(argv[1]))==0 ){
    ಪ್ರತ್ಯಾಗಮ;
  }ಅನ್ಯಥಾ{
    ಧ್ರುವ unsigned ವರ್ಣ *z;
    ಪ್ರತಿ(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    ಯದಿ( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(ಪರಿಮಾಣ(ವರ್ಣ*)+ಪರಿಮಾಣ(unsigned)));
      ಯದಿ( azChar==0 ){
        ಪ್ರತ್ಯಾಗಮ;
      }
      aLen = (unsigned*)&azChar[nChar];
      ಪ್ರತಿ(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned ವರ್ಣ *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  ಯದಿ( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    ಯದಿ( flags & 1 ){
      ಯಾವತ್( nIn>0 ){
        unsigned ಪೂರ್ಣಾಙ್ಕ len = 0;
        ಪ್ರತಿ(i=0; i<nChar; i++){
          len = aLen[i];
          ಯದಿ( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) ಭಙ್ಗ;
        }
        ಯದಿ( i>=nChar ) ಭಙ್ಗ;
        zIn += len;
        nIn -= len;
      }
    }
    ಯದಿ( flags & 2 ){
      ಯಾವತ್( nIn>0 ){
        unsigned ಪೂರ್ಣಾಙ್ಕ len = 0;
        ಪ್ರತಿ(i=0; i<nChar; i++){
          len = aLen[i];
          ಯದಿ( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) ಭಙ್ಗ;
        }
        ಯದಿ( i>=nChar ) ಭಙ್ಗ;
        nIn -= len;
      }
    }
    ಯದಿ( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (ವರ್ಣ*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ concatFuncCore(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  ಪೂರ್ಣಾಙ್ಕ nSep,
  ಧ್ರುವ ವರ್ಣ *zSep
){
  i64 j, n = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ bNotNull = 0;   /* True after at least NOT NULL argument seen */
  ವರ್ಣ *z;
  ಪ್ರತಿ(i=0; i<argc; i++){
    n += sqlite3_ಮೌಲ್ಯ_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  ಯದಿ( z==0 ){
    sqlite3_result_error_nomem(context);
    ಪ್ರತ್ಯಾಗಮ;
  }
  j = 0;
  ಪ್ರತಿ(i=0; i<argc; i++){
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[i])!=SQLITE_NULL ){
      ಪೂರ್ಣಾಙ್ಕ k = sqlite3_ಮೌಲ್ಯ_bytes(argv[i]);
      ಧ್ರುವ ವರ್ಣ *v = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
      ಯದಿ( v!=0 ){
        ಯದಿ( bNotNull && nSep>0 ){
          memcpy(&z[j], zSep, nSep);
          j += nSep;
        }
        memcpy(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  ಪ್ರತಿಜ್ಞಾ( j<=n );
  sqlite3_result_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
ಸ್ಥಿರ ಶೂನ್ಯ concatFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ nSep = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  ಧ್ರುವ ವರ್ಣ *zSep = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಯದಿ( zSep==0 ) ಪ್ರತ್ಯಾಗಮ;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  /* no-op */
  (ಶೂನ್ಯ)context;
  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)argv;
}
#endif /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#ifdef SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
ಸ್ಥಿರ ಶೂನ್ಯ soundexFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ವರ್ಣ zResult[8];
  ಧ್ರುವ u8 *zIn;
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ iCode[] = {
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
  zIn = (u8*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಯದಿ( zIn==0 ) zIn = (u8*)"";
  ಪ್ರತಿ(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  ಯದಿ( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    ಪ್ರತಿ(j=1; j<4 && zIn[i]; i++){
      ಪೂರ್ಣಾಙ್ಕ code = iCode[zIn[i]&0x7f];
      ಯದಿ( code>0 ){
        ಯದಿ( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }ಅನ್ಯಥಾ{
        prevcode = 0;
      }
    }
    ಯಾವತ್( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }ಅನ್ಯಥಾ{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
ಸ್ಥಿರ ಶೂನ್ಯ oomFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#endif /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
ಸ್ಥಿರ ಶೂನ್ಯ loadExt(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಧ್ರುವ ವರ್ಣ *zFile = (ಧ್ರುವ ವರ್ಣ *)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಧ್ರುವ ವರ್ಣ *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ವರ್ಣ *z_Err_ಸಂದೇಶ = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  ಯದಿ( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ( argc==2 ){
    zProc = (ಧ್ರುವ ವರ್ಣ *)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  }ಅನ್ಯಥಾ{
    zProc = 0;
  }
  ಯದಿ( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_ಸಂದೇಶ) ){
    sqlite3_result_error(context, z_Err_ಸಂದೇಶ, -1);
    sqlite3_free(z_Err_ಸಂದೇಶ);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ SumCtx SumCtx;
ಸಂರಚನಾ SumCtx {
  ದ್ವಿಗುಣ rSum;      /* Running sum as as a double */
  ದ್ವಿಗುಣ rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 iSum;         /* Running sum as a signed integer */
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
ಸ್ಥಿರ ಶೂನ್ಯ kahanBabuskaNeumaierStep(
  ಚಞ್ಚಲ SumCtx *pSum,
  ಚಞ್ಚಲ ದ್ವಿಗುಣ r
){
  ಚಞ್ಚಲ ದ್ವಿಗುಣ s = pSum->rSum;
  ಚಞ್ಚಲ ದ್ವಿಗುಣ t = s + r;
  ಯದಿ( fabs(s) > fabs(r) ){
    pSum->rErr += (s - t) + r;
  }ಅನ್ಯಥಾ{
    pSum->rErr += (r - t) + s;
  }
  pSum->rSum = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
ಸ್ಥಿರ ಶೂನ್ಯ kahanBabuskaNeumaierStepInt64(ಚಞ್ಚಲ SumCtx *pSum, i64 iVal){
  ಯದಿ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  }ಅನ್ಯಥಾ{
    kahanBabuskaNeumaierStep(pSum, (ದ್ವಿಗುಣ)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
ಸ್ಥಿರ ಶೂನ್ಯ kahanBabuskaNeumaierInit(
  ಚಞ್ಚಲ SumCtx *p,
  i64 iVal
){
  ಯದಿ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->rSum = (ದ್ವಿಗುಣ)(iVal - iSm);
    p->rErr = (ದ್ವಿಗುಣ)iSm;
  }ಅನ್ಯಥಾ{
    p->rSum = (ದ್ವಿಗುಣ)iVal;
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
ಸ್ಥಿರ ಶೂನ್ಯ sumStep(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  SumCtx *p;
  ಪೂರ್ಣಾಙ್ಕ type;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, ಪರಿಮಾಣ(*p));
  type = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಯದಿ( p && type!=SQLITE_NULL ){
    p->cnt++;
    ಯದಿ( p->approx==0 ){
      ಯದಿ( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_ಮೌಲ್ಯ_double(argv[0]));
      }ಅನ್ಯಥಾ{
        i64 x = p->iSum;
        ಯದಿ( sqlite3AddInt64(&x, sqlite3_ಮೌಲ್ಯ_int64(argv[0]))==0 ){
          p->iSum = x;
        }ಅನ್ಯಥಾ{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_ಮೌಲ್ಯ_int64(argv[0]));
        }
      }
    }ಅನ್ಯಥಾ{
      ಯದಿ( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_ಮೌಲ್ಯ_int64(argv[0]));
      }ಅನ್ಯಥಾ{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_ಮೌಲ್ಯ_double(argv[0]));
      }
    }
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ sumInverse(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ**argv){
  SumCtx *p;
  ಪೂರ್ಣಾಙ್ಕ type;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, ಪರಿಮಾಣ(*p));
  type = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  ಯದಿ( ALWAYS(p) && type!=SQLITE_NULL ){
    ಪ್ರತಿಜ್ಞಾ( p->cnt>0 );
    p->cnt--;
    ಯದಿ( !p->approx ){
      i64 x = p->iSum;
      ಯದಿ( sqlite3SubInt64(&x, sqlite3_ಮೌಲ್ಯ_int64(argv[0]))==0 ){
        p->iSum = x;
        ಪ್ರತ್ಯಾಗಮ;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    ಯದಿ( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
      ಯದಿ( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }ಅನ್ಯಥಾ{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }ಅನ್ಯಥಾ{
      kahanBabuskaNeumaierStep(p, -sqlite3_ಮೌಲ್ಯ_double(argv[0]));
    }
  }
}
#ಅನ್ಯಥಾ
# define sumInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
ಸ್ಥಿರ ಶೂನ್ಯ sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  ಯದಿ( p && p->cnt>0 ){
    ಯದಿ( p->approx ){
      ಯದಿ( p->ovrfl ){
        sqlite3_result_error(context,"integer overflow",-1);
      }ಅನ್ಯಥಾ ಯದಿ( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_result_double(context, p->rSum+p->rErr);
      }ಅನ್ಯಥಾ{
        sqlite3_result_double(context, p->rSum);
      }
    }ಅನ್ಯಥಾ{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  ಯದಿ( p && p->cnt>0 ){
    ದ್ವಿಗುಣ r;
    ಯದಿ( p->approx ){
      r = p->rSum;
      ಯದಿ( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }ಅನ್ಯಥಾ{
      r = (ದ್ವಿಗುಣ)(p->iSum);
    }
    sqlite3_result_double(context, r/(ದ್ವಿಗುಣ)p->cnt);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ totalFinalize(sqlite3_context *context){
  SumCtx *p;
  ದ್ವಿಗುಣ r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  ಯದಿ( p ){
    ಯದಿ( p->approx ){
      r = p->rSum;
      ಯದಿ( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }ಅನ್ಯಥಾ{
      r = (ದ್ವಿಗುಣ)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ಎಣಿಕೆ_Ctx ಎಣಿಕೆ_Ctx;
ಸಂರಚನಾ ಎಣಿಕೆ_Ctx {
  i64 n;
#ifdef SQLITE_DEBUG
  ಪೂರ್ಣಾಙ್ಕ bInverse;                   /* True if xInverse() ever called */
#endif
};

/*
** Routines to implement the count() aggregate function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಎಣಿಕೆ_Step(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಎಣಿಕೆ_Ctx *p;
  p = sqlite3_aggregate_context(context, ಪರಿಮಾಣ(*p));
  ಯದಿ( (argc==0 || SQLITE_NULL!=sqlite3_ಮೌಲ್ಯ_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  ಪ್ರತಿಜ್ಞಾ( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_ಎಣಿಕೆ(context) );
#endif
}  
ಸ್ಥಿರ ಶೂನ್ಯ ಎಣಿಕೆ_Finalize(sqlite3_context *context){
  ಎಣಿಕೆ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ ಎಣಿಕೆ_Inverse(sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಎಣಿಕೆ_Ctx *p;
  p = sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(*p));
  /* p is always non-NULL since countStep() will have been called first */
  ಯದಿ( (argc==0 || SQLITE_NULL!=sqlite3_ಮೌಲ್ಯ_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ifdef SQLITE_DEBUG
    p->bInverse = 1;
#endif
  }
}  
#ಅನ್ಯಥಾ
# define ಎಣಿಕೆ_Inverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
ಸ್ಥಿರ ಶೂನ್ಯ minmaxStep(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, ಪರಿಮಾಣ(*pBest));
  ಯದಿ( !pBest ) ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(pArg)==SQLITE_NULL ){
    ಯದಿ( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }ಅನ್ಯಥಾ ಯದಿ( pBest->flags ){
    ಪೂರ್ಣಾಙ್ಕ max;
    ಪೂರ್ಣಾಙ್ಕ cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_data(context)!=0;
    cmp = sqlite3MemCompare(pBest, pArg, pColl);
    ಯದಿ( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }ಅನ್ಯಥಾ{
      sqlite3SkipAccumulatorLoad(context);
    }
  }ಅನ್ಯಥಾ{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ min_Max_ಮೌಲ್ಯ_Finalize(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ b_ಮೌಲ್ಯ){
  sqlite3_ಮೌಲ್ಯ *pRes;
  pRes = (sqlite3_ಮೌಲ್ಯ *)sqlite3_aggregate_context(context, 0);
  ಯದಿ( pRes ){
    ಯದಿ( pRes->flags ){
      sqlite3_result_ಮೌಲ್ಯ(context, pRes);
    }
    ಯದಿ( b_ಮೌಲ್ಯ==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ min_Max_ಮೌಲ್ಯ(sqlite3_context *context){
  min_Max_ಮೌಲ್ಯ_Finalize(context, 1);
}
#ಅನ್ಯಥಾ
# define min_Max_ಮೌಲ್ಯ 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
  StrAccum str;          /* The accumulated concatenation */
#ifndef SQLITE_OMIT_WINDOWFUNC
  ಪೂರ್ಣಾಙ್ಕ nAccum;            /* Number of strings presently concatenated */
  ಪೂರ್ಣಾಙ್ಕ nFirstSepLength;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  ಪೂರ್ಣಾಙ್ಕ *pnSepLengths;
#endif
} GroupConcatCtx;

ಸ್ಥಿರ ಶೂನ್ಯ groupConcatStep(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ ವರ್ಣ *zVal;
  GroupConcatCtx *pGCC;
  ಧ್ರುವ ವರ್ಣ *zSep;
  ಪೂರ್ಣಾಙ್ಕ nVal, nSep;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, ಪರಿಮಾಣ(*pGCC));
  ಯದಿ( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    ಪೂರ್ಣಾಙ್ಕ firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    ಯದಿ( argc==1 ){
      ಯದಿ( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಅನ್ಯಥಾ{
        pGCC->nFirstSepLength = 1;
      }
#endif
    }ಅನ್ಯಥಾ ಯದಿ( !firstTerm ){
      zSep = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
      nSep = sqlite3_ಮೌಲ್ಯ_bytes(argv[1]);
      ಯದಿ( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಅನ್ಯಥಾ{
        nSep = 0;
      }
      ಯದಿ( nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0 ){
        ಪೂರ್ಣಾಙ್ಕ *pnsl = pGCC->pnSepLengths;
        ಯದಿ( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (ಪೂರ್ಣಾಙ್ಕ*)sqlite3_malloc64((pGCC->nAccum+1) * ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
          ಯದಿ( pnsl!=0 ){
            ಪೂರ್ಣಾಙ್ಕ i = 0, nA = pGCC->nAccum-1;
            ಯಾವತ್( i<nA ) pnsl[i++] = pGCC->nFirstSepLength;
          }
        }ಅನ್ಯಥಾ{
          pnsl = (ಪೂರ್ಣಾಙ್ಕ*)sqlite3_realloc64(pnsl, pGCC->nAccum * ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
        }
        ಯದಿ( pnsl!=0 ){
          ಯದಿ( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }ಅನ್ಯಥಾ{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#endif
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಅನ್ಯಥಾ{
      pGCC->nFirstSepLength = sqlite3_ಮೌಲ್ಯ_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#endif
    zVal = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    nVal = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
    ಯದಿ( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ifndef SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ groupConcatInverse(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  GroupConcatCtx *pGCC;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  (ಶೂನ್ಯ)argc;  /* Suppress unused parameter warning */
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, ಪರಿಮಾಣ(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  ಯದಿ( ALWAYS(pGCC) ){
    ಪೂರ್ಣಾಙ್ಕ nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (ಶೂನ್ಯ)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    nVS = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
    pGCC->nAccum -= 1;
    ಯದಿ( pGCC->pnSepLengths!=0 ){
      ಪ್ರತಿಜ್ಞಾ(pGCC->nAccum >= 0);
      ಯದಿ( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
      }
    }ಅನ್ಯಥಾ{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->nFirstSepLength;
    }
    ಯದಿ( nVS>=(ಪೂರ್ಣಾಙ್ಕ)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }ಅನ್ಯಥಾ{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    ಯದಿ( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#ಅನ್ಯಥಾ
# define groupConcatInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
ಸ್ಥಿರ ಶೂನ್ಯ groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  ಯದಿ( pGCC ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_XFER);
#ifndef SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#endif
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ಸ್ಥಿರ ಶೂನ್ಯ group_Concat_ಮೌಲ್ಯ(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  ಯದಿ( pGCC && pGCC->nAccum>0 ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#ಅನ್ಯಥಾ
# define group_Concat_ಮೌಲ್ಯ 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
ಶೂನ್ಯ sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_overload_function(db, "MATCH", 2);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  ಯದಿ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
ಶೂನ್ಯ sqlite3RegisterLikeFunctions(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ caseSensitive){
  FuncDef *pDef;
  ಸಂರಚನಾ compareInfo *pInfo;
  ಪೂರ್ಣಾಙ್ಕ flags;
  ಪೂರ್ಣಾಙ್ಕ nArg;
  ಯದಿ( caseSensitive ){
    pInfo = (ಸಂರಚನಾ compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }ಅನ್ಯಥಾ{
    pInfo = (ಸಂರಚನಾ compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  ಪ್ರತಿ(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3IsLikeFunction(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ *pIsNocase, ವರ್ಣ *aWc){
  FuncDef *pDef;
  ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಯದಿ( !p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  n_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
  pDef = sqlite3FindFunction(db, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, n_ಅಭಿವ್ಯಕ್ತಿ, SQLITE_UTF8, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ಯದಿ( pDef==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಯದಿ( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  ಪ್ರತಿಜ್ಞಾ( (ವರ್ಣ*)&likeInfoAlt == (ವರ್ಣ*)&likeInfoAlt.matchAll );
  ಪ್ರತಿಜ್ಞಾ( &((ವರ್ಣ*)&likeInfoAlt)[1] == (ವರ್ಣ*)&likeInfoAlt.matchOne );
  ಪ್ರತಿಜ್ಞಾ( &((ವರ್ಣ*)&likeInfoAlt)[2] == (ವರ್ಣ*)&likeInfoAlt.matchSet );

  ಯದಿ( n_ಅಭಿವ್ಯಕ್ತಿ<3 ){
    aWc[3] = 0;
  }ಅನ್ಯಥಾ{
    ಅಭಿವ್ಯಕ್ತಿ *pEscape = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[2].p_ಅಭಿವ್ಯಕ್ತಿ;
    ವರ್ಣ *zEscape;
    ಯದಿ( pEscape->op!=TK_STRING ) ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pEscape, EP_Int_ಮೌಲ್ಯ) );
    zEscape = pEscape->u.zToken;
    ಯದಿ( zEscape[0]==0 || zEscape[1]!=0 ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( zEscape[0]==aWc[0] ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( zEscape[0]==aWc[1] ) ಪ್ರತ್ಯಾಗಮ 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  ಪ್ರತ್ಯಾಗಮ 1;
}

/* Mathematical Constants */
#ifndef M_PI
# define M_PI   3.141592653589793238462643383279502884
#endif
#ifndef M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#endif
#ifndef M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#endif


/* Extra math functions that require linking with -lm
*/
#ifdef SQLITE_ENABLE_MATH_FUNCTIONS
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]) ){
    ಪಕ್ಷ SQLITE_INTEGER: {
       sqlite3_result_int64(context, sqlite3_ಮೌಲ್ಯ_int64(argv[0]));
       ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_FLOAT: {
       ದ್ವಿಗುಣ (*x)(ದ್ವಿಗುಣ) = (ದ್ವಿಗುಣ(*)(ದ್ವಿಗುಣ))sqlite3_user_data(context);
       sqlite3_result_double(context, x(sqlite3_ಮೌಲ್ಯ_double(argv[0])));
       ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
       ಭಙ್ಗ;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
ಸ್ಥಿರ ದ್ವಿಗುಣ xCeil(ದ್ವಿಗುಣ x){ ಪ್ರತ್ಯಾಗಮ ceil(x); }
ಸ್ಥಿರ ದ್ವಿಗುಣ xFloor(ದ್ವಿಗುಣ x){ ಪ್ರತ್ಯಾಗಮ floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#ಯದಿ defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#endif
#ಯದಿ defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*log(X))
#endif


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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ದ್ವಿಗುಣ x, b, ans;
  ಪ್ರತಿಜ್ಞಾ( argc==1 || argc==2 );
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]) ){
    ಪಕ್ಷ SQLITE_INTEGER:
    ಪಕ್ಷ SQLITE_FLOAT:
      x = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
      ಯದಿ( x<=0.0 ) ಪ್ರತ್ಯಾಗಮ;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( argc==2 ){
    ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_numeric_type(argv[1]) ){
      ಪಕ್ಷ SQLITE_INTEGER:
      ಪಕ್ಷ SQLITE_FLOAT:
        b = log(x);
        ಯದಿ( b<=0.0 ) ಪ್ರತ್ಯಾಗಮ;
        x = sqlite3_ಮೌಲ್ಯ_double(argv[1]);
        ಯದಿ( x<=0.0 ) ಪ್ರತ್ಯಾಗಮ;
        ಭಙ್ಗ;
     ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ;
    }
    ans = log(x)/b;
  }ಅನ್ಯಥಾ{
    ವಿಕಲ್ಪ( SQLITE_PTR_TO_INT(sqlite3_user_data(context)) ){
      ಪಕ್ಷ 1:
        ans = log10(x);
        ಭಙ್ಗ;
      ಪಕ್ಷ 2:
        ans = log2(x);
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        ans = log(x);
        ಭಙ್ಗ;
    }
  }
  sqlite3_result_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
ಸ್ಥಿರ ದ್ವಿಗುಣ degToRad(ದ್ವಿಗುಣ x){ ಪ್ರತ್ಯಾಗಮ x*(M_PI/180.0); }
ಸ್ಥಿರ ದ್ವಿಗುಣ radToDeg(ದ್ವಿಗುಣ x){ ಪ್ರತ್ಯಾಗಮ x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
ಸ್ಥಿರ ಶೂನ್ಯ math1Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ type0;
  ದ್ವಿಗುಣ v0, ans;
  ದ್ವಿಗುಣ (*x)(ದ್ವಿಗುಣ);
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  type0 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಯದಿ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ಪ್ರತ್ಯಾಗಮ;
  v0 = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  x = (ದ್ವಿಗುಣ(*)(ದ್ವಿಗುಣ))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
ಸ್ಥಿರ ಶೂನ್ಯ math2Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ type0, type1;
  ದ್ವಿಗುಣ v0, v1, ans;
  ದ್ವಿಗುಣ (*x)(ದ್ವಿಗುಣ,ದ್ವಿಗುಣ);
  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  type0 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಯದಿ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ಪ್ರತ್ಯಾಗಮ;
  type1 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[1]);
  ಯದಿ( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) ಪ್ರತ್ಯಾಗಮ;
  v0 = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  v1 = sqlite3_ಮೌಲ್ಯ_double(argv[1]);
  x = (ದ್ವಿಗುಣ(*)(ದ್ವಿಗುಣ,ದ್ವಿಗುಣ))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ piFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪ್ರತಿಜ್ಞಾ( argc==0 );
  (ಶೂನ್ಯ)argv;
  sqlite3_result_double(context, M_PI);
}

#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ signFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ type0;
  ದ್ವಿಗುಣ x;
  UNUSED_PARAMETER(argc);
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  type0 = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[0]);
  ಯದಿ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ಪ್ರತ್ಯಾಗಮ;
  x = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  sqlite3_result_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#ಯದಿ defined(SQLITE_ENABLE_PERCENTILE)
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Percentile Percentile;
ಸಂರಚನಾ Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  ವರ್ಣ bSorted;        /* True if a[] is already in sorted order */
  ವರ್ಣ bKeepSorted;    /* True if advantageous to keep a[] sorted */
  ವರ್ಣ bPctValid;      /* True if rPct is valid */
  ದ್ವಿಗುಣ rPct;         /* Fraction.  0.0 to 1.0 */
  ದ್ವಿಗುಣ *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ percentIsInfinity(ದ್ವಿಗುಣ r){
  sqlite3_uint64 u;
  ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(u)==ಪರಿಮಾಣ(r) );
  memcpy(&u, &r, ಪರಿಮಾಣ(u));
  ಪ್ರತ್ಯಾಗಮ ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ percent_Same_ಮೌಲ್ಯ(ದ್ವಿಗುಣ a, ದ್ವಿಗುಣ b){
  a -= b;
  ಪ್ರತ್ಯಾಗಮ a>=-0.001 && a<=0.001;
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
ಸ್ಥಿರ i64 percent_ದ್ವಿಮಾನ_ಹುಡುಕು(Percentile *p, ದ್ವಿಗುಣ y, ಪೂರ್ಣಾಙ್ಕ bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  ಯಾವತ್( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    ದ್ವಿಗುಣ x = p->a[iMid];
    ಯದಿ( x<y ){
      iFirst = iMid + 1;
    }ಅನ್ಯಥಾ ಯದಿ( x>y ){
      iLast = iMid - 1;
    }ಅನ್ಯಥಾ{
      ಪ್ರತ್ಯಾಗಮ iMid;
    }
  }
  ಯದಿ( bExact ) ಪ್ರತ್ಯಾಗಮ -1;
  ಪ್ರತ್ಯಾಗಮ iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentError(sqlite3_context *pCtx, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  ವರ್ಣ *zMsg1;
  ವರ್ಣ *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_result_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentStep(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  Percentile *p;
  ದ್ವಿಗುಣ rPct;
  ಪೂರ್ಣಾಙ್ಕ eType;
  ದ್ವಿಗುಣ y;
  ಪ್ರತಿಜ್ಞಾ( argc==2 || argc==1 );

  ಯದಿ( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }ಅನ್ಯಥಾ{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    ದ್ವಿಗುಣ mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_ಮೌಲ್ಯ_numeric_type(argv[1]);
    rPct = sqlite3_ಮೌಲ್ಯ_double(argv[1])/mxFrac;
    ಯದಿ( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (ದ್ವಿಗುಣ)mxFrac);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, ಪರಿಮಾಣ(*p));
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  ಯದಿ( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }ಅನ್ಯಥಾ ಯದಿ( !percent_Same_ಮೌಲ್ಯ(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    ಪ್ರತ್ಯಾಗಮ;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  ಯದಿ( eType==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ಯದಿ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    ಪ್ರತ್ಯಾಗಮ;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  ಯದಿ( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    ಪ್ರತ್ಯಾಗಮ;
  }

  /* Allocate and store the Y */
  ಯದಿ( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    ದ್ವಿಗುಣ *a = sqlite3_realloc64(p->a, ಪರಿಮಾಣ(ದ್ವಿಗುಣ)*n);
    ಯದಿ( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, ಪರಿಮಾಣ(*p));
      sqlite3_result_error_nomem(pCtx);
      ಪ್ರತ್ಯಾಗಮ;
    }
    p->nAlloc = n;
    p->a = a;
  }
  ಯದಿ( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }ಅನ್ಯಥಾ ಯದಿ( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }ಅನ್ಯಥಾ ಯದಿ( p->bKeepSorted ){
    i64 i;
    i = percent_ದ್ವಿಮಾನ_ಹುಡುಕು(p, y, 0);
    ಯದಿ( i<(ಪೂರ್ಣಾಙ್ಕ)p->nUsed ){
      memmove(&p->a[i+1], &p->a[i], (p->nUsed-i)*ಪರಿಮಾಣ(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }ಅನ್ಯಥಾ{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#define SWAP_DOUBLE(X,Y)  {ದ್ವಿಗುಣ ttt=(X);(X)=(Y);(Y)=ttt;}

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
ಸ್ಥಿರ ಶೂನ್ಯ percentSort(
  ದ್ವಿಗುಣ *a,                      /* Array to sort */
  unsigned ಪೂರ್ಣಾಙ್ಕ n,                 /* Number of elements in array a[] */
  ಪೂರ್ಣಾಙ್ಕ iReq                        /* Element caller cares about (or -ve) */
){
  ಪೂರ್ಣಾಙ್ಕ iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  ಪೂರ್ಣಾಙ್ಕ iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  ಪೂರ್ಣಾಙ್ಕ i;         /* Loop counter */
  ದ್ವಿಗುಣ rPivot; /* The pivot value */

  ಪ್ರತಿಜ್ಞಾ( n>=2 );
  ಕುರು{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    ಯದಿ( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    ಯದಿ( n==2 ) ಪ್ರತ್ಯಾಗಮ;
    iGt = n-1;
    i = n/2;
    ಯದಿ( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }ಅನ್ಯಥಾ ಯದಿ( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    ಯದಿ( n==3 ) ಪ್ರತ್ಯಾಗಮ;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    ಕುರು{
      ಯದಿ( a[i]<rPivot ){
        ಯದಿ( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }ಅನ್ಯಥಾ ಯದಿ( a[i]>rPivot ){
        ಕುರು{
          iGt--;
        }ಯಾವತ್( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }ಅನ್ಯಥಾ{
        i++;
      }
    }ಯಾವತ್( i<iGt );

    ಪ್ರತಿಜ್ಞಾ( iLt>0 && iLt<iGt && (unsigned)iGt<n );
    testcase( iGt>iLt+1 );
    ಪ್ರತಿಜ್ಞಾ( a[iLt]==rPivot );
    ಪ್ರತಿಜ್ಞಾ( a[iLt-1]<=rPivot );
    ಪ್ರತಿಜ್ಞಾ( a[iGt]>=rPivot );
    ಪ್ರತಿಜ್ಞಾ( a[iLt+1]>=rPivot );

    ಯದಿ( iReq>=0 ){
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
      ಯದಿ( iReq<iLt ){
        n = iLt;
      }ಅನ್ಯಥಾ{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }ಅನ್ಯಥಾ{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      ಯದಿ( iLt>(ಪೂರ್ಣಾಙ್ಕ)(n/2) ){
        ಯದಿ( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }ಅನ್ಯಥಾ{
        ಯದಿ( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }ಯಾವತ್( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentInverse(sqlite3_context *pCtx,ಪೂರ್ಣಾಙ್ಕ argc,sqlite3_ಮೌಲ್ಯ **argv){
  Percentile *p;
  ಪೂರ್ಣಾಙ್ಕ eType;
  ದ್ವಿಗುಣ y;
  i64 i;
  ಪ್ರತಿಜ್ಞಾ( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, ಪರಿಮಾಣ(*p));
  ಪ್ರತಿಜ್ಞಾ( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_ಮೌಲ್ಯ_type(argv[0]);
  ಯದಿ( eType==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ಯದಿ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    ಪ್ರತ್ಯಾಗಮ;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  ಯದಿ( percentIsInfinity(y) ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p->bSorted==0 ){
    ಪ್ರತಿಜ್ಞಾ( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_ದ್ವಿಮಾನ_ಹುಡುಕು(p, y, 1);
  ಯದಿ( i>=0 ){
    p->nUsed--;
    ಯದಿ( i<(ಪೂರ್ಣಾಙ್ಕ)p->nUsed ){
      memmove(&p->a[i], &p->a[i+1], (p->nUsed - i)*ಪರಿಮಾಣ(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
ಸ್ಥಿರ ಶೂನ್ಯ percentCompute(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ bIsFinal){
  Percentile *p;
  ಪೂರ್ಣಾಙ್ಕ settings = SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&1; /* Discrete? */
  unsigned i1, i2;
  ದ್ವಿಗುಣ v1, v2;
  ದ್ವಿಗುಣ ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p->a==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (unsigned)ix;
    ಯದಿ( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      ಪ್ರತಿಜ್ಞಾ( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (ಪೂರ್ಣಾಙ್ಕ)i1 : -1));
      p->bSorted = 1;
    }
    ಯದಿ( settings & 1 ){
      vx = p->a[i1];
    }ಅನ್ಯಥಾ{
      i2 = ix==(ದ್ವಿಗುಣ)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  ಯದಿ( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, ಪರಿಮಾಣ(*p));
  }ಅನ್ಯಥಾ{
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
#endif /* SQLITE_ENABLE_PERCENTILE */

#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
ಸ್ಥಿರ ಶೂನ್ಯ filestatFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಧ್ರುವ ವರ್ಣ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  ಯದಿ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    ಪೂರ್ಣಾಙ್ಕ rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ಪ್ರತಿಜ್ಞಾ( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    ಯದಿ( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }ಅನ್ಯಥಾ{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      ಯದಿ( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      ಯದಿ( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        ಯದಿ( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }ಅನ್ಯಥಾ{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಧ್ರುವ ವರ್ಣ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  ಯದಿ( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ಪ್ರತಿಜ್ಞಾ( pPager!=0 );
    pStr = sqlite3_str_new(db);
    ಯದಿ( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }ಅನ್ಯಥಾ{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }ಅನ್ಯಥಾ{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#ifdef SQLITE_DEBUG
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  FpDecode s;
  ದ್ವಿಗುಣ x;
  ಪೂರ್ಣಾಙ್ಕ y, z;
  ವರ್ಣ zBuf[100];
  UNUSED_PARAMETER(argc);
  ಪ್ರತಿಜ್ಞಾ( argc==3 );
  x = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  y = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  z = sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  ಯದಿ( z<=0 ) z = 1;
  ಯದಿ( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  ಯದಿ( s.isSpecial==2 ){
    sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf, "NaN");
  }ಅನ್ಯಥಾ{
    sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_result_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
#endif /* SQLITE_DEBUG */

#ifdef SQLITE_DEBUG
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
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_str *pResult;
  ಧ್ರುವ ವರ್ಣ *zVfs;
  ಧ್ರುವ ವರ್ಣ *zUri;
  unsigned ಪೂರ್ಣಾಙ್ಕ flgs;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_vfs *pVfs = 0;
  ವರ್ಣ *zFile = 0;
  ವರ್ಣ *zErr = 0;

  ಯದಿ( argc<2 ) ಪ್ರತ್ಯಾಗಮ;
  pVfs = sqlite3_vfs_find(0);
  ಪ್ರತಿಜ್ಞಾ( pVfs );
  zVfs = pVfs->zName;
  zUri = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಯದಿ( zUri==0 ) ಪ್ರತ್ಯಾಗಮ;
  flgs = (unsigned ಪೂರ್ಣಾಙ್ಕ)sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  pResult = sqlite3_str_new(0);
  ಯದಿ( !sqlite3_str_errcode(pResult) ){
    ಪೂರ್ಣಾಙ್ಕ i;
    sqlite3_str_appendf(pResult, "rc=%d", rc);
    sqlite3_str_appendf(pResult, ", flags=0x%x", flgs);
    sqlite3_str_appendf(pResult, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(pResult, ", err=%Q", zErr);
    sqlite3_str_appendf(pResult, ", file=%Q", zFile);
    ಯದಿ( zFile ){
      ಧ್ರುವ ವರ್ಣ *z = zFile;
      z += sqlite3Strlen30(z)+1;
      ಯಾವತ್( z[0] ){
        sqlite3_str_appendf(pResult, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      ಪ್ರತಿ(i=2; i<argc; i++){
        ಧ್ರುವ ವರ್ಣ *zArg;
        ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_INTEGER ){
          ಪೂರ್ಣಾಙ್ಕ k = sqlite3_ಮೌಲ್ಯ_int(argv[i]);
          sqlite3_str_appendf(pResult, ", '%d:%q'",k,sqlite3_uri_ಕೀಲಿ(zFile, k));
        }ಅನ್ಯಥಾ ಯದಿ( (zArg = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]))!=0 ){
          sqlite3_str_appendf(pResult, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }ಅನ್ಯಥಾ{
          sqlite3_str_appendf(pResult, ", NULL");
        }
      }
    }
  }
  sqlite3_result_str(ctx, pResult, SQLITE_FINISH);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#endif /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
ಶೂನ್ಯ sqlite3RegisterBuiltinFunctions(ಶೂನ್ಯ){
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
#ಯದಿ !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(ಅಭಿವ್ಯಕ್ತಿ_compare,        2, INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_compare,        0),
    TEST_FUNC(ಅಭಿವ್ಯಕ್ತಿ_implies_ಅಭಿವ್ಯಕ್ತಿ,   2, INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_implies_ಅಭಿವ್ಯಕ್ತಿ,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#endif /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#ifdef SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#endif
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    SFUNCTION(load_extension,    1, 0, 0, loadExt          ),
    SFUNCTION(load_extension,    2, 0, 0, loadExt          ),
#endif
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_used,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    INLINE_FUNC(sqlite_offset,   1, INLINEFUNC_sqlite_offset, 0 ),
#endif
#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#endif
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_ಮೌಲ್ಯ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_ಮೌಲ್ಯ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(length,            1, 0, 0, lengthFunc,  SQLITE_FUNC_LENGTH),
    FUNCTION2(octet_length,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(printf,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(ವರ್ಣ,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifdef SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#endif
#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#endif
#ifndef SQLITE_OMIT_FLOATING_POINT
    FUNCTION(round,              1, 0, 0, roundFunc        ),
    FUNCTION(round,              2, 0, 0, roundFunc        ),
#endif
    FUNCTION(upper,              1, 0, 0, upperFunc        ),
    FUNCTION(lower,              1, 0, 0, lowerFunc        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_version,    0, 0, 0, versionFunc      ),
    DFUNCTION(sqlite_source_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(last_ಸೇರಿಸು_rowid, 0, 0, 0, last_ಸೇರಿಸು_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(total_changes,     0, 0, 0, total_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(sum,   1,0,0, sumStep, sumFinalize, sumFinalize, sumInverse, 0),
    WAGGREGATE(total, 1,0,0, sumStep,totalFinalize,totalFinalize,sumInverse, 0),
    WAGGREGATE(avg,   1,0,0, sumStep, avgFinalize, avgFinalize, sumInverse, 0),
    WAGGREGATE(ಎಣಿಕೆ, 0,0,0, ಎಣಿಕೆ_Step,
        ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Inverse,
        SQLITE_FUNC_ಎಣಿಕೆ|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(ಎಣಿಕೆ, 1,0,0, ಎಣಿಕೆ_Step,
        ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Finalize, ಎಣಿಕೆ_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_ಮೌಲ್ಯ, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_ಮೌಲ್ಯ, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_ಮೌಲ್ಯ, groupConcatInverse, 0),

#ifdef SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_ಮೌಲ್ಯ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#endif /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ಅನ್ಯಥಾ
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#endif
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#endif
#ifdef SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#ಯದಿ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#endif
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(log,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(log,                2, 0, 0,      logFunc     ),
    MFUNCTION(exp,               1, exp,       math1Func   ),
    MFUNCTION(pow,               2, pow,       math2Func   ),
    MFUNCTION(power,             2, pow,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(cos,               1, cos,       math1Func   ),
    MFUNCTION(sin,               1, sin,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#ಯದಿ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#endif
    MFUNCTION(sqrt,              1, sqrt,      math1Func   ),
    MFUNCTION(radians,           1, degToRad,  math1Func   ),
    MFUNCTION(degrees,           1, radToDeg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(ಯದಿ,             -4, INLINEFUNC_iif,      0 ),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_ಸೇರಿಸು_Builtin_Funcs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#ಯದಿ 0  /* Enable to print out how the built-in functions are hashed */
  {
    ಪೂರ್ಣಾಙ್ಕ i;
    FuncDef *p;
    ಪ್ರತಿ(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      ಪ್ರತಿ(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        ಪೂರ್ಣಾಙ್ಕ n = sqlite3Strlen30(p->zName);
        ಪೂರ್ಣಾಙ್ಕ h = p->zName[0] + n;
        ಪ್ರತಿಜ್ಞಾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}
