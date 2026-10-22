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
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

/* True if X is a power of two.  0 is considered a power of two here.
** In other words, return true if X has at most one bit set.
*/
#تعریف_کرو ISPOWEROF2(X)  (((X)&((X)-1))==0)

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Check invariants on a Mem object.
**
** This routine is intended for use inside of assert() statements, like
** this:    assert( sqlite3VdbeCheckMemInvariants(pMem) );
*/
صحیح sqlite3VdbeCheckMemInvariants(Mem *p){
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

  اگر( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    أكد( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    اگر( (p->flags & (MEM_رکن|MEM_Subtype))==(MEM_رکن|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      أكد( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      أكد( (p->flags & ~(MEM_Null|MEM_رکن|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }ورنہ{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }ورنہ{
    /* The MEM_Cleared bit is only allowed on NULLs */
    أكد( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  أكد( p->szMalloc==0
       || (p->flags==MEM_Undefined 
           && p->szMalloc<=sqlite3_Db_Malloc_سائز(p->db,p->zMalloc))
       || p->szMalloc==sqlite3_Db_Malloc_سائز(p->db,p->zMalloc));

  /* If p holds a string or blob, the Mem.z must point to exactly
  ** one of the following:
  **
  **   (1) Memory in Mem.zMalloc and managed by the Mem object
  **   (2) Memory to be freed using Mem.xDel
  **   (3) An ephemeral string or blob
  **   (4) A static string or blob
  */
  اگر( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    أكد( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  واپس 1;
}
#اگر_ختم

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
ساکن خالی vdbe_Mem_Render_عدد(صحیح sz, حرف *zBuf, Mem *p){
  StrAccum acc;
  أكد( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  أكد( sz>22 );
  اگر( p->flags & (MEM_Int|MEM_IntReal) ){
#اگر GCC_نسخہ>=7000000 && GCC_نسخہ<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    أكد( (sqlite3Config.bSmallMalloc!=0xee)*8==حجم(x) );
    حافظہ_نقل(&x, (حرف*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3_Int64_مقصد_متن(x, zBuf);
#ورنہ
    p->n = sqlite3_Int64_مقصد_متن(p->u.i, zBuf);
#اگر_ختم
    اگر( p->flags & MEM_IntReal ){
      حافظہ_نقل(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }ورنہ{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    أكد( acc.z_متن==zBuf && acc.mxAlloc<=0 );
    zBuf[acc.nChar] = 0; /* Fast version of sqlite3StrAccumFinish(&acc) */
    p->n = acc.nChar;
  }
}

#اگر_تعریف_شدہ SQLITE_DEBUG
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
صحیح sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  حرف zBuf[100];
  حرف *z;
  صحیح i, j, incr;
  اگر( (p->flags & MEM_Str)==0 ) واپس 1;
  اگر( p->db && p->db->mallocFailed ) واپس 1;
  اگر( p->flags & MEM_رکن ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    اگر( p->szMalloc>0 && p->z==p->zMalloc ){
      أكد( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      أكد( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    أكد( p->z[p->n]==0 );
    أكد( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    أكد( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  اگر( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) واپس 1;
  اگر( p->db==0 ){
    واپس 1;  /* db->nFpDigit required to validate p->z[] */
  }
  حافظہ_نقل(&tmp, p, حجم(tmp));
  vdbe_Mem_Render_عدد(حجم(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  اگر( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    اگر( p->enc==SQLITE_UTF16BE ) z++;
  }
  جب( zBuf[j] ){
    اگر( zBuf[j++]!=z[i] ) واپس 0;
    i += incr;
  }
  واپس 1;
}
#اگر_ختم /* SQLITE_DEBUG */

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
صحیح sqlite3VdbeChangeEncoding(Mem *pMem, صحیح desiredEnc){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  صحیح rc;
#اگر_ختم
  أكد( pMem!=0 );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  أكد( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  اگر( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    واپس SQLITE_OK;
  }
  اگر( pMem->enc==desiredEnc ){
    واپس SQLITE_OK;
  }
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#اگر_تعریف_شدہ SQLITE_OMIT_UTF16
  واپس SQLITE_ERROR;
#ورنہ

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  أكد(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  أكد(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  أكد(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  واپس rc;
#اگر_ختم
}

/*
** Make sure pMem->z points to a writable allocation of at least n bytes.
**
** If the bPreserve argument is true, then copy of the content of
** pMem->z into the new allocation.  pMem must be either a string or
** blob if bPreserve is true.  If bPreserve is false, any prior content
** in pMem->z is discarded.
*/
SQLITE_NOINLINE صحیح sqlite3VdbeMemGrow(Mem *pMem, صحیح n, صحیح bPreserve){
  أكد( sqlite3VdbeCheckMemInvariants(pMem) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  أكد( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  أكد( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3_Db_Malloc_سائز(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3_Db_Malloc_سائز(pMem->db,pMem->zMalloc));
  اگر( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    اگر( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }ورنہ{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      اگر( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }ورنہ{
    اگر( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  اگر( pMem->zMalloc==0 ){
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    واپس SQLITE_NOMEM_BKPT;
  }ورنہ{
    pMem->szMalloc = sqlite3_Db_Malloc_سائز(pMem->db, pMem->zMalloc);
  }

  اگر( bPreserve && pMem->z ){
    أكد( pMem->z!=pMem->zMalloc );
    حافظہ_نقل(pMem->zMalloc, pMem->z, pMem->n);
  }
  اگر( (pMem->flags&MEM_Dyn)!=0 ){
    أكد( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((خالی *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  واپس SQLITE_OK;
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
صحیح sqlite3VdbeMemClearAndResize(Mem *pMem, صحیح szNew){
  أكد( CORRUPT_DB || szNew>0 );
  أكد( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  اگر( pMem->szMalloc<szNew ){
    واپس sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  أكد( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  واپس SQLITE_OK;
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
صحیح sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  اگر( (pMem->flags & (MEM_Str|MEM_رکن|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    واپس 0;
  }
  اگر( pMem->enc!=SQLITE_UTF8 ) واپس 0;
  أكد( pMem->z!=0 );
  اگر( pMem->flags & MEM_Dyn ){
    اگر( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_رکن;
      واپس 1;
    }
    اگر( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_رکن;
      واپس 1;
    }
  }ورنہ اگر( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_رکن;
    واپس 1;
  }
  واپس 0;
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
ساکن SQLITE_NOINLINE صحیح vdbe_Mem_جوڑو_Terminator(Mem *pMem){
  اگر( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    واپس SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_رکن;
  واپس SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
صحیح sqlite3VdbeMemMakeWriteable(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  اگر( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    اگر( ExpandBlob(pMem) ) واپس SQLITE_NOMEM;
    اگر( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      صحیح rc = vdbe_Mem_جوڑو_Terminator(pMem);
      اگر( rc ) واپس rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#اگر_تعریف_شدہ SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#اگر_ختم

  واپس SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
صحیح sqlite3VdbeMemExpandBlob(Mem *pMem){
  صحیح nByte;
  أكد( pMem!=0 );
  أكد( pMem->flags & MEM_Zero );
  أكد( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_قدر_nochange(pMem) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  اگر( nByte<=0 ){
    اگر( (pMem->flags & MEM_Blob)==0 ) واپس SQLITE_OK;
    nByte = 1;
  }
  اگر( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    واپس SQLITE_NOMEM_BKPT;
  }
  أكد( pMem->z!=0 );
  أكد( sqlite3_Db_Malloc_سائز(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_رکن);
  واپس SQLITE_OK;
}
#اگر_ختم

/*
** Make sure the given Mem is \u0000 terminated.
*/
صحیح sqlite3VdbeMemNulTerminate(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_رکن|MEM_Str))==(MEM_رکن|MEM_Str) );
  testcase( (pMem->flags & (MEM_رکن|MEM_Str))==0 );
  اگر( (pMem->flags & (MEM_رکن|MEM_Str))!=MEM_Str ){
    واپس SQLITE_OK;   /* Nothing to do */
  }ورنہ{
    واپس vdbe_Mem_جوڑو_Terminator(pMem);
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
صحیح sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  ثابت صحیح nByte = 32;

  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !(pMem->flags&MEM_Zero) );
  أكد( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  أكد( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );


  اگر( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    واپس SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_عدد(nByte, pMem->z, pMem);
  أكد( pMem->z!=0 );
  أكد( pMem->n==(صحیح)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_رکن;
  اگر( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  واپس SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
صحیح sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
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
  ctx.p_برآمد = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  أكد( (pMem->flags & MEM_Dyn)==0 );
  اگر( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  حافظہ_نقل(pMem, &t, حجم(t));
  واپس ctx.isError;
}

/*
** Memory cell pAccum contains the context of an aggregate function.
** This routine calls the xValue method for that function and stores
** the results in memory cell pMem.
**
** SQLITE_ERROR is returned if xValue() reports an error. SQLITE_OK 
** otherwise.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
صحیح sqlite3_Vdbe_Mem_Agg_قدر(Mem *pAccum, Mem *p_برآمد, FuncDef *pFunc){
  sqlite3_context ctx;
  أكد( pFunc!=0 );
  أكد( pFunc->x_قدر!=0 );
  أكد( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  أكد( pAccum->db!=0 );
  أكد( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, حجم(ctx));
  sqlite3_Vdbe_Mem_سیٹ_Null(p_برآمد);
  ctx.p_برآمد = p_برآمد;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_قدر(&ctx);
  واپس ctx.isError;
}
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

/*
** If the memory cell contains a value that must be freed by
** invoking the external callback in Mem.xDel, then this routine
** will free that value.  It also sets Mem.flags to MEM_Null.
**
** This is a helper routine for sqlite3VdbeMemSetNull() and
** for sqlite3VdbeMemRelease().  Use those other routines as the
** entry point for releasing Mem resources.
*/
ساکن SQLITE_NOINLINE خالی vdbe_Mem_Clear_Extern_And_سیٹ_Null(Mem *p){
  أكد( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  أكد( VdbeMemDynamic(p) );
  اگر( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    أكد( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  اگر( p->flags&MEM_Dyn ){
    أكد( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((خالی *)p->z);
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
ساکن SQLITE_NOINLINE خالی vdbeMemClear(Mem *p){
  اگر( VdbeMemDynamic(p) ){
    vdbe_Mem_Clear_Extern_And_سیٹ_Null(p);
  }
  اگر( p->szMalloc ){
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
خالی sqlite3VdbeMemRelease(Mem *p){
  أكد( sqlite3VdbeCheckMemInvariants(p) );
  اگر( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
خالی sqlite3VdbeMemReleaseMalloc(Mem *p){
  أكد( !VdbeMemDynamic(p) );
  اگر( p->szMalloc ) vdbeMemClear(p);
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
ساکن SQLITE_NOINLINE i64 mem_Int_قدر(ثابت Mem *pMem){
  i64 قدر = 0;
  sqlite3Atoi64(pMem->z, &قدر, pMem->n, pMem->enc);
  واپس قدر;
}
i64 sqlite3_Vdbe_Int_قدر(ثابت Mem *pMem){
  صحیح flags;
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  اگر( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    واپس pMem->u.i;
  }ورنہ اگر( flags & MEM_Real ){
    واپس sqlite3_Real_مقصد_I64(pMem->u.r);
  }ورنہ اگر( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    واپس mem_Int_قدر(pMem);
  }ورنہ{
    واپس 0;
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
ساکن SQLITE_NOINLINE صحیح sqlite3_Mem_Real_قدر_RCSlow_راستہ(
  Mem *pMem,
  دوہرا *p_قدر
){
  صحیح rc = SQLITE_OK;
  *p_قدر = 0.0;
  اگر( pMem->enc==SQLITE_UTF8 ){
    حرف *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    اگر( zCopy ){
      rc = sqlite3AtoF(zCopy, p_قدر);
      sqlite3DbFree(pMem->db, zCopy);
    }
    واپس rc;
  }ورنہ{
    صحیح n, i, j;
    حرف *zCopy;
    ثابت حرف *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    اگر( zCopy ){
      z = pMem->z;
      اگر( pMem->enc==SQLITE_UTF16LE ){
        برائے(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          اگر( z[i+1]!=0 ) توڑو;
        }
      }ورنہ{
        برائے(i=j=0; i<n-1; i+=2, j++){
          اگر( z[i]!=0 ) توڑو;
          zCopy[j] = z[i+1];
        }
      }     
      أكد( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_قدر);
      اگر( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    واپس rc;
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
صحیح sqlite3_Mem_Real_قدر_RC(Mem *pMem, دوہرا *p_قدر){
  testcase( pMem->db==0 );
  أكد( pMem->flags & (MEM_Str|MEM_Blob) );
  اگر( pMem->z==0 ){
    *p_قدر = 0.0;
    واپس 0;
  }ورنہ اگر( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_رکن)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    واپس sqlite3AtoF(pMem->z, p_قدر);
  }ورنہ اگر( pMem->n==0 ){
    *p_قدر = 0.0;
    واپس 0;
  }ورنہ{
    واپس sqlite3_Mem_Real_قدر_RCSlow_راستہ(pMem, p_قدر);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
ساکن SQLITE_NOINLINE دوہرا sqlite3_Mem_Real_قدر_No_RC(Mem *pMem){
  دوہرا r;
  (خالی)sqlite3_Mem_Real_قدر_RC(pMem, &r);
  واپس r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
دوہرا sqlite3_Vdbe_Real_قدر(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );
  اگر( pMem->flags & MEM_Real ){
    واپس pMem->u.r;
  }ورنہ اگر( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    واپس (دوہرا)pMem->u.i;
  }ورنہ اگر( pMem->flags & (MEM_Str|MEM_Blob) ){
    واپس sqlite3_Mem_Real_قدر_No_RC(pMem);
  }ورنہ{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    واپس (دوہرا)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
صحیح sqlite3_Vdbe_Boolean_قدر(Mem *pMem, صحیح ifNull){
  testcase( pMem->flags & MEM_IntReal );
  اگر( pMem->flags & (MEM_Int|MEM_IntReal) ) واپس pMem->u.i!=0;
  اگر( pMem->flags & MEM_Null ) واپس ifNull;
  واپس sqlite3_Vdbe_Real_قدر(pMem)!=0.0;
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
خالی sqlite3VdbeIntegerAffinity(Mem *pMem, صحیح b_From_متن){
  أكد( pMem!=0 );
  أكد( pMem->flags & (MEM_Real|MEM_IntReal) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );

  اگر( pMem->flags & MEM_IntReal ){
    Mem_سیٹ_Type_Flag(pMem, MEM_Int);
  }ورنہ{
    i64 ix = sqlite3_Real_مقصد_I64(pMem->u.r);

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
    اگر( pMem->u.r==ix && ix<سب_سے_بڑا_INT64
     && (!b_From_متن || ix>سب_سے_چھوٹا_INT64)
    ){
      pMem->u.i = ix;
      Mem_سیٹ_Type_Flag(pMem, MEM_Int);
    }
  }
}

/*
** Convert pMem to type integer.  Invalidate any prior representations.
*/
صحیح sqlite3VdbeMemIntegerify(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_قدر(pMem);
  Mem_سیٹ_Type_Flag(pMem, MEM_Int);
  واپس SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
صحیح sqlite3VdbeMemRealify(Mem *pMem){
  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_قدر(pMem);
  Mem_سیٹ_Type_Flag(pMem, MEM_Real);
  واپس SQLITE_OK;
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
صحیح sqlite3RealSameAsInt(دوہرا r1, sqlite3_int64 i){
  دوہرا r2 = (دوہرا)i;
  واپس r1==0.0
      || (memcmp(&r1, &r2, حجم(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3_Real_مقصد_I64(دوہرا r){
  اگر( r<-9223372036854774784.0 ) واپس سب_سے_چھوٹا_INT64;
  اگر( r>+9223372036854774784.0 ) واپس سب_سے_بڑا_INT64;
  واپس (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
صحیح sqlite3VdbeMemNumerify(Mem *pMem){
  أكد( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  اگر( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    صحیح rc;
    sqlite3_int64 ix;
    أكد( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_قدر_RC(pMem, &pMem->u.r);
    اگر( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3_Real_مقصد_I64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      Mem_سیٹ_Type_Flag(pMem, MEM_Int);
    }ورنہ{
      Mem_سیٹ_Type_Flag(pMem, MEM_Real);
    }
  }
  أكد( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  واپس SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
صحیح sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  اگر( pMem->flags & MEM_Null ) واپس SQLITE_OK;
  انتخاب( aff ){
    حالت SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      اگر( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_قدر_Apply_Affinity(pMem, SQLITE_AFF_متن, encoding);
        أكد( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        اگر( pMem->flags & MEM_Str ) Mem_سیٹ_Type_Flag(pMem, MEM_Blob);
      }ورنہ{
        pMem->flags &= ~(MEM_Type_نقاب&~MEM_Blob);
      }
      توڑو;
    }
    حالت SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      توڑو;
    }
    حالت SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      توڑو;
    }
    حالت SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      توڑو;
    }
    طےشدہ: {
      صحیح rc;
      أكد( aff==SQLITE_AFF_متن );
      أكد( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_قدر_Apply_Affinity(pMem, SQLITE_AFF_متن, encoding);
      أكد( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      اگر( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      اگر( rc ) واپس rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  واپس SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
خالی sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
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
خالی sqlite3_Vdbe_Mem_سیٹ_Null(Mem *pMem){
  اگر( VdbeMemDynamic(pMem) ){
    vdbe_Mem_Clear_Extern_And_سیٹ_Null(pMem);
  }ورنہ{
    pMem->flags = MEM_Null;
  }
}
خالی sqlite3_قدر_سیٹ_Null(sqlite3_قدر *p){
  sqlite3_Vdbe_Mem_سیٹ_Null((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
خالی sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(Mem *pMem, صحیح n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  اگر( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#ورنہ
صحیح sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(Mem *pMem, صحیح n){
  صحیح nByte = n>0?n:1;
  اگر( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    واپس SQLITE_NOMEM_BKPT;
  }
  أكد( pMem->z!=0 );
  أكد( sqlite3_Db_Malloc_سائز(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  واپس SQLITE_OK;
}
#اگر_ختم

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
ساکن SQLITE_NOINLINE خالی vdbe_Release_And_سیٹ_Int64(Mem *pMem, i64 val){
  sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
خالی sqlite3_Vdbe_Mem_سیٹ_Int64(Mem *pMem, i64 val){
  اگر( VdbeMemDynamic(pMem) ){
    vdbe_Release_And_سیٹ_Int64(pMem, val);
  }ورنہ{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
خالی sqlite3_Mem_سیٹ_Array_Int64(sqlite3_قدر *aMem, صحیح iIdx, i64 val){
  sqlite3_Vdbe_Mem_سیٹ_Int64(&aMem[iIdx], val);
}

/* A no-op destructor */
خالی sqlite3NoopDestructor(خالی *p){ غیر_مستعمل_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
خالی sqlite3_Vdbe_Mem_سیٹ_اشارہ_گر(
  Mem *pMem,
  خالی *pPtr,
  ثابت حرف *zPType,
  خالی (*xDestructor)(خالی*)
){
  أكد( pMem->flags==MEM_Null );
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = MEM_Null|MEM_Dyn|MEM_Subtype|MEM_رکن;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type REAL.
*/
خالی sqlite3_Vdbe_Mem_سیٹ_Double(Mem *pMem, دوہرا val){
  sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
  اگر( !sqlite3IsNaN(val) ){
    pMem->u.r = val;
    pMem->flags = MEM_Real;
  }
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Return true if the Mem holds a RowSet object.  This routine is intended
** for use inside of assert() statements.
*/
صحیح sqlite3_Vdbe_Mem_Is_صف_سیٹ(ثابت Mem *pMem){
  واپس (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
         && pMem->xDel==sqlite3_صف_سیٹ_Delete;
}
#اگر_ختم

/*
** Delete any previous value and set the value of pMem to be an
** empty boolean index.
**
** Return SQLITE_OK on success and SQLITE_NOMEM if a memory allocation
** error occurs.
*/
صحیح sqlite3_Vdbe_Mem_سیٹ_صف_سیٹ(Mem *pMem){
  sqlite3 *db = pMem->db;
  صف_سیٹ *p;
  أكد( db!=0 );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3_صف_سیٹ_Init(db);
  اگر( p==0 ) واپس SQLITE_NOMEM;
  pMem->z = (حرف*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3_صف_سیٹ_Delete;
  واپس SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
صحیح sqlite3VdbeMemTooBig(Mem *p){
  أكد( p->db!=0 );
  اگر( p->flags & (MEM_Str|MEM_Blob) ){
    صحیح n = p->n;
    اگر( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    واپس n>p->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی];
  }
  واپس 0; 
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** This routine prepares a memory cell for modification by breaking
** its link to a shallow copy and by marking any current shallow
** copies of this cell as invalid.
**
** This is used for testing and debugging only - to help ensure that shallow
** copies (created by OP_SCopy) are not misused.
*/
خالی sqlite3_Vdbe_Mem_About_مقصد_Change(Vdbe *pVdbe, Mem *pMem){
  صحیح i;
  Mem *pX;
  اگر( pMem->bScopy ){
    برائے(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      اگر( pX->pScopyFrom==pMem ){
        u16 mFlags;
        اگر( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (صحیح)(pX - pVdbe->aMem), (صحیح)(pMem - pVdbe->aMem));
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
#اگر_ختم /* SQLITE_DEBUG */

/*
** Make an shallow copy of pFrom into pTo.  Prior contents of
** pTo are freed.  The pFrom->z field is not duplicated.  If
** pFrom->z is used, then pTo->z points to the same thing as pFrom->z
** and flags gets srcType (either MEM_Ephem or MEM_Static).
*/
ساکن SQLITE_NOINLINE خالی vdbeClrCopy(Mem *p_مقصد, ثابت Mem *pFrom, صحیح eType){
  vdbe_Mem_Clear_Extern_And_سیٹ_Null(p_مقصد);
  أكد( !VdbeMemDynamic(p_مقصد) );
  sqlite3VdbeMemShallowCopy(p_مقصد, pFrom, eType);
}
خالی sqlite3VdbeMemShallowCopy(Mem *p_مقصد, ثابت Mem *pFrom, صحیح srcType){
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pFrom) );
  أكد( p_مقصد->db==pFrom->db );
  اگر( VdbeMemDynamic(p_مقصد) ){ vdbeClrCopy(p_مقصد,pFrom,srcType); واپس; }
  حافظہ_نقل(p_مقصد, pFrom, MEMCELLSIZE);
  اگر( (pFrom->flags&MEM_Static)==0 ){
    p_مقصد->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    أكد( srcType==MEM_Ephem || srcType==MEM_Static );
    p_مقصد->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
صحیح sqlite3VdbeMemCopy(Mem *p_مقصد, ثابت Mem *pFrom){
  صحیح rc = SQLITE_OK;

  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pFrom) );
  اگر( VdbeMemDynamic(p_مقصد) ) vdbe_Mem_Clear_Extern_And_سیٹ_Null(p_مقصد);
  حافظہ_نقل(p_مقصد, pFrom, MEMCELLSIZE);
  p_مقصد->flags &= ~MEM_Dyn;
  اگر( p_مقصد->flags&(MEM_Str|MEM_Blob) ){
    اگر( 0==(pFrom->flags&MEM_Static) ){
      p_مقصد->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(p_مقصد);
    }
  }

  واپس rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
خالی sqlite3VdbeMemMove(Mem *p_مقصد, Mem *pFrom){
  أكد( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  أكد( p_مقصد->db==0 || sqlite3_mutex_held(p_مقصد->db->mutex) );
  أكد( pFrom->db==0 || p_مقصد->db==0 || pFrom->db==p_مقصد->db );

  sqlite3VdbeMemRelease(p_مقصد);
  حافظہ_نقل(p_مقصد, pFrom, حجم(Mem));
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
صحیح sqlite3_Vdbe_Mem_سیٹ_Str(
  Mem *pMem,          /* Memory cell to set to string value */
  ثابت حرف *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  خالی (*xDel)(خالی*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  صحیح i_حد_بندی;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  أكد( pMem!=0 );
  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  أكد( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  اگر( !z ){
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
    واپس SQLITE_OK;
  }

  اگر( pMem->db ){
    i_حد_بندی = pMem->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی];
  }ورنہ{
    i_حد_بندی = SQLITE_MAX_لمبائی;
  }
  اگر( nByte<0 ){
    أكد( enc!=0 );
    اگر( enc==SQLITE_UTF8 ){
      nByte = لڑی_لمبائی(z);
    }ورنہ{
      برائے(nByte=0; nByte<=i_حد_بندی && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_رکن;
  }ورنہ اگر( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }ورنہ{
    flags = MEM_Str;
  }
  اگر( nByte>i_حد_بندی ){
    اگر( xDel && xDel!=SQLITE_TRANSIENT ){
      اگر( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (خالی*)z);
      }ورنہ{
        xDel((خالی*)z);
      }
    }
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
    واپس sqlite3_Error_مقصد_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  اگر( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    اگر( flags&MEM_رکن ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    اگر( sqlite3VdbeMemClearAndResize(pMem, (صحیح)MAX(nAlloc,32)) ){
      واپس SQLITE_NOMEM_BKPT;
    }
    أكد( pMem->z!=0 );
    حافظہ_نقل(pMem->z, z, nAlloc);
  }ورنہ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (حرف *)z;
    اگر( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_سائز(pMem->db, pMem->zMalloc);
    }ورنہ{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (صحیح)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  اگر( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    واپس SQLITE_NOMEM_BKPT;
  }
#اگر_ختم


  واپس SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
صحیح sqlite3_Vdbe_Mem_سیٹ_متن(
  Mem *pMem,          /* Memory cell to set to string value */
  ثابت حرف *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  خالی (*xDel)(خالی*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  أكد( pMem!=0 );
  أكد( pMem->db!=0 );
  أكد( sqlite3_mutex_held(pMem->db->mutex) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  اگر( !z ){
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
    واپس SQLITE_OK;
  }

  اگر( nByte<0 ){
    nByte = لڑی_لمبائی(z);
    flags = MEM_Str|MEM_رکن;
  }ورنہ{
    flags = MEM_Str;
  }
  اگر( nByte>(i64)pMem->db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] ){
    اگر( xDel && xDel!=SQLITE_TRANSIENT ){
      اگر( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (خالی*)z);
      }ورنہ{
        xDel((خالی*)z);
      }
    }
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem);
    واپس sqlite3_Error_مقصد_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  اگر( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    اگر( sqlite3VdbeMemClearAndResize(pMem, (صحیح)MAX(nAlloc,32)) ){
      واپس SQLITE_NOMEM_BKPT;
    }
    أكد( pMem->z!=0 );
    حافظہ_نقل(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }ورنہ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (حرف *)z;
    اگر( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_سائز(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }ورنہ اگر( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }ورنہ{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (صحیح)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  واپس SQLITE_OK;
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
صحیح sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 انحراف,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  صحیح rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_تخصیص_سائز-1 );
  testcase( amt==SQLITE_MAX_تخصیص_سائز );
  اگر( amt>=SQLITE_MAX_تخصیص_سائز ){
    واپس SQLITE_NOMEM_BKPT;
  }
  اگر( (u64)amt + (u64)انحراف > (u64)sqlite3_Btree_Max_ریکارڈ_سائز(pCur) ){
    واپس SQLITE_CORRUPT_BKPT;
  }
  اگر( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, انحراف, amt, pMem->z);
    اگر( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (صحیح)amt;
    }ورنہ{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  واپس rc;
}
صحیح sqlite3_Vdbe_Mem_From_Btree_Zero_انحراف(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 دستیاب = 0;  /* Number of bytes available on the local btree page */
  صحیح rc = SQLITE_OK; /* Return code */

  أكد( sqlite3BtreeCursorIsValid(pCur) );
  أكد( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem) );
  pMem->z = (حرف *)sqlite3BtreePayloadFetch(pCur, &دستیاب);
  أكد( pMem->z!=0 );

  اگر( amt<=دستیاب ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (صحیح)amt;
  }ورنہ{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  واپس rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
ساکن SQLITE_NOINLINE ثابت خالی *قدر_مقصد_متن(sqlite3_قدر* pVal, u8 enc){
  أكد( pVal!=0 );
  أكد( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  أكد( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pVal) );
  أكد( (pVal->flags & (MEM_Null))==0 );
  اگر( pVal->flags & (MEM_Blob|MEM_Str) ){
    اگر( ExpandBlob(pVal) ) واپس 0;
    pVal->flags |= MEM_Str;
    اگر( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    اگر( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_مقصد_INT(pVal->z)) ){
      أكد( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      اگر( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        واپس 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }ورنہ{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    أكد( 0==(1&SQLITE_PTR_مقصد_INT(pVal->z)) );
  }
  أكد(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  اگر( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    أكد( sqlite3VdbeMemValidStrRep(pVal) );
    واپس pVal->z;
  }ورنہ{
    واپس 0;
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
ثابت خالی *sqlite3_قدر_متن_2(sqlite3_قدر* pVal, u8 enc){
  اگر( !pVal ) واپس 0;
  أكد( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  أكد( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pVal) );
  اگر( (pVal->flags&(MEM_Str|MEM_رکن))==(MEM_Str|MEM_رکن) && pVal->enc==enc ){
    أكد( sqlite3VdbeMemValidStrRep(pVal) );
    واپس pVal->z;
  }
  اگر( pVal->flags&MEM_Null ){
    واپس 0;
  }
  واپس قدر_مقصد_متن(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
صحیح sqlite3_قدر_Is_کا_Class(ثابت sqlite3_قدر *pVal, خالی(*xFree)(خالی*)){
  اگر( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    واپس 1;
  }ورنہ{
    واپس 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_قدر *sqlite3_قدر_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, حجم(*p));
  اگر( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  واپس p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
ساخت قدر_New_Stat4_Ctx {
  Parse *pParse;
  اشاریہ *pIdx;
  Unpacked_ریکارڈ **ppRec;
  صحیح iVal;
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
ساکن sqlite3_قدر *قدر_New(sqlite3 *db, ساخت قدر_New_Stat4_Ctx *p){
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( p ){
    Unpacked_ریکارڈ *pRec = p->ppRec[0];

    اگر( pRec==0 ){
      اشاریہ *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      صحیح i;                      /* Counter variable */
      صحیح n_ستون = pIdx->n_عمود;   /* Number of index columns including rowid */
  
      nByte = حجم(Mem) * n_ستون + ROUND8(حجم(Unpacked_ریکارڈ));
      pRec = (Unpacked_ریکارڈ*)sqlite3DbMallocZero(db, nByte);
      اگر( pRec ){
        pRec->p_کلید_Info = sqlite3_کلید_Info_کا_اشاریہ(p->pParse, pIdx);
        اگر( pRec->p_کلید_Info ){
          أكد( pRec->p_کلید_Info->n_All_میدان==n_ستون );
          أكد( pRec->p_کلید_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(حجم(Unpacked_ریکارڈ)));
          برائے(i=0; i<n_ستون; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }ورنہ{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      اگر( pRec==0 ) واپس 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->n_میدان = p->iVal+1;
    sqlite3_Vdbe_Mem_سیٹ_Null(&pRec->aMem[p->iVal]);
    واپس &pRec->aMem[p->iVal];
  }
#ورنہ
  غیر_مستعمل_PARAMETER(p);
#اگر_ختم /* defined(SQLITE_ENABLE_STAT4) */
  واپس sqlite3_قدر_New(db);
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
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
ساکن صحیح قدر_From_Function(
  sqlite3 *db,                    /* The database connection */
  ثابت اظہاریہ *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_قدر **ppVal,          /* Write the new value here */
  ساخت قدر_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_قدر **apVal = 0;      /* Function arguments */
  صحیح nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_قدر *pVal = 0;        /* New value */
  صحیح rc = SQLITE_OK;             /* Return code */
  اظہاریہ_List *pList = 0;            /* Function arguments */
  صحیح i;                          /* Iterator variable */

  أكد( pCtx!=0 );
  أكد( (p->flags & EP_TokenOnly)==0 );
  أكد( اظہاریہ_Use_XList(p) );
  pList = p->x.pList;
  اگر( pList ) nVal = pList->n_اظہاریہ;
  أكد( !اظہاریہ_Has_Property(p, EP_Int_قدر) );
  pFunc = sqlite3_ڈھونڈو_Function(db, p->u.zToken, nVal, enc, 0);
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  اگر( pFunc==0 ) واپس SQLITE_OK;
#اگر_ختم
  أكد( pFunc );
  اگر( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    واپس SQLITE_OK;
  }

  اگر( pList ){
    apVal = (sqlite3_قدر**)sqlite3DbMallocZero(db, حجم(apVal[0]) * nVal);
    اگر( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      جاؤ قدر_from_function_برآمد;
    }
    برائے(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_قدر_From_اظہاریہ(pCtx->pParse, pList->a[i].p_اظہاریہ, aff,
                                     &apVal[i]);
      اگر( apVal[i]==0 || rc!=SQLITE_OK ) جاؤ قدر_from_function_برآمد;
    }
  }

  pVal = قدر_New(db, pCtx);
  اگر( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    جاؤ قدر_from_function_برآمد;
  }

  memset(&ctx, 0, حجم(ctx));
  ctx.p_برآمد = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  اگر( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_پیغام(pCtx->pParse, "%s", sqlite3_قدر_متن(pVal));
  }ورنہ{
    sqlite3_قدر_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    أكد( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    اگر( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 قدر_from_function_برآمد:
  اگر( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  اگر( apVal ){
    برائے(i=0; i<nVal; i++){
      sqlite3_قدر_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  واپس rc;
}
#ورنہ
# define قدر_From_Function(a,b,c,d,e,f) SQLITE_OK
#اگر_ختم /* defined(SQLITE_ENABLE_STAT4) */

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
ساکن صحیح قدر_From_اظہاریہ(
  sqlite3 *db,                    /* The database connection */
  ثابت اظہاریہ *p_اظہاریہ,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_قدر **ppVal,          /* Write the new value here */
  ساخت قدر_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  صحیح op;
  حرف *zVal = 0;
  sqlite3_قدر *pVal = 0;
  صحیح negInt = 1;
  ثابت حرف *zNeg = "";
  صحیح rc = SQLITE_OK;

  أكد( p_اظہاریہ!=0 );
  جب( (op = p_اظہاریہ->op)==TK_UPLUS || op==TK_وسعت ) p_اظہاریہ = p_اظہاریہ->p_بایاں;
  اگر( op==TK_رجسٹر ) op = p_اظہاریہ->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  أكد( (p_اظہاریہ->flags & EP_TokenOnly)==0 || pCtx==0 );

  اگر( op==TK_CAST ){
    u8 aff;
    أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
    aff = sqlite3AffinityType(p_اظہاریہ->u.zToken,0);
    rc = قدر_From_اظہاریہ(db, p_اظہاریہ->p_بایاں, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    اگر( *ppVal ){
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#ورنہ
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      أكد( (ppVal[0][0].flags & MEM_Zero)==0 );
#اگر_ختم
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_قدر_Apply_Affinity(*ppVal, affinity, enc);
    }
    واپس rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  اگر( op==TK_UMINUS ){
    اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
    اگر( (p_بایاں->op==TK_INTEGER || p_بایاں->op==TK_FLOAT) ){
      اگر( اظہاریہ_Has_Property(p_بایاں, EP_Int_قدر)
       || p_بایاں->u.zToken[0]!='0' || (p_بایاں->u.zToken[1] & ~0x20)!='X'
      ){
        p_اظہاریہ = p_بایاں;
        op = p_اظہاریہ->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  اگر( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = قدر_New(db, pCtx);
    اگر( pVal==0 ) جاؤ no_mem;
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) ){
      sqlite3_Vdbe_Mem_سیٹ_Int64(pVal, (i64)p_اظہاریہ->u.i_قدر*negInt);
    }ورنہ{
      i64 iVal;
      اگر( op==TK_INTEGER && 0==sqlite3_Dec_Or_Hex_مقصد_I64(p_اظہاریہ->u.zToken, &iVal) ){
        sqlite3_Vdbe_Mem_سیٹ_Int64(pVal, iVal*negInt);
      }ورنہ{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_اظہاریہ->u.zToken);
        اگر( zVal==0 ) جاؤ no_mem;
        sqlite3_قدر_سیٹ_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        اگر( op==TK_FLOAT ){
          أكد( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_رکن) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    اگر( affinity==SQLITE_AFF_BLOB ){
      اگر( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_قدر_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }ورنہ{
      sqlite3_قدر_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    أكد( (pVal->flags & MEM_IntReal)==0 );
    اگر( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    اگر( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }ورنہ اگر( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    اگر( SQLITE_OK==قدر_From_اظہاریہ(db,p_اظہاریہ->p_بایاں,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      اگر( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }ورنہ اگر( pVal->u.i==سب_سے_چھوٹا_INT64 ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
        pVal->u.r = -(دوہرا)سب_سے_چھوٹا_INT64;
#ورنہ
        pVal->u.r = سب_سے_بڑا_INT64;
#اگر_ختم
        Mem_سیٹ_Type_Flag(pVal, MEM_Real);
      }ورنہ{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_قدر_Apply_Affinity(pVal, affinity, enc);
    }
  }ورنہ اگر( op==TK_NULL ){
    pVal = قدر_New(db, pCtx);
    اگر( pVal==0 ) جاؤ no_mem;
    sqlite3_Vdbe_Mem_سیٹ_Null(pVal);
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_BLOB_LITERAL
  ورنہ اگر( op==TK_BLOB ){
    صحیح nVal;
    أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
    أكد( p_اظہاریہ->u.zToken[0]=='x' || p_اظہاریہ->u.zToken[0]=='X' );
    أكد( p_اظہاریہ->u.zToken[1]=='\'' );
    pVal = قدر_New(db, pCtx);
    اگر( !pVal ) جاؤ no_mem;
    zVal = &p_اظہاریہ->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    أكد( zVal[nVal]=='\'' );
    sqlite3_Vdbe_Mem_سیٹ_Str(pVal, sqlite3_Hex_مقصد_Blob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  ورنہ اگر( op==TK_FUNCTION && pCtx!=0 ){
    rc = قدر_From_Function(db, p_اظہاریہ, enc, affinity, &pVal, pCtx);
  }
#اگر_ختم
  ورنہ اگر( op==TK_TRUEFALSE ){
    أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
    pVal = قدر_New(db, pCtx);
    اگر( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_اظہاریہ->u.zToken[4]==0;
      sqlite3_قدر_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  واپس rc;

no_mem:
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#اگر_ختم
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  أكد( *ppVal==0 );
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( pCtx==0 ) sqlite3_قدر_Free(pVal);
#ورنہ
  أكد( pCtx==0 ); sqlite3_قدر_Free(pVal);
#اگر_ختم
  واپس SQLITE_NOMEM_BKPT;
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
صحیح sqlite3_قدر_From_اظہاریہ(
  sqlite3 *db,              /* The database connection */
  ثابت اظہاریہ *p_اظہاریہ,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_قدر **ppVal     /* Write the new value here */
){
  صحیح rc = SQLITE_OK;
  sqlite3_قدر *pVal = 0;
  اگر( p_اظہاریہ ){
    rc = قدر_From_اظہاریہ(db, p_اظہاریہ, enc, affinity, &pVal, 0);
    اگر( rc==SQLITE_OK && pVal 
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
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
ساکن صحیح stat4_قدر_From_اظہاریہ(
  Parse *pParse,                  /* Parse context */
  اظہاریہ *p_اظہاریہ,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  ساخت قدر_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_قدر **ppVal           /* OUT: New value object (or NULL) */
){
  صحیح rc = SQLITE_OK;
  sqlite3_قدر *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate(p_اظہاریہ);

  أكد( p_اظہاریہ==0 || p_اظہاریہ->op!=TK_رجسٹر || p_اظہاریہ->op2!=TK_VARIABLE );
  اگر( !p_اظہاریہ ){
    pVal = قدر_New(db, pAlloc);
    اگر( pVal ){
      sqlite3_Vdbe_Mem_سیٹ_Null((Mem*)pVal);
    }
  }ورنہ اگر( p_اظہاریہ->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    صحیح iBindVar = p_اظہاریہ->i_عمود;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    اگر( (v = pParse->pReprepare)!=0 ){
      pVal = قدر_New(db, pAlloc);
      اگر( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_قدر_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }ورنہ{
    rc = قدر_From_اظہاریہ(db, p_اظہاریہ, ENC(db), affinity, &pVal, pAlloc);
  }

  أكد( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  واپس rc;
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
صحیح sqlite3_Stat4_Probe_سیٹ_قدر(
  Parse *pParse,                  /* Parse context */
  اشاریہ *pIdx,                    /* Index being probed */
  Unpacked_ریکارڈ **ppRec,         /* IN/OUT: Probe record */
  اظہاریہ *p_اظہاریہ,                    /* The expression to extract a value from */
  صحیح nElem,                      /* Maximum number of values to append */
  صحیح iVal,                       /* Array element to populate */
  صحیح *pn_نکالو                  /* OUT: Values appended to the record */
){
  صحیح rc = SQLITE_OK;
  صحیح n_نکالو = 0;

  اگر( p_اظہاریہ==0 || p_اظہاریہ->op!=TK_SELECT ){
    صحیح i;
    ساخت قدر_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    برائے(i=0; i<nElem; i++){
      sqlite3_قدر *pVal = 0;
      اظہاریہ *pElem = (p_اظہاریہ ? sqlite3_سمتیہ_میدان_Subexpr(p_اظہاریہ, i) : 0);
      u8 aff = sqlite3_اشاریہ_عمود_Affinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_قدر_From_اظہاریہ(pParse, pElem, aff, &alloc, &pVal);
      اگر( !pVal ) توڑو;
      n_نکالو++;
    }
  }

  *pn_نکالو = n_نکالو;
  واپس rc;
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
صحیح sqlite3_Stat4_قدر_From_اظہاریہ(
  Parse *pParse,                  /* Parse context */
  اظہاریہ *p_اظہاریہ,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_قدر **ppVal           /* OUT: New value object (or NULL) */
){
  واپس stat4_قدر_From_اظہاریہ(pParse, p_اظہاریہ, affinity, 0, ppVal);
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
صحیح sqlite3_Stat4_عمود(
  sqlite3 *db,                    /* Database handle */
  ثابت خالی *pRec,               /* Pointer to buffer containing record */
  صحیح nRec,                       /* Size of buffer pRec in bytes */
  صحیح i_ستون,                       /* Column to extract */
  sqlite3_قدر **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 i_میدان;                     /* Next unread data byte */
  u32 sz_میدان = 0;                /* Size of the current data field */
  صحیح i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  أكد( i_ستون>0 );
  iHdr = getVarint32(a, nHdr);
  اگر( nHdr>(u32)nRec || iHdr>=nHdr ) واپس SQLITE_CORRUPT_BKPT;
  i_میدان = nHdr;
  برائے(i=0; i<=i_ستون; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    اگر( iHdr>nHdr ) واپس SQLITE_CORRUPT_BKPT;
    sz_میدان = sqlite3VdbeSerialTypeLen(t);
    i_میدان += sz_میدان;
  }
  testcase( i_میدان==nRec );
  testcase( i_میدان==nRec+1 );
  اگر( i_میدان>nRec ) واپس SQLITE_CORRUPT_BKPT;
  اگر( pMem==0 ){
    pMem = *ppVal = sqlite3_قدر_New(db);
    اگر( pMem==0 ) واپس SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[i_میدان-sz_میدان], t, pMem);
  pMem->enc = ENC(db);
  واپس SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
خالی sqlite3Stat4ProbeFree(Unpacked_ریکارڈ *pRec){
  اگر( pRec ){
    صحیح i;
    صحیح n_ستون = pRec->p_کلید_Info->n_All_میدان;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    برائے(i=0; i<n_ستون; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_کلید_Info_Unref(pRec->p_کلید_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#اگر_ختم /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
خالی sqlite3_قدر_سیٹ_Str(
  sqlite3_قدر *v,     /* Value to be set */
  صحیح n,                /* Length of string z */
  ثابت خالی *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  خالی (*xDel)(خالی*)   /* Destructor for the string */
){
  اگر( v ) sqlite3_Vdbe_Mem_سیٹ_Str((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
خالی sqlite3_قدر_Free(sqlite3_قدر *v){
  اگر( !v ) واپس;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
ساکن SQLITE_NOINLINE صحیح قدر_بائٹیں_2(sqlite3_قدر *pVal, u8 enc){
  واپس قدر_مقصد_متن(pVal, enc)!=0 ? pVal->n : 0;
}
صحیح sqlite3_قدر_بائٹیں_2(sqlite3_قدر *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  أكد( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  اگر( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    واپس p->n;
  }
  اگر( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    واپس p->n;
  }
  اگر( (p->flags & MEM_Blob)!=0 ){
    اگر( p->flags & MEM_Zero ){
      واپس p->n + p->u.nZero;
    }ورنہ{
      واپس p->n;
    }
  }
  اگر( p->flags & MEM_Null ) واپس 0;
  واپس قدر_بائٹیں_2(pVal, enc);
}
