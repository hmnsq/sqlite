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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

/* True if X is a power of two.  0 is considered a power of two here.
** In other words, return true if X has at most one bit set.
*/
#وصف_ڪريو ISPOWEROF2(X)  (((X)&((X)-1))==0)

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
/*
** Check invariants on a Mem object.
**
** This routine is intended for use inside of assert() statements, like
** this:    assert( sqlite3VdbeCheckMemInvariants(pMem) );
*/
صحيح sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  أكد( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  أكد( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  أكد( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  إذا( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    أكد( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    إذا( (p->flags & (MEM_رڪن|MEM_Subtype))==(MEM_رڪن|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      أكد( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      أكد( (p->flags & ~(MEM_Null|MEM_رڪن|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }وإلا{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }وإلا{
    /* The MEM_Cleared bit is only allowed on NULLs */
    أكد( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  أكد( p->szMalloc==0
       || (p->flags==MEM_Undefined 
           && p->szMalloc<=sqlite3_Db_Malloc_سائيز(p->db,p->zMalloc))
       || p->szMalloc==sqlite3_Db_Malloc_سائيز(p->db,p->zMalloc));

  /* If p holds a string or blob, the Mem.z must point to exactly
  ** one of the following:
  **
  **   (1) Memory in Mem.zMalloc and managed by the Mem object
  **   (2) Memory to be freed using Mem.xDel
  **   (3) An ephemeral string or blob
  **   (4) A static string or blob
  */
  إذا( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    أكد( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  أرجع 1;
}
#جيڪڏهن_پورو

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
ساكن فراغ vdbe_Mem_Render_انگ(صحيح sz, حرف *zBuf, Mem *p){
  StrAccum acc;
  أكد( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  أكد( sz>22 );
  إذا( p->flags & (MEM_Int|MEM_IntReal) ){
#إذا GCC_نسخو>=7000000 && GCC_نسخو<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    أكد( (sqlite3Config.bSmallMalloc!=0xee)*8==حجم(x) );
    يادگيري_نقل(&x, (حرف*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3_Int64_منزل_متن(x, zBuf);
#وإلا
    p->n = sqlite3_Int64_منزل_متن(p->u.i, zBuf);
#جيڪڏهن_پورو
    إذا( p->flags & MEM_IntReal ){
      يادگيري_نقل(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }وإلا{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    أكد( acc.z_متن==zBuf && acc.mxAlloc<=0 );
    zBuf[acc.nChar] = 0; /* Fast version of sqlite3StrAccumFinish(&acc) */
    p->n = acc.nChar;
  }
}

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
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
صحيح sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  حرف zBuf[100];
  حرف *z;
  صحيح i, j, incr;
  إذا( (p->flags & MEM_Str)==0 ) أرجع 1;
  إذا( p->db && p->db->mallocFailed ) أرجع 1;
  إذا( p->flags & MEM_رڪن ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    إذا( p->szMalloc>0 && p->z==p->zMalloc ){
      أكد( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      أكد( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    أكد( p->z[p->n]==0 );
    أكد( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    أكد( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  إذا( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) أرجع 1;
  إذا( p->db==0 ){
    أرجع 1;  /* db->nFpDigit required to validate p->z[] */
  }
  يادگيري_نقل(&tmp, p, حجم(tmp));
  vdbe_Mem_Render_انگ(حجم(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  إذا( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    إذا( p->enc==SQLITE_UTF16BE ) z++;
  }
  طالما( zBuf[j] ){
    إذا( zBuf[j++]!=z[i] ) أرجع 0;
    i += incr;
  }
  أرجع 1;
}
#جيڪڏهن_پورو /* SQLITE_DEBUG */

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
صحيح sqlite3VdbeChangeEncoding(Mem *pMem, صحيح desiredEnc){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UTF16
  صحيح rc;
#جيڪڏهن_پورو
  أكد( pMem!=0 );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  أكد( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  إذا( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    أرجع SQLITE_OK;
  }
  إذا( pMem->enc==desiredEnc ){
    أرجع SQLITE_OK;
  }
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#جيڪڏهن_وصف_ٿيل SQLITE_OMIT_UTF16
  أرجع SQLITE_ERROR;
#وإلا

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  أكد(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  أكد(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  أكد(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  أرجع rc;
#جيڪڏهن_پورو
}

/*
** Make sure pMem->z points to a writable allocation of at least n bytes.
**
** If the bPreserve argument is true, then copy of the content of
** pMem->z into the new allocation.  pMem must be either a string or
** blob if bPreserve is true.  If bPreserve is false, any prior content
** in pMem->z is discarded.
*/
SQLITE_NOINLINE صحيح sqlite3VdbeMemGrow(Mem *pMem, صحيح n, صحيح bPreserve){
  أكد( sqlite3VdbeCheckMemInvariants(pMem) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  أكد( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  أكد( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3_Db_Malloc_سائيز(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3_Db_Malloc_سائيز(pMem->db,pMem->zMalloc));
  إذا( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    إذا( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }وإلا{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      إذا( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }وإلا{
    إذا( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  إذا( pMem->zMalloc==0 ){
    sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    أرجع SQLITE_NOMEM_BKPT;
  }وإلا{
    pMem->szMalloc = sqlite3_Db_Malloc_سائيز(pMem->db, pMem->zMalloc);
  }

  إذا( bPreserve && pMem->z ){
    أكد( pMem->z!=pMem->zMalloc );
    يادگيري_نقل(pMem->zMalloc, pMem->z, pMem->n);
  }
  إذا( (pMem->flags&MEM_Dyn)!=0 ){
    أكد( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((فراغ *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  أرجع SQLITE_OK;
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
صحيح sqlite3VdbeMemClearAndResize(Mem *pMem, صحيح szNew){
  أكد( CORRUPT_DB || szNew>0 );
  أكد( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  إذا( pMem->szMalloc<szNew ){
    أرجع sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  أكد( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  أرجع SQLITE_OK;
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
صحيح sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  إذا( (pMem->flags & (MEM_Str|MEM_رڪن|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    أرجع 0;
  }
  إذا( pMem->enc!=SQLITE_UTF8 ) أرجع 0;
  أكد( pMem->z!=0 );
  إذا( pMem->flags & MEM_Dyn ){
    إذا( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_رڪن;
      أرجع 1;
    }
    إذا( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_رڪن;
      أرجع 1;
    }
  }وإلا إذا( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_رڪن;
    أرجع 1;
  }
  أرجع 0;
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
ساكن SQLITE_NOINLINE صحيح vdbe_Mem_جوڙيو_Terminator(Mem *pMem){
  إذا( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_رڪن;
  أرجع SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
صحيح sqlite3VdbeMemMakeWriteable(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  إذا( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    إذا( ExpandBlob(pMem) ) أرجع SQLITE_NOMEM;
    إذا( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      صحيح rc = vdbe_Mem_جوڙيو_Terminator(pMem);
      إذا( rc ) أرجع rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#جيڪڏهن_پورو

  أرجع SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_INCRBLOB
صحيح sqlite3VdbeMemExpandBlob(Mem *pMem){
  صحيح nByte;
  أكد( pMem!=0 );
  أكد( pMem->flags & MEM_Zero );
  أكد( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_قدر_nochange(pMem) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  إذا( nByte<=0 ){
    إذا( (pMem->flags & MEM_Blob)==0 ) أرجع SQLITE_OK;
    nByte = 1;
  }
  إذا( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  أكد( pMem->z!=0 );
  أكد( sqlite3_Db_Malloc_سائيز(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_رڪن);
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو

/*
** Make sure the given Mem is \u0000 terminated.
*/
صحيح sqlite3VdbeMemNulTerminate(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_رڪن|MEM_Str))==(MEM_رڪن|MEM_Str) );
  testcase( (pMem->flags & (MEM_رڪن|MEM_Str))==0 );
  إذا( (pMem->flags & (MEM_رڪن|MEM_Str))!=MEM_Str ){
    أرجع SQLITE_OK;   /* Nothing to do */
  }وإلا{
    أرجع vdbe_Mem_جوڙيو_Terminator(pMem);
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
صحيح sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  ثابت صحيح nByte = 32;

  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !(pMem->flags&MEM_Zero) );
  أكد( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  أكد( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );


  إذا( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    أرجع SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_انگ(nByte, pMem->z, pMem);
  أكد( pMem->z!=0 );
  أكد( pMem->n==(صحيح)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_رڪن;
  إذا( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  أرجع SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
صحيح sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  أكد( pFunc!=0 );
  أكد( pMem!=0 );
  أكد( pMem->db!=0 );
  أكد( pFunc->xFinalize!=0 );
  أكد( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  أكد( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, حجم(ctx));
  memset(&t, 0, حجم(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.p_ٻاهريون = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  أكد( (pMem->flags & MEM_Dyn)==0 );
  إذا( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  يادگيري_نقل(pMem, &t, حجم(t));
  أرجع ctx.isError;
}

/*
** Memory cell pAccum contains the context of an aggregate function.
** This routine calls the xValue method for that function and stores
** the results in memory cell pMem.
**
** SQLITE_ERROR is returned if xValue() reports an error. SQLITE_OK 
** otherwise.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
صحيح sqlite3_Vdbe_Mem_Agg_قدر(Mem *pAccum, Mem *p_ٻاهريون, FuncDef *pFunc){
  sqlite3_context ctx;
  أكد( pFunc!=0 );
  أكد( pFunc->x_قدر!=0 );
  أكد( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  أكد( pAccum->db!=0 );
  أكد( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, حجم(ctx));
  sqlite3_Vdbe_Mem_سيٽ_Null(p_ٻاهريون);
  ctx.p_ٻاهريون = p_ٻاهريون;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_قدر(&ctx);
  أرجع ctx.isError;
}
#جيڪڏهن_پورو /* SQLITE_OMIT_WINDOWFUNC */

/*
** If the memory cell contains a value that must be freed by
** invoking the external callback in Mem.xDel, then this routine
** will free that value.  It also sets Mem.flags to MEM_Null.
**
** This is a helper routine for sqlite3VdbeMemSetNull() and
** for sqlite3VdbeMemRelease().  Use those other routines as the
** entry point for releasing Mem resources.
*/
ساكن SQLITE_NOINLINE فراغ vdbe_Mem_Clear_Extern_And_سيٽ_Null(Mem *p){
  أكد( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  أكد( VdbeMemDynamic(p) );
  إذا( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    أكد( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  إذا( p->flags&MEM_Dyn ){
    أكد( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((فراغ *)p->z);
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
ساكن SQLITE_NOINLINE فراغ vdbeMemClear(Mem *p){
  إذا( VdbeMemDynamic(p) ){
    vdbe_Mem_Clear_Extern_And_سيٽ_Null(p);
  }
  إذا( p->szMalloc ){
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
فراغ sqlite3VdbeMemRelease(Mem *p){
  أكد( sqlite3VdbeCheckMemInvariants(p) );
  إذا( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
فراغ sqlite3VdbeMemReleaseMalloc(Mem *p){
  أكد( !VdbeMemDynamic(p) );
  إذا( p->szMalloc ) vdbeMemClear(p);
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
ساكن SQLITE_NOINLINE i64 mem_Int_قدر(ثابت Mem *pMem){
  i64 قدر = 0;
  sqlite3Atoi64(pMem->z, &قدر, pMem->n, pMem->enc);
  أرجع قدر;
}
i64 sqlite3_Vdbe_Int_قدر(ثابت Mem *pMem){
  صحيح flags;
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  إذا( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    أرجع pMem->u.i;
  }وإلا إذا( flags & MEM_Real ){
    أرجع sqlite3_Real_منزل_I64(pMem->u.r);
  }وإلا إذا( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    أرجع mem_Int_قدر(pMem);
  }وإلا{
    أرجع 0;
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
ساكن SQLITE_NOINLINE صحيح sqlite3_Mem_Real_قدر_RCSlow_رستو(
  Mem *pMem,
  مزدوج *p_قدر
){
  صحيح rc = SQLITE_OK;
  *p_قدر = 0.0;
  إذا( pMem->enc==SQLITE_UTF8 ){
    حرف *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    إذا( zCopy ){
      rc = sqlite3AtoF(zCopy, p_قدر);
      sqlite3DbFree(pMem->db, zCopy);
    }
    أرجع rc;
  }وإلا{
    صحيح n, i, j;
    حرف *zCopy;
    ثابت حرف *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    إذا( zCopy ){
      z = pMem->z;
      إذا( pMem->enc==SQLITE_UTF16LE ){
        لكل(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          إذا( z[i+1]!=0 ) اكسر;
        }
      }وإلا{
        لكل(i=j=0; i<n-1; i+=2, j++){
          إذا( z[i]!=0 ) اكسر;
          zCopy[j] = z[i+1];
        }
      }     
      أكد( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_قدر);
      إذا( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    أرجع rc;
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
صحيح sqlite3_Mem_Real_قدر_RC(Mem *pMem, مزدوج *p_قدر){
  testcase( pMem->db==0 );
  أكد( pMem->flags & (MEM_Str|MEM_Blob) );
  إذا( pMem->z==0 ){
    *p_قدر = 0.0;
    أرجع 0;
  }وإلا إذا( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_رڪن)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    أرجع sqlite3AtoF(pMem->z, p_قدر);
  }وإلا إذا( pMem->n==0 ){
    *p_قدر = 0.0;
    أرجع 0;
  }وإلا{
    أرجع sqlite3_Mem_Real_قدر_RCSlow_رستو(pMem, p_قدر);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
ساكن SQLITE_NOINLINE مزدوج sqlite3_Mem_Real_قدر_No_RC(Mem *pMem){
  مزدوج r;
  (فراغ)sqlite3_Mem_Real_قدر_RC(pMem, &r);
  أرجع r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
مزدوج sqlite3_Vdbe_Real_قدر(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );
  إذا( pMem->flags & MEM_Real ){
    أرجع pMem->u.r;
  }وإلا إذا( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    أرجع (مزدوج)pMem->u.i;
  }وإلا إذا( pMem->flags & (MEM_Str|MEM_Blob) ){
    أرجع sqlite3_Mem_Real_قدر_No_RC(pMem);
  }وإلا{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    أرجع (مزدوج)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
صحيح sqlite3_Vdbe_Boolean_قدر(Mem *pMem, صحيح ifNull){
  testcase( pMem->flags & MEM_IntReal );
  إذا( pMem->flags & (MEM_Int|MEM_IntReal) ) أرجع pMem->u.i!=0;
  إذا( pMem->flags & MEM_Null ) أرجع ifNull;
  أرجع sqlite3_Vdbe_Real_قدر(pMem)!=0.0;
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
فراغ sqlite3VdbeIntegerAffinity(Mem *pMem, صحيح b_From_متن){
  أكد( pMem!=0 );
  أكد( pMem->flags & (MEM_Real|MEM_IntReal) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );

  إذا( pMem->flags & MEM_IntReal ){
    Mem_سيٽ_Type_Flag(pMem, MEM_Int);
  }وإلا{
    i64 ix = sqlite3_Real_منزل_I64(pMem->u.r);

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
    إذا( pMem->u.r==ix && ix<سڀ_کان_وڏو_INT64
     && (!b_From_متن || ix>سڀ_کان_ننڍو_INT64)
    ){
      pMem->u.i = ix;
      Mem_سيٽ_Type_Flag(pMem, MEM_Int);
    }
  }
}

/*
** Convert pMem to type integer.  Invalidate any prior representations.
*/
صحيح sqlite3VdbeMemIntegerify(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_قدر(pMem);
  Mem_سيٽ_Type_Flag(pMem, MEM_Int);
  أرجع SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
صحيح sqlite3VdbeMemRealify(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_قدر(pMem);
  Mem_سيٽ_Type_Flag(pMem, MEM_Real);
  أرجع SQLITE_OK;
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
صحيح sqlite3RealSameAsInt(مزدوج r1, sqlite3_int64 i){
  مزدوج r2 = (مزدوج)i;
  أرجع r1==0.0
      || (memcmp(&r1, &r2, حجم(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3_Real_منزل_I64(مزدوج r){
  إذا( r<-9223372036854774784.0 ) أرجع سڀ_کان_ننڍو_INT64;
  إذا( r>+9223372036854774784.0 ) أرجع سڀ_کان_وڏو_INT64;
  أرجع (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
صحيح sqlite3VdbeMemNumerify(Mem *pMem){
  أكد( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  إذا( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    صحيح rc;
    sqlite3_int64 ix;
    أكد( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_قدر_RC(pMem, &pMem->u.r);
    إذا( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3_Real_منزل_I64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      Mem_سيٽ_Type_Flag(pMem, MEM_Int);
    }وإلا{
      Mem_سيٽ_Type_Flag(pMem, MEM_Real);
    }
  }
  أكد( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  أرجع SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
صحيح sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  إذا( pMem->flags & MEM_Null ) أرجع SQLITE_OK;
  بدل( aff ){
    حالة SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      إذا( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_قدر_Apply_Affinity(pMem, SQLITE_AFF_متن, encoding);
        أكد( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        إذا( pMem->flags & MEM_Str ) Mem_سيٽ_Type_Flag(pMem, MEM_Blob);
      }وإلا{
        pMem->flags &= ~(MEM_Type_نقاب&~MEM_Blob);
      }
      اكسر;
    }
    حالة SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      اكسر;
    }
    حالة SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      اكسر;
    }
    حالة SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      اكسر;
    }
    افتراضي: {
      صحيح rc;
      أكد( aff==SQLITE_AFF_متن );
      أكد( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_قدر_Apply_Affinity(pMem, SQLITE_AFF_متن, encoding);
      أكد( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      إذا( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      إذا( rc ) أرجع rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  أرجع SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
فراغ sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  أكد( (flags & ~MEM_Type_نقاب)==0 );
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
فراغ sqlite3_Vdbe_Mem_سيٽ_Null(Mem *pMem){
  إذا( VdbeMemDynamic(pMem) ){
    vdbe_Mem_Clear_Extern_And_سيٽ_Null(pMem);
  }وإلا{
    pMem->flags = MEM_Null;
  }
}
فراغ sqlite3_قدر_سيٽ_Null(sqlite3_قدر *p){
  sqlite3_Vdbe_Mem_سيٽ_Null((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_INCRBLOB
فراغ sqlite3_Vdbe_Mem_سيٽ_Zero_Blob(Mem *pMem, صحيح n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  إذا( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#وإلا
صحيح sqlite3_Vdbe_Mem_سيٽ_Zero_Blob(Mem *pMem, صحيح n){
  صحيح nByte = n>0?n:1;
  إذا( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  أكد( pMem->z!=0 );
  أكد( sqlite3_Db_Malloc_سائيز(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
ساكن SQLITE_NOINLINE فراغ vdbe_Release_And_سيٽ_Int64(Mem *pMem, i64 val){
  sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
فراغ sqlite3_Vdbe_Mem_سيٽ_Int64(Mem *pMem, i64 val){
  إذا( VdbeMemDynamic(pMem) ){
    vdbe_Release_And_سيٽ_Int64(pMem, val);
  }وإلا{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
فراغ sqlite3_Mem_سيٽ_Array_Int64(sqlite3_قدر *aMem, صحيح iIdx, i64 val){
  sqlite3_Vdbe_Mem_سيٽ_Int64(&aMem[iIdx], val);
}

/* A no-op destructor */
فراغ sqlite3NoopDestructor(فراغ *p){ اڻ_استعمال_ٿيل_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
فراغ sqlite3_Vdbe_Mem_سيٽ_اشارو(
  Mem *pMem,
  فراغ *pPtr,
  ثابت حرف *zPType,
  فراغ (*xDestructor)(فراغ*)
){
  أكد( pMem->flags==MEM_Null );
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = MEM_Null|MEM_Dyn|MEM_Subtype|MEM_رڪن;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_FLOATING_نقطو
/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type REAL.
*/
فراغ sqlite3_Vdbe_Mem_سيٽ_Double(Mem *pMem, مزدوج val){
  sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
  إذا( !sqlite3IsNaN(val) ){
    pMem->u.r = val;
    pMem->flags = MEM_Real;
  }
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
/*
** Return true if the Mem holds a RowSet object.  This routine is intended
** for use inside of assert() statements.
*/
صحيح sqlite3_Vdbe_Mem_Is_صف_سيٽ(ثابت Mem *pMem){
  أرجع (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
         && pMem->xDel==sqlite3_صف_سيٽ_Delete;
}
#جيڪڏهن_پورو

/*
** Delete any previous value and set the value of pMem to be an
** empty boolean index.
**
** Return SQLITE_OK on success and SQLITE_NOMEM if a memory allocation
** error occurs.
*/
صحيح sqlite3_Vdbe_Mem_سيٽ_صف_سيٽ(Mem *pMem){
  sqlite3 *db = pMem->db;
  صف_سيٽ *p;
  أكد( db!=0 );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3_صف_سيٽ_Init(db);
  إذا( p==0 ) أرجع SQLITE_NOMEM;
  pMem->z = (حرف*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3_صف_سيٽ_Delete;
  أرجع SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
صحيح sqlite3VdbeMemTooBig(Mem *p){
  أكد( p->db!=0 );
  إذا( p->flags & (MEM_Str|MEM_Blob) ){
    صحيح n = p->n;
    إذا( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    أرجع n>p->db->a_حد_بندي[SQLITE_حد_بندي_ڊيگهائي];
  }
  أرجع 0; 
}

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
/*
** This routine prepares a memory cell for modification by breaking
** its link to a shallow copy and by marking any current shallow
** copies of this cell as invalid.
**
** This is used for testing and debugging only - to help ensure that shallow
** copies (created by OP_SCopy) are not misused.
*/
فراغ sqlite3_Vdbe_Mem_About_منزل_Change(Vdbe *pVdbe, Mem *pMem){
  صحيح i;
  Mem *pX;
  إذا( pMem->bScopy ){
    لكل(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      إذا( pX->pScopyFrom==pMem ){
        u16 mFlags;
        إذا( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (صحيح)(pX - pVdbe->aMem), (صحيح)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        أكد( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
#جيڪڏهن_پورو /* SQLITE_DEBUG */

/*
** Make an shallow copy of pFrom into pTo.  Prior contents of
** pTo are freed.  The pFrom->z field is not duplicated.  If
** pFrom->z is used, then pTo->z points to the same thing as pFrom->z
** and flags gets srcType (either MEM_Ephem or MEM_Static).
*/
ساكن SQLITE_NOINLINE فراغ vdbeClrCopy(Mem *p_منزل, ثابت Mem *pFrom, صحيح eType){
  vdbe_Mem_Clear_Extern_And_سيٽ_Null(p_منزل);
  أكد( !VdbeMemDynamic(p_منزل) );
  sqlite3VdbeMemShallowCopy(p_منزل, pFrom, eType);
}
فراغ sqlite3VdbeMemShallowCopy(Mem *p_منزل, ثابت Mem *pFrom, صحيح srcType){
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pFrom) );
  أكد( p_منزل->db==pFrom->db );
  إذا( VdbeMemDynamic(p_منزل) ){ vdbeClrCopy(p_منزل,pFrom,srcType); أرجع; }
  يادگيري_نقل(p_منزل, pFrom, MEMCELLSIZE);
  إذا( (pFrom->flags&MEM_Static)==0 ){
    p_منزل->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    أكد( srcType==MEM_Ephem || srcType==MEM_Static );
    p_منزل->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
صحيح sqlite3VdbeMemCopy(Mem *p_منزل, ثابت Mem *pFrom){
  صحيح rc = SQLITE_OK;

  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pFrom) );
  إذا( VdbeMemDynamic(p_منزل) ) vdbe_Mem_Clear_Extern_And_سيٽ_Null(p_منزل);
  يادگيري_نقل(p_منزل, pFrom, MEMCELLSIZE);
  p_منزل->flags &= ~MEM_Dyn;
  إذا( p_منزل->flags&(MEM_Str|MEM_Blob) ){
    إذا( 0==(pFrom->flags&MEM_Static) ){
      p_منزل->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(p_منزل);
    }
  }

  أرجع rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
فراغ sqlite3VdbeMemMove(Mem *p_منزل, Mem *pFrom){
  أكد( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  أكد( p_منزل->db==0 || sqlite3_mutex_held(p_منزل->db->mutex) );
  أكد( pFrom->db==0 || p_منزل->db==0 || pFrom->db==p_منزل->db );

  sqlite3VdbeMemRelease(p_منزل);
  يادگيري_نقل(p_منزل, pFrom, حجم(Mem));
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
صحيح sqlite3_Vdbe_Mem_سيٽ_Str(
  Mem *pMem,          /* Memory cell to set to string value */
  ثابت حرف *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  فراغ (*xDel)(فراغ*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  صحيح i_حد_بندي;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  أكد( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  إذا( !z ){
    sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
    أرجع SQLITE_OK;
  }

  إذا( pMem->db ){
    i_حد_بندي = pMem->db->a_حد_بندي[SQLITE_حد_بندي_ڊيگهائي];
  }وإلا{
    i_حد_بندي = SQLITE_MAX_ڊيگهائي;
  }
  إذا( nByte<0 ){
    أكد( enc!=0 );
    إذا( enc==SQLITE_UTF8 ){
      nByte = لڙي_ڊيگهه(z);
    }وإلا{
      لكل(nByte=0; nByte<=i_حد_بندي && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_رڪن;
  }وإلا إذا( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }وإلا{
    flags = MEM_Str;
  }
  إذا( nByte>i_حد_بندي ){
    إذا( xDel && xDel!=SQLITE_TRANSIENT ){
      إذا( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (فراغ*)z);
      }وإلا{
        xDel((فراغ*)z);
      }
    }
    sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
    أرجع sqlite3_Error_منزل_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  إذا( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    إذا( flags&MEM_رڪن ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    إذا( sqlite3VdbeMemClearAndResize(pMem, (صحيح)MAX(nAlloc,32)) ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    أكد( pMem->z!=0 );
    يادگيري_نقل(pMem->z, z, nAlloc);
  }وإلا{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (حرف *)z;
    إذا( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_سائيز(pMem->db, pMem->zMalloc);
    }وإلا{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (صحيح)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  إذا( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    أرجع SQLITE_NOMEM_BKPT;
  }
#جيڪڏهن_پورو


  أرجع SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
صحيح sqlite3_Vdbe_Mem_سيٽ_متن(
  Mem *pMem,          /* Memory cell to set to string value */
  ثابت حرف *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  فراغ (*xDel)(فراغ*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  أكد( pMem!=0 );
  أكد( pMem->db!=0 );
  أكد( sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  إذا( !z ){
    sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
    أرجع SQLITE_OK;
  }

  إذا( nByte<0 ){
    nByte = لڙي_ڊيگهه(z);
    flags = MEM_Str|MEM_رڪن;
  }وإلا{
    flags = MEM_Str;
  }
  إذا( nByte>(i64)pMem->db->a_حد_بندي[SQLITE_حد_بندي_ڊيگهائي] ){
    إذا( xDel && xDel!=SQLITE_TRANSIENT ){
      إذا( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (فراغ*)z);
      }وإلا{
        xDel((فراغ*)z);
      }
    }
    sqlite3_Vdbe_Mem_سيٽ_Null(pMem);
    أرجع sqlite3_Error_منزل_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  إذا( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    إذا( sqlite3VdbeMemClearAndResize(pMem, (صحيح)MAX(nAlloc,32)) ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    أكد( pMem->z!=0 );
    يادگيري_نقل(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }وإلا{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (حرف *)z;
    إذا( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_سائيز(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }وإلا إذا( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }وإلا{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (صحيح)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  أرجع SQLITE_OK;
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
صحيح sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 وٿي,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  صحيح rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ورهاست_سائيز-1 );
  testcase( amt==SQLITE_MAX_ورهاست_سائيز );
  إذا( amt>=SQLITE_MAX_ورهاست_سائيز ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  إذا( (u64)amt + (u64)وٿي > (u64)sqlite3_Btree_Max_رڪارڊ_سائيز(pCur) ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, وٿي, amt, pMem->z);
    إذا( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (صحيح)amt;
    }وإلا{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  أرجع rc;
}
صحيح sqlite3_Vdbe_Mem_From_Btree_Zero_وٿي(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 موجود = 0;  /* Number of bytes available on the local btree page */
  صحيح rc = SQLITE_OK; /* Return code */

  أكد( sqlite3BtreeCursorIsValid(pCur) );
  أكد( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pMem) );
  pMem->z = (حرف *)sqlite3BtreePayloadFetch(pCur, &موجود);
  أكد( pMem->z!=0 );

  إذا( amt<=موجود ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (صحيح)amt;
  }وإلا{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  أرجع rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
ساكن SQLITE_NOINLINE ثابت فراغ *قدر_منزل_متن(sqlite3_قدر* pVal, u8 enc){
  أكد( pVal!=0 );
  أكد( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  أكد( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pVal) );
  أكد( (pVal->flags & (MEM_Null))==0 );
  إذا( pVal->flags & (MEM_Blob|MEM_Str) ){
    إذا( ExpandBlob(pVal) ) أرجع 0;
    pVal->flags |= MEM_Str;
    إذا( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    إذا( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_منزل_INT(pVal->z)) ){
      أكد( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      إذا( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        أرجع 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }وإلا{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    أكد( 0==(1&SQLITE_PTR_منزل_INT(pVal->z)) );
  }
  أكد(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  إذا( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    أكد( sqlite3VdbeMemValidStrRep(pVal) );
    أرجع pVal->z;
  }وإلا{
    أرجع 0;
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
ثابت فراغ *sqlite3_قدر_متن_2(sqlite3_قدر* pVal, u8 enc){
  إذا( !pVal ) أرجع 0;
  أكد( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  أكد( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سيٽ(pVal) );
  إذا( (pVal->flags&(MEM_Str|MEM_رڪن))==(MEM_Str|MEM_رڪن) && pVal->enc==enc ){
    أكد( sqlite3VdbeMemValidStrRep(pVal) );
    أرجع pVal->z;
  }
  إذا( pVal->flags&MEM_Null ){
    أرجع 0;
  }
  أرجع قدر_منزل_متن(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
صحيح sqlite3_قدر_Is_جو_Class(ثابت sqlite3_قدر *pVal, فراغ(*xFree)(فراغ*)){
  إذا( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    أرجع 1;
  }وإلا{
    أرجع 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_قدر *sqlite3_قدر_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, حجم(*p));
  إذا( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  أرجع p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
بنية قدر_New_Stat4_Ctx {
  Parse *pParse;
  اشاريو *pIdx;
  Unpacked_رڪارڊ **ppRec;
  صحيح iVal;
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
ساكن sqlite3_قدر *قدر_New(sqlite3 *db, بنية قدر_New_Stat4_Ctx *p){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
  إذا( p ){
    Unpacked_رڪارڊ *pRec = p->ppRec[0];

    إذا( pRec==0 ){
      اشاريو *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      صحيح i;                      /* Counter variable */
      صحيح n_کانو = pIdx->n_ڪالم;   /* Number of index columns including rowid */
  
      nByte = حجم(Mem) * n_کانو + ROUND8(حجم(Unpacked_رڪارڊ));
      pRec = (Unpacked_رڪارڊ*)sqlite3DbMallocZero(db, nByte);
      إذا( pRec ){
        pRec->p_ڪنجي_Info = sqlite3_ڪنجي_Info_جو_اشاريو(p->pParse, pIdx);
        إذا( pRec->p_ڪنجي_Info ){
          أكد( pRec->p_ڪنجي_Info->n_All_ميدان==n_کانو );
          أكد( pRec->p_ڪنجي_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(حجم(Unpacked_رڪارڊ)));
          لكل(i=0; i<n_کانو; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }وإلا{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      إذا( pRec==0 ) أرجع 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->n_ميدان = p->iVal+1;
    sqlite3_Vdbe_Mem_سيٽ_Null(&pRec->aMem[p->iVal]);
    أرجع &pRec->aMem[p->iVal];
  }
#وإلا
  اڻ_استعمال_ٿيل_PARAMETER(p);
#جيڪڏهن_پورو /* defined(SQLITE_ENABLE_STAT4) */
  أرجع sqlite3_قدر_New(db);
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
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
ساكن صحيح قدر_From_Function(
  sqlite3 *db,                    /* The database connection */
  ثابت اظهار *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_قدر **ppVal,          /* Write the new value here */
  بنية قدر_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_قدر **apVal = 0;      /* Function arguments */
  صحيح nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_قدر *pVal = 0;        /* New value */
  صحيح rc = SQLITE_OK;             /* Return code */
  اظهار_List *pList = 0;            /* Function arguments */
  صحيح i;                          /* Iterator variable */

  أكد( pCtx!=0 );
  أكد( (p->flags & EP_TokenOnly)==0 );
  أكد( اظهار_Use_XList(p) );
  pList = p->x.pList;
  إذا( pList ) nVal = pList->n_اظهار;
  أكد( !اظهار_Has_Property(p, EP_Int_قدر) );
  pFunc = sqlite3_ڳوليو_Function(db, p->u.zToken, nVal, enc, 0);
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  إذا( pFunc==0 ) أرجع SQLITE_OK;
#جيڪڏهن_پورو
  أكد( pFunc );
  إذا( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    أرجع SQLITE_OK;
  }

  إذا( pList ){
    apVal = (sqlite3_قدر**)sqlite3DbMallocZero(db, حجم(apVal[0]) * nVal);
    إذا( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      اذهب قدر_from_function_ٻاهريون;
    }
    لكل(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_قدر_From_اظهار(pCtx->pParse, pList->a[i].p_اظهار, aff,
                                     &apVal[i]);
      إذا( apVal[i]==0 || rc!=SQLITE_OK ) اذهب قدر_from_function_ٻاهريون;
    }
  }

  pVal = قدر_New(db, pCtx);
  إذا( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    اذهب قدر_from_function_ٻاهريون;
  }

  memset(&ctx, 0, حجم(ctx));
  ctx.p_ٻاهريون = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  إذا( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_نياپو(pCtx->pParse, "%s", sqlite3_قدر_متن(pVal));
  }وإلا{
    sqlite3_قدر_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    أكد( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    إذا( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 قدر_from_function_ٻاهريون:
  إذا( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  إذا( apVal ){
    لكل(i=0; i<nVal; i++){
      sqlite3_قدر_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  أرجع rc;
}
#وإلا
# define قدر_From_Function(a,b,c,d,e,f) SQLITE_OK
#جيڪڏهن_پورو /* defined(SQLITE_ENABLE_STAT4) */

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
ساكن صحيح قدر_From_اظهار(
  sqlite3 *db,                    /* The database connection */
  ثابت اظهار *p_اظهار,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_قدر **ppVal,          /* Write the new value here */
  بنية قدر_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  صحيح op;
  حرف *zVal = 0;
  sqlite3_قدر *pVal = 0;
  صحيح negInt = 1;
  ثابت حرف *zNeg = "";
  صحيح rc = SQLITE_OK;

  أكد( p_اظهار!=0 );
  طالما( (op = p_اظهار->op)==TK_UPLUS || op==TK_وسعت ) p_اظهار = p_اظهار->p_کاٻو;
  إذا( op==TK_رجسٽر ) op = p_اظهار->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  أكد( (p_اظهار->flags & EP_TokenOnly)==0 || pCtx==0 );

  إذا( op==TK_CAST ){
    u8 aff;
    أكد( !اظهار_Has_Property(p_اظهار, EP_Int_قدر) );
    aff = sqlite3AffinityType(p_اظهار->u.zToken,0);
    rc = قدر_From_اظهار(db, p_اظهار->p_کاٻو, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    إذا( *ppVal ){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#وإلا
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      أكد( (ppVal[0][0].flags & MEM_Zero)==0 );
#جيڪڏهن_پورو
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_قدر_Apply_Affinity(*ppVal, affinity, enc);
    }
    أرجع rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  إذا( op==TK_UMINUS ){
    اظهار *p_کاٻو = p_اظهار->p_کاٻو;
    إذا( (p_کاٻو->op==TK_INTEGER || p_کاٻو->op==TK_FLOAT) ){
      إذا( اظهار_Has_Property(p_کاٻو, EP_Int_قدر)
       || p_کاٻو->u.zToken[0]!='0' || (p_کاٻو->u.zToken[1] & ~0x20)!='X'
      ){
        p_اظهار = p_کاٻو;
        op = p_اظهار->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  إذا( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = قدر_New(db, pCtx);
    إذا( pVal==0 ) اذهب no_mem;
    إذا( اظهار_Has_Property(p_اظهار, EP_Int_قدر) ){
      sqlite3_Vdbe_Mem_سيٽ_Int64(pVal, (i64)p_اظهار->u.i_قدر*negInt);
    }وإلا{
      i64 iVal;
      إذا( op==TK_INTEGER && 0==sqlite3_Dec_Or_Hex_منزل_I64(p_اظهار->u.zToken, &iVal) ){
        sqlite3_Vdbe_Mem_سيٽ_Int64(pVal, iVal*negInt);
      }وإلا{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_اظهار->u.zToken);
        إذا( zVal==0 ) اذهب no_mem;
        sqlite3_قدر_سيٽ_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        إذا( op==TK_FLOAT ){
          أكد( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_رڪن) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    إذا( affinity==SQLITE_AFF_BLOB ){
      إذا( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_قدر_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }وإلا{
      sqlite3_قدر_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    أكد( (pVal->flags & MEM_IntReal)==0 );
    إذا( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    إذا( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }وإلا إذا( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    إذا( SQLITE_OK==قدر_From_اظهار(db,p_اظهار->p_کاٻو,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      إذا( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }وإلا إذا( pVal->u.i==سڀ_کان_ننڍو_INT64 ){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_FLOATING_نقطو
        pVal->u.r = -(مزدوج)سڀ_کان_ننڍو_INT64;
#وإلا
        pVal->u.r = سڀ_کان_وڏو_INT64;
#جيڪڏهن_پورو
        Mem_سيٽ_Type_Flag(pVal, MEM_Real);
      }وإلا{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_قدر_Apply_Affinity(pVal, affinity, enc);
    }
  }وإلا إذا( op==TK_NULL ){
    pVal = قدر_New(db, pCtx);
    إذا( pVal==0 ) اذهب no_mem;
    sqlite3_Vdbe_Mem_سيٽ_Null(pVal);
  }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_BLOB_LITERAL
  وإلا إذا( op==TK_BLOB ){
    صحيح nVal;
    أكد( !اظهار_Has_Property(p_اظهار, EP_Int_قدر) );
    أكد( p_اظهار->u.zToken[0]=='x' || p_اظهار->u.zToken[0]=='X' );
    أكد( p_اظهار->u.zToken[1]=='\'' );
    pVal = قدر_New(db, pCtx);
    إذا( !pVal ) اذهب no_mem;
    zVal = &p_اظهار->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    أكد( zVal[nVal]=='\'' );
    sqlite3_Vdbe_Mem_سيٽ_Str(pVal, sqlite3_Hex_منزل_Blob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
  وإلا إذا( op==TK_FUNCTION && pCtx!=0 ){
    rc = قدر_From_Function(db, p_اظهار, enc, affinity, &pVal, pCtx);
  }
#جيڪڏهن_پورو
  وإلا إذا( op==TK_TRUEFALSE ){
    أكد( !اظهار_Has_Property(p_اظهار, EP_Int_قدر) );
    pVal = قدر_New(db, pCtx);
    إذا( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_اظهار->u.zToken[4]==0;
      sqlite3_قدر_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  أرجع rc;

no_mem:
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
  إذا( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#جيڪڏهن_پورو
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  أكد( *ppVal==0 );
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
  إذا( pCtx==0 ) sqlite3_قدر_Free(pVal);
#وإلا
  أكد( pCtx==0 ); sqlite3_قدر_Free(pVal);
#جيڪڏهن_پورو
  أرجع SQLITE_NOMEM_BKPT;
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
صحيح sqlite3_قدر_From_اظهار(
  sqlite3 *db,              /* The database connection */
  ثابت اظهار *p_اظهار,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_قدر **ppVal     /* Write the new value here */
){
  صحيح rc = SQLITE_OK;
  sqlite3_قدر *pVal = 0;
  إذا( p_اظهار ){
    rc = قدر_From_اظهار(db, p_اظهار, enc, affinity, &pVal, 0);
    إذا( rc==SQLITE_OK && pVal 
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
  أرجع rc;
}

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
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
ساكن صحيح stat4_قدر_From_اظهار(
  Parse *pParse,                  /* Parse context */
  اظهار *p_اظهار,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  بنية قدر_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_قدر **ppVal           /* OUT: New value object (or NULL) */
){
  صحيح rc = SQLITE_OK;
  sqlite3_قدر *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_اظهار = sqlite3_اظهار_Skip_Collate(p_اظهار);

  أكد( p_اظهار==0 || p_اظهار->op!=TK_رجسٽر || p_اظهار->op2!=TK_VARIABLE );
  إذا( !p_اظهار ){
    pVal = قدر_New(db, pAlloc);
    إذا( pVal ){
      sqlite3_Vdbe_Mem_سيٽ_Null((Mem*)pVal);
    }
  }وإلا إذا( p_اظهار->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    صحيح iBindVar = p_اظهار->i_ڪالم;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    إذا( (v = pParse->pReprepare)!=0 ){
      pVal = قدر_New(db, pAlloc);
      إذا( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_قدر_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }وإلا{
    rc = قدر_From_اظهار(db, p_اظهار, ENC(db), affinity, &pVal, pAlloc);
  }

  أكد( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  أرجع rc;
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
صحيح sqlite3_Stat4_Probe_سيٽ_قدر(
  Parse *pParse,                  /* Parse context */
  اشاريو *pIdx,                    /* Index being probed */
  Unpacked_رڪارڊ **ppRec,         /* IN/OUT: Probe record */
  اظهار *p_اظهار,                    /* The expression to extract a value from */
  صحيح nElem,                      /* Maximum number of values to append */
  صحيح iVal,                       /* Array element to populate */
  صحيح *pn_ڪڍو                  /* OUT: Values appended to the record */
){
  صحيح rc = SQLITE_OK;
  صحيح n_ڪڍو = 0;

  إذا( p_اظهار==0 || p_اظهار->op!=TK_SELECT ){
    صحيح i;
    بنية قدر_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    لكل(i=0; i<nElem; i++){
      sqlite3_قدر *pVal = 0;
      اظهار *pElem = (p_اظهار ? sqlite3_ويڪٽر_ميدان_Subexpr(p_اظهار, i) : 0);
      u8 aff = sqlite3_اشاريو_ڪالم_Affinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_قدر_From_اظهار(pParse, pElem, aff, &alloc, &pVal);
      إذا( !pVal ) اكسر;
      n_ڪڍو++;
    }
  }

  *pn_ڪڍو = n_ڪڍو;
  أرجع rc;
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
صحيح sqlite3_Stat4_قدر_From_اظهار(
  Parse *pParse,                  /* Parse context */
  اظهار *p_اظهار,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_قدر **ppVal           /* OUT: New value object (or NULL) */
){
  أرجع stat4_قدر_From_اظهار(pParse, p_اظهار, affinity, 0, ppVal);
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
صحيح sqlite3_Stat4_ڪالم(
  sqlite3 *db,                    /* Database handle */
  ثابت فراغ *pRec,               /* Pointer to buffer containing record */
  صحيح nRec,                       /* Size of buffer pRec in bytes */
  صحيح i_کانو,                       /* Column to extract */
  sqlite3_قدر **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 i_ميدان;                     /* Next unread data byte */
  u32 sz_ميدان = 0;                /* Size of the current data field */
  صحيح i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  أكد( i_کانو>0 );
  iHdr = getVarint32(a, nHdr);
  إذا( nHdr>(u32)nRec || iHdr>=nHdr ) أرجع SQLITE_CORRUPT_BKPT;
  i_ميدان = nHdr;
  لكل(i=0; i<=i_کانو; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    إذا( iHdr>nHdr ) أرجع SQLITE_CORRUPT_BKPT;
    sz_ميدان = sqlite3VdbeSerialTypeLen(t);
    i_ميدان += sz_ميدان;
  }
  testcase( i_ميدان==nRec );
  testcase( i_ميدان==nRec+1 );
  إذا( i_ميدان>nRec ) أرجع SQLITE_CORRUPT_BKPT;
  إذا( pMem==0 ){
    pMem = *ppVal = sqlite3_قدر_New(db);
    إذا( pMem==0 ) أرجع SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[i_ميدان-sz_ميدان], t, pMem);
  pMem->enc = ENC(db);
  أرجع SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
فراغ sqlite3Stat4ProbeFree(Unpacked_رڪارڊ *pRec){
  إذا( pRec ){
    صحيح i;
    صحيح n_کانو = pRec->p_ڪنجي_Info->n_All_ميدان;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    لكل(i=0; i<n_کانو; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_ڪنجي_Info_Unref(pRec->p_ڪنجي_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#جيڪڏهن_پورو /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
فراغ sqlite3_قدر_سيٽ_Str(
  sqlite3_قدر *v,     /* Value to be set */
  صحيح n,                /* Length of string z */
  ثابت فراغ *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  فراغ (*xDel)(فراغ*)   /* Destructor for the string */
){
  إذا( v ) sqlite3_Vdbe_Mem_سيٽ_Str((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
فراغ sqlite3_قدر_Free(sqlite3_قدر *v){
  إذا( !v ) أرجع;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
ساكن SQLITE_NOINLINE صحيح قدر_بائيٽون_2(sqlite3_قدر *pVal, u8 enc){
  أرجع قدر_منزل_متن(pVal, enc)!=0 ? pVal->n : 0;
}
صحيح sqlite3_قدر_بائيٽون_2(sqlite3_قدر *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  أكد( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  إذا( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    أرجع p->n;
  }
  إذا( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    أرجع p->n;
  }
  إذا( (p->flags & MEM_Blob)!=0 ){
    إذا( p->flags & MEM_Zero ){
      أرجع p->n + p->u.nZero;
    }وإلا{
      أرجع p->n;
    }
  }
  إذا( p->flags & MEM_Null ) أرجع 0;
  أرجع قدر_بائيٽون_2(pVal, enc);
}
