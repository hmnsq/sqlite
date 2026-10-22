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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "vdbeInt.h"
#ଅନ୍ତର୍ଭାଵ "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_expired(sqlite3_stmt *pStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRet = 1;
  ଯଦି( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  ଫେରନ୍ତୁ iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vdbeSafety(Vdbe *p){
  ଯଦି( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
  }
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vdbeSafetyNotNull(Vdbe *p){
  ଯଦି( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  ପ୍ରତିଜ୍ଞା( p->startTime>0 );
  ପ୍ରତିଜ୍ଞା( db->init.busy==0 );
  ପ୍ରତିଜ୍ଞା( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  ଯଦି( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  ଯଦି( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (ଶୂନ୍ୟ*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   ଯଦି( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#ଅନ୍ୟଥା
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_finalize(sqlite3_stmt *pStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    ଯଦି( vdbeSafety(v) ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    ପ୍ରତିଜ୍ଞା( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_reset(sqlite3_stmt *pStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( pStmt==0 ){
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    ପ୍ରତିଜ୍ଞା( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#ଯଦି SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pStmt==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif
#ଯଦି SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  ପାଇଁ(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    ଯଦି( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  ପ୍ରତିଜ୍ଞା( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ଯଦି( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  ଫେରନ୍ତୁ rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ଅଚଳ ଶୂନ୍ୟ *sqlite3_ମୂଲ୍ୟ_blob(sqlite3_ମୂଲ୍ୟ *pVal){
  Mem *p = (Mem*)pVal;
  ଯଦି( p->flags & (MEM_Blob|MEM_Str) ){
    ଯଦି( ExpandBlob(p)!=SQLITE_OK ){
      ପ୍ରତିଜ୍ଞା( p->flags==MEM_Null && p->z==0 );
      ଫେରନ୍ତୁ 0;
    }
    p->flags |= MEM_Blob;
    ଫେରନ୍ତୁ p->n ? p->z : 0;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_text(pVal);
  }
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_bytes(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_Bytes(pVal, SQLITE_UTF8);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_bytes16(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_Bytes(pVal, SQLITE_UTF16NATIVE);
}
ଦ୍ୱିଗୁଣ sqlite3_ମୂଲ୍ୟ_double(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ sqlite3_Vdbe_Real_ମୂଲ୍ୟ((Mem*)pVal);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_int(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sqlite3_Vdbe_Int_ମୂଲ୍ୟ((Mem*)pVal);
}
sqlite_int64 sqlite3_ମୂଲ୍ୟ_int64(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ sqlite3_Vdbe_Int_ମୂଲ୍ୟ((Mem*)pVal);
}
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_subtype(sqlite3_ମୂଲ୍ୟ *pVal){
  Mem *pMem = (Mem*)pVal;
  ଫେରନ୍ତୁ ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
ଶୂନ୍ୟ *sqlite3_ମୂଲ୍ୟ_pointer(sqlite3_ମୂଲ୍ୟ *pVal, ଅଚଳ ଅକ୍ଷର *zPType){
  Mem *p = (Mem*)pVal;
  ଯଦି( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)p->z;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
  }
}
ଅଚଳ unsigned ଅକ୍ଷର *sqlite3_ମୂଲ୍ୟ_text(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ (ଅଚଳ unsigned ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_ମୂଲ୍ୟ_text16(sqlite3_ମୂଲ୍ୟ* pVal){
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_Text(pVal, SQLITE_UTF16NATIVE);
}
ଅଚଳ ଶୂନ୍ୟ *sqlite3_ମୂଲ୍ୟ_text16be(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_Text(pVal, SQLITE_UTF16BE);
}
ଅଚଳ ଶୂନ୍ୟ *sqlite3_ମୂଲ୍ୟ_text16le(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_type(sqlite3_ମୂଲ୍ୟ* pVal){
  ସ୍ଥିର ଅଚଳ u8 aType[] = {
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = SQLITE_BLOB;
    ଯଦି( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }ଅନ୍ୟଥା ଯଦି( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }ଅନ୍ୟଥା ଯଦି( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }ଅନ୍ୟଥା ଯଦି( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    ପ୍ରତିଜ୍ଞା( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  ଫେରନ୍ତୁ aType[pVal->flags&MEM_AffMask];
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_encoding(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_nochange(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ମୂଲ୍ୟ_frombind(sqlite3_ମୂଲ୍ୟ *pVal){
  ଫେରନ୍ତୁ (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_ମୂଲ୍ୟ *sqlite3_ମୂଲ୍ୟ_dup(ଅଚଳ sqlite3_ମୂଲ୍ୟ *pOrig){
  sqlite3_ମୂଲ୍ୟ *pNew;
  ଯଦି( pOrig==0 ) ଫେରନ୍ତୁ 0;
  pNew = sqlite3_malloc( ଆକାର(*pNew) );
  ଯଦି( pNew==0 ) ଫେରନ୍ତୁ 0;
  memset(pNew, 0, ଆକାର(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  ଯଦି( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    ଯଦି( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_ମୂଲ୍ୟ_Free(pNew);
      pNew = 0;
    }
  }ଅନ୍ୟଥା ଯଦି( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  ଫେରନ୍ତୁ pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
ଶୂନ୍ୟ sqlite3_ମୂଲ୍ୟ_free(sqlite3_ମୂଲ୍ୟ *pOld){
  sqlite3_ମୂଲ୍ୟ_Free(pOld);
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
ସ୍ଥିର ଶୂନ୍ୟ setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  ଅଚଳ ଅକ୍ଷର *z,          /* String pointer */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }ଅନ୍ୟଥା ଯଦି( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    ପ୍ରତିଜ୍ଞା( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }ଅନ୍ୟଥା{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  ଯଦି( rc ){
    ଯଦି( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }ଅନ୍ୟଥା{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      ପ୍ରତିଜ୍ଞା( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    ଫେରନ୍ତୁ;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  ଯଦି( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା invoke_ମୂଲ୍ୟ_Destructor(
  ଅଚଳ ଶୂନ୍ୟ *p,             /* Value to destroy */
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  ପ୍ରତିଜ୍ଞା( xDel!=SQLITE_DYNAMIC );
  ଯଦି( xDel==0 ){
    /* noop */
  }ଅନ୍ୟଥା ଯଦି( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }ଅନ୍ୟଥା{
    xDel((ଶୂନ୍ୟ*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  ଫେରନ୍ତୁ SQLITE_TOOBIG;
}
ଶୂନ୍ୟ sqlite3_result_blob(
  sqlite3_context *pCtx,
  ଅଚଳ ଶୂନ୍ୟ *z,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 || n<0 ){
    invoke_ମୂଲ୍ୟ_Destructor(z, xDel, pCtx);
    ଫେରନ୍ତୁ;
  }
#endif
  ପ୍ରତିଜ୍ଞା( n>=0 );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
ଶୂନ୍ୟ sqlite3_result_blob64(
  sqlite3_context *pCtx,
  ଅଚଳ ଶୂନ୍ୟ *z,
  sqlite3_uint64 n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *)
){
  ପ୍ରତିଜ୍ଞା( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ){
    invoke_ମୂଲ୍ୟ_Destructor(z, xDel, 0);
    ଫେରନ୍ତୁ;
  }
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  ଯଦି( n>0x7fffffff ){
    (ଶୂନ୍ୟ)invoke_ମୂଲ୍ୟ_Destructor(z, xDel, pCtx);
  }ଅନ୍ୟଥା{
    setResultStrOrError(pCtx, z, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n, 0, xDel);
  }
}
ଶୂନ୍ୟ sqlite3_result_double(sqlite3_context *pCtx, ଦ୍ୱିଗୁଣ rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
ଶୂନ୍ୟ sqlite3_result_error(sqlite3_context *pCtx, ଅଚଳ ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
ଶୂନ୍ୟ sqlite3_result_error16(sqlite3_context *pCtx, ଅଚଳ ଶୂନ୍ୟ *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
ଶୂନ୍ୟ sqlite3_result_int(sqlite3_context *pCtx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
ଶୂନ୍ୟ sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
ଶୂନ୍ୟ sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
ଶୂନ୍ୟ sqlite3_result_pointer(
  sqlite3_context *pCtx,
  ଶୂନ୍ୟ *pPtr,
  ଅଚଳ ଅକ୍ଷର *zPType,
  ଶୂନ୍ୟ (*xDestructor)(ଶୂନ୍ୟ*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ){
    invoke_ମୂଲ୍ୟ_Destructor(pPtr, xDestructor, 0);
    ଫେରନ୍ତୁ;
  }
#endif
  pOut = pCtx->pOut;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
ଶୂନ୍ୟ sqlite3_result_subtype(sqlite3_context *pCtx, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
#ଯଦି defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  ଯଦି( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    ଅକ୍ଷର zErr[200];
    sqlite3_snprintf(ଆକାର(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    ଫେରନ୍ତୁ;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
ଶୂନ୍ୟ sqlite3_result_text(
  sqlite3_context *pCtx,
  ଅଚଳ ଅକ୍ଷର *z,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ){
    invoke_ମୂଲ୍ୟ_Destructor(z, xDel, 0);
    ଫେରନ୍ତୁ;
  }
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
ଶୂନ୍ୟ sqlite3_result_text64(
  sqlite3_context *pCtx,
  ଅଚଳ ଅକ୍ଷର *z,
  sqlite3_uint64 n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *),
  unsigned ଅକ୍ଷର enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ){
    invoke_ମୂଲ୍ୟ_Destructor(z, xDel, 0);
    ଫେରନ୍ତୁ;
  }
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  ପ୍ରତିଜ୍ଞା( xDel!=SQLITE_DYNAMIC );
  ଯଦି( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ଯଦି( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  ଯଦି( n>0x7fffffff ){
    (ଶୂନ୍ୟ)invoke_ମୂଲ୍ୟ_Destructor(z, xDel, pCtx);
  }ଅନ୍ୟଥା{
    setResultStrOrError(pCtx, z, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
ଶୂନ୍ୟ sqlite3_result_text16(
  sqlite3_context *pCtx,
  ଅଚଳ ଶୂନ୍ୟ *z,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *)
){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
ଶୂନ୍ୟ sqlite3_result_text16be(
  sqlite3_context *pCtx,
  ଅଚଳ ଶୂନ୍ୟ *z,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *)
){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
ଶୂନ୍ୟ sqlite3_result_text16le(
  sqlite3_context *pCtx,
  ଅଚଳ ଶୂନ୍ୟ *z,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ *)
){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
ଶୂନ୍ୟ sqlite3_result_ମୂଲ୍ୟ(sqlite3_context *pCtx, sqlite3_ମୂଲ୍ୟ *p_ମୂଲ୍ୟ){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
  ଯଦି( p_ମୂଲ୍ୟ==0 ){
    sqlite3_result_null(pCtx);
    ଫେରନ୍ତୁ;
  }
#endif
  pOut = pCtx->pOut;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_ମୂଲ୍ୟ);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  ଯଦି( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
ଶୂନ୍ୟ sqlite3_result_zeroblob(sqlite3_context *pCtx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pOut->db->mutex) );
  ଯଦି( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    ଫେରନ୍ତୁ SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n);
  ଫେରନ୍ତୁ SQLITE_OK;
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n);
#endif
}
ଶୂନ୍ୟ sqlite3_result_error_code(sqlite3_context *pCtx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  ଯଦି( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  ଯଦି( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
ଶୂନ୍ୟ sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
ଶୂନ୍ୟ sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
ଶୂନ୍ୟ sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
  ଯଦି( pStr==0 ) ଫେରନ୍ତୁ;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  ଯଦି( pStr->accError==0 ){
    ଯଦି( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }ଅନ୍ୟଥା{
      ଅଚଳ ଅକ୍ଷର *zText = sqlite3_str_ମୂଲ୍ୟ(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      ପ୍ରତିଜ୍ଞା( isMalloced(pStr) );
      ଯଦି( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }ଅନ୍ୟଥା{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }ଅନ୍ୟଥା ଯଦି( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  ଯଦି( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
ଶୂନ୍ୟ sqlite3ResultIntReal(sqlite3_context *pCtx){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  ଯଦି( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା doWalCallbacks(sqlite3 *db){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ଯଦି( pBt ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      ଯଦି( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#ଅନ୍ୟଥା
  UNUSED_PARAMETER(db);
#endif
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3Step(Vdbe *p){
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ପ୍ରତିଜ୍ଞା(p);
  db = p->db;
  ଯଦି( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    ଯଦି( p->eVdbeState==VDBE_READY_STATE ){
      ଯଦି( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        ଯଦି( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        ଯାଅ end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      ଯଦି( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      ପ୍ରତିଜ୍ଞା( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      ଯଦି( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      ଯଦି( p->readOnly==0 ) db->nVdbeWrite++;
      ଯଦି( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }ଅନ୍ୟଥା

    ଯଦି( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      ଯଦି( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
      }
#ଅନ୍ୟଥା
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      ପ୍ରତିଜ୍ଞା( p->eVdbeState==VDBE_READY_STATE );
      ଯାଅ restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  ଯଦି( p->explain ){
    rc = sqlite3VdbeList(p);
  }ଅନ୍ୟଥା
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  ଯଦି( rc==SQLITE_ROW ){
    ପ୍ରତିଜ୍ଞା( p->rc==SQLITE_OK );
    ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    ଫେରନ୍ତୁ SQLITE_ROW;
  }ଅନ୍ୟଥା{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    ଯଦି( rc==SQLITE_DONE && db->autoCommit ){
      ପ୍ରତିଜ୍ଞା( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      ଯଦି( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }ଅନ୍ୟଥା ଯଦି( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  ଯଦି( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    ଯଦି( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  ପ୍ରତିଜ୍ଞା( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  ଫେରନ୍ତୁ (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_step(sqlite3_stmt *pStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  ଯଦି( vdbeSafetyNotNull(v) ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  ଯେତେବେଳେ( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    ଯଦି( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ଅଚଳ ଅକ୍ଷର *zErr = (ଅଚଳ ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_ସନ୍ଦେଶ);
      ଯଦି( !db->mallocFailed ){
        v->z_Err_ସନ୍ଦେଶ = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } ଅନ୍ୟଥା {
        v->z_Err_ସନ୍ଦେଶ = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      ଭାଙ୍ଗ;
    }
    sqlite3_reset(pStmt);
    ଯଦି( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    ପ୍ରତିଜ୍ଞା( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
ଶୂନ୍ୟ *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
#endif
  ପ୍ରତିଜ୍ଞା( p && p->pFunc );
  ଫେରନ୍ତୁ p->pFunc->pUserData;
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
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( p && p->pOut );
#endif
  ଫେରନ୍ତୁ p->pOut->db;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( p );
#endif
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
ଶୂନ୍ୟ sqlite3_Vdbe_ମୂଲ୍ୟ_List_Free(ଶୂନ୍ୟ *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ_From_ମୂଲ୍ୟ_List(
  sqlite3_ମୂଲ୍ୟ *pVal,        /* Pointer to the ValueList object */
  sqlite3_ମୂଲ୍ୟ **ppOut,      /* Store the next value from the list here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bNext                   /* 1 for _next(). 0 for _first() */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ମୂଲ୍ୟ_List *pRhs;

  *ppOut = 0;
  ଯଦି( pVal==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  ଯଦି( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_ମୂଲ୍ୟ_List_Free ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    ପ୍ରତିଜ୍ଞା( pVal->eSubtype=='p' );
    ପ୍ରତିଜ୍ଞା( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (ମୂଲ୍ୟ_List*)pVal->z;
  }
  ଯଦି( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    ଯଦି( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  ଯଦି( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, ଆକାର(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    ଯଦି( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_ମୂଲ୍ୟ *pOut = pRhs->pOut;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      ଯଦି( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }ଅନ୍ୟଥା{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vtab_in_first(sqlite3_ମୂଲ୍ୟ *pVal, sqlite3_ମୂଲ୍ୟ **ppOut){
  ଫେରନ୍ତୁ ମୂଲ୍ୟ_From_ମୂଲ୍ୟ_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vtab_in_next(sqlite3_ମୂଲ୍ୟ *pVal, sqlite3_ମୂଲ୍ୟ **ppOut){
  ଫେରନ୍ତୁ ମୂଲ୍ୟ_From_ମୂଲ୍ୟ_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  ପ୍ରତିଜ୍ଞା( p->pVdbe!=0 );
#ଅନ୍ୟଥା
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  ଯଦି( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    ଯଦି( rc ) *piTime = 0;
  }
  ଫେରନ୍ତୁ *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ *createAggContext(sqlite3_context *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  Mem *pMem = p->pMem;
  ପ୍ରତିଜ୍ଞା( (pMem->flags & MEM_Agg)==0 );
  ଯଦି( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }ଅନ୍ୟଥା{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    ଯଦି( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
ଶୂନ୍ୟ *sqlite3_aggregate_context(sqlite3_context *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  ପ୍ରତିଜ୍ଞା( p && p->pFunc && p->pFunc->xFinalize );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  ଯଦି( (p->pMem->flags & MEM_Agg)==0 ){
    ଫେରନ୍ତୁ createAggContext(p, nByte);
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)p->pMem->z;
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
ଶୂନ୍ୟ *sqlite3_get_auxdata(sqlite3_context *pCtx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ 0;
#endif
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ଯଦି SQLITE_ENABLE_STAT4
  ଯଦି( pCtx->pVdbe==0 ) ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( pCtx->pVdbe!=0 );
#endif
  ପାଇଁ(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    ଯଦି(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      ଫେରନ୍ତୁ pAuxData->pAux;
    }
  }
  ଫେରନ୍ତୁ 0;
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
ଶୂନ୍ୟ sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArg,
  ଶୂନ୍ୟ *pAux,
  ଶୂନ୍ୟ (*xDelete)(ଶୂନ୍ୟ*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCtx==0 ) ଫେରନ୍ତୁ;
#endif
  pVdbe= pCtx->pVdbe;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  ଯଦି( pVdbe==0 ) ଯାଅ failed;
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( pVdbe!=0 );
#endif

  ପାଇଁ(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    ଯଦି( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, ଆକାର(AuxData));
    ଯଦି( !pAuxData ) ଯାଅ failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    ଯଦି( pCtx->isError==0 ) pCtx->isError = -1;
  }ଅନ୍ୟଥା ଯଦି( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  ଫେରନ୍ତୁ;

failed:
  ଯଦି( xDelete ){
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_aggregate_ଗଣନା(sqlite3_context *p){
  ପ୍ରତିଜ୍ଞା( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  ଫେରନ୍ତୁ p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_column_ଗଣନା(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ଯଦି( pVm==0 ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_data_ଗଣନା(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ଯଦି( pVm==0 || pVm->pResultRow==0 ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ସ୍ଥିର ଅଚଳ Mem *column_Null_ମୂଲ୍ୟ(ଶୂନ୍ୟ){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ସ୍ଥିର ଅଚଳ Mem nullMem
#ଯଦି defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (ଅକ୍ଷର*)0,
        /* .n          = */ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (ଅକ୍ଷର*)0,
        /* .xDel       = */ (ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  ଫେରନ୍ତୁ &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ସ୍ଥିର Mem *columnMem(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  ଯଦି( pVm==0 ) ଫେରନ୍ତୁ (Mem*)column_Null_ମୂଲ୍ୟ();
  ପ୍ରତିଜ୍ଞା( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  ଯଦି( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }ଅନ୍ୟଥା{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_ମୂଲ୍ୟ();
  }
  ଫେରନ୍ତୁ pOut;
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
ସ୍ଥିର ଶୂନ୍ୟ columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  ଯଦି( p ){
    ପ୍ରତିଜ୍ଞା( p->db!=0 );
    ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_blob(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ଅଚଳ ଶୂନ୍ୟ *val;
  val = sqlite3_ମୂଲ୍ୟ_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_column_bytes(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = sqlite3_ମୂଲ୍ୟ_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_column_bytes16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = sqlite3_ମୂଲ୍ୟ_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
ଦ୍ୱିଗୁଣ sqlite3_column_double(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ଦ୍ୱିଗୁଣ val = sqlite3_ମୂଲ୍ୟ_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_column_int(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = sqlite3_ମୂଲ୍ୟ_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  sqlite_int64 val = sqlite3_ମୂଲ୍ୟ_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
ଅଚଳ unsigned ଅକ୍ଷର *sqlite3_column_text(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ଅଚଳ unsigned ଅକ୍ଷର *val = sqlite3_ମୂଲ୍ୟ_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
sqlite3_ମୂଲ୍ୟ *sqlite3_column_ମୂଲ୍ୟ(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  Mem *pOut = columnMem(pStmt, i);
  ଯଦି( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ (sqlite3_ମୂଲ୍ୟ *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_text16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ଅଚଳ ଶୂନ୍ୟ *val = sqlite3_ମୂଲ୍ୟ_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ val;
}
#endif /* SQLITE_OMIT_UTF16 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_column_type(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iType = sqlite3_ମୂଲ୍ୟ_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ଫେରନ୍ତୁ iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର * ଅଚଳ azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ସ୍ଥିର ଅଚଳ u16 azExplainColNames16data[] = {
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
ସ୍ଥିର ଅଚଳ u8 iExplainColNames16[] = {
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
ସ୍ଥିର ଅଚଳ ଶୂନ୍ୟ *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା N,                   /* Which column to get the name for */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା useUtf16,            /* True to return the name as UTF16 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା useType              /* What type of name */
){
  ଅଚଳ ଶୂନ୍ୟ *ret;
  Vdbe *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pStmt==0 ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଯଦି( N<0 ) ଫେରନ୍ତୁ 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  ଯଦି( p->explain ){
    ଯଦି( useType>0 ) ଯାଅ columnName_end;
    n = p->explain==1 ? 8 : 4;
    ଯଦି( N>=n ) ଯାଅ columnName_end;
    ଯଦି( useUtf16 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (ଶୂନ୍ୟ*)&azExplainColNames16data[i];
    }ଅନ୍ୟଥା{
      ret = (ଶୂନ୍ୟ*)azExplainColNames8[N + 8*p->explain - 8];
    }
    ଯାଅ columnName_end;
  }
  n = p->nResColumn;
  ଯଦି( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    ଯଦି( useUtf16 ){
      ret = sqlite3_ମୂଲ୍ୟ_text16((sqlite3_ମୂଲ୍ୟ*)&p->aColName[N]);
    }ଅନ୍ୟଥା
#endif
    {
      ret = sqlite3_ମୂଲ୍ୟ_text((sqlite3_ମୂଲ୍ୟ*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 || db->mallocFailed==1 );
    ଯଦି( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_column_name(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_name16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#ଯଦି defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_column_decltype(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_decltype16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_column_database_name(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_database_name16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_column_table_name(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_table_name16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_column_origin_name(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
ଅଚଳ ଶୂନ୍ୟ *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଫେରନ୍ତୁ columnName(pStmt, N, 1, COLNAME_COLUMN);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vdbeUnbind(Vdbe *p, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  Mem *pVar;
  ଯଦି( vdbeSafetyNotNull(p) ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ଯଦି( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
  ଯଦି( i>=(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ଫେରନ୍ତୁ SQLITE_RANGE;
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
  ପ୍ରତିଜ୍ଞା( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ପ୍ରତିଜ୍ଞା( (p->expmask & p->smimask)==p->smimask );
  ଯଦି( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    ଯଦି( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,                 /* Index of the parameter to bind */
  ଅଚଳ ଶୂନ୍ୟ *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  ଯଦି( rc==SQLITE_OK ){
    ପ୍ରତିଜ୍ଞା( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    ଯଦି( zData!=0 ){
      pVar = &p->aVar[i-1];
      ଯଦି( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }ଅନ୍ୟଥା ଯଦି( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        ପ୍ରତିଜ୍ଞା( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }ଅନ୍ୟଥା{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        ଯଦି( encoding==0 ) pVar->enc = ENC(p->db);
      }
      ଯଦି( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      ଯଦି( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }ଅନ୍ୟଥା ଯଦି( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((ଶୂନ୍ୟ*)zData);
  }
  ଫେରନ୍ତୁ rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,
  ଅଚଳ ଶୂନ୍ୟ *zData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( nData<0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  ଫେରନ୍ତୁ bindText(pStmt, i, zData, nData, xDel, 0);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,
  ଅଚଳ ଶୂନ୍ୟ *zData,
  sqlite3_uint64 nData,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*)
){
  ପ୍ରତିଜ୍ଞା( xDel!=SQLITE_DYNAMIC );
  ଫେରନ୍ତୁ bindText(pStmt, i, zData, nData, xDel, 0);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_double(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଦ୍ୱିଗୁଣ r_ମୂଲ୍ୟ){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ଯଦି( rc==SQLITE_OK ){
    ପ୍ରତିଜ୍ଞା( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_ମୂଲ୍ୟ);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ଫେରନ୍ତୁ rc;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_int(sqlite3_stmt *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ମୂଲ୍ୟ){
  ଫେରନ୍ତୁ sqlite3_bind_int64(p, i, (i64)i_ମୂଲ୍ୟ);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_int64(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, sqlite_int64 i_ମୂଲ୍ୟ){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  ଯଦି( vdbeSafetyNotNull(p) ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ଯଦି( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
  ଯଦି( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ଫେରନ୍ତୁ SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  ଯଦି( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    ପ୍ରତିଜ୍ଞା( (p->expmask & p->smimask)==p->smimask );
    ଯଦି( (p->expmask & expireMask)!=0 ){
      ଯଦି( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା sm2 = i_ମୂଲ୍ୟ>=0 && i_ମୂଲ୍ୟ<=1;
        ଯଦି( sm1!=sm2 ){
          p->expired = 1;
        }
      }ଅନ୍ୟଥା ଯଦି( (pVar->flags & MEM_Int)==0 || i_ମୂଲ୍ୟ!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_ମୂଲ୍ୟ);
  sqlite3_mutex_leave(p->db->mutex);
  ଫେରନ୍ତୁ SQLITE_OK;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_null(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ଯଦି( rc==SQLITE_OK ){
    ପ୍ରତିଜ୍ଞା( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  ଫେରନ୍ତୁ rc;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,
  ଶୂନ୍ୟ *pPtr,
  ଅଚଳ ଅକ୍ଷର *zPTtype,
  ଶୂନ୍ୟ (*xDestructor)(ଶୂନ୍ୟ*)
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ଯଦି( rc==SQLITE_OK ){
    ପ୍ରତିଜ୍ଞା( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }ଅନ୍ୟଥା ଯଦି( xDestructor ){
    xDestructor(pPtr);
  }
  ଫେରନ୍ତୁ rc;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,
  ଅଚଳ ଅକ୍ଷର *zData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*)
){
  ଫେରନ୍ତୁ bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,
  ଅଚଳ ଅକ୍ଷର *zData,
  sqlite3_uint64 nData,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*),
  unsigned ଅକ୍ଷର enc
){
  ପ୍ରତିଜ୍ଞା( xDel!=SQLITE_DYNAMIC );
  ଯଦି( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ଯଦି( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  ଫେରନ୍ତୁ bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,
  ଅଚଳ ଶୂନ୍ୟ *zData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*)
){
  ଫେରନ୍ତୁ bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_ମୂଲ୍ୟ(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଅଚଳ sqlite3_ମୂଲ୍ୟ *p_ମୂଲ୍ୟ){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ବାଛ( sqlite3_ମୂଲ୍ୟ_type((sqlite3_ମୂଲ୍ୟ*)p_ମୂଲ୍ୟ) ){
    ସ୍ଥିତି SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_ମୂଲ୍ୟ->u.i);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_FLOAT: {
      ପ୍ରତିଜ୍ଞା( p_ମୂଲ୍ୟ->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_ମୂଲ୍ୟ->flags & MEM_Real) ? p_ମୂଲ୍ୟ->u.r : (ଦ୍ୱିଗୁଣ)p_ମୂଲ୍ୟ->u.i
      );
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_BLOB: {
      ଯଦି( p_ମୂଲ୍ୟ->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_ମୂଲ୍ୟ->u.nZero);
      }ଅନ୍ୟଥା{
        rc = sqlite3_bind_blob(pStmt, i, p_ମୂଲ୍ୟ->z, p_ମୂଲ୍ୟ->n,SQLITE_TRANSIENT);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_ମୂଲ୍ୟ->z, p_ମୂଲ୍ୟ->n, SQLITE_TRANSIENT,
                              p_ମୂଲ୍ୟ->enc);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      rc = sqlite3_bind_null(pStmt, i);
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ rc;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ଯଦି( rc==SQLITE_OK ){
    ପ୍ରତିଜ୍ଞା( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#ଅନ୍ୟଥା
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  ଫେରନ୍ତୁ rc;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, sqlite3_uint64 n){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  ଯଦି( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  ଫେରନ୍ତୁ rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_parameter_ଗଣନା(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ଫେରନ୍ତୁ p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  Vdbe *p = (Vdbe*)pStmt;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ sqlite3_VList_ଅଙ୍କ_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VdbeParameterIndex(Vdbe *p, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName){
  ଯଦି( p==0 || zName==0 ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ sqlite3_VList_Name_To_ଅଙ୍କ(p->pVList, zName, nName);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, ଅଚଳ ଅକ୍ଷର *zName){
  ଫେରନ୍ତୁ sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( pTo->db==pFrom->db );
  ପ୍ରତିଜ୍ଞା( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  ପାଇଁ(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  ଫେରନ୍ତୁ SQLITE_OK;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  ଯଦି( pFrom->nVar!=pTo->nVar ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  ପ୍ରତିଜ୍ଞା( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  ଯଦି( pTo->expmask ){
    pTo->expired = 1;
  }
  ପ୍ରତିଜ୍ଞା( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  ଯଦି( pFrom->expmask ){
    pFrom->expired = 1;
  }
  ଫେରନ୍ତୁ sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  ଫେରନ୍ତୁ pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  ଫେରନ୍ତୁ pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  ଫେରନ୍ତୁ pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_explain(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eMode){
  Vdbe *v = (Vdbe*)pStmt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pStmt==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  ଯଦି( ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)v->explain)==eMode ){
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା ଯଦି( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା ଯଦି( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା ଯଦି( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }ଅନ୍ୟଥା ଯଦି( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    ଯଦି( rc ){
      v->explain = savedMode;
    }ଅନ୍ୟଥା{
      v->haveEqpOps = eMode==2;
    }
  }
  ଯଦି( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }ଅନ୍ୟଥା{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  ଫେରନ୍ତୁ rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  ଫେରନ୍ତୁ v!=0 && v->eVdbeState==VDBE_RUN_STATE;
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
  ଯଦି( !sqlite3SafetyCheckOk(pDb) ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  ଯଦି( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }ଅନ୍ୟଥା{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  ଫେରନ୍ତୁ pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_status(sqlite3_stmt *pStmt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଯଦି( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)&v;
    ପ୍ରତିଜ୍ଞା( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }ଅନ୍ୟଥା{
    v = pVdbe->aCounter[op];
    ଯଦି( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ଫେରନ୍ତୁ p ? p->zSql : 0;
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
ଅକ୍ଷର *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
  ଅକ୍ଷର *z = 0;
  ଅଚଳ ଅକ୍ଷର *zSql = sqlite3_sql(pStmt);
  ଯଦି( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ଫେରନ୍ତୁ z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ଫେରନ୍ତୁ p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ସ୍ଥିର UnpackedRecord *vdbeUnpackRecord(
  ଚାବି_Info *p_ଚାବି_Info,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି,
  ଅଚଳ ଶୂନ୍ୟ *p_ଚାବି
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_ଚାବି_Info);
  ଯଦି( pRet ){
    memset(pRet->aMem, 0, ଆକାର(Mem)*(p_ଚାବି_Info->n_ଚାବି_Field+1));
    sqlite3VdbeRecordUnpack(n_ଚାବି, p_ଚାବି, pRet);
  }
  ଫେରନ୍ତୁ pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_preupdate_old(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdx, sqlite3_ମୂଲ୍ୟ **pp_ମୂଲ୍ୟ){
  PreUpdate *p;
  Mem *pMem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( db==0 || pp_ମୂଲ୍ୟ==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  ଯଦି( !p || p->op==SQLITE_ଭର୍ତ୍ତି ){
    rc = SQLITE_MISUSE_BKPT;
    ଯାଅ preupdate_old_out;
  }
  ଯଦି( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }ଅନ୍ୟଥା ଯଦି( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    ଯାଅ preupdate_old_out;
  }ଅନ୍ୟଥା{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  ଯଦି( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    ଯାଅ preupdate_old_out;
  }

  ଯଦି( iIdx==p->pTab->iPKey ){
    *pp_ମୂଲ୍ୟ = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }ଅନ୍ୟଥା{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    ଯଦି( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      ପ୍ରତିଜ୍ଞା( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      ଯଦି( !aRec ) ଯାଅ preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      ଯଦି( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        ଯଦି( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      ଯଦି( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        ଯାଅ preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_ମୂଲ୍ୟ = &p->pUnpacked->aMem[iStore];
    ଯଦି( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      ଯଦି( pCol->iDflt>0 ){
        ଯଦି( p->apDflt==0 ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
          ପ୍ରତିଜ୍ଞା( ଆକାର(sqlite3_ମୂଲ୍ୟ*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = ଆକାର(sqlite3_ମୂଲ୍ୟ*)*p->pTab->nCol;
          p->apDflt = (sqlite3_ମୂଲ୍ୟ**)sqlite3DbMallocZero(db, nByte);
          ଯଦି( p->apDflt==0 ) ଯାଅ preupdate_old_out;
        }
        ଯଦି( p->apDflt[iIdx]==0 ){
          sqlite3_ମୂଲ୍ୟ *pVal = 0;
          ପଦସମଷ୍ଟି *pDflt;
          ପ୍ରତିଜ୍ଞା( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_ପଦସମଷ୍ଟି;
          rc = sqlite3_ମୂଲ୍ୟ_From_ପଦସମଷ୍ଟି(db, pDflt, ENC(db), pCol->affinity, &pVal);
          ଯଦି( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_ମୂଲ୍ୟ = p->apDflt[iIdx];
      }ଅନ୍ୟଥା{
        *pp_ମୂଲ୍ୟ = (sqlite3_ମୂଲ୍ୟ *)column_Null_ମୂଲ୍ୟ();
      }
    }ଅନ୍ୟଥା ଯଦି( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      ଯଦି( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  ଫେରନ୍ତୁ sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_preupdate_ଗଣନା(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ଅନ୍ୟଥା
  p = db->pPreUpdate;
#endif
  ଫେରନ୍ତୁ (p ? p->pKeyinfo->n_ଚାବି_Field : 0);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ଅନ୍ୟଥା
  p = db->pPreUpdate;
#endif
  ଫେରନ୍ତୁ (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ଅନ୍ୟଥା
  p = db->pPreUpdate;
#endif
  ଫେରନ୍ତୁ (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_preupdate_new(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdx, sqlite3_ମୂଲ୍ୟ **pp_ମୂଲ୍ୟ){
  PreUpdate *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  Mem *pMem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( db==0 || pp_ମୂଲ୍ୟ==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  ଯଦି( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    ଯାଅ preupdate_new_out;
  }
  ଯଦି( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }ଅନ୍ୟଥା ଯଦି( iIdx >= p->pTab->nCol ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }ଅନ୍ୟଥା{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  ଯଦି( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    ଯାଅ preupdate_new_out;
  }

  ଯଦି( p->op==SQLITE_ଭର୍ତ୍ତି ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    ଯଦି( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      ଯଦି( rc!=SQLITE_OK ) ଯାଅ preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      ଯଦି( !pUnpack ){
        rc = SQLITE_NOMEM;
        ଯାଅ preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    ଯଦି( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }ଅନ୍ୟଥା ଯଦି( iStore>=pUnpack->nField ){
      pMem = (sqlite3_ମୂଲ୍ୟ *)column_Null_ମୂଲ୍ୟ();
    }ଅନ୍ୟଥା ଯଦି( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }ଅନ୍ୟଥା{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    ପ୍ରତିଜ୍ଞା( p->op==SQLITE_UPDATE );
    ଯଦି( !p->aNew ){
      ପ୍ରତିଜ୍ଞା( ଆକାର(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, ଆକାର(Mem)*p->pCsr->nField);
      ଯଦି( !p->aNew ){
        rc = SQLITE_NOMEM;
        ଯାଅ preupdate_new_out;
      }
    }
    ପ୍ରତିଜ୍ଞା( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    ଯଦି( pMem->flags==0 ){
      ଯଦି( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }ଅନ୍ୟଥା{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        ଯଦି( rc!=SQLITE_OK ) ଯାଅ preupdate_new_out;
      }
    }
  }
  *pp_ମୂଲ୍ୟ = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  ଫେରନ୍ତୁ sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iScan,                      /* Index of loop to report on */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iScanStatusOp,              /* Which metric to return */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ଶୂନ୍ୟ *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOp;
  ScanStatus *pScan = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    ଫେରନ୍ତୁ 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  ଯଦି( p->pFrame ){
    VdbeFrame *pFrame;
    ପାଇଁ(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  ଯଦି( iScan<0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ଯଦି( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      ପାଇଁ(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      ଫେରନ୍ତୁ 0;
    }
    ଫେରନ୍ତୁ 1;
  }
  ଯଦି( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }ଅନ୍ୟଥା{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    ପାଇଁ(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      ଯଦି( pScan->zName ){
        iScan--;
        ଯଦି( iScan<0 ) ଭାଙ୍ଗ;
      }
    }
  }
  ଯଦି( idx>=p->nScan ) ଫେରନ୍ତୁ 1;
  ପ୍ରତିଜ୍ଞା( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  ବାଛ( iScanStatusOp ){
    ସ୍ଥିତି SQLITE_SCANSTAT_NLOOP: {
      ଯଦି( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }ଅନ୍ୟଥା{
        *(sqlite3_int64*)pOut = -1;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_NVISIT: {
      ଯଦି( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }ଅନ୍ୟଥା{
        *(sqlite3_int64*)pOut = -1;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_EST: {
      ଦ୍ୱିଗୁଣ r = 1.0;
      LogEst x = pScan->nEst;
      ଯେତେବେଳେ( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(ଦ୍ୱିଗୁଣ*)pOut = r*sqlite3LogEstToInt(x);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_NAME: {
      *(ଅଚଳ ଅକ୍ଷର**)pOut = pScan->zName;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_EXPLAIN: {
      ଯଦି( pScan->addrExplain ){
        *(ଅଚଳ ଅକ୍ଷର**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }ଅନ୍ୟଥା{
        *(ଅଚଳ ଅକ୍ଷର**)pOut = 0;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_SELECTID: {
      ଯଦି( pScan->addrExplain ){
        *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pOut = aOp[ pScan->addrExplain ].p1;
      }ଅନ୍ୟଥା{
        *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pOut = -1;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_PARENTID: {
      ଯଦି( pScan->addrExplain ){
        *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pOut = aOp[ pScan->addrExplain ].p2;
      }ଅନ୍ୟଥା{
        *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pOut = -1;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      ଯଦି( pScan->aAddrRange[0]==0 ){
        res = -1;
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
        ପାଇଁ(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIns = pScan->aAddrRange[ii];
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEnd = pScan->aAddrRange[ii+1];
          ଯଦି( iIns==0 ) ଭାଙ୍ଗ;
          ଯଦି( iIns>0 ){
            ଯେତେବେଳେ( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }ଅନ୍ୟଥା{
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOp;
            ପାଇଁ(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              ଯଦି( pOp->p1!=iEnd ) ଜାରି;
              ଯଦି( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                ଜାରି;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ଫେରନ୍ତୁ 1;
    }
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iScan,                      /* Index of loop to report on */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iScanStatusOp,              /* Which metric to return */
  ଶୂନ୍ୟ *pOut                      /* OUT: Write the answer here */
){
  ଫେରନ୍ତୁ sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
ଶୂନ୍ୟ sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପାଇଁ(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
