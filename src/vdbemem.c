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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "vdbeInt.h"

/* True if X is a power of two.  0 is considered a power of two here.
** In other words, return true if X has at most one bit set.
*/
#define ISPOWEROF2(X)  (((X)&((X)-1))==0)

#ifdef SQLITE_DEBUG
/*
** Check invariants on a Mem object.
**
** This routine is intended for use inside of assert() statements, like
** this:    assert( sqlite3VdbeCheckMemInvariants(pMem) );
*/
முழு sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  ப்ரதிஜ்ஞா( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  ப்ரதிஜ்ஞா( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  ப்ரதிஜ்ஞா( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  எனில்( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    ப்ரதிஜ்ஞா( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    எனில்( (p->flags & (MEM_Term|MEM_Subtype))==(MEM_Term|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      ப்ரதிஜ்ஞா( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      ப்ரதிஜ்ஞா( (p->flags & ~(MEM_Null|MEM_Term|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }இல்லையெனில்{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }இல்லையெனில்{
    /* The MEM_Cleared bit is only allowed on NULLs */
    ப்ரதிஜ்ஞா( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  ப்ரதிஜ்ஞா( p->szMalloc==0
       || (p->flags==MEM_Undefined 
           && p->szMalloc<=sqlite3DbMallocSize(p->db,p->zMalloc))
       || p->szMalloc==sqlite3DbMallocSize(p->db,p->zMalloc));

  /* If p holds a string or blob, the Mem.z must point to exactly
  ** one of the following:
  **
  **   (1) Memory in Mem.zMalloc and managed by the Mem object
  **   (2) Memory to be freed using Mem.xDel
  **   (3) An ephemeral string or blob
  **   (4) A static string or blob
  */
  எனில்( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    ப்ரதிஜ்ஞா( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  திரும்பு 1;
}
#endif

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
நிலையான வெறுமை vdbe_Mem_Render_இலக்கம்(முழு sz, எழுத்து *zBuf, Mem *p){
  StrAccum acc;
  ப்ரதிஜ்ஞா( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  ப்ரதிஜ்ஞா( sz>22 );
  எனில்( p->flags & (MEM_Int|MEM_IntReal) ){
#எனில் GCC_VERSION>=7000000 && GCC_VERSION<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    ப்ரதிஜ்ஞா( (sqlite3Config.bSmallMalloc!=0xee)*8==அளவு(x) );
    memcpy(&x, (எழுத்து*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3Int64ToText(x, zBuf);
#இல்லையெனில்
    p->n = sqlite3Int64ToText(p->u.i, zBuf);
#endif
    எனில்( p->flags & MEM_IntReal ){
      memcpy(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }இல்லையெனில்{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    ப்ரதிஜ்ஞா( acc.zText==zBuf && acc.mxAlloc<=0 );
    zBuf[acc.nChar] = 0; /* Fast version of sqlite3StrAccumFinish(&acc) */
    p->n = acc.nChar;
  }
}

#ifdef SQLITE_DEBUG
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
முழு sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  எழுத்து zBuf[100];
  எழுத்து *z;
  முழு i, j, incr;
  எனில்( (p->flags & MEM_Str)==0 ) திரும்பு 1;
  எனில்( p->db && p->db->mallocFailed ) திரும்பு 1;
  எனில்( p->flags & MEM_Term ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    எனில்( p->szMalloc>0 && p->z==p->zMalloc ){
      ப்ரதிஜ்ஞா( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      ப்ரதிஜ்ஞா( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    ப்ரதிஜ்ஞா( p->z[p->n]==0 );
    ப்ரதிஜ்ஞா( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    ப்ரதிஜ்ஞா( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  எனில்( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) திரும்பு 1;
  எனில்( p->db==0 ){
    திரும்பு 1;  /* db->nFpDigit required to validate p->z[] */
  }
  memcpy(&tmp, p, அளவு(tmp));
  vdbe_Mem_Render_இலக்கம்(அளவு(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  எனில்( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    எனில்( p->enc==SQLITE_UTF16BE ) z++;
  }
  போது( zBuf[j] ){
    எனில்( zBuf[j++]!=z[i] ) திரும்பு 0;
    i += incr;
  }
  திரும்பு 1;
}
#endif /* SQLITE_DEBUG */

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
முழு sqlite3VdbeChangeEncoding(Mem *pMem, முழு desiredEnc){
#ifndef SQLITE_OMIT_UTF16
  முழு rc;
#endif
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  ப்ரதிஜ்ஞா( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  எனில்( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    திரும்பு SQLITE_OK;
  }
  எனில்( pMem->enc==desiredEnc ){
    திரும்பு SQLITE_OK;
  }
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ifdef SQLITE_OMIT_UTF16
  திரும்பு SQLITE_ERROR;
#இல்லையெனில்

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  ப்ரதிஜ்ஞா(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  ப்ரதிஜ்ஞா(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  ப்ரதிஜ்ஞா(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  திரும்பு rc;
#endif
}

/*
** Make sure pMem->z points to a writable allocation of at least n bytes.
**
** If the bPreserve argument is true, then copy of the content of
** pMem->z into the new allocation.  pMem must be either a string or
** blob if bPreserve is true.  If bPreserve is false, any prior content
** in pMem->z is discarded.
*/
SQLITE_NOINLINE முழு sqlite3VdbeMemGrow(Mem *pMem, முழு n, முழு bPreserve){
  ப்ரதிஜ்ஞா( sqlite3VdbeCheckMemInvariants(pMem) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  ப்ரதிஜ்ஞா( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  ப்ரதிஜ்ஞா( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3DbMallocSize(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3DbMallocSize(pMem->db,pMem->zMalloc));
  எனில்( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    எனில்( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }இல்லையெனில்{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      எனில்( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }இல்லையெனில்{
    எனில்( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  எனில்( pMem->zMalloc==0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    திரும்பு SQLITE_NOMEM_BKPT;
  }இல்லையெனில்{
    pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
  }

  எனில்( bPreserve && pMem->z ){
    ப்ரதிஜ்ஞா( pMem->z!=pMem->zMalloc );
    memcpy(pMem->zMalloc, pMem->z, pMem->n);
  }
  எனில்( (pMem->flags&MEM_Dyn)!=0 ){
    ப்ரதிஜ்ஞா( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((வெறுமை *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  திரும்பு SQLITE_OK;
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
முழு sqlite3VdbeMemClearAndResize(Mem *pMem, முழு szNew){
  ப்ரதிஜ்ஞா( CORRUPT_DB || szNew>0 );
  ப்ரதிஜ்ஞா( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  எனில்( pMem->szMalloc<szNew ){
    திரும்பு sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  ப்ரதிஜ்ஞா( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  திரும்பு SQLITE_OK;
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
முழு sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  எனில்( (pMem->flags & (MEM_Str|MEM_Term|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    திரும்பு 0;
  }
  எனில்( pMem->enc!=SQLITE_UTF8 ) திரும்பு 0;
  ப்ரதிஜ்ஞா( pMem->z!=0 );
  எனில்( pMem->flags & MEM_Dyn ){
    எனில்( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_Term;
      திரும்பு 1;
    }
    எனில்( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_Term;
      திரும்பு 1;
    }
  }இல்லையெனில் எனில்( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_Term;
    திரும்பு 1;
  }
  திரும்பு 0;
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
நிலையான SQLITE_NOINLINE முழு vdbeMemAddTerminator(Mem *pMem){
  எனில்( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_Term;
  திரும்பு SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
முழு sqlite3VdbeMemMakeWriteable(Mem *pMem){
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  எனில்( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    எனில்( ExpandBlob(pMem) ) திரும்பு SQLITE_NOMEM;
    எனில்( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      முழு rc = vdbeMemAddTerminator(pMem);
      எனில்( rc ) திரும்பு rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ifdef SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#endif

  திரும்பு SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ifndef SQLITE_OMIT_INCRBLOB
முழு sqlite3VdbeMemExpandBlob(Mem *pMem){
  முழு nByte;
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->flags & MEM_Zero );
  ப்ரதிஜ்ஞா( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_மதிப்பு_nochange(pMem) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  எனில்( nByte<=0 ){
    எனில்( (pMem->flags & MEM_Blob)==0 ) திரும்பு SQLITE_OK;
    nByte = 1;
  }
  எனில்( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  ப்ரதிஜ்ஞா( pMem->z!=0 );
  ப்ரதிஜ்ஞா( sqlite3DbMallocSize(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_Term);
  திரும்பு SQLITE_OK;
}
#endif

/*
** Make sure the given Mem is \u0000 terminated.
*/
முழு sqlite3VdbeMemNulTerminate(Mem *pMem){
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==(MEM_Term|MEM_Str) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==0 );
  எனில்( (pMem->flags & (MEM_Term|MEM_Str))!=MEM_Str ){
    திரும்பு SQLITE_OK;   /* Nothing to do */
  }இல்லையெனில்{
    திரும்பு vdbeMemAddTerminator(pMem);
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
முழு sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  மாறிலி முழு nByte = 32;

  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( !(pMem->flags&MEM_Zero) );
  ப்ரதிஜ்ஞா( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  ப்ரதிஜ்ஞா( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );


  எனில்( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    திரும்பு SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_இலக்கம்(nByte, pMem->z, pMem);
  ப்ரதிஜ்ஞா( pMem->z!=0 );
  ப்ரதிஜ்ஞா( pMem->n==(முழு)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_Term;
  எனில்( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  திரும்பு SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
முழு sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  ப்ரதிஜ்ஞா( pFunc!=0 );
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db!=0 );
  ப்ரதிஜ்ஞா( pFunc->xFinalize!=0 );
  ப்ரதிஜ்ஞா( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, அளவு(ctx));
  memset(&t, 0, அளவு(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.pOut = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  ப்ரதிஜ்ஞா( (pMem->flags & MEM_Dyn)==0 );
  எனில்( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  memcpy(pMem, &t, அளவு(t));
  திரும்பு ctx.isError;
}

/*
** Memory cell pAccum contains the context of an aggregate function.
** This routine calls the xValue method for that function and stores
** the results in memory cell pMem.
**
** SQLITE_ERROR is returned if xValue() reports an error. SQLITE_OK 
** otherwise.
*/
#ifndef SQLITE_OMIT_WINDOWFUNC
முழு sqlite3_Vdbe_Mem_Agg_மதிப்பு(Mem *pAccum, Mem *pOut, FuncDef *pFunc){
  sqlite3_context ctx;
  ப்ரதிஜ்ஞா( pFunc!=0 );
  ப்ரதிஜ்ஞா( pFunc->x_மதிப்பு!=0 );
  ப்ரதிஜ்ஞா( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  ப்ரதிஜ்ஞா( pAccum->db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, அளவு(ctx));
  sqlite3VdbeMemSetNull(pOut);
  ctx.pOut = pOut;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_மதிப்பு(&ctx);
  திரும்பு ctx.isError;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** If the memory cell contains a value that must be freed by
** invoking the external callback in Mem.xDel, then this routine
** will free that value.  It also sets Mem.flags to MEM_Null.
**
** This is a helper routine for sqlite3VdbeMemSetNull() and
** for sqlite3VdbeMemRelease().  Use those other routines as the
** entry point for releasing Mem resources.
*/
நிலையான SQLITE_NOINLINE வெறுமை vdbeMemClearExternAndSetNull(Mem *p){
  ப்ரதிஜ்ஞா( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  ப்ரதிஜ்ஞா( VdbeMemDynamic(p) );
  எனில்( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    ப்ரதிஜ்ஞா( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  எனில்( p->flags&MEM_Dyn ){
    ப்ரதிஜ்ஞா( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((வெறுமை *)p->z);
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
நிலையான SQLITE_NOINLINE வெறுமை vdbeMemClear(Mem *p){
  எனில்( VdbeMemDynamic(p) ){
    vdbeMemClearExternAndSetNull(p);
  }
  எனில்( p->szMalloc ){
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
வெறுமை sqlite3VdbeMemRelease(Mem *p){
  ப்ரதிஜ்ஞா( sqlite3VdbeCheckMemInvariants(p) );
  எனில்( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
வெறுமை sqlite3VdbeMemReleaseMalloc(Mem *p){
  ப்ரதிஜ்ஞா( !VdbeMemDynamic(p) );
  எனில்( p->szMalloc ) vdbeMemClear(p);
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
நிலையான SQLITE_NOINLINE i64 mem_Int_மதிப்பு(மாறிலி Mem *pMem){
  i64 மதிப்பு = 0;
  sqlite3Atoi64(pMem->z, &மதிப்பு, pMem->n, pMem->enc);
  திரும்பு மதிப்பு;
}
i64 sqlite3_Vdbe_Int_மதிப்பு(மாறிலி Mem *pMem){
  முழு flags;
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  எனில்( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    திரும்பு pMem->u.i;
  }இல்லையெனில் எனில்( flags & MEM_Real ){
    திரும்பு sqlite3RealToI64(pMem->u.r);
  }இல்லையெனில் எனில்( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    திரும்பு mem_Int_மதிப்பு(pMem);
  }இல்லையெனில்{
    திரும்பு 0;
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
நிலையான SQLITE_NOINLINE முழு sqlite3_Mem_Real_மதிப்பு_RCSlow_Path(
  Mem *pMem,
  இரட்டை *p_மதிப்பு
){
  முழு rc = SQLITE_OK;
  *p_மதிப்பு = 0.0;
  எனில்( pMem->enc==SQLITE_UTF8 ){
    எழுத்து *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    எனில்( zCopy ){
      rc = sqlite3AtoF(zCopy, p_மதிப்பு);
      sqlite3DbFree(pMem->db, zCopy);
    }
    திரும்பு rc;
  }இல்லையெனில்{
    முழு n, i, j;
    எழுத்து *zCopy;
    மாறிலி எழுத்து *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    எனில்( zCopy ){
      z = pMem->z;
      எனில்( pMem->enc==SQLITE_UTF16LE ){
        க்கு(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          எனில்( z[i+1]!=0 ) நிறுத்து;
        }
      }இல்லையெனில்{
        க்கு(i=j=0; i<n-1; i+=2, j++){
          எனில்( z[i]!=0 ) நிறுத்து;
          zCopy[j] = z[i+1];
        }
      }     
      ப்ரதிஜ்ஞா( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_மதிப்பு);
      எனில்( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    திரும்பு rc;
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
முழு sqlite3_Mem_Real_மதிப்பு_RC(Mem *pMem, இரட்டை *p_மதிப்பு){
  testcase( pMem->db==0 );
  ப்ரதிஜ்ஞா( pMem->flags & (MEM_Str|MEM_Blob) );
  எனில்( pMem->z==0 ){
    *p_மதிப்பு = 0.0;
    திரும்பு 0;
  }இல்லையெனில் எனில்( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_Term)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    திரும்பு sqlite3AtoF(pMem->z, p_மதிப்பு);
  }இல்லையெனில் எனில்( pMem->n==0 ){
    *p_மதிப்பு = 0.0;
    திரும்பு 0;
  }இல்லையெனில்{
    திரும்பு sqlite3_Mem_Real_மதிப்பு_RCSlow_Path(pMem, p_மதிப்பு);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
நிலையான SQLITE_NOINLINE இரட்டை sqlite3_Mem_Real_மதிப்பு_No_RC(Mem *pMem){
  இரட்டை r;
  (வெறுமை)sqlite3_Mem_Real_மதிப்பு_RC(pMem, &r);
  திரும்பு r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
இரட்டை sqlite3_Vdbe_Real_மதிப்பு(Mem *pMem){
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );
  எனில்( pMem->flags & MEM_Real ){
    திரும்பு pMem->u.r;
  }இல்லையெனில் எனில்( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    திரும்பு (இரட்டை)pMem->u.i;
  }இல்லையெனில் எனில்( pMem->flags & (MEM_Str|MEM_Blob) ){
    திரும்பு sqlite3_Mem_Real_மதிப்பு_No_RC(pMem);
  }இல்லையெனில்{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    திரும்பு (இரட்டை)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
முழு sqlite3_Vdbe_Boolean_மதிப்பு(Mem *pMem, முழு ifNull){
  testcase( pMem->flags & MEM_IntReal );
  எனில்( pMem->flags & (MEM_Int|MEM_IntReal) ) திரும்பு pMem->u.i!=0;
  எனில்( pMem->flags & MEM_Null ) திரும்பு ifNull;
  திரும்பு sqlite3_Vdbe_Real_மதிப்பு(pMem)!=0.0;
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
வெறுமை sqlite3VdbeIntegerAffinity(Mem *pMem, முழு bFromText){
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->flags & (MEM_Real|MEM_IntReal) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );

  எனில்( pMem->flags & MEM_IntReal ){
    MemSetTypeFlag(pMem, MEM_Int);
  }இல்லையெனில்{
    i64 ix = sqlite3RealToI64(pMem->u.r);

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
    எனில்( pMem->u.r==ix && ix<LARGEST_INT64
     && (!bFromText || ix>SMALLEST_INT64)
    ){
      pMem->u.i = ix;
      MemSetTypeFlag(pMem, MEM_Int);
    }
  }
}

/*
** Convert pMem to type integer.  Invalidate any prior representations.
*/
முழு sqlite3VdbeMemIntegerify(Mem *pMem){
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_மதிப்பு(pMem);
  MemSetTypeFlag(pMem, MEM_Int);
  திரும்பு SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
முழு sqlite3VdbeMemRealify(Mem *pMem){
  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_மதிப்பு(pMem);
  MemSetTypeFlag(pMem, MEM_Real);
  திரும்பு SQLITE_OK;
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
முழு sqlite3RealSameAsInt(இரட்டை r1, sqlite3_int64 i){
  இரட்டை r2 = (இரட்டை)i;
  திரும்பு r1==0.0
      || (memcmp(&r1, &r2, அளவு(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3RealToI64(இரட்டை r){
  எனில்( r<-9223372036854774784.0 ) திரும்பு SMALLEST_INT64;
  எனில்( r>+9223372036854774784.0 ) திரும்பு LARGEST_INT64;
  திரும்பு (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
முழு sqlite3VdbeMemNumerify(Mem *pMem){
  ப்ரதிஜ்ஞா( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  எனில்( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    முழு rc;
    sqlite3_int64 ix;
    ப்ரதிஜ்ஞா( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_மதிப்பு_RC(pMem, &pMem->u.r);
    எனில்( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3RealToI64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      MemSetTypeFlag(pMem, MEM_Int);
    }இல்லையெனில்{
      MemSetTypeFlag(pMem, MEM_Real);
    }
  }
  ப்ரதிஜ்ஞா( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  திரும்பு SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
முழு sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  எனில்( pMem->flags & MEM_Null ) திரும்பு SQLITE_OK;
  தேர்வு( aff ){
    நிலை SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      எனில்( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_மதிப்பு_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
        ப்ரதிஜ்ஞா( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        எனில்( pMem->flags & MEM_Str ) MemSetTypeFlag(pMem, MEM_Blob);
      }இல்லையெனில்{
        pMem->flags &= ~(MEM_TypeMask&~MEM_Blob);
      }
      நிறுத்து;
    }
    நிலை SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      நிறுத்து;
    }
    நிலை SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      நிறுத்து;
    }
    நிலை SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      நிறுத்து;
    }
    இயல்பு: {
      முழு rc;
      ப்ரதிஜ்ஞா( aff==SQLITE_AFF_TEXT );
      ப்ரதிஜ்ஞா( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_மதிப்பு_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
      ப்ரதிஜ்ஞா( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      எனில்( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      எனில்( rc ) திரும்பு rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  திரும்பு SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
வெறுமை sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  ப்ரதிஜ்ஞா( (flags & ~MEM_TypeMask)==0 );
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
வெறுமை sqlite3VdbeMemSetNull(Mem *pMem){
  எனில்( VdbeMemDynamic(pMem) ){
    vdbeMemClearExternAndSetNull(pMem);
  }இல்லையெனில்{
    pMem->flags = MEM_Null;
  }
}
வெறுமை sqlite3_மதிப்பு_Set_Null(sqlite3_மதிப்பு *p){
  sqlite3VdbeMemSetNull((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ifndef SQLITE_OMIT_INCRBLOB
வெறுமை sqlite3VdbeMemSetZeroBlob(Mem *pMem, முழு n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  எனில்( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#இல்லையெனில்
முழு sqlite3VdbeMemSetZeroBlob(Mem *pMem, முழு n){
  முழு nByte = n>0?n:1;
  எனில்( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  ப்ரதிஜ்ஞா( pMem->z!=0 );
  ப்ரதிஜ்ஞா( sqlite3DbMallocSize(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  திரும்பு SQLITE_OK;
}
#endif

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
நிலையான SQLITE_NOINLINE வெறுமை vdbeReleaseAndSetInt64(Mem *pMem, i64 val){
  sqlite3VdbeMemSetNull(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
வெறுமை sqlite3VdbeMemSetInt64(Mem *pMem, i64 val){
  எனில்( VdbeMemDynamic(pMem) ){
    vdbeReleaseAndSetInt64(pMem, val);
  }இல்லையெனில்{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
வெறுமை sqlite3MemSetArrayInt64(sqlite3_மதிப்பு *aMem, முழு iIdx, i64 val){
  sqlite3VdbeMemSetInt64(&aMem[iIdx], val);
}

/* A no-op destructor */
வெறுமை sqlite3NoopDestructor(வெறுமை *p){ UNUSED_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
வெறுமை sqlite3VdbeMemSetPointer(
  Mem *pMem,
  வெறுமை *pPtr,
  மாறிலி எழுத்து *zPType,
  வெறுமை (*xDestructor)(வெறுமை*)
){
  ப்ரதிஜ்ஞா( pMem->flags==MEM_Null );
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = MEM_Null|MEM_Dyn|MEM_Subtype|MEM_Term;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type REAL.
*/
வெறுமை sqlite3VdbeMemSetDouble(Mem *pMem, இரட்டை val){
  sqlite3VdbeMemSetNull(pMem);
  எனில்( !sqlite3IsNaN(val) ){
    pMem->u.r = val;
    pMem->flags = MEM_Real;
  }
}
#endif

#ifdef SQLITE_DEBUG
/*
** Return true if the Mem holds a RowSet object.  This routine is intended
** for use inside of assert() statements.
*/
முழு sqlite3VdbeMemIsRowSet(மாறிலி Mem *pMem){
  திரும்பு (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
         && pMem->xDel==sqlite3RowSetDelete;
}
#endif

/*
** Delete any previous value and set the value of pMem to be an
** empty boolean index.
**
** Return SQLITE_OK on success and SQLITE_NOMEM if a memory allocation
** error occurs.
*/
முழு sqlite3VdbeMemSetRowSet(Mem *pMem){
  sqlite3 *db = pMem->db;
  RowSet *p;
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3RowSetInit(db);
  எனில்( p==0 ) திரும்பு SQLITE_NOMEM;
  pMem->z = (எழுத்து*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3RowSetDelete;
  திரும்பு SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
முழு sqlite3VdbeMemTooBig(Mem *p){
  ப்ரதிஜ்ஞா( p->db!=0 );
  எனில்( p->flags & (MEM_Str|MEM_Blob) ){
    முழு n = p->n;
    எனில்( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    திரும்பு n>p->db->aLimit[SQLITE_LIMIT_LENGTH];
  }
  திரும்பு 0; 
}

#ifdef SQLITE_DEBUG
/*
** This routine prepares a memory cell for modification by breaking
** its link to a shallow copy and by marking any current shallow
** copies of this cell as invalid.
**
** This is used for testing and debugging only - to help ensure that shallow
** copies (created by OP_SCopy) are not misused.
*/
வெறுமை sqlite3VdbeMemAboutToChange(Vdbe *pVdbe, Mem *pMem){
  முழு i;
  Mem *pX;
  எனில்( pMem->bScopy ){
    க்கு(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      எனில்( pX->pScopyFrom==pMem ){
        u16 mFlags;
        எனில்( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (முழு)(pX - pVdbe->aMem), (முழு)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        ப்ரதிஜ்ஞா( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
#endif /* SQLITE_DEBUG */

/*
** Make an shallow copy of pFrom into pTo.  Prior contents of
** pTo are freed.  The pFrom->z field is not duplicated.  If
** pFrom->z is used, then pTo->z points to the same thing as pFrom->z
** and flags gets srcType (either MEM_Ephem or MEM_Static).
*/
நிலையான SQLITE_NOINLINE வெறுமை vdbeClrCopy(Mem *pTo, மாறிலி Mem *pFrom, முழு eType){
  vdbeMemClearExternAndSetNull(pTo);
  ப்ரதிஜ்ஞா( !VdbeMemDynamic(pTo) );
  sqlite3VdbeMemShallowCopy(pTo, pFrom, eType);
}
வெறுமை sqlite3VdbeMemShallowCopy(Mem *pTo, மாறிலி Mem *pFrom, முழு srcType){
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pFrom) );
  ப்ரதிஜ்ஞா( pTo->db==pFrom->db );
  எனில்( VdbeMemDynamic(pTo) ){ vdbeClrCopy(pTo,pFrom,srcType); திரும்பு; }
  memcpy(pTo, pFrom, MEMCELLSIZE);
  எனில்( (pFrom->flags&MEM_Static)==0 ){
    pTo->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    ப்ரதிஜ்ஞா( srcType==MEM_Ephem || srcType==MEM_Static );
    pTo->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
முழு sqlite3VdbeMemCopy(Mem *pTo, மாறிலி Mem *pFrom){
  முழு rc = SQLITE_OK;

  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pFrom) );
  எனில்( VdbeMemDynamic(pTo) ) vdbeMemClearExternAndSetNull(pTo);
  memcpy(pTo, pFrom, MEMCELLSIZE);
  pTo->flags &= ~MEM_Dyn;
  எனில்( pTo->flags&(MEM_Str|MEM_Blob) ){
    எனில்( 0==(pFrom->flags&MEM_Static) ){
      pTo->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(pTo);
    }
  }

  திரும்பு rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
வெறுமை sqlite3VdbeMemMove(Mem *pTo, Mem *pFrom){
  ப்ரதிஜ்ஞா( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  ப்ரதிஜ்ஞா( pTo->db==0 || sqlite3_mutex_held(pTo->db->mutex) );
  ப்ரதிஜ்ஞா( pFrom->db==0 || pTo->db==0 || pFrom->db==pTo->db );

  sqlite3VdbeMemRelease(pTo);
  memcpy(pTo, pFrom, அளவு(Mem));
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
முழு sqlite3VdbeMemSetStr(
  Mem *pMem,          /* Memory cell to set to string value */
  மாறிலி எழுத்து *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  வெறுமை (*xDel)(வெறுமை*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  முழு iLimit;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  ப்ரதிஜ்ஞா( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  எனில்( !z ){
    sqlite3VdbeMemSetNull(pMem);
    திரும்பு SQLITE_OK;
  }

  எனில்( pMem->db ){
    iLimit = pMem->db->aLimit[SQLITE_LIMIT_LENGTH];
  }இல்லையெனில்{
    iLimit = SQLITE_MAX_LENGTH;
  }
  எனில்( nByte<0 ){
    ப்ரதிஜ்ஞா( enc!=0 );
    எனில்( enc==SQLITE_UTF8 ){
      nByte = strlen(z);
    }இல்லையெனில்{
      க்கு(nByte=0; nByte<=iLimit && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_Term;
  }இல்லையெனில் எனில்( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }இல்லையெனில்{
    flags = MEM_Str;
  }
  எனில்( nByte>iLimit ){
    எனில்( xDel && xDel!=SQLITE_TRANSIENT ){
      எனில்( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (வெறுமை*)z);
      }இல்லையெனில்{
        xDel((வெறுமை*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    திரும்பு sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  எனில்( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    எனில்( flags&MEM_Term ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    எனில்( sqlite3VdbeMemClearAndResize(pMem, (முழு)MAX(nAlloc,32)) ){
      திரும்பு SQLITE_NOMEM_BKPT;
    }
    ப்ரதிஜ்ஞா( pMem->z!=0 );
    memcpy(pMem->z, z, nAlloc);
  }இல்லையெனில்{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (எழுத்து *)z;
    எனில்( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
    }இல்லையெனில்{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (முழு)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ifndef SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  எனில்( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
#endif


  திரும்பு SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
முழு sqlite3VdbeMemSetText(
  Mem *pMem,          /* Memory cell to set to string value */
  மாறிலி எழுத்து *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  வெறுமை (*xDel)(வெறுமை*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  ப்ரதிஜ்ஞா( pMem!=0 );
  ப்ரதிஜ்ஞா( pMem->db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pMem->db->mutex) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  எனில்( !z ){
    sqlite3VdbeMemSetNull(pMem);
    திரும்பு SQLITE_OK;
  }

  எனில்( nByte<0 ){
    nByte = strlen(z);
    flags = MEM_Str|MEM_Term;
  }இல்லையெனில்{
    flags = MEM_Str;
  }
  எனில்( nByte>(i64)pMem->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    எனில்( xDel && xDel!=SQLITE_TRANSIENT ){
      எனில்( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (வெறுமை*)z);
      }இல்லையெனில்{
        xDel((வெறுமை*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    திரும்பு sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  எனில்( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    எனில்( sqlite3VdbeMemClearAndResize(pMem, (முழு)MAX(nAlloc,32)) ){
      திரும்பு SQLITE_NOMEM_BKPT;
    }
    ப்ரதிஜ்ஞா( pMem->z!=0 );
    memcpy(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }இல்லையெனில்{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (எழுத்து *)z;
    எனில்( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }இல்லையெனில் எனில்( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }இல்லையெனில்{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (முழு)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  திரும்பு SQLITE_OK;
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
முழு sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 offset,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  முழு rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE-1 );
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE );
  எனில்( amt>=SQLITE_MAX_ALLOCATION_SIZE ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  எனில்( (u64)amt + (u64)offset > (u64)sqlite3BtreeMaxRecordSize(pCur) ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  எனில்( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, offset, amt, pMem->z);
    எனில்( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (முழு)amt;
    }இல்லையெனில்{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  திரும்பு rc;
}
முழு sqlite3VdbeMemFromBtreeZeroOffset(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 available = 0;  /* Number of bytes available on the local btree page */
  முழு rc = SQLITE_OK; /* Return code */

  ப்ரதிஜ்ஞா( sqlite3BtreeCursorIsValid(pCur) );
  ப்ரதிஜ்ஞா( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem) );
  pMem->z = (எழுத்து *)sqlite3BtreePayloadFetch(pCur, &available);
  ப்ரதிஜ்ஞா( pMem->z!=0 );

  எனில்( amt<=available ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (முழு)amt;
  }இல்லையெனில்{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  திரும்பு rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
நிலையான SQLITE_NOINLINE மாறிலி வெறுமை *மதிப்பு_To_Text(sqlite3_மதிப்பு* pVal, u8 enc){
  ப்ரதிஜ்ஞா( pVal!=0 );
  ப்ரதிஜ்ஞா( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ப்ரதிஜ்ஞா( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pVal) );
  ப்ரதிஜ்ஞா( (pVal->flags & (MEM_Null))==0 );
  எனில்( pVal->flags & (MEM_Blob|MEM_Str) ){
    எனில்( ExpandBlob(pVal) ) திரும்பு 0;
    pVal->flags |= MEM_Str;
    எனில்( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    எனில்( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_TO_INT(pVal->z)) ){
      ப்ரதிஜ்ஞா( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      எனில்( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        திரும்பு 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }இல்லையெனில்{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    ப்ரதிஜ்ஞா( 0==(1&SQLITE_PTR_TO_INT(pVal->z)) );
  }
  ப்ரதிஜ்ஞா(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  எனில்( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    ப்ரதிஜ்ஞா( sqlite3VdbeMemValidStrRep(pVal) );
    திரும்பு pVal->z;
  }இல்லையெனில்{
    திரும்பு 0;
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
மாறிலி வெறுமை *sqlite3_மதிப்பு_Text(sqlite3_மதிப்பு* pVal, u8 enc){
  எனில்( !pVal ) திரும்பு 0;
  ப்ரதிஜ்ஞா( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ப்ரதிஜ்ஞா( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pVal) );
  எனில்( (pVal->flags&(MEM_Str|MEM_Term))==(MEM_Str|MEM_Term) && pVal->enc==enc ){
    ப்ரதிஜ்ஞா( sqlite3VdbeMemValidStrRep(pVal) );
    திரும்பு pVal->z;
  }
  எனில்( pVal->flags&MEM_Null ){
    திரும்பு 0;
  }
  திரும்பு மதிப்பு_To_Text(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
முழு sqlite3_மதிப்பு_Is_Of_Class(மாறிலி sqlite3_மதிப்பு *pVal, வெறுமை(*xFree)(வெறுமை*)){
  எனில்( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    திரும்பு 1;
  }இல்லையெனில்{
    திரும்பு 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_மதிப்பு *sqlite3_மதிப்பு_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, அளவு(*p));
  எனில்( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  திரும்பு p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
கட்டமைப்பு மதிப்பு_New_Stat4_Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  முழு iVal;
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
நிலையான sqlite3_மதிப்பு *மதிப்பு_New(sqlite3 *db, கட்டமைப்பு மதிப்பு_New_Stat4_Ctx *p){
#ifdef SQLITE_ENABLE_STAT4
  எனில்( p ){
    UnpackedRecord *pRec = p->ppRec[0];

    எனில்( pRec==0 ){
      Index *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      முழு i;                      /* Counter variable */
      முழு nCol = pIdx->nColumn;   /* Number of index columns including rowid */
  
      nByte = அளவு(Mem) * nCol + ROUND8(அளவு(UnpackedRecord));
      pRec = (UnpackedRecord*)sqlite3DbMallocZero(db, nByte);
      எனில்( pRec ){
        pRec->p_சாவி_Info = sqlite3_சாவி_Info_Of_Index(p->pParse, pIdx);
        எனில்( pRec->p_சாவி_Info ){
          ப்ரதிஜ்ஞா( pRec->p_சாவி_Info->nAllField==nCol );
          ப்ரதிஜ்ஞா( pRec->p_சாவி_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(அளவு(UnpackedRecord)));
          க்கு(i=0; i<nCol; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }இல்லையெனில்{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      எனில்( pRec==0 ) திரும்பு 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->nField = p->iVal+1;
    sqlite3VdbeMemSetNull(&pRec->aMem[p->iVal]);
    திரும்பு &pRec->aMem[p->iVal];
  }
#இல்லையெனில்
  UNUSED_PARAMETER(p);
#endif /* defined(SQLITE_ENABLE_STAT4) */
  திரும்பு sqlite3_மதிப்பு_New(db);
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
#ifdef SQLITE_ENABLE_STAT4
நிலையான முழு மதிப்பு_From_Function(
  sqlite3 *db,                    /* The database connection */
  மாறிலி கோவை *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_மதிப்பு **ppVal,          /* Write the new value here */
  கட்டமைப்பு மதிப்பு_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_மதிப்பு **apVal = 0;      /* Function arguments */
  முழு nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_மதிப்பு *pVal = 0;        /* New value */
  முழு rc = SQLITE_OK;             /* Return code */
  கோவை_List *pList = 0;            /* Function arguments */
  முழு i;                          /* Iterator variable */

  ப்ரதிஜ்ஞா( pCtx!=0 );
  ப்ரதிஜ்ஞா( (p->flags & EP_TokenOnly)==0 );
  ப்ரதிஜ்ஞா( கோவை_Use_XList(p) );
  pList = p->x.pList;
  எனில்( pList ) nVal = pList->n_கோவை;
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_Int_மதிப்பு) );
  pFunc = sqlite3FindFunction(db, p->u.zToken, nVal, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  எனில்( pFunc==0 ) திரும்பு SQLITE_OK;
#endif
  ப்ரதிஜ்ஞா( pFunc );
  எனில்( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    திரும்பு SQLITE_OK;
  }

  எனில்( pList ){
    apVal = (sqlite3_மதிப்பு**)sqlite3DbMallocZero(db, அளவு(apVal[0]) * nVal);
    எனில்( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      செல் மதிப்பு_from_function_out;
    }
    க்கு(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_மதிப்பு_From_கோவை(pCtx->pParse, pList->a[i].p_கோவை, aff,
                                     &apVal[i]);
      எனில்( apVal[i]==0 || rc!=SQLITE_OK ) செல் மதிப்பு_from_function_out;
    }
  }

  pVal = மதிப்பு_New(db, pCtx);
  எனில்( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    செல் மதிப்பு_from_function_out;
  }

  memset(&ctx, 0, அளவு(ctx));
  ctx.pOut = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  எனில்( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_செய்தி(pCtx->pParse, "%s", sqlite3_மதிப்பு_text(pVal));
  }இல்லையெனில்{
    sqlite3_மதிப்பு_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    ப்ரதிஜ்ஞா( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    எனில்( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 மதிப்பு_from_function_out:
  எனில்( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  எனில்( apVal ){
    க்கு(i=0; i<nVal; i++){
      sqlite3_மதிப்பு_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  திரும்பு rc;
}
#இல்லையெனில்
# define மதிப்பு_From_Function(a,b,c,d,e,f) SQLITE_OK
#endif /* defined(SQLITE_ENABLE_STAT4) */

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
நிலையான முழு மதிப்பு_From_கோவை(
  sqlite3 *db,                    /* The database connection */
  மாறிலி கோவை *p_கோவை,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_மதிப்பு **ppVal,          /* Write the new value here */
  கட்டமைப்பு மதிப்பு_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  முழு op;
  எழுத்து *zVal = 0;
  sqlite3_மதிப்பு *pVal = 0;
  முழு negInt = 1;
  மாறிலி எழுத்து *zNeg = "";
  முழு rc = SQLITE_OK;

  ப்ரதிஜ்ஞா( p_கோவை!=0 );
  போது( (op = p_கோவை->op)==TK_UPLUS || op==TK_SPAN ) p_கோவை = p_கோவை->p_இடது;
  எனில்( op==TK_REGISTER ) op = p_கோவை->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  ப்ரதிஜ்ஞா( (p_கோவை->flags & EP_TokenOnly)==0 || pCtx==0 );

  எனில்( op==TK_CAST ){
    u8 aff;
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
    aff = sqlite3AffinityType(p_கோவை->u.zToken,0);
    rc = மதிப்பு_From_கோவை(db, p_கோவை->p_இடது, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    எனில்( *ppVal ){
#ifdef SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#இல்லையெனில்
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      ப்ரதிஜ்ஞா( (ppVal[0][0].flags & MEM_Zero)==0 );
#endif
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_மதிப்பு_Apply_Affinity(*ppVal, affinity, enc);
    }
    திரும்பு rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  எனில்( op==TK_UMINUS ){
    கோவை *p_இடது = p_கோவை->p_இடது;
    எனில்( (p_இடது->op==TK_INTEGER || p_இடது->op==TK_FLOAT) ){
      எனில்( கோவை_Has_Property(p_இடது, EP_Int_மதிப்பு)
       || p_இடது->u.zToken[0]!='0' || (p_இடது->u.zToken[1] & ~0x20)!='X'
      ){
        p_கோவை = p_இடது;
        op = p_கோவை->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  எனில்( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = மதிப்பு_New(db, pCtx);
    எனில்( pVal==0 ) செல் no_mem;
    எனில்( கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) ){
      sqlite3VdbeMemSetInt64(pVal, (i64)p_கோவை->u.i_மதிப்பு*negInt);
    }இல்லையெனில்{
      i64 iVal;
      எனில்( op==TK_INTEGER && 0==sqlite3DecOrHexToI64(p_கோவை->u.zToken, &iVal) ){
        sqlite3VdbeMemSetInt64(pVal, iVal*negInt);
      }இல்லையெனில்{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_கோவை->u.zToken);
        எனில்( zVal==0 ) செல் no_mem;
        sqlite3_மதிப்பு_Set_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        எனில்( op==TK_FLOAT ){
          ப்ரதிஜ்ஞா( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_Term) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    எனில்( affinity==SQLITE_AFF_BLOB ){
      எனில்( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_மதிப்பு_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }இல்லையெனில்{
      sqlite3_மதிப்பு_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    ப்ரதிஜ்ஞா( (pVal->flags & MEM_IntReal)==0 );
    எனில்( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    எனில்( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }இல்லையெனில் எனில்( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    எனில்( SQLITE_OK==மதிப்பு_From_கோவை(db,p_கோவை->p_இடது,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      எனில்( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }இல்லையெனில் எனில்( pVal->u.i==SMALLEST_INT64 ){
#ifndef SQLITE_OMIT_FLOATING_POINT
        pVal->u.r = -(இரட்டை)SMALLEST_INT64;
#இல்லையெனில்
        pVal->u.r = LARGEST_INT64;
#endif
        MemSetTypeFlag(pVal, MEM_Real);
      }இல்லையெனில்{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_மதிப்பு_Apply_Affinity(pVal, affinity, enc);
    }
  }இல்லையெனில் எனில்( op==TK_NULL ){
    pVal = மதிப்பு_New(db, pCtx);
    எனில்( pVal==0 ) செல் no_mem;
    sqlite3VdbeMemSetNull(pVal);
  }
#ifndef SQLITE_OMIT_BLOB_LITERAL
  இல்லையெனில் எனில்( op==TK_BLOB ){
    முழு nVal;
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
    ப்ரதிஜ்ஞா( p_கோவை->u.zToken[0]=='x' || p_கோவை->u.zToken[0]=='X' );
    ப்ரதிஜ்ஞா( p_கோவை->u.zToken[1]=='\'' );
    pVal = மதிப்பு_New(db, pCtx);
    எனில்( !pVal ) செல் no_mem;
    zVal = &p_கோவை->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    ப்ரதிஜ்ஞா( zVal[nVal]=='\'' );
    sqlite3VdbeMemSetStr(pVal, sqlite3HexToBlob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#endif
#ifdef SQLITE_ENABLE_STAT4
  இல்லையெனில் எனில்( op==TK_FUNCTION && pCtx!=0 ){
    rc = மதிப்பு_From_Function(db, p_கோவை, enc, affinity, &pVal, pCtx);
  }
#endif
  இல்லையெனில் எனில்( op==TK_TRUEFALSE ){
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
    pVal = மதிப்பு_New(db, pCtx);
    எனில்( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_கோவை->u.zToken[4]==0;
      sqlite3_மதிப்பு_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  திரும்பு rc;

no_mem:
#ifdef SQLITE_ENABLE_STAT4
  எனில்( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#endif
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  ப்ரதிஜ்ஞா( *ppVal==0 );
#ifdef SQLITE_ENABLE_STAT4
  எனில்( pCtx==0 ) sqlite3_மதிப்பு_Free(pVal);
#இல்லையெனில்
  ப்ரதிஜ்ஞா( pCtx==0 ); sqlite3_மதிப்பு_Free(pVal);
#endif
  திரும்பு SQLITE_NOMEM_BKPT;
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
முழு sqlite3_மதிப்பு_From_கோவை(
  sqlite3 *db,              /* The database connection */
  மாறிலி கோவை *p_கோவை,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_மதிப்பு **ppVal     /* Write the new value here */
){
  முழு rc = SQLITE_OK;
  sqlite3_மதிப்பு *pVal = 0;
  எனில்( p_கோவை ){
    rc = மதிப்பு_From_கோவை(db, p_கோவை, enc, affinity, &pVal, 0);
    எனில்( rc==SQLITE_OK && pVal 
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
  திரும்பு rc;
}

#ifdef SQLITE_ENABLE_STAT4
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
நிலையான முழு stat4_மதிப்பு_From_கோவை(
  Parse *pParse,                  /* Parse context */
  கோவை *p_கோவை,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  கட்டமைப்பு மதிப்பு_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_மதிப்பு **ppVal           /* OUT: New value object (or NULL) */
){
  முழு rc = SQLITE_OK;
  sqlite3_மதிப்பு *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_கோவை = sqlite3_கோவை_Skip_Collate(p_கோவை);

  ப்ரதிஜ்ஞா( p_கோவை==0 || p_கோவை->op!=TK_REGISTER || p_கோவை->op2!=TK_VARIABLE );
  எனில்( !p_கோவை ){
    pVal = மதிப்பு_New(db, pAlloc);
    எனில்( pVal ){
      sqlite3VdbeMemSetNull((Mem*)pVal);
    }
  }இல்லையெனில் எனில்( p_கோவை->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    முழு iBindVar = p_கோவை->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    எனில்( (v = pParse->pReprepare)!=0 ){
      pVal = மதிப்பு_New(db, pAlloc);
      எனில்( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_மதிப்பு_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }இல்லையெனில்{
    rc = மதிப்பு_From_கோவை(db, p_கோவை, ENC(db), affinity, &pVal, pAlloc);
  }

  ப்ரதிஜ்ஞா( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  திரும்பு rc;
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
முழு sqlite3_Stat4_Probe_Set_மதிப்பு(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Index being probed */
  UnpackedRecord **ppRec,         /* IN/OUT: Probe record */
  கோவை *p_கோவை,                    /* The expression to extract a value from */
  முழு nElem,                      /* Maximum number of values to append */
  முழு iVal,                       /* Array element to populate */
  முழு *pnExtract                  /* OUT: Values appended to the record */
){
  முழு rc = SQLITE_OK;
  முழு nExtract = 0;

  எனில்( p_கோவை==0 || p_கோவை->op!=TK_SELECT ){
    முழு i;
    கட்டமைப்பு மதிப்பு_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    க்கு(i=0; i<nElem; i++){
      sqlite3_மதிப்பு *pVal = 0;
      கோவை *pElem = (p_கோவை ? sqlite3VectorFieldSubexpr(p_கோவை, i) : 0);
      u8 aff = sqlite3IndexColumnAffinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_மதிப்பு_From_கோவை(pParse, pElem, aff, &alloc, &pVal);
      எனில்( !pVal ) நிறுத்து;
      nExtract++;
    }
  }

  *pnExtract = nExtract;
  திரும்பு rc;
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
முழு sqlite3_Stat4_மதிப்பு_From_கோவை(
  Parse *pParse,                  /* Parse context */
  கோவை *p_கோவை,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_மதிப்பு **ppVal           /* OUT: New value object (or NULL) */
){
  திரும்பு stat4_மதிப்பு_From_கோவை(pParse, p_கோவை, affinity, 0, ppVal);
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
முழு sqlite3Stat4Column(
  sqlite3 *db,                    /* Database handle */
  மாறிலி வெறுமை *pRec,               /* Pointer to buffer containing record */
  முழு nRec,                       /* Size of buffer pRec in bytes */
  முழு iCol,                       /* Column to extract */
  sqlite3_மதிப்பு **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 iField;                     /* Next unread data byte */
  u32 szField = 0;                /* Size of the current data field */
  முழு i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  ப்ரதிஜ்ஞா( iCol>0 );
  iHdr = getVarint32(a, nHdr);
  எனில்( nHdr>(u32)nRec || iHdr>=nHdr ) திரும்பு SQLITE_CORRUPT_BKPT;
  iField = nHdr;
  க்கு(i=0; i<=iCol; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    எனில்( iHdr>nHdr ) திரும்பு SQLITE_CORRUPT_BKPT;
    szField = sqlite3VdbeSerialTypeLen(t);
    iField += szField;
  }
  testcase( iField==nRec );
  testcase( iField==nRec+1 );
  எனில்( iField>nRec ) திரும்பு SQLITE_CORRUPT_BKPT;
  எனில்( pMem==0 ){
    pMem = *ppVal = sqlite3_மதிப்பு_New(db);
    எனில்( pMem==0 ) திரும்பு SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[iField-szField], t, pMem);
  pMem->enc = ENC(db);
  திரும்பு SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
வெறுமை sqlite3Stat4ProbeFree(UnpackedRecord *pRec){
  எனில்( pRec ){
    முழு i;
    முழு nCol = pRec->p_சாவி_Info->nAllField;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    க்கு(i=0; i<nCol; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_சாவி_Info_Unref(pRec->p_சாவி_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#endif /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
வெறுமை sqlite3_மதிப்பு_Set_Str(
  sqlite3_மதிப்பு *v,     /* Value to be set */
  முழு n,                /* Length of string z */
  மாறிலி வெறுமை *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  வெறுமை (*xDel)(வெறுமை*)   /* Destructor for the string */
){
  எனில்( v ) sqlite3VdbeMemSetStr((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
வெறுமை sqlite3_மதிப்பு_Free(sqlite3_மதிப்பு *v){
  எனில்( !v ) திரும்பு;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
நிலையான SQLITE_NOINLINE முழு மதிப்பு_Bytes(sqlite3_மதிப்பு *pVal, u8 enc){
  திரும்பு மதிப்பு_To_Text(pVal, enc)!=0 ? pVal->n : 0;
}
முழு sqlite3_மதிப்பு_Bytes(sqlite3_மதிப்பு *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  ப்ரதிஜ்ஞா( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  எனில்( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    திரும்பு p->n;
  }
  எனில்( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    திரும்பு p->n;
  }
  எனில்( (p->flags & MEM_Blob)!=0 ){
    எனில்( p->flags & MEM_Zero ){
      திரும்பு p->n + p->u.nZero;
    }இல்லையெனில்{
      திரும்பு p->n;
    }
  }
  எனில்( p->flags & MEM_Null ) திரும்பு 0;
  திரும்பு மதிப்பு_Bytes(pVal, enc);
}
