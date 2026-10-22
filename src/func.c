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
#ضمن "sqliteInt.h"
#ضمن <stdlib.h>
#ضمن <assert.h>
#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
#ضمن <math.h>
#پایان_اگر
#ضمن "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
ساكن CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  أكد( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  أكد( pOp->opcode==OP_CollSeq );
  أكد( pOp->p4type==P4_COLLSEQ );
  أرجع pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
ساكن فراغ sqlite3_Skip_انباشتگر_بارگیری(sqlite3_context *context){
  أكد( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
ساكن فراغ minmaxFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح i;
  صحيح ماسک;    /* 0 for min() or 0xffffffff for max() */
  صحيح i_بهترین;
  CollSeq *pColl;

  أكد( argc>1 );
  ماسک = sqlite3_user_داده(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  أكد( pColl );
  أكد( ماسک==-1 || ماسک==0 );
  i_بهترین = 0;
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) أرجع;
  لكل(i=1; i<argc; i++){
    إذا( sqlite3_مقدار_type(argv[i])==SQLITE_NULL ) أرجع;
    إذا( (sqlite3MemCompare(argv[i_بهترین], argv[i], pColl)^ماسک)>=0 ){
      testcase( ماسک==0 );
      i_بهترین = i;
    }
  }
  sqlite3_نتیجه_مقدار(context, argv[i_بهترین]);
}

/*
** Return the type of the argument.
*/
ساكن فراغ typeofFunc(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **argv
){
  ساكن ثابت حرف *azType[] = { "integer", "real", "text", "blob", "null" };
  صحيح i = sqlite3_مقدار_type(argv[0]) - 1;
  استفادهنشده_PARAMETER(Not_استفاده_شده);
  أكد( i>=0 && i<Array_اندازه(azType) );
  أكد( SQLITE_INTEGER==1 );
  أكد( SQLITE_FLOAT==2 );
  أكد( SQLITE_متن==3 );
  أكد( SQLITE_BLOB==4 );
  أكد( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_نتیجه_متن(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
ساكن فراغ subtypeFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  استفادهنشده_PARAMETER(argc);
  sqlite3_نتیجه_int(context, sqlite3_مقدار_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
ساكن فراغ درازی_Func(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  بدل( sqlite3_مقدار_type(argv[0]) ){
    حالة SQLITE_BLOB:
    حالة SQLITE_INTEGER:
    حالة SQLITE_FLOAT: {
      sqlite3_نتیجه_int(context, sqlite3_مقدار_بایتها(argv[0]));
      اكسر;
    }
    حالة SQLITE_متن: {
      ثابت بیعلامت حرف *z = sqlite3_مقدار_متن(argv[0]);
      ثابت بیعلامت حرف *z0;
      إذا( z==0 ) أرجع;
      z0 = z;
      طالما( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        إذا( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }وإلا إذا( z[0]==0 ){
          اكسر;
        }وإلا{
          z++;
          طالما( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_نتیجه_int(context, (صحيح)(z-z0));
      اكسر;
    }
    افتراضي: {
      sqlite3_نتیجه_null(context);
      اكسر;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
ساكن فراغ bytelengthFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  بدل( sqlite3_مقدار_type(argv[0]) ){
    حالة SQLITE_BLOB: {
      sqlite3_نتیجه_int(context, sqlite3_مقدار_بایتها(argv[0]));
      اكسر;
    }
    حالة SQLITE_INTEGER:
    حالة SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_نتیجه_int64(context, sqlite3_مقدار_بایتها(argv[0])*m);
      اكسر;
    }
    حالة SQLITE_متن: {
      إذا( sqlite3_مقدار_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_نتیجه_int(context, sqlite3_مقدار_بایتها(argv[0]));
      }وإلا{
        sqlite3_نتیجه_int(context, sqlite3_مقدار_bytes16(argv[0]));
      }
      اكسر;
    }
    افتراضي: {
      sqlite3_نتیجه_null(context);
      اكسر;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
ساكن فراغ absFunc(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  بدل( sqlite3_مقدار_type(argv[0]) ){
    حالة SQLITE_INTEGER: {
      i64 iVal = sqlite3_مقدار_int64(argv[0]);
      إذا( iVal<0 ){
        إذا( iVal==کوچکترین_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_نتیجه_error(context, "integer overflow", -1);
          أرجع;
        }
        iVal = -iVal;
      }
      sqlite3_نتیجه_int64(context, iVal);
      اكسر;
    }
    حالة SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_نتیجه_null(context);
      اكسر;
    }
    افتراضي: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      مزدوج rVal = sqlite3_مقدار_double(argv[0]);
      إذا( rVal<0 ) rVal = -rVal;
      sqlite3_نتیجه_double(context, rVal);
      اكسر;
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
ساكن فراغ instrFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت بیعلامت حرف *zHaystack;
  ثابت بیعلامت حرف *zNeedle;
  صحيح nHaystack;
  صحيح nNeedle;
  صحيح typeHaystack, typeNeedle;
  صحيح N = 1;
  صحيح is_متن;
  بیعلامت حرف اولی_Char;
  sqlite3_مقدار *pC1 = 0;
  sqlite3_مقدار *pC2 = 0;

  استفادهنشده_PARAMETER(argc);
  typeHaystack = sqlite3_مقدار_type(argv[0]);
  typeNeedle = sqlite3_مقدار_type(argv[1]);
  إذا( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) أرجع;
  nHaystack = sqlite3_مقدار_بایتها(argv[0]);
  nNeedle = sqlite3_مقدار_بایتها(argv[1]);
  إذا( nNeedle>0 ){
    إذا( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_مقدار_blob(argv[0]);
      zNeedle = sqlite3_مقدار_blob(argv[1]);
      is_متن = 0;
    }وإلا إذا( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_مقدار_متن(argv[0]);
      zNeedle = sqlite3_مقدار_متن(argv[1]);
      is_متن = 1;
    }وإلا{
      pC1 = sqlite3_مقدار_dup(argv[0]);
      zHaystack = sqlite3_مقدار_متن(pC1);
      إذا( zHaystack==0 ) اذهب انتها_Instr_OOM;
      nHaystack = sqlite3_مقدار_بایتها(pC1);
      pC2 = sqlite3_مقدار_dup(argv[1]);
      zNeedle = sqlite3_مقدار_متن(pC2);
      إذا( zNeedle==0 ) اذهب انتها_Instr_OOM;
      nNeedle = sqlite3_مقدار_بایتها(pC2);
      is_متن = 1;
    }
    إذا( zNeedle==0 || (nHaystack && zHaystack==0) ) اذهب انتها_Instr_OOM;
    اولی_Char = zNeedle[0];
    طالما( nNeedle<=nHaystack
       && (zHaystack[0]!=اولی_Char || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      افعل{
        nHaystack--;
        zHaystack++;
      }طالما( is_متن && (zHaystack[0]&0xc0)==0x80 );
    }
    إذا( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_نتیجه_int(context, N);
انتها_Instr:
  sqlite3_مقدار_free(pC1);
  sqlite3_مقدار_free(pC2);
  أرجع;
انتها_Instr_OOM:
  sqlite3_نتیجه_error_nomem(context);
  اذهب انتها_Instr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
ساكن فراغ printfFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  PrintfArguments x;
  StrAccum str;
  ثابت حرف *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  إذا( argc>=1 && (zFormat = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.n_استفاده_شده = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->a_حد[SQLITE_حد_درازی]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_نتیجه_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
ساكن SQLITE_INLINE صحيح شامل_است_Nul_Or_Non_Ascii(u64 کلمه){
  ثابت u64 byteOnes = UINT64_C(0x0101010101010101);
  ثابت u64 byte_بالا_بیتها = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  أرجع ((کلمه | (کلمه - byteOnes)) & byte_بالا_بیتها)!=0;
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
ساكن فراغ substrFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت بیعلامت حرف *z;
  ثابت بیعلامت حرف *z2;
  صحيح len;
  صحيح p0type;
  i64 p1, p2;

  أكد( argc==3 || argc==2 );
  p0type = sqlite3_مقدار_type(argv[0]);
  p1 = sqlite3_مقدار_int64(argv[1]);
  إذا( p0type==SQLITE_BLOB ){
    len = sqlite3_مقدار_بایتها(argv[0]);
    z = sqlite3_مقدار_blob(argv[0]);
    إذا( z==0 ) أرجع;
    أكد( len==sqlite3_مقدار_بایتها(argv[0]) );
  }وإلا{
    z = sqlite3_مقدار_متن(argv[0]);
    إذا( z==0 ) أرجع;
    len = 0;
    إذا( p1<0 ){
      لكل(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  إذا( argc==3 ){
    p2 = sqlite3_مقدار_int64(argv[2]);
    إذا( p2==0 && sqlite3_مقدار_type(argv[2])==SQLITE_NULL ) أرجع;
  }وإلا{
    p2 = بزرگترین_INT64;
  }
  إذا( p1==0 ){
#اگر_تعریف_شده SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#پایان_اگر
    إذا( sqlite3_مقدار_type(argv[1])==SQLITE_NULL ) أرجع;
  }
  إذا( p1<0 ){
    p1 += len;
    إذا( p1<0 ){
      إذا( p2<0 ){
        p2 = 0;
      }وإلا{
        p2 += p1;
      }
      p1 = 0;
    }
  }وإلا إذا( p1>0 ){
    p1--;
  }وإلا إذا( p2>0 ){
    p2--;
  }
  إذا( p2<0 ){
    إذا( p2<-p1 ){
      p2 = p1;
    }وإلا{
      p2 = -p2;
    }
    p1 -= p2;
  }
  أكد( p1>=0 && p2>=0 );
  إذا( p0type!=SQLITE_BLOB ){
    إذا( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ثابت بیعلامت حرف *z_انتها = z + sqlite3_مقدار_بایتها(argv[0]);
      طالما( p1>=8 && z_انتها-z>=8 ){
        u64 کلمه;
        کپی_حافظه(&کلمه, z, حجم(کلمه));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        إذا( شامل_است_Nul_Or_Non_Ascii(کلمه) ){
          اكسر;
        }
        z += 8;
        p1 -= 8;
      }
    }
    لكل( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      إذا( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }وإلا إذا( z[0]==0 ){
        اكسر;
      }وإلا{
        افعل{ z++; }طالما( (z[0]&0xc0)==0x80 );
      }
    }
    لكل(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      إذا( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }وإلا إذا( z2[0]==0 ){
        اكسر;
      }وإلا{
        افعل{ z2++; }طالما( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_نتیجه_text64(context, (حرف*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }وإلا{
    إذا( p1>=len ){
      p1 = p2 = 0;
    }وإلا إذا( p2>len-p1 ){
      p2 = len-p1;
      أكد( p2>0 );
    }
    sqlite3_نتیجه_blob64(context, (حرف*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
ساكن فراغ دور_Func(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  i64 n = 0;
  مزدوج r;
  حرف *zBuf;
  أكد( argc==1 || argc==2 );
  إذا( argc==2 ){
    إذا( SQLITE_NULL==sqlite3_مقدار_type(argv[1]) ) أرجع;
    n = sqlite3_مقدار_int64(argv[1]);
    إذا( n>30 ) n = 30;
    إذا( n<0 ) n = 0;
  }
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) أرجع;
  r = sqlite3_مقدار_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  إذا( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }وإلا إذا( n==0 ){ 
    r = (مزدوج)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }وإلا{
    zBuf = sqlite3_mprintf("%!.*f",(صحيح)n,r);
    إذا( zBuf==0 ){
      sqlite3_نتیجه_error_nomem(context);
      أرجع;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_نتیجه_double(context, r);
}
#پایان_اگر

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
ساكن فراغ *contextMalloc(sqlite3_context *context, i64 nByte){
  حرف *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  أكد( nByte>0 );
  testcase( nByte==db->a_حد[SQLITE_حد_درازی] );
  testcase( nByte==(i64)db->a_حد[SQLITE_حد_درازی]+1 );
  إذا( nByte>db->a_حد[SQLITE_حد_درازی] ){
    sqlite3_نتیجه_error_toobig(context);
    z = 0;
  }وإلا{
    z = sqlite3Malloc(nByte);
    إذا( !z ){
      sqlite3_نتیجه_error_nomem(context);
    }
  }
  أرجع z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
ساكن فراغ فوقانی_Func(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  حرف *z1;
  ثابت حرف *z2;
  صحيح i, n;
  استفادهنشده_PARAMETER(argc);
  z2 = (حرف*)sqlite3_مقدار_متن(argv[0]);
  n = sqlite3_مقدار_بایتها(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  أكد( z2==(حرف*)sqlite3_مقدار_متن(argv[0]) );
  إذا( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    إذا( z1 ){
      لكل(i=0; i<n; i++){
        z1[i] = (حرف)sqlite3Toupper(z2[i]);
      }
      sqlite3_نتیجه_متن(context, z1, n, sqlite3_free);
    }
  }
}
ساكن فراغ پایینی_Func(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  حرف *z1;
  ثابت حرف *z2;
  صحيح i, n;
  استفادهنشده_PARAMETER(argc);
  z2 = (حرف*)sqlite3_مقدار_متن(argv[0]);
  n = sqlite3_مقدار_بایتها(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  أكد( z2==(حرف*)sqlite3_مقدار_متن(argv[0]) );
  إذا( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    إذا( z1 ){
      لكل(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_نتیجه_متن(context, z1, n, sqlite3_free);
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
#تعریف_کن noopFunc نسخه_Func   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
ساكن فراغ randomFunc(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **NotUsed2
){
  sqlite_int64 r;
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  sqlite3_randomness(حجم(r), &r);
  إذا( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & بزرگترین_INT64);
  }
  sqlite3_نتیجه_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
ساكن فراغ randomBlob(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  sqlite3_int64 n;
  بیعلامت حرف *p;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  n = sqlite3_مقدار_int64(argv[0]);
  إذا( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  إذا( p ){
    sqlite3_randomness(n, p);
    sqlite3_نتیجه_blob(context, (حرف*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
ساكن فراغ آخرین_درج_rowid(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_نتیجه_int64(context, sqlite3_آخرین_درج_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
ساكن فراغ changes(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  sqlite3_نتیجه_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
ساكن فراغ مجموع_changes(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_نتیجه_int64(context, sqlite3_مجموع_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
بنية compareInfo {
  u8 تطابق_All;          /* "*" or "%" */
  u8 تطابق_One;          /* "?" or "_" */
  u8 تطابق_مجموعه;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#إذا defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#وإلا
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#پایان_اگر

ساكن ثابت بنية compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
ساكن ثابت بنية compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
ساكن ثابت بنية compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#تعریف_کن SQLITE_تطابق             0
#تعریف_کن SQLITE_NOMATCH           1
#تعریف_کن SQLITE_NOWILDCARDMATCH   2

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
ساكن صحيح الگو_Compare(
  ثابت u8 *z_الگو,              /* The glob pattern */
  ثابت u8 *zString,               /* The string to compare against the glob */
  ثابت بنية compareInfo *pInfo, /* Information about how to do the compare */
  u32 تطابق_Other                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 تطابق_One = pInfo->تطابق_One;  /* "?" or "_" */
  u32 تطابق_All = pInfo->تطابق_All;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ثابت u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  طالما( (c = Utf8Read(z_الگو))!=0 ){
    إذا( c==تطابق_All ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      طالما( (c=Utf8Read(z_الگو)) == تطابق_All
             || (c == تطابق_One && تطابق_One!=0) ){
        إذا( c==تطابق_One && sqlite3Utf8Read(&zString)==0 ){
          أرجع SQLITE_NOWILDCARDMATCH;
        }
      }
      إذا( c==0 ){
        أرجع SQLITE_تطابق;   /* "*" at the end of the pattern matches */
      }وإلا إذا( c==تطابق_Other ){
        إذا( pInfo->تطابق_مجموعه==0 ){
          c = sqlite3Utf8Read(&z_الگو);
          إذا( c==0 ) أرجع SQLITE_NOWILDCARDMATCH;
        }وإلا{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          أكد( تطابق_Other<0x80 );  /* '[' is a single-byte character */
          طالما( *zString ){
            صحيح b_تطابق = الگو_Compare(&z_الگو[-1],zString,pInfo,تطابق_Other);
            إذا( b_تطابق!=SQLITE_NOMATCH ) أرجع b_تطابق;
            SQLITE_SKIP_UTF8(zString);
          }
          أرجع SQLITE_NOWILDCARDMATCH;
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
      إذا( c<0x80 ){
        حرف zStop[3];
        صحيح b_تطابق;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = z_الگو[0], ny = nx;
        إذا( nx>=0x80 || nx==تطابق_All || nx==تطابق_One || nx==تطابق_Other ){
          nx = 0;
        }
        إذا( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }وإلا{
          zStop[0] = c;
          zStop[1] = 0;
        }
        طالما(1){
          zString += strcspn((ثابت حرف*)zString, zStop);
          إذا( zString[0]==0 ) اكسر;
          zString++;
          إذا( nx && zString[0]!=nx && zString[0]!=ny ) استمر;
          b_تطابق = الگو_Compare(z_الگو,zString,pInfo,تطابق_Other);
          إذا( b_تطابق!=SQLITE_NOMATCH ) أرجع b_تطابق;
        }
      }وإلا{
        صحيح b_تطابق;
        طالما( (c2 = Utf8Read(zString))!=0 ){
          إذا( c2!=c ) استمر;
          b_تطابق = الگو_Compare(z_الگو,zString,pInfo,تطابق_Other);
          إذا( b_تطابق!=SQLITE_NOMATCH ) أرجع b_تطابق;
        }
      }
      أرجع SQLITE_NOWILDCARDMATCH;
    }
    إذا( c==تطابق_Other ){
      إذا( pInfo->تطابق_مجموعه==0 ){
        c = sqlite3Utf8Read(&z_الگو);
        إذا( c==0 ) أرجع SQLITE_NOMATCH;
        zEscaped = z_الگو;
      }وإلا{
        u32 پیشین_c = 0;
        صحيح دیده = 0;
        صحيح invert = 0;
        c = sqlite3Utf8Read(&zString);
        إذا( c==0 ) أرجع SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&z_الگو);
        إذا( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&z_الگو);
        }
        إذا( c2==']' ){
          إذا( c==']' ) دیده = 1;
          c2 = sqlite3Utf8Read(&z_الگو);
        }
        طالما( c2 && c2!=']' ){
          إذا( c2=='-' && z_الگو[0]!=']' && z_الگو[0]!=0 && پیشین_c>0 ){
            c2 = sqlite3Utf8Read(&z_الگو);
            إذا( c>=پیشین_c && c<=c2 ) دیده = 1;
            پیشین_c = 0;
          }وإلا{
            إذا( c==c2 ){
              دیده = 1;
            }
            پیشین_c = c2;
          }
          c2 = sqlite3Utf8Read(&z_الگو);
        }
        إذا( c2==0 || (دیده ^ invert)==0 ){
          أرجع SQLITE_NOMATCH;
        }
        استمر;
      }
    }
    c2 = Utf8Read(zString);
    إذا( c==c2 ) استمر;
    إذا( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      استمر;
    }
    إذا( c==تطابق_One && z_الگو!=zEscaped && c2!=0 ) استمر;
    أرجع SQLITE_NOMATCH;
  }
  أرجع *zString==0 ? SQLITE_تطابق : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
صحيح sqlite3_strglob(ثابت حرف *z_Glob_الگو, ثابت حرف *zString){
  إذا( zString==0 ){
    أرجع z_Glob_الگو!=0;
  }وإلا إذا( z_Glob_الگو==0 ){
    أرجع 1;
  }وإلا {
    أرجع الگو_Compare((u8*)z_Glob_الگو, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
صحيح sqlite3_strlike(ثابت حرف *z_الگو, ثابت حرف *zStr, بیعلامت صحيح esc){
  إذا( zStr==0 ){
    أرجع z_الگو!=0;
  }وإلا إذا( z_الگو==0 ){
    أرجع 1;
  }وإلا{
    أرجع الگو_Compare((u8*)z_الگو, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#اگر_تعریف_شده SQLITE_TEST
صحيح sqlite3_like_شمار = 0;
#پایان_اگر


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
ساكن فراغ likeFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت بیعلامت حرف *zA, *zB;
  u32 escape;
  صحيح nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  بنية compareInfo *pInfo = sqlite3_user_داده(context);
  بنية compareInfo backupInfo;

#اگر_تعریف_شده SQLITE_LIKE_DOESNT_تطابق_BLOBS
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_BLOB
   || sqlite3_مقدار_type(argv[1])==SQLITE_BLOB
  ){
#اگر_تعریف_شده SQLITE_TEST
    sqlite3_like_شمار++;
#پایان_اگر
    sqlite3_نتیجه_int(context, 0);
    أرجع;
  }
#پایان_اگر

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_مقدار_بایتها(argv[0]);
  testcase( nPat==db->a_حد[SQLITE_حد_LIKE_الگو_درازی] );
  testcase( nPat==db->a_حد[SQLITE_حد_LIKE_الگو_درازی]+1 );
  إذا( nPat > db->a_حد[SQLITE_حد_LIKE_الگو_درازی] ){
    sqlite3_نتیجه_error(context, "LIKE or GLOB pattern too complex", -1);
    أرجع;
  }
  إذا( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ثابت بیعلامت حرف *zEsc = sqlite3_مقدار_متن(argv[2]);
    إذا( zEsc==0 ) أرجع;
    إذا( sqlite3Utf8CharLen((حرف*)zEsc, -1)!=1 ){
      sqlite3_نتیجه_error(context,
          "ESCAPE expression must be a single character", -1);
      أرجع;
    }
    escape = sqlite3Utf8Read(&zEsc);
    إذا( escape==pInfo->تطابق_All || escape==pInfo->تطابق_One ){
      کپی_حافظه(&backupInfo, pInfo, حجم(backupInfo));
      pInfo = &backupInfo;
      إذا( escape==pInfo->تطابق_All ) pInfo->تطابق_All = 0;
      إذا( escape==pInfo->تطابق_One ) pInfo->تطابق_One = 0;
    }
  }وإلا{
    escape = pInfo->تطابق_مجموعه;
  }
  zB = sqlite3_مقدار_متن(argv[0]);
  zA = sqlite3_مقدار_متن(argv[1]);
  إذا( zA && zB ){
#اگر_تعریف_شده SQLITE_TEST
    sqlite3_like_شمار++;
#پایان_اگر
    sqlite3_نتیجه_int(context,
                      الگو_Compare(zB, zA, pInfo, escape)==SQLITE_تطابق);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
ساكن فراغ nullifFunc(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  استفادهنشده_PARAMETER(Not_استفاده_شده);
  إذا( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_نتیجه_مقدار(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
ساكن فراغ نسخه_Func(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **NotUsed2
){
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_نتیجه_متن(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
ساكن فراغ sourceidFunc(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **NotUsed2
){
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_نتیجه_متن(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
ساكن فراغ errlogFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  استفادهنشده_PARAMETER(argc);
  استفادهنشده_PARAMETER(context);
  sqlite3_log(sqlite3_مقدار_int(argv[0]), "%s", sqlite3_مقدار_متن(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#اگر_تعریف_نشده SQLITE_OMIT_COMPILEOPTION_DIAGS
ساكن فراغ compileoptionusedFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت حرف *zOptName;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  إذا( (zOptName = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]))!=0 ){
    sqlite3_نتیجه_int(context, sqlite3_compileoption_استفاده_شده(zOptName));
  }
}
#پایان_اگر /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#اگر_تعریف_نشده SQLITE_OMIT_COMPILEOPTION_DIAGS
ساكن فراغ compileoptiongetFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  i64 n;
  ثابت حرف *zRes;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_مقدار_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((صحيح)n) : 0;
  sqlite3_نتیجه_متن(context, zRes, -1, SQLITE_STATIC);
}
#پایان_اگر /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
ساكن ثابت حرف hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
فراغ sqlite3_Quote_مقدار(StrAccum *pStr, sqlite3_مقدار *p_مقدار, صحيح bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  أكد( pStr!=0 && pStr->nChar==0 );

  بدل( sqlite3_مقدار_type(p_مقدار) ){
    حالة SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_مقدار_double(p_مقدار));
      اكسر;
    }
    حالة SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_مقدار_int64(p_مقدار));
      اكسر;
    }
    حالة SQLITE_BLOB: {
      حرف ثابت *zBlob = sqlite3_مقدار_blob(p_مقدار);
      i64 nBlob = sqlite3_مقدار_بایتها(p_مقدار);
      أكد( zBlob==sqlite3_مقدار_blob(p_مقدار) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      إذا( pStr->accError==0 ){
        حرف *z_متن = pStr->z_متن;
        صحيح i;
        لكل(i=0; i<nBlob; i++){
          z_متن[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          z_متن[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        z_متن[(nBlob*2)+2] = '\'';
        z_متن[(nBlob*2)+3] = '\0';
        z_متن[0] = 'X';
        z_متن[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      اكسر;
    }
    حالة SQLITE_متن: {
      ثابت بیعلامت حرف *zArg = sqlite3_مقدار_متن(p_مقدار);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      اكسر;
    }
    افتراضي: {
      أكد( sqlite3_مقدار_type(p_مقدار)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      اكسر;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
ساكن صحيح isNHex(ثابت حرف *z, صحيح N, u32 *pVal){
  صحيح i;
  u32 v = 0;
  لكل(i=0; i<N; i++){
    إذا( !sqlite3Isxdigit(z[i]) ) أرجع 0;
    v = (v<<4) + sqlite3_Hex_مقصد_Int(z[i]);
  }
  *pVal = v;
  أرجع 1;
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
ساكن فراغ unistrFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  حرف *z_بیرون;
  ثابت حرف *z_درون;
  صحيح n_درون;
  صحيح i, j, n;
  u32 v;

  أكد( argc==1 );
  استفادهنشده_PARAMETER( argc );
  z_درون = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
  إذا( z_درون==0 ) أرجع;
  n_درون = sqlite3_مقدار_بایتها(argv[0]);
  z_بیرون = sqlite3_malloc64(n_درون+1);
  إذا( z_بیرون==0 ){
    sqlite3_نتیجه_error_nomem(context);
    أرجع;
  }
  i = j = 0;
  طالما( i<n_درون ){
    ثابت حرف *z = strchr(&z_درون[i],'\\');
    إذا( z==0 ){
      n = n_درون - i;
      memmove(&z_بیرون[j], &z_درون[i], n);
      j += n;
      اكسر;
    }
    n = z - &z_درون[i];
    إذا( n>0 ){
      memmove(&z_بیرون[j], &z_درون[i], n);
      j += n;
      i += n;
    }
    إذا( z_درون[i+1]=='\\' ){
      i += 2;
      z_بیرون[j++] = '\\';
    }وإلا إذا( sqlite3Isxdigit(z_درون[i+1]) ){
      إذا( !isNHex(&z_درون[i+1], 4, &v) ) اذهب unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&z_بیرون[j], v);
    }وإلا إذا( z_درون[i+1]=='+' ){
      إذا( !isNHex(&z_درون[i+2], 6, &v) ) اذهب unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&z_بیرون[j], v);
    }وإلا إذا( z_درون[i+1]=='u' ){
      إذا( !isNHex(&z_درون[i+2], 4, &v) ) اذهب unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&z_بیرون[j], v);
    }وإلا إذا( z_درون[i+1]=='U' ){
      إذا( !isNHex(&z_درون[i+2], 8, &v) ) اذهب unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&z_بیرون[j], v);
    }وإلا{
      اذهب unistr_error;
    }
  }
  z_بیرون[j] = 0;
  sqlite3_نتیجه_text64(context, z_بیرون, j, sqlite3_free, SQLITE_UTF8_ZT);
  أرجع;

unistr_error:
  sqlite3_free(z_بیرون);
  sqlite3_نتیجه_error(context, "invalid Unicode escape", -1);
  أرجع;
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
ساكن فراغ quoteFunc(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->a_حد[SQLITE_حد_درازی]);
  sqlite3_Quote_مقدار(&str,argv[0],SQLITE_PTR_مقصد_INT(sqlite3_user_داده(context)));
  sqlite3_نتیجه_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
ساكن فراغ unicodeFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت بیعلامت حرف *z = sqlite3_مقدار_متن(argv[0]);
  (فراغ)argc;
  إذا( z && z[0] ) sqlite3_نتیجه_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
ساكن فراغ charFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  بیعلامت حرف *z, *z_بیرون;
  صحيح i;
  z_بیرون = z = sqlite3_malloc64( argc*4+1 );
  إذا( z==0 ){
    sqlite3_نتیجه_error_nomem(context);
    أرجع;
  }
  لكل(i=0; i<argc; i++){
    sqlite3_int64 x;
    بیعلامت c;
    x = sqlite3_مقدار_int64(argv[i]);
    إذا( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (بیعلامت)(x & 0x1fffff);
    إذا( c<0x00080 ){
      *z_بیرون++ = (u8)(c&0xFF);
    }وإلا إذا( c<0x00800 ){
      *z_بیرون++ = 0xC0 + (u8)((c>>6)&0x1F);
      *z_بیرون++ = 0x80 + (u8)(c & 0x3F);
    }وإلا إذا( c<0x10000 ){
      *z_بیرون++ = 0xE0 + (u8)((c>>12)&0x0F);
      *z_بیرون++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_بیرون++ = 0x80 + (u8)(c & 0x3F);
    }وإلا{
      *z_بیرون++ = 0xF0 + (u8)((c>>18) & 0x07);
      *z_بیرون++ = 0x80 + (u8)((c>>12) & 0x3F);
      *z_بیرون++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_بیرون++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *z_بیرون = 0;
  sqlite3_نتیجه_text64(context, (حرف*)z, z_بیرون-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
ساكن فراغ hexFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح i, n;
  ثابت بیعلامت حرف *pBlob;
  حرف *zHex, *z;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  pBlob = sqlite3_مقدار_blob(argv[0]);
  n = sqlite3_مقدار_بایتها(argv[0]);
  أكد( pBlob==sqlite3_مقدار_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  إذا( zHex ){
    لكل(i=0; i<n; i++, pBlob++){
      بیعلامت حرف c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_نتیجه_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
ساكن صحيح str_شامل_است_Char(ثابت u8 *zStr, صحيح nStr, u32 ch){
  ثابت u8 *z_انتها = &zStr[nStr];
  ثابت u8 *z = zStr;
  طالما( z<z_انتها ){
    u32 tst = Utf8Read(z);
    إذا( tst==ch ) أرجع 1;
  }
  أرجع 0;
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
ساكن فراغ unhexFunc(
  sqlite3_context *pCtx,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت u8 *zPass = (ثابت u8*)"";
  صحيح nPass = 0;
  ثابت u8 *zHex = sqlite3_مقدار_متن(argv[0]);
  صحيح nHex = sqlite3_مقدار_بایتها(argv[0]);
#اگر_تعریف_شده SQLITE_DEBUG
  ثابت u8 *z_انتها = zHex ? &zHex[nHex] : 0;
#پایان_اگر
  u8 *pBlob = 0;
  u8 *p = 0;

  أكد( argc==1 || argc==2 );
  إذا( argc==2 ){
    zPass = sqlite3_مقدار_متن(argv[1]);
    nPass = sqlite3_مقدار_بایتها(argv[1]);
  }
  إذا( !zHex || !zPass ) أرجع;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  إذا( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    طالما( (c = *zHex)!=0x00 ){
      طالما( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        أكد( zHex<=z_انتها );
        إذا( !str_شامل_است_Char(zPass, nPass, ch) ) اذهب unhex_null;
        c = *zHex;
        إذا( c==0x00 ) اذهب unhex_انجامشده;
      }
      zHex++;
      أكد( *z_انتها==0x00 );
      أكد( zHex<=z_انتها );
      d = *(zHex++);
      إذا( !sqlite3Isxdigit(d) ) اذهب unhex_null;
      *(p++) = (sqlite3_Hex_مقصد_Int(c)<<4) | sqlite3_Hex_مقصد_Int(d);
    }
  }

 unhex_انجامشده:
  sqlite3_نتیجه_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  أرجع;

 unhex_null:
  sqlite3_free(pBlob);
  أرجع;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
ساكن فراغ zeroblobFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  i64 n;
  صحيح rc;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  n = sqlite3_مقدار_int64(argv[0]);
  إذا( n<0 ) n = 0;
  rc = sqlite3_نتیجه_zeroblob64(context, n); /* IMP: R-00293-64994 */
  إذا( rc ){
    sqlite3_نتیجه_error_کد(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
ساكن فراغ replaceFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت بیعلامت حرف *zStr;        /* The input string A */
  ثابت بیعلامت حرف *z_الگو;    /* The pattern string B */
  ثابت بیعلامت حرف *zRep;        /* The replacement string C */
  بیعلامت حرف *z_بیرون;              /* The output */
  صحيح nStr;                /* Size of zStr */
  صحيح n_الگو;            /* Size of zPattern */
  صحيح nRep;                /* Size of zRep */
  i64 n_بیرون;                /* Maximum size of zOut */
  صحيح loop_حد;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  بیعلامت cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  أكد( argc==3 );
  استفادهنشده_PARAMETER(argc);
  zStr = sqlite3_مقدار_متن(argv[0]);
  إذا( zStr==0 ) أرجع;
  nStr = sqlite3_مقدار_بایتها(argv[0]);
  أكد( zStr==sqlite3_مقدار_متن(argv[0]) );  /* No encoding change */
  z_الگو = sqlite3_مقدار_متن(argv[1]);
  إذا( z_الگو==0 ){
    أكد( sqlite3_مقدار_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    أرجع;
  }
  إذا( z_الگو[0]==0 ){
    أكد( sqlite3_مقدار_type(argv[1])!=SQLITE_NULL );
    sqlite3_نتیجه_متن(context, (ثابت حرف*)zStr, nStr, SQLITE_TRANSIENT);
    أرجع;
  }
  n_الگو = sqlite3_مقدار_بایتها(argv[1]);
  أكد( z_الگو==sqlite3_مقدار_متن(argv[1]) );  /* No encoding change */
  zRep = sqlite3_مقدار_متن(argv[2]);
  إذا( zRep==0 ) أرجع;
  nRep = sqlite3_مقدار_بایتها(argv[2]);
  أكد( zRep==sqlite3_مقدار_متن(argv[2]) );
  n_بیرون = nStr + 1;
  أكد( n_بیرون<SQLITE_MAX_درازی );
  z_بیرون = contextMalloc(context, n_بیرون);
  إذا( z_بیرون==0 ){
    أرجع;
  }
  loop_حد = nStr - n_الگو; 
  cntExpand = 0;
  لكل(i=j=0; i<=loop_حد; i++){
    إذا( zStr[i]!=z_الگو[0] || memcmp(&zStr[i], z_الگو, n_الگو) ){
      z_بیرون[j++] = zStr[i];
    }وإلا{
      إذا( nRep>n_الگو ){
        n_بیرون += nRep - n_الگو;
        testcase( n_بیرون-1==db->a_حد[SQLITE_حد_درازی] );
        testcase( n_بیرون-2==db->a_حد[SQLITE_حد_درازی] );
        إذا( n_بیرون-1>db->a_حد[SQLITE_حد_درازی] ){
          sqlite3_نتیجه_error_toobig(context);
          sqlite3_free(z_بیرون);
          أرجع;
        }
        cntExpand++;
        إذا( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = z_بیرون;
          z_بیرون = sqlite3Realloc(z_بیرون, (صحيح)n_بیرون + (n_بیرون - nStr - 1));
          إذا( z_بیرون==0 ){
            sqlite3_نتیجه_error_nomem(context);
            sqlite3_free(zOld);
            أرجع;
          }
        }
      }
      کپی_حافظه(&z_بیرون[j], zRep, nRep);
      j += nRep;
      i += n_الگو-1;
    }
  }
  أكد( j+nStr-i+1<=n_بیرون );
  کپی_حافظه(&z_بیرون[j], &zStr[i], nStr-i);
  j += nStr - i;
  أكد( j<=n_بیرون );
  z_بیرون[j] = 0;
  sqlite3_نتیجه_متن(context, (حرف*)z_بیرون, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
ساكن فراغ trimFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت بیعلامت حرف *z_درون;         /* Input string */
  ثابت بیعلامت حرف *z_Char_مجموعه;    /* Set of characters to trim */
  بیعلامت صحيح n_درون;                 /* Number of bytes in input */
  صحيح flags;                        /* 1: trimleft  2: trimright  3: trim */
  صحيح i;                            /* Loop counter */
  بیعلامت صحيح *aLen = 0;           /* Length of each character in zCharSet */
  بیعلامت حرف **azChar = 0;       /* Individual characters in zCharSet */
  صحيح nChar;                        /* Number of characters in zCharSet */

  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ){
    أرجع;
  }
  z_درون = sqlite3_مقدار_متن(argv[0]);
  إذا( z_درون==0 ) أرجع;
  n_درون = (بیعلامت)sqlite3_مقدار_بایتها(argv[0]);
  أكد( z_درون==sqlite3_مقدار_متن(argv[0]) );
  إذا( argc==1 ){
    ساكن ثابت بیعلامت lenOne[] = { 1 };
    ساكن بیعلامت حرف * ثابت azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (بیعلامت*)lenOne;
    azChar = (بیعلامت حرف **)azOne;
    z_Char_مجموعه = 0;
  }وإلا إذا( (z_Char_مجموعه = sqlite3_مقدار_متن(argv[1]))==0 ){
    أرجع;
  }وإلا{
    ثابت بیعلامت حرف *z;
    لكل(z=z_Char_مجموعه, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    إذا( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(حجم(حرف*)+حجم(بیعلامت)));
      إذا( azChar==0 ){
        أرجع;
      }
      aLen = (بیعلامت*)&azChar[nChar];
      لكل(z=z_Char_مجموعه, nChar=0; *z; nChar++){
        azChar[nChar] = (بیعلامت حرف *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (بیعلامت)(z - azChar[nChar]);
      }
    }
  }
  إذا( nChar>0 ){
    flags = SQLITE_PTR_مقصد_INT(sqlite3_user_داده(context));
    إذا( flags & 1 ){
      طالما( n_درون>0 ){
        بیعلامت صحيح len = 0;
        لكل(i=0; i<nChar; i++){
          len = aLen[i];
          إذا( len<=n_درون && memcmp(z_درون, azChar[i], len)==0 ) اكسر;
        }
        إذا( i>=nChar ) اكسر;
        z_درون += len;
        n_درون -= len;
      }
    }
    إذا( flags & 2 ){
      طالما( n_درون>0 ){
        بیعلامت صحيح len = 0;
        لكل(i=0; i<nChar; i++){
          len = aLen[i];
          إذا( len<=n_درون && memcmp(&z_درون[n_درون-len],azChar[i],len)==0 ) اكسر;
        }
        إذا( i>=nChar ) اكسر;
        n_درون -= len;
      }
    }
    إذا( z_Char_مجموعه ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_نتیجه_متن(context, (حرف*)z_درون, n_درون, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
ساكن فراغ concatFuncCore(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv,
  صحيح nSep,
  ثابت حرف *zSep
){
  i64 j, n = 0;
  صحيح i;
  صحيح bNotNull = 0;   /* True after at least NOT NULL argument seen */
  حرف *z;
  لكل(i=0; i<argc; i++){
    n += sqlite3_مقدار_بایتها(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  إذا( z==0 ){
    sqlite3_نتیجه_error_nomem(context);
    أرجع;
  }
  j = 0;
  لكل(i=0; i<argc; i++){
    إذا( sqlite3_مقدار_type(argv[i])!=SQLITE_NULL ){
      صحيح k = sqlite3_مقدار_بایتها(argv[i]);
      ثابت حرف *v = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]);
      إذا( v!=0 ){
        إذا( bNotNull && nSep>0 ){
          کپی_حافظه(&z[j], zSep, nSep);
          j += nSep;
        }
        کپی_حافظه(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  أكد( j<=n );
  sqlite3_نتیجه_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
ساكن فراغ concatFunc(
  sqlite3_context *context,
  صحيح argc,
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
ساكن فراغ concatwsFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح nSep = sqlite3_مقدار_بایتها(argv[0]);
  ثابت حرف *zSep = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
  إذا( zSep==0 ) أرجع;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#اگر_تعریف_شده SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
ساكن فراغ unknownFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  /* no-op */
  (فراغ)context;
  (فراغ)argc;
  (فراغ)argv;
}
#پایان_اگر /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#اگر_تعریف_شده SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
ساكن فراغ soundexFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  حرف z_نتیجه[8];
  ثابت u8 *z_درون;
  صحيح i, j;
  ساكن ثابت بیعلامت حرف i_کد[] = {
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
  z_درون = (u8*)sqlite3_مقدار_متن(argv[0]);
  إذا( z_درون==0 ) z_درون = (u8*)"";
  لكل(i=0; z_درون[i] && !sqlite3Isalpha(z_درون[i]); i++){}
  إذا( z_درون[i] ){
    u8 prevcode = i_کد[z_درون[i]&0x7f];
    z_نتیجه[0] = sqlite3Toupper(z_درون[i]);
    لكل(j=1; j<4 && z_درون[i]; i++){
      صحيح کد = i_کد[z_درون[i]&0x7f];
      إذا( کد>0 ){
        إذا( کد!=prevcode ){
          prevcode = کد;
          z_نتیجه[j++] = کد + '0';
        }
      }وإلا{
        prevcode = 0;
      }
    }
    طالما( j<4 ){
      z_نتیجه[j++] = '0';
    }
    z_نتیجه[j] = 0;
    sqlite3_نتیجه_متن(context, z_نتیجه, 4, SQLITE_TRANSIENT);
  }وإلا{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_نتیجه_متن(context, "?000", 4, SQLITE_STATIC);
  }
}
#پایان_اگر /* SQLITE_SOUNDEX */

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
ساكن فراغ oomFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#پایان_اگر /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#اگر_تعریف_نشده SQLITE_OMIT_بارگیری_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
ساكن فراغ بارگیری_Ext(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  ثابت حرف *zFile = (ثابت حرف *)sqlite3_مقدار_متن(argv[0]);
  ثابت حرف *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف *z_Err_پیام = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  إذا( (db->flags & SQLITE_بارگیری_Ext_Func)==0 ){
    sqlite3_نتیجه_error(context, "not authorized", -1);
    أرجع;
  }

  إذا( argc==2 ){
    zProc = (ثابت حرف *)sqlite3_مقدار_متن(argv[1]);
  }وإلا{
    zProc = 0;
  }
  إذا( zFile && sqlite3_بارگیری_extension(db, zFile, zProc, &z_Err_پیام) ){
    sqlite3_نتیجه_error(context, z_Err_پیام, -1);
    sqlite3_free(z_Err_پیام);
  }
}
#پایان_اگر


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
تعريفنوع بنية جمع_Ctx جمع_Ctx;
بنية جمع_Ctx {
  مزدوج r_جمع;      /* Running sum as as a double */
  مزدوج rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 i_جمع;         /* Running sum as a signed integer */
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
ساكن فراغ kahan_Babuska_Neumaier_گام(
  متطاير جمع_Ctx *p_جمع,
  متطاير مزدوج r
){
  متطاير مزدوج s = p_جمع->r_جمع;
  متطاير مزدوج t = s + r;
  إذا( fabs(s) > fabs(r) ){
    p_جمع->rErr += (s - t) + r;
  }وإلا{
    p_جمع->rErr += (r - t) + s;
  }
  p_جمع->r_جمع = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
ساكن فراغ kahan_Babuska_Neumaier_گام_Int64(متطاير جمع_Ctx *p_جمع, i64 iVal){
  إذا( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahan_Babuska_Neumaier_گام(p_جمع, iBig);
    kahan_Babuska_Neumaier_گام(p_جمع, iSm);
  }وإلا{
    kahan_Babuska_Neumaier_گام(p_جمع, (مزدوج)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
ساكن فراغ kahanBabuskaNeumaierInit(
  متطاير جمع_Ctx *p,
  i64 iVal
){
  إذا( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_جمع = (مزدوج)(iVal - iSm);
    p->rErr = (مزدوج)iSm;
  }وإلا{
    p->r_جمع = (مزدوج)iVal;
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
ساكن فراغ جمع_گام(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  جمع_Ctx *p;
  صحيح type;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, حجم(*p));
  type = sqlite3_مقدار_numeric_type(argv[0]);
  إذا( p && type!=SQLITE_NULL ){
    p->cnt++;
    إذا( p->approx==0 ){
      إذا( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_جمع);
        p->approx = 1;
        kahan_Babuska_Neumaier_گام(p, sqlite3_مقدار_double(argv[0]));
      }وإلا{
        i64 x = p->i_جمع;
        إذا( sqlite3_جمع_کن_Int64(&x, sqlite3_مقدار_int64(argv[0]))==0 ){
          p->i_جمع = x;
        }وإلا{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_جمع);
          p->approx = 1;
          kahan_Babuska_Neumaier_گام_Int64(p, sqlite3_مقدار_int64(argv[0]));
        }
      }
    }وإلا{
      إذا( type==SQLITE_INTEGER ){
        kahan_Babuska_Neumaier_گام_Int64(p, sqlite3_مقدار_int64(argv[0]));
      }وإلا{
        p->ovrfl = 0;
        kahan_Babuska_Neumaier_گام(p, sqlite3_مقدار_double(argv[0]));
      }
    }
  }
}
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
ساكن فراغ جمع_Inverse(sqlite3_context *context, صحيح argc, sqlite3_مقدار**argv){
  جمع_Ctx *p;
  صحيح type;
  أكد( argc==1 );
  استفادهنشده_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, حجم(*p));
  type = sqlite3_مقدار_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  إذا( ALWAYS(p) && type!=SQLITE_NULL ){
    أكد( p->cnt>0 );
    p->cnt--;
    إذا( !p->approx ){
      i64 x = p->i_جمع;
      إذا( sqlite3SubInt64(&x, sqlite3_مقدار_int64(argv[0]))==0 ){
        p->i_جمع = x;
        أرجع;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_جمع);
    }
    إذا( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_مقدار_int64(argv[0]);
      إذا( iVal!=کوچکترین_INT64 ){
        kahan_Babuska_Neumaier_گام_Int64(p, -iVal);
      }وإلا{
        kahan_Babuska_Neumaier_گام_Int64(p, بزرگترین_INT64);
        kahan_Babuska_Neumaier_گام_Int64(p, 1);
      }       
    }وإلا{
      kahan_Babuska_Neumaier_گام(p, -sqlite3_مقدار_double(argv[0]));
    }
  }
}
#وإلا
# define جمع_Inverse 0
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */
ساكن فراغ جمع_Finalize(sqlite3_context *context){
  جمع_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  إذا( p && p->cnt>0 ){
    إذا( p->approx ){
      إذا( p->ovrfl ){
        sqlite3_نتیجه_error(context,"integer overflow",-1);
      }وإلا إذا( !sqlite3_Is_سرریز(p->rErr) ){
        sqlite3_نتیجه_double(context, p->r_جمع+p->rErr);
      }وإلا{
        sqlite3_نتیجه_double(context, p->r_جمع);
      }
    }وإلا{
      sqlite3_نتیجه_int64(context, p->i_جمع);
    }
  }
}
ساكن فراغ avgFinalize(sqlite3_context *context){
  جمع_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  إذا( p && p->cnt>0 ){
    مزدوج r;
    إذا( p->approx ){
      r = p->r_جمع;
      إذا( !sqlite3_Is_سرریز(p->rErr) ) r += p->rErr;
    }وإلا{
      r = (مزدوج)(p->i_جمع);
    }
    sqlite3_نتیجه_double(context, r/(مزدوج)p->cnt);
  }
}
ساكن فراغ مجموع_Finalize(sqlite3_context *context){
  جمع_Ctx *p;
  مزدوج r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  إذا( p ){
    إذا( p->approx ){
      r = p->r_جمع;
      إذا( !sqlite3_Is_سرریز(p->rErr) ) r += p->rErr;
    }وإلا{
      r = (مزدوج)(p->i_جمع);
    }
  }
  sqlite3_نتیجه_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
تعريفنوع بنية شمار_Ctx شمار_Ctx;
بنية شمار_Ctx {
  i64 n;
#اگر_تعریف_شده SQLITE_DEBUG
  صحيح bInverse;                   /* True if xInverse() ever called */
#پایان_اگر
};

/*
** Routines to implement the count() aggregate function.
*/
ساكن فراغ شمار_گام(sqlite3_context *context, صحيح argc, sqlite3_مقدار **argv){
  شمار_Ctx *p;
  p = sqlite3_aggregate_context(context, حجم(*p));
  إذا( (argc==0 || SQLITE_NULL!=sqlite3_مقدار_type(argv[0])) && p ){
    p->n++;
  }

#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  أكد( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_شمار(context) );
#پایان_اگر
}  
ساكن فراغ شمار_Finalize(sqlite3_context *context){
  شمار_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_نتیجه_int64(context, p ? p->n : 0);
}
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
ساكن فراغ شمار_Inverse(sqlite3_context *ctx, صحيح argc, sqlite3_مقدار **argv){
  شمار_Ctx *p;
  p = sqlite3_aggregate_context(ctx, حجم(*p));
  /* p is always non-NULL since countStep() will have been called first */
  إذا( (argc==0 || SQLITE_NULL!=sqlite3_مقدار_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#اگر_تعریف_شده SQLITE_DEBUG
    p->bInverse = 1;
#پایان_اگر
  }
}  
#وإلا
# define شمار_Inverse 0
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
ساكن فراغ minmax_گام(
  sqlite3_context *context,
  صحيح Not_استفاده_شده,
  sqlite3_مقدار **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *p_بهترین;
  استفادهنشده_PARAMETER(Not_استفاده_شده);

  p_بهترین = (Mem *)sqlite3_aggregate_context(context, حجم(*p_بهترین));
  إذا( !p_بهترین ) أرجع;

  إذا( sqlite3_مقدار_type(pArg)==SQLITE_NULL ){
    إذا( p_بهترین->flags ) sqlite3_Skip_انباشتگر_بارگیری(context);
  }وإلا إذا( p_بهترین->flags ){
    صحيح max;
    صحيح cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_داده(context)!=0;
    cmp = sqlite3MemCompare(p_بهترین, pArg, pColl);
    إذا( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(p_بهترین, pArg);
    }وإلا{
      sqlite3_Skip_انباشتگر_بارگیری(context);
    }
  }وإلا{
    p_بهترین->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(p_بهترین, pArg);
  }
}
ساكن فراغ min_Max_مقدار_Finalize(sqlite3_context *context, صحيح b_مقدار){
  sqlite3_مقدار *pRes;
  pRes = (sqlite3_مقدار *)sqlite3_aggregate_context(context, 0);
  إذا( pRes ){
    إذا( pRes->flags ){
      sqlite3_نتیجه_مقدار(context, pRes);
    }
    إذا( b_مقدار==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
ساكن فراغ min_Max_مقدار(sqlite3_context *context){
  min_Max_مقدار_Finalize(context, 1);
}
#وإلا
# define min_Max_مقدار 0
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */
ساكن فراغ minMaxFinalize(sqlite3_context *context){
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
تعريفنوع بنية {
  StrAccum str;          /* The accumulated concatenation */
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
  صحيح nAccum;            /* Number of strings presently concatenated */
  صحيح n_اولی_Sep_درازی;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  صحيح *pnSepLengths;
#پایان_اگر
} گروه_Concat_Ctx;

ساكن فراغ گروه_Concat_گام(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  ثابت حرف *zVal;
  گروه_Concat_Ctx *pGCC;
  ثابت حرف *zSep;
  صحيح nVal, nSep;
  أكد( argc==1 || argc==2 );
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) أرجع;
  pGCC = (گروه_Concat_Ctx*)sqlite3_aggregate_context(context, حجم(*pGCC));
  إذا( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    صحيح اولی_جمله = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->a_حد[SQLITE_حد_درازی];
    إذا( argc==1 ){
      إذا( !اولی_جمله ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      وإلا{
        pGCC->n_اولی_Sep_درازی = 1;
      }
#پایان_اگر
    }وإلا إذا( !اولی_جمله ){
      zSep = (حرف*)sqlite3_مقدار_متن(argv[1]);
      nSep = sqlite3_مقدار_بایتها(argv[1]);
      إذا( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      وإلا{
        nSep = 0;
      }
      إذا( nSep != pGCC->n_اولی_Sep_درازی || pGCC->pnSepLengths != 0 ){
        صحيح *pnsl = pGCC->pnSepLengths;
        إذا( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (صحيح*)sqlite3_malloc64((pGCC->nAccum+1) * حجم(صحيح));
          إذا( pnsl!=0 ){
            صحيح i = 0, nA = pGCC->nAccum-1;
            طالما( i<nA ) pnsl[i++] = pGCC->n_اولی_Sep_درازی;
          }
        }وإلا{
          pnsl = (صحيح*)sqlite3_realloc64(pnsl, pGCC->nAccum * حجم(صحيح));
        }
        إذا( pnsl!=0 ){
          إذا( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }وإلا{
          sqlite3_Str_Accum_مجموعه_Error(&pGCC->str, SQLITE_NOMEM);
        }
      }
#پایان_اگر
    }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
    وإلا{
      pGCC->n_اولی_Sep_درازی = sqlite3_مقدار_بایتها(argv[1]);
    }
    pGCC->nAccum += 1;
#پایان_اگر
    zVal = (حرف*)sqlite3_مقدار_متن(argv[0]);
    nVal = sqlite3_مقدار_بایتها(argv[0]);
    إذا( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
ساكن فراغ گروه_Concat_Inverse(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  گروه_Concat_Ctx *pGCC;
  أكد( argc==1 || argc==2 );
  (فراغ)argc;  /* Suppress unused parameter warning */
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) أرجع;
  pGCC = (گروه_Concat_Ctx*)sqlite3_aggregate_context(context, حجم(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  إذا( ALWAYS(pGCC) ){
    صحيح nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (فراغ)sqlite3_مقدار_متن(argv[0]);
    nVS = sqlite3_مقدار_بایتها(argv[0]);
    pGCC->nAccum -= 1;
    إذا( pGCC->pnSepLengths!=0 ){
      أكد(pGCC->nAccum >= 0);
      إذا( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*حجم(صحيح));
      }
    }وإلا{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_اولی_Sep_درازی;
    }
    إذا( nVS>=(صحيح)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }وإلا{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.z_متن, &pGCC->str.z_متن[nVS], pGCC->str.nChar);
    }
    إذا( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#وإلا
# define گروه_Concat_Inverse 0
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */
ساكن فراغ گروه_Concat_Finalize(sqlite3_context *context){
  گروه_Concat_Ctx *pGCC
    = (گروه_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  إذا( pGCC ){
    sqlite3_نتیجه_str(context, &pGCC->str, SQLITE_XFER);
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#پایان_اگر
  }
}
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
ساكن فراغ گروه_Concat_مقدار(sqlite3_context *context){
  گروه_Concat_Ctx *pGCC
    = (گروه_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  إذا( pGCC && pGCC->nAccum>0 ){
    sqlite3_نتیجه_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#وإلا
# define گروه_Concat_مقدار 0
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
فراغ sqlite3_ثبات_در_هر_Connection_Builtin_Functions(sqlite3 *db){
  صحيح rc = sqlite3_overload_function(db, "MATCH", 2);
  أكد( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  إذا( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
فراغ sqlite3_ثبات_Like_Functions(sqlite3 *db, صحيح caseSensitive){
  FuncDef *pDef;
  بنية compareInfo *pInfo;
  صحيح flags;
  صحيح nArg;
  إذا( caseSensitive ){
    pInfo = (بنية compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }وإلا{
    pInfo = (بنية compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  لكل(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3_پیدا_کن_Function(db, "like", nArg, SQLITE_UTF8, 0);
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
صحيح sqlite3IsLikeFunction(sqlite3 *db, عبارت *p_عبارت, صحيح *pIsNocase, حرف *aWc){
  FuncDef *pDef;
  صحيح n_عبارت;
  أكد( p_عبارت!=0 );
  أكد( p_عبارت->op==TK_FUNCTION );
  أكد( عبارت_Use_XList(p_عبارت) );
  إذا( !p_عبارت->x.pList ){
    أرجع 0;
  }
  n_عبارت = p_عبارت->x.pList->n_عبارت;
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
  pDef = sqlite3_پیدا_کن_Function(db, p_عبارت->u.zToken, n_عبارت, SQLITE_UTF8, 0);
#اگر_تعریف_شده SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  إذا( pDef==0 ) أرجع 0;
#پایان_اگر
  إذا( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    أرجع 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  کپی_حافظه(aWc, pDef->p_User_داده, 3);
  أكد( (حرف*)&likeInfoAlt == (حرف*)&likeInfoAlt.تطابق_All );
  أكد( &((حرف*)&likeInfoAlt)[1] == (حرف*)&likeInfoAlt.تطابق_One );
  أكد( &((حرف*)&likeInfoAlt)[2] == (حرف*)&likeInfoAlt.تطابق_مجموعه );

  إذا( n_عبارت<3 ){
    aWc[3] = 0;
  }وإلا{
    عبارت *pEscape = p_عبارت->x.pList->a[2].p_عبارت;
    حرف *zEscape;
    إذا( pEscape->op!=TK_STRING ) أرجع 0;
    أكد( !عبارت_Has_Property(pEscape, EP_Int_مقدار) );
    zEscape = pEscape->u.zToken;
    إذا( zEscape[0]==0 || zEscape[1]!=0 ) أرجع 0;
    إذا( zEscape[0]==aWc[0] ) أرجع 0;
    إذا( zEscape[0]==aWc[1] ) أرجع 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  أرجع 1;
}

/* Mathematical Constants */
#اگر_تعریف_نشده M_PI
# define M_PI   3.141592653589793238462643383279502884
#پایان_اگر
#اگر_تعریف_نشده M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#پایان_اگر
#اگر_تعریف_نشده M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#پایان_اگر


/* Extra math functions that require linking with -lm
*/
#اگر_تعریف_شده SQLITE_ENABLE_MATH_FUNCTIONS
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
ساكن فراغ ceilingFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==1 );
  بدل( sqlite3_مقدار_numeric_type(argv[0]) ){
    حالة SQLITE_INTEGER: {
       sqlite3_نتیجه_int64(context, sqlite3_مقدار_int64(argv[0]));
       اكسر;
    }
    حالة SQLITE_FLOAT: {
       مزدوج (*x)(مزدوج) = (مزدوج(*)(مزدوج))sqlite3_user_داده(context);
       sqlite3_نتیجه_double(context, x(sqlite3_مقدار_double(argv[0])));
       اكسر;
    }
    افتراضي: {
       اكسر;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
ساكن مزدوج xCeil(مزدوج x){ أرجع ceil(x); }
ساكن مزدوج xFloor(مزدوج x){ أرجع floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#إذا defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*لگاریتم(X))
#پایان_اگر
#إذا defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*لگاریتم(X))
#پایان_اگر


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
ساكن فراغ logFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  مزدوج x, b, ans;
  أكد( argc==1 || argc==2 );
  بدل( sqlite3_مقدار_numeric_type(argv[0]) ){
    حالة SQLITE_INTEGER:
    حالة SQLITE_FLOAT:
      x = sqlite3_مقدار_double(argv[0]);
      إذا( x<=0.0 ) أرجع;
      اكسر;
    افتراضي:
      أرجع;
  }
  إذا( argc==2 ){
    بدل( sqlite3_مقدار_numeric_type(argv[1]) ){
      حالة SQLITE_INTEGER:
      حالة SQLITE_FLOAT:
        b = لگاریتم(x);
        إذا( b<=0.0 ) أرجع;
        x = sqlite3_مقدار_double(argv[1]);
        إذا( x<=0.0 ) أرجع;
        اكسر;
     افتراضي:
        أرجع;
    }
    ans = لگاریتم(x)/b;
  }وإلا{
    بدل( SQLITE_PTR_مقصد_INT(sqlite3_user_داده(context)) ){
      حالة 1:
        ans = log10(x);
        اكسر;
      حالة 2:
        ans = log2(x);
        اكسر;
      افتراضي:
        ans = لگاریتم(x);
        اكسر;
    }
  }
  sqlite3_نتیجه_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
ساكن مزدوج deg_مقصد_Rad(مزدوج x){ أرجع x*(M_PI/180.0); }
ساكن مزدوج rad_مقصد_Deg(مزدوج x){ أرجع x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
ساكن فراغ math1Func(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح type0;
  مزدوج v0, ans;
  مزدوج (*x)(مزدوج);
  أكد( argc==1 );
  type0 = sqlite3_مقدار_numeric_type(argv[0]);
  إذا( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) أرجع;
  v0 = sqlite3_مقدار_double(argv[0]);
  x = (مزدوج(*)(مزدوج))sqlite3_user_داده(context);
  ans = x(v0);
  sqlite3_نتیجه_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
ساكن فراغ math2Func(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح type0, type1;
  مزدوج v0, v1, ans;
  مزدوج (*x)(مزدوج,مزدوج);
  أكد( argc==2 );
  type0 = sqlite3_مقدار_numeric_type(argv[0]);
  إذا( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) أرجع;
  type1 = sqlite3_مقدار_numeric_type(argv[1]);
  إذا( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) أرجع;
  v0 = sqlite3_مقدار_double(argv[0]);
  v1 = sqlite3_مقدار_double(argv[1]);
  x = (مزدوج(*)(مزدوج,مزدوج))sqlite3_user_داده(context);
  ans = x(v0, v1);
  sqlite3_نتیجه_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
ساكن فراغ piFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  أكد( argc==0 );
  (فراغ)argv;
  sqlite3_نتیجه_double(context, M_PI);
}

#پایان_اگر /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
ساكن فراغ signFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح type0;
  مزدوج x;
  استفادهنشده_PARAMETER(argc);
  أكد( argc==1 );
  type0 = sqlite3_مقدار_numeric_type(argv[0]);
  إذا( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) أرجع;
  x = sqlite3_مقدار_double(argv[0]);
  sqlite3_نتیجه_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#إذا defined(SQLITE_ENABLE_PERCENTILE)
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
تعريفنوع بنية Percentile Percentile;
بنية Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 n_استفاده_شده;           /* Number of slots actually used in a[] */
  حرف bSorted;        /* True if a[] is already in sorted order */
  حرف bKeepSorted;    /* True if advantageous to keep a[] sorted */
  حرف bPctValid;      /* True if rPct is valid */
  مزدوج rPct;         /* Fraction.  0.0 to 1.0 */
  مزدوج *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
ساكن صحيح percentIsInfinity(مزدوج r){
  sqlite3_uint64 u;
  أكد( حجم(u)==حجم(r) );
  کپی_حافظه(&u, &r, حجم(u));
  أرجع ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
ساكن صحيح percent_Same_مقدار(مزدوج a, مزدوج b){
  a -= b;
  أرجع a>=-0.001 && a<=0.001;
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
ساكن i64 percent_دودویی_جستجو(Percentile *p, مزدوج y, صحيح bExact){
  i64 i_اولی = 0;                   /* First element of search range */
  i64 i_آخرین = (i64)p->n_استفاده_شده - 1;    /* Last element of search range */
  طالما( i_آخرین>=i_اولی ){
    i64 iMid = (i_اولی+i_آخرین)/2;
    مزدوج x = p->a[iMid];
    إذا( x<y ){
      i_اولی = iMid + 1;
    }وإلا إذا( x>y ){
      i_آخرین = iMid - 1;
    }وإلا{
      أرجع iMid;
    }
  }
  إذا( bExact ) أرجع -1;
  أرجع i_اولی;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
ساكن فراغ percentError(sqlite3_context *pCtx, ثابت حرف *zFormat, ...){
  حرف *zMsg1;
  حرف *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_نتیجه_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
ساكن فراغ percent_گام(sqlite3_context *pCtx, صحيح argc, sqlite3_مقدار **argv){
  Percentile *p;
  مزدوج rPct;
  صحيح eType;
  مزدوج y;
  أكد( argc==2 || argc==1 );

  إذا( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }وإلا{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    مزدوج mxFrac = (SQLITE_PTR_مقصد_INT(sqlite3_user_داده(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_مقدار_numeric_type(argv[1]);
    rPct = sqlite3_مقدار_double(argv[1])/mxFrac;
    إذا( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (مزدوج)mxFrac);
      أرجع;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, حجم(*p));
  إذا( p==0 ) أرجع;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  إذا( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }وإلا إذا( !percent_Same_مقدار(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    أرجع;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_مقدار_type(argv[0]);
  إذا( eType==SQLITE_NULL ) أرجع;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  إذا( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    أرجع;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_مقدار_double(argv[0]);
  إذا( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    أرجع;
  }

  /* Allocate and store the Y */
  إذا( p->n_استفاده_شده>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    مزدوج *a = sqlite3_realloc64(p->a, حجم(مزدوج)*n);
    إذا( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, حجم(*p));
      sqlite3_نتیجه_error_nomem(pCtx);
      أرجع;
    }
    p->nAlloc = n;
    p->a = a;
  }
  إذا( p->n_استفاده_شده==0 ){
    p->a[p->n_استفاده_شده++] = y;
    p->bSorted = 1;
  }وإلا إذا( !p->bSorted || y>=p->a[p->n_استفاده_شده-1] ){
    p->a[p->n_استفاده_شده++] = y;
  }وإلا إذا( p->bKeepSorted ){
    i64 i;
    i = percent_دودویی_جستجو(p, y, 0);
    إذا( i<(صحيح)p->n_استفاده_شده ){
      memmove(&p->a[i+1], &p->a[i], (p->n_استفاده_شده-i)*حجم(p->a[0]));
    }
    p->a[i] = y;
    p->n_استفاده_شده++;
  }وإلا{
    p->a[p->n_استفاده_شده++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#تعریف_کن جابجا_DOUBLE(X,Y)  {مزدوج ttt=(X);(X)=(Y);(Y)=ttt;}

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
ساكن فراغ percent_مرتب_سازی(
  مزدوج *a,                      /* Array to sort */
  بیعلامت صحيح n,                 /* Number of elements in array a[] */
  صحيح iReq                        /* Element caller cares about (or -ve) */
){
  صحيح iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  صحيح iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  صحيح i;         /* Loop counter */
  مزدوج r_محور; /* The pivot value */

  أكد( n>=2 );
  افعل{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    إذا( a[0]>a[n-1] ){
      جابجا_DOUBLE(a[0],a[n-1])
    }
    إذا( n==2 ) أرجع;
    iGt = n-1;
    i = n/2;
    إذا( a[0]>a[i] ){
      جابجا_DOUBLE(a[0],a[i])
    }وإلا إذا( a[i]>a[iGt] ){
      جابجا_DOUBLE(a[i],a[iGt])
    }
    إذا( n==3 ) أرجع;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    r_محور = a[i];
    iLt = i = 1;
    افعل{
      إذا( a[i]<r_محور ){
        إذا( i>iLt ) جابجا_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }وإلا إذا( a[i]>r_محور ){
        افعل{
          iGt--;
        }طالما( iGt>i && a[iGt]>r_محور );
        جابجا_DOUBLE(a[i],a[iGt])
      }وإلا{
        i++;
      }
    }طالما( i<iGt );

    أكد( iLt>0 && iLt<iGt && (بیعلامت)iGt<n );
    testcase( iGt>iLt+1 );
    أكد( a[iLt]==r_محور );
    أكد( a[iLt-1]<=r_محور );
    أكد( a[iGt]>=r_محور );
    أكد( a[iLt+1]>=r_محور );

    إذا( iReq>=0 ){
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
      إذا( iReq<iLt ){
        n = iLt;
      }وإلا{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }وإلا{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      إذا( iLt>(صحيح)(n/2) ){
        إذا( n-iGt>=2 ) percent_مرتب_سازی(a+iGt, n-iGt, -1);
        n = iLt;
      }وإلا{
        إذا( iLt>=2 ) percent_مرتب_سازی(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }طالما( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
ساكن فراغ percentInverse(sqlite3_context *pCtx,صحيح argc,sqlite3_مقدار **argv){
  Percentile *p;
  صحيح eType;
  مزدوج y;
  i64 i;
  أكد( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, حجم(*p));
  أكد( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_مقدار_type(argv[0]);
  إذا( eType==SQLITE_NULL ) أرجع;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  إذا( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    أرجع;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_مقدار_double(argv[0]);
  إذا( percentIsInfinity(y) ){
    أرجع;
  }
  إذا( p->bSorted==0 ){
    أكد( p->n_استفاده_شده>1 );
    percent_مرتب_سازی(p->a, p->n_استفاده_شده, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_دودویی_جستجو(p, y, 1);
  إذا( i>=0 ){
    p->n_استفاده_شده--;
    إذا( i<(صحيح)p->n_استفاده_شده ){
      memmove(&p->a[i], &p->a[i+1], (p->n_استفاده_شده - i)*حجم(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
ساكن فراغ percentCompute(sqlite3_context *pCtx, صحيح bIsFinal){
  Percentile *p;
  صحيح settings = SQLITE_PTR_مقصد_INT(sqlite3_user_داده(pCtx))&1; /* Discrete? */
  بیعلامت i1, i2;
  مزدوج v1, v2;
  مزدوج ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  إذا( p==0 ) أرجع;
  إذا( p->a==0 ) أرجع;
  إذا( p->n_استفاده_شده ){
    ix = p->rPct*(p->n_استفاده_شده-1);
    i1 = (بیعلامت)ix;
    إذا( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      أكد( p->n_استفاده_شده>1 );
      percent_مرتب_سازی(p->a, p->n_استفاده_شده, (bIsFinal ? (صحيح)i1 : -1));
      p->bSorted = 1;
    }
    إذا( settings & 1 ){
      vx = p->a[i1];
    }وإلا{
      i2 = ix==(مزدوج)i1 || i1==p->n_استفاده_شده-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_نتیجه_double(pCtx, vx);
  }
  إذا( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, حجم(*p));
  }وإلا{
    p->bKeepSorted = 1;
  }
}
ساكن فراغ percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
ساكن فراغ percent_مقدار(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#پایان_اگر /* SQLITE_ENABLE_PERCENTILE */

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
ساكن فراغ filestatFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
  pBtree = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
  إذا( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    صحيح rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    أكد( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    إذا( sqlite3_str_errcode(pStr) ){
      sqlite3_نتیجه_error_nomem(context);
    }وإلا{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      إذا( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      إذا( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        إذا( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_نتیجه_str(context, pStr, SQLITE_پایان);
    }
    sqlite3BtreeLeave(pBtree);
  }وإلا{
    sqlite3_نتیجه_متن(context, "{}", 2, SQLITE_STATIC);
  }
}
#پایان_اگر /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
ساكن فراغ walstatFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
  pBtree = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
  إذا( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    أكد( pPager!=0 );
    pStr = sqlite3_str_new(db);
    إذا( sqlite3_str_errcode(pStr) ){
      sqlite3_نتیجه_error_nomem(context);
    }وإلا{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_نتیجه_str(context, pStr, SQLITE_پایان);
    }
    sqlite3BtreeLeave(pBtree);
  }وإلا{
    sqlite3_نتیجه_متن(context, "{}", 2, SQLITE_STATIC);
  }
}
#پایان_اگر /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#اگر_تعریف_شده SQLITE_DEBUG
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
ساكن فراغ fpdecodeFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  FpDecode s;
  مزدوج x;
  صحيح y, z;
  حرف zBuf[100];
  استفادهنشده_PARAMETER(argc);
  أكد( argc==3 );
  x = sqlite3_مقدار_double(argv[0]);
  y = sqlite3_مقدار_int(argv[1]);
  z = sqlite3_مقدار_int(argv[2]);
  إذا( z<=0 ) z = 1;
  إذا( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  إذا( s.isSpecial==2 ){
    sqlite3_snprintf(حجم(zBuf), zBuf, "NaN");
  }وإلا{
    sqlite3_snprintf(حجم(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_نتیجه_متن(context, zBuf, -1, SQLITE_TRANSIENT);
}
#پایان_اگر /* SQLITE_DEBUG */

#اگر_تعریف_شده SQLITE_DEBUG
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
ساكن فراغ parseuriFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_مقدار **argv
){
  sqlite3_str *p_نتیجه;
  ثابت حرف *zVfs;
  ثابت حرف *zUri;
  بیعلامت صحيح flgs;
  صحيح rc;
  sqlite3_vfs *pVfs = 0;
  حرف *zFile = 0;
  حرف *zErr = 0;

  إذا( argc<2 ) أرجع;
  pVfs = sqlite3_vfs_پیدا_کن(0);
  أكد( pVfs );
  zVfs = pVfs->zName;
  zUri = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
  إذا( zUri==0 ) أرجع;
  flgs = (بیعلامت صحيح)sqlite3_مقدار_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_نتیجه = sqlite3_str_new(0);
  إذا( !sqlite3_str_errcode(p_نتیجه) ){
    صحيح i;
    sqlite3_str_appendf(p_نتیجه, "rc=%d", rc);
    sqlite3_str_appendf(p_نتیجه, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_نتیجه, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_نتیجه, ", err=%Q", zErr);
    sqlite3_str_appendf(p_نتیجه, ", file=%Q", zFile);
    إذا( zFile ){
      ثابت حرف *z = zFile;
      z += sqlite3Strlen30(z)+1;
      طالما( z[0] ){
        sqlite3_str_appendf(p_نتیجه, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      لكل(i=2; i<argc; i++){
        ثابت حرف *zArg;
        إذا( sqlite3_مقدار_type(argv[i])==SQLITE_INTEGER ){
          صحيح k = sqlite3_مقدار_int(argv[i]);
          sqlite3_str_appendf(p_نتیجه, ", '%d:%q'",k,sqlite3_uri_کلید(zFile, k));
        }وإلا إذا( (zArg = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]))!=0 ){
          sqlite3_str_appendf(p_نتیجه, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }وإلا{
          sqlite3_str_appendf(p_نتیجه, ", NULL");
        }
      }
    }
  }
  sqlite3_نتیجه_str(ctx, p_نتیجه, SQLITE_پایان);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#پایان_اگر /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
فراغ sqlite3_ثبات_Builtin_Functions(فراغ){
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
  ساكن FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#إذا !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(نتیجه_میدهد_nonnull_ردیف, 2, INLINEFUNC_نتیجه_میدهد_nonnull_ردیف, 0),
    TEST_FUNC(عبارت_compare,        2, INLINEFUNC_عبارت_compare,        0),
    TEST_FUNC(عبارت_نتیجه_میدهد_عبارت,   2, INLINEFUNC_عبارت_نتیجه_میدهد_عبارت,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#پایان_اگر /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#اگر_تعریف_شده SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_بارگیری_EXTENSION
    SFUNCTION(بارگیری_extension,    1, 0, 0, بارگیری_Ext          ),
    SFUNCTION(بارگیری_extension,    2, 0, 0, بارگیری_Ext          ),
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_استفاده_شده,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#پایان_اگر /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#اگر_تعریف_شده SQLITE_ENABLE_جابه_جایی_SQL_FUNC
    INLINE_FUNC(sqlite_جابه_جایی,   1, INLINEFUNC_sqlite_جابه_جایی, 0 ),
#پایان_اگر
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#پایان_اگر
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmax_گام, minMaxFinalize, min_Max_مقدار, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmax_گام, minMaxFinalize, min_Max_مقدار, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(درازی,            1, 0, 0, درازی_Func,  SQLITE_FUNC_درازی),
    FUNCTION2(octet_درازی,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(چاپ_کن,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(حرف,              -1, 0, 0, charFunc         ),
    FUNCTION(قدر_مطلق,                1, 0, 0, absFunc          ),
#اگر_تعریف_شده SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#پایان_اگر
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
    FUNCTION(دور,              1, 0, 0, دور_Func        ),
    FUNCTION(دور,              2, 0, 0, دور_Func        ),
#پایان_اگر
    FUNCTION(فوقانی,              1, 0, 0, فوقانی_Func        ),
    FUNCTION(پایینی,              1, 0, 0, پایینی_Func        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_نسخه,    0, 0, 0, نسخه_Func      ),
    DFUNCTION(sqlite_منبع_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(آخرین_درج_rowid, 0, 0, 0, آخرین_درج_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(مجموع_changes,     0, 0, 0, مجموع_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(جمع,   1,0,0, جمع_گام, جمع_Finalize, جمع_Finalize, جمع_Inverse, 0),
    WAGGREGATE(مجموع, 1,0,0, جمع_گام,مجموع_Finalize,مجموع_Finalize,جمع_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, جمع_گام, avgFinalize, avgFinalize, جمع_Inverse, 0),
    WAGGREGATE(شمار, 0,0,0, شمار_گام,
        شمار_Finalize, شمار_Finalize, شمار_Inverse,
        SQLITE_FUNC_شمار|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(شمار, 1,0,0, شمار_گام,
        شمار_Finalize, شمار_Finalize, شمار_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(گروه_concat, 1, 0, 0, گروه_Concat_گام,
        گروه_Concat_Finalize, گروه_Concat_مقدار, گروه_Concat_Inverse, 0),
    WAGGREGATE(گروه_concat, 2, 0, 0, گروه_Concat_گام,
        گروه_Concat_Finalize, گروه_Concat_مقدار, گروه_Concat_Inverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, گروه_Concat_گام,
        گروه_Concat_Finalize, گروه_Concat_مقدار, گروه_Concat_Inverse, 0),

#اگر_تعریف_شده SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percent_گام,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percent_گام,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percent_گام,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percent_گام,
        percentFinal, percent_مقدار, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#پایان_اگر /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#اگر_تعریف_شده SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#وإلا
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#پایان_اگر
#اگر_تعریف_شده SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#پایان_اگر
#اگر_تعریف_شده SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#إذا SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#پایان_اگر
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(لگاریتم,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(لگاریتم,                2, 0, 0,      logFunc     ),
    MFUNCTION(نمایی_تابع,               1, نمایی_تابع,       math1Func   ),
    MFUNCTION(توان,               2, توان,       math2Func   ),
    MFUNCTION(power,             2, توان,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(کسینوس,               1, کسینوس,       math1Func   ),
    MFUNCTION(سینوس,               1, سینوس,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#إذا SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#پایان_اگر
    MFUNCTION(جذر,              1, جذر,      math1Func   ),
    MFUNCTION(radians,           1, deg_مقصد_Rad,  math1Func   ),
    MFUNCTION(degrees,           1, rad_مقصد_Deg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#پایان_اگر /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(إذا,             -4, INLINEFUNC_iif,      0 ),
  };
#اگر_تعریف_نشده SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#پایان_اگر
  sqlite3_پنجره_Functions();
  sqlite3_ثبات_Date_زمان_Functions();
  sqlite3_ثبات_Json_Functions();
  sqlite3_درج_Builtin_Funcs(aBuiltinFunc, Array_اندازه(aBuiltinFunc));

#إذا 0  /* Enable to print out how the built-in functions are hashed */
  {
    صحيح i;
    FuncDef *p;
    لكل(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      چاپ_کن("FUNC-HASH %02d:", i);
      لكل(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        صحيح n = sqlite3Strlen30(p->zName);
        صحيح h = p->zName[0] + n;
        أكد( p->funcFlags & SQLITE_FUNC_BUILTIN );
        چاپ_کن(" %s(%d)", p->zName, h);
      }
      چاپ_کن("\n");
    }
  }
#پایان_اگر
}
