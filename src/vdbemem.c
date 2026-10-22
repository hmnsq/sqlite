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
** This file contains code use to manipulate "Mem" structure.  A "Mem"
** stores a single value in the VDBE.  Mem is an opaque structure visible
** only within the VDBE.  Interface routines refer to a Mem using the
** name sqlite_value
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/* True if X is a power of two.  0 is considered a power of two here.
** In other words, return true if X has at most one bit set.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ISPOWEROF2(X)  (((X)&((X)-1))==0)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Check invariants on a Mem object.
**
** This routine is intended for use inside of assert() statements, like
** this:    assert( sqlite3VdbeCheckMemInvariants(pMem) );
*/
ਪੂਰਨ sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  ਪ੍ਰਤਿਜ੍ਞਾ( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  ਜੇ( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    ਜੇ( (p->flags & (MEM_ਪਦ|MEM_Subtype))==(MEM_ਪਦ|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      ਪ੍ਰਤਿਜ੍ਞਾ( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & ~(MEM_Null|MEM_ਪਦ|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }ਨਹੀਂਤਾਂ{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }ਨਹੀਂਤਾਂ{
    /* The MEM_Cleared bit is only allowed on NULLs */
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->szMalloc==0
       || (p->flags==MEM_Undefined 
           && p->szMalloc<=sqlite3_Db_Malloc_ਮਾਪ(p->db,p->zMalloc))
       || p->szMalloc==sqlite3_Db_Malloc_ਮਾਪ(p->db,p->zMalloc));

  /* If p holds a string or blob, the Mem.z must point to exactly
  ** one of the following:
  **
  **   (1) Memory in Mem.zMalloc and managed by the Mem object
  **   (2) Memory to be freed using Mem.xDel
  **   (3) An ephemeral string or blob
  **   (4) A static string or blob
  */
  ਜੇ( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
ਸਥਿਰ ਖਾਲੀ vdbe_Mem_Render_ਅੰਕ(ਪੂਰਨ sz, ਅੱਖਰ *zBuf, Mem *p){
  StrAccum acc;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sz>22 );
  ਜੇ( p->flags & (MEM_Int|MEM_IntReal) ){
#ਜੇ GCC_ਸੰਸਕਰਣ>=7000000 && GCC_ਸੰਸਕਰਣ<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    ਪ੍ਰਤਿਜ੍ਞਾ( (sqlite3Config.bSmallMalloc!=0xee)*8==ਆਕਾਰ(x) );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&x, (ਅੱਖਰ*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3_Int64_ਮੰਜ਼ਿਲ_ਪਾਠ(x, zBuf);
#ਨਹੀਂਤਾਂ
    p->n = sqlite3_Int64_ਮੰਜ਼ਿਲ_ਪਾਠ(p->u.i, zBuf);
#ਜੇ_ਅੰਤ
    ਜੇ( p->flags & MEM_IntReal ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }ਨਹੀਂਤਾਂ{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    ਪ੍ਰਤਿਜ੍ਞਾ( acc.z_ਪਾਠ==zBuf && acc.mxAlloc<=0 );
    zBuf[acc.nChar] = 0; /* Fast version of sqlite3StrAccumFinish(&acc) */
    p->n = acc.nChar;
  }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Validity checks on pMem.  pMem holds a string.
**
** (1) Check that string value of pMem agrees with its integer or real value.
** (2) Check that the string is correctly zero terminated
**
** A single int or real value always converts to the same strings.  But
** many different strings can be converted into the same int or real.
** If a table contains a numeric value and an index is based on the
** corresponding string value, then it is important that the string be
** derived from the numeric value, not the other way around, to ensure
** that the index and table are consistent.  See ticket
** https://sqlite.org/src/info/343634942dd54ab (2018-01-31) for
** an example.
**
** This routine looks at pMem to verify that if it has both a numeric
** representation and a string representation then the string rep has
** been derived from the numeric and not the other way around.  It returns
** true if everything is ok and false if there is a problem.
**
** This routine is for use inside of assert() statements only.
*/
ਪੂਰਨ sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  ਅੱਖਰ zBuf[100];
  ਅੱਖਰ *z;
  ਪੂਰਨ i, j, incr;
  ਜੇ( (p->flags & MEM_Str)==0 ) ਵਾਪਸ 1;
  ਜੇ( p->db && p->db->mallocFailed ) ਵਾਪਸ 1;
  ਜੇ( p->flags & MEM_ਪਦ ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    ਜੇ( p->szMalloc>0 && p->z==p->zMalloc ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p->z[p->n]==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  ਜੇ( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) ਵਾਪਸ 1;
  ਜੇ( p->db==0 ){
    ਵਾਪਸ 1;  /* db->nFpDigit required to validate p->z[] */
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&tmp, p, ਆਕਾਰ(tmp));
  vdbe_Mem_Render_ਅੰਕ(ਆਕਾਰ(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  ਜੇ( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    ਜੇ( p->enc==SQLITE_UTF16BE ) z++;
  }
  ਜਦੋਂ( zBuf[j] ){
    ਜੇ( zBuf[j++]!=z[i] ) ਵਾਪਸ 0;
    i += incr;
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/*
** If pMem is an object with a valid string representation, this routine
** ensures the internal encoding for the string representation is
** 'desiredEnc', one of SQLITE_UTF8, SQLITE_UTF16LE or SQLITE_UTF16BE.
**
** If pMem is not a string object, or the encoding of the string
** representation is already stored using the requested encoding, then this
** routine is a no-op.
**
** SQLITE_OK is returned if the conversion is successful (or not required).
** SQLITE_NOMEM may be returned if a malloc() fails during conversion
** between formats.
*/
ਪੂਰਨ sqlite3VdbeChangeEncoding(Mem *pMem, ਪੂਰਨ desiredEnc){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  ਪੂਰਨ rc;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  ਜੇ( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    ਵਾਪਸ SQLITE_OK;
  }
  ਜੇ( pMem->enc==desiredEnc ){
    ਵਾਪਸ SQLITE_OK;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  ਵਾਪਸ SQLITE_ERROR;
#ਨਹੀਂਤਾਂ

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  ਪ੍ਰਤਿਜ੍ਞਾ(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  ਪ੍ਰਤਿਜ੍ਞਾ(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  ਪ੍ਰਤਿਜ੍ਞਾ(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
}

/*
** Make sure pMem->z points to a writable allocation of at least n bytes.
**
** If the bPreserve argument is true, then copy of the content of
** pMem->z into the new allocation.  pMem must be either a string or
** blob if bPreserve is true.  If bPreserve is false, any prior content
** in pMem->z is discarded.
*/
SQLITE_NOINLINE ਪੂਰਨ sqlite3VdbeMemGrow(Mem *pMem, ਪੂਰਨ n, ਪੂਰਨ bPreserve){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeCheckMemInvariants(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3_Db_Malloc_ਮਾਪ(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3_Db_Malloc_ਮਾਪ(pMem->db,pMem->zMalloc));
  ਜੇ( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    ਜੇ( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }ਨਹੀਂਤਾਂ{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      ਜੇ( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  ਜੇ( pMem->zMalloc==0 ){
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    pMem->szMalloc = sqlite3_Db_Malloc_ਮਾਪ(pMem->db, pMem->zMalloc);
  }

  ਜੇ( bPreserve && pMem->z ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=pMem->zMalloc );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pMem->zMalloc, pMem->z, pMem->n);
  }
  ਜੇ( (pMem->flags&MEM_Dyn)!=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((ਖਾਲੀ *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Change the pMem->zMalloc allocation to be at least szNew bytes.
** If pMem->zMalloc already meets or exceeds the requested size, this
** routine is a no-op.
**
** Any prior string or blob content in the pMem object may be discarded.
** The pMem->xDel destructor is called, if it exists.  Though MEM_Str
** and MEM_Blob values may be discarded, MEM_Int, MEM_Real, MEM_IntReal,
** and MEM_Null values are preserved.
**
** Return SQLITE_OK on success or an error code (probably SQLITE_NOMEM)
** if unable to complete the resizing.
*/
ਪੂਰਨ sqlite3VdbeMemClearAndResize(Mem *pMem, ਪੂਰਨ szNew){
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || szNew>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  ਜੇ( pMem->szMalloc<szNew ){
    ਵਾਪਸ sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  ਵਾਪਸ SQLITE_OK;
}

/*
** If pMem is already a string, detect if it is a zero-terminated
** string, or make it into one if possible, and mark it as such.
**
** This is an optimization.  Correct operation continues even if
** this routine is a no-op.
**
** Return true if the strig is zero-terminated after this routine is
** called and false if it is not.
*/
ਪੂਰਨ sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  ਜੇ( (pMem->flags & (MEM_Str|MEM_ਪਦ|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    ਵਾਪਸ 0;
  }
  ਜੇ( pMem->enc!=SQLITE_UTF8 ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );
  ਜੇ( pMem->flags & MEM_Dyn ){
    ਜੇ( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_ਪਦ;
      ਵਾਪਸ 1;
    }
    ਜੇ( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_ਪਦ;
      ਵਾਪਸ 1;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_ਪਦ;
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** It is already known that pMem contains an unterminated string.
** Add the zero terminator.
**
** Three bytes of zero are added.  In this way, there is guaranteed
** to be a double-zero byte at an even byte boundary in order to
** terminate a UTF16 string, even if the initial size of the buffer
** is an odd number of bytes.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ vdbe_Mem_ਜੋੜੋ_Terminator(Mem *pMem){
  ਜੇ( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_ਪਦ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
ਪੂਰਨ sqlite3VdbeMemMakeWriteable(Mem *pMem){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਜੇ( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    ਜੇ( ExpandBlob(pMem) ) ਵਾਪਸ SQLITE_NOMEM;
    ਜੇ( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      ਪੂਰਨ rc = vdbe_Mem_ਜੋੜੋ_Terminator(pMem);
      ਜੇ( rc ) ਵਾਪਸ rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
ਪੂਰਨ sqlite3VdbeMemExpandBlob(Mem *pMem){
  ਪੂਰਨ nByte;
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags & MEM_Zero );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_ਮੁੱਲ_nochange(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  ਜੇ( nByte<=0 ){
    ਜੇ( (pMem->flags & MEM_Blob)==0 ) ਵਾਪਸ SQLITE_OK;
    nByte = 1;
  }
  ਜੇ( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Db_Malloc_ਮਾਪ(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_ਪਦ);
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

/*
** Make sure the given Mem is \u0000 terminated.
*/
ਪੂਰਨ sqlite3VdbeMemNulTerminate(Mem *pMem){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_ਪਦ|MEM_Str))==(MEM_ਪਦ|MEM_Str) );
  testcase( (pMem->flags & (MEM_ਪਦ|MEM_Str))==0 );
  ਜੇ( (pMem->flags & (MEM_ਪਦ|MEM_Str))!=MEM_Str ){
    ਵਾਪਸ SQLITE_OK;   /* Nothing to do */
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ vdbe_Mem_ਜੋੜੋ_Terminator(pMem);
  }
}

/*
** Add MEM_Str to the set of representations for the given Mem.  This
** routine is only called if pMem is a number of some kind, not a NULL
** or a BLOB.
**
** Existing representations MEM_Int, MEM_Real, or MEM_IntReal are invalidated
** if bForce is true but are retained if bForce is false.
**
** A MEM_Null value will never be passed to this function. This function is
** used for converting values to text for returning to the user (i.e. via
** sqlite3_value_text()), or for ensuring that values to be used as btree
** keys are strings. In the former case a NULL pointer is returned the
** user and the latter is an internal programming error.
*/
ਪੂਰਨ sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  ਅਚਲ ਪੂਰਨ nByte = 32;

  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !(pMem->flags&MEM_Zero) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );


  ਜੇ( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_ਅੰਕ(nByte, pMem->z, pMem);
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->n==(ਪੂਰਨ)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_ਪਦ;
  ਜੇ( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
ਪੂਰਨ sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  ਪ੍ਰਤਿਜ੍ਞਾ( pFunc!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->xFinalize!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, ਆਕਾਰ(ctx));
  memset(&t, 0, ਆਕਾਰ(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.p_ਬਾਹਰਲਾ = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & MEM_Dyn)==0 );
  ਜੇ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pMem, &t, ਆਕਾਰ(t));
  ਵਾਪਸ ctx.isError;
}

/*
** Memory cell pAccum contains the context of an aggregate function.
** This routine calls the xValue method for that function and stores
** the results in memory cell pMem.
**
** SQLITE_ERROR is returned if xValue() reports an error. SQLITE_OK 
** otherwise.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਪੂਰਨ sqlite3_Vdbe_Mem_Agg_ਮੁੱਲ(Mem *pAccum, Mem *p_ਬਾਹਰਲਾ, FuncDef *pFunc){
  sqlite3_context ctx;
  ਪ੍ਰਤਿਜ੍ਞਾ( pFunc!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->x_ਮੁੱਲ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  ਪ੍ਰਤਿਜ੍ਞਾ( pAccum->db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, ਆਕਾਰ(ctx));
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(p_ਬਾਹਰਲਾ);
  ctx.p_ਬਾਹਰਲਾ = p_ਬਾਹਰਲਾ;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_ਮੁੱਲ(&ctx);
  ਵਾਪਸ ctx.isError;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

/*
** If the memory cell contains a value that must be freed by
** invoking the external callback in Mem.xDel, then this routine
** will free that value.  It also sets Mem.flags to MEM_Null.
**
** This is a helper routine for sqlite3VdbeMemSetNull() and
** for sqlite3VdbeMemRelease().  Use those other routines as the
** entry point for releasing Mem resources.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ vdbe_Mem_Clear_Extern_And_ਸੈੱਟ_Null(Mem *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( VdbeMemDynamic(p) );
  ਜੇ( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  ਜੇ( p->flags&MEM_Dyn ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((ਖਾਲੀ *)p->z);
  }
  p->flags = MEM_Null;
}

/*
** Release memory held by the Mem p, both external memory cleared
** by p->xDel and memory in p->zMalloc.
**
** This is a helper routine invoked by sqlite3VdbeMemRelease() in
** the unusual case where there really is memory in p that needs
** to be freed.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ vdbeMemClear(Mem *p){
  ਜੇ( VdbeMemDynamic(p) ){
    vdbe_Mem_Clear_Extern_And_ਸੈੱਟ_Null(p);
  }
  ਜੇ( p->szMalloc ){
    sqlite3DbFreeNN(p->db, p->zMalloc);
    p->szMalloc = 0;
  }
  p->z = 0;
}

/*
** Release any memory resources held by the Mem.  Both the memory that is
** free by Mem.xDel and the Mem.zMalloc allocation are freed.
**
** Use this routine prior to clean up prior to abandoning a Mem, or to
** reset a Mem back to its minimum memory utilization.
**
** Use sqlite3VdbeMemSetNull() to release just the Mem.xDel space
** prior to inserting new content into the Mem.
*/
ਖਾਲੀ sqlite3VdbeMemRelease(Mem *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeCheckMemInvariants(p) );
  ਜੇ( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
ਖਾਲੀ sqlite3VdbeMemReleaseMalloc(Mem *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( !VdbeMemDynamic(p) );
  ਜੇ( p->szMalloc ) vdbeMemClear(p);
}

/*
** Return some kind of integer value which is the best we can do
** at representing the value that *pMem describes as an integer.
** If pMem is an integer, then the value is exact.  If pMem is
** a floating-point then the value returned is the integer part.
** If pMem is a string or blob, then we make an attempt to convert
** it into an integer and return that.  If pMem represents an
** an SQL-NULL value, return 0.
**
** If pMem represents a string value, its encoding might be changed.
*/
ਸਥਿਰ SQLITE_NOINLINE i64 mem_Int_ਮੁੱਲ(ਅਚਲ Mem *pMem){
  i64 ਮੁੱਲ = 0;
  sqlite3Atoi64(pMem->z, &ਮੁੱਲ, pMem->n, pMem->enc);
  ਵਾਪਸ ਮੁੱਲ;
}
i64 sqlite3_Vdbe_Int_ਮੁੱਲ(ਅਚਲ Mem *pMem){
  ਪੂਰਨ flags;
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  ਜੇ( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    ਵਾਪਸ pMem->u.i;
  }ਨਹੀਂਤਾਂ ਜੇ( flags & MEM_Real ){
    ਵਾਪਸ sqlite3_Real_ਮੰਜ਼ਿਲ_I64(pMem->u.r);
  }ਨਹੀਂਤਾਂ ਜੇ( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    ਵਾਪਸ mem_Int_ਮੁੱਲ(pMem);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}

/*
** This routine implements the uncommon and slower path for
** sqlite3MemRealValueRC() that has to deal with input strings
** that are not UTF8 or that are not zero-terminated.  It is
** broken out into a separate no-inline routine so that the
** main sqlite3MemRealValueRC() routine can avoid unnecessary
** stack pushes.
**
** A text->float translation of pMem->z is written into *pValue.
**
** Result code invariants:
**
**    rc==0         =>   ERROR: Input string not well-formed, or OOM
**    rc<0          =>   Some prefix of the input is well-formed
**    rc>0          =>   All of the input is well-formed
**    (rc&2)==0     =>   The number is expressed as an integer, with no
**                       decimal point or eNNN suffix.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ sqlite3_Mem_Real_ਮੁੱਲ_RCSlow_ਰਸਤਾ(
  Mem *pMem,
  ਦੁੱਗਣਾ *p_ਮੁੱਲ
){
  ਪੂਰਨ rc = SQLITE_OK;
  *p_ਮੁੱਲ = 0.0;
  ਜੇ( pMem->enc==SQLITE_UTF8 ){
    ਅੱਖਰ *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    ਜੇ( zCopy ){
      rc = sqlite3AtoF(zCopy, p_ਮੁੱਲ);
      sqlite3DbFree(pMem->db, zCopy);
    }
    ਵਾਪਸ rc;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ n, i, j;
    ਅੱਖਰ *zCopy;
    ਅਚਲ ਅੱਖਰ *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    ਜੇ( zCopy ){
      z = pMem->z;
      ਜੇ( pMem->enc==SQLITE_UTF16LE ){
        ਲਈ(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          ਜੇ( z[i+1]!=0 ) ਰੋਕੋ;
        }
      }ਨਹੀਂਤਾਂ{
        ਲਈ(i=j=0; i<n-1; i+=2, j++){
          ਜੇ( z[i]!=0 ) ਰੋਕੋ;
          zCopy[j] = z[i+1];
        }
      }     
      ਪ੍ਰਤਿਜ੍ਞਾ( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_ਮੁੱਲ);
      ਜੇ( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    ਵਾਪਸ rc;
  }
}

/*
** Invoke sqlite3AtoF() on the text value of pMem.  Write the
** translation of the text input into *pValue.
**
** The caller must ensure that pMem->db!=0 and that pMem is in
** mode MEM_Str or MEM_Blob.
**
** Result code invariants:
**
**    rc==0         =>   ERROR: Input string not well-formed, or OOM
**    rc<0          =>   Some prefix of the input is well-formed
**    rc>0          =>   All of the input is well-formed
**    (rc&2)==0     =>   The number is expressed as an integer, with no
**                       decimal point or eNNN suffix.
*/
ਪੂਰਨ sqlite3_Mem_Real_ਮੁੱਲ_RC(Mem *pMem, ਦੁੱਗਣਾ *p_ਮੁੱਲ){
  testcase( pMem->db==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags & (MEM_Str|MEM_Blob) );
  ਜੇ( pMem->z==0 ){
    *p_ਮੁੱਲ = 0.0;
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ ਜੇ( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_ਪਦ)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    ਵਾਪਸ sqlite3AtoF(pMem->z, p_ਮੁੱਲ);
  }ਨਹੀਂਤਾਂ ਜੇ( pMem->n==0 ){
    *p_ਮੁੱਲ = 0.0;
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ sqlite3_Mem_Real_ਮੁੱਲ_RCSlow_ਰਸਤਾ(pMem, p_ਮੁੱਲ);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
ਸਥਿਰ SQLITE_NOINLINE ਦੁੱਗਣਾ sqlite3_Mem_Real_ਮੁੱਲ_No_RC(Mem *pMem){
  ਦੁੱਗਣਾ r;
  (ਖਾਲੀ)sqlite3_Mem_Real_ਮੁੱਲ_RC(pMem, &r);
  ਵਾਪਸ r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
ਦੁੱਗਣਾ sqlite3_Vdbe_Real_ਮੁੱਲ(Mem *pMem){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );
  ਜੇ( pMem->flags & MEM_Real ){
    ਵਾਪਸ pMem->u.r;
  }ਨਹੀਂਤਾਂ ਜੇ( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    ਵਾਪਸ (ਦੁੱਗਣਾ)pMem->u.i;
  }ਨਹੀਂਤਾਂ ਜੇ( pMem->flags & (MEM_Str|MEM_Blob) ){
    ਵਾਪਸ sqlite3_Mem_Real_ਮੁੱਲ_No_RC(pMem);
  }ਨਹੀਂਤਾਂ{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    ਵਾਪਸ (ਦੁੱਗਣਾ)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
ਪੂਰਨ sqlite3_Vdbe_Boolean_ਮੁੱਲ(Mem *pMem, ਪੂਰਨ ifNull){
  testcase( pMem->flags & MEM_IntReal );
  ਜੇ( pMem->flags & (MEM_Int|MEM_IntReal) ) ਵਾਪਸ pMem->u.i!=0;
  ਜੇ( pMem->flags & MEM_Null ) ਵਾਪਸ ifNull;
  ਵਾਪਸ sqlite3_Vdbe_Real_ਮੁੱਲ(pMem)!=0.0;
}

/*
** The MEM structure is already a MEM_Real or MEM_IntReal. Try to 
** make it a MEM_Int if we can.
**
** This routine only converts REAL values into INTEGER if the REAL
** has no fractional part and if the value can be exactly expressed
** as a signed 64-bit integer.
**
** The bFromText parameter is true if the MEM was originally text.
** If the value was originally text then this routine will not convert
** -9223372036854775808.0 into -9223372036854775808 because if the value
** was originally text, it would have been converted into an integer
** already, and since it was not, that means that the original text
** content had the last four significant digits greater than "5808"
** and the hence the text value was not really -9223372036854775808.0
** but rather some nearby value that happens to have the same IEEE 754
** representation. We don't want text '-9223372036854775900.0' to get
** converted into -9223372036854775808.
**
** But if bFromText is false, then the value started out as REAL and
** so it is ok to convert -9223372036854775808.0 into -9223372036854775808
** in that case, because -9223372036854775808.0 is the exact value of
** the underlying IEEE 754 representation.
*/
ਖਾਲੀ sqlite3VdbeIntegerAffinity(Mem *pMem, ਪੂਰਨ b_From_ਪਾਠ){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags & (MEM_Real|MEM_IntReal) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );

  ਜੇ( pMem->flags & MEM_IntReal ){
    Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Int);
  }ਨਹੀਂਤਾਂ{
    i64 ix = sqlite3_Real_ਮੰਜ਼ਿਲ_I64(pMem->u.r);

    /* Only mark the value as an integer if
    **
    **    (1) the round-trip conversion real->int->real is a no-op, and
    **    (2) The integer is neither the largest nor the smallest
    **        possible integer (ticket #3922)
    **
    ** The second and third terms in the following conditional enforces
    ** the second condition under the assumption that addition overflow causes
    ** values to wrap around.
    */
    ਜੇ( pMem->u.r==ix && ix<ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64
     && (!b_From_ਪਾਠ || ix>ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64)
    ){
      pMem->u.i = ix;
      Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Int);
    }
  }
}

/*
** Convert pMem to type integer.  Invalidate any prior representations.
*/
ਪੂਰਨ sqlite3VdbeMemIntegerify(Mem *pMem){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_ਮੁੱਲ(pMem);
  Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Int);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
ਪੂਰਨ sqlite3VdbeMemRealify(Mem *pMem){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_ਮੁੱਲ(pMem);
  Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Real);
  ਵਾਪਸ SQLITE_OK;
}

/* Compare a floating point value to an integer.  Return true if the two
** values are the same within the precision of the floating point value.
**
** This function assumes that i was obtained by assignment from r1.
**
** For some versions of GCC on 32-bit machines, if you do the more obvious
** comparison of "r1==(double)i" you sometimes get an answer of false even
** though the r1 and (double)i values are bit-for-bit the same.
*/
ਪੂਰਨ sqlite3RealSameAsInt(ਦੁੱਗਣਾ r1, sqlite3_int64 i){
  ਦੁੱਗਣਾ r2 = (ਦੁੱਗਣਾ)i;
  ਵਾਪਸ r1==0.0
      || (memcmp(&r1, &r2, ਆਕਾਰ(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3_Real_ਮੰਜ਼ਿਲ_I64(ਦੁੱਗਣਾ r){
  ਜੇ( r<-9223372036854774784.0 ) ਵਾਪਸ ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64;
  ਜੇ( r>+9223372036854774784.0 ) ਵਾਪਸ ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64;
  ਵਾਪਸ (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
ਪੂਰਨ sqlite3VdbeMemNumerify(Mem *pMem){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  ਜੇ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    ਪੂਰਨ rc;
    sqlite3_int64 ix;
    ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_ਮੁੱਲ_RC(pMem, &pMem->u.r);
    ਜੇ( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3_Real_ਮੰਜ਼ਿਲ_I64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Int);
    }ਨਹੀਂਤਾਂ{
      Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Real);
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
ਪੂਰਨ sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  ਜੇ( pMem->flags & MEM_Null ) ਵਾਪਸ SQLITE_OK;
  ਚੋਣ( aff ){
    ਹਾਲਤ SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      ਜੇ( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_ਮੁੱਲ_Apply_Affinity(pMem, SQLITE_AFF_ਪਾਠ, encoding);
        ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        ਜੇ( pMem->flags & MEM_Str ) Mem_ਸੈੱਟ_Type_Flag(pMem, MEM_Blob);
      }ਨਹੀਂਤਾਂ{
        pMem->flags &= ~(MEM_Type_ਨਕਾਬ&~MEM_Blob);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਪੂਰਨ rc;
      ਪ੍ਰਤਿਜ੍ਞਾ( aff==SQLITE_AFF_ਪਾਠ );
      ਪ੍ਰਤਿਜ੍ਞਾ( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_ਮੁੱਲ_Apply_Affinity(pMem, SQLITE_AFF_ਪਾਠ, encoding);
      ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      ਜੇ( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      ਜੇ( rc ) ਵਾਪਸ rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
ਖਾਲੀ sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & ~MEM_Type_ਨਕਾਬ)==0 );
  pMem->flags = flags;
  pMem->db = db;
  pMem->szMalloc = 0;
}


/*
** Delete any previous value and set the value stored in *pMem to NULL.
**
** This routine calls the Mem.xDel destructor to dispose of values that
** require the destructor.  But it preserves the Mem.zMalloc memory allocation.
** To free all resources, use sqlite3VdbeMemRelease(), which both calls this
** routine to invoke the destructor and deallocates Mem.zMalloc.
**
** Use this routine to reset the Mem prior to insert a new value.
**
** Use sqlite3VdbeMemRelease() to complete erase the Mem prior to abandoning it.
*/
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(Mem *pMem){
  ਜੇ( VdbeMemDynamic(pMem) ){
    vdbe_Mem_Clear_Extern_And_ਸੈੱਟ_Null(pMem);
  }ਨਹੀਂਤਾਂ{
    pMem->flags = MEM_Null;
  }
}
ਖਾਲੀ sqlite3_ਮੁੱਲ_ਸੈੱਟ_Null(sqlite3_ਮੁੱਲ *p){
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(Mem *pMem, ਪੂਰਨ n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  ਜੇ( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(Mem *pMem, ਪੂਰਨ n){
  ਪੂਰਨ nByte = n>0?n:1;
  ਜੇ( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Db_Malloc_ਮਾਪ(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ vdbe_Release_And_ਸੈੱਟ_Int64(Mem *pMem, i64 val){
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(Mem *pMem, i64 val){
  ਜੇ( VdbeMemDynamic(pMem) ){
    vdbe_Release_And_ਸੈੱਟ_Int64(pMem, val);
  }ਨਹੀਂਤਾਂ{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
ਖਾਲੀ sqlite3_Mem_ਸੈੱਟ_Array_Int64(sqlite3_ਮੁੱਲ *aMem, ਪੂਰਨ iIdx, i64 val){
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(&aMem[iIdx], val);
}

/* A no-op destructor */
ਖਾਲੀ sqlite3NoopDestructor(ਖਾਲੀ *p){ ਅਣਵਰਤਿਆ_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_ਸੂਚਕ(
  Mem *pMem,
  ਖਾਲੀ *pPtr,
  ਅਚਲ ਅੱਖਰ *zPType,
  ਖਾਲੀ (*xDestructor)(ਖਾਲੀ*)
){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags==MEM_Null );
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = MEM_Null|MEM_Dyn|MEM_Subtype|MEM_ਪਦ;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type REAL.
*/
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Double(Mem *pMem, ਦੁੱਗਣਾ val){
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
  ਜੇ( !sqlite3IsNaN(val) ){
    pMem->u.r = val;
    pMem->flags = MEM_Real;
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Return true if the Mem holds a RowSet object.  This routine is intended
** for use inside of assert() statements.
*/
ਪੂਰਨ sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(ਅਚਲ Mem *pMem){
  ਵਾਪਸ (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
         && pMem->xDel==sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Delete;
}
#ਜੇ_ਅੰਤ

/*
** Delete any previous value and set the value of pMem to be an
** empty boolean index.
**
** Return SQLITE_OK on success and SQLITE_NOMEM if a memory allocation
** error occurs.
*/
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪੰਕਤੀ_ਸੈੱਟ(Mem *pMem){
  sqlite3 *db = pMem->db;
  ਪੰਕਤੀ_ਸੈੱਟ *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Init(db);
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM;
  pMem->z = (ਅੱਖਰ*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Delete;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
ਪੂਰਨ sqlite3VdbeMemTooBig(Mem *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->db!=0 );
  ਜੇ( p->flags & (MEM_Str|MEM_Blob) ){
    ਪੂਰਨ n = p->n;
    ਜੇ( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    ਵਾਪਸ n>p->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ];
  }
  ਵਾਪਸ 0; 
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** This routine prepares a memory cell for modification by breaking
** its link to a shallow copy and by marking any current shallow
** copies of this cell as invalid.
**
** This is used for testing and debugging only - to help ensure that shallow
** copies (created by OP_SCopy) are not misused.
*/
ਖਾਲੀ sqlite3_Vdbe_Mem_About_ਮੰਜ਼ਿਲ_Change(Vdbe *pVdbe, Mem *pMem){
  ਪੂਰਨ i;
  Mem *pX;
  ਜੇ( pMem->bScopy ){
    ਲਈ(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      ਜੇ( pX->pScopyFrom==pMem ){
        u16 mFlags;
        ਜੇ( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (ਪੂਰਨ)(pX - pVdbe->aMem), (ਪੂਰਨ)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        ਪ੍ਰਤਿਜ੍ਞਾ( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
        /* pMem is the register that is changing.  But also mark pX as
        ** undefined so that we can quickly detect the shallow-copy error */
        pX->flags = MEM_Undefined;
        pX->pScopyFrom = 0;
      }
    }
    pMem->bScopy = 0;
  }
  pMem->pScopyFrom = 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/*
** Make an shallow copy of pFrom into pTo.  Prior contents of
** pTo are freed.  The pFrom->z field is not duplicated.  If
** pFrom->z is used, then pTo->z points to the same thing as pFrom->z
** and flags gets srcType (either MEM_Ephem or MEM_Static).
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ vdbeClrCopy(Mem *p_ਮੰਜ਼ਿਲ, ਅਚਲ Mem *pFrom, ਪੂਰਨ eType){
  vdbe_Mem_Clear_Extern_And_ਸੈੱਟ_Null(p_ਮੰਜ਼ਿਲ);
  ਪ੍ਰਤਿਜ੍ਞਾ( !VdbeMemDynamic(p_ਮੰਜ਼ਿਲ) );
  sqlite3VdbeMemShallowCopy(p_ਮੰਜ਼ਿਲ, pFrom, eType);
}
ਖਾਲੀ sqlite3VdbeMemShallowCopy(Mem *p_ਮੰਜ਼ਿਲ, ਅਚਲ Mem *pFrom, ਪੂਰਨ srcType){
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pFrom) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ->db==pFrom->db );
  ਜੇ( VdbeMemDynamic(p_ਮੰਜ਼ਿਲ) ){ vdbeClrCopy(p_ਮੰਜ਼ਿਲ,pFrom,srcType); ਵਾਪਸ; }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ, pFrom, MEMCELLSIZE);
  ਜੇ( (pFrom->flags&MEM_Static)==0 ){
    p_ਮੰਜ਼ਿਲ->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    ਪ੍ਰਤਿਜ੍ਞਾ( srcType==MEM_Ephem || srcType==MEM_Static );
    p_ਮੰਜ਼ਿਲ->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
ਪੂਰਨ sqlite3VdbeMemCopy(Mem *p_ਮੰਜ਼ਿਲ, ਅਚਲ Mem *pFrom){
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pFrom) );
  ਜੇ( VdbeMemDynamic(p_ਮੰਜ਼ਿਲ) ) vdbe_Mem_Clear_Extern_And_ਸੈੱਟ_Null(p_ਮੰਜ਼ਿਲ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ, pFrom, MEMCELLSIZE);
  p_ਮੰਜ਼ਿਲ->flags &= ~MEM_Dyn;
  ਜੇ( p_ਮੰਜ਼ਿਲ->flags&(MEM_Str|MEM_Blob) ){
    ਜੇ( 0==(pFrom->flags&MEM_Static) ){
      p_ਮੰਜ਼ਿਲ->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(p_ਮੰਜ਼ਿਲ);
    }
  }

  ਵਾਪਸ rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
ਖਾਲੀ sqlite3VdbeMemMove(Mem *p_ਮੰਜ਼ਿਲ, Mem *pFrom){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ->db==0 || sqlite3_mutex_held(p_ਮੰਜ਼ਿਲ->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->db==0 || p_ਮੰਜ਼ਿਲ->db==0 || pFrom->db==p_ਮੰਜ਼ਿਲ->db );

  sqlite3VdbeMemRelease(p_ਮੰਜ਼ਿਲ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ, pFrom, ਆਕਾਰ(Mem));
  pFrom->flags = MEM_Null;
  pFrom->szMalloc = 0;
}

/*
** Change the value of a Mem to be a string or a BLOB.
**
** The memory management strategy depends on the value of the xDel
** parameter. If the value passed is SQLITE_TRANSIENT, then the 
** string is copied into a (possibly existing) buffer managed by the 
** Mem structure. Otherwise, any existing buffer is freed and the
** pointer copied.
**
** If the string is too large (if it exceeds the SQLITE_LIMIT_LENGTH
** size limit) then no memory allocation occurs.  If the string can be
** stored without allocating memory, then it is.  If a memory allocation
** is required to store the string, then value of pMem is unchanged.  In
** either case, SQLITE_TOOBIG is returned.
**
** The "enc" parameter is the text encoding for the string, or zero
** to store a blob.  If enc not 0 or SQLITE_UTF8 and the string
** begins with a BOM, the BOM is removed.  To retain the BOM on a UTF16
** string, use a non-negative "n" and set enc to SQLITE_UTF8, and
** separately set pMem->enc to the correct encoding after this function
** returns.
**
** If n is negative and enc is not zero and is set correctly, then the
** string consists of all bytes up to but excluding the first zero
** character.  The n parameter must be non-negative for blobs or if
** enc is not set accurately.
*/
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(
  Mem *pMem,          /* Memory cell to set to string value */
  ਅਚਲ ਅੱਖਰ *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  ਪੂਰਨ i_ਹੱਦਬੰਦੀ;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  ਪ੍ਰਤਿਜ੍ਞਾ( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ਜੇ( !z ){
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
    ਵਾਪਸ SQLITE_OK;
  }

  ਜੇ( pMem->db ){
    i_ਹੱਦਬੰਦੀ = pMem->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ];
  }ਨਹੀਂਤਾਂ{
    i_ਹੱਦਬੰਦੀ = SQLITE_MAX_ਲੰਮਾਈ;
  }
  ਜੇ( nByte<0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( enc!=0 );
    ਜੇ( enc==SQLITE_UTF8 ){
      nByte = ਲੜੀ_ਲੰਬਾਈ(z);
    }ਨਹੀਂਤਾਂ{
      ਲਈ(nByte=0; nByte<=i_ਹੱਦਬੰਦੀ && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_ਪਦ;
  }ਨਹੀਂਤਾਂ ਜੇ( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }ਨਹੀਂਤਾਂ{
    flags = MEM_Str;
  }
  ਜੇ( nByte>i_ਹੱਦਬੰਦੀ ){
    ਜੇ( xDel && xDel!=SQLITE_TRANSIENT ){
      ਜੇ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (ਖਾਲੀ*)z);
      }ਨਹੀਂਤਾਂ{
        xDel((ਖਾਲੀ*)z);
      }
    }
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
    ਵਾਪਸ sqlite3_Error_ਮੰਜ਼ਿਲ_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ਜੇ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    ਜੇ( flags&MEM_ਪਦ ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ਜੇ( sqlite3VdbeMemClearAndResize(pMem, (ਪੂਰਨ)MAX(nAlloc,32)) ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pMem->z, z, nAlloc);
  }ਨਹੀਂਤਾਂ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (ਅੱਖਰ *)z;
    ਜੇ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_ਮਾਪ(pMem->db, pMem->zMalloc);
    }ਨਹੀਂਤਾਂ{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (ਪੂਰਨ)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  ਜੇ( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
#ਜੇ_ਅੰਤ


  ਵਾਪਸ SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(
  Mem *pMem,          /* Memory cell to set to string value */
  ਅਚਲ ਅੱਖਰ *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  ਪ੍ਰਤਿਜ੍ਞਾ( pMem!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ਜੇ( !z ){
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
    ਵਾਪਸ SQLITE_OK;
  }

  ਜੇ( nByte<0 ){
    nByte = ਲੜੀ_ਲੰਬਾਈ(z);
    flags = MEM_Str|MEM_ਪਦ;
  }ਨਹੀਂਤਾਂ{
    flags = MEM_Str;
  }
  ਜੇ( nByte>(i64)pMem->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] ){
    ਜੇ( xDel && xDel!=SQLITE_TRANSIENT ){
      ਜੇ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (ਖਾਲੀ*)z);
      }ਨਹੀਂਤਾਂ{
        xDel((ਖਾਲੀ*)z);
      }
    }
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem);
    ਵਾਪਸ sqlite3_Error_ਮੰਜ਼ਿਲ_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ਜੇ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ਜੇ( sqlite3VdbeMemClearAndResize(pMem, (ਪੂਰਨ)MAX(nAlloc,32)) ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }ਨਹੀਂਤਾਂ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (ਅੱਖਰ *)z;
    ਜੇ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_ਮਾਪ(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }ਨਹੀਂਤਾਂ{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (ਪੂਰਨ)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Move data out of a btree key or data field and into a Mem structure.
** The data is payload from the entry that pCur is currently pointing
** to.  offset and amt determine what portion of the data or key to retrieve.
** The result is written into the pMem element.
**
** The pMem object must have been initialized.  This routine will use
** pMem->zMalloc to hold the content from the btree, if possible.  New
** pMem->zMalloc space will be allocated if necessary.  The calling routine
** is responsible for making sure that the pMem object is eventually
** destroyed.
**
** If this routine fails for any reason (malloc returns NULL or unable
** to read from the disk) then the pMem is left in an inconsistent state.
*/
ਪੂਰਨ sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 ਵਿੱਥ,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  ਪੂਰਨ rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ਵੰਡ_ਮਾਪ-1 );
  testcase( amt==SQLITE_MAX_ਵੰਡ_ਮਾਪ );
  ਜੇ( amt>=SQLITE_MAX_ਵੰਡ_ਮਾਪ ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਜੇ( (u64)amt + (u64)ਵਿੱਥ > (u64)sqlite3_Btree_Max_ਰਿਕਾਰਡ_ਮਾਪ(pCur) ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  ਜੇ( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, ਵਿੱਥ, amt, pMem->z);
    ਜੇ( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (ਪੂਰਨ)amt;
    }ਨਹੀਂਤਾਂ{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_Vdbe_Mem_From_Btree_Zero_ਵਿੱਥ(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 ਉਪਲਬਧ = 0;  /* Number of bytes available on the local btree page */
  ਪੂਰਨ rc = SQLITE_OK; /* Return code */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeCursorIsValid(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem) );
  pMem->z = (ਅੱਖਰ *)sqlite3BtreePayloadFetch(pCur, &ਉਪਲਬਧ);
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->z!=0 );

  ਜੇ( amt<=ਉਪਲਬਧ ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (ਪੂਰਨ)amt;
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  ਵਾਪਸ rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
ਸਥਿਰ SQLITE_NOINLINE ਅਚਲ ਖਾਲੀ *ਮੁੱਲ_ਮੰਜ਼ਿਲ_ਪਾਠ(sqlite3_ਮੁੱਲ* pVal, u8 enc){
  ਪ੍ਰਤਿਜ੍ਞਾ( pVal!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pVal) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pVal->flags & (MEM_Null))==0 );
  ਜੇ( pVal->flags & (MEM_Blob|MEM_Str) ){
    ਜੇ( ExpandBlob(pVal) ) ਵਾਪਸ 0;
    pVal->flags |= MEM_Str;
    ਜੇ( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    ਜੇ( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(pVal->z)) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      ਜੇ( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        ਵਾਪਸ 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }ਨਹੀਂਤਾਂ{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( 0==(1&SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(pVal->z)) );
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  ਜੇ( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeMemValidStrRep(pVal) );
    ਵਾਪਸ pVal->z;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}

/* This function is only available internally, it is not part of the
** external API. It works in a similar way to sqlite3_value_text(),
** except the data returned is in the encoding specified by the second
** parameter, which must be one of SQLITE_UTF16BE, SQLITE_UTF16LE or
** SQLITE_UTF8.
**
** (2006-02-16:)  The enc value can be or-ed with SQLITE_UTF16_ALIGNED.
** If that is the case, then the result must be aligned on an even byte
** boundary.
*/
ਅਚਲ ਖਾਲੀ *sqlite3_ਮੁੱਲ_ਪਾਠ_2(sqlite3_ਮੁੱਲ* pVal, u8 enc){
  ਜੇ( !pVal ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pVal) );
  ਜੇ( (pVal->flags&(MEM_Str|MEM_ਪਦ))==(MEM_Str|MEM_ਪਦ) && pVal->enc==enc ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeMemValidStrRep(pVal) );
    ਵਾਪਸ pVal->z;
  }
  ਜੇ( pVal->flags&MEM_Null ){
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ ਮੁੱਲ_ਮੰਜ਼ਿਲ_ਪਾਠ(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
ਪੂਰਨ sqlite3_ਮੁੱਲ_Is_ਦਾ_Class(ਅਚਲ sqlite3_ਮੁੱਲ *pVal, ਖਾਲੀ(*xFree)(ਖਾਲੀ*)){
  ਜੇ( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_ਮੁੱਲ *sqlite3_ਮੁੱਲ_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, ਆਕਾਰ(*p));
  ਜੇ( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  ਵਾਪਸ p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
ਬਣਤਰ ਮੁੱਲ_New_Stat4_Ctx {
  Parse *pParse;
  ਸੂਚਕਾਂਕ *pIdx;
  Unpacked_ਰਿਕਾਰਡ **ppRec;
  ਪੂਰਨ iVal;
};

/*
** Allocate and return a pointer to a new sqlite3_value object. If
** the second argument to this function is NULL, the object is allocated
** by calling sqlite3ValueNew().
**
** Otherwise, if the second argument is non-zero, then this function is 
** being called indirectly by sqlite3Stat4ProbeSetValue(). If it has not
** already been allocated, allocate the UnpackedRecord structure that 
** that function will return to its caller here. Then return a pointer to
** an sqlite3_value within the UnpackedRecord.a[] array.
*/
ਸਥਿਰ sqlite3_ਮੁੱਲ *ਮੁੱਲ_New(sqlite3 *db, ਬਣਤਰ ਮੁੱਲ_New_Stat4_Ctx *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( p ){
    Unpacked_ਰਿਕਾਰਡ *pRec = p->ppRec[0];

    ਜੇ( pRec==0 ){
      ਸੂਚਕਾਂਕ *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      ਪੂਰਨ i;                      /* Counter variable */
      ਪੂਰਨ n_ਖਾਨਾ = pIdx->n_ਕਾਲਮ;   /* Number of index columns including rowid */
  
      nByte = ਆਕਾਰ(Mem) * n_ਖਾਨਾ + ROUND8(ਆਕਾਰ(Unpacked_ਰਿਕਾਰਡ));
      pRec = (Unpacked_ਰਿਕਾਰਡ*)sqlite3DbMallocZero(db, nByte);
      ਜੇ( pRec ){
        pRec->p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_ਦਾ_ਸੂਚਕਾਂਕ(p->pParse, pIdx);
        ਜੇ( pRec->p_ਕੁੰਜੀ_Info ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pRec->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ==n_ਖਾਨਾ );
          ਪ੍ਰਤਿਜ੍ਞਾ( pRec->p_ਕੁੰਜੀ_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(ਆਕਾਰ(Unpacked_ਰਿਕਾਰਡ)));
          ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }ਨਹੀਂਤਾਂ{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      ਜੇ( pRec==0 ) ਵਾਪਸ 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->n_ਖੇਤਰ = p->iVal+1;
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(&pRec->aMem[p->iVal]);
    ਵਾਪਸ &pRec->aMem[p->iVal];
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(p);
#ਜੇ_ਅੰਤ /* defined(SQLITE_ENABLE_STAT4) */
  ਵਾਪਸ sqlite3_ਮੁੱਲ_New(db);
}

/*
** The expression object indicated by the second argument is guaranteed
** to be a scalar SQL function. If
**
**   * all function arguments are SQL literals,
**   * one of the SQLITE_FUNC_CONSTANT or _SLOCHNG function flags is set, and
**   * the SQLITE_FUNC_NEEDCOLL function flag is not set,
**
** then this routine attempts to invoke the SQL function. Assuming no
** error occurs, output parameter (*ppVal) is set to point to a value 
** object containing the result before returning SQLITE_OK.
**
** Affinity aff is applied to the result of the function before returning.
** If the result is a text value, the sqlite3_value object uses encoding 
** enc.
**
** If the conditions above are not met, this function returns SQLITE_OK
** and sets (*ppVal) to NULL. Or, if an error occurs, (*ppVal) is set to
** NULL and an SQLite error code returned.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
ਸਥਿਰ ਪੂਰਨ ਮੁੱਲ_From_Function(
  sqlite3 *db,                    /* The database connection */
  ਅਚਲ ਵਿਅੰਜਕ *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_ਮੁੱਲ **ppVal,          /* Write the new value here */
  ਬਣਤਰ ਮੁੱਲ_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_ਮੁੱਲ **apVal = 0;      /* Function arguments */
  ਪੂਰਨ nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_ਮੁੱਲ *pVal = 0;        /* New value */
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  ਵਿਅੰਜਕ_List *pList = 0;            /* Function arguments */
  ਪੂਰਨ i;                          /* Iterator variable */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCtx!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & EP_TokenOnly)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p) );
  pList = p->x.pList;
  ਜੇ( pList ) nVal = pList->n_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) );
  pFunc = sqlite3_ਲੱਭੋ_Function(db, p->u.zToken, nVal, enc, 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ਜੇ( pFunc==0 ) ਵਾਪਸ SQLITE_OK;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( pFunc );
  ਜੇ( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    ਵਾਪਸ SQLITE_OK;
  }

  ਜੇ( pList ){
    apVal = (sqlite3_ਮੁੱਲ**)sqlite3DbMallocZero(db, ਆਕਾਰ(apVal[0]) * nVal);
    ਜੇ( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      ਜਾਓ ਮੁੱਲ_from_function_ਬਾਹਰਲਾ;
    }
    ਲਈ(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(pCtx->pParse, pList->a[i].p_ਵਿਅੰਜਕ, aff,
                                     &apVal[i]);
      ਜੇ( apVal[i]==0 || rc!=SQLITE_OK ) ਜਾਓ ਮੁੱਲ_from_function_ਬਾਹਰਲਾ;
    }
  }

  pVal = ਮੁੱਲ_New(db, pCtx);
  ਜੇ( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    ਜਾਓ ਮੁੱਲ_from_function_ਬਾਹਰਲਾ;
  }

  memset(&ctx, 0, ਆਕਾਰ(ctx));
  ctx.p_ਬਾਹਰਲਾ = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  ਜੇ( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_ਸੁਨੇਹਾ(pCtx->pParse, "%s", sqlite3_ਮੁੱਲ_ਪਾਠ(pVal));
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਮੁੱਲ_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    ਜੇ( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 ਮੁੱਲ_from_function_ਬਾਹਰਲਾ:
  ਜੇ( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  ਜੇ( apVal ){
    ਲਈ(i=0; i<nVal; i++){
      sqlite3_ਮੁੱਲ_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  ਵਾਪਸ rc;
}
#ਨਹੀਂਤਾਂ
# define ਮੁੱਲ_From_Function(a,b,c,d,e,f) SQLITE_OK
#ਜੇ_ਅੰਤ /* defined(SQLITE_ENABLE_STAT4) */

/*
** Extract a value from the supplied expression in the manner described
** above sqlite3ValueFromExpr(). Allocate the sqlite3_value object
** using valueNew().
**
** If pCtx is NULL and an error occurs after the sqlite3_value object
** has been allocated, it is freed before returning. Or, if pCtx is not
** NULL, it is assumed that the caller will free any allocated object
** in all cases.
*/
ਸਥਿਰ ਪੂਰਨ ਮੁੱਲ_From_ਵਿਅੰਜਕ(
  sqlite3 *db,                    /* The database connection */
  ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_ਮੁੱਲ **ppVal,          /* Write the new value here */
  ਬਣਤਰ ਮੁੱਲ_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  ਪੂਰਨ op;
  ਅੱਖਰ *zVal = 0;
  sqlite3_ਮੁੱਲ *pVal = 0;
  ਪੂਰਨ negInt = 1;
  ਅਚਲ ਅੱਖਰ *zNeg = "";
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  ਜਦੋਂ( (op = p_ਵਿਅੰਜਕ->op)==TK_UPLUS || op==TK_ਵਿਸਤਾਰ ) p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  ਜੇ( op==TK_ਰਜਿਸਟਰ ) op = p_ਵਿਅੰਜਕ->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਵਿਅੰਜਕ->flags & EP_TokenOnly)==0 || pCtx==0 );

  ਜੇ( op==TK_CAST ){
    u8 aff;
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
    aff = sqlite3AffinityType(p_ਵਿਅੰਜਕ->u.zToken,0);
    rc = ਮੁੱਲ_From_ਵਿਅੰਜਕ(db, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    ਜੇ( *ppVal ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#ਨਹੀਂਤਾਂ
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      ਪ੍ਰਤਿਜ੍ਞਾ( (ppVal[0][0].flags & MEM_Zero)==0 );
#ਜੇ_ਅੰਤ
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_ਮੁੱਲ_Apply_Affinity(*ppVal, affinity, enc);
    }
    ਵਾਪਸ rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  ਜੇ( op==TK_UMINUS ){
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
    ਜੇ( (p_ਖੱਬਾ->op==TK_INTEGER || p_ਖੱਬਾ->op==TK_FLOAT) ){
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਖੱਬਾ, EP_Int_ਮੁੱਲ)
       || p_ਖੱਬਾ->u.zToken[0]!='0' || (p_ਖੱਬਾ->u.zToken[1] & ~0x20)!='X'
      ){
        p_ਵਿਅੰਜਕ = p_ਖੱਬਾ;
        op = p_ਵਿਅੰਜਕ->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  ਜੇ( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = ਮੁੱਲ_New(db, pCtx);
    ਜੇ( pVal==0 ) ਜਾਓ no_mem;
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) ){
      sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pVal, (i64)p_ਵਿਅੰਜਕ->u.i_ਮੁੱਲ*negInt);
    }ਨਹੀਂਤਾਂ{
      i64 iVal;
      ਜੇ( op==TK_INTEGER && 0==sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(p_ਵਿਅੰਜਕ->u.zToken, &iVal) ){
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pVal, iVal*negInt);
      }ਨਹੀਂਤਾਂ{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_ਵਿਅੰਜਕ->u.zToken);
        ਜੇ( zVal==0 ) ਜਾਓ no_mem;
        sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        ਜੇ( op==TK_FLOAT ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_ਪਦ) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    ਜੇ( affinity==SQLITE_AFF_BLOB ){
      ਜੇ( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_ਮੁੱਲ_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਮੁੱਲ_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( (pVal->flags & MEM_IntReal)==0 );
    ਜੇ( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    ਜੇ( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    ਜੇ( SQLITE_OK==ਮੁੱਲ_From_ਵਿਅੰਜਕ(db,p_ਵਿਅੰਜਕ->p_ਖੱਬਾ,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      ਜੇ( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }ਨਹੀਂਤਾਂ ਜੇ( pVal->u.i==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
        pVal->u.r = -(ਦੁੱਗਣਾ)ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64;
#ਨਹੀਂਤਾਂ
        pVal->u.r = ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64;
#ਜੇ_ਅੰਤ
        Mem_ਸੈੱਟ_Type_Flag(pVal, MEM_Real);
      }ਨਹੀਂਤਾਂ{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_ਮੁੱਲ_Apply_Affinity(pVal, affinity, enc);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_NULL ){
    pVal = ਮੁੱਲ_New(db, pCtx);
    ਜੇ( pVal==0 ) ਜਾਓ no_mem;
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pVal);
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_BLOB_LITERAL
  ਨਹੀਂਤਾਂ ਜੇ( op==TK_BLOB ){
    ਪੂਰਨ nVal;
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken[0]=='x' || p_ਵਿਅੰਜਕ->u.zToken[0]=='X' );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken[1]=='\'' );
    pVal = ਮੁੱਲ_New(db, pCtx);
    ਜੇ( !pVal ) ਜਾਓ no_mem;
    zVal = &p_ਵਿਅੰਜਕ->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    ਪ੍ਰਤਿਜ੍ਞਾ( zVal[nVal]=='\'' );
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pVal, sqlite3_Hex_ਮੰਜ਼ਿਲ_Blob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਨਹੀਂਤਾਂ ਜੇ( op==TK_FUNCTION && pCtx!=0 ){
    rc = ਮੁੱਲ_From_Function(db, p_ਵਿਅੰਜਕ, enc, affinity, &pVal, pCtx);
  }
#ਜੇ_ਅੰਤ
  ਨਹੀਂਤਾਂ ਜੇ( op==TK_TRUEFALSE ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
    pVal = ਮੁੱਲ_New(db, pCtx);
    ਜੇ( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_ਵਿਅੰਜਕ->u.zToken[4]==0;
      sqlite3_ਮੁੱਲ_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  ਵਾਪਸ rc;

no_mem:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#ਜੇ_ਅੰਤ
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  ਪ੍ਰਤਿਜ੍ਞਾ( *ppVal==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( pCtx==0 ) sqlite3_ਮੁੱਲ_Free(pVal);
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( pCtx==0 ); sqlite3_ਮੁੱਲ_Free(pVal);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_NOMEM_BKPT;
}

/*
** Create a new sqlite3_value object, containing the value of pExpr.
**
** This only works for very simple expressions that consist of one constant
** token (i.e. "5", "5.1", "'a string'"). If the expression can
** be converted directly into a value, then the value is allocated and
** a pointer written to *ppVal. The caller is responsible for deallocating
** the value by passing it to sqlite3ValueFree() later on. If the expression
** cannot be converted to a value, then *ppVal is set to NULL.
*/
ਪੂਰਨ sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(
  sqlite3 *db,              /* The database connection */
  ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_ਮੁੱਲ **ppVal     /* Write the new value here */
){
  ਪੂਰਨ rc = SQLITE_OK;
  sqlite3_ਮੁੱਲ *pVal = 0;
  ਜੇ( p_ਵਿਅੰਜਕ ){
    rc = ਮੁੱਲ_From_ਵਿਅੰਜਕ(db, p_ਵਿਅੰਜਕ, enc, affinity, &pVal, 0);
    ਜੇ( rc==SQLITE_OK && pVal 
     && affinity==SQLITE_AFF_REAL 
     && (pVal->flags & MEM_Int) 
     && (pVal->u.i<-140737488355328LL || pVal->u.i>140737488355327LL)
    ){
      /* If the integer value is too large to fit in a 6-byte integer and
      ** the affinity is REAL, convert it to a real value now. In most
      ** cases an OP_RealAffinity opcode will be used to convert the 
      ** value to an actual real, but this opcode is omitted if the values
      ** are being read directly from a table in order to create an index
      ** key. It is important to get a "real" real value for the larger 
      ** magnitude integer values so that comparisons work correctly -
      ** SQLite by default will convert real values to integers before
      ** doing the comparison, which is different from converting to real
      ** first.  */
      sqlite3VdbeMemRealify(pVal);
    }
  }
  *ppVal = pVal;
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
/*
** Attempt to extract a value from pExpr and use it to construct *ppVal.
**
** If pAlloc is not NULL, then an UnpackedRecord object is created for
** pAlloc if one does not exist and the new value is added to the
** UnpackedRecord object.
**
** A value is extracted in the following cases:
**
**  * (pExpr==0). In this case the value is assumed to be an SQL NULL,
**
**  * The expression is a bound variable, and this is a reprepare, or
**
**  * The expression is a literal value.
**
** On success, *ppVal is made to point to the extracted value.  The caller
** is responsible for ensuring that the value is eventually freed.
*/
ਸਥਿਰ ਪੂਰਨ stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(
  Parse *pParse,                  /* Parse context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  ਬਣਤਰ ਮੁੱਲ_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_ਮੁੱਲ **ppVal           /* OUT: New value object (or NULL) */
){
  ਪੂਰਨ rc = SQLITE_OK;
  sqlite3_ਮੁੱਲ *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate(p_ਵਿਅੰਜਕ);

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ==0 || p_ਵਿਅੰਜਕ->op!=TK_ਰਜਿਸਟਰ || p_ਵਿਅੰਜਕ->op2!=TK_VARIABLE );
  ਜੇ( !p_ਵਿਅੰਜਕ ){
    pVal = ਮੁੱਲ_New(db, pAlloc);
    ਜੇ( pVal ){
      sqlite3_Vdbe_Mem_ਸੈੱਟ_Null((Mem*)pVal);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    ਪੂਰਨ iBindVar = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    ਜੇ( (v = pParse->pReprepare)!=0 ){
      pVal = ਮੁੱਲ_New(db, pAlloc);
      ਜੇ( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_ਮੁੱਲ_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }ਨਹੀਂਤਾਂ{
    rc = ਮੁੱਲ_From_ਵਿਅੰਜਕ(db, p_ਵਿਅੰਜਕ, ENC(db), affinity, &pVal, pAlloc);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  ਵਾਪਸ rc;
}

/*
** This function is used to allocate and populate UnpackedRecord 
** structures intended to be compared against sample index keys stored 
** in the sqlite_stat4 table.
**
** A single call to this function populates zero or more fields of the
** record starting with field iVal (fields are numbered from left to
** right starting with 0). A single field is populated if:
**
**  * (pExpr==0). In this case the value is assumed to be an SQL NULL,
**
**  * The expression is a bound variable, and this is a reprepare, or
**
**  * The sqlite3ValueFromExpr() function is able to extract a value 
**    from the expression (i.e. the expression is a literal value).
**
** Or, if pExpr is a TK_VECTOR, one field is populated for each of the
** vector components that match either of the two latter criteria listed
** above.
**
** Before any value is appended to the record, the affinity of the 
** corresponding column within index pIdx is applied to it. Before
** this function returns, output parameter *pnExtract is set to the
** number of values appended to the record.
**
** When this function is called, *ppRec must either point to an object
** allocated by an earlier call to this function, or must be NULL. If it
** is NULL and a value can be successfully extracted, a new UnpackedRecord
** is allocated (and *ppRec set to point to it) before returning.
**
** Unless an error is encountered, SQLITE_OK is returned. It is not an
** error if a value cannot be extracted from pExpr. If an error does
** occur, an SQLite error code is returned.
*/
ਪੂਰਨ sqlite3_Stat4_Probe_ਸੈੱਟ_ਮੁੱਲ(
  Parse *pParse,                  /* Parse context */
  ਸੂਚਕਾਂਕ *pIdx,                    /* Index being probed */
  Unpacked_ਰਿਕਾਰਡ **ppRec,         /* IN/OUT: Probe record */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,                    /* The expression to extract a value from */
  ਪੂਰਨ nElem,                      /* Maximum number of values to append */
  ਪੂਰਨ iVal,                       /* Array element to populate */
  ਪੂਰਨ *pn_ਕੱਢੋ                  /* OUT: Values appended to the record */
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ n_ਕੱਢੋ = 0;

  ਜੇ( p_ਵਿਅੰਜਕ==0 || p_ਵਿਅੰਜਕ->op!=TK_SELECT ){
    ਪੂਰਨ i;
    ਬਣਤਰ ਮੁੱਲ_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    ਲਈ(i=0; i<nElem; i++){
      sqlite3_ਮੁੱਲ *pVal = 0;
      ਵਿਅੰਜਕ *pElem = (p_ਵਿਅੰਜਕ ? sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਵਿਅੰਜਕ, i) : 0);
      u8 aff = sqlite3_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Affinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(pParse, pElem, aff, &alloc, &pVal);
      ਜੇ( !pVal ) ਰੋਕੋ;
      n_ਕੱਢੋ++;
    }
  }

  *pn_ਕੱਢੋ = n_ਕੱਢੋ;
  ਵਾਪਸ rc;
}

/*
** Attempt to extract a value from expression pExpr using the methods
** as described for sqlite3Stat4ProbeSetValue() above. 
**
** If successful, set *ppVal to point to a new value object and return 
** SQLITE_OK. If no value can be extracted, but no other error occurs
** (e.g. OOM), return SQLITE_OK and set *ppVal to NULL. Or, if an error
** does occur, return an SQLite error code. The final value of *ppVal
** is undefined in this case.
*/
ਪੂਰਨ sqlite3_Stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(
  Parse *pParse,                  /* Parse context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_ਮੁੱਲ **ppVal           /* OUT: New value object (or NULL) */
){
  ਵਾਪਸ stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(pParse, p_ਵਿਅੰਜਕ, affinity, 0, ppVal);
}

/*
** Extract the iCol-th column from the nRec-byte record in pRec.  Write
** the column value into *ppVal.  If *ppVal is initially NULL then a new
** sqlite3_value object is allocated.
**
** If *ppVal is initially NULL then the caller is responsible for 
** ensuring that the value written into *ppVal is eventually freed.
**
** If the buffer does not contain a well-formed record, this routine may
** read several bytes past the end of the buffer. Callers must therefore
** ensure that any buffer which may contain a corrupt record is padded
** with at least 8 bytes of addressable memory.
*/
ਪੂਰਨ sqlite3_Stat4_ਕਾਲਮ(
  sqlite3 *db,                    /* Database handle */
  ਅਚਲ ਖਾਲੀ *pRec,               /* Pointer to buffer containing record */
  ਪੂਰਨ nRec,                       /* Size of buffer pRec in bytes */
  ਪੂਰਨ i_ਖਾਨਾ,                       /* Column to extract */
  sqlite3_ਮੁੱਲ **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 i_ਖੇਤਰ;                     /* Next unread data byte */
  u32 sz_ਖੇਤਰ = 0;                /* Size of the current data field */
  ਪੂਰਨ i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>0 );
  iHdr = getVarint32(a, nHdr);
  ਜੇ( nHdr>(u32)nRec || iHdr>=nHdr ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  i_ਖੇਤਰ = nHdr;
  ਲਈ(i=0; i<=i_ਖਾਨਾ; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    ਜੇ( iHdr>nHdr ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    sz_ਖੇਤਰ = sqlite3VdbeSerialTypeLen(t);
    i_ਖੇਤਰ += sz_ਖੇਤਰ;
  }
  testcase( i_ਖੇਤਰ==nRec );
  testcase( i_ਖੇਤਰ==nRec+1 );
  ਜੇ( i_ਖੇਤਰ>nRec ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  ਜੇ( pMem==0 ){
    pMem = *ppVal = sqlite3_ਮੁੱਲ_New(db);
    ਜੇ( pMem==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[i_ਖੇਤਰ-sz_ਖੇਤਰ], t, pMem);
  pMem->enc = ENC(db);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
ਖਾਲੀ sqlite3Stat4ProbeFree(Unpacked_ਰਿਕਾਰਡ *pRec){
  ਜੇ( pRec ){
    ਪੂਰਨ i;
    ਪੂਰਨ n_ਖਾਨਾ = pRec->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_ਕੁੰਜੀ_Info_Unref(pRec->p_ਕੁੰਜੀ_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#ਜੇ_ਅੰਤ /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
ਖਾਲੀ sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(
  sqlite3_ਮੁੱਲ *v,     /* Value to be set */
  ਪੂਰਨ n,                /* Length of string z */
  ਅਚਲ ਖਾਲੀ *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)   /* Destructor for the string */
){
  ਜੇ( v ) sqlite3_Vdbe_Mem_ਸੈੱਟ_Str((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
ਖਾਲੀ sqlite3_ਮੁੱਲ_Free(sqlite3_ਮੁੱਲ *v){
  ਜੇ( !v ) ਵਾਪਸ;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਮੁੱਲ_ਬਾਈਟਾਂ_2(sqlite3_ਮੁੱਲ *pVal, u8 enc){
  ਵਾਪਸ ਮੁੱਲ_ਮੰਜ਼ਿਲ_ਪਾਠ(pVal, enc)!=0 ? pVal->n : 0;
}
ਪੂਰਨ sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ_2(sqlite3_ਮੁੱਲ *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  ਜੇ( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    ਵਾਪਸ p->n;
  }
  ਜੇ( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    ਵਾਪਸ p->n;
  }
  ਜੇ( (p->flags & MEM_Blob)!=0 ){
    ਜੇ( p->flags & MEM_Zero ){
      ਵਾਪਸ p->n + p->u.nZero;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ p->n;
    }
  }
  ਜੇ( p->flags & MEM_Null ) ਵਾਪਸ 0;
  ਵਾਪਸ ਮੁੱਲ_ਬਾਈਟਾਂ_2(pVal, enc);
}
