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
#అన్తర్భావ "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#నిర్వచించు MX_SMALL 10


/*
** Number of freelist hash slots
*/
#నిర్వచించు N_HASH  61

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
రకనిర్వచనం నిర్మాణం Mem3_ఖండం Mem3_ఖండం;
నిర్మాణం Mem3_ఖండం {
  సంఘం {
    నిర్మాణం {
      u32 prev_కొలత;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    నిర్మాణం {
      u32 తదుపరి;       /* Index in mem3.aPool[] of next free chunk */
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
స్థిర SQLITE_WSD నిర్మాణం Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3_ఖండం *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  పూర్ణాంకం alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_కీలకం_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_కీలకం_Blk;
  u32 sz_కీలకం_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#నిర్వచించు mem3 GLOBAL(నిర్మాణం Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
స్థిర శూన్య memsys3UnlinkFromList(u32 i, u32 *p_మూలం){
  u32 తదుపరి = mem3.aPool[i].u.list.తదుపరి;
  u32 prev = mem3.aPool[i].u.list.prev;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ఒకవేళ( prev==0 ){
    *p_మూలం = తదుపరి;
  }లేకపోతే{
    mem3.aPool[prev].u.list.తదుపరి = తదుపరి;
  }
  ఒకవేళ( తదుపరి ){
    mem3.aPool[తదుపరి].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.తదుపరి = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
స్థిర శూన్య memsys3Unlink(u32 i){
  u32 కొలత, hash;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ప్రతిజ్ఞా( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  ప్రతిజ్ఞా( i>=1 );
  కొలత = mem3.aPool[i-1].u.hdr.size4x/4;
  ప్రతిజ్ఞా( కొలత==mem3.aPool[i+కొలత-1].u.hdr.prev_కొలత );
  ప్రతిజ్ఞా( కొలత>=2 );
  ఒకవేళ( కొలత <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[కొలత-2]);
  }లేకపోతే{
    hash = కొలత % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
స్థిర శూన్య memsys3LinkIntoList(u32 i, u32 *p_మూలం){
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.తదుపరి = *p_మూలం;
  mem3.aPool[i].u.list.prev = 0;
  ఒకవేళ( *p_మూలం ){
    mem3.aPool[*p_మూలం].u.list.prev = i;
  }
  *p_మూలం = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
స్థిర శూన్య memsys3Link(u32 i){
  u32 కొలత, hash;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ప్రతిజ్ఞా( i>=1 );
  ప్రతిజ్ఞా( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  కొలత = mem3.aPool[i-1].u.hdr.size4x/4;
  ప్రతిజ్ఞా( కొలత==mem3.aPool[i+కొలత-1].u.hdr.prev_కొలత );
  ప్రతిజ్ఞా( కొలత>=2 );
  ఒకవేళ( కొలత <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[కొలత-2]);
  }లేకపోతే{
    hash = కొలత % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
స్థిర శూన్య memsys3Enter(శూన్య){
  ఒకవేళ( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
స్థిర శూన్య memsys3Leave(శూన్య){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
స్థిర శూన్య memsys3_బహిర్గతం_యొక్క_జ్ఞాపకం(పూర్ణాంకం nByte){
  ఒకవేళ( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
    sqlite3_mutex_leave(mem3.mutex);
    sqlite3_release_జ్ఞాపకం(nByte);
    sqlite3_mutex_enter(mem3.mutex);
    mem3.alarmBusy = 0;
  }
}


/*
** Chunk i is a free chunk that has been unlinked.  Adjust its 
** size parameters for check-out and return a pointer to the 
** user portion of the chunk.
*/
స్థిర శూన్య *memsys3Checkout(u32 i, u32 n_ఖండం){
  u32 x;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ప్రతిజ్ఞా( i>=1 );
  ప్రతిజ్ఞా( mem3.aPool[i-1].u.hdr.size4x/4==n_ఖండం );
  ప్రతిజ్ఞా( mem3.aPool[i+n_ఖండం-1].u.hdr.prev_కొలత==n_ఖండం );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = n_ఖండం*4 | 1 | (x&2);
  mem3.aPool[i+n_ఖండం-1].u.hdr.prev_కొలత = n_ఖండం;
  mem3.aPool[i+n_ఖండం-1].u.hdr.size4x |= 2;
  తిరిగి &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
స్థిర శూన్య *memsys3_From_కీలకం_Blk(u32 n_ఖండం){
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ప్రతిజ్ఞా( mem3.sz_కీలకం_Blk>=n_ఖండం );
  ఒకవేళ( n_ఖండం>=mem3.sz_కీలకం_Blk-1 ){
    /* Use the entire key chunk */
    శూన్య *p = memsys3Checkout(mem3.i_కీలకం_Blk, mem3.sz_కీలకం_Blk);
    mem3.i_కీలకం_Blk = 0;
    mem3.sz_కీలకం_Blk = 0;
    mem3.mn_కీలకం_Blk = 0;
    తిరిగి p;
  }లేకపోతే{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_కీలకం_Blk + mem3.sz_కీలకం_Blk - n_ఖండం;
    ప్రతిజ్ఞా( newi > mem3.i_కీలకం_Blk+1 );
    mem3.aPool[mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk-1].u.hdr.prev_కొలత = n_ఖండం;
    mem3.aPool[mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = n_ఖండం*4 + 1;
    mem3.sz_కీలకం_Blk -= n_ఖండం;
    mem3.aPool[newi-1].u.hdr.prev_కొలత = mem3.sz_కీలకం_Blk;
    x = mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x = mem3.sz_కీలకం_Blk*4 | x;
    ఒకవేళ( mem3.sz_కీలకం_Blk < mem3.mn_కీలకం_Blk ){
      mem3.mn_కీలకం_Blk = mem3.sz_కీలకం_Blk;
    }
    తిరిగి (శూన్య*)&mem3.aPool[newi];
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
స్థిర శూన్య memsys3_విలీనం(u32 *p_మూలం){
  u32 i_తదుపరి, prev, కొలత, i, x;

  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  కొరకు(i=*p_మూలం; i>0; i=i_తదుపరి){
    i_తదుపరి = mem3.aPool[i].u.list.తదుపరి;
    కొలత = mem3.aPool[i-1].u.hdr.size4x;
    ప్రతిజ్ఞా( (కొలత&1)==0 );
    ఒకవేళ( (కొలత&2)==0 ){
      memsys3UnlinkFromList(i, p_మూలం);
      ప్రతిజ్ఞా( i > mem3.aPool[i-1].u.hdr.prev_కొలత );
      prev = i - mem3.aPool[i-1].u.hdr.prev_కొలత;
      ఒకవేళ( prev==i_తదుపరి ){
        i_తదుపరి = mem3.aPool[prev].u.list.తదుపరి;
      }
      memsys3Unlink(prev);
      కొలత = i + కొలత/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = కొలత*4 | x;
      mem3.aPool[prev+కొలత-1].u.hdr.prev_కొలత = కొలత;
      memsys3Link(prev);
      i = prev;
    }లేకపోతే{
      కొలత /= 4;
    }
    ఒకవేళ( కొలత>mem3.sz_కీలకం_Blk ){
      mem3.i_కీలకం_Blk = i;
      mem3.sz_కీలకం_Blk = కొలత;
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
స్థిర శూన్య *memsys3MallocUnsafe(పూర్ణాంకం nByte){
  u32 i;
  u32 n_ఖండం;
  u32 గమ్యం_Free;

  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ప్రతిజ్ఞా( పరిమాణం(Mem3_ఖండం)==8 );
  ఒకవేళ( nByte<=12 ){
    n_ఖండం = 2;
  }లేకపోతే{
    n_ఖండం = (nByte + 11)/8;
  }
  ప్రతిజ్ఞా( n_ఖండం>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  ఒకవేళ( n_ఖండం <= MX_SMALL ){
    i = mem3.aiSmall[n_ఖండం-2];
    ఒకవేళ( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[n_ఖండం-2]);
      తిరిగి memsys3Checkout(i, n_ఖండం);
    }
  }లేకపోతే{
    పూర్ణాంకం hash = n_ఖండం % N_HASH;
    కొరకు(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.తదుపరి){
      ఒకవేళ( mem3.aPool[i-1].u.hdr.size4x/4==n_ఖండం ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        తిరిగి memsys3Checkout(i, n_ఖండం);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  ఒకవేళ( mem3.sz_కీలకం_Blk>=n_ఖండం ){
    తిరిగి memsys3_From_కీలకం_Blk(n_ఖండం);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  కొరకు(గమ్యం_Free=n_ఖండం*16; గమ్యం_Free<(mem3.nPool*16); గమ్యం_Free *= 2){
    memsys3_బహిర్గతం_యొక్క_జ్ఞాపకం(గమ్యం_Free);
    ఒకవేళ( mem3.i_కీలకం_Blk ){
      memsys3Link(mem3.i_కీలకం_Blk);
      mem3.i_కీలకం_Blk = 0;
      mem3.sz_కీలకం_Blk = 0;
    }
    కొరకు(i=0; i<N_HASH; i++){
      memsys3_విలీనం(&mem3.aiHash[i]);
    }
    కొరకు(i=0; i<MX_SMALL-1; i++){
      memsys3_విలీనం(&mem3.aiSmall[i]);
    }
    ఒకవేళ( mem3.sz_కీలకం_Blk ){
      memsys3Unlink(mem3.i_కీలకం_Blk);
      ఒకవేళ( mem3.sz_కీలకం_Blk>=n_ఖండం ){
        తిరిగి memsys3_From_కీలకం_Blk(n_ఖండం);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  తిరిగి 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
స్థిర శూన్య memsys3FreeUnsafe(శూన్య *pOld){
  Mem3_ఖండం *p = (Mem3_ఖండం*)pOld;
  పూర్ణాంకం i;
  u32 కొలత, x;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem3.mutex) );
  ప్రతిజ్ఞా( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  ప్రతిజ్ఞా( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  కొలత = mem3.aPool[i-1].u.hdr.size4x/4;
  ప్రతిజ్ఞా( i+కొలత<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+కొలత-1].u.hdr.prev_కొలత = కొలత;
  mem3.aPool[i+కొలత-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  ఒకవేళ( mem3.i_కీలకం_Blk ){
    అయితే( (mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x&2)==0 ){
      కొలత = mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.prev_కొలత;
      mem3.i_కీలకం_Blk -= కొలత;
      mem3.sz_కీలకం_Blk += కొలత;
      memsys3Unlink(mem3.i_కీలకం_Blk);
      x = mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x = mem3.sz_కీలకం_Blk*4 | x;
      mem3.aPool[mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk-1].u.hdr.prev_కొలత = mem3.sz_కీలకం_Blk;
    }
    x = mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x & 2;
    అయితే( (mem3.aPool[mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk);
      mem3.sz_కీలకం_Blk += mem3.aPool[mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_కీలకం_Blk-1].u.hdr.size4x = mem3.sz_కీలకం_Blk*4 | x;
      mem3.aPool[mem3.i_కీలకం_Blk+mem3.sz_కీలకం_Blk-1].u.hdr.prev_కొలత = mem3.sz_కీలకం_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
స్థిర పూర్ణాంకం memsys3_కొలత(శూన్య *p){
  Mem3_ఖండం *p_ఖండం;
  ప్రతిజ్ఞా( p!=0 );
  p_ఖండం = (Mem3_ఖండం*)p;
  ప్రతిజ్ఞా( (p_ఖండం[-1].u.hdr.size4x&1)!=0 );
  తిరిగి (p_ఖండం[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
స్థిర పూర్ణాంకం memsys3Roundup(పూర్ణాంకం n){
  ఒకవేళ( n<=12 ){
    తిరిగి 12;
  }లేకపోతే{
    తిరిగి ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
స్థిర శూన్య *memsys3Malloc(పూర్ణాంకం n_బైట్లు){
  sqlite3_int64 *p;
  ప్రతిజ్ఞా( n_బైట్లు>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(n_బైట్లు);
  memsys3Leave();
  తిరిగి (శూన్య*)p; 
}

/*
** Free memory.
*/
స్థిర శూన్య memsys3Free(శూన్య *p_పూర్వ){
  ప్రతిజ్ఞా( p_పూర్వ );
  memsys3Enter();
  memsys3FreeUnsafe(p_పూర్వ);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
స్థిర శూన్య *memsys3Realloc(శూన్య *p_పూర్వ, పూర్ణాంకం n_బైట్లు){
  పూర్ణాంకం nOld;
  శూన్య *p;
  ఒకవేళ( p_పూర్వ==0 ){
    తిరిగి sqlite3_malloc(n_బైట్లు);
  }
  ఒకవేళ( n_బైట్లు<=0 ){
    sqlite3_free(p_పూర్వ);
    తిరిగి 0;
  }
  nOld = memsys3_కొలత(p_పూర్వ);
  ఒకవేళ( n_బైట్లు<=nOld && n_బైట్లు>=nOld-128 ){
    తిరిగి p_పూర్వ;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(n_బైట్లు);
  ఒకవేళ( p ){
    ఒకవేళ( nOld<n_బైట్లు ){
      జ్ఞాపకం_నకలు(p, p_పూర్వ, nOld);
    }లేకపోతే{
      జ్ఞాపకం_నకలు(p, p_పూర్వ, n_బైట్లు);
    }
    memsys3FreeUnsafe(p_పూర్వ);
  }
  memsys3Leave();
  తిరిగి p;
}

/*
** Initialize this module.
*/
స్థిర పూర్ణాంకం memsys3Init(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  ఒకవేళ( !sqlite3GlobalConfig.p_కుప్ప ){
    తిరిగి SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  ప్రతిజ్ఞా( పరిమాణం(Mem3_ఖండం)==8 );
  mem3.aPool = (Mem3_ఖండం *)sqlite3GlobalConfig.p_కుప్ప;
  mem3.nPool = (sqlite3GlobalConfig.n_కుప్ప / పరిమాణం(Mem3_ఖండం)) - 2;

  /* Initialize the key block. */
  mem3.sz_కీలకం_Blk = mem3.nPool;
  mem3.mn_కీలకం_Blk = mem3.sz_కీలకం_Blk;
  mem3.i_కీలకం_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_కీలకం_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prev_కొలత = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  తిరిగి SQLITE_OK;
}

/*
** Deinitialize this module.
*/
స్థిర శూన్య memsys3Shutdown(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  mem3.mutex = 0;
  తిరిగి;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
శూన్య sqlite3Memsys3Dump(స్థిరరాశి అక్షరం *zFilename){
#నిర్వచితమైతే SQLITE_DEBUG
  FILE *బహిర్గతం;
  u32 i, j;
  u32 కొలత;
  ఒకవేళ( zFilename==0 || zFilename[0]==0 ){
    బహిర్గతం = stdout;
  }లేకపోతే{
    బహిర్గతం = fopen(zFilename, "w");
    ఒకవేళ( బహిర్గతం==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      తిరిగి;
    }
  }
  memsys3Enter();
  fprintf(బహిర్గతం, "CHUNKS:\n");
  కొరకు(i=1; i<=mem3.nPool; i+=కొలత/4){
    కొలత = mem3.aPool[i-1].u.hdr.size4x;
    ఒకవేళ( కొలత/4<=1 ){
      fprintf(బహిర్గతం, "%p size error\n", &mem3.aPool[i]);
      ప్రతిజ్ఞా( 0 );
      ఆపు;
    }
    ఒకవేళ( (కొలత&1)==0 && mem3.aPool[i+కొలత/4-1].u.hdr.prev_కొలత!=కొలత/4 ){
      fprintf(బహిర్గతం, "%p tail size does not match\n", &mem3.aPool[i]);
      ప్రతిజ్ఞా( 0 );
      ఆపు;
    }
    ఒకవేళ( ((mem3.aPool[i+కొలత/4-1].u.hdr.size4x&2)>>1)!=(కొలత&1) ){
      fprintf(బహిర్గతం, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      ప్రతిజ్ఞా( 0 );
      ఆపు;
    }
    ఒకవేళ( కొలత&1 ){
      fprintf(బహిర్గతం, "%p %6d bytes checked out\n", &mem3.aPool[i], (కొలత/4)*8-8);
    }లేకపోతే{
      fprintf(బహిర్గతం, "%p %6d bytes free%s\n", &mem3.aPool[i], (కొలత/4)*8-8,
                  i==mem3.i_కీలకం_Blk ? " **key**" : "");
    }
  }
  కొరకు(i=0; i<MX_SMALL-1; i++){
    ఒకవేళ( mem3.aiSmall[i]==0 ) కొనసాగు;
    fprintf(బహిర్గతం, "small(%2d):", i);
    కొరకు(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.తదుపరి){
      fprintf(బహిర్గతం, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(బహిర్గతం, "\n"); 
  }
  కొరకు(i=0; i<N_HASH; i++){
    ఒకవేళ( mem3.aiHash[i]==0 ) కొనసాగు;
    fprintf(బహిర్గతం, "hash(%2d):", i);
    కొరకు(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.తదుపరి){
      fprintf(బహిర్గతం, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(బహిర్గతం, "\n"); 
  }
  fprintf(బహిర్గతం, "key=%d\n", mem3.i_కీలకం_Blk);
  fprintf(బహిర్గతం, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_కీలకం_Blk*8);
  fprintf(బహిర్గతం, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_కీలకం_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  ఒకవేళ( బహిర్గతం==stdout ){
    fflush(stdout);
  }లేకపోతే{
    fclose(బహిర్గతం);
  }
#లేకపోతే
  వాడని_PARAMETER(zFilename);
#అయితే_ముగింపు
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
స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetMemsys3(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_కొలత,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  తిరిగి &mempoolMethods;
}

#అయితే_ముగింపు /* SQLITE_ENABLE_MEMSYS3 */
