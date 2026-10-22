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
#அந்தர்பாவ "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/அளவு(கட்டமைப்பு RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
கட்டமைப்பு RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  கட்டமைப்பு RowSetEntry *p_வலது;   /* Right subtree (larger entries) or list */
  கட்டமைப்பு RowSetEntry *p_இடது;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
கட்டமைப்பு RowSetChunk {
  கட்டமைப்பு RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  கட்டமைப்பு RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
கட்டமைப்பு RowSet {
  கட்டமைப்பு RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  கட்டமைப்பு RowSetEntry *pEntry;    /* List of entries using pRight */
  கட்டமைப்பு RowSetEntry *pLast;     /* Last entry on the pEntry list */
  கட்டமைப்பு RowSetEntry *pFresh;    /* Source of new entry objects */
  கட்டமைப்பு RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  முழு iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, அளவு(*p));
  எனில்( p ){
    முழு N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (கட்டமைப்பு RowSetEntry*)(ROUND8(அளவு(*p)) + (எழுத்து*)p);
    p->nFresh = (u16)((N - ROUND8(அளவு(*p)))/அளவு(கட்டமைப்பு RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  திரும்பு p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
வெறுமை sqlite3RowSetClear(வெறுமை *pArg){
  RowSet *p = (RowSet*)pArg;
  கட்டமைப்பு RowSetChunk *pChunk, *pNextChunk;
  க்கு(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
வெறுமை sqlite3RowSetDelete(வெறுமை *pArg){
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
நிலையான கட்டமைப்பு RowSetEntry *rowSetEntryAlloc(RowSet *p){
  ப்ரதிஜ்ஞா( p!=0 );
  எனில்( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    கட்டமைப்பு RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, அளவு(*pNew));
    எனில்( pNew==0 ){
      திரும்பு 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  திரும்பு p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
வெறுமை sqlite3_Row_Set_செருகு(RowSet *p, i64 rowid){
  கட்டமைப்பு RowSetEntry *pEntry;  /* The new entry */
  கட்டமைப்பு RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  ப்ரதிஜ்ஞா( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  எனில்( pEntry==0 ) திரும்பு;
  pEntry->v = rowid;
  pEntry->p_வலது = 0;
  pLast = p->pLast;
  எனில்( pLast ){
    எனில்( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_வலது = pEntry;
  }இல்லையெனில்{
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
நிலையான கட்டமைப்பு RowSetEntry *rowSetEntryMerge(
  கட்டமைப்பு RowSetEntry *pA,    /* First sorted list to be merged */
  கட்டமைப்பு RowSetEntry *pB     /* Second sorted list to be merged */
){
  கட்டமைப்பு RowSetEntry head;
  கட்டமைப்பு RowSetEntry *pTail;

  pTail = &head;
  ப்ரதிஜ்ஞா( pA!=0 && pB!=0 );
  க்கு(;;){
    ப்ரதிஜ்ஞா( pA->p_வலது==0 || pA->v<=pA->p_வலது->v );
    ப்ரதிஜ்ஞா( pB->p_வலது==0 || pB->v<=pB->p_வலது->v );
    எனில்( pA->v<=pB->v ){
      எனில்( pA->v<pB->v ) pTail = pTail->p_வலது = pA;
      pA = pA->p_வலது;
      எனில்( pA==0 ){
        pTail->p_வலது = pB;
        நிறுத்து;
      }
    }இல்லையெனில்{
      pTail = pTail->p_வலது = pB;
      pB = pB->p_வலது;
      எனில்( pB==0 ){
        pTail->p_வலது = pA;
        நிறுத்து;
      }
    }
  }
  திரும்பு head.p_வலது;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
நிலையான கட்டமைப்பு RowSetEntry *rowSetEntrySort(கட்டமைப்பு RowSetEntry *pIn){
  unsigned முழு i;
  கட்டமைப்பு RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, அளவு(aBucket));
  போது( pIn ){
    pNext = pIn->p_வலது;
    pIn->p_வலது = 0;
    க்கு(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  க்கு(i=1; i<அளவு(aBucket)/அளவு(aBucket[0]); i++){
    எனில்( aBucket[i]==0 ) தொடர்;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  திரும்பு pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
நிலையான வெறுமை row_Set_மரம்_To_List(
  கட்டமைப்பு RowSetEntry *pIn,         /* Root of the input tree */
  கட்டமைப்பு RowSetEntry **ppFirst,    /* Write head of the output list here */
  கட்டமைப்பு RowSetEntry **ppLast      /* Write tail of the output list here */
){
  ப்ரதிஜ்ஞா( pIn!=0 );
  எனில்( pIn->p_இடது ){
    கட்டமைப்பு RowSetEntry *p;
    row_Set_மரம்_To_List(pIn->p_இடது, ppFirst, &p);
    p->p_வலது = pIn;
  }இல்லையெனில்{
    *ppFirst = pIn;
  }
  எனில்( pIn->p_வலது ){
    row_Set_மரம்_To_List(pIn->p_வலது, &pIn->p_வலது, ppLast);
  }இல்லையெனில்{
    *ppLast = pIn;
  }
  ப்ரதிஜ்ஞா( (*ppLast)->p_வலது==0 );
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
நிலையான கட்டமைப்பு RowSetEntry *row_Set_NDeep_மரம்(
  கட்டமைப்பு RowSetEntry **ppList,
  முழு iDepth
){
  கட்டமைப்பு RowSetEntry *p;         /* Root of the new tree */
  கட்டமைப்பு RowSetEntry *p_இடது;     /* Left subtree */
  எனில்( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    திரும்பு 0;
  }
  எனில்( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_இடது = row_Set_NDeep_மரம்(ppList, iDepth-1);
    p = *ppList;
    எனில்( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      திரும்பு p_இடது;
    }
    p->p_இடது = p_இடது;
    *ppList = p->p_வலது;
    p->p_வலது = row_Set_NDeep_மரம்(ppList, iDepth-1);
  }இல்லையெனில்{
    p = *ppList;
    *ppList = p->p_வலது;
    p->p_இடது = p->p_வலது = 0;
  }
  திரும்பு p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
நிலையான கட்டமைப்பு RowSetEntry *row_Set_List_To_மரம்(கட்டமைப்பு RowSetEntry *pList){
  முழு iDepth;           /* Depth of the tree so far */
  கட்டமைப்பு RowSetEntry *p;       /* Current tree root */
  கட்டமைப்பு RowSetEntry *p_இடது;   /* Left subtree */

  ப்ரதிஜ்ஞா( pList!=0 );
  p = pList;
  pList = p->p_வலது;
  p->p_இடது = p->p_வலது = 0;
  க்கு(iDepth=1; pList; iDepth++){
    p_இடது = p;
    p = pList;
    pList = p->p_வலது;
    p->p_இடது = p_இடது;
    p->p_வலது = row_Set_NDeep_மரம்(&pList, iDepth);
  }
  திரும்பு p;
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
முழு sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  ப்ரதிஜ்ஞா( p!=0 );
  ப்ரதிஜ்ஞா( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  எனில்( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    எனில்( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  எனில்( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_வலது;
    எனில்( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    திரும்பு 1;
  }இல்லையெனில்{
    திரும்பு 0;
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
முழு sqlite3RowSetTest(RowSet *pRowSet, முழு iBatch, sqlite3_int64 iRowid){
  கட்டமைப்பு RowSetEntry *p, *p_மரம்;

  /* This routine is never called after sqlite3RowSetNext() */
  ப்ரதிஜ்ஞா( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  எனில்( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    எனில்( p ){
      கட்டமைப்பு RowSetEntry **pp_Prev_மரம் = &pRowSet->pForest;
      எனில்( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      க்கு(p_மரம் = pRowSet->pForest; p_மரம்; p_மரம்=p_மரம்->p_வலது){
        pp_Prev_மரம் = &p_மரம்->p_வலது;
        எனில்( p_மரம்->p_இடது==0 ){
          p_மரம்->p_இடது = row_Set_List_To_மரம்(p);
          நிறுத்து;
        }இல்லையெனில்{
          கட்டமைப்பு RowSetEntry *pAux, *pTail;
          row_Set_மரம்_To_List(p_மரம்->p_இடது, &pAux, &pTail);
          p_மரம்->p_இடது = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      எனில்( p_மரம்==0 ){
        *pp_Prev_மரம் = p_மரம் = rowSetEntryAlloc(pRowSet);
        எனில்( p_மரம் ){
          p_மரம்->v = 0;
          p_மரம்->p_வலது = 0;
          p_மரம்->p_இடது = row_Set_List_To_மரம்(p);
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
  க்கு(p_மரம் = pRowSet->pForest; p_மரம்; p_மரம்=p_மரம்->p_வலது){
    p = p_மரம்->p_இடது;
    போது( p ){
      எனில்( p->v<iRowid ){
        p = p->p_வலது;
      }இல்லையெனில் எனில்( p->v>iRowid ){
        p = p->p_இடது;
      }இல்லையெனில்{
        திரும்பு 1;
      }
    }
  }
  திரும்பு 0;
}
