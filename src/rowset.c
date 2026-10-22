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
#وصف_ڪريو ROWSET_ورهاست_سائيز 1024

/*
** The number of rowset entries per allocation chunk.
*/
#وصف_ڪريو ROWSET_داخلا_في_CHUNK  \
                       ((ROWSET_ورهاست_سائيز-8)/حجم(بنية صف_سيٽ_داخلا))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
بنية صف_سيٽ_داخلا {           
  i64 v;                        /* ROWID value for this entry */
  بنية صف_سيٽ_داخلا *p_ساڄو;   /* Right subtree (larger entries) or list */
  بنية صف_سيٽ_داخلا *p_کاٻو;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
بنية صف_سيٽ_Chunk {
  بنية صف_سيٽ_Chunk *p_ايندڙ_Chunk;        /* Next chunk on list of them all */
  بنية صف_سيٽ_داخلا a_داخلا[ROWSET_داخلا_في_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
بنية صف_سيٽ {
  بنية صف_سيٽ_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  بنية صف_سيٽ_داخلا *p_داخلا;    /* List of entries using pRight */
  بنية صف_سيٽ_داخلا *p_آخري;     /* Last entry on the pEntry list */
  بنية صف_سيٽ_داخلا *pFresh;    /* Source of new entry objects */
  بنية صف_سيٽ_داخلا *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  صحيح iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#وصف_ڪريو ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#وصف_ڪريو ROWSET_ايندڙ    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
صف_سيٽ *sqlite3_صف_سيٽ_Init(sqlite3 *db){
  صف_سيٽ *p = sqlite3DbMallocRawNN(db, حجم(*p));
  إذا( p ){
    صحيح N = sqlite3_Db_Malloc_سائيز(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_داخلا = 0;
    p->p_آخري = 0;
    p->pForest = 0;
    p->pFresh = (بنية صف_سيٽ_داخلا*)(ROUND8(حجم(*p)) + (حرف*)p);
    p->nFresh = (u16)((N - ROUND8(حجم(*p)))/حجم(بنية صف_سيٽ_داخلا));
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
فراغ sqlite3_صف_سيٽ_Clear(فراغ *pArg){
  صف_سيٽ *p = (صف_سيٽ*)pArg;
  بنية صف_سيٽ_Chunk *pChunk, *p_ايندڙ_Chunk;
  لكل(pChunk=p->pChunk; pChunk; pChunk = p_ايندڙ_Chunk){
    p_ايندڙ_Chunk = pChunk->p_ايندڙ_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_داخلا = 0;
  p->p_آخري = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
فراغ sqlite3_صف_سيٽ_Delete(فراغ *pArg){
  sqlite3_صف_سيٽ_Clear(pArg);
  sqlite3DbFree(((صف_سيٽ*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ساكن بنية صف_سيٽ_داخلا *صف_سيٽ_داخلا_Alloc(صف_سيٽ *p){
  أكد( p!=0 );
  إذا( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    بنية صف_سيٽ_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, حجم(*pNew));
    إذا( pNew==0 ){
      أرجع 0;
    }
    pNew->p_ايندڙ_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_داخلا;
    p->nFresh = ROWSET_داخلا_في_CHUNK;
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
فراغ sqlite3_صف_سيٽ_وجهو(صف_سيٽ *p, i64 rowid){
  بنية صف_سيٽ_داخلا *p_داخلا;  /* The new entry */
  بنية صف_سيٽ_داخلا *p_آخري;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p!=0 && (p->rsFlags & ROWSET_ايندڙ)==0 );

  p_داخلا = صف_سيٽ_داخلا_Alloc(p);
  إذا( p_داخلا==0 ) أرجع;
  p_داخلا->v = rowid;
  p_داخلا->p_ساڄو = 0;
  p_آخري = p->p_آخري;
  إذا( p_آخري ){
    إذا( rowid<=p_آخري->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_آخري->p_ساڄو = p_داخلا;
  }وإلا{
    p->p_داخلا = p_داخلا;
  }
  p->p_آخري = p_داخلا;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ساكن بنية صف_سيٽ_داخلا *صف_سيٽ_داخلا_ملايو(
  بنية صف_سيٽ_داخلا *pA,    /* First sorted list to be merged */
  بنية صف_سيٽ_داخلا *pB     /* Second sorted list to be merged */
){
  بنية صف_سيٽ_داخلا سر;
  بنية صف_سيٽ_داخلا *p_پڇ;

  p_پڇ = &سر;
  أكد( pA!=0 && pB!=0 );
  لكل(;;){
    أكد( pA->p_ساڄو==0 || pA->v<=pA->p_ساڄو->v );
    أكد( pB->p_ساڄو==0 || pB->v<=pB->p_ساڄو->v );
    إذا( pA->v<=pB->v ){
      إذا( pA->v<pB->v ) p_پڇ = p_پڇ->p_ساڄو = pA;
      pA = pA->p_ساڄو;
      إذا( pA==0 ){
        p_پڇ->p_ساڄو = pB;
        اكسر;
      }
    }وإلا{
      p_پڇ = p_پڇ->p_ساڄو = pB;
      pB = pB->p_ساڄو;
      إذا( pB==0 ){
        p_پڇ->p_ساڄو = pA;
        اكسر;
      }
    }
  }
  أرجع سر.p_ساڄو;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ساكن بنية صف_سيٽ_داخلا *صف_سيٽ_داخلا_ترتيب_ڏيو(بنية صف_سيٽ_داخلا *p_اندريون){
  بنانشان صحيح i;
  بنية صف_سيٽ_داخلا *p_ايندڙ, *a_ٻالٽي[40];

  memset(a_ٻالٽي, 0, حجم(a_ٻالٽي));
  طالما( p_اندريون ){
    p_ايندڙ = p_اندريون->p_ساڄو;
    p_اندريون->p_ساڄو = 0;
    لكل(i=0; a_ٻالٽي[i]; i++){
      p_اندريون = صف_سيٽ_داخلا_ملايو(a_ٻالٽي[i], p_اندريون);
      a_ٻالٽي[i] = 0;
    }
    a_ٻالٽي[i] = p_اندريون;
    p_اندريون = p_ايندڙ;
  }
  p_اندريون = a_ٻالٽي[0];
  لكل(i=1; i<حجم(a_ٻالٽي)/حجم(a_ٻالٽي[0]); i++){
    إذا( a_ٻالٽي[i]==0 ) استمر;
    p_اندريون = p_اندريون ? صف_سيٽ_داخلا_ملايو(p_اندريون, a_ٻالٽي[i]) : a_ٻالٽي[i];
  }
  أرجع p_اندريون;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ساكن فراغ صف_سيٽ_وڻ_منزل_List(
  بنية صف_سيٽ_داخلا *p_اندريون,         /* Root of the input tree */
  بنية صف_سيٽ_داخلا **pp_پهريون,    /* Write head of the output list here */
  بنية صف_سيٽ_داخلا **pp_آخري      /* Write tail of the output list here */
){
  أكد( p_اندريون!=0 );
  إذا( p_اندريون->p_کاٻو ){
    بنية صف_سيٽ_داخلا *p;
    صف_سيٽ_وڻ_منزل_List(p_اندريون->p_کاٻو, pp_پهريون, &p);
    p->p_ساڄو = p_اندريون;
  }وإلا{
    *pp_پهريون = p_اندريون;
  }
  إذا( p_اندريون->p_ساڄو ){
    صف_سيٽ_وڻ_منزل_List(p_اندريون->p_ساڄو, &p_اندريون->p_ساڄو, pp_آخري);
  }وإلا{
    *pp_آخري = p_اندريون;
  }
  أكد( (*pp_آخري)->p_ساڄو==0 );
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
ساكن بنية صف_سيٽ_داخلا *صف_سيٽ_NDeep_وڻ(
  بنية صف_سيٽ_داخلا **ppList,
  صحيح i_اونهائي
){
  بنية صف_سيٽ_داخلا *p;         /* Root of the new tree */
  بنية صف_سيٽ_داخلا *p_کاٻو;     /* Left subtree */
  إذا( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    أرجع 0;
  }
  إذا( i_اونهائي>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_کاٻو = صف_سيٽ_NDeep_وڻ(ppList, i_اونهائي-1);
    p = *ppList;
    إذا( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      أرجع p_کاٻو;
    }
    p->p_کاٻو = p_کاٻو;
    *ppList = p->p_ساڄو;
    p->p_ساڄو = صف_سيٽ_NDeep_وڻ(ppList, i_اونهائي-1);
  }وإلا{
    p = *ppList;
    *ppList = p->p_ساڄو;
    p->p_کاٻو = p->p_ساڄو = 0;
  }
  أرجع p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ساكن بنية صف_سيٽ_داخلا *صف_سيٽ_List_منزل_وڻ(بنية صف_سيٽ_داخلا *pList){
  صحيح i_اونهائي;           /* Depth of the tree so far */
  بنية صف_سيٽ_داخلا *p;       /* Current tree root */
  بنية صف_سيٽ_داخلا *p_کاٻو;   /* Left subtree */

  أكد( pList!=0 );
  p = pList;
  pList = p->p_ساڄو;
  p->p_کاٻو = p->p_ساڄو = 0;
  لكل(i_اونهائي=1; pList; i_اونهائي++){
    p_کاٻو = p;
    p = pList;
    pList = p->p_ساڄو;
    p->p_کاٻو = p_کاٻو;
    p->p_ساڄو = صف_سيٽ_NDeep_وڻ(&pList, i_اونهائي);
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
صحيح sqlite3_صف_سيٽ_ايندڙ(صف_سيٽ *p, i64 *pRowid){
  أكد( p!=0 );
  أكد( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  إذا( (p->rsFlags & ROWSET_ايندڙ)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    إذا( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_داخلا = صف_سيٽ_داخلا_ترتيب_ڏيو(p->p_داخلا);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_ايندڙ;
  }

  /* Return the next entry on the list */
  إذا( p->p_داخلا ){
    *pRowid = p->p_داخلا->v;
    p->p_داخلا = p->p_داخلا->p_ساڄو;
    إذا( p->p_داخلا==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_صف_سيٽ_Clear(p);
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
صحيح sqlite3_صف_سيٽ_Test(صف_سيٽ *p_صف_سيٽ, صحيح iBatch, sqlite3_int64 iRowid){
  بنية صف_سيٽ_داخلا *p, *p_وڻ;

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p_صف_سيٽ!=0 && (p_صف_سيٽ->rsFlags & ROWSET_ايندڙ)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  إذا( iBatch!=p_صف_سيٽ->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_صف_سيٽ->p_داخلا;
    إذا( p ){
      بنية صف_سيٽ_داخلا **pp_Prev_وڻ = &p_صف_سيٽ->pForest;
      إذا( (p_صف_سيٽ->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = صف_سيٽ_داخلا_ترتيب_ڏيو(p);
      }
      لكل(p_وڻ = p_صف_سيٽ->pForest; p_وڻ; p_وڻ=p_وڻ->p_ساڄو){
        pp_Prev_وڻ = &p_وڻ->p_ساڄو;
        إذا( p_وڻ->p_کاٻو==0 ){
          p_وڻ->p_کاٻو = صف_سيٽ_List_منزل_وڻ(p);
          اكسر;
        }وإلا{
          بنية صف_سيٽ_داخلا *pAux, *p_پڇ;
          صف_سيٽ_وڻ_منزل_List(p_وڻ->p_کاٻو, &pAux, &p_پڇ);
          p_وڻ->p_کاٻو = 0;
          p = صف_سيٽ_داخلا_ملايو(pAux, p);
        }
      }
      إذا( p_وڻ==0 ){
        *pp_Prev_وڻ = p_وڻ = صف_سيٽ_داخلا_Alloc(p_صف_سيٽ);
        إذا( p_وڻ ){
          p_وڻ->v = 0;
          p_وڻ->p_ساڄو = 0;
          p_وڻ->p_کاٻو = صف_سيٽ_List_منزل_وڻ(p);
        }
      }
      p_صف_سيٽ->p_داخلا = 0;
      p_صف_سيٽ->p_آخري = 0;
      p_صف_سيٽ->rsFlags |= ROWSET_SORTED;
    }
    p_صف_سيٽ->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  لكل(p_وڻ = p_صف_سيٽ->pForest; p_وڻ; p_وڻ=p_وڻ->p_ساڄو){
    p = p_وڻ->p_کاٻو;
    طالما( p ){
      إذا( p->v<iRowid ){
        p = p->p_ساڄو;
      }وإلا إذا( p->v>iRowid ){
        p = p->p_کاٻو;
      }وإلا{
        أرجع 1;
      }
    }
  }
  أرجع 0;
}
