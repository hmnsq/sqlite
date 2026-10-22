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
#شامل "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
قسمتعریف ساخت Mem5Link Mem5Link;
ساخت Mem5Link {
  صحیح اگلا;       /* Index of next free chunk */
  صحیح پچھلا;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#تعریف_کرو LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#تعریف_کرو CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#تعریف_کرو CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ساکن SQLITE_WSD ساخت Mem5Global {
  /*
  ** Memory available for allocation
  */
  صحیح szAtom;      /* Smallest possible allocation in bytes */
  صحیح n_بلاک;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 کل_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 کل_Excess;    /* Total internal fragmentation */
  u32 current_برآمد;     /* Current checkout, including internal fragmentation */
  u32 current_گنتی;   /* Current number of distinct checkouts */
  u32 max_برآمد;         /* Maximum instantaneous currentOut */
  u32 max_گنتی;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#اگر_ختم
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  صحیح aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#تعریف_کرو mem5 GLOBAL(ساخت Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#تعریف_کرو MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
ساکن خالی memsys5Unlink(صحیح i, صحیح iLogsize){
  صحیح اگلا, پچھلا;
  أكد( i>=0 && i<mem5.n_بلاک );
  أكد( iLogsize>=0 && iLogsize<=LOGMAX );
  أكد( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  اگلا = MEM5LINK(i)->اگلا;
  پچھلا = MEM5LINK(i)->پچھلا;
  اگر( پچھلا<0 ){
    mem5.aiFreelist[iLogsize] = اگلا;
  }ورنہ{
    MEM5LINK(پچھلا)->اگلا = اگلا;
  }
  اگر( اگلا>=0 ){
    MEM5LINK(اگلا)->پچھلا = پچھلا;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
ساکن خالی memsys5Link(صحیح i, صحیح iLogsize){
  صحیح x;
  أكد( sqlite3_mutex_held(mem5.mutex) );
  أكد( i>=0 && i<mem5.n_بلاک );
  أكد( iLogsize>=0 && iLogsize<=LOGMAX );
  أكد( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->اگلا = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->پچھلا = -1;
  اگر( x>=0 ){
    أكد( x<mem5.n_بلاک );
    MEM5LINK(x)->پچھلا = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
ساکن خالی memsys5Enter(خالی){
  sqlite3_mutex_enter(mem5.mutex);
}
ساکن خالی memsys5Leave(خالی){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
ساکن صحیح memsys5_سائز(خالی *p){
  صحیح i_سائز, i;
  أكد( p!=0 );
  i = (صحیح)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  أكد( i>=0 && i<mem5.n_بلاک );
  i_سائز = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  واپس i_سائز;
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
ساکن خالی *memsys5MallocUnsafe(صحیح nByte){
  صحیح i;           /* Index of a mem5.aPool[] slot */
  صحیح iBin;        /* Index into mem5.aiFreelist[] */
  صحیح i_بھرا_Sz;     /* Size of allocation rounded up to power of 2 */
  صحیح iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  أكد( nByte>0 );

  /* No more than 1GiB per allocation */
  اگر( nByte > 0x40000000 ) واپس 0;

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  اگر( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#اگر_ختم


  /* Round nByte up to the next valid power of two */
  برائے(i_بھرا_Sz=mem5.szAtom,iLogsize=0; i_بھرا_Sz<nByte; i_بھرا_Sz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  برائے(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  اگر( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    واپس 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  جب( iBin>iLogsize ){
    صحیح new_سائز_2;

    iBin--;
    new_سائز_2 = 1 << iBin;
    mem5.aCtrl[i+new_سائز_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_سائز_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.کل_Alloc += i_بھرا_Sz;
  mem5.کل_Excess += i_بھرا_Sz - nByte;
  mem5.current_گنتی++;
  mem5.current_برآمد += i_بھرا_Sz;
  اگر( mem5.max_گنتی<mem5.current_گنتی ) mem5.max_گنتی = mem5.current_گنتی;
  اگر( mem5.max_برآمد<mem5.current_برآمد ) mem5.max_برآمد = mem5.current_برآمد;
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, i_بھرا_Sz);
#اگر_ختم

  /* Return a pointer to the allocated memory. */
  واپس (خالی*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
ساکن خالی memsys5FreeUnsafe(خالی *pOld){
  u32 سائز, iLogsize;
  صحیح i_بلاک;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  i_بلاک = (صحیح)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  أكد( i_بلاک>=0 && i_بلاک<mem5.n_بلاک );
  أكد( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  أكد( (mem5.aCtrl[i_بلاک] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[i_بلاک] & CTRL_LOGSIZE;
  سائز = 1<<iLogsize;
  أكد( i_بلاک+سائز-1<(u32)mem5.n_بلاک );

  mem5.aCtrl[i_بلاک] |= CTRL_FREE;
  mem5.aCtrl[i_بلاک+سائز-1] |= CTRL_FREE;

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  أكد( mem5.current_گنتی>0 );
  أكد( mem5.current_برآمد>=(سائز*mem5.szAtom) );
  mem5.current_گنتی--;
  mem5.current_برآمد -= سائز*mem5.szAtom;
  أكد( mem5.current_برآمد>0 || mem5.current_گنتی==0 );
  أكد( mem5.current_گنتی>0 || mem5.current_برآمد==0 );
#اگر_ختم

  mem5.aCtrl[i_بلاک] = CTRL_FREE | iLogsize;
  جب( ALWAYS(iLogsize<LOGMAX) ){
    صحیح iBuddy;
    اگر( (i_بلاک>>iLogsize) & 1 ){
      iBuddy = i_بلاک - سائز;
      أكد( iBuddy>=0 );
    }ورنہ{
      iBuddy = i_بلاک + سائز;
      اگر( iBuddy>=mem5.n_بلاک ) توڑو;
    }
    اگر( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) توڑو;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    اگر( iBuddy<i_بلاک ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[i_بلاک] = 0;
      i_بلاک = iBuddy;
    }ورنہ{
      mem5.aCtrl[i_بلاک] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    سائز *= 2;
  }

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  memset(&mem5.zPool[i_بلاک*mem5.szAtom], 0x55, سائز);
#اگر_ختم

  memsys5Link(i_بلاک, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
ساکن خالی *memsys5Malloc(صحیح n_بائٹیں){
  sqlite3_int64 *p = 0;
  اگر( n_بائٹیں>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(n_بائٹیں);
    memsys5Leave();
  }
  واپس (خالی*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
ساکن خالی memsys5Free(خالی *p_پیشگی){
  أكد( p_پیشگی!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(p_پیشگی);
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
ساکن خالی *memsys5Realloc(خالی *p_پیشگی, صحیح n_بائٹیں){
  صحیح nOld;
  خالی *p;
  أكد( p_پیشگی!=0 );
  أكد( (n_بائٹیں&(n_بائٹیں-1))==0 );  /* EV: R-46199-30249 */
  أكد( n_بائٹیں>=0 );
  اگر( n_بائٹیں==0 ){
    واپس 0;
  }
  nOld = memsys5_سائز(p_پیشگی);
  اگر( n_بائٹیں<=nOld ){
    واپس p_پیشگی;
  }
  p = memsys5Malloc(n_بائٹیں);
  اگر( p ){
    حافظہ_نقل(p, p_پیشگی, nOld);
    memsys5Free(p_پیشگی);
  }
  واپس p;
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
ساکن صحیح memsys5Roundup(صحیح n){
  صحیح i_بھرا_Sz;
  اگر( n<=mem5.szAtom*2 ){
    اگر( n<=mem5.szAtom ) واپس mem5.szAtom;
    واپس mem5.szAtom*2;
  }
  اگر( n>0x10000000 ){
    اگر( n>0x40000000 ) واپس 0;
    اگر( n>0x20000000 ) واپس 0x40000000;
    واپس 0x20000000;
  }
  برائے(i_بھرا_Sz=mem5.szAtom*8; i_بھرا_Sz<n; i_بھرا_Sz *= 4);
  اگر( (i_بھرا_Sz/2)>=(i64)n ) واپس i_بھرا_Sz/2;
  واپس i_بھرا_Sz;
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
ساکن صحیح memsys5Log(صحیح i_قدر){
  صحیح iLog;
  برائے(iLog=0; (iLog<(صحیح)((حجم(صحیح)*8)-1)) && (1<<iLog)<i_قدر; iLog++);
  واپس iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
ساکن صحیح memsys5Init(خالی *Not_مستعمل){
  صحیح ii;            /* Loop counter */
  صحیح nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  صحیح nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  صحیح i_انحراف;       /* An offset into mem5.aCtrl[] */

  غیر_مستعمل_PARAMETER(Not_مستعمل);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  أكد( (حجم(Mem5Link)&(حجم(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.n_ڈھیر;
  zByte = (u8*)sqlite3GlobalConfig.p_ڈھیر;
  أكد( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  جب( (صحیح)حجم(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.n_بلاک = (nByte / (mem5.szAtom+حجم(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.n_بلاک*mem5.szAtom];

  برائے(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  i_انحراف = 0;
  برائے(ii=LOGMAX; ii>=0; ii--){
    صحیح nAlloc = (1<<ii);
    اگر( (i_انحراف+nAlloc)<=mem5.n_بلاک ){
      mem5.aCtrl[i_انحراف] = ii | CTRL_FREE;
      memsys5Link(i_انحراف, ii);
      i_انحراف += nAlloc;
    }
    أكد((i_انحراف+nAlloc)>mem5.n_بلاک);
  }

  /* If a mutex is required for normal operation, allocate one */
  اگر( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  واپس SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساکن خالی memsys5Shutdown(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  mem5.mutex = 0;
  واپس;
}

#اگر_تعریف_شدہ SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
خالی sqlite3Memsys5Dump(ثابت حرف *zFilename){
  FILE *برآمد;
  صحیح i, j, n;
  صحیح nMinLog;

  اگر( zFilename==0 || zFilename[0]==0 ){
    برآمد = stdout;
  }ورنہ{
    برآمد = fopen(zFilename, "w");
    اگر( برآمد==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      واپس;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  برائے(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    برائے(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->اگلا, n++){}
    fprintf(برآمد, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(برآمد, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(برآمد, "mem5.totalAlloc   = %llu\n", mem5.کل_Alloc);
  fprintf(برآمد, "mem5.totalExcess  = %llu\n", mem5.کل_Excess);
  fprintf(برآمد, "mem5.currentOut   = %u\n", mem5.current_برآمد);
  fprintf(برآمد, "mem5.currentCount = %u\n", mem5.current_گنتی);
  fprintf(برآمد, "mem5.maxOut       = %u\n", mem5.max_برآمد);
  fprintf(برآمد, "mem5.maxCount     = %u\n", mem5.max_گنتی);
  fprintf(برآمد, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  اگر( برآمد==stdout ){
    fflush(stdout);
  }ورنہ{
    fclose(برآمد);
  }
}
#اگر_ختم

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
ثابت sqlite3_mem_methods *sqlite3MemGetMemsys5(خالی){
  ساکن ثابت sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_سائز,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  واپس &memsys5Methods;
}

#اگر_ختم /* SQLITE_ENABLE_MEMSYS5 */
