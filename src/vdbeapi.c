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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"
#अन्तर्भाव "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
पूर्णांक sqlite3_expired(sqlite3_stmt *pStmt){
  पूर्णांक iRet = 1;
  जर( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  परत iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
स्थिर पूर्णांक vdbeSafety(Vdbe *p){
  जर( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    परत 1;
  }अन्यथा{
    परत 0;
  }
}
स्थिर पूर्णांक vdbeSafetyNotNull(Vdbe *p){
  जर( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    परत 1;
  }अन्यथा{
    परत vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
स्थिर SQLITE_NOINLINE शून्य invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  प्रतिज्ञा( p->startTime>0 );
  प्रतिज्ञा( db->init.busy==0 );
  प्रतिज्ञा( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  जर( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  जर( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (शून्य*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   जर( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#अन्यथा
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
पूर्णांक sqlite3_finalize(sqlite3_stmt *pStmt){
  पूर्णांक rc;
  जर( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }अन्यथा{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    जर( vdbeSafety(v) ) परत SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    प्रतिज्ञा( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  परत rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
पूर्णांक sqlite3_reset(sqlite3_stmt *pStmt){
  पूर्णांक rc;
  जर( pStmt==0 ){
    rc = SQLITE_OK;
  }अन्यथा{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    प्रतिज्ञा( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  परत rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
पूर्णांक sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  पूर्णांक i;
  पूर्णांक rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#जर SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pStmt==0 ){
    परत SQLITE_MISUSE_BKPT;
  }
#endif
#जर SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  साठी(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    जर( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  प्रतिज्ञा( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  जर( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  परत rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
स्थिरांक शून्य *sqlite3_मूल्य_blob(sqlite3_मूल्य *pVal){
  Mem *p = (Mem*)pVal;
  जर( p->flags & (MEM_Blob|MEM_Str) ){
    जर( ExpandBlob(p)!=SQLITE_OK ){
      प्रतिज्ञा( p->flags==MEM_Null && p->z==0 );
      परत 0;
    }
    p->flags |= MEM_Blob;
    परत p->n ? p->z : 0;
  }अन्यथा{
    परत sqlite3_मूल्य_text(pVal);
  }
}
पूर्णांक sqlite3_मूल्य_bytes(sqlite3_मूल्य *pVal){
  परत sqlite3_मूल्य_Bytes(pVal, SQLITE_UTF8);
}
पूर्णांक sqlite3_मूल्य_bytes16(sqlite3_मूल्य *pVal){
  परत sqlite3_मूल्य_Bytes(pVal, SQLITE_UTF16NATIVE);
}
द्विगुण sqlite3_मूल्य_double(sqlite3_मूल्य *pVal){
  परत sqlite3_Vdbe_Real_मूल्य((Mem*)pVal);
}
पूर्णांक sqlite3_मूल्य_int(sqlite3_मूल्य *pVal){
  परत (पूर्णांक)sqlite3_Vdbe_Int_मूल्य((Mem*)pVal);
}
sqlite_int64 sqlite3_मूल्य_int64(sqlite3_मूल्य *pVal){
  परत sqlite3_Vdbe_Int_मूल्य((Mem*)pVal);
}
अचिन्हित पूर्णांक sqlite3_मूल्य_subtype(sqlite3_मूल्य *pVal){
  Mem *pMem = (Mem*)pVal;
  परत ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
शून्य *sqlite3_मूल्य_pointer(sqlite3_मूल्य *pVal, स्थिरांक अक्षर *zPType){
  Mem *p = (Mem*)pVal;
  जर( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && म_भेद(p->u.zPType, zPType)==0
  ){
    परत (शून्य*)p->z;
  }अन्यथा{
    परत 0;
  }
}
स्थिरांक अचिन्हित अक्षर *sqlite3_मूल्य_text(sqlite3_मूल्य *pVal){
  परत (स्थिरांक अचिन्हित अक्षर *)sqlite3_मूल्य_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_मूल्य_text16(sqlite3_मूल्य* pVal){
  परत sqlite3_मूल्य_Text(pVal, SQLITE_UTF16NATIVE);
}
स्थिरांक शून्य *sqlite3_मूल्य_text16be(sqlite3_मूल्य *pVal){
  परत sqlite3_मूल्य_Text(pVal, SQLITE_UTF16BE);
}
स्थिरांक शून्य *sqlite3_मूल्य_text16le(sqlite3_मूल्य *pVal){
  परत sqlite3_मूल्य_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
पूर्णांक sqlite3_मूल्य_type(sqlite3_मूल्य* pVal){
  स्थिर स्थिरांक u8 aType[] = {
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
    पूर्णांक eType = SQLITE_BLOB;
    जर( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }अन्यथा जर( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }अन्यथा जर( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }अन्यथा जर( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    प्रतिज्ञा( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  परत aType[pVal->flags&MEM_AffMask];
}
पूर्णांक sqlite3_मूल्य_encoding(sqlite3_मूल्य *pVal){
  परत pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
पूर्णांक sqlite3_मूल्य_nochange(sqlite3_मूल्य *pVal){
  परत (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
पूर्णांक sqlite3_मूल्य_frombind(sqlite3_मूल्य *pVal){
  परत (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_मूल्य *sqlite3_मूल्य_dup(स्थिरांक sqlite3_मूल्य *pOrig){
  sqlite3_मूल्य *pNew;
  जर( pOrig==0 ) परत 0;
  pNew = sqlite3_malloc( आकार(*pNew) );
  जर( pNew==0 ) परत 0;
  स_रखो(pNew, 0, आकार(*pNew));
  स_नकल(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  जर( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    जर( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_मूल्य_Free(pNew);
      pNew = 0;
    }
  }अन्यथा जर( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  परत pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
शून्य sqlite3_मूल्य_free(sqlite3_मूल्य *pOld){
  sqlite3_मूल्य_Free(pOld);
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
स्थिर शून्य setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  स्थिरांक अक्षर *z,          /* String pointer */
  पूर्णांक n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  शून्य (*xDel)(शून्य*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  पूर्णांक rc;
  जर( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }अन्यथा जर( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    प्रतिज्ञा( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }अन्यथा{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  जर( rc ){
    जर( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }अन्यथा{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      प्रतिज्ञा( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    परत;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  जर( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
स्थिर पूर्णांक invoke_मूल्य_Destructor(
  स्थिरांक शून्य *p,             /* Value to destroy */
  शून्य (*xDel)(शून्य*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  जर( xDel==0 ){
    /* noop */
  }अन्यथा जर( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }अन्यथा{
    xDel((शून्य*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#अन्यथा
  प्रतिज्ञा( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  परत SQLITE_TOOBIG;
}
शून्य sqlite3_result_blob(
  sqlite3_context *pCtx,
  स्थिरांक शून्य *z,
  पूर्णांक n,
  शून्य (*xDel)(शून्य *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 || n<0 ){
    invoke_मूल्य_Destructor(z, xDel, pCtx);
    परत;
  }
#endif
  प्रतिज्ञा( n>=0 );
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
शून्य sqlite3_result_blob64(
  sqlite3_context *pCtx,
  स्थिरांक शून्य *z,
  sqlite3_uint64 n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ){
    invoke_मूल्य_Destructor(z, xDel, 0);
    परत;
  }
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  जर( n>0x7fffffff ){
    (शून्य)invoke_मूल्य_Destructor(z, xDel, pCtx);
  }अन्यथा{
    setResultStrOrError(pCtx, z, (पूर्णांक)n, 0, xDel);
  }
}
शून्य sqlite3_result_double(sqlite3_context *pCtx, द्विगुण rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
शून्य sqlite3_result_error(sqlite3_context *pCtx, स्थिरांक अक्षर *z, पूर्णांक n){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
शून्य sqlite3_result_error16(sqlite3_context *pCtx, स्थिरांक शून्य *z, पूर्णांक n){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
शून्य sqlite3_result_int(sqlite3_context *pCtx, पूर्णांक iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
शून्य sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
शून्य sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
शून्य sqlite3_result_pointer(
  sqlite3_context *pCtx,
  शून्य *pPtr,
  स्थिरांक अक्षर *zPType,
  शून्य (*xDestructor)(शून्य*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ){
    invoke_मूल्य_Destructor(pPtr, xDestructor, 0);
    परत;
  }
#endif
  pOut = pCtx->pOut;
  प्रतिज्ञा( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
शून्य sqlite3_result_subtype(sqlite3_context *pCtx, अचिन्हित पूर्णांक eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
#जर defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  जर( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    अक्षर zErr[200];
    sqlite3_snprintf(आकार(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    परत;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  प्रतिज्ञा( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
शून्य sqlite3_result_text(
  sqlite3_context *pCtx,
  स्थिरांक अक्षर *z,
  पूर्णांक n,
  शून्य (*xDel)(शून्य *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ){
    invoke_मूल्य_Destructor(z, xDel, 0);
    परत;
  }
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
शून्य sqlite3_result_text64(
  sqlite3_context *pCtx,
  स्थिरांक अक्षर *z,
  sqlite3_uint64 n,
  शून्य (*xDel)(शून्य *),
  अचिन्हित अक्षर enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ){
    invoke_मूल्य_Destructor(z, xDel, 0);
    परत;
  }
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  जर( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    जर( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  जर( n>0x7fffffff ){
    (शून्य)invoke_मूल्य_Destructor(z, xDel, pCtx);
  }अन्यथा{
    setResultStrOrError(pCtx, z, (पूर्णांक)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
शून्य sqlite3_result_text16(
  sqlite3_context *pCtx,
  स्थिरांक शून्य *z,
  पूर्णांक n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
शून्य sqlite3_result_text16be(
  sqlite3_context *pCtx,
  स्थिरांक शून्य *z,
  पूर्णांक n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
शून्य sqlite3_result_text16le(
  sqlite3_context *pCtx,
  स्थिरांक शून्य *z,
  पूर्णांक n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
शून्य sqlite3_result_मूल्य(sqlite3_context *pCtx, sqlite3_मूल्य *p_मूल्य){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
  जर( p_मूल्य==0 ){
    sqlite3_result_null(pCtx);
    परत;
  }
#endif
  pOut = pCtx->pOut;
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_मूल्य);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  जर( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
शून्य sqlite3_result_zeroblob(sqlite3_context *pCtx, पूर्णांक n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
पूर्णांक sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  प्रतिज्ञा( sqlite3_mutex_held(pOut->db->mutex) );
  जर( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    परत SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (पूर्णांक)n);
  परत SQLITE_OK;
#अन्यथा
  परत sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (पूर्णांक)n);
#endif
}
शून्य sqlite3_result_error_code(sqlite3_context *pCtx, पूर्णांक errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  जर( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  जर( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
शून्य sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
शून्य sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
शून्य sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, पूर्णांक eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
  जर( pStr==0 ) परत;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  जर( pStr->accError==0 ){
    जर( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }अन्यथा{
      स्थिरांक अक्षर *zText = sqlite3_str_मूल्य(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      प्रतिज्ञा( isMalloced(pStr) );
      जर( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }अन्यथा{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }अन्यथा जर( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }अन्यथा{
    प्रतिज्ञा( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  जर( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
शून्य sqlite3ResultIntReal(sqlite3_context *pCtx){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  जर( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
स्थिर पूर्णांक doWalCallbacks(sqlite3 *db){
  पूर्णांक rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  पूर्णांक i;
  साठी(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    जर( pBt ){
      पूर्णांक nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      जर( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#अन्यथा
  UNUSED_PARAMETER(db);
#endif
  परत rc;
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
स्थिर पूर्णांक sqlite3Step(Vdbe *p){
  sqlite3 *db;
  पूर्णांक rc;

  प्रतिज्ञा(p);
  db = p->db;
  जर( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    जर( p->eVdbeState==VDBE_READY_STATE ){
      जर( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        जर( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        जा end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      जर( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      प्रतिज्ञा( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      जर( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }अन्यथा{
        प्रतिज्ञा( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      जर( p->readOnly==0 ) db->nVdbeWrite++;
      जर( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }अन्यथा

    जर( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      जर( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }अन्यथा{
        परत SQLITE_MISUSE_BKPT;
      }
#अन्यथा
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      प्रतिज्ञा( p->eVdbeState==VDBE_READY_STATE );
      जा restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  जर( p->explain ){
    rc = sqlite3VdbeList(p);
  }अन्यथा
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  जर( rc==SQLITE_ROW ){
    प्रतिज्ञा( p->rc==SQLITE_OK );
    प्रतिज्ञा( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    परत SQLITE_ROW;
  }अन्यथा{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    जर( rc==SQLITE_DONE && db->autoCommit ){
      प्रतिज्ञा( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      जर( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }अन्यथा जर( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  जर( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    जर( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  प्रतिज्ञा( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  परत (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
पूर्णांक sqlite3_step(sqlite3_stmt *pStmt){
  पूर्णांक rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  पूर्णांक cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  जर( vdbeSafetyNotNull(v) ){
    परत SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  पर्यंत( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    पूर्णांक savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    जर( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      स्थिरांक अक्षर *zErr = (स्थिरांक अक्षर *)sqlite3_मूल्य_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_संदेश);
      जर( !db->mallocFailed ){
        v->z_Err_संदेश = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } अन्यथा {
        v->z_Err_संदेश = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      थांबा;
    }
    sqlite3_reset(pStmt);
    जर( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    प्रतिज्ञा( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  परत rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
शून्य *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( p==0 ) परत 0;
#endif
  प्रतिज्ञा( p && p->pFunc );
  परत p->pFunc->pUserData;
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
  जर( p==0 ) परत 0;
#अन्यथा
  प्रतिज्ञा( p && p->pOut );
#endif
  परत p->pOut->db;
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
पूर्णांक sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( p==0 ) परत 0;
#अन्यथा
  प्रतिज्ञा( p );
#endif
  परत sqlite3_मूल्य_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
शून्य sqlite3_Vdbe_मूल्य_List_Free(शून्य *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
स्थिर पूर्णांक मूल्य_From_मूल्य_List(
  sqlite3_मूल्य *pVal,        /* Pointer to the ValueList object */
  sqlite3_मूल्य **ppOut,      /* Store the next value from the list here */
  पूर्णांक bNext                   /* 1 for _next(). 0 for _first() */
){
  पूर्णांक rc;
  मूल्य_List *pRhs;

  *ppOut = 0;
  जर( pVal==0 ) परत SQLITE_MISUSE_BKPT;
  जर( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_मूल्य_List_Free ){
    परत SQLITE_ERROR;
  }अन्यथा{
    प्रतिज्ञा( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    प्रतिज्ञा( pVal->eSubtype=='p' );
    प्रतिज्ञा( pVal->u.zPType!=0 && म_भेद(pVal->u.zPType,"ValueList")==0 );
    pRhs = (मूल्य_List*)pVal->z;
  }
  जर( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }अन्यथा{
    पूर्णांक dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    प्रतिज्ञा( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    जर( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  जर( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    स_रखो(&sMem, 0, आकार(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    जर( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_मूल्य *pOut = pRhs->pOut;
      पूर्णांक iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      जर( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }अन्यथा{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  परत rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
पूर्णांक sqlite3_vtab_in_first(sqlite3_मूल्य *pVal, sqlite3_मूल्य **ppOut){
  परत मूल्य_From_मूल्य_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
पूर्णांक sqlite3_vtab_in_next(sqlite3_मूल्य *pVal, sqlite3_मूल्य **ppOut){
  परत मूल्य_From_मूल्य_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  पूर्णांक rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  प्रतिज्ञा( p->pVdbe!=0 );
#अन्यथा
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  जर( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    जर( rc ) *piTime = 0;
  }
  परत *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
स्थिर SQLITE_NOINLINE शून्य *createAggContext(sqlite3_context *p, पूर्णांक nByte){
  Mem *pMem = p->pMem;
  प्रतिज्ञा( (pMem->flags & MEM_Agg)==0 );
  जर( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }अन्यथा{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    जर( pMem->z ){
      स_रखो(pMem->z, 0, nByte);
    }
  }
  परत (शून्य*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
शून्य *sqlite3_aggregate_context(sqlite3_context *p, पूर्णांक nByte){
  प्रतिज्ञा( p && p->pFunc && p->pFunc->xFinalize );
  प्रतिज्ञा( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  जर( (p->pMem->flags & MEM_Agg)==0 ){
    परत createAggContext(p, nByte);
  }अन्यथा{
    परत (शून्य*)p->pMem->z;
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
शून्य *sqlite3_get_auxdata(sqlite3_context *pCtx, पूर्णांक iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत 0;
#endif
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#जर SQLITE_ENABLE_STAT4
  जर( pCtx->pVdbe==0 ) परत 0;
#अन्यथा
  प्रतिज्ञा( pCtx->pVdbe!=0 );
#endif
  साठी(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    जर(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      परत pAuxData->pAux;
    }
  }
  परत 0;
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
शून्य sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  पूर्णांक iArg,
  शून्य *pAux,
  शून्य (*xDelete)(शून्य*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pCtx==0 ) परत;
#endif
  pVdbe= pCtx->pVdbe;
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  जर( pVdbe==0 ) जा failed;
#अन्यथा
  प्रतिज्ञा( pVdbe!=0 );
#endif

  साठी(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    जर( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      थांबा;
    }
  }
  जर( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, आकार(AuxData));
    जर( !pAuxData ) जा failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    जर( pCtx->isError==0 ) pCtx->isError = -1;
  }अन्यथा जर( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  परत;

failed:
  जर( xDelete ){
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
पूर्णांक sqlite3_aggregate_मोजणी(sqlite3_context *p){
  प्रतिज्ञा( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  परत p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
पूर्णांक sqlite3_column_मोजणी(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  जर( pVm==0 ) परत 0;
  परत pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
पूर्णांक sqlite3_data_मोजणी(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  जर( pVm==0 || pVm->pResultRow==0 ) परत 0;
  परत pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
स्थिर स्थिरांक Mem *column_Null_मूल्य(शून्य){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  स्थिर स्थिरांक Mem nullMem
#जर defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (अक्षर*)0,
        /* .n          = */ (पूर्णांक)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (पूर्णांक)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (अक्षर*)0,
        /* .xDel       = */ (शून्य(*)(शून्य*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  परत &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
स्थिर Mem *columnMem(sqlite3_stmt *pStmt, पूर्णांक i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  जर( pVm==0 ) परत (Mem*)column_Null_मूल्य();
  प्रतिज्ञा( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  जर( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }अन्यथा{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_मूल्य();
  }
  परत pOut;
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
स्थिर शून्य columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  जर( p ){
    प्रतिज्ञा( p->db!=0 );
    प्रतिज्ञा( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
स्थिरांक शून्य *sqlite3_column_blob(sqlite3_stmt *pStmt, पूर्णांक i){
  स्थिरांक शून्य *val;
  val = sqlite3_मूल्य_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  परत val;
}
पूर्णांक sqlite3_column_bytes(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक val = sqlite3_मूल्य_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
पूर्णांक sqlite3_column_bytes16(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक val = sqlite3_मूल्य_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
द्विगुण sqlite3_column_double(sqlite3_stmt *pStmt, पूर्णांक i){
  द्विगुण val = sqlite3_मूल्य_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
पूर्णांक sqlite3_column_int(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक val = sqlite3_मूल्य_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, पूर्णांक i){
  sqlite_int64 val = sqlite3_मूल्य_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
स्थिरांक अचिन्हित अक्षर *sqlite3_column_text(sqlite3_stmt *pStmt, पूर्णांक i){
  स्थिरांक अचिन्हित अक्षर *val = sqlite3_मूल्य_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
sqlite3_मूल्य *sqlite3_column_मूल्य(sqlite3_stmt *pStmt, पूर्णांक i){
  Mem *pOut = columnMem(pStmt, i);
  जर( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  परत (sqlite3_मूल्य *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_column_text16(sqlite3_stmt *pStmt, पूर्णांक i){
  स्थिरांक शून्य *val = sqlite3_मूल्य_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत val;
}
#endif /* SQLITE_OMIT_UTF16 */
पूर्णांक sqlite3_column_type(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक iType = sqlite3_मूल्य_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  परत iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
स्थिर स्थिरांक अक्षर * स्थिरांक azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
स्थिर स्थिरांक u16 azExplainColNames16data[] = {
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
स्थिर स्थिरांक u8 iExplainColNames16[] = {
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
स्थिर स्थिरांक शून्य *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  पूर्णांक N,                   /* Which column to get the name for */
  पूर्णांक useUtf16,            /* True to return the name as UTF16 */
  पूर्णांक useType              /* What type of name */
){
  स्थिरांक शून्य *ret;
  Vdbe *p;
  पूर्णांक n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pStmt==0 ){
    (शून्य)SQLITE_MISUSE_BKPT;
    परत 0;
  }
#endif
  जर( N<0 ) परत 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  प्रतिज्ञा( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  जर( p->explain ){
    जर( useType>0 ) जा columnName_end;
    n = p->explain==1 ? 8 : 4;
    जर( N>=n ) जा columnName_end;
    जर( useUtf16 ){
      पूर्णांक i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (शून्य*)&azExplainColNames16data[i];
    }अन्यथा{
      ret = (शून्य*)azExplainColNames8[N + 8*p->explain - 8];
    }
    जा columnName_end;
  }
  n = p->nResColumn;
  जर( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    जर( useUtf16 ){
      ret = sqlite3_मूल्य_text16((sqlite3_मूल्य*)&p->aColName[N]);
    }अन्यथा
#endif
    {
      ret = sqlite3_मूल्य_text((sqlite3_मूल्य*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    प्रतिज्ञा( db->mallocFailed==0 || db->mallocFailed==1 );
    जर( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  परत ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
स्थिरांक अक्षर *sqlite3_column_name(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_column_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#जर defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
स्थिरांक अक्षर *sqlite3_column_decltype(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_column_decltype16(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
स्थिरांक अक्षर *sqlite3_column_database_name(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_column_database_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
स्थिरांक अक्षर *sqlite3_column_table_name(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_column_table_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
स्थिरांक अक्षर *sqlite3_column_origin_name(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
स्थिरांक शून्य *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  परत columnName(pStmt, N, 1, COLNAME_COLUMN);
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
स्थिर पूर्णांक vdbeUnbind(Vdbe *p, अचिन्हित पूर्णांक i){
  Mem *pVar;
  जर( vdbeSafetyNotNull(p) ){
    परत SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  जर( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    परत SQLITE_MISUSE_BKPT;
  }
  जर( i>=(अचिन्हित पूर्णांक)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    परत SQLITE_RANGE;
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
  प्रतिज्ञा( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  प्रतिज्ञा( (p->expmask & p->smimask)==p->smimask );
  जर( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    जर( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  परत SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
स्थिर पूर्णांक bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  पूर्णांक i,                 /* Index of the parameter to bind */
  स्थिरांक शून्य *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  शून्य (*xDel)(शून्य*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  पूर्णांक rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  जर( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    जर( zData!=0 ){
      pVar = &p->aVar[i-1];
      जर( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }अन्यथा जर( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        प्रतिज्ञा( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }अन्यथा{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        जर( encoding==0 ) pVar->enc = ENC(p->db);
      }
      जर( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      जर( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }अन्यथा जर( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((शून्य*)zData);
  }
  परत rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
पूर्णांक sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिरांक शून्य *zData,
  पूर्णांक nData,
  शून्य (*xDel)(शून्य*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( nData<0 ) परत SQLITE_MISUSE_BKPT;
#endif
  परत bindText(pStmt, i, zData, nData, xDel, 0);
}
पूर्णांक sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिरांक शून्य *zData,
  sqlite3_uint64 nData,
  शून्य (*xDel)(शून्य*)
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  परत bindText(pStmt, i, zData, nData, xDel, 0);
}
पूर्णांक sqlite3_bind_double(sqlite3_stmt *pStmt, पूर्णांक i, द्विगुण r_मूल्य){
  पूर्णांक rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  जर( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_मूल्य);
    sqlite3_mutex_leave(p->db->mutex);
  }
  परत rc;
}
पूर्णांक sqlite3_bind_int(sqlite3_stmt *p, पूर्णांक i, पूर्णांक i_मूल्य){
  परत sqlite3_bind_int64(p, i, (i64)i_मूल्य);
}
पूर्णांक sqlite3_bind_int64(sqlite3_stmt *pStmt, पूर्णांक i, sqlite_int64 i_मूल्य){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  जर( vdbeSafetyNotNull(p) ){
    परत SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  जर( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    परत SQLITE_MISUSE_BKPT;
  }
  जर( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    परत SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  जर( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    प्रतिज्ञा( (p->expmask & p->smimask)==p->smimask );
    जर( (p->expmask & expireMask)!=0 ){
      जर( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        पूर्णांक sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        पूर्णांक sm2 = i_मूल्य>=0 && i_मूल्य<=1;
        जर( sm1!=sm2 ){
          p->expired = 1;
        }
      }अन्यथा जर( (pVar->flags & MEM_Int)==0 || i_मूल्य!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_मूल्य);
  sqlite3_mutex_leave(p->db->mutex);
  परत SQLITE_OK;
}
पूर्णांक sqlite3_bind_null(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  जर( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  परत rc;
}
पूर्णांक sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  शून्य *pPtr,
  स्थिरांक अक्षर *zPTtype,
  शून्य (*xDestructor)(शून्य*)
){
  पूर्णांक rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  जर( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }अन्यथा जर( xDestructor ){
    xDestructor(pPtr);
  }
  परत rc;
}
पूर्णांक sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिरांक अक्षर *zData,
  पूर्णांक nData,
  शून्य (*xDel)(शून्य*)
){
  परत bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
पूर्णांक sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिरांक अक्षर *zData,
  sqlite3_uint64 nData,
  शून्य (*xDel)(शून्य*),
  अचिन्हित अक्षर enc
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  जर( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    जर( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  परत bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
पूर्णांक sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिरांक शून्य *zData,
  पूर्णांक n,
  शून्य (*xDel)(शून्य*)
){
  परत bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
पूर्णांक sqlite3_bind_मूल्य(sqlite3_stmt *pStmt, पूर्णांक i, स्थिरांक sqlite3_मूल्य *p_मूल्य){
  पूर्णांक rc;
  निवड( sqlite3_मूल्य_type((sqlite3_मूल्य*)p_मूल्य) ){
    स्थिती SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_मूल्य->u.i);
      थांबा;
    }
    स्थिती SQLITE_FLOAT: {
      प्रतिज्ञा( p_मूल्य->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_मूल्य->flags & MEM_Real) ? p_मूल्य->u.r : (द्विगुण)p_मूल्य->u.i
      );
      थांबा;
    }
    स्थिती SQLITE_BLOB: {
      जर( p_मूल्य->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_मूल्य->u.nZero);
      }अन्यथा{
        rc = sqlite3_bind_blob(pStmt, i, p_मूल्य->z, p_मूल्य->n,SQLITE_TRANSIENT);
      }
      थांबा;
    }
    स्थिती SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_मूल्य->z, p_मूल्य->n, SQLITE_TRANSIENT,
                              p_मूल्य->enc);
      थांबा;
    }
    पूर्वनिर्धारित: {
      rc = sqlite3_bind_null(pStmt, i);
      थांबा;
    }
  }
  परत rc;
}
पूर्णांक sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, पूर्णांक i, पूर्णांक n){
  पूर्णांक rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  जर( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#अन्यथा
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  परत rc;
}
पूर्णांक sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, पूर्णांक i, sqlite3_uint64 n){
  पूर्णांक rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( p==0 ) परत SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  जर( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }अन्यथा{
    प्रतिज्ञा( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  परत rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
पूर्णांक sqlite3_bind_parameter_मोजणी(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  परत p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
स्थिरांक अक्षर *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, पूर्णांक i){
  Vdbe *p = (Vdbe*)pStmt;
  जर( p==0 ) परत 0;
  परत sqlite3_VList_अंक_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
पूर्णांक sqlite3VdbeParameterIndex(Vdbe *p, स्थिरांक अक्षर *zName, पूर्णांक nName){
  जर( p==0 || zName==0 ) परत 0;
  परत sqlite3_VList_Name_To_अंक(p->pVList, zName, nName);
}
पूर्णांक sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, स्थिरांक अक्षर *zName){
  परत sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
पूर्णांक sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  पूर्णांक i;
  प्रतिज्ञा( pTo->db==pFrom->db );
  प्रतिज्ञा( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  साठी(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  परत SQLITE_OK;
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
पूर्णांक sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  जर( pFrom->nVar!=pTo->nVar ){
    परत SQLITE_ERROR;
  }
  प्रतिज्ञा( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  जर( pTo->expmask ){
    pTo->expired = 1;
  }
  प्रतिज्ञा( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  जर( pFrom->expmask ){
    pFrom->expired = 1;
  }
  परत sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  परत pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
पूर्णांक sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  परत pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
पूर्णांक sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  परत pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
पूर्णांक sqlite3_stmt_explain(sqlite3_stmt *pStmt, पूर्णांक eMode){
  Vdbe *v = (Vdbe*)pStmt;
  पूर्णांक rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( pStmt==0 ) परत SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  जर( ((पूर्णांक)v->explain)==eMode ){
    rc = SQLITE_OK;
  }अन्यथा जर( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }अन्यथा जर( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा जर( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }अन्यथा जर( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }अन्यथा{
    पूर्णांक savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    जर( rc ){
      v->explain = savedMode;
    }अन्यथा{
      v->haveEqpOps = eMode==2;
    }
  }
  जर( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }अन्यथा{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  परत rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
पूर्णांक sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  परत v!=0 && v->eVdbeState==VDBE_RUN_STATE;
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
  जर( !sqlite3SafetyCheckOk(pDb) ){
    (शून्य)SQLITE_MISUSE_BKPT;
    परत 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  जर( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }अन्यथा{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  परत pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
पूर्णांक sqlite3_stmt_status(sqlite3_stmt *pStmt, पूर्णांक op, पूर्णांक resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (शून्य)SQLITE_MISUSE_BKPT;
    परत 0;
  }
#endif
  जर( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (पूर्णांक*)&v;
    प्रतिज्ञा( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }अन्यथा{
    v = pVdbe->aCounter[op];
    जर( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  परत (पूर्णांक)v;
}

/*
** Return the SQL associated with a prepared statement
*/
स्थिरांक अक्षर *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  परत p ? p->zSql : 0;
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
अक्षर *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  परत 0;
#अन्यथा
  अक्षर *z = 0;
  स्थिरांक अक्षर *zSql = sqlite3_sql(pStmt);
  जर( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  परत z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
स्थिरांक अक्षर *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  जर( p==0 ) परत 0;
  जर( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  परत p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
स्थिर UnpackedRecord *vdbeUnpackRecord(
  किल्ली_Info *p_किल्ली_Info,
  पूर्णांक n_किल्ली,
  स्थिरांक शून्य *p_किल्ली
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_किल्ली_Info);
  जर( pRet ){
    स_रखो(pRet->aMem, 0, आकार(Mem)*(p_किल्ली_Info->n_किल्ली_Field+1));
    sqlite3VdbeRecordUnpack(n_किल्ली, p_किल्ली, pRet);
  }
  परत pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
पूर्णांक sqlite3_preupdate_old(sqlite3 *db, पूर्णांक iIdx, sqlite3_मूल्य **pp_मूल्य){
  PreUpdate *p;
  Mem *pMem;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( db==0 || pp_मूल्य==0 ){
    परत SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  जर( !p || p->op==SQLITE_घाला ){
    rc = SQLITE_MISUSE_BKPT;
    जा preupdate_old_out;
  }
  जर( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }अन्यथा जर( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    जा preupdate_old_out;
  }अन्यथा{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  जर( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    जा preupdate_old_out;
  }

  जर( iIdx==p->pTab->iPKey ){
    *pp_मूल्य = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }अन्यथा{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    जर( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      प्रतिज्ञा( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      जर( !aRec ) जा preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      जर( rc==SQLITE_OK ){
        स_रखो(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        जर( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      जर( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        जा preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_मूल्य = &p->pUnpacked->aMem[iStore];
    जर( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      जर( pCol->iDflt>0 ){
        जर( p->apDflt==0 ){
          पूर्णांक nByte;
          प्रतिज्ञा( आकार(sqlite3_मूल्य*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = आकार(sqlite3_मूल्य*)*p->pTab->nCol;
          p->apDflt = (sqlite3_मूल्य**)sqlite3DbMallocZero(db, nByte);
          जर( p->apDflt==0 ) जा preupdate_old_out;
        }
        जर( p->apDflt[iIdx]==0 ){
          sqlite3_मूल्य *pVal = 0;
          अभिव्यक्ती *pDflt;
          प्रतिज्ञा( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_अभिव्यक्ती;
          rc = sqlite3_मूल्य_From_अभिव्यक्ती(db, pDflt, ENC(db), pCol->affinity, &pVal);
          जर( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_मूल्य = p->apDflt[iIdx];
      }अन्यथा{
        *pp_मूल्य = (sqlite3_मूल्य *)column_Null_मूल्य();
      }
    }अन्यथा जर( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      जर( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  परत sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
पूर्णांक sqlite3_preupdate_मोजणी(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#endif
  परत (p ? p->pKeyinfo->n_किल्ली_Field : 0);
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
पूर्णांक sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#endif
  परत (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
पूर्णांक sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#endif
  परत (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
पूर्णांक sqlite3_preupdate_new(sqlite3 *db, पूर्णांक iIdx, sqlite3_मूल्य **pp_मूल्य){
  PreUpdate *p;
  पूर्णांक rc = SQLITE_OK;
  Mem *pMem;
  पूर्णांक iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( db==0 || pp_मूल्य==0 ){
    परत SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  जर( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    जा preupdate_new_out;
  }
  जर( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }अन्यथा जर( iIdx >= p->pTab->nCol ){
    परत SQLITE_MISUSE_BKPT;
  }अन्यथा{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  जर( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    जा preupdate_new_out;
  }

  जर( p->op==SQLITE_घाला ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    जर( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      जर( rc!=SQLITE_OK ) जा preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      जर( !pUnpack ){
        rc = SQLITE_NOMEM;
        जा preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    जर( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }अन्यथा जर( iStore>=pUnpack->nField ){
      pMem = (sqlite3_मूल्य *)column_Null_मूल्य();
    }अन्यथा जर( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }अन्यथा{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    प्रतिज्ञा( p->op==SQLITE_UPDATE );
    जर( !p->aNew ){
      प्रतिज्ञा( आकार(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, आकार(Mem)*p->pCsr->nField);
      जर( !p->aNew ){
        rc = SQLITE_NOMEM;
        जा preupdate_new_out;
      }
    }
    प्रतिज्ञा( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    जर( pMem->flags==0 ){
      जर( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }अन्यथा{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        जर( rc!=SQLITE_OK ) जा preupdate_new_out;
      }
    }
  }
  *pp_मूल्य = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  परत sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
पूर्णांक sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  पूर्णांक iScan,                      /* Index of loop to report on */
  पूर्णांक iScanStatusOp,              /* Which metric to return */
  पूर्णांक flags,
  शून्य *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  पूर्णांक nOp;
  ScanStatus *pScan = 0;
  पूर्णांक idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    परत 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  जर( p->pFrame ){
    VdbeFrame *pFrame;
    साठी(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  जर( iScan<0 ){
    पूर्णांक ii;
    जर( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      साठी(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      परत 0;
    }
    परत 1;
  }
  जर( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }अन्यथा{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    साठी(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      जर( pScan->zName ){
        iScan--;
        जर( iScan<0 ) थांबा;
      }
    }
  }
  जर( idx>=p->nScan ) परत 1;
  प्रतिज्ञा( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  निवड( iScanStatusOp ){
    स्थिती SQLITE_SCANSTAT_NLOOP: {
      जर( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }अन्यथा{
        *(sqlite3_int64*)pOut = -1;
      }
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_NVISIT: {
      जर( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }अन्यथा{
        *(sqlite3_int64*)pOut = -1;
      }
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_EST: {
      द्विगुण r = 1.0;
      LogEst x = pScan->nEst;
      पर्यंत( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(द्विगुण*)pOut = r*sqlite3LogEstToInt(x);
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_NAME: {
      *(स्थिरांक अक्षर**)pOut = pScan->zName;
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_EXPLAIN: {
      जर( pScan->addrExplain ){
        *(स्थिरांक अक्षर**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }अन्यथा{
        *(स्थिरांक अक्षर**)pOut = 0;
      }
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_SELECTID: {
      जर( pScan->addrExplain ){
        *(पूर्णांक*)pOut = aOp[ pScan->addrExplain ].p1;
      }अन्यथा{
        *(पूर्णांक*)pOut = -1;
      }
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_PARENTID: {
      जर( pScan->addrExplain ){
        *(पूर्णांक*)pOut = aOp[ pScan->addrExplain ].p2;
      }अन्यथा{
        *(पूर्णांक*)pOut = -1;
      }
      थांबा;
    }
    स्थिती SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      जर( pScan->aAddrRange[0]==0 ){
        res = -1;
      }अन्यथा{
        पूर्णांक ii;
        साठी(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          पूर्णांक iIns = pScan->aAddrRange[ii];
          पूर्णांक iEnd = pScan->aAddrRange[ii+1];
          जर( iIns==0 ) थांबा;
          जर( iIns>0 ){
            पर्यंत( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }अन्यथा{
            पूर्णांक iOp;
            साठी(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              जर( pOp->p1!=iEnd ) सुरूठेवा;
              जर( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                सुरूठेवा;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      थांबा;
    }
    पूर्वनिर्धारित: {
      परत 1;
    }
  }
  परत 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
पूर्णांक sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  पूर्णांक iScan,                      /* Index of loop to report on */
  पूर्णांक iScanStatusOp,              /* Which metric to return */
  शून्य *pOut                      /* OUT: Write the answer here */
){
  परत sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
शून्य sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  पूर्णांक ii;
  साठी(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
