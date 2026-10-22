/*
** 2007 October 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement a memory
** allocation subsystem for use by SQLite. 
**
** This version of the memory allocation subsystem omits all
** use of malloc(). The application gives SQLite a block of memory
** before calling sqlite3_initialize() from which allocations
** are made and returned by the xMalloc() and xRealloc() 
** implementations. Once sqlite3_initialize() has been called,
** the amount of memory available to SQLite is fixed and cannot
** be changed.
**
** This version of the memory allocation subsystem is included
** in the build only if SQLITE_ENABLE_MEMSYS5 is defined.
**
** This memory allocator uses the following algorithm:
**
**   1.  All memory allocation sizes are rounded up to a power of 2.
**
**   2.  If two adjacent free blocks are the halves of a larger block,
**       then the two blocks are coalesced into the single larger block.
**
**   3.  New memory is allocated from the first available free block.
**
** This algorithm is described in: J. M. Robson. "Bounds for Some Functions
** Concerning Dynamic Storage Allocation". Journal of the Association for
** Computing Machinery, Volume 21, Number 8, July 1974, pages 491-499.
** 
** Let n be the size of the largest allocation divided by the minimum
** allocation size (after rounding all sizes up to a power of 2.)  Let M
** be the maximum amount of memory ever outstanding at one time.  Let
** N be the total amount of memory available for allocation.  Robson
** proved that this memory allocator will never breakdown due to 
** fragmentation as long as the following constraint holds:
**
**      N >=  M*(1 + log2(n)/2) - n + 1
**
** The sqlite3_status() logic tracks the maximum values of n and M so
** that an application can, at any time, verify this constraint.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Mem5Link Mem5Link;
ਬਣਤਰ Mem5Link {
  ਪੂਰਨ ਅਗਲਾ;       /* Index of next free chunk */
  ਪੂਰਨ prev;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ Mem5Global {
  /*
  ** Memory available for allocation
  */
  ਪੂਰਨ szAtom;      /* Smallest possible allocation in bytes */
  ਪੂਰਨ n_ਖੰਡ;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 ਕੁੱਲ_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 ਕੁੱਲ_Excess;    /* Total internal fragmentation */
  u32 current_ਬਾਹਰਲਾ;     /* Current checkout, including internal fragmentation */
  u32 current_ਗਿਣਤੀ;   /* Current number of distinct checkouts */
  u32 max_ਬਾਹਰਲਾ;         /* Maximum instantaneous currentOut */
  u32 max_ਗਿਣਤੀ;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#ਜੇ_ਅੰਤ
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  ਪੂਰਨ aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mem5 GLOBAL(ਬਣਤਰ Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
ਸਥਿਰ ਖਾਲੀ memsys5Unlink(ਪੂਰਨ i, ਪੂਰਨ iLogsize){
  ਪੂਰਨ ਅਗਲਾ, prev;
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<mem5.n_ਖੰਡ );
  ਪ੍ਰਤਿਜ੍ਞਾ( iLogsize>=0 && iLogsize<=LOGMAX );
  ਪ੍ਰਤਿਜ੍ਞਾ( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  ਅਗਲਾ = MEM5LINK(i)->ਅਗਲਾ;
  prev = MEM5LINK(i)->prev;
  ਜੇ( prev<0 ){
    mem5.aiFreelist[iLogsize] = ਅਗਲਾ;
  }ਨਹੀਂਤਾਂ{
    MEM5LINK(prev)->ਅਗਲਾ = ਅਗਲਾ;
  }
  ਜੇ( ਅਗਲਾ>=0 ){
    MEM5LINK(ਅਗਲਾ)->prev = prev;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
ਸਥਿਰ ਖਾਲੀ memsys5Link(ਪੂਰਨ i, ਪੂਰਨ iLogsize){
  ਪੂਰਨ x;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem5.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<mem5.n_ਖੰਡ );
  ਪ੍ਰਤਿਜ੍ਞਾ( iLogsize>=0 && iLogsize<=LOGMAX );
  ਪ੍ਰਤਿਜ੍ਞਾ( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->ਅਗਲਾ = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  ਜੇ( x>=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( x<mem5.n_ਖੰਡ );
    MEM5LINK(x)->prev = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
ਸਥਿਰ ਖਾਲੀ memsys5Enter(ਖਾਲੀ){
  sqlite3_mutex_enter(mem5.mutex);
}
ਸਥਿਰ ਖਾਲੀ memsys5Leave(ਖਾਲੀ){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
ਸਥਿਰ ਪੂਰਨ memsys5_ਮਾਪ(ਖਾਲੀ *p){
  ਪੂਰਨ i_ਮਾਪ, i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  i = (ਪੂਰਨ)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<mem5.n_ਖੰਡ );
  i_ਮਾਪ = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  ਵਾਪਸ i_ਮਾਪ;
}

/*
** Return a block of memory of at least nBytes in size.
** Return NULL if unable.  Return NULL if nBytes==0.
**
** The caller guarantees that nByte is positive.
**
** The caller has obtained a mutex prior to invoking this
** routine so there is never any chance that two or more
** threads can be in this routine at the same time.
*/
ਸਥਿਰ ਖਾਲੀ *memsys5MallocUnsafe(ਪੂਰਨ nByte){
  ਪੂਰਨ i;           /* Index of a mem5.aPool[] slot */
  ਪੂਰਨ iBin;        /* Index into mem5.aiFreelist[] */
  ਪੂਰਨ i_ਭਰਿਆ_Sz;     /* Size of allocation rounded up to power of 2 */
  ਪੂਰਨ iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte>0 );

  /* No more than 1GiB per allocation */
  ਜੇ( nByte > 0x40000000 ) ਵਾਪਸ 0;

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  ਜੇ( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#ਜੇ_ਅੰਤ


  /* Round nByte up to the next valid power of two */
  ਲਈ(i_ਭਰਿਆ_Sz=mem5.szAtom,iLogsize=0; i_ਭਰਿਆ_Sz<nByte; i_ਭਰਿਆ_Sz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  ਲਈ(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  ਜੇ( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    ਵਾਪਸ 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  ਜਦੋਂ( iBin>iLogsize ){
    ਪੂਰਨ new_ਮਾਪ_2;

    iBin--;
    new_ਮਾਪ_2 = 1 << iBin;
    mem5.aCtrl[i+new_ਮਾਪ_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_ਮਾਪ_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.ਕੁੱਲ_Alloc += i_ਭਰਿਆ_Sz;
  mem5.ਕੁੱਲ_Excess += i_ਭਰਿਆ_Sz - nByte;
  mem5.current_ਗਿਣਤੀ++;
  mem5.current_ਬਾਹਰਲਾ += i_ਭਰਿਆ_Sz;
  ਜੇ( mem5.max_ਗਿਣਤੀ<mem5.current_ਗਿਣਤੀ ) mem5.max_ਗਿਣਤੀ = mem5.current_ਗਿਣਤੀ;
  ਜੇ( mem5.max_ਬਾਹਰਲਾ<mem5.current_ਬਾਹਰਲਾ ) mem5.max_ਬਾਹਰਲਾ = mem5.current_ਬਾਹਰਲਾ;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, i_ਭਰਿਆ_Sz);
#ਜੇ_ਅੰਤ

  /* Return a pointer to the allocated memory. */
  ਵਾਪਸ (ਖਾਲੀ*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
ਸਥਿਰ ਖਾਲੀ memsys5FreeUnsafe(ਖਾਲੀ *pOld){
  u32 ਮਾਪ, iLogsize;
  ਪੂਰਨ i_ਖੰਡ;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  i_ਖੰਡ = (ਪੂਰਨ)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖੰਡ>=0 && i_ਖੰਡ<mem5.n_ਖੰਡ );
  ਪ੍ਰਤਿਜ੍ਞਾ( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (mem5.aCtrl[i_ਖੰਡ] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[i_ਖੰਡ] & CTRL_LOGSIZE;
  ਮਾਪ = 1<<iLogsize;
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖੰਡ+ਮਾਪ-1<(u32)mem5.n_ਖੰਡ );

  mem5.aCtrl[i_ਖੰਡ] |= CTRL_FREE;
  mem5.aCtrl[i_ਖੰਡ+ਮਾਪ-1] |= CTRL_FREE;

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  ਪ੍ਰਤਿਜ੍ਞਾ( mem5.current_ਗਿਣਤੀ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( mem5.current_ਬਾਹਰਲਾ>=(ਮਾਪ*mem5.szAtom) );
  mem5.current_ਗਿਣਤੀ--;
  mem5.current_ਬਾਹਰਲਾ -= ਮਾਪ*mem5.szAtom;
  ਪ੍ਰਤਿਜ੍ਞਾ( mem5.current_ਬਾਹਰਲਾ>0 || mem5.current_ਗਿਣਤੀ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( mem5.current_ਗਿਣਤੀ>0 || mem5.current_ਬਾਹਰਲਾ==0 );
#ਜੇ_ਅੰਤ

  mem5.aCtrl[i_ਖੰਡ] = CTRL_FREE | iLogsize;
  ਜਦੋਂ( ALWAYS(iLogsize<LOGMAX) ){
    ਪੂਰਨ iBuddy;
    ਜੇ( (i_ਖੰਡ>>iLogsize) & 1 ){
      iBuddy = i_ਖੰਡ - ਮਾਪ;
      ਪ੍ਰਤਿਜ੍ਞਾ( iBuddy>=0 );
    }ਨਹੀਂਤਾਂ{
      iBuddy = i_ਖੰਡ + ਮਾਪ;
      ਜੇ( iBuddy>=mem5.n_ਖੰਡ ) ਰੋਕੋ;
    }
    ਜੇ( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) ਰੋਕੋ;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    ਜੇ( iBuddy<i_ਖੰਡ ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[i_ਖੰਡ] = 0;
      i_ਖੰਡ = iBuddy;
    }ਨਹੀਂਤਾਂ{
      mem5.aCtrl[i_ਖੰਡ] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    ਮਾਪ *= 2;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  memset(&mem5.zPool[i_ਖੰਡ*mem5.szAtom], 0x55, ਮਾਪ);
#ਜੇ_ਅੰਤ

  memsys5Link(i_ਖੰਡ, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
ਸਥਿਰ ਖਾਲੀ *memsys5Malloc(ਪੂਰਨ n_ਬਾਈਟਾਂ){
  sqlite3_int64 *p = 0;
  ਜੇ( n_ਬਾਈਟਾਂ>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(n_ਬਾਈਟਾਂ);
    memsys5Leave();
  }
  ਵਾਪਸ (ਖਾਲੀ*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
ਸਥਿਰ ਖਾਲੀ memsys5Free(ਖਾਲੀ *p_ਪੂਰਵ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(p_ਪੂਰਵ);
  memsys5Leave();  
}

/*
** Change the size of an existing memory allocation.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.  
**
** nBytes is always a value obtained from a prior call to
** memsys5Round().  Hence nBytes is always a non-negative power
** of two.  If nBytes==0 that means that an oversize allocation
** (an allocation larger than 0x40000000) was requested and this
** routine should return 0 without freeing pPrior.
*/
ਸਥਿਰ ਖਾਲੀ *memsys5Realloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ n_ਬਾਈਟਾਂ){
  ਪੂਰਨ nOld;
  ਖਾਲੀ *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (n_ਬਾਈਟਾਂ&(n_ਬਾਈਟਾਂ-1))==0 );  /* EV: R-46199-30249 */
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਈਟਾਂ>=0 );
  ਜੇ( n_ਬਾਈਟਾਂ==0 ){
    ਵਾਪਸ 0;
  }
  nOld = memsys5_ਮਾਪ(p_ਪੂਰਵ);
  ਜੇ( n_ਬਾਈਟਾਂ<=nOld ){
    ਵਾਪਸ p_ਪੂਰਵ;
  }
  p = memsys5Malloc(n_ਬਾਈਟਾਂ);
  ਜੇ( p ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, p_ਪੂਰਵ, nOld);
    memsys5Free(p_ਪੂਰਵ);
  }
  ਵਾਪਸ p;
}

/*
** Round up a request size to the next valid allocation size.  If
** the allocation is too large to be handled by this allocation system,
** return 0.
**
** All allocations must be a power of two and must be expressed by a
** 32-bit signed integer.  Hence the largest allocation is 0x40000000
** or 1073741824 bytes.
*/
ਸਥਿਰ ਪੂਰਨ memsys5Roundup(ਪੂਰਨ n){
  ਪੂਰਨ i_ਭਰਿਆ_Sz;
  ਜੇ( n<=mem5.szAtom*2 ){
    ਜੇ( n<=mem5.szAtom ) ਵਾਪਸ mem5.szAtom;
    ਵਾਪਸ mem5.szAtom*2;
  }
  ਜੇ( n>0x10000000 ){
    ਜੇ( n>0x40000000 ) ਵਾਪਸ 0;
    ਜੇ( n>0x20000000 ) ਵਾਪਸ 0x40000000;
    ਵਾਪਸ 0x20000000;
  }
  ਲਈ(i_ਭਰਿਆ_Sz=mem5.szAtom*8; i_ਭਰਿਆ_Sz<n; i_ਭਰਿਆ_Sz *= 4);
  ਜੇ( (i_ਭਰਿਆ_Sz/2)>=(i64)n ) ਵਾਪਸ i_ਭਰਿਆ_Sz/2;
  ਵਾਪਸ i_ਭਰਿਆ_Sz;
}

/*
** Return the ceiling of the logarithm base 2 of iValue.
**
** Examples:   memsys5Log(1) -> 0
**             memsys5Log(2) -> 1
**             memsys5Log(4) -> 2
**             memsys5Log(5) -> 3
**             memsys5Log(8) -> 3
**             memsys5Log(9) -> 4
*/
ਸਥਿਰ ਪੂਰਨ memsys5Log(ਪੂਰਨ i_ਮੁੱਲ){
  ਪੂਰਨ iLog;
  ਲਈ(iLog=0; (iLog<(ਪੂਰਨ)((ਆਕਾਰ(ਪੂਰਨ)*8)-1)) && (1<<iLog)<i_ਮੁੱਲ; iLog++);
  ਵਾਪਸ iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
ਸਥਿਰ ਪੂਰਨ memsys5Init(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਪੂਰਨ ii;            /* Loop counter */
  ਪੂਰਨ nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  ਪੂਰਨ nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  ਪੂਰਨ i_ਵਿੱਥ;       /* An offset into mem5.aCtrl[] */

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (ਆਕਾਰ(Mem5Link)&(ਆਕਾਰ(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.n_ਢੇਰ;
  zByte = (u8*)sqlite3GlobalConfig.p_ਢੇਰ;
  ਪ੍ਰਤਿਜ੍ਞਾ( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  ਜਦੋਂ( (ਪੂਰਨ)ਆਕਾਰ(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.n_ਖੰਡ = (nByte / (mem5.szAtom+ਆਕਾਰ(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.n_ਖੰਡ*mem5.szAtom];

  ਲਈ(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  i_ਵਿੱਥ = 0;
  ਲਈ(ii=LOGMAX; ii>=0; ii--){
    ਪੂਰਨ nAlloc = (1<<ii);
    ਜੇ( (i_ਵਿੱਥ+nAlloc)<=mem5.n_ਖੰਡ ){
      mem5.aCtrl[i_ਵਿੱਥ] = ii | CTRL_FREE;
      memsys5Link(i_ਵਿੱਥ, ii);
      i_ਵਿੱਥ += nAlloc;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ((i_ਵਿੱਥ+nAlloc)>mem5.n_ਖੰਡ);
  }

  /* If a mutex is required for normal operation, allocate one */
  ਜੇ( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ਸਥਿਰ ਖਾਲੀ memsys5Shutdown(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  mem5.mutex = 0;
  ਵਾਪਸ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ਖਾਲੀ sqlite3Memsys5Dump(ਅਚਲ ਅੱਖਰ *zFilename){
  FILE *ਬਾਹਰਲਾ;
  ਪੂਰਨ i, j, n;
  ਪੂਰਨ nMinLog;

  ਜੇ( zFilename==0 || zFilename[0]==0 ){
    ਬਾਹਰਲਾ = stdout;
  }ਨਹੀਂਤਾਂ{
    ਬਾਹਰਲਾ = fopen(zFilename, "w");
    ਜੇ( ਬਾਹਰਲਾ==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      ਵਾਪਸ;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  ਲਈ(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    ਲਈ(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->ਅਗਲਾ, n++){}
    fprintf(ਬਾਹਰਲਾ, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(ਬਾਹਰਲਾ, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(ਬਾਹਰਲਾ, "mem5.totalAlloc   = %llu\n", mem5.ਕੁੱਲ_Alloc);
  fprintf(ਬਾਹਰਲਾ, "mem5.totalExcess  = %llu\n", mem5.ਕੁੱਲ_Excess);
  fprintf(ਬਾਹਰਲਾ, "mem5.currentOut   = %u\n", mem5.current_ਬਾਹਰਲਾ);
  fprintf(ਬਾਹਰਲਾ, "mem5.currentCount = %u\n", mem5.current_ਗਿਣਤੀ);
  fprintf(ਬਾਹਰਲਾ, "mem5.maxOut       = %u\n", mem5.max_ਬਾਹਰਲਾ);
  fprintf(ਬਾਹਰਲਾ, "mem5.maxCount     = %u\n", mem5.max_ਗਿਣਤੀ);
  fprintf(ਬਾਹਰਲਾ, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  ਜੇ( ਬਾਹਰਲਾ==stdout ){
    fflush(stdout);
  }ਨਹੀਂਤਾਂ{
    fclose(ਬਾਹਰਲਾ);
  }
}
#ਜੇ_ਅੰਤ

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
ਅਚਲ sqlite3_mem_methods *sqlite3MemGetMemsys5(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_ਮਾਪ,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  ਵਾਪਸ &memsys5Methods;
}

#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_MEMSYS5 */
