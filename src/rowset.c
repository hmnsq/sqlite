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
#include "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/parimaa_na(sa_nracanaa RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
sa_nracanaa RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  sa_nracanaa RowSetEntry *pRight;   /* Right subtree (larger entries) or list */
  sa_nracanaa RowSetEntry *pLeft;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
sa_nracanaa RowSetChunk {
  sa_nracanaa RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  sa_nracanaa RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
sa_nracanaa RowSet {
  sa_nracanaa RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  sa_nracanaa RowSetEntry *pEntry;    /* List of entries using pRight */
  sa_nracanaa RowSetEntry *pLast;     /* Last entry on the pEntry list */
  sa_nracanaa RowSetEntry *pFresh;    /* Source of new entry objects */
  sa_nracanaa RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  puur_naa_nka iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, parimaa_na(*p));
  ᤀᤡ᤺( p ){
    puur_naa_nka N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (sa_nracanaa RowSetEntry*)(ROUND8(parimaa_na(*p)) + (var_na*)p);
    p->nFresh = (u16)((N - ROUND8(parimaa_na(*p)))/parimaa_na(sa_nracanaa RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  ᤀᤡᤴ p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
void sqlite3RowSetClear(void *pArg){
  RowSet *p = (RowSet*)pArg;
  sa_nracanaa RowSetChunk *pChunk, *pNextChunk;
  for(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
void sqlite3RowSetDelete(void *pArg){
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
static sa_nracanaa RowSetEntry *rowSetEntryAlloc(RowSet *p){
  assert( p!=0 );
  ᤀᤡ᤺( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    sa_nracanaa RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, parimaa_na(*pNew));
    ᤀᤡ᤺( pNew==0 ){
      ᤀᤡᤴ 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  ᤀᤡᤴ p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
void sqlite3RowSetInsert(RowSet *p, i64 rowid){
  sa_nracanaa RowSetEntry *pEntry;  /* The new entry */
  sa_nracanaa RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  assert( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  ᤀᤡ᤺( pEntry==0 ) ᤀᤡᤴ;
  pEntry->v = rowid;
  pEntry->pRight = 0;
  pLast = p->pLast;
  ᤀᤡ᤺( pLast ){
    ᤀᤡ᤺( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->pRight = pEntry;
  }ᤀᤡ᤺ᤆᤡ{
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
static sa_nracanaa RowSetEntry *rowSetEntryMerge(
  sa_nracanaa RowSetEntry *pA,    /* First sorted list to be merged */
  sa_nracanaa RowSetEntry *pB     /* Second sorted list to be merged */
){
  sa_nracanaa RowSetEntry head;
  sa_nracanaa RowSetEntry *pTail;

  pTail = &head;
  assert( pA!=0 && pB!=0 );
  for(;;){
    assert( pA->pRight==0 || pA->v<=pA->pRight->v );
    assert( pB->pRight==0 || pB->v<=pB->pRight->v );
    ᤀᤡ᤺( pA->v<=pB->v ){
      ᤀᤡ᤺( pA->v<pB->v ) pTail = pTail->pRight = pA;
      pA = pA->pRight;
      ᤀᤡ᤺( pA==0 ){
        pTail->pRight = pB;
        bha_nga;
      }
    }ᤀᤡ᤺ᤆᤡ{
      pTail = pTail->pRight = pB;
      pB = pB->pRight;
      ᤀᤡ᤺( pB==0 ){
        pTail->pRight = pA;
        bha_nga;
      }
    }
  }
  ᤀᤡᤴ head.pRight;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
static sa_nracanaa RowSetEntry *rowSetEntrySort(sa_nracanaa RowSetEntry *pIn){
  unsigned puur_naa_nka i;
  sa_nracanaa RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, parimaa_na(aBucket));
  ᤀᤠ᤺( pIn ){
    pNext = pIn->pRight;
    pIn->pRight = 0;
    for(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  for(i=1; i<parimaa_na(aBucket)/parimaa_na(aBucket[0]); i++){
    ᤀᤡ᤺( aBucket[i]==0 ) continue;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  ᤀᤡᤴ pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
static void rowSetTreeToList(
  sa_nracanaa RowSetEntry *pIn,         /* Root of the input tree */
  sa_nracanaa RowSetEntry **ppFirst,    /* Write head of the output list here */
  sa_nracanaa RowSetEntry **ppLast      /* Write tail of the output list here */
){
  assert( pIn!=0 );
  ᤀᤡ᤺( pIn->pLeft ){
    sa_nracanaa RowSetEntry *p;
    rowSetTreeToList(pIn->pLeft, ppFirst, &p);
    p->pRight = pIn;
  }ᤀᤡ᤺ᤆᤡ{
    *ppFirst = pIn;
  }
  ᤀᤡ᤺( pIn->pRight ){
    rowSetTreeToList(pIn->pRight, &pIn->pRight, ppLast);
  }ᤀᤡ᤺ᤆᤡ{
    *ppLast = pIn;
  }
  assert( (*ppLast)->pRight==0 );
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
static sa_nracanaa RowSetEntry *rowSetNDeepTree(
  sa_nracanaa RowSetEntry **ppList,
  puur_naa_nka iDepth
){
  sa_nracanaa RowSetEntry *p;         /* Root of the new tree */
  sa_nracanaa RowSetEntry *pLeft;     /* Left subtree */
  ᤀᤡ᤺( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    ᤀᤡᤴ 0;
  }
  ᤀᤡ᤺( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    pLeft = rowSetNDeepTree(ppList, iDepth-1);
    p = *ppList;
    ᤀᤡ᤺( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      ᤀᤡᤴ pLeft;
    }
    p->pLeft = pLeft;
    *ppList = p->pRight;
    p->pRight = rowSetNDeepTree(ppList, iDepth-1);
  }ᤀᤡ᤺ᤆᤡ{
    p = *ppList;
    *ppList = p->pRight;
    p->pLeft = p->pRight = 0;
  }
  ᤀᤡᤴ p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
static sa_nracanaa RowSetEntry *rowSetListToTree(sa_nracanaa RowSetEntry *pList){
  puur_naa_nka iDepth;           /* Depth of the tree so far */
  sa_nracanaa RowSetEntry *p;       /* Current tree root */
  sa_nracanaa RowSetEntry *pLeft;   /* Left subtree */

  assert( pList!=0 );
  p = pList;
  pList = p->pRight;
  p->pLeft = p->pRight = 0;
  for(iDepth=1; pList; iDepth++){
    pLeft = p;
    p = pList;
    pList = p->pRight;
    p->pLeft = pLeft;
    p->pRight = rowSetNDeepTree(&pList, iDepth);
  }
  ᤀᤡᤴ p;
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
puur_naa_nka sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  assert( p!=0 );
  assert( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  ᤀᤡ᤺( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    ᤀᤡ᤺( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  ᤀᤡ᤺( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->pRight;
    ᤀᤡ᤺( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    ᤀᤡᤴ 1;
  }ᤀᤡ᤺ᤆᤡ{
    ᤀᤡᤴ 0;
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
puur_naa_nka sqlite3RowSetTest(RowSet *pRowSet, puur_naa_nka iBatch, sqlite3_int64 iRowid){
  sa_nracanaa RowSetEntry *p, *pTree;

  /* This routine is never called after sqlite3RowSetNext() */
  assert( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  ᤀᤡ᤺( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    ᤀᤡ᤺( p ){
      sa_nracanaa RowSetEntry **ppPrevTree = &pRowSet->pForest;
      ᤀᤡ᤺( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      for(pTree = pRowSet->pForest; pTree; pTree=pTree->pRight){
        ppPrevTree = &pTree->pRight;
        ᤀᤡ᤺( pTree->pLeft==0 ){
          pTree->pLeft = rowSetListToTree(p);
          bha_nga;
        }ᤀᤡ᤺ᤆᤡ{
          sa_nracanaa RowSetEntry *pAux, *pTail;
          rowSetTreeToList(pTree->pLeft, &pAux, &pTail);
          pTree->pLeft = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      ᤀᤡ᤺( pTree==0 ){
        *ppPrevTree = pTree = rowSetEntryAlloc(pRowSet);
        ᤀᤡ᤺( pTree ){
          pTree->v = 0;
          pTree->pRight = 0;
          pTree->pLeft = rowSetListToTree(p);
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
  for(pTree = pRowSet->pForest; pTree; pTree=pTree->pRight){
    p = pTree->pLeft;
    ᤀᤠ᤺( p ){
      ᤀᤡ᤺( p->v<iRowid ){
        p = p->pRight;
      }ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺( p->v>iRowid ){
        p = p->pLeft;
      }ᤀᤡ᤺ᤆᤡ{
        ᤀᤡᤴ 1;
      }
    }
  }
  ᤀᤡᤴ 0;
}
