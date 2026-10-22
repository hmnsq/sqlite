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
#यदि_परिभाषितम् SQLITE_MEMDEBUG

/*
** The backtrace functionality is only available with GLIBC
*/
#यदि_परिभाषितम् __GLIBC__
  बाह्य पूर्णाङ्क backtrace(शून्य**,पूर्णाङ्क);
  बाह्य शून्य backtrace_symbols_fd(शून्य*ध्रुव*,पूर्णाङ्क,पूर्णाङ्क);
#अन्यथा
# define backtrace(A,B) 1
# define backtrace_symbols_fd(A,B,C)
#यदि_अन्तः
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
संरचना Mem_खण्डम्_Hdr {
  i64 i_आकार;                          /* Size of this allocation */
  संरचना Mem_खण्डम्_Hdr *p_अग्र, *p_पूर्व;  /* Linked list of all unfreed memory */
  वर्ण nBacktrace;                    /* Number of backtraces on this alloc */
  वर्ण nBacktraceSlots;               /* Available backtrace slots */
  u8 nTitle;                          /* Bytes of title; includes '\0' */
  u8 eType;                           /* Allocation type code */
  पूर्णाङ्क iForeGuard;                     /* Guard word for sanity */
};

/*
** Guard words
*/
#परिभाषय FOREGUARD 0x80F5E153
#परिभाषय REARGUARD 0xE4676B53

/*
** Number of malloc size increments to track.
*/
#परिभाषय NCSIZE  1000

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
  संरचना Mem_खण्डम्_Hdr *pFirst;
  संरचना Mem_खण्डम्_Hdr *p_चरमः;
  
  /*
  ** The number of levels of backtrace to save in new allocations.
  */
  पूर्णाङ्क nBacktrace;
  शून्य (*xBacktrace)(पूर्णाङ्क, पूर्णाङ्क, शून्य **);

  /*
  ** Title text to insert in front of each block
  */
  पूर्णाङ्क nTitle;        /* Bytes of zTitle to save.  Includes '\0' and padding */
  वर्ण zTitle[100];  /* The title text */

  /* 
  ** sqlite3MallocDisallow() increments the following counter.
  ** sqlite3MallocAllow() decrements it.
  */
  पूर्णाङ्क disallow; /* Do not allow memory allocation */

  /*
  ** Gather statistics on the sizes of memory allocations.
  ** nAlloc[i] is the number of allocation attempts of i*8
  ** bytes.  i==NCSIZE is the number of allocation attempts for
  ** sizes more than NCSIZE*8 bytes.
  */
  पूर्णाङ्क nAlloc[NCSIZE];      /* Total number of allocations */
  पूर्णाङ्क nCurrent[NCSIZE];    /* Current number of allocations */
  पूर्णाङ्क mxCurrent[NCSIZE];   /* Highwater mark for nCurrent */

} mem;


/*
** Adjust memory usage statistics
*/
स्थिर शून्य adjustStats(पूर्णाङ्क i_आकार, पूर्णाङ्क increment){
  पूर्णाङ्क i = ROUND8(i_आकार)/8;
  यदि( i>NCSIZE-1 ){
    i = NCSIZE - 1;
  }
  यदि( increment>0 ){
    mem.nAlloc[i]++;
    mem.nCurrent[i]++;
    यदि( mem.nCurrent[i]>mem.mxCurrent[i] ){
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
स्थिर संरचना Mem_खण्डम्_Hdr *sqlite3_Memsys_Get_शीर्षकम्(ध्रुव शून्य *p_आवण्टनम्){
  संरचना Mem_खण्डम्_Hdr *p;
  पूर्णाङ्क *pInt;
  u8 *pU8;
  पूर्णाङ्क nReserve;

  p = (संरचना Mem_खण्डम्_Hdr*)p_आवण्टनम्;
  p--;
  प्रतिज्ञा( p->iForeGuard==(पूर्णाङ्क)FOREGUARD );
  nReserve = ROUND8(p->i_आकार);
  pInt = (पूर्णाङ्क*)p_आवण्टनम्;
  pU8 = (u8*)p_आवण्टनम्;
  प्रतिज्ञा( pInt[nReserve/परिमाण(पूर्णाङ्क)]==(पूर्णाङ्क)REARGUARD );
  /* This checks any of the "extra" bytes allocated due
  ** to rounding up to an 8 byte boundary to ensure 
  ** they haven't been overwritten.
  */
  यावत्( nReserve-- > p->i_आकार ) प्रतिज्ञा( pU8[nReserve]==0x65 );
  प्रत्यागम p;
}

/*
** Return the number of bytes currently allocated at address p.
*/
स्थिर पूर्णाङ्क sqlite3_Mem_आकार(शून्य *p){
  संरचना Mem_खण्डम्_Hdr *pHdr;
  यदि( !p ){
    प्रत्यागम 0;
  }
  pHdr = sqlite3_Memsys_Get_शीर्षकम्(p);
  प्रत्यागम (पूर्णाङ्क)pHdr->i_आकार;
}

/*
** Initialize the memory allocation subsystem.
*/
स्थिर पूर्णाङ्क sqlite3MemInit(शून्य *Not_प्रयुक्तम्){
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  प्रतिज्ञा( (परिमाण(संरचना Mem_खण्डम्_Hdr)&7) == 0 );
  यदि( !sqlite3GlobalConfig.bMemstat ){
    /* If memory status is enabled, then the malloc.c wrapper will already
    ** hold the STATIC_MEM mutex when the routines here are invoked. */
    mem.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Deinitialize the memory allocation subsystem.
*/
स्थिर शून्य sqlite3MemShutdown(शून्य *Not_प्रयुक्तम्){
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  mem.mutex = 0;
}

/*
** Round up a request size to the next valid allocation size.
*/
स्थिर पूर्णाङ्क sqlite3MemRoundup(पूर्णाङ्क n){
  प्रत्यागम ROUND8(n);
}

/*
** Fill a buffer with pseudo-random bytes.  This is used to preset
** the content of a new memory allocation to unpredictable values and
** to clear the content of a freed allocation to unpredictable values.
*/
स्थिर शून्य randomFill(वर्ण *pBuf, पूर्णाङ्क nByte){
  अचिह्नित पूर्णाङ्क x, y, r;
  x = SQLITE_PTR_गन्तव्यम्_INT(pBuf);
  y = nByte | 1;
  यावत्( nByte >= 4 ){
    x = (x>>1) ^ (-(पूर्णाङ्क)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(पूर्णाङ्क*)pBuf = r;
    pBuf += 4;
    nByte -= 4;
  }
  यावत्( nByte-- > 0 ){
    x = (x>>1) ^ (-(पूर्णाङ्क)(x&1) & 0xd0000001);
    y = y*1103515245 + 12345;
    r = x ^ y;
    *(pBuf++) = r & 0xff;
  }
}

/*
** Allocate nByte bytes of memory.
*/
स्थिर शून्य *sqlite3MemMalloc(पूर्णाङ्क nByte){
  संरचना Mem_खण्डम्_Hdr *pHdr;
  शून्य **pBt;
  वर्ण *z;
  पूर्णाङ्क *pInt;
  शून्य *p = 0;
  पूर्णाङ्क योग_आकार;
  पूर्णाङ्क nReserve;
  sqlite3_mutex_enter(mem.mutex);
  प्रतिज्ञा( mem.disallow==0 );
  nReserve = ROUND8(nByte);
  योग_आकार = nReserve + परिमाण(*pHdr) + परिमाण(पूर्णाङ्क) +
               mem.nBacktrace*परिमाण(शून्य*) + mem.nTitle;
  p = स्मृतिं_देहि(योग_आकार);
  यदि( p ){
    z = p;
    pBt = (शून्य**)&z[mem.nTitle];
    pHdr = (संरचना Mem_खण्डम्_Hdr*)&pBt[mem.nBacktrace];
    pHdr->p_अग्र = 0;
    pHdr->p_पूर्व = mem.p_चरमः;
    यदि( mem.p_चरमः ){
      mem.p_चरमः->p_अग्र = pHdr;
    }अन्यथा{
      mem.pFirst = pHdr;
    }
    mem.p_चरमः = pHdr;
    pHdr->iForeGuard = FOREGUARD;
    pHdr->eType = MEMTYPE_HEAP;
    pHdr->nBacktraceSlots = mem.nBacktrace;
    pHdr->nTitle = mem.nTitle;
    यदि( mem.nBacktrace ){
      शून्य *aAddr[40];
      pHdr->nBacktrace = backtrace(aAddr, mem.nBacktrace+1)-1;
      स्मृतिं_प्रतिलिख(pBt, &aAddr[1], pHdr->nBacktrace*परिमाण(शून्य*));
      प्रतिज्ञा(pBt[0]);
      यदि( mem.xBacktrace ){
        mem.xBacktrace(nByte, pHdr->nBacktrace-1, &aAddr[1]);
      }
    }अन्यथा{
      pHdr->nBacktrace = 0;
    }
    यदि( mem.nTitle ){
      स्मृतिं_प्रतिलिख(z, mem.zTitle, mem.nTitle);
    }
    pHdr->i_आकार = nByte;
    adjustStats(nByte, +1);
    pInt = (पूर्णाङ्क*)&pHdr[1];
    pInt[nReserve/परिमाण(पूर्णाङ्क)] = REARGUARD;
    randomFill((वर्ण*)pInt, nByte);
    स्मृतिं_पूरय(((वर्ण*)pInt)+nByte, 0x65, nReserve-nByte);
    p = (शून्य*)pInt;
  }
  sqlite3_mutex_leave(mem.mutex);
  प्रत्यागम p; 
}

/*
** Free memory.
*/
स्थिर शून्य sqlite3MemFree(शून्य *p_प्राक्){
  संरचना Mem_खण्डम्_Hdr *pHdr;
  शून्य **pBt;
  वर्ण *z;
  प्रतिज्ञा( sqlite3GlobalConfig.bMemstat || sqlite3GlobalConfig.bCoreMutex==0 
       || mem.mutex!=0 );
  pHdr = sqlite3_Memsys_Get_शीर्षकम्(p_प्राक्);
  pBt = (शून्य**)pHdr;
  pBt -= pHdr->nBacktraceSlots;
  sqlite3_mutex_enter(mem.mutex);
  यदि( pHdr->p_पूर्व ){
    प्रतिज्ञा( pHdr->p_पूर्व->p_अग्र==pHdr );
    pHdr->p_पूर्व->p_अग्र = pHdr->p_अग्र;
  }अन्यथा{
    प्रतिज्ञा( mem.pFirst==pHdr );
    mem.pFirst = pHdr->p_अग्र;
  }
  यदि( pHdr->p_अग्र ){
    प्रतिज्ञा( pHdr->p_अग्र->p_पूर्व==pHdr );
    pHdr->p_अग्र->p_पूर्व = pHdr->p_पूर्व;
  }अन्यथा{
    प्रतिज्ञा( mem.p_चरमः==pHdr );
    mem.p_चरमः = pHdr->p_पूर्व;
  }
  z = (वर्ण*)pBt;
  z -= pHdr->nTitle;
  adjustStats((पूर्णाङ्क)pHdr->i_आकार, -1);
  randomFill(z, परिमाण(शून्य*)*pHdr->nBacktraceSlots + परिमाण(*pHdr) +
                (पूर्णाङ्क)pHdr->i_आकार + परिमाण(पूर्णाङ्क) + pHdr->nTitle);
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
स्थिर शून्य *sqlite3MemRealloc(शून्य *p_प्राक्, पूर्णाङ्क nByte){
  संरचना Mem_खण्डम्_Hdr *pOldHdr;
  शून्य *pNew;
  प्रतिज्ञा( mem.disallow==0 );
  प्रतिज्ञा( (nByte & 7)==0 );     /* EV: R-46199-30249 */
  pOldHdr = sqlite3_Memsys_Get_शीर्षकम्(p_प्राक्);
  pNew = sqlite3MemMalloc(nByte);
  यदि( pNew ){
    स्मृतिं_प्रतिलिख(pNew, p_प्राक्, (पूर्णाङ्क)(nByte<pOldHdr->i_आकार ? nByte : pOldHdr->i_आकार));
    यदि( nByte>pOldHdr->i_आकार ){
      randomFill(&((वर्ण*)pNew)[pOldHdr->i_आकार], nByte - (पूर्णाङ्क)pOldHdr->i_आकार);
    }
    sqlite3MemFree(p_प्राक्);
  }
  प्रत्यागम pNew;
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
शून्य sqlite3_Mem_समुच्चयः_Default(शून्य){
  स्थिर ध्रुव sqlite3_mem_methods defaultMethods = {
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
शून्य sqlite3_Memdebug_समुच्चयः_Type(शून्य *p, u8 eType){
  यदि( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    संरचना Mem_खण्डम्_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_शीर्षकम्(p);
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
पूर्णाङ्क sqlite3MemdebugHasType(ध्रुव शून्य *p, u8 eType){
  पूर्णाङ्क rc = 1;
  यदि( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    संरचना Mem_खण्डम्_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_शीर्षकम्(p);
    प्रतिज्ञा( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    यदि( (pHdr->eType&eType)==0 ){
      rc = 0;
    }
  }
  प्रत्यागम rc;
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
पूर्णाङ्क sqlite3MemdebugNoType(ध्रुव शून्य *p, u8 eType){
  पूर्णाङ्क rc = 1;
  यदि( p && sqlite3GlobalConfig.m.xFree==sqlite3MemFree ){
    संरचना Mem_खण्डम्_Hdr *pHdr;
    pHdr = sqlite3_Memsys_Get_शीर्षकम्(p);
    प्रतिज्ञा( pHdr->iForeGuard==FOREGUARD );         /* Allocation is valid */
    यदि( (pHdr->eType&eType)!=0 ){
      rc = 0;
    }
  }
  प्रत्यागम rc;
}

/*
** Set the number of backtrace levels kept for each allocation.
** A value of zero turns off backtracing.  The number is always rounded
** up to a multiple of 2.
*/
शून्य sqlite3MemdebugBacktrace(पूर्णाङ्क गभीरता){
  यदि( गभीरता<0 ){ गभीरता = 0; }
  यदि( गभीरता>20 ){ गभीरता = 20; }
  गभीरता = (गभीरता+1)&0xfe;
  mem.nBacktrace = गभीरता;
}

शून्य sqlite3MemdebugBacktraceCallback(शून्य (*xBacktrace)(पूर्णाङ्क, पूर्णाङ्क, शून्य **)){
  mem.xBacktrace = xBacktrace;
}

/*
** Set the title string for subsequent allocations.
*/
शून्य sqlite3MemdebugSettitle(ध्रुव वर्ण *zTitle){
  अचिह्नित पूर्णाङ्क n = sqlite3Strlen30(zTitle) + 1;
  sqlite3_mutex_enter(mem.mutex);
  यदि( n>=परिमाण(mem.zTitle) ) n = परिमाण(mem.zTitle)-1;
  स्मृतिं_प्रतिलिख(mem.zTitle, zTitle, n);
  mem.zTitle[n] = 0;
  mem.nTitle = ROUND8(n);
  sqlite3_mutex_leave(mem.mutex);
}

शून्य sqlite3MemdebugSync(){
  संरचना Mem_खण्डम्_Hdr *pHdr;
  प्रति(pHdr=mem.pFirst; pHdr; pHdr=pHdr->p_अग्र){
    शून्य **pBt = (शून्य**)pHdr;
    pBt -= pHdr->nBacktraceSlots;
    mem.xBacktrace((पूर्णाङ्क)pHdr->i_आकार, pHdr->nBacktrace-1, &pBt[1]);
  }
}

/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
शून्य sqlite3MemdebugDump(ध्रुव वर्ण *zFilename){
  FILE *बहिर्गमनम्;
  संरचना Mem_खण्डम्_Hdr *pHdr;
  शून्य **pBt;
  पूर्णाङ्क i;
  बहिर्गमनम् = fopen(zFilename, "w");
  यदि( बहिर्गमनम्==0 ){
    fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                    zFilename);
    प्रत्यागम;
  }
  प्रति(pHdr=mem.pFirst; pHdr; pHdr=pHdr->p_अग्र){
    वर्ण *z = (वर्ण*)pHdr;
    z -= pHdr->nBacktraceSlots*परिमाण(शून्य*) + pHdr->nTitle;
    fprintf(बहिर्गमनम्, "**** %lld bytes at %p from %s ****\n", 
            pHdr->i_आकार, &pHdr[1], pHdr->nTitle ? z : "???");
    यदि( pHdr->nBacktrace ){
      fflush(बहिर्गमनम्);
      pBt = (शून्य**)pHdr;
      pBt -= pHdr->nBacktraceSlots;
      backtrace_symbols_fd(pBt, pHdr->nBacktrace, fileno(बहिर्गमनम्));
      fprintf(बहिर्गमनम्, "\n");
    }
  }
  fprintf(बहिर्गमनम्, "COUNTS:\n");
  प्रति(i=0; i<NCSIZE-1; i++){
    यदि( mem.nAlloc[i] ){
      fprintf(बहिर्गमनम्, "   %5d: %10d %10d %10d\n", 
            i*8, mem.nAlloc[i], mem.nCurrent[i], mem.mxCurrent[i]);
    }
  }
  यदि( mem.nAlloc[NCSIZE-1] ){
    fprintf(बहिर्गमनम्, "   %5d: %10d %10d %10d\n",
             NCSIZE*8-8, mem.nAlloc[NCSIZE-1],
             mem.nCurrent[NCSIZE-1], mem.mxCurrent[NCSIZE-1]);
  }
  fclose(बहिर्गमनम्);
}

/*
** Return the number of times sqlite3MemMalloc() has been called.
*/
पूर्णाङ्क sqlite3_Memdebug_Malloc_गणनम्(){
  पूर्णाङ्क i;
  पूर्णाङ्क n_योग = 0;
  प्रति(i=0; i<NCSIZE; i++){
    n_योग += mem.nAlloc[i];
  }
  प्रत्यागम n_योग;
}


#यदि_अन्तः /* SQLITE_MEMDEBUG */
