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
#ضمن "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/حجم(بنية RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
بنية RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  بنية RowSetEntry *p_دام;   /* Right subtree (larger entries) or list */
  بنية RowSetEntry *p_هغو;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
بنية RowSetChunk {
  بنية RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  بنية RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
بنية RowSet {
  بنية RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  بنية RowSetEntry *pEntry;    /* List of entries using pRight */
  بنية RowSetEntry *pLast;     /* Last entry on the pEntry list */
  بنية RowSetEntry *pFresh;    /* Source of new entry objects */
  بنية RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  صحيح iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, حجم(*p));
  إذا( p ){
    صحيح N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (بنية RowSetEntry*)(ROUND8(حجم(*p)) + (حرف*)p);
    p->nFresh = (u16)((N - ROUND8(حجم(*p)))/حجم(بنية RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  أرجع p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
فراغ sqlite3RowSetClear(فراغ *pArg){
  RowSet *p = (RowSet*)pArg;
  بنية RowSetChunk *pChunk, *pNextChunk;
  لكل(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
فراغ sqlite3RowSetDelete(فراغ *pArg){
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
ساكن بنية RowSetEntry *rowSetEntryAlloc(RowSet *p){
  أكد( p!=0 );
  إذا( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    بنية RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, حجم(*pNew));
    إذا( pNew==0 ){
      أرجع 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  أرجع p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
فراغ sqlite3_Row_Set_سك(RowSet *p, i64 rowid){
  بنية RowSetEntry *pEntry;  /* The new entry */
  بنية RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  إذا( pEntry==0 ) أرجع;
  pEntry->v = rowid;
  pEntry->p_دام = 0;
  pLast = p->pLast;
  إذا( pLast ){
    إذا( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_دام = pEntry;
  }وإلا{
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
ساكن بنية RowSetEntry *rowSetEntryMerge(
  بنية RowSetEntry *pA,    /* First sorted list to be merged */
  بنية RowSetEntry *pB     /* Second sorted list to be merged */
){
  بنية RowSetEntry head;
  بنية RowSetEntry *pTail;

  pTail = &head;
  أكد( pA!=0 && pB!=0 );
  لكل(;;){
    أكد( pA->p_دام==0 || pA->v<=pA->p_دام->v );
    أكد( pB->p_دام==0 || pB->v<=pB->p_دام->v );
    إذا( pA->v<=pB->v ){
      إذا( pA->v<pB->v ) pTail = pTail->p_دام = pA;
      pA = pA->p_دام;
      إذا( pA==0 ){
        pTail->p_دام = pB;
        اكسر;
      }
    }وإلا{
      pTail = pTail->p_دام = pB;
      pB = pB->p_دام;
      إذا( pB==0 ){
        pTail->p_دام = pA;
        اكسر;
      }
    }
  }
  أرجع head.p_دام;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ساكن بنية RowSetEntry *rowSetEntrySort(بنية RowSetEntry *pIn){
  unsigned صحيح i;
  بنية RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, حجم(aBucket));
  طالما( pIn ){
    pNext = pIn->p_دام;
    pIn->p_دام = 0;
    لكل(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  لكل(i=1; i<حجم(aBucket)/حجم(aBucket[0]); i++){
    إذا( aBucket[i]==0 ) استمر;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  أرجع pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ساكن فراغ row_Set_بشيا_To_List(
  بنية RowSetEntry *pIn,         /* Root of the input tree */
  بنية RowSetEntry **ppFirst,    /* Write head of the output list here */
  بنية RowSetEntry **ppLast      /* Write tail of the output list here */
){
  أكد( pIn!=0 );
  إذا( pIn->p_هغو ){
    بنية RowSetEntry *p;
    row_Set_بشيا_To_List(pIn->p_هغو, ppFirst, &p);
    p->p_دام = pIn;
  }وإلا{
    *ppFirst = pIn;
  }
  إذا( pIn->p_دام ){
    row_Set_بشيا_To_List(pIn->p_دام, &pIn->p_دام, ppLast);
  }وإلا{
    *ppLast = pIn;
  }
  أكد( (*ppLast)->p_دام==0 );
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
ساكن بنية RowSetEntry *row_Set_NDeep_بشيا(
  بنية RowSetEntry **ppList,
  صحيح iDepth
){
  بنية RowSetEntry *p;         /* Root of the new tree */
  بنية RowSetEntry *p_هغو;     /* Left subtree */
  إذا( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    أرجع 0;
  }
  إذا( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_هغو = row_Set_NDeep_بشيا(ppList, iDepth-1);
    p = *ppList;
    إذا( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      أرجع p_هغو;
    }
    p->p_هغو = p_هغو;
    *ppList = p->p_دام;
    p->p_دام = row_Set_NDeep_بشيا(ppList, iDepth-1);
  }وإلا{
    p = *ppList;
    *ppList = p->p_دام;
    p->p_هغو = p->p_دام = 0;
  }
  أرجع p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ساكن بنية RowSetEntry *row_Set_List_To_بشيا(بنية RowSetEntry *pList){
  صحيح iDepth;           /* Depth of the tree so far */
  بنية RowSetEntry *p;       /* Current tree root */
  بنية RowSetEntry *p_هغو;   /* Left subtree */

  أكد( pList!=0 );
  p = pList;
  pList = p->p_دام;
  p->p_هغو = p->p_دام = 0;
  لكل(iDepth=1; pList; iDepth++){
    p_هغو = p;
    p = pList;
    pList = p->p_دام;
    p->p_هغو = p_هغو;
    p->p_دام = row_Set_NDeep_بشيا(&pList, iDepth);
  }
  أرجع p;
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
صحيح sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  أكد( p!=0 );
  أكد( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  إذا( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    إذا( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  إذا( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_دام;
    إذا( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    أرجع 1;
  }وإلا{
    أرجع 0;
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
صحيح sqlite3RowSetTest(RowSet *pRowSet, صحيح iBatch, sqlite3_int64 iRowid){
  بنية RowSetEntry *p, *p_بشيا;

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  إذا( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    إذا( p ){
      بنية RowSetEntry **pp_Prev_بشيا = &pRowSet->pForest;
      إذا( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      لكل(p_بشيا = pRowSet->pForest; p_بشيا; p_بشيا=p_بشيا->p_دام){
        pp_Prev_بشيا = &p_بشيا->p_دام;
        إذا( p_بشيا->p_هغو==0 ){
          p_بشيا->p_هغو = row_Set_List_To_بشيا(p);
          اكسر;
        }وإلا{
          بنية RowSetEntry *pAux, *pTail;
          row_Set_بشيا_To_List(p_بشيا->p_هغو, &pAux, &pTail);
          p_بشيا->p_هغو = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      إذا( p_بشيا==0 ){
        *pp_Prev_بشيا = p_بشيا = rowSetEntryAlloc(pRowSet);
        إذا( p_بشيا ){
          p_بشيا->v = 0;
          p_بشيا->p_دام = 0;
          p_بشيا->p_هغو = row_Set_List_To_بشيا(p);
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
  لكل(p_بشيا = pRowSet->pForest; p_بشيا; p_بشيا=p_بشيا->p_دام){
    p = p_بشيا->p_هغو;
    طالما( p ){
      إذا( p->v<iRowid ){
        p = p->p_دام;
      }وإلا إذا( p->v>iRowid ){
        p = p->p_هغو;
      }وإلا{
        أرجع 1;
      }
    }
  }
  أرجع 0;
}
