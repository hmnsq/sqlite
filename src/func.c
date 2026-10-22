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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
ਸਥਿਰ CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  ਪ੍ਰਤਿਜ੍ਞਾ( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode==OP_CollSeq );
  ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type==P4_COLLSEQ );
  ਵਾਪਸ pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_Skip_ਸੰਚਾਇਕ_ਲੱਦੋ(sqlite3_context *context){
  ਪ੍ਰਤਿਜ੍ਞਾ( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
ਸਥਿਰ ਖਾਲੀ minmaxFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i;
  ਪੂਰਨ ਨਕਾਬ;    /* 0 for min() or 0xffffffff for max() */
  ਪੂਰਨ i_ਉੱਤਮ;
  CollSeq *pColl;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc>1 );
  ਨਕਾਬ = sqlite3_user_ਅੰਕੜੇ(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  ਪ੍ਰਤਿਜ੍ਞਾ( pColl );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਨਕਾਬ==-1 || ਨਕਾਬ==0 );
  i_ਉੱਤਮ = 0;
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ) ਵਾਪਸ;
  ਲਈ(i=1; i<argc; i++){
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[i])==SQLITE_NULL ) ਵਾਪਸ;
    ਜੇ( (sqlite3MemCompare(argv[i_ਉੱਤਮ], argv[i], pColl)^ਨਕਾਬ)>=0 ){
      testcase( ਨਕਾਬ==0 );
      i_ਉੱਤਮ = i;
    }
  }
  sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, argv[i_ਉੱਤਮ]);
}

/*
** Return the type of the argument.
*/
ਸਥਿਰ ਖਾਲੀ typeofFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azType[] = { "integer", "real", "text", "blob", "null" };
  ਪੂਰਨ i = sqlite3_ਮੁੱਲ_type(argv[0]) - 1;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<Array_ਮਾਪ(azType) );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_INTEGER==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FLOAT==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_ਪਾਠ==3 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_BLOB==4 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
ਸਥਿਰ ਖਾਲੀ subtypeFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  sqlite3_ਨਤੀਜਾ_int(context, sqlite3_ਮੁੱਲ_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
ਸਥਿਰ ਖਾਲੀ ਲੰਮਾਈ_Func(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਚੋਣ( sqlite3_ਮੁੱਲ_type(argv[0]) ){
    ਹਾਲਤ SQLITE_BLOB:
    ਹਾਲਤ SQLITE_INTEGER:
    ਹਾਲਤ SQLITE_FLOAT: {
      sqlite3_ਨਤੀਜਾ_int(context, sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]));
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪਾਠ: {
      ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
      ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z0;
      ਜੇ( z==0 ) ਵਾਪਸ;
      z0 = z;
      ਜਦੋਂ( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        ਜੇ( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }ਨਹੀਂਤਾਂ ਜੇ( z[0]==0 ){
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          z++;
          ਜਦੋਂ( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_ਨਤੀਜਾ_int(context, (ਪੂਰਨ)(z-z0));
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      sqlite3_ਨਤੀਜਾ_null(context);
      ਰੋਕੋ;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
ਸਥਿਰ ਖਾਲੀ bytelengthFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਚੋਣ( sqlite3_ਮੁੱਲ_type(argv[0]) ){
    ਹਾਲਤ SQLITE_BLOB: {
      sqlite3_ਨਤੀਜਾ_int(context, sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]));
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_INTEGER:
    ਹਾਲਤ SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_ਨਤੀਜਾ_int64(context, sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0])*m);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪਾਠ: {
      ਜੇ( sqlite3_ਮੁੱਲ_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_ਨਤੀਜਾ_int(context, sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]));
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_int(context, sqlite3_ਮੁੱਲ_bytes16(argv[0]));
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      sqlite3_ਨਤੀਜਾ_null(context);
      ਰੋਕੋ;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
ਸਥਿਰ ਖਾਲੀ absFunc(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਚੋਣ( sqlite3_ਮੁੱਲ_type(argv[0]) ){
    ਹਾਲਤ SQLITE_INTEGER: {
      i64 iVal = sqlite3_ਮੁੱਲ_int64(argv[0]);
      ਜੇ( iVal<0 ){
        ਜੇ( iVal==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_ਨਤੀਜਾ_error(context, "integer overflow", -1);
          ਵਾਪਸ;
        }
        iVal = -iVal;
      }
      sqlite3_ਨਤੀਜਾ_int64(context, iVal);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_ਨਤੀਜਾ_null(context);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      ਦੁੱਗਣਾ rVal = sqlite3_ਮੁੱਲ_double(argv[0]);
      ਜੇ( rVal<0 ) rVal = -rVal;
      sqlite3_ਨਤੀਜਾ_double(context, rVal);
      ਰੋਕੋ;
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
ਸਥਿਰ ਖਾਲੀ instrFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zHaystack;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zNeedle;
  ਪੂਰਨ nHaystack;
  ਪੂਰਨ nNeedle;
  ਪੂਰਨ typeHaystack, typeNeedle;
  ਪੂਰਨ N = 1;
  ਪੂਰਨ is_ਪਾਠ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਪਹਿਲਾ_Char;
  sqlite3_ਮੁੱਲ *pC1 = 0;
  sqlite3_ਮੁੱਲ *pC2 = 0;

  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  typeHaystack = sqlite3_ਮੁੱਲ_type(argv[0]);
  typeNeedle = sqlite3_ਮੁੱਲ_type(argv[1]);
  ਜੇ( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) ਵਾਪਸ;
  nHaystack = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  nNeedle = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[1]);
  ਜੇ( nNeedle>0 ){
    ਜੇ( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_ਮੁੱਲ_blob(argv[0]);
      zNeedle = sqlite3_ਮੁੱਲ_blob(argv[1]);
      is_ਪਾਠ = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
      zNeedle = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
      is_ਪਾਠ = 1;
    }ਨਹੀਂਤਾਂ{
      pC1 = sqlite3_ਮੁੱਲ_dup(argv[0]);
      zHaystack = sqlite3_ਮੁੱਲ_ਪਾਠ(pC1);
      ਜੇ( zHaystack==0 ) ਜਾਓ ਸਿਰਾ_Instr_OOM;
      nHaystack = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pC1);
      pC2 = sqlite3_ਮੁੱਲ_dup(argv[1]);
      zNeedle = sqlite3_ਮੁੱਲ_ਪਾਠ(pC2);
      ਜੇ( zNeedle==0 ) ਜਾਓ ਸਿਰਾ_Instr_OOM;
      nNeedle = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pC2);
      is_ਪਾਠ = 1;
    }
    ਜੇ( zNeedle==0 || (nHaystack && zHaystack==0) ) ਜਾਓ ਸਿਰਾ_Instr_OOM;
    ਪਹਿਲਾ_Char = zNeedle[0];
    ਜਦੋਂ( nNeedle<=nHaystack
       && (zHaystack[0]!=ਪਹਿਲਾ_Char || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      ਕਰੋ{
        nHaystack--;
        zHaystack++;
      }ਜਦੋਂ( is_ਪਾਠ && (zHaystack[0]&0xc0)==0x80 );
    }
    ਜੇ( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_ਨਤੀਜਾ_int(context, N);
ਸਿਰਾ_Instr:
  sqlite3_ਮੁੱਲ_free(pC1);
  sqlite3_ਮੁੱਲ_free(pC2);
  ਵਾਪਸ;
ਸਿਰਾ_Instr_OOM:
  sqlite3_ਨਤੀਜਾ_error_nomem(context);
  ਜਾਓ ਸਿਰਾ_Instr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
ਸਥਿਰ ਖਾਲੀ printfFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  PrintfArguments x;
  StrAccum str;
  ਅਚਲ ਅੱਖਰ *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  ਜੇ( argc>=1 && (zFormat = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.n_ਵਰਤਿਆ = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_ਨਤੀਜਾ_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
ਸਥਿਰ SQLITE_INLINE ਪੂਰਨ ਸ਼ਾਮਲ_ਹੈ_Nul_Or_Non_Ascii(u64 ਸ਼ਬਦ){
  ਅਚਲ u64 byteOnes = UINT64_C(0x0101010101010101);
  ਅਚਲ u64 byte_ਉੱਚਾ_ਬਿੱਟ = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  ਵਾਪਸ ((ਸ਼ਬਦ | (ਸ਼ਬਦ - byteOnes)) & byte_ਉੱਚਾ_ਬਿੱਟ)!=0;
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
ਸਥਿਰ ਖਾਲੀ substrFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z2;
  ਪੂਰਨ len;
  ਪੂਰਨ p0type;
  i64 p1, p2;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==3 || argc==2 );
  p0type = sqlite3_ਮੁੱਲ_type(argv[0]);
  p1 = sqlite3_ਮੁੱਲ_int64(argv[1]);
  ਜੇ( p0type==SQLITE_BLOB ){
    len = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
    z = sqlite3_ਮੁੱਲ_blob(argv[0]);
    ਜੇ( z==0 ) ਵਾਪਸ;
    ਪ੍ਰਤਿਜ੍ਞਾ( len==sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]) );
  }ਨਹੀਂਤਾਂ{
    z = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    ਜੇ( z==0 ) ਵਾਪਸ;
    len = 0;
    ਜੇ( p1<0 ){
      ਲਈ(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  ਜੇ( argc==3 ){
    p2 = sqlite3_ਮੁੱਲ_int64(argv[2]);
    ਜੇ( p2==0 && sqlite3_ਮੁੱਲ_type(argv[2])==SQLITE_NULL ) ਵਾਪਸ;
  }ਨਹੀਂਤਾਂ{
    p2 = ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64;
  }
  ਜੇ( p1==0 ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#ਜੇ_ਅੰਤ
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[1])==SQLITE_NULL ) ਵਾਪਸ;
  }
  ਜੇ( p1<0 ){
    p1 += len;
    ਜੇ( p1<0 ){
      ਜੇ( p2<0 ){
        p2 = 0;
      }ਨਹੀਂਤਾਂ{
        p2 += p1;
      }
      p1 = 0;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p1>0 ){
    p1--;
  }ਨਹੀਂਤਾਂ ਜੇ( p2>0 ){
    p2--;
  }
  ਜੇ( p2<0 ){
    ਜੇ( p2<-p1 ){
      p2 = p1;
    }ਨਹੀਂਤਾਂ{
      p2 = -p2;
    }
    p1 -= p2;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p1>=0 && p2>=0 );
  ਜੇ( p0type!=SQLITE_BLOB ){
    ਜੇ( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਸਿਰਾ = z + sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
      ਜਦੋਂ( p1>=8 && z_ਸਿਰਾ-z>=8 ){
        u64 ਸ਼ਬਦ;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਸ਼ਬਦ, z, ਆਕਾਰ(ਸ਼ਬਦ));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        ਜੇ( ਸ਼ਾਮਲ_ਹੈ_Nul_Or_Non_Ascii(ਸ਼ਬਦ) ){
          ਰੋਕੋ;
        }
        z += 8;
        p1 -= 8;
      }
    }
    ਲਈ( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      ਜੇ( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }ਨਹੀਂਤਾਂ ਜੇ( z[0]==0 ){
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        ਕਰੋ{ z++; }ਜਦੋਂ( (z[0]&0xc0)==0x80 );
      }
    }
    ਲਈ(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      ਜੇ( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }ਨਹੀਂਤਾਂ ਜੇ( z2[0]==0 ){
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        ਕਰੋ{ z2++; }ਜਦੋਂ( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_ਨਤੀਜਾ_text64(context, (ਅੱਖਰ*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }ਨਹੀਂਤਾਂ{
    ਜੇ( p1>=len ){
      p1 = p2 = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( p2>len-p1 ){
      p2 = len-p1;
      ਪ੍ਰਤਿਜ੍ਞਾ( p2>0 );
    }
    sqlite3_ਨਤੀਜਾ_blob64(context, (ਅੱਖਰ*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
ਸਥਿਰ ਖਾਲੀ ਗੇੜਾ_Func(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  i64 n = 0;
  ਦੁੱਗਣਾ r;
  ਅੱਖਰ *zBuf;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 || argc==2 );
  ਜੇ( argc==2 ){
    ਜੇ( SQLITE_NULL==sqlite3_ਮੁੱਲ_type(argv[1]) ) ਵਾਪਸ;
    n = sqlite3_ਮੁੱਲ_int64(argv[1]);
    ਜੇ( n>30 ) n = 30;
    ਜੇ( n<0 ) n = 0;
  }
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ) ਵਾਪਸ;
  r = sqlite3_ਮੁੱਲ_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  ਜੇ( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }ਨਹੀਂਤਾਂ ਜੇ( n==0 ){ 
    r = (ਦੁੱਗਣਾ)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }ਨਹੀਂਤਾਂ{
    zBuf = sqlite3_mprintf("%!.*f",(ਪੂਰਨ)n,r);
    ਜੇ( zBuf==0 ){
      sqlite3_ਨਤੀਜਾ_error_nomem(context);
      ਵਾਪਸ;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_ਨਤੀਜਾ_double(context, r);
}
#ਜੇ_ਅੰਤ

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
ਸਥਿਰ ਖਾਲੀ *contextMalloc(sqlite3_context *context, i64 nByte){
  ਅੱਖਰ *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte>0 );
  testcase( nByte==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] );
  testcase( nByte==(i64)db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]+1 );
  ਜੇ( nByte>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] ){
    sqlite3_ਨਤੀਜਾ_error_toobig(context);
    z = 0;
  }ਨਹੀਂਤਾਂ{
    z = sqlite3Malloc(nByte);
    ਜੇ( !z ){
      sqlite3_ਨਤੀਜਾ_error_nomem(context);
    }
  }
  ਵਾਪਸ z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
ਸਥਿਰ ਖਾਲੀ ਉੱਤਲਾ_Func(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਅੱਖਰ *z1;
  ਅਚਲ ਅੱਖਰ *z2;
  ਪੂਰਨ i, n;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  z2 = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ਪ੍ਰਤਿਜ੍ਞਾ( z2==(ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]) );
  ਜੇ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ਜੇ( z1 ){
      ਲਈ(i=0; i<n; i++){
        z1[i] = (ਅੱਖਰ)sqlite3Toupper(z2[i]);
      }
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, z1, n, sqlite3_free);
    }
  }
}
ਸਥਿਰ ਖਾਲੀ ਹੇਠਲਾ_Func(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਅੱਖਰ *z1;
  ਅਚਲ ਅੱਖਰ *z2;
  ਪੂਰਨ i, n;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  z2 = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ਪ੍ਰਤਿਜ੍ਞਾ( z2==(ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]) );
  ਜੇ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ਜੇ( z1 ){
      ਲਈ(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, z1, n, sqlite3_free);
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ noopFunc ਸੰਸਕਰਣ_Func   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
ਸਥਿਰ ਖਾਲੀ randomFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **NotUsed2
){
  sqlite_int64 r;
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  sqlite3_randomness(ਆਕਾਰ(r), &r);
  ਜੇ( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64);
  }
  sqlite3_ਨਤੀਜਾ_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
ਸਥਿਰ ਖਾਲੀ randomBlob(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_int64 n;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  n = sqlite3_ਮੁੱਲ_int64(argv[0]);
  ਜੇ( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  ਜੇ( p ){
    sqlite3_randomness(n, p);
    sqlite3_ਨਤੀਜਾ_blob(context, (ਅੱਖਰ*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
ਸਥਿਰ ਖਾਲੀ ਆਖਰੀ_ਪਾਓ_rowid(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_ਨਤੀਜਾ_int64(context, sqlite3_ਆਖਰੀ_ਪਾਓ_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
ਸਥਿਰ ਖਾਲੀ changes(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  sqlite3_ਨਤੀਜਾ_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
ਸਥਿਰ ਖਾਲੀ ਕੁੱਲ_changes(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_ਨਤੀਜਾ_int64(context, sqlite3_ਕੁੱਲ_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
ਬਣਤਰ compareInfo {
  u8 ਮੇਲ_All;          /* "*" or "%" */
  u8 ਮੇਲ_One;          /* "?" or "_" */
  u8 ਮੇਲ_ਸੈੱਟ;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#ਜੇ defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#ਨਹੀਂਤਾਂ
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅਚਲ ਬਣਤਰ compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
ਸਥਿਰ ਅਚਲ ਬਣਤਰ compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
ਸਥਿਰ ਅਚਲ ਬਣਤਰ compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਮੇਲ             0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NOMATCH           1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NOWILDCARDMATCH   2

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
ਸਥਿਰ ਪੂਰਨ ਨਮੂਨਾ_Compare(
  ਅਚਲ u8 *z_ਨਮੂਨਾ,              /* The glob pattern */
  ਅਚਲ u8 *zString,               /* The string to compare against the glob */
  ਅਚਲ ਬਣਤਰ compareInfo *pInfo, /* Information about how to do the compare */
  u32 ਮੇਲ_Other                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 ਮੇਲ_One = pInfo->ਮੇਲ_One;  /* "?" or "_" */
  u32 ਮੇਲ_All = pInfo->ਮੇਲ_All;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ਅਚਲ u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  ਜਦੋਂ( (c = Utf8Read(z_ਨਮੂਨਾ))!=0 ){
    ਜੇ( c==ਮੇਲ_All ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      ਜਦੋਂ( (c=Utf8Read(z_ਨਮੂਨਾ)) == ਮੇਲ_All
             || (c == ਮੇਲ_One && ਮੇਲ_One!=0) ){
        ਜੇ( c==ਮੇਲ_One && sqlite3Utf8Read(&zString)==0 ){
          ਵਾਪਸ SQLITE_NOWILDCARDMATCH;
        }
      }
      ਜੇ( c==0 ){
        ਵਾਪਸ SQLITE_ਮੇਲ;   /* "*" at the end of the pattern matches */
      }ਨਹੀਂਤਾਂ ਜੇ( c==ਮੇਲ_Other ){
        ਜੇ( pInfo->ਮੇਲ_ਸੈੱਟ==0 ){
          c = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
          ਜੇ( c==0 ) ਵਾਪਸ SQLITE_NOWILDCARDMATCH;
        }ਨਹੀਂਤਾਂ{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          ਪ੍ਰਤਿਜ੍ਞਾ( ਮੇਲ_Other<0x80 );  /* '[' is a single-byte character */
          ਜਦੋਂ( *zString ){
            ਪੂਰਨ b_ਮੇਲ = ਨਮੂਨਾ_Compare(&z_ਨਮੂਨਾ[-1],zString,pInfo,ਮੇਲ_Other);
            ਜੇ( b_ਮੇਲ!=SQLITE_NOMATCH ) ਵਾਪਸ b_ਮੇਲ;
            SQLITE_SKIP_UTF8(zString);
          }
          ਵਾਪਸ SQLITE_NOWILDCARDMATCH;
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
      ਜੇ( c<0x80 ){
        ਅੱਖਰ zStop[3];
        ਪੂਰਨ b_ਮੇਲ;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = z_ਨਮੂਨਾ[0], ny = nx;
        ਜੇ( nx>=0x80 || nx==ਮੇਲ_All || nx==ਮੇਲ_One || nx==ਮੇਲ_Other ){
          nx = 0;
        }
        ਜੇ( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }ਨਹੀਂਤਾਂ{
          zStop[0] = c;
          zStop[1] = 0;
        }
        ਜਦੋਂ(1){
          zString += strcspn((ਅਚਲ ਅੱਖਰ*)zString, zStop);
          ਜੇ( zString[0]==0 ) ਰੋਕੋ;
          zString++;
          ਜੇ( nx && zString[0]!=nx && zString[0]!=ny ) ਜਾਰੀ;
          b_ਮੇਲ = ਨਮੂਨਾ_Compare(z_ਨਮੂਨਾ,zString,pInfo,ਮੇਲ_Other);
          ਜੇ( b_ਮੇਲ!=SQLITE_NOMATCH ) ਵਾਪਸ b_ਮੇਲ;
        }
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ b_ਮੇਲ;
        ਜਦੋਂ( (c2 = Utf8Read(zString))!=0 ){
          ਜੇ( c2!=c ) ਜਾਰੀ;
          b_ਮੇਲ = ਨਮੂਨਾ_Compare(z_ਨਮੂਨਾ,zString,pInfo,ਮੇਲ_Other);
          ਜੇ( b_ਮੇਲ!=SQLITE_NOMATCH ) ਵਾਪਸ b_ਮੇਲ;
        }
      }
      ਵਾਪਸ SQLITE_NOWILDCARDMATCH;
    }
    ਜੇ( c==ਮੇਲ_Other ){
      ਜੇ( pInfo->ਮੇਲ_ਸੈੱਟ==0 ){
        c = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
        ਜੇ( c==0 ) ਵਾਪਸ SQLITE_NOMATCH;
        zEscaped = z_ਨਮੂਨਾ;
      }ਨਹੀਂਤਾਂ{
        u32 ਪੂਰਵ_c = 0;
        ਪੂਰਨ ਵੇਖਿਆ = 0;
        ਪੂਰਨ invert = 0;
        c = sqlite3Utf8Read(&zString);
        ਜੇ( c==0 ) ਵਾਪਸ SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
        ਜੇ( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
        }
        ਜੇ( c2==']' ){
          ਜੇ( c==']' ) ਵੇਖਿਆ = 1;
          c2 = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
        }
        ਜਦੋਂ( c2 && c2!=']' ){
          ਜੇ( c2=='-' && z_ਨਮੂਨਾ[0]!=']' && z_ਨਮੂਨਾ[0]!=0 && ਪੂਰਵ_c>0 ){
            c2 = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
            ਜੇ( c>=ਪੂਰਵ_c && c<=c2 ) ਵੇਖਿਆ = 1;
            ਪੂਰਵ_c = 0;
          }ਨਹੀਂਤਾਂ{
            ਜੇ( c==c2 ){
              ਵੇਖਿਆ = 1;
            }
            ਪੂਰਵ_c = c2;
          }
          c2 = sqlite3Utf8Read(&z_ਨਮੂਨਾ);
        }
        ਜੇ( c2==0 || (ਵੇਖਿਆ ^ invert)==0 ){
          ਵਾਪਸ SQLITE_NOMATCH;
        }
        ਜਾਰੀ;
      }
    }
    c2 = Utf8Read(zString);
    ਜੇ( c==c2 ) ਜਾਰੀ;
    ਜੇ( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      ਜਾਰੀ;
    }
    ਜੇ( c==ਮੇਲ_One && z_ਨਮੂਨਾ!=zEscaped && c2!=0 ) ਜਾਰੀ;
    ਵਾਪਸ SQLITE_NOMATCH;
  }
  ਵਾਪਸ *zString==0 ? SQLITE_ਮੇਲ : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
ਪੂਰਨ sqlite3_strglob(ਅਚਲ ਅੱਖਰ *z_Glob_ਨਮੂਨਾ, ਅਚਲ ਅੱਖਰ *zString){
  ਜੇ( zString==0 ){
    ਵਾਪਸ z_Glob_ਨਮੂਨਾ!=0;
  }ਨਹੀਂਤਾਂ ਜੇ( z_Glob_ਨਮੂਨਾ==0 ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ {
    ਵਾਪਸ ਨਮੂਨਾ_Compare((u8*)z_Glob_ਨਮੂਨਾ, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
ਪੂਰਨ sqlite3_strlike(ਅਚਲ ਅੱਖਰ *z_ਨਮੂਨਾ, ਅਚਲ ਅੱਖਰ *zStr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ esc){
  ਜੇ( zStr==0 ){
    ਵਾਪਸ z_ਨਮੂਨਾ!=0;
  }ਨਹੀਂਤਾਂ ਜੇ( z_ਨਮੂਨਾ==0 ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ ਨਮੂਨਾ_Compare((u8*)z_ਨਮੂਨਾ, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
ਪੂਰਨ sqlite3_like_ਗਿਣਤੀ = 0;
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਖਾਲੀ likeFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zA, *zB;
  u32 escape;
  ਪੂਰਨ nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਬਣਤਰ compareInfo *pInfo = sqlite3_user_ਅੰਕੜੇ(context);
  ਬਣਤਰ compareInfo backupInfo;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_BLOB
   || sqlite3_ਮੁੱਲ_type(argv[1])==SQLITE_BLOB
  ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
    sqlite3_like_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ
    sqlite3_ਨਤੀਜਾ_int(context, 0);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  testcase( nPat==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ] );
  testcase( nPat==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ]+1 );
  ਜੇ( nPat > db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ] ){
    sqlite3_ਨਤੀਜਾ_error(context, "LIKE or GLOB pattern too complex", -1);
    ਵਾਪਸ;
  }
  ਜੇ( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zEsc = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[2]);
    ਜੇ( zEsc==0 ) ਵਾਪਸ;
    ਜੇ( sqlite3Utf8CharLen((ਅੱਖਰ*)zEsc, -1)!=1 ){
      sqlite3_ਨਤੀਜਾ_error(context,
          "ESCAPE expression must be a single character", -1);
      ਵਾਪਸ;
    }
    escape = sqlite3Utf8Read(&zEsc);
    ਜੇ( escape==pInfo->ਮੇਲ_All || escape==pInfo->ਮੇਲ_One ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&backupInfo, pInfo, ਆਕਾਰ(backupInfo));
      pInfo = &backupInfo;
      ਜੇ( escape==pInfo->ਮੇਲ_All ) pInfo->ਮੇਲ_All = 0;
      ਜੇ( escape==pInfo->ਮੇਲ_One ) pInfo->ਮੇਲ_One = 0;
    }
  }ਨਹੀਂਤਾਂ{
    escape = pInfo->ਮੇਲ_ਸੈੱਟ;
  }
  zB = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  zA = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  ਜੇ( zA && zB ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
    sqlite3_like_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ
    sqlite3_ਨਤੀਜਾ_int(context,
                      ਨਮੂਨਾ_Compare(zB, zA, pInfo, escape)==SQLITE_ਮੇਲ);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
ਸਥਿਰ ਖਾਲੀ nullifFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਜੇ( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
ਸਥਿਰ ਖਾਲੀ ਸੰਸਕਰਣ_Func(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **NotUsed2
){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
ਸਥਿਰ ਖਾਲੀ sourceidFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **NotUsed2
){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
ਸਥਿਰ ਖਾਲੀ errlogFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਅਣਵਰਤਿਆ_PARAMETER(context);
  sqlite3_log(sqlite3_ਮੁੱਲ_int(argv[0]), "%s", sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
ਸਥਿਰ ਖਾਲੀ compileoptionusedFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਅੱਖਰ *zOptName;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  ਜੇ( (zOptName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]))!=0 ){
    sqlite3_ਨਤੀਜਾ_int(context, sqlite3_compileoption_ਵਰਤਿਆ(zOptName));
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
ਸਥਿਰ ਖਾਲੀ compileoptiongetFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  i64 n;
  ਅਚਲ ਅੱਖਰ *zRes;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_ਮੁੱਲ_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((ਪੂਰਨ)n) : 0;
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, zRes, -1, SQLITE_STATIC);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
ਖਾਲੀ sqlite3_Quote_ਮੁੱਲ(StrAccum *pStr, sqlite3_ਮੁੱਲ *p_ਮੁੱਲ, ਪੂਰਨ bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pStr!=0 && pStr->nChar==0 );

  ਚੋਣ( sqlite3_ਮੁੱਲ_type(p_ਮੁੱਲ) ){
    ਹਾਲਤ SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_ਮੁੱਲ_double(p_ਮੁੱਲ));
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_ਮੁੱਲ_int64(p_ਮੁੱਲ));
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_BLOB: {
      ਅੱਖਰ ਅਚਲ *zBlob = sqlite3_ਮੁੱਲ_blob(p_ਮੁੱਲ);
      i64 nBlob = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(p_ਮੁੱਲ);
      ਪ੍ਰਤਿਜ੍ਞਾ( zBlob==sqlite3_ਮੁੱਲ_blob(p_ਮੁੱਲ) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      ਜੇ( pStr->accError==0 ){
        ਅੱਖਰ *z_ਪਾਠ = pStr->z_ਪਾਠ;
        ਪੂਰਨ i;
        ਲਈ(i=0; i<nBlob; i++){
          z_ਪਾਠ[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          z_ਪਾਠ[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        z_ਪਾਠ[(nBlob*2)+2] = '\'';
        z_ਪਾਠ[(nBlob*2)+3] = '\0';
        z_ਪਾਠ[0] = 'X';
        z_ਪਾਠ[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪਾਠ: {
      ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zArg = sqlite3_ਮੁੱਲ_ਪਾਠ(p_ਮੁੱਲ);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਮੁੱਲ_type(p_ਮੁੱਲ)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      ਰੋਕੋ;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
ਸਥਿਰ ਪੂਰਨ isNHex(ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ N, u32 *pVal){
  ਪੂਰਨ i;
  u32 v = 0;
  ਲਈ(i=0; i<N; i++){
    ਜੇ( !sqlite3Isxdigit(z[i]) ) ਵਾਪਸ 0;
    v = (v<<4) + sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(z[i]);
  }
  *pVal = v;
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਖਾਲੀ unistrFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ;
  ਪੂਰਨ n_ਵਿੱਚ;
  ਪੂਰਨ i, j, n;
  u32 v;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER( argc );
  z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਜੇ( z_ਵਿੱਚ==0 ) ਵਾਪਸ;
  n_ਵਿੱਚ = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  z_ਬਾਹਰਲਾ = sqlite3_malloc64(n_ਵਿੱਚ+1);
  ਜੇ( z_ਬਾਹਰਲਾ==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(context);
    ਵਾਪਸ;
  }
  i = j = 0;
  ਜਦੋਂ( i<n_ਵਿੱਚ ){
    ਅਚਲ ਅੱਖਰ *z = strchr(&z_ਵਿੱਚ[i],'\\');
    ਜੇ( z==0 ){
      n = n_ਵਿੱਚ - i;
      memmove(&z_ਬਾਹਰਲਾ[j], &z_ਵਿੱਚ[i], n);
      j += n;
      ਰੋਕੋ;
    }
    n = z - &z_ਵਿੱਚ[i];
    ਜੇ( n>0 ){
      memmove(&z_ਬਾਹਰਲਾ[j], &z_ਵਿੱਚ[i], n);
      j += n;
      i += n;
    }
    ਜੇ( z_ਵਿੱਚ[i+1]=='\\' ){
      i += 2;
      z_ਬਾਹਰਲਾ[j++] = '\\';
    }ਨਹੀਂਤਾਂ ਜੇ( sqlite3Isxdigit(z_ਵਿੱਚ[i+1]) ){
      ਜੇ( !isNHex(&z_ਵਿੱਚ[i+1], 4, &v) ) ਜਾਓ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&z_ਬਾਹਰਲਾ[j], v);
    }ਨਹੀਂਤਾਂ ਜੇ( z_ਵਿੱਚ[i+1]=='+' ){
      ਜੇ( !isNHex(&z_ਵਿੱਚ[i+2], 6, &v) ) ਜਾਓ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&z_ਬਾਹਰਲਾ[j], v);
    }ਨਹੀਂਤਾਂ ਜੇ( z_ਵਿੱਚ[i+1]=='u' ){
      ਜੇ( !isNHex(&z_ਵਿੱਚ[i+2], 4, &v) ) ਜਾਓ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&z_ਬਾਹਰਲਾ[j], v);
    }ਨਹੀਂਤਾਂ ਜੇ( z_ਵਿੱਚ[i+1]=='U' ){
      ਜੇ( !isNHex(&z_ਵਿੱਚ[i+2], 8, &v) ) ਜਾਓ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&z_ਬਾਹਰਲਾ[j], v);
    }ਨਹੀਂਤਾਂ{
      ਜਾਓ unistr_error;
    }
  }
  z_ਬਾਹਰਲਾ[j] = 0;
  sqlite3_ਨਤੀਜਾ_text64(context, z_ਬਾਹਰਲਾ, j, sqlite3_free, SQLITE_UTF8_ZT);
  ਵਾਪਸ;

unistr_error:
  sqlite3_free(z_ਬਾਹਰਲਾ);
  sqlite3_ਨਤੀਜਾ_error(context, "invalid Unicode escape", -1);
  ਵਾਪਸ;
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
ਸਥਿਰ ਖਾਲੀ quoteFunc(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]);
  sqlite3_Quote_ਮੁੱਲ(&str,argv[0],SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(context)));
  sqlite3_ਨਤੀਜਾ_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
ਸਥਿਰ ਖਾਲੀ unicodeFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  (ਖਾਲੀ)argc;
  ਜੇ( z && z[0] ) sqlite3_ਨਤੀਜਾ_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
ਸਥਿਰ ਖਾਲੀ charFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z, *z_ਬਾਹਰਲਾ;
  ਪੂਰਨ i;
  z_ਬਾਹਰਲਾ = z = sqlite3_malloc64( argc*4+1 );
  ਜੇ( z==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(context);
    ਵਾਪਸ;
  }
  ਲਈ(i=0; i<argc; i++){
    sqlite3_int64 x;
    ਬਿਨਾਨਿਸ਼ਾਨੀ c;
    x = sqlite3_ਮੁੱਲ_int64(argv[i]);
    ਜੇ( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(x & 0x1fffff);
    ਜੇ( c<0x00080 ){
      *z_ਬਾਹਰਲਾ++ = (u8)(c&0xFF);
    }ਨਹੀਂਤਾਂ ਜੇ( c<0x00800 ){
      *z_ਬਾਹਰਲਾ++ = 0xC0 + (u8)((c>>6)&0x1F);
      *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)(c & 0x3F);
    }ਨਹੀਂਤਾਂ ਜੇ( c<0x10000 ){
      *z_ਬਾਹਰਲਾ++ = 0xE0 + (u8)((c>>12)&0x0F);
      *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)(c & 0x3F);
    }ਨਹੀਂਤਾਂ{
      *z_ਬਾਹਰਲਾ++ = 0xF0 + (u8)((c>>18) & 0x07);
      *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)((c>>12) & 0x3F);
      *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *z_ਬਾਹਰਲਾ = 0;
  sqlite3_ਨਤੀਜਾ_text64(context, (ਅੱਖਰ*)z, z_ਬਾਹਰਲਾ-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
ਸਥਿਰ ਖਾਲੀ hexFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i, n;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pBlob;
  ਅੱਖਰ *zHex, *z;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  pBlob = sqlite3_ਮੁੱਲ_blob(argv[0]);
  n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  ਪ੍ਰਤਿਜ੍ਞਾ( pBlob==sqlite3_ਮੁੱਲ_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  ਜੇ( zHex ){
    ਲਈ(i=0; i<n; i++, pBlob++){
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_ਨਤੀਜਾ_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
ਸਥਿਰ ਪੂਰਨ str_ਸ਼ਾਮਲ_ਹੈ_Char(ਅਚਲ u8 *zStr, ਪੂਰਨ nStr, u32 ch){
  ਅਚਲ u8 *z_ਸਿਰਾ = &zStr[nStr];
  ਅਚਲ u8 *z = zStr;
  ਜਦੋਂ( z<z_ਸਿਰਾ ){
    u32 tst = Utf8Read(z);
    ਜੇ( tst==ch ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ unhexFunc(
  sqlite3_context *pCtx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ u8 *zPass = (ਅਚਲ u8*)"";
  ਪੂਰਨ nPass = 0;
  ਅਚਲ u8 *zHex = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਪੂਰਨ nHex = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਅਚਲ u8 *z_ਸਿਰਾ = zHex ? &zHex[nHex] : 0;
#ਜੇ_ਅੰਤ
  u8 *pBlob = 0;
  u8 *p = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 || argc==2 );
  ਜੇ( argc==2 ){
    zPass = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
    nPass = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[1]);
  }
  ਜੇ( !zHex || !zPass ) ਵਾਪਸ;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  ਜੇ( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    ਜਦੋਂ( (c = *zHex)!=0x00 ){
      ਜਦੋਂ( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        ਪ੍ਰਤਿਜ੍ਞਾ( zHex<=z_ਸਿਰਾ );
        ਜੇ( !str_ਸ਼ਾਮਲ_ਹੈ_Char(zPass, nPass, ch) ) ਜਾਓ unhex_null;
        c = *zHex;
        ਜੇ( c==0x00 ) ਜਾਓ unhex_ਪੂਰਾ;
      }
      zHex++;
      ਪ੍ਰਤਿਜ੍ਞਾ( *z_ਸਿਰਾ==0x00 );
      ਪ੍ਰਤਿਜ੍ਞਾ( zHex<=z_ਸਿਰਾ );
      d = *(zHex++);
      ਜੇ( !sqlite3Isxdigit(d) ) ਜਾਓ unhex_null;
      *(p++) = (sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(c)<<4) | sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(d);
    }
  }

 unhex_ਪੂਰਾ:
  sqlite3_ਨਤੀਜਾ_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  ਵਾਪਸ;

 unhex_null:
  sqlite3_free(pBlob);
  ਵਾਪਸ;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
ਸਥਿਰ ਖਾਲੀ zeroblobFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  i64 n;
  ਪੂਰਨ rc;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  n = sqlite3_ਮੁੱਲ_int64(argv[0]);
  ਜੇ( n<0 ) n = 0;
  rc = sqlite3_ਨਤੀਜਾ_zeroblob64(context, n); /* IMP: R-00293-64994 */
  ਜੇ( rc ){
    sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
ਸਥਿਰ ਖਾਲੀ replaceFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zStr;        /* The input string A */
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਨਮੂਨਾ;    /* The pattern string B */
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zRep;        /* The replacement string C */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਬਾਹਰਲਾ;              /* The output */
  ਪੂਰਨ nStr;                /* Size of zStr */
  ਪੂਰਨ n_ਨਮੂਨਾ;            /* Size of zPattern */
  ਪੂਰਨ nRep;                /* Size of zRep */
  i64 n_ਬਾਹਰਲਾ;                /* Maximum size of zOut */
  ਪੂਰਨ loop_ਹੱਦਬੰਦੀ;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  ਬਿਨਾਨਿਸ਼ਾਨੀ cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==3 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  zStr = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਜੇ( zStr==0 ) ਵਾਪਸ;
  nStr = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  ਪ੍ਰਤਿਜ੍ਞਾ( zStr==sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]) );  /* No encoding change */
  z_ਨਮੂਨਾ = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  ਜੇ( z_ਨਮੂਨਾ==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਮੁੱਲ_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    ਵਾਪਸ;
  }
  ਜੇ( z_ਨਮੂਨਾ[0]==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਮੁੱਲ_type(argv[1])!=SQLITE_NULL );
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, (ਅਚਲ ਅੱਖਰ*)zStr, nStr, SQLITE_TRANSIENT);
    ਵਾਪਸ;
  }
  n_ਨਮੂਨਾ = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[1]);
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਨਮੂਨਾ==sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]) );  /* No encoding change */
  zRep = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[2]);
  ਜੇ( zRep==0 ) ਵਾਪਸ;
  nRep = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[2]);
  ਪ੍ਰਤਿਜ੍ਞਾ( zRep==sqlite3_ਮੁੱਲ_ਪਾਠ(argv[2]) );
  n_ਬਾਹਰਲਾ = nStr + 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਹਰਲਾ<SQLITE_MAX_ਲੰਮਾਈ );
  z_ਬਾਹਰਲਾ = contextMalloc(context, n_ਬਾਹਰਲਾ);
  ਜੇ( z_ਬਾਹਰਲਾ==0 ){
    ਵਾਪਸ;
  }
  loop_ਹੱਦਬੰਦੀ = nStr - n_ਨਮੂਨਾ; 
  cntExpand = 0;
  ਲਈ(i=j=0; i<=loop_ਹੱਦਬੰਦੀ; i++){
    ਜੇ( zStr[i]!=z_ਨਮੂਨਾ[0] || memcmp(&zStr[i], z_ਨਮੂਨਾ, n_ਨਮੂਨਾ) ){
      z_ਬਾਹਰਲਾ[j++] = zStr[i];
    }ਨਹੀਂਤਾਂ{
      ਜੇ( nRep>n_ਨਮੂਨਾ ){
        n_ਬਾਹਰਲਾ += nRep - n_ਨਮੂਨਾ;
        testcase( n_ਬਾਹਰਲਾ-1==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] );
        testcase( n_ਬਾਹਰਲਾ-2==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] );
        ਜੇ( n_ਬਾਹਰਲਾ-1>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] ){
          sqlite3_ਨਤੀਜਾ_error_toobig(context);
          sqlite3_free(z_ਬਾਹਰਲਾ);
          ਵਾਪਸ;
        }
        cntExpand++;
        ਜੇ( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = z_ਬਾਹਰਲਾ;
          z_ਬਾਹਰਲਾ = sqlite3Realloc(z_ਬਾਹਰਲਾ, (ਪੂਰਨ)n_ਬਾਹਰਲਾ + (n_ਬਾਹਰਲਾ - nStr - 1));
          ਜੇ( z_ਬਾਹਰਲਾ==0 ){
            sqlite3_ਨਤੀਜਾ_error_nomem(context);
            sqlite3_free(zOld);
            ਵਾਪਸ;
          }
        }
      }
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&z_ਬਾਹਰਲਾ[j], zRep, nRep);
      j += nRep;
      i += n_ਨਮੂਨਾ-1;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( j+nStr-i+1<=n_ਬਾਹਰਲਾ );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&z_ਬਾਹਰਲਾ[j], &zStr[i], nStr-i);
  j += nStr - i;
  ਪ੍ਰਤਿਜ੍ਞਾ( j<=n_ਬਾਹਰਲਾ );
  z_ਬਾਹਰਲਾ[j] = 0;
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, (ਅੱਖਰ*)z_ਬਾਹਰਲਾ, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
ਸਥਿਰ ਖਾਲੀ trimFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਵਿੱਚ;         /* Input string */
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_Char_ਸੈੱਟ;    /* Set of characters to trim */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_ਵਿੱਚ;                 /* Number of bytes in input */
  ਪੂਰਨ flags;                        /* 1: trimleft  2: trimright  3: trim */
  ਪੂਰਨ i;                            /* Loop counter */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *aLen = 0;           /* Length of each character in zCharSet */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **azChar = 0;       /* Individual characters in zCharSet */
  ਪੂਰਨ nChar;                        /* Number of characters in zCharSet */

  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ){
    ਵਾਪਸ;
  }
  z_ਵਿੱਚ = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਜੇ( z_ਵਿੱਚ==0 ) ਵਾਪਸ;
  n_ਵਿੱਚ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਵਿੱਚ==sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]) );
  ਜੇ( argc==1 ){
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ lenOne[] = { 1 };
    ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ * ਅਚਲ azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (ਬਿਨਾਨਿਸ਼ਾਨੀ*)lenOne;
    azChar = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **)azOne;
    z_Char_ਸੈੱਟ = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( (z_Char_ਸੈੱਟ = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]))==0 ){
    ਵਾਪਸ;
  }ਨਹੀਂਤਾਂ{
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z;
    ਲਈ(z=z_Char_ਸੈੱਟ, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    ਜੇ( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(ਆਕਾਰ(ਅੱਖਰ*)+ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ)));
      ਜੇ( azChar==0 ){
        ਵਾਪਸ;
      }
      aLen = (ਬਿਨਾਨਿਸ਼ਾਨੀ*)&azChar[nChar];
      ਲਈ(z=z_Char_ਸੈੱਟ, nChar=0; *z; nChar++){
        azChar[nChar] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(z - azChar[nChar]);
      }
    }
  }
  ਜੇ( nChar>0 ){
    flags = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(context));
    ਜੇ( flags & 1 ){
      ਜਦੋਂ( n_ਵਿੱਚ>0 ){
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len = 0;
        ਲਈ(i=0; i<nChar; i++){
          len = aLen[i];
          ਜੇ( len<=n_ਵਿੱਚ && memcmp(z_ਵਿੱਚ, azChar[i], len)==0 ) ਰੋਕੋ;
        }
        ਜੇ( i>=nChar ) ਰੋਕੋ;
        z_ਵਿੱਚ += len;
        n_ਵਿੱਚ -= len;
      }
    }
    ਜੇ( flags & 2 ){
      ਜਦੋਂ( n_ਵਿੱਚ>0 ){
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len = 0;
        ਲਈ(i=0; i<nChar; i++){
          len = aLen[i];
          ਜੇ( len<=n_ਵਿੱਚ && memcmp(&z_ਵਿੱਚ[n_ਵਿੱਚ-len],azChar[i],len)==0 ) ਰੋਕੋ;
        }
        ਜੇ( i>=nChar ) ਰੋਕੋ;
        n_ਵਿੱਚ -= len;
      }
    }
    ਜੇ( z_Char_ਸੈੱਟ ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, (ਅੱਖਰ*)z_ਵਿੱਚ, n_ਵਿੱਚ, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
ਸਥਿਰ ਖਾਲੀ concatFuncCore(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv,
  ਪੂਰਨ nSep,
  ਅਚਲ ਅੱਖਰ *zSep
){
  i64 j, n = 0;
  ਪੂਰਨ i;
  ਪੂਰਨ bNotNull = 0;   /* True after at least NOT NULL argument seen */
  ਅੱਖਰ *z;
  ਲਈ(i=0; i<argc; i++){
    n += sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  ਜੇ( z==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(context);
    ਵਾਪਸ;
  }
  j = 0;
  ਲਈ(i=0; i<argc; i++){
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[i])!=SQLITE_NULL ){
      ਪੂਰਨ k = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[i]);
      ਅਚਲ ਅੱਖਰ *v = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
      ਜੇ( v!=0 ){
        ਜੇ( bNotNull && nSep>0 ){
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&z[j], zSep, nSep);
          j += nSep;
        }
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( j<=n );
  sqlite3_ਨਤੀਜਾ_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
ਸਥਿਰ ਖਾਲੀ concatFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
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
ਸਥਿਰ ਖਾਲੀ concatwsFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ nSep = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  ਅਚਲ ਅੱਖਰ *zSep = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਜੇ( zSep==0 ) ਵਾਪਸ;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
ਸਥਿਰ ਖਾਲੀ unknownFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  /* no-op */
  (ਖਾਲੀ)context;
  (ਖਾਲੀ)argc;
  (ਖਾਲੀ)argv;
}
#ਜੇ_ਅੰਤ /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
ਸਥਿਰ ਖਾਲੀ soundexFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅੱਖਰ z_ਨਤੀਜਾ[8];
  ਅਚਲ u8 *z_ਵਿੱਚ;
  ਪੂਰਨ i, j;
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ i_ਕੋਡ[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  z_ਵਿੱਚ = (u8*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਜੇ( z_ਵਿੱਚ==0 ) z_ਵਿੱਚ = (u8*)"";
  ਲਈ(i=0; z_ਵਿੱਚ[i] && !sqlite3Isalpha(z_ਵਿੱਚ[i]); i++){}
  ਜੇ( z_ਵਿੱਚ[i] ){
    u8 prevcode = i_ਕੋਡ[z_ਵਿੱਚ[i]&0x7f];
    z_ਨਤੀਜਾ[0] = sqlite3Toupper(z_ਵਿੱਚ[i]);
    ਲਈ(j=1; j<4 && z_ਵਿੱਚ[i]; i++){
      ਪੂਰਨ ਕੋਡ = i_ਕੋਡ[z_ਵਿੱਚ[i]&0x7f];
      ਜੇ( ਕੋਡ>0 ){
        ਜੇ( ਕੋਡ!=prevcode ){
          prevcode = ਕੋਡ;
          z_ਨਤੀਜਾ[j++] = ਕੋਡ + '0';
        }
      }ਨਹੀਂਤਾਂ{
        prevcode = 0;
      }
    }
    ਜਦੋਂ( j<4 ){
      z_ਨਤੀਜਾ[j++] = '0';
    }
    z_ਨਤੀਜਾ[j] = 0;
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, z_ਨਤੀਜਾ, 4, SQLITE_TRANSIENT);
  }ਨਹੀਂਤਾਂ{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, "?000", 4, SQLITE_STATIC);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_SOUNDEX */

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
ਸਥਿਰ ਖਾਲੀ oomFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Ext(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਅਚਲ ਅੱਖਰ *zFile = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅਚਲ ਅੱਖਰ *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  ਜੇ( (db->flags & SQLITE_ਲੱਦੋ_Ext_Func)==0 ){
    sqlite3_ਨਤੀਜਾ_error(context, "not authorized", -1);
    ਵਾਪਸ;
  }

  ਜੇ( argc==2 ){
    zProc = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  }ਨਹੀਂਤਾਂ{
    zProc = 0;
  }
  ਜੇ( zFile && sqlite3_ਲੱਦੋ_extension(db, zFile, zProc, &z_Err_ਸੁਨੇਹਾ) ){
    sqlite3_ਨਤੀਜਾ_error(context, z_Err_ਸੁਨੇਹਾ, -1);
    sqlite3_free(z_Err_ਸੁਨੇਹਾ);
  }
}
#ਜੇ_ਅੰਤ


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਜੋੜ_Ctx ਜੋੜ_Ctx;
ਬਣਤਰ ਜੋੜ_Ctx {
  ਦੁੱਗਣਾ r_ਜੋੜ;      /* Running sum as as a double */
  ਦੁੱਗਣਾ rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 i_ਜੋੜ;         /* Running sum as a signed integer */
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
ਸਥਿਰ ਖਾਲੀ kahan_Babuska_Neumaier_ਕਦਮ(
  ਅਸਥਿਰ ਜੋੜ_Ctx *p_ਜੋੜ,
  ਅਸਥਿਰ ਦੁੱਗਣਾ r
){
  ਅਸਥਿਰ ਦੁੱਗਣਾ s = p_ਜੋੜ->r_ਜੋੜ;
  ਅਸਥਿਰ ਦੁੱਗਣਾ t = s + r;
  ਜੇ( fabs(s) > fabs(r) ){
    p_ਜੋੜ->rErr += (s - t) + r;
  }ਨਹੀਂਤਾਂ{
    p_ਜੋੜ->rErr += (r - t) + s;
  }
  p_ਜੋੜ->r_ਜੋੜ = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
ਸਥਿਰ ਖਾਲੀ kahan_Babuska_Neumaier_ਕਦਮ_Int64(ਅਸਥਿਰ ਜੋੜ_Ctx *p_ਜੋੜ, i64 iVal){
  ਜੇ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahan_Babuska_Neumaier_ਕਦਮ(p_ਜੋੜ, iBig);
    kahan_Babuska_Neumaier_ਕਦਮ(p_ਜੋੜ, iSm);
  }ਨਹੀਂਤਾਂ{
    kahan_Babuska_Neumaier_ਕਦਮ(p_ਜੋੜ, (ਦੁੱਗਣਾ)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
ਸਥਿਰ ਖਾਲੀ kahanBabuskaNeumaierInit(
  ਅਸਥਿਰ ਜੋੜ_Ctx *p,
  i64 iVal
){
  ਜੇ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_ਜੋੜ = (ਦੁੱਗਣਾ)(iVal - iSm);
    p->rErr = (ਦੁੱਗਣਾ)iSm;
  }ਨਹੀਂਤਾਂ{
    p->r_ਜੋੜ = (ਦੁੱਗਣਾ)iVal;
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
ਸਥਿਰ ਖਾਲੀ ਜੋੜ_ਕਦਮ(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਜੋੜ_Ctx *p;
  ਪੂਰਨ type;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, ਆਕਾਰ(*p));
  type = sqlite3_ਮੁੱਲ_numeric_type(argv[0]);
  ਜੇ( p && type!=SQLITE_NULL ){
    p->cnt++;
    ਜੇ( p->approx==0 ){
      ਜੇ( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_ਜੋੜ);
        p->approx = 1;
        kahan_Babuska_Neumaier_ਕਦਮ(p, sqlite3_ਮੁੱਲ_double(argv[0]));
      }ਨਹੀਂਤਾਂ{
        i64 x = p->i_ਜੋੜ;
        ਜੇ( sqlite3_ਜੋੜੋ_Int64(&x, sqlite3_ਮੁੱਲ_int64(argv[0]))==0 ){
          p->i_ਜੋੜ = x;
        }ਨਹੀਂਤਾਂ{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_ਜੋੜ);
          p->approx = 1;
          kahan_Babuska_Neumaier_ਕਦਮ_Int64(p, sqlite3_ਮੁੱਲ_int64(argv[0]));
        }
      }
    }ਨਹੀਂਤਾਂ{
      ਜੇ( type==SQLITE_INTEGER ){
        kahan_Babuska_Neumaier_ਕਦਮ_Int64(p, sqlite3_ਮੁੱਲ_int64(argv[0]));
      }ਨਹੀਂਤਾਂ{
        p->ovrfl = 0;
        kahan_Babuska_Neumaier_ਕਦਮ(p, sqlite3_ਮੁੱਲ_double(argv[0]));
      }
    }
  }
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਸਥਿਰ ਖਾਲੀ ਜੋੜ_Inverse(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ**argv){
  ਜੋੜ_Ctx *p;
  ਪੂਰਨ type;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, ਆਕਾਰ(*p));
  type = sqlite3_ਮੁੱਲ_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  ਜੇ( ALWAYS(p) && type!=SQLITE_NULL ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->cnt>0 );
    p->cnt--;
    ਜੇ( !p->approx ){
      i64 x = p->i_ਜੋੜ;
      ਜੇ( sqlite3SubInt64(&x, sqlite3_ਮੁੱਲ_int64(argv[0]))==0 ){
        p->i_ਜੋੜ = x;
        ਵਾਪਸ;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_ਜੋੜ);
    }
    ਜੇ( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_ਮੁੱਲ_int64(argv[0]);
      ਜੇ( iVal!=ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 ){
        kahan_Babuska_Neumaier_ਕਦਮ_Int64(p, -iVal);
      }ਨਹੀਂਤਾਂ{
        kahan_Babuska_Neumaier_ਕਦਮ_Int64(p, ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64);
        kahan_Babuska_Neumaier_ਕਦਮ_Int64(p, 1);
      }       
    }ਨਹੀਂਤਾਂ{
      kahan_Babuska_Neumaier_ਕਦਮ(p, -sqlite3_ਮੁੱਲ_double(argv[0]));
    }
  }
}
#ਨਹੀਂਤਾਂ
# define ਜੋੜ_Inverse 0
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
ਸਥਿਰ ਖਾਲੀ ਜੋੜ_Finalize(sqlite3_context *context){
  ਜੋੜ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  ਜੇ( p && p->cnt>0 ){
    ਜੇ( p->approx ){
      ਜੇ( p->ovrfl ){
        sqlite3_ਨਤੀਜਾ_error(context,"integer overflow",-1);
      }ਨਹੀਂਤਾਂ ਜੇ( !sqlite3_Is_ਉਛਾਲ(p->rErr) ){
        sqlite3_ਨਤੀਜਾ_double(context, p->r_ਜੋੜ+p->rErr);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_double(context, p->r_ਜੋੜ);
      }
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_int64(context, p->i_ਜੋੜ);
    }
  }
}
ਸਥਿਰ ਖਾਲੀ avgFinalize(sqlite3_context *context){
  ਜੋੜ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  ਜੇ( p && p->cnt>0 ){
    ਦੁੱਗਣਾ r;
    ਜੇ( p->approx ){
      r = p->r_ਜੋੜ;
      ਜੇ( !sqlite3_Is_ਉਛਾਲ(p->rErr) ) r += p->rErr;
    }ਨਹੀਂਤਾਂ{
      r = (ਦੁੱਗਣਾ)(p->i_ਜੋੜ);
    }
    sqlite3_ਨਤੀਜਾ_double(context, r/(ਦੁੱਗਣਾ)p->cnt);
  }
}
ਸਥਿਰ ਖਾਲੀ ਕੁੱਲ_Finalize(sqlite3_context *context){
  ਜੋੜ_Ctx *p;
  ਦੁੱਗਣਾ r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  ਜੇ( p ){
    ਜੇ( p->approx ){
      r = p->r_ਜੋੜ;
      ਜੇ( !sqlite3_Is_ਉਛਾਲ(p->rErr) ) r += p->rErr;
    }ਨਹੀਂਤਾਂ{
      r = (ਦੁੱਗਣਾ)(p->i_ਜੋੜ);
    }
  }
  sqlite3_ਨਤੀਜਾ_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਗਿਣਤੀ_Ctx ਗਿਣਤੀ_Ctx;
ਬਣਤਰ ਗਿਣਤੀ_Ctx {
  i64 n;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪੂਰਨ bInverse;                   /* True if xInverse() ever called */
#ਜੇ_ਅੰਤ
};

/*
** Routines to implement the count() aggregate function.
*/
ਸਥਿਰ ਖਾਲੀ ਗਿਣਤੀ_ਕਦਮ(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਗਿਣਤੀ_Ctx *p;
  p = sqlite3_aggregate_context(context, ਆਕਾਰ(*p));
  ਜੇ( (argc==0 || SQLITE_NULL!=sqlite3_ਮੁੱਲ_type(argv[0])) && p ){
    p->n++;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_ਗਿਣਤੀ(context) );
#ਜੇ_ਅੰਤ
}  
ਸਥਿਰ ਖਾਲੀ ਗਿਣਤੀ_Finalize(sqlite3_context *context){
  ਗਿਣਤੀ_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_ਨਤੀਜਾ_int64(context, p ? p->n : 0);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਸਥਿਰ ਖਾਲੀ ਗਿਣਤੀ_Inverse(sqlite3_context *ctx, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਗਿਣਤੀ_Ctx *p;
  p = sqlite3_aggregate_context(ctx, ਆਕਾਰ(*p));
  /* p is always non-NULL since countStep() will have been called first */
  ਜੇ( (argc==0 || SQLITE_NULL!=sqlite3_ਮੁੱਲ_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    p->bInverse = 1;
#ਜੇ_ਅੰਤ
  }
}  
#ਨਹੀਂਤਾਂ
# define ਗਿਣਤੀ_Inverse 0
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
ਸਥਿਰ ਖਾਲੀ minmax_ਕਦਮ(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *p_ਉੱਤਮ;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  p_ਉੱਤਮ = (Mem *)sqlite3_aggregate_context(context, ਆਕਾਰ(*p_ਉੱਤਮ));
  ਜੇ( !p_ਉੱਤਮ ) ਵਾਪਸ;

  ਜੇ( sqlite3_ਮੁੱਲ_type(pArg)==SQLITE_NULL ){
    ਜੇ( p_ਉੱਤਮ->flags ) sqlite3_Skip_ਸੰਚਾਇਕ_ਲੱਦੋ(context);
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਉੱਤਮ->flags ){
    ਪੂਰਨ max;
    ਪੂਰਨ cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_ਅੰਕੜੇ(context)!=0;
    cmp = sqlite3MemCompare(p_ਉੱਤਮ, pArg, pColl);
    ਜੇ( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(p_ਉੱਤਮ, pArg);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Skip_ਸੰਚਾਇਕ_ਲੱਦੋ(context);
    }
  }ਨਹੀਂਤਾਂ{
    p_ਉੱਤਮ->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(p_ਉੱਤਮ, pArg);
  }
}
ਸਥਿਰ ਖਾਲੀ min_Max_ਮੁੱਲ_Finalize(sqlite3_context *context, ਪੂਰਨ b_ਮੁੱਲ){
  sqlite3_ਮੁੱਲ *pRes;
  pRes = (sqlite3_ਮੁੱਲ *)sqlite3_aggregate_context(context, 0);
  ਜੇ( pRes ){
    ਜੇ( pRes->flags ){
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, pRes);
    }
    ਜੇ( b_ਮੁੱਲ==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਸਥਿਰ ਖਾਲੀ min_Max_ਮੁੱਲ(sqlite3_context *context){
  min_Max_ਮੁੱਲ_Finalize(context, 1);
}
#ਨਹੀਂਤਾਂ
# define min_Max_ਮੁੱਲ 0
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
ਸਥਿਰ ਖਾਲੀ minMaxFinalize(sqlite3_context *context){
  min_Max_ਮੁੱਲ_Finalize(context, 0);
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  StrAccum str;          /* The accumulated concatenation */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਪੂਰਨ nAccum;            /* Number of strings presently concatenated */
  ਪੂਰਨ n_ਪਹਿਲਾ_Sep_ਲੰਮਾਈ;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  ਪੂਰਨ *pnSepLengths;
#ਜੇ_ਅੰਤ
} ਸਮੂਹ_Concat_Ctx;

ਸਥਿਰ ਖਾਲੀ ਸਮੂਹ_Concat_ਕਦਮ(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਅੱਖਰ *zVal;
  ਸਮੂਹ_Concat_Ctx *pGCC;
  ਅਚਲ ਅੱਖਰ *zSep;
  ਪੂਰਨ nVal, nSep;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 || argc==2 );
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ) ਵਾਪਸ;
  pGCC = (ਸਮੂਹ_Concat_Ctx*)sqlite3_aggregate_context(context, ਆਕਾਰ(*pGCC));
  ਜੇ( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    ਪੂਰਨ ਪਹਿਲਾ_ਪਦ = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ];
    ਜੇ( argc==1 ){
      ਜੇ( !ਪਹਿਲਾ_ਪਦ ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਨਹੀਂਤਾਂ{
        pGCC->n_ਪਹਿਲਾ_Sep_ਲੰਮਾਈ = 1;
      }
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ ਜੇ( !ਪਹਿਲਾ_ਪਦ ){
      zSep = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
      nSep = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[1]);
      ਜੇ( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਨਹੀਂਤਾਂ{
        nSep = 0;
      }
      ਜੇ( nSep != pGCC->n_ਪਹਿਲਾ_Sep_ਲੰਮਾਈ || pGCC->pnSepLengths != 0 ){
        ਪੂਰਨ *pnsl = pGCC->pnSepLengths;
        ਜੇ( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (ਪੂਰਨ*)sqlite3_malloc64((pGCC->nAccum+1) * ਆਕਾਰ(ਪੂਰਨ));
          ਜੇ( pnsl!=0 ){
            ਪੂਰਨ i = 0, nA = pGCC->nAccum-1;
            ਜਦੋਂ( i<nA ) pnsl[i++] = pGCC->n_ਪਹਿਲਾ_Sep_ਲੰਮਾਈ;
          }
        }ਨਹੀਂਤਾਂ{
          pnsl = (ਪੂਰਨ*)sqlite3_realloc64(pnsl, pGCC->nAccum * ਆਕਾਰ(ਪੂਰਨ));
        }
        ਜੇ( pnsl!=0 ){
          ਜੇ( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }ਨਹੀਂਤਾਂ{
          sqlite3_Str_Accum_ਸੈੱਟ_Error(&pGCC->str, SQLITE_NOMEM);
        }
      }
#ਜੇ_ਅੰਤ
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਨਹੀਂਤਾਂ{
      pGCC->n_ਪਹਿਲਾ_Sep_ਲੰਮਾਈ = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[1]);
    }
    pGCC->nAccum += 1;
#ਜੇ_ਅੰਤ
    zVal = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    nVal = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
    ਜੇ( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਸਥਿਰ ਖਾਲੀ ਸਮੂਹ_Concat_Inverse(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਸਮੂਹ_Concat_Ctx *pGCC;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 || argc==2 );
  (ਖਾਲੀ)argc;  /* Suppress unused parameter warning */
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ) ਵਾਪਸ;
  pGCC = (ਸਮੂਹ_Concat_Ctx*)sqlite3_aggregate_context(context, ਆਕਾਰ(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  ਜੇ( ALWAYS(pGCC) ){
    ਪੂਰਨ nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (ਖਾਲੀ)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    nVS = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
    pGCC->nAccum -= 1;
    ਜੇ( pGCC->pnSepLengths!=0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ(pGCC->nAccum >= 0);
      ਜੇ( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*ਆਕਾਰ(ਪੂਰਨ));
      }
    }ਨਹੀਂਤਾਂ{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_ਪਹਿਲਾ_Sep_ਲੰਮਾਈ;
    }
    ਜੇ( nVS>=(ਪੂਰਨ)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }ਨਹੀਂਤਾਂ{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.z_ਪਾਠ, &pGCC->str.z_ਪਾਠ[nVS], pGCC->str.nChar);
    }
    ਜੇ( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#ਨਹੀਂਤਾਂ
# define ਸਮੂਹ_Concat_Inverse 0
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
ਸਥਿਰ ਖਾਲੀ ਸਮੂਹ_Concat_Finalize(sqlite3_context *context){
  ਸਮੂਹ_Concat_Ctx *pGCC
    = (ਸਮੂਹ_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  ਜੇ( pGCC ){
    sqlite3_ਨਤੀਜਾ_str(context, &pGCC->str, SQLITE_XFER);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#ਜੇ_ਅੰਤ
  }
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਸਥਿਰ ਖਾਲੀ ਸਮੂਹ_Concat_ਮੁੱਲ(sqlite3_context *context){
  ਸਮੂਹ_Concat_Ctx *pGCC
    = (ਸਮੂਹ_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  ਜੇ( pGCC && pGCC->nAccum>0 ){
    sqlite3_ਨਤੀਜਾ_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#ਨਹੀਂਤਾਂ
# define ਸਮੂਹ_Concat_ਮੁੱਲ 0
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_ਪ੍ਰਤੀ_Connection_Builtin_Functions(sqlite3 *db){
  ਪੂਰਨ rc = sqlite3_overload_function(db, "MATCH", 2);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  ਜੇ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Like_Functions(sqlite3 *db, ਪੂਰਨ caseSensitive){
  FuncDef *pDef;
  ਬਣਤਰ compareInfo *pInfo;
  ਪੂਰਨ flags;
  ਪੂਰਨ nArg;
  ਜੇ( caseSensitive ){
    pInfo = (ਬਣਤਰ compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }ਨਹੀਂਤਾਂ{
    pInfo = (ਬਣਤਰ compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  ਲਈ(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3_ਲੱਭੋ_Function(db, "like", nArg, SQLITE_UTF8, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
ਪੂਰਨ sqlite3IsLikeFunction(sqlite3 *db, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ *pIsNocase, ਅੱਖਰ *aWc){
  FuncDef *pDef;
  ਪੂਰਨ n_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  ਜੇ( !p_ਵਿਅੰਜਕ->x.pList ){
    ਵਾਪਸ 0;
  }
  n_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
  pDef = sqlite3_ਲੱਭੋ_Function(db, p_ਵਿਅੰਜਕ->u.zToken, n_ਵਿਅੰਜਕ, SQLITE_UTF8, 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ਜੇ( pDef==0 ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਜੇ( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    ਵਾਪਸ 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(aWc, pDef->p_User_ਅੰਕੜੇ, 3);
  ਪ੍ਰਤਿਜ੍ਞਾ( (ਅੱਖਰ*)&likeInfoAlt == (ਅੱਖਰ*)&likeInfoAlt.ਮੇਲ_All );
  ਪ੍ਰਤਿਜ੍ਞਾ( &((ਅੱਖਰ*)&likeInfoAlt)[1] == (ਅੱਖਰ*)&likeInfoAlt.ਮੇਲ_One );
  ਪ੍ਰਤਿਜ੍ਞਾ( &((ਅੱਖਰ*)&likeInfoAlt)[2] == (ਅੱਖਰ*)&likeInfoAlt.ਮੇਲ_ਸੈੱਟ );

  ਜੇ( n_ਵਿਅੰਜਕ<3 ){
    aWc[3] = 0;
  }ਨਹੀਂਤਾਂ{
    ਵਿਅੰਜਕ *pEscape = p_ਵਿਅੰਜਕ->x.pList->a[2].p_ਵਿਅੰਜਕ;
    ਅੱਖਰ *zEscape;
    ਜੇ( pEscape->op!=TK_STRING ) ਵਾਪਸ 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pEscape, EP_Int_ਮੁੱਲ) );
    zEscape = pEscape->u.zToken;
    ਜੇ( zEscape[0]==0 || zEscape[1]!=0 ) ਵਾਪਸ 0;
    ਜੇ( zEscape[0]==aWc[0] ) ਵਾਪਸ 0;
    ਜੇ( zEscape[0]==aWc[1] ) ਵਾਪਸ 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  ਵਾਪਸ 1;
}

/* Mathematical Constants */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ M_PI
# define M_PI   3.141592653589793238462643383279502884
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#ਜੇ_ਅੰਤ


/* Extra math functions that require linking with -lm
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MATH_FUNCTIONS
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
ਸਥਿਰ ਖਾਲੀ ceilingFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਚੋਣ( sqlite3_ਮੁੱਲ_numeric_type(argv[0]) ){
    ਹਾਲਤ SQLITE_INTEGER: {
       sqlite3_ਨਤੀਜਾ_int64(context, sqlite3_ਮੁੱਲ_int64(argv[0]));
       ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_FLOAT: {
       ਦੁੱਗਣਾ (*x)(ਦੁੱਗਣਾ) = (ਦੁੱਗਣਾ(*)(ਦੁੱਗਣਾ))sqlite3_user_ਅੰਕੜੇ(context);
       sqlite3_ਨਤੀਜਾ_double(context, x(sqlite3_ਮੁੱਲ_double(argv[0])));
       ਰੋਕੋ;
    }
    ਮੂਲ: {
       ਰੋਕੋ;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
ਸਥਿਰ ਦੁੱਗਣਾ xCeil(ਦੁੱਗਣਾ x){ ਵਾਪਸ ceil(x); }
ਸਥਿਰ ਦੁੱਗਣਾ xFloor(ਦੁੱਗਣਾ x){ ਵਾਪਸ floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#ਜੇ defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*ਲਘੂਗਣਕ(X))
#ਜੇ_ਅੰਤ
#ਜੇ defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*ਲਘੂਗਣਕ(X))
#ਜੇ_ਅੰਤ


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
ਸਥਿਰ ਖਾਲੀ logFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਦੁੱਗਣਾ x, b, ans;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 || argc==2 );
  ਚੋਣ( sqlite3_ਮੁੱਲ_numeric_type(argv[0]) ){
    ਹਾਲਤ SQLITE_INTEGER:
    ਹਾਲਤ SQLITE_FLOAT:
      x = sqlite3_ਮੁੱਲ_double(argv[0]);
      ਜੇ( x<=0.0 ) ਵਾਪਸ;
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ;
  }
  ਜੇ( argc==2 ){
    ਚੋਣ( sqlite3_ਮੁੱਲ_numeric_type(argv[1]) ){
      ਹਾਲਤ SQLITE_INTEGER:
      ਹਾਲਤ SQLITE_FLOAT:
        b = ਲਘੂਗਣਕ(x);
        ਜੇ( b<=0.0 ) ਵਾਪਸ;
        x = sqlite3_ਮੁੱਲ_double(argv[1]);
        ਜੇ( x<=0.0 ) ਵਾਪਸ;
        ਰੋਕੋ;
     ਮੂਲ:
        ਵਾਪਸ;
    }
    ans = ਲਘੂਗਣਕ(x)/b;
  }ਨਹੀਂਤਾਂ{
    ਚੋਣ( SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(context)) ){
      ਹਾਲਤ 1:
        ans = log10(x);
        ਰੋਕੋ;
      ਹਾਲਤ 2:
        ans = log2(x);
        ਰੋਕੋ;
      ਮੂਲ:
        ans = ਲਘੂਗਣਕ(x);
        ਰੋਕੋ;
    }
  }
  sqlite3_ਨਤੀਜਾ_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
ਸਥਿਰ ਦੁੱਗਣਾ deg_ਮੰਜ਼ਿਲ_Rad(ਦੁੱਗਣਾ x){ ਵਾਪਸ x*(M_PI/180.0); }
ਸਥਿਰ ਦੁੱਗਣਾ rad_ਮੰਜ਼ਿਲ_Deg(ਦੁੱਗਣਾ x){ ਵਾਪਸ x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
ਸਥਿਰ ਖਾਲੀ math1Func(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ type0;
  ਦੁੱਗਣਾ v0, ans;
  ਦੁੱਗਣਾ (*x)(ਦੁੱਗਣਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  type0 = sqlite3_ਮੁੱਲ_numeric_type(argv[0]);
  ਜੇ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ਵਾਪਸ;
  v0 = sqlite3_ਮੁੱਲ_double(argv[0]);
  x = (ਦੁੱਗਣਾ(*)(ਦੁੱਗਣਾ))sqlite3_user_ਅੰਕੜੇ(context);
  ans = x(v0);
  sqlite3_ਨਤੀਜਾ_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
ਸਥਿਰ ਖਾਲੀ math2Func(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ type0, type1;
  ਦੁੱਗਣਾ v0, v1, ans;
  ਦੁੱਗਣਾ (*x)(ਦੁੱਗਣਾ,ਦੁੱਗਣਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==2 );
  type0 = sqlite3_ਮੁੱਲ_numeric_type(argv[0]);
  ਜੇ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ਵਾਪਸ;
  type1 = sqlite3_ਮੁੱਲ_numeric_type(argv[1]);
  ਜੇ( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) ਵਾਪਸ;
  v0 = sqlite3_ਮੁੱਲ_double(argv[0]);
  v1 = sqlite3_ਮੁੱਲ_double(argv[1]);
  x = (ਦੁੱਗਣਾ(*)(ਦੁੱਗਣਾ,ਦੁੱਗਣਾ))sqlite3_user_ਅੰਕੜੇ(context);
  ans = x(v0, v1);
  sqlite3_ਨਤੀਜਾ_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
ਸਥਿਰ ਖਾਲੀ piFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==0 );
  (ਖਾਲੀ)argv;
  sqlite3_ਨਤੀਜਾ_double(context, M_PI);
}

#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
ਸਥਿਰ ਖਾਲੀ signFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ type0;
  ਦੁੱਗਣਾ x;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  type0 = sqlite3_ਮੁੱਲ_numeric_type(argv[0]);
  ਜੇ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) ਵਾਪਸ;
  x = sqlite3_ਮੁੱਲ_double(argv[0]);
  sqlite3_ਨਤੀਜਾ_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#ਜੇ defined(SQLITE_ENABLE_PERCENTILE)
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Percentile Percentile;
ਬਣਤਰ Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 n_ਵਰਤਿਆ;           /* Number of slots actually used in a[] */
  ਅੱਖਰ bSorted;        /* True if a[] is already in sorted order */
  ਅੱਖਰ bKeepSorted;    /* True if advantageous to keep a[] sorted */
  ਅੱਖਰ bPctValid;      /* True if rPct is valid */
  ਦੁੱਗਣਾ rPct;         /* Fraction.  0.0 to 1.0 */
  ਦੁੱਗਣਾ *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
ਸਥਿਰ ਪੂਰਨ percentIsInfinity(ਦੁੱਗਣਾ r){
  sqlite3_uint64 u;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(u)==ਆਕਾਰ(r) );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&u, &r, ਆਕਾਰ(u));
  ਵਾਪਸ ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
ਸਥਿਰ ਪੂਰਨ percent_Same_ਮੁੱਲ(ਦੁੱਗਣਾ a, ਦੁੱਗਣਾ b){
  a -= b;
  ਵਾਪਸ a>=-0.001 && a<=0.001;
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
ਸਥਿਰ i64 percent_ਦੁਆਧਾਰੀ_ਖੋਜ(Percentile *p, ਦੁੱਗਣਾ y, ਪੂਰਨ bExact){
  i64 i_ਪਹਿਲਾ = 0;                   /* First element of search range */
  i64 i_ਆਖਰੀ = (i64)p->n_ਵਰਤਿਆ - 1;    /* Last element of search range */
  ਜਦੋਂ( i_ਆਖਰੀ>=i_ਪਹਿਲਾ ){
    i64 iMid = (i_ਪਹਿਲਾ+i_ਆਖਰੀ)/2;
    ਦੁੱਗਣਾ x = p->a[iMid];
    ਜੇ( x<y ){
      i_ਪਹਿਲਾ = iMid + 1;
    }ਨਹੀਂਤਾਂ ਜੇ( x>y ){
      i_ਆਖਰੀ = iMid - 1;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ iMid;
    }
  }
  ਜੇ( bExact ) ਵਾਪਸ -1;
  ਵਾਪਸ i_ਪਹਿਲਾ;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
ਸਥਿਰ ਖਾਲੀ percentError(sqlite3_context *pCtx, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  ਅੱਖਰ *zMsg1;
  ਅੱਖਰ *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_ਨਤੀਜਾ_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
ਸਥਿਰ ਖਾਲੀ percent_ਕਦਮ(sqlite3_context *pCtx, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  Percentile *p;
  ਦੁੱਗਣਾ rPct;
  ਪੂਰਨ eType;
  ਦੁੱਗਣਾ y;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==2 || argc==1 );

  ਜੇ( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }ਨਹੀਂਤਾਂ{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    ਦੁੱਗਣਾ mxFrac = (SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_ਮੁੱਲ_numeric_type(argv[1]);
    rPct = sqlite3_ਮੁੱਲ_double(argv[1])/mxFrac;
    ਜੇ( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (ਦੁੱਗਣਾ)mxFrac);
      ਵਾਪਸ;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਵਾਪਸ;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  ਜੇ( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( !percent_Same_ਮੁੱਲ(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    ਵਾਪਸ;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_ਮੁੱਲ_type(argv[0]);
  ਜੇ( eType==SQLITE_NULL ) ਵਾਪਸ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ਜੇ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    ਵਾਪਸ;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_ਮੁੱਲ_double(argv[0]);
  ਜੇ( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    ਵਾਪਸ;
  }

  /* Allocate and store the Y */
  ਜੇ( p->n_ਵਰਤਿਆ>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    ਦੁੱਗਣਾ *a = sqlite3_realloc64(p->a, ਆਕਾਰ(ਦੁੱਗਣਾ)*n);
    ਜੇ( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, ਆਕਾਰ(*p));
      sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
      ਵਾਪਸ;
    }
    p->nAlloc = n;
    p->a = a;
  }
  ਜੇ( p->n_ਵਰਤਿਆ==0 ){
    p->a[p->n_ਵਰਤਿਆ++] = y;
    p->bSorted = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( !p->bSorted || y>=p->a[p->n_ਵਰਤਿਆ-1] ){
    p->a[p->n_ਵਰਤਿਆ++] = y;
  }ਨਹੀਂਤਾਂ ਜੇ( p->bKeepSorted ){
    i64 i;
    i = percent_ਦੁਆਧਾਰੀ_ਖੋਜ(p, y, 0);
    ਜੇ( i<(ਪੂਰਨ)p->n_ਵਰਤਿਆ ){
      memmove(&p->a[i+1], &p->a[i], (p->n_ਵਰਤਿਆ-i)*ਆਕਾਰ(p->a[0]));
    }
    p->a[i] = y;
    p->n_ਵਰਤਿਆ++;
  }ਨਹੀਂਤਾਂ{
    p->a[p->n_ਵਰਤਿਆ++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਦਲਾ_ਬਦਲੀ_DOUBLE(X,Y)  {ਦੁੱਗਣਾ ttt=(X);(X)=(Y);(Y)=ttt;}

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
ਸਥਿਰ ਖਾਲੀ percent_ਛਾਂਟੋ(
  ਦੁੱਗਣਾ *a,                      /* Array to sort */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n,                 /* Number of elements in array a[] */
  ਪੂਰਨ iReq                        /* Element caller cares about (or -ve) */
){
  ਪੂਰਨ iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  ਪੂਰਨ iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  ਪੂਰਨ i;         /* Loop counter */
  ਦੁੱਗਣਾ r_ਧੁਰਾ; /* The pivot value */

  ਪ੍ਰਤਿਜ੍ਞਾ( n>=2 );
  ਕਰੋ{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    ਜੇ( a[0]>a[n-1] ){
      ਅਦਲਾ_ਬਦਲੀ_DOUBLE(a[0],a[n-1])
    }
    ਜੇ( n==2 ) ਵਾਪਸ;
    iGt = n-1;
    i = n/2;
    ਜੇ( a[0]>a[i] ){
      ਅਦਲਾ_ਬਦਲੀ_DOUBLE(a[0],a[i])
    }ਨਹੀਂਤਾਂ ਜੇ( a[i]>a[iGt] ){
      ਅਦਲਾ_ਬਦਲੀ_DOUBLE(a[i],a[iGt])
    }
    ਜੇ( n==3 ) ਵਾਪਸ;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    r_ਧੁਰਾ = a[i];
    iLt = i = 1;
    ਕਰੋ{
      ਜੇ( a[i]<r_ਧੁਰਾ ){
        ਜੇ( i>iLt ) ਅਦਲਾ_ਬਦਲੀ_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }ਨਹੀਂਤਾਂ ਜੇ( a[i]>r_ਧੁਰਾ ){
        ਕਰੋ{
          iGt--;
        }ਜਦੋਂ( iGt>i && a[iGt]>r_ਧੁਰਾ );
        ਅਦਲਾ_ਬਦਲੀ_DOUBLE(a[i],a[iGt])
      }ਨਹੀਂਤਾਂ{
        i++;
      }
    }ਜਦੋਂ( i<iGt );

    ਪ੍ਰਤਿਜ੍ਞਾ( iLt>0 && iLt<iGt && (ਬਿਨਾਨਿਸ਼ਾਨੀ)iGt<n );
    testcase( iGt>iLt+1 );
    ਪ੍ਰਤਿਜ੍ਞਾ( a[iLt]==r_ਧੁਰਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( a[iLt-1]<=r_ਧੁਰਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( a[iGt]>=r_ਧੁਰਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( a[iLt+1]>=r_ਧੁਰਾ );

    ਜੇ( iReq>=0 ){
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
      ਜੇ( iReq<iLt ){
        n = iLt;
      }ਨਹੀਂਤਾਂ{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }ਨਹੀਂਤਾਂ{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      ਜੇ( iLt>(ਪੂਰਨ)(n/2) ){
        ਜੇ( n-iGt>=2 ) percent_ਛਾਂਟੋ(a+iGt, n-iGt, -1);
        n = iLt;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( iLt>=2 ) percent_ਛਾਂਟੋ(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }ਜਦੋਂ( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
ਸਥਿਰ ਖਾਲੀ percentInverse(sqlite3_context *pCtx,ਪੂਰਨ argc,sqlite3_ਮੁੱਲ **argv){
  Percentile *p;
  ਪੂਰਨ eType;
  ਦੁੱਗਣਾ y;
  i64 i;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, ਆਕਾਰ(*p));
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_ਮੁੱਲ_type(argv[0]);
  ਜੇ( eType==SQLITE_NULL ) ਵਾਪਸ;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ਜੇ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    ਵਾਪਸ;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_ਮੁੱਲ_double(argv[0]);
  ਜੇ( percentIsInfinity(y) ){
    ਵਾਪਸ;
  }
  ਜੇ( p->bSorted==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਵਰਤਿਆ>1 );
    percent_ਛਾਂਟੋ(p->a, p->n_ਵਰਤਿਆ, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_ਦੁਆਧਾਰੀ_ਖੋਜ(p, y, 1);
  ਜੇ( i>=0 ){
    p->n_ਵਰਤਿਆ--;
    ਜੇ( i<(ਪੂਰਨ)p->n_ਵਰਤਿਆ ){
      memmove(&p->a[i], &p->a[i+1], (p->n_ਵਰਤਿਆ - i)*ਆਕਾਰ(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
ਸਥਿਰ ਖਾਲੀ percentCompute(sqlite3_context *pCtx, ਪੂਰਨ bIsFinal){
  Percentile *p;
  ਪੂਰਨ settings = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(pCtx))&1; /* Discrete? */
  ਬਿਨਾਨਿਸ਼ਾਨੀ i1, i2;
  ਦੁੱਗਣਾ v1, v2;
  ਦੁੱਗਣਾ ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਜੇ( p->a==0 ) ਵਾਪਸ;
  ਜੇ( p->n_ਵਰਤਿਆ ){
    ix = p->rPct*(p->n_ਵਰਤਿਆ-1);
    i1 = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ix;
    ਜੇ( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਵਰਤਿਆ>1 );
      percent_ਛਾਂਟੋ(p->a, p->n_ਵਰਤਿਆ, (bIsFinal ? (ਪੂਰਨ)i1 : -1));
      p->bSorted = 1;
    }
    ਜੇ( settings & 1 ){
      vx = p->a[i1];
    }ਨਹੀਂਤਾਂ{
      i2 = ix==(ਦੁੱਗਣਾ)i1 || i1==p->n_ਵਰਤਿਆ-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_ਨਤੀਜਾ_double(pCtx, vx);
  }
  ਜੇ( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, ਆਕਾਰ(*p));
  }ਨਹੀਂਤਾਂ{
    p->bKeepSorted = 1;
  }
}
ਸਥਿਰ ਖਾਲੀ percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
ਸਥਿਰ ਖਾਲੀ percent_ਮੁੱਲ(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PERCENTILE */

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
ਸਥਿਰ ਖਾਲੀ filestatFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਚਲ ਅੱਖਰ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  pBtree = sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(db, zDbName);
  ਜੇ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    ਪੂਰਨ rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ਪ੍ਰਤਿਜ੍ਞਾ( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    ਜੇ( sqlite3_str_errcode(pStr) ){
      sqlite3_ਨਤੀਜਾ_error_nomem(context);
    }ਨਹੀਂਤਾਂ{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      ਜੇ( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      ਜੇ( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        ਜੇ( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_ਨਤੀਜਾ_str(context, pStr, SQLITE_ਅੰਤ);
    }
    sqlite3BtreeLeave(pBtree);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, "{}", 2, SQLITE_STATIC);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
ਸਥਿਰ ਖਾਲੀ walstatFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਚਲ ਅੱਖਰ *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  pBtree = sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(db, zDbName);
  ਜੇ( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ਪ੍ਰਤਿਜ੍ਞਾ( pPager!=0 );
    pStr = sqlite3_str_new(db);
    ਜੇ( sqlite3_str_errcode(pStr) ){
      sqlite3_ਨਤੀਜਾ_error_nomem(context);
    }ਨਹੀਂਤਾਂ{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_ਨਤੀਜਾ_str(context, pStr, SQLITE_ਅੰਤ);
    }
    sqlite3BtreeLeave(pBtree);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, "{}", 2, SQLITE_STATIC);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਖਾਲੀ fpdecodeFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  FpDecode s;
  ਦੁੱਗਣਾ x;
  ਪੂਰਨ y, z;
  ਅੱਖਰ zBuf[100];
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==3 );
  x = sqlite3_ਮੁੱਲ_double(argv[0]);
  y = sqlite3_ਮੁੱਲ_int(argv[1]);
  z = sqlite3_ਮੁੱਲ_int(argv[2]);
  ਜੇ( z<=0 ) z = 1;
  ਜੇ( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  ਜੇ( s.isSpecial==2 ){
    sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "NaN");
  }ਨਹੀਂਤਾਂ{
    sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, zBuf, -1, SQLITE_TRANSIENT);
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਖਾਲੀ parseuriFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_str *p_ਨਤੀਜਾ;
  ਅਚਲ ਅੱਖਰ *zVfs;
  ਅਚਲ ਅੱਖਰ *zUri;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flgs;
  ਪੂਰਨ rc;
  sqlite3_vfs *pVfs = 0;
  ਅੱਖਰ *zFile = 0;
  ਅੱਖਰ *zErr = 0;

  ਜੇ( argc<2 ) ਵਾਪਸ;
  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  ਪ੍ਰਤਿਜ੍ਞਾ( pVfs );
  zVfs = pVfs->zName;
  zUri = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਜੇ( zUri==0 ) ਵਾਪਸ;
  flgs = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)sqlite3_ਮੁੱਲ_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_ਨਤੀਜਾ = sqlite3_str_new(0);
  ਜੇ( !sqlite3_str_errcode(p_ਨਤੀਜਾ) ){
    ਪੂਰਨ i;
    sqlite3_str_appendf(p_ਨਤੀਜਾ, "rc=%d", rc);
    sqlite3_str_appendf(p_ਨਤੀਜਾ, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_ਨਤੀਜਾ, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_ਨਤੀਜਾ, ", err=%Q", zErr);
    sqlite3_str_appendf(p_ਨਤੀਜਾ, ", file=%Q", zFile);
    ਜੇ( zFile ){
      ਅਚਲ ਅੱਖਰ *z = zFile;
      z += sqlite3Strlen30(z)+1;
      ਜਦੋਂ( z[0] ){
        sqlite3_str_appendf(p_ਨਤੀਜਾ, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      ਲਈ(i=2; i<argc; i++){
        ਅਚਲ ਅੱਖਰ *zArg;
        ਜੇ( sqlite3_ਮੁੱਲ_type(argv[i])==SQLITE_INTEGER ){
          ਪੂਰਨ k = sqlite3_ਮੁੱਲ_int(argv[i]);
          sqlite3_str_appendf(p_ਨਤੀਜਾ, ", '%d:%q'",k,sqlite3_uri_ਕੁੰਜੀ(zFile, k));
        }ਨਹੀਂਤਾਂ ਜੇ( (zArg = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]))!=0 ){
          sqlite3_str_appendf(p_ਨਤੀਜਾ, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }ਨਹੀਂਤਾਂ{
          sqlite3_str_appendf(p_ਨਤੀਜਾ, ", NULL");
        }
      }
    }
  }
  sqlite3_ਨਤੀਜਾ_str(ctx, p_ਨਤੀਜਾ, SQLITE_ਅੰਤ);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Builtin_Functions(ਖਾਲੀ){
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
  ਸਥਿਰ FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#ਜੇ !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(ਸਿੱਟਾ_ਦਿੰਦਾ_nonnull_ਪੰਕਤੀ, 2, INLINEFUNC_ਸਿੱਟਾ_ਦਿੰਦਾ_nonnull_ਪੰਕਤੀ, 0),
    TEST_FUNC(ਵਿਅੰਜਕ_compare,        2, INLINEFUNC_ਵਿਅੰਜਕ_compare,        0),
    TEST_FUNC(ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ,   2, INLINEFUNC_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#ਜੇ_ਅੰਤ /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
    SFUNCTION(ਲੱਦੋ_extension,    1, 0, 0, ਲੱਦੋ_Ext          ),
    SFUNCTION(ਲੱਦੋ_extension,    2, 0, 0, ਲੱਦੋ_Ext          ),
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_ਵਰਤਿਆ,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਵਿੱਥ_SQL_FUNC
    INLINE_FUNC(sqlite_ਵਿੱਥ,   1, INLINEFUNC_sqlite_ਵਿੱਥ, 0 ),
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#ਜੇ_ਅੰਤ
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmax_ਕਦਮ, minMaxFinalize, min_Max_ਮੁੱਲ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmax_ਕਦਮ, minMaxFinalize, min_Max_ਮੁੱਲ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(ਲੰਮਾਈ,            1, 0, 0, ਲੰਮਾਈ_Func,  SQLITE_FUNC_ਲੰਮਾਈ),
    FUNCTION2(octet_ਲੰਮਾਈ,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(ਲਿਖੋ,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(ਅੱਖਰ,              -1, 0, 0, charFunc         ),
    FUNCTION(ਨਿਰਪੇਖ,                1, 0, 0, absFunc          ),
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
    FUNCTION(ਗੇੜਾ,              1, 0, 0, ਗੇੜਾ_Func        ),
    FUNCTION(ਗੇੜਾ,              2, 0, 0, ਗੇੜਾ_Func        ),
#ਜੇ_ਅੰਤ
    FUNCTION(ਉੱਤਲਾ,              1, 0, 0, ਉੱਤਲਾ_Func        ),
    FUNCTION(ਹੇਠਲਾ,              1, 0, 0, ਹੇਠਲਾ_Func        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_ਸੰਸਕਰਣ,    0, 0, 0, ਸੰਸਕਰਣ_Func      ),
    DFUNCTION(sqlite_ਸਰੋਤ_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(ਆਖਰੀ_ਪਾਓ_rowid, 0, 0, 0, ਆਖਰੀ_ਪਾਓ_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(ਕੁੱਲ_changes,     0, 0, 0, ਕੁੱਲ_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(ਜੋੜ,   1,0,0, ਜੋੜ_ਕਦਮ, ਜੋੜ_Finalize, ਜੋੜ_Finalize, ਜੋੜ_Inverse, 0),
    WAGGREGATE(ਕੁੱਲ, 1,0,0, ਜੋੜ_ਕਦਮ,ਕੁੱਲ_Finalize,ਕੁੱਲ_Finalize,ਜੋੜ_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, ਜੋੜ_ਕਦਮ, avgFinalize, avgFinalize, ਜੋੜ_Inverse, 0),
    WAGGREGATE(ਗਿਣਤੀ_2, 0,0,0, ਗਿਣਤੀ_ਕਦਮ,
        ਗਿਣਤੀ_Finalize, ਗਿਣਤੀ_Finalize, ਗਿਣਤੀ_Inverse,
        SQLITE_FUNC_ਗਿਣਤੀ|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(ਗਿਣਤੀ_2, 1,0,0, ਗਿਣਤੀ_ਕਦਮ,
        ਗਿਣਤੀ_Finalize, ਗਿਣਤੀ_Finalize, ਗਿਣਤੀ_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(ਸਮੂਹ_concat, 1, 0, 0, ਸਮੂਹ_Concat_ਕਦਮ,
        ਸਮੂਹ_Concat_Finalize, ਸਮੂਹ_Concat_ਮੁੱਲ, ਸਮੂਹ_Concat_Inverse, 0),
    WAGGREGATE(ਸਮੂਹ_concat, 2, 0, 0, ਸਮੂਹ_Concat_ਕਦਮ,
        ਸਮੂਹ_Concat_Finalize, ਸਮੂਹ_Concat_ਮੁੱਲ, ਸਮੂਹ_Concat_Inverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, ਸਮੂਹ_Concat_ਕਦਮ,
        ਸਮੂਹ_Concat_Finalize, ਸਮੂਹ_Concat_ਮੁੱਲ, ਸਮੂਹ_Concat_Inverse, 0),

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percent_ਕਦਮ,
        percentFinal, percent_ਮੁੱਲ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percent_ਕਦਮ,
        percentFinal, percent_ਮੁੱਲ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percent_ਕਦਮ,
        percentFinal, percent_ਮੁੱਲ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percent_ਕਦਮ,
        percentFinal, percent_ਮੁੱਲ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ਨਹੀਂਤਾਂ
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#ਜੇ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#ਜੇ_ਅੰਤ
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(ਲਘੂਗਣਕ,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(ਲਘੂਗਣਕ,                2, 0, 0,      logFunc     ),
    MFUNCTION(ਘਾਤਾਂਕ,               1, ਘਾਤਾਂਕ,       math1Func   ),
    MFUNCTION(ਘਾਤ,               2, ਘਾਤ,       math2Func   ),
    MFUNCTION(power,             2, ਘਾਤ,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(ਕੋਜੀਵਾ,               1, ਕੋਜੀਵਾ,       math1Func   ),
    MFUNCTION(ਜੀਵਾ,               1, ਜੀਵਾ,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#ਜੇ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#ਜੇ_ਅੰਤ
    MFUNCTION(ਵਰਗਮੂਲ,              1, ਵਰਗਮੂਲ,      math1Func   ),
    MFUNCTION(radians,           1, deg_ਮੰਜ਼ਿਲ_Rad,  math1Func   ),
    MFUNCTION(degrees,           1, rad_ਮੰਜ਼ਿਲ_Deg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(ਜੇ,             -4, INLINEFUNC_iif,      0 ),
  };
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#ਜੇ_ਅੰਤ
  sqlite3_ਖਿੜਕੀ_Functions();
  sqlite3_ਰਜਿਸਟਰ_Date_ਸਮਾਂ_Functions();
  sqlite3_ਰਜਿਸਟਰ_Json_Functions();
  sqlite3_ਪਾਓ_Builtin_Funcs(aBuiltinFunc, Array_ਮਾਪ(aBuiltinFunc));

#ਜੇ 0  /* Enable to print out how the built-in functions are hashed */
  {
    ਪੂਰਨ i;
    FuncDef *p;
    ਲਈ(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      ਲਿਖੋ("FUNC-HASH %02d:", i);
      ਲਈ(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        ਪੂਰਨ n = sqlite3Strlen30(p->zName);
        ਪੂਰਨ h = p->zName[0] + n;
        ਪ੍ਰਤਿਜ੍ਞਾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        ਲਿਖੋ(" %s(%d)", p->zName, h);
      }
      ਲਿਖੋ("\n");
    }
  }
#ਜੇ_ਅੰਤ
}
