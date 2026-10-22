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
#अन्तर्भाव "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/परिमाण(संरचना RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
संरचना RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  संरचना RowSetEntry *p_दायाँ;   /* Right subtree (larger entries) or list */
  संरचना RowSetEntry *p_बायाँ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
संरचना RowSetChunk {
  संरचना RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  संरचना RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
संरचना RowSet {
  संरचना RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  संरचना RowSetEntry *pEntry;    /* List of entries using pRight */
  संरचना RowSetEntry *pLast;     /* Last entry on the pEntry list */
  संरचना RowSetEntry *pFresh;    /* Source of new entry objects */
  संरचना RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  पूर्णाङ्क iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, परिमाण(*p));
  यदि( p ){
    पूर्णाङ्क N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (संरचना RowSetEntry*)(ROUND8(परिमाण(*p)) + (वर्ण*)p);
    p->nFresh = (u16)((N - ROUND8(परिमाण(*p)))/परिमाण(संरचना RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  प्रत्यागम p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
शून्य sqlite3RowSetClear(शून्य *pArg){
  RowSet *p = (RowSet*)pArg;
  संरचना RowSetChunk *pChunk, *pNextChunk;
  प्रति(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
शून्य sqlite3RowSetDelete(शून्य *pArg){
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
स्थिर संरचना RowSetEntry *rowSetEntryAlloc(RowSet *p){
  प्रतिज्ञा( p!=0 );
  यदि( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    संरचना RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, परिमाण(*pNew));
    यदि( pNew==0 ){
      प्रत्यागम 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  प्रत्यागम p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
शून्य sqlite3_Row_Set_जोड़ो(RowSet *p, i64 rowid){
  संरचना RowSetEntry *pEntry;  /* The new entry */
  संरचना RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  प्रतिज्ञा( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  यदि( pEntry==0 ) प्रत्यागम;
  pEntry->v = rowid;
  pEntry->p_दायाँ = 0;
  pLast = p->pLast;
  यदि( pLast ){
    यदि( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_दायाँ = pEntry;
  }अन्यथा{
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
स्थिर संरचना RowSetEntry *rowSetEntryMerge(
  संरचना RowSetEntry *pA,    /* First sorted list to be merged */
  संरचना RowSetEntry *pB     /* Second sorted list to be merged */
){
  संरचना RowSetEntry head;
  संरचना RowSetEntry *pTail;

  pTail = &head;
  प्रतिज्ञा( pA!=0 && pB!=0 );
  प्रति(;;){
    प्रतिज्ञा( pA->p_दायाँ==0 || pA->v<=pA->p_दायाँ->v );
    प्रतिज्ञा( pB->p_दायाँ==0 || pB->v<=pB->p_दायाँ->v );
    यदि( pA->v<=pB->v ){
      यदि( pA->v<pB->v ) pTail = pTail->p_दायाँ = pA;
      pA = pA->p_दायाँ;
      यदि( pA==0 ){
        pTail->p_दायाँ = pB;
        भङ्ग;
      }
    }अन्यथा{
      pTail = pTail->p_दायाँ = pB;
      pB = pB->p_दायाँ;
      यदि( pB==0 ){
        pTail->p_दायाँ = pA;
        भङ्ग;
      }
    }
  }
  प्रत्यागम head.p_दायाँ;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
स्थिर संरचना RowSetEntry *rowSetEntrySort(संरचना RowSetEntry *pIn){
  unsigned पूर्णाङ्क i;
  संरचना RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, परिमाण(aBucket));
  यावत्( pIn ){
    pNext = pIn->p_दायाँ;
    pIn->p_दायाँ = 0;
    प्रति(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  प्रति(i=1; i<परिमाण(aBucket)/परिमाण(aBucket[0]); i++){
    यदि( aBucket[i]==0 ) अनुवर्त;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  प्रत्यागम pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
स्थिर शून्य row_Set_वृक्ष_To_List(
  संरचना RowSetEntry *pIn,         /* Root of the input tree */
  संरचना RowSetEntry **ppFirst,    /* Write head of the output list here */
  संरचना RowSetEntry **ppLast      /* Write tail of the output list here */
){
  प्रतिज्ञा( pIn!=0 );
  यदि( pIn->p_बायाँ ){
    संरचना RowSetEntry *p;
    row_Set_वृक्ष_To_List(pIn->p_बायाँ, ppFirst, &p);
    p->p_दायाँ = pIn;
  }अन्यथा{
    *ppFirst = pIn;
  }
  यदि( pIn->p_दायाँ ){
    row_Set_वृक्ष_To_List(pIn->p_दायाँ, &pIn->p_दायाँ, ppLast);
  }अन्यथा{
    *ppLast = pIn;
  }
  प्रतिज्ञा( (*ppLast)->p_दायाँ==0 );
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
स्थिर संरचना RowSetEntry *row_Set_NDeep_वृक्ष(
  संरचना RowSetEntry **ppList,
  पूर्णाङ्क iDepth
){
  संरचना RowSetEntry *p;         /* Root of the new tree */
  संरचना RowSetEntry *p_बायाँ;     /* Left subtree */
  यदि( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    प्रत्यागम 0;
  }
  यदि( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_बायाँ = row_Set_NDeep_वृक्ष(ppList, iDepth-1);
    p = *ppList;
    यदि( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      प्रत्यागम p_बायाँ;
    }
    p->p_बायाँ = p_बायाँ;
    *ppList = p->p_दायाँ;
    p->p_दायाँ = row_Set_NDeep_वृक्ष(ppList, iDepth-1);
  }अन्यथा{
    p = *ppList;
    *ppList = p->p_दायाँ;
    p->p_बायाँ = p->p_दायाँ = 0;
  }
  प्रत्यागम p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
स्थिर संरचना RowSetEntry *row_Set_List_To_वृक्ष(संरचना RowSetEntry *pList){
  पूर्णाङ्क iDepth;           /* Depth of the tree so far */
  संरचना RowSetEntry *p;       /* Current tree root */
  संरचना RowSetEntry *p_बायाँ;   /* Left subtree */

  प्रतिज्ञा( pList!=0 );
  p = pList;
  pList = p->p_दायाँ;
  p->p_बायाँ = p->p_दायाँ = 0;
  प्रति(iDepth=1; pList; iDepth++){
    p_बायाँ = p;
    p = pList;
    pList = p->p_दायाँ;
    p->p_बायाँ = p_बायाँ;
    p->p_दायाँ = row_Set_NDeep_वृक्ष(&pList, iDepth);
  }
  प्रत्यागम p;
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
पूर्णाङ्क sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  प्रतिज्ञा( p!=0 );
  प्रतिज्ञा( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  यदि( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    यदि( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  यदि( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_दायाँ;
    यदि( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    प्रत्यागम 1;
  }अन्यथा{
    प्रत्यागम 0;
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
पूर्णाङ्क sqlite3RowSetTest(RowSet *pRowSet, पूर्णाङ्क iBatch, sqlite3_int64 iRowid){
  संरचना RowSetEntry *p, *p_वृक्ष;

  /* This routine is never called after sqlite3RowSetNext() */
  प्रतिज्ञा( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  यदि( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    यदि( p ){
      संरचना RowSetEntry **pp_Prev_वृक्ष = &pRowSet->pForest;
      यदि( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      प्रति(p_वृक्ष = pRowSet->pForest; p_वृक्ष; p_वृक्ष=p_वृक्ष->p_दायाँ){
        pp_Prev_वृक्ष = &p_वृक्ष->p_दायाँ;
        यदि( p_वृक्ष->p_बायाँ==0 ){
          p_वृक्ष->p_बायाँ = row_Set_List_To_वृक्ष(p);
          भङ्ग;
        }अन्यथा{
          संरचना RowSetEntry *pAux, *pTail;
          row_Set_वृक्ष_To_List(p_वृक्ष->p_बायाँ, &pAux, &pTail);
          p_वृक्ष->p_बायाँ = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      यदि( p_वृक्ष==0 ){
        *pp_Prev_वृक्ष = p_वृक्ष = rowSetEntryAlloc(pRowSet);
        यदि( p_वृक्ष ){
          p_वृक्ष->v = 0;
          p_वृक्ष->p_दायाँ = 0;
          p_वृक्ष->p_बायाँ = row_Set_List_To_वृक्ष(p);
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
  प्रति(p_वृक्ष = pRowSet->pForest; p_वृक्ष; p_वृक्ष=p_वृक्ष->p_दायाँ){
    p = p_वृक्ष->p_बायाँ;
    यावत्( p ){
      यदि( p->v<iRowid ){
        p = p->p_दायाँ;
      }अन्यथा यदि( p->v>iRowid ){
        p = p->p_बायाँ;
      }अन्यथा{
        प्रत्यागम 1;
      }
    }
  }
  प्रत्यागम 0;
}
