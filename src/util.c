/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Utility functions used throughout sqlite.
**
** This file contains functions for allocating memory, comparing
** strings, and stuff like that.
**
*/
#شامل "sqliteInt.h"
#شامل <stdarg.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#شامل <math.h>

/* Work around a bug in older Microsoft compilers
** Forum post 2026-04-10T06:33:11z */
#اگر !defined(INFINITY) && defined(_MSC_VER)
# define INFINITY HUGE_VAL
#endif

#endif /* SQLITE_OMIT_FLOATING_POINT */

/*
** Calls to sqlite3FaultSim() are used to simulate a failure during testing,
** or to bypass normal error detection during testing in order to let
** execute proceed further downstream.
**
** In deployment, sqlite3FaultSim() *always* return SQLITE_OK (0).  The
** sqlite3FaultSim() function only returns non-zero during testing.
**
** During testing, if the test harness has set a fault-sim callback using
** a call to sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL), then
** each call to sqlite3FaultSim() is relayed to that application-supplied
** callback and the integer return value form the application-supplied
** callback is returned by sqlite3FaultSim().
**
** The integer argument to sqlite3FaultSim() is a code to identify which
** sqlite3FaultSim() instance is being invoked. Each call to sqlite3FaultSim()
** should have a unique code.  To prevent legacy testing applications from
** breaking, the codes should not be changed or reused.
*/
#ifndef SQLITE_UNTESTABLE
صحیح sqlite3FaultSim(صحیح iTest){
  صحیح (*xCallback)(صحیح) = sqlite3GlobalConfig.xTestCallback;
  بازگشت xCallback ? xCallback(iTest) : SQLITE_OK;
}
#endif

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is Not a Number (NaN).
**
** Use the math library isnan() function if compiled with SQLITE_HAVE_ISNAN.
** Otherwise, we have our own implementation that works on most systems.
*/
صحیح sqlite3IsNaN(دوگانه x){
  صحیح rc;   /* The value return */
#اگر !SQLITE_HAVE_ISNAN && !HAVE_ISNAN
  u64 y;
  memcpy(&y,&x,اندازه(y));
  rc = IsNaN(y);
#وگرنه
  rc = isnan(x);
#endif /* HAVE_ISNAN */
  testcase( rc );
  بازگشت rc;
}
#endif /* SQLITE_OMIT_FLOATING_POINT */

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is NaN or +Inf or -Inf.
*/
صحیح sqlite3IsOverflow(دوگانه x){
  صحیح rc;   /* The value return */
  u64 y;
  memcpy(&y,&x,اندازه(y));
  rc = IsOvfl(y);
  بازگشت rc;
}
#endif /* SQLITE_OMIT_FLOATING_POINT */

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
**
** The value returned will never be negative.  Nor will it ever be greater
** than the actual length of the string.  For very long strings (greater
** than 1GiB) the value returned might be less than the true string length.
*/
صحیح sqlite3Strlen30(ثابت نویسه *z){
  اگر( z==0 ) بازگشت 0;
  بازگشت 0x3fffffff & (صحیح)strlen(z);
}

/*
** Return the declared type of a column.  Or return zDflt if the column
** has no declared type.
**
** The column type is an extra string stored after the zero-terminator on
** the column name if and only if the COLFLAG_HASTYPE flag is set.
*/
نویسه *sqlite3ColumnType(Column *pCol, نویسه *zDflt){
  اگر( pCol->colFlags & COLFLAG_HASTYPE ){
    بازگشت pCol->zCnName + strlen(pCol->zCnName) + 1;
  }وگرنه اگر( pCol->eCType ){
    أكد( pCol->eCType<=SQLITE_N_STDTYPE );
    بازگشت (نویسه*)sqlite3StdType[pCol->eCType-1];
  }وگرنه{
    بازگشت zDflt;
  }
}

/*
** Helper function for sqlite3Error() - called rarely.  Broken out into
** a separate routine to avoid unnecessary register saves on entry to
** sqlite3Error().
*/
ایستا SQLITE_NOINLINE تهی  sqlite3ErrorFinish(sqlite3 *db, صحیح err_code){
  اگر( db->pErr ) sqlite3_مقدار_Set_Null(db->pErr);
  sqlite3SystemError(db, err_code);
}

/*
** Set the current error code to err_code and clear any prior error message.
** Also set iSysErrno (by calling sqlite3System) if the err_code indicates
** that would be appropriate.
*/
تهی sqlite3Error(sqlite3 *db, صحیح err_code){
  أكد( db!=0 );
  db->errCode = err_code;
  اگر( err_code || db->pErr ){
    sqlite3ErrorFinish(db, err_code);
  }وگرنه{
    db->errByteOffset = -1;
  }
}

/*
** The equivalent of sqlite3Error(db, SQLITE_OK).  Clear the error state
** and error message.
*/
تهی sqlite3ErrorClear(sqlite3 *db){
  أكد( db!=0 );
  db->errCode = SQLITE_OK;
  db->errByteOffset = -1;
  اگر( db->pErr ) sqlite3_مقدار_Set_Null(db->pErr);
}

/*
** Load the sqlite3.iSysErrno field if that is an appropriate thing
** to do based on the SQLite error code in rc.
*/
تهی sqlite3SystemError(sqlite3 *db, صحیح rc){
  اگر( rc==SQLITE_IOERR_NOMEM ) بازگشت;
#اگر defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
  اگر( rc==SQLITE_IOERR_IN_PAGE ){
    صحیح ii;
    صحیح iErr;
    sqlite3BtreeEnterAll(db);
    برای(ii=0; ii<db->nDb; ii++){
      اگر( db->aDb[ii].pBt ){
        iErr = sqlite3PagerWalSystemErrno(sqlite3BtreePager(db->aDb[ii].pBt));
        اگر( iErr ){
          db->iSysErrno = iErr;
        }
      }
    }
    sqlite3BtreeLeaveAll(db);
    بازگشت;
  }
#endif
  rc &= 0xff;
  اگر( rc==SQLITE_CANTOPEN || rc==SQLITE_IOERR ){
    db->iSysErrno = sqlite3OsGetLastError(db->pVfs);
  }
}

/*
** Set the most recent error code and error string for the sqlite
** handle "db". The error code is set to "err_code".
**
** If it is not NULL, string zFormat specifies the format of the
** error string.  zFormat and any string tokens that follow it are
** assumed to be encoded in UTF-8.
**
** To clear the most recent error for sqlite handle "db", sqlite3Error
** should be called with err_code set to SQLITE_OK and zFormat set
** to NULL.
*/
تهی sqlite3_Error_With_پیام(sqlite3 *db, صحیح err_code, ثابت نویسه *zFormat, ...){
  أكد( db!=0 );
  db->errCode = err_code;
  sqlite3SystemError(db, err_code);
  اگر( zFormat==0 ){
    sqlite3Error(db, err_code);
  }وگرنه اگر( db->pErr || (db->pErr = sqlite3_مقدار_New(db))!=0 ){
    نویسه *z;
    va_list ap;
    va_start(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);
    va_end(ap);
    sqlite3_مقدار_Set_Str(db->pErr, -1, z, SQLITE_UTF8, SQLITE_DYNAMIC);
  }
}

/*
** Check for interrupts and invoke progress callback.
*/
تهی sqlite3ProgressCheck(Parse *p){
  sqlite3 *db = p->db;
  اگر( AtomicLoad(&db->u1.isInterrupted) ){
    p->nErr++;
    p->rc = SQLITE_INTERRUPT;
  }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  اگر( db->xProgress ){
    اگر( p->rc==SQLITE_INTERRUPT ){
      p->nProgressSteps = 0;
    }وگرنه اگر( (++p->nProgressSteps)>=db->nProgressOps ){
      اگر( db->xProgress(db->pProgressArg) ){
        p->nErr++;
        p->rc = SQLITE_INTERRUPT;
      }
      p->nProgressSteps = 0;
    }
  }
#endif
}

/*
** Add an error message to pParse->zErrMsg and increment pParse->nErr.
**
** This function should be used to report any error that occurs while
** compiling an SQL statement (i.e. within sqlite3_prepare()). The
** last thing the sqlite3_prepare() function does is copy the error
** stored by this function into the database handle using sqlite3Error().
** Functions sqlite3Error() or sqlite3ErrorWithMsg() should be used
** during statement execution (sqlite3_step() etc.).
*/
تهی sqlite3_Error_پیام(Parse *pParse, ثابت نویسه *zFormat, ...){
  نویسه *z_پیام;
  va_list ap;
  sqlite3 *db = pParse->db;
  أكد( db!=0 );
  أكد( db->pParse==pParse || db->pParse->pToplevel==pParse );
  db->errByteOffset = -2;
  va_start(ap, zFormat);
  z_پیام = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  اگر( db->errByteOffset<-1 ) db->errByteOffset = -1;
  اگر( db->suppressErr ){
    sqlite3DbFree(db, z_پیام);
    اگر( db->mallocFailed ){
      pParse->nErr++;
      pParse->rc = SQLITE_NOMEM;
    }
  }وگرنه{
    pParse->nErr++;
    sqlite3DbFree(db, pParse->z_Err_پیام);
    pParse->z_Err_پیام = z_پیام;
    pParse->rc = SQLITE_ERROR;
    pParse->pWith = 0;
  }
}

/*
** If database connection db is currently parsing SQL, then transfer
** error code errCode to that parser if the parser has not already
** encountered some other kind of error.
*/
صحیح sqlite3ErrorToParser(sqlite3 *db, صحیح errCode){
  Parse *pParse;
  اگر( db==0 || (pParse = db->pParse)==0 ) بازگشت errCode;
  pParse->rc = errCode;
  pParse->nErr++;
  بازگشت errCode;
}

/*
** Convert an SQL-style quoted string into a normal string by removing
** the quote characters.  The conversion is done in-place.  If the
** input does not begin with a quote character, then this routine
** is a no-op.
**
** The input string must be zero-terminated.  A new zero-terminator
** is added to the dequoted string.
**
** The return value is -1 if no dequoting occurs or the length of the
** dequoted string, exclusive of the zero terminator, if dequoting does
** occur.
**
** 2002-02-14: This routine is extended to remove MS-Access style
** brackets from around identifiers.  For example:  "[a-b-c]" becomes
** "a-b-c".
*/
تهی sqlite3Dequote(نویسه *z){
  نویسه quote;
  صحیح i, j;
  اگر( z==0 ) بازگشت;
  quote = z[0];
  اگر( !sqlite3Isquote(quote) ) بازگشت;
  اگر( quote=='[' ) quote = ']';
  برای(i=1, j=0;; i++){
    أكد( z[i] );
    اگر( z[i]==quote ){
      اگر( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }وگرنه{
        بشکن;
      }
    }وگرنه{
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}
تهی sqlite3_Dequote_عبارت(عبارت *p){
  أكد( !عبارت_Has_Property(p, EP_Int_مقدار) );
  أكد( sqlite3Isquote(p->u.zToken[0]) );
  p->flags |= p->u.zToken[0]=='"' ? EP_Quoted|EP_DblQuoted : EP_Quoted;
  sqlite3Dequote(p->u.zToken);
}

/*
** Expression p is a QNUMBER (quoted number). Dequote the value in p->u.zToken
** and set the type to INTEGER or FLOAT. "Quoted" integers or floats are those
** that contain '_' characters that must be removed before further processing.
*/
تهی sqlite3_Dequote_عدد(Parse *pParse, عبارت *p){
  أكد( p!=0 || pParse->db->mallocFailed );
  اگر( p ){
    ثابت نویسه *pIn = p->u.zToken;
    نویسه *pOut = p->u.zToken;
    صحیح bHex = (pIn[0]=='0' && (pIn[1]=='x' || pIn[1]=='X'));
    صحیح i_مقدار;
    أكد( p->op==TK_QNUMBER );
    p->op = TK_INTEGER;
    انجامبده {
      اگر( *pIn!=SQLITE_DIGIT_SEPARATOR ){
        *pOut++ = *pIn;
        اگر( *pIn=='e' || *pIn=='E' || *pIn=='.' ) p->op = TK_FLOAT;
      }وگرنه{
        اگر( (bHex==0 && (!sqlite3Isdigit(pIn[-1]) || !sqlite3Isdigit(pIn[1])))
         || (bHex==1 && (!sqlite3Isxdigit(pIn[-1]) || !sqlite3Isxdigit(pIn[1])))
        ){
          sqlite3_Error_پیام(pParse, "unrecognized token: \"%s\"", p->u.zToken);
        }
      }
    }وقتی( *pIn++ );
    اگر( bHex ) p->op = TK_INTEGER;

    /* tag-20240227-a: If after dequoting, the number is an integer that
    ** fits in 32 bits, then it must be converted into EP_IntValue.  Other
    ** parts of the code expect this.  See also tag-20240227-b. */
    اگر( p->op==TK_INTEGER && sqlite3GetInt32(p->u.zToken, &i_مقدار) ){
      p->u.i_مقدار = i_مقدار;
      p->flags |= EP_Int_مقدار;
    }
  }
}

/*
** If the input token p is quoted, try to adjust the token to remove
** the quotes.  This is not always possible:
**
**     "abc"     ->   abc
**     "ab""cd"  ->   (not possible because of the interior "")
**
** Remove the quotes if possible.  This is a optimization.  The overall
** system should still return the correct answer even if this routine
** is always a no-op.
*/
تهی sqlite3DequoteToken(Token *p){
  unsigned صحیح i;
  اگر( p->n<2 ) بازگشت;
  اگر( !sqlite3Isquote(p->z[0]) ) بازگشت;
  برای(i=1; i<p->n-1; i++){
    اگر( sqlite3Isquote(p->z[i]) ) بازگشت;
  }
  p->n -= 2;
  p->z++;
}

/*
** Generate a Token object from a string
*/
تهی sqlite3TokenInit(Token *p, نویسه *z){
  p->z = z;
  p->n = sqlite3Strlen30(z);
}

/* Convenient short-hand */
#define UpperToLower sqlite3UpperToLower

/*
** Some systems have stricmp().  Others have strcasecmp().  Because
** there is no consistency, we will define our own.
**
** IMPLEMENTATION-OF: R-30243-02494 The sqlite3_stricmp() and
** sqlite3_strnicmp() APIs allow applications and extensions to compare
** the contents of two buffers containing UTF-8 strings in a
** case-independent fashion, using the same definition of "case
** independence" that SQLite uses internally when comparing identifiers.
*/
صحیح sqlite3_stricmp(ثابت نویسه *z_چپ, ثابت نویسه *z_راست){
  اگر( z_چپ==0 ){
    بازگشت z_راست ? -1 : 0;
  }وگرنه اگر( z_راست==0 ){
    بازگشت 1;
  }
  بازگشت sqlite3StrICmp(z_چپ, z_راست);
}
صحیح sqlite3StrICmp(ثابت نویسه *z_چپ, ثابت نویسه *z_راست){
  unsigned نویسه *a, *b;
  صحیح c, x;
  a = (unsigned نویسه *)z_چپ;
  b = (unsigned نویسه *)z_راست;
  برای(;;){
    c = *a;
    x = *b;
    اگر( c==x ){
      اگر( c==0 ) بشکن;
    }وگرنه{
      c = (صحیح)UpperToLower[c] - (صحیح)UpperToLower[x];
      اگر( c ) بشکن;
    }
    a++;
    b++;
  }
  بازگشت c;
}
صحیح sqlite3_strnicmp(ثابت نویسه *z_چپ, ثابت نویسه *z_راست, صحیح N){
  ثبات unsigned نویسه *a, *b;
  اگر( z_چپ==0 ){
    بازگشت z_راست ? -1 : 0;
  }وگرنه اگر( z_راست==0 ){
    بازگشت 1;
  }
  a = (unsigned نویسه *)z_چپ;
  b = (unsigned نویسه *)z_راست;
  وقتی( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  بازگشت N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/*
** Compute an 8-bit hash on a string that is insensitive to case differences
*/
u8 sqlite3StrIHash(ثابت نویسه *z){
  u8 h = 0;
  اگر( z==0 ) بازگشت 0;
  وقتی( z[0] ){
    h += UpperToLower[(unsigned نویسه)z[0]];
    z++;
  }
  بازگشت h;
}

#اگر !defined(SQLITE_DISABLE_INTRINSIC)               \
  && (defined(__GNUC__) || defined(__clang__))       \
  && (defined(__x86_64__) || defined(__aarch64__) || \
      (defined(__riscv) && defined(__riscv_xlen) && (__riscv_xlen>32)))
#define SQLITE_USE_UINT128
#endif

/*
** Two inputs are multiplied to get a 128-bit result.  Write the
** lower 64-bits of the result into *pLo, and return the high-order
** 64 bits.
*/
ایستا u64 sqlite3Multiply128(u64 a, u64 b, u64 *pLo){
#اگر defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  *pLo = (u64)r;
  بازگشت (u64)(r>>64);
#وگرنهاگر defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  *pLo = a*b;
  بازگشت __umulh(a, b);
#وگرنه
  u64 a0 = (u32)a;
  u64 a1 = a >> 32;
  u64 b0 = (u32)b;
  u64 b1 = b >> 32;
  u64 a0b0 = a0 * b0;
  u64 a1b1 = a1 * b1;
  u64 a0b1 = a0 * b1;
  u64 a1b0 = a1 * b0;
  u64 t = (a0b0 >> 32) + (u32)a0b1 + (u32)a1b0;
  *pLo = (a0b0 & UINT64_C(0xffffffff)) | (t << 32);
  بازگشت a1b1 + (a0b1>>32) + (a1b0>>32) + (t>>32);
#endif
}

/*
** A is an unsigned 96-bit integer formed by (a<<32)+aLo.
** B is an unsigned 64-bit integer.
**
** Compute the upper 96 bits of 160-bit result of A*B.
**
** Write ((A*B)>>64 & 0xffffffff) (the middle 32 bits of A*B)
** into *pLo.  Return the upper 64 bits of A*B.
**
** The lower 64 bits of A*B are discarded.
*/
ایستا u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo){
#اگر defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  r += ((__uint128_t)aLo * b) >> 32;
  *pLo = (r>>32)&0xffffffff;
  بازگشت r>>64;
#وگرنهاگر defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  u64 r1_hi = __umulh(a,b);
  u64 r1_lo = a*b;
  u64 r2 = (__umulh((u64)aLo,b)<<32) + ((aLo*b)>>32);
  u64 t = r1_lo + r2;
  اگر( t<r1_lo ) r1_hi++;
  *pLo = t>>32;
  بازگشت r1_hi;
#وگرنه
  u64 x2 = a>>32;
  u64 x1 = a&0xffffffff;
  u64 x0 = aLo;
  u64 y1 = b>>32;
  u64 y0 = b&0xffffffff;
  u64 x2y1 = x2*y1;
  u64 r4 = x2y1>>32;
  u64 x2y0 = x2*y0;
  u64 x1y1 = x1*y1;
  u64 r3 = (x2y1 & 0xffffffff) + (x2y0 >>32) + (x1y1 >>32);
  u64 x1y0 = x1*y0;
  u64 x0y1 = x0*y1;
  u64 r2 = (x2y0 & 0xffffffff) + (x1y1 & 0xffffffff) +
             (x1y0 >>32) + (x0y1>>32);
  u64 x0y0 = x0*y0;
  u64 r1 = (x1y0 & 0xffffffff) + (x0y1 & 0xffffffff) +
             (x0y0 >>32);
  r2 += r1>>32;
  r3 += r2>>32;
  *pLo = r2&0xffffffff;
  بازگشت (r4<<32) + r3;
#endif
}

#undef SQLITE_USE_UINT128

/*
** Return a u64 with the N-th bit set.
*/
#define U64_BIT(N)  (((u64)1)<<(N))

/*
** Range of powers of 10 that we need to deal with when converting
** IEEE754 doubles to and from decimal.
*/
#define POWERSOF10_FIRST (-348)
#define POWERSOF10_LAST  (+347)

/*
** For any p between -348 and +347, return the integer part of
**
**    pow(10,p) * pow(2,63-pow10to2(p))
**
** Or, in other words, for any p in range, return the most significant
** 64 bits of pow(10,p).  The pow(10,p) value is shifted left or right,
** as appropriate so the most significant 64 bits fit exactly into a
** 64-bit unsigned integer.
**
** Write into *pLo the next 32 significant bits of the answer after
** the first 64.
**
** Algorithm:
**
** (1) For p between 0 and 26, return the value directly from the aBase[]
**     lookup table.
**
** (2) For p outside the range 0 to 26, use aScale[] for the initial value
**     then refine that result (if necessary) by a single multiplication
**     against aBase[].
**
** The constant tables aBase[], aScale[], and aScaleLo[] are generated
** by the C program at ../tool/mkfptab.c run with the --round option.
*/
ایستا u64 powerOfTen(صحیح p, u32 *pLo){
  ایستا ثابت u64 aBase[] = {
    UINT64_C(0x8000000000000000), /*  0: 1.0e+0 << 63 */
    UINT64_C(0xa000000000000000), /*  1: 1.0e+1 << 60 */
    UINT64_C(0xc800000000000000), /*  2: 1.0e+2 << 57 */
    UINT64_C(0xfa00000000000000), /*  3: 1.0e+3 << 54 */
    UINT64_C(0x9c40000000000000), /*  4: 1.0e+4 << 50 */
    UINT64_C(0xc350000000000000), /*  5: 1.0e+5 << 47 */
    UINT64_C(0xf424000000000000), /*  6: 1.0e+6 << 44 */
    UINT64_C(0x9896800000000000), /*  7: 1.0e+7 << 40 */
    UINT64_C(0xbebc200000000000), /*  8: 1.0e+8 << 37 */
    UINT64_C(0xee6b280000000000), /*  9: 1.0e+9 << 34 */
    UINT64_C(0x9502f90000000000), /* 10: 1.0e+10 << 30 */
    UINT64_C(0xba43b74000000000), /* 11: 1.0e+11 << 27 */
    UINT64_C(0xe8d4a51000000000), /* 12: 1.0e+12 << 24 */
    UINT64_C(0x9184e72a00000000), /* 13: 1.0e+13 << 20 */
    UINT64_C(0xb5e620f480000000), /* 14: 1.0e+14 << 17 */
    UINT64_C(0xe35fa931a0000000), /* 15: 1.0e+15 << 14 */
    UINT64_C(0x8e1bc9bf04000000), /* 16: 1.0e+16 << 10 */
    UINT64_C(0xb1a2bc2ec5000000), /* 17: 1.0e+17 << 7 */
    UINT64_C(0xde0b6b3a76400000), /* 18: 1.0e+18 << 4 */
    UINT64_C(0x8ac7230489e80000), /* 19: 1.0e+19 >> 0 */
    UINT64_C(0xad78ebc5ac620000), /* 20: 1.0e+20 >> 3 */
    UINT64_C(0xd8d726b7177a8000), /* 21: 1.0e+21 >> 6 */
    UINT64_C(0x878678326eac9000), /* 22: 1.0e+22 >> 10 */
    UINT64_C(0xa968163f0a57b400), /* 23: 1.0e+23 >> 13 */
    UINT64_C(0xd3c21bcecceda100), /* 24: 1.0e+24 >> 16 */
    UINT64_C(0x84595161401484a0), /* 25: 1.0e+25 >> 20 */
    UINT64_C(0xa56fa5b99019a5c8), /* 26: 1.0e+26 >> 23 */
  };
  ایستا ثابت u64 aScale[] = {
    UINT64_C(0x8049a4ac0c5811ae), /*  0: 1.0e-351 << 1229 */
    UINT64_C(0xcf42894a5dce35ea), /*  1: 1.0e-324 << 1140 */
    UINT64_C(0xa76c582338ed2621), /*  2: 1.0e-297 << 1050 */
    UINT64_C(0x873e4f75e2224e68), /*  3: 1.0e-270 << 960 */
    UINT64_C(0xda7f5bf590966848), /*  4: 1.0e-243 << 871 */
    UINT64_C(0xb080392cc4349dec), /*  5: 1.0e-216 << 781 */
    UINT64_C(0x8e938662882af53e), /*  6: 1.0e-189 << 691 */
    UINT64_C(0xe65829b3046b0afa), /*  7: 1.0e-162 << 602 */
    UINT64_C(0xba121a4650e4ddeb), /*  8: 1.0e-135 << 512 */
    UINT64_C(0x964e858c91ba2655), /*  9: 1.0e-108 << 422 */
    UINT64_C(0xf2d56790ab41c2a2), /* 10: 1.0e-81 << 333 */
    UINT64_C(0xc428d05aa4751e4c), /* 11: 1.0e-54 << 243 */
    UINT64_C(0x9e74d1b791e07e48), /* 12: 1.0e-27 << 153 */
    UINT64_C(0xcccccccccccccccc), /* 13: 1.0e-1 << 67 (special case) */
    UINT64_C(0xcecb8f27f4200f3a), /* 14: 1.0e+27 >> 26 */
    UINT64_C(0xa70c3c40a64e6c51), /* 15: 1.0e+54 >> 116 */
    UINT64_C(0x86f0ac99b4e8dafd), /* 16: 1.0e+81 >> 206 */
    UINT64_C(0xda01ee641a708de9), /* 17: 1.0e+108 >> 295 */
    UINT64_C(0xb01ae745b101e9e4), /* 18: 1.0e+135 >> 385 */
    UINT64_C(0x8e41ade9fbebc27d), /* 19: 1.0e+162 >> 475 */
    UINT64_C(0xe5d3ef282a242e81), /* 20: 1.0e+189 >> 564 */
    UINT64_C(0xb9a74a0637ce2ee1), /* 21: 1.0e+216 >> 654 */
    UINT64_C(0x95f83d0a1fb69cd9), /* 22: 1.0e+243 >> 744 */
    UINT64_C(0xf24a01a73cf2dccf), /* 23: 1.0e+270 >> 833 */
    UINT64_C(0xc3b8358109e84f07), /* 24: 1.0e+297 >> 923 */
    UINT64_C(0x9e19db92b4e31ba9), /* 25: 1.0e+324 >> 1013 */
  };
  ایستا ثابت unsigned صحیح aScaleLo[] = {
    0x205b896d, /*  0: 1.0e-351 << 1229 */
    0x52064cad, /*  1: 1.0e-324 << 1140 */
    0xaf2af2b8, /*  2: 1.0e-297 << 1050 */
    0x5a7744a7, /*  3: 1.0e-270 << 960 */
    0xaf39a475, /*  4: 1.0e-243 << 871 */
    0xbd8d794e, /*  5: 1.0e-216 << 781 */
    0x547eb47b, /*  6: 1.0e-189 << 691 */
    0x0cb4a5a3, /*  7: 1.0e-162 << 602 */
    0x92f34d62, /*  8: 1.0e-135 << 512 */
    0x3a6a07f9, /*  9: 1.0e-108 << 422 */
    0xfae27299, /* 10: 1.0e-81 << 333 */
    0xaa97e14c, /* 11: 1.0e-54 << 243 */
    0x775ea265, /* 12: 1.0e-27 << 153 */
    0xcccccccc, /* 13: 1.0e-1 << 67 (special case) */
    0x00000000, /* 14: 1.0e+27 >> 26 */
    0x999090b6, /* 15: 1.0e+54 >> 116 */
    0x69a028bb, /* 16: 1.0e+81 >> 206 */
    0xe80e6f48, /* 17: 1.0e+108 >> 295 */
    0x5ec05dd0, /* 18: 1.0e+135 >> 385 */
    0x14588f14, /* 19: 1.0e+162 >> 475 */
    0x8f1668c9, /* 20: 1.0e+189 >> 564 */
    0x6d953e2c, /* 21: 1.0e+216 >> 654 */
    0x4abdaf10, /* 22: 1.0e+243 >> 744 */
    0xbc633b39, /* 23: 1.0e+270 >> 833 */
    0x0a862f81, /* 24: 1.0e+297 >> 923 */
    0x6c07a2c2, /* 25: 1.0e+324 >> 1013 */
  };
  صحیح g, n;
  u64 s, x;
  u32 lo;

  أكد( p>=POWERSOF10_FIRST && p<=POWERSOF10_LAST );
  اگر( p<0 ){
    اگر( p==(-1) ){
      *pLo = aScaleLo[13];
      بازگشت aScale[13];
    }
    g = p/27;
    n = p%27;
    اگر( n ){
      g--;
      n += 27;
    }
  }وگرنه اگر( p<27 ){
    *pLo = 0;
    بازگشت aBase[p];
  }وگرنه{
    g = p/27;
    n = p%27;
  }
  s = aScale[g+13];
  اگر( n==0 ){
    *pLo = aScaleLo[g+13];
    بازگشت s;
  }
  x = sqlite3Multiply160(s,aScaleLo[g+13],aBase[n],&lo);
  اگر( (U64_BIT(63) & x)==0 ){
    x  = x<<1 | ((lo>>31)&1);
    lo = (lo<<1) | 1;
  }
  *pLo = lo;
  بازگشت x;
}

/*
** pow10to2(x) computes floor(log2(pow(10,x))).
** pow2to10(y) computes floor(log10(pow(2,y))).
**
** Conceptually, pow10to2(p) converts a base-10 exponent p into
** a corresponding base-2 exponent, and pow2to10(e) converts a base-2
** exponent into a base-10 exponent.
**
** The conversions are based on the observation that:
**
**     ln(10.0)/ln(2.0) == 108853/32768     (approximately)
**     ln(2.0)/ln(10.0) == 78913/262144     (approximately)
**
** These ratios are approximate, but they are accurate to 5 digits,
** which is close enough for the usage here.  Right-shift is used
** for division so that rounding of negative numbers happens in the
** right direction.
*/
ایستا صحیح pwr10to2(صحیح p){ بازگشت (p*108853) >> 15; }
ایستا صحیح pwr2to10(صحیح p){ بازگشت (p*78913) >> 18; }

/*
** Count leading zeros for a 64-bit unsigned integer.
*/
ایستا صحیح شمار_Leading_Zeros(u64 m){
#اگر (defined(__GNUC__) || defined(__clang__)) \
    && !defined(SQLITE_DISABLE_INTRINSIC)
  بازگشت __builtin_clzll(m);
#وگرنه
  صحیح n = 0;
  اگر( m <= 0x00000000ffffffffULL) { n += 32; m <<= 32; }
  اگر( m <= 0x0000ffffffffffffULL) { n += 16; m <<= 16; }
  اگر( m <= 0x00ffffffffffffffULL) { n += 8;  m <<= 8;  }
  اگر( m <= 0x0fffffffffffffffULL) { n += 4;  m <<= 4;  }
  اگر( m <= 0x3fffffffffffffffULL) { n += 2;  m <<= 2;  }
  اگر( m <= 0x7fffffffffffffffULL) { n += 1;            }
  بازگشت n;
#endif
}

/*
** Given m and e, which represent a quantity r == m*pow(2,e),
** return values *pD and *pP such that r == (*pD)*pow(10,*pP),
** approximately.  *pD should contain at least n significant digits.
**
** The input m is required to have its highest bit set.  In other words,
** m should be left-shifted, and e decremented, to maximize the value of m.
*/
ایستا تهی sqlite3Fp2Convert10(u64 m, صحیح e, صحیح n, u64 *pD, صحیح *pP){
  صحیح p;
  u64 h, d1;
  u32 d2;
  أكد( n>=1 && n<=18 );
  p = n - 1 - pwr2to10(e+63);
  h = sqlite3Multiply128(m, powerOfTen(p,&d2), &d1);
  أكد( -(e + pwr10to2(p) + 2) >= 0  );
  أكد( -(e + pwr10to2(p) + 1) <= 63 );
  اگر( n==18 ){
    h >>= -(e + pwr10to2(p) + 2);
    *pD = (h + ((h<<1)&2))>>1;
  }وگرنه{
    *pD = h >> -(e + pwr10to2(p) + 1);
  }
  *pP = -p;
}

/*
** Return an IEEE754 floating point value that approximates d*pow(10,p).
**
** The (current) algorithm is adapted from the work of Ross Cox at
** https://github.com/rsc/fpfmt
*/
ایستا دوگانه sqlite3Fp10Convert2(u64 d, صحیح p){
  صحیح b, lp, e, adj, s;
  u32 pwr10l, mid1;
  u64 pwr10h, x, hi, lo, sticky, u, m;
  دوگانه r;
  اگر( p<POWERSOF10_FIRST ) بازگشت 0.0;
  اگر( p>POWERSOF10_LAST ) بازگشت (دوگانه)INFINITY;
  b = 64 - شمار_Leading_Zeros(d);
  lp = pwr10to2(p);
  e = 53 - b - lp;
  اگر( e > 1074 ){
    اگر( e>=1130 ) بازگشت 0.0;
    e = 1074;
  }
  s = -(e-(64-b) + lp + 3);
  pwr10h = powerOfTen(p, &pwr10l);
  اگر( pwr10l!=0 ){
    pwr10h++;
    pwr10l = ~pwr10l;
  }
  x = d<<(64-b);
  hi = sqlite3Multiply128(x,pwr10h,&lo);
  mid1 = lo>>32;
  sticky = 1;
  اگر( (hi & (U64_BIT(s)-1))==0 ) {
    u32 mid2 = sqlite3Multiply128(x,((u64)pwr10l)<<32,&lo)>>32;
    sticky = (mid1-mid2 > 1);
    hi -= mid1 < mid2;
  }
  u = (hi>>s) | sticky;
  adj = (u >= U64_BIT(55)-2);
  اگر( adj ){
    u = (u>>adj) | (u&1);
    e -= adj;
  }
  m = (u + 1 + ((u>>2)&1)) >> 2;
  اگر( e<=(-972) ) بازگشت (دوگانه)INFINITY;
  اگر((m & U64_BIT(52)) != 0){
    m = (m & ~U64_BIT(52)) | ((u64)(1075-e)<<52);
  }
  memcpy(&r,&m,8);
  بازگشت r;
}

/*
** The string z[] is an text representation of a real number.
** Convert this string to a double and write it into *pResult.
**
** z[] must be UTF-8 and zero-terminated.
**
** Return positive if the result is a valid real number (or integer) and
** zero or negative if the string is empty or contains extraneous text.
** Lower bits of the return value contain addition information about the
** parse:
**
**   bit 0       =>   Set if any prefix of the input is valid.  Clear if
**                    there is no prefix of the input that can be seen as
**                    a valid floating point number.
**   bit 1       =>   Set if the input contains a decimal point or eNNN
**                    clause.  Zero if the input is an integer.
**   bit 2       =>   The input is exactly 0.0, not an underflow from
**                    some value near zero.
**   bit 3       =>   Set if there are more than about 19 significant
**                    digits in the input.
**
** If the input contains a syntax error but begins with text that might
** be a valid number of some kind, then the result is negative.  The
** result is only zero if no prefix of the input could be interpreted as
** a number.
**
** Leading and trailing whitespace is ignored.  Valid numbers are in
** one of the formats below:
**
**    [+-]digits[E[+-]digits]
**    [+-]digits.[digits][E[+-]digits]
**    [+-].digits[E[+-]digits]
**
** Algorithm sketch:  Compute an unsigned 64-bit integer s and a base-10
** exponent d such that the value encoding by the input is s*pow(10,d).
** Then invoke sqlite3Fp10Convert2() to calculated the closest possible
** IEEE754 double.  The sign is added back afterwards, if the input string
** starts with a "-".  The use of an unsigned 64-bit s mantissa means that
** only about the first 19 significant digits of the input can contribute
** to the result.  This can result in suboptimal rounding decisions when
** correct rounding requires more than 19 input digits.  For example,
** this routine renders "3500000000000000.2500001" as
** 3500000000000000.0 instead of 3500000000000000.5 because the decision
** to round up instead of using banker's rounding to round down is determined
** by the 23rd significant digit, which this routine ignores. It is not
** possible to do better without some kind of BigNum.
*/
صحیح sqlite3AtoF(ثابت نویسه *zIn, دوگانه *pResult){
#ifndef SQLITE_OMIT_FLOATING_POINT
  ثابت unsigned نویسه *z = (ثابت unsigned نویسه*)zIn;
  صحیح neg = 0;       /* True for a negative value */
  u64 s = 0;         /* mantissa */
  صحیح d = 0;         /* Value is s * pow(10,d) */
  صحیح mState = 0;    /* 1: digit seen 2: fp 4: hard-zero */
  unsigned v;        /* Value of a single digit */

  start_of_text:
  اگر( (v = (unsigned)z[0] - '0')<10 ){
    parse_integer_part:
    mState = 1;
    s = v;
    z++;
    وقتی( (v = (unsigned)z[0] - '0')<10 ){
      s = s*10 + v;
      z++;
      اگر( s>=(LARGEST_UINT64-9)/10 ){
        mState = 9;
        وقتی( sqlite3Isdigit(z[0]) ){ z++; d++; }
        بشکن;
      }
    }
  }وگرنه اگر( z[0]=='-' ){
    neg = 1;
    z++;
    اگر( (v = (unsigned)z[0] - '0')<10 ) برو parse_integer_part;
  }وگرنه اگر( z[0]=='+' ){
    z++;
    اگر( (v = (unsigned)z[0] - '0')<10 ) برو parse_integer_part;
  }وگرنه اگر( sqlite3Isspace(z[0]) ){
    انجامبده{ z++; }وقتی( sqlite3Isspace(z[0]) );
    برو start_of_text;
  }وگرنه{
    s = 0;
  }

  /* if decimal point is present */
  اگر( *z=='.' ){
    z++;
    اگر( sqlite3Isdigit(z[0]) ){
      mState |= 1;
      انجامبده{
        اگر( s<(LARGEST_UINT64-9)/10 ){
          s = s*10 + z[0] - '0';
          d--;
        }وگرنه{
          mState = 11;
        }
      }وقتی( sqlite3Isdigit(*++z) );
    }وگرنه اگر( mState==0 ){
      *pResult = 0.0;
      بازگشت 0;
    }
    mState |= 2;
  }وگرنه اگر( mState==0 ){
    *pResult = 0.0;
    بازگشت 0;
  }

  /* if exponent is present */
  اگر( *z=='e' || *z=='E' ){
    صحیح esign;
    z++;
 
    /* get sign of exponent */
    اگر( *z=='-' ){
      esign = -1;
      z++;
    }وگرنه{
      esign = +1;
      اگر( *z=='+' ){
        z++;
      }
    }
    /* copy digits to exponent */
    اگر( (v = (unsigned)z[0] - '0')<10 ){
      صحیح exp = v;
      z++;
      mState |= 2;
      وقتی( (v = (unsigned)z[0] - '0')<10 ){
        exp = exp<10000 ? (exp*10 + v) : 10000;
        z++;
      }
      d += esign*exp;
    }وگرنه{
      z--;  /* Leave z[0] at 'e' or '+' or '-',
            ** so that the return is 0 or -1 */
    }
  }

  /* Convert s*pow(10,d) into real */
  اگر( s==0 ){
    *pResult = 0.0;
    mState |= 4;
  }وگرنه{
    *pResult = sqlite3Fp10Convert2(s,d);
  }
  اگر( neg ) *pResult = -*pResult;
  أكد( !sqlite3IsNaN(*pResult) );

  /* return true if number and no extra non-whitespace characters after */
  اگر( z[0]==0 ){
    بازگشت mState;
  }
  اگر( sqlite3Isspace(z[0]) ){
    انجامبده{ z++; }وقتی( sqlite3Isspace(*z) );
    اگر( z[0]==0 ){
      بازگشت mState;
    }
  }
  بازگشت 0xfffffff0 | mState;
#وگرنه
  بازگشت sqlite3Atoi64(zIn, pResult, strlen(zIn), SQLITE_UTF8)==0;
#endif /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Digit pairs used to convert a U64 or I64 into text, two digits
** at a time.
*/
ایستا ثابت اجتماع {
  نویسه a[201];
  short صحیح forceAlignment;
} sqlite3DigitPairs = {
  "00010203040506070809"
  "10111213141516171819"
  "20212223242526272829"
  "30313233343536373839"
  "40414243444546474849"
  "50515253545556575859"
  "60616263646566676869"
  "70717273747576777879"
  "80818283848586878889"
  "90919293949596979899"
};

/*
** ARMv6, ARMv7, PPC32 are known to not support hardware u64 division.
*/
#اگر (defined(__arm__) && !defined(__aarch64__)) || \
    (defined(__ppc__) && !defined(__ppc64__))
# define SQLITE_AVOID_U64_DIVIDE 1
#endif

#ifdef SQLITE_AVOID_U64_DIVIDE
/*
** Render an unsigned 64-bit integer as text onto the end of a 2-byte
** aligned buffer that is SQLITE_U64_DIGIT+1 bytes long.  The last byte
** of the buffer will be filled with a \000 byte.
**
** Return the index into the buffer of the first byte.
**
** This routine is used on platforms where u64-division is slow because
** it is not available in hardware and has to be emulated in software.
** It seeks to minimize the number of u64 divisions and use u32 divisions
** instead.  It is slower on platforms that have hardware u64 division,
** but much faster on platforms that do not.
*/
ایستا صحیح sqlite3UInt64ToText(u64 v, نویسه *zOut){
  u32 x32, kk;
  صحیح i;
  zOut[SQLITE_U64_DIGITS] = 0;
  i = SQLITE_U64_DIGITS;
  أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[0]) );
  أكد( TWO_BYTE_ALIGNMENT(zOut) );
  وقتی( (v>>32)!=0 ){
    u32 y, x0, x1, y0, y1;
    x32 = v % 100000000;
    v   = v / 100000000;
    y = x32 % 10000;
    x32 /= 10000;
    x1 = x32 / 100;
    x0 = x32 % 100;
    y1 = y / 100;
    y0 = y % 100;
    أكد( i>=8 );
    i -= 8;
    *(u16*)(&zOut[i]) = *(u16*)&sqlite3DigitPairs.a[x1*2];
    *(u16*)(&zOut[i+2]) = *(u16*)&sqlite3DigitPairs.a[x0*2];
    *(u16*)(&zOut[i+4]) = *(u16*)&sqlite3DigitPairs.a[y1*2];
    *(u16*)(&zOut[i+6]) = *(u16*)&sqlite3DigitPairs.a[y0*2];
  }
  x32 = v;
  وقتی( x32>=10 ){
    kk  = x32 % 100;
    x32 = x32 / 100;
    أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk*2]) );
    أكد( i>=2 );
    i -= 2;
    أكد( TWO_BYTE_ALIGNMENT(&zOut[i]) );
    *(u16*)(&zOut[i]) = *(u16*)&sqlite3DigitPairs.a[kk*2];
  }
  اگر( x32 ){
    أكد( i>0 );
    zOut[--i] = x32 + '0';
  }
  بازگشت i;
}
#endif /* defined(SQLITE_AVOID_U64_DIVIDE) */

/*
** Render an signed 64-bit integer as text.  Store the result in zOut[] and
** return the length of the string that was stored, in bytes.  The value
** returned does not include the zero terminator at the end of the output
** string.
**
** The caller must ensure that zOut[] is at least 21 bytes in size.
*/
صحیح sqlite3Int64ToText(i64 v, نویسه *zOut){
  صحیح i;
  u64 x;
  اجتماع {
    نویسه a[SQLITE_U64_DIGITS+1];
    u16 forceAlignment;
  } u;
  اگر( v>0 ){
    x = v;
  }وگرنه اگر( v==0 ){
    zOut[0] = '0';
    zOut[1] = 0;
    بازگشت 1;
  }وگرنه{
    x = (v==SMALLEST_INT64) ? ((u64)1)<<63 : (u64)-v;
  }
#ifdef SQLITE_AVOID_U64_DIVIDE
  i = sqlite3UInt64ToText(x, u.a);
#وگرنه
  i = اندازه(u.a)-1;
  u.a[i] = 0;
  وقتی( x>=10 ){
    صحیح kk = (x%100)*2;
    أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk]) );
    أكد( TWO_BYTE_ALIGNMENT(&u.a[i-2]) );
    *(u16*)(&u.a[i-2]) = *(u16*)&sqlite3DigitPairs.a[kk];
    i -= 2;
    x /= 100;
  }
  اگر( x ){
    u.a[--i] = x + '0';
  }
#endif /* SQLITE_AVOID_U64_DIVIDE */
  اگر( v<0 ) u.a[--i] = '-';
  memcpy(zOut, &u.a[i], اندازه(u.a)-i);
  بازگشت اندازه(u.a)-1-i;
}

/*
** Compare the 19-character string zNum against the text representation
** value 2^63:  9223372036854775808.  Return negative, zero, or positive
** if zNum is less than, equal to, or greater than the string.
** Note that zNum must contain exactly 19 characters.
**
** Unlike memcmp() this routine is guaranteed to return the difference
** in the values of the last digit if the only difference is in the
** last digit.  So, for example,
**
**      compare2pow63("9223372036854775800", 1)
**
** will return -8.
*/
ایستا صحیح compare2pow63(ثابت نویسه *z_رقم, صحیح incr){
  صحیح c = 0;
  صحیح i;
                    /* 012345678901234567 */
  ثابت نویسه *pow63 = "922337203685477580";
  برای(i=0; c==0 && i<18; i++){
    c = (z_رقم[i*incr]-pow63[i])*10;
  }
  اگر( c==0 ){
    c = z_رقم[18*incr] - '8';
    testcase( c==(-1) );
    testcase( c==0 );
    testcase( c==(+1) );
  }
  بازگشت c;
}

/*
** Convert zNum to a 64-bit signed integer.  zNum must be decimal. This
** routine does *not* accept hexadecimal notation.
**
** Returns:
**
**    -1    Not even a prefix of the input text looks like an integer
**     0    Successful transformation.  Fits in a 64-bit signed integer.
**     1    Excess non-space text after the integer value
**     2    Integer too large for a 64-bit signed integer or is malformed
**     3    Special case of 9223372036854775808
**
** length is the number of bytes in the string (bytes, not characters).
** The string is not necessarily zero-terminated.  The encoding is
** given by enc.
*/
صحیح sqlite3Atoi64(ثابت نویسه *z_رقم, i64 *p_رقم, صحیح length, u8 enc){
  صحیح incr;
  u64 u = 0;
  صحیح neg = 0; /* assume positive */
  صحیح i, j;
  unsigned صحیح c = 0;
  صحیح non_رقم = 0;  /* True if input contains UTF16 with high byte non-zero */
  صحیح rc;          /* Baseline return code */
  ثابت نویسه *zStart;
  ثابت نویسه *zEnd = z_رقم + length;
  أكد( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  اگر( enc==SQLITE_UTF8 ){
    incr = 1;
  }وگرنه{
    incr = 2;
    length &= ~1;
    أكد( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    برای(i=3-enc; i<length && z_رقم[i]==0; i+=2){}
    non_رقم = i<length;
    zEnd = &z_رقم[i^1];
    z_رقم += (enc&1);
  }
  وقتی( z_رقم<zEnd && sqlite3Isspace(*z_رقم) ) z_رقم+=incr;
  اگر( z_رقم<zEnd ){
    اگر( *z_رقم=='-' ){
      neg = 1;
      z_رقم+=incr;
    }وگرنه اگر( *z_رقم=='+' ){
      z_رقم+=incr;
    }
  }
  zStart = z_رقم;
  وقتی( z_رقم<zEnd && z_رقم[0]=='0' ){ z_رقم+=incr; } /* Skip leading zeros. */
  برای(i=0; &z_رقم[i]<zEnd && (c=(unsigned)z_رقم[i]-'0')<=9; i+=incr){
    u = u*10 + c;
  }
  testcase( i==18*incr );
  testcase( i==19*incr );
  testcase( i==20*incr );
  اگر( u>LARGEST_INT64 ){
    /* This test and assignment is needed only to suppress UB warnings
    ** from clang and -fsanitize=undefined.  This test and assignment make
    ** the code a little larger and slower, and no harm comes from omitting
    ** them, but we must appease the undefined-behavior pharisees. */
    *p_رقم = neg ? SMALLEST_INT64 : LARGEST_INT64;
  }وگرنه اگر( neg ){
    *p_رقم = -(i64)u;
  }وگرنه{
    *p_رقم = (i64)u;
  }
  rc = 0;
  اگر( i==0 && zStart==z_رقم ){    /* No digits */
    rc = -1;
  }وگرنه اگر( non_رقم ){            /* UTF16 with high-order bytes non-zero */
    rc = 1;
  }وگرنه اگر( &z_رقم[i]<zEnd ){     /* Extra bytes at the end */
    صحیح jj = i;
    انجامبده{
      اگر( !sqlite3Isspace(z_رقم[jj]) ){
        rc = 1;          /* Extra non-space text after the integer */
        بشکن;
      }
      jj += incr;
    }وقتی( &z_رقم[jj]<zEnd );
  }
  اگر( i<19*incr ){
    /* Less than 19 digits, so we know that it fits in 64 bits */
    أكد( u<=LARGEST_INT64 );
    بازگشت rc;
  }وگرنه{
    /* zNum is a 19-digit numbers.  Compare it against 9223372036854775808. */
    j = i>19*incr ? 1 : compare2pow63(z_رقم, incr);
    اگر( j<0 ){
      /* zNum is less than 9223372036854775808 so it fits */
      أكد( u<=LARGEST_INT64 );
      بازگشت rc;
    }وگرنه{
      *p_رقم = neg ? SMALLEST_INT64 : LARGEST_INT64;
      اگر( j>0 ){
        /* zNum is greater than 9223372036854775808 so it overflows */
        بازگشت 2;
      }وگرنه{
        /* zNum is exactly 9223372036854775808.  Fits if negative.  The
        ** special case 2 overflow if positive */
        أكد( u-1==LARGEST_INT64 );
        بازگشت neg ? rc : 3;
      }
    }
  }
}

/*
** Transform a UTF-8 integer literal, in either decimal or hexadecimal,
** into a 64-bit signed integer.  This routine accepts hexadecimal literals,
** whereas sqlite3Atoi64() does not.
**
** Returns:
**
**     0    Successful transformation.  Fits in a 64-bit signed integer.
**     1    Excess text after the integer value
**     2    Integer too large for a 64-bit signed integer or is malformed
**     3    Special case of 9223372036854775808
*/
صحیح sqlite3DecOrHexToI64(ثابت نویسه *z, i64 *pOut){
#ifndef SQLITE_OMIT_HEX_INTEGER
  اگر( z[0]=='0'
   && (z[1]=='x' || z[1]=='X')
  ){
    u64 u = 0;
    صحیح i, k;
    برای(i=2; z[i]=='0'; i++){}
    برای(k=i; sqlite3Isxdigit(z[k]); k++){
      u = u*16 + sqlite3HexToInt(z[k]);
    }
    memcpy(pOut, &u, 8);
    اگر( k-i>16 ) بازگشت 2;
    اگر( z[k]!=0 ) بازگشت 1;
    بازگشت 0;
  }وگرنه
#endif /* SQLITE_OMIT_HEX_INTEGER */
  {
    صحیح n = (صحیح)(0x3fffffff&strspn(z,"+- \n\t0123456789"));
    اگر( z[n] ) n++;
    بازگشت sqlite3Atoi64(z, pOut, n, SQLITE_UTF8);
  }
}

/*
** If zNum represents an integer that will fit in 32-bits, then set
** *pValue to that integer and return true.  Otherwise return false.
**
** This routine accepts both decimal and hexadecimal notation for integers.
**
** Any non-numeric characters that following zNum are ignored.
** This is different from sqlite3Atoi64() which requires the
** input number to be zero-terminated.
*/
صحیح sqlite3GetInt32(ثابت نویسه *z_رقم, صحیح *p_مقدار){
  sqlite_int64 v = 0;
  صحیح i, c;
  صحیح neg = 0;
  اگر( z_رقم[0]=='-' ){
    neg = 1;
    z_رقم++;
  }وگرنه اگر( z_رقم[0]=='+' ){
    z_رقم++;
  }
#ifndef SQLITE_OMIT_HEX_INTEGER
  وگرنه اگر( z_رقم[0]=='0'
        && (z_رقم[1]=='x' || z_رقم[1]=='X')
        && sqlite3Isxdigit(z_رقم[2])
  ){
    u32 u = 0;
    z_رقم += 2;
    وقتی( z_رقم[0]=='0' ) z_رقم++;
    برای(i=0; i<8 && sqlite3Isxdigit(z_رقم[i]); i++){
      u = u*16 + sqlite3HexToInt(z_رقم[i]);
    }
    اگر( (u&0x80000000)==0 && sqlite3Isxdigit(z_رقم[i])==0 ){
      memcpy(p_مقدار, &u, 4);
      بازگشت 1;
    }وگرنه{
      بازگشت 0;
    }
  }
#endif
  اگر( !sqlite3Isdigit(z_رقم[0]) ) بازگشت 0;
  وقتی( z_رقم[0]=='0' ) z_رقم++;
  برای(i=0; i<11 && (c = z_رقم[i] - '0')>=0 && c<=9; i++){
    v = v*10 + c;
  }

  /* The longest decimal representation of a 32 bit integer is 10 digits:
  **
  **             1234567890
  **     2^31 -> 2147483648
  */
  testcase( i==10 );
  اگر( i>10 ){
    بازگشت 0;
  }
  testcase( v-neg==2147483647 );
  اگر( v-neg>2147483647 ){
    بازگشت 0;
  }
  اگر( neg ){
    v = -v;
  }
  *p_مقدار = (صحیح)v;
  بازگشت 1;
}

/*
** Return a 32-bit integer value extracted from a string.  If the
** string is not an integer, just return 0.
*/
صحیح sqlite3Atoi(ثابت نویسه *z){
  صحیح x = 0;
  sqlite3GetInt32(z, &x);
  بازگشت x;
}

/*
** Decode a floating-point value into an approximate decimal
** representation.
**
** If iRound<=0 then round to -iRound significant digits to the
** the right of the decimal point, or to a maximum of mxRound total
** significant digits.
**
** If iRound>0 round to min(iRound,mxRound) significant digits total.
**
** mxRound must be positive.  iRound must not be less than -100000000
** (the SQLITE_FP_PRECISION_LIMIT maximum in printf.c) so that the
** computation of the number of significant digits cannot overflow.
**
** The significant digits of the decimal representation are
** stored in p->z[] which is a often (but not always) a pointer
** into the middle of p->zBuf[].  There are p->n significant digits.
** The p->z[] array is *not* zero-terminated.
*/
تهی sqlite3FpDecode(FpDecode *p, دوگانه r, صحیح iRound, صحیح mxRound){
  صحیح i;               /* Index into zBuf[] where to put next character */
  صحیح n;               /* Number of digits */
  u64 v;               /* mantissa */
  صحیح e, exp = 0;      /* Base-2 and base-10 exponent */
  نویسه *zBuf;          /* Local alias for p->zBuf */
  نویسه *z;             /* Local alias for p->z */

  p->isSpecial = 0;
  أكد( mxRound>0 );
  أكد( iRound>=-100000000 );

  /* Convert negative numbers to positive.  Deal with Infinity, 0.0, and
  ** NaN. */
  اگر( r<0.0 ){
    p->sign = '-';
    r = -r;
  }وگرنه اگر( r==0.0 ){
    p->sign = '+';
    p->n = 1;
    p->iDP = 1;
    p->z = "0";
    بازگشت;
  }وگرنه{
    p->sign = '+';
  }
  memcpy(&v,&r,8);
  e = (v>>52)&0x7ff;
  اگر( e==0x7ff ){
    p->isSpecial = 1 + (v!=0x7ff0000000000000LL);
    p->n = 0;
    p->iDP = 0;
    p->z = p->zBuf;
    بازگشت;
  }
  v &= 0x000fffffffffffffULL;
  اگر( e==0 ){
    صحیح nn = شمار_Leading_Zeros(v);
    v <<= nn;
    e = -1074 - nn;
  }وگرنه{
    v = (v<<11) | U64_BIT(63);
    e -= 1086;
  }
  sqlite3Fp2Convert10(v, e, (iRound<=0||iRound>=18)?18:iRound+1, &v, &exp);  

  /* Extract significant digits, start at the right-most slot in p->zBuf
  ** and working back to the right.  "i" keeps track of the next slot in
  ** which to store a digit. */
  أكد( اندازه(p->zBuf)==SQLITE_U64_DIGITS+1 );
  أكد( v>0 );
  zBuf = p->zBuf;
#ifdef SQLITE_AVOID_U64_DIVIDE
  i = sqlite3UInt64ToText(v, zBuf);
#وگرنه
  i = SQLITE_U64_DIGITS;
  وقتی( v>=10 ){
    صحیح kk = (v%100)*2;
    أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk]) );
    أكد( TWO_BYTE_ALIGNMENT(&zBuf[i]) );
    أكد( i-2>=0 );
    *(u16*)(&zBuf[i-2]) = *(u16*)&sqlite3DigitPairs.a[kk];
    i -= 2;
    v /= 100;
  }
  اگر( v ){
    أكد( v<10 );
    أكد( i>0 );
    zBuf[--i] = v + '0';
  }
#endif /* SQLITE_AVOID_U64_DIVIDE */
  أكد( i>=0 && i<SQLITE_U64_DIGITS );
  n = SQLITE_U64_DIGITS - i;  /* Total number of digits extracted */
  أكد( n>0 );
  أكد( n<=SQLITE_U64_DIGITS );
  p->iDP = n + exp;
  اگر( iRound<=0 ){
    iRound = p->iDP - iRound;
    اگر( iRound==0 && zBuf[i]>='5' ){
      iRound = 1;
      zBuf[--i] = '0';
      n++;
      p->iDP++;
    }
  }
  z = &zBuf[i];  /* z points to the first digit */
  اگر( iRound>0 && (iRound<n || n>mxRound) ){
    اگر( iRound>mxRound ) iRound = mxRound;
    اگر( iRound==17 ){
      /* If the precision is exactly 17, which only happens with the "!"
      ** flag (ex: "%!.17g") then try to reduce the precision if that
      ** yields text that will round-trip to the original floating-point.
      ** value.  Thus, for exaple, 49.47 will render as 49.47, rather than
      ** as 49.469999999999999. */
      اگر( z[15]=='9' && z[14]=='9' ){
        صحیح jj, kk;
        u64 v2;
        برای(jj=14; jj>0 && z[jj-1]=='9'; jj--){}
        اگر( jj==0 ){
          v2 = 1;
        }وگرنه{
          v2 = z[0] - '0';
          برای(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
          v2++;
        }
        اگر( r==sqlite3Fp10Convert2(v2, exp + n - jj) ){
          iRound = jj+1;
        }
      }وگرنه اگر( p->iDP>=n || (z[15]=='0' && z[14]=='0' && z[13]=='0') ){
        صحیح jj, kk;
        u64 v2;
        أكد( z[0]!='0' );
        برای(jj=13; z[jj-1]=='0'; jj--){}
        v2 = z[0] - '0';
        برای(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
        اگر( r==sqlite3Fp10Convert2(v2, exp + n - jj) ){
          iRound = jj+1;
        }
      }
    }
    n = iRound;
    اگر( z[iRound]>='5' ){
      صحیح j = iRound-1;
      وقتی( 1 /*exit-by-break*/ ){
        z[j]++;
        اگر( z[j]<='9' ) بشکن;
        z[j] = '0';
        اگر( j==0 ){
          z--;
          z[0] = '1';
          n++;
          p->iDP++;
          بشکن;
        }وگرنه{
          j--;
        }
      }
    }
  }
  أكد( n>0 );
  وقتی( z[n-1]=='0' ){
    n--;
    أكد( n>0 );
  }
  p->n = n;
  p->z = z;
}

/*
** Try to convert z into an unsigned 32-bit integer.  Return true on
** success and false if there is an error.
**
** Only decimal notation is accepted.
*/
صحیح sqlite3GetUInt32(ثابت نویسه *z, u32 *pI){
  u64 v = 0;
  صحیح i;
  برای(i=0; sqlite3Isdigit(z[i]); i++){
    v = v*10 + z[i] - '0';
    اگر( v>4294967295LL ){ *pI = 0; بازگشت 0; }
  }
  اگر( i==0 || z[i]!=0 ){ *pI = 0; بازگشت 0; }
  *pI = (u32)v;
  بازگشت 1;
}

/*
** The variable-length integer encoding is as follows:
**
** KEY:
**         A = 0xxxxxxx    7 bits of data and one flag bit
**         B = 1xxxxxxx    7 bits of data and one flag bit
**         C = xxxxxxxx    8 bits of data
**
**  7 bits - A
** 14 bits - BA
** 21 bits - BBA
** 28 bits - BBBA
** 35 bits - BBBBA
** 42 bits - BBBBBA
** 49 bits - BBBBBBA
** 56 bits - BBBBBBBA
** 64 bits - BBBBBBBBC
*/

/*
** Write a 64-bit variable-length integer to memory starting at p[0].
** The length of data write will be between 1 and 9 bytes.  The number
** of bytes written is returned.
**
** A variable-length integer consists of the lower 7 bits of each byte
** for all bytes that have the 8th bit set and one byte with the 8th
** bit clear.  Except, if we get to the 9th byte, it stores the full
** 8 bits and is the last byte.
*/
ایستا صحیح SQLITE_NOINLINE putVarint64(unsigned نویسه *p, u64 v){
  صحیح i, j, n;
  u8 buf[10];
  اگر( v & (((u64)0xff000000)<<32) ){
    p[8] = (u8)v;
    v >>= 8;
    برای(i=7; i>=0; i--){
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    بازگشت 9;
  }   
  n = 0;
  انجامبده{
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  }وقتی( v!=0 );
  buf[0] &= 0x7f;
  أكد( n<=9 );
  برای(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  بازگشت n;
}
صحیح sqlite3PutVarint(unsigned نویسه *p, u64 v){
  اگر( v<=0x7f ){
    p[0] = v&0x7f;
    بازگشت 1;
  }
  اگر( v<=0x3fff ){
    p[0] = ((v>>7)&0x7f)|0x80;
    p[1] = v&0x7f;
    بازگشت 2;
  }
  بازگشت putVarint64(p,v);
}

/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
u8 sqlite3GetVarint(ثابت unsigned نویسه *p, u64 *v){
  u64 i_کلید = p[0];
  ثابت u8 *pStart = p;
  اگر( i_کلید>=0x80 ){
    u8 x;
    i_کلید = (i_کلید<<7) ^ (x = *++p);
    اگر( x>=0x80 ){
      i_کلید = (i_کلید<<7) ^ (x = *++p);
      اگر( x>=0x80 ){
        i_کلید = (i_کلید<<7) ^ 0x10204000 ^ (x = *++p);
        اگر( x>=0x80 ){
          i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
          اگر( x>=0x80 ){
            i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
            اگر( x>=0x80 ){
              i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
              اگر( x>=0x80 ){
                i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
                اگر( x>=0x80 ){
                  i_کلید = (i_کلید<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }وگرنه{
        i_کلید ^= 0x204000;
      }
    }وگرنه{
      i_کلید ^= 0x4000;
    }
  }
  *v = i_کلید;
  بازگشت (u8)(p - pStart) + 1;
}

/*
** Return the value of a variable-length integer without computing
** its length.  This is an optimization on sqlite3GetVarint() for the
** cases when the return value of sqlite3GetVarint() is not needed.
*/
i64 sqlite3_Varint_مقدار(ثابت unsigned نویسه *p){
  u64 i_کلید = p[0];
  اگر( i_کلید>=0x80 ){
    u8 x;
    i_کلید = (i_کلید<<7) ^ (x = *++p);
    اگر( x>=0x80 ){
      i_کلید = (i_کلید<<7) ^ (x = *++p);
      اگر( x>=0x80 ){
        i_کلید = (i_کلید<<7) ^ 0x10204000 ^ (x = *++p);
        اگر( x>=0x80 ){
          i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
          اگر( x>=0x80 ){
            i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
            اگر( x>=0x80 ){
              i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
              اگر( x>=0x80 ){
                i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++p);
                اگر( x>=0x80 ){
                  i_کلید = (i_کلید<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }وگرنه{
        i_کلید ^= 0x204000;
      }
    }وگرنه{
      i_کلید ^= 0x4000;
    }
  }
  بازگشت *(i64*)&i_کلید;
}

/*
** Read a 32-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
**
** If the varint stored in p[0] is larger than can fit in a 32-bit unsigned
** integer, then set *v to 0xffffffff.
**
** A MACRO version, getVarint32, is provided which inlines the
** single-byte case.  All code should use the MACRO version as
** this function assumes the single-byte case has already been handled.
*/
u8 sqlite3GetVarint32(ثابت unsigned نویسه *p, u32 *v){
  u64 v64;
  u8 n;

  /* Assume that the single-byte case has already been handled by
  ** the getVarint32() macro */
  أكد( (p[0] & 0x80)!=0 );

  اگر( (p[1] & 0x80)==0 ){
    /* This is the two-byte case */
    *v = ((p[0]&0x7f)<<7) | p[1];
    بازگشت 2;
  }
  اگر( (p[2] & 0x80)==0 ){
    /* This is the three-byte case */
    *v = ((p[0]&0x7f)<<14) | ((p[1]&0x7f)<<7) | p[2];
    بازگشت 3;
  }
  /* four or more bytes */
  n = sqlite3GetVarint(p, &v64);
  أكد( n>3 && n<=9 );
  اگر( (v64 & SQLITE_MAX_U32)!=v64 ){
    *v = 0xffffffff;
  }وگرنه{
    *v = (u32)v64;
  }
  بازگشت n;
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
صحیح sqlite3VarintLen(u64 v){
  صحیح i;
  برای(i=1; (v >>= 7)!=0; i++){ أكد( i<10 ); }
  بازگشت i;
}


/*
** Read an unsigned 32-bit integer from an unaligned big-endian array of bytes.
*/
u32 sqlite3Get4byte(ثابت u8 *p){
#اگر SQLITE_BYTEORDER==4321
  u32 x;
  memcpy(&x,p,4);
  بازگشت x;
#وگرنهاگر SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u32 x;
  memcpy(&x,p,4);
  بازگشت __builtin_bswap32(x);
#وگرنهاگر SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u32 x;
  memcpy(&x,p,4);
  بازگشت _byteswap_ulong(x);
#وگرنه
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  بازگشت ((unsigned)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
#endif
}

/* Write an unsigned 32-bit integer into an unaligned big-endian array of bytes.
*/
تهی sqlite3Put4byte(unsigned نویسه *p, u32 v){
#اگر SQLITE_BYTEORDER==4321
  memcpy(p,&v,4);
#وگرنهاگر SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u32 x = __builtin_bswap32(v);
  memcpy(p,&x,4);
#وگرنهاگر SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u32 x = _byteswap_ulong(v);
  memcpy(p,&x,4);
#وگرنه
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  p[0] = (u8)(v>>24);
  p[1] = (u8)(v>>16);
  p[2] = (u8)(v>>8);
  p[3] = (u8)v;
#endif
}

/*
** Read an unsigned 64-bit integer from an unaligned big-endian byte array.
*/
SQLITE_OPT_INLINE u64 sqlite3Get8byte(ثابت u8 *p){
#اگر SQLITE_BYTEORDER==4321
  u64 x;
  memcpy(&x,p,8);
  بازگشت x;
#وگرنهاگر SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u64 x;
  memcpy(&x,p,8);
  بازگشت __builtin_bswap64(x);
#وگرنهاگر SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u64 x;
  memcpy(&x,p,8);
  بازگشت _byteswap_uint64(x);
#وگرنه
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  بازگشت (u64)(
    (((u64)p[0]) << 56) +
    (((u64)p[1]) << 48) +
    (((u64)p[2]) << 40) +
    (((u64)p[3]) << 32) +
    (((u64)p[4]) << 24) +
    (((u64)p[5]) << 16) +
    (((u64)p[6]) <<  8) +
    (((u64)p[7]) <<  0)
  );
#endif
}

#اگر SQLITE_BYTEORDER!=4321  /* Only used for little-endian machines */
/*
** Byte-swap a 64-bit unsigned integer.
*/
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64 x){
#اگر SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  بازگشت __builtin_bswap64(x);
#وگرنهاگر SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  بازگشت _byteswap_uint64(x);
#وگرنه
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  x = (x << 32) | (x >> 32);
  x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) |
      ((x & UINT64_C(0xffff0000ffff0000)) >> 16);
  x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) |
      ((x & UINT64_C(0xff00ff00ff00ff00)) >>  8);
  بازگشت x;
#endif
}
#endif /* SQLITE_BYTEORDER!=4321 */

/*
** Translate a single byte of Hex into an integer.
** This routine only works if h really is a valid hexadecimal
** character:  0..9a..fA..F
*/
u8 sqlite3HexToInt(صحیح h){
  أكد( (h>='0' && h<='9') ||  (h>='a' && h<='f') ||  (h>='A' && h<='F') );
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  بازگشت (u8)(h & 0xf);
}

#اگر !defined(SQLITE_OMIT_BLOB_LITERAL)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
تهی *sqlite3HexToBlob(sqlite3 *db, ثابت نویسه *z, صحیح n){
  نویسه *zBlob;
  صحیح i;

  zBlob = (نویسه *)sqlite3DbMallocRawNN(db, n/2 + 1);
  n--;
  اگر( zBlob ){
    برای(i=0; i<n; i+=2){
      zBlob[i/2] = (sqlite3HexToInt(z[i])<<4) | sqlite3HexToInt(z[i+1]);
    }
    zBlob[i/2] = 0;
  }
  بازگشت zBlob;
}
#endif /* !SQLITE_OMIT_BLOB_LITERAL */

/*
** Log an error that is an API call on a connection pointer that should
** not have been used.  The "type" of connection pointer is given as the
** argument.  The zType is a word like "NULL" or "closed" or "invalid".
*/
ایستا تهی logBadConnection(ثابت نویسه *zType){
  sqlite3_log(SQLITE_MISUSE,
     "API call with %s database connection pointer",
     zType
  );
}

/*
** Check to make sure we have a valid db pointer.  This test is not
** foolproof but it does provide some measure of protection against
** misuse of the interface such as passing in db pointers that are
** NULL or which have been previously closed.  If this routine returns
** 1 it means that the db pointer is valid and 0 if it should not be
** dereferenced for any reason.  The calling function should invoke
** SQLITE_MISUSE immediately.
**
** sqlite3SafetyCheckOk() requires that the db pointer be valid for
** use.  sqlite3SafetyCheckSickOrOk() allows a db pointer that failed to
** open properly and is not fit for general use but which can be
** used as an argument to sqlite3_errmsg() or sqlite3_close().
*/
صحیح sqlite3SafetyCheckOk(sqlite3 *db){
  u8 eOpenState;
  اگر( db==0 ){
    logBadConnection("NULL");
    بازگشت 0;
  }
  eOpenState = db->eOpenState;
  اگر( eOpenState!=SQLITE_STATE_OPEN ){
    اگر( sqlite3SafetyCheckSickOrOk(db) ){
      testcase( sqlite3GlobalConfig.xLog!=0 );
      logBadConnection("unopened");
    }
    بازگشت 0;
  }وگرنه{
    بازگشت 1;
  }
}
صحیح sqlite3SafetyCheckSickOrOk(sqlite3 *db){
  u8 eOpenState;
  eOpenState = db->eOpenState;
  اگر( eOpenState!=SQLITE_STATE_SICK &&
      eOpenState!=SQLITE_STATE_OPEN &&
      eOpenState!=SQLITE_STATE_BUSY ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    logBadConnection("invalid");
    بازگشت 0;
  }وگرنه{
    بازگشت 1;
  }
}

/*
** Attempt to add, subtract, or multiply the 64-bit signed value iB against
** the other 64-bit signed integer at *pA and store the result in *pA.
** Return 0 on success.  Or if the operation would have resulted in an
** overflow, leave *pA unchanged and return 1.
*/
صحیح sqlite3AddInt64(i64 *pA, i64 iB){
#اگر GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  بازگشت __builtin_add_overflow(*pA, iB, pA);
#وگرنه
  i64 iA = *pA;
  testcase( iA==0 ); testcase( iA==1 );
  testcase( iB==-1 ); testcase( iB==0 );
  اگر( iB>=0 ){
    testcase( iA>0 && LARGEST_INT64 - iA == iB );
    testcase( iA>0 && LARGEST_INT64 - iA == iB - 1 );
    اگر( iA>0 && LARGEST_INT64 - iA < iB ) بازگشت 1;
  }وگرنه{
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 1 );
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 2 );
    اگر( iA<0 && -(iA + LARGEST_INT64) > iB + 1 ) بازگشت 1;
  }
  *pA += iB;
  بازگشت 0;
#endif
}
صحیح sqlite3SubInt64(i64 *pA, i64 iB){
#اگر GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  بازگشت __builtin_sub_overflow(*pA, iB, pA);
#وگرنه
  testcase( iB==SMALLEST_INT64+1 );
  اگر( iB==SMALLEST_INT64 ){
    testcase( (*pA)==(-1) ); testcase( (*pA)==0 );
    اگر( (*pA)>=0 ) بازگشت 1;
    *pA -= iB;
    بازگشت 0;
  }وگرنه{
    بازگشت sqlite3AddInt64(pA, -iB);
  }
#endif
}
صحیح sqlite3MulInt64(i64 *pA, i64 iB){
#اگر GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  بازگشت __builtin_mul_overflow(*pA, iB, pA);
#وگرنه
  i64 iA = *pA;
  اگر( iB>0 ){
    اگر( iA>LARGEST_INT64/iB ) بازگشت 1;
    اگر( iA<SMALLEST_INT64/iB ) بازگشت 1;
  }وگرنه اگر( iB<0 ){
    اگر( iA>0 ){
      اگر( iB<SMALLEST_INT64/iA ) بازگشت 1;
    }وگرنه اگر( iA<0 ){
      اگر( iB==SMALLEST_INT64 ) بازگشت 1;
      اگر( iA==SMALLEST_INT64 ) بازگشت 1;
      اگر( -iA>LARGEST_INT64/-iB ) بازگشت 1;
    }
  }
  *pA = iA*iB;
  بازگشت 0;
#endif
}

/*
** Compute the absolute value of a 32-bit signed integer, if possible.  Or
** if the integer has a value of -2147483648, return +2147483647
*/
صحیح sqlite3AbsInt32(صحیح x){
  اگر( x>=0 ) بازگشت x;
  اگر( x==(صحیح)0x80000000 ) بازگشت 0x7fffffff;
  بازگشت -x;
}

#ifdef SQLITE_ENABLE_8_3_NAMES
/*
** If SQLITE_ENABLE_8_3_NAMES is set at compile-time and if the database
** filename in zBaseFilename is a URI with the "8_3_names=1" parameter and
** if filename in z[] has a suffix (a.k.a. "extension") that is longer than
** three characters, then shorten the suffix on z[] to be the last three
** characters of the original suffix.
**
** If SQLITE_ENABLE_8_3_NAMES is set to 2 at compile-time, then always
** do the suffix shortening regardless of URI parameter.
**
** Examples:
**
**     test.db-journal    =>   test.nal
**     test.db-wal        =>   test.wal
**     test.db-shm        =>   test.shm
**     test.db-mj7f3319fa =>   test.9fa
*/
تهی sqlite3FileSuffix3(ثابت نویسه *zBaseFilename, نویسه *z){
#اگر SQLITE_ENABLE_8_3_NAMES<2
  اگر( sqlite3_uri_boolean(zBaseFilename, "8_3_names", 0) )
#endif
  {
    صحیح i, sz;
    sz = sqlite3Strlen30(z);
    برای(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    اگر( z[i]=='.' && ALWAYS(sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
  }
}
#endif

/*
** Find (an approximate) sum of two LogEst values.  This computation is
** not a simple "+" operator because LogEst is stored as a logarithmic
** value.
**
*/
LogEst sqlite3LogEstAdd(LogEst a, LogEst b){
  ایستا ثابت unsigned نویسه x[] = {
     10, 10,                         /* 0,1 */
      9, 9,                          /* 2,3 */
      8, 8,                          /* 4,5 */
      7, 7, 7,                       /* 6,7,8 */
      6, 6, 6,                       /* 9,10,11 */
      5, 5, 5,                       /* 12-14 */
      4, 4, 4, 4,                    /* 15-18 */
      3, 3, 3, 3, 3, 3,              /* 19-24 */
      2, 2, 2, 2, 2, 2, 2,           /* 25-31 */
  };
  اگر( a>=b ){
    اگر( a>b+49 ) بازگشت a;
    اگر( a>b+31 ) بازگشت a+1;
    بازگشت a+x[a-b];
  }وگرنه{
    اگر( b>a+49 ) بازگشت b;
    اگر( b>a+31 ) بازگشت b+1;
    بازگشت b+x[b-a];
  }
}

/*
** Convert an integer into a LogEst.  In other words, compute an
** approximation for 10*log2(x).
*/
LogEst sqlite3LogEst(u64 x){
  ایستا LogEst a[] = { 0, 2, 3, 5, 6, 7, 8, 9 };
  LogEst y = 40;
  اگر( x<8 ){
    اگر( x<2 ) بازگشت 0;
    وقتی( x<8 ){  y -= 10; x <<= 1; }
  }وگرنه{
#اگر GCC_VERSION>=5004000
    صحیح i = 60 - __builtin_clzll(x);
    y += i*10;
    x >>= i;
#وگرنه
    وقتی( x>255 ){ y += 40; x >>= 4; }  /*OPTIMIZATION-IF-TRUE*/
    وقتی( x>15 ){  y += 10; x >>= 1; }
#endif
  }
  بازگشت a[x&7] + y - 10;
}

/*
** Convert a double into a LogEst
** In other words, compute an approximation for 10*log2(x).
*/
LogEst sqlite3LogEstFromDouble(دوگانه x){
  u64 a;
  LogEst e;
  أكد( اندازه(x)==8 && اندازه(a)==8 );
  اگر( x<=1 ) بازگشت 0;
  اگر( x<=2000000000 ) بازگشت sqlite3LogEst((u64)x);
  memcpy(&a, &x, 8);
  e = (a>>52) - 1022;
  بازگشت e*10;
}

/*
** Convert a LogEst into an integer.
*/
u64 sqlite3LogEstToInt(LogEst x){
  u64 n;
  n = x%10;
  x /= 10;
  اگر( n>=5 ) n -= 2;
  وگرنه اگر( n>=1 ) n -= 1;
  اگر( x>60 ) بازگشت (u64)LARGEST_INT64;
  بازگشت x>=3 ? (n+8)<<(x-3) : (n+8)>>(3-x);
}

/*
** Add a new name/number pair to a VList.  This might require that the
** VList object be reallocated, so return the new VList.  If an OOM
** error occurs, the original VList returned and the
** db->mallocFailed flag is set.
**
** A VList is really just an array of integers.  To destroy a VList,
** simply pass it to sqlite3DbFree().
**
** The first integer is the number of integers allocated for the whole
** VList.  The second integer is the number of integers actually used.
** Each name/number pair is encoded by subsequent groups of 3 or more
** integers.
**
** Each name/number pair starts with two integers which are the numeric
** value for the pair and the size of the name/number pair, respectively.
** The text name overlays one or more following integers.  The text name
** is always zero-terminated.
**
** Conceptually:
**
**    struct VList {
**      int nAlloc;   // Number of allocated slots
**      int nUsed;    // Number of used slots
**      struct VListEntry {
**        int iValue;    // Value for this entry
**        int nSlot;     // Slots used by this entry
**        // ... variable name goes here
**      } a[0];
**    }
**
** During code generation, pointers to the variable names within the
** VList are taken.  When that happens, nAlloc is set to zero as an
** indication that the VList may never again be enlarged, since the
** accompanying realloc() would invalidate the pointers.
*/
VList *sqlite3VListAdd(
  sqlite3 *db,           /* The database connection used for malloc() */
  VList *pIn,            /* The input VList.  Might be NULL */
  ثابت نویسه *zName,     /* Name of symbol to add */
  صحیح nName,             /* Bytes of text in zName */
  صحیح iVal               /* Value to associate with zName */
){
  صحیح nInt;              /* number of sizeof(int) objects needed for zName */
  نویسه *z;               /* Pointer to where zName will be stored */
  صحیح i;                 /* Index in pIn[] where zName is stored */

  nInt = nName/4 + 3;
  أكد( pIn==0 || pIn[0]>=3 );  /* Verify ok to add new elements */
  اگر( pIn==0 || pIn[1]+nInt > pIn[0] ){
    /* Enlarge the allocation */
    sqlite3_int64 nAlloc = (pIn ? 2*(sqlite3_int64)pIn[0] : 10) + nInt;
    VList *pOut = sqlite3DbRealloc(db, pIn, nAlloc*اندازه(صحیح));
    اگر( pOut==0 ) بازگشت pIn;
    اگر( pIn==0 ) pOut[1] = 2;
    pIn = pOut;
    pIn[0] = nAlloc;
  }
  i = pIn[1];
  pIn[i] = iVal;
  pIn[i+1] = nInt;
  z = (نویسه*)&pIn[i+2];
  pIn[1] = i+nInt;
  أكد( pIn[1]<=pIn[0] );
  memcpy(z, zName, nName);
  z[nName] = 0;
  بازگشت pIn;
}

/*
** Return a pointer to the name of a variable in the given VList that
** has the value iVal.  Or return a NULL if there is no such variable in
** the list
*/
ثابت نویسه *sqlite3_VList_رقم_To_Name(VList *pIn, صحیح iVal){
  صحیح i, mx;
  اگر( pIn==0 ) بازگشت 0;
  mx = pIn[1];
  i = 2;
  انجامبده{
    اگر( pIn[i]==iVal ) بازگشت (نویسه*)&pIn[i+2];
    i += pIn[i+1];
  }وقتی( i<mx );
  بازگشت 0;
}

/*
** Return the number of the variable named zName, if it is in VList.
** or return 0 if there is no such variable.
*/
صحیح sqlite3_VList_Name_To_رقم(VList *pIn, ثابت نویسه *zName, صحیح nName){
  صحیح i, mx;
  اگر( pIn==0 ) بازگشت 0;
  mx = pIn[1];
  i = 2;
  انجامبده{
    ثابت نویسه *z = (ثابت نویسه*)&pIn[i+2];
    اگر( strncmp(z,zName,nName)==0 && z[nName]==0 ) بازگشت pIn[i];
    i += pIn[i+1];
  }وقتی( i<mx );
  بازگشت 0;
}
