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
#ضمن "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#که_تعریف_شوی SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#تعریف_کړه MX_SMALL 10


/*
** Number of freelist hash slots
*/
#تعریف_کړه N_HASH  61

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
تعريفنوع بنية Mem3_بلاک Mem3_بلاک;
بنية Mem3_بلاک {
  اتحاد {
    بنية {
      u32 prev_اندازه;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    بنية {
      u32 بل;       /* Index in mem3.aPool[] of next free chunk */
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
ساكن SQLITE_WSD بنية Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3_بلاک *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  صحيح alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_کلی_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_کلی_Blk;
  u32 sz_کلی_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#تعریف_کړه mem3 GLOBAL(بنية Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
ساكن فراغ memsys3UnlinkFromList(u32 i, u32 *p_ریښه){
  u32 بل = mem3.aPool[i].u.list.بل;
  u32 prev = mem3.aPool[i].u.list.prev;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  إذا( prev==0 ){
    *p_ریښه = بل;
  }وإلا{
    mem3.aPool[prev].u.list.بل = بل;
  }
  إذا( بل ){
    mem3.aPool[بل].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.بل = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
ساكن فراغ memsys3Unlink(u32 i){
  u32 اندازه, hash;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  أكد( i>=1 );
  اندازه = mem3.aPool[i-1].u.hdr.size4x/4;
  أكد( اندازه==mem3.aPool[i+اندازه-1].u.hdr.prev_اندازه );
  أكد( اندازه>=2 );
  إذا( اندازه <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[اندازه-2]);
  }وإلا{
    hash = اندازه % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
ساكن فراغ memsys3LinkIntoList(u32 i, u32 *p_ریښه){
  أكد( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.بل = *p_ریښه;
  mem3.aPool[i].u.list.prev = 0;
  إذا( *p_ریښه ){
    mem3.aPool[*p_ریښه].u.list.prev = i;
  }
  *p_ریښه = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
ساكن فراغ memsys3Link(u32 i){
  u32 اندازه, hash;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( i>=1 );
  أكد( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  اندازه = mem3.aPool[i-1].u.hdr.size4x/4;
  أكد( اندازه==mem3.aPool[i+اندازه-1].u.hdr.prev_اندازه );
  أكد( اندازه>=2 );
  إذا( اندازه <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[اندازه-2]);
  }وإلا{
    hash = اندازه % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
ساكن فراغ memsys3Enter(فراغ){
  إذا( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
ساكن فراغ memsys3Leave(فراغ){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
ساكن فراغ memsys3_بهر_د_حافظه(صحيح nByte){
  إذا( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    أكد( sqlite3_mutex_held(mem3.mutex) );
    sqlite3_mutex_leave(mem3.mutex);
    sqlite3_release_حافظه(nByte);
    sqlite3_mutex_enter(mem3.mutex);
    mem3.alarmBusy = 0;
  }
}


/*
** Chunk i is a free chunk that has been unlinked.  Adjust its 
** size parameters for check-out and return a pointer to the 
** user portion of the chunk.
*/
ساكن فراغ *memsys3Checkout(u32 i, u32 n_بلاک){
  u32 x;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( i>=1 );
  أكد( mem3.aPool[i-1].u.hdr.size4x/4==n_بلاک );
  أكد( mem3.aPool[i+n_بلاک-1].u.hdr.prev_اندازه==n_بلاک );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = n_بلاک*4 | 1 | (x&2);
  mem3.aPool[i+n_بلاک-1].u.hdr.prev_اندازه = n_بلاک;
  mem3.aPool[i+n_بلاک-1].u.hdr.size4x |= 2;
  أرجع &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
ساكن فراغ *memsys3_From_کلی_Blk(u32 n_بلاک){
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( mem3.sz_کلی_Blk>=n_بلاک );
  إذا( n_بلاک>=mem3.sz_کلی_Blk-1 ){
    /* Use the entire key chunk */
    فراغ *p = memsys3Checkout(mem3.i_کلی_Blk, mem3.sz_کلی_Blk);
    mem3.i_کلی_Blk = 0;
    mem3.sz_کلی_Blk = 0;
    mem3.mn_کلی_Blk = 0;
    أرجع p;
  }وإلا{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_کلی_Blk + mem3.sz_کلی_Blk - n_بلاک;
    أكد( newi > mem3.i_کلی_Blk+1 );
    mem3.aPool[mem3.i_کلی_Blk+mem3.sz_کلی_Blk-1].u.hdr.prev_اندازه = n_بلاک;
    mem3.aPool[mem3.i_کلی_Blk+mem3.sz_کلی_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = n_بلاک*4 + 1;
    mem3.sz_کلی_Blk -= n_بلاک;
    mem3.aPool[newi-1].u.hdr.prev_اندازه = mem3.sz_کلی_Blk;
    x = mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x = mem3.sz_کلی_Blk*4 | x;
    إذا( mem3.sz_کلی_Blk < mem3.mn_کلی_Blk ){
      mem3.mn_کلی_Blk = mem3.sz_کلی_Blk;
    }
    أرجع (فراغ*)&mem3.aPool[newi];
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
ساكن فراغ memsys3_یوځای(u32 *p_ریښه){
  u32 i_بل, prev, اندازه, i, x;

  أكد( sqlite3_mutex_held(mem3.mutex) );
  لكل(i=*p_ریښه; i>0; i=i_بل){
    i_بل = mem3.aPool[i].u.list.بل;
    اندازه = mem3.aPool[i-1].u.hdr.size4x;
    أكد( (اندازه&1)==0 );
    إذا( (اندازه&2)==0 ){
      memsys3UnlinkFromList(i, p_ریښه);
      أكد( i > mem3.aPool[i-1].u.hdr.prev_اندازه );
      prev = i - mem3.aPool[i-1].u.hdr.prev_اندازه;
      إذا( prev==i_بل ){
        i_بل = mem3.aPool[prev].u.list.بل;
      }
      memsys3Unlink(prev);
      اندازه = i + اندازه/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = اندازه*4 | x;
      mem3.aPool[prev+اندازه-1].u.hdr.prev_اندازه = اندازه;
      memsys3Link(prev);
      i = prev;
    }وإلا{
      اندازه /= 4;
    }
    إذا( اندازه>mem3.sz_کلی_Blk ){
      mem3.i_کلی_Blk = i;
      mem3.sz_کلی_Blk = اندازه;
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
ساكن فراغ *memsys3MallocUnsafe(صحيح nByte){
  u32 i;
  u32 n_بلاک;
  u32 منزل_Free;

  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( حجم(Mem3_بلاک)==8 );
  إذا( nByte<=12 ){
    n_بلاک = 2;
  }وإلا{
    n_بلاک = (nByte + 11)/8;
  }
  أكد( n_بلاک>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  إذا( n_بلاک <= MX_SMALL ){
    i = mem3.aiSmall[n_بلاک-2];
    إذا( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[n_بلاک-2]);
      أرجع memsys3Checkout(i, n_بلاک);
    }
  }وإلا{
    صحيح hash = n_بلاک % N_HASH;
    لكل(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.بل){
      إذا( mem3.aPool[i-1].u.hdr.size4x/4==n_بلاک ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        أرجع memsys3Checkout(i, n_بلاک);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  إذا( mem3.sz_کلی_Blk>=n_بلاک ){
    أرجع memsys3_From_کلی_Blk(n_بلاک);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  لكل(منزل_Free=n_بلاک*16; منزل_Free<(mem3.nPool*16); منزل_Free *= 2){
    memsys3_بهر_د_حافظه(منزل_Free);
    إذا( mem3.i_کلی_Blk ){
      memsys3Link(mem3.i_کلی_Blk);
      mem3.i_کلی_Blk = 0;
      mem3.sz_کلی_Blk = 0;
    }
    لكل(i=0; i<N_HASH; i++){
      memsys3_یوځای(&mem3.aiHash[i]);
    }
    لكل(i=0; i<MX_SMALL-1; i++){
      memsys3_یوځای(&mem3.aiSmall[i]);
    }
    إذا( mem3.sz_کلی_Blk ){
      memsys3Unlink(mem3.i_کلی_Blk);
      إذا( mem3.sz_کلی_Blk>=n_بلاک ){
        أرجع memsys3_From_کلی_Blk(n_بلاک);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  أرجع 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
ساكن فراغ memsys3FreeUnsafe(فراغ *pOld){
  Mem3_بلاک *p = (Mem3_بلاک*)pOld;
  صحيح i;
  u32 اندازه, x;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  أكد( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  اندازه = mem3.aPool[i-1].u.hdr.size4x/4;
  أكد( i+اندازه<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+اندازه-1].u.hdr.prev_اندازه = اندازه;
  mem3.aPool[i+اندازه-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  إذا( mem3.i_کلی_Blk ){
    طالما( (mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x&2)==0 ){
      اندازه = mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.prev_اندازه;
      mem3.i_کلی_Blk -= اندازه;
      mem3.sz_کلی_Blk += اندازه;
      memsys3Unlink(mem3.i_کلی_Blk);
      x = mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x = mem3.sz_کلی_Blk*4 | x;
      mem3.aPool[mem3.i_کلی_Blk+mem3.sz_کلی_Blk-1].u.hdr.prev_اندازه = mem3.sz_کلی_Blk;
    }
    x = mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x & 2;
    طالما( (mem3.aPool[mem3.i_کلی_Blk+mem3.sz_کلی_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_کلی_Blk+mem3.sz_کلی_Blk);
      mem3.sz_کلی_Blk += mem3.aPool[mem3.i_کلی_Blk+mem3.sz_کلی_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_کلی_Blk-1].u.hdr.size4x = mem3.sz_کلی_Blk*4 | x;
      mem3.aPool[mem3.i_کلی_Blk+mem3.sz_کلی_Blk-1].u.hdr.prev_اندازه = mem3.sz_کلی_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
ساكن صحيح memsys3_اندازه(فراغ *p){
  Mem3_بلاک *p_بلاک;
  أكد( p!=0 );
  p_بلاک = (Mem3_بلاک*)p;
  أكد( (p_بلاک[-1].u.hdr.size4x&1)!=0 );
  أرجع (p_بلاک[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساكن صحيح memsys3Roundup(صحيح n){
  إذا( n<=12 ){
    أرجع 12;
  }وإلا{
    أرجع ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
ساكن فراغ *memsys3Malloc(صحيح n_بایټونه){
  sqlite3_int64 *p;
  أكد( n_بایټونه>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(n_بایټونه);
  memsys3Leave();
  أرجع (فراغ*)p; 
}

/*
** Free memory.
*/
ساكن فراغ memsys3Free(فراغ *p_مخکینی){
  أكد( p_مخکینی );
  memsys3Enter();
  memsys3FreeUnsafe(p_مخکینی);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
ساكن فراغ *memsys3Realloc(فراغ *p_مخکینی, صحيح n_بایټونه){
  صحيح nOld;
  فراغ *p;
  إذا( p_مخکینی==0 ){
    أرجع sqlite3_malloc(n_بایټونه);
  }
  إذا( n_بایټونه<=0 ){
    sqlite3_free(p_مخکینی);
    أرجع 0;
  }
  nOld = memsys3_اندازه(p_مخکینی);
  إذا( n_بایټونه<=nOld && n_بایټونه>=nOld-128 ){
    أرجع p_مخکینی;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(n_بایټونه);
  إذا( p ){
    إذا( nOld<n_بایټونه ){
      حافظه_نقل(p, p_مخکینی, nOld);
    }وإلا{
      حافظه_نقل(p, p_مخکینی, n_بایټونه);
    }
    memsys3FreeUnsafe(p_مخکینی);
  }
  memsys3Leave();
  أرجع p;
}

/*
** Initialize this module.
*/
ساكن صحيح memsys3Init(فراغ *Not_کارول_شوی){
  ناکارول_شوی_PARAMETER(Not_کارول_شوی);
  إذا( !sqlite3GlobalConfig.p_ډېرۍ ){
    أرجع SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  أكد( حجم(Mem3_بلاک)==8 );
  mem3.aPool = (Mem3_بلاک *)sqlite3GlobalConfig.p_ډېرۍ;
  mem3.nPool = (sqlite3GlobalConfig.n_ډېرۍ / حجم(Mem3_بلاک)) - 2;

  /* Initialize the key block. */
  mem3.sz_کلی_Blk = mem3.nPool;
  mem3.mn_کلی_Blk = mem3.sz_کلی_Blk;
  mem3.i_کلی_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_کلی_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prev_اندازه = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  أرجع SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساكن فراغ memsys3Shutdown(فراغ *Not_کارول_شوی){
  ناکارول_شوی_PARAMETER(Not_کارول_شوی);
  mem3.mutex = 0;
  أرجع;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
فراغ sqlite3Memsys3Dump(ثابت حرف *zFilename){
#که_تعریف_شوی SQLITE_DEBUG
  FILE *بهر;
  u32 i, j;
  u32 اندازه;
  إذا( zFilename==0 || zFilename[0]==0 ){
    بهر = stdout;
  }وإلا{
    بهر = fopen(zFilename, "w");
    إذا( بهر==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      أرجع;
    }
  }
  memsys3Enter();
  fprintf(بهر, "CHUNKS:\n");
  لكل(i=1; i<=mem3.nPool; i+=اندازه/4){
    اندازه = mem3.aPool[i-1].u.hdr.size4x;
    إذا( اندازه/4<=1 ){
      fprintf(بهر, "%p size error\n", &mem3.aPool[i]);
      أكد( 0 );
      اكسر;
    }
    إذا( (اندازه&1)==0 && mem3.aPool[i+اندازه/4-1].u.hdr.prev_اندازه!=اندازه/4 ){
      fprintf(بهر, "%p tail size does not match\n", &mem3.aPool[i]);
      أكد( 0 );
      اكسر;
    }
    إذا( ((mem3.aPool[i+اندازه/4-1].u.hdr.size4x&2)>>1)!=(اندازه&1) ){
      fprintf(بهر, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      أكد( 0 );
      اكسر;
    }
    إذا( اندازه&1 ){
      fprintf(بهر, "%p %6d bytes checked out\n", &mem3.aPool[i], (اندازه/4)*8-8);
    }وإلا{
      fprintf(بهر, "%p %6d bytes free%s\n", &mem3.aPool[i], (اندازه/4)*8-8,
                  i==mem3.i_کلی_Blk ? " **key**" : "");
    }
  }
  لكل(i=0; i<MX_SMALL-1; i++){
    إذا( mem3.aiSmall[i]==0 ) استمر;
    fprintf(بهر, "small(%2d):", i);
    لكل(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.بل){
      fprintf(بهر, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(بهر, "\n"); 
  }
  لكل(i=0; i<N_HASH; i++){
    إذا( mem3.aiHash[i]==0 ) استمر;
    fprintf(بهر, "hash(%2d):", i);
    لكل(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.بل){
      fprintf(بهر, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(بهر, "\n"); 
  }
  fprintf(بهر, "key=%d\n", mem3.i_کلی_Blk);
  fprintf(بهر, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_کلی_Blk*8);
  fprintf(بهر, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_کلی_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  إذا( بهر==stdout ){
    fflush(stdout);
  }وإلا{
    fclose(بهر);
  }
#وإلا
  ناکارول_شوی_PARAMETER(zFilename);
#د_که_پای
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
ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(فراغ){
  ساكن ثابت sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_اندازه,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  أرجع &mempoolMethods;
}

#د_که_پای /* SQLITE_ENABLE_MEMSYS3 */
