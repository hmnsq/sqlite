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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
ಪೂರ್ಣಾಂಕ sqlite3_expired(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಂಕ iRet = 1;
  ಆದರೆ( pStmt ){
    Vdbe *p = (Vdbe*)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಹಿಂತಿರುಗಿ iRet;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSafety(Vdbe *p){
  ಆದರೆ( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSafetyNotNull(Vdbe *p){
  ಆದರೆ( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ vdbeSafety(p);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
/*
** Invoke the profile callback.  This routine is only called if we already
** know that the profile callback is defined and needs to be invoked.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ invokeProfileCallback(sqlite3 *db, Vdbe *p){
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;
  ಪ್ರತಿಜ್ಞಾ( p->ಆರಂಭ_ಸಮಯ>0 );
  ಪ್ರತಿಜ್ಞಾ( db->init.busy==0 );
  ಪ್ರತಿಜ್ಞಾ( p->zSql!=0 );
  sqlite3_Os_Current_ಸಮಯ_Int64(db->pVfs, &iNow);
  iElapse = (iNow - p->ಆರಂಭ_ಸಮಯ)*1000000;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  ಆದರೆ( db->xProfile ){
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( db->mTrace & SQLITE_TRACE_PROFILE ){
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (ಶೂನ್ಯ*)&iElapse);
  }
  p->ಆರಂಭ_ಸಮಯ = 0;
}
/*
** The checkProfileCallback(DB,P) macro checks to see if a profile callback
** is needed, and it invokes the callback if it is needed.
*/
# define checkProfileCallback(DB,P) \
   ಆದರೆ( ((P)->ಆರಂಭ_ಸಮಯ)>0 ){ invokeProfileCallback(DB,P); }
#ಇಲ್ಲದಿದ್ದರೆ
# define checkProfileCallback(DB,P)  /*no-op*/
#ಆದರೆ_ಅಂತ್ಯ

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
ಪೂರ್ಣಾಂಕ sqlite3_finalize(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    ಆದರೆ( vdbeSafety(v) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    ಪ್ರತಿಜ್ಞಾ( v->eVdbeState>=VDBE_READY_STATE );
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
ಪೂರ್ಣಾಂಕ sqlite3_reset(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( pStmt==0 ){
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    checkProfileCallback(db, v);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    ಪ್ರತಿಜ್ಞಾ( (rc & (db->err_ಮುಖವಾಡ))==rc );
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
ಪೂರ್ಣಾಂಕ sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
  u32 ಮುಖವಾಡ_Small_Int = 0;  /* Bitmask of bindings with value 0 or 1 */
#ಆದರೆ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pStmt==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_THREADSAFE
  mutex = p->db->mutex;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_mutex_enter(mutex);
  ಗಾಗಿ(i=0; i<p->nVar; i++){
    Mem *pVar = &p->aVar[i];
    ಆದರೆ( (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1 ){
      ಮುಖವಾಡ_Small_Int |= i>=31 ? 0x80000000 : ((u32)1)<<i;
    }
    sqlite3VdbeMemRelease(pVar);
    pVar->flags = MEM_Null;
  }
  ಪ್ರತಿಜ್ಞಾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p->expmask==0 );
  ಆದರೆ( (p->expmask&~p->smimask)!=0 || (p->smimask&ಮುಖವಾಡ_Small_Int)!=0 ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  ಹಿಂತಿರುಗಿ rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_blob(sqlite3_ಮೌಲ್ಯ *pVal){
  Mem *p = (Mem*)pVal;
  ಆದರೆ( p->flags & (MEM_Blob|MEM_Str) ){
    ಆದರೆ( ExpandBlob(p)!=SQLITE_OK ){
      ಪ್ರತಿಜ್ಞಾ( p->flags==MEM_Null && p->z==0 );
      ಹಿಂತಿರುಗಿ 0;
    }
    p->flags |= MEM_Blob;
    ಹಿಂತಿರುಗಿ p->n ? p->z : 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pVal);
  }
}
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(pVal, SQLITE_UTF8);
}
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_bytes16(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(pVal, SQLITE_UTF16NATIVE);
}
ದ್ವಿಗುಣ sqlite3_ಮೌಲ್ಯ_double(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_Real_ಮೌಲ್ಯ((Mem*)pVal);
}
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_int(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)sqlite3_Vdbe_Int_ಮೌಲ್ಯ((Mem*)pVal);
}
sqlite_int64 sqlite3_ಮೌಲ್ಯ_int64(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_Int_ಮೌಲ್ಯ((Mem*)pVal);
}
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_subtype(sqlite3_ಮೌಲ್ಯ *pVal){
  Mem *pMem = (Mem*)pVal;
  ಹಿಂತಿರುಗಿ ((pMem->flags & MEM_Subtype) ? pMem->eSubtype : 0);
}
ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_ಸೂಚಕ(sqlite3_ಮೌಲ್ಯ *pVal, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPType){
  Mem *p = (Mem*)pVal;
  ಆದರೆ( (p->flags&(MEM_Type_ಮುಖವಾಡ|MEM_ಪದ|MEM_Subtype)) ==
                 (MEM_Null|MEM_ಪದ|MEM_Subtype)
   && zPType!=0
   && p->eSubtype=='p'
   && strcmp(p->u.zPType, zPType)==0
  ){
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)p->z;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}
ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, SQLITE_UTF8);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_text16(sqlite3_ಮೌಲ್ಯ* pVal){
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, SQLITE_UTF16NATIVE);
}
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_text16be(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, SQLITE_UTF16BE);
}
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_text16le(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, SQLITE_UTF16LE);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
/* EVIDENCE-OF: R-12793-43283 Every value in SQLite has one of five
** fundamental datatypes: 64-bit signed integer 64-bit IEEE floating
** point number string BLOB NULL
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_type(sqlite3_ಮೌಲ್ಯ* pVal){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 aType[] = {
     SQLITE_BLOB,     /* 0x00 (not possible) */
     SQLITE_NULL,     /* 0x01 NULL */
     SQLITE_ಪಠ್ಯ,     /* 0x02 TEXT */
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
     SQLITE_ಪಠ್ಯ,     /* 0x12 (not possible) */
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
     SQLITE_ಪಠ್ಯ,     /* 0x32 (not possible) */
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  {
    ಪೂರ್ಣಾಂಕ eType = SQLITE_BLOB;
    ಆದರೆ( pVal->flags & MEM_Null ){
      eType = SQLITE_NULL;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVal->flags & (MEM_Real|MEM_IntReal) ){
      eType = SQLITE_FLOAT;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVal->flags & MEM_Int ){
      eType = SQLITE_INTEGER;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVal->flags & MEM_Str ){
      eType = SQLITE_ಪಠ್ಯ;
    }
    ಪ್ರತಿಜ್ಞಾ( eType == aType[pVal->flags&MEM_Aff_ಮುಖವಾಡ] );
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ aType[pVal->flags&MEM_Aff_ಮುಖವಾಡ];
}
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_encoding(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ pVal->enc;
}

/* Return true if a parameter to xUpdate represents an unchanged column */
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_nochange(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ (pVal->flags&(MEM_Null|MEM_Zero))==(MEM_Null|MEM_Zero);
}

/* Return true if a parameter value originated from an sqlite3_bind() */
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_frombind(sqlite3_ಮೌಲ್ಯ *pVal){
  ಹಿಂತಿರುಗಿ (pVal->flags&MEM_FromBind)!=0;
}

/* Make a copy of an sqlite3_value object
*/
sqlite3_ಮೌಲ್ಯ *sqlite3_ಮೌಲ್ಯ_dup(ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ *pOrig){
  sqlite3_ಮೌಲ್ಯ *pNew;
  ಆದರೆ( pOrig==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew = sqlite3_malloc( ಗಾತ್ರ(*pNew) );
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ 0;
  memset(pNew, 0, ಗಾತ್ರ(*pNew));
  ಸ್ಮರಣೆ_ನಕಲು(pNew, pOrig, MEMCELLSIZE);
  pNew->flags &= ~MEM_Dyn;
  pNew->db = 0;
  ಆದರೆ( pNew->flags&(MEM_Str|MEM_Blob) ){
    pNew->flags &= ~(MEM_Static|MEM_Dyn);
    pNew->flags |= MEM_Ephem;
    ಆದರೆ( sqlite3VdbeMemMakeWriteable(pNew)!=SQLITE_OK ){
      sqlite3_ಮೌಲ್ಯ_Free(pNew);
      pNew = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pNew->flags & MEM_Null ){
    /* Do not duplicate pointer values */
    pNew->flags &= ~(MEM_ಪದ|MEM_Subtype);
  }
  ಹಿಂತಿರುಗಿ pNew;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(
  sqlite3_context *pCtx,  /* Function context */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z,          /* String pointer */
  ಪೂರ್ಣಾಂಕ n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)     /* Destructor function */
){
  Mem *p_ಹೊರಹೋಗು = pCtx->p_ಹೊರಹೋಗು;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( enc==SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(p_ಹೊರಹೋಗು, z, n, xDel);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( enc==SQLITE_UTF8_ZT ){
    /* It is usually considered improper to assert() on an input. However,
    ** the following assert() is checking for inputs that are documented
    ** to result in undefined behavior. */
    ಪ್ರತಿಜ್ಞಾ( z==0
         || n<0 
         || n>p_ಹೊರಹೋಗು->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]
         || z[n]==0
    );
    rc = sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(p_ಹೊರಹೋಗು, z, n, xDel);
    p_ಹೊರಹೋಗು->flags |= MEM_ಪದ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3_Vdbe_Mem_ಗಣ_Str(p_ಹೊರಹೋಗು, z, n, enc, xDel);
  }
  ಆದರೆ( rc ){
    ಆದರೆ( rc==SQLITE_TOOBIG ){
      sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* The only errors possible from sqlite3VdbeMemSetStr are
      ** SQLITE_TOOBIG and SQLITE_NOMEM */
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_NOMEM );
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
    }
    ಹಿಂತಿರುಗಿ;
  }
  sqlite3VdbeChangeEncoding(p_ಹೊರಹೋಗು, pCtx->enc);
  ಆದರೆ( sqlite3VdbeMemTooBig(p_ಹೊರಹೋಗು) ){
    sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ invoke_ಮೌಲ್ಯ_Destructor(
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p,             /* Value to destroy */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*),       /* The destructor */
  sqlite3_context *pCtx      /* Set a SQLITE_TOOBIG error if not NULL */
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಆದರೆ( xDel==0 ){
    /* noop */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xDel==SQLITE_TRANSIENT ){
    /* noop */
  }ಇಲ್ಲದಿದ್ದರೆ{
    xDel((ಶೂನ್ಯ*)p);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx!=0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( pCtx!=0 );
  sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_TOOBIG;
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_blob(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಂಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 || n<0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, pCtx);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( n>=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, n, 0, xDel);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_blob64(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  sqlite3_uint64 n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, 0);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಆದರೆ( n>0x7fffffff ){
    (ಶೂನ್ಯ)invoke_ಮೌಲ್ಯ_Destructor(z, xDel, pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, (ಪೂರ್ಣಾಂಕ)n, 0, xDel);
  }
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_double(sqlite3_context *pCtx, ದ್ವಿಗುಣ rVal){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3_Vdbe_Mem_ಗಣ_Double(pCtx->p_ಹೊರಹೋಗು, rVal);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_error(sqlite3_context *pCtx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ n){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_ಗಣ_Str(pCtx->p_ಹೊರಹೋಗು, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_error16(sqlite3_context *pCtx, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z, ಪೂರ್ಣಾಂಕ n){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3_Vdbe_Mem_ಗಣ_Str(pCtx->p_ಹೊರಹೋಗು, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_int(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ iVal){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3_Vdbe_Mem_ಗಣ_Int64(pCtx->p_ಹೊರಹೋಗು, (i64)iVal);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_int64(sqlite3_context *pCtx, i64 iVal){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3_Vdbe_Mem_ಗಣ_Int64(pCtx->p_ಹೊರಹೋಗು, iVal);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_null(sqlite3_context *pCtx){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3_Vdbe_Mem_ಗಣ_Null(pCtx->p_ಹೊರಹೋಗು);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_ಸೂಚಕ(
  sqlite3_context *pCtx,
  ಶೂನ್ಯ *pPtr,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPType,
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)
){
  Mem *p_ಹೊರಹೋಗು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(pPtr, xDestructor, 0);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  p_ಹೊರಹೋಗು = pCtx->p_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3VdbeMemRelease(p_ಹೊರಹೋಗು);
  p_ಹೊರಹೋಗು->flags = MEM_Null;
  sqlite3_Vdbe_Mem_ಗಣ_ಸೂಚಕ(p_ಹೊರಹೋಗು, pPtr, zPType, xDestructor);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_subtype(sqlite3_context *pCtx, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ eSubtype){
  Mem *p_ಹೊರಹೋಗು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_STRICT_SUBTYPE) && SQLITE_STRICT_SUBTYPE+0!=0
  ಆದರೆ( pCtx->pFunc!=0
   && (pCtx->pFunc->funcFlags & SQLITE_ಫಲಿತಾಂಶ_SUBTYPE)==0
  ){
    ಅಕ್ಷರ zErr[200];
    sqlite3_snprintf(ಗಾತ್ರ(zErr), zErr,
                     "misuse of sqlite3_result_subtype() by %s()", 
                     pCtx->pFunc->zName);
    sqlite3_ಫಲಿತಾಂಶ_error(pCtx, zErr, -1);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_STRICT_SUBTYPE */
  p_ಹೊರಹೋಗು = pCtx->p_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಹೊರಹೋಗು->db->mutex) );
  p_ಹೊರಹೋಗು->eSubtype = eSubtype & 0xff;
  p_ಹೊರಹೋಗು->flags |= MEM_Subtype;
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z,
  ಪೂರ್ಣಾಂಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, 0);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, n, SQLITE_UTF8, xDel);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_text64(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z,
  sqlite3_uint64 n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *),
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ enc
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ){
    invoke_ಮೌಲ್ಯ_Destructor(z, xDel, 0);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಆದರೆ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ಆದರೆ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n &= ~(u64)1;
  }
  ಆದರೆ( n>0x7fffffff ){
    (ಶೂನ್ಯ)invoke_ಮೌಲ್ಯ_Destructor(z, xDel, pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, (ಪೂರ್ಣಾಂಕ)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->p_ಹೊರಹೋಗು);
  }
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_text16(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಂಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16NATIVE, xDel);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_text16be(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಂಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16BE, xDel);
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_text16le(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಂಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ *)
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z, n & ~(u64)1, SQLITE_UTF16LE, xDel);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(sqlite3_context *pCtx, sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ){
  Mem *p_ಹೊರಹೋಗು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p_ಮೌಲ್ಯ==0 ){
    sqlite3_ಫಲಿತಾಂಶ_null(pCtx);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  p_ಹೊರಹೋಗು = pCtx->p_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3VdbeMemCopy(p_ಹೊರಹೋಗು, p_ಮೌಲ್ಯ);
  sqlite3VdbeChangeEncoding(p_ಹೊರಹೋಗು, pCtx->enc);
  ಆದರೆ( sqlite3VdbeMemTooBig(p_ಹೊರಹೋಗು) ){
    sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
  }
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_zeroblob(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ n){
  sqlite3_ಫಲಿತಾಂಶ_zeroblob64(pCtx, n>0 ? n : 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_ಫಲಿತಾಂಶ_zeroblob64(sqlite3_context *pCtx, u64 n){
  Mem *p_ಹೊರಹೋಗು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  p_ಹೊರಹೋಗು = pCtx->p_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಹೊರಹೋಗು->db->mutex) );
  ಆದರೆ( n>(u64)p_ಹೊರಹೋಗು->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
    ಹಿಂತಿರುಗಿ SQLITE_TOOBIG;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
  sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(pCtx->p_ಹೊರಹೋಗು, (ಪೂರ್ಣಾಂಕ)n);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(pCtx->p_ಹೊರಹೋಗು, (ಪೂರ್ಣಾಂಕ)n);
#ಆದರೆ_ಅಂತ್ಯ
}
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ err_ಸಂಕೇತ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  pCtx->isError = err_ಸಂಕೇತ ? err_ಸಂಕೇತ : -1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pCtx->pVdbe ) pCtx->pVdbe->rcApp = err_ಸಂಕೇತ;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pCtx->p_ಹೊರಹೋಗು->flags & MEM_Null ){
    ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, sqlite3ErrStr(err_ಸಂಕೇತ), -1, SQLITE_UTF8,
                        SQLITE_STATIC);
  }
}

/* Cause the SQL function to raise an SQLITE_TOOBIG error. */
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_error_toobig(sqlite3_context *pCtx){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3_Vdbe_Mem_ಗಣ_Str(pCtx->p_ಹೊರಹೋಗು, "string or blob too big", -1,
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* Cause the SQL function to raise an SQLITE_NOMEM error. */
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_error_nomem(sqlite3_context *pCtx){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  sqlite3_Vdbe_Mem_ಗಣ_Null(pCtx->p_ಹೊರಹೋಗು);
  pCtx->isError = SQLITE_NOMEM_BKPT;
  sqlite3OomFault(pCtx->p_ಹೊರಹೋಗು->db);
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
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_str(sqlite3_context *pCtx, sqlite3_str *pStr, ಪೂರ್ಣಾಂಕ eOwn){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pStr==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  testcase( pStr==(sqlite3_str*)&sqlite3OomStr );
  ಆದರೆ( pStr->accError==0 ){
    ಆದರೆ( pStr->nChar==0 ){
      ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, "", 0, SQLITE_UTF8_ZT, SQLITE_STATIC);
      sqlite3_str_reset(pStr);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ = sqlite3_str_ಮೌಲ್ಯ(pStr);
      /* Only internal code has the ability to capture a pointer to
      ** an sqlite3_str object that uses static buffer.  And none of
      ** those internal use cases every invoke the sqlite3_result_str()
      ** interface on a static-buffer sqlite3_str.  Should this change
      ** in the future, the following assert() will let us know. */
      ಪ್ರತಿಜ್ಞಾ( isMalloced(pStr) );
      ಆದರೆ( eOwn==SQLITE_COPY ){
        ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z_ಪಠ್ಯ, pStr->nChar, 
                            SQLITE_UTF8, SQLITE_TRANSIENT);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಗಣ_ಫಲಿತಾಂಶ_Str_Or_Error(pCtx, z_ಪಠ್ಯ, pStr->nChar,
                            SQLITE_UTF8_ZT, SQLITE_DYNAMIC);
        sqlite3StrAccumInit(pStr, pStr->db, 0, 0, pStr->mxAlloc);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pStr->accError==SQLITE_NOMEM ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pStr->accError==SQLITE_TOOBIG );
    sqlite3_ಫಲಿತಾಂಶ_error_toobig(pCtx);
  }
  ಆದರೆ( eOwn==SQLITE_ಮುಕ್ತಾಯ ){
    sqlite3_str_free(pStr);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
/* Force the INT64 value currently stored as the result to be
** a MEM_IntReal value.  See the SQLITE_TESTCTRL_RESULT_INTREAL
** test-control.
*/
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_Int_Real(sqlite3_context *pCtx){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
  ಆದರೆ( pCtx->p_ಹೊರಹೋಗು->flags & MEM_Int ){
    pCtx->p_ಹೊರಹೋಗು->flags &= ~MEM_Int;
    pCtx->p_ಹೊರಹೋಗು->flags |= MEM_IntReal;
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** This function is called after a transaction has been committed. It
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ doWalCallbacks(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ಆದರೆ( pBt ){
      ಪೂರ್ಣಾಂಕ n_ನಮೂದು;
      sqlite3BtreeEnter(pBt);
      n_ನಮೂದು = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      ಆದರೆ( n_ನಮೂದು>0 && db->xWalCallback && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, n_ನಮೂದು);
      }
    }
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(db);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ಹೆಜ್ಜೆ_2(Vdbe *p){
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ(p);
  db = p->db;
  ಆದರೆ( p->eVdbeState!=VDBE_ಸತತ_STATE ){
    restart_ಹೆಜ್ಜೆ:
    ಆದರೆ( p->eVdbeState==VDBE_READY_STATE ){
      ಆದರೆ( p->expired ){
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        ಆದರೆ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
          /* If this statement was prepared using saved SQL and an
          ** error has occurred, then return the error code in p->rc to the
          ** caller. Set the error code in the database handle to the same
          ** value.
          */
          rc = sqlite3VdbeTransferError(p);
        }
        ಹೋಗು ಕೊನೆ_ದ_ಹೆಜ್ಜೆ;
      }

      /* If there are no other statements currently running, then
      ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
      ** from interrupting a statement that has not yet started.
      */
      ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ==0 ){
        AtomicStore(&db->u1.isInterrupted, 0);
      }

      ಪ್ರತಿಜ್ಞಾ( db->nVdbeWrite>0 || db->autoCommit==0
          || ((db->nDeferredCons + db->nDeferredImmCons)==0)
      );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
      ಆದರೆ( (db->mTrace & (SQLITE_TRACE_PROFILE|SQLITE_TRACE_XPROFILE))!=0
          && !db->init.busy && p->zSql ){
        sqlite3_Os_Current_ಸಮಯ_Int64(db->pVfs, &p->ಆರಂಭ_ಸಮಯ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( p->ಆರಂಭ_ಸಮಯ==0 );
      }
#ಆದರೆ_ಅಂತ್ಯ

      db->n_Vdbe_ಸಕ್ರಿಯ++;
      ಆದರೆ( p->readOnly==0 ) db->nVdbeWrite++;
      ಆದರೆ( p->bIsReader ) db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = VDBE_ಸತತ_STATE;
    }ಇಲ್ಲದಿದ್ದರೆ

    ಆದರೆ( ALWAYS(p->eVdbeState==VDBE_HALT_STATE) ){
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTORESET
      ಆದರೆ( (rc = p->rc&0xff)==SQLITE_BUSY || rc==SQLITE_LOCKED ){
        sqlite3_reset((sqlite3_stmt*)p);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
      }
#ಇಲ್ಲದಿದ್ದರೆ
      sqlite3_reset((sqlite3_stmt*)p);
#ಆದರೆ_ಅಂತ್ಯ
      ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_READY_STATE );
      ಹೋಗು restart_ಹೆಜ್ಜೆ;
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  p->rcApp = SQLITE_OK;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
  ಆದರೆ( p->explain ){
    rc = sqlite3VdbeList(p);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_EXPLAIN */
  {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
    ಪ್ರತಿಜ್ಞಾ( p->rc==SQLITE_OK );
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
    db->err_ಸಂಕೇತ = SQLITE_ಅಡ್ಡಸಾಲು;
    ಹಿಂತಿರುಗಿ SQLITE_ಅಡ್ಡಸಾಲು;
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
    /* If the statement completed successfully, invoke the profile callback */
    checkProfileCallback(db, p);
#ಆದರೆ_ಅಂತ್ಯ
    p->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು = 0;
    ಆದರೆ( rc==SQLITE_ಮುಗಿದ && db->autoCommit ){
      ಪ್ರತಿಜ್ಞಾ( p->rc==SQLITE_OK );
      p->rc = doWalCallbacks(db);
      ಆದರೆ( p->rc!=SQLITE_OK ){
        rc = SQLITE_ERROR;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_ಮುಗಿದ && (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ){
      /* If this statement was prepared using saved SQL and an
      ** error has occurred, then return the error code in p->rc to the
      ** caller. Set the error code in the database handle to the same value.
      */
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->err_ಸಂಕೇತ = rc;
  ಆದರೆ( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM_BKPT;
    ಆದರೆ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 ) rc = p->rc;
  }
ಕೊನೆ_ದ_ಹೆಜ್ಜೆ:
  /* There are only a limited number of result codes allowed from the
  ** statements prepared using the legacy sqlite3_prepare() interface */
  ಪ್ರತಿಜ್ಞಾ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)!=0
       || rc==SQLITE_ಅಡ್ಡಸಾಲು  || rc==SQLITE_ಮುಗಿದ   || rc==SQLITE_ERROR
       || (rc&0xff)==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  ಹಿಂತಿರುಗಿ (rc&db->err_ಮುಖವಾಡ);
}

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಹೆಜ್ಜೆ(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;      /* Result from sqlite3Step() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  ಪೂರ್ಣಾಂಕ cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  ಆದರೆ( vdbeSafetyNotNull(v) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  ಆಗ( (rc = sqlite3_ಹೆಜ್ಜೆ_2(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY ){
    ಪೂರ್ಣಾಂಕ savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    ಆದರೆ( rc!=SQLITE_OK ){
      /* This case occurs after failing to recompile an sql statement.
      ** The error message from the SQL compiler has already been loaded
      ** into the database handle. This block copies the error message
      ** from the database handle into the statement and sets the statement
      ** program counter to 0 to ensure that when the statement is
      ** finalized or reset the parser error message is available via
      ** sqlite3_errmsg() and sqlite3_errcode().
      */
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(db->pErr);
      sqlite3DbFree(db, v->z_Err_ಸಂದೇಶ);
      ಆದರೆ( !db->mallocFailed ){
        v->z_Err_ಸಂದೇಶ = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } ಇಲ್ಲದಿದ್ದರೆ {
        v->z_Err_ಸಂದೇಶ = 0;
        v->rc = rc = SQLITE_NOMEM_BKPT;
      }
      ನಿಲ್ಲಿಸು;
    }
    sqlite3_reset(pStmt);
    ಆದರೆ( savedPc>=0 ){
      /* Setting minWriteFileFormat to 254 is a signal to the OP_Init and
      ** OP_Trace opcodes to *not* perform SQLITE_TRACE_STMT because it has
      ** already been done once on a prior invocation that failed due to
      ** SQLITE_SCHEMA.   tag-20220401a  */
      v->minWriteFileFormat = 254;
    }
    ಪ್ರತಿಜ್ಞಾ( v->expired==0 );
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
ಶೂನ್ಯ *sqlite3_user_ದತ್ತಾಂಶ(sqlite3_context *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( p && p->pFunc );
  ಹಿಂತಿರುಗಿ p->pFunc->p_User_ದತ್ತಾಂಶ;
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( p && p->p_ಹೊರಹೋಗು );
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ p->p_ಹೊರಹೋಗು->db;
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
ಪೂರ್ಣಾಂಕ sqlite3_vtab_nochange(sqlite3_context *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( p );
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_nochange(p->p_ಹೊರಹೋಗು);
}

/*
** The destructor function for a ValueList object.  This needs to be
** a separate function, unknowable to the application, to ensure that
** calls to sqlite3_vtab_in_first()/sqlite3_vtab_in_next() that are not
** preceded by activation of IN processing via sqlite3_vtab_int() do not
** try to access a fake ValueList object inserted by a hostile extension.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಮೌಲ್ಯ_List_Free(ಶೂನ್ಯ *p_ಗಮ್ಯ_Delete){
  sqlite3_free(p_ಗಮ್ಯ_Delete);
}

/*
** Implementation of sqlite3_vtab_in_first() (if bNext==0) and
** sqlite3_vtab_in_next() (if bNext!=0).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ_From_ಮೌಲ್ಯ_List(
  sqlite3_ಮೌಲ್ಯ *pVal,        /* Pointer to the ValueList object */
  sqlite3_ಮೌಲ್ಯ **pp_ಹೊರಹೋಗು,      /* Store the next value from the list here */
  ಪೂರ್ಣಾಂಕ b_ಮುಂದಿನ                   /* 1 for _next(). 0 for _first() */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಮೌಲ್ಯ_List *pRhs;

  *pp_ಹೊರಹೋಗು = 0;
  ಆದರೆ( pVal==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  ಆದರೆ( (pVal->flags & MEM_Dyn)==0 || pVal->xDel!=sqlite3_Vdbe_ಮೌಲ್ಯ_List_Free ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( (pVal->flags&(MEM_Type_ಮುಖವಾಡ|MEM_ಪದ|MEM_Subtype)) ==
                 (MEM_Null|MEM_ಪದ|MEM_Subtype) );
    ಪ್ರತಿಜ್ಞಾ( pVal->eSubtype=='p' );
    ಪ್ರತಿಜ್ಞಾ( pVal->u.zPType!=0 && strcmp(pVal->u.zPType,"ValueList")==0 );
    pRhs = (ಮೌಲ್ಯ_List*)pVal->z;
  }
  ಆದರೆ( b_ಮುಂದಿನ ){
    rc = sqlite3_Btree_ಮುಂದಿನ(pRhs->pCsr, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ dummy = 0;
    rc = sqlite3_Btree_ಮೊದಲ(pRhs->pCsr, &dummy);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || sqlite3BtreeEof(pRhs->pCsr) );
    ಆದರೆ( sqlite3BtreeEof(pRhs->pCsr) ) rc = SQLITE_ಮುಗಿದ;
  }
  ಆದರೆ( rc==SQLITE_OK ){
    u32 sz;       /* Size of current row in bytes */
    Mem sMem;     /* Raw content of current row */
    memset(&sMem, 0, ಗಾತ್ರ(sMem));
    sz = sqlite3_Btree_Payload_ಗಾತ್ರ(pRhs->pCsr);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pRhs->pCsr,sz,&sMem);
    ಆದರೆ( rc==SQLITE_OK ){
      u8 *zBuf = (u8*)sMem.z;
      u32 iSerial;
      sqlite3_ಮೌಲ್ಯ *p_ಹೊರಹೋಗು = pRhs->p_ಹೊರಹೋಗು;
      ಪೂರ್ಣಾಂಕ iOff = 1 + getVarint32(&zBuf[1], iSerial);
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, p_ಹೊರಹೋಗು);
      p_ಹೊರಹೋಗು->enc = ENC(p_ಹೊರಹೋಗು->db);
      ಆದರೆ( (p_ಹೊರಹೋಗು->flags & MEM_Ephem)!=0 && sqlite3VdbeMemMakeWriteable(p_ಹೊರಹೋಗು) ){
        rc = SQLITE_NOMEM;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *pp_ಹೊರಹೋಗು = p_ಹೊರಹೋಗು;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Set the iterator value pVal to point to the first value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ಪೂರ್ಣಾಂಕ sqlite3_vtab_ಒಳ_ಮೊದಲ(sqlite3_ಮೌಲ್ಯ *pVal, sqlite3_ಮೌಲ್ಯ **pp_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ_From_ಮೌಲ್ಯ_List(pVal, pp_ಹೊರಹೋಗು, 0);
}

/*
** Set the iterator value pVal to point to the next value in the set.
** Set (*ppOut) to point to this value before returning.
*/
ಪೂರ್ಣಾಂಕ sqlite3_vtab_ಒಳ_ಮುಂದಿನ(sqlite3_ಮೌಲ್ಯ *pVal, sqlite3_ಮೌಲ್ಯ **pp_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ_From_ಮೌಲ್ಯ_List(pVal, pp_ಹೊರಹೋಗು, 1);
}

/*
** Return the current time for a statement.  If the current time
** is requested more than once within the same run of a single prepared
** statement, the exact same time is returned for each invocation regardless
** of the amount of time that elapses between invocations.  In other words,
** the time returned is always the time of the first call.
*/
sqlite3_int64 sqlite3_Stmt_Current_ಸಮಯ(sqlite3_context *p){
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_STAT4
  sqlite3_int64 *pi_ಸಮಯ = &p->pVdbe->i_Current_ಸಮಯ;
  ಪ್ರತಿಜ್ಞಾ( p->pVdbe!=0 );
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_int64 i_ಸಮಯ = 0;
  sqlite3_int64 *pi_ಸಮಯ = p->pVdbe!=0 ? &p->pVdbe->i_Current_ಸಮಯ : &i_ಸಮಯ;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( *pi_ಸಮಯ==0 ){
    rc = sqlite3_Os_Current_ಸಮಯ_Int64(p->p_ಹೊರಹೋಗು->db->pVfs, pi_ಸಮಯ);
    ಆದರೆ( rc ) *pi_ಸಮಯ = 0;
  }
  ಹಿಂತಿರುಗಿ *pi_ಸಮಯ;
}

/*
** Create a new aggregate context for p and return a pointer to
** its pMem->z element.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *createAggContext(sqlite3_context *p, ಪೂರ್ಣಾಂಕ nByte){
  Mem *pMem = p->pMem;
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Agg)==0 );
  ಆದರೆ( nByte<=0 ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
    pMem->z = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = MEM_Agg;
    pMem->u.pDef = p->pFunc;
    ಆದರೆ( pMem->z ){
      memset(pMem->z, 0, nByte);
    }
  }
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)pMem->z;
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
ಶೂನ್ಯ *sqlite3_aggregate_context(sqlite3_context *p, ಪೂರ್ಣಾಂಕ nByte){
  ಪ್ರತಿಜ್ಞಾ( p && p->pFunc && p->pFunc->xFinalize );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->p_ಹೊರಹೋಗು->db->mutex) );
  testcase( nByte<0 );
  ಆದರೆ( (p->pMem->flags & MEM_Agg)==0 ){
    ಹಿಂತಿರುಗಿ createAggContext(p, nByte);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)p->pMem->z;
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
ಶೂನ್ಯ *sqlite3_get_auxdata(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ iArg){
  Aux_ದತ್ತಾಂಶ *p_Aux_ದತ್ತಾಂಶ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
#ಆದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( pCtx->pVdbe==0 ) ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( pCtx->pVdbe!=0 );
#ಆದರೆ_ಅಂತ್ಯ
  ಗಾಗಿ(p_Aux_ದತ್ತಾಂಶ=pCtx->pVdbe->p_Aux_ದತ್ತಾಂಶ; p_Aux_ದತ್ತಾಂಶ; p_Aux_ದತ್ತಾಂಶ=p_Aux_ದತ್ತಾಂಶ->p_ಮುಂದಿನ_Aux){
    ಆದರೆ(  p_Aux_ದತ್ತಾಂಶ->iAuxArg==iArg && (p_Aux_ದತ್ತಾಂಶ->iAuxOp==pCtx->iOp || iArg<0) ){
      ಹಿಂತಿರುಗಿ p_Aux_ದತ್ತಾಂಶ->pAux;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಶೂನ್ಯ sqlite3_ಗಣ_auxdata(
  sqlite3_context *pCtx,
  ಪೂರ್ಣಾಂಕ iArg,
  ಶೂನ್ಯ *pAux,
  ಶೂನ್ಯ (*xDelete)(ಶೂನ್ಯ*)
){
  Aux_ದತ್ತಾಂಶ *p_Aux_ದತ್ತಾಂಶ;
  Vdbe *pVdbe;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCtx==0 ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  pVdbe= pCtx->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCtx->p_ಹೊರಹೋಗು->db->mutex) );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( pVdbe==0 ) ಹೋಗು failed;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( pVdbe!=0 );
#ಆದರೆ_ಅಂತ್ಯ

  ಗಾಗಿ(p_Aux_ದತ್ತಾಂಶ=pVdbe->p_Aux_ದತ್ತಾಂಶ; p_Aux_ದತ್ತಾಂಶ; p_Aux_ದತ್ತಾಂಶ=p_Aux_ದತ್ತಾಂಶ->p_ಮುಂದಿನ_Aux){
    ಆದರೆ( p_Aux_ದತ್ತಾಂಶ->iAuxArg==iArg && (p_Aux_ದತ್ತಾಂಶ->iAuxOp==pCtx->iOp || iArg<0) ){
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( p_Aux_ದತ್ತಾಂಶ==0 ){
    p_Aux_ದತ್ತಾಂಶ = sqlite3DbMallocZero(pVdbe->db, ಗಾತ್ರ(Aux_ದತ್ತಾಂಶ));
    ಆದರೆ( !p_Aux_ದತ್ತಾಂಶ ) ಹೋಗು failed;
    p_Aux_ದತ್ತಾಂಶ->iAuxOp = pCtx->iOp;
    p_Aux_ದತ್ತಾಂಶ->iAuxArg = iArg;
    p_Aux_ದತ್ತಾಂಶ->p_ಮುಂದಿನ_Aux = pVdbe->p_Aux_ದತ್ತಾಂಶ;
    pVdbe->p_Aux_ದತ್ತಾಂಶ = p_Aux_ದತ್ತಾಂಶ;
    ಆದರೆ( pCtx->isError==0 ) pCtx->isError = -1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_Aux_ದತ್ತಾಂಶ->xDeleteAux ){
    p_Aux_ದತ್ತಾಂಶ->xDeleteAux(p_Aux_ದತ್ತಾಂಶ->pAux);
  }

  p_Aux_ದತ್ತಾಂಶ->pAux = pAux;
  p_Aux_ದತ್ತಾಂಶ->xDeleteAux = xDelete;
  ಹಿಂತಿರುಗಿ;

failed:
  ಆದರೆ( xDelete ){
    xDelete(pAux);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of an aggregate has been
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
ಪೂರ್ಣಾಂಕ sqlite3_aggregate_ಎಣಿಕೆ(sqlite3_context *p){
  ಪ್ರತಿಜ್ಞಾ( p && p->pMem && p->pFunc && p->pFunc->xFinalize );
  ಹಿಂತಿರುಗಿ p->pMem->n;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return the number of columns in the result set for the statement pStmt.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ಆದರೆ( pVm==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ pVm->n_Res_ಸ್ತಂಭ;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ದತ್ತಾಂಶ_ಎಣಿಕೆ(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  ಆದರೆ( pVm==0 || pVm->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ pVm->n_Res_ಸ್ತಂಭ;
}

/*
** Return a pointer to static memory containing an SQL NULL value.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Mem *ಸ್ತಂಭ_Null_ಮೌಲ್ಯ(ಶೂನ್ಯ){
  /* Even though the Mem structure contains an element
  ** of type i64, on certain architectures (x86) with certain compiler
  ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
  ** instead of an 8-byte one. This all works fine, except that when
  ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
  ** that a Mem structure is located on an 8-byte boundary. To prevent
  ** these assert()s from failing, when building with SQLITE_DEBUG defined
  ** using gcc, we force nullMem to be 8-byte aligned using the magical
  ** __attribute__((aligned(8))) macro.  */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Mem nullMem
#ಆದರೆ defined(SQLITE_DEBUG) && defined(__GNUC__)
    __attribute__((aligned(8)))
#ಆದರೆ_ಅಂತ್ಯ
    = {
        /* .u          = */ {0},
        /* .z          = */ (ಅಕ್ಷರ*)0,
        /* .n          = */ (ಪೂರ್ಣಾಂಕ)0,
        /* .flags      = */ (u16)MEM_Null,
        /* .enc        = */ (u8)0,
        /* .eSubtype   = */ (u8)0,
        /* .db         = */ (sqlite3*)0,
        /* .szMalloc   = */ (ಪೂರ್ಣಾಂಕ)0,
        /* .uTemp      = */ (u32)0,
        /* .zMalloc    = */ (ಅಕ್ಷರ*)0,
        /* .xDel       = */ (ಶೂನ್ಯ(*)(ಶೂನ್ಯ*))0,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
        /* .pScopyFrom = */ (Mem*)0,
        /* .mScopyFlags= */ 0,
        /* .bScopy     = */ 0,
#ಆದರೆ_ಅಂತ್ಯ
      };
  ಹಿಂತಿರುಗಿ &nullMem;
}

/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
ಸ್ಥಿರ Mem *ಸ್ತಂಭ_Mem(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  Vdbe *pVm;
  Mem *p_ಹೊರಹೋಗು;

  pVm = (Vdbe *)pStmt;
  ಆದರೆ( pVm==0 ) ಹಿಂತಿರುಗಿ (Mem*)ಸ್ತಂಭ_Null_ಮೌಲ್ಯ();
  ಪ್ರತಿಜ್ಞಾ( pVm->db );
  sqlite3_mutex_enter(pVm->db->mutex);
  ಆದರೆ( pVm->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು!=0 && i<pVm->n_Res_ಸ್ತಂಭ && i>=0 ){
    p_ಹೊರಹೋಗು = &pVm->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು[i];
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3Error(pVm->db, SQLITE_RANGE);
    p_ಹೊರಹೋಗು = (Mem*)ಸ್ತಂಭ_Null_ಮೌಲ್ಯ();
  }
  ಹಿಂತಿರುಗಿ p_ಹೊರಹೋಗು;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಸ್ತಂಭ_Malloc_Failure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  ಆದರೆ( p ){
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
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_blob(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *val;
  val = sqlite3_ಮೌಲ್ಯ_blob( ಸ್ತಂಭ_Mem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob()
  ** expression.
  */
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಪೂರ್ಣಾಂಕ val = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_bytes16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಪೂರ್ಣಾಂಕ val = sqlite3_ಮೌಲ್ಯ_bytes16( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
ದ್ವಿಗುಣ sqlite3_ಸ್ತಂಭ_double(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ದ್ವಿಗುಣ val = sqlite3_ಮೌಲ್ಯ_double( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_int(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಪೂರ್ಣಾಂಕ val = sqlite3_ಮೌಲ್ಯ_int( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
sqlite_int64 sqlite3_ಸ್ತಂಭ_int64(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  sqlite_int64 val = sqlite3_ಮೌಲ್ಯ_int64( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *val = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
sqlite3_ಮೌಲ್ಯ *sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  Mem *p_ಹೊರಹೋಗು = ಸ್ತಂಭ_Mem(pStmt, i);
  ಆದರೆ( p_ಹೊರಹೋಗು->flags&MEM_Static ){
    p_ಹೊರಹೋಗು->flags &= ~MEM_Static;
    p_ಹೊರಹೋಗು->flags |= MEM_Ephem;
  }
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ (sqlite3_ಮೌಲ್ಯ *)p_ಹೊರಹೋಗು;
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_text16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *val = sqlite3_ಮೌಲ್ಯ_text16( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ val;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_type(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಪೂರ್ಣಾಂಕ iType = sqlite3_ಮೌಲ್ಯ_type( ಸ್ತಂಭ_Mem(pStmt,i) );
  ಸ್ತಂಭ_Malloc_Failure(pStmt);
  ಹಿಂತಿರುಗಿ iType;
}

/*
** Column names appropriate for EXPLAIN or EXPLAIN QUERY PLAN.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ az_Explain_ಲಂಬಸಾಲು_Names8[] = {
   "addr", "opcode", "p1", "p2", "p3", "p4", "p5", "comment",  /* EXPLAIN */
   "id", "parent", "notused", "detail"                         /* EQP */
};
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u16 az_Explain_ಲಂಬಸಾಲು_Names16data[] = {
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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 i_Explain_ಲಂಬಸಾಲು_Names16[] = {
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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ತಂಭ_Name(
  sqlite3_stmt *pStmt,     /* The statement */
  ಪೂರ್ಣಾಂಕ N,                   /* Which column to get the name for */
  ಪೂರ್ಣಾಂಕ useUtf16,            /* True to return the name as UTF16 */
  ಪೂರ್ಣಾಂಕ useType              /* What type of name */
){
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ret;
  Vdbe *p;
  ಪೂರ್ಣಾಂಕ n;
  sqlite3 *db;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pStmt==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( N<0 ) ಹಿಂತಿರುಗಿ 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  sqlite3_mutex_enter(db->mutex);

  ಆದರೆ( p->explain ){
    ಆದರೆ( useType>0 ) ಹೋಗು ಸ್ತಂಭ_Name_ಕೊನೆ;
    n = p->explain==1 ? 8 : 4;
    ಆದರೆ( N>=n ) ಹೋಗು ಸ್ತಂಭ_Name_ಕೊನೆ;
    ಆದರೆ( useUtf16 ){
      ಪೂರ್ಣಾಂಕ i = i_Explain_ಲಂಬಸಾಲು_Names16[N + 8*p->explain - 8];
      ret = (ಶೂನ್ಯ*)&az_Explain_ಲಂಬಸಾಲು_Names16data[i];
    }ಇಲ್ಲದಿದ್ದರೆ{
      ret = (ಶೂನ್ಯ*)az_Explain_ಲಂಬಸಾಲು_Names8[N + 8*p->explain - 8];
    }
    ಹೋಗು ಸ್ತಂಭ_Name_ಕೊನೆ;
  }
  n = p->n_Res_ಸ್ತಂಭ;
  ಆದರೆ( N<n ){
    u8 ಪೂರ್ವ_malloc_Failed = db->mallocFailed;
    N += useType*n;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
    ಆದರೆ( useUtf16 ){
      ret = sqlite3_ಮೌಲ್ಯ_text16((sqlite3_ಮೌಲ್ಯ*)&p->a_ಲಂಬಸಾಲು_Name[N]);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ret = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ((sqlite3_ಮೌಲ್ಯ*)&p->a_ಲಂಬಸಾಲು_Name[N]);
    }
    /* A malloc may have failed inside of the _text() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || db->mallocFailed==1 );
    ಆದರೆ( db->mallocFailed > ಪೂರ್ವ_malloc_Failed ){
      sqlite3OomClear(db);
      ret = 0;
    }
  }
ಸ್ತಂಭ_Name_ಕೊನೆ:
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 0, COLNAME_NAME);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 1, COLNAME_NAME);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#ಆದರೆ defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_ಸ್ತಂಭ_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_ಸ್ತಂಭ_METADATA"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_decltype(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 0, COLNAME_DECLTYPE);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_decltype16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 1, COLNAME_DECLTYPE);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DECLTYPE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_database_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 0, COLNAME_DATABASE);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_database_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 1, COLNAME_DATABASE);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 0, COLNAME_ಕೋಷ್ಟಕ);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 1, COLNAME_ಕೋಷ್ಟಕ);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unambiguous reference to a database column.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_origin_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 0, COLNAME_ಸ್ತಂಭ);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಸ್ತಂಭ_origin_name16(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ N){
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_Name(pStmt, N, 1, COLNAME_ಸ್ತಂಭ);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_COLUMN_METADATA */


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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeUnbind(Vdbe *p, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i){
  Mem *pVar;
  ಆದರೆ( vdbeSafetyNotNull(p) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ಆದರೆ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  ಆದರೆ( i>=(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ಹಿಂತಿರುಗಿ SQLITE_RANGE;
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
  ಆದರೆ( p->expmask!=0 && (p->expmask & (i>=31 ? 0x80000000 : (u32)1<<i))!=0 ){
    /* If the p->smimask bit is also set, then only expire the statement if
    ** the value being clobbered was integer value 0 or 1. */
    ಆದರೆ( (p->smimask==0) 
     || (p->smimask & (i>=31 ? 0x80000000 : (u32)1<<i))==0 
     || ((pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1)
    ){
      p->expired = 1;
    }
  }

  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  p->db->err_ಸಂಕೇತ = SQLITE_OK;

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bind_ಪಠ್ಯ_2(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  ಪೂರ್ಣಾಂಕ i,                 /* Index of the parameter to bind */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z_ದತ್ತಾಂಶ,     /* Pointer to the data to be bound */
  i64 n_ದತ್ತಾಂಶ,             /* Number of bytes of data to be bound */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  ಪೂರ್ಣಾಂಕ rc;

  rc = vdbeUnbind(p, (u32)(i-1));
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    ಆದರೆ( z_ದತ್ತಾಂಶ!=0 ){
      pVar = &p->aVar[i-1];
      ಆದರೆ( encoding==SQLITE_UTF8 ){
        rc = sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(pVar, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, xDel);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( encoding==SQLITE_UTF8_ZT ){
        /* It is usually consider improper to assert() on an input.
        ** However, the following assert() is checking for inputs
        ** that are documented to result in undefined behavior. */
        ಪ್ರತಿಜ್ಞಾ( z_ದತ್ತಾಂಶ==0
             || n_ದತ್ತಾಂಶ<0 
             || n_ದತ್ತಾಂಶ>pVar->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]
             || ((u8*)z_ದತ್ತಾಂಶ)[n_ದತ್ತಾಂಶ]==0
        );
        rc = sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(pVar, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, xDel);
        pVar->flags |= MEM_ಪದ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = sqlite3_Vdbe_Mem_ಗಣ_Str(pVar, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, encoding, xDel);
        ಆದರೆ( encoding==0 ) pVar->enc = ENC(p->db);
      }
      ಆದರೆ( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      ಆದರೆ( rc ){
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((ಶೂನ್ಯ*)z_ದತ್ತಾಂಶ);
  }
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
ಪೂರ್ಣಾಂಕ sqlite3_bind_blob(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ i,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( n_ದತ್ತಾಂಶ<0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ bind_ಪಠ್ಯ_2(pStmt, i, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, xDel, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_blob64(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ i,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z_ದತ್ತಾಂಶ,
  sqlite3_uint64 n_ದತ್ತಾಂಶ,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಹಿಂತಿರುಗಿ bind_ಪಠ್ಯ_2(pStmt, i, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, xDel, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_double(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i, ದ್ವಿಗುಣ r_ಮೌಲ್ಯ){
  ಪೂರ್ಣಾಂಕ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_ಗಣ_Double(&p->aVar[i-1], r_ಮೌಲ್ಯ);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_int(sqlite3_stmt *p, ಪೂರ್ಣಾಂಕ i, ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ){
  ಹಿಂತಿರುಗಿ sqlite3_bind_int64(p, i, (i64)i_ಮೌಲ್ಯ);
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_int64(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i, sqlite_int64 i_ಮೌಲ್ಯ){
  Mem *pVar;
  Vdbe *p = (Vdbe*)pStmt;
  ಆದರೆ( vdbeSafetyNotNull(p) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  ಆದರೆ( p->eVdbeState!=VDBE_READY_STATE ){
    sqlite3Error(p->db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE,
        "bind on a busy prepared statement: [%s]", p->zSql);
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  ಆದರೆ( i<=0 || (--i)>=p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    ಹಿಂತಿರುಗಿ SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  ಆದರೆ( p->expmask!=0 ){
    u32 expire_ಮುಖವಾಡ = i>=32 ? 0x80000000 : (u32)1 << i;
    ಪ್ರತಿಜ್ಞಾ( (p->expmask & p->smimask)==p->smimask );
    ಆದರೆ( (p->expmask & expire_ಮುಖವಾಡ)!=0 ){
      ಆದರೆ( (p->smimask & expire_ಮುಖವಾಡ)!=0 ){
        /* If the smimask bit is set, only expire the prepared statement
        ** if the value is changing to or from (0,1) and something else */
        ಪೂರ್ಣಾಂಕ sm1 = (pVar->flags & MEM_Int)!=0 && pVar->u.i>=0 && pVar->u.i<=1;
        ಪೂರ್ಣಾಂಕ sm2 = i_ಮೌಲ್ಯ>=0 && i_ಮೌಲ್ಯ<=1;
        ಆದರೆ( sm1!=sm2 ){
          p->expired = 1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pVar->flags & MEM_Int)==0 || i_ಮೌಲ್ಯ!=pVar->u.i ){
        /* Always expire if the smimask bit is clear */
        p->expired = 1;
      }
    }
  }
  sqlite3_Vdbe_Mem_ಗಣ_Int64(pVar, i_ಮೌಲ್ಯ);
  sqlite3_mutex_leave(p->db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_null(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  ಪೂರ್ಣಾಂಕ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_ಸೂಚಕ(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ i,
  ಶೂನ್ಯ *pPtr,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPTtype,
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)
){
  ಪೂರ್ಣಾಂಕ rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
    sqlite3_Vdbe_Mem_ಗಣ_ಸೂಚಕ(&p->aVar[i-1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xDestructor ){
    xDestructor(pPtr);
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_ಪಠ್ಯ(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ i,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
  ಹಿಂತಿರುಗಿ bind_ಪಠ್ಯ_2(pStmt, i, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, xDel, SQLITE_UTF8);
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_text64(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ i,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ,
  sqlite3_uint64 n_ದತ್ತಾಂಶ,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*),
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ enc
){
  ಪ್ರತಿಜ್ಞಾ( xDel!=SQLITE_DYNAMIC );
  ಆದರೆ( enc!=SQLITE_UTF8 && enc!=SQLITE_UTF8_ZT ){
    ಆದರೆ( enc==SQLITE_UTF16 ) enc = SQLITE_UTF16NATIVE;
    n_ದತ್ತಾಂಶ &= ~(u64)1;
  }
  ಹಿಂತಿರುಗಿ bind_ಪಠ್ಯ_2(pStmt, i, z_ದತ್ತಾಂಶ, n_ದತ್ತಾಂಶ, xDel, enc);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಪೂರ್ಣಾಂಕ sqlite3_bind_text16(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ i,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ n,
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)
){
  ಹಿಂತಿರುಗಿ bind_ಪಠ್ಯ_2(pStmt, i, z_ದತ್ತಾಂಶ, n & ~(u64)1, xDel, SQLITE_UTF16NATIVE);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
ಪೂರ್ಣಾಂಕ sqlite3_bind_ಮೌಲ್ಯ(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i, ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ){
  ಪೂರ್ಣಾಂಕ rc;
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type((sqlite3_ಮೌಲ್ಯ*)p_ಮೌಲ್ಯ) ){
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, p_ಮೌಲ್ಯ->u.i);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
      ಪ್ರತಿಜ್ಞಾ( p_ಮೌಲ್ಯ->flags & (MEM_Real|MEM_IntReal) );
      rc = sqlite3_bind_double(pStmt, i,
          (p_ಮೌಲ್ಯ->flags & MEM_Real) ? p_ಮೌಲ್ಯ->u.r : (ದ್ವಿಗುಣ)p_ಮೌಲ್ಯ->u.i
      );
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_BLOB: {
      ಆದರೆ( p_ಮೌಲ್ಯ->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, p_ಮೌಲ್ಯ->u.nZero);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = sqlite3_bind_blob(pStmt, i, p_ಮೌಲ್ಯ->z, p_ಮೌಲ್ಯ->n,SQLITE_TRANSIENT);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ: {
      rc = bind_ಪಠ್ಯ_2(pStmt,i,  p_ಮೌಲ್ಯ->z, p_ಮೌಲ್ಯ->n, SQLITE_TRANSIENT,
                              p_ಮೌಲ್ಯ->enc);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      rc = sqlite3_bind_null(pStmt, i);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i, ಪೂರ್ಣಾಂಕ n){
  ಪೂರ್ಣಾಂಕ rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i-1));
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( p!=0 && p->aVar!=0 && i>0 && i<=p->nVar ); /* tag-20240917-01 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
    sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(&p->aVar[i-1], n);
#ಇಲ್ಲದಿದ್ದರೆ
    rc = sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(&p->aVar[i-1], n);
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i, sqlite3_uint64 n){
  ಪೂರ್ಣಾಂಕ rc;
  Vdbe *p = (Vdbe *)pStmt;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(p->db->mutex);
  ಆದರೆ( n>(u64)p->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    rc = SQLITE_TOOBIG;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( (n & 0x7FFFFFFF)==n );
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite. 
*/
ಪೂರ್ಣಾಂಕ sqlite3_bind_parameter_ಎಣಿಕೆ(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ಹಿಂತಿರುಗಿ p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ i){
  Vdbe *p = (Vdbe*)pStmt;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ sqlite3_VList_ಅಂಕೆ_ಗಮ್ಯ_Name(p->pVList, i);
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Parameter_ಸೂಚ್ಯಂಕ(Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ nName){
  ಆದರೆ( p==0 || zName==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ sqlite3_VList_Name_ಗಮ್ಯ_ಅಂಕೆ(p->pVList, zName, nName);
}
ಪೂರ್ಣಾಂಕ sqlite3_bind_parameter_ಸೂಚ್ಯಂಕ(sqlite3_stmt *pStmt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_Parameter_ಸೂಚ್ಯಂಕ((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
ಪೂರ್ಣಾಂಕ sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_ಗಮ್ಯ_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_ಗಮ್ಯ = (Vdbe*)p_ಗಮ್ಯ_Stmt;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ->db==pFrom->db );
  ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ->nVar==pFrom->nVar );
  sqlite3_mutex_enter(p_ಗಮ್ಯ->db->mutex);
  ಗಾಗಿ(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&p_ಗಮ್ಯ->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(p_ಗಮ್ಯ->db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
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
ಪೂರ್ಣಾಂಕ sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *p_ಗಮ್ಯ_Stmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *p_ಗಮ್ಯ = (Vdbe*)p_ಗಮ್ಯ_Stmt;
  ಆದರೆ( pFrom->nVar!=p_ಗಮ್ಯ->nVar ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  ಪ್ರತಿಜ್ಞಾ( (p_ಗಮ್ಯ->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || p_ಗಮ್ಯ->expmask==0 );
  ಆದರೆ( p_ಗಮ್ಯ->expmask ){
    p_ಗಮ್ಯ->expired = 1;
  }
  ಪ್ರತಿಜ್ಞಾ( (pFrom->prepFlags & SQLITE_PREPARE_SAVESQL)!=0 || pFrom->expmask==0 );
  ಆದರೆ( pFrom->expmask ){
    pFrom->expired = 1;
  }
  ಹಿಂತಿರುಗಿ sqlite3TransferBindings(pFromStmt, p_ಗಮ್ಯ_Stmt);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  ಹಿಂತಿರುಗಿ pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  ಹಿಂತಿರುಗಿ pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return 1 if the statement is an EXPLAIN and return 2 if the
** statement is an EXPLAIN QUERY PLAN
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_isexplain(sqlite3_stmt *pStmt){
  ಹಿಂತಿರುಗಿ pStmt ? ((Vdbe*)pStmt)->explain : 0;
}

/*
** Set the explain mode for a statement.
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_explain(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ){
  Vdbe *v = (Vdbe*)pStmt;
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pStmt==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(v->db->mutex);
  ಆದರೆ( ((ಪೂರ್ಣಾಂಕ)v->explain)==e_ಬಹುಲಕ ){
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( e_ಬಹುಲಕ<0 || e_ಬಹುಲಕ>2 ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (v->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v->eVdbeState!=VDBE_READY_STATE ){
    rc = SQLITE_BUSY;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v->nMem>=10 && (e_ಬಹುಲಕ!=2 || v->haveEqpOps) ){
    /* No reprepare necessary */
    v->explain = e_ಬಹುಲಕ;
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ saved_ಬಹುಲಕ = v->explain;
    v->explain = e_ಬಹುಲಕ;
    rc = sqlite3Reprepare(v);
    ಆದರೆ( rc ){
      v->explain = saved_ಬಹುಲಕ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      v->haveEqpOps = e_ಬಹುಲಕ==2;
    }
  }
  ಆದರೆ( v->explain ){
    v->n_Res_ಸ್ತಂಭ = 12 - 4*v->explain;
  }ಇಲ್ಲದಿದ್ದರೆ{
    v->n_Res_ಸ್ತಂಭ = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return true if the prepared statement is in need of being reset.
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_busy(sqlite3_stmt *pStmt){
  Vdbe *v = (Vdbe*)pStmt;
  ಹಿಂತಿರುಗಿ v!=0 && v->eVdbeState==VDBE_ಸತತ_STATE;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
sqlite3_stmt *sqlite3_ಮುಂದಿನ_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *p_ಮುಂದಿನ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(pDb) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(pDb->mutex);
  ಆದರೆ( pStmt==0 ){
    p_ಮುಂದಿನ = (sqlite3_stmt*)pDb->pVdbe;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಮುಂದಿನ = (sqlite3_stmt*)((Vdbe*)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  ಹಿಂತಿರುಗಿ p_ಮುಂದಿನ;
}

/*
** Return the value of a status counter for a prepared statement
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_ಸ್ಥಿತಿ(sqlite3_stmt *pStmt, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  u32 v;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !pStmt
   || (op!=SQLITE_STMTSTATUS_MEMUSED && (op<0||op>=Array_ಗಾತ್ರ(pVdbe->aCounter)))
  ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( op==SQLITE_STMTSTATUS_MEMUSED ){
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pn_ಬೈಟುಗಳು_Freed = (ಪೂರ್ಣಾಂಕ*)&v;
    ಪ್ರತಿಜ್ಞಾ( db->lookaside.p_ಕೊನೆ==db->lookaside.p_True_ಕೊನೆ );
    db->lookaside.p_ಕೊನೆ = db->lookaside.p_ಆರಂಭ;
    sqlite3VdbeDelete(pVdbe);
    db->pn_ಬೈಟುಗಳು_Freed = 0;
    db->lookaside.p_ಕೊನೆ = db->lookaside.p_True_ಕೊನೆ;
    sqlite3_mutex_leave(db->mutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    v = pVdbe->aCounter[op];
    ಆದರೆ( resetFlag ) pVdbe->aCounter[op] = 0;
  }
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)v;
}

/*
** Return the SQL associated with a prepared statement
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ಹಿಂತಿರುಗಿ p ? p->zSql : 0;
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
ಅಕ್ಷರ *sqlite3_expanded_sql(sqlite3_stmt *pStmt){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_TRACE
  ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
  ಅಕ್ಷರ *z = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = sqlite3_sql(pStmt);
  ಆದರೆ( zSql ){
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಹಿಂತಿರುಗಿ z;
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
/*
** Return the normalized SQL associated with a prepared statement.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_normalized_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->zNormSql==0 && ALWAYS(p->zSql!=0) ){
    sqlite3_mutex_enter(p->db->mutex);
    p->zNormSql = sqlite3Normalize(p, p->zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  ಹಿಂತಿರುಗಿ p->zNormSql;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_NORMALIZE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Allocate and populate an UnpackedRecord structure based on the serialized
** record in nKey/pKey. Return a pointer to the new UnpackedRecord structure
** if successful, or a NULL pointer if an OOM error is encountered.
*/
ಸ್ಥಿರ Unpacked_ದಾಖಲೆ *vdbe_Unpack_ದಾಖಲೆ(
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಕೀಲಿ
){
  Unpacked_ದಾಖಲೆ *pRet;           /* Return value */

  pRet = sqlite3_Vdbe_Alloc_Unpacked_ದಾಖಲೆ(p_ಕೀಲಿ_Info);
  ಆದರೆ( pRet ){
    memset(pRet->aMem, 0, ಗಾತ್ರ(Mem)*(p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ+1));
    sqlite3_Vdbe_ದಾಖಲೆ_Unpack(n_ಕೀಲಿ, p_ಕೀಲಿ, pRet);
  }
  ಹಿಂತಿರುಗಿ pRet;
}

/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or deleted.
*/
ಪೂರ್ಣಾಂಕ sqlite3_preupdate_old(sqlite3 *db, ಪೂರ್ಣಾಂಕ iIdx, sqlite3_ಮೌಲ್ಯ **pp_ಮೌಲ್ಯ){
  PreUpdate *p;
  Mem *pMem;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ iStore = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( db==0 || pp_ಮೌಲ್ಯ==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  p = db->pPreUpdate;
  /* Test that this call is being made from within an SQLITE_DELETE or
  ** SQLITE_UPDATE pre-update callback, and that iIdx is within range. */
  ಆದರೆ( !p || p->op==SQLITE_ಸೇರಿಸು ){
    rc = SQLITE_MISUSE_BKPT;
    ಹೋಗು preupdate_old_ಹೊರಹೋಗು;
  }
  ಆದರೆ( p->pPk ){
    iStore = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(p->pPk, iIdx);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iIdx >= p->pTab->n_ಲಂಬಸಾಲು ){
    rc = SQLITE_MISUSE_BKPT;
    ಹೋಗು preupdate_old_ಹೊರಹೋಗು;
  }ಇಲ್ಲದಿದ್ದರೆ{
    iStore = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(p->pTab, iIdx);
  }
  ಆದರೆ( iStore>=p->pCsr->n_ಕ್ಷೇತ್ರ || iStore<0 ){
    rc = SQLITE_RANGE;
    ಹೋಗು preupdate_old_ಹೊರಹೋಗು;
  }

  ಆದರೆ( iIdx==p->pTab->iPKey ){
    *pp_ಮೌಲ್ಯ = pMem = &p->oldipk;
    sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem, p->iKey1);
  }ಇಲ್ಲದಿದ್ದರೆ{

    /* If the old.* record has not yet been loaded into memory, do so now. */
    ಆದರೆ( p->pUnpacked==0 ){
      u32 nRec;
      u8 *aRec;

      ಪ್ರತಿಜ್ಞಾ( p->pCsr->eCurType==CURTYPE_BTREE );
      nRec = sqlite3_Btree_Payload_ಗಾತ್ರ(p->pCsr->uc.pCursor);
      aRec = sqlite3DbMallocRaw(db, (u64)nRec+8);
      ಆದರೆ( !aRec ) ಹೋಗು preupdate_old_ಹೊರಹೋಗು;
      rc = sqlite3BtreePayload(p->pCsr->uc.pCursor, 0, nRec, aRec);
      ಆದರೆ( rc==SQLITE_OK ){
        memset(aRec+nRec, 0, 8);
        p->pUnpacked = vdbe_Unpack_ದಾಖಲೆ(p->pKeyinfo, nRec, aRec);
        ಆದರೆ( !p->pUnpacked ) rc = SQLITE_NOMEM;
      }
      ಆದರೆ( rc!=SQLITE_OK ){
        sqlite3DbFree(db, aRec);
        ಹೋಗು preupdate_old_ಹೊರಹೋಗು;
      }
      p->a_ದಾಖಲೆ = aRec;
    }

    pMem = *pp_ಮೌಲ್ಯ = &p->pUnpacked->aMem[iStore];
    ಆದರೆ( iStore>=p->pUnpacked->n_ಕ್ಷೇತ್ರ ){
      /* This occurs when the table has been extended using ALTER TABLE
      ** ADD COLUMN. The value to return is the default value of the column. */
      ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = &p->pTab->a_ಲಂಬಸಾಲು[iIdx];
      ಆದರೆ( p_ಲಂಬಸಾಲು->iDflt>0 ){
        ಆದರೆ( p->apDflt==0 ){
          ಪೂರ್ಣಾಂಕ nByte;
          ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(sqlite3_ಮೌಲ್ಯ*)*UMXV(p->pTab->n_ಲಂಬಸಾಲು) < 0x7fffffff );
          nByte = ಗಾತ್ರ(sqlite3_ಮೌಲ್ಯ*)*p->pTab->n_ಲಂಬಸಾಲು;
          p->apDflt = (sqlite3_ಮೌಲ್ಯ**)sqlite3DbMallocZero(db, nByte);
          ಆದರೆ( p->apDflt==0 ) ಹೋಗು preupdate_old_ಹೊರಹೋಗು;
        }
        ಆದರೆ( p->apDflt[iIdx]==0 ){
          sqlite3_ಮೌಲ್ಯ *pVal = 0;
          ಅಭಿವ್ಯಕ್ತಿ *pDflt;
          ಪ್ರತಿಜ್ಞಾ( p->pTab!=0 && Is_Ordinary_ಕೋಷ್ಟಕ(p->pTab) );
          pDflt = p->pTab->u.tab.pDfltList->a[p_ಲಂಬಸಾಲು->iDflt-1].p_ಅಭಿವ್ಯಕ್ತಿ;
          rc = sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, pDflt, ENC(db), p_ಲಂಬಸಾಲು->affinity, &pVal);
          ಆದರೆ( rc==SQLITE_OK && pVal==0 ){
            rc = SQLITE_CORRUPT_BKPT;
          }
          p->apDflt[iIdx] = pVal;
        }
        *pp_ಮೌಲ್ಯ = p->apDflt[iIdx];
      }ಇಲ್ಲದಿದ್ದರೆ{
        *pp_ಮೌಲ್ಯ = (sqlite3_ಮೌಲ್ಯ *)ಸ್ತಂಭ_Null_ಮೌಲ್ಯ();
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->pTab->a_ಲಂಬಸಾಲು[iIdx].affinity==SQLITE_AFF_REAL ){
      ಆದರೆ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        testcase( pMem->flags & MEM_Int );
        testcase( pMem->flags & MEM_IntReal );
        sqlite3VdbeMemRealify(pMem);
      }
    }
  }

 preupdate_old_ಹೊರಹೋಗು:
  sqlite3Error(db, rc);
  ಹಿಂತಿರುಗಿ sqlite3ApiExit(db, rc);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** the number of columns in the row being updated, deleted or inserted.
*/
ಪೂರ್ಣಾಂಕ sqlite3_preupdate_ಎಣಿಕೆ(sqlite3 *db){
  PreUpdate *p;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ಇಲ್ಲದಿದ್ದರೆ
  p = db->pPreUpdate;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ (p ? p->pKeyinfo->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ : 0);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
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
ಪೂರ್ಣಾಂಕ sqlite3_preupdate_ಆಳ(sqlite3 *db){
  PreUpdate *p;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ಇಲ್ಲದಿದ್ದರೆ
  p = db->pPreUpdate;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ (p ? p->v->n_ಚೌಕಟ್ಟು : 0);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is designed to be called from within a pre-update callback
** only.
*/
ಪೂರ್ಣಾಂಕ sqlite3_preupdate_blobwrite(sqlite3 *db){
  PreUpdate *p;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  p = db!=0 ? db->pPreUpdate : 0;
#ಇಲ್ಲದಿದ್ದರೆ
  p = db->pPreUpdate;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ (p ? p->iBlobWrite : -1);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** This function is called from within a pre-update callback to retrieve
** a field of the row currently being updated or inserted.
*/
ಪೂರ್ಣಾಂಕ sqlite3_preupdate_new(sqlite3 *db, ಪೂರ್ಣಾಂಕ iIdx, sqlite3_ಮೌಲ್ಯ **pp_ಮೌಲ್ಯ){
  PreUpdate *p;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Mem *pMem;
  ಪೂರ್ಣಾಂಕ iStore = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( db==0 || pp_ಮೌಲ್ಯ==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  p = db->pPreUpdate;
  ಆದರೆ( !p || p->op==SQLITE_DELETE ){
    rc = SQLITE_MISUSE_BKPT;
    ಹೋಗು preupdate_new_ಹೊರಹೋಗು;
  }
  ಆದರೆ( p->pPk && p->op!=SQLITE_UPDATE ){
    iStore = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(p->pPk, iIdx);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iIdx >= p->pTab->n_ಲಂಬಸಾಲು ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    iStore = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(p->pTab, iIdx);
  }

  ಆದರೆ( iStore>=p->pCsr->n_ಕ್ಷೇತ್ರ || iStore<0 ){
    rc = SQLITE_RANGE;
    ಹೋಗು preupdate_new_ಹೊರಹೋಗು;
  }

  ಆದರೆ( p->op==SQLITE_ಸೇರಿಸು ){
    /* For an INSERT, memory cell p->iNewReg contains the serialized record
    ** that is being inserted. Deserialize it. */
    Unpacked_ದಾಖಲೆ *pUnpack = p->pNewUnpacked;
    ಆದರೆ( !pUnpack ){
      Mem *p_ದತ್ತಾಂಶ = &p->v->aMem[p->iNewReg];
      rc = ExpandBlob(p_ದತ್ತಾಂಶ);
      ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು preupdate_new_ಹೊರಹೋಗು;
      pUnpack = vdbe_Unpack_ದಾಖಲೆ(p->pKeyinfo, p_ದತ್ತಾಂಶ->n, p_ದತ್ತಾಂಶ->z);
      ಆದರೆ( !pUnpack ){
        rc = SQLITE_NOMEM;
        ಹೋಗು preupdate_new_ಹೊರಹೋಗು;
      }
      p->pNewUnpacked = pUnpack;
    }
    pMem = &pUnpack->aMem[iStore];
    ಆದರೆ( iIdx==p->pTab->iPKey ){
      sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem, p->iKey2);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iStore>=pUnpack->n_ಕ್ಷೇತ್ರ ){
      pMem = (sqlite3_ಮೌಲ್ಯ *)ಸ್ತಂಭ_Null_ಮೌಲ್ಯ();
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p->pTab->a_ಲಂಬಸಾಲು)
           && (p->pTab->a_ಲಂಬಸಾಲು[iIdx].affinity==SQLITE_AFF_REAL)
           && (pMem->flags & (MEM_Int|MEM_IntReal))
    ){
      sqlite3VdbeMemRealify(pMem);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* For an UPDATE, memory cell (p->iNewReg+1+iStore) contains the required
    ** value. Make a copy of the cell contents and return a pointer to it.
    ** It is not safe to return a pointer to the memory cell itself as the
    ** caller may modify the value text encoding.
    */
    ಪ್ರತಿಜ್ಞಾ( p->op==SQLITE_UPDATE );
    ಆದರೆ( !p->aNew ){
      ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(Mem)*UMXV(p->pCsr->n_ಕ್ಷೇತ್ರ) < 0x7fffffff );
      p->aNew = (Mem *)sqlite3DbMallocZero(db, ಗಾತ್ರ(Mem)*p->pCsr->n_ಕ್ಷೇತ್ರ);
      ಆದರೆ( !p->aNew ){
        rc = SQLITE_NOMEM;
        ಹೋಗು preupdate_new_ಹೊರಹೋಗು;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( iStore>=0 && iStore<p->pCsr->n_ಕ್ಷೇತ್ರ );
    pMem = &p->aNew[iStore];
    ಆದರೆ( pMem->flags==0 ){
      ಆದರೆ( iIdx==p->pTab->iPKey ){
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem, p->iKey2);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = sqlite3VdbeMemCopy(pMem, &p->v->aMem[p->iNewReg+1+iStore]);
        ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು preupdate_new_ಹೊರಹೋಗು;
      }
    }
  }
  *pp_ಮೌಲ್ಯ = pMem;

 preupdate_new_ಹೊರಹೋಗು:
  sqlite3Error(db, rc);
  ಹಿಂತಿರುಗಿ sqlite3ApiExit(db, rc);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Return status data for a single loop within query pStmt.
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_scanstatus_v2(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ಪೂರ್ಣಾಂಕ i_ಪರಿಶೀಲನೆ,                      /* Index of loop to report on */
  ಪೂರ್ಣಾಂಕ i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op,              /* Which metric to return */
  ಪೂರ್ಣಾಂಕ flags,
  ಶೂನ್ಯ *p_ಹೊರಹೋಗು                      /* OUT: Write the answer here */
){
  Vdbe *p = (Vdbe*)pStmt;
  VdbeOp *aOp;
  ಪೂರ್ಣಾಂಕ nOp;
  ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ *p_ಪರಿಶೀಲನೆ = 0;
  ಪೂರ್ಣಾಂಕ idx;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 || p_ಹೊರಹೋಗು==0
      || i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op<SQLITE_SCANSTAT_NLOOP
      || i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op>SQLITE_SCANSTAT_NCYCLE ){
    ಹಿಂತಿರುಗಿ 1;
  }
#ಆದರೆ_ಅಂತ್ಯ
  aOp = p->aOp;
  nOp = p->nOp;
  ಆದರೆ( p->p_ಚೌಕಟ್ಟು ){
    Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;
    ಗಾಗಿ(p_ಚೌಕಟ್ಟು=p->p_ಚೌಕಟ್ಟು; p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ; p_ಚೌಕಟ್ಟು=p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ);
    aOp = p_ಚೌಕಟ್ಟು->aOp;
    nOp = p_ಚೌಕಟ್ಟು->nOp;
  }

  ಆದರೆ( i_ಪರಿಶೀಲನೆ<0 ){
    ಪೂರ್ಣಾಂಕ ii;
    ಆದರೆ( i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op==SQLITE_SCANSTAT_NCYCLE ){
      i64 res = 0;
      ಗಾಗಿ(ii=0; ii<nOp; ii++){
        res += aOp[ii].n_ಚಕ್ರ;
      }
      *(i64*)p_ಹೊರಹೋಗು = res;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( flags & SQLITE_SCANSTAT_COMPLEX ){
    idx = i_ಪರಿಶೀಲನೆ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If the COMPLEX flag is clear, then this function must ignore any
    ** ScanStatus structures with ScanStatus.addrLoop set to 0. */
    ಗಾಗಿ(idx=0; idx<p->n_ಪರಿಶೀಲನೆ; idx++){
      p_ಪರಿಶೀಲನೆ = &p->a_ಪರಿಶೀಲನೆ[idx];
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->zName ){
        i_ಪರಿಶೀಲನೆ--;
        ಆದರೆ( i_ಪರಿಶೀಲನೆ<0 ) ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಆದರೆ( idx>=p->n_ಪರಿಶೀಲನೆ ) ಹಿಂತಿರುಗಿ 1;
  ಪ್ರತಿಜ್ಞಾ( p_ಪರಿಶೀಲನೆ==0 || p_ಪರಿಶೀಲನೆ==&p->a_ಪರಿಶೀಲನೆ[idx] );
  p_ಪರಿಶೀಲನೆ = &p->a_ಪರಿಶೀಲನೆ[idx];

  ಆಯ್ಕೆ( i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op ){
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_NLOOP: {
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addrLoop>0 ){
        *(sqlite3_int64*)p_ಹೊರಹೋಗು = aOp[p_ಪರಿಶೀಲನೆ->addrLoop].nExec;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *(sqlite3_int64*)p_ಹೊರಹೋಗು = -1;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_NVISIT: {
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addr_ಭೇಟಿ>0 ){
        *(sqlite3_int64*)p_ಹೊರಹೋಗು = aOp[p_ಪರಿಶೀಲನೆ->addr_ಭೇಟಿ].nExec;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *(sqlite3_int64*)p_ಹೊರಹೋಗು = -1;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_EST: {
      ದ್ವಿಗುಣ r = 1.0;
      LogEst x = p_ಪರಿಶೀಲನೆ->nEst;
      ಆಗ( x<100 ){
        x += 10;
        r *= 0.5;
      }
      *(ದ್ವಿಗುಣ*)p_ಹೊರಹೋಗು = r*sqlite3_Log_Est_ಗಮ್ಯ_Int(x);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_NAME: {
      *(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**)p_ಹೊರಹೋಗು = p_ಪರಿಶೀಲನೆ->zName;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_EXPLAIN: {
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addrExplain ){
        *(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**)p_ಹೊರಹೋಗು = aOp[ p_ಪರಿಶೀಲನೆ->addrExplain ].p4.z;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**)p_ಹೊರಹೋಗು = 0;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_SELECTID: {
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addrExplain ){
        *(ಪೂರ್ಣಾಂಕ*)p_ಹೊರಹೋಗು = aOp[ p_ಪರಿಶೀಲನೆ->addrExplain ].p1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *(ಪೂರ್ಣಾಂಕ*)p_ಹೊರಹೋಗು = -1;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_PARENTID: {
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addrExplain ){
        *(ಪೂರ್ಣಾಂಕ*)p_ಹೊರಹೋಗು = aOp[ p_ಪರಿಶೀಲನೆ->addrExplain ].p2;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *(ಪೂರ್ಣಾಂಕ*)p_ಹೊರಹೋಗು = -1;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_SCANSTAT_NCYCLE: {
      i64 res = 0;
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->aAddrRange[0]==0 ){
        res = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ ii;
        ಗಾಗಿ(ii=0; ii<Array_ಗಾತ್ರ(p_ಪರಿಶೀಲನೆ->aAddrRange); ii+=2){
          ಪೂರ್ಣಾಂಕ iIns = p_ಪರಿಶೀಲನೆ->aAddrRange[ii];
          ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = p_ಪರಿಶೀಲನೆ->aAddrRange[ii+1];
          ಆದರೆ( iIns==0 ) ನಿಲ್ಲಿಸು;
          ಆದರೆ( iIns>0 ){
            ಆಗ( iIns<=i_ಕೊನೆ ){
              res += aOp[iIns].n_ಚಕ್ರ;
              iIns++;
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ಣಾಂಕ iOp;
            ಗಾಗಿ(iOp=0; iOp<nOp; iOp++){
              Op *pOp = &aOp[iOp];
              ಆದರೆ( pOp->p1!=i_ಕೊನೆ ) ಮುಂದುವರಿ;
              ಆದರೆ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_NCYCLE)==0 ){
                ಮುಂದುವರಿ;
              }
              res += aOp[iOp].n_ಚಕ್ರ;
            }
          }
        }
      }
      *(i64*)p_ಹೊರಹೋಗು = res;
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return status data for a single loop within query pStmt.
*/
ಪೂರ್ಣಾಂಕ sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,            /* Prepared statement being queried */
  ಪೂರ್ಣಾಂಕ i_ಪರಿಶೀಲನೆ,                      /* Index of loop to report on */
  ಪೂರ್ಣಾಂಕ i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op,              /* Which metric to return */
  ಶೂನ್ಯ *p_ಹೊರಹೋಗು                      /* OUT: Write the answer here */
){
  ಹಿಂತಿರುಗಿ sqlite3_stmt_scanstatus_v2(pStmt, i_ಪರಿಶೀಲನೆ, i_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Op, 0, p_ಹೊರಹೋಗು);
}

/*
** Zero all counters associated with the sqlite3_stmt_scanstatus() data.
*/
ಶೂನ್ಯ sqlite3_stmt_scanstatus_reset(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  ಪೂರ್ಣಾಂಕ ii;
  ಗಾಗಿ(ii=0; p!=0 && ii<p->nOp; ii++){
    Op *pOp = &p->aOp[ii];
    pOp->nExec = 0;
    pOp->n_ಚಕ್ರ = 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STMT_SCANSTATUS */
