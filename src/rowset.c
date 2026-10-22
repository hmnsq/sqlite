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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ROWSET_ಹಂಚಿಕೆ_ಗಾತ್ರ 1024

/*
** The number of rowset entries per allocation chunk.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ROWSET_ನಮೂದು_ಪ್ರತಿ_CHUNK  \
                       ((ROWSET_ಹಂಚಿಕೆ_ಗಾತ್ರ-8)/ಗಾತ್ರ(ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು {           
  i64 v;                        /* ROWID value for this entry */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಬಲ;   /* Right subtree (larger entries) or list */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಎಡ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_Chunk {
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_Chunk *p_ಮುಂದಿನ_Chunk;        /* Next chunk on list of them all */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು a_ನಮೂದು[ROWSET_ನಮೂದು_ಪ್ರತಿ_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ {
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ನಮೂದು;    /* List of entries using pRight */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಕೊನೆಯ;     /* Last entry on the pEntry list */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *pFresh;    /* Source of new entry objects */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  ಪೂರ್ಣಾಂಕ iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#ವ್ಯಾಖ್ಯಾನಿಸು ROWSET_ಮುಂದಿನ    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
ಅಡ್ಡಸಾಲು_ಗಣ *sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Init(sqlite3 *db){
  ಅಡ್ಡಸಾಲು_ಗಣ *p = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(*p));
  ಆದರೆ( p ){
    ಪೂರ್ಣಾಂಕ N = sqlite3_Db_Malloc_ಗಾತ್ರ(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_ನಮೂದು = 0;
    p->p_ಕೊನೆಯ = 0;
    p->pForest = 0;
    p->pFresh = (ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು*)(ROUND8(ಗಾತ್ರ(*p)) + (ಅಕ್ಷರ*)p);
    p->nFresh = (u16)((N - ROUND8(ಗಾತ್ರ(*p)))/ಗಾತ್ರ(ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ಶೂನ್ಯ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Clear(ಶೂನ್ಯ *pArg){
  ಅಡ್ಡಸಾಲು_ಗಣ *p = (ಅಡ್ಡಸಾಲು_ಗಣ*)pArg;
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_Chunk *pChunk, *p_ಮುಂದಿನ_Chunk;
  ಗಾಗಿ(pChunk=p->pChunk; pChunk; pChunk = p_ಮುಂದಿನ_Chunk){
    p_ಮುಂದಿನ_Chunk = pChunk->p_ಮುಂದಿನ_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_ನಮೂದು = 0;
  p->p_ಕೊನೆಯ = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ಶೂನ್ಯ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Delete(ಶೂನ್ಯ *pArg){
  sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Clear(pArg);
  sqlite3DbFree(((ಅಡ್ಡಸಾಲು_ಗಣ*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ಸ್ಥಿರ ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_Alloc(ಅಡ್ಡಸಾಲು_ಗಣ *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಆದರೆ( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, ಗಾತ್ರ(*pNew));
    ಆದರೆ( pNew==0 ){
      ಹಿಂತಿರುಗಿ 0;
    }
    pNew->p_ಮುಂದಿನ_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_ನಮೂದು;
    p->nFresh = ROWSET_ನಮೂದು_ಪ್ರತಿ_CHUNK;
  }
  p->nFresh--;
  ಹಿಂತಿರುಗಿ p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
ಶೂನ್ಯ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಸೇರಿಸು(ಅಡ್ಡಸಾಲು_ಗಣ *p, i64 rowid){
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ನಮೂದು;  /* The new entry */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಕೊನೆಯ;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  ಪ್ರತಿಜ್ಞಾ( p!=0 && (p->rsFlags & ROWSET_ಮುಂದಿನ)==0 );

  p_ನಮೂದು = ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_Alloc(p);
  ಆದರೆ( p_ನಮೂದು==0 ) ಹಿಂತಿರುಗಿ;
  p_ನಮೂದು->v = rowid;
  p_ನಮೂದು->p_ಬಲ = 0;
  p_ಕೊನೆಯ = p->p_ಕೊನೆಯ;
  ಆದರೆ( p_ಕೊನೆಯ ){
    ಆದರೆ( rowid<=p_ಕೊನೆಯ->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_ಕೊನೆಯ->p_ಬಲ = p_ನಮೂದು;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->p_ನಮೂದು = p_ನಮೂದು;
  }
  p->p_ಕೊನೆಯ = p_ನಮೂದು;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ಸ್ಥಿರ ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಲೀನ(
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *pA,    /* First sorted list to be merged */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *pB     /* Second sorted list to be merged */
){
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು ತಲೆ;
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಬಾಲ;

  p_ಬಾಲ = &ತಲೆ;
  ಪ್ರತಿಜ್ಞಾ( pA!=0 && pB!=0 );
  ಗಾಗಿ(;;){
    ಪ್ರತಿಜ್ಞಾ( pA->p_ಬಲ==0 || pA->v<=pA->p_ಬಲ->v );
    ಪ್ರತಿಜ್ಞಾ( pB->p_ಬಲ==0 || pB->v<=pB->p_ಬಲ->v );
    ಆದರೆ( pA->v<=pB->v ){
      ಆದರೆ( pA->v<pB->v ) p_ಬಾಲ = p_ಬಾಲ->p_ಬಲ = pA;
      pA = pA->p_ಬಲ;
      ಆದರೆ( pA==0 ){
        p_ಬಾಲ->p_ಬಲ = pB;
        ನಿಲ್ಲಿಸು;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಬಾಲ = p_ಬಾಲ->p_ಬಲ = pB;
      pB = pB->p_ಬಲ;
      ಆದರೆ( pB==0 ){
        p_ಬಾಲ->p_ಬಲ = pA;
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ ತಲೆ.p_ಬಲ;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ಸ್ಥಿರ ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಂಗಡಿಸು(ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಒಳ){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಮುಂದಿನ, *a_ಪಾತ್ರೆ[40];

  memset(a_ಪಾತ್ರೆ, 0, ಗಾತ್ರ(a_ಪಾತ್ರೆ));
  ಆಗ( p_ಒಳ ){
    p_ಮುಂದಿನ = p_ಒಳ->p_ಬಲ;
    p_ಒಳ->p_ಬಲ = 0;
    ಗಾಗಿ(i=0; a_ಪಾತ್ರೆ[i]; i++){
      p_ಒಳ = ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಲೀನ(a_ಪಾತ್ರೆ[i], p_ಒಳ);
      a_ಪಾತ್ರೆ[i] = 0;
    }
    a_ಪಾತ್ರೆ[i] = p_ಒಳ;
    p_ಒಳ = p_ಮುಂದಿನ;
  }
  p_ಒಳ = a_ಪಾತ್ರೆ[0];
  ಗಾಗಿ(i=1; i<ಗಾತ್ರ(a_ಪಾತ್ರೆ)/ಗಾತ್ರ(a_ಪಾತ್ರೆ[0]); i++){
    ಆದರೆ( a_ಪಾತ್ರೆ[i]==0 ) ಮುಂದುವರಿ;
    p_ಒಳ = p_ಒಳ ? ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಲೀನ(p_ಒಳ, a_ಪಾತ್ರೆ[i]) : a_ಪಾತ್ರೆ[i];
  }
  ಹಿಂತಿರುಗಿ p_ಒಳ;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಅಡ್ಡಸಾಲು_ಗಣ_ವೃಕ್ಷ_ಗಮ್ಯ_List(
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಒಳ,         /* Root of the input tree */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು **pp_ಮೊದಲ,    /* Write head of the output list here */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು **pp_ಕೊನೆಯ      /* Write tail of the output list here */
){
  ಪ್ರತಿಜ್ಞಾ( p_ಒಳ!=0 );
  ಆದರೆ( p_ಒಳ->p_ಎಡ ){
    ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p;
    ಅಡ್ಡಸಾಲು_ಗಣ_ವೃಕ್ಷ_ಗಮ್ಯ_List(p_ಒಳ->p_ಎಡ, pp_ಮೊದಲ, &p);
    p->p_ಬಲ = p_ಒಳ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pp_ಮೊದಲ = p_ಒಳ;
  }
  ಆದರೆ( p_ಒಳ->p_ಬಲ ){
    ಅಡ್ಡಸಾಲು_ಗಣ_ವೃಕ್ಷ_ಗಮ್ಯ_List(p_ಒಳ->p_ಬಲ, &p_ಒಳ->p_ಬಲ, pp_ಕೊನೆಯ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pp_ಕೊನೆಯ = p_ಒಳ;
  }
  ಪ್ರತಿಜ್ಞಾ( (*pp_ಕೊನೆಯ)->p_ಬಲ==0 );
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
ಸ್ಥಿರ ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *ಅಡ್ಡಸಾಲು_ಗಣ_NDeep_ವೃಕ್ಷ(
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು **ppList,
  ಪೂರ್ಣಾಂಕ i_ಆಳ
){
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p;         /* Root of the new tree */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಎಡ;     /* Left subtree */
  ಆದರೆ( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( i_ಆಳ>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_ಎಡ = ಅಡ್ಡಸಾಲು_ಗಣ_NDeep_ವೃಕ್ಷ(ppList, i_ಆಳ-1);
    p = *ppList;
    ಆದರೆ( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      ಹಿಂತಿರುಗಿ p_ಎಡ;
    }
    p->p_ಎಡ = p_ಎಡ;
    *ppList = p->p_ಬಲ;
    p->p_ಬಲ = ಅಡ್ಡಸಾಲು_ಗಣ_NDeep_ವೃಕ್ಷ(ppList, i_ಆಳ-1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = *ppList;
    *ppList = p->p_ಬಲ;
    p->p_ಎಡ = p->p_ಬಲ = 0;
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ಸ್ಥಿರ ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *ಅಡ್ಡಸಾಲು_ಗಣ_List_ಗಮ್ಯ_ವೃಕ್ಷ(ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *pList){
  ಪೂರ್ಣಾಂಕ i_ಆಳ;           /* Depth of the tree so far */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p;       /* Current tree root */
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p_ಎಡ;   /* Left subtree */

  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  p = pList;
  pList = p->p_ಬಲ;
  p->p_ಎಡ = p->p_ಬಲ = 0;
  ಗಾಗಿ(i_ಆಳ=1; pList; i_ಆಳ++){
    p_ಎಡ = p;
    p = pList;
    pList = p->p_ಬಲ;
    p->p_ಎಡ = p_ಎಡ;
    p->p_ಬಲ = ಅಡ್ಡಸಾಲು_ಗಣ_NDeep_ವೃಕ್ಷ(&pList, i_ಆಳ);
  }
  ಹಿಂತಿರುಗಿ p;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಮುಂದಿನ(ಅಡ್ಡಸಾಲು_ಗಣ *p, i64 *pRowid){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  ಆದರೆ( (p->rsFlags & ROWSET_ಮುಂದಿನ)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    ಆದರೆ( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_ನಮೂದು = ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಂಗಡಿಸು(p->p_ನಮೂದು);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_ಮುಂದಿನ;
  }

  /* Return the next entry on the list */
  ಆದರೆ( p->p_ನಮೂದು ){
    *pRowid = p->p_ನಮೂದು->v;
    p->p_ನಮೂದು = p->p_ನಮೂದು->p_ಬಲ;
    ಆದರೆ( p->p_ನಮೂದು==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Clear(p);
    }
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Test(ಅಡ್ಡಸಾಲು_ಗಣ *p_ಅಡ್ಡಸಾಲು_ಗಣ, ಪೂರ್ಣಾಂಕ iBatch, sqlite3_int64 iRowid){
  ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *p, *p_ವೃಕ್ಷ;

  /* This routine is never called after sqlite3RowSetNext() */
  ಪ್ರತಿಜ್ಞಾ( p_ಅಡ್ಡಸಾಲು_ಗಣ!=0 && (p_ಅಡ್ಡಸಾಲು_ಗಣ->rsFlags & ROWSET_ಮುಂದಿನ)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  ಆದರೆ( iBatch!=p_ಅಡ್ಡಸಾಲು_ಗಣ->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_ಅಡ್ಡಸಾಲು_ಗಣ->p_ನಮೂದು;
    ಆದರೆ( p ){
      ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು **pp_Prev_ವೃಕ್ಷ = &p_ಅಡ್ಡಸಾಲು_ಗಣ->pForest;
      ಆದರೆ( (p_ಅಡ್ಡಸಾಲು_ಗಣ->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಂಗಡಿಸು(p);
      }
      ಗಾಗಿ(p_ವೃಕ್ಷ = p_ಅಡ್ಡಸಾಲು_ಗಣ->pForest; p_ವೃಕ್ಷ; p_ವೃಕ್ಷ=p_ವೃಕ್ಷ->p_ಬಲ){
        pp_Prev_ವೃಕ್ಷ = &p_ವೃಕ್ಷ->p_ಬಲ;
        ಆದರೆ( p_ವೃಕ್ಷ->p_ಎಡ==0 ){
          p_ವೃಕ್ಷ->p_ಎಡ = ಅಡ್ಡಸಾಲು_ಗಣ_List_ಗಮ್ಯ_ವೃಕ್ಷ(p);
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು *pAux, *p_ಬಾಲ;
          ಅಡ್ಡಸಾಲು_ಗಣ_ವೃಕ್ಷ_ಗಮ್ಯ_List(p_ವೃಕ್ಷ->p_ಎಡ, &pAux, &p_ಬಾಲ);
          p_ವೃಕ್ಷ->p_ಎಡ = 0;
          p = ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_ವಿಲೀನ(pAux, p);
        }
      }
      ಆದರೆ( p_ವೃಕ್ಷ==0 ){
        *pp_Prev_ವೃಕ್ಷ = p_ವೃಕ್ಷ = ಅಡ್ಡಸಾಲು_ಗಣ_ನಮೂದು_Alloc(p_ಅಡ್ಡಸಾಲು_ಗಣ);
        ಆದರೆ( p_ವೃಕ್ಷ ){
          p_ವೃಕ್ಷ->v = 0;
          p_ವೃಕ್ಷ->p_ಬಲ = 0;
          p_ವೃಕ್ಷ->p_ಎಡ = ಅಡ್ಡಸಾಲು_ಗಣ_List_ಗಮ್ಯ_ವೃಕ್ಷ(p);
        }
      }
      p_ಅಡ್ಡಸಾಲು_ಗಣ->p_ನಮೂದು = 0;
      p_ಅಡ್ಡಸಾಲು_ಗಣ->p_ಕೊನೆಯ = 0;
      p_ಅಡ್ಡಸಾಲು_ಗಣ->rsFlags |= ROWSET_SORTED;
    }
    p_ಅಡ್ಡಸಾಲು_ಗಣ->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  ಗಾಗಿ(p_ವೃಕ್ಷ = p_ಅಡ್ಡಸಾಲು_ಗಣ->pForest; p_ವೃಕ್ಷ; p_ವೃಕ್ಷ=p_ವೃಕ್ಷ->p_ಬಲ){
    p = p_ವೃಕ್ಷ->p_ಎಡ;
    ಆಗ( p ){
      ಆದರೆ( p->v<iRowid ){
        p = p->p_ಬಲ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->v>iRowid ){
        p = p->p_ಎಡ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ 1;
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}
