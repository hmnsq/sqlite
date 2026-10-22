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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା Mem3Block Mem3Block;
ସଂରଚନା Mem3Block {
  ସଂଘ {
    ସଂରଚନା {
      u32 prevSize;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    ସଂରଚନା {
      u32 next;       /* Index in mem3.aPool[] of next free chunk */
      u32 prev;       /* Index in mem3.aPool[] of previous free chunk */
    } list;
  } u;
};

/*
** All of the static variables used by this module are collected
** into a single structure named "mem3".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ସ୍ଥିର SQLITE_WSD ସଂରଚନା Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3Block *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_ଚାବି_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_ଚାବି_Blk;
  u32 sz_ଚାବି_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#define mem3 GLOBAL(ସଂରଚନା Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3UnlinkFromList(u32 i, u32 *p_ମୂଳ){
  u32 next = mem3.aPool[i].u.list.next;
  u32 prev = mem3.aPool[i].u.list.prev;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ଯଦି( prev==0 ){
    *p_ମୂଳ = next;
  }ଅନ୍ୟଥା{
    mem3.aPool[prev].u.list.next = next;
  }
  ଯଦି( next ){
    mem3.aPool[next].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.next = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3Unlink(u32 i){
  u32 size, hash;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପ୍ରତିଜ୍ଞା( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  ପ୍ରତିଜ୍ଞା( i>=1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  ପ୍ରତିଜ୍ଞା( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  ପ୍ରତିଜ୍ଞା( size>=2 );
  ଯଦି( size <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[size-2]);
  }ଅନ୍ୟଥା{
    hash = size % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3LinkIntoList(u32 i, u32 *p_ମୂଳ){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.next = *p_ମୂଳ;
  mem3.aPool[i].u.list.prev = 0;
  ଯଦି( *p_ମୂଳ ){
    mem3.aPool[*p_ମୂଳ].u.list.prev = i;
  }
  *p_ମୂଳ = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3Link(u32 i){
  u32 size, hash;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପ୍ରତିଜ୍ଞା( i>=1 );
  ପ୍ରତିଜ୍ଞା( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  ପ୍ରତିଜ୍ଞା( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  ପ୍ରତିଜ୍ଞା( size>=2 );
  ଯଦି( size <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[size-2]);
  }ଅନ୍ୟଥା{
    hash = size % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3Enter(ଶୂନ୍ୟ){
  ଯଦି( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
ସ୍ଥିର ଶୂନ୍ୟ memsys3Leave(ଶୂନ୍ୟ){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3OutOfMemory(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  ଯଦି( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
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
ସ୍ଥିର ଶୂନ୍ୟ *memsys3Checkout(u32 i, u32 nBlock){
  u32 x;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପ୍ରତିଜ୍ଞା( i>=1 );
  ପ୍ରତିଜ୍ଞା( mem3.aPool[i-1].u.hdr.size4x/4==nBlock );
  ପ୍ରତିଜ୍ଞା( mem3.aPool[i+nBlock-1].u.hdr.prevSize==nBlock );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = nBlock*4 | 1 | (x&2);
  mem3.aPool[i+nBlock-1].u.hdr.prevSize = nBlock;
  mem3.aPool[i+nBlock-1].u.hdr.size4x |= 2;
  ଫେରନ୍ତୁ &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
ସ୍ଥିର ଶୂନ୍ୟ *memsys3_From_ଚାବି_Blk(u32 nBlock){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପ୍ରତିଜ୍ଞା( mem3.sz_ଚାବି_Blk>=nBlock );
  ଯଦି( nBlock>=mem3.sz_ଚାବି_Blk-1 ){
    /* Use the entire key chunk */
    ଶୂନ୍ୟ *p = memsys3Checkout(mem3.i_ଚାବି_Blk, mem3.sz_ଚାବି_Blk);
    mem3.i_ଚାବି_Blk = 0;
    mem3.sz_ଚାବି_Blk = 0;
    mem3.mn_ଚାବି_Blk = 0;
    ଫେରନ୍ତୁ p;
  }ଅନ୍ୟଥା{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_ଚାବି_Blk + mem3.sz_ଚାବି_Blk - nBlock;
    ପ୍ରତିଜ୍ଞା( newi > mem3.i_ଚାବି_Blk+1 );
    mem3.aPool[mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk-1].u.hdr.prevSize = nBlock;
    mem3.aPool[mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = nBlock*4 + 1;
    mem3.sz_ଚାବି_Blk -= nBlock;
    mem3.aPool[newi-1].u.hdr.prevSize = mem3.sz_ଚାବି_Blk;
    x = mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x = mem3.sz_ଚାବି_Blk*4 | x;
    ଯଦି( mem3.sz_ଚାବି_Blk < mem3.mn_ଚାବି_Blk ){
      mem3.mn_ଚାବି_Blk = mem3.sz_ଚାବି_Blk;
    }
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)&mem3.aPool[newi];
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
ସ୍ଥିର ଶୂନ୍ୟ memsys3Merge(u32 *p_ମୂଳ){
  u32 iNext, prev, size, i, x;

  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପାଇଁ(i=*p_ମୂଳ; i>0; i=iNext){
    iNext = mem3.aPool[i].u.list.next;
    size = mem3.aPool[i-1].u.hdr.size4x;
    ପ୍ରତିଜ୍ଞା( (size&1)==0 );
    ଯଦି( (size&2)==0 ){
      memsys3UnlinkFromList(i, p_ମୂଳ);
      ପ୍ରତିଜ୍ଞା( i > mem3.aPool[i-1].u.hdr.prevSize );
      prev = i - mem3.aPool[i-1].u.hdr.prevSize;
      ଯଦି( prev==iNext ){
        iNext = mem3.aPool[prev].u.list.next;
      }
      memsys3Unlink(prev);
      size = i + size/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = size*4 | x;
      mem3.aPool[prev+size-1].u.hdr.prevSize = size;
      memsys3Link(prev);
      i = prev;
    }ଅନ୍ୟଥା{
      size /= 4;
    }
    ଯଦି( size>mem3.sz_ଚାବି_Blk ){
      mem3.i_ଚାବି_Blk = i;
      mem3.sz_ଚାବି_Blk = size;
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
ସ୍ଥିର ଶୂନ୍ୟ *memsys3MallocUnsafe(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  u32 i;
  u32 nBlock;
  u32 toFree;

  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପ୍ରତିଜ୍ଞା( ଆକାର(Mem3Block)==8 );
  ଯଦି( nByte<=12 ){
    nBlock = 2;
  }ଅନ୍ୟଥା{
    nBlock = (nByte + 11)/8;
  }
  ପ୍ରତିଜ୍ଞା( nBlock>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  ଯଦି( nBlock <= MX_SMALL ){
    i = mem3.aiSmall[nBlock-2];
    ଯଦି( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[nBlock-2]);
      ଫେରନ୍ତୁ memsys3Checkout(i, nBlock);
    }
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash = nBlock % N_HASH;
    ପାଇଁ(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.next){
      ଯଦି( mem3.aPool[i-1].u.hdr.size4x/4==nBlock ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        ଫେରନ୍ତୁ memsys3Checkout(i, nBlock);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  ଯଦି( mem3.sz_ଚାବି_Blk>=nBlock ){
    ଫେରନ୍ତୁ memsys3_From_ଚାବି_Blk(nBlock);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  ପାଇଁ(toFree=nBlock*16; toFree<(mem3.nPool*16); toFree *= 2){
    memsys3OutOfMemory(toFree);
    ଯଦି( mem3.i_ଚାବି_Blk ){
      memsys3Link(mem3.i_ଚାବି_Blk);
      mem3.i_ଚାବି_Blk = 0;
      mem3.sz_ଚାବି_Blk = 0;
    }
    ପାଇଁ(i=0; i<N_HASH; i++){
      memsys3Merge(&mem3.aiHash[i]);
    }
    ପାଇଁ(i=0; i<MX_SMALL-1; i++){
      memsys3Merge(&mem3.aiSmall[i]);
    }
    ଯଦି( mem3.sz_ଚାବି_Blk ){
      memsys3Unlink(mem3.i_ଚାବି_Blk);
      ଯଦି( mem3.sz_ଚାବି_Blk>=nBlock ){
        ଫେରନ୍ତୁ memsys3_From_ଚାବି_Blk(nBlock);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  ଫେରନ୍ତୁ 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3FreeUnsafe(ଶୂନ୍ୟ *pOld){
  Mem3Block *p = (Mem3Block*)pOld;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  u32 size, x;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem3.mutex) );
  ପ୍ରତିଜ୍ଞା( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  ପ୍ରତିଜ୍ଞା( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  ପ୍ରତିଜ୍ଞା( i+size<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+size-1].u.hdr.prevSize = size;
  mem3.aPool[i+size-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  ଯଦି( mem3.i_ଚାବି_Blk ){
    ଯେତେବେଳେ( (mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x&2)==0 ){
      size = mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.prevSize;
      mem3.i_ଚାବି_Blk -= size;
      mem3.sz_ଚାବି_Blk += size;
      memsys3Unlink(mem3.i_ଚାବି_Blk);
      x = mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x = mem3.sz_ଚାବି_Blk*4 | x;
      mem3.aPool[mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk-1].u.hdr.prevSize = mem3.sz_ଚାବି_Blk;
    }
    x = mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x & 2;
    ଯେତେବେଳେ( (mem3.aPool[mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk);
      mem3.sz_ଚାବି_Blk += mem3.aPool[mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_ଚାବି_Blk-1].u.hdr.size4x = mem3.sz_ଚାବି_Blk*4 | x;
      mem3.aPool[mem3.i_ଚାବି_Blk+mem3.sz_ଚାବି_Blk-1].u.hdr.prevSize = mem3.sz_ଚାବି_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା memsys3Size(ଶୂନ୍ୟ *p){
  Mem3Block *pBlock;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  pBlock = (Mem3Block*)p;
  ପ୍ରତିଜ୍ଞା( (pBlock[-1].u.hdr.size4x&1)!=0 );
  ଫେରନ୍ତୁ (pBlock[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା memsys3Roundup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ଯଦି( n<=12 ){
    ଫେରନ୍ତୁ 12;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
ସ୍ଥିର ଶୂନ୍ୟ *memsys3Malloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes){
  sqlite3_int64 *p;
  ପ୍ରତିଜ୍ଞା( nBytes>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  memsys3Leave();
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)p; 
}

/*
** Free memory.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3Free(ଶୂନ୍ୟ *pPrior){
  ପ୍ରତିଜ୍ଞା( pPrior );
  memsys3Enter();
  memsys3FreeUnsafe(pPrior);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
ସ୍ଥିର ଶୂନ୍ୟ *memsys3Realloc(ଶୂନ୍ୟ *pPrior, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOld;
  ଶୂନ୍ୟ *p;
  ଯଦି( pPrior==0 ){
    ଫେରନ୍ତୁ sqlite3_malloc(nBytes);
  }
  ଯଦି( nBytes<=0 ){
    sqlite3_free(pPrior);
    ଫେରନ୍ତୁ 0;
  }
  nOld = memsys3Size(pPrior);
  ଯଦି( nBytes<=nOld && nBytes>=nOld-128 ){
    ଫେରନ୍ତୁ pPrior;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  ଯଦି( p ){
    ଯଦି( nOld<nBytes ){
      memcpy(p, pPrior, nOld);
    }ଅନ୍ୟଥା{
      memcpy(p, pPrior, nBytes);
    }
    memsys3FreeUnsafe(pPrior);
  }
  memsys3Leave();
  ଫେରନ୍ତୁ p;
}

/*
** Initialize this module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା memsys3Init(ଶୂନ୍ୟ *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  ଯଦି( !sqlite3GlobalConfig.pHeap ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  ପ୍ରତିଜ୍ଞା( ଆକାର(Mem3Block)==8 );
  mem3.aPool = (Mem3Block *)sqlite3GlobalConfig.pHeap;
  mem3.nPool = (sqlite3GlobalConfig.nHeap / ଆକାର(Mem3Block)) - 2;

  /* Initialize the key block. */
  mem3.sz_ଚାବି_Blk = mem3.nPool;
  mem3.mn_ଚାବି_Blk = mem3.sz_ଚାବି_Blk;
  mem3.i_ଚାବି_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_ଚାବି_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prevSize = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ସ୍ଥିର ଶୂନ୍ୟ memsys3Shutdown(ଶୂନ୍ୟ *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem3.mutex = 0;
  ଫେରନ୍ତୁ;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ଶୂନ୍ୟ sqlite3Memsys3Dump(ଅଚଳ ଅକ୍ଷର *zFilename){
#ifdef SQLITE_DEBUG
  FILE *out;
  u32 i, j;
  u32 size;
  ଯଦି( zFilename==0 || zFilename[0]==0 ){
    out = stdout;
  }ଅନ୍ୟଥା{
    out = fopen(zFilename, "w");
    ଯଦି( out==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      ଫେରନ୍ତୁ;
    }
  }
  memsys3Enter();
  fprintf(out, "CHUNKS:\n");
  ପାଇଁ(i=1; i<=mem3.nPool; i+=size/4){
    size = mem3.aPool[i-1].u.hdr.size4x;
    ଯଦି( size/4<=1 ){
      fprintf(out, "%p size error\n", &mem3.aPool[i]);
      ପ୍ରତିଜ୍ଞା( 0 );
      ଭାଙ୍ଗ;
    }
    ଯଦି( (size&1)==0 && mem3.aPool[i+size/4-1].u.hdr.prevSize!=size/4 ){
      fprintf(out, "%p tail size does not match\n", &mem3.aPool[i]);
      ପ୍ରତିଜ୍ଞା( 0 );
      ଭାଙ୍ଗ;
    }
    ଯଦି( ((mem3.aPool[i+size/4-1].u.hdr.size4x&2)>>1)!=(size&1) ){
      fprintf(out, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      ପ୍ରତିଜ୍ଞା( 0 );
      ଭାଙ୍ଗ;
    }
    ଯଦି( size&1 ){
      fprintf(out, "%p %6d bytes checked out\n", &mem3.aPool[i], (size/4)*8-8);
    }ଅନ୍ୟଥା{
      fprintf(out, "%p %6d bytes free%s\n", &mem3.aPool[i], (size/4)*8-8,
                  i==mem3.i_ଚାବି_Blk ? " **key**" : "");
    }
  }
  ପାଇଁ(i=0; i<MX_SMALL-1; i++){
    ଯଦି( mem3.aiSmall[i]==0 ) ଜାରି;
    fprintf(out, "small(%2d):", i);
    ପାଇଁ(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.next){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  ପାଇଁ(i=0; i<N_HASH; i++){
    ଯଦି( mem3.aiHash[i]==0 ) ଜାରି;
    fprintf(out, "hash(%2d):", i);
    ପାଇଁ(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.next){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  fprintf(out, "key=%d\n", mem3.i_ଚାବି_Blk);
  fprintf(out, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_ଚାବି_Blk*8);
  fprintf(out, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_ଚାବି_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  ଯଦି( out==stdout ){
    fflush(stdout);
  }ଅନ୍ୟଥା{
    fclose(out);
  }
#ଅନ୍ୟଥା
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
ଅଚଳ sqlite3_mem_methods *sqlite3MemGetMemsys3(ଶୂନ୍ୟ){
  ସ୍ଥିର ଅଚଳ sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3Size,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  ଫେରନ୍ତୁ &mempoolMethods;
}

#endif /* SQLITE_ENABLE_MEMSYS3 */
