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
  extern ціле backtrace(void**,ціле);
  extern void backtrace_symbols_fd(void*const*,ціле,ціле);
#інакше
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
struct MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  struct MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  char nBacktrace;                    /* Number of backtraces on this alloc */
  char nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  ціле iForeGuard;                     /* Guard word for sanity */
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
static struct {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  struct MemBlockHdr *pFirst;
  struct MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  ціле nBacktrace;
  void (*xBacktrace)(ціле, ціле, void **);

  /*
  ** Title text to insert in front of each block
  */
  ціле nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  char zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  ціле disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  ціле nAlloc[NCSIZE];      /* Total number of allocations */
  ціле nCurrent[NCSIZE];    /* Current number of allocations */
  ціле mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
static void adjustStats(ціле iSize, ціле increment){
  ціле i = ROUND8(iSize)/8;
  якщо( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  якщо( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    якщо( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }інакше{
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
static struct MemBlockHdr *sqlite3MemsysGetHeader(const void *pAllocation){
  struct MemBlockHdr *p;
  ціле *pInt;
  u8 *pU8;
  ціле nReserve;

  p = (struct MemBlockHdr*)pAllocation;
  p--;
  assert( p->iForeGuard==(ціле)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (ціле*)pAllocation;
  pU8 = (u8*)pAllocation;
  assert( pInt[nReserve/sizeof(ціле)]==(ціле)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  поки( nReserve-- > p->iSize ) assert( pU8[nReserve]==0x65 );
  повернути p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
static ціле sqlite3MemSize(void *p){
  struct MemBlockHdr *pHdr;
  якщо( !p ){
    повернути 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  повернути (ціле)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
static ціле sqlite3MemInit(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  assert( (sizeof(struct MemBlockHdr)&7) == 0 );
  якщо( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  повернути SQLITE_OK;
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
static ціле sqlite3MemRoundup(ціле n){
  повернути ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
static void randomFill(char *pBuf, ціле nByte){
  unsigned ціле x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  поки( nByte >= 4 ){
    x = (x>>1) ^ (-(ціле)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(ціле*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  поки( nByte-- > 0 ){
    x = (x>>1) ^ (-(ціле)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
static void *sqlite3MemMalloc(ціле nByte){
  struct MemBlockHdr *pHdr;
  void **pBt;
  char *z;
  ціле *pInt;
  void *p = 0;
  ціле totalSize;
  ціле nReserve;
  sqlite3_mutex_enter(mem.mutex);
  assert( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + sizeof(*pHdr) + sizeof(ціле) +
               mem.nBacktrace*sizeof(void*) + mem.nTitle;
  p = malloc(totalSize);
  якщо( p ){
    z = p;
    pBt = (void**)&z[mem.nTitle];
    pHdr = (struct MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    якщо( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }інакше{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    якщо( mem.nBacktrace ){
      void *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*sizeof(void*));
      assert(pBt[0]);
      якщо( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }інакше{
      pHdr->nBacktrace = 0;
    }
    якщо( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (ціле*)&pHdr[1];
    pInt[nReserve/sizeof(ціле)] = REARGUARD;
    randomFill((char*)pInt, nByte);
    memset(((char*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (void*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  повернути p; 
}

/*
** Free memory.
*/
static void sqlite3MemFree(void *pPrior){
  struct MemBlockHdr *pHdr;
  void **pBt;
  char *z;
  assert( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (void**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  якщо( pHdr->pPrev ){
    assert( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }інакше{
    assert( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  якщо( pHdr->pNext ){
    assert( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }інакше{
    assert( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (char*)pBt;
  z -= pHdr->nTitle;
  adjustStats((ціле)pHdr->iSize, -1);
  randomFill(z, sizeof(void*)*pHdr->nBacktraceSlots + sizeof(*pHdr) +
                (ціле)pHdr->iSize + sizeof(ціле) + pHdr->nTitle);
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
static void *sqlite3MemRealloc(void *pPrior, ціле nByte){
  struct MemBlockHdr *pOldHdr;
  void *pNew;
  assert( mem.disallow==0 );
  assert( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  якщо( pNew ){
    memcpy(pNew, pPrior, (ціле)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    якщо( nByte>pOldHdr->iSize ){
      randomFill(&((char*)pNew)[pOldHdr->iSize], nByte - (ціле)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  повернути pNew;
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
  якщо( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    struct MemBlockHdr *pHdr;
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
ціле sqlite3MemdebugHasType(const void *p, u8 eType){
  ціле rc = 1;
  якщо( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    struct MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    якщо( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  повернути rc;
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
ціле sqlite3MemdebugNoType(const void *p, u8 eType){
  ціле rc = 1;
  якщо( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    struct MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    якщо( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  повернути rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
void sqlite3MemdebugBacktrace(ціле depth){
  якщо( depth<0 ){ depth = 0; }
  якщо( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

void sqlite3MemdebugBacktraceCallback(void (*xBacktrace)(ціле, ціле, void **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
void sqlite3MemdebugSettitle(const char *zTitle){
  unsigned ціле n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  якщо( n>=sizeof(mem.zTitle) ) n = sizeof(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

void sqlite3MemdebugSync(){
  struct MemBlockHdr *pHdr;
  для(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    void **pBt = (void**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((ціле)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
void sqlite3MemdebugDump(const char *zFilename){
  FILE *out;
  struct MemBlockHdr *pHdr;
  void **pBt;
  ціле i;
  out = fopen(zFilename, "w");
  якщо( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    повернути;
  }
  для(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    char *z = (char*)pHdr;
    z -= pHdr->nBacktraceSlots*sizeof(void*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    якщо( pHdr->nBacktrace ){
      fflush(out);
      pBt = (void**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  для(i=0; i<NCSIZE-1; i++){
    якщо( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  якщо( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
ціле sqlite3MemdebugMallocCount(){
  ціле i;
  ціле nTotal = 0;
  для(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  повернути nTotal;
}


#endif /* SQLITE_MEMDEBUG */
