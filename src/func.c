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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
#شامل <math.h>
#اگر_ختم
#شامل "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
ساکن CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  أكد( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  أكد( pOp->opcode==OP_CollSeq );
  أكد( pOp->p4type==P4_COLLSEQ );
  واپس pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
ساکن خالی sqlite3_Skip_مجمع_بھرو(sqlite3_context *context){
  أكد( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
ساکن خالی minmaxFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح i;
  صحیح نقاب;    /* 0 for min() or 0xffffffff for max() */
  صحیح i_بہترین;
  CollSeq *pColl;

  أكد( argc>1 );
  نقاب = sqlite3_user_ڈیٹا(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  أكد( pColl );
  أكد( نقاب==-1 || نقاب==0 );
  i_بہترین = 0;
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) واپس;
  برائے(i=1; i<argc; i++){
    اگر( sqlite3_قدر_type(argv[i])==SQLITE_NULL ) واپس;
    اگر( (sqlite3MemCompare(argv[i_بہترین], argv[i], pColl)^نقاب)>=0 ){
      testcase( نقاب==0 );
      i_بہترین = i;
    }
  }
  sqlite3_نتیجہ_قدر(context, argv[i_بہترین]);
}

/*
** Return the type of the argument.
*/
ساکن خالی typeofFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  ساکن ثابت حرف *azType[] = { "integer", "real", "text", "blob", "null" };
  صحیح i = sqlite3_قدر_type(argv[0]) - 1;
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  أكد( i>=0 && i<Array_سائز(azType) );
  أكد( SQLITE_INTEGER==1 );
  أكد( SQLITE_FLOAT==2 );
  أكد( SQLITE_متن==3 );
  أكد( SQLITE_BLOB==4 );
  أكد( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_نتیجہ_متن(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
ساکن خالی subtypeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  غیر_مستعمل_PARAMETER(argc);
  sqlite3_نتیجہ_int(context, sqlite3_قدر_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
ساکن خالی لمبائی_Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  انتخاب( sqlite3_قدر_type(argv[0]) ){
    حالت SQLITE_BLOB:
    حالت SQLITE_INTEGER:
    حالت SQLITE_FLOAT: {
      sqlite3_نتیجہ_int(context, sqlite3_قدر_بائٹیں(argv[0]));
      توڑو;
    }
    حالت SQLITE_متن: {
      ثابت غیرعلامتی حرف *z = sqlite3_قدر_متن(argv[0]);
      ثابت غیرعلامتی حرف *z0;
      اگر( z==0 ) واپس;
      z0 = z;
      جب( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        اگر( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }ورنہ اگر( z[0]==0 ){
          توڑو;
        }ورنہ{
          z++;
          جب( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_نتیجہ_int(context, (صحیح)(z-z0));
      توڑو;
    }
    طےشدہ: {
      sqlite3_نتیجہ_null(context);
      توڑو;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
ساکن خالی bytelengthFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  انتخاب( sqlite3_قدر_type(argv[0]) ){
    حالت SQLITE_BLOB: {
      sqlite3_نتیجہ_int(context, sqlite3_قدر_بائٹیں(argv[0]));
      توڑو;
    }
    حالت SQLITE_INTEGER:
    حالت SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_نتیجہ_int64(context, sqlite3_قدر_بائٹیں(argv[0])*m);
      توڑو;
    }
    حالت SQLITE_متن: {
      اگر( sqlite3_قدر_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_نتیجہ_int(context, sqlite3_قدر_بائٹیں(argv[0]));
      }ورنہ{
        sqlite3_نتیجہ_int(context, sqlite3_قدر_bytes16(argv[0]));
      }
      توڑو;
    }
    طےشدہ: {
      sqlite3_نتیجہ_null(context);
      توڑو;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
ساکن خالی absFunc(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  انتخاب( sqlite3_قدر_type(argv[0]) ){
    حالت SQLITE_INTEGER: {
      i64 iVal = sqlite3_قدر_int64(argv[0]);
      اگر( iVal<0 ){
        اگر( iVal==سب_سے_چھوٹا_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_نتیجہ_error(context, "integer overflow", -1);
          واپس;
        }
        iVal = -iVal;
      }
      sqlite3_نتیجہ_int64(context, iVal);
      توڑو;
    }
    حالت SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_نتیجہ_null(context);
      توڑو;
    }
    طےشدہ: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      دوہرا rVal = sqlite3_قدر_double(argv[0]);
      اگر( rVal<0 ) rVal = -rVal;
      sqlite3_نتیجہ_double(context, rVal);
      توڑو;
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
ساکن خالی instrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت غیرعلامتی حرف *zHaystack;
  ثابت غیرعلامتی حرف *zNeedle;
  صحیح nHaystack;
  صحیح nNeedle;
  صحیح typeHaystack, typeNeedle;
  صحیح N = 1;
  صحیح is_متن;
  غیرعلامتی حرف پہلا_Char;
  sqlite3_قدر *pC1 = 0;
  sqlite3_قدر *pC2 = 0;

  غیر_مستعمل_PARAMETER(argc);
  typeHaystack = sqlite3_قدر_type(argv[0]);
  typeNeedle = sqlite3_قدر_type(argv[1]);
  اگر( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) واپس;
  nHaystack = sqlite3_قدر_بائٹیں(argv[0]);
  nNeedle = sqlite3_قدر_بائٹیں(argv[1]);
  اگر( nNeedle>0 ){
    اگر( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_قدر_blob(argv[0]);
      zNeedle = sqlite3_قدر_blob(argv[1]);
      is_متن = 0;
    }ورنہ اگر( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_قدر_متن(argv[0]);
      zNeedle = sqlite3_قدر_متن(argv[1]);
      is_متن = 1;
    }ورنہ{
      pC1 = sqlite3_قدر_dup(argv[0]);
      zHaystack = sqlite3_قدر_متن(pC1);
      اگر( zHaystack==0 ) جاؤ انتہا_Instr_OOM;
      nHaystack = sqlite3_قدر_بائٹیں(pC1);
      pC2 = sqlite3_قدر_dup(argv[1]);
      zNeedle = sqlite3_قدر_متن(pC2);
      اگر( zNeedle==0 ) جاؤ انتہا_Instr_OOM;
      nNeedle = sqlite3_قدر_بائٹیں(pC2);
      is_متن = 1;
    }
    اگر( zNeedle==0 || (nHaystack && zHaystack==0) ) جاؤ انتہا_Instr_OOM;
    پہلا_Char = zNeedle[0];
    جب( nNeedle<=nHaystack
       && (zHaystack[0]!=پہلا_Char || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      کرو{
        nHaystack--;
        zHaystack++;
      }جب( is_متن && (zHaystack[0]&0xc0)==0x80 );
    }
    اگر( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_نتیجہ_int(context, N);
انتہا_Instr:
  sqlite3_قدر_free(pC1);
  sqlite3_قدر_free(pC2);
  واپس;
انتہا_Instr_OOM:
  sqlite3_نتیجہ_error_nomem(context);
  جاؤ انتہا_Instr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
ساکن خالی printfFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  PrintfArguments x;
  StrAccum str;
  ثابت حرف *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  اگر( argc>=1 && (zFormat = (ثابت حرف*)sqlite3_قدر_متن(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.n_مستعمل = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_نتیجہ_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
ساکن SQLITE_INLINE صحیح شامل_ہے_Nul_Or_Non_Ascii(u64 لفظ){
  ثابت u64 byteOnes = UINT64_C(0x0101010101010101);
  ثابت u64 byte_بلند_بٹ = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  واپس ((لفظ | (لفظ - byteOnes)) & byte_بلند_بٹ)!=0;
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
ساکن خالی substrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت غیرعلامتی حرف *z;
  ثابت غیرعلامتی حرف *z2;
  صحیح len;
  صحیح p0type;
  i64 p1, p2;

  أكد( argc==3 || argc==2 );
  p0type = sqlite3_قدر_type(argv[0]);
  p1 = sqlite3_قدر_int64(argv[1]);
  اگر( p0type==SQLITE_BLOB ){
    len = sqlite3_قدر_بائٹیں(argv[0]);
    z = sqlite3_قدر_blob(argv[0]);
    اگر( z==0 ) واپس;
    أكد( len==sqlite3_قدر_بائٹیں(argv[0]) );
  }ورنہ{
    z = sqlite3_قدر_متن(argv[0]);
    اگر( z==0 ) واپس;
    len = 0;
    اگر( p1<0 ){
      برائے(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  اگر( argc==3 ){
    p2 = sqlite3_قدر_int64(argv[2]);
    اگر( p2==0 && sqlite3_قدر_type(argv[2])==SQLITE_NULL ) واپس;
  }ورنہ{
    p2 = سب_سے_بڑا_INT64;
  }
  اگر( p1==0 ){
#اگر_تعریف_شدہ SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#اگر_ختم
    اگر( sqlite3_قدر_type(argv[1])==SQLITE_NULL ) واپس;
  }
  اگر( p1<0 ){
    p1 += len;
    اگر( p1<0 ){
      اگر( p2<0 ){
        p2 = 0;
      }ورنہ{
        p2 += p1;
      }
      p1 = 0;
    }
  }ورنہ اگر( p1>0 ){
    p1--;
  }ورنہ اگر( p2>0 ){
    p2--;
  }
  اگر( p2<0 ){
    اگر( p2<-p1 ){
      p2 = p1;
    }ورنہ{
      p2 = -p2;
    }
    p1 -= p2;
  }
  أكد( p1>=0 && p2>=0 );
  اگر( p0type!=SQLITE_BLOB ){
    اگر( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ثابت غیرعلامتی حرف *z_انتہا = z + sqlite3_قدر_بائٹیں(argv[0]);
      جب( p1>=8 && z_انتہا-z>=8 ){
        u64 لفظ;
        حافظہ_نقل(&لفظ, z, حجم(لفظ));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        اگر( شامل_ہے_Nul_Or_Non_Ascii(لفظ) ){
          توڑو;
        }
        z += 8;
        p1 -= 8;
      }
    }
    برائے( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      اگر( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }ورنہ اگر( z[0]==0 ){
        توڑو;
      }ورنہ{
        کرو{ z++; }جب( (z[0]&0xc0)==0x80 );
      }
    }
    برائے(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      اگر( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }ورنہ اگر( z2[0]==0 ){
        توڑو;
      }ورنہ{
        کرو{ z2++; }جب( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_نتیجہ_text64(context, (حرف*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }ورنہ{
    اگر( p1>=len ){
      p1 = p2 = 0;
    }ورنہ اگر( p2>len-p1 ){
      p2 = len-p1;
      أكد( p2>0 );
    }
    sqlite3_نتیجہ_blob64(context, (حرف*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
ساکن خالی باری_Func(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  i64 n = 0;
  دوہرا r;
  حرف *zBuf;
  أكد( argc==1 || argc==2 );
  اگر( argc==2 ){
    اگر( SQLITE_NULL==sqlite3_قدر_type(argv[1]) ) واپس;
    n = sqlite3_قدر_int64(argv[1]);
    اگر( n>30 ) n = 30;
    اگر( n<0 ) n = 0;
  }
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) واپس;
  r = sqlite3_قدر_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  اگر( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }ورنہ اگر( n==0 ){ 
    r = (دوہرا)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }ورنہ{
    zBuf = sqlite3_mprintf("%!.*f",(صحیح)n,r);
    اگر( zBuf==0 ){
      sqlite3_نتیجہ_error_nomem(context);
      واپس;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_نتیجہ_double(context, r);
}
#اگر_ختم

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
ساکن خالی *contextMalloc(sqlite3_context *context, i64 nByte){
  حرف *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  أكد( nByte>0 );
  testcase( nByte==db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] );
  testcase( nByte==(i64)db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]+1 );
  اگر( nByte>db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] ){
    sqlite3_نتیجہ_error_toobig(context);
    z = 0;
  }ورنہ{
    z = sqlite3Malloc(nByte);
    اگر( !z ){
      sqlite3_نتیجہ_error_nomem(context);
    }
  }
  واپس z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
ساکن خالی بالائی_Func(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  حرف *z1;
  ثابت حرف *z2;
  صحیح i, n;
  غیر_مستعمل_PARAMETER(argc);
  z2 = (حرف*)sqlite3_قدر_متن(argv[0]);
  n = sqlite3_قدر_بائٹیں(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  أكد( z2==(حرف*)sqlite3_قدر_متن(argv[0]) );
  اگر( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    اگر( z1 ){
      برائے(i=0; i<n; i++){
        z1[i] = (حرف)sqlite3Toupper(z2[i]);
      }
      sqlite3_نتیجہ_متن(context, z1, n, sqlite3_free);
    }
  }
}
ساکن خالی زیریں_Func(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  حرف *z1;
  ثابت حرف *z2;
  صحیح i, n;
  غیر_مستعمل_PARAMETER(argc);
  z2 = (حرف*)sqlite3_قدر_متن(argv[0]);
  n = sqlite3_قدر_بائٹیں(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  أكد( z2==(حرف*)sqlite3_قدر_متن(argv[0]) );
  اگر( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    اگر( z1 ){
      برائے(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_نتیجہ_متن(context, z1, n, sqlite3_free);
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
#تعریف_کرو noopFunc نسخہ_Func   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
ساکن خالی randomFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  sqlite_int64 r;
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  sqlite3_randomness(حجم(r), &r);
  اگر( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & سب_سے_بڑا_INT64);
  }
  sqlite3_نتیجہ_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
ساکن خالی randomBlob(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3_int64 n;
  غیرعلامتی حرف *p;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  n = sqlite3_قدر_int64(argv[0]);
  اگر( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  اگر( p ){
    sqlite3_randomness(n, p);
    sqlite3_نتیجہ_blob(context, (حرف*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
ساکن خالی آخری_داخل_کرو_rowid(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_نتیجہ_int64(context, sqlite3_آخری_داخل_کرو_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
ساکن خالی changes(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  sqlite3_نتیجہ_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
ساکن خالی کل_changes(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_نتیجہ_int64(context, sqlite3_کل_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
ساخت compareInfo {
  u8 مطابقت_All;          /* "*" or "%" */
  u8 مطابقت_One;          /* "?" or "_" */
  u8 مطابقت_سیٹ;          /* "[" or 0 */
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
#ورنہ
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#اگر_ختم

ساکن ثابت ساخت compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
ساکن ثابت ساخت compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
ساکن ثابت ساخت compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#تعریف_کرو SQLITE_مطابقت             0
#تعریف_کرو SQLITE_NOMATCH           1
#تعریف_کرو SQLITE_NOWILDCARDMATCH   2

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
ساکن صحیح نمونہ_Compare(
  ثابت u8 *z_نمونہ,              /* The glob pattern */
  ثابت u8 *zString,               /* The string to compare against the glob */
  ثابت ساخت compareInfo *pInfo, /* Information about how to do the compare */
  u32 مطابقت_Other                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 مطابقت_One = pInfo->مطابقت_One;  /* "?" or "_" */
  u32 مطابقت_All = pInfo->مطابقت_All;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ثابت u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  جب( (c = Utf8Read(z_نمونہ))!=0 ){
    اگر( c==مطابقت_All ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      جب( (c=Utf8Read(z_نمونہ)) == مطابقت_All
             || (c == مطابقت_One && مطابقت_One!=0) ){
        اگر( c==مطابقت_One && sqlite3Utf8Read(&zString)==0 ){
          واپس SQLITE_NOWILDCARDMATCH;
        }
      }
      اگر( c==0 ){
        واپس SQLITE_مطابقت;   /* "*" at the end of the pattern matches */
      }ورنہ اگر( c==مطابقت_Other ){
        اگر( pInfo->مطابقت_سیٹ==0 ){
          c = sqlite3Utf8Read(&z_نمونہ);
          اگر( c==0 ) واپس SQLITE_NOWILDCARDMATCH;
        }ورنہ{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          أكد( مطابقت_Other<0x80 );  /* '[' is a single-byte character */
          جب( *zString ){
            صحیح b_مطابقت = نمونہ_Compare(&z_نمونہ[-1],zString,pInfo,مطابقت_Other);
            اگر( b_مطابقت!=SQLITE_NOMATCH ) واپس b_مطابقت;
            SQLITE_SKIP_UTF8(zString);
          }
          واپس SQLITE_NOWILDCARDMATCH;
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
        حرف zStop[3];
        صحیح b_مطابقت;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = z_نمونہ[0], ny = nx;
        اگر( nx>=0x80 || nx==مطابقت_All || nx==مطابقت_One || nx==مطابقت_Other ){
          nx = 0;
        }
        اگر( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }ورنہ{
          zStop[0] = c;
          zStop[1] = 0;
        }
        جب(1){
          zString += strcspn((ثابت حرف*)zString, zStop);
          اگر( zString[0]==0 ) توڑو;
          zString++;
          اگر( nx && zString[0]!=nx && zString[0]!=ny ) جاری;
          b_مطابقت = نمونہ_Compare(z_نمونہ,zString,pInfo,مطابقت_Other);
          اگر( b_مطابقت!=SQLITE_NOMATCH ) واپس b_مطابقت;
        }
      }ورنہ{
        صحیح b_مطابقت;
        جب( (c2 = Utf8Read(zString))!=0 ){
          اگر( c2!=c ) جاری;
          b_مطابقت = نمونہ_Compare(z_نمونہ,zString,pInfo,مطابقت_Other);
          اگر( b_مطابقت!=SQLITE_NOMATCH ) واپس b_مطابقت;
        }
      }
      واپس SQLITE_NOWILDCARDMATCH;
    }
    اگر( c==مطابقت_Other ){
      اگر( pInfo->مطابقت_سیٹ==0 ){
        c = sqlite3Utf8Read(&z_نمونہ);
        اگر( c==0 ) واپس SQLITE_NOMATCH;
        zEscaped = z_نمونہ;
      }ورنہ{
        u32 پیشگی_c = 0;
        صحیح نظر_آیا = 0;
        صحیح invert = 0;
        c = sqlite3Utf8Read(&zString);
        اگر( c==0 ) واپس SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&z_نمونہ);
        اگر( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&z_نمونہ);
        }
        اگر( c2==']' ){
          اگر( c==']' ) نظر_آیا = 1;
          c2 = sqlite3Utf8Read(&z_نمونہ);
        }
        جب( c2 && c2!=']' ){
          اگر( c2=='-' && z_نمونہ[0]!=']' && z_نمونہ[0]!=0 && پیشگی_c>0 ){
            c2 = sqlite3Utf8Read(&z_نمونہ);
            اگر( c>=پیشگی_c && c<=c2 ) نظر_آیا = 1;
            پیشگی_c = 0;
          }ورنہ{
            اگر( c==c2 ){
              نظر_آیا = 1;
            }
            پیشگی_c = c2;
          }
          c2 = sqlite3Utf8Read(&z_نمونہ);
        }
        اگر( c2==0 || (نظر_آیا ^ invert)==0 ){
          واپس SQLITE_NOMATCH;
        }
        جاری;
      }
    }
    c2 = Utf8Read(zString);
    اگر( c==c2 ) جاری;
    اگر( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      جاری;
    }
    اگر( c==مطابقت_One && z_نمونہ!=zEscaped && c2!=0 ) جاری;
    واپس SQLITE_NOMATCH;
  }
  واپس *zString==0 ? SQLITE_مطابقت : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
صحیح sqlite3_strglob(ثابت حرف *z_Glob_نمونہ, ثابت حرف *zString){
  اگر( zString==0 ){
    واپس z_Glob_نمونہ!=0;
  }ورنہ اگر( z_Glob_نمونہ==0 ){
    واپس 1;
  }ورنہ {
    واپس نمونہ_Compare((u8*)z_Glob_نمونہ, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
صحیح sqlite3_strlike(ثابت حرف *z_نمونہ, ثابت حرف *zStr, غیرعلامتی صحیح esc){
  اگر( zStr==0 ){
    واپس z_نمونہ!=0;
  }ورنہ اگر( z_نمونہ==0 ){
    واپس 1;
  }ورنہ{
    واپس نمونہ_Compare((u8*)z_نمونہ, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#اگر_تعریف_شدہ SQLITE_TEST
صحیح sqlite3_like_گنتی = 0;
#اگر_ختم


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
ساکن خالی likeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت غیرعلامتی حرف *zA, *zB;
  u32 escape;
  صحیح nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ساخت compareInfo *pInfo = sqlite3_user_ڈیٹا(context);
  ساخت compareInfo backupInfo;

#اگر_تعریف_شدہ SQLITE_LIKE_DOESNT_مطابقت_BLOBS
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_BLOB
   || sqlite3_قدر_type(argv[1])==SQLITE_BLOB
  ){
#اگر_تعریف_شدہ SQLITE_TEST
    sqlite3_like_گنتی++;
#اگر_ختم
    sqlite3_نتیجہ_int(context, 0);
    واپس;
  }
#اگر_ختم

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_قدر_بائٹیں(argv[0]);
  testcase( nPat==db->a_حد_بندی[SQLITE_حد_بندی_LIKE_نمونہ_لمبائی] );
  testcase( nPat==db->a_حد_بندی[SQLITE_حد_بندی_LIKE_نمونہ_لمبائی]+1 );
  اگر( nPat > db->a_حد_بندی[SQLITE_حد_بندی_LIKE_نمونہ_لمبائی] ){
    sqlite3_نتیجہ_error(context, "LIKE or GLOB pattern too complex", -1);
    واپس;
  }
  اگر( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ثابت غیرعلامتی حرف *zEsc = sqlite3_قدر_متن(argv[2]);
    اگر( zEsc==0 ) واپس;
    اگر( sqlite3Utf8CharLen((حرف*)zEsc, -1)!=1 ){
      sqlite3_نتیجہ_error(context,
          "ESCAPE expression must be a single character", -1);
      واپس;
    }
    escape = sqlite3Utf8Read(&zEsc);
    اگر( escape==pInfo->مطابقت_All || escape==pInfo->مطابقت_One ){
      حافظہ_نقل(&backupInfo, pInfo, حجم(backupInfo));
      pInfo = &backupInfo;
      اگر( escape==pInfo->مطابقت_All ) pInfo->مطابقت_All = 0;
      اگر( escape==pInfo->مطابقت_One ) pInfo->مطابقت_One = 0;
    }
  }ورنہ{
    escape = pInfo->مطابقت_سیٹ;
  }
  zB = sqlite3_قدر_متن(argv[0]);
  zA = sqlite3_قدر_متن(argv[1]);
  اگر( zA && zB ){
#اگر_تعریف_شدہ SQLITE_TEST
    sqlite3_like_گنتی++;
#اگر_ختم
    sqlite3_نتیجہ_int(context,
                      نمونہ_Compare(zB, zA, pInfo, escape)==SQLITE_مطابقت);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
ساکن خالی nullifFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  اگر( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_نتیجہ_قدر(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
ساکن خالی نسخہ_Func(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_نتیجہ_متن(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
ساکن خالی sourceidFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_نتیجہ_متن(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
ساکن خالی errlogFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  غیر_مستعمل_PARAMETER(argc);
  غیر_مستعمل_PARAMETER(context);
  sqlite3_log(sqlite3_قدر_int(argv[0]), "%s", sqlite3_قدر_متن(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPILEOPTION_DIAGS
ساکن خالی compileoptionusedFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت حرف *zOptName;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  اگر( (zOptName = (ثابت حرف*)sqlite3_قدر_متن(argv[0]))!=0 ){
    sqlite3_نتیجہ_int(context, sqlite3_compileoption_مستعمل(zOptName));
  }
}
#اگر_ختم /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPILEOPTION_DIAGS
ساکن خالی compileoptiongetFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  i64 n;
  ثابت حرف *zRes;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_قدر_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((صحیح)n) : 0;
  sqlite3_نتیجہ_متن(context, zRes, -1, SQLITE_STATIC);
}
#اگر_ختم /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
ساکن ثابت حرف hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
خالی sqlite3_Quote_قدر(StrAccum *pStr, sqlite3_قدر *p_قدر, صحیح bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  أكد( pStr!=0 && pStr->nChar==0 );

  انتخاب( sqlite3_قدر_type(p_قدر) ){
    حالت SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_قدر_double(p_قدر));
      توڑو;
    }
    حالت SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_قدر_int64(p_قدر));
      توڑو;
    }
    حالت SQLITE_BLOB: {
      حرف ثابت *zBlob = sqlite3_قدر_blob(p_قدر);
      i64 nBlob = sqlite3_قدر_بائٹیں(p_قدر);
      أكد( zBlob==sqlite3_قدر_blob(p_قدر) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      اگر( pStr->accError==0 ){
        حرف *z_متن = pStr->z_متن;
        صحیح i;
        برائے(i=0; i<nBlob; i++){
          z_متن[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          z_متن[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        z_متن[(nBlob*2)+2] = '\'';
        z_متن[(nBlob*2)+3] = '\0';
        z_متن[0] = 'X';
        z_متن[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      توڑو;
    }
    حالت SQLITE_متن: {
      ثابت غیرعلامتی حرف *zArg = sqlite3_قدر_متن(p_قدر);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      توڑو;
    }
    طےشدہ: {
      أكد( sqlite3_قدر_type(p_قدر)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      توڑو;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
ساکن صحیح isNHex(ثابت حرف *z, صحیح N, u32 *pVal){
  صحیح i;
  u32 v = 0;
  برائے(i=0; i<N; i++){
    اگر( !sqlite3Isxdigit(z[i]) ) واپس 0;
    v = (v<<4) + sqlite3_Hex_مقصد_Int(z[i]);
  }
  *pVal = v;
  واپس 1;
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
ساکن خالی unistrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  حرف *z_برآمد;
  ثابت حرف *z_اندر;
  صحیح n_اندر;
  صحیح i, j, n;
  u32 v;

  أكد( argc==1 );
  غیر_مستعمل_PARAMETER( argc );
  z_اندر = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  اگر( z_اندر==0 ) واپس;
  n_اندر = sqlite3_قدر_بائٹیں(argv[0]);
  z_برآمد = sqlite3_malloc64(n_اندر+1);
  اگر( z_برآمد==0 ){
    sqlite3_نتیجہ_error_nomem(context);
    واپس;
  }
  i = j = 0;
  جب( i<n_اندر ){
    ثابت حرف *z = strchr(&z_اندر[i],'\\');
    اگر( z==0 ){
      n = n_اندر - i;
      memmove(&z_برآمد[j], &z_اندر[i], n);
      j += n;
      توڑو;
    }
    n = z - &z_اندر[i];
    اگر( n>0 ){
      memmove(&z_برآمد[j], &z_اندر[i], n);
      j += n;
      i += n;
    }
    اگر( z_اندر[i+1]=='\\' ){
      i += 2;
      z_برآمد[j++] = '\\';
    }ورنہ اگر( sqlite3Isxdigit(z_اندر[i+1]) ){
      اگر( !isNHex(&z_اندر[i+1], 4, &v) ) جاؤ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&z_برآمد[j], v);
    }ورنہ اگر( z_اندر[i+1]=='+' ){
      اگر( !isNHex(&z_اندر[i+2], 6, &v) ) جاؤ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&z_برآمد[j], v);
    }ورنہ اگر( z_اندر[i+1]=='u' ){
      اگر( !isNHex(&z_اندر[i+2], 4, &v) ) جاؤ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&z_برآمد[j], v);
    }ورنہ اگر( z_اندر[i+1]=='U' ){
      اگر( !isNHex(&z_اندر[i+2], 8, &v) ) جاؤ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&z_برآمد[j], v);
    }ورنہ{
      جاؤ unistr_error;
    }
  }
  z_برآمد[j] = 0;
  sqlite3_نتیجہ_text64(context, z_برآمد, j, sqlite3_free, SQLITE_UTF8_ZT);
  واپس;

unistr_error:
  sqlite3_free(z_برآمد);
  sqlite3_نتیجہ_error(context, "invalid Unicode escape", -1);
  واپس;
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
ساکن خالی quoteFunc(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]);
  sqlite3_Quote_قدر(&str,argv[0],SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(context)));
  sqlite3_نتیجہ_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
ساکن خالی unicodeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت غیرعلامتی حرف *z = sqlite3_قدر_متن(argv[0]);
  (خالی)argc;
  اگر( z && z[0] ) sqlite3_نتیجہ_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
ساکن خالی charFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  غیرعلامتی حرف *z, *z_برآمد;
  صحیح i;
  z_برآمد = z = sqlite3_malloc64( argc*4+1 );
  اگر( z==0 ){
    sqlite3_نتیجہ_error_nomem(context);
    واپس;
  }
  برائے(i=0; i<argc; i++){
    sqlite3_int64 x;
    غیرعلامتی c;
    x = sqlite3_قدر_int64(argv[i]);
    اگر( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (غیرعلامتی)(x & 0x1fffff);
    اگر( c<0x00080 ){
      *z_برآمد++ = (u8)(c&0xFF);
    }ورنہ اگر( c<0x00800 ){
      *z_برآمد++ = 0xC0 + (u8)((c>>6)&0x1F);
      *z_برآمد++ = 0x80 + (u8)(c & 0x3F);
    }ورنہ اگر( c<0x10000 ){
      *z_برآمد++ = 0xE0 + (u8)((c>>12)&0x0F);
      *z_برآمد++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_برآمد++ = 0x80 + (u8)(c & 0x3F);
    }ورنہ{
      *z_برآمد++ = 0xF0 + (u8)((c>>18) & 0x07);
      *z_برآمد++ = 0x80 + (u8)((c>>12) & 0x3F);
      *z_برآمد++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_برآمد++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *z_برآمد = 0;
  sqlite3_نتیجہ_text64(context, (حرف*)z, z_برآمد-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
ساکن خالی hexFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح i, n;
  ثابت غیرعلامتی حرف *pBlob;
  حرف *zHex, *z;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  pBlob = sqlite3_قدر_blob(argv[0]);
  n = sqlite3_قدر_بائٹیں(argv[0]);
  أكد( pBlob==sqlite3_قدر_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  اگر( zHex ){
    برائے(i=0; i<n; i++, pBlob++){
      غیرعلامتی حرف c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_نتیجہ_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
ساکن صحیح str_شامل_ہے_Char(ثابت u8 *zStr, صحیح nStr, u32 ch){
  ثابت u8 *z_انتہا = &zStr[nStr];
  ثابت u8 *z = zStr;
  جب( z<z_انتہا ){
    u32 tst = Utf8Read(z);
    اگر( tst==ch ) واپس 1;
  }
  واپس 0;
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
ساکن خالی unhexFunc(
  sqlite3_context *pCtx,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت u8 *zPass = (ثابت u8*)"";
  صحیح nPass = 0;
  ثابت u8 *zHex = sqlite3_قدر_متن(argv[0]);
  صحیح nHex = sqlite3_قدر_بائٹیں(argv[0]);
#اگر_تعریف_شدہ SQLITE_DEBUG
  ثابت u8 *z_انتہا = zHex ? &zHex[nHex] : 0;
#اگر_ختم
  u8 *pBlob = 0;
  u8 *p = 0;

  أكد( argc==1 || argc==2 );
  اگر( argc==2 ){
    zPass = sqlite3_قدر_متن(argv[1]);
    nPass = sqlite3_قدر_بائٹیں(argv[1]);
  }
  اگر( !zHex || !zPass ) واپس;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  اگر( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    جب( (c = *zHex)!=0x00 ){
      جب( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        أكد( zHex<=z_انتہا );
        اگر( !str_شامل_ہے_Char(zPass, nPass, ch) ) جاؤ unhex_null;
        c = *zHex;
        اگر( c==0x00 ) جاؤ unhex_مکمل;
      }
      zHex++;
      أكد( *z_انتہا==0x00 );
      أكد( zHex<=z_انتہا );
      d = *(zHex++);
      اگر( !sqlite3Isxdigit(d) ) جاؤ unhex_null;
      *(p++) = (sqlite3_Hex_مقصد_Int(c)<<4) | sqlite3_Hex_مقصد_Int(d);
    }
  }

 unhex_مکمل:
  sqlite3_نتیجہ_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  واپس;

 unhex_null:
  sqlite3_free(pBlob);
  واپس;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
ساکن خالی zeroblobFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  i64 n;
  صحیح rc;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  n = sqlite3_قدر_int64(argv[0]);
  اگر( n<0 ) n = 0;
  rc = sqlite3_نتیجہ_zeroblob64(context, n); /* IMP: R-00293-64994 */
  اگر( rc ){
    sqlite3_نتیجہ_error_رمز(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
ساکن خالی replaceFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت غیرعلامتی حرف *zStr;        /* The input string A */
  ثابت غیرعلامتی حرف *z_نمونہ;    /* The pattern string B */
  ثابت غیرعلامتی حرف *zRep;        /* The replacement string C */
  غیرعلامتی حرف *z_برآمد;              /* The output */
  صحیح nStr;                /* Size of zStr */
  صحیح n_نمونہ;            /* Size of zPattern */
  صحیح nRep;                /* Size of zRep */
  i64 n_برآمد;                /* Maximum size of zOut */
  صحیح loop_حد_بندی;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  غیرعلامتی cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  أكد( argc==3 );
  غیر_مستعمل_PARAMETER(argc);
  zStr = sqlite3_قدر_متن(argv[0]);
  اگر( zStr==0 ) واپس;
  nStr = sqlite3_قدر_بائٹیں(argv[0]);
  أكد( zStr==sqlite3_قدر_متن(argv[0]) );  /* No encoding change */
  z_نمونہ = sqlite3_قدر_متن(argv[1]);
  اگر( z_نمونہ==0 ){
    أكد( sqlite3_قدر_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    واپس;
  }
  اگر( z_نمونہ[0]==0 ){
    أكد( sqlite3_قدر_type(argv[1])!=SQLITE_NULL );
    sqlite3_نتیجہ_متن(context, (ثابت حرف*)zStr, nStr, SQLITE_TRANSIENT);
    واپس;
  }
  n_نمونہ = sqlite3_قدر_بائٹیں(argv[1]);
  أكد( z_نمونہ==sqlite3_قدر_متن(argv[1]) );  /* No encoding change */
  zRep = sqlite3_قدر_متن(argv[2]);
  اگر( zRep==0 ) واپس;
  nRep = sqlite3_قدر_بائٹیں(argv[2]);
  أكد( zRep==sqlite3_قدر_متن(argv[2]) );
  n_برآمد = nStr + 1;
  أكد( n_برآمد<SQLITE_MAX_لمبائی );
  z_برآمد = contextMalloc(context, n_برآمد);
  اگر( z_برآمد==0 ){
    واپس;
  }
  loop_حد_بندی = nStr - n_نمونہ; 
  cntExpand = 0;
  برائے(i=j=0; i<=loop_حد_بندی; i++){
    اگر( zStr[i]!=z_نمونہ[0] || memcmp(&zStr[i], z_نمونہ, n_نمونہ) ){
      z_برآمد[j++] = zStr[i];
    }ورنہ{
      اگر( nRep>n_نمونہ ){
        n_برآمد += nRep - n_نمونہ;
        testcase( n_برآمد-1==db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] );
        testcase( n_برآمد-2==db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] );
        اگر( n_برآمد-1>db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] ){
          sqlite3_نتیجہ_error_toobig(context);
          sqlite3_free(z_برآمد);
          واپس;
        }
        cntExpand++;
        اگر( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = z_برآمد;
          z_برآمد = sqlite3Realloc(z_برآمد, (صحیح)n_برآمد + (n_برآمد - nStr - 1));
          اگر( z_برآمد==0 ){
            sqlite3_نتیجہ_error_nomem(context);
            sqlite3_free(zOld);
            واپس;
          }
        }
      }
      حافظہ_نقل(&z_برآمد[j], zRep, nRep);
      j += nRep;
      i += n_نمونہ-1;
    }
  }
  أكد( j+nStr-i+1<=n_برآمد );
  حافظہ_نقل(&z_برآمد[j], &zStr[i], nStr-i);
  j += nStr - i;
  أكد( j<=n_برآمد );
  z_برآمد[j] = 0;
  sqlite3_نتیجہ_متن(context, (حرف*)z_برآمد, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
ساکن خالی trimFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت غیرعلامتی حرف *z_اندر;         /* Input string */
  ثابت غیرعلامتی حرف *z_Char_سیٹ;    /* Set of characters to trim */
  غیرعلامتی صحیح n_اندر;                 /* Number of bytes in input */
  صحیح flags;                        /* 1: trimleft  2: trimright  3: trim */
  صحیح i;                            /* Loop counter */
  غیرعلامتی صحیح *aLen = 0;           /* Length of each character in zCharSet */
  غیرعلامتی حرف **azChar = 0;       /* Individual characters in zCharSet */
  صحیح nChar;                        /* Number of characters in zCharSet */

  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ){
    واپس;
  }
  z_اندر = sqlite3_قدر_متن(argv[0]);
  اگر( z_اندر==0 ) واپس;
  n_اندر = (غیرعلامتی)sqlite3_قدر_بائٹیں(argv[0]);
  أكد( z_اندر==sqlite3_قدر_متن(argv[0]) );
  اگر( argc==1 ){
    ساکن ثابت غیرعلامتی lenOne[] = { 1 };
    ساکن غیرعلامتی حرف * ثابت azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (غیرعلامتی*)lenOne;
    azChar = (غیرعلامتی حرف **)azOne;
    z_Char_سیٹ = 0;
  }ورنہ اگر( (z_Char_سیٹ = sqlite3_قدر_متن(argv[1]))==0 ){
    واپس;
  }ورنہ{
    ثابت غیرعلامتی حرف *z;
    برائے(z=z_Char_سیٹ, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    اگر( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(حجم(حرف*)+حجم(غیرعلامتی)));
      اگر( azChar==0 ){
        واپس;
      }
      aLen = (غیرعلامتی*)&azChar[nChar];
      برائے(z=z_Char_سیٹ, nChar=0; *z; nChar++){
        azChar[nChar] = (غیرعلامتی حرف *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (غیرعلامتی)(z - azChar[nChar]);
      }
    }
  }
  اگر( nChar>0 ){
    flags = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(context));
    اگر( flags & 1 ){
      جب( n_اندر>0 ){
        غیرعلامتی صحیح len = 0;
        برائے(i=0; i<nChar; i++){
          len = aLen[i];
          اگر( len<=n_اندر && memcmp(z_اندر, azChar[i], len)==0 ) توڑو;
        }
        اگر( i>=nChar ) توڑو;
        z_اندر += len;
        n_اندر -= len;
      }
    }
    اگر( flags & 2 ){
      جب( n_اندر>0 ){
        غیرعلامتی صحیح len = 0;
        برائے(i=0; i<nChar; i++){
          len = aLen[i];
          اگر( len<=n_اندر && memcmp(&z_اندر[n_اندر-len],azChar[i],len)==0 ) توڑو;
        }
        اگر( i>=nChar ) توڑو;
        n_اندر -= len;
      }
    }
    اگر( z_Char_سیٹ ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_نتیجہ_متن(context, (حرف*)z_اندر, n_اندر, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
ساکن خالی concatFuncCore(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv,
  صحیح nSep,
  ثابت حرف *zSep
){
  i64 j, n = 0;
  صحیح i;
  صحیح bNotNull = 0;   /* True after at least NOT NULL argument seen */
  حرف *z;
  برائے(i=0; i<argc; i++){
    n += sqlite3_قدر_بائٹیں(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  اگر( z==0 ){
    sqlite3_نتیجہ_error_nomem(context);
    واپس;
  }
  j = 0;
  برائے(i=0; i<argc; i++){
    اگر( sqlite3_قدر_type(argv[i])!=SQLITE_NULL ){
      صحیح k = sqlite3_قدر_بائٹیں(argv[i]);
      ثابت حرف *v = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
      اگر( v!=0 ){
        اگر( bNotNull && nSep>0 ){
          حافظہ_نقل(&z[j], zSep, nSep);
          j += nSep;
        }
        حافظہ_نقل(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  أكد( j<=n );
  sqlite3_نتیجہ_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
ساکن خالی concatFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
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
ساکن خالی concatwsFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح nSep = sqlite3_قدر_بائٹیں(argv[0]);
  ثابت حرف *zSep = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  اگر( zSep==0 ) واپس;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
ساکن خالی unknownFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  /* no-op */
  (خالی)context;
  (خالی)argc;
  (خالی)argv;
}
#اگر_ختم /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#اگر_تعریف_شدہ SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
ساکن خالی soundexFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  حرف z_نتیجہ[8];
  ثابت u8 *z_اندر;
  صحیح i, j;
  ساکن ثابت غیرعلامتی حرف i_رمز[] = {
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
  z_اندر = (u8*)sqlite3_قدر_متن(argv[0]);
  اگر( z_اندر==0 ) z_اندر = (u8*)"";
  برائے(i=0; z_اندر[i] && !sqlite3Isalpha(z_اندر[i]); i++){}
  اگر( z_اندر[i] ){
    u8 prevcode = i_رمز[z_اندر[i]&0x7f];
    z_نتیجہ[0] = sqlite3Toupper(z_اندر[i]);
    برائے(j=1; j<4 && z_اندر[i]; i++){
      صحیح رمز = i_رمز[z_اندر[i]&0x7f];
      اگر( رمز>0 ){
        اگر( رمز!=prevcode ){
          prevcode = رمز;
          z_نتیجہ[j++] = رمز + '0';
        }
      }ورنہ{
        prevcode = 0;
      }
    }
    جب( j<4 ){
      z_نتیجہ[j++] = '0';
    }
    z_نتیجہ[j] = 0;
    sqlite3_نتیجہ_متن(context, z_نتیجہ, 4, SQLITE_TRANSIENT);
  }ورنہ{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_نتیجہ_متن(context, "?000", 4, SQLITE_STATIC);
  }
}
#اگر_ختم /* SQLITE_SOUNDEX */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
ساکن خالی oomFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#اگر_ختم /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
ساکن خالی بھرو_Ext(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  ثابت حرف *zFile = (ثابت حرف *)sqlite3_قدر_متن(argv[0]);
  ثابت حرف *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف *z_Err_پیغام = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  اگر( (db->flags & SQLITE_بھرو_Ext_Func)==0 ){
    sqlite3_نتیجہ_error(context, "not authorized", -1);
    واپس;
  }

  اگر( argc==2 ){
    zProc = (ثابت حرف *)sqlite3_قدر_متن(argv[1]);
  }ورنہ{
    zProc = 0;
  }
  اگر( zFile && sqlite3_بھرو_extension(db, zFile, zProc, &z_Err_پیغام) ){
    sqlite3_نتیجہ_error(context, z_Err_پیغام, -1);
    sqlite3_free(z_Err_پیغام);
  }
}
#اگر_ختم


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
قسمتعریف ساخت مجموع_Ctx مجموع_Ctx;
ساخت مجموع_Ctx {
  دوہرا r_مجموع;      /* Running sum as as a double */
  دوہرا rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 i_مجموع;         /* Running sum as a signed integer */
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
ساکن خالی kahan_Babuska_Neumaier_قدم(
  متلون مجموع_Ctx *p_مجموع,
  متلون دوہرا r
){
  متلون دوہرا s = p_مجموع->r_مجموع;
  متلون دوہرا t = s + r;
  اگر( fabs(s) > fabs(r) ){
    p_مجموع->rErr += (s - t) + r;
  }ورنہ{
    p_مجموع->rErr += (r - t) + s;
  }
  p_مجموع->r_مجموع = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
ساکن خالی kahan_Babuska_Neumaier_قدم_Int64(متلون مجموع_Ctx *p_مجموع, i64 iVal){
  اگر( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahan_Babuska_Neumaier_قدم(p_مجموع, iBig);
    kahan_Babuska_Neumaier_قدم(p_مجموع, iSm);
  }ورنہ{
    kahan_Babuska_Neumaier_قدم(p_مجموع, (دوہرا)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
ساکن خالی kahanBabuskaNeumaierInit(
  متلون مجموع_Ctx *p,
  i64 iVal
){
  اگر( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_مجموع = (دوہرا)(iVal - iSm);
    p->rErr = (دوہرا)iSm;
  }ورنہ{
    p->r_مجموع = (دوہرا)iVal;
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
ساکن خالی مجموع_قدم(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  مجموع_Ctx *p;
  صحیح type;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, حجم(*p));
  type = sqlite3_قدر_numeric_type(argv[0]);
  اگر( p && type!=SQLITE_NULL ){
    p->cnt++;
    اگر( p->approx==0 ){
      اگر( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_مجموع);
        p->approx = 1;
        kahan_Babuska_Neumaier_قدم(p, sqlite3_قدر_double(argv[0]));
      }ورنہ{
        i64 x = p->i_مجموع;
        اگر( sqlite3_جوڑو_Int64(&x, sqlite3_قدر_int64(argv[0]))==0 ){
          p->i_مجموع = x;
        }ورنہ{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_مجموع);
          p->approx = 1;
          kahan_Babuska_Neumaier_قدم_Int64(p, sqlite3_قدر_int64(argv[0]));
        }
      }
    }ورنہ{
      اگر( type==SQLITE_INTEGER ){
        kahan_Babuska_Neumaier_قدم_Int64(p, sqlite3_قدر_int64(argv[0]));
      }ورنہ{
        p->ovrfl = 0;
        kahan_Babuska_Neumaier_قدم(p, sqlite3_قدر_double(argv[0]));
      }
    }
  }
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
ساکن خالی مجموع_Inverse(sqlite3_context *context, صحیح argc, sqlite3_قدر**argv){
  مجموع_Ctx *p;
  صحیح type;
  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, حجم(*p));
  type = sqlite3_قدر_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  اگر( ALWAYS(p) && type!=SQLITE_NULL ){
    أكد( p->cnt>0 );
    p->cnt--;
    اگر( !p->approx ){
      i64 x = p->i_مجموع;
      اگر( sqlite3SubInt64(&x, sqlite3_قدر_int64(argv[0]))==0 ){
        p->i_مجموع = x;
        واپس;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_مجموع);
    }
    اگر( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_قدر_int64(argv[0]);
      اگر( iVal!=سب_سے_چھوٹا_INT64 ){
        kahan_Babuska_Neumaier_قدم_Int64(p, -iVal);
      }ورنہ{
        kahan_Babuska_Neumaier_قدم_Int64(p, سب_سے_بڑا_INT64);
        kahan_Babuska_Neumaier_قدم_Int64(p, 1);
      }       
    }ورنہ{
      kahan_Babuska_Neumaier_قدم(p, -sqlite3_قدر_double(argv[0]));
    }
  }
}
#ورنہ
# define مجموع_Inverse 0
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
ساکن خالی مجموع_Finalize(sqlite3_context *context){
  مجموع_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  اگر( p && p->cnt>0 ){
    اگر( p->approx ){
      اگر( p->ovrfl ){
        sqlite3_نتیجہ_error(context,"integer overflow",-1);
      }ورنہ اگر( !sqlite3_Is_حد_سے_تجاوز(p->rErr) ){
        sqlite3_نتیجہ_double(context, p->r_مجموع+p->rErr);
      }ورنہ{
        sqlite3_نتیجہ_double(context, p->r_مجموع);
      }
    }ورنہ{
      sqlite3_نتیجہ_int64(context, p->i_مجموع);
    }
  }
}
ساکن خالی avgFinalize(sqlite3_context *context){
  مجموع_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  اگر( p && p->cnt>0 ){
    دوہرا r;
    اگر( p->approx ){
      r = p->r_مجموع;
      اگر( !sqlite3_Is_حد_سے_تجاوز(p->rErr) ) r += p->rErr;
    }ورنہ{
      r = (دوہرا)(p->i_مجموع);
    }
    sqlite3_نتیجہ_double(context, r/(دوہرا)p->cnt);
  }
}
ساکن خالی کل_Finalize(sqlite3_context *context){
  مجموع_Ctx *p;
  دوہرا r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  اگر( p ){
    اگر( p->approx ){
      r = p->r_مجموع;
      اگر( !sqlite3_Is_حد_سے_تجاوز(p->rErr) ) r += p->rErr;
    }ورنہ{
      r = (دوہرا)(p->i_مجموع);
    }
  }
  sqlite3_نتیجہ_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
قسمتعریف ساخت گنتی_Ctx گنتی_Ctx;
ساخت گنتی_Ctx {
  i64 n;
#اگر_تعریف_شدہ SQLITE_DEBUG
  صحیح bInverse;                   /* True if xInverse() ever called */
#اگر_ختم
};

/*
** Routines to implement the count() aggregate function.
*/
ساکن خالی گنتی_قدم(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  گنتی_Ctx *p;
  p = sqlite3_aggregate_context(context, حجم(*p));
  اگر( (argc==0 || SQLITE_NULL!=sqlite3_قدر_type(argv[0])) && p ){
    p->n++;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  أكد( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_گنتی(context) );
#اگر_ختم
}  
ساکن خالی گنتی_Finalize(sqlite3_context *context){
  گنتی_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_نتیجہ_int64(context, p ? p->n : 0);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
ساکن خالی گنتی_Inverse(sqlite3_context *ctx, صحیح argc, sqlite3_قدر **argv){
  گنتی_Ctx *p;
  p = sqlite3_aggregate_context(ctx, حجم(*p));
  /* p is always non-NULL since countStep() will have been called first */
  اگر( (argc==0 || SQLITE_NULL!=sqlite3_قدر_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#اگر_تعریف_شدہ SQLITE_DEBUG
    p->bInverse = 1;
#اگر_ختم
  }
}  
#ورنہ
# define گنتی_Inverse 0
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
ساکن خالی minmax_قدم(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *p_بہترین;
  غیر_مستعمل_PARAMETER(Not_مستعمل);

  p_بہترین = (Mem *)sqlite3_aggregate_context(context, حجم(*p_بہترین));
  اگر( !p_بہترین ) واپس;

  اگر( sqlite3_قدر_type(pArg)==SQLITE_NULL ){
    اگر( p_بہترین->flags ) sqlite3_Skip_مجمع_بھرو(context);
  }ورنہ اگر( p_بہترین->flags ){
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
    max = sqlite3_user_ڈیٹا(context)!=0;
    cmp = sqlite3MemCompare(p_بہترین, pArg, pColl);
    اگر( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(p_بہترین, pArg);
    }ورنہ{
      sqlite3_Skip_مجمع_بھرو(context);
    }
  }ورنہ{
    p_بہترین->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(p_بہترین, pArg);
  }
}
ساکن خالی min_Max_قدر_Finalize(sqlite3_context *context, صحیح b_قدر){
  sqlite3_قدر *pRes;
  pRes = (sqlite3_قدر *)sqlite3_aggregate_context(context, 0);
  اگر( pRes ){
    اگر( pRes->flags ){
      sqlite3_نتیجہ_قدر(context, pRes);
    }
    اگر( b_قدر==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
ساکن خالی min_Max_قدر(sqlite3_context *context){
  min_Max_قدر_Finalize(context, 1);
}
#ورنہ
# define min_Max_قدر 0
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
ساکن خالی minMaxFinalize(sqlite3_context *context){
  min_Max_قدر_Finalize(context, 0);
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
قسمتعریف ساخت {
  StrAccum str;          /* The accumulated concatenation */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  صحیح nAccum;            /* Number of strings presently concatenated */
  صحیح n_پہلا_Sep_لمبائی;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  صحیح *pnSepLengths;
#اگر_ختم
} گروہ_Concat_Ctx;

ساکن خالی گروہ_Concat_قدم(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  ثابت حرف *zVal;
  گروہ_Concat_Ctx *pGCC;
  ثابت حرف *zSep;
  صحیح nVal, nSep;
  أكد( argc==1 || argc==2 );
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) واپس;
  pGCC = (گروہ_Concat_Ctx*)sqlite3_aggregate_context(context, حجم(*pGCC));
  اگر( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    صحیح پہلا_رکن = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->a_حد_بندی[SQLITE_حد_بندی_لمبائی];
    اگر( argc==1 ){
      اگر( !پہلا_رکن ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      ورنہ{
        pGCC->n_پہلا_Sep_لمبائی = 1;
      }
#اگر_ختم
    }ورنہ اگر( !پہلا_رکن ){
      zSep = (حرف*)sqlite3_قدر_متن(argv[1]);
      nSep = sqlite3_قدر_بائٹیں(argv[1]);
      اگر( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      ورنہ{
        nSep = 0;
      }
      اگر( nSep != pGCC->n_پہلا_Sep_لمبائی || pGCC->pnSepLengths != 0 ){
        صحیح *pnsl = pGCC->pnSepLengths;
        اگر( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (صحیح*)sqlite3_malloc64((pGCC->nAccum+1) * حجم(صحیح));
          اگر( pnsl!=0 ){
            صحیح i = 0, nA = pGCC->nAccum-1;
            جب( i<nA ) pnsl[i++] = pGCC->n_پہلا_Sep_لمبائی;
          }
        }ورنہ{
          pnsl = (صحیح*)sqlite3_realloc64(pnsl, pGCC->nAccum * حجم(صحیح));
        }
        اگر( pnsl!=0 ){
          اگر( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }ورنہ{
          sqlite3_Str_Accum_سیٹ_Error(&pGCC->str, SQLITE_NOMEM);
        }
      }
#اگر_ختم
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    ورنہ{
      pGCC->n_پہلا_Sep_لمبائی = sqlite3_قدر_بائٹیں(argv[1]);
    }
    pGCC->nAccum += 1;
#اگر_ختم
    zVal = (حرف*)sqlite3_قدر_متن(argv[0]);
    nVal = sqlite3_قدر_بائٹیں(argv[0]);
    اگر( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
ساکن خالی گروہ_Concat_Inverse(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  گروہ_Concat_Ctx *pGCC;
  أكد( argc==1 || argc==2 );
  (خالی)argc;  /* Suppress unused parameter warning */
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) واپس;
  pGCC = (گروہ_Concat_Ctx*)sqlite3_aggregate_context(context, حجم(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  اگر( ALWAYS(pGCC) ){
    صحیح nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (خالی)sqlite3_قدر_متن(argv[0]);
    nVS = sqlite3_قدر_بائٹیں(argv[0]);
    pGCC->nAccum -= 1;
    اگر( pGCC->pnSepLengths!=0 ){
      أكد(pGCC->nAccum >= 0);
      اگر( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*حجم(صحیح));
      }
    }ورنہ{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_پہلا_Sep_لمبائی;
    }
    اگر( nVS>=(صحیح)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }ورنہ{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.z_متن, &pGCC->str.z_متن[nVS], pGCC->str.nChar);
    }
    اگر( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#ورنہ
# define گروہ_Concat_Inverse 0
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
ساکن خالی گروہ_Concat_Finalize(sqlite3_context *context){
  گروہ_Concat_Ctx *pGCC
    = (گروہ_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  اگر( pGCC ){
    sqlite3_نتیجہ_str(context, &pGCC->str, SQLITE_XFER);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#اگر_ختم
  }
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
ساکن خالی گروہ_Concat_قدر(sqlite3_context *context){
  گروہ_Concat_Ctx *pGCC
    = (گروہ_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  اگر( pGCC && pGCC->nAccum>0 ){
    sqlite3_نتیجہ_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#ورنہ
# define گروہ_Concat_قدر 0
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
خالی sqlite3_رجسٹر_فی_Connection_Builtin_Functions(sqlite3 *db){
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
خالی sqlite3_رجسٹر_Like_Functions(sqlite3 *db, صحیح caseSensitive){
  FuncDef *pDef;
  ساخت compareInfo *pInfo;
  صحیح flags;
  صحیح nArg;
  اگر( caseSensitive ){
    pInfo = (ساخت compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }ورنہ{
    pInfo = (ساخت compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  برائے(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3_ڈھونڈو_Function(db, "like", nArg, SQLITE_UTF8, 0);
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
صحیح sqlite3IsLikeFunction(sqlite3 *db, اظہاریہ *p_اظہاریہ, صحیح *pIsNocase, حرف *aWc){
  FuncDef *pDef;
  صحیح n_اظہاریہ;
  أكد( p_اظہاریہ!=0 );
  أكد( p_اظہاریہ->op==TK_FUNCTION );
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  اگر( !p_اظہاریہ->x.pList ){
    واپس 0;
  }
  n_اظہاریہ = p_اظہاریہ->x.pList->n_اظہاریہ;
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
  pDef = sqlite3_ڈھونڈو_Function(db, p_اظہاریہ->u.zToken, n_اظہاریہ, SQLITE_UTF8, 0);
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  اگر( pDef==0 ) واپس 0;
#اگر_ختم
  اگر( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    واپس 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  حافظہ_نقل(aWc, pDef->p_User_ڈیٹا, 3);
  أكد( (حرف*)&likeInfoAlt == (حرف*)&likeInfoAlt.مطابقت_All );
  أكد( &((حرف*)&likeInfoAlt)[1] == (حرف*)&likeInfoAlt.مطابقت_One );
  أكد( &((حرف*)&likeInfoAlt)[2] == (حرف*)&likeInfoAlt.مطابقت_سیٹ );

  اگر( n_اظہاریہ<3 ){
    aWc[3] = 0;
  }ورنہ{
    اظہاریہ *pEscape = p_اظہاریہ->x.pList->a[2].p_اظہاریہ;
    حرف *zEscape;
    اگر( pEscape->op!=TK_STRING ) واپس 0;
    أكد( !اظہاریہ_Has_Property(pEscape, EP_Int_قدر) );
    zEscape = pEscape->u.zToken;
    اگر( zEscape[0]==0 || zEscape[1]!=0 ) واپس 0;
    اگر( zEscape[0]==aWc[0] ) واپس 0;
    اگر( zEscape[0]==aWc[1] ) واپس 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  واپس 1;
}

/* Mathematical Constants */
#اگر_غیر_تعریف_شدہ M_PI
# define M_PI   3.141592653589793238462643383279502884
#اگر_ختم
#اگر_غیر_تعریف_شدہ M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#اگر_ختم
#اگر_غیر_تعریف_شدہ M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#اگر_ختم


/* Extra math functions that require linking with -lm
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_MATH_FUNCTIONS
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
ساکن خالی ceilingFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  أكد( argc==1 );
  انتخاب( sqlite3_قدر_numeric_type(argv[0]) ){
    حالت SQLITE_INTEGER: {
       sqlite3_نتیجہ_int64(context, sqlite3_قدر_int64(argv[0]));
       توڑو;
    }
    حالت SQLITE_FLOAT: {
       دوہرا (*x)(دوہرا) = (دوہرا(*)(دوہرا))sqlite3_user_ڈیٹا(context);
       sqlite3_نتیجہ_double(context, x(sqlite3_قدر_double(argv[0])));
       توڑو;
    }
    طےشدہ: {
       توڑو;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
ساکن دوہرا xCeil(دوہرا x){ واپس ceil(x); }
ساکن دوہرا xFloor(دوہرا x){ واپس floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#اگر defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*لوگارتھم(X))
#اگر_ختم
#اگر defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*لوگارتھم(X))
#اگر_ختم


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
ساکن خالی logFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  دوہرا x, b, ans;
  أكد( argc==1 || argc==2 );
  انتخاب( sqlite3_قدر_numeric_type(argv[0]) ){
    حالت SQLITE_INTEGER:
    حالت SQLITE_FLOAT:
      x = sqlite3_قدر_double(argv[0]);
      اگر( x<=0.0 ) واپس;
      توڑو;
    طےشدہ:
      واپس;
  }
  اگر( argc==2 ){
    انتخاب( sqlite3_قدر_numeric_type(argv[1]) ){
      حالت SQLITE_INTEGER:
      حالت SQLITE_FLOAT:
        b = لوگارتھم(x);
        اگر( b<=0.0 ) واپس;
        x = sqlite3_قدر_double(argv[1]);
        اگر( x<=0.0 ) واپس;
        توڑو;
     طےشدہ:
        واپس;
    }
    ans = لوگارتھم(x)/b;
  }ورنہ{
    انتخاب( SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(context)) ){
      حالت 1:
        ans = log10(x);
        توڑو;
      حالت 2:
        ans = log2(x);
        توڑو;
      طےشدہ:
        ans = لوگارتھم(x);
        توڑو;
    }
  }
  sqlite3_نتیجہ_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
ساکن دوہرا deg_مقصد_Rad(دوہرا x){ واپس x*(M_PI/180.0); }
ساکن دوہرا rad_مقصد_Deg(دوہرا x){ واپس x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
ساکن خالی math1Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح type0;
  دوہرا v0, ans;
  دوہرا (*x)(دوہرا);
  أكد( argc==1 );
  type0 = sqlite3_قدر_numeric_type(argv[0]);
  اگر( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) واپس;
  v0 = sqlite3_قدر_double(argv[0]);
  x = (دوہرا(*)(دوہرا))sqlite3_user_ڈیٹا(context);
  ans = x(v0);
  sqlite3_نتیجہ_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
ساکن خالی math2Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح type0, type1;
  دوہرا v0, v1, ans;
  دوہرا (*x)(دوہرا,دوہرا);
  أكد( argc==2 );
  type0 = sqlite3_قدر_numeric_type(argv[0]);
  اگر( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) واپس;
  type1 = sqlite3_قدر_numeric_type(argv[1]);
  اگر( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) واپس;
  v0 = sqlite3_قدر_double(argv[0]);
  v1 = sqlite3_قدر_double(argv[1]);
  x = (دوہرا(*)(دوہرا,دوہرا))sqlite3_user_ڈیٹا(context);
  ans = x(v0, v1);
  sqlite3_نتیجہ_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
ساکن خالی piFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  أكد( argc==0 );
  (خالی)argv;
  sqlite3_نتیجہ_double(context, M_PI);
}

#اگر_ختم /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
ساکن خالی signFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح type0;
  دوہرا x;
  غیر_مستعمل_PARAMETER(argc);
  أكد( argc==1 );
  type0 = sqlite3_قدر_numeric_type(argv[0]);
  اگر( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) واپس;
  x = sqlite3_قدر_double(argv[0]);
  sqlite3_نتیجہ_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
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
قسمتعریف ساخت Percentile Percentile;
ساخت Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 n_مستعمل;           /* Number of slots actually used in a[] */
  حرف bSorted;        /* True if a[] is already in sorted order */
  حرف bKeepSorted;    /* True if advantageous to keep a[] sorted */
  حرف bPctValid;      /* True if rPct is valid */
  دوہرا rPct;         /* Fraction.  0.0 to 1.0 */
  دوہرا *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
ساکن صحیح percentIsInfinity(دوہرا r){
  sqlite3_uint64 u;
  أكد( حجم(u)==حجم(r) );
  حافظہ_نقل(&u, &r, حجم(u));
  واپس ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
ساکن صحیح percent_Same_قدر(دوہرا a, دوہرا b){
  a -= b;
  واپس a>=-0.001 && a<=0.001;
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
ساکن i64 percent_ثنائی_تلاش(Percentile *p, دوہرا y, صحیح bExact){
  i64 i_پہلا = 0;                   /* First element of search range */
  i64 i_آخری = (i64)p->n_مستعمل - 1;    /* Last element of search range */
  جب( i_آخری>=i_پہلا ){
    i64 iMid = (i_پہلا+i_آخری)/2;
    دوہرا x = p->a[iMid];
    اگر( x<y ){
      i_پہلا = iMid + 1;
    }ورنہ اگر( x>y ){
      i_آخری = iMid - 1;
    }ورنہ{
      واپس iMid;
    }
  }
  اگر( bExact ) واپس -1;
  واپس i_پہلا;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
ساکن خالی percentError(sqlite3_context *pCtx, ثابت حرف *zFormat, ...){
  حرف *zMsg1;
  حرف *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_نتیجہ_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
ساکن خالی percent_قدم(sqlite3_context *pCtx, صحیح argc, sqlite3_قدر **argv){
  Percentile *p;
  دوہرا rPct;
  صحیح eType;
  دوہرا y;
  أكد( argc==2 || argc==1 );

  اگر( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }ورنہ{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    دوہرا mxFrac = (SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_قدر_numeric_type(argv[1]);
    rPct = sqlite3_قدر_double(argv[1])/mxFrac;
    اگر( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (دوہرا)mxFrac);
      واپس;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, حجم(*p));
  اگر( p==0 ) واپس;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  اگر( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }ورنہ اگر( !percent_Same_قدر(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    واپس;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_قدر_type(argv[0]);
  اگر( eType==SQLITE_NULL ) واپس;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  اگر( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    واپس;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_قدر_double(argv[0]);
  اگر( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    واپس;
  }

  /* Allocate and store the Y */
  اگر( p->n_مستعمل>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    دوہرا *a = sqlite3_realloc64(p->a, حجم(دوہرا)*n);
    اگر( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, حجم(*p));
      sqlite3_نتیجہ_error_nomem(pCtx);
      واپس;
    }
    p->nAlloc = n;
    p->a = a;
  }
  اگر( p->n_مستعمل==0 ){
    p->a[p->n_مستعمل++] = y;
    p->bSorted = 1;
  }ورنہ اگر( !p->bSorted || y>=p->a[p->n_مستعمل-1] ){
    p->a[p->n_مستعمل++] = y;
  }ورنہ اگر( p->bKeepSorted ){
    i64 i;
    i = percent_ثنائی_تلاش(p, y, 0);
    اگر( i<(صحیح)p->n_مستعمل ){
      memmove(&p->a[i+1], &p->a[i], (p->n_مستعمل-i)*حجم(p->a[0]));
    }
    p->a[i] = y;
    p->n_مستعمل++;
  }ورنہ{
    p->a[p->n_مستعمل++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#تعریف_کرو تبادلہ_DOUBLE(X,Y)  {دوہرا ttt=(X);(X)=(Y);(Y)=ttt;}

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
ساکن خالی percent_ترتیب_دو(
  دوہرا *a,                      /* Array to sort */
  غیرعلامتی صحیح n,                 /* Number of elements in array a[] */
  صحیح iReq                        /* Element caller cares about (or -ve) */
){
  صحیح iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  صحیح iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  صحیح i;         /* Loop counter */
  دوہرا r_محور; /* The pivot value */

  أكد( n>=2 );
  کرو{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    اگر( a[0]>a[n-1] ){
      تبادلہ_DOUBLE(a[0],a[n-1])
    }
    اگر( n==2 ) واپس;
    iGt = n-1;
    i = n/2;
    اگر( a[0]>a[i] ){
      تبادلہ_DOUBLE(a[0],a[i])
    }ورنہ اگر( a[i]>a[iGt] ){
      تبادلہ_DOUBLE(a[i],a[iGt])
    }
    اگر( n==3 ) واپس;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    r_محور = a[i];
    iLt = i = 1;
    کرو{
      اگر( a[i]<r_محور ){
        اگر( i>iLt ) تبادلہ_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }ورنہ اگر( a[i]>r_محور ){
        کرو{
          iGt--;
        }جب( iGt>i && a[iGt]>r_محور );
        تبادلہ_DOUBLE(a[i],a[iGt])
      }ورنہ{
        i++;
      }
    }جب( i<iGt );

    أكد( iLt>0 && iLt<iGt && (غیرعلامتی)iGt<n );
    testcase( iGt>iLt+1 );
    أكد( a[iLt]==r_محور );
    أكد( a[iLt-1]<=r_محور );
    أكد( a[iGt]>=r_محور );
    أكد( a[iLt+1]>=r_محور );

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
      }ورنہ{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }ورنہ{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      اگر( iLt>(صحیح)(n/2) ){
        اگر( n-iGt>=2 ) percent_ترتیب_دو(a+iGt, n-iGt, -1);
        n = iLt;
      }ورنہ{
        اگر( iLt>=2 ) percent_ترتیب_دو(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }جب( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
ساکن خالی percentInverse(sqlite3_context *pCtx,صحیح argc,sqlite3_قدر **argv){
  Percentile *p;
  صحیح eType;
  دوہرا y;
  i64 i;
  أكد( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, حجم(*p));
  أكد( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_قدر_type(argv[0]);
  اگر( eType==SQLITE_NULL ) واپس;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  اگر( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    واپس;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_قدر_double(argv[0]);
  اگر( percentIsInfinity(y) ){
    واپس;
  }
  اگر( p->bSorted==0 ){
    أكد( p->n_مستعمل>1 );
    percent_ترتیب_دو(p->a, p->n_مستعمل, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_ثنائی_تلاش(p, y, 1);
  اگر( i>=0 ){
    p->n_مستعمل--;
    اگر( i<(صحیح)p->n_مستعمل ){
      memmove(&p->a[i], &p->a[i+1], (p->n_مستعمل - i)*حجم(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
ساکن خالی percentCompute(sqlite3_context *pCtx, صحیح bIsFinal){
  Percentile *p;
  صحیح settings = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(pCtx))&1; /* Discrete? */
  غیرعلامتی i1, i2;
  دوہرا v1, v2;
  دوہرا ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  اگر( p==0 ) واپس;
  اگر( p->a==0 ) واپس;
  اگر( p->n_مستعمل ){
    ix = p->rPct*(p->n_مستعمل-1);
    i1 = (غیرعلامتی)ix;
    اگر( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      أكد( p->n_مستعمل>1 );
      percent_ترتیب_دو(p->a, p->n_مستعمل, (bIsFinal ? (صحیح)i1 : -1));
      p->bSorted = 1;
    }
    اگر( settings & 1 ){
      vx = p->a[i1];
    }ورنہ{
      i2 = ix==(دوہرا)i1 || i1==p->n_مستعمل-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_نتیجہ_double(pCtx, vx);
  }
  اگر( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, حجم(*p));
  }ورنہ{
    p->bKeepSorted = 1;
  }
}
ساکن خالی percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
ساکن خالی percent_قدر(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#اگر_ختم /* SQLITE_ENABLE_PERCENTILE */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
ساکن خالی filestatFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  pBtree = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
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
      sqlite3_نتیجہ_error_nomem(context);
    }ورنہ{
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
      sqlite3_نتیجہ_str(context, pStr, SQLITE_اختتام);
    }
    sqlite3BtreeLeave(pBtree);
  }ورنہ{
    sqlite3_نتیجہ_متن(context, "{}", 2, SQLITE_STATIC);
  }
}
#اگر_ختم /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
ساکن خالی walstatFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  pBtree = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
  اگر( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    أكد( pPager!=0 );
    pStr = sqlite3_str_new(db);
    اگر( sqlite3_str_errcode(pStr) ){
      sqlite3_نتیجہ_error_nomem(context);
    }ورنہ{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_نتیجہ_str(context, pStr, SQLITE_اختتام);
    }
    sqlite3BtreeLeave(pBtree);
  }ورنہ{
    sqlite3_نتیجہ_متن(context, "{}", 2, SQLITE_STATIC);
  }
}
#اگر_ختم /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن خالی fpdecodeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  FpDecode s;
  دوہرا x;
  صحیح y, z;
  حرف zBuf[100];
  غیر_مستعمل_PARAMETER(argc);
  أكد( argc==3 );
  x = sqlite3_قدر_double(argv[0]);
  y = sqlite3_قدر_int(argv[1]);
  z = sqlite3_قدر_int(argv[2]);
  اگر( z<=0 ) z = 1;
  اگر( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  اگر( s.isSpecial==2 ){
    sqlite3_snprintf(حجم(zBuf), zBuf, "NaN");
  }ورنہ{
    sqlite3_snprintf(حجم(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_نتیجہ_متن(context, zBuf, -1, SQLITE_TRANSIENT);
}
#اگر_ختم /* SQLITE_DEBUG */

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن خالی parseuriFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3_str *p_نتیجہ;
  ثابت حرف *zVfs;
  ثابت حرف *zUri;
  غیرعلامتی صحیح flgs;
  صحیح rc;
  sqlite3_vfs *pVfs = 0;
  حرف *zFile = 0;
  حرف *zErr = 0;

  اگر( argc<2 ) واپس;
  pVfs = sqlite3_vfs_ڈھونڈو(0);
  أكد( pVfs );
  zVfs = pVfs->zName;
  zUri = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  اگر( zUri==0 ) واپس;
  flgs = (غیرعلامتی صحیح)sqlite3_قدر_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_نتیجہ = sqlite3_str_new(0);
  اگر( !sqlite3_str_errcode(p_نتیجہ) ){
    صحیح i;
    sqlite3_str_appendf(p_نتیجہ, "rc=%d", rc);
    sqlite3_str_appendf(p_نتیجہ, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_نتیجہ, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_نتیجہ, ", err=%Q", zErr);
    sqlite3_str_appendf(p_نتیجہ, ", file=%Q", zFile);
    اگر( zFile ){
      ثابت حرف *z = zFile;
      z += sqlite3Strlen30(z)+1;
      جب( z[0] ){
        sqlite3_str_appendf(p_نتیجہ, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      برائے(i=2; i<argc; i++){
        ثابت حرف *zArg;
        اگر( sqlite3_قدر_type(argv[i])==SQLITE_INTEGER ){
          صحیح k = sqlite3_قدر_int(argv[i]);
          sqlite3_str_appendf(p_نتیجہ, ", '%d:%q'",k,sqlite3_uri_کلید(zFile, k));
        }ورنہ اگر( (zArg = (ثابت حرف*)sqlite3_قدر_متن(argv[i]))!=0 ){
          sqlite3_str_appendf(p_نتیجہ, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }ورنہ{
          sqlite3_str_appendf(p_نتیجہ, ", NULL");
        }
      }
    }
  }
  sqlite3_نتیجہ_str(ctx, p_نتیجہ, SQLITE_اختتام);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#اگر_ختم /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
خالی sqlite3_رجسٹر_Builtin_Functions(خالی){
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
  ساکن FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#اگر !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(مستلزم_nonnull_صف, 2, INLINEFUNC_مستلزم_nonnull_صف, 0),
    TEST_FUNC(اظہاریہ_compare,        2, INLINEFUNC_اظہاریہ_compare,        0),
    TEST_FUNC(اظہاریہ_مستلزم_اظہاریہ,   2, INLINEFUNC_اظہاریہ_مستلزم_اظہاریہ,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#اگر_ختم /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#اگر_تعریف_شدہ SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
    SFUNCTION(بھرو_extension,    1, 0, 0, بھرو_Ext          ),
    SFUNCTION(بھرو_extension,    2, 0, 0, بھرو_Ext          ),
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_مستعمل,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#اگر_ختم /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#اگر_تعریف_شدہ SQLITE_ENABLE_انحراف_SQL_FUNC
    INLINE_FUNC(sqlite_انحراف,   1, INLINEFUNC_sqlite_انحراف, 0 ),
#اگر_ختم
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#اگر_ختم
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmax_قدم, minMaxFinalize, min_Max_قدر, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmax_قدم, minMaxFinalize, min_Max_قدر, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(لمبائی,            1, 0, 0, لمبائی_Func,  SQLITE_FUNC_لمبائی),
    FUNCTION2(octet_لمبائی,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(لکھو,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(حرف,              -1, 0, 0, charFunc         ),
    FUNCTION(مطلق_قدر,                1, 0, 0, absFunc          ),
#اگر_تعریف_شدہ SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#اگر_ختم
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
    FUNCTION(باری,              1, 0, 0, باری_Func        ),
    FUNCTION(باری,              2, 0, 0, باری_Func        ),
#اگر_ختم
    FUNCTION(بالائی,              1, 0, 0, بالائی_Func        ),
    FUNCTION(زیریں,              1, 0, 0, زیریں_Func        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_نسخہ,    0, 0, 0, نسخہ_Func      ),
    DFUNCTION(sqlite_ماخذ_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(آخری_داخل_کرو_rowid, 0, 0, 0, آخری_داخل_کرو_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(کل_changes,     0, 0, 0, کل_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(مجموع,   1,0,0, مجموع_قدم, مجموع_Finalize, مجموع_Finalize, مجموع_Inverse, 0),
    WAGGREGATE(کل, 1,0,0, مجموع_قدم,کل_Finalize,کل_Finalize,مجموع_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, مجموع_قدم, avgFinalize, avgFinalize, مجموع_Inverse, 0),
    WAGGREGATE(گنتی, 0,0,0, گنتی_قدم,
        گنتی_Finalize, گنتی_Finalize, گنتی_Inverse,
        SQLITE_FUNC_گنتی|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(گنتی, 1,0,0, گنتی_قدم,
        گنتی_Finalize, گنتی_Finalize, گنتی_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(گروہ_concat, 1, 0, 0, گروہ_Concat_قدم,
        گروہ_Concat_Finalize, گروہ_Concat_قدر, گروہ_Concat_Inverse, 0),
    WAGGREGATE(گروہ_concat, 2, 0, 0, گروہ_Concat_قدم,
        گروہ_Concat_Finalize, گروہ_Concat_قدر, گروہ_Concat_Inverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, گروہ_Concat_قدم,
        گروہ_Concat_Finalize, گروہ_Concat_قدر, گروہ_Concat_Inverse, 0),

#اگر_تعریف_شدہ SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percent_قدم,
        percentFinal, percent_قدر, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percent_قدم,
        percentFinal, percent_قدر, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percent_قدم,
        percentFinal, percent_قدر, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percent_قدم,
        percentFinal, percent_قدر, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#اگر_ختم /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#اگر_تعریف_شدہ SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#ورنہ
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#اگر SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#اگر_ختم
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(لوگارتھم,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(لوگارتھم,                2, 0, 0,      logFunc     ),
    MFUNCTION(قوت_نما,               1, قوت_نما,       math1Func   ),
    MFUNCTION(قوت,               2, قوت,       math2Func   ),
    MFUNCTION(power,             2, قوت,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(جیب_تمام,               1, جیب_تمام,       math1Func   ),
    MFUNCTION(جیب,               1, جیب,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#اگر SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#اگر_ختم
    MFUNCTION(جذر,              1, جذر,      math1Func   ),
    MFUNCTION(radians,           1, deg_مقصد_Rad,  math1Func   ),
    MFUNCTION(degrees,           1, rad_مقصد_Deg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#اگر_ختم /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(اگر,             -4, INLINEFUNC_iif,      0 ),
  };
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#اگر_ختم
  sqlite3_کھڑکی_Functions();
  sqlite3_رجسٹر_Date_وقت_Functions();
  sqlite3_رجسٹر_Json_Functions();
  sqlite3_داخل_کرو_Builtin_Funcs(aBuiltinFunc, Array_سائز(aBuiltinFunc));

#اگر 0  /* Enable to print out how the built-in functions are hashed */
  {
    صحیح i;
    FuncDef *p;
    برائے(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      لکھو("FUNC-HASH %02d:", i);
      برائے(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        صحیح n = sqlite3Strlen30(p->zName);
        صحیح h = p->zName[0] + n;
        أكد( p->funcFlags & SQLITE_FUNC_BUILTIN );
        لکھو(" %s(%d)", p->zName, h);
      }
      لکھو("\n");
    }
  }
#اگر_ختم
}
