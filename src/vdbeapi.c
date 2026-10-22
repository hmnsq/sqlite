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
#include "sqliteInt.h"
#include "vdbeInt.h"
#include "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
số_nguyên sqlite3_expired(sqlite3_stmt *pStmt){
  số_nguyên iRet = 1;
  nếu( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  trả_về iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
static số_nguyên vdbeSafety(Vdbe *p){
  nếu( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    trả_về 1;
  }không_thì{
    trả_về 0;
  }
}
static số_nguyên vdbeSafetyNotNull(Vdbe *p){
  nếu( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    trả_về 1;
  }không_thì{
    trả_về vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
static SQLITE_NOINLINE void invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  assert( p->startTime>0 );
  assert( db->init.busy==0 );
  assert( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  nếu( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  nếu( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (void*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   nếu( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#không_thì
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
số_nguyên sqlite3_finalize(sqlite3_stmt *pStmt){
  số_nguyên rc;
  nếu( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }không_thì{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    nếu( vdbeSafety(v) ) trả_về SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    assert( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  trả_về rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
số_nguyên sqlite3_reset(sqlite3_stmt *pStmt){
  số_nguyên rc;
  nếu( pStmt==0 ){
    rc = SQLITE_OK;
  }không_thì{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    assert( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  trả_về rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
số_nguyên sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  số_nguyên i;
  số_nguyên rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#nếu SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pStmt==0 ){
    trả_về SQLITE_MISUSE_BKPT;
  }
#endif
#nếu SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  cho(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    nếu( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  assert( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  nếu( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  trả_về rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
const void *sqlite3_value_blob(sqlite3_value *pVal){
  Mem *p = (Mem*)pVal;
  nếu( p->flags & (MEM_Blob|MEM_Str) ){
    nếu( ExpandBlob(p)!=SQLITE_OK ){
      assert( p->flags==MEM_Null && p->z==0 );
      trả_về 0;
    }
    p->flags |= MEM_Blob;
    trả_về p->n ? p->z : 0;
  }không_thì{
    trả_về sqlite3_value_text(pVal);
  }
}
số_nguyên sqlite3_value_bytes(sqlite3_value *pVal){
  trả_về sqlite3ValueBytes(pVal, SQLITE_UTF8);
}
số_nguyên sqlite3_value_bytes16(sqlite3_value *pVal){
  trả_về sqlite3ValueBytes(pVal, SQLITE_UTF16NATIVE);
}
double sqlite3_value_double(sqlite3_value *pVal){
  trả_về sqlite3VdbeRealValue((Mem*)pVal);
}
số_nguyên sqlite3_value_int(sqlite3_value *pVal){
  trả_về (số_nguyên)sqlite3VdbeIntValue((Mem*)pVal);
}
sqlite_int64 sqlite3_value_int64(sqlite3_value *pVal){
  trả_về sqlite3VdbeIntValue((Mem*)pVal);
}
unsigned số_nguyên sqlite3_value_subtype(sqlite3_value *pVal){
  Mem *pMem = (Mem*)pVal;
  trả_về ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
void *sqlite3_value_pointer(sqlite3_value *pVal, const char *zPType){
  Mem *p = (Mem*)pVal;
  nếu( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    trả_về (void*)p->z;
  }không_thì{
    trả_về 0;
  }
}
const unsigned char *sqlite3_value_text(sqlite3_value *pVal){
  trả_về (const unsigned char *)sqlite3ValueText(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_value_text16(sqlite3_value* pVal){
  trả_về sqlite3ValueText(pVal, SQLITE_UTF16NATIVE);
}
const void *sqlite3_value_text16be(sqlite3_value *pVal){
  trả_về sqlite3ValueText(pVal, SQLITE_UTF16BE);
}
const void *sqlite3_value_text16le(sqlite3_value *pVal){
  trả_về sqlite3ValueText(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
số_nguyên sqlite3_value_type(sqlite3_value* pVal){
  static const u8 aType[] = {
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
    số_nguyên eType = SQLITE_BLOB;
    nếu( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }không_thì nếu( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }không_thì nếu( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }không_thì nếu( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    assert( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  trả_về aType[pVal->flags&MEM_AffMask];
}
số_nguyên sqlite3_value_encoding(sqlite3_value *pVal){
  trả_về pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
số_nguyên sqlite3_value_nochange(sqlite3_value *pVal){
  trả_về (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
số_nguyên sqlite3_value_frombind(sqlite3_value *pVal){
  trả_về (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_value *sqlite3_value_dup(const sqlite3_value *pOrig){
  sqlite3_value *pNew;
  nếu( pOrig==0 ) trả_về 0;
  pNew = sqlite3_malloc( sizeof(*pNew) );
  nếu( pNew==0 ) trả_về 0;
  memset(pNew, 0, sizeof(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  nếu( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    nếu( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3ValueFree(pNew);
      pNew = 0;
    }
  }không_thì nếu( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  trả_về pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
void sqlite3_value_free(sqlite3_value *pOld){
  sqlite3ValueFree(pOld);
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
static void setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  const char *z,          /* String pointer */
  số_nguyên n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  void (*xDel)(void*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  số_nguyên rc;
  nếu( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }không_thì nếu( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    assert( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }không_thì{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  nếu( rc ){
    nếu( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }không_thì{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      assert( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    trả_về;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  nếu( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
static số_nguyên invokeValueDestructor(
  const void *p,             /* Value to destroy */
  void (*xDel)(void*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  assert( xDel!=SQLITE_DYNAMIC );
  nếu( xDel==0 ){
    /* noop */
  }không_thì nếu( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }không_thì{
    xDel((void*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#không_thì
  assert( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  trả_về SQLITE_TOOBIG;
}
void sqlite3_result_blob(
  sqlite3_context *pCtx,
  const void *z,
  số_nguyên n,
  void (*xDel)(void *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 || n<0 ){
    invokeValueDestructor(z, xDel, pCtx);
    trả_về;
  }
#endif
  assert( n>=0 );
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
void sqlite3_result_blob64(
  sqlite3_context *pCtx,
  const void *z,
  sqlite3_uint64 n,
  void (*xDel)(void *)
){
  assert( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ){
    invokeValueDestructor(z, xDel, 0);
    trả_về;
  }
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  nếu( n>0x7fffffff ){
    (void)invokeValueDestructor(z, xDel, pCtx);
  }không_thì{
    setResultStrOrError(pCtx, z, (số_nguyên)n, 0, xDel);
  }
}
void sqlite3_result_double(sqlite3_context *pCtx, double rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
void sqlite3_result_error(sqlite3_context *pCtx, const char *z, số_nguyên n){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
void sqlite3_result_error16(sqlite3_context *pCtx, const void *z, số_nguyên n){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
void sqlite3_result_int(sqlite3_context *pCtx, số_nguyên iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
void sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
void sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
void sqlite3_result_pointer(
  sqlite3_context *pCtx,
  void *pPtr,
  const char *zPType,
  void (*xDestructor)(void*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ){
    invokeValueDestructor(pPtr, xDestructor, 0);
    trả_về;
  }
#endif
  pOut = pCtx->pOut;
  assert( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
void sqlite3_result_subtype(sqlite3_context *pCtx, unsigned số_nguyên eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
#nếu defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  nếu( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    char zErr[200];
    sqlite3_snprintf(sizeof(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    trả_về;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  assert( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
void sqlite3_result_text(
  sqlite3_context *pCtx,
  const char *z,
  số_nguyên n,
  void (*xDel)(void *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ){
    invokeValueDestructor(z, xDel, 0);
    trả_về;
  }
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
void sqlite3_result_text64(
  sqlite3_context *pCtx,
  const char *z,
  sqlite3_uint64 n,
  void (*xDel)(void *),
  unsigned char enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ){
    invokeValueDestructor(z, xDel, 0);
    trả_về;
  }
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  assert( xDel!=SQLITE_DYNAMIC );
  nếu( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    nếu( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  nếu( n>0x7fffffff ){
    (void)invokeValueDestructor(z, xDel, pCtx);
  }không_thì{
    setResultStrOrError(pCtx, z, (số_nguyên)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
void sqlite3_result_text16(
  sqlite3_context *pCtx,
  const void *z,
  số_nguyên n,
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
void sqlite3_result_text16be(
  sqlite3_context *pCtx,
  const void *z,
  số_nguyên n,
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
void sqlite3_result_text16le(
  sqlite3_context *pCtx,
  const void *z,
  số_nguyên n,
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
void sqlite3_result_value(sqlite3_context *pCtx, sqlite3_value *pValue){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
  nếu( pValue==0 ){
    sqlite3_result_null(pCtx);
    trả_về;
  }
#endif
  pOut = pCtx->pOut;
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, pValue);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  nếu( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
void sqlite3_result_zeroblob(sqlite3_context *pCtx, số_nguyên n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
số_nguyên sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  assert( sqlite3_mutex_held(pOut->db->mutex) );
  nếu( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    trả_về SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (số_nguyên)n);
  trả_về SQLITE_OK;
#không_thì
  trả_về sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (số_nguyên)n);
#endif
}
void sqlite3_result_error_code(sqlite3_context *pCtx, số_nguyên errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  nếu( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  nếu( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
void sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
void sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
void sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, số_nguyên eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
  nếu( pStr==0 ) trả_về;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  nếu( pStr->accError==0 ){
    nếu( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }không_thì{
      const char *zText = sqlite3_str_value(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      assert( isMalloced(pStr) );
      nếu( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }không_thì{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }không_thì nếu( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }không_thì{
    assert( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  nếu( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
void sqlite3ResultIntReal(sqlite3_context *pCtx){
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  nếu( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
static số_nguyên doWalCallbacks(sqlite3 *db){
  số_nguyên rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  số_nguyên i;
  cho(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    nếu( pBt ){
      số_nguyên nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      nếu( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#không_thì
  UNUSED_PARAMETER(db);
#endif
  trả_về rc;
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
static số_nguyên sqlite3Step(Vdbe *p){
  sqlite3 *db;
  số_nguyên rc;

  assert(p);
  db = p->db;
  nếu( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    nếu( p->eVdbeState==VDBE_READY_STATE ){
      nếu( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        nếu( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        goto end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      nếu( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      assert( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      nếu( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }không_thì{
        assert( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      nếu( p->readOnly==0 ) db->nVdbeWrite++;
      nếu( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }không_thì

    nếu( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      nếu( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }không_thì{
        trả_về SQLITE_MISUSE_BKPT;
      }
#không_thì
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      assert( p->eVdbeState==VDBE_READY_STATE );
      goto restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  nếu( p->explain ){
    rc = sqlite3VdbeList(p);
  }không_thì
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  nếu( rc==SQLITE_ROW ){
    assert( p->rc==SQLITE_OK );
    assert( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    trả_về SQLITE_ROW;
  }không_thì{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    nếu( rc==SQLITE_DONE && db->autoCommit ){
      assert( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      nếu( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }không_thì nếu( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  nếu( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    nếu( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  assert( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  trả_về (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
số_nguyên sqlite3_step(sqlite3_stmt *pStmt){
  số_nguyên rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  số_nguyên cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  nếu( vdbeSafetyNotNull(v) ){
    trả_về SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  trong_khi( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    số_nguyên savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    nếu( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      const char *zErr = (const char *)sqlite3_value_text(db->pErr);
      sqlite3DbFree(db, v->zErrMsg);
      nếu( !db->mallocFailed ){
        v->zErrMsg = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } không_thì {
        v->zErrMsg = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      break;
    }
    sqlite3_reset(pStmt);
    nếu( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    assert( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  trả_về rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
void *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( p==0 ) trả_về 0;
#endif
  assert( p && p->pFunc );
  trả_về p->pFunc->pUserData;
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
  nếu( p==0 ) trả_về 0;
#không_thì
  assert( p && p->pOut );
#endif
  trả_về p->pOut->db;
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
số_nguyên sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( p==0 ) trả_về 0;
#không_thì
  assert( p );
#endif
  trả_về sqlite3_value_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
void sqlite3VdbeValueListFree(void *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
static số_nguyên valueFromValueList(
  sqlite3_value *pVal,        /* Pointer to the ValueList object */
  sqlite3_value **ppOut,      /* Store the next value from the list here */
  số_nguyên bNext                   /* 1 for _next(). 0 for _first() */
){
  số_nguyên rc;
  ValueList *pRhs;

  *ppOut = 0;
  nếu( pVal==0 ) trả_về SQLITE_MISUSE_BKPT;
  nếu( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3VdbeValueListFree ){
    trả_về SQLITE_ERROR;
  }không_thì{
    assert( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    assert( pVal->eSubtype=='p' );
    assert( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (ValueList*)pVal->z;
  }
  nếu( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }không_thì{
    số_nguyên dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    assert( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    nếu( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  nếu( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, sizeof(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    nếu( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_value *pOut = pRhs->pOut;
      số_nguyên iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      nếu( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }không_thì{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  trả_về rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
số_nguyên sqlite3_vtab_in_first(sqlite3_value *pVal, sqlite3_value **ppOut){
  trả_về valueFromValueList(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
số_nguyên sqlite3_vtab_in_next(sqlite3_value *pVal, sqlite3_value **ppOut){
  trả_về valueFromValueList(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  số_nguyên rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  assert( p->pVdbe!=0 );
#không_thì
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  nếu( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    nếu( rc ) *piTime = 0;
  }
  trả_về *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
static SQLITE_NOINLINE void *createAggContext(sqlite3_context *p, số_nguyên nByte){
  Mem *pMem = p->pMem;
  assert( (pMem->flags & MEM_Agg)==0 );
  nếu( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }không_thì{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    nếu( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  trả_về (void*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
void *sqlite3_aggregate_context(sqlite3_context *p, số_nguyên nByte){
  assert( p && p->pFunc && p->pFunc->xFinalize );
  assert( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  nếu( (p->pMem->flags & MEM_Agg)==0 ){
    trả_về createAggContext(p, nByte);
  }không_thì{
    trả_về (void*)p->pMem->z;
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
void *sqlite3_get_auxdata(sqlite3_context *pCtx, số_nguyên iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về 0;
#endif
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#nếu SQLITE_ENABLE_STAT4
  nếu( pCtx->pVdbe==0 ) trả_về 0;
#không_thì
  assert( pCtx->pVdbe!=0 );
#endif
  cho(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    nếu(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      trả_về pAuxData->pAux;
    }
  }
  trả_về 0;
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
void sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  số_nguyên iArg,
  void *pAux,
  void (*xDelete)(void*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pCtx==0 ) trả_về;
#endif
  pVdbe= pCtx->pVdbe;
  assert( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  nếu( pVdbe==0 ) goto failed;
#không_thì
  assert( pVdbe!=0 );
#endif

  cho(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    nếu( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      break;
    }
  }
  nếu( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, sizeof(AuxData));
    nếu( !pAuxData ) goto failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    nếu( pCtx->isError==0 ) pCtx->isError = -1;
  }không_thì nếu( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  trả_về;

failed:
  nếu( xDelete ){
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
số_nguyên sqlite3_aggregate_count(sqlite3_context *p){
  assert( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  trả_về p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
số_nguyên sqlite3_column_count(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  nếu( pVm==0 ) trả_về 0;
  trả_về pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
số_nguyên sqlite3_data_count(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  nếu( pVm==0 || pVm->pResultRow==0 ) trả_về 0;
  trả_về pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
static const Mem *columnNullValue(void){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  static const Mem nullMem
#nếu defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (char*)0,
        /* .n          = */ (số_nguyên)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (số_nguyên)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (char*)0,
        /* .xDel       = */ (void(*)(void*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  trả_về &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
static Mem *columnMem(sqlite3_stmt *pStmt, số_nguyên i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  nếu( pVm==0 ) trả_về (Mem*)columnNullValue();
  assert( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  nếu( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }không_thì{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)columnNullValue();
  }
  trả_về pOut;
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
static void columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  nếu( p ){
    assert( p->db!=0 );
    assert( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
const void *sqlite3_column_blob(sqlite3_stmt *pStmt, số_nguyên i){
  const void *val;
  val = sqlite3_value_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  trả_về val;
}
số_nguyên sqlite3_column_bytes(sqlite3_stmt *pStmt, số_nguyên i){
  số_nguyên val = sqlite3_value_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
số_nguyên sqlite3_column_bytes16(sqlite3_stmt *pStmt, số_nguyên i){
  số_nguyên val = sqlite3_value_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
double sqlite3_column_double(sqlite3_stmt *pStmt, số_nguyên i){
  double val = sqlite3_value_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
số_nguyên sqlite3_column_int(sqlite3_stmt *pStmt, số_nguyên i){
  số_nguyên val = sqlite3_value_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, số_nguyên i){
  sqlite_int64 val = sqlite3_value_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, số_nguyên i){
  const unsigned char *val = sqlite3_value_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
sqlite3_value *sqlite3_column_value(sqlite3_stmt *pStmt, số_nguyên i){
  Mem *pOut = columnMem(pStmt, i);
  nếu( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  trả_về (sqlite3_value *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_column_text16(sqlite3_stmt *pStmt, số_nguyên i){
  const void *val = sqlite3_value_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về val;
}
#endif /* SQLITE_OMIT_UTF16 */
số_nguyên sqlite3_column_type(sqlite3_stmt *pStmt, số_nguyên i){
  số_nguyên iType = sqlite3_value_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  trả_về iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
static const char * const azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
static const u16 azExplainColNames16data[] = {
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
static const u8 iExplainColNames16[] = {
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
static const void *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  số_nguyên N,                   /* Which column to get the name for */
  số_nguyên useUtf16,            /* True to return the name as UTF16 */
  số_nguyên useType              /* What type of name */
){
  const void *ret;
  Vdbe *p;
  số_nguyên n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pStmt==0 ){
    (void)SQLITE_MISUSE_BKPT;
    trả_về 0;
  }
#endif
  nếu( N<0 ) trả_về 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  assert( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  nếu( p->explain ){
    nếu( useType>0 ) goto columnName_end;
    n = p->explain==1 ? 8 : 4;
    nếu( N>=n ) goto columnName_end;
    nếu( useUtf16 ){
      số_nguyên i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (void*)&azExplainColNames16data[i];
    }không_thì{
      ret = (void*)azExplainColNames8[N + 8*p->explain - 8];
    }
    goto columnName_end;
  }
  n = p->nResColumn;
  nếu( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    nếu( useUtf16 ){
      ret = sqlite3_value_text16((sqlite3_value*)&p->aColName[N]);
    }không_thì
#endif
    {
      ret = sqlite3_value_text((sqlite3_value*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    assert( db->mallocFailed==0 || db->mallocFailed==1 );
    nếu( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  trả_về ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
const char *sqlite3_column_name(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_column_name16(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#nếu defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
const char *sqlite3_column_decltype(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_column_decltype16(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
const char *sqlite3_column_database_name(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_column_database_name16(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
const char *sqlite3_column_table_name(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_column_table_name16(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
const char *sqlite3_column_origin_name(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
const void *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, số_nguyên N){
  trả_về columnName(pStmt, N, 1, COLNAME_COLUMN);
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
static số_nguyên vdbeUnbind(Vdbe *p, unsigned số_nguyên i){
  Mem *pVar;
  nếu( vdbeSafetyNotNull(p) ){
    trả_về SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  nếu( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    trả_về SQLITE_MISUSE_BKPT;
  }
  nếu( i>=(unsigned số_nguyên)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    trả_về SQLITE_RANGE;
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
  assert( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  assert( (p->expmask & p->smimask)==p->smimask );
  nếu( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    nếu( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  trả_về SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
static số_nguyên bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  số_nguyên i,                 /* Index of the parameter to bind */
  const void *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  void (*xDel)(void*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  số_nguyên rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  nếu( rc==SQLITE_OK ){
    assert( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    nếu( zData!=0 ){
      pVar = &p->aVar[i-1];
      nếu( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }không_thì nếu( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        assert( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }không_thì{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        nếu( encoding==0 ) pVar->enc = ENC(p->db);
      }
      nếu( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      nếu( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }không_thì nếu( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((void*)zData);
  }
  trả_về rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
số_nguyên sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  số_nguyên i,
  const void *zData,
  số_nguyên nData,
  void (*xDel)(void*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( nData<0 ) trả_về SQLITE_MISUSE_BKPT;
#endif
  trả_về bindText(pStmt, i, zData, nData, xDel, 0);
}
số_nguyên sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  số_nguyên i,
  const void *zData,
  sqlite3_uint64 nData,
  void (*xDel)(void*)
){
  assert( xDel!=SQLITE_DYNAMIC );
  trả_về bindText(pStmt, i, zData, nData, xDel, 0);
}
số_nguyên sqlite3_bind_double(sqlite3_stmt *pStmt, số_nguyên i, double rValue){
  số_nguyên rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  nếu( rc==SQLITE_OK ){
    assert( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], rValue);
    sqlite3_mutex_leave(p->db->mutex);
  }
  trả_về rc;
}
số_nguyên sqlite3_bind_int(sqlite3_stmt *p, số_nguyên i, số_nguyên iValue){
  trả_về sqlite3_bind_int64(p, i, (i64)iValue);
}
số_nguyên sqlite3_bind_int64(sqlite3_stmt *pStmt, số_nguyên i, sqlite_int64 iValue){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  nếu( vdbeSafetyNotNull(p) ){
    trả_về SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  nếu( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    trả_về SQLITE_MISUSE_BKPT;
  }
  nếu( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    trả_về SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  nếu( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    assert( (p->expmask & p->smimask)==p->smimask );
    nếu( (p->expmask & expireMask)!=0 ){
      nếu( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        số_nguyên sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        số_nguyên sm2 = iValue>=0 && iValue<=1;
        nếu( sm1!=sm2 ){
          p->expired = 1;
        }
      }không_thì nếu( (pVar->flags & MEM_Int)==0 || iValue!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, iValue);
  sqlite3_mutex_leave(p->db->mutex);
  trả_về SQLITE_OK;
}
số_nguyên sqlite3_bind_null(sqlite3_stmt *pStmt, số_nguyên i){
  số_nguyên rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  nếu( rc==SQLITE_OK ){
    assert( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  trả_về rc;
}
số_nguyên sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  số_nguyên i,
  void *pPtr,
  const char *zPTtype,
  void (*xDestructor)(void*)
){
  số_nguyên rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  nếu( rc==SQLITE_OK ){
    assert( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }không_thì nếu( xDestructor ){
    xDestructor(pPtr);
  }
  trả_về rc;
}
số_nguyên sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  số_nguyên i,
  const char *zData,
  số_nguyên nData,
  void (*xDel)(void*)
){
  trả_về bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
số_nguyên sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  số_nguyên i,
  const char *zData,
  sqlite3_uint64 nData,
  void (*xDel)(void*),
  unsigned char enc
){
  assert( xDel!=SQLITE_DYNAMIC );
  nếu( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    nếu( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  trả_về bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
số_nguyên sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  số_nguyên i,
  const void *zData,
  số_nguyên n,
  void (*xDel)(void*)
){
  trả_về bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
số_nguyên sqlite3_bind_value(sqlite3_stmt *pStmt, số_nguyên i, const sqlite3_value *pValue){
  số_nguyên rc;
  switch( sqlite3_value_type((sqlite3_value*)pValue) ){
    case SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, pValue->u.i);
      break;
    }
    case SQLITE_FLOAT: {
      assert( pValue->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (pValue->flags & MEM_Real) ? pValue->u.r : (double)pValue->u.i
      );
      break;
    }
    case SQLITE_BLOB: {
      nếu( pValue->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, pValue->u.nZero);
      }không_thì{
        rc = sqlite3_bind_blob(pStmt, i, pValue->z, pValue->n,SQLITE_TRANSIENT);
      }
      break;
    }
    case SQLITE_TEXT: {
      rc = bindText(pStmt,i,  pValue->z, pValue->n, SQLITE_TRANSIENT,
                              pValue->enc);
      break;
    }
    default: {
      rc = sqlite3_bind_null(pStmt, i);
      break;
    }
  }
  trả_về rc;
}
số_nguyên sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, số_nguyên i, số_nguyên n){
  số_nguyên rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  nếu( rc==SQLITE_OK ){
    assert( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#không_thì
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  trả_về rc;
}
số_nguyên sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, số_nguyên i, sqlite3_uint64 n){
  số_nguyên rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( p==0 ) trả_về SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  nếu( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }không_thì{
    assert( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  trả_về rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
số_nguyên sqlite3_bind_parameter_count(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  trả_về p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
const char *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, số_nguyên i){
  Vdbe *p = (Vdbe*)pStmt;
  nếu( p==0 ) trả_về 0;
  trả_về sqlite3VListNumToName(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
số_nguyên sqlite3VdbeParameterIndex(Vdbe *p, const char *zName, số_nguyên nName){
  nếu( p==0 || zName==0 ) trả_về 0;
  trả_về sqlite3VListNameToNum(p->pVList, zName, nName);
}
số_nguyên sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName){
  trả_về sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
số_nguyên sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  số_nguyên i;
  assert( pTo->db==pFrom->db );
  assert( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  cho(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  trả_về SQLITE_OK;
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
số_nguyên sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  nếu( pFrom->nVar!=pTo->nVar ){
    trả_về SQLITE_ERROR;
  }
  assert( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  nếu( pTo->expmask ){
    pTo->expired = 1;
  }
  assert( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  nếu( pFrom->expmask ){
    pFrom->expired = 1;
  }
  trả_về sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  trả_về pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
số_nguyên sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  trả_về pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
số_nguyên sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  trả_về pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
số_nguyên sqlite3_stmt_explain(sqlite3_stmt *pStmt, số_nguyên eMode){
  Vdbe *v = (Vdbe*)pStmt;
  số_nguyên rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pStmt==0 ) trả_về SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  nếu( ((số_nguyên)v->explain)==eMode ){
    rc = SQLITE_OK;
  }không_thì nếu( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }không_thì nếu( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }không_thì nếu( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }không_thì nếu( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }không_thì{
    số_nguyên savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    nếu( rc ){
      v->explain = savedMode;
    }không_thì{
      v->haveEqpOps = eMode==2;
    }
  }
  nếu( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }không_thì{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  trả_về rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
số_nguyên sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  trả_về v!=0 && v->eVdbeState==VDBE_RUN_STATE;
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
  nếu( !sqlite3SafetyCheckOk(pDb) ){
    (void)SQLITE_MISUSE_BKPT;
    trả_về 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  nếu( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }không_thì{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  trả_về pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
số_nguyên sqlite3_stmt_status(sqlite3_stmt *pStmt, số_nguyên op, số_nguyên resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (void)SQLITE_MISUSE_BKPT;
    trả_về 0;
  }
#endif
  nếu( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (số_nguyên*)&v;
    assert( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }không_thì{
    v = pVdbe->aCounter[op];
    nếu( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  trả_về (số_nguyên)v;
}

/*
** Return the SQL associated with a prepared statement
*/
const char *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  trả_về p ? p->zSql : 0;
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
char *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  trả_về 0;
#không_thì
  char *z = 0;
  const char *zSql = sqlite3_sql(pStmt);
  nếu( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  trả_về z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
const char *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  nếu( p==0 ) trả_về 0;
  nếu( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  trả_về p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
static UnpackedRecord *vdbeUnpackRecord(
  KeyInfo *pKeyInfo,
  số_nguyên nKey,
  const void *pKey
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(pKeyInfo);
  nếu( pRet ){
    memset(pRet->aMem, 0, sizeof(Mem)*(pKeyInfo->nKeyField+1));
    sqlite3VdbeRecordUnpack(nKey, pKey, pRet);
  }
  trả_về pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
số_nguyên sqlite3_preupdate_old(sqlite3 *db, số_nguyên iIdx, sqlite3_value **ppValue){
  PreUpdate *p;
  Mem *pMem;
  số_nguyên rc = SQLITE_OK;
  số_nguyên iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( db==0 || ppValue==0 ){
    trả_về SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  nếu( !p || p->op==SQLITE_INSERT ){
    rc = SQLITE_MISUSE_BKPT;
    goto preupdate_old_out;
  }
  nếu( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }không_thì nếu( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    goto preupdate_old_out;
  }không_thì{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  nếu( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    goto preupdate_old_out;
  }

  nếu( iIdx==p->pTab->iPKey ){
    *ppValue = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }không_thì{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    nếu( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      assert( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      nếu( !aRec ) goto preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      nếu( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        nếu( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      nếu( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        goto preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *ppValue = &p->pUnpacked->aMem[iStore];
    nếu( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      nếu( pCol->iDflt>0 ){
        nếu( p->apDflt==0 ){
          số_nguyên nByte;
          assert( sizeof(sqlite3_value*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = sizeof(sqlite3_value*)*p->pTab->nCol;
          p->apDflt = (sqlite3_value**)sqlite3DbMallocZero(db, nByte);
          nếu( p->apDflt==0 ) goto preupdate_old_out;
        }
        nếu( p->apDflt[iIdx]==0 ){
          sqlite3_value *pVal = 0;
          Expr *pDflt;
          assert( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].pExpr;
          rc = sqlite3ValueFromExpr(db, pDflt, ENC(db), pCol->affinity, &pVal);
          nếu( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *ppValue = p->apDflt[iIdx];
      }không_thì{
        *ppValue = (sqlite3_value *)columnNullValue();
      }
    }không_thì nếu( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      nếu( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  trả_về sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
số_nguyên sqlite3_preupdate_count(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#không_thì
  p = db->pPreUpdate;
#endif
  trả_về (p ? p->pKeyinfo->nKeyField : 0);
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
số_nguyên sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#không_thì
  p = db->pPreUpdate;
#endif
  trả_về (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
số_nguyên sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#không_thì
  p = db->pPreUpdate;
#endif
  trả_về (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
số_nguyên sqlite3_preupdate_new(sqlite3 *db, số_nguyên iIdx, sqlite3_value **ppValue){
  PreUpdate *p;
  số_nguyên rc = SQLITE_OK;
  Mem *pMem;
  số_nguyên iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( db==0 || ppValue==0 ){
    trả_về SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  nếu( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    goto preupdate_new_out;
  }
  nếu( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }không_thì nếu( iIdx >= p->pTab->nCol ){
    trả_về SQLITE_MISUSE_BKPT;
  }không_thì{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  nếu( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    goto preupdate_new_out;
  }

  nếu( p->op==SQLITE_INSERT ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    nếu( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      nếu( rc!=SQLITE_OK ) goto preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      nếu( !pUnpack ){
        rc = SQLITE_NOMEM;
        goto preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    nếu( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }không_thì nếu( iStore>=pUnpack->nField ){
      pMem = (sqlite3_value *)columnNullValue();
    }không_thì nếu( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }không_thì{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    assert( p->op==SQLITE_UPDATE );
    nếu( !p->aNew ){
      assert( sizeof(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, sizeof(Mem)*p->pCsr->nField);
      nếu( !p->aNew ){
        rc = SQLITE_NOMEM;
        goto preupdate_new_out;
      }
    }
    assert( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    nếu( pMem->flags==0 ){
      nếu( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }không_thì{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        nếu( rc!=SQLITE_OK ) goto preupdate_new_out;
      }
    }
  }
  *ppValue = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  trả_về sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
số_nguyên sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  số_nguyên iScan,                      /* Index of loop to report on */
  số_nguyên iScanStatusOp,              /* Which metric to return */
  số_nguyên flags,
  void *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  số_nguyên nOp;
  ScanStatus *pScan = 0;
  số_nguyên idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    trả_về 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  nếu( p->pFrame ){
    VdbeFrame *pFrame;
    cho(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  nếu( iScan<0 ){
    số_nguyên ii;
    nếu( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      cho(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      trả_về 0;
    }
    trả_về 1;
  }
  nếu( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }không_thì{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    cho(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      nếu( pScan->zName ){
        iScan--;
        nếu( iScan<0 ) break;
      }
    }
  }
  nếu( idx>=p->nScan ) trả_về 1;
  assert( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  switch( iScanStatusOp ){
    case SQLITE_SCANSTAT_NLOOP: {
      nếu( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }không_thì{
        *(sqlite3_int64*)pOut = -1;
      }
      break;
    }
    case SQLITE_SCANSTAT_NVISIT: {
      nếu( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }không_thì{
        *(sqlite3_int64*)pOut = -1;
      }
      break;
    }
    case SQLITE_SCANSTAT_EST: {
      double r = 1.0;
      LogEst x = pScan->nEst;
      trong_khi( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(double*)pOut = r*sqlite3LogEstToInt(x);
      break;
    }
    case SQLITE_SCANSTAT_NAME: {
      *(const char**)pOut = pScan->zName;
      break;
    }
    case SQLITE_SCANSTAT_EXPLAIN: {
      nếu( pScan->addrExplain ){
        *(const char**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }không_thì{
        *(const char**)pOut = 0;
      }
      break;
    }
    case SQLITE_SCANSTAT_SELECTID: {
      nếu( pScan->addrExplain ){
        *(số_nguyên*)pOut = aOp[ pScan->addrExplain ].p1;
      }không_thì{
        *(số_nguyên*)pOut = -1;
      }
      break;
    }
    case SQLITE_SCANSTAT_PARENTID: {
      nếu( pScan->addrExplain ){
        *(số_nguyên*)pOut = aOp[ pScan->addrExplain ].p2;
      }không_thì{
        *(số_nguyên*)pOut = -1;
      }
      break;
    }
    case SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      nếu( pScan->aAddrRange[0]==0 ){
        res = -1;
      }không_thì{
        số_nguyên ii;
        cho(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          số_nguyên iIns = pScan->aAddrRange[ii];
          số_nguyên iEnd = pScan->aAddrRange[ii+1];
          nếu( iIns==0 ) break;
          nếu( iIns>0 ){
            trong_khi( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }không_thì{
            số_nguyên iOp;
            cho(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              nếu( pOp->p1!=iEnd ) continue;
              nếu( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                continue;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      break;
    }
    default: {
      trả_về 1;
    }
  }
  trả_về 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
số_nguyên sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  số_nguyên iScan,                      /* Index of loop to report on */
  số_nguyên iScanStatusOp,              /* Which metric to return */
  void *pOut                      /* OUT: Write the answer here */
){
  trả_về sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
void sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  số_nguyên ii;
  cho(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
