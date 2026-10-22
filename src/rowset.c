/*
** 2008 December 3
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This module implements an object we call a "RowSet".
**
** The RowSet object is a collection of rowids.  Rowids
** are inserted into the RowSet in an arbitrary order.  Inserts
** can be intermixed with tests to see if a given rowid has been
** previously inserted into the RowSet.
**
** After all inserts are finished, it is possible to extract the
** elements of the RowSet in sorted order.  Once this extraction
** process has started, no new elements may be inserted.
**
** Hence, the primitive operations for a RowSet are:
**
**    CREATE
**    INSERT
**    TEST
**    SMALLEST
**    DESTROY
**
** The CREATE and DESTROY primitives are the constructor and destructor,
** obviously.  The INSERT primitive adds a new element to the RowSet.
** TEST checks to see if an element is already in the RowSet.  SMALLEST
** extracts the least value from the RowSet.
**
** The INSERT primitive might allocate additional memory.  Memory is
** allocated in chunks so most INSERTs do no allocation.  There is an
** upper bound on the size of allocated memory.  No memory is freed
** until DESTROY.
**
** The TEST primitive includes a "batch" number.  The TEST primitive
** will only see elements that were inserted before the last change
** in the batch number.  In other words, if an INSERT occurs between
** two TESTs where the TESTs have the same batch number, then the
** value added by the INSERT will not be visible to the second TEST.
** The initial batch number is zero, so if the very first TEST contains
** a non-zero batch number, it will see all prior INSERTs.
**
** No INSERTs may occurs after a SMALLEST.  An assertion will fail if
** that is attempted.
**
** The cost of an INSERT is roughly constant.  (Sometimes new memory
** has to be allocated on an INSERT.)  The cost of a TEST with a new
** batch number is O(NlogN) where N is the number of elements in the RowSet.
** The cost of a TEST using the same batch number is O(logN).  The cost
** of the first SMALLEST is O(NlogN).  Second and subsequent SMALLEST
** primitives are constant time.  The cost of DESTROY is O(N).
**
** TEST and SMALLEST may not be used by the same RowSet.  This used to
** be possible, but the feature was not used, so it was removed in order
** to simplify the code.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROWSET_ਵੰਡ_ਮਾਪ 1024

/*
** The number of rowset entries per allocation chunk.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROWSET_ਇੰਦਰਾਜ_ਪ੍ਰਤੀ_CHUNK  \
                       ((ROWSET_ਵੰਡ_ਮਾਪ-8)/ਆਕਾਰ(ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ {           
  i64 v;                        /* ROWID value for this entry */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਸੱਜਾ;   /* Right subtree (larger entries) or list */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਖੱਬਾ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_Chunk {
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_Chunk *p_ਅਗਲਾ_Chunk;        /* Next chunk on list of them all */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ a_ਇੰਦਰਾਜ[ROWSET_ਇੰਦਰਾਜ_ਪ੍ਰਤੀ_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ {
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਇੰਦਰਾਜ;    /* List of entries using pRight */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਆਖਰੀ;     /* Last entry on the pEntry list */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *pFresh;    /* Source of new entry objects */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  ਪੂਰਨ iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROWSET_ਅਗਲਾ    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
ਪੰਕਤੀ_ਸੈੱਟ *sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Init(sqlite3 *db){
  ਪੰਕਤੀ_ਸੈੱਟ *p = sqlite3DbMallocRawNN(db, ਆਕਾਰ(*p));
  ਜੇ( p ){
    ਪੂਰਨ N = sqlite3_Db_Malloc_ਮਾਪ(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_ਇੰਦਰਾਜ = 0;
    p->p_ਆਖਰੀ = 0;
    p->pForest = 0;
    p->pFresh = (ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ*)(ROUND8(ਆਕਾਰ(*p)) + (ਅੱਖਰ*)p);
    p->nFresh = (u16)((N - ROUND8(ਆਕਾਰ(*p)))/ਆਕਾਰ(ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  ਵਾਪਸ p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ਖਾਲੀ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Clear(ਖਾਲੀ *pArg){
  ਪੰਕਤੀ_ਸੈੱਟ *p = (ਪੰਕਤੀ_ਸੈੱਟ*)pArg;
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_Chunk *pChunk, *p_ਅਗਲਾ_Chunk;
  ਲਈ(pChunk=p->pChunk; pChunk; pChunk = p_ਅਗਲਾ_Chunk){
    p_ਅਗਲਾ_Chunk = pChunk->p_ਅਗਲਾ_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_ਇੰਦਰਾਜ = 0;
  p->p_ਆਖਰੀ = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ਖਾਲੀ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Delete(ਖਾਲੀ *pArg){
  sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Clear(pArg);
  sqlite3DbFree(((ਪੰਕਤੀ_ਸੈੱਟ*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ਸਥਿਰ ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_Alloc(ਪੰਕਤੀ_ਸੈੱਟ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਜੇ( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, ਆਕਾਰ(*pNew));
    ਜੇ( pNew==0 ){
      ਵਾਪਸ 0;
    }
    pNew->p_ਅਗਲਾ_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_ਇੰਦਰਾਜ;
    p->nFresh = ROWSET_ਇੰਦਰਾਜ_ਪ੍ਰਤੀ_CHUNK;
  }
  p->nFresh--;
  ਵਾਪਸ p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
ਖਾਲੀ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_ਪਾਓ(ਪੰਕਤੀ_ਸੈੱਟ *p, i64 rowid){
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਇੰਦਰਾਜ;  /* The new entry */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਆਖਰੀ;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && (p->rsFlags & ROWSET_ਅਗਲਾ)==0 );

  p_ਇੰਦਰਾਜ = ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_Alloc(p);
  ਜੇ( p_ਇੰਦਰਾਜ==0 ) ਵਾਪਸ;
  p_ਇੰਦਰਾਜ->v = rowid;
  p_ਇੰਦਰਾਜ->p_ਸੱਜਾ = 0;
  p_ਆਖਰੀ = p->p_ਆਖਰੀ;
  ਜੇ( p_ਆਖਰੀ ){
    ਜੇ( rowid<=p_ਆਖਰੀ->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_ਆਖਰੀ->p_ਸੱਜਾ = p_ਇੰਦਰਾਜ;
  }ਨਹੀਂਤਾਂ{
    p->p_ਇੰਦਰਾਜ = p_ਇੰਦਰਾਜ;
  }
  p->p_ਆਖਰੀ = p_ਇੰਦਰਾਜ;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ਸਥਿਰ ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਮਿਲਾਓ(
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *pA,    /* First sorted list to be merged */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *pB     /* Second sorted list to be merged */
){
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ ਸਿਰ;
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਪੂਛ;

  p_ਪੂਛ = &ਸਿਰ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pA!=0 && pB!=0 );
  ਲਈ(;;){
    ਪ੍ਰਤਿਜ੍ਞਾ( pA->p_ਸੱਜਾ==0 || pA->v<=pA->p_ਸੱਜਾ->v );
    ਪ੍ਰਤਿਜ੍ਞਾ( pB->p_ਸੱਜਾ==0 || pB->v<=pB->p_ਸੱਜਾ->v );
    ਜੇ( pA->v<=pB->v ){
      ਜੇ( pA->v<pB->v ) p_ਪੂਛ = p_ਪੂਛ->p_ਸੱਜਾ = pA;
      pA = pA->p_ਸੱਜਾ;
      ਜੇ( pA==0 ){
        p_ਪੂਛ->p_ਸੱਜਾ = pB;
        ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ{
      p_ਪੂਛ = p_ਪੂਛ->p_ਸੱਜਾ = pB;
      pB = pB->p_ਸੱਜਾ;
      ਜੇ( pB==0 ){
        p_ਪੂਛ->p_ਸੱਜਾ = pA;
        ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ ਸਿਰ.p_ਸੱਜਾ;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ਸਥਿਰ ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਛਾਂਟੋ(ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਵਿੱਚ){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਅਗਲਾ, *a_ਬਾਲਟੀ[40];

  memset(a_ਬਾਲਟੀ, 0, ਆਕਾਰ(a_ਬਾਲਟੀ));
  ਜਦੋਂ( p_ਵਿੱਚ ){
    p_ਅਗਲਾ = p_ਵਿੱਚ->p_ਸੱਜਾ;
    p_ਵਿੱਚ->p_ਸੱਜਾ = 0;
    ਲਈ(i=0; a_ਬਾਲਟੀ[i]; i++){
      p_ਵਿੱਚ = ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਮਿਲਾਓ(a_ਬਾਲਟੀ[i], p_ਵਿੱਚ);
      a_ਬਾਲਟੀ[i] = 0;
    }
    a_ਬਾਲਟੀ[i] = p_ਵਿੱਚ;
    p_ਵਿੱਚ = p_ਅਗਲਾ;
  }
  p_ਵਿੱਚ = a_ਬਾਲਟੀ[0];
  ਲਈ(i=1; i<ਆਕਾਰ(a_ਬਾਲਟੀ)/ਆਕਾਰ(a_ਬਾਲਟੀ[0]); i++){
    ਜੇ( a_ਬਾਲਟੀ[i]==0 ) ਜਾਰੀ;
    p_ਵਿੱਚ = p_ਵਿੱਚ ? ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਮਿਲਾਓ(p_ਵਿੱਚ, a_ਬਾਲਟੀ[i]) : a_ਬਾਲਟੀ[i];
  }
  ਵਾਪਸ p_ਵਿੱਚ;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ਸਥਿਰ ਖਾਲੀ ਪੰਕਤੀ_ਸੈੱਟ_ਰੁੱਖ_ਮੰਜ਼ਿਲ_List(
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਵਿੱਚ,         /* Root of the input tree */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ **pp_ਪਹਿਲਾ,    /* Write head of the output list here */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ **pp_ਆਖਰੀ      /* Write tail of the output list here */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿੱਚ!=0 );
  ਜੇ( p_ਵਿੱਚ->p_ਖੱਬਾ ){
    ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p;
    ਪੰਕਤੀ_ਸੈੱਟ_ਰੁੱਖ_ਮੰਜ਼ਿਲ_List(p_ਵਿੱਚ->p_ਖੱਬਾ, pp_ਪਹਿਲਾ, &p);
    p->p_ਸੱਜਾ = p_ਵਿੱਚ;
  }ਨਹੀਂਤਾਂ{
    *pp_ਪਹਿਲਾ = p_ਵਿੱਚ;
  }
  ਜੇ( p_ਵਿੱਚ->p_ਸੱਜਾ ){
    ਪੰਕਤੀ_ਸੈੱਟ_ਰੁੱਖ_ਮੰਜ਼ਿਲ_List(p_ਵਿੱਚ->p_ਸੱਜਾ, &p_ਵਿੱਚ->p_ਸੱਜਾ, pp_ਆਖਰੀ);
  }ਨਹੀਂਤਾਂ{
    *pp_ਆਖਰੀ = p_ਵਿੱਚ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (*pp_ਆਖਰੀ)->p_ਸੱਜਾ==0 );
}


/*
** Convert a sorted list of elements (connected by pRight) into a binary
** tree with depth of iDepth.  A depth of 1 means the tree contains a single
** node taken from the head of *ppList.  A depth of 2 means a tree with
** three nodes.  And so forth.
**
** Use as many entries from the input list as required and update the
** *ppList to point to the unused elements of the list.  If the input
** list contains too few elements, then construct an incomplete tree
** and leave *ppList set to NULL.
**
** Return a pointer to the root of the constructed binary tree.
*/
ਸਥਿਰ ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *ਪੰਕਤੀ_ਸੈੱਟ_NDeep_ਰੁੱਖ(
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ **ppList,
  ਪੂਰਨ i_ਡੂੰਘਾਈ
){
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p;         /* Root of the new tree */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਖੱਬਾ;     /* Left subtree */
  ਜੇ( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    ਵਾਪਸ 0;
  }
  ਜੇ( i_ਡੂੰਘਾਈ>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_ਖੱਬਾ = ਪੰਕਤੀ_ਸੈੱਟ_NDeep_ਰੁੱਖ(ppList, i_ਡੂੰਘਾਈ-1);
    p = *ppList;
    ਜੇ( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      ਵਾਪਸ p_ਖੱਬਾ;
    }
    p->p_ਖੱਬਾ = p_ਖੱਬਾ;
    *ppList = p->p_ਸੱਜਾ;
    p->p_ਸੱਜਾ = ਪੰਕਤੀ_ਸੈੱਟ_NDeep_ਰੁੱਖ(ppList, i_ਡੂੰਘਾਈ-1);
  }ਨਹੀਂਤਾਂ{
    p = *ppList;
    *ppList = p->p_ਸੱਜਾ;
    p->p_ਖੱਬਾ = p->p_ਸੱਜਾ = 0;
  }
  ਵਾਪਸ p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ਸਥਿਰ ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *ਪੰਕਤੀ_ਸੈੱਟ_List_ਮੰਜ਼ਿਲ_ਰੁੱਖ(ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *pList){
  ਪੂਰਨ i_ਡੂੰਘਾਈ;           /* Depth of the tree so far */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p;       /* Current tree root */
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p_ਖੱਬਾ;   /* Left subtree */

  ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
  p = pList;
  pList = p->p_ਸੱਜਾ;
  p->p_ਖੱਬਾ = p->p_ਸੱਜਾ = 0;
  ਲਈ(i_ਡੂੰਘਾਈ=1; pList; i_ਡੂੰਘਾਈ++){
    p_ਖੱਬਾ = p;
    p = pList;
    pList = p->p_ਸੱਜਾ;
    p->p_ਖੱਬਾ = p_ਖੱਬਾ;
    p->p_ਸੱਜਾ = ਪੰਕਤੀ_ਸੈੱਟ_NDeep_ਰੁੱਖ(&pList, i_ਡੂੰਘਾਈ);
  }
  ਵਾਪਸ p;
}

/*
** Extract the smallest element from the RowSet.
** Write the element into *pRowid.  Return 1 on success.  Return
** 0 if the RowSet is already empty.
**
** After this routine has been called, the sqlite3RowSetInsert()
** routine may not be called again.
**
** This routine may not be called after sqlite3RowSetTest() has
** been used.  Older versions of RowSet allowed that, but as the
** capability was not used by the code generator, it was removed
** for code economy.
*/
ਪੂਰਨ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_ਅਗਲਾ(ਪੰਕਤੀ_ਸੈੱਟ *p, i64 *pRowid){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  ਜੇ( (p->rsFlags & ROWSET_ਅਗਲਾ)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    ਜੇ( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_ਇੰਦਰਾਜ = ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਛਾਂਟੋ(p->p_ਇੰਦਰਾਜ);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_ਅਗਲਾ;
  }

  /* Return the next entry on the list */
  ਜੇ( p->p_ਇੰਦਰਾਜ ){
    *pRowid = p->p_ਇੰਦਰਾਜ->v;
    p->p_ਇੰਦਰਾਜ = p->p_ਇੰਦਰਾਜ->p_ਸੱਜਾ;
    ਜੇ( p->p_ਇੰਦਰਾਜ==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Clear(p);
    }
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}

/*
** Check to see if element iRowid was inserted into the rowset as
** part of any insert batch prior to iBatch.  Return 1 or 0.
**
** If this is the first test of a new batch and if there exist entries
** on pRowSet->pEntry, then sort those entries into the forest at
** pRowSet->pForest so that they can be tested.
*/
ਪੂਰਨ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Test(ਪੰਕਤੀ_ਸੈੱਟ *p_ਪੰਕਤੀ_ਸੈੱਟ, ਪੂਰਨ iBatch, sqlite3_int64 iRowid){
  ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *p, *p_ਰੁੱਖ;

  /* This routine is never called after sqlite3RowSetNext() */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਕਤੀ_ਸੈੱਟ!=0 && (p_ਪੰਕਤੀ_ਸੈੱਟ->rsFlags & ROWSET_ਅਗਲਾ)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  ਜੇ( iBatch!=p_ਪੰਕਤੀ_ਸੈੱਟ->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_ਪੰਕਤੀ_ਸੈੱਟ->p_ਇੰਦਰਾਜ;
    ਜੇ( p ){
      ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ **pp_Prev_ਰੁੱਖ = &p_ਪੰਕਤੀ_ਸੈੱਟ->pForest;
      ਜੇ( (p_ਪੰਕਤੀ_ਸੈੱਟ->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਛਾਂਟੋ(p);
      }
      ਲਈ(p_ਰੁੱਖ = p_ਪੰਕਤੀ_ਸੈੱਟ->pForest; p_ਰੁੱਖ; p_ਰੁੱਖ=p_ਰੁੱਖ->p_ਸੱਜਾ){
        pp_Prev_ਰੁੱਖ = &p_ਰੁੱਖ->p_ਸੱਜਾ;
        ਜੇ( p_ਰੁੱਖ->p_ਖੱਬਾ==0 ){
          p_ਰੁੱਖ->p_ਖੱਬਾ = ਪੰਕਤੀ_ਸੈੱਟ_List_ਮੰਜ਼ਿਲ_ਰੁੱਖ(p);
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ *pAux, *p_ਪੂਛ;
          ਪੰਕਤੀ_ਸੈੱਟ_ਰੁੱਖ_ਮੰਜ਼ਿਲ_List(p_ਰੁੱਖ->p_ਖੱਬਾ, &pAux, &p_ਪੂਛ);
          p_ਰੁੱਖ->p_ਖੱਬਾ = 0;
          p = ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_ਮਿਲਾਓ(pAux, p);
        }
      }
      ਜੇ( p_ਰੁੱਖ==0 ){
        *pp_Prev_ਰੁੱਖ = p_ਰੁੱਖ = ਪੰਕਤੀ_ਸੈੱਟ_ਇੰਦਰਾਜ_Alloc(p_ਪੰਕਤੀ_ਸੈੱਟ);
        ਜੇ( p_ਰੁੱਖ ){
          p_ਰੁੱਖ->v = 0;
          p_ਰੁੱਖ->p_ਸੱਜਾ = 0;
          p_ਰੁੱਖ->p_ਖੱਬਾ = ਪੰਕਤੀ_ਸੈੱਟ_List_ਮੰਜ਼ਿਲ_ਰੁੱਖ(p);
        }
      }
      p_ਪੰਕਤੀ_ਸੈੱਟ->p_ਇੰਦਰਾਜ = 0;
      p_ਪੰਕਤੀ_ਸੈੱਟ->p_ਆਖਰੀ = 0;
      p_ਪੰਕਤੀ_ਸੈੱਟ->rsFlags |= ROWSET_SORTED;
    }
    p_ਪੰਕਤੀ_ਸੈੱਟ->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  ਲਈ(p_ਰੁੱਖ = p_ਪੰਕਤੀ_ਸੈੱਟ->pForest; p_ਰੁੱਖ; p_ਰੁੱਖ=p_ਰੁੱਖ->p_ਸੱਜਾ){
    p = p_ਰੁੱਖ->p_ਖੱਬਾ;
    ਜਦੋਂ( p ){
      ਜੇ( p->v<iRowid ){
        p = p->p_ਸੱਜਾ;
      }ਨਹੀਂਤਾਂ ਜੇ( p->v>iRowid ){
        p = p->p_ਖੱਬਾ;
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ 1;
      }
    }
  }
  ਵਾਪਸ 0;
}
