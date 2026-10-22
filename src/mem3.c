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
#شامل "sqliteInt.h"

/*
** This version of the memory allocator is only built into the library
** SQLITE_ENABLE_MEMSYS3 is defined. Defining this symbol does not
** mean that the library will use a memory-pool by default, just that
** it is available. The mempool allocator is activated by calling
** sqlite3_config().
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS3

/*
** Maximum size (in Mem3Blocks) of a "small" chunk.
*/
#تعریف_کرو MX_SMALL 10


/*
** Number of freelist hash slots
*/
#تعریف_کرو N_HASH  61

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
قسمتعریف ساخت Mem3_بلاک Mem3_بلاک;
ساخت Mem3_بلاک {
  اتحاد {
    ساخت {
      u32 پچھلا_سائز;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    ساخت {
      u32 اگلا;       /* Index in mem3.aPool[] of next free chunk */
      u32 پچھلا;       /* Index in mem3.aPool[] of previous free chunk */
    } list;
  } u;
};

/*
** All of the static variables used by this module are collected
** into a single structure named "mem3".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
ساکن SQLITE_WSD ساخت Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3_بلاک *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  صحیح alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mn_کلید_Blk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 i_کلید_Blk;
  u32 sz_کلید_Blk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#تعریف_کرو mem3 GLOBAL(ساخت Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
ساکن خالی memsys3UnlinkFromList(u32 i, u32 *p_جڑ){
  u32 اگلا = mem3.aPool[i].u.list.اگلا;
  u32 پچھلا = mem3.aPool[i].u.list.پچھلا;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  اگر( پچھلا==0 ){
    *p_جڑ = اگلا;
  }ورنہ{
    mem3.aPool[پچھلا].u.list.اگلا = اگلا;
  }
  اگر( اگلا ){
    mem3.aPool[اگلا].u.list.پچھلا = پچھلا;
  }
  mem3.aPool[i].u.list.اگلا = 0;
  mem3.aPool[i].u.list.پچھلا = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
ساکن خالی memsys3Unlink(u32 i){
  u32 سائز, hash;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  أكد( i>=1 );
  سائز = mem3.aPool[i-1].u.hdr.size4x/4;
  أكد( سائز==mem3.aPool[i+سائز-1].u.hdr.پچھلا_سائز );
  أكد( سائز>=2 );
  اگر( سائز <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[سائز-2]);
  }ورنہ{
    hash = سائز % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
ساکن خالی memsys3LinkIntoList(u32 i, u32 *p_جڑ){
  أكد( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.اگلا = *p_جڑ;
  mem3.aPool[i].u.list.پچھلا = 0;
  اگر( *p_جڑ ){
    mem3.aPool[*p_جڑ].u.list.پچھلا = i;
  }
  *p_جڑ = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
ساکن خالی memsys3Link(u32 i){
  u32 سائز, hash;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( i>=1 );
  أكد( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  سائز = mem3.aPool[i-1].u.hdr.size4x/4;
  أكد( سائز==mem3.aPool[i+سائز-1].u.hdr.پچھلا_سائز );
  أكد( سائز>=2 );
  اگر( سائز <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[سائز-2]);
  }ورنہ{
    hash = سائز % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
ساکن خالی memsys3Enter(خالی){
  اگر( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
ساکن خالی memsys3Leave(خالی){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
ساکن خالی memsys3_برآمد_کا_حافظہ(صحیح nByte){
  اگر( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    أكد( sqlite3_mutex_held(mem3.mutex) );
    sqlite3_mutex_leave(mem3.mutex);
    sqlite3_release_حافظہ(nByte);
    sqlite3_mutex_enter(mem3.mutex);
    mem3.alarmBusy = 0;
  }
}


/*
** Chunk i is a free chunk that has been unlinked.  Adjust its 
** size parameters for check-out and return a pointer to the 
** user portion of the chunk.
*/
ساکن خالی *memsys3Checkout(u32 i, u32 n_بلاک){
  u32 x;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( i>=1 );
  أكد( mem3.aPool[i-1].u.hdr.size4x/4==n_بلاک );
  أكد( mem3.aPool[i+n_بلاک-1].u.hdr.پچھلا_سائز==n_بلاک );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = n_بلاک*4 | 1 | (x&2);
  mem3.aPool[i+n_بلاک-1].u.hdr.پچھلا_سائز = n_بلاک;
  mem3.aPool[i+n_بلاک-1].u.hdr.size4x |= 2;
  واپس &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
ساکن خالی *memsys3_From_کلید_Blk(u32 n_بلاک){
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( mem3.sz_کلید_Blk>=n_بلاک );
  اگر( n_بلاک>=mem3.sz_کلید_Blk-1 ){
    /* Use the entire key chunk */
    خالی *p = memsys3Checkout(mem3.i_کلید_Blk, mem3.sz_کلید_Blk);
    mem3.i_کلید_Blk = 0;
    mem3.sz_کلید_Blk = 0;
    mem3.mn_کلید_Blk = 0;
    واپس p;
  }ورنہ{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.i_کلید_Blk + mem3.sz_کلید_Blk - n_بلاک;
    أكد( newi > mem3.i_کلید_Blk+1 );
    mem3.aPool[mem3.i_کلید_Blk+mem3.sz_کلید_Blk-1].u.hdr.پچھلا_سائز = n_بلاک;
    mem3.aPool[mem3.i_کلید_Blk+mem3.sz_کلید_Blk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = n_بلاک*4 + 1;
    mem3.sz_کلید_Blk -= n_بلاک;
    mem3.aPool[newi-1].u.hdr.پچھلا_سائز = mem3.sz_کلید_Blk;
    x = mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x = mem3.sz_کلید_Blk*4 | x;
    اگر( mem3.sz_کلید_Blk < mem3.mn_کلید_Blk ){
      mem3.mn_کلید_Blk = mem3.sz_کلید_Blk;
    }
    واپس (خالی*)&mem3.aPool[newi];
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
ساکن خالی memsys3_ملاؤ(u32 *p_جڑ){
  u32 i_اگلا, پچھلا, سائز, i, x;

  أكد( sqlite3_mutex_held(mem3.mutex) );
  برائے(i=*p_جڑ; i>0; i=i_اگلا){
    i_اگلا = mem3.aPool[i].u.list.اگلا;
    سائز = mem3.aPool[i-1].u.hdr.size4x;
    أكد( (سائز&1)==0 );
    اگر( (سائز&2)==0 ){
      memsys3UnlinkFromList(i, p_جڑ);
      أكد( i > mem3.aPool[i-1].u.hdr.پچھلا_سائز );
      پچھلا = i - mem3.aPool[i-1].u.hdr.پچھلا_سائز;
      اگر( پچھلا==i_اگلا ){
        i_اگلا = mem3.aPool[پچھلا].u.list.اگلا;
      }
      memsys3Unlink(پچھلا);
      سائز = i + سائز/4 - پچھلا;
      x = mem3.aPool[پچھلا-1].u.hdr.size4x & 2;
      mem3.aPool[پچھلا-1].u.hdr.size4x = سائز*4 | x;
      mem3.aPool[پچھلا+سائز-1].u.hdr.پچھلا_سائز = سائز;
      memsys3Link(پچھلا);
      i = پچھلا;
    }ورنہ{
      سائز /= 4;
    }
    اگر( سائز>mem3.sz_کلید_Blk ){
      mem3.i_کلید_Blk = i;
      mem3.sz_کلید_Blk = سائز;
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
ساکن خالی *memsys3MallocUnsafe(صحیح nByte){
  u32 i;
  u32 n_بلاک;
  u32 مقصد_Free;

  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( حجم(Mem3_بلاک)==8 );
  اگر( nByte<=12 ){
    n_بلاک = 2;
  }ورنہ{
    n_بلاک = (nByte + 11)/8;
  }
  أكد( n_بلاک>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  اگر( n_بلاک <= MX_SMALL ){
    i = mem3.aiSmall[n_بلاک-2];
    اگر( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[n_بلاک-2]);
      واپس memsys3Checkout(i, n_بلاک);
    }
  }ورنہ{
    صحیح hash = n_بلاک % N_HASH;
    برائے(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.اگلا){
      اگر( mem3.aPool[i-1].u.hdr.size4x/4==n_بلاک ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        واپس memsys3Checkout(i, n_بلاک);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  اگر( mem3.sz_کلید_Blk>=n_بلاک ){
    واپس memsys3_From_کلید_Blk(n_بلاک);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  برائے(مقصد_Free=n_بلاک*16; مقصد_Free<(mem3.nPool*16); مقصد_Free *= 2){
    memsys3_برآمد_کا_حافظہ(مقصد_Free);
    اگر( mem3.i_کلید_Blk ){
      memsys3Link(mem3.i_کلید_Blk);
      mem3.i_کلید_Blk = 0;
      mem3.sz_کلید_Blk = 0;
    }
    برائے(i=0; i<N_HASH; i++){
      memsys3_ملاؤ(&mem3.aiHash[i]);
    }
    برائے(i=0; i<MX_SMALL-1; i++){
      memsys3_ملاؤ(&mem3.aiSmall[i]);
    }
    اگر( mem3.sz_کلید_Blk ){
      memsys3Unlink(mem3.i_کلید_Blk);
      اگر( mem3.sz_کلید_Blk>=n_بلاک ){
        واپس memsys3_From_کلید_Blk(n_بلاک);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  واپس 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
ساکن خالی memsys3FreeUnsafe(خالی *pOld){
  Mem3_بلاک *p = (Mem3_بلاک*)pOld;
  صحیح i;
  u32 سائز, x;
  أكد( sqlite3_mutex_held(mem3.mutex) );
  أكد( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  أكد( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  سائز = mem3.aPool[i-1].u.hdr.size4x/4;
  أكد( i+سائز<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+سائز-1].u.hdr.پچھلا_سائز = سائز;
  mem3.aPool[i+سائز-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  اگر( mem3.i_کلید_Blk ){
    جب( (mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x&2)==0 ){
      سائز = mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.پچھلا_سائز;
      mem3.i_کلید_Blk -= سائز;
      mem3.sz_کلید_Blk += سائز;
      memsys3Unlink(mem3.i_کلید_Blk);
      x = mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x = mem3.sz_کلید_Blk*4 | x;
      mem3.aPool[mem3.i_کلید_Blk+mem3.sz_کلید_Blk-1].u.hdr.پچھلا_سائز = mem3.sz_کلید_Blk;
    }
    x = mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x & 2;
    جب( (mem3.aPool[mem3.i_کلید_Blk+mem3.sz_کلید_Blk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.i_کلید_Blk+mem3.sz_کلید_Blk);
      mem3.sz_کلید_Blk += mem3.aPool[mem3.i_کلید_Blk+mem3.sz_کلید_Blk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.i_کلید_Blk-1].u.hdr.size4x = mem3.sz_کلید_Blk*4 | x;
      mem3.aPool[mem3.i_کلید_Blk+mem3.sz_کلید_Blk-1].u.hdr.پچھلا_سائز = mem3.sz_کلید_Blk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
ساکن صحیح memsys3_سائز(خالی *p){
  Mem3_بلاک *p_بلاک;
  أكد( p!=0 );
  p_بلاک = (Mem3_بلاک*)p;
  أكد( (p_بلاک[-1].u.hdr.size4x&1)!=0 );
  واپس (p_بلاک[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساکن صحیح memsys3Roundup(صحیح n){
  اگر( n<=12 ){
    واپس 12;
  }ورنہ{
    واپس ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
ساکن خالی *memsys3Malloc(صحیح n_بائٹیں){
  sqlite3_int64 *p;
  أكد( n_بائٹیں>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(n_بائٹیں);
  memsys3Leave();
  واپس (خالی*)p; 
}

/*
** Free memory.
*/
ساکن خالی memsys3Free(خالی *p_پیشگی){
  أكد( p_پیشگی );
  memsys3Enter();
  memsys3FreeUnsafe(p_پیشگی);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
ساکن خالی *memsys3Realloc(خالی *p_پیشگی, صحیح n_بائٹیں){
  صحیح nOld;
  خالی *p;
  اگر( p_پیشگی==0 ){
    واپس sqlite3_malloc(n_بائٹیں);
  }
  اگر( n_بائٹیں<=0 ){
    sqlite3_free(p_پیشگی);
    واپس 0;
  }
  nOld = memsys3_سائز(p_پیشگی);
  اگر( n_بائٹیں<=nOld && n_بائٹیں>=nOld-128 ){
    واپس p_پیشگی;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(n_بائٹیں);
  اگر( p ){
    اگر( nOld<n_بائٹیں ){
      حافظہ_نقل(p, p_پیشگی, nOld);
    }ورنہ{
      حافظہ_نقل(p, p_پیشگی, n_بائٹیں);
    }
    memsys3FreeUnsafe(p_پیشگی);
  }
  memsys3Leave();
  واپس p;
}

/*
** Initialize this module.
*/
ساکن صحیح memsys3Init(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  اگر( !sqlite3GlobalConfig.p_ڈھیر ){
    واپس SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  أكد( حجم(Mem3_بلاک)==8 );
  mem3.aPool = (Mem3_بلاک *)sqlite3GlobalConfig.p_ڈھیر;
  mem3.nPool = (sqlite3GlobalConfig.n_ڈھیر / حجم(Mem3_بلاک)) - 2;

  /* Initialize the key block. */
  mem3.sz_کلید_Blk = mem3.nPool;
  mem3.mn_کلید_Blk = mem3.sz_کلید_Blk;
  mem3.i_کلید_Blk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.sz_کلید_Blk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.پچھلا_سائز = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  واپس SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساکن خالی memsys3Shutdown(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  mem3.mutex = 0;
  واپس;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
خالی sqlite3Memsys3Dump(ثابت حرف *zFilename){
#اگر_تعریف_شدہ SQLITE_DEBUG
  FILE *برآمد;
  u32 i, j;
  u32 سائز;
  اگر( zFilename==0 || zFilename[0]==0 ){
    برآمد = stdout;
  }ورنہ{
    برآمد = fopen(zFilename, "w");
    اگر( برآمد==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      واپس;
    }
  }
  memsys3Enter();
  fprintf(برآمد, "CHUNKS:\n");
  برائے(i=1; i<=mem3.nPool; i+=سائز/4){
    سائز = mem3.aPool[i-1].u.hdr.size4x;
    اگر( سائز/4<=1 ){
      fprintf(برآمد, "%p size error\n", &mem3.aPool[i]);
      أكد( 0 );
      توڑو;
    }
    اگر( (سائز&1)==0 && mem3.aPool[i+سائز/4-1].u.hdr.پچھلا_سائز!=سائز/4 ){
      fprintf(برآمد, "%p tail size does not match\n", &mem3.aPool[i]);
      أكد( 0 );
      توڑو;
    }
    اگر( ((mem3.aPool[i+سائز/4-1].u.hdr.size4x&2)>>1)!=(سائز&1) ){
      fprintf(برآمد, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      أكد( 0 );
      توڑو;
    }
    اگر( سائز&1 ){
      fprintf(برآمد, "%p %6d bytes checked out\n", &mem3.aPool[i], (سائز/4)*8-8);
    }ورنہ{
      fprintf(برآمد, "%p %6d bytes free%s\n", &mem3.aPool[i], (سائز/4)*8-8,
                  i==mem3.i_کلید_Blk ? " **key**" : "");
    }
  }
  برائے(i=0; i<MX_SMALL-1; i++){
    اگر( mem3.aiSmall[i]==0 ) جاری;
    fprintf(برآمد, "small(%2d):", i);
    برائے(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.اگلا){
      fprintf(برآمد, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(برآمد, "\n"); 
  }
  برائے(i=0; i<N_HASH; i++){
    اگر( mem3.aiHash[i]==0 ) جاری;
    fprintf(برآمد, "hash(%2d):", i);
    برائے(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.اگلا){
      fprintf(برآمد, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(برآمد, "\n"); 
  }
  fprintf(برآمد, "key=%d\n", mem3.i_کلید_Blk);
  fprintf(برآمد, "nowUsed=%d\n", mem3.nPool*8 - mem3.sz_کلید_Blk*8);
  fprintf(برآمد, "mxUsed=%d\n", mem3.nPool*8 - mem3.mn_کلید_Blk*8);
  sqlite3_mutex_leave(mem3.mutex);
  اگر( برآمد==stdout ){
    fflush(stdout);
  }ورنہ{
    fclose(برآمد);
  }
#ورنہ
  غیر_مستعمل_PARAMETER(zFilename);
#اگر_ختم
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
ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(خالی){
  ساکن ثابت sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3_سائز,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  واپس &mempoolMethods;
}

#اگر_ختم /* SQLITE_ENABLE_MEMSYS3 */
