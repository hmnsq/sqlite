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
#شامل "sqliteInt.h"
#شامل <stdlib.h>
#شامل <assert.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#شامل <math.h>
#endif
#شامل "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
ایستا CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  أكد( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  أكد( pOp->opcode==OP_CollSeq );
  أكد( pOp->p4type==P4_COLLSEQ );
  بازگشت pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
ایستا تهی sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  أكد( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
ایستا تهی minmaxFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح i;
  صحیح mask;    /* 0 for min() or 0xffffffff for max() */
  صحیح iBest;
  CollSeq *pColl;

  أكد( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  أكد( pColl );
  أكد( mask==-1 || mask==0 );
  iBest = 0;
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) بازگشت;
  برای(i=1; i<argc; i++){
    اگر( sqlite3_مقدار_type(argv[i])==SQLITE_NULL ) بازگشت;
    اگر( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_مقدار(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
ایستا تهی typeofFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  ایستا ثابت نویسه *azType[] = { "integer", "real", "text", "blob", "null" };
  صحیح i = sqlite3_مقدار_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  أكد( i>=0 && i<ArraySize(azType) );
  أكد( SQLITE_INTEGER==1 );
  أكد( SQLITE_FLOAT==2 );
  أكد( SQLITE_TEXT==3 );
  أكد( SQLITE_BLOB==4 );
  أكد( SQLITE_NULL==5 );
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
ایستا تهی subtypeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_result_int(context, sqlite3_مقدار_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
ایستا تهی lengthFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  انتخاب( sqlite3_مقدار_type(argv[0]) ){
    حالت SQLITE_BLOB:
    حالت SQLITE_INTEGER:
    حالت SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_مقدار_bytes(argv[0]));
      بشکن;
    }
    حالت SQLITE_TEXT: {
      ثابت unsigned نویسه *z = sqlite3_مقدار_text(argv[0]);
      ثابت unsigned نویسه *z0;
      اگر( z==0 ) بازگشت;
      z0 = z;
      وقتی( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        اگر( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }وگرنه اگر( z[0]==0 ){
          بشکن;
        }وگرنه{
          z++;
          وقتی( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_result_int(context, (صحیح)(z-z0));
      بشکن;
    }
    پیشفرض: {
      sqlite3_result_null(context);
      بشکن;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
ایستا تهی bytelengthFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  انتخاب( sqlite3_مقدار_type(argv[0]) ){
    حالت SQLITE_BLOB: {
      sqlite3_result_int(context, sqlite3_مقدار_bytes(argv[0]));
      بشکن;
    }
    حالت SQLITE_INTEGER:
    حالت SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_result_int64(context, sqlite3_مقدار_bytes(argv[0])*m);
      بشکن;
    }
    حالت SQLITE_TEXT: {
      اگر( sqlite3_مقدار_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_result_int(context, sqlite3_مقدار_bytes(argv[0]));
      }وگرنه{
        sqlite3_result_int(context, sqlite3_مقدار_bytes16(argv[0]));
      }
      بشکن;
    }
    پیشفرض: {
      sqlite3_result_null(context);
      بشکن;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
ایستا تهی absFunc(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  انتخاب( sqlite3_مقدار_type(argv[0]) ){
    حالت SQLITE_INTEGER: {
      i64 iVal = sqlite3_مقدار_int64(argv[0]);
      اگر( iVal<0 ){
        اگر( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          بازگشت;
        }
        iVal = -iVal;
      }
      sqlite3_result_int64(context, iVal);
      بشکن;
    }
    حالت SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      بشکن;
    }
    پیشفرض: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      دوگانه rVal = sqlite3_مقدار_double(argv[0]);
      اگر( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      بشکن;
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
ایستا تهی instrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت unsigned نویسه *zHaystack;
  ثابت unsigned نویسه *zNeedle;
  صحیح nHaystack;
  صحیح nNeedle;
  صحیح typeHaystack, typeNeedle;
  صحیح N = 1;
  صحیح isText;
  unsigned نویسه firstChar;
  sqlite3_مقدار *pC1 = 0;
  sqlite3_مقدار *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_مقدار_type(argv[0]);
  typeNeedle = sqlite3_مقدار_type(argv[1]);
  اگر( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) بازگشت;
  nHaystack = sqlite3_مقدار_bytes(argv[0]);
  nNeedle = sqlite3_مقدار_bytes(argv[1]);
  اگر( nNeedle>0 ){
    اگر( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_مقدار_blob(argv[0]);
      zNeedle = sqlite3_مقدار_blob(argv[1]);
      isText = 0;
    }وگرنه اگر( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_مقدار_text(argv[0]);
      zNeedle = sqlite3_مقدار_text(argv[1]);
      isText = 1;
    }وگرنه{
      pC1 = sqlite3_مقدار_dup(argv[0]);
      zHaystack = sqlite3_مقدار_text(pC1);
      اگر( zHaystack==0 ) برو endInstrOOM;
      nHaystack = sqlite3_مقدار_bytes(pC1);
      pC2 = sqlite3_مقدار_dup(argv[1]);
      zNeedle = sqlite3_مقدار_text(pC2);
      اگر( zNeedle==0 ) برو endInstrOOM;
      nNeedle = sqlite3_مقدار_bytes(pC2);
      isText = 1;
    }
    اگر( zNeedle==0 || (nHaystack && zHaystack==0) ) برو endInstrOOM;
    firstChar = zNeedle[0];
    وقتی( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      انجامبده{
        nHaystack--;
        zHaystack++;
      }وقتی( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    اگر( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_مقدار_free(pC1);
  sqlite3_مقدار_free(pC2);
  بازگشت;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  برو endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
ایستا تهی printfFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  PrintfArguments x;
  StrAccum str;
  ثابت نویسه *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  اگر( argc>=1 && (zFormat = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]))!=0 ){
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
ایستا SQLITE_INLINE صحیح containsNulOrNonAscii(u64 word){
  ثابت u64 byteOnes = UINT64_C(0x0101010101010101);
  ثابت u64 byte_بالا_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  بازگشت ((word | (word - byteOnes)) & byte_بالا_Bits)!=0;
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
ایستا تهی substrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت unsigned نویسه *z;
  ثابت unsigned نویسه *z2;
  صحیح len;
  صحیح p0type;
  i64 p1, p2;

  أكد( argc==3 || argc==2 );
  p0type = sqlite3_مقدار_type(argv[0]);
  p1 = sqlite3_مقدار_int64(argv[1]);
  اگر( p0type==SQLITE_BLOB ){
    len = sqlite3_مقدار_bytes(argv[0]);
    z = sqlite3_مقدار_blob(argv[0]);
    اگر( z==0 ) بازگشت;
    أكد( len==sqlite3_مقدار_bytes(argv[0]) );
  }وگرنه{
    z = sqlite3_مقدار_text(argv[0]);
    اگر( z==0 ) بازگشت;
    len = 0;
    اگر( p1<0 ){
      برای(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  اگر( argc==3 ){
    p2 = sqlite3_مقدار_int64(argv[2]);
    اگر( p2==0 && sqlite3_مقدار_type(argv[2])==SQLITE_NULL ) بازگشت;
  }وگرنه{
    p2 = LARGEST_INT64;
  }
  اگر( p1==0 ){
#ifdef SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#endif
    اگر( sqlite3_مقدار_type(argv[1])==SQLITE_NULL ) بازگشت;
  }
  اگر( p1<0 ){
    p1 += len;
    اگر( p1<0 ){
      اگر( p2<0 ){
        p2 = 0;
      }وگرنه{
        p2 += p1;
      }
      p1 = 0;
    }
  }وگرنه اگر( p1>0 ){
    p1--;
  }وگرنه اگر( p2>0 ){
    p2--;
  }
  اگر( p2<0 ){
    اگر( p2<-p1 ){
      p2 = p1;
    }وگرنه{
      p2 = -p2;
    }
    p1 -= p2;
  }
  أكد( p1>=0 && p2>=0 );
  اگر( p0type!=SQLITE_BLOB ){
    اگر( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ثابت unsigned نویسه *zEnd = z + sqlite3_مقدار_bytes(argv[0]);
      وقتی( p1>=8 && zEnd-z>=8 ){
        u64 word;
        memcpy(&word, z, اندازه(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        اگر( containsNulOrNonAscii(word) ){
          بشکن;
        }
        z += 8;
        p1 -= 8;
      }
    }
    برای( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      اگر( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }وگرنه اگر( z[0]==0 ){
        بشکن;
      }وگرنه{
        انجامبده{ z++; }وقتی( (z[0]&0xc0)==0x80 );
      }
    }
    برای(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      اگر( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }وگرنه اگر( z2[0]==0 ){
        بشکن;
      }وگرنه{
        انجامبده{ z2++; }وقتی( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_result_text64(context, (نویسه*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }وگرنه{
    اگر( p1>=len ){
      p1 = p2 = 0;
    }وگرنه اگر( p2>len-p1 ){
      p2 = len-p1;
      أكد( p2>0 );
    }
    sqlite3_result_blob64(context, (نویسه*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
ایستا تهی roundFunc(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  i64 n = 0;
  دوگانه r;
  نویسه *zBuf;
  أكد( argc==1 || argc==2 );
  اگر( argc==2 ){
    اگر( SQLITE_NULL==sqlite3_مقدار_type(argv[1]) ) بازگشت;
    n = sqlite3_مقدار_int64(argv[1]);
    اگر( n>30 ) n = 30;
    اگر( n<0 ) n = 0;
  }
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) بازگشت;
  r = sqlite3_مقدار_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  اگر( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }وگرنه اگر( n==0 ){ 
    r = (دوگانه)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }وگرنه{
    zBuf = sqlite3_mprintf("%!.*f",(صحیح)n,r);
    اگر( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      بازگشت;
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
ایستا تهی *contextMalloc(sqlite3_context *context, i64 nByte){
  نویسه *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  أكد( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  اگر( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }وگرنه{
    z = sqlite3Malloc(nByte);
    اگر( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  بازگشت z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
ایستا تهی upperFunc(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  نویسه *z1;
  ثابت نویسه *z2;
  صحیح i, n;
  UNUSED_PARAMETER(argc);
  z2 = (نویسه*)sqlite3_مقدار_text(argv[0]);
  n = sqlite3_مقدار_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  أكد( z2==(نویسه*)sqlite3_مقدار_text(argv[0]) );
  اگر( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    اگر( z1 ){
      برای(i=0; i<n; i++){
        z1[i] = (نویسه)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
ایستا تهی lowerFunc(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  نویسه *z1;
  ثابت نویسه *z2;
  صحیح i, n;
  UNUSED_PARAMETER(argc);
  z2 = (نویسه*)sqlite3_مقدار_text(argv[0]);
  n = sqlite3_مقدار_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  أكد( z2==(نویسه*)sqlite3_مقدار_text(argv[0]) );
  اگر( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    اگر( z1 ){
      برای(i=0; i<n; i++){
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
ایستا تهی randomFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(اندازه(r), &r);
  اگر( r<0 ){
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
ایستا تهی randomBlob(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3_int64 n;
  unsigned نویسه *p;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_مقدار_int64(argv[0]);
  اگر( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  اگر( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (نویسه*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
ایستا تهی last_درج_rowid(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_درج_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
ایستا تهی changes(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
ایستا تهی total_changes(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **NotUsed2
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
ساختار compareInfo {
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
#اگر defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#وگرنه
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

ایستا ثابت ساختار compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
ایستا ثابت ساختار compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
ایستا ثابت ساختار compareInfo likeInfoAlt = { '%', '_',   0, 0 };

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
ایستا صحیح patternCompare(
  ثابت u8 *zPattern,              /* The glob pattern */
  ثابت u8 *zString,               /* The string to compare against the glob */
  ثابت ساختار compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ثابت u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  وقتی( (c = Utf8Read(zPattern))!=0 ){
    اگر( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      وقتی( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        اگر( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          بازگشت SQLITE_NOWILDCARDMATCH;
        }
      }
      اگر( c==0 ){
        بازگشت SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }وگرنه اگر( c==matchOther ){
        اگر( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          اگر( c==0 ) بازگشت SQLITE_NOWILDCARDMATCH;
        }وگرنه{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          أكد( matchOther<0x80 );  /* '[' is a single-byte character */
          وقتی( *zString ){
            صحیح bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            اگر( bMatch!=SQLITE_NOMATCH ) بازگشت bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          بازگشت SQLITE_NOWILDCARDMATCH;
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
      اگر( c<0x80 ){
        نویسه zStop[3];
        صحیح bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        اگر( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        اگر( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }وگرنه{
          zStop[0] = c;
          zStop[1] = 0;
        }
        وقتی(1){
          zString += strcspn((ثابت نویسه*)zString, zStop);
          اگر( zString[0]==0 ) بشکن;
          zString++;
          اگر( nx && zString[0]!=nx && zString[0]!=ny ) ادامه;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          اگر( bMatch!=SQLITE_NOMATCH ) بازگشت bMatch;
        }
      }وگرنه{
        صحیح bMatch;
        وقتی( (c2 = Utf8Read(zString))!=0 ){
          اگر( c2!=c ) ادامه;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          اگر( bMatch!=SQLITE_NOMATCH ) بازگشت bMatch;
        }
      }
      بازگشت SQLITE_NOWILDCARDMATCH;
    }
    اگر( c==matchOther ){
      اگر( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        اگر( c==0 ) بازگشت SQLITE_NOMATCH;
        zEscaped = zPattern;
      }وگرنه{
        u32 prior_c = 0;
        صحیح seen = 0;
        صحیح invert = 0;
        c = sqlite3Utf8Read(&zString);
        اگر( c==0 ) بازگشت SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        اگر( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        اگر( c2==']' ){
          اگر( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        وقتی( c2 && c2!=']' ){
          اگر( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            اگر( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }وگرنه{
            اگر( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        اگر( c2==0 || (seen ^ invert)==0 ){
          بازگشت SQLITE_NOMATCH;
        }
        ادامه;
      }
    }
    c2 = Utf8Read(zString);
    اگر( c==c2 ) ادامه;
    اگر( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      ادامه;
    }
    اگر( c==matchOne && zPattern!=zEscaped && c2!=0 ) ادامه;
    بازگشت SQLITE_NOMATCH;
  }
  بازگشت *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
صحیح sqlite3_strglob(ثابت نویسه *zGlobPattern, ثابت نویسه *zString){
  اگر( zString==0 ){
    بازگشت zGlobPattern!=0;
  }وگرنه اگر( zGlobPattern==0 ){
    بازگشت 1;
  }وگرنه {
    بازگشت patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
صحیح sqlite3_strlike(ثابت نویسه *zPattern, ثابت نویسه *zStr, unsigned صحیح esc){
  اگر( zStr==0 ){
    بازگشت zPattern!=0;
  }وگرنه اگر( zPattern==0 ){
    بازگشت 1;
  }وگرنه{
    بازگشت patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
صحیح sqlite3_like_شمار = 0;
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
ایستا تهی likeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت unsigned نویسه *zA, *zB;
  u32 escape;
  صحیح nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ساختار compareInfo *pInfo = sqlite3_user_data(context);
  ساختار compareInfo backupInfo;

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_BLOB
   || sqlite3_مقدار_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_شمار++;
#endif
    sqlite3_result_int(context, 0);
    بازگشت;
  }
#endif

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_مقدار_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  اگر( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    بازگشت;
  }
  اگر( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ثابت unsigned نویسه *zEsc = sqlite3_مقدار_text(argv[2]);
    اگر( zEsc==0 ) بازگشت;
    اگر( sqlite3Utf8CharLen((نویسه*)zEsc, -1)!=1 ){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      بازگشت;
    }
    escape = sqlite3Utf8Read(&zEsc);
    اگر( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      memcpy(&backupInfo, pInfo, اندازه(backupInfo));
      pInfo = &backupInfo;
      اگر( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      اگر( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }وگرنه{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_مقدار_text(argv[0]);
  zA = sqlite3_مقدار_text(argv[1]);
  اگر( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_شمار++;
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
ایستا تهی nullifFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  اگر( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_مقدار(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
ایستا تهی versionFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **NotUsed2
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
ایستا تهی sourceidFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **NotUsed2
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
ایستا تهی errlogFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_مقدار_int(argv[0]), "%s", sqlite3_مقدار_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
ایستا تهی compileoptionusedFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت نویسه *zOptName;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  اگر( (zOptName = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]))!=0 ){
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
ایستا تهی compileoptiongetFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  i64 n;
  ثابت نویسه *zRes;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_مقدار_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((صحیح)n) : 0;
  sqlite3_result_text(context, zRes, -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
ایستا ثابت نویسه hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
تهی sqlite3_Quote_مقدار(StrAccum *pStr, sqlite3_مقدار *p_مقدار, صحیح bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  أكد( pStr!=0 && pStr->nChar==0 );

  انتخاب( sqlite3_مقدار_type(p_مقدار) ){
    حالت SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_مقدار_double(p_مقدار));
      بشکن;
    }
    حالت SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_مقدار_int64(p_مقدار));
      بشکن;
    }
    حالت SQLITE_BLOB: {
      نویسه ثابت *zBlob = sqlite3_مقدار_blob(p_مقدار);
      i64 nBlob = sqlite3_مقدار_bytes(p_مقدار);
      أكد( zBlob==sqlite3_مقدار_blob(p_مقدار) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      اگر( pStr->accError==0 ){
        نویسه *zText = pStr->zText;
        صحیح i;
        برای(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      بشکن;
    }
    حالت SQLITE_TEXT: {
      ثابت unsigned نویسه *zArg = sqlite3_مقدار_text(p_مقدار);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      بشکن;
    }
    پیشفرض: {
      أكد( sqlite3_مقدار_type(p_مقدار)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      بشکن;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
ایستا صحیح isNHex(ثابت نویسه *z, صحیح N, u32 *pVal){
  صحیح i;
  u32 v = 0;
  برای(i=0; i<N; i++){
    اگر( !sqlite3Isxdigit(z[i]) ) بازگشت 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  بازگشت 1;
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
ایستا تهی unistrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  نویسه *zOut;
  ثابت نویسه *zIn;
  صحیح nIn;
  صحیح i, j, n;
  u32 v;

  أكد( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  اگر( zIn==0 ) بازگشت;
  nIn = sqlite3_مقدار_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  اگر( zOut==0 ){
    sqlite3_result_error_nomem(context);
    بازگشت;
  }
  i = j = 0;
  وقتی( i<nIn ){
    ثابت نویسه *z = strchr(&zIn[i],'\\');
    اگر( z==0 ){
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      بشکن;
    }
    n = z - &zIn[i];
    اگر( n>0 ){
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    اگر( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }وگرنه اگر( sqlite3Isxdigit(zIn[i+1]) ){
      اگر( !isNHex(&zIn[i+1], 4, &v) ) برو unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }وگرنه اگر( zIn[i+1]=='+' ){
      اگر( !isNHex(&zIn[i+2], 6, &v) ) برو unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }وگرنه اگر( zIn[i+1]=='u' ){
      اگر( !isNHex(&zIn[i+2], 4, &v) ) برو unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }وگرنه اگر( zIn[i+1]=='U' ){
      اگر( !isNHex(&zIn[i+2], 8, &v) ) برو unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }وگرنه{
      برو unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  بازگشت;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  بازگشت;
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
ایستا تهی quoteFunc(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  sqlite3_Quote_مقدار(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_data(context)));
  sqlite3_result_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
ایستا تهی unicodeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت unsigned نویسه *z = sqlite3_مقدار_text(argv[0]);
  (تهی)argc;
  اگر( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
ایستا تهی charFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  unsigned نویسه *z, *zOut;
  صحیح i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  اگر( z==0 ){
    sqlite3_result_error_nomem(context);
    بازگشت;
  }
  برای(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_مقدار_int64(argv[i]);
    اگر( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    اگر( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }وگرنه اگر( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }وگرنه اگر( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }وگرنه{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_result_text64(context, (نویسه*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
ایستا تهی hexFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح i, n;
  ثابت unsigned نویسه *pBlob;
  نویسه *zHex, *z;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_مقدار_blob(argv[0]);
  n = sqlite3_مقدار_bytes(argv[0]);
  أكد( pBlob==sqlite3_مقدار_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  اگر( zHex ){
    برای(i=0; i<n; i++, pBlob++){
      unsigned نویسه c = *pBlob;
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
ایستا صحیح strContainsChar(ثابت u8 *zStr, صحیح nStr, u32 ch){
  ثابت u8 *zEnd = &zStr[nStr];
  ثابت u8 *z = zStr;
  وقتی( z<zEnd ){
    u32 tst = Utf8Read(z);
    اگر( tst==ch ) بازگشت 1;
  }
  بازگشت 0;
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
ایستا تهی unhexFunc(
  sqlite3_context *pCtx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت u8 *zPass = (ثابت u8*)"";
  صحیح nPass = 0;
  ثابت u8 *zHex = sqlite3_مقدار_text(argv[0]);
  صحیح nHex = sqlite3_مقدار_bytes(argv[0]);
#ifdef SQLITE_DEBUG
  ثابت u8 *zEnd = zHex ? &zHex[nHex] : 0;
#endif
  u8 *pBlob = 0;
  u8 *p = 0;

  أكد( argc==1 || argc==2 );
  اگر( argc==2 ){
    zPass = sqlite3_مقدار_text(argv[1]);
    nPass = sqlite3_مقدار_bytes(argv[1]);
  }
  اگر( !zHex || !zPass ) بازگشت;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  اگر( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    وقتی( (c = *zHex)!=0x00 ){
      وقتی( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        أكد( zHex<=zEnd );
        اگر( !strContainsChar(zPass, nPass, ch) ) برو unhex_null;
        c = *zHex;
        اگر( c==0x00 ) برو unhex_done;
      }
      zHex++;
      أكد( *zEnd==0x00 );
      أكد( zHex<=zEnd );
      d = *(zHex++);
      اگر( !sqlite3Isxdigit(d) ) برو unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  بازگشت;

 unhex_null:
  sqlite3_free(pBlob);
  بازگشت;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
ایستا تهی zeroblobFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  i64 n;
  صحیح rc;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_مقدار_int64(argv[0]);
  اگر( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  اگر( rc ){
    sqlite3_result_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
ایستا تهی replaceFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت unsigned نویسه *zStr;        /* The input string A */
  ثابت unsigned نویسه *zPattern;    /* The pattern string B */
  ثابت unsigned نویسه *zRep;        /* The replacement string C */
  unsigned نویسه *zOut;              /* The output */
  صحیح nStr;                /* Size of zStr */
  صحیح nPattern;            /* Size of zPattern */
  صحیح nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  صحیح loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  unsigned cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  أكد( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_مقدار_text(argv[0]);
  اگر( zStr==0 ) بازگشت;
  nStr = sqlite3_مقدار_bytes(argv[0]);
  أكد( zStr==sqlite3_مقدار_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_مقدار_text(argv[1]);
  اگر( zPattern==0 ){
    أكد( sqlite3_مقدار_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    بازگشت;
  }
  اگر( zPattern[0]==0 ){
    أكد( sqlite3_مقدار_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_text(context, (ثابت نویسه*)zStr, nStr, SQLITE_TRANSIENT);
    بازگشت;
  }
  nPattern = sqlite3_مقدار_bytes(argv[1]);
  أكد( zPattern==sqlite3_مقدار_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_مقدار_text(argv[2]);
  اگر( zRep==0 ) بازگشت;
  nRep = sqlite3_مقدار_bytes(argv[2]);
  أكد( zRep==sqlite3_مقدار_text(argv[2]) );
  nOut = nStr + 1;
  أكد( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, nOut);
  اگر( zOut==0 ){
    بازگشت;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  برای(i=j=0; i<=loopLimit; i++){
    اگر( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }وگرنه{
      اگر( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
        اگر( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          بازگشت;
        }
        cntExpand++;
        اگر( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (صحیح)nOut + (nOut - nStr - 1));
          اگر( zOut==0 ){
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            بازگشت;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  أكد( j+nStr-i+1<=nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  أكد( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (نویسه*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
ایستا تهی trimFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت unsigned نویسه *zIn;         /* Input string */
  ثابت unsigned نویسه *zCharSet;    /* Set of characters to trim */
  unsigned صحیح nIn;                 /* Number of bytes in input */
  صحیح flags;                        /* 1: trimleft  2: trimright  3: trim */
  صحیح i;                            /* Loop counter */
  unsigned صحیح *aLen = 0;           /* Length of each character in zCharSet */
  unsigned نویسه **azChar = 0;       /* Individual characters in zCharSet */
  صحیح nChar;                        /* Number of characters in zCharSet */

  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ){
    بازگشت;
  }
  zIn = sqlite3_مقدار_text(argv[0]);
  اگر( zIn==0 ) بازگشت;
  nIn = (unsigned)sqlite3_مقدار_bytes(argv[0]);
  أكد( zIn==sqlite3_مقدار_text(argv[0]) );
  اگر( argc==1 ){
    ایستا ثابت unsigned lenOne[] = { 1 };
    ایستا unsigned نویسه * ثابت azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (unsigned*)lenOne;
    azChar = (unsigned نویسه **)azOne;
    zCharSet = 0;
  }وگرنه اگر( (zCharSet = sqlite3_مقدار_text(argv[1]))==0 ){
    بازگشت;
  }وگرنه{
    ثابت unsigned نویسه *z;
    برای(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    اگر( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(اندازه(نویسه*)+اندازه(unsigned)));
      اگر( azChar==0 ){
        بازگشت;
      }
      aLen = (unsigned*)&azChar[nChar];
      برای(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned نویسه *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  اگر( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    اگر( flags & 1 ){
      وقتی( nIn>0 ){
        unsigned صحیح len = 0;
        برای(i=0; i<nChar; i++){
          len = aLen[i];
          اگر( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) بشکن;
        }
        اگر( i>=nChar ) بشکن;
        zIn += len;
        nIn -= len;
      }
    }
    اگر( flags & 2 ){
      وقتی( nIn>0 ){
        unsigned صحیح len = 0;
        برای(i=0; i<nChar; i++){
          len = aLen[i];
          اگر( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) بشکن;
        }
        اگر( i>=nChar ) بشکن;
        nIn -= len;
      }
    }
    اگر( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (نویسه*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
ایستا تهی concatFuncCore(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv,
  صحیح nSep,
  ثابت نویسه *zSep
){
  i64 j, n = 0;
  صحیح i;
  صحیح bNotNull = 0;   /* True after at least NOT NULL argument seen */
  نویسه *z;
  برای(i=0; i<argc; i++){
    n += sqlite3_مقدار_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  اگر( z==0 ){
    sqlite3_result_error_nomem(context);
    بازگشت;
  }
  j = 0;
  برای(i=0; i<argc; i++){
    اگر( sqlite3_مقدار_type(argv[i])!=SQLITE_NULL ){
      صحیح k = sqlite3_مقدار_bytes(argv[i]);
      ثابت نویسه *v = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
      اگر( v!=0 ){
        اگر( bNotNull && nSep>0 ){
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
  أكد( j<=n );
  sqlite3_result_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
ایستا تهی concatFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
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
ایستا تهی concatwsFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح nSep = sqlite3_مقدار_bytes(argv[0]);
  ثابت نویسه *zSep = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  اگر( zSep==0 ) بازگشت;
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
ایستا تهی unknownFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  /* no-op */
  (تهی)context;
  (تهی)argc;
  (تهی)argv;
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
ایستا تهی soundexFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  نویسه zResult[8];
  ثابت u8 *zIn;
  صحیح i, j;
  ایستا ثابت unsigned نویسه iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  أكد( argc==1 );
  zIn = (u8*)sqlite3_مقدار_text(argv[0]);
  اگر( zIn==0 ) zIn = (u8*)"";
  برای(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  اگر( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    برای(j=1; j<4 && zIn[i]; i++){
      صحیح code = iCode[zIn[i]&0x7f];
      اگر( code>0 ){
        اگر( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }وگرنه{
        prevcode = 0;
      }
    }
    وقتی( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }وگرنه{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
ایستا تهی oomFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#endif /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
ایستا تهی loadExt(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  ثابت نویسه *zFile = (ثابت نویسه *)sqlite3_مقدار_text(argv[0]);
  ثابت نویسه *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  نویسه *z_Err_پیام = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  اگر( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    بازگشت;
  }

  اگر( argc==2 ){
    zProc = (ثابت نویسه *)sqlite3_مقدار_text(argv[1]);
  }وگرنه{
    zProc = 0;
  }
  اگر( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_پیام) ){
    sqlite3_result_error(context, z_Err_پیام, -1);
    sqlite3_free(z_Err_پیام);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
تعریفنوع ساختار SumCtx SumCtx;
ساختار SumCtx {
  دوگانه rSum;      /* Running sum as as a double */
  دوگانه rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
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
ایستا تهی kahanBabuskaNeumaierStep(
  فرار SumCtx *pSum,
  فرار دوگانه r
){
  فرار دوگانه s = pSum->rSum;
  فرار دوگانه t = s + r;
  اگر( fabs(s) > fabs(r) ){
    pSum->rErr += (s - t) + r;
  }وگرنه{
    pSum->rErr += (r - t) + s;
  }
  pSum->rSum = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
ایستا تهی kahanBabuskaNeumaierStepInt64(فرار SumCtx *pSum, i64 iVal){
  اگر( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  }وگرنه{
    kahanBabuskaNeumaierStep(pSum, (دوگانه)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
ایستا تهی kahanBabuskaNeumaierInit(
  فرار SumCtx *p,
  i64 iVal
){
  اگر( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->rSum = (دوگانه)(iVal - iSm);
    p->rErr = (دوگانه)iSm;
  }وگرنه{
    p->rSum = (دوگانه)iVal;
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
ایستا تهی sumStep(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  SumCtx *p;
  صحیح type;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, اندازه(*p));
  type = sqlite3_مقدار_numeric_type(argv[0]);
  اگر( p && type!=SQLITE_NULL ){
    p->cnt++;
    اگر( p->approx==0 ){
      اگر( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_مقدار_double(argv[0]));
      }وگرنه{
        i64 x = p->iSum;
        اگر( sqlite3AddInt64(&x, sqlite3_مقدار_int64(argv[0]))==0 ){
          p->iSum = x;
        }وگرنه{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_مقدار_int64(argv[0]));
        }
      }
    }وگرنه{
      اگر( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_مقدار_int64(argv[0]));
      }وگرنه{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_مقدار_double(argv[0]));
      }
    }
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ایستا تهی sumInverse(sqlite3_context *context, صحیح argc, sqlite3_مقدار**argv){
  SumCtx *p;
  صحیح type;
  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, اندازه(*p));
  type = sqlite3_مقدار_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  اگر( ALWAYS(p) && type!=SQLITE_NULL ){
    أكد( p->cnt>0 );
    p->cnt--;
    اگر( !p->approx ){
      i64 x = p->iSum;
      اگر( sqlite3SubInt64(&x, sqlite3_مقدار_int64(argv[0]))==0 ){
        p->iSum = x;
        بازگشت;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    اگر( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_مقدار_int64(argv[0]);
      اگر( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }وگرنه{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }وگرنه{
      kahanBabuskaNeumaierStep(p, -sqlite3_مقدار_double(argv[0]));
    }
  }
}
#وگرنه
# define sumInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
ایستا تهی sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  اگر( p && p->cnt>0 ){
    اگر( p->approx ){
      اگر( p->ovrfl ){
        sqlite3_result_error(context,"integer overflow",-1);
      }وگرنه اگر( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_result_double(context, p->rSum+p->rErr);
      }وگرنه{
        sqlite3_result_double(context, p->rSum);
      }
    }وگرنه{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
ایستا تهی avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  اگر( p && p->cnt>0 ){
    دوگانه r;
    اگر( p->approx ){
      r = p->rSum;
      اگر( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }وگرنه{
      r = (دوگانه)(p->iSum);
    }
    sqlite3_result_double(context, r/(دوگانه)p->cnt);
  }
}
ایستا تهی totalFinalize(sqlite3_context *context){
  SumCtx *p;
  دوگانه r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  اگر( p ){
    اگر( p->approx ){
      r = p->rSum;
      اگر( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }وگرنه{
      r = (دوگانه)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
تعریفنوع ساختار شمار_Ctx شمار_Ctx;
ساختار شمار_Ctx {
  i64 n;
#ifdef SQLITE_DEBUG
  صحیح bInverse;                   /* True if xInverse() ever called */
#endif
};

/*
** Routines to implement the count() aggregate function.
*/
ایستا تهی شمار_Step(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  شمار_Ctx *p;
  p = sqlite3_aggregate_context(context, اندازه(*p));
  اگر( (argc==0 || SQLITE_NULL!=sqlite3_مقدار_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  أكد( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_شمار(context) );
#endif
}  
ایستا تهی شمار_Finalize(sqlite3_context *context){
  شمار_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ایستا تهی شمار_Inverse(sqlite3_context *ctx, صحیح argc, sqlite3_مقدار **argv){
  شمار_Ctx *p;
  p = sqlite3_aggregate_context(ctx, اندازه(*p));
  /* p is always non-NULL since countStep() will have been called first */
  اگر( (argc==0 || SQLITE_NULL!=sqlite3_مقدار_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ifdef SQLITE_DEBUG
    p->bInverse = 1;
#endif
  }
}  
#وگرنه
# define شمار_Inverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
ایستا تهی minmaxStep(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, اندازه(*pBest));
  اگر( !pBest ) بازگشت;

  اگر( sqlite3_مقدار_type(pArg)==SQLITE_NULL ){
    اگر( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }وگرنه اگر( pBest->flags ){
    صحیح max;
    صحیح cmp;
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
    اگر( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }وگرنه{
      sqlite3SkipAccumulatorLoad(context);
    }
  }وگرنه{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
ایستا تهی min_Max_مقدار_Finalize(sqlite3_context *context, صحیح b_مقدار){
  sqlite3_مقدار *pRes;
  pRes = (sqlite3_مقدار *)sqlite3_aggregate_context(context, 0);
  اگر( pRes ){
    اگر( pRes->flags ){
      sqlite3_result_مقدار(context, pRes);
    }
    اگر( b_مقدار==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ایستا تهی min_Max_مقدار(sqlite3_context *context){
  min_Max_مقدار_Finalize(context, 1);
}
#وگرنه
# define min_Max_مقدار 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
ایستا تهی minMaxFinalize(sqlite3_context *context){
  min_Max_مقدار_Finalize(context, 0);
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
تعریفنوع ساختار {
  StrAccum str;          /* The accumulated concatenation */
#ifndef SQLITE_OMIT_WINDOWFUNC
  صحیح nAccum;            /* Number of strings presently concatenated */
  صحیح nFirstSepLength;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  صحیح *pnSepLengths;
#endif
} GroupConcatCtx;

ایستا تهی groupConcatStep(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  ثابت نویسه *zVal;
  GroupConcatCtx *pGCC;
  ثابت نویسه *zSep;
  صحیح nVal, nSep;
  أكد( argc==1 || argc==2 );
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) بازگشت;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, اندازه(*pGCC));
  اگر( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    صحیح firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    اگر( argc==1 ){
      اگر( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      وگرنه{
        pGCC->nFirstSepLength = 1;
      }
#endif
    }وگرنه اگر( !firstTerm ){
      zSep = (نویسه*)sqlite3_مقدار_text(argv[1]);
      nSep = sqlite3_مقدار_bytes(argv[1]);
      اگر( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      وگرنه{
        nSep = 0;
      }
      اگر( nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0 ){
        صحیح *pnsl = pGCC->pnSepLengths;
        اگر( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (صحیح*)sqlite3_malloc64((pGCC->nAccum+1) * اندازه(صحیح));
          اگر( pnsl!=0 ){
            صحیح i = 0, nA = pGCC->nAccum-1;
            وقتی( i<nA ) pnsl[i++] = pGCC->nFirstSepLength;
          }
        }وگرنه{
          pnsl = (صحیح*)sqlite3_realloc64(pnsl, pGCC->nAccum * اندازه(صحیح));
        }
        اگر( pnsl!=0 ){
          اگر( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }وگرنه{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#endif
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    وگرنه{
      pGCC->nFirstSepLength = sqlite3_مقدار_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#endif
    zVal = (نویسه*)sqlite3_مقدار_text(argv[0]);
    nVal = sqlite3_مقدار_bytes(argv[0]);
    اگر( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ifndef SQLITE_OMIT_WINDOWFUNC
ایستا تهی groupConcatInverse(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  GroupConcatCtx *pGCC;
  أكد( argc==1 || argc==2 );
  (تهی)argc;  /* Suppress unused parameter warning */
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) بازگشت;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, اندازه(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  اگر( ALWAYS(pGCC) ){
    صحیح nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (تهی)sqlite3_مقدار_text(argv[0]);
    nVS = sqlite3_مقدار_bytes(argv[0]);
    pGCC->nAccum -= 1;
    اگر( pGCC->pnSepLengths!=0 ){
      أكد(pGCC->nAccum >= 0);
      اگر( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*اندازه(صحیح));
      }
    }وگرنه{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->nFirstSepLength;
    }
    اگر( nVS>=(صحیح)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }وگرنه{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    اگر( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#وگرنه
# define groupConcatInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
ایستا تهی groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  اگر( pGCC ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_XFER);
#ifndef SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#endif
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
ایستا تهی group_Concat_مقدار(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  اگر( pGCC && pGCC->nAccum>0 ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#وگرنه
# define group_Concat_مقدار 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
تهی sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  صحیح rc = sqlite3_overload_function(db, "MATCH", 2);
  أكد( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  اگر( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
تهی sqlite3RegisterLikeFunctions(sqlite3 *db, صحیح caseSensitive){
  FuncDef *pDef;
  ساختار compareInfo *pInfo;
  صحیح flags;
  صحیح nArg;
  اگر( caseSensitive ){
    pInfo = (ساختار compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }وگرنه{
    pInfo = (ساختار compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  برای(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    أكد( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
صحیح sqlite3IsLikeFunction(sqlite3 *db, عبارت *p_عبارت, صحیح *pIsNocase, نویسه *aWc){
  FuncDef *pDef;
  صحیح n_عبارت;
  أكد( p_عبارت!=0 );
  أكد( p_عبارت->op==TK_FUNCTION );
  أكد( عبارت_Use_XList(p_عبارت) );
  اگر( !p_عبارت->x.pList ){
    بازگشت 0;
  }
  n_عبارت = p_عبارت->x.pList->n_عبارت;
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
  pDef = sqlite3FindFunction(db, p_عبارت->u.zToken, n_عبارت, SQLITE_UTF8, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  اگر( pDef==0 ) بازگشت 0;
#endif
  اگر( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    بازگشت 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  أكد( (نویسه*)&likeInfoAlt == (نویسه*)&likeInfoAlt.matchAll );
  أكد( &((نویسه*)&likeInfoAlt)[1] == (نویسه*)&likeInfoAlt.matchOne );
  أكد( &((نویسه*)&likeInfoAlt)[2] == (نویسه*)&likeInfoAlt.matchSet );

  اگر( n_عبارت<3 ){
    aWc[3] = 0;
  }وگرنه{
    عبارت *pEscape = p_عبارت->x.pList->a[2].p_عبارت;
    نویسه *zEscape;
    اگر( pEscape->op!=TK_STRING ) بازگشت 0;
    أكد( !عبارت_Has_Property(pEscape, EP_Int_مقدار) );
    zEscape = pEscape->u.zToken;
    اگر( zEscape[0]==0 || zEscape[1]!=0 ) بازگشت 0;
    اگر( zEscape[0]==aWc[0] ) بازگشت 0;
    اگر( zEscape[0]==aWc[1] ) بازگشت 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  بازگشت 1;
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
ایستا تهی ceilingFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==1 );
  انتخاب( sqlite3_مقدار_numeric_type(argv[0]) ){
    حالت SQLITE_INTEGER: {
       sqlite3_result_int64(context, sqlite3_مقدار_int64(argv[0]));
       بشکن;
    }
    حالت SQLITE_FLOAT: {
       دوگانه (*x)(دوگانه) = (دوگانه(*)(دوگانه))sqlite3_user_data(context);
       sqlite3_result_double(context, x(sqlite3_مقدار_double(argv[0])));
       بشکن;
    }
    پیشفرض: {
       بشکن;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
ایستا دوگانه xCeil(دوگانه x){ بازگشت ceil(x); }
ایستا دوگانه xFloor(دوگانه x){ بازگشت floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#اگر defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#endif
#اگر defined(HAVE_LOG2) && HAVE_LOG2==0
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
ایستا تهی logFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  دوگانه x, b, ans;
  أكد( argc==1 || argc==2 );
  انتخاب( sqlite3_مقدار_numeric_type(argv[0]) ){
    حالت SQLITE_INTEGER:
    حالت SQLITE_FLOAT:
      x = sqlite3_مقدار_double(argv[0]);
      اگر( x<=0.0 ) بازگشت;
      بشکن;
    پیشفرض:
      بازگشت;
  }
  اگر( argc==2 ){
    انتخاب( sqlite3_مقدار_numeric_type(argv[1]) ){
      حالت SQLITE_INTEGER:
      حالت SQLITE_FLOAT:
        b = log(x);
        اگر( b<=0.0 ) بازگشت;
        x = sqlite3_مقدار_double(argv[1]);
        اگر( x<=0.0 ) بازگشت;
        بشکن;
     پیشفرض:
        بازگشت;
    }
    ans = log(x)/b;
  }وگرنه{
    انتخاب( SQLITE_PTR_TO_INT(sqlite3_user_data(context)) ){
      حالت 1:
        ans = log10(x);
        بشکن;
      حالت 2:
        ans = log2(x);
        بشکن;
      پیشفرض:
        ans = log(x);
        بشکن;
    }
  }
  sqlite3_result_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
ایستا دوگانه degToRad(دوگانه x){ بازگشت x*(M_PI/180.0); }
ایستا دوگانه radToDeg(دوگانه x){ بازگشت x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
ایستا تهی math1Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح type0;
  دوگانه v0, ans;
  دوگانه (*x)(دوگانه);
  أكد( argc==1 );
  type0 = sqlite3_مقدار_numeric_type(argv[0]);
  اگر( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) بازگشت;
  v0 = sqlite3_مقدار_double(argv[0]);
  x = (دوگانه(*)(دوگانه))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
ایستا تهی math2Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح type0, type1;
  دوگانه v0, v1, ans;
  دوگانه (*x)(دوگانه,دوگانه);
  أكد( argc==2 );
  type0 = sqlite3_مقدار_numeric_type(argv[0]);
  اگر( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) بازگشت;
  type1 = sqlite3_مقدار_numeric_type(argv[1]);
  اگر( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) بازگشت;
  v0 = sqlite3_مقدار_double(argv[0]);
  v1 = sqlite3_مقدار_double(argv[1]);
  x = (دوگانه(*)(دوگانه,دوگانه))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
ایستا تهی piFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==0 );
  (تهی)argv;
  sqlite3_result_double(context, M_PI);
}

#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
ایستا تهی signFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح type0;
  دوگانه x;
  UNUSED_PARAMETER(argc);
  أكد( argc==1 );
  type0 = sqlite3_مقدار_numeric_type(argv[0]);
  اگر( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) بازگشت;
  x = sqlite3_مقدار_double(argv[0]);
  sqlite3_result_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#اگر defined(SQLITE_ENABLE_PERCENTILE)
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
تعریفنوع ساختار Percentile Percentile;
ساختار Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  نویسه bSorted;        /* True if a[] is already in sorted order */
  نویسه bKeepSorted;    /* True if advantageous to keep a[] sorted */
  نویسه bPctValid;      /* True if rPct is valid */
  دوگانه rPct;         /* Fraction.  0.0 to 1.0 */
  دوگانه *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
ایستا صحیح percentIsInfinity(دوگانه r){
  sqlite3_uint64 u;
  أكد( اندازه(u)==اندازه(r) );
  memcpy(&u, &r, اندازه(u));
  بازگشت ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
ایستا صحیح percent_Same_مقدار(دوگانه a, دوگانه b){
  a -= b;
  بازگشت a>=-0.001 && a<=0.001;
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
ایستا i64 percent_دودویی_جستجو(Percentile *p, دوگانه y, صحیح bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  وقتی( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    دوگانه x = p->a[iMid];
    اگر( x<y ){
      iFirst = iMid + 1;
    }وگرنه اگر( x>y ){
      iLast = iMid - 1;
    }وگرنه{
      بازگشت iMid;
    }
  }
  اگر( bExact ) بازگشت -1;
  بازگشت iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
ایستا تهی percentError(sqlite3_context *pCtx, ثابت نویسه *zFormat, ...){
  نویسه *zMsg1;
  نویسه *zMsg2;
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
ایستا تهی percentStep(sqlite3_context *pCtx, صحیح argc, sqlite3_مقدار **argv){
  Percentile *p;
  دوگانه rPct;
  صحیح eType;
  دوگانه y;
  أكد( argc==2 || argc==1 );

  اگر( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }وگرنه{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    دوگانه mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_مقدار_numeric_type(argv[1]);
    rPct = sqlite3_مقدار_double(argv[1])/mxFrac;
    اگر( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (دوگانه)mxFrac);
      بازگشت;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, اندازه(*p));
  اگر( p==0 ) بازگشت;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  اگر( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }وگرنه اگر( !percent_Same_مقدار(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    بازگشت;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_مقدار_type(argv[0]);
  اگر( eType==SQLITE_NULL ) بازگشت;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  اگر( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    بازگشت;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_مقدار_double(argv[0]);
  اگر( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    بازگشت;
  }

  /* Allocate and store the Y */
  اگر( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    دوگانه *a = sqlite3_realloc64(p->a, اندازه(دوگانه)*n);
    اگر( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, اندازه(*p));
      sqlite3_result_error_nomem(pCtx);
      بازگشت;
    }
    p->nAlloc = n;
    p->a = a;
  }
  اگر( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }وگرنه اگر( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }وگرنه اگر( p->bKeepSorted ){
    i64 i;
    i = percent_دودویی_جستجو(p, y, 0);
    اگر( i<(صحیح)p->nUsed ){
      memmove(&p->a[i+1], &p->a[i], (p->nUsed-i)*اندازه(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }وگرنه{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#define SWAP_DOUBLE(X,Y)  {دوگانه ttt=(X);(X)=(Y);(Y)=ttt;}

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
ایستا تهی percentSort(
  دوگانه *a,                      /* Array to sort */
  unsigned صحیح n,                 /* Number of elements in array a[] */
  صحیح iReq                        /* Element caller cares about (or -ve) */
){
  صحیح iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  صحیح iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  صحیح i;         /* Loop counter */
  دوگانه rPivot; /* The pivot value */

  أكد( n>=2 );
  انجامبده{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    اگر( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    اگر( n==2 ) بازگشت;
    iGt = n-1;
    i = n/2;
    اگر( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }وگرنه اگر( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    اگر( n==3 ) بازگشت;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    انجامبده{
      اگر( a[i]<rPivot ){
        اگر( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }وگرنه اگر( a[i]>rPivot ){
        انجامبده{
          iGt--;
        }وقتی( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }وگرنه{
        i++;
      }
    }وقتی( i<iGt );

    أكد( iLt>0 && iLt<iGt && (unsigned)iGt<n );
    testcase( iGt>iLt+1 );
    أكد( a[iLt]==rPivot );
    أكد( a[iLt-1]<=rPivot );
    أكد( a[iGt]>=rPivot );
    أكد( a[iLt+1]>=rPivot );

    اگر( iReq>=0 ){
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
      اگر( iReq<iLt ){
        n = iLt;
      }وگرنه{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }وگرنه{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      اگر( iLt>(صحیح)(n/2) ){
        اگر( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }وگرنه{
        اگر( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }وقتی( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
ایستا تهی percentInverse(sqlite3_context *pCtx,صحیح argc,sqlite3_مقدار **argv){
  Percentile *p;
  صحیح eType;
  دوگانه y;
  i64 i;
  أكد( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, اندازه(*p));
  أكد( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_مقدار_type(argv[0]);
  اگر( eType==SQLITE_NULL ) بازگشت;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  اگر( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    بازگشت;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_مقدار_double(argv[0]);
  اگر( percentIsInfinity(y) ){
    بازگشت;
  }
  اگر( p->bSorted==0 ){
    أكد( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_دودویی_جستجو(p, y, 1);
  اگر( i>=0 ){
    p->nUsed--;
    اگر( i<(صحیح)p->nUsed ){
      memmove(&p->a[i], &p->a[i+1], (p->nUsed - i)*اندازه(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
ایستا تهی percentCompute(sqlite3_context *pCtx, صحیح bIsFinal){
  Percentile *p;
  صحیح settings = SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&1; /* Discrete? */
  unsigned i1, i2;
  دوگانه v1, v2;
  دوگانه ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  اگر( p==0 ) بازگشت;
  اگر( p->a==0 ) بازگشت;
  اگر( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (unsigned)ix;
    اگر( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      أكد( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (صحیح)i1 : -1));
      p->bSorted = 1;
    }
    اگر( settings & 1 ){
      vx = p->a[i1];
    }وگرنه{
      i2 = ix==(دوگانه)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  اگر( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, اندازه(*p));
  }وگرنه{
    p->bKeepSorted = 1;
  }
}
ایستا تهی percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
ایستا تهی percent_مقدار(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#endif /* SQLITE_ENABLE_PERCENTILE */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
ایستا تهی filestatFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت نویسه *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  اگر( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    صحیح rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    أكد( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    اگر( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }وگرنه{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      اگر( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      اگر( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        اگر( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }وگرنه{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
ایستا تهی walstatFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت نویسه *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  اگر( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    أكد( pPager!=0 );
    pStr = sqlite3_str_new(db);
    اگر( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }وگرنه{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }وگرنه{
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
ایستا تهی fpdecodeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  FpDecode s;
  دوگانه x;
  صحیح y, z;
  نویسه zBuf[100];
  UNUSED_PARAMETER(argc);
  أكد( argc==3 );
  x = sqlite3_مقدار_double(argv[0]);
  y = sqlite3_مقدار_int(argv[1]);
  z = sqlite3_مقدار_int(argv[2]);
  اگر( z<=0 ) z = 1;
  اگر( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  اگر( s.isSpecial==2 ){
    sqlite3_snprintf(اندازه(zBuf), zBuf, "NaN");
  }وگرنه{
    sqlite3_snprintf(اندازه(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
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
ایستا تهی parseuriFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3_str *pResult;
  ثابت نویسه *zVfs;
  ثابت نویسه *zUri;
  unsigned صحیح flgs;
  صحیح rc;
  sqlite3_vfs *pVfs = 0;
  نویسه *zFile = 0;
  نویسه *zErr = 0;

  اگر( argc<2 ) بازگشت;
  pVfs = sqlite3_vfs_find(0);
  أكد( pVfs );
  zVfs = pVfs->zName;
  zUri = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  اگر( zUri==0 ) بازگشت;
  flgs = (unsigned صحیح)sqlite3_مقدار_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  pResult = sqlite3_str_new(0);
  اگر( !sqlite3_str_errcode(pResult) ){
    صحیح i;
    sqlite3_str_appendf(pResult, "rc=%d", rc);
    sqlite3_str_appendf(pResult, ", flags=0x%x", flgs);
    sqlite3_str_appendf(pResult, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(pResult, ", err=%Q", zErr);
    sqlite3_str_appendf(pResult, ", file=%Q", zFile);
    اگر( zFile ){
      ثابت نویسه *z = zFile;
      z += sqlite3Strlen30(z)+1;
      وقتی( z[0] ){
        sqlite3_str_appendf(pResult, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      برای(i=2; i<argc; i++){
        ثابت نویسه *zArg;
        اگر( sqlite3_مقدار_type(argv[i])==SQLITE_INTEGER ){
          صحیح k = sqlite3_مقدار_int(argv[i]);
          sqlite3_str_appendf(pResult, ", '%d:%q'",k,sqlite3_uri_کلید(zFile, k));
        }وگرنه اگر( (zArg = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]))!=0 ){
          sqlite3_str_appendf(pResult, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }وگرنه{
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
تهی sqlite3RegisterBuiltinFunctions(تهی){
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
  ایستا FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#اگر !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(عبارت_compare,        2, INLINEFUNC_عبارت_compare,        0),
    TEST_FUNC(عبارت_implies_عبارت,   2, INLINEFUNC_عبارت_implies_عبارت,   0),
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
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_مقدار, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_مقدار, 0,
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
    FUNCTION(نویسه,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifdef SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#endif
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
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
    VFUNCTION(last_درج_rowid, 0, 0, 0, last_درج_rowid),
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
    WAGGREGATE(شمار, 0,0,0, شمار_Step,
        شمار_Finalize, شمار_Finalize, شمار_Inverse,
        SQLITE_FUNC_شمار|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(شمار, 1,0,0, شمار_Step,
        شمار_Finalize, شمار_Finalize, شمار_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_مقدار, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_مقدار, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_مقدار, groupConcatInverse, 0),

#ifdef SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#endif /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#وگرنه
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
#اگر SQLITE_HAVE_C99_MATH_FUNCS
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
#اگر SQLITE_HAVE_C99_MATH_FUNCS
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
    INLINE_FUNC(اگر,             -4, INLINEFUNC_iif,      0 ),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_درج_Builtin_Funcs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#اگر 0  /* Enable to print out how the built-in functions are hashed */
  {
    صحیح i;
    FuncDef *p;
    برای(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      برای(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        صحیح n = sqlite3Strlen30(p->zName);
        صحیح h = p->zName[0] + n;
        أكد( p->funcFlags & SQLITE_FUNC_BUILTIN );
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}
