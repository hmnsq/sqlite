/*
** 2007 August 15
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
** This file contains low-level memory allocation drivers for when
** SQLite will use the standard C-library malloc/realloc/free interface
** to obtain the memory it needs while adding lots of additional debugging
** information to each allocation in order to help detect and fix memory
** leaks and memory usage errors.
**
** This file contains implementations of the low-level memory allocation
** routines specified in the sqlite3_mem_methods object.
*/
#شامل "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  بیرونی صحیح backtrace(تهی**,صحیح);
  بیرونی تهی backtrace_symbols_fd(تهی*ثابت*,صحیح,صحیح);
#وگرنه
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#شامل <stdio.h>

/*
** Each memory allocation looks like this:
**
**  ------------------------------------------------------------------------
**  | Title |  backtrace pointers |  MemBlockHdr |  allocation |  EndGuard |
**  ------------------------------------------------------------------------
**
** The application code sees only a pointer to the allocation.  We have
** to back up from the allocation pointer to find the MemBlockHdr.  The
** MemBlockHdr tells us the size of the allocation and the number of
** backtrace pointers.  There is also a guard word at the end of the
** MemBlockHdr.
*/
ساختار MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  ساختار MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  نویسه nBacktrace;                    /* Number of backtraces on this alloc */
  نویسه nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  صحیح iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#define FOREGUARD 0x80F5E153
#define REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#define NCSIZE  1000

/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ایستا ساختار {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  ساختار MemBlockHdr *pFirst;
  ساختار MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  صحیح nBacktrace;
  تهی (*xBacktrace)(صحیح, صحیح, تهی **);

  /*
  ** Title text to insert in front of each block
  */
  صحیح nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  نویسه zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  صحیح disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  صحیح nAlloc[NCSIZE];      /* Total number of allocations */
  صحیح nCurrent[NCSIZE];    /* Current number of allocations */
  صحیح mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ایستا تهی adjustStats(صحیح iSize, صحیح increment){
  صحیح i = ROUND8(iSize)/8;
  اگر( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  اگر( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    اگر( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }وگرنه{
    mem.nCurrent[i]--;
    أكد( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
ایستا ساختار MemBlockHdr *sqlite3MemsysGetHeader(ثابت تهی *pAllocation){
  ساختار MemBlockHdr *p;
  صحیح *pInt;
  u8 *pU8;
  صحیح nReserve;

  p = (ساختار MemBlockHdr*)pAllocation;
  p--;
  أكد( p->iForeGuard==(صحیح)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (صحیح*)pAllocation;
  pU8 = (u8*)pAllocation;
  أكد( pInt[nReserve/اندازه(صحیح)]==(صحیح)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  وقتی( nReserve-- > p->iSize ) أكد( pU8[nReserve]==0x65 );
  بازگشت p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ایستا صحیح sqlite3MemSize(تهی *p){
  ساختار MemBlockHdr *pHdr;
  اگر( !p ){
    بازگشت 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  بازگشت (صحیح)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
ایستا صحیح sqlite3MemInit(تهی *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  أكد( (اندازه(ساختار MemBlockHdr)&7) == 0 );
  اگر( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  بازگشت SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ایستا تهی sqlite3MemShutdown(تهی *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ایستا صحیح sqlite3MemRoundup(صحیح n){
  بازگشت ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ایستا تهی randomFill(نویسه *pBuf, صحیح nByte){
  unsigned صحیح x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  وقتی( nByte >= 4 ){
    x = (x>>1) ^ (-(صحیح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(صحیح*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  وقتی( nByte-- > 0 ){
    x = (x>>1) ^ (-(صحیح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ایستا تهی *sqlite3MemMalloc(صحیح nByte){
  ساختار MemBlockHdr *pHdr;
  تهی **pBt;
  نویسه *z;
  صحیح *pInt;
  تهی *p = 0;
  صحیح totalSize;
  صحیح nReserve;
  sqlite3_mutex_enter(mem.mutex);
  أكد( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + اندازه(*pHdr) + اندازه(صحیح) +
               mem.nBacktrace*اندازه(تهی*) + mem.nTitle;
  p = malloc(totalSize);
  اگر( p ){
    z = p;
    pBt = (تهی**)&z[mem.nTitle];
    pHdr = (ساختار MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    اگر( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }وگرنه{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    اگر( mem.nBacktrace ){
      تهی *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*اندازه(تهی*));
      أكد(pBt[0]);
      اگر( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }وگرنه{
      pHdr->nBacktrace = 0;
    }
    اگر( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (صحیح*)&pHdr[1];
    pInt[nReserve/اندازه(صحیح)] = REARGUARD;
    randomFill((نویسه*)pInt, nByte);
    memset(((نویسه*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (تهی*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  بازگشت p; 
}

/*
** Free memory.
*/
ایستا تهی sqlite3MemFree(تهی *pPrior){
  ساختار MemBlockHdr *pHdr;
  تهی **pBt;
  نویسه *z;
  أكد( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (تهی**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  اگر( pHdr->pPrev ){
    أكد( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }وگرنه{
    أكد( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  اگر( pHdr->pNext ){
    أكد( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }وگرنه{
    أكد( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (نویسه*)pBt;
  z -= pHdr->nTitle;
  adjustStats((صحیح)pHdr->iSize, -1);
  randomFill(z, اندازه(تهی*)*pHdr->nBacktraceSlots + اندازه(*pHdr) +
                (صحیح)pHdr->iSize + اندازه(صحیح) + pHdr->nTitle);
  آزاد(z);
  sqlite3_mutex_leave(mem.mutex);  
}

/*
** Change the size of an existing memory allocation.
**
** For this debugging implementation, we *always* make a copy of the
** allocation into a new place in memory.  In this way, if the 
** higher level code is using pointer to the old allocation, it is 
** much more likely to break and we are much more liking to find
** the error.
*/
ایستا تهی *sqlite3MemRealloc(تهی *pPrior, صحیح nByte){
  ساختار MemBlockHdr *pOldHdr;
  تهی *pNew;
  أكد( mem.disallow==0 );
  أكد( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  اگر( pNew ){
    memcpy(pNew, pPrior, (صحیح)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    اگر( nByte>pOldHdr->iSize ){
      randomFill(&((نویسه*)pNew)[pOldHdr->iSize], nByte - (صحیح)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  بازگشت pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
تهی sqlite3MemSetDefault(تهی){
  ایستا ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3MemSize,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

/*
** Set the "type" of an allocation.
*/
تهی sqlite3MemdebugSetType(تهی *p, u8 eType){
  اگر( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ساختار MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    أكد( pHdr->iForeGuard==FOREGUARD );
    pHdr->eType = eType;
  }
}

/*
** Return TRUE if the mask of type in eType matches the type of the
** allocation p.  Also return true if p==NULL.
**
** This routine is designed for use within an assert() statement, to
** verify the type of an allocation.  For example:
**
**     assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
*/
صحیح sqlite3MemdebugHasType(ثابت تهی *p, u8 eType){
  صحیح rc = 1;
  اگر( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ساختار MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    اگر( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  بازگشت rc;
}

/*
** Return TRUE if the mask of type in eType matches no bits of the type of the
** allocation p.  Also return true if p==NULL.
**
** This routine is designed for use within an assert() statement, to
** verify the type of an allocation.  For example:
**
**     assert( sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
*/
صحیح sqlite3MemdebugNoType(ثابت تهی *p, u8 eType){
  صحیح rc = 1;
  اگر( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ساختار MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    اگر( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  بازگشت rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
تهی sqlite3MemdebugBacktrace(صحیح depth){
  اگر( depth<0 ){ depth = 0; }
  اگر( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

تهی sqlite3MemdebugBacktraceCallback(تهی (*xBacktrace)(صحیح, صحیح, تهی **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
تهی sqlite3MemdebugSettitle(ثابت نویسه *zTitle){
  unsigned صحیح n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  اگر( n>=اندازه(mem.zTitle) ) n = اندازه(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

تهی sqlite3MemdebugSync(){
  ساختار MemBlockHdr *pHdr;
  برای(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    تهی **pBt = (تهی**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((صحیح)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
تهی sqlite3MemdebugDump(ثابت نویسه *zFilename){
  FILE *out;
  ساختار MemBlockHdr *pHdr;
  تهی **pBt;
  صحیح i;
  out = fopen(zFilename, "w");
  اگر( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    بازگشت;
  }
  برای(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    نویسه *z = (نویسه*)pHdr;
    z -= pHdr->nBacktraceSlots*اندازه(تهی*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    اگر( pHdr->nBacktrace ){
      fflush(out);
      pBt = (تهی**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  برای(i=0; i<NCSIZE-1; i++){
    اگر( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  اگر( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
صحیح sqlite3_Memdebug_Malloc_شمار(){
  صحیح i;
  صحیح nTotal = 0;
  برای(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  بازگشت nTotal;
}


#endif /* SQLITE_MEMDEBUG */
