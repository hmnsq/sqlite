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
#অন্তর্ভাব "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#ifdef SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
প্রকারনির্দেশ সংরচনা Mem5Link Mem5Link;
সংরচনা Mem5Link {
  পূর্ণাঙ্ক next;       /* Index of next free chunk */
  পূর্ণাঙ্ক prev;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#define LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#define CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#define CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
স্থির SQLITE_WSD সংরচনা Mem5Global {
  /*
  ** Memory available for allocation
  */
  পূর্ণাঙ্ক szAtom;      /* Smallest possible allocation in bytes */
  পূর্ণাঙ্ক nBlock;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 totalAlloc;     /* Total of all malloc calls - includes internal frag */
  u64 totalExcess;    /* Total internal fragmentation */
  u32 currentOut;     /* Current checkout, including internal fragmentation */
  u32 current_গুনতি;   /* Current number of distinct checkouts */
  u32 maxOut;         /* Maximum instantaneous currentOut */
  u32 max_গুনতি;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#endif
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  পূর্ণাঙ্ক aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#define mem5 GLOBAL(সংরচনা Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#define MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
স্থির শূন্য memsys5Unlink(পূর্ণাঙ্ক i, পূর্ণাঙ্ক iLogsize){
  পূর্ণাঙ্ক next, prev;
  প্রতিজ্ঞা( i>=0 && i<mem5.nBlock );
  প্রতিজ্ঞা( iLogsize>=0 && iLogsize<=LOGMAX );
  প্রতিজ্ঞা( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  next = MEM5LINK(i)->next;
  prev = MEM5LINK(i)->prev;
  যদি( prev<0 ){
    mem5.aiFreelist[iLogsize] = next;
  }অন্যথা{
    MEM5LINK(prev)->next = next;
  }
  যদি( next>=0 ){
    MEM5LINK(next)->prev = prev;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
স্থির শূন্য memsys5Link(পূর্ণাঙ্ক i, পূর্ণাঙ্ক iLogsize){
  পূর্ণাঙ্ক x;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem5.mutex) );
  প্রতিজ্ঞা( i>=0 && i<mem5.nBlock );
  প্রতিজ্ঞা( iLogsize>=0 && iLogsize<=LOGMAX );
  প্রতিজ্ঞা( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->next = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  যদি( x>=0 ){
    প্রতিজ্ঞা( x<mem5.nBlock );
    MEM5LINK(x)->prev = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
স্থির শূন্য memsys5Enter(শূন্য){
  sqlite3_mutex_enter(mem5.mutex);
}
স্থির শূন্য memsys5Leave(শূন্য){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
স্থির পূর্ণাঙ্ক memsys5Size(শূন্য *p){
  পূর্ণাঙ্ক iSize, i;
  প্রতিজ্ঞা( p!=0 );
  i = (পূর্ণাঙ্ক)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  প্রতিজ্ঞা( i>=0 && i<mem5.nBlock );
  iSize = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  প্রত্যাগম iSize;
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
স্থির শূন্য *memsys5MallocUnsafe(পূর্ণাঙ্ক nByte){
  পূর্ণাঙ্ক i;           /* Index of a mem5.aPool[] slot */
  পূর্ণাঙ্ক iBin;        /* Index into mem5.aiFreelist[] */
  পূর্ণাঙ্ক iFullSz;     /* Size of allocation rounded up to power of 2 */
  পূর্ণাঙ্ক iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  প্রতিজ্ঞা( nByte>0 );

  /* No more than 1GiB per allocation */
  যদি( nByte > 0x40000000 ) প্রত্যাগম 0;

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  যদি( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#endif


  /* Round nByte up to the next valid power of two */
  প্রতি(iFullSz=mem5.szAtom,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  প্রতি(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  যদি( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    প্রত্যাগম 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  যাবত্( iBin>iLogsize ){
    পূর্ণাঙ্ক newSize;

    iBin--;
    newSize = 1 << iBin;
    mem5.aCtrl[i+newSize] = CTRL_FREE | iBin;
    memsys5Link(i+newSize, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.totalAlloc += iFullSz;
  mem5.totalExcess += iFullSz - nByte;
  mem5.current_গুনতি++;
  mem5.currentOut += iFullSz;
  যদি( mem5.max_গুনতি<mem5.current_গুনতি ) mem5.max_গুনতি = mem5.current_গুনতি;
  যদি( mem5.maxOut<mem5.currentOut ) mem5.maxOut = mem5.currentOut;
#endif

#ifdef SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, iFullSz);
#endif

  /* Return a pointer to the allocated memory. */
  প্রত্যাগম (শূন্য*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
স্থির শূন্য memsys5FreeUnsafe(শূন্য *pOld){
  u32 size, iLogsize;
  পূর্ণাঙ্ক iBlock;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  iBlock = (পূর্ণাঙ্ক)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  প্রতিজ্ঞা( iBlock>=0 && iBlock<mem5.nBlock );
  প্রতিজ্ঞা( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  প্রতিজ্ঞা( (mem5.aCtrl[iBlock] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[iBlock] & CTRL_LOGSIZE;
  size = 1<<iLogsize;
  প্রতিজ্ঞা( iBlock+size-1<(u32)mem5.nBlock );

  mem5.aCtrl[iBlock] |= CTRL_FREE;
  mem5.aCtrl[iBlock+size-1] |= CTRL_FREE;

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  প্রতিজ্ঞা( mem5.current_গুনতি>0 );
  প্রতিজ্ঞা( mem5.currentOut>=(size*mem5.szAtom) );
  mem5.current_গুনতি--;
  mem5.currentOut -= size*mem5.szAtom;
  প্রতিজ্ঞা( mem5.currentOut>0 || mem5.current_গুনতি==0 );
  প্রতিজ্ঞা( mem5.current_গুনতি>0 || mem5.currentOut==0 );
#endif

  mem5.aCtrl[iBlock] = CTRL_FREE | iLogsize;
  যাবত্( ALWAYS(iLogsize<LOGMAX) ){
    পূর্ণাঙ্ক iBuddy;
    যদি( (iBlock>>iLogsize) & 1 ){
      iBuddy = iBlock - size;
      প্রতিজ্ঞা( iBuddy>=0 );
    }অন্যথা{
      iBuddy = iBlock + size;
      যদি( iBuddy>=mem5.nBlock ) ভঙ্গ;
    }
    যদি( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) ভঙ্গ;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    যদি( iBuddy<iBlock ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBlock] = 0;
      iBlock = iBuddy;
    }অন্যথা{
      mem5.aCtrl[iBlock] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    size *= 2;
  }

#ifdef SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  memset(&mem5.zPool[iBlock*mem5.szAtom], 0x55, size);
#endif

  memsys5Link(iBlock, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
স্থির শূন্য *memsys5Malloc(পূর্ণাঙ্ক nBytes){
  sqlite3_int64 *p = 0;
  যদি( nBytes>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(nBytes);
    memsys5Leave();
  }
  প্রত্যাগম (শূন্য*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
স্থির শূন্য memsys5Free(শূন্য *pPrior){
  প্রতিজ্ঞা( pPrior!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(pPrior);
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
স্থির শূন্য *memsys5Realloc(শূন্য *pPrior, পূর্ণাঙ্ক nBytes){
  পূর্ণাঙ্ক nOld;
  শূন্য *p;
  প্রতিজ্ঞা( pPrior!=0 );
  প্রতিজ্ঞা( (nBytes&(nBytes-1))==0 );  /* EV: R-46199-30249 */
  প্রতিজ্ঞা( nBytes>=0 );
  যদি( nBytes==0 ){
    প্রত্যাগম 0;
  }
  nOld = memsys5Size(pPrior);
  যদি( nBytes<=nOld ){
    প্রত্যাগম pPrior;
  }
  p = memsys5Malloc(nBytes);
  যদি( p ){
    memcpy(p, pPrior, nOld);
    memsys5Free(pPrior);
  }
  প্রত্যাগম p;
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
স্থির পূর্ণাঙ্ক memsys5Roundup(পূর্ণাঙ্ক n){
  পূর্ণাঙ্ক iFullSz;
  যদি( n<=mem5.szAtom*2 ){
    যদি( n<=mem5.szAtom ) প্রত্যাগম mem5.szAtom;
    প্রত্যাগম mem5.szAtom*2;
  }
  যদি( n>0x10000000 ){
    যদি( n>0x40000000 ) প্রত্যাগম 0;
    যদি( n>0x20000000 ) প্রত্যাগম 0x40000000;
    প্রত্যাগম 0x20000000;
  }
  প্রতি(iFullSz=mem5.szAtom*8; iFullSz<n; iFullSz *= 4);
  যদি( (iFullSz/2)>=(i64)n ) প্রত্যাগম iFullSz/2;
  প্রত্যাগম iFullSz;
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
স্থির পূর্ণাঙ্ক memsys5Log(পূর্ণাঙ্ক i_মান){
  পূর্ণাঙ্ক iLog;
  প্রতি(iLog=0; (iLog<(পূর্ণাঙ্ক)((পরিমাণ(পূর্ণাঙ্ক)*8)-1)) && (1<<iLog)<i_মান; iLog++);
  প্রত্যাগম iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
স্থির পূর্ণাঙ্ক memsys5Init(শূন্য *NotUsed){
  পূর্ণাঙ্ক ii;            /* Loop counter */
  পূর্ণাঙ্ক nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  পূর্ণাঙ্ক nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  পূর্ণাঙ্ক iOffset;       /* An offset into mem5.aCtrl[] */

  UNUSED_PARAMETER(NotUsed);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  প্রতিজ্ঞা( (পরিমাণ(Mem5Link)&(পরিমাণ(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.nHeap;
  zByte = (u8*)sqlite3GlobalConfig.pHeap;
  প্রতিজ্ঞা( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  যাবত্( (পূর্ণাঙ্ক)পরিমাণ(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.nBlock = (nByte / (mem5.szAtom+পরিমাণ(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.nBlock*mem5.szAtom];

  প্রতি(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  iOffset = 0;
  প্রতি(ii=LOGMAX; ii>=0; ii--){
    পূর্ণাঙ্ক nAlloc = (1<<ii);
    যদি( (iOffset+nAlloc)<=mem5.nBlock ){
      mem5.aCtrl[iOffset] = ii | CTRL_FREE;
      memsys5Link(iOffset, ii);
      iOffset += nAlloc;
    }
    প্রতিজ্ঞা((iOffset+nAlloc)>mem5.nBlock);
  }

  /* If a mutex is required for normal operation, allocate one */
  যদি( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  প্রত্যাগম SQLITE_OK;
}

/*
** Deinitialize this module.
*/
স্থির শূন্য memsys5Shutdown(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem5.mutex = 0;
  প্রত্যাগম;
}

#ifdef SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
শূন্য sqlite3Memsys5Dump(ধ্রুব বর্ণ *zFilename){
  FILE *out;
  পূর্ণাঙ্ক i, j, n;
  পূর্ণাঙ্ক nMinLog;

  যদি( zFilename==0 || zFilename[0]==0 ){
    out = stdout;
  }অন্যথা{
    out = fopen(zFilename, "w");
    যদি( out==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      প্রত্যাগম;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  প্রতি(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    প্রতি(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->next, n++){}
    fprintf(out, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(out, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(out, "mem5.totalAlloc   = %llu\n", mem5.totalAlloc);
  fprintf(out, "mem5.totalExcess  = %llu\n", mem5.totalExcess);
  fprintf(out, "mem5.currentOut   = %u\n", mem5.currentOut);
  fprintf(out, "mem5.currentCount = %u\n", mem5.current_গুনতি);
  fprintf(out, "mem5.maxOut       = %u\n", mem5.maxOut);
  fprintf(out, "mem5.maxCount     = %u\n", mem5.max_গুনতি);
  fprintf(out, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  যদি( out==stdout ){
    fflush(stdout);
  }অন্যথা{
    fclose(out);
  }
}
#endif

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
ধ্রুব sqlite3_mem_methods *sqlite3MemGetMemsys5(শূন্য){
  স্থির ধ্রুব sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5Size,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  প্রত্যাগম &memsys5Methods;
}

#endif /* SQLITE_ENABLE_MEMSYS5 */
