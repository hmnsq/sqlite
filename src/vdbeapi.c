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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "vdbeInt.h"
#অন্তর্ভাব "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
পূর্ণসংখ্যা sqlite3_expired(sqlite3_stmt *pStmt){
  পূর্ণসংখ্যা iRet = 1;
  যদি( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  ফেরত iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
স্থির পূর্ণসংখ্যা vdbeSafety(Vdbe *p){
  যদি( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    ফেরত 1;
  }অন্যথা{
    ফেরত 0;
  }
}
স্থির পূর্ণসংখ্যা vdbeSafetyNotNull(Vdbe *p){
  যদি( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    ফেরত 1;
  }অন্যথা{
    ফেরত vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
স্থির SQLITE_NOINLINE শূন্য invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  প্রতিজ্ঞা( p->startTime>0 );
  প্রতিজ্ঞা( db->init.busy==0 );
  প্রতিজ্ঞা( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  যদি( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  যদি( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (শূন্য*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   যদি( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#অন্যথা
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
পূর্ণসংখ্যা sqlite3_finalize(sqlite3_stmt *pStmt){
  পূর্ণসংখ্যা rc;
  যদি( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }অন্যথা{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    যদি( vdbeSafety(v) ) ফেরত SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    প্রতিজ্ঞা( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  ফেরত rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
পূর্ণসংখ্যা sqlite3_reset(sqlite3_stmt *pStmt){
  পূর্ণসংখ্যা rc;
  যদি( pStmt==0 ){
    rc = SQLITE_OK;
  }অন্যথা{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    প্রতিজ্ঞা( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  ফেরত rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
পূর্ণসংখ্যা sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#যদি SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pStmt==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
#যদি SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  জন্য(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    যদি( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  প্রতিজ্ঞা( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  যদি( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  ফেরত rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ধ্রুবক শূন্য *sqlite3_মান_blob(sqlite3_মান *pVal){
  Mem *p = (Mem*)pVal;
  যদি( p->flags & (MEM_Blob|MEM_Str) ){
    যদি( ExpandBlob(p)!=SQLITE_OK ){
      প্রতিজ্ঞা( p->flags==MEM_Null && p->z==0 );
      ফেরত 0;
    }
    p->flags |= MEM_Blob;
    ফেরত p->n ? p->z : 0;
  }অন্যথা{
    ফেরত sqlite3_মান_text(pVal);
  }
}
পূর্ণসংখ্যা sqlite3_মান_bytes(sqlite3_মান *pVal){
  ফেরত sqlite3_মান_Bytes(pVal, SQLITE_UTF8);
}
পূর্ণসংখ্যা sqlite3_মান_bytes16(sqlite3_মান *pVal){
  ফেরত sqlite3_মান_Bytes(pVal, SQLITE_UTF16NATIVE);
}
দ্বিগুণ sqlite3_মান_double(sqlite3_মান *pVal){
  ফেরত sqlite3_Vdbe_Real_মান((Mem*)pVal);
}
পূর্ণসংখ্যা sqlite3_মান_int(sqlite3_মান *pVal){
  ফেরত (পূর্ণসংখ্যা)sqlite3_Vdbe_Int_মান((Mem*)pVal);
}
sqlite_int64 sqlite3_মান_int64(sqlite3_মান *pVal){
  ফেরত sqlite3_Vdbe_Int_মান((Mem*)pVal);
}
unsigned পূর্ণসংখ্যা sqlite3_মান_subtype(sqlite3_মান *pVal){
  Mem *pMem = (Mem*)pVal;
  ফেরত ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
শূন্য *sqlite3_মান_pointer(sqlite3_মান *pVal, ধ্রুবক অক্ষর *zPType){
  Mem *p = (Mem*)pVal;
  যদি( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    ফেরত (শূন্য*)p->z;
  }অন্যথা{
    ফেরত 0;
  }
}
ধ্রুবক unsigned অক্ষর *sqlite3_মান_text(sqlite3_মান *pVal){
  ফেরত (ধ্রুবক unsigned অক্ষর *)sqlite3_মান_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_মান_text16(sqlite3_মান* pVal){
  ফেরত sqlite3_মান_Text(pVal, SQLITE_UTF16NATIVE);
}
ধ্রুবক শূন্য *sqlite3_মান_text16be(sqlite3_মান *pVal){
  ফেরত sqlite3_মান_Text(pVal, SQLITE_UTF16BE);
}
ধ্রুবক শূন্য *sqlite3_মান_text16le(sqlite3_মান *pVal){
  ফেরত sqlite3_মান_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
পূর্ণসংখ্যা sqlite3_মান_type(sqlite3_মান* pVal){
  স্থির ধ্রুবক u8 aType[] = {
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
    পূর্ণসংখ্যা eType = SQLITE_BLOB;
    যদি( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }অন্যথা যদি( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }অন্যথা যদি( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }অন্যথা যদি( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    প্রতিজ্ঞা( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  ফেরত aType[pVal->flags&MEM_AffMask];
}
পূর্ণসংখ্যা sqlite3_মান_encoding(sqlite3_মান *pVal){
  ফেরত pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
পূর্ণসংখ্যা sqlite3_মান_nochange(sqlite3_মান *pVal){
  ফেরত (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
পূর্ণসংখ্যা sqlite3_মান_frombind(sqlite3_মান *pVal){
  ফেরত (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_মান *sqlite3_মান_dup(ধ্রুবক sqlite3_মান *pOrig){
  sqlite3_মান *pNew;
  যদি( pOrig==0 ) ফেরত 0;
  pNew = sqlite3_malloc( আকার(*pNew) );
  যদি( pNew==0 ) ফেরত 0;
  memset(pNew, 0, আকার(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  যদি( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    যদি( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_মান_Free(pNew);
      pNew = 0;
    }
  }অন্যথা যদি( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  ফেরত pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
শূন্য sqlite3_মান_free(sqlite3_মান *pOld){
  sqlite3_মান_Free(pOld);
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
স্থির শূন্য set_ফলাফল_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  ধ্রুবক অক্ষর *z,          /* String pointer */
  পূর্ণসংখ্যা n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  শূন্য (*xDel)(শূন্য*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  পূর্ণসংখ্যা rc;
  যদি( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }অন্যথা যদি( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    প্রতিজ্ঞা( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_দৈর্ঘ্য]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }অন্যথা{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  যদি( rc ){
    যদি( rc==SQLITE_TOOBIG ){
      sqlite3_ফলাফল_error_toobig(pCtx);
    }অন্যথা{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      প্রতিজ্ঞা( rc==SQLITE_NOMEM );
      sqlite3_ফলাফল_error_nomem(pCtx);
    }
    ফেরত;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  যদি( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_ফলাফল_error_toobig(pCtx);
  }
}
স্থির পূর্ণসংখ্যা invoke_মান_Destructor(
  ধ্রুবক শূন্য *p,             /* Value to destroy */
  শূন্য (*xDel)(শূন্য*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  প্রতিজ্ঞা( xDel!=SQLITE_DYNAMIC );
  যদি( xDel==0 ){
    /* noop */
  }অন্যথা যদি( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }অন্যথা{
    xDel((শূন্য*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx!=0 ){
    sqlite3_ফলাফল_error_toobig(pCtx);
  }
#অন্যথা
  প্রতিজ্ঞা( pCtx!=0 );
  sqlite3_ফলাফল_error_toobig(pCtx);
#endif
  ফেরত SQLITE_TOOBIG;
}
শূন্য sqlite3_ফলাফল_blob(
  sqlite3_context *pCtx,
  ধ্রুবক শূন্য *z,
  পূর্ণসংখ্যা n,
  শূন্য (*xDel)(শূন্য *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 || n<0 ){
    invoke_মান_Destructor(z, xDel, pCtx);
    ফেরত;
  }
#endif
  প্রতিজ্ঞা( n>=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_ফলাফল_Str_Or_Error(pCtx, z, n, 0, xDel);
}
শূন্য sqlite3_ফলাফল_blob64(
  sqlite3_context *pCtx,
  ধ্রুবক শূন্য *z,
  sqlite3_uint64 n,
  শূন্য (*xDel)(শূন্য *)
){
  প্রতিজ্ঞা( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ){
    invoke_মান_Destructor(z, xDel, 0);
    ফেরত;
  }
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  যদি( n>0x7fffffff ){
    (শূন্য)invoke_মান_Destructor(z, xDel, pCtx);
  }অন্যথা{
    set_ফলাফল_Str_Or_Error(pCtx, z, (পূর্ণসংখ্যা)n, 0, xDel);
  }
}
শূন্য sqlite3_ফলাফল_double(sqlite3_context *pCtx, দ্বিগুণ rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
শূন্য sqlite3_ফলাফল_error(sqlite3_context *pCtx, ধ্রুবক অক্ষর *z, পূর্ণসংখ্যা n){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
শূন্য sqlite3_ফলাফল_error16(sqlite3_context *pCtx, ধ্রুবক শূন্য *z, পূর্ণসংখ্যা n){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
শূন্য sqlite3_ফলাফল_int(sqlite3_context *pCtx, পূর্ণসংখ্যা iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
শূন্য sqlite3_ফলাফল_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
শূন্য sqlite3_ফলাফল_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
শূন্য sqlite3_ফলাফল_pointer(
  sqlite3_context *pCtx,
  শূন্য *pPtr,
  ধ্রুবক অক্ষর *zPType,
  শূন্য (*xDestructor)(শূন্য*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ){
    invoke_মান_Destructor(pPtr, xDestructor, 0);
    ফেরত;
  }
#endif
  pOut = pCtx->pOut;
  প্রতিজ্ঞা( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
শূন্য sqlite3_ফলাফল_subtype(sqlite3_context *pCtx, unsigned পূর্ণসংখ্যা eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
#যদি defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  যদি( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_ফলাফল_SUBTYPE)==0
  ){
    অক্ষর zErr[200];
    sqlite3_snprintf(আকার(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_ফলাফল_error(pCtx, zErr, -1);
    ফেরত;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  প্রতিজ্ঞা( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
শূন্য sqlite3_ফলাফল_text(
  sqlite3_context *pCtx,
  ধ্রুবক অক্ষর *z,
  পূর্ণসংখ্যা n,
  শূন্য (*xDel)(শূন্য *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ){
    invoke_মান_Destructor(z, xDel, 0);
    ফেরত;
  }
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_ফলাফল_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
শূন্য sqlite3_ফলাফল_text64(
  sqlite3_context *pCtx,
  ধ্রুবক অক্ষর *z,
  sqlite3_uint64 n,
  শূন্য (*xDel)(শূন্য *),
  unsigned অক্ষর enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ){
    invoke_মান_Destructor(z, xDel, 0);
    ফেরত;
  }
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  প্রতিজ্ঞা( xDel!=SQLITE_DYNAMIC );
  যদি( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    যদি( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  যদি( n>0x7fffffff ){
    (শূন্য)invoke_মান_Destructor(z, xDel, pCtx);
  }অন্যথা{
    set_ফলাফল_Str_Or_Error(pCtx, z, (পূর্ণসংখ্যা)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
শূন্য sqlite3_ফলাফল_text16(
  sqlite3_context *pCtx,
  ধ্রুবক শূন্য *z,
  পূর্ণসংখ্যা n,
  শূন্য (*xDel)(শূন্য *)
){
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_ফলাফল_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
শূন্য sqlite3_ফলাফল_text16be(
  sqlite3_context *pCtx,
  ধ্রুবক শূন্য *z,
  পূর্ণসংখ্যা n,
  শূন্য (*xDel)(শূন্য *)
){
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_ফলাফল_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
শূন্য sqlite3_ফলাফল_text16le(
  sqlite3_context *pCtx,
  ধ্রুবক শূন্য *z,
  পূর্ণসংখ্যা n,
  শূন্য (*xDel)(শূন্য *)
){
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_ফলাফল_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
শূন্য sqlite3_ফলাফল_মান(sqlite3_context *pCtx, sqlite3_মান *p_মান){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
  যদি( p_মান==0 ){
    sqlite3_ফলাফল_null(pCtx);
    ফেরত;
  }
#endif
  pOut = pCtx->pOut;
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_মান);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  যদি( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_ফলাফল_error_toobig(pCtx);
  }
}
শূন্য sqlite3_ফলাফল_zeroblob(sqlite3_context *pCtx, পূর্ণসংখ্যা n){
  sqlite3_ফলাফল_zeroblob64(pCtx, n>0 ? n : 0);
}
পূর্ণসংখ্যা sqlite3_ফলাফল_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  প্রতিজ্ঞা( sqlite3_mutex_held(pOut->db->mutex) );
  যদি( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_দৈর্ঘ্য] ){
    sqlite3_ফলাফল_error_toobig(pCtx);
    ফেরত SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (পূর্ণসংখ্যা)n);
  ফেরত SQLITE_OK;
#অন্যথা
  ফেরত sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (পূর্ণসংখ্যা)n);
#endif
}
শূন্য sqlite3_ফলাফল_error_code(sqlite3_context *pCtx, পূর্ণসংখ্যা errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  যদি( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  যদি( pCtx->pOut->flags & MEM_Null ){
    set_ফলাফল_Str_Or_Error(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
শূন্য sqlite3_ফলাফল_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
শূন্য sqlite3_ফলাফল_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
শূন্য sqlite3_ফলাফল_str(sqlite3_context *pCtx, sqlite3_str *pStr, পূর্ণসংখ্যা eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
  যদি( pStr==0 ) ফেরত;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  যদি( pStr->accError==0 ){
    যদি( pStr->nChar==0 ){
      set_ফলাফল_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }অন্যথা{
      ধ্রুবক অক্ষর *zText = sqlite3_str_মান(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      প্রতিজ্ঞা( isMalloced(pStr) );
      যদি( eOwn==SQLITE_COPY ){
        set_ফলাফল_Str_Or_Error(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }অন্যথা{
        set_ফলাফল_Str_Or_Error(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }অন্যথা যদি( pStr->accError==SQLITE_NOMEM ){
    sqlite3_ফলাফল_error_nomem(pCtx);
  }অন্যথা{
    প্রতিজ্ঞা( pStr->accError==SQLITE_TOOBIG );
    sqlite3_ফলাফল_error_toobig(pCtx);
  }
  যদি( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
শূন্য sqlite3_ফলাফল_Int_Real(sqlite3_context *pCtx){
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  যদি( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
স্থির পূর্ণসংখ্যা doWalCallbacks(sqlite3 *db){
  পূর্ণসংখ্যা rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    যদি( pBt ){
      পূর্ণসংখ্যা nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      যদি( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#অন্যথা
  UNUSED_PARAMETER(db);
#endif
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা sqlite3Step(Vdbe *p){
  sqlite3 *db;
  পূর্ণসংখ্যা rc;

  প্রতিজ্ঞা(p);
  db = p->db;
  যদি( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    যদি( p->eVdbeState==VDBE_READY_STATE ){
      যদি( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        যদি( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        যাও end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      যদি( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      প্রতিজ্ঞা( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      যদি( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }অন্যথা{
        প্রতিজ্ঞা( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      যদি( p->readOnly==0 ) db->nVdbeWrite++;
      যদি( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }অন্যথা

    যদি( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      যদি( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }অন্যথা{
        ফেরত SQLITE_MISUSE_BKPT;
      }
#অন্যথা
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      প্রতিজ্ঞা( p->eVdbeState==VDBE_READY_STATE );
      যাও restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  যদি( p->explain ){
    rc = sqlite3VdbeList(p);
  }অন্যথা
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  যদি( rc==SQLITE_ROW ){
    প্রতিজ্ঞা( p->rc==SQLITE_OK );
    প্রতিজ্ঞা( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    ফেরত SQLITE_ROW;
  }অন্যথা{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->p_ফলাফল_Row = 0;
    যদি( rc==SQLITE_DONE && db->autoCommit ){
      প্রতিজ্ঞা( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      যদি( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }অন্যথা যদি( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  যদি( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    যদি( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  প্রতিজ্ঞা( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  ফেরত (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
পূর্ণসংখ্যা sqlite3_step(sqlite3_stmt *pStmt){
  পূর্ণসংখ্যা rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  পূর্ণসংখ্যা cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  যদি( vdbeSafetyNotNull(v) ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  যখন( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    পূর্ণসংখ্যা savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    যদি( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ধ্রুবক অক্ষর *zErr = (ধ্রুবক অক্ষর *)sqlite3_মান_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_বার্তা);
      যদি( !db->mallocFailed ){
        v->z_Err_বার্তা = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } অন্যথা {
        v->z_Err_বার্তা = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      বিরতি;
    }
    sqlite3_reset(pStmt);
    যদি( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    প্রতিজ্ঞা( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
শূন্য *sqlite3_user_তথ্য(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( p==0 ) ফেরত 0;
#endif
  প্রতিজ্ঞা( p && p->pFunc );
  ফেরত p->pFunc->p_User_তথ্য;
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
  যদি( p==0 ) ফেরত 0;
#অন্যথা
  প্রতিজ্ঞা( p && p->pOut );
#endif
  ফেরত p->pOut->db;
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
পূর্ণসংখ্যা sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( p==0 ) ফেরত 0;
#অন্যথা
  প্রতিজ্ঞা( p );
#endif
  ফেরত sqlite3_মান_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
শূন্য sqlite3_Vdbe_মান_List_Free(শূন্য *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
স্থির পূর্ণসংখ্যা মান_From_মান_List(
  sqlite3_মান *pVal,        /* Pointer to the ValueList object */
  sqlite3_মান **ppOut,      /* Store the next value from the list here */
  পূর্ণসংখ্যা b_পরবর্তী                   /* 1 for _next(). 0 for _first() */
){
  পূর্ণসংখ্যা rc;
  মান_List *pRhs;

  *ppOut = 0;
  যদি( pVal==0 ) ফেরত SQLITE_MISUSE_BKPT;
  যদি( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_মান_List_Free ){
    ফেরত SQLITE_ERROR;
  }অন্যথা{
    প্রতিজ্ঞা( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    প্রতিজ্ঞা( pVal->eSubtype=='p' );
    প্রতিজ্ঞা( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (মান_List*)pVal->z;
  }
  যদি( b_পরবর্তী ){
    rc = sqlite3_Btree_পরবর্তী(pRhs->pCsr, 0);
  }অন্যথা{
    পূর্ণসংখ্যা dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    প্রতিজ্ঞা( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    যদি( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  যদি( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, আকার(sMem));
    sz = sqlite3_Btree_Payload_আকার(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    যদি( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_মান *pOut = pRhs->pOut;
      পূর্ণসংখ্যা iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      যদি( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }অন্যথা{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  ফেরত rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
পূর্ণসংখ্যা sqlite3_vtab_in_first(sqlite3_মান *pVal, sqlite3_মান **ppOut){
  ফেরত মান_From_মান_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
পূর্ণসংখ্যা sqlite3_vtab_in_পরবর্তী(sqlite3_মান *pVal, sqlite3_মান **ppOut){
  ফেরত মান_From_মান_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  পূর্ণসংখ্যা rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  প্রতিজ্ঞা( p->pVdbe!=0 );
#অন্যথা
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  যদি( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    যদি( rc ) *piTime = 0;
  }
  ফেরত *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
স্থির SQLITE_NOINLINE শূন্য *createAggContext(sqlite3_context *p, পূর্ণসংখ্যা nByte){
  Mem *pMem = p->pMem;
  প্রতিজ্ঞা( (pMem->flags & MEM_Agg)==0 );
  যদি( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }অন্যথা{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    যদি( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  ফেরত (শূন্য*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
শূন্য *sqlite3_aggregate_context(sqlite3_context *p, পূর্ণসংখ্যা nByte){
  প্রতিজ্ঞা( p && p->pFunc && p->pFunc->xFinalize );
  প্রতিজ্ঞা( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  যদি( (p->pMem->flags & MEM_Agg)==0 ){
    ফেরত createAggContext(p, nByte);
  }অন্যথা{
    ফেরত (শূন্য*)p->pMem->z;
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
শূন্য *sqlite3_get_auxdata(sqlite3_context *pCtx, পূর্ণসংখ্যা iArg){
  Aux_তথ্য *p_Aux_তথ্য;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত 0;
#endif
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#যদি SQLITE_ENABLE_STAT4
  যদি( pCtx->pVdbe==0 ) ফেরত 0;
#অন্যথা
  প্রতিজ্ঞা( pCtx->pVdbe!=0 );
#endif
  জন্য(p_Aux_তথ্য=pCtx->pVdbe->p_Aux_তথ্য; p_Aux_তথ্য; p_Aux_তথ্য=p_Aux_তথ্য->p_পরবর্তী_Aux){
    যদি(  p_Aux_তথ্য->iAuxArg==iArg && (p_Aux_তথ্য->iAuxOp==pCtx->iOp || iArg<0) ){
      ফেরত p_Aux_তথ্য->pAux;
    }
  }
  ফেরত 0;
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
শূন্য sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  পূর্ণসংখ্যা iArg,
  শূন্য *pAux,
  শূন্য (*xDelete)(শূন্য*)
){
  Aux_তথ্য *p_Aux_তথ্য;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCtx==0 ) ফেরত;
#endif
  pVdbe= pCtx->pVdbe;
  প্রতিজ্ঞা( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  যদি( pVdbe==0 ) যাও failed;
#অন্যথা
  প্রতিজ্ঞা( pVdbe!=0 );
#endif

  জন্য(p_Aux_তথ্য=pVdbe->p_Aux_তথ্য; p_Aux_তথ্য; p_Aux_তথ্য=p_Aux_তথ্য->p_পরবর্তী_Aux){
    যদি( p_Aux_তথ্য->iAuxArg==iArg && (p_Aux_তথ্য->iAuxOp==pCtx->iOp || iArg<0) ){
      বিরতি;
    }
  }
  যদি( p_Aux_তথ্য==0 ){
    p_Aux_তথ্য = sqlite3DbMallocZero(pVdbe->db, আকার(Aux_তথ্য));
    যদি( !p_Aux_তথ্য ) যাও failed;
    p_Aux_তথ্য->iAuxOp = pCtx->iOp;
    p_Aux_তথ্য->iAuxArg = iArg;
    p_Aux_তথ্য->p_পরবর্তী_Aux = pVdbe->p_Aux_তথ্য;
    pVdbe->p_Aux_তথ্য = p_Aux_তথ্য;
    যদি( pCtx->isError==0 ) pCtx->isError = -1;
  }অন্যথা যদি( p_Aux_তথ্য->xDeleteAux ){
    p_Aux_তথ্য->xDeleteAux(p_Aux_তথ্য->pAux);
  }

  p_Aux_তথ্য->pAux = pAux;
  p_Aux_তথ্য->xDeleteAux = xDelete;
  ফেরত;

failed:
  যদি( xDelete ){
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
পূর্ণসংখ্যা sqlite3_aggregate_গুনতি(sqlite3_context *p){
  প্রতিজ্ঞা( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  ফেরত p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
পূর্ণসংখ্যা sqlite3_column_গুনতি(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  যদি( pVm==0 ) ফেরত 0;
  ফেরত pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
পূর্ণসংখ্যা sqlite3_তথ্য_গুনতি(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  যদি( pVm==0 || pVm->p_ফলাফল_Row==0 ) ফেরত 0;
  ফেরত pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
স্থির ধ্রুবক Mem *column_Null_মান(শূন্য){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  স্থির ধ্রুবক Mem nullMem
#যদি defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (অক্ষর*)0,
        /* .n          = */ (পূর্ণসংখ্যা)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (পূর্ণসংখ্যা)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (অক্ষর*)0,
        /* .xDel       = */ (শূন্য(*)(শূন্য*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  ফেরত &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
স্থির Mem *columnMem(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  যদি( pVm==0 ) ফেরত (Mem*)column_Null_মান();
  প্রতিজ্ঞা( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  যদি( pVm->p_ফলাফল_Row!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->p_ফলাফল_Row[i];
  }অন্যথা{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_মান();
  }
  ফেরত pOut;
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
স্থির শূন্য columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  যদি( p ){
    প্রতিজ্ঞা( p->db!=0 );
    প্রতিজ্ঞা( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
ধ্রুবক শূন্য *sqlite3_column_blob(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  ধ্রুবক শূন্য *val;
  val = sqlite3_মান_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  ফেরত val;
}
পূর্ণসংখ্যা sqlite3_column_bytes(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  পূর্ণসংখ্যা val = sqlite3_মান_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
পূর্ণসংখ্যা sqlite3_column_bytes16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  পূর্ণসংখ্যা val = sqlite3_মান_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
দ্বিগুণ sqlite3_column_double(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  দ্বিগুণ val = sqlite3_মান_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
পূর্ণসংখ্যা sqlite3_column_int(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  পূর্ণসংখ্যা val = sqlite3_মান_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  sqlite_int64 val = sqlite3_মান_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
ধ্রুবক unsigned অক্ষর *sqlite3_column_text(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  ধ্রুবক unsigned অক্ষর *val = sqlite3_মান_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
sqlite3_মান *sqlite3_column_মান(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  Mem *pOut = columnMem(pStmt, i);
  যদি( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  ফেরত (sqlite3_মান *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_column_text16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  ধ্রুবক শূন্য *val = sqlite3_মান_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত val;
}
#endif /* SQLITE_OMIT_UTF16 */
পূর্ণসংখ্যা sqlite3_column_type(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  পূর্ণসংখ্যা iType = sqlite3_মান_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ফেরত iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
স্থির ধ্রুবক অক্ষর * ধ্রুবক azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
স্থির ধ্রুবক u16 azExplainColNames16data[] = {
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
স্থির ধ্রুবক u8 iExplainColNames16[] = {
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
স্থির ধ্রুবক শূন্য *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  পূর্ণসংখ্যা N,                   /* Which column to get the name for */
  পূর্ণসংখ্যা useUtf16,            /* True to return the name as UTF16 */
  পূর্ণসংখ্যা useType              /* What type of name */
){
  ধ্রুবক শূন্য *ret;
  Vdbe *p;
  পূর্ণসংখ্যা n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pStmt==0 ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
  যদি( N<0 ) ফেরত 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  প্রতিজ্ঞা( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  যদি( p->explain ){
    যদি( useType>0 ) যাও columnName_end;
    n = p->explain==1 ? 8 : 4;
    যদি( N>=n ) যাও columnName_end;
    যদি( useUtf16 ){
      পূর্ণসংখ্যা i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (শূন্য*)&azExplainColNames16data[i];
    }অন্যথা{
      ret = (শূন্য*)azExplainColNames8[N + 8*p->explain - 8];
    }
    যাও columnName_end;
  }
  n = p->nResColumn;
  যদি( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    যদি( useUtf16 ){
      ret = sqlite3_মান_text16((sqlite3_মান*)&p->aColName[N]);
    }অন্যথা
#endif
    {
      ret = sqlite3_মান_text((sqlite3_মান*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    প্রতিজ্ঞা( db->mallocFailed==0 || db->mallocFailed==1 );
    যদি( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  ফেরত ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ধ্রুবক অক্ষর *sqlite3_column_name(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_column_name16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#যদি defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ধ্রুবক অক্ষর *sqlite3_column_decltype(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_column_decltype16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ধ্রুবক অক্ষর *sqlite3_column_database_name(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_column_database_name16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ধ্রুবক অক্ষর *sqlite3_column_table_name(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_column_table_name16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ধ্রুবক অক্ষর *sqlite3_column_origin_name(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
ধ্রুবক শূন্য *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, পূর্ণসংখ্যা N){
  ফেরত columnName(pStmt, N, 1, COLNAME_COLUMN);
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
স্থির পূর্ণসংখ্যা vdbeUnbind(Vdbe *p, unsigned পূর্ণসংখ্যা i){
  Mem *pVar;
  যদি( vdbeSafetyNotNull(p) ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  যদি( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ফেরত SQLITE_MISUSE_BKPT;
  }
  যদি( i>=(unsigned পূর্ণসংখ্যা)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ফেরত SQLITE_RANGE;
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
  প্রতিজ্ঞা( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  প্রতিজ্ঞা( (p->expmask & p->smimask)==p->smimask );
  যদি( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    যদি( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  ফেরত SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
স্থির পূর্ণসংখ্যা bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  পূর্ণসংখ্যা i,                 /* Index of the parameter to bind */
  ধ্রুবক শূন্য *z_তথ্য,     /* Pointer to the data to be bound */
  i64 n_তথ্য,             /* Number of bytes of data to be bound */
  শূন্য (*xDel)(শূন্য*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  পূর্ণসংখ্যা rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  যদি( rc==SQLITE_OK ){
    প্রতিজ্ঞা( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    যদি( z_তথ্য!=0 ){
      pVar = &p->aVar[i-1];
      যদি( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, z_তথ্য, n_তথ্য, xDel);
      }অন্যথা যদি( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        প্রতিজ্ঞা( z_তথ্য==0
             || n_তথ্য<0 
             || n_তথ্য>pVar->db->aLimit[SQLITE_LIMIT_দৈর্ঘ্য]
             || ((u8*)z_তথ্য)[n_তথ্য]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, z_তথ্য, n_তথ্য, xDel);
        pVar->flags |= MEM_Term;
      }অন্যথা{
        rc = sqlite3VdbeMemSetStr(pVar, z_তথ্য, n_তথ্য, encoding, xDel);
        যদি( encoding==0 ) pVar->enc = ENC(p->db);
      }
      যদি( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      যদি( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }অন্যথা যদি( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((শূন্য*)z_তথ্য);
  }
  ফেরত rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
পূর্ণসংখ্যা sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা i,
  ধ্রুবক শূন্য *z_তথ্য,
  পূর্ণসংখ্যা n_তথ্য,
  শূন্য (*xDel)(শূন্য*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( n_তথ্য<0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  ফেরত bindText(pStmt, i, z_তথ্য, n_তথ্য, xDel, 0);
}
পূর্ণসংখ্যা sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা i,
  ধ্রুবক শূন্য *z_তথ্য,
  sqlite3_uint64 n_তথ্য,
  শূন্য (*xDel)(শূন্য*)
){
  প্রতিজ্ঞা( xDel!=SQLITE_DYNAMIC );
  ফেরত bindText(pStmt, i, z_তথ্য, n_তথ্য, xDel, 0);
}
পূর্ণসংখ্যা sqlite3_bind_double(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i, দ্বিগুণ r_মান){
  পূর্ণসংখ্যা rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  যদি( rc==SQLITE_OK ){
    প্রতিজ্ঞা( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_মান);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_bind_int(sqlite3_stmt *p, পূর্ণসংখ্যা i, পূর্ণসংখ্যা i_মান){
  ফেরত sqlite3_bind_int64(p, i, (i64)i_মান);
}
পূর্ণসংখ্যা sqlite3_bind_int64(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i, sqlite_int64 i_মান){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  যদি( vdbeSafetyNotNull(p) ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  যদি( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ফেরত SQLITE_MISUSE_BKPT;
  }
  যদি( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ফেরত SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  যদি( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    প্রতিজ্ঞা( (p->expmask & p->smimask)==p->smimask );
    যদি( (p->expmask & expireMask)!=0 ){
      যদি( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        পূর্ণসংখ্যা sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        পূর্ণসংখ্যা sm2 = i_মান>=0 && i_মান<=1;
        যদি( sm1!=sm2 ){
          p->expired = 1;
        }
      }অন্যথা যদি( (pVar->flags & MEM_Int)==0 || i_মান!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_মান);
  sqlite3_mutex_leave(p->db->mutex);
  ফেরত SQLITE_OK;
}
পূর্ণসংখ্যা sqlite3_bind_null(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  পূর্ণসংখ্যা rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  যদি( rc==SQLITE_OK ){
    প্রতিজ্ঞা( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা i,
  শূন্য *pPtr,
  ধ্রুবক অক্ষর *zPTtype,
  শূন্য (*xDestructor)(শূন্য*)
){
  পূর্ণসংখ্যা rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  যদি( rc==SQLITE_OK ){
    প্রতিজ্ঞা( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }অন্যথা যদি( xDestructor ){
    xDestructor(pPtr);
  }
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা i,
  ধ্রুবক অক্ষর *z_তথ্য,
  পূর্ণসংখ্যা n_তথ্য,
  শূন্য (*xDel)(শূন্য*)
){
  ফেরত bindText(pStmt, i, z_তথ্য, n_তথ্য, xDel, SQLITE_UTF8);
}
পূর্ণসংখ্যা sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা i,
  ধ্রুবক অক্ষর *z_তথ্য,
  sqlite3_uint64 n_তথ্য,
  শূন্য (*xDel)(শূন্য*),
  unsigned অক্ষর enc
){
  প্রতিজ্ঞা( xDel!=SQLITE_DYNAMIC );
  যদি( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    যদি( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_তথ্য &= ~(u64)1;
  }
  ফেরত bindText(pStmt, i, z_তথ্য, n_তথ্য, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
পূর্ণসংখ্যা sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা i,
  ধ্রুবক শূন্য *z_তথ্য,
  পূর্ণসংখ্যা n,
  শূন্য (*xDel)(শূন্য*)
){
  ফেরত bindText(pStmt, i, z_তথ্য, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
পূর্ণসংখ্যা sqlite3_bind_মান(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i, ধ্রুবক sqlite3_মান *p_মান){
  পূর্ণসংখ্যা rc;
  নির্বাচন( sqlite3_মান_type((sqlite3_মান*)p_মান) ){
    ক্ষেত্র SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_মান->u.i);
      বিরতি;
    }
    ক্ষেত্র SQLITE_FLOAT: {
      প্রতিজ্ঞা( p_মান->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_মান->flags & MEM_Real) ? p_মান->u.r : (দ্বিগুণ)p_মান->u.i
      );
      বিরতি;
    }
    ক্ষেত্র SQLITE_BLOB: {
      যদি( p_মান->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_মান->u.nZero);
      }অন্যথা{
        rc = sqlite3_bind_blob(pStmt, i, p_মান->z, p_মান->n,SQLITE_TRANSIENT);
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_মান->z, p_মান->n, SQLITE_TRANSIENT,
                              p_মান->enc);
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      rc = sqlite3_bind_null(pStmt, i);
      বিরতি;
    }
  }
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i, পূর্ণসংখ্যা n){
  পূর্ণসংখ্যা rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  যদি( rc==SQLITE_OK ){
    প্রতিজ্ঞা( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#অন্যথা
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i, sqlite3_uint64 n){
  পূর্ণসংখ্যা rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( p==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  যদি( n>(u64)p->db->aLimit[SQLITE_LIMIT_দৈর্ঘ্য] ){
    rc = SQLITE_TOOBIG;
  }অন্যথা{
    প্রতিজ্ঞা( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  ফেরত rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
পূর্ণসংখ্যা sqlite3_bind_parameter_গুনতি(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ফেরত p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ধ্রুবক অক্ষর *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, পূর্ণসংখ্যা i){
  Vdbe *p = (Vdbe*)pStmt;
  যদি( p==0 ) ফেরত 0;
  ফেরত sqlite3_VList_অঙ্ক_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
পূর্ণসংখ্যা sqlite3_Vdbe_Parameter_সূচক(Vdbe *p, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা nName){
  যদি( p==0 || zName==0 ) ফেরত 0;
  ফেরত sqlite3_VList_Name_To_অঙ্ক(p->pVList, zName, nName);
}
পূর্ণসংখ্যা sqlite3_bind_parameter_সূচক(sqlite3_stmt *pStmt, ধ্রুবক অক্ষর *zName){
  ফেরত sqlite3_Vdbe_Parameter_সূচক((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
পূর্ণসংখ্যা sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  পূর্ণসংখ্যা i;
  প্রতিজ্ঞা( pTo->db==pFrom->db );
  প্রতিজ্ঞা( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  জন্য(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  ফেরত SQLITE_OK;
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
পূর্ণসংখ্যা sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  যদি( pFrom->nVar!=pTo->nVar ){
    ফেরত SQLITE_ERROR;
  }
  প্রতিজ্ঞা( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  যদি( pTo->expmask ){
    pTo->expired = 1;
  }
  প্রতিজ্ঞা( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  যদি( pFrom->expmask ){
    pFrom->expired = 1;
  }
  ফেরত sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  ফেরত pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
পূর্ণসংখ্যা sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  ফেরত pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
পূর্ণসংখ্যা sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  ফেরত pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
পূর্ণসংখ্যা sqlite3_stmt_explain(sqlite3_stmt *pStmt, পূর্ণসংখ্যা eMode){
  Vdbe *v = (Vdbe*)pStmt;
  পূর্ণসংখ্যা rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pStmt==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  যদি( ((পূর্ণসংখ্যা)v->explain)==eMode ){
    rc = SQLITE_OK;
  }অন্যথা যদি( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }অন্যথা যদি( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }অন্যথা যদি( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }অন্যথা যদি( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }অন্যথা{
    পূর্ণসংখ্যা savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    যদি( rc ){
      v->explain = savedMode;
    }অন্যথা{
      v->haveEqpOps = eMode==2;
    }
  }
  যদি( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }অন্যথা{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  ফেরত rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
পূর্ণসংখ্যা sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  ফেরত v!=0 && v->eVdbeState==VDBE_RUN_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_পরবর্তী_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_পরবর্তী;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(pDb) ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  যদি( pStmt==0 ){
    p_পরবর্তী = (sqlite3_stmt*)pDb->pVdbe;
  }অন্যথা{
    p_পরবর্তী = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  ফেরত p_পরবর্তী;
}

/*
** Return the value of a status counter for a prepared statement
*/
পূর্ণসংখ্যা sqlite3_stmt_status(sqlite3_stmt *pStmt, পূর্ণসংখ্যা op, পূর্ণসংখ্যা resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_আকার(pVdbe->a_গণক)))
  ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
  যদি( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (পূর্ণসংখ্যা*)&v;
    প্রতিজ্ঞা( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }অন্যথা{
    v = pVdbe->a_গণক[op];
    যদি( resetFlag ) pVdbe->a_গণক[op] = 0;
  }
  ফেরত (পূর্ণসংখ্যা)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ধ্রুবক অক্ষর *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ফেরত p ? p->zSql : 0;
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
অক্ষর *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  ফেরত 0;
#অন্যথা
  অক্ষর *z = 0;
  ধ্রুবক অক্ষর *zSql = sqlite3_sql(pStmt);
  যদি( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ফেরত z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ধ্রুবক অক্ষর *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  যদি( p==0 ) ফেরত 0;
  যদি( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ফেরত p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
স্থির UnpackedRecord *vdbeUnpackRecord(
  চাবি_Info *p_চাবি_Info,
  পূর্ণসংখ্যা n_চাবি,
  ধ্রুবক শূন্য *p_চাবি
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_চাবি_Info);
  যদি( pRet ){
    memset(pRet->aMem, 0, আকার(Mem)*(p_চাবি_Info->n_চাবি_Field+1));
    sqlite3VdbeRecordUnpack(n_চাবি, p_চাবি, pRet);
  }
  ফেরত pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
পূর্ণসংখ্যা sqlite3_preupdate_old(sqlite3 *db, পূর্ণসংখ্যা iIdx, sqlite3_মান **pp_মান){
  PreUpdate *p;
  Mem *pMem;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( db==0 || pp_মান==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  যদি( !p || p->op==SQLITE_ঢোকাও ){
    rc = SQLITE_MISUSE_BKPT;
    যাও preupdate_old_out;
  }
  যদি( p->pPk ){
    iStore = sqlite3_Table_Column_To_সূচক(p->pPk, iIdx);
  }অন্যথা যদি( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    যাও preupdate_old_out;
  }অন্যথা{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  যদি( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    যাও preupdate_old_out;
  }

  যদি( iIdx==p->pTab->iPKey ){
    *pp_মান = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }অন্যথা{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    যদি( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      প্রতিজ্ঞা( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_আকার(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      যদি( !aRec ) যাও preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      যদি( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        যদি( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      যদি( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        যাও preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_মান = &p->pUnpacked->aMem[iStore];
    যদি( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      যদি( pCol->iDflt>0 ){
        যদি( p->apDflt==0 ){
          পূর্ণসংখ্যা nByte;
          প্রতিজ্ঞা( আকার(sqlite3_মান*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = আকার(sqlite3_মান*)*p->pTab->nCol;
          p->apDflt = (sqlite3_মান**)sqlite3DbMallocZero(db, nByte);
          যদি( p->apDflt==0 ) যাও preupdate_old_out;
        }
        যদি( p->apDflt[iIdx]==0 ){
          sqlite3_মান *pVal = 0;
          রাশিমালা *pDflt;
          প্রতিজ্ঞা( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_রাশিমালা;
          rc = sqlite3_মান_From_রাশিমালা(db, pDflt, ENC(db), pCol->affinity, &pVal);
          যদি( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_মান = p->apDflt[iIdx];
      }অন্যথা{
        *pp_মান = (sqlite3_মান *)column_Null_মান();
      }
    }অন্যথা যদি( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      যদি( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  ফেরত sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
পূর্ণসংখ্যা sqlite3_preupdate_গুনতি(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#অন্যথা
  p = db->pPreUpdate;
#endif
  ফেরত (p ? p->pKeyinfo->n_চাবি_Field : 0);
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
পূর্ণসংখ্যা sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#অন্যথা
  p = db->pPreUpdate;
#endif
  ফেরত (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
পূর্ণসংখ্যা sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#অন্যথা
  p = db->pPreUpdate;
#endif
  ফেরত (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
পূর্ণসংখ্যা sqlite3_preupdate_new(sqlite3 *db, পূর্ণসংখ্যা iIdx, sqlite3_মান **pp_মান){
  PreUpdate *p;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  Mem *pMem;
  পূর্ণসংখ্যা iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( db==0 || pp_মান==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  যদি( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    যাও preupdate_new_out;
  }
  যদি( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_Table_Column_To_সূচক(p->pPk, iIdx);
  }অন্যথা যদি( iIdx >= p->pTab->nCol ){
    ফেরত SQLITE_MISUSE_BKPT;
  }অন্যথা{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  যদি( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    যাও preupdate_new_out;
  }

  যদি( p->op==SQLITE_ঢোকাও ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    যদি( !pUnpack ){
      Mem *p_তথ্য = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_তথ্য);
      যদি( rc!=SQLITE_OK ) যাও preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, p_তথ্য->n, p_তথ্য->z);
      যদি( !pUnpack ){
        rc = SQLITE_NOMEM;
        যাও preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    যদি( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }অন্যথা যদি( iStore>=pUnpack->nField ){
      pMem = (sqlite3_মান *)column_Null_মান();
    }অন্যথা যদি( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }অন্যথা{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    প্রতিজ্ঞা( p->op==SQLITE_UPDATE );
    যদি( !p->aNew ){
      প্রতিজ্ঞা( আকার(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, আকার(Mem)*p->pCsr->nField);
      যদি( !p->aNew ){
        rc = SQLITE_NOMEM;
        যাও preupdate_new_out;
      }
    }
    প্রতিজ্ঞা( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    যদি( pMem->flags==0 ){
      যদি( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }অন্যথা{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        যদি( rc!=SQLITE_OK ) যাও preupdate_new_out;
      }
    }
  }
  *pp_মান = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  ফেরত sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
পূর্ণসংখ্যা sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  পূর্ণসংখ্যা iScan,                      /* Index of loop to report on */
  পূর্ণসংখ্যা iScanStatusOp,              /* Which metric to return */
  পূর্ণসংখ্যা flags,
  শূন্য *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  পূর্ণসংখ্যা nOp;
  ScanStatus *pScan = 0;
  পূর্ণসংখ্যা idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    ফেরত 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  যদি( p->pFrame ){
    VdbeFrame *pFrame;
    জন্য(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  যদি( iScan<0 ){
    পূর্ণসংখ্যা ii;
    যদি( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      জন্য(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      ফেরত 0;
    }
    ফেরত 1;
  }
  যদি( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }অন্যথা{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    জন্য(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      যদি( pScan->zName ){
        iScan--;
        যদি( iScan<0 ) বিরতি;
      }
    }
  }
  যদি( idx>=p->nScan ) ফেরত 1;
  প্রতিজ্ঞা( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  নির্বাচন( iScanStatusOp ){
    ক্ষেত্র SQLITE_SCANSTAT_NLOOP: {
      যদি( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }অন্যথা{
        *(sqlite3_int64*)pOut = -1;
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_NVISIT: {
      যদি( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }অন্যথা{
        *(sqlite3_int64*)pOut = -1;
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_EST: {
      দ্বিগুণ r = 1.0;
      LogEst x = pScan->nEst;
      যখন( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(দ্বিগুণ*)pOut = r*sqlite3LogEstToInt(x);
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_NAME: {
      *(ধ্রুবক অক্ষর**)pOut = pScan->zName;
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_EXPLAIN: {
      যদি( pScan->addrExplain ){
        *(ধ্রুবক অক্ষর**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }অন্যথা{
        *(ধ্রুবক অক্ষর**)pOut = 0;
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_SELECTID: {
      যদি( pScan->addrExplain ){
        *(পূর্ণসংখ্যা*)pOut = aOp[ pScan->addrExplain ].p1;
      }অন্যথা{
        *(পূর্ণসংখ্যা*)pOut = -1;
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_PARENTID: {
      যদি( pScan->addrExplain ){
        *(পূর্ণসংখ্যা*)pOut = aOp[ pScan->addrExplain ].p2;
      }অন্যথা{
        *(পূর্ণসংখ্যা*)pOut = -1;
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      যদি( pScan->aAddrRange[0]==0 ){
        res = -1;
      }অন্যথা{
        পূর্ণসংখ্যা ii;
        জন্য(ii=0; ii<Array_আকার(pScan->aAddrRange); ii+=2){
          পূর্ণসংখ্যা iIns = pScan->aAddrRange[ii];
          পূর্ণসংখ্যা iEnd = pScan->aAddrRange[ii+1];
          যদি( iIns==0 ) বিরতি;
          যদি( iIns>0 ){
            যখন( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }অন্যথা{
            পূর্ণসংখ্যা iOp;
            জন্য(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              যদি( pOp->p1!=iEnd ) চালু;
              যদি( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                চালু;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      ফেরত 1;
    }
  }
  ফেরত 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
পূর্ণসংখ্যা sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  পূর্ণসংখ্যা iScan,                      /* Index of loop to report on */
  পূর্ণসংখ্যা iScanStatusOp,              /* Which metric to return */
  শূন্য *pOut                      /* OUT: Write the answer here */
){
  ফেরত sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
শূন্য sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  পূর্ণসংখ্যা ii;
  জন্য(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
