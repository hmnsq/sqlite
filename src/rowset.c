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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/ଆକାର(ସଂରଚନା RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
ସଂରଚନା RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  ସଂରଚନା RowSetEntry *p_ଡାହାଣ;   /* Right subtree (larger entries) or list */
  ସଂରଚନା RowSetEntry *p_ବାମ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
ସଂରଚନା RowSetChunk {
  ସଂରଚନା RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  ସଂରଚନା RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
ସଂରଚନା RowSet {
  ସଂରଚନା RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  ସଂରଚନା RowSetEntry *pEntry;    /* List of entries using pRight */
  ସଂରଚନା RowSetEntry *pLast;     /* Last entry on the pEntry list */
  ସଂରଚନା RowSetEntry *pFresh;    /* Source of new entry objects */
  ସଂରଚନା RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, ଆକାର(*p));
  ଯଦି( p ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (ସଂରଚନା RowSetEntry*)(ROUND8(ଆକାର(*p)) + (ଅକ୍ଷର*)p);
    p->nFresh = (u16)((N - ROUND8(ଆକାର(*p)))/ଆକାର(ସଂରଚନା RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  ଫେରନ୍ତୁ p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ଶୂନ୍ୟ sqlite3RowSetClear(ଶୂନ୍ୟ *pArg){
  RowSet *p = (RowSet*)pArg;
  ସଂରଚନା RowSetChunk *pChunk, *pNextChunk;
  ପାଇଁ(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
ଶୂନ୍ୟ sqlite3RowSetDelete(ଶୂନ୍ୟ *pArg){
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
ସ୍ଥିର ସଂରଚନା RowSetEntry *rowSetEntryAlloc(RowSet *p){
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଯଦି( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    ସଂରଚନା RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, ଆକାର(*pNew));
    ଯଦି( pNew==0 ){
      ଫେରନ୍ତୁ 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  ଫେରନ୍ତୁ p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
ଶୂନ୍ୟ sqlite3_Row_Set_ଭର୍ତ୍ତି(RowSet *p, i64 rowid){
  ସଂରଚନା RowSetEntry *pEntry;  /* The new entry */
  ସଂରଚନା RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  ପ୍ରତିଜ୍ଞା( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  ଯଦି( pEntry==0 ) ଫେରନ୍ତୁ;
  pEntry->v = rowid;
  pEntry->p_ଡାହାଣ = 0;
  pLast = p->pLast;
  ଯଦି( pLast ){
    ଯଦି( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_ଡାହାଣ = pEntry;
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ସଂରଚନା RowSetEntry *rowSetEntryMerge(
  ସଂରଚନା RowSetEntry *pA,    /* First sorted list to be merged */
  ସଂରଚନା RowSetEntry *pB     /* Second sorted list to be merged */
){
  ସଂରଚନା RowSetEntry head;
  ସଂରଚନା RowSetEntry *pTail;

  pTail = &head;
  ପ୍ରତିଜ୍ଞା( pA!=0 && pB!=0 );
  ପାଇଁ(;;){
    ପ୍ରତିଜ୍ଞା( pA->p_ଡାହାଣ==0 || pA->v<=pA->p_ଡାହାଣ->v );
    ପ୍ରତିଜ୍ଞା( pB->p_ଡାହାଣ==0 || pB->v<=pB->p_ଡାହାଣ->v );
    ଯଦି( pA->v<=pB->v ){
      ଯଦି( pA->v<pB->v ) pTail = pTail->p_ଡାହାଣ = pA;
      pA = pA->p_ଡାହାଣ;
      ଯଦି( pA==0 ){
        pTail->p_ଡାହାଣ = pB;
        ଭାଙ୍ଗ;
      }
    }ଅନ୍ୟଥା{
      pTail = pTail->p_ଡାହାଣ = pB;
      pB = pB->p_ଡାହାଣ;
      ଯଦି( pB==0 ){
        pTail->p_ଡାହାଣ = pA;
        ଭାଙ୍ଗ;
      }
    }
  }
  ଫେରନ୍ତୁ head.p_ଡାହାଣ;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ସ୍ଥିର ସଂରଚନା RowSetEntry *rowSetEntrySort(ସଂରଚନା RowSetEntry *pIn){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ସଂରଚନା RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, ଆକାର(aBucket));
  ଯେତେବେଳେ( pIn ){
    pNext = pIn->p_ଡାହାଣ;
    pIn->p_ଡାହାଣ = 0;
    ପାଇଁ(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  ପାଇଁ(i=1; i<ଆକାର(aBucket)/ଆକାର(aBucket[0]); i++){
    ଯଦି( aBucket[i]==0 ) ଜାରି;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  ଫେରନ୍ତୁ pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ସ୍ଥିର ଶୂନ୍ୟ row_Set_ବୃକ୍ଷ_To_List(
  ସଂରଚନା RowSetEntry *pIn,         /* Root of the input tree */
  ସଂରଚନା RowSetEntry **ppFirst,    /* Write head of the output list here */
  ସଂରଚନା RowSetEntry **ppLast      /* Write tail of the output list here */
){
  ପ୍ରତିଜ୍ଞା( pIn!=0 );
  ଯଦି( pIn->p_ବାମ ){
    ସଂରଚନା RowSetEntry *p;
    row_Set_ବୃକ୍ଷ_To_List(pIn->p_ବାମ, ppFirst, &p);
    p->p_ଡାହାଣ = pIn;
  }ଅନ୍ୟଥା{
    *ppFirst = pIn;
  }
  ଯଦି( pIn->p_ଡାହାଣ ){
    row_Set_ବୃକ୍ଷ_To_List(pIn->p_ଡାହାଣ, &pIn->p_ଡାହାଣ, ppLast);
  }ଅନ୍ୟଥା{
    *ppLast = pIn;
  }
  ପ୍ରତିଜ୍ଞା( (*ppLast)->p_ଡାହାଣ==0 );
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
ସ୍ଥିର ସଂରଚନା RowSetEntry *row_Set_NDeep_ବୃକ୍ଷ(
  ସଂରଚନା RowSetEntry **ppList,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDepth
){
  ସଂରଚନା RowSetEntry *p;         /* Root of the new tree */
  ସଂରଚନା RowSetEntry *p_ବାମ;     /* Left subtree */
  ଯଦି( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_ବାମ = row_Set_NDeep_ବୃକ୍ଷ(ppList, iDepth-1);
    p = *ppList;
    ଯଦି( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      ଫେରନ୍ତୁ p_ବାମ;
    }
    p->p_ବାମ = p_ବାମ;
    *ppList = p->p_ଡାହାଣ;
    p->p_ଡାହାଣ = row_Set_NDeep_ବୃକ୍ଷ(ppList, iDepth-1);
  }ଅନ୍ୟଥା{
    p = *ppList;
    *ppList = p->p_ଡାହାଣ;
    p->p_ବାମ = p->p_ଡାହାଣ = 0;
  }
  ଫେରନ୍ତୁ p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ସ୍ଥିର ସଂରଚନା RowSetEntry *row_Set_List_To_ବୃକ୍ଷ(ସଂରଚନା RowSetEntry *pList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDepth;           /* Depth of the tree so far */
  ସଂରଚନା RowSetEntry *p;       /* Current tree root */
  ସଂରଚନା RowSetEntry *p_ବାମ;   /* Left subtree */

  ପ୍ରତିଜ୍ଞା( pList!=0 );
  p = pList;
  pList = p->p_ଡାହାଣ;
  p->p_ବାମ = p->p_ଡାହାଣ = 0;
  ପାଇଁ(iDepth=1; pList; iDepth++){
    p_ବାମ = p;
    p = pList;
    pList = p->p_ଡାହାଣ;
    p->p_ବାମ = p_ବାମ;
    p->p_ଡାହାଣ = row_Set_NDeep_ବୃକ୍ଷ(&pList, iDepth);
  }
  ଫେରନ୍ତୁ p;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ପ୍ରତିଜ୍ଞା( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  ଯଦି( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    ଯଦି( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  ଯଦି( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_ଡାହାଣ;
    ଯଦି( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3RowSetTest(RowSet *pRowSet, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBatch, sqlite3_int64 iRowid){
  ସଂରଚନା RowSetEntry *p, *p_ବୃକ୍ଷ;

  /* This routine is never called after sqlite3RowSetNext() */
  ପ୍ରତିଜ୍ଞା( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  ଯଦି( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    ଯଦି( p ){
      ସଂରଚନା RowSetEntry **pp_Prev_ବୃକ୍ଷ = &pRowSet->pForest;
      ଯଦି( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      ପାଇଁ(p_ବୃକ୍ଷ = pRowSet->pForest; p_ବୃକ୍ଷ; p_ବୃକ୍ଷ=p_ବୃକ୍ଷ->p_ଡାହାଣ){
        pp_Prev_ବୃକ୍ଷ = &p_ବୃକ୍ଷ->p_ଡାହାଣ;
        ଯଦି( p_ବୃକ୍ଷ->p_ବାମ==0 ){
          p_ବୃକ୍ଷ->p_ବାମ = row_Set_List_To_ବୃକ୍ଷ(p);
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା{
          ସଂରଚନା RowSetEntry *pAux, *pTail;
          row_Set_ବୃକ୍ଷ_To_List(p_ବୃକ୍ଷ->p_ବାମ, &pAux, &pTail);
          p_ବୃକ୍ଷ->p_ବାମ = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      ଯଦି( p_ବୃକ୍ଷ==0 ){
        *pp_Prev_ବୃକ୍ଷ = p_ବୃକ୍ଷ = rowSetEntryAlloc(pRowSet);
        ଯଦି( p_ବୃକ୍ଷ ){
          p_ବୃକ୍ଷ->v = 0;
          p_ବୃକ୍ଷ->p_ଡାହାଣ = 0;
          p_ବୃକ୍ଷ->p_ବାମ = row_Set_List_To_ବୃକ୍ଷ(p);
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
  ପାଇଁ(p_ବୃକ୍ଷ = pRowSet->pForest; p_ବୃକ୍ଷ; p_ବୃକ୍ଷ=p_ବୃକ୍ଷ->p_ଡାହାଣ){
    p = p_ବୃକ୍ଷ->p_ବାମ;
    ଯେତେବେଳେ( p ){
      ଯଦି( p->v<iRowid ){
        p = p->p_ଡାହାଣ;
      }ଅନ୍ୟଥା ଯଦି( p->v>iRowid ){
        p = p->p_ବାମ;
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ 1;
      }
    }
  }
  ଫେରନ୍ତୁ 0;
}
