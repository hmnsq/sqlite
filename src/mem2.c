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
#ضمن "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#جے_تعریف_شدہ SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#جے_تعریف_شدہ __GLIBC__
  خارجي صحيح backtrace(فراغ**,صحيح);
  خارجي فراغ backtrace_symbols_fd(فراغ*ثابت*,صحيح,صحيح);
#وإلا
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#جے_انت
#ضمن <stdio.h>

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
بنية Mem_بلاک_Hdr {
  i64 i_ناپ;                          /* Size of this allocation */
  بنية Mem_بلاک_Hdr *p_اگلا, *pPrev;  /* Linked list of all unfreed memory */
  حرف nBacktrace;                    /* Number of backtraces on this alloc */
  حرف nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  صحيح iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#تعریف_کرو FOREGUARD 0x80F5E153
#تعریف_کرو REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#تعریف_کرو NCSIZE  1000

/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ساكن بنية {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  بنية Mem_بلاک_Hdr *p_پہلا;
  بنية Mem_بلاک_Hdr *p_آخری;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  صحيح nBacktrace;
  فراغ (*xBacktrace)(صحيح, صحيح, فراغ **);

  /*
  ** Title text to insert in front of each block
  */
  صحيح nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  حرف zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  صحيح disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  صحيح nAlloc[NCSIZE];      /* Total number of allocations */
  صحيح nCurrent[NCSIZE];    /* Current number of allocations */
  صحيح mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ساكن فراغ adjustStats(صحيح i_ناپ, صحيح increment){
  صحيح i = ROUND8(i_ناپ)/8;
  إذا( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  إذا( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    إذا( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }وإلا{
    mem.nCurrent[i]--;
    أكد( mem.nCurrent[i]>=0 );
  }
}

/*
** Given an allocation, find the MemBlockHdr for that allocation.
**
** This routine checks the guards at either end of the allocation and
** if they are incorrect it asserts.
*/
ساكن بنية Mem_بلاک_Hdr *sqlite3_Memsys_Get_سرخی(ثابت فراغ *p_ونڈ){
  بنية Mem_بلاک_Hdr *p;
  صحيح *pInt;
  u8 *pU8;
  صحيح nReserve;

  p = (بنية Mem_بلاک_Hdr*)p_ونڈ;
  p--;
  أكد( p->iForeGuard==(صحيح)FOREGUARD );
  nReserve = ROUND8(p->i_ناپ);
  pInt = (صحيح*)p_ونڈ;
  pU8 = (u8*)p_ونڈ;
  أكد( pInt[nReserve/حجم(صحيح)]==(صحيح)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  طالما( nReserve-- > p->i_ناپ ) أكد( pU8[nReserve]==0x65 );
  أرجع p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ساكن صحيح sqlite3_Mem_ناپ(فراغ *p){
  بنية Mem_بلاک_Hdr *pHdr;
  إذا( !p ){
    أرجع 0;
  }
  pHdr = sqlite3_Memsys_Get_سرخی(p);
  أرجع (صحيح)pHdr->i_ناپ;
}

/*
** Initialize the memory allocation subsystem.
*/
ساكن صحيح sqlite3MemInit(فراغ *Not_ورتیا){
  بن_ورتیا_PARAMETER(Not_ورتیا);
  أكد( (حجم(بنية Mem_بلاک_Hdr)&7) == 0 );
  إذا( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  أرجع SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ساكن فراغ sqlite3MemShutdown(فراغ *Not_ورتیا){
  بن_ورتیا_PARAMETER(Not_ورتیا);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساكن صحيح sqlite3MemRoundup(صحيح n){
  أرجع ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ساكن فراغ randomFill(حرف *pBuf, صحيح nByte){
  بنانشانی صحيح x, y, r;
  x = SQLITE_PTR_منزل_INT(pBuf);
  y = nByte | 1;
  طالما( nByte >= 4 ){
    x = (x>>1) ^ (-(صحيح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(صحيح*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  طالما( nByte-- > 0 ){
    x = (x>>1) ^ (-(صحيح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ساكن فراغ *sqlite3MemMalloc(صحيح nByte){
  بنية Mem_بلاک_Hdr *pHdr;
  فراغ **pBt;
  حرف *z;
  صحيح *pInt;
  فراغ *p = 0;
  صحيح کل_ناپ;
  صحيح nReserve;
  sqlite3_mutex_enter(mem.mutex);
  أكد( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  کل_ناپ = nReserve + حجم(*pHdr) + حجم(صحيح) +
               mem.nBacktrace*حجم(فراغ*) + mem.nTitle;
  p = حافظہ_دیو(کل_ناپ);
  إذا( p ){
    z = p;
    pBt = (فراغ**)&z[mem.nTitle];
    pHdr = (بنية Mem_بلاک_Hdr*)&pBt[mem.nBacktrace];
    pHdr->p_اگلا = 0;
    pHdr->pPrev = mem.p_آخری;
    إذا( mem.p_آخری ){
      mem.p_آخری->p_اگلا = pHdr;
    }وإلا{
      mem.p_پہلا = pHdr;
    }
    mem.p_آخری = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_ڈھیر;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    إذا( mem.nBacktrace ){
      فراغ *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      حافظہ_نقل(pBt, &aAddr[1], pHdr->nBacktrace*حجم(فراغ*));
      أكد(pBt[0]);
      إذا( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }وإلا{
      pHdr->nBacktrace = 0;
    }
    إذا( mem.nTitle ){
      حافظہ_نقل(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_ناپ = nByte;
    adjustStats(nByte, +1);
    pInt = (صحيح*)&pHdr[1];
    pInt[nReserve/حجم(صحيح)] = REARGUARD;
    randomFill((حرف*)pInt, nByte);
    memset(((حرف*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (فراغ*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  أرجع p; 
}

/*
** Free memory.
*/
ساكن فراغ sqlite3MemFree(فراغ *p_پہلوں){
  بنية Mem_بلاک_Hdr *pHdr;
  فراغ **pBt;
  حرف *z;
  أكد( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3_Memsys_Get_سرخی(p_پہلوں);
  pBt = (فراغ**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  إذا( pHdr->pPrev ){
    أكد( pHdr->pPrev->p_اگلا==pHdr );
    pHdr->pPrev->p_اگلا = pHdr->p_اگلا;
  }وإلا{
    أكد( mem.p_پہلا==pHdr );
    mem.p_پہلا = pHdr->p_اگلا;
  }
  إذا( pHdr->p_اگلا ){
    أكد( pHdr->p_اگلا->pPrev==pHdr );
    pHdr->p_اگلا->pPrev = pHdr->pPrev;
  }وإلا{
    أكد( mem.p_آخری==pHdr );
    mem.p_آخری = pHdr->pPrev;
  }
  z = (حرف*)pBt;
  z -= pHdr->nTitle;
  adjustStats((صحيح)pHdr->i_ناپ, -1);
  randomFill(z, حجم(فراغ*)*pHdr->nBacktraceSlots + حجم(*pHdr) +
                (صحيح)pHdr->i_ناپ + حجم(صحيح) + pHdr->nTitle);
  حرر(z);
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
ساكن فراغ *sqlite3MemRealloc(فراغ *p_پہلوں, صحيح nByte){
  بنية Mem_بلاک_Hdr *pOldHdr;
  فراغ *pNew;
  أكد( mem.disallow==0 );
  أكد( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3_Memsys_Get_سرخی(p_پہلوں);
  pNew = sqlite3MemMalloc(nByte);
  إذا( pNew ){
    حافظہ_نقل(pNew, p_پہلوں, (صحيح)(nByte<pOldHdr->i_ناپ ? nByte : pOldHdr->i_ناپ));
    إذا( nByte>pOldHdr->i_ناپ ){
      randomFill(&((حرف*)pNew)[pOldHdr->i_ناپ], nByte - (صحيح)pOldHdr->i_ناپ);
    }
    sqlite3MemFree(p_پہلوں);
  }
  أرجع pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
فراغ sqlite3_Mem_سیٹ_Default(فراغ){
  ساكن ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ناپ,
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
فراغ sqlite3_Memdebug_سیٹ_Type(فراغ *p, u8 eType){
  إذا( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    بنية Mem_بلاک_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_سرخی(p);
    أكد( pHdr->iForeGuard==FOREGUARD );
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
صحيح sqlite3MemdebugHasType(ثابت فراغ *p, u8 eType){
  صحيح rc = 1;
  إذا( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    بنية Mem_بلاک_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_سرخی(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    إذا( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  أرجع rc;
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
صحيح sqlite3MemdebugNoType(ثابت فراغ *p, u8 eType){
  صحيح rc = 1;
  إذا( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    بنية Mem_بلاک_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_سرخی(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    إذا( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  أرجع rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
فراغ sqlite3MemdebugBacktrace(صحيح ڈونگھائی){
  إذا( ڈونگھائی<0 ){ ڈونگھائی = 0; }
  إذا( ڈونگھائی>20 ){ ڈونگھائی = 20; }
  ڈونگھائی = (ڈونگھائی+1)&0xfe;
  mem.nBacktrace = ڈونگھائی;
}

فراغ sqlite3MemdebugBacktraceCallback(فراغ (*xBacktrace)(صحيح, صحيح, فراغ **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
فراغ sqlite3MemdebugSettitle(ثابت حرف *zTitle){
  بنانشانی صحيح n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  إذا( n>=حجم(mem.zTitle) ) n = حجم(mem.zTitle)-1;
  حافظہ_نقل(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

فراغ sqlite3MemdebugSync(){
  بنية Mem_بلاک_Hdr *pHdr;
  لكل(pHdr=mem.p_پہلا; pHdr; pHdr=pHdr->p_اگلا){
    فراغ **pBt = (فراغ**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((صحيح)pHdr->i_ناپ, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
فراغ sqlite3MemdebugDump(ثابت حرف *zFilename){
  FILE *باہرلا;
  بنية Mem_بلاک_Hdr *pHdr;
  فراغ **pBt;
  صحيح i;
  باہرلا = fopen(zFilename, "w");
  إذا( باہرلا==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    أرجع;
  }
  لكل(pHdr=mem.p_پہلا; pHdr; pHdr=pHdr->p_اگلا){
    حرف *z = (حرف*)pHdr;
    z -= pHdr->nBacktraceSlots*حجم(فراغ*) + pHdr->nTitle;
    fprintf(باہرلا, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_ناپ, &pHdr[1], pHdr->nTitle ? z : "???");
    إذا( pHdr->nBacktrace ){
      fflush(باہرلا);
      pBt = (فراغ**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(باہرلا));
      fprintf(باہرلا, "\n");
    }
  }
  fprintf(باہرلا, "COUNTS:\n");
  لكل(i=0; i<NCSIZE-1; i++){
    إذا( mem.nAlloc[i] ){
      fprintf(باہرلا, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  إذا( mem.nAlloc[NCSIZE-1] ){
    fprintf(باہرلا, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(باہرلا);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
صحيح sqlite3_Memdebug_Malloc_گنتی(){
  صحيح i;
  صحيح n_کل = 0;
  لكل(i=0; i<NCSIZE; i++){
    n_کل += mem.nAlloc[i];
  }
  أرجع n_کل;
}


#جے_انت /* SQLITE_MEMDEBUG */
