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
#include "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  extern ຈຳນວນເຕັມ backtrace(void**,ຈຳນວນເຕັມ);
  extern void backtrace_symbols_fd(void*const*,ຈຳນວນເຕັມ,ຈຳນວນເຕັມ);
#ບໍ່ດັ່ງນັ້ນ
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#include <stdio.h>

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
sa_nracanaa MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  sa_nracanaa MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  var_na nBacktrace;                    /* Number of backtraces on this alloc */
  var_na nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  ຈຳນວນເຕັມ iForeGuard;                     /* Guard word for sanity */
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
static sa_nracanaa {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  sa_nracanaa MemBlockHdr *pFirst;
  sa_nracanaa MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  ຈຳນວນເຕັມ nBacktrace;
  void (*xBacktrace)(ຈຳນວນເຕັມ, ຈຳນວນເຕັມ, void **);

  /*
  ** Title text to insert in front of each block
  */
  ຈຳນວນເຕັມ nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  var_na zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  ຈຳນວນເຕັມ disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  ຈຳນວນເຕັມ nAlloc[NCSIZE];      /* Total number of allocations */
  ຈຳນວນເຕັມ nCurrent[NCSIZE];    /* Current number of allocations */
  ຈຳນວນເຕັມ mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
static void adjustStats(ຈຳນວນເຕັມ iSize, ຈຳນວນເຕັມ increment){
  ຈຳນວນເຕັມ i = ROUND8(iSize)/8;
  ຖ້າ( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  ຖ້າ( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    ຖ້າ( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }ບໍ່ດັ່ງນັ້ນ{
    mem.nCurrent[i]--;
    assert( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
static sa_nracanaa MemBlockHdr *sqlite3MemsysGetHeader(const void *pAllocation){
  sa_nracanaa MemBlockHdr *p;
  ຈຳນວນເຕັມ *pInt;
  u8 *pU8;
  ຈຳນວນເຕັມ nReserve;

  p = (sa_nracanaa MemBlockHdr*)pAllocation;
  p--;
  assert( p->iForeGuard==(ຈຳນວນເຕັມ)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (ຈຳນວນເຕັມ*)pAllocation;
  pU8 = (u8*)pAllocation;
  assert( pInt[nReserve/parimaa_na(ຈຳນວນເຕັມ)]==(ຈຳນວນເຕັມ)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  ໃນຂະນະທີ່( nReserve-- > p->iSize ) assert( pU8[nReserve]==0x65 );
  ສົ່ງຄືນ p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
static ຈຳນວນເຕັມ sqlite3MemSize(void *p){
  sa_nracanaa MemBlockHdr *pHdr;
  ຖ້າ( !p ){
    ສົ່ງຄືນ 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  ສົ່ງຄືນ (ຈຳນວນເຕັມ)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
static ຈຳນວນເຕັມ sqlite3MemInit(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  assert( (parimaa_na(sa_nracanaa MemBlockHdr)&7) == 0 );
  ຖ້າ( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
static void sqlite3MemShutdown(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
static ຈຳນວນເຕັມ sqlite3MemRoundup(ຈຳນວນເຕັມ n){
  ສົ່ງຄືນ ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
static void randomFill(var_na *pBuf, ຈຳນວນເຕັມ nByte){
  unsigned ຈຳນວນເຕັມ x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  ໃນຂະນະທີ່( nByte >= 4 ){
    x = (x>>1) ^ (-(ຈຳນວນເຕັມ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(ຈຳນວນເຕັມ*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  ໃນຂະນະທີ່( nByte-- > 0 ){
    x = (x>>1) ^ (-(ຈຳນວນເຕັມ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
static void *sqlite3MemMalloc(ຈຳນວນເຕັມ nByte){
  sa_nracanaa MemBlockHdr *pHdr;
  void **pBt;
  var_na *z;
  ຈຳນວນເຕັມ *pInt;
  void *p = 0;
  ຈຳນວນເຕັມ totalSize;
  ຈຳນວນເຕັມ nReserve;
  sqlite3_mutex_enter(mem.mutex);
  assert( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + parimaa_na(*pHdr) + parimaa_na(ຈຳນວນເຕັມ) +
               mem.nBacktrace*parimaa_na(void*) + mem.nTitle;
  p = malloc(totalSize);
  ຖ້າ( p ){
    z = p;
    pBt = (void**)&z[mem.nTitle];
    pHdr = (sa_nracanaa MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    ຖ້າ( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }ບໍ່ດັ່ງນັ້ນ{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    ຖ້າ( mem.nBacktrace ){
      void *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*parimaa_na(void*));
      assert(pBt[0]);
      ຖ້າ( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }ບໍ່ດັ່ງນັ້ນ{
      pHdr->nBacktrace = 0;
    }
    ຖ້າ( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (ຈຳນວນເຕັມ*)&pHdr[1];
    pInt[nReserve/parimaa_na(ຈຳນວນເຕັມ)] = REARGUARD;
    randomFill((var_na*)pInt, nByte);
    memset(((var_na*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (void*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  ສົ່ງຄືນ p; 
}

/*
** Free memory.
*/
static void sqlite3MemFree(void *pPrior){
  sa_nracanaa MemBlockHdr *pHdr;
  void **pBt;
  var_na *z;
  assert( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (void**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  ຖ້າ( pHdr->pPrev ){
    assert( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }ບໍ່ດັ່ງນັ້ນ{
    assert( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  ຖ້າ( pHdr->pNext ){
    assert( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }ບໍ່ດັ່ງນັ້ນ{
    assert( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (var_na*)pBt;
  z -= pHdr->nTitle;
  adjustStats((ຈຳນວນເຕັມ)pHdr->iSize, -1);
  randomFill(z, parimaa_na(void*)*pHdr->nBacktraceSlots + parimaa_na(*pHdr) +
                (ຈຳນວນເຕັມ)pHdr->iSize + parimaa_na(ຈຳນວນເຕັມ) + pHdr->nTitle);
  free(z);
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
static void *sqlite3MemRealloc(void *pPrior, ຈຳນວນເຕັມ nByte){
  sa_nracanaa MemBlockHdr *pOldHdr;
  void *pNew;
  assert( mem.disallow==0 );
  assert( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  ຖ້າ( pNew ){
    memcpy(pNew, pPrior, (ຈຳນວນເຕັມ)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    ຖ້າ( nByte>pOldHdr->iSize ){
      randomFill(&((var_na*)pNew)[pOldHdr->iSize], nByte - (ຈຳນວນເຕັມ)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  ສົ່ງຄືນ pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
void sqlite3MemSetDefault(void){
  static const sqlite3_mem_methods defaultMethods = {
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
void sqlite3MemdebugSetType(void *p, u8 eType){
  ຖ້າ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    sa_nracanaa MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );
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
ຈຳນວນເຕັມ sqlite3MemdebugHasType(const void *p, u8 eType){
  ຈຳນວນເຕັມ rc = 1;
  ຖ້າ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    sa_nracanaa MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ຖ້າ( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  ສົ່ງຄືນ rc;
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
ຈຳນວນເຕັມ sqlite3MemdebugNoType(const void *p, u8 eType){
  ຈຳນວນເຕັມ rc = 1;
  ຖ້າ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    sa_nracanaa MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ຖ້າ( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  ສົ່ງຄືນ rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
void sqlite3MemdebugBacktrace(ຈຳນວນເຕັມ depth){
  ຖ້າ( depth<0 ){ depth = 0; }
  ຖ້າ( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

void sqlite3MemdebugBacktraceCallback(void (*xBacktrace)(ຈຳນວນເຕັມ, ຈຳນວນເຕັມ, void **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
void sqlite3MemdebugSettitle(const var_na *zTitle){
  unsigned ຈຳນວນເຕັມ n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  ຖ້າ( n>=parimaa_na(mem.zTitle) ) n = parimaa_na(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

void sqlite3MemdebugSync(){
  sa_nracanaa MemBlockHdr *pHdr;
  ສຳລັບ(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    void **pBt = (void**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((ຈຳນວນເຕັມ)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
void sqlite3MemdebugDump(const var_na *zFilename){
  FILE *out;
  sa_nracanaa MemBlockHdr *pHdr;
  void **pBt;
  ຈຳນວນເຕັມ i;
  out = fopen(zFilename, "w");
  ຖ້າ( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    ສົ່ງຄືນ;
  }
  ສຳລັບ(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    var_na *z = (var_na*)pHdr;
    z -= pHdr->nBacktraceSlots*parimaa_na(void*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    ຖ້າ( pHdr->nBacktrace ){
      fflush(out);
      pBt = (void**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  ສຳລັບ(i=0; i<NCSIZE-1; i++){
    ຖ້າ( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  ຖ້າ( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
ຈຳນວນເຕັມ sqlite3MemdebugMallocCount(){
  ຈຳນວນເຕັມ i;
  ຈຳນວນເຕັມ nTotal = 0;
  ສຳລັບ(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  ສົ່ງຄືນ nTotal;
}


#endif /* SQLITE_MEMDEBUG */
