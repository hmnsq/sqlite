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
#تعریف_کړه ROWSET_ویش_اندازه 1024

/*
** The number of rowset entries per allocation chunk.
*/
#تعریف_کړه ROWSET_ثبت_فی_CHUNK  \
                       ((ROWSET_ویش_اندازه-8)/حجم(بنية لیکه_سېټ_ثبت))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
بنية لیکه_سېټ_ثبت {           
  i64 v;                        /* ROWID value for this entry */
  بنية لیکه_سېټ_ثبت *p_ښی;   /* Right subtree (larger entries) or list */
  بنية لیکه_سېټ_ثبت *p_کیڼ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
بنية لیکه_سېټ_Chunk {
  بنية لیکه_سېټ_Chunk *p_بل_Chunk;        /* Next chunk on list of them all */
  بنية لیکه_سېټ_ثبت a_ثبت[ROWSET_ثبت_فی_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
بنية لیکه_سېټ {
  بنية لیکه_سېټ_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  بنية لیکه_سېټ_ثبت *p_ثبت;    /* List of entries using pRight */
  بنية لیکه_سېټ_ثبت *p_وروستی;     /* Last entry on the pEntry list */
  بنية لیکه_سېټ_ثبت *pFresh;    /* Source of new entry objects */
  بنية لیکه_سېټ_ثبت *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  صحيح iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#تعریف_کړه ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#تعریف_کړه ROWSET_بل    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
لیکه_سېټ *sqlite3_لیکه_سېټ_Init(sqlite3 *db){
  لیکه_سېټ *p = sqlite3DbMallocRawNN(db, حجم(*p));
  إذا( p ){
    صحيح N = sqlite3_Db_Malloc_اندازه(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_ثبت = 0;
    p->p_وروستی = 0;
    p->pForest = 0;
    p->pFresh = (بنية لیکه_سېټ_ثبت*)(ROUND8(حجم(*p)) + (حرف*)p);
    p->nFresh = (u16)((N - ROUND8(حجم(*p)))/حجم(بنية لیکه_سېټ_ثبت));
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
فراغ sqlite3_لیکه_سېټ_Clear(فراغ *pArg){
  لیکه_سېټ *p = (لیکه_سېټ*)pArg;
  بنية لیکه_سېټ_Chunk *pChunk, *p_بل_Chunk;
  لكل(pChunk=p->pChunk; pChunk; pChunk = p_بل_Chunk){
    p_بل_Chunk = pChunk->p_بل_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_ثبت = 0;
  p->p_وروستی = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
فراغ sqlite3_لیکه_سېټ_Delete(فراغ *pArg){
  sqlite3_لیکه_سېټ_Clear(pArg);
  sqlite3DbFree(((لیکه_سېټ*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ساكن بنية لیکه_سېټ_ثبت *لیکه_سېټ_ثبت_Alloc(لیکه_سېټ *p){
  أكد( p!=0 );
  إذا( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    بنية لیکه_سېټ_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, حجم(*pNew));
    إذا( pNew==0 ){
      أرجع 0;
    }
    pNew->p_بل_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_ثبت;
    p->nFresh = ROWSET_ثبت_فی_CHUNK;
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
فراغ sqlite3_لیکه_سېټ_ورننباسه(لیکه_سېټ *p, i64 rowid){
  بنية لیکه_سېټ_ثبت *p_ثبت;  /* The new entry */
  بنية لیکه_سېټ_ثبت *p_وروستی;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p!=0 && (p->rsFlags & ROWSET_بل)==0 );

  p_ثبت = لیکه_سېټ_ثبت_Alloc(p);
  إذا( p_ثبت==0 ) أرجع;
  p_ثبت->v = rowid;
  p_ثبت->p_ښی = 0;
  p_وروستی = p->p_وروستی;
  إذا( p_وروستی ){
    إذا( rowid<=p_وروستی->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_وروستی->p_ښی = p_ثبت;
  }وإلا{
    p->p_ثبت = p_ثبت;
  }
  p->p_وروستی = p_ثبت;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ساكن بنية لیکه_سېټ_ثبت *لیکه_سېټ_ثبت_یوځای(
  بنية لیکه_سېټ_ثبت *pA,    /* First sorted list to be merged */
  بنية لیکه_سېټ_ثبت *pB     /* Second sorted list to be merged */
){
  بنية لیکه_سېټ_ثبت سر_2;
  بنية لیکه_سېټ_ثبت *p_لکۍ;

  p_لکۍ = &سر_2;
  أكد( pA!=0 && pB!=0 );
  لكل(;;){
    أكد( pA->p_ښی==0 || pA->v<=pA->p_ښی->v );
    أكد( pB->p_ښی==0 || pB->v<=pB->p_ښی->v );
    إذا( pA->v<=pB->v ){
      إذا( pA->v<pB->v ) p_لکۍ = p_لکۍ->p_ښی = pA;
      pA = pA->p_ښی;
      إذا( pA==0 ){
        p_لکۍ->p_ښی = pB;
        اكسر;
      }
    }وإلا{
      p_لکۍ = p_لکۍ->p_ښی = pB;
      pB = pB->p_ښی;
      إذا( pB==0 ){
        p_لکۍ->p_ښی = pA;
        اكسر;
      }
    }
  }
  أرجع سر_2.p_ښی;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ساكن بنية لیکه_سېټ_ثبت *لیکه_سېټ_ثبت_ترتیبول(بنية لیکه_سېټ_ثبت *p_دننه){
  بینښه صحيح i;
  بنية لیکه_سېټ_ثبت *p_بل, *a_لوښی[40];

  memset(a_لوښی, 0, حجم(a_لوښی));
  طالما( p_دننه ){
    p_بل = p_دننه->p_ښی;
    p_دننه->p_ښی = 0;
    لكل(i=0; a_لوښی[i]; i++){
      p_دننه = لیکه_سېټ_ثبت_یوځای(a_لوښی[i], p_دننه);
      a_لوښی[i] = 0;
    }
    a_لوښی[i] = p_دننه;
    p_دننه = p_بل;
  }
  p_دننه = a_لوښی[0];
  لكل(i=1; i<حجم(a_لوښی)/حجم(a_لوښی[0]); i++){
    إذا( a_لوښی[i]==0 ) استمر;
    p_دننه = p_دننه ? لیکه_سېټ_ثبت_یوځای(p_دننه, a_لوښی[i]) : a_لوښی[i];
  }
  أرجع p_دننه;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ساكن فراغ لیکه_سېټ_ونه_منزل_List(
  بنية لیکه_سېټ_ثبت *p_دننه,         /* Root of the input tree */
  بنية لیکه_سېټ_ثبت **pp_لومړی,    /* Write head of the output list here */
  بنية لیکه_سېټ_ثبت **pp_وروستی      /* Write tail of the output list here */
){
  أكد( p_دننه!=0 );
  إذا( p_دننه->p_کیڼ ){
    بنية لیکه_سېټ_ثبت *p;
    لیکه_سېټ_ونه_منزل_List(p_دننه->p_کیڼ, pp_لومړی, &p);
    p->p_ښی = p_دننه;
  }وإلا{
    *pp_لومړی = p_دننه;
  }
  إذا( p_دننه->p_ښی ){
    لیکه_سېټ_ونه_منزل_List(p_دننه->p_ښی, &p_دننه->p_ښی, pp_وروستی);
  }وإلا{
    *pp_وروستی = p_دننه;
  }
  أكد( (*pp_وروستی)->p_ښی==0 );
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
ساكن بنية لیکه_سېټ_ثبت *لیکه_سېټ_NDeep_ونه(
  بنية لیکه_سېټ_ثبت **ppList,
  صحيح i_ژوروالی
){
  بنية لیکه_سېټ_ثبت *p;         /* Root of the new tree */
  بنية لیکه_سېټ_ثبت *p_کیڼ;     /* Left subtree */
  إذا( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    أرجع 0;
  }
  إذا( i_ژوروالی>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_کیڼ = لیکه_سېټ_NDeep_ونه(ppList, i_ژوروالی-1);
    p = *ppList;
    إذا( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      أرجع p_کیڼ;
    }
    p->p_کیڼ = p_کیڼ;
    *ppList = p->p_ښی;
    p->p_ښی = لیکه_سېټ_NDeep_ونه(ppList, i_ژوروالی-1);
  }وإلا{
    p = *ppList;
    *ppList = p->p_ښی;
    p->p_کیڼ = p->p_ښی = 0;
  }
  أرجع p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ساكن بنية لیکه_سېټ_ثبت *لیکه_سېټ_List_منزل_ونه(بنية لیکه_سېټ_ثبت *pList){
  صحيح i_ژوروالی;           /* Depth of the tree so far */
  بنية لیکه_سېټ_ثبت *p;       /* Current tree root */
  بنية لیکه_سېټ_ثبت *p_کیڼ;   /* Left subtree */

  أكد( pList!=0 );
  p = pList;
  pList = p->p_ښی;
  p->p_کیڼ = p->p_ښی = 0;
  لكل(i_ژوروالی=1; pList; i_ژوروالی++){
    p_کیڼ = p;
    p = pList;
    pList = p->p_ښی;
    p->p_کیڼ = p_کیڼ;
    p->p_ښی = لیکه_سېټ_NDeep_ونه(&pList, i_ژوروالی);
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
صحيح sqlite3_لیکه_سېټ_بل(لیکه_سېټ *p, i64 *pRowid){
  أكد( p!=0 );
  أكد( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  إذا( (p->rsFlags & ROWSET_بل)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    إذا( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_ثبت = لیکه_سېټ_ثبت_ترتیبول(p->p_ثبت);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_بل;
  }

  /* Return the next entry on the list */
  إذا( p->p_ثبت ){
    *pRowid = p->p_ثبت->v;
    p->p_ثبت = p->p_ثبت->p_ښی;
    إذا( p->p_ثبت==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_لیکه_سېټ_Clear(p);
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
صحيح sqlite3_لیکه_سېټ_Test(لیکه_سېټ *p_لیکه_سېټ, صحيح iBatch, sqlite3_int64 iRowid){
  بنية لیکه_سېټ_ثبت *p, *p_ونه;

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p_لیکه_سېټ!=0 && (p_لیکه_سېټ->rsFlags & ROWSET_بل)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  إذا( iBatch!=p_لیکه_سېټ->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_لیکه_سېټ->p_ثبت;
    إذا( p ){
      بنية لیکه_سېټ_ثبت **pp_Prev_ونه = &p_لیکه_سېټ->pForest;
      إذا( (p_لیکه_سېټ->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = لیکه_سېټ_ثبت_ترتیبول(p);
      }
      لكل(p_ونه = p_لیکه_سېټ->pForest; p_ونه; p_ونه=p_ونه->p_ښی){
        pp_Prev_ونه = &p_ونه->p_ښی;
        إذا( p_ونه->p_کیڼ==0 ){
          p_ونه->p_کیڼ = لیکه_سېټ_List_منزل_ونه(p);
          اكسر;
        }وإلا{
          بنية لیکه_سېټ_ثبت *pAux, *p_لکۍ;
          لیکه_سېټ_ونه_منزل_List(p_ونه->p_کیڼ, &pAux, &p_لکۍ);
          p_ونه->p_کیڼ = 0;
          p = لیکه_سېټ_ثبت_یوځای(pAux, p);
        }
      }
      إذا( p_ونه==0 ){
        *pp_Prev_ونه = p_ونه = لیکه_سېټ_ثبت_Alloc(p_لیکه_سېټ);
        إذا( p_ونه ){
          p_ونه->v = 0;
          p_ونه->p_ښی = 0;
          p_ونه->p_کیڼ = لیکه_سېټ_List_منزل_ونه(p);
        }
      }
      p_لیکه_سېټ->p_ثبت = 0;
      p_لیکه_سېټ->p_وروستی = 0;
      p_لیکه_سېټ->rsFlags |= ROWSET_SORTED;
    }
    p_لیکه_سېټ->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  لكل(p_ونه = p_لیکه_سېټ->pForest; p_ونه; p_ونه=p_ونه->p_ښی){
    p = p_ونه->p_کیڼ;
    طالما( p ){
      إذا( p->v<iRowid ){
        p = p->p_ښی;
      }وإلا إذا( p->v>iRowid ){
        p = p->p_کیڼ;
      }وإلا{
        أرجع 1;
      }
    }
  }
  أرجع 0;
}
