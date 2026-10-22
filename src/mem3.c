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
#include "sqliteInt.h"

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
typedef struct Mem3Block Mem3Block;
struct Mem3Block {
  union {
    struct {
      u32 prevSize;   /* Size of previous chunk in Mem3Block elements */
      u32 size4x;     /* 4x the size of current chunk in Mem3Block elements */
    } hdr;
    struct {
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
static SQLITE_WSD struct Mem3Global {
  /*
  ** Memory available for allocation. nPool is the size of the array
  ** (in Mem3Blocks) pointed to by aPool less 2.
  */
  u32 nPool;
  Mem3Block *aPool;

  /*
  ** True if we are evaluating an out-of-memory callback.
  */
  цео alarmBusy;
  
  /*
  ** Mutex to control access to the memory allocation subsystem.
  */
  sqlite3_mutex *mutex;
  
  /*
  ** The minimum amount of free space that we have seen.
  */
  u32 mnKeyBlk;

  /*
  ** iKeyBlk is the index of the key chunk.  Most new allocations
  ** occur off of this chunk.  szKeyBlk is the size (in Mem3Blocks)
  ** of the current key chunk.  iKeyBlk is 0 if there is no key chunk.
  ** The key chunk is not in either the aiHash[] or aiSmall[].
  */
  u32 iKeyBlk;
  u32 szKeyBlk;

  /*
  ** Array of lists of free blocks according to the block size 
  ** for smaller chunks, or a hash on the block size for larger
  ** chunks.
  */
  u32 aiSmall[MX_SMALL-1];   /* For sizes 2 through MX_SMALL, inclusive */
  u32 aiHash[N_HASH];        /* For sizes MX_SMALL+1 and larger */
} mem3 = { 97535575 };

#define mem3 GLOBAL(struct Mem3Global, mem3)

/*
** Unlink the chunk at mem3.aPool[i] from list it is currently
** on.  *pRoot is the list that i is a member of.
*/
static void memsys3UnlinkFromList(u32 i, u32 *pRoot){
  u32 next = mem3.aPool[i].u.list.next;
  u32 prev = mem3.aPool[i].u.list.prev;
  assert( sqlite3_mutex_held(mem3.mutex) );
  ако( prev==0 ){
    *pRoot = next;
  }инако{
    mem3.aPool[prev].u.list.next = next;
  }
  ако( next ){
    mem3.aPool[next].u.list.prev = prev;
  }
  mem3.aPool[i].u.list.next = 0;
  mem3.aPool[i].u.list.prev = 0;
}

/*
** Unlink the chunk at index i from 
** whatever list is currently a member of.
*/
static void memsys3Unlink(u32 i){
  u32 size, hash;
  assert( sqlite3_mutex_held(mem3.mutex) );
  assert( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  assert( i>=1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  assert( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  assert( size>=2 );
  ако( size <= MX_SMALL ){
    memsys3UnlinkFromList(i, &mem3.aiSmall[size-2]);
  }инако{
    hash = size % N_HASH;
    memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
  }
}

/*
** Link the chunk at mem3.aPool[i] so that is on the list rooted
** at *pRoot.
*/
static void memsys3LinkIntoList(u32 i, u32 *pRoot){
  assert( sqlite3_mutex_held(mem3.mutex) );
  mem3.aPool[i].u.list.next = *pRoot;
  mem3.aPool[i].u.list.prev = 0;
  ако( *pRoot ){
    mem3.aPool[*pRoot].u.list.prev = i;
  }
  *pRoot = i;
}

/*
** Link the chunk at index i into either the appropriate
** small chunk list, or into the large chunk hash table.
*/
static void memsys3Link(u32 i){
  u32 size, hash;
  assert( sqlite3_mutex_held(mem3.mutex) );
  assert( i>=1 );
  assert( (mem3.aPool[i-1].u.hdr.size4x & 1)==0 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  assert( size==mem3.aPool[i+size-1].u.hdr.prevSize );
  assert( size>=2 );
  ако( size <= MX_SMALL ){
    memsys3LinkIntoList(i, &mem3.aiSmall[size-2]);
  }инако{
    hash = size % N_HASH;
    memsys3LinkIntoList(i, &mem3.aiHash[hash]);
  }
}

/*
** If the STATIC_MEM mutex is not already held, obtain it now. The mutex
** will already be held (obtained by code in malloc.c) if
** sqlite3GlobalConfig.bMemStat is true.
*/
static void memsys3Enter(void){
  ако( sqlite3GlobalConfig.bMemstat==0 && mem3.mutex==0 ){
    mem3.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  }
  sqlite3_mutex_enter(mem3.mutex);
}
static void memsys3Leave(void){
  sqlite3_mutex_leave(mem3.mutex);
}

/*
** Called when we are unable to satisfy an allocation of nBytes.
*/
static void memsys3OutOfMemory(цео nByte){
  ако( !mem3.alarmBusy ){
    mem3.alarmBusy = 1;
    assert( sqlite3_mutex_held(mem3.mutex) );
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
static void *memsys3Checkout(u32 i, u32 nBlock){
  u32 x;
  assert( sqlite3_mutex_held(mem3.mutex) );
  assert( i>=1 );
  assert( mem3.aPool[i-1].u.hdr.size4x/4==nBlock );
  assert( mem3.aPool[i+nBlock-1].u.hdr.prevSize==nBlock );
  x = mem3.aPool[i-1].u.hdr.size4x;
  mem3.aPool[i-1].u.hdr.size4x = nBlock*4 | 1 | (x&2);
  mem3.aPool[i+nBlock-1].u.hdr.prevSize = nBlock;
  mem3.aPool[i+nBlock-1].u.hdr.size4x |= 2;
  врати &mem3.aPool[i];
}

/*
** Carve a piece off of the end of the mem3.iKeyBlk free chunk.
** Return a pointer to the new allocation.  Or, if the key chunk
** is not large enough, return 0.
*/
static void *memsys3FromKeyBlk(u32 nBlock){
  assert( sqlite3_mutex_held(mem3.mutex) );
  assert( mem3.szKeyBlk>=nBlock );
  ако( nBlock>=mem3.szKeyBlk-1 ){
    /* Use the entire key chunk */
    void *p = memsys3Checkout(mem3.iKeyBlk, mem3.szKeyBlk);
    mem3.iKeyBlk = 0;
    mem3.szKeyBlk = 0;
    mem3.mnKeyBlk = 0;
    врати p;
  }инако{
    /* Split the key block.  Return the tail. */
    u32 newi, x;
    newi = mem3.iKeyBlk + mem3.szKeyBlk - nBlock;
    assert( newi > mem3.iKeyBlk+1 );
    mem3.aPool[mem3.iKeyBlk+mem3.szKeyBlk-1].u.hdr.prevSize = nBlock;
    mem3.aPool[mem3.iKeyBlk+mem3.szKeyBlk-1].u.hdr.size4x |= 2;
    mem3.aPool[newi-1].u.hdr.size4x = nBlock*4 + 1;
    mem3.szKeyBlk -= nBlock;
    mem3.aPool[newi-1].u.hdr.prevSize = mem3.szKeyBlk;
    x = mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x & 2;
    mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x = mem3.szKeyBlk*4 | x;
    ако( mem3.szKeyBlk < mem3.mnKeyBlk ){
      mem3.mnKeyBlk = mem3.szKeyBlk;
    }
    врати (void*)&mem3.aPool[newi];
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
static void memsys3Merge(u32 *pRoot){
  u32 iNext, prev, size, i, x;

  assert( sqlite3_mutex_held(mem3.mutex) );
  за(i=*pRoot; i>0; i=iNext){
    iNext = mem3.aPool[i].u.list.next;
    size = mem3.aPool[i-1].u.hdr.size4x;
    assert( (size&1)==0 );
    ако( (size&2)==0 ){
      memsys3UnlinkFromList(i, pRoot);
      assert( i > mem3.aPool[i-1].u.hdr.prevSize );
      prev = i - mem3.aPool[i-1].u.hdr.prevSize;
      ако( prev==iNext ){
        iNext = mem3.aPool[prev].u.list.next;
      }
      memsys3Unlink(prev);
      size = i + size/4 - prev;
      x = mem3.aPool[prev-1].u.hdr.size4x & 2;
      mem3.aPool[prev-1].u.hdr.size4x = size*4 | x;
      mem3.aPool[prev+size-1].u.hdr.prevSize = size;
      memsys3Link(prev);
      i = prev;
    }инако{
      size /= 4;
    }
    ако( size>mem3.szKeyBlk ){
      mem3.iKeyBlk = i;
      mem3.szKeyBlk = size;
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
static void *memsys3MallocUnsafe(цео nByte){
  u32 i;
  u32 nBlock;
  u32 toFree;

  assert( sqlite3_mutex_held(mem3.mutex) );
  assert( sizeof(Mem3Block)==8 );
  ако( nByte<=12 ){
    nBlock = 2;
  }инако{
    nBlock = (nByte + 11)/8;
  }
  assert( nBlock>=2 );

  /* STEP 1:
  ** Look for an entry of the correct size in either the small
  ** chunk table or in the large chunk hash table.  This is
  ** successful most of the time (about 9 times out of 10).
  */
  ако( nBlock <= MX_SMALL ){
    i = mem3.aiSmall[nBlock-2];
    ако( i>0 ){
      memsys3UnlinkFromList(i, &mem3.aiSmall[nBlock-2]);
      врати memsys3Checkout(i, nBlock);
    }
  }инако{
    цео hash = nBlock % N_HASH;
    за(i=mem3.aiHash[hash]; i>0; i=mem3.aPool[i].u.list.next){
      ако( mem3.aPool[i-1].u.hdr.size4x/4==nBlock ){
        memsys3UnlinkFromList(i, &mem3.aiHash[hash]);
        врати memsys3Checkout(i, nBlock);
      }
    }
  }

  /* STEP 2:
  ** Try to satisfy the allocation by carving a piece off of the end
  ** of the key chunk.  This step usually works if step 1 fails.
  */
  ако( mem3.szKeyBlk>=nBlock ){
    врати memsys3FromKeyBlk(nBlock);
  }


  /* STEP 3:  
  ** Loop through the entire memory pool.  Coalesce adjacent free
  ** chunks.  Recompute the key chunk as the largest free chunk.
  ** Then try again to satisfy the allocation by carving a piece off
  ** of the end of the key chunk.  This step happens very
  ** rarely (we hope!)
  */
  за(toFree=nBlock*16; toFree<(mem3.nPool*16); toFree *= 2){
    memsys3OutOfMemory(toFree);
    ако( mem3.iKeyBlk ){
      memsys3Link(mem3.iKeyBlk);
      mem3.iKeyBlk = 0;
      mem3.szKeyBlk = 0;
    }
    за(i=0; i<N_HASH; i++){
      memsys3Merge(&mem3.aiHash[i]);
    }
    за(i=0; i<MX_SMALL-1; i++){
      memsys3Merge(&mem3.aiSmall[i]);
    }
    ако( mem3.szKeyBlk ){
      memsys3Unlink(mem3.iKeyBlk);
      ако( mem3.szKeyBlk>=nBlock ){
        врати memsys3FromKeyBlk(nBlock);
      }
    }
  }

  /* If none of the above worked, then we fail. */
  врати 0;
}

/*
** Free an outstanding memory allocation.
**
** This function assumes that the necessary mutexes, if any, are
** already held by the caller. Hence "Unsafe".
*/
static void memsys3FreeUnsafe(void *pOld){
  Mem3Block *p = (Mem3Block*)pOld;
  цео i;
  u32 size, x;
  assert( sqlite3_mutex_held(mem3.mutex) );
  assert( p>mem3.aPool && p<&mem3.aPool[mem3.nPool] );
  i = p - mem3.aPool;
  assert( (mem3.aPool[i-1].u.hdr.size4x&1)==1 );
  size = mem3.aPool[i-1].u.hdr.size4x/4;
  assert( i+size<=mem3.nPool+1 );
  mem3.aPool[i-1].u.hdr.size4x &= ~1;
  mem3.aPool[i+size-1].u.hdr.prevSize = size;
  mem3.aPool[i+size-1].u.hdr.size4x &= ~2;
  memsys3Link(i);

  /* Try to expand the key using the newly freed chunk */
  ако( mem3.iKeyBlk ){
    док( (mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x&2)==0 ){
      size = mem3.aPool[mem3.iKeyBlk-1].u.hdr.prevSize;
      mem3.iKeyBlk -= size;
      mem3.szKeyBlk += size;
      memsys3Unlink(mem3.iKeyBlk);
      x = mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x & 2;
      mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x = mem3.szKeyBlk*4 | x;
      mem3.aPool[mem3.iKeyBlk+mem3.szKeyBlk-1].u.hdr.prevSize = mem3.szKeyBlk;
    }
    x = mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x & 2;
    док( (mem3.aPool[mem3.iKeyBlk+mem3.szKeyBlk-1].u.hdr.size4x&1)==0 ){
      memsys3Unlink(mem3.iKeyBlk+mem3.szKeyBlk);
      mem3.szKeyBlk += mem3.aPool[mem3.iKeyBlk+mem3.szKeyBlk-1].u.hdr.size4x/4;
      mem3.aPool[mem3.iKeyBlk-1].u.hdr.size4x = mem3.szKeyBlk*4 | x;
      mem3.aPool[mem3.iKeyBlk+mem3.szKeyBlk-1].u.hdr.prevSize = mem3.szKeyBlk;
    }
  }
}

/*
** Return the size of an outstanding allocation, in bytes.  The
** size returned omits the 8-byte header overhead.  This only
** works for chunks that are currently checked out.
*/
static цео memsys3Size(void *p){
  Mem3Block *pBlock;
  assert( p!=0 );
  pBlock = (Mem3Block*)p;
  assert( (pBlock[-1].u.hdr.size4x&1)!=0 );
  врати (pBlock[-1].u.hdr.size4x&~3)*2 - 4;
}

/*
** Round up a request size to the next valid allocation size.
*/
static цео memsys3Roundup(цео n){
  ако( n<=12 ){
    врати 12;
  }инако{
    врати ((n+11)&~7) - 4;
  }
}

/*
** Allocate nBytes of memory.
*/
static void *memsys3Malloc(цео nBytes){
  sqlite3_int64 *p;
  assert( nBytes>0 );          /* malloc.c filters out 0 byte requests */
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  memsys3Leave();
  врати (void*)p; 
}

/*
** Free memory.
*/
static void memsys3Free(void *pPrior){
  assert( pPrior );
  memsys3Enter();
  memsys3FreeUnsafe(pPrior);
  memsys3Leave();
}

/*
** Change the size of an existing memory allocation
*/
static void *memsys3Realloc(void *pPrior, цео nBytes){
  цео nOld;
  void *p;
  ако( pPrior==0 ){
    врати sqlite3_malloc(nBytes);
  }
  ако( nBytes<=0 ){
    sqlite3_free(pPrior);
    врати 0;
  }
  nOld = memsys3Size(pPrior);
  ако( nBytes<=nOld && nBytes>=nOld-128 ){
    врати pPrior;
  }
  memsys3Enter();
  p = memsys3MallocUnsafe(nBytes);
  ако( p ){
    ако( nOld<nBytes ){
      memcpy(p, pPrior, nOld);
    }инако{
      memcpy(p, pPrior, nBytes);
    }
    memsys3FreeUnsafe(pPrior);
  }
  memsys3Leave();
  врати p;
}

/*
** Initialize this module.
*/
static цео memsys3Init(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  ако( !sqlite3GlobalConfig.pHeap ){
    врати SQLITE_ERROR;
  }

  /* Store a pointer to the memory block in global structure mem3. */
  assert( sizeof(Mem3Block)==8 );
  mem3.aPool = (Mem3Block *)sqlite3GlobalConfig.pHeap;
  mem3.nPool = (sqlite3GlobalConfig.nHeap / sizeof(Mem3Block)) - 2;

  /* Initialize the key block. */
  mem3.szKeyBlk = mem3.nPool;
  mem3.mnKeyBlk = mem3.szKeyBlk;
  mem3.iKeyBlk = 1;
  mem3.aPool[0].u.hdr.size4x = (mem3.szKeyBlk<<2) + 2;
  mem3.aPool[mem3.nPool].u.hdr.prevSize = mem3.nPool;
  mem3.aPool[mem3.nPool].u.hdr.size4x = 1;

  врати SQLITE_OK;
}

/*
** Deinitialize this module.
*/
static void memsys3Shutdown(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  mem3.mutex = 0;
  врати;
}



/*
** Open the file indicated and write a log of all unfreed memory 
** allocations into that log.
*/
void sqlite3Memsys3Dump(const char *zFilename){
#ifdef SQLITE_DEBUG
  FILE *out;
  u32 i, j;
  u32 size;
  ако( zFilename==0 || zFilename[0]==0 ){
    out = stdout;
  }инако{
    out = fopen(zFilename, "w");
    ако( out==0 ){
      fprintf(stderr, "** Unable to output memory debug output log: %s **\n",
                      zFilename);
      врати;
    }
  }
  memsys3Enter();
  fprintf(out, "CHUNKS:\n");
  за(i=1; i<=mem3.nPool; i+=size/4){
    size = mem3.aPool[i-1].u.hdr.size4x;
    ако( size/4<=1 ){
      fprintf(out, "%p size error\n", &mem3.aPool[i]);
      assert( 0 );
      break;
    }
    ако( (size&1)==0 && mem3.aPool[i+size/4-1].u.hdr.prevSize!=size/4 ){
      fprintf(out, "%p tail size does not match\n", &mem3.aPool[i]);
      assert( 0 );
      break;
    }
    ако( ((mem3.aPool[i+size/4-1].u.hdr.size4x&2)>>1)!=(size&1) ){
      fprintf(out, "%p tail checkout bit is incorrect\n", &mem3.aPool[i]);
      assert( 0 );
      break;
    }
    ако( size&1 ){
      fprintf(out, "%p %6d bytes checked out\n", &mem3.aPool[i], (size/4)*8-8);
    }инако{
      fprintf(out, "%p %6d bytes free%s\n", &mem3.aPool[i], (size/4)*8-8,
                  i==mem3.iKeyBlk ? " **key**" : "");
    }
  }
  за(i=0; i<MX_SMALL-1; i++){
    ако( mem3.aiSmall[i]==0 ) continue;
    fprintf(out, "small(%2d):", i);
    за(j = mem3.aiSmall[i]; j>0; j=mem3.aPool[j].u.list.next){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  за(i=0; i<N_HASH; i++){
    ако( mem3.aiHash[i]==0 ) continue;
    fprintf(out, "hash(%2d):", i);
    за(j = mem3.aiHash[i]; j>0; j=mem3.aPool[j].u.list.next){
      fprintf(out, " %p(%d)", &mem3.aPool[j],
              (mem3.aPool[j-1].u.hdr.size4x/4)*8-8);
    }
    fprintf(out, "\n"); 
  }
  fprintf(out, "key=%d\n", mem3.iKeyBlk);
  fprintf(out, "nowUsed=%d\n", mem3.nPool*8 - mem3.szKeyBlk*8);
  fprintf(out, "mxUsed=%d\n", mem3.nPool*8 - mem3.mnKeyBlk*8);
  sqlite3_mutex_leave(mem3.mutex);
  ако( out==stdout ){
    fflush(stdout);
  }инако{
    fclose(out);
  }
#инако
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
const sqlite3_mem_methods *sqlite3MemGetMemsys3(void){
  static const sqlite3_mem_methods mempoolMethods = {
     memsys3Malloc,
     memsys3Free,
     memsys3Realloc,
     memsys3Size,
     memsys3Roundup,
     memsys3Init,
     memsys3Shutdown,
     0
  };
  врати &mempoolMethods;
}

#endif /* SQLITE_ENABLE_MEMSYS3 */
