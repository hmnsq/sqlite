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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MX_SMALL 10


/*
** Number of freelist hash slots
*/
#ವ್ಯಾಖ್ಯಾನಿಸು N_HASH  61

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Mem3_ಖಂಡ Mem3_ಖಂಡ;
ರಚನೆ Mem3_ಖಂಡ {
  ಒಕ್ಕೂಟ {
    ರಚನೆ {
      u32 prev_ಗಾತ್ರ;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    ರಚನೆ {
      u32 ಮುಂದಿನ;       /* Index in mem3.aPool[] of next free chunk */
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
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3_ಖಂಡ *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  ಪೂರ್ಣಾಂಕ alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_ಕೀಲಿ_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_ಕೀಲಿ_Blk;
  u32 sz_ಕೀಲಿ_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#ವ್ಯಾಖ್ಯಾನಿಸು mem3 GLOBAL(ರಚನೆ Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3UnlinkFromList(u32 i, u32 *p_ಬೇರು){
  u32 ಮುಂದಿನ = mem3.aPool[i].u.list.ಮುಂದಿನ;
  u32 prev = mem3.aPool[i].u.list.prev;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಆದರೆ( prev==0 ){
    *p_ಬೇರು = ಮುಂದಿನ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    mem3.aPool[prev].u.list.ಮುಂದಿನ = ಮುಂದಿನ;
  }
  ಆದರೆ( ಮುಂದಿನ ){
    mem3.aPool[ಮುಂದಿನ].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.ಮುಂದಿನ = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3Unlink(u32 i){
  u32 ಗಾತ್ರ_2, hash;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಪ್ರತಿಜ್ಞಾ( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  ಪ್ರತಿಜ್ಞಾ( i>=1 );
  ಗಾತ್ರ_2 = mem3.aPool[i-1].u.hdr.size4x/4;
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ_2==mem3.aPool[i+ಗಾತ್ರ_2-1].u.hdr.prev_ಗಾತ್ರ );
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ_2>=2 );
  ಆದರೆ( ಗಾತ್ರ_2 <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[ಗಾತ್ರ_2-2]);
  }ಇಲ್ಲದಿದ್ದರೆ{
    hash = ಗಾತ್ರ_2 % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3LinkIntoList(u32 i, u32 *p_ಬೇರು){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.ಮುಂದಿನ = *p_ಬೇರು;
  mem3.aPool[i].u.list.prev = 0;
  ಆದರೆ( *p_ಬೇರು ){
    mem3.aPool[*p_ಬೇರು].u.list.prev = i;
  }
  *p_ಬೇರು = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3Link(u32 i){
  u32 ಗಾತ್ರ_2, hash;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಪ್ರತಿಜ್ಞಾ( i>=1 );
  ಪ್ರತಿಜ್ಞಾ( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  ಗಾತ್ರ_2 = mem3.aPool[i-1].u.hdr.size4x/4;
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ_2==mem3.aPool[i+ಗಾತ್ರ_2-1].u.hdr.prev_ಗಾತ್ರ );
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ_2>=2 );
  ಆದರೆ( ಗಾತ್ರ_2 <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[ಗಾತ್ರ_2-2]);
  }ಇಲ್ಲದಿದ್ದರೆ{
    hash = ಗಾತ್ರ_2 % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3Enter(ಶೂನ್ಯ){
  ಆದರೆ( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
ಸ್ಥಿರ ಶೂನ್ಯ memsys3Leave(ಶೂನ್ಯ){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3_ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ(ಪೂರ್ಣಾಂಕ nByte){
  ಆದರೆ( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
    sqlite3_mutex_leave(mem3.mutex);
    sqlite3_release_ಸ್ಮರಣೆ(nByte);
    sqlite3_mutex_enter(mem3.mutex);
    mem3.alarmBusy = 0;
  }
}


/*
** Chunk i is a free chunk that has been unlinked.  Adjust its 
** size parameters for check-out and return a pointer to the 
** user portion of the chunk.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memsys3Checkout(u32 i, u32 n_ಖಂಡ){
  u32 x;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಪ್ರತಿಜ್ಞಾ( i>=1 );
  ಪ್ರತಿಜ್ಞಾ( mem3.aPool[i-1].u.hdr.size4x/4==n_ಖಂಡ );
  ಪ್ರತಿಜ್ಞಾ( mem3.aPool[i+n_ಖಂಡ-1].u.hdr.prev_ಗಾತ್ರ==n_ಖಂಡ );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = n_ಖಂಡ*4 | 1 | (x&2);
  mem3.aPool[i+n_ಖಂಡ-1].u.hdr.prev_ಗಾತ್ರ = n_ಖಂಡ;
  mem3.aPool[i+n_ಖಂಡ-1].u.hdr.size4x |= 2;
  ಹಿಂತಿರುಗಿ &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memsys3_From_ಕೀಲಿ_Blk(u32 n_ಖಂಡ){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಪ್ರತಿಜ್ಞಾ( mem3.sz_ಕೀಲಿ_Blk>=n_ಖಂಡ );
  ಆದರೆ( n_ಖಂಡ>=mem3.sz_ಕೀಲಿ_Blk-1 ){
    /* Use the entire key chunk */
    ಶೂನ್ಯ *p = memsys3Checkout(mem3.i_ಕೀಲಿ_Blk, mem3.sz_ಕೀಲಿ_Blk);
    mem3.i_ಕೀಲಿ_Blk = 0;
    mem3.sz_ಕೀಲಿ_Blk = 0;
    mem3.mn_ಕೀಲಿ_Blk = 0;
    ಹಿಂತಿರುಗಿ p;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_ಕೀಲಿ_Blk + mem3.sz_ಕೀಲಿ_Blk - n_ಖಂಡ;
    ಪ್ರತಿಜ್ಞಾ( newi > mem3.i_ಕೀಲಿ_Blk+1 );
    mem3.aPool[mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk-1].u.hdr.prev_ಗಾತ್ರ = n_ಖಂಡ;
    mem3.aPool[mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = n_ಖಂಡ*4 + 1;
    mem3.sz_ಕೀಲಿ_Blk -= n_ಖಂಡ;
    mem3.aPool[newi-1].u.hdr.prev_ಗಾತ್ರ = mem3.sz_ಕೀಲಿ_Blk;
    x = mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x = mem3.sz_ಕೀಲಿ_Blk*4 | x;
    ಆದರೆ( mem3.sz_ಕೀಲಿ_Blk < mem3.mn_ಕೀಲಿ_Blk ){
      mem3.mn_ಕೀಲಿ_Blk = mem3.sz_ಕೀಲಿ_Blk;
    }
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)&mem3.aPool[newi];
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
ಸ್ಥಿರ ಶೂನ್ಯ memsys3_ವಿಲೀನ(u32 *p_ಬೇರು){
  u32 i_ಮುಂದಿನ, prev, ಗಾತ್ರ_2, i, x;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಗಾಗಿ(i=*p_ಬೇರು; i>0; i=i_ಮುಂದಿನ){
    i_ಮುಂದಿನ = mem3.aPool[i].u.list.ಮುಂದಿನ;
    ಗಾತ್ರ_2 = mem3.aPool[i-1].u.hdr.size4x;
    ಪ್ರತಿಜ್ಞಾ( (ಗಾತ್ರ_2&1)==0 );
    ಆದರೆ( (ಗಾತ್ರ_2&2)==0 ){
      memsys3UnlinkFromList(i, p_ಬೇರು);
      ಪ್ರತಿಜ್ಞಾ( i > mem3.aPool[i-1].u.hdr.prev_ಗಾತ್ರ );
      prev = i - mem3.aPool[i-1].u.hdr.prev_ಗಾತ್ರ;
      ಆದರೆ( prev==i_ಮುಂದಿನ ){
        i_ಮುಂದಿನ = mem3.aPool[prev].u.list.ಮುಂದಿನ;
      }
      memsys3Unlink(prev);
      ಗಾತ್ರ_2 = i + ಗಾತ್ರ_2/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = ಗಾತ್ರ_2*4 | x;
      mem3.aPool[prev+ಗಾತ್ರ_2-1].u.hdr.prev_ಗಾತ್ರ = ಗಾತ್ರ_2;
      memsys3Link(prev);
      i = prev;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಗಾತ್ರ_2 /= 4;
    }
    ಆದರೆ( ಗಾತ್ರ_2>mem3.sz_ಕೀಲಿ_Blk ){
      mem3.i_ಕೀಲಿ_Blk = i;
      mem3.sz_ಕೀಲಿ_Blk = ಗಾತ್ರ_2;
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
ಸ್ಥಿರ ಶೂನ್ಯ *memsys3MallocUnsafe(ಪೂರ್ಣಾಂಕ nByte){
  u32 i;
  u32 n_ಖಂಡ;
  u32 ಗಮ್ಯ_Free;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(Mem3_ಖಂಡ)==8 );
  ಆದರೆ( nByte<=12 ){
    n_ಖಂಡ = 2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    n_ಖಂಡ = (nByte + 11)/8;
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಖಂಡ>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  ಆದರೆ( n_ಖಂಡ <= MX_SMALL ){
    i = mem3.aiSmall[n_ಖಂಡ-2];
    ಆದರೆ( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[n_ಖಂಡ-2]);
      ಹಿಂತಿರುಗಿ memsys3Checkout(i, n_ಖಂಡ);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ hash = n_ಖಂಡ % N_HASH;
    ಗಾಗಿ(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.ಮುಂದಿನ){
      ಆದರೆ( mem3.aPool[i-1].u.hdr.size4x/4==n_ಖಂಡ ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        ಹಿಂತಿರುಗಿ memsys3Checkout(i, n_ಖಂಡ);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  ಆದರೆ( mem3.sz_ಕೀಲಿ_Blk>=n_ಖಂಡ ){
    ಹಿಂತಿರುಗಿ memsys3_From_ಕೀಲಿ_Blk(n_ಖಂಡ);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  ಗಾಗಿ(ಗಮ್ಯ_Free=n_ಖಂಡ*16; ಗಮ್ಯ_Free<(mem3.nPool*16); ಗಮ್ಯ_Free *= 2){
    memsys3_ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ(ಗಮ್ಯ_Free);
    ಆದರೆ( mem3.i_ಕೀಲಿ_Blk ){
      memsys3Link(mem3.i_ಕೀಲಿ_Blk);
      mem3.i_ಕೀಲಿ_Blk = 0;
      mem3.sz_ಕೀಲಿ_Blk = 0;
    }
    ಗಾಗಿ(i=0; i<N_HASH; i++){
      memsys3_ವಿಲೀನ(&mem3.aiHash[i]);
    }
    ಗಾಗಿ(i=0; i<MX_SMALL-1; i++){
      memsys3_ವಿಲೀನ(&mem3.aiSmall[i]);
    }
    ಆದರೆ( mem3.sz_ಕೀಲಿ_Blk ){
      memsys3Unlink(mem3.i_ಕೀಲಿ_Blk);
      ಆದರೆ( mem3.sz_ಕೀಲಿ_Blk>=n_ಖಂಡ ){
        ಹಿಂತಿರುಗಿ memsys3_From_ಕೀಲಿ_Blk(n_ಖಂಡ);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3FreeUnsafe(ಶೂನ್ಯ *pOld){
  Mem3_ಖಂಡ *p = (Mem3_ಖಂಡ*)pOld;
  ಪೂರ್ಣಾಂಕ i;
  u32 ಗಾತ್ರ_2, x;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem3.mutex) );
  ಪ್ರತಿಜ್ಞಾ( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  ಪ್ರತಿಜ್ಞಾ( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  ಗಾತ್ರ_2 = mem3.aPool[i-1].u.hdr.size4x/4;
  ಪ್ರತಿಜ್ಞಾ( i+ಗಾತ್ರ_2<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+ಗಾತ್ರ_2-1].u.hdr.prev_ಗಾತ್ರ = ಗಾತ್ರ_2;
  mem3.aPool[i+ಗಾತ್ರ_2-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  ಆದರೆ( mem3.i_ಕೀಲಿ_Blk ){
    ಆಗ( (mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x&2)==0 ){
      ಗಾತ್ರ_2 = mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.prev_ಗಾತ್ರ;
      mem3.i_ಕೀಲಿ_Blk -= ಗಾತ್ರ_2;
      mem3.sz_ಕೀಲಿ_Blk += ಗಾತ್ರ_2;
      memsys3Unlink(mem3.i_ಕೀಲಿ_Blk);
      x = mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x = mem3.sz_ಕೀಲಿ_Blk*4 | x;
      mem3.aPool[mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk-1].u.hdr.prev_ಗಾತ್ರ = mem3.sz_ಕೀಲಿ_Blk;
    }
    x = mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x & 2;
    ಆಗ( (mem3.aPool[mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk);
      mem3.sz_ಕೀಲಿ_Blk += mem3.aPool[mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_ಕೀಲಿ_Blk-1].u.hdr.size4x = mem3.sz_ಕೀಲಿ_Blk*4 | x;
      mem3.aPool[mem3.i_ಕೀಲಿ_Blk+mem3.sz_ಕೀಲಿ_Blk-1].u.hdr.prev_ಗಾತ್ರ = mem3.sz_ಕೀಲಿ_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys3_ಗಾತ್ರ(ಶೂನ್ಯ *p){
  Mem3_ಖಂಡ *p_ಖಂಡ;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  p_ಖಂಡ = (Mem3_ಖಂಡ*)p;
  ಪ್ರತಿಜ್ಞಾ( (p_ಖಂಡ[-1].u.hdr.size4x&1)!=0 );
  ಹಿಂತಿರುಗಿ (p_ಖಂಡ[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys3Roundup(ಪೂರ್ಣಾಂಕ n){
  ಆದರೆ( n<=12 ){
    ಹಿಂತಿರುಗಿ 12;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memsys3Malloc(ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು){
  sqlite3_int64 *p;
  ಪ್ರತಿಜ್ಞಾ( n_ಬೈಟುಗಳು>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(n_ಬೈಟುಗಳು);
  memsys3Leave();
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)p; 
}

/*
** Free memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3Free(ಶೂನ್ಯ *p_ಪೂರ್ವ){
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ );
  memsys3Enter();
  memsys3FreeUnsafe(p_ಪೂರ್ವ);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memsys3Realloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು){
  ಪೂರ್ಣಾಂಕ nOld;
  ಶೂನ್ಯ *p;
  ಆದರೆ( p_ಪೂರ್ವ==0 ){
    ಹಿಂತಿರುಗಿ sqlite3_malloc(n_ಬೈಟುಗಳು);
  }
  ಆದರೆ( n_ಬೈಟುಗಳು<=0 ){
    sqlite3_free(p_ಪೂರ್ವ);
    ಹಿಂತಿರುಗಿ 0;
  }
  nOld = memsys3_ಗಾತ್ರ(p_ಪೂರ್ವ);
  ಆದರೆ( n_ಬೈಟುಗಳು<=nOld && n_ಬೈಟುಗಳು>=nOld-128 ){
    ಹಿಂತಿರುಗಿ p_ಪೂರ್ವ;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(n_ಬೈಟುಗಳು);
  ಆದರೆ( p ){
    ಆದರೆ( nOld<n_ಬೈಟುಗಳು ){
      ಸ್ಮರಣೆ_ನಕಲು(p, p_ಪೂರ್ವ, nOld);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಮರಣೆ_ನಕಲು(p, p_ಪೂರ್ವ, n_ಬೈಟುಗಳು);
    }
    memsys3FreeUnsafe(p_ಪೂರ್ವ);
  }
  memsys3Leave();
  ಹಿಂತಿರುಗಿ p;
}

/*
** Initialize this module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memsys3Init(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಆದರೆ( !sqlite3GlobalConfig.p_ರಾಶಿ ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(Mem3_ಖಂಡ)==8 );
  mem3.aPool = (Mem3_ಖಂಡ *)sqlite3GlobalConfig.p_ರಾಶಿ;
  mem3.nPool = (sqlite3GlobalConfig.n_ರಾಶಿ / ಗಾತ್ರ(Mem3_ಖಂಡ)) - 2;

  /* Initialize the key block. */
  mem3.sz_ಕೀಲಿ_Blk = mem3.nPool;
  mem3.mn_ಕೀಲಿ_Blk = mem3.sz_ಕೀಲಿ_Blk;
  mem3.i_ಕೀಲಿ_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_ಕೀಲಿ_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prev_ಗಾತ್ರ = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memsys3Shutdown(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  mem3.mutex = 0;
  ಹಿಂತಿರುಗಿ;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
ಶೂನ್ಯ sqlite3Memsys3Dump(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  FILE *ಹೊರಹೋಗು;
  u32 i, j;
  u32 ಗಾತ್ರ_2;
  ಆದರೆ( zFilename==0 || zFilename[0]==0 ){
    ಹೊರಹೋಗು = stdout;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹೊರಹೋಗು = fopen(zFilename, "w");
    ಆದರೆ( ಹೊರಹೋಗು==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      ಹಿಂತಿರುಗಿ;
    }
  }
  memsys3Enter();
  fprintf(ಹೊರಹೋಗು, "CHUNKS:\n");
  ಗಾಗಿ(i=1; i<=mem3.nPool; i+=ಗಾತ್ರ_2/4){
    ಗಾತ್ರ_2 = mem3.aPool[i-1].u.hdr.size4x;
    ಆದರೆ( ಗಾತ್ರ_2/4<=1 ){
      fprintf(ಹೊರಹೋಗು, "%p size error\n", &mem3.aPool[i]);
      ಪ್ರತಿಜ್ಞಾ( 0 );
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( (ಗಾತ್ರ_2&1)==0 && mem3.aPool[i+ಗಾತ್ರ_2/4-1].u.hdr.prev_ಗಾತ್ರ!=ಗಾತ್ರ_2/4 ){
      fprintf(ಹೊರಹೋಗು, "%p tail size does not match\n", &mem3.aPool[i]);
      ಪ್ರತಿಜ್ಞಾ( 0 );
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( ((mem3.aPool[i+ಗಾತ್ರ_2/4-1].u.hdr.size4x&2)>>1)!=(ಗಾತ್ರ_2&1) ){
      fprintf(ಹೊರಹೋಗು, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      ಪ್ರತಿಜ್ಞಾ( 0 );
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( ಗಾತ್ರ_2&1 ){
      fprintf(ಹೊರಹೋಗು, "%p %6d bytes checked out\n", &mem3.aPool[i], (ಗಾತ್ರ_2/4)*8-8);
    }ಇಲ್ಲದಿದ್ದರೆ{
      fprintf(ಹೊರಹೋಗು, "%p %6d bytes free%s\n", &mem3.aPool[i], (ಗಾತ್ರ_2/4)*8-8,
                  i==mem3.i_ಕೀಲಿ_Blk ? " **key**" : "");
    }
  }
  ಗಾಗಿ(i=0; i<MX_SMALL-1; i++){
    ಆದರೆ( mem3.aiSmall[i]==0 ) ಮುಂದುವರಿ;
    fprintf(ಹೊರಹೋಗು, "small(%2d):", i);
    ಗಾಗಿ(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.ಮುಂದಿನ){
      fprintf(ಹೊರಹೋಗು, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(ಹೊರಹೋಗು, "\n"); 
  }
  ಗಾಗಿ(i=0; i<N_HASH; i++){
    ಆದರೆ( mem3.aiHash[i]==0 ) ಮುಂದುವರಿ;
    fprintf(ಹೊರಹೋಗು, "hash(%2d):", i);
    ಗಾಗಿ(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.ಮುಂದಿನ){
      fprintf(ಹೊರಹೋಗು, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(ಹೊರಹೋಗು, "\n"); 
  }
  fprintf(ಹೊರಹೋಗು, "key=%d\n", mem3.i_ಕೀಲಿ_Blk);
  fprintf(ಹೊರಹೋಗು, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_ಕೀಲಿ_Blk*8);
  fprintf(ಹೊರಹೋಗು, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_ಕೀಲಿ_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  ಆದರೆ( ಹೊರಹೋಗು==stdout ){
    fflush(stdout);
  }ಇಲ್ಲದಿದ್ದರೆ{
    fclose(ಹೊರಹೋಗು);
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(zFilename);
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetMemsys3(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_ಗಾತ್ರ,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  ಹಿಂತಿರುಗಿ &mempoolMethods;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_MEMSYS3 */
