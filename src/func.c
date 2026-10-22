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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <assert.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#અન્તર્ભાવ <math.h>
#endif
#અન્તર્ભાવ "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
સ્થિર CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  પ્રતિજ્ઞા( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  પ્રતિજ્ઞા( pOp->opcode==OP_CollSeq );
  પ્રતિજ્ઞા( pOp->p4type==P4_COLLSEQ );
  પાછા pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
સ્થિર શૂન્ય sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  પ્રતિજ્ઞા( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
સ્થિર શૂન્ય minmaxFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક i;
  પૂર્ણાંક mask;    /* 0 for min() or 0xffffffff for max() */
  પૂર્ણાંક iBest;
  CollSeq *pColl;

  પ્રતિજ્ઞા( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  પ્રતિજ્ઞા( pColl );
  પ્રતિજ્ઞા( mask==-1 || mask==0 );
  iBest = 0;
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ) પાછા;
  માટે(i=1; i<argc; i++){
    જો( sqlite3_કિંમત_type(argv[i])==SQLITE_NULL ) પાછા;
    જો( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_કિંમત(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
સ્થિર શૂન્ય typeofFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  સ્થિર અચલ અક્ષર *azType[] = { "integer", "real", "text", "blob", "null" };
  પૂર્ણાંક i = sqlite3_કિંમત_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  પ્રતિજ્ઞા( i>=0 && i<ArraySize(azType) );
  પ્રતિજ્ઞા( SQLITE_INTEGER==1 );
  પ્રતિજ્ઞા( SQLITE_FLOAT==2 );
  પ્રતિજ્ઞા( SQLITE_TEXT==3 );
  પ્રતિજ્ઞા( SQLITE_BLOB==4 );
  પ્રતિજ્ઞા( SQLITE_NULL==5 );
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
સ્થિર શૂન્ય subtypeFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_result_int(context, sqlite3_કિંમત_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
સ્થિર શૂન્ય lengthFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  પસંદગી( sqlite3_કિંમત_type(argv[0]) ){
    સ્થિતિ SQLITE_BLOB:
    સ્થિતિ SQLITE_INTEGER:
    સ્થિતિ SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_કિંમત_bytes(argv[0]));
      તોડો;
    }
    સ્થિતિ SQLITE_TEXT: {
      અચલ unsigned અક્ષર *z = sqlite3_કિંમત_text(argv[0]);
      અચલ unsigned અક્ષર *z0;
      જો( z==0 ) પાછા;
      z0 = z;
      જ્યારે( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        જો( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }નહીંતર જો( z[0]==0 ){
          તોડો;
        }નહીંતર{
          z++;
          જ્યારે( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_result_int(context, (પૂર્ણાંક)(z-z0));
      તોડો;
    }
    મૂળભૂત: {
      sqlite3_result_null(context);
      તોડો;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
સ્થિર શૂન્ય bytelengthFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  પસંદગી( sqlite3_કિંમત_type(argv[0]) ){
    સ્થિતિ SQLITE_BLOB: {
      sqlite3_result_int(context, sqlite3_કિંમત_bytes(argv[0]));
      તોડો;
    }
    સ્થિતિ SQLITE_INTEGER:
    સ્થિતિ SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_result_int64(context, sqlite3_કિંમત_bytes(argv[0])*m);
      તોડો;
    }
    સ્થિતિ SQLITE_TEXT: {
      જો( sqlite3_કિંમત_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_result_int(context, sqlite3_કિંમત_bytes(argv[0]));
      }નહીંતર{
        sqlite3_result_int(context, sqlite3_કિંમત_bytes16(argv[0]));
      }
      તોડો;
    }
    મૂળભૂત: {
      sqlite3_result_null(context);
      તોડો;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
સ્થિર શૂન્ય absFunc(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  પસંદગી( sqlite3_કિંમત_type(argv[0]) ){
    સ્થિતિ SQLITE_INTEGER: {
      i64 iVal = sqlite3_કિંમત_int64(argv[0]);
      જો( iVal<0 ){
        જો( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          પાછા;
        }
        iVal = -iVal;
      }
      sqlite3_result_int64(context, iVal);
      તોડો;
    }
    સ્થિતિ SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      તોડો;
    }
    મૂળભૂત: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      દ્વિગુણ rVal = sqlite3_કિંમત_double(argv[0]);
      જો( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      તોડો;
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
સ્થિર શૂન્ય instrFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ unsigned અક્ષર *zHaystack;
  અચલ unsigned અક્ષર *zNeedle;
  પૂર્ણાંક nHaystack;
  પૂર્ણાંક nNeedle;
  પૂર્ણાંક typeHaystack, typeNeedle;
  પૂર્ણાંક N = 1;
  પૂર્ણાંક isText;
  unsigned અક્ષર firstChar;
  sqlite3_કિંમત *pC1 = 0;
  sqlite3_કિંમત *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_કિંમત_type(argv[0]);
  typeNeedle = sqlite3_કિંમત_type(argv[1]);
  જો( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) પાછા;
  nHaystack = sqlite3_કિંમત_bytes(argv[0]);
  nNeedle = sqlite3_કિંમત_bytes(argv[1]);
  જો( nNeedle>0 ){
    જો( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_કિંમત_blob(argv[0]);
      zNeedle = sqlite3_કિંમત_blob(argv[1]);
      isText = 0;
    }નહીંતર જો( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_કિંમત_text(argv[0]);
      zNeedle = sqlite3_કિંમત_text(argv[1]);
      isText = 1;
    }નહીંતર{
      pC1 = sqlite3_કિંમત_dup(argv[0]);
      zHaystack = sqlite3_કિંમત_text(pC1);
      જો( zHaystack==0 ) જાઓ endInstrOOM;
      nHaystack = sqlite3_કિંમત_bytes(pC1);
      pC2 = sqlite3_કિંમત_dup(argv[1]);
      zNeedle = sqlite3_કિંમત_text(pC2);
      જો( zNeedle==0 ) જાઓ endInstrOOM;
      nNeedle = sqlite3_કિંમત_bytes(pC2);
      isText = 1;
    }
    જો( zNeedle==0 || (nHaystack && zHaystack==0) ) જાઓ endInstrOOM;
    firstChar = zNeedle[0];
    જ્યારે( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      કરો{
        nHaystack--;
        zHaystack++;
      }જ્યારે( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    જો( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_કિંમત_free(pC1);
  sqlite3_કિંમત_free(pC2);
  પાછા;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  જાઓ endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
સ્થિર શૂન્ય printfFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  PrintfArguments x;
  StrAccum str;
  અચલ અક્ષર *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  જો( argc>=1 && (zFormat = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]))!=0 ){
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
સ્થિર SQLITE_INLINE પૂર્ણાંક containsNulOrNonAscii(u64 word){
  અચલ u64 byteOnes = UINT64_C(0x0101010101010101);
  અચલ u64 byte_ઉપલું_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  પાછા ((word | (word - byteOnes)) & byte_ઉપલું_Bits)!=0;
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
સ્થિર શૂન્ય substrFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ unsigned અક્ષર *z;
  અચલ unsigned અક્ષર *z2;
  પૂર્ણાંક len;
  પૂર્ણાંક p0type;
  i64 p1, p2;

  પ્રતિજ્ઞા( argc==3 || argc==2 );
  p0type = sqlite3_કિંમત_type(argv[0]);
  p1 = sqlite3_કિંમત_int64(argv[1]);
  જો( p0type==SQLITE_BLOB ){
    len = sqlite3_કિંમત_bytes(argv[0]);
    z = sqlite3_કિંમત_blob(argv[0]);
    જો( z==0 ) પાછા;
    પ્રતિજ્ઞા( len==sqlite3_કિંમત_bytes(argv[0]) );
  }નહીંતર{
    z = sqlite3_કિંમત_text(argv[0]);
    જો( z==0 ) પાછા;
    len = 0;
    જો( p1<0 ){
      માટે(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  જો( argc==3 ){
    p2 = sqlite3_કિંમત_int64(argv[2]);
    જો( p2==0 && sqlite3_કિંમત_type(argv[2])==SQLITE_NULL ) પાછા;
  }નહીંતર{
    p2 = LARGEST_INT64;
  }
  જો( p1==0 ){
#ifdef SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#endif
    જો( sqlite3_કિંમત_type(argv[1])==SQLITE_NULL ) પાછા;
  }
  જો( p1<0 ){
    p1 += len;
    જો( p1<0 ){
      જો( p2<0 ){
        p2 = 0;
      }નહીંતર{
        p2 += p1;
      }
      p1 = 0;
    }
  }નહીંતર જો( p1>0 ){
    p1--;
  }નહીંતર જો( p2>0 ){
    p2--;
  }
  જો( p2<0 ){
    જો( p2<-p1 ){
      p2 = p1;
    }નહીંતર{
      p2 = -p2;
    }
    p1 -= p2;
  }
  પ્રતિજ્ઞા( p1>=0 && p2>=0 );
  જો( p0type!=SQLITE_BLOB ){
    જો( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      અચલ unsigned અક્ષર *zEnd = z + sqlite3_કિંમત_bytes(argv[0]);
      જ્યારે( p1>=8 && zEnd-z>=8 ){
        u64 word;
        memcpy(&word, z, કદ(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        જો( containsNulOrNonAscii(word) ){
          તોડો;
        }
        z += 8;
        p1 -= 8;
      }
    }
    માટે( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      જો( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }નહીંતર જો( z[0]==0 ){
        તોડો;
      }નહીંતર{
        કરો{ z++; }જ્યારે( (z[0]&0xc0)==0x80 );
      }
    }
    માટે(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      જો( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }નહીંતર જો( z2[0]==0 ){
        તોડો;
      }નહીંતર{
        કરો{ z2++; }જ્યારે( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_result_text64(context, (અક્ષર*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }નહીંતર{
    જો( p1>=len ){
      p1 = p2 = 0;
    }નહીંતર જો( p2>len-p1 ){
      p2 = len-p1;
      પ્રતિજ્ઞા( p2>0 );
    }
    sqlite3_result_blob64(context, (અક્ષર*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
સ્થિર શૂન્ય roundFunc(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  i64 n = 0;
  દ્વિગુણ r;
  અક્ષર *zBuf;
  પ્રતિજ્ઞા( argc==1 || argc==2 );
  જો( argc==2 ){
    જો( SQLITE_NULL==sqlite3_કિંમત_type(argv[1]) ) પાછા;
    n = sqlite3_કિંમત_int64(argv[1]);
    જો( n>30 ) n = 30;
    જો( n<0 ) n = 0;
  }
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ) પાછા;
  r = sqlite3_કિંમત_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  જો( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }નહીંતર જો( n==0 ){ 
    r = (દ્વિગુણ)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }નહીંતર{
    zBuf = sqlite3_mprintf("%!.*f",(પૂર્ણાંક)n,r);
    જો( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      પાછા;
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
સ્થિર શૂન્ય *contextMalloc(sqlite3_context *context, i64 nByte){
  અક્ષર *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  પ્રતિજ્ઞા( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  જો( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }નહીંતર{
    z = sqlite3Malloc(nByte);
    જો( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  પાછા z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
સ્થિર શૂન્ય upperFunc(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  અક્ષર *z1;
  અચલ અક્ષર *z2;
  પૂર્ણાંક i, n;
  UNUSED_PARAMETER(argc);
  z2 = (અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  n = sqlite3_કિંમત_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  પ્રતિજ્ઞા( z2==(અક્ષર*)sqlite3_કિંમત_text(argv[0]) );
  જો( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    જો( z1 ){
      માટે(i=0; i<n; i++){
        z1[i] = (અક્ષર)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
સ્થિર શૂન્ય lowerFunc(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  અક્ષર *z1;
  અચલ અક્ષર *z2;
  પૂર્ણાંક i, n;
  UNUSED_PARAMETER(argc);
  z2 = (અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  n = sqlite3_કિંમત_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  પ્રતિજ્ઞા( z2==(અક્ષર*)sqlite3_કિંમત_text(argv[0]) );
  જો( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    જો( z1 ){
      માટે(i=0; i<n; i++){
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
સ્થિર શૂન્ય randomFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(કદ(r), &r);
  જો( r<0 ){
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
સ્થિર શૂન્ય randomBlob(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3_int64 n;
  unsigned અક્ષર *p;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_કિંમત_int64(argv[0]);
  જો( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  જો( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (અક્ષર*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
સ્થિર શૂન્ય last_ઉમેરો_rowid(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_ઉમેરો_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
સ્થિર શૂન્ય changes(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
સ્થિર શૂન્ય total_changes(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **NotUsed2
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
સંરચના compareInfo {
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
#જો defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#નહીંતર
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

સ્થિર અચલ સંરચના compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
સ્થિર અચલ સંરચના compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
સ્થિર અચલ સંરચના compareInfo likeInfoAlt = { '%', '_',   0, 0 };

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
સ્થિર પૂર્ણાંક patternCompare(
  અચલ u8 *zPattern,              /* The glob pattern */
  અચલ u8 *zString,               /* The string to compare against the glob */
  અચલ સંરચના compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  અચલ u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  જ્યારે( (c = Utf8Read(zPattern))!=0 ){
    જો( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      જ્યારે( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        જો( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          પાછા SQLITE_NOWILDCARDMATCH;
        }
      }
      જો( c==0 ){
        પાછા SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }નહીંતર જો( c==matchOther ){
        જો( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          જો( c==0 ) પાછા SQLITE_NOWILDCARDMATCH;
        }નહીંતર{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          પ્રતિજ્ઞા( matchOther<0x80 );  /* '[' is a single-byte character */
          જ્યારે( *zString ){
            પૂર્ણાંક bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            જો( bMatch!=SQLITE_NOMATCH ) પાછા bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          પાછા SQLITE_NOWILDCARDMATCH;
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
      જો( c<0x80 ){
        અક્ષર zStop[3];
        પૂર્ણાંક bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        જો( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        જો( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }નહીંતર{
          zStop[0] = c;
          zStop[1] = 0;
        }
        જ્યારે(1){
          zString += strcspn((અચલ અક્ષર*)zString, zStop);
          જો( zString[0]==0 ) તોડો;
          zString++;
          જો( nx && zString[0]!=nx && zString[0]!=ny ) ચાલુ;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          જો( bMatch!=SQLITE_NOMATCH ) પાછા bMatch;
        }
      }નહીંતર{
        પૂર્ણાંક bMatch;
        જ્યારે( (c2 = Utf8Read(zString))!=0 ){
          જો( c2!=c ) ચાલુ;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          જો( bMatch!=SQLITE_NOMATCH ) પાછા bMatch;
        }
      }
      પાછા SQLITE_NOWILDCARDMATCH;
    }
    જો( c==matchOther ){
      જો( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        જો( c==0 ) પાછા SQLITE_NOMATCH;
        zEscaped = zPattern;
      }નહીંતર{
        u32 prior_c = 0;
        પૂર્ણાંક seen = 0;
        પૂર્ણાંક invert = 0;
        c = sqlite3Utf8Read(&zString);
        જો( c==0 ) પાછા SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        જો( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        જો( c2==']' ){
          જો( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        જ્યારે( c2 && c2!=']' ){
          જો( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            જો( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }નહીંતર{
            જો( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        જો( c2==0 || (seen ^ invert)==0 ){
          પાછા SQLITE_NOMATCH;
        }
        ચાલુ;
      }
    }
    c2 = Utf8Read(zString);
    જો( c==c2 ) ચાલુ;
    જો( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      ચાલુ;
    }
    જો( c==matchOne && zPattern!=zEscaped && c2!=0 ) ચાલુ;
    પાછા SQLITE_NOMATCH;
  }
  પાછા *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
પૂર્ણાંક sqlite3_strglob(અચલ અક્ષર *zGlobPattern, અચલ અક્ષર *zString){
  જો( zString==0 ){
    પાછા zGlobPattern!=0;
  }નહીંતર જો( zGlobPattern==0 ){
    પાછા 1;
  }નહીંતર {
    પાછા patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
પૂર્ણાંક sqlite3_strlike(અચલ અક્ષર *zPattern, અચલ અક્ષર *zStr, unsigned પૂર્ણાંક esc){
  જો( zStr==0 ){
    પાછા zPattern!=0;
  }નહીંતર જો( zPattern==0 ){
    પાછા 1;
  }નહીંતર{
    પાછા patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
પૂર્ણાંક sqlite3_like_ગણતરી = 0;
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
સ્થિર શૂન્ય likeFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ unsigned અક્ષર *zA, *zB;
  u32 escape;
  પૂર્ણાંક nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  સંરચના compareInfo *pInfo = sqlite3_user_data(context);
  સંરચના compareInfo backupInfo;

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_BLOB
   || sqlite3_કિંમત_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_ગણતરી++;
#endif
    sqlite3_result_int(context, 0);
    પાછા;
  }
#endif

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_કિંમત_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  જો( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    પાછા;
  }
  જો( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    અચલ unsigned અક્ષર *zEsc = sqlite3_કિંમત_text(argv[2]);
    જો( zEsc==0 ) પાછા;
    જો( sqlite3Utf8CharLen((અક્ષર*)zEsc, -1)!=1 ){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      પાછા;
    }
    escape = sqlite3Utf8Read(&zEsc);
    જો( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      memcpy(&backupInfo, pInfo, કદ(backupInfo));
      pInfo = &backupInfo;
      જો( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      જો( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }નહીંતર{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_કિંમત_text(argv[0]);
  zA = sqlite3_કિંમત_text(argv[1]);
  જો( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_ગણતરી++;
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
સ્થિર શૂન્ય nullifFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  જો( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_કિંમત(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
સ્થિર શૂન્ય versionFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **NotUsed2
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
સ્થિર શૂન્ય sourceidFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **NotUsed2
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
સ્થિર શૂન્ય errlogFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_કિંમત_int(argv[0]), "%s", sqlite3_કિંમત_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
સ્થિર શૂન્ય compileoptionusedFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ અક્ષર *zOptName;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  જો( (zOptName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]))!=0 ){
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
સ્થિર શૂન્ય compileoptiongetFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  i64 n;
  અચલ અક્ષર *zRes;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_કિંમત_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((પૂર્ણાંક)n) : 0;
  sqlite3_result_text(context, zRes, -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
સ્થિર અચલ અક્ષર hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
શૂન્ય sqlite3_Quote_કિંમત(StrAccum *pStr, sqlite3_કિંમત *p_કિંમત, પૂર્ણાંક bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  પ્રતિજ્ઞા( pStr!=0 && pStr->nChar==0 );

  પસંદગી( sqlite3_કિંમત_type(p_કિંમત) ){
    સ્થિતિ SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_કિંમત_double(p_કિંમત));
      તોડો;
    }
    સ્થિતિ SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_કિંમત_int64(p_કિંમત));
      તોડો;
    }
    સ્થિતિ SQLITE_BLOB: {
      અક્ષર અચલ *zBlob = sqlite3_કિંમત_blob(p_કિંમત);
      i64 nBlob = sqlite3_કિંમત_bytes(p_કિંમત);
      પ્રતિજ્ઞા( zBlob==sqlite3_કિંમત_blob(p_કિંમત) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      જો( pStr->accError==0 ){
        અક્ષર *zText = pStr->zText;
        પૂર્ણાંક i;
        માટે(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_TEXT: {
      અચલ unsigned અક્ષર *zArg = sqlite3_કિંમત_text(p_કિંમત);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      તોડો;
    }
    મૂળભૂત: {
      પ્રતિજ્ઞા( sqlite3_કિંમત_type(p_કિંમત)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      તોડો;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
સ્થિર પૂર્ણાંક isNHex(અચલ અક્ષર *z, પૂર્ણાંક N, u32 *pVal){
  પૂર્ણાંક i;
  u32 v = 0;
  માટે(i=0; i<N; i++){
    જો( !sqlite3Isxdigit(z[i]) ) પાછા 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  પાછા 1;
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
સ્થિર શૂન્ય unistrFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અક્ષર *zOut;
  અચલ અક્ષર *zIn;
  પૂર્ણાંક nIn;
  પૂર્ણાંક i, j, n;
  u32 v;

  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  જો( zIn==0 ) પાછા;
  nIn = sqlite3_કિંમત_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  જો( zOut==0 ){
    sqlite3_result_error_nomem(context);
    પાછા;
  }
  i = j = 0;
  જ્યારે( i<nIn ){
    અચલ અક્ષર *z = strchr(&zIn[i],'\\');
    જો( z==0 ){
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      તોડો;
    }
    n = z - &zIn[i];
    જો( n>0 ){
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    જો( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }નહીંતર જો( sqlite3Isxdigit(zIn[i+1]) ){
      જો( !isNHex(&zIn[i+1], 4, &v) ) જાઓ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }નહીંતર જો( zIn[i+1]=='+' ){
      જો( !isNHex(&zIn[i+2], 6, &v) ) જાઓ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }નહીંતર જો( zIn[i+1]=='u' ){
      જો( !isNHex(&zIn[i+2], 4, &v) ) જાઓ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }નહીંતર જો( zIn[i+1]=='U' ){
      જો( !isNHex(&zIn[i+2], 8, &v) ) જાઓ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }નહીંતર{
      જાઓ unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  પાછા;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  પાછા;
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
સ્થિર શૂન્ય quoteFunc(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  sqlite3_Quote_કિંમત(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_data(context)));
  sqlite3_result_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
સ્થિર શૂન્ય unicodeFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ unsigned અક્ષર *z = sqlite3_કિંમત_text(argv[0]);
  (શૂન્ય)argc;
  જો( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
સ્થિર શૂન્ય charFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  unsigned અક્ષર *z, *zOut;
  પૂર્ણાંક i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  જો( z==0 ){
    sqlite3_result_error_nomem(context);
    પાછા;
  }
  માટે(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_કિંમત_int64(argv[i]);
    જો( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    જો( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }નહીંતર જો( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }નહીંતર જો( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }નહીંતર{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_result_text64(context, (અક્ષર*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
સ્થિર શૂન્ય hexFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક i, n;
  અચલ unsigned અક્ષર *pBlob;
  અક્ષર *zHex, *z;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_કિંમત_blob(argv[0]);
  n = sqlite3_કિંમત_bytes(argv[0]);
  પ્રતિજ્ઞા( pBlob==sqlite3_કિંમત_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  જો( zHex ){
    માટે(i=0; i<n; i++, pBlob++){
      unsigned અક્ષર c = *pBlob;
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
સ્થિર પૂર્ણાંક strContainsChar(અચલ u8 *zStr, પૂર્ણાંક nStr, u32 ch){
  અચલ u8 *zEnd = &zStr[nStr];
  અચલ u8 *z = zStr;
  જ્યારે( z<zEnd ){
    u32 tst = Utf8Read(z);
    જો( tst==ch ) પાછા 1;
  }
  પાછા 0;
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
સ્થિર શૂન્ય unhexFunc(
  sqlite3_context *pCtx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ u8 *zPass = (અચલ u8*)"";
  પૂર્ણાંક nPass = 0;
  અચલ u8 *zHex = sqlite3_કિંમત_text(argv[0]);
  પૂર્ણાંક nHex = sqlite3_કિંમત_bytes(argv[0]);
#ifdef SQLITE_DEBUG
  અચલ u8 *zEnd = zHex ? &zHex[nHex] : 0;
#endif
  u8 *pBlob = 0;
  u8 *p = 0;

  પ્રતિજ્ઞા( argc==1 || argc==2 );
  જો( argc==2 ){
    zPass = sqlite3_કિંમત_text(argv[1]);
    nPass = sqlite3_કિંમત_bytes(argv[1]);
  }
  જો( !zHex || !zPass ) પાછા;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  જો( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    જ્યારે( (c = *zHex)!=0x00 ){
      જ્યારે( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        પ્રતિજ્ઞા( zHex<=zEnd );
        જો( !strContainsChar(zPass, nPass, ch) ) જાઓ unhex_null;
        c = *zHex;
        જો( c==0x00 ) જાઓ unhex_done;
      }
      zHex++;
      પ્રતિજ્ઞા( *zEnd==0x00 );
      પ્રતિજ્ઞા( zHex<=zEnd );
      d = *(zHex++);
      જો( !sqlite3Isxdigit(d) ) જાઓ unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  પાછા;

 unhex_null:
  sqlite3_free(pBlob);
  પાછા;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
સ્થિર શૂન્ય zeroblobFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  i64 n;
  પૂર્ણાંક rc;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_કિંમત_int64(argv[0]);
  જો( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  જો( rc ){
    sqlite3_result_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
સ્થિર શૂન્ય replaceFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ unsigned અક્ષર *zStr;        /* The input string A */
  અચલ unsigned અક્ષર *zPattern;    /* The pattern string B */
  અચલ unsigned અક્ષર *zRep;        /* The replacement string C */
  unsigned અક્ષર *zOut;              /* The output */
  પૂર્ણાંક nStr;                /* Size of zStr */
  પૂર્ણાંક nPattern;            /* Size of zPattern */
  પૂર્ણાંક nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  પૂર્ણાંક loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  unsigned cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  પ્રતિજ્ઞા( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_કિંમત_text(argv[0]);
  જો( zStr==0 ) પાછા;
  nStr = sqlite3_કિંમત_bytes(argv[0]);
  પ્રતિજ્ઞા( zStr==sqlite3_કિંમત_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_કિંમત_text(argv[1]);
  જો( zPattern==0 ){
    પ્રતિજ્ઞા( sqlite3_કિંમત_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    પાછા;
  }
  જો( zPattern[0]==0 ){
    પ્રતિજ્ઞા( sqlite3_કિંમત_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_text(context, (અચલ અક્ષર*)zStr, nStr, SQLITE_TRANSIENT);
    પાછા;
  }
  nPattern = sqlite3_કિંમત_bytes(argv[1]);
  પ્રતિજ્ઞા( zPattern==sqlite3_કિંમત_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_કિંમત_text(argv[2]);
  જો( zRep==0 ) પાછા;
  nRep = sqlite3_કિંમત_bytes(argv[2]);
  પ્રતિજ્ઞા( zRep==sqlite3_કિંમત_text(argv[2]) );
  nOut = nStr + 1;
  પ્રતિજ્ઞા( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, nOut);
  જો( zOut==0 ){
    પાછા;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  માટે(i=j=0; i<=loopLimit; i++){
    જો( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }નહીંતર{
      જો( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
        જો( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          પાછા;
        }
        cntExpand++;
        જો( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (પૂર્ણાંક)nOut + (nOut - nStr - 1));
          જો( zOut==0 ){
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            પાછા;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  પ્રતિજ્ઞા( j+nStr-i+1<=nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  પ્રતિજ્ઞા( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (અક્ષર*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
સ્થિર શૂન્ય trimFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ unsigned અક્ષર *zIn;         /* Input string */
  અચલ unsigned અક્ષર *zCharSet;    /* Set of characters to trim */
  unsigned પૂર્ણાંક nIn;                 /* Number of bytes in input */
  પૂર્ણાંક flags;                        /* 1: trimleft  2: trimright  3: trim */
  પૂર્ણાંક i;                            /* Loop counter */
  unsigned પૂર્ણાંક *aLen = 0;           /* Length of each character in zCharSet */
  unsigned અક્ષર **azChar = 0;       /* Individual characters in zCharSet */
  પૂર્ણાંક nChar;                        /* Number of characters in zCharSet */

  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ){
    પાછા;
  }
  zIn = sqlite3_કિંમત_text(argv[0]);
  જો( zIn==0 ) પાછા;
  nIn = (unsigned)sqlite3_કિંમત_bytes(argv[0]);
  પ્રતિજ્ઞા( zIn==sqlite3_કિંમત_text(argv[0]) );
  જો( argc==1 ){
    સ્થિર અચલ unsigned lenOne[] = { 1 };
    સ્થિર unsigned અક્ષર * અચલ azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (unsigned*)lenOne;
    azChar = (unsigned અક્ષર **)azOne;
    zCharSet = 0;
  }નહીંતર જો( (zCharSet = sqlite3_કિંમત_text(argv[1]))==0 ){
    પાછા;
  }નહીંતર{
    અચલ unsigned અક્ષર *z;
    માટે(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    જો( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(કદ(અક્ષર*)+કદ(unsigned)));
      જો( azChar==0 ){
        પાછા;
      }
      aLen = (unsigned*)&azChar[nChar];
      માટે(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned અક્ષર *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  જો( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    જો( flags & 1 ){
      જ્યારે( nIn>0 ){
        unsigned પૂર્ણાંક len = 0;
        માટે(i=0; i<nChar; i++){
          len = aLen[i];
          જો( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) તોડો;
        }
        જો( i>=nChar ) તોડો;
        zIn += len;
        nIn -= len;
      }
    }
    જો( flags & 2 ){
      જ્યારે( nIn>0 ){
        unsigned પૂર્ણાંક len = 0;
        માટે(i=0; i<nChar; i++){
          len = aLen[i];
          જો( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) તોડો;
        }
        જો( i>=nChar ) તોડો;
        nIn -= len;
      }
    }
    જો( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (અક્ષર*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
સ્થિર શૂન્ય concatFuncCore(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv,
  પૂર્ણાંક nSep,
  અચલ અક્ષર *zSep
){
  i64 j, n = 0;
  પૂર્ણાંક i;
  પૂર્ણાંક bNotNull = 0;   /* True after at least NOT NULL argument seen */
  અક્ષર *z;
  માટે(i=0; i<argc; i++){
    n += sqlite3_કિંમત_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  જો( z==0 ){
    sqlite3_result_error_nomem(context);
    પાછા;
  }
  j = 0;
  માટે(i=0; i<argc; i++){
    જો( sqlite3_કિંમત_type(argv[i])!=SQLITE_NULL ){
      પૂર્ણાંક k = sqlite3_કિંમત_bytes(argv[i]);
      અચલ અક્ષર *v = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
      જો( v!=0 ){
        જો( bNotNull && nSep>0 ){
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
  પ્રતિજ્ઞા( j<=n );
  sqlite3_result_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
સ્થિર શૂન્ય concatFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
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
સ્થિર શૂન્ય concatwsFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક nSep = sqlite3_કિંમત_bytes(argv[0]);
  અચલ અક્ષર *zSep = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  જો( zSep==0 ) પાછા;
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
સ્થિર શૂન્ય unknownFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  /* no-op */
  (શૂન્ય)context;
  (શૂન્ય)argc;
  (શૂન્ય)argv;
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
સ્થિર શૂન્ય soundexFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અક્ષર zResult[8];
  અચલ u8 *zIn;
  પૂર્ણાંક i, j;
  સ્થિર અચલ unsigned અક્ષર iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  પ્રતિજ્ઞા( argc==1 );
  zIn = (u8*)sqlite3_કિંમત_text(argv[0]);
  જો( zIn==0 ) zIn = (u8*)"";
  માટે(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  જો( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    માટે(j=1; j<4 && zIn[i]; i++){
      પૂર્ણાંક code = iCode[zIn[i]&0x7f];
      જો( code>0 ){
        જો( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }નહીંતર{
        prevcode = 0;
      }
    }
    જ્યારે( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }નહીંતર{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#જો defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
સ્થિર શૂન્ય oomFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#endif /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
સ્થિર શૂન્ય loadExt(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  અચલ અક્ષર *zFile = (અચલ અક્ષર *)sqlite3_કિંમત_text(argv[0]);
  અચલ અક્ષર *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  અક્ષર *z_Err_સંદેશ = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  જો( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    પાછા;
  }

  જો( argc==2 ){
    zProc = (અચલ અક્ષર *)sqlite3_કિંમત_text(argv[1]);
  }નહીંતર{
    zProc = 0;
  }
  જો( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_સંદેશ) ){
    sqlite3_result_error(context, z_Err_સંદેશ, -1);
    sqlite3_free(z_Err_સંદેશ);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
પ્રકારવ્યાખ્યા સંરચના SumCtx SumCtx;
સંરચના SumCtx {
  દ્વિગુણ rSum;      /* Running sum as as a double */
  દ્વિગુણ rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
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
સ્થિર શૂન્ય kahanBabuskaNeumaierStep(
  અસ્થિર SumCtx *pSum,
  અસ્થિર દ્વિગુણ r
){
  અસ્થિર દ્વિગુણ s = pSum->rSum;
  અસ્થિર દ્વિગુણ t = s + r;
  જો( fabs(s) > fabs(r) ){
    pSum->rErr += (s - t) + r;
  }નહીંતર{
    pSum->rErr += (r - t) + s;
  }
  pSum->rSum = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
સ્થિર શૂન્ય kahanBabuskaNeumaierStepInt64(અસ્થિર SumCtx *pSum, i64 iVal){
  જો( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  }નહીંતર{
    kahanBabuskaNeumaierStep(pSum, (દ્વિગુણ)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
સ્થિર શૂન્ય kahanBabuskaNeumaierInit(
  અસ્થિર SumCtx *p,
  i64 iVal
){
  જો( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->rSum = (દ્વિગુણ)(iVal - iSm);
    p->rErr = (દ્વિગુણ)iSm;
  }નહીંતર{
    p->rSum = (દ્વિગુણ)iVal;
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
સ્થિર શૂન્ય sumStep(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  SumCtx *p;
  પૂર્ણાંક type;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, કદ(*p));
  type = sqlite3_કિંમત_numeric_type(argv[0]);
  જો( p && type!=SQLITE_NULL ){
    p->cnt++;
    જો( p->approx==0 ){
      જો( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_કિંમત_double(argv[0]));
      }નહીંતર{
        i64 x = p->iSum;
        જો( sqlite3AddInt64(&x, sqlite3_કિંમત_int64(argv[0]))==0 ){
          p->iSum = x;
        }નહીંતર{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_કિંમત_int64(argv[0]));
        }
      }
    }નહીંતર{
      જો( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_કિંમત_int64(argv[0]));
      }નહીંતર{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_કિંમત_double(argv[0]));
      }
    }
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
સ્થિર શૂન્ય sumInverse(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત**argv){
  SumCtx *p;
  પૂર્ણાંક type;
  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, કદ(*p));
  type = sqlite3_કિંમત_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  જો( ALWAYS(p) && type!=SQLITE_NULL ){
    પ્રતિજ્ઞા( p->cnt>0 );
    p->cnt--;
    જો( !p->approx ){
      i64 x = p->iSum;
      જો( sqlite3SubInt64(&x, sqlite3_કિંમત_int64(argv[0]))==0 ){
        p->iSum = x;
        પાછા;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    જો( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_કિંમત_int64(argv[0]);
      જો( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }નહીંતર{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }નહીંતર{
      kahanBabuskaNeumaierStep(p, -sqlite3_કિંમત_double(argv[0]));
    }
  }
}
#નહીંતર
# define sumInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
સ્થિર શૂન્ય sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  જો( p && p->cnt>0 ){
    જો( p->approx ){
      જો( p->ovrfl ){
        sqlite3_result_error(context,"integer overflow",-1);
      }નહીંતર જો( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_result_double(context, p->rSum+p->rErr);
      }નહીંતર{
        sqlite3_result_double(context, p->rSum);
      }
    }નહીંતર{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
સ્થિર શૂન્ય avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  જો( p && p->cnt>0 ){
    દ્વિગુણ r;
    જો( p->approx ){
      r = p->rSum;
      જો( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }નહીંતર{
      r = (દ્વિગુણ)(p->iSum);
    }
    sqlite3_result_double(context, r/(દ્વિગુણ)p->cnt);
  }
}
સ્થિર શૂન્ય totalFinalize(sqlite3_context *context){
  SumCtx *p;
  દ્વિગુણ r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  જો( p ){
    જો( p->approx ){
      r = p->rSum;
      જો( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }નહીંતર{
      r = (દ્વિગુણ)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
પ્રકારવ્યાખ્યા સંરચના ગણતરી_Ctx ગણતરી_Ctx;
સંરચના ગણતરી_Ctx {
  i64 n;
#ifdef SQLITE_DEBUG
  પૂર્ણાંક bInverse;                   /* True if xInverse() ever called */
#endif
};

/*
** Routines to implement the count() aggregate function.
*/
સ્થિર શૂન્ય ગણતરી_Step(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  ગણતરી_Ctx *p;
  p = sqlite3_aggregate_context(context, કદ(*p));
  જો( (argc==0 || SQLITE_NULL!=sqlite3_કિંમત_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  પ્રતિજ્ઞા( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_ગણતરી(context) );
#endif
}  
સ્થિર શૂન્ય ગણતરી_Finalize(sqlite3_context *context){
  ગણતરી_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}
#ifndef SQLITE_OMIT_WINDOWFUNC
સ્થિર શૂન્ય ગણતરી_Inverse(sqlite3_context *ctx, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  ગણતરી_Ctx *p;
  p = sqlite3_aggregate_context(ctx, કદ(*p));
  /* p is always non-NULL since countStep() will have been called first */
  જો( (argc==0 || SQLITE_NULL!=sqlite3_કિંમત_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ifdef SQLITE_DEBUG
    p->bInverse = 1;
#endif
  }
}  
#નહીંતર
# define ગણતરી_Inverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
સ્થિર શૂન્ય minmaxStep(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, કદ(*pBest));
  જો( !pBest ) પાછા;

  જો( sqlite3_કિંમત_type(pArg)==SQLITE_NULL ){
    જો( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }નહીંતર જો( pBest->flags ){
    પૂર્ણાંક max;
    પૂર્ણાંક cmp;
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
    જો( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }નહીંતર{
      sqlite3SkipAccumulatorLoad(context);
    }
  }નહીંતર{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
સ્થિર શૂન્ય min_Max_કિંમત_Finalize(sqlite3_context *context, પૂર્ણાંક b_કિંમત){
  sqlite3_કિંમત *pRes;
  pRes = (sqlite3_કિંમત *)sqlite3_aggregate_context(context, 0);
  જો( pRes ){
    જો( pRes->flags ){
      sqlite3_result_કિંમત(context, pRes);
    }
    જો( b_કિંમત==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
સ્થિર શૂન્ય min_Max_કિંમત(sqlite3_context *context){
  min_Max_કિંમત_Finalize(context, 1);
}
#નહીંતર
# define min_Max_કિંમત 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
સ્થિર શૂન્ય minMaxFinalize(sqlite3_context *context){
  min_Max_કિંમત_Finalize(context, 0);
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
પ્રકારવ્યાખ્યા સંરચના {
  StrAccum str;          /* The accumulated concatenation */
#ifndef SQLITE_OMIT_WINDOWFUNC
  પૂર્ણાંક nAccum;            /* Number of strings presently concatenated */
  પૂર્ણાંક nFirstSepLength;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  પૂર્ણાંક *pnSepLengths;
#endif
} GroupConcatCtx;

સ્થિર શૂન્ય groupConcatStep(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  અચલ અક્ષર *zVal;
  GroupConcatCtx *pGCC;
  અચલ અક્ષર *zSep;
  પૂર્ણાંક nVal, nSep;
  પ્રતિજ્ઞા( argc==1 || argc==2 );
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ) પાછા;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, કદ(*pGCC));
  જો( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    પૂર્ણાંક firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    જો( argc==1 ){
      જો( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      નહીંતર{
        pGCC->nFirstSepLength = 1;
      }
#endif
    }નહીંતર જો( !firstTerm ){
      zSep = (અક્ષર*)sqlite3_કિંમત_text(argv[1]);
      nSep = sqlite3_કિંમત_bytes(argv[1]);
      જો( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      નહીંતર{
        nSep = 0;
      }
      જો( nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0 ){
        પૂર્ણાંક *pnsl = pGCC->pnSepLengths;
        જો( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (પૂર્ણાંક*)sqlite3_malloc64((pGCC->nAccum+1) * કદ(પૂર્ણાંક));
          જો( pnsl!=0 ){
            પૂર્ણાંક i = 0, nA = pGCC->nAccum-1;
            જ્યારે( i<nA ) pnsl[i++] = pGCC->nFirstSepLength;
          }
        }નહીંતર{
          pnsl = (પૂર્ણાંક*)sqlite3_realloc64(pnsl, pGCC->nAccum * કદ(પૂર્ણાંક));
        }
        જો( pnsl!=0 ){
          જો( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }નહીંતર{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#endif
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    નહીંતર{
      pGCC->nFirstSepLength = sqlite3_કિંમત_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#endif
    zVal = (અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    nVal = sqlite3_કિંમત_bytes(argv[0]);
    જો( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ifndef SQLITE_OMIT_WINDOWFUNC
સ્થિર શૂન્ય groupConcatInverse(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  GroupConcatCtx *pGCC;
  પ્રતિજ્ઞા( argc==1 || argc==2 );
  (શૂન્ય)argc;  /* Suppress unused parameter warning */
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ) પાછા;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, કદ(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  જો( ALWAYS(pGCC) ){
    પૂર્ણાંક nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (શૂન્ય)sqlite3_કિંમત_text(argv[0]);
    nVS = sqlite3_કિંમત_bytes(argv[0]);
    pGCC->nAccum -= 1;
    જો( pGCC->pnSepLengths!=0 ){
      પ્રતિજ્ઞા(pGCC->nAccum >= 0);
      જો( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*કદ(પૂર્ણાંક));
      }
    }નહીંતર{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->nFirstSepLength;
    }
    જો( nVS>=(પૂર્ણાંક)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }નહીંતર{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    જો( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#નહીંતર
# define groupConcatInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
સ્થિર શૂન્ય groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  જો( pGCC ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_XFER);
#ifndef SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#endif
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
સ્થિર શૂન્ય group_Concat_કિંમત(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  જો( pGCC && pGCC->nAccum>0 ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#નહીંતર
# define group_Concat_કિંમત 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
શૂન્ય sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  પૂર્ણાંક rc = sqlite3_overload_function(db, "MATCH", 2);
  પ્રતિજ્ઞા( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  જો( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
શૂન્ય sqlite3RegisterLikeFunctions(sqlite3 *db, પૂર્ણાંક caseSensitive){
  FuncDef *pDef;
  સંરચના compareInfo *pInfo;
  પૂર્ણાંક flags;
  પૂર્ણાંક nArg;
  જો( caseSensitive ){
    pInfo = (સંરચના compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }નહીંતર{
    pInfo = (સંરચના compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  માટે(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    પ્રતિજ્ઞા( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
પૂર્ણાંક sqlite3IsLikeFunction(sqlite3 *db, પદાવલિ *p_પદાવલિ, પૂર્ણાંક *pIsNocase, અક્ષર *aWc){
  FuncDef *pDef;
  પૂર્ણાંક n_પદાવલિ;
  પ્રતિજ્ઞા( p_પદાવલિ!=0 );
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_FUNCTION );
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
  જો( !p_પદાવલિ->x.pList ){
    પાછા 0;
  }
  n_પદાવલિ = p_પદાવલિ->x.pList->n_પદાવલિ;
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
  pDef = sqlite3FindFunction(db, p_પદાવલિ->u.zToken, n_પદાવલિ, SQLITE_UTF8, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  જો( pDef==0 ) પાછા 0;
#endif
  જો( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    પાછા 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  પ્રતિજ્ઞા( (અક્ષર*)&likeInfoAlt == (અક્ષર*)&likeInfoAlt.matchAll );
  પ્રતિજ્ઞા( &((અક્ષર*)&likeInfoAlt)[1] == (અક્ષર*)&likeInfoAlt.matchOne );
  પ્રતિજ્ઞા( &((અક્ષર*)&likeInfoAlt)[2] == (અક્ષર*)&likeInfoAlt.matchSet );

  જો( n_પદાવલિ<3 ){
    aWc[3] = 0;
  }નહીંતર{
    પદાવલિ *pEscape = p_પદાવલિ->x.pList->a[2].p_પદાવલિ;
    અક્ષર *zEscape;
    જો( pEscape->op!=TK_STRING ) પાછા 0;
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pEscape, EP_Int_કિંમત) );
    zEscape = pEscape->u.zToken;
    જો( zEscape[0]==0 || zEscape[1]!=0 ) પાછા 0;
    જો( zEscape[0]==aWc[0] ) પાછા 0;
    જો( zEscape[0]==aWc[1] ) પાછા 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  પાછા 1;
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
સ્થિર શૂન્ય ceilingFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પ્રતિજ્ઞા( argc==1 );
  પસંદગી( sqlite3_કિંમત_numeric_type(argv[0]) ){
    સ્થિતિ SQLITE_INTEGER: {
       sqlite3_result_int64(context, sqlite3_કિંમત_int64(argv[0]));
       તોડો;
    }
    સ્થિતિ SQLITE_FLOAT: {
       દ્વિગુણ (*x)(દ્વિગુણ) = (દ્વિગુણ(*)(દ્વિગુણ))sqlite3_user_data(context);
       sqlite3_result_double(context, x(sqlite3_કિંમત_double(argv[0])));
       તોડો;
    }
    મૂળભૂત: {
       તોડો;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
સ્થિર દ્વિગુણ xCeil(દ્વિગુણ x){ પાછા ceil(x); }
સ્થિર દ્વિગુણ xFloor(દ્વિગુણ x){ પાછા floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#જો defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#endif
#જો defined(HAVE_LOG2) && HAVE_LOG2==0
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
સ્થિર શૂન્ય logFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  દ્વિગુણ x, b, ans;
  પ્રતિજ્ઞા( argc==1 || argc==2 );
  પસંદગી( sqlite3_કિંમત_numeric_type(argv[0]) ){
    સ્થિતિ SQLITE_INTEGER:
    સ્થિતિ SQLITE_FLOAT:
      x = sqlite3_કિંમત_double(argv[0]);
      જો( x<=0.0 ) પાછા;
      તોડો;
    મૂળભૂત:
      પાછા;
  }
  જો( argc==2 ){
    પસંદગી( sqlite3_કિંમત_numeric_type(argv[1]) ){
      સ્થિતિ SQLITE_INTEGER:
      સ્થિતિ SQLITE_FLOAT:
        b = log(x);
        જો( b<=0.0 ) પાછા;
        x = sqlite3_કિંમત_double(argv[1]);
        જો( x<=0.0 ) પાછા;
        તોડો;
     મૂળભૂત:
        પાછા;
    }
    ans = log(x)/b;
  }નહીંતર{
    પસંદગી( SQLITE_PTR_TO_INT(sqlite3_user_data(context)) ){
      સ્થિતિ 1:
        ans = log10(x);
        તોડો;
      સ્થિતિ 2:
        ans = log2(x);
        તોડો;
      મૂળભૂત:
        ans = log(x);
        તોડો;
    }
  }
  sqlite3_result_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
સ્થિર દ્વિગુણ degToRad(દ્વિગુણ x){ પાછા x*(M_PI/180.0); }
સ્થિર દ્વિગુણ radToDeg(દ્વિગુણ x){ પાછા x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
સ્થિર શૂન્ય math1Func(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક type0;
  દ્વિગુણ v0, ans;
  દ્વિગુણ (*x)(દ્વિગુણ);
  પ્રતિજ્ઞા( argc==1 );
  type0 = sqlite3_કિંમત_numeric_type(argv[0]);
  જો( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) પાછા;
  v0 = sqlite3_કિંમત_double(argv[0]);
  x = (દ્વિગુણ(*)(દ્વિગુણ))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
સ્થિર શૂન્ય math2Func(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક type0, type1;
  દ્વિગુણ v0, v1, ans;
  દ્વિગુણ (*x)(દ્વિગુણ,દ્વિગુણ);
  પ્રતિજ્ઞા( argc==2 );
  type0 = sqlite3_કિંમત_numeric_type(argv[0]);
  જો( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) પાછા;
  type1 = sqlite3_કિંમત_numeric_type(argv[1]);
  જો( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) પાછા;
  v0 = sqlite3_કિંમત_double(argv[0]);
  v1 = sqlite3_કિંમત_double(argv[1]);
  x = (દ્વિગુણ(*)(દ્વિગુણ,દ્વિગુણ))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
સ્થિર શૂન્ય piFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પ્રતિજ્ઞા( argc==0 );
  (શૂન્ય)argv;
  sqlite3_result_double(context, M_PI);
}

#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
સ્થિર શૂન્ય signFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક type0;
  દ્વિગુણ x;
  UNUSED_PARAMETER(argc);
  પ્રતિજ્ઞા( argc==1 );
  type0 = sqlite3_કિંમત_numeric_type(argv[0]);
  જો( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) પાછા;
  x = sqlite3_કિંમત_double(argv[0]);
  sqlite3_result_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#જો defined(SQLITE_ENABLE_PERCENTILE)
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
પ્રકારવ્યાખ્યા સંરચના Percentile Percentile;
સંરચના Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  અક્ષર bSorted;        /* True if a[] is already in sorted order */
  અક્ષર bKeepSorted;    /* True if advantageous to keep a[] sorted */
  અક્ષર bPctValid;      /* True if rPct is valid */
  દ્વિગુણ rPct;         /* Fraction.  0.0 to 1.0 */
  દ્વિગુણ *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
સ્થિર પૂર્ણાંક percentIsInfinity(દ્વિગુણ r){
  sqlite3_uint64 u;
  પ્રતિજ્ઞા( કદ(u)==કદ(r) );
  memcpy(&u, &r, કદ(u));
  પાછા ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
સ્થિર પૂર્ણાંક percent_Same_કિંમત(દ્વિગુણ a, દ્વિગુણ b){
  a -= b;
  પાછા a>=-0.001 && a<=0.001;
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
સ્થિર i64 percent_દ્વિઅંકી_શોધ(Percentile *p, દ્વિગુણ y, પૂર્ણાંક bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  જ્યારે( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    દ્વિગુણ x = p->a[iMid];
    જો( x<y ){
      iFirst = iMid + 1;
    }નહીંતર જો( x>y ){
      iLast = iMid - 1;
    }નહીંતર{
      પાછા iMid;
    }
  }
  જો( bExact ) પાછા -1;
  પાછા iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
સ્થિર શૂન્ય percentError(sqlite3_context *pCtx, અચલ અક્ષર *zFormat, ...){
  અક્ષર *zMsg1;
  અક્ષર *zMsg2;
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
સ્થિર શૂન્ય percentStep(sqlite3_context *pCtx, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  Percentile *p;
  દ્વિગુણ rPct;
  પૂર્ણાંક eType;
  દ્વિગુણ y;
  પ્રતિજ્ઞા( argc==2 || argc==1 );

  જો( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }નહીંતર{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    દ્વિગુણ mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_કિંમત_numeric_type(argv[1]);
    rPct = sqlite3_કિંમત_double(argv[1])/mxFrac;
    જો( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (દ્વિગુણ)mxFrac);
      પાછા;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, કદ(*p));
  જો( p==0 ) પાછા;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  જો( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }નહીંતર જો( !percent_Same_કિંમત(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    પાછા;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_કિંમત_type(argv[0]);
  જો( eType==SQLITE_NULL ) પાછા;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  જો( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    પાછા;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_કિંમત_double(argv[0]);
  જો( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    પાછા;
  }

  /* Allocate and store the Y */
  જો( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    દ્વિગુણ *a = sqlite3_realloc64(p->a, કદ(દ્વિગુણ)*n);
    જો( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, કદ(*p));
      sqlite3_result_error_nomem(pCtx);
      પાછા;
    }
    p->nAlloc = n;
    p->a = a;
  }
  જો( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }નહીંતર જો( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }નહીંતર જો( p->bKeepSorted ){
    i64 i;
    i = percent_દ્વિઅંકી_શોધ(p, y, 0);
    જો( i<(પૂર્ણાંક)p->nUsed ){
      memmove(&p->a[i+1], &p->a[i], (p->nUsed-i)*કદ(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }નહીંતર{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#define SWAP_DOUBLE(X,Y)  {દ્વિગુણ ttt=(X);(X)=(Y);(Y)=ttt;}

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
સ્થિર શૂન્ય percentSort(
  દ્વિગુણ *a,                      /* Array to sort */
  unsigned પૂર્ણાંક n,                 /* Number of elements in array a[] */
  પૂર્ણાંક iReq                        /* Element caller cares about (or -ve) */
){
  પૂર્ણાંક iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  પૂર્ણાંક iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  પૂર્ણાંક i;         /* Loop counter */
  દ્વિગુણ rPivot; /* The pivot value */

  પ્રતિજ્ઞા( n>=2 );
  કરો{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    જો( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    જો( n==2 ) પાછા;
    iGt = n-1;
    i = n/2;
    જો( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }નહીંતર જો( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    જો( n==3 ) પાછા;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    કરો{
      જો( a[i]<rPivot ){
        જો( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }નહીંતર જો( a[i]>rPivot ){
        કરો{
          iGt--;
        }જ્યારે( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }નહીંતર{
        i++;
      }
    }જ્યારે( i<iGt );

    પ્રતિજ્ઞા( iLt>0 && iLt<iGt && (unsigned)iGt<n );
    testcase( iGt>iLt+1 );
    પ્રતિજ્ઞા( a[iLt]==rPivot );
    પ્રતિજ્ઞા( a[iLt-1]<=rPivot );
    પ્રતિજ્ઞા( a[iGt]>=rPivot );
    પ્રતિજ્ઞા( a[iLt+1]>=rPivot );

    જો( iReq>=0 ){
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
      જો( iReq<iLt ){
        n = iLt;
      }નહીંતર{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }નહીંતર{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      જો( iLt>(પૂર્ણાંક)(n/2) ){
        જો( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }નહીંતર{
        જો( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }જ્યારે( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
સ્થિર શૂન્ય percentInverse(sqlite3_context *pCtx,પૂર્ણાંક argc,sqlite3_કિંમત **argv){
  Percentile *p;
  પૂર્ણાંક eType;
  દ્વિગુણ y;
  i64 i;
  પ્રતિજ્ઞા( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, કદ(*p));
  પ્રતિજ્ઞા( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_કિંમત_type(argv[0]);
  જો( eType==SQLITE_NULL ) પાછા;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  જો( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    પાછા;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_કિંમત_double(argv[0]);
  જો( percentIsInfinity(y) ){
    પાછા;
  }
  જો( p->bSorted==0 ){
    પ્રતિજ્ઞા( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_દ્વિઅંકી_શોધ(p, y, 1);
  જો( i>=0 ){
    p->nUsed--;
    જો( i<(પૂર્ણાંક)p->nUsed ){
      memmove(&p->a[i], &p->a[i+1], (p->nUsed - i)*કદ(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
સ્થિર શૂન્ય percentCompute(sqlite3_context *pCtx, પૂર્ણાંક bIsFinal){
  Percentile *p;
  પૂર્ણાંક settings = SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&1; /* Discrete? */
  unsigned i1, i2;
  દ્વિગુણ v1, v2;
  દ્વિગુણ ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  જો( p==0 ) પાછા;
  જો( p->a==0 ) પાછા;
  જો( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (unsigned)ix;
    જો( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      પ્રતિજ્ઞા( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (પૂર્ણાંક)i1 : -1));
      p->bSorted = 1;
    }
    જો( settings & 1 ){
      vx = p->a[i1];
    }નહીંતર{
      i2 = ix==(દ્વિગુણ)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  જો( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, કદ(*p));
  }નહીંતર{
    p->bKeepSorted = 1;
  }
}
સ્થિર શૂન્ય percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
સ્થિર શૂન્ય percent_કિંમત(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#endif /* SQLITE_ENABLE_PERCENTILE */

#જો defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
સ્થિર શૂન્ય filestatFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  અચલ અક્ષર *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  જો( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    પૂર્ણાંક rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    પ્રતિજ્ઞા( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    જો( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }નહીંતર{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      જો( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      જો( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        જો( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }નહીંતર{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#જો defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
સ્થિર શૂન્ય walstatFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  અચલ અક્ષર *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  જો( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    પ્રતિજ્ઞા( pPager!=0 );
    pStr = sqlite3_str_new(db);
    જો( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }નહીંતર{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }નહીંતર{
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
સ્થિર શૂન્ય fpdecodeFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  FpDecode s;
  દ્વિગુણ x;
  પૂર્ણાંક y, z;
  અક્ષર zBuf[100];
  UNUSED_PARAMETER(argc);
  પ્રતિજ્ઞા( argc==3 );
  x = sqlite3_કિંમત_double(argv[0]);
  y = sqlite3_કિંમત_int(argv[1]);
  z = sqlite3_કિંમત_int(argv[2]);
  જો( z<=0 ) z = 1;
  જો( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  જો( s.isSpecial==2 ){
    sqlite3_snprintf(કદ(zBuf), zBuf, "NaN");
  }નહીંતર{
    sqlite3_snprintf(કદ(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
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
સ્થિર શૂન્ય parseuriFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3_str *pResult;
  અચલ અક્ષર *zVfs;
  અચલ અક્ષર *zUri;
  unsigned પૂર્ણાંક flgs;
  પૂર્ણાંક rc;
  sqlite3_vfs *pVfs = 0;
  અક્ષર *zFile = 0;
  અક્ષર *zErr = 0;

  જો( argc<2 ) પાછા;
  pVfs = sqlite3_vfs_find(0);
  પ્રતિજ્ઞા( pVfs );
  zVfs = pVfs->zName;
  zUri = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  જો( zUri==0 ) પાછા;
  flgs = (unsigned પૂર્ણાંક)sqlite3_કિંમત_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  pResult = sqlite3_str_new(0);
  જો( !sqlite3_str_errcode(pResult) ){
    પૂર્ણાંક i;
    sqlite3_str_appendf(pResult, "rc=%d", rc);
    sqlite3_str_appendf(pResult, ", flags=0x%x", flgs);
    sqlite3_str_appendf(pResult, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(pResult, ", err=%Q", zErr);
    sqlite3_str_appendf(pResult, ", file=%Q", zFile);
    જો( zFile ){
      અચલ અક્ષર *z = zFile;
      z += sqlite3Strlen30(z)+1;
      જ્યારે( z[0] ){
        sqlite3_str_appendf(pResult, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      માટે(i=2; i<argc; i++){
        અચલ અક્ષર *zArg;
        જો( sqlite3_કિંમત_type(argv[i])==SQLITE_INTEGER ){
          પૂર્ણાંક k = sqlite3_કિંમત_int(argv[i]);
          sqlite3_str_appendf(pResult, ", '%d:%q'",k,sqlite3_uri_ચાવી(zFile, k));
        }નહીંતર જો( (zArg = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]))!=0 ){
          sqlite3_str_appendf(pResult, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }નહીંતર{
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
શૂન્ય sqlite3RegisterBuiltinFunctions(શૂન્ય){
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
  સ્થિર FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#જો !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(પદાવલિ_compare,        2, INLINEFUNC_પદાવલિ_compare,        0),
    TEST_FUNC(પદાવલિ_implies_પદાવલિ,   2, INLINEFUNC_પદાવલિ_implies_પદાવલિ,   0),
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
#જો defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_કિંમત, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_કિંમત, 0,
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
    FUNCTION(અક્ષર,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifdef SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#endif
#જો defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
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
    VFUNCTION(last_ઉમેરો_rowid, 0, 0, 0, last_ઉમેરો_rowid),
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
    WAGGREGATE(ગણતરી, 0,0,0, ગણતરી_Step,
        ગણતરી_Finalize, ગણતરી_Finalize, ગણતરી_Inverse,
        SQLITE_FUNC_ગણતરી|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(ગણતરી, 1,0,0, ગણતરી_Step,
        ગણતરી_Finalize, ગણતરી_Finalize, ગણતરી_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_કિંમત, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_કિંમત, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_કિંમત, groupConcatInverse, 0),

#ifdef SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_કિંમત, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_કિંમત, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_કિંમત, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_કિંમત, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#endif /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#નહીંતર
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
#જો SQLITE_HAVE_C99_MATH_FUNCS
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
#જો SQLITE_HAVE_C99_MATH_FUNCS
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
    INLINE_FUNC(જો,             -4, INLINEFUNC_iif,      0 ),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_ઉમેરો_Builtin_Funcs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#જો 0  /* Enable to print out how the built-in functions are hashed */
  {
    પૂર્ણાંક i;
    FuncDef *p;
    માટે(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      માટે(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        પૂર્ણાંક n = sqlite3Strlen30(p->zName);
        પૂર્ણાંક h = p->zName[0] + n;
        પ્રતિજ્ઞા( p->funcFlags & SQLITE_FUNC_BUILTIN );
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}
