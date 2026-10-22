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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __GLIBC__
  ਬਾਹਰੀ ਪੂਰਨ backtrace(ਖਾਲੀ**,ਪੂਰਨ);
  ਬਾਹਰੀ ਖਾਲੀ backtrace_symbols_fd(ਖਾਲੀ*ਅਚਲ*,ਪੂਰਨ,ਪੂਰਨ);
#ਨਹੀਂਤਾਂ
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

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
ਬਣਤਰ Mem_ਖੰਡ_Hdr {
  i64 i_ਮਾਪ;                          /* Size of this allocation */
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *p_ਅਗਲਾ, *pPrev;  /* Linked list of all unfreed memory */
  ਅੱਖਰ nBacktrace;                    /* Number of backtraces on this alloc */
  ਅੱਖਰ nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  ਪੂਰਨ iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOREGUARD 0x80F5E153
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NCSIZE  1000

/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ਸਥਿਰ ਬਣਤਰ {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *p_ਪਹਿਲਾ;
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *p_ਆਖਰੀ;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  ਪੂਰਨ nBacktrace;
  ਖਾਲੀ (*xBacktrace)(ਪੂਰਨ, ਪੂਰਨ, ਖਾਲੀ **);

  /*
  ** Title text to insert in front of each block
  */
  ਪੂਰਨ nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  ਅੱਖਰ zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  ਪੂਰਨ disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  ਪੂਰਨ nAlloc[NCSIZE];      /* Total number of allocations */
  ਪੂਰਨ nCurrent[NCSIZE];    /* Current number of allocations */
  ਪੂਰਨ mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ਸਥਿਰ ਖਾਲੀ adjustStats(ਪੂਰਨ i_ਮਾਪ, ਪੂਰਨ increment){
  ਪੂਰਨ i = ROUND8(i_ਮਾਪ)/8;
  ਜੇ( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  ਜੇ( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    ਜੇ( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }ਨਹੀਂਤਾਂ{
    mem.nCurrent[i]--;
    ਪ੍ਰਤਿਜ੍ਞਾ( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
ਸਥਿਰ ਬਣਤਰ Mem_ਖੰਡ_Hdr *sqlite3_Memsys_Get_ਸਿਰਲੇਖ(ਅਚਲ ਖਾਲੀ *p_ਵੰਡ){
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *p;
  ਪੂਰਨ *pInt;
  u8 *pU8;
  ਪੂਰਨ nReserve;

  p = (ਬਣਤਰ Mem_ਖੰਡ_Hdr*)p_ਵੰਡ;
  p--;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iForeGuard==(ਪੂਰਨ)FOREGUARD );
  nReserve = ROUND8(p->i_ਮਾਪ);
  pInt = (ਪੂਰਨ*)p_ਵੰਡ;
  pU8 = (u8*)p_ਵੰਡ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pInt[nReserve/ਆਕਾਰ(ਪੂਰਨ)]==(ਪੂਰਨ)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  ਜਦੋਂ( nReserve-- > p->i_ਮਾਪ ) ਪ੍ਰਤਿਜ੍ਞਾ( pU8[nReserve]==0x65 );
  ਵਾਪਸ p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_Mem_ਮਾਪ(ਖਾਲੀ *p){
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
  ਜੇ( !p ){
    ਵਾਪਸ 0;
  }
  pHdr = sqlite3_Memsys_Get_ਸਿਰਲੇਖ(p);
  ਵਾਪਸ (ਪੂਰਨ)pHdr->i_ਮਾਪ;
}

/*
** Initialize the memory allocation subsystem.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3MemInit(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਪ੍ਰਤਿਜ੍ਞਾ( (ਆਕਾਰ(ਬਣਤਰ Mem_ਖੰਡ_Hdr)&7) == 0 );
  ਜੇ( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3MemShutdown(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3MemRoundup(ਪੂਰਨ n){
  ਵਾਪਸ ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ਸਥਿਰ ਖਾਲੀ randomFill(ਅੱਖਰ *pBuf, ਪੂਰਨ nByte){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x, y, r;
  x = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(pBuf);
  y = nByte | 1;
  ਜਦੋਂ( nByte >= 4 ){
    x = (x>>1) ^ (-(ਪੂਰਨ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(ਪੂਰਨ*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  ਜਦੋਂ( nByte-- > 0 ){
    x = (x>>1) ^ (-(ਪੂਰਨ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ਸਥਿਰ ਖਾਲੀ *sqlite3MemMalloc(ਪੂਰਨ nByte){
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
  ਖਾਲੀ **pBt;
  ਅੱਖਰ *z;
  ਪੂਰਨ *pInt;
  ਖਾਲੀ *p = 0;
  ਪੂਰਨ ਕੁੱਲ_ਮਾਪ;
  ਪੂਰਨ nReserve;
  sqlite3_mutex_enter(mem.mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  ਕੁੱਲ_ਮਾਪ = nReserve + ਆਕਾਰ(*pHdr) + ਆਕਾਰ(ਪੂਰਨ) +
               mem.nBacktrace*ਆਕਾਰ(ਖਾਲੀ*) + mem.nTitle;
  p = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਕੁੱਲ_ਮਾਪ);
  ਜੇ( p ){
    z = p;
    pBt = (ਖਾਲੀ**)&z[mem.nTitle];
    pHdr = (ਬਣਤਰ Mem_ਖੰਡ_Hdr*)&pBt[mem.nBacktrace];
    pHdr->p_ਅਗਲਾ = 0;
    pHdr->pPrev = mem.p_ਆਖਰੀ;
    ਜੇ( mem.p_ਆਖਰੀ ){
      mem.p_ਆਖਰੀ->p_ਅਗਲਾ = pHdr;
    }ਨਹੀਂਤਾਂ{
      mem.p_ਪਹਿਲਾ = pHdr;
    }
    mem.p_ਆਖਰੀ = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_ਢੇਰ;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    ਜੇ( mem.nBacktrace ){
      ਖਾਲੀ *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pBt, &aAddr[1], pHdr->nBacktrace*ਆਕਾਰ(ਖਾਲੀ*));
      ਪ੍ਰਤਿਜ੍ਞਾ(pBt[0]);
      ਜੇ( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }ਨਹੀਂਤਾਂ{
      pHdr->nBacktrace = 0;
    }
    ਜੇ( mem.nTitle ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_ਮਾਪ = nByte;
    adjustStats(nByte, +1);
    pInt = (ਪੂਰਨ*)&pHdr[1];
    pInt[nReserve/ਆਕਾਰ(ਪੂਰਨ)] = REARGUARD;
    randomFill((ਅੱਖਰ*)pInt, nByte);
    memset(((ਅੱਖਰ*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (ਖਾਲੀ*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  ਵਾਪਸ p; 
}

/*
** Free memory.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3MemFree(ਖਾਲੀ *p_ਪੂਰਵ){
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
  ਖਾਲੀ **pBt;
  ਅੱਖਰ *z;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3_Memsys_Get_ਸਿਰਲੇਖ(p_ਪੂਰਵ);
  pBt = (ਖਾਲੀ**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  ਜੇ( pHdr->pPrev ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pHdr->pPrev->p_ਅਗਲਾ==pHdr );
    pHdr->pPrev->p_ਅਗਲਾ = pHdr->p_ਅਗਲਾ;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( mem.p_ਪਹਿਲਾ==pHdr );
    mem.p_ਪਹਿਲਾ = pHdr->p_ਅਗਲਾ;
  }
  ਜੇ( pHdr->p_ਅਗਲਾ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pHdr->p_ਅਗਲਾ->pPrev==pHdr );
    pHdr->p_ਅਗਲਾ->pPrev = pHdr->pPrev;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( mem.p_ਆਖਰੀ==pHdr );
    mem.p_ਆਖਰੀ = pHdr->pPrev;
  }
  z = (ਅੱਖਰ*)pBt;
  z -= pHdr->nTitle;
  adjustStats((ਪੂਰਨ)pHdr->i_ਮਾਪ, -1);
  randomFill(z, ਆਕਾਰ(ਖਾਲੀ*)*pHdr->nBacktraceSlots + ਆਕਾਰ(*pHdr) +
                (ਪੂਰਨ)pHdr->i_ਮਾਪ + ਆਕਾਰ(ਪੂਰਨ) + pHdr->nTitle);
  ਮੁਕਤ(z);
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
ਸਥਿਰ ਖਾਲੀ *sqlite3MemRealloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ nByte){
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *pOldHdr;
  ਖਾਲੀ *pNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( mem.disallow==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3_Memsys_Get_ਸਿਰਲੇਖ(p_ਪੂਰਵ);
  pNew = sqlite3MemMalloc(nByte);
  ਜੇ( pNew ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew, p_ਪੂਰਵ, (ਪੂਰਨ)(nByte<pOldHdr->i_ਮਾਪ ? nByte : pOldHdr->i_ਮਾਪ));
    ਜੇ( nByte>pOldHdr->i_ਮਾਪ ){
      randomFill(&((ਅੱਖਰ*)pNew)[pOldHdr->i_ਮਾਪ], nByte - (ਪੂਰਨ)pOldHdr->i_ਮਾਪ);
    }
    sqlite3MemFree(p_ਪੂਰਵ);
  }
  ਵਾਪਸ pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ਖਾਲੀ sqlite3_Mem_ਸੈੱਟ_Default(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ਮਾਪ,
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
ਖਾਲੀ sqlite3_Memdebug_ਸੈੱਟ_Type(ਖਾਲੀ *p, u8 eType){
  ਜੇ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_ਸਿਰਲੇਖ(p);
    ਪ੍ਰਤਿਜ੍ਞਾ( pHdr->iForeGuard==FOREGUARD );
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
ਪੂਰਨ sqlite3MemdebugHasType(ਅਚਲ ਖਾਲੀ *p, u8 eType){
  ਪੂਰਨ rc = 1;
  ਜੇ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_ਸਿਰਲੇਖ(p);
    ਪ੍ਰਤਿਜ੍ਞਾ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ਜੇ( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3MemdebugNoType(ਅਚਲ ਖਾਲੀ *p, u8 eType){
  ਪੂਰਨ rc = 1;
  ਜੇ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_ਸਿਰਲੇਖ(p);
    ਪ੍ਰਤਿਜ੍ਞਾ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ਜੇ( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  ਵਾਪਸ rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
ਖਾਲੀ sqlite3MemdebugBacktrace(ਪੂਰਨ ਡੂੰਘਾਈ){
  ਜੇ( ਡੂੰਘਾਈ<0 ){ ਡੂੰਘਾਈ = 0; }
  ਜੇ( ਡੂੰਘਾਈ>20 ){ ਡੂੰਘਾਈ = 20; }
  ਡੂੰਘਾਈ = (ਡੂੰਘਾਈ+1)&0xfe;
  mem.nBacktrace = ਡੂੰਘਾਈ;
}

ਖਾਲੀ sqlite3MemdebugBacktraceCallback(ਖਾਲੀ (*xBacktrace)(ਪੂਰਨ, ਪੂਰਨ, ਖਾਲੀ **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
ਖਾਲੀ sqlite3MemdebugSettitle(ਅਚਲ ਅੱਖਰ *zTitle){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  ਜੇ( n>=ਆਕਾਰ(mem.zTitle) ) n = ਆਕਾਰ(mem.zTitle)-1;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

ਖਾਲੀ sqlite3MemdebugSync(){
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
  ਲਈ(pHdr=mem.p_ਪਹਿਲਾ; pHdr; pHdr=pHdr->p_ਅਗਲਾ){
    ਖਾਲੀ **pBt = (ਖਾਲੀ**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((ਪੂਰਨ)pHdr->i_ਮਾਪ, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ਖਾਲੀ sqlite3MemdebugDump(ਅਚਲ ਅੱਖਰ *zFilename){
  FILE *ਬਾਹਰਲਾ;
  ਬਣਤਰ Mem_ਖੰਡ_Hdr *pHdr;
  ਖਾਲੀ **pBt;
  ਪੂਰਨ i;
  ਬਾਹਰਲਾ = fopen(zFilename, "w");
  ਜੇ( ਬਾਹਰਲਾ==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    ਵਾਪਸ;
  }
  ਲਈ(pHdr=mem.p_ਪਹਿਲਾ; pHdr; pHdr=pHdr->p_ਅਗਲਾ){
    ਅੱਖਰ *z = (ਅੱਖਰ*)pHdr;
    z -= pHdr->nBacktraceSlots*ਆਕਾਰ(ਖਾਲੀ*) + pHdr->nTitle;
    fprintf(ਬਾਹਰਲਾ, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_ਮਾਪ, &pHdr[1], pHdr->nTitle ? z : "???");
    ਜੇ( pHdr->nBacktrace ){
      fflush(ਬਾਹਰਲਾ);
      pBt = (ਖਾਲੀ**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(ਬਾਹਰਲਾ));
      fprintf(ਬਾਹਰਲਾ, "\n");
    }
  }
  fprintf(ਬਾਹਰਲਾ, "COUNTS:\n");
  ਲਈ(i=0; i<NCSIZE-1; i++){
    ਜੇ( mem.nAlloc[i] ){
      fprintf(ਬਾਹਰਲਾ, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  ਜੇ( mem.nAlloc[NCSIZE-1] ){
    fprintf(ਬਾਹਰਲਾ, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(ਬਾਹਰਲਾ);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
ਪੂਰਨ sqlite3_Memdebug_Malloc_ਗਿਣਤੀ(){
  ਪੂਰਨ i;
  ਪੂਰਨ n_ਕੁੱਲ = 0;
  ਲਈ(i=0; i<NCSIZE; i++){
    n_ਕੁੱਲ += mem.nAlloc[i];
  }
  ਵਾਪਸ n_ਕੁੱਲ;
}


#ਜੇ_ਅੰਤ /* SQLITE_MEMDEBUG */
