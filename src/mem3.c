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
#അന്തര്ഭാവ "sqliteInt.h"

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
തരനിർവചനം ഘടന Mem3Block Mem3Block;
ഘടന Mem3Block {
  സംഘം {
    ഘടന {
      u32 prevSize;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    ഘടന {
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
സ്ഥിതം SQLITE_WSD ഘടന Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3Block *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  പൂർണ്ണസംഖ്യ alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_താക്കോൽ_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_താക്കോൽ_Blk;
  u32 sz_താക്കോൽ_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#define mem3 GLOBAL(ഘടന Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
സ്ഥിതം ശൂന്യം memsys3UnlinkFromList(u32 i, u32 *p_വേര്){
  u32 next = mem3.aPool[i].u.list.next;
  u32 prev = mem3.aPool[i].u.list.prev;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  എങ്കിൽ( prev==0 ){
    *p_വേര് = next;
  }അല്ലെങ്കിൽ{
    mem3.aPool[prev].u.list.next = next;
  }
  എങ്കിൽ( next ){
    mem3.aPool[next].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.next = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
സ്ഥിതം ശൂന്യം memsys3Unlink(u32 i){
  u32 size, hash;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  പ്രതിജ്ഞാ( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  പ്രതിജ്ഞാ( i>=1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  പ്രതിജ്ഞാ( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  പ്രതിജ്ഞാ( size>=2 );
  എങ്കിൽ( size <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[size-2]);
  }അല്ലെങ്കിൽ{
    hash = size % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
സ്ഥിതം ശൂന്യം memsys3LinkIntoList(u32 i, u32 *p_വേര്){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.next = *p_വേര്;
  mem3.aPool[i].u.list.prev = 0;
  എങ്കിൽ( *p_വേര് ){
    mem3.aPool[*p_വേര്].u.list.prev = i;
  }
  *p_വേര് = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
സ്ഥിതം ശൂന്യം memsys3Link(u32 i){
  u32 size, hash;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  പ്രതിജ്ഞാ( i>=1 );
  പ്രതിജ്ഞാ( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  പ്രതിജ്ഞാ( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  പ്രതിജ്ഞാ( size>=2 );
  എങ്കിൽ( size <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[size-2]);
  }അല്ലെങ്കിൽ{
    hash = size % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
സ്ഥിതം ശൂന്യം memsys3Enter(ശൂന്യം){
  എങ്കിൽ( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
സ്ഥിതം ശൂന്യം memsys3Leave(ശൂന്യം){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
സ്ഥിതം ശൂന്യം memsys3OutOfMemory(പൂർണ്ണസംഖ്യ nByte){
  എങ്കിൽ( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
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
സ്ഥിതം ശൂന്യം *memsys3Checkout(u32 i, u32 nBlock){
  u32 x;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  പ്രതിജ്ഞാ( i>=1 );
  പ്രതിജ്ഞാ( mem3.aPool[i-1].u.hdr.size4x/4==nBlock );
  പ്രതിജ്ഞാ( mem3.aPool[i+nBlock-1].u.hdr.prevSize==nBlock );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = nBlock*4 | 1 | (x&2);
  mem3.aPool[i+nBlock-1].u.hdr.prevSize = nBlock;
  mem3.aPool[i+nBlock-1].u.hdr.size4x |= 2;
  തിരികെ &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
സ്ഥിതം ശൂന്യം *memsys3_From_താക്കോൽ_Blk(u32 nBlock){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  പ്രതിജ്ഞാ( mem3.sz_താക്കോൽ_Blk>=nBlock );
  എങ്കിൽ( nBlock>=mem3.sz_താക്കോൽ_Blk-1 ){
    /* Use the entire key chunk */
    ശൂന്യം *p = memsys3Checkout(mem3.i_താക്കോൽ_Blk, mem3.sz_താക്കോൽ_Blk);
    mem3.i_താക്കോൽ_Blk = 0;
    mem3.sz_താക്കോൽ_Blk = 0;
    mem3.mn_താക്കോൽ_Blk = 0;
    തിരികെ p;
  }അല്ലെങ്കിൽ{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_താക്കോൽ_Blk + mem3.sz_താക്കോൽ_Blk - nBlock;
    പ്രതിജ്ഞാ( newi > mem3.i_താക്കോൽ_Blk+1 );
    mem3.aPool[mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk-1].u.hdr.prevSize = nBlock;
    mem3.aPool[mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = nBlock*4 + 1;
    mem3.sz_താക്കോൽ_Blk -= nBlock;
    mem3.aPool[newi-1].u.hdr.prevSize = mem3.sz_താക്കോൽ_Blk;
    x = mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x = mem3.sz_താക്കോൽ_Blk*4 | x;
    എങ്കിൽ( mem3.sz_താക്കോൽ_Blk < mem3.mn_താക്കോൽ_Blk ){
      mem3.mn_താക്കോൽ_Blk = mem3.sz_താക്കോൽ_Blk;
    }
    തിരികെ (ശൂന്യം*)&mem3.aPool[newi];
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
സ്ഥിതം ശൂന്യം memsys3Merge(u32 *p_വേര്){
  u32 iNext, prev, size, i, x;

  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  വേണ്ടി(i=*p_വേര്; i>0; i=iNext){
    iNext = mem3.aPool[i].u.list.next;
    size = mem3.aPool[i-1].u.hdr.size4x;
    പ്രതിജ്ഞാ( (size&1)==0 );
    എങ്കിൽ( (size&2)==0 ){
      memsys3UnlinkFromList(i, p_വേര്);
      പ്രതിജ്ഞാ( i > mem3.aPool[i-1].u.hdr.prevSize );
      prev = i - mem3.aPool[i-1].u.hdr.prevSize;
      എങ്കിൽ( prev==iNext ){
        iNext = mem3.aPool[prev].u.list.next;
      }
      memsys3Unlink(prev);
      size = i + size/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = size*4 | x;
      mem3.aPool[prev+size-1].u.hdr.prevSize = size;
      memsys3Link(prev);
      i = prev;
    }അല്ലെങ്കിൽ{
      size /= 4;
    }
    എങ്കിൽ( size>mem3.sz_താക്കോൽ_Blk ){
      mem3.i_താക്കോൽ_Blk = i;
      mem3.sz_താക്കോൽ_Blk = size;
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
സ്ഥിതം ശൂന്യം *memsys3MallocUnsafe(പൂർണ്ണസംഖ്യ nByte){
  u32 i;
  u32 nBlock;
  u32 toFree;

  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  പ്രതിജ്ഞാ( വലുപ്പം(Mem3Block)==8 );
  എങ്കിൽ( nByte<=12 ){
    nBlock = 2;
  }അല്ലെങ്കിൽ{
    nBlock = (nByte + 11)/8;
  }
  പ്രതിജ്ഞാ( nBlock>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  എങ്കിൽ( nBlock <= MX_SMALL ){
    i = mem3.aiSmall[nBlock-2];
    എങ്കിൽ( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[nBlock-2]);
      തിരികെ memsys3Checkout(i, nBlock);
    }
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ hash = nBlock % N_HASH;
    വേണ്ടി(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.next){
      എങ്കിൽ( mem3.aPool[i-1].u.hdr.size4x/4==nBlock ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        തിരികെ memsys3Checkout(i, nBlock);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  എങ്കിൽ( mem3.sz_താക്കോൽ_Blk>=nBlock ){
    തിരികെ memsys3_From_താക്കോൽ_Blk(nBlock);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  വേണ്ടി(toFree=nBlock*16; toFree<(mem3.nPool*16); toFree *= 2){
    memsys3OutOfMemory(toFree);
    എങ്കിൽ( mem3.i_താക്കോൽ_Blk ){
      memsys3Link(mem3.i_താക്കോൽ_Blk);
      mem3.i_താക്കോൽ_Blk = 0;
      mem3.sz_താക്കോൽ_Blk = 0;
    }
    വേണ്ടി(i=0; i<N_HASH; i++){
      memsys3Merge(&mem3.aiHash[i]);
    }
    വേണ്ടി(i=0; i<MX_SMALL-1; i++){
      memsys3Merge(&mem3.aiSmall[i]);
    }
    എങ്കിൽ( mem3.sz_താക്കോൽ_Blk ){
      memsys3Unlink(mem3.i_താക്കോൽ_Blk);
      എങ്കിൽ( mem3.sz_താക്കോൽ_Blk>=nBlock ){
        തിരികെ memsys3_From_താക്കോൽ_Blk(nBlock);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  തിരികെ 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
സ്ഥിതം ശൂന്യം memsys3FreeUnsafe(ശൂന്യം *pOld){
  Mem3Block *p = (Mem3Block*)pOld;
  പൂർണ്ണസംഖ്യ i;
  u32 size, x;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem3.mutex) );
  പ്രതിജ്ഞാ( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  പ്രതിജ്ഞാ( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  പ്രതിജ്ഞാ( i+size<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+size-1].u.hdr.prevSize = size;
  mem3.aPool[i+size-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  എങ്കിൽ( mem3.i_താക്കോൽ_Blk ){
    ആകുമ്പോൾ( (mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x&2)==0 ){
      size = mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.prevSize;
      mem3.i_താക്കോൽ_Blk -= size;
      mem3.sz_താക്കോൽ_Blk += size;
      memsys3Unlink(mem3.i_താക്കോൽ_Blk);
      x = mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x = mem3.sz_താക്കോൽ_Blk*4 | x;
      mem3.aPool[mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk-1].u.hdr.prevSize = mem3.sz_താക്കോൽ_Blk;
    }
    x = mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x & 2;
    ആകുമ്പോൾ( (mem3.aPool[mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk);
      mem3.sz_താക്കോൽ_Blk += mem3.aPool[mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_താക്കോൽ_Blk-1].u.hdr.size4x = mem3.sz_താക്കോൽ_Blk*4 | x;
      mem3.aPool[mem3.i_താക്കോൽ_Blk+mem3.sz_താക്കോൽ_Blk-1].u.hdr.prevSize = mem3.sz_താക്കോൽ_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys3Size(ശൂന്യം *p){
  Mem3Block *pBlock;
  പ്രതിജ്ഞാ( p!=0 );
  pBlock = (Mem3Block*)p;
  പ്രതിജ്ഞാ( (pBlock[-1].u.hdr.size4x&1)!=0 );
  തിരികെ (pBlock[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys3Roundup(പൂർണ്ണസംഖ്യ n){
  എങ്കിൽ( n<=12 ){
    തിരികെ 12;
  }അല്ലെങ്കിൽ{
    തിരികെ ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
സ്ഥിതം ശൂന്യം *memsys3Malloc(പൂർണ്ണസംഖ്യ nBytes){
  sqlite3_int64 *p;
  പ്രതിജ്ഞാ( nBytes>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  memsys3Leave();
  തിരികെ (ശൂന്യം*)p; 
}

/*
** Free memory.
*/
സ്ഥിതം ശൂന്യം memsys3Free(ശൂന്യം *pPrior){
  പ്രതിജ്ഞാ( pPrior );
  memsys3Enter();
  memsys3FreeUnsafe(pPrior);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
സ്ഥിതം ശൂന്യം *memsys3Realloc(ശൂന്യം *pPrior, പൂർണ്ണസംഖ്യ nBytes){
  പൂർണ്ണസംഖ്യ nOld;
  ശൂന്യം *p;
  എങ്കിൽ( pPrior==0 ){
    തിരികെ sqlite3_malloc(nBytes);
  }
  എങ്കിൽ( nBytes<=0 ){
    sqlite3_free(pPrior);
    തിരികെ 0;
  }
  nOld = memsys3Size(pPrior);
  എങ്കിൽ( nBytes<=nOld && nBytes>=nOld-128 ){
    തിരികെ pPrior;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  എങ്കിൽ( p ){
    എങ്കിൽ( nOld<nBytes ){
      memcpy(p, pPrior, nOld);
    }അല്ലെങ്കിൽ{
      memcpy(p, pPrior, nBytes);
    }
    memsys3FreeUnsafe(pPrior);
  }
  memsys3Leave();
  തിരികെ p;
}

/*
** Initialize this module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memsys3Init(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  എങ്കിൽ( !sqlite3GlobalConfig.pHeap ){
    തിരികെ SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  പ്രതിജ്ഞാ( വലുപ്പം(Mem3Block)==8 );
  mem3.aPool = (Mem3Block *)sqlite3GlobalConfig.pHeap;
  mem3.nPool = (sqlite3GlobalConfig.nHeap / വലുപ്പം(Mem3Block)) - 2;

  /* Initialize the key block. */
  mem3.sz_താക്കോൽ_Blk = mem3.nPool;
  mem3.mn_താക്കോൽ_Blk = mem3.sz_താക്കോൽ_Blk;
  mem3.i_താക്കോൽ_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_താക്കോൽ_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prevSize = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  തിരികെ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
സ്ഥിതം ശൂന്യം memsys3Shutdown(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem3.mutex = 0;
  തിരികെ;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ശൂന്യം sqlite3Memsys3Dump(സ്ഥിരം അക്ഷരം *zFilename){
#ifdef SQLITE_DEBUG
  FILE *out;
  u32 i, j;
  u32 size;
  എങ്കിൽ( zFilename==0 || zFilename[0]==0 ){
    out = stdout;
  }അല്ലെങ്കിൽ{
    out = fopen(zFilename, "w");
    എങ്കിൽ( out==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      തിരികെ;
    }
  }
  memsys3Enter();
  fprintf(out, "CHUNKS:\n");
  വേണ്ടി(i=1; i<=mem3.nPool; i+=size/4){
    size = mem3.aPool[i-1].u.hdr.size4x;
    എങ്കിൽ( size/4<=1 ){
      fprintf(out, "%p size error\n", &mem3.aPool[i]);
      പ്രതിജ്ഞാ( 0 );
      നിർത്തുക;
    }
    എങ്കിൽ( (size&1)==0 && mem3.aPool[i+size/4-1].u.hdr.prevSize!=size/4 ){
      fprintf(out, "%p tail size does not match\n", &mem3.aPool[i]);
      പ്രതിജ്ഞാ( 0 );
      നിർത്തുക;
    }
    എങ്കിൽ( ((mem3.aPool[i+size/4-1].u.hdr.size4x&2)>>1)!=(size&1) ){
      fprintf(out, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      പ്രതിജ്ഞാ( 0 );
      നിർത്തുക;
    }
    എങ്കിൽ( size&1 ){
      fprintf(out, "%p %6d bytes checked out\n", &mem3.aPool[i], (size/4)*8-8);
    }അല്ലെങ്കിൽ{
      fprintf(out, "%p %6d bytes free%s\n", &mem3.aPool[i], (size/4)*8-8,
                  i==mem3.i_താക്കോൽ_Blk ? " **key**" : "");
    }
  }
  വേണ്ടി(i=0; i<MX_SMALL-1; i++){
    എങ്കിൽ( mem3.aiSmall[i]==0 ) തുടരുക;
    fprintf(out, "small(%2d):", i);
    വേണ്ടി(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.next){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  വേണ്ടി(i=0; i<N_HASH; i++){
    എങ്കിൽ( mem3.aiHash[i]==0 ) തുടരുക;
    fprintf(out, "hash(%2d):", i);
    വേണ്ടി(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.next){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  fprintf(out, "key=%d\n", mem3.i_താക്കോൽ_Blk);
  fprintf(out, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_താക്കോൽ_Blk*8);
  fprintf(out, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_താക്കോൽ_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  എങ്കിൽ( out==stdout ){
    fflush(stdout);
  }അല്ലെങ്കിൽ{
    fclose(out);
  }
#അല്ലെങ്കിൽ
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
സ്ഥിരം sqlite3_mem_methods *sqlite3MemGetMemsys3(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3Size,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  തിരികെ &mempoolMethods;
}

#endif /* SQLITE_ENABLE_MEMSYS3 */
