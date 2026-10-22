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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <assert.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#அந்தர்பாவ <math.h>
#endif
#அந்தர்பாவ "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
நிலையான CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  ப்ரதிஜ்ஞா( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  ப்ரதிஜ்ஞா( pOp->opcode==OP_CollSeq );
  ப்ரதிஜ்ஞா( pOp->p4type==P4_COLLSEQ );
  திரும்பு pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
நிலையான வெறுமை sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  ப்ரதிஜ்ஞா( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
நிலையான வெறுமை minmaxFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு i;
  முழு mask;    /* 0 for min() or 0xffffffff for max() */
  முழு iBest;
  CollSeq *pColl;

  ப்ரதிஜ்ஞா( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  ப்ரதிஜ்ஞா( pColl );
  ப்ரதிஜ்ஞா( mask==-1 || mask==0 );
  iBest = 0;
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ) திரும்பு;
  க்கு(i=1; i<argc; i++){
    எனில்( sqlite3_மதிப்பு_type(argv[i])==SQLITE_NULL ) திரும்பு;
    எனில்( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_மதிப்பு(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
நிலையான வெறுமை typeofFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **argv
){
  நிலையான மாறிலி எழுத்து *azType[] = { "integer", "real", "text", "blob", "null" };
  முழு i = sqlite3_மதிப்பு_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  ப்ரதிஜ்ஞா( i>=0 && i<ArraySize(azType) );
  ப்ரதிஜ்ஞா( SQLITE_INTEGER==1 );
  ப்ரதிஜ்ஞா( SQLITE_FLOAT==2 );
  ப்ரதிஜ்ஞா( SQLITE_TEXT==3 );
  ப்ரதிஜ்ஞா( SQLITE_BLOB==4 );
  ப்ரதிஜ்ஞா( SQLITE_NULL==5 );
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
நிலையான வெறுமை subtypeFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_result_int(context, sqlite3_மதிப்பு_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
நிலையான வெறுமை lengthFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  தேர்வு( sqlite3_மதிப்பு_type(argv[0]) ){
    நிலை SQLITE_BLOB:
    நிலை SQLITE_INTEGER:
    நிலை SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_மதிப்பு_bytes(argv[0]));
      நிறுத்து;
    }
    நிலை SQLITE_TEXT: {
      மாறிலி unsigned எழுத்து *z = sqlite3_மதிப்பு_text(argv[0]);
      மாறிலி unsigned எழுத்து *z0;
      எனில்( z==0 ) திரும்பு;
      z0 = z;
      போது( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        எனில்( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }இல்லையெனில் எனில்( z[0]==0 ){
          நிறுத்து;
        }இல்லையெனில்{
          z++;
          போது( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_result_int(context, (முழு)(z-z0));
      நிறுத்து;
    }
    இயல்பு: {
      sqlite3_result_null(context);
      நிறுத்து;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
நிலையான வெறுமை bytelengthFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  தேர்வு( sqlite3_மதிப்பு_type(argv[0]) ){
    நிலை SQLITE_BLOB: {
      sqlite3_result_int(context, sqlite3_மதிப்பு_bytes(argv[0]));
      நிறுத்து;
    }
    நிலை SQLITE_INTEGER:
    நிலை SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_result_int64(context, sqlite3_மதிப்பு_bytes(argv[0])*m);
      நிறுத்து;
    }
    நிலை SQLITE_TEXT: {
      எனில்( sqlite3_மதிப்பு_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_result_int(context, sqlite3_மதிப்பு_bytes(argv[0]));
      }இல்லையெனில்{
        sqlite3_result_int(context, sqlite3_மதிப்பு_bytes16(argv[0]));
      }
      நிறுத்து;
    }
    இயல்பு: {
      sqlite3_result_null(context);
      நிறுத்து;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
நிலையான வெறுமை absFunc(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  தேர்வு( sqlite3_மதிப்பு_type(argv[0]) ){
    நிலை SQLITE_INTEGER: {
      i64 iVal = sqlite3_மதிப்பு_int64(argv[0]);
      எனில்( iVal<0 ){
        எனில்( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          திரும்பு;
        }
        iVal = -iVal;
      }
      sqlite3_result_int64(context, iVal);
      நிறுத்து;
    }
    நிலை SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      நிறுத்து;
    }
    இயல்பு: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      இரட்டை rVal = sqlite3_மதிப்பு_double(argv[0]);
      எனில்( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      நிறுத்து;
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
நிலையான வெறுமை instrFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி unsigned எழுத்து *zHaystack;
  மாறிலி unsigned எழுத்து *zNeedle;
  முழு nHaystack;
  முழு nNeedle;
  முழு typeHaystack, typeNeedle;
  முழு N = 1;
  முழு isText;
  unsigned எழுத்து firstChar;
  sqlite3_மதிப்பு *pC1 = 0;
  sqlite3_மதிப்பு *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_மதிப்பு_type(argv[0]);
  typeNeedle = sqlite3_மதிப்பு_type(argv[1]);
  எனில்( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) திரும்பு;
  nHaystack = sqlite3_மதிப்பு_bytes(argv[0]);
  nNeedle = sqlite3_மதிப்பு_bytes(argv[1]);
  எனில்( nNeedle>0 ){
    எனில்( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_மதிப்பு_blob(argv[0]);
      zNeedle = sqlite3_மதிப்பு_blob(argv[1]);
      isText = 0;
    }இல்லையெனில் எனில்( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_மதிப்பு_text(argv[0]);
      zNeedle = sqlite3_மதிப்பு_text(argv[1]);
      isText = 1;
    }இல்லையெனில்{
      pC1 = sqlite3_மதிப்பு_dup(argv[0]);
      zHaystack = sqlite3_மதிப்பு_text(pC1);
      எனில்( zHaystack==0 ) செல் endInstrOOM;
      nHaystack = sqlite3_மதிப்பு_bytes(pC1);
      pC2 = sqlite3_மதிப்பு_dup(argv[1]);
      zNeedle = sqlite3_மதிப்பு_text(pC2);
      எனில்( zNeedle==0 ) செல் endInstrOOM;
      nNeedle = sqlite3_மதிப்பு_bytes(pC2);
      isText = 1;
    }
    எனில்( zNeedle==0 || (nHaystack && zHaystack==0) ) செல் endInstrOOM;
    firstChar = zNeedle[0];
    போது( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      செய்{
        nHaystack--;
        zHaystack++;
      }போது( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    எனில்( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_மதிப்பு_free(pC1);
  sqlite3_மதிப்பு_free(pC2);
  திரும்பு;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  செல் endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
நிலையான வெறுமை printfFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  PrintfArguments x;
  StrAccum str;
  மாறிலி எழுத்து *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  எனில்( argc>=1 && (zFormat = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]))!=0 ){
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
நிலையான SQLITE_INLINE முழு containsNulOrNonAscii(u64 word){
  மாறிலி u64 byteOnes = UINT64_C(0x0101010101010101);
  மாறிலி u64 byte_உயர்வு_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  திரும்பு ((word | (word - byteOnes)) & byte_உயர்வு_Bits)!=0;
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
நிலையான வெறுமை substrFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி unsigned எழுத்து *z;
  மாறிலி unsigned எழுத்து *z2;
  முழு len;
  முழு p0type;
  i64 p1, p2;

  ப்ரதிஜ்ஞா( argc==3 || argc==2 );
  p0type = sqlite3_மதிப்பு_type(argv[0]);
  p1 = sqlite3_மதிப்பு_int64(argv[1]);
  எனில்( p0type==SQLITE_BLOB ){
    len = sqlite3_மதிப்பு_bytes(argv[0]);
    z = sqlite3_மதிப்பு_blob(argv[0]);
    எனில்( z==0 ) திரும்பு;
    ப்ரதிஜ்ஞா( len==sqlite3_மதிப்பு_bytes(argv[0]) );
  }இல்லையெனில்{
    z = sqlite3_மதிப்பு_text(argv[0]);
    எனில்( z==0 ) திரும்பு;
    len = 0;
    எனில்( p1<0 ){
      க்கு(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  எனில்( argc==3 ){
    p2 = sqlite3_மதிப்பு_int64(argv[2]);
    எனில்( p2==0 && sqlite3_மதிப்பு_type(argv[2])==SQLITE_NULL ) திரும்பு;
  }இல்லையெனில்{
    p2 = LARGEST_INT64;
  }
  எனில்( p1==0 ){
#ifdef SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#endif
    எனில்( sqlite3_மதிப்பு_type(argv[1])==SQLITE_NULL ) திரும்பு;
  }
  எனில்( p1<0 ){
    p1 += len;
    எனில்( p1<0 ){
      எனில்( p2<0 ){
        p2 = 0;
      }இல்லையெனில்{
        p2 += p1;
      }
      p1 = 0;
    }
  }இல்லையெனில் எனில்( p1>0 ){
    p1--;
  }இல்லையெனில் எனில்( p2>0 ){
    p2--;
  }
  எனில்( p2<0 ){
    எனில்( p2<-p1 ){
      p2 = p1;
    }இல்லையெனில்{
      p2 = -p2;
    }
    p1 -= p2;
  }
  ப்ரதிஜ்ஞா( p1>=0 && p2>=0 );
  எனில்( p0type!=SQLITE_BLOB ){
    எனில்( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      மாறிலி unsigned எழுத்து *zEnd = z + sqlite3_மதிப்பு_bytes(argv[0]);
      போது( p1>=8 && zEnd-z>=8 ){
        u64 word;
        memcpy(&word, z, அளவு(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        எனில்( containsNulOrNonAscii(word) ){
          நிறுத்து;
        }
        z += 8;
        p1 -= 8;
      }
    }
    க்கு( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      எனில்( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }இல்லையெனில் எனில்( z[0]==0 ){
        நிறுத்து;
      }இல்லையெனில்{
        செய்{ z++; }போது( (z[0]&0xc0)==0x80 );
      }
    }
    க்கு(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      எனில்( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }இல்லையெனில் எனில்( z2[0]==0 ){
        நிறுத்து;
      }இல்லையெனில்{
        செய்{ z2++; }போது( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_result_text64(context, (எழுத்து*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }இல்லையெனில்{
    எனில்( p1>=len ){
      p1 = p2 = 0;
    }இல்லையெனில் எனில்( p2>len-p1 ){
      p2 = len-p1;
      ப்ரதிஜ்ஞா( p2>0 );
    }
    sqlite3_result_blob64(context, (எழுத்து*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
நிலையான வெறுமை roundFunc(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  i64 n = 0;
  இரட்டை r;
  எழுத்து *zBuf;
  ப்ரதிஜ்ஞா( argc==1 || argc==2 );
  எனில்( argc==2 ){
    எனில்( SQLITE_NULL==sqlite3_மதிப்பு_type(argv[1]) ) திரும்பு;
    n = sqlite3_மதிப்பு_int64(argv[1]);
    எனில்( n>30 ) n = 30;
    எனில்( n<0 ) n = 0;
  }
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ) திரும்பு;
  r = sqlite3_மதிப்பு_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  எனில்( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }இல்லையெனில் எனில்( n==0 ){ 
    r = (இரட்டை)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }இல்லையெனில்{
    zBuf = sqlite3_mprintf("%!.*f",(முழு)n,r);
    எனில்( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      திரும்பு;
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
நிலையான வெறுமை *contextMalloc(sqlite3_context *context, i64 nByte){
  எழுத்து *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ப்ரதிஜ்ஞா( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  எனில்( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }இல்லையெனில்{
    z = sqlite3Malloc(nByte);
    எனில்( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  திரும்பு z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
நிலையான வெறுமை upperFunc(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  எழுத்து *z1;
  மாறிலி எழுத்து *z2;
  முழு i, n;
  UNUSED_PARAMETER(argc);
  z2 = (எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  n = sqlite3_மதிப்பு_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ப்ரதிஜ்ஞா( z2==(எழுத்து*)sqlite3_மதிப்பு_text(argv[0]) );
  எனில்( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    எனில்( z1 ){
      க்கு(i=0; i<n; i++){
        z1[i] = (எழுத்து)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
நிலையான வெறுமை lowerFunc(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  எழுத்து *z1;
  மாறிலி எழுத்து *z2;
  முழு i, n;
  UNUSED_PARAMETER(argc);
  z2 = (எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  n = sqlite3_மதிப்பு_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ப்ரதிஜ்ஞா( z2==(எழுத்து*)sqlite3_மதிப்பு_text(argv[0]) );
  எனில்( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    எனில்( z1 ){
      க்கு(i=0; i<n; i++){
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
நிலையான வெறுமை randomFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(அளவு(r), &r);
  எனில்( r<0 ){
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
நிலையான வெறுமை randomBlob(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  sqlite3_int64 n;
  unsigned எழுத்து *p;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_மதிப்பு_int64(argv[0]);
  எனில்( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  எனில்( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (எழுத்து*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
நிலையான வெறுமை last_செருகு_rowid(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_செருகு_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
நிலையான வெறுமை changes(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
நிலையான வெறுமை total_changes(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **NotUsed2
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
கட்டமைப்பு compareInfo {
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
#எனில் defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#இல்லையெனில்
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

நிலையான மாறிலி கட்டமைப்பு compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
நிலையான மாறிலி கட்டமைப்பு compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
நிலையான மாறிலி கட்டமைப்பு compareInfo likeInfoAlt = { '%', '_',   0, 0 };

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
நிலையான முழு patternCompare(
  மாறிலி u8 *zPattern,              /* The glob pattern */
  மாறிலி u8 *zString,               /* The string to compare against the glob */
  மாறிலி கட்டமைப்பு compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  மாறிலி u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  போது( (c = Utf8Read(zPattern))!=0 ){
    எனில்( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      போது( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        எனில்( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          திரும்பு SQLITE_NOWILDCARDMATCH;
        }
      }
      எனில்( c==0 ){
        திரும்பு SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }இல்லையெனில் எனில்( c==matchOther ){
        எனில்( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          எனில்( c==0 ) திரும்பு SQLITE_NOWILDCARDMATCH;
        }இல்லையெனில்{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          ப்ரதிஜ்ஞா( matchOther<0x80 );  /* '[' is a single-byte character */
          போது( *zString ){
            முழு bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            எனில்( bMatch!=SQLITE_NOMATCH ) திரும்பு bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          திரும்பு SQLITE_NOWILDCARDMATCH;
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
      எனில்( c<0x80 ){
        எழுத்து zStop[3];
        முழு bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        எனில்( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        எனில்( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }இல்லையெனில்{
          zStop[0] = c;
          zStop[1] = 0;
        }
        போது(1){
          zString += strcspn((மாறிலி எழுத்து*)zString, zStop);
          எனில்( zString[0]==0 ) நிறுத்து;
          zString++;
          எனில்( nx && zString[0]!=nx && zString[0]!=ny ) தொடர்;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          எனில்( bMatch!=SQLITE_NOMATCH ) திரும்பு bMatch;
        }
      }இல்லையெனில்{
        முழு bMatch;
        போது( (c2 = Utf8Read(zString))!=0 ){
          எனில்( c2!=c ) தொடர்;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          எனில்( bMatch!=SQLITE_NOMATCH ) திரும்பு bMatch;
        }
      }
      திரும்பு SQLITE_NOWILDCARDMATCH;
    }
    எனில்( c==matchOther ){
      எனில்( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        எனில்( c==0 ) திரும்பு SQLITE_NOMATCH;
        zEscaped = zPattern;
      }இல்லையெனில்{
        u32 prior_c = 0;
        முழு seen = 0;
        முழு invert = 0;
        c = sqlite3Utf8Read(&zString);
        எனில்( c==0 ) திரும்பு SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        எனில்( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        எனில்( c2==']' ){
          எனில்( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        போது( c2 && c2!=']' ){
          எனில்( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            எனில்( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }இல்லையெனில்{
            எனில்( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        எனில்( c2==0 || (seen ^ invert)==0 ){
          திரும்பு SQLITE_NOMATCH;
        }
        தொடர்;
      }
    }
    c2 = Utf8Read(zString);
    எனில்( c==c2 ) தொடர்;
    எனில்( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      தொடர்;
    }
    எனில்( c==matchOne && zPattern!=zEscaped && c2!=0 ) தொடர்;
    திரும்பு SQLITE_NOMATCH;
  }
  திரும்பு *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
முழு sqlite3_strglob(மாறிலி எழுத்து *zGlobPattern, மாறிலி எழுத்து *zString){
  எனில்( zString==0 ){
    திரும்பு zGlobPattern!=0;
  }இல்லையெனில் எனில்( zGlobPattern==0 ){
    திரும்பு 1;
  }இல்லையெனில் {
    திரும்பு patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
முழு sqlite3_strlike(மாறிலி எழுத்து *zPattern, மாறிலி எழுத்து *zStr, unsigned முழு esc){
  எனில்( zStr==0 ){
    திரும்பு zPattern!=0;
  }இல்லையெனில் எனில்( zPattern==0 ){
    திரும்பு 1;
  }இல்லையெனில்{
    திரும்பு patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
முழு sqlite3_like_மொத்தம் = 0;
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
நிலையான வெறுமை likeFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி unsigned எழுத்து *zA, *zB;
  u32 escape;
  முழு nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  கட்டமைப்பு compareInfo *pInfo = sqlite3_user_data(context);
  கட்டமைப்பு compareInfo backupInfo;

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_BLOB
   || sqlite3_மதிப்பு_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_மொத்தம்++;
#endif
    sqlite3_result_int(context, 0);
    திரும்பு;
  }
#endif

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_மதிப்பு_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  எனில்( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    திரும்பு;
  }
  எனில்( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    மாறிலி unsigned எழுத்து *zEsc = sqlite3_மதிப்பு_text(argv[2]);
    எனில்( zEsc==0 ) திரும்பு;
    எனில்( sqlite3Utf8CharLen((எழுத்து*)zEsc, -1)!=1 ){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      திரும்பு;
    }
    escape = sqlite3Utf8Read(&zEsc);
    எனில்( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      memcpy(&backupInfo, pInfo, அளவு(backupInfo));
      pInfo = &backupInfo;
      எனில்( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      எனில்( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }இல்லையெனில்{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_மதிப்பு_text(argv[0]);
  zA = sqlite3_மதிப்பு_text(argv[1]);
  எனில்( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_மொத்தம்++;
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
நிலையான வெறுமை nullifFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  எனில்( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_மதிப்பு(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
நிலையான வெறுமை versionFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **NotUsed2
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
நிலையான வெறுமை sourceidFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **NotUsed2
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
நிலையான வெறுமை errlogFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_மதிப்பு_int(argv[0]), "%s", sqlite3_மதிப்பு_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
நிலையான வெறுமை compileoptionusedFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி எழுத்து *zOptName;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  எனில்( (zOptName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]))!=0 ){
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
நிலையான வெறுமை compileoptiongetFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  i64 n;
  மாறிலி எழுத்து *zRes;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_மதிப்பு_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((முழு)n) : 0;
  sqlite3_result_text(context, zRes, -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
நிலையான மாறிலி எழுத்து hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
வெறுமை sqlite3_Quote_மதிப்பு(StrAccum *pStr, sqlite3_மதிப்பு *p_மதிப்பு, முழு bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  ப்ரதிஜ்ஞா( pStr!=0 && pStr->nChar==0 );

  தேர்வு( sqlite3_மதிப்பு_type(p_மதிப்பு) ){
    நிலை SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_மதிப்பு_double(p_மதிப்பு));
      நிறுத்து;
    }
    நிலை SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_மதிப்பு_int64(p_மதிப்பு));
      நிறுத்து;
    }
    நிலை SQLITE_BLOB: {
      எழுத்து மாறிலி *zBlob = sqlite3_மதிப்பு_blob(p_மதிப்பு);
      i64 nBlob = sqlite3_மதிப்பு_bytes(p_மதிப்பு);
      ப்ரதிஜ்ஞா( zBlob==sqlite3_மதிப்பு_blob(p_மதிப்பு) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      எனில்( pStr->accError==0 ){
        எழுத்து *zText = pStr->zText;
        முழு i;
        க்கு(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      நிறுத்து;
    }
    நிலை SQLITE_TEXT: {
      மாறிலி unsigned எழுத்து *zArg = sqlite3_மதிப்பு_text(p_மதிப்பு);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      நிறுத்து;
    }
    இயல்பு: {
      ப்ரதிஜ்ஞா( sqlite3_மதிப்பு_type(p_மதிப்பு)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      நிறுத்து;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
நிலையான முழு isNHex(மாறிலி எழுத்து *z, முழு N, u32 *pVal){
  முழு i;
  u32 v = 0;
  க்கு(i=0; i<N; i++){
    எனில்( !sqlite3Isxdigit(z[i]) ) திரும்பு 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  திரும்பு 1;
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
நிலையான வெறுமை unistrFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  எழுத்து *zOut;
  மாறிலி எழுத்து *zIn;
  முழு nIn;
  முழு i, j, n;
  u32 v;

  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  எனில்( zIn==0 ) திரும்பு;
  nIn = sqlite3_மதிப்பு_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  எனில்( zOut==0 ){
    sqlite3_result_error_nomem(context);
    திரும்பு;
  }
  i = j = 0;
  போது( i<nIn ){
    மாறிலி எழுத்து *z = strchr(&zIn[i],'\\');
    எனில்( z==0 ){
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      நிறுத்து;
    }
    n = z - &zIn[i];
    எனில்( n>0 ){
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    எனில்( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }இல்லையெனில் எனில்( sqlite3Isxdigit(zIn[i+1]) ){
      எனில்( !isNHex(&zIn[i+1], 4, &v) ) செல் unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }இல்லையெனில் எனில்( zIn[i+1]=='+' ){
      எனில்( !isNHex(&zIn[i+2], 6, &v) ) செல் unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }இல்லையெனில் எனில்( zIn[i+1]=='u' ){
      எனில்( !isNHex(&zIn[i+2], 4, &v) ) செல் unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }இல்லையெனில் எனில்( zIn[i+1]=='U' ){
      எனில்( !isNHex(&zIn[i+2], 8, &v) ) செல் unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }இல்லையெனில்{
      செல் unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  திரும்பு;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  திரும்பு;
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
நிலையான வெறுமை quoteFunc(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  sqlite3_Quote_மதிப்பு(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_data(context)));
  sqlite3_result_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
நிலையான வெறுமை unicodeFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி unsigned எழுத்து *z = sqlite3_மதிப்பு_text(argv[0]);
  (வெறுமை)argc;
  எனில்( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
நிலையான வெறுமை charFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  unsigned எழுத்து *z, *zOut;
  முழு i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  எனில்( z==0 ){
    sqlite3_result_error_nomem(context);
    திரும்பு;
  }
  க்கு(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_மதிப்பு_int64(argv[i]);
    எனில்( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    எனில்( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }இல்லையெனில் எனில்( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }இல்லையெனில் எனில்( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }இல்லையெனில்{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_result_text64(context, (எழுத்து*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
நிலையான வெறுமை hexFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு i, n;
  மாறிலி unsigned எழுத்து *pBlob;
  எழுத்து *zHex, *z;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_மதிப்பு_blob(argv[0]);
  n = sqlite3_மதிப்பு_bytes(argv[0]);
  ப்ரதிஜ்ஞா( pBlob==sqlite3_மதிப்பு_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  எனில்( zHex ){
    க்கு(i=0; i<n; i++, pBlob++){
      unsigned எழுத்து c = *pBlob;
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
நிலையான முழு strContainsChar(மாறிலி u8 *zStr, முழு nStr, u32 ch){
  மாறிலி u8 *zEnd = &zStr[nStr];
  மாறிலி u8 *z = zStr;
  போது( z<zEnd ){
    u32 tst = Utf8Read(z);
    எனில்( tst==ch ) திரும்பு 1;
  }
  திரும்பு 0;
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
நிலையான வெறுமை unhexFunc(
  sqlite3_context *pCtx,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி u8 *zPass = (மாறிலி u8*)"";
  முழு nPass = 0;
  மாறிலி u8 *zHex = sqlite3_மதிப்பு_text(argv[0]);
  முழு nHex = sqlite3_மதிப்பு_bytes(argv[0]);
#ifdef SQLITE_DEBUG
  மாறிலி u8 *zEnd = zHex ? &zHex[nHex] : 0;
#endif
  u8 *pBlob = 0;
  u8 *p = 0;

  ப்ரதிஜ்ஞா( argc==1 || argc==2 );
  எனில்( argc==2 ){
    zPass = sqlite3_மதிப்பு_text(argv[1]);
    nPass = sqlite3_மதிப்பு_bytes(argv[1]);
  }
  எனில்( !zHex || !zPass ) திரும்பு;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  எனில்( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    போது( (c = *zHex)!=0x00 ){
      போது( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        ப்ரதிஜ்ஞா( zHex<=zEnd );
        எனில்( !strContainsChar(zPass, nPass, ch) ) செல் unhex_null;
        c = *zHex;
        எனில்( c==0x00 ) செல் unhex_done;
      }
      zHex++;
      ப்ரதிஜ்ஞா( *zEnd==0x00 );
      ப்ரதிஜ்ஞா( zHex<=zEnd );
      d = *(zHex++);
      எனில்( !sqlite3Isxdigit(d) ) செல் unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  திரும்பு;

 unhex_null:
  sqlite3_free(pBlob);
  திரும்பு;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
நிலையான வெறுமை zeroblobFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  i64 n;
  முழு rc;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_மதிப்பு_int64(argv[0]);
  எனில்( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  எனில்( rc ){
    sqlite3_result_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
நிலையான வெறுமை replaceFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி unsigned எழுத்து *zStr;        /* The input string A */
  மாறிலி unsigned எழுத்து *zPattern;    /* The pattern string B */
  மாறிலி unsigned எழுத்து *zRep;        /* The replacement string C */
  unsigned எழுத்து *zOut;              /* The output */
  முழு nStr;                /* Size of zStr */
  முழு nPattern;            /* Size of zPattern */
  முழு nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  முழு loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  unsigned cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  ப்ரதிஜ்ஞா( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_மதிப்பு_text(argv[0]);
  எனில்( zStr==0 ) திரும்பு;
  nStr = sqlite3_மதிப்பு_bytes(argv[0]);
  ப்ரதிஜ்ஞா( zStr==sqlite3_மதிப்பு_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_மதிப்பு_text(argv[1]);
  எனில்( zPattern==0 ){
    ப்ரதிஜ்ஞா( sqlite3_மதிப்பு_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    திரும்பு;
  }
  எனில்( zPattern[0]==0 ){
    ப்ரதிஜ்ஞா( sqlite3_மதிப்பு_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_text(context, (மாறிலி எழுத்து*)zStr, nStr, SQLITE_TRANSIENT);
    திரும்பு;
  }
  nPattern = sqlite3_மதிப்பு_bytes(argv[1]);
  ப்ரதிஜ்ஞா( zPattern==sqlite3_மதிப்பு_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_மதிப்பு_text(argv[2]);
  எனில்( zRep==0 ) திரும்பு;
  nRep = sqlite3_மதிப்பு_bytes(argv[2]);
  ப்ரதிஜ்ஞா( zRep==sqlite3_மதிப்பு_text(argv[2]) );
  nOut = nStr + 1;
  ப்ரதிஜ்ஞா( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, nOut);
  எனில்( zOut==0 ){
    திரும்பு;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  க்கு(i=j=0; i<=loopLimit; i++){
    எனில்( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }இல்லையெனில்{
      எனில்( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
        எனில்( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          திரும்பு;
        }
        cntExpand++;
        எனில்( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (முழு)nOut + (nOut - nStr - 1));
          எனில்( zOut==0 ){
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            திரும்பு;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  ப்ரதிஜ்ஞா( j+nStr-i+1<=nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  ப்ரதிஜ்ஞா( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (எழுத்து*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
நிலையான வெறுமை trimFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி unsigned எழுத்து *zIn;         /* Input string */
  மாறிலி unsigned எழுத்து *zCharSet;    /* Set of characters to trim */
  unsigned முழு nIn;                 /* Number of bytes in input */
  முழு flags;                        /* 1: trimleft  2: trimright  3: trim */
  முழு i;                            /* Loop counter */
  unsigned முழு *aLen = 0;           /* Length of each character in zCharSet */
  unsigned எழுத்து **azChar = 0;       /* Individual characters in zCharSet */
  முழு nChar;                        /* Number of characters in zCharSet */

  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ){
    திரும்பு;
  }
  zIn = sqlite3_மதிப்பு_text(argv[0]);
  எனில்( zIn==0 ) திரும்பு;
  nIn = (unsigned)sqlite3_மதிப்பு_bytes(argv[0]);
  ப்ரதிஜ்ஞா( zIn==sqlite3_மதிப்பு_text(argv[0]) );
  எனில்( argc==1 ){
    நிலையான மாறிலி unsigned lenOne[] = { 1 };
    நிலையான unsigned எழுத்து * மாறிலி azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (unsigned*)lenOne;
    azChar = (unsigned எழுத்து **)azOne;
    zCharSet = 0;
  }இல்லையெனில் எனில்( (zCharSet = sqlite3_மதிப்பு_text(argv[1]))==0 ){
    திரும்பு;
  }இல்லையெனில்{
    மாறிலி unsigned எழுத்து *z;
    க்கு(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    எனில்( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(அளவு(எழுத்து*)+அளவு(unsigned)));
      எனில்( azChar==0 ){
        திரும்பு;
      }
      aLen = (unsigned*)&azChar[nChar];
      க்கு(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned எழுத்து *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  எனில்( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    எனில்( flags & 1 ){
      போது( nIn>0 ){
        unsigned முழு len = 0;
        க்கு(i=0; i<nChar; i++){
          len = aLen[i];
          எனில்( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) நிறுத்து;
        }
        எனில்( i>=nChar ) நிறுத்து;
        zIn += len;
        nIn -= len;
      }
    }
    எனில்( flags & 2 ){
      போது( nIn>0 ){
        unsigned முழு len = 0;
        க்கு(i=0; i<nChar; i++){
          len = aLen[i];
          எனில்( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) நிறுத்து;
        }
        எனில்( i>=nChar ) நிறுத்து;
        nIn -= len;
      }
    }
    எனில்( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (எழுத்து*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
நிலையான வெறுமை concatFuncCore(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv,
  முழு nSep,
  மாறிலி எழுத்து *zSep
){
  i64 j, n = 0;
  முழு i;
  முழு bNotNull = 0;   /* True after at least NOT NULL argument seen */
  எழுத்து *z;
  க்கு(i=0; i<argc; i++){
    n += sqlite3_மதிப்பு_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  எனில்( z==0 ){
    sqlite3_result_error_nomem(context);
    திரும்பு;
  }
  j = 0;
  க்கு(i=0; i<argc; i++){
    எனில்( sqlite3_மதிப்பு_type(argv[i])!=SQLITE_NULL ){
      முழு k = sqlite3_மதிப்பு_bytes(argv[i]);
      மாறிலி எழுத்து *v = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
      எனில்( v!=0 ){
        எனில்( bNotNull && nSep>0 ){
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
  ப்ரதிஜ்ஞா( j<=n );
  sqlite3_result_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
நிலையான வெறுமை concatFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
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
நிலையான வெறுமை concatwsFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு nSep = sqlite3_மதிப்பு_bytes(argv[0]);
  மாறிலி எழுத்து *zSep = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  எனில்( zSep==0 ) திரும்பு;
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
நிலையான வெறுமை unknownFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  /* no-op */
  (வெறுமை)context;
  (வெறுமை)argc;
  (வெறுமை)argv;
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
நிலையான வெறுமை soundexFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  எழுத்து zResult[8];
  மாறிலி u8 *zIn;
  முழு i, j;
  நிலையான மாறிலி unsigned எழுத்து iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  ப்ரதிஜ்ஞா( argc==1 );
  zIn = (u8*)sqlite3_மதிப்பு_text(argv[0]);
  எனில்( zIn==0 ) zIn = (u8*)"";
  க்கு(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  எனில்( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    க்கு(j=1; j<4 && zIn[i]; i++){
      முழு code = iCode[zIn[i]&0x7f];
      எனில்( code>0 ){
        எனில்( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }இல்லையெனில்{
        prevcode = 0;
      }
    }
    போது( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }இல்லையெனில்{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
நிலையான வெறுமை oomFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#endif /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
நிலையான வெறுமை loadExt(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  மாறிலி எழுத்து *zFile = (மாறிலி எழுத்து *)sqlite3_மதிப்பு_text(argv[0]);
  மாறிலி எழுத்து *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  எழுத்து *z_Err_செய்தி = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  எனில்( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    திரும்பு;
  }

  எனில்( argc==2 ){
    zProc = (மாறிலி எழுத்து *)sqlite3_மதிப்பு_text(argv[1]);
  }இல்லையெனில்{
    zProc = 0;
  }
  எனில்( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_செய்தி) ){
    sqlite3_result_error(context, z_Err_செய்தி, -1);
    sqlite3_free(z_Err_செய்தி);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
வகைவரை கட்டமைப்பு SumCtx SumCtx;
கட்டமைப்பு SumCtx {
  இரட்டை rSum;      /* Running sum as as a double */
  இரட்டை rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
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
நிலையான வெறுமை kahanBabuskaNeumaierStep(
  நிலையற்ற SumCtx *pSum,
  நிலையற்ற இரட்டை r
){
  நிலையற்ற இரட்டை s = pSum->rSum;
  நிலையற்ற இரட்டை t = s + r;
  எனில்( fabs(s) > fabs(r) ){
    pSum->rErr += (s - t) + r;
  }இல்லையெனில்{
    pSum->rErr += (r - t) + s;
  }
  pSum->rSum = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
நிலையான வெறுமை kahanBabuskaNeumaierStepInt64(நிலையற்ற SumCtx *pSum, i64 iVal){
  எனில்( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  }இல்லையெனில்{
    kahanBabuskaNeumaierStep(pSum, (இரட்டை)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
நிலையான வெறுமை kahanBabuskaNeumaierInit(
  நிலையற்ற SumCtx *p,
  i64 iVal
){
  எனில்( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->rSum = (இரட்டை)(iVal - iSm);
    p->rErr = (இரட்டை)iSm;
  }இல்லையெனில்{
    p->rSum = (இரட்டை)iVal;
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
நிலையான வெறுமை sumStep(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  SumCtx *p;
  முழு type;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, அளவு(*p));
  type = sqlite3_மதிப்பு_numeric_type(argv[0]);
  எனில்( p && type!=SQLITE_NULL ){
    p->cnt++;
    எனில்( p->approx==0 ){
      எனில்( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_மதிப்பு_double(argv[0]));
      }இல்லையெனில்{
        i64 x = p->iSum;
        எனில்( sqlite3AddInt64(&x, sqlite3_மதிப்பு_int64(argv[0]))==0 ){
          p->iSum = x;
        }இல்லையெனில்{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_மதிப்பு_int64(argv[0]));
        }
      }
    }இல்லையெனில்{
      எனில்( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_மதிப்பு_int64(argv[0]));
      }இல்லையெனில்{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_மதிப்பு_double(argv[0]));
      }
    }
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
நிலையான வெறுமை sumInverse(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு**argv){
  SumCtx *p;
  முழு type;
  ப்ரதிஜ்ஞா( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, அளவு(*p));
  type = sqlite3_மதிப்பு_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  எனில்( ALWAYS(p) && type!=SQLITE_NULL ){
    ப்ரதிஜ்ஞா( p->cnt>0 );
    p->cnt--;
    எனில்( !p->approx ){
      i64 x = p->iSum;
      எனில்( sqlite3SubInt64(&x, sqlite3_மதிப்பு_int64(argv[0]))==0 ){
        p->iSum = x;
        திரும்பு;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    எனில்( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_மதிப்பு_int64(argv[0]);
      எனில்( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }இல்லையெனில்{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }இல்லையெனில்{
      kahanBabuskaNeumaierStep(p, -sqlite3_மதிப்பு_double(argv[0]));
    }
  }
}
#இல்லையெனில்
# define sumInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
நிலையான வெறுமை sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  எனில்( p && p->cnt>0 ){
    எனில்( p->approx ){
      எனில்( p->ovrfl ){
        sqlite3_result_error(context,"integer overflow",-1);
      }இல்லையெனில் எனில்( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_result_double(context, p->rSum+p->rErr);
      }இல்லையெனில்{
        sqlite3_result_double(context, p->rSum);
      }
    }இல்லையெனில்{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
நிலையான வெறுமை avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  எனில்( p && p->cnt>0 ){
    இரட்டை r;
    எனில்( p->approx ){
      r = p->rSum;
      எனில்( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }இல்லையெனில்{
      r = (இரட்டை)(p->iSum);
    }
    sqlite3_result_double(context, r/(இரட்டை)p->cnt);
  }
}
நிலையான வெறுமை totalFinalize(sqlite3_context *context){
  SumCtx *p;
  இரட்டை r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  எனில்( p ){
    எனில்( p->approx ){
      r = p->rSum;
      எனில்( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }இல்லையெனில்{
      r = (இரட்டை)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
வகைவரை கட்டமைப்பு மொத்தம்_Ctx மொத்தம்_Ctx;
கட்டமைப்பு மொத்தம்_Ctx {
  i64 n;
#ifdef SQLITE_DEBUG
  முழு bInverse;                   /* True if xInverse() ever called */
#endif
};

/*
** Routines to implement the count() aggregate function.
*/
நிலையான வெறுமை மொத்தம்_Step(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு **argv){
  மொத்தம்_Ctx *p;
  p = sqlite3_aggregate_context(context, அளவு(*p));
  எனில்( (argc==0 || SQLITE_NULL!=sqlite3_மதிப்பு_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  ப்ரதிஜ்ஞா( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_மொத்தம்(context) );
#endif
}  
நிலையான வெறுமை மொத்தம்_Finalize(sqlite3_context *context){
  மொத்தம்_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}
#ifndef SQLITE_OMIT_WINDOWFUNC
நிலையான வெறுமை மொத்தம்_Inverse(sqlite3_context *ctx, முழு argc, sqlite3_மதிப்பு **argv){
  மொத்தம்_Ctx *p;
  p = sqlite3_aggregate_context(ctx, அளவு(*p));
  /* p is always non-NULL since countStep() will have been called first */
  எனில்( (argc==0 || SQLITE_NULL!=sqlite3_மதிப்பு_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ifdef SQLITE_DEBUG
    p->bInverse = 1;
#endif
  }
}  
#இல்லையெனில்
# define மொத்தம்_Inverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
நிலையான வெறுமை minmaxStep(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, அளவு(*pBest));
  எனில்( !pBest ) திரும்பு;

  எனில்( sqlite3_மதிப்பு_type(pArg)==SQLITE_NULL ){
    எனில்( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }இல்லையெனில் எனில்( pBest->flags ){
    முழு max;
    முழு cmp;
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
    எனில்( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }இல்லையெனில்{
      sqlite3SkipAccumulatorLoad(context);
    }
  }இல்லையெனில்{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
நிலையான வெறுமை min_Max_மதிப்பு_Finalize(sqlite3_context *context, முழு b_மதிப்பு){
  sqlite3_மதிப்பு *pRes;
  pRes = (sqlite3_மதிப்பு *)sqlite3_aggregate_context(context, 0);
  எனில்( pRes ){
    எனில்( pRes->flags ){
      sqlite3_result_மதிப்பு(context, pRes);
    }
    எனில்( b_மதிப்பு==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
நிலையான வெறுமை min_Max_மதிப்பு(sqlite3_context *context){
  min_Max_மதிப்பு_Finalize(context, 1);
}
#இல்லையெனில்
# define min_Max_மதிப்பு 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
நிலையான வெறுமை minMaxFinalize(sqlite3_context *context){
  min_Max_மதிப்பு_Finalize(context, 0);
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
வகைவரை கட்டமைப்பு {
  StrAccum str;          /* The accumulated concatenation */
#ifndef SQLITE_OMIT_WINDOWFUNC
  முழு nAccum;            /* Number of strings presently concatenated */
  முழு nFirstSepLength;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  முழு *pnSepLengths;
#endif
} GroupConcatCtx;

நிலையான வெறுமை groupConcatStep(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  மாறிலி எழுத்து *zVal;
  GroupConcatCtx *pGCC;
  மாறிலி எழுத்து *zSep;
  முழு nVal, nSep;
  ப்ரதிஜ்ஞா( argc==1 || argc==2 );
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ) திரும்பு;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, அளவு(*pGCC));
  எனில்( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    முழு firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    எனில்( argc==1 ){
      எனில்( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      இல்லையெனில்{
        pGCC->nFirstSepLength = 1;
      }
#endif
    }இல்லையெனில் எனில்( !firstTerm ){
      zSep = (எழுத்து*)sqlite3_மதிப்பு_text(argv[1]);
      nSep = sqlite3_மதிப்பு_bytes(argv[1]);
      எனில்( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      இல்லையெனில்{
        nSep = 0;
      }
      எனில்( nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0 ){
        முழு *pnsl = pGCC->pnSepLengths;
        எனில்( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (முழு*)sqlite3_malloc64((pGCC->nAccum+1) * அளவு(முழு));
          எனில்( pnsl!=0 ){
            முழு i = 0, nA = pGCC->nAccum-1;
            போது( i<nA ) pnsl[i++] = pGCC->nFirstSepLength;
          }
        }இல்லையெனில்{
          pnsl = (முழு*)sqlite3_realloc64(pnsl, pGCC->nAccum * அளவு(முழு));
        }
        எனில்( pnsl!=0 ){
          எனில்( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }இல்லையெனில்{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#endif
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    இல்லையெனில்{
      pGCC->nFirstSepLength = sqlite3_மதிப்பு_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#endif
    zVal = (எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    nVal = sqlite3_மதிப்பு_bytes(argv[0]);
    எனில்( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ifndef SQLITE_OMIT_WINDOWFUNC
நிலையான வெறுமை groupConcatInverse(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  GroupConcatCtx *pGCC;
  ப்ரதிஜ்ஞா( argc==1 || argc==2 );
  (வெறுமை)argc;  /* Suppress unused parameter warning */
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ) திரும்பு;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, அளவு(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  எனில்( ALWAYS(pGCC) ){
    முழு nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (வெறுமை)sqlite3_மதிப்பு_text(argv[0]);
    nVS = sqlite3_மதிப்பு_bytes(argv[0]);
    pGCC->nAccum -= 1;
    எனில்( pGCC->pnSepLengths!=0 ){
      ப்ரதிஜ்ஞா(pGCC->nAccum >= 0);
      எனில்( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*அளவு(முழு));
      }
    }இல்லையெனில்{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->nFirstSepLength;
    }
    எனில்( nVS>=(முழு)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }இல்லையெனில்{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    எனில்( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#இல்லையெனில்
# define groupConcatInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
நிலையான வெறுமை groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  எனில்( pGCC ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_XFER);
#ifndef SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#endif
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
நிலையான வெறுமை group_Concat_மதிப்பு(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  எனில்( pGCC && pGCC->nAccum>0 ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#இல்லையெனில்
# define group_Concat_மதிப்பு 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
வெறுமை sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  முழு rc = sqlite3_overload_function(db, "MATCH", 2);
  ப்ரதிஜ்ஞா( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  எனில்( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
வெறுமை sqlite3RegisterLikeFunctions(sqlite3 *db, முழு caseSensitive){
  FuncDef *pDef;
  கட்டமைப்பு compareInfo *pInfo;
  முழு flags;
  முழு nArg;
  எனில்( caseSensitive ){
    pInfo = (கட்டமைப்பு compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }இல்லையெனில்{
    pInfo = (கட்டமைப்பு compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  க்கு(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    ப்ரதிஜ்ஞா( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
முழு sqlite3IsLikeFunction(sqlite3 *db, கோவை *p_கோவை, முழு *pIsNocase, எழுத்து *aWc){
  FuncDef *pDef;
  முழு n_கோவை;
  ப்ரதிஜ்ஞா( p_கோவை!=0 );
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_FUNCTION );
  ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
  எனில்( !p_கோவை->x.pList ){
    திரும்பு 0;
  }
  n_கோவை = p_கோவை->x.pList->n_கோவை;
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
  pDef = sqlite3FindFunction(db, p_கோவை->u.zToken, n_கோவை, SQLITE_UTF8, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  எனில்( pDef==0 ) திரும்பு 0;
#endif
  எனில்( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    திரும்பு 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  ப்ரதிஜ்ஞா( (எழுத்து*)&likeInfoAlt == (எழுத்து*)&likeInfoAlt.matchAll );
  ப்ரதிஜ்ஞா( &((எழுத்து*)&likeInfoAlt)[1] == (எழுத்து*)&likeInfoAlt.matchOne );
  ப்ரதிஜ்ஞா( &((எழுத்து*)&likeInfoAlt)[2] == (எழுத்து*)&likeInfoAlt.matchSet );

  எனில்( n_கோவை<3 ){
    aWc[3] = 0;
  }இல்லையெனில்{
    கோவை *pEscape = p_கோவை->x.pList->a[2].p_கோவை;
    எழுத்து *zEscape;
    எனில்( pEscape->op!=TK_STRING ) திரும்பு 0;
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(pEscape, EP_Int_மதிப்பு) );
    zEscape = pEscape->u.zToken;
    எனில்( zEscape[0]==0 || zEscape[1]!=0 ) திரும்பு 0;
    எனில்( zEscape[0]==aWc[0] ) திரும்பு 0;
    எனில்( zEscape[0]==aWc[1] ) திரும்பு 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  திரும்பு 1;
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
நிலையான வெறுமை ceilingFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  ப்ரதிஜ்ஞா( argc==1 );
  தேர்வு( sqlite3_மதிப்பு_numeric_type(argv[0]) ){
    நிலை SQLITE_INTEGER: {
       sqlite3_result_int64(context, sqlite3_மதிப்பு_int64(argv[0]));
       நிறுத்து;
    }
    நிலை SQLITE_FLOAT: {
       இரட்டை (*x)(இரட்டை) = (இரட்டை(*)(இரட்டை))sqlite3_user_data(context);
       sqlite3_result_double(context, x(sqlite3_மதிப்பு_double(argv[0])));
       நிறுத்து;
    }
    இயல்பு: {
       நிறுத்து;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
நிலையான இரட்டை xCeil(இரட்டை x){ திரும்பு ceil(x); }
நிலையான இரட்டை xFloor(இரட்டை x){ திரும்பு floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#எனில் defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#endif
#எனில் defined(HAVE_LOG2) && HAVE_LOG2==0
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
நிலையான வெறுமை logFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  இரட்டை x, b, ans;
  ப்ரதிஜ்ஞா( argc==1 || argc==2 );
  தேர்வு( sqlite3_மதிப்பு_numeric_type(argv[0]) ){
    நிலை SQLITE_INTEGER:
    நிலை SQLITE_FLOAT:
      x = sqlite3_மதிப்பு_double(argv[0]);
      எனில்( x<=0.0 ) திரும்பு;
      நிறுத்து;
    இயல்பு:
      திரும்பு;
  }
  எனில்( argc==2 ){
    தேர்வு( sqlite3_மதிப்பு_numeric_type(argv[1]) ){
      நிலை SQLITE_INTEGER:
      நிலை SQLITE_FLOAT:
        b = log(x);
        எனில்( b<=0.0 ) திரும்பு;
        x = sqlite3_மதிப்பு_double(argv[1]);
        எனில்( x<=0.0 ) திரும்பு;
        நிறுத்து;
     இயல்பு:
        திரும்பு;
    }
    ans = log(x)/b;
  }இல்லையெனில்{
    தேர்வு( SQLITE_PTR_TO_INT(sqlite3_user_data(context)) ){
      நிலை 1:
        ans = log10(x);
        நிறுத்து;
      நிலை 2:
        ans = log2(x);
        நிறுத்து;
      இயல்பு:
        ans = log(x);
        நிறுத்து;
    }
  }
  sqlite3_result_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
நிலையான இரட்டை degToRad(இரட்டை x){ திரும்பு x*(M_PI/180.0); }
நிலையான இரட்டை radToDeg(இரட்டை x){ திரும்பு x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
நிலையான வெறுமை math1Func(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு type0;
  இரட்டை v0, ans;
  இரட்டை (*x)(இரட்டை);
  ப்ரதிஜ்ஞா( argc==1 );
  type0 = sqlite3_மதிப்பு_numeric_type(argv[0]);
  எனில்( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) திரும்பு;
  v0 = sqlite3_மதிப்பு_double(argv[0]);
  x = (இரட்டை(*)(இரட்டை))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
நிலையான வெறுமை math2Func(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு type0, type1;
  இரட்டை v0, v1, ans;
  இரட்டை (*x)(இரட்டை,இரட்டை);
  ப்ரதிஜ்ஞா( argc==2 );
  type0 = sqlite3_மதிப்பு_numeric_type(argv[0]);
  எனில்( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) திரும்பு;
  type1 = sqlite3_மதிப்பு_numeric_type(argv[1]);
  எனில்( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) திரும்பு;
  v0 = sqlite3_மதிப்பு_double(argv[0]);
  v1 = sqlite3_மதிப்பு_double(argv[1]);
  x = (இரட்டை(*)(இரட்டை,இரட்டை))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
நிலையான வெறுமை piFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  ப்ரதிஜ்ஞா( argc==0 );
  (வெறுமை)argv;
  sqlite3_result_double(context, M_PI);
}

#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
நிலையான வெறுமை signFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு type0;
  இரட்டை x;
  UNUSED_PARAMETER(argc);
  ப்ரதிஜ்ஞா( argc==1 );
  type0 = sqlite3_மதிப்பு_numeric_type(argv[0]);
  எனில்( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) திரும்பு;
  x = sqlite3_மதிப்பு_double(argv[0]);
  sqlite3_result_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#எனில் defined(SQLITE_ENABLE_PERCENTILE)
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
வகைவரை கட்டமைப்பு Percentile Percentile;
கட்டமைப்பு Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  எழுத்து bSorted;        /* True if a[] is already in sorted order */
  எழுத்து bKeepSorted;    /* True if advantageous to keep a[] sorted */
  எழுத்து bPctValid;      /* True if rPct is valid */
  இரட்டை rPct;         /* Fraction.  0.0 to 1.0 */
  இரட்டை *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
நிலையான முழு percentIsInfinity(இரட்டை r){
  sqlite3_uint64 u;
  ப்ரதிஜ்ஞா( அளவு(u)==அளவு(r) );
  memcpy(&u, &r, அளவு(u));
  திரும்பு ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
நிலையான முழு percent_Same_மதிப்பு(இரட்டை a, இரட்டை b){
  a -= b;
  திரும்பு a>=-0.001 && a<=0.001;
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
நிலையான i64 percent_இருமம்_தேடல்(Percentile *p, இரட்டை y, முழு bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  போது( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    இரட்டை x = p->a[iMid];
    எனில்( x<y ){
      iFirst = iMid + 1;
    }இல்லையெனில் எனில்( x>y ){
      iLast = iMid - 1;
    }இல்லையெனில்{
      திரும்பு iMid;
    }
  }
  எனில்( bExact ) திரும்பு -1;
  திரும்பு iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
நிலையான வெறுமை percentError(sqlite3_context *pCtx, மாறிலி எழுத்து *zFormat, ...){
  எழுத்து *zMsg1;
  எழுத்து *zMsg2;
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
நிலையான வெறுமை percentStep(sqlite3_context *pCtx, முழு argc, sqlite3_மதிப்பு **argv){
  Percentile *p;
  இரட்டை rPct;
  முழு eType;
  இரட்டை y;
  ப்ரதிஜ்ஞா( argc==2 || argc==1 );

  எனில்( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }இல்லையெனில்{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    இரட்டை mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_மதிப்பு_numeric_type(argv[1]);
    rPct = sqlite3_மதிப்பு_double(argv[1])/mxFrac;
    எனில்( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (இரட்டை)mxFrac);
      திரும்பு;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, அளவு(*p));
  எனில்( p==0 ) திரும்பு;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  எனில்( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }இல்லையெனில் எனில்( !percent_Same_மதிப்பு(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    திரும்பு;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_மதிப்பு_type(argv[0]);
  எனில்( eType==SQLITE_NULL ) திரும்பு;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  எனில்( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    திரும்பு;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_மதிப்பு_double(argv[0]);
  எனில்( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    திரும்பு;
  }

  /* Allocate and store the Y */
  எனில்( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    இரட்டை *a = sqlite3_realloc64(p->a, அளவு(இரட்டை)*n);
    எனில்( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, அளவு(*p));
      sqlite3_result_error_nomem(pCtx);
      திரும்பு;
    }
    p->nAlloc = n;
    p->a = a;
  }
  எனில்( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }இல்லையெனில் எனில்( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }இல்லையெனில் எனில்( p->bKeepSorted ){
    i64 i;
    i = percent_இருமம்_தேடல்(p, y, 0);
    எனில்( i<(முழு)p->nUsed ){
      memmove(&p->a[i+1], &p->a[i], (p->nUsed-i)*அளவு(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }இல்லையெனில்{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#define SWAP_DOUBLE(X,Y)  {இரட்டை ttt=(X);(X)=(Y);(Y)=ttt;}

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
நிலையான வெறுமை percentSort(
  இரட்டை *a,                      /* Array to sort */
  unsigned முழு n,                 /* Number of elements in array a[] */
  முழு iReq                        /* Element caller cares about (or -ve) */
){
  முழு iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  முழு iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  முழு i;         /* Loop counter */
  இரட்டை rPivot; /* The pivot value */

  ப்ரதிஜ்ஞா( n>=2 );
  செய்{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    எனில்( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    எனில்( n==2 ) திரும்பு;
    iGt = n-1;
    i = n/2;
    எனில்( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }இல்லையெனில் எனில்( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    எனில்( n==3 ) திரும்பு;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    செய்{
      எனில்( a[i]<rPivot ){
        எனில்( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }இல்லையெனில் எனில்( a[i]>rPivot ){
        செய்{
          iGt--;
        }போது( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }இல்லையெனில்{
        i++;
      }
    }போது( i<iGt );

    ப்ரதிஜ்ஞா( iLt>0 && iLt<iGt && (unsigned)iGt<n );
    testcase( iGt>iLt+1 );
    ப்ரதிஜ்ஞா( a[iLt]==rPivot );
    ப்ரதிஜ்ஞா( a[iLt-1]<=rPivot );
    ப்ரதிஜ்ஞா( a[iGt]>=rPivot );
    ப்ரதிஜ்ஞா( a[iLt+1]>=rPivot );

    எனில்( iReq>=0 ){
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
      எனில்( iReq<iLt ){
        n = iLt;
      }இல்லையெனில்{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }இல்லையெனில்{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      எனில்( iLt>(முழு)(n/2) ){
        எனில்( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }இல்லையெனில்{
        எனில்( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }போது( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
நிலையான வெறுமை percentInverse(sqlite3_context *pCtx,முழு argc,sqlite3_மதிப்பு **argv){
  Percentile *p;
  முழு eType;
  இரட்டை y;
  i64 i;
  ப்ரதிஜ்ஞா( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, அளவு(*p));
  ப்ரதிஜ்ஞா( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_மதிப்பு_type(argv[0]);
  எனில்( eType==SQLITE_NULL ) திரும்பு;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  எனில்( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    திரும்பு;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_மதிப்பு_double(argv[0]);
  எனில்( percentIsInfinity(y) ){
    திரும்பு;
  }
  எனில்( p->bSorted==0 ){
    ப்ரதிஜ்ஞா( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_இருமம்_தேடல்(p, y, 1);
  எனில்( i>=0 ){
    p->nUsed--;
    எனில்( i<(முழு)p->nUsed ){
      memmove(&p->a[i], &p->a[i+1], (p->nUsed - i)*அளவு(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
நிலையான வெறுமை percentCompute(sqlite3_context *pCtx, முழு bIsFinal){
  Percentile *p;
  முழு settings = SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&1; /* Discrete? */
  unsigned i1, i2;
  இரட்டை v1, v2;
  இரட்டை ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  எனில்( p==0 ) திரும்பு;
  எனில்( p->a==0 ) திரும்பு;
  எனில்( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (unsigned)ix;
    எனில்( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      ப்ரதிஜ்ஞா( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (முழு)i1 : -1));
      p->bSorted = 1;
    }
    எனில்( settings & 1 ){
      vx = p->a[i1];
    }இல்லையெனில்{
      i2 = ix==(இரட்டை)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  எனில்( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, அளவு(*p));
  }இல்லையெனில்{
    p->bKeepSorted = 1;
  }
}
நிலையான வெறுமை percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
நிலையான வெறுமை percent_மதிப்பு(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#endif /* SQLITE_ENABLE_PERCENTILE */

#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
நிலையான வெறுமை filestatFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  மாறிலி எழுத்து *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  எனில்( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    முழு rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ப்ரதிஜ்ஞா( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    எனில்( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }இல்லையெனில்{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      எனில்( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      எனில்( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        எனில்( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }இல்லையெனில்{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
நிலையான வெறுமை walstatFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  மாறிலி எழுத்து *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  எனில்( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ப்ரதிஜ்ஞா( pPager!=0 );
    pStr = sqlite3_str_new(db);
    எனில்( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }இல்லையெனில்{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }இல்லையெனில்{
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
நிலையான வெறுமை fpdecodeFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  FpDecode s;
  இரட்டை x;
  முழு y, z;
  எழுத்து zBuf[100];
  UNUSED_PARAMETER(argc);
  ப்ரதிஜ்ஞா( argc==3 );
  x = sqlite3_மதிப்பு_double(argv[0]);
  y = sqlite3_மதிப்பு_int(argv[1]);
  z = sqlite3_மதிப்பு_int(argv[2]);
  எனில்( z<=0 ) z = 1;
  எனில்( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  எனில்( s.isSpecial==2 ){
    sqlite3_snprintf(அளவு(zBuf), zBuf, "NaN");
  }இல்லையெனில்{
    sqlite3_snprintf(அளவு(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
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
நிலையான வெறுமை parseuriFunc(
  sqlite3_context *ctx,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  sqlite3_str *pResult;
  மாறிலி எழுத்து *zVfs;
  மாறிலி எழுத்து *zUri;
  unsigned முழு flgs;
  முழு rc;
  sqlite3_vfs *pVfs = 0;
  எழுத்து *zFile = 0;
  எழுத்து *zErr = 0;

  எனில்( argc<2 ) திரும்பு;
  pVfs = sqlite3_vfs_find(0);
  ப்ரதிஜ்ஞா( pVfs );
  zVfs = pVfs->zName;
  zUri = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  எனில்( zUri==0 ) திரும்பு;
  flgs = (unsigned முழு)sqlite3_மதிப்பு_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  pResult = sqlite3_str_new(0);
  எனில்( !sqlite3_str_errcode(pResult) ){
    முழு i;
    sqlite3_str_appendf(pResult, "rc=%d", rc);
    sqlite3_str_appendf(pResult, ", flags=0x%x", flgs);
    sqlite3_str_appendf(pResult, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(pResult, ", err=%Q", zErr);
    sqlite3_str_appendf(pResult, ", file=%Q", zFile);
    எனில்( zFile ){
      மாறிலி எழுத்து *z = zFile;
      z += sqlite3Strlen30(z)+1;
      போது( z[0] ){
        sqlite3_str_appendf(pResult, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      க்கு(i=2; i<argc; i++){
        மாறிலி எழுத்து *zArg;
        எனில்( sqlite3_மதிப்பு_type(argv[i])==SQLITE_INTEGER ){
          முழு k = sqlite3_மதிப்பு_int(argv[i]);
          sqlite3_str_appendf(pResult, ", '%d:%q'",k,sqlite3_uri_சாவி(zFile, k));
        }இல்லையெனில் எனில்( (zArg = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]))!=0 ){
          sqlite3_str_appendf(pResult, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }இல்லையெனில்{
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
வெறுமை sqlite3RegisterBuiltinFunctions(வெறுமை){
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
  நிலையான FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#எனில் !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(கோவை_compare,        2, INLINEFUNC_கோவை_compare,        0),
    TEST_FUNC(கோவை_implies_கோவை,   2, INLINEFUNC_கோவை_implies_கோவை,   0),
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
#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_மதிப்பு, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_மதிப்பு, 0,
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
    FUNCTION(எழுத்து,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifdef SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#endif
#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
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
    VFUNCTION(last_செருகு_rowid, 0, 0, 0, last_செருகு_rowid),
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
    WAGGREGATE(மொத்தம், 0,0,0, மொத்தம்_Step,
        மொத்தம்_Finalize, மொத்தம்_Finalize, மொத்தம்_Inverse,
        SQLITE_FUNC_மொத்தம்|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(மொத்தம், 1,0,0, மொத்தம்_Step,
        மொத்தம்_Finalize, மொத்தம்_Finalize, மொத்தம்_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_மதிப்பு, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_மதிப்பு, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_மதிப்பு, groupConcatInverse, 0),

#ifdef SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_மதிப்பு, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_மதிப்பு, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_மதிப்பு, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_மதிப்பு, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#endif /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#இல்லையெனில்
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
#எனில் SQLITE_HAVE_C99_MATH_FUNCS
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
#எனில் SQLITE_HAVE_C99_MATH_FUNCS
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
    INLINE_FUNC(எனில்,             -4, INLINEFUNC_iif,      0 ),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_செருகு_Builtin_Funcs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#எனில் 0  /* Enable to print out how the built-in functions are hashed */
  {
    முழு i;
    FuncDef *p;
    க்கு(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      க்கு(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        முழு n = sqlite3Strlen30(p->zName);
        முழு h = p->zName[0] + n;
        ப்ரதிஜ்ஞா( p->funcFlags & SQLITE_FUNC_BUILTIN );
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}
