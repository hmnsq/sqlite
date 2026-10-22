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
#অন্তর্ভাব "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_আকার 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_আকার-8)/আকার(কাঠামো RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
কাঠামো RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  কাঠামো RowSetEntry *p_ডান;   /* Right subtree (larger entries) or list */
  কাঠামো RowSetEntry *p_বাম;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
কাঠামো RowSetChunk {
  কাঠামো RowSetChunk *p_পরবর্তী_Chunk;        /* Next chunk on list of them all */
  কাঠামো RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
কাঠামো RowSet {
  কাঠামো RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  কাঠামো RowSetEntry *pEntry;    /* List of entries using pRight */
  কাঠামো RowSetEntry *pLast;     /* Last entry on the pEntry list */
  কাঠামো RowSetEntry *pFresh;    /* Source of new entry objects */
  কাঠামো RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  পূর্ণসংখ্যা iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#define ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#define ROWSET_পরবর্তী    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
RowSet *sqlite3RowSetInit(sqlite3 *db){
  RowSet *p = sqlite3DbMallocRawNN(db, আকার(*p));
  যদি( p ){
    পূর্ণসংখ্যা N = sqlite3_Db_Malloc_আকার(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (কাঠামো RowSetEntry*)(ROUND8(আকার(*p)) + (অক্ষর*)p);
    p->nFresh = (u16)((N - ROUND8(আকার(*p)))/আকার(কাঠামো RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  ফেরত p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
শূন্য sqlite3RowSetClear(শূন্য *pArg){
  RowSet *p = (RowSet*)pArg;
  কাঠামো RowSetChunk *pChunk, *p_পরবর্তী_Chunk;
  জন্য(pChunk=p->pChunk; pChunk; pChunk = p_পরবর্তী_Chunk){
    p_পরবর্তী_Chunk = pChunk->p_পরবর্তী_Chunk;
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
শূন্য sqlite3RowSetDelete(শূন্য *pArg){
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
স্থির কাঠামো RowSetEntry *rowSetEntryAlloc(RowSet *p){
  প্রতিজ্ঞা( p!=0 );
  যদি( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    কাঠামো RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, আকার(*pNew));
    যদি( pNew==0 ){
      ফেরত 0;
    }
    pNew->p_পরবর্তী_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  ফেরত p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
শূন্য sqlite3_Row_Set_ঢোকাও(RowSet *p, i64 rowid){
  কাঠামো RowSetEntry *pEntry;  /* The new entry */
  কাঠামো RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  প্রতিজ্ঞা( p!=0 && (p->rsFlags & ROWSET_পরবর্তী)==0 );

  pEntry = rowSetEntryAlloc(p);
  যদি( pEntry==0 ) ফেরত;
  pEntry->v = rowid;
  pEntry->p_ডান = 0;
  pLast = p->pLast;
  যদি( pLast ){
    যদি( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_ডান = pEntry;
  }অন্যথা{
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
স্থির কাঠামো RowSetEntry *rowSetEntryMerge(
  কাঠামো RowSetEntry *pA,    /* First sorted list to be merged */
  কাঠামো RowSetEntry *pB     /* Second sorted list to be merged */
){
  কাঠামো RowSetEntry শীর্ষ_2;
  কাঠামো RowSetEntry *p_লেজ;

  p_লেজ = &শীর্ষ_2;
  প্রতিজ্ঞা( pA!=0 && pB!=0 );
  জন্য(;;){
    প্রতিজ্ঞা( pA->p_ডান==0 || pA->v<=pA->p_ডান->v );
    প্রতিজ্ঞা( pB->p_ডান==0 || pB->v<=pB->p_ডান->v );
    যদি( pA->v<=pB->v ){
      যদি( pA->v<pB->v ) p_লেজ = p_লেজ->p_ডান = pA;
      pA = pA->p_ডান;
      যদি( pA==0 ){
        p_লেজ->p_ডান = pB;
        বিরতি;
      }
    }অন্যথা{
      p_লেজ = p_লেজ->p_ডান = pB;
      pB = pB->p_ডান;
      যদি( pB==0 ){
        p_লেজ->p_ডান = pA;
        বিরতি;
      }
    }
  }
  ফেরত শীর্ষ_2.p_ডান;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
স্থির কাঠামো RowSetEntry *rowSetEntrySort(কাঠামো RowSetEntry *pIn){
  unsigned পূর্ণসংখ্যা i;
  কাঠামো RowSetEntry *p_পরবর্তী, *aBucket[40];

  memset(aBucket, 0, আকার(aBucket));
  যখন( pIn ){
    p_পরবর্তী = pIn->p_ডান;
    pIn->p_ডান = 0;
    জন্য(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = p_পরবর্তী;
  }
  pIn = aBucket[0];
  জন্য(i=1; i<আকার(aBucket)/আকার(aBucket[0]); i++){
    যদি( aBucket[i]==0 ) চালু;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  ফেরত pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
স্থির শূন্য row_Set_বৃক্ষ_To_List(
  কাঠামো RowSetEntry *pIn,         /* Root of the input tree */
  কাঠামো RowSetEntry **ppFirst,    /* Write head of the output list here */
  কাঠামো RowSetEntry **ppLast      /* Write tail of the output list here */
){
  প্রতিজ্ঞা( pIn!=0 );
  যদি( pIn->p_বাম ){
    কাঠামো RowSetEntry *p;
    row_Set_বৃক্ষ_To_List(pIn->p_বাম, ppFirst, &p);
    p->p_ডান = pIn;
  }অন্যথা{
    *ppFirst = pIn;
  }
  যদি( pIn->p_ডান ){
    row_Set_বৃক্ষ_To_List(pIn->p_ডান, &pIn->p_ডান, ppLast);
  }অন্যথা{
    *ppLast = pIn;
  }
  প্রতিজ্ঞা( (*ppLast)->p_ডান==0 );
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
স্থির কাঠামো RowSetEntry *row_Set_NDeep_বৃক্ষ(
  কাঠামো RowSetEntry **ppList,
  পূর্ণসংখ্যা iDepth
){
  কাঠামো RowSetEntry *p;         /* Root of the new tree */
  কাঠামো RowSetEntry *p_বাম;     /* Left subtree */
  যদি( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    ফেরত 0;
  }
  যদি( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_বাম = row_Set_NDeep_বৃক্ষ(ppList, iDepth-1);
    p = *ppList;
    যদি( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      ফেরত p_বাম;
    }
    p->p_বাম = p_বাম;
    *ppList = p->p_ডান;
    p->p_ডান = row_Set_NDeep_বৃক্ষ(ppList, iDepth-1);
  }অন্যথা{
    p = *ppList;
    *ppList = p->p_ডান;
    p->p_বাম = p->p_ডান = 0;
  }
  ফেরত p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
স্থির কাঠামো RowSetEntry *row_Set_List_To_বৃক্ষ(কাঠামো RowSetEntry *pList){
  পূর্ণসংখ্যা iDepth;           /* Depth of the tree so far */
  কাঠামো RowSetEntry *p;       /* Current tree root */
  কাঠামো RowSetEntry *p_বাম;   /* Left subtree */

  প্রতিজ্ঞা( pList!=0 );
  p = pList;
  pList = p->p_ডান;
  p->p_বাম = p->p_ডান = 0;
  জন্য(iDepth=1; pList; iDepth++){
    p_বাম = p;
    p = pList;
    pList = p->p_ডান;
    p->p_বাম = p_বাম;
    p->p_ডান = row_Set_NDeep_বৃক্ষ(&pList, iDepth);
  }
  ফেরত p;
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
পূর্ণসংখ্যা sqlite3_Row_Set_পরবর্তী(RowSet *p, i64 *pRowid){
  প্রতিজ্ঞা( p!=0 );
  প্রতিজ্ঞা( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  যদি( (p->rsFlags & ROWSET_পরবর্তী)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    যদি( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_পরবর্তী;
  }

  /* Return the next entry on the list */
  যদি( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_ডান;
    যদি( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    ফেরত 1;
  }অন্যথা{
    ফেরত 0;
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
পূর্ণসংখ্যা sqlite3RowSetTest(RowSet *pRowSet, পূর্ণসংখ্যা iBatch, sqlite3_int64 iRowid){
  কাঠামো RowSetEntry *p, *p_বৃক্ষ;

  /* This routine is never called after sqlite3RowSetNext() */
  প্রতিজ্ঞা( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_পরবর্তী)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  যদি( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    যদি( p ){
      কাঠামো RowSetEntry **pp_পূর্ব_বৃক্ষ = &pRowSet->pForest;
      যদি( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      জন্য(p_বৃক্ষ = pRowSet->pForest; p_বৃক্ষ; p_বৃক্ষ=p_বৃক্ষ->p_ডান){
        pp_পূর্ব_বৃক্ষ = &p_বৃক্ষ->p_ডান;
        যদি( p_বৃক্ষ->p_বাম==0 ){
          p_বৃক্ষ->p_বাম = row_Set_List_To_বৃক্ষ(p);
          বিরতি;
        }অন্যথা{
          কাঠামো RowSetEntry *pAux, *p_লেজ;
          row_Set_বৃক্ষ_To_List(p_বৃক্ষ->p_বাম, &pAux, &p_লেজ);
          p_বৃক্ষ->p_বাম = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      যদি( p_বৃক্ষ==0 ){
        *pp_পূর্ব_বৃক্ষ = p_বৃক্ষ = rowSetEntryAlloc(pRowSet);
        যদি( p_বৃক্ষ ){
          p_বৃক্ষ->v = 0;
          p_বৃক্ষ->p_ডান = 0;
          p_বৃক্ষ->p_বাম = row_Set_List_To_বৃক্ষ(p);
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
  জন্য(p_বৃক্ষ = pRowSet->pForest; p_বৃক্ষ; p_বৃক্ষ=p_বৃক্ষ->p_ডান){
    p = p_বৃক্ষ->p_বাম;
    যখন( p ){
      যদি( p->v<iRowid ){
        p = p->p_ডান;
      }অন্যথা যদি( p->v>iRowid ){
        p = p->p_বাম;
      }অন্যথা{
        ফেরত 1;
      }
    }
  }
  ফেরত 0;
}
