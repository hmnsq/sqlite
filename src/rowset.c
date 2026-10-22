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
#અન્તર્ભાવ "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/કદ(સંરચના RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
સંરચના RowSetEntry {           
  i64 v;                        /* ROWID value for this entry */
  સંરચના RowSetEntry *p_જમણું;   /* Right subtree (larger entries) or list */
  સંરચના RowSetEntry *p_ડાબું;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
સંરચના RowSetChunk {
  સંરચના RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  સંરચના RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
સંરચના RowSet {
  સંરચના RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  સંરચના RowSetEntry *pEntry;    /* List of entries using pRight */
  સંરચના RowSetEntry *pLast;     /* Last entry on the pEntry list */
  સંરચના RowSetEntry *pFresh;    /* Source of new entry objects */
  સંરચના RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  પૂર્ણાંક iBatch;                    /* Current insert batch */
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
  RowSet *p = sqlite3DbMallocRawNN(db, કદ(*p));
  જો( p ){
    પૂર્ણાંક N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (સંરચના RowSetEntry*)(ROUND8(કદ(*p)) + (અક્ષર*)p);
    p->nFresh = (u16)((N - ROUND8(કદ(*p)))/કદ(સંરચના RowSetEntry));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  પાછા p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
શૂન્ય sqlite3RowSetClear(શૂન્ય *pArg){
  RowSet *p = (RowSet*)pArg;
  સંરચના RowSetChunk *pChunk, *pNextChunk;
  માટે(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
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
શૂન્ય sqlite3RowSetDelete(શૂન્ય *pArg){
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
સ્થિર સંરચના RowSetEntry *rowSetEntryAlloc(RowSet *p){
  પ્રતિજ્ઞા( p!=0 );
  જો( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    સંરચના RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, કદ(*pNew));
    જો( pNew==0 ){
      પાછા 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  પાછા p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
શૂન્ય sqlite3_Row_Set_ઉમેરો(RowSet *p, i64 rowid){
  સંરચના RowSetEntry *pEntry;  /* The new entry */
  સંરચના RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  પ્રતિજ્ઞા( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  જો( pEntry==0 ) પાછા;
  pEntry->v = rowid;
  pEntry->p_જમણું = 0;
  pLast = p->pLast;
  જો( pLast ){
    જો( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->p_જમણું = pEntry;
  }નહીંતર{
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
સ્થિર સંરચના RowSetEntry *rowSetEntryMerge(
  સંરચના RowSetEntry *pA,    /* First sorted list to be merged */
  સંરચના RowSetEntry *pB     /* Second sorted list to be merged */
){
  સંરચના RowSetEntry head;
  સંરચના RowSetEntry *pTail;

  pTail = &head;
  પ્રતિજ્ઞા( pA!=0 && pB!=0 );
  માટે(;;){
    પ્રતિજ્ઞા( pA->p_જમણું==0 || pA->v<=pA->p_જમણું->v );
    પ્રતિજ્ઞા( pB->p_જમણું==0 || pB->v<=pB->p_જમણું->v );
    જો( pA->v<=pB->v ){
      જો( pA->v<pB->v ) pTail = pTail->p_જમણું = pA;
      pA = pA->p_જમણું;
      જો( pA==0 ){
        pTail->p_જમણું = pB;
        તોડો;
      }
    }નહીંતર{
      pTail = pTail->p_જમણું = pB;
      pB = pB->p_જમણું;
      જો( pB==0 ){
        pTail->p_જમણું = pA;
        તોડો;
      }
    }
  }
  પાછા head.p_જમણું;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
સ્થિર સંરચના RowSetEntry *rowSetEntrySort(સંરચના RowSetEntry *pIn){
  unsigned પૂર્ણાંક i;
  સંરચના RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, કદ(aBucket));
  જ્યારે( pIn ){
    pNext = pIn->p_જમણું;
    pIn->p_જમણું = 0;
    માટે(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  માટે(i=1; i<કદ(aBucket)/કદ(aBucket[0]); i++){
    જો( aBucket[i]==0 ) ચાલુ;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  પાછા pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
સ્થિર શૂન્ય row_Set_વૃક્ષ_To_List(
  સંરચના RowSetEntry *pIn,         /* Root of the input tree */
  સંરચના RowSetEntry **ppFirst,    /* Write head of the output list here */
  સંરચના RowSetEntry **ppLast      /* Write tail of the output list here */
){
  પ્રતિજ્ઞા( pIn!=0 );
  જો( pIn->p_ડાબું ){
    સંરચના RowSetEntry *p;
    row_Set_વૃક્ષ_To_List(pIn->p_ડાબું, ppFirst, &p);
    p->p_જમણું = pIn;
  }નહીંતર{
    *ppFirst = pIn;
  }
  જો( pIn->p_જમણું ){
    row_Set_વૃક્ષ_To_List(pIn->p_જમણું, &pIn->p_જમણું, ppLast);
  }નહીંતર{
    *ppLast = pIn;
  }
  પ્રતિજ્ઞા( (*ppLast)->p_જમણું==0 );
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
સ્થિર સંરચના RowSetEntry *row_Set_NDeep_વૃક્ષ(
  સંરચના RowSetEntry **ppList,
  પૂર્ણાંક iDepth
){
  સંરચના RowSetEntry *p;         /* Root of the new tree */
  સંરચના RowSetEntry *p_ડાબું;     /* Left subtree */
  જો( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    પાછા 0;
  }
  જો( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_ડાબું = row_Set_NDeep_વૃક્ષ(ppList, iDepth-1);
    p = *ppList;
    જો( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      પાછા p_ડાબું;
    }
    p->p_ડાબું = p_ડાબું;
    *ppList = p->p_જમણું;
    p->p_જમણું = row_Set_NDeep_વૃક્ષ(ppList, iDepth-1);
  }નહીંતર{
    p = *ppList;
    *ppList = p->p_જમણું;
    p->p_ડાબું = p->p_જમણું = 0;
  }
  પાછા p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
સ્થિર સંરચના RowSetEntry *row_Set_List_To_વૃક્ષ(સંરચના RowSetEntry *pList){
  પૂર્ણાંક iDepth;           /* Depth of the tree so far */
  સંરચના RowSetEntry *p;       /* Current tree root */
  સંરચના RowSetEntry *p_ડાબું;   /* Left subtree */

  પ્રતિજ્ઞા( pList!=0 );
  p = pList;
  pList = p->p_જમણું;
  p->p_ડાબું = p->p_જમણું = 0;
  માટે(iDepth=1; pList; iDepth++){
    p_ડાબું = p;
    p = pList;
    pList = p->p_જમણું;
    p->p_ડાબું = p_ડાબું;
    p->p_જમણું = row_Set_NDeep_વૃક્ષ(&pList, iDepth);
  }
  પાછા p;
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
પૂર્ણાંક sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  પ્રતિજ્ઞા( p!=0 );
  પ્રતિજ્ઞા( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  જો( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    જો( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  જો( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->p_જમણું;
    જો( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    પાછા 1;
  }નહીંતર{
    પાછા 0;
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
પૂર્ણાંક sqlite3RowSetTest(RowSet *pRowSet, પૂર્ણાંક iBatch, sqlite3_int64 iRowid){
  સંરચના RowSetEntry *p, *p_વૃક્ષ;

  /* This routine is never called after sqlite3RowSetNext() */
  પ્રતિજ્ઞા( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  જો( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    જો( p ){
      સંરચના RowSetEntry **pp_Prev_વૃક્ષ = &pRowSet->pForest;
      જો( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = rowSetEntrySort(p);
      }
      માટે(p_વૃક્ષ = pRowSet->pForest; p_વૃક્ષ; p_વૃક્ષ=p_વૃક્ષ->p_જમણું){
        pp_Prev_વૃક્ષ = &p_વૃક્ષ->p_જમણું;
        જો( p_વૃક્ષ->p_ડાબું==0 ){
          p_વૃક્ષ->p_ડાબું = row_Set_List_To_વૃક્ષ(p);
          તોડો;
        }નહીંતર{
          સંરચના RowSetEntry *pAux, *pTail;
          row_Set_વૃક્ષ_To_List(p_વૃક્ષ->p_ડાબું, &pAux, &pTail);
          p_વૃક્ષ->p_ડાબું = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      જો( p_વૃક્ષ==0 ){
        *pp_Prev_વૃક્ષ = p_વૃક્ષ = rowSetEntryAlloc(pRowSet);
        જો( p_વૃક્ષ ){
          p_વૃક્ષ->v = 0;
          p_વૃક્ષ->p_જમણું = 0;
          p_વૃક્ષ->p_ડાબું = row_Set_List_To_વૃક્ષ(p);
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
  માટે(p_વૃક્ષ = pRowSet->pForest; p_વૃક્ષ; p_વૃક્ષ=p_વૃક્ષ->p_જમણું){
    p = p_વૃક્ષ->p_ડાબું;
    જ્યારે( p ){
      જો( p->v<iRowid ){
        p = p->p_જમણું;
      }નહીંતર જો( p->v>iRowid ){
        p = p->p_ડાબું;
      }નહીંતર{
        પાછા 1;
      }
    }
  }
  પાછા 0;
}
