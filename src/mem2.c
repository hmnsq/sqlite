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
#അന്തര്ഭാവ "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  ബാഹ്യ പൂർണ്ണസംഖ്യ backtrace(ശൂന്യം**,പൂർണ്ണസംഖ്യ);
  ബാഹ്യ ശൂന്യം backtrace_symbols_fd(ശൂന്യം*സ്ഥിരം*,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ);
#അല്ലെങ്കിൽ
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#അന്തര്ഭാവ <stdio.h>

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
ഘടന MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  ഘടന MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  അക്ഷരം nBacktrace;                    /* Number of backtraces on this alloc */
  അക്ഷരം nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  പൂർണ്ണസംഖ്യ iForeGuard;                     /* Guard word for sanity */
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
സ്ഥിതം ഘടന {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  ഘടന MemBlockHdr *pFirst;
  ഘടന MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  പൂർണ്ണസംഖ്യ nBacktrace;
  ശൂന്യം (*xBacktrace)(പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, ശൂന്യം **);

  /*
  ** Title text to insert in front of each block
  */
  പൂർണ്ണസംഖ്യ nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  അക്ഷരം zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  പൂർണ്ണസംഖ്യ disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  പൂർണ്ണസംഖ്യ nAlloc[NCSIZE];      /* Total number of allocations */
  പൂർണ്ണസംഖ്യ nCurrent[NCSIZE];    /* Current number of allocations */
  പൂർണ്ണസംഖ്യ mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
സ്ഥിതം ശൂന്യം adjustStats(പൂർണ്ണസംഖ്യ iSize, പൂർണ്ണസംഖ്യ increment){
  പൂർണ്ണസംഖ്യ i = ROUND8(iSize)/8;
  എങ്കിൽ( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  എങ്കിൽ( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    എങ്കിൽ( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }അല്ലെങ്കിൽ{
    mem.nCurrent[i]--;
    പ്രതിജ്ഞാ( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
സ്ഥിതം ഘടന MemBlockHdr *sqlite3MemsysGetHeader(സ്ഥിരം ശൂന്യം *pAllocation){
  ഘടന MemBlockHdr *p;
  പൂർണ്ണസംഖ്യ *pInt;
  u8 *pU8;
  പൂർണ്ണസംഖ്യ nReserve;

  p = (ഘടന MemBlockHdr*)pAllocation;
  p--;
  പ്രതിജ്ഞാ( p->iForeGuard==(പൂർണ്ണസംഖ്യ)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (പൂർണ്ണസംഖ്യ*)pAllocation;
  pU8 = (u8*)pAllocation;
  പ്രതിജ്ഞാ( pInt[nReserve/വലുപ്പം(പൂർണ്ണസംഖ്യ)]==(പൂർണ്ണസംഖ്യ)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  ആകുമ്പോൾ( nReserve-- > p->iSize ) പ്രതിജ്ഞാ( pU8[nReserve]==0x65 );
  തിരികെ p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3MemSize(ശൂന്യം *p){
  ഘടന MemBlockHdr *pHdr;
  എങ്കിൽ( !p ){
    തിരികെ 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  തിരികെ (പൂർണ്ണസംഖ്യ)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3MemInit(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  പ്രതിജ്ഞാ( (വലുപ്പം(ഘടന MemBlockHdr)&7) == 0 );
  എങ്കിൽ( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  തിരികെ SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
സ്ഥിതം ശൂന്യം sqlite3MemShutdown(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3MemRoundup(പൂർണ്ണസംഖ്യ n){
  തിരികെ ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
സ്ഥിതം ശൂന്യം randomFill(അക്ഷരം *pBuf, പൂർണ്ണസംഖ്യ nByte){
  unsigned പൂർണ്ണസംഖ്യ x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  ആകുമ്പോൾ( nByte >= 4 ){
    x = (x>>1) ^ (-(പൂർണ്ണസംഖ്യ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(പൂർണ്ണസംഖ്യ*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  ആകുമ്പോൾ( nByte-- > 0 ){
    x = (x>>1) ^ (-(പൂർണ്ണസംഖ്യ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
സ്ഥിതം ശൂന്യം *sqlite3MemMalloc(പൂർണ്ണസംഖ്യ nByte){
  ഘടന MemBlockHdr *pHdr;
  ശൂന്യം **pBt;
  അക്ഷരം *z;
  പൂർണ്ണസംഖ്യ *pInt;
  ശൂന്യം *p = 0;
  പൂർണ്ണസംഖ്യ totalSize;
  പൂർണ്ണസംഖ്യ nReserve;
  sqlite3_mutex_enter(mem.mutex);
  പ്രതിജ്ഞാ( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + വലുപ്പം(*pHdr) + വലുപ്പം(പൂർണ്ണസംഖ്യ) +
               mem.nBacktrace*വലുപ്പം(ശൂന്യം*) + mem.nTitle;
  p = malloc(totalSize);
  എങ്കിൽ( p ){
    z = p;
    pBt = (ശൂന്യം**)&z[mem.nTitle];
    pHdr = (ഘടന MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    എങ്കിൽ( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }അല്ലെങ്കിൽ{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    എങ്കിൽ( mem.nBacktrace ){
      ശൂന്യം *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*വലുപ്പം(ശൂന്യം*));
      പ്രതിജ്ഞാ(pBt[0]);
      എങ്കിൽ( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }അല്ലെങ്കിൽ{
      pHdr->nBacktrace = 0;
    }
    എങ്കിൽ( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (പൂർണ്ണസംഖ്യ*)&pHdr[1];
    pInt[nReserve/വലുപ്പം(പൂർണ്ണസംഖ്യ)] = REARGUARD;
    randomFill((അക്ഷരം*)pInt, nByte);
    memset(((അക്ഷരം*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (ശൂന്യം*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  തിരികെ p; 
}

/*
** Free memory.
*/
സ്ഥിതം ശൂന്യം sqlite3MemFree(ശൂന്യം *pPrior){
  ഘടന MemBlockHdr *pHdr;
  ശൂന്യം **pBt;
  അക്ഷരം *z;
  പ്രതിജ്ഞാ( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (ശൂന്യം**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  എങ്കിൽ( pHdr->pPrev ){
    പ്രതിജ്ഞാ( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  എങ്കിൽ( pHdr->pNext ){
    പ്രതിജ്ഞാ( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (അക്ഷരം*)pBt;
  z -= pHdr->nTitle;
  adjustStats((പൂർണ്ണസംഖ്യ)pHdr->iSize, -1);
  randomFill(z, വലുപ്പം(ശൂന്യം*)*pHdr->nBacktraceSlots + വലുപ്പം(*pHdr) +
                (പൂർണ്ണസംഖ്യ)pHdr->iSize + വലുപ്പം(പൂർണ്ണസംഖ്യ) + pHdr->nTitle);
  സ്വതന്ത്രം(z);
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
സ്ഥിതം ശൂന്യം *sqlite3MemRealloc(ശൂന്യം *pPrior, പൂർണ്ണസംഖ്യ nByte){
  ഘടന MemBlockHdr *pOldHdr;
  ശൂന്യം *pNew;
  പ്രതിജ്ഞാ( mem.disallow==0 );
  പ്രതിജ്ഞാ( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  എങ്കിൽ( pNew ){
    memcpy(pNew, pPrior, (പൂർണ്ണസംഖ്യ)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    എങ്കിൽ( nByte>pOldHdr->iSize ){
      randomFill(&((അക്ഷരം*)pNew)[pOldHdr->iSize], nByte - (പൂർണ്ണസംഖ്യ)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  തിരികെ pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ശൂന്യം sqlite3MemSetDefault(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം sqlite3_mem_methods defaultMethods = {
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
ശൂന്യം sqlite3MemdebugSetType(ശൂന്യം *p, u8 eType){
  എങ്കിൽ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ഘടന MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    പ്രതിജ്ഞാ( pHdr->iForeGuard==FOREGUARD );
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
പൂർണ്ണസംഖ്യ sqlite3MemdebugHasType(സ്ഥിരം ശൂന്യം *p, u8 eType){
  പൂർണ്ണസംഖ്യ rc = 1;
  എങ്കിൽ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ഘടന MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    പ്രതിജ്ഞാ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    എങ്കിൽ( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sqlite3MemdebugNoType(സ്ഥിരം ശൂന്യം *p, u8 eType){
  പൂർണ്ണസംഖ്യ rc = 1;
  എങ്കിൽ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ഘടന MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    പ്രതിജ്ഞാ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    എങ്കിൽ( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  തിരികെ rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
ശൂന്യം sqlite3MemdebugBacktrace(പൂർണ്ണസംഖ്യ depth){
  എങ്കിൽ( depth<0 ){ depth = 0; }
  എങ്കിൽ( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

ശൂന്യം sqlite3MemdebugBacktraceCallback(ശൂന്യം (*xBacktrace)(പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, ശൂന്യം **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
ശൂന്യം sqlite3MemdebugSettitle(സ്ഥിരം അക്ഷരം *zTitle){
  unsigned പൂർണ്ണസംഖ്യ n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  എങ്കിൽ( n>=വലുപ്പം(mem.zTitle) ) n = വലുപ്പം(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

ശൂന്യം sqlite3MemdebugSync(){
  ഘടന MemBlockHdr *pHdr;
  വേണ്ടി(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    ശൂന്യം **pBt = (ശൂന്യം**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((പൂർണ്ണസംഖ്യ)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ശൂന്യം sqlite3MemdebugDump(സ്ഥിരം അക്ഷരം *zFilename){
  FILE *out;
  ഘടന MemBlockHdr *pHdr;
  ശൂന്യം **pBt;
  പൂർണ്ണസംഖ്യ i;
  out = fopen(zFilename, "w");
  എങ്കിൽ( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    തിരികെ;
  }
  വേണ്ടി(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    അക്ഷരം *z = (അക്ഷരം*)pHdr;
    z -= pHdr->nBacktraceSlots*വലുപ്പം(ശൂന്യം*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    എങ്കിൽ( pHdr->nBacktrace ){
      fflush(out);
      pBt = (ശൂന്യം**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  വേണ്ടി(i=0; i<NCSIZE-1; i++){
    എങ്കിൽ( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  എങ്കിൽ( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
പൂർണ്ണസംഖ്യ sqlite3_Memdebug_Malloc_എണ്ണം(){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ nTotal = 0;
  വേണ്ടി(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  തിരികെ nTotal;
}


#endif /* SQLITE_MEMDEBUG */
