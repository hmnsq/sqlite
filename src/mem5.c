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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Mem5Link Mem5Link;
ರಚನೆ Mem5Link {
  ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;       /* Index of next free chunk */
  ಪೂರ್ಣಾಂಕ prev;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#ವ್ಯಾಖ್ಯಾನಿಸು CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ Mem5Global {
  /*
  ** Memory available for allocation
  */
  ಪೂರ್ಣಾಂಕ szAtom;      /* Smallest possible allocation in bytes */
  ಪೂರ್ಣಾಂಕ n_ಖಂಡ;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 ಒಟ್ಟು_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 ಒಟ್ಟು_Excess;    /* Total internal fragmentation */
  u32 current_ಹೊರಹೋಗು;     /* Current checkout, including internal fragmentation */
  u32 current_ಎಣಿಕೆ;   /* Current number of distinct checkouts */
  u32 max_ಹೊರಹೋಗು;         /* Maximum instantaneous currentOut */
  u32 max_ಎಣಿಕೆ;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#ಆದರೆ_ಅಂತ್ಯ
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  ಪೂರ್ಣಾಂಕ aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು mem5 GLOBAL(ರಚನೆ Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys5Unlink(ಪೂರ್ಣಾಂಕ i, ಪೂರ್ಣಾಂಕ iLogsize){
  ಪೂರ್ಣಾಂಕ ಮುಂದಿನ, prev;
  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<mem5.n_ಖಂಡ );
  ಪ್ರತಿಜ್ಞಾ( iLogsize>=0 && iLogsize<=LOGMAX );
  ಪ್ರತಿಜ್ಞಾ( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  ಮುಂದಿನ = MEM5LINK(i)->ಮುಂದಿನ;
  prev = MEM5LINK(i)->prev;
  ಆದರೆ( prev<0 ){
    mem5.aiFreelist[iLogsize] = ಮುಂದಿನ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    MEM5LINK(prev)->ಮುಂದಿನ = ಮುಂದಿನ;
  }
  ಆದರೆ( ಮುಂದಿನ>=0 ){
    MEM5LINK(ಮುಂದಿನ)->prev = prev;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys5Link(ಪೂರ್ಣಾಂಕ i, ಪೂರ್ಣಾಂಕ iLogsize){
  ಪೂರ್ಣಾಂಕ x;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem5.mutex) );
  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<mem5.n_ಖಂಡ );
  ಪ್ರತಿಜ್ಞಾ( iLogsize>=0 && iLogsize<=LOGMAX );
  ಪ್ರತಿಜ್ಞಾ( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->ಮುಂದಿನ = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  ಆದರೆ( x>=0 ){
    ಪ್ರತಿಜ್ಞಾ( x<mem5.n_ಖಂಡ );
    MEM5LINK(x)->prev = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys5Enter(ಶೂನ್ಯ){
  sqlite3_mutex_enter(mem5.mutex);
}
ಸ್ಥಿರ ಶೂನ್ಯ memsys5Leave(ಶೂನ್ಯ){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys5_ಗಾತ್ರ(ಶೂನ್ಯ *p){
  ಪೂರ್ಣಾಂಕ i_ಗಾತ್ರ, i;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  i = (ಪೂರ್ಣಾಂಕ)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<mem5.n_ಖಂಡ );
  i_ಗಾತ್ರ = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  ಹಿಂತಿರುಗಿ i_ಗಾತ್ರ;
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
ಸ್ಥಿರ ಶೂನ್ಯ *memsys5MallocUnsafe(ಪೂರ್ಣಾಂಕ nByte){
  ಪೂರ್ಣಾಂಕ i;           /* Index of a mem5.aPool[] slot */
  ಪೂರ್ಣಾಂಕ iBin;        /* Index into mem5.aiFreelist[] */
  ಪೂರ್ಣಾಂಕ i_ತುಂಬಿದ_Sz;     /* Size of allocation rounded up to power of 2 */
  ಪೂರ್ಣಾಂಕ iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  ಪ್ರತಿಜ್ಞಾ( nByte>0 );

  /* No more than 1GiB per allocation */
  ಆದರೆ( nByte > 0x40000000 ) ಹಿಂತಿರುಗಿ 0;

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  ಆದರೆ( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#ಆದರೆ_ಅಂತ್ಯ


  /* Round nByte up to the next valid power of two */
  ಗಾಗಿ(i_ತುಂಬಿದ_Sz=mem5.szAtom,iLogsize=0; i_ತುಂಬಿದ_Sz<nByte; i_ತುಂಬಿದ_Sz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  ಗಾಗಿ(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  ಆದರೆ( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    ಹಿಂತಿರುಗಿ 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  ಆಗ( iBin>iLogsize ){
    ಪೂರ್ಣಾಂಕ new_ಗಾತ್ರ_2;

    iBin--;
    new_ಗಾತ್ರ_2 = 1 << iBin;
    mem5.aCtrl[i+new_ಗಾತ್ರ_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_ಗಾತ್ರ_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.ಒಟ್ಟು_Alloc += i_ತುಂಬಿದ_Sz;
  mem5.ಒಟ್ಟು_Excess += i_ತುಂಬಿದ_Sz - nByte;
  mem5.current_ಎಣಿಕೆ++;
  mem5.current_ಹೊರಹೋಗು += i_ತುಂಬಿದ_Sz;
  ಆದರೆ( mem5.max_ಎಣಿಕೆ<mem5.current_ಎಣಿಕೆ ) mem5.max_ಎಣಿಕೆ = mem5.current_ಎಣಿಕೆ;
  ಆದರೆ( mem5.max_ಹೊರಹೋಗು<mem5.current_ಹೊರಹೋಗು ) mem5.max_ಹೊರಹೋಗು = mem5.current_ಹೊರಹೋಗು;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, i_ತುಂಬಿದ_Sz);
#ಆದರೆ_ಅಂತ್ಯ

  /* Return a pointer to the allocated memory. */
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys5FreeUnsafe(ಶೂನ್ಯ *pOld){
  u32 ಗಾತ್ರ_2, iLogsize;
  ಪೂರ್ಣಾಂಕ i_ಖಂಡ;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  i_ಖಂಡ = (ಪೂರ್ಣಾಂಕ)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  ಪ್ರತಿಜ್ಞಾ( i_ಖಂಡ>=0 && i_ಖಂಡ<mem5.n_ಖಂಡ );
  ಪ್ರತಿಜ್ಞಾ( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  ಪ್ರತಿಜ್ಞಾ( (mem5.aCtrl[i_ಖಂಡ] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[i_ಖಂಡ] & CTRL_LOGSIZE;
  ಗಾತ್ರ_2 = 1<<iLogsize;
  ಪ್ರತಿಜ್ಞಾ( i_ಖಂಡ+ಗಾತ್ರ_2-1<(u32)mem5.n_ಖಂಡ );

  mem5.aCtrl[i_ಖಂಡ] |= CTRL_FREE;
  mem5.aCtrl[i_ಖಂಡ+ಗಾತ್ರ_2-1] |= CTRL_FREE;

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  ಪ್ರತಿಜ್ಞಾ( mem5.current_ಎಣಿಕೆ>0 );
  ಪ್ರತಿಜ್ಞಾ( mem5.current_ಹೊರಹೋಗು>=(ಗಾತ್ರ_2*mem5.szAtom) );
  mem5.current_ಎಣಿಕೆ--;
  mem5.current_ಹೊರಹೋಗು -= ಗಾತ್ರ_2*mem5.szAtom;
  ಪ್ರತಿಜ್ಞಾ( mem5.current_ಹೊರಹೋಗು>0 || mem5.current_ಎಣಿಕೆ==0 );
  ಪ್ರತಿಜ್ಞಾ( mem5.current_ಎಣಿಕೆ>0 || mem5.current_ಹೊರಹೋಗು==0 );
#ಆದರೆ_ಅಂತ್ಯ

  mem5.aCtrl[i_ಖಂಡ] = CTRL_FREE | iLogsize;
  ಆಗ( ALWAYS(iLogsize<LOGMAX) ){
    ಪೂರ್ಣಾಂಕ iBuddy;
    ಆದರೆ( (i_ಖಂಡ>>iLogsize) & 1 ){
      iBuddy = i_ಖಂಡ - ಗಾತ್ರ_2;
      ಪ್ರತಿಜ್ಞಾ( iBuddy>=0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      iBuddy = i_ಖಂಡ + ಗಾತ್ರ_2;
      ಆದರೆ( iBuddy>=mem5.n_ಖಂಡ ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) ನಿಲ್ಲಿಸು;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    ಆದರೆ( iBuddy<i_ಖಂಡ ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[i_ಖಂಡ] = 0;
      i_ಖಂಡ = iBuddy;
    }ಇಲ್ಲದಿದ್ದರೆ{
      mem5.aCtrl[i_ಖಂಡ] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    ಗಾತ್ರ_2 *= 2;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  memset(&mem5.zPool[i_ಖಂಡ*mem5.szAtom], 0x55, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ

  memsys5Link(i_ಖಂಡ, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memsys5Malloc(ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು){
  sqlite3_int64 *p = 0;
  ಆದರೆ( n_ಬೈಟುಗಳು>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(n_ಬೈಟುಗಳು);
    memsys5Leave();
  }
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys5Free(ಶೂನ್ಯ *p_ಪೂರ್ವ){
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(p_ಪೂರ್ವ);
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
ಸ್ಥಿರ ಶೂನ್ಯ *memsys5Realloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು){
  ಪೂರ್ಣಾಂಕ nOld;
  ಶೂನ್ಯ *p;
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 );
  ಪ್ರತಿಜ್ಞಾ( (n_ಬೈಟುಗಳು&(n_ಬೈಟುಗಳು-1))==0 );  /* EV: R-46199-30249 */
  ಪ್ರತಿಜ್ಞಾ( n_ಬೈಟುಗಳು>=0 );
  ಆದರೆ( n_ಬೈಟುಗಳು==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nOld = memsys5_ಗಾತ್ರ(p_ಪೂರ್ವ);
  ಆದರೆ( n_ಬೈಟುಗಳು<=nOld ){
    ಹಿಂತಿರುಗಿ p_ಪೂರ್ವ;
  }
  p = memsys5Malloc(n_ಬೈಟುಗಳು);
  ಆದರೆ( p ){
    ಸ್ಮರಣೆ_ನಕಲು(p, p_ಪೂರ್ವ, nOld);
    memsys5Free(p_ಪೂರ್ವ);
  }
  ಹಿಂತಿರುಗಿ p;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys5Roundup(ಪೂರ್ಣಾಂಕ n){
  ಪೂರ್ಣಾಂಕ i_ತುಂಬಿದ_Sz;
  ಆದರೆ( n<=mem5.szAtom*2 ){
    ಆದರೆ( n<=mem5.szAtom ) ಹಿಂತಿರುಗಿ mem5.szAtom;
    ಹಿಂತಿರುಗಿ mem5.szAtom*2;
  }
  ಆದರೆ( n>0x10000000 ){
    ಆದರೆ( n>0x40000000 ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( n>0x20000000 ) ಹಿಂತಿರುಗಿ 0x40000000;
    ಹಿಂತಿರುಗಿ 0x20000000;
  }
  ಗಾಗಿ(i_ತುಂಬಿದ_Sz=mem5.szAtom*8; i_ತುಂಬಿದ_Sz<n; i_ತುಂಬಿದ_Sz *= 4);
  ಆದರೆ( (i_ತುಂಬಿದ_Sz/2)>=(i64)n ) ಹಿಂತಿರುಗಿ i_ತುಂಬಿದ_Sz/2;
  ಹಿಂತಿರುಗಿ i_ತುಂಬಿದ_Sz;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys5Log(ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ){
  ಪೂರ್ಣಾಂಕ iLog;
  ಗಾಗಿ(iLog=0; (iLog<(ಪೂರ್ಣಾಂಕ)((ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*8)-1)) && (1<<iLog)<i_ಮೌಲ್ಯ; iLog++);
  ಹಿಂತಿರುಗಿ iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys5Init(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಪೂರ್ಣಾಂಕ ii;            /* Loop counter */
  ಪೂರ್ಣಾಂಕ nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  ಪೂರ್ಣಾಂಕ nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;       /* An offset into mem5.aCtrl[] */

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  ಪ್ರತಿಜ್ಞಾ( (ಗಾತ್ರ(Mem5Link)&(ಗಾತ್ರ(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.n_ರಾಶಿ;
  zByte = (u8*)sqlite3GlobalConfig.p_ರಾಶಿ;
  ಪ್ರತಿಜ್ಞಾ( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  ಆಗ( (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.n_ಖಂಡ = (nByte / (mem5.szAtom+ಗಾತ್ರ(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.n_ಖಂಡ*mem5.szAtom];

  ಗಾಗಿ(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  i_ಸ್ಥಾನಾಂತರ = 0;
  ಗಾಗಿ(ii=LOGMAX; ii>=0; ii--){
    ಪೂರ್ಣಾಂಕ nAlloc = (1<<ii);
    ಆದರೆ( (i_ಸ್ಥಾನಾಂತರ+nAlloc)<=mem5.n_ಖಂಡ ){
      mem5.aCtrl[i_ಸ್ಥಾನಾಂತರ] = ii | CTRL_FREE;
      memsys5Link(i_ಸ್ಥಾನಾಂತರ, ii);
      i_ಸ್ಥಾನಾಂತರ += nAlloc;
    }
    ಪ್ರತಿಜ್ಞಾ((i_ಸ್ಥಾನಾಂತರ+nAlloc)>mem5.n_ಖಂಡ);
  }

  /* If a mutex is required for normal operation, allocate one */
  ಆದರೆ( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys5Shutdown(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  mem5.mutex = 0;
  ಹಿಂತಿರುಗಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ಶೂನ್ಯ sqlite3Memsys5Dump(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  FILE *ಹೊರಹೋಗು;
  ಪೂರ್ಣಾಂಕ i, j, n;
  ಪೂರ್ಣಾಂಕ nMinLog;

  ಆದರೆ( zFilename==0 || zFilename[0]==0 ){
    ಹೊರಹೋಗು = stdout;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹೊರಹೋಗು = fopen(zFilename, "w");
    ಆದರೆ( ಹೊರಹೋಗು==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      ಹಿಂತಿರುಗಿ;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  ಗಾಗಿ(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    ಗಾಗಿ(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->ಮುಂದಿನ, n++){}
    fprintf(ಹೊರಹೋಗು, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(ಹೊರಹೋಗು, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(ಹೊರಹೋಗು, "mem5.totalAlloc   = %llu\n", mem5.ಒಟ್ಟು_Alloc);
  fprintf(ಹೊರಹೋಗು, "mem5.totalExcess  = %llu\n", mem5.ಒಟ್ಟು_Excess);
  fprintf(ಹೊರಹೋಗು, "mem5.currentOut   = %u\n", mem5.current_ಹೊರಹೋಗು);
  fprintf(ಹೊರಹೋಗು, "mem5.currentCount = %u\n", mem5.current_ಎಣಿಕೆ);
  fprintf(ಹೊರಹೋಗು, "mem5.maxOut       = %u\n", mem5.max_ಹೊರಹೋಗು);
  fprintf(ಹೊರಹೋಗು, "mem5.maxCount     = %u\n", mem5.max_ಎಣಿಕೆ);
  fprintf(ಹೊರಹೋಗು, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  ಆದರೆ( ಹೊರಹೋಗು==stdout ){
    fflush(stdout);
  }ಇಲ್ಲದಿದ್ದರೆ{
    fclose(ಹೊರಹೋಗು);
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetMemsys5(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_ಗಾತ್ರ,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  ಹಿಂತಿರುಗಿ &memsys5Methods;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_MEMSYS5 */
