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
#অন্তর্ভাব "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  বাহ্য পূর্ণাঙ্ক backtrace(শূন্য**,পূর্ণাঙ্ক);
  বাহ্য শূন্য backtrace_symbols_fd(শূন্য*ধ্রুব*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
#অন্যথা
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#অন্তর্ভাব <stdio.h>

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
সংরচনা MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  সংরচনা MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  বর্ণ nBacktrace;                    /* Number of backtraces on this alloc */
  বর্ণ nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  পূর্ণাঙ্ক iForeGuard;                     /* Guard word for sanity */
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
স্থির সংরচনা {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  সংরচনা MemBlockHdr *pFirst;
  সংরচনা MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  পূর্ণাঙ্ক nBacktrace;
  শূন্য (*xBacktrace)(পূর্ণাঙ্ক, পূর্ণাঙ্ক, শূন্য **);

  /*
  ** Title text to insert in front of each block
  */
  পূর্ণাঙ্ক nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  বর্ণ zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  পূর্ণাঙ্ক disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  পূর্ণাঙ্ক nAlloc[NCSIZE];      /* Total number of allocations */
  পূর্ণাঙ্ক nCurrent[NCSIZE];    /* Current number of allocations */
  পূর্ণাঙ্ক mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
স্থির শূন্য adjustStats(পূর্ণাঙ্ক iSize, পূর্ণাঙ্ক increment){
  পূর্ণাঙ্ক i = ROUND8(iSize)/8;
  যদি( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  যদি( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    যদি( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }অন্যথা{
    mem.nCurrent[i]--;
    প্রতিজ্ঞা( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
স্থির সংরচনা MemBlockHdr *sqlite3MemsysGetHeader(ধ্রুব শূন্য *pAllocation){
  সংরচনা MemBlockHdr *p;
  পূর্ণাঙ্ক *pInt;
  u8 *pU8;
  পূর্ণাঙ্ক nReserve;

  p = (সংরচনা MemBlockHdr*)pAllocation;
  p--;
  প্রতিজ্ঞা( p->iForeGuard==(পূর্ণাঙ্ক)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (পূর্ণাঙ্ক*)pAllocation;
  pU8 = (u8*)pAllocation;
  প্রতিজ্ঞা( pInt[nReserve/পরিমাণ(পূর্ণাঙ্ক)]==(পূর্ণাঙ্ক)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  যাবত্( nReserve-- > p->iSize ) প্রতিজ্ঞা( pU8[nReserve]==0x65 );
  প্রত্যাগম p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
স্থির পূর্ণাঙ্ক sqlite3MemSize(শূন্য *p){
  সংরচনা MemBlockHdr *pHdr;
  যদি( !p ){
    প্রত্যাগম 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  প্রত্যাগম (পূর্ণাঙ্ক)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
স্থির পূর্ণাঙ্ক sqlite3MemInit(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  প্রতিজ্ঞা( (পরিমাণ(সংরচনা MemBlockHdr)&7) == 0 );
  যদি( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
স্থির শূন্য sqlite3MemShutdown(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
স্থির পূর্ণাঙ্ক sqlite3MemRoundup(পূর্ণাঙ্ক n){
  প্রত্যাগম ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
স্থির শূন্য randomFill(বর্ণ *pBuf, পূর্ণাঙ্ক nByte){
  unsigned পূর্ণাঙ্ক x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  যাবত্( nByte >= 4 ){
    x = (x>>1) ^ (-(পূর্ণাঙ্ক)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(পূর্ণাঙ্ক*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  যাবত্( nByte-- > 0 ){
    x = (x>>1) ^ (-(পূর্ণাঙ্ক)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
স্থির শূন্য *sqlite3MemMalloc(পূর্ণাঙ্ক nByte){
  সংরচনা MemBlockHdr *pHdr;
  শূন্য **pBt;
  বর্ণ *z;
  পূর্ণাঙ্ক *pInt;
  শূন্য *p = 0;
  পূর্ণাঙ্ক totalSize;
  পূর্ণাঙ্ক nReserve;
  sqlite3_mutex_enter(mem.mutex);
  প্রতিজ্ঞা( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + পরিমাণ(*pHdr) + পরিমাণ(পূর্ণাঙ্ক) +
               mem.nBacktrace*পরিমাণ(শূন্য*) + mem.nTitle;
  p = malloc(totalSize);
  যদি( p ){
    z = p;
    pBt = (শূন্য**)&z[mem.nTitle];
    pHdr = (সংরচনা MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    যদি( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }অন্যথা{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    যদি( mem.nBacktrace ){
      শূন্য *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*পরিমাণ(শূন্য*));
      প্রতিজ্ঞা(pBt[0]);
      যদি( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }অন্যথা{
      pHdr->nBacktrace = 0;
    }
    যদি( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (পূর্ণাঙ্ক*)&pHdr[1];
    pInt[nReserve/পরিমাণ(পূর্ণাঙ্ক)] = REARGUARD;
    randomFill((বর্ণ*)pInt, nByte);
    memset(((বর্ণ*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (শূন্য*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  প্রত্যাগম p; 
}

/*
** Free memory.
*/
স্থির শূন্য sqlite3MemFree(শূন্য *pPrior){
  সংরচনা MemBlockHdr *pHdr;
  শূন্য **pBt;
  বর্ণ *z;
  প্রতিজ্ঞা( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (শূন্য**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  যদি( pHdr->pPrev ){
    প্রতিজ্ঞা( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }অন্যথা{
    প্রতিজ্ঞা( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  যদি( pHdr->pNext ){
    প্রতিজ্ঞা( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }অন্যথা{
    প্রতিজ্ঞা( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (বর্ণ*)pBt;
  z -= pHdr->nTitle;
  adjustStats((পূর্ণাঙ্ক)pHdr->iSize, -1);
  randomFill(z, পরিমাণ(শূন্য*)*pHdr->nBacktraceSlots + পরিমাণ(*pHdr) +
                (পূর্ণাঙ্ক)pHdr->iSize + পরিমাণ(পূর্ণাঙ্ক) + pHdr->nTitle);
  মুক্ত(z);
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
স্থির শূন্য *sqlite3MemRealloc(শূন্য *pPrior, পূর্ণাঙ্ক nByte){
  সংরচনা MemBlockHdr *pOldHdr;
  শূন্য *pNew;
  প্রতিজ্ঞা( mem.disallow==0 );
  প্রতিজ্ঞা( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  যদি( pNew ){
    memcpy(pNew, pPrior, (পূর্ণাঙ্ক)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    যদি( nByte>pOldHdr->iSize ){
      randomFill(&((বর্ণ*)pNew)[pOldHdr->iSize], nByte - (পূর্ণাঙ্ক)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  প্রত্যাগম pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
শূন্য sqlite3MemSetDefault(শূন্য){
  স্থির ধ্রুব sqlite3_mem_methods defaultMethods = {
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
শূন্য sqlite3MemdebugSetType(শূন্য *p, u8 eType){
  যদি( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    সংরচনা MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    প্রতিজ্ঞা( pHdr->iForeGuard==FOREGUARD );
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
পূর্ণাঙ্ক sqlite3MemdebugHasType(ধ্রুব শূন্য *p, u8 eType){
  পূর্ণাঙ্ক rc = 1;
  যদি( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    সংরচনা MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    প্রতিজ্ঞা( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    যদি( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  প্রত্যাগম rc;
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
পূর্ণাঙ্ক sqlite3MemdebugNoType(ধ্রুব শূন্য *p, u8 eType){
  পূর্ণাঙ্ক rc = 1;
  যদি( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    সংরচনা MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    প্রতিজ্ঞা( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    যদি( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  প্রত্যাগম rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
শূন্য sqlite3MemdebugBacktrace(পূর্ণাঙ্ক depth){
  যদি( depth<0 ){ depth = 0; }
  যদি( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

শূন্য sqlite3MemdebugBacktraceCallback(শূন্য (*xBacktrace)(পূর্ণাঙ্ক, পূর্ণাঙ্ক, শূন্য **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
শূন্য sqlite3MemdebugSettitle(ধ্রুব বর্ণ *zTitle){
  unsigned পূর্ণাঙ্ক n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  যদি( n>=পরিমাণ(mem.zTitle) ) n = পরিমাণ(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

শূন্য sqlite3MemdebugSync(){
  সংরচনা MemBlockHdr *pHdr;
  প্রতি(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    শূন্য **pBt = (শূন্য**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((পূর্ণাঙ্ক)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
শূন্য sqlite3MemdebugDump(ধ্রুব বর্ণ *zFilename){
  FILE *out;
  সংরচনা MemBlockHdr *pHdr;
  শূন্য **pBt;
  পূর্ণাঙ্ক i;
  out = fopen(zFilename, "w");
  যদি( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    প্রত্যাগম;
  }
  প্রতি(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    বর্ণ *z = (বর্ণ*)pHdr;
    z -= pHdr->nBacktraceSlots*পরিমাণ(শূন্য*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    যদি( pHdr->nBacktrace ){
      fflush(out);
      pBt = (শূন্য**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  প্রতি(i=0; i<NCSIZE-1; i++){
    যদি( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  যদি( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
পূর্ণাঙ্ক sqlite3_Memdebug_Malloc_গুনতি(){
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক nTotal = 0;
  প্রতি(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  প্রত্যাগম nTotal;
}


#endif /* SQLITE_MEMDEBUG */
