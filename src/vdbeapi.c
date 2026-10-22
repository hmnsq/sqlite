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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"
#ಅನ್ತರ್ಭಾವ "opcodes.h"

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_expired(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಙ್ಕ iRet = 1;
  ಯದಿ( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ iRet;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vdbeSafety(Vdbe *p){
  ಯದಿ( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vdbeSafetyNotNull(Vdbe *p){
  ಯದಿ( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ vdbeSafety(p);
  }
}

#ifndef SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  ಪ್ರತಿಜ್ಞಾ( p->startTime>0 );
  ಪ್ರತಿಜ್ಞಾ( db->init.busy==0 );
  ಪ್ರತಿಜ್ಞಾ( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#ifndef SQLITE_OMIT_DEPRECATED
  ಯದಿ( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#endif
  ಯದಿ( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (ಶೂನ್ಯ*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   ಯದಿ( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#ಅನ್ಯಥಾ
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_finalize(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }ಅನ್ಯಥಾ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    ಯದಿ( vdbeSafety(v) ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    ಪ್ರತಿಜ್ಞಾ( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_reset(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( pStmt==0 ){
    rc = SQLITE_OK;
  }ಅನ್ಯಥಾ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    ಪ್ರತಿಜ್ಞಾ( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#ಯದಿ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pStmt==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
#endif
#ಯದಿ SQLITE_THREADSAFE
  mutex = p->db->mutex;
#endif

  sqlite3_mutex_enter(mutex);
  ಪ್ರತಿ(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    ಯದಿ( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  ಪ್ರತಿಜ್ಞಾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ಯದಿ( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_blob(sqlite3_ಮೌಲ್ಯ *pVal){
  Mem *p = (Mem*)pVal;
  ಯದಿ( p->flags & (MEM_Blob|MEM_Str) ){
    ಯದಿ( ExpandBlob(p)!=SQLITE_OK ){
      ಪ್ರತಿಜ್ಞಾ( p->flags==MEM_Null && p->z==0 );
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    p->flags |= MEM_Blob;
    ಪ್ರತ್ಯಾಗಮ p->n ? p->z : 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_text(pVal);
  }
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_bytes(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_Bytes(pVal, SQLITE_UTF8);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_bytes16(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_Bytes(pVal, SQLITE_UTF16NATIVE);
}
ದ್ವಿಗುಣ sqlite3_ಮೌಲ್ಯ_double(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_Vdbe_Real_ಮೌಲ್ಯ((Mem*)pVal);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_int(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)sqlite3_Vdbe_Int_ಮೌಲ್ಯ((Mem*)pVal);
}
sqlite_int64 sqlite3_ಮೌಲ್ಯ_int64(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_Vdbe_Int_ಮೌಲ್ಯ((Mem*)pVal);
}
unsigned ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_subtype(sqlite3_ಮೌಲ್ಯ *pVal){
  Mem *pMem = (Mem*)pVal;
  ಪ್ರತ್ಯಾಗಮ ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_pointer(sqlite3_ಮೌಲ್ಯ *pVal, ಧ್ರುವ ವರ್ಣ *zPType){
  Mem *p = (Mem*)pVal;
  ಯದಿ( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)p->z;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}
ಧ್ರುವ unsigned ವರ್ಣ *sqlite3_ಮೌಲ್ಯ_text(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ (ಧ್ರುವ unsigned ವರ್ಣ *)sqlite3_ಮೌಲ್ಯ_Text(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_text16(sqlite3_ಮೌಲ್ಯ* pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_Text(pVal, SQLITE_UTF16NATIVE);
}
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_text16be(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_Text(pVal, SQLITE_UTF16BE);
}
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_text16le(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_Text(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_type(sqlite3_ಮೌಲ್ಯ* pVal){
  ಸ್ಥಿರ ಧ್ರುವ u8 aType[] = {
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
    ಪೂರ್ಣಾಙ್ಕ eType = SQLITE_BLOB;
    ಯದಿ( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }ಅನ್ಯಥಾ ಯದಿ( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }ಅನ್ಯಥಾ ಯದಿ( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }ಅನ್ಯಥಾ ಯದಿ( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    ಪ್ರತಿಜ್ಞಾ( eType == aType[pVal->flags&MEM_AffMask] );
  }
#endif
  ಪ್ರತ್ಯಾಗಮ aType[pVal->flags&MEM_AffMask];
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_encoding(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_nochange(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಮೌಲ್ಯ_frombind(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪ್ರತ್ಯಾಗಮ (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_ಮೌಲ್ಯ *sqlite3_ಮೌಲ್ಯ_dup(ಧ್ರುವ sqlite3_ಮೌಲ್ಯ *pOrig){
  sqlite3_ಮೌಲ್ಯ *pNew;
  ಯದಿ( pOrig==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew = sqlite3_malloc( ಪರಿಮಾಣ(*pNew) );
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  memset(pNew, 0, ಪರಿಮಾಣ(*pNew));
  memcpy(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  ಯದಿ( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    ಯದಿ( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_ಮೌಲ್ಯ_Free(pNew);
      pNew = 0;
    }
  }ಅನ್ಯಥಾ ಯದಿ( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_free(sqlite3_ಮೌಲ್ಯ *pOld){
  sqlite3_ಮೌಲ್ಯ_Free(pOld);
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
ಸ್ಥಿರ ಶೂನ್ಯ setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  ಧ್ರುವ ವರ್ಣ *z,          /* String pointer */
  ಪೂರ್ಣಾಙ್ಕ n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }ಅನ್ಯಥಾ ಯದಿ( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    ಪ್ರತಿಜ್ಞಾ( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_LENGTH]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }ಅನ್ಯಥಾ{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  ಯದಿ( rc ){
    ಯದಿ( rc==SQLITE_TOOBIG ){
      sqlite3_result_error_toobig(pCtx);
    }ಅನ್ಯಥಾ{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_NOMEM );
      sqlite3_result_error_nomem(pCtx);
    }
    ಪ್ರತ್ಯಾಗಮ;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  ಯದಿ( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ invoke_ಮೌಲ್ಯ_Destructor(
  ಧ್ರುವ ಶೂನ್ಯ *p,             /* Value to destroy */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಯದಿ( xDel==0 ){
    /* noop */
  }ಅನ್ಯಥಾ ಯದಿ( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }ಅನ್ಯಥಾ{
    xDel((ಶೂನ್ಯ*)p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx!=0 ){
    sqlite3_result_error_toobig(pCtx);
  }
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( pCtx!=0 );
  sqlite3_result_error_toobig(pCtx);
#endif
  ಪ್ರತ್ಯಾಗಮ SQLITE_TOOBIG;
}
ಶೂನ್ಯ sqlite3_result_blob(
  sqlite3_context *pCtx,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 || n<0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, pCtx);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  ಪ್ರತಿಜ್ಞಾ( n>=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
ಶೂನ್ಯ sqlite3_result_blob64(
  sqlite3_context *pCtx,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  sqlite3_uint64 n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, 0);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  ಯದಿ( n>0x7fffffff ){
    (ಶೂನ್ಯ)invoke_ಮೌಲ್ಯ_Destructor(z, xDel, pCtx);
  }ಅನ್ಯಥಾ{
    setResultStrOrError(pCtx, z, (ಪೂರ್ಣಾಙ್ಕ)n, 0, xDel);
  }
}
ಶೂನ್ಯ sqlite3_result_double(sqlite3_context *pCtx, ದ್ವಿಗುಣ rVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
ಶೂನ್ಯ sqlite3_result_error(sqlite3_context *pCtx, ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ n){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
ಶೂನ್ಯ sqlite3_result_error16(sqlite3_context *pCtx, ಧ್ರುವ ಶೂನ್ಯ *z, ಪೂರ್ಣಾಙ್ಕ n){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
ಶೂನ್ಯ sqlite3_result_int(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
ಶೂನ್ಯ sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
ಶೂನ್ಯ sqlite3_result_null(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
ಶೂನ್ಯ sqlite3_result_pointer(
  sqlite3_context *pCtx,
  ಶೂನ್ಯ *pPtr,
  ಧ್ರುವ ವರ್ಣ *zPType,
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)
){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(pPtr, xDestructor, 0);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  pOut = pCtx->pOut;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
ಶೂನ್ಯ sqlite3_result_subtype(sqlite3_context *pCtx, unsigned ಪೂರ್ಣಾಙ್ಕ eSubtype){
  Mem *pOut;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
#ಯದಿ defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  ಯದಿ( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_RESULT_SUBTYPE)==0
  ){
    ವರ್ಣ zErr[200];
    sqlite3_snprintf(ಪರಿಮಾಣ(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_result_error(pCtx, zErr, -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
ಶೂನ್ಯ sqlite3_result_text(
  sqlite3_context *pCtx,
  ಧ್ರುವ ವರ್ಣ *z,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, 0);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
ಶೂನ್ಯ sqlite3_result_text64(
  sqlite3_context *pCtx,
  ಧ್ರುವ ವರ್ಣ *z,
  sqlite3_uint64 n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *),
  unsigned ವರ್ಣ enc
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, 0);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಯದಿ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ಯದಿ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  ಯದಿ( n>0x7fffffff ){
    (ಶೂನ್ಯ)invoke_ಮೌಲ್ಯ_Destructor(z, xDel, pCtx);
  }ಅನ್ಯಥಾ{
    setResultStrOrError(pCtx, z, (ಪೂರ್ಣಾಙ್ಕ)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#ifndef SQLITE_OMIT_UTF16
ಶೂನ್ಯ sqlite3_result_text16(
  sqlite3_context *pCtx,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
ಶೂನ್ಯ sqlite3_result_text16be(
  sqlite3_context *pCtx,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
ಶೂನ್ಯ sqlite3_result_text16le(
  sqlite3_context *pCtx,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  setResultStrOrError(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
ಶೂನ್ಯ sqlite3_result_ಮೌಲ್ಯ(sqlite3_context *pCtx, sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p_ಮೌಲ್ಯ==0 ){
    sqlite3_result_null(pCtx);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  pOut = pCtx->pOut;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_ಮೌಲ್ಯ);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  ಯದಿ( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_result_error_toobig(pCtx);
  }
}
ಶೂನ್ಯ sqlite3_result_zeroblob(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ n){
  sqlite3_result_zeroblob64(pCtx, n>0 ? n : 0);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  pOut = pCtx->pOut;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pOut->db->mutex) );
  ಯದಿ( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(pCtx);
    ಪ್ರತ್ಯಾಗಮ SQLITE_TOOBIG;
  }
#ifndef SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (ಪೂರ್ಣಾಙ್ಕ)n);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (ಪೂರ್ಣಾಙ್ಕ)n);
#endif
}
ಶೂನ್ಯ sqlite3_result_error_code(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ errCode){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  pCtx->isError = errCode ? errCode : -1;
#ifdef SQLITE_DEBUG
  ಯದಿ( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#endif
  ಯದಿ( pCtx->pOut->flags & MEM_Null ){
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
ಶೂನ್ಯ sqlite3_result_error_toobig(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
ಶೂನ್ಯ sqlite3_result_error_nomem(sqlite3_context *pCtx){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
ಶೂನ್ಯ sqlite3_result_str(sqlite3_context *pCtx, sqlite3_str *pStr, ಪೂರ್ಣಾಙ್ಕ eOwn){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( pStr==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  ಯದಿ( pStr->accError==0 ){
    ಯದಿ( pStr->nChar==0 ){
      setResultStrOrError(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }ಅನ್ಯಥಾ{
      ಧ್ರುವ ವರ್ಣ *zText = sqlite3_str_ಮೌಲ್ಯ(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      ಪ್ರತಿಜ್ಞಾ( isMalloced(pStr) );
      ಯದಿ( eOwn==SQLITE_COPY ){
        setResultStrOrError(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }ಅನ್ಯಥಾ{
        setResultStrOrError(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }ಅನ್ಯಥಾ ಯದಿ( pStr->accError==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(pCtx);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( pStr->accError==SQLITE_TOOBIG );
    sqlite3_result_error_toobig(pCtx);
  }
  ಯದಿ( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#ifndef SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
ಶೂನ್ಯ sqlite3ResultIntReal(sqlite3_context *pCtx){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  ಯದಿ( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#endif


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ doWalCallbacks(sqlite3 *db){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ಯದಿ( pBt ){
      ಪೂರ್ಣಾಙ್ಕ nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      ಯದಿ( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#ಅನ್ಯಥಾ
  UNUSED_PARAMETER(db);
#endif
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3Step(Vdbe *p){
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಪ್ರತಿಜ್ಞಾ(p);
  db = p->db;
  ಯದಿ( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    ಯದಿ( p->eVdbeState==VDBE_READY_STATE ){
      ಯದಿ( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        ಯದಿ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        ಗಚ್ಛ end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      ಯದಿ( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      ಪ್ರತಿಜ್ಞಾ( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ifndef SQLITE_OMIT_TRACE
      ಯದಿ( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( p->startTime==0 );
      }
#endif

      db->nVdbeActive++;
      ಯದಿ( p->readOnly==0 ) db->nVdbeWrite++;
      ಯದಿ( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }ಅನ್ಯಥಾ

    ಯದಿ( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
      ಯದಿ( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
      }
#ಅನ್ಯಥಾ
      sqlite3_reset((sqlite3_stmt*)p);
#endif
      ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_READY_STATE );
      ಗಚ್ಛ restart_step;
    }
  }

#ifdef SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#endif
#ifndef SQLITE_OMIT_EXPLAIN
  ಯದಿ( p->explain ){
    rc = sqlite3VdbeList(p);
  }ಅನ್ಯಥಾ
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  ಯದಿ( rc==SQLITE_ROW ){
    ಪ್ರತಿಜ್ಞಾ( p->rc==SQLITE_OK );
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    ಪ್ರತ್ಯಾಗಮ SQLITE_ROW;
  }ಅನ್ಯಥಾ{
#ifndef SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#endif
    p->pResultRow = 0;
    ಯದಿ( rc==SQLITE_DONE && db->autoCommit ){
      ಪ್ರತಿಜ್ಞಾ( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      ಯದಿ( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }ಅನ್ಯಥಾ ಯದಿ( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  ಯದಿ( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    ಯದಿ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  ಪ್ರತಿಜ್ಞಾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  ಪ್ರತ್ಯಾಗಮ (rc&db->errMask);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_step(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  ಪೂರ್ಣಾಙ್ಕ cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  ಯದಿ( vdbeSafetyNotNull(v) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  ಯಾವತ್( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    ಪೂರ್ಣಾಙ್ಕ savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    ಯದಿ( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ಧ್ರುವ ವರ್ಣ *zErr = (ಧ್ರುವ ವರ್ಣ *)sqlite3_ಮೌಲ್ಯ_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_ಸಂದೇಶ);
      ಯದಿ( !db->mallocFailed ){
        v->z_Err_ಸಂದೇಶ = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } ಅನ್ಯಥಾ {
        v->z_Err_ಸಂದೇಶ = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      ಭಙ್ಗ;
    }
    sqlite3_reset(pStmt);
    ಯದಿ( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    ಪ್ರತಿಜ್ಞಾ( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
ಶೂನ್ಯ *sqlite3_user_data(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತಿಜ್ಞಾ( p && p->pFunc );
  ಪ್ರತ್ಯಾಗಮ p->pFunc->pUserData;
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
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( p && p->pOut );
#endif
  ಪ್ರತ್ಯಾಗಮ p->pOut->db;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_vtab_nochange(sqlite3_context *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( p );
#endif
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಮೌಲ್ಯ_List_Free(ಶೂನ್ಯ *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ_From_ಮೌಲ್ಯ_List(
  sqlite3_ಮೌಲ್ಯ *pVal,        /* Pointer to the ValueList object */
  sqlite3_ಮೌಲ್ಯ **ppOut,      /* Store the next value from the list here */
  ಪೂರ್ಣಾಙ್ಕ bNext                   /* 1 for _next(). 0 for _first() */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಮೌಲ್ಯ_List *pRhs;

  *ppOut = 0;
  ಯದಿ( pVal==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  ಯದಿ( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_ಮೌಲ್ಯ_List_Free ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    ಪ್ರತಿಜ್ಞಾ( pVal->eSubtype=='p' );
    ಪ್ರತಿಜ್ಞಾ( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (ಮೌಲ್ಯ_List*)pVal->z;
  }
  ಯದಿ( bNext ){
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    ಯದಿ( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  ಯದಿ( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, ಪರಿಮಾಣ(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    ಯದಿ( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_ಮೌಲ್ಯ *pOut = pRhs->pOut;
      ಪೂರ್ಣಾಙ್ಕ iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      ಯದಿ( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }ಅನ್ಯಥಾ{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_vtab_in_first(sqlite3_ಮೌಲ್ಯ *pVal, sqlite3_ಮೌಲ್ಯ **ppOut){
  ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ_From_ಮೌಲ್ಯ_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_vtab_in_next(sqlite3_ಮೌಲ್ಯ *pVal, sqlite3_ಮೌಲ್ಯ **ppOut){
  ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ_From_ಮೌಲ್ಯ_List(pVal, ppOut, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  ಪೂರ್ಣಾಙ್ಕ rc;
#ifndef SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  ಪ್ರತಿಜ್ಞಾ( p->pVdbe!=0 );
#ಅನ್ಯಥಾ
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#endif
  ಯದಿ( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    ಯದಿ( rc ) *piTime = 0;
  }
  ಪ್ರತ್ಯಾಗಮ *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *createAggContext(sqlite3_context *p, ಪೂರ್ಣಾಙ್ಕ nByte){
  Mem *pMem = p->pMem;
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Agg)==0 );
  ಯದಿ( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }ಅನ್ಯಥಾ{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    ಯದಿ( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
ಶೂನ್ಯ *sqlite3_aggregate_context(sqlite3_context *p, ಪೂರ್ಣಾಙ್ಕ nByte){
  ಪ್ರತಿಜ್ಞಾ( p && p->pFunc && p->pFunc->xFinalize );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  ಯದಿ( (p->pMem->flags & MEM_Agg)==0 ){
    ಪ್ರತ್ಯಾಗಮ createAggContext(p, nByte);
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)p->pMem->z;
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
ಶೂನ್ಯ *sqlite3_get_auxdata(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ iArg){
  AuxData *pAuxData;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ಯದಿ SQLITE_ENABLE_STAT4
  ಯದಿ( pCtx->pVdbe==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( pCtx->pVdbe!=0 );
#endif
  ಪ್ರತಿ(pAuxData=pCtx->pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    ಯದಿ(  pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      ಪ್ರತ್ಯಾಗಮ pAuxData->pAux;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಶೂನ್ಯ sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  ಪೂರ್ಣಾಙ್ಕ iArg,
  ಶೂನ್ಯ *pAux,
  ಶೂನ್ಯ (*xDelete)(ಶೂನ್ಯ*)
){
  AuxData *pAuxData;
  Vdbe *pVdbe;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCtx==0 ) ಪ್ರತ್ಯಾಗಮ;
#endif
  pVdbe= pCtx->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#ifdef SQLITE_ENABLE_STAT4
  ಯದಿ( pVdbe==0 ) ಗಚ್ಛ failed;
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( pVdbe!=0 );
#endif

  ಪ್ರತಿ(pAuxData=pVdbe->pAuxData; pAuxData; pAuxData=pAuxData->pNextAux){
    ಯದಿ( pAuxData->iAuxArg==iArg && (pAuxData->iAuxOp==pCtx->iOp || iArg<0) ){
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( pAuxData==0 ){
    pAuxData = sqlite3DbMallocZero(pVdbe->db, ಪರಿಮಾಣ(AuxData));
    ಯದಿ( !pAuxData ) ಗಚ್ಛ failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    ಯದಿ( pCtx->isError==0 ) pCtx->isError = -1;
  }ಅನ್ಯಥಾ ಯದಿ( pAuxData->xDeleteAux ){
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  ಪ್ರತ್ಯಾಗಮ;

failed:
  ಯದಿ( xDelete ){
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_aggregate_ಎಣಿಕೆ(sqlite3_context *p){
  ಪ್ರತಿಜ್ಞಾ( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  ಪ್ರತ್ಯಾಗಮ p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_column_ಎಣಿಕೆ(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ಯದಿ( pVm==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_data_ಎಣಿಕೆ(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ಯದಿ( pVm==0 || pVm->pResultRow==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ಸ್ಥಿರ ಧ್ರುವ Mem *column_Null_ಮೌಲ್ಯ(ಶೂನ್ಯ){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ಸ್ಥಿರ ಧ್ರುವ Mem nullMem
#ಯದಿ defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#endif
    = {
        /* .u          = */ {0},
        /* .z          = */ (ವರ್ಣ*)0,
        /* .n          = */ (ಪೂರ್ಣಾಙ್ಕ)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (ಪೂರ್ಣಾಙ್ಕ)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (ವರ್ಣ*)0,
        /* .xDel       = */ (ಶೂನ್ಯ(*)(ಶೂನ್ಯ*))0,
#ifdef SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#endif
      };
  ಪ್ರತ್ಯಾಗಮ &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ಸ್ಥಿರ Mem *columnMem(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  ಯದಿ( pVm==0 ) ಪ್ರತ್ಯಾಗಮ (Mem*)column_Null_ಮೌಲ್ಯ();
  ಪ್ರತಿಜ್ಞಾ( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  ಯದಿ( pVm->pResultRow!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->pResultRow[i];
  }ಅನ್ಯಥಾ{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_ಮೌಲ್ಯ();
  }
  ಪ್ರತ್ಯಾಗಮ pOut;
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
ಸ್ಥಿರ ಶೂನ್ಯ columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( p->db!=0 );
    ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_blob(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಧ್ರುವ ಶೂನ್ಯ *val;
  val = sqlite3_ಮೌಲ್ಯ_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_column_bytes(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಪೂರ್ಣಾಙ್ಕ val = sqlite3_ಮೌಲ್ಯ_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_column_bytes16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಪೂರ್ಣಾಙ್ಕ val = sqlite3_ಮೌಲ್ಯ_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
ದ್ವಿಗುಣ sqlite3_column_double(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ದ್ವಿಗುಣ val = sqlite3_ಮೌಲ್ಯ_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_column_int(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಪೂರ್ಣಾಙ್ಕ val = sqlite3_ಮೌಲ್ಯ_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  sqlite_int64 val = sqlite3_ಮೌಲ್ಯ_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
ಧ್ರುವ unsigned ವರ್ಣ *sqlite3_column_text(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಧ್ರುವ unsigned ವರ್ಣ *val = sqlite3_ಮೌಲ್ಯ_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
sqlite3_ಮೌಲ್ಯ *sqlite3_column_ಮೌಲ್ಯ(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  Mem *pOut = columnMem(pStmt, i);
  ಯದಿ( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ (sqlite3_ಮೌಲ್ಯ *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_text16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಧ್ರುವ ಶೂನ್ಯ *val = sqlite3_ಮೌಲ್ಯ_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ val;
}
#endif /* SQLITE_OMIT_UTF16 */
ಪೂರ್ಣಾಙ್ಕ sqlite3_column_type(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಪೂರ್ಣಾಙ್ಕ iType = sqlite3_ಮೌಲ್ಯ_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  ಪ್ರತ್ಯಾಗಮ iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ಸ್ಥಿರ ಧ್ರುವ u16 azExplainColNames16data[] = {
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
ಸ್ಥಿರ ಧ್ರುವ u8 iExplainColNames16[] = {
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
ಸ್ಥಿರ ಧ್ರುವ ಶೂನ್ಯ *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  ಪೂರ್ಣಾಙ್ಕ N,                   /* Which column to get the name for */
  ಪೂರ್ಣಾಙ್ಕ useUtf16,            /* True to return the name as UTF16 */
  ಪೂರ್ಣಾಙ್ಕ useType              /* What type of name */
){
  ಧ್ರುವ ಶೂನ್ಯ *ret;
  Vdbe *p;
  ಪೂರ್ಣಾಙ್ಕ n;
  sqlite3 *db;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pStmt==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಯದಿ( N<0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  ಯದಿ( p->explain ){
    ಯದಿ( useType>0 ) ಗಚ್ಛ columnName_end;
    n = p->explain==1 ? 8 : 4;
    ಯದಿ( N>=n ) ಗಚ್ಛ columnName_end;
    ಯದಿ( useUtf16 ){
      ಪೂರ್ಣಾಙ್ಕ i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (ಶೂನ್ಯ*)&azExplainColNames16data[i];
    }ಅನ್ಯಥಾ{
      ret = (ಶೂನ್ಯ*)azExplainColNames8[N + 8*p->explain - 8];
    }
    ಗಚ್ಛ columnName_end;
  }
  n = p->nResColumn;
  ಯದಿ( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#ifndef SQLITE_OMIT_UTF16
    ಯದಿ( useUtf16 ){
      ret = sqlite3_ಮೌಲ್ಯ_text16((sqlite3_ಮೌಲ್ಯ*)&p->aColName[N]);
    }ಅನ್ಯಥಾ
#endif
    {
      ret = sqlite3_ಮೌಲ್ಯ_text((sqlite3_ಮೌಲ್ಯ*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || db->mallocFailed==1 );
    ಯದಿ( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_column_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 0, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 1, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#ಯದಿ defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_column_decltype(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_decltype16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_column_database_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_database_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_column_table_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 0, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_table_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 1, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_column_origin_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
ಧ್ರುವ ಶೂನ್ಯ *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತ್ಯಾಗಮ columnName(pStmt, N, 1, COLNAME_COLUMN);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vdbeUnbind(Vdbe *p, unsigned ಪೂರ್ಣಾಙ್ಕ i){
  Mem *pVar;
  ಯದಿ( vdbeSafetyNotNull(p) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ಯದಿ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  ಯದಿ( i>=(unsigned ಪೂರ್ಣಾಙ್ಕ)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ಪ್ರತ್ಯಾಗಮ SQLITE_RANGE;
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
  ಪ್ರತಿಜ್ಞಾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ಪ್ರತಿಜ್ಞಾ( (p->expmask & p->smimask)==p->smimask );
  ಯದಿ( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    ಯದಿ( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  ಪೂರ್ಣಾಙ್ಕ i,                 /* Index of the parameter to bind */
  ಧ್ರುವ ಶೂನ್ಯ *zData,     /* Pointer to the data to be bound */
  i64 nData,             /* Number of bytes of data to be bound */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  ಪೂರ್ಣಾಙ್ಕ rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  ಯದಿ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    ಯದಿ( zData!=0 ){
      pVar = &p->aVar[i-1];
      ಯದಿ( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      }ಅನ್ಯಥಾ ಯದಿ( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        ಪ್ರತಿಜ್ಞಾ( zData==0
             || nData<0 
             || nData>pVar->db->aLimit[SQLITE_LIMIT_LENGTH]
             || ((u8*)zData)[nData]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= MEM_Term;
      }ಅನ್ಯಥಾ{
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        ಯದಿ( encoding==0 ) pVar->enc = ENC(p->db);
      }
      ಯದಿ( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      ಯದಿ( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }ಅನ್ಯಥಾ ಯದಿ( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((ಶೂನ್ಯ*)zData);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ i,
  ಧ್ರುವ ಶೂನ್ಯ *zData,
  ಪೂರ್ಣಾಙ್ಕ nData,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( nData<0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  ಪ್ರತ್ಯಾಗಮ bindText(pStmt, i, zData, nData, xDel, 0);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ i,
  ಧ್ರುವ ಶೂನ್ಯ *zData,
  sqlite3_uint64 nData,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಪ್ರತ್ಯಾಗಮ bindText(pStmt, i, zData, nData, xDel, 0);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_double(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i, ದ್ವಿಗುಣ r_ಮೌಲ್ಯ){
  ಪೂರ್ಣಾಙ್ಕ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಯದಿ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_ಮೌಲ್ಯ);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_int(sqlite3_stmt *p, ಪೂರ್ಣಾಙ್ಕ i, ಪೂರ್ಣಾಙ್ಕ i_ಮೌಲ್ಯ){
  ಪ್ರತ್ಯಾಗಮ sqlite3_bind_int64(p, i, (i64)i_ಮೌಲ್ಯ);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_int64(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i, sqlite_int64 i_ಮೌಲ್ಯ){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  ಯದಿ( vdbeSafetyNotNull(p) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ಯದಿ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  ಯದಿ( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ಪ್ರತ್ಯಾಗಮ SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  ಯದಿ( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    ಪ್ರತಿಜ್ಞಾ( (p->expmask & p->smimask)==p->smimask );
    ಯದಿ( (p->expmask & expireMask)!=0 ){
      ಯದಿ( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        ಪೂರ್ಣಾಙ್ಕ sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        ಪೂರ್ಣಾಙ್ಕ sm2 = i_ಮೌಲ್ಯ>=0 && i_ಮೌಲ್ಯ<=1;
        ಯದಿ( sm1!=sm2 ){
          p->expired = 1;
        }
      }ಅನ್ಯಥಾ ಯದಿ( (pVar->flags & MEM_Int)==0 || i_ಮೌಲ್ಯ!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_ಮೌಲ್ಯ);
  sqlite3_mutex_leave(p->db->mutex);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_null(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  ಪೂರ್ಣಾಙ್ಕ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಯದಿ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ i,
  ಶೂನ್ಯ *pPtr,
  ಧ್ರುವ ವರ್ಣ *zPTtype,
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಯದಿ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }ಅನ್ಯಥಾ ಯದಿ( xDestructor ){
    xDestructor(pPtr);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ i,
  ಧ್ರುವ ವರ್ಣ *zData,
  ಪೂರ್ಣಾಙ್ಕ nData,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
  ಪ್ರತ್ಯಾಗಮ bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ i,
  ಧ್ರುವ ವರ್ಣ *zData,
  sqlite3_uint64 nData,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*),
  unsigned ವರ್ಣ enc
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಯದಿ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ಯದಿ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    nData &= ~(u64)1;
  }
  ಪ್ರತ್ಯಾಗಮ bindText(pStmt, i, zData, nData, xDel, enc);
}
#ifndef SQLITE_OMIT_UTF16
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ i,
  ಧ್ರುವ ಶೂನ್ಯ *zData,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
  ಪ್ರತ್ಯಾಗಮ bindText(pStmt, i, zData, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_ಮೌಲ್ಯ(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i, ಧ್ರುವ sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type((sqlite3_ಮೌಲ್ಯ*)p_ಮೌಲ್ಯ) ){
    ಪಕ್ಷ SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_ಮೌಲ್ಯ->u.i);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_FLOAT: {
      ಪ್ರತಿಜ್ಞಾ( p_ಮೌಲ್ಯ->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_ಮೌಲ್ಯ->flags & MEM_Real) ? p_ಮೌಲ್ಯ->u.r : (ದ್ವಿಗುಣ)p_ಮೌಲ್ಯ->u.i
      );
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_BLOB: {
      ಯದಿ( p_ಮೌಲ್ಯ->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_ಮೌಲ್ಯ->u.nZero);
      }ಅನ್ಯಥಾ{
        rc = sqlite3_bind_blob(pStmt, i, p_ಮೌಲ್ಯ->z, p_ಮೌಲ್ಯ->n,SQLITE_TRANSIENT);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_ಮೌಲ್ಯ->z, p_ಮೌಲ್ಯ->n, SQLITE_TRANSIENT,
                              p_ಮೌಲ್ಯ->enc);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      rc = sqlite3_bind_null(pStmt, i);
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i, ಪೂರ್ಣಾಙ್ಕ n){
  ಪೂರ್ಣಾಙ್ಕ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಯದಿ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ifndef SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#ಅನ್ಯಥಾ
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#endif
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i, sqlite3_uint64 n){
  ಪೂರ್ಣಾಙ್ಕ rc;
  Vdbe *p = (Vdbe *)pStmt;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->db->mutex);
  ಯದಿ( n>(u64)p->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    rc = SQLITE_TOOBIG;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_parameter_ಎಣಿಕೆ(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ಪ್ರತ್ಯಾಗಮ p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ i){
  Vdbe *p = (Vdbe*)pStmt;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ sqlite3_VList_ಅಂಕೆ_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3VdbeParameterIndex(Vdbe *p, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ nName){
  ಯದಿ( p==0 || zName==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ sqlite3_VList_Name_To_ಅಂಕೆ(p->pVList, zName, nName);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, ಧ್ರುವ ವರ್ಣ *zName){
  ಪ್ರತ್ಯಾಗಮ sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( pTo->db==pFrom->db );
  ಪ್ರತಿಜ್ಞಾ( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  ಪ್ರತಿ(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  ಯದಿ( pFrom->nVar!=pTo->nVar ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  ಪ್ರತಿಜ್ಞಾ( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  ಯದಿ( pTo->expmask ){
    pTo->expired = 1;
  }
  ಪ್ರತಿಜ್ಞಾ( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  ಯದಿ( pFrom->expmask ){
    pFrom->expired = 1;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  ಪ್ರತ್ಯಾಗಮ pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  ಪ್ರತ್ಯಾಗಮ pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  ಪ್ರತ್ಯಾಗಮ pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_explain(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ eMode){
  Vdbe *v = (Vdbe*)pStmt;
  ಪೂರ್ಣಾಙ್ಕ rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pStmt==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(v->db->mutex);
  ಯದಿ( ((ಪೂರ್ಣಾಙ್ಕ)v->explain)==eMode ){
    rc = SQLITE_OK;
  }ಅನ್ಯಥಾ ಯದಿ( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ ಯದಿ( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ ಯದಿ( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }ಅನ್ಯಥಾ ಯದಿ( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    ಯದಿ( rc ){
      v->explain = savedMode;
    }ಅನ್ಯಥಾ{
      v->haveEqpOps = eMode==2;
    }
  }
  ಯದಿ( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }ಅನ್ಯಥಾ{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  ಪ್ರತ್ಯಾಗಮ v!=0 && v->eVdbeState==VDBE_RUN_STATE;
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
  ಯದಿ( !sqlite3SafetyCheckOk(pDb) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  sqlite3_mutex_enter(pDb->mutex);
  ಯದಿ( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }ಅನ್ಯಥಾ{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  ಪ್ರತ್ಯಾಗಮ pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_status(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಙ್ಕ op, ಪೂರ್ಣಾಙ್ಕ resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=ArraySize(pVdbe->aCounter)))
  ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಯದಿ( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (ಪೂರ್ಣಾಙ್ಕ*)&v;
    ಪ್ರತಿಜ್ಞಾ( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }ಅನ್ಯಥಾ{
    v = pVdbe->aCounter[op];
    ಯದಿ( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ಪ್ರತ್ಯಾಗಮ p ? p->zSql : 0;
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
ವರ್ಣ *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ifdef SQLITE_OMIT_TRACE
  ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
  ವರ್ಣ *z = 0;
  ಧ್ರುವ ವರ್ಣ *zSql = sqlite3_sql(pStmt);
  ಯದಿ( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ z;
#endif
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಪ್ರತ್ಯಾಗಮ p->zNormSql;
}
#endif /* SQLITE_ENABLE_NORMALIZE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ಸ್ಥಿರ UnpackedRecord *vdbeUnpackRecord(
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,
  ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ,
  ಧ್ರುವ ಶೂನ್ಯ *p_ಕೀಲಿ
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_ಕೀಲಿ_Info);
  ಯದಿ( pRet ){
    memset(pRet->aMem, 0, ಪರಿಮಾಣ(Mem)*(p_ಕೀಲಿ_Info->n_ಕೀಲಿ_Field+1));
    sqlite3VdbeRecordUnpack(n_ಕೀಲಿ, p_ಕೀಲಿ, pRet);
  }
  ಪ್ರತ್ಯಾಗಮ pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_preupdate_old(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ iIdx, sqlite3_ಮೌಲ್ಯ **pp_ಮೌಲ್ಯ){
  PreUpdate *p;
  Mem *pMem;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( db==0 || pp_ಮೌಲ್ಯ==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  ಯದಿ( !p || p->op==SQLITE_ಸೇರಿಸು ){
    rc = SQLITE_MISUSE_BKPT;
    ಗಚ್ಛ preupdate_old_out;
  }
  ಯದಿ( p->pPk ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }ಅನ್ಯಥಾ ಯದಿ( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    ಗಚ್ಛ preupdate_old_out;
  }ಅನ್ಯಥಾ{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  ಯದಿ( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    ಗಚ್ಛ preupdate_old_out;
  }

  ಯದಿ( iIdx==p->pTab->iPKey ){
    *pp_ಮೌಲ್ಯ = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }ಅನ್ಯಥಾ{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    ಯದಿ( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      ಪ್ರತಿಜ್ಞಾ( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3BtreePayloadSize(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      ಯದಿ( !aRec ) ಗಚ್ಛ preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      ಯದಿ( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        ಯದಿ( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      ಯದಿ( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        ಗಚ್ಛ preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_ಮೌಲ್ಯ = &p->pUnpacked->aMem[iStore];
    ಯದಿ( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      ಯದಿ( pCol->iDflt>0 ){
        ಯದಿ( p->apDflt==0 ){
          ಪೂರ್ಣಾಙ್ಕ nByte;
          ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(sqlite3_ಮೌಲ್ಯ*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = ಪರಿಮಾಣ(sqlite3_ಮೌಲ್ಯ*)*p->pTab->nCol;
          p->apDflt = (sqlite3_ಮೌಲ್ಯ**)sqlite3DbMallocZero(db, nByte);
          ಯದಿ( p->apDflt==0 ) ಗಚ್ಛ preupdate_old_out;
        }
        ಯದಿ( p->apDflt[iIdx]==0 ){
          sqlite3_ಮೌಲ್ಯ *pVal = 0;
          ಅಭಿವ್ಯಕ್ತಿ *pDflt;
          ಪ್ರತಿಜ್ಞಾ( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_ಅಭಿವ್ಯಕ್ತಿ;
          rc = sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, pDflt, ENC(db), pCol->affinity, &pVal);
          ಯದಿ( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_ಮೌಲ್ಯ = p->apDflt[iIdx];
      }ಅನ್ಯಥಾ{
        *pp_ಮೌಲ್ಯ = (sqlite3_ಮೌಲ್ಯ *)column_Null_ಮೌಲ್ಯ();
      }
    }ಅನ್ಯಥಾ ಯದಿ( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      ಯದಿ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  ಪ್ರತ್ಯಾಗಮ sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_preupdate_ಎಣಿಕೆ(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ಅನ್ಯಥಾ
  p = db->pPreUpdate;
#endif
  ಪ್ರತ್ಯಾಗಮ (p ? p->pKeyinfo->n_ಕೀಲಿ_Field : 0);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_preupdate_depth(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ಅನ್ಯಥಾ
  p = db->pPreUpdate;
#endif
  ಪ್ರತ್ಯಾಗಮ (p ? p->v->nFrame : 0);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ಅನ್ಯಥಾ
  p = db->pPreUpdate;
#endif
  ಪ್ರತ್ಯಾಗಮ (p ? p->iBlobWrite : -1);
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_preupdate_new(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ iIdx, sqlite3_ಮೌಲ್ಯ **pp_ಮೌಲ್ಯ){
  PreUpdate *p;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  Mem *pMem;
  ಪೂರ್ಣಾಙ್ಕ iStore = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( db==0 || pp_ಮೌಲ್ಯ==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
#endif
  p = db->pPreUpdate;
  ಯದಿ( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    ಗಚ್ಛ preupdate_new_out;
  }
  ಯದಿ( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3TableColumnToIndex(p->pPk, iIdx);
  }ಅನ್ಯಥಾ ಯದಿ( iIdx >= p->pTab->nCol ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }ಅನ್ಯಥಾ{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  ಯದಿ( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    ಗಚ್ಛ preupdate_new_out;
  }

  ಯದಿ( p->op==SQLITE_ಸೇರಿಸು ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    ಯದಿ( !pUnpack ){
      Mem *pData = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(pData);
      ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, pData->n, pData->z);
      ಯದಿ( !pUnpack ){
        rc = SQLITE_NOMEM;
        ಗಚ್ಛ preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    ಯದಿ( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }ಅನ್ಯಥಾ ಯದಿ( iStore>=pUnpack->nField ){
      pMem = (sqlite3_ಮೌಲ್ಯ *)column_Null_ಮೌಲ್ಯ();
    }ಅನ್ಯಥಾ ಯದಿ( (p->pTab->aCol)
           && (p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }ಅನ್ಯಥಾ{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    ಪ್ರತಿಜ್ಞಾ( p->op==SQLITE_UPDATE );
    ಯದಿ( !p->aNew ){
      ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Mem)*p->pCsr->nField);
      ಯದಿ( !p->aNew ){
        rc = SQLITE_NOMEM;
        ಗಚ್ಛ preupdate_new_out;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    ಯದಿ( pMem->flags==0 ){
      ಯದಿ( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }ಅನ್ಯಥಾ{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ preupdate_new_out;
      }
    }
  }
  *pp_ಮೌಲ್ಯ = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  ಪ್ರತ್ಯಾಗಮ sqlite3ApiExit(db, rc);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ಪೂರ್ಣಾಙ್ಕ iScan,                      /* Index of loop to report on */
  ಪೂರ್ಣಾಙ್ಕ iScanStatusOp,              /* Which metric to return */
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಶೂನ್ಯ *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  ಪೂರ್ಣಾಙ್ಕ nOp;
  ScanStatus *pScan = 0;
  ಪೂರ್ಣಾಙ್ಕ idx;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
#endif
  aOp = p->aOp;
  nOp = p->nOp;
  ಯದಿ( p->pFrame ){
    VdbeFrame *pFrame;
    ಪ್ರತಿ(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  ಯದಿ( iScan<0 ){
    ಪೂರ್ಣಾಙ್ಕ ii;
    ಯದಿ( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      ಪ್ರತಿ(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }ಅನ್ಯಥಾ{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    ಪ್ರತಿ(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      ಯದಿ( pScan->zName ){
        iScan--;
        ಯದಿ( iScan<0 ) ಭಙ್ಗ;
      }
    }
  }
  ಯದಿ( idx>=p->nScan ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತಿಜ್ಞಾ( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  ವಿಕಲ್ಪ( iScanStatusOp ){
    ಪಕ್ಷ SQLITE_SCANSTAT_NLOOP: {
      ಯದಿ( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }ಅನ್ಯಥಾ{
        *(sqlite3_int64*)pOut = -1;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_NVISIT: {
      ಯದಿ( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }ಅನ್ಯಥಾ{
        *(sqlite3_int64*)pOut = -1;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_EST: {
      ದ್ವಿಗುಣ r = 1.0;
      LogEst x = pScan->nEst;
      ಯಾವತ್( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(ದ್ವಿಗುಣ*)pOut = r*sqlite3LogEstToInt(x);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_NAME: {
      *(ಧ್ರುವ ವರ್ಣ**)pOut = pScan->zName;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_EXPLAIN: {
      ಯದಿ( pScan->addrExplain ){
        *(ಧ್ರುವ ವರ್ಣ**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }ಅನ್ಯಥಾ{
        *(ಧ್ರುವ ವರ್ಣ**)pOut = 0;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_SELECTID: {
      ಯದಿ( pScan->addrExplain ){
        *(ಪೂರ್ಣಾಙ್ಕ*)pOut = aOp[ pScan->addrExplain ].p1;
      }ಅನ್ಯಥಾ{
        *(ಪೂರ್ಣಾಙ್ಕ*)pOut = -1;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_PARENTID: {
      ಯದಿ( pScan->addrExplain ){
        *(ಪೂರ್ಣಾಙ್ಕ*)pOut = aOp[ pScan->addrExplain ].p2;
      }ಅನ್ಯಥಾ{
        *(ಪೂರ್ಣಾಙ್ಕ*)pOut = -1;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      ಯದಿ( pScan->aAddrRange[0]==0 ){
        res = -1;
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ ii;
        ಪ್ರತಿ(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
          ಪೂರ್ಣಾಙ್ಕ iIns = pScan->aAddrRange[ii];
          ಪೂರ್ಣಾಙ್ಕ iEnd = pScan->aAddrRange[ii+1];
          ಯದಿ( iIns==0 ) ಭಙ್ಗ;
          ಯದಿ( iIns>0 ){
            ಯಾವತ್( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }ಅನ್ಯಥಾ{
            ಪೂರ್ಣಾಙ್ಕ iOp;
            ಪ್ರತಿ(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              ಯದಿ( pOp->p1!=iEnd ) ಅನುವರ್ತ;
              ಯದಿ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                ಅನುವರ್ತ;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ಪೂರ್ಣಾಙ್ಕ iScan,                      /* Index of loop to report on */
  ಪೂರ್ಣಾಙ್ಕ iScanStatusOp,              /* Which metric to return */
  ಶೂನ್ಯ *pOut                      /* OUT: Write the answer here */
){
  ಪ್ರತ್ಯಾಗಮ sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
ಶೂನ್ಯ sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪ್ರತಿ(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#endif /* SQLITE_ENABLE_STMT_SCANSTATUS */
