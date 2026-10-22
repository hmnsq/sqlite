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
#समावेश "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#अगर_घोषित SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#घोषणा MX_SMALL 10


/*
** Number of freelist hash slots
*/
#घोषणा N_HASH  61

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
प्रकार काष्ठा Mem3Block Mem3Block;
काष्ठा Mem3Block {
  जोड़ {
    काष्ठा {
      u32 पूर्व_आकार;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    काष्ठा {
      u32 अग्र;       /* Index in mem3.aPool[] of next free chunk */
      u32 पूर्व;       /* Index in mem3.aPool[] of previous free chunk */
    } list;
  } u;
};

/*
** All of the static variables used by this module are collected
** into a single structure named "mem3".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
अटल SQLITE_WSD काष्ठा Mem3Global {
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
  u32 mn_कुंजी_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_कुंजी_Blk;
  u32 sz_कुंजी_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#घोषणा mem3 GLOBAL(काष्ठा Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
अटल व्योम memsys3UnlinkFromList(u32 i, u32 *p_मूल){
  u32 अग्र = mem3.aPool[i].u.list.अग्र;
  u32 पूर्व = mem3.aPool[i].u.list.पूर्व;
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  अगर( पूर्व==0 ){
    *p_मूल = अग्र;
  }अन्यथा{
    mem3.aPool[पूर्व].u.list.अग्र = अग्र;
  }
  अगर( अग्र ){
    mem3.aPool[अग्र].u.list.पूर्व = पूर्व;
  }
  mem3.aPool[i].u.list.अग्र = 0;
  mem3.aPool[i].u.list.पूर्व = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
अटल व्योम memsys3Unlink(u32 i){
  u32 आकार, hash;
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  निश्चित( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  निश्चित( i>=1 );
  आकार = mem3.aPool[i-1].u.hdr.size4x/4;
  निश्चित( आकार==mem3.aPool[i+आकार-1].u.hdr.पूर्व_आकार );
  निश्चित( आकार>=2 );
  अगर( आकार <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[आकार-2]);
  }अन्यथा{
    hash = आकार % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
अटल व्योम memsys3LinkIntoList(u32 i, u32 *p_मूल){
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.अग्र = *p_मूल;
  mem3.aPool[i].u.list.पूर्व = 0;
  अगर( *p_मूल ){
    mem3.aPool[*p_मूल].u.list.पूर्व = i;
  }
  *p_मूल = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
अटल व्योम memsys3Link(u32 i){
  u32 आकार, hash;
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  निश्चित( i>=1 );
  निश्चित( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  आकार = mem3.aPool[i-1].u.hdr.size4x/4;
  निश्चित( आकार==mem3.aPool[i+आकार-1].u.hdr.पूर्व_आकार );
  निश्चित( आकार>=2 );
  अगर( आकार <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[आकार-2]);
  }अन्यथा{
    hash = आकार % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
अटल व्योम memsys3Enter(व्योम){
  अगर( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
अटल व्योम memsys3Leave(व्योम){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
अटल व्योम memsys3OutOfMemory(पूर्णांक nByte){
  अगर( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    निश्चित( sqlite3_mutex_held(mem3.mutex) );
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
अटल व्योम *memsys3Checkout(u32 i, u32 nBlock){
  u32 x;
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  निश्चित( i>=1 );
  निश्चित( mem3.aPool[i-1].u.hdr.size4x/4==nBlock );
  निश्चित( mem3.aPool[i+nBlock-1].u.hdr.पूर्व_आकार==nBlock );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = nBlock*4 | 1 | (x&2);
  mem3.aPool[i+nBlock-1].u.hdr.पूर्व_आकार = nBlock;
  mem3.aPool[i+nBlock-1].u.hdr.size4x |= 2;
  वापस &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
अटल व्योम *memsys3_From_कुंजी_Blk(u32 nBlock){
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  निश्चित( mem3.sz_कुंजी_Blk>=nBlock );
  अगर( nBlock>=mem3.sz_कुंजी_Blk-1 ){
    /* Use the entire key chunk */
    व्योम *p = memsys3Checkout(mem3.i_कुंजी_Blk, mem3.sz_कुंजी_Blk);
    mem3.i_कुंजी_Blk = 0;
    mem3.sz_कुंजी_Blk = 0;
    mem3.mn_कुंजी_Blk = 0;
    वापस p;
  }अन्यथा{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_कुंजी_Blk + mem3.sz_कुंजी_Blk - nBlock;
    निश्चित( newi > mem3.i_कुंजी_Blk+1 );
    mem3.aPool[mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk-1].u.hdr.पूर्व_आकार = nBlock;
    mem3.aPool[mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = nBlock*4 + 1;
    mem3.sz_कुंजी_Blk -= nBlock;
    mem3.aPool[newi-1].u.hdr.पूर्व_आकार = mem3.sz_कुंजी_Blk;
    x = mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x = mem3.sz_कुंजी_Blk*4 | x;
    अगर( mem3.sz_कुंजी_Blk < mem3.mn_कुंजी_Blk ){
      mem3.mn_कुंजी_Blk = mem3.sz_कुंजी_Blk;
    }
    वापस (व्योम*)&mem3.aPool[newi];
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
अटल व्योम memsys3Merge(u32 *p_मूल){
  u32 i_अग्र, पूर्व, आकार, i, x;

  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  क्रम(i=*p_मूल; i>0; i=i_अग्र){
    i_अग्र = mem3.aPool[i].u.list.अग्र;
    आकार = mem3.aPool[i-1].u.hdr.size4x;
    निश्चित( (आकार&1)==0 );
    अगर( (आकार&2)==0 ){
      memsys3UnlinkFromList(i, p_मूल);
      निश्चित( i > mem3.aPool[i-1].u.hdr.पूर्व_आकार );
      पूर्व = i - mem3.aPool[i-1].u.hdr.पूर्व_आकार;
      अगर( पूर्व==i_अग्र ){
        i_अग्र = mem3.aPool[पूर्व].u.list.अग्र;
      }
      memsys3Unlink(पूर्व);
      आकार = i + आकार/4 - पूर्व;
      x = mem3.aPool[पूर्व-1].u.hdr.size4x & 2;
      mem3.aPool[पूर्व-1].u.hdr.size4x = आकार*4 | x;
      mem3.aPool[पूर्व+आकार-1].u.hdr.पूर्व_आकार = आकार;
      memsys3Link(पूर्व);
      i = पूर्व;
    }अन्यथा{
      आकार /= 4;
    }
    अगर( आकार>mem3.sz_कुंजी_Blk ){
      mem3.i_कुंजी_Blk = i;
      mem3.sz_कुंजी_Blk = आकार;
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
अटल व्योम *memsys3MallocUnsafe(पूर्णांक nByte){
  u32 i;
  u32 nBlock;
  u32 toFree;

  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  निश्चित( माप(Mem3Block)==8 );
  अगर( nByte<=12 ){
    nBlock = 2;
  }अन्यथा{
    nBlock = (nByte + 11)/8;
  }
  निश्चित( nBlock>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  अगर( nBlock <= MX_SMALL ){
    i = mem3.aiSmall[nBlock-2];
    अगर( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[nBlock-2]);
      वापस memsys3Checkout(i, nBlock);
    }
  }अन्यथा{
    पूर्णांक hash = nBlock % N_HASH;
    क्रम(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.अग्र){
      अगर( mem3.aPool[i-1].u.hdr.size4x/4==nBlock ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        वापस memsys3Checkout(i, nBlock);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  अगर( mem3.sz_कुंजी_Blk>=nBlock ){
    वापस memsys3_From_कुंजी_Blk(nBlock);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  क्रम(toFree=nBlock*16; toFree<(mem3.nPool*16); toFree *= 2){
    memsys3OutOfMemory(toFree);
    अगर( mem3.i_कुंजी_Blk ){
      memsys3Link(mem3.i_कुंजी_Blk);
      mem3.i_कुंजी_Blk = 0;
      mem3.sz_कुंजी_Blk = 0;
    }
    क्रम(i=0; i<N_HASH; i++){
      memsys3Merge(&mem3.aiHash[i]);
    }
    क्रम(i=0; i<MX_SMALL-1; i++){
      memsys3Merge(&mem3.aiSmall[i]);
    }
    अगर( mem3.sz_कुंजी_Blk ){
      memsys3Unlink(mem3.i_कुंजी_Blk);
      अगर( mem3.sz_कुंजी_Blk>=nBlock ){
        वापस memsys3_From_कुंजी_Blk(nBlock);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  वापस 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
अटल व्योम memsys3FreeUnsafe(व्योम *pOld){
  Mem3Block *p = (Mem3Block*)pOld;
  पूर्णांक i;
  u32 आकार, x;
  निश्चित( sqlite3_mutex_held(mem3.mutex) );
  निश्चित( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  निश्चित( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  आकार = mem3.aPool[i-1].u.hdr.size4x/4;
  निश्चित( i+आकार<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+आकार-1].u.hdr.पूर्व_आकार = आकार;
  mem3.aPool[i+आकार-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  अगर( mem3.i_कुंजी_Blk ){
    जबतक( (mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x&2)==0 ){
      आकार = mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.पूर्व_आकार;
      mem3.i_कुंजी_Blk -= आकार;
      mem3.sz_कुंजी_Blk += आकार;
      memsys3Unlink(mem3.i_कुंजी_Blk);
      x = mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x = mem3.sz_कुंजी_Blk*4 | x;
      mem3.aPool[mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk-1].u.hdr.पूर्व_आकार = mem3.sz_कुंजी_Blk;
    }
    x = mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x & 2;
    जबतक( (mem3.aPool[mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk);
      mem3.sz_कुंजी_Blk += mem3.aPool[mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_कुंजी_Blk-1].u.hdr.size4x = mem3.sz_कुंजी_Blk*4 | x;
      mem3.aPool[mem3.i_कुंजी_Blk+mem3.sz_कुंजी_Blk-1].u.hdr.पूर्व_आकार = mem3.sz_कुंजी_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
अटल पूर्णांक memsys3_आकार(व्योम *p){
  Mem3Block *pBlock;
  निश्चित( p!=0 );
  pBlock = (Mem3Block*)p;
  निश्चित( (pBlock[-1].u.hdr.size4x&1)!=0 );
  वापस (pBlock[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
अटल पूर्णांक memsys3Roundup(पूर्णांक n){
  अगर( n<=12 ){
    वापस 12;
  }अन्यथा{
    वापस ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
अटल व्योम *memsys3Malloc(पूर्णांक nBytes){
  sqlite3_int64 *p;
  निश्चित( nBytes>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  memsys3Leave();
  वापस (व्योम*)p; 
}

/*
** Free memory.
*/
अटल व्योम memsys3Free(व्योम *pPrior){
  निश्चित( pPrior );
  memsys3Enter();
  memsys3FreeUnsafe(pPrior);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
अटल व्योम *memsys3Realloc(व्योम *pPrior, पूर्णांक nBytes){
  पूर्णांक nOld;
  व्योम *p;
  अगर( pPrior==0 ){
    वापस sqlite3_malloc(nBytes);
  }
  अगर( nBytes<=0 ){
    sqlite3_free(pPrior);
    वापस 0;
  }
  nOld = memsys3_आकार(pPrior);
  अगर( nBytes<=nOld && nBytes>=nOld-128 ){
    वापस pPrior;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  अगर( p ){
    अगर( nOld<nBytes ){
      स_नकल(p, pPrior, nOld);
    }अन्यथा{
      स_नकल(p, pPrior, nBytes);
    }
    memsys3FreeUnsafe(pPrior);
  }
  memsys3Leave();
  वापस p;
}

/*
** Initialize this module.
*/
अटल पूर्णांक memsys3Init(व्योम *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  अगर( !sqlite3GlobalConfig.pHeap ){
    वापस SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  निश्चित( माप(Mem3Block)==8 );
  mem3.aPool = (Mem3Block *)sqlite3GlobalConfig.pHeap;
  mem3.nPool = (sqlite3GlobalConfig.nHeap / माप(Mem3Block)) - 2;

  /* Initialize the key block. */
  mem3.sz_कुंजी_Blk = mem3.nPool;
  mem3.mn_कुंजी_Blk = mem3.sz_कुंजी_Blk;
  mem3.i_कुंजी_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_कुंजी_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.पूर्व_आकार = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  वापस SQLITE_OK;
}

/*
** Deinitialize this module.
*/
अटल व्योम memsys3Shutdown(व्योम *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem3.mutex = 0;
  वापस;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
व्योम sqlite3Memsys3Dump(स्थिर अक्षर *zFilename){
#अगर_घोषित SQLITE_DEBUG
  खाता *out;
  u32 i, j;
  u32 आकार;
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
  memsys3Enter();
  ख_लिखो(out, "CHUNKS:\n");
  क्रम(i=1; i<=mem3.nPool; i+=आकार/4){
    आकार = mem3.aPool[i-1].u.hdr.size4x;
    अगर( आकार/4<=1 ){
      ख_लिखो(out, "%p size error\n", &mem3.aPool[i]);
      निश्चित( 0 );
      अवरोध;
    }
    अगर( (आकार&1)==0 && mem3.aPool[i+आकार/4-1].u.hdr.पूर्व_आकार!=आकार/4 ){
      ख_लिखो(out, "%p tail size does not match\n", &mem3.aPool[i]);
      निश्चित( 0 );
      अवरोध;
    }
    अगर( ((mem3.aPool[i+आकार/4-1].u.hdr.size4x&2)>>1)!=(आकार&1) ){
      ख_लिखो(out, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      निश्चित( 0 );
      अवरोध;
    }
    अगर( आकार&1 ){
      ख_लिखो(out, "%p %6d bytes checked out\n", &mem3.aPool[i], (आकार/4)*8-8);
    }अन्यथा{
      ख_लिखो(out, "%p %6d bytes free%s\n", &mem3.aPool[i], (आकार/4)*8-8,
                  i==mem3.i_कुंजी_Blk ? " **key**" : "");
    }
  }
  क्रम(i=0; i<MX_SMALL-1; i++){
    अगर( mem3.aiSmall[i]==0 ) जारी;
    ख_लिखो(out, "small(%2d):", i);
    क्रम(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.अग्र){
      ख_लिखो(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    ख_लिखो(out, "\n"); 
  }
  क्रम(i=0; i<N_HASH; i++){
    अगर( mem3.aiHash[i]==0 ) जारी;
    ख_लिखो(out, "hash(%2d):", i);
    क्रम(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.अग्र){
      ख_लिखो(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    ख_लिखो(out, "\n"); 
  }
  ख_लिखो(out, "key=%d\n", mem3.i_कुंजी_Blk);
  ख_लिखो(out, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_कुंजी_Blk*8);
  ख_लिखो(out, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_कुंजी_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  अगर( out==मानक_निकास ){
    ख_साफ(मानक_निकास);
  }अन्यथा{
    ख_बंद(out);
  }
#अन्यथा
  UNUSED_PARAMETER(zFilename);
#पूर्ण_अगर
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
स्थिर sqlite3_mem_methods *sqlite3MemGetMemsys3(व्योम){
  अटल स्थिर sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_आकार,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  वापस &mempoolMethods;
}

#पूर्ण_अगर /* SQLITE_ENABLE_MEMSYS3 */
