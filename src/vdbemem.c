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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "vdbeInt.h"

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
പൂർണ്ണസംഖ്യ sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  പ്രതിജ്ഞാ( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  പ്രതിജ്ഞാ( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  പ്രതിജ്ഞാ( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  എങ്കിൽ( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    പ്രതിജ്ഞാ( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    എങ്കിൽ( (p->flags & (MEM_Term|MEM_Subtype))==(MEM_Term|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      പ്രതിജ്ഞാ( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      പ്രതിജ്ഞാ( (p->flags & ~(MEM_Null|MEM_Term|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }അല്ലെങ്കിൽ{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }അല്ലെങ്കിൽ{
    /* The MEM_Cleared bit is only allowed on NULLs */
    പ്രതിജ്ഞാ( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  പ്രതിജ്ഞാ( p->szMalloc==0
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
  എങ്കിൽ( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    പ്രതിജ്ഞാ( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  തിരികെ 1;
}
#endif

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
സ്ഥിതം ശൂന്യം vdbe_Mem_Render_അക്കം(പൂർണ്ണസംഖ്യ sz, അക്ഷരം *zBuf, Mem *p){
  StrAccum acc;
  പ്രതിജ്ഞാ( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  പ്രതിജ്ഞാ( sz>22 );
  എങ്കിൽ( p->flags & (MEM_Int|MEM_IntReal) ){
#എങ്കിൽ GCC_VERSION>=7000000 && GCC_VERSION<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    പ്രതിജ്ഞാ( (sqlite3Config.bSmallMalloc!=0xee)*8==വലുപ്പം(x) );
    memcpy(&x, (അക്ഷരം*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3Int64ToText(x, zBuf);
#അല്ലെങ്കിൽ
    p->n = sqlite3Int64ToText(p->u.i, zBuf);
#endif
    എങ്കിൽ( p->flags & MEM_IntReal ){
      memcpy(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }അല്ലെങ്കിൽ{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    പ്രതിജ്ഞാ( acc.zText==zBuf && acc.mxAlloc<=0 );
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  അക്ഷരം zBuf[100];
  അക്ഷരം *z;
  പൂർണ്ണസംഖ്യ i, j, incr;
  എങ്കിൽ( (p->flags & MEM_Str)==0 ) തിരികെ 1;
  എങ്കിൽ( p->db && p->db->mallocFailed ) തിരികെ 1;
  എങ്കിൽ( p->flags & MEM_Term ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    എങ്കിൽ( p->szMalloc>0 && p->z==p->zMalloc ){
      പ്രതിജ്ഞാ( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      പ്രതിജ്ഞാ( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    പ്രതിജ്ഞാ( p->z[p->n]==0 );
    പ്രതിജ്ഞാ( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    പ്രതിജ്ഞാ( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  എങ്കിൽ( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) തിരികെ 1;
  എങ്കിൽ( p->db==0 ){
    തിരികെ 1;  /* db->nFpDigit required to validate p->z[] */
  }
  memcpy(&tmp, p, വലുപ്പം(tmp));
  vdbe_Mem_Render_അക്കം(വലുപ്പം(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  എങ്കിൽ( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    എങ്കിൽ( p->enc==SQLITE_UTF16BE ) z++;
  }
  ആകുമ്പോൾ( zBuf[j] ){
    എങ്കിൽ( zBuf[j++]!=z[i] ) തിരികെ 0;
    i += incr;
  }
  തിരികെ 1;
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
പൂർണ്ണസംഖ്യ sqlite3VdbeChangeEncoding(Mem *pMem, പൂർണ്ണസംഖ്യ desiredEnc){
#ifndef SQLITE_OMIT_UTF16
  പൂർണ്ണസംഖ്യ rc;
#endif
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  പ്രതിജ്ഞാ( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  എങ്കിൽ( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    തിരികെ SQLITE_OK;
  }
  എങ്കിൽ( pMem->enc==desiredEnc ){
    തിരികെ SQLITE_OK;
  }
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ifdef SQLITE_OMIT_UTF16
  തിരികെ SQLITE_ERROR;
#അല്ലെങ്കിൽ

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  പ്രതിജ്ഞാ(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  പ്രതിജ്ഞാ(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  പ്രതിജ്ഞാ(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  തിരികെ rc;
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
SQLITE_NOINLINE പൂർണ്ണസംഖ്യ sqlite3VdbeMemGrow(Mem *pMem, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ bPreserve){
  പ്രതിജ്ഞാ( sqlite3VdbeCheckMemInvariants(pMem) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  പ്രതിജ്ഞാ( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  പ്രതിജ്ഞാ( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3DbMallocSize(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3DbMallocSize(pMem->db,pMem->zMalloc));
  എങ്കിൽ( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    എങ്കിൽ( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }അല്ലെങ്കിൽ{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      എങ്കിൽ( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  എങ്കിൽ( pMem->zMalloc==0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    തിരികെ SQLITE_NOMEM_BKPT;
  }അല്ലെങ്കിൽ{
    pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
  }

  എങ്കിൽ( bPreserve && pMem->z ){
    പ്രതിജ്ഞാ( pMem->z!=pMem->zMalloc );
    memcpy(pMem->zMalloc, pMem->z, pMem->n);
  }
  എങ്കിൽ( (pMem->flags&MEM_Dyn)!=0 ){
    പ്രതിജ്ഞാ( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((ശൂന്യം *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemClearAndResize(Mem *pMem, പൂർണ്ണസംഖ്യ szNew){
  പ്രതിജ്ഞാ( CORRUPT_DB || szNew>0 );
  പ്രതിജ്ഞാ( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  എങ്കിൽ( pMem->szMalloc<szNew ){
    തിരികെ sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  പ്രതിജ്ഞാ( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  എങ്കിൽ( (pMem->flags & (MEM_Str|MEM_Term|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    തിരികെ 0;
  }
  എങ്കിൽ( pMem->enc!=SQLITE_UTF8 ) തിരികെ 0;
  പ്രതിജ്ഞാ( pMem->z!=0 );
  എങ്കിൽ( pMem->flags & MEM_Dyn ){
    എങ്കിൽ( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_Term;
      തിരികെ 1;
    }
    എങ്കിൽ( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_Term;
      തിരികെ 1;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_Term;
    തിരികെ 1;
  }
  തിരികെ 0;
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ vdbeMemAddTerminator(Mem *pMem){
  എങ്കിൽ( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_Term;
  തിരികെ SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemMakeWriteable(Mem *pMem){
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  എങ്കിൽ( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    എങ്കിൽ( ExpandBlob(pMem) ) തിരികെ SQLITE_NOMEM;
    എങ്കിൽ( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      പൂർണ്ണസംഖ്യ rc = vdbeMemAddTerminator(pMem);
      എങ്കിൽ( rc ) തിരികെ rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ifdef SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#endif

  തിരികെ SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ifndef SQLITE_OMIT_INCRBLOB
പൂർണ്ണസംഖ്യ sqlite3VdbeMemExpandBlob(Mem *pMem){
  പൂർണ്ണസംഖ്യ nByte;
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->flags & MEM_Zero );
  പ്രതിജ്ഞാ( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_മൂല്യം_nochange(pMem) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  എങ്കിൽ( nByte<=0 ){
    എങ്കിൽ( (pMem->flags & MEM_Blob)==0 ) തിരികെ SQLITE_OK;
    nByte = 1;
  }
  എങ്കിൽ( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }
  പ്രതിജ്ഞാ( pMem->z!=0 );
  പ്രതിജ്ഞാ( sqlite3DbMallocSize(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_Term);
  തിരികെ SQLITE_OK;
}
#endif

/*
** Make sure the given Mem is \u0000 terminated.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemNulTerminate(Mem *pMem){
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==(MEM_Term|MEM_Str) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==0 );
  എങ്കിൽ( (pMem->flags & (MEM_Term|MEM_Str))!=MEM_Str ){
    തിരികെ SQLITE_OK;   /* Nothing to do */
  }അല്ലെങ്കിൽ{
    തിരികെ vdbeMemAddTerminator(pMem);
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  സ്ഥിരം പൂർണ്ണസംഖ്യ nByte = 32;

  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( !(pMem->flags&MEM_Zero) );
  പ്രതിജ്ഞാ( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  പ്രതിജ്ഞാ( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pMem) );


  എങ്കിൽ( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    തിരികെ SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_അക്കം(nByte, pMem->z, pMem);
  പ്രതിജ്ഞാ( pMem->z!=0 );
  പ്രതിജ്ഞാ( pMem->n==(പൂർണ്ണസംഖ്യ)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_Term;
  എങ്കിൽ( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  തിരികെ SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  പ്രതിജ്ഞാ( pFunc!=0 );
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db!=0 );
  പ്രതിജ്ഞാ( pFunc->xFinalize!=0 );
  പ്രതിജ്ഞാ( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, വലുപ്പം(ctx));
  memset(&t, 0, വലുപ്പം(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.pOut = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  പ്രതിജ്ഞാ( (pMem->flags & MEM_Dyn)==0 );
  എങ്കിൽ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  memcpy(pMem, &t, വലുപ്പം(t));
  തിരികെ ctx.isError;
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
പൂർണ്ണസംഖ്യ sqlite3_Vdbe_Mem_Agg_മൂല്യം(Mem *pAccum, Mem *pOut, FuncDef *pFunc){
  sqlite3_context ctx;
  പ്രതിജ്ഞാ( pFunc!=0 );
  പ്രതിജ്ഞാ( pFunc->x_മൂല്യം!=0 );
  പ്രതിജ്ഞാ( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  പ്രതിജ്ഞാ( pAccum->db!=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, വലുപ്പം(ctx));
  sqlite3VdbeMemSetNull(pOut);
  ctx.pOut = pOut;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_മൂല്യം(&ctx);
  തിരികെ ctx.isError;
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
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം vdbeMemClearExternAndSetNull(Mem *p){
  പ്രതിജ്ഞാ( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  പ്രതിജ്ഞാ( VdbeMemDynamic(p) );
  എങ്കിൽ( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    പ്രതിജ്ഞാ( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  എങ്കിൽ( p->flags&MEM_Dyn ){
    പ്രതിജ്ഞാ( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((ശൂന്യം *)p->z);
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
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം vdbeMemClear(Mem *p){
  എങ്കിൽ( VdbeMemDynamic(p) ){
    vdbeMemClearExternAndSetNull(p);
  }
  എങ്കിൽ( p->szMalloc ){
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
ശൂന്യം sqlite3VdbeMemRelease(Mem *p){
  പ്രതിജ്ഞാ( sqlite3VdbeCheckMemInvariants(p) );
  എങ്കിൽ( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
ശൂന്യം sqlite3VdbeMemReleaseMalloc(Mem *p){
  പ്രതിജ്ഞാ( !VdbeMemDynamic(p) );
  എങ്കിൽ( p->szMalloc ) vdbeMemClear(p);
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
സ്ഥിതം SQLITE_NOINLINE i64 mem_Int_മൂല്യം(സ്ഥിരം Mem *pMem){
  i64 മൂല്യം = 0;
  sqlite3Atoi64(pMem->z, &മൂല്യം, pMem->n, pMem->enc);
  തിരികെ മൂല്യം;
}
i64 sqlite3_Vdbe_Int_മൂല്യം(സ്ഥിരം Mem *pMem){
  പൂർണ്ണസംഖ്യ flags;
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  എങ്കിൽ( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    തിരികെ pMem->u.i;
  }അല്ലെങ്കിൽ എങ്കിൽ( flags & MEM_Real ){
    തിരികെ sqlite3RealToI64(pMem->u.r);
  }അല്ലെങ്കിൽ എങ്കിൽ( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    തിരികെ mem_Int_മൂല്യം(pMem);
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ sqlite3_Mem_Real_മൂല്യം_RCSlow_Path(
  Mem *pMem,
  ഇരട്ട *p_മൂല്യം
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  *p_മൂല്യം = 0.0;
  എങ്കിൽ( pMem->enc==SQLITE_UTF8 ){
    അക്ഷരം *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    എങ്കിൽ( zCopy ){
      rc = sqlite3AtoF(zCopy, p_മൂല്യം);
      sqlite3DbFree(pMem->db, zCopy);
    }
    തിരികെ rc;
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ n, i, j;
    അക്ഷരം *zCopy;
    സ്ഥിരം അക്ഷരം *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    എങ്കിൽ( zCopy ){
      z = pMem->z;
      എങ്കിൽ( pMem->enc==SQLITE_UTF16LE ){
        വേണ്ടി(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          എങ്കിൽ( z[i+1]!=0 ) നിർത്തുക;
        }
      }അല്ലെങ്കിൽ{
        വേണ്ടി(i=j=0; i<n-1; i+=2, j++){
          എങ്കിൽ( z[i]!=0 ) നിർത്തുക;
          zCopy[j] = z[i+1];
        }
      }     
      പ്രതിജ്ഞാ( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_മൂല്യം);
      എങ്കിൽ( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sqlite3_Mem_Real_മൂല്യം_RC(Mem *pMem, ഇരട്ട *p_മൂല്യം){
  testcase( pMem->db==0 );
  പ്രതിജ്ഞാ( pMem->flags & (MEM_Str|MEM_Blob) );
  എങ്കിൽ( pMem->z==0 ){
    *p_മൂല്യം = 0.0;
    തിരികെ 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_Term)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    തിരികെ sqlite3AtoF(pMem->z, p_മൂല്യം);
  }അല്ലെങ്കിൽ എങ്കിൽ( pMem->n==0 ){
    *p_മൂല്യം = 0.0;
    തിരികെ 0;
  }അല്ലെങ്കിൽ{
    തിരികെ sqlite3_Mem_Real_മൂല്യം_RCSlow_Path(pMem, p_മൂല്യം);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
സ്ഥിതം SQLITE_NOINLINE ഇരട്ട sqlite3_Mem_Real_മൂല്യം_No_RC(Mem *pMem){
  ഇരട്ട r;
  (ശൂന്യം)sqlite3_Mem_Real_മൂല്യം_RC(pMem, &r);
  തിരികെ r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
ഇരട്ട sqlite3_Vdbe_Real_മൂല്യം(Mem *pMem){
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pMem) );
  എങ്കിൽ( pMem->flags & MEM_Real ){
    തിരികെ pMem->u.r;
  }അല്ലെങ്കിൽ എങ്കിൽ( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    തിരികെ (ഇരട്ട)pMem->u.i;
  }അല്ലെങ്കിൽ എങ്കിൽ( pMem->flags & (MEM_Str|MEM_Blob) ){
    തിരികെ sqlite3_Mem_Real_മൂല്യം_No_RC(pMem);
  }അല്ലെങ്കിൽ{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    തിരികെ (ഇരട്ട)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
പൂർണ്ണസംഖ്യ sqlite3_Vdbe_Boolean_മൂല്യം(Mem *pMem, പൂർണ്ണസംഖ്യ ifNull){
  testcase( pMem->flags & MEM_IntReal );
  എങ്കിൽ( pMem->flags & (MEM_Int|MEM_IntReal) ) തിരികെ pMem->u.i!=0;
  എങ്കിൽ( pMem->flags & MEM_Null ) തിരികെ ifNull;
  തിരികെ sqlite3_Vdbe_Real_മൂല്യം(pMem)!=0.0;
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
ശൂന്യം sqlite3VdbeIntegerAffinity(Mem *pMem, പൂർണ്ണസംഖ്യ bFromText){
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->flags & (MEM_Real|MEM_IntReal) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pMem) );

  എങ്കിൽ( pMem->flags & MEM_IntReal ){
    MemSetTypeFlag(pMem, MEM_Int);
  }അല്ലെങ്കിൽ{
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
    എങ്കിൽ( pMem->u.r==ix && ix<LARGEST_INT64
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemIntegerify(Mem *pMem){
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_മൂല്യം(pMem);
  MemSetTypeFlag(pMem, MEM_Int);
  തിരികെ SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemRealify(Mem *pMem){
  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_മൂല്യം(pMem);
  MemSetTypeFlag(pMem, MEM_Real);
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3RealSameAsInt(ഇരട്ട r1, sqlite3_int64 i){
  ഇരട്ട r2 = (ഇരട്ട)i;
  തിരികെ r1==0.0
      || (memcmp(&r1, &r2, വലുപ്പം(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3RealToI64(ഇരട്ട r){
  എങ്കിൽ( r<-9223372036854774784.0 ) തിരികെ SMALLEST_INT64;
  എങ്കിൽ( r>+9223372036854774784.0 ) തിരികെ LARGEST_INT64;
  തിരികെ (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemNumerify(Mem *pMem){
  പ്രതിജ്ഞാ( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  എങ്കിൽ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    പൂർണ്ണസംഖ്യ rc;
    sqlite3_int64 ix;
    പ്രതിജ്ഞാ( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_മൂല്യം_RC(pMem, &pMem->u.r);
    എങ്കിൽ( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3RealToI64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      MemSetTypeFlag(pMem, MEM_Int);
    }അല്ലെങ്കിൽ{
      MemSetTypeFlag(pMem, MEM_Real);
    }
  }
  പ്രതിജ്ഞാ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  തിരികെ SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  എങ്കിൽ( pMem->flags & MEM_Null ) തിരികെ SQLITE_OK;
  തിരഞ്ഞെടുപ്പ്( aff ){
    അവസ്ഥ SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      എങ്കിൽ( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_മൂല്യം_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
        പ്രതിജ്ഞാ( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        എങ്കിൽ( pMem->flags & MEM_Str ) MemSetTypeFlag(pMem, MEM_Blob);
      }അല്ലെങ്കിൽ{
        pMem->flags &= ~(MEM_TypeMask&~MEM_Blob);
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      പൂർണ്ണസംഖ്യ rc;
      പ്രതിജ്ഞാ( aff==SQLITE_AFF_TEXT );
      പ്രതിജ്ഞാ( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_മൂല്യം_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
      പ്രതിജ്ഞാ( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      എങ്കിൽ( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      എങ്കിൽ( rc ) തിരികെ rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  തിരികെ SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
ശൂന്യം sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  പ്രതിജ്ഞാ( (flags & ~MEM_TypeMask)==0 );
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
ശൂന്യം sqlite3VdbeMemSetNull(Mem *pMem){
  എങ്കിൽ( VdbeMemDynamic(pMem) ){
    vdbeMemClearExternAndSetNull(pMem);
  }അല്ലെങ്കിൽ{
    pMem->flags = MEM_Null;
  }
}
ശൂന്യം sqlite3_മൂല്യം_Set_Null(sqlite3_മൂല്യം *p){
  sqlite3VdbeMemSetNull((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ifndef SQLITE_OMIT_INCRBLOB
ശൂന്യം sqlite3VdbeMemSetZeroBlob(Mem *pMem, പൂർണ്ണസംഖ്യ n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  എങ്കിൽ( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ sqlite3VdbeMemSetZeroBlob(Mem *pMem, പൂർണ്ണസംഖ്യ n){
  പൂർണ്ണസംഖ്യ nByte = n>0?n:1;
  എങ്കിൽ( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }
  പ്രതിജ്ഞാ( pMem->z!=0 );
  പ്രതിജ്ഞാ( sqlite3DbMallocSize(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  തിരികെ SQLITE_OK;
}
#endif

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം vdbeReleaseAndSetInt64(Mem *pMem, i64 val){
  sqlite3VdbeMemSetNull(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
ശൂന്യം sqlite3VdbeMemSetInt64(Mem *pMem, i64 val){
  എങ്കിൽ( VdbeMemDynamic(pMem) ){
    vdbeReleaseAndSetInt64(pMem, val);
  }അല്ലെങ്കിൽ{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
ശൂന്യം sqlite3MemSetArrayInt64(sqlite3_മൂല്യം *aMem, പൂർണ്ണസംഖ്യ iIdx, i64 val){
  sqlite3VdbeMemSetInt64(&aMem[iIdx], val);
}

/* A no-op destructor */
ശൂന്യം sqlite3NoopDestructor(ശൂന്യം *p){ UNUSED_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
ശൂന്യം sqlite3VdbeMemSetPointer(
  Mem *pMem,
  ശൂന്യം *pPtr,
  സ്ഥിരം അക്ഷരം *zPType,
  ശൂന്യം (*xDestructor)(ശൂന്യം*)
){
  പ്രതിജ്ഞാ( pMem->flags==MEM_Null );
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
ശൂന്യം sqlite3VdbeMemSetDouble(Mem *pMem, ഇരട്ട val){
  sqlite3VdbeMemSetNull(pMem);
  എങ്കിൽ( !sqlite3IsNaN(val) ){
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemIsRowSet(സ്ഥിരം Mem *pMem){
  തിരികെ (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemSetRowSet(Mem *pMem){
  sqlite3 *db = pMem->db;
  RowSet *p;
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3RowSetInit(db);
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM;
  pMem->z = (അക്ഷരം*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3RowSetDelete;
  തിരികെ SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemTooBig(Mem *p){
  പ്രതിജ്ഞാ( p->db!=0 );
  എങ്കിൽ( p->flags & (MEM_Str|MEM_Blob) ){
    പൂർണ്ണസംഖ്യ n = p->n;
    എങ്കിൽ( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    തിരികെ n>p->db->aLimit[SQLITE_LIMIT_LENGTH];
  }
  തിരികെ 0; 
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
ശൂന്യം sqlite3VdbeMemAboutToChange(Vdbe *pVdbe, Mem *pMem){
  പൂർണ്ണസംഖ്യ i;
  Mem *pX;
  എങ്കിൽ( pMem->bScopy ){
    വേണ്ടി(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      എങ്കിൽ( pX->pScopyFrom==pMem ){
        u16 mFlags;
        എങ്കിൽ( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (പൂർണ്ണസംഖ്യ)(pX - pVdbe->aMem), (പൂർണ്ണസംഖ്യ)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        പ്രതിജ്ഞാ( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം vdbeClrCopy(Mem *pTo, സ്ഥിരം Mem *pFrom, പൂർണ്ണസംഖ്യ eType){
  vdbeMemClearExternAndSetNull(pTo);
  പ്രതിജ്ഞാ( !VdbeMemDynamic(pTo) );
  sqlite3VdbeMemShallowCopy(pTo, pFrom, eType);
}
ശൂന്യം sqlite3VdbeMemShallowCopy(Mem *pTo, സ്ഥിരം Mem *pFrom, പൂർണ്ണസംഖ്യ srcType){
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pFrom) );
  പ്രതിജ്ഞാ( pTo->db==pFrom->db );
  എങ്കിൽ( VdbeMemDynamic(pTo) ){ vdbeClrCopy(pTo,pFrom,srcType); തിരികെ; }
  memcpy(pTo, pFrom, MEMCELLSIZE);
  എങ്കിൽ( (pFrom->flags&MEM_Static)==0 ){
    pTo->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    പ്രതിജ്ഞാ( srcType==MEM_Ephem || srcType==MEM_Static );
    pTo->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemCopy(Mem *pTo, സ്ഥിരം Mem *pFrom){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pFrom) );
  എങ്കിൽ( VdbeMemDynamic(pTo) ) vdbeMemClearExternAndSetNull(pTo);
  memcpy(pTo, pFrom, MEMCELLSIZE);
  pTo->flags &= ~MEM_Dyn;
  എങ്കിൽ( pTo->flags&(MEM_Str|MEM_Blob) ){
    എങ്കിൽ( 0==(pFrom->flags&MEM_Static) ){
      pTo->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(pTo);
    }
  }

  തിരികെ rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
ശൂന്യം sqlite3VdbeMemMove(Mem *pTo, Mem *pFrom){
  പ്രതിജ്ഞാ( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  പ്രതിജ്ഞാ( pTo->db==0 || sqlite3_mutex_held(pTo->db->mutex) );
  പ്രതിജ്ഞാ( pFrom->db==0 || pTo->db==0 || pFrom->db==pTo->db );

  sqlite3VdbeMemRelease(pTo);
  memcpy(pTo, pFrom, വലുപ്പം(Mem));
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemSetStr(
  Mem *pMem,          /* Memory cell to set to string value */
  സ്ഥിരം അക്ഷരം *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  ശൂന്യം (*xDel)(ശൂന്യം*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  പൂർണ്ണസംഖ്യ iLimit;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  പ്രതിജ്ഞാ( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  എങ്കിൽ( !z ){
    sqlite3VdbeMemSetNull(pMem);
    തിരികെ SQLITE_OK;
  }

  എങ്കിൽ( pMem->db ){
    iLimit = pMem->db->aLimit[SQLITE_LIMIT_LENGTH];
  }അല്ലെങ്കിൽ{
    iLimit = SQLITE_MAX_LENGTH;
  }
  എങ്കിൽ( nByte<0 ){
    പ്രതിജ്ഞാ( enc!=0 );
    എങ്കിൽ( enc==SQLITE_UTF8 ){
      nByte = strlen(z);
    }അല്ലെങ്കിൽ{
      വേണ്ടി(nByte=0; nByte<=iLimit && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_Term;
  }അല്ലെങ്കിൽ എങ്കിൽ( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }അല്ലെങ്കിൽ{
    flags = MEM_Str;
  }
  എങ്കിൽ( nByte>iLimit ){
    എങ്കിൽ( xDel && xDel!=SQLITE_TRANSIENT ){
      എങ്കിൽ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (ശൂന്യം*)z);
      }അല്ലെങ്കിൽ{
        xDel((ശൂന്യം*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    തിരികെ sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  എങ്കിൽ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    എങ്കിൽ( flags&MEM_Term ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    എങ്കിൽ( sqlite3VdbeMemClearAndResize(pMem, (പൂർണ്ണസംഖ്യ)MAX(nAlloc,32)) ){
      തിരികെ SQLITE_NOMEM_BKPT;
    }
    പ്രതിജ്ഞാ( pMem->z!=0 );
    memcpy(pMem->z, z, nAlloc);
  }അല്ലെങ്കിൽ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (അക്ഷരം *)z;
    എങ്കിൽ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
    }അല്ലെങ്കിൽ{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (പൂർണ്ണസംഖ്യ)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ifndef SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  എങ്കിൽ( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }
#endif


  തിരികെ SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
പൂർണ്ണസംഖ്യ sqlite3VdbeMemSetText(
  Mem *pMem,          /* Memory cell to set to string value */
  സ്ഥിരം അക്ഷരം *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  ശൂന്യം (*xDel)(ശൂന്യം*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  പ്രതിജ്ഞാ( pMem!=0 );
  പ്രതിജ്ഞാ( pMem->db!=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pMem->db->mutex) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  എങ്കിൽ( !z ){
    sqlite3VdbeMemSetNull(pMem);
    തിരികെ SQLITE_OK;
  }

  എങ്കിൽ( nByte<0 ){
    nByte = strlen(z);
    flags = MEM_Str|MEM_Term;
  }അല്ലെങ്കിൽ{
    flags = MEM_Str;
  }
  എങ്കിൽ( nByte>(i64)pMem->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    എങ്കിൽ( xDel && xDel!=SQLITE_TRANSIENT ){
      എങ്കിൽ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (ശൂന്യം*)z);
      }അല്ലെങ്കിൽ{
        xDel((ശൂന്യം*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    തിരികെ sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  എങ്കിൽ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    എങ്കിൽ( sqlite3VdbeMemClearAndResize(pMem, (പൂർണ്ണസംഖ്യ)MAX(nAlloc,32)) ){
      തിരികെ SQLITE_NOMEM_BKPT;
    }
    പ്രതിജ്ഞാ( pMem->z!=0 );
    memcpy(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }അല്ലെങ്കിൽ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (അക്ഷരം *)z;
    എങ്കിൽ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }അല്ലെങ്കിൽ{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (പൂർണ്ണസംഖ്യ)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 offset,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  പൂർണ്ണസംഖ്യ rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE-1 );
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE );
  എങ്കിൽ( amt>=SQLITE_MAX_ALLOCATION_SIZE ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }
  എങ്കിൽ( (u64)amt + (u64)offset > (u64)sqlite3BtreeMaxRecordSize(pCur) ){
    തിരികെ SQLITE_CORRUPT_BKPT;
  }
  എങ്കിൽ( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, offset, amt, pMem->z);
    എങ്കിൽ( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (പൂർണ്ണസംഖ്യ)amt;
    }അല്ലെങ്കിൽ{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3VdbeMemFromBtreeZeroOffset(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 available = 0;  /* Number of bytes available on the local btree page */
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK; /* Return code */

  പ്രതിജ്ഞാ( sqlite3BtreeCursorIsValid(pCur) );
  പ്രതിജ്ഞാ( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pMem) );
  pMem->z = (അക്ഷരം *)sqlite3BtreePayloadFetch(pCur, &available);
  പ്രതിജ്ഞാ( pMem->z!=0 );

  എങ്കിൽ( amt<=available ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (പൂർണ്ണസംഖ്യ)amt;
  }അല്ലെങ്കിൽ{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  തിരികെ rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
സ്ഥിതം SQLITE_NOINLINE സ്ഥിരം ശൂന്യം *മൂല്യം_To_Text(sqlite3_മൂല്യം* pVal, u8 enc){
  പ്രതിജ്ഞാ( pVal!=0 );
  പ്രതിജ്ഞാ( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  പ്രതിജ്ഞാ( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pVal) );
  പ്രതിജ്ഞാ( (pVal->flags & (MEM_Null))==0 );
  എങ്കിൽ( pVal->flags & (MEM_Blob|MEM_Str) ){
    എങ്കിൽ( ExpandBlob(pVal) ) തിരികെ 0;
    pVal->flags |= MEM_Str;
    എങ്കിൽ( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    എങ്കിൽ( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_TO_INT(pVal->z)) ){
      പ്രതിജ്ഞാ( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      എങ്കിൽ( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        തിരികെ 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }അല്ലെങ്കിൽ{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    പ്രതിജ്ഞാ( 0==(1&SQLITE_PTR_TO_INT(pVal->z)) );
  }
  പ്രതിജ്ഞാ(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  എങ്കിൽ( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    പ്രതിജ്ഞാ( sqlite3VdbeMemValidStrRep(pVal) );
    തിരികെ pVal->z;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
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
സ്ഥിരം ശൂന്യം *sqlite3_മൂല്യം_Text(sqlite3_മൂല്യം* pVal, u8 enc){
  എങ്കിൽ( !pVal ) തിരികെ 0;
  പ്രതിജ്ഞാ( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  പ്രതിജ്ഞാ( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  പ്രതിജ്ഞാ( !sqlite3VdbeMemIsRowSet(pVal) );
  എങ്കിൽ( (pVal->flags&(MEM_Str|MEM_Term))==(MEM_Str|MEM_Term) && pVal->enc==enc ){
    പ്രതിജ്ഞാ( sqlite3VdbeMemValidStrRep(pVal) );
    തിരികെ pVal->z;
  }
  എങ്കിൽ( pVal->flags&MEM_Null ){
    തിരികെ 0;
  }
  തിരികെ മൂല്യം_To_Text(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_Is_Of_Class(സ്ഥിരം sqlite3_മൂല്യം *pVal, ശൂന്യം(*xFree)(ശൂന്യം*)){
  എങ്കിൽ( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_മൂല്യം *sqlite3_മൂല്യം_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, വലുപ്പം(*p));
  എങ്കിൽ( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  തിരികെ p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
ഘടന മൂല്യം_New_Stat4_Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  പൂർണ്ണസംഖ്യ iVal;
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
സ്ഥിതം sqlite3_മൂല്യം *മൂല്യം_New(sqlite3 *db, ഘടന മൂല്യം_New_Stat4_Ctx *p){
#ifdef SQLITE_ENABLE_STAT4
  എങ്കിൽ( p ){
    UnpackedRecord *pRec = p->ppRec[0];

    എങ്കിൽ( pRec==0 ){
      Index *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      പൂർണ്ണസംഖ്യ i;                      /* Counter variable */
      പൂർണ്ണസംഖ്യ nCol = pIdx->nColumn;   /* Number of index columns including rowid */
  
      nByte = വലുപ്പം(Mem) * nCol + ROUND8(വലുപ്പം(UnpackedRecord));
      pRec = (UnpackedRecord*)sqlite3DbMallocZero(db, nByte);
      എങ്കിൽ( pRec ){
        pRec->p_താക്കോൽ_Info = sqlite3_താക്കോൽ_Info_Of_Index(p->pParse, pIdx);
        എങ്കിൽ( pRec->p_താക്കോൽ_Info ){
          പ്രതിജ്ഞാ( pRec->p_താക്കോൽ_Info->nAllField==nCol );
          പ്രതിജ്ഞാ( pRec->p_താക്കോൽ_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(വലുപ്പം(UnpackedRecord)));
          വേണ്ടി(i=0; i<nCol; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }അല്ലെങ്കിൽ{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      എങ്കിൽ( pRec==0 ) തിരികെ 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->nField = p->iVal+1;
    sqlite3VdbeMemSetNull(&pRec->aMem[p->iVal]);
    തിരികെ &pRec->aMem[p->iVal];
  }
#അല്ലെങ്കിൽ
  UNUSED_PARAMETER(p);
#endif /* defined(SQLITE_ENABLE_STAT4) */
  തിരികെ sqlite3_മൂല്യം_New(db);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ മൂല്യം_From_Function(
  sqlite3 *db,                    /* The database connection */
  സ്ഥിരം വ്യഞ്ജകം *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_മൂല്യം **ppVal,          /* Write the new value here */
  ഘടന മൂല്യം_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_മൂല്യം **apVal = 0;      /* Function arguments */
  പൂർണ്ണസംഖ്യ nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_മൂല്യം *pVal = 0;        /* New value */
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;             /* Return code */
  വ്യഞ്ജകം_List *pList = 0;            /* Function arguments */
  പൂർണ്ണസംഖ്യ i;                          /* Iterator variable */

  പ്രതിജ്ഞാ( pCtx!=0 );
  പ്രതിജ്ഞാ( (p->flags & EP_TokenOnly)==0 );
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p) );
  pList = p->x.pList;
  എങ്കിൽ( pList ) nVal = pList->n_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_Int_മൂല്യം) );
  pFunc = sqlite3FindFunction(db, p->u.zToken, nVal, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  എങ്കിൽ( pFunc==0 ) തിരികെ SQLITE_OK;
#endif
  പ്രതിജ്ഞാ( pFunc );
  എങ്കിൽ( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    തിരികെ SQLITE_OK;
  }

  എങ്കിൽ( pList ){
    apVal = (sqlite3_മൂല്യം**)sqlite3DbMallocZero(db, വലുപ്പം(apVal[0]) * nVal);
    എങ്കിൽ( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      പോകുക മൂല്യം_from_function_out;
    }
    വേണ്ടി(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_മൂല്യം_From_വ്യഞ്ജകം(pCtx->pParse, pList->a[i].p_വ്യഞ്ജകം, aff,
                                     &apVal[i]);
      എങ്കിൽ( apVal[i]==0 || rc!=SQLITE_OK ) പോകുക മൂല്യം_from_function_out;
    }
  }

  pVal = മൂല്യം_New(db, pCtx);
  എങ്കിൽ( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    പോകുക മൂല്യം_from_function_out;
  }

  memset(&ctx, 0, വലുപ്പം(ctx));
  ctx.pOut = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  എങ്കിൽ( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_സന്ദേശം(pCtx->pParse, "%s", sqlite3_മൂല്യം_text(pVal));
  }അല്ലെങ്കിൽ{
    sqlite3_മൂല്യം_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    പ്രതിജ്ഞാ( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    എങ്കിൽ( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 മൂല്യം_from_function_out:
  എങ്കിൽ( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  എങ്കിൽ( apVal ){
    വേണ്ടി(i=0; i<nVal; i++){
      sqlite3_മൂല്യം_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  തിരികെ rc;
}
#അല്ലെങ്കിൽ
# define മൂല്യം_From_Function(a,b,c,d,e,f) SQLITE_OK
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
സ്ഥിതം പൂർണ്ണസംഖ്യ മൂല്യം_From_വ്യഞ്ജകം(
  sqlite3 *db,                    /* The database connection */
  സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_മൂല്യം **ppVal,          /* Write the new value here */
  ഘടന മൂല്യം_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  പൂർണ്ണസംഖ്യ op;
  അക്ഷരം *zVal = 0;
  sqlite3_മൂല്യം *pVal = 0;
  പൂർണ്ണസംഖ്യ negInt = 1;
  സ്ഥിരം അക്ഷരം *zNeg = "";
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
  ആകുമ്പോൾ( (op = p_വ്യഞ്ജകം->op)==TK_UPLUS || op==TK_SPAN ) p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
  എങ്കിൽ( op==TK_REGISTER ) op = p_വ്യഞ്ജകം->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  പ്രതിജ്ഞാ( (p_വ്യഞ്ജകം->flags & EP_TokenOnly)==0 || pCtx==0 );

  എങ്കിൽ( op==TK_CAST ){
    u8 aff;
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
    aff = sqlite3AffinityType(p_വ്യഞ്ജകം->u.zToken,0);
    rc = മൂല്യം_From_വ്യഞ്ജകം(db, p_വ്യഞ്ജകം->p_ഇടത്, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    എങ്കിൽ( *ppVal ){
#ifdef SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#അല്ലെങ്കിൽ
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      പ്രതിജ്ഞാ( (ppVal[0][0].flags & MEM_Zero)==0 );
#endif
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_മൂല്യം_Apply_Affinity(*ppVal, affinity, enc);
    }
    തിരികെ rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  എങ്കിൽ( op==TK_UMINUS ){
    വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
    എങ്കിൽ( (p_ഇടത്->op==TK_INTEGER || p_ഇടത്->op==TK_FLOAT) ){
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_ഇടത്, EP_Int_മൂല്യം)
       || p_ഇടത്->u.zToken[0]!='0' || (p_ഇടത്->u.zToken[1] & ~0x20)!='X'
      ){
        p_വ്യഞ്ജകം = p_ഇടത്;
        op = p_വ്യഞ്ജകം->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  എങ്കിൽ( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = മൂല്യം_New(db, pCtx);
    എങ്കിൽ( pVal==0 ) പോകുക no_mem;
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) ){
      sqlite3VdbeMemSetInt64(pVal, (i64)p_വ്യഞ്ജകം->u.i_മൂല്യം*negInt);
    }അല്ലെങ്കിൽ{
      i64 iVal;
      എങ്കിൽ( op==TK_INTEGER && 0==sqlite3DecOrHexToI64(p_വ്യഞ്ജകം->u.zToken, &iVal) ){
        sqlite3VdbeMemSetInt64(pVal, iVal*negInt);
      }അല്ലെങ്കിൽ{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_വ്യഞ്ജകം->u.zToken);
        എങ്കിൽ( zVal==0 ) പോകുക no_mem;
        sqlite3_മൂല്യം_Set_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        എങ്കിൽ( op==TK_FLOAT ){
          പ്രതിജ്ഞാ( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_Term) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    എങ്കിൽ( affinity==SQLITE_AFF_BLOB ){
      എങ്കിൽ( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_മൂല്യം_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }അല്ലെങ്കിൽ{
      sqlite3_മൂല്യം_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    പ്രതിജ്ഞാ( (pVal->flags & MEM_IntReal)==0 );
    എങ്കിൽ( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    എങ്കിൽ( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    എങ്കിൽ( SQLITE_OK==മൂല്യം_From_വ്യഞ്ജകം(db,p_വ്യഞ്ജകം->p_ഇടത്,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      എങ്കിൽ( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }അല്ലെങ്കിൽ എങ്കിൽ( pVal->u.i==SMALLEST_INT64 ){
#ifndef SQLITE_OMIT_FLOATING_POINT
        pVal->u.r = -(ഇരട്ട)SMALLEST_INT64;
#അല്ലെങ്കിൽ
        pVal->u.r = LARGEST_INT64;
#endif
        MemSetTypeFlag(pVal, MEM_Real);
      }അല്ലെങ്കിൽ{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_മൂല്യം_Apply_Affinity(pVal, affinity, enc);
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_NULL ){
    pVal = മൂല്യം_New(db, pCtx);
    എങ്കിൽ( pVal==0 ) പോകുക no_mem;
    sqlite3VdbeMemSetNull(pVal);
  }
#ifndef SQLITE_OMIT_BLOB_LITERAL
  അല്ലെങ്കിൽ എങ്കിൽ( op==TK_BLOB ){
    പൂർണ്ണസംഖ്യ nVal;
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->u.zToken[0]=='x' || p_വ്യഞ്ജകം->u.zToken[0]=='X' );
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->u.zToken[1]=='\'' );
    pVal = മൂല്യം_New(db, pCtx);
    എങ്കിൽ( !pVal ) പോകുക no_mem;
    zVal = &p_വ്യഞ്ജകം->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    പ്രതിജ്ഞാ( zVal[nVal]=='\'' );
    sqlite3VdbeMemSetStr(pVal, sqlite3HexToBlob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#endif
#ifdef SQLITE_ENABLE_STAT4
  അല്ലെങ്കിൽ എങ്കിൽ( op==TK_FUNCTION && pCtx!=0 ){
    rc = മൂല്യം_From_Function(db, p_വ്യഞ്ജകം, enc, affinity, &pVal, pCtx);
  }
#endif
  അല്ലെങ്കിൽ എങ്കിൽ( op==TK_TRUEFALSE ){
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
    pVal = മൂല്യം_New(db, pCtx);
    എങ്കിൽ( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_വ്യഞ്ജകം->u.zToken[4]==0;
      sqlite3_മൂല്യം_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  തിരികെ rc;

no_mem:
#ifdef SQLITE_ENABLE_STAT4
  എങ്കിൽ( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#endif
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  പ്രതിജ്ഞാ( *ppVal==0 );
#ifdef SQLITE_ENABLE_STAT4
  എങ്കിൽ( pCtx==0 ) sqlite3_മൂല്യം_Free(pVal);
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( pCtx==0 ); sqlite3_മൂല്യം_Free(pVal);
#endif
  തിരികെ SQLITE_NOMEM_BKPT;
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
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_From_വ്യഞ്ജകം(
  sqlite3 *db,              /* The database connection */
  സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_മൂല്യം **ppVal     /* Write the new value here */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  sqlite3_മൂല്യം *pVal = 0;
  എങ്കിൽ( p_വ്യഞ്ജകം ){
    rc = മൂല്യം_From_വ്യഞ്ജകം(db, p_വ്യഞ്ജകം, enc, affinity, &pVal, 0);
    എങ്കിൽ( rc==SQLITE_OK && pVal 
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
  തിരികെ rc;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ stat4_മൂല്യം_From_വ്യഞ്ജകം(
  Parse *pParse,                  /* Parse context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  ഘടന മൂല്യം_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_മൂല്യം **ppVal           /* OUT: New value object (or NULL) */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  sqlite3_മൂല്യം *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Skip_Collate(p_വ്യഞ്ജകം);

  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം==0 || p_വ്യഞ്ജകം->op!=TK_REGISTER || p_വ്യഞ്ജകം->op2!=TK_VARIABLE );
  എങ്കിൽ( !p_വ്യഞ്ജകം ){
    pVal = മൂല്യം_New(db, pAlloc);
    എങ്കിൽ( pVal ){
      sqlite3VdbeMemSetNull((Mem*)pVal);
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    പൂർണ്ണസംഖ്യ iBindVar = p_വ്യഞ്ജകം->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    എങ്കിൽ( (v = pParse->pReprepare)!=0 ){
      pVal = മൂല്യം_New(db, pAlloc);
      എങ്കിൽ( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_മൂല്യം_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }അല്ലെങ്കിൽ{
    rc = മൂല്യം_From_വ്യഞ്ജകം(db, p_വ്യഞ്ജകം, ENC(db), affinity, &pVal, pAlloc);
  }

  പ്രതിജ്ഞാ( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sqlite3_Stat4_Probe_Set_മൂല്യം(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Index being probed */
  UnpackedRecord **ppRec,         /* IN/OUT: Probe record */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,                    /* The expression to extract a value from */
  പൂർണ്ണസംഖ്യ nElem,                      /* Maximum number of values to append */
  പൂർണ്ണസംഖ്യ iVal,                       /* Array element to populate */
  പൂർണ്ണസംഖ്യ *pnExtract                  /* OUT: Values appended to the record */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ nExtract = 0;

  എങ്കിൽ( p_വ്യഞ്ജകം==0 || p_വ്യഞ്ജകം->op!=TK_SELECT ){
    പൂർണ്ണസംഖ്യ i;
    ഘടന മൂല്യം_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    വേണ്ടി(i=0; i<nElem; i++){
      sqlite3_മൂല്യം *pVal = 0;
      വ്യഞ്ജകം *pElem = (p_വ്യഞ്ജകം ? sqlite3VectorFieldSubexpr(p_വ്യഞ്ജകം, i) : 0);
      u8 aff = sqlite3IndexColumnAffinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_മൂല്യം_From_വ്യഞ്ജകം(pParse, pElem, aff, &alloc, &pVal);
      എങ്കിൽ( !pVal ) നിർത്തുക;
      nExtract++;
    }
  }

  *pnExtract = nExtract;
  തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sqlite3_Stat4_മൂല്യം_From_വ്യഞ്ജകം(
  Parse *pParse,                  /* Parse context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_മൂല്യം **ppVal           /* OUT: New value object (or NULL) */
){
  തിരികെ stat4_മൂല്യം_From_വ്യഞ്ജകം(pParse, p_വ്യഞ്ജകം, affinity, 0, ppVal);
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
പൂർണ്ണസംഖ്യ sqlite3Stat4Column(
  sqlite3 *db,                    /* Database handle */
  സ്ഥിരം ശൂന്യം *pRec,               /* Pointer to buffer containing record */
  പൂർണ്ണസംഖ്യ nRec,                       /* Size of buffer pRec in bytes */
  പൂർണ്ണസംഖ്യ iCol,                       /* Column to extract */
  sqlite3_മൂല്യം **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 iField;                     /* Next unread data byte */
  u32 szField = 0;                /* Size of the current data field */
  പൂർണ്ണസംഖ്യ i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  പ്രതിജ്ഞാ( iCol>0 );
  iHdr = getVarint32(a, nHdr);
  എങ്കിൽ( nHdr>(u32)nRec || iHdr>=nHdr ) തിരികെ SQLITE_CORRUPT_BKPT;
  iField = nHdr;
  വേണ്ടി(i=0; i<=iCol; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    എങ്കിൽ( iHdr>nHdr ) തിരികെ SQLITE_CORRUPT_BKPT;
    szField = sqlite3VdbeSerialTypeLen(t);
    iField += szField;
  }
  testcase( iField==nRec );
  testcase( iField==nRec+1 );
  എങ്കിൽ( iField>nRec ) തിരികെ SQLITE_CORRUPT_BKPT;
  എങ്കിൽ( pMem==0 ){
    pMem = *ppVal = sqlite3_മൂല്യം_New(db);
    എങ്കിൽ( pMem==0 ) തിരികെ SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[iField-szField], t, pMem);
  pMem->enc = ENC(db);
  തിരികെ SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
ശൂന്യം sqlite3Stat4ProbeFree(UnpackedRecord *pRec){
  എങ്കിൽ( pRec ){
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ nCol = pRec->p_താക്കോൽ_Info->nAllField;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    വേണ്ടി(i=0; i<nCol; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_താക്കോൽ_Info_Unref(pRec->p_താക്കോൽ_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#endif /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
ശൂന്യം sqlite3_മൂല്യം_Set_Str(
  sqlite3_മൂല്യം *v,     /* Value to be set */
  പൂർണ്ണസംഖ്യ n,                /* Length of string z */
  സ്ഥിരം ശൂന്യം *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  ശൂന്യം (*xDel)(ശൂന്യം*)   /* Destructor for the string */
){
  എങ്കിൽ( v ) sqlite3VdbeMemSetStr((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
ശൂന്യം sqlite3_മൂല്യം_Free(sqlite3_മൂല്യം *v){
  എങ്കിൽ( !v ) തിരികെ;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ മൂല്യം_Bytes(sqlite3_മൂല്യം *pVal, u8 enc){
  തിരികെ മൂല്യം_To_Text(pVal, enc)!=0 ? pVal->n : 0;
}
പൂർണ്ണസംഖ്യ sqlite3_മൂല്യം_Bytes(sqlite3_മൂല്യം *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  പ്രതിജ്ഞാ( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  എങ്കിൽ( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    തിരികെ p->n;
  }
  എങ്കിൽ( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    തിരികെ p->n;
  }
  എങ്കിൽ( (p->flags & MEM_Blob)!=0 ){
    എങ്കിൽ( p->flags & MEM_Zero ){
      തിരികെ p->n + p->u.nZero;
    }അല്ലെങ്കിൽ{
      തിരികെ p->n;
    }
  }
  എങ്കിൽ( p->flags & MEM_Null ) തിരികെ 0;
  തിരികെ മൂല്യം_Bytes(pVal, enc);
}
