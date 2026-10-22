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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdarg.h>
#ifndef SQLITE_OMIT_FLOATING_POINT
#अन्तर्भाव <math.h>

/* Work around a bug in older Microsoft compilers
** Forum post 2026-04-10T06:33:11z */
#यदि !defined(अनन्त) && defined(_MSC_VER)
# define अनन्त विशाल_मान
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
पूर्णाङ्क sqlite3FaultSim(पूर्णाङ्क iTest){
  पूर्णाङ्क (*xCallback)(पूर्णाङ्क) = sqlite3GlobalConfig.xTestCallback;
  फर्काऊ xCallback ? xCallback(iTest) : SQLITE_OK;
}
#endif

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is Not a Number (NaN).
**
** Use the math library isnan() function if compiled with SQLITE_HAVE_ISNAN.
** Otherwise, we have our own implementation that works on most systems.
*/
पूर्णाङ्क sqlite3IsNaN(द्विगुण x){
  पूर्णाङ्क rc;   /* The value return */
#यदि !SQLITE_HAVE_ISNAN && !HAVE_ISNAN
  u64 y;
  स_नकल(&y,&x,परिमाण(y));
  rc = IsNaN(y);
#अन्यथा
  rc = isnan(x);
#endif /* HAVE_ISNAN */
  testcase( rc );
  फर्काऊ rc;
}
#endif /* SQLITE_OMIT_FLOATING_POINT */

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is NaN or +Inf or -Inf.
*/
पूर्णाङ्क sqlite3IsOverflow(द्विगुण x){
  पूर्णाङ्क rc;   /* The value return */
  u64 y;
  स_नकल(&y,&x,परिमाण(y));
  rc = IsOvfl(y);
  फर्काऊ rc;
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
पूर्णाङ्क sqlite3Strlen30(अचर वर्ण *z){
  यदि( z==0 ) फर्काऊ 0;
  फर्काऊ 0x3fffffff & (पूर्णाङ्क)म_माप(z);
}

/*
** Return the declared type of a column.  Or return zDflt if the column
** has no declared type.
**
** The column type is an extra string stored after the zero-terminator on
** the column name if and only if the COLFLAG_HASTYPE flag is set.
*/
वर्ण *sqlite3ColumnType(Column *pCol, वर्ण *zDflt){
  यदि( pCol->colFlags & COLFLAG_HASTYPE ){
    फर्काऊ pCol->zCnName + म_माप(pCol->zCnName) + 1;
  }अन्यथा यदि( pCol->eCType ){
    प्रतिज्ञा( pCol->eCType<=SQLITE_N_STDTYPE );
    फर्काऊ (वर्ण*)sqlite3StdType[pCol->eCType-1];
  }अन्यथा{
    फर्काऊ zDflt;
  }
}

/*
** Helper function for sqlite3Error() - called rarely.  Broken out into
** a separate routine to avoid unnecessary register saves on entry to
** sqlite3Error().
*/
स्थिर SQLITE_NOINLINE शून्य  sqlite3ErrorFinish(sqlite3 *db, पूर्णाङ्क err_code){
  यदि( db->pErr ) sqlite3_मान_Set_Null(db->pErr);
  sqlite3SystemError(db, err_code);
}

/*
** Set the current error code to err_code and clear any prior error message.
** Also set iSysErrno (by calling sqlite3System) if the err_code indicates
** that would be appropriate.
*/
शून्य sqlite3Error(sqlite3 *db, पूर्णाङ्क err_code){
  प्रतिज्ञा( db!=0 );
  db->errCode = err_code;
  यदि( err_code || db->pErr ){
    sqlite3ErrorFinish(db, err_code);
  }अन्यथा{
    db->errByteOffset = -1;
  }
}

/*
** The equivalent of sqlite3Error(db, SQLITE_OK).  Clear the error state
** and error message.
*/
शून्य sqlite3ErrorClear(sqlite3 *db){
  प्रतिज्ञा( db!=0 );
  db->errCode = SQLITE_OK;
  db->errByteOffset = -1;
  यदि( db->pErr ) sqlite3_मान_Set_Null(db->pErr);
}

/*
** Load the sqlite3.iSysErrno field if that is an appropriate thing
** to do based on the SQLite error code in rc.
*/
शून्य sqlite3SystemError(sqlite3 *db, पूर्णाङ्क rc){
  यदि( rc==SQLITE_IOERR_NOMEM ) फर्काऊ;
#यदि defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
  यदि( rc==SQLITE_IOERR_IN_PAGE ){
    पूर्णाङ्क ii;
    पूर्णाङ्क iErr;
    sqlite3BtreeEnterAll(db);
    लागि(ii=0; ii<db->nDb; ii++){
      यदि( db->aDb[ii].pBt ){
        iErr = sqlite3PagerWalSystemErrno(sqlite3BtreePager(db->aDb[ii].pBt));
        यदि( iErr ){
          db->iSysErrno = iErr;
        }
      }
    }
    sqlite3BtreeLeaveAll(db);
    फर्काऊ;
  }
#endif
  rc &= 0xff;
  यदि( rc==SQLITE_CANTOPEN || rc==SQLITE_IOERR ){
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
शून्य sqlite3_Error_With_सन्देश(sqlite3 *db, पूर्णाङ्क err_code, अचर वर्ण *zFormat, ...){
  प्रतिज्ञा( db!=0 );
  db->errCode = err_code;
  sqlite3SystemError(db, err_code);
  यदि( zFormat==0 ){
    sqlite3Error(db, err_code);
  }अन्यथा यदि( db->pErr || (db->pErr = sqlite3_मान_New(db))!=0 ){
    वर्ण *z;
    बहु_सूची ap;
    बहु_शुरू(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);
    बहु_पूर्ण(ap);
    sqlite3_मान_Set_Str(db->pErr, -1, z, SQLITE_UTF8, SQLITE_DYNAMIC);
  }
}

/*
** Check for interrupts and invoke progress callback.
*/
शून्य sqlite3ProgressCheck(Parse *p){
  sqlite3 *db = p->db;
  यदि( AtomicLoad(&db->u1.isInterrupted) ){
    p->nErr++;
    p->rc = SQLITE_INTERRUPT;
  }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  यदि( db->xProgress ){
    यदि( p->rc==SQLITE_INTERRUPT ){
      p->nProgressSteps = 0;
    }अन्यथा यदि( (++p->nProgressSteps)>=db->nProgressOps ){
      यदि( db->xProgress(db->pProgressArg) ){
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
शून्य sqlite3_Error_सन्देश(Parse *pParse, अचर वर्ण *zFormat, ...){
  वर्ण *z_सन्देश;
  बहु_सूची ap;
  sqlite3 *db = pParse->db;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( db->pParse==pParse || db->pParse->pToplevel==pParse );
  db->errByteOffset = -2;
  बहु_शुरू(ap, zFormat);
  z_सन्देश = sqlite3VMPrintf(db, zFormat, ap);
  बहु_पूर्ण(ap);
  यदि( db->errByteOffset<-1 ) db->errByteOffset = -1;
  यदि( db->suppressErr ){
    sqlite3DbFree(db, z_सन्देश);
    यदि( db->mallocFailed ){
      pParse->nErr++;
      pParse->rc = SQLITE_NOMEM;
    }
  }अन्यथा{
    pParse->nErr++;
    sqlite3DbFree(db, pParse->z_Err_सन्देश);
    pParse->z_Err_सन्देश = z_सन्देश;
    pParse->rc = SQLITE_ERROR;
    pParse->pWith = 0;
  }
}

/*
** If database connection db is currently parsing SQL, then transfer
** error code errCode to that parser if the parser has not already
** encountered some other kind of error.
*/
पूर्णाङ्क sqlite3ErrorToParser(sqlite3 *db, पूर्णाङ्क errCode){
  Parse *pParse;
  यदि( db==0 || (pParse = db->pParse)==0 ) फर्काऊ errCode;
  pParse->rc = errCode;
  pParse->nErr++;
  फर्काऊ errCode;
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
शून्य sqlite3Dequote(वर्ण *z){
  वर्ण quote;
  पूर्णाङ्क i, j;
  यदि( z==0 ) फर्काऊ;
  quote = z[0];
  यदि( !sqlite3Isquote(quote) ) फर्काऊ;
  यदि( quote=='[' ) quote = ']';
  लागि(i=1, j=0;; i++){
    प्रतिज्ञा( z[i] );
    यदि( z[i]==quote ){
      यदि( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }अन्यथा{
        रोक;
      }
    }अन्यथा{
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}
शून्य sqlite3_Dequote_अभिव्यक्ति(अभिव्यक्ति *p){
  प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p, EP_Int_मान) );
  प्रतिज्ञा( sqlite3Isquote(p->u.zToken[0]) );
  p->flags |= p->u.zToken[0]=='"' ? EP_Quoted|EP_DblQuoted : EP_Quoted;
  sqlite3Dequote(p->u.zToken);
}

/*
** Expression p is a QNUMBER (quoted number). Dequote the value in p->u.zToken
** and set the type to INTEGER or FLOAT. "Quoted" integers or floats are those
** that contain '_' characters that must be removed before further processing.
*/
शून्य sqlite3_Dequote_सङ्ख्या(Parse *pParse, अभिव्यक्ति *p){
  प्रतिज्ञा( p!=0 || pParse->db->mallocFailed );
  यदि( p ){
    अचर वर्ण *pIn = p->u.zToken;
    वर्ण *pOut = p->u.zToken;
    पूर्णाङ्क bHex = (pIn[0]=='0' && (pIn[1]=='x' || pIn[1]=='X'));
    पूर्णाङ्क i_मान;
    प्रतिज्ञा( p->op==TK_QNUMBER );
    p->op = TK_INTEGER;
    गर {
      यदि( *pIn!=SQLITE_DIGIT_SEPARATOR ){
        *pOut++ = *pIn;
        यदि( *pIn=='e' || *pIn=='E' || *pIn=='.' ) p->op = TK_FLOAT;
      }अन्यथा{
        यदि( (bHex==0 && (!sqlite3Isdigit(pIn[-1]) || !sqlite3Isdigit(pIn[1])))
         || (bHex==1 && (!sqlite3Isxdigit(pIn[-1]) || !sqlite3Isxdigit(pIn[1])))
        ){
          sqlite3_Error_सन्देश(pParse, "unrecognized token: \"%s\"", p->u.zToken);
        }
      }
    }जबसम्म( *pIn++ );
    यदि( bHex ) p->op = TK_INTEGER;

    /* tag-20240227-a: If after dequoting, the number is an integer that
    ** fits in 32 bits, then it must be converted into EP_IntValue.  Other
    ** parts of the code expect this.  See also tag-20240227-b. */
    यदि( p->op==TK_INTEGER && sqlite3GetInt32(p->u.zToken, &i_मान) ){
      p->u.i_मान = i_मान;
      p->flags |= EP_Int_मान;
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
शून्य sqlite3DequoteToken(Token *p){
  अचिन्हित पूर्णाङ्क i;
  यदि( p->n<2 ) फर्काऊ;
  यदि( !sqlite3Isquote(p->z[0]) ) फर्काऊ;
  लागि(i=1; i<p->n-1; i++){
    यदि( sqlite3Isquote(p->z[i]) ) फर्काऊ;
  }
  p->n -= 2;
  p->z++;
}

/*
** Generate a Token object from a string
*/
शून्य sqlite3TokenInit(Token *p, वर्ण *z){
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
पूर्णाङ्क sqlite3_stricmp(अचर वर्ण *z_बायाँ, अचर वर्ण *z_दायाँ){
  यदि( z_बायाँ==0 ){
    फर्काऊ z_दायाँ ? -1 : 0;
  }अन्यथा यदि( z_दायाँ==0 ){
    फर्काऊ 1;
  }
  फर्काऊ sqlite3StrICmp(z_बायाँ, z_दायाँ);
}
पूर्णाङ्क sqlite3StrICmp(अचर वर्ण *z_बायाँ, अचर वर्ण *z_दायाँ){
  अचिन्हित वर्ण *a, *b;
  पूर्णाङ्क c, x;
  a = (अचिन्हित वर्ण *)z_बायाँ;
  b = (अचिन्हित वर्ण *)z_दायाँ;
  लागि(;;){
    c = *a;
    x = *b;
    यदि( c==x ){
      यदि( c==0 ) रोक;
    }अन्यथा{
      c = (पूर्णाङ्क)UpperToLower[c] - (पूर्णाङ्क)UpperToLower[x];
      यदि( c ) रोक;
    }
    a++;
    b++;
  }
  फर्काऊ c;
}
पूर्णाङ्क sqlite3_strnicmp(अचर वर्ण *z_बायाँ, अचर वर्ण *z_दायाँ, पूर्णाङ्क N){
  पञ्जिका अचिन्हित वर्ण *a, *b;
  यदि( z_बायाँ==0 ){
    फर्काऊ z_दायाँ ? -1 : 0;
  }अन्यथा यदि( z_दायाँ==0 ){
    फर्काऊ 1;
  }
  a = (अचिन्हित वर्ण *)z_बायाँ;
  b = (अचिन्हित वर्ण *)z_दायाँ;
  जबसम्म( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  फर्काऊ N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/*
** Compute an 8-bit hash on a string that is insensitive to case differences
*/
u8 sqlite3StrIHash(अचर वर्ण *z){
  u8 h = 0;
  यदि( z==0 ) फर्काऊ 0;
  जबसम्म( z[0] ){
    h += UpperToLower[(अचिन्हित वर्ण)z[0]];
    z++;
  }
  फर्काऊ h;
}

#यदि !defined(SQLITE_DISABLE_INTRINSIC)               \
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
स्थिर u64 sqlite3Multiply128(u64 a, u64 b, u64 *pLo){
#यदि defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  *pLo = (u64)r;
  फर्काऊ (u64)(r>>64);
#अथवायदि defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  *pLo = a*b;
  फर्काऊ __umulh(a, b);
#अन्यथा
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
  फर्काऊ a1b1 + (a0b1>>32) + (a1b0>>32) + (t>>32);
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
स्थिर u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo){
#यदि defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  r += ((__uint128_t)aLo * b) >> 32;
  *pLo = (r>>32)&0xffffffff;
  फर्काऊ r>>64;
#अथवायदि defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  u64 r1_hi = __umulh(a,b);
  u64 r1_lo = a*b;
  u64 r2 = (__umulh((u64)aLo,b)<<32) + ((aLo*b)>>32);
  u64 t = r1_lo + r2;
  यदि( t<r1_lo ) r1_hi++;
  *pLo = t>>32;
  फर्काऊ r1_hi;
#अन्यथा
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
  फर्काऊ (r4<<32) + r3;
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
स्थिर u64 powerOfTen(पूर्णाङ्क p, u32 *pLo){
  स्थिर अचर u64 aBase[] = {
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
  स्थिर अचर u64 aScale[] = {
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
  स्थिर अचर अचिन्हित पूर्णाङ्क aScaleLo[] = {
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
  पूर्णाङ्क g, n;
  u64 s, x;
  u32 lo;

  प्रतिज्ञा( p>=POWERSOF10_FIRST && p<=POWERSOF10_LAST );
  यदि( p<0 ){
    यदि( p==(-1) ){
      *pLo = aScaleLo[13];
      फर्काऊ aScale[13];
    }
    g = p/27;
    n = p%27;
    यदि( n ){
      g--;
      n += 27;
    }
  }अन्यथा यदि( p<27 ){
    *pLo = 0;
    फर्काऊ aBase[p];
  }अन्यथा{
    g = p/27;
    n = p%27;
  }
  s = aScale[g+13];
  यदि( n==0 ){
    *pLo = aScaleLo[g+13];
    फर्काऊ s;
  }
  x = sqlite3Multiply160(s,aScaleLo[g+13],aBase[n],&lo);
  यदि( (U64_BIT(63) & x)==0 ){
    x  = x<<1 | ((lo>>31)&1);
    lo = (lo<<1) | 1;
  }
  *pLo = lo;
  फर्काऊ x;
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
स्थिर पूर्णाङ्क pwr10to2(पूर्णाङ्क p){ फर्काऊ (p*108853) >> 15; }
स्थिर पूर्णाङ्क pwr2to10(पूर्णाङ्क p){ फर्काऊ (p*78913) >> 18; }

/*
** Count leading zeros for a 64-bit unsigned integer.
*/
स्थिर पूर्णाङ्क गन्ती_Leading_Zeros(u64 m){
#यदि (defined(__GNUC__) || defined(__clang__)) \
    && !defined(SQLITE_DISABLE_INTRINSIC)
  फर्काऊ __builtin_clzll(m);
#अन्यथा
  पूर्णाङ्क n = 0;
  यदि( m <= 0x00000000ffffffffULL) { n += 32; m <<= 32; }
  यदि( m <= 0x0000ffffffffffffULL) { n += 16; m <<= 16; }
  यदि( m <= 0x00ffffffffffffffULL) { n += 8;  m <<= 8;  }
  यदि( m <= 0x0fffffffffffffffULL) { n += 4;  m <<= 4;  }
  यदि( m <= 0x3fffffffffffffffULL) { n += 2;  m <<= 2;  }
  यदि( m <= 0x7fffffffffffffffULL) { n += 1;            }
  फर्काऊ n;
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
स्थिर शून्य sqlite3Fp2Convert10(u64 m, पूर्णाङ्क e, पूर्णाङ्क n, u64 *pD, पूर्णाङ्क *pP){
  पूर्णाङ्क p;
  u64 h, d1;
  u32 d2;
  प्रतिज्ञा( n>=1 && n<=18 );
  p = n - 1 - pwr2to10(e+63);
  h = sqlite3Multiply128(m, powerOfTen(p,&d2), &d1);
  प्रतिज्ञा( -(e + pwr10to2(p) + 2) >= 0  );
  प्रतिज्ञा( -(e + pwr10to2(p) + 1) <= 63 );
  यदि( n==18 ){
    h >>= -(e + pwr10to2(p) + 2);
    *pD = (h + ((h<<1)&2))>>1;
  }अन्यथा{
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
स्थिर द्विगुण sqlite3Fp10Convert2(u64 d, पूर्णाङ्क p){
  पूर्णाङ्क b, lp, e, adj, s;
  u32 pwr10l, mid1;
  u64 pwr10h, x, hi, lo, sticky, u, m;
  द्विगुण r;
  यदि( p<POWERSOF10_FIRST ) फर्काऊ 0.0;
  यदि( p>POWERSOF10_LAST ) फर्काऊ (द्विगुण)अनन्त;
  b = 64 - गन्ती_Leading_Zeros(d);
  lp = pwr10to2(p);
  e = 53 - b - lp;
  यदि( e > 1074 ){
    यदि( e>=1130 ) फर्काऊ 0.0;
    e = 1074;
  }
  s = -(e-(64-b) + lp + 3);
  pwr10h = powerOfTen(p, &pwr10l);
  यदि( pwr10l!=0 ){
    pwr10h++;
    pwr10l = ~pwr10l;
  }
  x = d<<(64-b);
  hi = sqlite3Multiply128(x,pwr10h,&lo);
  mid1 = lo>>32;
  sticky = 1;
  यदि( (hi & (U64_BIT(s)-1))==0 ) {
    u32 mid2 = sqlite3Multiply128(x,((u64)pwr10l)<<32,&lo)>>32;
    sticky = (mid1-mid2 > 1);
    hi -= mid1 < mid2;
  }
  u = (hi>>s) | sticky;
  adj = (u >= U64_BIT(55)-2);
  यदि( adj ){
    u = (u>>adj) | (u&1);
    e -= adj;
  }
  m = (u + 1 + ((u>>2)&1)) >> 2;
  यदि( e<=(-972) ) फर्काऊ (द्विगुण)अनन्त;
  यदि((m & U64_BIT(52)) != 0){
    m = (m & ~U64_BIT(52)) | ((u64)(1075-e)<<52);
  }
  स_नकल(&r,&m,8);
  फर्काऊ r;
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
पूर्णाङ्क sqlite3AtoF(अचर वर्ण *zIn, द्विगुण *pResult){
#ifndef SQLITE_OMIT_FLOATING_POINT
  अचर अचिन्हित वर्ण *z = (अचर अचिन्हित वर्ण*)zIn;
  पूर्णाङ्क neg = 0;       /* True for a negative value */
  u64 s = 0;         /* mantissa */
  पूर्णाङ्क d = 0;         /* Value is s * pow(10,d) */
  पूर्णाङ्क mState = 0;    /* 1: digit seen 2: fp 4: hard-zero */
  अचिन्हित v;        /* Value of a single digit */

  start_of_text:
  यदि( (v = (अचिन्हित)z[0] - '0')<10 ){
    parse_integer_part:
    mState = 1;
    s = v;
    z++;
    जबसम्म( (v = (अचिन्हित)z[0] - '0')<10 ){
      s = s*10 + v;
      z++;
      यदि( s>=(LARGEST_UINT64-9)/10 ){
        mState = 9;
        जबसम्म( sqlite3Isdigit(z[0]) ){ z++; d++; }
        रोक;
      }
    }
  }अन्यथा यदि( z[0]=='-' ){
    neg = 1;
    z++;
    यदि( (v = (अचिन्हित)z[0] - '0')<10 ) गच्छ parse_integer_part;
  }अन्यथा यदि( z[0]=='+' ){
    z++;
    यदि( (v = (अचिन्हित)z[0] - '0')<10 ) गच्छ parse_integer_part;
  }अन्यथा यदि( sqlite3Isspace(z[0]) ){
    गर{ z++; }जबसम्म( sqlite3Isspace(z[0]) );
    गच्छ start_of_text;
  }अन्यथा{
    s = 0;
  }

  /* if decimal point is present */
  यदि( *z=='.' ){
    z++;
    यदि( sqlite3Isdigit(z[0]) ){
      mState |= 1;
      गर{
        यदि( s<(LARGEST_UINT64-9)/10 ){
          s = s*10 + z[0] - '0';
          d--;
        }अन्यथा{
          mState = 11;
        }
      }जबसम्म( sqlite3Isdigit(*++z) );
    }अन्यथा यदि( mState==0 ){
      *pResult = 0.0;
      फर्काऊ 0;
    }
    mState |= 2;
  }अन्यथा यदि( mState==0 ){
    *pResult = 0.0;
    फर्काऊ 0;
  }

  /* if exponent is present */
  यदि( *z=='e' || *z=='E' ){
    पूर्णाङ्क esign;
    z++;
 
    /* get sign of exponent */
    यदि( *z=='-' ){
      esign = -1;
      z++;
    }अन्यथा{
      esign = +1;
      यदि( *z=='+' ){
        z++;
      }
    }
    /* copy digits to exponent */
    यदि( (v = (अचिन्हित)z[0] - '0')<10 ){
      पूर्णाङ्क exp = v;
      z++;
      mState |= 2;
      जबसम्म( (v = (अचिन्हित)z[0] - '0')<10 ){
        exp = exp<10000 ? (exp*10 + v) : 10000;
        z++;
      }
      d += esign*exp;
    }अन्यथा{
      z--;  /* Leave z[0] at 'e' or '+' or '-',
            ** so that the return is 0 or -1 */
    }
  }

  /* Convert s*pow(10,d) into real */
  यदि( s==0 ){
    *pResult = 0.0;
    mState |= 4;
  }अन्यथा{
    *pResult = sqlite3Fp10Convert2(s,d);
  }
  यदि( neg ) *pResult = -*pResult;
  प्रतिज्ञा( !sqlite3IsNaN(*pResult) );

  /* return true if number and no extra non-whitespace characters after */
  यदि( z[0]==0 ){
    फर्काऊ mState;
  }
  यदि( sqlite3Isspace(z[0]) ){
    गर{ z++; }जबसम्म( sqlite3Isspace(*z) );
    यदि( z[0]==0 ){
      फर्काऊ mState;
    }
  }
  फर्काऊ 0xfffffff0 | mState;
#अन्यथा
  फर्काऊ sqlite3Atoi64(zIn, pResult, म_माप(zIn), SQLITE_UTF8)==0;
#endif /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Digit pairs used to convert a U64 or I64 into text, two digits
** at a time.
*/
स्थिर अचर सङ्घ {
  वर्ण a[201];
  लघु पूर्णाङ्क forceAlignment;
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
#यदि (defined(__arm__) && !defined(__aarch64__)) || \
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
स्थिर पूर्णाङ्क sqlite3UInt64ToText(u64 v, वर्ण *zOut){
  u32 x32, kk;
  पूर्णाङ्क i;
  zOut[SQLITE_U64_DIGITS] = 0;
  i = SQLITE_U64_DIGITS;
  प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[0]) );
  प्रतिज्ञा( TWO_BYTE_ALIGNMENT(zOut) );
  जबसम्म( (v>>32)!=0 ){
    u32 y, x0, x1, y0, y1;
    x32 = v % 100000000;
    v   = v / 100000000;
    y = x32 % 10000;
    x32 /= 10000;
    x1 = x32 / 100;
    x0 = x32 % 100;
    y1 = y / 100;
    y0 = y % 100;
    प्रतिज्ञा( i>=8 );
    i -= 8;
    *(u16*)(&zOut[i]) = *(u16*)&sqlite3DigitPairs.a[x1*2];
    *(u16*)(&zOut[i+2]) = *(u16*)&sqlite3DigitPairs.a[x0*2];
    *(u16*)(&zOut[i+4]) = *(u16*)&sqlite3DigitPairs.a[y1*2];
    *(u16*)(&zOut[i+6]) = *(u16*)&sqlite3DigitPairs.a[y0*2];
  }
  x32 = v;
  जबसम्म( x32>=10 ){
    kk  = x32 % 100;
    x32 = x32 / 100;
    प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk*2]) );
    प्रतिज्ञा( i>=2 );
    i -= 2;
    प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&zOut[i]) );
    *(u16*)(&zOut[i]) = *(u16*)&sqlite3DigitPairs.a[kk*2];
  }
  यदि( x32 ){
    प्रतिज्ञा( i>0 );
    zOut[--i] = x32 + '0';
  }
  फर्काऊ i;
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
पूर्णाङ्क sqlite3Int64ToText(i64 v, वर्ण *zOut){
  पूर्णाङ्क i;
  u64 x;
  सङ्घ {
    वर्ण a[SQLITE_U64_DIGITS+1];
    u16 forceAlignment;
  } u;
  यदि( v>0 ){
    x = v;
  }अन्यथा यदि( v==0 ){
    zOut[0] = '0';
    zOut[1] = 0;
    फर्काऊ 1;
  }अन्यथा{
    x = (v==SMALLEST_INT64) ? ((u64)1)<<63 : (u64)-v;
  }
#ifdef SQLITE_AVOID_U64_DIVIDE
  i = sqlite3UInt64ToText(x, u.a);
#अन्यथा
  i = परिमाण(u.a)-1;
  u.a[i] = 0;
  जबसम्म( x>=10 ){
    पूर्णाङ्क kk = (x%100)*2;
    प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk]) );
    प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&u.a[i-2]) );
    *(u16*)(&u.a[i-2]) = *(u16*)&sqlite3DigitPairs.a[kk];
    i -= 2;
    x /= 100;
  }
  यदि( x ){
    u.a[--i] = x + '0';
  }
#endif /* SQLITE_AVOID_U64_DIVIDE */
  यदि( v<0 ) u.a[--i] = '-';
  स_नकल(zOut, &u.a[i], परिमाण(u.a)-i);
  फर्काऊ परिमाण(u.a)-1-i;
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
स्थिर पूर्णाङ्क compare2pow63(अचर वर्ण *z_अङ्क, पूर्णाङ्क incr){
  पूर्णाङ्क c = 0;
  पूर्णाङ्क i;
                    /* 012345678901234567 */
  अचर वर्ण *pow63 = "922337203685477580";
  लागि(i=0; c==0 && i<18; i++){
    c = (z_अङ्क[i*incr]-pow63[i])*10;
  }
  यदि( c==0 ){
    c = z_अङ्क[18*incr] - '8';
    testcase( c==(-1) );
    testcase( c==0 );
    testcase( c==(+1) );
  }
  फर्काऊ c;
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
पूर्णाङ्क sqlite3Atoi64(अचर वर्ण *z_अङ्क, i64 *p_अङ्क, पूर्णाङ्क length, u8 enc){
  पूर्णाङ्क incr;
  u64 u = 0;
  पूर्णाङ्क neg = 0; /* assume positive */
  पूर्णाङ्क i, j;
  अचिन्हित पूर्णाङ्क c = 0;
  पूर्णाङ्क non_अङ्क = 0;  /* True if input contains UTF16 with high byte non-zero */
  पूर्णाङ्क rc;          /* Baseline return code */
  अचर वर्ण *zStart;
  अचर वर्ण *zEnd = z_अङ्क + length;
  प्रतिज्ञा( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  यदि( enc==SQLITE_UTF8 ){
    incr = 1;
  }अन्यथा{
    incr = 2;
    length &= ~1;
    प्रतिज्ञा( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    लागि(i=3-enc; i<length && z_अङ्क[i]==0; i+=2){}
    non_अङ्क = i<length;
    zEnd = &z_अङ्क[i^1];
    z_अङ्क += (enc&1);
  }
  जबसम्म( z_अङ्क<zEnd && sqlite3Isspace(*z_अङ्क) ) z_अङ्क+=incr;
  यदि( z_अङ्क<zEnd ){
    यदि( *z_अङ्क=='-' ){
      neg = 1;
      z_अङ्क+=incr;
    }अन्यथा यदि( *z_अङ्क=='+' ){
      z_अङ्क+=incr;
    }
  }
  zStart = z_अङ्क;
  जबसम्म( z_अङ्क<zEnd && z_अङ्क[0]=='0' ){ z_अङ्क+=incr; } /* Skip leading zeros. */
  लागि(i=0; &z_अङ्क[i]<zEnd && (c=(अचिन्हित)z_अङ्क[i]-'0')<=9; i+=incr){
    u = u*10 + c;
  }
  testcase( i==18*incr );
  testcase( i==19*incr );
  testcase( i==20*incr );
  यदि( u>LARGEST_INT64 ){
    /* This test and assignment is needed only to suppress UB warnings
    ** from clang and -fsanitize=undefined.  This test and assignment make
    ** the code a little larger and slower, and no harm comes from omitting
    ** them, but we must appease the undefined-behavior pharisees. */
    *p_अङ्क = neg ? SMALLEST_INT64 : LARGEST_INT64;
  }अन्यथा यदि( neg ){
    *p_अङ्क = -(i64)u;
  }अन्यथा{
    *p_अङ्क = (i64)u;
  }
  rc = 0;
  यदि( i==0 && zStart==z_अङ्क ){    /* No digits */
    rc = -1;
  }अन्यथा यदि( non_अङ्क ){            /* UTF16 with high-order bytes non-zero */
    rc = 1;
  }अन्यथा यदि( &z_अङ्क[i]<zEnd ){     /* Extra bytes at the end */
    पूर्णाङ्क jj = i;
    गर{
      यदि( !sqlite3Isspace(z_अङ्क[jj]) ){
        rc = 1;          /* Extra non-space text after the integer */
        रोक;
      }
      jj += incr;
    }जबसम्म( &z_अङ्क[jj]<zEnd );
  }
  यदि( i<19*incr ){
    /* Less than 19 digits, so we know that it fits in 64 bits */
    प्रतिज्ञा( u<=LARGEST_INT64 );
    फर्काऊ rc;
  }अन्यथा{
    /* zNum is a 19-digit numbers.  Compare it against 9223372036854775808. */
    j = i>19*incr ? 1 : compare2pow63(z_अङ्क, incr);
    यदि( j<0 ){
      /* zNum is less than 9223372036854775808 so it fits */
      प्रतिज्ञा( u<=LARGEST_INT64 );
      फर्काऊ rc;
    }अन्यथा{
      *p_अङ्क = neg ? SMALLEST_INT64 : LARGEST_INT64;
      यदि( j>0 ){
        /* zNum is greater than 9223372036854775808 so it overflows */
        फर्काऊ 2;
      }अन्यथा{
        /* zNum is exactly 9223372036854775808.  Fits if negative.  The
        ** special case 2 overflow if positive */
        प्रतिज्ञा( u-1==LARGEST_INT64 );
        फर्काऊ neg ? rc : 3;
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
पूर्णाङ्क sqlite3DecOrHexToI64(अचर वर्ण *z, i64 *pOut){
#ifndef SQLITE_OMIT_HEX_INTEGER
  यदि( z[0]=='0'
   && (z[1]=='x' || z[1]=='X')
  ){
    u64 u = 0;
    पूर्णाङ्क i, k;
    लागि(i=2; z[i]=='0'; i++){}
    लागि(k=i; sqlite3Isxdigit(z[k]); k++){
      u = u*16 + sqlite3HexToInt(z[k]);
    }
    स_नकल(pOut, &u, 8);
    यदि( k-i>16 ) फर्काऊ 2;
    यदि( z[k]!=0 ) फर्काऊ 1;
    फर्काऊ 0;
  }अन्यथा
#endif /* SQLITE_OMIT_HEX_INTEGER */
  {
    पूर्णाङ्क n = (पूर्णाङ्क)(0x3fffffff&म_अखोज(z,"+- \n\t0123456789"));
    यदि( z[n] ) n++;
    फर्काऊ sqlite3Atoi64(z, pOut, n, SQLITE_UTF8);
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
पूर्णाङ्क sqlite3GetInt32(अचर वर्ण *z_अङ्क, पूर्णाङ्क *p_मान){
  sqlite_int64 v = 0;
  पूर्णाङ्क i, c;
  पूर्णाङ्क neg = 0;
  यदि( z_अङ्क[0]=='-' ){
    neg = 1;
    z_अङ्क++;
  }अन्यथा यदि( z_अङ्क[0]=='+' ){
    z_अङ्क++;
  }
#ifndef SQLITE_OMIT_HEX_INTEGER
  अन्यथा यदि( z_अङ्क[0]=='0'
        && (z_अङ्क[1]=='x' || z_अङ्क[1]=='X')
        && sqlite3Isxdigit(z_अङ्क[2])
  ){
    u32 u = 0;
    z_अङ्क += 2;
    जबसम्म( z_अङ्क[0]=='0' ) z_अङ्क++;
    लागि(i=0; i<8 && sqlite3Isxdigit(z_अङ्क[i]); i++){
      u = u*16 + sqlite3HexToInt(z_अङ्क[i]);
    }
    यदि( (u&0x80000000)==0 && sqlite3Isxdigit(z_अङ्क[i])==0 ){
      स_नकल(p_मान, &u, 4);
      फर्काऊ 1;
    }अन्यथा{
      फर्काऊ 0;
    }
  }
#endif
  यदि( !sqlite3Isdigit(z_अङ्क[0]) ) फर्काऊ 0;
  जबसम्म( z_अङ्क[0]=='0' ) z_अङ्क++;
  लागि(i=0; i<11 && (c = z_अङ्क[i] - '0')>=0 && c<=9; i++){
    v = v*10 + c;
  }

  /* The longest decimal representation of a 32 bit integer is 10 digits:
  **
  **             1234567890
  **     2^31 -> 2147483648
  */
  testcase( i==10 );
  यदि( i>10 ){
    फर्काऊ 0;
  }
  testcase( v-neg==2147483647 );
  यदि( v-neg>2147483647 ){
    फर्काऊ 0;
  }
  यदि( neg ){
    v = -v;
  }
  *p_मान = (पूर्णाङ्क)v;
  फर्काऊ 1;
}

/*
** Return a 32-bit integer value extracted from a string.  If the
** string is not an integer, just return 0.
*/
पूर्णाङ्क sqlite3Atoi(अचर वर्ण *z){
  पूर्णाङ्क x = 0;
  sqlite3GetInt32(z, &x);
  फर्काऊ x;
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
शून्य sqlite3FpDecode(FpDecode *p, द्विगुण r, पूर्णाङ्क iRound, पूर्णाङ्क mxRound){
  पूर्णाङ्क i;               /* Index into zBuf[] where to put next character */
  पूर्णाङ्क n;               /* Number of digits */
  u64 v;               /* mantissa */
  पूर्णाङ्क e, exp = 0;      /* Base-2 and base-10 exponent */
  वर्ण *zBuf;          /* Local alias for p->zBuf */
  वर्ण *z;             /* Local alias for p->z */

  p->isSpecial = 0;
  प्रतिज्ञा( mxRound>0 );
  प्रतिज्ञा( iRound>=-100000000 );

  /* Convert negative numbers to positive.  Deal with Infinity, 0.0, and
  ** NaN. */
  यदि( r<0.0 ){
    p->sign = '-';
    r = -r;
  }अन्यथा यदि( r==0.0 ){
    p->sign = '+';
    p->n = 1;
    p->iDP = 1;
    p->z = "0";
    फर्काऊ;
  }अन्यथा{
    p->sign = '+';
  }
  स_नकल(&v,&r,8);
  e = (v>>52)&0x7ff;
  यदि( e==0x7ff ){
    p->isSpecial = 1 + (v!=0x7ff0000000000000LL);
    p->n = 0;
    p->iDP = 0;
    p->z = p->zBuf;
    फर्काऊ;
  }
  v &= 0x000fffffffffffffULL;
  यदि( e==0 ){
    पूर्णाङ्क nn = गन्ती_Leading_Zeros(v);
    v <<= nn;
    e = -1074 - nn;
  }अन्यथा{
    v = (v<<11) | U64_BIT(63);
    e -= 1086;
  }
  sqlite3Fp2Convert10(v, e, (iRound<=0||iRound>=18)?18:iRound+1, &v, &exp);  

  /* Extract significant digits, start at the right-most slot in p->zBuf
  ** and working back to the right.  "i" keeps track of the next slot in
  ** which to store a digit. */
  प्रतिज्ञा( परिमाण(p->zBuf)==SQLITE_U64_DIGITS+1 );
  प्रतिज्ञा( v>0 );
  zBuf = p->zBuf;
#ifdef SQLITE_AVOID_U64_DIVIDE
  i = sqlite3UInt64ToText(v, zBuf);
#अन्यथा
  i = SQLITE_U64_DIGITS;
  जबसम्म( v>=10 ){
    पूर्णाङ्क kk = (v%100)*2;
    प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&sqlite3DigitPairs.a[kk]) );
    प्रतिज्ञा( TWO_BYTE_ALIGNMENT(&zBuf[i]) );
    प्रतिज्ञा( i-2>=0 );
    *(u16*)(&zBuf[i-2]) = *(u16*)&sqlite3DigitPairs.a[kk];
    i -= 2;
    v /= 100;
  }
  यदि( v ){
    प्रतिज्ञा( v<10 );
    प्रतिज्ञा( i>0 );
    zBuf[--i] = v + '0';
  }
#endif /* SQLITE_AVOID_U64_DIVIDE */
  प्रतिज्ञा( i>=0 && i<SQLITE_U64_DIGITS );
  n = SQLITE_U64_DIGITS - i;  /* Total number of digits extracted */
  प्रतिज्ञा( n>0 );
  प्रतिज्ञा( n<=SQLITE_U64_DIGITS );
  p->iDP = n + exp;
  यदि( iRound<=0 ){
    iRound = p->iDP - iRound;
    यदि( iRound==0 && zBuf[i]>='5' ){
      iRound = 1;
      zBuf[--i] = '0';
      n++;
      p->iDP++;
    }
  }
  z = &zBuf[i];  /* z points to the first digit */
  यदि( iRound>0 && (iRound<n || n>mxRound) ){
    यदि( iRound>mxRound ) iRound = mxRound;
    यदि( iRound==17 ){
      /* If the precision is exactly 17, which only happens with the "!"
      ** flag (ex: "%!.17g") then try to reduce the precision if that
      ** yields text that will round-trip to the original floating-point.
      ** value.  Thus, for exaple, 49.47 will render as 49.47, rather than
      ** as 49.469999999999999. */
      यदि( z[15]=='9' && z[14]=='9' ){
        पूर्णाङ्क jj, kk;
        u64 v2;
        लागि(jj=14; jj>0 && z[jj-1]=='9'; jj--){}
        यदि( jj==0 ){
          v2 = 1;
        }अन्यथा{
          v2 = z[0] - '0';
          लागि(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
          v2++;
        }
        यदि( r==sqlite3Fp10Convert2(v2, exp + n - jj) ){
          iRound = jj+1;
        }
      }अन्यथा यदि( p->iDP>=n || (z[15]=='0' && z[14]=='0' && z[13]=='0') ){
        पूर्णाङ्क jj, kk;
        u64 v2;
        प्रतिज्ञा( z[0]!='0' );
        लागि(jj=13; z[jj-1]=='0'; jj--){}
        v2 = z[0] - '0';
        लागि(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
        यदि( r==sqlite3Fp10Convert2(v2, exp + n - jj) ){
          iRound = jj+1;
        }
      }
    }
    n = iRound;
    यदि( z[iRound]>='5' ){
      पूर्णाङ्क j = iRound-1;
      जबसम्म( 1 /*exit-by-break*/ ){
        z[j]++;
        यदि( z[j]<='9' ) रोक;
        z[j] = '0';
        यदि( j==0 ){
          z--;
          z[0] = '1';
          n++;
          p->iDP++;
          रोक;
        }अन्यथा{
          j--;
        }
      }
    }
  }
  प्रतिज्ञा( n>0 );
  जबसम्म( z[n-1]=='0' ){
    n--;
    प्रतिज्ञा( n>0 );
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
पूर्णाङ्क sqlite3GetUInt32(अचर वर्ण *z, u32 *pI){
  u64 v = 0;
  पूर्णाङ्क i;
  लागि(i=0; sqlite3Isdigit(z[i]); i++){
    v = v*10 + z[i] - '0';
    यदि( v>4294967295LL ){ *pI = 0; फर्काऊ 0; }
  }
  यदि( i==0 || z[i]!=0 ){ *pI = 0; फर्काऊ 0; }
  *pI = (u32)v;
  फर्काऊ 1;
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
स्थिर पूर्णाङ्क SQLITE_NOINLINE putVarint64(अचिन्हित वर्ण *p, u64 v){
  पूर्णाङ्क i, j, n;
  u8 buf[10];
  यदि( v & (((u64)0xff000000)<<32) ){
    p[8] = (u8)v;
    v >>= 8;
    लागि(i=7; i>=0; i--){
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    फर्काऊ 9;
  }   
  n = 0;
  गर{
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  }जबसम्म( v!=0 );
  buf[0] &= 0x7f;
  प्रतिज्ञा( n<=9 );
  लागि(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  फर्काऊ n;
}
पूर्णाङ्क sqlite3PutVarint(अचिन्हित वर्ण *p, u64 v){
  यदि( v<=0x7f ){
    p[0] = v&0x7f;
    फर्काऊ 1;
  }
  यदि( v<=0x3fff ){
    p[0] = ((v>>7)&0x7f)|0x80;
    p[1] = v&0x7f;
    फर्काऊ 2;
  }
  फर्काऊ putVarint64(p,v);
}

/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
u8 sqlite3GetVarint(अचर अचिन्हित वर्ण *p, u64 *v){
  u64 i_साँचो = p[0];
  अचर u8 *pStart = p;
  यदि( i_साँचो>=0x80 ){
    u8 x;
    i_साँचो = (i_साँचो<<7) ^ (x = *++p);
    यदि( x>=0x80 ){
      i_साँचो = (i_साँचो<<7) ^ (x = *++p);
      यदि( x>=0x80 ){
        i_साँचो = (i_साँचो<<7) ^ 0x10204000 ^ (x = *++p);
        यदि( x>=0x80 ){
          i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
          यदि( x>=0x80 ){
            i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
            यदि( x>=0x80 ){
              i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
              यदि( x>=0x80 ){
                i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
                यदि( x>=0x80 ){
                  i_साँचो = (i_साँचो<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }अन्यथा{
        i_साँचो ^= 0x204000;
      }
    }अन्यथा{
      i_साँचो ^= 0x4000;
    }
  }
  *v = i_साँचो;
  फर्काऊ (u8)(p - pStart) + 1;
}

/*
** Return the value of a variable-length integer without computing
** its length.  This is an optimization on sqlite3GetVarint() for the
** cases when the return value of sqlite3GetVarint() is not needed.
*/
i64 sqlite3_Varint_मान(अचर अचिन्हित वर्ण *p){
  u64 i_साँचो = p[0];
  यदि( i_साँचो>=0x80 ){
    u8 x;
    i_साँचो = (i_साँचो<<7) ^ (x = *++p);
    यदि( x>=0x80 ){
      i_साँचो = (i_साँचो<<7) ^ (x = *++p);
      यदि( x>=0x80 ){
        i_साँचो = (i_साँचो<<7) ^ 0x10204000 ^ (x = *++p);
        यदि( x>=0x80 ){
          i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
          यदि( x>=0x80 ){
            i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
            यदि( x>=0x80 ){
              i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
              यदि( x>=0x80 ){
                i_साँचो = (i_साँचो<<7) ^ 0x4000 ^ (x = *++p);
                यदि( x>=0x80 ){
                  i_साँचो = (i_साँचो<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }अन्यथा{
        i_साँचो ^= 0x204000;
      }
    }अन्यथा{
      i_साँचो ^= 0x4000;
    }
  }
  फर्काऊ *(i64*)&i_साँचो;
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
u8 sqlite3GetVarint32(अचर अचिन्हित वर्ण *p, u32 *v){
  u64 v64;
  u8 n;

  /* Assume that the single-byte case has already been handled by
  ** the getVarint32() macro */
  प्रतिज्ञा( (p[0] & 0x80)!=0 );

  यदि( (p[1] & 0x80)==0 ){
    /* This is the two-byte case */
    *v = ((p[0]&0x7f)<<7) | p[1];
    फर्काऊ 2;
  }
  यदि( (p[2] & 0x80)==0 ){
    /* This is the three-byte case */
    *v = ((p[0]&0x7f)<<14) | ((p[1]&0x7f)<<7) | p[2];
    फर्काऊ 3;
  }
  /* four or more bytes */
  n = sqlite3GetVarint(p, &v64);
  प्रतिज्ञा( n>3 && n<=9 );
  यदि( (v64 & SQLITE_MAX_U32)!=v64 ){
    *v = 0xffffffff;
  }अन्यथा{
    *v = (u32)v64;
  }
  फर्काऊ n;
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
पूर्णाङ्क sqlite3VarintLen(u64 v){
  पूर्णाङ्क i;
  लागि(i=1; (v >>= 7)!=0; i++){ प्रतिज्ञा( i<10 ); }
  फर्काऊ i;
}


/*
** Read an unsigned 32-bit integer from an unaligned big-endian array of bytes.
*/
u32 sqlite3Get4byte(अचर u8 *p){
#यदि SQLITE_BYTEORDER==4321
  u32 x;
  स_नकल(&x,p,4);
  फर्काऊ x;
#अथवायदि SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u32 x;
  स_नकल(&x,p,4);
  फर्काऊ __builtin_bswap32(x);
#अथवायदि SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u32 x;
  स_नकल(&x,p,4);
  फर्काऊ _byteswap_ulong(x);
#अन्यथा
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  फर्काऊ ((अचिन्हित)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
#endif
}

/* Write an unsigned 32-bit integer into an unaligned big-endian array of bytes.
*/
शून्य sqlite3Put4byte(अचिन्हित वर्ण *p, u32 v){
#यदि SQLITE_BYTEORDER==4321
  स_नकल(p,&v,4);
#अथवायदि SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u32 x = __builtin_bswap32(v);
  स_नकल(p,&x,4);
#अथवायदि SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u32 x = _byteswap_ulong(v);
  स_नकल(p,&x,4);
#अन्यथा
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
SQLITE_OPT_INLINE u64 sqlite3Get8byte(अचर u8 *p){
#यदि SQLITE_BYTEORDER==4321
  u64 x;
  स_नकल(&x,p,8);
  फर्काऊ x;
#अथवायदि SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u64 x;
  स_नकल(&x,p,8);
  फर्काऊ __builtin_bswap64(x);
#अथवायदि SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u64 x;
  स_नकल(&x,p,8);
  फर्काऊ _byteswap_uint64(x);
#अन्यथा
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  फर्काऊ (u64)(
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

#यदि SQLITE_BYTEORDER!=4321  /* Only used for little-endian machines */
/*
** Byte-swap a 64-bit unsigned integer.
*/
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64 x){
#यदि SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  फर्काऊ __builtin_bswap64(x);
#अथवायदि SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  फर्काऊ _byteswap_uint64(x);
#अन्यथा
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  x = (x << 32) | (x >> 32);
  x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) |
      ((x & UINT64_C(0xffff0000ffff0000)) >> 16);
  x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) |
      ((x & UINT64_C(0xff00ff00ff00ff00)) >>  8);
  फर्काऊ x;
#endif
}
#endif /* SQLITE_BYTEORDER!=4321 */

/*
** Translate a single byte of Hex into an integer.
** This routine only works if h really is a valid hexadecimal
** character:  0..9a..fA..F
*/
u8 sqlite3HexToInt(पूर्णाङ्क h){
  प्रतिज्ञा( (h>='0' && h<='9') ||  (h>='a' && h<='f') ||  (h>='A' && h<='F') );
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  फर्काऊ (u8)(h & 0xf);
}

#यदि !defined(SQLITE_OMIT_BLOB_LITERAL)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
शून्य *sqlite3HexToBlob(sqlite3 *db, अचर वर्ण *z, पूर्णाङ्क n){
  वर्ण *zBlob;
  पूर्णाङ्क i;

  zBlob = (वर्ण *)sqlite3DbMallocRawNN(db, n/2 + 1);
  n--;
  यदि( zBlob ){
    लागि(i=0; i<n; i+=2){
      zBlob[i/2] = (sqlite3HexToInt(z[i])<<4) | sqlite3HexToInt(z[i+1]);
    }
    zBlob[i/2] = 0;
  }
  फर्काऊ zBlob;
}
#endif /* !SQLITE_OMIT_BLOB_LITERAL */

/*
** Log an error that is an API call on a connection pointer that should
** not have been used.  The "type" of connection pointer is given as the
** argument.  The zType is a word like "NULL" or "closed" or "invalid".
*/
स्थिर शून्य logBadConnection(अचर वर्ण *zType){
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
पूर्णाङ्क sqlite3SafetyCheckOk(sqlite3 *db){
  u8 eOpenState;
  यदि( db==0 ){
    logBadConnection("NULL");
    फर्काऊ 0;
  }
  eOpenState = db->eOpenState;
  यदि( eOpenState!=SQLITE_STATE_OPEN ){
    यदि( sqlite3SafetyCheckSickOrOk(db) ){
      testcase( sqlite3GlobalConfig.xLog!=0 );
      logBadConnection("unopened");
    }
    फर्काऊ 0;
  }अन्यथा{
    फर्काऊ 1;
  }
}
पूर्णाङ्क sqlite3SafetyCheckSickOrOk(sqlite3 *db){
  u8 eOpenState;
  eOpenState = db->eOpenState;
  यदि( eOpenState!=SQLITE_STATE_SICK &&
      eOpenState!=SQLITE_STATE_OPEN &&
      eOpenState!=SQLITE_STATE_BUSY ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    logBadConnection("invalid");
    फर्काऊ 0;
  }अन्यथा{
    फर्काऊ 1;
  }
}

/*
** Attempt to add, subtract, or multiply the 64-bit signed value iB against
** the other 64-bit signed integer at *pA and store the result in *pA.
** Return 0 on success.  Or if the operation would have resulted in an
** overflow, leave *pA unchanged and return 1.
*/
पूर्णाङ्क sqlite3AddInt64(i64 *pA, i64 iB){
#यदि GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  फर्काऊ __builtin_add_overflow(*pA, iB, pA);
#अन्यथा
  i64 iA = *pA;
  testcase( iA==0 ); testcase( iA==1 );
  testcase( iB==-1 ); testcase( iB==0 );
  यदि( iB>=0 ){
    testcase( iA>0 && LARGEST_INT64 - iA == iB );
    testcase( iA>0 && LARGEST_INT64 - iA == iB - 1 );
    यदि( iA>0 && LARGEST_INT64 - iA < iB ) फर्काऊ 1;
  }अन्यथा{
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 1 );
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 2 );
    यदि( iA<0 && -(iA + LARGEST_INT64) > iB + 1 ) फर्काऊ 1;
  }
  *pA += iB;
  फर्काऊ 0;
#endif
}
पूर्णाङ्क sqlite3SubInt64(i64 *pA, i64 iB){
#यदि GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  फर्काऊ __builtin_sub_overflow(*pA, iB, pA);
#अन्यथा
  testcase( iB==SMALLEST_INT64+1 );
  यदि( iB==SMALLEST_INT64 ){
    testcase( (*pA)==(-1) ); testcase( (*pA)==0 );
    यदि( (*pA)>=0 ) फर्काऊ 1;
    *pA -= iB;
    फर्काऊ 0;
  }अन्यथा{
    फर्काऊ sqlite3AddInt64(pA, -iB);
  }
#endif
}
पूर्णाङ्क sqlite3MulInt64(i64 *pA, i64 iB){
#यदि GCC_VERSION>=5004000 && !defined(__INTEL_COMPILER)
  फर्काऊ __builtin_mul_overflow(*pA, iB, pA);
#अन्यथा
  i64 iA = *pA;
  यदि( iB>0 ){
    यदि( iA>LARGEST_INT64/iB ) फर्काऊ 1;
    यदि( iA<SMALLEST_INT64/iB ) फर्काऊ 1;
  }अन्यथा यदि( iB<0 ){
    यदि( iA>0 ){
      यदि( iB<SMALLEST_INT64/iA ) फर्काऊ 1;
    }अन्यथा यदि( iA<0 ){
      यदि( iB==SMALLEST_INT64 ) फर्काऊ 1;
      यदि( iA==SMALLEST_INT64 ) फर्काऊ 1;
      यदि( -iA>LARGEST_INT64/-iB ) फर्काऊ 1;
    }
  }
  *pA = iA*iB;
  फर्काऊ 0;
#endif
}

/*
** Compute the absolute value of a 32-bit signed integer, if possible.  Or
** if the integer has a value of -2147483648, return +2147483647
*/
पूर्णाङ्क sqlite3AbsInt32(पूर्णाङ्क x){
  यदि( x>=0 ) फर्काऊ x;
  यदि( x==(पूर्णाङ्क)0x80000000 ) फर्काऊ 0x7fffffff;
  फर्काऊ -x;
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
शून्य sqlite3FileSuffix3(अचर वर्ण *zBaseFilename, वर्ण *z){
#यदि SQLITE_ENABLE_8_3_NAMES<2
  यदि( sqlite3_uri_boolean(zBaseFilename, "8_3_names", 0) )
#endif
  {
    पूर्णाङ्क i, sz;
    sz = sqlite3Strlen30(z);
    लागि(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    यदि( z[i]=='.' && ALWAYS(sz>i+4) ) स_हटाओ(&z[i+1], &z[sz-3], 4);
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
  स्थिर अचर अचिन्हित वर्ण x[] = {
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
  यदि( a>=b ){
    यदि( a>b+49 ) फर्काऊ a;
    यदि( a>b+31 ) फर्काऊ a+1;
    फर्काऊ a+x[a-b];
  }अन्यथा{
    यदि( b>a+49 ) फर्काऊ b;
    यदि( b>a+31 ) फर्काऊ b+1;
    फर्काऊ b+x[b-a];
  }
}

/*
** Convert an integer into a LogEst.  In other words, compute an
** approximation for 10*log2(x).
*/
LogEst sqlite3LogEst(u64 x){
  स्थिर LogEst a[] = { 0, 2, 3, 5, 6, 7, 8, 9 };
  LogEst y = 40;
  यदि( x<8 ){
    यदि( x<2 ) फर्काऊ 0;
    जबसम्म( x<8 ){  y -= 10; x <<= 1; }
  }अन्यथा{
#यदि GCC_VERSION>=5004000
    पूर्णाङ्क i = 60 - __builtin_clzll(x);
    y += i*10;
    x >>= i;
#अन्यथा
    जबसम्म( x>255 ){ y += 40; x >>= 4; }  /*OPTIMIZATION-IF-TRUE*/
    जबसम्म( x>15 ){  y += 10; x >>= 1; }
#endif
  }
  फर्काऊ a[x&7] + y - 10;
}

/*
** Convert a double into a LogEst
** In other words, compute an approximation for 10*log2(x).
*/
LogEst sqlite3LogEstFromDouble(द्विगुण x){
  u64 a;
  LogEst e;
  प्रतिज्ञा( परिमाण(x)==8 && परिमाण(a)==8 );
  यदि( x<=1 ) फर्काऊ 0;
  यदि( x<=2000000000 ) फर्काऊ sqlite3LogEst((u64)x);
  स_नकल(&a, &x, 8);
  e = (a>>52) - 1022;
  फर्काऊ e*10;
}

/*
** Convert a LogEst into an integer.
*/
u64 sqlite3LogEstToInt(LogEst x){
  u64 n;
  n = x%10;
  x /= 10;
  यदि( n>=5 ) n -= 2;
  अन्यथा यदि( n>=1 ) n -= 1;
  यदि( x>60 ) फर्काऊ (u64)LARGEST_INT64;
  फर्काऊ x>=3 ? (n+8)<<(x-3) : (n+8)>>(3-x);
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
  अचर वर्ण *zName,     /* Name of symbol to add */
  पूर्णाङ्क nName,             /* Bytes of text in zName */
  पूर्णाङ्क iVal               /* Value to associate with zName */
){
  पूर्णाङ्क nInt;              /* number of sizeof(int) objects needed for zName */
  वर्ण *z;               /* Pointer to where zName will be stored */
  पूर्णाङ्क i;                 /* Index in pIn[] where zName is stored */

  nInt = nName/4 + 3;
  प्रतिज्ञा( pIn==0 || pIn[0]>=3 );  /* Verify ok to add new elements */
  यदि( pIn==0 || pIn[1]+nInt > pIn[0] ){
    /* Enlarge the allocation */
    sqlite3_int64 nAlloc = (pIn ? 2*(sqlite3_int64)pIn[0] : 10) + nInt;
    VList *pOut = sqlite3DbRealloc(db, pIn, nAlloc*परिमाण(पूर्णाङ्क));
    यदि( pOut==0 ) फर्काऊ pIn;
    यदि( pIn==0 ) pOut[1] = 2;
    pIn = pOut;
    pIn[0] = nAlloc;
  }
  i = pIn[1];
  pIn[i] = iVal;
  pIn[i+1] = nInt;
  z = (वर्ण*)&pIn[i+2];
  pIn[1] = i+nInt;
  प्रतिज्ञा( pIn[1]<=pIn[0] );
  स_नकल(z, zName, nName);
  z[nName] = 0;
  फर्काऊ pIn;
}

/*
** Return a pointer to the name of a variable in the given VList that
** has the value iVal.  Or return a NULL if there is no such variable in
** the list
*/
अचर वर्ण *sqlite3_VList_अङ्क_To_Name(VList *pIn, पूर्णाङ्क iVal){
  पूर्णाङ्क i, mx;
  यदि( pIn==0 ) फर्काऊ 0;
  mx = pIn[1];
  i = 2;
  गर{
    यदि( pIn[i]==iVal ) फर्काऊ (वर्ण*)&pIn[i+2];
    i += pIn[i+1];
  }जबसम्म( i<mx );
  फर्काऊ 0;
}

/*
** Return the number of the variable named zName, if it is in VList.
** or return 0 if there is no such variable.
*/
पूर्णाङ्क sqlite3_VList_Name_To_अङ्क(VList *pIn, अचर वर्ण *zName, पूर्णाङ्क nName){
  पूर्णाङ्क i, mx;
  यदि( pIn==0 ) फर्काऊ 0;
  mx = pIn[1];
  i = 2;
  गर{
    अचर वर्ण *z = (अचर वर्ण*)&pIn[i+2];
    यदि( म_भेदन(z,zName,nName)==0 && z[nName]==0 ) फर्काऊ pIn[i];
    i += pIn[i+1];
  }जबसम्म( i<mx );
  फर्काऊ 0;
}
