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

#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
पूर्णाङ्क sqlite3_expired(sqlite3_stmt *pStmt){
  पूर्णाङ्क iRet = 1;
  यदि( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  प्रत्यागम iRet;
}
#यदि_अन्तः

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
स्थिर पूर्णाङ्क vdbeSafety(Vdbe *p){
  यदि( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    प्रत्यागम 1;
  }अन्यथा{
    प्रत्यागम 0;
  }
}
स्थिर पूर्णाङ्क vdbeSafetyNotNull(Vdbe *p){
  यदि( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    प्रत्यागम 1;
  }अन्यथा{
    प्रत्यागम vdbeSafety(p);
  }
}

#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE
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
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  यदि( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#यदि_अन्तः
  यदि( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (शून्य*)&iElapse);
  }
  p->startTime = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   यदि( ((P)->startTime)>0 ){ invokeProfileCallback(DB,P); }
#अन्यथा
# define checkProfileCallback(DB,P)  /*no-op*/
#यदि_अन्तः

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
पूर्णाङ्क sqlite3_finalize(sqlite3_stmt *pStmt){
  पूर्णाङ्क rc;
  यदि( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }अन्यथा{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    यदि( vdbeSafety(v) ) प्रत्यागम SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    प्रतिज्ञा( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  प्रत्यागम rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
पूर्णाङ्क sqlite3_reset(sqlite3_stmt *pStmt){
  पूर्णाङ्क rc;
  यदि( pStmt==0 ){
    rc = SQLITE_OK;
  }अन्यथा{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    प्रतिज्ञा( (rc & (db->err_आवरणम्))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  प्रत्यागम rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
पूर्णाङ्क sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  पूर्णाङ्क i;
  पूर्णाङ्क rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 आवरणम्_Small_Int = 0;  /* Bitmask of bindings with value 0 or 1 */
#यदि SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pStmt==0 ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
#यदि_अन्तः
#यदि SQLITE_THREADSAFE
  mutex = p->db->mutex;
#यदि_अन्तः

  sqlite3_mutex_enter(mutex);
  प्रति(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    यदि( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      आवरणम्_Small_Int |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  प्रतिज्ञा( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  यदि( (p->expmask&~p->smimask)!=0 || (p->smimask&आवरणम्_Small_Int)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  प्रत्यागम rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ध्रुव शून्य *sqlite3_मानम्_blob(sqlite3_मानम् *pVal){
  Mem *p = (Mem*)pVal;
  यदि( p->flags & (MEM_Blob|MEM_Str) ){
    यदि( ExpandBlob(p)!=SQLITE_OK ){
      प्रतिज्ञा( p->flags==MEM_Null && p->z==0 );
      प्रत्यागम 0;
    }
    p->flags |= MEM_Blob;
    प्रत्यागम p->n ? p->z : 0;
  }अन्यथा{
    प्रत्यागम sqlite3_मानम्_text(pVal);
  }
}
पूर्णाङ्क sqlite3_मानम्_अष्टकानि(sqlite3_मानम् *pVal){
  प्रत्यागम sqlite3_मानम्_अष्टकानि_2(pVal, SQLITE_UTF8);
}
पूर्णाङ्क sqlite3_मानम्_bytes16(sqlite3_मानम् *pVal){
  प्रत्यागम sqlite3_मानम्_अष्टकानि_2(pVal, SQLITE_UTF16NATIVE);
}
द्विगुण sqlite3_मानम्_double(sqlite3_मानम् *pVal){
  प्रत्यागम sqlite3_Vdbe_Real_मानम्((Mem*)pVal);
}
पूर्णाङ्क sqlite3_मानम्_int(sqlite3_मानम् *pVal){
  प्रत्यागम (पूर्णाङ्क)sqlite3_Vdbe_Int_मानम्((Mem*)pVal);
}
sqlite_int64 sqlite3_मानम्_int64(sqlite3_मानम् *pVal){
  प्रत्यागम sqlite3_Vdbe_Int_मानम्((Mem*)pVal);
}
अचिह्नित पूर्णाङ्क sqlite3_मानम्_subtype(sqlite3_मानम् *pVal){
  Mem *pMem = (Mem*)pVal;
  प्रत्यागम ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
शून्य *sqlite3_मानम्_pointer(sqlite3_मानम् *pVal, ध्रुव वर्ण *zPType){
  Mem *p = (Mem*)pVal;
  यदि( (p->flags&(MEM_Type_आवरणम्|MEM_पदांशः|MEM_Subtype)) ==
                 (MEM_Null|MEM_पदांशः|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && वाक्यं_तुलय(p->u.zPType, zPType)==0
  ){
    प्रत्यागम (शून्य*)p->z;
  }अन्यथा{
    प्रत्यागम 0;
  }
}
ध्रुव अचिह्नित वर्ण *sqlite3_मानम्_text(sqlite3_मानम् *pVal){
  प्रत्यागम (ध्रुव अचिह्नित वर्ण *)sqlite3_मानम्_Text(pVal, SQLITE_UTF8);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_मानम्_text16(sqlite3_मानम्* pVal){
  प्रत्यागम sqlite3_मानम्_Text(pVal, SQLITE_UTF16NATIVE);
}
ध्रुव शून्य *sqlite3_मानम्_text16be(sqlite3_मानम् *pVal){
  प्रत्यागम sqlite3_मानम्_Text(pVal, SQLITE_UTF16BE);
}
ध्रुव शून्य *sqlite3_मानम्_text16le(sqlite3_मानम् *pVal){
  प्रत्यागम sqlite3_मानम्_Text(pVal, SQLITE_UTF16LE);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
पूर्णाङ्क sqlite3_मानम्_type(sqlite3_मानम्* pVal){
  स्थिर ध्रुव u8 aType[] = {
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
#यदि_परिभाषितम् SQLITE_DEBUG
  {
    पूर्णाङ्क eType = SQLITE_BLOB;
    यदि( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }अन्यथा यदि( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }अन्यथा यदि( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }अन्यथा यदि( pVal->flags & MEM_Str ){
      eType = SQLITE_TEXT;
    }
    प्रतिज्ञा( eType == aType[pVal->flags&MEM_Aff_आवरणम्] );
  }
#यदि_अन्तः
  प्रत्यागम aType[pVal->flags&MEM_Aff_आवरणम्];
}
पूर्णाङ्क sqlite3_मानम्_encoding(sqlite3_मानम् *pVal){
  प्रत्यागम pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
पूर्णाङ्क sqlite3_मानम्_nochange(sqlite3_मानम् *pVal){
  प्रत्यागम (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
पूर्णाङ्क sqlite3_मानम्_frombind(sqlite3_मानम् *pVal){
  प्रत्यागम (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_मानम् *sqlite3_मानम्_dup(ध्रुव sqlite3_मानम् *pOrig){
  sqlite3_मानम् *pNew;
  यदि( pOrig==0 ) प्रत्यागम 0;
  pNew = sqlite3_malloc( परिमाण(*pNew) );
  यदि( pNew==0 ) प्रत्यागम 0;
  स्मृतिं_पूरय(pNew, 0, परिमाण(*pNew));
  स्मृतिं_प्रतिलिख(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  यदि( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    यदि( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_मानम्_Free(pNew);
      pNew = 0;
    }
  }अन्यथा यदि( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_पदांशः|MEM_Subtype);
  }
  प्रत्यागम pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
शून्य sqlite3_मानम्_free(sqlite3_मानम् *pOld){
  sqlite3_मानम्_Free(pOld);
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
स्थिर शून्य समुच्चयः_परिणाम_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  ध्रुव वर्ण *z,          /* String pointer */
  पूर्णाङ्क n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  शून्य (*xDel)(शून्य*)     /* Destructor function */
){
  Mem *p_बहिर्गमनम् = pCtx->p_बहिर्गमनम्;
  पूर्णाङ्क rc;
  यदि( enc==SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_समुच्चयः_Text(p_बहिर्गमनम्, z, n, xDel);
  }अन्यथा यदि( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    प्रतिज्ञा( z==0
         || n<0 
         || n>p_बहिर्गमनम्->db->a_सीमा[SQLITE_सीमा_दीर्घता]
         || z[n]==0
    );
    rc = sqlite3_Vdbe_Mem_समुच्चयः_Text(p_बहिर्गमनम्, z, n, xDel);
    p_बहिर्गमनम्->flags |= MEM_पदांशः;
  }अन्यथा{
    rc = sqlite3_Vdbe_Mem_समुच्चयः_Str(p_बहिर्गमनम्, z, n, enc, xDel);
  }
  यदि( rc ){
    यदि( rc==SQLITE_TOOBIG ){
      sqlite3_परिणाम_error_toobig(pCtx);
    }अन्यथा{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      प्रतिज्ञा( rc==SQLITE_NOMEM );
      sqlite3_परिणाम_error_nomem(pCtx);
    }
    प्रत्यागम;
  }
  sqlite3VdbeChangeEncoding(p_बहिर्गमनम्, pCtx->enc);
  यदि( sqlite3VdbeMemTooBig(p_बहिर्गमनम्) ){
    sqlite3_परिणाम_error_toobig(pCtx);
  }
}
स्थिर पूर्णाङ्क invoke_मानम्_Destructor(
  ध्रुव शून्य *p,             /* Value to destroy */
  शून्य (*xDel)(शून्य*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  यदि( xDel==0 ){
    /* noop */
  }अन्यथा यदि( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }अन्यथा{
    xDel((शून्य*)p);
  }
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx!=0 ){
    sqlite3_परिणाम_error_toobig(pCtx);
  }
#अन्यथा
  प्रतिज्ञा( pCtx!=0 );
  sqlite3_परिणाम_error_toobig(pCtx);
#यदि_अन्तः
  प्रत्यागम SQLITE_TOOBIG;
}
शून्य sqlite3_परिणाम_blob(
  sqlite3_context *pCtx,
  ध्रुव शून्य *z,
  पूर्णाङ्क n,
  शून्य (*xDel)(शून्य *)
){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 || n<0 ){
    invoke_मानम्_Destructor(z, xDel, pCtx);
    प्रत्यागम;
  }
#यदि_अन्तः
  प्रतिज्ञा( n>=0 );
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, n, 0, xDel);
}
शून्य sqlite3_परिणाम_blob64(
  sqlite3_context *pCtx,
  ध्रुव शून्य *z,
  sqlite3_uint64 n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ){
    invoke_मानम्_Destructor(z, xDel, 0);
    प्रत्यागम;
  }
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  यदि( n>0x7fffffff ){
    (शून्य)invoke_मानम्_Destructor(z, xDel, pCtx);
  }अन्यथा{
    समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, (पूर्णाङ्क)n, 0, xDel);
  }
}
शून्य sqlite3_परिणाम_double(sqlite3_context *pCtx, द्विगुण rVal){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  sqlite3_Vdbe_Mem_समुच्चयः_Double(pCtx->p_बहिर्गमनम्, rVal);
}
शून्य sqlite3_परिणाम_error(sqlite3_context *pCtx, ध्रुव वर्ण *z, पूर्णाङ्क n){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_समुच्चयः_Str(pCtx->p_बहिर्गमनम्, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
शून्य sqlite3_परिणाम_error16(sqlite3_context *pCtx, ध्रुव शून्य *z, पूर्णाङ्क n){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_समुच्चयः_Str(pCtx->p_बहिर्गमनम्, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#यदि_अन्तः
शून्य sqlite3_परिणाम_int(sqlite3_context *pCtx, पूर्णाङ्क iVal){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  sqlite3_Vdbe_Mem_समुच्चयः_Int64(pCtx->p_बहिर्गमनम्, (i64)iVal);
}
शून्य sqlite3_परिणाम_int64(sqlite3_context *pCtx, i64 iVal){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  sqlite3_Vdbe_Mem_समुच्चयः_Int64(pCtx->p_बहिर्गमनम्, iVal);
}
शून्य sqlite3_परिणाम_null(sqlite3_context *pCtx){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  sqlite3_Vdbe_Mem_समुच्चयः_Null(pCtx->p_बहिर्गमनम्);
}
शून्य sqlite3_परिणाम_pointer(
  sqlite3_context *pCtx,
  शून्य *pPtr,
  ध्रुव वर्ण *zPType,
  शून्य (*xDestructor)(शून्य*)
){
  Mem *p_बहिर्गमनम्;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ){
    invoke_मानम्_Destructor(pPtr, xDestructor, 0);
    प्रत्यागम;
  }
#यदि_अन्तः
  p_बहिर्गमनम् = pCtx->p_बहिर्गमनम्;
  प्रतिज्ञा( sqlite3_mutex_held(p_बहिर्गमनम्->db->mutex) );
  sqlite3VdbeMemRelease(p_बहिर्गमनम्);
  p_बहिर्गमनम्->flags = MEM_Null;
  sqlite3_Vdbe_Mem_समुच्चयः_Pointer(p_बहिर्गमनम्, pPtr, zPType, xDestructor);
}
शून्य sqlite3_परिणाम_subtype(sqlite3_context *pCtx, अचिह्नित पूर्णाङ्क eSubtype){
  Mem *p_बहिर्गमनम्;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
#यदि defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  यदि( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_परिणाम_SUBTYPE)==0
  ){
    वर्ण zErr[200];
    sqlite3_snprintf(परिमाण(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_परिणाम_error(pCtx, zErr, -1);
    प्रत्यागम;
  }
#यदि_अन्तः /* SQLITE_STRICT_SUBTYPE */
  p_बहिर्गमनम् = pCtx->p_बहिर्गमनम्;
  प्रतिज्ञा( sqlite3_mutex_held(p_बहिर्गमनम्->db->mutex) );
  p_बहिर्गमनम्->eSubtype = eSubtype & 0xff;
  p_बहिर्गमनम्->flags |= MEM_Subtype;
}
शून्य sqlite3_परिणाम_text(
  sqlite3_context *pCtx,
  ध्रुव वर्ण *z,
  पूर्णाङ्क n,
  शून्य (*xDel)(शून्य *)
){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ){
    invoke_मानम्_Destructor(z, xDel, 0);
    प्रत्यागम;
  }
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
शून्य sqlite3_परिणाम_text64(
  sqlite3_context *pCtx,
  ध्रुव वर्ण *z,
  sqlite3_uint64 n,
  शून्य (*xDel)(शून्य *),
  अचिह्नित वर्ण enc
){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ){
    invoke_मानम्_Destructor(z, xDel, 0);
    प्रत्यागम;
  }
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  यदि( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    यदि( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  यदि( n>0x7fffffff ){
    (शून्य)invoke_मानम्_Destructor(z, xDel, pCtx);
  }अन्यथा{
    समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, (पूर्णाङ्क)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->p_बहिर्गमनम्);
  }
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
शून्य sqlite3_परिणाम_text16(
  sqlite3_context *pCtx,
  ध्रुव शून्य *z,
  पूर्णाङ्क n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
शून्य sqlite3_परिणाम_text16be(
  sqlite3_context *pCtx,
  ध्रुव शून्य *z,
  पूर्णाङ्क n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
शून्य sqlite3_परिणाम_text16le(
  sqlite3_context *pCtx,
  ध्रुव शून्य *z,
  पूर्णाङ्क n,
  शून्य (*xDel)(शून्य *)
){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  समुच्चयः_परिणाम_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
शून्य sqlite3_परिणाम_मानम्(sqlite3_context *pCtx, sqlite3_मानम् *p_मानम्){
  Mem *p_बहिर्गमनम्;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
  यदि( p_मानम्==0 ){
    sqlite3_परिणाम_null(pCtx);
    प्रत्यागम;
  }
#यदि_अन्तः
  p_बहिर्गमनम् = pCtx->p_बहिर्गमनम्;
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  sqlite3VdbeMemCopy(p_बहिर्गमनम्, p_मानम्);
  sqlite3VdbeChangeEncoding(p_बहिर्गमनम्, pCtx->enc);
  यदि( sqlite3VdbeMemTooBig(p_बहिर्गमनम्) ){
    sqlite3_परिणाम_error_toobig(pCtx);
  }
}
शून्य sqlite3_परिणाम_zeroblob(sqlite3_context *pCtx, पूर्णाङ्क n){
  sqlite3_परिणाम_zeroblob64(pCtx, n>0 ? n : 0);
}
पूर्णाङ्क sqlite3_परिणाम_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *p_बहिर्गमनम्;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
  p_बहिर्गमनम् = pCtx->p_बहिर्गमनम्;
  प्रतिज्ञा( sqlite3_mutex_held(p_बहिर्गमनम्->db->mutex) );
  यदि( n>(u64)p_बहिर्गमनम्->db->a_सीमा[SQLITE_सीमा_दीर्घता] ){
    sqlite3_परिणाम_error_toobig(pCtx);
    प्रत्यागम SQLITE_TOOBIG;
  }
#यदि_अपरिभाषितम् SQLITE_OMIT_INCRBLOB
  sqlite3_Vdbe_Mem_समुच्चयः_Zero_Blob(pCtx->p_बहिर्गमनम्, (पूर्णाङ्क)n);
  प्रत्यागम SQLITE_OK;
#अन्यथा
  प्रत्यागम sqlite3_Vdbe_Mem_समुच्चयः_Zero_Blob(pCtx->p_बहिर्गमनम्, (पूर्णाङ्क)n);
#यदि_अन्तः
}
शून्य sqlite3_परिणाम_error_संहिता(sqlite3_context *pCtx, पूर्णाङ्क err_संहिता){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  pCtx->isError = err_संहिता ? err_संहिता : -1;
#यदि_परिभाषितम् SQLITE_DEBUG
  यदि( pCtx->pVdbe ) pCtx->pVdbe->rcApp = err_संहिता;
#यदि_अन्तः
  यदि( pCtx->p_बहिर्गमनम्->flags & MEM_Null ){
    समुच्चयः_परिणाम_Str_Or_Error(pCtx, sqlite3ErrStr(err_संहिता), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
शून्य sqlite3_परिणाम_error_toobig(sqlite3_context *pCtx){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3_Vdbe_Mem_समुच्चयः_Str(pCtx->p_बहिर्गमनम्, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
शून्य sqlite3_परिणाम_error_nomem(sqlite3_context *pCtx){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  sqlite3_Vdbe_Mem_समुच्चयः_Null(pCtx->p_बहिर्गमनम्);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->p_बहिर्गमनम्->db);
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
शून्य sqlite3_परिणाम_str(sqlite3_context *pCtx, sqlite3_str *pStr, पूर्णाङ्क eOwn){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
  यदि( pStr==0 ) प्रत्यागम;
#यदि_अन्तः
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  यदि( pStr->accError==0 ){
    यदि( pStr->nChar==0 ){
      समुच्चयः_परिणाम_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }अन्यथा{
      ध्रुव वर्ण *zText = sqlite3_str_मानम्(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      प्रतिज्ञा( isMalloced(pStr) );
      यदि( eOwn==SQLITE_COPY ){
        समुच्चयः_परिणाम_Str_Or_Error(pCtx, zText, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }अन्यथा{
        समुच्चयः_परिणाम_Str_Or_Error(pCtx, zText, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }अन्यथा यदि( pStr->accError==SQLITE_NOMEM ){
    sqlite3_परिणाम_error_nomem(pCtx);
  }अन्यथा{
    प्रतिज्ञा( pStr->accError==SQLITE_TOOBIG );
    sqlite3_परिणाम_error_toobig(pCtx);
  }
  यदि( eOwn==SQLITE_FINISH ){
    sqlite3_str_free(pStr);
  }
}

#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
शून्य sqlite3_परिणाम_Int_Real(sqlite3_context *pCtx){
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
  यदि( pCtx->p_बहिर्गमनम्->flags & MEM_Int ){
    pCtx->p_बहिर्गमनम्->flags &= ~MEM_Int;
    pCtx->p_बहिर्गमनम्->flags |= MEM_IntReal;
  }
}
#यदि_अन्तः


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
स्थिर पूर्णाङ्क doWalCallbacks(sqlite3 *db){
  पूर्णाङ्क rc = SQLITE_OK;
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  पूर्णाङ्क i;
  प्रति(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    यदि( pBt ){
      पूर्णाङ्क n_प्रविष्टिः;
      sqlite3BtreeEnter(pBt);
      n_प्रविष्टिः = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      यदि( n_प्रविष्टिः>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, n_प्रविष्टिः);
      }
    }
  }
#अन्यथा
  अप्रयुक्तम्_PARAMETER(db);
#यदि_अन्तः
  प्रत्यागम rc;
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
स्थिर पूर्णाङ्क sqlite3_चरणः_2(Vdbe *p){
  sqlite3 *db;
  पूर्णाङ्क rc;

  प्रतिज्ञा(p);
  db = p->db;
  यदि( p->eVdbeState!=VDBE_सततम्_STATE ){
    restart_चरणः:
    यदि( p->eVdbeState==VDBE_READY_STATE ){
      यदि( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        यदि( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        गच्छ अन्तः_सम्बन्धि_चरणः;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      यदि( db->n_Vdbe_सक्रिय==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      प्रतिज्ञा( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE
      यदि( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      }अन्यथा{
        प्रतिज्ञा( p->startTime==0 );
      }
#यदि_अन्तः

      db->n_Vdbe_सक्रिय++;
      यदि( p->readOnly==0 ) db->nVdbeWrite++;
      यदि( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_सततम्_STATE;
    }अन्यथा

    यदि( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
#यदि_परिभाषितम् SQLITE_OMIT_AUTORESET
      यदि( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }अन्यथा{
        प्रत्यागम SQLITE_MISUSE_BKPT;
      }
#अन्यथा
      sqlite3_reset((sqlite3_stmt*)p);
#यदि_अन्तः
      प्रतिज्ञा( p->eVdbeState==VDBE_READY_STATE );
      गच्छ restart_चरणः;
    }
  }

#यदि_परिभाषितम् SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN
  यदि( p->explain ){
    rc = sqlite3VdbeList(p);
  }अन्यथा
#यदि_अन्तः /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  यदि( rc==SQLITE_ROW ){
    प्रतिज्ञा( p->rc==SQLITE_OK );
    प्रतिज्ञा( db->mallocFailed==0 );
    db->err_संहिता = SQLITE_ROW;
    प्रत्यागम SQLITE_ROW;
  }अन्यथा{
#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#यदि_अन्तः
    p->p_परिणाम_Row = 0;
    यदि( rc==SQLITE_समाप्तम् && db->autoCommit ){
      प्रतिज्ञा( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      यदि( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }अन्यथा यदि( rc!=SQLITE_समाप्तम् && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->err_संहिता = rc;
  यदि( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    यदि( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
अन्तः_सम्बन्धि_चरणः:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  प्रतिज्ञा( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ROW  || rc==SQLITE_समाप्तम्   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  प्रत्यागम (rc&db->err_आवरणम्);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
पूर्णाङ्क sqlite3_चरणः(sqlite3_stmt *pStmt){
  पूर्णाङ्क rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  पूर्णाङ्क cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  यदि( vdbeSafetyNotNull(v) ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  यावत्( (rc = sqlite3_चरणः_2(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    पूर्णाङ्क savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    यदि( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ध्रुव वर्ण *zErr = (ध्रुव वर्ण *)sqlite3_मानम्_text(db->pErr);
      sqlite3DbFree(db, v->z_Err_सन्देशः);
      यदि( !db->mallocFailed ){
        v->z_Err_सन्देशः = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } अन्यथा {
        v->z_Err_सन्देशः = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      भङ्ग;
    }
    sqlite3_reset(pStmt);
    यदि( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    प्रतिज्ञा( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
शून्य *sqlite3_user_दत्त(sqlite3_context *p){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( p==0 ) प्रत्यागम 0;
#यदि_अन्तः
  प्रतिज्ञा( p && p->pFunc );
  प्रत्यागम p->pFunc->p_User_दत्त;
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
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( p==0 ) प्रत्यागम 0;
#अन्यथा
  प्रतिज्ञा( p && p->p_बहिर्गमनम् );
#यदि_अन्तः
  प्रत्यागम p->p_बहिर्गमनम्->db;
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
पूर्णाङ्क sqlite3_vtab_nochange(sqlite3_context *p){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( p==0 ) प्रत्यागम 0;
#अन्यथा
  प्रतिज्ञा( p );
#यदि_अन्तः
  प्रत्यागम sqlite3_मानम्_nochange(p->p_बहिर्गमनम्);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
शून्य sqlite3_Vdbe_मानम्_List_Free(शून्य *p_गन्तव्यम्_Delete){
  sqlite3_free(p_गन्तव्यम्_Delete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
स्थिर पूर्णाङ्क मानम्_From_मानम्_List(
  sqlite3_मानम् *pVal,        /* Pointer to the ValueList object */
  sqlite3_मानम् **pp_बहिर्गमनम्,      /* Store the next value from the list here */
  पूर्णाङ्क b_अग्र                   /* 1 for _next(). 0 for _first() */
){
  पूर्णाङ्क rc;
  मानम्_List *pRhs;

  *pp_बहिर्गमनम् = 0;
  यदि( pVal==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
  यदि( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_मानम्_List_Free ){
    प्रत्यागम SQLITE_ERROR;
  }अन्यथा{
    प्रतिज्ञा( (pVal->flags&(MEM_Type_आवरणम्|MEM_पदांशः|MEM_Subtype)) ==
                 (MEM_Null|MEM_पदांशः|MEM_Subtype) );
    प्रतिज्ञा( pVal->eSubtype=='p' );
    प्रतिज्ञा( pVal->u.zPType!=0 && वाक्यं_तुलय(pVal->u.zPType,"ValueList")==0 );
    pRhs = (मानम्_List*)pVal->z;
  }
  यदि( b_अग्र ){
    rc = sqlite3_Btree_अग्र(pRhs->pCsr, 0);
  }अन्यथा{
    पूर्णाङ्क dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);
    प्रतिज्ञा( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    यदि( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_समाप्तम्;
  }
  यदि( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    स्मृतिं_पूरय(&sMem, 0, परिमाण(sMem));
    sz = sqlite3_Btree_Payload_आकार(pRhs->pCsr);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_विस्थापनम्(pRhs->pCsr,sz,&sMem);
    यदि( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_मानम् *p_बहिर्गमनम् = pRhs->p_बहिर्गमनम्;
      पूर्णाङ्क iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, p_बहिर्गमनम्);
      p_बहिर्गमनम्->enc = ENC(p_बहिर्गमनम्->db);
      यदि( (p_बहिर्गमनम्->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(p_बहिर्गमनम्) ){
        rc = SQLITE_NOMEM;
      }अन्यथा{
        *pp_बहिर्गमनम् = p_बहिर्गमनम्;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  प्रत्यागम rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
पूर्णाङ्क sqlite3_vtab_अन्तर्_first(sqlite3_मानम् *pVal, sqlite3_मानम् **pp_बहिर्गमनम्){
  प्रत्यागम मानम्_From_मानम्_List(pVal, pp_बहिर्गमनम्, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
पूर्णाङ्क sqlite3_vtab_अन्तर्_अग्र(sqlite3_मानम् *pVal, sqlite3_मानम् **pp_बहिर्गमनम्){
  प्रत्यागम मानम्_From_मानम्_List(pVal, pp_बहिर्गमनम्, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p){
  पूर्णाङ्क rc;
#यदि_अपरिभाषितम् SQLITE_ENABLE_STAT4
  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;
  प्रतिज्ञा( p->pVdbe!=0 );
#अन्यथा
  sqlite3_int64 iTime = 0;
  sqlite3_int64 *piTime = p->pVdbe!=0 ? &p->pVdbe->iCurrentTime : &iTime;
#यदि_अन्तः
  यदि( *piTime==0 ){
    rc = sqlite3OsCurrentTimeInt64(p->p_बहिर्गमनम्->db->pVfs, piTime);
    यदि( rc ) *piTime = 0;
  }
  प्रत्यागम *piTime;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
स्थिर SQLITE_NOINLINE शून्य *createAggContext(sqlite3_context *p, पूर्णाङ्क nByte){
  Mem *pMem = p->pMem;
  प्रतिज्ञा( (pMem->flags & MEM_Agg)==0 );
  यदि( nByte<=0 ){
    sqlite3_Vdbe_Mem_समुच्चयः_Null(pMem);
    pMem->z = 0;
  }अन्यथा{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    यदि( pMem->z ){
      स्मृतिं_पूरय(pMem->z, 0, nByte);
    }
  }
  प्रत्यागम (शून्य*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
शून्य *sqlite3_aggregate_context(sqlite3_context *p, पूर्णाङ्क nByte){
  प्रतिज्ञा( p && p->pFunc && p->pFunc->xFinalize );
  प्रतिज्ञा( sqlite3_mutex_held(p->p_बहिर्गमनम्->db->mutex) );
  testcase( nByte<0 );
  यदि( (p->pMem->flags & MEM_Agg)==0 ){
    प्रत्यागम createAggContext(p, nByte);
  }अन्यथा{
    प्रत्यागम (शून्य*)p->pMem->z;
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
शून्य *sqlite3_get_auxdata(sqlite3_context *pCtx, पूर्णाङ्क iArg){
  Aux_दत्त *p_Aux_दत्त;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम 0;
#यदि_अन्तः
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
#यदि SQLITE_ENABLE_STAT4
  यदि( pCtx->pVdbe==0 ) प्रत्यागम 0;
#अन्यथा
  प्रतिज्ञा( pCtx->pVdbe!=0 );
#यदि_अन्तः
  प्रति(p_Aux_दत्त=pCtx->pVdbe->p_Aux_दत्त; p_Aux_दत्त; p_Aux_दत्त=p_Aux_दत्त->p_अग्र_Aux){
    यदि(  p_Aux_दत्त->iAuxArg==iArg && (p_Aux_दत्त->iAuxOp==pCtx->iOp || iArg<0) ){
      प्रत्यागम p_Aux_दत्त->pAux;
    }
  }
  प्रत्यागम 0;
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
शून्य sqlite3_समुच्चयः_auxdata(
  sqlite3_context *pCtx,
  पूर्णाङ्क iArg,
  शून्य *pAux,
  शून्य (*xDelete)(शून्य*)
){
  Aux_दत्त *p_Aux_दत्त;
  Vdbe *pVdbe;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pCtx==0 ) प्रत्यागम;
#यदि_अन्तः
  pVdbe= pCtx->pVdbe;
  प्रतिज्ञा( sqlite3_mutex_held(pCtx->p_बहिर्गमनम्->db->mutex) );
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
  यदि( pVdbe==0 ) गच्छ failed;
#अन्यथा
  प्रतिज्ञा( pVdbe!=0 );
#यदि_अन्तः

  प्रति(p_Aux_दत्त=pVdbe->p_Aux_दत्त; p_Aux_दत्त; p_Aux_दत्त=p_Aux_दत्त->p_अग्र_Aux){
    यदि( p_Aux_दत्त->iAuxArg==iArg && (p_Aux_दत्त->iAuxOp==pCtx->iOp || iArg<0) ){
      भङ्ग;
    }
  }
  यदि( p_Aux_दत्त==0 ){
    p_Aux_दत्त = sqlite3DbMallocZero(pVdbe->db, परिमाण(Aux_दत्त));
    यदि( !p_Aux_दत्त ) गच्छ failed;
    p_Aux_दत्त->iAuxOp = pCtx->iOp;
    p_Aux_दत्त->iAuxArg = iArg;
    p_Aux_दत्त->p_अग्र_Aux = pVdbe->p_Aux_दत्त;
    pVdbe->p_Aux_दत्त = p_Aux_दत्त;
    यदि( pCtx->isError==0 ) pCtx->isError = -1;
  }अन्यथा यदि( p_Aux_दत्त->xDeleteAux ){
    p_Aux_दत्त->xDeleteAux(p_Aux_दत्त->pAux);
  }

  p_Aux_दत्त->pAux = pAux;
  p_Aux_दत्त->xDeleteAux = xDelete;
  प्रत्यागम;

failed:
  यदि( xDelete ){
    xDelete(pAux);
  }
}

#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
पूर्णाङ्क sqlite3_aggregate_गणनम्(sqlite3_context *p){
  प्रतिज्ञा( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  प्रत्यागम p->pMem->n;
}
#यदि_अन्तः

/*
** Return the number of columns in the result set for the statement pStmt.
*/
पूर्णाङ्क sqlite3_column_गणनम्(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  यदि( pVm==0 ) प्रत्यागम 0;
  प्रत्यागम pVm->nResColumn;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
पूर्णाङ्क sqlite3_दत्त_गणनम्(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  यदि( pVm==0 || pVm->p_परिणाम_Row==0 ) प्रत्यागम 0;
  प्रत्यागम pVm->nResColumn;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
स्थिर ध्रुव Mem *column_Null_मानम्(शून्य){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  स्थिर ध्रुव Mem nullMem
#यदि defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#यदि_अन्तः
    = {
        /* .u          = */ {0},
        /* .z          = */ (वर्ण*)0,
        /* .n          = */ (पूर्णाङ्क)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (पूर्णाङ्क)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (वर्ण*)0,
        /* .xDel       = */ (शून्य(*)(शून्य*))0,
#यदि_परिभाषितम् SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#यदि_अन्तः
      };
  प्रत्यागम &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
स्थिर Mem *columnMem(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  Vdbe *pVm;
  Mem *p_बहिर्गमनम्;

  pVm = (Vdbe *)pStmt;
  यदि( pVm==0 ) प्रत्यागम (Mem*)column_Null_मानम्();
  प्रतिज्ञा( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  यदि( pVm->p_परिणाम_Row!=0 && i<pVm->nResColumn && i>=0 ){
    p_बहिर्गमनम् = &pVm->p_परिणाम_Row[i];
  }अन्यथा{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    p_बहिर्गमनम् = (Mem*)column_Null_मानम्();
  }
  प्रत्यागम p_बहिर्गमनम्;
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
  यदि( p ){
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
ध्रुव शून्य *sqlite3_column_blob(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  ध्रुव शून्य *val;
  val = sqlite3_मानम्_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
पूर्णाङ्क sqlite3_column_अष्टकानि(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  पूर्णाङ्क val = sqlite3_मानम्_अष्टकानि( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
पूर्णाङ्क sqlite3_column_bytes16(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  पूर्णाङ्क val = sqlite3_मानम्_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
द्विगुण sqlite3_column_double(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  द्विगुण val = sqlite3_मानम्_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
पूर्णाङ्क sqlite3_column_int(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  पूर्णाङ्क val = sqlite3_मानम्_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  sqlite_int64 val = sqlite3_मानम्_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
ध्रुव अचिह्नित वर्ण *sqlite3_column_text(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  ध्रुव अचिह्नित वर्ण *val = sqlite3_मानम्_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
sqlite3_मानम् *sqlite3_column_मानम्(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  Mem *p_बहिर्गमनम् = columnMem(pStmt, i);
  यदि( p_बहिर्गमनम्->flags&MEM_Static ){
    p_बहिर्गमनम्->flags &= ~MEM_Static;
    p_बहिर्गमनम्->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  प्रत्यागम (sqlite3_मानम् *)p_बहिर्गमनम्;
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_column_text16(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  ध्रुव शून्य *val = sqlite3_मानम्_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम val;
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
पूर्णाङ्क sqlite3_column_type(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  पूर्णाङ्क iType = sqlite3_मानम्_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  प्रत्यागम iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
स्थिर ध्रुव वर्ण * ध्रुव azExplainColNames8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
स्थिर ध्रुव u16 azExplainColNames16data[] = {
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
स्थिर ध्रुव u8 iExplainColNames16[] = {
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
स्थिर ध्रुव शून्य *columnName(
  sqlite3_stmt *pStmt,     /* The statement */
  पूर्णाङ्क N,                   /* Which column to get the name for */
  पूर्णाङ्क useUtf16,            /* True to return the name as UTF16 */
  पूर्णाङ्क useType              /* What type of name */
){
  ध्रुव शून्य *ret;
  Vdbe *p;
  पूर्णाङ्क n;
  sqlite3 *db;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pStmt==0 ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  यदि( N<0 ) प्रत्यागम 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  प्रतिज्ञा( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  यदि( p->explain ){
    यदि( useType>0 ) गच्छ column_Name_अन्तः;
    n = p->explain==1 ? 8 : 4;
    यदि( N>=n ) गच्छ column_Name_अन्तः;
    यदि( useUtf16 ){
      पूर्णाङ्क i = iExplainColNames16[N + 8*p->explain - 8];
      ret = (शून्य*)&azExplainColNames16data[i];
    }अन्यथा{
      ret = (शून्य*)azExplainColNames8[N + 8*p->explain - 8];
    }
    गच्छ column_Name_अन्तः;
  }
  n = p->nResColumn;
  यदि( N<n ){
    u8 प्राक्_malloc_Failed = db->mallocFailed;
    N += useType*n;
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
    यदि( useUtf16 ){
      ret = sqlite3_मानम्_text16((sqlite3_मानम्*)&p->aColName[N]);
    }अन्यथा
#यदि_अन्तः
    {
      ret = sqlite3_मानम्_text((sqlite3_मानम्*)&p->aColName[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    प्रतिज्ञा( db->mallocFailed==0 || db->mallocFailed==1 );
    यदि( db->mallocFailed > प्राक्_malloc_Failed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
column_Name_अन्तः:
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ध्रुव वर्ण *sqlite3_column_name(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 0, COLNAME_NAME);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_column_name16(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 1, COLNAME_NAME);
}
#यदि_अन्तः

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#यदि defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ध्रुव वर्ण *sqlite3_column_decltype(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 0, COLNAME_DECLTYPE);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_column_decltype16(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 1, COLNAME_DECLTYPE);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
#यदि_अन्तः /* SQLITE_OMIT_DECLTYPE */

#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ध्रुव वर्ण *sqlite3_column_database_name(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 0, COLNAME_DATABASE);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_column_database_name16(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 1, COLNAME_DATABASE);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ध्रुव वर्ण *sqlite3_column_सारणी_name(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 0, COLNAME_सारणी);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_column_सारणी_name16(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 1, COLNAME_सारणी);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ध्रुव वर्ण *sqlite3_column_origin_name(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 0, COLNAME_COLUMN);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
ध्रुव शून्य *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, पूर्णाङ्क N){
  प्रत्यागम columnName(pStmt, N, 1, COLNAME_COLUMN);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
#यदि_अन्तः /* SQLITE_ENABLE_COLUMN_METADATA */


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
स्थिर पूर्णाङ्क vdbeUnbind(Vdbe *p, अचिह्नित पूर्णाङ्क i){
  Mem *pVar;
  यदि( vdbeSafetyNotNull(p) ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  यदि( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  यदि( i>=(अचिह्नित पूर्णाङ्क)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    प्रत्यागम SQLITE_RANGE;
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
  यदि( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    यदि( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->err_संहिता = SQLITE_OK;

  प्रत्यागम SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
स्थिर पूर्णाङ्क bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  पूर्णाङ्क i,                 /* Index of the parameter to bind */
  ध्रुव शून्य *z_दत्त,     /* Pointer to the data to be bound */
  i64 n_दत्त,             /* Number of bytes of data to be bound */
  शून्य (*xDel)(शून्य*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  पूर्णाङ्क rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  यदि( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    यदि( z_दत्त!=0 ){
      pVar = &p->aVar[i-1];
      यदि( encoding==SQLITE_UTF8 ){
        rc = sqlite3_Vdbe_Mem_समुच्चयः_Text(pVar, z_दत्त, n_दत्त, xDel);
      }अन्यथा यदि( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        प्रतिज्ञा( z_दत्त==0
             || n_दत्त<0 
             || n_दत्त>pVar->db->a_सीमा[SQLITE_सीमा_दीर्घता]
             || ((u8*)z_दत्त)[n_दत्त]==0
        );
        rc = sqlite3_Vdbe_Mem_समुच्चयः_Text(pVar, z_दत्त, n_दत्त, xDel);
        pVar->flags |= MEM_पदांशः;
      }अन्यथा{
        rc = sqlite3_Vdbe_Mem_समुच्चयः_Str(pVar, z_दत्त, n_दत्त, encoding, xDel);
        यदि( encoding==0 ) pVar->enc = ENC(p->db);
      }
      यदि( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      यदि( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }अन्यथा यदि( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((शून्य*)z_दत्त);
  }
  प्रत्यागम rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
पूर्णाङ्क sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  पूर्णाङ्क i,
  ध्रुव शून्य *z_दत्त,
  पूर्णाङ्क n_दत्त,
  शून्य (*xDel)(शून्य*)
){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( n_दत्त<0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
  प्रत्यागम bindText(pStmt, i, z_दत्त, n_दत्त, xDel, 0);
}
पूर्णाङ्क sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  पूर्णाङ्क i,
  ध्रुव शून्य *z_दत्त,
  sqlite3_uint64 n_दत्त,
  शून्य (*xDel)(शून्य*)
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  प्रत्यागम bindText(pStmt, i, z_दत्त, n_दत्त, xDel, 0);
}
पूर्णाङ्क sqlite3_bind_double(sqlite3_stmt *pStmt, पूर्णाङ्क i, द्विगुण r_मानम्){
  पूर्णाङ्क rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  यदि( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_समुच्चयः_Double(&p->aVar[i-1], r_मानम्);
    sqlite3_mutex_leave(p->db->mutex);
  }
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_bind_int(sqlite3_stmt *p, पूर्णाङ्क i, पूर्णाङ्क i_मानम्){
  प्रत्यागम sqlite3_bind_int64(p, i, (i64)i_मानम्);
}
पूर्णाङ्क sqlite3_bind_int64(sqlite3_stmt *pStmt, पूर्णाङ्क i, sqlite_int64 i_मानम्){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  यदि( vdbeSafetyNotNull(p) ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  यदि( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  यदि( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    प्रत्यागम SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  यदि( p->expmask!=0 ){
    u32 expire_आवरणम् = i>=32 ? 0x80000000 : (u32)1 << i;
    प्रतिज्ञा( (p->expmask & p->smimask)==p->smimask );
    यदि( (p->expmask & expire_आवरणम्)!=0 ){
      यदि( (p->smimask & expire_आवरणम्)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        पूर्णाङ्क sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        पूर्णाङ्क sm2 = i_मानम्>=0 && i_मानम्<=1;
        यदि( sm1!=sm2 ){
          p->expired = 1;
        }
      }अन्यथा यदि( (pVar->flags & MEM_Int)==0 || i_मानम्!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3_Vdbe_Mem_समुच्चयः_Int64(pVar, i_मानम्);
  sqlite3_mutex_leave(p->db->mutex);
  प्रत्यागम SQLITE_OK;
}
पूर्णाङ्क sqlite3_bind_null(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  पूर्णाङ्क rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  यदि( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_bind_pointer(
  sqlite3_stmt *pStmt,
  पूर्णाङ्क i,
  शून्य *pPtr,
  ध्रुव वर्ण *zPTtype,
  शून्य (*xDestructor)(शून्य*)
){
  पूर्णाङ्क rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  यदि( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_समुच्चयः_Pointer(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }अन्यथा यदि( xDestructor ){
    xDestructor(pPtr);
  }
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_bind_text(
  sqlite3_stmt *pStmt,
  पूर्णाङ्क i,
  ध्रुव वर्ण *z_दत्त,
  पूर्णाङ्क n_दत्त,
  शून्य (*xDel)(शून्य*)
){
  प्रत्यागम bindText(pStmt, i, z_दत्त, n_दत्त, xDel, SQLITE_UTF8);
}
पूर्णाङ्क sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  पूर्णाङ्क i,
  ध्रुव वर्ण *z_दत्त,
  sqlite3_uint64 n_दत्त,
  शून्य (*xDel)(शून्य*),
  अचिह्नित वर्ण enc
){
  प्रतिज्ञा( xDel!=SQLITE_DYNAMIC );
  यदि( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    यदि( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_दत्त &= ~(u64)1;
  }
  प्रत्यागम bindText(pStmt, i, z_दत्त, n_दत्त, xDel, enc);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
पूर्णाङ्क sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  पूर्णाङ्क i,
  ध्रुव शून्य *z_दत्त,
  पूर्णाङ्क n,
  शून्य (*xDel)(शून्य*)
){
  प्रत्यागम bindText(pStmt, i, z_दत्त, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
पूर्णाङ्क sqlite3_bind_मानम्(sqlite3_stmt *pStmt, पूर्णाङ्क i, ध्रुव sqlite3_मानम् *p_मानम्){
  पूर्णाङ्क rc;
  विकल्प( sqlite3_मानम्_type((sqlite3_मानम्*)p_मानम्) ){
    पक्ष SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_मानम्->u.i);
      भङ्ग;
    }
    पक्ष SQLITE_FLOAT: {
      प्रतिज्ञा( p_मानम्->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_मानम्->flags & MEM_Real) ? p_मानम्->u.r : (द्विगुण)p_मानम्->u.i
      );
      भङ्ग;
    }
    पक्ष SQLITE_BLOB: {
      यदि( p_मानम्->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_मानम्->u.nZero);
      }अन्यथा{
        rc = sqlite3_bind_blob(pStmt, i, p_मानम्->z, p_मानम्->n,SQLITE_TRANSIENT);
      }
      भङ्ग;
    }
    पक्ष SQLITE_TEXT: {
      rc = bindText(pStmt,i,  p_मानम्->z, p_मानम्->n, SQLITE_TRANSIENT,
                              p_मानम्->enc);
      भङ्ग;
    }
    स्वभाव: {
      rc = sqlite3_bind_null(pStmt, i);
      भङ्ग;
    }
  }
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, पूर्णाङ्क i, पूर्णाङ्क n){
  पूर्णाङ्क rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  यदि( rc==SQLITE_OK ){
    प्रतिज्ञा( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#यदि_अपरिभाषितम् SQLITE_OMIT_INCRBLOB
    sqlite3_Vdbe_Mem_समुच्चयः_Zero_Blob(&p->aVar[i-1], n);
#अन्यथा
    rc = sqlite3_Vdbe_Mem_समुच्चयः_Zero_Blob(&p->aVar[i-1], n);
#यदि_अन्तः
    sqlite3_mutex_leave(p->db->mutex);
  }
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, पूर्णाङ्क i, sqlite3_uint64 n){
  पूर्णाङ्क rc;
  Vdbe *p = (Vdbe *)pStmt;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( p==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
  sqlite3_mutex_enter(p->db->mutex);
  यदि( n>(u64)p->db->a_सीमा[SQLITE_सीमा_दीर्घता] ){
    rc = SQLITE_TOOBIG;
  }अन्यथा{
    प्रतिज्ञा( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  प्रत्यागम rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
पूर्णाङ्क sqlite3_bind_parameter_गणनम्(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  प्रत्यागम p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ध्रुव वर्ण *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, पूर्णाङ्क i){
  Vdbe *p = (Vdbe*)pStmt;
  यदि( p==0 ) प्रत्यागम 0;
  प्रत्यागम sqlite3_VList_अङ्कः_गन्तव्यम्_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
पूर्णाङ्क sqlite3_Vdbe_Parameter_सूचकांक(Vdbe *p, ध्रुव वर्ण *zName, पूर्णाङ्क nName){
  यदि( p==0 || zName==0 ) प्रत्यागम 0;
  प्रत्यागम sqlite3_VList_Name_गन्तव्यम्_अङ्कः(p->pVList, zName, nName);
}
पूर्णाङ्क sqlite3_bind_parameter_सूचकांक(sqlite3_stmt *pStmt, ध्रुव वर्ण *zName){
  प्रत्यागम sqlite3_Vdbe_Parameter_सूचकांक((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
पूर्णाङ्क sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_गन्तव्यम्_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_गन्तव्यम् = (Vdbe*)p_गन्तव्यम्_Stmt;
  पूर्णाङ्क i;
  प्रतिज्ञा( p_गन्तव्यम्->db==pFrom->db );
  प्रतिज्ञा( p_गन्तव्यम्->nVar==pFrom->nVar );
  sqlite3_mutex_enter(p_गन्तव्यम्->db->mutex);
  प्रति(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&p_गन्तव्यम्->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(p_गन्तव्यम्->db->mutex);
  प्रत्यागम SQLITE_OK;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
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
पूर्णाङ्क sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_गन्तव्यम्_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_गन्तव्यम् = (Vdbe*)p_गन्तव्यम्_Stmt;
  यदि( pFrom->nVar!=p_गन्तव्यम्->nVar ){
    प्रत्यागम SQLITE_ERROR;
  }
  प्रतिज्ञा( (p_गन्तव्यम्->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p_गन्तव्यम्->expmask==0 );
  यदि( p_गन्तव्यम्->expmask ){
    p_गन्तव्यम्->expired = 1;
  }
  प्रतिज्ञा( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  यदि( pFrom->expmask ){
    pFrom->expired = 1;
  }
  प्रत्यागम sqlite3TransferBindings(pFromStmt, p_गन्तव्यम्_Stmt);
}
#यदि_अन्तः

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  प्रत्यागम pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
पूर्णाङ्क sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  प्रत्यागम pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
पूर्णाङ्क sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  प्रत्यागम pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
पूर्णाङ्क sqlite3_stmt_explain(sqlite3_stmt *pStmt, पूर्णाङ्क e_बहुलकम्){
  Vdbe *v = (Vdbe*)pStmt;
  पूर्णाङ्क rc;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( pStmt==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
  sqlite3_mutex_enter(v->db->mutex);
  यदि( ((पूर्णाङ्क)v->explain)==e_बहुलकम् ){
    rc = SQLITE_OK;
  }अन्यथा यदि( e_बहुलकम्<0 || e_बहुलकम्>2 ){
    rc = SQLITE_ERROR;
  }अन्यथा यदि( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा यदि( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }अन्यथा यदि( v->nMem>=10 && (e_बहुलकम्!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = e_बहुलकम्;
    rc = SQLITE_OK;
  }अन्यथा{
    पूर्णाङ्क saved_बहुलकम् = v->explain;
    v->explain = e_बहुलकम्;
    rc = sqlite3Reprepare(v);
    यदि( rc ){
      v->explain = saved_बहुलकम्;
    }अन्यथा{
      v->haveEqpOps = e_बहुलकम्==2;
    }
  }
  यदि( v->explain ){
    v->nResColumn = 12 - 4*v->explain;
  }अन्यथा{
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  प्रत्यागम rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
पूर्णाङ्क sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  प्रत्यागम v!=0 && v->eVdbeState==VDBE_सततम्_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_अग्र_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_अग्र;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(pDb) ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  sqlite3_mutex_enter(pDb->mutex);
  यदि( pStmt==0 ){
    p_अग्र = (sqlite3_stmt*)pDb->pVdbe;
  }अन्यथा{
    p_अग्र = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  प्रत्यागम p_अग्र;
}

/*
** Return the value of a status counter for a prepared statement
*/
पूर्णाङ्क sqlite3_stmt_स्थितिः(sqlite3_stmt *pStmt, पूर्णाङ्क op, पूर्णाङ्क resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_आकार(pVdbe->a_गणक)))
  ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  यदि( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pn_अष्टकानि_Freed = (पूर्णाङ्क*)&v;
    प्रतिज्ञा( db->lookaside.p_अन्तः==db->lookaside.p_True_अन्तः );
    db->lookaside.p_अन्तः = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pn_अष्टकानि_Freed = 0;
    db->lookaside.p_अन्तः = db->lookaside.p_True_अन्तः;
    sqlite3_mutex_leave(db->mutex);
  }अन्यथा{
    v = pVdbe->a_गणक[op];
    यदि( resetFlag ) pVdbe->a_गणक[op] = 0;
  }
  प्रत्यागम (पूर्णाङ्क)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ध्रुव वर्ण *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  प्रत्यागम p ? p->zSql : 0;
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
वर्ण *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#यदि_परिभाषितम् SQLITE_OMIT_TRACE
  प्रत्यागम 0;
#अन्यथा
  वर्ण *z = 0;
  ध्रुव वर्ण *zSql = sqlite3_sql(pStmt);
  यदि( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  प्रत्यागम z;
#यदि_अन्तः
}

#यदि_परिभाषितम् SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ध्रुव वर्ण *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  यदि( p==0 ) प्रत्यागम 0;
  यदि( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  प्रत्यागम p->zNormSql;
}
#यदि_अन्तः /* SQLITE_ENABLE_NORMALIZE */

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
स्थिर Unpacked_लेखः *vdbe_Unpack_लेखः(
  कुञ्चिका_Info *p_कुञ्चिका_Info,
  पूर्णाङ्क n_कुञ्चिका,
  ध्रुव शून्य *p_कुञ्चिका
){
  Unpacked_लेखः *pRet;           /* Return value */

  pRet = sqlite3_Vdbe_Alloc_Unpacked_लेखः(p_कुञ्चिका_Info);
  यदि( pRet ){
    स्मृतिं_पूरय(pRet->aMem, 0, परिमाण(Mem)*(p_कुञ्चिका_Info->n_कुञ्चिका_क्षेत्रम्+1));
    sqlite3_Vdbe_लेखः_Unpack(n_कुञ्चिका, p_कुञ्चिका, pRet);
  }
  प्रत्यागम pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
पूर्णाङ्क sqlite3_preupdate_old(sqlite3 *db, पूर्णाङ्क iIdx, sqlite3_मानम् **pp_मानम्){
  PreUpdate *p;
  Mem *pMem;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क iStore = 0;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( db==0 || pp_मानम्==0 ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
#यदि_अन्तः
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  यदि( !p || p->op==SQLITE_निवेशय ){
    rc = SQLITE_MISUSE_BKPT;
    गच्छ preupdate_old_बहिर्गमनम्;
  }
  यदि( p->pPk ){
    iStore = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(p->pPk, iIdx);
  }अन्यथा यदि( iIdx >= p->pTab->nCol ){
    rc = SQLITE_MISUSE_BKPT;
    गच्छ preupdate_old_बहिर्गमनम्;
  }अन्यथा{
    iStore = sqlite3_सारणी_Column_गन्तव्यम्_Storage(p->pTab, iIdx);
  }
  यदि( iStore>=p->pCsr->n_क्षेत्रम् || iStore<0 ){
    rc = SQLITE_RANGE;
    गच्छ preupdate_old_बहिर्गमनम्;
  }

  यदि( iIdx==p->pTab->iPKey ){
    *pp_मानम् = pMem = &p->oldipk;
    sqlite3_Vdbe_Mem_समुच्चयः_Int64(pMem, p->iKey1);
  }अन्यथा{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    यदि( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      प्रतिज्ञा( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_आकार(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      यदि( !aRec ) गच्छ preupdate_old_बहिर्गमनम्;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      यदि( rc==SQLITE_OK ){
        स्मृतिं_पूरय(aRec+nRec, 0, 8);
        p->pUnpacked = vdbe_Unpack_लेखः(p->pKeyinfo, nRec, aRec);
        यदि( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      यदि( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        गच्छ preupdate_old_बहिर्गमनम्;
      }
      p->a_लेखः = aRec;
    }

    pMem = *pp_मानम् = &p->pUnpacked->aMem[iStore];
    यदि( iStore>=p->pUnpacked->n_क्षेत्रम् ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      Column *pCol = &p->pTab->aCol[iIdx];
      यदि( pCol->iDflt>0 ){
        यदि( p->apDflt==0 ){
          पूर्णाङ्क nByte;
          प्रतिज्ञा( परिमाण(sqlite3_मानम्*)*UMXV(p->pTab->nCol) < 0x7fffffff );
          nByte = परिमाण(sqlite3_मानम्*)*p->pTab->nCol;
          p->apDflt = (sqlite3_मानम्**)sqlite3DbMallocZero(db, nByte);
          यदि( p->apDflt==0 ) गच्छ preupdate_old_बहिर्गमनम्;
        }
        यदि( p->apDflt[iIdx]==0 ){
          sqlite3_मानम् *pVal = 0;
          व्यञ्जकम् *pDflt;
          प्रतिज्ञा( p->pTab!=0 && Is_Ordinary_सारणी(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[pCol->iDflt-1].p_व्यञ्जकम्;
          rc = sqlite3_मानम्_From_व्यञ्जकम्(db, pDflt, ENC(db), pCol->affinity, &pVal);
          यदि( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_मानम् = p->apDflt[iIdx];
      }अन्यथा{
        *pp_मानम् = (sqlite3_मानम् *)column_Null_मानम्();
      }
    }अन्यथा यदि( p->pTab->aCol[iIdx].affinity==SQLITE_AFF_REAL ){
      यदि( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_बहिर्गमनम्:
  sqlite3Error(db, rc);
  प्रत्यागम sqlite3ApiExit(db, rc);
}
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
पूर्णाङ्क sqlite3_preupdate_गणनम्(sqlite3 *db){
  PreUpdate *p;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#यदि_अन्तः
  प्रत्यागम (p ? p->pKeyinfo->n_कुञ्चिका_क्षेत्रम् : 0);
}
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
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
पूर्णाङ्क sqlite3_preupdate_गभीरता(sqlite3 *db){
  PreUpdate *p;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#यदि_अन्तः
  प्रत्यागम (p ? p->v->nFrame : 0);
}
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
पूर्णाङ्क sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#अन्यथा
  p = db->pPreUpdate;
#यदि_अन्तः
  प्रत्यागम (p ? p->iBlobWrite : -1);
}
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
पूर्णाङ्क sqlite3_preupdate_new(sqlite3 *db, पूर्णाङ्क iIdx, sqlite3_मानम् **pp_मानम्){
  PreUpdate *p;
  पूर्णाङ्क rc = SQLITE_OK;
  Mem *pMem;
  पूर्णाङ्क iStore = 0;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( db==0 || pp_मानम्==0 ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
#यदि_अन्तः
  p = db->pPreUpdate;
  यदि( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    गच्छ preupdate_new_बहिर्गमनम्;
  }
  यदि( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(p->pPk, iIdx);
  }अन्यथा यदि( iIdx >= p->pTab->nCol ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }अन्यथा{
    iStore = sqlite3_सारणी_Column_गन्तव्यम्_Storage(p->pTab, iIdx);
  }

  यदि( iStore>=p->pCsr->n_क्षेत्रम् || iStore<0 ){
    rc = SQLITE_RANGE;
    गच्छ preupdate_new_बहिर्गमनम्;
  }

  यदि( p->op==SQLITE_निवेशय ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    Unpacked_लेखः *pUnpack = p->pNewUnpacked;
    यदि( !pUnpack ){
      Mem *p_दत्त = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_दत्त);
      यदि( rc!=SQLITE_OK ) गच्छ preupdate_new_बहिर्गमनम्;
      pUnpack = vdbe_Unpack_लेखः(p->pKeyinfo, p_दत्त->n, p_दत्त->z);
      यदि( !pUnpack ){
        rc = SQLITE_NOMEM;
        गच्छ preupdate_new_बहिर्गमनम्;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    यदि( iIdx==p->pTab->iPKey ){
      sqlite3_Vdbe_Mem_समुच्चयः_Int64(pMem, p->iKey2);
    }अन्यथा यदि( iStore>=pUnpack->n_क्षेत्रम् ){
      pMem = (sqlite3_मानम् *)column_Null_मानम्();
    }अन्यथा यदि( (p->pTab->aCol)
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
    यदि( !p->aNew ){
      प्रतिज्ञा( परिमाण(Mem)*UMXV(p->pCsr->n_क्षेत्रम्) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, परिमाण(Mem)*p->pCsr->n_क्षेत्रम्);
      यदि( !p->aNew ){
        rc = SQLITE_NOMEM;
        गच्छ preupdate_new_बहिर्गमनम्;
      }
    }
    प्रतिज्ञा( iStore>=0 && iStore<p->pCsr->n_क्षेत्रम् );
    pMem = &p->aNew[iStore];
    यदि( pMem->flags==0 ){
      यदि( iIdx==p->pTab->iPKey ){
        sqlite3_Vdbe_Mem_समुच्चयः_Int64(pMem, p->iKey2);
      }अन्यथा{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        यदि( rc!=SQLITE_OK ) गच्छ preupdate_new_बहिर्गमनम्;
      }
    }
  }
  *pp_मानम् = pMem;

 preupdate_new_बहिर्गमनम्:
  sqlite3Error(db, rc);
  प्रत्यागम sqlite3ApiExit(db, rc);
}
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
पूर्णाङ्क sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  पूर्णाङ्क i_अवलोकनम्,                      /* Index of loop to report on */
  पूर्णाङ्क i_अवलोकनम्_स्थितिः_Op,              /* Which metric to return */
  पूर्णाङ्क flags,
  शून्य *p_बहिर्गमनम्                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  पूर्णाङ्क nOp;
  अवलोकनम्_स्थितिः *p_अवलोकनम् = 0;
  पूर्णाङ्क idx;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( p==0 || p_बहिर्गमनम्==0
      || i_अवलोकनम्_स्थितिः_Op<SQLITE_SCANSTAT_NLOOP
      || i_अवलोकनम्_स्थितिः_Op>SQLITE_SCANSTAT_NCYCLE ){
    प्रत्यागम 1;
  }
#यदि_अन्तः
  aOp = p->aOp;
  nOp = p->nOp;
  यदि( p->pFrame ){
    VdbeFrame *pFrame;
    प्रति(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    aOp = pFrame->aOp;
    nOp = pFrame->nOp;
  }

  यदि( i_अवलोकनम्<0 ){
    पूर्णाङ्क ii;
    यदि( i_अवलोकनम्_स्थितिः_Op==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      प्रति(ii=0; ii<nOp; ii++){
        res += aOp[ii].n_आवृत्तिः;
      }
      *(i64*)p_बहिर्गमनम् = res;
      प्रत्यागम 0;
    }
    प्रत्यागम 1;
  }
  यदि( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = i_अवलोकनम्;
  }अन्यथा{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    प्रति(idx=0; idx<p->n_अवलोकनम्; idx++){
      p_अवलोकनम् = &p->a_अवलोकनम्[idx];
      यदि( p_अवलोकनम्->zName ){
        i_अवलोकनम्--;
        यदि( i_अवलोकनम्<0 ) भङ्ग;
      }
    }
  }
  यदि( idx>=p->n_अवलोकनम् ) प्रत्यागम 1;
  प्रतिज्ञा( p_अवलोकनम्==0 || p_अवलोकनम्==&p->a_अवलोकनम्[idx] );
  p_अवलोकनम् = &p->a_अवलोकनम्[idx];

  विकल्प( i_अवलोकनम्_स्थितिः_Op ){
    पक्ष SQLITE_SCANSTAT_NLOOP: {
      यदि( p_अवलोकनम्->addrLoop>0 ){
        *(sqlite3_int64*)p_बहिर्गमनम् = aOp[p_अवलोकनम्->addrLoop].nExec;
      }अन्यथा{
        *(sqlite3_int64*)p_बहिर्गमनम् = -1;
      }
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_NVISIT: {
      यदि( p_अवलोकनम्->addr_दर्शय>0 ){
        *(sqlite3_int64*)p_बहिर्गमनम् = aOp[p_अवलोकनम्->addr_दर्शय].nExec;
      }अन्यथा{
        *(sqlite3_int64*)p_बहिर्गमनम् = -1;
      }
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_EST: {
      द्विगुण r = 1.0;
      LogEst x = p_अवलोकनम्->nEst;
      यावत्( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(द्विगुण*)p_बहिर्गमनम् = r*sqlite3_Log_Est_गन्तव्यम्_Int(x);
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_NAME: {
      *(ध्रुव वर्ण**)p_बहिर्गमनम् = p_अवलोकनम्->zName;
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_EXPLAIN: {
      यदि( p_अवलोकनम्->addrExplain ){
        *(ध्रुव वर्ण**)p_बहिर्गमनम् = aOp[ p_अवलोकनम्->addrExplain ].p4.z;
      }अन्यथा{
        *(ध्रुव वर्ण**)p_बहिर्गमनम् = 0;
      }
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_SELECTID: {
      यदि( p_अवलोकनम्->addrExplain ){
        *(पूर्णाङ्क*)p_बहिर्गमनम् = aOp[ p_अवलोकनम्->addrExplain ].p1;
      }अन्यथा{
        *(पूर्णाङ्क*)p_बहिर्गमनम् = -1;
      }
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_PARENTID: {
      यदि( p_अवलोकनम्->addrExplain ){
        *(पूर्णाङ्क*)p_बहिर्गमनम् = aOp[ p_अवलोकनम्->addrExplain ].p2;
      }अन्यथा{
        *(पूर्णाङ्क*)p_बहिर्गमनम् = -1;
      }
      भङ्ग;
    }
    पक्ष SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      यदि( p_अवलोकनम्->aAddrRange[0]==0 ){
        res = -1;
      }अन्यथा{
        पूर्णाङ्क ii;
        प्रति(ii=0; ii<Array_आकार(p_अवलोकनम्->aAddrRange); ii+=2){
          पूर्णाङ्क iIns = p_अवलोकनम्->aAddrRange[ii];
          पूर्णाङ्क i_अन्तः = p_अवलोकनम्->aAddrRange[ii+1];
          यदि( iIns==0 ) भङ्ग;
          यदि( iIns>0 ){
            यावत्( iIns<=i_अन्तः ){
              res += aOp[iIns].n_आवृत्तिः;
              iIns++;
            }
          }अन्यथा{
            पूर्णाङ्क iOp;
            प्रति(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              यदि( pOp->p1!=i_अन्तः ) अनुवर्त;
              यदि( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                अनुवर्त;
              }
              res += aOp[iOp].n_आवृत्तिः;
            }
          }
        }
      }
      *(i64*)p_बहिर्गमनम् = res;
      भङ्ग;
    }
    स्वभाव: {
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
पूर्णाङ्क sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  पूर्णाङ्क i_अवलोकनम्,                      /* Index of loop to report on */
  पूर्णाङ्क i_अवलोकनम्_स्थितिः_Op,              /* Which metric to return */
  शून्य *p_बहिर्गमनम्                      /* OUT: Write the answer here */
){
  प्रत्यागम sqlite3_stmt_scanstatus_v2(pStmt, i_अवलोकनम्, i_अवलोकनम्_स्थितिः_Op, 0, p_बहिर्गमनम्);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
शून्य sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  पूर्णाङ्क ii;
  प्रति(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->n_आवृत्तिः = 0;
  }
}
#यदि_अन्तः /* SQLITE_ENABLE_STMT_SCANSTATUS */
