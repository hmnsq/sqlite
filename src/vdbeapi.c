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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "opcodes.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
ਪੂਰਨ sqlite3_expired(sqlite3_stmt *pStmt){
  ਪੂਰਨ iRet = 1;
  ਜੇ( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  ਵਾਪਸ iRet;
}
#ਜੇ_ਅੰਤ

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSafety(Vdbe *p){
  ਜੇ( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}
ਸਥਿਰ ਪੂਰਨ vdbeSafetyNotNull(Vdbe *p){
  ਜੇ( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ vdbeSafety(p);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਸ਼ੁਰੂ_ਸਮਾਂ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->init.busy==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->zSql!=0 );
  sqlite3_Os_Current_ਸਮਾਂ_Int64(db->pVfs, &iNow);
  iElapse = (iNow - p->ਸ਼ੁਰੂ_ਸਮਾਂ)*1000000;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  ਜੇ( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (ਖਾਲੀ*)&iElapse);
  }
  p->ਸ਼ੁਰੂ_ਸਮਾਂ = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   ਜੇ( ((P)->ਸ਼ੁਰੂ_ਸਮਾਂ)>0 ){ invokeProfileCallback(DB,P); }
#ਨਹੀਂਤਾਂ
# define checkProfileCallback(DB,P)  /*no-op*/
#ਜੇ_ਅੰਤ

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
ਪੂਰਨ sqlite3_finalize(sqlite3_stmt *pStmt){
  ਪੂਰਨ rc;
  ਜੇ( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    ਜੇ( vdbeSafety(v) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    ਪ੍ਰਤਿਜ੍ਞਾ( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  ਵਾਪਸ rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
ਪੂਰਨ sqlite3_reset(sqlite3_stmt *pStmt){
  ਪੂਰਨ rc;
  ਜੇ( pStmt==0 ){
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    ਪ੍ਰਤਿਜ੍ਞਾ( (rc & (db->err_ਨਕਾਬ))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  ਵਾਪਸ rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
ਪੂਰਨ sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 ਨਕਾਬ_Small_Int = 0;  /* Bitmask of bindings with value 0 or 1 */
#ਜੇ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pStmt==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_THREADSAFE
  mutex = p->db->mutex;
#ਜੇ_ਅੰਤ

  sqlite3_mutex_enter(mutex);
  ਲਈ(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    ਜੇ( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      ਨਕਾਬ_Small_Int |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ਜੇ( (p->expmask&~p->smimask)!=0 || (p->smimask&ਨਕਾਬ_Small_Int)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  ਵਾਪਸ rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ਅਚਲ ਖਾਲੀ *sqlite3_ਮੁੱਲ_blob(sqlite3_ਮੁੱਲ *pVal){
  Mem *p = (Mem*)pVal;
  ਜੇ( p->flags & (MEM_Blob|MEM_Str) ){
    ਜੇ( ExpandBlob(p)!=SQLITE_OK ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->flags==MEM_Null && p->z==0 );
      ਵਾਪਸ 0;
    }
    p->flags |= MEM_Blob;
    ਵਾਪਸ p->n ? p->z : 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ sqlite3_ਮੁੱਲ_ਪਾਠ(pVal);
  }
}
ਪੂਰਨ sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ_2(pVal, SQLITE_UTF8);
}
ਪੂਰਨ sqlite3_ਮੁੱਲ_bytes16(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ_2(pVal, SQLITE_UTF16NATIVE);
}
ਦੁੱਗਣਾ sqlite3_ਮੁੱਲ_double(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ sqlite3_Vdbe_Real_ਮੁੱਲ((Mem*)pVal);
}
ਪੂਰਨ sqlite3_ਮੁੱਲ_int(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ (ਪੂਰਨ)sqlite3_Vdbe_Int_ਮੁੱਲ((Mem*)pVal);
}
sqlite_int64 sqlite3_ਮੁੱਲ_int64(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ sqlite3_Vdbe_Int_ਮੁੱਲ((Mem*)pVal);
}
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sqlite3_ਮੁੱਲ_subtype(sqlite3_ਮੁੱਲ *pVal){
  Mem *pMem = (Mem*)pVal;
  ਵਾਪਸ ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
ਖਾਲੀ *sqlite3_ਮੁੱਲ_ਸੂਚਕ(sqlite3_ਮੁੱਲ *pVal, ਅਚਲ ਅੱਖਰ *zPType){
  Mem *p = (Mem*)pVal;
  ਜੇ( (p->flags&(MEM_Type_ਨਕਾਬ|MEM_ਪਦ|MEM_Subtype)) ==
                 (MEM_Null|MEM_ਪਦ|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    ਵਾਪਸ (ਖਾਲੀ*)p->z;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}
ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *sqlite3_ਮੁੱਲ_ਪਾਠ(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, SQLITE_UTF8);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਮੁੱਲ_text16(sqlite3_ਮੁੱਲ* pVal){
  ਵਾਪਸ sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, SQLITE_UTF16NATIVE);
}
ਅਚਲ ਖਾਲੀ *sqlite3_ਮੁੱਲ_text16be(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, SQLITE_UTF16BE);
}
ਅਚਲ ਖਾਲੀ *sqlite3_ਮੁੱਲ_text16le(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, SQLITE_UTF16LE);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
ਪੂਰਨ sqlite3_ਮੁੱਲ_type(sqlite3_ਮੁੱਲ* pVal){
  ਸਥਿਰ ਅਚਲ u8 aType[] = {
     SQLITE_BLOB,     /* 0x00 (not possible) */
     SQLITE_NULL,     /* 0x01 NULL */
     SQLITE_ਪਾਠ,     /* 0x02 TEXT */
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
     SQLITE_ਪਾਠ,     /* 0x12 (not possible) */
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
     SQLITE_ਪਾਠ,     /* 0x32 (not possible) */
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  {
    ਪੂਰਨ eType = SQLITE_BLOB;
    ਜੇ( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }ਨਹੀਂਤਾਂ ਜੇ( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }ਨਹੀਂਤਾਂ ਜੇ( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }ਨਹੀਂਤਾਂ ਜੇ( pVal->flags & MEM_Str ){
      eType = SQLITE_ਪਾਠ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( eType == aType[pVal->flags&MEM_Aff_ਨਕਾਬ] );
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ aType[pVal->flags&MEM_Aff_ਨਕਾਬ];
}
ਪੂਰਨ sqlite3_ਮੁੱਲ_encoding(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
ਪੂਰਨ sqlite3_ਮੁੱਲ_nochange(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
ਪੂਰਨ sqlite3_ਮੁੱਲ_frombind(sqlite3_ਮੁੱਲ *pVal){
  ਵਾਪਸ (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_ਮੁੱਲ *sqlite3_ਮੁੱਲ_dup(ਅਚਲ sqlite3_ਮੁੱਲ *pOrig){
  sqlite3_ਮੁੱਲ *pNew;
  ਜੇ( pOrig==0 ) ਵਾਪਸ 0;
  pNew = sqlite3_malloc( ਆਕਾਰ(*pNew) );
  ਜੇ( pNew==0 ) ਵਾਪਸ 0;
  memset(pNew, 0, ਆਕਾਰ(*pNew));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  ਜੇ( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    ਜੇ( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_ਮੁੱਲ_Free(pNew);
      pNew = 0;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_ਪਦ|MEM_Subtype);
  }
  ਵਾਪਸ pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
ਖਾਲੀ sqlite3_ਮੁੱਲ_free(sqlite3_ਮੁੱਲ *pOld){
  sqlite3_ਮੁੱਲ_Free(pOld);
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
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  ਅਚਲ ਅੱਖਰ *z,          /* String pointer */
  ਪੂਰਨ n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)     /* Destructor function */
){
  Mem *p_ਬਾਹਰਲਾ = pCtx->p_ਬਾਹਰਲਾ;
  ਪੂਰਨ rc;
  ਜੇ( enc==SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(p_ਬਾਹਰਲਾ, z, n, xDel);
  }ਨਹੀਂਤਾਂ ਜੇ( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    ਪ੍ਰਤਿਜ੍ਞਾ( z==0
         || n<0 
         || n>p_ਬਾਹਰਲਾ->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]
         || z[n]==0
    );
    rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(p_ਬਾਹਰਲਾ, z, n, xDel);
    p_ਬਾਹਰਲਾ->flags |= MEM_ਪਦ;
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(p_ਬਾਹਰਲਾ, z, n, enc, xDel);
  }
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_TOOBIG ){
      sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
    }ਨਹੀਂਤਾਂ{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_NOMEM );
      sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
    }
    ਵਾਪਸ;
  }
  sqlite3VdbeChangeEncoding(p_ਬਾਹਰਲਾ, pCtx->enc);
  ਜੇ( sqlite3VdbeMemTooBig(p_ਬਾਹਰਲਾ) ){
    sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
  }
}
ਸਥਿਰ ਪੂਰਨ invoke_ਮੁੱਲ_Destructor(
  ਅਚਲ ਖਾਲੀ *p,             /* Value to destroy */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( xDel!=SQLITE_DYNAMIC );
  ਜੇ( xDel==0 ){
    /* noop */
  }ਨਹੀਂਤਾਂ ਜੇ( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }ਨਹੀਂਤਾਂ{
    xDel((ਖਾਲੀ*)p);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx!=0 ){
    sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
  }
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( pCtx!=0 );
  sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_TOOBIG;
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_blob(
  sqlite3_context *pCtx,
  ਅਚਲ ਖਾਲੀ *z,
  ਪੂਰਨ n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *)
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 || n<0 ){
    invoke_ਮੁੱਲ_Destructor(z, xDel, pCtx);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( n>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, n, 0, xDel);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_blob64(
  sqlite3_context *pCtx,
  ਅਚਲ ਖਾਲੀ *z,
  sqlite3_uint64 n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *)
){
  ਪ੍ਰਤਿਜ੍ਞਾ( xDel!=SQLITE_DYNAMIC );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ){
    invoke_ਮੁੱਲ_Destructor(z, xDel, 0);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਜੇ( n>0x7fffffff ){
    (ਖਾਲੀ)invoke_ਮੁੱਲ_Destructor(z, xDel, pCtx);
  }ਨਹੀਂਤਾਂ{
    ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, (ਪੂਰਨ)n, 0, xDel);
  }
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_double(sqlite3_context *pCtx, ਦੁੱਗਣਾ rVal){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Double(pCtx->p_ਬਾਹਰਲਾ, rVal);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_error(sqlite3_context *pCtx, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ n){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pCtx->p_ਬਾਹਰਲਾ, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_error16(sqlite3_context *pCtx, ਅਚਲ ਖਾਲੀ *z, ਪੂਰਨ n){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pCtx->p_ਬਾਹਰਲਾ, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_int(sqlite3_context *pCtx, ਪੂਰਨ iVal){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pCtx->p_ਬਾਹਰਲਾ, (i64)iVal);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_int64(sqlite3_context *pCtx, i64 iVal){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pCtx->p_ਬਾਹਰਲਾ, iVal);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_null(sqlite3_context *pCtx){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pCtx->p_ਬਾਹਰਲਾ);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_ਸੂਚਕ(
  sqlite3_context *pCtx,
  ਖਾਲੀ *pPtr,
  ਅਚਲ ਅੱਖਰ *zPType,
  ਖਾਲੀ (*xDestructor)(ਖਾਲੀ*)
){
  Mem *p_ਬਾਹਰਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ){
    invoke_ਮੁੱਲ_Destructor(pPtr, xDestructor, 0);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  p_ਬਾਹਰਲਾ = pCtx->p_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3VdbeMemRelease(p_ਬਾਹਰਲਾ);
  p_ਬਾਹਰਲਾ->flags = MEM_Null;
  sqlite3_Vdbe_Mem_ਸੈੱਟ_ਸੂਚਕ(p_ਬਾਹਰਲਾ, pPtr, zPType, xDestructor);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_subtype(sqlite3_context *pCtx, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ eSubtype){
  Mem *p_ਬਾਹਰਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  ਜੇ( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_ਨਤੀਜਾ_SUBTYPE)==0
  ){
    ਅੱਖਰ zErr[200];
    sqlite3_snprintf(ਆਕਾਰ(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_ਨਤੀਜਾ_error(pCtx, zErr, -1);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ /* SQLITE_STRICT_SUBTYPE */
  p_ਬਾਹਰਲਾ = pCtx->p_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਬਾਹਰਲਾ->db->mutex) );
  p_ਬਾਹਰਲਾ->eSubtype = eSubtype & 0xff;
  p_ਬਾਹਰਲਾ->flags |= MEM_Subtype;
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_ਪਾਠ(
  sqlite3_context *pCtx,
  ਅਚਲ ਅੱਖਰ *z,
  ਪੂਰਨ n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *)
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ){
    invoke_ਮੁੱਲ_Destructor(z, xDel, 0);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_text64(
  sqlite3_context *pCtx,
  ਅਚਲ ਅੱਖਰ *z,
  sqlite3_uint64 n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *),
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ enc
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ){
    invoke_ਮੁੱਲ_Destructor(z, xDel, 0);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( xDel!=SQLITE_DYNAMIC );
  ਜੇ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ਜੇ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  ਜੇ( n>0x7fffffff ){
    (ਖਾਲੀ)invoke_ਮੁੱਲ_Destructor(z, xDel, pCtx);
  }ਨਹੀਂਤਾਂ{
    ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, (ਪੂਰਨ)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->p_ਬਾਹਰਲਾ);
  }
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_text16(
  sqlite3_context *pCtx,
  ਅਚਲ ਖਾਲੀ *z,
  ਪੂਰਨ n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *)
){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_text16be(
  sqlite3_context *pCtx,
  ਅਚਲ ਖਾਲੀ *z,
  ਪੂਰਨ n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *)
){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_text16le(
  sqlite3_context *pCtx,
  ਅਚਲ ਖਾਲੀ *z,
  ਪੂਰਨ n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ *)
){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(sqlite3_context *pCtx, sqlite3_ਮੁੱਲ *p_ਮੁੱਲ){
  Mem *p_ਬਾਹਰਲਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
  ਜੇ( p_ਮੁੱਲ==0 ){
    sqlite3_ਨਤੀਜਾ_null(pCtx);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  p_ਬਾਹਰਲਾ = pCtx->p_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3VdbeMemCopy(p_ਬਾਹਰਲਾ, p_ਮੁੱਲ);
  sqlite3VdbeChangeEncoding(p_ਬਾਹਰਲਾ, pCtx->enc);
  ਜੇ( sqlite3VdbeMemTooBig(p_ਬਾਹਰਲਾ) ){
    sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
  }
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_zeroblob(sqlite3_context *pCtx, ਪੂਰਨ n){
  sqlite3_ਨਤੀਜਾ_zeroblob64(pCtx, n>0 ? n : 0);
}
ਪੂਰਨ sqlite3_ਨਤੀਜਾ_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *p_ਬਾਹਰਲਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  p_ਬਾਹਰਲਾ = pCtx->p_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਬਾਹਰਲਾ->db->mutex) );
  ਜੇ( n>(u64)p_ਬਾਹਰਲਾ->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] ){
    sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
    ਵਾਪਸ SQLITE_TOOBIG;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(pCtx->p_ਬਾਹਰਲਾ, (ਪੂਰਨ)n);
  ਵਾਪਸ SQLITE_OK;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(pCtx->p_ਬਾਹਰਲਾ, (ਪੂਰਨ)n);
#ਜੇ_ਅੰਤ
}
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(sqlite3_context *pCtx, ਪੂਰਨ err_ਕੋਡ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  pCtx->isError = err_ਕੋਡ ? err_ਕੋਡ : -1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( pCtx->pVdbe ) pCtx->pVdbe->rcApp = err_ਕੋਡ;
#ਜੇ_ਅੰਤ
  ਜੇ( pCtx->p_ਬਾਹਰਲਾ->flags & MEM_Null ){
    ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, sqlite3ErrStr(err_ਕੋਡ), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_error_toobig(sqlite3_context *pCtx){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pCtx->p_ਬਾਹਰਲਾ, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_error_nomem(sqlite3_context *pCtx){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pCtx->p_ਬਾਹਰਲਾ);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->p_ਬਾਹਰਲਾ->db);
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
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_str(sqlite3_context *pCtx, sqlite3_str *pStr, ਪੂਰਨ eOwn){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
  ਜੇ( pStr==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  ਜੇ( pStr->accError==0 ){
    ਜੇ( pStr->nChar==0 ){
      ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }ਨਹੀਂਤਾਂ{
      ਅਚਲ ਅੱਖਰ *z_ਪਾਠ = sqlite3_str_ਮੁੱਲ(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      ਪ੍ਰਤਿਜ੍ਞਾ( isMalloced(pStr) );
      ਜੇ( eOwn==SQLITE_COPY ){
        ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z_ਪਾਠ, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }ਨਹੀਂਤਾਂ{
        ਸੈੱਟ_ਨਤੀਜਾ_Str_Or_Error(pCtx, z_ਪਾਠ, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pStr->accError==SQLITE_NOMEM ){
    sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pStr->accError==SQLITE_TOOBIG );
    sqlite3_ਨਤੀਜਾ_error_toobig(pCtx);
  }
  ਜੇ( eOwn==SQLITE_ਅੰਤ ){
    sqlite3_str_free(pStr);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_Int_Real(sqlite3_context *pCtx){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
  ਜੇ( pCtx->p_ਬਾਹਰਲਾ->flags & MEM_Int ){
    pCtx->p_ਬਾਹਰਲਾ->flags &= ~MEM_Int;
    pCtx->p_ਬਾਹਰਲਾ->flags |= MEM_IntReal;
  }
}
#ਜੇ_ਅੰਤ


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ਸਥਿਰ ਪੂਰਨ doWalCallbacks(sqlite3 *db){
  ਪੂਰਨ rc = SQLITE_OK;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ i;
  ਲਈ(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ਜੇ( pBt ){
      ਪੂਰਨ n_ਇੰਦਰਾਜ;
      sqlite3BtreeEnter(pBt);
      n_ਇੰਦਰਾਜ = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      ਜੇ( n_ਇੰਦਰਾਜ>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, n_ਇੰਦਰਾਜ);
      }
    }
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(db);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ sqlite3_ਕਦਮ_2(Vdbe *p){
  sqlite3 *db;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ(p);
  db = p->db;
  ਜੇ( p->eVdbeState!=VDBE_ਲੜੀਵਾਰ_STATE ){
    restart_ਕਦਮ:
    ਜੇ( p->eVdbeState==VDBE_READY_STATE ){
      ਜੇ( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        ਜੇ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        ਜਾਓ ਸਿਰਾ_ਦਾ_ਕਦਮ;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      ਜੇ( db->n_Vdbe_ਸਰਗਰਮ==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      ਪ੍ਰਤਿਜ੍ਞਾ( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
      ਜੇ( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3_Os_Current_ਸਮਾਂ_Int64(db->pVfs, &p->ਸ਼ੁਰੂ_ਸਮਾਂ);
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( p->ਸ਼ੁਰੂ_ਸਮਾਂ==0 );
      }
#ਜੇ_ਅੰਤ

      db->n_Vdbe_ਸਰਗਰਮ++;
      ਜੇ( p->readOnly==0 ) db->nVdbeWrite++;
      ਜੇ( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_ਲੜੀਵਾਰ_STATE;
    }ਨਹੀਂਤਾਂ

    ਜੇ( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTORESET
      ਜੇ( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ SQLITE_MISUSE_BKPT;
      }
#ਨਹੀਂਤਾਂ
      sqlite3_reset((sqlite3_stmt*)p);
#ਜੇ_ਅੰਤ
      ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_READY_STATE );
      ਜਾਓ restart_ਕਦਮ;
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
  ਜੇ( p->explain ){
    rc = sqlite3VdbeList(p);
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->rc==SQLITE_OK );
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
    db->err_ਕੋਡ = SQLITE_ਪੰਕਤੀ;
    ਵਾਪਸ SQLITE_ਪੰਕਤੀ;
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#ਜੇ_ਅੰਤ
    p->p_ਨਤੀਜਾ_ਪੰਕਤੀ = 0;
    ਜੇ( rc==SQLITE_ਪੂਰਾ && db->autoCommit ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      ਜੇ( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( rc!=SQLITE_ਪੂਰਾ && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->err_ਕੋਡ = rc;
  ਜੇ( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    ਜੇ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
ਸਿਰਾ_ਦਾ_ਕਦਮ:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ਪੰਕਤੀ  || rc==SQLITE_ਪੂਰਾ   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  ਵਾਪਸ (rc&db->err_ਨਕਾਬ);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
ਪੂਰਨ sqlite3_ਕਦਮ(sqlite3_stmt *pStmt){
  ਪੂਰਨ rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  ਪੂਰਨ cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  ਜੇ( vdbeSafetyNotNull(v) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  ਜਦੋਂ( (rc = sqlite3_ਕਦਮ_2(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    ਪੂਰਨ savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    ਜੇ( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ਅਚਲ ਅੱਖਰ *zErr = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(db->pErr);
      sqlite3DbFree(db, v->z_Err_ਸੁਨੇਹਾ);
      ਜੇ( !db->mallocFailed ){
        v->z_Err_ਸੁਨੇਹਾ = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } ਨਹੀਂਤਾਂ {
        v->z_Err_ਸੁਨੇਹਾ = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      ਰੋਕੋ;
    }
    sqlite3_reset(pStmt);
    ਜੇ( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
ਖਾਲੀ *sqlite3_user_ਅੰਕੜੇ(sqlite3_context *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p && p->pFunc );
  ਵਾਪਸ p->pFunc->p_User_ਅੰਕੜੇ;
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ) ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( p && p->p_ਬਾਹਰਲਾ );
#ਜੇ_ਅੰਤ
  ਵਾਪਸ p->p_ਬਾਹਰਲਾ->db;
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
ਪੂਰਨ sqlite3_vtab_nochange(sqlite3_context *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ) ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
#ਜੇ_ਅੰਤ
  ਵਾਪਸ sqlite3_ਮੁੱਲ_nochange(p->p_ਬਾਹਰਲਾ);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
ਖਾਲੀ sqlite3_Vdbe_ਮੁੱਲ_List_Free(ਖਾਲੀ *p_ਮੰਜ਼ਿਲ_Delete){
  sqlite3_free(p_ਮੰਜ਼ਿਲ_Delete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ਸਥਿਰ ਪੂਰਨ ਮੁੱਲ_From_ਮੁੱਲ_List(
  sqlite3_ਮੁੱਲ *pVal,        /* Pointer to the ValueList object */
  sqlite3_ਮੁੱਲ **pp_ਬਾਹਰਲਾ,      /* Store the next value from the list here */
  ਪੂਰਨ b_ਅਗਲਾ                   /* 1 for _next(). 0 for _first() */
){
  ਪੂਰਨ rc;
  ਮੁੱਲ_List *pRhs;

  *pp_ਬਾਹਰਲਾ = 0;
  ਜੇ( pVal==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
  ਜੇ( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_ਮੁੱਲ_List_Free ){
    ਵਾਪਸ SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( (pVal->flags&(MEM_Type_ਨਕਾਬ|MEM_ਪਦ|MEM_Subtype)) ==
                 (MEM_Null|MEM_ਪਦ|MEM_Subtype) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pVal->eSubtype=='p' );
    ਪ੍ਰਤਿਜ੍ਞਾ( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (ਮੁੱਲ_List*)pVal->z;
  }
  ਜੇ( b_ਅਗਲਾ ){
    rc = sqlite3_Btree_ਅਗਲਾ(pRhs->pCsr, 0);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ dummy = 0;
    rc = sqlite3_Btree_ਪਹਿਲਾ(pRhs->pCsr, &dummy);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    ਜੇ( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_ਪੂਰਾ;
  }
  ਜੇ( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, ਆਕਾਰ(sMem));
    sz = sqlite3_Btree_Payload_ਮਾਪ(pRhs->pCsr);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ਵਿੱਥ(pRhs->pCsr,sz,&sMem);
    ਜੇ( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_ਮੁੱਲ *p_ਬਾਹਰਲਾ = pRhs->p_ਬਾਹਰਲਾ;
      ਪੂਰਨ iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, p_ਬਾਹਰਲਾ);
      p_ਬਾਹਰਲਾ->enc = ENC(p_ਬਾਹਰਲਾ->db);
      ਜੇ( (p_ਬਾਹਰਲਾ->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(p_ਬਾਹਰਲਾ) ){
        rc = SQLITE_NOMEM;
      }ਨਹੀਂਤਾਂ{
        *pp_ਬਾਹਰਲਾ = p_ਬਾਹਰਲਾ;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  ਵਾਪਸ rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ਪੂਰਨ sqlite3_vtab_ਵਿੱਚ_ਪਹਿਲਾ(sqlite3_ਮੁੱਲ *pVal, sqlite3_ਮੁੱਲ **pp_ਬਾਹਰਲਾ){
  ਵਾਪਸ ਮੁੱਲ_From_ਮੁੱਲ_List(pVal, pp_ਬਾਹਰਲਾ, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ਪੂਰਨ sqlite3_vtab_ਵਿੱਚ_ਅਗਲਾ(sqlite3_ਮੁੱਲ *pVal, sqlite3_ਮੁੱਲ **pp_ਬਾਹਰਲਾ){
  ਵਾਪਸ ਮੁੱਲ_From_ਮੁੱਲ_List(pVal, pp_ਬਾਹਰਲਾ, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3_Stmt_Current_ਸਮਾਂ(sqlite3_context *p){
  ਪੂਰਨ rc;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  sqlite3_int64 *pi_ਸਮਾਂ = &p->pVdbe->i_Current_ਸਮਾਂ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pVdbe!=0 );
#ਨਹੀਂਤਾਂ
  sqlite3_int64 i_ਸਮਾਂ = 0;
  sqlite3_int64 *pi_ਸਮਾਂ = p->pVdbe!=0 ? &p->pVdbe->i_Current_ਸਮਾਂ : &i_ਸਮਾਂ;
#ਜੇ_ਅੰਤ
  ਜੇ( *pi_ਸਮਾਂ==0 ){
    rc = sqlite3_Os_Current_ਸਮਾਂ_Int64(p->p_ਬਾਹਰਲਾ->db->pVfs, pi_ਸਮਾਂ);
    ਜੇ( rc ) *pi_ਸਮਾਂ = 0;
  }
  ਵਾਪਸ *pi_ਸਮਾਂ;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ *createAggContext(sqlite3_context *p, ਪੂਰਨ nByte){
  Mem *pMem = p->pMem;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & MEM_Agg)==0 );
  ਜੇ( nByte<=0 ){
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
    pMem->z = 0;
  }ਨਹੀਂਤਾਂ{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    ਜੇ( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  ਵਾਪਸ (ਖਾਲੀ*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
ਖਾਲੀ *sqlite3_aggregate_context(sqlite3_context *p, ਪੂਰਨ nByte){
  ਪ੍ਰਤਿਜ੍ਞਾ( p && p->pFunc && p->pFunc->xFinalize );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->p_ਬਾਹਰਲਾ->db->mutex) );
  testcase( nByte<0 );
  ਜੇ( (p->pMem->flags & MEM_Agg)==0 ){
    ਵਾਪਸ createAggContext(p, nByte);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ (ਖਾਲੀ*)p->pMem->z;
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
ਖਾਲੀ *sqlite3_get_auxdata(sqlite3_context *pCtx, ਪੂਰਨ iArg){
  Aux_ਅੰਕੜੇ *p_Aux_ਅੰਕੜੇ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
#ਜੇ SQLITE_ENABLE_STAT4
  ਜੇ( pCtx->pVdbe==0 ) ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( pCtx->pVdbe!=0 );
#ਜੇ_ਅੰਤ
  ਲਈ(p_Aux_ਅੰਕੜੇ=pCtx->pVdbe->p_Aux_ਅੰਕੜੇ; p_Aux_ਅੰਕੜੇ; p_Aux_ਅੰਕੜੇ=p_Aux_ਅੰਕੜੇ->p_ਅਗਲਾ_Aux){
    ਜੇ(  p_Aux_ਅੰਕੜੇ->iAuxArg==iArg && (p_Aux_ਅੰਕੜੇ->iAuxOp==pCtx->iOp || iArg<0) ){
      ਵਾਪਸ p_Aux_ਅੰਕੜੇ->pAux;
    }
  }
  ਵਾਪਸ 0;
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
ਖਾਲੀ sqlite3_ਸੈੱਟ_auxdata(
  sqlite3_context *pCtx,
  ਪੂਰਨ iArg,
  ਖਾਲੀ *pAux,
  ਖਾਲੀ (*xDelete)(ਖਾਲੀ*)
){
  Aux_ਅੰਕੜੇ *p_Aux_ਅੰਕੜੇ;
  Vdbe *pVdbe;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCtx==0 ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  pVdbe= pCtx->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCtx->p_ਬਾਹਰਲਾ->db->mutex) );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( pVdbe==0 ) ਜਾਓ failed;
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( pVdbe!=0 );
#ਜੇ_ਅੰਤ

  ਲਈ(p_Aux_ਅੰਕੜੇ=pVdbe->p_Aux_ਅੰਕੜੇ; p_Aux_ਅੰਕੜੇ; p_Aux_ਅੰਕੜੇ=p_Aux_ਅੰਕੜੇ->p_ਅਗਲਾ_Aux){
    ਜੇ( p_Aux_ਅੰਕੜੇ->iAuxArg==iArg && (p_Aux_ਅੰਕੜੇ->iAuxOp==pCtx->iOp || iArg<0) ){
      ਰੋਕੋ;
    }
  }
  ਜੇ( p_Aux_ਅੰਕੜੇ==0 ){
    p_Aux_ਅੰਕੜੇ = sqlite3DbMallocZero(pVdbe->db, ਆਕਾਰ(Aux_ਅੰਕੜੇ));
    ਜੇ( !p_Aux_ਅੰਕੜੇ ) ਜਾਓ failed;
    p_Aux_ਅੰਕੜੇ->iAuxOp = pCtx->iOp;
    p_Aux_ਅੰਕੜੇ->iAuxArg = iArg;
    p_Aux_ਅੰਕੜੇ->p_ਅਗਲਾ_Aux = pVdbe->p_Aux_ਅੰਕੜੇ;
    pVdbe->p_Aux_ਅੰਕੜੇ = p_Aux_ਅੰਕੜੇ;
    ਜੇ( pCtx->isError==0 ) pCtx->isError = -1;
  }ਨਹੀਂਤਾਂ ਜੇ( p_Aux_ਅੰਕੜੇ->xDeleteAux ){
    p_Aux_ਅੰਕੜੇ->xDeleteAux(p_Aux_ਅੰਕੜੇ->pAux);
  }

  p_Aux_ਅੰਕੜੇ->pAux = pAux;
  p_Aux_ਅੰਕੜੇ->xDeleteAux = xDelete;
  ਵਾਪਸ;

failed:
  ਜੇ( xDelete ){
    xDelete(pAux);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
ਪੂਰਨ sqlite3_aggregate_ਗਿਣਤੀ(sqlite3_context *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  ਵਾਪਸ p->pMem->n;
}
#ਜੇ_ਅੰਤ

/*
** Return the number of columns in the result set for the statement pStmt.
*/
ਪੂਰਨ sqlite3_ਕਾਲਮ_ਗਿਣਤੀ(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ਜੇ( pVm==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ pVm->n_Res_ਕਾਲਮ;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
ਪੂਰਨ sqlite3_ਅੰਕੜੇ_ਗਿਣਤੀ(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ਜੇ( pVm==0 || pVm->p_ਨਤੀਜਾ_ਪੰਕਤੀ==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ pVm->n_Res_ਕਾਲਮ;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ਸਥਿਰ ਅਚਲ Mem *ਕਾਲਮ_Null_ਮੁੱਲ(ਖਾਲੀ){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ਸਥਿਰ ਅਚਲ Mem nullMem
#ਜੇ defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#ਜੇ_ਅੰਤ
    = {
        /* .u          = */ {0},
        /* .z          = */ (ਅੱਖਰ*)0,
        /* .n          = */ (ਪੂਰਨ)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (ਪੂਰਨ)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (ਅੱਖਰ*)0,
        /* .xDel       = */ (ਖਾਲੀ(*)(ਖਾਲੀ*))0,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#ਜੇ_ਅੰਤ
      };
  ਵਾਪਸ &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ਸਥਿਰ Mem *ਕਾਲਮ_Mem(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  Vdbe *pVm;
  Mem *p_ਬਾਹਰਲਾ;

  pVm = (Vdbe *)pStmt;
  ਜੇ( pVm==0 ) ਵਾਪਸ (Mem*)ਕਾਲਮ_Null_ਮੁੱਲ();
  ਪ੍ਰਤਿਜ੍ਞਾ( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  ਜੇ( pVm->p_ਨਤੀਜਾ_ਪੰਕਤੀ!=0 && i<pVm->n_Res_ਕਾਲਮ && i>=0 ){
    p_ਬਾਹਰਲਾ = &pVm->p_ਨਤੀਜਾ_ਪੰਕਤੀ[i];
  }ਨਹੀਂਤਾਂ{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    p_ਬਾਹਰਲਾ = (Mem*)ਕਾਲਮ_Null_ਮੁੱਲ();
  }
  ਵਾਪਸ p_ਬਾਹਰਲਾ;
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
ਸਥਿਰ ਖਾਲੀ ਕਾਲਮ_Malloc_Failure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->db!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_blob(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਅਚਲ ਖਾਲੀ *val;
  val = sqlite3_ਮੁੱਲ_blob( ਕਾਲਮ_Mem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
ਪੂਰਨ sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਪੂਰਨ val = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
ਪੂਰਨ sqlite3_ਕਾਲਮ_bytes16(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਪੂਰਨ val = sqlite3_ਮੁੱਲ_bytes16( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
ਦੁੱਗਣਾ sqlite3_ਕਾਲਮ_double(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਦੁੱਗਣਾ val = sqlite3_ਮੁੱਲ_double( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
ਪੂਰਨ sqlite3_ਕਾਲਮ_int(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਪੂਰਨ val = sqlite3_ਮੁੱਲ_int( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
sqlite_int64 sqlite3_ਕਾਲਮ_int64(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  sqlite_int64 val = sqlite3_ਮੁੱਲ_int64( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *sqlite3_ਕਾਲਮ_ਪਾਠ(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *val = sqlite3_ਮੁੱਲ_ਪਾਠ( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
sqlite3_ਮੁੱਲ *sqlite3_ਕਾਲਮ_ਮੁੱਲ(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  Mem *p_ਬਾਹਰਲਾ = ਕਾਲਮ_Mem(pStmt, i);
  ਜੇ( p_ਬਾਹਰਲਾ->flags&MEM_Static ){
    p_ਬਾਹਰਲਾ->flags &= ~MEM_Static;
    p_ਬਾਹਰਲਾ->flags |= MEM_Ephem;
  }
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ (sqlite3_ਮੁੱਲ *)p_ਬਾਹਰਲਾ;
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_text16(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਅਚਲ ਖਾਲੀ *val = sqlite3_ਮੁੱਲ_text16( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ val;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
ਪੂਰਨ sqlite3_ਕਾਲਮ_type(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਪੂਰਨ iType = sqlite3_ਮੁੱਲ_type( ਕਾਲਮ_Mem(pStmt,i) );
  ਕਾਲਮ_Malloc_Failure(pStmt);
  ਵਾਪਸ iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ * ਅਚਲ az_Explain_ਖਾਨਾ_Names8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ਸਥਿਰ ਅਚਲ u16 az_Explain_ਖਾਨਾ_Names16data[] = {
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
ਸਥਿਰ ਅਚਲ u8 i_Explain_ਖਾਨਾ_Names16[] = {
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
ਸਥਿਰ ਅਚਲ ਖਾਲੀ *ਕਾਲਮ_Name(
  sqlite3_stmt *pStmt,     /* The statement */
  ਪੂਰਨ N,                   /* Which column to get the name for */
  ਪੂਰਨ useUtf16,            /* True to return the name as UTF16 */
  ਪੂਰਨ useType              /* What type of name */
){
  ਅਚਲ ਖਾਲੀ *ret;
  Vdbe *p;
  ਪੂਰਨ n;
  sqlite3 *db;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pStmt==0 ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਜੇ( N<0 ) ਵਾਪਸ 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  ਜੇ( p->explain ){
    ਜੇ( useType>0 ) ਜਾਓ ਕਾਲਮ_Name_ਸਿਰਾ;
    n = p->explain==1 ? 8 : 4;
    ਜੇ( N>=n ) ਜਾਓ ਕਾਲਮ_Name_ਸਿਰਾ;
    ਜੇ( useUtf16 ){
      ਪੂਰਨ i = i_Explain_ਖਾਨਾ_Names16[N + 8*p->explain - 8];
      ret = (ਖਾਲੀ*)&az_Explain_ਖਾਨਾ_Names16data[i];
    }ਨਹੀਂਤਾਂ{
      ret = (ਖਾਲੀ*)az_Explain_ਖਾਨਾ_Names8[N + 8*p->explain - 8];
    }
    ਜਾਓ ਕਾਲਮ_Name_ਸਿਰਾ;
  }
  n = p->n_Res_ਕਾਲਮ;
  ਜੇ( N<n ){
    u8 ਪੂਰਵ_malloc_Failed = db->mallocFailed;
    N += useType*n;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
    ਜੇ( useUtf16 ){
      ret = sqlite3_ਮੁੱਲ_text16((sqlite3_ਮੁੱਲ*)&p->a_ਖਾਨਾ_Name[N]);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ret = sqlite3_ਮੁੱਲ_ਪਾਠ((sqlite3_ਮੁੱਲ*)&p->a_ਖਾਨਾ_Name[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 || db->mallocFailed==1 );
    ਜੇ( db->mallocFailed > ਪੂਰਵ_malloc_Failed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
ਕਾਲਮ_Name_ਸਿਰਾ:
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_ਕਾਲਮ_name(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 0, COLNAME_NAME);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_name16(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 1, COLNAME_NAME);
}
#ਜੇ_ਅੰਤ

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#ਜੇ defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_ਕਾਲਮ_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_ਕਾਲਮ_METADATA"
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_ਕਾਲਮ_decltype(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_decltype16(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 1, COLNAME_DECLTYPE);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DECLTYPE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_ਕਾਲਮ_database_name(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 0, COLNAME_DATABASE);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_database_name16(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 1, COLNAME_DATABASE);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 0, COLNAME_ਸਾਰਣੀ);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name16(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 1, COLNAME_ਸਾਰਣੀ);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_ਕਾਲਮ_origin_name(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 0, COLNAME_ਕਾਲਮ);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅਚਲ ਖਾਲੀ *sqlite3_ਕਾਲਮ_origin_name16(sqlite3_stmt *pStmt, ਪੂਰਨ N){
  ਵਾਪਸ ਕਾਲਮ_Name(pStmt, N, 1, COLNAME_ਕਾਲਮ);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_COLUMN_METADATA */


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
ਸਥਿਰ ਪੂਰਨ vdbeUnbind(Vdbe *p, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i){
  Mem *pVar;
  ਜੇ( vdbeSafetyNotNull(p) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ਜੇ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  ਜੇ( i>=(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ਵਾਪਸ SQLITE_RANGE;
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
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->expmask & p->smimask)==p->smimask );
  ਜੇ( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    ਜੇ( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->err_ਕੋਡ = SQLITE_OK;

  ਵਾਪਸ SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ਸਥਿਰ ਪੂਰਨ bind_ਪਾਠ_2(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  ਪੂਰਨ i,                 /* Index of the parameter to bind */
  ਅਚਲ ਖਾਲੀ *z_ਅੰਕੜੇ,     /* Pointer to the data to be bound */
  i64 n_ਅੰਕੜੇ,             /* Number of bytes of data to be bound */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  ਪੂਰਨ rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    ਜੇ( z_ਅੰਕੜੇ!=0 ){
      pVar = &p->aVar[i-1];
      ਜੇ( encoding==SQLITE_UTF8 ){
        rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(pVar, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, xDel);
      }ਨਹੀਂਤਾਂ ਜੇ( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        ਪ੍ਰਤਿਜ੍ਞਾ( z_ਅੰਕੜੇ==0
             || n_ਅੰਕੜੇ<0 
             || n_ਅੰਕੜੇ>pVar->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]
             || ((u8*)z_ਅੰਕੜੇ)[n_ਅੰਕੜੇ]==0
        );
        rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(pVar, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, xDel);
        pVar->flags |= MEM_ਪਦ;
      }ਨਹੀਂਤਾਂ{
        rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pVar, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, encoding, xDel);
        ਜੇ( encoding==0 ) pVar->enc = ENC(p->db);
      }
      ਜੇ( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      ਜੇ( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }ਨਹੀਂਤਾਂ ਜੇ( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((ਖਾਲੀ*)z_ਅੰਕੜੇ);
  }
  ਵਾਪਸ rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
ਪੂਰਨ sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ i,
  ਅਚਲ ਖਾਲੀ *z_ਅੰਕੜੇ,
  ਪੂਰਨ n_ਅੰਕੜੇ,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( n_ਅੰਕੜੇ<0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ bind_ਪਾਠ_2(pStmt, i, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, xDel, 0);
}
ਪੂਰਨ sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ i,
  ਅਚਲ ਖਾਲੀ *z_ਅੰਕੜੇ,
  sqlite3_uint64 n_ਅੰਕੜੇ,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)
){
  ਪ੍ਰਤਿਜ੍ਞਾ( xDel!=SQLITE_DYNAMIC );
  ਵਾਪਸ bind_ਪਾਠ_2(pStmt, i, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, xDel, 0);
}
ਪੂਰਨ sqlite3_bind_double(sqlite3_stmt *pStmt, ਪੂਰਨ i, ਦੁੱਗਣਾ r_ਮੁੱਲ){
  ਪੂਰਨ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Double(&p->aVar[i-1], r_ਮੁੱਲ);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_bind_int(sqlite3_stmt *p, ਪੂਰਨ i, ਪੂਰਨ i_ਮੁੱਲ){
  ਵਾਪਸ sqlite3_bind_int64(p, i, (i64)i_ਮੁੱਲ);
}
ਪੂਰਨ sqlite3_bind_int64(sqlite3_stmt *pStmt, ਪੂਰਨ i, sqlite_int64 i_ਮੁੱਲ){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  ਜੇ( vdbeSafetyNotNull(p) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ਜੇ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  ਜੇ( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ਵਾਪਸ SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  ਜੇ( p->expmask!=0 ){
    u32 expire_ਨਕਾਬ = i>=32 ? 0x80000000 : (u32)1 << i;
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->expmask & p->smimask)==p->smimask );
    ਜੇ( (p->expmask & expire_ਨਕਾਬ)!=0 ){
      ਜੇ( (p->smimask & expire_ਨਕਾਬ)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        ਪੂਰਨ sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        ਪੂਰਨ sm2 = i_ਮੁੱਲ>=0 && i_ਮੁੱਲ<=1;
        ਜੇ( sm1!=sm2 ){
          p->expired = 1;
        }
      }ਨਹੀਂਤਾਂ ਜੇ( (pVar->flags & MEM_Int)==0 || i_ਮੁੱਲ!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pVar, i_ਮੁੱਲ);
  sqlite3_mutex_leave(p->db->mutex);
  ਵਾਪਸ SQLITE_OK;
}
ਪੂਰਨ sqlite3_bind_null(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  ਪੂਰਨ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_bind_ਸੂਚਕ(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ i,
  ਖਾਲੀ *pPtr,
  ਅਚਲ ਅੱਖਰ *zPTtype,
  ਖਾਲੀ (*xDestructor)(ਖਾਲੀ*)
){
  ਪੂਰਨ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_ਸੈੱਟ_ਸੂਚਕ(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }ਨਹੀਂਤਾਂ ਜੇ( xDestructor ){
    xDestructor(pPtr);
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_bind_ਪਾਠ(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ i,
  ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ,
  ਪੂਰਨ n_ਅੰਕੜੇ,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)
){
  ਵਾਪਸ bind_ਪਾਠ_2(pStmt, i, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, xDel, SQLITE_UTF8);
}
ਪੂਰਨ sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ i,
  ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ,
  sqlite3_uint64 n_ਅੰਕੜੇ,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*),
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ enc
){
  ਪ੍ਰਤਿਜ੍ਞਾ( xDel!=SQLITE_DYNAMIC );
  ਜੇ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ਜੇ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_ਅੰਕੜੇ &= ~(u64)1;
  }
  ਵਾਪਸ bind_ਪਾਠ_2(pStmt, i, z_ਅੰਕੜੇ, n_ਅੰਕੜੇ, xDel, enc);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਪੂਰਨ sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ i,
  ਅਚਲ ਖਾਲੀ *z_ਅੰਕੜੇ,
  ਪੂਰਨ n,
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)
){
  ਵਾਪਸ bind_ਪਾਠ_2(pStmt, i, z_ਅੰਕੜੇ, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
ਪੂਰਨ sqlite3_bind_ਮੁੱਲ(sqlite3_stmt *pStmt, ਪੂਰਨ i, ਅਚਲ sqlite3_ਮੁੱਲ *p_ਮੁੱਲ){
  ਪੂਰਨ rc;
  ਚੋਣ( sqlite3_ਮੁੱਲ_type((sqlite3_ਮੁੱਲ*)p_ਮੁੱਲ) ){
    ਹਾਲਤ SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_ਮੁੱਲ->u.i);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_FLOAT: {
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੁੱਲ->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_ਮੁੱਲ->flags & MEM_Real) ? p_ਮੁੱਲ->u.r : (ਦੁੱਗਣਾ)p_ਮੁੱਲ->u.i
      );
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_BLOB: {
      ਜੇ( p_ਮੁੱਲ->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_ਮੁੱਲ->u.nZero);
      }ਨਹੀਂਤਾਂ{
        rc = sqlite3_bind_blob(pStmt, i, p_ਮੁੱਲ->z, p_ਮੁੱਲ->n,SQLITE_TRANSIENT);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪਾਠ: {
      rc = bind_ਪਾਠ_2(pStmt,i,  p_ਮੁੱਲ->z, p_ਮੁੱਲ->n, SQLITE_TRANSIENT,
                              p_ਮੁੱਲ->enc);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      rc = sqlite3_bind_null(pStmt, i);
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, ਪੂਰਨ i, ਪੂਰਨ n){
  ਪੂਰਨ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(&p->aVar[i-1], n);
#ਨਹੀਂਤਾਂ
    rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(&p->aVar[i-1], n);
#ਜੇ_ਅੰਤ
    sqlite3_mutex_leave(p->db->mutex);
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, ਪੂਰਨ i, sqlite3_uint64 n){
  ਪੂਰਨ rc;
  Vdbe *p = (Vdbe *)pStmt;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(p->db->mutex);
  ਜੇ( n>(u64)p->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] ){
    rc = SQLITE_TOOBIG;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  ਵਾਪਸ rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
ਪੂਰਨ sqlite3_bind_parameter_ਗਿਣਤੀ(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ਵਾਪਸ p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, ਪੂਰਨ i){
  Vdbe *p = (Vdbe*)pStmt;
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ sqlite3_VList_ਅੰਕ_ਮੰਜ਼ਿਲ_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
ਪੂਰਨ sqlite3_Vdbe_Parameter_ਸੂਚਕਾਂਕ(Vdbe *p, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ nName){
  ਜੇ( p==0 || zName==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ sqlite3_VList_Name_ਮੰਜ਼ਿਲ_ਅੰਕ(p->pVList, zName, nName);
}
ਪੂਰਨ sqlite3_bind_parameter_ਸੂਚਕਾਂਕ(sqlite3_stmt *pStmt, ਅਚਲ ਅੱਖਰ *zName){
  ਵਾਪਸ sqlite3_Vdbe_Parameter_ਸੂਚਕਾਂਕ((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
ਪੂਰਨ sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_ਮੰਜ਼ਿਲ_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_ਮੰਜ਼ਿਲ = (Vdbe*)p_ਮੰਜ਼ਿਲ_Stmt;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ->db==pFrom->db );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ->nVar==pFrom->nVar );
  sqlite3_mutex_enter(p_ਮੰਜ਼ਿਲ->db->mutex);
  ਲਈ(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&p_ਮੰਜ਼ਿਲ->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(p_ਮੰਜ਼ਿਲ->db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
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
ਪੂਰਨ sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_ਮੰਜ਼ਿਲ_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_ਮੰਜ਼ਿਲ = (Vdbe*)p_ਮੰਜ਼ਿਲ_Stmt;
  ਜੇ( pFrom->nVar!=p_ਮੰਜ਼ਿਲ->nVar ){
    ਵਾਪਸ SQLITE_ERROR;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਮੰਜ਼ਿਲ->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p_ਮੰਜ਼ਿਲ->expmask==0 );
  ਜੇ( p_ਮੰਜ਼ਿਲ->expmask ){
    p_ਮੰਜ਼ਿਲ->expired = 1;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  ਜੇ( pFrom->expmask ){
    pFrom->expired = 1;
  }
  ਵਾਪਸ sqlite3TransferBindings(pFromStmt, p_ਮੰਜ਼ਿਲ_Stmt);
}
#ਜੇ_ਅੰਤ

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  ਵਾਪਸ pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
ਪੂਰਨ sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  ਵਾਪਸ pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
ਪੂਰਨ sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  ਵਾਪਸ pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
ਪੂਰਨ sqlite3_stmt_explain(sqlite3_stmt *pStmt, ਪੂਰਨ e_ਬਹੁਲਕ){
  Vdbe *v = (Vdbe*)pStmt;
  ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pStmt==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(v->db->mutex);
  ਜੇ( ((ਪੂਰਨ)v->explain)==e_ਬਹੁਲਕ ){
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ ਜੇ( e_ਬਹੁਲਕ<0 || e_ਬਹੁਲਕ>2 ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ ਜੇ( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ ਜੇ( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }ਨਹੀਂਤਾਂ ਜੇ( v->nMem>=10 && (e_ਬਹੁਲਕ!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = e_ਬਹੁਲਕ;
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ saved_ਬਹੁਲਕ = v->explain;
    v->explain = e_ਬਹੁਲਕ;
    rc = sqlite3Reprepare(v);
    ਜੇ( rc ){
      v->explain = saved_ਬਹੁਲਕ;
    }ਨਹੀਂਤਾਂ{
      v->haveEqpOps = e_ਬਹੁਲਕ==2;
    }
  }
  ਜੇ( v->explain ){
    v->n_Res_ਕਾਲਮ = 12 - 4*v->explain;
  }ਨਹੀਂਤਾਂ{
    v->n_Res_ਕਾਲਮ = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  ਵਾਪਸ rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
ਪੂਰਨ sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  ਵਾਪਸ v!=0 && v->eVdbeState==VDBE_ਲੜੀਵਾਰ_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_ਅਗਲਾ_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_ਅਗਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(pDb) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(pDb->mutex);
  ਜੇ( pStmt==0 ){
    p_ਅਗਲਾ = (sqlite3_stmt*)pDb->pVdbe;
  }ਨਹੀਂਤਾਂ{
    p_ਅਗਲਾ = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  ਵਾਪਸ p_ਅਗਲਾ;
}

/*
** Return the value of a status counter for a prepared statement
*/
ਪੂਰਨ sqlite3_stmt_ਸਥਿਤੀ(sqlite3_stmt *pStmt, ਪੂਰਨ op, ਪੂਰਨ resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_ਮਾਪ(pVdbe->aCounter)))
  ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਜੇ( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pn_ਬਾਈਟਾਂ_Freed = (ਪੂਰਨ*)&v;
    ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.p_ਸਿਰਾ==db->lookaside.p_True_ਸਿਰਾ );
    db->lookaside.p_ਸਿਰਾ = db->lookaside.p_ਸ਼ੁਰੂ;
    sqlite3VdbeDelete(pVdbe);
    db->pn_ਬਾਈਟਾਂ_Freed = 0;
    db->lookaside.p_ਸਿਰਾ = db->lookaside.p_True_ਸਿਰਾ;
    sqlite3_mutex_leave(db->mutex);
  }ਨਹੀਂਤਾਂ{
    v = pVdbe->aCounter[op];
    ਜੇ( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  ਵਾਪਸ (ਪੂਰਨ)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ਅਚਲ ਅੱਖਰ *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ਵਾਪਸ p ? p->zSql : 0;
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
ਅੱਖਰ *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
  ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
  ਅੱਖਰ *z = 0;
  ਅਚਲ ਅੱਖਰ *zSql = sqlite3_sql(pStmt);
  ਜੇ( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ਵਾਪਸ z;
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  ਜੇ( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ਵਾਪਸ p->zNormSql;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_NORMALIZE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ਸਥਿਰ Unpacked_ਰਿਕਾਰਡ *vdbe_Unpack_ਰਿਕਾਰਡ(
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info,
  ਪੂਰਨ n_ਕੁੰਜੀ,
  ਅਚਲ ਖਾਲੀ *p_ਕੁੰਜੀ
){
  Unpacked_ਰਿਕਾਰਡ *pRet;           /* Return value */

  pRet = sqlite3_Vdbe_Alloc_Unpacked_ਰਿਕਾਰਡ(p_ਕੁੰਜੀ_Info);
  ਜੇ( pRet ){
    memset(pRet->aMem, 0, ਆਕਾਰ(Mem)*(p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ+1));
    sqlite3_Vdbe_ਰਿਕਾਰਡ_Unpack(n_ਕੁੰਜੀ, p_ਕੁੰਜੀ, pRet);
  }
  ਵਾਪਸ pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
ਪੂਰਨ sqlite3_preupdate_old(sqlite3 *db, ਪੂਰਨ iIdx, sqlite3_ਮੁੱਲ **pp_ਮੁੱਲ){
  PreUpdate *p;
  Mem *pMem;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iStore = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( db==0 || pp_ਮੁੱਲ==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  ਜੇ( !p || p->op==SQLITE_ਪਾਓ ){
    rc = SQLITE_MISUSE_BKPT;
    ਜਾਓ preupdate_old_ਬਾਹਰਲਾ;
  }
  ਜੇ( p->pPk ){
    iStore = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(p->pPk, iIdx);
  }ਨਹੀਂਤਾਂ ਜੇ( iIdx >= p->pTab->n_ਖਾਨਾ ){
    rc = SQLITE_MISUSE_BKPT;
    ਜਾਓ preupdate_old_ਬਾਹਰਲਾ;
  }ਨਹੀਂਤਾਂ{
    iStore = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(p->pTab, iIdx);
  }
  ਜੇ( iStore>=p->pCsr->n_ਖੇਤਰ || iStore<0 ){
    rc = SQLITE_RANGE;
    ਜਾਓ preupdate_old_ਬਾਹਰਲਾ;
  }

  ਜੇ( iIdx==p->pTab->iPKey ){
    *pp_ਮੁੱਲ = pMem = &p->oldipk;
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem, p->iKey1);
  }ਨਹੀਂਤਾਂ{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    ਜੇ( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      ਪ੍ਰਤਿਜ੍ਞਾ( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_ਮਾਪ(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      ਜੇ( !aRec ) ਜਾਓ preupdate_old_ਬਾਹਰਲਾ;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      ਜੇ( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbe_Unpack_ਰਿਕਾਰਡ(p->pKeyinfo, nRec, aRec);
        ਜੇ( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      ਜੇ( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        ਜਾਓ preupdate_old_ਬਾਹਰਲਾ;
      }
      p->a_ਰਿਕਾਰਡ = aRec;
    }

    pMem = *pp_ਮੁੱਲ = &p->pUnpacked->aMem[iStore];
    ਜੇ( iStore>=p->pUnpacked->n_ਖੇਤਰ ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      ਕਾਲਮ *p_ਖਾਨਾ = &p->pTab->a_ਖਾਨਾ[iIdx];
      ਜੇ( p_ਖਾਨਾ->iDflt>0 ){
        ਜੇ( p->apDflt==0 ){
          ਪੂਰਨ nByte;
          ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(sqlite3_ਮੁੱਲ*)*UMXV(p->pTab->n_ਖਾਨਾ) < 0x7fffffff );
          nByte = ਆਕਾਰ(sqlite3_ਮੁੱਲ*)*p->pTab->n_ਖਾਨਾ;
          p->apDflt = (sqlite3_ਮੁੱਲ**)sqlite3DbMallocZero(db, nByte);
          ਜੇ( p->apDflt==0 ) ਜਾਓ preupdate_old_ਬਾਹਰਲਾ;
        }
        ਜੇ( p->apDflt[iIdx]==0 ){
          sqlite3_ਮੁੱਲ *pVal = 0;
          ਵਿਅੰਜਕ *pDflt;
          ਪ੍ਰਤਿਜ੍ਞਾ( p->pTab!=0 && Is_Ordinary_ਸਾਰਣੀ(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[p_ਖਾਨਾ->iDflt-1].p_ਵਿਅੰਜਕ;
          rc = sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(db, pDflt, ENC(db), p_ਖਾਨਾ->affinity, &pVal);
          ਜੇ( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_ਮੁੱਲ = p->apDflt[iIdx];
      }ਨਹੀਂਤਾਂ{
        *pp_ਮੁੱਲ = (sqlite3_ਮੁੱਲ *)ਕਾਲਮ_Null_ਮੁੱਲ();
      }
    }ਨਹੀਂਤਾਂ ਜੇ( p->pTab->a_ਖਾਨਾ[iIdx].affinity==SQLITE_AFF_REAL ){
      ਜੇ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_ਬਾਹਰਲਾ:
  sqlite3Error(db, rc);
  ਵਾਪਸ sqlite3ApiExit(db, rc);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
ਪੂਰਨ sqlite3_preupdate_ਗਿਣਤੀ(sqlite3 *db){
  PreUpdate *p;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ਨਹੀਂਤਾਂ
  p = db->pPreUpdate;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ (p ? p->pKeyinfo->n_ਕੁੰਜੀ_ਖੇਤਰ : 0);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
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
ਪੂਰਨ sqlite3_preupdate_ਡੂੰਘਾਈ(sqlite3 *db){
  PreUpdate *p;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ਨਹੀਂਤਾਂ
  p = db->pPreUpdate;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ (p ? p->v->n_ਚੌਖਟਾ : 0);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
ਪੂਰਨ sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ਨਹੀਂਤਾਂ
  p = db->pPreUpdate;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ (p ? p->iBlobWrite : -1);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
ਪੂਰਨ sqlite3_preupdate_new(sqlite3 *db, ਪੂਰਨ iIdx, sqlite3_ਮੁੱਲ **pp_ਮੁੱਲ){
  PreUpdate *p;
  ਪੂਰਨ rc = SQLITE_OK;
  Mem *pMem;
  ਪੂਰਨ iStore = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( db==0 || pp_ਮੁੱਲ==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  p = db->pPreUpdate;
  ਜੇ( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    ਜਾਓ preupdate_new_ਬਾਹਰਲਾ;
  }
  ਜੇ( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(p->pPk, iIdx);
  }ਨਹੀਂਤਾਂ ਜੇ( iIdx >= p->pTab->n_ਖਾਨਾ ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }ਨਹੀਂਤਾਂ{
    iStore = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(p->pTab, iIdx);
  }

  ਜੇ( iStore>=p->pCsr->n_ਖੇਤਰ || iStore<0 ){
    rc = SQLITE_RANGE;
    ਜਾਓ preupdate_new_ਬਾਹਰਲਾ;
  }

  ਜੇ( p->op==SQLITE_ਪਾਓ ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    Unpacked_ਰਿਕਾਰਡ *pUnpack = p->pNewUnpacked;
    ਜੇ( !pUnpack ){
      Mem *p_ਅੰਕੜੇ = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_ਅੰਕੜੇ);
      ਜੇ( rc!=SQLITE_OK ) ਜਾਓ preupdate_new_ਬਾਹਰਲਾ;
      pUnpack = vdbe_Unpack_ਰਿਕਾਰਡ(p->pKeyinfo, p_ਅੰਕੜੇ->n, p_ਅੰਕੜੇ->z);
      ਜੇ( !pUnpack ){
        rc = SQLITE_NOMEM;
        ਜਾਓ preupdate_new_ਬਾਹਰਲਾ;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    ਜੇ( iIdx==p->pTab->iPKey ){
      sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem, p->iKey2);
    }ਨਹੀਂਤਾਂ ਜੇ( iStore>=pUnpack->n_ਖੇਤਰ ){
      pMem = (sqlite3_ਮੁੱਲ *)ਕਾਲਮ_Null_ਮੁੱਲ();
    }ਨਹੀਂਤਾਂ ਜੇ( (p->pTab->a_ਖਾਨਾ)
           && (p->pTab->a_ਖਾਨਾ[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }ਨਹੀਂਤਾਂ{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->op==SQLITE_UPDATE );
    ਜੇ( !p->aNew ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(Mem)*UMXV(p->pCsr->n_ਖੇਤਰ) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, ਆਕਾਰ(Mem)*p->pCsr->n_ਖੇਤਰ);
      ਜੇ( !p->aNew ){
        rc = SQLITE_NOMEM;
        ਜਾਓ preupdate_new_ਬਾਹਰਲਾ;
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( iStore>=0 && iStore<p->pCsr->n_ਖੇਤਰ );
    pMem = &p->aNew[iStore];
    ਜੇ( pMem->flags==0 ){
      ਜੇ( iIdx==p->pTab->iPKey ){
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem, p->iKey2);
      }ਨਹੀਂਤਾਂ{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        ਜੇ( rc!=SQLITE_OK ) ਜਾਓ preupdate_new_ਬਾਹਰਲਾ;
      }
    }
  }
  *pp_ਮੁੱਲ = pMem;

 preupdate_new_ਬਾਹਰਲਾ:
  sqlite3Error(db, rc);
  ਵਾਪਸ sqlite3ApiExit(db, rc);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
ਪੂਰਨ sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ਪੂਰਨ i_ਜਾਂਚ,                      /* Index of loop to report on */
  ਪੂਰਨ i_ਜਾਂਚ_ਸਥਿਤੀ_Op,              /* Which metric to return */
  ਪੂਰਨ flags,
  ਖਾਲੀ *p_ਬਾਹਰਲਾ                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  ਪੂਰਨ nOp;
  ਜਾਂਚ_ਸਥਿਤੀ *p_ਜਾਂਚ = 0;
  ਪੂਰਨ idx;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 || p_ਬਾਹਰਲਾ==0
      || i_ਜਾਂਚ_ਸਥਿਤੀ_Op<SQLITE_SCANSTAT_NLOOP
      || i_ਜਾਂਚ_ਸਥਿਤੀ_Op>SQLITE_SCANSTAT_NCYCLE ){
    ਵਾਪਸ 1;
  }
#ਜੇ_ਅੰਤ
  aOp = p->aOp;
  nOp = p->nOp;
  ਜੇ( p->p_ਚੌਖਟਾ ){
    Vdbe_ਚੌਖਟਾ *p_ਚੌਖਟਾ;
    ਲਈ(p_ਚੌਖਟਾ=p->p_ਚੌਖਟਾ; p_ਚੌਖਟਾ->p_ਮਾਪਾ; p_ਚੌਖਟਾ=p_ਚੌਖਟਾ->p_ਮਾਪਾ);
    aOp = p_ਚੌਖਟਾ->aOp;
    nOp = p_ਚੌਖਟਾ->nOp;
  }

  ਜੇ( i_ਜਾਂਚ<0 ){
    ਪੂਰਨ ii;
    ਜੇ( i_ਜਾਂਚ_ਸਥਿਤੀ_Op==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      ਲਈ(ii=0; ii<nOp; ii++){
        res += aOp[ii].n_ਚੱਕਰ;
      }
      *(i64*)p_ਬਾਹਰਲਾ = res;
      ਵਾਪਸ 0;
    }
    ਵਾਪਸ 1;
  }
  ਜੇ( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = i_ਜਾਂਚ;
  }ਨਹੀਂਤਾਂ{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    ਲਈ(idx=0; idx<p->n_ਜਾਂਚ; idx++){
      p_ਜਾਂਚ = &p->a_ਜਾਂਚ[idx];
      ਜੇ( p_ਜਾਂਚ->zName ){
        i_ਜਾਂਚ--;
        ਜੇ( i_ਜਾਂਚ<0 ) ਰੋਕੋ;
      }
    }
  }
  ਜੇ( idx>=p->n_ਜਾਂਚ ) ਵਾਪਸ 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਜਾਂਚ==0 || p_ਜਾਂਚ==&p->a_ਜਾਂਚ[idx] );
  p_ਜਾਂਚ = &p->a_ਜਾਂਚ[idx];

  ਚੋਣ( i_ਜਾਂਚ_ਸਥਿਤੀ_Op ){
    ਹਾਲਤ SQLITE_SCANSTAT_NLOOP: {
      ਜੇ( p_ਜਾਂਚ->addrLoop>0 ){
        *(sqlite3_int64*)p_ਬਾਹਰਲਾ = aOp[p_ਜਾਂਚ->addrLoop].nExec;
      }ਨਹੀਂਤਾਂ{
        *(sqlite3_int64*)p_ਬਾਹਰਲਾ = -1;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_NVISIT: {
      ਜੇ( p_ਜਾਂਚ->addr_ਦੇਖੋ>0 ){
        *(sqlite3_int64*)p_ਬਾਹਰਲਾ = aOp[p_ਜਾਂਚ->addr_ਦੇਖੋ].nExec;
      }ਨਹੀਂਤਾਂ{
        *(sqlite3_int64*)p_ਬਾਹਰਲਾ = -1;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_EST: {
      ਦੁੱਗਣਾ r = 1.0;
      LogEst x = p_ਜਾਂਚ->nEst;
      ਜਦੋਂ( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(ਦੁੱਗਣਾ*)p_ਬਾਹਰਲਾ = r*sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(x);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_NAME: {
      *(ਅਚਲ ਅੱਖਰ**)p_ਬਾਹਰਲਾ = p_ਜਾਂਚ->zName;
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_EXPLAIN: {
      ਜੇ( p_ਜਾਂਚ->addrExplain ){
        *(ਅਚਲ ਅੱਖਰ**)p_ਬਾਹਰਲਾ = aOp[ p_ਜਾਂਚ->addrExplain ].p4.z;
      }ਨਹੀਂਤਾਂ{
        *(ਅਚਲ ਅੱਖਰ**)p_ਬਾਹਰਲਾ = 0;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_SELECTID: {
      ਜੇ( p_ਜਾਂਚ->addrExplain ){
        *(ਪੂਰਨ*)p_ਬਾਹਰਲਾ = aOp[ p_ਜਾਂਚ->addrExplain ].p1;
      }ਨਹੀਂਤਾਂ{
        *(ਪੂਰਨ*)p_ਬਾਹਰਲਾ = -1;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_PARENTID: {
      ਜੇ( p_ਜਾਂਚ->addrExplain ){
        *(ਪੂਰਨ*)p_ਬਾਹਰਲਾ = aOp[ p_ਜਾਂਚ->addrExplain ].p2;
      }ਨਹੀਂਤਾਂ{
        *(ਪੂਰਨ*)p_ਬਾਹਰਲਾ = -1;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      ਜੇ( p_ਜਾਂਚ->aAddrRange[0]==0 ){
        res = -1;
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ ii;
        ਲਈ(ii=0; ii<Array_ਮਾਪ(p_ਜਾਂਚ->aAddrRange); ii+=2){
          ਪੂਰਨ iIns = p_ਜਾਂਚ->aAddrRange[ii];
          ਪੂਰਨ i_ਸਿਰਾ = p_ਜਾਂਚ->aAddrRange[ii+1];
          ਜੇ( iIns==0 ) ਰੋਕੋ;
          ਜੇ( iIns>0 ){
            ਜਦੋਂ( iIns<=i_ਸਿਰਾ ){
              res += aOp[iIns].n_ਚੱਕਰ;
              iIns++;
            }
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ iOp;
            ਲਈ(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              ਜੇ( pOp->p1!=i_ਸਿਰਾ ) ਜਾਰੀ;
              ਜੇ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                ਜਾਰੀ;
              }
              res += aOp[iOp].n_ਚੱਕਰ;
            }
          }
        }
      }
      *(i64*)p_ਬਾਹਰਲਾ = res;
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
ਪੂਰਨ sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ਪੂਰਨ i_ਜਾਂਚ,                      /* Index of loop to report on */
  ਪੂਰਨ i_ਜਾਂਚ_ਸਥਿਤੀ_Op,              /* Which metric to return */
  ਖਾਲੀ *p_ਬਾਹਰਲਾ                      /* OUT: Write the answer here */
){
  ਵਾਪਸ sqlite3_stmt_scanstatus_v2(pStmt, i_ਜਾਂਚ, i_ਜਾਂਚ_ਸਥਿਤੀ_Op, 0, p_ਬਾਹਰਲਾ);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
ਖਾਲੀ sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ਪੂਰਨ ii;
  ਲਈ(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->n_ਚੱਕਰ = 0;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STMT_SCANSTATUS */
