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
#ضمن "sqliteInt.h"
#ضمن <stdarg.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#ضمن <math.h>

/* Work around a bug in older Microsoft compilers
** Forum post 2026-04-10T06:33:11z */
#إذا !defined(INFINITY) && defined(_MSC_VER)
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
صحيح sqlite3FaultSim(صحيح iTest){
  صحيح (*xCallback)(صحيح) = sqlite3GlobalConfig.xTestCallback;
  أرجع xCallback ? xCallback(iTest) : SQLITE_OK;
}
#endif

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is Not a Number (NaN).
**
** Use the math library isnan() function if compiled with SQLITE_HAVE_ISNAN.
** Otherwise, we have our own implementation that works on most systems.
*/
صحيح sqlite3IsNaN(مزدوج x){
  صحيح rc;   /* The value return */
#إذا !SQLITE_HAVE_ISNAN && !HAVE_ISNAN
  u64 y;
  memcpy(&y,&x,حجم(y));
  rc = IsNaN(y);
#وإلا
  rc = isnan(x);
#endif /* HAVE_ISNAN */
  testcase( rc );
  أرجع rc;
}
#endif /* SQLITE_OMIT_FLOATING_POINT */

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is NaN or +Inf or -Inf.
*/
صحيح sqlite3IsOverflow(مزدوج x){
  صحيح rc;   /* The value return */
  u64 y;
  memcpy(&y,&x,حجم(y));
  rc = IsOvfl(y);
  أرجع rc;
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
صحيح sqlite3Strlen30(ثابت حرف *z){
  إذا( z==0 ) أرجع 0;
  أرجع 0x3fffffff & (صحيح)strlen(z);
}

/*
** Return the declared type of a column.  Or return zDflt if the column
** has no declared type.
**
** The column type is an extra string stored after the zero-terminator on
** the column name if and only if the COLFLAG_HASTYPE flag is set.
*/
حرف *sqlite3ColumnType(Column *pCol, حرف *zDflt){
  إذا( pCol->colFlags & COLFLAG_HASTYPE ){
    أرجع pCol->zCnName + strlen(pCol->zCnName) + 1;
  }وإلا إذا( pCol->eCType ){
    أكد( pCol->eCType<=SQLITE_N_STDTYPE );
    أرجع (حرف*)sqlite3StdType[pCol->eCType-1];
  }وإلا{
    أرجع zDflt;
  }
}

/*
** Helper function for sqlite3Error() - called rarely.  Broken out into
** a separate routine to avoid unnecessary register saves on entry to
** sqlite3Error().
*/
ساكن SQLITE_NOINLINE فراغ  sqlite3ErrorFinish(sqlite3 *db, صحيح err_code){
  إذا( db->pErr ) sqlite3_قيمة_Set_Null(db->pErr);
  sqlite3SystemError(db, err_code);
}

/*
** Set the current error code to err_code and clear any prior error message.
** Also set iSysErrno (by calling sqlite3System) if the err_code indicates
** that would be appropriate.
*/
فراغ sqlite3Error(sqlite3 *db, صحيح err_code){
  أكد( db!=0 );
  db->errCode = err_code;
  إذا( err_code || db->pErr ){
    sqlite3ErrorFinish(db, err_code);
  }وإلا{
    db->errByteOffset = -1;
  }
}

/*
** The equivalent of sqlite3Error(db, SQLITE_OK).  Clear the error state
** and error message.
*/
فراغ sqlite3ErrorClear(sqlite3 *db){
  أكد( db!=0 );
  db->errCode = SQLITE_OK;
  db->errByteOffset = -1;
  إذا( db->pErr ) sqlite3_قيمة_Set_Null(db->pErr);
}

/*
** Load the sqlite3.iSysErrno field if that is an appropriate thing
** to do based on the SQLite error code in rc.
*/
فراغ sqlite3SystemError(sqlite3 *db, صحيح rc){
  إذا( rc==SQLITE_IOERR_NOMEM ) أرجع;
#إذا defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
  إذا( rc==SQLITE_IOERR_IN_PAGE ){
    صحيح ii;
    صحيح iErr;
    sqlite3BtreeEnterAll(db);
    لكل(ii=0; ii<db->nDb; ii++){
      إذا( db->aDb[ii].pBt ){
        iErr = sqlite3PagerWalSystemErrno(sqlite3BtreePager(db->aDb[ii].pBt));
        إذا( iErr ){
          db->iSysErrno = iErr;
        }
      }
    }
    sqlite3BtreeLeaveAll(db);
    أرجع;
  }
#endif
  rc &= 0xff;
  إذا( rc==SQLITE_CANTOPEN || rc==SQLITE_IOERR ){
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
فراغ sqlite3_Error_With_رسالة(sqlite3 *db, صحيح err_code, ثابت حرف *zFormat, ...){
  أكد( db!=0 );
  db->errCode = err_code;
  sqlite3SystemError(db, err_code);
  إذا( zFormat==0 ){
    sqlite3Error(db, err_code);
  }وإلا إذا( db->pErr || (db->pErr = sqlite3_قيمة_New(db))!=0 ){
    حرف *z;
    va_list ap;
    va_start(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);
    va_end(ap);
    sqlite3_قيمة_Set_Str(db->pErr, -1, z, SQLITE_UTF8, SQLITE_DYNAMIC);
  }
}

/*
** Check for interrupts and invoke progress callback.
*/
فراغ sqlite3ProgressCheck(Parse *p){
  sqlite3 *db = p->db;
  إذا( AtomicLoad(&db->u1.isInterrupted) ){
    p->nErr++;
    p->rc = SQLITE_INTERRUPT;
  }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  إذا( db->xProgress ){
    إذا( p->rc==SQLITE_INTERRUPT ){
      p->nProgressSteps = 0;
    }وإلا إذا( (++p->nProgressSteps)>=db->nProgressOps ){
      إذا( db->xProgress(db->pProgressArg) ){
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
فراغ sqlite3_Error_رسالة(Parse *pParse, ثابت حرف *zFormat, ...){
  حرف *z_رسالة;
  va_list ap;
  sqlite3 *db = pParse->db;
  أكد( db!=0 );
  أكد( db->pParse==pParse || db->pParse->pToplevel==pParse );
  db->errByteOffset = -2;
  va_start(ap, zFormat);
  z_رسالة = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  إذا( db->errByteOffset<-1 ) db->errByteOffset = -1;
  إذا( db->suppressErr ){
    sqlite3DbFree(db, z_رسالة);
    إذا( db->mallocFailed ){
      pParse->nErr++;
      pParse->rc = SQLITE_NOMEM;
    }
  }وإلا{
    pParse->nErr++;
    sqlite3DbFree(db, pParse->z_Err_رسالة);
    pParse->z_Err_رسالة = z_رسالة;
    pParse->rc = SQLITE_ERROR;
    pParse->pWith = 0;
  }
}

/*
** If database connection db is currently parsing SQL, then transfer
** error code errCode to that parser if the parser has not already
** encountered some other kind of error.
*/
صحيح sqlite3ErrorToParser(sqlite3 *db, صحيح errCode){
  Parse *pParse;
  إذا( db==0 || (pParse = db->pParse)==0 ) أرجع errCode;
  pParse->rc = errCode;
  pParse->nErr++;
  أرجع errCode;
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
فراغ sqlite3Dequote(حرف *z){
  حرف quote;
  صحيح i, j;
  إذا( z==0 ) أرجع;
  quote = z[0];
  إذا( !sqlite3Isquote(quote) ) أرجع;
  إذا( quote=='[' ) quote = ']';
  لكل(i=1, j=0;; i++){
    أكد( z[i] );
    إذا( z[i]==quote ){
      إذا( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }وإلا{
        اكسر;
      }
    }وإلا{
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}
فراغ sqlite3_Dequote_تعبير(تعبير *p){
  أكد( !تعبير_Has_Property(p, EP_Int_قيمة) );
  أكد( sqlite3Isquote(p->u.zToken[0]) );
  p->flags |= p->u.zToken[0]=='"' ? EP_Quoted|EP_DblQuoted : EP_Quoted;
  sqlite3Dequote(p->u.zToken);
}

/*
** Expression p is a QNUMBER (quoted number). Dequote the value in p->u.zToken
** and set the type to INTEGER or FLOAT. "Quoted" integers or floats are those
** that contain '_' characters that must be removed before further processing.
*/
فراغ sqlite3_Dequote_رقم(Parse *pParse, تعبير *p){
  أكد( p!=0 || pParse->db->mallocFailed );
  إذا( p ){
    ثابت حرف *pIn = p->u.zToken;
    حرف *pOut = p->u.zToken;
    صحيح bHex = (pIn[0]=='0' && (pIn[1]=='x' || pIn[1]=='X'));
    صحيح i_قيمة;
    أكد( p->op==TK_QNUMBER );
    p->op = TK_INTEGER;
    افعل {
      إذا( *pIn!=SQLITE_DIGIT_SEPARATOR ){
        *pOut++ = *pIn;
        إذا( *pIn=='e' || *pIn=='E' || *pIn=='.' ) p->op = TK_FLOAT;
      }وإلا{
        إذا( (bHex==0 && (!sqlite3Isdigit(pIn[-1]) || !sqlite3Isdigit(pIn[1])))
         || (bHex==1 && (!sqlite3Isxdigit(pIn[-1]) || !sqlite3Isxdigit(pIn[1])))
        ){
          sqlite3_Error_رسالة(pParse, "unrecognized token: \"%s\"", p->u.zToken);
        }
      }
    }طالما( *pIn++ );
    إذا( bHex ) p->op = TK_INTEGER;

    /* tag-20240227-a: If after dequoting, the number is an integer that
    ** fits in 32 bits, then it must be converted into EP_IntValue.  Other
    ** parts of the code expect this.  See also tag-20240227-b. */
    إذا( p->op==TK_INTEGER && sqlite3GetInt32(p->u.zToken, &i_قيمة) ){
      p->u.i_قيمة = i_قيمة;
      p->flags |= EP_Int_قيمة;
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
فراغ sqlite3DequoteToken(Token *p){
  unsigned صحيح i;
  إذا( p->n<2 ) أرجع;
  إذا( !sqlite3Isquote(p->z[0]) ) أرجع;
  لكل(i=1; i<p->n-1; i++){
    إذا( sqlite3Isquote(p->z[i]) ) أرجع;
  }
  p->n -= 2;
  p->z++;
}

/*
** Generate a Token object from a string
*/
فراغ sqlite3TokenInit(Token *p, حرف *z){
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
صحيح sqlite3_stricmp(ثابت حرف *z_يسار, ثابت حرف *z_يمين){
  إذا( z_يسار==0 ){
    أرجع z_يمين ? -1 : 0;
  }وإلا إذا( z_يمين==0 ){
    أرجع 1;
  }
  أرجع sqlite3StrICmp(z_يسار, z_يمين);
}
صحيح sqlite3StrICmp(ثابت حرف *z_يسار, ثابت حرف *z_يمين){
  unsigned حرف *a, *b;
  صحيح c, x;
  a = (unsigned حرف *)z_يسار;
  b = (unsigned حرف *)z_يمين;
  لكل(;;){
    c = *a;
    x = *b;
    إذا( c==x ){
      إذا( c==0 ) اكسر;
    }وإلا{
      c = (صحيح)UpperToLower[c] - (صحيح)UpperToLower[x];
      إذا( c ) اكسر;
    }
    a++;
    b++;
  }
  أرجع c;
}
صحيح sqlite3_strnicmp(ثابت حرف *z_يسار, ثابت حرف *z_يمين, صحيح N){
  سجل unsigned حرف *a, *b;
  إذا( z_يسار==0 ){
    أرجع z_يمين ? -1 : 0;
  }وإلا إذا( z_يمين==0 ){
    أرجع 1;
  }
  a = (unsigned حرف *)z_يسار;
  b = (unsigned حرف *)z_يمين;
  طالما( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  أرجع N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/*
** Compute an 8-bit hash on a string that is insensitive to case differences
*/
u8 sqlite3StrIHash(ثابت حرف *z){
  u8 h = 0;
  إذا( z==0 ) أرجع 0;
  طالما( z[0] ){
    h += UpperToLower[(unsigned حرف)z[0]];
    z++;
  }
  أرجع h;
}

#إذا !defined(SQLITE_DISABLE_INTRINSIC)               \
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
ساكن u64 sqlite3Multiply128(u64 a, u64 b, u64 *pLo){
#إذا defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  *pLo = (u64)r;
  أرجع (u64)(r>>64);
#وإلاإذا defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  *pLo = a*b;
  أرجع __umulh(a, b);
#وإلا
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
  أرجع a1b1 + (a0b1>>32) + (a1b0>>32) + (t>>32);
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
ساكن u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo){
#إذا defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  r += ((__uint128_t)aLo * b) >> 32;
  *pLo = (r>>32)&0xffffffff;
  أرجع r>>64;
#وإلاإذا defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  u64 r1_hi = __umulh(a,b);
  u64 r1_lo = a*b;
  u64 r2 = (__umulh((u64)aLo,b)<<32) + ((aLo*b)>>32);
  u64 t = r1_lo + r2;
  إذا( t<r1_lo ) r1_hi++;
  *pLo = t>>32;
  أرجع r1_hi;
#وإلا
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
  أرجع (r4<<32) + r3;
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
ساكن u64 powerOfTen(صحيح p, u32 *pLo){
  ساكن ثابت u64 aBase[] = {
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
  ساكن ثابت u64 aScale[] = {
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
  ساكن ثابت unsigned صحيح aScaleLo[] = {
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
  صحيح g, n;
  u64 s, x;
  u32 lo;

  أكد( p>=POWERSOF10_FIRST && p<=POWERSOF10_LAST );
  إذا( p<0 ){
    إذا( p==(-1) ){
      *pLo = aScaleLo[13];
      أرجع aScale[13];
    }
    g = p/27;
    n = p%27;
    إذا( n ){
      g--;
      n += 27;
    }
  }وإلا إذا( p<27 ){
    *pLo = 0;
    أرجع aBase[p];
  }وإلا{
    g = p/27;
    n = p%27;
  }
  s = aScale[g+13];
  إذا( n==0 ){
    *pLo = aScaleLo[g+13];
    أرجع s;
  }
  x = sqlite3Multiply160(s,aScaleLo[g+13],aBase[n],&lo);
  إذا( (U64_BIT(63) & x)==0 ){
    x  = x<<1 | ((lo>>31)&1);
    lo = (lo<<1) | 1;
  }
  *pLo = lo;
  أرجع x;
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
ساكن صحيح pwr10to2(صحيح p){ أرجع (p*108853) >> 15; }
ساكن صحيح pwr2to10(صحيح p){ أرجع (p*78913) >> 18; }

/*
** Count leading zeros for a 64-bit unsigned integer.
*/
ساكن صحيح عدد_Leading_Zeros(u64 m){
#إذا (defined(__GNUC__) || defined(__clang__)) \
    && !defined(SQLITE_DISABLE_INTRINSIC)
  أرجع __builtin_clzll(m);
#وإلا
  صحيح n = 0;
  إذا( m <= 0x00000000ffffffffULL) { n += 32; m <<= 32; }
  إذا( m <= 0x0000ffffffffffffULL) { n += 16; m <<= 16; }
  إذا( m <= 0x00ffffffffffffffULL) { n += 8;  m <<= 8;  }
  إذا( m <= 0x0fffffffffffffffULL) { n += 4;  m <<= 4;  }
  إذا( m <= 0x3fffffffffffffffULL) { n += 2;  m <<= 2;  }
  إذا( m <= 0x7fffffffffffffffULL) { n += 1;            }
  أرجع n;
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
ساكن فراغ sqlite3Fp2Convert10(u64 m, صحيح e, صحيح n, u64 *pD, صحيح *pP){
  صحيح p;
  u64 h, d1;
  u32 d2;
  أكد( n>=1 && n<=18 );
  p = n - 1 - pwr2to10(e+63);
  h = sqlite3Multiply128(m, powerOfTen(p,&d2), &d1);
  أكد( -(e + pwr10to2(p) + 2) >= 0  );
  أكد( -(e + pwr10to2(p) + 1) <= 63 );
  إذا( n==18 ){
    h >>= -(e + pwr10to2(p) + 2);
    *pD = (h + ((h<<1)&2))>>1;
  }وإلا{
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
ساكن مزدوج sqlite3Fp10Convert2(u64 d, صحيح p){
  صحيح b, lp, e, adj, s;
  u32 pwr10l, mid1;
  u64 pwr10h, x, hi, lo, sticky, u, m;
  مزدوج r;
  إذا( p<POWERSOF10_FIRST ) أرجع 0.0;
  إذا( p>POWERSOF10_LAST ) أرجع (مزدوج)INFINITY;
  b = 64 - عدد_Leading_Zeros(d);
  lp = pwr10to2(p);
  e = 53 - b - lp;
  إذا( e > 1074 ){
    إذا( e>=1130 ) أرجع 0.0;
    e = 1074;
  }
  s = -(e-(64-b) + lp + 3);
  pwr10h = powerOfTen(p, &pwr10l);
  إذا( pwr10l!=0 ){
    pwr10h++;
    pwr10l = ~pwr10l;
  }
  x = d<<(64-b);
  hi = sqlite3Multiply128(x,pwr10h,&lo);
  mid1 = lo>>32;
  sticky = 1;
  إذا( (hi & (U64_BIT(s)-1))==0 ) {
    u32 mid2 = sqlite3Multiply128(x,((u64)pwr10l)<<32,&lo)>>32;
    sticky = (mid1-mid2 > 1);
    hi -= mid1 < mid2;
  }
  u = (hi>>s) | sticky;
  adj = (u >= U64_BIT(55)-2);
  إذا( adj ){
    u = (u>>adj) | (u&1);
    e -= adj;
  }
  m = (u + 1 + ((u>>2)&1)) >> 2;
  إذا( e<=(-972) ) أرجع (مزدوج)INFINITY;
  إذا((m & U64_BIT(52)) != 0){
    m = (m & ~U64_BIT(52)) | ((u64)(1075-e)<<52);
  }
  memcpy(&r,&m,8);
  أرجع r;
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
صحيح sqlite3AtoF(ثابت حرف *zIn, مزدوج *pResult){
#ifndef SQLITE_OMIT_FLOATING_POINT
  ثابت unsigned حرف *z = (ثابت unsigned حرف*)zIn;
  صحيح neg = 0;       /* True for a negative value */
  u64 s = 0;         /* mantissa */
  صحيح d = 0;         /* Value is s * pow(10,d) */
  صحيح mState = 0;    /* 1: digit seen 2: fp 4: hard-zero */
  unsigned v;        /* Value of a single digit */

  start_of_text:
  إذا( (v = (unsigned)z[0] - '0')<10 ){
    parse_integer_part:
    mState = 1;
    s = v;
    z++;
    طالما( (v = (unsigned)z[0] - '0')<10 ){
      s = s*10 + v;
      z++;
      إذا( s>=(LARGEST_UINT64-9)/10 ){
        mState = 9;
        طالما( sqlite3Isdigit(z[0]) ){ z++; d++; }
        اكسر;
      }
    }
  }وإلا إذا( z[0]=='-' ){
    neg = 1;
    z++;
    إذا( (v = (unsigned)z[0] - '0')<10 ) اذهب parse_integer_part;
  }وإلا إذا( z[0]=='+' ){
    z++;
    إذا( (v = (unsigned)z[0] - '0')<10 ) اذهب parse_integer_part;
  }وإلا إذا( sqlite3Isspace(z[0]) ){
    افعل{ z++; }طالما( sqlite3Isspace(z[0]) );
    اذهب start_of_text;
  }وإلا{
    s = 0;
  }

  /* if decimal point is present */
  إذا( *z=='.' ){
    z++;
    إذا( sqlite3Isdigit(z[0]) ){
      mState |= 1;
      افعل{
        إذا( s<(LARGEST_UINT64-9)/10 ){
          s = s*10 + z[0] - '0';
          d--;
        }وإلا{
          mState = 11;
        }
      }طالما( sqlite3Isdigit(*++z) );
    }وإلا إذا( mState==0 ){
      *pResult = 0.0;
      أرجع 0;
    }
    mState |= 2;
  }وإلا إذا( mState==0 ){
    *pResult = 0.0;
    أرجع 0;
  }

  /* if exponent is present */
  إذا( *z=='e' || *z=='E' ){
    صحيح esign;
    z++;
 
    /* get sign of exponent */
    إذا( *z=='-' ){
      esign = -1;
      z++;
    }وإلا{
      esign = +1;
      إذا( *z=='+' ){
        z++;
      }
    }
    /* copy digits to exponent */
    إذا( (v = (unsigned)z[0] - '0')<10 ){
      صحيح exp = v;
      z++;
      mState |= 2;
      طالما( (v = (unsigned)z[0] - '0')<10 ){
        exp = exp<10000 ? (exp*10 + v) : 10000;
        z++;
      }
      d += esign*exp;
    }وإلا{
      z--;  /* Leave z[0] at 'e' or '+' or '-',
            ** so that the return is 0 or -1 */
    }
  }

  /* Convert s*pow(10,d) into real */
  إذا( s==0 ){
    *pResult = 0.0;
    mState |= 4;
  }وإلا{
    *pResult = sqlite3Fp10Convert2(s,d);
  }
  إذا( neg ) *pResult = -*pResult;
  أكد( !sqlite3IsNaN(*pResult) );

  /* return true if number and no extra non-whitespace characters after */
  إذا( z[0]==0 ){
    أرجع mState;
  }
  إذا( sqlite3Isspace(z[0]) ){
    افعل{ z++; }طالما( sqlite3Isspace(*z) );
    إذا( z[0]==0 ){
      أرجع mState;
    }
  }
  أرجع 0xfffffff0 | mState;
#وإلا
  أرجع sqlite3Atoi64(zIn, pResult, strlen(zIn), SQLITE_UTF8)==0;
#endif /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Digit pairs used to convert a U64 or I64 into text, two digits
** at a time.
*/
ساكن ثابت اتحاد {
  حرف a[201];
  short صحيح forceAlignment;
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
#إذا (defined(__arm__) && !defined(__aarch64__)) || \
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
ساكن صحيح sqlite3UInt64ToText(u64 v, حرف *zOut){
  u32 x32, kk;
  صحيح i;
  zOut[SQLITE_U64_DIGITS] = 0;
  i = SQLITE_U64_DIGITS;
  أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[0]) );
  أكد( TWO_BYTE_ALIGNMENT(zOut) );
  طالما( (v>>32)!=0 ){
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
  طالما( x32>=10 ){
    kk  = x32 % 100;
    x32 = x32 / 100;
    أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk*2]) );
    أكد( i>=2 );
    i -= 2;
    أكد( TWO_BYTE_ALIGNMENT(&zOut[i]) );
    *(u16*)(&zOut[i]) = *(u16*)&sqlite3DigitPairs.a[kk*2];
  }
  إذا( x32 ){
    أكد( i>0 );
    zOut[--i] = x32 + '0';
  }
  أرجع i;
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
صحيح sqlite3Int64ToText(i64 v, حرف *zOut){
  صحيح i;
  u64 x;
  اتحاد {
    حرف a[SQLITE_U64_DIGITS+1];
    u16 forceAlignment;
  } u;
  إذا( v>0 ){
    x = v;
  }وإلا إذا( v==0 ){
    zOut[0] = '0';
    zOut[1] = 0;
    أرجع 1;
  }وإلا{
    x = (v==SMALLEST_INT64) ? ((u64)1)<<63 : (u64)-v;
  }
#ifdef SQLITE_AVOID_U64_DIVIDE
  i = sqlite3UInt64ToText(x, u.a);
#وإلا
  i = حجم(u.a)-1;
  u.a[i] = 0;
  طالما( x>=10 ){
    صحيح kk = (x%100)*2;
    أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk]) );
    أكد( TWO_BYTE_ALIGNMENT(&u.a[i-2]) );
    *(u16*)(&u.a[i-2]) = *(u16*)&sqlite3DigitPairs.a[kk];
    i -= 2;
    x /= 100;
  }
  إذا( x ){
    u.a[--i] = x + '0';
  }
#endif /* SQLITE_AVOID_U64_DIVIDE */
  إذا( v<0 ) u.a[--i] = '-';
  memcpy(zOut, &u.a[i], حجم(u.a)-i);
  أرجع حجم(u.a)-1-i;
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
ساكن صحيح compare2pow63(ثابت حرف *z_رقم, صحيح incr){
  صحيح c = 0;
  صحيح i;
                    /* 012345678901234567 */
  ثابت حرف *pow63 = "922337203685477580";
  لكل(i=0; c==0 && i<18; i++){
    c = (z_رقم[i*incr]-pow63[i])*10;
  }
  إذا( c==0 ){
    c = z_رقم[18*incr] - '8';
    testcase( c==(-1) );
    testcase( c==0 );
    testcase( c==(+1) );
  }
  أرجع c;
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
صحيح sqlite3Atoi64(ثابت حرف *z_رقم, i64 *p_رقم, صحيح length, u8 enc){
  صحيح incr;
  u64 u = 0;
  صحيح neg = 0; /* assume positive */
  صحيح i, j;
  unsigned صحيح c = 0;
  صحيح non_رقم = 0;  /* True if input contains UTF16 with high byte non-zero */
  صحيح rc;          /* Baseline return code */
  ثابت حرف *zStart;
  ثابت حرف *zEnd = z_رقم + length;
  أكد( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  إذا( enc==SQLITE_UTF8 ){
    incr = 1;
  }وإلا{
    incr = 2;
    length &= ~1;
    أكد( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    لكل(i=3-enc; i<length && z_رقم[i]==0; i+=2){}
    non_رقم = i<length;
    zEnd = &z_رقم[i^1];
    z_رقم += (enc&1);
  }
  طالما( z_رقم<zEnd && sqlite3Isspace(*z_رقم) ) z_رقم+=incr;
  إذا( z_رقم<zEnd ){
    إذا( *z_رقم=='-' ){
      neg = 1;
      z_رقم+=incr;
    }وإلا إذا( *z_رقم=='+' ){
      z_رقم+=incr;
    }
  }
  zStart = z_رقم;
  طالما( z_رقم<zEnd && z_رقم[0]=='0' ){ z_رقم+=incr; } /* Skip leading zeros. */
  لكل(i=0; &z_رقم[i]<zEnd && (c=(unsigned)z_رقم[i]-'0')<=9; i+=incr){
    u = u*10 + c;
  }
  testcase( i==18*incr );
  testcase( i==19*incr );
  testcase( i==20*incr );
  إذا( u>LARGEST_INT64 ){
    /* This test and assignment is needed only to suppress UB warnings
    ** from clang and -fsanitize=undefined.  This test and assignment make
    ** the code a little larger and slower, and no harm comes from omitting
    ** them, but we must appease the undefined-behavior pharisees. */
    *p_رقم = neg ? SMALLEST_INT64 : LARGEST_INT64;
  }وإلا إذا( neg ){
    *p_رقم = -(i64)u;
  }وإلا{
    *p_رقم = (i64)u;
  }
  rc = 0;
  إذا( i==0 && zStart==z_رقم ){    /* No digits */
    rc = -1;
  }وإلا إذا( non_رقم ){            /* UTF16 with high-order bytes non-zero */
    rc = 1;
  }وإلا إذا( &z_رقم[i]<zEnd ){     /* Extra bytes at the end */
    صحيح jj = i;
    افعل{
      إذا( !sqlite3Isspace(z_رقم[jj]) ){
        rc = 1;          /* Extra non-space text after the integer */
        اكسر;
      }
      jj += incr;
    }طالما( &z_رقم[jj]<zEnd );
  }
  إذا( i<19*incr ){
    /* Less than 19 digits, so we know that it fits in 64 bits */
    أكد( u<=LARGEST_INT64 );
    أرجع rc;
  }وإلا{
    /* zNum is a 19-digit numbers.  Compare it against 9223372036854775808. */
    j = i>19*incr ? 1 : compare2pow63(z_رقم, incr);
    إذا( j<0 ){
      /* zNum is less than 9223372036854775808 so it fits */
      أكد( u<=LARGEST_INT64 );
      أرجع rc;
    }وإلا{
      *p_رقم = neg ? SMALLEST_INT64 : LARGEST_INT64;
      إذا( j>0 ){
        /* zNum is greater than 9223372036854775808 so it overflows */
        أرجع 2;
      }وإلا{
        /* zNum is exactly 9223372036854775808.  Fits if negative.  The
        ** special case 2 overflow if positive */
        أكد( u-1==LARGEST_INT64 );
        أرجع neg ? rc : 3;
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
صحيح sqlite3DecOrHexToI64(ثابت حرف *z, i64 *pOut){
#ifndef SQLITE_OMIT_HEX_INTEGER
  إذا( z[0]=='0'
   && (z[1]=='x' || z[1]=='X')
  ){
    u64 u = 0;
    صحيح i, k;
    لكل(i=2; z[i]=='0'; i++){}
    لكل(k=i; sqlite3Isxdigit(z[k]); k++){
      u = u*16 + sqlite3HexToInt(z[k]);
    }
    memcpy(pOut, &u, 8);
    إذا( k-i>16 ) أرجع 2;
    إذا( z[k]!=0 ) أرجع 1;
    أرجع 0;
  }وإلا
#endif /* SQLITE_OMIT_HEX_INTEGER */
  {
    صحيح n = (صحيح)(0x3fffffff&strspn(z,"+- \n\t0123456789"));
    إذا( z[n] ) n++;
    أرجع sqlite3Atoi64(z, pOut, n, SQLITE_UTF8);
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
صحيح sqlite3GetInt32(ثابت حرف *z_رقم, صحيح *p_قيمة){
  sqlite_int64 v = 0;
  صحيح i, c;
  صحيح neg = 0;
  إذا( z_رقم[0]=='-' ){
    neg = 1;
    z_رقم++;
  }وإلا إذا( z_رقم[0]=='+' ){
    z_رقم++;
  }
#ifndef SQLITE_OMIT_HEX_INTEGER
  وإلا إذا( z_رقم[0]=='0'
        && (z_رقم[1]=='x' || z_رقم[1]=='X')
        && sqlite3Isxdigit(z_رقم[2])
  ){
    u32 u = 0;
    z_رقم += 2;
    طالما( z_رقم[0]=='0' ) z_رقم++;
    لكل(i=0; i<8 && sqlite3Isxdigit(z_رقم[i]); i++){
      u = u*16 + sqlite3HexToInt(z_رقم[i]);
    }
    إذا( (u&0x80000000)==0 && sqlite3Isxdigit(z_رقم[i])==0 ){
      memcpy(p_قيمة, &u, 4);
      أرجع 1;
    }وإلا{
      أرجع 0;
    }
  }
#endif
  إذا( !sqlite3Isdigit(z_رقم[0]) ) أرجع 0;
  طالما( z_رقم[0]=='0' ) z_رقم++;
  لكل(i=0; i<11 && (c = z_رقم[i] - '0')>=0 && c<=9; i++){
    v = v*10 + c;
  }

  /* The longest decimal representation of a 32 bit integer is 10 digits:
  **
  **             1234567890
  **     2^31 -> 2147483648
  */
  testcase( i==10 );
  إذا( i>10 ){
    أرجع 0;
  }
  testcase( v-neg==2147483647 );
  إذا( v-neg>2147483647 ){
    أرجع 0;
  }
  إذا( neg ){
    v = -v;
  }
  *p_قيمة = (صحيح)v;
  أرجع 1;
}

/*
** Return a 32-bit integer value extracted from a string.  If the
** string is not an integer, just return 0.
*/
صحيح sqlite3Atoi(ثابت حرف *z){
  صحيح x = 0;
  sqlite3GetInt32(z, &x);
  أرجع x;
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
فراغ sqlite3FpDecode(FpDecode *p, مزدوج r, صحيح iRound, صحيح mxRound){
  صحيح i;               /* Index into zBuf[] where to put next character */
  صحيح n;               /* Number of digits */
  u64 v;               /* mantissa */
  صحيح e, exp = 0;      /* Base-2 and base-10 exponent */
  حرف *zBuf;          /* Local alias for p->zBuf */
  حرف *z;             /* Local alias for p->z */

  p->isSpecial = 0;
  أكد( mxRound>0 );
  أكد( iRound>=-100000000 );

  /* Convert negative numbers to positive.  Deal with Infinity, 0.0, and
  ** NaN. */
  إذا( r<0.0 ){
    p->sign = '-';
    r = -r;
  }وإلا إذا( r==0.0 ){
    p->sign = '+';
    p->n = 1;
    p->iDP = 1;
    p->z = "0";
    أرجع;
  }وإلا{
    p->sign = '+';
  }
  memcpy(&v,&r,8);
  e = (v>>52)&0x7ff;
  إذا( e==0x7ff ){
    p->isSpecial = 1 + (v!=0x7ff0000000000000LL);
    p->n = 0;
    p->iDP = 0;
    p->z = p->zBuf;
    أرجع;
  }
  v &= 0x000fffffffffffffULL;
  إذا( e==0 ){
    صحيح nn = عدد_Leading_Zeros(v);
    v <<= nn;
    e = -1074 - nn;
  }وإلا{
    v = (v<<11) | U64_BIT(63);
    e -= 1086;
  }
  sqlite3Fp2Convert10(v, e, (iRound<=0||iRound>=18)?18:iRound+1, &v, &exp);  

  /* Extract significant digits, start at the right-most slot in p->zBuf
  ** and working back to the right.  "i" keeps track of the next slot in
  ** which to store a digit. */
  أكد( حجم(p->zBuf)==SQLITE_U64_DIGITS+1 );
  أكد( v>0 );
  zBuf = p->zBuf;
#ifdef SQLITE_AVOID_U64_DIVIDE
  i = sqlite3UInt64ToText(v, zBuf);
#وإلا
  i = SQLITE_U64_DIGITS;
  طالما( v>=10 ){
    صحيح kk = (v%100)*2;
    أكد( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk]) );
    أكد( TWO_BYTE_ALIGNMENT(&zBuf[i]) );
    أكد( i-2>=0 );
    *(u16*)(&zBuf[i-2]) = *(u16*)&sqlite3DigitPairs.a[kk];
    i -= 2;
    v /= 100;
  }
  إذا( v ){
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
  إذا( iRound<=0 ){
    iRound = p->iDP - iRound;
    إذا( iRound==0 && zBuf[i]>='5' ){
      iRound = 1;
      zBuf[--i] = '0';
      n++;
      p->iDP++;
    }
  }
  z = &zBuf[i];  /* z points to the first digit */
  إذا( iRound>0 && (iRound<n || n>mxRound) ){
    إذا( iRound>mxRound ) iRound = mxRound;
    إذا( iRound==17 ){
      /* If the precision is exactly 17, which only happens with the "!"
      ** flag (ex: "%!.17g") then try to reduce the precision if that
      ** yields text that will round-trip to the original floating-point.
      ** value.  Thus, for exaple, 49.47 will render as 49.47, rather than
      ** as 49.469999999999999. */
      إذا( z[15]=='9' && z[14]=='9' ){
        صحيح jj, kk;
        u64 v2;
        لكل(jj=14; jj>0 && z[jj-1]=='9'; jj--){}
        إذا( jj==0 ){
          v2 = 1;
        }وإلا{
          v2 = z[0] - '0';
          لكل(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
          v2++;
        }
        إذا( r==sqlite3Fp10Convert2(v2, exp + n - jj) ){
          iRound = jj+1;
        }
      }وإلا إذا( p->iDP>=n || (z[15]=='0' && z[14]=='0' && z[13]=='0') ){
        صحيح jj, kk;
        u64 v2;
        أكد( z[0]!='0' );
        لكل(jj=13; z[jj-1]=='0'; jj--){}
        v2 = z[0] - '0';
        لكل(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
        إذا( r==sqlite3Fp10Convert2(v2, exp + n - jj) ){
          iRound = jj+1;
        }
      }
    }
    n = iRound;
    إذا( z[iRound]>='5' ){
      صحيح j = iRound-1;
      طالما( 1 /*exit-by-break*/ ){
        z[j]++;
        إذا( z[j]<='9' ) اكسر;
        z[j] = '0';
        إذا( j==0 ){
          z--;
          z[0] = '1';
          n++;
          p->iDP++;
          اكسر;
        }وإلا{
          j--;
        }
      }
    }
  }
  أكد( n>0 );
  طالما( z[n-1]=='0' ){
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
صحيح sqlite3GetUInt32(ثابت حرف *z, u32 *pI){
  u64 v = 0;
  صحيح i;
  لكل(i=0; sqlite3Isdigit(z[i]); i++){
    v = v*10 + z[i] - '0';
    إذا( v>4294967295LL ){ *pI = 0; أرجع 0; }
  }
  إذا( i==0 || z[i]!=0 ){ *pI = 0; أرجع 0; }
  *pI = (u32)v;
  أرجع 1;
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
ساكن صحيح SQLITE_NOINLINE putVarint64(unsigned حرف *p, u64 v){
  صحيح i, j, n;
  u8 buf[10];
  إذا( v & (((u64)0xff000000)<<32) ){
    p[8] = (u8)v;
    v >>= 8;
    لكل(i=7; i>=0; i--){
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    أرجع 9;
  }   
  n = 0;
  افعل{
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  }طالما( v!=0 );
  buf[0] &= 0x7f;
  أكد( n<=9 );
  لكل(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  أرجع n;
}
صحيح sqlite3PutVarint(unsigned حرف *p, u64 v){
  إذا( v<=0x7f ){
    p[0] = v&0x7f;
    أرجع 1;
  }
  إذا( v<=0x3fff ){
    p[0] = ((v>>7)&0x7f)|0x80;
    p[1] = v&0x7f;
    أرجع 2;
  }
  أرجع putVarint64(p,v);
}

/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
u8 sqlite3GetVarint(ثابت unsigned حرف *p, u64 *v){
  u64 i_مفتاح = p[0];
  ثابت u8 *pStart = p;
  إذا( i_مفتاح>=0x80 ){
    u8 x;
    i_مفتاح = (i_مفتاح<<7) ^ (x = *++p);
    إذا( x>=0x80 ){
      i_مفتاح = (i_مفتاح<<7) ^ (x = *++p);
      إذا( x>=0x80 ){
        i_مفتاح = (i_مفتاح<<7) ^ 0x10204000 ^ (x = *++p);
        إذا( x>=0x80 ){
          i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
          إذا( x>=0x80 ){
            i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
            إذا( x>=0x80 ){
              i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
              إذا( x>=0x80 ){
                i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
                إذا( x>=0x80 ){
                  i_مفتاح = (i_مفتاح<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }وإلا{
        i_مفتاح ^= 0x204000;
      }
    }وإلا{
      i_مفتاح ^= 0x4000;
    }
  }
  *v = i_مفتاح;
  أرجع (u8)(p - pStart) + 1;
}

/*
** Return the value of a variable-length integer without computing
** its length.  This is an optimization on sqlite3GetVarint() for the
** cases when the return value of sqlite3GetVarint() is not needed.
*/
i64 sqlite3_Varint_قيمة(ثابت unsigned حرف *p){
  u64 i_مفتاح = p[0];
  إذا( i_مفتاح>=0x80 ){
    u8 x;
    i_مفتاح = (i_مفتاح<<7) ^ (x = *++p);
    إذا( x>=0x80 ){
      i_مفتاح = (i_مفتاح<<7) ^ (x = *++p);
      إذا( x>=0x80 ){
        i_مفتاح = (i_مفتاح<<7) ^ 0x10204000 ^ (x = *++p);
        إذا( x>=0x80 ){
          i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
          إذا( x>=0x80 ){
            i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
            إذا( x>=0x80 ){
              i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
              إذا( x>=0x80 ){
                i_مفتاح = (i_مفتاح<<7) ^ 0x4000 ^ (x = *++p);
                إذا( x>=0x80 ){
                  i_مفتاح = (i_مفتاح<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }وإلا{
        i_مفتاح ^= 0x204000;
      }
    }وإلا{
      i_مفتاح ^= 0x4000;
    }
  }
  أرجع *(i64*)&i_مفتاح;
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
u8 sqlite3GetVarint32(ثابت unsigned حرف *p, u32 *v){
  u64 v64;
  u8 n;

  /* Assume that the single-byte case has already been handled by
  ** the getVarint32() macro */
  أكد( (p[0] & 0x80)!=0 );

  إذا( (p[1] & 0x80)==0 ){
    /* This is the two-byte case */
    *v = ((p[0]&0x7f)<<7) | p[1];
    أرجع 2;
  }
  إذا( (p[2] & 0x80)==0 ){
    /* This is the three-byte case */
    *v = ((p[0]&0x7f)<<14) | ((p[1]&0x7f)<<7) | p[2];
    أرجع 3;
  }
  /* four or more bytes */
  n = sqlite3GetVarint(p, &v64);
  أكد( n>3 && n<=9 );
  إذا( (v64 & SQLITE_MAX_U32)!=v64 ){
    *v = 0xffffffff;
  }وإلا{
    *v = (u32)v64;
  }
  أرجع n;
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
صحيح sqlite3VarintLen(u64 v){
  صحيح i;
  لكل(i=1; (v >>= 7)!=0; i++){ أكد( i<10 ); }
  أرجع i;
}


/*
** Read an unsigned 32-bit integer from an unaligned big-endian array of bytes.
*/
u32 sqlite3Get4byte(ثابت u8 *p){
#إذا SQLITE_BYTEORDER==4321
  u32 x;
  memcpy(&x,p,4);
  أرجع x;
#وإلاإذا SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u32 x;
  memcpy(&x,p,4);
  أرجع __builtin_bswap32(x);
#وإلاإذا SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u32 x;
  memcpy(&x,p,4);
  أرجع _byteswap_ulong(x);
#وإلا
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  أرجع ((unsigned)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
#endif
}

/* Write an unsigned 32-bit integer into an unaligned big-endian array of bytes.
*/
فراغ sqlite3Put4byte(unsigned حرف *p, u32 v){
#إذا SQLITE_BYTEORDER==4321
  memcpy(p,&v,4);
#وإلاإذا SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u32 x = __builtin_bswap32(v);
  memcpy(p,&x,4);
#وإلاإذا SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u32 x = _byteswap_ulong(v);
  memcpy(p,&x,4);
#وإلا
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
#إذا SQLITE_BYTEORDER==4321
  u64 x;
  memcpy(&x,p,8);
  أرجع x;
#وإلاإذا SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u64 x;
  memcpy(&x,p,8);
  أرجع __builtin_bswap64(x);
#وإلاإذا SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u64 x;
  memcpy(&x,p,8);
  أرجع _byteswap_uint64(x);
#وإلا
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  أرجع (u64)(
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

#إذا SQLITE_BYTEORDER!=4321  /* Only used for little-endian machines */
/*
** Byte-swap a 64-bit unsigned integer.
*/
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64 x){
#إذا SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  أرجع __builtin_bswap64(x);
#وإلاإذا SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  أرجع _byteswap_uint64(x);
#وإلا
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  x = (x << 32) | (x >> 32);
  x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) |
      ((x & UINT64_C(0xffff0000ffff0000)) >> 16);
  x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) |
      ((x & UINT64_C(0xff00ff00ff00ff00)) >>  8);
  أرجع x;
#endif
}
#endif /* SQLITE_BYTEORDER!=4321 */

/*
** Translate a single byte of Hex into an integer.
** This routine only works if h really is a valid hexadecimal
** character:  0..9a..fA..F
*/
u8 sqlite3HexToInt(صحيح h){
  أكد( (h>='0' && h<='9') ||  (h>='a' && h<='f') ||  (h>='A' && h<='F') );
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  أرجع (u8)(h & 0xf);
}

#إذا !defined(SQLITE_OMIT_BLOB_LITERAL)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
فراغ *sqlite3HexToBlob(sqlite3 *db, ثابت حرف *z, صحيح n){
  حرف *zBlob;
  صحيح i;

  zBlob = (حرف *)sqlite3DbMallocRawNN(db, n/2 + 1);
  n--;
  إذا( zBlob ){
    لكل(i=0; i<n; i+=2){
      zBlob[i/2] = (sqlite3HexToInt(z[i])<<4) | sqlite3HexToInt(z[i+1]);
    }
    zBlob[i/2] = 0;
  }
  أرجع zBlob;
}
#endif /* !SQLITE_OMIT_BLOB_LITERAL */

/*
** Log an error that is an API call on a connection pointer that should
** not have been used.  The "type" of connection pointer is given as the
** argument.  The zType is a word like "NULL" or "closed" or "invalid".
*/
ساكن فراغ logBadConnection(ثابت حرف *zType){
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
صحيح sqlite3SafetyCheckOk(sqlite3 *db){
  u8 eOpenState;
  إذا( db==0 ){
    logBadConnection("NULL");
    أرجع 0;
  }
  eOpenState = db->eOpenState;
  إذا( eOpenState!=SQLITE_STATE_OPEN ){
    إذا( sqlite3SafetyCheckSickOrOk(db) ){
      testcase( sqlite3GlobalConfig.xLog!=0 );
      logBadConnection("unopened");
    }
    أرجع 0;
  }وإلا{
    أرجع 1;
  }
}
صحيح sqlite3SafetyCheckSickOrOk(sqlite3 *db){
  u8 eOpenState;
  eOpenState = db->eOpenState;
  إذا( eOpenState!=SQLITE_STATE_SICK &&
      eOpenState!=SQLITE_STATE_OPEN &&
      eOpenState!=SQLITE_STATE_BUSY ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    logBadConnection("invalid");
    أرجع 0;
  }وإلا{
    أرجع 1;
  }
}

/*
** Attempt to add, subtract, or multiply the 64-bit signed value iB against
** the other 64-bit signed integer at *pA and store the result in *pA.
** Return 0 on success.  Or if the operation would have resulted in an
** overflow, leave *pA unchanged and return 1.
*/
صحيح sqlite3AddInt64(i64 *pA, i64 iB){
#إذا GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  أرجع __builtin_add_overflow(*pA, iB, pA);
#وإلا
  i64 iA = *pA;
  testcase( iA==0 ); testcase( iA==1 );
  testcase( iB==-1 ); testcase( iB==0 );
  إذا( iB>=0 ){
    testcase( iA>0 && LARGEST_INT64 - iA == iB );
    testcase( iA>0 && LARGEST_INT64 - iA == iB - 1 );
    إذا( iA>0 && LARGEST_INT64 - iA < iB ) أرجع 1;
  }وإلا{
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 1 );
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 2 );
    إذا( iA<0 && -(iA + LARGEST_INT64) > iB + 1 ) أرجع 1;
  }
  *pA += iB;
  أرجع 0;
#endif
}
صحيح sqlite3SubInt64(i64 *pA, i64 iB){
#إذا GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  أرجع __builtin_sub_overflow(*pA, iB, pA);
#وإلا
  testcase( iB==SMALLEST_INT64+1 );
  إذا( iB==SMALLEST_INT64 ){
    testcase( (*pA)==(-1) ); testcase( (*pA)==0 );
    إذا( (*pA)>=0 ) أرجع 1;
    *pA -= iB;
    أرجع 0;
  }وإلا{
    أرجع sqlite3AddInt64(pA, -iB);
  }
#endif
}
صحيح sqlite3MulInt64(i64 *pA, i64 iB){
#إذا GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  أرجع __builtin_mul_overflow(*pA, iB, pA);
#وإلا
  i64 iA = *pA;
  إذا( iB>0 ){
    إذا( iA>LARGEST_INT64/iB ) أرجع 1;
    إذا( iA<SMALLEST_INT64/iB ) أرجع 1;
  }وإلا إذا( iB<0 ){
    إذا( iA>0 ){
      إذا( iB<SMALLEST_INT64/iA ) أرجع 1;
    }وإلا إذا( iA<0 ){
      إذا( iB==SMALLEST_INT64 ) أرجع 1;
      إذا( iA==SMALLEST_INT64 ) أرجع 1;
      إذا( -iA>LARGEST_INT64/-iB ) أرجع 1;
    }
  }
  *pA = iA*iB;
  أرجع 0;
#endif
}

/*
** Compute the absolute value of a 32-bit signed integer, if possible.  Or
** if the integer has a value of -2147483648, return +2147483647
*/
صحيح sqlite3AbsInt32(صحيح x){
  إذا( x>=0 ) أرجع x;
  إذا( x==(صحيح)0x80000000 ) أرجع 0x7fffffff;
  أرجع -x;
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
فراغ sqlite3FileSuffix3(ثابت حرف *zBaseFilename, حرف *z){
#إذا SQLITE_ENABLE_8_3_NAMES<2
  إذا( sqlite3_uri_boolean(zBaseFilename, "8_3_names", 0) )
#endif
  {
    صحيح i, sz;
    sz = sqlite3Strlen30(z);
    لكل(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    إذا( z[i]=='.' && ALWAYS(sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
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
  ساكن ثابت unsigned حرف x[] = {
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
  إذا( a>=b ){
    إذا( a>b+49 ) أرجع a;
    إذا( a>b+31 ) أرجع a+1;
    أرجع a+x[a-b];
  }وإلا{
    إذا( b>a+49 ) أرجع b;
    إذا( b>a+31 ) أرجع b+1;
    أرجع b+x[b-a];
  }
}

/*
** Convert an integer into a LogEst.  In other words, compute an
** approximation for 10*log2(x).
*/
LogEst sqlite3LogEst(u64 x){
  ساكن LogEst a[] = { 0, 2, 3, 5, 6, 7, 8, 9 };
  LogEst y = 40;
  إذا( x<8 ){
    إذا( x<2 ) أرجع 0;
    طالما( x<8 ){  y -= 10; x <<= 1; }
  }وإلا{
#إذا GCC_VERSION>=5004000
    صحيح i = 60 - __builtin_clzll(x);
    y += i*10;
    x >>= i;
#وإلا
    طالما( x>255 ){ y += 40; x >>= 4; }  /*OPTIMIZATION-IF-TRUE*/
    طالما( x>15 ){  y += 10; x >>= 1; }
#endif
  }
  أرجع a[x&7] + y - 10;
}

/*
** Convert a double into a LogEst
** In other words, compute an approximation for 10*log2(x).
*/
LogEst sqlite3LogEstFromDouble(مزدوج x){
  u64 a;
  LogEst e;
  أكد( حجم(x)==8 && حجم(a)==8 );
  إذا( x<=1 ) أرجع 0;
  إذا( x<=2000000000 ) أرجع sqlite3LogEst((u64)x);
  memcpy(&a, &x, 8);
  e = (a>>52) - 1022;
  أرجع e*10;
}

/*
** Convert a LogEst into an integer.
*/
u64 sqlite3LogEstToInt(LogEst x){
  u64 n;
  n = x%10;
  x /= 10;
  إذا( n>=5 ) n -= 2;
  وإلا إذا( n>=1 ) n -= 1;
  إذا( x>60 ) أرجع (u64)LARGEST_INT64;
  أرجع x>=3 ? (n+8)<<(x-3) : (n+8)>>(3-x);
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
  ثابت حرف *zName,     /* Name of symbol to add */
  صحيح nName,             /* Bytes of text in zName */
  صحيح iVal               /* Value to associate with zName */
){
  صحيح nInt;              /* number of sizeof(int) objects needed for zName */
  حرف *z;               /* Pointer to where zName will be stored */
  صحيح i;                 /* Index in pIn[] where zName is stored */

  nInt = nName/4 + 3;
  أكد( pIn==0 || pIn[0]>=3 );  /* Verify ok to add new elements */
  إذا( pIn==0 || pIn[1]+nInt > pIn[0] ){
    /* Enlarge the allocation */
    sqlite3_int64 nAlloc = (pIn ? 2*(sqlite3_int64)pIn[0] : 10) + nInt;
    VList *pOut = sqlite3DbRealloc(db, pIn, nAlloc*حجم(صحيح));
    إذا( pOut==0 ) أرجع pIn;
    إذا( pIn==0 ) pOut[1] = 2;
    pIn = pOut;
    pIn[0] = nAlloc;
  }
  i = pIn[1];
  pIn[i] = iVal;
  pIn[i+1] = nInt;
  z = (حرف*)&pIn[i+2];
  pIn[1] = i+nInt;
  أكد( pIn[1]<=pIn[0] );
  memcpy(z, zName, nName);
  z[nName] = 0;
  أرجع pIn;
}

/*
** Return a pointer to the name of a variable in the given VList that
** has the value iVal.  Or return a NULL if there is no such variable in
** the list
*/
ثابت حرف *sqlite3_VList_رقم_To_Name(VList *pIn, صحيح iVal){
  صحيح i, mx;
  إذا( pIn==0 ) أرجع 0;
  mx = pIn[1];
  i = 2;
  افعل{
    إذا( pIn[i]==iVal ) أرجع (حرف*)&pIn[i+2];
    i += pIn[i+1];
  }طالما( i<mx );
  أرجع 0;
}

/*
** Return the number of the variable named zName, if it is in VList.
** or return 0 if there is no such variable.
*/
صحيح sqlite3_VList_Name_To_رقم(VList *pIn, ثابت حرف *zName, صحيح nName){
  صحيح i, mx;
  إذا( pIn==0 ) أرجع 0;
  mx = pIn[1];
  i = 2;
  افعل{
    ثابت حرف *z = (ثابت حرف*)&pIn[i+2];
    إذا( strncmp(z,zName,nName)==0 && z[nName]==0 ) أرجع pIn[i];
    i += pIn[i+1];
  }طالما( i<mx );
  أرجع 0;
}
