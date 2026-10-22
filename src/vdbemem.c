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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

/* True if X is a power of two.  0 is considered a power of two here.
** In other words, return true if X has at most one bit set.
*/
#నిర్వచించు ISPOWEROF2(X)  (((X)&((X)-1))==0)

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Check invariants on a Mem object.
**
** This routine is intended for use inside of assert() statements, like
** this:    assert( sqlite3VdbeCheckMemInvariants(pMem) );
*/
పూర్ణాంకం sqlite3VdbeCheckMemInvariants(Mem *p){
  /* If MEM_Dyn is set then Mem.xDel!=0.  
  ** Mem.xDel might not be initialized if MEM_Dyn is clear.
  */
  ప్రతిజ్ఞా( (p->flags & MEM_Dyn)==0 || p->xDel!=0 );

  /* MEM_Dyn may only be set if Mem.szMalloc==0.  In this way we
  ** ensure that if Mem.szMalloc>0 then it is safe to do
  ** Mem.z = Mem.zMalloc without having to check Mem.flags&MEM_Dyn.
  ** That saves a few cycles in inner loops. */
  ప్రతిజ్ఞా( (p->flags & MEM_Dyn)==0 || p->szMalloc==0 );

  /* Cannot have more than one of MEM_Int, MEM_Real, or MEM_IntReal */
  ప్రతిజ్ఞా( ISPOWEROF2(p->flags & (MEM_Int|MEM_Real|MEM_IntReal)) );

  ఒకవేళ( p->flags & MEM_Null ){
    /* Cannot be both MEM_Null and some other type */
    ప్రతిజ్ఞా( (p->flags & (MEM_Int|MEM_Real|MEM_Str|MEM_Blob|MEM_Agg))==0 );

    /* If MEM_Null is set, then either the value is a pure NULL (the usual
    ** case) or it is a pointer set using sqlite3_bind_pointer() or
    ** sqlite3_result_pointer().  If a pointer, then MEM_Term must also be
    ** set.
    */
    ఒకవేళ( (p->flags & (MEM_పదం|MEM_Subtype))==(MEM_పదం|MEM_Subtype) ){
      /* This is a pointer type.  There may be a flag to indicate what to
      ** do with the pointer. */
      ప్రతిజ్ఞా( ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
              ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
              ((p->flags&MEM_Static)!=0 ? 1 : 0) <= 1 );

      /* No other bits set */
      ప్రతిజ్ఞా( (p->flags & ~(MEM_Null|MEM_పదం|MEM_Subtype|MEM_FromBind
                           |MEM_Dyn|MEM_Ephem|MEM_Static))==0 );
    }లేకపోతే{
      /* A pure NULL might have other flags, such as MEM_Static, MEM_Dyn,
      ** MEM_Ephem, MEM_Cleared, or MEM_Subtype */
    }
  }లేకపోతే{
    /* The MEM_Cleared bit is only allowed on NULLs */
    ప్రతిజ్ఞా( (p->flags & MEM_Cleared)==0 );
  }

  /* The szMalloc field holds the correct memory allocation size */
  ప్రతిజ్ఞా( p->szMalloc==0
       || (p->flags==MEM_Undefined 
           && p->szMalloc<=sqlite3_Db_Malloc_కొలత(p->db,p->zMalloc))
       || p->szMalloc==sqlite3_Db_Malloc_కొలత(p->db,p->zMalloc));

  /* If p holds a string or blob, the Mem.z must point to exactly
  ** one of the following:
  **
  **   (1) Memory in Mem.zMalloc and managed by the Mem object
  **   (2) Memory to be freed using Mem.xDel
  **   (3) An ephemeral string or blob
  **   (4) A static string or blob
  */
  ఒకవేళ( (p->flags & (MEM_Str|MEM_Blob)) && p->n>0 ){
    ప్రతిజ్ఞా( 
      ((p->szMalloc>0 && p->z==p->zMalloc)? 1 : 0) +
      ((p->flags&MEM_Dyn)!=0 ? 1 : 0) +
      ((p->flags&MEM_Ephem)!=0 ? 1 : 0) +
      ((p->flags&MEM_Static)!=0 ? 1 : 0) == 1
    );
  }
  తిరిగి 1;
}
#అయితే_ముగింపు

/*
** Render a Mem object which is one of MEM_Int, MEM_Real, or MEM_IntReal
** into a buffer.
*/
స్థిర శూన్య vdbe_Mem_Render_అంకె(పూర్ణాంకం sz, అక్షరం *zBuf, Mem *p){
  StrAccum acc;
  ప్రతిజ్ఞా( p->flags & (MEM_Int|MEM_Real|MEM_IntReal) );
  ప్రతిజ్ఞా( sz>22 );
  ఒకవేళ( p->flags & (MEM_Int|MEM_IntReal) ){
#ఒకవేళ GCC_సంస్కరణ>=7000000 && GCC_సంస్కరణ<15000000 && defined(__i386__)
    /* Work-around for GCC bug or bugs:
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96270
    ** https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114659
    ** The problem appears to be fixed in GCC 15 */
    i64 x;
    ప్రతిజ్ఞా( (sqlite3Config.bSmallMalloc!=0xee)*8==పరిమాణం(x) );
    జ్ఞాపకం_నకలు(&x, (అక్షరం*)&p->u.i, (sqlite3Config.bSmallMalloc!=0xee)*8);
    p->n = sqlite3_Int64_గమ్యం_పాఠ్యం(x, zBuf);
#లేకపోతే
    p->n = sqlite3_Int64_గమ్యం_పాఠ్యం(p->u.i, zBuf);
#అయితే_ముగింపు
    ఒకవేళ( p->flags & MEM_IntReal ){
      జ్ఞాపకం_నకలు(zBuf+p->n,".0", 3);
      p->n += 2;
    }
  }లేకపోతే{
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g",
         (p->db ? p->db->nFpDigit : 17), p->u.r);
    ప్రతిజ్ఞా( acc.z_పాఠ్యం==zBuf && acc.mxAlloc<=0 );
    zBuf[acc.nChar] = 0; /* Fast version of sqlite3StrAccumFinish(&acc) */
    p->n = acc.nChar;
  }
}

#నిర్వచితమైతే SQLITE_DEBUG
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
పూర్ణాంకం sqlite3VdbeMemValidStrRep(Mem *p){
  Mem tmp;
  అక్షరం zBuf[100];
  అక్షరం *z;
  పూర్ణాంకం i, j, incr;
  ఒకవేళ( (p->flags & MEM_Str)==0 ) తిరిగి 1;
  ఒకవేళ( p->db && p->db->mallocFailed ) తిరిగి 1;
  ఒకవేళ( p->flags & MEM_పదం ){
    /* Insure that the string is properly zero-terminated.  Pay particular
    ** attention to the case where p->n is odd */
    ఒకవేళ( p->szMalloc>0 && p->z==p->zMalloc ){
      ప్రతిజ్ఞా( p->enc==SQLITE_UTF8 || p->szMalloc >= ((p->n+1)&~1)+2 );
      ప్రతిజ్ఞా( p->enc!=SQLITE_UTF8 || p->szMalloc >= p->n+1 );
    }
    ప్రతిజ్ఞా( p->z[p->n]==0 );
    ప్రతిజ్ఞా( p->enc==SQLITE_UTF8 || p->z[(p->n+1)&~1]==0 );
    ప్రతిజ్ఞా( p->enc==SQLITE_UTF8 || p->z[((p->n+1)&~1)+1]==0 );
  }
  ఒకవేళ( (p->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 ) తిరిగి 1;
  ఒకవేళ( p->db==0 ){
    తిరిగి 1;  /* db->nFpDigit required to validate p->z[] */
  }
  జ్ఞాపకం_నకలు(&tmp, p, పరిమాణం(tmp));
  vdbe_Mem_Render_అంకె(పరిమాణం(zBuf), zBuf, &tmp);
  z = p->z;
  i = j = 0;
  incr = 1;
  ఒకవేళ( p->enc!=SQLITE_UTF8 ){
    incr = 2;
    ఒకవేళ( p->enc==SQLITE_UTF16BE ) z++;
  }
  అయితే( zBuf[j] ){
    ఒకవేళ( zBuf[j++]!=z[i] ) తిరిగి 0;
    i += incr;
  }
  తిరిగి 1;
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

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
పూర్ణాంకం sqlite3VdbeChangeEncoding(Mem *pMem, పూర్ణాంకం desiredEnc){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  పూర్ణాంకం rc;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ప్రతిజ్ఞా( desiredEnc==SQLITE_UTF8 || desiredEnc==SQLITE_UTF16LE
           || desiredEnc==SQLITE_UTF16BE );
  ఒకవేళ( !(pMem->flags&MEM_Str) ){
    pMem->enc = desiredEnc;
    తిరిగి SQLITE_OK;
  }
  ఒకవేళ( pMem->enc==desiredEnc ){
    తిరిగి SQLITE_OK;
  }
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
#నిర్వచితమైతే SQLITE_OMIT_UTF16
  తిరిగి SQLITE_ERROR;
#లేకపోతే

  /* MemTranslate() may return SQLITE_OK or SQLITE_NOMEM. If NOMEM is returned,
  ** then the encoding of the value may not have changed.
  */
  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);
  ప్రతిజ్ఞా(rc==SQLITE_OK    || rc==SQLITE_NOMEM);
  ప్రతిజ్ఞా(rc==SQLITE_OK    || pMem->enc!=desiredEnc);
  ప్రతిజ్ఞా(rc==SQLITE_NOMEM || pMem->enc==desiredEnc);
  తిరిగి rc;
#అయితే_ముగింపు
}

/*
** Make sure pMem->z points to a writable allocation of at least n bytes.
**
** If the bPreserve argument is true, then copy of the content of
** pMem->z into the new allocation.  pMem must be either a string or
** blob if bPreserve is true.  If bPreserve is false, any prior content
** in pMem->z is discarded.
*/
SQLITE_NOINLINE పూర్ణాంకం sqlite3VdbeMemGrow(Mem *pMem, పూర్ణాంకం n, పూర్ణాంకం bPreserve){
  ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(pMem) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  testcase( pMem->db==0 );

  /* If the bPreserve flag is set to true, then the memory cell must already
  ** contain a valid string or blob value.  */
  ప్రతిజ్ఞా( bPreserve==0 || pMem->flags&(MEM_Blob|MEM_Str) );
  testcase( bPreserve && pMem->z==0 );

  ప్రతిజ్ఞా( pMem->szMalloc==0
       || (pMem->flags==MEM_Undefined 
           && pMem->szMalloc<=sqlite3_Db_Malloc_కొలత(pMem->db,pMem->zMalloc))
       || pMem->szMalloc==sqlite3_Db_Malloc_కొలత(pMem->db,pMem->zMalloc));
  ఒకవేళ( pMem->szMalloc>0 && bPreserve && pMem->z==pMem->zMalloc ){
    ఒకవేళ( pMem->db ){
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    }లేకపోతే{
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      ఒకవేళ( pMem->zMalloc==0 ) sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  }లేకపోతే{
    ఒకవేళ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  ఒకవేళ( pMem->zMalloc==0 ){
    sqlite3_Vdbe_Mem_సమితి_Null(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    తిరిగి SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    pMem->szMalloc = sqlite3_Db_Malloc_కొలత(pMem->db, pMem->zMalloc);
  }

  ఒకవేళ( bPreserve && pMem->z ){
    ప్రతిజ్ఞా( pMem->z!=pMem->zMalloc );
    జ్ఞాపకం_నకలు(pMem->zMalloc, pMem->z, pMem->n);
  }
  ఒకవేళ( (pMem->flags&MEM_Dyn)!=0 ){
    ప్రతిజ్ఞా( pMem->xDel!=0 && pMem->xDel!=SQLITE_DYNAMIC );
    pMem->xDel((శూన్య *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(MEM_Dyn|MEM_Ephem|MEM_Static);
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3VdbeMemClearAndResize(Mem *pMem, పూర్ణాంకం szNew){
  ప్రతిజ్ఞా( CORRUPT_DB || szNew>0 );
  ప్రతిజ్ఞా( (pMem->flags & MEM_Dyn)==0 || pMem->szMalloc==0 );
  ఒకవేళ( pMem->szMalloc<szNew ){
    తిరిగి sqlite3VdbeMemGrow(pMem, szNew, 0);
  }
  ప్రతిజ్ఞా( (pMem->flags & MEM_Dyn)==0 );
  pMem->z = pMem->zMalloc;
  pMem->flags &= (MEM_Null|MEM_Int|MEM_Real|MEM_IntReal);
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem){
  ఒకవేళ( (pMem->flags & (MEM_Str|MEM_పదం|MEM_Ephem|MEM_Static))!=MEM_Str ){
    /* pMem must be a string, and it cannot be an ephemeral or static string */
    తిరిగి 0;
  }
  ఒకవేళ( pMem->enc!=SQLITE_UTF8 ) తిరిగి 0;
  ప్రతిజ్ఞా( pMem->z!=0 );
  ఒకవేళ( pMem->flags & MEM_Dyn ){
    ఒకవేళ( pMem->xDel==sqlite3_free
     && sqlite3_msize(pMem->z) >= (u64)(pMem->n+1)
    ){
      pMem->z[pMem->n] = 0;
      pMem->flags |= MEM_పదం;
      తిరిగి 1;
    }
    ఒకవేళ( pMem->xDel==sqlite3RCStrUnref ){
      /* Blindly assume that all RCStr objects are zero-terminated */
      pMem->flags |= MEM_పదం;
      తిరిగి 1;
    }
  }లేకపోతే ఒకవేళ( pMem->szMalloc >= pMem->n+1 ){
    pMem->z[pMem->n] = 0;
    pMem->flags |= MEM_పదం;
    తిరిగి 1;
  }
  తిరిగి 0;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbe_Mem_కూడు_Terminator(Mem *pMem){
  ఒకవేళ( sqlite3VdbeMemGrow(pMem, pMem->n+3, 1) ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n+1] = 0;
  pMem->z[pMem->n+2] = 0;
  pMem->flags |= MEM_పదం;
  తిరిగి SQLITE_OK;
}

/*
** Change pMem so that its MEM_Str or MEM_Blob value is stored in
** MEM.zMalloc, where it can be safely written.
**
** Return SQLITE_OK on success or SQLITE_NOMEM if malloc fails.
*/
పూర్ణాంకం sqlite3VdbeMemMakeWriteable(Mem *pMem){
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ఒకవేళ( (pMem->flags & (MEM_Str|MEM_Blob))!=0 ){
    ఒకవేళ( ExpandBlob(pMem) ) తిరిగి SQLITE_NOMEM;
    ఒకవేళ( pMem->szMalloc==0 || pMem->z!=pMem->zMalloc ){
      పూర్ణాంకం rc = vdbe_Mem_కూడు_Terminator(pMem);
      ఒకవేళ( rc ) తిరిగి rc;
    }
  }
  pMem->flags &= ~MEM_Ephem;
#నిర్వచితమైతే SQLITE_DEBUG
  pMem->pScopyFrom = 0;
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
}

/*
** If the given Mem* has a zero-filled tail, turn it into an ordinary
** blob stored in dynamically allocated space.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
పూర్ణాంకం sqlite3VdbeMemExpandBlob(Mem *pMem){
  పూర్ణాంకం nByte;
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->flags & MEM_Zero );
  ప్రతిజ్ఞా( (pMem->flags&MEM_Blob)!=0 || MemNullNochng(pMem) );
  testcase( sqlite3_విలువ_nochange(pMem) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );

  /* Set nByte to the number of bytes required to store the expanded blob. */
  nByte = pMem->n + pMem->u.nZero;
  ఒకవేళ( nByte<=0 ){
    ఒకవేళ( (pMem->flags & MEM_Blob)==0 ) తిరిగి SQLITE_OK;
    nByte = 1;
  }
  ఒకవేళ( sqlite3VdbeMemGrow(pMem, nByte, 1) ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  ప్రతిజ్ఞా( pMem->z!=0 );
  ప్రతిజ్ఞా( sqlite3_Db_Malloc_కొలత(pMem->db,pMem->z) >= nByte );

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(MEM_Zero|MEM_పదం);
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

/*
** Make sure the given Mem is \u0000 terminated.
*/
పూర్ణాంకం sqlite3VdbeMemNulTerminate(Mem *pMem){
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  testcase( (pMem->flags & (MEM_పదం|MEM_Str))==(MEM_పదం|MEM_Str) );
  testcase( (pMem->flags & (MEM_పదం|MEM_Str))==0 );
  ఒకవేళ( (pMem->flags & (MEM_పదం|MEM_Str))!=MEM_Str ){
    తిరిగి SQLITE_OK;   /* Nothing to do */
  }లేకపోతే{
    తిరిగి vdbe_Mem_కూడు_Terminator(pMem);
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
పూర్ణాంకం sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce){
  స్థిరరాశి పూర్ణాంకం nByte = 32;

  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( !(pMem->flags&MEM_Zero) );
  ప్రతిజ్ఞా( !(pMem->flags&(MEM_Str|MEM_Blob)) );
  ప్రతిజ్ఞా( pMem->flags&(MEM_Int|MEM_Real|MEM_IntReal) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );


  ఒకవేళ( sqlite3VdbeMemClearAndResize(pMem, nByte) ){
    pMem->enc = 0;
    తిరిగి SQLITE_NOMEM_BKPT;
  }

  vdbe_Mem_Render_అంకె(nByte, pMem->z, pMem);
  ప్రతిజ్ఞా( pMem->z!=0 );
  ప్రతిజ్ఞా( pMem->n==(పూర్ణాంకం)sqlite3Strlen30NN(pMem->z) );
  pMem->enc = SQLITE_UTF8;
  pMem->flags |= MEM_Str|MEM_పదం;
  ఒకవేళ( bForce ) pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
  sqlite3VdbeChangeEncoding(pMem, enc);
  తిరిగి SQLITE_OK;
}

/*
** Memory cell pMem contains the context of an aggregate function.
** This routine calls the finalize method for that function.  The
** result of the aggregate is stored back into pMem.
**
** Return SQLITE_ERROR if the finalizer reports an error.  SQLITE_OK
** otherwise.
*/
పూర్ణాంకం sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc){
  sqlite3_context ctx;
  Mem t;
  ప్రతిజ్ఞా( pFunc!=0 );
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db!=0 );
  ప్రతిజ్ఞా( pFunc->xFinalize!=0 );
  ప్రతిజ్ఞా( (pMem->flags & MEM_Null)!=0 || pFunc==pMem->u.pDef );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pMem->db->mutex) );
  memset(&ctx, 0, పరిమాణం(ctx));
  memset(&t, 0, పరిమాణం(t));
  t.flags = MEM_Null;
  t.db = pMem->db;
  ctx.p_బహిర్గతం = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(t.db);
  pFunc->xFinalize(&ctx); /* IMP: R-24505-23230 */
  ప్రతిజ్ఞా( (pMem->flags & MEM_Dyn)==0 );
  ఒకవేళ( pMem->szMalloc>0 ) sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  జ్ఞాపకం_నకలు(pMem, &t, పరిమాణం(t));
  తిరిగి ctx.isError;
}

/*
** Memory cell pAccum contains the context of an aggregate function.
** This routine calls the xValue method for that function and stores
** the results in memory cell pMem.
**
** SQLITE_ERROR is returned if xValue() reports an error. SQLITE_OK 
** otherwise.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
పూర్ణాంకం sqlite3_Vdbe_Mem_Agg_విలువ(Mem *pAccum, Mem *p_బహిర్గతం, FuncDef *pFunc){
  sqlite3_context ctx;
  ప్రతిజ్ఞా( pFunc!=0 );
  ప్రతిజ్ఞా( pFunc->x_విలువ!=0 );
  ప్రతిజ్ఞా( (pAccum->flags & MEM_Null)!=0 || pFunc==pAccum->u.pDef );
  ప్రతిజ్ఞా( pAccum->db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pAccum->db->mutex) );
  memset(&ctx, 0, పరిమాణం(ctx));
  sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  ctx.p_బహిర్గతం = p_బహిర్గతం;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(pAccum->db);
  pFunc->x_విలువ(&ctx);
  తిరిగి ctx.isError;
}
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

/*
** If the memory cell contains a value that must be freed by
** invoking the external callback in Mem.xDel, then this routine
** will free that value.  It also sets Mem.flags to MEM_Null.
**
** This is a helper routine for sqlite3VdbeMemSetNull() and
** for sqlite3VdbeMemRelease().  Use those other routines as the
** entry point for releasing Mem resources.
*/
స్థిర SQLITE_NOINLINE శూన్య vdbe_Mem_Clear_Extern_And_సమితి_Null(Mem *p){
  ప్రతిజ్ఞా( p->db==0 || sqlite3_mutex_held(p->db->mutex) );
  ప్రతిజ్ఞా( VdbeMemDynamic(p) );
  ఒకవేళ( p->flags&MEM_Agg ){
    sqlite3VdbeMemFinalize(p, p->u.pDef);
    ప్రతిజ్ఞా( (p->flags & MEM_Agg)==0 );
    testcase( p->flags & MEM_Dyn );
  }
  ఒకవేళ( p->flags&MEM_Dyn ){
    ప్రతిజ్ఞా( p->xDel!=SQLITE_DYNAMIC && p->xDel!=0 );
    p->xDel((శూన్య *)p->z);
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
స్థిర SQLITE_NOINLINE శూన్య vdbeMemClear(Mem *p){
  ఒకవేళ( VdbeMemDynamic(p) ){
    vdbe_Mem_Clear_Extern_And_సమితి_Null(p);
  }
  ఒకవేళ( p->szMalloc ){
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
శూన్య sqlite3VdbeMemRelease(Mem *p){
  ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(p) );
  ఒకవేళ( VdbeMemDynamic(p) || p->szMalloc ){
    vdbeMemClear(p);
  }
}

/* Like sqlite3VdbeMemRelease() but faster for cases where we
** know in advance that the Mem is not MEM_Dyn or MEM_Agg.
*/
శూన్య sqlite3VdbeMemReleaseMalloc(Mem *p){
  ప్రతిజ్ఞా( !VdbeMemDynamic(p) );
  ఒకవేళ( p->szMalloc ) vdbeMemClear(p);
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
స్థిర SQLITE_NOINLINE i64 mem_Int_విలువ(స్థిరరాశి Mem *pMem){
  i64 విలువ = 0;
  sqlite3Atoi64(pMem->z, &విలువ, pMem->n, pMem->enc);
  తిరిగి విలువ;
}
i64 sqlite3_Vdbe_Int_విలువ(స్థిరరాశి Mem *pMem){
  పూర్ణాంకం flags;
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );
  flags = pMem->flags;
  ఒకవేళ( flags & (MEM_Int|MEM_IntReal) ){
    testcase( flags & MEM_IntReal );
    తిరిగి pMem->u.i;
  }లేకపోతే ఒకవేళ( flags & MEM_Real ){
    తిరిగి sqlite3_Real_గమ్యం_I64(pMem->u.r);
  }లేకపోతే ఒకవేళ( (flags & (MEM_Str|MEM_Blob))!=0 && pMem->z!=0 ){
    తిరిగి mem_Int_విలువ(pMem);
  }లేకపోతే{
    తిరిగి 0;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం sqlite3_Mem_Real_విలువ_RCSlow_మార్గం(
  Mem *pMem,
  ద్విగుణ *p_విలువ
){
  పూర్ణాంకం rc = SQLITE_OK;
  *p_విలువ = 0.0;
  ఒకవేళ( pMem->enc==SQLITE_UTF8 ){
    అక్షరం *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    ఒకవేళ( zCopy ){
      rc = sqlite3AtoF(zCopy, p_విలువ);
      sqlite3DbFree(pMem->db, zCopy);
    }
    తిరిగి rc;
  }లేకపోతే{
    పూర్ణాంకం n, i, j;
    అక్షరం *zCopy;
    స్థిరరాశి అక్షరం *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n/2 + 2);
    ఒకవేళ( zCopy ){
      z = pMem->z;
      ఒకవేళ( pMem->enc==SQLITE_UTF16LE ){
        కొరకు(i=j=0; i<n-1; i+=2, j++){
          zCopy[j] = z[i];
          ఒకవేళ( z[i+1]!=0 ) ఆపు;
        }
      }లేకపోతే{
        కొరకు(i=j=0; i<n-1; i+=2, j++){
          ఒకవేళ( z[i]!=0 ) ఆపు;
          zCopy[j] = z[i+1];
        }
      }     
      ప్రతిజ్ఞా( j<=n/2 );
      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, p_విలువ);
      ఒకవేళ( i<n ) rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    తిరిగి rc;
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
పూర్ణాంకం sqlite3_Mem_Real_విలువ_RC(Mem *pMem, ద్విగుణ *p_విలువ){
  testcase( pMem->db==0 );
  ప్రతిజ్ఞా( pMem->flags & (MEM_Str|MEM_Blob) );
  ఒకవేళ( pMem->z==0 ){
    *p_విలువ = 0.0;
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( pMem->enc==SQLITE_UTF8 
   && ((pMem->flags & MEM_పదం)!=0 || sqlite3VdbeMemZeroTerminateIfAble(pMem)) 
  ){
    తిరిగి sqlite3AtoF(pMem->z, p_విలువ);
  }లేకపోతే ఒకవేళ( pMem->n==0 ){
    *p_విలువ = 0.0;
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి sqlite3_Mem_Real_విలువ_RCSlow_మార్గం(pMem, p_విలువ);
  }
}

/*
** This routine acts as a bridge from sqlite3VdbeRealValue() to
** sqlite3VdbeRealValueRC, allowing sqlite3VdbeRealValue() to avoid
** stuffing values onto the stack.
*/
స్థిర SQLITE_NOINLINE ద్విగుణ sqlite3_Mem_Real_విలువ_No_RC(Mem *pMem){
  ద్విగుణ r;
  (శూన్య)sqlite3_Mem_Real_విలువ_RC(pMem, &r);
  తిరిగి r;
}

/*
** Return the best representation of pMem that we can get into a
** double.  If pMem is already a double or an integer, return its
** value.  If it is a string or blob, try to convert it to a double.
** If it is a NULL, return 0.0.
*/
ద్విగుణ sqlite3_Vdbe_Real_విలువ(Mem *pMem){
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );
  ఒకవేళ( pMem->flags & MEM_Real ){
    తిరిగి pMem->u.r;
  }లేకపోతే ఒకవేళ( pMem->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pMem->flags & MEM_IntReal );
    తిరిగి (ద్విగుణ)pMem->u.i;
  }లేకపోతే ఒకవేళ( pMem->flags & (MEM_Str|MEM_Blob) ){
    తిరిగి sqlite3_Mem_Real_విలువ_No_RC(pMem);
  }లేకపోతే{
    /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
    తిరిగి (ద్విగుణ)0;
  }
}

/*
** Return 1 if pMem represents true, and return 0 if pMem represents false.
** Return the value ifNull if pMem is NULL.  
*/
పూర్ణాంకం sqlite3_Vdbe_Boolean_విలువ(Mem *pMem, పూర్ణాంకం ifNull){
  testcase( pMem->flags & MEM_IntReal );
  ఒకవేళ( pMem->flags & (MEM_Int|MEM_IntReal) ) తిరిగి pMem->u.i!=0;
  ఒకవేళ( pMem->flags & MEM_Null ) తిరిగి ifNull;
  తిరిగి sqlite3_Vdbe_Real_విలువ(pMem)!=0.0;
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
శూన్య sqlite3VdbeIntegerAffinity(Mem *pMem, పూర్ణాంకం b_From_పాఠ్యం){
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->flags & (MEM_Real|MEM_IntReal) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );

  ఒకవేళ( pMem->flags & MEM_IntReal ){
    Mem_సమితి_Type_Flag(pMem, MEM_Int);
  }లేకపోతే{
    i64 ix = sqlite3_Real_గమ్యం_I64(pMem->u.r);

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
    ఒకవేళ( pMem->u.r==ix && ix<అతిపెద్ద_INT64
     && (!b_From_పాఠ్యం || ix>అతిచిన్న_INT64)
    ){
      pMem->u.i = ix;
      Mem_సమితి_Type_Flag(pMem, MEM_Int);
    }
  }
}

/*
** Convert pMem to type integer.  Invalidate any prior representations.
*/
పూర్ణాంకం sqlite3VdbeMemIntegerify(Mem *pMem){
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.i = sqlite3_Vdbe_Int_విలువ(pMem);
  Mem_సమితి_Type_Flag(pMem, MEM_Int);
  తిరిగి SQLITE_OK;
}

/*
** Convert pMem so that it is of type MEM_Real.
** Invalidate any prior representations.
*/
పూర్ణాంకం sqlite3VdbeMemRealify(Mem *pMem){
  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );

  pMem->u.r = sqlite3_Vdbe_Real_విలువ(pMem);
  Mem_సమితి_Type_Flag(pMem, MEM_Real);
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3RealSameAsInt(ద్విగుణ r1, sqlite3_int64 i){
  ద్విగుణ r2 = (ద్విగుణ)i;
  తిరిగి r1==0.0
      || (memcmp(&r1, &r2, పరిమాణం(r1))==0
          && i >= -2251799813685248LL && i < 2251799813685248LL);
}

/* Convert a floating point value to its closest integer.  Do so in
** a way that avoids 'outside the range of representable values' warnings
** from UBSAN.
*/
i64 sqlite3_Real_గమ్యం_I64(ద్విగుణ r){
  ఒకవేళ( r<-9223372036854774784.0 ) తిరిగి అతిచిన్న_INT64;
  ఒకవేళ( r>+9223372036854774784.0 ) తిరిగి అతిపెద్ద_INT64;
  తిరిగి (i64)r;
}

/*
** Convert pMem so that it has type MEM_Real or MEM_Int.
** Invalidate any prior representations.
**
** Every effort is made to force the conversion, even if the input
** is a string that does not look completely like a number.  Convert
** as much of the string as we can and ignore the rest.
*/
పూర్ణాంకం sqlite3VdbeMemNumerify(Mem *pMem){
  ప్రతిజ్ఞా( pMem!=0 );
  testcase( pMem->flags & MEM_Int );
  testcase( pMem->flags & MEM_Real );
  testcase( pMem->flags & MEM_IntReal );
  testcase( pMem->flags & MEM_Null );
  ఒకవేళ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))==0 ){
    పూర్ణాంకం rc;
    sqlite3_int64 ix;
    ప్రతిజ్ఞా( (pMem->flags & (MEM_Blob|MEM_Str))!=0 );
    ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
    rc = sqlite3_Mem_Real_విలువ_RC(pMem, &pMem->u.r);
    ఒకవేళ( ((rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<2)
     || sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3_Real_గమ్యం_I64(pMem->u.r)))
    ){
      pMem->u.i = ix;
      Mem_సమితి_Type_Flag(pMem, MEM_Int);
    }లేకపోతే{
      Mem_సమితి_Type_Flag(pMem, MEM_Real);
    }
  }
  ప్రతిజ్ఞా( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null))!=0 );
  pMem->flags &= ~(MEM_Str|MEM_Blob|MEM_Zero);
  తిరిగి SQLITE_OK;
}

/*
** Cast the datatype of the value in pMem according to the affinity
** "aff".  Casting is different from applying affinity in that a cast
** is forced.  In other words, the value is converted into the desired
** affinity even if that results in loss of data.  This routine is
** used (for example) to implement the SQL "cast()" operator.
*/
పూర్ణాంకం sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding){
  ఒకవేళ( pMem->flags & MEM_Null ) తిరిగి SQLITE_OK;
  ఎంపిక( aff ){
    స్థితి SQLITE_AFF_BLOB: {   /* Really a cast to BLOB */
      ఒకవేళ( (pMem->flags & MEM_Blob)==0 ){
        sqlite3_విలువ_Apply_Affinity(pMem, SQLITE_AFF_పాఠ్యం, encoding);
        ప్రతిజ్ఞా( pMem->flags & MEM_Str || pMem->db->mallocFailed );
        ఒకవేళ( pMem->flags & MEM_Str ) Mem_సమితి_Type_Flag(pMem, MEM_Blob);
      }లేకపోతే{
        pMem->flags &= ~(MEM_Type_ముసుగు&~MEM_Blob);
      }
      ఆపు;
    }
    స్థితి SQLITE_AFF_NUMERIC: {
      sqlite3VdbeMemNumerify(pMem);
      ఆపు;
    }
    స్థితి SQLITE_AFF_INTEGER: {
      sqlite3VdbeMemIntegerify(pMem);
      ఆపు;
    }
    స్థితి SQLITE_AFF_REAL: {
      sqlite3VdbeMemRealify(pMem);
      ఆపు;
    }
    అప్రమేయ: {
      పూర్ణాంకం rc;
      ప్రతిజ్ఞా( aff==SQLITE_AFF_పాఠ్యం );
      ప్రతిజ్ఞా( MEM_Str==(MEM_Blob>>3) );
      pMem->flags |= (pMem->flags&MEM_Blob)>>3;
      sqlite3_విలువ_Apply_Affinity(pMem, SQLITE_AFF_పాఠ్యం, encoding);
      ప్రతిజ్ఞా( pMem->flags & MEM_Str || pMem->db->mallocFailed );
      pMem->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal|MEM_Blob|MEM_Zero);
      ఒకవేళ( encoding!=SQLITE_UTF8 ) pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      ఒకవేళ( rc ) తిరిగి rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  తిరిగి SQLITE_OK;
}

/*
** Initialize bulk memory to be a consistent Mem object.
**
** The minimum amount of initialization feasible is performed.
*/
శూన్య sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags){
  ప్రతిజ్ఞా( (flags & ~MEM_Type_ముసుగు)==0 );
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
శూన్య sqlite3_Vdbe_Mem_సమితి_Null(Mem *pMem){
  ఒకవేళ( VdbeMemDynamic(pMem) ){
    vdbe_Mem_Clear_Extern_And_సమితి_Null(pMem);
  }లేకపోతే{
    pMem->flags = MEM_Null;
  }
}
శూన్య sqlite3_విలువ_సమితి_Null(sqlite3_విలువ *p){
  sqlite3_Vdbe_Mem_సమితి_Null((Mem*)p); 
}

/*
** Delete any previous value and set the value to be a BLOB of length
** n containing all zeros.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
శూన్య sqlite3_Vdbe_Mem_సమితి_Zero_Blob(Mem *pMem, పూర్ణాంకం n){
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Blob|MEM_Zero;
  pMem->n = 0;
  ఒకవేళ( n<0 ) n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}
#లేకపోతే
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_Zero_Blob(Mem *pMem, పూర్ణాంకం n){
  పూర్ణాంకం nByte = n>0?n:1;
  ఒకవేళ( sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  ప్రతిజ్ఞా( pMem->z!=0 );
  ప్రతిజ్ఞా( sqlite3_Db_Malloc_కొలత(pMem->db, pMem->z)>=nByte );
  memset(pMem->z, 0, nByte);
  pMem->n = n>0?n:0;
  pMem->flags = MEM_Blob;
  pMem->enc = SQLITE_UTF8;
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

/*
** The pMem is known to contain content that needs to be destroyed prior
** to a value change.  So invoke the destructor, then set the value to
** a 64-bit integer.
*/
స్థిర SQLITE_NOINLINE శూన్య vdbe_Release_And_సమితి_Int64(Mem *pMem, i64 val){
  sqlite3_Vdbe_Mem_సమితి_Null(pMem);
  pMem->u.i = val;
  pMem->flags = MEM_Int;
}

/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type INTEGER.
*/
శూన్య sqlite3_Vdbe_Mem_సమితి_Int64(Mem *pMem, i64 val){
  ఒకవేళ( VdbeMemDynamic(pMem) ){
    vdbe_Release_And_సమితి_Int64(pMem, val);
  }లేకపోతే{
    pMem->u.i = val;
    pMem->flags = MEM_Int;
  }
}

/*
** Set the iIdx'th entry of array aMem[] to contain integer value val.
*/
శూన్య sqlite3_Mem_సమితి_Array_Int64(sqlite3_విలువ *aMem, పూర్ణాంకం iIdx, i64 val){
  sqlite3_Vdbe_Mem_సమితి_Int64(&aMem[iIdx], val);
}

/* A no-op destructor */
శూన్య sqlite3NoopDestructor(శూన్య *p){ వాడని_PARAMETER(p); }

/*
** Set the value stored in *pMem should already be a NULL.
** Also store a pointer to go with it.
*/
శూన్య sqlite3_Vdbe_Mem_సమితి_సూచిక(
  Mem *pMem,
  శూన్య *pPtr,
  స్థిరరాశి అక్షరం *zPType,
  శూన్య (*xDestructor)(శూన్య*)
){
  ప్రతిజ్ఞా( pMem->flags==MEM_Null );
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = MEM_Null|MEM_Dyn|MEM_Subtype|MEM_పదం;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
/*
** Delete any previous value and set the value stored in *pMem to val,
** manifest type REAL.
*/
శూన్య sqlite3_Vdbe_Mem_సమితి_Double(Mem *pMem, ద్విగుణ val){
  sqlite3_Vdbe_Mem_సమితి_Null(pMem);
  ఒకవేళ( !sqlite3IsNaN(val) ){
    pMem->u.r = val;
    pMem->flags = MEM_Real;
  }
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Return true if the Mem holds a RowSet object.  This routine is intended
** for use inside of assert() statements.
*/
పూర్ణాంకం sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(స్థిరరాశి Mem *pMem){
  తిరిగి (pMem->flags&(MEM_Blob|MEM_Dyn))==(MEM_Blob|MEM_Dyn)
         && pMem->xDel==sqlite3_అడ్డువరుస_సమితి_Delete;
}
#అయితే_ముగింపు

/*
** Delete any previous value and set the value of pMem to be an
** empty boolean index.
**
** Return SQLITE_OK on success and SQLITE_NOMEM if a memory allocation
** error occurs.
*/
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_అడ్డువరుస_సమితి(Mem *pMem){
  sqlite3 *db = pMem->db;
  అడ్డువరుస_సమితి *p;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  sqlite3VdbeMemRelease(pMem);
  p = sqlite3_అడ్డువరుస_సమితి_Init(db);
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM;
  pMem->z = (అక్షరం*)p;
  pMem->flags = MEM_Blob|MEM_Dyn;
  pMem->xDel = sqlite3_అడ్డువరుస_సమితి_Delete;
  తిరిగి SQLITE_OK;
}

/*
** Return true if the Mem object contains a TEXT or BLOB that is
** too large - whose size exceeds SQLITE_MAX_LENGTH.
*/
పూర్ణాంకం sqlite3VdbeMemTooBig(Mem *p){
  ప్రతిజ్ఞా( p->db!=0 );
  ఒకవేళ( p->flags & (MEM_Str|MEM_Blob) ){
    పూర్ణాంకం n = p->n;
    ఒకవేళ( p->flags & MEM_Zero ){
      n += p->u.nZero;
    }
    తిరిగి n>p->db->a_హద్దు[SQLITE_హద్దు_నిడివి];
  }
  తిరిగి 0; 
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** This routine prepares a memory cell for modification by breaking
** its link to a shallow copy and by marking any current shallow
** copies of this cell as invalid.
**
** This is used for testing and debugging only - to help ensure that shallow
** copies (created by OP_SCopy) are not misused.
*/
శూన్య sqlite3_Vdbe_Mem_About_గమ్యం_Change(Vdbe *pVdbe, Mem *pMem){
  పూర్ణాంకం i;
  Mem *pX;
  ఒకవేళ( pMem->bScopy ){
    కొరకు(i=1, pX=pVdbe->aMem+1; i<pVdbe->nMem; i++, pX++){
      ఒకవేళ( pX->pScopyFrom==pMem ){
        u16 mFlags;
        ఒకవేళ( pVdbe->db->flags & SQLITE_VdbeTrace ){
          sqlite3DebugPrintf("Invalidate R[%d] due to change in R[%d]\n",
            (పూర్ణాంకం)(pX - pVdbe->aMem), (పూర్ణాంకం)(pMem - pVdbe->aMem));
        }
        /* If pX is marked as a shallow copy of pMem, then try to verify that
        ** no significant changes have been made to pX since the OP_SCopy.
        ** A significant change would indicated a missed call to this
        ** function for pX.  Minor changes, such as adding or removing a
        ** dual type, are allowed, as long as the underlying value is the
        ** same. */
        mFlags = pMem->flags & pX->flags & pX->mScopyFlags;
        ప్రతిజ్ఞా( (mFlags&(MEM_Int|MEM_IntReal))==0 || pMem->u.i==pX->u.i );
        
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
#అయితే_ముగింపు /* SQLITE_DEBUG */

/*
** Make an shallow copy of pFrom into pTo.  Prior contents of
** pTo are freed.  The pFrom->z field is not duplicated.  If
** pFrom->z is used, then pTo->z points to the same thing as pFrom->z
** and flags gets srcType (either MEM_Ephem or MEM_Static).
*/
స్థిర SQLITE_NOINLINE శూన్య vdbeClrCopy(Mem *p_గమ్యం, స్థిరరాశి Mem *pFrom, పూర్ణాంకం eType){
  vdbe_Mem_Clear_Extern_And_సమితి_Null(p_గమ్యం);
  ప్రతిజ్ఞా( !VdbeMemDynamic(p_గమ్యం) );
  sqlite3VdbeMemShallowCopy(p_గమ్యం, pFrom, eType);
}
శూన్య sqlite3VdbeMemShallowCopy(Mem *p_గమ్యం, స్థిరరాశి Mem *pFrom, పూర్ణాంకం srcType){
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pFrom) );
  ప్రతిజ్ఞా( p_గమ్యం->db==pFrom->db );
  ఒకవేళ( VdbeMemDynamic(p_గమ్యం) ){ vdbeClrCopy(p_గమ్యం,pFrom,srcType); తిరిగి; }
  జ్ఞాపకం_నకలు(p_గమ్యం, pFrom, MEMCELLSIZE);
  ఒకవేళ( (pFrom->flags&MEM_Static)==0 ){
    p_గమ్యం->flags &= ~(MEM_Dyn|MEM_Static|MEM_Ephem);
    ప్రతిజ్ఞా( srcType==MEM_Ephem || srcType==MEM_Static );
    p_గమ్యం->flags |= srcType;
  }
}

/*
** Make a full copy of pFrom into pTo.  Prior contents of pTo are
** freed before the copy is made.
*/
పూర్ణాంకం sqlite3VdbeMemCopy(Mem *p_గమ్యం, స్థిరరాశి Mem *pFrom){
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pFrom) );
  ఒకవేళ( VdbeMemDynamic(p_గమ్యం) ) vdbe_Mem_Clear_Extern_And_సమితి_Null(p_గమ్యం);
  జ్ఞాపకం_నకలు(p_గమ్యం, pFrom, MEMCELLSIZE);
  p_గమ్యం->flags &= ~MEM_Dyn;
  ఒకవేళ( p_గమ్యం->flags&(MEM_Str|MEM_Blob) ){
    ఒకవేళ( 0==(pFrom->flags&MEM_Static) ){
      p_గమ్యం->flags |= MEM_Ephem;
      rc = sqlite3VdbeMemMakeWriteable(p_గమ్యం);
    }
  }

  తిరిగి rc;
}

/*
** Transfer the contents of pFrom to pTo. Any existing value in pTo is
** freed. If pFrom contains ephemeral data, a copy is made.
**
** pFrom contains an SQL NULL when this routine returns.
*/
శూన్య sqlite3VdbeMemMove(Mem *p_గమ్యం, Mem *pFrom){
  ప్రతిజ్ఞా( pFrom->db==0 || sqlite3_mutex_held(pFrom->db->mutex) );
  ప్రతిజ్ఞా( p_గమ్యం->db==0 || sqlite3_mutex_held(p_గమ్యం->db->mutex) );
  ప్రతిజ్ఞా( pFrom->db==0 || p_గమ్యం->db==0 || pFrom->db==p_గమ్యం->db );

  sqlite3VdbeMemRelease(p_గమ్యం);
  జ్ఞాపకం_నకలు(p_గమ్యం, pFrom, పరిమాణం(Mem));
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
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_Str(
  Mem *pMem,          /* Memory cell to set to string value */
  స్థిరరాశి అక్షరం *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  u8 enc,             /* Encoding of z.  0 for BLOBs */
  శూన్య (*xDel)(శూన్య*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  పూర్ణాంకం i_హద్దు;         /* Maximum allowed string or blob size */
  u16 flags;          /* New value for pMem->flags */

  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  ప్రతిజ్ఞా( enc!=0 || n>=0 );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ఒకవేళ( !z ){
    sqlite3_Vdbe_Mem_సమితి_Null(pMem);
    తిరిగి SQLITE_OK;
  }

  ఒకవేళ( pMem->db ){
    i_హద్దు = pMem->db->a_హద్దు[SQLITE_హద్దు_నిడివి];
  }లేకపోతే{
    i_హద్దు = SQLITE_MAX_నిడివి;
  }
  ఒకవేళ( nByte<0 ){
    ప్రతిజ్ఞా( enc!=0 );
    ఒకవేళ( enc==SQLITE_UTF8 ){
      nByte = అక్షరశ్రేణి_పొడవు(z);
    }లేకపోతే{
      కొరకు(nByte=0; nByte<=i_హద్దు && (z[nByte] | z[nByte+1]); nByte+=2){}
    }
    flags= MEM_Str|MEM_పదం;
  }లేకపోతే ఒకవేళ( enc==0 ){
    flags = MEM_Blob;
    enc = SQLITE_UTF8;
  }లేకపోతే{
    flags = MEM_Str;
  }
  ఒకవేళ( nByte>i_హద్దు ){
    ఒకవేళ( xDel && xDel!=SQLITE_TRANSIENT ){
      ఒకవేళ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (శూన్య*)z);
      }లేకపోతే{
        xDel((శూన్య*)z);
      }
    }
    sqlite3_Vdbe_Mem_సమితి_Null(pMem);
    తిరిగి sqlite3_Error_గమ్యం_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ఒకవేళ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte;
    ఒకవేళ( flags&MEM_పదం ){
      nAlloc += (enc==SQLITE_UTF8?1:2);
    }
    testcase( nAlloc==0 );
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ఒకవేళ( sqlite3VdbeMemClearAndResize(pMem, (పూర్ణాంకం)MAX(nAlloc,32)) ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    ప్రతిజ్ఞా( pMem->z!=0 );
    జ్ఞాపకం_నకలు(pMem->z, z, nAlloc);
  }లేకపోతే{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (అక్షరం *)z;
    ఒకవేళ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_కొలత(pMem->db, pMem->zMalloc);
    }లేకపోతే{
      pMem->xDel = xDel;
      flags |= ((xDel==SQLITE_STATIC)?MEM_Static:MEM_Dyn);
    }
  }

  pMem->n = (పూర్ణాంకం)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  /* tag-20261016-1:  To retain the BOM, use enc==SQLITE_UTF8, even
  ** for UTF-16 text, and then set pMem->enc correctly after this
  ** function returns.  Note, however, that n must be non-negative
  ** for this trick to work. */
  ఒకవేళ( enc>SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem) ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
#అయితే_ముగింపు


  తిరిగి SQLITE_OK;
}

/* Like sqlite3VdbeMemSetStr() except:
**
**   enc is always SQLITE_UTF8
**   pMem->db is always non-NULL
*/
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(
  Mem *pMem,          /* Memory cell to set to string value */
  స్థిరరాశి అక్షరం *z,      /* String pointer */
  i64 n,              /* Bytes in string, or negative */
  శూన్య (*xDel)(శూన్య*) /* Destructor function */
){
  i64 nByte = n;      /* New value for pMem->n */
  u16 flags;

  ప్రతిజ్ఞా( pMem!=0 );
  ప్రతిజ్ఞా( pMem->db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );

  /* If z is a NULL pointer, set pMem to contain an SQL NULL. */
  ఒకవేళ( !z ){
    sqlite3_Vdbe_Mem_సమితి_Null(pMem);
    తిరిగి SQLITE_OK;
  }

  ఒకవేళ( nByte<0 ){
    nByte = అక్షరశ్రేణి_పొడవు(z);
    flags = MEM_Str|MEM_పదం;
  }లేకపోతే{
    flags = MEM_Str;
  }
  ఒకవేళ( nByte>(i64)pMem->db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    ఒకవేళ( xDel && xDel!=SQLITE_TRANSIENT ){
      ఒకవేళ( xDel==SQLITE_DYNAMIC ){
        sqlite3DbFree(pMem->db, (శూన్య*)z);
      }లేకపోతే{
        xDel((శూన్య*)z);
      }
    }
    sqlite3_Vdbe_Mem_సమితి_Null(pMem);
    తిరిగి sqlite3_Error_గమ్యం_Parser(pMem->db, SQLITE_TOOBIG);
  }

  /* The following block sets the new values of Mem.z and Mem.xDel. It
  ** also sets a flag in local variable "flags" to indicate the memory
  ** management (one of MEM_Dyn or MEM_Static).
  */
  ఒకవేళ( xDel==SQLITE_TRANSIENT ){
    i64 nAlloc = nByte + 1;
    testcase( nAlloc==31 );
    testcase( nAlloc==32 );
    ఒకవేళ( sqlite3VdbeMemClearAndResize(pMem, (పూర్ణాంకం)MAX(nAlloc,32)) ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    ప్రతిజ్ఞా( pMem->z!=0 );
    జ్ఞాపకం_నకలు(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  }లేకపోతే{
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (అక్షరం *)z;
    ఒకవేళ( xDel==SQLITE_DYNAMIC ){
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3_Db_Malloc_కొలత(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    }లేకపోతే ఒకవేళ( xDel==SQLITE_STATIC ){
      pMem->xDel = xDel;
      flags |= MEM_Static;
    }లేకపోతే{
      pMem->xDel = xDel;
      flags |= MEM_Dyn;
    }
  }
  pMem->flags = flags;
  pMem->n = (పూర్ణాంకం)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3VdbeMemFromBtree(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 స్థానభ్రంశం,       /* Offset from the start of data to return bytes from. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  పూర్ణాంకం rc;
  pMem->flags = MEM_Null;
  testcase( amt==SQLITE_MAX_కేటాయింపు_కొలత-1 );
  testcase( amt==SQLITE_MAX_కేటాయింపు_కొలత );
  ఒకవేళ( amt>=SQLITE_MAX_కేటాయింపు_కొలత ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  ఒకవేళ( (u64)amt + (u64)స్థానభ్రంశం > (u64)sqlite3_Btree_Max_రికార్డు_కొలత(pCur) ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  ఒకవేళ( SQLITE_OK==(rc = sqlite3VdbeMemClearAndResize(pMem, amt+8)) ){
    rc = sqlite3BtreePayload(pCur, స్థానభ్రంశం, amt, pMem->z);
    ఒకవేళ( rc==SQLITE_OK ){
      memset(pMem->z+amt,0,8);  /* Overrun area for malformed records */
      pMem->flags = MEM_Blob;
      pMem->n = (పూర్ణాంకం)amt;
    }లేకపోతే{
      sqlite3VdbeMemRelease(pMem);
    }
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(
  BtCursor *pCur,   /* Cursor pointing at record to retrieve. */
  u32 amt,          /* Number of bytes to return. */
  Mem *pMem         /* OUT: Return data in this Mem structure. */
){
  u32 అందుబాటు = 0;  /* Number of bytes available on the local btree page */
  పూర్ణాంకం rc = SQLITE_OK; /* Return code */

  ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pCur) );
  ప్రతిజ్ఞా( !VdbeMemDynamic(pMem) );

  /* Note: the calls to BtreeKeyFetch() and DataFetch() below assert() 
  ** that both the BtShared and database handle mutexes are held. */
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem) );
  pMem->z = (అక్షరం *)sqlite3BtreePayloadFetch(pCur, &అందుబాటు);
  ప్రతిజ్ఞా( pMem->z!=0 );

  ఒకవేళ( amt<=అందుబాటు ){
    pMem->flags = MEM_Blob|MEM_Ephem;
    pMem->n = (పూర్ణాంకం)amt;
  }లేకపోతే{
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  తిరిగి rc;
}

/*
** The pVal argument is known to be a value other than NULL.
** Convert it into a string with encoding enc and return a pointer
** to a zero-terminated version of that string.
*/
స్థిర SQLITE_NOINLINE స్థిరరాశి శూన్య *విలువ_గమ్యం_పాఠ్యం(sqlite3_విలువ* pVal, u8 enc){
  ప్రతిజ్ఞా( pVal!=0 );
  ప్రతిజ్ఞా( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ప్రతిజ్ఞా( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pVal) );
  ప్రతిజ్ఞా( (pVal->flags & (MEM_Null))==0 );
  ఒకవేళ( pVal->flags & (MEM_Blob|MEM_Str) ){
    ఒకవేళ( ExpandBlob(pVal) ) తిరిగి 0;
    pVal->flags |= MEM_Str;
    ఒకవేళ( pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED) ){
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    ఒకవేళ( (enc & SQLITE_UTF16_ALIGNED)!=0 && 1==(1&SQLITE_PTR_గమ్యం_INT(pVal->z)) ){
      ప్రతిజ్ఞా( (pVal->flags & (MEM_Ephem|MEM_Static))!=0 );
      ఒకవేళ( sqlite3VdbeMemMakeWriteable(pVal)!=SQLITE_OK ){
        తిరిగి 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal); /* IMP: R-31275-44060 */
  }లేకపోతే{
    sqlite3VdbeMemStringify(pVal, enc, 0);
    ప్రతిజ్ఞా( 0==(1&SQLITE_PTR_గమ్యం_INT(pVal->z)) );
  }
  ప్రతిజ్ఞా(pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) || pVal->db==0
              || pVal->db->mallocFailed );
  ఒకవేళ( pVal->enc==(enc & ~SQLITE_UTF16_ALIGNED) ){
    ప్రతిజ్ఞా( sqlite3VdbeMemValidStrRep(pVal) );
    తిరిగి pVal->z;
  }లేకపోతే{
    తిరిగి 0;
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
స్థిరరాశి శూన్య *sqlite3_విలువ_పాఠ్యం_2(sqlite3_విలువ* pVal, u8 enc){
  ఒకవేళ( !pVal ) తిరిగి 0;
  ప్రతిజ్ఞా( pVal->db==0 || sqlite3_mutex_held(pVal->db->mutex) );
  ప్రతిజ్ఞా( (enc&3)==(enc&~SQLITE_UTF16_ALIGNED) );
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pVal) );
  ఒకవేళ( (pVal->flags&(MEM_Str|MEM_పదం))==(MEM_Str|MEM_పదం) && pVal->enc==enc ){
    ప్రతిజ్ఞా( sqlite3VdbeMemValidStrRep(pVal) );
    తిరిగి pVal->z;
  }
  ఒకవేళ( pVal->flags&MEM_Null ){
    తిరిగి 0;
  }
  తిరిగి విలువ_గమ్యం_పాఠ్యం(pVal, enc);
}

/* Return true if sqlit3_value object pVal is a string or blob value
** that uses the destructor specified in the second argument.
**
** TODO:  Maybe someday promote this interface into a published API so
** that third-party extensions can get access to it?
*/
పూర్ణాంకం sqlite3_విలువ_Is_యొక్క_Class(స్థిరరాశి sqlite3_విలువ *pVal, శూన్య(*xFree)(శూన్య*)){
  ఒకవేళ( ALWAYS(pVal!=0)
   && ALWAYS((pVal->flags & (MEM_Str|MEM_Blob))!=0)
   && (pVal->flags & MEM_Dyn)!=0
   && pVal->xDel==xFree
  ){
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి 0;
  }
}

/*
** Create a new sqlite3_value object.
*/
sqlite3_విలువ *sqlite3_విలువ_New(sqlite3 *db){
  Mem *p = sqlite3DbMallocZero(db, పరిమాణం(*p));
  ఒకవేళ( p ){
    p->flags = MEM_Null;
    p->db = db;
  }
  తిరిగి p;
}

/*
** Context object passed by sqlite3Stat4ProbeSetValue() through to 
** valueNew(). See comments above valueNew() for details.
*/
నిర్మాణం విలువ_New_Stat4_Ctx {
  Parse *pParse;
  సూచీ *pIdx;
  Unpacked_రికార్డు **ppRec;
  పూర్ణాంకం iVal;
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
స్థిర sqlite3_విలువ *విలువ_New(sqlite3 *db, నిర్మాణం విలువ_New_Stat4_Ctx *p){
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( p ){
    Unpacked_రికార్డు *pRec = p->ppRec[0];

    ఒకవేళ( pRec==0 ){
      సూచీ *pIdx = p->pIdx;      /* Index being probed */
      i64 nByte;                  /* Bytes of space to allocate */
      పూర్ణాంకం i;                      /* Counter variable */
      పూర్ణాంకం n_నిలువు = pIdx->n_స్తంభం;   /* Number of index columns including rowid */
  
      nByte = పరిమాణం(Mem) * n_నిలువు + ROUND8(పరిమాణం(Unpacked_రికార్డు));
      pRec = (Unpacked_రికార్డు*)sqlite3DbMallocZero(db, nByte);
      ఒకవేళ( pRec ){
        pRec->p_కీలకం_Info = sqlite3_కీలకం_Info_యొక్క_సూచీ(p->pParse, pIdx);
        ఒకవేళ( pRec->p_కీలకం_Info ){
          ప్రతిజ్ఞా( pRec->p_కీలకం_Info->n_All_క్షేత్రం==n_నిలువు );
          ప్రతిజ్ఞా( pRec->p_కీలకం_Info->enc==ENC(db) );
          pRec->aMem = (Mem *)((u8*)pRec + ROUND8(పరిమాణం(Unpacked_రికార్డు)));
          కొరకు(i=0; i<n_నిలువు; i++){
            pRec->aMem[i].flags = MEM_Null;
            pRec->aMem[i].db = db;
          }
        }లేకపోతే{
          sqlite3DbFreeNN(db, pRec);
          pRec = 0;
        }
      }
      ఒకవేళ( pRec==0 ) తిరిగి 0;
      p->ppRec[0] = pRec;
    }
  
    pRec->n_క్షేత్రం = p->iVal+1;
    sqlite3_Vdbe_Mem_సమితి_Null(&pRec->aMem[p->iVal]);
    తిరిగి &pRec->aMem[p->iVal];
  }
#లేకపోతే
  వాడని_PARAMETER(p);
#అయితే_ముగింపు /* defined(SQLITE_ENABLE_STAT4) */
  తిరిగి sqlite3_విలువ_New(db);
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
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
స్థిర పూర్ణాంకం విలువ_From_Function(
  sqlite3 *db,                    /* The database connection */
  స్థిరరాశి సమాసం *p,                  /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 aff,                         /* Affinity to use */
  sqlite3_విలువ **ppVal,          /* Write the new value here */
  నిర్మాణం విలువ_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  sqlite3_context ctx;            /* Context object for function invocation */
  sqlite3_విలువ **apVal = 0;      /* Function arguments */
  పూర్ణాంకం nVal = 0;                   /* Number of function arguments */
  FuncDef *pFunc = 0;             /* Function definition */
  sqlite3_విలువ *pVal = 0;        /* New value */
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  సమాసం_List *pList = 0;            /* Function arguments */
  పూర్ణాంకం i;                          /* Iterator variable */

  ప్రతిజ్ఞా( pCtx!=0 );
  ప్రతిజ్ఞా( (p->flags & EP_TokenOnly)==0 );
  ప్రతిజ్ఞా( సమాసం_Use_XList(p) );
  pList = p->x.pList;
  ఒకవేళ( pList ) nVal = pList->n_సమాసం;
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_Int_విలువ) );
  pFunc = sqlite3_కనుగొను_Function(db, p->u.zToken, nVal, enc, 0);
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ఒకవేళ( pFunc==0 ) తిరిగి SQLITE_OK;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pFunc );
  ఒకవేళ( (pFunc->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || (pFunc->funcFlags & (SQLITE_FUNC_NEEDCOLL|SQLITE_FUNC_RUNONLY))!=0
  ){
    తిరిగి SQLITE_OK;
  }

  ఒకవేళ( pList ){
    apVal = (sqlite3_విలువ**)sqlite3DbMallocZero(db, పరిమాణం(apVal[0]) * nVal);
    ఒకవేళ( apVal==0 ){
      rc = SQLITE_NOMEM_BKPT;
      వెళ్ళు విలువ_from_function_బహిర్గతం;
    }
    కొరకు(i=0; i<nVal; i++){
      rc = sqlite3_Stat4_విలువ_From_సమాసం(pCtx->pParse, pList->a[i].p_సమాసం, aff,
                                     &apVal[i]);
      ఒకవేళ( apVal[i]==0 || rc!=SQLITE_OK ) వెళ్ళు విలువ_from_function_బహిర్గతం;
    }
  }

  pVal = విలువ_New(db, pCtx);
  ఒకవేళ( pVal==0 ){
    rc = SQLITE_NOMEM_BKPT;
    వెళ్ళు విలువ_from_function_బహిర్గతం;
  }

  memset(&ctx, 0, పరిమాణం(ctx));
  ctx.p_బహిర్గతం = pVal;
  ctx.pFunc = pFunc;
  ctx.enc = ENC(db);
  pFunc->xSFunc(&ctx, nVal, apVal);
  ఒకవేళ( ctx.isError ){
    rc = ctx.isError;
    sqlite3_Error_సందేశం(pCtx->pParse, "%s", sqlite3_విలువ_పాఠ్యం(pVal));
  }లేకపోతే{
    sqlite3_విలువ_Apply_Affinity(pVal, aff, SQLITE_UTF8);
    ప్రతిజ్ఞా( rc==SQLITE_OK );
    rc = sqlite3VdbeChangeEncoding(pVal, enc);
    ఒకవేళ( NEVER(rc==SQLITE_OK && sqlite3VdbeMemTooBig(pVal)) ){
      rc = SQLITE_TOOBIG;
      pCtx->pParse->nErr++;
    }
  }

 విలువ_from_function_బహిర్గతం:
  ఒకవేళ( rc!=SQLITE_OK ){
    pVal = 0;
    pCtx->pParse->rc = rc;
  }
  ఒకవేళ( apVal ){
    కొరకు(i=0; i<nVal; i++){
      sqlite3_విలువ_Free(apVal[i]);
    }
    sqlite3DbFreeNN(db, apVal);
  }

  *ppVal = pVal;
  తిరిగి rc;
}
#లేకపోతే
# define విలువ_From_Function(a,b,c,d,e,f) SQLITE_OK
#అయితే_ముగింపు /* defined(SQLITE_ENABLE_STAT4) */

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
స్థిర పూర్ణాంకం విలువ_From_సమాసం(
  sqlite3 *db,                    /* The database connection */
  స్థిరరాశి సమాసం *p_సమాసం,              /* The expression to evaluate */
  u8 enc,                         /* Encoding to use */
  u8 affinity,                    /* Affinity to use */
  sqlite3_విలువ **ppVal,          /* Write the new value here */
  నిర్మాణం విలువ_New_Stat4_Ctx *pCtx   /* Second argument for valueNew() */
){
  పూర్ణాంకం op;
  అక్షరం *zVal = 0;
  sqlite3_విలువ *pVal = 0;
  పూర్ణాంకం negInt = 1;
  స్థిరరాశి అక్షరం *zNeg = "";
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( p_సమాసం!=0 );
  అయితే( (op = p_సమాసం->op)==TK_UPLUS || op==TK_పరిధి ) p_సమాసం = p_సమాసం->p_ఎడమ;
  ఒకవేళ( op==TK_రిజిస్టరు ) op = p_సమాసం->op2;

  /* Compressed expressions only appear when parsing the DEFAULT clause
  ** on a table column definition, and hence only when pCtx==0.  This
  ** check ensures that an EP_TokenOnly expression is never passed down
  ** into valueFromFunction(). */
  ప్రతిజ్ఞా( (p_సమాసం->flags & EP_TokenOnly)==0 || pCtx==0 );

  ఒకవేళ( op==TK_CAST ){
    u8 aff;
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
    aff = sqlite3AffinityType(p_సమాసం->u.zToken,0);
    rc = విలువ_From_సమాసం(db, p_సమాసం->p_ఎడమ, enc, aff, ppVal, pCtx);
    testcase( rc!=SQLITE_OK );
    ఒకవేళ( *ppVal ){
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
      rc = ExpandBlob(*ppVal);
#లేకపోతే
      /* zero-blobs only come from functions, not literal values.  And
      ** functions are only processed under STAT4 */
      ప్రతిజ్ఞా( (ppVal[0][0].flags & MEM_Zero)==0 );
#అయితే_ముగింపు
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3_విలువ_Apply_Affinity(*ppVal, affinity, enc);
    }
    తిరిగి rc;
  }

  /* Handle negative integers in a single step.  This is needed in the
  ** case when the value is -9223372036854775808. Except - do not do this
  ** for hexadecimal literals.  */
  ఒకవేళ( op==TK_UMINUS ){
    సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
    ఒకవేళ( (p_ఎడమ->op==TK_INTEGER || p_ఎడమ->op==TK_FLOAT) ){
      ఒకవేళ( సమాసం_Has_Property(p_ఎడమ, EP_Int_విలువ)
       || p_ఎడమ->u.zToken[0]!='0' || (p_ఎడమ->u.zToken[1] & ~0x20)!='X'
      ){
        p_సమాసం = p_ఎడమ;
        op = p_సమాసం->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  ఒకవేళ( op==TK_STRING || op==TK_FLOAT || op==TK_INTEGER ){
    pVal = విలువ_New(db, pCtx);
    ఒకవేళ( pVal==0 ) వెళ్ళు no_mem;
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) ){
      sqlite3_Vdbe_Mem_సమితి_Int64(pVal, (i64)p_సమాసం->u.i_విలువ*negInt);
    }లేకపోతే{
      i64 iVal;
      ఒకవేళ( op==TK_INTEGER && 0==sqlite3_Dec_Or_Hex_గమ్యం_I64(p_సమాసం->u.zToken, &iVal) ){
        sqlite3_Vdbe_Mem_సమితి_Int64(pVal, iVal*negInt);
      }లేకపోతే{
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, p_సమాసం->u.zToken);
        ఒకవేళ( zVal==0 ) వెళ్ళు no_mem;
        sqlite3_విలువ_సమితి_Str(pVal, -1, zVal, SQLITE_UTF8, SQLITE_DYNAMIC);
        ఒకవేళ( op==TK_FLOAT ){
          ప్రతిజ్ఞా( pVal && pVal->z && pVal->flags==(MEM_Str|MEM_పదం) );
          sqlite3AtoF(pVal->z, &pVal->u.r);
          pVal->flags = MEM_Real;
        }
      }
    }
    ఒకవేళ( affinity==SQLITE_AFF_BLOB ){
      ఒకవేళ( op==TK_INTEGER ){
        /* This case is required by -9223372036854775808 and other strings
        ** that look like integers but cannot be handled by the
        ** sqlite3DecOrHexToI64() call above.  */
        sqlite3_విలువ_Apply_Affinity(pVal, SQLITE_AFF_NUMERIC, SQLITE_UTF8);
      }
    }లేకపోతే{
      sqlite3_విలువ_Apply_Affinity(pVal, affinity, SQLITE_UTF8);
    }
    ప్రతిజ్ఞా( (pVal->flags & MEM_IntReal)==0 );
    ఒకవేళ( pVal->flags & (MEM_Int|MEM_IntReal|MEM_Real) ){
      testcase( pVal->flags & MEM_Int );
      testcase( pVal->flags & MEM_Real );
      pVal->flags &= ~MEM_Str;
    }
    ఒకవేళ( enc!=SQLITE_UTF8 ){
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  }లేకపోతే ఒకవేళ( op==TK_UMINUS ) {
    /* This branch happens for multiple negative signs.  Ex: -(-5) */
    ఒకవేళ( SQLITE_OK==విలువ_From_సమాసం(db,p_సమాసం->p_ఎడమ,enc,affinity,&pVal,pCtx) 
     && pVal!=0
    ){
      sqlite3VdbeMemNumerify(pVal);
      ఒకవేళ( pVal->flags & MEM_Real ){
        pVal->u.r = -pVal->u.r;
      }లేకపోతే ఒకవేళ( pVal->u.i==అతిచిన్న_INT64 ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
        pVal->u.r = -(ద్విగుణ)అతిచిన్న_INT64;
#లేకపోతే
        pVal->u.r = అతిపెద్ద_INT64;
#అయితే_ముగింపు
        Mem_సమితి_Type_Flag(pVal, MEM_Real);
      }లేకపోతే{
        pVal->u.i = -pVal->u.i;
      }
      sqlite3_విలువ_Apply_Affinity(pVal, affinity, enc);
    }
  }లేకపోతే ఒకవేళ( op==TK_NULL ){
    pVal = విలువ_New(db, pCtx);
    ఒకవేళ( pVal==0 ) వెళ్ళు no_mem;
    sqlite3_Vdbe_Mem_సమితి_Null(pVal);
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_BLOB_LITERAL
  లేకపోతే ఒకవేళ( op==TK_BLOB ){
    పూర్ణాంకం nVal;
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
    ప్రతిజ్ఞా( p_సమాసం->u.zToken[0]=='x' || p_సమాసం->u.zToken[0]=='X' );
    ప్రతిజ్ఞా( p_సమాసం->u.zToken[1]=='\'' );
    pVal = విలువ_New(db, pCtx);
    ఒకవేళ( !pVal ) వెళ్ళు no_mem;
    zVal = &p_సమాసం->u.zToken[2];
    nVal = sqlite3Strlen30(zVal)-1;
    ప్రతిజ్ఞా( zVal[nVal]=='\'' );
    sqlite3_Vdbe_Mem_సమితి_Str(pVal, sqlite3_Hex_గమ్యం_Blob(db, zVal, nVal), nVal/2,
                         0, SQLITE_DYNAMIC);
    pVal->enc = enc;
  }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  లేకపోతే ఒకవేళ( op==TK_FUNCTION && pCtx!=0 ){
    rc = విలువ_From_Function(db, p_సమాసం, enc, affinity, &pVal, pCtx);
  }
#అయితే_ముగింపు
  లేకపోతే ఒకవేళ( op==TK_TRUEFALSE ){
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
    pVal = విలువ_New(db, pCtx);
    ఒకవేళ( pVal ){
      pVal->flags = MEM_Int;
      pVal->u.i = p_సమాసం->u.zToken[4]==0;
      sqlite3_విలువ_Apply_Affinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  తిరిగి rc;

no_mem:
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( pCtx==0 || NEVER(pCtx->pParse->nErr==0) )
#అయితే_ముగింపు
    sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);
  ప్రతిజ్ఞా( *ppVal==0 );
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( pCtx==0 ) sqlite3_విలువ_Free(pVal);
#లేకపోతే
  ప్రతిజ్ఞా( pCtx==0 ); sqlite3_విలువ_Free(pVal);
#అయితే_ముగింపు
  తిరిగి SQLITE_NOMEM_BKPT;
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
పూర్ణాంకం sqlite3_విలువ_From_సమాసం(
  sqlite3 *db,              /* The database connection */
  స్థిరరాశి సమాసం *p_సమాసం,        /* The expression to evaluate */
  u8 enc,                   /* Encoding to use */
  u8 affinity,              /* Affinity to use */
  sqlite3_విలువ **ppVal     /* Write the new value here */
){
  పూర్ణాంకం rc = SQLITE_OK;
  sqlite3_విలువ *pVal = 0;
  ఒకవేళ( p_సమాసం ){
    rc = విలువ_From_సమాసం(db, p_సమాసం, enc, affinity, &pVal, 0);
    ఒకవేళ( rc==SQLITE_OK && pVal 
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
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
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
స్థిర పూర్ణాంకం stat4_విలువ_From_సమాసం(
  Parse *pParse,                  /* Parse context */
  సమాసం *p_సమాసం,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  నిర్మాణం విలువ_New_Stat4_Ctx *pAlloc,/* How to allocate space.  Or NULL */
  sqlite3_విలువ **ppVal           /* OUT: New value object (or NULL) */
){
  పూర్ణాంకం rc = SQLITE_OK;
  sqlite3_విలువ *pVal = 0;
  sqlite3 *db = pParse->db;

  /* Skip over any TK_COLLATE nodes */
  p_సమాసం = sqlite3_సమాసం_Skip_Collate(p_సమాసం);

  ప్రతిజ్ఞా( p_సమాసం==0 || p_సమాసం->op!=TK_రిజిస్టరు || p_సమాసం->op2!=TK_VARIABLE );
  ఒకవేళ( !p_సమాసం ){
    pVal = విలువ_New(db, pAlloc);
    ఒకవేళ( pVal ){
      sqlite3_Vdbe_Mem_సమితి_Null((Mem*)pVal);
    }
  }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *v;
    పూర్ణాంకం iBindVar = p_సమాసం->i_స్తంభం;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iBindVar, 0);
    ఒకవేళ( (v = pParse->pReprepare)!=0 ){
      pVal = విలువ_New(db, pAlloc);
      ఒకవేళ( pVal ){
        rc = sqlite3VdbeMemCopy((Mem*)pVal, &v->aVar[iBindVar-1]);
        sqlite3_విలువ_Apply_Affinity(pVal, affinity, ENC(db));
        pVal->db = pParse->db;
      }
    }
  }లేకపోతే{
    rc = విలువ_From_సమాసం(db, p_సమాసం, ENC(db), affinity, &pVal, pAlloc);
  }

  ప్రతిజ్ఞా( pVal==0 || pVal->db==db );
  *ppVal = pVal;
  తిరిగి rc;
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
పూర్ణాంకం sqlite3_Stat4_Probe_సమితి_విలువ(
  Parse *pParse,                  /* Parse context */
  సూచీ *pIdx,                    /* Index being probed */
  Unpacked_రికార్డు **ppRec,         /* IN/OUT: Probe record */
  సమాసం *p_సమాసం,                    /* The expression to extract a value from */
  పూర్ణాంకం nElem,                      /* Maximum number of values to append */
  పూర్ణాంకం iVal,                       /* Array element to populate */
  పూర్ణాంకం *pn_వెలికితీయి                  /* OUT: Values appended to the record */
){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం n_వెలికితీయి = 0;

  ఒకవేళ( p_సమాసం==0 || p_సమాసం->op!=TK_SELECT ){
    పూర్ణాంకం i;
    నిర్మాణం విలువ_New_Stat4_Ctx alloc;

    alloc.pParse = pParse;
    alloc.pIdx = pIdx;
    alloc.ppRec = ppRec;

    కొరకు(i=0; i<nElem; i++){
      sqlite3_విలువ *pVal = 0;
      సమాసం *pElem = (p_సమాసం ? sqlite3_సదిశ_క్షేత్రం_Subexpr(p_సమాసం, i) : 0);
      u8 aff = sqlite3_సూచీ_స్తంభం_Affinity(pParse->db, pIdx, iVal+i);
      alloc.iVal = iVal+i;
      rc = stat4_విలువ_From_సమాసం(pParse, pElem, aff, &alloc, &pVal);
      ఒకవేళ( !pVal ) ఆపు;
      n_వెలికితీయి++;
    }
  }

  *pn_వెలికితీయి = n_వెలికితీయి;
  తిరిగి rc;
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
పూర్ణాంకం sqlite3_Stat4_విలువ_From_సమాసం(
  Parse *pParse,                  /* Parse context */
  సమాసం *p_సమాసం,                    /* The expression to extract a value from */
  u8 affinity,                    /* Affinity to use */
  sqlite3_విలువ **ppVal           /* OUT: New value object (or NULL) */
){
  తిరిగి stat4_విలువ_From_సమాసం(pParse, p_సమాసం, affinity, 0, ppVal);
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
పూర్ణాంకం sqlite3_Stat4_స్తంభం(
  sqlite3 *db,                    /* Database handle */
  స్థిరరాశి శూన్య *pRec,               /* Pointer to buffer containing record */
  పూర్ణాంకం nRec,                       /* Size of buffer pRec in bytes */
  పూర్ణాంకం i_నిలువు,                       /* Column to extract */
  sqlite3_విలువ **ppVal           /* OUT: Extracted value */
){
  u32 t = 0;                      /* a column type code */
  u32 nHdr;                       /* Size of the header in the record */
  u32 iHdr;                       /* Next unread header byte */
  i64 i_క్షేత్రం;                     /* Next unread data byte */
  u32 sz_క్షేత్రం = 0;                /* Size of the current data field */
  పూర్ణాంకం i;                          /* Column index */
  u8 *a = (u8*)pRec;              /* Typecast byte array */
  Mem *pMem = *ppVal;             /* Write result into this Mem object */

  ప్రతిజ్ఞా( i_నిలువు>0 );
  iHdr = getVarint32(a, nHdr);
  ఒకవేళ( nHdr>(u32)nRec || iHdr>=nHdr ) తిరిగి SQLITE_CORRUPT_BKPT;
  i_క్షేత్రం = nHdr;
  కొరకు(i=0; i<=i_నిలువు; i++){
    iHdr += getVarint32(&a[iHdr], t);
    testcase( iHdr==nHdr );
    testcase( iHdr==nHdr+1 );
    ఒకవేళ( iHdr>nHdr ) తిరిగి SQLITE_CORRUPT_BKPT;
    sz_క్షేత్రం = sqlite3VdbeSerialTypeLen(t);
    i_క్షేత్రం += sz_క్షేత్రం;
  }
  testcase( i_క్షేత్రం==nRec );
  testcase( i_క్షేత్రం==nRec+1 );
  ఒకవేళ( i_క్షేత్రం>nRec ) తిరిగి SQLITE_CORRUPT_BKPT;
  ఒకవేళ( pMem==0 ){
    pMem = *ppVal = sqlite3_విలువ_New(db);
    ఒకవేళ( pMem==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  }
  sqlite3VdbeSerialGet(&a[i_క్షేత్రం-sz_క్షేత్రం], t, pMem);
  pMem->enc = ENC(db);
  తిరిగి SQLITE_OK;
}

/*
** Unless it is NULL, the argument must be an UnpackedRecord object returned
** by an earlier call to sqlite3Stat4ProbeSetValue(). This call deletes
** the object.
*/
శూన్య sqlite3Stat4ProbeFree(Unpacked_రికార్డు *pRec){
  ఒకవేళ( pRec ){
    పూర్ణాంకం i;
    పూర్ణాంకం n_నిలువు = pRec->p_కీలకం_Info->n_All_క్షేత్రం;
    Mem *aMem = pRec->aMem;
    sqlite3 *db = aMem[0].db;
    కొరకు(i=0; i<n_నిలువు; i++){
      sqlite3VdbeMemRelease(&aMem[i]);
    }
    sqlite3_కీలకం_Info_Unref(pRec->p_కీలకం_Info);
    sqlite3DbFreeNN(db, pRec);
  }
}
#అయితే_ముగింపు /* ifdef SQLITE_ENABLE_STAT4 */

/*
** Change the string value of an sqlite3_value object
*/
శూన్య sqlite3_విలువ_సమితి_Str(
  sqlite3_విలువ *v,     /* Value to be set */
  పూర్ణాంకం n,                /* Length of string z */
  స్థిరరాశి శూన్య *z,        /* Text of the new string */
  u8 enc,               /* Encoding to use */
  శూన్య (*xDel)(శూన్య*)   /* Destructor for the string */
){
  ఒకవేళ( v ) sqlite3_Vdbe_Mem_సమితి_Str((Mem *)v, z, n, enc, xDel);
}

/*
** Free an sqlite3_value object
*/
శూన్య sqlite3_విలువ_Free(sqlite3_విలువ *v){
  ఒకవేళ( !v ) తిరిగి;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem*)v)->db, v);
}

/*
** The sqlite3ValueBytes() routine returns the number of bytes in the
** sqlite3_value object assuming that it uses the encoding "enc".
** The valueBytes() routine is a helper function.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం విలువ_బైట్లు_2(sqlite3_విలువ *pVal, u8 enc){
  తిరిగి విలువ_గమ్యం_పాఠ్యం(pVal, enc)!=0 ? pVal->n : 0;
}
పూర్ణాంకం sqlite3_విలువ_బైట్లు_2(sqlite3_విలువ *pVal, u8 enc){
  Mem *p = (Mem*)pVal;
  ప్రతిజ్ఞా( (p->flags & MEM_Null)==0 || (p->flags & (MEM_Str|MEM_Blob))==0 );
  ఒకవేళ( (p->flags & MEM_Str)!=0 && pVal->enc==enc ){
    తిరిగి p->n;
  }
  ఒకవేళ( (p->flags & MEM_Str)!=0 && enc!=SQLITE_UTF8 && pVal->enc!=SQLITE_UTF8 ){
    తిరిగి p->n;
  }
  ఒకవేళ( (p->flags & MEM_Blob)!=0 ){
    ఒకవేళ( p->flags & MEM_Zero ){
      తిరిగి p->n + p->u.nZero;
    }లేకపోతే{
      తిరిగి p->n;
    }
  }
  ఒకవేళ( p->flags & MEM_Null ) తిరిగి 0;
  తిరిగి విలువ_బైట్లు_2(pVal, enc);
}
