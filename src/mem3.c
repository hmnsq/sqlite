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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MX_SMALL 10


/*
** Number of freelist hash slots
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N_HASH  61

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Mem3_ਖੰਡ Mem3_ਖੰਡ;
ਬਣਤਰ Mem3_ਖੰਡ {
  ਸੰਘ {
    ਬਣਤਰ {
      u32 prev_ਮਾਪ;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    ਬਣਤਰ {
      u32 ਅਗਲਾ;       /* Index in mem3.aPool[] of next free chunk */
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
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3_ਖੰਡ *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  ਪੂਰਨ alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_ਕੁੰਜੀ_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_ਕੁੰਜੀ_Blk;
  u32 sz_ਕੁੰਜੀ_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mem3 GLOBAL(ਬਣਤਰ Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
ਸਥਿਰ ਖਾਲੀ memsys3UnlinkFromList(u32 i, u32 *p_ਜੜ੍ਹ){
  u32 ਅਗਲਾ = mem3.aPool[i].u.list.ਅਗਲਾ;
  u32 prev = mem3.aPool[i].u.list.prev;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਜੇ( prev==0 ){
    *p_ਜੜ੍ਹ = ਅਗਲਾ;
  }ਨਹੀਂਤਾਂ{
    mem3.aPool[prev].u.list.ਅਗਲਾ = ਅਗਲਾ;
  }
  ਜੇ( ਅਗਲਾ ){
    mem3.aPool[ਅਗਲਾ].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.ਅਗਲਾ = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
ਸਥਿਰ ਖਾਲੀ memsys3Unlink(u32 i){
  u32 ਮਾਪ, hash;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=1 );
  ਮਾਪ = mem3.aPool[i-1].u.hdr.size4x/4;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਮਾਪ==mem3.aPool[i+ਮਾਪ-1].u.hdr.prev_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਮਾਪ>=2 );
  ਜੇ( ਮਾਪ <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[ਮਾਪ-2]);
  }ਨਹੀਂਤਾਂ{
    hash = ਮਾਪ % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
ਸਥਿਰ ਖਾਲੀ memsys3LinkIntoList(u32 i, u32 *p_ਜੜ੍ਹ){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.ਅਗਲਾ = *p_ਜੜ੍ਹ;
  mem3.aPool[i].u.list.prev = 0;
  ਜੇ( *p_ਜੜ੍ਹ ){
    mem3.aPool[*p_ਜੜ੍ਹ].u.list.prev = i;
  }
  *p_ਜੜ੍ਹ = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
ਸਥਿਰ ਖਾਲੀ memsys3Link(u32 i){
  u32 ਮਾਪ, hash;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  ਮਾਪ = mem3.aPool[i-1].u.hdr.size4x/4;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਮਾਪ==mem3.aPool[i+ਮਾਪ-1].u.hdr.prev_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਮਾਪ>=2 );
  ਜੇ( ਮਾਪ <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[ਮਾਪ-2]);
  }ਨਹੀਂਤਾਂ{
    hash = ਮਾਪ % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
ਸਥਿਰ ਖਾਲੀ memsys3Enter(ਖਾਲੀ){
  ਜੇ( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
ਸਥਿਰ ਖਾਲੀ memsys3Leave(ਖਾਲੀ){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
ਸਥਿਰ ਖਾਲੀ memsys3_ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ(ਪੂਰਨ nByte){
  ਜੇ( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
    sqlite3_mutex_leave(mem3.mutex);
    sqlite3_release_ਸਮ੍ਰਿਤੀ(nByte);
    sqlite3_mutex_enter(mem3.mutex);
    mem3.alarmBusy = 0;
  }
}


/*
** Chunk i is a free chunk that has been unlinked.  Adjust its 
** size parameters for check-out and return a pointer to the 
** user portion of the chunk.
*/
ਸਥਿਰ ਖਾਲੀ *memsys3Checkout(u32 i, u32 n_ਖੰਡ){
  u32 x;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( mem3.aPool[i-1].u.hdr.size4x/4==n_ਖੰਡ );
  ਪ੍ਰਤਿਜ੍ਞਾ( mem3.aPool[i+n_ਖੰਡ-1].u.hdr.prev_ਮਾਪ==n_ਖੰਡ );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = n_ਖੰਡ*4 | 1 | (x&2);
  mem3.aPool[i+n_ਖੰਡ-1].u.hdr.prev_ਮਾਪ = n_ਖੰਡ;
  mem3.aPool[i+n_ਖੰਡ-1].u.hdr.size4x |= 2;
  ਵਾਪਸ &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
ਸਥਿਰ ਖਾਲੀ *memsys3_From_ਕੁੰਜੀ_Blk(u32 n_ਖੰਡ){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( mem3.sz_ਕੁੰਜੀ_Blk>=n_ਖੰਡ );
  ਜੇ( n_ਖੰਡ>=mem3.sz_ਕੁੰਜੀ_Blk-1 ){
    /* Use the entire key chunk */
    ਖਾਲੀ *p = memsys3Checkout(mem3.i_ਕੁੰਜੀ_Blk, mem3.sz_ਕੁੰਜੀ_Blk);
    mem3.i_ਕੁੰਜੀ_Blk = 0;
    mem3.sz_ਕੁੰਜੀ_Blk = 0;
    mem3.mn_ਕੁੰਜੀ_Blk = 0;
    ਵਾਪਸ p;
  }ਨਹੀਂਤਾਂ{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_ਕੁੰਜੀ_Blk + mem3.sz_ਕੁੰਜੀ_Blk - n_ਖੰਡ;
    ਪ੍ਰਤਿਜ੍ਞਾ( newi > mem3.i_ਕੁੰਜੀ_Blk+1 );
    mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk-1].u.hdr.prev_ਮਾਪ = n_ਖੰਡ;
    mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = n_ਖੰਡ*4 + 1;
    mem3.sz_ਕੁੰਜੀ_Blk -= n_ਖੰਡ;
    mem3.aPool[newi-1].u.hdr.prev_ਮਾਪ = mem3.sz_ਕੁੰਜੀ_Blk;
    x = mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x = mem3.sz_ਕੁੰਜੀ_Blk*4 | x;
    ਜੇ( mem3.sz_ਕੁੰਜੀ_Blk < mem3.mn_ਕੁੰਜੀ_Blk ){
      mem3.mn_ਕੁੰਜੀ_Blk = mem3.sz_ਕੁੰਜੀ_Blk;
    }
    ਵਾਪਸ (ਖਾਲੀ*)&mem3.aPool[newi];
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
ਸਥਿਰ ਖਾਲੀ memsys3_ਮਿਲਾਓ(u32 *p_ਜੜ੍ਹ){
  u32 i_ਅਗਲਾ, prev, ਮਾਪ, i, x;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਲਈ(i=*p_ਜੜ੍ਹ; i>0; i=i_ਅਗਲਾ){
    i_ਅਗਲਾ = mem3.aPool[i].u.list.ਅਗਲਾ;
    ਮਾਪ = mem3.aPool[i-1].u.hdr.size4x;
    ਪ੍ਰਤਿਜ੍ਞਾ( (ਮਾਪ&1)==0 );
    ਜੇ( (ਮਾਪ&2)==0 ){
      memsys3UnlinkFromList(i, p_ਜੜ੍ਹ);
      ਪ੍ਰਤਿਜ੍ਞਾ( i > mem3.aPool[i-1].u.hdr.prev_ਮਾਪ );
      prev = i - mem3.aPool[i-1].u.hdr.prev_ਮਾਪ;
      ਜੇ( prev==i_ਅਗਲਾ ){
        i_ਅਗਲਾ = mem3.aPool[prev].u.list.ਅਗਲਾ;
      }
      memsys3Unlink(prev);
      ਮਾਪ = i + ਮਾਪ/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = ਮਾਪ*4 | x;
      mem3.aPool[prev+ਮਾਪ-1].u.hdr.prev_ਮਾਪ = ਮਾਪ;
      memsys3Link(prev);
      i = prev;
    }ਨਹੀਂਤਾਂ{
      ਮਾਪ /= 4;
    }
    ਜੇ( ਮਾਪ>mem3.sz_ਕੁੰਜੀ_Blk ){
      mem3.i_ਕੁੰਜੀ_Blk = i;
      mem3.sz_ਕੁੰਜੀ_Blk = ਮਾਪ;
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
ਸਥਿਰ ਖਾਲੀ *memsys3MallocUnsafe(ਪੂਰਨ nByte){
  u32 i;
  u32 n_ਖੰਡ;
  u32 ਮੰਜ਼ਿਲ_Free;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(Mem3_ਖੰਡ)==8 );
  ਜੇ( nByte<=12 ){
    n_ਖੰਡ = 2;
  }ਨਹੀਂਤਾਂ{
    n_ਖੰਡ = (nByte + 11)/8;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਖੰਡ>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  ਜੇ( n_ਖੰਡ <= MX_SMALL ){
    i = mem3.aiSmall[n_ਖੰਡ-2];
    ਜੇ( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[n_ਖੰਡ-2]);
      ਵਾਪਸ memsys3Checkout(i, n_ਖੰਡ);
    }
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ hash = n_ਖੰਡ % N_HASH;
    ਲਈ(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.ਅਗਲਾ){
      ਜੇ( mem3.aPool[i-1].u.hdr.size4x/4==n_ਖੰਡ ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        ਵਾਪਸ memsys3Checkout(i, n_ਖੰਡ);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  ਜੇ( mem3.sz_ਕੁੰਜੀ_Blk>=n_ਖੰਡ ){
    ਵਾਪਸ memsys3_From_ਕੁੰਜੀ_Blk(n_ਖੰਡ);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  ਲਈ(ਮੰਜ਼ਿਲ_Free=n_ਖੰਡ*16; ਮੰਜ਼ਿਲ_Free<(mem3.nPool*16); ਮੰਜ਼ਿਲ_Free *= 2){
    memsys3_ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ(ਮੰਜ਼ਿਲ_Free);
    ਜੇ( mem3.i_ਕੁੰਜੀ_Blk ){
      memsys3Link(mem3.i_ਕੁੰਜੀ_Blk);
      mem3.i_ਕੁੰਜੀ_Blk = 0;
      mem3.sz_ਕੁੰਜੀ_Blk = 0;
    }
    ਲਈ(i=0; i<N_HASH; i++){
      memsys3_ਮਿਲਾਓ(&mem3.aiHash[i]);
    }
    ਲਈ(i=0; i<MX_SMALL-1; i++){
      memsys3_ਮਿਲਾਓ(&mem3.aiSmall[i]);
    }
    ਜੇ( mem3.sz_ਕੁੰਜੀ_Blk ){
      memsys3Unlink(mem3.i_ਕੁੰਜੀ_Blk);
      ਜੇ( mem3.sz_ਕੁੰਜੀ_Blk>=n_ਖੰਡ ){
        ਵਾਪਸ memsys3_From_ਕੁੰਜੀ_Blk(n_ਖੰਡ);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  ਵਾਪਸ 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
ਸਥਿਰ ਖਾਲੀ memsys3FreeUnsafe(ਖਾਲੀ *pOld){
  Mem3_ਖੰਡ *p = (Mem3_ਖੰਡ*)pOld;
  ਪੂਰਨ i;
  u32 ਮਾਪ, x;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem3.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  ਪ੍ਰਤਿਜ੍ਞਾ( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  ਮਾਪ = mem3.aPool[i-1].u.hdr.size4x/4;
  ਪ੍ਰਤਿਜ੍ਞਾ( i+ਮਾਪ<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+ਮਾਪ-1].u.hdr.prev_ਮਾਪ = ਮਾਪ;
  mem3.aPool[i+ਮਾਪ-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  ਜੇ( mem3.i_ਕੁੰਜੀ_Blk ){
    ਜਦੋਂ( (mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x&2)==0 ){
      ਮਾਪ = mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.prev_ਮਾਪ;
      mem3.i_ਕੁੰਜੀ_Blk -= ਮਾਪ;
      mem3.sz_ਕੁੰਜੀ_Blk += ਮਾਪ;
      memsys3Unlink(mem3.i_ਕੁੰਜੀ_Blk);
      x = mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x = mem3.sz_ਕੁੰਜੀ_Blk*4 | x;
      mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk-1].u.hdr.prev_ਮਾਪ = mem3.sz_ਕੁੰਜੀ_Blk;
    }
    x = mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x & 2;
    ਜਦੋਂ( (mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk);
      mem3.sz_ਕੁੰਜੀ_Blk += mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk-1].u.hdr.size4x = mem3.sz_ਕੁੰਜੀ_Blk*4 | x;
      mem3.aPool[mem3.i_ਕੁੰਜੀ_Blk+mem3.sz_ਕੁੰਜੀ_Blk-1].u.hdr.prev_ਮਾਪ = mem3.sz_ਕੁੰਜੀ_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
ਸਥਿਰ ਪੂਰਨ memsys3_ਮਾਪ(ਖਾਲੀ *p){
  Mem3_ਖੰਡ *p_ਖੰਡ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  p_ਖੰਡ = (Mem3_ਖੰਡ*)p;
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਖੰਡ[-1].u.hdr.size4x&1)!=0 );
  ਵਾਪਸ (p_ਖੰਡ[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
ਸਥਿਰ ਪੂਰਨ memsys3Roundup(ਪੂਰਨ n){
  ਜੇ( n<=12 ){
    ਵਾਪਸ 12;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
ਸਥਿਰ ਖਾਲੀ *memsys3Malloc(ਪੂਰਨ n_ਬਾਈਟਾਂ){
  sqlite3_int64 *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਈਟਾਂ>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(n_ਬਾਈਟਾਂ);
  memsys3Leave();
  ਵਾਪਸ (ਖਾਲੀ*)p; 
}

/*
** Free memory.
*/
ਸਥਿਰ ਖਾਲੀ memsys3Free(ਖਾਲੀ *p_ਪੂਰਵ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ );
  memsys3Enter();
  memsys3FreeUnsafe(p_ਪੂਰਵ);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
ਸਥਿਰ ਖਾਲੀ *memsys3Realloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ n_ਬਾਈਟਾਂ){
  ਪੂਰਨ nOld;
  ਖਾਲੀ *p;
  ਜੇ( p_ਪੂਰਵ==0 ){
    ਵਾਪਸ sqlite3_malloc(n_ਬਾਈਟਾਂ);
  }
  ਜੇ( n_ਬਾਈਟਾਂ<=0 ){
    sqlite3_free(p_ਪੂਰਵ);
    ਵਾਪਸ 0;
  }
  nOld = memsys3_ਮਾਪ(p_ਪੂਰਵ);
  ਜੇ( n_ਬਾਈਟਾਂ<=nOld && n_ਬਾਈਟਾਂ>=nOld-128 ){
    ਵਾਪਸ p_ਪੂਰਵ;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(n_ਬਾਈਟਾਂ);
  ਜੇ( p ){
    ਜੇ( nOld<n_ਬਾਈਟਾਂ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, p_ਪੂਰਵ, nOld);
    }ਨਹੀਂਤਾਂ{
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, p_ਪੂਰਵ, n_ਬਾਈਟਾਂ);
    }
    memsys3FreeUnsafe(p_ਪੂਰਵ);
  }
  memsys3Leave();
  ਵਾਪਸ p;
}

/*
** Initialize this module.
*/
ਸਥਿਰ ਪੂਰਨ memsys3Init(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਜੇ( !sqlite3GlobalConfig.p_ਢੇਰ ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(Mem3_ਖੰਡ)==8 );
  mem3.aPool = (Mem3_ਖੰਡ *)sqlite3GlobalConfig.p_ਢੇਰ;
  mem3.nPool = (sqlite3GlobalConfig.n_ਢੇਰ / ਆਕਾਰ(Mem3_ਖੰਡ)) - 2;

  /* Initialize the key block. */
  mem3.sz_ਕੁੰਜੀ_Blk = mem3.nPool;
  mem3.mn_ਕੁੰਜੀ_Blk = mem3.sz_ਕੁੰਜੀ_Blk;
  mem3.i_ਕੁੰਜੀ_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_ਕੁੰਜੀ_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prev_ਮਾਪ = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  ਵਾਪਸ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ਸਥਿਰ ਖਾਲੀ memsys3Shutdown(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  mem3.mutex = 0;
  ਵਾਪਸ;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ਖਾਲੀ sqlite3Memsys3Dump(ਅਚਲ ਅੱਖਰ *zFilename){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  FILE *ਬਾਹਰਲਾ;
  u32 i, j;
  u32 ਮਾਪ;
  ਜੇ( zFilename==0 || zFilename[0]==0 ){
    ਬਾਹਰਲਾ = stdout;
  }ਨਹੀਂਤਾਂ{
    ਬਾਹਰਲਾ = fopen(zFilename, "w");
    ਜੇ( ਬਾਹਰਲਾ==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      ਵਾਪਸ;
    }
  }
  memsys3Enter();
  fprintf(ਬਾਹਰਲਾ, "CHUNKS:\n");
  ਲਈ(i=1; i<=mem3.nPool; i+=ਮਾਪ/4){
    ਮਾਪ = mem3.aPool[i-1].u.hdr.size4x;
    ਜੇ( ਮਾਪ/4<=1 ){
      fprintf(ਬਾਹਰਲਾ, "%p size error\n", &mem3.aPool[i]);
      ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
      ਰੋਕੋ;
    }
    ਜੇ( (ਮਾਪ&1)==0 && mem3.aPool[i+ਮਾਪ/4-1].u.hdr.prev_ਮਾਪ!=ਮਾਪ/4 ){
      fprintf(ਬਾਹਰਲਾ, "%p tail size does not match\n", &mem3.aPool[i]);
      ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
      ਰੋਕੋ;
    }
    ਜੇ( ((mem3.aPool[i+ਮਾਪ/4-1].u.hdr.size4x&2)>>1)!=(ਮਾਪ&1) ){
      fprintf(ਬਾਹਰਲਾ, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
      ਰੋਕੋ;
    }
    ਜੇ( ਮਾਪ&1 ){
      fprintf(ਬਾਹਰਲਾ, "%p %6d bytes checked out\n", &mem3.aPool[i], (ਮਾਪ/4)*8-8);
    }ਨਹੀਂਤਾਂ{
      fprintf(ਬਾਹਰਲਾ, "%p %6d bytes free%s\n", &mem3.aPool[i], (ਮਾਪ/4)*8-8,
                  i==mem3.i_ਕੁੰਜੀ_Blk ? " **key**" : "");
    }
  }
  ਲਈ(i=0; i<MX_SMALL-1; i++){
    ਜੇ( mem3.aiSmall[i]==0 ) ਜਾਰੀ;
    fprintf(ਬਾਹਰਲਾ, "small(%2d):", i);
    ਲਈ(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.ਅਗਲਾ){
      fprintf(ਬਾਹਰਲਾ, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(ਬਾਹਰਲਾ, "\n"); 
  }
  ਲਈ(i=0; i<N_HASH; i++){
    ਜੇ( mem3.aiHash[i]==0 ) ਜਾਰੀ;
    fprintf(ਬਾਹਰਲਾ, "hash(%2d):", i);
    ਲਈ(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.ਅਗਲਾ){
      fprintf(ਬਾਹਰਲਾ, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(ਬਾਹਰਲਾ, "\n"); 
  }
  fprintf(ਬਾਹਰਲਾ, "key=%d\n", mem3.i_ਕੁੰਜੀ_Blk);
  fprintf(ਬਾਹਰਲਾ, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_ਕੁੰਜੀ_Blk*8);
  fprintf(ਬਾਹਰਲਾ, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_ਕੁੰਜੀ_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  ਜੇ( ਬਾਹਰਲਾ==stdout ){
    fflush(stdout);
  }ਨਹੀਂਤਾਂ{
    fclose(ਬਾਹਰਲਾ);
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(zFilename);
#ਜੇ_ਅੰਤ
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
ਅਚਲ sqlite3_mem_methods *sqlite3MemGetMemsys3(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_ਮਾਪ,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  ਵਾਪਸ &mempoolMethods;
}

#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_MEMSYS3 */
