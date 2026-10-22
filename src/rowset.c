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
#അന്തര്ഭാവ "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/വലുപ്പം(ഘടന RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
ഘടന RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  ഘടന RowSetEntry *p_വലത്;   /* Right subtree (larger entries) or list */
  ഘടന RowSetEntry *p_ഇടത്;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
ഘടന RowSetChunk {
  ഘടന RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  ഘടന RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
ഘടന RowSet {
  ഘടന RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  ഘടന RowSetEntry *pEntry;    /* List of entries using pRight */
  ഘടന RowSetEntry *pLast;     /* Last entry on the pEntry list */
  ഘടന RowSetEntry *pFresh;    /* Source of new entry objects */
  ഘടന RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  പൂർണ്ണസംഖ്യ iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, വലുപ്പം(*p));
  എങ്കിൽ( p ){
    പൂർണ്ണസംഖ്യ N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (ഘടന RowSetEntry*)(ROUND8(വലുപ്പം(*p)) + (അക്ഷരം*)p);
    p->nFresh = (u16)((N - ROUND8(വലുപ്പം(*p)))/വലുപ്പം(ഘടന RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  തിരികെ p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
ശൂന്യം sqlite3RowSetClear(ശൂന്യം *pArg){
  RowSet *p = (RowSet*)pArg;
  ഘടന RowSetChunk *pChunk, *pNextChunk;
  വേണ്ടി(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
ശൂന്യം sqlite3RowSetDelete(ശൂന്യം *pArg){
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
സ്ഥിതം ഘടന RowSetEntry *rowSetEntryAlloc(RowSet *p){
  പ്രതിജ്ഞാ( p!=0 );
  എങ്കിൽ( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    ഘടന RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, വലുപ്പം(*pNew));
    എങ്കിൽ( pNew==0 ){
      തിരികെ 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  തിരികെ p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
ശൂന്യം sqlite3_Row_Set_തിരുകുക(RowSet *p, i64 rowid){
  ഘടന RowSetEntry *pEntry;  /* The new entry */
  ഘടന RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  പ്രതിജ്ഞാ( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  എങ്കിൽ( pEntry==0 ) തിരികെ;
  pEntry->v = rowid;
  pEntry->p_വലത് = 0;
  pLast = p->pLast;
  എങ്കിൽ( pLast ){
    എങ്കിൽ( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_വലത് = pEntry;
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ഘടന RowSetEntry *rowSetEntryMerge(
  ഘടന RowSetEntry *pA,    /* First sorted list to be merged */
  ഘടന RowSetEntry *pB     /* Second sorted list to be merged */
){
  ഘടന RowSetEntry head;
  ഘടന RowSetEntry *pTail;

  pTail = &head;
  പ്രതിജ്ഞാ( pA!=0 && pB!=0 );
  വേണ്ടി(;;){
    പ്രതിജ്ഞാ( pA->p_വലത്==0 || pA->v<=pA->p_വലത്->v );
    പ്രതിജ്ഞാ( pB->p_വലത്==0 || pB->v<=pB->p_വലത്->v );
    എങ്കിൽ( pA->v<=pB->v ){
      എങ്കിൽ( pA->v<pB->v ) pTail = pTail->p_വലത് = pA;
      pA = pA->p_വലത്;
      എങ്കിൽ( pA==0 ){
        pTail->p_വലത് = pB;
        നിർത്തുക;
      }
    }അല്ലെങ്കിൽ{
      pTail = pTail->p_വലത് = pB;
      pB = pB->p_വലത്;
      എങ്കിൽ( pB==0 ){
        pTail->p_വലത് = pA;
        നിർത്തുക;
      }
    }
  }
  തിരികെ head.p_വലത്;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
സ്ഥിതം ഘടന RowSetEntry *rowSetEntrySort(ഘടന RowSetEntry *pIn){
  unsigned പൂർണ്ണസംഖ്യ i;
  ഘടന RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, വലുപ്പം(aBucket));
  ആകുമ്പോൾ( pIn ){
    pNext = pIn->p_വലത്;
    pIn->p_വലത് = 0;
    വേണ്ടി(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  വേണ്ടി(i=1; i<വലുപ്പം(aBucket)/വലുപ്പം(aBucket[0]); i++){
    എങ്കിൽ( aBucket[i]==0 ) തുടരുക;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  തിരികെ pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
സ്ഥിതം ശൂന്യം row_Set_വൃക്ഷം_To_List(
  ഘടന RowSetEntry *pIn,         /* Root of the input tree */
  ഘടന RowSetEntry **ppFirst,    /* Write head of the output list here */
  ഘടന RowSetEntry **ppLast      /* Write tail of the output list here */
){
  പ്രതിജ്ഞാ( pIn!=0 );
  എങ്കിൽ( pIn->p_ഇടത് ){
    ഘടന RowSetEntry *p;
    row_Set_വൃക്ഷം_To_List(pIn->p_ഇടത്, ppFirst, &p);
    p->p_വലത് = pIn;
  }അല്ലെങ്കിൽ{
    *ppFirst = pIn;
  }
  എങ്കിൽ( pIn->p_വലത് ){
    row_Set_വൃക്ഷം_To_List(pIn->p_വലത്, &pIn->p_വലത്, ppLast);
  }അല്ലെങ്കിൽ{
    *ppLast = pIn;
  }
  പ്രതിജ്ഞാ( (*ppLast)->p_വലത്==0 );
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
സ്ഥിതം ഘടന RowSetEntry *row_Set_NDeep_വൃക്ഷം(
  ഘടന RowSetEntry **ppList,
  പൂർണ്ണസംഖ്യ iDepth
){
  ഘടന RowSetEntry *p;         /* Root of the new tree */
  ഘടന RowSetEntry *p_ഇടത്;     /* Left subtree */
  എങ്കിൽ( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    തിരികെ 0;
  }
  എങ്കിൽ( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_ഇടത് = row_Set_NDeep_വൃക്ഷം(ppList, iDepth-1);
    p = *ppList;
    എങ്കിൽ( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      തിരികെ p_ഇടത്;
    }
    p->p_ഇടത് = p_ഇടത്;
    *ppList = p->p_വലത്;
    p->p_വലത് = row_Set_NDeep_വൃക്ഷം(ppList, iDepth-1);
  }അല്ലെങ്കിൽ{
    p = *ppList;
    *ppList = p->p_വലത്;
    p->p_ഇടത് = p->p_വലത് = 0;
  }
  തിരികെ p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
സ്ഥിതം ഘടന RowSetEntry *row_Set_List_To_വൃക്ഷം(ഘടന RowSetEntry *pList){
  പൂർണ്ണസംഖ്യ iDepth;           /* Depth of the tree so far */
  ഘടന RowSetEntry *p;       /* Current tree root */
  ഘടന RowSetEntry *p_ഇടത്;   /* Left subtree */

  പ്രതിജ്ഞാ( pList!=0 );
  p = pList;
  pList = p->p_വലത്;
  p->p_ഇടത് = p->p_വലത് = 0;
  വേണ്ടി(iDepth=1; pList; iDepth++){
    p_ഇടത് = p;
    p = pList;
    pList = p->p_വലത്;
    p->p_ഇടത് = p_ഇടത്;
    p->p_വലത് = row_Set_NDeep_വൃക്ഷം(&pList, iDepth);
  }
  തിരികെ p;
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
പൂർണ്ണസംഖ്യ sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  പ്രതിജ്ഞാ( p!=0 );
  പ്രതിജ്ഞാ( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  എങ്കിൽ( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    എങ്കിൽ( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  എങ്കിൽ( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_വലത്;
    എങ്കിൽ( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
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
പൂർണ്ണസംഖ്യ sqlite3RowSetTest(RowSet *pRowSet, പൂർണ്ണസംഖ്യ iBatch, sqlite3_int64 iRowid){
  ഘടന RowSetEntry *p, *p_വൃക്ഷം;

  /* This routine is never called after sqlite3RowSetNext() */
  പ്രതിജ്ഞാ( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  എങ്കിൽ( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    എങ്കിൽ( p ){
      ഘടന RowSetEntry **pp_Prev_വൃക്ഷം = &pRowSet->pForest;
      എങ്കിൽ( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      വേണ്ടി(p_വൃക്ഷം = pRowSet->pForest; p_വൃക്ഷം; p_വൃക്ഷം=p_വൃക്ഷം->p_വലത്){
        pp_Prev_വൃക്ഷം = &p_വൃക്ഷം->p_വലത്;
        എങ്കിൽ( p_വൃക്ഷം->p_ഇടത്==0 ){
          p_വൃക്ഷം->p_ഇടത് = row_Set_List_To_വൃക്ഷം(p);
          നിർത്തുക;
        }അല്ലെങ്കിൽ{
          ഘടന RowSetEntry *pAux, *pTail;
          row_Set_വൃക്ഷം_To_List(p_വൃക്ഷം->p_ഇടത്, &pAux, &pTail);
          p_വൃക്ഷം->p_ഇടത് = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      എങ്കിൽ( p_വൃക്ഷം==0 ){
        *pp_Prev_വൃക്ഷം = p_വൃക്ഷം = rowSetEntryAlloc(pRowSet);
        എങ്കിൽ( p_വൃക്ഷം ){
          p_വൃക്ഷം->v = 0;
          p_വൃക്ഷം->p_വലത് = 0;
          p_വൃക്ഷം->p_ഇടത് = row_Set_List_To_വൃക്ഷം(p);
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
  വേണ്ടി(p_വൃക്ഷം = pRowSet->pForest; p_വൃക്ഷം; p_വൃക്ഷം=p_വൃക്ഷം->p_വലത്){
    p = p_വൃക്ഷം->p_ഇടത്;
    ആകുമ്പോൾ( p ){
      എങ്കിൽ( p->v<iRowid ){
        p = p->p_വലത്;
      }അല്ലെങ്കിൽ എങ്കിൽ( p->v>iRowid ){
        p = p->p_ഇടത്;
      }അല്ലെങ്കിൽ{
        തിരികെ 1;
      }
    }
  }
  തിരികെ 0;
}
