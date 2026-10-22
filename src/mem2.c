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
#अन्तर्भाव "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ifdef SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ifdef __GLIBC__
  बाह्य पूर्णांक backtrace(शून्य**,पूर्णांक);
  बाह्य शून्य backtrace_symbols_fd(शून्य*स्थिरांक*,पूर्णांक,पूर्णांक);
#अन्यथा
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#endif
#अन्तर्भाव <stdio.h>

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
संरचना MemBlockHdr {
  i64 iSize;                          /* Size of this allocation */
  संरचना MemBlockHdr *pNext, *pPrev;  /* Linked list of all unfreed memory */
  अक्षर nBacktrace;                    /* Number of backtraces on this alloc */
  अक्षर nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  पूर्णांक iForeGuard;                     /* Guard word for sanity */
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
स्थिर संरचना {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  संरचना MemBlockHdr *pFirst;
  संरचना MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  पूर्णांक nBacktrace;
  शून्य (*xBacktrace)(पूर्णांक, पूर्णांक, शून्य **);

  /*
  ** Title text to insert in front of each block
  */
  पूर्णांक nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  अक्षर zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  पूर्णांक disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  पूर्णांक nAlloc[NCSIZE];      /* Total number of allocations */
  पूर्णांक nCurrent[NCSIZE];    /* Current number of allocations */
  पूर्णांक mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
स्थिर शून्य adjustStats(पूर्णांक iSize, पूर्णांक increment){
  पूर्णांक i = ROUND8(iSize)/8;
  जर( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  जर( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    जर( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }अन्यथा{
    mem.nCurrent[i]--;
    प्रतिज्ञा( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
स्थिर संरचना MemBlockHdr *sqlite3MemsysGetHeader(स्थिरांक शून्य *pAllocation){
  संरचना MemBlockHdr *p;
  पूर्णांक *pInt;
  u8 *pU8;
  पूर्णांक nReserve;

  p = (संरचना MemBlockHdr*)pAllocation;
  p--;
  प्रतिज्ञा( p->iForeGuard==(पूर्णांक)FOREGUARD );
  nReserve = ROUND8(p->iSize);
  pInt = (पूर्णांक*)pAllocation;
  pU8 = (u8*)pAllocation;
  प्रतिज्ञा( pInt[nReserve/आकार(पूर्णांक)]==(पूर्णांक)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  पर्यंत( nReserve-- > p->iSize ) प्रतिज्ञा( pU8[nReserve]==0x65 );
  परत p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
स्थिर पूर्णांक sqlite3MemSize(शून्य *p){
  संरचना MemBlockHdr *pHdr;
  जर( !p ){
    परत 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  परत (पूर्णांक)pHdr->iSize;
}

/*
** Initialize the memory allocation subsystem.
*/
स्थिर पूर्णांक sqlite3MemInit(शून्य *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  प्रतिज्ञा( (आकार(संरचना MemBlockHdr)&7) == 0 );
  जर( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  परत SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
स्थिर शून्य sqlite3MemShutdown(शून्य *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
स्थिर पूर्णांक sqlite3MemRoundup(पूर्णांक n){
  परत ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
स्थिर शून्य randomFill(अक्षर *pBuf, पूर्णांक nByte){
  अचिन्हित पूर्णांक x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  पर्यंत( nByte >= 4 ){
    x = (x>>1) ^ (-(पूर्णांक)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(पूर्णांक*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  पर्यंत( nByte-- > 0 ){
    x = (x>>1) ^ (-(पूर्णांक)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
स्थिर शून्य *sqlite3MemMalloc(पूर्णांक nByte){
  संरचना MemBlockHdr *pHdr;
  शून्य **pBt;
  अक्षर *z;
  पूर्णांक *pInt;
  शून्य *p = 0;
  पूर्णांक totalSize;
  पूर्णांक nReserve;
  sqlite3_mutex_enter(mem.mutex);
  प्रतिज्ञा( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  totalSize = nReserve + आकार(*pHdr) + आकार(पूर्णांक) +
               mem.nBacktrace*आकार(शून्य*) + mem.nTitle;
  p = दो_स्मृति(totalSize);
  जर( p ){
    z = p;
    pBt = (शून्य**)&z[mem.nTitle];
    pHdr = (संरचना MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->pNext = 0;
    pHdr->pPrev = mem.pLast;
    जर( mem.pLast ){
      mem.pLast->pNext = pHdr;
    }अन्यथा{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    जर( mem.nBacktrace ){
      शून्य *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      स_नकल(pBt, &aAddr[1], pHdr->nBacktrace*आकार(शून्य*));
      प्रतिज्ञा(pBt[0]);
      जर( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }अन्यथा{
      pHdr->nBacktrace = 0;
    }
    जर( mem.nTitle ){
      स_नकल(z, mem.zTitle, mem.nTitle);
    }
    pHdr->iSize = nByte;
    adjustStats(nByte, +1);
    pInt = (पूर्णांक*)&pHdr[1];
    pInt[nReserve/आकार(पूर्णांक)] = REARGUARD;
    randomFill((अक्षर*)pInt, nByte);
    स_रखो(((अक्षर*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (शून्य*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  परत p; 
}

/*
** Free memory.
*/
स्थिर शून्य sqlite3MemFree(शून्य *pPrior){
  संरचना MemBlockHdr *pHdr;
  शून्य **pBt;
  अक्षर *z;
  प्रतिज्ञा( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (शून्य**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  जर( pHdr->pPrev ){
    प्रतिज्ञा( pHdr->pPrev->pNext==pHdr );
    pHdr->pPrev->pNext = pHdr->pNext;
  }अन्यथा{
    प्रतिज्ञा( mem.pFirst==pHdr );
    mem.pFirst = pHdr->pNext;
  }
  जर( pHdr->pNext ){
    प्रतिज्ञा( pHdr->pNext->pPrev==pHdr );
    pHdr->pNext->pPrev = pHdr->pPrev;
  }अन्यथा{
    प्रतिज्ञा( mem.pLast==pHdr );
    mem.pLast = pHdr->pPrev;
  }
  z = (अक्षर*)pBt;
  z -= pHdr->nTitle;
  adjustStats((पूर्णांक)pHdr->iSize, -1);
  randomFill(z, आकार(शून्य*)*pHdr->nBacktraceSlots + आकार(*pHdr) +
                (पूर्णांक)pHdr->iSize + आकार(पूर्णांक) + pHdr->nTitle);
  मुक्त(z);
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
स्थिर शून्य *sqlite3MemRealloc(शून्य *pPrior, पूर्णांक nByte){
  संरचना MemBlockHdr *pOldHdr;
  शून्य *pNew;
  प्रतिज्ञा( mem.disallow==0 );
  प्रतिज्ञा( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  जर( pNew ){
    स_नकल(pNew, pPrior, (पूर्णांक)(nByte<pOldHdr->iSize ? nByte : pOldHdr->iSize));
    जर( nByte>pOldHdr->iSize ){
      randomFill(&((अक्षर*)pNew)[pOldHdr->iSize], nByte - (पूर्णांक)pOldHdr->iSize);
    }
    sqlite3MemFree(pPrior);
  }
  परत pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
शून्य sqlite3MemSetDefault(शून्य){
  स्थिर स्थिरांक sqlite3_mem_methods defaultMethods = {
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
शून्य sqlite3MemdebugSetType(शून्य *p, u8 eType){
  जर( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    संरचना MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    प्रतिज्ञा( pHdr->iForeGuard==FOREGUARD );
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
पूर्णांक sqlite3MemdebugHasType(स्थिरांक शून्य *p, u8 eType){
  पूर्णांक rc = 1;
  जर( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    संरचना MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    प्रतिज्ञा( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    जर( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  परत rc;
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
पूर्णांक sqlite3MemdebugNoType(स्थिरांक शून्य *p, u8 eType){
  पूर्णांक rc = 1;
  जर( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    संरचना MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    प्रतिज्ञा( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    जर( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  परत rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
शून्य sqlite3MemdebugBacktrace(पूर्णांक depth){
  जर( depth<0 ){ depth = 0; }
  जर( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

शून्य sqlite3MemdebugBacktraceCallback(शून्य (*xBacktrace)(पूर्णांक, पूर्णांक, शून्य **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
शून्य sqlite3MemdebugSettitle(स्थिरांक अक्षर *zTitle){
  अचिन्हित पूर्णांक n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  जर( n>=आकार(mem.zTitle) ) n = आकार(mem.zTitle)-1;
  स_नकल(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

शून्य sqlite3MemdebugSync(){
  संरचना MemBlockHdr *pHdr;
  साठी(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    शून्य **pBt = (शून्य**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((पूर्णांक)pHdr->iSize, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
शून्य sqlite3MemdebugDump(स्थिरांक अक्षर *zFilename){
  खाता *out;
  संरचना MemBlockHdr *pHdr;
  शून्य **pBt;
  पूर्णांक i;
  out = ख_खोलो(zFilename, "w");
  जर( out==0 ){
    ख_लिखो(मानक_त्रुटि, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    परत;
  }
  साठी(pHdr=mem.pFirst; pHdr; pHdr=pHdr->pNext){
    अक्षर *z = (अक्षर*)pHdr;
    z -= pHdr->nBacktraceSlots*आकार(शून्य*) + pHdr->nTitle;
    ख_लिखो(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->iSize, &pHdr[1], pHdr->nTitle ? z : "???");
    जर( pHdr->nBacktrace ){
      ख_साफ(out);
      pBt = (शून्य**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      ख_लिखो(out, "\n");
    }
  }
  ख_लिखो(out, "COUNTS:\n");
  साठी(i=0; i<NCSIZE-1; i++){
    जर( mem.nAlloc[i] ){
      ख_लिखो(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  जर( mem.nAlloc[NCSIZE-1] ){
    ख_लिखो(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  ख_बंद(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
पूर्णांक sqlite3_Memdebug_Malloc_मोजणी(){
  पूर्णांक i;
  पूर्णांक nTotal = 0;
  साठी(i=0; i<NCSIZE; i++){
    nTotal += mem.nAlloc[i];
  }
  परत nTotal;
}


#endif /* SQLITE_MEMDEBUG */
