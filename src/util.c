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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdarg.h>
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
#అన్తర్భావ <math.h>

/* Work around a bug in older Microsoft compilers
** Forum post 2026-04-10T06:33:11z */
#ఒకవేళ !defined(INFINITY) && defined(_MSC_VER)
# define INFINITY HUGE_VAL
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITE_OMIT_FLOATING_POINT */

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
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
పూర్ణాంకం sqlite3FaultSim(పూర్ణాంకం iTest){
  పూర్ణాంకం (*xCallback)(పూర్ణాంకం) = sqlite3GlobalConfig.xTestCallback;
  తిరిగి xCallback ? xCallback(iTest) : SQLITE_OK;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
/*
** Return true if the floating point value is Not a Number (NaN).
**
** Use the math library isnan() function if compiled with SQLITE_HAVE_ISNAN.
** Otherwise, we have our own implementation that works on most systems.
*/
పూర్ణాంకం sqlite3IsNaN(ద్విగుణ x){
  పూర్ణాంకం rc;   /* The value return */
#ఒకవేళ !SQLITE_HAVE_ISNAN && !HAVE_ISNAN
  u64 y;
  జ్ఞాపకం_నకలు(&y,&x,పరిమాణం(y));
  rc = IsNaN(y);
#లేకపోతే
  rc = isnan(x);
#అయితే_ముగింపు /* HAVE_ISNAN */
  testcase( rc );
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_FLOATING_POINT */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
/*
** Return true if the floating point value is NaN or +Inf or -Inf.
*/
పూర్ణాంకం sqlite3_Is_అధికప్రవాహం(ద్విగుణ x){
  పూర్ణాంకం rc;   /* The value return */
  u64 y;
  జ్ఞాపకం_నకలు(&y,&x,పరిమాణం(y));
  rc = IsOvfl(y);
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_FLOATING_POINT */

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
**
** The value returned will never be negative.  Nor will it ever be greater
** than the actual length of the string.  For very long strings (greater
** than 1GiB) the value returned might be less than the true string length.
*/
పూర్ణాంకం sqlite3Strlen30(స్థిరరాశి అక్షరం *z){
  ఒకవేళ( z==0 ) తిరిగి 0;
  తిరిగి 0x3fffffff & (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z);
}

/*
** Return the declared type of a column.  Or return zDflt if the column
** has no declared type.
**
** The column type is an extra string stored after the zero-terminator on
** the column name if and only if the COLFLAG_HASTYPE flag is set.
*/
అక్షరం *sqlite3_స్తంభం_Type(స్తంభం *p_నిలువు, అక్షరం *zDflt){
  ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_HASTYPE ){
    తిరిగి p_నిలువు->zCnName + అక్షరశ్రేణి_పొడవు(p_నిలువు->zCnName) + 1;
  }లేకపోతే ఒకవేళ( p_నిలువు->eCType ){
    ప్రతిజ్ఞా( p_నిలువు->eCType<=SQLITE_N_STDTYPE );
    తిరిగి (అక్షరం*)sqlite3StdType[p_నిలువు->eCType-1];
  }లేకపోతే{
    తిరిగి zDflt;
  }
}

/*
** Helper function for sqlite3Error() - called rarely.  Broken out into
** a separate routine to avoid unnecessary register saves on entry to
** sqlite3Error().
*/
స్థిర SQLITE_NOINLINE శూన్య  sqlite3_Error_ముగింపు(sqlite3 *db, పూర్ణాంకం err_సంకేతం_2){
  ఒకవేళ( db->pErr ) sqlite3_విలువ_సమితి_Null(db->pErr);
  sqlite3SystemError(db, err_సంకేతం_2);
}

/*
** Set the current error code to err_code and clear any prior error message.
** Also set iSysErrno (by calling sqlite3System) if the err_code indicates
** that would be appropriate.
*/
శూన్య sqlite3Error(sqlite3 *db, పూర్ణాంకం err_సంకేతం_2){
  ప్రతిజ్ఞా( db!=0 );
  db->err_సంకేతం = err_సంకేతం_2;
  ఒకవేళ( err_సంకేతం_2 || db->pErr ){
    sqlite3_Error_ముగింపు(db, err_సంకేతం_2);
  }లేకపోతే{
    db->err_Byte_స్థానభ్రంశం = -1;
  }
}

/*
** The equivalent of sqlite3Error(db, SQLITE_OK).  Clear the error state
** and error message.
*/
శూన్య sqlite3ErrorClear(sqlite3 *db){
  ప్రతిజ్ఞా( db!=0 );
  db->err_సంకేతం = SQLITE_OK;
  db->err_Byte_స్థానభ్రంశం = -1;
  ఒకవేళ( db->pErr ) sqlite3_విలువ_సమితి_Null(db->pErr);
}

/*
** Load the sqlite3.iSysErrno field if that is an appropriate thing
** to do based on the SQLite error code in rc.
*/
శూన్య sqlite3SystemError(sqlite3 *db, పూర్ణాంకం rc){
  ఒకవేళ( rc==SQLITE_IOERR_NOMEM ) తిరిగి;
#ఒకవేళ defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
  ఒకవేళ( rc==SQLITE_IOERR_లోపల_పుట ){
    పూర్ణాంకం ii;
    పూర్ణాంకం iErr;
    sqlite3BtreeEnterAll(db);
    కొరకు(ii=0; ii<db->nDb; ii++){
      ఒకవేళ( db->aDb[ii].pBt ){
        iErr = sqlite3PagerWalSystemErrno(sqlite3BtreePager(db->aDb[ii].pBt));
        ఒకవేళ( iErr ){
          db->iSysErrno = iErr;
        }
      }
    }
    sqlite3BtreeLeaveAll(db);
    తిరిగి;
  }
#అయితే_ముగింపు
  rc &= 0xff;
  ఒకవేళ( rc==SQLITE_CANTOPEN || rc==SQLITE_IOERR ){
    db->iSysErrno = sqlite3_Os_Get_చివరి_Error(db->pVfs);
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
శూన్య sqlite3_Error_With_సందేశం(sqlite3 *db, పూర్ణాంకం err_సంకేతం_2, స్థిరరాశి అక్షరం *zFormat, ...){
  ప్రతిజ్ఞా( db!=0 );
  db->err_సంకేతం = err_సంకేతం_2;
  sqlite3SystemError(db, err_సంకేతం_2);
  ఒకవేళ( zFormat==0 ){
    sqlite3Error(db, err_సంకేతం_2);
  }లేకపోతే ఒకవేళ( db->pErr || (db->pErr = sqlite3_విలువ_New(db))!=0 ){
    అక్షరం *z;
    va_list ap;
    va_start(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);
    va_end(ap);
    sqlite3_విలువ_సమితి_Str(db->pErr, -1, z, SQLITE_UTF8, SQLITE_DYNAMIC);
  }
}

/*
** Check for interrupts and invoke progress callback.
*/
శూన్య sqlite3_పురోగతి_Check(Parse *p){
  sqlite3 *db = p->db;
  ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ){
    p->nErr++;
    p->rc = SQLITE_అంతరాయం;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  ఒకవేళ( db->x_పురోగతి ){
    ఒకవేళ( p->rc==SQLITE_అంతరాయం ){
      p->n_పురోగతి_అడుగులు = 0;
    }లేకపోతే ఒకవేళ( (++p->n_పురోగతి_అడుగులు)>=db->n_పురోగతి_Ops ){
      ఒకవేళ( db->x_పురోగతి(db->p_పురోగతి_Arg) ){
        p->nErr++;
        p->rc = SQLITE_అంతరాయం;
      }
      p->n_పురోగతి_అడుగులు = 0;
    }
  }
#అయితే_ముగింపు
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
శూన్య sqlite3_Error_సందేశం(Parse *pParse, స్థిరరాశి అక్షరం *zFormat, ...){
  అక్షరం *z_సందేశం;
  va_list ap;
  sqlite3 *db = pParse->db;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( db->pParse==pParse || db->pParse->pToplevel==pParse );
  db->err_Byte_స్థానభ్రంశం = -2;
  va_start(ap, zFormat);
  z_సందేశం = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ఒకవేళ( db->err_Byte_స్థానభ్రంశం<-1 ) db->err_Byte_స్థానభ్రంశం = -1;
  ఒకవేళ( db->suppressErr ){
    sqlite3DbFree(db, z_సందేశం);
    ఒకవేళ( db->mallocFailed ){
      pParse->nErr++;
      pParse->rc = SQLITE_NOMEM;
    }
  }లేకపోతే{
    pParse->nErr++;
    sqlite3DbFree(db, pParse->z_Err_సందేశం);
    pParse->z_Err_సందేశం = z_సందేశం;
    pParse->rc = SQLITE_ERROR;
    pParse->pWith = 0;
  }
}

/*
** If database connection db is currently parsing SQL, then transfer
** error code errCode to that parser if the parser has not already
** encountered some other kind of error.
*/
పూర్ణాంకం sqlite3_Error_గమ్యం_Parser(sqlite3 *db, పూర్ణాంకం err_సంకేతం){
  Parse *pParse;
  ఒకవేళ( db==0 || (pParse = db->pParse)==0 ) తిరిగి err_సంకేతం;
  pParse->rc = err_సంకేతం;
  pParse->nErr++;
  తిరిగి err_సంకేతం;
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
శూన్య sqlite3Dequote(అక్షరం *z){
  అక్షరం quote;
  పూర్ణాంకం i, j;
  ఒకవేళ( z==0 ) తిరిగి;
  quote = z[0];
  ఒకవేళ( !sqlite3Isquote(quote) ) తిరిగి;
  ఒకవేళ( quote=='[' ) quote = ']';
  కొరకు(i=1, j=0;; i++){
    ప్రతిజ్ఞా( z[i] );
    ఒకవేళ( z[i]==quote ){
      ఒకవేళ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }లేకపోతే{
        ఆపు;
      }
    }లేకపోతే{
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}
శూన్య sqlite3_Dequote_సమాసం(సమాసం *p){
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_Int_విలువ) );
  ప్రతిజ్ఞా( sqlite3Isquote(p->u.zToken[0]) );
  p->flags |= p->u.zToken[0]=='"' ? EP_Quoted|EP_DblQuoted : EP_Quoted;
  sqlite3Dequote(p->u.zToken);
}

/*
** Expression p is a QNUMBER (quoted number). Dequote the value in p->u.zToken
** and set the type to INTEGER or FLOAT. "Quoted" integers or floats are those
** that contain '_' characters that must be removed before further processing.
*/
శూన్య sqlite3_Dequote_సంఖ్య(Parse *pParse, సమాసం *p){
  ప్రతిజ్ఞా( p!=0 || pParse->db->mallocFailed );
  ఒకవేళ( p ){
    స్థిరరాశి అక్షరం *p_లోపల = p->u.zToken;
    అక్షరం *p_బహిర్గతం = p->u.zToken;
    పూర్ణాంకం bHex = (p_లోపల[0]=='0' && (p_లోపల[1]=='x' || p_లోపల[1]=='X'));
    పూర్ణాంకం i_విలువ;
    ప్రతిజ్ఞా( p->op==TK_QNUMBER );
    p->op = TK_INTEGER;
    చేయి {
      ఒకవేళ( *p_లోపల!=SQLITE_DIGIT_SEPARATOR ){
        *p_బహిర్గతం++ = *p_లోపల;
        ఒకవేళ( *p_లోపల=='e' || *p_లోపల=='E' || *p_లోపల=='.' ) p->op = TK_FLOAT;
      }లేకపోతే{
        ఒకవేళ( (bHex==0 && (!sqlite3Isdigit(p_లోపల[-1]) || !sqlite3Isdigit(p_లోపల[1])))
         || (bHex==1 && (!sqlite3Isxdigit(p_లోపల[-1]) || !sqlite3Isxdigit(p_లోపల[1])))
        ){
          sqlite3_Error_సందేశం(pParse, "unrecognized token: \"%s\"", p->u.zToken);
        }
      }
    }అయితే( *p_లోపల++ );
    ఒకవేళ( bHex ) p->op = TK_INTEGER;

    /* tag-20240227-a: If after dequoting, the number is an integer that
    ** fits in 32 bits, then it must be converted into EP_IntValue.  Other
    ** parts of the code expect this.  See also tag-20240227-b. */
    ఒకవేళ( p->op==TK_INTEGER && sqlite3GetInt32(p->u.zToken, &i_విలువ) ){
      p->u.i_విలువ = i_విలువ;
      p->flags |= EP_Int_విలువ;
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
శూన్య sqlite3DequoteToken(Token *p){
  గుర్తులేని పూర్ణాంకం i;
  ఒకవేళ( p->n<2 ) తిరిగి;
  ఒకవేళ( !sqlite3Isquote(p->z[0]) ) తిరిగి;
  కొరకు(i=1; i<p->n-1; i++){
    ఒకవేళ( sqlite3Isquote(p->z[i]) ) తిరిగి;
  }
  p->n -= 2;
  p->z++;
}

/*
** Generate a Token object from a string
*/
శూన్య sqlite3TokenInit(Token *p, అక్షరం *z){
  p->z = z;
  p->n = sqlite3Strlen30(z);
}

/* Convenient short-hand */
#నిర్వచించు పై_గమ్యం_క్రింది sqlite3_పై_గమ్యం_క్రింది

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
పూర్ణాంకం sqlite3_stricmp(స్థిరరాశి అక్షరం *z_ఎడమ, స్థిరరాశి అక్షరం *z_కుడి){
  ఒకవేళ( z_ఎడమ==0 ){
    తిరిగి z_కుడి ? -1 : 0;
  }లేకపోతే ఒకవేళ( z_కుడి==0 ){
    తిరిగి 1;
  }
  తిరిగి sqlite3StrICmp(z_ఎడమ, z_కుడి);
}
పూర్ణాంకం sqlite3StrICmp(స్థిరరాశి అక్షరం *z_ఎడమ, స్థిరరాశి అక్షరం *z_కుడి){
  గుర్తులేని అక్షరం *a, *b;
  పూర్ణాంకం c, x;
  a = (గుర్తులేని అక్షరం *)z_ఎడమ;
  b = (గుర్తులేని అక్షరం *)z_కుడి;
  కొరకు(;;){
    c = *a;
    x = *b;
    ఒకవేళ( c==x ){
      ఒకవేళ( c==0 ) ఆపు;
    }లేకపోతే{
      c = (పూర్ణాంకం)పై_గమ్యం_క్రింది[c] - (పూర్ణాంకం)పై_గమ్యం_క్రింది[x];
      ఒకవేళ( c ) ఆపు;
    }
    a++;
    b++;
  }
  తిరిగి c;
}
పూర్ణాంకం sqlite3_strnicmp(స్థిరరాశి అక్షరం *z_ఎడమ, స్థిరరాశి అక్షరం *z_కుడి, పూర్ణాంకం N){
  నమోదు గుర్తులేని అక్షరం *a, *b;
  ఒకవేళ( z_ఎడమ==0 ){
    తిరిగి z_కుడి ? -1 : 0;
  }లేకపోతే ఒకవేళ( z_కుడి==0 ){
    తిరిగి 1;
  }
  a = (గుర్తులేని అక్షరం *)z_ఎడమ;
  b = (గుర్తులేని అక్షరం *)z_కుడి;
  అయితే( N-- > 0 && *a!=0 && పై_గమ్యం_క్రింది[*a]==పై_గమ్యం_క్రింది[*b]){ a++; b++; }
  తిరిగి N<0 ? 0 : పై_గమ్యం_క్రింది[*a] - పై_గమ్యం_క్రింది[*b];
}

/*
** Compute an 8-bit hash on a string that is insensitive to case differences
*/
u8 sqlite3StrIHash(స్థిరరాశి అక్షరం *z){
  u8 h = 0;
  ఒకవేళ( z==0 ) తిరిగి 0;
  అయితే( z[0] ){
    h += పై_గమ్యం_క్రింది[(గుర్తులేని అక్షరం)z[0]];
    z++;
  }
  తిరిగి h;
}

#ఒకవేళ !defined(SQLITE_DISABLE_INTRINSIC)               \
  && (defined(__GNUC__) || defined(__clang__))       \
  && (defined(__x86_64__) || defined(__aarch64__) || \
      (defined(__riscv) && defined(__riscv_xlen) && (__riscv_xlen>32)))
#నిర్వచించు SQLITE_USE_UINT128
#అయితే_ముగింపు

/*
** Two inputs are multiplied to get a 128-bit result.  Write the
** lower 64-bits of the result into *pLo, and return the high-order
** 64 bits.
*/
స్థిర u64 sqlite3Multiply128(u64 a, u64 b, u64 *pLo){
#ఒకవేళ defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  *pLo = (u64)r;
  తిరిగి (u64)(r>>64);
#లేదావేళ defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  *pLo = a*b;
  తిరిగి __umulh(a, b);
#లేకపోతే
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
  తిరిగి a1b1 + (a0b1>>32) + (a1b0>>32) + (t>>32);
#అయితే_ముగింపు
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
స్థిర u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo){
#ఒకవేళ defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  r += ((__uint128_t)aLo * b) >> 32;
  *pLo = (r>>32)&0xffffffff;
  తిరిగి r>>64;
#లేదావేళ defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  u64 r1_hi = __umulh(a,b);
  u64 r1_lo = a*b;
  u64 r2 = (__umulh((u64)aLo,b)<<32) + ((aLo*b)>>32);
  u64 t = r1_lo + r2;
  ఒకవేళ( t<r1_lo ) r1_hi++;
  *pLo = t>>32;
  తిరిగి r1_hi;
#లేకపోతే
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
  తిరిగి (r4<<32) + r3;
#అయితే_ముగింపు
}

#నిర్వచనం_రద్దు SQLITE_USE_UINT128

/*
** Return a u64 with the N-th bit set.
*/
#నిర్వచించు U64_బిట్టు(N)  (((u64)1)<<(N))

/*
** Range of powers of 10 that we need to deal with when converting
** IEEE754 doubles to and from decimal.
*/
#నిర్వచించు POWERSOF10_మొదటి (-348)
#నిర్వచించు POWERSOF10_చివరి  (+347)

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
స్థిర u64 power_యొక్క_Ten(పూర్ణాంకం p, u32 *pLo){
  స్థిర స్థిరరాశి u64 a_పునాది[] = {
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
  స్థిర స్థిరరాశి u64 aScale[] = {
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
  స్థిర స్థిరరాశి గుర్తులేని పూర్ణాంకం aScaleLo[] = {
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
  పూర్ణాంకం g, n;
  u64 s, x;
  u32 lo;

  ప్రతిజ్ఞా( p>=POWERSOF10_మొదటి && p<=POWERSOF10_చివరి );
  ఒకవేళ( p<0 ){
    ఒకవేళ( p==(-1) ){
      *pLo = aScaleLo[13];
      తిరిగి aScale[13];
    }
    g = p/27;
    n = p%27;
    ఒకవేళ( n ){
      g--;
      n += 27;
    }
  }లేకపోతే ఒకవేళ( p<27 ){
    *pLo = 0;
    తిరిగి a_పునాది[p];
  }లేకపోతే{
    g = p/27;
    n = p%27;
  }
  s = aScale[g+13];
  ఒకవేళ( n==0 ){
    *pLo = aScaleLo[g+13];
    తిరిగి s;
  }
  x = sqlite3Multiply160(s,aScaleLo[g+13],a_పునాది[n],&lo);
  ఒకవేళ( (U64_బిట్టు(63) & x)==0 ){
    x  = x<<1 | ((lo>>31)&1);
    lo = (lo<<1) | 1;
  }
  *pLo = lo;
  తిరిగి x;
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
స్థిర పూర్ణాంకం pwr10to2(పూర్ణాంకం p){ తిరిగి (p*108853) >> 15; }
స్థిర పూర్ణాంకం pwr2to10(పూర్ణాంకం p){ తిరిగి (p*78913) >> 18; }

/*
** Count leading zeros for a 64-bit unsigned integer.
*/
స్థిర పూర్ణాంకం లెక్క_Leading_Zeros(u64 m){
#ఒకవేళ (defined(__GNUC__) || defined(__clang__)) \
    && !defined(SQLITE_DISABLE_INTRINSIC)
  తిరిగి __builtin_clzll(m);
#లేకపోతే
  పూర్ణాంకం n = 0;
  ఒకవేళ( m <= 0x00000000ffffffffULL) { n += 32; m <<= 32; }
  ఒకవేళ( m <= 0x0000ffffffffffffULL) { n += 16; m <<= 16; }
  ఒకవేళ( m <= 0x00ffffffffffffffULL) { n += 8;  m <<= 8;  }
  ఒకవేళ( m <= 0x0fffffffffffffffULL) { n += 4;  m <<= 4;  }
  ఒకవేళ( m <= 0x3fffffffffffffffULL) { n += 2;  m <<= 2;  }
  ఒకవేళ( m <= 0x7fffffffffffffffULL) { n += 1;            }
  తిరిగి n;
#అయితే_ముగింపు
}

/*
** Given m and e, which represent a quantity r == m*pow(2,e),
** return values *pD and *pP such that r == (*pD)*pow(10,*pP),
** approximately.  *pD should contain at least n significant digits.
**
** The input m is required to have its highest bit set.  In other words,
** m should be left-shifted, and e decremented, to maximize the value of m.
*/
స్థిర శూన్య sqlite3Fp2Convert10(u64 m, పూర్ణాంకం e, పూర్ణాంకం n, u64 *pD, పూర్ణాంకం *pP){
  పూర్ణాంకం p;
  u64 h, d1;
  u32 d2;
  ప్రతిజ్ఞా( n>=1 && n<=18 );
  p = n - 1 - pwr2to10(e+63);
  h = sqlite3Multiply128(m, power_యొక్క_Ten(p,&d2), &d1);
  ప్రతిజ్ఞా( -(e + pwr10to2(p) + 2) >= 0  );
  ప్రతిజ్ఞా( -(e + pwr10to2(p) + 1) <= 63 );
  ఒకవేళ( n==18 ){
    h >>= -(e + pwr10to2(p) + 2);
    *pD = (h + ((h<<1)&2))>>1;
  }లేకపోతే{
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
స్థిర ద్విగుణ sqlite3Fp10Convert2(u64 d, పూర్ణాంకం p){
  పూర్ణాంకం b, lp, e, adj, s;
  u32 pwr10l, mid1;
  u64 pwr10h, x, hi, lo, sticky, u, m;
  ద్విగుణ r;
  ఒకవేళ( p<POWERSOF10_మొదటి ) తిరిగి 0.0;
  ఒకవేళ( p>POWERSOF10_చివరి ) తిరిగి (ద్విగుణ)INFINITY;
  b = 64 - లెక్క_Leading_Zeros(d);
  lp = pwr10to2(p);
  e = 53 - b - lp;
  ఒకవేళ( e > 1074 ){
    ఒకవేళ( e>=1130 ) తిరిగి 0.0;
    e = 1074;
  }
  s = -(e-(64-b) + lp + 3);
  pwr10h = power_యొక్క_Ten(p, &pwr10l);
  ఒకవేళ( pwr10l!=0 ){
    pwr10h++;
    pwr10l = ~pwr10l;
  }
  x = d<<(64-b);
  hi = sqlite3Multiply128(x,pwr10h,&lo);
  mid1 = lo>>32;
  sticky = 1;
  ఒకవేళ( (hi & (U64_బిట్టు(s)-1))==0 ) {
    u32 mid2 = sqlite3Multiply128(x,((u64)pwr10l)<<32,&lo)>>32;
    sticky = (mid1-mid2 > 1);
    hi -= mid1 < mid2;
  }
  u = (hi>>s) | sticky;
  adj = (u >= U64_బిట్టు(55)-2);
  ఒకవేళ( adj ){
    u = (u>>adj) | (u&1);
    e -= adj;
  }
  m = (u + 1 + ((u>>2)&1)) >> 2;
  ఒకవేళ( e<=(-972) ) తిరిగి (ద్విగుణ)INFINITY;
  ఒకవేళ((m & U64_బిట్టు(52)) != 0){
    m = (m & ~U64_బిట్టు(52)) | ((u64)(1075-e)<<52);
  }
  జ్ఞాపకం_నకలు(&r,&m,8);
  తిరిగి r;
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
పూర్ణాంకం sqlite3AtoF(స్థిరరాశి అక్షరం *z_లోపల, ద్విగుణ *p_ఫలితం){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
  స్థిరరాశి గుర్తులేని అక్షరం *z = (స్థిరరాశి గుర్తులేని అక్షరం*)z_లోపల;
  పూర్ణాంకం neg = 0;       /* True for a negative value */
  u64 s = 0;         /* mantissa */
  పూర్ణాంకం d = 0;         /* Value is s * pow(10,d) */
  పూర్ణాంకం mState = 0;    /* 1: digit seen 2: fp 4: hard-zero */
  గుర్తులేని v;        /* Value of a single digit */

  ప్రారంభం_యొక్క_పాఠ్యం:
  ఒకవేళ( (v = (గుర్తులేని)z[0] - '0')<10 ){
    parse_integer_part:
    mState = 1;
    s = v;
    z++;
    అయితే( (v = (గుర్తులేని)z[0] - '0')<10 ){
      s = s*10 + v;
      z++;
      ఒకవేళ( s>=(అతిపెద్ద_UINT64-9)/10 ){
        mState = 9;
        అయితే( sqlite3Isdigit(z[0]) ){ z++; d++; }
        ఆపు;
      }
    }
  }లేకపోతే ఒకవేళ( z[0]=='-' ){
    neg = 1;
    z++;
    ఒకవేళ( (v = (గుర్తులేని)z[0] - '0')<10 ) వెళ్ళు parse_integer_part;
  }లేకపోతే ఒకవేళ( z[0]=='+' ){
    z++;
    ఒకవేళ( (v = (గుర్తులేని)z[0] - '0')<10 ) వెళ్ళు parse_integer_part;
  }లేకపోతే ఒకవేళ( sqlite3Isspace(z[0]) ){
    చేయి{ z++; }అయితే( sqlite3Isspace(z[0]) );
    వెళ్ళు ప్రారంభం_యొక్క_పాఠ్యం;
  }లేకపోతే{
    s = 0;
  }

  /* if decimal point is present */
  ఒకవేళ( *z=='.' ){
    z++;
    ఒకవేళ( sqlite3Isdigit(z[0]) ){
      mState |= 1;
      చేయి{
        ఒకవేళ( s<(అతిపెద్ద_UINT64-9)/10 ){
          s = s*10 + z[0] - '0';
          d--;
        }లేకపోతే{
          mState = 11;
        }
      }అయితే( sqlite3Isdigit(*++z) );
    }లేకపోతే ఒకవేళ( mState==0 ){
      *p_ఫలితం = 0.0;
      తిరిగి 0;
    }
    mState |= 2;
  }లేకపోతే ఒకవేళ( mState==0 ){
    *p_ఫలితం = 0.0;
    తిరిగి 0;
  }

  /* if exponent is present */
  ఒకవేళ( *z=='e' || *z=='E' ){
    పూర్ణాంకం esign;
    z++;
 
    /* get sign of exponent */
    ఒకవేళ( *z=='-' ){
      esign = -1;
      z++;
    }లేకపోతే{
      esign = +1;
      ఒకవేళ( *z=='+' ){
        z++;
      }
    }
    /* copy digits to exponent */
    ఒకవేళ( (v = (గుర్తులేని)z[0] - '0')<10 ){
      పూర్ణాంకం ఘాతాంకం = v;
      z++;
      mState |= 2;
      అయితే( (v = (గుర్తులేని)z[0] - '0')<10 ){
        ఘాతాంకం = ఘాతాంకం<10000 ? (ఘాతాంకం*10 + v) : 10000;
        z++;
      }
      d += esign*ఘాతాంకం;
    }లేకపోతే{
      z--;  /* Leave z[0] at 'e' or '+' or '-',
            ** so that the return is 0 or -1 */
    }
  }

  /* Convert s*pow(10,d) into real */
  ఒకవేళ( s==0 ){
    *p_ఫలితం = 0.0;
    mState |= 4;
  }లేకపోతే{
    *p_ఫలితం = sqlite3Fp10Convert2(s,d);
  }
  ఒకవేళ( neg ) *p_ఫలితం = -*p_ఫలితం;
  ప్రతిజ్ఞా( !sqlite3IsNaN(*p_ఫలితం) );

  /* return true if number and no extra non-whitespace characters after */
  ఒకవేళ( z[0]==0 ){
    తిరిగి mState;
  }
  ఒకవేళ( sqlite3Isspace(z[0]) ){
    చేయి{ z++; }అయితే( sqlite3Isspace(*z) );
    ఒకవేళ( z[0]==0 ){
      తిరిగి mState;
    }
  }
  తిరిగి 0xfffffff0 | mState;
#లేకపోతే
  తిరిగి sqlite3Atoi64(z_లోపల, p_ఫలితం, అక్షరశ్రేణి_పొడవు(z_లోపల), SQLITE_UTF8)==0;
#అయితే_ముగింపు /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Digit pairs used to convert a U64 or I64 into text, two digits
** at a time.
*/
స్థిర స్థిరరాశి సంఘం {
  అక్షరం a[201];
  పొట్టి పూర్ణాంకం forceAlignment;
} sqlite3_Digit_జతలు = {
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
#ఒకవేళ (defined(__arm__) && !defined(__aarch64__)) || \
    (defined(__ppc__) && !defined(__ppc64__))
# define SQLITE_AVOID_U64_DIVIDE 1
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_AVOID_U64_DIVIDE
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
స్థిర పూర్ణాంకం sqlite3_UInt64_గమ్యం_పాఠ్యం(u64 v, అక్షరం *z_బహిర్గతం){
  u32 x32, kk;
  పూర్ణాంకం i;
  z_బహిర్గతం[SQLITE_U64_DIGITS] = 0;
  i = SQLITE_U64_DIGITS;
  ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_జతలు.a[0]) );
  ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(z_బహిర్గతం) );
  అయితే( (v>>32)!=0 ){
    u32 y, x0, x1, y0, y1;
    x32 = v % 100000000;
    v   = v / 100000000;
    y = x32 % 10000;
    x32 /= 10000;
    x1 = x32 / 100;
    x0 = x32 % 100;
    y1 = y / 100;
    y0 = y % 100;
    ప్రతిజ్ఞా( i>=8 );
    i -= 8;
    *(u16*)(&z_బహిర్గతం[i]) = *(u16*)&sqlite3_Digit_జతలు.a[x1*2];
    *(u16*)(&z_బహిర్గతం[i+2]) = *(u16*)&sqlite3_Digit_జతలు.a[x0*2];
    *(u16*)(&z_బహిర్గతం[i+4]) = *(u16*)&sqlite3_Digit_జతలు.a[y1*2];
    *(u16*)(&z_బహిర్గతం[i+6]) = *(u16*)&sqlite3_Digit_జతలు.a[y0*2];
  }
  x32 = v;
  అయితే( x32>=10 ){
    kk  = x32 % 100;
    x32 = x32 / 100;
    ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_జతలు.a[kk*2]) );
    ప్రతిజ్ఞా( i>=2 );
    i -= 2;
    ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&z_బహిర్గతం[i]) );
    *(u16*)(&z_బహిర్గతం[i]) = *(u16*)&sqlite3_Digit_జతలు.a[kk*2];
  }
  ఒకవేళ( x32 ){
    ప్రతిజ్ఞా( i>0 );
    z_బహిర్గతం[--i] = x32 + '0';
  }
  తిరిగి i;
}
#అయితే_ముగింపు /* defined(SQLITE_AVOID_U64_DIVIDE) */

/*
** Render an signed 64-bit integer as text.  Store the result in zOut[] and
** return the length of the string that was stored, in bytes.  The value
** returned does not include the zero terminator at the end of the output
** string.
**
** The caller must ensure that zOut[] is at least 21 bytes in size.
*/
పూర్ణాంకం sqlite3_Int64_గమ్యం_పాఠ్యం(i64 v, అక్షరం *z_బహిర్గతం){
  పూర్ణాంకం i;
  u64 x;
  సంఘం {
    అక్షరం a[SQLITE_U64_DIGITS+1];
    u16 forceAlignment;
  } u;
  ఒకవేళ( v>0 ){
    x = v;
  }లేకపోతే ఒకవేళ( v==0 ){
    z_బహిర్గతం[0] = '0';
    z_బహిర్గతం[1] = 0;
    తిరిగి 1;
  }లేకపోతే{
    x = (v==అతిచిన్న_INT64) ? ((u64)1)<<63 : (u64)-v;
  }
#నిర్వచితమైతే SQLITE_AVOID_U64_DIVIDE
  i = sqlite3_UInt64_గమ్యం_పాఠ్యం(x, u.a);
#లేకపోతే
  i = పరిమాణం(u.a)-1;
  u.a[i] = 0;
  అయితే( x>=10 ){
    పూర్ణాంకం kk = (x%100)*2;
    ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_జతలు.a[kk]) );
    ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&u.a[i-2]) );
    *(u16*)(&u.a[i-2]) = *(u16*)&sqlite3_Digit_జతలు.a[kk];
    i -= 2;
    x /= 100;
  }
  ఒకవేళ( x ){
    u.a[--i] = x + '0';
  }
#అయితే_ముగింపు /* SQLITE_AVOID_U64_DIVIDE */
  ఒకవేళ( v<0 ) u.a[--i] = '-';
  జ్ఞాపకం_నకలు(z_బహిర్గతం, &u.a[i], పరిమాణం(u.a)-i);
  తిరిగి పరిమాణం(u.a)-1-i;
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
స్థిర పూర్ణాంకం compare2pow63(స్థిరరాశి అక్షరం *z_అంకె, పూర్ణాంకం incr){
  పూర్ణాంకం c = 0;
  పూర్ణాంకం i;
                    /* 012345678901234567 */
  స్థిరరాశి అక్షరం *pow63 = "922337203685477580";
  కొరకు(i=0; c==0 && i<18; i++){
    c = (z_అంకె[i*incr]-pow63[i])*10;
  }
  ఒకవేళ( c==0 ){
    c = z_అంకె[18*incr] - '8';
    testcase( c==(-1) );
    testcase( c==0 );
    testcase( c==(+1) );
  }
  తిరిగి c;
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
పూర్ణాంకం sqlite3Atoi64(స్థిరరాశి అక్షరం *z_అంకె, i64 *p_అంకె, పూర్ణాంకం నిడివి, u8 enc){
  పూర్ణాంకం incr;
  u64 u = 0;
  పూర్ణాంకం neg = 0; /* assume positive */
  పూర్ణాంకం i, j;
  గుర్తులేని పూర్ణాంకం c = 0;
  పూర్ణాంకం non_అంకె = 0;  /* True if input contains UTF16 with high byte non-zero */
  పూర్ణాంకం rc;          /* Baseline return code */
  స్థిరరాశి అక్షరం *z_ప్రారంభం;
  స్థిరరాశి అక్షరం *z_చివర = z_అంకె + నిడివి;
  ప్రతిజ్ఞా( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  ఒకవేళ( enc==SQLITE_UTF8 ){
    incr = 1;
  }లేకపోతే{
    incr = 2;
    నిడివి &= ~1;
    ప్రతిజ్ఞా( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    కొరకు(i=3-enc; i<నిడివి && z_అంకె[i]==0; i+=2){}
    non_అంకె = i<నిడివి;
    z_చివర = &z_అంకె[i^1];
    z_అంకె += (enc&1);
  }
  అయితే( z_అంకె<z_చివర && sqlite3Isspace(*z_అంకె) ) z_అంకె+=incr;
  ఒకవేళ( z_అంకె<z_చివర ){
    ఒకవేళ( *z_అంకె=='-' ){
      neg = 1;
      z_అంకె+=incr;
    }లేకపోతే ఒకవేళ( *z_అంకె=='+' ){
      z_అంకె+=incr;
    }
  }
  z_ప్రారంభం = z_అంకె;
  అయితే( z_అంకె<z_చివర && z_అంకె[0]=='0' ){ z_అంకె+=incr; } /* Skip leading zeros. */
  కొరకు(i=0; &z_అంకె[i]<z_చివర && (c=(గుర్తులేని)z_అంకె[i]-'0')<=9; i+=incr){
    u = u*10 + c;
  }
  testcase( i==18*incr );
  testcase( i==19*incr );
  testcase( i==20*incr );
  ఒకవేళ( u>అతిపెద్ద_INT64 ){
    /* This test and assignment is needed only to suppress UB warnings
    ** from clang and -fsanitize=undefined.  This test and assignment make
    ** the code a little larger and slower, and no harm comes from omitting
    ** them, but we must appease the undefined-behavior pharisees. */
    *p_అంకె = neg ? అతిచిన్న_INT64 : అతిపెద్ద_INT64;
  }లేకపోతే ఒకవేళ( neg ){
    *p_అంకె = -(i64)u;
  }లేకపోతే{
    *p_అంకె = (i64)u;
  }
  rc = 0;
  ఒకవేళ( i==0 && z_ప్రారంభం==z_అంకె ){    /* No digits */
    rc = -1;
  }లేకపోతే ఒకవేళ( non_అంకె ){            /* UTF16 with high-order bytes non-zero */
    rc = 1;
  }లేకపోతే ఒకవేళ( &z_అంకె[i]<z_చివర ){     /* Extra bytes at the end */
    పూర్ణాంకం jj = i;
    చేయి{
      ఒకవేళ( !sqlite3Isspace(z_అంకె[jj]) ){
        rc = 1;          /* Extra non-space text after the integer */
        ఆపు;
      }
      jj += incr;
    }అయితే( &z_అంకె[jj]<z_చివర );
  }
  ఒకవేళ( i<19*incr ){
    /* Less than 19 digits, so we know that it fits in 64 bits */
    ప్రతిజ్ఞా( u<=అతిపెద్ద_INT64 );
    తిరిగి rc;
  }లేకపోతే{
    /* zNum is a 19-digit numbers.  Compare it against 9223372036854775808. */
    j = i>19*incr ? 1 : compare2pow63(z_అంకె, incr);
    ఒకవేళ( j<0 ){
      /* zNum is less than 9223372036854775808 so it fits */
      ప్రతిజ్ఞా( u<=అతిపెద్ద_INT64 );
      తిరిగి rc;
    }లేకపోతే{
      *p_అంకె = neg ? అతిచిన్న_INT64 : అతిపెద్ద_INT64;
      ఒకవేళ( j>0 ){
        /* zNum is greater than 9223372036854775808 so it overflows */
        తిరిగి 2;
      }లేకపోతే{
        /* zNum is exactly 9223372036854775808.  Fits if negative.  The
        ** special case 2 overflow if positive */
        ప్రతిజ్ఞా( u-1==అతిపెద్ద_INT64 );
        తిరిగి neg ? rc : 3;
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
పూర్ణాంకం sqlite3_Dec_Or_Hex_గమ్యం_I64(స్థిరరాశి అక్షరం *z, i64 *p_బహిర్గతం){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_HEX_INTEGER
  ఒకవేళ( z[0]=='0'
   && (z[1]=='x' || z[1]=='X')
  ){
    u64 u = 0;
    పూర్ణాంకం i, k;
    కొరకు(i=2; z[i]=='0'; i++){}
    కొరకు(k=i; sqlite3Isxdigit(z[k]); k++){
      u = u*16 + sqlite3_Hex_గమ్యం_Int(z[k]);
    }
    జ్ఞాపకం_నకలు(p_బహిర్గతం, &u, 8);
    ఒకవేళ( k-i>16 ) తిరిగి 2;
    ఒకవేళ( z[k]!=0 ) తిరిగి 1;
    తిరిగి 0;
  }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_HEX_INTEGER */
  {
    పూర్ణాంకం n = (పూర్ణాంకం)(0x3fffffff&strspn(z,"+- \n\t0123456789"));
    ఒకవేళ( z[n] ) n++;
    తిరిగి sqlite3Atoi64(z, p_బహిర్గతం, n, SQLITE_UTF8);
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
పూర్ణాంకం sqlite3GetInt32(స్థిరరాశి అక్షరం *z_అంకె, పూర్ణాంకం *p_విలువ){
  sqlite_int64 v = 0;
  పూర్ణాంకం i, c;
  పూర్ణాంకం neg = 0;
  ఒకవేళ( z_అంకె[0]=='-' ){
    neg = 1;
    z_అంకె++;
  }లేకపోతే ఒకవేళ( z_అంకె[0]=='+' ){
    z_అంకె++;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_HEX_INTEGER
  లేకపోతే ఒకవేళ( z_అంకె[0]=='0'
        && (z_అంకె[1]=='x' || z_అంకె[1]=='X')
        && sqlite3Isxdigit(z_అంకె[2])
  ){
    u32 u = 0;
    z_అంకె += 2;
    అయితే( z_అంకె[0]=='0' ) z_అంకె++;
    కొరకు(i=0; i<8 && sqlite3Isxdigit(z_అంకె[i]); i++){
      u = u*16 + sqlite3_Hex_గమ్యం_Int(z_అంకె[i]);
    }
    ఒకవేళ( (u&0x80000000)==0 && sqlite3Isxdigit(z_అంకె[i])==0 ){
      జ్ఞాపకం_నకలు(p_విలువ, &u, 4);
      తిరిగి 1;
    }లేకపోతే{
      తిరిగి 0;
    }
  }
#అయితే_ముగింపు
  ఒకవేళ( !sqlite3Isdigit(z_అంకె[0]) ) తిరిగి 0;
  అయితే( z_అంకె[0]=='0' ) z_అంకె++;
  కొరకు(i=0; i<11 && (c = z_అంకె[i] - '0')>=0 && c<=9; i++){
    v = v*10 + c;
  }

  /* The longest decimal representation of a 32 bit integer is 10 digits:
  **
  **             1234567890
  **     2^31 -> 2147483648
  */
  testcase( i==10 );
  ఒకవేళ( i>10 ){
    తిరిగి 0;
  }
  testcase( v-neg==2147483647 );
  ఒకవేళ( v-neg>2147483647 ){
    తిరిగి 0;
  }
  ఒకవేళ( neg ){
    v = -v;
  }
  *p_విలువ = (పూర్ణాంకం)v;
  తిరిగి 1;
}

/*
** Return a 32-bit integer value extracted from a string.  If the
** string is not an integer, just return 0.
*/
పూర్ణాంకం sqlite3Atoi(స్థిరరాశి అక్షరం *z){
  పూర్ణాంకం x = 0;
  sqlite3GetInt32(z, &x);
  తిరిగి x;
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
శూన్య sqlite3FpDecode(FpDecode *p, ద్విగుణ r, పూర్ణాంకం i_విడత, పూర్ణాంకం mx_విడత){
  పూర్ణాంకం i;               /* Index into zBuf[] where to put next character */
  పూర్ణాంకం n;               /* Number of digits */
  u64 v;               /* mantissa */
  పూర్ణాంకం e, ఘాతాంకం = 0;      /* Base-2 and base-10 exponent */
  అక్షరం *zBuf;          /* Local alias for p->zBuf */
  అక్షరం *z;             /* Local alias for p->z */

  p->isSpecial = 0;
  ప్రతిజ్ఞా( mx_విడత>0 );
  ప్రతిజ్ఞా( i_విడత>=-100000000 );

  /* Convert negative numbers to positive.  Deal with Infinity, 0.0, and
  ** NaN. */
  ఒకవేళ( r<0.0 ){
    p->sign = '-';
    r = -r;
  }లేకపోతే ఒకవేళ( r==0.0 ){
    p->sign = '+';
    p->n = 1;
    p->iDP = 1;
    p->z = "0";
    తిరిగి;
  }లేకపోతే{
    p->sign = '+';
  }
  జ్ఞాపకం_నకలు(&v,&r,8);
  e = (v>>52)&0x7ff;
  ఒకవేళ( e==0x7ff ){
    p->isSpecial = 1 + (v!=0x7ff0000000000000LL);
    p->n = 0;
    p->iDP = 0;
    p->z = p->zBuf;
    తిరిగి;
  }
  v &= 0x000fffffffffffffULL;
  ఒకవేళ( e==0 ){
    పూర్ణాంకం nn = లెక్క_Leading_Zeros(v);
    v <<= nn;
    e = -1074 - nn;
  }లేకపోతే{
    v = (v<<11) | U64_బిట్టు(63);
    e -= 1086;
  }
  sqlite3Fp2Convert10(v, e, (i_విడత<=0||i_విడత>=18)?18:i_విడత+1, &v, &ఘాతాంకం);  

  /* Extract significant digits, start at the right-most slot in p->zBuf
  ** and working back to the right.  "i" keeps track of the next slot in
  ** which to store a digit. */
  ప్రతిజ్ఞా( పరిమాణం(p->zBuf)==SQLITE_U64_DIGITS+1 );
  ప్రతిజ్ఞా( v>0 );
  zBuf = p->zBuf;
#నిర్వచితమైతే SQLITE_AVOID_U64_DIVIDE
  i = sqlite3_UInt64_గమ్యం_పాఠ్యం(v, zBuf);
#లేకపోతే
  i = SQLITE_U64_DIGITS;
  అయితే( v>=10 ){
    పూర్ణాంకం kk = (v%100)*2;
    ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_జతలు.a[kk]) );
    ప్రతిజ్ఞా( TWO_BYTE_ALIGNMENT(&zBuf[i]) );
    ప్రతిజ్ఞా( i-2>=0 );
    *(u16*)(&zBuf[i-2]) = *(u16*)&sqlite3_Digit_జతలు.a[kk];
    i -= 2;
    v /= 100;
  }
  ఒకవేళ( v ){
    ప్రతిజ్ఞా( v<10 );
    ప్రతిజ్ఞా( i>0 );
    zBuf[--i] = v + '0';
  }
#అయితే_ముగింపు /* SQLITE_AVOID_U64_DIVIDE */
  ప్రతిజ్ఞా( i>=0 && i<SQLITE_U64_DIGITS );
  n = SQLITE_U64_DIGITS - i;  /* Total number of digits extracted */
  ప్రతిజ్ఞా( n>0 );
  ప్రతిజ్ఞా( n<=SQLITE_U64_DIGITS );
  p->iDP = n + ఘాతాంకం;
  ఒకవేళ( i_విడత<=0 ){
    i_విడత = p->iDP - i_విడత;
    ఒకవేళ( i_విడత==0 && zBuf[i]>='5' ){
      i_విడత = 1;
      zBuf[--i] = '0';
      n++;
      p->iDP++;
    }
  }
  z = &zBuf[i];  /* z points to the first digit */
  ఒకవేళ( i_విడత>0 && (i_విడత<n || n>mx_విడత) ){
    ఒకవేళ( i_విడత>mx_విడత ) i_విడత = mx_విడత;
    ఒకవేళ( i_విడత==17 ){
      /* If the precision is exactly 17, which only happens with the "!"
      ** flag (ex: "%!.17g") then try to reduce the precision if that
      ** yields text that will round-trip to the original floating-point.
      ** value.  Thus, for exaple, 49.47 will render as 49.47, rather than
      ** as 49.469999999999999. */
      ఒకవేళ( z[15]=='9' && z[14]=='9' ){
        పూర్ణాంకం jj, kk;
        u64 v2;
        కొరకు(jj=14; jj>0 && z[jj-1]=='9'; jj--){}
        ఒకవేళ( jj==0 ){
          v2 = 1;
        }లేకపోతే{
          v2 = z[0] - '0';
          కొరకు(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
          v2++;
        }
        ఒకవేళ( r==sqlite3Fp10Convert2(v2, ఘాతాంకం + n - jj) ){
          i_విడత = jj+1;
        }
      }లేకపోతే ఒకవేళ( p->iDP>=n || (z[15]=='0' && z[14]=='0' && z[13]=='0') ){
        పూర్ణాంకం jj, kk;
        u64 v2;
        ప్రతిజ్ఞా( z[0]!='0' );
        కొరకు(jj=13; z[jj-1]=='0'; jj--){}
        v2 = z[0] - '0';
        కొరకు(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
        ఒకవేళ( r==sqlite3Fp10Convert2(v2, ఘాతాంకం + n - jj) ){
          i_విడత = jj+1;
        }
      }
    }
    n = i_విడత;
    ఒకవేళ( z[i_విడత]>='5' ){
      పూర్ణాంకం j = i_విడత-1;
      అయితే( 1 /*exit-by-break*/ ){
        z[j]++;
        ఒకవేళ( z[j]<='9' ) ఆపు;
        z[j] = '0';
        ఒకవేళ( j==0 ){
          z--;
          z[0] = '1';
          n++;
          p->iDP++;
          ఆపు;
        }లేకపోతే{
          j--;
        }
      }
    }
  }
  ప్రతిజ్ఞా( n>0 );
  అయితే( z[n-1]=='0' ){
    n--;
    ప్రతిజ్ఞా( n>0 );
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
పూర్ణాంకం sqlite3GetUInt32(స్థిరరాశి అక్షరం *z, u32 *pI){
  u64 v = 0;
  పూర్ణాంకం i;
  కొరకు(i=0; sqlite3Isdigit(z[i]); i++){
    v = v*10 + z[i] - '0';
    ఒకవేళ( v>4294967295LL ){ *pI = 0; తిరిగి 0; }
  }
  ఒకవేళ( i==0 || z[i]!=0 ){ *pI = 0; తిరిగి 0; }
  *pI = (u32)v;
  తిరిగి 1;
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
స్థిర పూర్ణాంకం SQLITE_NOINLINE putVarint64(గుర్తులేని అక్షరం *p, u64 v){
  పూర్ణాంకం i, j, n;
  u8 buf[10];
  ఒకవేళ( v & (((u64)0xff000000)<<32) ){
    p[8] = (u8)v;
    v >>= 8;
    కొరకు(i=7; i>=0; i--){
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    తిరిగి 9;
  }   
  n = 0;
  చేయి{
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  }అయితే( v!=0 );
  buf[0] &= 0x7f;
  ప్రతిజ్ఞా( n<=9 );
  కొరకు(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  తిరిగి n;
}
పూర్ణాంకం sqlite3PutVarint(గుర్తులేని అక్షరం *p, u64 v){
  ఒకవేళ( v<=0x7f ){
    p[0] = v&0x7f;
    తిరిగి 1;
  }
  ఒకవేళ( v<=0x3fff ){
    p[0] = ((v>>7)&0x7f)|0x80;
    p[1] = v&0x7f;
    తిరిగి 2;
  }
  తిరిగి putVarint64(p,v);
}

/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
u8 sqlite3GetVarint(స్థిరరాశి గుర్తులేని అక్షరం *p, u64 *v){
  u64 i_కీలకం = p[0];
  స్థిరరాశి u8 *p_ప్రారంభం = p;
  ఒకవేళ( i_కీలకం>=0x80 ){
    u8 x;
    i_కీలకం = (i_కీలకం<<7) ^ (x = *++p);
    ఒకవేళ( x>=0x80 ){
      i_కీలకం = (i_కీలకం<<7) ^ (x = *++p);
      ఒకవేళ( x>=0x80 ){
        i_కీలకం = (i_కీలకం<<7) ^ 0x10204000 ^ (x = *++p);
        ఒకవేళ( x>=0x80 ){
          i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
          ఒకవేళ( x>=0x80 ){
            i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
            ఒకవేళ( x>=0x80 ){
              i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
              ఒకవేళ( x>=0x80 ){
                i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
                ఒకవేళ( x>=0x80 ){
                  i_కీలకం = (i_కీలకం<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }లేకపోతే{
        i_కీలకం ^= 0x204000;
      }
    }లేకపోతే{
      i_కీలకం ^= 0x4000;
    }
  }
  *v = i_కీలకం;
  తిరిగి (u8)(p - p_ప్రారంభం) + 1;
}

/*
** Return the value of a variable-length integer without computing
** its length.  This is an optimization on sqlite3GetVarint() for the
** cases when the return value of sqlite3GetVarint() is not needed.
*/
i64 sqlite3_Varint_విలువ(స్థిరరాశి గుర్తులేని అక్షరం *p){
  u64 i_కీలకం = p[0];
  ఒకవేళ( i_కీలకం>=0x80 ){
    u8 x;
    i_కీలకం = (i_కీలకం<<7) ^ (x = *++p);
    ఒకవేళ( x>=0x80 ){
      i_కీలకం = (i_కీలకం<<7) ^ (x = *++p);
      ఒకవేళ( x>=0x80 ){
        i_కీలకం = (i_కీలకం<<7) ^ 0x10204000 ^ (x = *++p);
        ఒకవేళ( x>=0x80 ){
          i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
          ఒకవేళ( x>=0x80 ){
            i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
            ఒకవేళ( x>=0x80 ){
              i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
              ఒకవేళ( x>=0x80 ){
                i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++p);
                ఒకవేళ( x>=0x80 ){
                  i_కీలకం = (i_కీలకం<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }లేకపోతే{
        i_కీలకం ^= 0x204000;
      }
    }లేకపోతే{
      i_కీలకం ^= 0x4000;
    }
  }
  తిరిగి *(i64*)&i_కీలకం;
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
u8 sqlite3GetVarint32(స్థిరరాశి గుర్తులేని అక్షరం *p, u32 *v){
  u64 v64;
  u8 n;

  /* Assume that the single-byte case has already been handled by
  ** the getVarint32() macro */
  ప్రతిజ్ఞా( (p[0] & 0x80)!=0 );

  ఒకవేళ( (p[1] & 0x80)==0 ){
    /* This is the two-byte case */
    *v = ((p[0]&0x7f)<<7) | p[1];
    తిరిగి 2;
  }
  ఒకవేళ( (p[2] & 0x80)==0 ){
    /* This is the three-byte case */
    *v = ((p[0]&0x7f)<<14) | ((p[1]&0x7f)<<7) | p[2];
    తిరిగి 3;
  }
  /* four or more bytes */
  n = sqlite3GetVarint(p, &v64);
  ప్రతిజ్ఞా( n>3 && n<=9 );
  ఒకవేళ( (v64 & SQLITE_MAX_U32)!=v64 ){
    *v = 0xffffffff;
  }లేకపోతే{
    *v = (u32)v64;
  }
  తిరిగి n;
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
పూర్ణాంకం sqlite3VarintLen(u64 v){
  పూర్ణాంకం i;
  కొరకు(i=1; (v >>= 7)!=0; i++){ ప్రతిజ్ఞా( i<10 ); }
  తిరిగి i;
}


/*
** Read an unsigned 32-bit integer from an unaligned big-endian array of bytes.
*/
u32 sqlite3Get4byte(స్థిరరాశి u8 *p){
#ఒకవేళ SQLITE_BYTEORDER==4321
  u32 x;
  జ్ఞాపకం_నకలు(&x,p,4);
  తిరిగి x;
#లేదావేళ SQLITE_BYTEORDER==1234 && GCC_సంస్కరణ>=4003000
  u32 x;
  జ్ఞాపకం_నకలు(&x,p,4);
  తిరిగి __builtin_bswap32(x);
#లేదావేళ SQLITE_BYTEORDER==1234 && MSVC_సంస్కరణ>=1300
  u32 x;
  జ్ఞాపకం_నకలు(&x,p,4);
  తిరిగి _byteswap_ulong(x);
#లేకపోతే
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  తిరిగి ((గుర్తులేని)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
#అయితే_ముగింపు
}

/* Write an unsigned 32-bit integer into an unaligned big-endian array of bytes.
*/
శూన్య sqlite3Put4byte(గుర్తులేని అక్షరం *p, u32 v){
#ఒకవేళ SQLITE_BYTEORDER==4321
  జ్ఞాపకం_నకలు(p,&v,4);
#లేదావేళ SQLITE_BYTEORDER==1234 && GCC_సంస్కరణ>=4003000
  u32 x = __builtin_bswap32(v);
  జ్ఞాపకం_నకలు(p,&x,4);
#లేదావేళ SQLITE_BYTEORDER==1234 && MSVC_సంస్కరణ>=1300
  u32 x = _byteswap_ulong(v);
  జ్ఞాపకం_నకలు(p,&x,4);
#లేకపోతే
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  p[0] = (u8)(v>>24);
  p[1] = (u8)(v>>16);
  p[2] = (u8)(v>>8);
  p[3] = (u8)v;
#అయితే_ముగింపు
}

/*
** Read an unsigned 64-bit integer from an unaligned big-endian byte array.
*/
SQLITE_OPT_INLINE u64 sqlite3Get8byte(స్థిరరాశి u8 *p){
#ఒకవేళ SQLITE_BYTEORDER==4321
  u64 x;
  జ్ఞాపకం_నకలు(&x,p,8);
  తిరిగి x;
#లేదావేళ SQLITE_BYTEORDER==1234 && GCC_సంస్కరణ>=4003000
  u64 x;
  జ్ఞాపకం_నకలు(&x,p,8);
  తిరిగి __builtin_bswap64(x);
#లేదావేళ SQLITE_BYTEORDER==1234 && MSVC_సంస్కరణ>=1300
  u64 x;
  జ్ఞాపకం_నకలు(&x,p,8);
  తిరిగి _byteswap_uint64(x);
#లేకపోతే
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  తిరిగి (u64)(
    (((u64)p[0]) << 56) +
    (((u64)p[1]) << 48) +
    (((u64)p[2]) << 40) +
    (((u64)p[3]) << 32) +
    (((u64)p[4]) << 24) +
    (((u64)p[5]) << 16) +
    (((u64)p[6]) <<  8) +
    (((u64)p[7]) <<  0)
  );
#అయితే_ముగింపు
}

#ఒకవేళ SQLITE_BYTEORDER!=4321  /* Only used for little-endian machines */
/*
** Byte-swap a 64-bit unsigned integer.
*/
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64 x){
#ఒకవేళ SQLITE_BYTEORDER==1234 && GCC_సంస్కరణ>=4003000
  తిరిగి __builtin_bswap64(x);
#లేదావేళ SQLITE_BYTEORDER==1234 && MSVC_సంస్కరణ>=1300
  తిరిగి _byteswap_uint64(x);
#లేకపోతే
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  x = (x << 32) | (x >> 32);
  x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) |
      ((x & UINT64_C(0xffff0000ffff0000)) >> 16);
  x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) |
      ((x & UINT64_C(0xff00ff00ff00ff00)) >>  8);
  తిరిగి x;
#అయితే_ముగింపు
}
#అయితే_ముగింపు /* SQLITE_BYTEORDER!=4321 */

/*
** Translate a single byte of Hex into an integer.
** This routine only works if h really is a valid hexadecimal
** character:  0..9a..fA..F
*/
u8 sqlite3_Hex_గమ్యం_Int(పూర్ణాంకం h){
  ప్రతిజ్ఞా( (h>='0' && h<='9') ||  (h>='a' && h<='f') ||  (h>='A' && h<='F') );
#నిర్వచితమైతే SQLITE_ASCII
  h += 9*(1&(h>>6));
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#అయితే_ముగింపు
  తిరిగి (u8)(h & 0xf);
}

#ఒకవేళ !defined(SQLITE_OMIT_BLOB_LITERAL)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
శూన్య *sqlite3_Hex_గమ్యం_Blob(sqlite3 *db, స్థిరరాశి అక్షరం *z, పూర్ణాంకం n){
  అక్షరం *zBlob;
  పూర్ణాంకం i;

  zBlob = (అక్షరం *)sqlite3DbMallocRawNN(db, n/2 + 1);
  n--;
  ఒకవేళ( zBlob ){
    కొరకు(i=0; i<n; i+=2){
      zBlob[i/2] = (sqlite3_Hex_గమ్యం_Int(z[i])<<4) | sqlite3_Hex_గమ్యం_Int(z[i+1]);
    }
    zBlob[i/2] = 0;
  }
  తిరిగి zBlob;
}
#అయితే_ముగింపు /* !SQLITE_OMIT_BLOB_LITERAL */

/*
** Log an error that is an API call on a connection pointer that should
** not have been used.  The "type" of connection pointer is given as the
** argument.  The zType is a word like "NULL" or "closed" or "invalid".
*/
స్థిర శూన్య logBadConnection(స్థిరరాశి అక్షరం *zType){
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
పూర్ణాంకం sqlite3SafetyCheckOk(sqlite3 *db){
  u8 eOpenState;
  ఒకవేళ( db==0 ){
    logBadConnection("NULL");
    తిరిగి 0;
  }
  eOpenState = db->eOpenState;
  ఒకవేళ( eOpenState!=SQLITE_STATE_OPEN ){
    ఒకవేళ( sqlite3SafetyCheckSickOrOk(db) ){
      testcase( sqlite3GlobalConfig.xLog!=0 );
      logBadConnection("unopened");
    }
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి 1;
  }
}
పూర్ణాంకం sqlite3SafetyCheckSickOrOk(sqlite3 *db){
  u8 eOpenState;
  eOpenState = db->eOpenState;
  ఒకవేళ( eOpenState!=SQLITE_STATE_SICK &&
      eOpenState!=SQLITE_STATE_OPEN &&
      eOpenState!=SQLITE_STATE_BUSY ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    logBadConnection("invalid");
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి 1;
  }
}

/*
** Attempt to add, subtract, or multiply the 64-bit signed value iB against
** the other 64-bit signed integer at *pA and store the result in *pA.
** Return 0 on success.  Or if the operation would have resulted in an
** overflow, leave *pA unchanged and return 1.
*/
పూర్ణాంకం sqlite3_కూడు_Int64(i64 *pA, i64 iB){
#ఒకవేళ GCC_సంస్కరణ>=5004000 && !defined(__INTEL_COMPILER)
  తిరిగి __builtin_add_overflow(*pA, iB, pA);
#లేకపోతే
  i64 iA = *pA;
  testcase( iA==0 ); testcase( iA==1 );
  testcase( iB==-1 ); testcase( iB==0 );
  ఒకవేళ( iB>=0 ){
    testcase( iA>0 && అతిపెద్ద_INT64 - iA == iB );
    testcase( iA>0 && అతిపెద్ద_INT64 - iA == iB - 1 );
    ఒకవేళ( iA>0 && అతిపెద్ద_INT64 - iA < iB ) తిరిగి 1;
  }లేకపోతే{
    testcase( iA<0 && -(iA + అతిపెద్ద_INT64) == iB + 1 );
    testcase( iA<0 && -(iA + అతిపెద్ద_INT64) == iB + 2 );
    ఒకవేళ( iA<0 && -(iA + అతిపెద్ద_INT64) > iB + 1 ) తిరిగి 1;
  }
  *pA += iB;
  తిరిగి 0;
#అయితే_ముగింపు
}
పూర్ణాంకం sqlite3SubInt64(i64 *pA, i64 iB){
#ఒకవేళ GCC_సంస్కరణ>=5004000 && !defined(__INTEL_COMPILER)
  తిరిగి __builtin_sub_overflow(*pA, iB, pA);
#లేకపోతే
  testcase( iB==అతిచిన్న_INT64+1 );
  ఒకవేళ( iB==అతిచిన్న_INT64 ){
    testcase( (*pA)==(-1) ); testcase( (*pA)==0 );
    ఒకవేళ( (*pA)>=0 ) తిరిగి 1;
    *pA -= iB;
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి sqlite3_కూడు_Int64(pA, -iB);
  }
#అయితే_ముగింపు
}
పూర్ణాంకం sqlite3MulInt64(i64 *pA, i64 iB){
#ఒకవేళ GCC_సంస్కరణ>=5004000 && !defined(__INTEL_COMPILER)
  తిరిగి __builtin_mul_overflow(*pA, iB, pA);
#లేకపోతే
  i64 iA = *pA;
  ఒకవేళ( iB>0 ){
    ఒకవేళ( iA>అతిపెద్ద_INT64/iB ) తిరిగి 1;
    ఒకవేళ( iA<అతిచిన్న_INT64/iB ) తిరిగి 1;
  }లేకపోతే ఒకవేళ( iB<0 ){
    ఒకవేళ( iA>0 ){
      ఒకవేళ( iB<అతిచిన్న_INT64/iA ) తిరిగి 1;
    }లేకపోతే ఒకవేళ( iA<0 ){
      ఒకవేళ( iB==అతిచిన్న_INT64 ) తిరిగి 1;
      ఒకవేళ( iA==అతిచిన్న_INT64 ) తిరిగి 1;
      ఒకవేళ( -iA>అతిపెద్ద_INT64/-iB ) తిరిగి 1;
    }
  }
  *pA = iA*iB;
  తిరిగి 0;
#అయితే_ముగింపు
}

/*
** Compute the absolute value of a 32-bit signed integer, if possible.  Or
** if the integer has a value of -2147483648, return +2147483647
*/
పూర్ణాంకం sqlite3AbsInt32(పూర్ణాంకం x){
  ఒకవేళ( x>=0 ) తిరిగి x;
  ఒకవేళ( x==(పూర్ణాంకం)0x80000000 ) తిరిగి 0x7fffffff;
  తిరిగి -x;
}

#నిర్వచితమైతే SQLITE_ENABLE_8_3_పేర్లు
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
శూన్య sqlite3FileSuffix3(స్థిరరాశి అక్షరం *z_పునాది_Filename, అక్షరం *z){
#ఒకవేళ SQLITE_ENABLE_8_3_పేర్లు<2
  ఒకవేళ( sqlite3_uri_boolean(z_పునాది_Filename, "8_3_names", 0) )
#అయితే_ముగింపు
  {
    పూర్ణాంకం i, sz;
    sz = sqlite3Strlen30(z);
    కొరకు(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    ఒకవేళ( z[i]=='.' && ALWAYS(sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
  }
}
#అయితే_ముగింపు

/*
** Find (an approximate) sum of two LogEst values.  This computation is
** not a simple "+" operator because LogEst is stored as a logarithmic
** value.
**
*/
LogEst sqlite3_Log_Est_కూడు(LogEst a, LogEst b){
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం x[] = {
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
  ఒకవేళ( a>=b ){
    ఒకవేళ( a>b+49 ) తిరిగి a;
    ఒకవేళ( a>b+31 ) తిరిగి a+1;
    తిరిగి a+x[a-b];
  }లేకపోతే{
    ఒకవేళ( b>a+49 ) తిరిగి b;
    ఒకవేళ( b>a+31 ) తిరిగి b+1;
    తిరిగి b+x[b-a];
  }
}

/*
** Convert an integer into a LogEst.  In other words, compute an
** approximation for 10*log2(x).
*/
LogEst sqlite3LogEst(u64 x){
  స్థిర LogEst a[] = { 0, 2, 3, 5, 6, 7, 8, 9 };
  LogEst y = 40;
  ఒకవేళ( x<8 ){
    ఒకవేళ( x<2 ) తిరిగి 0;
    అయితే( x<8 ){  y -= 10; x <<= 1; }
  }లేకపోతే{
#ఒకవేళ GCC_సంస్కరణ>=5004000
    పూర్ణాంకం i = 60 - __builtin_clzll(x);
    y += i*10;
    x >>= i;
#లేకపోతే
    అయితే( x>255 ){ y += 40; x >>= 4; }  /*OPTIMIZATION-IF-TRUE*/
    అయితే( x>15 ){  y += 10; x >>= 1; }
#అయితే_ముగింపు
  }
  తిరిగి a[x&7] + y - 10;
}

/*
** Convert a double into a LogEst
** In other words, compute an approximation for 10*log2(x).
*/
LogEst sqlite3LogEstFromDouble(ద్విగుణ x){
  u64 a;
  LogEst e;
  ప్రతిజ్ఞా( పరిమాణం(x)==8 && పరిమాణం(a)==8 );
  ఒకవేళ( x<=1 ) తిరిగి 0;
  ఒకవేళ( x<=2000000000 ) తిరిగి sqlite3LogEst((u64)x);
  జ్ఞాపకం_నకలు(&a, &x, 8);
  e = (a>>52) - 1022;
  తిరిగి e*10;
}

/*
** Convert a LogEst into an integer.
*/
u64 sqlite3_Log_Est_గమ్యం_Int(LogEst x){
  u64 n;
  n = x%10;
  x /= 10;
  ఒకవేళ( n>=5 ) n -= 2;
  లేకపోతే ఒకవేళ( n>=1 ) n -= 1;
  ఒకవేళ( x>60 ) తిరిగి (u64)అతిపెద్ద_INT64;
  తిరిగి x>=3 ? (n+8)<<(x-3) : (n+8)>>(3-x);
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
VList *sqlite3_VList_కూడు(
  sqlite3 *db,           /* The database connection used for malloc() */
  VList *p_లోపల,            /* The input VList.  Might be NULL */
  స్థిరరాశి అక్షరం *zName,     /* Name of symbol to add */
  పూర్ణాంకం nName,             /* Bytes of text in zName */
  పూర్ణాంకం iVal               /* Value to associate with zName */
){
  పూర్ణాంకం nInt;              /* number of sizeof(int) objects needed for zName */
  అక్షరం *z;               /* Pointer to where zName will be stored */
  పూర్ణాంకం i;                 /* Index in pIn[] where zName is stored */

  nInt = nName/4 + 3;
  ప్రతిజ్ఞా( p_లోపల==0 || p_లోపల[0]>=3 );  /* Verify ok to add new elements */
  ఒకవేళ( p_లోపల==0 || p_లోపల[1]+nInt > p_లోపల[0] ){
    /* Enlarge the allocation */
    sqlite3_int64 nAlloc = (p_లోపల ? 2*(sqlite3_int64)p_లోపల[0] : 10) + nInt;
    VList *p_బహిర్గతం = sqlite3DbRealloc(db, p_లోపల, nAlloc*పరిమాణం(పూర్ణాంకం));
    ఒకవేళ( p_బహిర్గతం==0 ) తిరిగి p_లోపల;
    ఒకవేళ( p_లోపల==0 ) p_బహిర్గతం[1] = 2;
    p_లోపల = p_బహిర్గతం;
    p_లోపల[0] = nAlloc;
  }
  i = p_లోపల[1];
  p_లోపల[i] = iVal;
  p_లోపల[i+1] = nInt;
  z = (అక్షరం*)&p_లోపల[i+2];
  p_లోపల[1] = i+nInt;
  ప్రతిజ్ఞా( p_లోపల[1]<=p_లోపల[0] );
  జ్ఞాపకం_నకలు(z, zName, nName);
  z[nName] = 0;
  తిరిగి p_లోపల;
}

/*
** Return a pointer to the name of a variable in the given VList that
** has the value iVal.  Or return a NULL if there is no such variable in
** the list
*/
స్థిరరాశి అక్షరం *sqlite3_VList_అంకె_గమ్యం_Name(VList *p_లోపల, పూర్ణాంకం iVal){
  పూర్ణాంకం i, mx;
  ఒకవేళ( p_లోపల==0 ) తిరిగి 0;
  mx = p_లోపల[1];
  i = 2;
  చేయి{
    ఒకవేళ( p_లోపల[i]==iVal ) తిరిగి (అక్షరం*)&p_లోపల[i+2];
    i += p_లోపల[i+1];
  }అయితే( i<mx );
  తిరిగి 0;
}

/*
** Return the number of the variable named zName, if it is in VList.
** or return 0 if there is no such variable.
*/
పూర్ణాంకం sqlite3_VList_Name_గమ్యం_అంకె(VList *p_లోపల, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం nName){
  పూర్ణాంకం i, mx;
  ఒకవేళ( p_లోపల==0 ) తిరిగి 0;
  mx = p_లోపల[1];
  i = 2;
  చేయి{
    స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)&p_లోపల[i+2];
    ఒకవేళ( అక్షరశ్రేణి_పోలిక(z,zName,nName)==0 && z[nName]==0 ) తిరిగి p_లోపల[i];
    i += p_లోపల[i+1];
  }అయితే( i<mx );
  తిరిగి 0;
}
