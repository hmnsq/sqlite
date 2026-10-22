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
#અન્તર્ભાવ "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  બાહ્ય પૂર્ણાંક backtrace(શૂન્ય**,પૂર્ણાંક);
  બાહ્ય શૂન્ય backtrace_symbols_fd(શૂન્ય*અચલ*,પૂર્ણાંક,પૂર્ણાંક);
#નહીંતર
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#અન્તર્ભાવ <stdio.h>

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
સંરચના MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  સંરચના MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  અક્ષર nBacktrace;                    /* Number of backtraces on this alloc */
  અક્ષર nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  પૂર્ણાંક iForeGuard;                     /* Guard word for sanity */
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
સ્થિર સંરચના {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  સંરચના MemBlockHdr *pFirst;
  સંરચના MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  પૂર્ણાંક nBacktrace;
  શૂન્ય (*xBacktrace)(પૂર્ણાંક, પૂર્ણાંક, શૂન્ય **);

  /*
  ** Title text to insert in front of each block
  */
  પૂર્ણાંક nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  અક્ષર zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  પૂર્ણાંક disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  પૂર્ણાંક nAlloc[NCSIZE];      /* Total number of allocations */
  પૂર્ણાંક nCurrent[NCSIZE];    /* Current number of allocations */
  પૂર્ણાંક mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
સ્થિર શૂન્ય adjustStats(પૂર્ણાંક iSize, પૂર્ણાંક increment){
  પૂર્ણાંક i = ROUND8(iSize)/8;
  જો( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  જો( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    જો( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }નહીંતર{
    mem.nCurrent[i]--;
    પ્રતિજ્ઞા( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
સ્થિર સંરચના MemBlockHdr *sqlite3MemsysGetHeader(અચલ શૂન્ય *pAllocation){
  સંરચના MemBlockHdr *p;
  પૂર્ણાંક *pInt;
  u8 *pU8;
  પૂર્ણાંક nReserve;

  p = (સંરચના MemBlockHdr*)pAllocation;
  p--;
  પ્રતિજ્ઞા( p->iForeGuard==(પૂર્ણાંક)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (પૂર્ણાંક*)pAllocation;
  pU8 = (u8*)pAllocation;
  પ્રતિજ્ઞા( pInt[nReserve/કદ(પૂર્ણાંક)]==(પૂર્ણાંક)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  જ્યારે( nReserve-- > p->iSize ) પ્રતિજ્ઞા( pU8[nReserve]==0x65 );
  પાછા p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
સ્થિર પૂર્ણાંક sqlite3MemSize(શૂન્ય *p){
  સંરચના MemBlockHdr *pHdr;
  જો( !p ){
    પાછા 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  પાછા (પૂર્ણાંક)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
સ્થિર પૂર્ણાંક sqlite3MemInit(શૂન્ય *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  પ્રતિજ્ઞા( (કદ(સંરચના MemBlockHdr)&7) == 0 );
  જો( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  પાછા SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
સ્થિર શૂન્ય sqlite3MemShutdown(શૂન્ય *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
સ્થિર પૂર્ણાંક sqlite3MemRoundup(પૂર્ણાંક n){
  પાછા ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
સ્થિર શૂન્ય randomFill(અક્ષર *pBuf, પૂર્ણાંક nByte){
  unsigned પૂર્ણાંક x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  જ્યારે( nByte >= 4 ){
    x = (x>>1) ^ (-(પૂર્ણાંક)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(પૂર્ણાંક*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  જ્યારે( nByte-- > 0 ){
    x = (x>>1) ^ (-(પૂર્ણાંક)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
સ્થિર શૂન્ય *sqlite3MemMalloc(પૂર્ણાંક nByte){
  સંરચના MemBlockHdr *pHdr;
  શૂન્ય **pBt;
  અક્ષર *z;
  પૂર્ણાંક *pInt;
  શૂન્ય *p = 0;
  પૂર્ણાંક totalSize;
  પૂર્ણાંક nReserve;
  sqlite3_mutex_enter(mem.mutex);
  પ્રતિજ્ઞા( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + કદ(*pHdr) + કદ(પૂર્ણાંક) +
               mem.nBacktrace*કદ(શૂન્ય*) + mem.nTitle;
  p = malloc(totalSize);
  જો( p ){
    z = p;
    pBt = (શૂન્ય**)&z[mem.nTitle];
    pHdr = (સંરચના MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    જો( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }નહીંતર{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    જો( mem.nBacktrace ){
      શૂન્ય *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      memcpy(pBt, &aAddr[1], pHdr->nBacktrace*કદ(શૂન્ય*));
      પ્રતિજ્ઞા(pBt[0]);
      જો( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }નહીંતર{
      pHdr->nBacktrace = 0;
    }
    જો( mem.nTitle ){
      memcpy(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (પૂર્ણાંક*)&pHdr[1];
    pInt[nReserve/કદ(પૂર્ણાંક)] = REARGUARD;
    randomFill((અક્ષર*)pInt, nByte);
    memset(((અક્ષર*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (શૂન્ય*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  પાછા p; 
}

/*
** Free memory.
*/
સ્થિર શૂન્ય sqlite3MemFree(શૂન્ય *pPrior){
  સંરચના MemBlockHdr *pHdr;
  શૂન્ય **pBt;
  અક્ષર *z;
  પ્રતિજ્ઞા( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (શૂન્ય**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  જો( pHdr->pPrev ){
    પ્રતિજ્ઞા( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }નહીંતર{
    પ્રતિજ્ઞા( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  જો( pHdr->pNext ){
    પ્રતિજ્ઞા( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }નહીંતર{
    પ્રતિજ્ઞા( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (અક્ષર*)pBt;
  z -= pHdr->nTitle;
  adjustStats((પૂર્ણાંક)pHdr->iSize, -1);
  randomFill(z, કદ(શૂન્ય*)*pHdr->nBacktraceSlots + કદ(*pHdr) +
                (પૂર્ણાંક)pHdr->iSize + કદ(પૂર્ણાંક) + pHdr->nTitle);
  મુક્ત(z);
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
સ્થિર શૂન્ય *sqlite3MemRealloc(શૂન્ય *pPrior, પૂર્ણાંક nByte){
  સંરચના MemBlockHdr *pOldHdr;
  શૂન્ય *pNew;
  પ્રતિજ્ઞા( mem.disallow==0 );
  પ્રતિજ્ઞા( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  જો( pNew ){
    memcpy(pNew, pPrior, (પૂર્ણાંક)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    જો( nByte>pOldHdr->iSize ){
      randomFill(&((અક્ષર*)pNew)[pOldHdr->iSize], nByte - (પૂર્ણાંક)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  પાછા pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
શૂન્ય sqlite3MemSetDefault(શૂન્ય){
  સ્થિર અચલ sqlite3_mem_methods defaultMethods = {
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
શૂન્ય sqlite3MemdebugSetType(શૂન્ય *p, u8 eType){
  જો( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    સંરચના MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    પ્રતિજ્ઞા( pHdr->iForeGuard==FOREGUARD );
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
પૂર્ણાંક sqlite3MemdebugHasType(અચલ શૂન્ય *p, u8 eType){
  પૂર્ણાંક rc = 1;
  જો( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    સંરચના MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    પ્રતિજ્ઞા( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    જો( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  પાછા rc;
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
પૂર્ણાંક sqlite3MemdebugNoType(અચલ શૂન્ય *p, u8 eType){
  પૂર્ણાંક rc = 1;
  જો( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    સંરચના MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    પ્રતિજ્ઞા( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    જો( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  પાછા rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
શૂન્ય sqlite3MemdebugBacktrace(પૂર્ણાંક depth){
  જો( depth<0 ){ depth = 0; }
  જો( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

શૂન્ય sqlite3MemdebugBacktraceCallback(શૂન્ય (*xBacktrace)(પૂર્ણાંક, પૂર્ણાંક, શૂન્ય **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
શૂન્ય sqlite3MemdebugSettitle(અચલ અક્ષર *zTitle){
  unsigned પૂર્ણાંક n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  જો( n>=કદ(mem.zTitle) ) n = કદ(mem.zTitle)-1;
  memcpy(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

શૂન્ય sqlite3MemdebugSync(){
  સંરચના MemBlockHdr *pHdr;
  માટે(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    શૂન્ય **pBt = (શૂન્ય**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((પૂર્ણાંક)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
શૂન્ય sqlite3MemdebugDump(અચલ અક્ષર *zFilename){
  FILE *out;
  સંરચના MemBlockHdr *pHdr;
  શૂન્ય **pBt;
  પૂર્ણાંક i;
  out = fopen(zFilename, "w");
  જો( out==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    પાછા;
  }
  માટે(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    અક્ષર *z = (અક્ષર*)pHdr;
    z -= pHdr->nBacktraceSlots*કદ(શૂન્ય*) + pHdr->nTitle;
    fprintf(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    જો( pHdr->nBacktrace ){
      fflush(out);
      pBt = (શૂન્ય**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      fprintf(out, "\n");
    }
  }
  fprintf(out, "COUNTS:\n");
  માટે(i=0; i<NCSIZE-1; i++){
    જો( mem.nAlloc[i] ){
      fprintf(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  જો( mem.nAlloc[NCSIZE-1] ){
    fprintf(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
પૂર્ણાંક sqlite3_Memdebug_Malloc_ગણતરી(){
  પૂર્ણાંક i;
  પૂર્ણાંક nTotal = 0;
  માટે(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  પાછા nTotal;
}


#endif /* SQLITE_MEMDEBUG */
