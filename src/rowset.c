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
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/ಪರಿಮಾಣ(ಸಂರಚನಾ RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
ಸಂರಚನಾ RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  ಸಂರಚನಾ RowSetEntry *p_ಬಲ;   /* Right subtree (larger entries) or list */
  ಸಂರಚನಾ RowSetEntry *p_ಎಡ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
ಸಂರಚನಾ RowSetChunk {
  ಸಂರಚನಾ RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  ಸಂರಚನಾ RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
ಸಂರಚನಾ RowSet {
  ಸಂರಚನಾ RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  ಸಂರಚನಾ RowSetEntry *pEntry;    /* List of entries using pRight */
  ಸಂರಚನಾ RowSetEntry *pLast;     /* Last entry on the pEntry list */
  ಸಂರಚನಾ RowSetEntry *pFresh;    /* Source of new entry objects */
  ಸಂರಚನಾ RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  ಪೂರ್ಣಾಙ್ಕ iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#define ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#define ROWSET_NEXT    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
RowSet *sqlite3RowSetInit(sqlite3 *db){
  RowSet *p = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(*p));
  ಯದಿ( p ){
    ಪೂರ್ಣಾಙ್ಕ N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (ಸಂರಚನಾ RowSetEntry*)(ROUND8(ಪರಿಮಾಣ(*p)) + (ವರ್ಣ*)p);
    p->nFresh = (u16)((N - ROUND8(ಪರಿಮಾಣ(*p)))/ಪರಿಮಾಣ(ಸಂರಚನಾ RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ಶೂನ್ಯ sqlite3RowSetClear(ಶೂನ್ಯ *pArg){
  RowSet *p = (RowSet*)pArg;
  ಸಂರಚನಾ RowSetChunk *pChunk, *pNextChunk;
  ಪ್ರತಿ(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
    pNextChunk = pChunk->pNextChunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->pEntry = 0;
  p->pLast = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ಶೂನ್ಯ sqlite3RowSetDelete(ಶೂನ್ಯ *pArg){
  sqlite3RowSetClear(pArg);
  sqlite3DbFree(((RowSet*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ಸ್ಥಿರ ಸಂರಚನಾ RowSetEntry *rowSetEntryAlloc(RowSet *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಯದಿ( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    ಸಂರಚನಾ RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, ಪರಿಮಾಣ(*pNew));
    ಯದಿ( pNew==0 ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  ಪ್ರತ್ಯಾಗಮ p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
ಶೂನ್ಯ sqlite3_Row_Set_ಸೇರಿಸು(RowSet *p, i64 rowid){
  ಸಂರಚನಾ RowSetEntry *pEntry;  /* The new entry */
  ಸಂರಚನಾ RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  ಪ್ರತಿಜ್ಞಾ( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  ಯದಿ( pEntry==0 ) ಪ್ರತ್ಯಾಗಮ;
  pEntry->v = rowid;
  pEntry->p_ಬಲ = 0;
  pLast = p->pLast;
  ಯದಿ( pLast ){
    ಯದಿ( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_ಬಲ = pEntry;
  }ಅನ್ಯಥಾ{
    p->pEntry = pEntry;
  }
  p->pLast = pEntry;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ಸ್ಥಿರ ಸಂರಚನಾ RowSetEntry *rowSetEntryMerge(
  ಸಂರಚನಾ RowSetEntry *pA,    /* First sorted list to be merged */
  ಸಂರಚನಾ RowSetEntry *pB     /* Second sorted list to be merged */
){
  ಸಂರಚನಾ RowSetEntry head;
  ಸಂರಚನಾ RowSetEntry *pTail;

  pTail = &head;
  ಪ್ರತಿಜ್ಞಾ( pA!=0 && pB!=0 );
  ಪ್ರತಿ(;;){
    ಪ್ರತಿಜ್ಞಾ( pA->p_ಬಲ==0 || pA->v<=pA->p_ಬಲ->v );
    ಪ್ರತಿಜ್ಞಾ( pB->p_ಬಲ==0 || pB->v<=pB->p_ಬಲ->v );
    ಯದಿ( pA->v<=pB->v ){
      ಯದಿ( pA->v<pB->v ) pTail = pTail->p_ಬಲ = pA;
      pA = pA->p_ಬಲ;
      ಯದಿ( pA==0 ){
        pTail->p_ಬಲ = pB;
        ಭಙ್ಗ;
      }
    }ಅನ್ಯಥಾ{
      pTail = pTail->p_ಬಲ = pB;
      pB = pB->p_ಬಲ;
      ಯದಿ( pB==0 ){
        pTail->p_ಬಲ = pA;
        ಭಙ್ಗ;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ head.p_ಬಲ;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ಸ್ಥಿರ ಸಂರಚನಾ RowSetEntry *rowSetEntrySort(ಸಂರಚನಾ RowSetEntry *pIn){
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಸಂರಚನಾ RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, ಪರಿಮಾಣ(aBucket));
  ಯಾವತ್( pIn ){
    pNext = pIn->p_ಬಲ;
    pIn->p_ಬಲ = 0;
    ಪ್ರತಿ(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  ಪ್ರತಿ(i=1; i<ಪರಿಮಾಣ(aBucket)/ಪರಿಮಾಣ(aBucket[0]); i++){
    ಯದಿ( aBucket[i]==0 ) ಅನುವರ್ತ;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  ಪ್ರತ್ಯಾಗಮ pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ಸ್ಥಿರ ಶೂನ್ಯ row_Set_ವೃಕ್ಷ_To_List(
  ಸಂರಚನಾ RowSetEntry *pIn,         /* Root of the input tree */
  ಸಂರಚನಾ RowSetEntry **ppFirst,    /* Write head of the output list here */
  ಸಂರಚನಾ RowSetEntry **ppLast      /* Write tail of the output list here */
){
  ಪ್ರತಿಜ್ಞಾ( pIn!=0 );
  ಯದಿ( pIn->p_ಎಡ ){
    ಸಂರಚನಾ RowSetEntry *p;
    row_Set_ವೃಕ್ಷ_To_List(pIn->p_ಎಡ, ppFirst, &p);
    p->p_ಬಲ = pIn;
  }ಅನ್ಯಥಾ{
    *ppFirst = pIn;
  }
  ಯದಿ( pIn->p_ಬಲ ){
    row_Set_ವೃಕ್ಷ_To_List(pIn->p_ಬಲ, &pIn->p_ಬಲ, ppLast);
  }ಅನ್ಯಥಾ{
    *ppLast = pIn;
  }
  ಪ್ರತಿಜ್ಞಾ( (*ppLast)->p_ಬಲ==0 );
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
ಸ್ಥಿರ ಸಂರಚನಾ RowSetEntry *row_Set_NDeep_ವೃಕ್ಷ(
  ಸಂರಚನಾ RowSetEntry **ppList,
  ಪೂರ್ಣಾಙ್ಕ iDepth
){
  ಸಂರಚನಾ RowSetEntry *p;         /* Root of the new tree */
  ಸಂರಚನಾ RowSetEntry *p_ಎಡ;     /* Left subtree */
  ಯದಿ( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_ಎಡ = row_Set_NDeep_ವೃಕ್ಷ(ppList, iDepth-1);
    p = *ppList;
    ಯದಿ( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      ಪ್ರತ್ಯಾಗಮ p_ಎಡ;
    }
    p->p_ಎಡ = p_ಎಡ;
    *ppList = p->p_ಬಲ;
    p->p_ಬಲ = row_Set_NDeep_ವೃಕ್ಷ(ppList, iDepth-1);
  }ಅನ್ಯಥಾ{
    p = *ppList;
    *ppList = p->p_ಬಲ;
    p->p_ಎಡ = p->p_ಬಲ = 0;
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ಸ್ಥಿರ ಸಂರಚನಾ RowSetEntry *row_Set_List_To_ವೃಕ್ಷ(ಸಂರಚನಾ RowSetEntry *pList){
  ಪೂರ್ಣಾಙ್ಕ iDepth;           /* Depth of the tree so far */
  ಸಂರಚನಾ RowSetEntry *p;       /* Current tree root */
  ಸಂರಚನಾ RowSetEntry *p_ಎಡ;   /* Left subtree */

  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  p = pList;
  pList = p->p_ಬಲ;
  p->p_ಎಡ = p->p_ಬಲ = 0;
  ಪ್ರತಿ(iDepth=1; pList; iDepth++){
    p_ಎಡ = p;
    p = pList;
    pList = p->p_ಬಲ;
    p->p_ಎಡ = p_ಎಡ;
    p->p_ಬಲ = row_Set_NDeep_ವೃಕ್ಷ(&pList, iDepth);
  }
  ಪ್ರತ್ಯಾಗಮ p;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  ಯದಿ( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    ಯದಿ( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  ಯದಿ( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_ಬಲ;
    ಯದಿ( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3RowSetTest(RowSet *pRowSet, ಪೂರ್ಣಾಙ್ಕ iBatch, sqlite3_int64 iRowid){
  ಸಂರಚನಾ RowSetEntry *p, *p_ವೃಕ್ಷ;

  /* This routine is never called after sqlite3RowSetNext() */
  ಪ್ರತಿಜ್ಞಾ( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  ಯದಿ( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    ಯದಿ( p ){
      ಸಂರಚನಾ RowSetEntry **pp_Prev_ವೃಕ್ಷ = &pRowSet->pForest;
      ಯದಿ( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      ಪ್ರತಿ(p_ವೃಕ್ಷ = pRowSet->pForest; p_ವೃಕ್ಷ; p_ವೃಕ್ಷ=p_ವೃಕ್ಷ->p_ಬಲ){
        pp_Prev_ವೃಕ್ಷ = &p_ವೃಕ್ಷ->p_ಬಲ;
        ಯದಿ( p_ವೃಕ್ಷ->p_ಎಡ==0 ){
          p_ವೃಕ್ಷ->p_ಎಡ = row_Set_List_To_ವೃಕ್ಷ(p);
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ{
          ಸಂರಚನಾ RowSetEntry *pAux, *pTail;
          row_Set_ವೃಕ್ಷ_To_List(p_ವೃಕ್ಷ->p_ಎಡ, &pAux, &pTail);
          p_ವೃಕ್ಷ->p_ಎಡ = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      ಯದಿ( p_ವೃಕ್ಷ==0 ){
        *pp_Prev_ವೃಕ್ಷ = p_ವೃಕ್ಷ = rowSetEntryAlloc(pRowSet);
        ಯದಿ( p_ವೃಕ್ಷ ){
          p_ವೃಕ್ಷ->v = 0;
          p_ವೃಕ್ಷ->p_ಬಲ = 0;
          p_ವೃಕ್ಷ->p_ಎಡ = row_Set_List_To_ವೃಕ್ಷ(p);
        }
      }
      pRowSet->pEntry = 0;
      pRowSet->pLast = 0;
      pRowSet->rsFlags |= ROWSET_SORTED;
    }
    pRowSet->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  ಪ್ರತಿ(p_ವೃಕ್ಷ = pRowSet->pForest; p_ವೃಕ್ಷ; p_ವೃಕ್ಷ=p_ವೃಕ್ಷ->p_ಬಲ){
    p = p_ವೃಕ್ಷ->p_ಎಡ;
    ಯಾವತ್( p ){
      ಯದಿ( p->v<iRowid ){
        p = p->p_ಬಲ;
      }ಅನ್ಯಥಾ ಯದಿ( p->v>iRowid ){
        p = p->p_ಎಡ;
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ 1;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}
