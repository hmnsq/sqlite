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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"
#అన్తర్భావ "opcodes.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
పూర్ణాంకం sqlite3_expired(sqlite3_stmt *pStmt){
  పూర్ణాంకం iRet = 1;
  ఒకవేళ( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  తిరిగి iRet;
}
#అయితే_ముగింపు

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
స్థిర పూర్ణాంకం vdbeSafety(Vdbe *p){
  ఒకవేళ( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి 0;
  }
}
స్థిర పూర్ణాంకం vdbeSafetyNotNull(Vdbe *p){
  ఒకవేళ( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి vdbeSafety(p);
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
స్థిర SQLITE_NOINLINE శూన్య invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  ప్రతిజ్ఞా( p->ప్రారంభం_సమయం>0 );
  ప్రతిజ్ఞా( db->init.busy==0 );
  ప్రతిజ్ఞా( p->zSql!=0 );
  sqlite3_Os_Current_సమయం_Int64(db->pVfs, &iNow);
  iElapse = (iNow - p->ప్రారంభం_సమయం)*1000000;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  ఒకవేళ( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#అయితే_ముగింపు
  ఒకవేళ( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (శూన్య*)&iElapse);
  }
  p->ప్రారంభం_సమయం = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   ఒకవేళ( ((P)->ప్రారంభం_సమయం)>0 ){ invokeProfileCallback(DB,P); }
#లేకపోతే
# define checkProfileCallback(DB,P)  /*no-op*/
#అయితే_ముగింపు

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
పూర్ణాంకం sqlite3_finalize(sqlite3_stmt *pStmt){
  పూర్ణాంకం rc;
  ఒకవేళ( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }లేకపోతే{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    ఒకవేళ( vdbeSafety(v) ) తిరిగి SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    ప్రతిజ్ఞా( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  తిరిగి rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
పూర్ణాంకం sqlite3_reset(sqlite3_stmt *pStmt){
  పూర్ణాంకం rc;
  ఒకవేళ( pStmt==0 ){
    rc = SQLITE_OK;
  }లేకపోతే{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    ప్రతిజ్ఞా( (rc & (db->err_ముసుగు))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  తిరిగి rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
పూర్ణాంకం sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 ముసుగు_Small_Int = 0;  /* Bitmask of bindings with value 0 or 1 */
#ఒకవేళ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pStmt==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
#ఒకవేళ SQLITE_THREADSAFE
  mutex = p->db->mutex;
#అయితే_ముగింపు

  sqlite3_mutex_enter(mutex);
  కొరకు(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    ఒకవేళ( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      ముసుగు_Small_Int |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  ప్రతిజ్ఞా( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ఒకవేళ( (p->expmask&~p->smimask)!=0 || (p->smimask&ముసుగు_Small_Int)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  తిరిగి rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
స్థిరరాశి శూన్య *sqlite3_విలువ_blob(sqlite3_విలువ *pVal){
  Mem *p = (Mem*)pVal;
  ఒకవేళ( p->flags & (MEM_Blob|MEM_Str) ){
    ఒకవేళ( ExpandBlob(p)!=SQLITE_OK ){
      ప్రతిజ్ఞా( p->flags==MEM_Null && p->z==0 );
      తిరిగి 0;
    }
    p->flags |= MEM_Blob;
    తిరిగి p->n ? p->z : 0;
  }లేకపోతే{
    తిరిగి sqlite3_విలువ_పాఠ్యం(pVal);
  }
}
పూర్ణాంకం sqlite3_విలువ_బైట్లు(sqlite3_విలువ *pVal){
  తిరిగి sqlite3_విలువ_బైట్లు_2(pVal, SQLITE_UTF8);
}
పూర్ణాంకం sqlite3_విలువ_bytes16(sqlite3_విలువ *pVal){
  తిరిగి sqlite3_విలువ_బైట్లు_2(pVal, SQLITE_UTF16NATIVE);
}
ద్విగుణ sqlite3_విలువ_double(sqlite3_విలువ *pVal){
  తిరిగి sqlite3_Vdbe_Real_విలువ((Mem*)pVal);
}
పూర్ణాంకం sqlite3_విలువ_int(sqlite3_విలువ *pVal){
  తిరిగి (పూర్ణాంకం)sqlite3_Vdbe_Int_విలువ((Mem*)pVal);
}
sqlite_int64 sqlite3_విలువ_int64(sqlite3_విలువ *pVal){
  తిరిగి sqlite3_Vdbe_Int_విలువ((Mem*)pVal);
}
గుర్తులేని పూర్ణాంకం sqlite3_విలువ_subtype(sqlite3_విలువ *pVal){
  Mem *pMem = (Mem*)pVal;
  తిరిగి ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
శూన్య *sqlite3_విలువ_సూచిక(sqlite3_విలువ *pVal, స్థిరరాశి అక్షరం *zPType){
  Mem *p = (Mem*)pVal;
  ఒకవేళ( (p->flags&(MEM_Type_ముసుగు|MEM_పదం|MEM_Subtype)) ==
                 (MEM_Null|MEM_పదం|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    తిరిగి (శూన్య*)p->z;
  }లేకపోతే{
    తిరిగి 0;
  }
}
స్థిరరాశి గుర్తులేని అక్షరం *sqlite3_విలువ_పాఠ్యం(sqlite3_విలువ *pVal){
  తిరిగి (స్థిరరాశి గుర్తులేని అక్షరం *)sqlite3_విలువ_పాఠ్యం_2(pVal, SQLITE_UTF8);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_విలువ_text16(sqlite3_విలువ* pVal){
  తిరిగి sqlite3_విలువ_పాఠ్యం_2(pVal, SQLITE_UTF16NATIVE);
}
స్థిరరాశి శూన్య *sqlite3_విలువ_text16be(sqlite3_విలువ *pVal){
  తిరిగి sqlite3_విలువ_పాఠ్యం_2(pVal, SQLITE_UTF16BE);
}
స్థిరరాశి శూన్య *sqlite3_విలువ_text16le(sqlite3_విలువ *pVal){
  తిరిగి sqlite3_విలువ_పాఠ్యం_2(pVal, SQLITE_UTF16LE);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
పూర్ణాంకం sqlite3_విలువ_type(sqlite3_విలువ* pVal){
  స్థిర స్థిరరాశి u8 aType[] = {
     SQLITE_BLOB,     /* 0x00 (not possible) */
     SQLITE_NULL,     /* 0x01 NULL */
     SQLITE_పాఠ్యం,     /* 0x02 TEXT */
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
     SQLITE_పాఠ్యం,     /* 0x12 (not possible) */
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
     SQLITE_పాఠ్యం,     /* 0x32 (not possible) */
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
#నిర్వచితమైతే SQLITE_DEBUG
  {
    పూర్ణాంకం eType = SQLITE_BLOB;
    ఒకవేళ( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }లేకపోతే ఒకవేళ( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }లేకపోతే ఒకవేళ( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }లేకపోతే ఒకవేళ( pVal->flags & MEM_Str ){
      eType = SQLITE_పాఠ్యం;
    }
    ప్రతిజ్ఞా( eType == aType[pVal->flags&MEM_Aff_ముసుగు] );
  }
#అయితే_ముగింపు
  తిరిగి aType[pVal->flags&MEM_Aff_ముసుగు];
}
పూర్ణాంకం sqlite3_విలువ_encoding(sqlite3_విలువ *pVal){
  తిరిగి pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
పూర్ణాంకం sqlite3_విలువ_nochange(sqlite3_విలువ *pVal){
  తిరిగి (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
పూర్ణాంకం sqlite3_విలువ_frombind(sqlite3_విలువ *pVal){
  తిరిగి (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_విలువ *sqlite3_విలువ_dup(స్థిరరాశి sqlite3_విలువ *pOrig){
  sqlite3_విలువ *pNew;
  ఒకవేళ( pOrig==0 ) తిరిగి 0;
  pNew = sqlite3_malloc( పరిమాణం(*pNew) );
  ఒకవేళ( pNew==0 ) తిరిగి 0;
  memset(pNew, 0, పరిమాణం(*pNew));
  జ్ఞాపకం_నకలు(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  ఒకవేళ( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    ఒకవేళ( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_విలువ_Free(pNew);
      pNew = 0;
    }
  }లేకపోతే ఒకవేళ( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_పదం|MEM_Subtype);
  }
  తిరిగి pNew;
}

/* Destroy an sqlite3_value object previously obtained from
** sqlite3_value_dup().
*/
శూన్య sqlite3_విలువ_free(sqlite3_విలువ *pOld){
  sqlite3_విలువ_Free(pOld);
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
స్థిర శూన్య సమితి_ఫలితం_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  స్థిరరాశి అక్షరం *z,          /* String pointer */
  పూర్ణాంకం n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  శూన్య (*xDel)(శూన్య*)     /* Destructor function */
){
  Mem *p_బహిర్గతం = pCtx->p_బహిర్గతం;
  పూర్ణాంకం rc;
  ఒకవేళ( enc==SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(p_బహిర్గతం, z, n, xDel);
  }లేకపోతే ఒకవేళ( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    ప్రతిజ్ఞా( z==0
         || n<0 
         || n>p_బహిర్గతం->db->a_హద్దు[SQLITE_హద్దు_నిడివి]
         || z[n]==0
    );
    rc = sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(p_బహిర్గతం, z, n, xDel);
    p_బహిర్గతం->flags |= MEM_పదం;
  }లేకపోతే{
    rc = sqlite3_Vdbe_Mem_సమితి_Str(p_బహిర్గతం, z, n, enc, xDel);
  }
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_TOOBIG ){
      sqlite3_ఫలితం_error_toobig(pCtx);
    }లేకపోతే{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      ప్రతిజ్ఞా( rc==SQLITE_NOMEM );
      sqlite3_ఫలితం_error_nomem(pCtx);
    }
    తిరిగి;
  }
  sqlite3VdbeChangeEncoding(p_బహిర్గతం, pCtx->enc);
  ఒకవేళ( sqlite3VdbeMemTooBig(p_బహిర్గతం) ){
    sqlite3_ఫలితం_error_toobig(pCtx);
  }
}
స్థిర పూర్ణాంకం invoke_విలువ_Destructor(
  స్థిరరాశి శూన్య *p,             /* Value to destroy */
  శూన్య (*xDel)(శూన్య*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  ప్రతిజ్ఞా( xDel!=SQLITE_DYNAMIC );
  ఒకవేళ( xDel==0 ){
    /* noop */
  }లేకపోతే ఒకవేళ( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }లేకపోతే{
    xDel((శూన్య*)p);
  }
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx!=0 ){
    sqlite3_ఫలితం_error_toobig(pCtx);
  }
#లేకపోతే
  ప్రతిజ్ఞా( pCtx!=0 );
  sqlite3_ఫలితం_error_toobig(pCtx);
#అయితే_ముగింపు
  తిరిగి SQLITE_TOOBIG;
}
శూన్య sqlite3_ఫలితం_blob(
  sqlite3_context *pCtx,
  స్థిరరాశి శూన్య *z,
  పూర్ణాంకం n,
  శూన్య (*xDel)(శూన్య *)
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 || n<0 ){
    invoke_విలువ_Destructor(z, xDel, pCtx);
    తిరిగి;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( n>=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  సమితి_ఫలితం_Str_Or_Error(pCtx, z, n, 0, xDel);
}
శూన్య sqlite3_ఫలితం_blob64(
  sqlite3_context *pCtx,
  స్థిరరాశి శూన్య *z,
  sqlite3_uint64 n,
  శూన్య (*xDel)(శూన్య *)
){
  ప్రతిజ్ఞా( xDel!=SQLITE_DYNAMIC );
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ){
    invoke_విలువ_Destructor(z, xDel, 0);
    తిరిగి;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  ఒకవేళ( n>0x7fffffff ){
    (శూన్య)invoke_విలువ_Destructor(z, xDel, pCtx);
  }లేకపోతే{
    సమితి_ఫలితం_Str_Or_Error(pCtx, z, (పూర్ణాంకం)n, 0, xDel);
  }
}
శూన్య sqlite3_ఫలితం_double(sqlite3_context *pCtx, ద్విగుణ rVal){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  sqlite3_Vdbe_Mem_సమితి_Double(pCtx->p_బహిర్గతం, rVal);
}
శూన్య sqlite3_ఫలితం_error(sqlite3_context *pCtx, స్థిరరాశి అక్షరం *z, పూర్ణాంకం n){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_సమితి_Str(pCtx->p_బహిర్గతం, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
శూన్య sqlite3_ఫలితం_error16(sqlite3_context *pCtx, స్థిరరాశి శూన్య *z, పూర్ణాంకం n){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_సమితి_Str(pCtx->p_బహిర్గతం, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#అయితే_ముగింపు
శూన్య sqlite3_ఫలితం_int(sqlite3_context *pCtx, పూర్ణాంకం iVal){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  sqlite3_Vdbe_Mem_సమితి_Int64(pCtx->p_బహిర్గతం, (i64)iVal);
}
శూన్య sqlite3_ఫలితం_int64(sqlite3_context *pCtx, i64 iVal){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  sqlite3_Vdbe_Mem_సమితి_Int64(pCtx->p_బహిర్గతం, iVal);
}
శూన్య sqlite3_ఫలితం_null(sqlite3_context *pCtx){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  sqlite3_Vdbe_Mem_సమితి_Null(pCtx->p_బహిర్గతం);
}
శూన్య sqlite3_ఫలితం_సూచిక(
  sqlite3_context *pCtx,
  శూన్య *pPtr,
  స్థిరరాశి అక్షరం *zPType,
  శూన్య (*xDestructor)(శూన్య*)
){
  Mem *p_బహిర్గతం;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ){
    invoke_విలువ_Destructor(pPtr, xDestructor, 0);
    తిరిగి;
  }
#అయితే_ముగింపు
  p_బహిర్గతం = pCtx->p_బహిర్గతం;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_బహిర్గతం->db->mutex) );
  sqlite3VdbeMemRelease(p_బహిర్గతం);
  p_బహిర్గతం->flags = MEM_Null;
  sqlite3_Vdbe_Mem_సమితి_సూచిక(p_బహిర్గతం, pPtr, zPType, xDestructor);
}
శూన్య sqlite3_ఫలితం_subtype(sqlite3_context *pCtx, గుర్తులేని పూర్ణాంకం eSubtype){
  Mem *p_బహిర్గతం;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  ఒకవేళ( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_ఫలితం_SUBTYPE)==0
  ){
    అక్షరం zErr[200];
    sqlite3_snprintf(పరిమాణం(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_ఫలితం_error(pCtx, zErr, -1);
    తిరిగి;
  }
#అయితే_ముగింపు /* SQLITE_STRICT_SUBTYPE */
  p_బహిర్గతం = pCtx->p_బహిర్గతం;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_బహిర్గతం->db->mutex) );
  p_బహిర్గతం->eSubtype = eSubtype & 0xff;
  p_బహిర్గతం->flags |= MEM_Subtype;
}
శూన్య sqlite3_ఫలితం_పాఠ్యం(
  sqlite3_context *pCtx,
  స్థిరరాశి అక్షరం *z,
  పూర్ణాంకం n,
  శూన్య (*xDel)(శూన్య *)
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ){
    invoke_విలువ_Destructor(z, xDel, 0);
    తిరిగి;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  సమితి_ఫలితం_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
శూన్య sqlite3_ఫలితం_text64(
  sqlite3_context *pCtx,
  స్థిరరాశి అక్షరం *z,
  sqlite3_uint64 n,
  శూన్య (*xDel)(శూన్య *),
  గుర్తులేని అక్షరం enc
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ){
    invoke_విలువ_Destructor(z, xDel, 0);
    తిరిగి;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  ప్రతిజ్ఞా( xDel!=SQLITE_DYNAMIC );
  ఒకవేళ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ఒకవేళ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  ఒకవేళ( n>0x7fffffff ){
    (శూన్య)invoke_విలువ_Destructor(z, xDel, pCtx);
  }లేకపోతే{
    సమితి_ఫలితం_Str_Or_Error(pCtx, z, (పూర్ణాంకం)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->p_బహిర్గతం);
  }
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
శూన్య sqlite3_ఫలితం_text16(
  sqlite3_context *pCtx,
  స్థిరరాశి శూన్య *z,
  పూర్ణాంకం n,
  శూన్య (*xDel)(శూన్య *)
){
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  సమితి_ఫలితం_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
శూన్య sqlite3_ఫలితం_text16be(
  sqlite3_context *pCtx,
  స్థిరరాశి శూన్య *z,
  పూర్ణాంకం n,
  శూన్య (*xDel)(శూన్య *)
){
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  సమితి_ఫలితం_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
శూన్య sqlite3_ఫలితం_text16le(
  sqlite3_context *pCtx,
  స్థిరరాశి శూన్య *z,
  పూర్ణాంకం n,
  శూన్య (*xDel)(శూన్య *)
){
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  సమితి_ఫలితం_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
శూన్య sqlite3_ఫలితం_విలువ(sqlite3_context *pCtx, sqlite3_విలువ *p_విలువ){
  Mem *p_బహిర్గతం;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
  ఒకవేళ( p_విలువ==0 ){
    sqlite3_ఫలితం_null(pCtx);
    తిరిగి;
  }
#అయితే_ముగింపు
  p_బహిర్గతం = pCtx->p_బహిర్గతం;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  sqlite3VdbeMemCopy(p_బహిర్గతం, p_విలువ);
  sqlite3VdbeChangeEncoding(p_బహిర్గతం, pCtx->enc);
  ఒకవేళ( sqlite3VdbeMemTooBig(p_బహిర్గతం) ){
    sqlite3_ఫలితం_error_toobig(pCtx);
  }
}
శూన్య sqlite3_ఫలితం_zeroblob(sqlite3_context *pCtx, పూర్ణాంకం n){
  sqlite3_ఫలితం_zeroblob64(pCtx, n>0 ? n : 0);
}
పూర్ణాంకం sqlite3_ఫలితం_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *p_బహిర్గతం;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  p_బహిర్గతం = pCtx->p_బహిర్గతం;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_బహిర్గతం->db->mutex) );
  ఒకవేళ( n>(u64)p_బహిర్గతం->db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    sqlite3_ఫలితం_error_toobig(pCtx);
    తిరిగి SQLITE_TOOBIG;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
  sqlite3_Vdbe_Mem_సమితి_Zero_Blob(pCtx->p_బహిర్గతం, (పూర్ణాంకం)n);
  తిరిగి SQLITE_OK;
#లేకపోతే
  తిరిగి sqlite3_Vdbe_Mem_సమితి_Zero_Blob(pCtx->p_బహిర్గతం, (పూర్ణాంకం)n);
#అయితే_ముగింపు
}
శూన్య sqlite3_ఫలితం_error_సంకేతం(sqlite3_context *pCtx, పూర్ణాంకం err_సంకేతం){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  pCtx->isError = err_సంకేతం ? err_సంకేతం : -1;
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pCtx->pVdbe ) pCtx->pVdbe->rcApp = err_సంకేతం;
#అయితే_ముగింపు
  ఒకవేళ( pCtx->p_బహిర్గతం->flags & MEM_Null ){
    సమితి_ఫలితం_Str_Or_Error(pCtx, sqlite3ErrStr(err_సంకేతం), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
శూన్య sqlite3_ఫలితం_error_toobig(sqlite3_context *pCtx){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3_Vdbe_Mem_సమితి_Str(pCtx->p_బహిర్గతం, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
శూన్య sqlite3_ఫలితం_error_nomem(sqlite3_context *pCtx){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  sqlite3_Vdbe_Mem_సమితి_Null(pCtx->p_బహిర్గతం);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->p_బహిర్గతం->db);
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
శూన్య sqlite3_ఫలితం_str(sqlite3_context *pCtx, sqlite3_str *pStr, పూర్ణాంకం eOwn){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
  ఒకవేళ( pStr==0 ) తిరిగి;
#అయితే_ముగింపు
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  ఒకవేళ( pStr->accError==0 ){
    ఒకవేళ( pStr->nChar==0 ){
      సమితి_ఫలితం_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }లేకపోతే{
      స్థిరరాశి అక్షరం *z_పాఠ్యం = sqlite3_str_విలువ(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      ప్రతిజ్ఞా( isMalloced(pStr) );
      ఒకవేళ( eOwn==SQLITE_COPY ){
        సమితి_ఫలితం_Str_Or_Error(pCtx, z_పాఠ్యం, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }లేకపోతే{
        సమితి_ఫలితం_Str_Or_Error(pCtx, z_పాఠ్యం, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }లేకపోతే ఒకవేళ( pStr->accError==SQLITE_NOMEM ){
    sqlite3_ఫలితం_error_nomem(pCtx);
  }లేకపోతే{
    ప్రతిజ్ఞా( pStr->accError==SQLITE_TOOBIG );
    sqlite3_ఫలితం_error_toobig(pCtx);
  }
  ఒకవేళ( eOwn==SQLITE_ముగింపు ){
    sqlite3_str_free(pStr);
  }
}

#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
శూన్య sqlite3_ఫలితం_Int_Real(sqlite3_context *pCtx){
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
  ఒకవేళ( pCtx->p_బహిర్గతం->flags & MEM_Int ){
    pCtx->p_బహిర్గతం->flags &= ~MEM_Int;
    pCtx->p_బహిర్గతం->flags |= MEM_IntReal;
  }
}
#అయితే_ముగింపు


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
స్థిర పూర్ణాంకం doWalCallbacks(sqlite3 *db){
  పూర్ణాంకం rc = SQLITE_OK;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  పూర్ణాంకం i;
  కొరకు(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ఒకవేళ( pBt ){
      పూర్ణాంకం n_నమోదు;
      sqlite3BtreeEnter(pBt);
      n_నమోదు = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      ఒకవేళ( n_నమోదు>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, n_నమోదు);
      }
    }
  }
#లేకపోతే
  వాడని_PARAMETER(db);
#అయితే_ముగింపు
  తిరిగి rc;
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
స్థిర పూర్ణాంకం sqlite3_అడుగు_2(Vdbe *p){
  sqlite3 *db;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా(p);
  db = p->db;
  ఒకవేళ( p->eVdbeState!=VDBE_నిరంతరం_STATE ){
    restart_అడుగు:
    ఒకవేళ( p->eVdbeState==VDBE_READY_STATE ){
      ఒకవేళ( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        ఒకవేళ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        వెళ్ళు చివర_యొక్క_అడుగు;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      ఒకవేళ( db->n_Vdbe_చురుకైన==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      ప్రతిజ్ఞా( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
      ఒకవేళ( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3_Os_Current_సమయం_Int64(db->pVfs, &p->ప్రారంభం_సమయం);
      }లేకపోతే{
        ప్రతిజ్ఞా( p->ప్రారంభం_సమయం==0 );
      }
#అయితే_ముగింపు

      db->n_Vdbe_చురుకైన++;
      ఒకవేళ( p->readOnly==0 ) db->nVdbeWrite++;
      ఒకవేళ( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_నిరంతరం_STATE;
    }లేకపోతే

    ఒకవేళ( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
#నిర్వచితమైతే SQLITE_OMIT_AUTORESET
      ఒకవేళ( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }లేకపోతే{
        తిరిగి SQLITE_MISUSE_BKPT;
      }
#లేకపోతే
      sqlite3_reset((sqlite3_stmt*)p);
#అయితే_ముగింపు
      ప్రతిజ్ఞా( p->eVdbeState==VDBE_READY_STATE );
      వెళ్ళు restart_అడుగు;
    }
  }

#నిర్వచితమైతే SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
  ఒకవేళ( p->explain ){
    rc = sqlite3VdbeList(p);
  }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
    ప్రతిజ్ఞా( p->rc==SQLITE_OK );
    ప్రతిజ్ఞా( db->mallocFailed==0 );
    db->err_సంకేతం = SQLITE_అడ్డువరుస;
    తిరిగి SQLITE_అడ్డువరుస;
  }లేకపోతే{
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#అయితే_ముగింపు
    p->p_ఫలితం_అడ్డువరుస = 0;
    ఒకవేళ( rc==SQLITE_పూర్తి && db->autoCommit ){
      ప్రతిజ్ఞా( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      ఒకవేళ( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }లేకపోతే ఒకవేళ( rc!=SQLITE_పూర్తి && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->err_సంకేతం = rc;
  ఒకవేళ( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    ఒకవేళ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
చివర_యొక్క_అడుగు:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  ప్రతిజ్ఞా( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_అడ్డువరుస  || rc==SQLITE_పూర్తి   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  తిరిగి (rc&db->err_ముసుగు);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
పూర్ణాంకం sqlite3_అడుగు(sqlite3_stmt *pStmt){
  పూర్ణాంకం rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  పూర్ణాంకం cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  ఒకవేళ( vdbeSafetyNotNull(v) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  అయితే( (rc = sqlite3_అడుగు_2(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    పూర్ణాంకం savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    ఒకవేళ( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      స్థిరరాశి అక్షరం *zErr = (స్థిరరాశి అక్షరం *)sqlite3_విలువ_పాఠ్యం(db->pErr);
      sqlite3DbFree(db, v->z_Err_సందేశం);
      ఒకవేళ( !db->mallocFailed ){
        v->z_Err_సందేశం = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } లేకపోతే {
        v->z_Err_సందేశం = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      ఆపు;
    }
    sqlite3_reset(pStmt);
    ఒకవేళ( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    ప్రతిజ్ఞా( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
శూన్య *sqlite3_user_దత్తాంశం(sqlite3_context *p){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ) తిరిగి 0;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p && p->pFunc );
  తిరిగి p->pFunc->p_User_దత్తాంశం;
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
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ) తిరిగి 0;
#లేకపోతే
  ప్రతిజ్ఞా( p && p->p_బహిర్గతం );
#అయితే_ముగింపు
  తిరిగి p->p_బహిర్గతం->db;
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
పూర్ణాంకం sqlite3_vtab_nochange(sqlite3_context *p){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ) తిరిగి 0;
#లేకపోతే
  ప్రతిజ్ఞా( p );
#అయితే_ముగింపు
  తిరిగి sqlite3_విలువ_nochange(p->p_బహిర్గతం);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
శూన్య sqlite3_Vdbe_విలువ_List_Free(శూన్య *p_గమ్యం_Delete){
  sqlite3_free(p_గమ్యం_Delete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
స్థిర పూర్ణాంకం విలువ_From_విలువ_List(
  sqlite3_విలువ *pVal,        /* Pointer to the ValueList object */
  sqlite3_విలువ **pp_బహిర్గతం,      /* Store the next value from the list here */
  పూర్ణాంకం b_తదుపరి                   /* 1 for _next(). 0 for _first() */
){
  పూర్ణాంకం rc;
  విలువ_List *pRhs;

  *pp_బహిర్గతం = 0;
  ఒకవేళ( pVal==0 ) తిరిగి SQLITE_MISUSE_BKPT;
  ఒకవేళ( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_విలువ_List_Free ){
    తిరిగి SQLITE_ERROR;
  }లేకపోతే{
    ప్రతిజ్ఞా( (pVal->flags&(MEM_Type_ముసుగు|MEM_పదం|MEM_Subtype)) ==
                 (MEM_Null|MEM_పదం|MEM_Subtype) );
    ప్రతిజ్ఞా( pVal->eSubtype=='p' );
    ప్రతిజ్ఞా( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (విలువ_List*)pVal->z;
  }
  ఒకవేళ( b_తదుపరి ){
    rc = sqlite3_Btree_తదుపరి(pRhs->pCsr, 0);
  }లేకపోతే{
    పూర్ణాంకం dummy = 0;
    rc = sqlite3_Btree_మొదటి(pRhs->pCsr, &dummy);
    ప్రతిజ్ఞా( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    ఒకవేళ( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_పూర్తి;
  }
  ఒకవేళ( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, పరిమాణం(sMem));
    sz = sqlite3_Btree_Payload_కొలత(pRhs->pCsr);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pRhs->pCsr,sz,&sMem);
    ఒకవేళ( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_విలువ *p_బహిర్గతం = pRhs->p_బహిర్గతం;
      పూర్ణాంకం iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, p_బహిర్గతం);
      p_బహిర్గతం->enc = ENC(p_బహిర్గతం->db);
      ఒకవేళ( (p_బహిర్గతం->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(p_బహిర్గతం) ){
        rc = SQLITE_NOMEM;
      }లేకపోతే{
        *pp_బహిర్గతం = p_బహిర్గతం;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  తిరిగి rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
పూర్ణాంకం sqlite3_vtab_లోపల_మొదటి(sqlite3_విలువ *pVal, sqlite3_విలువ **pp_బహిర్గతం){
  తిరిగి విలువ_From_విలువ_List(pVal, pp_బహిర్గతం, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
పూర్ణాంకం sqlite3_vtab_లోపల_తదుపరి(sqlite3_విలువ *pVal, sqlite3_విలువ **pp_బహిర్గతం){
  తిరిగి విలువ_From_విలువ_List(pVal, pp_బహిర్గతం, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3_Stmt_Current_సమయం(sqlite3_context *p){
  పూర్ణాంకం rc;
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_STAT4
  sqlite3_int64 *pi_సమయం = &p->pVdbe->i_Current_సమయం;
  ప్రతిజ్ఞా( p->pVdbe!=0 );
#లేకపోతే
  sqlite3_int64 i_సమయం = 0;
  sqlite3_int64 *pi_సమయం = p->pVdbe!=0 ? &p->pVdbe->i_Current_సమయం : &i_సమయం;
#అయితే_ముగింపు
  ఒకవేళ( *pi_సమయం==0 ){
    rc = sqlite3_Os_Current_సమయం_Int64(p->p_బహిర్గతం->db->pVfs, pi_సమయం);
    ఒకవేళ( rc ) *pi_సమయం = 0;
  }
  తిరిగి *pi_సమయం;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
స్థిర SQLITE_NOINLINE శూన్య *createAggContext(sqlite3_context *p, పూర్ణాంకం nByte){
  Mem *pMem = p->pMem;
  ప్రతిజ్ఞా( (pMem->flags & MEM_Agg)==0 );
  ఒకవేళ( nByte<=0 ){
    sqlite3_Vdbe_Mem_సమితి_Null(pMem);
    pMem->z = 0;
  }లేకపోతే{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    ఒకవేళ( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  తిరిగి (శూన్య*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
శూన్య *sqlite3_aggregate_context(sqlite3_context *p, పూర్ణాంకం nByte){
  ప్రతిజ్ఞా( p && p->pFunc && p->pFunc->xFinalize );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->p_బహిర్గతం->db->mutex) );
  testcase( nByte<0 );
  ఒకవేళ( (p->pMem->flags & MEM_Agg)==0 ){
    తిరిగి createAggContext(p, nByte);
  }లేకపోతే{
    తిరిగి (శూన్య*)p->pMem->z;
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
శూన్య *sqlite3_get_auxdata(sqlite3_context *pCtx, పూర్ణాంకం iArg){
  Aux_దత్తాంశం *p_Aux_దత్తాంశం;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి 0;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
#ఒకవేళ SQLITE_ENABLE_STAT4
  ఒకవేళ( pCtx->pVdbe==0 ) తిరిగి 0;
#లేకపోతే
  ప్రతిజ్ఞా( pCtx->pVdbe!=0 );
#అయితే_ముగింపు
  కొరకు(p_Aux_దత్తాంశం=pCtx->pVdbe->p_Aux_దత్తాంశం; p_Aux_దత్తాంశం; p_Aux_దత్తాంశం=p_Aux_దత్తాంశం->p_తదుపరి_Aux){
    ఒకవేళ(  p_Aux_దత్తాంశం->iAuxArg==iArg && (p_Aux_దత్తాంశం->iAuxOp==pCtx->iOp || iArg<0) ){
      తిరిగి p_Aux_దత్తాంశం->pAux;
    }
  }
  తిరిగి 0;
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
శూన్య sqlite3_సమితి_auxdata(
  sqlite3_context *pCtx,
  పూర్ణాంకం iArg,
  శూన్య *pAux,
  శూన్య (*xDelete)(శూన్య*)
){
  Aux_దత్తాంశం *p_Aux_దత్తాంశం;
  Vdbe *pVdbe;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCtx==0 ) తిరిగి;
#అయితే_ముగింపు
  pVdbe= pCtx->pVdbe;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCtx->p_బహిర్గతం->db->mutex) );
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( pVdbe==0 ) వెళ్ళు failed;
#లేకపోతే
  ప్రతిజ్ఞా( pVdbe!=0 );
#అయితే_ముగింపు

  కొరకు(p_Aux_దత్తాంశం=pVdbe->p_Aux_దత్తాంశం; p_Aux_దత్తాంశం; p_Aux_దత్తాంశం=p_Aux_దత్తాంశం->p_తదుపరి_Aux){
    ఒకవేళ( p_Aux_దత్తాంశం->iAuxArg==iArg && (p_Aux_దత్తాంశం->iAuxOp==pCtx->iOp || iArg<0) ){
      ఆపు;
    }
  }
  ఒకవేళ( p_Aux_దత్తాంశం==0 ){
    p_Aux_దత్తాంశం = sqlite3DbMallocZero(pVdbe->db, పరిమాణం(Aux_దత్తాంశం));
    ఒకవేళ( !p_Aux_దత్తాంశం ) వెళ్ళు failed;
    p_Aux_దత్తాంశం->iAuxOp = pCtx->iOp;
    p_Aux_దత్తాంశం->iAuxArg = iArg;
    p_Aux_దత్తాంశం->p_తదుపరి_Aux = pVdbe->p_Aux_దత్తాంశం;
    pVdbe->p_Aux_దత్తాంశం = p_Aux_దత్తాంశం;
    ఒకవేళ( pCtx->isError==0 ) pCtx->isError = -1;
  }లేకపోతే ఒకవేళ( p_Aux_దత్తాంశం->xDeleteAux ){
    p_Aux_దత్తాంశం->xDeleteAux(p_Aux_దత్తాంశం->pAux);
  }

  p_Aux_దత్తాంశం->pAux = pAux;
  p_Aux_దత్తాంశం->xDeleteAux = xDelete;
  తిరిగి;

failed:
  ఒకవేళ( xDelete ){
    xDelete(pAux);
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
పూర్ణాంకం sqlite3_aggregate_లెక్క(sqlite3_context *p){
  ప్రతిజ్ఞా( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  తిరిగి p->pMem->n;
}
#అయితే_ముగింపు

/*
** Return the number of columns in the result set for the statement pStmt.
*/
పూర్ణాంకం sqlite3_స్తంభం_లెక్క(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ఒకవేళ( pVm==0 ) తిరిగి 0;
  తిరిగి pVm->n_Res_స్తంభం;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
పూర్ణాంకం sqlite3_దత్తాంశం_లెక్క(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ఒకవేళ( pVm==0 || pVm->p_ఫలితం_అడ్డువరుస==0 ) తిరిగి 0;
  తిరిగి pVm->n_Res_స్తంభం;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
స్థిర స్థిరరాశి Mem *స్తంభం_Null_విలువ(శూన్య){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  స్థిర స్థిరరాశి Mem nullMem
#ఒకవేళ defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#అయితే_ముగింపు
    = {
        /* .u          = */ {0},
        /* .z          = */ (అక్షరం*)0,
        /* .n          = */ (పూర్ణాంకం)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (పూర్ణాంకం)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (అక్షరం*)0,
        /* .xDel       = */ (శూన్య(*)(శూన్య*))0,
#నిర్వచితమైతే SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#అయితే_ముగింపు
      };
  తిరిగి &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
స్థిర Mem *స్తంభం_Mem(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  Vdbe *pVm;
  Mem *p_బహిర్గతం;

  pVm = (Vdbe *)pStmt;
  ఒకవేళ( pVm==0 ) తిరిగి (Mem*)స్తంభం_Null_విలువ();
  ప్రతిజ్ఞా( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  ఒకవేళ( pVm->p_ఫలితం_అడ్డువరుస!=0 && i<pVm->n_Res_స్తంభం && i>=0 ){
    p_బహిర్గతం = &pVm->p_ఫలితం_అడ్డువరుస[i];
  }లేకపోతే{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    p_బహిర్గతం = (Mem*)స్తంభం_Null_విలువ();
  }
  తిరిగి p_బహిర్గతం;
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
స్థిర శూన్య స్తంభం_Malloc_Failure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( p->db!=0 );
    ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
స్థిరరాశి శూన్య *sqlite3_స్తంభం_blob(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  స్థిరరాశి శూన్య *val;
  val = sqlite3_విలువ_blob( స్తంభం_Mem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
పూర్ణాంకం sqlite3_స్తంభం_బైట్లు(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  పూర్ణాంకం val = sqlite3_విలువ_బైట్లు( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
పూర్ణాంకం sqlite3_స్తంభం_bytes16(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  పూర్ణాంకం val = sqlite3_విలువ_bytes16( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
ద్విగుణ sqlite3_స్తంభం_double(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  ద్విగుణ val = sqlite3_విలువ_double( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
పూర్ణాంకం sqlite3_స్తంభం_int(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  పూర్ణాంకం val = sqlite3_విలువ_int( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
sqlite_int64 sqlite3_స్తంభం_int64(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  sqlite_int64 val = sqlite3_విలువ_int64( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
స్థిరరాశి గుర్తులేని అక్షరం *sqlite3_స్తంభం_పాఠ్యం(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  స్థిరరాశి గుర్తులేని అక్షరం *val = sqlite3_విలువ_పాఠ్యం( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
sqlite3_విలువ *sqlite3_స్తంభం_విలువ(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  Mem *p_బహిర్గతం = స్తంభం_Mem(pStmt, i);
  ఒకవేళ( p_బహిర్గతం->flags&MEM_Static ){
    p_బహిర్గతం->flags &= ~MEM_Static;
    p_బహిర్గతం->flags |= MEM_Ephem;
  }
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి (sqlite3_విలువ *)p_బహిర్గతం;
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_స్తంభం_text16(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  స్థిరరాశి శూన్య *val = sqlite3_విలువ_text16( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి val;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
పూర్ణాంకం sqlite3_స్తంభం_type(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  పూర్ణాంకం iType = sqlite3_విలువ_type( స్తంభం_Mem(pStmt,i) );
  స్తంభం_Malloc_Failure(pStmt);
  తిరిగి iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
స్థిర స్థిరరాశి అక్షరం * స్థిరరాశి az_Explain_నిలువు_Names8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
స్థిర స్థిరరాశి u16 az_Explain_నిలువు_Names16data[] = {
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
స్థిర స్థిరరాశి u8 i_Explain_నిలువు_Names16[] = {
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
స్థిర స్థిరరాశి శూన్య *స్తంభం_Name(
  sqlite3_stmt *pStmt,     /* The statement */
  పూర్ణాంకం N,                   /* Which column to get the name for */
  పూర్ణాంకం useUtf16,            /* True to return the name as UTF16 */
  పూర్ణాంకం useType              /* What type of name */
){
  స్థిరరాశి శూన్య *ret;
  Vdbe *p;
  పూర్ణాంకం n;
  sqlite3 *db;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pStmt==0 ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  ఒకవేళ( N<0 ) తిరిగి 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  ప్రతిజ్ఞా( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  ఒకవేళ( p->explain ){
    ఒకవేళ( useType>0 ) వెళ్ళు స్తంభం_Name_చివర;
    n = p->explain==1 ? 8 : 4;
    ఒకవేళ( N>=n ) వెళ్ళు స్తంభం_Name_చివర;
    ఒకవేళ( useUtf16 ){
      పూర్ణాంకం i = i_Explain_నిలువు_Names16[N + 8*p->explain - 8];
      ret = (శూన్య*)&az_Explain_నిలువు_Names16data[i];
    }లేకపోతే{
      ret = (శూన్య*)az_Explain_నిలువు_Names8[N + 8*p->explain - 8];
    }
    వెళ్ళు స్తంభం_Name_చివర;
  }
  n = p->n_Res_స్తంభం;
  ఒకవేళ( N<n ){
    u8 పూర్వ_malloc_Failed = db->mallocFailed;
    N += useType*n;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
    ఒకవేళ( useUtf16 ){
      ret = sqlite3_విలువ_text16((sqlite3_విలువ*)&p->a_నిలువు_Name[N]);
    }లేకపోతే
#అయితే_ముగింపు
    {
      ret = sqlite3_విలువ_పాఠ్యం((sqlite3_విలువ*)&p->a_నిలువు_Name[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    ప్రతిజ్ఞా( db->mallocFailed==0 || db->mallocFailed==1 );
    ఒకవేళ( db->mallocFailed > పూర్వ_malloc_Failed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
స్తంభం_Name_చివర:
  sqlite3_mutex_leave(db->mutex);
  తిరిగి ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
స్థిరరాశి అక్షరం *sqlite3_స్తంభం_name(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 0, COLNAME_NAME);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_స్తంభం_name16(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 1, COLNAME_NAME);
}
#అయితే_ముగింపు

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#ఒకవేళ defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_స్తంభం_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_స్తంభం_METADATA"
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
స్థిరరాశి అక్షరం *sqlite3_స్తంభం_decltype(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 0, COLNAME_DECLTYPE);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_స్తంభం_decltype16(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 1, COLNAME_DECLTYPE);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
#అయితే_ముగింపు /* SQLITE_OMIT_DECLTYPE */

#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
స్థిరరాశి అక్షరం *sqlite3_స్తంభం_database_name(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 0, COLNAME_DATABASE);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_స్తంభం_database_name16(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 1, COLNAME_DATABASE);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
స్థిరరాశి అక్షరం *sqlite3_స్తంభం_పట్టిక_name(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 0, COLNAME_పట్టిక);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_స్తంభం_పట్టిక_name16(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 1, COLNAME_పట్టిక);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
స్థిరరాశి అక్షరం *sqlite3_స్తంభం_origin_name(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 0, COLNAME_స్తంభం);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిరరాశి శూన్య *sqlite3_స్తంభం_origin_name16(sqlite3_stmt *pStmt, పూర్ణాంకం N){
  తిరిగి స్తంభం_Name(pStmt, N, 1, COLNAME_స్తంభం);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
#అయితే_ముగింపు /* SQLITE_ENABLE_COLUMN_METADATA */


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
స్థిర పూర్ణాంకం vdbeUnbind(Vdbe *p, గుర్తులేని పూర్ణాంకం i){
  Mem *pVar;
  ఒకవేళ( vdbeSafetyNotNull(p) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ఒకవేళ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  ఒకవేళ( i>=(గుర్తులేని పూర్ణాంకం)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    తిరిగి SQLITE_RANGE;
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
  ప్రతిజ్ఞా( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ప్రతిజ్ఞా( (p->expmask & p->smimask)==p->smimask );
  ఒకవేళ( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    ఒకవేళ( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->err_సంకేతం = SQLITE_OK;

  తిరిగి SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
స్థిర పూర్ణాంకం bind_పాఠ్యం_2(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  పూర్ణాంకం i,                 /* Index of the parameter to bind */
  స్థిరరాశి శూన్య *z_దత్తాంశం,     /* Pointer to the data to be bound */
  i64 n_దత్తాంశం,             /* Number of bytes of data to be bound */
  శూన్య (*xDel)(శూన్య*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  పూర్ణాంకం rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    ఒకవేళ( z_దత్తాంశం!=0 ){
      pVar = &p->aVar[i-1];
      ఒకవేళ( encoding==SQLITE_UTF8 ){
        rc = sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(pVar, z_దత్తాంశం, n_దత్తాంశం, xDel);
      }లేకపోతే ఒకవేళ( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        ప్రతిజ్ఞా( z_దత్తాంశం==0
             || n_దత్తాంశం<0 
             || n_దత్తాంశం>pVar->db->a_హద్దు[SQLITE_హద్దు_నిడివి]
             || ((u8*)z_దత్తాంశం)[n_దత్తాంశం]==0
        );
        rc = sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(pVar, z_దత్తాంశం, n_దత్తాంశం, xDel);
        pVar->flags |= MEM_పదం;
      }లేకపోతే{
        rc = sqlite3_Vdbe_Mem_సమితి_Str(pVar, z_దత్తాంశం, n_దత్తాంశం, encoding, xDel);
        ఒకవేళ( encoding==0 ) pVar->enc = ENC(p->db);
      }
      ఒకవేళ( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      ఒకవేళ( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }లేకపోతే ఒకవేళ( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((శూన్య*)z_దత్తాంశం);
  }
  తిరిగి rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
పూర్ణాంకం sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం i,
  స్థిరరాశి శూన్య *z_దత్తాంశం,
  పూర్ణాంకం n_దత్తాంశం,
  శూన్య (*xDel)(శూన్య*)
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( n_దత్తాంశం<0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  తిరిగి bind_పాఠ్యం_2(pStmt, i, z_దత్తాంశం, n_దత్తాంశం, xDel, 0);
}
పూర్ణాంకం sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం i,
  స్థిరరాశి శూన్య *z_దత్తాంశం,
  sqlite3_uint64 n_దత్తాంశం,
  శూన్య (*xDel)(శూన్య*)
){
  ప్రతిజ్ఞా( xDel!=SQLITE_DYNAMIC );
  తిరిగి bind_పాఠ్యం_2(pStmt, i, z_దత్తాంశం, n_దత్తాంశం, xDel, 0);
}
పూర్ణాంకం sqlite3_bind_double(sqlite3_stmt *pStmt, పూర్ణాంకం i, ద్విగుణ r_విలువ){
  పూర్ణాంకం rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_సమితి_Double(&p->aVar[i-1], r_విలువ);
    sqlite3_mutex_leave(p->db->mutex);
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_bind_int(sqlite3_stmt *p, పూర్ణాంకం i, పూర్ణాంకం i_విలువ){
  తిరిగి sqlite3_bind_int64(p, i, (i64)i_విలువ);
}
పూర్ణాంకం sqlite3_bind_int64(sqlite3_stmt *pStmt, పూర్ణాంకం i, sqlite_int64 i_విలువ){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  ఒకవేళ( vdbeSafetyNotNull(p) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ఒకవేళ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  ఒకవేళ( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    తిరిగి SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  ఒకవేళ( p->expmask!=0 ){
    u32 expire_ముసుగు = i>=32 ? 0x80000000 : (u32)1 << i;
    ప్రతిజ్ఞా( (p->expmask & p->smimask)==p->smimask );
    ఒకవేళ( (p->expmask & expire_ముసుగు)!=0 ){
      ఒకవేళ( (p->smimask & expire_ముసుగు)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        పూర్ణాంకం sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        పూర్ణాంకం sm2 = i_విలువ>=0 && i_విలువ<=1;
        ఒకవేళ( sm1!=sm2 ){
          p->expired = 1;
        }
      }లేకపోతే ఒకవేళ( (pVar->flags & MEM_Int)==0 || i_విలువ!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3_Vdbe_Mem_సమితి_Int64(pVar, i_విలువ);
  sqlite3_mutex_leave(p->db->mutex);
  తిరిగి SQLITE_OK;
}
పూర్ణాంకం sqlite3_bind_null(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  పూర్ణాంకం rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_bind_సూచిక(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం i,
  శూన్య *pPtr,
  స్థిరరాశి అక్షరం *zPTtype,
  శూన్య (*xDestructor)(శూన్య*)
){
  పూర్ణాంకం rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_సమితి_సూచిక(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }లేకపోతే ఒకవేళ( xDestructor ){
    xDestructor(pPtr);
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_bind_పాఠ్యం(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం i,
  స్థిరరాశి అక్షరం *z_దత్తాంశం,
  పూర్ణాంకం n_దత్తాంశం,
  శూన్య (*xDel)(శూన్య*)
){
  తిరిగి bind_పాఠ్యం_2(pStmt, i, z_దత్తాంశం, n_దత్తాంశం, xDel, SQLITE_UTF8);
}
పూర్ణాంకం sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం i,
  స్థిరరాశి అక్షరం *z_దత్తాంశం,
  sqlite3_uint64 n_దత్తాంశం,
  శూన్య (*xDel)(శూన్య*),
  గుర్తులేని అక్షరం enc
){
  ప్రతిజ్ఞా( xDel!=SQLITE_DYNAMIC );
  ఒకవేళ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ఒకవేళ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_దత్తాంశం &= ~(u64)1;
  }
  తిరిగి bind_పాఠ్యం_2(pStmt, i, z_దత్తాంశం, n_దత్తాంశం, xDel, enc);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
పూర్ణాంకం sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం i,
  స్థిరరాశి శూన్య *z_దత్తాంశం,
  పూర్ణాంకం n,
  శూన్య (*xDel)(శూన్య*)
){
  తిరిగి bind_పాఠ్యం_2(pStmt, i, z_దత్తాంశం, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
పూర్ణాంకం sqlite3_bind_విలువ(sqlite3_stmt *pStmt, పూర్ణాంకం i, స్థిరరాశి sqlite3_విలువ *p_విలువ){
  పూర్ణాంకం rc;
  ఎంపిక( sqlite3_విలువ_type((sqlite3_విలువ*)p_విలువ) ){
    స్థితి SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_విలువ->u.i);
      ఆపు;
    }
    స్థితి SQLITE_FLOAT: {
      ప్రతిజ్ఞా( p_విలువ->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_విలువ->flags & MEM_Real) ? p_విలువ->u.r : (ద్విగుణ)p_విలువ->u.i
      );
      ఆపు;
    }
    స్థితి SQLITE_BLOB: {
      ఒకవేళ( p_విలువ->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_విలువ->u.nZero);
      }లేకపోతే{
        rc = sqlite3_bind_blob(pStmt, i, p_విలువ->z, p_విలువ->n,SQLITE_TRANSIENT);
      }
      ఆపు;
    }
    స్థితి SQLITE_పాఠ్యం: {
      rc = bind_పాఠ్యం_2(pStmt,i,  p_విలువ->z, p_విలువ->n, SQLITE_TRANSIENT,
                              p_విలువ->enc);
      ఆపు;
    }
    అప్రమేయ: {
      rc = sqlite3_bind_null(pStmt, i);
      ఆపు;
    }
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, పూర్ణాంకం i, పూర్ణాంకం n){
  పూర్ణాంకం rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
    sqlite3_Vdbe_Mem_సమితి_Zero_Blob(&p->aVar[i-1], n);
#లేకపోతే
    rc = sqlite3_Vdbe_Mem_సమితి_Zero_Blob(&p->aVar[i-1], n);
#అయితే_ముగింపు
    sqlite3_mutex_leave(p->db->mutex);
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, పూర్ణాంకం i, sqlite3_uint64 n){
  పూర్ణాంకం rc;
  Vdbe *p = (Vdbe *)pStmt;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(p->db->mutex);
  ఒకవేళ( n>(u64)p->db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    rc = SQLITE_TOOBIG;
  }లేకపోతే{
    ప్రతిజ్ఞా( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  తిరిగి rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
పూర్ణాంకం sqlite3_bind_parameter_లెక్క(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  తిరిగి p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
స్థిరరాశి అక్షరం *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, పూర్ణాంకం i){
  Vdbe *p = (Vdbe*)pStmt;
  ఒకవేళ( p==0 ) తిరిగి 0;
  తిరిగి sqlite3_VList_అంకె_గమ్యం_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
పూర్ణాంకం sqlite3_Vdbe_Parameter_సూచీ(Vdbe *p, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం nName){
  ఒకవేళ( p==0 || zName==0 ) తిరిగి 0;
  తిరిగి sqlite3_VList_Name_గమ్యం_అంకె(p->pVList, zName, nName);
}
పూర్ణాంకం sqlite3_bind_parameter_సూచీ(sqlite3_stmt *pStmt, స్థిరరాశి అక్షరం *zName){
  తిరిగి sqlite3_Vdbe_Parameter_సూచీ((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
పూర్ణాంకం sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_గమ్యం_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_గమ్యం = (Vdbe*)p_గమ్యం_Stmt;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( p_గమ్యం->db==pFrom->db );
  ప్రతిజ్ఞా( p_గమ్యం->nVar==pFrom->nVar );
  sqlite3_mutex_enter(p_గమ్యం->db->mutex);
  కొరకు(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&p_గమ్యం->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(p_గమ్యం->db->mutex);
  తిరిగి SQLITE_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
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
పూర్ణాంకం sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_గమ్యం_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_గమ్యం = (Vdbe*)p_గమ్యం_Stmt;
  ఒకవేళ( pFrom->nVar!=p_గమ్యం->nVar ){
    తిరిగి SQLITE_ERROR;
  }
  ప్రతిజ్ఞా( (p_గమ్యం->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p_గమ్యం->expmask==0 );
  ఒకవేళ( p_గమ్యం->expmask ){
    p_గమ్యం->expired = 1;
  }
  ప్రతిజ్ఞా( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  ఒకవేళ( pFrom->expmask ){
    pFrom->expired = 1;
  }
  తిరిగి sqlite3TransferBindings(pFromStmt, p_గమ్యం_Stmt);
}
#అయితే_ముగింపు

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  తిరిగి pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
పూర్ణాంకం sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  తిరిగి pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
పూర్ణాంకం sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  తిరిగి pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
పూర్ణాంకం sqlite3_stmt_explain(sqlite3_stmt *pStmt, పూర్ణాంకం e_బహుళకం){
  Vdbe *v = (Vdbe*)pStmt;
  పూర్ణాంకం rc;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pStmt==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(v->db->mutex);
  ఒకవేళ( ((పూర్ణాంకం)v->explain)==e_బహుళకం ){
    rc = SQLITE_OK;
  }లేకపోతే ఒకవేళ( e_బహుళకం<0 || e_బహుళకం>2 ){
    rc = SQLITE_ERROR;
  }లేకపోతే ఒకవేళ( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }లేకపోతే ఒకవేళ( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }లేకపోతే ఒకవేళ( v->nMem>=10 && (e_బహుళకం!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = e_బహుళకం;
    rc = SQLITE_OK;
  }లేకపోతే{
    పూర్ణాంకం saved_బహుళకం = v->explain;
    v->explain = e_బహుళకం;
    rc = sqlite3Reprepare(v);
    ఒకవేళ( rc ){
      v->explain = saved_బహుళకం;
    }లేకపోతే{
      v->haveEqpOps = e_బహుళకం==2;
    }
  }
  ఒకవేళ( v->explain ){
    v->n_Res_స్తంభం = 12 - 4*v->explain;
  }లేకపోతే{
    v->n_Res_స్తంభం = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  తిరిగి rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
పూర్ణాంకం sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  తిరిగి v!=0 && v->eVdbeState==VDBE_నిరంతరం_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_తదుపరి_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_తదుపరి;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(pDb) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(pDb->mutex);
  ఒకవేళ( pStmt==0 ){
    p_తదుపరి = (sqlite3_stmt*)pDb->pVdbe;
  }లేకపోతే{
    p_తదుపరి = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  తిరిగి p_తదుపరి;
}

/*
** Return the value of a status counter for a prepared statement
*/
పూర్ణాంకం sqlite3_stmt_స్థితి(sqlite3_stmt *pStmt, పూర్ణాంకం op, పూర్ణాంకం resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_కొలత(pVdbe->aCounter)))
  ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  ఒకవేళ( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pn_బైట్లు_Freed = (పూర్ణాంకం*)&v;
    ప్రతిజ్ఞా( db->lookaside.p_చివర==db->lookaside.p_True_చివర );
    db->lookaside.p_చివర = db->lookaside.p_ప్రారంభం;
    sqlite3VdbeDelete(pVdbe);
    db->pn_బైట్లు_Freed = 0;
    db->lookaside.p_చివర = db->lookaside.p_True_చివర;
    sqlite3_mutex_leave(db->mutex);
  }లేకపోతే{
    v = pVdbe->aCounter[op];
    ఒకవేళ( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  తిరిగి (పూర్ణాంకం)v;
}

/*
** Return the SQL associated with a prepared statement
*/
స్థిరరాశి అక్షరం *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  తిరిగి p ? p->zSql : 0;
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
అక్షరం *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#నిర్వచితమైతే SQLITE_OMIT_TRACE
  తిరిగి 0;
#లేకపోతే
  అక్షరం *z = 0;
  స్థిరరాశి అక్షరం *zSql = sqlite3_sql(pStmt);
  ఒకవేళ( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  తిరిగి z;
#అయితే_ముగింపు
}

#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
స్థిరరాశి అక్షరం *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ఒకవేళ( p==0 ) తిరిగి 0;
  ఒకవేళ( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  తిరిగి p->zNormSql;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_NORMALIZE */

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
స్థిర Unpacked_రికార్డు *vdbe_Unpack_రికార్డు(
  కీలకం_Info *p_కీలకం_Info,
  పూర్ణాంకం n_కీలకం,
  స్థిరరాశి శూన్య *p_కీలకం
){
  Unpacked_రికార్డు *pRet;           /* Return value */

  pRet = sqlite3_Vdbe_Alloc_Unpacked_రికార్డు(p_కీలకం_Info);
  ఒకవేళ( pRet ){
    memset(pRet->aMem, 0, పరిమాణం(Mem)*(p_కీలకం_Info->n_కీలకం_క్షేత్రం+1));
    sqlite3_Vdbe_రికార్డు_Unpack(n_కీలకం, p_కీలకం, pRet);
  }
  తిరిగి pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
పూర్ణాంకం sqlite3_preupdate_old(sqlite3 *db, పూర్ణాంకం iIdx, sqlite3_విలువ **pp_విలువ){
  PreUpdate *p;
  Mem *pMem;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iStore = 0;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( db==0 || pp_విలువ==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  ఒకవేళ( !p || p->op==SQLITE_చొప్పించు ){
    rc = SQLITE_MISUSE_BKPT;
    వెళ్ళు preupdate_old_బహిర్గతం;
  }
  ఒకవేళ( p->pPk ){
    iStore = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(p->pPk, iIdx);
  }లేకపోతే ఒకవేళ( iIdx >= p->pTab->n_నిలువు ){
    rc = SQLITE_MISUSE_BKPT;
    వెళ్ళు preupdate_old_బహిర్గతం;
  }లేకపోతే{
    iStore = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(p->pTab, iIdx);
  }
  ఒకవేళ( iStore>=p->pCsr->n_క్షేత్రం || iStore<0 ){
    rc = SQLITE_RANGE;
    వెళ్ళు preupdate_old_బహిర్గతం;
  }

  ఒకవేళ( iIdx==p->pTab->iPKey ){
    *pp_విలువ = pMem = &p->oldipk;
    sqlite3_Vdbe_Mem_సమితి_Int64(pMem, p->iKey1);
  }లేకపోతే{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    ఒకవేళ( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      ప్రతిజ్ఞా( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_కొలత(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      ఒకవేళ( !aRec ) వెళ్ళు preupdate_old_బహిర్గతం;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      ఒకవేళ( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbe_Unpack_రికార్డు(p->pKeyinfo, nRec, aRec);
        ఒకవేళ( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      ఒకవేళ( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        వెళ్ళు preupdate_old_బహిర్గతం;
      }
      p->a_రికార్డు = aRec;
    }

    pMem = *pp_విలువ = &p->pUnpacked->aMem[iStore];
    ఒకవేళ( iStore>=p->pUnpacked->n_క్షేత్రం ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      స్తంభం *p_నిలువు = &p->pTab->a_నిలువు[iIdx];
      ఒకవేళ( p_నిలువు->iDflt>0 ){
        ఒకవేళ( p->apDflt==0 ){
          పూర్ణాంకం nByte;
          ప్రతిజ్ఞా( పరిమాణం(sqlite3_విలువ*)*UMXV(p->pTab->n_నిలువు) < 0x7fffffff );
          nByte = పరిమాణం(sqlite3_విలువ*)*p->pTab->n_నిలువు;
          p->apDflt = (sqlite3_విలువ**)sqlite3DbMallocZero(db, nByte);
          ఒకవేళ( p->apDflt==0 ) వెళ్ళు preupdate_old_బహిర్గతం;
        }
        ఒకవేళ( p->apDflt[iIdx]==0 ){
          sqlite3_విలువ *pVal = 0;
          సమాసం *pDflt;
          ప్రతిజ్ఞా( p->pTab!=0 && Is_Ordinary_పట్టిక(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[p_నిలువు->iDflt-1].p_సమాసం;
          rc = sqlite3_విలువ_From_సమాసం(db, pDflt, ENC(db), p_నిలువు->affinity, &pVal);
          ఒకవేళ( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_విలువ = p->apDflt[iIdx];
      }లేకపోతే{
        *pp_విలువ = (sqlite3_విలువ *)స్తంభం_Null_విలువ();
      }
    }లేకపోతే ఒకవేళ( p->pTab->a_నిలువు[iIdx].affinity==SQLITE_AFF_REAL ){
      ఒకవేళ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_బహిర్గతం:
  sqlite3Error(db, rc);
  తిరిగి sqlite3ApiExit(db, rc);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
పూర్ణాంకం sqlite3_preupdate_లెక్క(sqlite3 *db){
  PreUpdate *p;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#లేకపోతే
  p = db->pPreUpdate;
#అయితే_ముగింపు
  తిరిగి (p ? p->pKeyinfo->n_కీలకం_క్షేత్రం : 0);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
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
పూర్ణాంకం sqlite3_preupdate_లోతు(sqlite3 *db){
  PreUpdate *p;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#లేకపోతే
  p = db->pPreUpdate;
#అయితే_ముగింపు
  తిరిగి (p ? p->v->n_చట్రం : 0);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
పూర్ణాంకం sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#లేకపోతే
  p = db->pPreUpdate;
#అయితే_ముగింపు
  తిరిగి (p ? p->iBlobWrite : -1);
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
పూర్ణాంకం sqlite3_preupdate_new(sqlite3 *db, పూర్ణాంకం iIdx, sqlite3_విలువ **pp_విలువ){
  PreUpdate *p;
  పూర్ణాంకం rc = SQLITE_OK;
  Mem *pMem;
  పూర్ణాంకం iStore = 0;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( db==0 || pp_విలువ==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  p = db->pPreUpdate;
  ఒకవేళ( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    వెళ్ళు preupdate_new_బహిర్గతం;
  }
  ఒకవేళ( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(p->pPk, iIdx);
  }లేకపోతే ఒకవేళ( iIdx >= p->pTab->n_నిలువు ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }లేకపోతే{
    iStore = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(p->pTab, iIdx);
  }

  ఒకవేళ( iStore>=p->pCsr->n_క్షేత్రం || iStore<0 ){
    rc = SQLITE_RANGE;
    వెళ్ళు preupdate_new_బహిర్గతం;
  }

  ఒకవేళ( p->op==SQLITE_చొప్పించు ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    Unpacked_రికార్డు *pUnpack = p->pNewUnpacked;
    ఒకవేళ( !pUnpack ){
      Mem *p_దత్తాంశం = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_దత్తాంశం);
      ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు preupdate_new_బహిర్గతం;
      pUnpack = vdbe_Unpack_రికార్డు(p->pKeyinfo, p_దత్తాంశం->n, p_దత్తాంశం->z);
      ఒకవేళ( !pUnpack ){
        rc = SQLITE_NOMEM;
        వెళ్ళు preupdate_new_బహిర్గతం;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    ఒకవేళ( iIdx==p->pTab->iPKey ){
      sqlite3_Vdbe_Mem_సమితి_Int64(pMem, p->iKey2);
    }లేకపోతే ఒకవేళ( iStore>=pUnpack->n_క్షేత్రం ){
      pMem = (sqlite3_విలువ *)స్తంభం_Null_విలువ();
    }లేకపోతే ఒకవేళ( (p->pTab->a_నిలువు)
           && (p->pTab->a_నిలువు[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }లేకపోతే{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    ప్రతిజ్ఞా( p->op==SQLITE_UPDATE );
    ఒకవేళ( !p->aNew ){
      ప్రతిజ్ఞా( పరిమాణం(Mem)*UMXV(p->pCsr->n_క్షేత్రం) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, పరిమాణం(Mem)*p->pCsr->n_క్షేత్రం);
      ఒకవేళ( !p->aNew ){
        rc = SQLITE_NOMEM;
        వెళ్ళు preupdate_new_బహిర్గతం;
      }
    }
    ప్రతిజ్ఞా( iStore>=0 && iStore<p->pCsr->n_క్షేత్రం );
    pMem = &p->aNew[iStore];
    ఒకవేళ( pMem->flags==0 ){
      ఒకవేళ( iIdx==p->pTab->iPKey ){
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem, p->iKey2);
      }లేకపోతే{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు preupdate_new_బహిర్గతం;
      }
    }
  }
  *pp_విలువ = pMem;

 preupdate_new_బహిర్గతం:
  sqlite3Error(db, rc);
  తిరిగి sqlite3ApiExit(db, rc);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
పూర్ణాంకం sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  పూర్ణాంకం i_పరిశీలన,                      /* Index of loop to report on */
  పూర్ణాంకం i_పరిశీలన_స్థితి_Op,              /* Which metric to return */
  పూర్ణాంకం flags,
  శూన్య *p_బహిర్గతం                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  పూర్ణాంకం nOp;
  పరిశీలన_స్థితి *p_పరిశీలన = 0;
  పూర్ణాంకం idx;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 || p_బహిర్గతం==0
      || i_పరిశీలన_స్థితి_Op<SQLITE_SCANSTAT_NLOOP
      || i_పరిశీలన_స్థితి_Op>SQLITE_SCANSTAT_NCYCLE ){
    తిరిగి 1;
  }
#అయితే_ముగింపు
  aOp = p->aOp;
  nOp = p->nOp;
  ఒకవేళ( p->p_చట్రం ){
    Vdbe_చట్రం *p_చట్రం;
    కొరకు(p_చట్రం=p->p_చట్రం; p_చట్రం->p_మాతృ; p_చట్రం=p_చట్రం->p_మాతృ);
    aOp = p_చట్రం->aOp;
    nOp = p_చట్రం->nOp;
  }

  ఒకవేళ( i_పరిశీలన<0 ){
    పూర్ణాంకం ii;
    ఒకవేళ( i_పరిశీలన_స్థితి_Op==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      కొరకు(ii=0; ii<nOp; ii++){
        res += aOp[ii].n_చక్రం;
      }
      *(i64*)p_బహిర్గతం = res;
      తిరిగి 0;
    }
    తిరిగి 1;
  }
  ఒకవేళ( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = i_పరిశీలన;
  }లేకపోతే{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    కొరకు(idx=0; idx<p->n_పరిశీలన; idx++){
      p_పరిశీలన = &p->a_పరిశీలన[idx];
      ఒకవేళ( p_పరిశీలన->zName ){
        i_పరిశీలన--;
        ఒకవేళ( i_పరిశీలన<0 ) ఆపు;
      }
    }
  }
  ఒకవేళ( idx>=p->n_పరిశీలన ) తిరిగి 1;
  ప్రతిజ్ఞా( p_పరిశీలన==0 || p_పరిశీలన==&p->a_పరిశీలన[idx] );
  p_పరిశీలన = &p->a_పరిశీలన[idx];

  ఎంపిక( i_పరిశీలన_స్థితి_Op ){
    స్థితి SQLITE_SCANSTAT_NLOOP: {
      ఒకవేళ( p_పరిశీలన->addrLoop>0 ){
        *(sqlite3_int64*)p_బహిర్గతం = aOp[p_పరిశీలన->addrLoop].nExec;
      }లేకపోతే{
        *(sqlite3_int64*)p_బహిర్గతం = -1;
      }
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_NVISIT: {
      ఒకవేళ( p_పరిశీలన->addr_సందర్శించు>0 ){
        *(sqlite3_int64*)p_బహిర్గతం = aOp[p_పరిశీలన->addr_సందర్శించు].nExec;
      }లేకపోతే{
        *(sqlite3_int64*)p_బహిర్గతం = -1;
      }
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_EST: {
      ద్విగుణ r = 1.0;
      LogEst x = p_పరిశీలన->nEst;
      అయితే( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(ద్విగుణ*)p_బహిర్గతం = r*sqlite3_Log_Est_గమ్యం_Int(x);
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_NAME: {
      *(స్థిరరాశి అక్షరం**)p_బహిర్గతం = p_పరిశీలన->zName;
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_EXPLAIN: {
      ఒకవేళ( p_పరిశీలన->addrExplain ){
        *(స్థిరరాశి అక్షరం**)p_బహిర్గతం = aOp[ p_పరిశీలన->addrExplain ].p4.z;
      }లేకపోతే{
        *(స్థిరరాశి అక్షరం**)p_బహిర్గతం = 0;
      }
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_SELECTID: {
      ఒకవేళ( p_పరిశీలన->addrExplain ){
        *(పూర్ణాంకం*)p_బహిర్గతం = aOp[ p_పరిశీలన->addrExplain ].p1;
      }లేకపోతే{
        *(పూర్ణాంకం*)p_బహిర్గతం = -1;
      }
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_PARENTID: {
      ఒకవేళ( p_పరిశీలన->addrExplain ){
        *(పూర్ణాంకం*)p_బహిర్గతం = aOp[ p_పరిశీలన->addrExplain ].p2;
      }లేకపోతే{
        *(పూర్ణాంకం*)p_బహిర్గతం = -1;
      }
      ఆపు;
    }
    స్థితి SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      ఒకవేళ( p_పరిశీలన->aAddrRange[0]==0 ){
        res = -1;
      }లేకపోతే{
        పూర్ణాంకం ii;
        కొరకు(ii=0; ii<Array_కొలత(p_పరిశీలన->aAddrRange); ii+=2){
          పూర్ణాంకం iIns = p_పరిశీలన->aAddrRange[ii];
          పూర్ణాంకం i_చివర = p_పరిశీలన->aAddrRange[ii+1];
          ఒకవేళ( iIns==0 ) ఆపు;
          ఒకవేళ( iIns>0 ){
            అయితే( iIns<=i_చివర ){
              res += aOp[iIns].n_చక్రం;
              iIns++;
            }
          }లేకపోతే{
            పూర్ణాంకం iOp;
            కొరకు(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              ఒకవేళ( pOp->p1!=i_చివర ) కొనసాగు;
              ఒకవేళ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                కొనసాగు;
              }
              res += aOp[iOp].n_చక్రం;
            }
          }
        }
      }
      *(i64*)p_బహిర్గతం = res;
      ఆపు;
    }
    అప్రమేయ: {
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
పూర్ణాంకం sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  పూర్ణాంకం i_పరిశీలన,                      /* Index of loop to report on */
  పూర్ణాంకం i_పరిశీలన_స్థితి_Op,              /* Which metric to return */
  శూన్య *p_బహిర్గతం                      /* OUT: Write the answer here */
){
  తిరిగి sqlite3_stmt_scanstatus_v2(pStmt, i_పరిశీలన, i_పరిశీలన_స్థితి_Op, 0, p_బహిర్గతం);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
శూన్య sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  పూర్ణాంకం ii;
  కొరకు(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->n_చక్రం = 0;
  }
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STMT_SCANSTATUS */
