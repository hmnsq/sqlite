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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/* True if X is a power of two.  0 is considered a power of two here.
** In other words, return true if X has at most one bit set.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ISPOWEROF2(X)  (((X)&((X)-1))==0)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Check invariants on a Mem object.
**
** This routine is intended for use inside of assert() statements, like
** this:    assert( sqlite3VdbeCheckMemInvariants(pMem) );
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  ಪ್ರತಿಜ್ಞಾ( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  ಪ್ರತಿಜ್ಞಾ( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  ಪ್ರತಿಜ್ಞಾ( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  ಆದರೆ( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    ಪ್ರತಿಜ್ಞಾ( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    ಆದರೆ( (p->flags & (MEM_ಪದ|MEM_Subtype))==(MEM_ಪದ|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      ಪ್ರತಿಜ್ಞಾ( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      ಪ್ರತಿಜ್ಞಾ( (p->flags & ~(MEM_Null|MEM_ಪದ|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* The MEM_Cleared bit is only allowed on NULLs */
    ಪ್ರತಿಜ್ಞಾ( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  ಪ್ರತಿಜ್ಞಾ( p->szMalloc==0
       || (p->flags==MEM_Undefined 
           && p->szMalloc<=sqlite3_Db_Malloc_ಗಾತ್ರ(p->db,p->zMalloc))
       || p->szMalloc==sqlite3_Db_Malloc_ಗಾತ್ರ(p->db,p->zMalloc));

  /* If p holds a string or blob, the Mem.z must point to exactly
  ** one of the following:
  **
  **   (1) Memory in Mem.zMalloc and managed by the Mem object
  **   (2) Memory to be freed using Mem.xDel
  **   (3) An ephemeral string or blob
  **   (4) A static string or blob
  */
  ಆದರೆ( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    ಪ್ರತಿಜ್ಞಾ( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_Mem_Render_ಅಂಕೆ(ಪೂರ್ಣಾಂಕ sz, ಅಕ್ಷರ *zBuf, Mem *p){
  StrAccum acc;
  ಪ್ರತಿಜ್ಞಾ( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  ಪ್ರತಿಜ್ಞಾ( sz>22 );
  ಆದರೆ( p->flags & (MEM_Int|MEM_IntReal) ){
#ಆದರೆ GCC_ಆವೃತ್ತಿ>=7000000 && GCC_ಆವೃತ್ತಿ<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    ಪ್ರತಿಜ್ಞಾ( (sqlite3Config.bSmallMalloc!=0xee)*8==ಗಾತ್ರ(x) );
    ಸ್ಮರಣೆ_ನಕಲು(&x, (ಅಕ್ಷರ*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3_Int64_ಗಮ್ಯ_ಪಠ್ಯ(x, zBuf);
#ಇಲ್ಲದಿದ್ದರೆ
    p->n = sqlite3_Int64_ಗಮ್ಯ_ಪಠ್ಯ(p->u.i, zBuf);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p->flags & MEM_IntReal ){
      ಸ್ಮರಣೆ_ನಕಲು(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    ಪ್ರತಿಜ್ಞಾ( acc.z_ಪಠ್ಯ==zBuf && acc.mxAlloc<=0 );
    zBuf[acc.nChar] = 0; /* Fast version of sqlite3StrAccumFinish(&acc) */
    p->n = acc.nChar;
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  ಅಕ್ಷರ zBuf[100];
  ಅಕ್ಷರ *z;
  ಪೂರ್ಣಾಂಕ i, j, incr;
  ಆದರೆ( (p->flags & MEM_Str)==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( p->db && p->db->mallocFailed ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( p->flags & MEM_ಪದ ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    ಆದರೆ( p->szMalloc>0 && p->z==p->zMalloc ){
      ಪ್ರತಿಜ್ಞಾ( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      ಪ್ರತಿಜ್ಞಾ( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    ಪ್ರತಿಜ್ಞಾ( p->z[p->n]==0 );
    ಪ್ರತಿಜ್ಞಾ( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    ಪ್ರತಿಜ್ಞಾ( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  ಆದರೆ( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( p->db==0 ){
    ಹಿಂತಿರುಗಿ 1;  /* db->nFpDigit required to validate p->z[] */
  }
  ಸ್ಮರಣೆ_ನಕಲು(&tmp, p, ಗಾತ್ರ(tmp));
  vdbe_Mem_Render_ಅಂಕೆ(ಗಾತ್ರ(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  ಆದರೆ( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    ಆದರೆ( p->enc==SQLITE_UTF16BE ) z++;
  }
  ಆಗ( zBuf[j] ){
    ಆದರೆ( zBuf[j++]!=z[i] ) ಹಿಂತಿರುಗಿ 0;
    i += incr;
  }
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

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
ಪೂರ್ಣಾಂಕ sqlite3VdbeChangeEncoding(Mem *pMem, ಪೂರ್ಣಾಂಕ desiredEnc){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  ಪೂರ್ಣಾಂಕ rc;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಪ್ರತಿಜ್ಞಾ( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  ಆದರೆ( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಆದರೆ( pMem->enc==desiredEnc ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_UTF16
  ಹಿಂತಿರುಗಿ SQLITE_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  ಪ್ರತಿಜ್ಞಾ(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  ಪ್ರತಿಜ್ಞಾ(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  ಪ್ರತಿಜ್ಞಾ(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Make sure pMem->z points to a writable allocation of at least n bytes.
**
** If the bPreserve argument is true, then copy of the content of
** pMem->z into the new allocation.  pMem must be either a string or
** blob if bPreserve is true.  If bPreserve is false, any prior content
** in pMem->z is discarded.
*/
SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3VdbeMemGrow(Mem *pMem, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ bPreserve){
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(pMem) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  ಪ್ರತಿಜ್ಞಾ( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  ಪ್ರತಿಜ್ಞಾ( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db,pMem->zMalloc));
  ಆದರೆ( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    ಆದರೆ( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      ಆದರೆ( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  ಆದರೆ( pMem->zMalloc==0 ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pMem->szMalloc = sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db, pMem->zMalloc);
  }

  ಆದರೆ( bPreserve && pMem->z ){
    ಪ್ರತಿಜ್ಞಾ( pMem->z!=pMem->zMalloc );
    ಸ್ಮರಣೆ_ನಕಲು(pMem->zMalloc, pMem->z, pMem->n);
  }
  ಆದರೆ( (pMem->flags&MEM_Dyn)!=0 ){
    ಪ್ರತಿಜ್ಞಾ( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((ಶೂನ್ಯ *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemClearAndResize(Mem *pMem, ಪೂರ್ಣಾಂಕ szNew){
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || szNew>0 );
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  ಆದರೆ( pMem->szMalloc<szNew ){
    ಹಿಂತಿರುಗಿ sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  ಆದರೆ( (pMem->flags & (MEM_Str|MEM_ಪದ|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( pMem->enc!=SQLITE_UTF8 ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );
  ಆದರೆ( pMem->flags & MEM_Dyn ){
    ಆದರೆ( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_ಪದ;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_ಪದ;
      ಹಿಂತಿರುಗಿ 1;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_ಪದ;
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbe_Mem_ಕೂಡಿಸು_Terminator(Mem *pMem){
  ಆದರೆ( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_ಪದ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemMakeWriteable(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಆದರೆ( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    ಆದರೆ( ExpandBlob(pMem) ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    ಆದರೆ( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      ಪೂರ್ಣಾಂಕ rc = vdbe_Mem_ಕೂಡಿಸು_Terminator(pMem);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemExpandBlob(Mem *pMem){
  ಪೂರ್ಣಾಂಕ nByte;
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->flags & MEM_Zero );
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_ಮೌಲ್ಯ_nochange(pMem) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  ಆದರೆ( nByte<=0 ){
    ಆದರೆ( (pMem->flags & MEM_Blob)==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
    nByte = 1;
  }
  ಆದರೆ( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_ಪದ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Make sure the given Mem is \u0000 terminated.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemNulTerminate(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_ಪದ|MEM_Str))==(MEM_ಪದ|MEM_Str) );
  testcase( (pMem->flags & (MEM_ಪದ|MEM_Str))==0 );
  ಆದರೆ( (pMem->flags & (MEM_ಪದ|MEM_Str))!=MEM_Str ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;   /* Nothing to do */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ vdbe_Mem_ಕೂಡಿಸು_Terminator(pMem);
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ nByte = 32;

  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( !(pMem->flags&MEM_Zero) );
  ಪ್ರತಿಜ್ಞಾ( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  ಪ್ರತಿಜ್ಞಾ( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );


  ಆದರೆ( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_ಅಂಕೆ(nByte, pMem->z, pMem);
  ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->n==(ಪೂರ್ಣಾಂಕ)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_ಪದ;
  ಆದರೆ( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  ಪ್ರತಿಜ್ಞಾ( pFunc!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db!=0 );
  ಪ್ರತಿಜ್ಞಾ( pFunc->xFinalize!=0 );
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, ಗಾತ್ರ(ctx));
  memset(&t, 0, ಗಾತ್ರ(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.p_ಹೊರಹೋಗು = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Dyn)==0 );
  ಆದರೆ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  ಸ್ಮರಣೆ_ನಕಲು(pMem, &t, ಗಾತ್ರ(t));
  ಹಿಂತಿರುಗಿ ctx.isError;
}

/*
** Memory cell pAccum contains the context of an aggregate function.
** This routine calls the xValue method for that function and stores
** the results in memory cell pMem.
**
** SQLITE_ERROR is returned if xValue() reports an error. SQLITE_OK 
** otherwise.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_Agg_ಮೌಲ್ಯ(Mem *pAccum, Mem *p_ಹೊರಹೋಗು, FuncDef *pFunc){
  sqlite3_context ctx;
  ಪ್ರತಿಜ್ಞಾ( pFunc!=0 );
  ಪ್ರತಿಜ್ಞಾ( pFunc->x_ಮೌಲ್ಯ!=0 );
  ಪ್ರತಿಜ್ಞಾ( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  ಪ್ರತಿಜ್ಞಾ( pAccum->db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, ಗಾತ್ರ(ctx));
  sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  ctx.p_ಹೊರಹೋಗು = p_ಹೊರಹೋಗು;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_ಮೌಲ್ಯ(&ctx);
  ಹಿಂತಿರುಗಿ ctx.isError;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

/*
** If the memory cell contains a value that must be freed by
** invoking the external callback in Mem.xDel, then this routine
** will free that value.  It also sets Mem.flags to MEM_Null.
**
** This is a helper routine for sqlite3VdbeMemSetNull() and
** for sqlite3VdbeMemRelease().  Use those other routines as the
** entry point for releasing Mem resources.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ vdbe_Mem_Clear_Extern_And_ಗಣ_Null(Mem *p){
  ಪ್ರತಿಜ್ಞಾ( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( VdbeMemDynamic(p) );
  ಆದರೆ( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    ಪ್ರತಿಜ್ಞಾ( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  ಆದರೆ( p->flags&MEM_Dyn ){
    ಪ್ರತಿಜ್ಞಾ( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((ಶೂನ್ಯ *)p->z);
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
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ vdbeMemClear(Mem *p){
  ಆದರೆ( VdbeMemDynamic(p) ){
    vdbe_Mem_Clear_Extern_And_ಗಣ_Null(p);
  }
  ಆದರೆ( p->szMalloc ){
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
ಶೂನ್ಯ sqlite3VdbeMemRelease(Mem *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(p) );
  ಆದರೆ( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
ಶೂನ್ಯ sqlite3VdbeMemReleaseMalloc(Mem *p){
  ಪ್ರತಿಜ್ಞಾ( !VdbeMemDynamic(p) );
  ಆದರೆ( p->szMalloc ) vdbeMemClear(p);
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
ಸ್ಥಿರ SQLITE_NOINLINE i64 mem_Int_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ Mem *pMem){
  i64 ಮೌಲ್ಯ = 0;
  sqlite3Atoi64(pMem->z, &ಮೌಲ್ಯ, pMem->n, pMem->enc);
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}
i64 sqlite3_Vdbe_Int_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ Mem *pMem){
  ಪೂರ್ಣಾಂಕ flags;
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  ಆದರೆ( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    ಹಿಂತಿರುಗಿ pMem->u.i;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & MEM_Real ){
    ಹಿಂತಿರುಗಿ sqlite3_Real_ಗಮ್ಯ_I64(pMem->u.r);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    ಹಿಂತಿರುಗಿ mem_Int_ಮೌಲ್ಯ(pMem);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3_Mem_Real_ಮೌಲ್ಯ_RCSlow_ಮಾರ್ಗ(
  Mem *pMem,
  ದ್ವಿಗುಣ *p_ಮೌಲ್ಯ
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  *p_ಮೌಲ್ಯ = 0.0;
  ಆದರೆ( pMem->enc==SQLITE_UTF8 ){
    ಅಕ್ಷರ *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    ಆದರೆ( zCopy ){
      rc = sqlite3AtoF(zCopy, p_ಮೌಲ್ಯ);
      sqlite3DbFree(pMem->db, zCopy);
    }
    ಹಿಂತಿರುಗಿ rc;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ n, i, j;
    ಅಕ್ಷರ *zCopy;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    ಆದರೆ( zCopy ){
      z = pMem->z;
      ಆದರೆ( pMem->enc==SQLITE_UTF16LE ){
        ಗಾಗಿ(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          ಆದರೆ( z[i+1]!=0 ) ನಿಲ್ಲಿಸು;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಗಾಗಿ(i=j=0; i<n-1; i+=2, j++){
          ಆದರೆ( z[i]!=0 ) ನಿಲ್ಲಿಸು;
          zCopy[j] = z[i+1];
        }
      }     
      ಪ್ರತಿಜ್ಞಾ( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_ಮೌಲ್ಯ);
      ಆದರೆ( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_Mem_Real_ಮೌಲ್ಯ_RC(Mem *pMem, ದ್ವಿಗುಣ *p_ಮೌಲ್ಯ){
  testcase( pMem->db==0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->flags & (MEM_Str|MEM_Blob) );
  ಆದರೆ( pMem->z==0 ){
    *p_ಮೌಲ್ಯ = 0.0;
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_ಪದ)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    ಹಿಂತಿರುಗಿ sqlite3AtoF(pMem->z, p_ಮೌಲ್ಯ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->n==0 ){
    *p_ಮೌಲ್ಯ = 0.0;
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ sqlite3_Mem_Real_ಮೌಲ್ಯ_RCSlow_ಮಾರ್ಗ(pMem, p_ಮೌಲ್ಯ);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ದ್ವಿಗುಣ sqlite3_Mem_Real_ಮೌಲ್ಯ_No_RC(Mem *pMem){
  ದ್ವಿಗುಣ r;
  (ಶೂನ್ಯ)sqlite3_Mem_Real_ಮೌಲ್ಯ_RC(pMem, &r);
  ಹಿಂತಿರುಗಿ r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
ದ್ವಿಗುಣ sqlite3_Vdbe_Real_ಮೌಲ್ಯ(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );
  ಆದರೆ( pMem->flags & MEM_Real ){
    ಹಿಂತಿರುಗಿ pMem->u.r;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    ಹಿಂತಿರುಗಿ (ದ್ವಿಗುಣ)pMem->u.i;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->flags & (MEM_Str|MEM_Blob) ){
    ಹಿಂತಿರುಗಿ sqlite3_Mem_Real_ಮೌಲ್ಯ_No_RC(pMem);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    ಹಿಂತಿರುಗಿ (ದ್ವಿಗುಣ)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(Mem *pMem, ಪೂರ್ಣಾಂಕ ifNull){
  testcase( pMem->flags & MEM_IntReal );
  ಆದರೆ( pMem->flags & (MEM_Int|MEM_IntReal) ) ಹಿಂತಿರುಗಿ pMem->u.i!=0;
  ಆದರೆ( pMem->flags & MEM_Null ) ಹಿಂತಿರುಗಿ ifNull;
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_Real_ಮೌಲ್ಯ(pMem)!=0.0;
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
ಶೂನ್ಯ sqlite3VdbeIntegerAffinity(Mem *pMem, ಪೂರ್ಣಾಂಕ b_From_ಪಠ್ಯ){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->flags & (MEM_Real|MEM_IntReal) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );

  ಆದರೆ( pMem->flags & MEM_IntReal ){
    Mem_ಗಣ_Type_Flag(pMem, MEM_Int);
  }ಇಲ್ಲದಿದ್ದರೆ{
    i64 ix = sqlite3_Real_ಗಮ್ಯ_I64(pMem->u.r);

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
    ಆದರೆ( pMem->u.r==ix && ix<ಅತಿದೊಡ್ಡ_INT64
     && (!b_From_ಪಠ್ಯ || ix>ಅತಿಚಿಕ್ಕ_INT64)
    ){
      pMem->u.i = ix;
      Mem_ಗಣ_Type_Flag(pMem, MEM_Int);
    }
  }
}

/*
** Convert pMem to type integer.  Invalidate any prior representations.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemIntegerify(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pMem);
  Mem_ಗಣ_Type_Flag(pMem, MEM_Int);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemRealify(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_ಮೌಲ್ಯ(pMem);
  Mem_ಗಣ_Type_Flag(pMem, MEM_Real);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3RealSameAsInt(ದ್ವಿಗುಣ r1, sqlite3_int64 i){
  ದ್ವಿಗುಣ r2 = (ದ್ವಿಗುಣ)i;
  ಹಿಂತಿರುಗಿ r1==0.0
      || (memcmp(&r1, &r2, ಗಾತ್ರ(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3_Real_ಗಮ್ಯ_I64(ದ್ವಿಗುಣ r){
  ಆದರೆ( r<-9223372036854774784.0 ) ಹಿಂತಿರುಗಿ ಅತಿಚಿಕ್ಕ_INT64;
  ಆದರೆ( r>+9223372036854774784.0 ) ಹಿಂತಿರುಗಿ ಅತಿದೊಡ್ಡ_INT64;
  ಹಿಂತಿರುಗಿ (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemNumerify(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  ಆದರೆ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    ಪೂರ್ಣಾಂಕ rc;
    sqlite3_int64 ix;
    ಪ್ರತಿಜ್ಞಾ( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_ಮೌಲ್ಯ_RC(pMem, &pMem->u.r);
    ಆದರೆ( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3_Real_ಗಮ್ಯ_I64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      Mem_ಗಣ_Type_Flag(pMem, MEM_Int);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Mem_ಗಣ_Type_Flag(pMem, MEM_Real);
    }
  }
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  ಆದರೆ( pMem->flags & MEM_Null ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಆಯ್ಕೆ( aff ){
    ಸ್ಥಿತಿ SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      ಆದರೆ( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pMem, SQLITE_AFF_ಪಠ್ಯ, encoding);
        ಪ್ರತಿಜ್ಞಾ( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        ಆದರೆ( pMem->flags & MEM_Str ) Mem_ಗಣ_Type_Flag(pMem, MEM_Blob);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pMem->flags &= ~(MEM_Type_ಮುಖವಾಡ&~MEM_Blob);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪೂರ್ಣಾಂಕ rc;
      ಪ್ರತಿಜ್ಞಾ( aff==SQLITE_AFF_ಪಠ್ಯ );
      ಪ್ರತಿಜ್ಞಾ( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pMem, SQLITE_AFF_ಪಠ್ಯ, encoding);
      ಪ್ರತಿಜ್ಞಾ( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      ಆದರೆ( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
ಶೂನ್ಯ sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  ಪ್ರತಿಜ್ಞಾ( (flags & ~MEM_Type_ಮುಖವಾಡ)==0 );
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
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Null(Mem *pMem){
  ಆದರೆ( VdbeMemDynamic(pMem) ){
    vdbe_Mem_Clear_Extern_And_ಗಣ_Null(pMem);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pMem->flags = MEM_Null;
  }
}
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_ಗಣ_Null(sqlite3_ಮೌಲ್ಯ *p){
  sqlite3_Vdbe_Mem_ಗಣ_Null((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(Mem *pMem, ಪೂರ್ಣಾಂಕ n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  ಆದರೆ( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(Mem *pMem, ಪೂರ್ಣಾಂಕ n){
  ಪೂರ್ಣಾಂಕ nByte = n>0?n:1;
  ಆದರೆ( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ vdbe_Release_And_ಗಣ_Int64(Mem *pMem, i64 val){
  sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Int64(Mem *pMem, i64 val){
  ಆದರೆ( VdbeMemDynamic(pMem) ){
    vdbe_Release_And_ಗಣ_Int64(pMem, val);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
ಶೂನ್ಯ sqlite3_Mem_ಗಣ_Array_Int64(sqlite3_ಮೌಲ್ಯ *aMem, ಪೂರ್ಣಾಂಕ iIdx, i64 val){
  sqlite3_Vdbe_Mem_ಗಣ_Int64(&aMem[iIdx], val);
}

/* A no-op destructor */
ಶೂನ್ಯ sqlite3NoopDestructor(ಶೂನ್ಯ *p){ ಬಳಸದ_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_ಸೂಚಕ(
  Mem *pMem,
  ಶೂನ್ಯ *pPtr,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPType,
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)
){
  ಪ್ರತಿಜ್ಞಾ( pMem->flags==MEM_Null );
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = MEM_Null|MEM_Dyn|MEM_Subtype|MEM_ಪದ;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type REAL.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Double(Mem *pMem, ದ್ವಿಗುಣ val){
  sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
  ಆದರೆ( !sqlite3IsNaN(val) ){
    pMem->u.r = val;
    pMem->flags = MEM_Real;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Return true if the Mem holds a RowSet object.  This routine is intended
** for use inside of assert() statements.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(ಸ್ಥಿರಾಂಕ Mem *pMem){
  ಹಿಂತಿರುಗಿ (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
         && pMem->xDel==sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Delete;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Delete any previous value and set the value of pMem to be an
** empty boolean index.
**
** Return SQLITE_OK on success and SQLITE_NOMEM if a memory allocation
** error occurs.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_ಅಡ್ಡಸಾಲು_ಗಣ(Mem *pMem){
  sqlite3 *db = pMem->db;
  ಅಡ್ಡಸಾಲು_ಗಣ *p;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Init(db);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  pMem->z = (ಅಕ್ಷರ*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Delete;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemTooBig(Mem *p){
  ಪ್ರತಿಜ್ಞಾ( p->db!=0 );
  ಆದರೆ( p->flags & (MEM_Str|MEM_Blob) ){
    ಪೂರ್ಣಾಂಕ n = p->n;
    ಆದರೆ( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    ಹಿಂತಿರುಗಿ n>p->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ];
  }
  ಹಿಂತಿರುಗಿ 0; 
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** This routine prepares a memory cell for modification by breaking
** its link to a shallow copy and by marking any current shallow
** copies of this cell as invalid.
**
** This is used for testing and debugging only - to help ensure that shallow
** copies (created by OP_SCopy) are not misused.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Mem_About_ಗಮ್ಯ_Change(Vdbe *pVdbe, Mem *pMem){
  ಪೂರ್ಣಾಂಕ i;
  Mem *pX;
  ಆದರೆ( pMem->bScopy ){
    ಗಾಗಿ(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      ಆದರೆ( pX->pScopyFrom==pMem ){
        u16 mFlags;
        ಆದರೆ( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (ಪೂರ್ಣಾಂಕ)(pX - pVdbe->aMem), (ಪೂರ್ಣಾಂಕ)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        ಪ್ರತಿಜ್ಞಾ( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

/*
** Make an shallow copy of pFrom into pTo.  Prior contents of
** pTo are freed.  The pFrom->z field is not duplicated.  If
** pFrom->z is used, then pTo->z points to the same thing as pFrom->z
** and flags gets srcType (either MEM_Ephem or MEM_Static).
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ vdbeClrCopy(Mem *p_ಗಮ್ಯ, ಸ್ಥಿರಾಂಕ Mem *pFrom, ಪೂರ್ಣಾಂಕ eType){
  vdbe_Mem_Clear_Extern_And_ಗಣ_Null(p_ಗಮ್ಯ);
  ಪ್ರತಿಜ್ಞಾ( !VdbeMemDynamic(p_ಗಮ್ಯ) );
  sqlite3VdbeMemShallowCopy(p_ಗಮ್ಯ, pFrom, eType);
}
ಶೂನ್ಯ sqlite3VdbeMemShallowCopy(Mem *p_ಗಮ್ಯ, ಸ್ಥಿರಾಂಕ Mem *pFrom, ಪೂರ್ಣಾಂಕ srcType){
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pFrom) );
  ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ->db==pFrom->db );
  ಆದರೆ( VdbeMemDynamic(p_ಗಮ್ಯ) ){ vdbeClrCopy(p_ಗಮ್ಯ,pFrom,srcType); ಹಿಂತಿರುಗಿ; }
  ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ, pFrom, MEMCELLSIZE);
  ಆದರೆ( (pFrom->flags&MEM_Static)==0 ){
    p_ಗಮ್ಯ->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    ಪ್ರತಿಜ್ಞಾ( srcType==MEM_Ephem || srcType==MEM_Static );
    p_ಗಮ್ಯ->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemCopy(Mem *p_ಗಮ್ಯ, ಸ್ಥಿರಾಂಕ Mem *pFrom){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pFrom) );
  ಆದರೆ( VdbeMemDynamic(p_ಗಮ್ಯ) ) vdbe_Mem_Clear_Extern_And_ಗಣ_Null(p_ಗಮ್ಯ);
  ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ, pFrom, MEMCELLSIZE);
  p_ಗಮ್ಯ->flags &= ~MEM_Dyn;
  ಆದರೆ( p_ಗಮ್ಯ->flags&(MEM_Str|MEM_Blob) ){
    ಆದರೆ( 0==(pFrom->flags&MEM_Static) ){
      p_ಗಮ್ಯ->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(p_ಗಮ್ಯ);
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
ಶೂನ್ಯ sqlite3VdbeMemMove(Mem *p_ಗಮ್ಯ, Mem *pFrom){
  ಪ್ರತಿಜ್ಞಾ( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ->db==0 || sqlite3_mutex_held(p_ಗಮ್ಯ->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pFrom->db==0 || p_ಗಮ್ಯ->db==0 || pFrom->db==p_ಗಮ್ಯ->db );

  sqlite3VdbeMemRelease(p_ಗಮ್ಯ);
  ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ, pFrom, ಗಾತ್ರ(Mem));
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
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_Str(
  Mem *pMem,          /* Memory cell to set to string value */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  ಪ್ರತಿಜ್ಞಾ( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ಆದರೆ( !z ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  ಆದರೆ( pMem->db ){
    i_ಎಲ್ಲೆ = pMem->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ];
  }ಇಲ್ಲದಿದ್ದರೆ{
    i_ಎಲ್ಲೆ = SQLITE_MAX_ಉದ್ದಳತೆ;
  }
  ಆದರೆ( nByte<0 ){
    ಪ್ರತಿಜ್ಞಾ( enc!=0 );
    ಆದರೆ( enc==SQLITE_UTF8 ){
      nByte = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಗಾಗಿ(nByte=0; nByte<=i_ಎಲ್ಲೆ && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_ಪದ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }ಇಲ್ಲದಿದ್ದರೆ{
    flags = MEM_Str;
  }
  ಆದರೆ( nByte>i_ಎಲ್ಲೆ ){
    ಆದರೆ( xDel && xDel!=SQLITE_TRANSIENT ){
      ಆದರೆ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (ಶೂನ್ಯ*)z);
      }ಇಲ್ಲದಿದ್ದರೆ{
        xDel((ಶೂನ್ಯ*)z);
      }
    }
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
    ಹಿಂತಿರುಗಿ sqlite3_Error_ಗಮ್ಯ_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ಆದರೆ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    ಆದರೆ( flags&MEM_ಪದ ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ಆದರೆ( sqlite3VdbeMemClearAndResize(pMem, (ಪೂರ್ಣಾಂಕ)MAX(nAlloc,32)) ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );
    ಸ್ಮರಣೆ_ನಕಲು(pMem->z, z, nAlloc);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (ಅಕ್ಷರ *)z;
    ಆದರೆ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db, pMem->zMalloc);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (ಪೂರ್ಣಾಂಕ)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  ಆದರೆ( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ


  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(
  Mem *pMem,          /* Memory cell to set to string value */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  ಪ್ರತಿಜ್ಞಾ( pMem!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ಆದರೆ( !z ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  ಆದರೆ( nByte<0 ){
    nByte = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z);
    flags = MEM_Str|MEM_ಪದ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    flags = MEM_Str;
  }
  ಆದರೆ( nByte>(i64)pMem->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    ಆದರೆ( xDel && xDel!=SQLITE_TRANSIENT ){
      ಆದರೆ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (ಶೂನ್ಯ*)z);
      }ಇಲ್ಲದಿದ್ದರೆ{
        xDel((ಶೂನ್ಯ*)z);
      }
    }
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem);
    ಹಿಂತಿರುಗಿ sqlite3_Error_ಗಮ್ಯ_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ಆದರೆ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ಆದರೆ( sqlite3VdbeMemClearAndResize(pMem, (ಪೂರ್ಣಾಂಕ)MAX(nAlloc,32)) ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );
    ಸ್ಮರಣೆ_ನಕಲು(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (ಅಕ್ಷರ *)z;
    ಆದರೆ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (ಪೂರ್ಣಾಂಕ)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 ಸ್ಥಾನಾಂತರ,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  ಪೂರ್ಣಾಂಕ rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ಹಂಚಿಕೆ_ಗಾತ್ರ-1 );
  testcase( amt==SQLITE_MAX_ಹಂಚಿಕೆ_ಗಾತ್ರ );
  ಆದರೆ( amt>=SQLITE_MAX_ಹಂಚಿಕೆ_ಗಾತ್ರ ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಆದರೆ( (u64)amt + (u64)ಸ್ಥಾನಾಂತರ > (u64)sqlite3_Btree_Max_ದಾಖಲೆ_ಗಾತ್ರ(pCur) ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  ಆದರೆ( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, ಸ್ಥಾನಾಂತರ, amt, pMem->z);
    ಆದರೆ( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (ಪೂರ್ಣಾಂಕ)amt;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 ಲಭ್ಯ = 0;  /* Number of bytes available on the local btree page */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK; /* Return code */

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pCur) );
  ಪ್ರತಿಜ್ಞಾ( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem) );
  pMem->z = (ಅಕ್ಷರ *)sqlite3BtreePayloadFetch(pCur, &ಲಭ್ಯ);
  ಪ್ರತಿಜ್ಞಾ( pMem->z!=0 );

  ಆದರೆ( amt<=ಲಭ್ಯ ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (ಪೂರ್ಣಾಂಕ)amt;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಮೌಲ್ಯ_ಗಮ್ಯ_ಪಠ್ಯ(sqlite3_ಮೌಲ್ಯ* pVal, u8 enc){
  ಪ್ರತಿಜ್ಞಾ( pVal!=0 );
  ಪ್ರತಿಜ್ಞಾ( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pVal) );
  ಪ್ರತಿಜ್ಞಾ( (pVal->flags & (MEM_Null))==0 );
  ಆದರೆ( pVal->flags & (MEM_Blob|MEM_Str) ){
    ಆದರೆ( ExpandBlob(pVal) ) ಹಿಂತಿರುಗಿ 0;
    pVal->flags |= MEM_Str;
    ಆದರೆ( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    ಆದರೆ( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_ಗಮ್ಯ_INT(pVal->z)) ){
      ಪ್ರತಿಜ್ಞಾ( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      ಆದರೆ( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    ಪ್ರತಿಜ್ಞಾ( 0==(1&SQLITE_PTR_ಗಮ್ಯ_INT(pVal->z)) );
  }
  ಪ್ರತಿಜ್ಞಾ(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  ಆದರೆ( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeMemValidStrRep(pVal) );
    ಹಿಂತಿರುಗಿ pVal->z;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(sqlite3_ಮೌಲ್ಯ* pVal, u8 enc){
  ಆದರೆ( !pVal ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pVal) );
  ಆದರೆ( (pVal->flags&(MEM_Str|MEM_ಪದ))==(MEM_Str|MEM_ಪದ) && pVal->enc==enc ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeMemValidStrRep(pVal) );
    ಹಿಂತಿರುಗಿ pVal->z;
  }
  ಆದರೆ( pVal->flags&MEM_Null ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ_ಗಮ್ಯ_ಪಠ್ಯ(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_Is_ದ_Class(ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ *pVal, ಶೂನ್ಯ(*xFree)(ಶೂನ್ಯ*)){
  ಆದರೆ( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_ಮೌಲ್ಯ *sqlite3_ಮೌಲ್ಯ_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, ಗಾತ್ರ(*p));
  ಆದರೆ( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
ರಚನೆ ಮೌಲ್ಯ_New_Stat4_Ctx {
  Parse *pParse;
  ಸೂಚ್ಯಂಕ *pIdx;
  Unpacked_ದಾಖಲೆ **ppRec;
  ಪೂರ್ಣಾಂಕ iVal;
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
ಸ್ಥಿರ sqlite3_ಮೌಲ್ಯ *ಮೌಲ್ಯ_New(sqlite3 *db, ರಚನೆ ಮೌಲ್ಯ_New_Stat4_Ctx *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( p ){
    Unpacked_ದಾಖಲೆ *pRec = p->ppRec[0];

    ಆದರೆ( pRec==0 ){
      ಸೂಚ್ಯಂಕ *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      ಪೂರ್ಣಾಂಕ i;                      /* Counter variable */
      ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pIdx->n_ಸ್ತಂಭ;   /* Number of index columns including rowid */
  
      nByte = ಗಾತ್ರ(Mem) * n_ಲಂಬಸಾಲು + ROUND8(ಗಾತ್ರ(Unpacked_ದಾಖಲೆ));
      pRec = (Unpacked_ದಾಖಲೆ*)sqlite3DbMallocZero(db, nByte);
      ಆದರೆ( pRec ){
        pRec->p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_ದ_ಸೂಚ್ಯಂಕ(p->pParse, pIdx);
        ಆದರೆ( pRec->p_ಕೀಲಿ_Info ){
          ಪ್ರತಿಜ್ಞಾ( pRec->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ==n_ಲಂಬಸಾಲು );
          ಪ್ರತಿಜ್ಞಾ( pRec->p_ಕೀಲಿ_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(ಗಾತ್ರ(Unpacked_ದಾಖಲೆ)));
          ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      ಆದರೆ( pRec==0 ) ಹಿಂತಿರುಗಿ 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->n_ಕ್ಷೇತ್ರ = p->iVal+1;
    sqlite3_Vdbe_Mem_ಗಣ_Null(&pRec->aMem[p->iVal]);
    ಹಿಂತಿರುಗಿ &pRec->aMem[p->iVal];
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(p);
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_ENABLE_STAT4) */
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_New(db);
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ_From_Function(
  sqlite3 *db,                    /* The database connection */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_ಮೌಲ್ಯ **ppVal,          /* Write the new value here */
  ರಚನೆ ಮೌಲ್ಯ_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_ಮೌಲ್ಯ **apVal = 0;      /* Function arguments */
  ಪೂರ್ಣಾಂಕ nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_ಮೌಲ್ಯ *pVal = 0;        /* New value */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList = 0;            /* Function arguments */
  ಪೂರ್ಣಾಂಕ i;                          /* Iterator variable */

  ಪ್ರತಿಜ್ಞಾ( pCtx!=0 );
  ಪ್ರತಿಜ್ಞಾ( (p->flags & EP_TokenOnly)==0 );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
  pList = p->x.pList;
  ಆದರೆ( pList ) nVal = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) );
  pFunc = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, p->u.zToken, nVal, enc, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ಆದರೆ( pFunc==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( pFunc );
  ಆದರೆ( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  ಆದರೆ( pList ){
    apVal = (sqlite3_ಮೌಲ್ಯ**)sqlite3DbMallocZero(db, ಗಾತ್ರ(apVal[0]) * nVal);
    ಆದರೆ( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      ಹೋಗು ಮೌಲ್ಯ_from_function_ಹೊರಹೋಗು;
    }
    ಗಾಗಿ(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(pCtx->pParse, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, aff,
                                     &apVal[i]);
      ಆದರೆ( apVal[i]==0 || rc!=SQLITE_OK ) ಹೋಗು ಮೌಲ್ಯ_from_function_ಹೊರಹೋಗು;
    }
  }

  pVal = ಮೌಲ್ಯ_New(db, pCtx);
  ಆದರೆ( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    ಹೋಗು ಮೌಲ್ಯ_from_function_ಹೊರಹೋಗು;
  }

  memset(&ctx, 0, ಗಾತ್ರ(ctx));
  ctx.p_ಹೊರಹೋಗು = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  ಆದರೆ( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_ಸಂದೇಶ(pCtx->pParse, "%s", sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pVal));
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    ಆದರೆ( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 ಮೌಲ್ಯ_from_function_ಹೊರಹೋಗು:
  ಆದರೆ( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  ಆದರೆ( apVal ){
    ಗಾಗಿ(i=0; i<nVal; i++){
      sqlite3_ಮೌಲ್ಯ_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  ಹಿಂತಿರುಗಿ rc;
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಮೌಲ್ಯ_From_Function(a,b,c,d,e,f) SQLITE_OK
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_ENABLE_STAT4) */

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(
  sqlite3 *db,                    /* The database connection */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_ಮೌಲ್ಯ **ppVal,          /* Write the new value here */
  ರಚನೆ ಮೌಲ್ಯ_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  ಪೂರ್ಣಾಂಕ op;
  ಅಕ್ಷರ *zVal = 0;
  sqlite3_ಮೌಲ್ಯ *pVal = 0;
  ಪೂರ್ಣಾಂಕ negInt = 1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNeg = "";
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಆಗ( (op = p_ಅಭಿವ್ಯಕ್ತಿ->op)==TK_UPLUS || op==TK_ಹರವು ) p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಆದರೆ( op==TK_ರಿಜಿಸ್ಟರ್ ) op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  ಪ್ರತಿಜ್ಞಾ( (p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_TokenOnly)==0 || pCtx==0 );

  ಆದರೆ( op==TK_CAST ){
    u8 aff;
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
    aff = sqlite3AffinityType(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,0);
    rc = ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    ಆದರೆ( *ppVal ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#ಇಲ್ಲದಿದ್ದರೆ
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      ಪ್ರತಿಜ್ಞಾ( (ppVal[0][0].flags & MEM_Zero)==0 );
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_ಮೌಲ್ಯ_Apply_Affinity(*ppVal, affinity, enc);
    }
    ಹಿಂತಿರುಗಿ rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  ಆದರೆ( op==TK_UMINUS ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    ಆದರೆ( (p_ಎಡ->op==TK_INTEGER || p_ಎಡ->op==TK_FLOAT) ){
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಎಡ, EP_Int_ಮೌಲ್ಯ)
       || p_ಎಡ->u.zToken[0]!='0' || (p_ಎಡ->u.zToken[1] & ~0x20)!='X'
      ){
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಎಡ;
        op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  ಆದರೆ( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = ಮೌಲ್ಯ_New(db, pCtx);
    ಆದರೆ( pVal==0 ) ಹೋಗು no_mem;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) ){
      sqlite3_Vdbe_Mem_ಗಣ_Int64(pVal, (i64)p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ*negInt);
    }ಇಲ್ಲದಿದ್ದರೆ{
      i64 iVal;
      ಆದರೆ( op==TK_INTEGER && 0==sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, &iVal) ){
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pVal, iVal*negInt);
      }ಇಲ್ಲದಿದ್ದರೆ{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
        ಆದರೆ( zVal==0 ) ಹೋಗು no_mem;
        sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        ಆದರೆ( op==TK_FLOAT ){
          ಪ್ರತಿಜ್ಞಾ( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_ಪದ) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    ಆದರೆ( affinity==SQLITE_AFF_BLOB ){
      ಆದರೆ( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    ಪ್ರತಿಜ್ಞಾ( (pVal->flags & MEM_IntReal)==0 );
    ಆದರೆ( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    ಆದರೆ( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    ಆದರೆ( SQLITE_OK==ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db,p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      ಆದರೆ( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVal->u.i==ಅತಿಚಿಕ್ಕ_INT64 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
        pVal->u.r = -(ದ್ವಿಗುಣ)ಅತಿಚಿಕ್ಕ_INT64;
#ಇಲ್ಲದಿದ್ದರೆ
        pVal->u.r = ಅತಿದೊಡ್ಡ_INT64;
#ಆದರೆ_ಅಂತ್ಯ
        Mem_ಗಣ_Type_Flag(pVal, MEM_Real);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pVal, affinity, enc);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_NULL ){
    pVal = ಮೌಲ್ಯ_New(db, pCtx);
    ಆದರೆ( pVal==0 ) ಹೋಗು no_mem;
    sqlite3_Vdbe_Mem_ಗಣ_Null(pVal);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_BLOB_LITERAL
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_BLOB ){
    ಪೂರ್ಣಾಂಕ nVal;
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]=='x' || p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]=='X' );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[1]=='\'' );
    pVal = ಮೌಲ್ಯ_New(db, pCtx);
    ಆದರೆ( !pVal ) ಹೋಗು no_mem;
    zVal = &p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    ಪ್ರತಿಜ್ಞಾ( zVal[nVal]=='\'' );
    sqlite3_Vdbe_Mem_ಗಣ_Str(pVal, sqlite3_Hex_ಗಮ್ಯ_Blob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_FUNCTION && pCtx!=0 ){
    rc = ಮೌಲ್ಯ_From_Function(db, p_ಅಭಿವ್ಯಕ್ತಿ, enc, affinity, &pVal, pCtx);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_TRUEFALSE ){
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
    pVal = ಮೌಲ್ಯ_New(db, pCtx);
    ಆದರೆ( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[4]==0;
      sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  ಹಿಂತಿರುಗಿ rc;

no_mem:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  ಪ್ರತಿಜ್ಞಾ( *ppVal==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( pCtx==0 ) sqlite3_ಮೌಲ್ಯ_Free(pVal);
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( pCtx==0 ); sqlite3_ಮೌಲ್ಯ_Free(pVal);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(
  sqlite3 *db,              /* The database connection */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_ಮೌಲ್ಯ **ppVal     /* Write the new value here */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  sqlite3_ಮೌಲ್ಯ *pVal = 0;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    rc = ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, p_ಅಭಿವ್ಯಕ್ತಿ, enc, affinity, &pVal, 0);
    ಆದರೆ( rc==SQLITE_OK && pVal 
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
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stat4_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(
  Parse *pParse,                  /* Parse context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  ರಚನೆ ಮೌಲ್ಯ_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_ಮೌಲ್ಯ **ppVal           /* OUT: New value object (or NULL) */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  sqlite3_ಮೌಲ್ಯ *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಅಭಿವ್ಯಕ್ತಿ);

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ==0 || p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ರಿಜಿಸ್ಟರ್ || p_ಅಭಿವ್ಯಕ್ತಿ->op2!=TK_VARIABLE );
  ಆದರೆ( !p_ಅಭಿವ್ಯಕ್ತಿ ){
    pVal = ಮೌಲ್ಯ_New(db, pAlloc);
    ಆದರೆ( pVal ){
      sqlite3_Vdbe_Mem_ಗಣ_Null((Mem*)pVal);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    ಪೂರ್ಣಾಂಕ iBindVar = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    ಆದರೆ( (v = pParse->pReprepare)!=0 ){
      pVal = ಮೌಲ್ಯ_New(db, pAlloc);
      ಆದರೆ( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, p_ಅಭಿವ್ಯಕ್ತಿ, ENC(db), affinity, &pVal, pAlloc);
  }

  ಪ್ರತಿಜ್ಞಾ( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_Stat4_Probe_ಗಣ_ಮೌಲ್ಯ(
  Parse *pParse,                  /* Parse context */
  ಸೂಚ್ಯಂಕ *pIdx,                    /* Index being probed */
  Unpacked_ದಾಖಲೆ **ppRec,         /* IN/OUT: Probe record */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                    /* The expression to extract a value from */
  ಪೂರ್ಣಾಂಕ nElem,                      /* Maximum number of values to append */
  ಪೂರ್ಣಾಂಕ iVal,                       /* Array element to populate */
  ಪೂರ್ಣಾಂಕ *pn_ಹೊರತೆಗೆ                  /* OUT: Values appended to the record */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ n_ಹೊರತೆಗೆ = 0;

  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 || p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_SELECT ){
    ಪೂರ್ಣಾಂಕ i;
    ರಚನೆ ಮೌಲ್ಯ_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    ಗಾಗಿ(i=0; i<nElem; i++){
      sqlite3_ಮೌಲ್ಯ *pVal = 0;
      ಅಭಿವ್ಯಕ್ತಿ *pElem = (p_ಅಭಿವ್ಯಕ್ತಿ ? sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಅಭಿವ್ಯಕ್ತಿ, i) : 0);
      u8 aff = sqlite3_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Affinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(pParse, pElem, aff, &alloc, &pVal);
      ಆದರೆ( !pVal ) ನಿಲ್ಲಿಸು;
      n_ಹೊರತೆಗೆ++;
    }
  }

  *pn_ಹೊರತೆಗೆ = n_ಹೊರತೆಗೆ;
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_Stat4_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(
  Parse *pParse,                  /* Parse context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_ಮೌಲ್ಯ **ppVal           /* OUT: New value object (or NULL) */
){
  ಹಿಂತಿರುಗಿ stat4_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, affinity, 0, ppVal);
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
ಪೂರ್ಣಾಂಕ sqlite3_Stat4_ಸ್ತಂಭ(
  sqlite3 *db,                    /* Database handle */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pRec,               /* Pointer to buffer containing record */
  ಪೂರ್ಣಾಂಕ nRec,                       /* Size of buffer pRec in bytes */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,                       /* Column to extract */
  sqlite3_ಮೌಲ್ಯ **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 i_ಕ್ಷೇತ್ರ;                     /* Next unread data byte */
  u32 sz_ಕ್ಷೇತ್ರ = 0;                /* Size of the current data field */
  ಪೂರ್ಣಾಂಕ i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>0 );
  iHdr = getVarint32(a, nHdr);
  ಆದರೆ( nHdr>(u32)nRec || iHdr>=nHdr ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  i_ಕ್ಷೇತ್ರ = nHdr;
  ಗಾಗಿ(i=0; i<=i_ಲಂಬಸಾಲು; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    ಆದರೆ( iHdr>nHdr ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    sz_ಕ್ಷೇತ್ರ = sqlite3VdbeSerialTypeLen(t);
    i_ಕ್ಷೇತ್ರ += sz_ಕ್ಷೇತ್ರ;
  }
  testcase( i_ಕ್ಷೇತ್ರ==nRec );
  testcase( i_ಕ್ಷೇತ್ರ==nRec+1 );
  ಆದರೆ( i_ಕ್ಷೇತ್ರ>nRec ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  ಆದರೆ( pMem==0 ){
    pMem = *ppVal = sqlite3_ಮೌಲ್ಯ_New(db);
    ಆದರೆ( pMem==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[i_ಕ್ಷೇತ್ರ-sz_ಕ್ಷೇತ್ರ], t, pMem);
  pMem->enc = ENC(db);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
ಶೂನ್ಯ sqlite3Stat4ProbeFree(Unpacked_ದಾಖಲೆ *pRec){
  ಆದರೆ( pRec ){
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pRec->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_ಕೀಲಿ_Info_Unref(pRec->p_ಕೀಲಿ_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(
  sqlite3_ಮೌಲ್ಯ *v,     /* Value to be set */
  ಪೂರ್ಣಾಂಕ n,                /* Length of string z */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)   /* Destructor for the string */
){
  ಆದರೆ( v ) sqlite3_Vdbe_Mem_ಗಣ_Str((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_Free(sqlite3_ಮೌಲ್ಯ *v){
  ಆದರೆ( !v ) ಹಿಂತಿರುಗಿ;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(sqlite3_ಮೌಲ್ಯ *pVal, u8 enc){
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ_ಗಮ್ಯ_ಪಠ್ಯ(pVal, enc)!=0 ? pVal->n : 0;
}
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(sqlite3_ಮೌಲ್ಯ *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  ಪ್ರತಿಜ್ಞಾ( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  ಆದರೆ( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    ಹಿಂತಿರುಗಿ p->n;
  }
  ಆದರೆ( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    ಹಿಂತಿರುಗಿ p->n;
  }
  ಆದರೆ( (p->flags & MEM_Blob)!=0 ){
    ಆದರೆ( p->flags & MEM_Zero ){
      ಹಿಂತಿರುಗಿ p->n + p->u.nZero;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ p->n;
    }
  }
  ಆದರೆ( p->flags & MEM_Null ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(pVal, enc);
}
