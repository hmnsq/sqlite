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

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
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
  واپس iRet;
}
#اگر_ختم

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ساکن صحیح vdbeSafety(Vdbe *p){
  اگر( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    واپس 1;
  }ورنہ{
    واپس 0;
  }
}
ساکن صحیح vdbeSafetyNotNull(Vdbe *p){
  اگر( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    واپس 1;
  }ورنہ{
    واپس vdbeSafety(p);
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ساکن SQLITE_NOINLINE خالی invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  أكد( p->آغاز_وقت>0 );
  أكد( db->init.busy==0 );
  أكد( p->zSql!=0 );
  sqlite3_Os_Current_وقت_Int64(db->pVfs, &iNow);
  iElapse = (iNow - p->آغاز_وقت)*1000000;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  اگر( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#اگر_ختم
  اگر( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (خالی*)&iElapse);
  }
  p->آغاز_وقت = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   اگر( ((P)->آغاز_وقت)>0 ){ invokeProfileCallback(DB,P); }
#ورنہ
# define checkProfileCallback(DB,P)  /*no-op*/
#اگر_ختم

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
  }ورنہ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    اگر( vdbeSafety(v) ) واپس SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    أكد( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  واپس rc;
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
  }ورنہ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    أكد( (rc & (db->err_نقاب))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  واپس rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
صحیح sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  صحیح i;
  صحیح rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 نقاب_Small_Int = 0;  /* Bitmask of bindings with value 0 or 1 */
#اگر SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pStmt==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
#اگر SQLITE_THREADSAFE
  mutex = p->db->mutex;
#اگر_ختم

  sqlite3_mutex_enter(mutex);
  برائے(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    اگر( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      نقاب_Small_Int |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  اگر( (p->expmask&~p->smimask)!=0 || (p->smimask&نقاب_Small_Int)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  واپس rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ثابت خالی *sqlite3_قدر_blob(sqlite3_قدر *pVal){
  Mem *p = (Mem*)pVal;
  اگر( p->flags & (MEM_Blob|MEM_Str) ){
    اگر( ExpandBlob(p)!=SQLITE_OK ){
      أكد( p->flags==MEM_Null && p->z==0 );
      واپس 0;
    }
    p->flags |= MEM_Blob;
    واپس p->n ? p->z : 0;
  }ورنہ{
    واپس sqlite3_قدر_متن(pVal);
  }
}
صحیح sqlite3_قدر_بائٹیں(sqlite3_قدر *pVal){
  واپس sqlite3_قدر_بائٹیں_2(pVal, SQLITE_UTF8);
}
صحیح sqlite3_قدر_bytes16(sqlite3_قدر *pVal){
  واپس sqlite3_قدر_بائٹیں_2(pVal, SQLITE_UTF16NATIVE);
}
دوہرا sqlite3_قدر_double(sqlite3_قدر *pVal){
  واپس sqlite3_Vdbe_Real_قدر((Mem*)pVal);
}
صحیح sqlite3_قدر_int(sqlite3_قدر *pVal){
  واپس (صحیح)sqlite3_Vdbe_Int_قدر((Mem*)pVal);
}
sqlite_int64 sqlite3_قدر_int64(sqlite3_قدر *pVal){
  واپس sqlite3_Vdbe_Int_قدر((Mem*)pVal);
}
غیرعلامتی صحیح sqlite3_قدر_subtype(sqlite3_قدر *pVal){
  Mem *pMem = (Mem*)pVal;
  واپس ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
خالی *sqlite3_قدر_اشارہ_گر(sqlite3_قدر *pVal, ثابت حرف *zPType){
  Mem *p = (Mem*)pVal;
  اگر( (p->flags&(MEM_Type_نقاب|MEM_رکن|MEM_Subtype)) ==
                 (MEM_Null|MEM_رکن|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    واپس (خالی*)p->z;
  }ورنہ{
    واپس 0;
  }
}
ثابت غیرعلامتی حرف *sqlite3_قدر_متن(sqlite3_قدر *pVal){
  واپس (ثابت غیرعلامتی حرف *)sqlite3_قدر_متن_2(pVal, SQLITE_UTF8);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_قدر_text16(sqlite3_قدر* pVal){
  واپس sqlite3_قدر_متن_2(pVal, SQLITE_UTF16NATIVE);
}
ثابت خالی *sqlite3_قدر_text16be(sqlite3_قدر *pVal){
  واپس sqlite3_قدر_متن_2(pVal, SQLITE_UTF16BE);
}
ثابت خالی *sqlite3_قدر_text16le(sqlite3_قدر *pVal){
  واپس sqlite3_قدر_متن_2(pVal, SQLITE_UTF16LE);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
صحیح sqlite3_قدر_type(sqlite3_قدر* pVal){
  ساکن ثابت u8 aType[] = {
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
#اگر_تعریف_شدہ SQLITE_DEBUG
  {
    صحیح eType = SQLITE_BLOB;
    اگر( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }ورنہ اگر( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }ورنہ اگر( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }ورنہ اگر( pVal->flags & MEM_Str ){
      eType = SQLITE_متن;
    }
    أكد( eType == aType[pVal->flags&MEM_Aff_نقاب] );
  }
#اگر_ختم
  واپس aType[pVal->flags&MEM_Aff_نقاب];
}
صحیح sqlite3_قدر_encoding(sqlite3_قدر *pVal){
  واپس pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
صحیح sqlite3_قدر_nochange(sqlite3_قدر *pVal){
  واپس (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
صحیح sqlite3_قدر_frombind(sqlite3_قدر *pVal){
  واپس (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_قدر *sqlite3_قدر_dup(ثابت sqlite3_قدر *pOrig){
  sqlite3_قدر *pNew;
  اگر( pOrig==0 ) واپس 0;
  pNew = sqlite3_malloc( حجم(*pNew) );
  اگر( pNew==0 ) واپس 0;
  memset(pNew, 0, حجم(*pNew));
  حافظہ_نقل(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  اگر( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    اگر( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_قدر_Free(pNew);
      pNew = 0;
    }
  }ورنہ اگر( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_رکن|MEM_Subtype);
  }
  واپس pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
خالی sqlite3_قدر_free(sqlite3_قدر *pOld){
  sqlite3_قدر_Free(pOld);
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
ساکن خالی سیٹ_نتیجہ_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  ثابت حرف *z,          /* String pointer */
  صحیح n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  خالی (*xDel)(خالی*)     /* Destructor function */
){
  Mem *p_برآمد = pCtx->p_برآمد;
  صحیح rc;
  اگر( enc==SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_سیٹ_متن(p_برآمد, z, n, xDel);
  }ورنہ اگر( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    أكد( z==0
         || n<0 
         || n>p_برآمد->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]
         || z[n]==0
    );
    rc = sqlite3_Vdbe_Mem_سیٹ_متن(p_برآمد, z, n, xDel);
    p_برآمد->flags |= MEM_رکن;
  }ورنہ{
    rc = sqlite3_Vdbe_Mem_سیٹ_Str(p_برآمد, z, n, enc, xDel);
  }
  اگر( rc ){
    اگر( rc==SQLITE_TOOBIG ){
      sqlite3_نتیجہ_error_toobig(pCtx);
    }ورنہ{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      أكد( rc==SQLITE_NOMEM );
      sqlite3_نتیجہ_error_nomem(pCtx);
    }
    واپس;
  }
  sqlite3VdbeChangeEncoding(p_برآمد, pCtx->enc);
  اگر( sqlite3VdbeMemTooBig(p_برآمد) ){
    sqlite3_نتیجہ_error_toobig(pCtx);
  }
}
ساکن صحیح invoke_قدر_Destructor(
  ثابت خالی *p,             /* Value to destroy */
  خالی (*xDel)(خالی*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  أكد( xDel!=SQLITE_DYNAMIC );
  اگر( xDel==0 ){
    /* noop */
  }ورنہ اگر( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }ورنہ{
    xDel((خالی*)p);
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx!=0 ){
    sqlite3_نتیجہ_error_toobig(pCtx);
  }
#ورنہ
  أكد( pCtx!=0 );
  sqlite3_نتیجہ_error_toobig(pCtx);
#اگر_ختم
  واپس SQLITE_TOOBIG;
}
خالی sqlite3_نتیجہ_blob(
  sqlite3_context *pCtx,
  ثابت خالی *z,
  صحیح n,
  خالی (*xDel)(خالی *)
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 || n<0 ){
    invoke_قدر_Destructor(z, xDel, pCtx);
    واپس;
  }
#اگر_ختم
  أكد( n>=0 );
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  سیٹ_نتیجہ_Str_Or_Error(pCtx, z, n, 0, xDel);
}
خالی sqlite3_نتیجہ_blob64(
  sqlite3_context *pCtx,
  ثابت خالی *z,
  sqlite3_uint64 n,
  خالی (*xDel)(خالی *)
){
  أكد( xDel!=SQLITE_DYNAMIC );
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_قدر_Destructor(z, xDel, 0);
    واپس;
  }
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  اگر( n>0x7fffffff ){
    (خالی)invoke_قدر_Destructor(z, xDel, pCtx);
  }ورنہ{
    سیٹ_نتیجہ_Str_Or_Error(pCtx, z, (صحیح)n, 0, xDel);
  }
}
خالی sqlite3_نتیجہ_double(sqlite3_context *pCtx, دوہرا rVal){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  sqlite3_Vdbe_Mem_سیٹ_Double(pCtx->p_برآمد, rVal);
}
خالی sqlite3_نتیجہ_error(sqlite3_context *pCtx, ثابت حرف *z, صحیح n){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_سیٹ_Str(pCtx->p_برآمد, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
خالی sqlite3_نتیجہ_error16(sqlite3_context *pCtx, ثابت خالی *z, صحیح n){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_سیٹ_Str(pCtx->p_برآمد, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#اگر_ختم
خالی sqlite3_نتیجہ_int(sqlite3_context *pCtx, صحیح iVal){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  sqlite3_Vdbe_Mem_سیٹ_Int64(pCtx->p_برآمد, (i64)iVal);
}
خالی sqlite3_نتیجہ_int64(sqlite3_context *pCtx, i64 iVal){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  sqlite3_Vdbe_Mem_سیٹ_Int64(pCtx->p_برآمد, iVal);
}
خالی sqlite3_نتیجہ_null(sqlite3_context *pCtx){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  sqlite3_Vdbe_Mem_سیٹ_Null(pCtx->p_برآمد);
}
خالی sqlite3_نتیجہ_اشارہ_گر(
  sqlite3_context *pCtx,
  خالی *pPtr,
  ثابت حرف *zPType,
  خالی (*xDestructor)(خالی*)
){
  Mem *p_برآمد;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_قدر_Destructor(pPtr, xDestructor, 0);
    واپس;
  }
#اگر_ختم
  p_برآمد = pCtx->p_برآمد;
  أكد( sqlite3_mutex_held(p_برآمد->db->mutex) );
  sqlite3VdbeMemRelease(p_برآمد);
  p_برآمد->flags = MEM_Null;
  sqlite3_Vdbe_Mem_سیٹ_اشارہ_گر(p_برآمد, pPtr, zPType, xDestructor);
}
خالی sqlite3_نتیجہ_subtype(sqlite3_context *pCtx, غیرعلامتی صحیح eSubtype){
  Mem *p_برآمد;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
#اگر defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  اگر( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_نتیجہ_SUBTYPE)==0
  ){
    حرف zErr[200];
    sqlite3_snprintf(حجم(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_نتیجہ_error(pCtx, zErr, -1);
    واپس;
  }
#اگر_ختم /* SQLITE_STRICT_SUBTYPE */
  p_برآمد = pCtx->p_برآمد;
  أكد( sqlite3_mutex_held(p_برآمد->db->mutex) );
  p_برآمد->eSubtype = eSubtype & 0xff;
  p_برآمد->flags |= MEM_Subtype;
}
خالی sqlite3_نتیجہ_متن(
  sqlite3_context *pCtx,
  ثابت حرف *z,
  صحیح n,
  خالی (*xDel)(خالی *)
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_قدر_Destructor(z, xDel, 0);
    واپس;
  }
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  سیٹ_نتیجہ_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
خالی sqlite3_نتیجہ_text64(
  sqlite3_context *pCtx,
  ثابت حرف *z,
  sqlite3_uint64 n,
  خالی (*xDel)(خالی *),
  غیرعلامتی حرف enc
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ){
    invoke_قدر_Destructor(z, xDel, 0);
    واپس;
  }
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  أكد( xDel!=SQLITE_DYNAMIC );
  اگر( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    اگر( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  اگر( n>0x7fffffff ){
    (خالی)invoke_قدر_Destructor(z, xDel, pCtx);
  }ورنہ{
    سیٹ_نتیجہ_Str_Or_Error(pCtx, z, (صحیح)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->p_برآمد);
  }
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
خالی sqlite3_نتیجہ_text16(
  sqlite3_context *pCtx,
  ثابت خالی *z,
  صحیح n,
  خالی (*xDel)(خالی *)
){
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  سیٹ_نتیجہ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
خالی sqlite3_نتیجہ_text16be(
  sqlite3_context *pCtx,
  ثابت خالی *z,
  صحیح n,
  خالی (*xDel)(خالی *)
){
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  سیٹ_نتیجہ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
خالی sqlite3_نتیجہ_text16le(
  sqlite3_context *pCtx,
  ثابت خالی *z,
  صحیح n,
  خالی (*xDel)(خالی *)
){
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  سیٹ_نتیجہ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */
خالی sqlite3_نتیجہ_قدر(sqlite3_context *pCtx, sqlite3_قدر *p_قدر){
  Mem *p_برآمد;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
  اگر( p_قدر==0 ){
    sqlite3_نتیجہ_null(pCtx);
    واپس;
  }
#اگر_ختم
  p_برآمد = pCtx->p_برآمد;
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  sqlite3VdbeMemCopy(p_برآمد, p_قدر);
  sqlite3VdbeChangeEncoding(p_برآمد, pCtx->enc);
  اگر( sqlite3VdbeMemTooBig(p_برآمد) ){
    sqlite3_نتیجہ_error_toobig(pCtx);
  }
}
خالی sqlite3_نتیجہ_zeroblob(sqlite3_context *pCtx, صحیح n){
  sqlite3_نتیجہ_zeroblob64(pCtx, n>0 ? n : 0);
}
صحیح sqlite3_نتیجہ_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *p_برآمد;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  p_برآمد = pCtx->p_برآمد;
  أكد( sqlite3_mutex_held(p_برآمد->db->mutex) );
  اگر( n>(u64)p_برآمد->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] ){
    sqlite3_نتیجہ_error_toobig(pCtx);
    واپس SQLITE_TOOBIG;
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
  sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(pCtx->p_برآمد, (صحیح)n);
  واپس SQLITE_OK;
#ورنہ
  واپس sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(pCtx->p_برآمد, (صحیح)n);
#اگر_ختم
}
خالی sqlite3_نتیجہ_error_رمز(sqlite3_context *pCtx, صحیح err_رمز){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  pCtx->isError = err_رمز ? err_رمز : -1;
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( pCtx->pVdbe ) pCtx->pVdbe->rcApp = err_رمز;
#اگر_ختم
  اگر( pCtx->p_برآمد->flags & MEM_Null ){
    سیٹ_نتیجہ_Str_Or_Error(pCtx, sqlite3ErrStr(err_رمز), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
خالی sqlite3_نتیجہ_error_toobig(sqlite3_context *pCtx){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3_Vdbe_Mem_سیٹ_Str(pCtx->p_برآمد, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
خالی sqlite3_نتیجہ_error_nomem(sqlite3_context *pCtx){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  sqlite3_Vdbe_Mem_سیٹ_Null(pCtx->p_برآمد);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->p_برآمد->db);
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
خالی sqlite3_نتیجہ_str(sqlite3_context *pCtx, sqlite3_str *pStr, صحیح eOwn){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
  اگر( pStr==0 ) واپس;
#اگر_ختم
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  اگر( pStr->accError==0 ){
    اگر( pStr->nChar==0 ){
      سیٹ_نتیجہ_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }ورنہ{
      ثابت حرف *z_متن = sqlite3_str_قدر(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      أكد( isMalloced(pStr) );
      اگر( eOwn==SQLITE_COPY ){
        سیٹ_نتیجہ_Str_Or_Error(pCtx, z_متن, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }ورنہ{
        سیٹ_نتیجہ_Str_Or_Error(pCtx, z_متن, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }ورنہ اگر( pStr->accError==SQLITE_NOMEM ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }ورنہ{
    أكد( pStr->accError==SQLITE_TOOBIG );
    sqlite3_نتیجہ_error_toobig(pCtx);
  }
  اگر( eOwn==SQLITE_اختتام ){
    sqlite3_str_free(pStr);
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
خالی sqlite3_نتیجہ_Int_Real(sqlite3_context *pCtx){
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
  اگر( pCtx->p_برآمد->flags & MEM_Int ){
    pCtx->p_برآمد->flags &= ~MEM_Int;
    pCtx->p_برآمد->flags |= MEM_IntReal;
  }
}
#اگر_ختم


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ساکن صحیح doWalCallbacks(sqlite3 *db){
  صحیح rc = SQLITE_OK;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  صحیح i;
  برائے(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt ){
      صحیح n_اندراج;
      sqlite3BtreeEnter(pBt);
      n_اندراج = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      اگر( n_اندراج>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, n_اندراج);
      }
    }
  }
#ورنہ
  غیر_مستعمل_PARAMETER(db);
#اگر_ختم
  واپس rc;
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
ساکن صحیح sqlite3_قدم_2(Vdbe *p){
  sqlite3 *db;
  صحیح rc;

  أكد(p);
  db = p->db;
  اگر( p->eVdbeState!=VDBE_تسلسل_STATE ){
    restart_قدم:
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
        جاؤ انتہا_کا_قدم;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      اگر( db->n_Vdbe_فعال==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      أكد( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
      اگر( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3_Os_Current_وقت_Int64(db->pVfs, &p->آغاز_وقت);
      }ورنہ{
        أكد( p->آغاز_وقت==0 );
      }
#اگر_ختم

      db->n_Vdbe_فعال++;
      اگر( p->readOnly==0 ) db->nVdbeWrite++;
      اگر( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_تسلسل_STATE;
    }ورنہ

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
#اگر_تعریف_شدہ SQLITE_OMIT_AUTORESET
      اگر( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }ورنہ{
        واپس SQLITE_MISUSE_BKPT;
      }
#ورنہ
      sqlite3_reset((sqlite3_stmt*)p);
#اگر_ختم
      أكد( p->eVdbeState==VDBE_READY_STATE );
      جاؤ restart_قدم;
    }
  }

#اگر_تعریف_شدہ SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
  اگر( p->explain ){
    rc = sqlite3VdbeList(p);
  }ورنہ
#اگر_ختم /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  اگر( rc==SQLITE_صف ){
    أكد( p->rc==SQLITE_OK );
    أكد( db->mallocFailed==0 );
    db->err_رمز = SQLITE_صف;
    واپس SQLITE_صف;
  }ورنہ{
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#اگر_ختم
    p->p_نتیجہ_صف = 0;
    اگر( rc==SQLITE_مکمل && db->autoCommit ){
      أكد( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      اگر( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }ورنہ اگر( rc!=SQLITE_مکمل && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->err_رمز = rc;
  اگر( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    اگر( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
انتہا_کا_قدم:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  أكد( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_صف  || rc==SQLITE_مکمل   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  واپس (rc&db->err_نقاب);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
صحیح sqlite3_قدم(sqlite3_stmt *pStmt){
  صحیح rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  صحیح cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  اگر( vdbeSafetyNotNull(v) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  جب( (rc = sqlite3_قدم_2(v))==SQLITE_SCHEMA
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
      ثابت حرف *zErr = (ثابت حرف *)sqlite3_قدر_متن(db->pErr);
      sqlite3DbFree(db, v->z_Err_پیغام);
      اگر( !db->mallocFailed ){
        v->z_Err_پیغام = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } ورنہ {
        v->z_Err_پیغام = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      توڑو;
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
  واپس rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
خالی *sqlite3_user_ڈیٹا(sqlite3_context *p){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) واپس 0;
#اگر_ختم
  أكد( p && p->pFunc );
  واپس p->pFunc->p_User_ڈیٹا;
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
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) واپس 0;
#ورنہ
  أكد( p && p->p_برآمد );
#اگر_ختم
  واپس p->p_برآمد->db;
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
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) واپس 0;
#ورنہ
  أكد( p );
#اگر_ختم
  واپس sqlite3_قدر_nochange(p->p_برآمد);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
خالی sqlite3_Vdbe_قدر_List_Free(خالی *p_مقصد_Delete){
  sqlite3_free(p_مقصد_Delete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ساکن صحیح قدر_From_قدر_List(
  sqlite3_قدر *pVal,        /* Pointer to the ValueList object */
  sqlite3_قدر **pp_برآمد,      /* Store the next value from the list here */
  صحیح b_اگلا                   /* 1 for _next(). 0 for _first() */
){
  صحیح rc;
  قدر_List *pRhs;

  *pp_برآمد = 0;
  اگر( pVal==0 ) واپس SQLITE_MISUSE_BKPT;
  اگر( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_قدر_List_Free ){
    واپس SQLITE_ERROR;
  }ورنہ{
    أكد( (pVal->flags&(MEM_Type_نقاب|MEM_رکن|MEM_Subtype)) ==
                 (MEM_Null|MEM_رکن|MEM_Subtype) );
    أكد( pVal->eSubtype=='p' );
    أكد( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (قدر_List*)pVal->z;
  }
  اگر( b_اگلا ){
    rc = sqlite3_Btree_اگلا(pRhs->pCsr, 0);
  }ورنہ{
    صحیح dummy = 0;
    rc = sqlite3_Btree_پہلا(pRhs->pCsr, &dummy);
    أكد( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    اگر( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_مکمل;
  }
  اگر( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, حجم(sMem));
    sz = sqlite3_Btree_Payload_سائز(pRhs->pCsr);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_انحراف(pRhs->pCsr,sz,&sMem);
    اگر( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_قدر *p_برآمد = pRhs->p_برآمد;
      صحیح iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, p_برآمد);
      p_برآمد->enc = ENC(p_برآمد->db);
      اگر( (p_برآمد->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(p_برآمد) ){
        rc = SQLITE_NOMEM;
      }ورنہ{
        *pp_برآمد = p_برآمد;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  واپس rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
صحیح sqlite3_vtab_اندر_پہلا(sqlite3_قدر *pVal, sqlite3_قدر **pp_برآمد){
  واپس قدر_From_قدر_List(pVal, pp_برآمد, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
صحیح sqlite3_vtab_اندر_اگلا(sqlite3_قدر *pVal, sqlite3_قدر **pp_برآمد){
  واپس قدر_From_قدر_List(pVal, pp_برآمد, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3_Stmt_Current_وقت(sqlite3_context *p){
  صحیح rc;
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_STAT4
  sqlite3_int64 *pi_وقت = &p->pVdbe->i_Current_وقت;
  أكد( p->pVdbe!=0 );
#ورنہ
  sqlite3_int64 i_وقت = 0;
  sqlite3_int64 *pi_وقت = p->pVdbe!=0 ? &p->pVdbe->i_Current_وقت : &i_وقت;
#اگر_ختم
  اگر( *pi_وقت==0 ){
    rc = sqlite3_Os_Current_وقت_Int64(p->p_برآمد->db->pVfs, pi_وقت);
    اگر( rc ) *pi_وقت = 0;
  }
  واپس *pi_وقت;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ساکن SQLITE_NOINLINE خالی *createAggContext(sqlite3_context *p, صحیح nByte){
  Mem *pMem = p->pMem;
  أكد( (pMem->flags & MEM_Agg)==0 );
  اگر( nByte<=0 ){
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
    pMem->z = 0;
  }ورنہ{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    اگر( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  واپس (خالی*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
خالی *sqlite3_aggregate_context(sqlite3_context *p, صحیح nByte){
  أكد( p && p->pFunc && p->pFunc->xFinalize );
  أكد( sqlite3_mutex_held(p->p_برآمد->db->mutex) );
  testcase( nByte<0 );
  اگر( (p->pMem->flags & MEM_Agg)==0 ){
    واپس createAggContext(p, nByte);
  }ورنہ{
    واپس (خالی*)p->pMem->z;
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
خالی *sqlite3_get_auxdata(sqlite3_context *pCtx, صحیح iArg){
  Aux_ڈیٹا *p_Aux_ڈیٹا;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس 0;
#اگر_ختم
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
#اگر SQLITE_ENABLE_STAT4
  اگر( pCtx->pVdbe==0 ) واپس 0;
#ورنہ
  أكد( pCtx->pVdbe!=0 );
#اگر_ختم
  برائے(p_Aux_ڈیٹا=pCtx->pVdbe->p_Aux_ڈیٹا; p_Aux_ڈیٹا; p_Aux_ڈیٹا=p_Aux_ڈیٹا->p_اگلا_Aux){
    اگر(  p_Aux_ڈیٹا->iAuxArg==iArg && (p_Aux_ڈیٹا->iAuxOp==pCtx->iOp || iArg<0) ){
      واپس p_Aux_ڈیٹا->pAux;
    }
  }
  واپس 0;
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
خالی sqlite3_سیٹ_auxdata(
  sqlite3_context *pCtx,
  صحیح iArg,
  خالی *pAux,
  خالی (*xDelete)(خالی*)
){
  Aux_ڈیٹا *p_Aux_ڈیٹا;
  Vdbe *pVdbe;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCtx==0 ) واپس;
#اگر_ختم
  pVdbe= pCtx->pVdbe;
  أكد( sqlite3_mutex_held(pCtx->p_برآمد->db->mutex) );
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( pVdbe==0 ) جاؤ failed;
#ورنہ
  أكد( pVdbe!=0 );
#اگر_ختم

  برائے(p_Aux_ڈیٹا=pVdbe->p_Aux_ڈیٹا; p_Aux_ڈیٹا; p_Aux_ڈیٹا=p_Aux_ڈیٹا->p_اگلا_Aux){
    اگر( p_Aux_ڈیٹا->iAuxArg==iArg && (p_Aux_ڈیٹا->iAuxOp==pCtx->iOp || iArg<0) ){
      توڑو;
    }
  }
  اگر( p_Aux_ڈیٹا==0 ){
    p_Aux_ڈیٹا = sqlite3DbMallocZero(pVdbe->db, حجم(Aux_ڈیٹا));
    اگر( !p_Aux_ڈیٹا ) جاؤ failed;
    p_Aux_ڈیٹا->iAuxOp = pCtx->iOp;
    p_Aux_ڈیٹا->iAuxArg = iArg;
    p_Aux_ڈیٹا->p_اگلا_Aux = pVdbe->p_Aux_ڈیٹا;
    pVdbe->p_Aux_ڈیٹا = p_Aux_ڈیٹا;
    اگر( pCtx->isError==0 ) pCtx->isError = -1;
  }ورنہ اگر( p_Aux_ڈیٹا->xDeleteAux ){
    p_Aux_ڈیٹا->xDeleteAux(p_Aux_ڈیٹا->pAux);
  }

  p_Aux_ڈیٹا->pAux = pAux;
  p_Aux_ڈیٹا->xDeleteAux = xDelete;
  واپس;

failed:
  اگر( xDelete ){
    xDelete(pAux);
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
صحیح sqlite3_aggregate_گنتی(sqlite3_context *p){
  أكد( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  واپس p->pMem->n;
}
#اگر_ختم

/*
** Return the number of columns in the result set for the statement pStmt.
*/
صحیح sqlite3_عمود_گنتی(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  اگر( pVm==0 ) واپس 0;
  واپس pVm->n_Res_عمود;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
صحیح sqlite3_ڈیٹا_گنتی(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  اگر( pVm==0 || pVm->p_نتیجہ_صف==0 ) واپس 0;
  واپس pVm->n_Res_عمود;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ساکن ثابت Mem *عمود_Null_قدر(خالی){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ساکن ثابت Mem nullMem
#اگر defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#اگر_ختم
    = {
        /* .u          = */ {0},
        /* .z          = */ (حرف*)0,
        /* .n          = */ (صحیح)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (صحیح)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (حرف*)0,
        /* .xDel       = */ (خالی(*)(خالی*))0,
#اگر_تعریف_شدہ SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#اگر_ختم
      };
  واپس &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ساکن Mem *عمود_Mem(sqlite3_stmt *pStmt, صحیح i){
  Vdbe *pVm;
  Mem *p_برآمد;

  pVm = (Vdbe *)pStmt;
  اگر( pVm==0 ) واپس (Mem*)عمود_Null_قدر();
  أكد( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  اگر( pVm->p_نتیجہ_صف!=0 && i<pVm->n_Res_عمود && i>=0 ){
    p_برآمد = &pVm->p_نتیجہ_صف[i];
  }ورنہ{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    p_برآمد = (Mem*)عمود_Null_قدر();
  }
  واپس p_برآمد;
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
ساکن خالی عمود_Malloc_Failure(sqlite3_stmt *pStmt)
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
ثابت خالی *sqlite3_عمود_blob(sqlite3_stmt *pStmt, صحیح i){
  ثابت خالی *val;
  val = sqlite3_قدر_blob( عمود_Mem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
صحیح sqlite3_عمود_بائٹیں(sqlite3_stmt *pStmt, صحیح i){
  صحیح val = sqlite3_قدر_بائٹیں( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
صحیح sqlite3_عمود_bytes16(sqlite3_stmt *pStmt, صحیح i){
  صحیح val = sqlite3_قدر_bytes16( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
دوہرا sqlite3_عمود_double(sqlite3_stmt *pStmt, صحیح i){
  دوہرا val = sqlite3_قدر_double( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
صحیح sqlite3_عمود_int(sqlite3_stmt *pStmt, صحیح i){
  صحیح val = sqlite3_قدر_int( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
sqlite_int64 sqlite3_عمود_int64(sqlite3_stmt *pStmt, صحیح i){
  sqlite_int64 val = sqlite3_قدر_int64( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
ثابت غیرعلامتی حرف *sqlite3_عمود_متن(sqlite3_stmt *pStmt, صحیح i){
  ثابت غیرعلامتی حرف *val = sqlite3_قدر_متن( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
sqlite3_قدر *sqlite3_عمود_قدر(sqlite3_stmt *pStmt, صحیح i){
  Mem *p_برآمد = عمود_Mem(pStmt, i);
  اگر( p_برآمد->flags&MEM_Static ){
    p_برآمد->flags &= ~MEM_Static;
    p_برآمد->flags |= MEM_Ephem;
  }
  عمود_Malloc_Failure(pStmt);
  واپس (sqlite3_قدر *)p_برآمد;
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_عمود_text16(sqlite3_stmt *pStmt, صحیح i){
  ثابت خالی *val = sqlite3_قدر_text16( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس val;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */
صحیح sqlite3_عمود_type(sqlite3_stmt *pStmt, صحیح i){
  صحیح iType = sqlite3_قدر_type( عمود_Mem(pStmt,i) );
  عمود_Malloc_Failure(pStmt);
  واپس iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ساکن ثابت حرف * ثابت az_Explain_ستون_Names8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ساکن ثابت u16 az_Explain_ستون_Names16data[] = {
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
ساکن ثابت u8 i_Explain_ستون_Names16[] = {
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
ساکن ثابت خالی *عمود_Name(
  sqlite3_stmt *pStmt,     /* The statement */
  صحیح N,                   /* Which column to get the name for */
  صحیح useUtf16,            /* True to return the name as UTF16 */
  صحیح useType              /* What type of name */
){
  ثابت خالی *ret;
  Vdbe *p;
  صحیح n;
  sqlite3 *db;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pStmt==0 ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  اگر( N<0 ) واپس 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  أكد( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  اگر( p->explain ){
    اگر( useType>0 ) جاؤ عمود_Name_انتہا;
    n = p->explain==1 ? 8 : 4;
    اگر( N>=n ) جاؤ عمود_Name_انتہا;
    اگر( useUtf16 ){
      صحیح i = i_Explain_ستون_Names16[N + 8*p->explain - 8];
      ret = (خالی*)&az_Explain_ستون_Names16data[i];
    }ورنہ{
      ret = (خالی*)az_Explain_ستون_Names8[N + 8*p->explain - 8];
    }
    جاؤ عمود_Name_انتہا;
  }
  n = p->n_Res_عمود;
  اگر( N<n ){
    u8 پیشگی_malloc_Failed = db->mallocFailed;
    N += useType*n;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
    اگر( useUtf16 ){
      ret = sqlite3_قدر_text16((sqlite3_قدر*)&p->a_ستون_Name[N]);
    }ورنہ
#اگر_ختم
    {
      ret = sqlite3_قدر_متن((sqlite3_قدر*)&p->a_ستون_Name[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    أكد( db->mallocFailed==0 || db->mallocFailed==1 );
    اگر( db->mallocFailed > پیشگی_malloc_Failed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
عمود_Name_انتہا:
  sqlite3_mutex_leave(db->mutex);
  واپس ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ثابت حرف *sqlite3_عمود_name(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 0, COLNAME_NAME);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_عمود_name16(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 1, COLNAME_NAME);
}
#اگر_ختم

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#اگر defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_عمود_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_عمود_METADATA"
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ثابت حرف *sqlite3_عمود_decltype(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 0, COLNAME_DECLTYPE);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_عمود_decltype16(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 1, COLNAME_DECLTYPE);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */
#اگر_ختم /* SQLITE_OMIT_DECLTYPE */

#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت حرف *sqlite3_عمود_database_name(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 0, COLNAME_DATABASE);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_عمود_database_name16(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 1, COLNAME_DATABASE);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت حرف *sqlite3_عمود_جدول_name(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 0, COLNAME_جدول);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_عمود_جدول_name16(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 1, COLNAME_جدول);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ثابت حرف *sqlite3_عمود_origin_name(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 0, COLNAME_عمود);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ثابت خالی *sqlite3_عمود_origin_name16(sqlite3_stmt *pStmt, صحیح N){
  واپس عمود_Name(pStmt, N, 1, COLNAME_عمود);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */
#اگر_ختم /* SQLITE_ENABLE_COLUMN_METADATA */


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
ساکن صحیح vdbeUnbind(Vdbe *p, غیرعلامتی صحیح i){
  Mem *pVar;
  اگر( vdbeSafetyNotNull(p) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  اگر( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    واپس SQLITE_MISUSE_BKPT;
  }
  اگر( i>=(غیرعلامتی صحیح)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    واپس SQLITE_RANGE;
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
  p->db->err_رمز = SQLITE_OK;

  واپس SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ساکن صحیح bind_متن_2(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  صحیح i,                 /* Index of the parameter to bind */
  ثابت خالی *z_ڈیٹا,     /* Pointer to the data to be bound */
  i64 n_ڈیٹا,             /* Number of bytes of data to be bound */
  خالی (*xDel)(خالی*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  صحیح rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    اگر( z_ڈیٹا!=0 ){
      pVar = &p->aVar[i-1];
      اگر( encoding==SQLITE_UTF8 ){
        rc = sqlite3_Vdbe_Mem_سیٹ_متن(pVar, z_ڈیٹا, n_ڈیٹا, xDel);
      }ورنہ اگر( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        أكد( z_ڈیٹا==0
             || n_ڈیٹا<0 
             || n_ڈیٹا>pVar->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]
             || ((u8*)z_ڈیٹا)[n_ڈیٹا]==0
        );
        rc = sqlite3_Vdbe_Mem_سیٹ_متن(pVar, z_ڈیٹا, n_ڈیٹا, xDel);
        pVar->flags |= MEM_رکن;
      }ورنہ{
        rc = sqlite3_Vdbe_Mem_سیٹ_Str(pVar, z_ڈیٹا, n_ڈیٹا, encoding, xDel);
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
  }ورنہ اگر( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((خالی*)z_ڈیٹا);
  }
  واپس rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
صحیح sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت خالی *z_ڈیٹا,
  صحیح n_ڈیٹا,
  خالی (*xDel)(خالی*)
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( n_ڈیٹا<0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  واپس bind_متن_2(pStmt, i, z_ڈیٹا, n_ڈیٹا, xDel, 0);
}
صحیح sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت خالی *z_ڈیٹا,
  sqlite3_uint64 n_ڈیٹا,
  خالی (*xDel)(خالی*)
){
  أكد( xDel!=SQLITE_DYNAMIC );
  واپس bind_متن_2(pStmt, i, z_ڈیٹا, n_ڈیٹا, xDel, 0);
}
صحیح sqlite3_bind_double(sqlite3_stmt *pStmt, صحیح i, دوہرا r_قدر){
  صحیح rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_سیٹ_Double(&p->aVar[i-1], r_قدر);
    sqlite3_mutex_leave(p->db->mutex);
  }
  واپس rc;
}
صحیح sqlite3_bind_int(sqlite3_stmt *p, صحیح i, صحیح i_قدر){
  واپس sqlite3_bind_int64(p, i, (i64)i_قدر);
}
صحیح sqlite3_bind_int64(sqlite3_stmt *pStmt, صحیح i, sqlite_int64 i_قدر){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  اگر( vdbeSafetyNotNull(p) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  اگر( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    واپس SQLITE_MISUSE_BKPT;
  }
  اگر( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    واپس SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  اگر( p->expmask!=0 ){
    u32 expire_نقاب = i>=32 ? 0x80000000 : (u32)1 << i;
    أكد( (p->expmask & p->smimask)==p->smimask );
    اگر( (p->expmask & expire_نقاب)!=0 ){
      اگر( (p->smimask & expire_نقاب)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        صحیح sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        صحیح sm2 = i_قدر>=0 && i_قدر<=1;
        اگر( sm1!=sm2 ){
          p->expired = 1;
        }
      }ورنہ اگر( (pVar->flags & MEM_Int)==0 || i_قدر!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3_Vdbe_Mem_سیٹ_Int64(pVar, i_قدر);
  sqlite3_mutex_leave(p->db->mutex);
  واپس SQLITE_OK;
}
صحیح sqlite3_bind_null(sqlite3_stmt *pStmt, صحیح i){
  صحیح rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  واپس rc;
}
صحیح sqlite3_bind_اشارہ_گر(
  sqlite3_stmt *pStmt,
  صحیح i,
  خالی *pPtr,
  ثابت حرف *zPTtype,
  خالی (*xDestructor)(خالی*)
){
  صحیح rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_سیٹ_اشارہ_گر(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }ورنہ اگر( xDestructor ){
    xDestructor(pPtr);
  }
  واپس rc;
}
صحیح sqlite3_bind_متن(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت حرف *z_ڈیٹا,
  صحیح n_ڈیٹا,
  خالی (*xDel)(خالی*)
){
  واپس bind_متن_2(pStmt, i, z_ڈیٹا, n_ڈیٹا, xDel, SQLITE_UTF8);
}
صحیح sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت حرف *z_ڈیٹا,
  sqlite3_uint64 n_ڈیٹا,
  خالی (*xDel)(خالی*),
  غیرعلامتی حرف enc
){
  أكد( xDel!=SQLITE_DYNAMIC );
  اگر( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    اگر( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_ڈیٹا &= ~(u64)1;
  }
  واپس bind_متن_2(pStmt, i, z_ڈیٹا, n_ڈیٹا, xDel, enc);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
صحیح sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  صحیح i,
  ثابت خالی *z_ڈیٹا,
  صحیح n,
  خالی (*xDel)(خالی*)
){
  واپس bind_متن_2(pStmt, i, z_ڈیٹا, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */
صحیح sqlite3_bind_قدر(sqlite3_stmt *pStmt, صحیح i, ثابت sqlite3_قدر *p_قدر){
  صحیح rc;
  انتخاب( sqlite3_قدر_type((sqlite3_قدر*)p_قدر) ){
    حالت SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_قدر->u.i);
      توڑو;
    }
    حالت SQLITE_FLOAT: {
      أكد( p_قدر->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_قدر->flags & MEM_Real) ? p_قدر->u.r : (دوہرا)p_قدر->u.i
      );
      توڑو;
    }
    حالت SQLITE_BLOB: {
      اگر( p_قدر->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_قدر->u.nZero);
      }ورنہ{
        rc = sqlite3_bind_blob(pStmt, i, p_قدر->z, p_قدر->n,SQLITE_TRANSIENT);
      }
      توڑو;
    }
    حالت SQLITE_متن: {
      rc = bind_متن_2(pStmt,i,  p_قدر->z, p_قدر->n, SQLITE_TRANSIENT,
                              p_قدر->enc);
      توڑو;
    }
    طےشدہ: {
      rc = sqlite3_bind_null(pStmt, i);
      توڑو;
    }
  }
  واپس rc;
}
صحیح sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, صحیح i, صحیح n){
  صحیح rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  اگر( rc==SQLITE_OK ){
    أكد( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
    sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(&p->aVar[i-1], n);
#ورنہ
    rc = sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(&p->aVar[i-1], n);
#اگر_ختم
    sqlite3_mutex_leave(p->db->mutex);
  }
  واپس rc;
}
صحیح sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, صحیح i, sqlite3_uint64 n){
  صحیح rc;
  Vdbe *p = (Vdbe *)pStmt;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( p==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(p->db->mutex);
  اگر( n>(u64)p->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] ){
    rc = SQLITE_TOOBIG;
  }ورنہ{
    أكد( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  واپس rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
صحیح sqlite3_bind_parameter_گنتی(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  واپس p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ثابت حرف *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, صحیح i){
  Vdbe *p = (Vdbe*)pStmt;
  اگر( p==0 ) واپس 0;
  واپس sqlite3_VList_عدد_مقصد_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
صحیح sqlite3_Vdbe_Parameter_اشاریہ(Vdbe *p, ثابت حرف *zName, صحیح nName){
  اگر( p==0 || zName==0 ) واپس 0;
  واپس sqlite3_VList_Name_مقصد_عدد(p->pVList, zName, nName);
}
صحیح sqlite3_bind_parameter_اشاریہ(sqlite3_stmt *pStmt, ثابت حرف *zName){
  واپس sqlite3_Vdbe_Parameter_اشاریہ((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
صحیح sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_مقصد_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_مقصد = (Vdbe*)p_مقصد_Stmt;
  صحیح i;
  أكد( p_مقصد->db==pFrom->db );
  أكد( p_مقصد->nVar==pFrom->nVar );
  sqlite3_mutex_enter(p_مقصد->db->mutex);
  برائے(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&p_مقصد->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(p_مقصد->db->mutex);
  واپس SQLITE_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
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
صحیح sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_مقصد_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_مقصد = (Vdbe*)p_مقصد_Stmt;
  اگر( pFrom->nVar!=p_مقصد->nVar ){
    واپس SQLITE_ERROR;
  }
  أكد( (p_مقصد->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p_مقصد->expmask==0 );
  اگر( p_مقصد->expmask ){
    p_مقصد->expired = 1;
  }
  أكد( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  اگر( pFrom->expmask ){
    pFrom->expired = 1;
  }
  واپس sqlite3TransferBindings(pFromStmt, p_مقصد_Stmt);
}
#اگر_ختم

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  واپس pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
صحیح sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  واپس pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
صحیح sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  واپس pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
صحیح sqlite3_stmt_explain(sqlite3_stmt *pStmt, صحیح e_منوال){
  Vdbe *v = (Vdbe*)pStmt;
  صحیح rc;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pStmt==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(v->db->mutex);
  اگر( ((صحیح)v->explain)==e_منوال ){
    rc = SQLITE_OK;
  }ورنہ اگر( e_منوال<0 || e_منوال>2 ){
    rc = SQLITE_ERROR;
  }ورنہ اگر( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }ورنہ اگر( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }ورنہ اگر( v->nMem>=10 && (e_منوال!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = e_منوال;
    rc = SQLITE_OK;
  }ورنہ{
    صحیح saved_منوال = v->explain;
    v->explain = e_منوال;
    rc = sqlite3Reprepare(v);
    اگر( rc ){
      v->explain = saved_منوال;
    }ورنہ{
      v->haveEqpOps = e_منوال==2;
    }
  }
  اگر( v->explain ){
    v->n_Res_عمود = 12 - 4*v->explain;
  }ورنہ{
    v->n_Res_عمود = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  واپس rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
صحیح sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  واپس v!=0 && v->eVdbeState==VDBE_تسلسل_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_اگلا_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_اگلا;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(pDb) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(pDb->mutex);
  اگر( pStmt==0 ){
    p_اگلا = (sqlite3_stmt*)pDb->pVdbe;
  }ورنہ{
    p_اگلا = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  واپس p_اگلا;
}

/*
** Return the value of a status counter for a prepared statement
*/
صحیح sqlite3_stmt_حالت(sqlite3_stmt *pStmt, صحیح op, صحیح resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_سائز(pVdbe->a_شمار)))
  ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  اگر( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pn_بائٹیں_Freed = (صحیح*)&v;
    أكد( db->lookaside.p_انتہا==db->lookaside.p_True_انتہا );
    db->lookaside.p_انتہا = db->lookaside.p_آغاز;
    sqlite3VdbeDelete(pVdbe);
    db->pn_بائٹیں_Freed = 0;
    db->lookaside.p_انتہا = db->lookaside.p_True_انتہا;
    sqlite3_mutex_leave(db->mutex);
  }ورنہ{
    v = pVdbe->a_شمار[op];
    اگر( resetFlag ) pVdbe->a_شمار[op] = 0;
  }
  واپس (صحیح)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ثابت حرف *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  واپس p ? p->zSql : 0;
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
#اگر_تعریف_شدہ SQLITE_OMIT_TRACE
  واپس 0;
#ورنہ
  حرف *z = 0;
  ثابت حرف *zSql = sqlite3_sql(pStmt);
  اگر( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  واپس z;
#اگر_ختم
}

#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ثابت حرف *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  اگر( p==0 ) واپس 0;
  اگر( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  واپس p->zNormSql;
}
#اگر_ختم /* SQLITE_ENABLE_NORMALIZE */

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ساکن Unpacked_ریکارڈ *vdbe_Unpack_ریکارڈ(
  کلید_Info *p_کلید_Info,
  صحیح n_کلید,
  ثابت خالی *p_کلید
){
  Unpacked_ریکارڈ *pRet;           /* Return value */

  pRet = sqlite3_Vdbe_Alloc_Unpacked_ریکارڈ(p_کلید_Info);
  اگر( pRet ){
    memset(pRet->aMem, 0, حجم(Mem)*(p_کلید_Info->n_کلید_میدان+1));
    sqlite3_Vdbe_ریکارڈ_Unpack(n_کلید, p_کلید, pRet);
  }
  واپس pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
صحیح sqlite3_preupdate_old(sqlite3 *db, صحیح iIdx, sqlite3_قدر **pp_قدر){
  PreUpdate *p;
  Mem *pMem;
  صحیح rc = SQLITE_OK;
  صحیح iStore = 0;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( db==0 || pp_قدر==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  اگر( !p || p->op==SQLITE_داخل_کرو ){
    rc = SQLITE_MISUSE_BKPT;
    جاؤ preupdate_old_برآمد;
  }
  اگر( p->pPk ){
    iStore = sqlite3_جدول_عمود_مقصد_اشاریہ(p->pPk, iIdx);
  }ورنہ اگر( iIdx >= p->pTab->n_ستون ){
    rc = SQLITE_MISUSE_BKPT;
    جاؤ preupdate_old_برآمد;
  }ورنہ{
    iStore = sqlite3_جدول_عمود_مقصد_Storage(p->pTab, iIdx);
  }
  اگر( iStore>=p->pCsr->n_میدان || iStore<0 ){
    rc = SQLITE_RANGE;
    جاؤ preupdate_old_برآمد;
  }

  اگر( iIdx==p->pTab->iPKey ){
    *pp_قدر = pMem = &p->oldipk;
    sqlite3_Vdbe_Mem_سیٹ_Int64(pMem, p->iKey1);
  }ورنہ{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    اگر( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      أكد( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_سائز(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      اگر( !aRec ) جاؤ preupdate_old_برآمد;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      اگر( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbe_Unpack_ریکارڈ(p->pKeyinfo, nRec, aRec);
        اگر( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      اگر( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        جاؤ preupdate_old_برآمد;
      }
      p->a_ریکارڈ = aRec;
    }

    pMem = *pp_قدر = &p->pUnpacked->aMem[iStore];
    اگر( iStore>=p->pUnpacked->n_میدان ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      عمود *p_ستون = &p->pTab->a_ستون[iIdx];
      اگر( p_ستون->iDflt>0 ){
        اگر( p->apDflt==0 ){
          صحیح nByte;
          أكد( حجم(sqlite3_قدر*)*UMXV(p->pTab->n_ستون) < 0x7fffffff );
          nByte = حجم(sqlite3_قدر*)*p->pTab->n_ستون;
          p->apDflt = (sqlite3_قدر**)sqlite3DbMallocZero(db, nByte);
          اگر( p->apDflt==0 ) جاؤ preupdate_old_برآمد;
        }
        اگر( p->apDflt[iIdx]==0 ){
          sqlite3_قدر *pVal = 0;
          اظہاریہ *pDflt;
          أكد( p->pTab!=0 && Is_Ordinary_جدول(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[p_ستون->iDflt-1].p_اظہاریہ;
          rc = sqlite3_قدر_From_اظہاریہ(db, pDflt, ENC(db), p_ستون->affinity, &pVal);
          اگر( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_قدر = p->apDflt[iIdx];
      }ورنہ{
        *pp_قدر = (sqlite3_قدر *)عمود_Null_قدر();
      }
    }ورنہ اگر( p->pTab->a_ستون[iIdx].affinity==SQLITE_AFF_REAL ){
      اگر( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_برآمد:
  sqlite3Error(db, rc);
  واپس sqlite3ApiExit(db, rc);
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
صحیح sqlite3_preupdate_گنتی(sqlite3 *db){
  PreUpdate *p;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ورنہ
  p = db->pPreUpdate;
#اگر_ختم
  واپس (p ? p->pKeyinfo->n_کلید_میدان : 0);
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
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
صحیح sqlite3_preupdate_گہرائی(sqlite3 *db){
  PreUpdate *p;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ورنہ
  p = db->pPreUpdate;
#اگر_ختم
  واپس (p ? p->v->n_چوکھٹا : 0);
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
صحیح sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ورنہ
  p = db->pPreUpdate;
#اگر_ختم
  واپس (p ? p->iBlobWrite : -1);
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
صحیح sqlite3_preupdate_new(sqlite3 *db, صحیح iIdx, sqlite3_قدر **pp_قدر){
  PreUpdate *p;
  صحیح rc = SQLITE_OK;
  Mem *pMem;
  صحیح iStore = 0;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( db==0 || pp_قدر==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  p = db->pPreUpdate;
  اگر( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    جاؤ preupdate_new_برآمد;
  }
  اگر( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_جدول_عمود_مقصد_اشاریہ(p->pPk, iIdx);
  }ورنہ اگر( iIdx >= p->pTab->n_ستون ){
    واپس SQLITE_MISUSE_BKPT;
  }ورنہ{
    iStore = sqlite3_جدول_عمود_مقصد_Storage(p->pTab, iIdx);
  }

  اگر( iStore>=p->pCsr->n_میدان || iStore<0 ){
    rc = SQLITE_RANGE;
    جاؤ preupdate_new_برآمد;
  }

  اگر( p->op==SQLITE_داخل_کرو ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    Unpacked_ریکارڈ *pUnpack = p->pNewUnpacked;
    اگر( !pUnpack ){
      Mem *p_ڈیٹا = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_ڈیٹا);
      اگر( rc!=SQLITE_OK ) جاؤ preupdate_new_برآمد;
      pUnpack = vdbe_Unpack_ریکارڈ(p->pKeyinfo, p_ڈیٹا->n, p_ڈیٹا->z);
      اگر( !pUnpack ){
        rc = SQLITE_NOMEM;
        جاؤ preupdate_new_برآمد;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    اگر( iIdx==p->pTab->iPKey ){
      sqlite3_Vdbe_Mem_سیٹ_Int64(pMem, p->iKey2);
    }ورنہ اگر( iStore>=pUnpack->n_میدان ){
      pMem = (sqlite3_قدر *)عمود_Null_قدر();
    }ورنہ اگر( (p->pTab->a_ستون)
           && (p->pTab->a_ستون[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }ورنہ{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    أكد( p->op==SQLITE_UPDATE );
    اگر( !p->aNew ){
      أكد( حجم(Mem)*UMXV(p->pCsr->n_میدان) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, حجم(Mem)*p->pCsr->n_میدان);
      اگر( !p->aNew ){
        rc = SQLITE_NOMEM;
        جاؤ preupdate_new_برآمد;
      }
    }
    أكد( iStore>=0 && iStore<p->pCsr->n_میدان );
    pMem = &p->aNew[iStore];
    اگر( pMem->flags==0 ){
      اگر( iIdx==p->pTab->iPKey ){
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem, p->iKey2);
      }ورنہ{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        اگر( rc!=SQLITE_OK ) جاؤ preupdate_new_برآمد;
      }
    }
  }
  *pp_قدر = pMem;

 preupdate_new_برآمد:
  sqlite3Error(db, rc);
  واپس sqlite3ApiExit(db, rc);
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
صحیح sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  صحیح i_جائزہ,                      /* Index of loop to report on */
  صحیح i_جائزہ_حالت_Op,              /* Which metric to return */
  صحیح flags,
  خالی *p_برآمد                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  صحیح nOp;
  جائزہ_حالت *p_جائزہ = 0;
  صحیح idx;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( p==0 || p_برآمد==0
      || i_جائزہ_حالت_Op<SQLITE_SCANSTAT_NLOOP
      || i_جائزہ_حالت_Op>SQLITE_SCANSTAT_NCYCLE ){
    واپس 1;
  }
#اگر_ختم
  aOp = p->aOp;
  nOp = p->nOp;
  اگر( p->p_چوکھٹا ){
    Vdbe_چوکھٹا *p_چوکھٹا;
    برائے(p_چوکھٹا=p->p_چوکھٹا; p_چوکھٹا->p_والد; p_چوکھٹا=p_چوکھٹا->p_والد);
    aOp = p_چوکھٹا->aOp;
    nOp = p_چوکھٹا->nOp;
  }

  اگر( i_جائزہ<0 ){
    صحیح ii;
    اگر( i_جائزہ_حالت_Op==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      برائے(ii=0; ii<nOp; ii++){
        res += aOp[ii].n_چکر;
      }
      *(i64*)p_برآمد = res;
      واپس 0;
    }
    واپس 1;
  }
  اگر( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = i_جائزہ;
  }ورنہ{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    برائے(idx=0; idx<p->n_جائزہ; idx++){
      p_جائزہ = &p->a_جائزہ[idx];
      اگر( p_جائزہ->zName ){
        i_جائزہ--;
        اگر( i_جائزہ<0 ) توڑو;
      }
    }
  }
  اگر( idx>=p->n_جائزہ ) واپس 1;
  أكد( p_جائزہ==0 || p_جائزہ==&p->a_جائزہ[idx] );
  p_جائزہ = &p->a_جائزہ[idx];

  انتخاب( i_جائزہ_حالت_Op ){
    حالت SQLITE_SCANSTAT_NLOOP: {
      اگر( p_جائزہ->addrLoop>0 ){
        *(sqlite3_int64*)p_برآمد = aOp[p_جائزہ->addrLoop].nExec;
      }ورنہ{
        *(sqlite3_int64*)p_برآمد = -1;
      }
      توڑو;
    }
    حالت SQLITE_SCANSTAT_NVISIT: {
      اگر( p_جائزہ->addr_دورہ>0 ){
        *(sqlite3_int64*)p_برآمد = aOp[p_جائزہ->addr_دورہ].nExec;
      }ورنہ{
        *(sqlite3_int64*)p_برآمد = -1;
      }
      توڑو;
    }
    حالت SQLITE_SCANSTAT_EST: {
      دوہرا r = 1.0;
      LogEst x = p_جائزہ->nEst;
      جب( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(دوہرا*)p_برآمد = r*sqlite3_Log_Est_مقصد_Int(x);
      توڑو;
    }
    حالت SQLITE_SCANSTAT_NAME: {
      *(ثابت حرف**)p_برآمد = p_جائزہ->zName;
      توڑو;
    }
    حالت SQLITE_SCANSTAT_EXPLAIN: {
      اگر( p_جائزہ->addrExplain ){
        *(ثابت حرف**)p_برآمد = aOp[ p_جائزہ->addrExplain ].p4.z;
      }ورنہ{
        *(ثابت حرف**)p_برآمد = 0;
      }
      توڑو;
    }
    حالت SQLITE_SCANSTAT_SELECTID: {
      اگر( p_جائزہ->addrExplain ){
        *(صحیح*)p_برآمد = aOp[ p_جائزہ->addrExplain ].p1;
      }ورنہ{
        *(صحیح*)p_برآمد = -1;
      }
      توڑو;
    }
    حالت SQLITE_SCANSTAT_PARENTID: {
      اگر( p_جائزہ->addrExplain ){
        *(صحیح*)p_برآمد = aOp[ p_جائزہ->addrExplain ].p2;
      }ورنہ{
        *(صحیح*)p_برآمد = -1;
      }
      توڑو;
    }
    حالت SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      اگر( p_جائزہ->aAddrRange[0]==0 ){
        res = -1;
      }ورنہ{
        صحیح ii;
        برائے(ii=0; ii<Array_سائز(p_جائزہ->aAddrRange); ii+=2){
          صحیح iIns = p_جائزہ->aAddrRange[ii];
          صحیح i_انتہا = p_جائزہ->aAddrRange[ii+1];
          اگر( iIns==0 ) توڑو;
          اگر( iIns>0 ){
            جب( iIns<=i_انتہا ){
              res += aOp[iIns].n_چکر;
              iIns++;
            }
          }ورنہ{
            صحیح iOp;
            برائے(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              اگر( pOp->p1!=i_انتہا ) جاری;
              اگر( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                جاری;
              }
              res += aOp[iOp].n_چکر;
            }
          }
        }
      }
      *(i64*)p_برآمد = res;
      توڑو;
    }
    طےشدہ: {
      واپس 1;
    }
  }
  واپس 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
صحیح sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  صحیح i_جائزہ,                      /* Index of loop to report on */
  صحیح i_جائزہ_حالت_Op,              /* Which metric to return */
  خالی *p_برآمد                      /* OUT: Write the answer here */
){
  واپس sqlite3_stmt_scanstatus_v2(pStmt, i_جائزہ, i_جائزہ_حالت_Op, 0, p_برآمد);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
خالی sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  صحیح ii;
  برائے(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->n_چکر = 0;
  }
}
#اگر_ختم /* SQLITE_ENABLE_STMT_SCANSTATUS */
