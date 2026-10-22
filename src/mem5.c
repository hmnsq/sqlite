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
#അന്തര്ഭാവ "sqliteInt.h"

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
തരനിർവചനം ഘടന Mem5Link Mem5Link;
ഘടന Mem5Link {
  പൂർണ്ണസംഖ്യ next;       /* Index of next free chunk */
  പൂർണ്ണസംഖ്യ prev;       /* Index of previous free chunk */
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
സ്ഥിതം SQLITE_WSD ഘടന Mem5Global {
  /*
  ** Memory available for allocation
  */
  പൂർണ്ണസംഖ്യ szAtom;      /* Smallest possible allocation in bytes */
  പൂർണ്ണസംഖ്യ nBlock;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 totalAlloc;     /* Total of all malloc calls - includes internal frag */
  u64 totalExcess;    /* Total internal fragmentation */
  u32 currentOut;     /* Current checkout, including internal fragmentation */
  u32 current_എണ്ണം;   /* Current number of distinct checkouts */
  u32 maxOut;         /* Maximum instantaneous currentOut */
  u32 max_എണ്ണം;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#endif
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  പൂർണ്ണസംഖ്യ aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#define mem5 GLOBAL(ഘടന Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#define MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
സ്ഥിതം ശൂന്യം memsys5Unlink(പൂർണ്ണസംഖ്യ i, പൂർണ്ണസംഖ്യ iLogsize){
  പൂർണ്ണസംഖ്യ next, prev;
  പ്രതിജ്ഞാ( i>=0 && i<mem5.nBlock );
  പ്രതിജ്ഞാ( iLogsize>=0 && iLogsize<=LOGMAX );
  പ്രതിജ്ഞാ( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  next = MEM5LINK(i)->next;
  prev = MEM5LINK(i)->prev;
  എങ്കിൽ( prev<0 ){
    mem5.aiFreelist[iLogsize] = next;
  }അല്ലെങ്കിൽ{
    MEM5LINK(prev)->next = next;
  }
  എങ്കിൽ( next>=0 ){
    MEM5LINK(next)->prev = prev;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
സ്ഥിതം ശൂന്യം memsys5Link(പൂർണ്ണസംഖ്യ i, പൂർണ്ണസംഖ്യ iLogsize){
  പൂർണ്ണസംഖ്യ x;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem5.mutex) );
  പ്രതിജ്ഞാ( i>=0 && i<mem5.nBlock );
  പ്രതിജ്ഞാ( iLogsize>=0 && iLogsize<=LOGMAX );
  പ്രതിജ്ഞാ( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->next = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  എങ്കിൽ( x>=0 ){
    പ്രതിജ്ഞാ( x<mem5.nBlock );
    MEM5LINK(x)->prev = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
സ്ഥിതം ശൂന്യം memsys5Enter(ശൂന്യം){
  sqlite3_mutex_enter(mem5.mutex);
}
സ്ഥിതം ശൂന്യം memsys5Leave(ശൂന്യം){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys5Size(ശൂന്യം *p){
  പൂർണ്ണസംഖ്യ iSize, i;
  പ്രതിജ്ഞാ( p!=0 );
  i = (പൂർണ്ണസംഖ്യ)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  പ്രതിജ്ഞാ( i>=0 && i<mem5.nBlock );
  iSize = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  തിരികെ iSize;
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
സ്ഥിതം ശൂന്യം *memsys5MallocUnsafe(പൂർണ്ണസംഖ്യ nByte){
  പൂർണ്ണസംഖ്യ i;           /* Index of a mem5.aPool[] slot */
  പൂർണ്ണസംഖ്യ iBin;        /* Index into mem5.aiFreelist[] */
  പൂർണ്ണസംഖ്യ iFullSz;     /* Size of allocation rounded up to power of 2 */
  പൂർണ്ണസംഖ്യ iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  പ്രതിജ്ഞാ( nByte>0 );

  /* No more than 1GiB per allocation */
  എങ്കിൽ( nByte > 0x40000000 ) തിരികെ 0;

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  എങ്കിൽ( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#endif


  /* Round nByte up to the next valid power of two */
  വേണ്ടി(iFullSz=mem5.szAtom,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  വേണ്ടി(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  എങ്കിൽ( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    തിരികെ 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  ആകുമ്പോൾ( iBin>iLogsize ){
    പൂർണ്ണസംഖ്യ newSize;

    iBin--;
    newSize = 1 << iBin;
    mem5.aCtrl[i+newSize] = CTRL_FREE | iBin;
    memsys5Link(i+newSize, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.totalAlloc += iFullSz;
  mem5.totalExcess += iFullSz - nByte;
  mem5.current_എണ്ണം++;
  mem5.currentOut += iFullSz;
  എങ്കിൽ( mem5.max_എണ്ണം<mem5.current_എണ്ണം ) mem5.max_എണ്ണം = mem5.current_എണ്ണം;
  എങ്കിൽ( mem5.maxOut<mem5.currentOut ) mem5.maxOut = mem5.currentOut;
#endif

#ifdef SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, iFullSz);
#endif

  /* Return a pointer to the allocated memory. */
  തിരികെ (ശൂന്യം*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
സ്ഥിതം ശൂന്യം memsys5FreeUnsafe(ശൂന്യം *pOld){
  u32 size, iLogsize;
  പൂർണ്ണസംഖ്യ iBlock;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  iBlock = (പൂർണ്ണസംഖ്യ)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  പ്രതിജ്ഞാ( iBlock>=0 && iBlock<mem5.nBlock );
  പ്രതിജ്ഞാ( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  പ്രതിജ്ഞാ( (mem5.aCtrl[iBlock] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[iBlock] & CTRL_LOGSIZE;
  size = 1<<iLogsize;
  പ്രതിജ്ഞാ( iBlock+size-1<(u32)mem5.nBlock );

  mem5.aCtrl[iBlock] |= CTRL_FREE;
  mem5.aCtrl[iBlock+size-1] |= CTRL_FREE;

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  പ്രതിജ്ഞാ( mem5.current_എണ്ണം>0 );
  പ്രതിജ്ഞാ( mem5.currentOut>=(size*mem5.szAtom) );
  mem5.current_എണ്ണം--;
  mem5.currentOut -= size*mem5.szAtom;
  പ്രതിജ്ഞാ( mem5.currentOut>0 || mem5.current_എണ്ണം==0 );
  പ്രതിജ്ഞാ( mem5.current_എണ്ണം>0 || mem5.currentOut==0 );
#endif

  mem5.aCtrl[iBlock] = CTRL_FREE | iLogsize;
  ആകുമ്പോൾ( ALWAYS(iLogsize<LOGMAX) ){
    പൂർണ്ണസംഖ്യ iBuddy;
    എങ്കിൽ( (iBlock>>iLogsize) & 1 ){
      iBuddy = iBlock - size;
      പ്രതിജ്ഞാ( iBuddy>=0 );
    }അല്ലെങ്കിൽ{
      iBuddy = iBlock + size;
      എങ്കിൽ( iBuddy>=mem5.nBlock ) നിർത്തുക;
    }
    എങ്കിൽ( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) നിർത്തുക;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    എങ്കിൽ( iBuddy<iBlock ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBlock] = 0;
      iBlock = iBuddy;
    }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം *memsys5Malloc(പൂർണ്ണസംഖ്യ nBytes){
  sqlite3_int64 *p = 0;
  എങ്കിൽ( nBytes>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(nBytes);
    memsys5Leave();
  }
  തിരികെ (ശൂന്യം*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
സ്ഥിതം ശൂന്യം memsys5Free(ശൂന്യം *pPrior){
  പ്രതിജ്ഞാ( pPrior!=0 );
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
സ്ഥിതം ശൂന്യം *memsys5Realloc(ശൂന്യം *pPrior, പൂർണ്ണസംഖ്യ nBytes){
  പൂർണ്ണസംഖ്യ nOld;
  ശൂന്യം *p;
  പ്രതിജ്ഞാ( pPrior!=0 );
  പ്രതിജ്ഞാ( (nBytes&(nBytes-1))==0 );  /* EV: R-46199-30249 */
  പ്രതിജ്ഞാ( nBytes>=0 );
  എങ്കിൽ( nBytes==0 ){
    തിരികെ 0;
  }
  nOld = memsys5Size(pPrior);
  എങ്കിൽ( nBytes<=nOld ){
    തിരികെ pPrior;
  }
  p = memsys5Malloc(nBytes);
  എങ്കിൽ( p ){
    memcpy(p, pPrior, nOld);
    memsys5Free(pPrior);
  }
  തിരികെ p;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys5Roundup(പൂർണ്ണസംഖ്യ n){
  പൂർണ്ണസംഖ്യ iFullSz;
  എങ്കിൽ( n<=mem5.szAtom*2 ){
    എങ്കിൽ( n<=mem5.szAtom ) തിരികെ mem5.szAtom;
    തിരികെ mem5.szAtom*2;
  }
  എങ്കിൽ( n>0x10000000 ){
    എങ്കിൽ( n>0x40000000 ) തിരികെ 0;
    എങ്കിൽ( n>0x20000000 ) തിരികെ 0x40000000;
    തിരികെ 0x20000000;
  }
  വേണ്ടി(iFullSz=mem5.szAtom*8; iFullSz<n; iFullSz *= 4);
  എങ്കിൽ( (iFullSz/2)>=(i64)n ) തിരികെ iFullSz/2;
  തിരികെ iFullSz;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys5Log(പൂർണ്ണസംഖ്യ i_മൂല്യം){
  പൂർണ്ണസംഖ്യ iLog;
  വേണ്ടി(iLog=0; (iLog<(പൂർണ്ണസംഖ്യ)((വലുപ്പം(പൂർണ്ണസംഖ്യ)*8)-1)) && (1<<iLog)<i_മൂല്യം; iLog++);
  തിരികെ iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys5Init(ശൂന്യം *NotUsed){
  പൂർണ്ണസംഖ്യ ii;            /* Loop counter */
  പൂർണ്ണസംഖ്യ nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  പൂർണ്ണസംഖ്യ nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  പൂർണ്ണസംഖ്യ iOffset;       /* An offset into mem5.aCtrl[] */

  UNUSED_PARAMETER(NotUsed);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  പ്രതിജ്ഞാ( (വലുപ്പം(Mem5Link)&(വലുപ്പം(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.nHeap;
  zByte = (u8*)sqlite3GlobalConfig.pHeap;
  പ്രതിജ്ഞാ( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  ആകുമ്പോൾ( (പൂർണ്ണസംഖ്യ)വലുപ്പം(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.nBlock = (nByte / (mem5.szAtom+വലുപ്പം(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.nBlock*mem5.szAtom];

  വേണ്ടി(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  iOffset = 0;
  വേണ്ടി(ii=LOGMAX; ii>=0; ii--){
    പൂർണ്ണസംഖ്യ nAlloc = (1<<ii);
    എങ്കിൽ( (iOffset+nAlloc)<=mem5.nBlock ){
      mem5.aCtrl[iOffset] = ii | CTRL_FREE;
      memsys5Link(iOffset, ii);
      iOffset += nAlloc;
    }
    പ്രതിജ്ഞാ((iOffset+nAlloc)>mem5.nBlock);
  }

  /* If a mutex is required for normal operation, allocate one */
  എങ്കിൽ( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  തിരികെ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
സ്ഥിതം ശൂന്യം memsys5Shutdown(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem5.mutex = 0;
  തിരികെ;
}

#ifdef SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ശൂന്യം sqlite3Memsys5Dump(സ്ഥിരം അക്ഷരം *zFilename){
  FILE *out;
  പൂർണ്ണസംഖ്യ i, j, n;
  പൂർണ്ണസംഖ്യ nMinLog;

  എങ്കിൽ( zFilename==0 || zFilename[0]==0 ){
    out = stdout;
  }അല്ലെങ്കിൽ{
    out = fopen(zFilename, "w");
    എങ്കിൽ( out==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      തിരികെ;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  വേണ്ടി(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    വേണ്ടി(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->next, n++){}
    fprintf(out, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(out, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(out, "mem5.totalAlloc   = %llu\n", mem5.totalAlloc);
  fprintf(out, "mem5.totalExcess  = %llu\n", mem5.totalExcess);
  fprintf(out, "mem5.currentOut   = %u\n", mem5.currentOut);
  fprintf(out, "mem5.currentCount = %u\n", mem5.current_എണ്ണം);
  fprintf(out, "mem5.maxOut       = %u\n", mem5.maxOut);
  fprintf(out, "mem5.maxCount     = %u\n", mem5.max_എണ്ണം);
  fprintf(out, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  എങ്കിൽ( out==stdout ){
    fflush(stdout);
  }അല്ലെങ്കിൽ{
    fclose(out);
  }
}
#endif

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
സ്ഥിരം sqlite3_mem_methods *sqlite3MemGetMemsys5(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5Size,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  തിരികെ &memsys5Methods;
}

#endif /* SQLITE_ENABLE_MEMSYS5 */
