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
#समावेश "sqliteInt.h"

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/
#अगर_घोषित SQLITE_ENABLE_MEMSYS5

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/
प्रकार काष्ठा Mem5Link Mem5Link;
काष्ठा Mem5Link {
  पूर्णांक अग्र;       /* Index of next free chunk */
  पूर्णांक पूर्व;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem5.szAtom). Since
** mem5.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#घोषणा LOGMAX 30

/*
** Masks used for mem5.aCtrl[] elements.
*/
#घोषणा CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#घोषणा CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
अटल SQLITE_WSD काष्ठा Mem5Global {
  /*
  ** Memory available for allocation
  */
  पूर्णांक szAtom;      /* Smallest possible allocation in bytes */
  पूर्णांक nBlock;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Performance statistics
  */
  u64 nAlloc;         /* Total number of calls to malloc */
  u64 योग_Alloc;     /* Total of all malloc calls - includes internal frag */
  u64 योग_Excess;    /* Total internal fragmentation */
  u32 currentOut;     /* Current checkout, including internal fragmentation */
  u32 current_गिनती;   /* Current number of distinct checkouts */
  u32 maxOut;         /* Maximum instantaneous currentOut */
  u32 max_गिनती;       /* Maximum instantaneous currentCount */
  u32 maxRequest;     /* Largest allocation (exclusive of internal frag) */
#पूर्ण_अगर
  
  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size mem5.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  पूर्णांक aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} mem5;

/*
** Access the static variable through a macro for SQLITE_OMIT_WSD.
*/
#घोषणा mem5 GLOBAL(काष्ठा Mem5Global, mem5)

/*
** Assuming mem5.zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#घोषणा MEM5LINK(idx) ((Mem5Link *)(&mem5.zPool[(idx)*mem5.szAtom]))

/*
** Unlink the chunk at mem5.aPool[i] from list it is currently
** on.  It should be found on mem5.aiFreelist[iLogsize].
*/
अटल व्योम memsys5Unlink(पूर्णांक i, पूर्णांक iLogsize){
  पूर्णांक अग्र, पूर्व;
  निश्चित( i>=0 && i<mem5.nBlock );
  निश्चित( iLogsize>=0 && iLogsize<=LOGMAX );
  निश्चित( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  अग्र = MEM5LINK(i)->अग्र;
  पूर्व = MEM5LINK(i)->पूर्व;
  अगर( पूर्व<0 ){
    mem5.aiFreelist[iLogsize] = अग्र;
  }अन्यथा{
    MEM5LINK(पूर्व)->अग्र = अग्र;
  }
  अगर( अग्र>=0 ){
    MEM5LINK(अग्र)->पूर्व = पूर्व;
  }
}

/*
** Link the chunk at mem5.aPool[i] so that is on the iLogsize
** free list.
*/
अटल व्योम memsys5Link(पूर्णांक i, पूर्णांक iLogsize){
  पूर्णांक x;
  निश्चित( sqlite3_mutex_held(mem5.mutex) );
  निश्चित( i>=0 && i<mem5.nBlock );
  निश्चित( iLogsize>=0 && iLogsize<=LOGMAX );
  निश्चित( (mem5.aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->अग्र = mem5.aiFreelist[iLogsize];
  MEM5LINK(i)->पूर्व = -1;
  अगर( x>=0 ){
    निश्चित( x<mem5.nBlock );
    MEM5LINK(x)->पूर्व = i;
  }
  mem5.aiFreelist[iLogsize] = i;
}

/*
** Obtain or release the mutex needed to access global data structures.
*/
अटल व्योम memsys5Enter(व्योम){
  sqlite3_mutex_enter(mem5.mutex);
}
अटल व्योम memsys5Leave(व्योम){
  sqlite3_mutex_leave(mem5.mutex);
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
अटल पूर्णांक memsys5_आकार(व्योम *p){
  पूर्णांक i_आकार, i;
  निश्चित( p!=0 );
  i = (पूर्णांक)(((u8 *)p-mem5.zPool)/mem5.szAtom);
  निश्चित( i>=0 && i<mem5.nBlock );
  i_आकार = mem5.szAtom * (1 << (mem5.aCtrl[i]&CTRL_LOGSIZE));
  वापस i_आकार;
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
अटल व्योम *memsys5MallocUnsafe(पूर्णांक nByte){
  पूर्णांक i;           /* Index of a mem5.aPool[] slot */
  पूर्णांक iBin;        /* Index into mem5.aiFreelist[] */
  पूर्णांक iFullSz;     /* Size of allocation rounded up to power of 2 */
  पूर्णांक iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  /* nByte must be a positive */
  निश्चित( nByte>0 );

  /* No more than 1GiB per allocation */
  अगर( nByte > 0x40000000 ) वापस 0;

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Keep track of the maximum allocation request.  Even unfulfilled
  ** requests are counted */
  अगर( (u32)nByte>mem5.maxRequest ){
    mem5.maxRequest = nByte;
  }
#पूर्ण_अगर


  /* Round nByte up to the next valid power of two */
  क्रम(iFullSz=mem5.szAtom,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}

  /* Make sure mem5.aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  क्रम(iBin=iLogsize; iBin<=LOGMAX && mem5.aiFreelist[iBin]<0; iBin++){}
  अगर( iBin>LOGMAX ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes", nByte);
    वापस 0;
  }
  i = mem5.aiFreelist[iBin];
  memsys5Unlink(i, iBin);
  जबतक( iBin>iLogsize ){
    पूर्णांक new_आकार_2;

    iBin--;
    new_आकार_2 = 1 << iBin;
    mem5.aCtrl[i+new_आकार_2] = CTRL_FREE | iBin;
    memsys5Link(i+new_आकार_2, iBin);
  }
  mem5.aCtrl[i] = iLogsize;

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /* Update allocator performance statistics. */
  mem5.nAlloc++;
  mem5.योग_Alloc += iFullSz;
  mem5.योग_Excess += iFullSz - nByte;
  mem5.current_गिनती++;
  mem5.currentOut += iFullSz;
  अगर( mem5.max_गिनती<mem5.current_गिनती ) mem5.max_गिनती = mem5.current_गिनती;
  अगर( mem5.maxOut<mem5.currentOut ) mem5.maxOut = mem5.currentOut;
#पूर्ण_अगर

#अगर_घोषित SQLITE_DEBUG
  /* Make sure the allocated memory does not assume that it is set to zero
  ** or retains a value from a previous allocation */
  स_रखो(&mem5.zPool[i*mem5.szAtom], 0xAA, iFullSz);
#पूर्ण_अगर

  /* Return a pointer to the allocated memory. */
  वापस (व्योम*)&mem5.zPool[i*mem5.szAtom];
}

/*
** Free an outstanding memory allocation.
*/
अटल व्योम memsys5FreeUnsafe(व्योम *pOld){
  u32 आकार, iLogsize;
  पूर्णांक iBlock;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem5.szAtom byte blocks pointed to by mem5.zPool.
  */
  iBlock = (पूर्णांक)(((u8 *)pOld-mem5.zPool)/mem5.szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  निश्चित( iBlock>=0 && iBlock<mem5.nBlock );
  निश्चित( ((u8 *)pOld-mem5.zPool)%mem5.szAtom==0 );
  निश्चित( (mem5.aCtrl[iBlock] & CTRL_FREE)==0 );

  iLogsize = mem5.aCtrl[iBlock] & CTRL_LOGSIZE;
  आकार = 1<<iLogsize;
  निश्चित( iBlock+आकार-1<(u32)mem5.nBlock );

  mem5.aCtrl[iBlock] |= CTRL_FREE;
  mem5.aCtrl[iBlock+आकार-1] |= CTRL_FREE;

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  निश्चित( mem5.current_गिनती>0 );
  निश्चित( mem5.currentOut>=(आकार*mem5.szAtom) );
  mem5.current_गिनती--;
  mem5.currentOut -= आकार*mem5.szAtom;
  निश्चित( mem5.currentOut>0 || mem5.current_गिनती==0 );
  निश्चित( mem5.current_गिनती>0 || mem5.currentOut==0 );
#पूर्ण_अगर

  mem5.aCtrl[iBlock] = CTRL_FREE | iLogsize;
  जबतक( ALWAYS(iLogsize<LOGMAX) ){
    पूर्णांक iBuddy;
    अगर( (iBlock>>iLogsize) & 1 ){
      iBuddy = iBlock - आकार;
      निश्चित( iBuddy>=0 );
    }अन्यथा{
      iBuddy = iBlock + आकार;
      अगर( iBuddy>=mem5.nBlock ) अवरोध;
    }
    अगर( mem5.aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) अवरोध;
    memsys5Unlink(iBuddy, iLogsize);
    iLogsize++;
    अगर( iBuddy<iBlock ){
      mem5.aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBlock] = 0;
      iBlock = iBuddy;
    }अन्यथा{
      mem5.aCtrl[iBlock] = CTRL_FREE | iLogsize;
      mem5.aCtrl[iBuddy] = 0;
    }
    आकार *= 2;
  }

#अगर_घोषित SQLITE_DEBUG
  /* Overwrite freed memory with the 0x55 bit pattern to verify that it is
  ** not used after being freed */
  स_रखो(&mem5.zPool[iBlock*mem5.szAtom], 0x55, आकार);
#पूर्ण_अगर

  memsys5Link(iBlock, iLogsize);
}

/*
** Allocate nBytes of memory.
*/
अटल व्योम *memsys5Malloc(पूर्णांक nBytes){
  sqlite3_int64 *p = 0;
  अगर( nBytes>0 ){
    memsys5Enter();
    p = memsys5MallocUnsafe(nBytes);
    memsys5Leave();
  }
  वापस (व्योम*)p; 
}

/*
** Free memory.
**
** The outer layer memory allocator prevents this routine from
** being called with pPrior==0.
*/
अटल व्योम memsys5Free(व्योम *pPrior){
  निश्चित( pPrior!=0 );
  memsys5Enter();
  memsys5FreeUnsafe(pPrior);
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
अटल व्योम *memsys5Realloc(व्योम *pPrior, पूर्णांक nBytes){
  पूर्णांक nOld;
  व्योम *p;
  निश्चित( pPrior!=0 );
  निश्चित( (nBytes&(nBytes-1))==0 );  /* EV: R-46199-30249 */
  निश्चित( nBytes>=0 );
  अगर( nBytes==0 ){
    वापस 0;
  }
  nOld = memsys5_आकार(pPrior);
  अगर( nBytes<=nOld ){
    वापस pPrior;
  }
  p = memsys5Malloc(nBytes);
  अगर( p ){
    स_नकल(p, pPrior, nOld);
    memsys5Free(pPrior);
  }
  वापस p;
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
अटल पूर्णांक memsys5Roundup(पूर्णांक n){
  पूर्णांक iFullSz;
  अगर( n<=mem5.szAtom*2 ){
    अगर( n<=mem5.szAtom ) वापस mem5.szAtom;
    वापस mem5.szAtom*2;
  }
  अगर( n>0x10000000 ){
    अगर( n>0x40000000 ) वापस 0;
    अगर( n>0x20000000 ) वापस 0x40000000;
    वापस 0x20000000;
  }
  क्रम(iFullSz=mem5.szAtom*8; iFullSz<n; iFullSz *= 4);
  अगर( (iFullSz/2)>=(i64)n ) वापस iFullSz/2;
  वापस iFullSz;
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
अटल पूर्णांक memsys5Log(पूर्णांक i_मूल्य){
  पूर्णांक iLog;
  क्रम(iLog=0; (iLog<(पूर्णांक)((माप(पूर्णांक)*8)-1)) && (1<<iLog)<i_मूल्य; iLog++);
  वापस iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
अटल पूर्णांक memsys5Init(व्योम *NotUsed){
  पूर्णांक ii;            /* Loop counter */
  पूर्णांक nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  पूर्णांक nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  पूर्णांक iOffset;       /* An offset into mem5.aCtrl[] */

  UNUSED_PARAMETER(NotUsed);

  /* For the purposes of this routine, disable the mutex */
  mem5.mutex = 0;

  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  निश्चित( (माप(Mem5Link)&(माप(Mem5Link)-1))==0 );

  nByte = sqlite3GlobalConfig.nHeap;
  zByte = (u8*)sqlite3GlobalConfig.pHeap;
  निश्चित( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(sqlite3GlobalConfig.mnReq);
  mem5.szAtom = (1<<nMinLog);
  जबतक( (पूर्णांक)माप(Mem5Link)>mem5.szAtom ){
    mem5.szAtom = mem5.szAtom << 1;
  }

  mem5.nBlock = (nByte / (mem5.szAtom+माप(u8)));
  mem5.zPool = zByte;
  mem5.aCtrl = (u8 *)&mem5.zPool[mem5.nBlock*mem5.szAtom];

  क्रम(ii=0; ii<=LOGMAX; ii++){
    mem5.aiFreelist[ii] = -1;
  }

  iOffset = 0;
  क्रम(ii=LOGMAX; ii>=0; ii--){
    पूर्णांक nAlloc = (1<<ii);
    अगर( (iOffset+nAlloc)<=mem5.nBlock ){
      mem5.aCtrl[iOffset] = ii | CTRL_FREE;
      memsys5Link(iOffset, ii);
      iOffset += nAlloc;
    }
    निश्चित((iOffset+nAlloc)>mem5.nBlock);
  }

  /* If a mutex is required for normal operation, allocate one */
  अगर( sqlite3GlobalConfig.bMemstat==0 ){
    mem5.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }

  वापस SQLITE_OK;
}

/*
** Deinitialize this module.
*/
अटल व्योम memsys5Shutdown(व्योम *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem5.mutex = 0;
  वापस;
}

#अगर_घोषित SQLITE_TEST
/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
व्योम sqlite3Memsys5Dump(स्थिर अक्षर *zFilename){
  खाता *out;
  पूर्णांक i, j, n;
  पूर्णांक nMinLog;

  अगर( zFilename==0 || zFilename[0]==0 ){
    out = मानक_निकास;
  }अन्यथा{
    out = ख_खोलो(zFilename, "w");
    अगर( out==0 ){
      ख_लिखो(मानक_त्रुटि, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      वापस;
    }
  }
  memsys5Enter();
  nMinLog = memsys5Log(mem5.szAtom);
  क्रम(i=0; i<=LOGMAX && i+nMinLog<32; i++){
    क्रम(n=0, j=mem5.aiFreelist[i]; j>=0; j = MEM5LINK(j)->अग्र, n++){}
    ख_लिखो(out, "freelist items of size %d: %d\n", mem5.szAtom << i, n);
  }
  ख_लिखो(out, "mem5.nAlloc       = %llu\n", mem5.nAlloc);
  ख_लिखो(out, "mem5.totalAlloc   = %llu\n", mem5.योग_Alloc);
  ख_लिखो(out, "mem5.totalExcess  = %llu\n", mem5.योग_Excess);
  ख_लिखो(out, "mem5.currentOut   = %u\n", mem5.currentOut);
  ख_लिखो(out, "mem5.currentCount = %u\n", mem5.current_गिनती);
  ख_लिखो(out, "mem5.maxOut       = %u\n", mem5.maxOut);
  ख_लिखो(out, "mem5.maxCount     = %u\n", mem5.max_गिनती);
  ख_लिखो(out, "mem5.maxRequest   = %u\n", mem5.maxRequest);
  memsys5Leave();
  अगर( out==मानक_निकास ){
    ख_साफ(मानक_निकास);
  }अन्यथा{
    ख_बंद(out);
  }
}
#पूर्ण_अगर

/*
** This routine is the only routine in this file with external 
** linkage. It returns a pointer to a static sqlite3_mem_methods
** struct populated with the memsys5 methods.
*/
स्थिर sqlite3_mem_methods *sqlite3MemGetMemsys5(व्योम){
  अटल स्थिर sqlite3_mem_methods memsys5Methods = {
     memsys5Malloc,
     memsys5Free,
     memsys5Realloc,
     memsys5_आकार,
     memsys5Roundup,
     memsys5Init,
     memsys5Shutdown,
     0
  };
  वापस &memsys5Methods;
}

#पूर्ण_अगर /* SQLITE_ENABLE_MEMSYS5 */
