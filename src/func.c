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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <assert.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#അന്തര്ഭാവ <math.h>
#endif
#അന്തര്ഭാവ "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
സ്ഥിതം CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  പ്രതിജ്ഞാ( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  പ്രതിജ്ഞാ( pOp->opcode==OP_CollSeq );
  പ്രതിജ്ഞാ( pOp->p4type==P4_COLLSEQ );
  തിരികെ pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
സ്ഥിതം ശൂന്യം sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  പ്രതിജ്ഞാ( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
സ്ഥിതം ശൂന്യം minmaxFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ mask;    /* 0 for min() or 0xffffffff for max() */
  പൂർണ്ണസംഖ്യ iBest;
  CollSeq *pColl;

  പ്രതിജ്ഞാ( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  പ്രതിജ്ഞാ( pColl );
  പ്രതിജ്ഞാ( mask==-1 || mask==0 );
  iBest = 0;
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ) തിരികെ;
  വേണ്ടി(i=1; i<argc; i++){
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[i])==SQLITE_NULL ) തിരികെ;
    എങ്കിൽ( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_മൂല്യം(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
സ്ഥിതം ശൂന്യം typeofFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിതം സ്ഥിരം അക്ഷരം *azType[] = { "integer", "real", "text", "blob", "null" };
  പൂർണ്ണസംഖ്യ i = sqlite3_മൂല്യം_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  പ്രതിജ്ഞാ( i>=0 && i<ArraySize(azType) );
  പ്രതിജ്ഞാ( SQLITE_INTEGER==1 );
  പ്രതിജ്ഞാ( SQLITE_FLOAT==2 );
  പ്രതിജ്ഞാ( SQLITE_TEXT==3 );
  പ്രതിജ്ഞാ( SQLITE_BLOB==4 );
  പ്രതിജ്ഞാ( SQLITE_NULL==5 );
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
സ്ഥിതം ശൂന്യം subtypeFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_result_int(context, sqlite3_മൂല്യം_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
സ്ഥിതം ശൂന്യം lengthFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(argv[0]) ){
    അവസ്ഥ SQLITE_BLOB:
    അവസ്ഥ SQLITE_INTEGER:
    അവസ്ഥ SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_മൂല്യം_bytes(argv[0]));
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TEXT: {
      സ്ഥിരം unsigned അക്ഷരം *z = sqlite3_മൂല്യം_text(argv[0]);
      സ്ഥിരം unsigned അക്ഷരം *z0;
      എങ്കിൽ( z==0 ) തിരികെ;
      z0 = z;
      ആകുമ്പോൾ( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        എങ്കിൽ( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }അല്ലെങ്കിൽ എങ്കിൽ( z[0]==0 ){
          നിർത്തുക;
        }അല്ലെങ്കിൽ{
          z++;
          ആകുമ്പോൾ( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_result_int(context, (പൂർണ്ണസംഖ്യ)(z-z0));
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      sqlite3_result_null(context);
      നിർത്തുക;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
സ്ഥിതം ശൂന്യം bytelengthFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(argv[0]) ){
    അവസ്ഥ SQLITE_BLOB: {
      sqlite3_result_int(context, sqlite3_മൂല്യം_bytes(argv[0]));
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_INTEGER:
    അവസ്ഥ SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_result_int64(context, sqlite3_മൂല്യം_bytes(argv[0])*m);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TEXT: {
      എങ്കിൽ( sqlite3_മൂല്യം_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_result_int(context, sqlite3_മൂല്യം_bytes(argv[0]));
      }അല്ലെങ്കിൽ{
        sqlite3_result_int(context, sqlite3_മൂല്യം_bytes16(argv[0]));
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      sqlite3_result_null(context);
      നിർത്തുക;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
സ്ഥിതം ശൂന്യം absFunc(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(argv[0]) ){
    അവസ്ഥ SQLITE_INTEGER: {
      i64 iVal = sqlite3_മൂല്യം_int64(argv[0]);
      എങ്കിൽ( iVal<0 ){
        എങ്കിൽ( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          തിരികെ;
        }
        iVal = -iVal;
      }
      sqlite3_result_int64(context, iVal);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      ഇരട്ട rVal = sqlite3_മൂല്യം_double(argv[0]);
      എങ്കിൽ( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      നിർത്തുക;
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
സ്ഥിതം ശൂന്യം instrFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം unsigned അക്ഷരം *zHaystack;
  സ്ഥിരം unsigned അക്ഷരം *zNeedle;
  പൂർണ്ണസംഖ്യ nHaystack;
  പൂർണ്ണസംഖ്യ nNeedle;
  പൂർണ്ണസംഖ്യ typeHaystack, typeNeedle;
  പൂർണ്ണസംഖ്യ N = 1;
  പൂർണ്ണസംഖ്യ isText;
  unsigned അക്ഷരം firstChar;
  sqlite3_മൂല്യം *pC1 = 0;
  sqlite3_മൂല്യം *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_മൂല്യം_type(argv[0]);
  typeNeedle = sqlite3_മൂല്യം_type(argv[1]);
  എങ്കിൽ( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) തിരികെ;
  nHaystack = sqlite3_മൂല്യം_bytes(argv[0]);
  nNeedle = sqlite3_മൂല്യം_bytes(argv[1]);
  എങ്കിൽ( nNeedle>0 ){
    എങ്കിൽ( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_മൂല്യം_blob(argv[0]);
      zNeedle = sqlite3_മൂല്യം_blob(argv[1]);
      isText = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_മൂല്യം_text(argv[0]);
      zNeedle = sqlite3_മൂല്യം_text(argv[1]);
      isText = 1;
    }അല്ലെങ്കിൽ{
      pC1 = sqlite3_മൂല്യം_dup(argv[0]);
      zHaystack = sqlite3_മൂല്യം_text(pC1);
      എങ്കിൽ( zHaystack==0 ) പോകുക endInstrOOM;
      nHaystack = sqlite3_മൂല്യം_bytes(pC1);
      pC2 = sqlite3_മൂല്യം_dup(argv[1]);
      zNeedle = sqlite3_മൂല്യം_text(pC2);
      എങ്കിൽ( zNeedle==0 ) പോകുക endInstrOOM;
      nNeedle = sqlite3_മൂല്യം_bytes(pC2);
      isText = 1;
    }
    എങ്കിൽ( zNeedle==0 || (nHaystack && zHaystack==0) ) പോകുക endInstrOOM;
    firstChar = zNeedle[0];
    ആകുമ്പോൾ( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      ചെയ്യുക{
        nHaystack--;
        zHaystack++;
      }ആകുമ്പോൾ( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    എങ്കിൽ( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_മൂല്യം_free(pC1);
  sqlite3_മൂല്യം_free(pC2);
  തിരികെ;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  പോകുക endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
സ്ഥിതം ശൂന്യം printfFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  PrintfArguments x;
  StrAccum str;
  സ്ഥിരം അക്ഷരം *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  എങ്കിൽ( argc>=1 && (zFormat = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]))!=0 ){
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
സ്ഥിതം SQLITE_INLINE പൂർണ്ണസംഖ്യ containsNulOrNonAscii(u64 word){
  സ്ഥിരം u64 byteOnes = UINT64_C(0x0101010101010101);
  സ്ഥിരം u64 byte_മുകളിലെ_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  തിരികെ ((word | (word - byteOnes)) & byte_മുകളിലെ_Bits)!=0;
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
സ്ഥിതം ശൂന്യം substrFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം unsigned അക്ഷരം *z;
  സ്ഥിരം unsigned അക്ഷരം *z2;
  പൂർണ്ണസംഖ്യ len;
  പൂർണ്ണസംഖ്യ p0type;
  i64 p1, p2;

  പ്രതിജ്ഞാ( argc==3 || argc==2 );
  p0type = sqlite3_മൂല്യം_type(argv[0]);
  p1 = sqlite3_മൂല്യം_int64(argv[1]);
  എങ്കിൽ( p0type==SQLITE_BLOB ){
    len = sqlite3_മൂല്യം_bytes(argv[0]);
    z = sqlite3_മൂല്യം_blob(argv[0]);
    എങ്കിൽ( z==0 ) തിരികെ;
    പ്രതിജ്ഞാ( len==sqlite3_മൂല്യം_bytes(argv[0]) );
  }അല്ലെങ്കിൽ{
    z = sqlite3_മൂല്യം_text(argv[0]);
    എങ്കിൽ( z==0 ) തിരികെ;
    len = 0;
    എങ്കിൽ( p1<0 ){
      വേണ്ടി(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  എങ്കിൽ( argc==3 ){
    p2 = sqlite3_മൂല്യം_int64(argv[2]);
    എങ്കിൽ( p2==0 && sqlite3_മൂല്യം_type(argv[2])==SQLITE_NULL ) തിരികെ;
  }അല്ലെങ്കിൽ{
    p2 = LARGEST_INT64;
  }
  എങ്കിൽ( p1==0 ){
#ifdef SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#endif
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[1])==SQLITE_NULL ) തിരികെ;
  }
  എങ്കിൽ( p1<0 ){
    p1 += len;
    എങ്കിൽ( p1<0 ){
      എങ്കിൽ( p2<0 ){
        p2 = 0;
      }അല്ലെങ്കിൽ{
        p2 += p1;
      }
      p1 = 0;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( p1>0 ){
    p1--;
  }അല്ലെങ്കിൽ എങ്കിൽ( p2>0 ){
    p2--;
  }
  എങ്കിൽ( p2<0 ){
    എങ്കിൽ( p2<-p1 ){
      p2 = p1;
    }അല്ലെങ്കിൽ{
      p2 = -p2;
    }
    p1 -= p2;
  }
  പ്രതിജ്ഞാ( p1>=0 && p2>=0 );
  എങ്കിൽ( p0type!=SQLITE_BLOB ){
    എങ്കിൽ( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      സ്ഥിരം unsigned അക്ഷരം *zEnd = z + sqlite3_മൂല്യം_bytes(argv[0]);
      ആകുമ്പോൾ( p1>=8 && zEnd-z>=8 ){
        u64 word;
        memcpy(&word, z, വലുപ്പം(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        എങ്കിൽ( containsNulOrNonAscii(word) ){
          നിർത്തുക;
        }
        z += 8;
        p1 -= 8;
      }
    }
    വേണ്ടി( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      എങ്കിൽ( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }അല്ലെങ്കിൽ എങ്കിൽ( z[0]==0 ){
        നിർത്തുക;
      }അല്ലെങ്കിൽ{
        ചെയ്യുക{ z++; }ആകുമ്പോൾ( (z[0]&0xc0)==0x80 );
      }
    }
    വേണ്ടി(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      എങ്കിൽ( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }അല്ലെങ്കിൽ എങ്കിൽ( z2[0]==0 ){
        നിർത്തുക;
      }അല്ലെങ്കിൽ{
        ചെയ്യുക{ z2++; }ആകുമ്പോൾ( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_result_text64(context, (അക്ഷരം*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( p1>=len ){
      p1 = p2 = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( p2>len-p1 ){
      p2 = len-p1;
      പ്രതിജ്ഞാ( p2>0 );
    }
    sqlite3_result_blob64(context, (അക്ഷരം*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
സ്ഥിതം ശൂന്യം roundFunc(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  i64 n = 0;
  ഇരട്ട r;
  അക്ഷരം *zBuf;
  പ്രതിജ്ഞാ( argc==1 || argc==2 );
  എങ്കിൽ( argc==2 ){
    എങ്കിൽ( SQLITE_NULL==sqlite3_മൂല്യം_type(argv[1]) ) തിരികെ;
    n = sqlite3_മൂല്യം_int64(argv[1]);
    എങ്കിൽ( n>30 ) n = 30;
    എങ്കിൽ( n<0 ) n = 0;
  }
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ) തിരികെ;
  r = sqlite3_മൂല്യം_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  എങ്കിൽ( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }അല്ലെങ്കിൽ എങ്കിൽ( n==0 ){ 
    r = (ഇരട്ട)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }അല്ലെങ്കിൽ{
    zBuf = sqlite3_mprintf("%!.*f",(പൂർണ്ണസംഖ്യ)n,r);
    എങ്കിൽ( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      തിരികെ;
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
സ്ഥിതം ശൂന്യം *contextMalloc(sqlite3_context *context, i64 nByte){
  അക്ഷരം *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  പ്രതിജ്ഞാ( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  എങ്കിൽ( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }അല്ലെങ്കിൽ{
    z = sqlite3Malloc(nByte);
    എങ്കിൽ( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  തിരികെ z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
സ്ഥിതം ശൂന്യം upperFunc(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  അക്ഷരം *z1;
  സ്ഥിരം അക്ഷരം *z2;
  പൂർണ്ണസംഖ്യ i, n;
  UNUSED_PARAMETER(argc);
  z2 = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  n = sqlite3_മൂല്യം_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  പ്രതിജ്ഞാ( z2==(അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]) );
  എങ്കിൽ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    എങ്കിൽ( z1 ){
      വേണ്ടി(i=0; i<n; i++){
        z1[i] = (അക്ഷരം)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
സ്ഥിതം ശൂന്യം lowerFunc(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  അക്ഷരം *z1;
  സ്ഥിരം അക്ഷരം *z2;
  പൂർണ്ണസംഖ്യ i, n;
  UNUSED_PARAMETER(argc);
  z2 = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  n = sqlite3_മൂല്യം_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  പ്രതിജ്ഞാ( z2==(അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]) );
  എങ്കിൽ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    എങ്കിൽ( z1 ){
      വേണ്ടി(i=0; i<n; i++){
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
സ്ഥിതം ശൂന്യം randomFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(വലുപ്പം(r), &r);
  എങ്കിൽ( r<0 ){
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
സ്ഥിതം ശൂന്യം randomBlob(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  sqlite3_int64 n;
  unsigned അക്ഷരം *p;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_മൂല്യം_int64(argv[0]);
  എങ്കിൽ( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  എങ്കിൽ( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (അക്ഷരം*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
സ്ഥിതം ശൂന്യം last_തിരുകുക_rowid(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_തിരുകുക_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
സ്ഥിതം ശൂന്യം changes(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
സ്ഥിതം ശൂന്യം total_changes(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **NotUsed2
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
ഘടന compareInfo {
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
#എങ്കിൽ defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#അല്ലെങ്കിൽ
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

സ്ഥിതം സ്ഥിരം ഘടന compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
സ്ഥിതം സ്ഥിരം ഘടന compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
സ്ഥിതം സ്ഥിരം ഘടന compareInfo likeInfoAlt = { '%', '_',   0, 0 };

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
സ്ഥിതം പൂർണ്ണസംഖ്യ patternCompare(
  സ്ഥിരം u8 *zPattern,              /* The glob pattern */
  സ്ഥിരം u8 *zString,               /* The string to compare against the glob */
  സ്ഥിരം ഘടന compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  സ്ഥിരം u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  ആകുമ്പോൾ( (c = Utf8Read(zPattern))!=0 ){
    എങ്കിൽ( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      ആകുമ്പോൾ( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        എങ്കിൽ( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          തിരികെ SQLITE_NOWILDCARDMATCH;
        }
      }
      എങ്കിൽ( c==0 ){
        തിരികെ SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }അല്ലെങ്കിൽ എങ്കിൽ( c==matchOther ){
        എങ്കിൽ( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          എങ്കിൽ( c==0 ) തിരികെ SQLITE_NOWILDCARDMATCH;
        }അല്ലെങ്കിൽ{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          പ്രതിജ്ഞാ( matchOther<0x80 );  /* '[' is a single-byte character */
          ആകുമ്പോൾ( *zString ){
            പൂർണ്ണസംഖ്യ bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            എങ്കിൽ( bMatch!=SQLITE_NOMATCH ) തിരികെ bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          തിരികെ SQLITE_NOWILDCARDMATCH;
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
      എങ്കിൽ( c<0x80 ){
        അക്ഷരം zStop[3];
        പൂർണ്ണസംഖ്യ bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        എങ്കിൽ( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        എങ്കിൽ( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }അല്ലെങ്കിൽ{
          zStop[0] = c;
          zStop[1] = 0;
        }
        ആകുമ്പോൾ(1){
          zString += strcspn((സ്ഥിരം അക്ഷരം*)zString, zStop);
          എങ്കിൽ( zString[0]==0 ) നിർത്തുക;
          zString++;
          എങ്കിൽ( nx && zString[0]!=nx && zString[0]!=ny ) തുടരുക;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          എങ്കിൽ( bMatch!=SQLITE_NOMATCH ) തിരികെ bMatch;
        }
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ bMatch;
        ആകുമ്പോൾ( (c2 = Utf8Read(zString))!=0 ){
          എങ്കിൽ( c2!=c ) തുടരുക;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          എങ്കിൽ( bMatch!=SQLITE_NOMATCH ) തിരികെ bMatch;
        }
      }
      തിരികെ SQLITE_NOWILDCARDMATCH;
    }
    എങ്കിൽ( c==matchOther ){
      എങ്കിൽ( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        എങ്കിൽ( c==0 ) തിരികെ SQLITE_NOMATCH;
        zEscaped = zPattern;
      }അല്ലെങ്കിൽ{
        u32 prior_c = 0;
        പൂർണ്ണസംഖ്യ seen = 0;
        പൂർണ്ണസംഖ്യ invert = 0;
        c = sqlite3Utf8Read(&zString);
        എങ്കിൽ( c==0 ) തിരികെ SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        എങ്കിൽ( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        എങ്കിൽ( c2==']' ){
          എങ്കിൽ( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        ആകുമ്പോൾ( c2 && c2!=']' ){
          എങ്കിൽ( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            എങ്കിൽ( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }അല്ലെങ്കിൽ{
            എങ്കിൽ( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        എങ്കിൽ( c2==0 || (seen ^ invert)==0 ){
          തിരികെ SQLITE_NOMATCH;
        }
        തുടരുക;
      }
    }
    c2 = Utf8Read(zString);
    എങ്കിൽ( c==c2 ) തുടരുക;
    എങ്കിൽ( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      തുടരുക;
    }
    എങ്കിൽ( c==matchOne && zPattern!=zEscaped && c2!=0 ) തുടരുക;
    തിരികെ SQLITE_NOMATCH;
  }
  തിരികെ *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
പൂർണ്ണസംഖ്യ sqlite3_strglob(സ്ഥിരം അക്ഷരം *zGlobPattern, സ്ഥിരം അക്ഷരം *zString){
  എങ്കിൽ( zString==0 ){
    തിരികെ zGlobPattern!=0;
  }അല്ലെങ്കിൽ എങ്കിൽ( zGlobPattern==0 ){
    തിരികെ 1;
  }അല്ലെങ്കിൽ {
    തിരികെ patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
പൂർണ്ണസംഖ്യ sqlite3_strlike(സ്ഥിരം അക്ഷരം *zPattern, സ്ഥിരം അക്ഷരം *zStr, unsigned പൂർണ്ണസംഖ്യ esc){
  എങ്കിൽ( zStr==0 ){
    തിരികെ zPattern!=0;
  }അല്ലെങ്കിൽ എങ്കിൽ( zPattern==0 ){
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
പൂർണ്ണസംഖ്യ sqlite3_like_എണ്ണം = 0;
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
സ്ഥിതം ശൂന്യം likeFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം unsigned അക്ഷരം *zA, *zB;
  u32 escape;
  പൂർണ്ണസംഖ്യ nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ഘടന compareInfo *pInfo = sqlite3_user_data(context);
  ഘടന compareInfo backupInfo;

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_BLOB
   || sqlite3_മൂല്യം_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_എണ്ണം++;
#endif
    sqlite3_result_int(context, 0);
    തിരികെ;
  }
#endif

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_മൂല്യം_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  എങ്കിൽ( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    തിരികെ;
  }
  എങ്കിൽ( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    സ്ഥിരം unsigned അക്ഷരം *zEsc = sqlite3_മൂല്യം_text(argv[2]);
    എങ്കിൽ( zEsc==0 ) തിരികെ;
    എങ്കിൽ( sqlite3Utf8CharLen((അക്ഷരം*)zEsc, -1)!=1 ){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      തിരികെ;
    }
    escape = sqlite3Utf8Read(&zEsc);
    എങ്കിൽ( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      memcpy(&backupInfo, pInfo, വലുപ്പം(backupInfo));
      pInfo = &backupInfo;
      എങ്കിൽ( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      എങ്കിൽ( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }അല്ലെങ്കിൽ{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_മൂല്യം_text(argv[0]);
  zA = sqlite3_മൂല്യം_text(argv[1]);
  എങ്കിൽ( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_എണ്ണം++;
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
സ്ഥിതം ശൂന്യം nullifFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  എങ്കിൽ( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_മൂല്യം(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
സ്ഥിതം ശൂന്യം versionFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **NotUsed2
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
സ്ഥിതം ശൂന്യം sourceidFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **NotUsed2
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
സ്ഥിതം ശൂന്യം errlogFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_മൂല്യം_int(argv[0]), "%s", sqlite3_മൂല്യം_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
സ്ഥിതം ശൂന്യം compileoptionusedFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം അക്ഷരം *zOptName;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  എങ്കിൽ( (zOptName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]))!=0 ){
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
സ്ഥിതം ശൂന്യം compileoptiongetFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  i64 n;
  സ്ഥിരം അക്ഷരം *zRes;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_മൂല്യം_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((പൂർണ്ണസംഖ്യ)n) : 0;
  sqlite3_result_text(context, zRes, -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
സ്ഥിതം സ്ഥിരം അക്ഷരം hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
ശൂന്യം sqlite3_Quote_മൂല്യം(StrAccum *pStr, sqlite3_മൂല്യം *p_മൂല്യം, പൂർണ്ണസംഖ്യ bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  പ്രതിജ്ഞാ( pStr!=0 && pStr->nChar==0 );

  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(p_മൂല്യം) ){
    അവസ്ഥ SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_മൂല്യം_double(p_മൂല്യം));
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_മൂല്യം_int64(p_മൂല്യം));
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_BLOB: {
      അക്ഷരം സ്ഥിരം *zBlob = sqlite3_മൂല്യം_blob(p_മൂല്യം);
      i64 nBlob = sqlite3_മൂല്യം_bytes(p_മൂല്യം);
      പ്രതിജ്ഞാ( zBlob==sqlite3_മൂല്യം_blob(p_മൂല്യം) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      എങ്കിൽ( pStr->accError==0 ){
        അക്ഷരം *zText = pStr->zText;
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TEXT: {
      സ്ഥിരം unsigned അക്ഷരം *zArg = sqlite3_മൂല്യം_text(p_മൂല്യം);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      പ്രതിജ്ഞാ( sqlite3_മൂല്യം_type(p_മൂല്യം)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      നിർത്തുക;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isNHex(സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ N, u32 *pVal){
  പൂർണ്ണസംഖ്യ i;
  u32 v = 0;
  വേണ്ടി(i=0; i<N; i++){
    എങ്കിൽ( !sqlite3Isxdigit(z[i]) ) തിരികെ 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  തിരികെ 1;
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
സ്ഥിതം ശൂന്യം unistrFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  അക്ഷരം *zOut;
  സ്ഥിരം അക്ഷരം *zIn;
  പൂർണ്ണസംഖ്യ nIn;
  പൂർണ്ണസംഖ്യ i, j, n;
  u32 v;

  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  എങ്കിൽ( zIn==0 ) തിരികെ;
  nIn = sqlite3_മൂല്യം_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  എങ്കിൽ( zOut==0 ){
    sqlite3_result_error_nomem(context);
    തിരികെ;
  }
  i = j = 0;
  ആകുമ്പോൾ( i<nIn ){
    സ്ഥിരം അക്ഷരം *z = strchr(&zIn[i],'\\');
    എങ്കിൽ( z==0 ){
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      നിർത്തുക;
    }
    n = z - &zIn[i];
    എങ്കിൽ( n>0 ){
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    എങ്കിൽ( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3Isxdigit(zIn[i+1]) ){
      എങ്കിൽ( !isNHex(&zIn[i+1], 4, &v) ) പോകുക unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }അല്ലെങ്കിൽ എങ്കിൽ( zIn[i+1]=='+' ){
      എങ്കിൽ( !isNHex(&zIn[i+2], 6, &v) ) പോകുക unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }അല്ലെങ്കിൽ എങ്കിൽ( zIn[i+1]=='u' ){
      എങ്കിൽ( !isNHex(&zIn[i+2], 4, &v) ) പോകുക unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }അല്ലെങ്കിൽ എങ്കിൽ( zIn[i+1]=='U' ){
      എങ്കിൽ( !isNHex(&zIn[i+2], 8, &v) ) പോകുക unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }അല്ലെങ്കിൽ{
      പോകുക unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  തിരികെ;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  തിരികെ;
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
സ്ഥിതം ശൂന്യം quoteFunc(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  sqlite3_Quote_മൂല്യം(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_data(context)));
  sqlite3_result_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
സ്ഥിതം ശൂന്യം unicodeFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം unsigned അക്ഷരം *z = sqlite3_മൂല്യം_text(argv[0]);
  (ശൂന്യം)argc;
  എങ്കിൽ( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
സ്ഥിതം ശൂന്യം charFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  unsigned അക്ഷരം *z, *zOut;
  പൂർണ്ണസംഖ്യ i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  എങ്കിൽ( z==0 ){
    sqlite3_result_error_nomem(context);
    തിരികെ;
  }
  വേണ്ടി(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_മൂല്യം_int64(argv[i]);
    എങ്കിൽ( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    എങ്കിൽ( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }അല്ലെങ്കിൽ എങ്കിൽ( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }അല്ലെങ്കിൽ എങ്കിൽ( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }അല്ലെങ്കിൽ{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_result_text64(context, (അക്ഷരം*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
സ്ഥിതം ശൂന്യം hexFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ i, n;
  സ്ഥിരം unsigned അക്ഷരം *pBlob;
  അക്ഷരം *zHex, *z;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_മൂല്യം_blob(argv[0]);
  n = sqlite3_മൂല്യം_bytes(argv[0]);
  പ്രതിജ്ഞാ( pBlob==sqlite3_മൂല്യം_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  എങ്കിൽ( zHex ){
    വേണ്ടി(i=0; i<n; i++, pBlob++){
      unsigned അക്ഷരം c = *pBlob;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ strContainsChar(സ്ഥിരം u8 *zStr, പൂർണ്ണസംഖ്യ nStr, u32 ch){
  സ്ഥിരം u8 *zEnd = &zStr[nStr];
  സ്ഥിരം u8 *z = zStr;
  ആകുമ്പോൾ( z<zEnd ){
    u32 tst = Utf8Read(z);
    എങ്കിൽ( tst==ch ) തിരികെ 1;
  }
  തിരികെ 0;
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
സ്ഥിതം ശൂന്യം unhexFunc(
  sqlite3_context *pCtx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം u8 *zPass = (സ്ഥിരം u8*)"";
  പൂർണ്ണസംഖ്യ nPass = 0;
  സ്ഥിരം u8 *zHex = sqlite3_മൂല്യം_text(argv[0]);
  പൂർണ്ണസംഖ്യ nHex = sqlite3_മൂല്യം_bytes(argv[0]);
#ifdef SQLITE_DEBUG
  സ്ഥിരം u8 *zEnd = zHex ? &zHex[nHex] : 0;
#endif
  u8 *pBlob = 0;
  u8 *p = 0;

  പ്രതിജ്ഞാ( argc==1 || argc==2 );
  എങ്കിൽ( argc==2 ){
    zPass = sqlite3_മൂല്യം_text(argv[1]);
    nPass = sqlite3_മൂല്യം_bytes(argv[1]);
  }
  എങ്കിൽ( !zHex || !zPass ) തിരികെ;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  എങ്കിൽ( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    ആകുമ്പോൾ( (c = *zHex)!=0x00 ){
      ആകുമ്പോൾ( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        പ്രതിജ്ഞാ( zHex<=zEnd );
        എങ്കിൽ( !strContainsChar(zPass, nPass, ch) ) പോകുക unhex_null;
        c = *zHex;
        എങ്കിൽ( c==0x00 ) പോകുക unhex_done;
      }
      zHex++;
      പ്രതിജ്ഞാ( *zEnd==0x00 );
      പ്രതിജ്ഞാ( zHex<=zEnd );
      d = *(zHex++);
      എങ്കിൽ( !sqlite3Isxdigit(d) ) പോകുക unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  തിരികെ;

 unhex_null:
  sqlite3_free(pBlob);
  തിരികെ;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
സ്ഥിതം ശൂന്യം zeroblobFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  i64 n;
  പൂർണ്ണസംഖ്യ rc;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_മൂല്യം_int64(argv[0]);
  എങ്കിൽ( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  എങ്കിൽ( rc ){
    sqlite3_result_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
സ്ഥിതം ശൂന്യം replaceFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം unsigned അക്ഷരം *zStr;        /* The input string A */
  സ്ഥിരം unsigned അക്ഷരം *zPattern;    /* The pattern string B */
  സ്ഥിരം unsigned അക്ഷരം *zRep;        /* The replacement string C */
  unsigned അക്ഷരം *zOut;              /* The output */
  പൂർണ്ണസംഖ്യ nStr;                /* Size of zStr */
  പൂർണ്ണസംഖ്യ nPattern;            /* Size of zPattern */
  പൂർണ്ണസംഖ്യ nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  പൂർണ്ണസംഖ്യ loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  unsigned cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  പ്രതിജ്ഞാ( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_മൂല്യം_text(argv[0]);
  എങ്കിൽ( zStr==0 ) തിരികെ;
  nStr = sqlite3_മൂല്യം_bytes(argv[0]);
  പ്രതിജ്ഞാ( zStr==sqlite3_മൂല്യം_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_മൂല്യം_text(argv[1]);
  എങ്കിൽ( zPattern==0 ){
    പ്രതിജ്ഞാ( sqlite3_മൂല്യം_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    തിരികെ;
  }
  എങ്കിൽ( zPattern[0]==0 ){
    പ്രതിജ്ഞാ( sqlite3_മൂല്യം_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_text(context, (സ്ഥിരം അക്ഷരം*)zStr, nStr, SQLITE_TRANSIENT);
    തിരികെ;
  }
  nPattern = sqlite3_മൂല്യം_bytes(argv[1]);
  പ്രതിജ്ഞാ( zPattern==sqlite3_മൂല്യം_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_മൂല്യം_text(argv[2]);
  എങ്കിൽ( zRep==0 ) തിരികെ;
  nRep = sqlite3_മൂല്യം_bytes(argv[2]);
  പ്രതിജ്ഞാ( zRep==sqlite3_മൂല്യം_text(argv[2]) );
  nOut = nStr + 1;
  പ്രതിജ്ഞാ( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, nOut);
  എങ്കിൽ( zOut==0 ){
    തിരികെ;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  വേണ്ടി(i=j=0; i<=loopLimit; i++){
    എങ്കിൽ( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
        എങ്കിൽ( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          തിരികെ;
        }
        cntExpand++;
        എങ്കിൽ( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (പൂർണ്ണസംഖ്യ)nOut + (nOut - nStr - 1));
          എങ്കിൽ( zOut==0 ){
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            തിരികെ;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  പ്രതിജ്ഞാ( j+nStr-i+1<=nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  പ്രതിജ്ഞാ( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (അക്ഷരം*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
സ്ഥിതം ശൂന്യം trimFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം unsigned അക്ഷരം *zIn;         /* Input string */
  സ്ഥിരം unsigned അക്ഷരം *zCharSet;    /* Set of characters to trim */
  unsigned പൂർണ്ണസംഖ്യ nIn;                 /* Number of bytes in input */
  പൂർണ്ണസംഖ്യ flags;                        /* 1: trimleft  2: trimright  3: trim */
  പൂർണ്ണസംഖ്യ i;                            /* Loop counter */
  unsigned പൂർണ്ണസംഖ്യ *aLen = 0;           /* Length of each character in zCharSet */
  unsigned അക്ഷരം **azChar = 0;       /* Individual characters in zCharSet */
  പൂർണ്ണസംഖ്യ nChar;                        /* Number of characters in zCharSet */

  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ){
    തിരികെ;
  }
  zIn = sqlite3_മൂല്യം_text(argv[0]);
  എങ്കിൽ( zIn==0 ) തിരികെ;
  nIn = (unsigned)sqlite3_മൂല്യം_bytes(argv[0]);
  പ്രതിജ്ഞാ( zIn==sqlite3_മൂല്യം_text(argv[0]) );
  എങ്കിൽ( argc==1 ){
    സ്ഥിതം സ്ഥിരം unsigned lenOne[] = { 1 };
    സ്ഥിതം unsigned അക്ഷരം * സ്ഥിരം azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (unsigned*)lenOne;
    azChar = (unsigned അക്ഷരം **)azOne;
    zCharSet = 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( (zCharSet = sqlite3_മൂല്യം_text(argv[1]))==0 ){
    തിരികെ;
  }അല്ലെങ്കിൽ{
    സ്ഥിരം unsigned അക്ഷരം *z;
    വേണ്ടി(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    എങ്കിൽ( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(വലുപ്പം(അക്ഷരം*)+വലുപ്പം(unsigned)));
      എങ്കിൽ( azChar==0 ){
        തിരികെ;
      }
      aLen = (unsigned*)&azChar[nChar];
      വേണ്ടി(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned അക്ഷരം *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  എങ്കിൽ( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    എങ്കിൽ( flags & 1 ){
      ആകുമ്പോൾ( nIn>0 ){
        unsigned പൂർണ്ണസംഖ്യ len = 0;
        വേണ്ടി(i=0; i<nChar; i++){
          len = aLen[i];
          എങ്കിൽ( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) നിർത്തുക;
        }
        എങ്കിൽ( i>=nChar ) നിർത്തുക;
        zIn += len;
        nIn -= len;
      }
    }
    എങ്കിൽ( flags & 2 ){
      ആകുമ്പോൾ( nIn>0 ){
        unsigned പൂർണ്ണസംഖ്യ len = 0;
        വേണ്ടി(i=0; i<nChar; i++){
          len = aLen[i];
          എങ്കിൽ( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) നിർത്തുക;
        }
        എങ്കിൽ( i>=nChar ) നിർത്തുക;
        nIn -= len;
      }
    }
    എങ്കിൽ( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (അക്ഷരം*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
സ്ഥിതം ശൂന്യം concatFuncCore(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv,
  പൂർണ്ണസംഖ്യ nSep,
  സ്ഥിരം അക്ഷരം *zSep
){
  i64 j, n = 0;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ bNotNull = 0;   /* True after at least NOT NULL argument seen */
  അക്ഷരം *z;
  വേണ്ടി(i=0; i<argc; i++){
    n += sqlite3_മൂല്യം_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  എങ്കിൽ( z==0 ){
    sqlite3_result_error_nomem(context);
    തിരികെ;
  }
  j = 0;
  വേണ്ടി(i=0; i<argc; i++){
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[i])!=SQLITE_NULL ){
      പൂർണ്ണസംഖ്യ k = sqlite3_മൂല്യം_bytes(argv[i]);
      സ്ഥിരം അക്ഷരം *v = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
      എങ്കിൽ( v!=0 ){
        എങ്കിൽ( bNotNull && nSep>0 ){
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
  പ്രതിജ്ഞാ( j<=n );
  sqlite3_result_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
സ്ഥിതം ശൂന്യം concatFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
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
സ്ഥിതം ശൂന്യം concatwsFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ nSep = sqlite3_മൂല്യം_bytes(argv[0]);
  സ്ഥിരം അക്ഷരം *zSep = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  എങ്കിൽ( zSep==0 ) തിരികെ;
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
സ്ഥിതം ശൂന്യം unknownFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  /* no-op */
  (ശൂന്യം)context;
  (ശൂന്യം)argc;
  (ശൂന്യം)argv;
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
സ്ഥിതം ശൂന്യം soundexFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  അക്ഷരം zResult[8];
  സ്ഥിരം u8 *zIn;
  പൂർണ്ണസംഖ്യ i, j;
  സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  പ്രതിജ്ഞാ( argc==1 );
  zIn = (u8*)sqlite3_മൂല്യം_text(argv[0]);
  എങ്കിൽ( zIn==0 ) zIn = (u8*)"";
  വേണ്ടി(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  എങ്കിൽ( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    വേണ്ടി(j=1; j<4 && zIn[i]; i++){
      പൂർണ്ണസംഖ്യ code = iCode[zIn[i]&0x7f];
      എങ്കിൽ( code>0 ){
        എങ്കിൽ( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }അല്ലെങ്കിൽ{
        prevcode = 0;
      }
    }
    ആകുമ്പോൾ( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }അല്ലെങ്കിൽ{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
സ്ഥിതം ശൂന്യം oomFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#endif /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
സ്ഥിതം ശൂന്യം loadExt(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  സ്ഥിരം അക്ഷരം *zFile = (സ്ഥിരം അക്ഷരം *)sqlite3_മൂല്യം_text(argv[0]);
  സ്ഥിരം അക്ഷരം *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  അക്ഷരം *z_Err_സന്ദേശം = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  എങ്കിൽ( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    തിരികെ;
  }

  എങ്കിൽ( argc==2 ){
    zProc = (സ്ഥിരം അക്ഷരം *)sqlite3_മൂല്യം_text(argv[1]);
  }അല്ലെങ്കിൽ{
    zProc = 0;
  }
  എങ്കിൽ( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_സന്ദേശം) ){
    sqlite3_result_error(context, z_Err_സന്ദേശം, -1);
    sqlite3_free(z_Err_സന്ദേശം);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
തരനിർവചനം ഘടന SumCtx SumCtx;
ഘടന SumCtx {
  ഇരട്ട rSum;      /* Running sum as as a double */
  ഇരട്ട rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
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
സ്ഥിതം ശൂന്യം kahanBabuskaNeumaierStep(
  അസ്ഥിര SumCtx *pSum,
  അസ്ഥിര ഇരട്ട r
){
  അസ്ഥിര ഇരട്ട s = pSum->rSum;
  അസ്ഥിര ഇരട്ട t = s + r;
  എങ്കിൽ( fabs(s) > fabs(r) ){
    pSum->rErr += (s - t) + r;
  }അല്ലെങ്കിൽ{
    pSum->rErr += (r - t) + s;
  }
  pSum->rSum = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
സ്ഥിതം ശൂന്യം kahanBabuskaNeumaierStepInt64(അസ്ഥിര SumCtx *pSum, i64 iVal){
  എങ്കിൽ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  }അല്ലെങ്കിൽ{
    kahanBabuskaNeumaierStep(pSum, (ഇരട്ട)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
സ്ഥിതം ശൂന്യം kahanBabuskaNeumaierInit(
  അസ്ഥിര SumCtx *p,
  i64 iVal
){
  എങ്കിൽ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->rSum = (ഇരട്ട)(iVal - iSm);
    p->rErr = (ഇരട്ട)iSm;
  }അല്ലെങ്കിൽ{
    p->rSum = (ഇരട്ട)iVal;
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
സ്ഥിതം ശൂന്യം sumStep(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  SumCtx *p;
  പൂർണ്ണസംഖ്യ type;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, വലുപ്പം(*p));
  type = sqlite3_മൂല്യം_numeric_type(argv[0]);
  എങ്കിൽ( p && type!=SQLITE_NULL ){
    p->cnt++;
    എങ്കിൽ( p->approx==0 ){
      എങ്കിൽ( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_മൂല്യം_double(argv[0]));
      }അല്ലെങ്കിൽ{
        i64 x = p->iSum;
        എങ്കിൽ( sqlite3AddInt64(&x, sqlite3_മൂല്യം_int64(argv[0]))==0 ){
          p->iSum = x;
        }അല്ലെങ്കിൽ{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_മൂല്യം_int64(argv[0]));
        }
      }
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_മൂല്യം_int64(argv[0]));
      }അല്ലെങ്കിൽ{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_മൂല്യം_double(argv[0]));
      }
    }
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
സ്ഥിതം ശൂന്യം sumInverse(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം**argv){
  SumCtx *p;
  പൂർണ്ണസംഖ്യ type;
  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, വലുപ്പം(*p));
  type = sqlite3_മൂല്യം_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  എങ്കിൽ( ALWAYS(p) && type!=SQLITE_NULL ){
    പ്രതിജ്ഞാ( p->cnt>0 );
    p->cnt--;
    എങ്കിൽ( !p->approx ){
      i64 x = p->iSum;
      എങ്കിൽ( sqlite3SubInt64(&x, sqlite3_മൂല്യം_int64(argv[0]))==0 ){
        p->iSum = x;
        തിരികെ;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    എങ്കിൽ( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_മൂല്യം_int64(argv[0]);
      എങ്കിൽ( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }അല്ലെങ്കിൽ{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }അല്ലെങ്കിൽ{
      kahanBabuskaNeumaierStep(p, -sqlite3_മൂല്യം_double(argv[0]));
    }
  }
}
#അല്ലെങ്കിൽ
# define sumInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
സ്ഥിതം ശൂന്യം sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  എങ്കിൽ( p && p->cnt>0 ){
    എങ്കിൽ( p->approx ){
      എങ്കിൽ( p->ovrfl ){
        sqlite3_result_error(context,"integer overflow",-1);
      }അല്ലെങ്കിൽ എങ്കിൽ( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_result_double(context, p->rSum+p->rErr);
      }അല്ലെങ്കിൽ{
        sqlite3_result_double(context, p->rSum);
      }
    }അല്ലെങ്കിൽ{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
സ്ഥിതം ശൂന്യം avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  എങ്കിൽ( p && p->cnt>0 ){
    ഇരട്ട r;
    എങ്കിൽ( p->approx ){
      r = p->rSum;
      എങ്കിൽ( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }അല്ലെങ്കിൽ{
      r = (ഇരട്ട)(p->iSum);
    }
    sqlite3_result_double(context, r/(ഇരട്ട)p->cnt);
  }
}
സ്ഥിതം ശൂന്യം totalFinalize(sqlite3_context *context){
  SumCtx *p;
  ഇരട്ട r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  എങ്കിൽ( p ){
    എങ്കിൽ( p->approx ){
      r = p->rSum;
      എങ്കിൽ( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }അല്ലെങ്കിൽ{
      r = (ഇരട്ട)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
തരനിർവചനം ഘടന എണ്ണം_Ctx എണ്ണം_Ctx;
ഘടന എണ്ണം_Ctx {
  i64 n;
#ifdef SQLITE_DEBUG
  പൂർണ്ണസംഖ്യ bInverse;                   /* True if xInverse() ever called */
#endif
};

/*
** Routines to implement the count() aggregate function.
*/
സ്ഥിതം ശൂന്യം എണ്ണം_Step(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  എണ്ണം_Ctx *p;
  p = sqlite3_aggregate_context(context, വലുപ്പം(*p));
  എങ്കിൽ( (argc==0 || SQLITE_NULL!=sqlite3_മൂല്യം_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  പ്രതിജ്ഞാ( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_എണ്ണം(context) );
#endif
}  
സ്ഥിതം ശൂന്യം എണ്ണം_Finalize(sqlite3_context *context){
  എണ്ണം_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}
#ifndef SQLITE_OMIT_WINDOWFUNC
സ്ഥിതം ശൂന്യം എണ്ണം_Inverse(sqlite3_context *ctx, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  എണ്ണം_Ctx *p;
  p = sqlite3_aggregate_context(ctx, വലുപ്പം(*p));
  /* p is always non-NULL since countStep() will have been called first */
  എങ്കിൽ( (argc==0 || SQLITE_NULL!=sqlite3_മൂല്യം_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ifdef SQLITE_DEBUG
    p->bInverse = 1;
#endif
  }
}  
#അല്ലെങ്കിൽ
# define എണ്ണം_Inverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
സ്ഥിതം ശൂന്യം minmaxStep(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, വലുപ്പം(*pBest));
  എങ്കിൽ( !pBest ) തിരികെ;

  എങ്കിൽ( sqlite3_മൂല്യം_type(pArg)==SQLITE_NULL ){
    എങ്കിൽ( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }അല്ലെങ്കിൽ എങ്കിൽ( pBest->flags ){
    പൂർണ്ണസംഖ്യ max;
    പൂർണ്ണസംഖ്യ cmp;
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
    എങ്കിൽ( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }അല്ലെങ്കിൽ{
      sqlite3SkipAccumulatorLoad(context);
    }
  }അല്ലെങ്കിൽ{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
സ്ഥിതം ശൂന്യം min_Max_മൂല്യം_Finalize(sqlite3_context *context, പൂർണ്ണസംഖ്യ b_മൂല്യം){
  sqlite3_മൂല്യം *pRes;
  pRes = (sqlite3_മൂല്യം *)sqlite3_aggregate_context(context, 0);
  എങ്കിൽ( pRes ){
    എങ്കിൽ( pRes->flags ){
      sqlite3_result_മൂല്യം(context, pRes);
    }
    എങ്കിൽ( b_മൂല്യം==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
സ്ഥിതം ശൂന്യം min_Max_മൂല്യം(sqlite3_context *context){
  min_Max_മൂല്യം_Finalize(context, 1);
}
#അല്ലെങ്കിൽ
# define min_Max_മൂല്യം 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
സ്ഥിതം ശൂന്യം minMaxFinalize(sqlite3_context *context){
  min_Max_മൂല്യം_Finalize(context, 0);
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
തരനിർവചനം ഘടന {
  StrAccum str;          /* The accumulated concatenation */
#ifndef SQLITE_OMIT_WINDOWFUNC
  പൂർണ്ണസംഖ്യ nAccum;            /* Number of strings presently concatenated */
  പൂർണ്ണസംഖ്യ nFirstSepLength;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  പൂർണ്ണസംഖ്യ *pnSepLengths;
#endif
} GroupConcatCtx;

സ്ഥിതം ശൂന്യം groupConcatStep(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം അക്ഷരം *zVal;
  GroupConcatCtx *pGCC;
  സ്ഥിരം അക്ഷരം *zSep;
  പൂർണ്ണസംഖ്യ nVal, nSep;
  പ്രതിജ്ഞാ( argc==1 || argc==2 );
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ) തിരികെ;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, വലുപ്പം(*pGCC));
  എങ്കിൽ( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    പൂർണ്ണസംഖ്യ firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    എങ്കിൽ( argc==1 ){
      എങ്കിൽ( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      അല്ലെങ്കിൽ{
        pGCC->nFirstSepLength = 1;
      }
#endif
    }അല്ലെങ്കിൽ എങ്കിൽ( !firstTerm ){
      zSep = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[1]);
      nSep = sqlite3_മൂല്യം_bytes(argv[1]);
      എങ്കിൽ( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      അല്ലെങ്കിൽ{
        nSep = 0;
      }
      എങ്കിൽ( nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0 ){
        പൂർണ്ണസംഖ്യ *pnsl = pGCC->pnSepLengths;
        എങ്കിൽ( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (പൂർണ്ണസംഖ്യ*)sqlite3_malloc64((pGCC->nAccum+1) * വലുപ്പം(പൂർണ്ണസംഖ്യ));
          എങ്കിൽ( pnsl!=0 ){
            പൂർണ്ണസംഖ്യ i = 0, nA = pGCC->nAccum-1;
            ആകുമ്പോൾ( i<nA ) pnsl[i++] = pGCC->nFirstSepLength;
          }
        }അല്ലെങ്കിൽ{
          pnsl = (പൂർണ്ണസംഖ്യ*)sqlite3_realloc64(pnsl, pGCC->nAccum * വലുപ്പം(പൂർണ്ണസംഖ്യ));
        }
        എങ്കിൽ( pnsl!=0 ){
          എങ്കിൽ( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }അല്ലെങ്കിൽ{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#endif
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    അല്ലെങ്കിൽ{
      pGCC->nFirstSepLength = sqlite3_മൂല്യം_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#endif
    zVal = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    nVal = sqlite3_മൂല്യം_bytes(argv[0]);
    എങ്കിൽ( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ifndef SQLITE_OMIT_WINDOWFUNC
സ്ഥിതം ശൂന്യം groupConcatInverse(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  GroupConcatCtx *pGCC;
  പ്രതിജ്ഞാ( argc==1 || argc==2 );
  (ശൂന്യം)argc;  /* Suppress unused parameter warning */
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ) തിരികെ;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, വലുപ്പം(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  എങ്കിൽ( ALWAYS(pGCC) ){
    പൂർണ്ണസംഖ്യ nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (ശൂന്യം)sqlite3_മൂല്യം_text(argv[0]);
    nVS = sqlite3_മൂല്യം_bytes(argv[0]);
    pGCC->nAccum -= 1;
    എങ്കിൽ( pGCC->pnSepLengths!=0 ){
      പ്രതിജ്ഞാ(pGCC->nAccum >= 0);
      എങ്കിൽ( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*വലുപ്പം(പൂർണ്ണസംഖ്യ));
      }
    }അല്ലെങ്കിൽ{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->nFirstSepLength;
    }
    എങ്കിൽ( nVS>=(പൂർണ്ണസംഖ്യ)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }അല്ലെങ്കിൽ{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    എങ്കിൽ( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#അല്ലെങ്കിൽ
# define groupConcatInverse 0
#endif /* SQLITE_OMIT_WINDOWFUNC */
സ്ഥിതം ശൂന്യം groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  എങ്കിൽ( pGCC ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_XFER);
#ifndef SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#endif
  }
}
#ifndef SQLITE_OMIT_WINDOWFUNC
സ്ഥിതം ശൂന്യം group_Concat_മൂല്യം(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  എങ്കിൽ( pGCC && pGCC->nAccum>0 ){
    sqlite3_result_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#അല്ലെങ്കിൽ
# define group_Concat_മൂല്യം 0
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
ശൂന്യം sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  പൂർണ്ണസംഖ്യ rc = sqlite3_overload_function(db, "MATCH", 2);
  പ്രതിജ്ഞാ( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  എങ്കിൽ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
ശൂന്യം sqlite3RegisterLikeFunctions(sqlite3 *db, പൂർണ്ണസംഖ്യ caseSensitive){
  FuncDef *pDef;
  ഘടന compareInfo *pInfo;
  പൂർണ്ണസംഖ്യ flags;
  പൂർണ്ണസംഖ്യ nArg;
  എങ്കിൽ( caseSensitive ){
    pInfo = (ഘടന compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }അല്ലെങ്കിൽ{
    pInfo = (ഘടന compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  വേണ്ടി(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    പ്രതിജ്ഞാ( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
പൂർണ്ണസംഖ്യ sqlite3IsLikeFunction(sqlite3 *db, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ *pIsNocase, അക്ഷരം *aWc){
  FuncDef *pDef;
  പൂർണ്ണസംഖ്യ n_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_FUNCTION );
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
  എങ്കിൽ( !p_വ്യഞ്ജകം->x.pList ){
    തിരികെ 0;
  }
  n_വ്യഞ്ജകം = p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
  pDef = sqlite3FindFunction(db, p_വ്യഞ്ജകം->u.zToken, n_വ്യഞ്ജകം, SQLITE_UTF8, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  എങ്കിൽ( pDef==0 ) തിരികെ 0;
#endif
  എങ്കിൽ( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    തിരികെ 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  പ്രതിജ്ഞാ( (അക്ഷരം*)&likeInfoAlt == (അക്ഷരം*)&likeInfoAlt.matchAll );
  പ്രതിജ്ഞാ( &((അക്ഷരം*)&likeInfoAlt)[1] == (അക്ഷരം*)&likeInfoAlt.matchOne );
  പ്രതിജ്ഞാ( &((അക്ഷരം*)&likeInfoAlt)[2] == (അക്ഷരം*)&likeInfoAlt.matchSet );

  എങ്കിൽ( n_വ്യഞ്ജകം<3 ){
    aWc[3] = 0;
  }അല്ലെങ്കിൽ{
    വ്യഞ്ജകം *pEscape = p_വ്യഞ്ജകം->x.pList->a[2].p_വ്യഞ്ജകം;
    അക്ഷരം *zEscape;
    എങ്കിൽ( pEscape->op!=TK_STRING ) തിരികെ 0;
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pEscape, EP_Int_മൂല്യം) );
    zEscape = pEscape->u.zToken;
    എങ്കിൽ( zEscape[0]==0 || zEscape[1]!=0 ) തിരികെ 0;
    എങ്കിൽ( zEscape[0]==aWc[0] ) തിരികെ 0;
    എങ്കിൽ( zEscape[0]==aWc[1] ) തിരികെ 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  തിരികെ 1;
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
സ്ഥിതം ശൂന്യം ceilingFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പ്രതിജ്ഞാ( argc==1 );
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_numeric_type(argv[0]) ){
    അവസ്ഥ SQLITE_INTEGER: {
       sqlite3_result_int64(context, sqlite3_മൂല്യം_int64(argv[0]));
       നിർത്തുക;
    }
    അവസ്ഥ SQLITE_FLOAT: {
       ഇരട്ട (*x)(ഇരട്ട) = (ഇരട്ട(*)(ഇരട്ട))sqlite3_user_data(context);
       sqlite3_result_double(context, x(sqlite3_മൂല്യം_double(argv[0])));
       നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
       നിർത്തുക;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
സ്ഥിതം ഇരട്ട xCeil(ഇരട്ട x){ തിരികെ ceil(x); }
സ്ഥിതം ഇരട്ട xFloor(ഇരട്ട x){ തിരികെ floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#എങ്കിൽ defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#endif
#എങ്കിൽ defined(HAVE_LOG2) && HAVE_LOG2==0
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
സ്ഥിതം ശൂന്യം logFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  ഇരട്ട x, b, ans;
  പ്രതിജ്ഞാ( argc==1 || argc==2 );
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_numeric_type(argv[0]) ){
    അവസ്ഥ SQLITE_INTEGER:
    അവസ്ഥ SQLITE_FLOAT:
      x = sqlite3_മൂല്യം_double(argv[0]);
      എങ്കിൽ( x<=0.0 ) തിരികെ;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ;
  }
  എങ്കിൽ( argc==2 ){
    തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_numeric_type(argv[1]) ){
      അവസ്ഥ SQLITE_INTEGER:
      അവസ്ഥ SQLITE_FLOAT:
        b = log(x);
        എങ്കിൽ( b<=0.0 ) തിരികെ;
        x = sqlite3_മൂല്യം_double(argv[1]);
        എങ്കിൽ( x<=0.0 ) തിരികെ;
        നിർത്തുക;
     സ്ഥിരസ്ഥിതി:
        തിരികെ;
    }
    ans = log(x)/b;
  }അല്ലെങ്കിൽ{
    തിരഞ്ഞെടുപ്പ്( SQLITE_PTR_TO_INT(sqlite3_user_data(context)) ){
      അവസ്ഥ 1:
        ans = log10(x);
        നിർത്തുക;
      അവസ്ഥ 2:
        ans = log2(x);
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        ans = log(x);
        നിർത്തുക;
    }
  }
  sqlite3_result_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
സ്ഥിതം ഇരട്ട degToRad(ഇരട്ട x){ തിരികെ x*(M_PI/180.0); }
സ്ഥിതം ഇരട്ട radToDeg(ഇരട്ട x){ തിരികെ x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
സ്ഥിതം ശൂന്യം math1Func(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ type0;
  ഇരട്ട v0, ans;
  ഇരട്ട (*x)(ഇരട്ട);
  പ്രതിജ്ഞാ( argc==1 );
  type0 = sqlite3_മൂല്യം_numeric_type(argv[0]);
  എങ്കിൽ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) തിരികെ;
  v0 = sqlite3_മൂല്യം_double(argv[0]);
  x = (ഇരട്ട(*)(ഇരട്ട))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
സ്ഥിതം ശൂന്യം math2Func(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ type0, type1;
  ഇരട്ട v0, v1, ans;
  ഇരട്ട (*x)(ഇരട്ട,ഇരട്ട);
  പ്രതിജ്ഞാ( argc==2 );
  type0 = sqlite3_മൂല്യം_numeric_type(argv[0]);
  എങ്കിൽ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) തിരികെ;
  type1 = sqlite3_മൂല്യം_numeric_type(argv[1]);
  എങ്കിൽ( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) തിരികെ;
  v0 = sqlite3_മൂല്യം_double(argv[0]);
  v1 = sqlite3_മൂല്യം_double(argv[1]);
  x = (ഇരട്ട(*)(ഇരട്ട,ഇരട്ട))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
സ്ഥിതം ശൂന്യം piFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പ്രതിജ്ഞാ( argc==0 );
  (ശൂന്യം)argv;
  sqlite3_result_double(context, M_PI);
}

#endif /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
സ്ഥിതം ശൂന്യം signFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ type0;
  ഇരട്ട x;
  UNUSED_PARAMETER(argc);
  പ്രതിജ്ഞാ( argc==1 );
  type0 = sqlite3_മൂല്യം_numeric_type(argv[0]);
  എങ്കിൽ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) തിരികെ;
  x = sqlite3_മൂല്യം_double(argv[0]);
  sqlite3_result_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#എങ്കിൽ defined(SQLITE_ENABLE_PERCENTILE)
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
തരനിർവചനം ഘടന Percentile Percentile;
ഘടന Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  അക്ഷരം bSorted;        /* True if a[] is already in sorted order */
  അക്ഷരം bKeepSorted;    /* True if advantageous to keep a[] sorted */
  അക്ഷരം bPctValid;      /* True if rPct is valid */
  ഇരട്ട rPct;         /* Fraction.  0.0 to 1.0 */
  ഇരട്ട *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ percentIsInfinity(ഇരട്ട r){
  sqlite3_uint64 u;
  പ്രതിജ്ഞാ( വലുപ്പം(u)==വലുപ്പം(r) );
  memcpy(&u, &r, വലുപ്പം(u));
  തിരികെ ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ percent_Same_മൂല്യം(ഇരട്ട a, ഇരട്ട b){
  a -= b;
  തിരികെ a>=-0.001 && a<=0.001;
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
സ്ഥിതം i64 percent_ദ്വിമാന_തിരയൽ(Percentile *p, ഇരട്ട y, പൂർണ്ണസംഖ്യ bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  ആകുമ്പോൾ( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    ഇരട്ട x = p->a[iMid];
    എങ്കിൽ( x<y ){
      iFirst = iMid + 1;
    }അല്ലെങ്കിൽ എങ്കിൽ( x>y ){
      iLast = iMid - 1;
    }അല്ലെങ്കിൽ{
      തിരികെ iMid;
    }
  }
  എങ്കിൽ( bExact ) തിരികെ -1;
  തിരികെ iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
സ്ഥിതം ശൂന്യം percentError(sqlite3_context *pCtx, സ്ഥിരം അക്ഷരം *zFormat, ...){
  അക്ഷരം *zMsg1;
  അക്ഷരം *zMsg2;
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
സ്ഥിതം ശൂന്യം percentStep(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  Percentile *p;
  ഇരട്ട rPct;
  പൂർണ്ണസംഖ്യ eType;
  ഇരട്ട y;
  പ്രതിജ്ഞാ( argc==2 || argc==1 );

  എങ്കിൽ( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }അല്ലെങ്കിൽ{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    ഇരട്ട mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_മൂല്യം_numeric_type(argv[1]);
    rPct = sqlite3_മൂല്യം_double(argv[1])/mxFrac;
    എങ്കിൽ( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (ഇരട്ട)mxFrac);
      തിരികെ;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) തിരികെ;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  എങ്കിൽ( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( !percent_Same_മൂല്യം(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    തിരികെ;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_മൂല്യം_type(argv[0]);
  എങ്കിൽ( eType==SQLITE_NULL ) തിരികെ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  എങ്കിൽ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    തിരികെ;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_മൂല്യം_double(argv[0]);
  എങ്കിൽ( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    തിരികെ;
  }

  /* Allocate and store the Y */
  എങ്കിൽ( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    ഇരട്ട *a = sqlite3_realloc64(p->a, വലുപ്പം(ഇരട്ട)*n);
    എങ്കിൽ( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, വലുപ്പം(*p));
      sqlite3_result_error_nomem(pCtx);
      തിരികെ;
    }
    p->nAlloc = n;
    p->a = a;
  }
  എങ്കിൽ( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }അല്ലെങ്കിൽ എങ്കിൽ( p->bKeepSorted ){
    i64 i;
    i = percent_ദ്വിമാന_തിരയൽ(p, y, 0);
    എങ്കിൽ( i<(പൂർണ്ണസംഖ്യ)p->nUsed ){
      memmove(&p->a[i+1], &p->a[i], (p->nUsed-i)*വലുപ്പം(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }അല്ലെങ്കിൽ{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#define SWAP_DOUBLE(X,Y)  {ഇരട്ട ttt=(X);(X)=(Y);(Y)=ttt;}

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
സ്ഥിതം ശൂന്യം percentSort(
  ഇരട്ട *a,                      /* Array to sort */
  unsigned പൂർണ്ണസംഖ്യ n,                 /* Number of elements in array a[] */
  പൂർണ്ണസംഖ്യ iReq                        /* Element caller cares about (or -ve) */
){
  പൂർണ്ണസംഖ്യ iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  പൂർണ്ണസംഖ്യ iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  പൂർണ്ണസംഖ്യ i;         /* Loop counter */
  ഇരട്ട rPivot; /* The pivot value */

  പ്രതിജ്ഞാ( n>=2 );
  ചെയ്യുക{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    എങ്കിൽ( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    എങ്കിൽ( n==2 ) തിരികെ;
    iGt = n-1;
    i = n/2;
    എങ്കിൽ( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }അല്ലെങ്കിൽ എങ്കിൽ( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    എങ്കിൽ( n==3 ) തിരികെ;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    ചെയ്യുക{
      എങ്കിൽ( a[i]<rPivot ){
        എങ്കിൽ( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }അല്ലെങ്കിൽ എങ്കിൽ( a[i]>rPivot ){
        ചെയ്യുക{
          iGt--;
        }ആകുമ്പോൾ( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }അല്ലെങ്കിൽ{
        i++;
      }
    }ആകുമ്പോൾ( i<iGt );

    പ്രതിജ്ഞാ( iLt>0 && iLt<iGt && (unsigned)iGt<n );
    testcase( iGt>iLt+1 );
    പ്രതിജ്ഞാ( a[iLt]==rPivot );
    പ്രതിജ്ഞാ( a[iLt-1]<=rPivot );
    പ്രതിജ്ഞാ( a[iGt]>=rPivot );
    പ്രതിജ്ഞാ( a[iLt+1]>=rPivot );

    എങ്കിൽ( iReq>=0 ){
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
      എങ്കിൽ( iReq<iLt ){
        n = iLt;
      }അല്ലെങ്കിൽ{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }അല്ലെങ്കിൽ{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      എങ്കിൽ( iLt>(പൂർണ്ണസംഖ്യ)(n/2) ){
        എങ്കിൽ( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }ആകുമ്പോൾ( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
സ്ഥിതം ശൂന്യം percentInverse(sqlite3_context *pCtx,പൂർണ്ണസംഖ്യ argc,sqlite3_മൂല്യം **argv){
  Percentile *p;
  പൂർണ്ണസംഖ്യ eType;
  ഇരട്ട y;
  i64 i;
  പ്രതിജ്ഞാ( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, വലുപ്പം(*p));
  പ്രതിജ്ഞാ( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_മൂല്യം_type(argv[0]);
  എങ്കിൽ( eType==SQLITE_NULL ) തിരികെ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  എങ്കിൽ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    തിരികെ;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_മൂല്യം_double(argv[0]);
  എങ്കിൽ( percentIsInfinity(y) ){
    തിരികെ;
  }
  എങ്കിൽ( p->bSorted==0 ){
    പ്രതിജ്ഞാ( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_ദ്വിമാന_തിരയൽ(p, y, 1);
  എങ്കിൽ( i>=0 ){
    p->nUsed--;
    എങ്കിൽ( i<(പൂർണ്ണസംഖ്യ)p->nUsed ){
      memmove(&p->a[i], &p->a[i+1], (p->nUsed - i)*വലുപ്പം(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
സ്ഥിതം ശൂന്യം percentCompute(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ bIsFinal){
  Percentile *p;
  പൂർണ്ണസംഖ്യ settings = SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx))&1; /* Discrete? */
  unsigned i1, i2;
  ഇരട്ട v1, v2;
  ഇരട്ട ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( p->a==0 ) തിരികെ;
  എങ്കിൽ( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (unsigned)ix;
    എങ്കിൽ( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      പ്രതിജ്ഞാ( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (പൂർണ്ണസംഖ്യ)i1 : -1));
      p->bSorted = 1;
    }
    എങ്കിൽ( settings & 1 ){
      vx = p->a[i1];
    }അല്ലെങ്കിൽ{
      i2 = ix==(ഇരട്ട)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  എങ്കിൽ( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, വലുപ്പം(*p));
  }അല്ലെങ്കിൽ{
    p->bKeepSorted = 1;
  }
}
സ്ഥിതം ശൂന്യം percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
സ്ഥിതം ശൂന്യം percent_മൂല്യം(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#endif /* SQLITE_ENABLE_PERCENTILE */

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
സ്ഥിതം ശൂന്യം filestatFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  സ്ഥിരം അക്ഷരം *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  എങ്കിൽ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    പൂർണ്ണസംഖ്യ rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    പ്രതിജ്ഞാ( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    എങ്കിൽ( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }അല്ലെങ്കിൽ{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      എങ്കിൽ( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      എങ്കിൽ( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        എങ്കിൽ( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }അല്ലെങ്കിൽ{
    sqlite3_result_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
സ്ഥിതം ശൂന്യം walstatFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  സ്ഥിരം അക്ഷരം *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  എങ്കിൽ( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    പ്രതിജ്ഞാ( pPager!=0 );
    pStr = sqlite3_str_new(db);
    എങ്കിൽ( sqlite3_str_errcode(pStr) ){
      sqlite3_result_error_nomem(context);
    }അല്ലെങ്കിൽ{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_result_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം fpdecodeFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  FpDecode s;
  ഇരട്ട x;
  പൂർണ്ണസംഖ്യ y, z;
  അക്ഷരം zBuf[100];
  UNUSED_PARAMETER(argc);
  പ്രതിജ്ഞാ( argc==3 );
  x = sqlite3_മൂല്യം_double(argv[0]);
  y = sqlite3_മൂല്യം_int(argv[1]);
  z = sqlite3_മൂല്യം_int(argv[2]);
  എങ്കിൽ( z<=0 ) z = 1;
  എങ്കിൽ( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  എങ്കിൽ( s.isSpecial==2 ){
    sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "NaN");
  }അല്ലെങ്കിൽ{
    sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
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
സ്ഥിതം ശൂന്യം parseuriFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  sqlite3_str *pResult;
  സ്ഥിരം അക്ഷരം *zVfs;
  സ്ഥിരം അക്ഷരം *zUri;
  unsigned പൂർണ്ണസംഖ്യ flgs;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_vfs *pVfs = 0;
  അക്ഷരം *zFile = 0;
  അക്ഷരം *zErr = 0;

  എങ്കിൽ( argc<2 ) തിരികെ;
  pVfs = sqlite3_vfs_find(0);
  പ്രതിജ്ഞാ( pVfs );
  zVfs = pVfs->zName;
  zUri = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  എങ്കിൽ( zUri==0 ) തിരികെ;
  flgs = (unsigned പൂർണ്ണസംഖ്യ)sqlite3_മൂല്യം_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  pResult = sqlite3_str_new(0);
  എങ്കിൽ( !sqlite3_str_errcode(pResult) ){
    പൂർണ്ണസംഖ്യ i;
    sqlite3_str_appendf(pResult, "rc=%d", rc);
    sqlite3_str_appendf(pResult, ", flags=0x%x", flgs);
    sqlite3_str_appendf(pResult, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(pResult, ", err=%Q", zErr);
    sqlite3_str_appendf(pResult, ", file=%Q", zFile);
    എങ്കിൽ( zFile ){
      സ്ഥിരം അക്ഷരം *z = zFile;
      z += sqlite3Strlen30(z)+1;
      ആകുമ്പോൾ( z[0] ){
        sqlite3_str_appendf(pResult, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      വേണ്ടി(i=2; i<argc; i++){
        സ്ഥിരം അക്ഷരം *zArg;
        എങ്കിൽ( sqlite3_മൂല്യം_type(argv[i])==SQLITE_INTEGER ){
          പൂർണ്ണസംഖ്യ k = sqlite3_മൂല്യം_int(argv[i]);
          sqlite3_str_appendf(pResult, ", '%d:%q'",k,sqlite3_uri_താക്കോൽ(zFile, k));
        }അല്ലെങ്കിൽ എങ്കിൽ( (zArg = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]))!=0 ){
          sqlite3_str_appendf(pResult, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }അല്ലെങ്കിൽ{
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
ശൂന്യം sqlite3RegisterBuiltinFunctions(ശൂന്യം){
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
  സ്ഥിതം FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#എങ്കിൽ !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(വ്യഞ്ജകം_compare,        2, INLINEFUNC_വ്യഞ്ജകം_compare,        0),
    TEST_FUNC(വ്യഞ്ജകം_implies_വ്യഞ്ജകം,   2, INLINEFUNC_വ്യഞ്ജകം_implies_വ്യഞ്ജകം,   0),
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
#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_മൂല്യം, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_മൂല്യം, 0,
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
    FUNCTION(അക്ഷരം,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifdef SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#endif
#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
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
    VFUNCTION(last_തിരുകുക_rowid, 0, 0, 0, last_തിരുകുക_rowid),
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
    WAGGREGATE(എണ്ണം, 0,0,0, എണ്ണം_Step,
        എണ്ണം_Finalize, എണ്ണം_Finalize, എണ്ണം_Inverse,
        SQLITE_FUNC_എണ്ണം|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(എണ്ണം, 1,0,0, എണ്ണം_Step,
        എണ്ണം_Finalize, എണ്ണം_Finalize, എണ്ണം_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_മൂല്യം, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_മൂല്യം, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_മൂല്യം, groupConcatInverse, 0),

#ifdef SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_മൂല്യം, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_മൂല്യം, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_മൂല്യം, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_മൂല്യം, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#endif /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#അല്ലെങ്കിൽ
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
#എങ്കിൽ SQLITE_HAVE_C99_MATH_FUNCS
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
#എങ്കിൽ SQLITE_HAVE_C99_MATH_FUNCS
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
    INLINE_FUNC(എങ്കിൽ,             -4, INLINEFUNC_iif,      0 ),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_തിരുകുക_Builtin_Funcs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#എങ്കിൽ 0  /* Enable to print out how the built-in functions are hashed */
  {
    പൂർണ്ണസംഖ്യ i;
    FuncDef *p;
    വേണ്ടി(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      വേണ്ടി(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        പൂർണ്ണസംഖ്യ n = sqlite3Strlen30(p->zName);
        പൂർണ്ണസംഖ്യ h = p->zName[0] + n;
        പ്രതിജ്ഞാ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}
