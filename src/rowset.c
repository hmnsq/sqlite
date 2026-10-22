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
#समावेश "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#घोषणा ROWSET_ALLOCATION_आकार 1024

/*
** The number of rowset entries per allocation chunk.
*/
#घोषणा ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_आकार-8)/माप(काष्ठा RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
काष्ठा RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  काष्ठा RowSetEntry *p_दायाँ;   /* Right subtree (larger entries) or list */
  काष्ठा RowSetEntry *p_बायाँ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
काष्ठा RowSetChunk {
  काष्ठा RowSetChunk *p_अग्र_Chunk;        /* Next chunk on list of them all */
  काष्ठा RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
काष्ठा RowSet {
  काष्ठा RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  काष्ठा RowSetEntry *pEntry;    /* List of entries using pRight */
  काष्ठा RowSetEntry *pLast;     /* Last entry on the pEntry list */
  काष्ठा RowSetEntry *pFresh;    /* Source of new entry objects */
  काष्ठा RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  पूर्णांक iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#घोषणा ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#घोषणा ROWSET_अग्र    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
RowSet *sqlite3RowSetInit(sqlite3 *db){
  RowSet *p = sqlite3DbMallocRawNN(db, माप(*p));
  अगर( p ){
    पूर्णांक N = sqlite3_Db_Malloc_आकार(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (काष्ठा RowSetEntry*)(ROUND8(माप(*p)) + (अक्षर*)p);
    p->nFresh = (u16)((N - ROUND8(माप(*p)))/माप(काष्ठा RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  वापस p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
व्योम sqlite3RowSetClear(व्योम *pArg){
  RowSet *p = (RowSet*)pArg;
  काष्ठा RowSetChunk *pChunk, *p_अग्र_Chunk;
  क्रम(pChunk=p->pChunk; pChunk; pChunk = p_अग्र_Chunk){
    p_अग्र_Chunk = pChunk->p_अग्र_Chunk;
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
व्योम sqlite3RowSetDelete(व्योम *pArg){
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
अटल काष्ठा RowSetEntry *rowSetEntryAlloc(RowSet *p){
  निश्चित( p!=0 );
  अगर( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    काष्ठा RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, माप(*pNew));
    अगर( pNew==0 ){
      वापस 0;
    }
    pNew->p_अग्र_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  वापस p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
व्योम sqlite3_Row_Set_जोड़ो(RowSet *p, i64 rowid){
  काष्ठा RowSetEntry *pEntry;  /* The new entry */
  काष्ठा RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  निश्चित( p!=0 && (p->rsFlags & ROWSET_अग्र)==0 );

  pEntry = rowSetEntryAlloc(p);
  अगर( pEntry==0 ) वापस;
  pEntry->v = rowid;
  pEntry->p_दायाँ = 0;
  pLast = p->pLast;
  अगर( pLast ){
    अगर( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
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
अटल काष्ठा RowSetEntry *rowSetEntryMerge(
  काष्ठा RowSetEntry *pA,    /* First sorted list to be merged */
  काष्ठा RowSetEntry *pB     /* Second sorted list to be merged */
){
  काष्ठा RowSetEntry शीर्ष_2;
  काष्ठा RowSetEntry *p_पुच्छ;

  p_पुच्छ = &शीर्ष_2;
  निश्चित( pA!=0 && pB!=0 );
  क्रम(;;){
    निश्चित( pA->p_दायाँ==0 || pA->v<=pA->p_दायाँ->v );
    निश्चित( pB->p_दायाँ==0 || pB->v<=pB->p_दायाँ->v );
    अगर( pA->v<=pB->v ){
      अगर( pA->v<pB->v ) p_पुच्छ = p_पुच्छ->p_दायाँ = pA;
      pA = pA->p_दायाँ;
      अगर( pA==0 ){
        p_पुच्छ->p_दायाँ = pB;
        अवरोध;
      }
    }अन्यथा{
      p_पुच्छ = p_पुच्छ->p_दायाँ = pB;
      pB = pB->p_दायाँ;
      अगर( pB==0 ){
        p_पुच्छ->p_दायाँ = pA;
        अवरोध;
      }
    }
  }
  वापस शीर्ष_2.p_दायाँ;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
अटल काष्ठा RowSetEntry *rowSetEntrySort(काष्ठा RowSetEntry *pIn){
  अचिन्हित पूर्णांक i;
  काष्ठा RowSetEntry *p_अग्र, *aBucket[40];

  स_रखो(aBucket, 0, माप(aBucket));
  जबतक( pIn ){
    p_अग्र = pIn->p_दायाँ;
    pIn->p_दायाँ = 0;
    क्रम(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = p_अग्र;
  }
  pIn = aBucket[0];
  क्रम(i=1; i<माप(aBucket)/माप(aBucket[0]); i++){
    अगर( aBucket[i]==0 ) जारी;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  वापस pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
अटल व्योम row_Set_वृक्ष_To_List(
  काष्ठा RowSetEntry *pIn,         /* Root of the input tree */
  काष्ठा RowSetEntry **ppFirst,    /* Write head of the output list here */
  काष्ठा RowSetEntry **ppLast      /* Write tail of the output list here */
){
  निश्चित( pIn!=0 );
  अगर( pIn->p_बायाँ ){
    काष्ठा RowSetEntry *p;
    row_Set_वृक्ष_To_List(pIn->p_बायाँ, ppFirst, &p);
    p->p_दायाँ = pIn;
  }अन्यथा{
    *ppFirst = pIn;
  }
  अगर( pIn->p_दायाँ ){
    row_Set_वृक्ष_To_List(pIn->p_दायाँ, &pIn->p_दायाँ, ppLast);
  }अन्यथा{
    *ppLast = pIn;
  }
  निश्चित( (*ppLast)->p_दायाँ==0 );
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
अटल काष्ठा RowSetEntry *row_Set_NDeep_वृक्ष(
  काष्ठा RowSetEntry **ppList,
  पूर्णांक iDepth
){
  काष्ठा RowSetEntry *p;         /* Root of the new tree */
  काष्ठा RowSetEntry *p_बायाँ;     /* Left subtree */
  अगर( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    वापस 0;
  }
  अगर( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_बायाँ = row_Set_NDeep_वृक्ष(ppList, iDepth-1);
    p = *ppList;
    अगर( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      वापस p_बायाँ;
    }
    p->p_बायाँ = p_बायाँ;
    *ppList = p->p_दायाँ;
    p->p_दायाँ = row_Set_NDeep_वृक्ष(ppList, iDepth-1);
  }अन्यथा{
    p = *ppList;
    *ppList = p->p_दायाँ;
    p->p_बायाँ = p->p_दायाँ = 0;
  }
  वापस p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
अटल काष्ठा RowSetEntry *row_Set_List_To_वृक्ष(काष्ठा RowSetEntry *pList){
  पूर्णांक iDepth;           /* Depth of the tree so far */
  काष्ठा RowSetEntry *p;       /* Current tree root */
  काष्ठा RowSetEntry *p_बायाँ;   /* Left subtree */

  निश्चित( pList!=0 );
  p = pList;
  pList = p->p_दायाँ;
  p->p_बायाँ = p->p_दायाँ = 0;
  क्रम(iDepth=1; pList; iDepth++){
    p_बायाँ = p;
    p = pList;
    pList = p->p_दायाँ;
    p->p_बायाँ = p_बायाँ;
    p->p_दायाँ = row_Set_NDeep_वृक्ष(&pList, iDepth);
  }
  वापस p;
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
पूर्णांक sqlite3_Row_Set_अग्र(RowSet *p, i64 *pRowid){
  निश्चित( p!=0 );
  निश्चित( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  अगर( (p->rsFlags & ROWSET_अग्र)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    अगर( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_अग्र;
  }

  /* Return the next entry on the list */
  अगर( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_दायाँ;
    अगर( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    वापस 1;
  }अन्यथा{
    वापस 0;
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
पूर्णांक sqlite3RowSetTest(RowSet *pRowSet, पूर्णांक iBatch, sqlite3_int64 iRowid){
  काष्ठा RowSetEntry *p, *p_वृक्ष;

  /* This routine is never called after sqlite3RowSetNext() */
  निश्चित( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_अग्र)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  अगर( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    अगर( p ){
      काष्ठा RowSetEntry **pp_पूर्व_वृक्ष = &pRowSet->pForest;
      अगर( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      क्रम(p_वृक्ष = pRowSet->pForest; p_वृक्ष; p_वृक्ष=p_वृक्ष->p_दायाँ){
        pp_पूर्व_वृक्ष = &p_वृक्ष->p_दायाँ;
        अगर( p_वृक्ष->p_बायाँ==0 ){
          p_वृक्ष->p_बायाँ = row_Set_List_To_वृक्ष(p);
          अवरोध;
        }अन्यथा{
          काष्ठा RowSetEntry *pAux, *p_पुच्छ;
          row_Set_वृक्ष_To_List(p_वृक्ष->p_बायाँ, &pAux, &p_पुच्छ);
          p_वृक्ष->p_बायाँ = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      अगर( p_वृक्ष==0 ){
        *pp_पूर्व_वृक्ष = p_वृक्ष = rowSetEntryAlloc(pRowSet);
        अगर( p_वृक्ष ){
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
  क्रम(p_वृक्ष = pRowSet->pForest; p_वृक्ष; p_वृक्ष=p_वृक्ष->p_दायाँ){
    p = p_वृक्ष->p_बायाँ;
    जबतक( p ){
      अगर( p->v<iRowid ){
        p = p->p_दायाँ;
      }अन्यथा अगर( p->v>iRowid ){
        p = p->p_बायाँ;
      }अन्यथा{
        वापस 1;
      }
    }
  }
  वापस 0;
}
