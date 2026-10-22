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
#ضمن "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  خارجي صحيح backtrace(فراغ**,صحيح);
  خارجي فراغ backtrace_symbols_fd(فراغ*ثابت*,صحيح,صحيح);
#وإلا
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#ضمن <stdio.h>

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
بنية MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  بنية MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  حرف nBacktrace;                    /* Number of backtraces on this alloc */
  حرف nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  صحيح iForeGuard;                     /* Guard word for sanity */
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
ساكن بنية {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  بنية MemBlockHdr *pFirst;
  بنية MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  صحيح nBacktrace;
  فراغ (*xBacktrace)(صحيح, صحيح, فراغ **);

  /*
  ** Title text to insert in front of each block
  */
  صحيح nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  حرف zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  صحيح disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  صحيح nAlloc[NCSIZE];      /* Total number of allocations */
  صحيح nCurrent[NCSIZE];    /* Current number of allocations */
  صحيح mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ساكن فراغ adjustStats(صحيح iSize, صحيح increment){
  صحيح i = ROUND8(iSize)/8;
  𐡄𐡍( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  𐡄𐡍( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    𐡄𐡍( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }وإلا{
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
ساكن بنية MemBlockHdr *sqlite3MemsysGetHeader(ثابت فراغ *pAllocation){
  بنية MemBlockHdr *p;
  صحيح *pInt;
  u8 *pU8;
  صحيح nReserve;

  p = (بنية MemBlockHdr*)pAllocation;
  p--;
  أكد( p->iForeGuard==(صحيح)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (صحيح*)pAllocation;
  pU8 = (u8*)pAllocation;
  أكد( pInt[nReserve/حجم(صحيح)]==(صحيح)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  طالما( nReserve-- > p->iSize ) أكد( pU8[nReserve]==0x65 );
  أرجع p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ساكن صحيح sqlite3MemSize(فراغ *p){
  بنية MemBlockHdr *pHdr;
  𐡄𐡍( !p ){
    أرجع 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  أرجع (صحيح)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
ساكن صحيح sqlite3MemInit(فراغ *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  أكد( (حجم(بنية MemBlockHdr)&7) == 0 );
  𐡄𐡍( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  أرجع SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ساكن فراغ sqlite3MemShutdown(فراغ *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساكن صحيح sqlite3MemRoundup(صحيح n){
  أرجع ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ساكن فراغ randomFill(حرف *pBuf, صحيح nByte){
  unsigned صحيح x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  طالما( nByte >= 4 ){
    x = (x>>1) ^ (-(صحيح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(صحيح*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  طالما( nByte-- > 0 ){
    x = (x>>1) ^ (-(صحيح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ساكن فراغ *sqlite3MemMalloc(صحيح nByte){
  بنية MemBlockHdr *pHdr;
  فراغ **pBt;
  حرف *z;
  صحيح *pInt;
  فراغ *p = 0;
  صحيح totalSize;
  صحيح nReserve;
  sqlite3_mutex_enter(mem.mutex);
  أكد( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + حجم(*pHdr) + حجم(صحيح) +
               mem.nBacktrace*حجم(فراغ*) + mem.nTitle;
  p = malloc(totalSize);
  𐡄𐡍( p ){
    z = p;
    pBt = (فراغ**)&z[mem.nTitle];
    pHdr = (بنية MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    𐡄𐡍( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }وإلا{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    𐡄𐡍( mem.nBacktrace ){
      فراغ *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*حجم(فراغ*));
      أكد(pBt[0]);
      𐡄𐡍( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }وإلا{
      pHdr->nBacktrace = 0;
    }
    𐡄𐡍( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (صحيح*)&pHdr[1];
    pInt[nReserve/حجم(صحيح)] = REARGUARD;
    randomFill((حرف*)pInt, nByte);
    memset(((حرف*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (فراغ*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  أرجع p; 
}

/*
** Free memory.
*/
ساكن فراغ sqlite3MemFree(فراغ *pPrior){
  بنية MemBlockHdr *pHdr;
  فراغ **pBt;
  حرف *z;
  أكد( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (فراغ**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  𐡄𐡍( pHdr->pPrev ){
    أكد( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }وإلا{
    أكد( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  𐡄𐡍( pHdr->pNext ){
    أكد( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }وإلا{
    أكد( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (حرف*)pBt;
  z -= pHdr->nTitle;
  adjustStats((صحيح)pHdr->iSize, -1);
  randomFill(z, حجم(فراغ*)*pHdr->nBacktraceSlots + حجم(*pHdr) +
                (صحيح)pHdr->iSize + حجم(صحيح) + pHdr->nTitle);
  حرر(z);
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
ساكن فراغ *sqlite3MemRealloc(فراغ *pPrior, صحيح nByte){
  بنية MemBlockHdr *pOldHdr;
  فراغ *pNew;
  أكد( mem.disallow==0 );
  أكد( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  𐡄𐡍( pNew ){
    memcpy(pNew, pPrior, (صحيح)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    𐡄𐡍( nByte>pOldHdr->iSize ){
      randomFill(&((حرف*)pNew)[pOldHdr->iSize], nByte - (صحيح)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  أرجع pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
فراغ sqlite3MemSetDefault(فراغ){
  ساكن ثابت sqlite3_mem_methods defaultMethods = {
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
فراغ sqlite3MemdebugSetType(فراغ *p, u8 eType){
  𐡄𐡍( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    بنية MemBlockHdr *pHdr;
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
صحيح sqlite3MemdebugHasType(ثابت فراغ *p, u8 eType){
  صحيح rc = 1;
  𐡄𐡍( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    بنية MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    𐡄𐡍( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  أرجع rc;
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
صحيح sqlite3MemdebugNoType(ثابت فراغ *p, u8 eType){
  صحيح rc = 1;
  𐡄𐡍( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    بنية MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    𐡄𐡍( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  أرجع rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
فراغ sqlite3MemdebugBacktrace(صحيح depth){
  𐡄𐡍( depth<0 ){ depth = 0; }
  𐡄𐡍( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

فراغ sqlite3MemdebugBacktraceCallback(فراغ (*xBacktrace)(صحيح, صحيح, فراغ **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
فراغ sqlite3MemdebugSettitle(ثابت حرف *zTitle){
  unsigned صحيح n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  𐡄𐡍( n>=حجم(mem.zTitle) ) n = حجم(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

فراغ sqlite3MemdebugSync(){
  بنية MemBlockHdr *pHdr;
  لكل(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    فراغ **pBt = (فراغ**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((صحيح)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
فراغ sqlite3MemdebugDump(ثابت حرف *zFilename){
  FILE *out;
  بنية MemBlockHdr *pHdr;
  فراغ **pBt;
  صحيح i;
  out = fopen(zFilename, "w");
  𐡄𐡍( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    أرجع;
  }
  لكل(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    حرف *z = (حرف*)pHdr;
    z -= pHdr->nBacktraceSlots*حجم(فراغ*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    𐡄𐡍( pHdr->nBacktrace ){
      fflush(out);
      pBt = (فراغ**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  لكل(i=0; i<NCSIZE-1; i++){
    𐡄𐡍( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  𐡄𐡍( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
صحيح sqlite3MemdebugMallocCount(){
  صحيح i;
  صحيح nTotal = 0;
  لكل(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  أرجع nTotal;
}


#endif /* SQLITE_MEMDEBUG */
