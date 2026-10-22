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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <assert.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#অন্তর্ভাব <math.h>
#endif
#অন্তর্ভাব "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
স্থির CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  প্রতিজ্ঞা( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  প্রতিজ্ঞা( pOp->opcode==OP_CollSeq );
  প্রতিজ্ঞা( pOp->p4type==P4_COLLSEQ );
  প্রত্যাগম pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
স্থির শূন্য sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  প্রতিজ্ঞা( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
স্থির শূন্য minmaxFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক mask;    /* 0 for min() or 0xffffffff for max() */
  পূর্ণাঙ্ক iBest;
  CollSeq *pColl;

  প্রতিজ্ঞা( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  প্রতিজ্ঞা( pColl );
  প্রতিজ্ঞা( mask==-1 || mask==0 );
  iBest = 0;
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ) প্রত্যাগম;
  প্রতি(i=1; i<argc; i++){
    যদি( sqlite3_মান_type(argv[i])==SQLITE_NULL ) প্রত্যাগম;
    যদি( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_মান(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
স্থির শূন্য typeofFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **argv
){
  স্থির ধ্রুব বর্ণ *azType[] = { "integer", "real", "text", "blob", "null" };
  পূর্ণাঙ্ক i = sqlite3_মান_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  প্রতিজ্ঞা( i>=0 && i<ArraySize(azType) );
  প্রতিজ্ঞা( SQLITE_INTEGER==1 );
  প্রতিজ্ঞা( SQLITE_FLOAT==2 );
  প্রতিজ্ঞা( SQLITE_TEXT==3 );
  প্রতিজ্ঞা( SQLITE_BLOB==4 );
  প্রতিজ্ঞা( SQLITE_NULL==5 );
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
স্থির শূন্য subtypeFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_result_int(context, sqlite3_মান_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
স্থির শূন্য lengthFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  বিকল্প( sqlite3_মান_type(argv[0]) ){
    পক্ষ SQLITE_BLOB:
    পক্ষ SQLITE_INTEGER:
    পক্ষ SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_মান_bytes(argv[0]));
      ভঙ্গ;
    }
    পক্ষ SQLITE_TEXT: {
      ধ্রুব unsigned বর্ণ *z = sqlite3_মান_text(argv[0]);
      ধ্রুব unsigned বর্ণ *z0;
      যদি( z==0 ) প্রত্যাগম;
      z0 = z;
      যাবত্( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        যদি( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }অন্যথা যদি( z[0]==0 ){
          ভঙ্গ;
        }অন্যথা{
          z++;
          যাবত্( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_result_int(context, (পূর্ণাঙ্ক)(z-z0));
      ভঙ্গ;
    }
    স্বভাব: {
      sqlite3_result_null(context);
      ভঙ্গ;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
স্থির শূন্য bytelengthFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  বিকল্প( sqlite3_মান_type(argv[0]) ){
    পক্ষ SQLITE_BLOB: {
      sqlite3_result_int(context, sqlite3_মান_bytes(argv[0]));
      ভঙ্গ;
    }
    পক্ষ SQLITE_INTEGER:
    পক্ষ SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_result_int64(context, sqlite3_মান_bytes(argv[0])*m);
      ভঙ্গ;
    }
    পক্ষ SQLITE_TEXT: {
      যদি( sqlite3_মান_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_result_int(context, sqlite3_মান_bytes(argv[0]));
      }অন্যথা{
        sqlite3_result_int(context, sqlite3_মান_bytes16(argv[0]));
      }
      ভঙ্গ;
    }
    স্বভাব: {
      sqlite3_result_null(context);
      ভঙ্গ;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
স্থির শূন্য absFunc(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  বিকল্প( sqlite3_মান_type(argv[0]) ){
    পক্ষ SQLITE_INTEGER: {
      i64 iVal = sqlite3_মান_int64(argv[0]);
      যদি( iVal<0 ){
        যদি( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          প্রত্যাগম;
        }
        iVal = -iVal;
      }
      sqlite3_result_int64(context, iVal);
      ভঙ্গ;
    }
    পক্ষ SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      ভঙ্গ;
    }
    স্বভাব: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      দ্বিগুণ rVal = sqlite3_মান_double(argv[0]);
      যদি( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      ভঙ্গ;
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
স্থির শূন্য instrFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব unsigned বর্ণ *zHaystack;
  ধ্রুব unsigned বর্ণ *zNeedle;
  পূর্ণাঙ্ক nHaystack;
  পূর্ণাঙ্ক nNeedle;
  পূর্ণাঙ্ক typeHaystack, typeNeedle;
  পূর্ণাঙ্ক N = 1;
  পূর্ণাঙ্ক isText;
  unsigned বর্ণ firstChar;
  sqlite3_মান *pC1 = 0;
  sqlite3_মান *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_মান_type(argv[0]);
  typeNeedle = sqlite3_মান_type(argv[1]);
  যদি( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) প্রত্যাগম;
  nHaystack = sqlite3_মান_bytes(argv[0]);
  nNeedle = sqlite3_মান_bytes(argv[1]);
  যদি( nNeedle>0 ){
    যদি( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_মান_blob(argv[0]);
      zNeedle = sqlite3_মান_blob(argv[1]);
      isText = 0;
    }অন্যথা যদি( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_মান_text(argv[0]);
      zNeedle = sqlite3_মান_text(argv[1]);
      isText = 1;
    }অন্যথা{
      pC1 = sqlite3_মান_dup(argv[0]);
      zHaystack = sqlite3_মান_text(pC1);
      যদি( zHaystack==0 ) গচ্ছ endInstrOOM;
      nHaystack = sqlite3_মান_bytes(pC1);
      pC2 = sqlite3_মান_dup(argv[1]);
      zNeedle = sqlite3_মান_text(pC2);
      যদি( zNeedle==0 ) গচ্ছ endInstrOOM;
      nNeedle = sqlite3_মান_bytes(pC2);
      isText = 1;
    }
    যদি( zNeedle==0 || (nHaystack && zHaystack==0) ) গচ্ছ endInstrOOM;
    firstChar = zNeedle[0];
    যাবত্( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      কুরু{
        nHaystack--;
        zHaystack++;
      }যাবত্( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    যদি( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_মান_free(pC1);
  sqlite3_মান_free(pC2);
  প্রত্যাগম;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  গচ্ছ endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
স্থির শূন্য printfFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  PrintfArguments x;
  StrAccum str;
  ধ্রুব বর্ণ *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  যদি( argc>=1 && (zFormat = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]))!=0 ){
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
স্থির SQLITE_INLINE পূর্ণাঙ্ক containsNulOrNonAscii(u64 word){
  ধ্রুব u64 byteOnes = UINT64_C(0x0101010101010101);
  ধ্রুব u64 byte_উপরের_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  প্রত্যাগম ((word | (word - byteOnes)) & byte_উপরের_Bits)!=0;
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
স্থির শূন্য substrFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব unsigned বর্ণ *z;
  ধ্রুব unsigned বর্ণ *z2;
  পূর্ণাঙ্ক len;
  পূর্ণাঙ্ক p0type;
  i64 p1, p2;

  প্রতিজ্ঞা( argc==3 || argc==2 );
  p0type = sqlite3_মান_type(argv[0]);
  p1 = sqlite3_মান_int64(argv[1]);
  যদি( p0type==SQLITE_BLOB ){
    len = sqlite3_মান_bytes(argv[0]);
    z = sqlite3_মান_blob(argv[0]);
    যদি( z==0 ) প্রত্যাগম;
    প্রতিজ্ঞা( len==sqlite3_মান_bytes(argv[0]) );
  }অন্যথা{
    z = sqlite3_মান_text(argv[0]);
    যদি( z==0 ) প্রত্যাগম;
    len = 0;
    যদি( p1<0 ){
      প্রতি(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  যদি( argc==3 ){
    p2 = sqlite3_মান_int64(argv[2]);
    যদি( p2==0 && sqlite3_মান_type(argv[2])==SQLITE_NULL ) প্রত্যাগম;
  }অন্যথা{
    p2 = LARGEST_INT64;
  }
  যদি( p1==0 ){
#ifdef SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#endif
    যদি( sqlite3_মান_type(argv[1])==SQLITE_NULL ) প্রত্যাগম;
  }
  যদি( p1<0 ){
    p1 += len;
    যদি( p1<0 ){
      যদি( p2<0 ){
        p2 = 0;
      }অন্যথা{
        p2 += p1;
      }
      p1 = 0;
    }
  }অন্যথা যদি( p1>0 ){
    p1--;
  }অন্যথা যদি( p2>0 ){
    p2--;
  }
  যদি( p2<0 ){
    যদি( p2<-p1 ){
      p2 = p1;
    }অন্যথা{
      p2 = -p2;
    }
    p1 -= p2;
  }
  প্রতিজ্ঞা( p1>=0 && p2>=0 );
  যদি( p0type!=SQLITE_BLOB ){
    যদি( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ধ্রুব unsigned বর্ণ *zEnd = z + sqlite3_মান_bytes(argv[0]);
      যাবত্( p1>=8 && zEnd-z>=8 ){
        u64 word;
        memcpy(&word, z, পরিমাণ(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        যদি( containsNulOrNonAscii(word) ){
          ভঙ্গ;
        }
        z += 8;
        p1 -= 8;
      }
    }
    প্রতি( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      যদি( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }অন্যথা যদি( z[0]==0 ){
        ভঙ্গ;
      }অন্যথা{
        কুরু{ z++; }যাবত্( (z[0]&0xc0)==0x80 );
      }
    }
    প্রতি(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      যদি( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }অন্যথা যদি( z2[0]==0 ){
        ভঙ্গ;
      }অন্যথা{
        কুরু{ z2++; }যাবত্( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_result_text64(context, (বর্ণ*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }অন্যথা{
    যদি( p1>=len ){
      p1 = p2 = 0;
    }অন্যথা যদি( p2>len-p1 ){
      p2 = len-p1;
      প্রতিজ্ঞা( p2>0 );
    }
    sqlite3_result_blob64(context, (বর্ণ*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
স্থির শূন্য roundFunc(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  i64 n = 0;
  দ্বিগুণ r;
  বর্ণ *zBuf;
  প্রতিজ্ঞা( argc==1 || argc==2 );
  যদি( argc==2 ){
    যদি( SQLITE_NULL==sqlite3_মান_type(argv[1]) ) প্রত্যাগম;
    n = sqlite3_মান_int64(argv[1]);
    যদি( n>30 ) n = 30;
    যদি( n<0 ) n = 0;
  }
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ) প্রত্যাগম;
  r = sqlite3_মান_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  যদি( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }অন্যথা যদি( n==0 ){ 
    r = (দ্বিগুণ)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }অন্যথা{
    zBuf = sqlite3_mprintf("%!.*f",(পূর্ণাঙ্ক)n,r);
    যদি( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      প্রত্যাগম;
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
স্থির শূন্য *contextMalloc(sqlite3_context *context, i64 nByte){
  বর্ণ *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  প্রতিজ্ঞা( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  যদি( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }অন্যথা{
    z = sqlite3Malloc(nByte);
    যদি( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  প্রত্যাগম z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
স্থির শূন্য upperFunc(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  বর্ণ *z1;
  ধ্রুব বর্ণ *z2;
  পূর্ণাঙ্ক i, n;
  UNUSED_PARAMETER(argc);
  z2 = (বর্ণ*)sqlite3_মান_text(argv[0]);
  n = sqlite3_মান_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  প্রতিজ্ঞা( z2==(বর্ণ*)sqlite3_মান_text(argv[0]) );
  যদি( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    যদি( z1 ){
      প্রতি(i=0; i<n; i++){
        z1[i] = (বর্ণ)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
স্থির শূন্য lowerFunc(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  বর্ণ *z1;
  ধ্রুব বর্ণ *z2;
  পূর্ণাঙ্ক i, n;
  UNUSED_PARAMETER(argc);
  z2 = (বর্ণ*)sqlite3_মান_text(argv[0]);
  n = sqlite3_মান_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  প্রতিজ্ঞা( z2==(বর্ণ*)sqlite3_মান_text(argv[0]) );
  যদি( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    যদি( z1 ){
      প্রতি(i=0; i<n; i++){
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
স্থির শূন্য randomFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(পরিমাণ(r), &r);
  যদি( r<0 ){
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
স্থির শূন্য randomBlob(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  sqlite3_int64 n;
  unsigned বর্ণ *p;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_মান_int64(argv[0]);
  যদি( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  যদি( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (বর্ণ*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
স্থির শূন্য last_ঢোকাও_rowid(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_ঢোকাও_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
স্থির শূন্য changes(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
স্থির শূন্য total_changes(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **NotUsed2
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
সংরচনা compareInfo {
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
#যদি defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#অন্যথা
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

স্থির ধ্রুব সংরচনা compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
স্থির ধ্রুব সংরচনা compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
স্থির ধ্রুব সংরচনা compareInfo likeInfoAlt = { '%', '_',   0, 0 };

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
স্থির পূর্ণাঙ্ক patternCompare(
  ধ্রুব u8 *zPattern,              /* The glob pattern */
  ধ্রুব u8 *zString,               /* The string to compare against the glob */
  ধ্রুব সংরচনা compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ধ্রুব u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  যাবত্( (c = Utf8Read(zPattern))!=0 ){
    যদি( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      যাবত্( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        যদি( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          প্রত্যাগম SQLITE_NOWILDCARDMATCH;
        }
      }
      যদি( c==0 ){
        প্রত্যাগম SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }অন্যথা যদি( c==matchOther ){
        যদি( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          যদি( c==0 ) প্রত্যাগম SQLITE_NOWILDCARDMATCH;
        }অন্যথা{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          প্রতিজ্ঞা( matchOther<0x80 );  /* '[' is a single-byte character */
          যাবত্( *zString ){
            পূর্ণাঙ্ক bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            যদি( bMatch!=SQLITE_NOMATCH ) প্রত্যাগম bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          প্রত্যাগম SQLITE_NOWILDCARDMATCH;
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
      যদি( c<0x80 ){
        বর্ণ zStop[3];
        পূর্ণাঙ্ক bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        যদি( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        যদি( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }অন্যথা{
          zStop[0] = c;
          zStop[1] = 0;
        }
        যাবত্(1){
          zString += strcspn((ধ্রুব বর্ণ*)zString, zStop);
          যদি( zString[0]==0 ) ভঙ্গ;
          zString++;
          যদি( nx && zString[0]!=nx && zString[0]!=ny ) অনুবর্ত;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          যদি( bMatch!=SQLITE_NOMATCH ) প্রত্যাগম bMatch;
        }
      }অন্যথা{
        পূর্ণাঙ্ক bMatch;
        যাবত্( (c2 = Utf8Read(zString))!=0 ){
          যদি( c2!=c ) অনুবর্ত;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          যদি( bMatch!=SQLITE_NOMATCH ) প্রত্যাগম bMatch;
        }
      }
      প্রত্যাগম SQLITE_NOWILDCARDMATCH;
    }
    যদি( c==matchOther ){
      যদি( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        যদি( c==0 ) প্রত্যাগম SQLITE_NOMATCH;
        zEscaped = zPattern;
      }অন্যথা{
        u32 prior_c = 0;
        পূর্ণাঙ্ক seen = 0;
        পূর্ণাঙ্ক invert = 0;
        c = sqlite3Utf8Read(&zString);
        যদি( c==0 ) প্রত্যাগম SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        যদি( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        যদি( c2==']' ){
          যদি( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        যাবত্( c2 && c2!=']' ){
          যদি( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            যদি( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }অন্যথা{
            যদি( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        যদি( c2==0 || (seen ^ invert)==0 ){
          প্রত্যাগম SQLITE_NOMATCH;
        }
        অনুবর্ত;
      }
    }
    c2 = Utf8Read(zString);
    যদি( c==c2 ) অনুবর্ত;
    যদি( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      অনুবর্ত;
    }
    যদি( c==matchOne && zPattern!=zEscaped && c2!=0 ) অনুবর্ত;
    প্রত্যাগম SQLITE_NOMATCH;
  }
  প্রত্যাগম *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
পূর্ণাঙ্ক sqlite3_strglob(ধ্রুব বর্ণ *zGlobPattern, ধ্রুব বর্ণ *zString){
  যদি( zString==0 ){
    প্রত্যাগম zGlobPattern!=0;
  }অন্যথা যদি( zGlobPattern==0 ){
    প্রত্যাগম 1;
  }অন্যথা {
    প্রত্যাগম patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
পূর্ণাঙ্ক sqlite3_strlike(ধ্রুব বর্ণ *zPattern, ধ্রুব বর্ণ *zStr, unsigned পূর্ণাঙ্ক esc){
  যদি( zStr==0 ){
    প্রত্যাগম zPattern!=0;
  }অন্যথা যদি( zPattern==0 ){
    প্রত্যাগম 1;
  }অন্যথা{
    প্রত্যাগম patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
পূর্ণাঙ্ক sqlite3_like_গুনতি = 0;
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
স্থির শূন্য likeFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব unsigned বর্ণ *zA, *zB;
  u32 escape;
  পূর্ণাঙ্ক nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  সংরচনা compareInfo *pInfo = sqlite3_user_data(context);
  সংরচনা compareInfo backupInfo;

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  যদি( sqlite3_মান_type(argv[0])==SQLITE_BLOB
   || sqlite3_মান_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_গুনতি++;
#endif
    sqlite3_result_int(context, 0);
    প্রত্যাগম;
  }
#endif

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_মান_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  যদি( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    প্রত্যাগম;
  }
  যদি( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ধ্রুব unsigned বর্ণ *zEsc = sqlite3_মান_text(argv[2]);
    যদি( zEsc==0 ) প্রত্যাগম;
    যদি( sqlite3Utf8CharLen((বর্ণ*)zEsc, -1)!=1 ){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      প্রত্যাগম;
    }
    escape = sqlite3Utf8Read(&zEsc);
    যদি( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      memcpy(&backupInfo, pInfo, পরিমাণ(backupInfo));
      pInfo = &backupInfo;
      যদি( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      যদি( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }অন্যথা{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_মান_text(argv[0]);
  zA = sqlite3_মান_text(argv[1]);
  যদি( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_গুনতি++;
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
স্থির শূন্য nullifFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  যদি( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_মান(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
স্থির শূন্য versionFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **NotUsed2
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
স্থির শূন্য sourceidFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **NotUsed2
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
স্থির শূন্য errlogFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_মান_int(argv[0]), "%s", sqlite3_মান_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
স্থির শূন্য compileoptionusedFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব বর্ণ *zOptName;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  যদি( (zOptName = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]))!=0 ){
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
স্থির শূন্য compileoptiongetFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  i64 n;
  ধ্রুব বর্ণ *zRes;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_মান_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((পূর্ণাঙ্ক)n) : 0;
  sqlite3_result_text(context, zRes, -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
স্থির ধ্রুব বর্ণ hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
শূন্য sqlite3_Quote_মান(StrAccum *pStr, sqlite3_মান *p_মান, পূর্ণাঙ্ক bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  প্রতিজ্ঞা( pStr!=0 && pStr->nChar==0 );

  বিকল্প( sqlite3_মান_type(p_মান) ){
    পক্ষ SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_মান_double(p_মান));
      ভঙ্গ;
    }
    পক্ষ SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_মান_int64(p_মান));
      ভঙ্গ;
    }
    পক্ষ SQLITE_BLOB: {
      বর্ণ ধ্রুব *zBlob = sqlite3_মান_blob(p_মান);
      i64 nBlob = sqlite3_মান_bytes(p_মান);
      প্রতিজ্ঞা( zBlob==sqlite3_মান_blob(p_মান) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      যদি( pStr->accError==0 ){
        বর্ণ *zText = pStr->zText;
        পূর্ণাঙ্ক i;
        প্রতি(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      ভঙ্গ;
    }
    পক্ষ SQLITE_TEXT: {
      ধ্রুব unsigned বর্ণ *zArg = sqlite3_মান_text(p_মান);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      ভঙ্গ;
    }
    স্বভাব: {
      প্রতিজ্ঞা( sqlite3_মান_type(p_মান)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      ভঙ্গ;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
স্থির পূর্ণাঙ্ক isNHex(ধ্রুব বর্ণ *z, পূর্ণাঙ্ক N, u32 *pVal){
  পূর্ণাঙ্ক i;
  u32 v = 0;
  প্রতি(i=0; i<N; i++){
    যদি( !sqlite3Isxdigit(z[i]) ) প্রত্যাগম 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  প্রত্যাগম 1;
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
স্থির শূন্য unistrFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  বর্ণ *zOut;
  ধ্রুব বর্ণ *zIn;
  পূর্ণাঙ্ক nIn;
  পূর্ণাঙ্ক i, j, n;
  u32 v;

  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]);
  যদি( zIn==0 ) প্রত্যাগম;
  nIn = sqlite3_মান_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  যদি( zOut==0 ){
    sqlite3_result_error_nomem(context);
    প্রত্যাগম;
  }
  i = j = 0;
  যাবত্( i<nIn ){
    ধ্রুব বর্ণ *z = strchr(&zIn[i],'\\');
    যদি( z==0 ){
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      ভঙ্গ;
    }
    n = z - &zIn[i];
    যদি( n>0 ){
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    যদি( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }অন্যথা যদি( sqlite3Isxdigit(zIn[i+1]) ){
      যদি( !isNHex(&zIn[i+1], 4, &v) ) গচ্ছ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }অন্যথা যদি( zIn[i+1]=='+' ){
      যদি( !isNHex(&zIn[i+2], 6, &v) ) গচ্ছ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }অন্যথা যদি( zIn[i+1]=='u' ){
      যদি( !isNHex(&zIn[i+2], 4, &v) ) গচ্ছ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }অন্যথা যদি( zIn[i+1]=='U' ){
      যদি( !isNHex(&zIn[i+2], 8, &v) ) গচ্ছ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }অন্যথা{
      গচ্ছ unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  প্রত্যাগম;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  প্রত্যাগম;
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
স্থির শূন্য quoteFunc(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  sqlite3_Quote_মান(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_data(context)));
  sqlite3_result_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
স্থির শূন্য unicodeFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব unsigned বর্ণ *z = sqlite3_মান_text(argv[0]);
  (শূন্য)argc;
  যদি( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
স্থির শূন্য charFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  unsigned বর্ণ *z, *zOut;
  পূর্ণাঙ্ক i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  যদি( z==0 ){
    sqlite3_result_error_nomem(context);
    প্রত্যাগম;
  }
  প্রতি(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_মান_int64(argv[i]);
    যদি( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    যদি( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }অন্যথা যদি( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }অন্যথা যদি( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }অন্যথা{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_result_text64(context, (বর্ণ*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
স্থির শূন্য hexFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক i, n;
  ধ্রুব unsigned বর্ণ *pBlob;
  বর্ণ *zHex, *z;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_মান_blob(argv[0]);
  n = sqlite3_মান_bytes(argv[0]);
  প্রতিজ্ঞা( pBlob==sqlite3_মান_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  যদি( zHex ){
    প্রতি(i=0; i<n; i++, pBlob++){
      unsigned বর্ণ c = *pBlob;
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
স্থির পূর্ণাঙ্ক strContainsChar(ধ্রুব u8 *zStr, পূর্ণাঙ্ক nStr, u32 ch){
  ধ্রুব u8 *zEnd = &zStr[nStr];
  ধ্রুব u8 *z = zStr;
  যাবত্( z<zEnd ){
    u32 tst = Utf8Read(z);
    যদি( tst==ch ) প্রত্যাগম 1;
  }
  প্রত্যাগম 0;
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
স্থির শূন্য unhexFunc(
  sqlite3_context *pCtx,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব u8 *zPass = (ধ্রুব u8*)"";
  পূর্ণাঙ্ক nPass = 0;
  ধ্রুব u8 *zHex = sqlite3_মান_text(argv[0]);
  পূর্ণাঙ্ক nHex = sqlite3_মান_bytes(argv[0]);
#ifdef SQLITE_DEBUG
  ধ্রুব u8 *zEnd = zHex ? &zHex[nHex] : 0;
#endif
  u8 *pBlob = 0;
  u8 *p = 0;

  প্রতিজ্ঞা( argc==1 || argc==2 );
  যদি( argc==2 ){
    zPass = sqlite3_মান_text(argv[1]);
    nPass = sqlite3_মান_bytes(argv[1]);
  }
  যদি( !zHex || !zPass ) প্রত্যাগম;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  যদি( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    যাবত্( (c = *zHex)!=0x00 ){
      যাবত্( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        প্রতিজ্ঞা( zHex<=zEnd );
        যদি( !strContainsChar(zPass, nPass, ch) ) গচ্ছ unhex_null;
        c = *zHex;
        যদি( c==0x00 ) গচ্ছ unhex_done;
      }
      zHex++;
      প্রতিজ্ঞা( *zEnd==0x00 );
      প্রতিজ্ঞা( zHex<=zEnd );
      d = *(zHex++);
      যদি( !sqlite3Isxdigit(d) ) গচ্ছ unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  প্রত্যাগম;

 unhex_null:
  sqlite3_free(pBlob);
  প্রত্যাগম;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
স্থির শূন্য zeroblobFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  i64 n;
  পূর্ণাঙ্ক rc;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_মান_int64(argv[0]);
  যদি( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  যদি( rc ){
    sqlite3_result_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
স্থির শূন্য replaceFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব unsigned বর্ণ *zStr;        /* The input string A */
  ধ্রুব unsigned বর্ণ *zPattern;    /* The pattern string B */
  ধ্রুব unsigned বর্ণ *zRep;        /* The replacement string C */
  unsigned বর্ণ *zOut;              /* The output */
  পূর্ণাঙ্ক nStr;                /* Size of zStr */
  পূর্ণাঙ্ক nPattern;            /* Size of zPattern */
  পূর্ণাঙ্ক nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  পূর্ণাঙ্ক loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  unsigned cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  প্রতিজ্ঞা( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_মান_text(argv[0]);
  যদি( zStr==0 ) প্রত্যাগম;
  nStr = sqlite3_মান_bytes(argv[0]);
  প্রতিজ্ঞা( zStr==sqlite3_মান_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_মান_text(argv[1]);
  যদি( zPattern==0 ){
    প্রতিজ্ঞা( sqlite3_মান_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    প্রত্যাগম;
  }
  যদি( zPattern[0]==0 ){
    প্রতিজ্ঞা( sqlite3_মান_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_text(context, (ধ্রুব বর্ণ*)zStr, nStr, SQLITE_TRANSIENT);
    প্রত্যাগম;
  }
  nPattern = sqlite3_মান_bytes(argv[1]);
  প্রতিজ্ঞা( zPattern==sqlite3_মান_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_মান_text(argv[2]);
  যদি( zRep==0 ) প্রত্যাগম;
  nRep = sqlite3_মান_bytes(argv[2]);
  প্রতিজ্ঞা( zRep==sqlite3_মান_text(argv[2]) );
  nOut = nStr + 1;
  প্রতিজ্ঞা( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, nOut);
  যদি( zOut==0 ){
    প্রত্যাগম;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  প্রতি(i=j=0; i<=loopLimit; i++){
    যদি( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }অন্যথা{
      যদি( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
        যদি( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          প্রত্যাগম;
        }
        cntExpand++;
        যদি( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (পূর্ণাঙ্ক)nOut + (nOut - nStr - 1));
          যদি( zOut==0 ){
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            প্রত্যাগম;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  প্রতিজ্ঞা( j+nStr-i+1<=nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  প্রতিজ্ঞা( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (বর্ণ*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
স্থির শূন্য trimFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব unsigned বর্ণ *zIn;         /* Input string */
  ধ্রুব unsigned বর্ণ *zCharSet;    /* Set of characters to trim */
  unsigned পূর্ণাঙ্ক nIn;                 /* Number of bytes in input */
  পূর্ণাঙ্ক flags;                        /* 1: trimleft  2: trimright  3: trim */
  পূর্ণাঙ্ক i;                            /* Loop counter */
  unsigned পূর্ণাঙ্ক *aLen = 0;           /* Length of each character in zCharSet */
  unsigned বর্ণ **azChar = 0;       /* Individual characters in zCharSet */
  পূর্ণাঙ্ক nChar;                        /* Number of characters in zCharSet */

  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ){
    প্রত্যাগম;
  }
  zIn = sqlite3_মান_text(argv[0]);
  যদি( zIn==0 ) প্রত্যাগম;
  nIn = (unsigned)sqlite3_মান_bytes(argv[0]);
  প্রতিজ্ঞা( zIn==sqlite3_মান_text(argv[0]) );
  যদি( argc==1 ){
    স্থির ধ্রুব unsigned lenOne[] = { 1 };
    স্থির unsigned বর্ণ * ধ্রুব azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (unsigned*)lenOne;
    azChar = (unsigned বর্ণ **)azOne;
    zCharSet = 0;
  }অন্যথা যদি( (zCharSet = sqlite3_মান_text(argv[1]))==0 ){
    প্রত্যাগম;
  }অন্যথা{
    ধ্রুব unsigned বর্ণ *z;
    প্রতি(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    যদি( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(পরিমাণ(বর্ণ*)+পরিমাণ(unsigned)));
      যদি( azChar==0 ){
        প্রত্যাগম;
      }
      aLen = (unsigned*)&azChar[nChar];
      প্রতি(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned বর্ণ *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  যদি( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    যদি( flags & 1 ){
      যাবত্( nIn>0 ){
        unsigned পূর্ণাঙ্ক len = 0;
        প্রতি(i=0; i<nChar; i++){
          len = aLen[i];
          যদি( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) ভঙ্গ;
        }
        যদি( i>=nChar ) ভঙ্গ;
        zIn += len;
        nIn -= len;
      }
    }
    যদি( flags & 2 ){
      যাবত্( nIn>0 ){
        unsigned পূর্ণাঙ্ক len = 0;
        প্রতি(i=0; i<nChar; i++){
          len = aLen[i];
          যদি( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) ভঙ্গ;
        }
        যদি( i>=nChar ) ভঙ্গ;
        nIn -= len;
      }
    }
    যদি( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (বর্ণ*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
স্থির শূন্য concatFuncCore(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv,
  পূর্ণাঙ্ক nSep,
  ধ্রুব বর্ণ *zSep
){
  i64 j, n = 0;
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক bNotNull = 0;   /* True after at least NOT NULL argument seen */
  বর্ণ *z;
  প্রতি(i=0; i<argc; i++){
    n += sqlite3_মান_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  যদি( z==0 ){
    sqlite3_result_error_nomem(context);
    প্রত্যাগম;
  }
  j = 0;
  প্রতি(i=0; i<argc; i++){
    যদি( sqlite3_মান_type(argv[i])!=SQLITE_NULL ){
      পূর্ণাঙ্ক k = sqlite3_মান_bytes(argv[i]);
      ধ্রুব বর্ণ *v = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[i]);
      যদি( v!=0 ){
        যদি( bNotNull && nSep>0 ){
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
  প্রতিজ্ঞা( j<=n );
  sqlite3_result_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
স্থির শূন্য concatFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
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
স্থির শূন্য concatwsFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক nSep = sqlite3_মান_bytes(argv[0]);
  ধ্রুব বর্ণ *zSep = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]);
  যদি( zSep==0 ) প্রত্যাগম;
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
স্থির শূন্য unknownFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  /* no-op */
  (শূন্য)context;
  (শূন্য)argc;
  (শূন্য)argv;
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
স্থির শূন্য soundexFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  বর্ণ zResult[8];
  ধ্রুব u8 *zIn;
  পূর্ণাঙ্ক i, j;
  স্থির ধ্রুব unsigned বর্ণ iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  প্রতিজ্ঞা( argc==1 );
  zIn = (u8*)sqlite3_মান_text(argv[0]);
  যদি( zIn==0 ) zIn = (u8*)"";
  প্রতি(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  যদি( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    প্রতি(j=1; j<4 && zIn[i]; i++){
      পূর্ণাঙ্ক code = iCode[zIn[i]&0x7f];
      যদি( code>0 ){
        যদি( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }অন্যথা{
        prevcode = 0;
      }
    }
    যাবত্( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }অন্যথা{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
স্থির শূন্য oomFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#endif /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
স্থির শূন্য loadExt(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  ধ্রুব বর্ণ *zFile = (ধ্রুব বর্ণ *)sqlite3_মান_text(argv[0]);
  ধ্রুব বর্ণ *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  বর্ণ *z_Err_বার্তা = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  যদি( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    প্রত্যাগম;
  }

  যদি( argc==2 ){
    zProc = (ধ্রুব বর্ণ *)sqlite3_মান_text(argv[1]);
  }অন্যথা{
    zProc = 0;
  }
  যদি( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_বার্তা) ){
    sqlite3_result_error(context, z_Err_বার্তা, -1);
    sqlite3_free(z_Err_বার্তা);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
প্রকারনির্দেশ সংরচনা SumCtx SumCtx;
সংরচনা SumCtx {
  দ্বিগুণ rSum;      /* Running sum as as a double */
  দ্বিগুণ rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
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
স্থির শূন্য kahanBabuskaNeumaierStep(
  চঞ্চল SumCtx *pSum,
  চঞ্চল দ্বিগুণ r
){
  চঞ্চল দ্বিগুণ s = pSum->rSum;
  চঞ্চল দ্বিগুণ t = s + r;
  যদি( fabs(s) > fabs(r) ){
    pSum->rErr += (s - t) + r;
  }অন্যথা{
    pSum->rErr += (r - t) + s;
  }
  pSum->rSum = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
স্থির শূন্য kahanBabuskaNeumaierStepInt64(চঞ্চল SumCtx *pSum, i64 iVal){
  যদি( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  }অন্যথা{
    kahanBabuskaNeumaierStep(pSum, (দ্বিগুণ)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
স্থির শূন্য kahanBabuskaNeumaierInit(
  চঞ্চল SumCtx *p,
  i64 iVal
){
  যদি( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->rSum = (দ্বিগুণ)(iVal - iSm);
    p->rErr = (দ্বিগুণ)iSm;
  }অন্যথা{
    p->rSum = (দ্বিগুণ)iVal;
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
স্থির শূন্য sumStep(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  SumCtx *p;
  পূর্ণাঙ্ক type;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, পরিমাণ(*p));
  type = sqlite3_মান_numeric_type(argv[0]);
  যদি( p && type!=SQLITE_NULL ){
    p->cnt++;
    যদি( p->approx==0 ){
      যদি( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_মান_double(argv[0]));
      }অন্যথা{
        i64 x = p->iSum;
        যদি( sqlite3AddInt64(&x, sqlite3_মান_int64(argv[0]))==0 ){
          p->iSum = x;
        }অন্যথা{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_মান_int64(argv[0]));
        }
      }
    }অন্যথা{
      যদি( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_মান_int64(argv[0]));
      }অন্যথা{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_মান_double(argv[0]));
      }
    }
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
স্থির শূন্য sumInverse(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান**argv){
  SumCtx *p;
  পূর্ণাঙ্ক type;
  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, পরিমাণ(*p));
  type = sqlite3_মান_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  যদি( ALWAYS(p) && type!=SQLITE_NULL ){
    প্রতিজ্ঞা( p->cnt>0 );
    p->cnt--;
    যদি( !p->approx ){
      i64 x = p->iSum;
      যদি( sqlite3SubInt64(&x, sqlite3_মান_int64(argv[0]))==0 ){
        p->iSum = x;
        প্রত্যাগম;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    যদি( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_মান_int64(argv[0]);
      যদি( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }অন্যথা{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }অন্যথা{
      kahanBabuskaNeumaierStep(p, -sqlite3_মান_double(argv[0]));
    }
  }
}
#অন্যথা
# define sumInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
স্থির শূন্য sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  যদি( p && p->cnt>0 ){
    যদি( p->approx ){
      যদি( p->ovrfl ){
        sqlite3_result_error(context,"integer overflow",-1);
      }অন্যথা যদি( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_result_double(context, p->rSum+p->rErr);
      }অন্যথা{
        sqlite3_result_double(context, p->rSum);
      }
    }অন্যথা{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
স্থির শূন্য avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  যদি( p && p->cnt>0 ){
    দ্বিগুণ r;
    যদি( p->approx ){
      r = p->rSum;
      যদি( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }অন্যথা{
      r = (দ্বিগুণ)(p->iSum);
    }
    sqlite3_result_double(context, r/(দ্বিগুণ)p->cnt);
  }
}
স্থির শূন্য totalFinalize(sqlite3_context *context){
  SumCtx *p;
  দ্বিগুণ r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  যদি( p ){
    যদি( p->approx ){
      r = p->rSum;
      যদি( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }অন্যথা{
      r = (দ্বিগুণ)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
প্রকারনির্দেশ সংরচনা গুনতি_Ctx গুনতি_Ctx;
সংরচনা গুনতি_Ctx {
  i64 n;
#ifdef SQLITE_DEBUG
  পূর্ণাঙ্ক bInverse;                   /* True if xInverse() ever called */
#endif
};

/*
** Routines to implement the count() aggregate function.
*/
স্থির শূন্য গুনতি_Step(sqlite3_context *context, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  গুনতি_Ctx *p;
  p = sqlite3_aggregate_context(context, পরিমাণ(*p));
  যদি( (argc==0 || SQLITE_NULL!=sqlite3_মান_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  প্রতিজ্ঞা( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_গুনতি(context) );
#endif
}  
স্থির শূন্য গুনতি_Finalize(sqlite3_context *context){
  গুনতি_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}
#ifndef SQLITE_OMIT_WINDOWFUNC
স্থির শূন্য গুনতি_Inverse(sqlite3_context *ctx, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  গুনতি_Ctx *p;
  p = sqlite3_aggregate_context(ctx, পরিমাণ(*p));
  /* p is always non-NULL since countStep() will have been called first */
  যদি( (argc==0 || SQLITE_NULL!=sqlite3_মান_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ifdef SQLITE_DEBUG
    p->bInverse = 1;
#endif
  }
}  
#অন্যথা
# define গুনতি_Inverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
স্থির শূন্য minmaxStep(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, পরিমাণ(*pBest));
  যদি( !pBest ) প্রত্যাগম;

  যদি( sqlite3_মান_type(pArg)==SQLITE_NULL ){
    যদি( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }অন্যথা যদি( pBest->flags ){
    পূর্ণাঙ্ক max;
    পূর্ণাঙ্ক cmp;
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
    যদি( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }অন্যথা{
      sqlite3SkipAccumulatorLoad(context);
    }
  }অন্যথা{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
স্থির শূন্য min_Max_মান_Finalize(sqlite3_context *context, পূর্ণাঙ্ক b_মান){
  sqlite3_মান *pRes;
  pRes = (sqlite3_মান *)sqlite3_aggregate_context(context, 0);
  যদি( pRes ){
    যদি( pRes->flags ){
      sqlite3_result_মান(context, pRes);
    }
    যদি( b_মান==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
স্থির শূন্য min_Max_মান(sqlite3_context *context){
  min_Max_মান_Finalize(context, 1);
}
#অন্যথা
# define min_Max_মান 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
স্থির শূন্য minMaxFinalize(sqlite3_context *context){
  min_Max_মান_Finalize(context, 0);
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
প্রকারনির্দেশ সংরচনা {
  StrAccum str;          /* The accumulated concatenation */
#ifndef SQLITE_OMIT_WINDOWFUNC
  পূর্ণাঙ্ক nAccum;            /* Number of strings presently concatenated */
  পূর্ণাঙ্ক nFirstSepLength;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  পূর্ণাঙ্ক *pnSepLengths;
#endif
} GroupConcatCtx;

স্থির শূন্য groupConcatStep(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  ধ্রুব বর্ণ *zVal;
  GroupConcatCtx *pGCC;
  ধ্রুব বর্ণ *zSep;
  পূর্ণাঙ্ক nVal, nSep;
  প্রতিজ্ঞা( argc==1 || argc==2 );
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ) প্রত্যাগম;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, পরিমাণ(*pGCC));
  যদি( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    পূর্ণাঙ্ক firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    যদি( argc==1 ){
      যদি( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      অন্যথা{
        pGCC->nFirstSepLength = 1;
      }
#endif
    }অন্যথা যদি( !firstTerm ){
      zSep = (বর্ণ*)sqlite3_মান_text(argv[1]);
      nSep = sqlite3_মান_bytes(argv[1]);
      যদি( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      অন্যথা{
        nSep = 0;
      }
      যদি( nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0 ){
        পূর্ণাঙ্ক *pnsl = pGCC->pnSepLengths;
        যদি( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (পূর্ণাঙ্ক*)sqlite3_malloc64((pGCC->nAccum+1) * পরিমাণ(পূর্ণাঙ্ক));
          যদি( pnsl!=0 ){
            পূর্ণাঙ্ক i = 0, nA = pGCC->nAccum-1;
            যাবত্( i<nA ) pnsl[i++] = pGCC->nFirstSepLength;
          }
        }অন্যথা{
          pnsl = (পূর্ণাঙ্ক*)sqlite3_realloc64(pnsl, pGCC->nAccum * পরিমাণ(পূর্ণাঙ্ক));
        }
        যদি( pnsl!=0 ){
          যদি( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }অন্যথা{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#endif
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    অন্যথা{
      pGCC->nFirstSepLength = sqlite3_মান_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#endif
    zVal = (বর্ণ*)sqlite3_মান_text(argv[0]);
    nVal = sqlite3_মান_bytes(argv[0]);
    যদি( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ifndef SQLITE_OMIT_WINDOWFUNC
স্থির শূন্য groupConcatInverse(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  GroupConcatCtx *pGCC;
  প্রতিজ্ঞা( argc==1 || argc==2 );
  (শূন্য)argc;  /* Suppress unused parameter warning */
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ) প্রত্যাগম;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, পরিমাণ(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  যদি( ALWAYS(pGCC) ){
    পূর্ণাঙ্ক nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (শূন্য)sqlite3_মান_text(argv[0]);
    nVS = sqlite3_মান_bytes(argv[0]);
    pGCC->nAccum -= 1;
    যদি( pGCC->pnSepLengths!=0 ){
      প্রতিজ্ঞা(pGCC->nAccum >= 0);
      যদি( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*পরিমাণ(পূর্ণাঙ্ক));
      }
    }অন্যথা{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->nFirstSepLength;
    }
    যদি( nVS>=(পূর্ণাঙ্ক)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }অন্যথা{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    যদি( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#অন্যথা
# define groupConcatInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
স্থির শূন্য groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  যদি( pGCC ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_XFER);
#ifndef SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#endif
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
স্থির শূন্য group_Concat_মান(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  যদি( pGCC && pGCC->nAccum>0 ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#অন্যথা
# define group_Concat_মান 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
শূন্য sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  পূর্ণাঙ্ক rc = sqlite3_overload_function(db, "MATCH", 2);
  প্রতিজ্ঞা( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  যদি( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
শূন্য sqlite3RegisterLikeFunctions(sqlite3 *db, পূর্ণাঙ্ক caseSensitive){
  FuncDef *pDef;
  সংরচনা compareInfo *pInfo;
  পূর্ণাঙ্ক flags;
  পূর্ণাঙ্ক nArg;
  যদি( caseSensitive ){
    pInfo = (সংরচনা compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }অন্যথা{
    pInfo = (সংরচনা compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  প্রতি(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    প্রতিজ্ঞা( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
পূর্ণাঙ্ক sqlite3IsLikeFunction(sqlite3 *db, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক *pIsNocase, বর্ণ *aWc){
  FuncDef *pDef;
  পূর্ণাঙ্ক n_রাশিমালা;
  প্রতিজ্ঞা( p_রাশিমালা!=0 );
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_FUNCTION );
  প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
  যদি( !p_রাশিমালা->x.pList ){
    প্রত্যাগম 0;
  }
  n_রাশিমালা = p_রাশিমালা->x.pList->n_রাশিমালা;
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
  pDef = sqlite3FindFunction(db, p_রাশিমালা->u.zToken, n_রাশিমালা, SQLITE_UTF8, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  যদি( pDef==0 ) প্রত্যাগম 0;
#endif
  যদি( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    প্রত্যাগম 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  প্রতিজ্ঞা( (বর্ণ*)&likeInfoAlt == (বর্ণ*)&likeInfoAlt.matchAll );
  প্রতিজ্ঞা( &((বর্ণ*)&likeInfoAlt)[1] == (বর্ণ*)&likeInfoAlt.matchOne );
  প্রতিজ্ঞা( &((বর্ণ*)&likeInfoAlt)[2] == (বর্ণ*)&likeInfoAlt.matchSet );

  যদি( n_রাশিমালা<3 ){
    aWc[3] = 0;
  }অন্যথা{
    রাশিমালা *pEscape = p_রাশিমালা->x.pList->a[2].p_রাশিমালা;
    বর্ণ *zEscape;
    যদি( pEscape->op!=TK_STRING ) প্রত্যাগম 0;
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(pEscape, EP_Int_মান) );
    zEscape = pEscape->u.zToken;
    যদি( zEscape[0]==0 || zEscape[1]!=0 ) প্রত্যাগম 0;
    যদি( zEscape[0]==aWc[0] ) প্রত্যাগম 0;
    যদি( zEscape[0]==aWc[1] ) প্রত্যাগম 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  প্রত্যাগম 1;
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
স্থির শূন্য ceilingFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  প্রতিজ্ঞা( argc==1 );
  বিকল্প( sqlite3_মান_numeric_type(argv[0]) ){
    পক্ষ SQLITE_INTEGER: {
       sqlite3_result_int64(context, sqlite3_মান_int64(argv[0]));
       ভঙ্গ;
    }
    পক্ষ SQLITE_FLOAT: {
       দ্বিগুণ (*x)(দ্বিগুণ) = (দ্বিগুণ(*)(দ্বিগুণ))sqlite3_user_data(context);
       sqlite3_result_double(context, x(sqlite3_মান_double(argv[0])));
       ভঙ্গ;
    }
    স্বভাব: {
       ভঙ্গ;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
স্থির দ্বিগুণ xCeil(দ্বিগুণ x){ প্রত্যাগম ceil(x); }
স্থির দ্বিগুণ xFloor(দ্বিগুণ x){ প্রত্যাগম floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#যদি defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#endif
#যদি defined(HAVE_LOG2) && HAVE_LOG2==0
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
স্থির শূন্য logFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  দ্বিগুণ x, b, ans;
  প্রতিজ্ঞা( argc==1 || argc==2 );
  বিকল্প( sqlite3_মান_numeric_type(argv[0]) ){
    পক্ষ SQLITE_INTEGER:
    পক্ষ SQLITE_FLOAT:
      x = sqlite3_মান_double(argv[0]);
      যদি( x<=0.0 ) প্রত্যাগম;
      ভঙ্গ;
    স্বভাব:
      প্রত্যাগম;
  }
  যদি( argc==2 ){
    বিকল্প( sqlite3_মান_numeric_type(argv[1]) ){
      পক্ষ SQLITE_INTEGER:
      পক্ষ SQLITE_FLOAT:
        b = log(x);
        যদি( b<=0.0 ) প্রত্যাগম;
        x = sqlite3_মান_double(argv[1]);
        যদি( x<=0.0 ) প্রত্যাগম;
        ভঙ্গ;
     স্বভাব:
        প্রত্যাগম;
    }
    ans = log(x)/b;
  }অন্যথা{
    বিকল্প( SQLITE_PTR_TO_INT(sqlite3_user_data(context)) ){
      পক্ষ 1:
        ans = log10(x);
        ভঙ্গ;
      পক্ষ 2:
        ans = log2(x);
        ভঙ্গ;
      স্বভাব:
        ans = log(x);
        ভঙ্গ;
    }
  }
  sqlite3_result_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
স্থির দ্বিগুণ degToRad(দ্বিগুণ x){ প্রত্যাগম x*(M_PI/180.0); }
স্থির দ্বিগুণ radToDeg(দ্বিগুণ x){ প্রত্যাগম x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
স্থির শূন্য math1Func(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক type0;
  দ্বিগুণ v0, ans;
  দ্বিগুণ (*x)(দ্বিগুণ);
  প্রতিজ্ঞা( argc==1 );
  type0 = sqlite3_মান_numeric_type(argv[0]);
  যদি( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) প্রত্যাগম;
  v0 = sqlite3_মান_double(argv[0]);
  x = (দ্বিগুণ(*)(দ্বিগুণ))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
স্থির শূন্য math2Func(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক type0, type1;
  দ্বিগুণ v0, v1, ans;
  দ্বিগুণ (*x)(দ্বিগুণ,দ্বিগুণ);
  প্রতিজ্ঞা( argc==2 );
  type0 = sqlite3_মান_numeric_type(argv[0]);
  যদি( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) প্রত্যাগম;
  type1 = sqlite3_মান_numeric_type(argv[1]);
  যদি( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) প্রত্যাগম;
  v0 = sqlite3_মান_double(argv[0]);
  v1 = sqlite3_মান_double(argv[1]);
  x = (দ্বিগুণ(*)(দ্বিগুণ,দ্বিগুণ))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
স্থির শূন্য piFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  প্রতিজ্ঞা( argc==0 );
  (শূন্য)argv;
  sqlite3_result_double(context, M_PI);
}

#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
স্থির শূন্য signFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক type0;
  দ্বিগুণ x;
  UNUSED_PARAMETER(argc);
  প্রতিজ্ঞা( argc==1 );
  type0 = sqlite3_মান_numeric_type(argv[0]);
  যদি( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) প্রত্যাগম;
  x = sqlite3_মান_double(argv[0]);
  sqlite3_result_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#যদি defined(SQLITE_ENABLE_PERCENTILE)
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
প্রকারনির্দেশ সংরচনা Percentile Percentile;
সংরচনা Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  বর্ণ bSorted;        /* True if a[] is already in sorted order */
  বর্ণ bKeepSorted;    /* True if advantageous to keep a[] sorted */
  বর্ণ bPctValid;      /* True if rPct is valid */
  দ্বিগুণ rPct;         /* Fraction.  0.0 to 1.0 */
  দ্বিগুণ *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
স্থির পূর্ণাঙ্ক percentIsInfinity(দ্বিগুণ r){
  sqlite3_uint64 u;
  প্রতিজ্ঞা( পরিমাণ(u)==পরিমাণ(r) );
  memcpy(&u, &r, পরিমাণ(u));
  প্রত্যাগম ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
স্থির পূর্ণাঙ্ক percent_Same_মান(দ্বিগুণ a, দ্বিগুণ b){
  a -= b;
  প্রত্যাগম a>=-0.001 && a<=0.001;
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
স্থির i64 percent_দ্বিমিক_অনুসন্ধান(Percentile *p, দ্বিগুণ y, পূর্ণাঙ্ক bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  যাবত্( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    দ্বিগুণ x = p->a[iMid];
    যদি( x<y ){
      iFirst = iMid + 1;
    }অন্যথা যদি( x>y ){
      iLast = iMid - 1;
    }অন্যথা{
      প্রত্যাগম iMid;
    }
  }
  যদি( bExact ) প্রত্যাগম -1;
  প্রত্যাগম iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
স্থির শূন্য percentError(sqlite3_context *pCtx, ধ্রুব বর্ণ *zFormat, ...){
  বর্ণ *zMsg1;
  বর্ণ *zMsg2;
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
স্থির শূন্য percentStep(sqlite3_context *pCtx, পূর্ণাঙ্ক argc, sqlite3_মান **argv){
  Percentile *p;
  দ্বিগুণ rPct;
  পূর্ণাঙ্ক eType;
  দ্বিগুণ y;
  প্রতিজ্ঞা( argc==2 || argc==1 );

  যদি( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }অন্যথা{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    দ্বিগুণ mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_মান_numeric_type(argv[1]);
    rPct = sqlite3_মান_double(argv[1])/mxFrac;
    যদি( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (দ্বিগুণ)mxFrac);
      প্রত্যাগম;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, পরিমাণ(*p));
  যদি( p==0 ) প্রত্যাগম;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  যদি( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }অন্যথা যদি( !percent_Same_মান(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    প্রত্যাগম;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_মান_type(argv[0]);
  যদি( eType==SQLITE_NULL ) প্রত্যাগম;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  যদি( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    প্রত্যাগম;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_মান_double(argv[0]);
  যদি( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    প্রত্যাগম;
  }

  /* Allocate and store the Y */
  যদি( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    দ্বিগুণ *a = sqlite3_realloc64(p->a, পরিমাণ(দ্বিগুণ)*n);
    যদি( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, পরিমাণ(*p));
      sqlite3_result_error_nomem(pCtx);
      প্রত্যাগম;
    }
    p->nAlloc = n;
    p->a = a;
  }
  যদি( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }অন্যথা যদি( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }অন্যথা যদি( p->bKeepSorted ){
    i64 i;
    i = percent_দ্বিমিক_অনুসন্ধান(p, y, 0);
    যদি( i<(পূর্ণাঙ্ক)p->nUsed ){
      memmove(&p->a[i+1], &p->a[i], (p->nUsed-i)*পরিমাণ(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }অন্যথা{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#define SWAP_DOUBLE(X,Y)  {দ্বিগুণ ttt=(X);(X)=(Y);(Y)=ttt;}

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
স্থির শূন্য percentSort(
  দ্বিগুণ *a,                      /* Array to sort */
  unsigned পূর্ণাঙ্ক n,                 /* Number of elements in array a[] */
  পূর্ণাঙ্ক iReq                        /* Element caller cares about (or -ve) */
){
  পূর্ণাঙ্ক iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  পূর্ণাঙ্ক iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  পূর্ণাঙ্ক i;         /* Loop counter */
  দ্বিগুণ rPivot; /* The pivot value */

  প্রতিজ্ঞা( n>=2 );
  কুরু{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    যদি( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    যদি( n==2 ) প্রত্যাগম;
    iGt = n-1;
    i = n/2;
    যদি( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }অন্যথা যদি( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    যদি( n==3 ) প্রত্যাগম;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    কুরু{
      যদি( a[i]<rPivot ){
        যদি( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }অন্যথা যদি( a[i]>rPivot ){
        কুরু{
          iGt--;
        }যাবত্( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }অন্যথা{
        i++;
      }
    }যাবত্( i<iGt );

    প্রতিজ্ঞা( iLt>0 && iLt<iGt && (unsigned)iGt<n );
    testcase( iGt>iLt+1 );
    প্রতিজ্ঞা( a[iLt]==rPivot );
    প্রতিজ্ঞা( a[iLt-1]<=rPivot );
    প্রতিজ্ঞা( a[iGt]>=rPivot );
    প্রতিজ্ঞা( a[iLt+1]>=rPivot );

    যদি( iReq>=0 ){
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
      যদি( iReq<iLt ){
        n = iLt;
      }অন্যথা{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }অন্যথা{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      যদি( iLt>(পূর্ণাঙ্ক)(n/2) ){
        যদি( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }অন্যথা{
        যদি( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }যাবত্( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
স্থির শূন্য percentInverse(sqlite3_context *pCtx,পূর্ণাঙ্ক argc,sqlite3_মান **argv){
  Percentile *p;
  পূর্ণাঙ্ক eType;
  দ্বিগুণ y;
  i64 i;
  প্রতিজ্ঞা( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, পরিমাণ(*p));
  প্রতিজ্ঞা( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_মান_type(argv[0]);
  যদি( eType==SQLITE_NULL ) প্রত্যাগম;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  যদি( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    প্রত্যাগম;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_মান_double(argv[0]);
  যদি( percentIsInfinity(y) ){
    প্রত্যাগম;
  }
  যদি( p->bSorted==0 ){
    প্রতিজ্ঞা( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_দ্বিমিক_অনুসন্ধান(p, y, 1);
  যদি( i>=0 ){
    p->nUsed--;
    যদি( i<(পূর্ণাঙ্ক)p->nUsed ){
      memmove(&p->a[i], &p->a[i+1], (p->nUsed - i)*পরিমাণ(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
স্থির শূন্য percentCompute(sqlite3_context *pCtx, পূর্ণাঙ্ক bIsFinal){
  Percentile *p;
  পূর্ণাঙ্ক settings = SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&1; /* Discrete? */
  unsigned i1, i2;
  দ্বিগুণ v1, v2;
  দ্বিগুণ ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  যদি( p==0 ) প্রত্যাগম;
  যদি( p->a==0 ) প্রত্যাগম;
  যদি( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (unsigned)ix;
    যদি( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      প্রতিজ্ঞা( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (পূর্ণাঙ্ক)i1 : -1));
      p->bSorted = 1;
    }
    যদি( settings & 1 ){
      vx = p->a[i1];
    }অন্যথা{
      i2 = ix==(দ্বিগুণ)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  যদি( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, পরিমাণ(*p));
  }অন্যথা{
    p->bKeepSorted = 1;
  }
}
স্থির শূন্য percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
স্থির শূন্য percent_মান(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#endif /* SQLITE_ENABLE_PERCENTILE */

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
স্থির শূন্য filestatFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ধ্রুব বর্ণ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  যদি( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    পূর্ণাঙ্ক rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    প্রতিজ্ঞা( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    যদি( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }অন্যথা{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      যদি( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      যদি( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        যদি( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }অন্যথা{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
স্থির শূন্য walstatFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ধ্রুব বর্ণ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  যদি( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    প্রতিজ্ঞা( pPager!=0 );
    pStr = sqlite3_str_new(db);
    যদি( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }অন্যথা{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }অন্যথা{
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
স্থির শূন্য fpdecodeFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  FpDecode s;
  দ্বিগুণ x;
  পূর্ণাঙ্ক y, z;
  বর্ণ zBuf[100];
  UNUSED_PARAMETER(argc);
  প্রতিজ্ঞা( argc==3 );
  x = sqlite3_মান_double(argv[0]);
  y = sqlite3_মান_int(argv[1]);
  z = sqlite3_মান_int(argv[2]);
  যদি( z<=0 ) z = 1;
  যদি( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  যদি( s.isSpecial==2 ){
    sqlite3_snprintf(পরিমাণ(zBuf), zBuf, "NaN");
  }অন্যথা{
    sqlite3_snprintf(পরিমাণ(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
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
স্থির শূন্য parseuriFunc(
  sqlite3_context *ctx,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  sqlite3_str *pResult;
  ধ্রুব বর্ণ *zVfs;
  ধ্রুব বর্ণ *zUri;
  unsigned পূর্ণাঙ্ক flgs;
  পূর্ণাঙ্ক rc;
  sqlite3_vfs *pVfs = 0;
  বর্ণ *zFile = 0;
  বর্ণ *zErr = 0;

  যদি( argc<2 ) প্রত্যাগম;
  pVfs = sqlite3_vfs_find(0);
  প্রতিজ্ঞা( pVfs );
  zVfs = pVfs->zName;
  zUri = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[0]);
  যদি( zUri==0 ) প্রত্যাগম;
  flgs = (unsigned পূর্ণাঙ্ক)sqlite3_মান_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  pResult = sqlite3_str_new(0);
  যদি( !sqlite3_str_errcode(pResult) ){
    পূর্ণাঙ্ক i;
    sqlite3_str_appendf(pResult, "rc=%d", rc);
    sqlite3_str_appendf(pResult, ", flags=0x%x", flgs);
    sqlite3_str_appendf(pResult, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(pResult, ", err=%Q", zErr);
    sqlite3_str_appendf(pResult, ", file=%Q", zFile);
    যদি( zFile ){
      ধ্রুব বর্ণ *z = zFile;
      z += sqlite3Strlen30(z)+1;
      যাবত্( z[0] ){
        sqlite3_str_appendf(pResult, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      প্রতি(i=2; i<argc; i++){
        ধ্রুব বর্ণ *zArg;
        যদি( sqlite3_মান_type(argv[i])==SQLITE_INTEGER ){
          পূর্ণাঙ্ক k = sqlite3_মান_int(argv[i]);
          sqlite3_str_appendf(pResult, ", '%d:%q'",k,sqlite3_uri_চাবি(zFile, k));
        }অন্যথা যদি( (zArg = (ধ্রুব বর্ণ*)sqlite3_মান_text(argv[i]))!=0 ){
          sqlite3_str_appendf(pResult, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }অন্যথা{
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
শূন্য sqlite3RegisterBuiltinFunctions(শূন্য){
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
  স্থির FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#যদি !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(রাশিমালা_compare,        2, INLINEFUNC_রাশিমালা_compare,        0),
    TEST_FUNC(রাশিমালা_implies_রাশিমালা,   2, INLINEFUNC_রাশিমালা_implies_রাশিমালা,   0),
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
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_মান, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_মান, 0,
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
    FUNCTION(বর্ণ,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifdef SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#endif
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
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
    VFUNCTION(last_ঢোকাও_rowid, 0, 0, 0, last_ঢোকাও_rowid),
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
    WAGGREGATE(গুনতি, 0,0,0, গুনতি_Step,
        গুনতি_Finalize, গুনতি_Finalize, গুনতি_Inverse,
        SQLITE_FUNC_গুনতি|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(গুনতি, 1,0,0, গুনতি_Step,
        গুনতি_Finalize, গুনতি_Finalize, গুনতি_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_মান, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_মান, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_মান, groupConcatInverse, 0),

#ifdef SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_মান, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_মান, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_মান, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_মান, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#endif /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#অন্যথা
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
#যদি SQLITE_HAVE_C99_MATH_FUNCS
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
#যদি SQLITE_HAVE_C99_MATH_FUNCS
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
    INLINE_FUNC(যদি,             -4, INLINEFUNC_iif,      0 ),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_ঢোকাও_Builtin_Funcs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#যদি 0  /* Enable to print out how the built-in functions are hashed */
  {
    পূর্ণাঙ্ক i;
    FuncDef *p;
    প্রতি(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      প্রতি(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        পূর্ণাঙ্ক n = sqlite3Strlen30(p->zName);
        পূর্ণাঙ্ক h = p->zName[0] + n;
        প্রতিজ্ঞা( p->funcFlags & SQLITE_FUNC_BUILTIN );
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}
