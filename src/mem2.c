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
#شامل "sqliteInt.h"

/*
** This version of the memory allocator is used only if the
** SQLITE_MEMDEBUG macro is defined
*/
#اگر_تعریف_شدہ SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#اگر_تعریف_شدہ __GLIBC__
  خارجی صحیح backtrace(خالی**,صحیح);
  خارجی خالی backtrace_symbols_fd(خالی*ثابت*,صحیح,صحیح);
#ورنہ
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#اگر_ختم
#شامل <stdio.h>

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
ساخت Mem_بلاک_Hdr {
  i64 i_سائز;                          /* Size of this allocation */
  ساخت Mem_بلاک_Hdr *p_اگلا, *p_پچھلا;  /* Linked list of all unfreed memory */
  حرف nBacktrace;                    /* Number of backtraces on this alloc */
  حرف nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  صحیح iForeGuard;                     /* Guard word for sanity */
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
ساکن ساخت {
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

  /*
  ** Head and tail of a linked list of all outstanding allocations
  */
  ساخت Mem_بلاک_Hdr *p_پہلا;
  ساخت Mem_بلاک_Hdr *p_آخری;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  صحیح nBacktrace;
  خالی (*xBacktrace)(صحیح, صحیح, خالی **);

  /*
  ** Title text to insert in front of each block
  */
  صحیح nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  حرف zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  صحیح disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  صحیح nAlloc[NCSIZE];      /* Total number of allocations */
  صحیح nCurrent[NCSIZE];    /* Current number of allocations */
  صحیح mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
ساکن خالی adjustStats(صحیح i_سائز, صحیح increment){
  صحیح i = ROUND8(i_سائز)/8;
  اگر( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  اگر( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    اگر( mem.nCurrent[i]>mem.mxCurrent[i] ){
      mem.mxCurrent[i] = mem.nCurrent[i];
    }
  }ورنہ{
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
ساکن ساخت Mem_بلاک_Hdr *sqlite3_Memsys_Get_سرخی(ثابت خالی *p_تخصیص){
  ساخت Mem_بلاک_Hdr *p;
  صحیح *pInt;
  u8 *pU8;
  صحیح nReserve;

  p = (ساخت Mem_بلاک_Hdr*)p_تخصیص;
  p--;
  أكد( p->iForeGuard==(صحیح)FOREGUARD );
  nReserve = ROUND8(p->i_سائز);
  pInt = (صحیح*)p_تخصیص;
  pU8 = (u8*)p_تخصیص;
  أكد( pInt[nReserve/حجم(صحیح)]==(صحیح)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  جب( nReserve-- > p->i_سائز ) أكد( pU8[nReserve]==0x65 );
  واپس p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
ساکن صحیح sqlite3_Mem_سائز(خالی *p){
  ساخت Mem_بلاک_Hdr *pHdr;
  اگر( !p ){
    واپس 0;
  }
  pHdr = sqlite3_Memsys_Get_سرخی(p);
  واپس (صحیح)pHdr->i_سائز;
}

/*
** Initialize the memory allocation subsystem.
*/
ساکن صحیح sqlite3MemInit(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  أكد( (حجم(ساخت Mem_بلاک_Hdr)&7) == 0 );
  اگر( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  واپس SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
ساکن خالی sqlite3MemShutdown(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساکن صحیح sqlite3MemRoundup(صحیح n){
  واپس ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
ساکن خالی randomFill(حرف *pBuf, صحیح nByte){
  غیرعلامتی صحیح x, y, r;
  x = SQLITE_PTR_مقصد_INT(pBuf);
  y = nByte | 1;
  جب( nByte >= 4 ){
    x = (x>>1) ^ (-(صحیح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(صحیح*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  جب( nByte-- > 0 ){
    x = (x>>1) ^ (-(صحیح)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
ساکن خالی *sqlite3MemMalloc(صحیح nByte){
  ساخت Mem_بلاک_Hdr *pHdr;
  خالی **pBt;
  حرف *z;
  صحیح *pInt;
  خالی *p = 0;
  صحیح کل_سائز;
  صحیح nReserve;
  sqlite3_mutex_enter(mem.mutex);
  أكد( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  کل_سائز = nReserve + حجم(*pHdr) + حجم(صحیح) +
               mem.nBacktrace*حجم(خالی*) + mem.nTitle;
  p = حافظہ_دو(کل_سائز);
  اگر( p ){
    z = p;
    pBt = (خالی**)&z[mem.nTitle];
    pHdr = (ساخت Mem_بلاک_Hdr*)&pBt[mem.nBacktrace];
    pHdr->p_اگلا = 0;
    pHdr->p_پچھلا = mem.p_آخری;
    اگر( mem.p_آخری ){
      mem.p_آخری->p_اگلا = pHdr;
    }ورنہ{
      mem.p_پہلا = pHdr;
    }
    mem.p_آخری = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_ڈھیر;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    اگر( mem.nBacktrace ){
      خالی *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      حافظہ_نقل(pBt, &aAddr[1], pHdr->nBacktrace*حجم(خالی*));
      أكد(pBt[0]);
      اگر( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }ورنہ{
      pHdr->nBacktrace = 0;
    }
    اگر( mem.nTitle ){
      حافظہ_نقل(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_سائز = nByte;
    adjustStats(nByte, +1);
    pInt = (صحیح*)&pHdr[1];
    pInt[nReserve/حجم(صحیح)] = REARGUARD;
    randomFill((حرف*)pInt, nByte);
    memset(((حرف*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (خالی*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  واپس p; 
}

/*
** Free memory.
*/
ساکن خالی sqlite3MemFree(خالی *p_پیشگی){
  ساخت Mem_بلاک_Hdr *pHdr;
  خالی **pBt;
  حرف *z;
  أكد( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3_Memsys_Get_سرخی(p_پیشگی);
  pBt = (خالی**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  اگر( pHdr->p_پچھلا ){
    أكد( pHdr->p_پچھلا->p_اگلا==pHdr );
    pHdr->p_پچھلا->p_اگلا = pHdr->p_اگلا;
  }ورنہ{
    أكد( mem.p_پہلا==pHdr );
    mem.p_پہلا = pHdr->p_اگلا;
  }
  اگر( pHdr->p_اگلا ){
    أكد( pHdr->p_اگلا->p_پچھلا==pHdr );
    pHdr->p_اگلا->p_پچھلا = pHdr->p_پچھلا;
  }ورنہ{
    أكد( mem.p_آخری==pHdr );
    mem.p_آخری = pHdr->p_پچھلا;
  }
  z = (حرف*)pBt;
  z -= pHdr->nTitle;
  adjustStats((صحیح)pHdr->i_سائز, -1);
  randomFill(z, حجم(خالی*)*pHdr->nBacktraceSlots + حجم(*pHdr) +
                (صحیح)pHdr->i_سائز + حجم(صحیح) + pHdr->nTitle);
  آزاد(z);
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
ساکن خالی *sqlite3MemRealloc(خالی *p_پیشگی, صحیح nByte){
  ساخت Mem_بلاک_Hdr *pOldHdr;
  خالی *pNew;
  أكد( mem.disallow==0 );
  أكد( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3_Memsys_Get_سرخی(p_پیشگی);
  pNew = sqlite3MemMalloc(nByte);
  اگر( pNew ){
    حافظہ_نقل(pNew, p_پیشگی, (صحیح)(nByte<pOldHdr->i_سائز ? nByte : pOldHdr->i_سائز));
    اگر( nByte>pOldHdr->i_سائز ){
      randomFill(&((حرف*)pNew)[pOldHdr->i_سائز], nByte - (صحیح)pOldHdr->i_سائز);
    }
    sqlite3MemFree(p_پیشگی);
  }
  واپس pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
خالی sqlite3_Mem_سیٹ_Default(خالی){
  ساکن ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_سائز,
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
خالی sqlite3_Memdebug_سیٹ_Type(خالی *p, u8 eType){
  اگر( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ساخت Mem_بلاک_Hdr *pHdr;
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
صحیح sqlite3MemdebugHasType(ثابت خالی *p, u8 eType){
  صحیح rc = 1;
  اگر( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ساخت Mem_بلاک_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_سرخی(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    اگر( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  واپس rc;
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
صحیح sqlite3MemdebugNoType(ثابت خالی *p, u8 eType){
  صحیح rc = 1;
  اگر( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    ساخت Mem_بلاک_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_سرخی(p);
    أكد( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    اگر( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  واپس rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
خالی sqlite3MemdebugBacktrace(صحیح گہرائی){
  اگر( گہرائی<0 ){ گہرائی = 0; }
  اگر( گہرائی>20 ){ گہرائی = 20; }
  گہرائی = (گہرائی+1)&0xfe;
  mem.nBacktrace = گہرائی;
}

خالی sqlite3MemdebugBacktraceCallback(خالی (*xBacktrace)(صحیح, صحیح, خالی **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
خالی sqlite3MemdebugSettitle(ثابت حرف *zTitle){
  غیرعلامتی صحیح n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  اگر( n>=حجم(mem.zTitle) ) n = حجم(mem.zTitle)-1;
  حافظہ_نقل(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

خالی sqlite3MemdebugSync(){
  ساخت Mem_بلاک_Hdr *pHdr;
  برائے(pHdr=mem.p_پہلا; pHdr; pHdr=pHdr->p_اگلا){
    خالی **pBt = (خالی**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((صحیح)pHdr->i_سائز, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
خالی sqlite3MemdebugDump(ثابت حرف *zFilename){
  FILE *برآمد;
  ساخت Mem_بلاک_Hdr *pHdr;
  خالی **pBt;
  صحیح i;
  برآمد = fopen(zFilename, "w");
  اگر( برآمد==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    واپس;
  }
  برائے(pHdr=mem.p_پہلا; pHdr; pHdr=pHdr->p_اگلا){
    حرف *z = (حرف*)pHdr;
    z -= pHdr->nBacktraceSlots*حجم(خالی*) + pHdr->nTitle;
    fprintf(برآمد, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_سائز, &pHdr[1], pHdr->nTitle ? z : "???");
    اگر( pHdr->nBacktrace ){
      fflush(برآمد);
      pBt = (خالی**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(برآمد));
      fprintf(برآمد, "\n");
    }
  }
  fprintf(برآمد, "COUNTS:\n");
  برائے(i=0; i<NCSIZE-1; i++){
    اگر( mem.nAlloc[i] ){
      fprintf(برآمد, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  اگر( mem.nAlloc[NCSIZE-1] ){
    fprintf(برآمد, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(برآمد);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
صحیح sqlite3_Memdebug_Malloc_گنتی(){
  صحیح i;
  صحیح n_کل = 0;
  برائے(i=0; i<NCSIZE; i++){
    n_کل += mem.nAlloc[i];
  }
  واپس n_کل;
}


#اگر_ختم /* SQLITE_MEMDEBUG */
