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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "vdbeInt.h"
#અન્તર્ભાવ "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
પૂર્ણાંક sqlite3_expired(sqlite3_stmt *pStmt){
  પૂર્ણાંક iRet = 1;
  જો( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  પાછા iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
સ્થિર પૂર્ણાંક vdbeSafety(Vdbe *p){
  જો( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    પાછા 1;
  }નહીંતર{
    પાછા 0;
  }
}
સ્થિર પૂર્ણાંક vdbeSafetyNotNull(Vdbe *p){
  જો( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    પાછા 1;
  }નહીંતર{
    પાછા vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  પ્રતિજ્ઞા( p->startTime>0 );
  પ્રતિજ્ઞા( db->init.busy==0 );
  પ્રતિજ્ઞા( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  જો( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  જો( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (શૂન્ય*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   જો( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#નહીંતર
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
પૂર્ણાંક sqlite3_finalize(sqlite3_stmt *pStmt){
  પૂર્ણાંક rc;
  જો( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }નહીંતર{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    જો( vdbeSafety(v) ) પાછા SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    પ્રતિજ્ઞા( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  પાછા rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
પૂર્ણાંક sqlite3_reset(sqlite3_stmt *pStmt){
  પૂર્ણાંક rc;
  જો( pStmt==0 ){
    rc = SQLITE_OK;
  }નહીંતર{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    પ્રતિજ્ઞા( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  પાછા rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
પૂર્ણાંક sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  પૂર્ણાંક i;
  પૂર્ણાંક rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#જો SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pStmt==0 ){
    પાછા SQLITE_MISUSE_BKPT;
  }
#endif
#જો SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  માટે(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    જો( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  પ્રતિજ્ઞા( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  જો( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  પાછા rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
અચલ શૂન્ય *sqlite3_કિંમત_blob(sqlite3_કિંમત *pVal){
  Mem *p = (Mem*)pVal;
  જો( p->flags & (MEM_Blob|MEM_Str) ){
    જો( ExpandBlob(p)!=SQLITE_OK ){
      પ્રતિજ્ઞા( p->flags==MEM_Null && p->z==0 );
      પાછા 0;
    }
    p->flags |= MEM_Blob;
    પાછા p->n ? p->z : 0;
  }નહીંતર{
    પાછા sqlite3_કિંમત_text(pVal);
  }
}
પૂર્ણાંક sqlite3_કિંમત_bytes(sqlite3_કિંમત *pVal){
  પાછા sqlite3_કિંમત_Bytes(pVal, SQLITE_UTF8);
}
પૂર્ણાંક sqlite3_કિંમત_bytes16(sqlite3_કિંમત *pVal){
  પાછા sqlite3_કિંમત_Bytes(pVal, SQLITE_UTF16NATIVE);
}
દ્વિગુણ sqlite3_કિંમત_double(sqlite3_કિંમત *pVal){
  પાછા sqlite3_Vdbe_Real_કિંમત((Mem*)pVal);
}
પૂર્ણાંક sqlite3_કિંમત_int(sqlite3_કિંમત *pVal){
  પાછા (પૂર્ણાંક)sqlite3_Vdbe_Int_કિંમત((Mem*)pVal);
}
sqlite_int64 sqlite3_કિંમત_int64(sqlite3_કિંમત *pVal){
  પાછા sqlite3_Vdbe_Int_કિંમત((Mem*)pVal);
}
unsigned પૂર્ણાંક sqlite3_કિંમત_subtype(sqlite3_કિંમત *pVal){
  Mem *pMem = (Mem*)pVal;
  પાછા ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
શૂન્ય *sqlite3_કિંમત_pointer(sqlite3_કિંમત *pVal, અચલ અક્ષર *zPType){
  Mem *p = (Mem*)pVal;
  જો( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    પાછા (શૂન્ય*)p->z;
  }નહીંતર{
    પાછા 0;
  }
}
અચલ unsigned અક્ષર *sqlite3_કિંમત_text(sqlite3_કિંમત *pVal){
  પાછા (અચલ unsigned અક્ષર *)sqlite3_કિંમત_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_કિંમત_text16(sqlite3_કિંમત* pVal){
  પાછા sqlite3_કિંમત_Text(pVal, SQLITE_UTF16NATIVE);
}
અચલ શૂન્ય *sqlite3_કિંમત_text16be(sqlite3_કિંમત *pVal){
  પાછા sqlite3_કિંમત_Text(pVal, SQLITE_UTF16BE);
}
અચલ શૂન્ય *sqlite3_કિંમત_text16le(sqlite3_કિંમત *pVal){
  પાછા sqlite3_કિંમત_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
પૂર્ણાંક sqlite3_કિંમત_type(sqlite3_કિંમત* pVal){
  સ્થિર અચલ u8 aType[] = {
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
    પૂર્ણાંક eType = SQLITE_BLOB;
    જો( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }નહીંતર જો( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }નહીંતર જો( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }નહીંતર જો( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    પ્રતિજ્ઞા( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  પાછા aType[pVal->flags&MEM_AffMask];
}
પૂર્ણાંક sqlite3_કિંમત_encoding(sqlite3_કિંમત *pVal){
  પાછા pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
પૂર્ણાંક sqlite3_કિંમત_nochange(sqlite3_કિંમત *pVal){
  પાછા (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
પૂર્ણાંક sqlite3_કિંમત_frombind(sqlite3_કિંમત *pVal){
  પાછા (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_કિંમત *sqlite3_કિંમત_dup(અચલ sqlite3_કિંમત *pOrig){
  sqlite3_કિંમત *pNew;
  જો( pOrig==0 ) પાછા 0;
  pNew = sqlite3_malloc( કદ(*pNew) );
  જો( pNew==0 ) પાછા 0;
  memset(pNew, 0, કદ(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  જો( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    જો( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_કિંમત_Free(pNew);
      pNew = 0;
    }
  }નહીંતર જો( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  પાછા pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
શૂન્ય sqlite3_કિંમત_free(sqlite3_કિંમત *pOld){
  sqlite3_કિંમત_Free(pOld);
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
સ્થિર શૂન્ય setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  અચલ અક્ષર *z,          /* String pointer */
  પૂર્ણાંક n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  શૂન્ય (*xDel)(શૂન્ય*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  પૂર્ણાંક rc;
  જો( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }નહીંતર જો( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    પ્રતિજ્ઞા( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }નહીંતર{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  જો( rc ){
    જો( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }નહીંતર{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      પ્રતિજ્ઞા( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    પાછા;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  જો( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
સ્થિર પૂર્ણાંક invoke_કિંમત_Destructor(
  અચલ શૂન્ય *p,             /* Value to destroy */
  શૂન્ય (*xDel)(શૂન્ય*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  પ્રતિજ્ઞા( xDel!=SQLITE_DYNAMIC );
  જો( xDel==0 ){
    /* noop */
  }નહીંતર જો( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }નહીંતર{
    xDel((શૂન્ય*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#નહીંતર
  પ્રતિજ્ઞા( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  પાછા SQLITE_TOOBIG;
}
શૂન્ય sqlite3_result_blob(
  sqlite3_context *pCtx,
  અચલ શૂન્ય *z,
  પૂર્ણાંક n,
  શૂન્ય (*xDel)(શૂન્ય *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 || n<0 ){
    invoke_કિંમત_Destructor(z, xDel, pCtx);
    પાછા;
  }
#endif
  પ્રતિજ્ઞા( n>=0 );
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
શૂન્ય sqlite3_result_blob64(
  sqlite3_context *pCtx,
  અચલ શૂન્ય *z,
  sqlite3_uint64 n,
  શૂન્ય (*xDel)(શૂન્ય *)
){
  પ્રતિજ્ઞા( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ){
    invoke_કિંમત_Destructor(z, xDel, 0);
    પાછા;
  }
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  જો( n>0x7fffffff ){
    (શૂન્ય)invoke_કિંમત_Destructor(z, xDel, pCtx);
  }નહીંતર{
    setResultStrOrError(pCtx, z, (પૂર્ણાંક)n, 0, xDel);
  }
}
શૂન્ય sqlite3_result_double(sqlite3_context *pCtx, દ્વિગુણ rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
શૂન્ય sqlite3_result_error(sqlite3_context *pCtx, અચલ અક્ષર *z, પૂર્ણાંક n){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
શૂન્ય sqlite3_result_error16(sqlite3_context *pCtx, અચલ શૂન્ય *z, પૂર્ણાંક n){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
શૂન્ય sqlite3_result_int(sqlite3_context *pCtx, પૂર્ણાંક iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
શૂન્ય sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
શૂન્ય sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
શૂન્ય sqlite3_result_pointer(
  sqlite3_context *pCtx,
  શૂન્ય *pPtr,
  અચલ અક્ષર *zPType,
  શૂન્ય (*xDestructor)(શૂન્ય*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ){
    invoke_કિંમત_Destructor(pPtr, xDestructor, 0);
    પાછા;
  }
#endif
  pOut = pCtx->pOut;
  પ્રતિજ્ઞા( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
શૂન્ય sqlite3_result_subtype(sqlite3_context *pCtx, unsigned પૂર્ણાંક eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
#જો defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  જો( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    અક્ષર zErr[200];
    sqlite3_snprintf(કદ(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    પાછા;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  પ્રતિજ્ઞા( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
શૂન્ય sqlite3_result_text(
  sqlite3_context *pCtx,
  અચલ અક્ષર *z,
  પૂર્ણાંક n,
  શૂન્ય (*xDel)(શૂન્ય *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ){
    invoke_કિંમત_Destructor(z, xDel, 0);
    પાછા;
  }
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
શૂન્ય sqlite3_result_text64(
  sqlite3_context *pCtx,
  અચલ અક્ષર *z,
  sqlite3_uint64 n,
  શૂન્ય (*xDel)(શૂન્ય *),
  unsigned અક્ષર enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ){
    invoke_કિંમત_Destructor(z, xDel, 0);
    પાછા;
  }
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  પ્રતિજ્ઞા( xDel!=SQLITE_DYNAMIC );
  જો( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    જો( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  જો( n>0x7fffffff ){
    (શૂન્ય)invoke_કિંમત_Destructor(z, xDel, pCtx);
  }નહીંતર{
    setResultStrOrError(pCtx, z, (પૂર્ણાંક)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
શૂન્ય sqlite3_result_text16(
  sqlite3_context *pCtx,
  અચલ શૂન્ય *z,
  પૂર્ણાંક n,
  શૂન્ય (*xDel)(શૂન્ય *)
){
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
શૂન્ય sqlite3_result_text16be(
  sqlite3_context *pCtx,
  અચલ શૂન્ય *z,
  પૂર્ણાંક n,
  શૂન્ય (*xDel)(શૂન્ય *)
){
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
શૂન્ય sqlite3_result_text16le(
  sqlite3_context *pCtx,
  અચલ શૂન્ય *z,
  પૂર્ણાંક n,
  શૂન્ય (*xDel)(શૂન્ય *)
){
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
શૂન્ય sqlite3_result_કિંમત(sqlite3_context *pCtx, sqlite3_કિંમત *p_કિંમત){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
  જો( p_કિંમત==0 ){
    sqlite3_result_null(pCtx);
    પાછા;
  }
#endif
  pOut = pCtx->pOut;
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_કિંમત);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  જો( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
શૂન્ય sqlite3_result_zeroblob(sqlite3_context *pCtx, પૂર્ણાંક n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
પૂર્ણાંક sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  પ્રતિજ્ઞા( sqlite3_mutex_held(pOut->db->mutex) );
  જો( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    પાછા SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (પૂર્ણાંક)n);
  પાછા SQLITE_OK;
#નહીંતર
  પાછા sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (પૂર્ણાંક)n);
#endif
}
શૂન્ય sqlite3_result_error_code(sqlite3_context *pCtx, પૂર્ણાંક errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  જો( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  જો( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
શૂન્ય sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
શૂન્ય sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
શૂન્ય sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, પૂર્ણાંક eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
  જો( pStr==0 ) પાછા;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  જો( pStr->accError==0 ){
    જો( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }નહીંતર{
      અચલ અક્ષર *zText = sqlite3_str_કિંમત(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      પ્રતિજ્ઞા( isMalloced(pStr) );
      જો( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }નહીંતર{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }નહીંતર જો( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }નહીંતર{
    પ્રતિજ્ઞા( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  જો( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
શૂન્ય sqlite3ResultIntReal(sqlite3_context *pCtx){
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  જો( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
સ્થિર પૂર્ણાંક doWalCallbacks(sqlite3 *db){
  પૂર્ણાંક rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  પૂર્ણાંક i;
  માટે(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    જો( pBt ){
      પૂર્ણાંક nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      જો( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#નહીંતર
  UNUSED_PARAMETER(db);
#endif
  પાછા rc;
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
સ્થિર પૂર્ણાંક sqlite3Step(Vdbe *p){
  sqlite3 *db;
  પૂર્ણાંક rc;

  પ્રતિજ્ઞા(p);
  db = p->db;
  જો( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    જો( p->eVdbeState==VDBE_READY_STATE ){
      જો( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        જો( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        જાઓ end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      જો( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      પ્રતિજ્ઞા( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      જો( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }નહીંતર{
        પ્રતિજ્ઞા( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      જો( p->readOnly==0 ) db->nVdbeWrite++;
      જો( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }નહીંતર

    જો( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      જો( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }નહીંતર{
        પાછા SQLITE_MISUSE_BKPT;
      }
#નહીંતર
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      પ્રતિજ્ઞા( p->eVdbeState==VDBE_READY_STATE );
      જાઓ restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  જો( p->explain ){
    rc = sqlite3VdbeList(p);
  }નહીંતર
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  જો( rc==SQLITE_ROW ){
    પ્રતિજ્ઞા( p->rc==SQLITE_OK );
    પ્રતિજ્ઞા( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    પાછા SQLITE_ROW;
  }નહીંતર{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    જો( rc==SQLITE_DONE && db->autoCommit ){
      પ્રતિજ્ઞા( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      જો( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }નહીંતર જો( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  જો( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    જો( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  પ્રતિજ્ઞા( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  પાછા (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
પૂર્ણાંક sqlite3_step(sqlite3_stmt *pStmt){
  પૂર્ણાંક rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  પૂર્ણાંક cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  જો( vdbeSafetyNotNull(v) ){
    પાછા SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  જ્યારે( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    પૂર્ણાંક savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    જો( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      અચલ અક્ષર *zErr = (અચલ અક્ષર *)sqlite3_કિંમત_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_સંદેશ);
      જો( !db->mallocFailed ){
        v->z_Err_સંદેશ = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } નહીંતર {
        v->z_Err_સંદેશ = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      તોડો;
    }
    sqlite3_reset(pStmt);
    જો( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    પ્રતિજ્ઞા( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  પાછા rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
શૂન્ય *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( p==0 ) પાછા 0;
#endif
  પ્રતિજ્ઞા( p && p->pFunc );
  પાછા p->pFunc->pUserData;
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
  જો( p==0 ) પાછા 0;
#નહીંતર
  પ્રતિજ્ઞા( p && p->pOut );
#endif
  પાછા p->pOut->db;
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
પૂર્ણાંક sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( p==0 ) પાછા 0;
#નહીંતર
  પ્રતિજ્ઞા( p );
#endif
  પાછા sqlite3_કિંમત_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
શૂન્ય sqlite3_Vdbe_કિંમત_List_Free(શૂન્ય *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
સ્થિર પૂર્ણાંક કિંમત_From_કિંમત_List(
  sqlite3_કિંમત *pVal,        /* Pointer to the ValueList object */
  sqlite3_કિંમત **ppOut,      /* Store the next value from the list here */
  પૂર્ણાંક bNext                   /* 1 for _next(). 0 for _first() */
){
  પૂર્ણાંક rc;
  કિંમત_List *pRhs;

  *ppOut = 0;
  જો( pVal==0 ) પાછા SQLITE_MISUSE_BKPT;
  જો( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_કિંમત_List_Free ){
    પાછા SQLITE_ERROR;
  }નહીંતર{
    પ્રતિજ્ઞા( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    પ્રતિજ્ઞા( pVal->eSubtype=='p' );
    પ્રતિજ્ઞા( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (કિંમત_List*)pVal->z;
  }
  જો( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }નહીંતર{
    પૂર્ણાંક dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    પ્રતિજ્ઞા( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    જો( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  જો( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, કદ(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    જો( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_કિંમત *pOut = pRhs->pOut;
      પૂર્ણાંક iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      જો( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }નહીંતર{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  પાછા rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
પૂર્ણાંક sqlite3_vtab_in_first(sqlite3_કિંમત *pVal, sqlite3_કિંમત **ppOut){
  પાછા કિંમત_From_કિંમત_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
પૂર્ણાંક sqlite3_vtab_in_next(sqlite3_કિંમત *pVal, sqlite3_કિંમત **ppOut){
  પાછા કિંમત_From_કિંમત_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  પૂર્ણાંક rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  પ્રતિજ્ઞા( p->pVdbe!=0 );
#નહીંતર
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  જો( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    જો( rc ) *piTime = 0;
  }
  પાછા *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય *createAggContext(sqlite3_context *p, પૂર્ણાંક nByte){
  Mem *pMem = p->pMem;
  પ્રતિજ્ઞા( (pMem->flags & MEM_Agg)==0 );
  જો( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }નહીંતર{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    જો( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  પાછા (શૂન્ય*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
શૂન્ય *sqlite3_aggregate_context(sqlite3_context *p, પૂર્ણાંક nByte){
  પ્રતિજ્ઞા( p && p->pFunc && p->pFunc->xFinalize );
  પ્રતિજ્ઞા( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  જો( (p->pMem->flags & MEM_Agg)==0 ){
    પાછા createAggContext(p, nByte);
  }નહીંતર{
    પાછા (શૂન્ય*)p->pMem->z;
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
શૂન્ય *sqlite3_get_auxdata(sqlite3_context *pCtx, પૂર્ણાંક iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા 0;
#endif
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#જો SQLITE_ENABLE_STAT4
  જો( pCtx->pVdbe==0 ) પાછા 0;
#નહીંતર
  પ્રતિજ્ઞા( pCtx->pVdbe!=0 );
#endif
  માટે(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    જો(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      પાછા pAuxData->pAux;
    }
  }
  પાછા 0;
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
શૂન્ય sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  પૂર્ણાંક iArg,
  શૂન્ય *pAux,
  શૂન્ય (*xDelete)(શૂન્ય*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pCtx==0 ) પાછા;
#endif
  pVdbe= pCtx->pVdbe;
  પ્રતિજ્ઞા( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  જો( pVdbe==0 ) જાઓ failed;
#નહીંતર
  પ્રતિજ્ઞા( pVdbe!=0 );
#endif

  માટે(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    જો( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      તોડો;
    }
  }
  જો( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, કદ(AuxData));
    જો( !pAuxData ) જાઓ failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    જો( pCtx->isError==0 ) pCtx->isError = -1;
  }નહીંતર જો( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  પાછા;

failed:
  જો( xDelete ){
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
પૂર્ણાંક sqlite3_aggregate_ગણતરી(sqlite3_context *p){
  પ્રતિજ્ઞા( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  પાછા p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
પૂર્ણાંક sqlite3_column_ગણતરી(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  જો( pVm==0 ) પાછા 0;
  પાછા pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
પૂર્ણાંક sqlite3_data_ગણતરી(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  જો( pVm==0 || pVm->pResultRow==0 ) પાછા 0;
  પાછા pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
સ્થિર અચલ Mem *column_Null_કિંમત(શૂન્ય){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  સ્થિર અચલ Mem nullMem
#જો defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (અક્ષર*)0,
        /* .n          = */ (પૂર્ણાંક)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (પૂર્ણાંક)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (અક્ષર*)0,
        /* .xDel       = */ (શૂન્ય(*)(શૂન્ય*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  પાછા &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
સ્થિર Mem *columnMem(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  જો( pVm==0 ) પાછા (Mem*)column_Null_કિંમત();
  પ્રતિજ્ઞા( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  જો( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }નહીંતર{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_કિંમત();
  }
  પાછા pOut;
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
સ્થિર શૂન્ય columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  જો( p ){
    પ્રતિજ્ઞા( p->db!=0 );
    પ્રતિજ્ઞા( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
અચલ શૂન્ય *sqlite3_column_blob(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  અચલ શૂન્ય *val;
  val = sqlite3_કિંમત_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  પાછા val;
}
પૂર્ણાંક sqlite3_column_bytes(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  પૂર્ણાંક val = sqlite3_કિંમત_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
પૂર્ણાંક sqlite3_column_bytes16(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  પૂર્ણાંક val = sqlite3_કિંમત_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
દ્વિગુણ sqlite3_column_double(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  દ્વિગુણ val = sqlite3_કિંમત_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
પૂર્ણાંક sqlite3_column_int(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  પૂર્ણાંક val = sqlite3_કિંમત_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  sqlite_int64 val = sqlite3_કિંમત_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
અચલ unsigned અક્ષર *sqlite3_column_text(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  અચલ unsigned અક્ષર *val = sqlite3_કિંમત_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
sqlite3_કિંમત *sqlite3_column_કિંમત(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  Mem *pOut = columnMem(pStmt, i);
  જો( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  પાછા (sqlite3_કિંમત *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_column_text16(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  અચલ શૂન્ય *val = sqlite3_કિંમત_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા val;
}
#endif /* SQLITE_OMIT_UTF16 */
પૂર્ણાંક sqlite3_column_type(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  પૂર્ણાંક iType = sqlite3_કિંમત_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  પાછા iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
સ્થિર અચલ અક્ષર * અચલ azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
સ્થિર અચલ u16 azExplainColNames16data[] = {
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
સ્થિર અચલ u8 iExplainColNames16[] = {
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
સ્થિર અચલ શૂન્ય *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  પૂર્ણાંક N,                   /* Which column to get the name for */
  પૂર્ણાંક useUtf16,            /* True to return the name as UTF16 */
  પૂર્ણાંક useType              /* What type of name */
){
  અચલ શૂન્ય *ret;
  Vdbe *p;
  પૂર્ણાંક n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pStmt==0 ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
  જો( N<0 ) પાછા 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  પ્રતિજ્ઞા( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  જો( p->explain ){
    જો( useType>0 ) જાઓ columnName_end;
    n = p->explain==1 ? 8 : 4;
    જો( N>=n ) જાઓ columnName_end;
    જો( useUtf16 ){
      પૂર્ણાંક i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (શૂન્ય*)&azExplainColNames16data[i];
    }નહીંતર{
      ret = (શૂન્ય*)azExplainColNames8[N + 8*p->explain - 8];
    }
    જાઓ columnName_end;
  }
  n = p->nResColumn;
  જો( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    જો( useUtf16 ){
      ret = sqlite3_કિંમત_text16((sqlite3_કિંમત*)&p->aColName[N]);
    }નહીંતર
#endif
    {
      ret = sqlite3_કિંમત_text((sqlite3_કિંમત*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    પ્રતિજ્ઞા( db->mallocFailed==0 || db->mallocFailed==1 );
    જો( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  પાછા ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
અચલ અક્ષર *sqlite3_column_name(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_column_name16(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#જો defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
અચલ અક્ષર *sqlite3_column_decltype(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_column_decltype16(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
અચલ અક્ષર *sqlite3_column_database_name(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_column_database_name16(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
અચલ અક્ષર *sqlite3_column_table_name(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_column_table_name16(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
અચલ અક્ષર *sqlite3_column_origin_name(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
અચલ શૂન્ય *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, પૂર્ણાંક N){
  પાછા columnName(pStmt, N, 1, COLNAME_COLUMN);
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
સ્થિર પૂર્ણાંક vdbeUnbind(Vdbe *p, unsigned પૂર્ણાંક i){
  Mem *pVar;
  જો( vdbeSafetyNotNull(p) ){
    પાછા SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  જો( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    પાછા SQLITE_MISUSE_BKPT;
  }
  જો( i>=(unsigned પૂર્ણાંક)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    પાછા SQLITE_RANGE;
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
  પ્રતિજ્ઞા( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  પ્રતિજ્ઞા( (p->expmask & p->smimask)==p->smimask );
  જો( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    જો( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  પાછા SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
સ્થિર પૂર્ણાંક bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  પૂર્ણાંક i,                 /* Index of the parameter to bind */
  અચલ શૂન્ય *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  શૂન્ય (*xDel)(શૂન્ય*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  પૂર્ણાંક rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  જો( rc==SQLITE_OK ){
    પ્રતિજ્ઞા( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    જો( zData!=0 ){
      pVar = &p->aVar[i-1];
      જો( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }નહીંતર જો( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        પ્રતિજ્ઞા( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }નહીંતર{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        જો( encoding==0 ) pVar->enc = ENC(p->db);
      }
      જો( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      જો( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }નહીંતર જો( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((શૂન્ય*)zData);
  }
  પાછા rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
પૂર્ણાંક sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક i,
  અચલ શૂન્ય *zData,
  પૂર્ણાંક nData,
  શૂન્ય (*xDel)(શૂન્ય*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( nData<0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  પાછા bindText(pStmt, i, zData, nData, xDel, 0);
}
પૂર્ણાંક sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક i,
  અચલ શૂન્ય *zData,
  sqlite3_uint64 nData,
  શૂન્ય (*xDel)(શૂન્ય*)
){
  પ્રતિજ્ઞા( xDel!=SQLITE_DYNAMIC );
  પાછા bindText(pStmt, i, zData, nData, xDel, 0);
}
પૂર્ણાંક sqlite3_bind_double(sqlite3_stmt *pStmt, પૂર્ણાંક i, દ્વિગુણ r_કિંમત){
  પૂર્ણાંક rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  જો( rc==SQLITE_OK ){
    પ્રતિજ્ઞા( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_કિંમત);
    sqlite3_mutex_leave(p->db->mutex);
  }
  પાછા rc;
}
પૂર્ણાંક sqlite3_bind_int(sqlite3_stmt *p, પૂર્ણાંક i, પૂર્ણાંક i_કિંમત){
  પાછા sqlite3_bind_int64(p, i, (i64)i_કિંમત);
}
પૂર્ણાંક sqlite3_bind_int64(sqlite3_stmt *pStmt, પૂર્ણાંક i, sqlite_int64 i_કિંમત){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  જો( vdbeSafetyNotNull(p) ){
    પાછા SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  જો( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    પાછા SQLITE_MISUSE_BKPT;
  }
  જો( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    પાછા SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  જો( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    પ્રતિજ્ઞા( (p->expmask & p->smimask)==p->smimask );
    જો( (p->expmask & expireMask)!=0 ){
      જો( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        પૂર્ણાંક sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        પૂર્ણાંક sm2 = i_કિંમત>=0 && i_કિંમત<=1;
        જો( sm1!=sm2 ){
          p->expired = 1;
        }
      }નહીંતર જો( (pVar->flags & MEM_Int)==0 || i_કિંમત!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_કિંમત);
  sqlite3_mutex_leave(p->db->mutex);
  પાછા SQLITE_OK;
}
પૂર્ણાંક sqlite3_bind_null(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  પૂર્ણાંક rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  જો( rc==SQLITE_OK ){
    પ્રતિજ્ઞા( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  પાછા rc;
}
પૂર્ણાંક sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક i,
  શૂન્ય *pPtr,
  અચલ અક્ષર *zPTtype,
  શૂન્ય (*xDestructor)(શૂન્ય*)
){
  પૂર્ણાંક rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  જો( rc==SQLITE_OK ){
    પ્રતિજ્ઞા( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }નહીંતર જો( xDestructor ){
    xDestructor(pPtr);
  }
  પાછા rc;
}
પૂર્ણાંક sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક i,
  અચલ અક્ષર *zData,
  પૂર્ણાંક nData,
  શૂન્ય (*xDel)(શૂન્ય*)
){
  પાછા bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
પૂર્ણાંક sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક i,
  અચલ અક્ષર *zData,
  sqlite3_uint64 nData,
  શૂન્ય (*xDel)(શૂન્ય*),
  unsigned અક્ષર enc
){
  પ્રતિજ્ઞા( xDel!=SQLITE_DYNAMIC );
  જો( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    જો( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  પાછા bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
પૂર્ણાંક sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક i,
  અચલ શૂન્ય *zData,
  પૂર્ણાંક n,
  શૂન્ય (*xDel)(શૂન્ય*)
){
  પાછા bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
પૂર્ણાંક sqlite3_bind_કિંમત(sqlite3_stmt *pStmt, પૂર્ણાંક i, અચલ sqlite3_કિંમત *p_કિંમત){
  પૂર્ણાંક rc;
  પસંદગી( sqlite3_કિંમત_type((sqlite3_કિંમત*)p_કિંમત) ){
    સ્થિતિ SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_કિંમત->u.i);
      તોડો;
    }
    સ્થિતિ SQLITE_FLOAT: {
      પ્રતિજ્ઞા( p_કિંમત->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_કિંમત->flags & MEM_Real) ? p_કિંમત->u.r : (દ્વિગુણ)p_કિંમત->u.i
      );
      તોડો;
    }
    સ્થિતિ SQLITE_BLOB: {
      જો( p_કિંમત->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_કિંમત->u.nZero);
      }નહીંતર{
        rc = sqlite3_bind_blob(pStmt, i, p_કિંમત->z, p_કિંમત->n,SQLITE_TRANSIENT);
      }
      તોડો;
    }
    સ્થિતિ SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_કિંમત->z, p_કિંમત->n, SQLITE_TRANSIENT,
                              p_કિંમત->enc);
      તોડો;
    }
    મૂળભૂત: {
      rc = sqlite3_bind_null(pStmt, i);
      તોડો;
    }
  }
  પાછા rc;
}
પૂર્ણાંક sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, પૂર્ણાંક i, પૂર્ણાંક n){
  પૂર્ણાંક rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  જો( rc==SQLITE_OK ){
    પ્રતિજ્ઞા( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#નહીંતર
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  પાછા rc;
}
પૂર્ણાંક sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, પૂર્ણાંક i, sqlite3_uint64 n){
  પૂર્ણાંક rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( p==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  જો( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }નહીંતર{
    પ્રતિજ્ઞા( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  પાછા rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
પૂર્ણાંક sqlite3_bind_parameter_ગણતરી(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  પાછા p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
અચલ અક્ષર *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, પૂર્ણાંક i){
  Vdbe *p = (Vdbe*)pStmt;
  જો( p==0 ) પાછા 0;
  પાછા sqlite3_VList_અંક_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
પૂર્ણાંક sqlite3VdbeParameterIndex(Vdbe *p, અચલ અક્ષર *zName, પૂર્ણાંક nName){
  જો( p==0 || zName==0 ) પાછા 0;
  પાછા sqlite3_VList_Name_To_અંક(p->pVList, zName, nName);
}
પૂર્ણાંક sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, અચલ અક્ષર *zName){
  પાછા sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
પૂર્ણાંક sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( pTo->db==pFrom->db );
  પ્રતિજ્ઞા( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  માટે(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  પાછા SQLITE_OK;
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
પૂર્ણાંક sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  જો( pFrom->nVar!=pTo->nVar ){
    પાછા SQLITE_ERROR;
  }
  પ્રતિજ્ઞા( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  જો( pTo->expmask ){
    pTo->expired = 1;
  }
  પ્રતિજ્ઞા( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  જો( pFrom->expmask ){
    pFrom->expired = 1;
  }
  પાછા sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  પાછા pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
પૂર્ણાંક sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  પાછા pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
પૂર્ણાંક sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  પાછા pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
પૂર્ણાંક sqlite3_stmt_explain(sqlite3_stmt *pStmt, પૂર્ણાંક eMode){
  Vdbe *v = (Vdbe*)pStmt;
  પૂર્ણાંક rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pStmt==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  જો( ((પૂર્ણાંક)v->explain)==eMode ){
    rc = SQLITE_OK;
  }નહીંતર જો( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }નહીંતર જો( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }નહીંતર જો( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }નહીંતર જો( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }નહીંતર{
    પૂર્ણાંક savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    જો( rc ){
      v->explain = savedMode;
    }નહીંતર{
      v->haveEqpOps = eMode==2;
    }
  }
  જો( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }નહીંતર{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  પાછા rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
પૂર્ણાંક sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  પાછા v!=0 && v->eVdbeState==VDBE_RUN_STATE;
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
  જો( !sqlite3SafetyCheckOk(pDb) ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  જો( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }નહીંતર{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  પાછા pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
પૂર્ણાંક sqlite3_stmt_status(sqlite3_stmt *pStmt, પૂર્ણાંક op, પૂર્ણાંક resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
  જો( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (પૂર્ણાંક*)&v;
    પ્રતિજ્ઞા( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }નહીંતર{
    v = pVdbe->aCounter[op];
    જો( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  પાછા (પૂર્ણાંક)v;
}

/*
** Return the SQL associated with a prepared statement
*/
અચલ અક્ષર *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  પાછા p ? p->zSql : 0;
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
અક્ષર *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  પાછા 0;
#નહીંતર
  અક્ષર *z = 0;
  અચલ અક્ષર *zSql = sqlite3_sql(pStmt);
  જો( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  પાછા z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
અચલ અક્ષર *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  જો( p==0 ) પાછા 0;
  જો( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  પાછા p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
સ્થિર UnpackedRecord *vdbeUnpackRecord(
  ચાવી_Info *p_ચાવી_Info,
  પૂર્ણાંક n_ચાવી,
  અચલ શૂન્ય *p_ચાવી
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_ચાવી_Info);
  જો( pRet ){
    memset(pRet->aMem, 0, કદ(Mem)*(p_ચાવી_Info->n_ચાવી_Field+1));
    sqlite3VdbeRecordUnpack(n_ચાવી, p_ચાવી, pRet);
  }
  પાછા pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
પૂર્ણાંક sqlite3_preupdate_old(sqlite3 *db, પૂર્ણાંક iIdx, sqlite3_કિંમત **pp_કિંમત){
  PreUpdate *p;
  Mem *pMem;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( db==0 || pp_કિંમત==0 ){
    પાછા SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  જો( !p || p->op==SQLITE_ઉમેરો ){
    rc = SQLITE_MISUSE_BKPT;
    જાઓ preupdate_old_out;
  }
  જો( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }નહીંતર જો( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    જાઓ preupdate_old_out;
  }નહીંતર{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  જો( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    જાઓ preupdate_old_out;
  }

  જો( iIdx==p->pTab->iPKey ){
    *pp_કિંમત = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }નહીંતર{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    જો( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      પ્રતિજ્ઞા( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      જો( !aRec ) જાઓ preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      જો( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        જો( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      જો( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        જાઓ preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_કિંમત = &p->pUnpacked->aMem[iStore];
    જો( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      જો( pCol->iDflt>0 ){
        જો( p->apDflt==0 ){
          પૂર્ણાંક nByte;
          પ્રતિજ્ઞા( કદ(sqlite3_કિંમત*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = કદ(sqlite3_કિંમત*)*p->pTab->nCol;
          p->apDflt = (sqlite3_કિંમત**)sqlite3DbMallocZero(db, nByte);
          જો( p->apDflt==0 ) જાઓ preupdate_old_out;
        }
        જો( p->apDflt[iIdx]==0 ){
          sqlite3_કિંમત *pVal = 0;
          પદાવલિ *pDflt;
          પ્રતિજ્ઞા( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_પદાવલિ;
          rc = sqlite3_કિંમત_From_પદાવલિ(db, pDflt, ENC(db), pCol->affinity, &pVal);
          જો( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_કિંમત = p->apDflt[iIdx];
      }નહીંતર{
        *pp_કિંમત = (sqlite3_કિંમત *)column_Null_કિંમત();
      }
    }નહીંતર જો( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      જો( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  પાછા sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
પૂર્ણાંક sqlite3_preupdate_ગણતરી(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#નહીંતર
  p = db->pPreUpdate;
#endif
  પાછા (p ? p->pKeyinfo->n_ચાવી_Field : 0);
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
પૂર્ણાંક sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#નહીંતર
  p = db->pPreUpdate;
#endif
  પાછા (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
પૂર્ણાંક sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#નહીંતર
  p = db->pPreUpdate;
#endif
  પાછા (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
પૂર્ણાંક sqlite3_preupdate_new(sqlite3 *db, પૂર્ણાંક iIdx, sqlite3_કિંમત **pp_કિંમત){
  PreUpdate *p;
  પૂર્ણાંક rc = SQLITE_OK;
  Mem *pMem;
  પૂર્ણાંક iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( db==0 || pp_કિંમત==0 ){
    પાછા SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  જો( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    જાઓ preupdate_new_out;
  }
  જો( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }નહીંતર જો( iIdx >= p->pTab->nCol ){
    પાછા SQLITE_MISUSE_BKPT;
  }નહીંતર{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  જો( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    જાઓ preupdate_new_out;
  }

  જો( p->op==SQLITE_ઉમેરો ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    જો( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      જો( rc!=SQLITE_OK ) જાઓ preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      જો( !pUnpack ){
        rc = SQLITE_NOMEM;
        જાઓ preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    જો( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }નહીંતર જો( iStore>=pUnpack->nField ){
      pMem = (sqlite3_કિંમત *)column_Null_કિંમત();
    }નહીંતર જો( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }નહીંતર{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    પ્રતિજ્ઞા( p->op==SQLITE_UPDATE );
    જો( !p->aNew ){
      પ્રતિજ્ઞા( કદ(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, કદ(Mem)*p->pCsr->nField);
      જો( !p->aNew ){
        rc = SQLITE_NOMEM;
        જાઓ preupdate_new_out;
      }
    }
    પ્રતિજ્ઞા( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    જો( pMem->flags==0 ){
      જો( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }નહીંતર{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        જો( rc!=SQLITE_OK ) જાઓ preupdate_new_out;
      }
    }
  }
  *pp_કિંમત = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  પાછા sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
પૂર્ણાંક sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  પૂર્ણાંક iScan,                      /* Index of loop to report on */
  પૂર્ણાંક iScanStatusOp,              /* Which metric to return */
  પૂર્ણાંક flags,
  શૂન્ય *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  પૂર્ણાંક nOp;
  ScanStatus *pScan = 0;
  પૂર્ણાંક idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    પાછા 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  જો( p->pFrame ){
    VdbeFrame *pFrame;
    માટે(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  જો( iScan<0 ){
    પૂર્ણાંક ii;
    જો( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      માટે(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      પાછા 0;
    }
    પાછા 1;
  }
  જો( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }નહીંતર{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    માટે(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      જો( pScan->zName ){
        iScan--;
        જો( iScan<0 ) તોડો;
      }
    }
  }
  જો( idx>=p->nScan ) પાછા 1;
  પ્રતિજ્ઞા( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  પસંદગી( iScanStatusOp ){
    સ્થિતિ SQLITE_SCANSTAT_NLOOP: {
      જો( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }નહીંતર{
        *(sqlite3_int64*)pOut = -1;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_NVISIT: {
      જો( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }નહીંતર{
        *(sqlite3_int64*)pOut = -1;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_EST: {
      દ્વિગુણ r = 1.0;
      LogEst x = pScan->nEst;
      જ્યારે( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(દ્વિગુણ*)pOut = r*sqlite3LogEstToInt(x);
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_NAME: {
      *(અચલ અક્ષર**)pOut = pScan->zName;
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_EXPLAIN: {
      જો( pScan->addrExplain ){
        *(અચલ અક્ષર**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }નહીંતર{
        *(અચલ અક્ષર**)pOut = 0;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_SELECTID: {
      જો( pScan->addrExplain ){
        *(પૂર્ણાંક*)pOut = aOp[ pScan->addrExplain ].p1;
      }નહીંતર{
        *(પૂર્ણાંક*)pOut = -1;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_PARENTID: {
      જો( pScan->addrExplain ){
        *(પૂર્ણાંક*)pOut = aOp[ pScan->addrExplain ].p2;
      }નહીંતર{
        *(પૂર્ણાંક*)pOut = -1;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      જો( pScan->aAddrRange[0]==0 ){
        res = -1;
      }નહીંતર{
        પૂર્ણાંક ii;
        માટે(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          પૂર્ણાંક iIns = pScan->aAddrRange[ii];
          પૂર્ણાંક iEnd = pScan->aAddrRange[ii+1];
          જો( iIns==0 ) તોડો;
          જો( iIns>0 ){
            જ્યારે( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }નહીંતર{
            પૂર્ણાંક iOp;
            માટે(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              જો( pOp->p1!=iEnd ) ચાલુ;
              જો( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                ચાલુ;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      તોડો;
    }
    મૂળભૂત: {
      પાછા 1;
    }
  }
  પાછા 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
પૂર્ણાંક sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  પૂર્ણાંક iScan,                      /* Index of loop to report on */
  પૂર્ણાંક iScanStatusOp,              /* Which metric to return */
  શૂન્ય *pOut                      /* OUT: Write the answer here */
){
  પાછા sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
શૂન્ય sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  પૂર્ણાંક ii;
  માટે(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
