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
#ضمن "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#اگر_تعریف_شده SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
تعريفنوع بنية Mem5Link Mem5Link;
بنية Mem5Link {
  صحيح بعدی;       /* Index of next free chunk */
  صحيح prev;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#تعریف_کن LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#تعریف_کن CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#تعریف_کن CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ساكن SQLITE_WSD بنية Mem5Global {
  /*
  ** Memory available for allocation
  */
  صحيح szAtom;      /* Smallest possible allocation in bytes */
  صحيح n_بلوک;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 مجموع_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 مجموع_Excess;    /* Total internal fragmentation */
  u32 current_بیرون;     /* Current checkout, including internal fragmentation */
  u32 current_شمار;   /* Current number of distinct checkouts */
  u32 max_بیرون;         /* Maximum instantaneous currentOut */
  u32 max_شمار;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#پایان_اگر
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  صحيح aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#تعریف_کن mem5 GLOBAL(بنية Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#تعریف_کن MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
ساكن فراغ memsys5Unlink(صحيح i, صحيح iLogsize){
  صحيح بعدی, prev;
  أكد( i>=0 && i<mem5.n_بلوک );
  أكد( iLogsize>=0 && iLogsize<=LOGMAX );
  أكد( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  بعدی = MEM5LINK(i)->بعدی;
  prev = MEM5LINK(i)->prev;
  إذا( prev<0 ){
    mem5.aiFreelist[iLogsize] = بعدی;
  }وإلا{
    MEM5LINK(prev)->بعدی = بعدی;
  }
  إذا( بعدی>=0 ){
    MEM5LINK(بعدی)->prev = prev;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
ساكن فراغ memsys5Link(صحيح i, صحيح iLogsize){
  صحيح x;
  أكد( sqlite3_mutex_held(mem5.mutex) );
  أكد( i>=0 && i<mem5.n_بلوک );
  أكد( iLogsize>=0 && iLogsize<=LOGMAX );
  أكد( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->بعدی = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  إذا( x>=0 ){
    أكد( x<mem5.n_بلوک );
    MEM5LINK(x)->prev = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
ساكن فراغ memsys5Enter(فراغ){
  sqlite3_mutex_enter(mem5.mutex);
}
ساكن فراغ memsys5Leave(فراغ){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
ساكن صحيح memsys5_اندازه(فراغ *p){
  صحيح i_اندازه, i;
  أكد( p!=0 );
  i = (صحيح)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  أكد( i>=0 && i<mem5.n_بلوک );
  i_اندازه = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  أرجع i_اندازه;
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
ساكن فراغ *memsys5MallocUnsafe(صحيح nByte){
  صحيح i;           /* Index of a mem5.aPool[] slot */
  صحيح iBin;        /* Index into mem5.aiFreelist[] */
  صحيح i_پر_Sz;     /* Size of allocation rounded up to power of 2 */
  صحيح iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  أكد( nByte>0 );

  /* No more than 1GiB per allocation */
  إذا( nByte > 0x40000000 ) أرجع 0;

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  إذا( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#پایان_اگر


  /* Round nByte up to the next valid power of two */
  لكل(i_پر_Sz=mem5.szAtom,iLogsize=0; i_پر_Sz<nByte; i_پر_Sz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  لكل(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  إذا( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    أرجع 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  طالما( iBin>iLogsize ){
    صحيح new_اندازه_2;

    iBin--;
    new_اندازه_2 = 1 << iBin;
    mem5.aCtrl[i+new_اندازه_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_اندازه_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.مجموع_Alloc += i_پر_Sz;
  mem5.مجموع_Excess += i_پر_Sz - nByte;
  mem5.current_شمار++;
  mem5.current_بیرون += i_پر_Sz;
  إذا( mem5.max_شمار<mem5.current_شمار ) mem5.max_شمار = mem5.current_شمار;
  إذا( mem5.max_بیرون<mem5.current_بیرون ) mem5.max_بیرون = mem5.current_بیرون;
#پایان_اگر

#اگر_تعریف_شده SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  memset(&mem5.zPool[i*mem5.szAtom], 0xAA, i_پر_Sz);
#پایان_اگر

  /* Return a pointer to the allocated memory. */
  أرجع (فراغ*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
ساكن فراغ memsys5FreeUnsafe(فراغ *pOld){
  u32 اندازه, iLogsize;
  صحيح i_بلوک;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  i_بلوک = (صحيح)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  أكد( i_بلوک>=0 && i_بلوک<mem5.n_بلوک );
  أكد( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  أكد( (mem5.aCtrl[i_بلوک] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[i_بلوک] & CTRL_LOGSIZE;
  اندازه = 1<<iLogsize;
  أكد( i_بلوک+اندازه-1<(u32)mem5.n_بلوک );

  mem5.aCtrl[i_بلوک] |= CTRL_FREE;
  mem5.aCtrl[i_بلوک+اندازه-1] |= CTRL_FREE;

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  أكد( mem5.current_شمار>0 );
  أكد( mem5.current_بیرون>=(اندازه*mem5.szAtom) );
  mem5.current_شمار--;
  mem5.current_بیرون -= اندازه*mem5.szAtom;
  أكد( mem5.current_بیرون>0 || mem5.current_شمار==0 );
  أكد( mem5.current_شمار>0 || mem5.current_بیرون==0 );
#پایان_اگر

  mem5.aCtrl[i_بلوک] = CTRL_FREE | iLogsize;
  طالما( ALWAYS(iLogsize<LOGMAX) ){
    صحيح iBuddy;
    إذا( (i_بلوک>>iLogsize) & 1 ){
      iBuddy = i_بلوک - اندازه;
      أكد( iBuddy>=0 );
    }وإلا{
      iBuddy = i_بلوک + اندازه;
      إذا( iBuddy>=mem5.n_بلوک ) اكسر;
    }
    إذا( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) اكسر;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    إذا( iBuddy<i_بلوک ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[i_بلوک] = 0;
      i_بلوک = iBuddy;
    }وإلا{
      mem5.aCtrl[i_بلوک] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    اندازه *= 2;
  }

#اگر_تعریف_شده SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  memset(&mem5.zPool[i_بلوک*mem5.szAtom], 0x55, اندازه);
#پایان_اگر

  memsys5Link(i_بلوک, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
ساكن فراغ *memsys5Malloc(صحيح n_بایتها){
  sqlite3_int64 *p = 0;
  إذا( n_بایتها>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(n_بایتها);
    memsys5Leave();
  }
  أرجع (فراغ*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
ساكن فراغ memsys5Free(فراغ *p_پیشین){
  أكد( p_پیشین!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(p_پیشین);
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
ساكن فراغ *memsys5Realloc(فراغ *p_پیشین, صحيح n_بایتها){
  صحيح nOld;
  فراغ *p;
  أكد( p_پیشین!=0 );
  أكد( (n_بایتها&(n_بایتها-1))==0 );  /* EV: R-46199-30249 */
  أكد( n_بایتها>=0 );
  إذا( n_بایتها==0 ){
    أرجع 0;
  }
  nOld = memsys5_اندازه(p_پیشین);
  إذا( n_بایتها<=nOld ){
    أرجع p_پیشین;
  }
  p = memsys5Malloc(n_بایتها);
  إذا( p ){
    کپی_حافظه(p, p_پیشین, nOld);
    memsys5Free(p_پیشین);
  }
  أرجع p;
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
ساكن صحيح memsys5Roundup(صحيح n){
  صحيح i_پر_Sz;
  إذا( n<=mem5.szAtom*2 ){
    إذا( n<=mem5.szAtom ) أرجع mem5.szAtom;
    أرجع mem5.szAtom*2;
  }
  إذا( n>0x10000000 ){
    إذا( n>0x40000000 ) أرجع 0;
    إذا( n>0x20000000 ) أرجع 0x40000000;
    أرجع 0x20000000;
  }
  لكل(i_پر_Sz=mem5.szAtom*8; i_پر_Sz<n; i_پر_Sz *= 4);
  إذا( (i_پر_Sz/2)>=(i64)n ) أرجع i_پر_Sz/2;
  أرجع i_پر_Sz;
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
ساكن صحيح memsys5Log(صحيح i_مقدار){
  صحيح iLog;
  لكل(iLog=0; (iLog<(صحيح)((حجم(صحيح)*8)-1)) && (1<<iLog)<i_مقدار; iLog++);
  أرجع iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
ساكن صحيح memsys5Init(فراغ *Not_استفاده_شده){
  صحيح ii;            /* Loop counter */
  صحيح nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  صحيح nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  صحيح i_جابه_جایی;       /* An offset into mem5.aCtrl[] */

  استفادهنشده_PARAMETER(Not_استفاده_شده);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  أكد( (حجم(Mem5Link)&(حجم(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.n_انباشته;
  zByte = (u8*)sqlite3GlobalConfig.p_انباشته;
  أكد( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  طالما( (صحيح)حجم(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.n_بلوک = (nByte / (mem5.szAtom+حجم(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.n_بلوک*mem5.szAtom];

  لكل(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  i_جابه_جایی = 0;
  لكل(ii=LOGMAX; ii>=0; ii--){
    صحيح nAlloc = (1<<ii);
    إذا( (i_جابه_جایی+nAlloc)<=mem5.n_بلوک ){
      mem5.aCtrl[i_جابه_جایی] = ii | CTRL_FREE;
      memsys5Link(i_جابه_جایی, ii);
      i_جابه_جایی += nAlloc;
    }
    أكد((i_جابه_جایی+nAlloc)>mem5.n_بلوک);
  }

  /* If a mutex is required for normal operation, allocate one */
  إذا( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  أرجع SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساكن فراغ memsys5Shutdown(فراغ *Not_استفاده_شده){
  استفادهنشده_PARAMETER(Not_استفاده_شده);
  mem5.mutex = 0;
  أرجع;
}

#اگر_تعریف_شده SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
فراغ sqlite3Memsys5Dump(ثابت حرف *zFilename){
  FILE *بیرون;
  صحيح i, j, n;
  صحيح nMinLog;

  إذا( zFilename==0 || zFilename[0]==0 ){
    بیرون = stdout;
  }وإلا{
    بیرون = fopen(zFilename, "w");
    إذا( بیرون==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      أرجع;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  لكل(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    لكل(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->بعدی, n++){}
    fprintf(بیرون, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(بیرون, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(بیرون, "mem5.totalAlloc   = %llu\n", mem5.مجموع_Alloc);
  fprintf(بیرون, "mem5.totalExcess  = %llu\n", mem5.مجموع_Excess);
  fprintf(بیرون, "mem5.currentOut   = %u\n", mem5.current_بیرون);
  fprintf(بیرون, "mem5.currentCount = %u\n", mem5.current_شمار);
  fprintf(بیرون, "mem5.maxOut       = %u\n", mem5.max_بیرون);
  fprintf(بیرون, "mem5.maxCount     = %u\n", mem5.max_شمار);
  fprintf(بیرون, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  إذا( بیرون==stdout ){
    fflush(stdout);
  }وإلا{
    fclose(بیرون);
  }
}
#پایان_اگر

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
ثابت sqlite3_mem_methods *sqlite3MemGetMemsys5(فراغ){
  ساكن ثابت sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_اندازه,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  أرجع &memsys5Methods;
}

#پایان_اگر /* SQLITE_ENABLE_MEMSYS5 */
