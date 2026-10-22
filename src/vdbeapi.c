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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"
#ضمن "opcodes.h"

#که_نه_تعریف_شوی SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
صحيح sqlite3_expired(sqlite3_stmt *pStmt){
  صحيح iRet = 1;
  إذا( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  أرجع iRet;
}
#د_که_پای

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ساكن صحيح vdbeSafety(Vdbe *p){
  إذا( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    أرجع 1;
  }وإلا{
    أرجع 0;
  }
}
ساكن صحيح vdbeSafetyNotNull(Vdbe *p){
  إذا( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    أرجع 1;
  }وإلا{
    أرجع vdbeSafety(p);
  }
}

#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ساكن SQLITE_NOINLINE فراغ invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  أكد( p->پیل_وخت>0 );
  أكد( db->init.busy==0 );
  أكد( p->zSql!=0 );
  sqlite3_Os_Current_وخت_Int64(db->pVfs, &iNow);
  iElapse = (iNow - p->پیل_وخت)*1000000;
#که_نه_تعریف_شوی SQLITE_OMIT_DEPRECATED
  إذا( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#د_که_پای
  إذا( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (فراغ*)&iElapse);
  }
  p->پیل_وخت = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   إذا( ((P)->پیل_وخت)>0 ){ invokeProfileCallback(DB,P); }
#وإلا
# define checkProfileCallback(DB,P)  /*no-op*/
#د_که_پای

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
صحيح sqlite3_finalize(sqlite3_stmt *pStmt){
  صحيح rc;
  إذا( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }وإلا{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    إذا( vdbeSafety(v) ) أرجع SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    أكد( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  أرجع rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
صحيح sqlite3_reset(sqlite3_stmt *pStmt){
  صحيح rc;
  إذا( pStmt==0 ){
    rc = SQLITE_OK;
  }وإلا{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    أكد( (rc & (db->err_ماسک))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  أرجع rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
صحيح sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  صحيح i;
  صحيح rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 ماسک_Small_Int = 0;  /* Bitmask of bindings with value 0 or 1 */
#إذا SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#د_که_پای
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pStmt==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#د_که_پای
#إذا SQLITE_THREADSAFE
  mutex = p->db->mutex;
#د_که_پای

  sqlite3_mutex_enter(mutex);
  لكل(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    إذا( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      ماسک_Small_Int |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  إذا( (p->expmask&~p->smimask)!=0 || (p->smimask&ماسک_Small_Int)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  أرجع rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ثابت فراغ *sqlite3_ارزښت_blob(sqlite3_ارزښت *pVal){
  Mem *p = (Mem*)pVal;
  إذا( p->flags & (MEM_Blob|MEM_Str) ){
    إذا( ExpandBlob(p)!=SQLITE_OK ){
      أكد( p->flags==MEM_Null && p->z==0 );
      أرجع 0;
    }
    p->flags |= MEM_Blob;
    أرجع p->n ? p->z : 0;
  }وإلا{
    أرجع sqlite3_ارزښت_متن(pVal);
  }
}
صحيح sqlite3_ارزښت_بایټونه(sqlite3_ارزښت *pVal){
  أرجع sqlite3_ارزښت_بایټونه_2(pVal, SQLITE_UTF8);
}
صحيح sqlite3_ارزښت_bytes16(sqlite3_ارزښت *pVal){
  أرجع sqlite3_ارزښت_بایټونه_2(pVal, SQLITE_UTF16NATIVE);
}
مزدوج sqlite3_ارزښت_double(sqlite3_ارزښت *pVal){
  أرجع sqlite3_Vdbe_Real_ارزښت((Mem*)pVal);
}
صحيح sqlite3_ارزښت_int(sqlite3_ارزښت *pVal){
  أرجع (صحيح)sqlite3_Vdbe_Int_ارزښت((Mem*)pVal);
}
sqlite_int64 sqlite3_ارزښت_int64(sqlite3_ارزښت *pVal){
  أرجع sqlite3_Vdbe_Int_ارزښت((Mem*)pVal);
}
بینښه صحيح sqlite3_ارزښت_subtype(sqlite3_ارزښت *pVal){
  Mem *pMem = (Mem*)pVal;
  أرجع ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
فراغ *sqlite3_ارزښت_اشاره_ګر(sqlite3_ارزښت *pVal, ثابت حرف *zPType){
  Mem *p = (Mem*)pVal;
  إذا( (p->flags&(MEM_Type_ماسک|MEM_غړی|MEM_Subtype)) ==
                 (MEM_Null|MEM_غړی|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    أرجع (فراغ*)p->z;
  }وإلا{
    أرجع 0;
  }
}
ثابت بینښه حرف *sqlite3_ارزښت_متن(sqlite3_ارزښت *pVal){
  أرجع (ثابت بینښه حرف *)sqlite3_ارزښت_متن_2(pVal, SQLITE_UTF8);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ارزښت_text16(sqlite3_ارزښت* pVal){
  أرجع sqlite3_ارزښت_متن_2(pVal, SQLITE_UTF16NATIVE);
}
ثابت فراغ *sqlite3_ارزښت_text16be(sqlite3_ارزښت *pVal){
  أرجع sqlite3_ارزښت_متن_2(pVal, SQLITE_UTF16BE);
}
ثابت فراغ *sqlite3_ارزښت_text16le(sqlite3_ارزښت *pVal){
  أرجع sqlite3_ارزښت_متن_2(pVal, SQLITE_UTF16LE);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
صحيح sqlite3_ارزښت_type(sqlite3_ارزښت* pVal){
  ساكن ثابت u8 aType[] = {
     SQLITE_BLOB,     /* 0x00 (not possible) */
     SQLITE_NULL,     /* 0x01 NULL */
     SQLITE_متن,     /* 0x02 TEXT */
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
     SQLITE_متن,     /* 0x12 (not possible) */
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
     SQLITE_متن,     /* 0x32 (not possible) */
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
#که_تعریف_شوی SQLITE_DEBUG
  {
    صحيح eType = SQLITE_BLOB;
    إذا( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }وإلا إذا( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }وإلا إذا( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }وإلا إذا( pVal->flags & MEM_Str ){
      eType = SQLITE_متن;
    }
    أكد( eType == aType[pVal->flags&MEM_Aff_ماسک] );
  }
#د_که_پای
  أرجع aType[pVal->flags&MEM_Aff_ماسک];
}
صحيح sqlite3_ارزښت_encoding(sqlite3_ارزښت *pVal){
  أرجع pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
صحيح sqlite3_ارزښت_nochange(sqlite3_ارزښت *pVal){
  أرجع (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
صحيح sqlite3_ارزښت_frombind(sqlite3_ارزښت *pVal){
  أرجع (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_ارزښت *sqlite3_ارزښت_dup(ثابت sqlite3_ارزښت *pOrig){
  sqlite3_ارزښت *pNew;
  إذا( pOrig==0 ) أرجع 0;
  pNew = sqlite3_malloc( حجم(*pNew) );
  إذا( pNew==0 ) أرجع 0;
  memset(pNew, 0, حجم(*pNew));
  حافظه_نقل(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  إذا( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    إذا( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_ارزښت_Free(pNew);
      pNew = 0;
    }
  }وإلا إذا( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_غړی|MEM_Subtype);
  }
  أرجع pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
فراغ sqlite3_ارزښت_free(sqlite3_ارزښت *pOld){
  sqlite3_ارزښت_Free(pOld);
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
ساكن فراغ سېټ_پایله_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  ثابت حرف *z,          /* String pointer */
  صحيح n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  فراغ (*xDel)(فراغ*)     /* Destructor function */
){
  Mem *p_بهر = pCtx->p_بهر;
  صحيح rc;
  إذا( enc==SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_سېټ_متن(p_بهر, z, n, xDel);
  }وإلا إذا( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    أكد( z==0
         || n<0 
         || n>p_بهر->db->a_حد[SQLITE_حد_اوږدتیا]
         || z[n]==0
    );
    rc = sqlite3_Vdbe_Mem_سېټ_متن(p_بهر, z, n, xDel);
    p_بهر->flags |= MEM_غړی;
  }وإلا{
    rc = sqlite3_Vdbe_Mem_سېټ_Str(p_بهر, z, n, enc, xDel);
  }
  إذا( rc ){
    إذا( rc==SQLITE_TOOBIG ){
      sqlite3_پایله_error_toobig(pCtx);
    }وإلا{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      أكد( rc==SQLITE_NOMEM );
      sqlite3_پایله_error_nomem(pCtx);
    }
    أرجع;
  }
  sqlite3VdbeChangeEncoding(p_بهر, pCtx->enc);
  إذا( sqlite3VdbeMemTooBig(p_بهر) ){
    sqlite3_پایله_error_toobig(pCtx);
  }
}
ساكن صحيح invoke_ارزښت_Destructor(
  ثابت فراغ *p,             /* Value to destroy */
  فراغ (*xDel)(فراغ*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  أكد( xDel!=SQLITE_DYNAMIC );
  إذا( xDel==0 ){
    /* noop */
  }وإلا إذا( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }وإلا{
    xDel((فراغ*)p);
  }
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx!=0 ){
    sqlite3_پایله_error_toobig(pCtx);
  }
#وإلا
  أكد( pCtx!=0 );
  sqlite3_پایله_error_toobig(pCtx);
#د_که_پای
  أرجع SQLITE_TOOBIG;
}
فراغ sqlite3_پایله_blob(
  sqlite3_context *pCtx,
  ثابت فراغ *z,
  صحيح n,
  فراغ (*xDel)(فراغ *)
){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 || n<0 ){
    invoke_ارزښت_Destructor(z, xDel, pCtx);
    أرجع;
  }
#د_که_پای
  أكد( n>=0 );
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  سېټ_پایله_Str_Or_Error(pCtx, z, n, 0, xDel);
}
فراغ sqlite3_پایله_blob64(
  sqlite3_context *pCtx,
  ثابت فراغ *z,
  sqlite3_uint64 n,
  فراغ (*xDel)(فراغ *)
){
  أكد( xDel!=SQLITE_DYNAMIC );
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ){
    invoke_ارزښت_Destructor(z, xDel, 0);
    أرجع;
  }
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  إذا( n>0x7fffffff ){
    (فراغ)invoke_ارزښت_Destructor(z, xDel, pCtx);
  }وإلا{
    سېټ_پایله_Str_Or_Error(pCtx, z, (صحيح)n, 0, xDel);
  }
}
فراغ sqlite3_پایله_double(sqlite3_context *pCtx, مزدوج rVal){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  sqlite3_Vdbe_Mem_سېټ_Double(pCtx->p_بهر, rVal);
}
فراغ sqlite3_پایله_error(sqlite3_context *pCtx, ثابت حرف *z, صحيح n){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_سېټ_Str(pCtx->p_بهر, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
فراغ sqlite3_پایله_error16(sqlite3_context *pCtx, ثابت فراغ *z, صحيح n){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_سېټ_Str(pCtx->p_بهر, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#د_که_پای
فراغ sqlite3_پایله_int(sqlite3_context *pCtx, صحيح iVal){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  sqlite3_Vdbe_Mem_سېټ_Int64(pCtx->p_بهر, (i64)iVal);
}
فراغ sqlite3_پایله_int64(sqlite3_context *pCtx, i64 iVal){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  sqlite3_Vdbe_Mem_سېټ_Int64(pCtx->p_بهر, iVal);
}
فراغ sqlite3_پایله_null(sqlite3_context *pCtx){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  sqlite3_Vdbe_Mem_سېټ_Null(pCtx->p_بهر);
}
فراغ sqlite3_پایله_اشاره_ګر(
  sqlite3_context *pCtx,
  فراغ *pPtr,
  ثابت حرف *zPType,
  فراغ (*xDestructor)(فراغ*)
){
  Mem *p_بهر;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ){
    invoke_ارزښت_Destructor(pPtr, xDestructor, 0);
    أرجع;
  }
#د_که_پای
  p_بهر = pCtx->p_بهر;
  أكد( sqlite3_mutex_held(p_بهر->db->mutex) );
  sqlite3VdbeMemRelease(p_بهر);
  p_بهر->flags = MEM_Null;
  sqlite3_Vdbe_Mem_سېټ_اشاره_ګر(p_بهر, pPtr, zPType, xDestructor);
}
فراغ sqlite3_پایله_subtype(sqlite3_context *pCtx, بینښه صحيح eSubtype){
  Mem *p_بهر;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
#إذا defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  إذا( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_پایله_SUBTYPE)==0
  ){
    حرف zErr[200];
    sqlite3_snprintf(حجم(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_پایله_error(pCtx, zErr, -1);
    أرجع;
  }
#د_که_پای /* SQLITE_STRICT_SUBTYPE */
  p_بهر = pCtx->p_بهر;
  أكد( sqlite3_mutex_held(p_بهر->db->mutex) );
  p_بهر->eSubtype = eSubtype & 0xff;
  p_بهر->flags |= MEM_Subtype;
}
فراغ sqlite3_پایله_متن(
  sqlite3_context *pCtx,
  ثابت حرف *z,
  صحيح n,
  فراغ (*xDel)(فراغ *)
){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ){
    invoke_ارزښت_Destructor(z, xDel, 0);
    أرجع;
  }
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  سېټ_پایله_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
فراغ sqlite3_پایله_text64(
  sqlite3_context *pCtx,
  ثابت حرف *z,
  sqlite3_uint64 n,
  فراغ (*xDel)(فراغ *),
  بینښه حرف enc
){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ){
    invoke_ارزښت_Destructor(z, xDel, 0);
    أرجع;
  }
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  أكد( xDel!=SQLITE_DYNAMIC );
  إذا( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    إذا( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  إذا( n>0x7fffffff ){
    (فراغ)invoke_ارزښت_Destructor(z, xDel, pCtx);
  }وإلا{
    سېټ_پایله_Str_Or_Error(pCtx, z, (صحيح)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->p_بهر);
  }
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
فراغ sqlite3_پایله_text16(
  sqlite3_context *pCtx,
  ثابت فراغ *z,
  صحيح n,
  فراغ (*xDel)(فراغ *)
){
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  سېټ_پایله_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
فراغ sqlite3_پایله_text16be(
  sqlite3_context *pCtx,
  ثابت فراغ *z,
  صحيح n,
  فراغ (*xDel)(فراغ *)
){
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  سېټ_پایله_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
فراغ sqlite3_پایله_text16le(
  sqlite3_context *pCtx,
  ثابت فراغ *z,
  صحيح n,
  فراغ (*xDel)(فراغ *)
){
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  سېټ_پایله_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */
فراغ sqlite3_پایله_ارزښت(sqlite3_context *pCtx, sqlite3_ارزښت *p_ارزښت){
  Mem *p_بهر;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
  إذا( p_ارزښت==0 ){
    sqlite3_پایله_null(pCtx);
    أرجع;
  }
#د_که_پای
  p_بهر = pCtx->p_بهر;
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  sqlite3VdbeMemCopy(p_بهر, p_ارزښت);
  sqlite3VdbeChangeEncoding(p_بهر, pCtx->enc);
  إذا( sqlite3VdbeMemTooBig(p_بهر) ){
    sqlite3_پایله_error_toobig(pCtx);
  }
}
فراغ sqlite3_پایله_zeroblob(sqlite3_context *pCtx, صحيح n){
  sqlite3_پایله_zeroblob64(pCtx, n>0 ? n : 0);
}
صحيح sqlite3_پایله_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *p_بهر;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع SQLITE_MISUSE_BKPT;
#د_که_پای
  p_بهر = pCtx->p_بهر;
  أكد( sqlite3_mutex_held(p_بهر->db->mutex) );
  إذا( n>(u64)p_بهر->db->a_حد[SQLITE_حد_اوږدتیا] ){
    sqlite3_پایله_error_toobig(pCtx);
    أرجع SQLITE_TOOBIG;
  }
#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB
  sqlite3_Vdbe_Mem_سېټ_Zero_Blob(pCtx->p_بهر, (صحيح)n);
  أرجع SQLITE_OK;
#وإلا
  أرجع sqlite3_Vdbe_Mem_سېټ_Zero_Blob(pCtx->p_بهر, (صحيح)n);
#د_که_پای
}
فراغ sqlite3_پایله_error_کوډ(sqlite3_context *pCtx, صحيح err_کوډ){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  pCtx->isError = err_کوډ ? err_کوډ : -1;
#که_تعریف_شوی SQLITE_DEBUG
  إذا( pCtx->pVdbe ) pCtx->pVdbe->rcApp = err_کوډ;
#د_که_پای
  إذا( pCtx->p_بهر->flags & MEM_Null ){
    سېټ_پایله_Str_Or_Error(pCtx, sqlite3ErrStr(err_کوډ), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
فراغ sqlite3_پایله_error_toobig(sqlite3_context *pCtx){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3_Vdbe_Mem_سېټ_Str(pCtx->p_بهر, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
فراغ sqlite3_پایله_error_nomem(sqlite3_context *pCtx){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  sqlite3_Vdbe_Mem_سېټ_Null(pCtx->p_بهر);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->p_بهر->db);
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
فراغ sqlite3_پایله_str(sqlite3_context *pCtx, sqlite3_str *pStr, صحيح eOwn){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
  إذا( pStr==0 ) أرجع;
#د_که_پای
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  إذا( pStr->accError==0 ){
    إذا( pStr->nChar==0 ){
      سېټ_پایله_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }وإلا{
      ثابت حرف *z_متن = sqlite3_str_ارزښت(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      أكد( isMalloced(pStr) );
      إذا( eOwn==SQLITE_COPY ){
        سېټ_پایله_Str_Or_Error(pCtx, z_متن, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }وإلا{
        سېټ_پایله_Str_Or_Error(pCtx, z_متن, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }وإلا إذا( pStr->accError==SQLITE_NOMEM ){
    sqlite3_پایله_error_nomem(pCtx);
  }وإلا{
    أكد( pStr->accError==SQLITE_TOOBIG );
    sqlite3_پایله_error_toobig(pCtx);
  }
  إذا( eOwn==SQLITE_پای ){
    sqlite3_str_free(pStr);
  }
}

#که_نه_تعریف_شوی SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
فراغ sqlite3_پایله_Int_Real(sqlite3_context *pCtx){
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
  إذا( pCtx->p_بهر->flags & MEM_Int ){
    pCtx->p_بهر->flags &= ~MEM_Int;
    pCtx->p_بهر->flags |= MEM_IntReal;
  }
}
#د_که_پای


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ساكن صحيح doWalCallbacks(sqlite3 *db){
  صحيح rc = SQLITE_OK;
#که_نه_تعریف_شوی SQLITE_OMIT_WAL
  صحيح i;
  لكل(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    إذا( pBt ){
      صحيح n_ثبت;
      sqlite3BtreeEnter(pBt);
      n_ثبت = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      إذا( n_ثبت>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, n_ثبت);
      }
    }
  }
#وإلا
  ناکارول_شوی_PARAMETER(db);
#د_که_پای
  أرجع rc;
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
ساكن صحيح sqlite3_ګام_2(Vdbe *p){
  sqlite3 *db;
  صحيح rc;

  أكد(p);
  db = p->db;
  إذا( p->eVdbeState!=VDBE_پرلپسې_STATE ){
    restart_ګام:
    إذا( p->eVdbeState==VDBE_READY_STATE ){
      إذا( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        إذا( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        اذهب پای_ټکی_د_ګام;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      إذا( db->n_Vdbe_فعال==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      أكد( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
      إذا( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3_Os_Current_وخت_Int64(db->pVfs, &p->پیل_وخت);
      }وإلا{
        أكد( p->پیل_وخت==0 );
      }
#د_که_پای

      db->n_Vdbe_فعال++;
      إذا( p->readOnly==0 ) db->nVdbeWrite++;
      إذا( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_پرلپسې_STATE;
    }وإلا

    إذا( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
#که_تعریف_شوی SQLITE_OMIT_AUTORESET
      إذا( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }وإلا{
        أرجع SQLITE_MISUSE_BKPT;
      }
#وإلا
      sqlite3_reset((sqlite3_stmt*)p);
#د_که_پای
      أكد( p->eVdbeState==VDBE_READY_STATE );
      اذهب restart_ګام;
    }
  }

#که_تعریف_شوی SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
  إذا( p->explain ){
    rc = sqlite3VdbeList(p);
  }وإلا
#د_که_پای /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  إذا( rc==SQLITE_لیکه ){
    أكد( p->rc==SQLITE_OK );
    أكد( db->mallocFailed==0 );
    db->err_کوډ = SQLITE_لیکه;
    أرجع SQLITE_لیکه;
  }وإلا{
#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#د_که_پای
    p->p_پایله_لیکه = 0;
    إذا( rc==SQLITE_بشپړ && db->autoCommit ){
      أكد( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      إذا( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }وإلا إذا( rc!=SQLITE_بشپړ && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->err_کوډ = rc;
  إذا( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    إذا( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
پای_ټکی_د_ګام:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_لیکه  || rc==SQLITE_بشپړ   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  أرجع (rc&db->err_ماسک);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
صحيح sqlite3_ګام(sqlite3_stmt *pStmt){
  صحيح rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  صحيح cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  إذا( vdbeSafetyNotNull(v) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  طالما( (rc = sqlite3_ګام_2(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    صحيح savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    إذا( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ثابت حرف *zErr = (ثابت حرف *)sqlite3_ارزښت_متن(db->pErr);
      sqlite3DbFree(db, v->z_Err_پیغام);
      إذا( !db->mallocFailed ){
        v->z_Err_پیغام = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } وإلا {
        v->z_Err_پیغام = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      اكسر;
    }
    sqlite3_reset(pStmt);
    إذا( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    أكد( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
فراغ *sqlite3_user_معلومات(sqlite3_context *p){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ) أرجع 0;
#د_که_پای
  أكد( p && p->pFunc );
  أرجع p->pFunc->p_User_معلومات;
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
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ) أرجع 0;
#وإلا
  أكد( p && p->p_بهر );
#د_که_پای
  أرجع p->p_بهر->db;
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
صحيح sqlite3_vtab_nochange(sqlite3_context *p){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ) أرجع 0;
#وإلا
  أكد( p );
#د_که_پای
  أرجع sqlite3_ارزښت_nochange(p->p_بهر);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
فراغ sqlite3_Vdbe_ارزښت_List_Free(فراغ *p_منزل_Delete){
  sqlite3_free(p_منزل_Delete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ساكن صحيح ارزښت_From_ارزښت_List(
  sqlite3_ارزښت *pVal,        /* Pointer to the ValueList object */
  sqlite3_ارزښت **pp_بهر,      /* Store the next value from the list here */
  صحيح b_بل                   /* 1 for _next(). 0 for _first() */
){
  صحيح rc;
  ارزښت_List *pRhs;

  *pp_بهر = 0;
  إذا( pVal==0 ) أرجع SQLITE_MISUSE_BKPT;
  إذا( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_ارزښت_List_Free ){
    أرجع SQLITE_ERROR;
  }وإلا{
    أكد( (pVal->flags&(MEM_Type_ماسک|MEM_غړی|MEM_Subtype)) ==
                 (MEM_Null|MEM_غړی|MEM_Subtype) );
    أكد( pVal->eSubtype=='p' );
    أكد( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (ارزښت_List*)pVal->z;
  }
  إذا( b_بل ){
    rc = sqlite3_Btree_بل(pRhs->pCsr, 0);
  }وإلا{
    صحيح dummy = 0;
    rc = sqlite3_Btree_لومړی(pRhs->pCsr, &dummy);
    أكد( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    إذا( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_بشپړ;
  }
  إذا( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, حجم(sMem));
    sz = sqlite3_Btree_Payload_اندازه(pRhs->pCsr);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_دننه_واټن(pRhs->pCsr,sz,&sMem);
    إذا( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_ارزښت *p_بهر = pRhs->p_بهر;
      صحيح iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, p_بهر);
      p_بهر->enc = ENC(p_بهر->db);
      إذا( (p_بهر->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(p_بهر) ){
        rc = SQLITE_NOMEM;
      }وإلا{
        *pp_بهر = p_بهر;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  أرجع rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
صحيح sqlite3_vtab_دننه_لومړی(sqlite3_ارزښت *pVal, sqlite3_ارزښت **pp_بهر){
  أرجع ارزښت_From_ارزښت_List(pVal, pp_بهر, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
صحيح sqlite3_vtab_دننه_بل(sqlite3_ارزښت *pVal, sqlite3_ارزښت **pp_بهر){
  أرجع ارزښت_From_ارزښت_List(pVal, pp_بهر, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3_Stmt_Current_وخت(sqlite3_context *p){
  صحيح rc;
#که_نه_تعریف_شوی SQLITE_ENABLE_STAT4
  sqlite3_int64 *pi_وخت = &p->pVdbe->i_Current_وخت;
  أكد( p->pVdbe!=0 );
#وإلا
  sqlite3_int64 i_وخت = 0;
  sqlite3_int64 *pi_وخت = p->pVdbe!=0 ? &p->pVdbe->i_Current_وخت : &i_وخت;
#د_که_پای
  إذا( *pi_وخت==0 ){
    rc = sqlite3_Os_Current_وخت_Int64(p->p_بهر->db->pVfs, pi_وخت);
    إذا( rc ) *pi_وخت = 0;
  }
  أرجع *pi_وخت;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ساكن SQLITE_NOINLINE فراغ *createAggContext(sqlite3_context *p, صحيح nByte){
  Mem *pMem = p->pMem;
  أكد( (pMem->flags & MEM_Agg)==0 );
  إذا( nByte<=0 ){
    sqlite3_Vdbe_Mem_سېټ_Null(pMem);
    pMem->z = 0;
  }وإلا{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    إذا( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  أرجع (فراغ*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
فراغ *sqlite3_aggregate_context(sqlite3_context *p, صحيح nByte){
  أكد( p && p->pFunc && p->pFunc->xFinalize );
  أكد( sqlite3_mutex_held(p->p_بهر->db->mutex) );
  testcase( nByte<0 );
  إذا( (p->pMem->flags & MEM_Agg)==0 ){
    أرجع createAggContext(p, nByte);
  }وإلا{
    أرجع (فراغ*)p->pMem->z;
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
فراغ *sqlite3_get_auxdata(sqlite3_context *pCtx, صحيح iArg){
  Aux_معلومات *p_Aux_معلومات;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع 0;
#د_که_پای
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
#إذا SQLITE_ENABLE_STAT4
  إذا( pCtx->pVdbe==0 ) أرجع 0;
#وإلا
  أكد( pCtx->pVdbe!=0 );
#د_که_پای
  لكل(p_Aux_معلومات=pCtx->pVdbe->p_Aux_معلومات; p_Aux_معلومات; p_Aux_معلومات=p_Aux_معلومات->p_بل_Aux){
    إذا(  p_Aux_معلومات->iAuxArg==iArg && (p_Aux_معلومات->iAuxOp==pCtx->iOp || iArg<0) ){
      أرجع p_Aux_معلومات->pAux;
    }
  }
  أرجع 0;
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
فراغ sqlite3_سېټ_auxdata(
  sqlite3_context *pCtx,
  صحيح iArg,
  فراغ *pAux,
  فراغ (*xDelete)(فراغ*)
){
  Aux_معلومات *p_Aux_معلومات;
  Vdbe *pVdbe;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pCtx==0 ) أرجع;
#د_که_پای
  pVdbe= pCtx->pVdbe;
  أكد( sqlite3_mutex_held(pCtx->p_بهر->db->mutex) );
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( pVdbe==0 ) اذهب failed;
#وإلا
  أكد( pVdbe!=0 );
#د_که_پای

  لكل(p_Aux_معلومات=pVdbe->p_Aux_معلومات; p_Aux_معلومات; p_Aux_معلومات=p_Aux_معلومات->p_بل_Aux){
    إذا( p_Aux_معلومات->iAuxArg==iArg && (p_Aux_معلومات->iAuxOp==pCtx->iOp || iArg<0) ){
      اكسر;
    }
  }
  إذا( p_Aux_معلومات==0 ){
    p_Aux_معلومات = sqlite3DbMallocZero(pVdbe->db, حجم(Aux_معلومات));
    إذا( !p_Aux_معلومات ) اذهب failed;
    p_Aux_معلومات->iAuxOp = pCtx->iOp;
    p_Aux_معلومات->iAuxArg = iArg;
    p_Aux_معلومات->p_بل_Aux = pVdbe->p_Aux_معلومات;
    pVdbe->p_Aux_معلومات = p_Aux_معلومات;
    إذا( pCtx->isError==0 ) pCtx->isError = -1;
  }وإلا إذا( p_Aux_معلومات->xDeleteAux ){
    p_Aux_معلومات->xDeleteAux(p_Aux_معلومات->pAux);
  }

  p_Aux_معلومات->pAux = pAux;
  p_Aux_معلومات->xDeleteAux = xDelete;
  أرجع;

failed:
  إذا( xDelete ){
    xDelete(pAux);
  }
}

#که_نه_تعریف_شوی SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
صحيح sqlite3_aggregate_شمېر(sqlite3_context *p){
  أكد( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  أرجع p->pMem->n;
}
#د_که_پای

/*
** Return the number of columns in the result set for the statement pStmt.
*/
صحيح sqlite3_ستنه_شمېر(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  إذا( pVm==0 ) أرجع 0;
  أرجع pVm->n_Res_ستنه;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
صحيح sqlite3_معلومات_شمېر(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  إذا( pVm==0 || pVm->p_پایله_لیکه==0 ) أرجع 0;
  أرجع pVm->n_Res_ستنه;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ساكن ثابت Mem *ستنه_Null_ارزښت(فراغ){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ساكن ثابت Mem nullMem
#إذا defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#د_که_پای
    = {
        /* .u          = */ {0},
        /* .z          = */ (حرف*)0,
        /* .n          = */ (صحيح)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (صحيح)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (حرف*)0,
        /* .xDel       = */ (فراغ(*)(فراغ*))0,
#که_تعریف_شوی SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#د_که_پای
      };
  أرجع &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ساكن Mem *ستنه_Mem(sqlite3_stmt *pStmt, صحيح i){
  Vdbe *pVm;
  Mem *p_بهر;

  pVm = (Vdbe *)pStmt;
  إذا( pVm==0 ) أرجع (Mem*)ستنه_Null_ارزښت();
  أكد( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  إذا( pVm->p_پایله_لیکه!=0 && i<pVm->n_Res_ستنه && i>=0 ){
    p_بهر = &pVm->p_پایله_لیکه[i];
  }وإلا{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    p_بهر = (Mem*)ستنه_Null_ارزښت();
  }
  أرجع p_بهر;
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
ساكن فراغ ستنه_Malloc_Failure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  إذا( p ){
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
ثابت فراغ *sqlite3_ستنه_blob(sqlite3_stmt *pStmt, صحيح i){
  ثابت فراغ *val;
  val = sqlite3_ارزښت_blob( ستنه_Mem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
صحيح sqlite3_ستنه_بایټونه(sqlite3_stmt *pStmt, صحيح i){
  صحيح val = sqlite3_ارزښت_بایټونه( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
صحيح sqlite3_ستنه_bytes16(sqlite3_stmt *pStmt, صحيح i){
  صحيح val = sqlite3_ارزښت_bytes16( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
مزدوج sqlite3_ستنه_double(sqlite3_stmt *pStmt, صحيح i){
  مزدوج val = sqlite3_ارزښت_double( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
صحيح sqlite3_ستنه_int(sqlite3_stmt *pStmt, صحيح i){
  صحيح val = sqlite3_ارزښت_int( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
sqlite_int64 sqlite3_ستنه_int64(sqlite3_stmt *pStmt, صحيح i){
  sqlite_int64 val = sqlite3_ارزښت_int64( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
ثابت بینښه حرف *sqlite3_ستنه_متن(sqlite3_stmt *pStmt, صحيح i){
  ثابت بینښه حرف *val = sqlite3_ارزښت_متن( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
sqlite3_ارزښت *sqlite3_ستنه_ارزښت(sqlite3_stmt *pStmt, صحيح i){
  Mem *p_بهر = ستنه_Mem(pStmt, i);
  إذا( p_بهر->flags&MEM_Static ){
    p_بهر->flags &= ~MEM_Static;
    p_بهر->flags |= MEM_Ephem;
  }
  ستنه_Malloc_Failure(pStmt);
  أرجع (sqlite3_ارزښت *)p_بهر;
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ستنه_text16(sqlite3_stmt *pStmt, صحيح i){
  ثابت فراغ *val = sqlite3_ارزښت_text16( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع val;
}
#د_که_پای /* SQLITE_OMIT_UTF16 */
صحيح sqlite3_ستنه_type(sqlite3_stmt *pStmt, صحيح i){
  صحيح iType = sqlite3_ارزښت_type( ستنه_Mem(pStmt,i) );
  ستنه_Malloc_Failure(pStmt);
  أرجع iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ساكن ثابت حرف * ثابت az_Explain_ستون_Names8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ساكن ثابت u16 az_Explain_ستون_Names16data[] = {
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
ساكن ثابت u8 i_Explain_ستون_Names16[] = {
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
ساكن ثابت فراغ *ستنه_Name(
  sqlite3_stmt *pStmt,     /* The statement */
  صحيح N,                   /* Which column to get the name for */
  صحيح useUtf16,            /* True to return the name as UTF16 */
  صحيح useType              /* What type of name */
){
  ثابت فراغ *ret;
  Vdbe *p;
  صحيح n;
  sqlite3 *db;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pStmt==0 ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#د_که_پای
  إذا( N<0 ) أرجع 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  أكد( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  إذا( p->explain ){
    إذا( useType>0 ) اذهب ستنه_Name_پای_ټکی;
    n = p->explain==1 ? 8 : 4;
    إذا( N>=n ) اذهب ستنه_Name_پای_ټکی;
    إذا( useUtf16 ){
      صحيح i = i_Explain_ستون_Names16[N + 8*p->explain - 8];
      ret = (فراغ*)&az_Explain_ستون_Names16data[i];
    }وإلا{
      ret = (فراغ*)az_Explain_ستون_Names8[N + 8*p->explain - 8];
    }
    اذهب ستنه_Name_پای_ټکی;
  }
  n = p->n_Res_ستنه;
  إذا( N<n ){
    u8 مخکینی_malloc_Failed = db->mallocFailed;
    N += useType*n;
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
    إذا( useUtf16 ){
      ret = sqlite3_ارزښت_text16((sqlite3_ارزښت*)&p->a_ستون_Name[N]);
    }وإلا
#د_که_پای
    {
      ret = sqlite3_ارزښت_متن((sqlite3_ارزښت*)&p->a_ستون_Name[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    أكد( db->mallocFailed==0 || db->mallocFailed==1 );
    إذا( db->mallocFailed > مخکینی_malloc_Failed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
ستنه_Name_پای_ټکی:
  sqlite3_mutex_leave(db->mutex);
  أرجع ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ثابت حرف *sqlite3_ستنه_name(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 0, COLNAME_NAME);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ستنه_name16(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 1, COLNAME_NAME);
}
#د_که_پای

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#إذا defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_ستنه_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_ستنه_METADATA"
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ثابت حرف *sqlite3_ستنه_decltype(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 0, COLNAME_DECLTYPE);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ستنه_decltype16(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 1, COLNAME_DECLTYPE);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */
#د_که_پای /* SQLITE_OMIT_DECLTYPE */

#که_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت حرف *sqlite3_ستنه_database_name(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 0, COLNAME_DATABASE);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ستنه_database_name16(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 1, COLNAME_DATABASE);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت حرف *sqlite3_ستنه_جدول_name(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 0, COLNAME_جدول);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ستنه_جدول_name16(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 1, COLNAME_جدول);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت حرف *sqlite3_ستنه_origin_name(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 0, COLNAME_ستنه);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
ثابت فراغ *sqlite3_ستنه_origin_name16(sqlite3_stmt *pStmt, صحيح N){
  أرجع ستنه_Name(pStmt, N, 1, COLNAME_ستنه);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */
#د_که_پای /* SQLITE_ENABLE_COLUMN_METADATA */


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
ساكن صحيح vdbeUnbind(Vdbe *p, بینښه صحيح i){
  Mem *pVar;
  إذا( vdbeSafetyNotNull(p) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  إذا( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    أرجع SQLITE_MISUSE_BKPT;
  }
  إذا( i>=(بینښه صحيح)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    أرجع SQLITE_RANGE;
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
  إذا( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    إذا( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->err_کوډ = SQLITE_OK;

  أرجع SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ساكن صحيح bind_متن_2(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  صحيح i,                 /* Index of the parameter to bind */
  ثابت فراغ *z_معلومات,     /* Pointer to the data to be bound */
  i64 n_معلومات,             /* Number of bytes of data to be bound */
  فراغ (*xDel)(فراغ*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  صحيح rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  إذا( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    إذا( z_معلومات!=0 ){
      pVar = &p->aVar[i-1];
      إذا( encoding==SQLITE_UTF8 ){
        rc = sqlite3_Vdbe_Mem_سېټ_متن(pVar, z_معلومات, n_معلومات, xDel);
      }وإلا إذا( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        أكد( z_معلومات==0
             || n_معلومات<0 
             || n_معلومات>pVar->db->a_حد[SQLITE_حد_اوږدتیا]
             || ((u8*)z_معلومات)[n_معلومات]==0
        );
        rc = sqlite3_Vdbe_Mem_سېټ_متن(pVar, z_معلومات, n_معلومات, xDel);
        pVar->flags |= MEM_غړی;
      }وإلا{
        rc = sqlite3_Vdbe_Mem_سېټ_Str(pVar, z_معلومات, n_معلومات, encoding, xDel);
        إذا( encoding==0 ) pVar->enc = ENC(p->db);
      }
      إذا( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      إذا( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }وإلا إذا( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((فراغ*)z_معلومات);
  }
  أرجع rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
صحيح sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  صحيح i,
  ثابت فراغ *z_معلومات,
  صحيح n_معلومات,
  فراغ (*xDel)(فراغ*)
){
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( n_معلومات<0 ) أرجع SQLITE_MISUSE_BKPT;
#د_که_پای
  أرجع bind_متن_2(pStmt, i, z_معلومات, n_معلومات, xDel, 0);
}
صحيح sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  صحيح i,
  ثابت فراغ *z_معلومات,
  sqlite3_uint64 n_معلومات,
  فراغ (*xDel)(فراغ*)
){
  أكد( xDel!=SQLITE_DYNAMIC );
  أرجع bind_متن_2(pStmt, i, z_معلومات, n_معلومات, xDel, 0);
}
صحيح sqlite3_bind_double(sqlite3_stmt *pStmt, صحيح i, مزدوج r_ارزښت){
  صحيح rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  إذا( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_سېټ_Double(&p->aVar[i-1], r_ارزښت);
    sqlite3_mutex_leave(p->db->mutex);
  }
  أرجع rc;
}
صحيح sqlite3_bind_int(sqlite3_stmt *p, صحيح i, صحيح i_ارزښت){
  أرجع sqlite3_bind_int64(p, i, (i64)i_ارزښت);
}
صحيح sqlite3_bind_int64(sqlite3_stmt *pStmt, صحيح i, sqlite_int64 i_ارزښت){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  إذا( vdbeSafetyNotNull(p) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  إذا( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    أرجع SQLITE_MISUSE_BKPT;
  }
  إذا( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    أرجع SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  إذا( p->expmask!=0 ){
    u32 expire_ماسک = i>=32 ? 0x80000000 : (u32)1 << i;
    أكد( (p->expmask & p->smimask)==p->smimask );
    إذا( (p->expmask & expire_ماسک)!=0 ){
      إذا( (p->smimask & expire_ماسک)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        صحيح sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        صحيح sm2 = i_ارزښت>=0 && i_ارزښت<=1;
        إذا( sm1!=sm2 ){
          p->expired = 1;
        }
      }وإلا إذا( (pVar->flags & MEM_Int)==0 || i_ارزښت!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3_Vdbe_Mem_سېټ_Int64(pVar, i_ارزښت);
  sqlite3_mutex_leave(p->db->mutex);
  أرجع SQLITE_OK;
}
صحيح sqlite3_bind_null(sqlite3_stmt *pStmt, صحيح i){
  صحيح rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  إذا( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  أرجع rc;
}
صحيح sqlite3_bind_اشاره_ګر(
  sqlite3_stmt *pStmt,
  صحيح i,
  فراغ *pPtr,
  ثابت حرف *zPTtype,
  فراغ (*xDestructor)(فراغ*)
){
  صحيح rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  إذا( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_سېټ_اشاره_ګر(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }وإلا إذا( xDestructor ){
    xDestructor(pPtr);
  }
  أرجع rc;
}
صحيح sqlite3_bind_متن(
  sqlite3_stmt *pStmt,
  صحيح i,
  ثابت حرف *z_معلومات,
  صحيح n_معلومات,
  فراغ (*xDel)(فراغ*)
){
  أرجع bind_متن_2(pStmt, i, z_معلومات, n_معلومات, xDel, SQLITE_UTF8);
}
صحيح sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  صحيح i,
  ثابت حرف *z_معلومات,
  sqlite3_uint64 n_معلومات,
  فراغ (*xDel)(فراغ*),
  بینښه حرف enc
){
  أكد( xDel!=SQLITE_DYNAMIC );
  إذا( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    إذا( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_معلومات &= ~(u64)1;
  }
  أرجع bind_متن_2(pStmt, i, z_معلومات, n_معلومات, xDel, enc);
}
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
صحيح sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  صحيح i,
  ثابت فراغ *z_معلومات,
  صحيح n,
  فراغ (*xDel)(فراغ*)
){
  أرجع bind_متن_2(pStmt, i, z_معلومات, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#د_که_پای /* SQLITE_OMIT_UTF16 */
صحيح sqlite3_bind_ارزښت(sqlite3_stmt *pStmt, صحيح i, ثابت sqlite3_ارزښت *p_ارزښت){
  صحيح rc;
  بدل( sqlite3_ارزښت_type((sqlite3_ارزښت*)p_ارزښت) ){
    حالة SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_ارزښت->u.i);
      اكسر;
    }
    حالة SQLITE_FLOAT: {
      أكد( p_ارزښت->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_ارزښت->flags & MEM_Real) ? p_ارزښت->u.r : (مزدوج)p_ارزښت->u.i
      );
      اكسر;
    }
    حالة SQLITE_BLOB: {
      إذا( p_ارزښت->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_ارزښت->u.nZero);
      }وإلا{
        rc = sqlite3_bind_blob(pStmt, i, p_ارزښت->z, p_ارزښت->n,SQLITE_TRANSIENT);
      }
      اكسر;
    }
    حالة SQLITE_متن: {
      rc = bind_متن_2(pStmt,i,  p_ارزښت->z, p_ارزښت->n, SQLITE_TRANSIENT,
                              p_ارزښت->enc);
      اكسر;
    }
    افتراضي: {
      rc = sqlite3_bind_null(pStmt, i);
      اكسر;
    }
  }
  أرجع rc;
}
صحيح sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, صحيح i, صحيح n){
  صحيح rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  إذا( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB
    sqlite3_Vdbe_Mem_سېټ_Zero_Blob(&p->aVar[i-1], n);
#وإلا
    rc = sqlite3_Vdbe_Mem_سېټ_Zero_Blob(&p->aVar[i-1], n);
#د_که_پای
    sqlite3_mutex_leave(p->db->mutex);
  }
  أرجع rc;
}
صحيح sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, صحيح i, sqlite3_uint64 n){
  صحيح rc;
  Vdbe *p = (Vdbe *)pStmt;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ) أرجع SQLITE_MISUSE_BKPT;
#د_که_پای
  sqlite3_mutex_enter(p->db->mutex);
  إذا( n>(u64)p->db->a_حد[SQLITE_حد_اوږدتیا] ){
    rc = SQLITE_TOOBIG;
  }وإلا{
    أكد( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  أرجع rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
صحيح sqlite3_bind_parameter_شمېر(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  أرجع p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ثابت حرف *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, صحيح i){
  Vdbe *p = (Vdbe*)pStmt;
  إذا( p==0 ) أرجع 0;
  أرجع sqlite3_VList_عدد_منزل_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
صحيح sqlite3_Vdbe_Parameter_شاخص(Vdbe *p, ثابت حرف *zName, صحيح nName){
  إذا( p==0 || zName==0 ) أرجع 0;
  أرجع sqlite3_VList_Name_منزل_عدد(p->pVList, zName, nName);
}
صحيح sqlite3_bind_parameter_شاخص(sqlite3_stmt *pStmt, ثابت حرف *zName){
  أرجع sqlite3_Vdbe_Parameter_شاخص((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
صحيح sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_منزل_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_منزل = (Vdbe*)p_منزل_Stmt;
  صحيح i;
  أكد( p_منزل->db==pFrom->db );
  أكد( p_منزل->nVar==pFrom->nVar );
  sqlite3_mutex_enter(p_منزل->db->mutex);
  لكل(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&p_منزل->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(p_منزل->db->mutex);
  أرجع SQLITE_OK;
}

#که_نه_تعریف_شوی SQLITE_OMIT_DEPRECATED
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
صحيح sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_منزل_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_منزل = (Vdbe*)p_منزل_Stmt;
  إذا( pFrom->nVar!=p_منزل->nVar ){
    أرجع SQLITE_ERROR;
  }
  أكد( (p_منزل->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p_منزل->expmask==0 );
  إذا( p_منزل->expmask ){
    p_منزل->expired = 1;
  }
  أكد( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  إذا( pFrom->expmask ){
    pFrom->expired = 1;
  }
  أرجع sqlite3TransferBindings(pFromStmt, p_منزل_Stmt);
}
#د_که_پای

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  أرجع pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
صحيح sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  أرجع pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
صحيح sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  أرجع pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
صحيح sqlite3_stmt_explain(sqlite3_stmt *pStmt, صحيح e_ډېرپېښ){
  Vdbe *v = (Vdbe*)pStmt;
  صحيح rc;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( pStmt==0 ) أرجع SQLITE_MISUSE_BKPT;
#د_که_پای
  sqlite3_mutex_enter(v->db->mutex);
  إذا( ((صحيح)v->explain)==e_ډېرپېښ ){
    rc = SQLITE_OK;
  }وإلا إذا( e_ډېرپېښ<0 || e_ډېرپېښ>2 ){
    rc = SQLITE_ERROR;
  }وإلا إذا( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }وإلا إذا( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }وإلا إذا( v->nMem>=10 && (e_ډېرپېښ!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = e_ډېرپېښ;
    rc = SQLITE_OK;
  }وإلا{
    صحيح saved_ډېرپېښ = v->explain;
    v->explain = e_ډېرپېښ;
    rc = sqlite3Reprepare(v);
    إذا( rc ){
      v->explain = saved_ډېرپېښ;
    }وإلا{
      v->haveEqpOps = e_ډېرپېښ==2;
    }
  }
  إذا( v->explain ){
    v->n_Res_ستنه = 12 - 4*v->explain;
  }وإلا{
    v->n_Res_ستنه = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  أرجع rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
صحيح sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  أرجع v!=0 && v->eVdbeState==VDBE_پرلپسې_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_بل_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_بل;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(pDb) ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#د_که_پای
  sqlite3_mutex_enter(pDb->mutex);
  إذا( pStmt==0 ){
    p_بل = (sqlite3_stmt*)pDb->pVdbe;
  }وإلا{
    p_بل = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  أرجع p_بل;
}

/*
** Return the value of a status counter for a prepared statement
*/
صحيح sqlite3_stmt_حالت(sqlite3_stmt *pStmt, صحيح op, صحيح resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_اندازه(pVdbe->aCounter)))
  ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#د_که_پای
  إذا( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pn_بایټونه_Freed = (صحيح*)&v;
    أكد( db->lookaside.p_پای_ټکی==db->lookaside.p_True_پای_ټکی );
    db->lookaside.p_پای_ټکی = db->lookaside.p_پیل;
    sqlite3VdbeDelete(pVdbe);
    db->pn_بایټونه_Freed = 0;
    db->lookaside.p_پای_ټکی = db->lookaside.p_True_پای_ټکی;
    sqlite3_mutex_leave(db->mutex);
  }وإلا{
    v = pVdbe->aCounter[op];
    إذا( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  أرجع (صحيح)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ثابت حرف *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  أرجع p ? p->zSql : 0;
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
حرف *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#که_تعریف_شوی SQLITE_OMIT_TRACE
  أرجع 0;
#وإلا
  حرف *z = 0;
  ثابت حرف *zSql = sqlite3_sql(pStmt);
  إذا( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  أرجع z;
#د_که_پای
}

#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ثابت حرف *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  إذا( p==0 ) أرجع 0;
  إذا( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  أرجع p->zNormSql;
}
#د_که_پای /* SQLITE_ENABLE_NORMALIZE */

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ساكن Unpacked_ریکارډ *vdbe_Unpack_ریکارډ(
  کلی_Info *p_کلی_Info,
  صحيح n_کلی,
  ثابت فراغ *p_کلی
){
  Unpacked_ریکارډ *pRet;           /* Return value */

  pRet = sqlite3_Vdbe_Alloc_Unpacked_ریکارډ(p_کلی_Info);
  إذا( pRet ){
    memset(pRet->aMem, 0, حجم(Mem)*(p_کلی_Info->n_کلی_ډګر+1));
    sqlite3_Vdbe_ریکارډ_Unpack(n_کلی, p_کلی, pRet);
  }
  أرجع pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
صحيح sqlite3_preupdate_old(sqlite3 *db, صحيح iIdx, sqlite3_ارزښت **pp_ارزښت){
  PreUpdate *p;
  Mem *pMem;
  صحيح rc = SQLITE_OK;
  صحيح iStore = 0;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( db==0 || pp_ارزښت==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#د_که_پای
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  إذا( !p || p->op==SQLITE_ورننباسه ){
    rc = SQLITE_MISUSE_BKPT;
    اذهب preupdate_old_بهر;
  }
  إذا( p->pPk ){
    iStore = sqlite3_جدول_ستنه_منزل_شاخص(p->pPk, iIdx);
  }وإلا إذا( iIdx >= p->pTab->n_ستون ){
    rc = SQLITE_MISUSE_BKPT;
    اذهب preupdate_old_بهر;
  }وإلا{
    iStore = sqlite3_جدول_ستنه_منزل_Storage(p->pTab, iIdx);
  }
  إذا( iStore>=p->pCsr->n_ډګر || iStore<0 ){
    rc = SQLITE_RANGE;
    اذهب preupdate_old_بهر;
  }

  إذا( iIdx==p->pTab->iPKey ){
    *pp_ارزښت = pMem = &p->oldipk;
    sqlite3_Vdbe_Mem_سېټ_Int64(pMem, p->iKey1);
  }وإلا{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    إذا( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      أكد( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_اندازه(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      إذا( !aRec ) اذهب preupdate_old_بهر;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      إذا( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbe_Unpack_ریکارډ(p->pKeyinfo, nRec, aRec);
        إذا( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      إذا( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        اذهب preupdate_old_بهر;
      }
      p->a_ریکارډ = aRec;
    }

    pMem = *pp_ارزښت = &p->pUnpacked->aMem[iStore];
    إذا( iStore>=p->pUnpacked->n_ډګر ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      ستنه *p_ستون = &p->pTab->a_ستون[iIdx];
      إذا( p_ستون->iDflt>0 ){
        إذا( p->apDflt==0 ){
          صحيح nByte;
          أكد( حجم(sqlite3_ارزښت*)*UMXV(p->pTab->n_ستون) < 0x7fffffff );
          nByte = حجم(sqlite3_ارزښت*)*p->pTab->n_ستون;
          p->apDflt = (sqlite3_ارزښت**)sqlite3DbMallocZero(db, nByte);
          إذا( p->apDflt==0 ) اذهب preupdate_old_بهر;
        }
        إذا( p->apDflt[iIdx]==0 ){
          sqlite3_ارزښت *pVal = 0;
          بیان *pDflt;
          أكد( p->pTab!=0 && Is_Ordinary_جدول(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[p_ستون->iDflt-1].p_بیان;
          rc = sqlite3_ارزښت_From_بیان(db, pDflt, ENC(db), p_ستون->affinity, &pVal);
          إذا( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_ارزښت = p->apDflt[iIdx];
      }وإلا{
        *pp_ارزښت = (sqlite3_ارزښت *)ستنه_Null_ارزښت();
      }
    }وإلا إذا( p->pTab->a_ستون[iIdx].affinity==SQLITE_AFF_REAL ){
      إذا( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_بهر:
  sqlite3Error(db, rc);
  أرجع sqlite3ApiExit(db, rc);
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
صحيح sqlite3_preupdate_شمېر(sqlite3 *db){
  PreUpdate *p;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#وإلا
  p = db->pPreUpdate;
#د_که_پای
  أرجع (p ? p->pKeyinfo->n_کلی_ډګر : 0);
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
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
صحيح sqlite3_preupdate_ژوروالی(sqlite3 *db){
  PreUpdate *p;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#وإلا
  p = db->pPreUpdate;
#د_که_پای
  أرجع (p ? p->v->n_چوکاټ : 0);
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
صحيح sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#وإلا
  p = db->pPreUpdate;
#د_که_پای
  أرجع (p ? p->iBlobWrite : -1);
}
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
صحيح sqlite3_preupdate_new(sqlite3 *db, صحيح iIdx, sqlite3_ارزښت **pp_ارزښت){
  PreUpdate *p;
  صحيح rc = SQLITE_OK;
  Mem *pMem;
  صحيح iStore = 0;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( db==0 || pp_ارزښت==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#د_که_پای
  p = db->pPreUpdate;
  إذا( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    اذهب preupdate_new_بهر;
  }
  إذا( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_جدول_ستنه_منزل_شاخص(p->pPk, iIdx);
  }وإلا إذا( iIdx >= p->pTab->n_ستون ){
    أرجع SQLITE_MISUSE_BKPT;
  }وإلا{
    iStore = sqlite3_جدول_ستنه_منزل_Storage(p->pTab, iIdx);
  }

  إذا( iStore>=p->pCsr->n_ډګر || iStore<0 ){
    rc = SQLITE_RANGE;
    اذهب preupdate_new_بهر;
  }

  إذا( p->op==SQLITE_ورننباسه ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    Unpacked_ریکارډ *pUnpack = p->pNewUnpacked;
    إذا( !pUnpack ){
      Mem *p_معلومات = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_معلومات);
      إذا( rc!=SQLITE_OK ) اذهب preupdate_new_بهر;
      pUnpack = vdbe_Unpack_ریکارډ(p->pKeyinfo, p_معلومات->n, p_معلومات->z);
      إذا( !pUnpack ){
        rc = SQLITE_NOMEM;
        اذهب preupdate_new_بهر;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    إذا( iIdx==p->pTab->iPKey ){
      sqlite3_Vdbe_Mem_سېټ_Int64(pMem, p->iKey2);
    }وإلا إذا( iStore>=pUnpack->n_ډګر ){
      pMem = (sqlite3_ارزښت *)ستنه_Null_ارزښت();
    }وإلا إذا( (p->pTab->a_ستون)
           && (p->pTab->a_ستون[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }وإلا{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    أكد( p->op==SQLITE_UPDATE );
    إذا( !p->aNew ){
      أكد( حجم(Mem)*UMXV(p->pCsr->n_ډګر) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, حجم(Mem)*p->pCsr->n_ډګر);
      إذا( !p->aNew ){
        rc = SQLITE_NOMEM;
        اذهب preupdate_new_بهر;
      }
    }
    أكد( iStore>=0 && iStore<p->pCsr->n_ډګر );
    pMem = &p->aNew[iStore];
    إذا( pMem->flags==0 ){
      إذا( iIdx==p->pTab->iPKey ){
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem, p->iKey2);
      }وإلا{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        إذا( rc!=SQLITE_OK ) اذهب preupdate_new_بهر;
      }
    }
  }
  *pp_ارزښت = pMem;

 preupdate_new_بهر:
  sqlite3Error(db, rc);
  أرجع sqlite3ApiExit(db, rc);
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
صحيح sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  صحيح i_لټون,                      /* Index of loop to report on */
  صحيح i_لټون_حالت_Op,              /* Which metric to return */
  صحيح flags,
  فراغ *p_بهر                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  صحيح nOp;
  لټون_حالت *p_لټون = 0;
  صحيح idx;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( p==0 || p_بهر==0
      || i_لټون_حالت_Op<SQLITE_SCANSTAT_NLOOP
      || i_لټون_حالت_Op>SQLITE_SCANSTAT_NCYCLE ){
    أرجع 1;
  }
#د_که_پای
  aOp = p->aOp;
  nOp = p->nOp;
  إذا( p->p_چوکاټ ){
    Vdbe_چوکاټ *p_چوکاټ;
    لكل(p_چوکاټ=p->p_چوکاټ; p_چوکاټ->p_پلار; p_چوکاټ=p_چوکاټ->p_پلار);
    aOp = p_چوکاټ->aOp;
    nOp = p_چوکاټ->nOp;
  }

  إذا( i_لټون<0 ){
    صحيح ii;
    إذا( i_لټون_حالت_Op==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      لكل(ii=0; ii<nOp; ii++){
        res += aOp[ii].n_دوره;
      }
      *(i64*)p_بهر = res;
      أرجع 0;
    }
    أرجع 1;
  }
  إذا( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = i_لټون;
  }وإلا{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    لكل(idx=0; idx<p->n_لټون; idx++){
      p_لټون = &p->a_لټون[idx];
      إذا( p_لټون->zName ){
        i_لټون--;
        إذا( i_لټون<0 ) اكسر;
      }
    }
  }
  إذا( idx>=p->n_لټون ) أرجع 1;
  أكد( p_لټون==0 || p_لټون==&p->a_لټون[idx] );
  p_لټون = &p->a_لټون[idx];

  بدل( i_لټون_حالت_Op ){
    حالة SQLITE_SCANSTAT_NLOOP: {
      إذا( p_لټون->addrLoop>0 ){
        *(sqlite3_int64*)p_بهر = aOp[p_لټون->addrLoop].nExec;
      }وإلا{
        *(sqlite3_int64*)p_بهر = -1;
      }
      اكسر;
    }
    حالة SQLITE_SCANSTAT_NVISIT: {
      إذا( p_لټون->addr_لیدنه>0 ){
        *(sqlite3_int64*)p_بهر = aOp[p_لټون->addr_لیدنه].nExec;
      }وإلا{
        *(sqlite3_int64*)p_بهر = -1;
      }
      اكسر;
    }
    حالة SQLITE_SCANSTAT_EST: {
      مزدوج r = 1.0;
      LogEst x = p_لټون->nEst;
      طالما( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(مزدوج*)p_بهر = r*sqlite3_Log_Est_منزل_Int(x);
      اكسر;
    }
    حالة SQLITE_SCANSTAT_NAME: {
      *(ثابت حرف**)p_بهر = p_لټون->zName;
      اكسر;
    }
    حالة SQLITE_SCANSTAT_EXPLAIN: {
      إذا( p_لټون->addrExplain ){
        *(ثابت حرف**)p_بهر = aOp[ p_لټون->addrExplain ].p4.z;
      }وإلا{
        *(ثابت حرف**)p_بهر = 0;
      }
      اكسر;
    }
    حالة SQLITE_SCANSTAT_SELECTID: {
      إذا( p_لټون->addrExplain ){
        *(صحيح*)p_بهر = aOp[ p_لټون->addrExplain ].p1;
      }وإلا{
        *(صحيح*)p_بهر = -1;
      }
      اكسر;
    }
    حالة SQLITE_SCANSTAT_PARENTID: {
      إذا( p_لټون->addrExplain ){
        *(صحيح*)p_بهر = aOp[ p_لټون->addrExplain ].p2;
      }وإلا{
        *(صحيح*)p_بهر = -1;
      }
      اكسر;
    }
    حالة SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      إذا( p_لټون->aAddrRange[0]==0 ){
        res = -1;
      }وإلا{
        صحيح ii;
        لكل(ii=0; ii<Array_اندازه(p_لټون->aAddrRange); ii+=2){
          صحيح iIns = p_لټون->aAddrRange[ii];
          صحيح i_پای_ټکی = p_لټون->aAddrRange[ii+1];
          إذا( iIns==0 ) اكسر;
          إذا( iIns>0 ){
            طالما( iIns<=i_پای_ټکی ){
              res += aOp[iIns].n_دوره;
              iIns++;
            }
          }وإلا{
            صحيح iOp;
            لكل(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              إذا( pOp->p1!=i_پای_ټکی ) استمر;
              إذا( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                استمر;
              }
              res += aOp[iOp].n_دوره;
            }
          }
        }
      }
      *(i64*)p_بهر = res;
      اكسر;
    }
    افتراضي: {
      أرجع 1;
    }
  }
  أرجع 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
صحيح sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  صحيح i_لټون,                      /* Index of loop to report on */
  صحيح i_لټون_حالت_Op,              /* Which metric to return */
  فراغ *p_بهر                      /* OUT: Write the answer here */
){
  أرجع sqlite3_stmt_scanstatus_v2(pStmt, i_لټون, i_لټون_حالت_Op, 0, p_بهر);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
فراغ sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  صحيح ii;
  لكل(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->n_دوره = 0;
  }
}
#د_که_پای /* SQLITE_ENABLE_STMT_SCANSTATUS */
