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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "vdbeInt.h"
#അന്തര്ഭാവ "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
പൂർണ്ണസംഖ്യ sqlite3_expired(sqlite3_stmt *pStmt){
  പൂർണ്ണസംഖ്യ iRet = 1;
  എങ്കിൽ( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  തിരികെ iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vdbeSafety(Vdbe *p){
  എങ്കിൽ( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}
സ്ഥിതം പൂർണ്ണസംഖ്യ vdbeSafetyNotNull(Vdbe *p){
  എങ്കിൽ( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  പ്രതിജ്ഞാ( p->startTime>0 );
  പ്രതിജ്ഞാ( db->init.busy==0 );
  പ്രതിജ്ഞാ( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  എങ്കിൽ( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  എങ്കിൽ( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (ശൂന്യം*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   എങ്കിൽ( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#അല്ലെങ്കിൽ
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
പൂർണ്ണസംഖ്യ sqlite3_finalize(sqlite3_stmt *pStmt){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }അല്ലെങ്കിൽ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    എങ്കിൽ( vdbeSafety(v) ) തിരികെ SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    പ്രതിജ്ഞാ( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  തിരികെ rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
പൂർണ്ണസംഖ്യ sqlite3_reset(sqlite3_stmt *pStmt){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( pStmt==0 ){
    rc = SQLITE_OK;
  }അല്ലെങ്കിൽ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    പ്രതിജ്ഞാ( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  തിരികെ rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
പൂർണ്ണസംഖ്യ sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#എങ്കിൽ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pStmt==0 ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
#endif
#എങ്കിൽ SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  വേണ്ടി(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    എങ്കിൽ( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  പ്രതിജ്ഞാ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  എങ്കിൽ( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  തിരികെ rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
സ്ഥിരം ശൂന്യം *sqlite3_മൂല്യം_blob(sqlite3_മൂല്യം *pVal){
  Mem *p = (Mem*)pVal;
  എങ്കിൽ( p->flags & (MEM_Blob|MEM_Str) ){
    എങ്കിൽ( ExpandBlob(p)!=SQLITE_OK ){
      പ്രതിജ്ഞാ( p->flags==MEM_Null && p->z==0 );
      തിരികെ 0;
    }
    p->flags |= MEM_Blob;
    തിരികെ p->n ? p->z : 0;
  }അല്ലെങ്കിൽ{
    തിരികെ sqlite3_മൂല്യം_text(pVal);
  }
}
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_bytes(sqlite3_മൂല്യം *pVal){
  തിരികെ sqlite3_മൂല്യം_Bytes(pVal, SQLITE_UTF8);
}
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_bytes16(sqlite3_മൂല്യം *pVal){
  തിരികെ sqlite3_മൂല്യം_Bytes(pVal, SQLITE_UTF16NATIVE);
}
ഇരട്ട sqlite3_മൂല്യം_double(sqlite3_മൂല്യം *pVal){
  തിരികെ sqlite3_Vdbe_Real_മൂല്യം((Mem*)pVal);
}
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_int(sqlite3_മൂല്യം *pVal){
  തിരികെ (പൂർണ്ണസംഖ്യ)sqlite3_Vdbe_Int_മൂല്യം((Mem*)pVal);
}
sqlite_int64 sqlite3_മൂല്യം_int64(sqlite3_മൂല്യം *pVal){
  തിരികെ sqlite3_Vdbe_Int_മൂല്യം((Mem*)pVal);
}
unsigned പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_subtype(sqlite3_മൂല്യം *pVal){
  Mem *pMem = (Mem*)pVal;
  തിരികെ ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
ശൂന്യം *sqlite3_മൂല്യം_pointer(sqlite3_മൂല്യം *pVal, സ്ഥിരം അക്ഷരം *zPType){
  Mem *p = (Mem*)pVal;
  എങ്കിൽ( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    തിരികെ (ശൂന്യം*)p->z;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}
സ്ഥിരം unsigned അക്ഷരം *sqlite3_മൂല്യം_text(sqlite3_മൂല്യം *pVal){
  തിരികെ (സ്ഥിരം unsigned അക്ഷരം *)sqlite3_മൂല്യം_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_മൂല്യം_text16(sqlite3_മൂല്യം* pVal){
  തിരികെ sqlite3_മൂല്യം_Text(pVal, SQLITE_UTF16NATIVE);
}
സ്ഥിരം ശൂന്യം *sqlite3_മൂല്യം_text16be(sqlite3_മൂല്യം *pVal){
  തിരികെ sqlite3_മൂല്യം_Text(pVal, SQLITE_UTF16BE);
}
സ്ഥിരം ശൂന്യം *sqlite3_മൂല്യം_text16le(sqlite3_മൂല്യം *pVal){
  തിരികെ sqlite3_മൂല്യം_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_type(sqlite3_മൂല്യം* pVal){
  സ്ഥിതം സ്ഥിരം u8 aType[] = {
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
    പൂർണ്ണസംഖ്യ eType = SQLITE_BLOB;
    എങ്കിൽ( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }അല്ലെങ്കിൽ എങ്കിൽ( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }അല്ലെങ്കിൽ എങ്കിൽ( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }അല്ലെങ്കിൽ എങ്കിൽ( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    പ്രതിജ്ഞാ( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  തിരികെ aType[pVal->flags&MEM_AffMask];
}
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_encoding(sqlite3_മൂല്യം *pVal){
  തിരികെ pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_nochange(sqlite3_മൂല്യം *pVal){
  തിരികെ (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_frombind(sqlite3_മൂല്യം *pVal){
  തിരികെ (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_മൂല്യം *sqlite3_മൂല്യം_dup(സ്ഥിരം sqlite3_മൂല്യം *pOrig){
  sqlite3_മൂല്യം *pNew;
  എങ്കിൽ( pOrig==0 ) തിരികെ 0;
  pNew = sqlite3_malloc( വലുപ്പം(*pNew) );
  എങ്കിൽ( pNew==0 ) തിരികെ 0;
  memset(pNew, 0, വലുപ്പം(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  എങ്കിൽ( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    എങ്കിൽ( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_മൂല്യം_Free(pNew);
      pNew = 0;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  തിരികെ pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
ശൂന്യം sqlite3_മൂല്യം_free(sqlite3_മൂല്യം *pOld){
  sqlite3_മൂല്യം_Free(pOld);
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
സ്ഥിതം ശൂന്യം setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  സ്ഥിരം അക്ഷരം *z,          /* String pointer */
  പൂർണ്ണസംഖ്യ n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  ശൂന്യം (*xDel)(ശൂന്യം*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }അല്ലെങ്കിൽ എങ്കിൽ( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    പ്രതിജ്ഞാ( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }അല്ലെങ്കിൽ{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  എങ്കിൽ( rc ){
    എങ്കിൽ( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }അല്ലെങ്കിൽ{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      പ്രതിജ്ഞാ( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    തിരികെ;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  എങ്കിൽ( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
സ്ഥിതം പൂർണ്ണസംഖ്യ invoke_മൂല്യം_Destructor(
  സ്ഥിരം ശൂന്യം *p,             /* Value to destroy */
  ശൂന്യം (*xDel)(ശൂന്യം*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  പ്രതിജ്ഞാ( xDel!=SQLITE_DYNAMIC );
  എങ്കിൽ( xDel==0 ){
    /* noop */
  }അല്ലെങ്കിൽ എങ്കിൽ( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }അല്ലെങ്കിൽ{
    xDel((ശൂന്യം*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  തിരികെ SQLITE_TOOBIG;
}
ശൂന്യം sqlite3_result_blob(
  sqlite3_context *pCtx,
  സ്ഥിരം ശൂന്യം *z,
  പൂർണ്ണസംഖ്യ n,
  ശൂന്യം (*xDel)(ശൂന്യം *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 || n<0 ){
    invoke_മൂല്യം_Destructor(z, xDel, pCtx);
    തിരികെ;
  }
#endif
  പ്രതിജ്ഞാ( n>=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
ശൂന്യം sqlite3_result_blob64(
  sqlite3_context *pCtx,
  സ്ഥിരം ശൂന്യം *z,
  sqlite3_uint64 n,
  ശൂന്യം (*xDel)(ശൂന്യം *)
){
  പ്രതിജ്ഞാ( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ){
    invoke_മൂല്യം_Destructor(z, xDel, 0);
    തിരികെ;
  }
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  എങ്കിൽ( n>0x7fffffff ){
    (ശൂന്യം)invoke_മൂല്യം_Destructor(z, xDel, pCtx);
  }അല്ലെങ്കിൽ{
    setResultStrOrError(pCtx, z, (പൂർണ്ണസംഖ്യ)n, 0, xDel);
  }
}
ശൂന്യം sqlite3_result_double(sqlite3_context *pCtx, ഇരട്ട rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
ശൂന്യം sqlite3_result_error(sqlite3_context *pCtx, സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ n){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
ശൂന്യം sqlite3_result_error16(sqlite3_context *pCtx, സ്ഥിരം ശൂന്യം *z, പൂർണ്ണസംഖ്യ n){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
ശൂന്യം sqlite3_result_int(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
ശൂന്യം sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
ശൂന്യം sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
ശൂന്യം sqlite3_result_pointer(
  sqlite3_context *pCtx,
  ശൂന്യം *pPtr,
  സ്ഥിരം അക്ഷരം *zPType,
  ശൂന്യം (*xDestructor)(ശൂന്യം*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ){
    invoke_മൂല്യം_Destructor(pPtr, xDestructor, 0);
    തിരികെ;
  }
#endif
  pOut = pCtx->pOut;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
ശൂന്യം sqlite3_result_subtype(sqlite3_context *pCtx, unsigned പൂർണ്ണസംഖ്യ eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
#എങ്കിൽ defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  എങ്കിൽ( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    അക്ഷരം zErr[200];
    sqlite3_snprintf(വലുപ്പം(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    തിരികെ;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
ശൂന്യം sqlite3_result_text(
  sqlite3_context *pCtx,
  സ്ഥിരം അക്ഷരം *z,
  പൂർണ്ണസംഖ്യ n,
  ശൂന്യം (*xDel)(ശൂന്യം *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ){
    invoke_മൂല്യം_Destructor(z, xDel, 0);
    തിരികെ;
  }
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
ശൂന്യം sqlite3_result_text64(
  sqlite3_context *pCtx,
  സ്ഥിരം അക്ഷരം *z,
  sqlite3_uint64 n,
  ശൂന്യം (*xDel)(ശൂന്യം *),
  unsigned അക്ഷരം enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ){
    invoke_മൂല്യം_Destructor(z, xDel, 0);
    തിരികെ;
  }
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  പ്രതിജ്ഞാ( xDel!=SQLITE_DYNAMIC );
  എങ്കിൽ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    എങ്കിൽ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  എങ്കിൽ( n>0x7fffffff ){
    (ശൂന്യം)invoke_മൂല്യം_Destructor(z, xDel, pCtx);
  }അല്ലെങ്കിൽ{
    setResultStrOrError(pCtx, z, (പൂർണ്ണസംഖ്യ)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
ശൂന്യം sqlite3_result_text16(
  sqlite3_context *pCtx,
  സ്ഥിരം ശൂന്യം *z,
  പൂർണ്ണസംഖ്യ n,
  ശൂന്യം (*xDel)(ശൂന്യം *)
){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
ശൂന്യം sqlite3_result_text16be(
  sqlite3_context *pCtx,
  സ്ഥിരം ശൂന്യം *z,
  പൂർണ്ണസംഖ്യ n,
  ശൂന്യം (*xDel)(ശൂന്യം *)
){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
ശൂന്യം sqlite3_result_text16le(
  sqlite3_context *pCtx,
  സ്ഥിരം ശൂന്യം *z,
  പൂർണ്ണസംഖ്യ n,
  ശൂന്യം (*xDel)(ശൂന്യം *)
){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
ശൂന്യം sqlite3_result_മൂല്യം(sqlite3_context *pCtx, sqlite3_മൂല്യം *p_മൂല്യം){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
  എങ്കിൽ( p_മൂല്യം==0 ){
    sqlite3_result_null(pCtx);
    തിരികെ;
  }
#endif
  pOut = pCtx->pOut;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_മൂല്യം);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  എങ്കിൽ( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
ശൂന്യം sqlite3_result_zeroblob(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
പൂർണ്ണസംഖ്യ sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pOut->db->mutex) );
  എങ്കിൽ( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    തിരികെ SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (പൂർണ്ണസംഖ്യ)n);
  തിരികെ SQLITE_OK;
#അല്ലെങ്കിൽ
  തിരികെ sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (പൂർണ്ണസംഖ്യ)n);
#endif
}
ശൂന്യം sqlite3_result_error_code(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  എങ്കിൽ( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  എങ്കിൽ( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
ശൂന്യം sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
ശൂന്യം sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
ശൂന്യം sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, പൂർണ്ണസംഖ്യ eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
  എങ്കിൽ( pStr==0 ) തിരികെ;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  എങ്കിൽ( pStr->accError==0 ){
    എങ്കിൽ( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }അല്ലെങ്കിൽ{
      സ്ഥിരം അക്ഷരം *zText = sqlite3_str_മൂല്യം(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      പ്രതിജ്ഞാ( isMalloced(pStr) );
      എങ്കിൽ( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }അല്ലെങ്കിൽ{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  എങ്കിൽ( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
ശൂന്യം sqlite3ResultIntReal(sqlite3_context *pCtx){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  എങ്കിൽ( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ doWalCallbacks(sqlite3 *db){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    എങ്കിൽ( pBt ){
      പൂർണ്ണസംഖ്യ nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      എങ്കിൽ( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#അല്ലെങ്കിൽ
  UNUSED_PARAMETER(db);
#endif
  തിരികെ rc;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3Step(Vdbe *p){
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;

  പ്രതിജ്ഞാ(p);
  db = p->db;
  എങ്കിൽ( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    എങ്കിൽ( p->eVdbeState==VDBE_READY_STATE ){
      എങ്കിൽ( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        എങ്കിൽ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        പോകുക end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      എങ്കിൽ( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      പ്രതിജ്ഞാ( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      എങ്കിൽ( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }അല്ലെങ്കിൽ{
        പ്രതിജ്ഞാ( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      എങ്കിൽ( p->readOnly==0 ) db->nVdbeWrite++;
      എങ്കിൽ( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }അല്ലെങ്കിൽ

    എങ്കിൽ( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      എങ്കിൽ( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }അല്ലെങ്കിൽ{
        തിരികെ SQLITE_MISUSE_BKPT;
      }
#അല്ലെങ്കിൽ
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      പ്രതിജ്ഞാ( p->eVdbeState==VDBE_READY_STATE );
      പോകുക restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  എങ്കിൽ( p->explain ){
    rc = sqlite3VdbeList(p);
  }അല്ലെങ്കിൽ
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  എങ്കിൽ( rc==SQLITE_ROW ){
    പ്രതിജ്ഞാ( p->rc==SQLITE_OK );
    പ്രതിജ്ഞാ( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    തിരികെ SQLITE_ROW;
  }അല്ലെങ്കിൽ{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    എങ്കിൽ( rc==SQLITE_DONE && db->autoCommit ){
      പ്രതിജ്ഞാ( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      എങ്കിൽ( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  എങ്കിൽ( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    എങ്കിൽ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  പ്രതിജ്ഞാ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  തിരികെ (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
പൂർണ്ണസംഖ്യ sqlite3_step(sqlite3_stmt *pStmt){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  പൂർണ്ണസംഖ്യ cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  എങ്കിൽ( vdbeSafetyNotNull(v) ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  ആകുമ്പോൾ( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    പൂർണ്ണസംഖ്യ savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    എങ്കിൽ( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      സ്ഥിരം അക്ഷരം *zErr = (സ്ഥിരം അക്ഷരം *)sqlite3_മൂല്യം_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_സന്ദേശം);
      എങ്കിൽ( !db->mallocFailed ){
        v->z_Err_സന്ദേശം = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } അല്ലെങ്കിൽ {
        v->z_Err_സന്ദേശം = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      നിർത്തുക;
    }
    sqlite3_reset(pStmt);
    എങ്കിൽ( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    പ്രതിജ്ഞാ( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  തിരികെ rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
ശൂന്യം *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( p==0 ) തിരികെ 0;
#endif
  പ്രതിജ്ഞാ( p && p->pFunc );
  തിരികെ p->pFunc->pUserData;
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
  എങ്കിൽ( p==0 ) തിരികെ 0;
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( p && p->pOut );
#endif
  തിരികെ p->pOut->db;
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
പൂർണ്ണസംഖ്യ sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( p==0 ) തിരികെ 0;
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( p );
#endif
  തിരികെ sqlite3_മൂല്യം_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
ശൂന്യം sqlite3_Vdbe_മൂല്യം_List_Free(ശൂന്യം *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ മൂല്യം_From_മൂല്യം_List(
  sqlite3_മൂല്യം *pVal,        /* Pointer to the ValueList object */
  sqlite3_മൂല്യം **ppOut,      /* Store the next value from the list here */
  പൂർണ്ണസംഖ്യ bNext                   /* 1 for _next(). 0 for _first() */
){
  പൂർണ്ണസംഖ്യ rc;
  മൂല്യം_List *pRhs;

  *ppOut = 0;
  എങ്കിൽ( pVal==0 ) തിരികെ SQLITE_MISUSE_BKPT;
  എങ്കിൽ( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_മൂല്യം_List_Free ){
    തിരികെ SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    പ്രതിജ്ഞാ( pVal->eSubtype=='p' );
    പ്രതിജ്ഞാ( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (മൂല്യം_List*)pVal->z;
  }
  എങ്കിൽ( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    പ്രതിജ്ഞാ( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    എങ്കിൽ( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, വലുപ്പം(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    എങ്കിൽ( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_മൂല്യം *pOut = pRhs->pOut;
      പൂർണ്ണസംഖ്യ iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      എങ്കിൽ( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }അല്ലെങ്കിൽ{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  തിരികെ rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
പൂർണ്ണസംഖ്യ sqlite3_vtab_in_first(sqlite3_മൂല്യം *pVal, sqlite3_മൂല്യം **ppOut){
  തിരികെ മൂല്യം_From_മൂല്യം_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
പൂർണ്ണസംഖ്യ sqlite3_vtab_in_next(sqlite3_മൂല്യം *pVal, sqlite3_മൂല്യം **ppOut){
  തിരികെ മൂല്യം_From_മൂല്യം_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  പൂർണ്ണസംഖ്യ rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  പ്രതിജ്ഞാ( p->pVdbe!=0 );
#അല്ലെങ്കിൽ
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  എങ്കിൽ( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    എങ്കിൽ( rc ) *piTime = 0;
  }
  തിരികെ *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം *createAggContext(sqlite3_context *p, പൂർണ്ണസംഖ്യ nByte){
  Mem *pMem = p->pMem;
  പ്രതിജ്ഞാ( (pMem->flags & MEM_Agg)==0 );
  എങ്കിൽ( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }അല്ലെങ്കിൽ{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    എങ്കിൽ( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  തിരികെ (ശൂന്യം*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
ശൂന്യം *sqlite3_aggregate_context(sqlite3_context *p, പൂർണ്ണസംഖ്യ nByte){
  പ്രതിജ്ഞാ( p && p->pFunc && p->pFunc->xFinalize );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  എങ്കിൽ( (p->pMem->flags & MEM_Agg)==0 ){
    തിരികെ createAggContext(p, nByte);
  }അല്ലെങ്കിൽ{
    തിരികെ (ശൂന്യം*)p->pMem->z;
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
ശൂന്യം *sqlite3_get_auxdata(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ 0;
#endif
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#എങ്കിൽ SQLITE_ENABLE_STAT4
  എങ്കിൽ( pCtx->pVdbe==0 ) തിരികെ 0;
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( pCtx->pVdbe!=0 );
#endif
  വേണ്ടി(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    എങ്കിൽ(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      തിരികെ pAuxData->pAux;
    }
  }
  തിരികെ 0;
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
ശൂന്യം sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  പൂർണ്ണസംഖ്യ iArg,
  ശൂന്യം *pAux,
  ശൂന്യം (*xDelete)(ശൂന്യം*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCtx==0 ) തിരികെ;
#endif
  pVdbe= pCtx->pVdbe;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  എങ്കിൽ( pVdbe==0 ) പോകുക failed;
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( pVdbe!=0 );
#endif

  വേണ്ടി(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    എങ്കിൽ( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      നിർത്തുക;
    }
  }
  എങ്കിൽ( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, വലുപ്പം(AuxData));
    എങ്കിൽ( !pAuxData ) പോകുക failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    എങ്കിൽ( pCtx->isError==0 ) pCtx->isError = -1;
  }അല്ലെങ്കിൽ എങ്കിൽ( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  തിരികെ;

failed:
  എങ്കിൽ( xDelete ){
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
പൂർണ്ണസംഖ്യ sqlite3_aggregate_എണ്ണം(sqlite3_context *p){
  പ്രതിജ്ഞാ( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  തിരികെ p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
പൂർണ്ണസംഖ്യ sqlite3_column_എണ്ണം(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  എങ്കിൽ( pVm==0 ) തിരികെ 0;
  തിരികെ pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
പൂർണ്ണസംഖ്യ sqlite3_data_എണ്ണം(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  എങ്കിൽ( pVm==0 || pVm->pResultRow==0 ) തിരികെ 0;
  തിരികെ pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
സ്ഥിതം സ്ഥിരം Mem *column_Null_മൂല്യം(ശൂന്യം){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  സ്ഥിതം സ്ഥിരം Mem nullMem
#എങ്കിൽ defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (അക്ഷരം*)0,
        /* .n          = */ (പൂർണ്ണസംഖ്യ)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (പൂർണ്ണസംഖ്യ)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (അക്ഷരം*)0,
        /* .xDel       = */ (ശൂന്യം(*)(ശൂന്യം*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  തിരികെ &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
സ്ഥിതം Mem *columnMem(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  എങ്കിൽ( pVm==0 ) തിരികെ (Mem*)column_Null_മൂല്യം();
  പ്രതിജ്ഞാ( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  എങ്കിൽ( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }അല്ലെങ്കിൽ{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_മൂല്യം();
  }
  തിരികെ pOut;
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
സ്ഥിതം ശൂന്യം columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  എങ്കിൽ( p ){
    പ്രതിജ്ഞാ( p->db!=0 );
    പ്രതിജ്ഞാ( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
സ്ഥിരം ശൂന്യം *sqlite3_column_blob(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  സ്ഥിരം ശൂന്യം *val;
  val = sqlite3_മൂല്യം_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  തിരികെ val;
}
പൂർണ്ണസംഖ്യ sqlite3_column_bytes(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  പൂർണ്ണസംഖ്യ val = sqlite3_മൂല്യം_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
പൂർണ്ണസംഖ്യ sqlite3_column_bytes16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  പൂർണ്ണസംഖ്യ val = sqlite3_മൂല്യം_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
ഇരട്ട sqlite3_column_double(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  ഇരട്ട val = sqlite3_മൂല്യം_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
പൂർണ്ണസംഖ്യ sqlite3_column_int(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  പൂർണ്ണസംഖ്യ val = sqlite3_മൂല്യം_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  sqlite_int64 val = sqlite3_മൂല്യം_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
സ്ഥിരം unsigned അക്ഷരം *sqlite3_column_text(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  സ്ഥിരം unsigned അക്ഷരം *val = sqlite3_മൂല്യം_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
sqlite3_മൂല്യം *sqlite3_column_മൂല്യം(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  Mem *pOut = columnMem(pStmt, i);
  എങ്കിൽ( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  തിരികെ (sqlite3_മൂല്യം *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_column_text16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  സ്ഥിരം ശൂന്യം *val = sqlite3_മൂല്യം_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ val;
}
#endif /* SQLITE_OMIT_UTF16 */
പൂർണ്ണസംഖ്യ sqlite3_column_type(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  പൂർണ്ണസംഖ്യ iType = sqlite3_മൂല്യം_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  തിരികെ iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം * സ്ഥിരം azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
സ്ഥിതം സ്ഥിരം u16 azExplainColNames16data[] = {
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
സ്ഥിതം സ്ഥിരം u8 iExplainColNames16[] = {
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
സ്ഥിതം സ്ഥിരം ശൂന്യം *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  പൂർണ്ണസംഖ്യ N,                   /* Which column to get the name for */
  പൂർണ്ണസംഖ്യ useUtf16,            /* True to return the name as UTF16 */
  പൂർണ്ണസംഖ്യ useType              /* What type of name */
){
  സ്ഥിരം ശൂന്യം *ret;
  Vdbe *p;
  പൂർണ്ണസംഖ്യ n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pStmt==0 ){
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    തിരികെ 0;
  }
#endif
  എങ്കിൽ( N<0 ) തിരികെ 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  പ്രതിജ്ഞാ( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  എങ്കിൽ( p->explain ){
    എങ്കിൽ( useType>0 ) പോകുക columnName_end;
    n = p->explain==1 ? 8 : 4;
    എങ്കിൽ( N>=n ) പോകുക columnName_end;
    എങ്കിൽ( useUtf16 ){
      പൂർണ്ണസംഖ്യ i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (ശൂന്യം*)&azExplainColNames16data[i];
    }അല്ലെങ്കിൽ{
      ret = (ശൂന്യം*)azExplainColNames8[N + 8*p->explain - 8];
    }
    പോകുക columnName_end;
  }
  n = p->nResColumn;
  എങ്കിൽ( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    എങ്കിൽ( useUtf16 ){
      ret = sqlite3_മൂല്യം_text16((sqlite3_മൂല്യം*)&p->aColName[N]);
    }അല്ലെങ്കിൽ
#endif
    {
      ret = sqlite3_മൂല്യം_text((sqlite3_മൂല്യം*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    പ്രതിജ്ഞാ( db->mallocFailed==0 || db->mallocFailed==1 );
    എങ്കിൽ( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  തിരികെ ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
സ്ഥിരം അക്ഷരം *sqlite3_column_name(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_column_name16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#എങ്കിൽ defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
സ്ഥിരം അക്ഷരം *sqlite3_column_decltype(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_column_decltype16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
സ്ഥിരം അക്ഷരം *sqlite3_column_database_name(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_column_database_name16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
സ്ഥിരം അക്ഷരം *sqlite3_column_table_name(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_column_table_name16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
സ്ഥിരം അക്ഷരം *sqlite3_column_origin_name(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
സ്ഥിരം ശൂന്യം *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ N){
  തിരികെ columnName(pStmt, N, 1, COLNAME_COLUMN);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ vdbeUnbind(Vdbe *p, unsigned പൂർണ്ണസംഖ്യ i){
  Mem *pVar;
  എങ്കിൽ( vdbeSafetyNotNull(p) ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  എങ്കിൽ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    തിരികെ SQLITE_MISUSE_BKPT;
  }
  എങ്കിൽ( i>=(unsigned പൂർണ്ണസംഖ്യ)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    തിരികെ SQLITE_RANGE;
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
  പ്രതിജ്ഞാ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  പ്രതിജ്ഞാ( (p->expmask & p->smimask)==p->smimask );
  എങ്കിൽ( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    എങ്കിൽ( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  തിരികെ SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  പൂർണ്ണസംഖ്യ i,                 /* Index of the parameter to bind */
  സ്ഥിരം ശൂന്യം *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  ശൂന്യം (*xDel)(ശൂന്യം*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  പൂർണ്ണസംഖ്യ rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  എങ്കിൽ( rc==SQLITE_OK ){
    പ്രതിജ്ഞാ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    എങ്കിൽ( zData!=0 ){
      pVar = &p->aVar[i-1];
      എങ്കിൽ( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }അല്ലെങ്കിൽ എങ്കിൽ( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        പ്രതിജ്ഞാ( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }അല്ലെങ്കിൽ{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        എങ്കിൽ( encoding==0 ) pVar->enc = ENC(p->db);
      }
      എങ്കിൽ( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      എങ്കിൽ( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }അല്ലെങ്കിൽ എങ്കിൽ( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((ശൂന്യം*)zData);
  }
  തിരികെ rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
പൂർണ്ണസംഖ്യ sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ i,
  സ്ഥിരം ശൂന്യം *zData,
  പൂർണ്ണസംഖ്യ nData,
  ശൂന്യം (*xDel)(ശൂന്യം*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( nData<0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  തിരികെ bindText(pStmt, i, zData, nData, xDel, 0);
}
പൂർണ്ണസംഖ്യ sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ i,
  സ്ഥിരം ശൂന്യം *zData,
  sqlite3_uint64 nData,
  ശൂന്യം (*xDel)(ശൂന്യം*)
){
  പ്രതിജ്ഞാ( xDel!=SQLITE_DYNAMIC );
  തിരികെ bindText(pStmt, i, zData, nData, xDel, 0);
}
പൂർണ്ണസംഖ്യ sqlite3_bind_double(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i, ഇരട്ട r_മൂല്യം){
  പൂർണ്ണസംഖ്യ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  എങ്കിൽ( rc==SQLITE_OK ){
    പ്രതിജ്ഞാ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_മൂല്യം);
    sqlite3_mutex_leave(p->db->mutex);
  }
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3_bind_int(sqlite3_stmt *p, പൂർണ്ണസംഖ്യ i, പൂർണ്ണസംഖ്യ i_മൂല്യം){
  തിരികെ sqlite3_bind_int64(p, i, (i64)i_മൂല്യം);
}
പൂർണ്ണസംഖ്യ sqlite3_bind_int64(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i, sqlite_int64 i_മൂല്യം){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  എങ്കിൽ( vdbeSafetyNotNull(p) ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  എങ്കിൽ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    തിരികെ SQLITE_MISUSE_BKPT;
  }
  എങ്കിൽ( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    തിരികെ SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  എങ്കിൽ( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    പ്രതിജ്ഞാ( (p->expmask & p->smimask)==p->smimask );
    എങ്കിൽ( (p->expmask & expireMask)!=0 ){
      എങ്കിൽ( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        പൂർണ്ണസംഖ്യ sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        പൂർണ്ണസംഖ്യ sm2 = i_മൂല്യം>=0 && i_മൂല്യം<=1;
        എങ്കിൽ( sm1!=sm2 ){
          p->expired = 1;
        }
      }അല്ലെങ്കിൽ എങ്കിൽ( (pVar->flags & MEM_Int)==0 || i_മൂല്യം!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_മൂല്യം);
  sqlite3_mutex_leave(p->db->mutex);
  തിരികെ SQLITE_OK;
}
പൂർണ്ണസംഖ്യ sqlite3_bind_null(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  പൂർണ്ണസംഖ്യ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  എങ്കിൽ( rc==SQLITE_OK ){
    പ്രതിജ്ഞാ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ i,
  ശൂന്യം *pPtr,
  സ്ഥിരം അക്ഷരം *zPTtype,
  ശൂന്യം (*xDestructor)(ശൂന്യം*)
){
  പൂർണ്ണസംഖ്യ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  എങ്കിൽ( rc==SQLITE_OK ){
    പ്രതിജ്ഞാ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }അല്ലെങ്കിൽ എങ്കിൽ( xDestructor ){
    xDestructor(pPtr);
  }
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ i,
  സ്ഥിരം അക്ഷരം *zData,
  പൂർണ്ണസംഖ്യ nData,
  ശൂന്യം (*xDel)(ശൂന്യം*)
){
  തിരികെ bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
പൂർണ്ണസംഖ്യ sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ i,
  സ്ഥിരം അക്ഷരം *zData,
  sqlite3_uint64 nData,
  ശൂന്യം (*xDel)(ശൂന്യം*),
  unsigned അക്ഷരം enc
){
  പ്രതിജ്ഞാ( xDel!=SQLITE_DYNAMIC );
  എങ്കിൽ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    എങ്കിൽ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  തിരികെ bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
പൂർണ്ണസംഖ്യ sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ i,
  സ്ഥിരം ശൂന്യം *zData,
  പൂർണ്ണസംഖ്യ n,
  ശൂന്യം (*xDel)(ശൂന്യം*)
){
  തിരികെ bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
പൂർണ്ണസംഖ്യ sqlite3_bind_മൂല്യം(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i, സ്ഥിരം sqlite3_മൂല്യം *p_മൂല്യം){
  പൂർണ്ണസംഖ്യ rc;
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type((sqlite3_മൂല്യം*)p_മൂല്യം) ){
    അവസ്ഥ SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_മൂല്യം->u.i);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_FLOAT: {
      പ്രതിജ്ഞാ( p_മൂല്യം->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_മൂല്യം->flags & MEM_Real) ? p_മൂല്യം->u.r : (ഇരട്ട)p_മൂല്യം->u.i
      );
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_BLOB: {
      എങ്കിൽ( p_മൂല്യം->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_മൂല്യം->u.nZero);
      }അല്ലെങ്കിൽ{
        rc = sqlite3_bind_blob(pStmt, i, p_മൂല്യം->z, p_മൂല്യം->n,SQLITE_TRANSIENT);
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_മൂല്യം->z, p_മൂല്യം->n, SQLITE_TRANSIENT,
                              p_മൂല്യം->enc);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      rc = sqlite3_bind_null(pStmt, i);
      നിർത്തുക;
    }
  }
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i, പൂർണ്ണസംഖ്യ n){
  പൂർണ്ണസംഖ്യ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  എങ്കിൽ( rc==SQLITE_OK ){
    പ്രതിജ്ഞാ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#അല്ലെങ്കിൽ
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i, sqlite3_uint64 n){
  പൂർണ്ണസംഖ്യ rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  എങ്കിൽ( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  തിരികെ rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
പൂർണ്ണസംഖ്യ sqlite3_bind_parameter_എണ്ണം(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  തിരികെ p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
സ്ഥിരം അക്ഷരം *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ i){
  Vdbe *p = (Vdbe*)pStmt;
  എങ്കിൽ( p==0 ) തിരികെ 0;
  തിരികെ sqlite3_VList_അക്കം_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeParameterIndex(Vdbe *p, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ nName){
  എങ്കിൽ( p==0 || zName==0 ) തിരികെ 0;
  തിരികെ sqlite3_VList_Name_To_അക്കം(p->pVList, zName, nName);
}
പൂർണ്ണസംഖ്യ sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, സ്ഥിരം അക്ഷരം *zName){
  തിരികെ sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
പൂർണ്ണസംഖ്യ sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( pTo->db==pFrom->db );
  പ്രതിജ്ഞാ( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  വേണ്ടി(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  എങ്കിൽ( pFrom->nVar!=pTo->nVar ){
    തിരികെ SQLITE_ERROR;
  }
  പ്രതിജ്ഞാ( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  എങ്കിൽ( pTo->expmask ){
    pTo->expired = 1;
  }
  പ്രതിജ്ഞാ( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  എങ്കിൽ( pFrom->expmask ){
    pFrom->expired = 1;
  }
  തിരികെ sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  തിരികെ pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  തിരികെ pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  തിരികെ pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_explain(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ eMode){
  Vdbe *v = (Vdbe*)pStmt;
  പൂർണ്ണസംഖ്യ rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pStmt==0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  എങ്കിൽ( ((പൂർണ്ണസംഖ്യ)v->explain)==eMode ){
    rc = SQLITE_OK;
  }അല്ലെങ്കിൽ എങ്കിൽ( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ എങ്കിൽ( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ എങ്കിൽ( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }അല്ലെങ്കിൽ എങ്കിൽ( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    എങ്കിൽ( rc ){
      v->explain = savedMode;
    }അല്ലെങ്കിൽ{
      v->haveEqpOps = eMode==2;
    }
  }
  എങ്കിൽ( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }അല്ലെങ്കിൽ{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  തിരികെ rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  തിരികെ v!=0 && v->eVdbeState==VDBE_RUN_STATE;
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
  എങ്കിൽ( !sqlite3SafetyCheckOk(pDb) ){
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    തിരികെ 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  എങ്കിൽ( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }അല്ലെങ്കിൽ{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  തിരികെ pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_status(sqlite3_stmt *pStmt, പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    തിരികെ 0;
  }
#endif
  എങ്കിൽ( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (പൂർണ്ണസംഖ്യ*)&v;
    പ്രതിജ്ഞാ( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }അല്ലെങ്കിൽ{
    v = pVdbe->aCounter[op];
    എങ്കിൽ( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  തിരികെ (പൂർണ്ണസംഖ്യ)v;
}

/*
** Return the SQL associated with a prepared statement
*/
സ്ഥിരം അക്ഷരം *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  തിരികെ p ? p->zSql : 0;
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
അക്ഷരം *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  തിരികെ 0;
#അല്ലെങ്കിൽ
  അക്ഷരം *z = 0;
  സ്ഥിരം അക്ഷരം *zSql = sqlite3_sql(pStmt);
  എങ്കിൽ( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  തിരികെ z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
സ്ഥിരം അക്ഷരം *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  എങ്കിൽ( p==0 ) തിരികെ 0;
  എങ്കിൽ( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  തിരികെ p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
സ്ഥിതം UnpackedRecord *vdbeUnpackRecord(
  താക്കോൽ_Info *p_താക്കോൽ_Info,
  പൂർണ്ണസംഖ്യ n_താക്കോൽ,
  സ്ഥിരം ശൂന്യം *p_താക്കോൽ
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_താക്കോൽ_Info);
  എങ്കിൽ( pRet ){
    memset(pRet->aMem, 0, വലുപ്പം(Mem)*(p_താക്കോൽ_Info->n_താക്കോൽ_Field+1));
    sqlite3VdbeRecordUnpack(n_താക്കോൽ, p_താക്കോൽ, pRet);
  }
  തിരികെ pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
പൂർണ്ണസംഖ്യ sqlite3_preupdate_old(sqlite3 *db, പൂർണ്ണസംഖ്യ iIdx, sqlite3_മൂല്യം **pp_മൂല്യം){
  PreUpdate *p;
  Mem *pMem;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( db==0 || pp_മൂല്യം==0 ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  എങ്കിൽ( !p || p->op==SQLITE_തിരുകുക ){
    rc = SQLITE_MISUSE_BKPT;
    പോകുക preupdate_old_out;
  }
  എങ്കിൽ( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }അല്ലെങ്കിൽ എങ്കിൽ( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    പോകുക preupdate_old_out;
  }അല്ലെങ്കിൽ{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  എങ്കിൽ( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    പോകുക preupdate_old_out;
  }

  എങ്കിൽ( iIdx==p->pTab->iPKey ){
    *pp_മൂല്യം = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }അല്ലെങ്കിൽ{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    എങ്കിൽ( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      പ്രതിജ്ഞാ( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      എങ്കിൽ( !aRec ) പോകുക preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      എങ്കിൽ( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        എങ്കിൽ( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      എങ്കിൽ( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        പോകുക preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_മൂല്യം = &p->pUnpacked->aMem[iStore];
    എങ്കിൽ( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      എങ്കിൽ( pCol->iDflt>0 ){
        എങ്കിൽ( p->apDflt==0 ){
          പൂർണ്ണസംഖ്യ nByte;
          പ്രതിജ്ഞാ( വലുപ്പം(sqlite3_മൂല്യം*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = വലുപ്പം(sqlite3_മൂല്യം*)*p->pTab->nCol;
          p->apDflt = (sqlite3_മൂല്യം**)sqlite3DbMallocZero(db, nByte);
          എങ്കിൽ( p->apDflt==0 ) പോകുക preupdate_old_out;
        }
        എങ്കിൽ( p->apDflt[iIdx]==0 ){
          sqlite3_മൂല്യം *pVal = 0;
          വ്യഞ്ജകം *pDflt;
          പ്രതിജ്ഞാ( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_വ്യഞ്ജകം;
          rc = sqlite3_മൂല്യം_From_വ്യഞ്ജകം(db, pDflt, ENC(db), pCol->affinity, &pVal);
          എങ്കിൽ( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_മൂല്യം = p->apDflt[iIdx];
      }അല്ലെങ്കിൽ{
        *pp_മൂല്യം = (sqlite3_മൂല്യം *)column_Null_മൂല്യം();
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      എങ്കിൽ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  തിരികെ sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
പൂർണ്ണസംഖ്യ sqlite3_preupdate_എണ്ണം(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#അല്ലെങ്കിൽ
  p = db->pPreUpdate;
#endif
  തിരികെ (p ? p->pKeyinfo->n_താക്കോൽ_Field : 0);
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
പൂർണ്ണസംഖ്യ sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#അല്ലെങ്കിൽ
  p = db->pPreUpdate;
#endif
  തിരികെ (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
പൂർണ്ണസംഖ്യ sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#അല്ലെങ്കിൽ
  p = db->pPreUpdate;
#endif
  തിരികെ (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
പൂർണ്ണസംഖ്യ sqlite3_preupdate_new(sqlite3 *db, പൂർണ്ണസംഖ്യ iIdx, sqlite3_മൂല്യം **pp_മൂല്യം){
  PreUpdate *p;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  Mem *pMem;
  പൂർണ്ണസംഖ്യ iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( db==0 || pp_മൂല്യം==0 ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  എങ്കിൽ( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    പോകുക preupdate_new_out;
  }
  എങ്കിൽ( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }അല്ലെങ്കിൽ എങ്കിൽ( iIdx >= p->pTab->nCol ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }അല്ലെങ്കിൽ{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  എങ്കിൽ( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    പോകുക preupdate_new_out;
  }

  എങ്കിൽ( p->op==SQLITE_തിരുകുക ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    എങ്കിൽ( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      എങ്കിൽ( rc!=SQLITE_OK ) പോകുക preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      എങ്കിൽ( !pUnpack ){
        rc = SQLITE_NOMEM;
        പോകുക preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    എങ്കിൽ( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }അല്ലെങ്കിൽ എങ്കിൽ( iStore>=pUnpack->nField ){
      pMem = (sqlite3_മൂല്യം *)column_Null_മൂല്യം();
    }അല്ലെങ്കിൽ എങ്കിൽ( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }അല്ലെങ്കിൽ{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    പ്രതിജ്ഞാ( p->op==SQLITE_UPDATE );
    എങ്കിൽ( !p->aNew ){
      പ്രതിജ്ഞാ( വലുപ്പം(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, വലുപ്പം(Mem)*p->pCsr->nField);
      എങ്കിൽ( !p->aNew ){
        rc = SQLITE_NOMEM;
        പോകുക preupdate_new_out;
      }
    }
    പ്രതിജ്ഞാ( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    എങ്കിൽ( pMem->flags==0 ){
      എങ്കിൽ( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }അല്ലെങ്കിൽ{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        എങ്കിൽ( rc!=SQLITE_OK ) പോകുക preupdate_new_out;
      }
    }
  }
  *pp_മൂല്യം = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  തിരികെ sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  പൂർണ്ണസംഖ്യ iScan,                      /* Index of loop to report on */
  പൂർണ്ണസംഖ്യ iScanStatusOp,              /* Which metric to return */
  പൂർണ്ണസംഖ്യ flags,
  ശൂന്യം *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  പൂർണ്ണസംഖ്യ nOp;
  ScanStatus *pScan = 0;
  പൂർണ്ണസംഖ്യ idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    തിരികെ 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  എങ്കിൽ( p->pFrame ){
    VdbeFrame *pFrame;
    വേണ്ടി(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  എങ്കിൽ( iScan<0 ){
    പൂർണ്ണസംഖ്യ ii;
    എങ്കിൽ( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      വേണ്ടി(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      തിരികെ 0;
    }
    തിരികെ 1;
  }
  എങ്കിൽ( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }അല്ലെങ്കിൽ{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    വേണ്ടി(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      എങ്കിൽ( pScan->zName ){
        iScan--;
        എങ്കിൽ( iScan<0 ) നിർത്തുക;
      }
    }
  }
  എങ്കിൽ( idx>=p->nScan ) തിരികെ 1;
  പ്രതിജ്ഞാ( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  തിരഞ്ഞെടുപ്പ്( iScanStatusOp ){
    അവസ്ഥ SQLITE_SCANSTAT_NLOOP: {
      എങ്കിൽ( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }അല്ലെങ്കിൽ{
        *(sqlite3_int64*)pOut = -1;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_NVISIT: {
      എങ്കിൽ( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }അല്ലെങ്കിൽ{
        *(sqlite3_int64*)pOut = -1;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_EST: {
      ഇരട്ട r = 1.0;
      LogEst x = pScan->nEst;
      ആകുമ്പോൾ( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(ഇരട്ട*)pOut = r*sqlite3LogEstToInt(x);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_NAME: {
      *(സ്ഥിരം അക്ഷരം**)pOut = pScan->zName;
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_EXPLAIN: {
      എങ്കിൽ( pScan->addrExplain ){
        *(സ്ഥിരം അക്ഷരം**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }അല്ലെങ്കിൽ{
        *(സ്ഥിരം അക്ഷരം**)pOut = 0;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_SELECTID: {
      എങ്കിൽ( pScan->addrExplain ){
        *(പൂർണ്ണസംഖ്യ*)pOut = aOp[ pScan->addrExplain ].p1;
      }അല്ലെങ്കിൽ{
        *(പൂർണ്ണസംഖ്യ*)pOut = -1;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_PARENTID: {
      എങ്കിൽ( pScan->addrExplain ){
        *(പൂർണ്ണസംഖ്യ*)pOut = aOp[ pScan->addrExplain ].p2;
      }അല്ലെങ്കിൽ{
        *(പൂർണ്ണസംഖ്യ*)pOut = -1;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      എങ്കിൽ( pScan->aAddrRange[0]==0 ){
        res = -1;
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ ii;
        വേണ്ടി(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          പൂർണ്ണസംഖ്യ iIns = pScan->aAddrRange[ii];
          പൂർണ്ണസംഖ്യ iEnd = pScan->aAddrRange[ii+1];
          എങ്കിൽ( iIns==0 ) നിർത്തുക;
          എങ്കിൽ( iIns>0 ){
            ആകുമ്പോൾ( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }അല്ലെങ്കിൽ{
            പൂർണ്ണസംഖ്യ iOp;
            വേണ്ടി(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              എങ്കിൽ( pOp->p1!=iEnd ) തുടരുക;
              എങ്കിൽ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                തുടരുക;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      തിരികെ 1;
    }
  }
  തിരികെ 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
പൂർണ്ണസംഖ്യ sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  പൂർണ്ണസംഖ്യ iScan,                      /* Index of loop to report on */
  പൂർണ്ണസംഖ്യ iScanStatusOp,              /* Which metric to return */
  ശൂന്യം *pOut                      /* OUT: Write the answer here */
){
  തിരികെ sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
ശൂന്യം sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  പൂർണ്ണസംഖ്യ ii;
  വേണ്ടി(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
