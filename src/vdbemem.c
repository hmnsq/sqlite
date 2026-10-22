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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"

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
पूर्णाङ्क sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  प्रतिज्ञा( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  प्रतिज्ञा( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  प्रतिज्ञा( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  यदि( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    प्रतिज्ञा( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    यदि( (p->flags & (MEM_Term|MEM_Subtype))==(MEM_Term|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      प्रतिज्ञा( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      प्रतिज्ञा( (p->flags & ~(MEM_Null|MEM_Term|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }अन्यथा{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }अन्यथा{
    /* The MEM_Cleared bit is only allowed on NULLs */
    प्रतिज्ञा( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  प्रतिज्ञा( p->szMalloc==0
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
  यदि( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    प्रतिज्ञा( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  प्रत्यागम 1;
}
#endif

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
स्थिर शून्य vdbe_Mem_Render_अंक(पूर्णाङ्क sz, वर्ण *zBuf, Mem *p){
  StrAccum acc;
  प्रतिज्ञा( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  प्रतिज्ञा( sz>22 );
  यदि( p->flags & (MEM_Int|MEM_IntReal) ){
#यदि GCC_VERSION>=7000000 && GCC_VERSION<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    प्रतिज्ञा( (sqlite3Config.bSmallMalloc!=0xee)*8==परिमाण(x) );
    memcpy(&x, (वर्ण*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3Int64ToText(x, zBuf);
#अन्यथा
    p->n = sqlite3Int64ToText(p->u.i, zBuf);
#endif
    यदि( p->flags & MEM_IntReal ){
      memcpy(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }अन्यथा{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    प्रतिज्ञा( acc.zText==zBuf && acc.mxAlloc<=0 );
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
पूर्णाङ्क sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  वर्ण zBuf[100];
  वर्ण *z;
  पूर्णाङ्क i, j, incr;
  यदि( (p->flags & MEM_Str)==0 ) प्रत्यागम 1;
  यदि( p->db && p->db->mallocFailed ) प्रत्यागम 1;
  यदि( p->flags & MEM_Term ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    यदि( p->szMalloc>0 && p->z==p->zMalloc ){
      प्रतिज्ञा( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      प्रतिज्ञा( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    प्रतिज्ञा( p->z[p->n]==0 );
    प्रतिज्ञा( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    प्रतिज्ञा( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  यदि( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) प्रत्यागम 1;
  यदि( p->db==0 ){
    प्रत्यागम 1;  /* db->nFpDigit required to validate p->z[] */
  }
  memcpy(&tmp, p, परिमाण(tmp));
  vdbe_Mem_Render_अंक(परिमाण(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  यदि( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    यदि( p->enc==SQLITE_UTF16BE ) z++;
  }
  यावत्( zBuf[j] ){
    यदि( zBuf[j++]!=z[i] ) प्रत्यागम 0;
    i += incr;
  }
  प्रत्यागम 1;
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
पूर्णाङ्क sqlite3VdbeChangeEncoding(Mem *pMem, पूर्णाङ्क desiredEnc){
#ifndef SQLITE_OMIT_UTF16
  पूर्णाङ्क rc;
#endif
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  प्रतिज्ञा( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  यदि( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    प्रत्यागम SQLITE_OK;
  }
  यदि( pMem->enc==desiredEnc ){
    प्रत्यागम SQLITE_OK;
  }
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#ifdef SQLITE_OMIT_UTF16
  प्रत्यागम SQLITE_ERROR;
#अन्यथा

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  प्रतिज्ञा(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  प्रतिज्ञा(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  प्रतिज्ञा(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  प्रत्यागम rc;
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
SQLITE_NOINLINE पूर्णाङ्क sqlite3VdbeMemGrow(Mem *pMem, पूर्णाङ्क n, पूर्णाङ्क bPreserve){
  प्रतिज्ञा( sqlite3VdbeCheckMemInvariants(pMem) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  प्रतिज्ञा( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  प्रतिज्ञा( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3DbMallocSize(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3DbMallocSize(pMem->db,pMem->zMalloc));
  यदि( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    यदि( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }अन्यथा{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      यदि( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }अन्यथा{
    यदि( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  यदि( pMem->zMalloc==0 ){
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }अन्यथा{
    pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
  }

  यदि( bPreserve && pMem->z ){
    प्रतिज्ञा( pMem->z!=pMem->zMalloc );
    memcpy(pMem->zMalloc, pMem->z, pMem->n);
  }
  यदि( (pMem->flags&MEM_Dyn)!=0 ){
    प्रतिज्ञा( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((शून्य *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  प्रत्यागम SQLITE_OK;
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
पूर्णाङ्क sqlite3VdbeMemClearAndResize(Mem *pMem, पूर्णाङ्क szNew){
  प्रतिज्ञा( CORRUPT_DB || szNew>0 );
  प्रतिज्ञा( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  यदि( pMem->szMalloc<szNew ){
    प्रत्यागम sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  प्रतिज्ञा( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  प्रत्यागम SQLITE_OK;
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
पूर्णाङ्क sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  यदि( (pMem->flags & (MEM_Str|MEM_Term|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    प्रत्यागम 0;
  }
  यदि( pMem->enc!=SQLITE_UTF8 ) प्रत्यागम 0;
  प्रतिज्ञा( pMem->z!=0 );
  यदि( pMem->flags & MEM_Dyn ){
    यदि( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_Term;
      प्रत्यागम 1;
    }
    यदि( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_Term;
      प्रत्यागम 1;
    }
  }अन्यथा यदि( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_Term;
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क vdbeMemAddTerminator(Mem *pMem){
  यदि( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_Term;
  प्रत्यागम SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
पूर्णाङ्क sqlite3VdbeMemMakeWriteable(Mem *pMem){
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  यदि( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    यदि( ExpandBlob(pMem) ) प्रत्यागम SQLITE_NOMEM;
    यदि( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      पूर्णाङ्क rc = vdbeMemAddTerminator(pMem);
      यदि( rc ) प्रत्यागम rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#ifdef SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#endif

  प्रत्यागम SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#ifndef SQLITE_OMIT_INCRBLOB
पूर्णाङ्क sqlite3VdbeMemExpandBlob(Mem *pMem){
  पूर्णाङ्क nByte;
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->flags & MEM_Zero );
  प्रतिज्ञा( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_मूल्य_nochange(pMem) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  यदि( nByte<=0 ){
    यदि( (pMem->flags & MEM_Blob)==0 ) प्रत्यागम SQLITE_OK;
    nByte = 1;
  }
  यदि( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  प्रतिज्ञा( pMem->z!=0 );
  प्रतिज्ञा( sqlite3DbMallocSize(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_Term);
  प्रत्यागम SQLITE_OK;
}
#endif

/*
** Make sure the given Mem is \u0000 terminated.
*/
पूर्णाङ्क sqlite3VdbeMemNulTerminate(Mem *pMem){
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==(MEM_Term|MEM_Str) );
  testcase( (pMem->flags & (MEM_Term|MEM_Str))==0 );
  यदि( (pMem->flags & (MEM_Term|MEM_Str))!=MEM_Str ){
    प्रत्यागम SQLITE_OK;   /* Nothing to do */
  }अन्यथा{
    प्रत्यागम vdbeMemAddTerminator(pMem);
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
पूर्णाङ्क sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  ध्रुव पूर्णाङ्क nByte = 32;

  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( !(pMem->flags&MEM_Zero) );
  प्रतिज्ञा( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  प्रतिज्ञा( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pMem) );


  यदि( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_अंक(nByte, pMem->z, pMem);
  प्रतिज्ञा( pMem->z!=0 );
  प्रतिज्ञा( pMem->n==(पूर्णाङ्क)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_Term;
  यदि( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  प्रत्यागम SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
पूर्णाङ्क sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  प्रतिज्ञा( pFunc!=0 );
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db!=0 );
  प्रतिज्ञा( pFunc->xFinalize!=0 );
  प्रतिज्ञा( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  प्रतिज्ञा( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, परिमाण(ctx));
  memset(&t, 0, परिमाण(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.pOut = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  प्रतिज्ञा( (pMem->flags & MEM_Dyn)==0 );
  यदि( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  memcpy(pMem, &t, परिमाण(t));
  प्रत्यागम ctx.isError;
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
पूर्णाङ्क sqlite3_Vdbe_Mem_Agg_मूल्य(Mem *pAccum, Mem *pOut, FuncDef *pFunc){
  sqlite3_context ctx;
  प्रतिज्ञा( pFunc!=0 );
  प्रतिज्ञा( pFunc->x_मूल्य!=0 );
  प्रतिज्ञा( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  प्रतिज्ञा( pAccum->db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, परिमाण(ctx));
  sqlite3VdbeMemSetNull(pOut);
  ctx.pOut = pOut;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_मूल्य(&ctx);
  प्रत्यागम ctx.isError;
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
स्थिर SQLITE_NOINLINE शून्य vdbeMemClearExternAndSetNull(Mem *p){
  प्रतिज्ञा( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  प्रतिज्ञा( VdbeMemDynamic(p) );
  यदि( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    प्रतिज्ञा( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  यदि( p->flags&MEM_Dyn ){
    प्रतिज्ञा( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((शून्य *)p->z);
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
स्थिर SQLITE_NOINLINE शून्य vdbeMemClear(Mem *p){
  यदि( VdbeMemDynamic(p) ){
    vdbeMemClearExternAndSetNull(p);
  }
  यदि( p->szMalloc ){
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
शून्य sqlite3VdbeMemRelease(Mem *p){
  प्रतिज्ञा( sqlite3VdbeCheckMemInvariants(p) );
  यदि( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
शून्य sqlite3VdbeMemReleaseMalloc(Mem *p){
  प्रतिज्ञा( !VdbeMemDynamic(p) );
  यदि( p->szMalloc ) vdbeMemClear(p);
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
स्थिर SQLITE_NOINLINE i64 mem_Int_मूल्य(ध्रुव Mem *pMem){
  i64 मूल्य = 0;
  sqlite3Atoi64(pMem->z, &मूल्य, pMem->n, pMem->enc);
  प्रत्यागम मूल्य;
}
i64 sqlite3_Vdbe_Int_मूल्य(ध्रुव Mem *pMem){
  पूर्णाङ्क flags;
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  यदि( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    प्रत्यागम pMem->u.i;
  }अन्यथा यदि( flags & MEM_Real ){
    प्रत्यागम sqlite3RealToI64(pMem->u.r);
  }अन्यथा यदि( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    प्रत्यागम mem_Int_मूल्य(pMem);
  }अन्यथा{
    प्रत्यागम 0;
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क sqlite3_Mem_Real_मूल्य_RCSlow_Path(
  Mem *pMem,
  द्विगुण *p_मूल्य
){
  पूर्णाङ्क rc = SQLITE_OK;
  *p_मूल्य = 0.0;
  यदि( pMem->enc==SQLITE_UTF8 ){
    वर्ण *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    यदि( zCopy ){
      rc = sqlite3AtoF(zCopy, p_मूल्य);
      sqlite3DbFree(pMem->db, zCopy);
    }
    प्रत्यागम rc;
  }अन्यथा{
    पूर्णाङ्क n, i, j;
    वर्ण *zCopy;
    ध्रुव वर्ण *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    यदि( zCopy ){
      z = pMem->z;
      यदि( pMem->enc==SQLITE_UTF16LE ){
        प्रति(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          यदि( z[i+1]!=0 ) भङ्ग;
        }
      }अन्यथा{
        प्रति(i=j=0; i<n-1; i+=2, j++){
          यदि( z[i]!=0 ) भङ्ग;
          zCopy[j] = z[i+1];
        }
      }     
      प्रतिज्ञा( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_मूल्य);
      यदि( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    प्रत्यागम rc;
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
पूर्णाङ्क sqlite3_Mem_Real_मूल्य_RC(Mem *pMem, द्विगुण *p_मूल्य){
  testcase( pMem->db==0 );
  प्रतिज्ञा( pMem->flags & (MEM_Str|MEM_Blob) );
  यदि( pMem->z==0 ){
    *p_मूल्य = 0.0;
    प्रत्यागम 0;
  }अन्यथा यदि( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_Term)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    प्रत्यागम sqlite3AtoF(pMem->z, p_मूल्य);
  }अन्यथा यदि( pMem->n==0 ){
    *p_मूल्य = 0.0;
    प्रत्यागम 0;
  }अन्यथा{
    प्रत्यागम sqlite3_Mem_Real_मूल्य_RCSlow_Path(pMem, p_मूल्य);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
स्थिर SQLITE_NOINLINE द्विगुण sqlite3_Mem_Real_मूल्य_No_RC(Mem *pMem){
  द्विगुण r;
  (शून्य)sqlite3_Mem_Real_मूल्य_RC(pMem, &r);
  प्रत्यागम r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
द्विगुण sqlite3_Vdbe_Real_मूल्य(Mem *pMem){
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pMem) );
  यदि( pMem->flags & MEM_Real ){
    प्रत्यागम pMem->u.r;
  }अन्यथा यदि( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    प्रत्यागम (द्विगुण)pMem->u.i;
  }अन्यथा यदि( pMem->flags & (MEM_Str|MEM_Blob) ){
    प्रत्यागम sqlite3_Mem_Real_मूल्य_No_RC(pMem);
  }अन्यथा{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    प्रत्यागम (द्विगुण)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
पूर्णाङ्क sqlite3_Vdbe_Boolean_मूल्य(Mem *pMem, पूर्णाङ्क ifNull){
  testcase( pMem->flags & MEM_IntReal );
  यदि( pMem->flags & (MEM_Int|MEM_IntReal) ) प्रत्यागम pMem->u.i!=0;
  यदि( pMem->flags & MEM_Null ) प्रत्यागम ifNull;
  प्रत्यागम sqlite3_Vdbe_Real_मूल्य(pMem)!=0.0;
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
शून्य sqlite3VdbeIntegerAffinity(Mem *pMem, पूर्णाङ्क bFromText){
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->flags & (MEM_Real|MEM_IntReal) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pMem) );

  यदि( pMem->flags & MEM_IntReal ){
    MemSetTypeFlag(pMem, MEM_Int);
  }अन्यथा{
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
    यदि( pMem->u.r==ix && ix<LARGEST_INT64
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
पूर्णाङ्क sqlite3VdbeMemIntegerify(Mem *pMem){
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_मूल्य(pMem);
  MemSetTypeFlag(pMem, MEM_Int);
  प्रत्यागम SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
पूर्णाङ्क sqlite3VdbeMemRealify(Mem *pMem){
  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_मूल्य(pMem);
  MemSetTypeFlag(pMem, MEM_Real);
  प्रत्यागम SQLITE_OK;
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
पूर्णाङ्क sqlite3RealSameAsInt(द्विगुण r1, sqlite3_int64 i){
  द्विगुण r2 = (द्विगुण)i;
  प्रत्यागम r1==0.0
      || (memcmp(&r1, &r2, परिमाण(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3RealToI64(द्विगुण r){
  यदि( r<-9223372036854774784.0 ) प्रत्यागम SMALLEST_INT64;
  यदि( r>+9223372036854774784.0 ) प्रत्यागम LARGEST_INT64;
  प्रत्यागम (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
पूर्णाङ्क sqlite3VdbeMemNumerify(Mem *pMem){
  प्रतिज्ञा( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  यदि( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    पूर्णाङ्क rc;
    sqlite3_int64 ix;
    प्रतिज्ञा( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_मूल्य_RC(pMem, &pMem->u.r);
    यदि( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3RealToI64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      MemSetTypeFlag(pMem, MEM_Int);
    }अन्यथा{
      MemSetTypeFlag(pMem, MEM_Real);
    }
  }
  प्रतिज्ञा( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  प्रत्यागम SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
पूर्णाङ्क sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  यदि( pMem->flags & MEM_Null ) प्रत्यागम SQLITE_OK;
  विकल्प( aff ){
    पक्ष SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      यदि( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_मूल्य_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
        प्रतिज्ञा( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        यदि( pMem->flags & MEM_Str ) MemSetTypeFlag(pMem, MEM_Blob);
      }अन्यथा{
        pMem->flags &= ~(MEM_TypeMask&~MEM_Blob);
      }
      भङ्ग;
    }
    पक्ष SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      भङ्ग;
    }
    पक्ष SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      भङ्ग;
    }
    पक्ष SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      भङ्ग;
    }
    स्वभाव: {
      पूर्णाङ्क rc;
      प्रतिज्ञा( aff==SQLITE_AFF_TEXT );
      प्रतिज्ञा( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_मूल्य_Apply_Affinity(pMem, SQLITE_AFF_TEXT, encoding);
      प्रतिज्ञा( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      यदि( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      यदि( rc ) प्रत्यागम rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
शून्य sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  प्रतिज्ञा( (flags & ~MEM_TypeMask)==0 );
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
शून्य sqlite3VdbeMemSetNull(Mem *pMem){
  यदि( VdbeMemDynamic(pMem) ){
    vdbeMemClearExternAndSetNull(pMem);
  }अन्यथा{
    pMem->flags = MEM_Null;
  }
}
शून्य sqlite3_मूल्य_Set_Null(sqlite3_मूल्य *p){
  sqlite3VdbeMemSetNull((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#ifndef SQLITE_OMIT_INCRBLOB
शून्य sqlite3VdbeMemSetZeroBlob(Mem *pMem, पूर्णाङ्क n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  यदि( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#अन्यथा
पूर्णाङ्क sqlite3VdbeMemSetZeroBlob(Mem *pMem, पूर्णाङ्क n){
  पूर्णाङ्क nByte = n>0?n:1;
  यदि( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  प्रतिज्ञा( pMem->z!=0 );
  प्रतिज्ञा( sqlite3DbMallocSize(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  प्रत्यागम SQLITE_OK;
}
#endif

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
स्थिर SQLITE_NOINLINE शून्य vdbeReleaseAndSetInt64(Mem *pMem, i64 val){
  sqlite3VdbeMemSetNull(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
शून्य sqlite3VdbeMemSetInt64(Mem *pMem, i64 val){
  यदि( VdbeMemDynamic(pMem) ){
    vdbeReleaseAndSetInt64(pMem, val);
  }अन्यथा{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
शून्य sqlite3MemSetArrayInt64(sqlite3_मूल्य *aMem, पूर्णाङ्क iIdx, i64 val){
  sqlite3VdbeMemSetInt64(&aMem[iIdx], val);
}

/* A no-op destructor */
शून्य sqlite3NoopDestructor(शून्य *p){ UNUSED_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
शून्य sqlite3VdbeMemSetPointer(
  Mem *pMem,
  शून्य *pPtr,
  ध्रुव वर्ण *zPType,
  शून्य (*xDestructor)(शून्य*)
){
  प्रतिज्ञा( pMem->flags==MEM_Null );
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
शून्य sqlite3VdbeMemSetDouble(Mem *pMem, द्विगुण val){
  sqlite3VdbeMemSetNull(pMem);
  यदि( !sqlite3IsNaN(val) ){
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
पूर्णाङ्क sqlite3VdbeMemIsRowSet(ध्रुव Mem *pMem){
  प्रत्यागम (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
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
पूर्णाङ्क sqlite3VdbeMemSetRowSet(Mem *pMem){
  sqlite3 *db = pMem->db;
  RowSet *p;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3RowSetInit(db);
  यदि( p==0 ) प्रत्यागम SQLITE_NOMEM;
  pMem->z = (वर्ण*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3RowSetDelete;
  प्रत्यागम SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
पूर्णाङ्क sqlite3VdbeMemTooBig(Mem *p){
  प्रतिज्ञा( p->db!=0 );
  यदि( p->flags & (MEM_Str|MEM_Blob) ){
    पूर्णाङ्क n = p->n;
    यदि( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    प्रत्यागम n>p->db->aLimit[SQLITE_LIMIT_LENGTH];
  }
  प्रत्यागम 0; 
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
शून्य sqlite3VdbeMemAboutToChange(Vdbe *pVdbe, Mem *pMem){
  पूर्णाङ्क i;
  Mem *pX;
  यदि( pMem->bScopy ){
    प्रति(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      यदि( pX->pScopyFrom==pMem ){
        u16 mFlags;
        यदि( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (पूर्णाङ्क)(pX - pVdbe->aMem), (पूर्णाङ्क)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        प्रतिज्ञा( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
स्थिर SQLITE_NOINLINE शून्य vdbeClrCopy(Mem *pTo, ध्रुव Mem *pFrom, पूर्णाङ्क eType){
  vdbeMemClearExternAndSetNull(pTo);
  प्रतिज्ञा( !VdbeMemDynamic(pTo) );
  sqlite3VdbeMemShallowCopy(pTo, pFrom, eType);
}
शून्य sqlite3VdbeMemShallowCopy(Mem *pTo, ध्रुव Mem *pFrom, पूर्णाङ्क srcType){
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pFrom) );
  प्रतिज्ञा( pTo->db==pFrom->db );
  यदि( VdbeMemDynamic(pTo) ){ vdbeClrCopy(pTo,pFrom,srcType); प्रत्यागम; }
  memcpy(pTo, pFrom, MEMCELLSIZE);
  यदि( (pFrom->flags&MEM_Static)==0 ){
    pTo->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    प्रतिज्ञा( srcType==MEM_Ephem || srcType==MEM_Static );
    pTo->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
पूर्णाङ्क sqlite3VdbeMemCopy(Mem *pTo, ध्रुव Mem *pFrom){
  पूर्णाङ्क rc = SQLITE_OK;

  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pFrom) );
  यदि( VdbeMemDynamic(pTo) ) vdbeMemClearExternAndSetNull(pTo);
  memcpy(pTo, pFrom, MEMCELLSIZE);
  pTo->flags &= ~MEM_Dyn;
  यदि( pTo->flags&(MEM_Str|MEM_Blob) ){
    यदि( 0==(pFrom->flags&MEM_Static) ){
      pTo->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(pTo);
    }
  }

  प्रत्यागम rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
शून्य sqlite3VdbeMemMove(Mem *pTo, Mem *pFrom){
  प्रतिज्ञा( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  प्रतिज्ञा( pTo->db==0 || sqlite3_mutex_held(pTo->db->mutex) );
  प्रतिज्ञा( pFrom->db==0 || pTo->db==0 || pFrom->db==pTo->db );

  sqlite3VdbeMemRelease(pTo);
  memcpy(pTo, pFrom, परिमाण(Mem));
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
पूर्णाङ्क sqlite3VdbeMemSetStr(
  Mem *pMem,          /* Memory cell to set to string value */
  ध्रुव वर्ण *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  शून्य (*xDel)(शून्य*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  पूर्णाङ्क iLimit;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  प्रतिज्ञा( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  यदि( !z ){
    sqlite3VdbeMemSetNull(pMem);
    प्रत्यागम SQLITE_OK;
  }

  यदि( pMem->db ){
    iLimit = pMem->db->aLimit[SQLITE_LIMIT_LENGTH];
  }अन्यथा{
    iLimit = SQLITE_MAX_LENGTH;
  }
  यदि( nByte<0 ){
    प्रतिज्ञा( enc!=0 );
    यदि( enc==SQLITE_UTF8 ){
      nByte = strlen(z);
    }अन्यथा{
      प्रति(nByte=0; nByte<=iLimit && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_Term;
  }अन्यथा यदि( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }अन्यथा{
    flags = MEM_Str;
  }
  यदि( nByte>iLimit ){
    यदि( xDel && xDel!=SQLITE_TRANSIENT ){
      यदि( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (शून्य*)z);
      }अन्यथा{
        xDel((शून्य*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    प्रत्यागम sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  यदि( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    यदि( flags&MEM_Term ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    यदि( sqlite3VdbeMemClearAndResize(pMem, (पूर्णाङ्क)MAX(nAlloc,32)) ){
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    प्रतिज्ञा( pMem->z!=0 );
    memcpy(pMem->z, z, nAlloc);
  }अन्यथा{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (वर्ण *)z;
    यदि( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
    }अन्यथा{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (पूर्णाङ्क)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#ifndef SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  यदि( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
#endif


  प्रत्यागम SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
पूर्णाङ्क sqlite3VdbeMemSetText(
  Mem *pMem,          /* Memory cell to set to string value */
  ध्रुव वर्ण *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  शून्य (*xDel)(शून्य*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  प्रतिज्ञा( pMem!=0 );
  प्रतिज्ञा( pMem->db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  यदि( !z ){
    sqlite3VdbeMemSetNull(pMem);
    प्रत्यागम SQLITE_OK;
  }

  यदि( nByte<0 ){
    nByte = strlen(z);
    flags = MEM_Str|MEM_Term;
  }अन्यथा{
    flags = MEM_Str;
  }
  यदि( nByte>(i64)pMem->db->aLimit[SQLITE_LIMIT_LENGTH] ){
    यदि( xDel && xDel!=SQLITE_TRANSIENT ){
      यदि( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (शून्य*)z);
      }अन्यथा{
        xDel((शून्य*)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    प्रत्यागम sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  यदि( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    यदि( sqlite3VdbeMemClearAndResize(pMem, (पूर्णाङ्क)MAX(nAlloc,32)) ){
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    प्रतिज्ञा( pMem->z!=0 );
    memcpy(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }अन्यथा{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (वर्ण *)z;
    यदि( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }अन्यथा यदि( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }अन्यथा{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (पूर्णाङ्क)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  प्रत्यागम SQLITE_OK;
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
पूर्णाङ्क sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 offset,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  पूर्णाङ्क rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE-1 );
  testcase( amt==SQLITE_MAX_ALLOCATION_SIZE );
  यदि( amt>=SQLITE_MAX_ALLOCATION_SIZE ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  यदि( (u64)amt + (u64)offset > (u64)sqlite3BtreeMaxRecordSize(pCur) ){
    प्रत्यागम SQLITE_CORRUPT_BKPT;
  }
  यदि( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, offset, amt, pMem->z);
    यदि( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (पूर्णाङ्क)amt;
    }अन्यथा{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3VdbeMemFromBtreeZeroOffset(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 available = 0;  /* Number of bytes available on the local btree page */
  पूर्णाङ्क rc = SQLITE_OK; /* Return code */

  प्रतिज्ञा( sqlite3BtreeCursorIsValid(pCur) );
  प्रतिज्ञा( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pMem) );
  pMem->z = (वर्ण *)sqlite3BtreePayloadFetch(pCur, &available);
  प्रतिज्ञा( pMem->z!=0 );

  यदि( amt<=available ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (पूर्णाङ्क)amt;
  }अन्यथा{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  प्रत्यागम rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
स्थिर SQLITE_NOINLINE ध्रुव शून्य *मूल्य_To_Text(sqlite3_मूल्य* pVal, u8 enc){
  प्रतिज्ञा( pVal!=0 );
  प्रतिज्ञा( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  प्रतिज्ञा( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pVal) );
  प्रतिज्ञा( (pVal->flags & (MEM_Null))==0 );
  यदि( pVal->flags & (MEM_Blob|MEM_Str) ){
    यदि( ExpandBlob(pVal) ) प्रत्यागम 0;
    pVal->flags |= MEM_Str;
    यदि( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    यदि( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_TO_INT(pVal->z)) ){
      प्रतिज्ञा( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      यदि( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        प्रत्यागम 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }अन्यथा{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    प्रतिज्ञा( 0==(1&SQLITE_PTR_TO_INT(pVal->z)) );
  }
  प्रतिज्ञा(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  यदि( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    प्रतिज्ञा( sqlite3VdbeMemValidStrRep(pVal) );
    प्रत्यागम pVal->z;
  }अन्यथा{
    प्रत्यागम 0;
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
ध्रुव शून्य *sqlite3_मूल्य_Text(sqlite3_मूल्य* pVal, u8 enc){
  यदि( !pVal ) प्रत्यागम 0;
  प्रतिज्ञा( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  प्रतिज्ञा( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  प्रतिज्ञा( !sqlite3VdbeMemIsRowSet(pVal) );
  यदि( (pVal->flags&(MEM_Str|MEM_Term))==(MEM_Str|MEM_Term) && pVal->enc==enc ){
    प्रतिज्ञा( sqlite3VdbeMemValidStrRep(pVal) );
    प्रत्यागम pVal->z;
  }
  यदि( pVal->flags&MEM_Null ){
    प्रत्यागम 0;
  }
  प्रत्यागम मूल्य_To_Text(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
पूर्णाङ्क sqlite3_मूल्य_Is_Of_Class(ध्रुव sqlite3_मूल्य *pVal, शून्य(*xFree)(शून्य*)){
  यदि( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    प्रत्यागम 1;
  }अन्यथा{
    प्रत्यागम 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_मूल्य *sqlite3_मूल्य_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, परिमाण(*p));
  यदि( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  प्रत्यागम p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
संरचना मूल्य_New_Stat4_Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  पूर्णाङ्क iVal;
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
स्थिर sqlite3_मूल्य *मूल्य_New(sqlite3 *db, संरचना मूल्य_New_Stat4_Ctx *p){
#ifdef SQLITE_ENABLE_STAT4
  यदि( p ){
    UnpackedRecord *pRec = p->ppRec[0];

    यदि( pRec==0 ){
      Index *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      पूर्णाङ्क i;                      /* Counter variable */
      पूर्णाङ्क nCol = pIdx->nColumn;   /* Number of index columns including rowid */
  
      nByte = परिमाण(Mem) * nCol + ROUND8(परिमाण(UnpackedRecord));
      pRec = (UnpackedRecord*)sqlite3DbMallocZero(db, nByte);
      यदि( pRec ){
        pRec->p_किल्ली_Info = sqlite3_किल्ली_Info_Of_Index(p->pParse, pIdx);
        यदि( pRec->p_किल्ली_Info ){
          प्रतिज्ञा( pRec->p_किल्ली_Info->nAllField==nCol );
          प्रतिज्ञा( pRec->p_किल्ली_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(परिमाण(UnpackedRecord)));
          प्रति(i=0; i<nCol; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }अन्यथा{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      यदि( pRec==0 ) प्रत्यागम 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->nField = p->iVal+1;
    sqlite3VdbeMemSetNull(&pRec->aMem[p->iVal]);
    प्रत्यागम &pRec->aMem[p->iVal];
  }
#अन्यथा
  UNUSED_PARAMETER(p);
#endif /* defined(SQLITE_ENABLE_STAT4) */
  प्रत्यागम sqlite3_मूल्य_New(db);
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
स्थिर पूर्णाङ्क मूल्य_From_Function(
  sqlite3 *db,                    /* The database connection */
  ध्रुव अभिव्यक्ती *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_मूल्य **ppVal,          /* Write the new value here */
  संरचना मूल्य_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_मूल्य **apVal = 0;      /* Function arguments */
  पूर्णाङ्क nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_मूल्य *pVal = 0;        /* New value */
  पूर्णाङ्क rc = SQLITE_OK;             /* Return code */
  अभिव्यक्ती_List *pList = 0;            /* Function arguments */
  पूर्णाङ्क i;                          /* Iterator variable */

  प्रतिज्ञा( pCtx!=0 );
  प्रतिज्ञा( (p->flags & EP_TokenOnly)==0 );
  प्रतिज्ञा( अभिव्यक्ती_Use_XList(p) );
  pList = p->x.pList;
  यदि( pList ) nVal = pList->n_अभिव्यक्ती;
  प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p, EP_Int_मूल्य) );
  pFunc = sqlite3FindFunction(db, p->u.zToken, nVal, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  यदि( pFunc==0 ) प्रत्यागम SQLITE_OK;
#endif
  प्रतिज्ञा( pFunc );
  यदि( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    प्रत्यागम SQLITE_OK;
  }

  यदि( pList ){
    apVal = (sqlite3_मूल्य**)sqlite3DbMallocZero(db, परिमाण(apVal[0]) * nVal);
    यदि( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      गच्छ मूल्य_from_function_out;
    }
    प्रति(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_मूल्य_From_अभिव्यक्ती(pCtx->pParse, pList->a[i].p_अभिव्यक्ती, aff,
                                     &apVal[i]);
      यदि( apVal[i]==0 || rc!=SQLITE_OK ) गच्छ मूल्य_from_function_out;
    }
  }

  pVal = मूल्य_New(db, pCtx);
  यदि( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    गच्छ मूल्य_from_function_out;
  }

  memset(&ctx, 0, परिमाण(ctx));
  ctx.pOut = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  यदि( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_संदेश(pCtx->pParse, "%s", sqlite3_मूल्य_text(pVal));
  }अन्यथा{
    sqlite3_मूल्य_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    प्रतिज्ञा( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    यदि( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 मूल्य_from_function_out:
  यदि( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  यदि( apVal ){
    प्रति(i=0; i<nVal; i++){
      sqlite3_मूल्य_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  प्रत्यागम rc;
}
#अन्यथा
# define मूल्य_From_Function(a,b,c,d,e,f) SQLITE_OK
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
स्थिर पूर्णाङ्क मूल्य_From_अभिव्यक्ती(
  sqlite3 *db,                    /* The database connection */
  ध्रुव अभिव्यक्ती *p_अभिव्यक्ती,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_मूल्य **ppVal,          /* Write the new value here */
  संरचना मूल्य_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  पूर्णाङ्क op;
  वर्ण *zVal = 0;
  sqlite3_मूल्य *pVal = 0;
  पूर्णाङ्क negInt = 1;
  ध्रुव वर्ण *zNeg = "";
  पूर्णाङ्क rc = SQLITE_OK;

  प्रतिज्ञा( p_अभिव्यक्ती!=0 );
  यावत्( (op = p_अभिव्यक्ती->op)==TK_UPLUS || op==TK_SPAN ) p_अभिव्यक्ती = p_अभिव्यक्ती->p_डावा;
  यदि( op==TK_REGISTER ) op = p_अभिव्यक्ती->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  प्रतिज्ञा( (p_अभिव्यक्ती->flags & EP_TokenOnly)==0 || pCtx==0 );

  यदि( op==TK_CAST ){
    u8 aff;
    प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
    aff = sqlite3AffinityType(p_अभिव्यक्ती->u.zToken,0);
    rc = मूल्य_From_अभिव्यक्ती(db, p_अभिव्यक्ती->p_डावा, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    यदि( *ppVal ){
#ifdef SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#अन्यथा
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      प्रतिज्ञा( (ppVal[0][0].flags & MEM_Zero)==0 );
#endif
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_मूल्य_Apply_Affinity(*ppVal, affinity, enc);
    }
    प्रत्यागम rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  यदि( op==TK_UMINUS ){
    अभिव्यक्ती *p_डावा = p_अभिव्यक्ती->p_डावा;
    यदि( (p_डावा->op==TK_INTEGER || p_डावा->op==TK_FLOAT) ){
      यदि( अभिव्यक्ती_Has_Property(p_डावा, EP_Int_मूल्य)
       || p_डावा->u.zToken[0]!='0' || (p_डावा->u.zToken[1] & ~0x20)!='X'
      ){
        p_अभिव्यक्ती = p_डावा;
        op = p_अभिव्यक्ती->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  यदि( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = मूल्य_New(db, pCtx);
    यदि( pVal==0 ) गच्छ no_mem;
    यदि( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) ){
      sqlite3VdbeMemSetInt64(pVal, (i64)p_अभिव्यक्ती->u.i_मूल्य*negInt);
    }अन्यथा{
      i64 iVal;
      यदि( op==TK_INTEGER && 0==sqlite3DecOrHexToI64(p_अभिव्यक्ती->u.zToken, &iVal) ){
        sqlite3VdbeMemSetInt64(pVal, iVal*negInt);
      }अन्यथा{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_अभिव्यक्ती->u.zToken);
        यदि( zVal==0 ) गच्छ no_mem;
        sqlite3_मूल्य_Set_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        यदि( op==TK_FLOAT ){
          प्रतिज्ञा( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_Term) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    यदि( affinity==SQLITE_AFF_BLOB ){
      यदि( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_मूल्य_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }अन्यथा{
      sqlite3_मूल्य_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    प्रतिज्ञा( (pVal->flags & MEM_IntReal)==0 );
    यदि( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    यदि( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }अन्यथा यदि( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    यदि( SQLITE_OK==मूल्य_From_अभिव्यक्ती(db,p_अभिव्यक्ती->p_डावा,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      यदि( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }अन्यथा यदि( pVal->u.i==SMALLEST_INT64 ){
#ifndef SQLITE_OMIT_FLOATING_POINT
        pVal->u.r = -(द्विगुण)SMALLEST_INT64;
#अन्यथा
        pVal->u.r = LARGEST_INT64;
#endif
        MemSetTypeFlag(pVal, MEM_Real);
      }अन्यथा{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_मूल्य_Apply_Affinity(pVal, affinity, enc);
    }
  }अन्यथा यदि( op==TK_NULL ){
    pVal = मूल्य_New(db, pCtx);
    यदि( pVal==0 ) गच्छ no_mem;
    sqlite3VdbeMemSetNull(pVal);
  }
#ifndef SQLITE_OMIT_BLOB_LITERAL
  अन्यथा यदि( op==TK_BLOB ){
    पूर्णाङ्क nVal;
    प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
    प्रतिज्ञा( p_अभिव्यक्ती->u.zToken[0]=='x' || p_अभिव्यक्ती->u.zToken[0]=='X' );
    प्रतिज्ञा( p_अभिव्यक्ती->u.zToken[1]=='\'' );
    pVal = मूल्य_New(db, pCtx);
    यदि( !pVal ) गच्छ no_mem;
    zVal = &p_अभिव्यक्ती->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    प्रतिज्ञा( zVal[nVal]=='\'' );
    sqlite3VdbeMemSetStr(pVal, sqlite3HexToBlob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#endif
#ifdef SQLITE_ENABLE_STAT4
  अन्यथा यदि( op==TK_FUNCTION && pCtx!=0 ){
    rc = मूल्य_From_Function(db, p_अभिव्यक्ती, enc, affinity, &pVal, pCtx);
  }
#endif
  अन्यथा यदि( op==TK_TRUEFALSE ){
    प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
    pVal = मूल्य_New(db, pCtx);
    यदि( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_अभिव्यक्ती->u.zToken[4]==0;
      sqlite3_मूल्य_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  प्रत्यागम rc;

no_mem:
#ifdef SQLITE_ENABLE_STAT4
  यदि( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#endif
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  प्रतिज्ञा( *ppVal==0 );
#ifdef SQLITE_ENABLE_STAT4
  यदि( pCtx==0 ) sqlite3_मूल्य_Free(pVal);
#अन्यथा
  प्रतिज्ञा( pCtx==0 ); sqlite3_मूल्य_Free(pVal);
#endif
  प्रत्यागम SQLITE_NOMEM_BKPT;
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
पूर्णाङ्क sqlite3_मूल्य_From_अभिव्यक्ती(
  sqlite3 *db,              /* The database connection */
  ध्रुव अभिव्यक्ती *p_अभिव्यक्ती,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_मूल्य **ppVal     /* Write the new value here */
){
  पूर्णाङ्क rc = SQLITE_OK;
  sqlite3_मूल्य *pVal = 0;
  यदि( p_अभिव्यक्ती ){
    rc = मूल्य_From_अभिव्यक्ती(db, p_अभिव्यक्ती, enc, affinity, &pVal, 0);
    यदि( rc==SQLITE_OK && pVal 
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
  प्रत्यागम rc;
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
स्थिर पूर्णाङ्क stat4_मूल्य_From_अभिव्यक्ती(
  Parse *pParse,                  /* Parse context */
  अभिव्यक्ती *p_अभिव्यक्ती,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  संरचना मूल्य_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_मूल्य **ppVal           /* OUT: New value object (or NULL) */
){
  पूर्णाङ्क rc = SQLITE_OK;
  sqlite3_मूल्य *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_अभिव्यक्ती = sqlite3_अभिव्यक्ती_Skip_Collate(p_अभिव्यक्ती);

  प्रतिज्ञा( p_अभिव्यक्ती==0 || p_अभिव्यक्ती->op!=TK_REGISTER || p_अभिव्यक्ती->op2!=TK_VARIABLE );
  यदि( !p_अभिव्यक्ती ){
    pVal = मूल्य_New(db, pAlloc);
    यदि( pVal ){
      sqlite3VdbeMemSetNull((Mem*)pVal);
    }
  }अन्यथा यदि( p_अभिव्यक्ती->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    पूर्णाङ्क iBindVar = p_अभिव्यक्ती->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    यदि( (v = pParse->pReprepare)!=0 ){
      pVal = मूल्य_New(db, pAlloc);
      यदि( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_मूल्य_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }अन्यथा{
    rc = मूल्य_From_अभिव्यक्ती(db, p_अभिव्यक्ती, ENC(db), affinity, &pVal, pAlloc);
  }

  प्रतिज्ञा( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  प्रत्यागम rc;
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
पूर्णाङ्क sqlite3_Stat4_Probe_Set_मूल्य(
  Parse *pParse,                  /* Parse context */
  Index *pIdx,                    /* Index being probed */
  UnpackedRecord **ppRec,         /* IN/OUT: Probe record */
  अभिव्यक्ती *p_अभिव्यक्ती,                    /* The expression to extract a value from */
  पूर्णाङ्क nElem,                      /* Maximum number of values to append */
  पूर्णाङ्क iVal,                       /* Array element to populate */
  पूर्णाङ्क *pnExtract                  /* OUT: Values appended to the record */
){
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क nExtract = 0;

  यदि( p_अभिव्यक्ती==0 || p_अभिव्यक्ती->op!=TK_SELECT ){
    पूर्णाङ्क i;
    संरचना मूल्य_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    प्रति(i=0; i<nElem; i++){
      sqlite3_मूल्य *pVal = 0;
      अभिव्यक्ती *pElem = (p_अभिव्यक्ती ? sqlite3VectorFieldSubexpr(p_अभिव्यक्ती, i) : 0);
      u8 aff = sqlite3IndexColumnAffinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_मूल्य_From_अभिव्यक्ती(pParse, pElem, aff, &alloc, &pVal);
      यदि( !pVal ) भङ्ग;
      nExtract++;
    }
  }

  *pnExtract = nExtract;
  प्रत्यागम rc;
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
पूर्णाङ्क sqlite3_Stat4_मूल्य_From_अभिव्यक्ती(
  Parse *pParse,                  /* Parse context */
  अभिव्यक्ती *p_अभिव्यक्ती,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_मूल्य **ppVal           /* OUT: New value object (or NULL) */
){
  प्रत्यागम stat4_मूल्य_From_अभिव्यक्ती(pParse, p_अभिव्यक्ती, affinity, 0, ppVal);
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
पूर्णाङ्क sqlite3Stat4Column(
  sqlite3 *db,                    /* Database handle */
  ध्रुव शून्य *pRec,               /* Pointer to buffer containing record */
  पूर्णाङ्क nRec,                       /* Size of buffer pRec in bytes */
  पूर्णाङ्क iCol,                       /* Column to extract */
  sqlite3_मूल्य **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 iField;                     /* Next unread data byte */
  u32 szField = 0;                /* Size of the current data field */
  पूर्णाङ्क i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  प्रतिज्ञा( iCol>0 );
  iHdr = getVarint32(a, nHdr);
  यदि( nHdr>(u32)nRec || iHdr>=nHdr ) प्रत्यागम SQLITE_CORRUPT_BKPT;
  iField = nHdr;
  प्रति(i=0; i<=iCol; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    यदि( iHdr>nHdr ) प्रत्यागम SQLITE_CORRUPT_BKPT;
    szField = sqlite3VdbeSerialTypeLen(t);
    iField += szField;
  }
  testcase( iField==nRec );
  testcase( iField==nRec+1 );
  यदि( iField>nRec ) प्रत्यागम SQLITE_CORRUPT_BKPT;
  यदि( pMem==0 ){
    pMem = *ppVal = sqlite3_मूल्य_New(db);
    यदि( pMem==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[iField-szField], t, pMem);
  pMem->enc = ENC(db);
  प्रत्यागम SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
शून्य sqlite3Stat4ProbeFree(UnpackedRecord *pRec){
  यदि( pRec ){
    पूर्णाङ्क i;
    पूर्णाङ्क nCol = pRec->p_किल्ली_Info->nAllField;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    प्रति(i=0; i<nCol; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_किल्ली_Info_Unref(pRec->p_किल्ली_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#endif /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
शून्य sqlite3_मूल्य_Set_Str(
  sqlite3_मूल्य *v,     /* Value to be set */
  पूर्णाङ्क n,                /* Length of string z */
  ध्रुव शून्य *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  शून्य (*xDel)(शून्य*)   /* Destructor for the string */
){
  यदि( v ) sqlite3VdbeMemSetStr((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
शून्य sqlite3_मूल्य_Free(sqlite3_मूल्य *v){
  यदि( !v ) प्रत्यागम;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क मूल्य_Bytes(sqlite3_मूल्य *pVal, u8 enc){
  प्रत्यागम मूल्य_To_Text(pVal, enc)!=0 ? pVal->n : 0;
}
पूर्णाङ्क sqlite3_मूल्य_Bytes(sqlite3_मूल्य *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  प्रतिज्ञा( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  यदि( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    प्रत्यागम p->n;
  }
  यदि( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    प्रत्यागम p->n;
  }
  यदि( (p->flags & MEM_Blob)!=0 ){
    यदि( p->flags & MEM_Zero ){
      प्रत्यागम p->n + p->u.nZero;
    }अन्यथा{
      प्रत्यागम p->n;
    }
  }
  यदि( p->flags & MEM_Null ) प्रत्यागम 0;
  प्रत्यागम मूल्य_Bytes(pVal, enc);
}
