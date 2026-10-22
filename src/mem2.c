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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ backtrace(ಶೂನ್ಯ**,ಪೂರ್ಣಾಙ್ಕ);
  ಬಾಹ್ಯ ಶೂನ್ಯ backtrace_symbols_fd(ಶೂನ್ಯ*ಧ್ರುವ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
#ಅನ್ಯಥಾ
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#ಅನ್ತರ್ಭಾವ <stdio.h>

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
ಸಂರಚನಾ MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  ಸಂರಚನಾ MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  ವರ್ಣ nBacktrace;                    /* Number of backtraces on this alloc */
  ವರ್ಣ nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  ಪೂರ್ಣಾಙ್ಕ iForeGuard;                     /* Guard word for sanity */
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
ಸ್ಥಿರ ಸಂರಚನಾ {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  ಸಂರಚನಾ MemBlockHdr *pFirst;
  ಸಂರಚನಾ MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  ಪೂರ್ಣಾಙ್ಕ nBacktrace;
  ಶೂನ್ಯ (*xBacktrace)(ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ **);

  /*
  ** Title text to insert in front of each block
  */
  ಪೂರ್ಣಾಙ್ಕ nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  ವರ್ಣ zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  ಪೂರ್ಣಾಙ್ಕ disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  ಪೂರ್ಣಾಙ್ಕ nAlloc[NCSIZE];      /* Total number of allocations */
  ಪೂರ್ಣಾಙ್ಕ nCurrent[NCSIZE];    /* Current number of allocations */
  ಪೂರ್ಣಾಙ್ಕ mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjustStats(ಪೂರ್ಣಾಙ್ಕ iSize, ಪೂರ್ಣಾಙ್ಕ increment){
  ಪೂರ್ಣಾಙ್ಕ i = ROUND8(iSize)/8;
  ಯದಿ( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  ಯದಿ( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    ಯದಿ( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }ಅನ್ಯಥಾ{
    mem.nCurrent[i]--;
    ಪ್ರತಿಜ್ಞಾ( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
ಸ್ಥಿರ ಸಂರಚನಾ MemBlockHdr *sqlite3MemsysGetHeader(ಧ್ರುವ ಶೂನ್ಯ *pAllocation){
  ಸಂರಚನಾ MemBlockHdr *p;
  ಪೂರ್ಣಾಙ್ಕ *pInt;
  u8 *pU8;
  ಪೂರ್ಣಾಙ್ಕ nReserve;

  p = (ಸಂರಚನಾ MemBlockHdr*)pAllocation;
  p--;
  ಪ್ರತಿಜ್ಞಾ( p->iForeGuard==(ಪೂರ್ಣಾಙ್ಕ)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (ಪೂರ್ಣಾಙ್ಕ*)pAllocation;
  pU8 = (u8*)pAllocation;
  ಪ್ರತಿಜ್ಞಾ( pInt[nReserve/ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)]==(ಪೂರ್ಣಾಙ್ಕ)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  ಯಾವತ್( nReserve-- > p->iSize ) ಪ್ರತಿಜ್ಞಾ( pU8[nReserve]==0x65 );
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3MemSize(ಶೂನ್ಯ *p){
  ಸಂರಚನಾ MemBlockHdr *pHdr;
  ಯದಿ( !p ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3MemInit(ಶೂನ್ಯ *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  ಪ್ರತಿಜ್ಞಾ( (ಪರಿಮಾಣ(ಸಂರಚನಾ MemBlockHdr)&7) == 0 );
  ಯದಿ( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemShutdown(ಶೂನ್ಯ *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3MemRoundup(ಪೂರ್ಣಾಙ್ಕ n){
  ಪ್ರತ್ಯಾಗಮ ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ಸ್ಥಿರ ಶೂನ್ಯ randomFill(ವರ್ಣ *pBuf, ಪೂರ್ಣಾಙ್ಕ nByte){
  unsigned ಪೂರ್ಣಾಙ್ಕ x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  ಯಾವತ್( nByte >= 4 ){
    x = (x>>1) ^ (-(ಪೂರ್ಣಾಙ್ಕ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(ಪೂರ್ಣಾಙ್ಕ*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  ಯಾವತ್( nByte-- > 0 ){
    x = (x>>1) ^ (-(ಪೂರ್ಣಾಙ್ಕ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemMalloc(ಪೂರ್ಣಾಙ್ಕ nByte){
  ಸಂರಚನಾ MemBlockHdr *pHdr;
  ಶೂನ್ಯ **pBt;
  ವರ್ಣ *z;
  ಪೂರ್ಣಾಙ್ಕ *pInt;
  ಶೂನ್ಯ *p = 0;
  ಪೂರ್ಣಾಙ್ಕ totalSize;
  ಪೂರ್ಣಾಙ್ಕ nReserve;
  sqlite3_mutex_enter(mem.mutex);
  ಪ್ರತಿಜ್ಞಾ( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + ಪರಿಮಾಣ(*pHdr) + ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) +
               mem.nBacktrace*ಪರಿಮಾಣ(ಶೂನ್ಯ*) + mem.nTitle;
  p = malloc(totalSize);
  ಯದಿ( p ){
    z = p;
    pBt = (ಶೂನ್ಯ**)&z[mem.nTitle];
    pHdr = (ಸಂರಚನಾ MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    ಯದಿ( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }ಅನ್ಯಥಾ{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    ಯದಿ( mem.nBacktrace ){
      ಶೂನ್ಯ *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*ಪರಿಮಾಣ(ಶೂನ್ಯ*));
      ಪ್ರತಿಜ್ಞಾ(pBt[0]);
      ಯದಿ( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }ಅನ್ಯಥಾ{
      pHdr->nBacktrace = 0;
    }
    ಯದಿ( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (ಪೂರ್ಣಾಙ್ಕ*)&pHdr[1];
    pInt[nReserve/ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)] = REARGUARD;
    randomFill((ವರ್ಣ*)pInt, nByte);
    memset(((ವರ್ಣ*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (ಶೂನ್ಯ*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  ಪ್ರತ್ಯಾಗಮ p; 
}

/*
** Free memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemFree(ಶೂನ್ಯ *pPrior){
  ಸಂರಚನಾ MemBlockHdr *pHdr;
  ಶೂನ್ಯ **pBt;
  ವರ್ಣ *z;
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (ಶೂನ್ಯ**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  ಯದಿ( pHdr->pPrev ){
    ಪ್ರತಿಜ್ಞಾ( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  ಯದಿ( pHdr->pNext ){
    ಪ್ರತಿಜ್ಞಾ( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (ವರ್ಣ*)pBt;
  z -= pHdr->nTitle;
  adjustStats((ಪೂರ್ಣಾಙ್ಕ)pHdr->iSize, -1);
  randomFill(z, ಪರಿಮಾಣ(ಶೂನ್ಯ*)*pHdr->nBacktraceSlots + ಪರಿಮಾಣ(*pHdr) +
                (ಪೂರ್ಣಾಙ್ಕ)pHdr->iSize + ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) + pHdr->nTitle);
  ಮುಕ್ತ(z);
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
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemRealloc(ಶೂನ್ಯ *pPrior, ಪೂರ್ಣಾಙ್ಕ nByte){
  ಸಂರಚನಾ MemBlockHdr *pOldHdr;
  ಶೂನ್ಯ *pNew;
  ಪ್ರತಿಜ್ಞಾ( mem.disallow==0 );
  ಪ್ರತಿಜ್ಞಾ( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  ಯದಿ( pNew ){
    memcpy(pNew, pPrior, (ಪೂರ್ಣಾಙ್ಕ)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    ಯದಿ( nByte>pOldHdr->iSize ){
      randomFill(&((ವರ್ಣ*)pNew)[pOldHdr->iSize], nByte - (ಪೂರ್ಣಾಙ್ಕ)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ಶೂನ್ಯ sqlite3MemSetDefault(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_mem_methods defaultMethods = {
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
ಶೂನ್ಯ sqlite3MemdebugSetType(ಶೂನ್ಯ *p, u8 eType){
  ಯದಿ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ಸಂರಚನಾ MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    ಪ್ರತಿಜ್ಞಾ( pHdr->iForeGuard==FOREGUARD );
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
ಪೂರ್ಣಾಙ್ಕ sqlite3MemdebugHasType(ಧ್ರುವ ಶೂನ್ಯ *p, u8 eType){
  ಪೂರ್ಣಾಙ್ಕ rc = 1;
  ಯದಿ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ಸಂರಚನಾ MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    ಪ್ರತಿಜ್ಞಾ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ಯದಿ( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3MemdebugNoType(ಧ್ರುವ ಶೂನ್ಯ *p, u8 eType){
  ಪೂರ್ಣಾಙ್ಕ rc = 1;
  ಯದಿ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ಸಂರಚನಾ MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    ಪ್ರತಿಜ್ಞಾ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ಯದಿ( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
ಶೂನ್ಯ sqlite3MemdebugBacktrace(ಪೂರ್ಣಾಙ್ಕ depth){
  ಯದಿ( depth<0 ){ depth = 0; }
  ಯದಿ( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

ಶೂನ್ಯ sqlite3MemdebugBacktraceCallback(ಶೂನ್ಯ (*xBacktrace)(ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
ಶೂನ್ಯ sqlite3MemdebugSettitle(ಧ್ರುವ ವರ್ಣ *zTitle){
  unsigned ಪೂರ್ಣಾಙ್ಕ n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  ಯದಿ( n>=ಪರಿಮಾಣ(mem.zTitle) ) n = ಪರಿಮಾಣ(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

ಶೂನ್ಯ sqlite3MemdebugSync(){
  ಸಂರಚನಾ MemBlockHdr *pHdr;
  ಪ್ರತಿ(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    ಶೂನ್ಯ **pBt = (ಶೂನ್ಯ**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((ಪೂರ್ಣಾಙ್ಕ)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ಶೂನ್ಯ sqlite3MemdebugDump(ಧ್ರುವ ವರ್ಣ *zFilename){
  FILE *out;
  ಸಂರಚನಾ MemBlockHdr *pHdr;
  ಶೂನ್ಯ **pBt;
  ಪೂರ್ಣಾಙ್ಕ i;
  out = fopen(zFilename, "w");
  ಯದಿ( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿ(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    ವರ್ಣ *z = (ವರ್ಣ*)pHdr;
    z -= pHdr->nBacktraceSlots*ಪರಿಮಾಣ(ಶೂನ್ಯ*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    ಯದಿ( pHdr->nBacktrace ){
      fflush(out);
      pBt = (ಶೂನ್ಯ**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  ಪ್ರತಿ(i=0; i<NCSIZE-1; i++){
    ಯದಿ( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  ಯದಿ( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_Memdebug_Malloc_ಎಣಿಕೆ(){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ nTotal = 0;
  ಪ್ರತಿ(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  ಪ್ರತ್ಯಾಗಮ nTotal;
}


#endif /* SQLITE_MEMDEBUG */
