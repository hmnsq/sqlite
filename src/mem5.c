/*
** 2007 October 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement a memory
** allocation subsystem for use by SQLite. 
**
** This version of the memory allocation subsystem omits all
** use of malloc(). The application gives SQLite a block of memory
** before calling sqlite3_initialize() from which allocations
** are made and returned by the xMalloc() and xRealloc() 
** implementations. Once sqlite3_initialize() has been called,
** the amount of memory available to SQLite is fixed and cannot
** be changed.
**
** This version of the memory allocation subsystem is included
** in the build only if SQLITE_ENABLE_MEMSYS5 is defined.
**
** This memory allocator uses the following algorithm:
**
**   1.  All memory allocation sizes are rounded up to a power of 2.
**
**   2.  If two adjacent free blocks are the halves of a larger block,
**       then the two blocks are coalesced into the single larger block.
**
**   3.  New memory is allocated from the first available free block.
**
** This algorithm is described in: J. M. Robson. "Bounds for Some Functions
** Concerning Dynamic Storage Allocation". Journal of the Association for
** Computing Machinery, Volume 21, Number 8, July 1974, pages 491-499.
** 
** Let n be the size of the largest allocation divided by the minimum
** allocation size (after rounding all sizes up to a power of 2.)  Let M
** be the maximum amount of memory ever outstanding at one time.  Let
** N be the total amount of memory available for allocation.  Robson
** proved that this memory allocator will never breakdown due to 
** fragmentation as long as the following constraint holds:
**
**      N >=  M*(1 + log2(n)/2) - n + 1
**
** The sqlite3_status() logic tracks the maximum values of n and M so
** that an application can, at any time, verify this constraint.
*/
#अन्तर्भाव "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#यदि_परिभाषितम् SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
प्रकारनिर्देश संरचना Mem5Link Mem5Link;
संरचना Mem5Link {
  पूर्णाङ्क अग्र;       /* Index of next free chunk */
  पूर्णाङ्क पूर्व;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#परिभाषय LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#परिभाषय CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#परिभाषय CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
स्थिर SQLITE_WSD संरचना Mem5Global {
  /*
  ** Memory available for allocation
  */
  पूर्णाङ्क szAtom;      /* Smallest possible allocation in bytes */
  पूर्णाङ्क n_खण्डम्;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 योग_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 योग_Excess;    /* Total internal fragmentation */
  u32 current_बहिर्गमनम्;     /* Current checkout, including internal fragmentation */
  u32 current_गणनम्;   /* Current number of distinct checkouts */
  u32 max_बहिर्गमनम्;         /* Maximum instantaneous currentOut */
  u32 max_गणनम्;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#यदि_अन्तः
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  पूर्णाङ्क aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#परिभाषय mem5 GLOBAL(संरचना Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#परिभाषय MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
स्थिर शून्य memsys5Unlink(पूर्णाङ्क i, पूर्णाङ्क iLogsize){
  पूर्णाङ्क अग्र, पूर्व;
  प्रतिज्ञा( i>=0 && i<mem5.n_खण्डम् );
  प्रतिज्ञा( iLogsize>=0 && iLogsize<=LOGMAX );
  प्रतिज्ञा( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  अग्र = MEM5LINK(i)->अग्र;
  पूर्व = MEM5LINK(i)->पूर्व;
  यदि( पूर्व<0 ){
    mem5.aiFreelist[iLogsize] = अग्र;
  }अन्यथा{
    MEM5LINK(पूर्व)->अग्र = अग्र;
  }
  यदि( अग्र>=0 ){
    MEM5LINK(अग्र)->पूर्व = पूर्व;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
स्थिर शून्य memsys5Link(पूर्णाङ्क i, पूर्णाङ्क iLogsize){
  पूर्णाङ्क x;
  प्रतिज्ञा( sqlite3_mutex_held(mem5.mutex) );
  प्रतिज्ञा( i>=0 && i<mem5.n_खण्डम् );
  प्रतिज्ञा( iLogsize>=0 && iLogsize<=LOGMAX );
  प्रतिज्ञा( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->अग्र = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->पूर्व = -1;
  यदि( x>=0 ){
    प्रतिज्ञा( x<mem5.n_खण्डम् );
    MEM5LINK(x)->पूर्व = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
स्थिर शून्य memsys5Enter(शून्य){
  sqlite3_mutex_enter(mem5.mutex);
}
स्थिर शून्य memsys5Leave(शून्य){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
स्थिर पूर्णाङ्क memsys5_आकार(शून्य *p){
  पूर्णाङ्क i_आकार, i;
  प्रतिज्ञा( p!=0 );
  i = (पूर्णाङ्क)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  प्रतिज्ञा( i>=0 && i<mem5.n_खण्डम् );
  i_आकार = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  प्रत्यागम i_आकार;
}

/*
** Return a block of memory of at least nBytes in size.
** Return NULL if unable.  Return NULL if nBytes==0.
**
** The caller guarantees that nByte is positive.
**
** The caller has obtained a mutex prior to invoking this
** routine so there is never any chance that two or more
** threads can be in this routine at the same time.
*/
स्थिर शून्य *memsys5MallocUnsafe(पूर्णाङ्क nByte){
  पूर्णाङ्क i;           /* Index of a mem5.aPool[] slot */
  पूर्णाङ्क iBin;        /* Index into mem5.aiFreelist[] */
  पूर्णाङ्क i_पूर्णम्_Sz;     /* Size of allocation rounded up to power of 2 */
  पूर्णाङ्क iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  प्रतिज्ञा( nByte>0 );

  /* No more than 1GiB per allocation */
  यदि( nByte > 0x40000000 ) प्रत्यागम 0;

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  यदि( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#यदि_अन्तः


  /* Round nByte up to the next valid power of two */
  प्रति(i_पूर्णम्_Sz=mem5.szAtom,iLogsize=0; i_पूर्णम्_Sz<nByte; i_पूर्णम्_Sz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  प्रति(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  यदि( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    प्रत्यागम 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  यावत्( iBin>iLogsize ){
    पूर्णाङ्क new_आकार_2;

    iBin--;
    new_आकार_2 = 1 << iBin;
    mem5.aCtrl[i+new_आकार_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_आकार_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.योग_Alloc += i_पूर्णम्_Sz;
  mem5.योग_Excess += i_पूर्णम्_Sz - nByte;
  mem5.current_गणनम्++;
  mem5.current_बहिर्गमनम् += i_पूर्णम्_Sz;
  यदि( mem5.max_गणनम्<mem5.current_गणनम् ) mem5.max_गणनम् = mem5.current_गणनम्;
  यदि( mem5.max_बहिर्गमनम्<mem5.current_बहिर्गमनम् ) mem5.max_बहिर्गमनम् = mem5.current_बहिर्गमनम्;
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  स्मृतिं_पूरय(&mem5.zPool[i*mem5.szAtom], 0xAA, i_पूर्णम्_Sz);
#यदि_अन्तः

  /* Return a pointer to the allocated memory. */
  प्रत्यागम (शून्य*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
स्थिर शून्य memsys5FreeUnsafe(शून्य *pOld){
  u32 आकार, iLogsize;
  पूर्णाङ्क i_खण्डम्;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  i_खण्डम् = (पूर्णाङ्क)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  प्रतिज्ञा( i_खण्डम्>=0 && i_खण्डम्<mem5.n_खण्डम् );
  प्रतिज्ञा( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  प्रतिज्ञा( (mem5.aCtrl[i_खण्डम्] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[i_खण्डम्] & CTRL_LOGSIZE;
  आकार = 1<<iLogsize;
  प्रतिज्ञा( i_खण्डम्+आकार-1<(u32)mem5.n_खण्डम् );

  mem5.aCtrl[i_खण्डम्] |= CTRL_FREE;
  mem5.aCtrl[i_खण्डम्+आकार-1] |= CTRL_FREE;

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  प्रतिज्ञा( mem5.current_गणनम्>0 );
  प्रतिज्ञा( mem5.current_बहिर्गमनम्>=(आकार*mem5.szAtom) );
  mem5.current_गणनम्--;
  mem5.current_बहिर्गमनम् -= आकार*mem5.szAtom;
  प्रतिज्ञा( mem5.current_बहिर्गमनम्>0 || mem5.current_गणनम्==0 );
  प्रतिज्ञा( mem5.current_गणनम्>0 || mem5.current_बहिर्गमनम्==0 );
#यदि_अन्तः

  mem5.aCtrl[i_खण्डम्] = CTRL_FREE | iLogsize;
  यावत्( ALWAYS(iLogsize<LOGMAX) ){
    पूर्णाङ्क iBuddy;
    यदि( (i_खण्डम्>>iLogsize) & 1 ){
      iBuddy = i_खण्डम् - आकार;
      प्रतिज्ञा( iBuddy>=0 );
    }अन्यथा{
      iBuddy = i_खण्डम् + आकार;
      यदि( iBuddy>=mem5.n_खण्डम् ) भङ्ग;
    }
    यदि( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) भङ्ग;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    यदि( iBuddy<i_खण्डम् ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[i_खण्डम्] = 0;
      i_खण्डम् = iBuddy;
    }अन्यथा{
      mem5.aCtrl[i_खण्डम्] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    आकार *= 2;
  }

#यदि_परिभाषितम् SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  स्मृतिं_पूरय(&mem5.zPool[i_खण्डम्*mem5.szAtom], 0x55, आकार);
#यदि_अन्तः

  memsys5Link(i_खण्डम्, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
स्थिर शून्य *memsys5Malloc(पूर्णाङ्क n_अष्टकानि){
  sqlite3_int64 *p = 0;
  यदि( n_अष्टकानि>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(n_अष्टकानि);
    memsys5Leave();
  }
  प्रत्यागम (शून्य*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
स्थिर शून्य memsys5Free(शून्य *p_प्राक्){
  प्रतिज्ञा( p_प्राक्!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(p_प्राक्);
  memsys5Leave();  
}

/*
** Change the size of an existing memory allocation.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.  
**
** nBytes is always a value obtained from a prior call to
** memsys5Round().  Hence nBytes is always a non-negative power
** of two.  If nBytes==0 that means that an oversize allocation
** (an allocation larger than 0x40000000) was requested and this
** routine should return 0 without freeing pPrior.
*/
स्थिर शून्य *memsys5Realloc(शून्य *p_प्राक्, पूर्णाङ्क n_अष्टकानि){
  पूर्णाङ्क nOld;
  शून्य *p;
  प्रतिज्ञा( p_प्राक्!=0 );
  प्रतिज्ञा( (n_अष्टकानि&(n_अष्टकानि-1))==0 );  /* EV: R-46199-30249 */
  प्रतिज्ञा( n_अष्टकानि>=0 );
  यदि( n_अष्टकानि==0 ){
    प्रत्यागम 0;
  }
  nOld = memsys5_आकार(p_प्राक्);
  यदि( n_अष्टकानि<=nOld ){
    प्रत्यागम p_प्राक्;
  }
  p = memsys5Malloc(n_अष्टकानि);
  यदि( p ){
    स्मृतिं_प्रतिलिख(p, p_प्राक्, nOld);
    memsys5Free(p_प्राक्);
  }
  प्रत्यागम p;
}

/*
** Round up a request size to the next valid allocation size.  If
** the allocation is too large to be handled by this allocation system,
** return 0.
**
** All allocations must be a power of two and must be expressed by a
** 32-bit signed integer.  Hence the largest allocation is 0x40000000
** or 1073741824 bytes.
*/
स्थिर पूर्णाङ्क memsys5Roundup(पूर्णाङ्क n){
  पूर्णाङ्क i_पूर्णम्_Sz;
  यदि( n<=mem5.szAtom*2 ){
    यदि( n<=mem5.szAtom ) प्रत्यागम mem5.szAtom;
    प्रत्यागम mem5.szAtom*2;
  }
  यदि( n>0x10000000 ){
    यदि( n>0x40000000 ) प्रत्यागम 0;
    यदि( n>0x20000000 ) प्रत्यागम 0x40000000;
    प्रत्यागम 0x20000000;
  }
  प्रति(i_पूर्णम्_Sz=mem5.szAtom*8; i_पूर्णम्_Sz<n; i_पूर्णम्_Sz *= 4);
  यदि( (i_पूर्णम्_Sz/2)>=(i64)n ) प्रत्यागम i_पूर्णम्_Sz/2;
  प्रत्यागम i_पूर्णम्_Sz;
}

/*
** Return the ceiling of the logarithm base 2 of iValue.
**
** Examples:   memsys5Log(1) -> 0
**             memsys5Log(2) -> 1
**             memsys5Log(4) -> 2
**             memsys5Log(5) -> 3
**             memsys5Log(8) -> 3
**             memsys5Log(9) -> 4
*/
स्थिर पूर्णाङ्क memsys5Log(पूर्णाङ्क i_मानम्){
  पूर्णाङ्क iLog;
  प्रति(iLog=0; (iLog<(पूर्णाङ्क)((परिमाण(पूर्णाङ्क)*8)-1)) && (1<<iLog)<i_मानम्; iLog++);
  प्रत्यागम iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
स्थिर पूर्णाङ्क memsys5Init(शून्य *Not_प्रयुक्तम्){
  पूर्णाङ्क ii;            /* Loop counter */
  पूर्णाङ्क nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  पूर्णाङ्क nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  पूर्णाङ्क i_विस्थापनम्;       /* An offset into mem5.aCtrl[] */

  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  प्रतिज्ञा( (परिमाण(Mem5Link)&(परिमाण(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.nHeap;
  zByte = (u8*)sqlite3GlobalConfig.pHeap;
  प्रतिज्ञा( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  यावत्( (पूर्णाङ्क)परिमाण(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.n_खण्डम् = (nByte / (mem5.szAtom+परिमाण(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.n_खण्डम्*mem5.szAtom];

  प्रति(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  i_विस्थापनम् = 0;
  प्रति(ii=LOGMAX; ii>=0; ii--){
    पूर्णाङ्क nAlloc = (1<<ii);
    यदि( (i_विस्थापनम्+nAlloc)<=mem5.n_खण्डम् ){
      mem5.aCtrl[i_विस्थापनम्] = ii | CTRL_FREE;
      memsys5Link(i_विस्थापनम्, ii);
      i_विस्थापनम् += nAlloc;
    }
    प्रतिज्ञा((i_विस्थापनम्+nAlloc)>mem5.n_खण्डम्);
  }

  /* If a mutex is required for normal operation, allocate one */
  यदि( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  प्रत्यागम SQLITE_OK;
}

/*
** Deinitialize this module.
*/
स्थिर शून्य memsys5Shutdown(शून्य *Not_प्रयुक्तम्){
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  mem5.mutex = 0;
  प्रत्यागम;
}

#यदि_परिभाषितम् SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
शून्य sqlite3Memsys5Dump(ध्रुव वर्ण *zFilename){
  FILE *बहिर्गमनम्;
  पूर्णाङ्क i, j, n;
  पूर्णाङ्क nMinLog;

  यदि( zFilename==0 || zFilename[0]==0 ){
    बहिर्गमनम् = stdout;
  }अन्यथा{
    बहिर्गमनम् = fopen(zFilename, "w");
    यदि( बहिर्गमनम्==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      प्रत्यागम;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  प्रति(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    प्रति(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->अग्र, n++){}
    fprintf(बहिर्गमनम्, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  fprintf(बहिर्गमनम्, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  fprintf(बहिर्गमनम्, "mem5.totalAlloc   = %llu\n", mem5.योग_Alloc);
  fprintf(बहिर्गमनम्, "mem5.totalExcess  = %llu\n", mem5.योग_Excess);
  fprintf(बहिर्गमनम्, "mem5.currentOut   = %u\n", mem5.current_बहिर्गमनम्);
  fprintf(बहिर्गमनम्, "mem5.currentCount = %u\n", mem5.current_गणनम्);
  fprintf(बहिर्गमनम्, "mem5.maxOut       = %u\n", mem5.max_बहिर्गमनम्);
  fprintf(बहिर्गमनम्, "mem5.maxCount     = %u\n", mem5.max_गणनम्);
  fprintf(बहिर्गमनम्, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  यदि( बहिर्गमनम्==stdout ){
    fflush(stdout);
  }अन्यथा{
    fclose(बहिर्गमनम्);
  }
}
#यदि_अन्तः

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
ध्रुव sqlite3_mem_methods *sqlite3MemGetMemsys5(शून्य){
  स्थिर ध्रुव sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_आकार,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  प्रत्यागम &memsys5Methods;
}

#यदि_अन्तः /* SQLITE_ENABLE_MEMSYS5 */
