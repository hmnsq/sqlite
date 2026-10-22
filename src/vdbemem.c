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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "vdbeInt.h"

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
পূর্ণাঙ্ক sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  প্রতিজ্ঞা( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  প্রতিজ্ঞা( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  প্রতিজ্ঞা( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  যদি( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    প্রতিজ্ঞা( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    যদি( (p->flags & (MEM_Term|MEM_Subtype))==(MEM_Term|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      প্রতিজ্ঞা( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      প্রতিজ্ঞা( (p->flags & ~(MEM_Null|MEM_Term|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }অন্যথা{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }অন্যথা{
    /* The MEM_Cleared bit is only allowed on NULLs */
    প্রতিজ্ঞা( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  প্রতিজ্ঞা( p->szMalloc==0
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
  যদি( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    প্রতিজ্ঞা( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  প্রত্যাগম 1;
}
#endif

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
স্থির শূন্য vdbe_Mem_Render_অঙ্ক(পূর্ণাঙ্ক sz, বর্ণ *zBuf, Mem *p){
  StrAccum acc;
  প্রতিজ্ঞা( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  প্রতিজ্ঞা( sz>22 );
  যদি( p->flags & (MEM_Int|MEM_IntReal) ){
#যদি GCC_VERSION>=7000000 && GCC_VERSION<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    প্রতিজ্ঞা( (sqlite3Config.bSmallMalloc!=0xee)*8==পরিমাণ(x) );
    memcpy(&x, (বর্ণ*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3Int64ToText(x, zBuf);
#অন্যথা
    p->n = sqlite3Int64ToText(p->u.i, zBuf);
#endif
    যদি( p->flags & MEM_IntReal ){
      memcpy(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }অন্যথা{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    প্রতিজ্ঞা( acc.zText==zBuf && acc.mxAlloc<=0 );
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
পূর্ণাঙ্ক sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  বর্ণ zBuf[100];
  বর্ণ *z;
  পূর্ণাঙ্ক i, j, incr;
  যদি( (p->flags & MEM_Str)==0 ) প্রত্যাগম 1;
  যদি( p->db && p->db->mallocFailed ) প্রত্যাগম 1;
  যদি( p->flags & MEM_Term ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    যদি( p->szMalloc>0 && p->z==p->zMalloc ){
      প্রতিজ্ঞা( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      প্রতিজ্ঞা( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    প্রতিজ্ঞা( p->z[p->n]==0 );
    প্রতিজ্ঞা( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    প্রতিজ্ঞা( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  যদি( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) প্রত্যাগম 1;
  যদি( p->db==0 ){
    প্রত্যাগম 1;  /* db->nFpDigit required to validate p->z[] */
  }
  memcpy(&tmp, p, পরিমাণ(tmp));
  vdbe_Mem_Render_অঙ্ক(পরিমাণ(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  যদি( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    যদি( p->enc==SQLITE_UTF16BE ) z++;
  }
  যাবত্( zBuf[j] ){
    যদি( zBuf[j++]!=z[i] ) প্রত্যাগম 0;
    i += incr;
  }
  প্রত্যাগম 1;
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
পূর্ণাঙ্ক sqlite3VdbeChangeEncoding(Mem *pMem, পূর্ণাঙ্ক desiredEnc){
#ifndef SQLITE_OMIT_UTF16
  পূর্ণাঙ্ক rc;
#endif
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  প্রতিজ্ঞা( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  যদি( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    প্রত্যাগম SQLITE_OK;
  }
  যদি( pMem->enc==desiredEnc ){
    প্রত্যাগম SQLITE_OK;
  }
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ifdef SQLITE_OMIT_UTF16
  প্রত্যাগম SQLITE_ERROR;
#অন্যথা

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  প্রতিজ্ঞা(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  প্রতিজ্ঞা(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  প্রতিজ্ঞা(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  প্রত্যাগম rc;
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
SQLITE_NOINLINE পূর্ণাঙ্ক sqlite3VdbeMemGrow(Mem *pMem, পূর্ণাঙ্ক n, পূর্ণাঙ্ক bPreserve){
  প্রতিজ্ঞা( sqlite3VdbeCheckMemInvariants(pMem) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  প্রতিজ্ঞা( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  প্রতিজ্ঞা( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3DbMallocSize(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3DbMallocSize(pMem->db,pMem->zMalloc));
  যদি( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    যদি( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }অন্যথা{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      যদি( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }অন্যথা{
    যদি( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  যদি( pMem->zMalloc==0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }অন্যথা{
    pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
  }

  যদি( bPreserve && pMem->z ){
    প্রতিজ্ঞা( pMem->z!=pMem->zMalloc );
    memcpy(pMem->zMalloc, pMem->z, pMem->n);
  }
  যদি( (pMem->flags&MEM_Dyn)!=0 ){
    প্রতিজ্ঞা( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((শূন্য *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  প্রত্যাগম SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3VdbeMemClearAndResize(Mem *pMem, পূর্ণাঙ্ক szNew){
  প্রতিজ্ঞা( CORRUPT_DB || szNew>0 );
  প্রতিজ্ঞা( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  যদি( pMem->szMalloc<szNew ){
    প্রত্যাগম sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  প্রতিজ্ঞা( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  প্রত্যাগম SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  যদি( (pMem->flags & (MEM_Str|MEM_Term|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    প্রত্যাগম 0;
  }
  যদি( pMem->enc!=SQLITE_UTF8 ) প্রত্যাগম 0;
  প্রতিজ্ঞা( pMem->z!=0 );
  যদি( pMem->flags & MEM_Dyn ){
    যদি( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_Term;
      প্রত্যাগম 1;
    }
    যদি( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_Term;
      প্রত্যাগম 1;
    }
  }অন্যথা যদি( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_Term;
    প্রত্যাগম 1;
  }
  প্রত্যাগম 0;
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
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক vdbeMemAddTerminator(Mem *pMem){
  যদি( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_Term;
  প্রত্যাগম SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
পূর্ণাঙ্ক sqlite3VdbeMemMakeWriteable(Mem *pMem){
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  যদি( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    যদি( ExpandBlob(pMem) ) প্রত্যাগম SQLITE_NOMEM;
    যদি( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      পূর্ণাঙ্ক rc = vdbeMemAddTerminator(pMem);
      যদি( rc ) প্রত্যাগম rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ifdef SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#endif

  প্রত্যাগম SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ifndef SQLITE_OMIT_INCRBLOB
পূর্ণাঙ্ক sqlite3VdbeMemExpandBlob(Mem *pMem){
  পূর্ণাঙ্ক nByte;
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->flags & MEM_Zero );
  প্রতিজ্ঞা( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_মান_nochange(pMem) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  যদি( nByte<=0 ){
    যদি( (pMem->flags & MEM_Blob)==0 ) প্রত্যাগম SQLITE_OK;
    nByte = 1;
  }
  যদি( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  প্রতিজ্ঞা( pMem->z!=0 );
  প্রতিজ্ঞা( sqlite3DbMallocSize(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_Term);
  প্রত্যাগম SQLITE_OK;
}
#endif

/*
** Make sure the given Mem is \u0000 terminated.
*/
পূর্ণাঙ্ক sqlite3VdbeMemNulTerminate(Mem *pMem){
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==(MEM_Term|MEM_Str) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==0 );
  যদি( (pMem->flags & (MEM_Term|MEM_Str))!=MEM_Str ){
    প্রত্যাগম SQLITE_OK;   /* Nothing to do */
  }অন্যথা{
    প্রত্যাগম vdbeMemAddTerminator(pMem);
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
পূর্ণাঙ্ক sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  ধ্রুব পূর্ণাঙ্ক nByte = 32;

  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( !(pMem->flags&MEM_Zero) );
  প্রতিজ্ঞা( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  প্রতিজ্ঞা( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pMem) );


  যদি( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_অঙ্ক(nByte, pMem->z, pMem);
  প্রতিজ্ঞা( pMem->z!=0 );
  প্রতিজ্ঞা( pMem->n==(পূর্ণাঙ্ক)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_Term;
  যদি( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  প্রত্যাগম SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
পূর্ণাঙ্ক sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  প্রতিজ্ঞা( pFunc!=0 );
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db!=0 );
  প্রতিজ্ঞা( pFunc->xFinalize!=0 );
  প্রতিজ্ঞা( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  প্রতিজ্ঞা( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, পরিমাণ(ctx));
  memset(&t, 0, পরিমাণ(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.pOut = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  প্রতিজ্ঞা( (pMem->flags & MEM_Dyn)==0 );
  যদি( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  memcpy(pMem, &t, পরিমাণ(t));
  প্রত্যাগম ctx.isError;
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
পূর্ণাঙ্ক sqlite3_Vdbe_Mem_Agg_মান(Mem *pAccum, Mem *pOut, FuncDef *pFunc){
  sqlite3_context ctx;
  প্রতিজ্ঞা( pFunc!=0 );
  প্রতিজ্ঞা( pFunc->x_মান!=0 );
  প্রতিজ্ঞা( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  প্রতিজ্ঞা( pAccum->db!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, পরিমাণ(ctx));
  sqlite3VdbeMemSetNull(pOut);
  ctx.pOut = pOut;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_মান(&ctx);
  প্রত্যাগম ctx.isError;
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
স্থির SQLITE_NOINLINE শূন্য vdbeMemClearExternAndSetNull(Mem *p){
  প্রতিজ্ঞা( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  প্রতিজ্ঞা( VdbeMemDynamic(p) );
  যদি( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    প্রতিজ্ঞা( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  যদি( p->flags&MEM_Dyn ){
    প্রতিজ্ঞা( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((শূন্য *)p->z);
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
স্থির SQLITE_NOINLINE শূন্য vdbeMemClear(Mem *p){
  যদি( VdbeMemDynamic(p) ){
    vdbeMemClearExternAndSetNull(p);
  }
  যদি( p->szMalloc ){
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
শূন্য sqlite3VdbeMemRelease(Mem *p){
  প্রতিজ্ঞা( sqlite3VdbeCheckMemInvariants(p) );
  যদি( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
শূন্য sqlite3VdbeMemReleaseMalloc(Mem *p){
  প্রতিজ্ঞা( !VdbeMemDynamic(p) );
  যদি( p->szMalloc ) vdbeMemClear(p);
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
স্থির SQLITE_NOINLINE i64 mem_Int_মান(ধ্রুব Mem *pMem){
  i64 মান = 0;
  sqlite3Atoi64(pMem->z, &মান, pMem->n, pMem->enc);
  প্রত্যাগম মান;
}
i64 sqlite3_Vdbe_Int_মান(ধ্রুব Mem *pMem){
  পূর্ণাঙ্ক flags;
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  যদি( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    প্রত্যাগম pMem->u.i;
  }অন্যথা যদি( flags & MEM_Real ){
    প্রত্যাগম sqlite3RealToI64(pMem->u.r);
  }অন্যথা যদি( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    প্রত্যাগম mem_Int_মান(pMem);
  }অন্যথা{
    প্রত্যাগম 0;
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
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক sqlite3_Mem_Real_মান_RCSlow_Path(
  Mem *pMem,
  দ্বিগুণ *p_মান
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  *p_মান = 0.0;
  যদি( pMem->enc==SQLITE_UTF8 ){
    বর্ণ *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    যদি( zCopy ){
      rc = sqlite3AtoF(zCopy, p_মান);
      sqlite3DbFree(pMem->db, zCopy);
    }
    প্রত্যাগম rc;
  }অন্যথা{
    পূর্ণাঙ্ক n, i, j;
    বর্ণ *zCopy;
    ধ্রুব বর্ণ *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    যদি( zCopy ){
      z = pMem->z;
      যদি( pMem->enc==SQLITE_UTF16LE ){
        প্রতি(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          যদি( z[i+1]!=0 ) ভঙ্গ;
        }
      }অন্যথা{
        প্রতি(i=j=0; i<n-1; i+=2, j++){
          যদি( z[i]!=0 ) ভঙ্গ;
          zCopy[j] = z[i+1];
        }
      }     
      প্রতিজ্ঞা( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_মান);
      যদি( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    প্রত্যাগম rc;
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
পূর্ণাঙ্ক sqlite3_Mem_Real_মান_RC(Mem *pMem, দ্বিগুণ *p_মান){
  testcase( pMem->db==0 );
  প্রতিজ্ঞা( pMem->flags & (MEM_Str|MEM_Blob) );
  যদি( pMem->z==0 ){
    *p_মান = 0.0;
    প্রত্যাগম 0;
  }অন্যথা যদি( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_Term)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    প্রত্যাগম sqlite3AtoF(pMem->z, p_মান);
  }অন্যথা যদি( pMem->n==0 ){
    *p_মান = 0.0;
    প্রত্যাগম 0;
  }অন্যথা{
    প্রত্যাগম sqlite3_Mem_Real_মান_RCSlow_Path(pMem, p_মান);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
স্থির SQLITE_NOINLINE দ্বিগুণ sqlite3_Mem_Real_মান_No_RC(Mem *pMem){
  দ্বিগুণ r;
  (শূন্য)sqlite3_Mem_Real_মান_RC(pMem, &r);
  প্রত্যাগম r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
দ্বিগুণ sqlite3_Vdbe_Real_মান(Mem *pMem){
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pMem) );
  যদি( pMem->flags & MEM_Real ){
    প্রত্যাগম pMem->u.r;
  }অন্যথা যদি( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    প্রত্যাগম (দ্বিগুণ)pMem->u.i;
  }অন্যথা যদি( pMem->flags & (MEM_Str|MEM_Blob) ){
    প্রত্যাগম sqlite3_Mem_Real_মান_No_RC(pMem);
  }অন্যথা{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    প্রত্যাগম (দ্বিগুণ)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
পূর্ণাঙ্ক sqlite3_Vdbe_Boolean_মান(Mem *pMem, পূর্ণাঙ্ক ifNull){
  testcase( pMem->flags & MEM_IntReal );
  যদি( pMem->flags & (MEM_Int|MEM_IntReal) ) প্রত্যাগম pMem->u.i!=0;
  যদি( pMem->flags & MEM_Null ) প্রত্যাগম ifNull;
  প্রত্যাগম sqlite3_Vdbe_Real_মান(pMem)!=0.0;
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
শূন্য sqlite3VdbeIntegerAffinity(Mem *pMem, পূর্ণাঙ্ক bFromText){
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->flags & (MEM_Real|MEM_IntReal) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pMem) );

  যদি( pMem->flags & MEM_IntReal ){
    MemSetTypeFlag(pMem, MEM_Int);
  }অন্যথা{
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
    যদি( pMem->u.r==ix && ix<LARGEST_INT64
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
পূর্ণাঙ্ক sqlite3VdbeMemIntegerify(Mem *pMem){
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_মান(pMem);
  MemSetTypeFlag(pMem, MEM_Int);
  প্রত্যাগম SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
পূর্ণাঙ্ক sqlite3VdbeMemRealify(Mem *pMem){
  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_মান(pMem);
  MemSetTypeFlag(pMem, MEM_Real);
  প্রত্যাগম SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3RealSameAsInt(দ্বিগুণ r1, sqlite3_int64 i){
  দ্বিগুণ r2 = (দ্বিগুণ)i;
  প্রত্যাগম r1==0.0
      || (memcmp(&r1, &r2, পরিমাণ(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3RealToI64(দ্বিগুণ r){
  যদি( r<-9223372036854774784.0 ) প্রত্যাগম SMALLEST_INT64;
  যদি( r>+9223372036854774784.0 ) প্রত্যাগম LARGEST_INT64;
  প্রত্যাগম (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
পূর্ণাঙ্ক sqlite3VdbeMemNumerify(Mem *pMem){
  প্রতিজ্ঞা( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  যদি( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    পূর্ণাঙ্ক rc;
    sqlite3_int64 ix;
    প্রতিজ্ঞা( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_মান_RC(pMem, &pMem->u.r);
    যদি( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3RealToI64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      MemSetTypeFlag(pMem, MEM_Int);
    }অন্যথা{
      MemSetTypeFlag(pMem, MEM_Real);
    }
  }
  প্রতিজ্ঞা( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  প্রত্যাগম SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
পূর্ণাঙ্ক sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  যদি( pMem->flags & MEM_Null ) প্রত্যাগম SQLITE_OK;
  বিকল্প( aff ){
    পক্ষ SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      যদি( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_মান_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
        প্রতিজ্ঞা( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        যদি( pMem->flags & MEM_Str ) MemSetTypeFlag(pMem, MEM_Blob);
      }অন্যথা{
        pMem->flags &= ~(MEM_TypeMask&~MEM_Blob);
      }
      ভঙ্গ;
    }
    পক্ষ SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      ভঙ্গ;
    }
    পক্ষ SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      ভঙ্গ;
    }
    পক্ষ SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      ভঙ্গ;
    }
    স্বভাব: {
      পূর্ণাঙ্ক rc;
      প্রতিজ্ঞা( aff==SQLITE_AFF_TEXT );
      প্রতিজ্ঞা( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_মান_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
      প্রতিজ্ঞা( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      যদি( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      যদি( rc ) প্রত্যাগম rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
শূন্য sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  প্রতিজ্ঞা( (flags & ~MEM_TypeMask)==0 );
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
শূন্য sqlite3VdbeMemSetNull(Mem *pMem){
  যদি( VdbeMemDynamic(pMem) ){
    vdbeMemClearExternAndSetNull(pMem);
  }অন্যথা{
    pMem->flags = MEM_Null;
  }
}
শূন্য sqlite3_মান_Set_Null(sqlite3_মান *p){
  sqlite3VdbeMemSetNull((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ifndef SQLITE_OMIT_INCRBLOB
শূন্য sqlite3VdbeMemSetZeroBlob(Mem *pMem, পূর্ণাঙ্ক n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  যদি( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#অন্যথা
পূর্ণাঙ্ক sqlite3VdbeMemSetZeroBlob(Mem *pMem, পূর্ণাঙ্ক n){
  পূর্ণাঙ্ক nByte = n>0?n:1;
  যদি( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  প্রতিজ্ঞা( pMem->z!=0 );
  প্রতিজ্ঞা( sqlite3DbMallocSize(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  প্রত্যাগম SQLITE_OK;
}
#endif

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
স্থির SQLITE_NOINLINE শূন্য vdbeReleaseAndSetInt64(Mem *pMem, i64 val){
  sqlite3VdbeMemSetNull(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
শূন্য sqlite3VdbeMemSetInt64(Mem *pMem, i64 val){
  যদি( VdbeMemDynamic(pMem) ){
    vdbeReleaseAndSetInt64(pMem, val);
  }অন্যথা{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
শূন্য sqlite3MemSetArrayInt64(sqlite3_মান *aMem, পূর্ণাঙ্ক iIdx, i64 val){
  sqlite3VdbeMemSetInt64(&aMem[iIdx], val);
}

/* A no-op destructor */
শূন্য sqlite3NoopDestructor(শূন্য *p){ UNUSED_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
শূন্য sqlite3VdbeMemSetPointer(
  Mem *pMem,
  শূন্য *pPtr,
  ধ্রুব বর্ণ *zPType,
  শূন্য (*xDestructor)(শূন্য*)
){
  প্রতিজ্ঞা( pMem->flags==MEM_Null );
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
শূন্য sqlite3VdbeMemSetDouble(Mem *pMem, দ্বিগুণ val){
  sqlite3VdbeMemSetNull(pMem);
  যদি( !sqlite3IsNaN(val) ){
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
পূর্ণাঙ্ক sqlite3VdbeMemIsRowSet(ধ্রুব Mem *pMem){
  প্রত্যাগম (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
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
পূর্ণাঙ্ক sqlite3VdbeMemSetRowSet(Mem *pMem){
  sqlite3 *db = pMem->db;
  RowSet *p;
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3RowSetInit(db);
  যদি( p==0 ) প্রত্যাগম SQLITE_NOMEM;
  pMem->z = (বর্ণ*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3RowSetDelete;
  প্রত্যাগম SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
পূর্ণাঙ্ক sqlite3VdbeMemTooBig(Mem *p){
  প্রতিজ্ঞা( p->db!=0 );
  যদি( p->flags & (MEM_Str|MEM_Blob) ){
    পূর্ণাঙ্ক n = p->n;
    যদি( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    প্রত্যাগম n>p->db->aLimit[SQLITE_LIMIT_LENGTH];
  }
  প্রত্যাগম 0; 
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
শূন্য sqlite3VdbeMemAboutToChange(Vdbe *pVdbe, Mem *pMem){
  পূর্ণাঙ্ক i;
  Mem *pX;
  যদি( pMem->bScopy ){
    প্রতি(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      যদি( pX->pScopyFrom==pMem ){
        u16 mFlags;
        যদি( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (পূর্ণাঙ্ক)(pX - pVdbe->aMem), (পূর্ণাঙ্ক)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        প্রতিজ্ঞা( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
স্থির SQLITE_NOINLINE শূন্য vdbeClrCopy(Mem *pTo, ধ্রুব Mem *pFrom, পূর্ণাঙ্ক eType){
  vdbeMemClearExternAndSetNull(pTo);
  প্রতিজ্ঞা( !VdbeMemDynamic(pTo) );
  sqlite3VdbeMemShallowCopy(pTo, pFrom, eType);
}
শূন্য sqlite3VdbeMemShallowCopy(Mem *pTo, ধ্রুব Mem *pFrom, পূর্ণাঙ্ক srcType){
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pFrom) );
  প্রতিজ্ঞা( pTo->db==pFrom->db );
  যদি( VdbeMemDynamic(pTo) ){ vdbeClrCopy(pTo,pFrom,srcType); প্রত্যাগম; }
  memcpy(pTo, pFrom, MEMCELLSIZE);
  যদি( (pFrom->flags&MEM_Static)==0 ){
    pTo->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    প্রতিজ্ঞা( srcType==MEM_Ephem || srcType==MEM_Static );
    pTo->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
পূর্ণাঙ্ক sqlite3VdbeMemCopy(Mem *pTo, ধ্রুব Mem *pFrom){
  পূর্ণাঙ্ক rc = SQLITE_OK;

  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pFrom) );
  যদি( VdbeMemDynamic(pTo) ) vdbeMemClearExternAndSetNull(pTo);
  memcpy(pTo, pFrom, MEMCELLSIZE);
  pTo->flags &= ~MEM_Dyn;
  যদি( pTo->flags&(MEM_Str|MEM_Blob) ){
    যদি( 0==(pFrom->flags&MEM_Static) ){
      pTo->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(pTo);
    }
  }

  প্রত্যাগম rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
শূন্য sqlite3VdbeMemMove(Mem *pTo, Mem *pFrom){
  প্রতিজ্ঞা( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  প্রতিজ্ঞা( pTo->db==0 || sqlite3_mutex_held(pTo->db->mutex) );
  প্রতিজ্ঞা( pFrom->db==0 || pTo->db==0 || pFrom->db==pTo->db );

  sqlite3VdbeMemRelease(pTo);
  memcpy(pTo, pFrom, পরিমাণ(Mem));
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
পূর্ণাঙ্ক sqlite3VdbeMemSetStr(
  Mem *pMem,          /* Memory cell to set to string value */
  ধ্রুব বর্ণ *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  শূন্য (*xDel)(শূন্য*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  পূর্ণাঙ্ক iLimit;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  প্রতিজ্ঞা( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  যদি( !z ){
    sqlite3VdbeMemSetNull(pMem);
    প্রত্যাগম SQLITE_OK;
  }

  যদি( pMem->db ){
    iLimit = pMem->db->aLimit[SQLITE_LIMIT_LENGTH];
  }অন্যথা{
    iLimit = SQLITE_MAX_LENGTH;
  }
  যদি( nByte<0 ){
    প্রতিজ্ঞা( enc!=0 );
    যদি( enc==SQLITE_UTF8 ){
      nByte = strlen(z);
    }অন্যথা{
      প্রতি(nByte=0; nByte<=iLimit && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_Term;
  }অন্যথা যদি( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }অন্যথা{
    flags = MEM_Str;
  }
  যদি( nByte>iLimit ){
    যদি( xDel && xDel!=SQLITE_TRANSIENT ){
      যদি( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (শূন্য*)z);
      }অন্যথা{
        xDel((শূন্য*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    প্রত্যাগম sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  যদি( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    যদি( flags&MEM_Term ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    যদি( sqlite3VdbeMemClearAndResize(pMem, (পূর্ণাঙ্ক)MAX(nAlloc,32)) ){
      প্রত্যাগম SQLITE_NOMEM_BKPT;
    }
    প্রতিজ্ঞা( pMem->z!=0 );
    memcpy(pMem->z, z, nAlloc);
  }অন্যথা{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (বর্ণ *)z;
    যদি( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
    }অন্যথা{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (পূর্ণাঙ্ক)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ifndef SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  যদি( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
#endif


  প্রত্যাগম SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
পূর্ণাঙ্ক sqlite3VdbeMemSetText(
  Mem *pMem,          /* Memory cell to set to string value */
  ধ্রুব বর্ণ *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  শূন্য (*xDel)(শূন্য*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  প্রতিজ্ঞা( pMem!=0 );
  প্রতিজ্ঞা( pMem->db!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  যদি( !z ){
    sqlite3VdbeMemSetNull(pMem);
    প্রত্যাগম SQLITE_OK;
  }

  যদি( nByte<0 ){
    nByte = strlen(z);
    flags = MEM_Str|MEM_Term;
  }অন্যথা{
    flags = MEM_Str;
  }
  যদি( nByte>(i64)pMem->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    যদি( xDel && xDel!=SQLITE_TRANSIENT ){
      যদি( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (শূন্য*)z);
      }অন্যথা{
        xDel((শূন্য*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    প্রত্যাগম sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  যদি( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    যদি( sqlite3VdbeMemClearAndResize(pMem, (পূর্ণাঙ্ক)MAX(nAlloc,32)) ){
      প্রত্যাগম SQLITE_NOMEM_BKPT;
    }
    প্রতিজ্ঞা( pMem->z!=0 );
    memcpy(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }অন্যথা{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (বর্ণ *)z;
    যদি( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }অন্যথা যদি( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }অন্যথা{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (পূর্ণাঙ্ক)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  প্রত্যাগম SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 offset,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  পূর্ণাঙ্ক rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE-1 );
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE );
  যদি( amt>=SQLITE_MAX_ALLOCATION_SIZE ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  যদি( (u64)amt + (u64)offset > (u64)sqlite3BtreeMaxRecordSize(pCur) ){
    প্রত্যাগম SQLITE_CORRUPT_BKPT;
  }
  যদি( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, offset, amt, pMem->z);
    যদি( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (পূর্ণাঙ্ক)amt;
    }অন্যথা{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  প্রত্যাগম rc;
}
পূর্ণাঙ্ক sqlite3VdbeMemFromBtreeZeroOffset(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 available = 0;  /* Number of bytes available on the local btree page */
  পূর্ণাঙ্ক rc = SQLITE_OK; /* Return code */

  প্রতিজ্ঞা( sqlite3BtreeCursorIsValid(pCur) );
  প্রতিজ্ঞা( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pMem) );
  pMem->z = (বর্ণ *)sqlite3BtreePayloadFetch(pCur, &available);
  প্রতিজ্ঞা( pMem->z!=0 );

  যদি( amt<=available ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (পূর্ণাঙ্ক)amt;
  }অন্যথা{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  প্রত্যাগম rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
স্থির SQLITE_NOINLINE ধ্রুব শূন্য *মান_To_Text(sqlite3_মান* pVal, u8 enc){
  প্রতিজ্ঞা( pVal!=0 );
  প্রতিজ্ঞা( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  প্রতিজ্ঞা( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pVal) );
  প্রতিজ্ঞা( (pVal->flags & (MEM_Null))==0 );
  যদি( pVal->flags & (MEM_Blob|MEM_Str) ){
    যদি( ExpandBlob(pVal) ) প্রত্যাগম 0;
    pVal->flags |= MEM_Str;
    যদি( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    যদি( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_TO_INT(pVal->z)) ){
      প্রতিজ্ঞা( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      যদি( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        প্রত্যাগম 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }অন্যথা{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    প্রতিজ্ঞা( 0==(1&SQLITE_PTR_TO_INT(pVal->z)) );
  }
  প্রতিজ্ঞা(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  যদি( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    প্রতিজ্ঞা( sqlite3VdbeMemValidStrRep(pVal) );
    প্রত্যাগম pVal->z;
  }অন্যথা{
    প্রত্যাগম 0;
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
ধ্রুব শূন্য *sqlite3_মান_Text(sqlite3_মান* pVal, u8 enc){
  যদি( !pVal ) প্রত্যাগম 0;
  প্রতিজ্ঞা( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  প্রতিজ্ঞা( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  প্রতিজ্ঞা( !sqlite3VdbeMemIsRowSet(pVal) );
  যদি( (pVal->flags&(MEM_Str|MEM_Term))==(MEM_Str|MEM_Term) && pVal->enc==enc ){
    প্রতিজ্ঞা( sqlite3VdbeMemValidStrRep(pVal) );
    প্রত্যাগম pVal->z;
  }
  যদি( pVal->flags&MEM_Null ){
    প্রত্যাগম 0;
  }
  প্রত্যাগম মান_To_Text(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
পূর্ণাঙ্ক sqlite3_মান_Is_Of_Class(ধ্রুব sqlite3_মান *pVal, শূন্য(*xFree)(শূন্য*)){
  যদি( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    প্রত্যাগম 1;
  }অন্যথা{
    প্রত্যাগম 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_মান *sqlite3_মান_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, পরিমাণ(*p));
  যদি( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  প্রত্যাগম p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
সংরচনা মান_New_Stat4_Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  পূর্ণাঙ্ক iVal;
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
স্থির sqlite3_মান *মান_New(sqlite3 *db, সংরচনা মান_New_Stat4_Ctx *p){
#ifdef SQLITE_ENABLE_STAT4
  যদি( p ){
    UnpackedRecord *pRec = p->ppRec[0];

    যদি( pRec==0 ){
      Index *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      পূর্ণাঙ্ক i;                      /* Counter variable */
      পূর্ণাঙ্ক nCol = pIdx->nColumn;   /* Number of index columns including rowid */
  
      nByte = পরিমাণ(Mem) * nCol + ROUND8(পরিমাণ(UnpackedRecord));
      pRec = (UnpackedRecord*)sqlite3DbMallocZero(db, nByte);
      যদি( pRec ){
        pRec->p_চাবি_Info = sqlite3_চাবি_Info_Of_Index(p->pParse, pIdx);
        যদি( pRec->p_চাবি_Info ){
          প্রতিজ্ঞা( pRec->p_চাবি_Info->nAllField==nCol );
          প্রতিজ্ঞা( pRec->p_চাবি_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(পরিমাণ(UnpackedRecord)));
          প্রতি(i=0; i<nCol; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }অন্যথা{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      যদি( pRec==0 ) প্রত্যাগম 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->nField = p->iVal+1;
    sqlite3VdbeMemSetNull(&pRec->aMem[p->iVal]);
    প্রত্যাগম &pRec->aMem[p->iVal];
  }
#অন্যথা
  UNUSED_PARAMETER(p);
#endif /* defined(SQLITE_ENABLE_STAT4) */
  প্রত্যাগম sqlite3_মান_New(db);
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
স্থির পূর্ণাঙ্ক মান_From_Function(
  sqlite3 *db,                    /* The database connection */
  ধ্রুব রাশিমালা *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_মান **ppVal,          /* Write the new value here */
  সংরচনা মান_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_মান **apVal = 0;      /* Function arguments */
  পূর্ণাঙ্ক nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_মান *pVal = 0;        /* New value */
  পূর্ণাঙ্ক rc = SQLITE_OK;             /* Return code */
  রাশিমালা_List *pList = 0;            /* Function arguments */
  পূর্ণাঙ্ক i;                          /* Iterator variable */

  প্রতিজ্ঞা( pCtx!=0 );
  প্রতিজ্ঞা( (p->flags & EP_TokenOnly)==0 );
  প্রতিজ্ঞা( রাশিমালা_Use_XList(p) );
  pList = p->x.pList;
  যদি( pList ) nVal = pList->n_রাশিমালা;
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_Int_মান) );
  pFunc = sqlite3FindFunction(db, p->u.zToken, nVal, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  যদি( pFunc==0 ) প্রত্যাগম SQLITE_OK;
#endif
  প্রতিজ্ঞা( pFunc );
  যদি( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    প্রত্যাগম SQLITE_OK;
  }

  যদি( pList ){
    apVal = (sqlite3_মান**)sqlite3DbMallocZero(db, পরিমাণ(apVal[0]) * nVal);
    যদি( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      গচ্ছ মান_from_function_out;
    }
    প্রতি(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_মান_From_রাশিমালা(pCtx->pParse, pList->a[i].p_রাশিমালা, aff,
                                     &apVal[i]);
      যদি( apVal[i]==0 || rc!=SQLITE_OK ) গচ্ছ মান_from_function_out;
    }
  }

  pVal = মান_New(db, pCtx);
  যদি( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    গচ্ছ মান_from_function_out;
  }

  memset(&ctx, 0, পরিমাণ(ctx));
  ctx.pOut = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  যদি( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_বার্তা(pCtx->pParse, "%s", sqlite3_মান_text(pVal));
  }অন্যথা{
    sqlite3_মান_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    প্রতিজ্ঞা( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    যদি( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 মান_from_function_out:
  যদি( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  যদি( apVal ){
    প্রতি(i=0; i<nVal; i++){
      sqlite3_মান_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  প্রত্যাগম rc;
}
#অন্যথা
# define মান_From_Function(a,b,c,d,e,f) SQLITE_OK
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
স্থির পূর্ণাঙ্ক মান_From_রাশিমালা(
  sqlite3 *db,                    /* The database connection */
  ধ্রুব রাশিমালা *p_রাশিমালা,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_মান **ppVal,          /* Write the new value here */
  সংরচনা মান_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  পূর্ণাঙ্ক op;
  বর্ণ *zVal = 0;
  sqlite3_মান *pVal = 0;
  পূর্ণাঙ্ক negInt = 1;
  ধ্রুব বর্ণ *zNeg = "";
  পূর্ণাঙ্ক rc = SQLITE_OK;

  প্রতিজ্ঞা( p_রাশিমালা!=0 );
  যাবত্( (op = p_রাশিমালা->op)==TK_UPLUS || op==TK_SPAN ) p_রাশিমালা = p_রাশিমালা->p_বাম;
  যদি( op==TK_REGISTER ) op = p_রাশিমালা->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  প্রতিজ্ঞা( (p_রাশিমালা->flags & EP_TokenOnly)==0 || pCtx==0 );

  যদি( op==TK_CAST ){
    u8 aff;
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
    aff = sqlite3AffinityType(p_রাশিমালা->u.zToken,0);
    rc = মান_From_রাশিমালা(db, p_রাশিমালা->p_বাম, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    যদি( *ppVal ){
#ifdef SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#অন্যথা
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      প্রতিজ্ঞা( (ppVal[0][0].flags & MEM_Zero)==0 );
#endif
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_মান_Apply_Affinity(*ppVal, affinity, enc);
    }
    প্রত্যাগম rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  যদি( op==TK_UMINUS ){
    রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
    যদি( (p_বাম->op==TK_INTEGER || p_বাম->op==TK_FLOAT) ){
      যদি( রাশিমালা_Has_Property(p_বাম, EP_Int_মান)
       || p_বাম->u.zToken[0]!='0' || (p_বাম->u.zToken[1] & ~0x20)!='X'
      ){
        p_রাশিমালা = p_বাম;
        op = p_রাশিমালা->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  যদি( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = মান_New(db, pCtx);
    যদি( pVal==0 ) গচ্ছ no_mem;
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) ){
      sqlite3VdbeMemSetInt64(pVal, (i64)p_রাশিমালা->u.i_মান*negInt);
    }অন্যথা{
      i64 iVal;
      যদি( op==TK_INTEGER && 0==sqlite3DecOrHexToI64(p_রাশিমালা->u.zToken, &iVal) ){
        sqlite3VdbeMemSetInt64(pVal, iVal*negInt);
      }অন্যথা{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_রাশিমালা->u.zToken);
        যদি( zVal==0 ) গচ্ছ no_mem;
        sqlite3_মান_Set_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        যদি( op==TK_FLOAT ){
          প্রতিজ্ঞা( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_Term) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    যদি( affinity==SQLITE_AFF_BLOB ){
      যদি( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_মান_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }অন্যথা{
      sqlite3_মান_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    প্রতিজ্ঞা( (pVal->flags & MEM_IntReal)==0 );
    যদি( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    যদি( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }অন্যথা যদি( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    যদি( SQLITE_OK==মান_From_রাশিমালা(db,p_রাশিমালা->p_বাম,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      যদি( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }অন্যথা যদি( pVal->u.i==SMALLEST_INT64 ){
#ifndef SQLITE_OMIT_FLOATING_POINT
        pVal->u.r = -(দ্বিগুণ)SMALLEST_INT64;
#অন্যথা
        pVal->u.r = LARGEST_INT64;
#endif
        MemSetTypeFlag(pVal, MEM_Real);
      }অন্যথা{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_মান_Apply_Affinity(pVal, affinity, enc);
    }
  }অন্যথা যদি( op==TK_NULL ){
    pVal = মান_New(db, pCtx);
    যদি( pVal==0 ) গচ্ছ no_mem;
    sqlite3VdbeMemSetNull(pVal);
  }
#ifndef SQLITE_OMIT_BLOB_LITERAL
  অন্যথা যদি( op==TK_BLOB ){
    পূর্ণাঙ্ক nVal;
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
    প্রতিজ্ঞা( p_রাশিমালা->u.zToken[0]=='x' || p_রাশিমালা->u.zToken[0]=='X' );
    প্রতিজ্ঞা( p_রাশিমালা->u.zToken[1]=='\'' );
    pVal = মান_New(db, pCtx);
    যদি( !pVal ) গচ্ছ no_mem;
    zVal = &p_রাশিমালা->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    প্রতিজ্ঞা( zVal[nVal]=='\'' );
    sqlite3VdbeMemSetStr(pVal, sqlite3HexToBlob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#endif
#ifdef SQLITE_ENABLE_STAT4
  অন্যথা যদি( op==TK_FUNCTION && pCtx!=0 ){
    rc = মান_From_Function(db, p_রাশিমালা, enc, affinity, &pVal, pCtx);
  }
#endif
  অন্যথা যদি( op==TK_TRUEFALSE ){
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
    pVal = মান_New(db, pCtx);
    যদি( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_রাশিমালা->u.zToken[4]==0;
      sqlite3_মান_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  প্রত্যাগম rc;

no_mem:
#ifdef SQLITE_ENABLE_STAT4
  যদি( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#endif
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  প্রতিজ্ঞা( *ppVal==0 );
#ifdef SQLITE_ENABLE_STAT4
  যদি( pCtx==0 ) sqlite3_মান_Free(pVal);
#অন্যথা
  প্রতিজ্ঞা( pCtx==0 ); sqlite3_মান_Free(pVal);
#endif
  প্রত্যাগম SQLITE_NOMEM_BKPT;
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
পূর্ণাঙ্ক sqlite3_মান_From_রাশিমালা(
  sqlite3 *db,              /* The database connection */
  ধ্রুব রাশিমালা *p_রাশিমালা,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_মান **ppVal     /* Write the new value here */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  sqlite3_মান *pVal = 0;
  যদি( p_রাশিমালা ){
    rc = মান_From_রাশিমালা(db, p_রাশিমালা, enc, affinity, &pVal, 0);
    যদি( rc==SQLITE_OK && pVal 
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
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক stat4_মান_From_রাশিমালা(
  Parse *pParse,                  /* Parse context */
  রাশিমালা *p_রাশিমালা,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  সংরচনা মান_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_মান **ppVal           /* OUT: New value object (or NULL) */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  sqlite3_মান *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_রাশিমালা = sqlite3_রাশিমালা_Skip_Collate(p_রাশিমালা);

  প্রতিজ্ঞা( p_রাশিমালা==0 || p_রাশিমালা->op!=TK_REGISTER || p_রাশিমালা->op2!=TK_VARIABLE );
  যদি( !p_রাশিমালা ){
    pVal = মান_New(db, pAlloc);
    যদি( pVal ){
      sqlite3VdbeMemSetNull((Mem*)pVal);
    }
  }অন্যথা যদি( p_রাশিমালা->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    পূর্ণাঙ্ক iBindVar = p_রাশিমালা->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    যদি( (v = pParse->pReprepare)!=0 ){
      pVal = মান_New(db, pAlloc);
      যদি( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_মান_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }অন্যথা{
    rc = মান_From_রাশিমালা(db, p_রাশিমালা, ENC(db), affinity, &pVal, pAlloc);
  }

  প্রতিজ্ঞা( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  প্রত্যাগম rc;
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
পূর্ণাঙ্ক sqlite3_Stat4_Probe_Set_মান(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Index being probed */
  UnpackedRecord **ppRec,         /* IN/OUT: Probe record */
  রাশিমালা *p_রাশিমালা,                    /* The expression to extract a value from */
  পূর্ণাঙ্ক nElem,                      /* Maximum number of values to append */
  পূর্ণাঙ্ক iVal,                       /* Array element to populate */
  পূর্ণাঙ্ক *pnExtract                  /* OUT: Values appended to the record */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক nExtract = 0;

  যদি( p_রাশিমালা==0 || p_রাশিমালা->op!=TK_SELECT ){
    পূর্ণাঙ্ক i;
    সংরচনা মান_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    প্রতি(i=0; i<nElem; i++){
      sqlite3_মান *pVal = 0;
      রাশিমালা *pElem = (p_রাশিমালা ? sqlite3VectorFieldSubexpr(p_রাশিমালা, i) : 0);
      u8 aff = sqlite3IndexColumnAffinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_মান_From_রাশিমালা(pParse, pElem, aff, &alloc, &pVal);
      যদি( !pVal ) ভঙ্গ;
      nExtract++;
    }
  }

  *pnExtract = nExtract;
  প্রত্যাগম rc;
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
পূর্ণাঙ্ক sqlite3_Stat4_মান_From_রাশিমালা(
  Parse *pParse,                  /* Parse context */
  রাশিমালা *p_রাশিমালা,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_মান **ppVal           /* OUT: New value object (or NULL) */
){
  প্রত্যাগম stat4_মান_From_রাশিমালা(pParse, p_রাশিমালা, affinity, 0, ppVal);
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
পূর্ণাঙ্ক sqlite3Stat4Column(
  sqlite3 *db,                    /* Database handle */
  ধ্রুব শূন্য *pRec,               /* Pointer to buffer containing record */
  পূর্ণাঙ্ক nRec,                       /* Size of buffer pRec in bytes */
  পূর্ণাঙ্ক iCol,                       /* Column to extract */
  sqlite3_মান **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 iField;                     /* Next unread data byte */
  u32 szField = 0;                /* Size of the current data field */
  পূর্ণাঙ্ক i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  প্রতিজ্ঞা( iCol>0 );
  iHdr = getVarint32(a, nHdr);
  যদি( nHdr>(u32)nRec || iHdr>=nHdr ) প্রত্যাগম SQLITE_CORRUPT_BKPT;
  iField = nHdr;
  প্রতি(i=0; i<=iCol; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    যদি( iHdr>nHdr ) প্রত্যাগম SQLITE_CORRUPT_BKPT;
    szField = sqlite3VdbeSerialTypeLen(t);
    iField += szField;
  }
  testcase( iField==nRec );
  testcase( iField==nRec+1 );
  যদি( iField>nRec ) প্রত্যাগম SQLITE_CORRUPT_BKPT;
  যদি( pMem==0 ){
    pMem = *ppVal = sqlite3_মান_New(db);
    যদি( pMem==0 ) প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[iField-szField], t, pMem);
  pMem->enc = ENC(db);
  প্রত্যাগম SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
শূন্য sqlite3Stat4ProbeFree(UnpackedRecord *pRec){
  যদি( pRec ){
    পূর্ণাঙ্ক i;
    পূর্ণাঙ্ক nCol = pRec->p_চাবি_Info->nAllField;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    প্রতি(i=0; i<nCol; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_চাবি_Info_Unref(pRec->p_চাবি_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#endif /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
শূন্য sqlite3_মান_Set_Str(
  sqlite3_মান *v,     /* Value to be set */
  পূর্ণাঙ্ক n,                /* Length of string z */
  ধ্রুব শূন্য *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  শূন্য (*xDel)(শূন্য*)   /* Destructor for the string */
){
  যদি( v ) sqlite3VdbeMemSetStr((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
শূন্য sqlite3_মান_Free(sqlite3_মান *v){
  যদি( !v ) প্রত্যাগম;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক মান_Bytes(sqlite3_মান *pVal, u8 enc){
  প্রত্যাগম মান_To_Text(pVal, enc)!=0 ? pVal->n : 0;
}
পূর্ণাঙ্ক sqlite3_মান_Bytes(sqlite3_মান *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  প্রতিজ্ঞা( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  যদি( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    প্রত্যাগম p->n;
  }
  যদি( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    প্রত্যাগম p->n;
  }
  যদি( (p->flags & MEM_Blob)!=0 ){
    যদি( p->flags & MEM_Zero ){
      প্রত্যাগম p->n + p->u.nZero;
    }অন্যথা{
      প্রত্যাগম p->n;
    }
  }
  যদি( p->flags & MEM_Null ) প্রত্যাগম 0;
  প্রত্যাগম মান_Bytes(pVal, enc);
}
