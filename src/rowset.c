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
#परिभाषय ROWSET_आवण्टनम्_आकार 1024

/*
** The number of rowset entries per allocation chunk.
*/
#परिभाषय ROWSET_प्रविष्टिः_प्रत्येकम्_CHUNK  \
                       ((ROWSET_आवण्टनम्_आकार-8)/परिमाण(संरचना Row_समुच्चयः_प्रविष्टिः))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
संरचना Row_समुच्चयः_प्रविष्टिः {           
  i64 v;                        /* ROWID value for this entry */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_दक्षिणः;   /* Right subtree (larger entries) or list */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_वामः;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
संरचना Row_समुच्चयः_Chunk {
  संरचना Row_समुच्चयः_Chunk *p_अग्र_Chunk;        /* Next chunk on list of them all */
  संरचना Row_समुच्चयः_प्रविष्टिः a_प्रविष्टिः[ROWSET_प्रविष्टिः_प्रत्येकम्_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
संरचना Row_समुच्चयः {
  संरचना Row_समुच्चयः_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_प्रविष्टिः;    /* List of entries using pRight */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_चरमः;     /* Last entry on the pEntry list */
  संरचना Row_समुच्चयः_प्रविष्टिः *pFresh;    /* Source of new entry objects */
  संरचना Row_समुच्चयः_प्रविष्टिः *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  पूर्णाङ्क iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#परिभाषय ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#परिभाषय ROWSET_अग्र    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
Row_समुच्चयः *sqlite3_Row_समुच्चयः_Init(sqlite3 *db){
  Row_समुच्चयः *p = sqlite3DbMallocRawNN(db, परिमाण(*p));
  यदि( p ){
    पूर्णाङ्क N = sqlite3_Db_Malloc_आकार(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_प्रविष्टिः = 0;
    p->p_चरमः = 0;
    p->pForest = 0;
    p->pFresh = (संरचना Row_समुच्चयः_प्रविष्टिः*)(ROUND8(परिमाण(*p)) + (वर्ण*)p);
    p->nFresh = (u16)((N - ROUND8(परिमाण(*p)))/परिमाण(संरचना Row_समुच्चयः_प्रविष्टिः));
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
शून्य sqlite3_Row_समुच्चयः_Clear(शून्य *pArg){
  Row_समुच्चयः *p = (Row_समुच्चयः*)pArg;
  संरचना Row_समुच्चयः_Chunk *pChunk, *p_अग्र_Chunk;
  प्रति(pChunk=p->pChunk; pChunk; pChunk = p_अग्र_Chunk){
    p_अग्र_Chunk = pChunk->p_अग्र_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_प्रविष्टिः = 0;
  p->p_चरमः = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
शून्य sqlite3_Row_समुच्चयः_Delete(शून्य *pArg){
  sqlite3_Row_समुच्चयः_Clear(pArg);
  sqlite3DbFree(((Row_समुच्चयः*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
स्थिर संरचना Row_समुच्चयः_प्रविष्टिः *row_समुच्चयः_प्रविष्टिः_Alloc(Row_समुच्चयः *p){
  प्रतिज्ञा( p!=0 );
  यदि( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    संरचना Row_समुच्चयः_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, परिमाण(*pNew));
    यदि( pNew==0 ){
      प्रत्यागम 0;
    }
    pNew->p_अग्र_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_प्रविष्टिः;
    p->nFresh = ROWSET_प्रविष्टिः_प्रत्येकम्_CHUNK;
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
शून्य sqlite3_Row_समुच्चयः_निवेशय(Row_समुच्चयः *p, i64 rowid){
  संरचना Row_समुच्चयः_प्रविष्टिः *p_प्रविष्टिः;  /* The new entry */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_चरमः;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  प्रतिज्ञा( p!=0 && (p->rsFlags & ROWSET_अग्र)==0 );

  p_प्रविष्टिः = row_समुच्चयः_प्रविष्टिः_Alloc(p);
  यदि( p_प्रविष्टिः==0 ) प्रत्यागम;
  p_प्रविष्टिः->v = rowid;
  p_प्रविष्टिः->p_दक्षिणः = 0;
  p_चरमः = p->p_चरमः;
  यदि( p_चरमः ){
    यदि( rowid<=p_चरमः->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_चरमः->p_दक्षिणः = p_प्रविष्टिः;
  }अन्यथा{
    p->p_प्रविष्टिः = p_प्रविष्टिः;
  }
  p->p_चरमः = p_प्रविष्टिः;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
स्थिर संरचना Row_समुच्चयः_प्रविष्टिः *row_समुच्चयः_प्रविष्टिः_Merge(
  संरचना Row_समुच्चयः_प्रविष्टिः *pA,    /* First sorted list to be merged */
  संरचना Row_समुच्चयः_प्रविष्टिः *pB     /* Second sorted list to be merged */
){
  संरचना Row_समुच्चयः_प्रविष्टिः शीर्ष;
  संरचना Row_समुच्चयः_प्रविष्टिः *p_पुच्छ;

  p_पुच्छ = &शीर्ष;
  प्रतिज्ञा( pA!=0 && pB!=0 );
  प्रति(;;){
    प्रतिज्ञा( pA->p_दक्षिणः==0 || pA->v<=pA->p_दक्षिणः->v );
    प्रतिज्ञा( pB->p_दक्षिणः==0 || pB->v<=pB->p_दक्षिणः->v );
    यदि( pA->v<=pB->v ){
      यदि( pA->v<pB->v ) p_पुच्छ = p_पुच्छ->p_दक्षिणः = pA;
      pA = pA->p_दक्षिणः;
      यदि( pA==0 ){
        p_पुच्छ->p_दक्षिणः = pB;
        भङ्ग;
      }
    }अन्यथा{
      p_पुच्छ = p_पुच्छ->p_दक्षिणः = pB;
      pB = pB->p_दक्षिणः;
      यदि( pB==0 ){
        p_पुच्छ->p_दक्षिणः = pA;
        भङ्ग;
      }
    }
  }
  प्रत्यागम शीर्ष.p_दक्षिणः;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
स्थिर संरचना Row_समुच्चयः_प्रविष्टिः *row_समुच्चयः_प्रविष्टिः_क्रमय(संरचना Row_समुच्चयः_प्रविष्टिः *p_अन्तर्){
  अचिह्नित पूर्णाङ्क i;
  संरचना Row_समुच्चयः_प्रविष्टिः *p_अग्र, *aBucket[40];

  स्मृतिं_पूरय(aBucket, 0, परिमाण(aBucket));
  यावत्( p_अन्तर् ){
    p_अग्र = p_अन्तर्->p_दक्षिणः;
    p_अन्तर्->p_दक्षिणः = 0;
    प्रति(i=0; aBucket[i]; i++){
      p_अन्तर् = row_समुच्चयः_प्रविष्टिः_Merge(aBucket[i], p_अन्तर्);
      aBucket[i] = 0;
    }
    aBucket[i] = p_अन्तर्;
    p_अन्तर् = p_अग्र;
  }
  p_अन्तर् = aBucket[0];
  प्रति(i=1; i<परिमाण(aBucket)/परिमाण(aBucket[0]); i++){
    यदि( aBucket[i]==0 ) अनुवर्त;
    p_अन्तर् = p_अन्तर् ? row_समुच्चयः_प्रविष्टिः_Merge(p_अन्तर्, aBucket[i]) : aBucket[i];
  }
  प्रत्यागम p_अन्तर्;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
स्थिर शून्य row_समुच्चयः_वृक्षः_गन्तव्यम्_List(
  संरचना Row_समुच्चयः_प्रविष्टिः *p_अन्तर्,         /* Root of the input tree */
  संरचना Row_समुच्चयः_प्रविष्टिः **ppFirst,    /* Write head of the output list here */
  संरचना Row_समुच्चयः_प्रविष्टिः **pp_चरमः      /* Write tail of the output list here */
){
  प्रतिज्ञा( p_अन्तर्!=0 );
  यदि( p_अन्तर्->p_वामः ){
    संरचना Row_समुच्चयः_प्रविष्टिः *p;
    row_समुच्चयः_वृक्षः_गन्तव्यम्_List(p_अन्तर्->p_वामः, ppFirst, &p);
    p->p_दक्षिणः = p_अन्तर्;
  }अन्यथा{
    *ppFirst = p_अन्तर्;
  }
  यदि( p_अन्तर्->p_दक्षिणः ){
    row_समुच्चयः_वृक्षः_गन्तव्यम्_List(p_अन्तर्->p_दक्षिणः, &p_अन्तर्->p_दक्षिणः, pp_चरमः);
  }अन्यथा{
    *pp_चरमः = p_अन्तर्;
  }
  प्रतिज्ञा( (*pp_चरमः)->p_दक्षिणः==0 );
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
स्थिर संरचना Row_समुच्चयः_प्रविष्टिः *row_समुच्चयः_NDeep_वृक्षः(
  संरचना Row_समुच्चयः_प्रविष्टिः **ppList,
  पूर्णाङ्क i_गभीरता
){
  संरचना Row_समुच्चयः_प्रविष्टिः *p;         /* Root of the new tree */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_वामः;     /* Left subtree */
  यदि( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    प्रत्यागम 0;
  }
  यदि( i_गभीरता>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_वामः = row_समुच्चयः_NDeep_वृक्षः(ppList, i_गभीरता-1);
    p = *ppList;
    यदि( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      प्रत्यागम p_वामः;
    }
    p->p_वामः = p_वामः;
    *ppList = p->p_दक्षिणः;
    p->p_दक्षिणः = row_समुच्चयः_NDeep_वृक्षः(ppList, i_गभीरता-1);
  }अन्यथा{
    p = *ppList;
    *ppList = p->p_दक्षिणः;
    p->p_वामः = p->p_दक्षिणः = 0;
  }
  प्रत्यागम p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
स्थिर संरचना Row_समुच्चयः_प्रविष्टिः *row_समुच्चयः_List_गन्तव्यम्_वृक्षः(संरचना Row_समुच्चयः_प्रविष्टिः *pList){
  पूर्णाङ्क i_गभीरता;           /* Depth of the tree so far */
  संरचना Row_समुच्चयः_प्रविष्टिः *p;       /* Current tree root */
  संरचना Row_समुच्चयः_प्रविष्टिः *p_वामः;   /* Left subtree */

  प्रतिज्ञा( pList!=0 );
  p = pList;
  pList = p->p_दक्षिणः;
  p->p_वामः = p->p_दक्षिणः = 0;
  प्रति(i_गभीरता=1; pList; i_गभीरता++){
    p_वामः = p;
    p = pList;
    pList = p->p_दक्षिणः;
    p->p_वामः = p_वामः;
    p->p_दक्षिणः = row_समुच्चयः_NDeep_वृक्षः(&pList, i_गभीरता);
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
पूर्णाङ्क sqlite3_Row_समुच्चयः_अग्र(Row_समुच्चयः *p, i64 *pRowid){
  प्रतिज्ञा( p!=0 );
  प्रतिज्ञा( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  यदि( (p->rsFlags & ROWSET_अग्र)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    यदि( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_प्रविष्टिः = row_समुच्चयः_प्रविष्टिः_क्रमय(p->p_प्रविष्टिः);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_अग्र;
  }

  /* Return the next entry on the list */
  यदि( p->p_प्रविष्टिः ){
    *pRowid = p->p_प्रविष्टिः->v;
    p->p_प्रविष्टिः = p->p_प्रविष्टिः->p_दक्षिणः;
    यदि( p->p_प्रविष्टिः==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_Row_समुच्चयः_Clear(p);
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
पूर्णाङ्क sqlite3_Row_समुच्चयः_Test(Row_समुच्चयः *p_Row_समुच्चयः, पूर्णाङ्क iBatch, sqlite3_int64 iRowid){
  संरचना Row_समुच्चयः_प्रविष्टिः *p, *p_वृक्षः;

  /* This routine is never called after sqlite3RowSetNext() */
  प्रतिज्ञा( p_Row_समुच्चयः!=0 && (p_Row_समुच्चयः->rsFlags & ROWSET_अग्र)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  यदि( iBatch!=p_Row_समुच्चयः->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_Row_समुच्चयः->p_प्रविष्टिः;
    यदि( p ){
      संरचना Row_समुच्चयः_प्रविष्टिः **pp_पूर्व_वृक्षः = &p_Row_समुच्चयः->pForest;
      यदि( (p_Row_समुच्चयः->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = row_समुच्चयः_प्रविष्टिः_क्रमय(p);
      }
      प्रति(p_वृक्षः = p_Row_समुच्चयः->pForest; p_वृक्षः; p_वृक्षः=p_वृक्षः->p_दक्षिणः){
        pp_पूर्व_वृक्षः = &p_वृक्षः->p_दक्षिणः;
        यदि( p_वृक्षः->p_वामः==0 ){
          p_वृक्षः->p_वामः = row_समुच्चयः_List_गन्तव्यम्_वृक्षः(p);
          भङ्ग;
        }अन्यथा{
          संरचना Row_समुच्चयः_प्रविष्टिः *pAux, *p_पुच्छ;
          row_समुच्चयः_वृक्षः_गन्तव्यम्_List(p_वृक्षः->p_वामः, &pAux, &p_पुच्छ);
          p_वृक्षः->p_वामः = 0;
          p = row_समुच्चयः_प्रविष्टिः_Merge(pAux, p);
        }
      }
      यदि( p_वृक्षः==0 ){
        *pp_पूर्व_वृक्षः = p_वृक्षः = row_समुच्चयः_प्रविष्टिः_Alloc(p_Row_समुच्चयः);
        यदि( p_वृक्षः ){
          p_वृक्षः->v = 0;
          p_वृक्षः->p_दक्षिणः = 0;
          p_वृक्षः->p_वामः = row_समुच्चयः_List_गन्तव्यम्_वृक्षः(p);
        }
      }
      p_Row_समुच्चयः->p_प्रविष्टिः = 0;
      p_Row_समुच्चयः->p_चरमः = 0;
      p_Row_समुच्चयः->rsFlags |= ROWSET_SORTED;
    }
    p_Row_समुच्चयः->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  प्रति(p_वृक्षः = p_Row_समुच्चयः->pForest; p_वृक्षः; p_वृक्षः=p_वृक्षः->p_दक्षिणः){
    p = p_वृक्षः->p_वामः;
    यावत्( p ){
      यदि( p->v<iRowid ){
        p = p->p_दक्षिणः;
      }अन्यथा यदि( p->v>iRowid ){
        p = p->p_वामः;
      }अन्यथा{
        प्रत्यागम 1;
      }
    }
  }
  प्रत्यागम 0;
}
