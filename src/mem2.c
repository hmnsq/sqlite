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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __GLIBC__
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ backtrace(ಶೂನ್ಯ**,ಪೂರ್ಣಾಂಕ);
  ಬಾಹ್ಯ ಶೂನ್ಯ backtrace_symbols_fd(ಶೂನ್ಯ*ಸ್ಥಿರಾಂಕ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
#ಇಲ್ಲದಿದ್ದರೆ
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#ಆದರೆ_ಅಂತ್ಯ
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
ರಚನೆ Mem_ಖಂಡ_Hdr {
  i64 i_ಗಾತ್ರ;                          /* Size of this allocation */
  ರಚನೆ Mem_ಖಂಡ_Hdr *p_ಮುಂದಿನ, *pPrev;  /* Linked list of all unfreed memory */
  ಅಕ್ಷರ nBacktrace;                    /* Number of backtraces on this alloc */
  ಅಕ್ಷರ nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  ಪೂರ್ಣಾಂಕ iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#ವ್ಯಾಖ್ಯಾನಿಸು FOREGUARD 0x80F5E153
#ವ್ಯಾಖ್ಯಾನಿಸು REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು NCSIZE  1000

/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ಸ್ಥಿರ ರಚನೆ {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  ರಚನೆ Mem_ಖಂಡ_Hdr *p_ಮೊದಲ;
  ರಚನೆ Mem_ಖಂಡ_Hdr *p_ಕೊನೆಯ;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  ಪೂರ್ಣಾಂಕ nBacktrace;
  ಶೂನ್ಯ (*xBacktrace)(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ **);

  /*
  ** Title text to insert in front of each block
  */
  ಪೂರ್ಣಾಂಕ nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  ಅಕ್ಷರ zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  ಪೂರ್ಣಾಂಕ disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  ಪೂರ್ಣಾಂಕ nAlloc[NCSIZE];      /* Total number of allocations */
  ಪೂರ್ಣಾಂಕ nCurrent[NCSIZE];    /* Current number of allocations */
  ಪೂರ್ಣಾಂಕ mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjustStats(ಪೂರ್ಣಾಂಕ i_ಗಾತ್ರ, ಪೂರ್ಣಾಂಕ increment){
  ಪೂರ್ಣಾಂಕ i = ROUND8(i_ಗಾತ್ರ)/8;
  ಆದರೆ( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  ಆದರೆ( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    ಆದರೆ( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
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
ಸ್ಥಿರ ರಚನೆ Mem_ಖಂಡ_Hdr *sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಹಂಚಿಕೆ){
  ರಚನೆ Mem_ಖಂಡ_Hdr *p;
  ಪೂರ್ಣಾಂಕ *pInt;
  u8 *pU8;
  ಪೂರ್ಣಾಂಕ nReserve;

  p = (ರಚನೆ Mem_ಖಂಡ_Hdr*)p_ಹಂಚಿಕೆ;
  p--;
  ಪ್ರತಿಜ್ಞಾ( p->iForeGuard==(ಪೂರ್ಣಾಂಕ)FOREGUARD );
  nReserve = ROUND8(p->i_ಗಾತ್ರ);
  pInt = (ಪೂರ್ಣಾಂಕ*)p_ಹಂಚಿಕೆ;
  pU8 = (u8*)p_ಹಂಚಿಕೆ;
  ಪ್ರತಿಜ್ಞಾ( pInt[nReserve/ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)]==(ಪೂರ್ಣಾಂಕ)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  ಆಗ( nReserve-- > p->i_ಗಾತ್ರ ) ಪ್ರತಿಜ್ಞಾ( pU8[nReserve]==0x65 );
  ಹಿಂತಿರುಗಿ p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p){
  ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
  ಆದರೆ( !p ){
    ಹಿಂತಿರುಗಿ 0;
  }
  pHdr = sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(p);
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)pHdr->i_ಗಾತ್ರ;
}

/*
** Initialize the memory allocation subsystem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3MemInit(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಪ್ರತಿಜ್ಞಾ( (ಗಾತ್ರ(ರಚನೆ Mem_ಖಂಡ_Hdr)&7) == 0 );
  ಆದರೆ( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemShutdown(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3MemRoundup(ಪೂರ್ಣಾಂಕ n){
  ಹಿಂತಿರುಗಿ ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ಸ್ಥಿರ ಶೂನ್ಯ randomFill(ಅಕ್ಷರ *pBuf, ಪೂರ್ಣಾಂಕ nByte){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x, y, r;
  x = SQLITE_PTR_ಗಮ್ಯ_INT(pBuf);
  y = nByte | 1;
  ಆಗ( nByte >= 4 ){
    x = (x>>1) ^ (-(ಪೂರ್ಣಾಂಕ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(ಪೂರ್ಣಾಂಕ*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  ಆಗ( nByte-- > 0 ){
    x = (x>>1) ^ (-(ಪೂರ್ಣಾಂಕ)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemMalloc(ಪೂರ್ಣಾಂಕ nByte){
  ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
  ಶೂನ್ಯ **pBt;
  ಅಕ್ಷರ *z;
  ಪೂರ್ಣಾಂಕ *pInt;
  ಶೂನ್ಯ *p = 0;
  ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_ಗಾತ್ರ;
  ಪೂರ್ಣಾಂಕ nReserve;
  sqlite3_mutex_enter(mem.mutex);
  ಪ್ರತಿಜ್ಞಾ( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  ಒಟ್ಟು_ಗಾತ್ರ = nReserve + ಗಾತ್ರ(*pHdr) + ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) +
               mem.nBacktrace*ಗಾತ್ರ(ಶೂನ್ಯ*) + mem.nTitle;
  p = ಸ್ಮರಣೆ_ಕೊಡು(ಒಟ್ಟು_ಗಾತ್ರ);
  ಆದರೆ( p ){
    z = p;
    pBt = (ಶೂನ್ಯ**)&z[mem.nTitle];
    pHdr = (ರಚನೆ Mem_ಖಂಡ_Hdr*)&pBt[mem.nBacktrace];
    pHdr->p_ಮುಂದಿನ = 0;
    pHdr->pPrev = mem.p_ಕೊನೆಯ;
    ಆದರೆ( mem.p_ಕೊನೆಯ ){
      mem.p_ಕೊನೆಯ->p_ಮುಂದಿನ = pHdr;
    }ಇಲ್ಲದಿದ್ದರೆ{
      mem.p_ಮೊದಲ = pHdr;
    }
    mem.p_ಕೊನೆಯ = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_ರಾಶಿ;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    ಆದರೆ( mem.nBacktrace ){
      ಶೂನ್ಯ *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      ಸ್ಮರಣೆ_ನಕಲು(pBt, &aAddr[1], pHdr->nBacktrace*ಗಾತ್ರ(ಶೂನ್ಯ*));
      ಪ್ರತಿಜ್ಞಾ(pBt[0]);
      ಆದರೆ( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      pHdr->nBacktrace = 0;
    }
    ಆದರೆ( mem.nTitle ){
      ಸ್ಮರಣೆ_ನಕಲು(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_ಗಾತ್ರ = nByte;
    adjustStats(nByte, +1);
    pInt = (ಪೂರ್ಣಾಂಕ*)&pHdr[1];
    pInt[nReserve/ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)] = REARGUARD;
    randomFill((ಅಕ್ಷರ*)pInt, nByte);
    memset(((ಅಕ್ಷರ*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (ಶೂನ್ಯ*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  ಹಿಂತಿರುಗಿ p; 
}

/*
** Free memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemFree(ಶೂನ್ಯ *p_ಪೂರ್ವ){
  ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
  ಶೂನ್ಯ **pBt;
  ಅಕ್ಷರ *z;
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(p_ಪೂರ್ವ);
  pBt = (ಶೂನ್ಯ**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  ಆದರೆ( pHdr->pPrev ){
    ಪ್ರತಿಜ್ಞಾ( pHdr->pPrev->p_ಮುಂದಿನ==pHdr );
    pHdr->pPrev->p_ಮುಂದಿನ = pHdr->p_ಮುಂದಿನ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( mem.p_ಮೊದಲ==pHdr );
    mem.p_ಮೊದಲ = pHdr->p_ಮುಂದಿನ;
  }
  ಆದರೆ( pHdr->p_ಮುಂದಿನ ){
    ಪ್ರತಿಜ್ಞಾ( pHdr->p_ಮುಂದಿನ->pPrev==pHdr );
    pHdr->p_ಮುಂದಿನ->pPrev = pHdr->pPrev;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( mem.p_ಕೊನೆಯ==pHdr );
    mem.p_ಕೊನೆಯ = pHdr->pPrev;
  }
  z = (ಅಕ್ಷರ*)pBt;
  z -= pHdr->nTitle;
  adjustStats((ಪೂರ್ಣಾಂಕ)pHdr->i_ಗಾತ್ರ, -1);
  randomFill(z, ಗಾತ್ರ(ಶೂನ್ಯ*)*pHdr->nBacktraceSlots + ಗಾತ್ರ(*pHdr) +
                (ಪೂರ್ಣಾಂಕ)pHdr->i_ಗಾತ್ರ + ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) + pHdr->nTitle);
  ಬಿಡುಗಡೆ(z);
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
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemRealloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ nByte){
  ರಚನೆ Mem_ಖಂಡ_Hdr *pOldHdr;
  ಶೂನ್ಯ *pNew;
  ಪ್ರತಿಜ್ಞಾ( mem.disallow==0 );
  ಪ್ರತಿಜ್ಞಾ( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(p_ಪೂರ್ವ);
  pNew = sqlite3MemMalloc(nByte);
  ಆದರೆ( pNew ){
    ಸ್ಮರಣೆ_ನಕಲು(pNew, p_ಪೂರ್ವ, (ಪೂರ್ಣಾಂಕ)(nByte<pOldHdr->i_ಗಾತ್ರ ? nByte : pOldHdr->i_ಗಾತ್ರ));
    ಆದರೆ( nByte>pOldHdr->i_ಗಾತ್ರ ){
      randomFill(&((ಅಕ್ಷರ*)pNew)[pOldHdr->i_ಗಾತ್ರ], nByte - (ಪೂರ್ಣಾಂಕ)pOldHdr->i_ಗಾತ್ರ);
    }
    sqlite3MemFree(p_ಪೂರ್ವ);
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ಶೂನ್ಯ sqlite3_Mem_ಗಣ_Default(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ಗಾತ್ರ,
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
ಶೂನ್ಯ sqlite3_Memdebug_ಗಣ_Type(ಶೂನ್ಯ *p, u8 eType){
  ಆದರೆ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(p);
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
ಪೂರ್ಣಾಂಕ sqlite3MemdebugHasType(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p, u8 eType){
  ಪೂರ್ಣಾಂಕ rc = 1;
  ಆದರೆ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(p);
    ಪ್ರತಿಜ್ಞಾ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ಆದರೆ( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3MemdebugNoType(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p, u8 eType){
  ಪೂರ್ಣಾಂಕ rc = 1;
  ಆದರೆ( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_ಶೀರ್ಷಿಕೆ(p);
    ಪ್ರತಿಜ್ಞಾ( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    ಆದರೆ( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
ಶೂನ್ಯ sqlite3MemdebugBacktrace(ಪೂರ್ಣಾಂಕ ಆಳ){
  ಆದರೆ( ಆಳ<0 ){ ಆಳ = 0; }
  ಆದರೆ( ಆಳ>20 ){ ಆಳ = 20; }
  ಆಳ = (ಆಳ+1)&0xfe;
  mem.nBacktrace = ಆಳ;
}

ಶೂನ್ಯ sqlite3MemdebugBacktraceCallback(ಶೂನ್ಯ (*xBacktrace)(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
ಶೂನ್ಯ sqlite3MemdebugSettitle(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTitle){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  ಆದರೆ( n>=ಗಾತ್ರ(mem.zTitle) ) n = ಗಾತ್ರ(mem.zTitle)-1;
  ಸ್ಮರಣೆ_ನಕಲು(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

ಶೂನ್ಯ sqlite3MemdebugSync(){
  ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
  ಗಾಗಿ(pHdr=mem.p_ಮೊದಲ; pHdr; pHdr=pHdr->p_ಮುಂದಿನ){
    ಶೂನ್ಯ **pBt = (ಶೂನ್ಯ**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((ಪೂರ್ಣಾಂಕ)pHdr->i_ಗಾತ್ರ, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ಶೂನ್ಯ sqlite3MemdebugDump(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  FILE *ಹೊರಹೋಗು;
  ರಚನೆ Mem_ಖಂಡ_Hdr *pHdr;
  ಶೂನ್ಯ **pBt;
  ಪೂರ್ಣಾಂಕ i;
  ಹೊರಹೋಗು = fopen(zFilename, "w");
  ಆದರೆ( ಹೊರಹೋಗು==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    ಹಿಂತಿರುಗಿ;
  }
  ಗಾಗಿ(pHdr=mem.p_ಮೊದಲ; pHdr; pHdr=pHdr->p_ಮುಂದಿನ){
    ಅಕ್ಷರ *z = (ಅಕ್ಷರ*)pHdr;
    z -= pHdr->nBacktraceSlots*ಗಾತ್ರ(ಶೂನ್ಯ*) + pHdr->nTitle;
    fprintf(ಹೊರಹೋಗು, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_ಗಾತ್ರ, &pHdr[1], pHdr->nTitle ? z : "???");
    ಆದರೆ( pHdr->nBacktrace ){
      fflush(ಹೊರಹೋಗು);
      pBt = (ಶೂನ್ಯ**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(ಹೊರಹೋಗು));
      fprintf(ಹೊರಹೋಗು, "\n");
    }
  }
  fprintf(ಹೊರಹೋಗು, "COUNTS:\n");
  ಗಾಗಿ(i=0; i<NCSIZE-1; i++){
    ಆದರೆ( mem.nAlloc[i] ){
      fprintf(ಹೊರಹೋಗು, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  ಆದರೆ( mem.nAlloc[NCSIZE-1] ){
    fprintf(ಹೊರಹೋಗು, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(ಹೊರಹೋಗು);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Memdebug_Malloc_ಎಣಿಕೆ(){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n_ಒಟ್ಟು = 0;
  ಗಾಗಿ(i=0; i<NCSIZE; i++){
    n_ಒಟ್ಟು += mem.nAlloc[i];
  }
  ಹಿಂತಿರುಗಿ n_ಒಟ್ಟು;
}


#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MEMDEBUG */
