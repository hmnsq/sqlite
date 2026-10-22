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
#అన్తర్భావ "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
రకనిర్వచనం నిర్మాణం Mem5Link Mem5Link;
నిర్మాణం Mem5Link {
  పూర్ణాంకం తదుపరి;       /* Index of next free chunk */
  పూర్ణాంకం prev;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#నిర్వచించు LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#నిర్వచించు CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#నిర్వచించు CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
స్థిర SQLITE_WSD నిర్మాణం Mem5Global {
  /*
  ** Memory available for allocation
  */
  పూర్ణాంకం szAtom;      /* Smallest possible allocation in bytes */
  పూర్ణాంకం n_ఖండం;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 మొత్తం_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 మొత్తం_Excess;    /* Total internal fragmentation */
  u32 current_బహిర్గతం;     /* Current checkout, including internal fragmentation */
  u32 current_లెక్క;   /* Current number of distinct checkouts */
  u32 max_బహిర్గతం;         /* Maximum instantaneous currentOut */
  u32 max_లెక్క;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#అయితే_ముగింపు
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  పూర్ణాంకం aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#నిర్వచించు mem5 GLOBAL(నిర్మాణం Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#నిర్వచించు MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
స్థిర శూన్య memsys5Unlink(పూర్ణాంకం i, పూర్ణాంకం iLogsize){
  పూర్ణాంకం తదుపరి, prev;
  ప్రతిజ్ఞా( i>=0 && i<mem5.n_ఖండం );
  ప్రతిజ్ఞా( iLogsize>=0 && iLogsize<=LOGMAX );
  ప్రతిజ్ఞా( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  తదుపరి = MEM5LINK(i)->తదుపరి;
  prev = MEM5LINK(i)->prev;
  ఒకవేళ( prev<0 ){
    mem5.aiFreelist[iLogsize] = తదుపరి;
  }లేకపోతే{
    MEM5LINK(prev)->తదుపరి = తదుపరి;
  }
  ఒకవేళ( తదుపరి>=0 ){
    MEM5LINK(తదుపరి)->prev = prev;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
స్థిర శూన్య memsys5Link(పూర్ణాంకం i, పూర్ణాంకం iLogsize){
  పూర్ణాంకం x;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem5.mutex) );
  ప్రతిజ్ఞా( i>=0 && i<mem5.n_ఖండం );
  ప్రతిజ్ఞా( iLogsize>=0 && iLogsize<=LOGMAX );
  ప్రతిజ్ఞా( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->తదుపరి = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  ఒకవేళ( x>=0 ){
    ప్రతిజ్ఞా( x<mem5.n_ఖండం );
    MEM5LINK(x)->prev = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
స్థిర శూన్య memsys5Enter(శూన్య){
  sqlite3_mutex_enter(mem5.mutex);
}
స్థిర శూన్య memsys5Leave(శూన్య){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
స్థిర పూర్ణాంకం memsys5_కొలత(శూన్య *p){
  పూర్ణాంకం i_కొలత, i;
  ప్రతిజ్ఞా( p!=0 );
  i = (పూర్ణాంకం)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  ప్రతిజ్ఞా( i>=0 && i<mem5.n_ఖండం );
  i_కొలత = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  తిరిగి i_కొలత;
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
స్థిర శూన్య *memsys5MallocUnsafe(పూర్ణాంకం nByte){
  పూర్ణాంకం i;           /* Index of a mem5.aPool[] slot */
  పూర్ణాంకం iBin;        /* Index into mem5.aiFreelist[] */
  పూర్ణాంకం i_నిండిన_Sz;     /* Size of allocation rounded up to power of 2 */
  పూర్ణాంకం iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  ప్రతిజ్ఞా( nByte>0 );

  /* No more than 1GiB per allocation */
  ఒకవేళ( nByte > 0x40000000 ) తిరిగి 0;

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  ఒకవేళ( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#అయితే_ముగింపు


  /* Round nByte up to the next valid power of two */
  కొరకు(i_నిండిన_Sz=mem5.szAtom,iLogsize=0; i_నిండిన_Sz<nByte; i_నిండిన_Sz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  కొరకు(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  ఒకవేళ( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    తిరిగి 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  అయితే( iBin>iLogsize ){
    పూర్ణాంకం new_కొలత_2;

    iBin--;
    new_కొలత_2 = 1 << iBin;
    mem5.aCtrl[i+new_కొలత_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_కొలత_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.మొత్తం_Alloc += i_నిండిన_Sz;
  mem5.మొత్తం_Excess += i_నిండిన_Sz - nByte;
  mem5.current_లెక్క++;
  mem5.current_బహిర్గతం += i_నిండిన_Sz;
  ఒకవేళ( mem5.max_లెక్క<mem5.current_లెక్క ) mem5.max_లెక్క = mem5.current_లెక్క;
  ఒకవేళ( mem5.max_బహిర్గతం<mem5.current_బహిర్గతం ) mem5.max_బహిర్గతం = mem5.current_బహిర్గతం;
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, i_నిండిన_Sz);
#అయితే_ముగింపు

  /* Return a pointer to the allocated memory. */
  తిరిగి (శూన్య*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
స్థిర శూన్య memsys5FreeUnsafe(శూన్య *pOld){
  u32 కొలత, iLogsize;
  పూర్ణాంకం i_ఖండం;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  i_ఖండం = (పూర్ణాంకం)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  ప్రతిజ్ఞా( i_ఖండం>=0 && i_ఖండం<mem5.n_ఖండం );
  ప్రతిజ్ఞా( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  ప్రతిజ్ఞా( (mem5.aCtrl[i_ఖండం] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[i_ఖండం] & CTRL_LOGSIZE;
  కొలత = 1<<iLogsize;
  ప్రతిజ్ఞా( i_ఖండం+కొలత-1<(u32)mem5.n_ఖండం );

  mem5.aCtrl[i_ఖండం] |= CTRL_FREE;
  mem5.aCtrl[i_ఖండం+కొలత-1] |= CTRL_FREE;

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  ప్రతిజ్ఞా( mem5.current_లెక్క>0 );
  ప్రతిజ్ఞా( mem5.current_బహిర్గతం>=(కొలత*mem5.szAtom) );
  mem5.current_లెక్క--;
  mem5.current_బహిర్గతం -= కొలత*mem5.szAtom;
  ప్రతిజ్ఞా( mem5.current_బహిర్గతం>0 || mem5.current_లెక్క==0 );
  ప్రతిజ్ఞా( mem5.current_లెక్క>0 || mem5.current_బహిర్గతం==0 );
#అయితే_ముగింపు

  mem5.aCtrl[i_ఖండం] = CTRL_FREE | iLogsize;
  అయితే( ALWAYS(iLogsize<LOGMAX) ){
    పూర్ణాంకం iBuddy;
    ఒకవేళ( (i_ఖండం>>iLogsize) & 1 ){
      iBuddy = i_ఖండం - కొలత;
      ప్రతిజ్ఞా( iBuddy>=0 );
    }లేకపోతే{
      iBuddy = i_ఖండం + కొలత;
      ఒకవేళ( iBuddy>=mem5.n_ఖండం ) ఆపు;
    }
    ఒకవేళ( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) ఆపు;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    ఒకవేళ( iBuddy<i_ఖండం ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[i_ఖండం] = 0;
      i_ఖండం = iBuddy;
    }లేకపోతే{
      mem5.aCtrl[i_ఖండం] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    కొలత *= 2;
  }

#నిర్వచితమైతే SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  memset(&mem5.zPool[i_ఖండం*mem5.szAtom], 0x55, కొలత);
#అయితే_ముగింపు

  memsys5Link(i_ఖండం, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
స్థిర శూన్య *memsys5Malloc(పూర్ణాంకం n_బైట్లు){
  sqlite3_int64 *p = 0;
  ఒకవేళ( n_బైట్లు>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(n_బైట్లు);
    memsys5Leave();
  }
  తిరిగి (శూన్య*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
స్థిర శూన్య memsys5Free(శూన్య *p_పూర్వ){
  ప్రతిజ్ఞా( p_పూర్వ!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(p_పూర్వ);
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
స్థిర శూన్య *memsys5Realloc(శూన్య *p_పూర్వ, పూర్ణాంకం n_బైట్లు){
  పూర్ణాంకం nOld;
  శూన్య *p;
  ప్రతిజ్ఞా( p_పూర్వ!=0 );
  ప్రతిజ్ఞా( (n_బైట్లు&(n_బైట్లు-1))==0 );  /* EV: R-46199-30249 */
  ప్రతిజ్ఞా( n_బైట్లు>=0 );
  ఒకవేళ( n_బైట్లు==0 ){
    తిరిగి 0;
  }
  nOld = memsys5_కొలత(p_పూర్వ);
  ఒకవేళ( n_బైట్లు<=nOld ){
    తిరిగి p_పూర్వ;
  }
  p = memsys5Malloc(n_బైట్లు);
  ఒకవేళ( p ){
    జ్ఞాపకం_నకలు(p, p_పూర్వ, nOld);
    memsys5Free(p_పూర్వ);
  }
  తిరిగి p;
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
స్థిర పూర్ణాంకం memsys5Roundup(పూర్ణాంకం n){
  పూర్ణాంకం i_నిండిన_Sz;
  ఒకవేళ( n<=mem5.szAtom*2 ){
    ఒకవేళ( n<=mem5.szAtom ) తిరిగి mem5.szAtom;
    తిరిగి mem5.szAtom*2;
  }
  ఒకవేళ( n>0x10000000 ){
    ఒకవేళ( n>0x40000000 ) తిరిగి 0;
    ఒకవేళ( n>0x20000000 ) తిరిగి 0x40000000;
    తిరిగి 0x20000000;
  }
  కొరకు(i_నిండిన_Sz=mem5.szAtom*8; i_నిండిన_Sz<n; i_నిండిన_Sz *= 4);
  ఒకవేళ( (i_నిండిన_Sz/2)>=(i64)n ) తిరిగి i_నిండిన_Sz/2;
  తిరిగి i_నిండిన_Sz;
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
స్థిర పూర్ణాంకం memsys5Log(పూర్ణాంకం i_విలువ){
  పూర్ణాంకం iLog;
  కొరకు(iLog=0; (iLog<(పూర్ణాంకం)((పరిమాణం(పూర్ణాంకం)*8)-1)) && (1<<iLog)<i_విలువ; iLog++);
  తిరిగి iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
స్థిర పూర్ణాంకం memsys5Init(శూన్య *Not_వాడిన){
  పూర్ణాంకం ii;            /* Loop counter */
  పూర్ణాంకం nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  పూర్ణాంకం nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  పూర్ణాంకం i_స్థానభ్రంశం;       /* An offset into mem5.aCtrl[] */

  వాడని_PARAMETER(Not_వాడిన);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  ప్రతిజ్ఞా( (పరిమాణం(Mem5Link)&(పరిమాణం(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.n_కుప్ప;
  zByte = (u8*)sqlite3GlobalConfig.p_కుప్ప;
  ప్రతిజ్ఞా( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  అయితే( (పూర్ణాంకం)పరిమాణం(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.n_ఖండం = (nByte / (mem5.szAtom+పరిమాణం(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.n_ఖండం*mem5.szAtom];

  కొరకు(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  i_స్థానభ్రంశం = 0;
  కొరకు(ii=LOGMAX; ii>=0; ii--){
    పూర్ణాంకం nAlloc = (1<<ii);
    ఒకవేళ( (i_స్థానభ్రంశం+nAlloc)<=mem5.n_ఖండం ){
      mem5.aCtrl[i_స్థానభ్రంశం] = ii | CTRL_FREE;
      memsys5Link(i_స్థానభ్రంశం, ii);
      i_స్థానభ్రంశం += nAlloc;
    }
    ప్రతిజ్ఞా((i_స్థానభ్రంశం+nAlloc)>mem5.n_ఖండం);
  }

  /* If a mutex is required for normal operation, allocate one */
  ఒకవేళ( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  తిరిగి SQLITE_OK;
}

/*
** Deinitialize this module.
*/
స్థిర శూన్య memsys5Shutdown(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  mem5.mutex = 0;
  తిరిగి;
}

#నిర్వచితమైతే SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
శూన్య sqlite3Memsys5Dump(స్థిరరాశి అక్షరం *zFilename){
  FILE *బహిర్గతం;
  పూర్ణాంకం i, j, n;
  పూర్ణాంకం nMinLog;

  ఒకవేళ( zFilename==0 || zFilename[0]==0 ){
    బహిర్గతం = stdout;
  }లేకపోతే{
    బహిర్గతం = fopen(zFilename, "w");
    ఒకవేళ( బహిర్గతం==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      తిరిగి;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  కొరకు(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    కొరకు(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->తదుపరి, n++){}
    fprintf(బహిర్గతం, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(బహిర్గతం, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(బహిర్గతం, "mem5.totalAlloc   = %llu\n", mem5.మొత్తం_Alloc);
  fprintf(బహిర్గతం, "mem5.totalExcess  = %llu\n", mem5.మొత్తం_Excess);
  fprintf(బహిర్గతం, "mem5.currentOut   = %u\n", mem5.current_బహిర్గతం);
  fprintf(బహిర్గతం, "mem5.currentCount = %u\n", mem5.current_లెక్క);
  fprintf(బహిర్గతం, "mem5.maxOut       = %u\n", mem5.max_బహిర్గతం);
  fprintf(బహిర్గతం, "mem5.maxCount     = %u\n", mem5.max_లెక్క);
  fprintf(బహిర్గతం, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  ఒకవేళ( బహిర్గతం==stdout ){
    fflush(stdout);
  }లేకపోతే{
    fclose(బహిర్గతం);
  }
}
#అయితే_ముగింపు

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetMemsys5(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_కొలత,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  తిరిగి &memsys5Methods;
}

#అయితే_ముగింపు /* SQLITE_ENABLE_MEMSYS5 */
