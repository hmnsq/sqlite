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
#include "sqliteInt.h"
#include "vdbeInt.h"

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
puur_naa_nka sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  assert( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  assert( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  assert( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  ꦪꦼꦤ꧀( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    assert( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    ꦪꦼꦤ꧀( (p->flags & (MEM_Term|MEM_Subtype))==(MEM_Term|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      assert( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      assert( (p->flags & ~(MEM_Null|MEM_Term|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }ꦭꦶꦪ{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }ꦭꦶꦪ{
    /* The MEM_Cleared bit is only allowed on NULLs */
    assert( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  assert( p->szMalloc==0
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
  ꦪꦼꦤ꧀( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    assert( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  ꦮꦭꦶ 1;
}
#endif

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
static void vdbeMemRenderNum(puur_naa_nka sz, var_na *zBuf, Mem *p){
  StrAccum acc;
  assert( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  assert( sz>22 );
  ꦪꦼꦤ꧀( p->flags & (MEM_Int|MEM_IntReal) ){
#ꦪꦼꦤ꧀ GCC_VERSION>=7000000 && GCC_VERSION<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    assert( (sqlite3Config.bSmallMalloc!=0xee)*8==parimaa_na(x) );
    memcpy(&x, (var_na*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3Int64ToText(x, zBuf);
#ꦭꦶꦪ
    p->n = sqlite3Int64ToText(p->u.i, zBuf);
#endif
    ꦪꦼꦤ꧀( p->flags & MEM_IntReal ){
      memcpy(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }ꦭꦶꦪ{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    assert( acc.zText==zBuf && acc.mxAlloc<=0 );
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
puur_naa_nka sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  var_na zBuf[100];
  var_na *z;
  puur_naa_nka i, j, incr;
  ꦪꦼꦤ꧀( (p->flags & MEM_Str)==0 ) ꦮꦭꦶ 1;
  ꦪꦼꦤ꧀( p->db && p->db->mallocFailed ) ꦮꦭꦶ 1;
  ꦪꦼꦤ꧀( p->flags & MEM_Term ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    ꦪꦼꦤ꧀( p->szMalloc>0 && p->z==p->zMalloc ){
      assert( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      assert( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    assert( p->z[p->n]==0 );
    assert( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    assert( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  ꦪꦼꦤ꧀( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) ꦮꦭꦶ 1;
  ꦪꦼꦤ꧀( p->db==0 ){
    ꦮꦭꦶ 1;  /* db->nFpDigit required to validate p->z[] */
  }
  memcpy(&tmp, p, parimaa_na(tmp));
  vdbeMemRenderNum(parimaa_na(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  ꦪꦼꦤ꧀( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    ꦪꦼꦤ꧀( p->enc==SQLITE_UTF16BE ) z++;
  }
  ꦱꦼꦢꦼꦭ( zBuf[j] ){
    ꦪꦼꦤ꧀( zBuf[j++]!=z[i] ) ꦮꦭꦶ 0;
    i += incr;
  }
  ꦮꦭꦶ 1;
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
puur_naa_nka sqlite3VdbeChangeEncoding(Mem *pMem, puur_naa_nka desiredEnc){
#ifndef SQLITE_OMIT_UTF16
  puur_naa_nka rc;
#endif
  assert( pMem!=0 );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  assert( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  ꦪꦼꦤ꧀( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    ꦮꦭꦶ SQLITE_OK;
  }
  ꦪꦼꦤ꧀( pMem->enc==desiredEnc ){
    ꦮꦭꦶ SQLITE_OK;
  }
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ifdef SQLITE_OMIT_UTF16
  ꦮꦭꦶ SQLITE_ERROR;
#ꦭꦶꦪ

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  assert(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  assert(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  assert(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  ꦮꦭꦶ rc;
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
SQLITE_NOINLINE puur_naa_nka sqlite3VdbeMemGrow(Mem *pMem, puur_naa_nka n, puur_naa_nka bPreserve){
  assert( sqlite3VdbeCheckMemInvariants(pMem) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  assert( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  assert( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3DbMallocSize(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3DbMallocSize(pMem->db,pMem->zMalloc));
  ꦪꦼꦤ꧀( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    ꦪꦼꦤ꧀( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }ꦭꦶꦪ{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      ꦪꦼꦤ꧀( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }ꦭꦶꦪ{
    ꦪꦼꦤ꧀( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  ꦪꦼꦤ꧀( pMem->zMalloc==0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }ꦭꦶꦪ{
    pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
  }

  ꦪꦼꦤ꧀( bPreserve && pMem->z ){
    assert( pMem->z!=pMem->zMalloc );
    memcpy(pMem->zMalloc, pMem->z, pMem->n);
  }
  ꦪꦼꦤ꧀( (pMem->flags&MEM_Dyn)!=0 ){
    assert( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((void *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  ꦮꦭꦶ SQLITE_OK;
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
puur_naa_nka sqlite3VdbeMemClearAndResize(Mem *pMem, puur_naa_nka szNew){
  assert( CORRUPT_DB || szNew>0 );
  assert( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  ꦪꦼꦤ꧀( pMem->szMalloc<szNew ){
    ꦮꦭꦶ sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  assert( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  ꦮꦭꦶ SQLITE_OK;
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
puur_naa_nka sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  ꦪꦼꦤ꧀( (pMem->flags & (MEM_Str|MEM_Term|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    ꦮꦭꦶ 0;
  }
  ꦪꦼꦤ꧀( pMem->enc!=SQLITE_UTF8 ) ꦮꦭꦶ 0;
  assert( pMem->z!=0 );
  ꦪꦼꦤ꧀( pMem->flags & MEM_Dyn ){
    ꦪꦼꦤ꧀( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_Term;
      ꦮꦭꦶ 1;
    }
    ꦪꦼꦤ꧀( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_Term;
      ꦮꦭꦶ 1;
    }
  }ꦭꦶꦪ ꦪꦼꦤ꧀( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_Term;
    ꦮꦭꦶ 1;
  }
  ꦮꦭꦶ 0;
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
static SQLITE_NOINLINE puur_naa_nka vdbeMemAddTerminator(Mem *pMem){
  ꦪꦼꦤ꧀( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_Term;
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
puur_naa_nka sqlite3VdbeMemMakeWriteable(Mem *pMem){
  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  ꦪꦼꦤ꧀( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    ꦪꦼꦤ꧀( ExpandBlob(pMem) ) ꦮꦭꦶ SQLITE_NOMEM;
    ꦪꦼꦤ꧀( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      puur_naa_nka rc = vdbeMemAddTerminator(pMem);
      ꦪꦼꦤ꧀( rc ) ꦮꦭꦶ rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ifdef SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#endif

  ꦮꦭꦶ SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ifndef SQLITE_OMIT_INCRBLOB
puur_naa_nka sqlite3VdbeMemExpandBlob(Mem *pMem){
  puur_naa_nka nByte;
  assert( pMem!=0 );
  assert( pMem->flags & MEM_Zero );
  assert( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_value_nochange(pMem) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  ꦪꦼꦤ꧀( nByte<=0 ){
    ꦪꦼꦤ꧀( (pMem->flags & MEM_Blob)==0 ) ꦮꦭꦶ SQLITE_OK;
    nByte = 1;
  }
  ꦪꦼꦤ꧀( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }
  assert( pMem->z!=0 );
  assert( sqlite3DbMallocSize(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_Term);
  ꦮꦭꦶ SQLITE_OK;
}
#endif

/*
** Make sure the given Mem is \u0000 terminated.
*/
puur_naa_nka sqlite3VdbeMemNulTerminate(Mem *pMem){
  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==(MEM_Term|MEM_Str) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==0 );
  ꦪꦼꦤ꧀( (pMem->flags & (MEM_Term|MEM_Str))!=MEM_Str ){
    ꦮꦭꦶ SQLITE_OK;   /* Nothing to do */
  }ꦭꦶꦪ{
    ꦮꦭꦶ vdbeMemAddTerminator(pMem);
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
puur_naa_nka sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  const puur_naa_nka nByte = 32;

  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( !(pMem->flags&MEM_Zero) );
  assert( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  assert( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );


  ꦪꦼꦤ꧀( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }

  vdbeMemRenderNum(nByte, pMem->z, pMem);
  assert( pMem->z!=0 );
  assert( pMem->n==(puur_naa_nka)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_Term;
  ꦪꦼꦤ꧀( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
puur_naa_nka sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  assert( pFunc!=0 );
  assert( pMem!=0 );
  assert( pMem->db!=0 );
  assert( pFunc->xFinalize!=0 );
  assert( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  assert( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, parimaa_na(ctx));
  memset(&t, 0, parimaa_na(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.pOut = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  assert( (pMem->flags & MEM_Dyn)==0 );
  ꦪꦼꦤ꧀( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  memcpy(pMem, &t, parimaa_na(t));
  ꦮꦭꦶ ctx.isError;
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
puur_naa_nka sqlite3VdbeMemAggValue(Mem *pAccum, Mem *pOut, FuncDef *pFunc){
  sqlite3_context ctx;
  assert( pFunc!=0 );
  assert( pFunc->xValue!=0 );
  assert( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  assert( pAccum->db!=0 );
  assert( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, parimaa_na(ctx));
  sqlite3VdbeMemSetNull(pOut);
  ctx.pOut = pOut;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->xValue(&ctx);
  ꦮꦭꦶ ctx.isError;
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
static SQLITE_NOINLINE void vdbeMemClearExternAndSetNull(Mem *p){
  assert( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  assert( VdbeMemDynamic(p) );
  ꦪꦼꦤ꧀( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    assert( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  ꦪꦼꦤ꧀( p->flags&MEM_Dyn ){
    assert( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((void *)p->z);
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
static SQLITE_NOINLINE void vdbeMemClear(Mem *p){
  ꦪꦼꦤ꧀( VdbeMemDynamic(p) ){
    vdbeMemClearExternAndSetNull(p);
  }
  ꦪꦼꦤ꧀( p->szMalloc ){
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
void sqlite3VdbeMemRelease(Mem *p){
  assert( sqlite3VdbeCheckMemInvariants(p) );
  ꦪꦼꦤ꧀( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
void sqlite3VdbeMemReleaseMalloc(Mem *p){
  assert( !VdbeMemDynamic(p) );
  ꦪꦼꦤ꧀( p->szMalloc ) vdbeMemClear(p);
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
static SQLITE_NOINLINE i64 memIntValue(const Mem *pMem){
  i64 value = 0;
  sqlite3Atoi64(pMem->z, &value, pMem->n, pMem->enc);
  ꦮꦭꦶ value;
}
i64 sqlite3VdbeIntValue(const Mem *pMem){
  puur_naa_nka flags;
  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  ꦪꦼꦤ꧀( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    ꦮꦭꦶ pMem->u.i;
  }ꦭꦶꦪ ꦪꦼꦤ꧀( flags & MEM_Real ){
    ꦮꦭꦶ sqlite3RealToI64(pMem->u.r);
  }ꦭꦶꦪ ꦪꦼꦤ꧀( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    ꦮꦭꦶ memIntValue(pMem);
  }ꦭꦶꦪ{
    ꦮꦭꦶ 0;
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
static SQLITE_NOINLINE puur_naa_nka sqlite3MemRealValueRCSlowPath(
  Mem *pMem,
  dviguu_na *pValue
){
  puur_naa_nka rc = SQLITE_OK;
  *pValue = 0.0;
  ꦪꦼꦤ꧀( pMem->enc==SQLITE_UTF8 ){
    var_na *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    ꦪꦼꦤ꧀( zCopy ){
      rc = sqlite3AtoF(zCopy, pValue);
      sqlite3DbFree(pMem->db, zCopy);
    }
    ꦮꦭꦶ rc;
  }ꦭꦶꦪ{
    puur_naa_nka n, i, j;
    var_na *zCopy;
    const var_na *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    ꦪꦼꦤ꧀( zCopy ){
      z = pMem->z;
      ꦪꦼꦤ꧀( pMem->enc==SQLITE_UTF16LE ){
        ꦏꦁꦒꦺꦴ(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          ꦪꦼꦤ꧀( z[i+1]!=0 ) bha_nga;
        }
      }ꦭꦶꦪ{
        ꦏꦁꦒꦺꦴ(i=j=0; i<n-1; i+=2, j++){
          ꦪꦼꦤ꧀( z[i]!=0 ) bha_nga;
          zCopy[j] = z[i+1];
        }
      }     
      assert( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, pValue);
      ꦪꦼꦤ꧀( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    ꦮꦭꦶ rc;
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
puur_naa_nka sqlite3MemRealValueRC(Mem *pMem, dviguu_na *pValue){
  testcase( pMem->db==0 );
  assert( pMem->flags & (MEM_Str|MEM_Blob) );
  ꦪꦼꦤ꧀( pMem->z==0 ){
    *pValue = 0.0;
    ꦮꦭꦶ 0;
  }ꦭꦶꦪ ꦪꦼꦤ꧀( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_Term)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    ꦮꦭꦶ sqlite3AtoF(pMem->z, pValue);
  }ꦭꦶꦪ ꦪꦼꦤ꧀( pMem->n==0 ){
    *pValue = 0.0;
    ꦮꦭꦶ 0;
  }ꦭꦶꦪ{
    ꦮꦭꦶ sqlite3MemRealValueRCSlowPath(pMem, pValue);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
static SQLITE_NOINLINE dviguu_na sqlite3MemRealValueNoRC(Mem *pMem){
  dviguu_na r;
  (void)sqlite3MemRealValueRC(pMem, &r);
  ꦮꦭꦶ r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
dviguu_na sqlite3VdbeRealValue(Mem *pMem){
  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );
  ꦪꦼꦤ꧀( pMem->flags & MEM_Real ){
    ꦮꦭꦶ pMem->u.r;
  }ꦭꦶꦪ ꦪꦼꦤ꧀( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    ꦮꦭꦶ (dviguu_na)pMem->u.i;
  }ꦭꦶꦪ ꦪꦼꦤ꧀( pMem->flags & (MEM_Str|MEM_Blob) ){
    ꦮꦭꦶ sqlite3MemRealValueNoRC(pMem);
  }ꦭꦶꦪ{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    ꦮꦭꦶ (dviguu_na)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
puur_naa_nka sqlite3VdbeBooleanValue(Mem *pMem, puur_naa_nka ifNull){
  testcase( pMem->flags & MEM_IntReal );
  ꦪꦼꦤ꧀( pMem->flags & (MEM_Int|MEM_IntReal) ) ꦮꦭꦶ pMem->u.i!=0;
  ꦪꦼꦤ꧀( pMem->flags & MEM_Null ) ꦮꦭꦶ ifNull;
  ꦮꦭꦶ sqlite3VdbeRealValue(pMem)!=0.0;
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
void sqlite3VdbeIntegerAffinity(Mem *pMem, puur_naa_nka bFromText){
  assert( pMem!=0 );
  assert( pMem->flags & (MEM_Real|MEM_IntReal) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );

  ꦪꦼꦤ꧀( pMem->flags & MEM_IntReal ){
    MemSetTypeFlag(pMem, MEM_Int);
  }ꦭꦶꦪ{
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
    ꦪꦼꦤ꧀( pMem->u.r==ix && ix<LARGEST_INT64
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
puur_naa_nka sqlite3VdbeMemIntegerify(Mem *pMem){
  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3VdbeIntValue(pMem);
  MemSetTypeFlag(pMem, MEM_Int);
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
puur_naa_nka sqlite3VdbeMemRealify(Mem *pMem){
  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3VdbeRealValue(pMem);
  MemSetTypeFlag(pMem, MEM_Real);
  ꦮꦭꦶ SQLITE_OK;
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
puur_naa_nka sqlite3RealSameAsInt(dviguu_na r1, sqlite3_int64 i){
  dviguu_na r2 = (dviguu_na)i;
  ꦮꦭꦶ r1==0.0
      || (memcmp(&r1, &r2, parimaa_na(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3RealToI64(dviguu_na r){
  ꦪꦼꦤ꧀( r<-9223372036854774784.0 ) ꦮꦭꦶ SMALLEST_INT64;
  ꦪꦼꦤ꧀( r>+9223372036854774784.0 ) ꦮꦭꦶ LARGEST_INT64;
  ꦮꦭꦶ (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
puur_naa_nka sqlite3VdbeMemNumerify(Mem *pMem){
  assert( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  ꦪꦼꦤ꧀( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    puur_naa_nka rc;
    sqlite3_int64 ix;
    assert( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3MemRealValueRC(pMem, &pMem->u.r);
    ꦪꦼꦤ꧀( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3RealToI64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      MemSetTypeFlag(pMem, MEM_Int);
    }ꦭꦶꦪ{
      MemSetTypeFlag(pMem, MEM_Real);
    }
  }
  assert( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
puur_naa_nka sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  ꦪꦼꦤ꧀( pMem->flags & MEM_Null ) ꦮꦭꦶ SQLITE_OK;
  switch( aff ){
    pak_sa SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      ꦪꦼꦤ꧀( (pMem->flags & MEM_Blob)==0 ){
        sqlite3ValueApplyAffinity(pMem, SQLITE_AFF_TEXT, encoding);
        assert( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        ꦪꦼꦤ꧀( pMem->flags & MEM_Str ) MemSetTypeFlag(pMem, MEM_Blob);
      }ꦭꦶꦪ{
        pMem->flags &= ~(MEM_TypeMask&~MEM_Blob);
      }
      bha_nga;
    }
    pak_sa SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      bha_nga;
    }
    pak_sa SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      bha_nga;
    }
    pak_sa SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      bha_nga;
    }
    default: {
      puur_naa_nka rc;
      assert( aff==SQLITE_AFF_TEXT );
      assert( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3ValueApplyAffinity(pMem, SQLITE_AFF_TEXT, encoding);
      assert( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      ꦪꦼꦤ꧀( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      ꦪꦼꦤ꧀( rc ) ꦮꦭꦶ rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
void sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  assert( (flags & ~MEM_TypeMask)==0 );
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
void sqlite3VdbeMemSetNull(Mem *pMem){
  ꦪꦼꦤ꧀( VdbeMemDynamic(pMem) ){
    vdbeMemClearExternAndSetNull(pMem);
  }ꦭꦶꦪ{
    pMem->flags = MEM_Null;
  }
}
void sqlite3ValueSetNull(sqlite3_value *p){
  sqlite3VdbeMemSetNull((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ifndef SQLITE_OMIT_INCRBLOB
void sqlite3VdbeMemSetZeroBlob(Mem *pMem, puur_naa_nka n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  ꦪꦼꦤ꧀( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#ꦭꦶꦪ
puur_naa_nka sqlite3VdbeMemSetZeroBlob(Mem *pMem, puur_naa_nka n){
  puur_naa_nka nByte = n>0?n:1;
  ꦪꦼꦤ꧀( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }
  assert( pMem->z!=0 );
  assert( sqlite3DbMallocSize(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  ꦮꦭꦶ SQLITE_OK;
}
#endif

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
static SQLITE_NOINLINE void vdbeReleaseAndSetInt64(Mem *pMem, i64 val){
  sqlite3VdbeMemSetNull(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
void sqlite3VdbeMemSetInt64(Mem *pMem, i64 val){
  ꦪꦼꦤ꧀( VdbeMemDynamic(pMem) ){
    vdbeReleaseAndSetInt64(pMem, val);
  }ꦭꦶꦪ{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
void sqlite3MemSetArrayInt64(sqlite3_value *aMem, puur_naa_nka iIdx, i64 val){
  sqlite3VdbeMemSetInt64(&aMem[iIdx], val);
}

/* A no-op destructor */
void sqlite3NoopDestructor(void *p){ UNUSED_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
void sqlite3VdbeMemSetPointer(
  Mem *pMem,
  void *pPtr,
  const var_na *zPType,
  void (*xDestructor)(void*)
){
  assert( pMem->flags==MEM_Null );
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
void sqlite3VdbeMemSetDouble(Mem *pMem, dviguu_na val){
  sqlite3VdbeMemSetNull(pMem);
  ꦪꦼꦤ꧀( !sqlite3IsNaN(val) ){
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
puur_naa_nka sqlite3VdbeMemIsRowSet(const Mem *pMem){
  ꦮꦭꦶ (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
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
puur_naa_nka sqlite3VdbeMemSetRowSet(Mem *pMem){
  sqlite3 *db = pMem->db;
  RowSet *p;
  assert( db!=0 );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3RowSetInit(db);
  ꦪꦼꦤ꧀( p==0 ) ꦮꦭꦶ SQLITE_NOMEM;
  pMem->z = (var_na*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3RowSetDelete;
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
puur_naa_nka sqlite3VdbeMemTooBig(Mem *p){
  assert( p->db!=0 );
  ꦪꦼꦤ꧀( p->flags & (MEM_Str|MEM_Blob) ){
    puur_naa_nka n = p->n;
    ꦪꦼꦤ꧀( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    ꦮꦭꦶ n>p->db->aLimit[SQLITE_LIMIT_LENGTH];
  }
  ꦮꦭꦶ 0; 
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
void sqlite3VdbeMemAboutToChange(Vdbe *pVdbe, Mem *pMem){
  puur_naa_nka i;
  Mem *pX;
  ꦪꦼꦤ꧀( pMem->bScopy ){
    ꦏꦁꦒꦺꦴ(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      ꦪꦼꦤ꧀( pX->pScopyFrom==pMem ){
        u16 mFlags;
        ꦪꦼꦤ꧀( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (puur_naa_nka)(pX - pVdbe->aMem), (puur_naa_nka)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        assert( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
static SQLITE_NOINLINE void vdbeClrCopy(Mem *pTo, const Mem *pFrom, puur_naa_nka eType){
  vdbeMemClearExternAndSetNull(pTo);
  assert( !VdbeMemDynamic(pTo) );
  sqlite3VdbeMemShallowCopy(pTo, pFrom, eType);
}
void sqlite3VdbeMemShallowCopy(Mem *pTo, const Mem *pFrom, puur_naa_nka srcType){
  assert( !sqlite3VdbeMemIsRowSet(pFrom) );
  assert( pTo->db==pFrom->db );
  ꦪꦼꦤ꧀( VdbeMemDynamic(pTo) ){ vdbeClrCopy(pTo,pFrom,srcType); ꦮꦭꦶ; }
  memcpy(pTo, pFrom, MEMCELLSIZE);
  ꦪꦼꦤ꧀( (pFrom->flags&MEM_Static)==0 ){
    pTo->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    assert( srcType==MEM_Ephem || srcType==MEM_Static );
    pTo->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
puur_naa_nka sqlite3VdbeMemCopy(Mem *pTo, const Mem *pFrom){
  puur_naa_nka rc = SQLITE_OK;

  assert( !sqlite3VdbeMemIsRowSet(pFrom) );
  ꦪꦼꦤ꧀( VdbeMemDynamic(pTo) ) vdbeMemClearExternAndSetNull(pTo);
  memcpy(pTo, pFrom, MEMCELLSIZE);
  pTo->flags &= ~MEM_Dyn;
  ꦪꦼꦤ꧀( pTo->flags&(MEM_Str|MEM_Blob) ){
    ꦪꦼꦤ꧀( 0==(pFrom->flags&MEM_Static) ){
      pTo->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(pTo);
    }
  }

  ꦮꦭꦶ rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
void sqlite3VdbeMemMove(Mem *pTo, Mem *pFrom){
  assert( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  assert( pTo->db==0 || sqlite3_mutex_held(pTo->db->mutex) );
  assert( pFrom->db==0 || pTo->db==0 || pFrom->db==pTo->db );

  sqlite3VdbeMemRelease(pTo);
  memcpy(pTo, pFrom, parimaa_na(Mem));
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
puur_naa_nka sqlite3VdbeMemSetStr(
  Mem *pMem,          /* Memory cell to set to string value */
  const var_na *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  void (*xDel)(void*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  puur_naa_nka iLimit;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  assert( pMem!=0 );
  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  assert( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ꦪꦼꦤ꧀( !z ){
    sqlite3VdbeMemSetNull(pMem);
    ꦮꦭꦶ SQLITE_OK;
  }

  ꦪꦼꦤ꧀( pMem->db ){
    iLimit = pMem->db->aLimit[SQLITE_LIMIT_LENGTH];
  }ꦭꦶꦪ{
    iLimit = SQLITE_MAX_LENGTH;
  }
  ꦪꦼꦤ꧀( nByte<0 ){
    assert( enc!=0 );
    ꦪꦼꦤ꧀( enc==SQLITE_UTF8 ){
      nByte = strlen(z);
    }ꦭꦶꦪ{
      ꦏꦁꦒꦺꦴ(nByte=0; nByte<=iLimit && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_Term;
  }ꦭꦶꦪ ꦪꦼꦤ꧀( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }ꦭꦶꦪ{
    flags = MEM_Str;
  }
  ꦪꦼꦤ꧀( nByte>iLimit ){
    ꦪꦼꦤ꧀( xDel && xDel!=SQLITE_TRANSIENT ){
      ꦪꦼꦤ꧀( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (void*)z);
      }ꦭꦶꦪ{
        xDel((void*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    ꦮꦭꦶ sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ꦪꦼꦤ꧀( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    ꦪꦼꦤ꧀( flags&MEM_Term ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ꦪꦼꦤ꧀( sqlite3VdbeMemClearAndResize(pMem, (puur_naa_nka)MAX(nAlloc,32)) ){
      ꦮꦭꦶ SQLITE_NOMEM_BKPT;
    }
    assert( pMem->z!=0 );
    memcpy(pMem->z, z, nAlloc);
  }ꦭꦶꦪ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (var_na *)z;
    ꦪꦼꦤ꧀( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
    }ꦭꦶꦪ{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (puur_naa_nka)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ifndef SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  ꦪꦼꦤ꧀( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }
#endif


  ꦮꦭꦶ SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
puur_naa_nka sqlite3VdbeMemSetText(
  Mem *pMem,          /* Memory cell to set to string value */
  const var_na *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  void (*xDel)(void*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  assert( pMem!=0 );
  assert( pMem->db!=0 );
  assert( sqlite3_mutex_held(pMem->db->mutex) );
  assert( !sqlite3VdbeMemIsRowSet(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ꦪꦼꦤ꧀( !z ){
    sqlite3VdbeMemSetNull(pMem);
    ꦮꦭꦶ SQLITE_OK;
  }

  ꦪꦼꦤ꧀( nByte<0 ){
    nByte = strlen(z);
    flags = MEM_Str|MEM_Term;
  }ꦭꦶꦪ{
    flags = MEM_Str;
  }
  ꦪꦼꦤ꧀( nByte>(i64)pMem->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    ꦪꦼꦤ꧀( xDel && xDel!=SQLITE_TRANSIENT ){
      ꦪꦼꦤ꧀( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (void*)z);
      }ꦭꦶꦪ{
        xDel((void*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    ꦮꦭꦶ sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ꦪꦼꦤ꧀( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ꦪꦼꦤ꧀( sqlite3VdbeMemClearAndResize(pMem, (puur_naa_nka)MAX(nAlloc,32)) ){
      ꦮꦭꦶ SQLITE_NOMEM_BKPT;
    }
    assert( pMem->z!=0 );
    memcpy(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }ꦭꦶꦪ{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (var_na *)z;
    ꦪꦼꦤ꧀( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }ꦭꦶꦪ ꦪꦼꦤ꧀( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }ꦭꦶꦪ{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (puur_naa_nka)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  ꦮꦭꦶ SQLITE_OK;
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
puur_naa_nka sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 offset,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  puur_naa_nka rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE-1 );
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE );
  ꦪꦼꦤ꧀( amt>=SQLITE_MAX_ALLOCATION_SIZE ){
    ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }
  ꦪꦼꦤ꧀( (u64)amt + (u64)offset > (u64)sqlite3BtreeMaxRecordSize(pCur) ){
    ꦮꦭꦶ SQLITE_CORRUPT_BKPT;
  }
  ꦪꦼꦤ꧀( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, offset, amt, pMem->z);
    ꦪꦼꦤ꧀( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (puur_naa_nka)amt;
    }ꦭꦶꦪ{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  ꦮꦭꦶ rc;
}
puur_naa_nka sqlite3VdbeMemFromBtreeZeroOffset(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 available = 0;  /* Number of bytes available on the local btree page */
  puur_naa_nka rc = SQLITE_OK; /* Return code */

  assert( sqlite3BtreeCursorIsValid(pCur) );
  assert( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  assert( !sqlite3VdbeMemIsRowSet(pMem) );
  pMem->z = (var_na *)sqlite3BtreePayloadFetch(pCur, &available);
  assert( pMem->z!=0 );

  ꦪꦼꦤ꧀( amt<=available ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (puur_naa_nka)amt;
  }ꦭꦶꦪ{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  ꦮꦭꦶ rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
static SQLITE_NOINLINE const void *valueToText(sqlite3_value* pVal, u8 enc){
  assert( pVal!=0 );
  assert( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  assert( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  assert( !sqlite3VdbeMemIsRowSet(pVal) );
  assert( (pVal->flags & (MEM_Null))==0 );
  ꦪꦼꦤ꧀( pVal->flags & (MEM_Blob|MEM_Str) ){
    ꦪꦼꦤ꧀( ExpandBlob(pVal) ) ꦮꦭꦶ 0;
    pVal->flags |= MEM_Str;
    ꦪꦼꦤ꧀( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    ꦪꦼꦤ꧀( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_TO_INT(pVal->z)) ){
      assert( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      ꦪꦼꦤ꧀( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        ꦮꦭꦶ 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }ꦭꦶꦪ{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    assert( 0==(1&SQLITE_PTR_TO_INT(pVal->z)) );
  }
  assert(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  ꦪꦼꦤ꧀( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    assert( sqlite3VdbeMemValidStrRep(pVal) );
    ꦮꦭꦶ pVal->z;
  }ꦭꦶꦪ{
    ꦮꦭꦶ 0;
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
const void *sqlite3ValueText(sqlite3_value* pVal, u8 enc){
  ꦪꦼꦤ꧀( !pVal ) ꦮꦭꦶ 0;
  assert( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  assert( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  assert( !sqlite3VdbeMemIsRowSet(pVal) );
  ꦪꦼꦤ꧀( (pVal->flags&(MEM_Str|MEM_Term))==(MEM_Str|MEM_Term) && pVal->enc==enc ){
    assert( sqlite3VdbeMemValidStrRep(pVal) );
    ꦮꦭꦶ pVal->z;
  }
  ꦪꦼꦤ꧀( pVal->flags&MEM_Null ){
    ꦮꦭꦶ 0;
  }
  ꦮꦭꦶ valueToText(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
puur_naa_nka sqlite3ValueIsOfClass(const sqlite3_value *pVal, void(*xFree)(void*)){
  ꦪꦼꦤ꧀( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    ꦮꦭꦶ 1;
  }ꦭꦶꦪ{
    ꦮꦭꦶ 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_value *sqlite3ValueNew(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, parimaa_na(*p));
  ꦪꦼꦤ꧀( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  ꦮꦭꦶ p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
sa_nracanaa ValueNewStat4Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  puur_naa_nka iVal;
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
static sqlite3_value *valueNew(sqlite3 *db, sa_nracanaa ValueNewStat4Ctx *p){
#ifdef SQLITE_ENABLE_STAT4
  ꦪꦼꦤ꧀( p ){
    UnpackedRecord *pRec = p->ppRec[0];

    ꦪꦼꦤ꧀( pRec==0 ){
      Index *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      puur_naa_nka i;                      /* Counter variable */
      puur_naa_nka nCol = pIdx->nColumn;   /* Number of index columns including rowid */
  
      nByte = parimaa_na(Mem) * nCol + ROUND8(parimaa_na(UnpackedRecord));
      pRec = (UnpackedRecord*)sqlite3DbMallocZero(db, nByte);
      ꦪꦼꦤ꧀( pRec ){
        pRec->pKeyInfo = sqlite3KeyInfoOfIndex(p->pParse, pIdx);
        ꦪꦼꦤ꧀( pRec->pKeyInfo ){
          assert( pRec->pKeyInfo->nAllField==nCol );
          assert( pRec->pKeyInfo->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(parimaa_na(UnpackedRecord)));
          ꦏꦁꦒꦺꦴ(i=0; i<nCol; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }ꦭꦶꦪ{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      ꦪꦼꦤ꧀( pRec==0 ) ꦮꦭꦶ 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->nField = p->iVal+1;
    sqlite3VdbeMemSetNull(&pRec->aMem[p->iVal]);
    ꦮꦭꦶ &pRec->aMem[p->iVal];
  }
#ꦭꦶꦪ
  UNUSED_PARAMETER(p);
#endif /* defined(SQLITE_ENABLE_STAT4) */
  ꦮꦭꦶ sqlite3ValueNew(db);
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
static puur_naa_nka valueFromFunction(
  sqlite3 *db,                    /* The database connection */
  const Expr *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_value **ppVal,          /* Write the new value here */
  sa_nracanaa ValueNewStat4Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_value **apVal = 0;      /* Function arguments */
  puur_naa_nka nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_value *pVal = 0;        /* New value */
  puur_naa_nka rc = SQLITE_OK;             /* Return code */
  ExprList *pList = 0;            /* Function arguments */
  puur_naa_nka i;                          /* Iterator variable */

  assert( pCtx!=0 );
  assert( (p->flags & EP_TokenOnly)==0 );
  assert( ExprUseXList(p) );
  pList = p->x.pList;
  ꦪꦼꦤ꧀( pList ) nVal = pList->nExpr;
  assert( !ExprHasProperty(p, EP_IntValue) );
  pFunc = sqlite3FindFunction(db, p->u.zToken, nVal, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ꦪꦼꦤ꧀( pFunc==0 ) ꦮꦭꦶ SQLITE_OK;
#endif
  assert( pFunc );
  ꦪꦼꦤ꧀( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    ꦮꦭꦶ SQLITE_OK;
  }

  ꦪꦼꦤ꧀( pList ){
    apVal = (sqlite3_value**)sqlite3DbMallocZero(db, parimaa_na(apVal[0]) * nVal);
    ꦪꦼꦤ꧀( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      goto value_from_function_out;
    }
    ꦏꦁꦒꦺꦴ(i=0; i<nVal; i++){
      rc = sqlite3Stat4ValueFromExpr(pCtx->pParse, pList->a[i].pExpr, aff,
                                     &apVal[i]);
      ꦪꦼꦤ꧀( apVal[i]==0 || rc!=SQLITE_OK ) goto value_from_function_out;
    }
  }

  pVal = valueNew(db, pCtx);
  ꦪꦼꦤ꧀( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    goto value_from_function_out;
  }

  memset(&ctx, 0, parimaa_na(ctx));
  ctx.pOut = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  ꦪꦼꦤ꧀( ctx.isError ){
    rc = ctx.isError;
    sqlite3ErrorMsg(pCtx->pParse, "%s", sqlite3_value_text(pVal));
  }ꦭꦶꦪ{
    sqlite3ValueApplyAffinity(pVal, aff, SQLITE_UTF8);
    assert( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    ꦪꦼꦤ꧀( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 value_from_function_out:
  ꦪꦼꦤ꧀( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  ꦪꦼꦤ꧀( apVal ){
    ꦏꦁꦒꦺꦴ(i=0; i<nVal; i++){
      sqlite3ValueFree(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  ꦮꦭꦶ rc;
}
#ꦭꦶꦪ
# define valueFromFunction(a,b,c,d,e,f) SQLITE_OK
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
static puur_naa_nka valueFromExpr(
  sqlite3 *db,                    /* The database connection */
  const Expr *pExpr,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_value **ppVal,          /* Write the new value here */
  sa_nracanaa ValueNewStat4Ctx *pCtx   /* Second argument for valueNew() */
){
  puur_naa_nka op;
  var_na *zVal = 0;
  sqlite3_value *pVal = 0;
  puur_naa_nka negInt = 1;
  const var_na *zNeg = "";
  puur_naa_nka rc = SQLITE_OK;

  assert( pExpr!=0 );
  ꦱꦼꦢꦼꦭ( (op = pExpr->op)==TK_UPLUS || op==TK_SPAN ) pExpr = pExpr->pLeft;
  ꦪꦼꦤ꧀( op==TK_REGISTER ) op = pExpr->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  assert( (pExpr->flags & EP_TokenOnly)==0 || pCtx==0 );

  ꦪꦼꦤ꧀( op==TK_CAST ){
    u8 aff;
    assert( !ExprHasProperty(pExpr, EP_IntValue) );
    aff = sqlite3AffinityType(pExpr->u.zToken,0);
    rc = valueFromExpr(db, pExpr->pLeft, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    ꦪꦼꦤ꧀( *ppVal ){
#ifdef SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#ꦭꦶꦪ
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      assert( (ppVal[0][0].flags & MEM_Zero)==0 );
#endif
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3ValueApplyAffinity(*ppVal, affinity, enc);
    }
    ꦮꦭꦶ rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  ꦪꦼꦤ꧀( op==TK_UMINUS ){
    Expr *pLeft = pExpr->pLeft;
    ꦪꦼꦤ꧀( (pLeft->op==TK_INTEGER || pLeft->op==TK_FLOAT) ){
      ꦪꦼꦤ꧀( ExprHasProperty(pLeft, EP_IntValue)
       || pLeft->u.zToken[0]!='0' || (pLeft->u.zToken[1] & ~0x20)!='X'
      ){
        pExpr = pLeft;
        op = pExpr->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  ꦪꦼꦤ꧀( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = valueNew(db, pCtx);
    ꦪꦼꦤ꧀( pVal==0 ) goto no_mem;
    ꦪꦼꦤ꧀( ExprHasProperty(pExpr, EP_IntValue) ){
      sqlite3VdbeMemSetInt64(pVal, (i64)pExpr->u.iValue*negInt);
    }ꦭꦶꦪ{
      i64 iVal;
      ꦪꦼꦤ꧀( op==TK_INTEGER && 0==sqlite3DecOrHexToI64(pExpr->u.zToken, &iVal) ){
        sqlite3VdbeMemSetInt64(pVal, iVal*negInt);
      }ꦭꦶꦪ{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, pExpr->u.zToken);
        ꦪꦼꦤ꧀( zVal==0 ) goto no_mem;
        sqlite3ValueSetStr(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        ꦪꦼꦤ꧀( op==TK_FLOAT ){
          assert( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_Term) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    ꦪꦼꦤ꧀( affinity==SQLITE_AFF_BLOB ){
      ꦪꦼꦤ꧀( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3ValueApplyAffinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }ꦭꦶꦪ{
      sqlite3ValueApplyAffinity(pVal, affinity, SQLITE_UTF8);
    }
    assert( (pVal->flags & MEM_IntReal)==0 );
    ꦪꦼꦤ꧀( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    ꦪꦼꦤ꧀( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }ꦭꦶꦪ ꦪꦼꦤ꧀( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    ꦪꦼꦤ꧀( SQLITE_OK==valueFromExpr(db,pExpr->pLeft,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      ꦪꦼꦤ꧀( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }ꦭꦶꦪ ꦪꦼꦤ꧀( pVal->u.i==SMALLEST_INT64 ){
#ifndef SQLITE_OMIT_FLOATING_POINT
        pVal->u.r = -(dviguu_na)SMALLEST_INT64;
#ꦭꦶꦪ
        pVal->u.r = LARGEST_INT64;
#endif
        MemSetTypeFlag(pVal, MEM_Real);
      }ꦭꦶꦪ{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3ValueApplyAffinity(pVal, affinity, enc);
    }
  }ꦭꦶꦪ ꦪꦼꦤ꧀( op==TK_NULL ){
    pVal = valueNew(db, pCtx);
    ꦪꦼꦤ꧀( pVal==0 ) goto no_mem;
    sqlite3VdbeMemSetNull(pVal);
  }
#ifndef SQLITE_OMIT_BLOB_LITERAL
  ꦭꦶꦪ ꦪꦼꦤ꧀( op==TK_BLOB ){
    puur_naa_nka nVal;
    assert( !ExprHasProperty(pExpr, EP_IntValue) );
    assert( pExpr->u.zToken[0]=='x' || pExpr->u.zToken[0]=='X' );
    assert( pExpr->u.zToken[1]=='\'' );
    pVal = valueNew(db, pCtx);
    ꦪꦼꦤ꧀( !pVal ) goto no_mem;
    zVal = &pExpr->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    assert( zVal[nVal]=='\'' );
    sqlite3VdbeMemSetStr(pVal, sqlite3HexToBlob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#endif
#ifdef SQLITE_ENABLE_STAT4
  ꦭꦶꦪ ꦪꦼꦤ꧀( op==TK_FUNCTION && pCtx!=0 ){
    rc = valueFromFunction(db, pExpr, enc, affinity, &pVal, pCtx);
  }
#endif
  ꦭꦶꦪ ꦪꦼꦤ꧀( op==TK_TRUEFALSE ){
    assert( !ExprHasProperty(pExpr, EP_IntValue) );
    pVal = valueNew(db, pCtx);
    ꦪꦼꦤ꧀( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = pExpr->u.zToken[4]==0;
      sqlite3ValueApplyAffinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  ꦮꦭꦶ rc;

no_mem:
#ifdef SQLITE_ENABLE_STAT4
  ꦪꦼꦤ꧀( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#endif
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  assert( *ppVal==0 );
#ifdef SQLITE_ENABLE_STAT4
  ꦪꦼꦤ꧀( pCtx==0 ) sqlite3ValueFree(pVal);
#ꦭꦶꦪ
  assert( pCtx==0 ); sqlite3ValueFree(pVal);
#endif
  ꦮꦭꦶ SQLITE_NOMEM_BKPT;
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
puur_naa_nka sqlite3ValueFromExpr(
  sqlite3 *db,              /* The database connection */
  const Expr *pExpr,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_value **ppVal     /* Write the new value here */
){
  puur_naa_nka rc = SQLITE_OK;
  sqlite3_value *pVal = 0;
  ꦪꦼꦤ꧀( pExpr ){
    rc = valueFromExpr(db, pExpr, enc, affinity, &pVal, 0);
    ꦪꦼꦤ꧀( rc==SQLITE_OK && pVal 
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
  ꦮꦭꦶ rc;
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
static puur_naa_nka stat4ValueFromExpr(
  Parse *pParse,                  /* Parse context */
  Expr *pExpr,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sa_nracanaa ValueNewStat4Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_value **ppVal           /* OUT: New value object (or NULL) */
){
  puur_naa_nka rc = SQLITE_OK;
  sqlite3_value *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  pExpr = sqlite3ExprSkipCollate(pExpr);

  assert( pExpr==0 || pExpr->op!=TK_REGISTER || pExpr->op2!=TK_VARIABLE );
  ꦪꦼꦤ꧀( !pExpr ){
    pVal = valueNew(db, pAlloc);
    ꦪꦼꦤ꧀( pVal ){
      sqlite3VdbeMemSetNull((Mem*)pVal);
    }
  }ꦭꦶꦪ ꦪꦼꦤ꧀( pExpr->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    puur_naa_nka iBindVar = pExpr->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    ꦪꦼꦤ꧀( (v = pParse->pReprepare)!=0 ){
      pVal = valueNew(db, pAlloc);
      ꦪꦼꦤ꧀( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3ValueApplyAffinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }ꦭꦶꦪ{
    rc = valueFromExpr(db, pExpr, ENC(db), affinity, &pVal, pAlloc);
  }

  assert( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  ꦮꦭꦶ rc;
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
puur_naa_nka sqlite3Stat4ProbeSetValue(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Index being probed */
  UnpackedRecord **ppRec,         /* IN/OUT: Probe record */
  Expr *pExpr,                    /* The expression to extract a value from */
  puur_naa_nka nElem,                      /* Maximum number of values to append */
  puur_naa_nka iVal,                       /* Array element to populate */
  puur_naa_nka *pnExtract                  /* OUT: Values appended to the record */
){
  puur_naa_nka rc = SQLITE_OK;
  puur_naa_nka nExtract = 0;

  ꦪꦼꦤ꧀( pExpr==0 || pExpr->op!=TK_SELECT ){
    puur_naa_nka i;
    sa_nracanaa ValueNewStat4Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    ꦏꦁꦒꦺꦴ(i=0; i<nElem; i++){
      sqlite3_value *pVal = 0;
      Expr *pElem = (pExpr ? sqlite3VectorFieldSubexpr(pExpr, i) : 0);
      u8 aff = sqlite3IndexColumnAffinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4ValueFromExpr(pParse, pElem, aff, &alloc, &pVal);
      ꦪꦼꦤ꧀( !pVal ) bha_nga;
      nExtract++;
    }
  }

  *pnExtract = nExtract;
  ꦮꦭꦶ rc;
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
puur_naa_nka sqlite3Stat4ValueFromExpr(
  Parse *pParse,                  /* Parse context */
  Expr *pExpr,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_value **ppVal           /* OUT: New value object (or NULL) */
){
  ꦮꦭꦶ stat4ValueFromExpr(pParse, pExpr, affinity, 0, ppVal);
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
puur_naa_nka sqlite3Stat4Column(
  sqlite3 *db,                    /* Database handle */
  const void *pRec,               /* Pointer to buffer containing record */
  puur_naa_nka nRec,                       /* Size of buffer pRec in bytes */
  puur_naa_nka iCol,                       /* Column to extract */
  sqlite3_value **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 iField;                     /* Next unread data byte */
  u32 szField = 0;                /* Size of the current data field */
  puur_naa_nka i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  assert( iCol>0 );
  iHdr = getVarint32(a, nHdr);
  ꦪꦼꦤ꧀( nHdr>(u32)nRec || iHdr>=nHdr ) ꦮꦭꦶ SQLITE_CORRUPT_BKPT;
  iField = nHdr;
  ꦏꦁꦒꦺꦴ(i=0; i<=iCol; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    ꦪꦼꦤ꧀( iHdr>nHdr ) ꦮꦭꦶ SQLITE_CORRUPT_BKPT;
    szField = sqlite3VdbeSerialTypeLen(t);
    iField += szField;
  }
  testcase( iField==nRec );
  testcase( iField==nRec+1 );
  ꦪꦼꦤ꧀( iField>nRec ) ꦮꦭꦶ SQLITE_CORRUPT_BKPT;
  ꦪꦼꦤ꧀( pMem==0 ){
    pMem = *ppVal = sqlite3ValueNew(db);
    ꦪꦼꦤ꧀( pMem==0 ) ꦮꦭꦶ SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[iField-szField], t, pMem);
  pMem->enc = ENC(db);
  ꦮꦭꦶ SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
void sqlite3Stat4ProbeFree(UnpackedRecord *pRec){
  ꦪꦼꦤ꧀( pRec ){
    puur_naa_nka i;
    puur_naa_nka nCol = pRec->pKeyInfo->nAllField;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    ꦏꦁꦒꦺꦴ(i=0; i<nCol; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3KeyInfoUnref(pRec->pKeyInfo);
    sqlite3DbFreeNN(db, pRec);
  }
}
#endif /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
void sqlite3ValueSetStr(
  sqlite3_value *v,     /* Value to be set */
  puur_naa_nka n,                /* Length of string z */
  const void *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  void (*xDel)(void*)   /* Destructor for the string */
){
  ꦪꦼꦤ꧀( v ) sqlite3VdbeMemSetStr((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
void sqlite3ValueFree(sqlite3_value *v){
  ꦪꦼꦤ꧀( !v ) ꦮꦭꦶ;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
static SQLITE_NOINLINE puur_naa_nka valueBytes(sqlite3_value *pVal, u8 enc){
  ꦮꦭꦶ valueToText(pVal, enc)!=0 ? pVal->n : 0;
}
puur_naa_nka sqlite3ValueBytes(sqlite3_value *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  assert( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  ꦪꦼꦤ꧀( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    ꦮꦭꦶ p->n;
  }
  ꦪꦼꦤ꧀( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    ꦮꦭꦶ p->n;
  }
  ꦪꦼꦤ꧀( (p->flags & MEM_Blob)!=0 ){
    ꦪꦼꦤ꧀( p->flags & MEM_Zero ){
      ꦮꦭꦶ p->n + p->u.nZero;
    }ꦭꦶꦪ{
      ꦮꦭꦶ p->n;
    }
  }
  ꦪꦼꦤ꧀( p->flags & MEM_Null ) ꦮꦭꦶ 0;
  ꦮꦭꦶ valueBytes(pVal, enc);
}
