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
#అన్తర్భావ "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#నిర్వచితమైతే SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#నిర్వచితమైతే __GLIBC__
  బాహ్య పూర్ణాంకం backtrace(శూన్య**,పూర్ణాంకం);
  బాహ్య శూన్య backtrace_symbols_fd(శూన్య*స్థిరరాశి*,పూర్ణాంకం,పూర్ణాంకం);
#లేకపోతే
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#అయితే_ముగింపు
#అన్తర్భావ <stdio.h>

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
నిర్మాణం Mem_ఖండం_Hdr {
  i64 i_కొలత;                          /* Size of this allocation */
  నిర్మాణం Mem_ఖండం_Hdr *p_తదుపరి, *pPrev;  /* Linked list of all unfreed memory */
  అక్షరం nBacktrace;                    /* Number of backtraces on this alloc */
  అక్షరం nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  పూర్ణాంకం iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#నిర్వచించు FOREGUARD 0x80F5E153
#నిర్వచించు REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#నిర్వచించు NCSIZE  1000

/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
స్థిర నిర్మాణం {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  నిర్మాణం Mem_ఖండం_Hdr *p_మొదటి;
  నిర్మాణం Mem_ఖండం_Hdr *p_చివరి;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  పూర్ణాంకం nBacktrace;
  శూన్య (*xBacktrace)(పూర్ణాంకం, పూర్ణాంకం, శూన్య **);

  /*
  ** Title text to insert in front of each block
  */
  పూర్ణాంకం nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  అక్షరం zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  పూర్ణాంకం disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  పూర్ణాంకం nAlloc[NCSIZE];      /* Total number of allocations */
  పూర్ణాంకం nCurrent[NCSIZE];    /* Current number of allocations */
  పూర్ణాంకం mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
స్థిర శూన్య adjustStats(పూర్ణాంకం i_కొలత, పూర్ణాంకం increment){
  పూర్ణాంకం i = ROUND8(i_కొలత)/8;
  ఒకవేళ( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  ఒకవేళ( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    ఒకవేళ( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }లేకపోతే{
    mem.nCurrent[i]--;
    ప్రతిజ్ఞా( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
స్థిర నిర్మాణం Mem_ఖండం_Hdr *sqlite3_Memsys_Get_శీర్షిక(స్థిరరాశి శూన్య *p_కేటాయింపు){
  నిర్మాణం Mem_ఖండం_Hdr *p;
  పూర్ణాంకం *pInt;
  u8 *pU8;
  పూర్ణాంకం nReserve;

  p = (నిర్మాణం Mem_ఖండం_Hdr*)p_కేటాయింపు;
  p--;
  ప్రతిజ్ఞా( p->iForeGuard==(పూర్ణాంకం)FOREGUARD );
  nReserve = ROUND8(p->i_కొలత);
  pInt = (పూర్ణాంకం*)p_కేటాయింపు;
  pU8 = (u8*)p_కేటాయింపు;
  ప్రతిజ్ఞా( pInt[nReserve/పరిమాణం(పూర్ణాంకం)]==(పూర్ణాంకం)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  అయితే( nReserve-- > p->i_కొలత ) ప్రతిజ్ఞా( pU8[nReserve]==0x65 );
  తిరిగి p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
స్థిర పూర్ణాంకం sqlite3_Mem_కొలత(శూన్య *p){
  నిర్మాణం Mem_ఖండం_Hdr *pHdr;
  ఒకవేళ( !p ){
    తిరిగి 0;
  }
  pHdr = sqlite3_Memsys_Get_శీర్షిక(p);
  తిరిగి (పూర్ణాంకం)pHdr->i_కొలత;
}

/*
** Initialize the memory allocation subsystem.
*/
స్థిర పూర్ణాంకం sqlite3MemInit(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  ప్రతిజ్ఞా( (పరిమాణం(నిర్మాణం Mem_ఖండం_Hdr)&7) == 0 );
  ఒకవేళ( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  తిరిగి SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
స్థిర శూన్య sqlite3MemShutdown(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
స్థిర పూర్ణాంకం sqlite3MemRoundup(పూర్ణాంకం n){
  తిరిగి ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
స్థిర శూన్య randomFill(అక్షరం *pBuf, పూర్ణాంకం nByte){
  గుర్తులేని పూర్ణాంకం x, y, r;
  x = SQLITE_PTR_గమ్యం_INT(pBuf);
  y = nByte | 1;
  అయితే( nByte >= 4 ){
    x = (x>>1) ^ (-(పూర్ణాంకం)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(పూర్ణాంకం*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  అయితే( nByte-- > 0 ){
    x = (x>>1) ^ (-(పూర్ణాంకం)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
స్థిర శూన్య *sqlite3MemMalloc(పూర్ణాంకం nByte){
  నిర్మాణం Mem_ఖండం_Hdr *pHdr;
  శూన్య **pBt;
  అక్షరం *z;
  పూర్ణాంకం *pInt;
  శూన్య *p = 0;
  పూర్ణాంకం మొత్తం_కొలత;
  పూర్ణాంకం nReserve;
  sqlite3_mutex_enter(mem.mutex);
  ప్రతిజ్ఞా( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  మొత్తం_కొలత = nReserve + పరిమాణం(*pHdr) + పరిమాణం(పూర్ణాంకం) +
               mem.nBacktrace*పరిమాణం(శూన్య*) + mem.nTitle;
  p = జ్ఞాపకం_ఇవ్వు(మొత్తం_కొలత);
  ఒకవేళ( p ){
    z = p;
    pBt = (శూన్య**)&z[mem.nTitle];
    pHdr = (నిర్మాణం Mem_ఖండం_Hdr*)&pBt[mem.nBacktrace];
    pHdr->p_తదుపరి = 0;
    pHdr->pPrev = mem.p_చివరి;
    ఒకవేళ( mem.p_చివరి ){
      mem.p_చివరి->p_తదుపరి = pHdr;
    }లేకపోతే{
      mem.p_మొదటి = pHdr;
    }
    mem.p_చివరి = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_కుప్ప;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    ఒకవేళ( mem.nBacktrace ){
      శూన్య *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      జ్ఞాపకం_నకలు(pBt, &aAddr[1], pHdr->nBacktrace*పరిమాణం(శూన్య*));
      ప్రతిజ్ఞా(pBt[0]);
      ఒకవేళ( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }లేకపోతే{
      pHdr->nBacktrace = 0;
    }
    ఒకవేళ( mem.nTitle ){
      జ్ఞాపకం_నకలు(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_కొలత = nByte;
    adjustStats(nByte, +1);
    pInt = (పూర్ణాంకం*)&pHdr[1];
    pInt[nReserve/పరిమాణం(పూర్ణాంకం)] = REARGUARD;
    randomFill((అక్షరం*)pInt, nByte);
    memset(((అక్షరం*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (శూన్య*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  తిరిగి p; 
}

/*
** Free memory.
*/
స్థిర శూన్య sqlite3MemFree(శూన్య *p_పూర్వ){
  నిర్మాణం Mem_ఖండం_Hdr *pHdr;
  శూన్య **pBt;
  అక్షరం *z;
  ప్రతిజ్ఞా( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3_Memsys_Get_శీర్షిక(p_పూర్వ);
  pBt = (శూన్య**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  ఒకవేళ( pHdr->pPrev ){
    ప్రతిజ్ఞా( pHdr->pPrev->p_తదుపరి==pHdr );
    pHdr->pPrev->p_తదుపరి = pHdr->p_తదుపరి;
  }లేకపోతే{
    ప్రతిజ్ఞా( mem.p_మొదటి==pHdr );
    mem.p_మొదటి = pHdr->p_తదుపరి;
  }
  ఒకవేళ( pHdr->p_తదుపరి ){
    ప్రతిజ్ఞా( pHdr->p_తదుపరి->pPrev==pHdr );
    pHdr->p_తదుపరి->pPrev = pHdr->pPrev;
  }లేకపోతే{
    ప్రతిజ్ఞా( mem.p_చివరి==pHdr );
    mem.p_చివరి = pHdr->pPrev;
  }
  z = (అక్షరం*)pBt;
  z -= pHdr->nTitle;
  adjustStats((పూర్ణాంకం)pHdr->i_కొలత, -1);
  randomFill(z, పరిమాణం(శూన్య*)*pHdr->nBacktraceSlots + పరిమాణం(*pHdr) +
                (పూర్ణాంకం)pHdr->i_కొలత + పరిమాణం(పూర్ణాంకం) + pHdr->nTitle);
  విడుదల(z);
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
స్థిర శూన్య *sqlite3MemRealloc(శూన్య *p_పూర్వ, పూర్ణాంకం nByte){
  నిర్మాణం Mem_ఖండం_Hdr *pOldHdr;
  శూన్య *pNew;
  ప్రతిజ్ఞా( mem.disallow==0 );
  ప్రతిజ్ఞా( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3_Memsys_Get_శీర్షిక(p_పూర్వ);
  pNew = sqlite3MemMalloc(nByte);
  ఒకవేళ( pNew ){
    జ్ఞాపకం_నకలు(pNew, p_పూర్వ, (పూర్ణాంకం)(nByte<pOldHdr->i_కొలత ? nByte : pOldHdr->i_కొలత));
    ఒకవేళ( nByte>pOldHdr->i_కొలత ){
      randomFill(&((అక్షరం*)pNew)[pOldHdr->i_కొలత], nByte - (పూర్ణాంకం)pOldHdr->i_కొలత);
    }
    sqlite3MemFree(p_పూర్వ);
  }
  తిరిగి pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
శూన్య sqlite3_Mem_సమితి_Default(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_కొలత,
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
శూన్య sqlite3_Memdebug_సమితి_Type(శూన్య *p, u8 eType){
  ఒకవేళ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    నిర్మాణం Mem_ఖండం_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_శీర్షిక(p);
    ప్రతిజ్ఞా( pHdr->iForeGuard==FOREGUARD );
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
పూర్ణాంకం sqlite3MemdebugHasType(స్థిరరాశి శూన్య *p, u8 eType){
  పూర్ణాంకం rc = 1;
  ఒకవేళ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    నిర్మాణం Mem_ఖండం_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_శీర్షిక(p);
    ప్రతిజ్ఞా( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ఒకవేళ( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  తిరిగి rc;
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
పూర్ణాంకం sqlite3MemdebugNoType(స్థిరరాశి శూన్య *p, u8 eType){
  పూర్ణాంకం rc = 1;
  ఒకవేళ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    నిర్మాణం Mem_ఖండం_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_శీర్షిక(p);
    ప్రతిజ్ఞా( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ఒకవేళ( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  తిరిగి rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
శూన్య sqlite3MemdebugBacktrace(పూర్ణాంకం లోతు){
  ఒకవేళ( లోతు<0 ){ లోతు = 0; }
  ఒకవేళ( లోతు>20 ){ లోతు = 20; }
  లోతు = (లోతు+1)&0xfe;
  mem.nBacktrace = లోతు;
}

శూన్య sqlite3MemdebugBacktraceCallback(శూన్య (*xBacktrace)(పూర్ణాంకం, పూర్ణాంకం, శూన్య **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
శూన్య sqlite3MemdebugSettitle(స్థిరరాశి అక్షరం *zTitle){
  గుర్తులేని పూర్ణాంకం n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  ఒకవేళ( n>=పరిమాణం(mem.zTitle) ) n = పరిమాణం(mem.zTitle)-1;
  జ్ఞాపకం_నకలు(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

శూన్య sqlite3MemdebugSync(){
  నిర్మాణం Mem_ఖండం_Hdr *pHdr;
  కొరకు(pHdr=mem.p_మొదటి; pHdr; pHdr=pHdr->p_తదుపరి){
    శూన్య **pBt = (శూన్య**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((పూర్ణాంకం)pHdr->i_కొలత, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
శూన్య sqlite3MemdebugDump(స్థిరరాశి అక్షరం *zFilename){
  FILE *బహిర్గతం;
  నిర్మాణం Mem_ఖండం_Hdr *pHdr;
  శూన్య **pBt;
  పూర్ణాంకం i;
  బహిర్గతం = fopen(zFilename, "w");
  ఒకవేళ( బహిర్గతం==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    తిరిగి;
  }
  కొరకు(pHdr=mem.p_మొదటి; pHdr; pHdr=pHdr->p_తదుపరి){
    అక్షరం *z = (అక్షరం*)pHdr;
    z -= pHdr->nBacktraceSlots*పరిమాణం(శూన్య*) + pHdr->nTitle;
    fprintf(బహిర్గతం, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_కొలత, &pHdr[1], pHdr->nTitle ? z : "???");
    ఒకవేళ( pHdr->nBacktrace ){
      fflush(బహిర్గతం);
      pBt = (శూన్య**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(బహిర్గతం));
      fprintf(బహిర్గతం, "\n");
    }
  }
  fprintf(బహిర్గతం, "COUNTS:\n");
  కొరకు(i=0; i<NCSIZE-1; i++){
    ఒకవేళ( mem.nAlloc[i] ){
      fprintf(బహిర్గతం, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  ఒకవేళ( mem.nAlloc[NCSIZE-1] ){
    fprintf(బహిర్గతం, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(బహిర్గతం);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
పూర్ణాంకం sqlite3_Memdebug_Malloc_లెక్క(){
  పూర్ణాంకం i;
  పూర్ణాంకం n_మొత్తం = 0;
  కొరకు(i=0; i<NCSIZE; i++){
    n_మొత్తం += mem.nAlloc[i];
  }
  తిరిగి n_మొత్తం;
}


#అయితే_ముగింపు /* SQLITE_MEMDEBUG */
