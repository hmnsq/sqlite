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
  extern цео backtrace(void**,цео);
  extern void backtrace_symbols_fd(void*const*,цео,цео);
#инако
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
  цео iForeGuard;                     /* Guard word for sanity */
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
  цео nBacktrace;
  void (*xBacktrace)(цео, цео, void **);

  /*
  ** Title text to insert in front of each block
  */
  цео nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  char zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  цео disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  цео nAlloc[NCSIZE];      /* Total number of allocations */
  цео nCurrent[NCSIZE];    /* Current number of allocations */
  цео mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
static void adjustStats(цео iSize, цео increment){
  цео i = ROUND8(iSize)/8;
  ако( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  ако( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    ако( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }инако{
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
  цео *pInt;
  u8 *pU8;
  цео nReserve;

  p = (struct MemBlockHdr*)pAllocation;
  p--;
  assert( p->iForeGuard==(цео)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (цео*)pAllocation;
  pU8 = (u8*)pAllocation;
  assert( pInt[nReserve/sizeof(цео)]==(цео)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  док( nReserve-- > p->iSize ) assert( pU8[nReserve]==0x65 );
  врати p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
static цео sqlite3MemSize(void *p){
  struct MemBlockHdr *pHdr;
  ако( !p ){
    врати 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  врати (цео)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
static цео sqlite3MemInit(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  assert( (sizeof(struct MemBlockHdr)&7) == 0 );
  ако( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  врати SQLITE_OK;
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
static цео sqlite3MemRoundup(цео n){
  врати ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
static void randomFill(char *pBuf, цео nByte){
  unsigned цео x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  док( nByte >= 4 ){
    x = (x>>1) ^ (-(цео)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(цео*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  док( nByte-- > 0 ){
    x = (x>>1) ^ (-(цео)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
static void *sqlite3MemMalloc(цео nByte){
  struct MemBlockHdr *pHdr;
  void **pBt;
  char *z;
  цео *pInt;
  void *p = 0;
  цео totalSize;
  цео nReserve;
  sqlite3_mutex_enter(mem.mutex);
  assert( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + sizeof(*pHdr) + sizeof(цео) +
               mem.nBacktrace*sizeof(void*) + mem.nTitle;
  p = malloc(totalSize);
  ако( p ){
    z = p;
    pBt = (void**)&z[mem.nTitle];
    pHdr = (struct MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    ако( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }инако{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    ако( mem.nBacktrace ){
      void *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*sizeof(void*));
      assert(pBt[0]);
      ако( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }инако{
      pHdr->nBacktrace = 0;
    }
    ако( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (цео*)&pHdr[1];
    pInt[nReserve/sizeof(цео)] = REARGUARD;
    randomFill((char*)pInt, nByte);
    memset(((char*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (void*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  врати p; 
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
  ако( pHdr->pPrev ){
    assert( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }инако{
    assert( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  ако( pHdr->pNext ){
    assert( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }инако{
    assert( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (char*)pBt;
  z -= pHdr->nTitle;
  adjustStats((цео)pHdr->iSize, -1);
  randomFill(z, sizeof(void*)*pHdr->nBacktraceSlots + sizeof(*pHdr) +
                (цео)pHdr->iSize + sizeof(цео) + pHdr->nTitle);
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
static void *sqlite3MemRealloc(void *pPrior, цео nByte){
  struct MemBlockHdr *pOldHdr;
  void *pNew;
  assert( mem.disallow==0 );
  assert( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  ако( pNew ){
    memcpy(pNew, pPrior, (цео)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    ако( nByte>pOldHdr->iSize ){
      randomFill(&((char*)pNew)[pOldHdr->iSize], nByte - (цео)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  врати pNew;
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
  ако( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
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
цео sqlite3MemdebugHasType(const void *p, u8 eType){
  цео rc = 1;
  ако( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    struct MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ако( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  врати rc;
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
цео sqlite3MemdebugNoType(const void *p, u8 eType){
  цео rc = 1;
  ако( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    struct MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    assert( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ако( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  врати rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
void sqlite3MemdebugBacktrace(цео depth){
  ако( depth<0 ){ depth = 0; }
  ако( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

void sqlite3MemdebugBacktraceCallback(void (*xBacktrace)(цео, цео, void **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
void sqlite3MemdebugSettitle(const char *zTitle){
  unsigned цео n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  ако( n>=sizeof(mem.zTitle) ) n = sizeof(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

void sqlite3MemdebugSync(){
  struct MemBlockHdr *pHdr;
  за(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    void **pBt = (void**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((цео)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
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
  цео i;
  out = fopen(zFilename, "w");
  ако( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    врати;
  }
  за(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    char *z = (char*)pHdr;
    z -= pHdr->nBacktraceSlots*sizeof(void*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    ако( pHdr->nBacktrace ){
      fflush(out);
      pBt = (void**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  за(i=0; i<NCSIZE-1; i++){
    ако( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  ако( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
цео sqlite3MemdebugMallocCount(){
  цео i;
  цео nTotal = 0;
  за(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  врати nTotal;
}


#endif /* SQLITE_MEMDEBUG */
