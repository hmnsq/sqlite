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
  বাহ্যিক পূর্ণসংখ্যা backtrace(শূন্য**,পূর্ণসংখ্যা);
  বাহ্যিক শূন্য backtrace_symbols_fd(শূন্য*ধ্রুবক*,পূর্ণসংখ্যা,পূর্ণসংখ্যা);
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
কাঠামো MemBlockHdr {
  i64 i_আকার;                          /* Size of this allocation */
  কাঠামো MemBlockHdr *p_পরবর্তী, *p_পূর্ব;  /* Linked list of all unfreed memory */
  অক্ষর nBacktrace;                    /* Number of backtraces on this alloc */
  অক্ষর nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  পূর্ণসংখ্যা iForeGuard;                     /* Guard word for sanity */
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
স্থির কাঠামো {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  কাঠামো MemBlockHdr *pFirst;
  কাঠামো MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  পূর্ণসংখ্যা nBacktrace;
  শূন্য (*xBacktrace)(পূর্ণসংখ্যা, পূর্ণসংখ্যা, শূন্য **);

  /*
  ** Title text to insert in front of each block
  */
  পূর্ণসংখ্যা nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  অক্ষর zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  পূর্ণসংখ্যা disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  পূর্ণসংখ্যা nAlloc[NCSIZE];      /* Total number of allocations */
  পূর্ণসংখ্যা nCurrent[NCSIZE];    /* Current number of allocations */
  পূর্ণসংখ্যা mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
স্থির শূন্য adjustStats(পূর্ণসংখ্যা i_আকার, পূর্ণসংখ্যা increment){
  পূর্ণসংখ্যা i = ROUND8(i_আকার)/8;
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
স্থির কাঠামো MemBlockHdr *sqlite3MemsysGetHeader(ধ্রুবক শূন্য *pAllocation){
  কাঠামো MemBlockHdr *p;
  পূর্ণসংখ্যা *pInt;
  u8 *pU8;
  পূর্ণসংখ্যা nReserve;

  p = (কাঠামো MemBlockHdr*)pAllocation;
  p--;
  প্রতিজ্ঞা( p->iForeGuard==(পূর্ণসংখ্যা)FOREGUARD );
  nReserve = ROUND8(p->i_আকার);
  pInt = (পূর্ণসংখ্যা*)pAllocation;
  pU8 = (u8*)pAllocation;
  প্রতিজ্ঞা( pInt[nReserve/আকার(পূর্ণসংখ্যা)]==(পূর্ণসংখ্যা)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  যখন( nReserve-- > p->i_আকার ) প্রতিজ্ঞা( pU8[nReserve]==0x65 );
  ফেরত p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
স্থির পূর্ণসংখ্যা sqlite3_Mem_আকার(শূন্য *p){
  কাঠামো MemBlockHdr *pHdr;
  যদি( !p ){
    ফেরত 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  ফেরত (পূর্ণসংখ্যা)pHdr->i_আকার;
}

/*
** Initialize the memory allocation subsystem.
*/
স্থির পূর্ণসংখ্যা sqlite3MemInit(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  প্রতিজ্ঞা( (আকার(কাঠামো MemBlockHdr)&7) == 0 );
  যদি( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা sqlite3MemRoundup(পূর্ণসংখ্যা n){
  ফেরত ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
স্থির শূন্য randomFill(অক্ষর *pBuf, পূর্ণসংখ্যা nByte){
  unsigned পূর্ণসংখ্যা x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  যখন( nByte >= 4 ){
    x = (x>>1) ^ (-(পূর্ণসংখ্যা)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(পূর্ণসংখ্যা*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  যখন( nByte-- > 0 ){
    x = (x>>1) ^ (-(পূর্ণসংখ্যা)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
স্থির শূন্য *sqlite3MemMalloc(পূর্ণসংখ্যা nByte){
  কাঠামো MemBlockHdr *pHdr;
  শূন্য **pBt;
  অক্ষর *z;
  পূর্ণসংখ্যা *pInt;
  শূন্য *p = 0;
  পূর্ণসংখ্যা যোগ_আকার;
  পূর্ণসংখ্যা nReserve;
  sqlite3_mutex_enter(mem.mutex);
  প্রতিজ্ঞা( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  যোগ_আকার = nReserve + আকার(*pHdr) + আকার(পূর্ণসংখ্যা) +
               mem.nBacktrace*আকার(শূন্য*) + mem.nTitle;
  p = malloc(যোগ_আকার);
  যদি( p ){
    z = p;
    pBt = (শূন্য**)&z[mem.nTitle];
    pHdr = (কাঠামো MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->p_পরবর্তী = 0;
    pHdr->p_পূর্ব = mem.pLast;
    যদি( mem.pLast ){
      mem.pLast->p_পরবর্তী = pHdr;
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
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*আকার(শূন্য*));
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
    pHdr->i_আকার = nByte;
    adjustStats(nByte, +1);
    pInt = (পূর্ণসংখ্যা*)&pHdr[1];
    pInt[nReserve/আকার(পূর্ণসংখ্যা)] = REARGUARD;
    randomFill((অক্ষর*)pInt, nByte);
    memset(((অক্ষর*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (শূন্য*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  ফেরত p; 
}

/*
** Free memory.
*/
স্থির শূন্য sqlite3MemFree(শূন্য *pPrior){
  কাঠামো MemBlockHdr *pHdr;
  শূন্য **pBt;
  অক্ষর *z;
  প্রতিজ্ঞা( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (শূন্য**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  যদি( pHdr->p_পূর্ব ){
    প্রতিজ্ঞা( pHdr->p_পূর্ব->p_পরবর্তী==pHdr );
    pHdr->p_পূর্ব->p_পরবর্তী = pHdr->p_পরবর্তী;
  }অন্যথা{
    প্রতিজ্ঞা( mem.pFirst==pHdr );
    mem.pFirst = pHdr->p_পরবর্তী;
  }
  যদি( pHdr->p_পরবর্তী ){
    প্রতিজ্ঞা( pHdr->p_পরবর্তী->p_পূর্ব==pHdr );
    pHdr->p_পরবর্তী->p_পূর্ব = pHdr->p_পূর্ব;
  }অন্যথা{
    প্রতিজ্ঞা( mem.pLast==pHdr );
    mem.pLast = pHdr->p_পূর্ব;
  }
  z = (অক্ষর*)pBt;
  z -= pHdr->nTitle;
  adjustStats((পূর্ণসংখ্যা)pHdr->i_আকার, -1);
  randomFill(z, আকার(শূন্য*)*pHdr->nBacktraceSlots + আকার(*pHdr) +
                (পূর্ণসংখ্যা)pHdr->i_আকার + আকার(পূর্ণসংখ্যা) + pHdr->nTitle);
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
স্থির শূন্য *sqlite3MemRealloc(শূন্য *pPrior, পূর্ণসংখ্যা nByte){
  কাঠামো MemBlockHdr *pOldHdr;
  শূন্য *pNew;
  প্রতিজ্ঞা( mem.disallow==0 );
  প্রতিজ্ঞা( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  যদি( pNew ){
    memcpy(pNew, pPrior, (পূর্ণসংখ্যা)(nByte<pOldHdr->i_আকার ? nByte : pOldHdr->i_আকার));
    যদি( nByte>pOldHdr->i_আকার ){
      randomFill(&((অক্ষর*)pNew)[pOldHdr->i_আকার], nByte - (পূর্ণসংখ্যা)pOldHdr->i_আকার);
    }
    sqlite3MemFree(pPrior);
  }
  ফেরত pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
শূন্য sqlite3MemSetDefault(শূন্য){
  স্থির ধ্রুবক sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_আকার,
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
    কাঠামো MemBlockHdr *pHdr;
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
পূর্ণসংখ্যা sqlite3MemdebugHasType(ধ্রুবক শূন্য *p, u8 eType){
  পূর্ণসংখ্যা rc = 1;
  যদি( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    কাঠামো MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    প্রতিজ্ঞা( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    যদি( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  ফেরত rc;
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
পূর্ণসংখ্যা sqlite3MemdebugNoType(ধ্রুবক শূন্য *p, u8 eType){
  পূর্ণসংখ্যা rc = 1;
  যদি( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    কাঠামো MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    প্রতিজ্ঞা( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    যদি( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  ফেরত rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
শূন্য sqlite3MemdebugBacktrace(পূর্ণসংখ্যা depth){
  যদি( depth<0 ){ depth = 0; }
  যদি( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

শূন্য sqlite3MemdebugBacktraceCallback(শূন্য (*xBacktrace)(পূর্ণসংখ্যা, পূর্ণসংখ্যা, শূন্য **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
শূন্য sqlite3MemdebugSettitle(ধ্রুবক অক্ষর *zTitle){
  unsigned পূর্ণসংখ্যা n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  যদি( n>=আকার(mem.zTitle) ) n = আকার(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

শূন্য sqlite3MemdebugSync(){
  কাঠামো MemBlockHdr *pHdr;
  জন্য(pHdr=mem.pFirst; pHdr; pHdr=pHdr->p_পরবর্তী){
    শূন্য **pBt = (শূন্য**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((পূর্ণসংখ্যা)pHdr->i_আকার, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
শূন্য sqlite3MemdebugDump(ধ্রুবক অক্ষর *zFilename){
  FILE *out;
  কাঠামো MemBlockHdr *pHdr;
  শূন্য **pBt;
  পূর্ণসংখ্যা i;
  out = fopen(zFilename, "w");
  যদি( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    ফেরত;
  }
  জন্য(pHdr=mem.pFirst; pHdr; pHdr=pHdr->p_পরবর্তী){
    অক্ষর *z = (অক্ষর*)pHdr;
    z -= pHdr->nBacktraceSlots*আকার(শূন্য*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_আকার, &pHdr[1], pHdr->nTitle ? z : "???");
    যদি( pHdr->nBacktrace ){
      fflush(out);
      pBt = (শূন্য**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  জন্য(i=0; i<NCSIZE-1; i++){
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
পূর্ণসংখ্যা sqlite3_Memdebug_Malloc_গুনতি(){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n_যোগ = 0;
  জন্য(i=0; i<NCSIZE; i++){
    n_যোগ += mem.nAlloc[i];
  }
  ফেরত n_যোগ;
}


#endif /* SQLITE_MEMDEBUG */
