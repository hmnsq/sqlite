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
#अन्तर्भाव "sqliteInt.h"

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
प्रकारव्याख्या संरचना Mem3Block Mem3Block;
संरचना Mem3Block {
  संघ {
    संरचना {
      u32 prevSize;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    संरचना {
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
स्थिर SQLITE_WSD संरचना Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3Block *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  पूर्णांक alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_किल्ली_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_किल्ली_Blk;
  u32 sz_किल्ली_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#define mem3 GLOBAL(संरचना Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
स्थिर शून्य memsys3UnlinkFromList(u32 i, u32 *p_मूळ){
  u32 next = mem3.aPool[i].u.list.next;
  u32 prev = mem3.aPool[i].u.list.prev;
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  जर( prev==0 ){
    *p_मूळ = next;
  }अन्यथा{
    mem3.aPool[prev].u.list.next = next;
  }
  जर( next ){
    mem3.aPool[next].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.next = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
स्थिर शून्य memsys3Unlink(u32 i){
  u32 size, hash;
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  प्रतिज्ञा( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  प्रतिज्ञा( i>=1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  प्रतिज्ञा( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  प्रतिज्ञा( size>=2 );
  जर( size <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[size-2]);
  }अन्यथा{
    hash = size % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
स्थिर शून्य memsys3LinkIntoList(u32 i, u32 *p_मूळ){
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.next = *p_मूळ;
  mem3.aPool[i].u.list.prev = 0;
  जर( *p_मूळ ){
    mem3.aPool[*p_मूळ].u.list.prev = i;
  }
  *p_मूळ = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
स्थिर शून्य memsys3Link(u32 i){
  u32 size, hash;
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  प्रतिज्ञा( i>=1 );
  प्रतिज्ञा( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  प्रतिज्ञा( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  प्रतिज्ञा( size>=2 );
  जर( size <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[size-2]);
  }अन्यथा{
    hash = size % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
स्थिर शून्य memsys3Enter(शून्य){
  जर( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
स्थिर शून्य memsys3Leave(शून्य){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
स्थिर शून्य memsys3OutOfMemory(पूर्णांक nByte){
  जर( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
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
स्थिर शून्य *memsys3Checkout(u32 i, u32 nBlock){
  u32 x;
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  प्रतिज्ञा( i>=1 );
  प्रतिज्ञा( mem3.aPool[i-1].u.hdr.size4x/4==nBlock );
  प्रतिज्ञा( mem3.aPool[i+nBlock-1].u.hdr.prevSize==nBlock );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = nBlock*4 | 1 | (x&2);
  mem3.aPool[i+nBlock-1].u.hdr.prevSize = nBlock;
  mem3.aPool[i+nBlock-1].u.hdr.size4x |= 2;
  परत &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
स्थिर शून्य *memsys3_From_किल्ली_Blk(u32 nBlock){
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  प्रतिज्ञा( mem3.sz_किल्ली_Blk>=nBlock );
  जर( nBlock>=mem3.sz_किल्ली_Blk-1 ){
    /* Use the entire key chunk */
    शून्य *p = memsys3Checkout(mem3.i_किल्ली_Blk, mem3.sz_किल्ली_Blk);
    mem3.i_किल्ली_Blk = 0;
    mem3.sz_किल्ली_Blk = 0;
    mem3.mn_किल्ली_Blk = 0;
    परत p;
  }अन्यथा{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_किल्ली_Blk + mem3.sz_किल्ली_Blk - nBlock;
    प्रतिज्ञा( newi > mem3.i_किल्ली_Blk+1 );
    mem3.aPool[mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk-1].u.hdr.prevSize = nBlock;
    mem3.aPool[mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = nBlock*4 + 1;
    mem3.sz_किल्ली_Blk -= nBlock;
    mem3.aPool[newi-1].u.hdr.prevSize = mem3.sz_किल्ली_Blk;
    x = mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x = mem3.sz_किल्ली_Blk*4 | x;
    जर( mem3.sz_किल्ली_Blk < mem3.mn_किल्ली_Blk ){
      mem3.mn_किल्ली_Blk = mem3.sz_किल्ली_Blk;
    }
    परत (शून्य*)&mem3.aPool[newi];
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
स्थिर शून्य memsys3Merge(u32 *p_मूळ){
  u32 iNext, prev, size, i, x;

  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  साठी(i=*p_मूळ; i>0; i=iNext){
    iNext = mem3.aPool[i].u.list.next;
    size = mem3.aPool[i-1].u.hdr.size4x;
    प्रतिज्ञा( (size&1)==0 );
    जर( (size&2)==0 ){
      memsys3UnlinkFromList(i, p_मूळ);
      प्रतिज्ञा( i > mem3.aPool[i-1].u.hdr.prevSize );
      prev = i - mem3.aPool[i-1].u.hdr.prevSize;
      जर( prev==iNext ){
        iNext = mem3.aPool[prev].u.list.next;
      }
      memsys3Unlink(prev);
      size = i + size/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = size*4 | x;
      mem3.aPool[prev+size-1].u.hdr.prevSize = size;
      memsys3Link(prev);
      i = prev;
    }अन्यथा{
      size /= 4;
    }
    जर( size>mem3.sz_किल्ली_Blk ){
      mem3.i_किल्ली_Blk = i;
      mem3.sz_किल्ली_Blk = size;
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
स्थिर शून्य *memsys3MallocUnsafe(पूर्णांक nByte){
  u32 i;
  u32 nBlock;
  u32 toFree;

  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  प्रतिज्ञा( आकार(Mem3Block)==8 );
  जर( nByte<=12 ){
    nBlock = 2;
  }अन्यथा{
    nBlock = (nByte + 11)/8;
  }
  प्रतिज्ञा( nBlock>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  जर( nBlock <= MX_SMALL ){
    i = mem3.aiSmall[nBlock-2];
    जर( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[nBlock-2]);
      परत memsys3Checkout(i, nBlock);
    }
  }अन्यथा{
    पूर्णांक hash = nBlock % N_HASH;
    साठी(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.next){
      जर( mem3.aPool[i-1].u.hdr.size4x/4==nBlock ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        परत memsys3Checkout(i, nBlock);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  जर( mem3.sz_किल्ली_Blk>=nBlock ){
    परत memsys3_From_किल्ली_Blk(nBlock);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  साठी(toFree=nBlock*16; toFree<(mem3.nPool*16); toFree *= 2){
    memsys3OutOfMemory(toFree);
    जर( mem3.i_किल्ली_Blk ){
      memsys3Link(mem3.i_किल्ली_Blk);
      mem3.i_किल्ली_Blk = 0;
      mem3.sz_किल्ली_Blk = 0;
    }
    साठी(i=0; i<N_HASH; i++){
      memsys3Merge(&mem3.aiHash[i]);
    }
    साठी(i=0; i<MX_SMALL-1; i++){
      memsys3Merge(&mem3.aiSmall[i]);
    }
    जर( mem3.sz_किल्ली_Blk ){
      memsys3Unlink(mem3.i_किल्ली_Blk);
      जर( mem3.sz_किल्ली_Blk>=nBlock ){
        परत memsys3_From_किल्ली_Blk(nBlock);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  परत 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
स्थिर शून्य memsys3FreeUnsafe(शून्य *pOld){
  Mem3Block *p = (Mem3Block*)pOld;
  पूर्णांक i;
  u32 size, x;
  प्रतिज्ञा( sqlite3_mutex_held(mem3.mutex) );
  प्रतिज्ञा( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  प्रतिज्ञा( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  प्रतिज्ञा( i+size<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+size-1].u.hdr.prevSize = size;
  mem3.aPool[i+size-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  जर( mem3.i_किल्ली_Blk ){
    पर्यंत( (mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x&2)==0 ){
      size = mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.prevSize;
      mem3.i_किल्ली_Blk -= size;
      mem3.sz_किल्ली_Blk += size;
      memsys3Unlink(mem3.i_किल्ली_Blk);
      x = mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x = mem3.sz_किल्ली_Blk*4 | x;
      mem3.aPool[mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk-1].u.hdr.prevSize = mem3.sz_किल्ली_Blk;
    }
    x = mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x & 2;
    पर्यंत( (mem3.aPool[mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk);
      mem3.sz_किल्ली_Blk += mem3.aPool[mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_किल्ली_Blk-1].u.hdr.size4x = mem3.sz_किल्ली_Blk*4 | x;
      mem3.aPool[mem3.i_किल्ली_Blk+mem3.sz_किल्ली_Blk-1].u.hdr.prevSize = mem3.sz_किल्ली_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
स्थिर पूर्णांक memsys3Size(शून्य *p){
  Mem3Block *pBlock;
  प्रतिज्ञा( p!=0 );
  pBlock = (Mem3Block*)p;
  प्रतिज्ञा( (pBlock[-1].u.hdr.size4x&1)!=0 );
  परत (pBlock[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
स्थिर पूर्णांक memsys3Roundup(पूर्णांक n){
  जर( n<=12 ){
    परत 12;
  }अन्यथा{
    परत ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
स्थिर शून्य *memsys3Malloc(पूर्णांक nBytes){
  sqlite3_int64 *p;
  प्रतिज्ञा( nBytes>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  memsys3Leave();
  परत (शून्य*)p; 
}

/*
** Free memory.
*/
स्थिर शून्य memsys3Free(शून्य *pPrior){
  प्रतिज्ञा( pPrior );
  memsys3Enter();
  memsys3FreeUnsafe(pPrior);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
स्थिर शून्य *memsys3Realloc(शून्य *pPrior, पूर्णांक nBytes){
  पूर्णांक nOld;
  शून्य *p;
  जर( pPrior==0 ){
    परत sqlite3_malloc(nBytes);
  }
  जर( nBytes<=0 ){
    sqlite3_free(pPrior);
    परत 0;
  }
  nOld = memsys3Size(pPrior);
  जर( nBytes<=nOld && nBytes>=nOld-128 ){
    परत pPrior;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  जर( p ){
    जर( nOld<nBytes ){
      स_नकल(p, pPrior, nOld);
    }अन्यथा{
      स_नकल(p, pPrior, nBytes);
    }
    memsys3FreeUnsafe(pPrior);
  }
  memsys3Leave();
  परत p;
}

/*
** Initialize this module.
*/
स्थिर पूर्णांक memsys3Init(शून्य *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  जर( !sqlite3GlobalConfig.pHeap ){
    परत SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  प्रतिज्ञा( आकार(Mem3Block)==8 );
  mem3.aPool = (Mem3Block *)sqlite3GlobalConfig.pHeap;
  mem3.nPool = (sqlite3GlobalConfig.nHeap / आकार(Mem3Block)) - 2;

  /* Initialize the key block. */
  mem3.sz_किल्ली_Blk = mem3.nPool;
  mem3.mn_किल्ली_Blk = mem3.sz_किल्ली_Blk;
  mem3.i_किल्ली_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_किल्ली_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prevSize = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  परत SQLITE_OK;
}

/*
** Deinitialize this module.
*/
स्थिर शून्य memsys3Shutdown(शून्य *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem3.mutex = 0;
  परत;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
शून्य sqlite3Memsys3Dump(स्थिरांक अक्षर *zFilename){
#ifdef SQLITE_DEBUG
  खाता *out;
  u32 i, j;
  u32 size;
  जर( zFilename==0 || zFilename[0]==0 ){
    out = मानक_निकास;
  }अन्यथा{
    out = ख_खोलो(zFilename, "w");
    जर( out==0 ){
      ख_लिखो(मानक_त्रुटि, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      परत;
    }
  }
  memsys3Enter();
  ख_लिखो(out, "CHUNKS:\n");
  साठी(i=1; i<=mem3.nPool; i+=size/4){
    size = mem3.aPool[i-1].u.hdr.size4x;
    जर( size/4<=1 ){
      ख_लिखो(out, "%p size error\n", &mem3.aPool[i]);
      प्रतिज्ञा( 0 );
      थांबा;
    }
    जर( (size&1)==0 && mem3.aPool[i+size/4-1].u.hdr.prevSize!=size/4 ){
      ख_लिखो(out, "%p tail size does not match\n", &mem3.aPool[i]);
      प्रतिज्ञा( 0 );
      थांबा;
    }
    जर( ((mem3.aPool[i+size/4-1].u.hdr.size4x&2)>>1)!=(size&1) ){
      ख_लिखो(out, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      प्रतिज्ञा( 0 );
      थांबा;
    }
    जर( size&1 ){
      ख_लिखो(out, "%p %6d bytes checked out\n", &mem3.aPool[i], (size/4)*8-8);
    }अन्यथा{
      ख_लिखो(out, "%p %6d bytes free%s\n", &mem3.aPool[i], (size/4)*8-8,
                  i==mem3.i_किल्ली_Blk ? " **key**" : "");
    }
  }
  साठी(i=0; i<MX_SMALL-1; i++){
    जर( mem3.aiSmall[i]==0 ) सुरूठेवा;
    ख_लिखो(out, "small(%2d):", i);
    साठी(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.next){
      ख_लिखो(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    ख_लिखो(out, "\n"); 
  }
  साठी(i=0; i<N_HASH; i++){
    जर( mem3.aiHash[i]==0 ) सुरूठेवा;
    ख_लिखो(out, "hash(%2d):", i);
    साठी(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.next){
      ख_लिखो(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    ख_लिखो(out, "\n"); 
  }
  ख_लिखो(out, "key=%d\n", mem3.i_किल्ली_Blk);
  ख_लिखो(out, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_किल्ली_Blk*8);
  ख_लिखो(out, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_किल्ली_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  जर( out==मानक_निकास ){
    ख_साफ(मानक_निकास);
  }अन्यथा{
    ख_बंद(out);
  }
#अन्यथा
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
स्थिरांक sqlite3_mem_methods *sqlite3MemGetMemsys3(शून्य){
  स्थिर स्थिरांक sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3Size,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  परत &mempoolMethods;
}

#endif /* SQLITE_ENABLE_MEMSYS3 */
