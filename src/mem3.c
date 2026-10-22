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
** use of malloc(). The SQLite user supplies a block of memory
** before calling sqlite3_initialize() from which allocations
** are made and returned by the xMalloc() and xRealloc() 
** implementations. Once sqlite3_initialize() has been called,
** the amount of memory available to SQLite is fixed and cannot
** be changed.
**
** This version of the memory allocation subsystem is included
** in the build only if SQLITE_ENABLE_MEMSYS3 is defined.
*/
#অন্তর্ভাব "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#ifdef SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#define MX_SMALL 10


/*
** Number of freelist hash slots
*/
#define N_HASH  61

/*
** A memory allocation (also called a "chunk") consists of two or 
** more blocks where each block is 8 bytes.  The first 8 bytes are 
** a header that is not returned to the user.
**
** A chunk is two or more blocks that is either checked out or
** free.  The first block has format u.hdr.  u.hdr.size4x is 4 times the
** size of the allocation in blocks if the allocation is free.
** The u.hdr.size4x&1 bit is true if the chunk is checked out and
** false if the chunk is on the freelist.  The u.hdr.size4x&2 bit
** is true if the previous chunk is checked out and false if the
** previous chunk is free.  The u.hdr.prevSize field is the size of
** the previous chunk in blocks if the previous chunk is on the
** freelist. If the previous chunk is checked out, then
** u.hdr.prevSize can be part of the data for that chunk and should
** not be read or written.
**
** We often identify a chunk by its index in mem3.aPool[].  When
** this is done, the chunk index refers to the second block of
** the chunk.  In this way, the first chunk has an index of 1.
** A chunk index of 0 means "no such chunk" and is the equivalent
** of a NULL pointer.
**
** The second block of free chunks is of the form u.list.  The
** two fields form a double-linked list of chunks of related sizes.
** Pointers to the head of the list are stored in mem3.aiSmall[] 
** for smaller chunks and mem3.aiHash[] for larger chunks.
**
** The second block of a chunk is user data if the chunk is checked 
** out.  If a chunk is checked out, the user data may extend into
** the u.hdr.prevSize value of the following chunk.
*/
প্রকারলেখ কাঠামো Mem3Block Mem3Block;
কাঠামো Mem3Block {
  সংঘ {
    কাঠামো {
      u32 পূর্ব_আকার;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    কাঠামো {
      u32 পরবর্তী;       /* Index in mem3.aPool[] of next free chunk */
      u32 পূর্ব;       /* Index in mem3.aPool[] of previous free chunk */
    } list;
  } u;
};

/*
** All of the static variables used by this module are collected
** into a single structure named "mem3".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
স্থির SQLITE_WSD কাঠামো Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3Block *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  পূর্ণসংখ্যা alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_চাবি_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_চাবি_Blk;
  u32 sz_চাবি_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#define mem3 GLOBAL(কাঠামো Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
স্থির শূন্য memsys3UnlinkFromList(u32 i, u32 *p_মূল){
  u32 পরবর্তী = mem3.aPool[i].u.list.পরবর্তী;
  u32 পূর্ব = mem3.aPool[i].u.list.পূর্ব;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  যদি( পূর্ব==0 ){
    *p_মূল = পরবর্তী;
  }অন্যথা{
    mem3.aPool[পূর্ব].u.list.পরবর্তী = পরবর্তী;
  }
  যদি( পরবর্তী ){
    mem3.aPool[পরবর্তী].u.list.পূর্ব = পূর্ব;
  }
  mem3.aPool[i].u.list.পরবর্তী = 0;
  mem3.aPool[i].u.list.পূর্ব = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
স্থির শূন্য memsys3Unlink(u32 i){
  u32 আকার_2, hash;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  প্রতিজ্ঞা( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  প্রতিজ্ঞা( i>=1 );
  আকার_2 = mem3.aPool[i-1].u.hdr.size4x/4;
  প্রতিজ্ঞা( আকার_2==mem3.aPool[i+আকার_2-1].u.hdr.পূর্ব_আকার );
  প্রতিজ্ঞা( আকার_2>=2 );
  যদি( আকার_2 <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[আকার_2-2]);
  }অন্যথা{
    hash = আকার_2 % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
স্থির শূন্য memsys3LinkIntoList(u32 i, u32 *p_মূল){
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.পরবর্তী = *p_মূল;
  mem3.aPool[i].u.list.পূর্ব = 0;
  যদি( *p_মূল ){
    mem3.aPool[*p_মূল].u.list.পূর্ব = i;
  }
  *p_মূল = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
স্থির শূন্য memsys3Link(u32 i){
  u32 আকার_2, hash;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  প্রতিজ্ঞা( i>=1 );
  প্রতিজ্ঞা( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  আকার_2 = mem3.aPool[i-1].u.hdr.size4x/4;
  প্রতিজ্ঞা( আকার_2==mem3.aPool[i+আকার_2-1].u.hdr.পূর্ব_আকার );
  প্রতিজ্ঞা( আকার_2>=2 );
  যদি( আকার_2 <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[আকার_2-2]);
  }অন্যথা{
    hash = আকার_2 % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
স্থির শূন্য memsys3Enter(শূন্য){
  যদি( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
স্থির শূন্য memsys3Leave(শূন্য){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
স্থির শূন্য memsys3OutOfMemory(পূর্ণসংখ্যা nByte){
  যদি( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
    sqlite3_mutex_leave(mem3.mutex);
    sqlite3_release_memory(nByte);
    sqlite3_mutex_enter(mem3.mutex);
    mem3.alarmBusy = 0;
  }
}


/*
** Chunk i is a free chunk that has been unlinked.  Adjust its 
** size parameters for check-out and return a pointer to the 
** user portion of the chunk.
*/
স্থির শূন্য *memsys3Checkout(u32 i, u32 nBlock){
  u32 x;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  প্রতিজ্ঞা( i>=1 );
  প্রতিজ্ঞা( mem3.aPool[i-1].u.hdr.size4x/4==nBlock );
  প্রতিজ্ঞা( mem3.aPool[i+nBlock-1].u.hdr.পূর্ব_আকার==nBlock );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = nBlock*4 | 1 | (x&2);
  mem3.aPool[i+nBlock-1].u.hdr.পূর্ব_আকার = nBlock;
  mem3.aPool[i+nBlock-1].u.hdr.size4x |= 2;
  ফেরত &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
স্থির শূন্য *memsys3_From_চাবি_Blk(u32 nBlock){
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  প্রতিজ্ঞা( mem3.sz_চাবি_Blk>=nBlock );
  যদি( nBlock>=mem3.sz_চাবি_Blk-1 ){
    /* Use the entire key chunk */
    শূন্য *p = memsys3Checkout(mem3.i_চাবি_Blk, mem3.sz_চাবি_Blk);
    mem3.i_চাবি_Blk = 0;
    mem3.sz_চাবি_Blk = 0;
    mem3.mn_চাবি_Blk = 0;
    ফেরত p;
  }অন্যথা{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_চাবি_Blk + mem3.sz_চাবি_Blk - nBlock;
    প্রতিজ্ঞা( newi > mem3.i_চাবি_Blk+1 );
    mem3.aPool[mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk-1].u.hdr.পূর্ব_আকার = nBlock;
    mem3.aPool[mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = nBlock*4 + 1;
    mem3.sz_চাবি_Blk -= nBlock;
    mem3.aPool[newi-1].u.hdr.পূর্ব_আকার = mem3.sz_চাবি_Blk;
    x = mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x = mem3.sz_চাবি_Blk*4 | x;
    যদি( mem3.sz_চাবি_Blk < mem3.mn_চাবি_Blk ){
      mem3.mn_চাবি_Blk = mem3.sz_চাবি_Blk;
    }
    ফেরত (শূন্য*)&mem3.aPool[newi];
  }
}

/*
** *pRoot is the head of a list of free chunks of the same size
** or same size hash.  In other words, *pRoot is an entry in either
** mem3.aiSmall[] or mem3.aiHash[].  
**
** This routine examines all entries on the given list and tries
** to coalesce each entries with adjacent free chunks.  
**
** If it sees a chunk that is larger than mem3.iKeyBlk, it replaces 
** the current mem3.iKeyBlk with the new larger chunk.  In order for
** this mem3.iKeyBlk replacement to work, the key chunk must be
** linked into the hash tables.  That is not the normal state of
** affairs, of course.  The calling routine must link the key
** chunk before invoking this routine, then must unlink the (possibly
** changed) key chunk once this routine has finished.
*/
স্থির শূন্য memsys3Merge(u32 *p_মূল){
  u32 i_পরবর্তী, পূর্ব, আকার_2, i, x;

  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  জন্য(i=*p_মূল; i>0; i=i_পরবর্তী){
    i_পরবর্তী = mem3.aPool[i].u.list.পরবর্তী;
    আকার_2 = mem3.aPool[i-1].u.hdr.size4x;
    প্রতিজ্ঞা( (আকার_2&1)==0 );
    যদি( (আকার_2&2)==0 ){
      memsys3UnlinkFromList(i, p_মূল);
      প্রতিজ্ঞা( i > mem3.aPool[i-1].u.hdr.পূর্ব_আকার );
      পূর্ব = i - mem3.aPool[i-1].u.hdr.পূর্ব_আকার;
      যদি( পূর্ব==i_পরবর্তী ){
        i_পরবর্তী = mem3.aPool[পূর্ব].u.list.পরবর্তী;
      }
      memsys3Unlink(পূর্ব);
      আকার_2 = i + আকার_2/4 - পূর্ব;
      x = mem3.aPool[পূর্ব-1].u.hdr.size4x & 2;
      mem3.aPool[পূর্ব-1].u.hdr.size4x = আকার_2*4 | x;
      mem3.aPool[পূর্ব+আকার_2-1].u.hdr.পূর্ব_আকার = আকার_2;
      memsys3Link(পূর্ব);
      i = পূর্ব;
    }অন্যথা{
      আকার_2 /= 4;
    }
    যদি( আকার_2>mem3.sz_চাবি_Blk ){
      mem3.i_চাবি_Blk = i;
      mem3.sz_চাবি_Blk = আকার_2;
    }
  }
}

/*
** Return a block of memory of at least nBytes in size.
** Return NULL if unable.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
স্থির শূন্য *memsys3MallocUnsafe(পূর্ণসংখ্যা nByte){
  u32 i;
  u32 nBlock;
  u32 toFree;

  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  প্রতিজ্ঞা( আকার(Mem3Block)==8 );
  যদি( nByte<=12 ){
    nBlock = 2;
  }অন্যথা{
    nBlock = (nByte + 11)/8;
  }
  প্রতিজ্ঞা( nBlock>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  যদি( nBlock <= MX_SMALL ){
    i = mem3.aiSmall[nBlock-2];
    যদি( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[nBlock-2]);
      ফেরত memsys3Checkout(i, nBlock);
    }
  }অন্যথা{
    পূর্ণসংখ্যা hash = nBlock % N_HASH;
    জন্য(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.পরবর্তী){
      যদি( mem3.aPool[i-1].u.hdr.size4x/4==nBlock ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        ফেরত memsys3Checkout(i, nBlock);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  যদি( mem3.sz_চাবি_Blk>=nBlock ){
    ফেরত memsys3_From_চাবি_Blk(nBlock);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  জন্য(toFree=nBlock*16; toFree<(mem3.nPool*16); toFree *= 2){
    memsys3OutOfMemory(toFree);
    যদি( mem3.i_চাবি_Blk ){
      memsys3Link(mem3.i_চাবি_Blk);
      mem3.i_চাবি_Blk = 0;
      mem3.sz_চাবি_Blk = 0;
    }
    জন্য(i=0; i<N_HASH; i++){
      memsys3Merge(&mem3.aiHash[i]);
    }
    জন্য(i=0; i<MX_SMALL-1; i++){
      memsys3Merge(&mem3.aiSmall[i]);
    }
    যদি( mem3.sz_চাবি_Blk ){
      memsys3Unlink(mem3.i_চাবি_Blk);
      যদি( mem3.sz_চাবি_Blk>=nBlock ){
        ফেরত memsys3_From_চাবি_Blk(nBlock);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  ফেরত 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
স্থির শূন্য memsys3FreeUnsafe(শূন্য *pOld){
  Mem3Block *p = (Mem3Block*)pOld;
  পূর্ণসংখ্যা i;
  u32 আকার_2, x;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem3.mutex) );
  প্রতিজ্ঞা( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  প্রতিজ্ঞা( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  আকার_2 = mem3.aPool[i-1].u.hdr.size4x/4;
  প্রতিজ্ঞা( i+আকার_2<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+আকার_2-1].u.hdr.পূর্ব_আকার = আকার_2;
  mem3.aPool[i+আকার_2-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  যদি( mem3.i_চাবি_Blk ){
    যখন( (mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x&2)==0 ){
      আকার_2 = mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.পূর্ব_আকার;
      mem3.i_চাবি_Blk -= আকার_2;
      mem3.sz_চাবি_Blk += আকার_2;
      memsys3Unlink(mem3.i_চাবি_Blk);
      x = mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x = mem3.sz_চাবি_Blk*4 | x;
      mem3.aPool[mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk-1].u.hdr.পূর্ব_আকার = mem3.sz_চাবি_Blk;
    }
    x = mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x & 2;
    যখন( (mem3.aPool[mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk);
      mem3.sz_চাবি_Blk += mem3.aPool[mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_চাবি_Blk-1].u.hdr.size4x = mem3.sz_চাবি_Blk*4 | x;
      mem3.aPool[mem3.i_চাবি_Blk+mem3.sz_চাবি_Blk-1].u.hdr.পূর্ব_আকার = mem3.sz_চাবি_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
স্থির পূর্ণসংখ্যা memsys3_আকার(শূন্য *p){
  Mem3Block *pBlock;
  প্রতিজ্ঞা( p!=0 );
  pBlock = (Mem3Block*)p;
  প্রতিজ্ঞা( (pBlock[-1].u.hdr.size4x&1)!=0 );
  ফেরত (pBlock[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
স্থির পূর্ণসংখ্যা memsys3Roundup(পূর্ণসংখ্যা n){
  যদি( n<=12 ){
    ফেরত 12;
  }অন্যথা{
    ফেরত ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
স্থির শূন্য *memsys3Malloc(পূর্ণসংখ্যা nBytes){
  sqlite3_int64 *p;
  প্রতিজ্ঞা( nBytes>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  memsys3Leave();
  ফেরত (শূন্য*)p; 
}

/*
** Free memory.
*/
স্থির শূন্য memsys3Free(শূন্য *pPrior){
  প্রতিজ্ঞা( pPrior );
  memsys3Enter();
  memsys3FreeUnsafe(pPrior);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
স্থির শূন্য *memsys3Realloc(শূন্য *pPrior, পূর্ণসংখ্যা nBytes){
  পূর্ণসংখ্যা nOld;
  শূন্য *p;
  যদি( pPrior==0 ){
    ফেরত sqlite3_malloc(nBytes);
  }
  যদি( nBytes<=0 ){
    sqlite3_free(pPrior);
    ফেরত 0;
  }
  nOld = memsys3_আকার(pPrior);
  যদি( nBytes<=nOld && nBytes>=nOld-128 ){
    ফেরত pPrior;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  যদি( p ){
    যদি( nOld<nBytes ){
      memcpy(p, pPrior, nOld);
    }অন্যথা{
      memcpy(p, pPrior, nBytes);
    }
    memsys3FreeUnsafe(pPrior);
  }
  memsys3Leave();
  ফেরত p;
}

/*
** Initialize this module.
*/
স্থির পূর্ণসংখ্যা memsys3Init(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  যদি( !sqlite3GlobalConfig.pHeap ){
    ফেরত SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  প্রতিজ্ঞা( আকার(Mem3Block)==8 );
  mem3.aPool = (Mem3Block *)sqlite3GlobalConfig.pHeap;
  mem3.nPool = (sqlite3GlobalConfig.nHeap / আকার(Mem3Block)) - 2;

  /* Initialize the key block. */
  mem3.sz_চাবি_Blk = mem3.nPool;
  mem3.mn_চাবি_Blk = mem3.sz_চাবি_Blk;
  mem3.i_চাবি_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_চাবি_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.পূর্ব_আকার = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  ফেরত SQLITE_OK;
}

/*
** Deinitialize this module.
*/
স্থির শূন্য memsys3Shutdown(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem3.mutex = 0;
  ফেরত;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
শূন্য sqlite3Memsys3Dump(ধ্রুবক অক্ষর *zFilename){
#ifdef SQLITE_DEBUG
  FILE *out;
  u32 i, j;
  u32 আকার_2;
  যদি( zFilename==0 || zFilename[0]==0 ){
    out = stdout;
  }অন্যথা{
    out = fopen(zFilename, "w");
    যদি( out==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      ফেরত;
    }
  }
  memsys3Enter();
  fprintf(out, "CHUNKS:\n");
  জন্য(i=1; i<=mem3.nPool; i+=আকার_2/4){
    আকার_2 = mem3.aPool[i-1].u.hdr.size4x;
    যদি( আকার_2/4<=1 ){
      fprintf(out, "%p size error\n", &mem3.aPool[i]);
      প্রতিজ্ঞা( 0 );
      বিরতি;
    }
    যদি( (আকার_2&1)==0 && mem3.aPool[i+আকার_2/4-1].u.hdr.পূর্ব_আকার!=আকার_2/4 ){
      fprintf(out, "%p tail size does not match\n", &mem3.aPool[i]);
      প্রতিজ্ঞা( 0 );
      বিরতি;
    }
    যদি( ((mem3.aPool[i+আকার_2/4-1].u.hdr.size4x&2)>>1)!=(আকার_2&1) ){
      fprintf(out, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      প্রতিজ্ঞা( 0 );
      বিরতি;
    }
    যদি( আকার_2&1 ){
      fprintf(out, "%p %6d bytes checked out\n", &mem3.aPool[i], (আকার_2/4)*8-8);
    }অন্যথা{
      fprintf(out, "%p %6d bytes free%s\n", &mem3.aPool[i], (আকার_2/4)*8-8,
                  i==mem3.i_চাবি_Blk ? " **key**" : "");
    }
  }
  জন্য(i=0; i<MX_SMALL-1; i++){
    যদি( mem3.aiSmall[i]==0 ) চালু;
    fprintf(out, "small(%2d):", i);
    জন্য(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.পরবর্তী){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  জন্য(i=0; i<N_HASH; i++){
    যদি( mem3.aiHash[i]==0 ) চালু;
    fprintf(out, "hash(%2d):", i);
    জন্য(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.পরবর্তী){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  fprintf(out, "key=%d\n", mem3.i_চাবি_Blk);
  fprintf(out, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_চাবি_Blk*8);
  fprintf(out, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_চাবি_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  যদি( out==stdout ){
    fflush(stdout);
  }অন্যথা{
    fclose(out);
  }
#অন্যথা
  UNUSED_PARAMETER(zFilename);
#endif
}

/*
** This routine is the only routine in this file with external 
** linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file. The
** arguments specify the block of memory to manage.
**
** This routine is only called by sqlite3_config(), and therefore
** is not required to be threadsafe (it is not).
*/
ধ্রুবক sqlite3_mem_methods *sqlite3MemGetMemsys3(শূন্য){
  স্থির ধ্রুবক sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_আকার,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  ফেরত &mempoolMethods;
}

#endif /* SQLITE_ENABLE_MEMSYS3 */
