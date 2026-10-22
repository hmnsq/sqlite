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
#समावेश "sqliteInt.h"
#समावेश "vdbeInt.h"
#समावेश "opcodes.h"

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
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
  अगर( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  वापस iRet;
}
#पूर्ण_अगर

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
अटल पूर्णांक vdbeSafety(Vdbe *p){
  अगर( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    वापस 1;
  }अन्यथा{
    वापस 0;
  }
}
अटल पूर्णांक vdbeSafetyNotNull(Vdbe *p){
  अगर( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    वापस 1;
  }अन्यथा{
    वापस vdbeSafety(p);
  }
}

#अगर_अघोषित SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
अटल SQLITE_NOINLINE व्योम invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  निश्चित( p->startTime>0 );
  निश्चित( db->init.busy==0 );
  निश्चित( p->zSql!=0 );
  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime)*1000000;
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
  अगर( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#पूर्ण_अगर
  अगर( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (व्योम*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   अगर( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#अन्यथा
# define checkProfileCallback(DB,P)  /*no-op*/
#पूर्ण_अगर

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
  अगर( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }अन्यथा{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    अगर( vdbeSafety(v) ) वापस SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    निश्चित( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  वापस rc;
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
  अगर( pStmt==0 ){
    rc = SQLITE_OK;
  }अन्यथा{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    निश्चित( (rc & (db->errMask))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  वापस rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
पूर्णांक sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  पूर्णांक i;
  पूर्णांक rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 maskSmallInt = 0;  /* Bitmask of bindings with value 0 or 1 */
#अगर SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pStmt==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
#अगर SQLITE_THREADSAFE
  mutex = p->db->mutex;
#पूर्ण_अगर

  sqlite3_mutex_enter(mutex);
  क्रम(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    अगर( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      maskSmallInt |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  निश्चित( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  अगर( (p->expmask&~p->smimask)!=0 || (p->smimask&maskSmallInt)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  वापस rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
स्थिर व्योम *sqlite3_मूल्य_blob(sqlite3_मूल्य *pVal){
  Mem *p = (Mem*)pVal;
  अगर( p->flags & (MEM_Blob|MEM_Str) ){
    अगर( ExpandBlob(p)!=SQLITE_OK ){
      निश्चित( p->flags==MEM_Null && p->z==0 );
      वापस 0;
    }
    p->flags |= MEM_Blob;
    वापस p->n ? p->z : 0;
  }अन्यथा{
    वापस sqlite3_मूल्य_text(pVal);
  }
}
पूर्णांक sqlite3_मूल्य_bytes(sqlite3_मूल्य *pVal){
  वापस sqlite3_मूल्य_Bytes(pVal, SQLITE_UTF8);
}
पूर्णांक sqlite3_मूल्य_bytes16(sqlite3_मूल्य *pVal){
  वापस sqlite3_मूल्य_Bytes(pVal, SQLITE_UTF16NATIVE);
}
द्विगुन sqlite3_मूल्य_double(sqlite3_मूल्य *pVal){
  वापस sqlite3_Vdbe_Real_मूल्य((Mem*)pVal);
}
पूर्णांक sqlite3_मूल्य_int(sqlite3_मूल्य *pVal){
  वापस (पूर्णांक)sqlite3_Vdbe_Int_मूल्य((Mem*)pVal);
}
sqlite_int64 sqlite3_मूल्य_int64(sqlite3_मूल्य *pVal){
  वापस sqlite3_Vdbe_Int_मूल्य((Mem*)pVal);
}
अचिन्हित पूर्णांक sqlite3_मूल्य_subtype(sqlite3_मूल्य *pVal){
  Mem *pMem = (Mem*)pVal;
  वापस ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
व्योम *sqlite3_मूल्य_pointer(sqlite3_मूल्य *pVal, स्थिर अक्षर *zPType){
  Mem *p = (Mem*)pVal;
  अगर( (p->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && म_भेद(p->u.zPType, zPType)==0
  ){
    वापस (व्योम*)p->z;
  }अन्यथा{
    वापस 0;
  }
}
स्थिर अचिन्हित अक्षर *sqlite3_मूल्य_text(sqlite3_मूल्य *pVal){
  वापस (स्थिर अचिन्हित अक्षर *)sqlite3_मूल्य_Text(pVal, SQLITE_UTF8);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_मूल्य_text16(sqlite3_मूल्य* pVal){
  वापस sqlite3_मूल्य_Text(pVal, SQLITE_UTF16NATIVE);
}
स्थिर व्योम *sqlite3_मूल्य_text16be(sqlite3_मूल्य *pVal){
  वापस sqlite3_मूल्य_Text(pVal, SQLITE_UTF16BE);
}
स्थिर व्योम *sqlite3_मूल्य_text16le(sqlite3_मूल्य *pVal){
  वापस sqlite3_मूल्य_Text(pVal, SQLITE_UTF16LE);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
पूर्णांक sqlite3_मूल्य_type(sqlite3_मूल्य* pVal){
  अटल स्थिर u8 aType[] = {
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
#अगर_घोषित SQLITE_DEBUG
  {
    पूर्णांक eType = SQLITE_BLOB;
    अगर( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }अन्यथा अगर( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }अन्यथा अगर( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }अन्यथा अगर( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    निश्चित( eType == aType[pVal->flags&MEM_AffMask] );
  }
#पूर्ण_अगर
  वापस aType[pVal->flags&MEM_AffMask];
}
पूर्णांक sqlite3_मूल्य_encoding(sqlite3_मूल्य *pVal){
  वापस pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
पूर्णांक sqlite3_मूल्य_nochange(sqlite3_मूल्य *pVal){
  वापस (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
पूर्णांक sqlite3_मूल्य_frombind(sqlite3_मूल्य *pVal){
  वापस (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_मूल्य *sqlite3_मूल्य_dup(स्थिर sqlite3_मूल्य *pOrig){
  sqlite3_मूल्य *pNew;
  अगर( pOrig==0 ) वापस 0;
  pNew = sqlite3_malloc( माप(*pNew) );
  अगर( pNew==0 ) वापस 0;
  स_रखो(pNew, 0, माप(*pNew));
  स_नकल(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  अगर( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    अगर( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_मूल्य_Free(pNew);
      pNew = 0;
    }
  }अन्यथा अगर( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_Term|MEM_Subtype);
  }
  वापस pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
व्योम sqlite3_मूल्य_free(sqlite3_मूल्य *pOld){
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
अटल व्योम set_परिणाम_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  स्थिर अक्षर *z,          /* String pointer */
  पूर्णांक n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  व्योम (*xDel)(व्योम*)     /* Destructor function */
){
  Mem *pOut = pCtx->pOut;
  पूर्णांक rc;
  अगर( enc==SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  }अन्यथा अगर( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    निश्चित( z==0
         || n<0 
         || n>pOut->db->aLimit[SQLITE_LIMIT_लंबाई]
         || z[n]==0
    );
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= MEM_Term;
  }अन्यथा{
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  अगर( rc ){
    अगर( rc==SQLITE_TOOBIG ){
      sqlite3_परिणाम_error_toobig(pCtx);
    }अन्यथा{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      निश्चित( rc==SQLITE_NOMEM );
      sqlite3_परिणाम_error_nomem(pCtx);
    }
    वापस;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  अगर( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_परिणाम_error_toobig(pCtx);
  }
}
अटल पूर्णांक invoke_मूल्य_Destructor(
  स्थिर व्योम *p,             /* Value to destroy */
  व्योम (*xDel)(व्योम*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  निश्चित( xDel!=SQLITE_DYNAMIC );
  अगर( xDel==0 ){
    /* noop */
  }अन्यथा अगर( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }अन्यथा{
    xDel((व्योम*)p);
  }
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx!=0 ){
    sqlite3_परिणाम_error_toobig(pCtx);
  }
#अन्यथा
  निश्चित( pCtx!=0 );
  sqlite3_परिणाम_error_toobig(pCtx);
#पूर्ण_अगर
  वापस SQLITE_TOOBIG;
}
व्योम sqlite3_परिणाम_blob(
  sqlite3_context *pCtx,
  स्थिर व्योम *z,
  पूर्णांक n,
  व्योम (*xDel)(व्योम *)
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 || n<0 ){
    invoke_मूल्य_Destructor(z, xDel, pCtx);
    वापस;
  }
#पूर्ण_अगर
  निश्चित( n>=0 );
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_परिणाम_Str_Or_Error(pCtx, z, n, 0, xDel);
}
व्योम sqlite3_परिणाम_blob64(
  sqlite3_context *pCtx,
  स्थिर व्योम *z,
  sqlite3_uint64 n,
  व्योम (*xDel)(व्योम *)
){
  निश्चित( xDel!=SQLITE_DYNAMIC );
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ){
    invoke_मूल्य_Destructor(z, xDel, 0);
    वापस;
  }
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  अगर( n>0x7fffffff ){
    (व्योम)invoke_मूल्य_Destructor(z, xDel, pCtx);
  }अन्यथा{
    set_परिणाम_Str_Or_Error(pCtx, z, (पूर्णांक)n, 0, xDel);
  }
}
व्योम sqlite3_परिणाम_double(sqlite3_context *pCtx, द्विगुन rVal){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetDouble(pCtx->pOut, rVal);
}
व्योम sqlite3_परिणाम_error(sqlite3_context *pCtx, स्थिर अक्षर *z, पूर्णांक n){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
व्योम sqlite3_परिणाम_error16(sqlite3_context *pCtx, स्थिर व्योम *z, पूर्णांक n){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#पूर्ण_अगर
व्योम sqlite3_परिणाम_int(sqlite3_context *pCtx, पूर्णांक iVal){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal);
}
व्योम sqlite3_परिणाम_int64(sqlite3_context *pCtx, i64 iVal){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetInt64(pCtx->pOut, iVal);
}
व्योम sqlite3_परिणाम_null(sqlite3_context *pCtx){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemSetNull(pCtx->pOut);
}
व्योम sqlite3_परिणाम_pointer(
  sqlite3_context *pCtx,
  व्योम *pPtr,
  स्थिर अक्षर *zPType,
  व्योम (*xDestructor)(व्योम*)
){
  Mem *pOut;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ){
    invoke_मूल्य_Destructor(pPtr, xDestructor, 0);
    वापस;
  }
#पूर्ण_अगर
  pOut = pCtx->pOut;
  निश्चित( sqlite3_mutex_held(pOut->db->mutex) );
  sqlite3VdbeMemRelease(pOut);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}
व्योम sqlite3_परिणाम_subtype(sqlite3_context *pCtx, अचिन्हित पूर्णांक eSubtype){
  Mem *pOut;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
#अगर defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  अगर( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_परिणाम_SUBTYPE)==0
  ){
    अक्षर zErr[200];
    sqlite3_snprintf(माप(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_परिणाम_error(pCtx, zErr, -1);
    वापस;
  }
#पूर्ण_अगर /* SQLITE_STRICT_SUBTYPE */
  pOut = pCtx->pOut;
  निश्चित( sqlite3_mutex_held(pOut->db->mutex) );
  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= MEM_Subtype;
}
व्योम sqlite3_परिणाम_text(
  sqlite3_context *pCtx,
  स्थिर अक्षर *z,
  पूर्णांक n,
  व्योम (*xDel)(व्योम *)
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ){
    invoke_मूल्य_Destructor(z, xDel, 0);
    वापस;
  }
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_परिणाम_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
व्योम sqlite3_परिणाम_text64(
  sqlite3_context *pCtx,
  स्थिर अक्षर *z,
  sqlite3_uint64 n,
  व्योम (*xDel)(व्योम *),
  अचिन्हित अक्षर enc
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ){
    invoke_मूल्य_Destructor(z, xDel, 0);
    वापस;
  }
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  निश्चित( xDel!=SQLITE_DYNAMIC );
  अगर( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    अगर( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  अगर( n>0x7fffffff ){
    (व्योम)invoke_मूल्य_Destructor(z, xDel, pCtx);
  }अन्यथा{
    set_परिणाम_Str_Or_Error(pCtx, z, (पूर्णांक)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}
#अगर_अघोषित SQLITE_OMIT_UTF16
व्योम sqlite3_परिणाम_text16(
  sqlite3_context *pCtx,
  स्थिर व्योम *z,
  पूर्णांक n,
  व्योम (*xDel)(व्योम *)
){
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_परिणाम_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
व्योम sqlite3_परिणाम_text16be(
  sqlite3_context *pCtx,
  स्थिर व्योम *z,
  पूर्णांक n,
  व्योम (*xDel)(व्योम *)
){
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_परिणाम_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
व्योम sqlite3_परिणाम_text16le(
  sqlite3_context *pCtx,
  स्थिर व्योम *z,
  पूर्णांक n,
  व्योम (*xDel)(व्योम *)
){
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  set_परिणाम_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
व्योम sqlite3_परिणाम_मूल्य(sqlite3_context *pCtx, sqlite3_मूल्य *p_मूल्य){
  Mem *pOut;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
  अगर( p_मूल्य==0 ){
    sqlite3_परिणाम_null(pCtx);
    वापस;
  }
#पूर्ण_अगर
  pOut = pCtx->pOut;
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  sqlite3VdbeMemCopy(pOut, p_मूल्य);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  अगर( sqlite3VdbeMemTooBig(pOut) ){
    sqlite3_परिणाम_error_toobig(pCtx);
  }
}
व्योम sqlite3_परिणाम_zeroblob(sqlite3_context *pCtx, पूर्णांक n){
  sqlite3_परिणाम_zeroblob64(pCtx, n>0 ? n : 0);
}
पूर्णांक sqlite3_परिणाम_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *pOut;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  pOut = pCtx->pOut;
  निश्चित( sqlite3_mutex_held(pOut->db->mutex) );
  अगर( n>(u64)pOut->db->aLimit[SQLITE_LIMIT_लंबाई] ){
    sqlite3_परिणाम_error_toobig(pCtx);
    वापस SQLITE_TOOBIG;
  }
#अगर_अघोषित SQLITE_OMIT_INCRBLOB
  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (पूर्णांक)n);
  वापस SQLITE_OK;
#अन्यथा
  वापस sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (पूर्णांक)n);
#पूर्ण_अगर
}
व्योम sqlite3_परिणाम_error_code(sqlite3_context *pCtx, पूर्णांक errCode){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  pCtx->isError = errCode ? errCode : -1;
#अगर_घोषित SQLITE_DEBUG
  अगर( pCtx->pVdbe ) pCtx->pVdbe->rcApp = errCode;
#पूर्ण_अगर
  अगर( pCtx->pOut->flags & MEM_Null ){
    set_परिणाम_Str_Or_Error(pCtx, sqlite3ErrStr(errCode), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
व्योम sqlite3_परिणाम_error_toobig(sqlite3_context *pCtx){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
व्योम sqlite3_परिणाम_error_nomem(sqlite3_context *pCtx){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
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
व्योम sqlite3_परिणाम_str(sqlite3_context *pCtx, sqlite3_str *pStr, पूर्णांक eOwn){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
  अगर( pStr==0 ) वापस;
#पूर्ण_अगर
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  अगर( pStr->accError==0 ){
    अगर( pStr->nChar==0 ){
      set_परिणाम_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }अन्यथा{
      स्थिर अक्षर *zText = sqlite3_str_मूल्य(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      निश्चित( isMalloced(pStr) );
      अगर( eOwn==SQLITE_COPY ){
        set_परिणाम_Str_Or_Error(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }अन्यथा{
        set_परिणाम_Str_Or_Error(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }अन्यथा अगर( pStr->accError==SQLITE_NOMEM ){
    sqlite3_परिणाम_error_nomem(pCtx);
  }अन्यथा{
    निश्चित( pStr->accError==SQLITE_TOOBIG );
    sqlite3_परिणाम_error_toobig(pCtx);
  }
  अगर( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#अगर_अघोषित SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
व्योम sqlite3_परिणाम_Int_Real(sqlite3_context *pCtx){
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
  अगर( pCtx->pOut->flags & MEM_Int ){
    pCtx->pOut->flags &= ~MEM_Int;
    pCtx->pOut->flags |= MEM_IntReal;
  }
}
#पूर्ण_अगर


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
अटल पूर्णांक doWalCallbacks(sqlite3 *db){
  पूर्णांक rc = SQLITE_OK;
#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक i;
  क्रम(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    अगर( pBt ){
      पूर्णांक nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      अगर( nEntry>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }
#अन्यथा
  UNUSED_PARAMETER(db);
#पूर्ण_अगर
  वापस rc;
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
अटल पूर्णांक sqlite3Step(Vdbe *p){
  sqlite3 *db;
  पूर्णांक rc;

  निश्चित(p);
  db = p->db;
  अगर( p->eVdbeState!=VDBE_RUN_STATE ){
    restart_step:
    अगर( p->eVdbeState==VDBE_READY_STATE ){
      अगर( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        अगर( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        जाओ end_of_step;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      अगर( db->nVdbeActive==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      निश्चित( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#अगर_अघोषित SQLITE_OMIT_TRACE
      अगर( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }अन्यथा{
        निश्चित( p->startTime==0 );
      }
#पूर्ण_अगर

      db->nVdbeActive++;
      अगर( p->readOnly==0 ) db->nVdbeWrite++;
      अगर( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_RUN_STATE;
    }अन्यथा

    अगर( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
#अगर_घोषित SQLITE_OMIT_AUTORESET
      अगर( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }अन्यथा{
        वापस SQLITE_MISUSE_BKPT;
      }
#अन्यथा
      sqlite3_reset((sqlite3_stmt*)p);
#पूर्ण_अगर
      निश्चित( p->eVdbeState==VDBE_READY_STATE );
      जाओ restart_step;
    }
  }

#अगर_घोषित SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_EXPLAIN
  अगर( p->explain ){
    rc = sqlite3VdbeList(p);
  }अन्यथा
#पूर्ण_अगर /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  अगर( rc==SQLITE_ROW ){
    निश्चित( p->rc==SQLITE_OK );
    निश्चित( db->mallocFailed==0 );
    db->errCode = SQLITE_ROW;
    वापस SQLITE_ROW;
  }अन्यथा{
#अगर_अघोषित SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#पूर्ण_अगर
    p->p_परिणाम_Row = 0;
    अगर( rc==SQLITE_DONE && db->autoCommit ){
      निश्चित( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      अगर( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }अन्यथा अगर( rc!=SQLITE_DONE && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  अगर( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    अगर( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
end_of_step:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  निश्चित( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  वापस (rc&db->errMask);
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

  अगर( vdbeSafetyNotNull(v) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  जबतक( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    पूर्णांक savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    अगर( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      स्थिर अक्षर *zErr = (स्थिर अक्षर *)sqlite3_मूल्य_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_संदेश);
      अगर( !db->mallocFailed ){
        v->z_Err_संदेश = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } अन्यथा {
        v->z_Err_संदेश = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      अवरोध;
    }
    sqlite3_reset(pStmt);
    अगर( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    निश्चित( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
व्योम *sqlite3_user_तथ्य(sqlite3_context *p){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( p==0 ) वापस 0;
#पूर्ण_अगर
  निश्चित( p && p->pFunc );
  वापस p->pFunc->p_User_तथ्य;
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
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( p==0 ) वापस 0;
#अन्यथा
  निश्चित( p && p->pOut );
#पूर्ण_अगर
  वापस p->pOut->db;
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
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( p==0 ) वापस 0;
#अन्यथा
  निश्चित( p );
#पूर्ण_अगर
  वापस sqlite3_मूल्य_nochange(p->pOut);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
व्योम sqlite3_Vdbe_मूल्य_List_Free(व्योम *pToDelete){
  sqlite3_free(pToDelete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
अटल पूर्णांक मूल्य_From_मूल्य_List(
  sqlite3_मूल्य *pVal,        /* Pointer to the ValueList object */
  sqlite3_मूल्य **ppOut,      /* Store the next value from the list here */
  पूर्णांक b_अग्र                   /* 1 for _next(). 0 for _first() */
){
  पूर्णांक rc;
  मूल्य_List *pRhs;

  *ppOut = 0;
  अगर( pVal==0 ) वापस SQLITE_MISUSE_BKPT;
  अगर( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_मूल्य_List_Free ){
    वापस SQLITE_ERROR;
  }अन्यथा{
    निश्चित( (pVal->flags&(MEM_TypeMask|MEM_Term|MEM_Subtype)) ==
                 (MEM_Null|MEM_Term|MEM_Subtype) );
    निश्चित( pVal->eSubtype=='p' );
    निश्चित( pVal->u.zPType!=0 && म_भेद(pVal->u.zPType,"ValueList")==0 );
    pRhs = (मूल्य_List*)pVal->z;
  }
  अगर( b_अग्र ){
    rc = sqlite3_Btree_अग्र(pRhs->pCsr, 0);
  }अन्यथा{
    पूर्णांक dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    निश्चित( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    अगर( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_DONE;
  }
  अगर( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    स_रखो(&sMem, 0, माप(sMem));
    sz = sqlite3_Btree_Payload_आकार(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr,sz,&sMem);
    अगर( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_मूल्य *pOut = pRhs->pOut;
      पूर्णांक iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ENC(pOut->db);
      अगर( (pOut->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(pOut) ){
        rc = SQLITE_NOMEM;
      }अन्यथा{
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  वापस rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
पूर्णांक sqlite3_vtab_in_first(sqlite3_मूल्य *pVal, sqlite3_मूल्य **ppOut){
  वापस मूल्य_From_मूल्य_List(pVal, ppOut, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
पूर्णांक sqlite3_vtab_in_अग्र(sqlite3_मूल्य *pVal, sqlite3_मूल्य **ppOut){
  वापस मूल्य_From_मूल्य_List(pVal, ppOut, 1);
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
#अगर_अघोषित SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  निश्चित( p->pVdbe!=0 );
#अन्यथा
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#पूर्ण_अगर
  अगर( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    अगर( rc ) *piTime = 0;
  }
  वापस *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
अटल SQLITE_NOINLINE व्योम *createAggContext(sqlite3_context *p, पूर्णांक nByte){
  Mem *pMem = p->pMem;
  निश्चित( (pMem->flags & MEM_Agg)==0 );
  अगर( nByte<=0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  }अन्यथा{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    अगर( pMem->z ){
      स_रखो(pMem->z, 0, nByte);
    }
  }
  वापस (व्योम*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
व्योम *sqlite3_aggregate_context(sqlite3_context *p, पूर्णांक nByte){
  निश्चित( p && p->pFunc && p->pFunc->xFinalize );
  निश्चित( sqlite3_mutex_held(p->pOut->db->mutex) );
  testcase( nByte<0 );
  अगर( (p->pMem->flags & MEM_Agg)==0 ){
    वापस createAggContext(p, nByte);
  }अन्यथा{
    वापस (व्योम*)p->pMem->z;
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
व्योम *sqlite3_get_auxdata(sqlite3_context *pCtx, पूर्णांक iArg){
  Aux_तथ्य *p_Aux_तथ्य;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस 0;
#पूर्ण_अगर
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#अगर SQLITE_ENABLE_STAT4
  अगर( pCtx->pVdbe==0 ) वापस 0;
#अन्यथा
  निश्चित( pCtx->pVdbe!=0 );
#पूर्ण_अगर
  क्रम(p_Aux_तथ्य=pCtx->pVdbe->p_Aux_तथ्य; p_Aux_तथ्य; p_Aux_तथ्य=p_Aux_तथ्य->p_अग्र_Aux){
    अगर(  p_Aux_तथ्य->iAuxArg==iArg && (p_Aux_तथ्य->iAuxOp==pCtx->iOp || iArg<0) ){
      वापस p_Aux_तथ्य->pAux;
    }
  }
  वापस 0;
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
व्योम sqlite3_set_auxdata(
  sqlite3_context *pCtx,
  पूर्णांक iArg,
  व्योम *pAux,
  व्योम (*xDelete)(व्योम*)
){
  Aux_तथ्य *p_Aux_तथ्य;
  Vdbe *pVdbe;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCtx==0 ) वापस;
#पूर्ण_अगर
  pVdbe= pCtx->pVdbe;
  निश्चित( sqlite3_mutex_held(pCtx->pOut->db->mutex) );
#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( pVdbe==0 ) जाओ failed;
#अन्यथा
  निश्चित( pVdbe!=0 );
#पूर्ण_अगर

  क्रम(p_Aux_तथ्य=pVdbe->p_Aux_तथ्य; p_Aux_तथ्य; p_Aux_तथ्य=p_Aux_तथ्य->p_अग्र_Aux){
    अगर( p_Aux_तथ्य->iAuxArg==iArg && (p_Aux_तथ्य->iAuxOp==pCtx->iOp || iArg<0) ){
      अवरोध;
    }
  }
  अगर( p_Aux_तथ्य==0 ){
    p_Aux_तथ्य = sqlite3DbMallocZero(pVdbe->db, माप(Aux_तथ्य));
    अगर( !p_Aux_तथ्य ) जाओ failed;
    p_Aux_तथ्य->iAuxOp = pCtx->iOp;
    p_Aux_तथ्य->iAuxArg = iArg;
    p_Aux_तथ्य->p_अग्र_Aux = pVdbe->p_Aux_तथ्य;
    pVdbe->p_Aux_तथ्य = p_Aux_तथ्य;
    अगर( pCtx->isError==0 ) pCtx->isError = -1;
  }अन्यथा अगर( p_Aux_तथ्य->xDeleteAux ){
    p_Aux_तथ्य->xDeleteAux(p_Aux_तथ्य->pAux);
  }

  p_Aux_तथ्य->pAux = pAux;
  p_Aux_तथ्य->xDeleteAux = xDelete;
  वापस;

failed:
  अगर( xDelete ){
    xDelete(pAux);
  }
}

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
पूर्णांक sqlite3_aggregate_गिनती(sqlite3_context *p){
  निश्चित( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  वापस p->pMem->n;
}
#पूर्ण_अगर

/*
** Return the number of columns in the result set for the statement pStmt.
*/
पूर्णांक sqlite3_column_गिनती(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  अगर( pVm==0 ) वापस 0;
  वापस pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
पूर्णांक sqlite3_तथ्य_गिनती(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  अगर( pVm==0 || pVm->p_परिणाम_Row==0 ) वापस 0;
  वापस pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
अटल स्थिर Mem *column_Null_मूल्य(व्योम){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  अटल स्थिर Mem nullMem
#अगर defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#पूर्ण_अगर
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
        /* .xDel       = */ (व्योम(*)(व्योम*))0,
#अगर_घोषित SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#पूर्ण_अगर
      };
  वापस &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
अटल Mem *columnMem(sqlite3_stmt *pStmt, पूर्णांक i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  अगर( pVm==0 ) वापस (Mem*)column_Null_मूल्य();
  निश्चित( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  अगर( pVm->p_परिणाम_Row!=0 && i<pVm->nResColumn && i>=0 ){
    pOut = &pVm->p_परिणाम_Row[i];
  }अन्यथा{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem*)column_Null_मूल्य();
  }
  वापस pOut;
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
अटल व्योम columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  अगर( p ){
    निश्चित( p->db!=0 );
    निश्चित( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
स्थिर व्योम *sqlite3_column_blob(sqlite3_stmt *pStmt, पूर्णांक i){
  स्थिर व्योम *val;
  val = sqlite3_मूल्य_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  वापस val;
}
पूर्णांक sqlite3_column_bytes(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक val = sqlite3_मूल्य_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
पूर्णांक sqlite3_column_bytes16(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक val = sqlite3_मूल्य_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
द्विगुन sqlite3_column_double(sqlite3_stmt *pStmt, पूर्णांक i){
  द्विगुन val = sqlite3_मूल्य_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
पूर्णांक sqlite3_column_int(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक val = sqlite3_मूल्य_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, पूर्णांक i){
  sqlite_int64 val = sqlite3_मूल्य_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
स्थिर अचिन्हित अक्षर *sqlite3_column_text(sqlite3_stmt *pStmt, पूर्णांक i){
  स्थिर अचिन्हित अक्षर *val = sqlite3_मूल्य_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
sqlite3_मूल्य *sqlite3_column_मूल्य(sqlite3_stmt *pStmt, पूर्णांक i){
  Mem *pOut = columnMem(pStmt, i);
  अगर( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  वापस (sqlite3_मूल्य *)pOut;
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_column_text16(sqlite3_stmt *pStmt, पूर्णांक i){
  स्थिर व्योम *val = sqlite3_मूल्य_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस val;
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
पूर्णांक sqlite3_column_type(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक iType = sqlite3_मूल्य_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  वापस iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
अटल स्थिर अक्षर * स्थिर azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
अटल स्थिर u16 azExplainColNames16data[] = {
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
अटल स्थिर u8 iExplainColNames16[] = {
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
अटल स्थिर व्योम *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  पूर्णांक N,                   /* Which column to get the name for */
  पूर्णांक useUtf16,            /* True to return the name as UTF16 */
  पूर्णांक useType              /* What type of name */
){
  स्थिर व्योम *ret;
  Vdbe *p;
  पूर्णांक n;
  sqlite3 *db;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pStmt==0 ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  अगर( N<0 ) वापस 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  निश्चित( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  अगर( p->explain ){
    अगर( useType>0 ) जाओ columnName_end;
    n = p->explain==1 ? 8 : 4;
    अगर( N>=n ) जाओ columnName_end;
    अगर( useUtf16 ){
      पूर्णांक i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (व्योम*)&azExplainColNames16data[i];
    }अन्यथा{
      ret = (व्योम*)azExplainColNames8[N + 8*p->explain - 8];
    }
    जाओ columnName_end;
  }
  n = p->nResColumn;
  अगर( N<n ){
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType*n;
#अगर_अघोषित SQLITE_OMIT_UTF16
    अगर( useUtf16 ){
      ret = sqlite3_मूल्य_text16((sqlite3_मूल्य*)&p->aColName[N]);
    }अन्यथा
#पूर्ण_अगर
    {
      ret = sqlite3_मूल्य_text((sqlite3_मूल्य*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    निश्चित( db->mallocFailed==0 || db->mallocFailed==1 );
    अगर( db->mallocFailed > prior_mallocFailed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  वापस ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
स्थिर अक्षर *sqlite3_column_name(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 0, COLNAME_NAME);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_column_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 1, COLNAME_NAME);
}
#पूर्ण_अगर

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#अगर defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
स्थिर अक्षर *sqlite3_column_decltype(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_column_decltype16(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
#पूर्ण_अगर /* SQLITE_OMIT_DECLTYPE */

#अगर_घोषित SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
स्थिर अक्षर *sqlite3_column_database_name(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_column_database_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
स्थिर अक्षर *sqlite3_column_table_name(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 0, COLNAME_TABLE);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_column_table_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 1, COLNAME_TABLE);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
स्थिर अक्षर *sqlite3_column_origin_name(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
स्थिर व्योम *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, पूर्णांक N){
  वापस columnName(pStmt, N, 1, COLNAME_COLUMN);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
#पूर्ण_अगर /* SQLITE_ENABLE_COLUMN_METADATA */


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
अटल पूर्णांक vdbeUnbind(Vdbe *p, अचिन्हित पूर्णांक i){
  Mem *pVar;
  अगर( vdbeSafetyNotNull(p) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  अगर( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    वापस SQLITE_MISUSE_BKPT;
  }
  अगर( i>=(अचिन्हित पूर्णांक)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    वापस SQLITE_RANGE;
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
  निश्चित( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  निश्चित( (p->expmask & p->smimask)==p->smimask );
  अगर( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    अगर( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->errCode = SQLITE_OK;

  वापस SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
अटल पूर्णांक bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  पूर्णांक i,                 /* Index of the parameter to bind */
  स्थिर व्योम *z_तथ्य,     /* Pointer to the data to be bound */
  i64 n_तथ्य,             /* Number of bytes of data to be bound */
  व्योम (*xDel)(व्योम*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  पूर्णांक rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  अगर( rc==SQLITE_OK ){
    निश्चित( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    अगर( z_तथ्य!=0 ){
      pVar = &p->aVar[i-1];
      अगर( encoding==SQLITE_UTF8 ){
        rc = sqlite3VdbeMemSetText(pVar, z_तथ्य, n_तथ्य, xDel);
      }अन्यथा अगर( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        निश्चित( z_तथ्य==0
             || n_तथ्य<0 
             || n_तथ्य>pVar->db->aLimit[SQLITE_LIMIT_लंबाई]
             || ((u8*)z_तथ्य)[n_तथ्य]==0
        );
        rc = sqlite3VdbeMemSetText(pVar, z_तथ्य, n_तथ्य, xDel);
        pVar->flags |= MEM_Term;
      }अन्यथा{
        rc = sqlite3VdbeMemSetStr(pVar, z_तथ्य, n_तथ्य, encoding, xDel);
        अगर( encoding==0 ) pVar->enc = ENC(p->db);
      }
      अगर( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      अगर( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }अन्यथा अगर( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((व्योम*)z_तथ्य);
  }
  वापस rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
पूर्णांक sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिर व्योम *z_तथ्य,
  पूर्णांक n_तथ्य,
  व्योम (*xDel)(व्योम*)
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( n_तथ्य<0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  वापस bindText(pStmt, i, z_तथ्य, n_तथ्य, xDel, 0);
}
पूर्णांक sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिर व्योम *z_तथ्य,
  sqlite3_uint64 n_तथ्य,
  व्योम (*xDel)(व्योम*)
){
  निश्चित( xDel!=SQLITE_DYNAMIC );
  वापस bindText(pStmt, i, z_तथ्य, n_तथ्य, xDel, 0);
}
पूर्णांक sqlite3_bind_double(sqlite3_stmt *pStmt, पूर्णांक i, द्विगुन r_मूल्य){
  पूर्णांक rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  अगर( rc==SQLITE_OK ){
    निश्चित( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], r_मूल्य);
    sqlite3_mutex_leave(p->db->mutex);
  }
  वापस rc;
}
पूर्णांक sqlite3_bind_int(sqlite3_stmt *p, पूर्णांक i, पूर्णांक i_मूल्य){
  वापस sqlite3_bind_int64(p, i, (i64)i_मूल्य);
}
पूर्णांक sqlite3_bind_int64(sqlite3_stmt *pStmt, पूर्णांक i, sqlite_int64 i_मूल्य){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  अगर( vdbeSafetyNotNull(p) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  अगर( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    वापस SQLITE_MISUSE_BKPT;
  }
  अगर( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    वापस SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  अगर( p->expmask!=0 ){
    u32 expireMask = i>=32 ? 0x80000000 : (u32)1 << i;
    निश्चित( (p->expmask & p->smimask)==p->smimask );
    अगर( (p->expmask & expireMask)!=0 ){
      अगर( (p->smimask & expireMask)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        पूर्णांक sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        पूर्णांक sm2 = i_मूल्य>=0 && i_मूल्य<=1;
        अगर( sm1!=sm2 ){
          p->expired = 1;
        }
      }अन्यथा अगर( (pVar->flags & MEM_Int)==0 || i_मूल्य!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3VdbeMemSetInt64(pVar, i_मूल्य);
  sqlite3_mutex_leave(p->db->mutex);
  वापस SQLITE_OK;
}
पूर्णांक sqlite3_bind_null(sqlite3_stmt *pStmt, पूर्णांक i){
  पूर्णांक rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  अगर( rc==SQLITE_OK ){
    निश्चित( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  वापस rc;
}
पूर्णांक sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  व्योम *pPtr,
  स्थिर अक्षर *zPTtype,
  व्योम (*xDestructor)(व्योम*)
){
  पूर्णांक rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  अगर( rc==SQLITE_OK ){
    निश्चित( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3VdbeMemSetPointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }अन्यथा अगर( xDestructor ){
    xDestructor(pPtr);
  }
  वापस rc;
}
पूर्णांक sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिर अक्षर *z_तथ्य,
  पूर्णांक n_तथ्य,
  व्योम (*xDel)(व्योम*)
){
  वापस bindText(pStmt, i, z_तथ्य, n_तथ्य, xDel, SQLITE_UTF8);
}
पूर्णांक sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिर अक्षर *z_तथ्य,
  sqlite3_uint64 n_तथ्य,
  व्योम (*xDel)(व्योम*),
  अचिन्हित अक्षर enc
){
  निश्चित( xDel!=SQLITE_DYNAMIC );
  अगर( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    अगर( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_तथ्य &= ~(u64)1;
  }
  वापस bindText(pStmt, i, z_तथ्य, n_तथ्य, xDel, enc);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
पूर्णांक sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  पूर्णांक i,
  स्थिर व्योम *z_तथ्य,
  पूर्णांक n,
  व्योम (*xDel)(व्योम*)
){
  वापस bindText(pStmt, i, z_तथ्य, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
पूर्णांक sqlite3_bind_मूल्य(sqlite3_stmt *pStmt, पूर्णांक i, स्थिर sqlite3_मूल्य *p_मूल्य){
  पूर्णांक rc;
  चयन( sqlite3_मूल्य_type((sqlite3_मूल्य*)p_मूल्य) ){
    हाल SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_मूल्य->u.i);
      अवरोध;
    }
    हाल SQLITE_FLOAT: {
      निश्चित( p_मूल्य->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_मूल्य->flags & MEM_Real) ? p_मूल्य->u.r : (द्विगुन)p_मूल्य->u.i
      );
      अवरोध;
    }
    हाल SQLITE_BLOB: {
      अगर( p_मूल्य->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_मूल्य->u.nZero);
      }अन्यथा{
        rc = sqlite3_bind_blob(pStmt, i, p_मूल्य->z, p_मूल्य->n,SQLITE_TRANSIENT);
      }
      अवरोध;
    }
    हाल SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_मूल्य->z, p_मूल्य->n, SQLITE_TRANSIENT,
                              p_मूल्य->enc);
      अवरोध;
    }
    शेष: {
      rc = sqlite3_bind_null(pStmt, i);
      अवरोध;
    }
  }
  वापस rc;
}
पूर्णांक sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, पूर्णांक i, पूर्णांक n){
  पूर्णांक rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  अगर( rc==SQLITE_OK ){
    निश्चित( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#अगर_अघोषित SQLITE_OMIT_INCRBLOB
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#अन्यथा
    rc = sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
#पूर्ण_अगर
    sqlite3_mutex_leave(p->db->mutex);
  }
  वापस rc;
}
पूर्णांक sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, पूर्णांक i, sqlite3_uint64 n){
  पूर्णांक rc;
  Vdbe *p = (Vdbe *)pStmt;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( p==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(p->db->mutex);
  अगर( n>(u64)p->db->aLimit[SQLITE_LIMIT_लंबाई] ){
    rc = SQLITE_TOOBIG;
  }अन्यथा{
    निश्चित( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  वापस rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
पूर्णांक sqlite3_bind_parameter_गिनती(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  वापस p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
स्थिर अक्षर *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, पूर्णांक i){
  Vdbe *p = (Vdbe*)pStmt;
  अगर( p==0 ) वापस 0;
  वापस sqlite3_VList_अंक_To_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
पूर्णांक sqlite3_Vdbe_Parameter_सूचकांक(Vdbe *p, स्थिर अक्षर *zName, पूर्णांक nName){
  अगर( p==0 || zName==0 ) वापस 0;
  वापस sqlite3_VList_Name_To_अंक(p->pVList, zName, nName);
}
पूर्णांक sqlite3_bind_parameter_सूचकांक(sqlite3_stmt *pStmt, स्थिर अक्षर *zName){
  वापस sqlite3_Vdbe_Parameter_सूचकांक((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
पूर्णांक sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  पूर्णांक i;
  निश्चित( pTo->db==pFrom->db );
  निश्चित( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  क्रम(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  वापस SQLITE_OK;
}

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
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
  अगर( pFrom->nVar!=pTo->nVar ){
    वापस SQLITE_ERROR;
  }
  निश्चित( (pTo->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pTo->expmask==0 );
  अगर( pTo->expmask ){
    pTo->expired = 1;
  }
  निश्चित( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  अगर( pFrom->expmask ){
    pFrom->expired = 1;
  }
  वापस sqlite3TransferBindings(pFromStmt, pToStmt);
}
#पूर्ण_अगर

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  वापस pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
पूर्णांक sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  वापस pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
पूर्णांक sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  वापस pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
पूर्णांक sqlite3_stmt_explain(sqlite3_stmt *pStmt, पूर्णांक eMode){
  Vdbe *v = (Vdbe*)pStmt;
  पूर्णांक rc;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pStmt==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(v->db->mutex);
  अगर( ((पूर्णांक)v->explain)==eMode ){
    rc = SQLITE_OK;
  }अन्यथा अगर( eMode<0 || eMode>2 ){
    rc = SQLITE_ERROR;
  }अन्यथा अगर( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा अगर( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }अन्यथा अगर( v->nMem>=10 && (eMode!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = eMode;
    rc = SQLITE_OK;
  }अन्यथा{
    पूर्णांक savedMode = v->explain;
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    अगर( rc ){
      v->explain = savedMode;
    }अन्यथा{
      v->haveEqpOps = eMode==2;
    }
  }
  अगर( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }अन्यथा{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  वापस rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
पूर्णांक sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  वापस v!=0 && v->eVdbeState==VDBE_RUN_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_अग्र_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_अग्र;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(pDb) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(pDb->mutex);
  अगर( pStmt==0 ){
    p_अग्र = (sqlite3_stmt*)pDb->pVdbe;
  }अन्यथा{
    p_अग्र = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  वापस p_अग्र;
}

/*
** Return the value of a status counter for a prepared statement
*/
पूर्णांक sqlite3_stmt_status(sqlite3_stmt *pStmt, पूर्णांक op, पूर्णांक resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_आकार(pVdbe->a_गणक)))
  ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  अगर( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (पूर्णांक*)&v;
    निश्चित( db->lookaside.pEnd==db->lookaside.pTrueEnd );
    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  }अन्यथा{
    v = pVdbe->a_गणक[op];
    अगर( resetFlag ) pVdbe->a_गणक[op] = 0;
  }
  वापस (पूर्णांक)v;
}

/*
** Return the SQL associated with a prepared statement
*/
स्थिर अक्षर *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  वापस p ? p->zSql : 0;
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
#अगर_घोषित SQLITE_OMIT_TRACE
  वापस 0;
#अन्यथा
  अक्षर *z = 0;
  स्थिर अक्षर *zSql = sqlite3_sql(pStmt);
  अगर( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  वापस z;
#पूर्ण_अगर
}

#अगर_घोषित SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
स्थिर अक्षर *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  अगर( p==0 ) वापस 0;
  अगर( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  वापस p->zNormSql;
}
#पूर्ण_अगर /* SQLITE_ENABLE_NORMALIZE */

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
अटल UnpackedRecord *vdbeUnpackRecord(
  कुंजी_Info *p_कुंजी_Info,
  पूर्णांक n_कुंजी,
  स्थिर व्योम *p_कुंजी
){
  UnpackedRecord *pRet;           /* Return value */

  pRet = sqlite3VdbeAllocUnpackedRecord(p_कुंजी_Info);
  अगर( pRet ){
    स_रखो(pRet->aMem, 0, माप(Mem)*(p_कुंजी_Info->n_कुंजी_Field+1));
    sqlite3VdbeRecordUnpack(n_कुंजी, p_कुंजी, pRet);
  }
  वापस pRet;
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

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( db==0 || pp_मूल्य==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  अगर( !p || p->op==SQLITE_जोड़ो ){
    rc = SQLITE_MISUSE_BKPT;
    जाओ preupdate_old_out;
  }
  अगर( p->pPk ){
    iStore = sqlite3_Table_Column_To_सूचकांक(p->pPk, iIdx);
  }अन्यथा अगर( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    जाओ preupdate_old_out;
  }अन्यथा{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }
  अगर( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    जाओ preupdate_old_out;
  }

  अगर( iIdx==p->pTab->iPKey ){
    *pp_मूल्य = pMem = &p->oldipk;
    sqlite3VdbeMemSetInt64(pMem, p->iKey1);
  }अन्यथा{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    अगर( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      निश्चित( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_आकार(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      अगर( !aRec ) जाओ preupdate_old_out;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      अगर( rc==SQLITE_OK ){
        स_रखो(aRec+nRec, 0, 8);
        p->pUnpacked = vdbeUnpackRecord(p->pKeyinfo, nRec, aRec);
        अगर( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      अगर( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        जाओ preupdate_old_out;
      }
      p->aRecord = aRec;
    }

    pMem = *pp_मूल्य = &p->pUnpacked->aMem[iStore];
    अगर( iStore>=p->pUnpacked->nField ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      अगर( pCol->iDflt>0 ){
        अगर( p->apDflt==0 ){
          पूर्णांक nByte;
          निश्चित( माप(sqlite3_मूल्य*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = माप(sqlite3_मूल्य*)*p->pTab->nCol;
          p->apDflt = (sqlite3_मूल्य**)sqlite3DbMallocZero(db, nByte);
          अगर( p->apDflt==0 ) जाओ preupdate_old_out;
        }
        अगर( p->apDflt[iIdx]==0 ){
          sqlite3_मूल्य *pVal = 0;
          व्यंजक *pDflt;
          निश्चित( p->pTab!=0 && IsOrdinaryTable(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_व्यंजक;
          rc = sqlite3_मूल्य_From_व्यंजक(db, pDflt, ENC(db), pCol->affinity, &pVal);
          अगर( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_मूल्य = p->apDflt[iIdx];
      }अन्यथा{
        *pp_मूल्य = (sqlite3_मूल्य *)column_Null_मूल्य();
      }
    }अन्यथा अगर( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      अगर( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_out:
  sqlite3Error(db, rc);
  वापस sqlite3ApiExit(db, rc);
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
पूर्णांक sqlite3_preupdate_गिनती(sqlite3 *db){
  PreUpdate *p;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#पूर्ण_अगर
  वापस (p ? p->pKeyinfo->n_कुंजी_Field : 0);
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
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
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#पूर्ण_अगर
  वापस (p ? p->v->nFrame : 0);
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
पूर्णांक sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#पूर्ण_अगर
  वापस (p ? p->iBlobWrite : -1);
}
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
पूर्णांक sqlite3_preupdate_new(sqlite3 *db, पूर्णांक iIdx, sqlite3_मूल्य **pp_मूल्य){
  PreUpdate *p;
  पूर्णांक rc = SQLITE_OK;
  Mem *pMem;
  पूर्णांक iStore = 0;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( db==0 || pp_मूल्य==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  p = db->pPreUpdate;
  अगर( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    जाओ preupdate_new_out;
  }
  अगर( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_Table_Column_To_सूचकांक(p->pPk, iIdx);
  }अन्यथा अगर( iIdx >= p->pTab->nCol ){
    वापस SQLITE_MISUSE_BKPT;
  }अन्यथा{
    iStore = sqlite3TableColumnToStorage(p->pTab, iIdx);
  }

  अगर( iStore>=p->pCsr->nField || iStore<0 ){
    rc = SQLITE_RANGE;
    जाओ preupdate_new_out;
  }

  अगर( p->op==SQLITE_जोड़ो ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    UnpackedRecord *pUnpack = p->pNewUnpacked;
    अगर( !pUnpack ){
      Mem *p_तथ्य = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_तथ्य);
      अगर( rc!=SQLITE_OK ) जाओ preupdate_new_out;
      pUnpack = vdbeUnpackRecord(p->pKeyinfo, p_तथ्य->n, p_तथ्य->z);
      अगर( !pUnpack ){
        rc = SQLITE_NOMEM;
        जाओ preupdate_new_out;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    अगर( iIdx==p->pTab->iPKey ){
      sqlite3VdbeMemSetInt64(pMem, p->iKey2);
    }अन्यथा अगर( iStore>=pUnpack->nField ){
      pMem = (sqlite3_मूल्य *)column_Null_मूल्य();
    }अन्यथा अगर( (p->pTab->aCol)
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
    निश्चित( p->op==SQLITE_UPDATE );
    अगर( !p->aNew ){
      निश्चित( माप(Mem)*UMXV(p->pCsr->nField) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, माप(Mem)*p->pCsr->nField);
      अगर( !p->aNew ){
        rc = SQLITE_NOMEM;
        जाओ preupdate_new_out;
      }
    }
    निश्चित( iStore>=0 && iStore<p->pCsr->nField );
    pMem = &p->aNew[iStore];
    अगर( pMem->flags==0 ){
      अगर( iIdx==p->pTab->iPKey ){
        sqlite3VdbeMemSetInt64(pMem, p->iKey2);
      }अन्यथा{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        अगर( rc!=SQLITE_OK ) जाओ preupdate_new_out;
      }
    }
  }
  *pp_मूल्य = pMem;

 preupdate_new_out:
  sqlite3Error(db, rc);
  वापस sqlite3ApiExit(db, rc);
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
पूर्णांक sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  पूर्णांक iScan,                      /* Index of loop to report on */
  पूर्णांक iScanStatusOp,              /* Which metric to return */
  पूर्णांक flags,
  व्योम *pOut                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  पूर्णांक nOp;
  ScanStatus *pScan = 0;
  पूर्णांक idx;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( p==0 || pOut==0
      || iScanStatusOp<SQLITE_SCANSTAT_NLOOP
      || iScanStatusOp>SQLITE_SCANSTAT_NCYCLE ){
    वापस 1;
  }
#पूर्ण_अगर
  aOp = p->aOp;
  nOp = p->nOp;
  अगर( p->pFrame ){
    VdbeFrame *pFrame;
    क्रम(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  अगर( iScan<0 ){
    पूर्णांक ii;
    अगर( iScanStatusOp==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      क्रम(ii=0; ii<nOp; ii++){
        res += aOp[ii].nCycle;
      }
      *(i64*)pOut = res;
      वापस 0;
    }
    वापस 1;
  }
  अगर( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = iScan;
  }अन्यथा{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    क्रम(idx=0; idx<p->nScan; idx++){
      pScan = &p->aScan[idx];
      अगर( pScan->zName ){
        iScan--;
        अगर( iScan<0 ) अवरोध;
      }
    }
  }
  अगर( idx>=p->nScan ) वापस 1;
  निश्चित( pScan==0 || pScan==&p->aScan[idx] );
  pScan = &p->aScan[idx];

  चयन( iScanStatusOp ){
    हाल SQLITE_SCANSTAT_NLOOP: {
      अगर( pScan->addrLoop>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrLoop].nExec;
      }अन्यथा{
        *(sqlite3_int64*)pOut = -1;
      }
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_NVISIT: {
      अगर( pScan->addrVisit>0 ){
        *(sqlite3_int64*)pOut = aOp[pScan->addrVisit].nExec;
      }अन्यथा{
        *(sqlite3_int64*)pOut = -1;
      }
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_EST: {
      द्विगुन r = 1.0;
      LogEst x = pScan->nEst;
      जबतक( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(द्विगुन*)pOut = r*sqlite3LogEstToInt(x);
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_NAME: {
      *(स्थिर अक्षर**)pOut = pScan->zName;
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_EXPLAIN: {
      अगर( pScan->addrExplain ){
        *(स्थिर अक्षर**)pOut = aOp[ pScan->addrExplain ].p4.z;
      }अन्यथा{
        *(स्थिर अक्षर**)pOut = 0;
      }
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_SELECTID: {
      अगर( pScan->addrExplain ){
        *(पूर्णांक*)pOut = aOp[ pScan->addrExplain ].p1;
      }अन्यथा{
        *(पूर्णांक*)pOut = -1;
      }
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_PARENTID: {
      अगर( pScan->addrExplain ){
        *(पूर्णांक*)pOut = aOp[ pScan->addrExplain ].p2;
      }अन्यथा{
        *(पूर्णांक*)pOut = -1;
      }
      अवरोध;
    }
    हाल SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      अगर( pScan->aAddrRange[0]==0 ){
        res = -1;
      }अन्यथा{
        पूर्णांक ii;
        क्रम(ii=0; ii<Array_आकार(pScan->aAddrRange); ii+=2){
          पूर्णांक iIns = pScan->aAddrRange[ii];
          पूर्णांक iEnd = pScan->aAddrRange[ii+1];
          अगर( iIns==0 ) अवरोध;
          अगर( iIns>0 ){
            जबतक( iIns<=iEnd ){
              res += aOp[iIns].nCycle;
              iIns++;
            }
          }अन्यथा{
            पूर्णांक iOp;
            क्रम(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              अगर( pOp->p1!=iEnd ) जारी;
              अगर( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                जारी;
              }
              res += aOp[iOp].nCycle;
            }
          }
        }
      }
      *(i64*)pOut = res;
      अवरोध;
    }
    शेष: {
      वापस 1;
    }
  }
  वापस 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
पूर्णांक sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  पूर्णांक iScan,                      /* Index of loop to report on */
  पूर्णांक iScanStatusOp,              /* Which metric to return */
  व्योम *pOut                      /* OUT: Write the answer here */
){
  वापस sqlite3_stmt_scanstatus_v2(pStmt, iScan, iScanStatusOp, 0, pOut);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
व्योम sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  पूर्णांक ii;
  क्रम(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->nCycle = 0;
  }
}
#पूर्ण_अगर /* SQLITE_ENABLE_STMT_SCANSTATUS */
