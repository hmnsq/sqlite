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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>

/* Work around a bug in older Microsoft compilers
** Forum post 2026-04-10T06:33:11z */
#ਜੇ !defined(INFINITY) && defined(_MSC_VER)
# define INFINITY HUGE_VAL
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_FLOATING_POINT */

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
ਪੂਰਨ sqlite3FaultSim(ਪੂਰਨ iTest){
  ਪੂਰਨ (*xCallback)(ਪੂਰਨ) = sqlite3GlobalConfig.xTestCallback;
  ਵਾਪਸ xCallback ? xCallback(iTest) : SQLITE_OK;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
/*
** Return true if the floating point value is Not a Number (NaN).
**
** Use the math library isnan() function if compiled with SQLITE_HAVE_ISNAN.
** Otherwise, we have our own implementation that works on most systems.
*/
ਪੂਰਨ sqlite3IsNaN(ਦੁੱਗਣਾ x){
  ਪੂਰਨ rc;   /* The value return */
#ਜੇ !SQLITE_HAVE_ISNAN && !HAVE_ISNAN
  u64 y;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&y,&x,ਆਕਾਰ(y));
  rc = IsNaN(y);
#ਨਹੀਂਤਾਂ
  rc = isnan(x);
#ਜੇ_ਅੰਤ /* HAVE_ISNAN */
  testcase( rc );
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_FLOATING_POINT */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
/*
** Return true if the floating point value is NaN or +Inf or -Inf.
*/
ਪੂਰਨ sqlite3_Is_ਉਛਾਲ(ਦੁੱਗਣਾ x){
  ਪੂਰਨ rc;   /* The value return */
  u64 y;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&y,&x,ਆਕਾਰ(y));
  rc = IsOvfl(y);
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_FLOATING_POINT */

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
**
** The value returned will never be negative.  Nor will it ever be greater
** than the actual length of the string.  For very long strings (greater
** than 1GiB) the value returned might be less than the true string length.
*/
ਪੂਰਨ sqlite3Strlen30(ਅਚਲ ਅੱਖਰ *z){
  ਜੇ( z==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ 0x3fffffff & (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z);
}

/*
** Return the declared type of a column.  Or return zDflt if the column
** has no declared type.
**
** The column type is an extra string stored after the zero-terminator on
** the column name if and only if the COLFLAG_HASTYPE flag is set.
*/
ਅੱਖਰ *sqlite3_ਕਾਲਮ_Type(ਕਾਲਮ *p_ਖਾਨਾ, ਅੱਖਰ *zDflt){
  ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_HASTYPE ){
    ਵਾਪਸ p_ਖਾਨਾ->zCnName + ਲੜੀ_ਲੰਬਾਈ(p_ਖਾਨਾ->zCnName) + 1;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ->eCType ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->eCType<=SQLITE_N_STDTYPE );
    ਵਾਪਸ (ਅੱਖਰ*)sqlite3StdType[p_ਖਾਨਾ->eCType-1];
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ zDflt;
  }
}

/*
** Helper function for sqlite3Error() - called rarely.  Broken out into
** a separate routine to avoid unnecessary register saves on entry to
** sqlite3Error().
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ  sqlite3_Error_ਅੰਤ(sqlite3 *db, ਪੂਰਨ err_ਕੋਡ_2){
  ਜੇ( db->pErr ) sqlite3_ਮੁੱਲ_ਸੈੱਟ_Null(db->pErr);
  sqlite3SystemError(db, err_ਕੋਡ_2);
}

/*
** Set the current error code to err_code and clear any prior error message.
** Also set iSysErrno (by calling sqlite3System) if the err_code indicates
** that would be appropriate.
*/
ਖਾਲੀ sqlite3Error(sqlite3 *db, ਪੂਰਨ err_ਕੋਡ_2){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  db->err_ਕੋਡ = err_ਕੋਡ_2;
  ਜੇ( err_ਕੋਡ_2 || db->pErr ){
    sqlite3_Error_ਅੰਤ(db, err_ਕੋਡ_2);
  }ਨਹੀਂਤਾਂ{
    db->err_Byte_ਵਿੱਥ = -1;
  }
}

/*
** The equivalent of sqlite3Error(db, SQLITE_OK).  Clear the error state
** and error message.
*/
ਖਾਲੀ sqlite3ErrorClear(sqlite3 *db){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  db->err_ਕੋਡ = SQLITE_OK;
  db->err_Byte_ਵਿੱਥ = -1;
  ਜੇ( db->pErr ) sqlite3_ਮੁੱਲ_ਸੈੱਟ_Null(db->pErr);
}

/*
** Load the sqlite3.iSysErrno field if that is an appropriate thing
** to do based on the SQLite error code in rc.
*/
ਖਾਲੀ sqlite3SystemError(sqlite3 *db, ਪੂਰਨ rc){
  ਜੇ( rc==SQLITE_IOERR_NOMEM ) ਵਾਪਸ;
#ਜੇ defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
  ਜੇ( rc==SQLITE_IOERR_ਵਿੱਚ_ਪੰਨਾ ){
    ਪੂਰਨ ii;
    ਪੂਰਨ iErr;
    sqlite3BtreeEnterAll(db);
    ਲਈ(ii=0; ii<db->nDb; ii++){
      ਜੇ( db->aDb[ii].pBt ){
        iErr = sqlite3PagerWalSystemErrno(sqlite3BtreePager(db->aDb[ii].pBt));
        ਜੇ( iErr ){
          db->iSysErrno = iErr;
        }
      }
    }
    sqlite3BtreeLeaveAll(db);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  rc &= 0xff;
  ਜੇ( rc==SQLITE_CANTOPEN || rc==SQLITE_IOERR ){
    db->iSysErrno = sqlite3_Os_Get_ਆਖਰੀ_Error(db->pVfs);
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
ਖਾਲੀ sqlite3_Error_With_ਸੁਨੇਹਾ(sqlite3 *db, ਪੂਰਨ err_ਕੋਡ_2, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  db->err_ਕੋਡ = err_ਕੋਡ_2;
  sqlite3SystemError(db, err_ਕੋਡ_2);
  ਜੇ( zFormat==0 ){
    sqlite3Error(db, err_ਕੋਡ_2);
  }ਨਹੀਂਤਾਂ ਜੇ( db->pErr || (db->pErr = sqlite3_ਮੁੱਲ_New(db))!=0 ){
    ਅੱਖਰ *z;
    va_list ap;
    va_start(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);
    va_end(ap);
    sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(db->pErr, -1, z, SQLITE_UTF8, SQLITE_DYNAMIC);
  }
}

/*
** Check for interrupts and invoke progress callback.
*/
ਖਾਲੀ sqlite3_ਤਰੱਕੀ_Check(Parse *p){
  sqlite3 *db = p->db;
  ਜੇ( Atomic_ਲੱਦੋ(&db->u1.isInterrupted) ){
    p->nErr++;
    p->rc = SQLITE_ਰੁਕਾਵਟ;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
  ਜੇ( db->x_ਤਰੱਕੀ ){
    ਜੇ( p->rc==SQLITE_ਰੁਕਾਵਟ ){
      p->n_ਤਰੱਕੀ_ਕਦਮਾਂ = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( (++p->n_ਤਰੱਕੀ_ਕਦਮਾਂ)>=db->n_ਤਰੱਕੀ_Ops ){
      ਜੇ( db->x_ਤਰੱਕੀ(db->p_ਤਰੱਕੀ_Arg) ){
        p->nErr++;
        p->rc = SQLITE_ਰੁਕਾਵਟ;
      }
      p->n_ਤਰੱਕੀ_ਕਦਮਾਂ = 0;
    }
  }
#ਜੇ_ਅੰਤ
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
ਖਾਲੀ sqlite3_Error_ਸੁਨੇਹਾ(Parse *pParse, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  ਅੱਖਰ *z_ਸੁਨੇਹਾ;
  va_list ap;
  sqlite3 *db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse || db->pParse->pToplevel==pParse );
  db->err_Byte_ਵਿੱਥ = -2;
  va_start(ap, zFormat);
  z_ਸੁਨੇਹਾ = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ਜੇ( db->err_Byte_ਵਿੱਥ<-1 ) db->err_Byte_ਵਿੱਥ = -1;
  ਜੇ( db->suppressErr ){
    sqlite3DbFree(db, z_ਸੁਨੇਹਾ);
    ਜੇ( db->mallocFailed ){
      pParse->nErr++;
      pParse->rc = SQLITE_NOMEM;
    }
  }ਨਹੀਂਤਾਂ{
    pParse->nErr++;
    sqlite3DbFree(db, pParse->z_Err_ਸੁਨੇਹਾ);
    pParse->z_Err_ਸੁਨੇਹਾ = z_ਸੁਨੇਹਾ;
    pParse->rc = SQLITE_ERROR;
    pParse->pWith = 0;
  }
}

/*
** If database connection db is currently parsing SQL, then transfer
** error code errCode to that parser if the parser has not already
** encountered some other kind of error.
*/
ਪੂਰਨ sqlite3_Error_ਮੰਜ਼ਿਲ_Parser(sqlite3 *db, ਪੂਰਨ err_ਕੋਡ){
  Parse *pParse;
  ਜੇ( db==0 || (pParse = db->pParse)==0 ) ਵਾਪਸ err_ਕੋਡ;
  pParse->rc = err_ਕੋਡ;
  pParse->nErr++;
  ਵਾਪਸ err_ਕੋਡ;
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
ਖਾਲੀ sqlite3Dequote(ਅੱਖਰ *z){
  ਅੱਖਰ quote;
  ਪੂਰਨ i, j;
  ਜੇ( z==0 ) ਵਾਪਸ;
  quote = z[0];
  ਜੇ( !sqlite3Isquote(quote) ) ਵਾਪਸ;
  ਜੇ( quote=='[' ) quote = ']';
  ਲਈ(i=1, j=0;; i++){
    ਪ੍ਰਤਿਜ੍ਞਾ( z[i] );
    ਜੇ( z[i]==quote ){
      ਜੇ( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }ਨਹੀਂਤਾਂ{
        ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ{
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}
ਖਾਲੀ sqlite3_Dequote_ਵਿਅੰਜਕ(ਵਿਅੰਜਕ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3Isquote(p->u.zToken[0]) );
  p->flags |= p->u.zToken[0]=='"' ? EP_Quoted|EP_DblQuoted : EP_Quoted;
  sqlite3Dequote(p->u.zToken);
}

/*
** Expression p is a QNUMBER (quoted number). Dequote the value in p->u.zToken
** and set the type to INTEGER or FLOAT. "Quoted" integers or floats are those
** that contain '_' characters that must be removed before further processing.
*/
ਖਾਲੀ sqlite3_Dequote_ਸੰਖਿਆ(Parse *pParse, ਵਿਅੰਜਕ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 || pParse->db->mallocFailed );
  ਜੇ( p ){
    ਅਚਲ ਅੱਖਰ *p_ਵਿੱਚ = p->u.zToken;
    ਅੱਖਰ *p_ਬਾਹਰਲਾ = p->u.zToken;
    ਪੂਰਨ bHex = (p_ਵਿੱਚ[0]=='0' && (p_ਵਿੱਚ[1]=='x' || p_ਵਿੱਚ[1]=='X'));
    ਪੂਰਨ i_ਮੁੱਲ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->op==TK_QNUMBER );
    p->op = TK_INTEGER;
    ਕਰੋ {
      ਜੇ( *p_ਵਿੱਚ!=SQLITE_DIGIT_SEPARATOR ){
        *p_ਬਾਹਰਲਾ++ = *p_ਵਿੱਚ;
        ਜੇ( *p_ਵਿੱਚ=='e' || *p_ਵਿੱਚ=='E' || *p_ਵਿੱਚ=='.' ) p->op = TK_FLOAT;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( (bHex==0 && (!sqlite3Isdigit(p_ਵਿੱਚ[-1]) || !sqlite3Isdigit(p_ਵਿੱਚ[1])))
         || (bHex==1 && (!sqlite3Isxdigit(p_ਵਿੱਚ[-1]) || !sqlite3Isxdigit(p_ਵਿੱਚ[1])))
        ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unrecognized token: \"%s\"", p->u.zToken);
        }
      }
    }ਜਦੋਂ( *p_ਵਿੱਚ++ );
    ਜੇ( bHex ) p->op = TK_INTEGER;

    /* tag-20240227-a: If after dequoting, the number is an integer that
    ** fits in 32 bits, then it must be converted into EP_IntValue.  Other
    ** parts of the code expect this.  See also tag-20240227-b. */
    ਜੇ( p->op==TK_INTEGER && sqlite3GetInt32(p->u.zToken, &i_ਮੁੱਲ) ){
      p->u.i_ਮੁੱਲ = i_ਮੁੱਲ;
      p->flags |= EP_Int_ਮੁੱਲ;
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
ਖਾਲੀ sqlite3DequoteToken(Token *p){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਜੇ( p->n<2 ) ਵਾਪਸ;
  ਜੇ( !sqlite3Isquote(p->z[0]) ) ਵਾਪਸ;
  ਲਈ(i=1; i<p->n-1; i++){
    ਜੇ( sqlite3Isquote(p->z[i]) ) ਵਾਪਸ;
  }
  p->n -= 2;
  p->z++;
}

/*
** Generate a Token object from a string
*/
ਖਾਲੀ sqlite3TokenInit(Token *p, ਅੱਖਰ *z){
  p->z = z;
  p->n = sqlite3Strlen30(z);
}

/* Convenient short-hand */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ

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
ਪੂਰਨ sqlite3_stricmp(ਅਚਲ ਅੱਖਰ *z_ਖੱਬਾ, ਅਚਲ ਅੱਖਰ *z_ਸੱਜਾ){
  ਜੇ( z_ਖੱਬਾ==0 ){
    ਵਾਪਸ z_ਸੱਜਾ ? -1 : 0;
  }ਨਹੀਂਤਾਂ ਜੇ( z_ਸੱਜਾ==0 ){
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ sqlite3StrICmp(z_ਖੱਬਾ, z_ਸੱਜਾ);
}
ਪੂਰਨ sqlite3StrICmp(ਅਚਲ ਅੱਖਰ *z_ਖੱਬਾ, ਅਚਲ ਅੱਖਰ *z_ਸੱਜਾ){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a, *b;
  ਪੂਰਨ c, x;
  a = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)z_ਖੱਬਾ;
  b = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)z_ਸੱਜਾ;
  ਲਈ(;;){
    c = *a;
    x = *b;
    ਜੇ( c==x ){
      ਜੇ( c==0 ) ਰੋਕੋ;
    }ਨਹੀਂਤਾਂ{
      c = (ਪੂਰਨ)ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[c] - (ਪੂਰਨ)ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[x];
      ਜੇ( c ) ਰੋਕੋ;
    }
    a++;
    b++;
  }
  ਵਾਪਸ c;
}
ਪੂਰਨ sqlite3_strnicmp(ਅਚਲ ਅੱਖਰ *z_ਖੱਬਾ, ਅਚਲ ਅੱਖਰ *z_ਸੱਜਾ, ਪੂਰਨ N){
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a, *b;
  ਜੇ( z_ਖੱਬਾ==0 ){
    ਵਾਪਸ z_ਸੱਜਾ ? -1 : 0;
  }ਨਹੀਂਤਾਂ ਜੇ( z_ਸੱਜਾ==0 ){
    ਵਾਪਸ 1;
  }
  a = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)z_ਖੱਬਾ;
  b = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)z_ਸੱਜਾ;
  ਜਦੋਂ( N-- > 0 && *a!=0 && ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[*a]==ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[*b]){ a++; b++; }
  ਵਾਪਸ N<0 ? 0 : ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[*a] - ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[*b];
}

/*
** Compute an 8-bit hash on a string that is insensitive to case differences
*/
u8 sqlite3StrIHash(ਅਚਲ ਅੱਖਰ *z){
  u8 h = 0;
  ਜੇ( z==0 ) ਵਾਪਸ 0;
  ਜਦੋਂ( z[0] ){
    h += ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)z[0]];
    z++;
  }
  ਵਾਪਸ h;
}

#ਜੇ !defined(SQLITE_DISABLE_INTRINSIC)               \
  && (defined(__GNUC__) || defined(__clang__))       \
  && (defined(__x86_64__) || defined(__aarch64__) || \
      (defined(__riscv) && defined(__riscv_xlen) && (__riscv_xlen>32)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_USE_UINT128
#ਜੇ_ਅੰਤ

/*
** Two inputs are multiplied to get a 128-bit result.  Write the
** lower 64-bits of the result into *pLo, and return the high-order
** 64 bits.
*/
ਸਥਿਰ u64 sqlite3Multiply128(u64 a, u64 b, u64 *pLo){
#ਜੇ defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  *pLo = (u64)r;
  ਵਾਪਸ (u64)(r>>64);
#ਜਾਂਜੇ defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  *pLo = a*b;
  ਵਾਪਸ __umulh(a, b);
#ਨਹੀਂਤਾਂ
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
  ਵਾਪਸ a1b1 + (a0b1>>32) + (a1b0>>32) + (t>>32);
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo){
#ਜੇ defined(SQLITE_USE_UINT128)
  __uint128_t r = (__uint128_t)a * b;
  r += ((__uint128_t)aLo * b) >> 32;
  *pLo = (r>>32)&0xffffffff;
  ਵਾਪਸ r>>64;
#ਜਾਂਜੇ defined(_WIN64) && !defined(SQLITE_DISABLE_INTRINSIC)
  u64 r1_hi = __umulh(a,b);
  u64 r1_lo = a*b;
  u64 r2 = (__umulh((u64)aLo,b)<<32) + ((aLo*b)>>32);
  u64 t = r1_lo + r2;
  ਜੇ( t<r1_lo ) r1_hi++;
  *pLo = t>>32;
  ਵਾਪਸ r1_hi;
#ਨਹੀਂਤਾਂ
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
  ਵਾਪਸ (r4<<32) + r3;
#ਜੇ_ਅੰਤ
}

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SQLITE_USE_UINT128

/*
** Return a u64 with the N-th bit set.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ U64_ਦੋਆਧਾਰੀ_ਅੰਕ(N)  (((u64)1)<<(N))

/*
** Range of powers of 10 that we need to deal with when converting
** IEEE754 doubles to and from decimal.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POWERSOF10_ਪਹਿਲਾ (-348)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POWERSOF10_ਆਖਰੀ  (+347)

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
ਸਥਿਰ u64 power_ਦਾ_Ten(ਪੂਰਨ p, u32 *pLo){
  ਸਥਿਰ ਅਚਲ u64 a_ਅਧਾਰ[] = {
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
  ਸਥਿਰ ਅਚਲ u64 aScale[] = {
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
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ aScaleLo[] = {
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
  ਪੂਰਨ g, n;
  u64 s, x;
  u32 lo;

  ਪ੍ਰਤਿਜ੍ਞਾ( p>=POWERSOF10_ਪਹਿਲਾ && p<=POWERSOF10_ਆਖਰੀ );
  ਜੇ( p<0 ){
    ਜੇ( p==(-1) ){
      *pLo = aScaleLo[13];
      ਵਾਪਸ aScale[13];
    }
    g = p/27;
    n = p%27;
    ਜੇ( n ){
      g--;
      n += 27;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p<27 ){
    *pLo = 0;
    ਵਾਪਸ a_ਅਧਾਰ[p];
  }ਨਹੀਂਤਾਂ{
    g = p/27;
    n = p%27;
  }
  s = aScale[g+13];
  ਜੇ( n==0 ){
    *pLo = aScaleLo[g+13];
    ਵਾਪਸ s;
  }
  x = sqlite3Multiply160(s,aScaleLo[g+13],a_ਅਧਾਰ[n],&lo);
  ਜੇ( (U64_ਦੋਆਧਾਰੀ_ਅੰਕ(63) & x)==0 ){
    x  = x<<1 | ((lo>>31)&1);
    lo = (lo<<1) | 1;
  }
  *pLo = lo;
  ਵਾਪਸ x;
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
ਸਥਿਰ ਪੂਰਨ pwr10to2(ਪੂਰਨ p){ ਵਾਪਸ (p*108853) >> 15; }
ਸਥਿਰ ਪੂਰਨ pwr2to10(ਪੂਰਨ p){ ਵਾਪਸ (p*78913) >> 18; }

/*
** Count leading zeros for a 64-bit unsigned integer.
*/
ਸਥਿਰ ਪੂਰਨ ਗਿਣਤੀ_Leading_Zeros(u64 m){
#ਜੇ (defined(__GNUC__) || defined(__clang__)) \
    && !defined(SQLITE_DISABLE_INTRINSIC)
  ਵਾਪਸ __builtin_clzll(m);
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ n = 0;
  ਜੇ( m <= 0x00000000ffffffffULL) { n += 32; m <<= 32; }
  ਜੇ( m <= 0x0000ffffffffffffULL) { n += 16; m <<= 16; }
  ਜੇ( m <= 0x00ffffffffffffffULL) { n += 8;  m <<= 8;  }
  ਜੇ( m <= 0x0fffffffffffffffULL) { n += 4;  m <<= 4;  }
  ਜੇ( m <= 0x3fffffffffffffffULL) { n += 2;  m <<= 2;  }
  ਜੇ( m <= 0x7fffffffffffffffULL) { n += 1;            }
  ਵਾਪਸ n;
#ਜੇ_ਅੰਤ
}

/*
** Given m and e, which represent a quantity r == m*pow(2,e),
** return values *pD and *pP such that r == (*pD)*pow(10,*pP),
** approximately.  *pD should contain at least n significant digits.
**
** The input m is required to have its highest bit set.  In other words,
** m should be left-shifted, and e decremented, to maximize the value of m.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3Fp2Convert10(u64 m, ਪੂਰਨ e, ਪੂਰਨ n, u64 *pD, ਪੂਰਨ *pP){
  ਪੂਰਨ p;
  u64 h, d1;
  u32 d2;
  ਪ੍ਰਤਿਜ੍ਞਾ( n>=1 && n<=18 );
  p = n - 1 - pwr2to10(e+63);
  h = sqlite3Multiply128(m, power_ਦਾ_Ten(p,&d2), &d1);
  ਪ੍ਰਤਿਜ੍ਞਾ( -(e + pwr10to2(p) + 2) >= 0  );
  ਪ੍ਰਤਿਜ੍ਞਾ( -(e + pwr10to2(p) + 1) <= 63 );
  ਜੇ( n==18 ){
    h >>= -(e + pwr10to2(p) + 2);
    *pD = (h + ((h<<1)&2))>>1;
  }ਨਹੀਂਤਾਂ{
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
ਸਥਿਰ ਦੁੱਗਣਾ sqlite3Fp10Convert2(u64 d, ਪੂਰਨ p){
  ਪੂਰਨ b, lp, e, adj, s;
  u32 pwr10l, mid1;
  u64 pwr10h, x, hi, lo, sticky, u, m;
  ਦੁੱਗਣਾ r;
  ਜੇ( p<POWERSOF10_ਪਹਿਲਾ ) ਵਾਪਸ 0.0;
  ਜੇ( p>POWERSOF10_ਆਖਰੀ ) ਵਾਪਸ (ਦੁੱਗਣਾ)INFINITY;
  b = 64 - ਗਿਣਤੀ_Leading_Zeros(d);
  lp = pwr10to2(p);
  e = 53 - b - lp;
  ਜੇ( e > 1074 ){
    ਜੇ( e>=1130 ) ਵਾਪਸ 0.0;
    e = 1074;
  }
  s = -(e-(64-b) + lp + 3);
  pwr10h = power_ਦਾ_Ten(p, &pwr10l);
  ਜੇ( pwr10l!=0 ){
    pwr10h++;
    pwr10l = ~pwr10l;
  }
  x = d<<(64-b);
  hi = sqlite3Multiply128(x,pwr10h,&lo);
  mid1 = lo>>32;
  sticky = 1;
  ਜੇ( (hi & (U64_ਦੋਆਧਾਰੀ_ਅੰਕ(s)-1))==0 ) {
    u32 mid2 = sqlite3Multiply128(x,((u64)pwr10l)<<32,&lo)>>32;
    sticky = (mid1-mid2 > 1);
    hi -= mid1 < mid2;
  }
  u = (hi>>s) | sticky;
  adj = (u >= U64_ਦੋਆਧਾਰੀ_ਅੰਕ(55)-2);
  ਜੇ( adj ){
    u = (u>>adj) | (u&1);
    e -= adj;
  }
  m = (u + 1 + ((u>>2)&1)) >> 2;
  ਜੇ( e<=(-972) ) ਵਾਪਸ (ਦੁੱਗਣਾ)INFINITY;
  ਜੇ((m & U64_ਦੋਆਧਾਰੀ_ਅੰਕ(52)) != 0){
    m = (m & ~U64_ਦੋਆਧਾਰੀ_ਅੰਕ(52)) | ((u64)(1075-e)<<52);
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&r,&m,8);
  ਵਾਪਸ r;
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
ਪੂਰਨ sqlite3AtoF(ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, ਦੁੱਗਣਾ *p_ਨਤੀਜਾ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)z_ਵਿੱਚ;
  ਪੂਰਨ neg = 0;       /* True for a negative value */
  u64 s = 0;         /* mantissa */
  ਪੂਰਨ d = 0;         /* Value is s * pow(10,d) */
  ਪੂਰਨ mState = 0;    /* 1: digit seen 2: fp 4: hard-zero */
  ਬਿਨਾਨਿਸ਼ਾਨੀ v;        /* Value of a single digit */

  ਸ਼ੁਰੂ_ਦਾ_ਪਾਠ:
  ਜੇ( (v = (ਬਿਨਾਨਿਸ਼ਾਨੀ)z[0] - '0')<10 ){
    parse_integer_part:
    mState = 1;
    s = v;
    z++;
    ਜਦੋਂ( (v = (ਬਿਨਾਨਿਸ਼ਾਨੀ)z[0] - '0')<10 ){
      s = s*10 + v;
      z++;
      ਜੇ( s>=(ਸਭ_ਤੋਂ_ਵੱਡਾ_UINT64-9)/10 ){
        mState = 9;
        ਜਦੋਂ( sqlite3Isdigit(z[0]) ){ z++; d++; }
        ਰੋਕੋ;
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( z[0]=='-' ){
    neg = 1;
    z++;
    ਜੇ( (v = (ਬਿਨਾਨਿਸ਼ਾਨੀ)z[0] - '0')<10 ) ਜਾਓ parse_integer_part;
  }ਨਹੀਂਤਾਂ ਜੇ( z[0]=='+' ){
    z++;
    ਜੇ( (v = (ਬਿਨਾਨਿਸ਼ਾਨੀ)z[0] - '0')<10 ) ਜਾਓ parse_integer_part;
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3Isspace(z[0]) ){
    ਕਰੋ{ z++; }ਜਦੋਂ( sqlite3Isspace(z[0]) );
    ਜਾਓ ਸ਼ੁਰੂ_ਦਾ_ਪਾਠ;
  }ਨਹੀਂਤਾਂ{
    s = 0;
  }

  /* if decimal point is present */
  ਜੇ( *z=='.' ){
    z++;
    ਜੇ( sqlite3Isdigit(z[0]) ){
      mState |= 1;
      ਕਰੋ{
        ਜੇ( s<(ਸਭ_ਤੋਂ_ਵੱਡਾ_UINT64-9)/10 ){
          s = s*10 + z[0] - '0';
          d--;
        }ਨਹੀਂਤਾਂ{
          mState = 11;
        }
      }ਜਦੋਂ( sqlite3Isdigit(*++z) );
    }ਨਹੀਂਤਾਂ ਜੇ( mState==0 ){
      *p_ਨਤੀਜਾ = 0.0;
      ਵਾਪਸ 0;
    }
    mState |= 2;
  }ਨਹੀਂਤਾਂ ਜੇ( mState==0 ){
    *p_ਨਤੀਜਾ = 0.0;
    ਵਾਪਸ 0;
  }

  /* if exponent is present */
  ਜੇ( *z=='e' || *z=='E' ){
    ਪੂਰਨ esign;
    z++;
 
    /* get sign of exponent */
    ਜੇ( *z=='-' ){
      esign = -1;
      z++;
    }ਨਹੀਂਤਾਂ{
      esign = +1;
      ਜੇ( *z=='+' ){
        z++;
      }
    }
    /* copy digits to exponent */
    ਜੇ( (v = (ਬਿਨਾਨਿਸ਼ਾਨੀ)z[0] - '0')<10 ){
      ਪੂਰਨ ਘਾਤਾਂਕ = v;
      z++;
      mState |= 2;
      ਜਦੋਂ( (v = (ਬਿਨਾਨਿਸ਼ਾਨੀ)z[0] - '0')<10 ){
        ਘਾਤਾਂਕ = ਘਾਤਾਂਕ<10000 ? (ਘਾਤਾਂਕ*10 + v) : 10000;
        z++;
      }
      d += esign*ਘਾਤਾਂਕ;
    }ਨਹੀਂਤਾਂ{
      z--;  /* Leave z[0] at 'e' or '+' or '-',
            ** so that the return is 0 or -1 */
    }
  }

  /* Convert s*pow(10,d) into real */
  ਜੇ( s==0 ){
    *p_ਨਤੀਜਾ = 0.0;
    mState |= 4;
  }ਨਹੀਂਤਾਂ{
    *p_ਨਤੀਜਾ = sqlite3Fp10Convert2(s,d);
  }
  ਜੇ( neg ) *p_ਨਤੀਜਾ = -*p_ਨਤੀਜਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3IsNaN(*p_ਨਤੀਜਾ) );

  /* return true if number and no extra non-whitespace characters after */
  ਜੇ( z[0]==0 ){
    ਵਾਪਸ mState;
  }
  ਜੇ( sqlite3Isspace(z[0]) ){
    ਕਰੋ{ z++; }ਜਦੋਂ( sqlite3Isspace(*z) );
    ਜੇ( z[0]==0 ){
      ਵਾਪਸ mState;
    }
  }
  ਵਾਪਸ 0xfffffff0 | mState;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ sqlite3Atoi64(z_ਵਿੱਚ, p_ਨਤੀਜਾ, ਲੜੀ_ਲੰਬਾਈ(z_ਵਿੱਚ), SQLITE_UTF8)==0;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Digit pairs used to convert a U64 or I64 into text, two digits
** at a time.
*/
ਸਥਿਰ ਅਚਲ ਸੰਘ {
  ਅੱਖਰ a[201];
  ਛੋਟਾ ਪੂਰਨ forceAlignment;
} sqlite3_Digit_ਜੋੜੇ = {
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
#ਜੇ (defined(__arm__) && !defined(__aarch64__)) || \
    (defined(__ppc__) && !defined(__ppc64__))
# define SQLITE_AVOID_U64_DIVIDE 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_AVOID_U64_DIVIDE
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
ਸਥਿਰ ਪੂਰਨ sqlite3_UInt64_ਮੰਜ਼ਿਲ_ਪਾਠ(u64 v, ਅੱਖਰ *z_ਬਾਹਰਲਾ){
  u32 x32, kk;
  ਪੂਰਨ i;
  z_ਬਾਹਰਲਾ[SQLITE_U64_DIGITS] = 0;
  i = SQLITE_U64_DIGITS;
  ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_ਜੋੜੇ.a[0]) );
  ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(z_ਬਾਹਰਲਾ) );
  ਜਦੋਂ( (v>>32)!=0 ){
    u32 y, x0, x1, y0, y1;
    x32 = v % 100000000;
    v   = v / 100000000;
    y = x32 % 10000;
    x32 /= 10000;
    x1 = x32 / 100;
    x0 = x32 % 100;
    y1 = y / 100;
    y0 = y % 100;
    ਪ੍ਰਤਿਜ੍ਞਾ( i>=8 );
    i -= 8;
    *(u16*)(&z_ਬਾਹਰਲਾ[i]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[x1*2];
    *(u16*)(&z_ਬਾਹਰਲਾ[i+2]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[x0*2];
    *(u16*)(&z_ਬਾਹਰਲਾ[i+4]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[y1*2];
    *(u16*)(&z_ਬਾਹਰਲਾ[i+6]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[y0*2];
  }
  x32 = v;
  ਜਦੋਂ( x32>=10 ){
    kk  = x32 % 100;
    x32 = x32 / 100;
    ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_ਜੋੜੇ.a[kk*2]) );
    ਪ੍ਰਤਿਜ੍ਞਾ( i>=2 );
    i -= 2;
    ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&z_ਬਾਹਰਲਾ[i]) );
    *(u16*)(&z_ਬਾਹਰਲਾ[i]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[kk*2];
  }
  ਜੇ( x32 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( i>0 );
    z_ਬਾਹਰਲਾ[--i] = x32 + '0';
  }
  ਵਾਪਸ i;
}
#ਜੇ_ਅੰਤ /* defined(SQLITE_AVOID_U64_DIVIDE) */

/*
** Render an signed 64-bit integer as text.  Store the result in zOut[] and
** return the length of the string that was stored, in bytes.  The value
** returned does not include the zero terminator at the end of the output
** string.
**
** The caller must ensure that zOut[] is at least 21 bytes in size.
*/
ਪੂਰਨ sqlite3_Int64_ਮੰਜ਼ਿਲ_ਪਾਠ(i64 v, ਅੱਖਰ *z_ਬਾਹਰਲਾ){
  ਪੂਰਨ i;
  u64 x;
  ਸੰਘ {
    ਅੱਖਰ a[SQLITE_U64_DIGITS+1];
    u16 forceAlignment;
  } u;
  ਜੇ( v>0 ){
    x = v;
  }ਨਹੀਂਤਾਂ ਜੇ( v==0 ){
    z_ਬਾਹਰਲਾ[0] = '0';
    z_ਬਾਹਰਲਾ[1] = 0;
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    x = (v==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64) ? ((u64)1)<<63 : (u64)-v;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_AVOID_U64_DIVIDE
  i = sqlite3_UInt64_ਮੰਜ਼ਿਲ_ਪਾਠ(x, u.a);
#ਨਹੀਂਤਾਂ
  i = ਆਕਾਰ(u.a)-1;
  u.a[i] = 0;
  ਜਦੋਂ( x>=10 ){
    ਪੂਰਨ kk = (x%100)*2;
    ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_ਜੋੜੇ.a[kk]) );
    ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&u.a[i-2]) );
    *(u16*)(&u.a[i-2]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[kk];
    i -= 2;
    x /= 100;
  }
  ਜੇ( x ){
    u.a[--i] = x + '0';
  }
#ਜੇ_ਅੰਤ /* SQLITE_AVOID_U64_DIVIDE */
  ਜੇ( v<0 ) u.a[--i] = '-';
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਬਾਹਰਲਾ, &u.a[i], ਆਕਾਰ(u.a)-i);
  ਵਾਪਸ ਆਕਾਰ(u.a)-1-i;
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
ਸਥਿਰ ਪੂਰਨ compare2pow63(ਅਚਲ ਅੱਖਰ *z_ਅੰਕ, ਪੂਰਨ incr){
  ਪੂਰਨ c = 0;
  ਪੂਰਨ i;
                    /* 012345678901234567 */
  ਅਚਲ ਅੱਖਰ *pow63 = "922337203685477580";
  ਲਈ(i=0; c==0 && i<18; i++){
    c = (z_ਅੰਕ[i*incr]-pow63[i])*10;
  }
  ਜੇ( c==0 ){
    c = z_ਅੰਕ[18*incr] - '8';
    testcase( c==(-1) );
    testcase( c==0 );
    testcase( c==(+1) );
  }
  ਵਾਪਸ c;
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
ਪੂਰਨ sqlite3Atoi64(ਅਚਲ ਅੱਖਰ *z_ਅੰਕ, i64 *p_ਅੰਕ, ਪੂਰਨ ਲੰਮਾਈ, u8 enc){
  ਪੂਰਨ incr;
  u64 u = 0;
  ਪੂਰਨ neg = 0; /* assume positive */
  ਪੂਰਨ i, j;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c = 0;
  ਪੂਰਨ non_ਅੰਕ = 0;  /* True if input contains UTF16 with high byte non-zero */
  ਪੂਰਨ rc;          /* Baseline return code */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ;
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ = z_ਅੰਕ + ਲੰਮਾਈ;
  ਪ੍ਰਤਿਜ੍ਞਾ( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  ਜੇ( enc==SQLITE_UTF8 ){
    incr = 1;
  }ਨਹੀਂਤਾਂ{
    incr = 2;
    ਲੰਮਾਈ &= ~1;
    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    ਲਈ(i=3-enc; i<ਲੰਮਾਈ && z_ਅੰਕ[i]==0; i+=2){}
    non_ਅੰਕ = i<ਲੰਮਾਈ;
    z_ਸਿਰਾ = &z_ਅੰਕ[i^1];
    z_ਅੰਕ += (enc&1);
  }
  ਜਦੋਂ( z_ਅੰਕ<z_ਸਿਰਾ && sqlite3Isspace(*z_ਅੰਕ) ) z_ਅੰਕ+=incr;
  ਜੇ( z_ਅੰਕ<z_ਸਿਰਾ ){
    ਜੇ( *z_ਅੰਕ=='-' ){
      neg = 1;
      z_ਅੰਕ+=incr;
    }ਨਹੀਂਤਾਂ ਜੇ( *z_ਅੰਕ=='+' ){
      z_ਅੰਕ+=incr;
    }
  }
  z_ਸ਼ੁਰੂ = z_ਅੰਕ;
  ਜਦੋਂ( z_ਅੰਕ<z_ਸਿਰਾ && z_ਅੰਕ[0]=='0' ){ z_ਅੰਕ+=incr; } /* Skip leading zeros. */
  ਲਈ(i=0; &z_ਅੰਕ[i]<z_ਸਿਰਾ && (c=(ਬਿਨਾਨਿਸ਼ਾਨੀ)z_ਅੰਕ[i]-'0')<=9; i+=incr){
    u = u*10 + c;
  }
  testcase( i==18*incr );
  testcase( i==19*incr );
  testcase( i==20*incr );
  ਜੇ( u>ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 ){
    /* This test and assignment is needed only to suppress UB warnings
    ** from clang and -fsanitize=undefined.  This test and assignment make
    ** the code a little larger and slower, and no harm comes from omitting
    ** them, but we must appease the undefined-behavior pharisees. */
    *p_ਅੰਕ = neg ? ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 : ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64;
  }ਨਹੀਂਤਾਂ ਜੇ( neg ){
    *p_ਅੰਕ = -(i64)u;
  }ਨਹੀਂਤਾਂ{
    *p_ਅੰਕ = (i64)u;
  }
  rc = 0;
  ਜੇ( i==0 && z_ਸ਼ੁਰੂ==z_ਅੰਕ ){    /* No digits */
    rc = -1;
  }ਨਹੀਂਤਾਂ ਜੇ( non_ਅੰਕ ){            /* UTF16 with high-order bytes non-zero */
    rc = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( &z_ਅੰਕ[i]<z_ਸਿਰਾ ){     /* Extra bytes at the end */
    ਪੂਰਨ jj = i;
    ਕਰੋ{
      ਜੇ( !sqlite3Isspace(z_ਅੰਕ[jj]) ){
        rc = 1;          /* Extra non-space text after the integer */
        ਰੋਕੋ;
      }
      jj += incr;
    }ਜਦੋਂ( &z_ਅੰਕ[jj]<z_ਸਿਰਾ );
  }
  ਜੇ( i<19*incr ){
    /* Less than 19 digits, so we know that it fits in 64 bits */
    ਪ੍ਰਤਿਜ੍ਞਾ( u<=ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 );
    ਵਾਪਸ rc;
  }ਨਹੀਂਤਾਂ{
    /* zNum is a 19-digit numbers.  Compare it against 9223372036854775808. */
    j = i>19*incr ? 1 : compare2pow63(z_ਅੰਕ, incr);
    ਜੇ( j<0 ){
      /* zNum is less than 9223372036854775808 so it fits */
      ਪ੍ਰਤਿਜ੍ਞਾ( u<=ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 );
      ਵਾਪਸ rc;
    }ਨਹੀਂਤਾਂ{
      *p_ਅੰਕ = neg ? ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 : ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64;
      ਜੇ( j>0 ){
        /* zNum is greater than 9223372036854775808 so it overflows */
        ਵਾਪਸ 2;
      }ਨਹੀਂਤਾਂ{
        /* zNum is exactly 9223372036854775808.  Fits if negative.  The
        ** special case 2 overflow if positive */
        ਪ੍ਰਤਿਜ੍ਞਾ( u-1==ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 );
        ਵਾਪਸ neg ? rc : 3;
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
ਪੂਰਨ sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(ਅਚਲ ਅੱਖਰ *z, i64 *p_ਬਾਹਰਲਾ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_HEX_INTEGER
  ਜੇ( z[0]=='0'
   && (z[1]=='x' || z[1]=='X')
  ){
    u64 u = 0;
    ਪੂਰਨ i, k;
    ਲਈ(i=2; z[i]=='0'; i++){}
    ਲਈ(k=i; sqlite3Isxdigit(z[k]); k++){
      u = u*16 + sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(z[k]);
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬਾਹਰਲਾ, &u, 8);
    ਜੇ( k-i>16 ) ਵਾਪਸ 2;
    ਜੇ( z[k]!=0 ) ਵਾਪਸ 1;
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_HEX_INTEGER */
  {
    ਪੂਰਨ n = (ਪੂਰਨ)(0x3fffffff&strspn(z,"+- \n\t0123456789"));
    ਜੇ( z[n] ) n++;
    ਵਾਪਸ sqlite3Atoi64(z, p_ਬਾਹਰਲਾ, n, SQLITE_UTF8);
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
ਪੂਰਨ sqlite3GetInt32(ਅਚਲ ਅੱਖਰ *z_ਅੰਕ, ਪੂਰਨ *p_ਮੁੱਲ){
  sqlite_int64 v = 0;
  ਪੂਰਨ i, c;
  ਪੂਰਨ neg = 0;
  ਜੇ( z_ਅੰਕ[0]=='-' ){
    neg = 1;
    z_ਅੰਕ++;
  }ਨਹੀਂਤਾਂ ਜੇ( z_ਅੰਕ[0]=='+' ){
    z_ਅੰਕ++;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_HEX_INTEGER
  ਨਹੀਂਤਾਂ ਜੇ( z_ਅੰਕ[0]=='0'
        && (z_ਅੰਕ[1]=='x' || z_ਅੰਕ[1]=='X')
        && sqlite3Isxdigit(z_ਅੰਕ[2])
  ){
    u32 u = 0;
    z_ਅੰਕ += 2;
    ਜਦੋਂ( z_ਅੰਕ[0]=='0' ) z_ਅੰਕ++;
    ਲਈ(i=0; i<8 && sqlite3Isxdigit(z_ਅੰਕ[i]); i++){
      u = u*16 + sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(z_ਅੰਕ[i]);
    }
    ਜੇ( (u&0x80000000)==0 && sqlite3Isxdigit(z_ਅੰਕ[i])==0 ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੁੱਲ, &u, 4);
      ਵਾਪਸ 1;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ 0;
    }
  }
#ਜੇ_ਅੰਤ
  ਜੇ( !sqlite3Isdigit(z_ਅੰਕ[0]) ) ਵਾਪਸ 0;
  ਜਦੋਂ( z_ਅੰਕ[0]=='0' ) z_ਅੰਕ++;
  ਲਈ(i=0; i<11 && (c = z_ਅੰਕ[i] - '0')>=0 && c<=9; i++){
    v = v*10 + c;
  }

  /* The longest decimal representation of a 32 bit integer is 10 digits:
  **
  **             1234567890
  **     2^31 -> 2147483648
  */
  testcase( i==10 );
  ਜੇ( i>10 ){
    ਵਾਪਸ 0;
  }
  testcase( v-neg==2147483647 );
  ਜੇ( v-neg>2147483647 ){
    ਵਾਪਸ 0;
  }
  ਜੇ( neg ){
    v = -v;
  }
  *p_ਮੁੱਲ = (ਪੂਰਨ)v;
  ਵਾਪਸ 1;
}

/*
** Return a 32-bit integer value extracted from a string.  If the
** string is not an integer, just return 0.
*/
ਪੂਰਨ sqlite3Atoi(ਅਚਲ ਅੱਖਰ *z){
  ਪੂਰਨ x = 0;
  sqlite3GetInt32(z, &x);
  ਵਾਪਸ x;
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
ਖਾਲੀ sqlite3FpDecode(FpDecode *p, ਦੁੱਗਣਾ r, ਪੂਰਨ i_ਗੇੜਾ, ਪੂਰਨ mx_ਗੇੜਾ){
  ਪੂਰਨ i;               /* Index into zBuf[] where to put next character */
  ਪੂਰਨ n;               /* Number of digits */
  u64 v;               /* mantissa */
  ਪੂਰਨ e, ਘਾਤਾਂਕ = 0;      /* Base-2 and base-10 exponent */
  ਅੱਖਰ *zBuf;          /* Local alias for p->zBuf */
  ਅੱਖਰ *z;             /* Local alias for p->z */

  p->isSpecial = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( mx_ਗੇੜਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਗੇੜਾ>=-100000000 );

  /* Convert negative numbers to positive.  Deal with Infinity, 0.0, and
  ** NaN. */
  ਜੇ( r<0.0 ){
    p->sign = '-';
    r = -r;
  }ਨਹੀਂਤਾਂ ਜੇ( r==0.0 ){
    p->sign = '+';
    p->n = 1;
    p->iDP = 1;
    p->z = "0";
    ਵਾਪਸ;
  }ਨਹੀਂਤਾਂ{
    p->sign = '+';
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&v,&r,8);
  e = (v>>52)&0x7ff;
  ਜੇ( e==0x7ff ){
    p->isSpecial = 1 + (v!=0x7ff0000000000000LL);
    p->n = 0;
    p->iDP = 0;
    p->z = p->zBuf;
    ਵਾਪਸ;
  }
  v &= 0x000fffffffffffffULL;
  ਜੇ( e==0 ){
    ਪੂਰਨ nn = ਗਿਣਤੀ_Leading_Zeros(v);
    v <<= nn;
    e = -1074 - nn;
  }ਨਹੀਂਤਾਂ{
    v = (v<<11) | U64_ਦੋਆਧਾਰੀ_ਅੰਕ(63);
    e -= 1086;
  }
  sqlite3Fp2Convert10(v, e, (i_ਗੇੜਾ<=0||i_ਗੇੜਾ>=18)?18:i_ਗੇੜਾ+1, &v, &ਘਾਤਾਂਕ);  

  /* Extract significant digits, start at the right-most slot in p->zBuf
  ** and working back to the right.  "i" keeps track of the next slot in
  ** which to store a digit. */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(p->zBuf)==SQLITE_U64_DIGITS+1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( v>0 );
  zBuf = p->zBuf;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_AVOID_U64_DIVIDE
  i = sqlite3_UInt64_ਮੰਜ਼ਿਲ_ਪਾਠ(v, zBuf);
#ਨਹੀਂਤਾਂ
  i = SQLITE_U64_DIGITS;
  ਜਦੋਂ( v>=10 ){
    ਪੂਰਨ kk = (v%100)*2;
    ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&sqlite3_Digit_ਜੋੜੇ.a[kk]) );
    ਪ੍ਰਤਿਜ੍ਞਾ( TWO_BYTE_ALIGNMENT(&zBuf[i]) );
    ਪ੍ਰਤਿਜ੍ਞਾ( i-2>=0 );
    *(u16*)(&zBuf[i-2]) = *(u16*)&sqlite3_Digit_ਜੋੜੇ.a[kk];
    i -= 2;
    v /= 100;
  }
  ਜੇ( v ){
    ਪ੍ਰਤਿਜ੍ਞਾ( v<10 );
    ਪ੍ਰਤਿਜ੍ਞਾ( i>0 );
    zBuf[--i] = v + '0';
  }
#ਜੇ_ਅੰਤ /* SQLITE_AVOID_U64_DIVIDE */
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<SQLITE_U64_DIGITS );
  n = SQLITE_U64_DIGITS - i;  /* Total number of digits extracted */
  ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( n<=SQLITE_U64_DIGITS );
  p->iDP = n + ਘਾਤਾਂਕ;
  ਜੇ( i_ਗੇੜਾ<=0 ){
    i_ਗੇੜਾ = p->iDP - i_ਗੇੜਾ;
    ਜੇ( i_ਗੇੜਾ==0 && zBuf[i]>='5' ){
      i_ਗੇੜਾ = 1;
      zBuf[--i] = '0';
      n++;
      p->iDP++;
    }
  }
  z = &zBuf[i];  /* z points to the first digit */
  ਜੇ( i_ਗੇੜਾ>0 && (i_ਗੇੜਾ<n || n>mx_ਗੇੜਾ) ){
    ਜੇ( i_ਗੇੜਾ>mx_ਗੇੜਾ ) i_ਗੇੜਾ = mx_ਗੇੜਾ;
    ਜੇ( i_ਗੇੜਾ==17 ){
      /* If the precision is exactly 17, which only happens with the "!"
      ** flag (ex: "%!.17g") then try to reduce the precision if that
      ** yields text that will round-trip to the original floating-point.
      ** value.  Thus, for exaple, 49.47 will render as 49.47, rather than
      ** as 49.469999999999999. */
      ਜੇ( z[15]=='9' && z[14]=='9' ){
        ਪੂਰਨ jj, kk;
        u64 v2;
        ਲਈ(jj=14; jj>0 && z[jj-1]=='9'; jj--){}
        ਜੇ( jj==0 ){
          v2 = 1;
        }ਨਹੀਂਤਾਂ{
          v2 = z[0] - '0';
          ਲਈ(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
          v2++;
        }
        ਜੇ( r==sqlite3Fp10Convert2(v2, ਘਾਤਾਂਕ + n - jj) ){
          i_ਗੇੜਾ = jj+1;
        }
      }ਨਹੀਂਤਾਂ ਜੇ( p->iDP>=n || (z[15]=='0' && z[14]=='0' && z[13]=='0') ){
        ਪੂਰਨ jj, kk;
        u64 v2;
        ਪ੍ਰਤਿਜ੍ਞਾ( z[0]!='0' );
        ਲਈ(jj=13; z[jj-1]=='0'; jj--){}
        v2 = z[0] - '0';
        ਲਈ(kk=1; kk<jj; kk++) v2 = (v2*10) + z[kk] - '0';
        ਜੇ( r==sqlite3Fp10Convert2(v2, ਘਾਤਾਂਕ + n - jj) ){
          i_ਗੇੜਾ = jj+1;
        }
      }
    }
    n = i_ਗੇੜਾ;
    ਜੇ( z[i_ਗੇੜਾ]>='5' ){
      ਪੂਰਨ j = i_ਗੇੜਾ-1;
      ਜਦੋਂ( 1 /*exit-by-break*/ ){
        z[j]++;
        ਜੇ( z[j]<='9' ) ਰੋਕੋ;
        z[j] = '0';
        ਜੇ( j==0 ){
          z--;
          z[0] = '1';
          n++;
          p->iDP++;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          j--;
        }
      }
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );
  ਜਦੋਂ( z[n-1]=='0' ){
    n--;
    ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );
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
ਪੂਰਨ sqlite3GetUInt32(ਅਚਲ ਅੱਖਰ *z, u32 *pI){
  u64 v = 0;
  ਪੂਰਨ i;
  ਲਈ(i=0; sqlite3Isdigit(z[i]); i++){
    v = v*10 + z[i] - '0';
    ਜੇ( v>4294967295LL ){ *pI = 0; ਵਾਪਸ 0; }
  }
  ਜੇ( i==0 || z[i]!=0 ){ *pI = 0; ਵਾਪਸ 0; }
  *pI = (u32)v;
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_NOINLINE putVarint64(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p, u64 v){
  ਪੂਰਨ i, j, n;
  u8 buf[10];
  ਜੇ( v & (((u64)0xff000000)<<32) ){
    p[8] = (u8)v;
    v >>= 8;
    ਲਈ(i=7; i>=0; i--){
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    ਵਾਪਸ 9;
  }   
  n = 0;
  ਕਰੋ{
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  }ਜਦੋਂ( v!=0 );
  buf[0] &= 0x7f;
  ਪ੍ਰਤਿਜ੍ਞਾ( n<=9 );
  ਲਈ(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  ਵਾਪਸ n;
}
ਪੂਰਨ sqlite3PutVarint(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p, u64 v){
  ਜੇ( v<=0x7f ){
    p[0] = v&0x7f;
    ਵਾਪਸ 1;
  }
  ਜੇ( v<=0x3fff ){
    p[0] = ((v>>7)&0x7f)|0x80;
    p[1] = v&0x7f;
    ਵਾਪਸ 2;
  }
  ਵਾਪਸ putVarint64(p,v);
}

/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
u8 sqlite3GetVarint(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p, u64 *v){
  u64 i_ਕੁੰਜੀ = p[0];
  ਅਚਲ u8 *p_ਸ਼ੁਰੂ = p;
  ਜੇ( i_ਕੁੰਜੀ>=0x80 ){
    u8 x;
    i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ (x = *++p);
    ਜੇ( x>=0x80 ){
      i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ (x = *++p);
      ਜੇ( x>=0x80 ){
        i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x10204000 ^ (x = *++p);
        ਜੇ( x>=0x80 ){
          i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
          ਜੇ( x>=0x80 ){
            i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
            ਜੇ( x>=0x80 ){
              i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
              ਜੇ( x>=0x80 ){
                i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
                ਜੇ( x>=0x80 ){
                  i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }ਨਹੀਂਤਾਂ{
        i_ਕੁੰਜੀ ^= 0x204000;
      }
    }ਨਹੀਂਤਾਂ{
      i_ਕੁੰਜੀ ^= 0x4000;
    }
  }
  *v = i_ਕੁੰਜੀ;
  ਵਾਪਸ (u8)(p - p_ਸ਼ੁਰੂ) + 1;
}

/*
** Return the value of a variable-length integer without computing
** its length.  This is an optimization on sqlite3GetVarint() for the
** cases when the return value of sqlite3GetVarint() is not needed.
*/
i64 sqlite3_Varint_ਮੁੱਲ(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p){
  u64 i_ਕੁੰਜੀ = p[0];
  ਜੇ( i_ਕੁੰਜੀ>=0x80 ){
    u8 x;
    i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ (x = *++p);
    ਜੇ( x>=0x80 ){
      i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ (x = *++p);
      ਜੇ( x>=0x80 ){
        i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x10204000 ^ (x = *++p);
        ਜੇ( x>=0x80 ){
          i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
          ਜੇ( x>=0x80 ){
            i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
            ਜੇ( x>=0x80 ){
              i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
              ਜੇ( x>=0x80 ){
                i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++p);
                ਜੇ( x>=0x80 ){
                  i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<8) ^ 0x8000 ^ (*++p);
                }
              }
            }
          }
        }
      }ਨਹੀਂਤਾਂ{
        i_ਕੁੰਜੀ ^= 0x204000;
      }
    }ਨਹੀਂਤਾਂ{
      i_ਕੁੰਜੀ ^= 0x4000;
    }
  }
  ਵਾਪਸ *(i64*)&i_ਕੁੰਜੀ;
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
u8 sqlite3GetVarint32(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p, u32 *v){
  u64 v64;
  u8 n;

  /* Assume that the single-byte case has already been handled by
  ** the getVarint32() macro */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p[0] & 0x80)!=0 );

  ਜੇ( (p[1] & 0x80)==0 ){
    /* This is the two-byte case */
    *v = ((p[0]&0x7f)<<7) | p[1];
    ਵਾਪਸ 2;
  }
  ਜੇ( (p[2] & 0x80)==0 ){
    /* This is the three-byte case */
    *v = ((p[0]&0x7f)<<14) | ((p[1]&0x7f)<<7) | p[2];
    ਵਾਪਸ 3;
  }
  /* four or more bytes */
  n = sqlite3GetVarint(p, &v64);
  ਪ੍ਰਤਿਜ੍ਞਾ( n>3 && n<=9 );
  ਜੇ( (v64 & SQLITE_MAX_U32)!=v64 ){
    *v = 0xffffffff;
  }ਨਹੀਂਤਾਂ{
    *v = (u32)v64;
  }
  ਵਾਪਸ n;
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
ਪੂਰਨ sqlite3VarintLen(u64 v){
  ਪੂਰਨ i;
  ਲਈ(i=1; (v >>= 7)!=0; i++){ ਪ੍ਰਤਿਜ੍ਞਾ( i<10 ); }
  ਵਾਪਸ i;
}


/*
** Read an unsigned 32-bit integer from an unaligned big-endian array of bytes.
*/
u32 sqlite3Get4byte(ਅਚਲ u8 *p){
#ਜੇ SQLITE_BYTEORDER==4321
  u32 x;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x,p,4);
  ਵਾਪਸ x;
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && GCC_ਸੰਸਕਰਣ>=4003000
  u32 x;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x,p,4);
  ਵਾਪਸ __builtin_bswap32(x);
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && MSVC_ਸੰਸਕਰਣ>=1300
  u32 x;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x,p,4);
  ਵਾਪਸ _byteswap_ulong(x);
#ਨਹੀਂਤਾਂ
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  ਵਾਪਸ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
#ਜੇ_ਅੰਤ
}

/* Write an unsigned 32-bit integer into an unaligned big-endian array of bytes.
*/
ਖਾਲੀ sqlite3Put4byte(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p, u32 v){
#ਜੇ SQLITE_BYTEORDER==4321
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p,&v,4);
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && GCC_ਸੰਸਕਰਣ>=4003000
  u32 x = __builtin_bswap32(v);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p,&x,4);
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && MSVC_ਸੰਸਕਰਣ>=1300
  u32 x = _byteswap_ulong(v);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p,&x,4);
#ਨਹੀਂਤਾਂ
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  p[0] = (u8)(v>>24);
  p[1] = (u8)(v>>16);
  p[2] = (u8)(v>>8);
  p[3] = (u8)v;
#ਜੇ_ਅੰਤ
}

/*
** Read an unsigned 64-bit integer from an unaligned big-endian byte array.
*/
SQLITE_OPT_INLINE u64 sqlite3Get8byte(ਅਚਲ u8 *p){
#ਜੇ SQLITE_BYTEORDER==4321
  u64 x;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x,p,8);
  ਵਾਪਸ x;
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && GCC_ਸੰਸਕਰਣ>=4003000
  u64 x;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x,p,8);
  ਵਾਪਸ __builtin_bswap64(x);
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && MSVC_ਸੰਸਕਰਣ>=1300
  u64 x;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x,p,8);
  ਵਾਪਸ _byteswap_uint64(x);
#ਨਹੀਂਤਾਂ
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  testcase( p[0]&0x80 );
  ਵਾਪਸ (u64)(
    (((u64)p[0]) << 56) +
    (((u64)p[1]) << 48) +
    (((u64)p[2]) << 40) +
    (((u64)p[3]) << 32) +
    (((u64)p[4]) << 24) +
    (((u64)p[5]) << 16) +
    (((u64)p[6]) <<  8) +
    (((u64)p[7]) <<  0)
  );
#ਜੇ_ਅੰਤ
}

#ਜੇ SQLITE_BYTEORDER!=4321  /* Only used for little-endian machines */
/*
** Byte-swap a 64-bit unsigned integer.
*/
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64 x){
#ਜੇ SQLITE_BYTEORDER==1234 && GCC_ਸੰਸਕਰਣ>=4003000
  ਵਾਪਸ __builtin_bswap64(x);
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234 && MSVC_ਸੰਸਕਰਣ>=1300
  ਵਾਪਸ _byteswap_uint64(x);
#ਨਹੀਂਤਾਂ
  /* Test this limb using -DSQLITE_BYTEORDER=0 */
  x = (x << 32) | (x >> 32);
  x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) |
      ((x & UINT64_C(0xffff0000ffff0000)) >> 16);
  x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) |
      ((x & UINT64_C(0xff00ff00ff00ff00)) >>  8);
  ਵਾਪਸ x;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ /* SQLITE_BYTEORDER!=4321 */

/*
** Translate a single byte of Hex into an integer.
** This routine only works if h really is a valid hexadecimal
** character:  0..9a..fA..F
*/
u8 sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(ਪੂਰਨ h){
  ਪ੍ਰਤਿਜ੍ਞਾ( (h>='0' && h<='9') ||  (h>='a' && h<='f') ||  (h>='A' && h<='F') );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
  h += 9*(1&(h>>6));
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#ਜੇ_ਅੰਤ
  ਵਾਪਸ (u8)(h & 0xf);
}

#ਜੇ !defined(SQLITE_OMIT_BLOB_LITERAL)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
ਖਾਲੀ *sqlite3_Hex_ਮੰਜ਼ਿਲ_Blob(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ n){
  ਅੱਖਰ *zBlob;
  ਪੂਰਨ i;

  zBlob = (ਅੱਖਰ *)sqlite3DbMallocRawNN(db, n/2 + 1);
  n--;
  ਜੇ( zBlob ){
    ਲਈ(i=0; i<n; i+=2){
      zBlob[i/2] = (sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(z[i])<<4) | sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(z[i+1]);
    }
    zBlob[i/2] = 0;
  }
  ਵਾਪਸ zBlob;
}
#ਜੇ_ਅੰਤ /* !SQLITE_OMIT_BLOB_LITERAL */

/*
** Log an error that is an API call on a connection pointer that should
** not have been used.  The "type" of connection pointer is given as the
** argument.  The zType is a word like "NULL" or "closed" or "invalid".
*/
ਸਥਿਰ ਖਾਲੀ logBadConnection(ਅਚਲ ਅੱਖਰ *zType){
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
ਪੂਰਨ sqlite3SafetyCheckOk(sqlite3 *db){
  u8 eOpenState;
  ਜੇ( db==0 ){
    logBadConnection("NULL");
    ਵਾਪਸ 0;
  }
  eOpenState = db->eOpenState;
  ਜੇ( eOpenState!=SQLITE_STATE_OPEN ){
    ਜੇ( sqlite3SafetyCheckSickOrOk(db) ){
      testcase( sqlite3GlobalConfig.xLog!=0 );
      logBadConnection("unopened");
    }
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 1;
  }
}
ਪੂਰਨ sqlite3SafetyCheckSickOrOk(sqlite3 *db){
  u8 eOpenState;
  eOpenState = db->eOpenState;
  ਜੇ( eOpenState!=SQLITE_STATE_SICK &&
      eOpenState!=SQLITE_STATE_OPEN &&
      eOpenState!=SQLITE_STATE_BUSY ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    logBadConnection("invalid");
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 1;
  }
}

/*
** Attempt to add, subtract, or multiply the 64-bit signed value iB against
** the other 64-bit signed integer at *pA and store the result in *pA.
** Return 0 on success.  Or if the operation would have resulted in an
** overflow, leave *pA unchanged and return 1.
*/
ਪੂਰਨ sqlite3_ਜੋੜੋ_Int64(i64 *pA, i64 iB){
#ਜੇ GCC_ਸੰਸਕਰਣ>=5004000 && !defined(__INTEL_COMPILER)
  ਵਾਪਸ __builtin_add_overflow(*pA, iB, pA);
#ਨਹੀਂਤਾਂ
  i64 iA = *pA;
  testcase( iA==0 ); testcase( iA==1 );
  testcase( iB==-1 ); testcase( iB==0 );
  ਜੇ( iB>=0 ){
    testcase( iA>0 && ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 - iA == iB );
    testcase( iA>0 && ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 - iA == iB - 1 );
    ਜੇ( iA>0 && ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 - iA < iB ) ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    testcase( iA<0 && -(iA + ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64) == iB + 1 );
    testcase( iA<0 && -(iA + ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64) == iB + 2 );
    ਜੇ( iA<0 && -(iA + ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64) > iB + 1 ) ਵਾਪਸ 1;
  }
  *pA += iB;
  ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}
ਪੂਰਨ sqlite3SubInt64(i64 *pA, i64 iB){
#ਜੇ GCC_ਸੰਸਕਰਣ>=5004000 && !defined(__INTEL_COMPILER)
  ਵਾਪਸ __builtin_sub_overflow(*pA, iB, pA);
#ਨਹੀਂਤਾਂ
  testcase( iB==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64+1 );
  ਜੇ( iB==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 ){
    testcase( (*pA)==(-1) ); testcase( (*pA)==0 );
    ਜੇ( (*pA)>=0 ) ਵਾਪਸ 1;
    *pA -= iB;
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ sqlite3_ਜੋੜੋ_Int64(pA, -iB);
  }
#ਜੇ_ਅੰਤ
}
ਪੂਰਨ sqlite3MulInt64(i64 *pA, i64 iB){
#ਜੇ GCC_ਸੰਸਕਰਣ>=5004000 && !defined(__INTEL_COMPILER)
  ਵਾਪਸ __builtin_mul_overflow(*pA, iB, pA);
#ਨਹੀਂਤਾਂ
  i64 iA = *pA;
  ਜੇ( iB>0 ){
    ਜੇ( iA>ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64/iB ) ਵਾਪਸ 1;
    ਜੇ( iA<ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64/iB ) ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ ਜੇ( iB<0 ){
    ਜੇ( iA>0 ){
      ਜੇ( iB<ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64/iA ) ਵਾਪਸ 1;
    }ਨਹੀਂਤਾਂ ਜੇ( iA<0 ){
      ਜੇ( iB==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 ) ਵਾਪਸ 1;
      ਜੇ( iA==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 ) ਵਾਪਸ 1;
      ਜੇ( -iA>ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64/-iB ) ਵਾਪਸ 1;
    }
  }
  *pA = iA*iB;
  ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

/*
** Compute the absolute value of a 32-bit signed integer, if possible.  Or
** if the integer has a value of -2147483648, return +2147483647
*/
ਪੂਰਨ sqlite3AbsInt32(ਪੂਰਨ x){
  ਜੇ( x>=0 ) ਵਾਪਸ x;
  ਜੇ( x==(ਪੂਰਨ)0x80000000 ) ਵਾਪਸ 0x7fffffff;
  ਵਾਪਸ -x;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_8_3_ਨਾਂ
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
ਖਾਲੀ sqlite3FileSuffix3(ਅਚਲ ਅੱਖਰ *z_ਅਧਾਰ_Filename, ਅੱਖਰ *z){
#ਜੇ SQLITE_ENABLE_8_3_ਨਾਂ<2
  ਜੇ( sqlite3_uri_boolean(z_ਅਧਾਰ_Filename, "8_3_names", 0) )
#ਜੇ_ਅੰਤ
  {
    ਪੂਰਨ i, sz;
    sz = sqlite3Strlen30(z);
    ਲਈ(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    ਜੇ( z[i]=='.' && ALWAYS(sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
  }
}
#ਜੇ_ਅੰਤ

/*
** Find (an approximate) sum of two LogEst values.  This computation is
** not a simple "+" operator because LogEst is stored as a logarithmic
** value.
**
*/
LogEst sqlite3_Log_Est_ਜੋੜੋ(LogEst a, LogEst b){
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ x[] = {
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
  ਜੇ( a>=b ){
    ਜੇ( a>b+49 ) ਵਾਪਸ a;
    ਜੇ( a>b+31 ) ਵਾਪਸ a+1;
    ਵਾਪਸ a+x[a-b];
  }ਨਹੀਂਤਾਂ{
    ਜੇ( b>a+49 ) ਵਾਪਸ b;
    ਜੇ( b>a+31 ) ਵਾਪਸ b+1;
    ਵਾਪਸ b+x[b-a];
  }
}

/*
** Convert an integer into a LogEst.  In other words, compute an
** approximation for 10*log2(x).
*/
LogEst sqlite3LogEst(u64 x){
  ਸਥਿਰ LogEst a[] = { 0, 2, 3, 5, 6, 7, 8, 9 };
  LogEst y = 40;
  ਜੇ( x<8 ){
    ਜੇ( x<2 ) ਵਾਪਸ 0;
    ਜਦੋਂ( x<8 ){  y -= 10; x <<= 1; }
  }ਨਹੀਂਤਾਂ{
#ਜੇ GCC_ਸੰਸਕਰਣ>=5004000
    ਪੂਰਨ i = 60 - __builtin_clzll(x);
    y += i*10;
    x >>= i;
#ਨਹੀਂਤਾਂ
    ਜਦੋਂ( x>255 ){ y += 40; x >>= 4; }  /*OPTIMIZATION-IF-TRUE*/
    ਜਦੋਂ( x>15 ){  y += 10; x >>= 1; }
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ a[x&7] + y - 10;
}

/*
** Convert a double into a LogEst
** In other words, compute an approximation for 10*log2(x).
*/
LogEst sqlite3LogEstFromDouble(ਦੁੱਗਣਾ x){
  u64 a;
  LogEst e;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(x)==8 && ਆਕਾਰ(a)==8 );
  ਜੇ( x<=1 ) ਵਾਪਸ 0;
  ਜੇ( x<=2000000000 ) ਵਾਪਸ sqlite3LogEst((u64)x);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a, &x, 8);
  e = (a>>52) - 1022;
  ਵਾਪਸ e*10;
}

/*
** Convert a LogEst into an integer.
*/
u64 sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(LogEst x){
  u64 n;
  n = x%10;
  x /= 10;
  ਜੇ( n>=5 ) n -= 2;
  ਨਹੀਂਤਾਂ ਜੇ( n>=1 ) n -= 1;
  ਜੇ( x>60 ) ਵਾਪਸ (u64)ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64;
  ਵਾਪਸ x>=3 ? (n+8)<<(x-3) : (n+8)>>(3-x);
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
VList *sqlite3_VList_ਜੋੜੋ(
  sqlite3 *db,           /* The database connection used for malloc() */
  VList *p_ਵਿੱਚ,            /* The input VList.  Might be NULL */
  ਅਚਲ ਅੱਖਰ *zName,     /* Name of symbol to add */
  ਪੂਰਨ nName,             /* Bytes of text in zName */
  ਪੂਰਨ iVal               /* Value to associate with zName */
){
  ਪੂਰਨ nInt;              /* number of sizeof(int) objects needed for zName */
  ਅੱਖਰ *z;               /* Pointer to where zName will be stored */
  ਪੂਰਨ i;                 /* Index in pIn[] where zName is stored */

  nInt = nName/4 + 3;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿੱਚ==0 || p_ਵਿੱਚ[0]>=3 );  /* Verify ok to add new elements */
  ਜੇ( p_ਵਿੱਚ==0 || p_ਵਿੱਚ[1]+nInt > p_ਵਿੱਚ[0] ){
    /* Enlarge the allocation */
    sqlite3_int64 nAlloc = (p_ਵਿੱਚ ? 2*(sqlite3_int64)p_ਵਿੱਚ[0] : 10) + nInt;
    VList *p_ਬਾਹਰਲਾ = sqlite3DbRealloc(db, p_ਵਿੱਚ, nAlloc*ਆਕਾਰ(ਪੂਰਨ));
    ਜੇ( p_ਬਾਹਰਲਾ==0 ) ਵਾਪਸ p_ਵਿੱਚ;
    ਜੇ( p_ਵਿੱਚ==0 ) p_ਬਾਹਰਲਾ[1] = 2;
    p_ਵਿੱਚ = p_ਬਾਹਰਲਾ;
    p_ਵਿੱਚ[0] = nAlloc;
  }
  i = p_ਵਿੱਚ[1];
  p_ਵਿੱਚ[i] = iVal;
  p_ਵਿੱਚ[i+1] = nInt;
  z = (ਅੱਖਰ*)&p_ਵਿੱਚ[i+2];
  p_ਵਿੱਚ[1] = i+nInt;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿੱਚ[1]<=p_ਵਿੱਚ[0] );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(z, zName, nName);
  z[nName] = 0;
  ਵਾਪਸ p_ਵਿੱਚ;
}

/*
** Return a pointer to the name of a variable in the given VList that
** has the value iVal.  Or return a NULL if there is no such variable in
** the list
*/
ਅਚਲ ਅੱਖਰ *sqlite3_VList_ਅੰਕ_ਮੰਜ਼ਿਲ_Name(VList *p_ਵਿੱਚ, ਪੂਰਨ iVal){
  ਪੂਰਨ i, mx;
  ਜੇ( p_ਵਿੱਚ==0 ) ਵਾਪਸ 0;
  mx = p_ਵਿੱਚ[1];
  i = 2;
  ਕਰੋ{
    ਜੇ( p_ਵਿੱਚ[i]==iVal ) ਵਾਪਸ (ਅੱਖਰ*)&p_ਵਿੱਚ[i+2];
    i += p_ਵਿੱਚ[i+1];
  }ਜਦੋਂ( i<mx );
  ਵਾਪਸ 0;
}

/*
** Return the number of the variable named zName, if it is in VList.
** or return 0 if there is no such variable.
*/
ਪੂਰਨ sqlite3_VList_Name_ਮੰਜ਼ਿਲ_ਅੰਕ(VList *p_ਵਿੱਚ, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ nName){
  ਪੂਰਨ i, mx;
  ਜੇ( p_ਵਿੱਚ==0 ) ਵਾਪਸ 0;
  mx = p_ਵਿੱਚ[1];
  i = 2;
  ਕਰੋ{
    ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)&p_ਵਿੱਚ[i+2];
    ਜੇ( ਲੜੀ_ਤੁਲਨਾ(z,zName,nName)==0 && z[nName]==0 ) ਵਾਪਸ p_ਵਿੱਚ[i];
    i += p_ਵਿੱਚ[i+1];
  }ਜਦੋਂ( i<mx );
  ਵਾਪਸ 0;
}
