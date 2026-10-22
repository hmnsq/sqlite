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
#समावेश "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#अगर_घोषित SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#अगर_घोषित __GLIBC__
  बाह्य पूर्णांक backtrace(व्योम**,पूर्णांक);
  बाह्य व्योम backtrace_symbols_fd(व्योम*स्थिर*,पूर्णांक,पूर्णांक);
#अन्यथा
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#पूर्ण_अगर
#समावेश <stdio.h>

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
काष्ठा MemBlockHdr {
  i64 i_आकार;                          /* Size of this allocation */
  काष्ठा MemBlockHdr *p_अग्र, *p_पूर्व;  /* Linked list of all unfreed memory */
  अक्षर nBacktrace;                    /* Number of backtraces on this alloc */
  अक्षर nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  पूर्णांक iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#घोषणा FOREGUARD 0x80F5E153
#घोषणा REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#घोषणा NCSIZE  1000

/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
अटल काष्ठा {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  काष्ठा MemBlockHdr *pFirst;
  काष्ठा MemBlockHdr *pLast;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  पूर्णांक nBacktrace;
  व्योम (*xBacktrace)(पूर्णांक, पूर्णांक, व्योम **);

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
अटल व्योम adjustStats(पूर्णांक i_आकार, पूर्णांक increment){
  पूर्णांक i = ROUND8(i_आकार)/8;
  अगर( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  अगर( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    अगर( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }अन्यथा{
    mem.nCurrent[i]--;
    निश्चित( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
अटल काष्ठा MemBlockHdr *sqlite3MemsysGetHeader(स्थिर व्योम *pAllocation){
  काष्ठा MemBlockHdr *p;
  पूर्णांक *pInt;
  u8 *pU8;
  पूर्णांक nReserve;

  p = (काष्ठा MemBlockHdr*)pAllocation;
  p--;
  निश्चित( p->iForeGuard==(पूर्णांक)FOREGUARD );
  nReserve = ROUND8(p->i_आकार);
  pInt = (पूर्णांक*)pAllocation;
  pU8 = (u8*)pAllocation;
  निश्चित( pInt[nReserve/माप(पूर्णांक)]==(पूर्णांक)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  जबतक( nReserve-- > p->i_आकार ) निश्चित( pU8[nReserve]==0x65 );
  वापस p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
अटल पूर्णांक sqlite3_Mem_आकार(व्योम *p){
  काष्ठा MemBlockHdr *pHdr;
  अगर( !p ){
    वापस 0;
  }
  pHdr = sqlite3MemsysGetHeader(p);
  वापस (पूर्णांक)pHdr->i_आकार;
}

/*
** Initialize the memory allocation subsystem.
*/
अटल पूर्णांक sqlite3MemInit(व्योम *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  निश्चित( (माप(काष्ठा MemBlockHdr)&7) == 0 );
  अगर( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  वापस SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
अटल व्योम sqlite3MemShutdown(व्योम *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
अटल पूर्णांक sqlite3MemRoundup(पूर्णांक n){
  वापस ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
अटल व्योम randomFill(अक्षर *pBuf, पूर्णांक nByte){
  अचिन्हित पूर्णांक x, y, r;
  x = SQLITE_PTR_TO_INT(pBuf);
  y = nByte | 1;
  जबतक( nByte >= 4 ){
    x = (x>>1) ^ (-(पूर्णांक)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(पूर्णांक*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  जबतक( nByte-- > 0 ){
    x = (x>>1) ^ (-(पूर्णांक)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
अटल व्योम *sqlite3MemMalloc(पूर्णांक nByte){
  काष्ठा MemBlockHdr *pHdr;
  व्योम **pBt;
  अक्षर *z;
  पूर्णांक *pInt;
  व्योम *p = 0;
  पूर्णांक योग_आकार;
  पूर्णांक nReserve;
  sqlite3_mutex_enter(mem.mutex);
  निश्चित( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  योग_आकार = nReserve + माप(*pHdr) + माप(पूर्णांक) +
               mem.nBacktrace*माप(व्योम*) + mem.nTitle;
  p = दो_स्मृति(योग_आकार);
  अगर( p ){
    z = p;
    pBt = (व्योम**)&z[mem.nTitle];
    pHdr = (काष्ठा MemBlockHdr*)&pBt[mem.nBacktrace];
    pHdr->p_अग्र = 0;
    pHdr->p_पूर्व = mem.pLast;
    अगर( mem.pLast ){
      mem.pLast->p_अग्र = pHdr;
    }अन्यथा{
      mem.pFirst = pHdr;
    }
    mem.pLast = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    अगर( mem.nBacktrace ){
      व्योम *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      स_नकल(pBt, &aAddr[1], pHdr->nBacktrace*माप(व्योम*));
      निश्चित(pBt[0]);
      अगर( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }अन्यथा{
      pHdr->nBacktrace = 0;
    }
    अगर( mem.nTitle ){
      स_नकल(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_आकार = nByte;
    adjustStats(nByte, +1);
    pInt = (पूर्णांक*)&pHdr[1];
    pInt[nReserve/माप(पूर्णांक)] = REARGUARD;
    randomFill((अक्षर*)pInt, nByte);
    स_रखो(((अक्षर*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (व्योम*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  वापस p; 
}

/*
** Free memory.
*/
अटल व्योम sqlite3MemFree(व्योम *pPrior){
  काष्ठा MemBlockHdr *pHdr;
  व्योम **pBt;
  अक्षर *z;
  निश्चित( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3MemsysGetHeader(pPrior);
  pBt = (व्योम**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  अगर( pHdr->p_पूर्व ){
    निश्चित( pHdr->p_पूर्व->p_अग्र==pHdr );
    pHdr->p_पूर्व->p_अग्र = pHdr->p_अग्र;
  }अन्यथा{
    निश्चित( mem.pFirst==pHdr );
    mem.pFirst = pHdr->p_अग्र;
  }
  अगर( pHdr->p_अग्र ){
    निश्चित( pHdr->p_अग्र->p_पूर्व==pHdr );
    pHdr->p_अग्र->p_पूर्व = pHdr->p_पूर्व;
  }अन्यथा{
    निश्चित( mem.pLast==pHdr );
    mem.pLast = pHdr->p_पूर्व;
  }
  z = (अक्षर*)pBt;
  z -= pHdr->nTitle;
  adjustStats((पूर्णांक)pHdr->i_आकार, -1);
  randomFill(z, माप(व्योम*)*pHdr->nBacktraceSlots + माप(*pHdr) +
                (पूर्णांक)pHdr->i_आकार + माप(पूर्णांक) + pHdr->nTitle);
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
अटल व्योम *sqlite3MemRealloc(व्योम *pPrior, पूर्णांक nByte){
  काष्ठा MemBlockHdr *pOldHdr;
  व्योम *pNew;
  निश्चित( mem.disallow==0 );
  निश्चित( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3MemsysGetHeader(pPrior);
  pNew = sqlite3MemMalloc(nByte);
  अगर( pNew ){
    स_नकल(pNew, pPrior, (पूर्णांक)(nByte<pOldHdr->i_आकार ? nByte : pOldHdr->i_आकार));
    अगर( nByte>pOldHdr->i_आकार ){
      randomFill(&((अक्षर*)pNew)[pOldHdr->i_आकार], nByte - (पूर्णांक)pOldHdr->i_आकार);
    }
    sqlite3MemFree(pPrior);
  }
  वापस pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
व्योम sqlite3MemSetDefault(व्योम){
  अटल स्थिर sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_आकार,
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
व्योम sqlite3MemdebugSetType(व्योम *p, u8 eType){
  अगर( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    काष्ठा MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    निश्चित( pHdr->iForeGuard==FOREGUARD );
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
पूर्णांक sqlite3MemdebugHasType(स्थिर व्योम *p, u8 eType){
  पूर्णांक rc = 1;
  अगर( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    काष्ठा MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    निश्चित( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    अगर( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  वापस rc;
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
पूर्णांक sqlite3MemdebugNoType(स्थिर व्योम *p, u8 eType){
  पूर्णांक rc = 1;
  अगर( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    काष्ठा MemBlockHdr *pHdr;
    pHdr = sqlite3MemsysGetHeader(p);
    निश्चित( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    अगर( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  वापस rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
व्योम sqlite3MemdebugBacktrace(पूर्णांक depth){
  अगर( depth<0 ){ depth = 0; }
  अगर( depth>20 ){ depth = 20; }
  depth = (depth+1)&0xfe;
  mem.nBacktrace = depth;
}

व्योम sqlite3MemdebugBacktraceCallback(व्योम (*xBacktrace)(पूर्णांक, पूर्णांक, व्योम **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
व्योम sqlite3MemdebugSettitle(स्थिर अक्षर *zTitle){
  अचिन्हित पूर्णांक n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  अगर( n>=माप(mem.zTitle) ) n = माप(mem.zTitle)-1;
  स_नकल(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

व्योम sqlite3MemdebugSync(){
  काष्ठा MemBlockHdr *pHdr;
  क्रम(pHdr=mem.pFirst; pHdr; pHdr=pHdr->p_अग्र){
    व्योम **pBt = (व्योम**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((पूर्णांक)pHdr->i_आकार, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
व्योम sqlite3MemdebugDump(स्थिर अक्षर *zFilename){
  खाता *out;
  काष्ठा MemBlockHdr *pHdr;
  व्योम **pBt;
  पूर्णांक i;
  out = ख_खोलो(zFilename, "w");
  अगर( out==0 ){
    ख_लिखो(मानक_त्रुटि, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    वापस;
  }
  क्रम(pHdr=mem.pFirst; pHdr; pHdr=pHdr->p_अग्र){
    अक्षर *z = (अक्षर*)pHdr;
    z -= pHdr->nBacktraceSlots*माप(व्योम*) + pHdr->nTitle;
    ख_लिखो(out, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_आकार, &pHdr[1], pHdr->nTitle ? z : "???");
    अगर( pHdr->nBacktrace ){
      ख_साफ(out);
      pBt = (व्योम**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(out));
      ख_लिखो(out, "\n");
    }
  }
  ख_लिखो(out, "COUNTS:\n");
  क्रम(i=0; i<NCSIZE-1; i++){
    अगर( mem.nAlloc[i] ){
      ख_लिखो(out, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  अगर( mem.nAlloc[NCSIZE-1] ){
    ख_लिखो(out, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  ख_बंद(out);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
पूर्णांक sqlite3_Memdebug_Malloc_गिनती(){
  पूर्णांक i;
  पूर्णांक n_योग = 0;
  क्रम(i=0; i<NCSIZE; i++){
    n_योग += mem.nAlloc[i];
  }
  वापस n_योग;
}


#पूर्ण_अगर /* SQLITE_MEMDEBUG */
