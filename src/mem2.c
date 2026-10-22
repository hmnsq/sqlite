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
#அந்தர்பாவ "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  வெளி முழு backtrace(வெறுமை**,முழு);
  வெளி வெறுமை backtrace_symbols_fd(வெறுமை*மாறிலி*,முழு,முழு);
#இல்லையெனில்
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#அந்தர்பாவ <stdio.h>

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
கட்டமைப்பு MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  கட்டமைப்பு MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  எழுத்து nBacktrace;                    /* Number of backtraces on this alloc */
  எழுத்து nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  முழு iForeGuard;                     /* Guard word for sanity */
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
நிலையான கட்டமைப்பு {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  கட்டமைப்பு MemBlockHdr *pFirst;
  கட்டமைப்பு MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  முழு nBacktrace;
  வெறுமை (*xBacktrace)(முழு, முழு, வெறுமை **);

  /*
  ** Title text to insert in front of each block
  */
  முழு nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  எழுத்து zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  முழு disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  முழு nAlloc[NCSIZE];      /* Total number of allocations */
  முழு nCurrent[NCSIZE];    /* Current number of allocations */
  முழு mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
நிலையான வெறுமை adjustStats(முழு iSize, முழு increment){
  முழு i = ROUND8(iSize)/8;
  எனில்( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  எனில்( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    எனில்( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }இல்லையெனில்{
    mem.nCurrent[i]--;
    ப்ரதிஜ்ஞா( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
நிலையான கட்டமைப்பு MemBlockHdr *sqlite3MemsysGetHeader(மாறிலி வெறுமை *pAllocation){
  கட்டமைப்பு MemBlockHdr *p;
  முழு *pInt;
  u8 *pU8;
  முழு nReserve;

  p = (கட்டமைப்பு MemBlockHdr*)pAllocation;
  p--;
  ப்ரதிஜ்ஞா( p->iForeGuard==(முழு)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (முழு*)pAllocation;
  pU8 = (u8*)pAllocation;
  ப்ரதிஜ்ஞா( pInt[nReserve/அளவு(முழு)]==(முழு)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  போது( nReserve-- > p->iSize ) ப்ரதிஜ்ஞா( pU8[nReserve]==0x65 );
  திரும்பு p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
நிலையான முழு sqlite3MemSize(வெறுமை *p){
  கட்டமைப்பு MemBlockHdr *pHdr;
  எனில்( !p ){
    திரும்பு 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  திரும்பு (முழு)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
நிலையான முழு sqlite3MemInit(வெறுமை *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  ப்ரதிஜ்ஞா( (அளவு(கட்டமைப்பு MemBlockHdr)&7) == 0 );
  எனில்( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  திரும்பு SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
நிலையான வெறுமை sqlite3MemShutdown(வெறுமை *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
நிலையான முழு sqlite3MemRoundup(முழு n){
  திரும்பு ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
நிலையான வெறுமை randomFill(எழுத்து *pBuf, முழு nByte){
  unsigned முழு x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  போது( nByte >= 4 ){
    x = (x>>1) ^ (-(முழு)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(முழு*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  போது( nByte-- > 0 ){
    x = (x>>1) ^ (-(முழு)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
நிலையான வெறுமை *sqlite3MemMalloc(முழு nByte){
  கட்டமைப்பு MemBlockHdr *pHdr;
  வெறுமை **pBt;
  எழுத்து *z;
  முழு *pInt;
  வெறுமை *p = 0;
  முழு totalSize;
  முழு nReserve;
  sqlite3_mutex_enter(mem.mutex);
  ப்ரதிஜ்ஞா( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + அளவு(*pHdr) + அளவு(முழு) +
               mem.nBacktrace*அளவு(வெறுமை*) + mem.nTitle;
  p = malloc(totalSize);
  எனில்( p ){
    z = p;
    pBt = (வெறுமை**)&z[mem.nTitle];
    pHdr = (கட்டமைப்பு MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    எனில்( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }இல்லையெனில்{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    எனில்( mem.nBacktrace ){
      வெறுமை *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*அளவு(வெறுமை*));
      ப்ரதிஜ்ஞா(pBt[0]);
      எனில்( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }இல்லையெனில்{
      pHdr->nBacktrace = 0;
    }
    எனில்( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (முழு*)&pHdr[1];
    pInt[nReserve/அளவு(முழு)] = REARGUARD;
    randomFill((எழுத்து*)pInt, nByte);
    memset(((எழுத்து*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (வெறுமை*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  திரும்பு p; 
}

/*
** Free memory.
*/
நிலையான வெறுமை sqlite3MemFree(வெறுமை *pPrior){
  கட்டமைப்பு MemBlockHdr *pHdr;
  வெறுமை **pBt;
  எழுத்து *z;
  ப்ரதிஜ்ஞா( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (வெறுமை**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  எனில்( pHdr->pPrev ){
    ப்ரதிஜ்ஞா( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  எனில்( pHdr->pNext ){
    ப்ரதிஜ்ஞா( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (எழுத்து*)pBt;
  z -= pHdr->nTitle;
  adjustStats((முழு)pHdr->iSize, -1);
  randomFill(z, அளவு(வெறுமை*)*pHdr->nBacktraceSlots + அளவு(*pHdr) +
                (முழு)pHdr->iSize + அளவு(முழு) + pHdr->nTitle);
  விடுவி(z);
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
நிலையான வெறுமை *sqlite3MemRealloc(வெறுமை *pPrior, முழு nByte){
  கட்டமைப்பு MemBlockHdr *pOldHdr;
  வெறுமை *pNew;
  ப்ரதிஜ்ஞா( mem.disallow==0 );
  ப்ரதிஜ்ஞா( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  எனில்( pNew ){
    memcpy(pNew, pPrior, (முழு)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    எனில்( nByte>pOldHdr->iSize ){
      randomFill(&((எழுத்து*)pNew)[pOldHdr->iSize], nByte - (முழு)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  திரும்பு pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
வெறுமை sqlite3MemSetDefault(வெறுமை){
  நிலையான மாறிலி sqlite3_mem_methods defaultMethods = {
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
வெறுமை sqlite3MemdebugSetType(வெறுமை *p, u8 eType){
  எனில்( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    கட்டமைப்பு MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    ப்ரதிஜ்ஞா( pHdr->iForeGuard==FOREGUARD );
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
முழு sqlite3MemdebugHasType(மாறிலி வெறுமை *p, u8 eType){
  முழு rc = 1;
  எனில்( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    கட்டமைப்பு MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    ப்ரதிஜ்ஞா( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    எனில்( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  திரும்பு rc;
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
முழு sqlite3MemdebugNoType(மாறிலி வெறுமை *p, u8 eType){
  முழு rc = 1;
  எனில்( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    கட்டமைப்பு MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    ப்ரதிஜ்ஞா( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    எனில்( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  திரும்பு rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
வெறுமை sqlite3MemdebugBacktrace(முழு depth){
  எனில்( depth<0 ){ depth = 0; }
  எனில்( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

வெறுமை sqlite3MemdebugBacktraceCallback(வெறுமை (*xBacktrace)(முழு, முழு, வெறுமை **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
வெறுமை sqlite3MemdebugSettitle(மாறிலி எழுத்து *zTitle){
  unsigned முழு n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  எனில்( n>=அளவு(mem.zTitle) ) n = அளவு(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

வெறுமை sqlite3MemdebugSync(){
  கட்டமைப்பு MemBlockHdr *pHdr;
  க்கு(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    வெறுமை **pBt = (வெறுமை**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((முழு)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
வெறுமை sqlite3MemdebugDump(மாறிலி எழுத்து *zFilename){
  FILE *out;
  கட்டமைப்பு MemBlockHdr *pHdr;
  வெறுமை **pBt;
  முழு i;
  out = fopen(zFilename, "w");
  எனில்( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    திரும்பு;
  }
  க்கு(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    எழுத்து *z = (எழுத்து*)pHdr;
    z -= pHdr->nBacktraceSlots*அளவு(வெறுமை*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    எனில்( pHdr->nBacktrace ){
      fflush(out);
      pBt = (வெறுமை**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  க்கு(i=0; i<NCSIZE-1; i++){
    எனில்( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  எனில்( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
முழு sqlite3_Memdebug_Malloc_மொத்தம்(){
  முழு i;
  முழு nTotal = 0;
  க்கு(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  திரும்பு nTotal;
}


#endif /* SQLITE_MEMDEBUG */
