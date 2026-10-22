/*
** 2004 May 26
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code use to implement APIs that are part of the
** VDBE.
*/
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"
#شامل "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
صحیح sqlite3_expired(sqlite3_stmt *pStmt){
  صحیح iRet = 1;
  اگر( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  بازگشت iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ایستا صحیح vdbeSafety(Vdbe *p){
  اگر( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    بازگشت 1;
  }وگرنه{
    بازگشت 0;
  }
}
ایستا صحیح vdbeSafetyNotNull(Vdbe *p){
  اگر( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    بازگشت 1;
  }وگرنه{
    بازگشت vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ایستا SQLITE_NOINLINE تهی invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  أكد( p->startTime>0 );
  أكد( db->init.busy==0 );
  أكد( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  اگر( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  اگر( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (تهی*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   اگر( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#وگرنه
# define checkProfileCallback(DB,P)  /*no-op*/
#endif

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
صحیح sqlite3_finalize(sqlite3_stmt *pStmt){
  صحیح rc;
  اگر( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }وگرنه{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    اگر( vdbeSafety(v) ) بازگشت SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    أكد( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  بازگشت rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
صحیح sqlite3_reset(sqlite3_stmt *pStmt){
  صحیح rc;
  اگر( pStmt==0 ){
    rc = SQLITE_OK;
  }وگرنه{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    أكد( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  بازگشت rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
صحیح sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  صحیح i;
  صحیح rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#اگر SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pStmt==0 ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
#endif
#اگر SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  برای(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    اگر( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  اگر( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  بازگشت rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ثابت تهی *sqlite3_مقدار_blob(sqlite3_مقدار *pVal){
  Mem *p = (Mem*)pVal;
  اگر( p->flags & (MEM_Blob|MEM_Str) ){
    اگر( ExpandBlob(p)!=SQLITE_OK ){
      أكد( p->flags==MEM_Null && p->z==0 );
      بازگشت 0;
    }
    p->flags |= MEM_Blob;
    بازگشت p->n ? p->z : 0;
  }وگرنه{
    بازگشت sqlite3_مقدار_text(pVal);
  }
}
صحیح sqlite3_مقدار_bytes(sqlite3_مقدار *pVal){
  بازگشت sqlite3_مقدار_Bytes(pVal, SQLITE_UTF8);
}
صحیح sqlite3_مقدار_bytes16(sqlite3_مقدار *pVal){
  بازگشت sqlite3_مقدار_Bytes(pVal, SQLITE_UTF16NATIVE);
}
دوگانه sqlite3_مقدار_double(sqlite3_مقدار *pVal){
  بازگشت sqlite3_Vdbe_Real_مقدار((Mem*)pVal);
}
صحیح sqlite3_مقدار_int(sqlite3_مقدار *pVal){
  بازگشت (صحیح)sqlite3_Vdbe_Int_مقدار((Mem*)pVal);
}
sqlite_int64 sqlite3_مقدار_int64(sqlite3_مقدار *pVal){
  بازگشت sqlite3_Vdbe_Int_مقدار((Mem*)pVal);
}
unsigned صحیح sqlite3_مقدار_subtype(sqlite3_مقدار *pVal){
  Mem *pMem = (Mem*)pVal;
  بازگشت ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
تهی *sqlite3_مقدار_pointer(sqlite3_مقدار *pVal, ثابت نویسه *zPType){
  Mem *p = (Mem*)pVal;
  اگر( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    بازگشت (تهی*)p->z;
  }وگرنه{
    بازگشت 0;
  }
}
ثابت unsigned نویسه *sqlite3_مقدار_text(sqlite3_مقدار *pVal){
  بازگشت (ثابت unsigned نویسه *)sqlite3_مقدار_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_مقدار_text16(sqlite3_مقدار* pVal){
  بازگشت sqlite3_مقدار_Text(pVal, SQLITE_UTF16NATIVE);
}
ثابت تهی *sqlite3_مقدار_text16be(sqlite3_مقدار *pVal){
  بازگشت sqlite3_مقدار_Text(pVal, SQLITE_UTF16BE);
}
ثابت تهی *sqlite3_مقدار_text16le(sqlite3_مقدار *pVal){
  بازگشت sqlite3_مقدار_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
صحیح sqlite3_مقدار_type(sqlite3_مقدار* pVal){
  ایستا ثابت u8 aType[] = {
     SQLITE_BLOB,     /* 0x00 (not possible) */
     SQLITE_NULL,     /* 0x01 NULL */
     SQLITE_TEXT,     /* 0x02 TEXT */
     SQLITE_NULL,     /* 0x03 (not possible) */
     SQLITE_INTEGER,  /* 0x04 INTEGER */
     SQLITE_NULL,     /* 0x05 (not possible) */
     SQLITE_INTEGER,  /* 0x06 INTEGER + TEXT */
     SQLITE_NULL,     /* 0x07 (not possible) */
     SQLITE_FLOAT,    /* 0x08 FLOAT */
     SQLITE_NULL,     /* 0x09 (not possible) */
     SQLITE_FLOAT,    /* 0x0a FLOAT + TEXT */
     SQLITE_NULL,     /* 0x0b (not possible) */
     SQLITE_INTEGER,  /* 0x0c (not possible) */
     SQLITE_NULL,     /* 0x0d (not possible) */
     SQLITE_INTEGER,  /* 0x0e (not possible) */
     SQLITE_NULL,     /* 0x0f (not possible) */
     SQLITE_BLOB,     /* 0x10 BLOB */
     SQLITE_NULL,     /* 0x11 (not possible) */
     SQLITE_TEXT,     /* 0x12 (not possible) */
     SQLITE_NULL,     /* 0x13 (not possible) */
     SQLITE_INTEGER,  /* 0x14 INTEGER + BLOB */
     SQLITE_NULL,     /* 0x15 (not possible) */
     SQLITE_INTEGER,  /* 0x16 (not possible) */
     SQLITE_NULL,     /* 0x17 (not possible) */
     SQLITE_FLOAT,    /* 0x18 FLOAT + BLOB */
     SQLITE_NULL,     /* 0x19 (not possible) */
     SQLITE_FLOAT,    /* 0x1a (not possible) */
     SQLITE_NULL,     /* 0x1b (not possible) */
     SQLITE_INTEGER,  /* 0x1c (not possible) */
     SQLITE_NULL,     /* 0x1d (not possible) */
     SQLITE_INTEGER,  /* 0x1e (not possible) */
     SQLITE_NULL,     /* 0x1f (not possible) */
     SQLITE_FLOAT,    /* 0x20 INTREAL */
     SQLITE_NULL,     /* 0x21 (not possible) */
     SQLITE_FLOAT,    /* 0x22 INTREAL + TEXT */
     SQLITE_NULL,     /* 0x23 (not possible) */
     SQLITE_FLOAT,    /* 0x24 (not possible) */
     SQLITE_NULL,     /* 0x25 (not possible) */
     SQLITE_FLOAT,    /* 0x26 (not possible) */
     SQLITE_NULL,     /* 0x27 (not possible) */
     SQLITE_FLOAT,    /* 0x28 (not possible) */
     SQLITE_NULL,     /* 0x29 (not possible) */
     SQLITE_FLOAT,    /* 0x2a (not possible) */
     SQLITE_NULL,     /* 0x2b (not possible) */
     SQLITE_FLOAT,    /* 0x2c (not possible) */
     SQLITE_NULL,     /* 0x2d (not possible) */
     SQLITE_FLOAT,    /* 0x2e (not possible) */
     SQLITE_NULL,     /* 0x2f (not possible) */
     SQLITE_BLOB,     /* 0x30 (not possible) */
     SQLITE_NULL,     /* 0x31 (not possible) */
     SQLITE_TEXT,     /* 0x32 (not possible) */
     SQLITE_NULL,     /* 0x33 (not possible) */
     SQLITE_FLOAT,    /* 0x34 (not possible) */
     SQLITE_NULL,     /* 0x35 (not possible) */
     SQLITE_FLOAT,    /* 0x36 (not possible) */
     SQLITE_NULL,     /* 0x37 (not possible) */
     SQLITE_FLOAT,    /* 0x38 (not possible) */
     SQLITE_NULL,     /* 0x39 (not possible) */
     SQLITE_FLOAT,    /* 0x3a (not possible) */
     SQLITE_NULL,     /* 0x3b (not possible) */
     SQLITE_FLOAT,    /* 0x3c (not possible) */
     SQLITE_NULL,     /* 0x3d (not possible) */
     SQLITE_FLOAT,    /* 0x3e (not possible) */
     SQLITE_NULL,     /* 0x3f (not possible) */
  };
#ifdef SQLITE_DEBUG
  {
    صحیح eType = SQLITE_BLOB;
    اگر( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }وگرنه اگر( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }وگرنه اگر( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }وگرنه اگر( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    أكد( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  بازگشت aType[pVal->flags&MEM_AffMask];
}
صحیح sqlite3_مقدار_encoding(sqlite3_مقدار *pVal){
  بازگشت pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
صحیح sqlite3_مقدار_nochange(sqlite3_مقدار *pVal){
  بازگشت (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
صحیح sqlite3_مقدار_frombind(sqlite3_مقدار *pVal){
  بازگشت (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_مقدار *sqlite3_مقدار_dup(ثابت sqlite3_مقدار *pOrig){
  sqlite3_مقدار *pNew;
  اگر( pOrig==0 ) بازگشت 0;
  pNew = sqlite3_malloc( اندازه(*pNew) );
  اگر( pNew==0 ) بازگشت 0;
  memset(pNew, 0, اندازه(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  اگر( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    اگر( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_مقدار_Free(pNew);
      pNew = 0;
    }
  }وگرنه اگر( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  بازگشت pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
تهی sqlite3_مقدار_free(sqlite3_مقدار *pOld){
  sqlite3_مقدار_Free(pOld);
}
 

/**************************** sqlite3_result_  *******************************
** The following routines are used by application-defined SQL functions to
** specify the function return value.  There are many variations on
** sqlite3_result_xxxx() for different types of return values.
**
** The setStrOrError() function is a helper function that invokes
** sqlite3VdbeMemSetStr() to store the result as a string or blob.
** Appropriate errors are set if the string/blob is too big or if
** an OOM occurs.
**
** The invokeValueDestructor(P,X) helper function invokes the destructor
** function X() on value P if P is not going to be used and need to
** be destroyed.
*/
ایستا تهی setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  ثابت نویسه *z,          /* String pointer */
  صحیح n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  تهی (*xDel)(تهی*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  صحیح rc;
  اگر( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }وگرنه اگر( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    أكد( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }وگرنه{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  اگر( rc ){
    اگر( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }وگرنه{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      أكد( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    بازگشت;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  اگر( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
ایستا صحیح invoke_مقدار_Destructor(
  ثابت تهی *p,             /* Value to destroy */
  تهی (*xDel)(تهی*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  أكد( xDel!=SQLITE_DYNAMIC );
  اگر( xDel==0 ){
    /* noop */
  }وگرنه اگر( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }وگرنه{
    xDel((تهی*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#وگرنه
  أكد( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  بازگشت SQLITE_TOOBIG;
}
تهی sqlite3_result_blob(
  sqlite3_context *pCtx,
  ثابت تهی *z,
  صحیح n,
  تهی (*xDel)(تهی *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 || n<0 ){
    invoke_مقدار_Destructor(z, xDel, pCtx);
    بازگشت;
  }
#endif
  أكد( n>=0 );
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
تهی sqlite3_result_blob64(
  sqlite3_context *pCtx,
  ثابت تهی *z,
  sqlite3_uint64 n,
  تهی (*xDel)(تهی *)
){
  أكد( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_مقدار_Destructor(z, xDel, 0);
    بازگشت;
  }
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  اگر( n>0x7fffffff ){
    (تهی)invoke_مقدار_Destructor(z, xDel, pCtx);
  }وگرنه{
    setResultStrOrError(pCtx, z, (صحیح)n, 0, xDel);
  }
}
تهی sqlite3_result_double(sqlite3_context *pCtx, دوگانه rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
تهی sqlite3_result_error(sqlite3_context *pCtx, ثابت نویسه *z, صحیح n){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
تهی sqlite3_result_error16(sqlite3_context *pCtx, ثابت تهی *z, صحیح n){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
تهی sqlite3_result_int(sqlite3_context *pCtx, صحیح iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
تهی sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
تهی sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
تهی sqlite3_result_pointer(
  sqlite3_context *pCtx,
  تهی *pPtr,
  ثابت نویسه *zPType,
  تهی (*xDestructor)(تهی*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_مقدار_Destructor(pPtr, xDestructor, 0);
    بازگشت;
  }
#endif
  pOut = pCtx->pOut;
  أكد( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
تهی sqlite3_result_subtype(sqlite3_context *pCtx, unsigned صحیح eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
#اگر defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  اگر( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    نویسه zErr[200];
    sqlite3_snprintf(اندازه(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    بازگشت;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  أكد( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
تهی sqlite3_result_text(
  sqlite3_context *pCtx,
  ثابت نویسه *z,
  صحیح n,
  تهی (*xDel)(تهی *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_مقدار_Destructor(z, xDel, 0);
    بازگشت;
  }
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
تهی sqlite3_result_text64(
  sqlite3_context *pCtx,
  ثابت نویسه *z,
  sqlite3_uint64 n,
  تهی (*xDel)(تهی *),
  unsigned نویسه enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_مقدار_Destructor(z, xDel, 0);
    بازگشت;
  }
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  أكد( xDel!=SQLITE_DYNAMIC );
  اگر( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    اگر( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  اگر( n>0x7fffffff ){
    (تهی)invoke_مقدار_Destructor(z, xDel, pCtx);
  }وگرنه{
    setResultStrOrError(pCtx, z, (صحیح)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
تهی sqlite3_result_text16(
  sqlite3_context *pCtx,
  ثابت تهی *z,
  صحیح n,
  تهی (*xDel)(تهی *)
){
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
تهی sqlite3_result_text16be(
  sqlite3_context *pCtx,
  ثابت تهی *z,
  صحیح n,
  تهی (*xDel)(تهی *)
){
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
تهی sqlite3_result_text16le(
  sqlite3_context *pCtx,
  ثابت تهی *z,
  صحیح n,
  تهی (*xDel)(تهی *)
){
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
تهی sqlite3_result_مقدار(sqlite3_context *pCtx, sqlite3_مقدار *p_مقدار){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
  اگر( p_مقدار==0 ){
    sqlite3_result_null(pCtx);
    بازگشت;
  }
#endif
  pOut = pCtx->pOut;
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_مقدار);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  اگر( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
تهی sqlite3_result_zeroblob(sqlite3_context *pCtx, صحیح n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
صحیح sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  أكد( sqlite3_mutex_held(pOut->db->mutex) );
  اگر( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    بازگشت SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (صحیح)n);
  بازگشت SQLITE_OK;
#وگرنه
  بازگشت sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (صحیح)n);
#endif
}
تهی sqlite3_result_error_code(sqlite3_context *pCtx, صحیح errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  اگر( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  اگر( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
تهی sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
تهی sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->pOut->db);
}

/* Make the return value of the SQL function or virtual table pCtx
** be the content of the sqlite3_str object pStr.  The eOwn flag
** determines ownership of the sqlite3_str object and its content.
**
**    eOwn             Ownership transfer
**    -------------    ------------------------------------------------
**
**    SQLITE_COPY      The SQL function returns a copy the sqlite3_str
**                     content and leaves the sqlite3_str object itself
**                     unchanged.
**
**    SQLITE_XFER      The content of the sqlite3_str is transferred to
**                     the SQL function and the SQL function takes
**                     responsibility for freeing that content when it is
**                     no longer needed.  The sqlite3_str object is reset
**                     to an empty string.
**
**    SQLITE_FINISH    Like SQLITE_XFER except that the pStr is also
**                     freed using sqlite3_str_free().
*/
تهی sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, صحیح eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
  اگر( pStr==0 ) بازگشت;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  اگر( pStr->accError==0 ){
    اگر( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }وگرنه{
      ثابت نویسه *zText = sqlite3_str_مقدار(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      أكد( isMalloced(pStr) );
      اگر( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }وگرنه{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }وگرنه اگر( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }وگرنه{
    أكد( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  اگر( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
تهی sqlite3ResultIntReal(sqlite3_context *pCtx){
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  اگر( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ایستا صحیح doWalCallbacks(sqlite3 *db){
  صحیح rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  صحیح i;
  برای(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt ){
      صحیح nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      اگر( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#وگرنه
  UNUSED_PARAMETER(db);
#endif
  بازگشت rc;
}


/*
** Execute the statement pStmt, either until a row of data is ready, the
** statement is completely executed or an error occurs.
**
** This routine implements the bulk of the logic behind the sqlite_step()
** API.  The only thing omitted is the automatic recompile if a
** schema change has occurred.  That detail is handled by the
** outer sqlite3_step() wrapper procedure.
*/
ایستا صحیح sqlite3Step(Vdbe *p){
  sqlite3 *db;
  صحیح rc;

  أكد(p);
  db = p->db;
  اگر( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    اگر( p->eVdbeState==VDBE_READY_STATE ){
      اگر( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        اگر( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        برو end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      اگر( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      أكد( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      اگر( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }وگرنه{
        أكد( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      اگر( p->readOnly==0 ) db->nVdbeWrite++;
      اگر( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }وگرنه

    اگر( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
      /* We used to require that sqlite3_reset() be called before retrying
      ** sqlite3_step() after any error or after SQLITE_DONE.  But beginning
      ** with version 3.7.0, we changed this so that sqlite3_reset() would
      ** be called automatically instead of throwing the SQLITE_MISUSE error.
      ** This "automatic-reset" change is not technically an incompatibility,
      ** since any application that receives an SQLITE_MISUSE is broken by
      ** definition.
      **
      ** Nevertheless, some published applications that were originally written
      ** for version 3.6.23 or earlier do in fact depend on SQLITE_MISUSE
      ** returns, and those were broken by the automatic-reset change.  As a
      ** a work-around, the SQLITE_OMIT_AUTORESET compile-time restores the
      ** legacy behavior of returning SQLITE_MISUSE for cases where the
      ** previous sqlite3_step() returned something other than a SQLITE_LOCKED
      ** or SQLITE_BUSY error.
      */
#ifdef SQLITE_OMIT_AUTORESET
      اگر( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }وگرنه{
        بازگشت SQLITE_MISUSE_BKPT;
      }
#وگرنه
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      أكد( p->eVdbeState==VDBE_READY_STATE );
      برو restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  اگر( p->explain ){
    rc = sqlite3VdbeList(p);
  }وگرنه
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  اگر( rc==SQLITE_ROW ){
    أكد( p->rc==SQLITE_OK );
    أكد( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    بازگشت SQLITE_ROW;
  }وگرنه{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    اگر( rc==SQLITE_DONE && db->autoCommit ){
      أكد( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      اگر( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }وگرنه اگر( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  اگر( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    اگر( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  بازگشت (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
صحیح sqlite3_step(sqlite3_stmt *pStmt){
  صحیح rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  صحیح cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  اگر( vdbeSafetyNotNull(v) ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  وقتی( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    صحیح savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    اگر( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ثابت نویسه *zErr = (ثابت نویسه *)sqlite3_مقدار_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_پیام);
      اگر( !db->mallocFailed ){
        v->z_Err_پیام = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } وگرنه {
        v->z_Err_پیام = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      بشکن;
    }
    sqlite3_reset(pStmt);
    اگر( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    أكد( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  بازگشت rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
تهی *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) بازگشت 0;
#endif
  أكد( p && p->pFunc );
  بازگشت p->pFunc->pUserData;
}

/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
**
** IMPLEMENTATION-OF: R-46798-50301 The sqlite3_context_db_handle() interface
** returns a copy of the pointer to the database connection (the 1st
** parameter) of the sqlite3_create_function() and
** sqlite3_create_function16() routines that originally registered the
** application defined function.
*/
sqlite3 *sqlite3_context_db_handle(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) بازگشت 0;
#وگرنه
  أكد( p && p->pOut );
#endif
  بازگشت p->pOut->db;
}

/*
** If this routine is invoked from within an xColumn method of a virtual
** table, then it returns true if and only if the the call is during an
** UPDATE operation and the value of the column will not be modified
** by the UPDATE.
**
** If this routine is called from any context other than within the
** xColumn method of a virtual table, then the return value is meaningless
** and arbitrary.
**
** Virtual table implements might use this routine to optimize their
** performance by substituting a NULL result, or some other light-weight
** value, as a signal to the xUpdate routine that the column is unchanged.
*/
صحیح sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) بازگشت 0;
#وگرنه
  أكد( p );
#endif
  بازگشت sqlite3_مقدار_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
تهی sqlite3_Vdbe_مقدار_List_Free(تهی *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ایستا صحیح مقدار_From_مقدار_List(
  sqlite3_مقدار *pVal,        /* Pointer to the ValueList object */
  sqlite3_مقدار **ppOut,      /* Store the next value from the list here */
  صحیح bNext                   /* 1 for _next(). 0 for _first() */
){
  صحیح rc;
  مقدار_List *pRhs;

  *ppOut = 0;
  اگر( pVal==0 ) بازگشت SQLITE_MISUSE_BKPT;
  اگر( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_مقدار_List_Free ){
    بازگشت SQLITE_ERROR;
  }وگرنه{
    أكد( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    أكد( pVal->eSubtype=='p' );
    أكد( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (مقدار_List*)pVal->z;
  }
  اگر( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }وگرنه{
    صحیح dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    أكد( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    اگر( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  اگر( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, اندازه(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    اگر( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_مقدار *pOut = pRhs->pOut;
      صحیح iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      اگر( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }وگرنه{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  بازگشت rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
صحیح sqlite3_vtab_in_first(sqlite3_مقدار *pVal, sqlite3_مقدار **ppOut){
  بازگشت مقدار_From_مقدار_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
صحیح sqlite3_vtab_in_next(sqlite3_مقدار *pVal, sqlite3_مقدار **ppOut){
  بازگشت مقدار_From_مقدار_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  صحیح rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  أكد( p->pVdbe!=0 );
#وگرنه
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  اگر( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    اگر( rc ) *piTime = 0;
  }
  بازگشت *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ایستا SQLITE_NOINLINE تهی *createAggContext(sqlite3_context *p, صحیح nByte){
  Mem *pMem = p->pMem;
  أكد( (pMem->flags & MEM_Agg)==0 );
  اگر( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }وگرنه{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    اگر( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  بازگشت (تهی*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
تهی *sqlite3_aggregate_context(sqlite3_context *p, صحیح nByte){
  أكد( p && p->pFunc && p->pFunc->xFinalize );
  أكد( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  اگر( (p->pMem->flags & MEM_Agg)==0 ){
    بازگشت createAggContext(p, nByte);
  }وگرنه{
    بازگشت (تهی*)p->pMem->z;
  }
}

/*
** Return the auxiliary data pointer, if any, for the iArg'th argument to
** the user-function defined by pCtx.
**
** The left-most argument is 0.
**
** Undocumented behavior:  If iArg is negative then access a cache of
** auxiliary data pointers that is available to all functions within a
** single prepared statement.  The iArg values must match.
*/
تهی *sqlite3_get_auxdata(sqlite3_context *pCtx, صحیح iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت 0;
#endif
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#اگر SQLITE_ENABLE_STAT4
  اگر( pCtx->pVdbe==0 ) بازگشت 0;
#وگرنه
  أكد( pCtx->pVdbe!=0 );
#endif
  برای(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    اگر(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      بازگشت pAuxData->pAux;
    }
  }
  بازگشت 0;
}

/*
** Set the auxiliary data pointer and delete function, for the iArg'th
** argument to the user-function defined by pCtx. Any previous value is
** deleted by calling the delete function specified when it was set.
**
** The left-most argument is 0.
**
** Undocumented behavior:  If iArg is negative then make the data available
** to all functions within the current prepared statement using iArg as an
** access code.
*/
تهی sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  صحیح iArg,
  تهی *pAux,
  تهی (*xDelete)(تهی*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) بازگشت;
#endif
  pVdbe= pCtx->pVdbe;
  أكد( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  اگر( pVdbe==0 ) برو failed;
#وگرنه
  أكد( pVdbe!=0 );
#endif

  برای(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    اگر( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      بشکن;
    }
  }
  اگر( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, اندازه(AuxData));
    اگر( !pAuxData ) برو failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    اگر( pCtx->isError==0 ) pCtx->isError = -1;
  }وگرنه اگر( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  بازگشت;

failed:
  اگر( xDelete ){
    xDelete(pAux);
  }
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
صحیح sqlite3_aggregate_شمار(sqlite3_context *p){
  أكد( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  بازگشت p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
صحیح sqlite3_column_شمار(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  اگر( pVm==0 ) بازگشت 0;
  بازگشت pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
صحیح sqlite3_data_شمار(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  اگر( pVm==0 || pVm->pResultRow==0 ) بازگشت 0;
  بازگشت pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ایستا ثابت Mem *column_Null_مقدار(تهی){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ایستا ثابت Mem nullMem
#اگر defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (نویسه*)0,
        /* .n          = */ (صحیح)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (صحیح)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (نویسه*)0,
        /* .xDel       = */ (تهی(*)(تهی*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  بازگشت &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ایستا Mem *columnMem(sqlite3_stmt *pStmt, صحیح i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  اگر( pVm==0 ) بازگشت (Mem*)column_Null_مقدار();
  أكد( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  اگر( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }وگرنه{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_مقدار();
  }
  بازگشت pOut;
}

/*
** This function is called after invoking an sqlite3_value_XXX function on a
** column value (i.e. a value returned by evaluating an SQL expression in the
** select list of a SELECT statement) that may cause a malloc() failure. If
** malloc() has failed, the threads mallocFailed flag is cleared and the result
** code of statement pStmt set to SQLITE_NOMEM.
**
** Specifically, this is called from within:
**
**     sqlite3_column_int()
**     sqlite3_column_int64()
**     sqlite3_column_text()
**     sqlite3_column_text16()
**     sqlite3_column_double()
**     sqlite3_column_bytes()
**     sqlite3_column_bytes16()
**     sqlite3_column_blob()
*/
ایستا تهی columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  اگر( p ){
    أكد( p->db!=0 );
    أكد( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
ثابت تهی *sqlite3_column_blob(sqlite3_stmt *pStmt, صحیح i){
  ثابت تهی *val;
  val = sqlite3_مقدار_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  بازگشت val;
}
صحیح sqlite3_column_bytes(sqlite3_stmt *pStmt, صحیح i){
  صحیح val = sqlite3_مقدار_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
صحیح sqlite3_column_bytes16(sqlite3_stmt *pStmt, صحیح i){
  صحیح val = sqlite3_مقدار_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
دوگانه sqlite3_column_double(sqlite3_stmt *pStmt, صحیح i){
  دوگانه val = sqlite3_مقدار_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
صحیح sqlite3_column_int(sqlite3_stmt *pStmt, صحیح i){
  صحیح val = sqlite3_مقدار_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, صحیح i){
  sqlite_int64 val = sqlite3_مقدار_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
ثابت unsigned نویسه *sqlite3_column_text(sqlite3_stmt *pStmt, صحیح i){
  ثابت unsigned نویسه *val = sqlite3_مقدار_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
sqlite3_مقدار *sqlite3_column_مقدار(sqlite3_stmt *pStmt, صحیح i){
  Mem *pOut = columnMem(pStmt, i);
  اگر( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  بازگشت (sqlite3_مقدار *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_column_text16(sqlite3_stmt *pStmt, صحیح i){
  ثابت تهی *val = sqlite3_مقدار_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت val;
}
#endif /* SQLITE_OMIT_UTF16 */
صحیح sqlite3_column_type(sqlite3_stmt *pStmt, صحیح i){
  صحیح iType = sqlite3_مقدار_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  بازگشت iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ایستا ثابت نویسه * ثابت azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ایستا ثابت u16 azExplainColNames16data[] = {
  /*   0 */  'a', 'd', 'd', 'r',                0,
  /*   5 */  'o', 'p', 'c', 'o', 'd', 'e',      0,
  /*  12 */  'p', '1',                          0, 
  /*  15 */  'p', '2',                          0,
  /*  18 */  'p', '3',                          0,
  /*  21 */  'p', '4',                          0,
  /*  24 */  'p', '5',                          0,
  /*  27 */  'c', 'o', 'm', 'm', 'e', 'n', 't', 0,
  /*  35 */  'i', 'd',                          0,
  /*  38 */  'p', 'a', 'r', 'e', 'n', 't',      0,
  /*  45 */  'n', 'o', 't', 'u', 's', 'e', 'd', 0,
  /*  53 */  'd', 'e', 't', 'a', 'i', 'l',      0
};
ایستا ثابت u8 iExplainColNames16[] = {
  0, 5, 12, 15, 18, 21, 24, 27,
  35, 38, 45, 53
};

/*
** Convert the N-th element of pStmt->pColName[] into a string using
** xFunc() then return that string.  If N is out of range, return 0.
**
** There are up to 5 names for each column.  useType determines which
** name is returned.  Here are the names:
**
**    0      The column name as it should be displayed for output
**    1      The datatype name for the column
**    2      The name of the database that the column derives from
**    3      The name of the table that the column derives from
**    4      The name of the table column that the result column derives from
**
** If the result is not a simple column reference (if it is an expression
** or a constant) then useTypes 2, 3, and 4 return NULL.
*/
ایستا ثابت تهی *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  صحیح N,                   /* Which column to get the name for */
  صحیح useUtf16,            /* True to return the name as UTF16 */
  صحیح useType              /* What type of name */
){
  ثابت تهی *ret;
  Vdbe *p;
  صحیح n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pStmt==0 ){
    (تهی)SQLITE_MISUSE_BKPT;
    بازگشت 0;
  }
#endif
  اگر( N<0 ) بازگشت 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  أكد( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  اگر( p->explain ){
    اگر( useType>0 ) برو columnName_end;
    n = p->explain==1 ? 8 : 4;
    اگر( N>=n ) برو columnName_end;
    اگر( useUtf16 ){
      صحیح i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (تهی*)&azExplainColNames16data[i];
    }وگرنه{
      ret = (تهی*)azExplainColNames8[N + 8*p->explain - 8];
    }
    برو columnName_end;
  }
  n = p->nResColumn;
  اگر( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    اگر( useUtf16 ){
      ret = sqlite3_مقدار_text16((sqlite3_مقدار*)&p->aColName[N]);
    }وگرنه
#endif
    {
      ret = sqlite3_مقدار_text((sqlite3_مقدار*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    أكد( db->mallocFailed==0 || db->mallocFailed==1 );
    اگر( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  بازگشت ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ثابت نویسه *sqlite3_column_name(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_column_name16(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#اگر defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ثابت نویسه *sqlite3_column_decltype(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_column_decltype16(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت نویسه *sqlite3_column_database_name(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_column_database_name16(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت نویسه *sqlite3_column_table_name(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_column_table_name16(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت نویسه *sqlite3_column_origin_name(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
ثابت تهی *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, صحیح N){
  بازگشت columnName(pStmt, N, 1, COLNAME_COLUMN);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_ENABLE_COLUMN_METADATA */


/******************************* sqlite3_bind_  ***************************
**
** Routines used to attach values to wildcards in a compiled SQL statement.
*/
/*
** Unbind the value bound to variable i in virtual machine p. This is the
** the same as binding a NULL value to the column. If the "i" parameter is
** out of range, then SQLITE_RANGE is returned. Otherwise SQLITE_OK.
**
** A successful evaluation of this routine acquires the mutex on p.
** the mutex is released if any kind of error occurs.
**
** The error code stored in database p->db is overwritten with the return
** value in any case.
**
** (tag-20240917-01) If  vdbeUnbind(p,(u32)(i-1))  returns SQLITE_OK,
** that means all of the the following will be true:
**
**     p!=0
**     p->pVar!=0
**     i>0
**     i<=p->nVar
**
** An assert() is normally added after vdbeUnbind() to help static analyzers
** realize this.
*/
ایستا صحیح vdbeUnbind(Vdbe *p, unsigned صحیح i){
  Mem *pVar;
  اگر( vdbeSafetyNotNull(p) ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  اگر( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    بازگشت SQLITE_MISUSE_BKPT;
  }
  اگر( i>=(unsigned صحیح)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    بازگشت SQLITE_RANGE;
  }

  pVar = &p->aVar[i];

  /* If the bit corresponding to this variable in Vdbe.expmask is set, then
  ** binding a new value to this variable invalidates the current query plan.
  **
  ** IMPLEMENTATION-OF: R-57496-20354 If the specific value bound to a host
  ** parameter in the WHERE clause might influence the choice of query plan
  ** for a statement, then the statement will be automatically recompiled,
  ** as if there had been a schema change, on the first sqlite3_step() call
  ** following any change to the bindings of that parameter.
  */
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  أكد( (p->expmask & p->smimask)==p->smimask );
  اگر( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    اگر( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  بازگشت SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ایستا صحیح bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  صحیح i,                 /* Index of the parameter to bind */
  ثابت تهی *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  تهی (*xDel)(تهی*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  صحیح rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    اگر( zData!=0 ){
      pVar = &p->aVar[i-1];
      اگر( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }وگرنه اگر( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        أكد( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }وگرنه{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        اگر( encoding==0 ) pVar->enc = ENC(p->db);
      }
      اگر( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      اگر( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }وگرنه اگر( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((تهی*)zData);
  }
  بازگشت rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
صحیح sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت تهی *zData,
  صحیح nData,
  تهی (*xDel)(تهی*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( nData<0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  بازگشت bindText(pStmt, i, zData, nData, xDel, 0);
}
صحیح sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت تهی *zData,
  sqlite3_uint64 nData,
  تهی (*xDel)(تهی*)
){
  أكد( xDel!=SQLITE_DYNAMIC );
  بازگشت bindText(pStmt, i, zData, nData, xDel, 0);
}
صحیح sqlite3_bind_double(sqlite3_stmt *pStmt, صحیح i, دوگانه r_مقدار){
  صحیح rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_مقدار);
    sqlite3_mutex_leave(p->db->mutex);
  }
  بازگشت rc;
}
صحیح sqlite3_bind_int(sqlite3_stmt *p, صحیح i, صحیح i_مقدار){
  بازگشت sqlite3_bind_int64(p, i, (i64)i_مقدار);
}
صحیح sqlite3_bind_int64(sqlite3_stmt *pStmt, صحیح i, sqlite_int64 i_مقدار){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  اگر( vdbeSafetyNotNull(p) ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  اگر( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    بازگشت SQLITE_MISUSE_BKPT;
  }
  اگر( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    بازگشت SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  اگر( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    أكد( (p->expmask & p->smimask)==p->smimask );
    اگر( (p->expmask & expireMask)!=0 ){
      اگر( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        صحیح sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        صحیح sm2 = i_مقدار>=0 && i_مقدار<=1;
        اگر( sm1!=sm2 ){
          p->expired = 1;
        }
      }وگرنه اگر( (pVar->flags & MEM_Int)==0 || i_مقدار!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_مقدار);
  sqlite3_mutex_leave(p->db->mutex);
  بازگشت SQLITE_OK;
}
صحیح sqlite3_bind_null(sqlite3_stmt *pStmt, صحیح i){
  صحیح rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  بازگشت rc;
}
صحیح sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  صحیح i,
  تهی *pPtr,
  ثابت نویسه *zPTtype,
  تهی (*xDestructor)(تهی*)
){
  صحیح rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }وگرنه اگر( xDestructor ){
    xDestructor(pPtr);
  }
  بازگشت rc;
}
صحیح sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت نویسه *zData,
  صحیح nData,
  تهی (*xDel)(تهی*)
){
  بازگشت bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
صحیح sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت نویسه *zData,
  sqlite3_uint64 nData,
  تهی (*xDel)(تهی*),
  unsigned نویسه enc
){
  أكد( xDel!=SQLITE_DYNAMIC );
  اگر( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    اگر( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  بازگشت bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
صحیح sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت تهی *zData,
  صحیح n,
  تهی (*xDel)(تهی*)
){
  بازگشت bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
صحیح sqlite3_bind_مقدار(sqlite3_stmt *pStmt, صحیح i, ثابت sqlite3_مقدار *p_مقدار){
  صحیح rc;
  انتخاب( sqlite3_مقدار_type((sqlite3_مقدار*)p_مقدار) ){
    حالت SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_مقدار->u.i);
      بشکن;
    }
    حالت SQLITE_FLOAT: {
      أكد( p_مقدار->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_مقدار->flags & MEM_Real) ? p_مقدار->u.r : (دوگانه)p_مقدار->u.i
      );
      بشکن;
    }
    حالت SQLITE_BLOB: {
      اگر( p_مقدار->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_مقدار->u.nZero);
      }وگرنه{
        rc = sqlite3_bind_blob(pStmt, i, p_مقدار->z, p_مقدار->n,SQLITE_TRANSIENT);
      }
      بشکن;
    }
    حالت SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_مقدار->z, p_مقدار->n, SQLITE_TRANSIENT,
                              p_مقدار->enc);
      بشکن;
    }
    پیشفرض: {
      rc = sqlite3_bind_null(pStmt, i);
      بشکن;
    }
  }
  بازگشت rc;
}
صحیح sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, صحیح i, صحیح n){
  صحیح rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#وگرنه
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  بازگشت rc;
}
صحیح sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, صحیح i, sqlite3_uint64 n){
  صحیح rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  اگر( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }وگرنه{
    أكد( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  بازگشت rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
صحیح sqlite3_bind_parameter_شمار(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  بازگشت p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ثابت نویسه *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, صحیح i){
  Vdbe *p = (Vdbe*)pStmt;
  اگر( p==0 ) بازگشت 0;
  بازگشت sqlite3_VList_رقم_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
صحیح sqlite3VdbeParameterIndex(Vdbe *p, ثابت نویسه *zName, صحیح nName){
  اگر( p==0 || zName==0 ) بازگشت 0;
  بازگشت sqlite3_VList_Name_To_رقم(p->pVList, zName, nName);
}
صحیح sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, ثابت نویسه *zName){
  بازگشت sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
صحیح sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  صحیح i;
  أكد( pTo->db==pFrom->db );
  أكد( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  برای(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  بازگشت SQLITE_OK;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  Internal/core SQLite code
** should call sqlite3TransferBindings.
**
** It is misuse to call this routine with statements from different
** database connections.  But as this is a deprecated interface, we
** will not bother to check for that condition.
**
** If the two statements contain a different number of bindings, then
** an SQLITE_ERROR is returned.  Nothing else can go wrong, so otherwise
** SQLITE_OK is returned.
*/
صحیح sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  اگر( pFrom->nVar!=pTo->nVar ){
    بازگشت SQLITE_ERROR;
  }
  أكد( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  اگر( pTo->expmask ){
    pTo->expired = 1;
  }
  أكد( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  اگر( pFrom->expmask ){
    pFrom->expired = 1;
  }
  بازگشت sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  بازگشت pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
صحیح sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  بازگشت pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
صحیح sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  بازگشت pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
صحیح sqlite3_stmt_explain(sqlite3_stmt *pStmt, صحیح eMode){
  Vdbe *v = (Vdbe*)pStmt;
  صحیح rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pStmt==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  اگر( ((صحیح)v->explain)==eMode ){
    rc = SQLITE_OK;
  }وگرنه اگر( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }وگرنه اگر( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }وگرنه اگر( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }وگرنه اگر( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }وگرنه{
    صحیح savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    اگر( rc ){
      v->explain = savedMode;
    }وگرنه{
      v->haveEqpOps = eMode==2;
    }
  }
  اگر( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }وگرنه{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  بازگشت rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
صحیح sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  بازگشت v!=0 && v->eVdbeState==VDBE_RUN_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(pDb) ){
    (تهی)SQLITE_MISUSE_BKPT;
    بازگشت 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  اگر( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }وگرنه{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  بازگشت pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
صحیح sqlite3_stmt_status(sqlite3_stmt *pStmt, صحیح op, صحیح resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (تهی)SQLITE_MISUSE_BKPT;
    بازگشت 0;
  }
#endif
  اگر( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (صحیح*)&v;
    أكد( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }وگرنه{
    v = pVdbe->aCounter[op];
    اگر( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  بازگشت (صحیح)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ثابت نویسه *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  بازگشت p ? p->zSql : 0;
}

/*
** Return the SQL associated with a prepared statement with
** bound parameters expanded.  Space to hold the returned string is
** obtained from sqlite3_malloc().  The caller is responsible for
** freeing the returned string by passing it to sqlite3_free().
**
** The SQLITE_TRACE_SIZE_LIMIT puts an upper bound on the size of
** expanded bound parameters.
*/
نویسه *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  بازگشت 0;
#وگرنه
  نویسه *z = 0;
  ثابت نویسه *zSql = sqlite3_sql(pStmt);
  اگر( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  بازگشت z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ثابت نویسه *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  اگر( p==0 ) بازگشت 0;
  اگر( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  بازگشت p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ایستا UnpackedRecord *vdbeUnpackRecord(
  کلید_Info *p_کلید_Info,
  صحیح n_کلید,
  ثابت تهی *p_کلید
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_کلید_Info);
  اگر( pRet ){
    memset(pRet->aMem, 0, اندازه(Mem)*(p_کلید_Info->n_کلید_Field+1));
    sqlite3VdbeRecordUnpack(n_کلید, p_کلید, pRet);
  }
  بازگشت pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
صحیح sqlite3_preupdate_old(sqlite3 *db, صحیح iIdx, sqlite3_مقدار **pp_مقدار){
  PreUpdate *p;
  Mem *pMem;
  صحیح rc = SQLITE_OK;
  صحیح iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( db==0 || pp_مقدار==0 ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  اگر( !p || p->op==SQLITE_درج ){
    rc = SQLITE_MISUSE_BKPT;
    برو preupdate_old_out;
  }
  اگر( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }وگرنه اگر( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    برو preupdate_old_out;
  }وگرنه{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  اگر( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    برو preupdate_old_out;
  }

  اگر( iIdx==p->pTab->iPKey ){
    *pp_مقدار = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }وگرنه{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    اگر( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      أكد( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      اگر( !aRec ) برو preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      اگر( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        اگر( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      اگر( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        برو preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_مقدار = &p->pUnpacked->aMem[iStore];
    اگر( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      اگر( pCol->iDflt>0 ){
        اگر( p->apDflt==0 ){
          صحیح nByte;
          أكد( اندازه(sqlite3_مقدار*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = اندازه(sqlite3_مقدار*)*p->pTab->nCol;
          p->apDflt = (sqlite3_مقدار**)sqlite3DbMallocZero(db, nByte);
          اگر( p->apDflt==0 ) برو preupdate_old_out;
        }
        اگر( p->apDflt[iIdx]==0 ){
          sqlite3_مقدار *pVal = 0;
          عبارت *pDflt;
          أكد( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_عبارت;
          rc = sqlite3_مقدار_From_عبارت(db, pDflt, ENC(db), pCol->affinity, &pVal);
          اگر( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_مقدار = p->apDflt[iIdx];
      }وگرنه{
        *pp_مقدار = (sqlite3_مقدار *)column_Null_مقدار();
      }
    }وگرنه اگر( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      اگر( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  بازگشت sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
صحیح sqlite3_preupdate_شمار(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#وگرنه
  p = db->pPreUpdate;
#endif
  بازگشت (p ? p->pKeyinfo->n_کلید_Field : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only. It returns zero if the change that caused the callback was made
** immediately by a user SQL statement. Or, if the change was made by a
** trigger program, it returns the number of trigger programs currently
** on the stack (1 for a top-level trigger, 2 for a trigger fired by a
** top-level trigger etc.).
**
** For the purposes of the previous paragraph, a foreign key CASCADE, SET NULL
** or SET DEFAULT action is considered a trigger.
*/
صحیح sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#وگرنه
  p = db->pPreUpdate;
#endif
  بازگشت (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
صحیح sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#وگرنه
  p = db->pPreUpdate;
#endif
  بازگشت (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
صحیح sqlite3_preupdate_new(sqlite3 *db, صحیح iIdx, sqlite3_مقدار **pp_مقدار){
  PreUpdate *p;
  صحیح rc = SQLITE_OK;
  Mem *pMem;
  صحیح iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( db==0 || pp_مقدار==0 ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  اگر( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    برو preupdate_new_out;
  }
  اگر( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }وگرنه اگر( iIdx >= p->pTab->nCol ){
    بازگشت SQLITE_MISUSE_BKPT;
  }وگرنه{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  اگر( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    برو preupdate_new_out;
  }

  اگر( p->op==SQLITE_درج ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    اگر( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      اگر( rc!=SQLITE_OK ) برو preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      اگر( !pUnpack ){
        rc = SQLITE_NOMEM;
        برو preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    اگر( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }وگرنه اگر( iStore>=pUnpack->nField ){
      pMem = (sqlite3_مقدار *)column_Null_مقدار();
    }وگرنه اگر( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }وگرنه{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    أكد( p->op==SQLITE_UPDATE );
    اگر( !p->aNew ){
      أكد( اندازه(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, اندازه(Mem)*p->pCsr->nField);
      اگر( !p->aNew ){
        rc = SQLITE_NOMEM;
        برو preupdate_new_out;
      }
    }
    أكد( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    اگر( pMem->flags==0 ){
      اگر( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }وگرنه{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        اگر( rc!=SQLITE_OK ) برو preupdate_new_out;
      }
    }
  }
  *pp_مقدار = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  بازگشت sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
صحیح sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  صحیح iScan,                      /* Index of loop to report on */
  صحیح iScanStatusOp,              /* Which metric to return */
  صحیح flags,
  تهی *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  صحیح nOp;
  ScanStatus *pScan = 0;
  صحیح idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    بازگشت 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  اگر( p->pFrame ){
    VdbeFrame *pFrame;
    برای(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  اگر( iScan<0 ){
    صحیح ii;
    اگر( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      برای(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      بازگشت 0;
    }
    بازگشت 1;
  }
  اگر( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }وگرنه{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    برای(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      اگر( pScan->zName ){
        iScan--;
        اگر( iScan<0 ) بشکن;
      }
    }
  }
  اگر( idx>=p->nScan ) بازگشت 1;
  أكد( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  انتخاب( iScanStatusOp ){
    حالت SQLITE_SCANSTAT_NLOOP: {
      اگر( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }وگرنه{
        *(sqlite3_int64*)pOut = -1;
      }
      بشکن;
    }
    حالت SQLITE_SCANSTAT_NVISIT: {
      اگر( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }وگرنه{
        *(sqlite3_int64*)pOut = -1;
      }
      بشکن;
    }
    حالت SQLITE_SCANSTAT_EST: {
      دوگانه r = 1.0;
      LogEst x = pScan->nEst;
      وقتی( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(دوگانه*)pOut = r*sqlite3LogEstToInt(x);
      بشکن;
    }
    حالت SQLITE_SCANSTAT_NAME: {
      *(ثابت نویسه**)pOut = pScan->zName;
      بشکن;
    }
    حالت SQLITE_SCANSTAT_EXPLAIN: {
      اگر( pScan->addrExplain ){
        *(ثابت نویسه**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }وگرنه{
        *(ثابت نویسه**)pOut = 0;
      }
      بشکن;
    }
    حالت SQLITE_SCANSTAT_SELECTID: {
      اگر( pScan->addrExplain ){
        *(صحیح*)pOut = aOp[ pScan->addrExplain ].p1;
      }وگرنه{
        *(صحیح*)pOut = -1;
      }
      بشکن;
    }
    حالت SQLITE_SCANSTAT_PARENTID: {
      اگر( pScan->addrExplain ){
        *(صحیح*)pOut = aOp[ pScan->addrExplain ].p2;
      }وگرنه{
        *(صحیح*)pOut = -1;
      }
      بشکن;
    }
    حالت SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      اگر( pScan->aAddrRange[0]==0 ){
        res = -1;
      }وگرنه{
        صحیح ii;
        برای(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          صحیح iIns = pScan->aAddrRange[ii];
          صحیح iEnd = pScan->aAddrRange[ii+1];
          اگر( iIns==0 ) بشکن;
          اگر( iIns>0 ){
            وقتی( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }وگرنه{
            صحیح iOp;
            برای(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              اگر( pOp->p1!=iEnd ) ادامه;
              اگر( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                ادامه;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      بشکن;
    }
    پیشفرض: {
      بازگشت 1;
    }
  }
  بازگشت 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
صحیح sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  صحیح iScan,                      /* Index of loop to report on */
  صحیح iScanStatusOp,              /* Which metric to return */
  تهی *pOut                      /* OUT: Write the answer here */
){
  بازگشت sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
تهی sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  صحیح ii;
  برای(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
